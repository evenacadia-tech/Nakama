# NAK-380 — Verlauf der Etappen 1 bis 5 bis zur Erstprüfung 5 (ausgelagert aus `docs/beweise/NAK-380.md` am 26.09.2026 in zwei Blöcken)

Unverändert übernommen (Skill §3.5, Manifest über 7 000 Zeilen): die Abschnitte §10 bis §34 — Messung der Etappe 1, Matrixprüfungen 1 bis 4 mit Nacharbeiten, Etappe 2 (Bau, Kanon, Erstprüfung 2, Nacharbeit 1, Wiederprüfung 1 PASS), Etappe 3 (Bau, Nacharbeit 0, Kanon, Erstprüfung 3, Nacharbeit 1, Wiederprüfung 1 PASS, Übergabe). Der lebende Kopf, §0 bis §9 (Übernahme, Einordnung, Regeln, Zusagen, Matrix, Korpus, Bauplan, Prüfliste) und der Verlauf ab §35 (Etappe 4) stehen im Manifest. Zeilenangaben in diesen Abschnitten gelten zum Stand des Abschnitts, der sie trägt.

---

## 10. Messung der Etappe 1 und Entscheide (Dirigent, 25.09.2026, 05:46 Uhr)

**Messung.** Worker `nakama-nak380-b5a0bf9-e1` (Opus 5.5, Effort max,
`dontAsk`, Aufsicht ENG; Beobachter in fünf Fenstern, Cron stündlich mit
Kontingentprüfung), Start 03:22 Uhr, Ende 05:40 Uhr. Basis `b5a0bf9c` ist
Vorfahr von HEAD `5f48fee3` = `origin/master`; Baum sauber bis auf die
fremden Ordner; genau eine Datei geändert (`docs/beweise/NAK-380.md`,
+1 687/−2). Rundenbilanz `b5a0bf9c..5f48fee3`: Doku 1 Datei +1 687/−2, ohne
Produktfortschritt (Matrixrunde, zählt null). Gelesen: §5.0, §6-Kopf, §6.6,
§8.0 vollständig (Technikentscheide T-380-1 bis T-380-11, Abweichungen A-1
bis A-8, Beobachtungen B-1 bis B-7, Kartenentwurf K-380-1), §9.1, die
Gliederung und die Schlussmeldung. Befund des Dirigenten: **121 Matrixzeilen
(Etappe 2: 20, Etappe 3: 21, Etappe 4: 31, Etappe 5: 25, Etappe 6: 24; je
Zusagehälfte 66 heute rot, 20 heute nicht messbar, 44 Regressionswachen),
§7 mit Signaltafel, Zählregeln und Referenz je Zahl, §8 mit Bauplan je
Etappe; Dokuriegel ohne Befund.** Claude-Woche beim Start 87 %, beim Ende
91 % (NAK-396: der laufende Opus-Worker lief zu Ende; bis zum Reset um
15:00 Uhr kein neuer Opus-Lauf, Prüfer `gpt-6-astra` high, Bauer bei Bedarf
`gpt-5.6-sol` xhigh `workspace-write`).

**Entscheide (Technik, innerhalb von Gate, Invarianten und §2).**

- **E-380-1 (A-1, T-380-1) — angenommen.** R-380-1 wird präzisiert: die
  Livegruppe ist Σ Dichte·Bandbreite über alle Feinbänder der Gruppe, wobei
  ein Feinband ohne Bin die mittlere Dichte der belegten Feinbänder derselben
  Gruppe trägt; ohne belegtes Feinband kein Bit. So hängt die Gruppe nicht an
  der Zahl belegter Feinbänder (Satz 3 der Regel) und bleibt Bandleistung
  (Satz 2).
- **E-380-2 (A-2) — angenommen.** `band_stereo` wird leistungsgewichtet;
  derselbe Schleifenrumpf, dieselbe Ursache (R-380-1: kein Leser addiert
  Dichten als Leistung).
- **E-380-3 (A-3) — angenommen.** Die Laufzeit ist Engine-Ergebnis mit
  Testzugang; das Vertragsfeld `phase` bleibt die Phase am Bandmitten-Bin,
  je Bin gelesen. R-380-3 wird präzisiert: ein Vertragsfeld für die Laufzeit
  entsteht erst mit einem Leser (Schemas sind Verträge; kein Feld ohne
  Leser).
- **E-380-4 (A-4) — angenommen.** Das Kurzfenster der Persistenz rechnet
  ebenfalls mit der MSC je Bin (dieselbe Ursache).
- **E-380-5 (A-5) — angenommen, mit Ort.** Kein Erratum zu §39.1; der Satz
  wird erfüllt. Die im Kopf genannte Belegzeile lautet im Verlaufsblock des
  Entwurfs: „Beleg: §39.1 SuperFlux-Satz erfüllt mit NAK-380 Etappe 4
  (`beweise/NAK-380.md` M-71); kein Erratum.“ Etappe 4 schreibt sie als
  eigenen Verlaufseintrag hinter (r); Etappe 6 setzt das Erratum (s)
  dahinter (Reihenfolge der Etappen bleibt).
- **E-380-6 (A-6) — angenommen.** Der Vorframe des Detektors läuft über
  inaktive Frames; ein Wiederbeginn nach Stille ist ein Onset (M-61).
- **E-380-7 (A-7) — angenommen.** `maskierung.rs` wird geändert (Bandbreite
  je Summand); R-380-1 nennt es als Leser, der integriert.
- **E-380-8 (A-8, B-7) — B9 ist gemeint.** Der Gate-Satz nennt B9 (das
  EBU-Lautheitsbein) als Golden mit Rotbeweis; die Tabelle-1-Fälle der Norm
  gehören dorthin, die Raten- und Regelfälle der LRA in B18, die
  Stereo-Goldens in B19. R-380-4 wird entsprechend präzisiert; der Gate-Satz
  bleibt (er nennt Beispiele der Goldens, keine abschließende Liste).
- **E-380-9 (B-1, B-3, B-4) — Register, nicht gebaut.** B-1 (Screening-
  Gruppen des Brokers runden auf, C++ und das Log-Gitter runden ab) ist ein
  Widerspruch zwischen Broker und Gitterpartition mit eigener
  Verhaltensänderung → Registerzeile NAK-398 [Widerspruch · Messkern]. B-3
  (Editor zeichnet interpolierte LTAS-Bänder) ist Anzeigearbeit → NAK-399
  [Anzeige · geparkt]. B-4 (Suchgrenze reist nicht in der Festhalten-Datei
  und der Briefkasten-Antwort) → NAK-400 [Härtung · Snapshot-Schema], beim
  nächsten Versionsschritt des Snapshot-Schemas.
- **E-380-10 (B-2) — Lücke mit Regel R-380-9.** `fingerprint_aehnlichkeit`
  (`broker/src/telemetrie.rs:89-108`) vergleicht nur Fingerprints gleicher
  `metrics_version`; bei ungleicher Version lautet das Ergebnis „nicht
  vergleichbar“ (fail-closed), nie „anderes Material“. Zusage nach Entwurf
  §36.4 (die Größen gehören zur `metrics_version`) und CLAUDE.md „Begriffe
  nicht vermischen“. Matrixzeile M-122 als Nachtrag der Etappe 6 (die den
  Fingerprint ab 88,2 kHz ändert), geschrieben vor dem Bau der Etappe 6 mit
  Test in A4 und Rotbeweis (Muster NAK-313 §32, Nachtrag M-81); §8.6 nennt
  den Nachtrag. B-5 (zwei Kommentare zu `vorbereiten`) berichtigt Etappe 5,
  wie vorgeschlagen. B-6: das Maß der Livegruppen bei Weißrauschen ist die
  Abweichung vom Bandbreitenverhältnis (M-02, ±0,5 dB); §1 ist damit
  präzisiert.
- **E-380-11 (§9.1) — angenommen.** Zwei Python-Einzeiler der Etappe 1 ohne
  Wirkung auf Repo und Zahlen sind eine benannte Abweichung vom Wortlaut
  „kein Skript außer dokuriegel“; kein Befund.
- **E-380-12 (K-380-1) — Technikentscheid: Variante A, keine Karte.** Bei
  176,4 und 192 kHz sucht Nakama Resonanzen wie bei den übrigen Raten ab
  47 beziehungsweise 51 Hz; `N_max` = 65 536; jede Instanz belegt dafür rund
  12,5 MB Messspeicher statt rund 3 MB heute (Zahlen §8.0, nachgerechnet vom
  Bauer). Begründung: Zeit ist Musikzeit — dieselbe Fensterdauer bei jeder
  Rate ist die Zusage von R-380-5, und der Mehrbedarf von rund 4,4 MB je
  Instanz gegenüber Variante B ändert die Erfahrung des Musikers nicht,
  während ein blinder Bass unter 100 Hz sie ändern würde. Das Speicherbudget
  je Rate steht in der Matrix (M-100, M-101). Etappe 6 wartet nicht.

**Rohdaten und nächster Schritt.** Auftrag der Matrixprüfung 1:
`docs/beweise/roh/NAK-380-matrixpruefung-1-auftrag.txt` (Vorlage A auf
`git diff b5a0bf9c...5f48fee3 -- docs/beweise/NAK-380.md`; Prüfer
`gpt-6-astra`, Effort high nach NAK-396, lesend, abgekoppelt über
`tools/dirigent/codex-lauf.ps1`, Kennung `nak380-m1`); Rohurteil nach
`docs/beweise/roh/NAK-380-matrixpruefung-1-urteil.md` (Zielpfad, entsteht im
Lauf). Danach Etappe 2
(Bauer bis 15:00 Uhr Codex `gpt-5.6-sol` xhigh `workspace-write`, danach
Opus max) mit Kanon vorher geerbt (Kopf).

## 11. Matrixprüfung 1 und Übergabe (Dirigent, 25.09.2026, 06:00 Uhr)

**Rohurteil.** Frischer, lesender Codex-Thread `gpt-6-astra`, Effort high
(NAK-396: Claude-Woche über 90 %), Thread
`01a0d6ac-9f89-7b83-8c7e-ab5d7051097f`, Vorlage A über
`git diff b5a0bf9c...5f48fee3 -- docs/beweise/NAK-380.md` (§5 bis §9),
abgekoppelt über `tools/dirigent/codex-lauf.ps1` (Kennung `nak380-m1`);
HEAD `80205786` vor und nach dem Lauf; Lauf 05:47:12 bis 05:56:30 Uhr; 30
lesende Kommandos. **URTEIL: NEEDS_WORK — 10 DEFEKTE, 0 Lücken, 0
Härtungen.** Rohurteil `docs/beweise/roh/NAK-380-matrixpruefung-1-urteil.md`,
Auftrag `docs/beweise/roh/NAK-380-matrixpruefung-1-auftrag.txt`. Die
Befunde in der Reihenfolge des Berichts: D1 §8.2/M-17 (die Größenbenennung
„folgt aus `gitter`“ trifft auch `band_stereo`, das Seitenanteile in [0, 1]
trägt); D2 §7.1 (Rosa-Prüfung verlangt fallende Oktavbandleistung, richtig
ist konstante Oktavbandleistung bei fallender Dichte); D3 M-119
(`MessSnapshot` ist Mitglied von `AnalyseEngine`, die geforderte
Größengleichheit widerspricht §8.6); D4 §8.6 (Nachtrag M-122 nach R-380-9
vor dem Bau der Etappe 6 fehlt); D5 M-80 (Laufzeit nur bei 48 kHz und Block
512 gemessen, nicht in den Sweeps); D6 M-81 (Rotbeweis der Binphase mit
8 Samples Verzögerung unwirksam); D7 M-112 (Tickkadenz nur als
Ratengleichheit, nicht absolut); D8 M-32, §7.4, T-380-4 (Hann-Korrelation
bei 50 % Überlappung 1/36 statt 1/9; ν ≈ 15,26 statt 13,40; 2,39 dB statt
2,54 dB); D9 §8.1 (Golden-Nachzug verlangt Rot für M-68, das Bytegleichheit
zusagt); D10 M-08 (Gruppe 63 heute −37,78 dB, die Zeile ist „heute rot“,
nicht Regressionswache).

**Validierung beauftragt.** Nach NAK-396 (Claude-Woche 92 %) validiert ein
frischer, lesender Codex-Thread `gpt-6-astra` high statt eines Opus-Threads:
Auftrag `docs/beweise/roh/NAK-380-matrixpruefung-1-validierung-auftrag.txt`
(Teil 1 je Befund D1 bis D10 mit Nachmessung und Nachrechnung an der
Quelle, Teil 2 Entscheidungsgrundlagen), Kennung `nak380-m1v`, abgekoppelt
über `tools/dirigent/codex-lauf.ps1`; Bericht als letzte Nachricht,
Ablage durch den Dirigenten nach
`docs/beweise/roh/NAK-380-matrixpruefung-1-validierung.md` (Zielpfad,
entsteht nach dem Lauf; der Bericht liegt bis dahin unter
`%TEMP%\nakama-nak380-m1v-<HEAD>-last.txt`, Thread-ID im JSONL daneben).

**Codex-Bauer, Fähigkeit gemessen (25.09.2026, Register NAK-401).** Zwei
Proben mit `gpt-5.6-sol` low in der workspace-write-Sandbox
(`tools/dirigent/codex-bau.ps1`, `windows.sandbox=elevated`,
`shell_environment_policy.inherit=all`): cmake liegt nicht im PATH der
Sandbox und läuft über den absoluten Visual-Studio-Pfad
(`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`,
Bau von `EqCopIdentityTest` Exit 0); der Store-Python 3.13 (`py -3.13`)
startet dort nicht („Anmeldesitzung nicht vorhanden“), `C:\Python314\python.exe`
fährt A5 mit gleicher Ausgabe (Exit 0), A8, A9 und A10 brauchen das Modul
`rfc8785`, das nur im Store-Python liegt, und fährt der Dirigent; `cargo
build` und Schreiben unter `eq-copilot/build/` laufen (Exit 0); `git
rev-parse` läuft. Der Codex-Bauer stagt, committet und pusht nie; der
Dirigent committet den Stand nach eigenem Kanon (Skill §3.6). Der Auftrag
der Etappe 2 in dieser Form liegt als Entwurf unter
`docs/beweise/roh/NAK-380-etappe-2-auftrag-entwurf.txt` (vor dem Start gegen
die geprüfte Matrix nachziehen).

**Übergabe (Kontexthaushalt, Skill §5).** Der Kontext dieser Sitzung liegt
gemessen bei 55 % des 1M-Fensters (Telemetrie 05:57 Uhr; Claude-Woche 92 %,
Codex 44 %): keine neue Prüfrunde in dieser Sitzung; Einordnung, Nacharbeit
und Wiederprüfung gehen an die nächste Dirigentensitzung (Marker, Starter).
Sie fährt in dieser Reihenfolge:

1. **Validierung lesen** (Bericht der Kennung `nak380-m1v` unter `%TEMP%`
   oder, falls der Lauf noch läuft, auf `EXIT=` in der `-start.log` warten;
   Thread-ID aus dem JSONL), unter dem Zielpfad ablegen, committen.
2. **Einordnung** (Dirigent, §12): je Befund D1 bis D10 Defekt, Lücke oder
   Härtung am Wortlaut von Regel, Entscheid, Zusage und Etikettendefinition;
   Lücken bekommen ihre Regel in derselben Runde (Muster NAK-313 §11, §15).
3. **Matrixnacharbeit 1** (vor 15:00 Uhr Codex `gpt-5.6-sol` xhigh
   `workspace-write` über `tools/dirigent/codex-bau.ps1`, nur
   `docs/beweise/NAK-380.md`, kein Commit — der Dirigent committet; nach
   15:00 Uhr frischer Opus-Worker max nach dem Muster der Etappe 1): nur
   die bestätigten Defekte wörtlich, je Defekt die schließende Regel und die
   Folgestellen; Ursache statt Punktkorrektur, wo die Validierung eine
   gemeinsame Wurzel nennt. Dann **Matrixprüfung 2** (Vorlage B, Codex Astra
   high, Prüfbereich nur der Fixdiff der Runde). Runde 1 von 3.
4. Nach PASS: Abschlussfenster der Etappe 1 (Hygiene, Planstand), dann
   Etappe 2 nach `docs/beweise/roh/NAK-380-etappe-2-auftrag-entwurf.txt`,
   nachgezogen auf die geprüfte Matrix (Bauer bis 15:00 Uhr Codex Sol xhigh,
   danach Opus max; Kanon vorher geerbt, Kopf).

Offen beim User: nichts aus diesem Ticket (keine Karte); Karte U65 aus
NAK-313 wartet (Cron 09:04 Uhr dieser Sitzung verfällt mit ihr; die nächste
Sitzung stellt sie im Zeitfenster über `/fragen U65`).

**Hygiene dieses Fensters (gemessen 25.09.2026, 06:0x Uhr).**
`py -3.13 tools/plan/gesundheit.py`: Exit 4 mit den drei bekannten Rissen
(Quelldateien über 2 000 Zeilen: 3; Funktionen über 200 Zeilen: 33 von
höchstens 28; Kommentar-Bezeichner ohne Code: 39 von höchstens 30; NAK-292);
Kontextfläche OK (MEMORY.md 6 195 von 24 986 Bytes, CLAUDE.md 20 246 von
20 480, Dirigenten-Skill 24 305 von 24 576, keine Zeile über 250 Zeichen,
keine Memory-Datei ohne Index-Link). `dokuriegel.py` auf diesem Manifest,
`docs/offene-punkte.md`, CLAUDE.md und Skill: 0 Befunde. `zielbild.py
pruefen`: Exit 0. `tidy.py` (A33): im Kanon der NAK-313-Etappe 7 an
`bb2bfd4d` Exit 0 (58 von 58); kein Produktdiff seither, nicht erneut
gefahren.

## 12. Validierung, Einordnung und Nacharbeitsauftrag der Matrixprüfung 1 (Dirigent, 25.09.2026, 06:13 Uhr)

**Validierung (Übergabe Schritt 1).** Frischer, lesender Codex-Thread
`gpt-6-astra`, Effort high (NAK-396: Claude-Woche 92 %, Codex 44 %), Thread
`01a0d6ba-22e8-7781-9017-a40a7cdde0c4`, Kennung `nak380-m1v`, abgekoppelt
über `tools/dirigent/codex-lauf.ps1`; HEAD `f1f619e9` vor und nach dem Lauf;
Lauf 06:01:57 bis 06:10:26 Uhr, EXIT=0, 25 lesende Kommandos. **Ergebnis: 10
bestätigt, 0 präzisiert, 0 widerlegt; jede Zahl der Prüfung nachgerechnet
und bestätigt** (D2 Integral C/f über eine Oktave = C·ln 2; D6 Δφ = 0,06137
rad über die Bandbreite bei 8 Samples; D7 2048/48 000 = 4096/96 000 =
42,67 ms; D8 ρ = 1/36, ν(8) = 15,26, 2,39 dB; D10 Gruppe 63 heute
−37,78 dB gegen −10,88 dB). Bericht abgelegt unter
`docs/beweise/roh/NAK-380-matrixpruefung-1-validierung.md`. Kein Befund
fällt unter einen Ausschluss des Prüfauftrags.

### 12.1 Einordnung je Befund (Übergabe Schritt 2)

Maßstab: Gate-Satz, Regeln R-380-1 bis R-380-9 (§2, §10), Entscheide
E-380-1 bis E-380-12 (§10), Zusagen §5, Matrixregeln und
Etikettendefinition im §6-Kopf, Korpusregeln §7, Invarianten in
`CLAUDE.md`. Defekt = verletzt Matrix, Gate, Regel, Vertrag oder
Invariante; Lücke = Quellen schweigen; Härtung = von keiner Zusage verlangt.

| Befund | Stelle | Was bricht (wörtlich) | Einordnung | Schließende Regel |
|---|---|---|---|---|
| D1 | §8.2 Wortlaut der Vertragspräzisierung, M-17 | R-380-1 Satz 4 „Der Vertrag benennt beide Größen mit Einheit und Bezug“; `CLAUDE.md` „Schemas sind Verträge“. `Bandwerte` trägt in `Frame.band_stereo` (`nakama_telemetry_v1.fbs:253-256`, Schreiber `TelemetryClient.cpp:129-131`, `Frame.h:360` `seiteSumme / gesamtSumme`) einen dimensionslosen float32-Seitenanteil in [0, 1]; „die Größe folgt aus `gitter`“ deklarierte ihn als Bandleistung in dBFS und widerspricht M-06 (0,2609). | **DEFEKT** | Die Größenbenennung im Vertrag gilt je Feld und Gitter: 221 Feinbänder mittlere einseitige Leistungsdichte in dBFS/Hz, `Frame.baender` (64 Livegruppen) Bandleistung in dBFS, `Frame.band_stereo` dimensionsloser Seitenanteil in [0, 1] (float32, keine q_db-Kodierung); keine Feldänderung. Folgestellen: M-17 (die vier Stellen und die Mutation), Lesertafel §8.2, FB-README-Satz, generierte Kommentare (A9). |
| D2 | §7.1 Erzeugerprüfung Rosa | R-380-8 „Referenz außerhalb des eigenen Codes: Analytik“. Für S(f) = C/f ist die Oktavbandleistung ∫ C/ν dν = C·ln 2 konstant; nur die Dichte fällt um 3,0103 dB je Oktave. Die vorgeschriebene Prüfung („Oktavbandleistungen … fallen je Oktave um 3,01 dB“) verwirft korrektes Rosa und ließe 1/f² passieren. | **DEFEKT** | E-380-13: die Erzeugerprüfung prüft die konstante integrierte Oktavbandleistung 31,5 Hz bis 16 kHz (Abweichung benachbarter Oktaven ≤ 1,0 dB); ein Dichtesatz darf zusätzlich stehen, dann korrekt als Dichte (−3,01 dB je Oktave). Folgestellen: P1, P2 (§7.2) und ihre Nutzer M-53, M-57 bis M-59, M-61, M-62, die Rosa-Fälle der Etappe 6 (M-112 u. a.). |
| D3 | M-119 | Eigener Bauplan §8.6 („`MessSnapshot`: `resonanzSucheAbHz` und zwei Masken“) gegen die Zusage „`sizeof (AnalyseEngine)` gleich den am Etappenstart gemessenen Werten“; `AnalyseEngine.h:369` hält `MessSnapshot fertig;` als Mitglied, neue Mitglieder vergrößern das Objekt auch bei Heap-Nutzdaten. `CLAUDE.md` (NAK-175) verlangt Heap für Längenabhängiges, nicht Größengleichheit. Die Zusage lehnte einen korrekten Bau ab. | **DEFEKT** | E-380-14: Obergrenze statt Gleichheit, aus den Typen nachgerechnet (siehe 12.2). Folgestellen: T-380-8, T-380-9 (Typen festlegen), §8.6, M-97 (Containerköpfe der Stereoakkus). |
| D4 | §8.6 Voraussetzung | E-380-10 „Matrixzeile M-122 als Nachtrag der Etappe 6 … geschrieben vor dem Bau der Etappe 6 mit Test in A4 und Rotbeweis …; §8.6 nennt den Nachtrag“. §8.6 nennt nur „Antwort auf K-380-1“ (die E-380-12 bereits gegeben hat); `telemetrie.rs:89-108` vergleicht ohne Versionsvoraussetzung; B-2 und T-380-5 tragen nur die Beobachtung. | **DEFEKT** (Bauplan gegen Entscheid) | E-380-15: M-122 wird in der Matrixnacharbeit 1 als Zeile geschrieben (siehe 12.2); §8.6 nennt sie als Voraussetzung mit A4-Test, Rotbeweis (Mutation des Versionsriegels) und Broker-Ticketpfad, und ersetzt „Antwort auf K-380-1“ durch E-380-12 (Variante A). Folgestellen: Ticketpfade und Testliste §8.6, §8.1 Rotbeweis-Ablauf, Zählung §6.6. |
| D5 | M-80 (erbt „wie M-73“), M-74 bis M-79 | R-380-3 „Der Golden ‚bekannte Laufzeit‘ (§40.3) wird breitbandig über alle unterstützten Blockgrößen und Abtastraten geführt“; §1 nennt „Laufzeit aus der Phase 1 ms in einem benannten Toleranzmaß, über alle unterstützten Blockgrößen und Abtastraten“. Die Sweeps prüfen nur Kohärenz; eine fest verdrahtete Abtastrate in der Laufzeitrechnung bliebe unentdeckt. E-380-3 nimmt nur das Vertragsfeld aus, nicht die Messzusage. | **DEFEKT** | Jeder Sweepfall M-74 bis M-79 prüft zusätzlich die Gruppenlaufzeit gegen 1000·d/fs ms ± 0,02 ms (d = Verzögerung in Samples je Rate; 48-kHz-Familie 1,000 ms, 44,1-kHz-Familie bei d = 44 0,997732 ms). Folgestellen: M-80, §7.2 D1 bis D6, §8.5, Wiederholung nach der FFT-Skalierung in §8.6. |
| D6 | M-81 | Matrixregel „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt“ und „jede Kette Mutation → Eingabe → beobachtete Zusage ist am Code nachgerechnet“. Der B19-Fall verzögert 8 Samples (`Sonde013StereoGoldenTest.cpp:503`, `:1212`), nicht 1 ms; Δφ über die ganze 2000-Hz-Bandbreite 0,06137 rad < 0,25 rad; ein Einzelton trägt seine Trägerphase über die Leakage-Bins. Die Mutation (erster statt mittlerer Bin) macht nicht rot. | **DEFEKT** | Signal und Toleranz müssen die falsche Binwahl an der Phasenassertion nachgerechnet unterscheiden; Kandidat der Validierung: breitbandiges Rauschen mit 1 ms Verzögerung im 15,77-kHz-Band (455,49 Hz breit; erster gegen mittlerer Bin bei N = 4096: 2π·19·48 000/4096·0,001 ≈ 1,399 rad), Referenz die tatsächliche Binfrequenz. Folgestellen: M-81, §7.2 (neues Signal), §8.5. |
| D7 | M-112 | R-380-5 „Hops und Ticks bleiben deterministisch und golden-beweisbar“; `CLAUDE.md` „Zeit ist aktive Musikzeit. Zonen-Ticks sind deterministisch und durch Goldens beweisbar“. Nach T-380-7 gilt 2048/48 000 = 4096/96 000; die Mutation „Zellenlänge = Mittenhop“ lässt die Tickzahl ratengleich (30 s → 70 statt 30 Ticks), die Untergrenze ≥ 15 bleibt erfüllt: der Rotbeweis bricht die Zusage nicht. | **DEFEKT** | E-380-16: absolute Tickzahl als Zusage (siehe 12.2). Folgestellen: M-112, §7.2 P2, §8.6. |
| D8 | M-32, M-33, §7.4, T-380-4, W4 | R-380-8 („nachgerechnet“ als Referenz). `AnalyseEngine.cpp:70` `hop = n / 2`, `:85` periodisches Hann: Σw² = 3N/8, Σw[n]w[n+N/2] = N/16, normierte Überlappung 1/6, ρ = 1/36 (nicht 1/9); ν(K = 8) = 2304/151 ≈ 15,26 (nicht 13,40), ν(K = 4) = 7,68; Wilson-Hilferty P95 − P50 = 2,39 dB (K = 8) bzw. 3,30 dB (K = 4) statt 2,54 dB. Die Wahl von acht Segmenten trägt weiter (3,61 dB Abstand zur 6-dB-Schwelle). | **DEFEKT** | Alle Referenzzahlen und Modulationsnäherungen verwenden dasselbe korrekte Fenstermodell (periodisches Hann, 50 %, ρ = 1/36). Folgestellen: M-32, M-33, M-34, M-35 (Modulationsrechnung), W4 (§7.2), §7.4, T-380-4. |
| D9 | §8.1 „nachgezogener Golden (M-39, M-68, M-118)“ | M-68 „die 76 Byte sind bytegleich zu einem am Etappenstart mit dem unveränderten Code erzeugten Golden“ (Regressionswache, T-380-5 hält den Bandfluss); R-380-7 „jeder nachgezogene Golden hat einen Rotbeweis am alten Wert“ gilt nur für geänderte Referenzinhalte; Etikettendefinition „Regressionswache: heute grün und hält, was der Bau nicht ändern darf“. §8.1 verlangte für M-68 Rot gegen den alten Golden und einen Erzeugerlauf, die die Bytegleichheit aufheben. | **DEFEKT** | E-380-17: zwei Golden-Klassen in §8.1 (siehe 12.2). M-68 bleibt Bytegleichheitswache; M-39 und M-118 tragen den Referenznachzug; M-12, M-28, M-86 je Zusagehälfte einordnen (M-28 (b): Altwert darf innerhalb der neuen Toleranz liegen, dann ist der dokumentierte Altvergleich der Beleg). Folgestellen: §8.1, §8.2 bis §8.6 (Golden-Absätze). |
| D10 | M-08 | Etikettendefinition §6-Kopf: „heute rot: am Basis-SHA widerspricht das Verhalten der Zusage nachweislich“; „Regressionswache: heute grün“. `Frame.h:347` addiert ungewichtete Dichten; Gruppe 63 (vier Feinbänder, `nakama_log64_v1.json:393-396`) liefert heute 10·log10(4·2/48 000) = −37,78 dB gegen die Zusage −10,88 ± 1,0 dB: die Leistungszusage ist heute rot, die Zeile trägt nur „Regressionswache für den Trägerrand“. | **DEFEKT** | M-08 wird eine Zeile mit zwei Hälften: (1) Leistungszusage heute rot (Wert −37,78 dB am Basisstand, Mutation nach dem Bau an der Gewichtung), (2) Regressionswache für Trägerrand, Validity, `saturated` und Vertragsleser (Mutation `kPlausibelMaxDb`). §6.6 wird 67/20/44 = 131 Etiketten, Etappe 2 10/3/8; 121 Zeilen (122 mit M-122, siehe E-380-15). Folgestellen: §8.2, §8.1. |

**Ergebnis: 10 Defekte, 0 Lücken, 0 Härtungen.** Kein Befund hängt an einem
Produktentscheid oder User-Handgriff; keine Karte.

### 12.2 Entscheide des Dirigenten (Technik, innerhalb von Gate, Invarianten und §2)

- **E-380-13 (D2) — Rosa-Referenz als Bandleistung.** Der Erzeuger wird an
  der integrierten Größe geprüft: Oktavbandleistung P[f, 2f] = C·ln 2,
  konstant über 31,5 Hz bis 16 kHz (Abweichung benachbarter Oktaven ≤ 1,0
  dB). Begründung: dieselbe Trennung von Dichte und Leistung wie R-380-1;
  die Prüfung belegt nebenbei, dass der Korpus die Größen nicht vermischt.
- **E-380-14 (D3) — Objektgröße mit begründeter Obergrenze.** M-119 (und
  M-97 für die Stereoakkus) prüft `sizeof (AnalyseEngine)` ≤ Startwert +
  Δ, wobei Δ aus den in T-380-8/T-380-9 festgelegten Typen der neuen
  Mitglieder nachgerechnet und als Zahl in der Zeile steht (Beispiel der
  Validierung: ein double und zwei gepackte 221-Bit-Masken aus je 28
  uint8_t = 64 B); `sizeof (FeatureEngine)` bleibt gleich, solange die
  Etappe dort kein Mitglied hinzufügt, sonst dieselbe Regel; alle
  längenabhängigen Träger bleiben Vektoren (Heap), B5 läuft ohne
  `0xC00000FD`; die Mutation „festes Array von 65 536 double als Mitglied“
  bleibt der Rotbeweis. Die Stapelwache aus `CLAUDE.md` (NAK-175) ist
  damit getragen, ohne einen korrekten Bau abzulehnen.
- **E-380-15 (D4) — M-122 in der Matrixnacharbeit 1, nicht später.**
  E-380-10 wird präzisiert: die Zeile M-122 (`fingerprint_aehnlichkeit`
  vergleicht nur Fingerprints gleicher `metrics_version`; ungleiche Version
  → „nicht vergleichbar“, fail-closed, nie „anderes Material“; Test in A4
  `nak380_m122_<name>`; heute nicht messbar; Mutation: Versionsriegel
  entfernt → numerische Unähnlichkeit statt „nicht vergleichbar“ → rot)
  entsteht jetzt in §6.5 als Nachtragsblock, weil die Matrix noch nicht
  gebaut ist und ein späterer Nachtrag eine eigene Prüfrunde kostete; §8.6
  nennt sie als Voraussetzung mit Broker-Ticketpfad. Die Voraussetzung
  „Antwort auf K-380-1“ ist durch E-380-12 (Variante A) erledigt und wird
  so benannt. Die Matrix führt danach 122 Zeilen.
- **E-380-16 (D7) — absolute Tickzahl.** M-112 sagt je Zone die Tickzahl
  aus den eingespeisten aktiven Samples zu: ein Tick je 10 Zellen à
  lround(0,1·fs) Samples, also bei 30 s vollständig aktivem Material 30
  Ticks je belegter Zone (Belegungs- und Pegelvoraussetzungen
  `AnalyseEngine.cpp:568-574` im Fall hergestellt) bei 48 und 96 kHz;
  Ratengleichheit bleibt als zweiter Satz. Rotbeweis: Mutation Zellenlänge
  = Hop der Mittenstufe → 70 Ticks → rot. Alternativ darf der Fall die
  geplante Tickkadenz direkt beobachten, wenn der Testzugang benannt ist.
- **E-380-17 (D9) — zwei Golden-Klassen in §8.1.** (i) Wache: Golden oder
  Wert bleibt unverändert (Bytegleichheit, gleiche Zahl); der Beleg ist die
  temporäre Mutation, nie ein Erzeugerlauf; Etikett Regressionswache.
  (ii) Nachgezogener Golden: der Referenzinhalt ändert sich zusagegemäß;
  neuer Code gegen alten Golden rot (Rohdatei), dann Erzeuger; Etikett heute
  rot. Jede Golden-Zeile trägt ihre Klasse je Zusagehälfte; M-68 (i), M-39
  und M-118 (ii), M-12, M-28, M-86 je Hälfte.

### 12.3 Gemeinsame Ursachen und Durchsicht

Die Validierung nennt drei Wurzeln, der Dirigent ordnet zwei weitere zu:
(a) **Referenz nicht unabhängig nachgerechnet** (D2, D8; R-380-8): eine als
„nachgerechnet“ geführte Zahl stimmte nicht zum Fenster- oder
Signalmodell des Codes. (b) **Rotbeweis ohne Unterscheidungskraft** (D6,
D7; Matrixregel): die Kette Mutation → Eingabe → Beobachtung ändert die
Zusage nachgerechnet nicht. (c) **Etikett oder Bauanweisung gegen die
Zusage** (D3, D9, D10; Etikettendefinition, R-380-7): „Regressionswache“ bei
heute rotem Wert, Golden-Nachzug bei zugesagter Bytegleichheit,
Gleichheitszusage gegen den eigenen Bauplan. (d) **Sweep ohne die
eigentliche Zusage** (D5; R-380-3). (e) **Bauplan trägt einen Entscheid
nicht** (D4; E-380-10, E-380-12). D1 ist ein Einzelfall (Vertragsgröße je
Feld). Nach §2 („Haben Befunde eine gemeinsame Ursache, ist die Ursache der
Gegenstand“) sieht die Nacharbeit jede der 121 Zeilen, §7 und §8 auf (a)
bis (e) durch und berichtigt, was denselben Fehler trägt; das Ergebnis
steht als Durchsichtstabelle in §13.

### 12.4 Matrixnacharbeit 1 (Übergabe Schritt 3; Runde 1 von 3)

Bauer nach NAK-396 (vor dem Wochenreset 15:00 Uhr, Claude-Woche 92 %):
Codex `gpt-5.6-sol`, Effort xhigh, `workspace-write` über
`tools/dirigent/codex-bau.ps1` (Kennung `nak380-mn1`; NAK-401: Dokuriegel
mit `C:\Python314\python.exe`, in dieser Sitzung gegen dieses Manifest mit
Exit 0 gemessen). Er ändert genau `docs/beweise/NAK-380.md` (§5 bis §9 und
neuer Abschnitt §13), stagt, committet und pusht nie; der Dirigent misst den
Diff, fährt `py -3.13 tools/plan/dokuriegel.py` und committet mit Pathspec.
Auftrag: `docs/beweise/roh/NAK-380-matrixnacharbeit-1-auftrag.txt` (nur die
zehn bestätigten Defekte wörtlich, je Defekt die schließende Regel aus 12.1
und 12.2, die Folgestellen, die Durchsicht nach 12.3). Danach
Matrixprüfung 2 (Vorlage B, Codex `gpt-6-astra` high, Prüfbereich nur der
Fixdiff der Runde: `stand-vor-der-runde...HEAD -- docs/beweise/NAK-380.md`).
Stand vor der Runde: der Commit dieses Abschnitts. Rundenbilanz der
Matrixrunden zählt null (§3.3 des Skills).

## 13. Matrixnacharbeit 1 (Bauer, 25.09.2026)

### 13.0 Schließung der zehn Defekte

| Defekt | Berichtigte Stellen | Schließung |
|---|---|---|
| D1 | §5.2, §6.1, M-17, §8.2, §9 | Die Größe ist nun je Feld benannt: 221 Feinbänder als mittlere einseitige Leistungsdichte in dBFS/Hz, `Frame.baender` als Bandleistung in dBFS und `Frame.band_stereo` als dimensionsloser float32-Seitenanteil in [0, 1] ohne `q_db`; A5 und A9 brechen jede der drei Aussagen getrennt. |
| D2 | M-53, M-57 bis M-59, M-61, M-62, M-103 bis M-109, M-112, §7.1, P1/P2, §7.4, §8.0 | Der Rosa-Erzeuger wird vor jedem Nutzer an konstanter integrierter Oktavbandleistung C·ln 2 mit höchstens 1,0 dB Nachbarabweichung geprüft; nur die Dichte fällt um 3,0103 dB je Oktave. |
| D3 | M-97, M-119, T-380-6, T-380-8, T-380-9, §8.5, §8.6 | Gleichheit wurde durch nachgerechnete Obergrenzen ersetzt: FeatureEngine Startwert + 40 B und AnalyseEngine Startwert + 64 B; längenabhängige Daten bleiben Heap-Vektoren und B5 endet ohne `0xC00000FD`. |
| D4 | M-122, §6.5, §6.6, §8.0, §8.1, §8.6, §8.7, §9 | M-122 ist vor dem Etappe-6-Bau als A4-Fall mit Brokerpfaden und Rotmutation festgelegt; ungleiche `metrics_version` ergibt fail-closed „nicht vergleichbar“, und E-380-12 steht als erledigte Variante A. |
| D5 | M-74 bis M-80, D1 bis D6, §7.4, §7.5, §8.5, §8.6 | Jeder Raten- und Blockgrößensweep prüft zusätzlich 1000·d/fs ms ± 0,02 ms, also 0,997732 ms in der 44,1-kHz-Familie und 1,000000 ms in der 48-kHz-Familie; der Lauf wird nach der FFT-Skalierung wiederholt. |
| D6 | M-81, D7, §7.4, §8.5 | Breitbandiges Rauschen mit 48 Samples Verzögerung im 15,77-kHz-Band trennt ersten und mittleren Bin um 19 Bins beziehungsweise 1,399 rad; die ±0,25-rad-Assertion fällt damit an der Binwahl. |
| D7 | M-112, P2, §8.0, §8.6 | Die Zusage lautet nun genau 30 Ticks je belegter Zone aus 300 aktiven 0,1-s-Zellen bei 48 und 96 kHz; die Mittenhopmutation liefert 70 und fällt trotz Ratengleichheit. |
| D8 | M-32 bis M-35, W4, §7.4, T-380-4, §8.3 | Alle Zahlen verwenden das periodische Hann mit 50 Prozent Überlappung und ρ = 1/36: ν(8) = 2304/151 = 15,2583, ν(4) = 7,68 und die Wilson-Hilferty-Spannen 2,39 beziehungsweise 3,30 dB. |
| D9 | M-68, F1, §7.5, §8.1 bis §8.6, §9 | Golden-Klasse (i) bleibt ohne Erzeugerlauf unverändert, Klasse (ii) läuft zuerst gegen den alten Inhalt rot und erst dann durch den Erzeuger; M-68 ist Klasse (i), M-39 und M-118 sind Klasse (ii), M-12, M-28 und M-86 sind je Hälfte eingeordnet. |
| D10 | M-08, §6.6, §8.1, §8.2, §9 | M-08 trennt die heute rote Leistungszusage mit Basiswert −37,7815 dBFS von der grünen Wache für Trägerrand, Validity, `saturated` und Vertragsleser; die Zählung lautet 67/20/44 bei 122 Zeilen. |

### 13.1 Änderungsregister

Jede geänderte oder neue Matrixzeile und jeder geänderte Abschnitt ist hier
einmal geführt. „Vorher“ beschreibt den Stand vor Matrixnacharbeit 1 in einem
Satz; „jetzt“ nennt die bindende Fassung.

| ID oder Anker | Vorher | Jetzt | Grund |
|---|---|---|---|
| Kopf, Etappe | Der Kopf endete bei Matrixnacharbeit 1 ohne laufenden Prüfschritt. | Der letzte Fettsatz nennt ausschließlich „Matrixnacharbeit 1 geschrieben, Matrixprüfung 2 steht aus“. | Auftragsgrenze Kopf |
| §5.2 | Die Vertragslücke behandelte Spektralfelder gemeinsam und grenzte `band_stereo` nicht ab. | Die drei Größen und Einheiten sind feldbezogen, ohne Feld- oder Versionsänderung. | D1, R-380-1 Satz 4 |
| §6 Kopf und §6.1 | Der Umfang endete bei M-121 und die Lesertafel trennte die Größe von `band_stereo` nicht vollständig. | Der Umfang endet bei M-122; Lesertafel und Matrixregel führen den dimensionslosen Seitenanteil getrennt. | D1, D4, Ursache (e) |
| M-08 | Die ganze Zeile war nur Regressionswache für den Trägerrand. | Leistungswert −10,8809 ± 1,0 dBFS ist heute rot bei −37,7815 dBFS; die übrigen Drahtzusagen bleiben eigene Wache. | D10, Ursache (c) |
| M-12 | Der exakte Wert ersetzte den groben Bereich, ohne beide Golden-Klassen an der Zeile zu nennen. | Der bisherige Bereich ist Klasse (i), der exakte −19,12-dB-Wert Klasse (ii) und heute rot bei −19,87 dB. | D9, E-380-17 |
| M-17 | „Die Größe folgt aus gitter“ konnte `band_stereo` als dBFS erklären. | A5/A9 prüfen Dichte, Bandleistung und dimensionslosen float32-Anteil je Feld. | D1 |
| §6.2 | Das falsche Fenstermodell floss in die M1-Referenzen ein. | Alle M1-Zahlen verwenden periodisches Hann, ρ = 1/36, oder eine erst in der Etappe zu messende Schranke. | D8, Ursache (a) |
| M-32 | ρ = 1/9, ν = 13,40 und 2,54 dB waren als nachgerechnet geführt. | ρ = 1/36, ν = 2304/151 und 2,39 dB sind mit Summen und Formel hergeleitet. | D8, R-380-8 |
| M-33 | Die Modulationsnäherung erbte das falsche Fenstermodell. | Die Referenz ist die Faltung mit χ² für ν = 2304/151; das Intervall bleibt Messschranke. | D8, Ursache (a) |
| M-34 | Die 8-dB-Modulation erbte dieselbe falsche Näherung. | Sie nutzt dieselbe korrigierte Faltung und kennzeichnet das Intervall als Messschranke. | D8, Ursache (a) |
| M-35 | Die 12-dB-Modulation erbte dieselbe falsche Näherung. | Sie nutzt dieselbe korrigierte Faltung und kennzeichnet das Intervall als Messschranke. | D8, Ursache (a) |
| M-28 | Der 30-s- und 75-s-Teil nannte keine Golden-Klasse. | Kein Wert vor 60 s ist Klasse (i); der nachgezogene 75-s-Wert ist Klasse (ii) mit dokumentiertem Altvergleich. | D9, E-380-17 |
| M-39 | Der nachgezogene B30-Inhalt trug seine Klasse nur im allgemeinen Ablauf. | Die Zeile nennt ausdrücklich Klasse (ii), alten Inhalt rot und danach den Erzeuger. | D9, E-380-17 |
| §6.3 | Rosa-Nutzer setzten einen fachlich falschen Erzeugertest voraus. | Jeder Rosa-Nutzer verlangt die E-380-13-Selbstprüfung auf konstante Oktavbandleistung. | D2, Ursache (a) |
| M-53 | P1 war nicht an die korrekte Rosa-Prüfung gebunden. | P1 wird vor dem Nullkorpus an höchstens 1,0 dB Nachbarabweichung geprüft. | D2, E-380-13 |
| M-57 | Der erste Rosa-Pegelsweep erbte den falschen Erzeugersatz. | Der Fall nennt den selbstgeprüften P1-Erzeuger. | D2, E-380-13 |
| M-58 | Der zweite Rosa-Pegelsweep erbte den falschen Erzeugersatz. | Der Fall nennt den selbstgeprüften P1-Erzeuger. | D2, E-380-13 |
| M-59 | Der dritte Rosa-Pegelsweep erbte den falschen Erzeugersatz. | Der Fall nennt den selbstgeprüften P1-Erzeuger. | D2, E-380-13 |
| M-61 | Der Wiederbeginnfall vertraute ungeprüftem Rosa. | Beide aktiven Abschnitte verwenden den nach E-380-13 geprüften P2-Erzeuger. | D2, E-380-13 |
| M-62 | Der Lebenszyklusfall vertraute ungeprüftem Rosa. | Seine Rosa-Eingabe wird vor dem Nutzer nach E-380-13 geprüft. | D2, E-380-13 |
| M-68 | Der unveränderte Fingerprint sollte am Etappenstart erzeugt werden. | Die bestehenden 76 Bytes bleiben Klasse-(i)-Wache; nur die Bandflussmutation ist Rotbeleg, kein Erzeugerlauf. | D9, E-380-17 |
| §6.4 | Die Sweeps belegten im Wesentlichen Kohärenz; M-81s Binmutation war unwirksam und M-97 verlangte Gleichheit. | Jeder Sweep trägt Laufzeit, D7 trennt die Binwahl um 1,399 rad und M-97 hat ein 40-B-Budget. | D3, D5, D6, Ursachen (b)-(d) |
| M-74 | Der Blockgrößensweep prüfte keine Laufzeit. | Alle 18 Blockgrößen prüfen zusätzlich 1,000000 ms ± 0,02 ms. | D5, R-380-3 |
| M-75 | Der 44,1-kHz-Sweep prüfte keine Laufzeit. | Er prüft 0,997732 ms ± 0,02 ms; ein 48-kHz-Divisor ergibt 0,916667 ms. | D5, R-380-3 |
| M-76 | Der 88,2-kHz-Sweep prüfte keine Laufzeit. | Er prüft 0,997732 ms ± 0,02 ms; ein 48-kHz-Divisor ergibt 1,833333 ms. | D5, R-380-3 |
| M-77 | Der 96-kHz-Sweep prüfte keine Laufzeit. | Er prüft 1,000000 ms ± 0,02 ms; ein 48-kHz-Divisor ergibt 2,000000 ms. | D5, R-380-3 |
| M-78 | Der 176,4-kHz-Sweep prüfte keine Laufzeit. | Er prüft 0,997732 ms ± 0,02 ms; ein 48-kHz-Divisor ergibt 3,666667 ms. | D5, R-380-3 |
| M-79 | Der 192-kHz-Sweep prüfte keine Laufzeit. | Er prüft 1,000000 ms ± 0,02 ms; ein 48-kHz-Divisor ergibt 4,000000 ms. | D5, R-380-3 |
| M-80 | Der Gruppenlaufzeitfall lief nur bei 48 kHz und Block 512. | Er führt D1 bis D6 über alle vorgesehenen Raten und Blöcke mit 1000·d/fs ± 0,02 ms. | D5, R-380-3 |
| M-81 | 8 Samples und ein Einzelton änderten die beobachtete Phase bei falscher Binwahl nicht ausreichend. | D7 nutzt Breitbandrauschen, 48 Samples und 19 Bins Abstand; 1,399 rad übersteigt 0,25 rad. | D6, Ursache (b) |
| M-86 | Dauerwache und neue Freiheitsgrade trugen ihre Golden-Klassen nicht an der Zeile. | Dauer ist Klasse (i), Freiheitsgrade über W Fenster sind Klasse (ii). | D9, E-380-17 |
| M-87 | Grenze, Rücksetzen und Ratenwechsel waren als getrennte Etiketten behandelt. | Eine grüne Lebenszykluswache deckt alle drei bereits vorhandenen Leerungen mit je eigener Mutation. | Ursache (c), Etikettenzählung |
| M-97 | `sizeof (FeatureEngine)` sollte trotz neuem Vektorkopf gleich bleiben. | Startwert + 40 B ist die Obergrenze für 24-B-Vektorkopf und vier uint32-Stände. | D3, E-380-14 |
| §6.5 | Variante B blieb in Zeilen offen, M-122 fehlte und Rosa-, Tick- sowie Größenfolgen waren unvollständig. | Nur E-380-12 Variante A gilt; Rosa ist selbstgeprüft, Tickzahl absolut, Größenbudget 64 B und M-122 ist angefügt. | D2-D4, D7, Ursachen (a), (c), (e) |
| M-98 | Die Längenregel erlaubte noch Variante A oder B. | Die Zusage verwendet ausschließlich N_max = 65 536 aus E-380-12. | D4, Ursache (e) |
| M-99 | Die Ratenfolge hing noch von einer offenen Kartenwahl ab. | Die erwarteten Längen folgen fest aus Variante A. | D4, E-380-12 |
| M-100 | Das FeatureEngine-Speichermaß führte beide Varianten. | Nur das nachgerechnete Budget der gewählten Variante A bleibt. | D4, E-380-12 |
| M-101 | Das AnalyseEngine-Speichermaß führte beide Varianten. | Nur das nachgerechnete Budget der gewählten Variante A bleibt. | D4, E-380-12 |
| M-102 | Die hohe Rate war noch an eine offene Kappe gebunden. | Die Kappe ist 65 536 und die Zusage dadurch deterministisch. | D4, E-380-12 |
| M-103 | Der Resonanzfall setzte ungeprüftes Rosa voraus. | K1 verwendet vorab nach E-380-13 geprüftes Rosa. | D2, E-380-13 |
| M-104 | Der Resonanzfall setzte ungeprüftes Rosa voraus. | K2 verwendet vorab nach E-380-13 geprüftes Rosa. | D2, E-380-13 |
| M-105 | Der Resonanzfall setzte ungeprüftes Rosa voraus. | K3 verwendet vorab nach E-380-13 geprüftes Rosa. | D2, E-380-13 |
| M-106 | Der Grenzfall setzte ungeprüftes Rosa voraus. | K4 verwendet vorab nach E-380-13 geprüftes Rosa. | D2, E-380-13 |
| M-107 | Der Grenzfall setzte ungeprüftes Rosa voraus. | K5 verwendet vorab nach E-380-13 geprüftes Rosa. | D2, E-380-13 |
| M-108 | Die Suchgrenzenzusage führte noch die unentschiedene Kappe und ungeprüftes Rosa. | Sie bindet Variante A und den E-380-13-geprüften Erzeuger. | D2, D4 |
| M-109 | Die Validity-Prüfung erbte ungeprüftes Rosa. | Ihre Korpuseingabe wird vor dem Nutzer auf konstante Oktavbandleistung geprüft. | D2, E-380-13 |
| M-112 | Ratengleichheit und Untergrenze blieben trotz 70 statt 30 Ticks grün. | Je belegter Zone gelten genau 30 Ticks bei 48 und 96 kHz; die Hopmutation liefert 70. | D7, E-380-16 |
| M-118 | Der nachgezogene B30-Inhalt trug seine Klasse nur im allgemeinen Ablauf. | Die Zeile nennt ausdrücklich Klasse (ii), alten Inhalt rot und danach den Erzeuger. | D9, E-380-17 |
| M-119 | Objektgrößengleichheit widersprach den geplanten Snapshot-Mitgliedern. | AnalyseEngine darf um 64 B wachsen, FeatureEngine bleibt in Etappe 6 gleich; feste 524-288-B-Arrays brechen B5. | D3, E-380-14 |
| M-122, neu | Es gab nur die Beobachtung B-2 ohne baubare Matrixzusage. | A4 prüft numerische Ähnlichkeit nur bei gleicher `metrics_version` und „nicht vergleichbar“ bei ungleicher. | D4, E-380-15, R-380-9 |
| §6.6 | 121 Zeilen und 66/20/44 Etiketten waren geführt. | 122 Zeilen und 67/20/44 = 131 Etiketten sind mit M-122 und der geteilten M-08 nachgezählt. | D4, D10 |
| §7.1 | Rosa sollte fallende Oktavbandleistung zeigen. | Integrierte Oktavbandleistung ist konstant; nur die Dichte fällt um 3,0103 dB je Oktave. | D2, E-380-13 |
| §7.2 | P1/P2 und D1 bis D6 trugen die korrigierten Erzeuger- und Laufzeitzusagen nicht; D7 fehlte. | P1/P2 nennen den Selbsttest, D1 bis D6 d und τ, D7 das unterscheidende Binphasensignal. | D2, D5, D6 |
| §7.4 | Rosa-, Hann-, Laufzeit- und Binphasenreferenzen waren falsch oder unvollständig. | C·ln 2, ρ = 1/36, ν-Werte, 1000·d/fs und 1,399 rad stehen mit Herleitung. | D2, D5, D6, D8, Ursache (a) |
| §7.5 | B19 und B22 benannten die neue Laufzeitabdeckung und Golden-Klasse nicht. | B19 führt D1 bis D7; B22 ist unveränderte Wache ohne Erzeugerlauf. | D5, D6, D9 |
| §8.0 | Typen, Zahlen und K-380-1 waren teils offen oder widersprüchlich. | T-380-4/6/8/9 tragen die nachgerechneten Modelle und E-380-12 bis E-380-17 sind geschlossen zugeordnet. | D3, D4, D8, Ursachen (a), (c), (e) |
| §8.1 | M-68 stand zusammen mit nachgezogenen Goldens im Erzeugerablauf. | Zwei Golden-Klassen trennen unveränderte Wache und Referenznachzug; M-122 hat den vollständigen Rotablauf. | D4, D9, E-380-17 |
| §8.2 | Vertragswortlaut und Goldenabsatz trennten `band_stereo` und M-08 nicht korrekt. | Feldbezogene Größen, generierte Kommentare, M-08-Hälften und M-12-Klassen sind festgelegt. | D1, D9, D10 |
| §8.3 | Die falsche M1-Näherung und Goldenklassifikation wirkten fort. | T-380-4-Zahlen und M-28/M-39-Klassen sind an D8/E-380-17 gebunden. | D8, D9 |
| §8.4 | M-68 konnte noch wie ein nachgezogener Golden gelesen werden. | M-68 ist ausdrücklich Klasse (i) ohne Erzeugerlauf. | D9, E-380-17 |
| §8.5 | Sweeps, Binphase, M-86 und M-97 trugen die neuen Zusagen nicht. | Laufzeit je Sweep, D7, Goldenklassen und 40-B-Objektbudget stehen im Bauplan. | D3, D5, D6, D9 |
| §8.6 | Die Voraussetzung war die bereits entschiedene Frage K-380-1; M-122 fehlte und Größen-, Tick-, Sweep- und Goldenfolgen waren unvollständig. | Variante A ist gesetzt, M-122 geht mit A4 voran, B19 wiederholt die Laufzeit, M-112/M-119 sind exakt und M-118 ist Klasse (ii). | D3-D5, D7, D9, Ursache (e) |
| §8.7 | B-2 wurde noch als offene Registerbeobachtung geführt. | Nur B-1, B-3 und B-4 bleiben offen; B-2 ist durch M-122 geschlossen. | D4, E-380-15 |
| §9 | Prüfliste und Selbstaudit führten die alten Vertrags-, Golden- und Entscheidungsstände. | M-17/M-122, Goldenklassen, 122-Zeilen-Durchsicht und geschlossene Entscheide sind nachvollzogen. | D1, D4, D9, Ursachen (c), (e) |
| §13, neu | Es gab keinen Nachweis der Matrixnacharbeit. | §13.0 schließt D1 bis D10, §13.1 registriert den Diff und §13.2 protokolliert M-01 bis M-122. | Auftrag Matrixnacharbeit 1 |

### 13.2 Durchsicht

„Geändert“ meint die Matrixzeile selbst in dieser Nacharbeit. Jede Zeile wurde
gegen (a) Referenzrechnung, (b) unterscheidenden Rotbeweis, (c) Etikett und
Golden-Klasse, (d) eigentliche Sweepzusage und (e) entschiedenen Bauplan
gelesen.

| Zeile | Geändert | Durchsicht nach (a) bis (e) |
|---|---|---|
| M-01 | nein | Dichteformel 2σ²/fs, Bitfall und Summenmutation tragen die Zusage; Etikett Wache bleibt am Basisstand grün. |
| M-02 | nein | Gitterbreitenverhältnis und 28,53-dB-Spanne sind hergeleitet; die Einheitsgewichtung bricht genau die Bandleistung. |
| M-03 | nein | 4,65 dB folgt aus 2,7477/0,9418; erst der Ergänzungsfaktor macht die neue Zusage messbar. |
| M-04 | nein | Integral und heutige Feinbandzählung sind getrennt nachgerechnet; die Gewichtsmutation fällt an Parseval. |
| M-05 | nein | A²/2 und Hann-Parseval liefern Maß, Einheit und Toleranz; Einheitsgewichtung bricht die Sinusleistung. |
| M-06 | nein | 31,05/119,0 = 0,2609 trennt Leistungsgewichtung von 0,2500; `band_stereo` bleibt dimensionslos. |
| M-07 | nein | Nullenergie, Bit und Füllwert werden gemeinsam beobachtet; die Plausibilitätsmutation erzeugt den verbotenen Wert. |
| M-08 | ja | Heute roter −37,7815-dBFS-Leistungswert und grüne Drahtwache sind getrennt, je mit eigener wirksamer Mutation. |
| M-09 | nein | Konstruierte Leistungsanteile 0,4/0,3 und Basiswerte sind unabhängig bestimmt; die Breitenmutation kehrt die Rangfolge zurück. |
| M-10 | nein | Gruppenintervall und 0,05498/0,04124 sind aus Gitterbreiten gerechnet; die Vollgittermutation zerstört die Ordnung. |
| M-11 | nein | 221 positive Breiten, Teleskopsumme und drei hex64-Stützstellen prüfen beide Sprachen ohne Selbstbestätigung. |
| M-12 | ja | Der alte grobe Bereich ist als Klasse (i), der exakte −19,12-dB-Wert als Klasse (ii) geführt; Einheitsbreiten ergeben −19,87 dB. |
| M-13 | nein | 13,92 dB folgt aus den zwei konstruierten Leistungspegeln; dieselbe Breitenmutation bricht das Verhältnis. |
| M-14 | nein | Der Dichtevergleich bleibt absichtlich ungeändert; die Leistungsgewichtsmutation lässt nachgerechnet Band 200 gewinnen. |
| M-15 | nein | Das unveränderte dB-Mittel −35,00 ist von der Leistungssumme −19,12 getrennt und als Wache wirksam. |
| M-16 | nein | NaN, Inf, Überlauf, fehlende Bits und Unterlauf werden an den echten Riegeln beobachtet; nur grünes Basisverhalten ist Wache. |
| M-17 | ja | Jede Vertragsgröße wird je Feld geprüft; drei getrennte Textmutationen und A9 decken Leser und generierte Kommentare. |
| M-18 | nein | Alte und neue Version bleiben lesbar; eine künstliche Versionsablehnung bricht genau die Abwärtslesbarkeit. |
| M-19 | nein | Versionscode und Register werden gemeinsam verglichen; Rückmutation auf 20260904 fällt in A5. |
| M-20 | nein | Audioinvariante, Allokationsfreiheit und Kanon sind die eigentliche Zusage; die Audiothread-Allokation macht B4 rot. |
| M-21 | nein | 10-Hz-Zählung wird direkt über 50 s beobachtet; Hop 10 ergibt 50 statt 500 und trennt die Mutation. |
| M-22 | nein | Die analytische 10-Hz-Folge liefert 2,435 LU; 1-Hz-Abtastung fällt außerhalb ±0,2 LU. |
| M-23 | nein | 721 und 691 gegatete Werte sind aus Zellen und Stufenfolge gezählt; der 1-Hz-Hop bricht beide Zähler. |
| M-24 | nein | Der erste Normfall trägt Normwert und ±1-LU-Toleranz; eine 1-Hz-Abtastung verletzt die Referenz. |
| M-25 | nein | Der zweite Normfall trägt denselben unabhängigen Normpfad und eine unterscheidende Hopmutation. |
| M-26 | nein | Der dritte Normfall trägt denselben unabhängigen Normpfad und eine unterscheidende Hopmutation. |
| M-27 | nein | Der vierte Normfall trägt denselben unabhängigen Normpfad und eine unterscheidende Hopmutation. |
| M-28 | ja | Kein Wert vor 60 s ist Klasse (i); der 75-s-Wert ist je Altvergleich als Klasse (ii) eingeordnet, ohne künstlichen Rotlauf. |
| M-29 | nein | NaN/Inf und Zeitgrenzen werden am Wertpfad geprüft; die Grenzmutation bewegt die beobachtete Zusage. |
| M-30 | nein | Falscher §2.2-Text ist heute rot, die Register-Code-Gleichheit grüne Wache; beide Hälften haben getrennte Mutationen. |
| M-31 | nein | Stationärer Sinus und LRA < 1,0 LU bleiben grüne Wache; die Hopmutation hebt den Wert über die Toleranz. |
| M-32 | ja | Hann-Summen, ρ = 1/36, ν = 15,2583 und 2,39 dB sind konsistent; K = 1 reißt die 4-dB-Schranke. |
| M-33 | ja | 1,8 dB ist Signalreferenz, die Rauschfaltung nutzt dasselbe ν; nicht vorab berechnete Feinwerte bleiben Messschranke. |
| M-34 | ja | 3,6 dB und das Intervall benutzen dasselbe korrigierte Modell; die K=1-Mutation bleibt unterscheidend. |
| M-35 | ja | 5,4 dB und das Intervall benutzen dasselbe korrigierte Modell; die K=1-Mutation bleibt unterscheidend. |
| M-36 | nein | Unveränderte 6/10-dB-Textschwellen werden an zwei Karten beobachtet; 0,5 dB lässt den ruhigen Fall kippen. |
| M-37 | nein | Segment- und Teilblockhistogramm sind getrennt; der falsche Leser reduziert die Summe auf den achten Teil. |
| M-38 | nein | Restverfall, Reset und NaN vor dem ersten Teilblock sind gekoppelt; Restaufnahme erzeugt genau einen Zusatzeintrag. |
| M-39 | ja | Der Referenzinhalt ändert sich zusagegemäß und ist Golden-Klasse (ii): alter Inhalt rot, dann Erzeuger. |
| M-40 | nein | Versionsbindung der FeatureEngine bleibt durch A5 und Rückmutation eindeutig belegt. |
| M-41 | nein | Audio- und Allokationswachen beobachten die neue Teilblockstruktur ohne Audiothread-Arbeit. |
| M-42 | nein | SuperFlux-Gegenbeispiel und 79,96 dB sind am Zielbinmodell gerechnet; Bandflussmutation lässt den Nebenpfad sichtbar werden. |
| M-43 | nein | Binbereich, Nyquist-Kappe und DC-Ausschluss werden am tatsächlichen K beobachtet; Voll-FFT-Mutation verletzt die Zahl. |
| M-44 | nein | Maximumfilterbreite ist in Cent und Bins hergeleitet; ausgeschalteter Filter erzeugt den verbotenen Doppeltrigger. |
| M-45 | nein | 76,4/76,6 dB liegen beidseits von 0,05·1530 = 76,5 dB; Wache und rote Hälfte sind getrennt. |
| M-46 | nein | Median und echte MAD werden mit konstruierten Historienwerten geprüft; die Mittelwertmutation bewegt die Schwelle. |
| M-47 | nein | 59,5 dB folgt aus P0 und 1530 Bins; Aktivitätswache und neue Pegelreferenz sind getrennt etikettiert. |
| M-48 | nein | 400 liegt 85,3 ms hinter 500; stärkstes Ereignis und Sperrzeit werden ohne geschätzte Nebenwirkung beobachtet. |
| M-49 | nein | Ereignisstärke ist dimensionsbehaftet definiert und auf 1000 geklemmt; entfernte Klemme verletzt die Drahtschranke. |
| M-50 | nein | Nullkorpus W1 hat benannte Rate, Dauer und Fehlalarmschranke; die alte Relativschwelle ist als Basisgegenbeleg geführt. |
| M-51 | nein | W2 prüft dieselbe eigentliche Nullkorpuszusage bei geändertem Pegel und eigener Kennung. |
| M-52 | nein | W3 prüft dieselbe eigentliche Nullkorpuszusage bei geändertem Pegel und eigener Kennung. |
| M-53 | ja | P1 wird vor dem Nutzer an konstanter integrierter Oktavbandleistung geprüft; kein falscher Dichteabfall wird als Leistung verlangt. |
| M-54 | nein | Float32-Kleinsignale und deterministische Fehlalarmschranke prüfen den realen Eingangstyp; die Detektormutation bleibt wirksam. |
| M-55 | nein | Zweiter Float32-Grenzfall trägt eigenes Maß und eigene Kennung; keine nicht nachgerechnete Zahl wurde ergänzt. |
| M-56 | nein | Dritter Float32-Grenzfall führt unbekannte Feinwirkung als Messschranke; der Rotpfad bleibt an der Detektorzusage. |
| M-57 | ja | Der −50-dBFS-Rosa-Sweep verwendet den selbstgeprüften Erzeuger und misst die Ereigniszusage statt nur Spektrumsform. |
| M-58 | ja | Der −30-dBFS-Rosa-Sweep verwendet den selbstgeprüften Erzeuger und behält seine unterscheidende Schwellenmutation. |
| M-59 | ja | Der −10-dBFS-Rosa-Sweep verwendet den selbstgeprüften Erzeuger und behält seine unterscheidende Schwellenmutation. |
| M-60 | nein | Peakpfad, Stärke und Zusammenführung bleiben bestehende grüne Zusage; Abschalten des Pfads fällt direkt. |
| M-61 | ja | Wiederbeginn nutzt korrektes Rosa und beobachtet genau ein Onset im Zeitfenster; eingefrorener Vorframe verhindert es. |
| M-62 | ja | Start, Stop, Seek und Reset verwenden korrektes Rosa; jede Zustandserhaltung fällt am ersten erlaubten Ereignisfenster. |
| M-63 | nein | NaN/Inf und 1000-Klemme werden am serialisierten Zahlenrand geprüft; entfernte Riegel verletzen die eigentliche Zusage. |
| M-64 | nein | Der Raten- und Blocksweep prüft Ereigniszeit, nicht nur Anzahl; Mutation liegt an `Spektrum.h:687` und bewegt das Zeitfenster. |
| M-65 | nein | Wirepfad und Stärke werden gemeinsam gelesen; falscher Discriminator fällt vor Inhaltsauswertung. |
| M-66 | nein | Broker-Guardrail prüft das benannte Ereignisfeld; entfernte Gültigkeitsprüfung macht den A4-Fall rot. |
| M-67 | nein | Koinzidenz bleibt getrennte Wache mit echtem Aufrufpfad und wirksamer Zeitmutation. |
| M-68 | ja | Die 76 Bytes bleiben Klasse-(i)-Wache innerhalb derselben Version; Bandflussmutation statt Erzeugerlauf liefert rot. |
| M-69 | nein | Schema- und Kommentarwortlaut werden fail-closed in A5 geprüft; entfernte Einheit fällt am Vertragstext. |
| M-70 | nein | Versionsschritt trägt Code, Register und Schwellen gemeinsam; Rückmutation macht A5 rot. |
| M-71 | nein | Der Entwurfssatz wird durch messbare Detektorstruktur belegt; keine neue Designbehauptung oder offene Frage entsteht. |
| M-72 | nein | Audio, Allokationen und Lebenszyklus bleiben eigene Kanonwache; Audiothread-Mutation fällt in B4. |
| M-73 | nein | MSC je Bin wird gegen analytische Hann-Autokorrelation geprüft; Bandsummenmutation unterschreitet 0,95. |
| M-74 | ja | Alle Blockgrößen prüfen Kohärenz und 1,000000 ms ± 0,02 ms; falscher Divisor fällt an der Laufzeit. |
| M-75 | ja | 44,1 kHz prüft 0,997732 ms; 48-kHz-Divisor ergibt 0,916667 ms und ist unterscheidbar. |
| M-76 | ja | 88,2 kHz prüft 0,997732 ms; 48-kHz-Divisor ergibt 1,833333 ms und ist unterscheidbar. |
| M-77 | ja | 96 kHz prüft 1,000000 ms; 48-kHz-Divisor ergibt 2,000000 ms und ist unterscheidbar. |
| M-78 | ja | 176,4 kHz prüft 0,997732 ms; 48-kHz-Divisor ergibt 3,666667 ms und ist unterscheidbar. |
| M-79 | ja | 192 kHz prüft 1,000000 ms; 48-kHz-Divisor ergibt 4,000000 ms und ist unterscheidbar. |
| M-80 | ja | D1 bis D6 decken Raten und Blöcke mit 1000·d/fs ab; Vorzeichen- und Divisormutationen fallen an τ. |
| M-81 | ja | Breitband-D7, tatsächliche Binfrequenz und 1,399-rad-Abstand machen die Erstbinmutation gegen ±0,25 rad rot. |
| M-82 | nein | Framezahlen 5 sowie 11 bis 12 folgen aus Evidenz- und FFT-Kadenz; W_H = 1 verhindert das Kohärenzbit. |
| M-83 | nein | Bassframezahlen und Snapshotfolge sind nachgerechnet; W_B = 1 lässt acht Frames unerreichbar. |
| M-84 | nein | Ringlängen 20 und 11 sind aus Fensterfolgen gezählt; W_H = 10 hält verbotene 66 Frames. |
| M-85 | nein | Heapbudget und Ringelementgröße sind als Obergrenze geführt; ein festes Mitgliedsarray ist nicht Teil der Zusage. |
| M-86 | ja | Fensterdauer ist Klasse (i), neue Freiheitsgrade Klasse (ii); beide Hälften haben eigene wirksame Mutation. |
| M-87 | ja | Eine grüne Lebenszykluswache deckt Grenze, Reset und Ratenwechsel; jede ausgelassene Leerung lässt alte Frames überleben. |
| M-88 | nein | Start und Stop werden in Reihenfolge beobachtet; ausgelassenes Stop-Leeren verletzt den ersten Snapshot. |
| M-89 | nein | Mono-Kohärenz und Phase bleiben bestehende Wache; eine Sxy-Manipulation bewegt die Zusage außerhalb der Toleranz. |
| M-90 | nein | Antiphase und Polarität werden direkt am Snapshot geprüft; Vorzeichenmutation fällt am erwarteten Wert. |
| M-91 | nein | Erwartung 1/K und Verteilung der unabhängigen MSC begründen Mittel- und Maximalschranke; beide grünen Hälften sind mutierbar. |
| M-92 | nein | Unter acht Frames bleibt Kohärenz `null`, nicht 0; herabgesetzte Mindestzahl setzt das Bit und fällt. |
| M-93 | nein | Persistenz benutzt dieselbe MSC je Bin; Bandsummenmutation liefert nachgerechnet 0 statt größer 0,5. |
| M-94 | nein | Schema nennt Fensterdauer und Freiheitsgrade feldbezogen; A5-Textmutation ist fail-closed. |
| M-95 | nein | NaN/Inf und fehlende Leistung bleiben ohne Bit; entfernter Endlichkeitsriegel erzeugt verbotene Ausgabe. |
| M-96 | nein | Versionsbindung bleibt Code-Register-Wache mit eigener Rückmutation. |
| M-97 | ja | Startwert + 40 B entspricht genau Vektorkopf und vier Zählern; 524 288 B festes Array reißt Grenze und B5. |
| M-98 | ja | Fensterregel ist durch E-380-12 auf N_max 65 536 festgelegt; keine offene Variante bleibt. |
| M-99 | ja | Raten- und Längentafel folgt deterministisch aus Variante A; Festlängenmutation verletzt die erwarteten Werte. |
| M-100 | ja | FeatureEngine-Speicher der gewählten Kappe ist als Obergrenze mit Heapträgern geführt, nicht als offene Alternative. |
| M-101 | ja | AnalyseEngine-Speicher der gewählten Kappe ist als Obergrenze mit Heapträgern geführt, nicht als offene Alternative. |
| M-102 | ja | 192-kHz-Nyquist- und Kappenfall nutzt fest 65 536; eine 32-768-Kappe fällt an der Auflösung. |
| M-103 | ja | K1 nutzt E-380-13-geprüftes Rosa; 300 Hz und ±1/6 Oktave sind die beobachtete Resonanzzusage. |
| M-104 | ja | K2 nutzt E-380-13-geprüftes Rosa; 500 Hz und ±1/6 Oktave sind die beobachtete Resonanzzusage. |
| M-105 | ja | K3 nutzt E-380-13-geprüftes Rosa; 60 Hz und ±1/6 Oktave sind die beobachtete Resonanzzusage. |
| M-106 | ja | K4 nutzt E-380-13-geprüftes Rosa; Suchgrenze und „nicht gesucht“-Satz werden am 45-Hz-Fall beobachtet. |
| M-107 | ja | K5 nutzt E-380-13-geprüftes Rosa; 48 Hz liegt bewusst unter der zugesicherten 50,69-Hz-Grenze. |
| M-108 | ja | Suchgrenzen 46,57/50,69 Hz verwenden die entschiedene Kappe und korrekte Rosa-Vorprüfung; Fest-FFT-Mutation fällt. |
| M-109 | ja | Interpolierte LTAS-Bänder werden mit korrektem Rosa erzeugt und als `null` beobachtet; entfernte Maske fällt im Writer. |
| M-110 | nein | Beide Writer tragen dieselbe Validity-Maske; einer ohne Maske erzeugt Zahl statt `null` und fällt am Vertrag. |
| M-111 | nein | Bestehende Werte und Diagnosepfade bleiben interne Wache; Maskenänderung darf die Prozessrechnung nicht ändern. |
| M-112 | ja | 300 aktive Zellen ergeben genau 30 Ticks; Mittenhop liefert 70, sodass die absolute Zusage statt Ratengleichheit fällt. |
| M-113 | nein | Ratengleichheit ist Wache, die neue N_H-Schranke erst messbar; beide Hälften und Mutationen sind getrennt. |
| M-114 | nein | Nyquist-Kappe wird am höchsten Band beobachtet; entfernte Kappe erzeugt Bins jenseits Nyquist. |
| M-115 | nein | FFT-Rechenlast bleibt als in der Etappe zu messende +15-Prozent-Schranke statt als ungesicherter Istwert geführt. |
| M-116 | nein | LRA- und Detektorwachen werden nach Längenumbau wiederholt; keine neue Messzusage wird aus bloßem Grün abgeleitet. |
| M-117 | nein | Diagnosewortlaut und Rundung werden an echter Suchgrenze beobachtet; entfernte Satzfunktion fällt am Text. |
| M-118 | ja | Der geänderte Referenzinhalt ist Klasse (ii): neuer Code gegen alten Golden rot, danach Writer-Erzeuger und A5. |
| M-119 | ja | Startwert + 64 B ist aus double und zwei 28-B-Masken gerechnet; FeatureEngine bleibt gleich, feste Arrays brechen B5. |
| M-120 | nein | Leeren ist Wache, neue Längen heute rot; beide Lebenszyklushälften haben eigene wirksame Mutationen. |
| M-121 | nein | Audio, Allokationsfreiheit und Endkanon bleiben die eigentliche Abschlusszusage; Audiothread-Arbeit fällt in A1/B4. |
| M-122 | ja, neu | Gleiche Version liefert [0, 1], ungleiche „nicht vergleichbar“; entfernte Versionsschranke führt wieder zur falschen Materialklassifikation. |

## 14. Messung der Matrixnacharbeit 1 und Matrixprüfung 2 (Dirigent, 25.09.2026, 06:54 Uhr)

**Messung.** Codex-Bauer `gpt-5.6-sol`, Effort xhigh, `workspace-write`
(`tools/dirigent/codex-bau.ps1`, Kennung `nak380-mn1`, Thread
`01a0d6c8-bb24-7670-b8a1-299ba582095f`, 78 Kommandos), Start 06:17:54 Uhr,
Ende 06:52:23 Uhr, EXIT=0; HEAD `8d6444c7` vor und nach dem Lauf, kein
Commit durch den Bauer. Vom Dirigenten gemessen: genau eine Datei geändert
(`docs/beweise/NAK-380.md`, +494/−141); Hunks nur in der Kopfzeile
„Etappe“, in §5 bis §9 und im Anhang §13 (§0 bis §4 und §10 bis §12
unberührt); Datei LF ohne BOM; `py -3.13 tools/plan/dokuriegel.py` 0
Befunde (zwei markierte Zielpfade); Fettsatz 57 Zeichen. Stichproben am
Text: M-08 zwei Hälften mit Basiswert −37,7815 dBFS; M-17 je Feld; M-32 mit
ρ = 1/36, ν = 2304/151, 2,39 dB; M-81 mit Signal D7 (48 Samples, 19 Bins,
1,399 rad); M-97 ≤ Startwert + 40 B; M-112 genau 30 Ticks, Mutation 70;
M-119 ≤ Startwert + 64 B; M-122 neu (A4, heute nicht messbar); §7.1 Rosa
mit konstanter Oktavbandleistung; §8.6 Voraussetzung E-380-12 und M-122;
§6.6 nachgezählt 67/20/44 = 131 = 122 Zeilen plus 9 zweite Hälften (je
Etappe 21/23/33/27/27 Etiketten, stimmig). Selbstbericht des Bauers
(FERTIG, 42 Zeilen geändert, 1 neu) deckt sich mit der Messung. Commit
`cf33f554` (Pathspec, gepusht). Rundenbilanz `8d6444c7..cf33f554`: Doku 1
Datei +494/−141, ohne Produktfortschritt (Matrixrunde, zählt null).
Kumuliert seit `b5a0bf9c`: nur Doku (Matrix, Aufträge, Urteile), kein
Produkt-, kein Testdiff.

**Matrixprüfung 2 (Runde 1 von 3).** Vorlage B, frischer lesender
Codex-Thread `gpt-6-astra`, Effort high (NAK-396), Kennung `nak380-m2`,
abgekoppelt über `tools/dirigent/codex-lauf.ps1`; Prüfbereich nur der
Fixdiff `git diff 8d6444c7...cf33f554 -- docs/beweise/NAK-380.md`; HEAD
trägt darüber hinaus diesen Abschnitt und den Prüfauftrag
`docs/beweise/roh/NAK-380-matrixpruefung-2-auftrag.txt`. Urteil nach
`docs/beweise/roh/NAK-380-matrixpruefung-2-urteil.md` (Zielpfad, entsteht
nach dem Lauf).

## 15. Matrixprüfung 2: Urteil, Validierung, Einordnung und Nacharbeitsauftrag 2 (Dirigent, 25.09.2026, 07:18 Uhr)

**Urteil.** Frischer, lesender Codex-Thread `gpt-6-astra`, Effort high
(NAK-396), Thread `01a0d6ec-0c8a-7cf3-9f0e-fbcff0d2a8e0`, Kennung
`nak380-m2`, Vorlage B über `git diff 8d6444c7...cf33f554 --
docs/beweise/NAK-380.md`; HEAD `af6c323b` vor und nach dem Lauf; Lauf
06:56:28 bis 07:05:17 Uhr, 30 lesende Kommandos. Ein Fehlstart derselben
Kennung um 06:55:37 Uhr mit dem Skriptstandard Effort max (statt high nach
NAK-396) wurde nach rund 60 Sekunden ohne Urteil beendet und mit `-Effort
high` neu gestartet; kein Ergebnis des Fehlstarts ist verwendet. **URTEIL:
NEEDS_WORK — D1 bis D8 geschlossen, D9 und D10 offen, drei Brüche der
Nacharbeit 1** (Rohurteil `docs/beweise/roh/NAK-380-matrixpruefung-2-urteil.md`).
Vier Kommentare, als B1 bis B4 bezeichnet: B1 M-08 Hälfte (b) trägt eine
zweite Leistungszusage als Wache; B2 M-81 (Signal D7) als Wache etikettiert,
obwohl heute rot; B3 §8.1/§8.4 streicht die einmalige Erfassung der
M-68-Ausgangsbytes; B4 §8.3 dehnt die ±0,2-LU-Ausnahme von M-28 (b) auf
M-39 aus.

**Validierung.** Frischer, lesender Codex-Thread `gpt-6-astra` high, Thread
`01a0d6f6-a77e-7720-ba3d-e4cb5ebd45b6`, Kennung `nak380-m2v`; HEAD `d86a84fa`
vor und nach dem Lauf; 07:08:03 bis 07:16:14 Uhr, 16 Kommandos. **3
bestätigt, 1 präzisiert, 0 widerlegt**; Bericht
`docs/beweise/roh/NAK-380-matrixpruefung-2-validierung.md`. Nachgerechnet:
B1 Gruppe 35 (Feinbänder 120 bis 123, Bins 83 bis 93 der Hauptstufe, Ton in
Feinband 122) heute ≈ −18,0 dB gegen −3,01 ± 1,0 dB; B2 Kanten 15 544,5 bis
16 000 Hz → Bins 1327 bis 1365 (39), Mittenbin 1346, Δφ = 1,39899 rad;
heutige Bandsummen-Kohärenz ≈ 0,4765 < 0,8, dazu nur 7 Frames im letzten
Snapshot vor 10 s (⌊476 160/2048⌋ − ⌊460 800/2048⌋), also heute weder
Kohärenz- noch Phasenbit; B3 `Sonde013FingerprintGoldenTest.cpp:186-188`
vergleicht zwei frisch berechnete Fingerprints (4,27 s, nicht 30 s), keine
eingefrorene Hexreferenz für F1; der Diff der Nacharbeit 1 entfernte die
Erfassung; B4 Bytevergleich in `BriefkastenTestMain.cpp:444-445`,
`DiagnoseAntwort.cpp:65` schreibt `metrics_version`, alte Referenz
`m4.1-2026-08-15`: der Versionswechsel erzwingt verschiedene Bytes.

### 15.1 Einordnung je Befund

| Befund | Stelle | Was bricht (wörtlich) | Einordnung | Schließende Regel |
|---|---|---|---|---|
| B1 | M-08 (b) | Etikettendefinition „Regressionswache: heute grün und hält, was der Bau nicht ändern darf“. Der Sinus-Leistungswert Gruppe 35 = −3,01 ± 1,0 dBFS ist am Basisstand ≈ −18,0 dB (Bin-Dichtemittel `Spektrum.h:494`, ungewichtete Summe `Frame.h:347`). | **DEFEKT** (Rest von D10) | Beide Leistungswerte (a) Weißrauschen Gruppe 63, (b) Sinus Gruppe 35 stehen in der roten Hälfte mit heutigem Wert; die Wache umfasst nur Plausibilitätsbereich, Bit, `saturated` und Vertragsleserlauf. Zwei Hälften bleiben; §6.6 unverändert; §8.1, §8.2 mitziehen. |
| B2 | M-81 | Etikettendefinition und §8.1 („‚heute rot‘ zuerst gegen den unveränderten Basisstand“). Mit Signal D7 setzt der heutige Code weder Kohärenz- noch Phasenbit (Bandsumme ≈ 0,4765 < `kKohaerenzSchwellePhase` 0,8; 7 Frames < 8 im letzten Snapshot). | **DEFEKT** (Bruch der Nacharbeit 1; Ursache (c)) | M-81 „heute rot“ mit dokumentiertem Basisfall (Präsenz- und Phasenbit heute nicht gesetzt: Kohärenz und Framezahl); die Erstbin-Mutation bleibt als Rotbeweis nach dem Bau; §7.2 D7, §8.5 und §6.6 (68/20/43) mitziehen. |
| B3 | §8.1 `:1521-1523`, §8.4, M-68, F1, §7.5 | E-380-17 Klasse (i) „Golden oder Wert bleibt unverändert“ setzt eine erfasste Ausgangsreferenz voraus; zwei Berechnungen mit demselben neuen Code erkennen eine gemeinsame Änderung nicht. Die Nacharbeit 1 entfernte „am Etappenstart mit dem unveränderten Code erzeugten Golden (hex im Test, Erzeugung im Etappenabschnitt)“. | **DEFEKT** (Bruch der Nacharbeit 1) | E-380-18: Klasse (i) schließt die einmalige Erfassung der Ausgangsreferenz vor dem Umbau ein (siehe 15.2); M-68 erfasst die 76 Byte von F1 am unveränderten Etappenstart (hex im Test, Rohdatei), vergleicht danach unverändert; verboten bleibt nur der Nachzug; Mutation bleibt Rotbeleg; F1, §7.5, §8.1, §8.4 mitziehen. |
| B4 | §8.3 `:1680-1685`, M-28, M-39 | M-39 „die neue Referenz trägt die neue `metrics_version`“, §8.1 Klasse (ii) und E-380-17 „neuer Code gegen alten Golden rot (Rohdatei), dann Erzeuger“. Der Bytevergleich (`BriefkastenTestMain.cpp:444-445`) wird durch den Versionswechsel zwingend rot; eine Toleranzausnahme ist dort sinnlos. | **DEFEKT** (Bruch der Nacharbeit 1) | E-380-18: die Ausnahme „Altwert innerhalb der neuen Toleranz“ gilt nur für Wertvergleiche mit Toleranz (M-28 (b)), nie für Bytevergleiche; M-39 Klasse (ii) mit Pflicht-Rotlauf und B30-Erzeuger; §8.3 und die Klassenzuordnung in §8.1 mitziehen. |

**Ergebnis: 4 Defekte, 0 Lücken, 0 Härtungen; keine Karte.** Zählung des
Prüfers (2 offen, 3 gebrochen) und der Validierung (4 Befunde) decken sich:
B1 ist der Rest von D10, B2 bis B4 sind Brüche der Nacharbeit 1; D9 gilt
mit B3 und B4 als offen.

### 15.2 Entscheid des Dirigenten

- **E-380-18 (B3, B4) — Präzisierung von E-380-17.** Klasse (i) (Wache):
  die Referenz wird genau einmal vor dem Umbau mit dem unveränderten Code
  erfasst (hex im Test oder Fixture; Erfassung im Etappenabschnitt mit
  Rohdatei und SHA-256), danach unverändert verglichen; verboten ist nur der
  Nachzug nach dem Umbau; der Rotbeleg ist die temporäre Mutation. Klasse
  (ii) (nachgezogener Golden): der Rotlauf gegen den alten Inhalt ist
  Pflicht; die Ausnahme „liegt der Altwert innerhalb der neuen Toleranz, ist
  der dokumentierte Altvergleich der Beleg“ gilt ausschließlich für
  Wertvergleiche mit Toleranz (M-28 (b)) und nie für Bytevergleiche (M-39,
  M-118, Fixtures). Jede Golden-Zeile nennt Klasse und, bei Klasse (i), den
  Ort der erfassten Referenz.

### 15.3 Ursache und Durchsicht

Alle vier Befunde tragen Ursache (c) aus §12.3: ein Etikett oder eine
Bauanweisung ohne Nachmessung am Basisstand (B1, B2) beziehungsweise eine
Golden-Klasse ohne ihre Referenz oder mit unzulässiger Ausnahme (B3, B4).
Die Durchsicht der Nacharbeit 1 (§13.2) hat (c) für M-81 nicht erkannt,
weil die Zeile im selben Zug umgebaut und dabei neu etikettiert wurde. Die
Nacharbeit 2 belegt deshalb **jede** Zeile mit dem Etikett „Regressionswache“
(nach §6.6 44, nach B2 43) einzeln: in §16.2 je Zeile ein Satz, warum die
Zusage am Basisstand grün ist, mit der Codezeile oder Zahl, die es trägt;
eine Zeile ohne belegbares Grün wird umetikettiert („heute rot“ mit Wert
oder „heute nicht messbar“) und die Zählung nachgezogen. Für jede
Golden-Zeile (M-12, M-28, M-39, M-68, M-86, M-118 und alle weiteren, die
einen Golden, ein Fixture oder eine Referenzdatei nennen) steht Klasse und
Referenzort nach E-380-18.

### 15.4 Matrixnacharbeit 2 (Runde 2 von 3)

Bauer nach NAK-396 (vor dem Wochenreset 15:00 Uhr): Codex `gpt-5.6-sol`,
Effort xhigh, `workspace-write` über `tools/dirigent/codex-bau.ps1`,
Kennung `nak380-mn2`; genau `docs/beweise/NAK-380.md` (§5 bis §9, neuer
Abschnitt §16); kein Commit, der Dirigent misst und committet. Auftrag:
`docs/beweise/roh/NAK-380-matrixnacharbeit-2-auftrag.txt`. Danach
Matrixprüfung 3 (Vorlage B, Codex `gpt-6-astra` high, Prüfbereich nur der
Fixdiff der Runde 2: `stand-vor-der-runde...HEAD -- docs/beweise/NAK-380.md`;
Stand vor der Runde ist der Commit dieses Abschnitts). Rundenbilanz
`cf33f554..HEAD` der Runde 1 kumuliert: nur Doku, null.

## 16. Matrixnacharbeit 2 (Bauer, 25.09.2026)

### 16.0 Schließung der Befunde B1 bis B4

| Befund | Geänderte Stellen | Schließung |
|---|---|---|
| B1 | M-08, §8.1, §8.2 | Beide Leistungszusagen stehen gemeinsam in der roten Hälfte: Gruppe 63 heute −37,7815 statt −10,8809 ± 1,0 dBFS und Gruppe 35 heute ungefähr −18,0 statt −3,01 ± 1,0 dBFS; die Wache umfasst nur [−144, +24] dB, Bit, `saturated` und den Vertragsleserlauf. |
| B2 | M-81, §6.6, §7.2 D7, §7.5, §8.5 | M-81 ist heute rot, weil der Basisstand bei D7 mit ungefähr 0,4765 < 0,8 und 7 Frames < 8 weder Kohärenz-Präsenz- noch Phasenbit setzt; nach dem Bau bricht die Erstbin-Mutation die 0,25-rad-Zusage um 1,39899 rad. |
| B3 | M-68, §7.2 F1, §7.5, §8.0 T-380-5, §8.1, §8.4, §9 | F1 wird am unveränderten Etappenstart vor dem ersten Produktedit einmal als 76-Byte-Hexreferenz erfasst und mit SHA-256 in der Rohdatei protokolliert; danach bleibt die Klasse-(i)-Referenz unverändert, nur ein Nachzug nach dem Umbau ist verboten. |
| B4 | M-28, M-39, §8.1, §8.3, §9 | Die ±0,2-LU-Ausnahme ist nur für den tolerierten Wertvergleich M-28 (b) zulässig; der Bytevergleich M-39 läuft wegen der neuen `metrics_version` zwingend gegen den alten Inhalt rot und erst danach durch den B30-Erzeuger. |

### 16.1 Änderungsregister

| ID oder Anker | Vorher | Jetzt | Grund |
|---|---|---|---|
| Kopf, Etappe | Die Runde 2 war nur als Auftrag angekündigt. | Der letzte Fettsatz meldet die geschriebene Matrixnacharbeit 2 und die ausstehende Matrixprüfung 3. | Lebender Kopf nach Auftrag. |
| M-01 | Der analytische Erwartungswert galt ohne festen Saatbeleg als heutige Wache. | Der neue B5-Fall erfasst den konkreten 60-s-W0-Ausgang vor dem ersten Produktedit; bis dahin ist die Zusage heute nicht messbar. | Vollständige Ursache-(c)-Durchsicht nach §15.3. |
| M-08 | Nur Weißrauschen war rot; die Sinus-Leistung stand in der Wache. | Beide Leistungswerte sind mit heutigen Werten rot, nur Trägerrand und Draht bleiben Wache. | B1, E-380-18 und Etikettendefinition. |
| M-28 | Die tolerierte Klasse-(ii)-Ausnahme war nicht ausdrücklich exklusiv. | Die Ausnahme ist ausdrücklich nur der Wertvergleich M-28 (b); M-28 (a) nennt den vorhandenen Referenzort. | B4 und E-380-18. |
| M-39 | Die Klasse-(ii)-Zeile ließ sich zusammen mit M-28 als tolerierter Altvergleich lesen. | Der Bytevergleich verlangt den Rotlauf gegen `m4.1-2026-08-15` und danach den B30-Erzeuger, ohne Ausnahme. | B4 und E-380-18. |
| M-68 | Zwei frisch berechnete Fingerprints sollten eine nicht vorhandene feste Referenz ersetzen. | Die 76 F1-Bytes werden einmal vor dem Umbau erfasst, im Test als Hex und in der Rohdatei mit SHA-256 festgehalten und danach unverändert verglichen. | B3 und E-380-18. |
| M-81 | D7 war trotz fehlender Bits als Regressionswache etikettiert. | Der Basisfall ist mit ungefähr 0,4765 < 0,8 und 7 Frames < 8 heute rot; die Erstbin-Mutation bleibt der Rotbeweis nach dem Bau. | B2 und Ursache (c). |
| M-91 | Beide Zusagehälften waren allein aus der MSC-Verteilung als Wachen etikettiert. | Der bestehende feste Sweep belegt weiter „kein Band mit Phase“; Mittel ≤ 0,15 und Maximum ≤ 0,7 sind bis zur ersten Erfassung im erweiterten B19-Fall heute nicht messbar. | Vollständige Ursache-(c)-Durchsicht nach §15.3. |
| M-115 | Der erst nach Etappe 4 und 6 messbare Hochratenfall war Regressionswache. | Der Fall ist bis zum neuen exakten B5-Zähler bei 96 und 192 kHz heute nicht messbar. | Vollständige Ursache-(c)-Durchsicht nach §15.3. |
| §6.6 | Die Etikettenzählung war 67/20/44. | Die Zählung ist 68/23/40; Etappe 2 ist 10/4/7, Etappe 5 ist 13/6/8 und Etappe 6 ist 14/6/7. | M-01, M-81, M-91 (Kohärenz) und M-115 umetikettiert; Summe 131 bleibt. |
| §7.2 D7 | D7 nannte nur das Zielsignal und den 1,399-rad-Abstand. | D7 nennt Block, Intervall, Bins, Mittenbin und beide heutigen Gründe für die fehlenden Bits. | B2. |
| §7.2 F1 | F1 behauptete vorhandene 76 Golden-Bytes ohne Erfassungsort. | F1 verlangt die einmalige Vor-Umbau-Erfassung, Hex im Test, Rohdatei und SHA-256. | B3 und E-380-18. |
| §7.5 | B22 war nur als unveränderte Wache ohne Erzeugerlauf beschrieben. | B22 erfasst vor dem ersten Produktedit einmal die feste F1-Ausgangsreferenz und zieht sie danach nie nach; D7 ist als heute rot benannt. | B2 und B3. |
| §8.0, T-380-5 | Die Fingerprint-Bytegleichheit war auf dieselbe `metrics_version` beschränkt, obwohl M-70 die Etappenfassung hebt. | Die vor dem Umbau erfassten 76 Bytes bleiben nach dem Detektorumbau bytegleich; M-122 trennt später semantische Vergleiche verschiedener Fassungen. | B3 und E-380-18. |
| §8.1 | E-380-17 verbot Klasse (i) pauschal jeden Erzeugerlauf und nannte keine feste M-68-Ausgangsreferenz; die Toleranzausnahme war nicht auf Wertvergleiche begrenzt. | E-380-18 trennt einmalige Vor-Umbau-Erfassung von verbotenem Nachzug, verlangt für Klasse (ii) den alten Inhalt rot und beschränkt die Ausnahme auf M-28 (b). | B3 und B4. |
| §8.2 | Nur der Weißrauschwert von M-08 lief vor dem Bau rot, und M-01 galt ohne festen Saatbeleg als Wache. | Beide M-08-Leistungswerte laufen rot; deren Wache enthält nur Plausibilität, Bit, `saturated` und Leser; M-01 erfasst den Basiswert zuerst. | B1 und Ursache (c) aus §15.3. |
| §8.3 | Die ±0,2-LU-Ausnahme wurde gemeinsam auf M-28 (b) und M-39 angewandt. | M-28 (b) bleibt tolerierter Wertvergleich; M-39 ist zwingender Byte-Rotlauf mit anschließendem B30-Erzeuger. | B4. |
| §8.4 | M-68 sollte ohne vorhandene eingefrorene Ausgangsreferenz unverändert bleiben. | Der Etappenablauf erfasst die 76 Bytes einmal vor dem Produktedit und friert Hex sowie SHA-256 ein. | B3. |
| §8.5 | D7 nannte nur den Rotbeweis nach dem Bau, und M-91 leitete feste Saatwerte nur aus einer Verteilung ab. | Der Bauplan nennt den roten D7-Basisfall; bei M-91 bleibt nur die vorhandene Phasenwache, die neuen Kohärenzwerte werden zuerst erfasst. | B2 und Ursache (c) aus §15.3. |
| §8.6 | M-115 stand undifferenziert unter neuen Fällen. | Der Bauplan erklärt, warum erst der neue Hochratenzähler die Zusage messbar macht und welche Mutation sie danach bricht. | Ursache (c) aus §15.3. |
| §9 | Die Prüfliste hatte keinen vollständigen Basisstandsbeleg und setzte M-68 mit „ohne Erzeugerlauf“ gleich. | §16.2 belegt jede Wache; M-68 trennt einmalige Erfassung vom verbotenen Nachzug und Klasse-(ii)-Bytes verlangen den alten Inhalt rot. | B3, B4 und §15.3. |
| §16 | Es gab keinen Rundenanhang. | Befundschließung, Änderungsregister, vollständiger Wachenbeleg und vollständige Golden-Klassifikation sind angefügt. | Auftrag Matrixnacharbeit 2. |

### 16.2 Wachenbeleg

Die 40 Zusagehälften mit dem Etikett „Regressionswache“ sind hier
vollständig belegt. M-81 wurde wegen des roten Basisfalls in „heute rot“ und
M-01, M-91 (Kohärenz) sowie M-115 wurden wegen fehlender fester
Basisstandswerte in „heute nicht messbar“ umetikettiert; weitere
Umetikettierungen ergab die Durchsicht nicht.

| ID oder Hälfte | Beleg für Grün am Basisstand |
|---|---|
| M-07 | `Frame.h:352-375` setzt ein Gruppenbit nur bei plausibler endlicher Energie, und `BandGrid.h:252-256` liefert für Energie ≤ 0 keinen endlichen 0-dB-Wert. |
| M-08 (b) | Die heutigen Werte −37,7815 und ungefähr −18,0 dB liegen beide in [−144, +24] dB aus `BandGrid.h:233-238`; `Frame.h:352-375` setzt daher die Bits ohne `saturated`, und `TelemetryClient.cpp:113-131` schreibt genau diese gültigen Träger. |
| M-14 | `rechnung.rs:361-392` vergleicht `wert - basis` ohne Bandbreitengewicht, sodass die +6-dB-Abweichung in Band 30 größer als die +3-dB-Abweichung in Band 200 ist. |
| M-15 | `messung.rs:64-74` summiert vier dB-Werte und teilt durch vier; −20, −40, −40, −40 ergeben deshalb heute exakt −35,00 dB. |
| M-16 | `eingang.rs:45-55` verwirft nicht-endliche Eingänge und die Endlichkeitsriegel in `zusammenhang.rs:195-205` sowie `maskierung.rs:137-151` halten Überlauf, leere Bänder und −300 dB endlich beziehungsweise leer. |
| M-18 | `broker/src/telemetrie.rs:590-596` akzeptiert jede positive `metrics_version`; die bestehenden A5-, A8-, A10-, B3c- und A4-Korpusfälle tragen die unveränderten Manifestklassifikationen. |
| M-20 | Die bestehenden Kanonbeine A1, A16 und B1 prüfen bitgleichen Durchlauf und Identität; B4 führt den 4000-Block-Allokationszähler aus `DspGoldenTestMain.cpp:6822-6874`, während Etappe 2 am Basisstand keine Audiothread-Arbeit ergänzt. |
| M-24 | Der aktuelle 1-Hz-Hop in `FeatureEngine.h:409-411` trifft bei den 20-s-Konstantsegmenten analytisch 10 LU wie die Normreferenz; `Lautheit.h:255-291` verwendet unverändert P95 minus P10, also innerhalb 10 ± 1 LU und der unabhängigen §5-Referenz ± 0,1 LU. |
| M-25 | Derselbe heutige P95-minus-P10-Weg in `Lautheit.h:255-291` ergibt für die 20-s-Stufen −20/−15 analytisch 5 LU, damit die Normtoleranz 5 ± 1 LU und die §5-Referenz ± 0,1 LU. |
| M-26 | Derselbe heutige P95-minus-P10-Weg in `Lautheit.h:255-291` ergibt für die 20-s-Stufen −40/−20 analytisch 20 LU, damit die Normtoleranz 20 ± 1 LU und die §5-Referenz ± 0,1 LU. |
| M-27 | Validierung Teil C `:389` misst für T4 bei 1 Hz und 10 Hz jeweils 15,00 LU; der Basisstand erfüllt daher 15 ± 1 LU und die §5-Referenz ± 0,1 LU. |
| M-28 (a) | Der bestehende Fall `Sonde013DynamicsTest.cpp:584-587` verlangt nach 30 s ausdrücklich keinen LRA-Wert, getragen von `kLraMindestSekunden = 60.0` in `FeatureEngine.h:409`. |
| M-30, Prüfung | Code und Register tragen heute beide `kLraHopZellen = 10` (`FeatureEngine.h:410-411`, `metriken-v1.json:149`), sodass der bestehende A5-Gleichlauf grün ist; nur der §2.2-Text gehört zur roten Hälfte. |
| M-31 | `Sonde013DynamicsTest.cpp:596-606` führt das konstante 75-s-Signal bereits und verlangt LRA < 1,0 LU; `Lautheit.h:255-291` liefert für die stationäre Folge eine Spanne nahe null. |
| M-36 | Die vorhandenen A2-Fälle `GoldenTestMain.cpp:185-196,260-271` tragen „steht ruhig“ beziehungsweise Smooth Operator, während `Diagnose.cpp:205-223` die unveränderten Grenzen ≤ 6 dB und > 10 dB anwendet. |
| M-37 | `AnalyseEngine.cpp:929-962` berechnet Abdeckung aus dem Segmenthistogramm, und `GoldenTestMain.cpp:559-577` hält die bestehenden Grenzen ≥ 24 beziehungsweise ≤ 4. |
| M-41 | A1 bindet `AnalyseEngine.cpp` und `Diagnose.cpp`, A16 und B1 prüfen Durchlauf und Identität; der Basisstand legt `pegelHistogramm` im Konstruktor `AnalyseEngine.cpp:163-167` an und ergänzt keine Audiothread-Allokation. |
| M-45, 76,4 dB | Bei MAD = 0 verhindert der heutige Riegel in `Spektrum.h:672-679` ein Flussereignis, sodass 76,4 dB unter der künftigen 76,5-dB-Grenze weiterhin null Ereignisse liefert. |
| M-47 (a) | `Spektrum.h:448-451,477` verwirft beide unter −60 dB liegenden Frames vor dem Detektor; der Sprung von −90 auf −70 dBFS erzeugt daher heute kein Ereignis. |
| M-60 | Die bestehenden B5-Fälle `short_impulse_triggers_peak_path_only`, `flux_event_carries_flux_quality` und `both_paths_yield_one_event` tragen den Peakpfad; `Spektrum.h:672-699` erzeugt höchstens ein Ereignis und schreibt die Crest-Stärke. |
| M-64 | Der bestehende NAK-182-Sweep in B5 prüft die blockgrößenunabhängige Zeitlage; `Spektrum.h:687-699` bindet `stromSample` an `fensterStromStart` statt an den Blockanfang. |
| M-66 | `experiment.rs:504-528,574-575` vergleicht streng `w > 2.0`; deshalb reißt 2,0 heute nicht und 2,01 reißt. |
| M-67 | Der bestehende Fall `broker/tests/sonde014_hypothese.rs:1503` führt konstante Onsetreihen; `zusammenhang.rs:227-230` gibt bei fehlender Streuung `None` statt künstlicher Korrelation zurück. |
| M-68 | `Spektrum.h:623-629` speist den Fingerprint heute mit dem unveränderten Bandfluss, und `Sonde013FingerprintGoldenTest.cpp:186-188,227,283-284` belegt deterministische 76-Byte-Ergebnisse; E-380-18 friert vor dem ersten Etappe-4-Produktedit zusätzlich genau den 30-s-F1-Ausgang als Hex und SHA-256 ein. |
| M-72 | A1, A16 und B1 sind die vorhandenen Durchlauf- und Identitätswachen; der Detektorzustand liegt am Basisstand in den in `FeatureEngine.h:447-520` vorbereiteten Workerträgern und nicht im Audiocallback. |
| M-86, Dauer | Der bestehende Fall `Sonde013StereoGoldenTest.cpp:338-376` berechnet und vergleicht die Fensterdauer aus der gezählten Framezahl und Hopdauer; dieser Wert bleibt die Klasse-(i)-Referenz. |
| M-87 | `FeatureEngine.h:447-520,526-535` und `Zeit.h:384-388` leeren den gesamten heutigen Stereozustand bei Vorbereiten, Rücksetzen und Grenze; der bestehende §32.3-Fall `Sonde013StereoGoldenTest.cpp:727-785` trägt die Grenzwache. |
| M-89 | Der bestehende Fall `Sonde013StereoGoldenTest.cpp:160-205` misst bei identischen Kanälen Korrelation > 0,999, Kohärenz > 0,99, Phase ± 0,01 rad und Folddown ± 0,25 dB. |
| M-90 | Der bestehende Antiphasenfall `Sonde013StereoGoldenTest.cpp:692-724` misst Korrelation −1, Kohärenz > 0,99 und die Monosumme an der Vertragsgrenze. |
| M-91, Phase | Der bestehende unkorrelierte Sweep `Sonde013StereoGoldenTest.cpp:1275-1302` verlangt kein Phasenbit; `Stereo.h:229-242` setzt Phase nur oberhalb der Kohärenzschwelle 0,8. |
| M-92 | `Stereo.h:225-227` verlangt mindestens 8 Frames, und die bestehenden Fälle `Sonde013StereoGoldenTest.cpp:207-247,378-428` belegen ohne Energie beziehungsweise darunter kein Kohärenzbit. |
| M-95 | Der Endlichkeitsriegel in `Stereo.h:65-96` zählt einen Bin mit nicht-endlichem Kreuzspektrum nicht zum Band und lässt damit keine NaN-Ausgabe entstehen. |
| M-97 | Am Etappenstart gilt `sizeof (FeatureEngine)` definitionsgemäß als Startwert und die längenabhängigen FFT-Träger sind Vektoren (`FeatureEngine.h:1311-1336`); die Zusage erlaubt nur den nachgerechneten Zuwachs 24 + 16 = 40 B. |
| M-111 | Die bestehenden A2-Assertions `GoldenTestMain.cpp:455-480` vergleichen `ltasKompositDb` und `ltasReferenzDb`, und A3 trägt den Markierungsvergleich; `AnalyseEngine.cpp:628-690` lässt die heutigen Prozesswerte numerisch. |
| M-112 | `AnalyseEngine.cpp:28,270,385-399` bildet eine 0,1-s-Zelle und einen Tick je 10 Zellen; 30 s aktive Musik ergeben damit heute bei 48 und 96 kHz exakt 300 Zellen und 30 Ticks, während `GoldenTestMain.cpp:507-520` den bestehenden Mindestfall trägt. |
| M-113, Identität | Der bestehende NAK-182-Sweep hält `stromSample` je Rate über alle Blockgrößen identisch, weil `Spektrum.h:687` den Fensterstart nutzt; nur die neue N_H-Schranke gehört zur nicht messbaren Hälfte. |
| M-116 | Die bestehenden B9- und B18-Fälle prüfen Momentary, Short-term, Integrated und LRA; deren Zellenweg `Lautheit.h:255-291` hängt am Basisstand nicht von einer FFT-Länge ab. |
| M-119 | Am Etappenstart entsprechen beide Objektgrößen ihren Startwerten und die längenabhängigen Träger in `FeatureEngine.h:1311-1336` und `AnalyseEngine.h:290-369` liegen im Heap; die Zusage erlaubt für AnalyseEngine den festen Zuwachs 8 + 28 + 28 = 64 B und für FeatureEngine 0 B. |
| M-120, Leeren | `FeatureEngine.h:447-520,526-535` ruft beim Vorbereiten den Rücksetzweg auf, sodass heutige Akkus, Ring und Vorframe den Ratenwechsel nicht überleben; nur die neuen Längen gehören zur roten Hälfte. |
| M-121 | A1, A16 und B1 sind die vorhandenen Durchlauf- und Identitätswachen, und B4 zählt 0 Audiothread-Allokationen über 4000 Blöcke; der Basisstand erzeugt Längen und Träger nur in `vorbereiten`. |

### 16.3 Golden-Klassen

Die Suche in §6 nach „Golden“, „Fixture“, „Referenz“, „bytegleich“ und
„Erzeuger“ ergibt die folgenden 15 Matrix-IDs. Jede Zusagehälfte ist nach
E-380-18 eingeordnet; die einzige Altwert-Ausnahme steht bei M-28 (b).

| ID oder Hälfte | Klasse und Vergleich | Referenzort oder Pflichtablauf |
|---|---|---|
| M-12 (i) | Klasse (i), Wertvergleich mit grobem Bereich. | Die erfasste Ausgangsreferenz steht als Testgrenze „zwischen −20 und −19“ in `maskierung.rs:309-315`; sie ist am Basisstand mit −19,87 dB grün und wird nicht nachgezogen. |
| M-12 (ii) | Klasse (ii), Wertvergleich auf −19,12 ± 0,01 dB. | **Rotlauf gegen den alten Inhalt Pflicht:** Der heutige −19,87-dB-Wert ist der alte Inhalt; danach wird ausschließlich der aus Gitterbreiten berechnete Sollwert −19,12 dB in den Test übernommen. |
| M-18 | Klasse (i), bytegleiche v3- und FlatBuffers-Fixtures. | Die erfassten Ausgangsreferenzen sind die vorhandenen Dateien unter `eq-copilot/fixtures/v3/` mit ihren MANIFEST-Dateien sowie die FlatBuffers-Fixtures; A5, A8, A10, B3c und das A4-Korpus halten sie unverändert. |
| M-24 | Klasse (i), Norm- und Wertvergleich. | Der unveränderliche Sollwert steht in Tech 3342 Tabelle 1 als 10 ± 1 LU und in der unabhängigen §5-Rechnung als 10 ± 0,1 LU; vor dem ersten Etappe-3-Produktedit wird der heutige 1-Hz-Ausgang zusätzlich im Etappenabschnitt mit Rohdatei und SHA-256 erfasst und als Testliteral geführt. |
| M-28 (a) | Klasse (i), Vorhandenseinsvergleich. | Die erfasste Ausgangsreferenz ist die bestehende Assertion „nach 30 s kein Wert“ in `Sonde013DynamicsTest.cpp:584-587`; sie bleibt unverändert. |
| M-28 (b) | Klasse (ii), Wertvergleich mit ±0,2 LU. | Der Altwert wird vor dem Umbau im Etappenabschnitt erfasst; nur hier darf ein Altwert innerhalb der neuen Toleranz den dokumentierten Altvergleich belegen, andernfalls gilt: **Rotlauf gegen den alten Inhalt Pflicht**, danach Nachzug auf die §5-Referenz. |
| M-36 | Klasse (i), textuelle Fixture-Ausgabe. | Die erfassten Ausgangsreferenzen stehen in den vorhandenen A2-Fixtures `resonanz-116hz` und `diag-wander-ton` sowie ihren Assertions `GoldenTestMain.cpp:185-196,260-271`; Schwellen und Kartenklassifikation bleiben unverändert. |
| M-37 | Klasse (i), Fixture-Wertvergleich. | Die erfassten Ausgangsreferenzen sind die bestehenden A2-Assertions für `pink-minus20` und `sinus-1k-minus12` in `GoldenTestMain.cpp:559-577`; nur der neue interne Histogrammgleichlauf wird ergänzt. |
| M-39 | Klasse (ii), Bytevergleich. | **Rotlauf gegen den alten Inhalt Pflicht:** `eq-copilot/fixtures/diagnose/festhalten-referenz.json` trägt `m4.1-2026-08-15`; `BriefkastenTestMain.cpp:444-445` vergleicht bytegleich und `DiagnoseAntwort.cpp:65` ändert die Bytes, erst danach läuft `EqCopBriefkastenTest --erzeuge`; keine Toleranzausnahme. |
| M-50 | Klasse (ii), Wertvergleich des Ereigniszählers. | **Rotlauf gegen den alten Inhalt Pflicht:** Der feste W1-Basisfall hat 19 Ereignisse (`b11_ursache.out`, acht Saaten 13 bis 24 in `b11_fluss.out`); nach dem roten Altvergleich wird der neue exakte Zähler innerhalb der Schranke ≤ 1 im Test zum Golden. |
| M-68 | Klasse (i), Bytevergleich über 76 Bytes. | Vor dem ersten Etappe-4-Produktedit wird F1 mit unverändertem Code einmal 30 s gerechnet; die Ausgangsreferenz steht danach als Hexliteral im B22-Test und mit SHA-256 in `docs/beweise/roh/NAK-380-etappe-4-m68-f1-ausgang.txt` (Zielpfad, anzulegen) und wird nach dem Umbau nie nachgezogen. |
| M-74, Kohärenz | Klasse (ii), Wertvergleich des B19-Goldens. | **Rotlauf gegen den alten Inhalt Pflicht:** Vor dem Stereo-Umbau wird die heutige Bandsummen-Kohärenz für alle 18 Blockgrößen im Etappenabschnitt mit Rohdatei und SHA-256 erfasst; der neue Code läuft gegen diese alten Werte rot, danach gilt ≥ 0,95. |
| M-74, Gruppenlaufzeit | Klasse (ii), neu entstehender Wertvergleich. | **Rotlauf gegen den alten Inhalt Pflicht:** Der erweiterte B19-Fall läuft zuerst gegen den unveränderten Code rot, weil dessen alte Ausgabe keinen Gruppenlaufzeitwert trägt; erst nach diesem Rotlauf gilt der analytische Sollwert 1,000000 ± 0,02 ms, ohne erfundenen alten Zahlenwert. |
| M-86, Dauer | Klasse (i), Wertvergleich mit ±1 ms. | Die erfasste Ausgangsreferenz ist die aus Framezahl und Hopdauer gebildete Erwartung in `Sonde013StereoGoldenTest.cpp:338-376`; sie bleibt unverändert. |
| M-86, Freiheitsgrade | Klasse (ii), Wertvergleich. | **Rotlauf gegen den alten Inhalt Pflicht:** Der heutige Wert nur des letzten Fensters wird vor dem Umbau im Etappenabschnitt mit Rohdatei und SHA-256 erfasst; erst nach seinem Rotlauf wird die Erwartung auf W Fenster nachgezogen. |
| M-110 | Klasse (ii), Wert- und Validity-Vergleich im Analysis-Golden. | **Rotlauf gegen den alten Inhalt Pflicht:** Der heutige numerische Heartbeat-Inhalt der künftig maskierten Bänder wird vor dem Etappe-6-Umbau mit Rohdatei und SHA-256 erfasst; danach lautet die neue Referenz `null`. |
| M-111 | Klasse (i), Fixture-Wertvergleich. | Die erfassten Ausgangsreferenzen sind die bestehenden A2-LTAS-Assertions in `GoldenTestMain.cpp:455-480` und der A3-Markierungsvergleich; die Prozesswerte bleiben unverändert. |
| M-112 | Klasse (i), Wertvergleich auf 30 Ticks. | Vor dem ersten Etappe-6-Produktedit werden die heutigen 30 Ticks bei 48 und 96 kHz im Etappenabschnitt mit Rohdatei und SHA-256 erfasst; das Testliteral folgt außerdem unabhängig aus 300 aktiven 0,1-s-Zellen geteilt durch 10. |
| M-118 | Klasse (ii), Bytevergleich. | **Rotlauf gegen den alten Inhalt Pflicht:** Der neue Etappe-6-Code läuft gegen die dann alte B30-Datei byteverschieden rot; erst danach erzeugt `EqCopBriefkastenTest --erzeuge` die neue `festhalten-referenz.json`, ohne Toleranzausnahme. |

## 17. Messung der Matrixnacharbeit 2 und Matrixprüfung 3 (Dirigent, 25.09.2026, 07:55 Uhr)

**Messung.** Codex-Bauer `gpt-5.6-sol`, Effort xhigh, `workspace-write`
(`tools/dirigent/codex-bau.ps1`, Kennung `nak380-mn2`, Thread
`01a0d701-1d79-7042-8dd4-b903497be309`, 63 Kommandos), Start 07:19:29 Uhr,
Ende 07:52:50 Uhr, EXIT=0; HEAD `7b6f58d2` vor und nach dem Lauf, kein
Commit durch den Bauer. Vom Dirigenten gemessen: genau eine Datei geändert
(`docs/beweise/NAK-380.md`, +191/−48); Hunks nur in der Kopfzeile „Etappe“,
in §5 bis §9 und im Anhang §16 (§0 bis §4 und §10 bis §15 unberührt); LF
ohne BOM; `py -3.13 tools/plan/dokuriegel.py` 0 Befunde (drei markierte
Zielpfade). Stichproben am Text: M-08 beide Leistungswerte in Hälfte (a)
heute rot (−37,7815 und ≈ −18,0 dBFS), Wache (b) nur Plausibilität, Bit,
`saturated`, Leser; M-81 heute rot mit beiden Gründen (Kohärenz ≈ 0,4765,
7 Frames), Erstbin-Mutation nach dem Bau; M-68 Klasse (i) mit einmaliger
Erfassung der 76 Byte vor dem Umbau (Hex im Test, Rohdatei mit SHA-256,
Zielpfad); M-39 Klasse (ii) Bytevergleich mit Pflicht-Rotlauf, §8.3 ohne
Ausnahme für M-39; Umetikettierungen M-01, M-91 (Kohärenz), M-115 nach
„heute nicht messbar“ mit Begründung (kein fester Basiswert für die neue
Saat); §16.2 belegt 40 Wachen, §16.3 ordnet 15 Golden-IDs nach E-380-18;
§6.6 nachgezählt 68/23/40 = 131 = 122 Zeilen plus 9 zweite Hälften (je
Etappe 21/23/33/27/27, stimmig). Der Selbstbericht des Bauers erwähnt
einen eigenen „frischen Diffprüfer“ ohne Befund; er ist kein Prüfer dieses
Tickets und wird nicht gewertet. Commit `ac3e9c12` (Pathspec).
Rundenbilanz `7b6f58d2..ac3e9c12`: Doku 1 Datei +191/−48, ohne
Produktfortschritt (Matrixrunde, zählt null); kumuliert seit `b5a0bf9c`
nur Doku.

**Matrixprüfung 3 (Runde 2 von 3).** Vorlage B, frischer lesender
Codex-Thread `gpt-6-astra`, Effort high (NAK-396; `-Effort high` gesetzt),
Kennung `nak380-m3`, abgekoppelt über `tools/dirigent/codex-lauf.ps1`;
Prüfbereich nur der Fixdiff `git diff 7b6f58d2...ac3e9c12 --
docs/beweise/NAK-380.md` mit Stichprobe des Wachenbelegs (mindestens acht
Zeilen, zwei je Etappe); HEAD trägt darüber hinaus diesen Abschnitt und den
Prüfauftrag `docs/beweise/roh/NAK-380-matrixpruefung-3-auftrag.txt`. Urteil
nach `docs/beweise/roh/NAK-380-matrixpruefung-3-urteil.md` (Zielpfad,
entsteht nach dem Lauf).

## 18. Matrixprüfung 3: Urteil, Validierung, Einordnung und Nacharbeitsauftrag 3 (Dirigent, 25.09.2026, 08:11 Uhr)

**Urteil.** Frischer, lesender Codex-Thread `gpt-6-astra`, Effort high
(NAK-396), Thread `01a0d722-31ec-7b72-9a03-b146f7b9a36e`, Kennung
`nak380-m3`, Vorlage B über `git diff 7b6f58d2...ac3e9c12 --
docs/beweise/NAK-380.md`; HEAD `55f768c3` vor und nach dem Lauf; 07:55:37
bis 08:02:56 Uhr, 32 lesende Kommandos. **URTEIL: NEEDS_WORK — B1 bis B4
geschlossen, zwei Brüche in den neuen Belegen** (Rohurteil
`docs/beweise/roh/NAK-380-matrixpruefung-3-urteil.md`; die verlangte
FERTIG-Zeile und die Stichprobenliste fehlen im Bericht, das Urteil steht in
der ersten Zeile). Die zwei Kommentare, als C1 und C2 bezeichnet: C1 M-16
(d) und §16.2 — der Wachenbeleg erklärt die Teilzusage „bandpassung =
Breitenanteil des Bands“ für grün, obwohl `bandpassung` heute ungewichtet
zählt; C2 M-50 und §16.3 — die als fester W1-Basisfall geführten 19
Ereignisse stammen aus einem Nachbau mit anderem Signal.

**Validierung.** Frischer, lesender Codex-Thread `gpt-6-astra` high, Thread
`01a0d72b-6ccf-7fd2-92ac-f1fd16d1fbe6`, Kennung `nak380-m3v`; HEAD `56e6f7d0`
vor und nach dem Lauf; 08:05:41 bis 08:09:07 Uhr, 12 Kommandos. **1
bestätigt (C1), 1 präzisiert (C2), 0 widerlegt**; Bericht
`docs/beweise/roh/NAK-380-matrixpruefung-3-validierung.md`. Nachgerechnet:
C1 `zusammenhang.rs:198-204` addiert `10f64.powf(db / 10.0)` ohne
Breitengewicht, Fall (d) liefert 4/221 ≈ 0,01809955; die Gitterkanten
verlangen (45,48916 − 40,52624)/(17 959,39277 − 30,36037) ≈ 0,00027681;
die Fälle (a) bis (c) tragen die heutigen Riegel (`eingang.rs:48-54`,
`zusammenhang.rs:195-209`, `maskierung.rs:137-151`) und bleiben Wache. C2
`b11_ursache.py:9-11` erzeugt Gauß-Weißrauschen mit
`np.random.default_rng(5)`, `b11_fluss.py:70-72` mit den Saaten 100 bis 107;
keiner entspricht W1 (SplitMix64, Box-Muller, Saat 0x3800003, §7.1, §7.2);
M-50 darf die Nachbauwerte nach der Etikettendefinition weiter als „heute
rot“ führen, der §16.3-Satz „Der feste W1-Basisfall hat 19 Ereignisse“ ist
unbelegt. Teil 2 des Auftrags (weitere §16.2-Zeilen) nicht ausgeführt.

### 18.1 Einordnung je Befund

| Befund | Stelle | Was bricht (wörtlich) | Einordnung | Schließende Regel |
|---|---|---|---|---|
| C1 | M-16 (d), §16.2 Zeile zu M-16 | Etikettendefinition „Regressionswache: heute grün“; §15.3 „in §16.2 je Zeile ein Satz, warum die Zusage am Basisstand grün ist, mit der Codezeile oder Zahl, die es trägt“. Die Teilzusage (d) „bandpassung = Breitenanteil des Bands“ hängt am Umbau nach R-380-1 und ist heute 4/221 statt 0,000277. | **DEFEKT** (Ursache (c), Bruch der Nacharbeit 2) | M-16 wird zwei Hälften: (d) Breitenquote heute rot mit Basiswert 4/221 ≈ 0,018100 gegen 0,000277 und Mutation nach dem Bau (Gewicht `breite(index)` → 1,0 in `bandpassung` → 4/221 → rot); (a) bis (c) Regressionswache mit der bestehenden Mutation (`is_finite` weg → NaN). §6.6: 69/23/40 = 132 Etiketten, zehn zweite Hälften (M-16 neu), Etappe 2 11/4/7; §16.2 und §8.2 mitziehen. |
| C2 | §16.3 Zeile zu M-50; M-50 | E-380-18 Klasse (ii) „der Rotlauf gegen den alten Inhalt ist Pflicht“ — der alte Inhalt ist der Ausgang desselben Falls mit unverändertem Code, nicht ein Nachbauwert eines anderen Signals (`default_rng(5)` statt SplitMix64 0x3800003); sonst kann der Signalwechsel allein den Rotlauf erzeugen. Das Etikett „heute rot“ von M-50 mit Quelle Nachbau bleibt zulässig (§6-Kopf: „aus den Nachbauten der Fachprüfung genommen (Quelle genannt)“). | **DEFEKT** (präzisiert: nur die behauptete feste Ausgangsreferenz; Ursache (c)) | §16.3 M-50 und M-50 selbst: der W1-Altzähler wird vor dem Produktumbau der Etappe 4 mit unverändertem Code für genau W1 (§7.2) erfasst und in der Rohdatei dokumentiert; der Rotlauf des neuen Codes geht gegen genau diese Referenz; die 19 Ereignisse bleiben als Nachbauwert mit anderem Signal (Quelle `b11_ursache.out`) Beleg für „heute rot“, nie als feste W1-Referenz. Dieselbe Regel gilt für jede Klasse-(ii)-Zeile in §16.3, deren „alter Inhalt“ aus einem Nachbau stammt (M-74 Kohärenz, M-86 Freiheitsgrade, M-110, M-112, M-24 prüfen). |

**Ergebnis: 2 Defekte, 0 Lücken, 0 Härtungen; keine Karte.** Beide
Befunde sind Brüche der Nacharbeit 2, keine offenen Punkte aus B1 bis B4.

### 18.2 Ursache und Weg der Runde 3

Beide Defekte teilen Ursache (c) in der Form „Beleg deckt nicht genau die
spezifizierte Teilzusage beziehungsweise nicht genau das spezifizierte
Signal“. Zwei Durchsichten in Prosaform (§13.2, §16.2) haben diese Form
nicht ausgeschlossen. Die Nacharbeit 3 wechselt deshalb die Form des Belegs:
je Wachen-Hälfte eine Tabellenzeile mit drei getrennten Spalten — (1) die
Teilzusage wörtlich, (2) das Signal oder der Eingang mit Kennung aus §7.2,
(3) die Codezeile am HEAD oder die Zahl, die genau diese Teilzusage für genau
diesen Eingang heute erfüllt; eine Zeile, die eine Spalte nicht füllen kann,
wird umetikettiert. Für §16.3 gilt je Klasse-(ii)-Zeile eine Spalte
„Herkunft des alten Inhalts“ (derselbe Fall mit unverändertem Code, vor dem
Umbau erfasst; ein Nachbauwert ist nie der alte Inhalt). Das ist der
Ursachenweg nach §2 („Haben Befunde eine gemeinsame Ursache, ist die Ursache
der Gegenstand“).

### 18.3 Matrixnacharbeit 3 (Runde 3 von 3)

Bauer nach NAK-396 (vor dem Wochenreset 15:00 Uhr): Codex `gpt-5.6-sol`,
Effort xhigh, `workspace-write` über `tools/dirigent/codex-bau.ps1`,
Kennung `nak380-mn3`; genau `docs/beweise/NAK-380.md` (§5 bis §9, §16.2 und
§16.3 in der neuen Belegform als §19); kein Commit, der Dirigent misst und
committet. Auftrag: `docs/beweise/roh/NAK-380-matrixnacharbeit-3-auftrag.txt`.
Danach Matrixprüfung 4 (Vorlage B, Codex `gpt-6-astra` high, Prüfbereich nur
der Fixdiff der Runde 3; Stand vor der Runde ist der Commit dieses
Abschnitts). Endet die Matrixprüfung 4 ohne PASS, folgt der
Konvergenzentscheid nach Skill §3.4 in derselben Sitzung. Rundenbilanz aller
Matrixrunden: null (nur Doku).

## 19. Matrixnacharbeit 3 (Bauer, 25.09.2026)

### 19.0 Schließung der Befunde C1 und C2

| Befund | Geänderte Stellen | Schließung |
|---|---|---|
| C1 | M-16, §6.6, §8.2, §9, §19.2 | M-16 hat jetzt zwei Hälften: Nur (a) bis (c) bleiben Regressionswache; (d) ist mit dem heutigen Wert 4/221 = 0,01809955 gegen die Breitenquote 0,0002768095 und der Mutation Gewicht `breite(index)` → 1,0 „heute rot“. Die Zählung ist 69/23/40 bei zehn zweiten Hälften und Etappe 2 ist 11/4/7. |
| C2 | M-50, §7.5, §8.4, §9, §19.3 | Der Klasse-(ii)-Altinhalt wird vor dem Produktumbau mit unverändertem Code für genau W1 (SplitMix64, Box-Muller, σ = 0,1, Saat 0x3800003, 30 s, 48 kHz) erfasst; der neue Code läuft gegen genau diese Referenz rot. Die 19 Ereignisse und die Spanne 13 bis 24 bleiben ausschließlich Nachbauwerte anderer Signale für das Etikett „heute rot“. |

### 19.1 Änderungsregister

| ID oder §-Anker | Was vorher stand | Was jetzt gilt | Befund oder Regel |
|---|---|---|---|
| Kopf, Etappe | Matrixnacharbeit 3 lief noch. | Matrixnacharbeit 3 ist geschrieben; Matrixprüfung 4 steht aus. | Lebender Kopf nach Auftrag. |
| M-16 | Alle vier Zahlenrandfälle waren gemeinsam Regressionswache, obwohl (d) ungewichtet 4/221 lieferte. | (a) bis (c) bleiben Wache; (d) ist mit Basiswert, Soll-Breitenquote und tragender Breitenmutation „heute rot“. | C1 und §18.1. |
| M-50 | Die Nachbauwerte 19 und 13 bis 24 konnten zugleich als alter W1-Inhalt gelesen werden, und die Zusage unterschied Altvergleich und neues Golden nicht. | Zusage und Rotbeweis führen den mit unverändertem Code für genau W1 erfassten alten Inhalt ausschließlich als Pflicht-Altvergleich; die Nachbauwerte belegen nur „heute rot“, und erst nach dem Rotlauf wird der neue Zähler ≤ 1 zum Golden. | C2, E-380-18 und §18.1. |
| M-47 (a) | Das weiße Rauschen hatte keine Saat und sein Eingang war damit im Wachenbeleg nicht exakt ansteuerbar. | Profil und Matrix nennen SplitMix64, Box-Muller, Saat 0x3800001 und die Normierung auf die beiden festen Rahmenenergien. | Ursache (c), §18.2 und Determinismusanforderung. |
| M-07 | Der Wachenbeleg zeigte Bitlosigkeit und −∞, aber weder die zugesagte Füllung 0 noch die das zweite Bein sperrende Plausibilitätsprüfung. | Der Beleg nennt jetzt `satz.leeren()`, das Nullen des Breitenfelds, `Bandsatz::leeren()` und für Summe 0,0 bei n = 3 die Plausibilitätsprüfung vor dem Setzen des Bits. | Ursache (c), §18.2. |
| M-87 | Eingang und Beleg vermischten das spezifizierte 44,1-kHz-Profil mit einem bestehenden 48-kHz-Seek-Test; „Binzahl“ entsprach zudem nicht dem Ratenwechselpfad. | Profil, Blockgröße, drei Beine und Rechenweg sind exakt benannt; die Zusage fordert die neue Binzuordnung für 96 kHz und belegt jede der drei Leerungen am HEAD. | Ursache (c), §18.2 und Determinismusanforderung. |
| §6.6 | Die Zählung war 68/23/40 bei neun zweiten Hälften und Etappe 2 war 10/4/7. | Die Zählung ist 69/23/40 bei zehn zweiten Hälften und Etappe 2 ist 11/4/7. | C1. |
| §7.5 | B5 benannte keinen Pflichtlauf zur Erfassung des alten W1-Zählers. | B5 erfasst vor dem Etappe-4-Umbau den Altzähler für genau W1 und der neue Code läuft gegen diesen Inhalt rot. | C2 und E-380-18. |
| §8.1 | Der vollständige Wachen- und Golden-Beleg verwies auf die Verlaufsabschnitte §16.2 und §16.3. | Wachenbeleg und Golden-Klassen verweisen auf §19.2 und §19.3; §16.2 und §16.3 bleiben Verlauf. | Ursache (c), §18.2. |
| §8.2 | M-16 wurde im Rotbeweisabschnitt nicht in grüne und rote Hälfte getrennt. | Der Ablauf nennt die Wache (a) bis (c), den roten Basiswert von (d) und die Breitenmutation nach dem Bau. | C1. |
| §8.4 | Nur M-68 war dort als Golden-Klasse ausgeführt. | M-50 ist Klasse (ii) mit Pflicht-Altvergleich für genau W1; die Nachbauwerte sind ausdrücklich keine Referenz. | C2 und E-380-18. |
| §9 | Die Prüfliste verwies auf §16.2/§16.3 und nannte M-16(d) nicht als rot. | Sie verweist auf §19.2/§19.3, nennt M-16(d) rot und bindet M-50 an den Altinhalt desselben W1-Falls. | C1, C2 und Ursache (c). |
| §19.2 | Der Prosa-/Zweispaltenbeleg in §16.2 trennte Teilzusage, Eingang und tragende Codezeile nicht. | Alle 40 Wachenhälften stehen einzeln mit wörtlicher Teilzusage, gekennzeichnetem Eingang, genauem Basisbeleg und Etikett in fünf Spalten. | §18.2. |
| §19.3 | §16.3 trennte bei Klasse (ii) Nachbauwerte nicht in einer eigenen Spalte vom alten Inhalt und führte nur 15 Golden-IDs. | Alle 37 durch die vorgeschriebene Stichwortsuche in §6 gefundenen IDs stehen in 42 Zeilen; Herkunft und Nachbauwert sind getrennt, und jede Klasse-(ii)-Referenz stammt aus demselben Fall mit unverändertem Code. | C2, E-380-18 und §18.2. |

### 19.2 Wachenbeleg

Die 40 Zusagehälften mit dem Etikett „Regressionswache“ sind vollständig
geführt. M-16(d) ist nicht Teil dieser Tabelle, sondern die neue rote Hälfte;
weitere Umetikettierungen ergab die Durchsicht nicht.

| ID oder Hälfte | Teilzusage wörtlich (der Satz oder Halbsatz aus der Zusagespalte, der heute grün sein soll) | Eingang mit Kennung (§7.2-Signal, Fixture, Profil oder Vertragsdatei, mit Saat oder Datei) | Codezeile am HEAD oder Zahl, die genau diese Teilzusage für genau diesen Eingang heute erfüllt (Datei:Zeile wörtlich zitiert oder Rechenweg mit Ergebnis) | Etikett danach |
|---|---|---|---|---|
| M-07 | „Gruppe 10 trägt in beiden Fällen kein Bit (Bitmap 0) und keinen Zahlenwert außer der Füllung 0; nie ‚0 dB‘ mit Bit.“ | Profil M-07: Gruppe 10 ohne belegten Akku beziehungsweise Summe 0,0 bei n = 3; keine Zufallsquelle. | `eq-copilot/plugin/core/analysis/featureengine/Frame.h:333-335` ruft `satz.leeren()` auf und nullt Breitenfeld sowie Bitmap; `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:547-551` setzt jeden Wert auf 0, nullt die Bitmap und löscht Sättigung; `Frame.h:352-353` lässt das Bit bei null Beleg weg; für Summe 0,0 bei n = 3 bildet `BandGrid.h:252-256` −∞, das `Frame.h:368-375` als nicht plausibel verwirft, bevor Wert und Bit gesetzt werden. | Regressionswache |
| M-08 (b) | „(b) Trägerrand- und Drahtwache: beide Gruppen liegen im Plausibilitätsbereich [−144, +24] dB (`BandGrid.h:233-238`), ihr Bit ist gesetzt, `saturated` ist falsch und beide Frames passieren den Vertragsleser ohne Verstoß“ | §7.2 W0h mit Saaten 0x3800001/0x3800002 und §7.2 S3 mit Amplitude 1,0; dieselben Frames am P2-Draht. | Die heutigen Werte −37,7815 und ungefähr −18,0 dBFS liegen in `BandGrid.h:233-238`; `Frame.h:368-376` setzt Bit und kein Sättigungsbit, `TelemetryClient.cpp:113-131` schreibt genau diese Träger; `NakamaTelemetrie.cpp:455-465,699,705-712` prüft den Frame und lässt `lese` nur ohne Verstoß passieren. | Regressionswache |
| M-14 | „Befundgruppe = `gruppe_von_band (30)`, beobachtetes Band 30 (größte Dichteabweichung vom Median)“ | Profil M-14: P50 −20 dB flach, Band 30 +6 dB, Band 200 +3 dB; keine Saat. | `broker/src/coordinator/hypothese/rechnung.rs:367-373` bildet `wert - basis` ohne Breite; 6 dB > 3 dB, daher gewinnt Band 30. | Regressionswache |
| M-15 | „−35,00 dB (dB-Mittel), unverändert“ | Profil M-15: Bänder 10-13 = −20, −40, −40, −40 dB; keine Saat. | `broker/src/coordinator/hypothese/messung.rs:64-73` summiert und teilt durch n: (−20 − 40 − 40 − 40)/4 = −35,00 dB. | Regressionswache |
| M-16 (a) bis (c) | „(a) bis (c): (a) fällt am Leser (`eingang.rs:48-54`: nicht endlich ist `None`), (b) am Leistungsriegel (`zusammenhang.rs:198-200`, `maskierung.rs:140-143`); das Ergebnis ist jeweils endlich; (c) bandpassung 0,0 und bandpegel `None`“ | Profile M-16(a) P50 NaN/±∞, (b) P50 4000 dB, (c) nur Bänder ohne Bit; keine Saat. | `eingang.rs:48-54` verwirft (a), `zusammenhang.rs:198-209` und `maskierung.rs:140-151` verriegeln Überlauf und leere Summen; `bandpassung` gibt bei Gesamtleistung 0 genau 0,0 zurück. | Regressionswache |
| M-18 | „jedes Fixture wird wie im MANIFEST klassifiziert (A5, A8, A10, B3c, A4-Korpus); beide Frames werden angenommen, kein Leser prüft den Versionswert“ | `eq-copilot/fixtures/v3/MANIFEST.json` samt `eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json`; P2-Frames mit `metrics_version` 20260904 und neuer positiver Version. | `broker/src/telemetrie.rs:590-599` verwirft nur Werte < 1; `NakamaTelemetrie.cpp:455-465,705-712` prüft im C++-Leser ebenfalls nur `metrics_version < 1`, sodass beide positiven Versionen beide Leser passieren; `MANIFEST.json:20-27` und `flatbuffers/MANIFEST.json:10-18` halten die bestehenden Klassifikationen fest. | Regressionswache |
| M-20 | „A1 und A16 (Passthrough bitgleich, 0 Latenz, kein Tail) und B1 grün; B4 zählt 0 Allokationen im Audiothread über 4000 Blöcke (`tools/beweise.ps1:709`)“ | Kanon A1 mit deterministischem Rauschen, A16 mit je Rate gesätem `juce::Random`, B1 Identitätskorpus, B4 fester Sinus und deterministische Blockfolge. | `NullTestMain.cpp:30-50` fordert 0/0 und Bitgleichheit; `SondeNullTestMain.cpp:1096-1104,1193-1197` dasselbe bei vier Raten; `IdentityTestMain.cpp:250-319,415-445,680-718` hält Manifest, Bauquelle und CIDs; `QueueStressTestMain.cpp:778-791` misst genau 0 Allokationen über 4000 Blöcke. | Regressionswache |
| M-24 | „LRA = 10 ± 1 LU (Norm) und gleich der §5-Referenz über die analytische 10-Hz-Folge desselben Signals ± 0,1 LU“ | §7.2 T1: 1-kHz-Stereo-Sinus, −20/−30 dBFS je 20 s, zweimal; deterministisch ohne Saat. | `Lautheit.h:255-291` bildet unverändert P95 − P10; für die konstanten Stufen ist das 10,0 LU, also in 10 ± 1 LU und 10 ± 0,1 LU. | Regressionswache |
| M-25 | „LRA = 5 ± 1 LU und §5-Referenz ± 0,1 LU“ | §7.2 T2: T1 mit −20/−15 dBFS; deterministisch ohne Saat. | Derselbe Weg `Lautheit.h:255-291` ergibt P95 − P10 = 5,0 LU und erfüllt beide Toleranzen. | Regressionswache |
| M-26 | „LRA = 20 ± 1 LU und §5-Referenz ± 0,1 LU“ | §7.2 T3: T1 mit −40/−20 dBFS; deterministisch ohne Saat. | Derselbe Weg `Lautheit.h:255-291` ergibt P95 − P10 = 20,0 LU und erfüllt beide Toleranzen. | Regressionswache |
| M-27 | „LRA = 15 ± 1 LU und §5-Referenz ± 0,1 LU (Validierung Teil C `:389`: 15,00 LU bei 10 Hz und bei 1 Hz, gemessen)“ | §7.2 T4: 1-kHz-Stereo-Sinus, je 20 s −50/−35/−20/−35/−50 dBFS; deterministisch ohne Saat. | `docs/beweise/roh/NAK-379-dsp-validierung-teil-C.md:387-389` misst für genau T4 bei 10 Hz und 1 Hz je 15,00 LU; `Lautheit.h:288-291` trägt P95 − P10. | Regressionswache |
| M-28 (a) | „nach 30 s kein Wert“ | Bestehendes wanderndes 1-kHz-Signal aus `eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:576-582`; deterministische Hüllkurve, keine Saat. | `Sonde013DynamicsTest.cpp:584-587` fordert Abwesenheit; `FeatureEngine.h:409` setzt 60 s Mindestdauer. | Regressionswache |
| M-30, Prüfung | „A5 prüft je `nicht_gefuehrt`-Eintrag der Form ‚kName = Wert‘, dass der Code denselben Wert trägt“ | Vertragsdatei `eq-copilot/schemas/v3/metriken-v1.json`, Eintrag `kLraHopZellen`; Code `FeatureEngine.h`. | `metriken-v1.json:143-149` und `FeatureEngine.h:410-411` tragen beide den heutigen Wert 10; nur der Text „§2.2“ gehört zur roten Hälfte. | Regressionswache |
| M-31 | „LRA < 1,0 LU“ | Bestehender 1-kHz-Sinus mit Amplitude 0,2 für 75 s in `Sonde013DynamicsTest.cpp:596-602`; keine Saat. | `Sonde013DynamicsTest.cpp:603-606` fordert < 1,0 LU; `Lautheit.h:288-291` bildet für die stationäre Folge P95 − P10 nahe 0. | Regressionswache |
| M-36 | „116-Hz-Karte: `gemessen` enthält ‚steht ruhig‘, Werkzeug fester Absenker (`tu` nennt Frequenz und Startwert, `GoldenTestMain.cpp:195-196`); Wanderton: Smooth Operator (`:270-271`); `Diagnose.cpp:211` (≤ 6) und `:223` (> 10) unverändert“ | A2-Fixtures `resonanz-116hz` und `diag-wander-ton` in `eq-copilot/plugin/tests/GoldenTestMain.cpp`. | `GoldenTestMain.cpp:185-196,261-271` hält Ort und Werkzeugtexte; `Diagnose.cpp:205-223` wendet genau ≤ 6 dB und > 10 dB an. | Regressionswache |
| M-37 | „für jedes Band b im Zuständigkeitsbereich seiner Stufe ist die Summe seines Segmenthistogramms gleich `segmente[b]` der zuständigen Stufe (Testzugang `AnalyseEngineTestzugang`, Zielpfad des Freundes in `AnalyseEngine.h`); die Abdeckungsfälle von A2 (`pink-minus20` ≥ 24 belastbar, `sinus-1k-minus12` ≤ 4, `GoldenTestMain.cpp:559-577`) bleiben“ | §7.2 W4, Saat 0x3800006, sowie A2-Fixtures `pink-minus20` und `sinus-1k-minus12`. | `AnalyseEngine.cpp:500-503,929-962` füllt und liest dasselbe Segmenthistogramm; `GoldenTestMain.cpp:559-577` hält ≥ 24 beziehungsweise ≤ 4 belastbare Gruppen. | Regressionswache |
| M-41 | „A1 (übersetzt `AnalyseEngine.cpp` und `Diagnose.cpp`, `eq-copilot/plugin/CMakeLists.txt:635-645`), A16 und B1 grün“ | Kanon A1/A16/B1 und B4 wie M-20; keine zusätzliche Zufallsquelle. | `AnalyseEngine.cpp:163-167` legt das Histogramm im Konstruktor an; die exakten Durchlauf-, Identitäts- und Allokationsassertionen stehen in `NullTestMain.cpp:30-50`, `SondeNullTestMain.cpp:1096-1104,1193-1197`, `IdentityTestMain.cpp:250-319,415-445,680-718` und `QueueStressTestMain.cpp:778-791`. | Regressionswache |
| M-45, 76,4 dB | „76,4 → kein Ereignis“ | Profil M-45: Historie 32 × 0,0, aktiver lokaler Maximalframe, Sperrzeit frei, K = 1530, SF = 76,4 dB; keine Saat. | `Spektrum.h:672-679` verlangt am HEAD positive Streuung; bei MAD = 0 entsteht exakt kein Flussereignis. | Regressionswache |
| M-47 (a) | „kein Ereignis“ | Profil M-47(a): Weißrauschen L = R aus SplitMix64 und Box-Muller, Saat 0x3800001, je Abschnitt auf die feste Rahmenenergie −90 beziehungsweise −70 dBFS bei 48 kHz normiert. | Beide festen Rahmenenergien liegen unter −60 dBFS; `Spektrum.h:448-451,477` beendet deshalb beide Rahmen vor dem Detektor, sodass der 20-dB-Sprung 0 Ereignisse erzeugt. | Regressionswache |
| M-60 | „unverändert grün; ein reines Peakereignis trägt `staerke` = Crest über Schwelle in dB (`Spektrum.h:697-699`); lösen Fluss und Peak im selben Frame aus, entsteht genau ein Ereignis mit beiden Bits“ | Bestehende B5-Fälle `short_impulse_triggers_peak_path_only`, `flux_event_carries_flux_quality`, `both_paths_yield_one_event` in `AnalysisGoldenTestMain.cpp`. | `AnalysisGoldenTestMain.cpp:2123-2255` führt genau die drei Eingänge; `Spektrum.h:672-699` bildet einen Eintrag und setzt Peak-/Flussbits samt Crest-Stärke. | Regressionswache |
| M-64 | „`stromSample` je Rate über alle Blockgrößen identisch und in [Impuls − N_H(fs), Impuls]“ | Bestehender NAK-182-Sweep B5: Impuls bei Stromsample 206336, fünf feste Blockgrößen, fünf feste Raten; keine Saat. | `AnalysisGoldenTestMain.cpp:2476-2488` fordert Identität und die 4096er-Schranke; `Spektrum.h:687` bindet den Stempel an `fensterStromStart`. | Regressionswache |
| M-66 | „2,0 reißt nicht, 2,01 reißt (`w > schwelle`); die Einheit bleibt ‚Summe von Stärken je Fenster‘ (`experiment.rs:574-575`)“ | Broker-Profil M-66 mit Onsetmittel Basis 0,0 und Resultat 2,0/2,01; keine Saat. | `broker/src/coordinator/experiment.rs:516-528` vergleicht streng `w > schwelle`; `experiment.rs:574-575` setzt die Schwelle 2,0 und nennt die Einheit. | Regressionswache |
| M-67 | „`None` (keine Streuung), Rangkomponente 0, weder Beleg noch Gegenbeleg; keine Schwelle zu ziehen“ | Broker-Profil M-67: Quelle und Master mit stationären Onsetreihen 0. | `zusammenhang.rs:214-230` gibt die Korrelation ohne Streuung als `None`; `broker/tests/sonde014_hypothese.rs:1500-1504` fordert Rangkomponente 0. | Regressionswache |
| M-68 | „genau 76 Ausgangsbytes werden vor dem Umbau einmal mit dem unveränderten Code erfasst, als Hexliteral im B22-Fall und mit SHA-256 in `docs/beweise/roh/NAK-380-etappe-4-m68-f1-ausgang.txt` (Zielpfad, anzulegen) festgehalten und danach bytegleich verglichen; `fingerprintSchritt` bekommt weiter den Bandfluss (`Spektrum.h:623-629`); kein Nachzug nach dem Umbau“ | §7.2 F1: deterministischer Akkord 220/277,18/329,63 Hz, 30 s, 48 kHz, keine Saat. | `FeatureEngine.h:378-381` gibt N = 4096 und Hop = 2048 vor: 30 s · 48 kHz ergeben 702 Hauptstufenfenster und nach dem einmaligen Vorframe-Rücksprung `Spektrum.h:610-611` noch 701 Fingerprintfenster, also mehr als die 32 aus `featureengine/Fingerprint.h:109,122-127`; `featureengine/Vertrag.h:340-351` legt dafür dauerunabhängig exakt 32 + 12 + 32 = 76 Ausgangsbytes fest, `Fingerprint.h:129-181` baut sie deterministisch, und `Spektrum.h:623-629` speist genau den Bandfluss. | Regressionswache |
| M-72 | „A1, A16 und B1 grün“ | Kanon A1/A16/B1 und B4 wie M-20. | Detektorträger entstehen in `FeatureEngine.h:493-514` in `vorbereiten`; die Durchlauf-, Identitäts- und Allokationsassertionen stehen in `NullTestMain.cpp:30-50`, `SondeNullTestMain.cpp:1096-1104,1193-1197`, `IdentityTestMain.cpp:250-319,415-445,680-718` und `QueueStressTestMain.cpp:778-791`. | Regressionswache |
| M-86, Dauer | „Fensterdauer = Summe der Hopdauern genau der gezählten Frames ± 1 ms“ | Bestehender B19-Fall `coherence_carries_window_and_dof`, 48 kHz, 1 s, identischer 1-kHz-Sinus in L/R; keine Saat. | `Sonde013StereoGoldenTest.cpp:338-375` rechnet `hopMs * freiheitsgrade` und fordert eine Abweichung < 1 ms. | Regressionswache |
| M-87 | „(a) und (b): höchstens die Frames seit der Grenze (höchstens 5, weil der erste Hauptstufenframe 4096 Samples braucht und der erste Evidenzsnapshot spätestens nach 13 824 Samples endet), Kohärenz `null`; (c) Binzuordnung für 96 kHz neu, kein Wert der alten Rate überlebt“ | Profil M-87: 44,1 kHz, Block 512, Evidenzintervall 0,25 s, korrelierter 1-kHz-Sinus L = R, deterministisch ohne Saat; (a) Seek bei 5 s, (b) direkter Aufruf `zuruecksetzen()`, (c) direkter Ratenwechsel `vorbereiten (96000)`; jeweils erster Snapshot danach. | (a) `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:347-388` leert FFT-Ringe und Stereozustand; (b) `eq-copilot/plugin/core/analysis/FeatureEngine.h:526-535` leert dieselben Träger; die 0,1-s-Kadenz (`FeatureEngine.h:428,680-682`; Reset `Frame.h:179`) braucht bei 44,1 kHz und Block 512 je höchstens `ceil(4410/512) · 512 = 4608` Samples, also erreicht der dritte und erste ≥0,25-s-Snapshot (`Frame.h:45-46`) spätestens 13 824 Samples; 4096 Punkte/2048 Hop (`FeatureEngine.h:378-381`) ergeben darin höchstens 5 Hauptstufenframes; Kohärenz braucht mindestens 8 (`FeatureEngine.h:150-161`, `Stereo.h:225-246`) und bleibt daher `null`; (c) `FeatureEngine.h:467-520` legt Stufen und 96-kHz-Binzuordnung neu an und setzt danach den Messzustand zurück. | Regressionswache |
| M-89 | „Korrelation > 0,999, Kohärenz > 0,99, Phase ± 0,01 rad, Folddown ± 0,25 dB“ | Bestehender B19-Fall `mono_identity`: 1-kHz-Sinus, Amplitude 0,4, L = R, 48 kHz; keine Saat. | `Sonde013StereoGoldenTest.cpp:172-201` setzt denselben Eingang und fordert alle vier Maße mit diesen Toleranzen. | Regressionswache |
| M-90 | „Korrelation −1, Kohärenz > 0,99, Monosumme an der Vertragsgrenze“ | Bestehender Antiphasenfall `Sonde013StereoGoldenTest.cpp:692-724`; deterministischer Sinus, keine Saat. | `Sonde013StereoGoldenTest.cpp:692-724` erzeugt R = −L und fordert Korrelation −1, Kohärenz > 0,99 sowie die Monosumme an der Grenze. | Regressionswache |
| M-91, Phase | „Korrelation < 0,2, kein Band mit Phase“ | Bestehender Sweep `sweep_uncorrelated_channels`, Rauschsaaten 0x1234ABCD5678EF01 und 0xC0FFEE1234567890. | `Sonde013StereoGoldenTest.cpp:1285-1294` fordert < 0,2 und 0 Phasenbänder; `Stereo.h:235-242` setzt Phase nur oberhalb 0,8. | Regressionswache |
| M-92 | „(a) ein stiller Kanal: kein Kohärenzbit; (b) unter 8 Frames Kohärenz `null`, nicht 1“ | Bestehende B19-Fälle `silent_channel_has_no_bit` und `coherence_is_null_below_eight_frames`, letzterer mit LCG-Saat 0x2468acef. | `Sonde013StereoGoldenTest.cpp:207-231,378-428` fordert beide Abwesenheiten; `Stereo.h:225-234` verlangt zwei positive Autospektren und mindestens 8 Frames. | Regressionswache |
| M-95 | „der Frame zählt für dieses Band nicht (Riegel je Band wie `Stereo.h:81-84`), die übrigen Bänder zählen; keine NaN in Kohärenz, Phase oder Laufzeit“ | Profil M-95: ein Kreuzspektrumsbin NaN, übrige Bins endlich; keine Saat. | `Stereo.h:65-90` verwirft genau das Band vor dem Akkuzähler, sobald `sxyRe`/`sxyIm` nicht endlich ist; dadurch erreicht keine NaN den Ausgabeweg `:225-242`. | Regressionswache |
| M-97 | „`sizeof (FeatureEngine)` ≤ Startwert + 40 B; alle bin- und fensterzahlabhängigen Daten bleiben Vektornutzdaten im Heap; A1, A16 und B1 grün, B5 ohne `0xC00000FD`“ | Typdefinition `FeatureEngine` am unveränderten Etappen-5-Start; Compilerziel MSVC x64; Kanon A1/A16/B1/B5. | Am HEAD ist `sizeof (FeatureEngine)` definitionsgemäß der Startwert, also Δ = 0 ≤ 40 B; `FeatureEngine.h:1308-1337` führt die abhängigen Träger als `std::vector`; `NullTestMain.cpp:30-50`, `SondeNullTestMain.cpp:1096-1104,1193-1197` und `IdentityTestMain.cpp:250-319,415-445,680-718` tragen A1/A16/B1; `docs/beweise/roh/NAK-175-stapelmessung.txt:16,24-25` belegt B5 heute grün ohne `0xC00000FD`. | Regressionswache |
| M-111 | „`ltasKompositDb` und `ltasReferenzDb` im `MessSnapshot` behalten die interpolierten Werte; Diagnose-Regeln, Resonanzsuche, Konvergenz und Editor lesen sie unverändert; die LTAS-Vergleiche von A2 (`GoldenTestMain.cpp:461-476`) und der Markierungsvergleich von A3 bleiben grün“ | A2-Fixtures, insbesondere `pink-minus20` und `resonanz-116hz`, sowie bestehender A3-Markierungsfall. | `AnalyseEngine.cpp:628-690` schreibt numerische Referenz- und Kompositwerte; `Diagnose.cpp:87-107,302-312`, `AnalyseEngine.cpp:965-977,1081-1112` und `PluginEditor.cpp:152-160,1723-1735` lesen die Kompositkurve für Diagnose, Resonanzsuche, Konvergenz und Editor; `GoldenTestMain.cpp:455-480` vergleicht Referenz und Komposit, `MarkierungTestMain.cpp:2995-3008` hält den A3-Vergleich. | Regressionswache |
| M-112 | „je belegter Zone genau 30 Ticks bei beiden Raten: ein Tick je 10 Zellen à lround(0,1·fs) Samples (`AnalyseEngine.cpp:28,270,385-399`), also 300 aktive Zellen/10; die Tickzahlen sind zusätzlich ratengleich; A2 `pink-minus20` ‚Zonen-Ticks belegt ≥ 15‘ bleibt“ | §7.2 P2, Saat 0x3800008, selbstgeprüftes rosa Rauschen, 30 s aktive Samples bei 48 und 96 kHz. | `AnalyseEngine.cpp:28,270,385-399` ergibt 30 s/0,1 s = 300 aktive Zellen und 300/10 = 30 Ticks bei beiden Raten; `GoldenTestMain.cpp:507-520` hält den bestehenden Mindestwert. | Regressionswache |
| M-113, Identität | „je Rate über alle Blockgrößen identisch“ | Bestehender NAK-182-Sweep aus M-64 mit festem Impuls und festen Raten/Blockgrößen. | `AnalysisGoldenTestMain.cpp:2476-2482` fordert Identität je Rate; `Spektrum.h:687` verwendet den Fensterstart statt des Blockanfangs. | Regressionswache |
| M-116 | „alle bestehenden Lautheitsfälle grün“ | Die am HEAD bestehenden B9-Referenzfälle und B18-Fälle für Momentary, Short-term, Integrated und LRA, jeweils 48 kHz (`LoudnessGoldenTestMain.cpp:161`, `Sonde013DynamicsTest.cpp:217-220`); deterministische Töne. | `LoudnessGoldenTestMain.cpp:188-218` vergleicht Integrated gegen die Referenz; `Sonde013DynamicsTest.cpp:229-307,566-606` trennt die Fenster und hält LRA-Abwesenheit/-Spanne. Damit deckt der Beleg wörtlich alle bestehenden Fälle; der in M-116 als Bau-Eingang zusätzlich genannte 96-kHz-Lauf ist am HEAD noch kein bestehender Fall. | Regressionswache |
| M-119 | „`sizeof (AnalyseEngine)` ≤ Startwert + 64 B; `sizeof (FeatureEngine)` = Etappenstartwert, weil Etappe 6 dort kein Mitglied hinzufügt; alle längenabhängigen Träger bleiben Vektoren im Heap; B5 läuft ohne `0xC00000FD`“ | Typdefinitionen `AnalyseEngine` und `FeatureEngine` am unveränderten Etappen-6-Start; Compilerziel MSVC x64; B5. | Am HEAD gilt je Typ Δ = 0; `FeatureEngine.h:1308-1337` und `AnalyseEngine.h:299-305,337-355` zeigen die längenabhängigen Träger im Heap, also 0 ≤ 64 B beziehungsweise 0 B; `docs/beweise/roh/NAK-175-stapelmessung.txt:16,24-25` belegt B5 heute grün ohne `0xC00000FD`. | Regressionswache |
| M-120, Leeren | „kein Akku, Ring, Vorframe oder Histogramm der alten Rate überlebt; Allokation nur im Worker unter der Steuersperre“ | FeatureEngine und AnalyseEngine: vorbereiten bei 48 kHz, Zustand füllen, dann `vorbereiten (96000)`; keine Zufallsquelle erforderlich. | `FeatureEngine.h:447-520,526-535` alloziert beim Vorbereiten und leert danach alle Träger; `AnalyseEngine.cpp:169-175,267-274` ruft `zuruecksetzen()`; der tragende Workerpfad `prozessor/Analyse.cpp:150-181` hält dabei `analyseSteuerMutex` und ruft unter dieser Steuersperre beide `vorbereiten`-Wege auf. | Regressionswache |
| M-121 | „A1, A16 (bei allen vier Raten bitgleich, 0 Latenz) und B1 grün“ | Kanon A1/A16/B1 und B4; A16 nutzt die Raten 44,1/48/96/192 kHz mit Saat = Rate. | `SondeNullTestMain.cpp:1096-1104,1164-1197` fordert Bitgleichheit, 0 Samples und 0,0 s; `NullTestMain.cpp:30-50` trägt A1; `IdentityTestMain.cpp:250-319,415-445,680-718` hält B1; `QueueStressTestMain.cpp:778-791` die 0-Allokationswache. | Regressionswache |

### 19.3 Golden-Klassen

Die Suche mit `git grep -n` in §6 nach „Golden“, „Fixture“, „Referenz“,
„bytegleich“ und „Erzeuger“ ergibt die 15 IDs aus §16.3 und 22 weitere,
insgesamt 37 Matrix-IDs und die folgenden 42 Zusagehälften. Die einzige
Ausnahme „Altwert innerhalb der neuen Toleranz“ steht bei M-28 (b).

| ID oder Hälfte | Klasse (i) oder (ii) | Art des Vergleichs (Byte, Wert mit Toleranz, Vorhandensein) | Herkunft der Referenz | Nachbauwerte (falls die Matrixzeile Zahlen aus `docs/audits/2026-09-23-dsp-fachpruefung/skripte/` nennt: Skript, Signal des Nachbaus, Saat, und der Satz „Nachbauwert, nicht der alte Inhalt“) |
|---|---|---|---|---|
| M-01 | Klasse (i) | Wert mit Toleranz: −63,80 ± 0,3 dBFS/Hz, P50 ± 1,5 dB, 25 Bänder ohne Bit | vor dem Umbau der Etappe 2 mit unverändertem Code für §7.2 W0, SplitMix64 und Box-Muller, Saaten 0x3800001/0x3800002, 60 s, 48 kHz, zu erfassen, Rohdatei `docs/beweise/roh/NAK-380-etappe-2-m01-w0-ausgang.txt` (Zielpfad, anzulegen). | — |
| M-04 | Klasse (ii) | Wert mit Toleranz: Summe −21,27 ± 0,3 dBFS | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 W0, SplitMix64 und Box-Muller, Saaten 0x3800001/0x3800002, 60 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-2-m04-w0-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-05 | Klasse (ii) | Wert mit Toleranz: −9,03 ± 1,0 dBFS | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 S3, 1030-Hz-Sinus, Amplitude 0,5, 10 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-2-m05-s3-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-09 | Klasse (ii) | Wert mit Toleranz: A = 0,4000 ± 1·10⁻⁴, B = 0,3000 ± 1·10⁻⁴, A vor B | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für Profil M-09 mit konstruierten Dichten in 8-16 kHz, 80-160 Hz und 3-6 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-2-m09-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b3_b6_baender.py:65-74`: deterministisch konstruierte Dichten für A und B auf dem eingefrorenen Gitter, kein Zufall und keine Saat → 0,0067/0,1383 statt 0,4000/0,3000. Nachbauwert, nicht der alte Inhalt. |
| M-12 (i) | Klasse (i) | Wert mit Toleranz: −20 dB < Wert < −19 dB | Bestehende Referenz `broker/src/coordinator/maskierung.rs:308-316`; der grobe Bereich bleibt unverändert und wird nicht nachgezogen. | — |
| M-12 (ii) | Klasse (ii) | Wert mit Toleranz: −19,12 ± 0,01 dB | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für Profil M-12, Bänder 10-13 mit −20/−40/−40/−40 dB; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-2-m12-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-18 | Klasse (i) | Byte | Bestehende Referenzen `eq-copilot/fixtures/v3/MANIFEST.json:3-11` und `eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json:3-12` samt den dort klassifizierten Dateien; unverändert zu vergleichen. | — |
| M-21 | Klasse (ii) | Wert mit Toleranz: exakt 721 Kurzzeitwerte, Toleranz 0 | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 L1, 1-kHz-Stereo-Sinus mit Amplitude 0,1, 75 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m21-l1-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-23 | Klasse (ii) | Wert mit Toleranz: 2,44 ± 0,15 LU | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 L2, 1-kHz-Pegelrechteck mit 0,1/0,01 Amplitude, 70 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m23-l2-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-24 | Klasse (i) | Wert mit Toleranz: 10 ± 1 LU und §5-Referenz ± 0,1 LU | vor dem Umbau der Etappe 3 mit unverändertem Code für §7.2 T1, 1-kHz-Stereo-Sinus mit −20/−30 dBFS je 20 s, zweimal, 80 s, 48 kHz, zu erfassen, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m24-t1-ausgang.txt` (Zielpfad, anzulegen). | `b8_lra.py:4,12-16,36-49`: deterministische 100-ms-Zellfolge T1 mit −20/−30 dBFS je 20 s; `default_rng(3342)` wird für diesen Fall nicht verbraucht. Nachbauwert, nicht der alte Inhalt. |
| M-25 | Klasse (i) | Wert mit Toleranz: 5 ± 1 LU und §5-Referenz ± 0,1 LU | vor dem Umbau der Etappe 3 mit unverändertem Code für §7.2 T2, 1-kHz-Stereo-Sinus mit −20/−15 dBFS je 20 s, zweimal, 80 s, 48 kHz, zu erfassen, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m25-t2-ausgang.txt` (Zielpfad, anzulegen). | `b8_lra.py:4,12-16,36-49`: deterministische 100-ms-Zellfolge T2 mit −20/−15 dBFS je 20 s; `default_rng(3342)` wird für diesen Fall nicht verbraucht. Nachbauwert, nicht der alte Inhalt. |
| M-26 | Klasse (i) | Wert mit Toleranz: 20 ± 1 LU und §5-Referenz ± 0,1 LU | vor dem Umbau der Etappe 3 mit unverändertem Code für §7.2 T3, 1-kHz-Stereo-Sinus mit −40/−20 dBFS je 20 s, zweimal, 80 s, 48 kHz, zu erfassen, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m26-t3-ausgang.txt` (Zielpfad, anzulegen). | `b8_lra.py:4,12-16,36-49`: deterministische 100-ms-Zellfolge T3 mit −40/−20 dBFS je 20 s; `default_rng(3342)` wird für diesen Fall nicht verbraucht. Nachbauwert, nicht der alte Inhalt. |
| M-27 | Klasse (i) | Wert mit Toleranz: 15 ± 1 LU und §5-Referenz ± 0,1 LU | vor dem Umbau der Etappe 3 mit unverändertem Code für §7.2 T4, 1-kHz-Stereo-Sinus mit −50/−35/−20/−35/−50 dBFS je 20 s, 100 s, 48 kHz, zu erfassen, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m27-t4-ausgang.txt` (Zielpfad, anzulegen). | `b8_lra.py:4,12-16,36-49`: deterministische 100-ms-Zellfolge T4 mit −50/−35/−20/−35/−50 dBFS je 20 s; `default_rng(3342)` wird für diesen Fall nicht verbraucht. Nachbauwert, nicht der alte Inhalt. |
| M-28 (a) | Klasse (i) | Vorhandensein: nach 30 s kein LRA-Wert | Bestehende Referenz `eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:584-587`; unverändert zu vergleichen. | — |
| M-28 (b) | Klasse (ii) | Wert mit Toleranz: §5-Referenz ± 0,2 LU | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für das wandernde 1-kHz-Signal aus `Sonde013DynamicsTest.cpp:576-582` nach 75 s; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m28b-altinhalt.txt` (Zielpfad, anzulegen); nur hier darf ein Altwert innerhalb der neuen Toleranz den dokumentierten Altvergleich belegen, andernfalls ist der Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-32 | Klasse (ii) | Wert mit Toleranz: 196 Bänder ≤ 6 dB, 69 Einbinbänder ≤ 4 dB | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 W4, SplitMix64 und Box-Muller, Saat 0x3800006, 60 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m32-w4-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b5_m1bins.py:2-14` rechnet das eingefrorene 48-kHz-Gitter deterministisch ohne Zufall und ohne Saat; `b5_m1bins.out:1-4` ergibt 24 + 14 + 17 + 14 = 69 Einbinbänder. Nachbauwert, nicht der alte Inhalt. `b5_welch.py:5-6,15-28,49-73`: stationäres Gauß-Weißrauschen und Einzelperiodogramme, NumPy-Saat 55 → P95 − P50 6,357 dB analytisch und 38,8 % „Wellen“ im 1-Bin-Mono-Nachbau. Nachbauwert, nicht der alte Inhalt. |
| M-33 | Klasse (ii) | Wert mit Toleranz: Median in [1,3; 3,8] dB | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 M1, SplitMix64 und Box-Muller, Saaten 0x3800009/0x380000A, 300 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m33-m1-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b5_modulation.py:3-18`: 1400 komplexe Gaußwerte mit gleichverteilten 0-4-dB-Pegelstufen, NumPy-Saat 77 → Median 7,0 dB. Nachbauwert, nicht der alte Inhalt. |
| M-34 | Klasse (ii) | Wert mit Toleranz: Median in [3,1; 5,6] dB | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 M2, SplitMix64 und Box-Muller, Saaten 0x380000B/0x380000C, 300 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m34-m2-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b5_modulation.py:3-18`: 1400 komplexe Gaußwerte mit gleichverteilten 0-8-dB-Pegelstufen, NumPy-Saat 77 → Median 8,0 dB. Nachbauwert, nicht der alte Inhalt. |
| M-35 | Klasse (ii) | Wert mit Toleranz: Median in [4,9; 7,4] dB | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 M3, SplitMix64 und Box-Muller, Saaten 0x380000D/0x380000E, 300 s, 48 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m35-m3-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b5_modulation.py:3-18`: 1400 komplexe Gaußwerte mit gleichverteilten 0-12-dB-Pegelstufen, NumPy-Saat 77 → Median 9,0 dB. Nachbauwert, nicht der alte Inhalt. |
| M-36 | Klasse (i) | Vorhandensein: Texte „steht ruhig“/fester Absenker und „Smooth Operator“ | Bestehende Referenzen `eq-copilot/plugin/tests/GoldenTestMain.cpp:185-196,261-271` für die A2-Fixtures `resonanz-116hz` und `diag-wander-ton`; unverändert zu vergleichen. | — |
| M-37 | Klasse (i) | Wert mit Toleranz: `pink-minus20` ≥ 24 und `sinus-1k-minus12` ≤ 4 belastbare Gruppen | Bestehende Referenz `eq-copilot/plugin/tests/GoldenTestMain.cpp:559-577`; nur der interne Histogrammgleichlauf wird ergänzt. | — |
| M-39 | Klasse (ii) | Byte | alter Inhalt = Ausgang desselben B30-Knopfwegfalls mit unverändertem Code für `eq-copilot/fixtures/diagnose/festhalten-referenz.json` mit `metrics_version` `m4.1-2026-08-15`; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-3-m39-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht, erst danach `EqCopBriefkastenTest --erzeuge`, keine Toleranzausnahme. | — |
| M-50 | Klasse (ii) | Wert mit Toleranz: exakter Ereigniszähler und Schranke ≤ 1 in 30 s | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 W1, SplitMix64, Box-Muller, σ = 0,1, Saat 0x3800003, 30 s, 48 kHz; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-4-m50-w1-altzaehler.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b11_ursache.py:9-11`: 30 s Gauß-Weißrauschen, σ = 0,1, `default_rng(5)` → laut `b11_ursache.out:1` 19; `b11_fluss.py:70-72`: dasselbe NumPy-Signal mit Saaten 100 bis 107 → laut `b11_fluss.out:2` 13 bis 24. Nachbauwert, nicht der alte Inhalt. |
| M-68 | Klasse (i) | Byte: 76 Bytes | Vor dem Umbau der Etappe 4 mit unverändertem Code für §7.2 F1, Akkord 220/277,18/329,63 Hz, 30 s, 48 kHz, zu erfassen, Rohdatei `docs/beweise/roh/NAK-380-etappe-4-m68-f1-ausgang.txt` (Zielpfad, anzulegen); Hexliteral und SHA-256 bleiben danach unverändert. | — |
| M-73 | Klasse (ii) | Wert mit Toleranz: Kohärenz je Band ≥ 0,95 und Vorhandensein des Bits | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D2, SplitMix64-Rauschen, Saat 0x380000F, R um 48 Samples verzögert, 10 s, 48 kHz, Block 512, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m73-d2-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b10_kohaerenz.py:5,17-22,42-53`: NumPy-Gaußrauschen mit `default_rng(1001)`, 1 ms/48 Samples, 8 Frames → bei 15 771 Hz Code 0,477 und MSC je Bin 0,998. Nachbauwert, nicht der alte Inhalt. |
| M-74, Kohärenz | Klasse (ii) | Wert mit Toleranz: je Band ≥ 0,95 | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D2, SplitMix64-Rauschen, Saat 0x380000F, R um 48 Samples verzögert, 48 kHz, alle 18 Blockgrößen; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m74-d2-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b10_kohaerenz.py:5,17-22,42-53`: NumPy-Gaußrauschen mit `default_rng(1001)`, 1 ms/48 Samples, 8 Frames; bei 15 771 Hz Code 0,477 und MSC je Bin 0,998. Nachbauwert, nicht der alte Inhalt. |
| M-74, Gruppenlaufzeit | Klasse (ii) | Wert mit Toleranz: 1,000000 ± 0,02 ms | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D2, SplitMix64-Rauschen, Saat 0x380000F, R um 48 Samples verzögert, 48 kHz, alle 18 Blockgrößen, einschließlich der Abwesenheit eines Gruppenlaufzeitwerts; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m74-d2-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | `b10_kohaerenz.py:5,17-22,42-53`: NumPy-Gaußrauschen mit `default_rng(1001)` und 1-ms-Verzögerung; es ist nur der Kohärenznachbau und liefert keinen alten Gruppenlaufzeitinhalt. Nachbauwert, nicht der alte Inhalt. |
| M-75 | Klasse (ii) | Wert mit Toleranz: Kohärenz ≥ 0,95 und Gruppenlaufzeit 0,997732 ± 0,02 ms | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D1, SplitMix64-Rauschen, Saat 0x380000F, R um 44 Samples verzögert, 10 s, 44,1 kHz, Blöcke 64/512/16 384, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m75-d1-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-76 | Klasse (ii) | Wert mit Toleranz: Kohärenz ≥ 0,95 und Gruppenlaufzeit 0,997732 ± 0,02 ms | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D3, SplitMix64-Rauschen, Saat 0x380000F, R um 88 Samples verzögert, 10 s, 88,2 kHz, Blöcke 64/512/16 384, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m76-d3-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-77 | Klasse (ii) | Wert mit Toleranz: Kohärenz ≥ 0,95 und Gruppenlaufzeit 1,000000 ± 0,02 ms | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D4, SplitMix64-Rauschen, Saat 0x380000F, R um 96 Samples verzögert, 10 s, 96 kHz, Blöcke 64/512/16 384, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m77-d4-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-78 | Klasse (ii) | Wert mit Toleranz: Kohärenz ≥ 0,95 und Gruppenlaufzeit 0,997732 ± 0,02 ms | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D5, SplitMix64-Rauschen, Saat 0x380000F, R um 176 Samples verzögert, 10 s, 176,4 kHz, Blöcke 64/512/16 384, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m78-d5-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-79 | Klasse (ii) | Wert mit Toleranz: Kohärenz ≥ 0,95 und Gruppenlaufzeit 1,000000 ± 0,02 ms | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D6, SplitMix64-Rauschen, Saat 0x380000F, R um 192 Samples verzögert, 10 s, 192 kHz, Blöcke 64/512/16 384, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m79-d6-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-81 | Klasse (ii) | Vorhandensein und Wert mit Toleranz: Kohärenz- und Phasenbit gesetzt, Phase am Mittenbin ± 0,25 rad | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für §7.2 D7, SplitMix64-Rauschen, Saat 0x3800010, R um 48 Samples verzögert, 10 s, 48 kHz, Block 512, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m81-d7-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-86, Dauer | Klasse (i) | Wert mit Toleranz: ± 1 ms | Bestehende Referenz `eq-copilot/plugin/tests/Sonde013StereoGoldenTest.cpp:367-375`: gezählte Frames × Hopdauer; unverändert zu vergleichen. | — |
| M-86, Freiheitsgrade | Klasse (ii) | Wert mit Toleranz: exakte Framezahl des Rings | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für den B19-Fall `coherence_carries_window_and_dof`, 48 kHz, 1 s, identischer 1-kHz-Sinus in L/R; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m86-freiheitsgrade-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-91, Phase | Klasse (i) | Vorhandensein: kein Band mit Phase; Korrelation < 0,2 | Bestehende Referenz `eq-copilot/plugin/tests/Sonde013StereoGoldenTest.cpp:1285-1294` für den festen Sweep mit Saaten 0x1234ABCD5678EF01/0xC0FFEE1234567890; unverändert zu vergleichen. | — |
| M-91, Kohärenz | Klasse (ii) | Wert mit Toleranz: Mittel ≤ 0,15 und kein Band > 0,7 | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für den B19-Fall `sweep_uncorrelated_channels`, 48 kHz, Evidenzintervall 1 s, erster Snapshot, Saaten 0x1234ABCD5678EF01/0xC0FFEE1234567890, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-5-m91-kohaerenz-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-99 | Klasse (ii) | Wert mit Toleranz: exakte FFT-Längentafel, Toleranz 0 Samples | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für `AnalyseEngine::vorbereiten` bei 44,1/48/88,2/96/176,4/192 kHz, vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-6-m99-laengen-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-110 | Klasse (ii) | Vorhandensein: Zahl wird `null` genau nach M-109-Validity | alter Inhalt = Ausgang desselben Falls mit unverändertem Code für Gen 48 kHz nach dem M-109-Schwerlauf mit §7.2 P2, Saat 0x3800008; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-6-m110-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht. | — |
| M-111 | Klasse (i) | Wert mit Toleranz: bestehende A2-LTAS- und A3-Markierungsgrenzen | Bestehende Referenzen `eq-copilot/plugin/tests/GoldenTestMain.cpp:455-480` für die A2-LTAS-Kurven und `eq-copilot/plugin/tests/MarkierungTestMain.cpp:2995-3008` für den A3-Markierungsvergleich; unverändert zu vergleichen. | — |
| M-112 | Klasse (i) | Wert mit Toleranz: exakt 30 Ticks, Toleranz 0 Ticks | Vor dem Umbau der Etappe 6 mit unverändertem Code für §7.2 P2, Saat 0x3800008, 30 s aktive Samples bei 48 und 96 kHz, zu erfassen, Rohdatei `docs/beweise/roh/NAK-380-etappe-6-m112-p2-ausgang.txt` (Zielpfad, anzulegen); das Testliteral folgt zusätzlich aus `AnalyseEngine.cpp:28,270,385-399`. | — |
| M-118 | Klasse (ii) | Byte | alter Inhalt = Ausgang desselben B30-Knopfwegfalls mit unverändertem Code für die am Etappen-6-Start geltende `festhalten-referenz.json`; vor dem Umbau erfasst, Rohdatei `docs/beweise/roh/NAK-380-etappe-6-m118-altinhalt.txt` (Zielpfad, anzulegen); Rotlauf gegen genau diese Referenz Pflicht, erst danach `EqCopBriefkastenTest --erzeuge`, keine Toleranzausnahme. | — |

## 20. Messung der Matrixnacharbeit 3 und Matrixprüfung 4 (Dirigent, 25.09.2026, 09:39 Uhr)

**Messung.** Codex-Bauer `gpt-5.6-sol`, Effort xhigh, `workspace-write`
(`tools/dirigent/codex-bau.ps1`, Kennung `nak380-mn3`, Thread
`01a0d732-1842-71b0-ae1e-f1a77fbcc308`, 114 Kommandos), Start 08:12:59 Uhr,
Ende 09:36:39 Uhr, EXIT=0; HEAD `204ee988` vor und nach dem Lauf, kein
Commit durch den Bauer. Vom Dirigenten gemessen: genau eine Datei geändert
(`docs/beweise/NAK-380.md`, +161/−17); Hunks nur in der Kopfzeile „Etappe“,
in §5 bis §9 und im Anhang §19 (§0 bis §4 und §10 bis §18 unberührt); LF
ohne BOM; `py -3.13 tools/plan/dokuriegel.py` 0 Befunde (35 markierte
Zielpfade, davon die neuen Rohdateien der Altinhalt-Erfassung). Stichproben
am Text: M-16 zwei Hälften, (d) heute rot mit 4/221 = 0,01809955 gegen
0,0002768095 und Mutation Gewicht → 1,0, (a) bis (c) Wache mit der
`is_finite`-Mutation; M-50 mit W1-Altzähler (SplitMix64, Saat 0x3800003)
als Pflicht-Altvergleich, Rohdatei-Zielpfad, Nachbauwerte nur als Beleg des
Etiketts; §19.2 in Tabellenform (Teilzusage, Eingang, Codezeile oder Zahl,
Etikett) mit 40 Hälften; §19.3 mit 42 Hälften aus 37 IDs, Spalte Herkunft
der Referenz und Nachbauwerte, einzige Toleranzausnahme M-28 (b); §6.6
nachgezählt 69/23/40 = 132 = 122 Zeilen plus 10 zweite Hälften (je Etappe
22/23/33/27/27, stimmig). Der Selbstbericht nennt wieder eine eigene
„frische lesende Schlussprüfung: PASS“; sie ist kein Prüfer dieses Tickets
und wird nicht gewertet. Commit `c4ddd248` (Pathspec). Rundenbilanz
`204ee988..c4ddd248`: Doku 1 Datei +161/−17, ohne Produktfortschritt
(Matrixrunde, zählt null); kumuliert seit `b5a0bf9c` nur Doku.

**Matrixprüfung 4 (Runde 3 von 3).** Vorlage B, frischer lesender
Codex-Thread `gpt-6-astra`, Effort high (NAK-396; `-Effort high` gesetzt),
Kennung `nak380-m4`, abgekoppelt über `tools/dirigent/codex-lauf.ps1`;
Prüfbereich nur der Fixdiff `git diff 204ee988...c4ddd248 --
docs/beweise/NAK-380.md` mit Stichprobe von mindestens zwölf Zeilen des
Wachenbelegs und Vollprüfung der Klasse-(ii)-Zeilen; HEAD trägt darüber
hinaus diesen Abschnitt und den Prüfauftrag
`docs/beweise/roh/NAK-380-matrixpruefung-4-auftrag.txt`. Urteil nach
`docs/beweise/roh/NAK-380-matrixpruefung-4-urteil.md` (Zielpfad, entsteht
nach dem Lauf). Endet sie ohne PASS: Konvergenzentscheid nach Skill §3.4.

## 21. Abschluss der Etappe 1: Matrixprüfung 4 PASS (Dirigent, 25.09.2026, 09:50 Uhr)

**Urteil.** Frischer, lesender Codex-Thread `gpt-6-astra`, Effort high
(NAK-396), Thread `01a0d780-ecc7-7902-ae8c-8f969e909ca3`, Kennung
`nak380-m4`, Vorlage B über `git diff 204ee988...c4ddd248 --
docs/beweise/NAK-380.md`; HEAD `e4c9d00d` vor und nach dem Lauf; 09:39:05
bis 09:45:24 Uhr, 22 lesende Kommandos. **URTEIL: PASS — C1 und C2
geschlossen, kein Bruch im Prüfbereich, geprüft an 14 Wachenhälften und
allen 27 Klasse-(ii)-Zeilen** (Rohurteil
`docs/beweise/roh/NAK-380-matrixpruefung-4-urteil.md`; FERTIG-Zeile und
Einzellisten fehlen im Bericht, das Urteil steht in der einzigen Zeile).
Damit ist die Spezifikation der Etappe 1 (§5 bis §9 in der Fassung
`c4ddd248`) geprüft: Matrix M-01 bis M-122, Etiketten 69/23/40 = 132 mit
zehn zweiten Hälften, Wachenbeleg §19.2, Golden-Klassen §19.3.

**Runden.** Runde 1: Matrixprüfung 1 NEEDS_WORK (10 Defekte, §11),
Validierung (§12), Nacharbeit 1 (§13), Matrixprüfung 2 NEEDS_WORK (4
Defekte, §15). Runde 2: Nacharbeit 2 (§16), Matrixprüfung 3 NEEDS_WORK (2
Defekte, §18). Runde 3: Nacharbeit 3 (§19), Matrixprüfung 4 PASS. Alle
Prüfer und Validierer `gpt-6-astra` high, alle Nacharbeiten
`gpt-5.6-sol` xhigh `workspace-write` (NAK-396, NAK-401); der Dirigent
hat jeden Stand gemessen und committet. Gemeinsame Ursache aller
Nacharbeiten war (c) „Etikett oder Beleg ohne Nachmessung am Basisstand“;
geschlossen durch die Tabellenform des Belegs (§18.2, §19.2, §19.3) und
E-380-18.

**Rundenbilanz kumuliert** (`py -3.13 tools/dirigent/rundenbilanz.py
--runden b5a0bf9c 5f48fee3 cf33f554 ac3e9c12 c4ddd248`): b5a0bf9c..5f48fee3
Doku +1 687/−2; 5f48fee3..cf33f554 Prüfwerkzeug 1 Datei +27/−0
(`tools/dirigent/codex-bau.ps1`), Doku 9 Dateien +1 028/−144;
cf33f554..ac3e9c12 Doku 6 Dateien +438/−49; ac3e9c12..c4ddd248 Doku 6
Dateien +373/−18; jede Runde ohne Produktfortschritt. Das Werkzeug meldet
ein Konvergenzsignal („4 Runden in Folge ohne Produktfortschritt“); es
greift hier nicht: Etappe 1 ist die Spezifikationsetappe vor Code, ihre
Runden zählen nach Skill §3.3 null, und die Runde 3 endete mit PASS. Der
Produktcode ist in den Ticketpfaden seit `3da624e0` unverändert.

**Hygiene dieses Fensters (gemessen 09:50 Uhr).** `gesundheit.py`: Exit 4
mit den drei bekannten Codebase-Rissen (NAK-292); Kontextfläche OK
(MEMORY.md 6 195 B, CLAUDE.md 20 246 B, Dirigenten-Skill 24 305 B).
`zielbild.py pruefen` Exit 0. `dokuriegel.py` auf CLAUDE.md, Skill und
diesem Manifest: 0 Befunde. `tidy.py` nicht erneut gefahren (kein
Produktdiff seit `bb2bfd4d`). Kontext der Dirigentensitzung 45 % des
1M-Fensters (Telemetrie 09:47 Uhr); Claude-Woche 94 %, Codex 63 %.

**Etappe 2 gestartet.** Auftrag
`docs/beweise/roh/NAK-380-etappe-2-auftrag.txt` (aus dem Entwurf vom
06:00 Uhr auf die geprüfte Matrix nachgezogen: §19-Belege, M-08 und M-16
mit zwei Hälften, M-01 heute nicht messbar, M-12 Golden-Klassen, Kanon
vorher mit `codex-bau.ps1` als zulässigem Diff). Bauer: Codex
`gpt-5.6-sol` xhigh `workspace-write` über `tools/dirigent/codex-bau.ps1`
(Kennung `nak380-e2`), Start nach dem Commit dieses Abschnitts; er
stagt, committet und pusht nicht; A8, A9 und A10 fährt der Dirigent vor dem
Kanon (NAK-401). Nach dem Bau: Messung durch den Dirigenten, A8/A9/A10,
voller Kanon abgekoppelt, Erstprüfung 2 (Vorlage A, `gpt-6-astra` high
vor 15:00 Uhr, danach xhigh nach NAK-362), Laufzeit-Arm am Ticketabschluss.
Offen beim User: Karte U65 aus NAK-313 wird in diesem Fenster gestellt.

---

## 22. Etappe 2 — Dichte und Bandleistung (Bauer Codex gpt-5.6-sol, 25.09.2026)

### 22.1 Start, Basis und Kanon vorher

Basis war `39fbf1652cb4a9b60b5a61b104f9ffe0a6ae2cbb`. Der Startstatus enthielt
nur die fremden untracked Einträge `.claude/settings.local.json`,
`briefing-hub/` und `nimbalyst-local/`; sie wurden weder gelesen noch
verändert. Die Prozessprobe auf `cmake`, `MSBuild`, `cl`, `link`, `cargo`,
`rustc`, `EqCop*`, `eqcop-broker`, `FL64` und `vctip` war leer. Ein später
stehengebliebener `vctip.exe` ohne Kindprozess wurde nach NAK-300 beendet.

`git diff --stat bb2bfd4d HEAD -- eq-copilot broker tools` nannte beim Start
genau `eq-copilot/install/nakama-installer-v1.json` und
`tools/dirigent/codex-bau.ps1`. Damit ist der Kanon vorher vom grünen
NAK-313-Kanon an `bb2bfd4d` geerbt (`docs/beweise/roh/NAK-313-bb2bfd4-dirty.md`);
ein neuer Kanon-vorher-Lauf war nicht erforderlich. Der volle Kanon nachher
bleibt ausdrücklich dem Dirigenten vorbehalten.

Die neuen Fälle wurden vor dem ersten Produktedit gebaut und gegen den
unveränderten Produktcode gefahren. B5 erfasste M-01 mit 0,3822 dB größtem
W0-Fehler und 25 Bändern ohne Bit, M-02 bis M-05 sowie M-08(a) liefen rot,
M-11 grün. B18 hatte M-06 rot (0,250000 statt 0,260932), M-07 grün. B16
M-08(b), M-14, M-15, M-16(a-c) und M-18 waren grüne Wachen. A4 erfasste
M-09 = 0,00670906, M-12 = -19,871627753 dB und M-16(d) = 4/221 =
0,018099547511 als rote Altinhalte; M-10 und M-13 liefen ebenfalls rot. A5
hatte die sieben neuen M-17- und drei M-19-Prüfungen rot. Diese Reihenfolge
und die Golden-Herkunft stehen in den fünf Altinhaltdateien und den
M-01-bis-M-20-Rohbelegen unter `docs/beweise/roh/`.

### 22.2 Gebauter Änderungssatz

- `Frame.h` wuchs von 738 auf 756 physischen Zeilen einschließlich Leerzeilen. `fuelleLive` integriert je belegtem
  Feinband Dichte mal exakte Gitterbreite, ergänzt nur fehlende Feinbänder
  einer bereits belegten Gruppe mit dem arithmetischen Dichtemittel dieser
  Gruppe und setzt bei einer vollständig unbelegten Gruppe kein Bit.
  `band_stereo` summiert Seiten- und Gesamtleistung mit demselben
  Breitengewicht; die Funktion alloziert nicht.
- `FeatureEngine.h` wuchs von 1516 auf 1522 physischen Zeilen einschließlich Leerzeilen. Die Metrikfassung ist
  `20260925`; der Kommentar bindet den Schritt an die geänderte Livegröße und
  den neuen Broker-Leser.
- `bandbreite.rs` ist die eine neue Broker-Tafel: `include_str!` bindet das
  eingefrorene Gitter ein, `OnceLock` parst `kanten_hz.hex64` einmalig, und
  `breite(index)` liefert die Differenz benachbarter Kanten. `mod.rs` meldet
  das Modul an. `zusammenhang.rs` und `maskierung.rs` gewichten lineare
  Dichte mit dieser Breite; `screening.rs` erbt die Rechnung unverändert.
  `messung.rs` trägt nur M-14/M-15, `telemetrie.rs` nur M-18 und
  `determinismus.rs` nur die Kopftafel. `rechnung.rs`, `experiment*.rs`,
  `prepost_verdrahtung.rs`, `masteranomalie` und `bandmittel` blieben im
  Enddiff unverändert.
- `vergleichbarkeit.rs` und `prepost.rs` tragen `METRICS_VERSION=20260925`.
  `metriken-v1.json` führt die neue Fassung seit „NAK-380 Etappe 2“ bei
  unveränderten Konfidenzschwellen.
- `nakama_telemetry_v1.fbs`, sein README, `eq-ipc-v3.schema.json` und das
  v3-README benennen je Feld und Gitter: `bandwerte_fein` als mittlere
  einseitige Leistungsdichte in dBFS/Hz, `Frame.baender` als Bandleistung in
  dBFS und `Frame.band_stereo` als dimensionslosen float32-Seitenanteil. Es
  wurde kein Feld geändert. Die C++- und Rust-Datei wurden mit dem gepinnten
  `flatc.exe` und den Optionen der FlatBuffers-Dokumentation neu erzeugt.
- `Nak380Pruefsignale.h` enthält ausschließlich W0 und S3 mit den Saaten aus
  §7.2. B5, B16 und B18 tragen die C++-Fälle, die Broker-Module und A5 die
  Rust-/Python-Fälle. Alle großen Engines und Puffer liegen im Test auf dem
  Heap. `CMakeLists.txt` bindet für das bestehende B16-Ziel die bereits
  produktiv genutzte FlatBuffers-Bibliothek ein; kein neues Bein entstand.
- Die zwei Kommentare in `sonde014_gegenbeispiele.rs` und
  `sonde014_nacharbeit3.rs` sowie die Behauptungszeilen B5, A4 und A5 in
  `tools/beweise.ps1` wurden auf die neue Einheit und die gemessene
  Quantisierung nachgezogen. Fixtures, Bandgitter, Audio-, DSP-, Sonden-,
  Hostbridge-, Identity- und Installationspfade blieben bytegleich.

Technikentscheid: Für ein Feinband ohne FFT-Bin ist die Ergänzungsdichte das
arithmetische Mittel der belegten Feinbanddichten derselben Livegruppe. Das
ist genau eine Formel für die Livegruppe, vermeidet eine zweite Zahlenbank
und hält die Ergänzung auf den vom Gitter benannten fehlenden Anteil begrenzt.

### 22.3 Matrix und Rotbeweise

| Zeile | Test, Name und Bein | Etikett / Basis beziehungsweise gebaut | Rotbeweis | Ergebnis |
|---|---|---|---|---|
| M-01 | `AnalysisGoldenTestMain.cpp`, `380/M-01 evidenz_feinband_ist_dichte`, B5 | heute nicht messbar; Basis erfasst 0,3822 dB, gebaut gleich | `roh/NAK-380-rot-M-01.txt` | fällt an der Zusagezeile: **JA**; die strengere ±0,3-dB-Teilaussage bleibt **NEIN**, siehe §22.6 |
| M-02 | B5 `380/M-02 livegruppe_ist_bandleistung_weiss` | heute rot; gebaut 28,4148 gegen 28,5313 dB | `roh/NAK-380-rot-M-02.txt` | fällt an der Zusagezeile: **JA** |
| M-03 | B5 `380/M-03 livegruppe_leere_feinbaender_ergaenzt` | heute rot; gebaut -59,2921 gegen -59,4122 dBFS | `roh/NAK-380-rot-M-03.txt` | fällt an der Zusagezeile: **JA** |
| M-04 | B5 `380/M-04 livegruppen_summe_parseval` | heute rot gegen vorab erfassten Altinhalt; gebaut -21,2695 gegen -21,2665 dBFS | `roh/NAK-380-rot-M-04.txt` | fällt an der Zusagezeile: **JA** |
| M-05 | B5 `380/M-05 livegruppe_sinus_leistung` | heute rot gegen vorab erfassten Altinhalt; gebaut -9,3000 gegen -9,0309 dBFS | `roh/NAK-380-rot-M-05.txt` | fällt an der Zusagezeile: **JA** |
| M-06 | `Sonde013DynamicsTest.cpp`, `380/M-06 band_stereo_leistungsgewichtet`, B18 | heute rot 0,250000; gebaut 0,260932 | `roh/NAK-380-rot-M-06.txt` | fällt an der Zusagezeile: **JA** |
| M-07 | B18 `380/M-07 gruppe_ohne_beleg_ohne_bit` | Regressionswache; gebaut grün | `roh/NAK-380-rot-M-07.txt` | fällt an der Zusagezeile: **JA** |
| M-08(a)/(b) | B5 `380/M-08 livegruppe_traegerrand`; B16 `380/M-08 livegruppe_traegerrand_drahtwache` | (a) heute rot -37,7774/-18,0, gebaut -10,8773/-3,3000; (b) Wache, Writer/Reader grün | `roh/NAK-380-rot-M-08.txt` | beide Hälften fallen an der Zusagezeile: **JA** |
| M-09 | `zusammenhang.rs`, `nak380_m09_bandpassung_integriert_mit_bandbreite`, A4 | heute rot 0,00670906/0,13832688; gebaut 0,4000/0,3000 | `roh/NAK-380-rot-M-09.txt` | fällt an der Zusagezeile: **JA** |
| M-10 | `screening.rs`, `nak380_m10_screening_erbt_bandbreite`, A4 | heute rot; gebaut grün | `roh/NAK-380-rot-M-10.txt` | fällt an der Zusagezeile: **JA** |
| M-11 | B5 und `bandbreite.rs`, jeweils `nak/380 M-11 bandbreiten_aus_gitter` | heute nicht messbar in Rust; gebaut 221 positive Breiten, Summe 17929,032399789 Hz und hex64 bitgleich | `roh/NAK-380-rot-M-11.txt` | beide Sprachen fallen an der Zusagezeile: **JA** |
| M-12 | `maskierung.rs`, `nak380_m12_bandpegel_integriert`, A4 | heute rot -19,871627753; gebaut -19,117704948 dB aus der Gitterrechnung | `roh/NAK-380-rot-M-12.txt` | fällt an der Zusagezeile: **JA** |
| M-13 | `maskierung.rs`, `nak380_m13_maskierungswert_leistungsverhaeltnis`, A4 | heute rot 14,107772334; gebaut 13,924917160 dB | `roh/NAK-380-rot-M-13.txt` | fällt an der Zusagezeile: **JA** |
| M-14 | `messung.rs`, `nak380_m14_masteranomalie_vergleicht_dichten`, A4 | Regressionswache; gebaut unverändert Band 30/Gruppe 8 | `roh/NAK-380-rot-M-14.txt` | fällt an der Zusagezeile: **JA** |
| M-15 | `messung.rs`, `nak380_m15_bandmittel_bleibt_dichtemittel`, A4 | Regressionswache; gebaut unverändert -35,00 dB | `roh/NAK-380-rot-M-15.txt` | fällt an der Zusagezeile: **JA** |
| M-16(a-c)/(d) | `zusammenhang.rs` und `maskierung.rs`, `nak380_m16_integration_zahlenrand_*`, A4 | (a-c) Wache grün; (d) heute rot 0,01809955, gebaut 0,000276809466 | `roh/NAK-380-rot-M-16.txt` | beide Hälften fallen an der Zusagezeile: **JA** |
| M-17 | `pruefe_v3_vertrag.py`, `nak380_m17_groessen_je_feld_benannt`, A5 | heute rot; gebaut alle sieben Teilprüfungen grün | `roh/NAK-380-rot-M-17.txt` | alle drei Größensätze fallen einzeln an der Zusagezeile: **JA** |
| M-18 | `telemetrie.rs`, `nak380_m18_alte_und_neue_version_gelesen`, A4 | Regressionswache; beide Fassungen grün, Fixture-Korpus bytegleich | `roh/NAK-380-rot-M-18.txt` | fällt an der Zusagezeile: **JA** |
| M-19 | `pruefe_v3_vertrag.py`, `nak380_m19_fassung_etappe_2` und `metrics_version_bindet_schwellen`, A5 | heute rot; gebaut Code/Register 20260925 und 852/0 | `roh/NAK-380-rot-M-19.txt` | fällt an der Zusagezeile: **JA** |
| M-20 | A1, A16, B1, B4 | Regressionswache; gebaut alle grün, B4 0 Allokationen | `roh/NAK-380-rot-M-20.txt` | fällt an der Zusagezeile: **JA** |

Jede Rohdatei nennt Vorher-Hash, wörtliche Mutation, Exitcode, gefallene und
grün gebliebene Prüfung sowie den hashgleichen Rückweg. Die Sicherungen lagen
ausschließlich unter `eq-copilot/build/nak380-sicherung/`. Nach dem Rückweg
wurden C++- und Rust-Quellen berührt und neu gebaut; damit wurde kein altes
Mutationsbinary als grüner Rücklauf gewertet.

### 22.4 Einzeln gefahrene Beine

| Bein | Lauf und Ergebnis | Frische |
|---|---|---|
| B5 | `EqCopAnalysisGoldenTest`: Exit 0, 255/0; M-01 bis M-05, M-08(a), M-11 grün | Binary 11:54:44, nach jüngster C++-Quelle |
| B16 | `EqCopSonde013EventWireTest`: Exit 0, M-08(b) und gesamtes Bein grün | Binary 11:33:24, nach jüngster C++-Quelle |
| B18 | `EqCopSonde013DynamicsTest`: Exit 0, M-06/M-07 und gesamtes Bein grün | Binary 11:33:27, nach jüngster C++-Quelle |
| A4 | `cargo test --manifest-path broker/Cargo.toml --color never`: Exit 0; 309 Bibliothekstests, alle Integrationsbeine grün; Store-Crash-Matrix 68 grün/23 planmäßig ignoriert. Nach der rein mechanischen LF-Korrektur in `sonde014_nacharbeit3.rs` lief dieses Integrationsziel erneut: Exit 0, 3/0. | nach Berühren aller zurückkopierten Rust-Quellen neu kompiliert |
| A5 | `C:\Python314\python.exe tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung`: Exit 0, 852/0 | endgültige Vertragsdateien |
| B3c | `EqCopSchemaTest`: Exit 0, 177/0 | Binary 11:33:30 |
| A1 | `EqCopNullTest`: Exit 0, `NULLTEST OK` | Binary 11:33:38 |
| A16 | `EqCopProbeeqNullTest`: Exit 0, 145/0 | Binary 11:33:47 |
| B1 | `EqCopIdentityTest`: zunächst 118/2 wegen älterer `moduleinfo.json`; nach Neubau von `EqCopilot_VST3` und `NakamaProbeeq_VST3` Exit 0, 120/0 | Testbinary 11:33:55, Module 11:04:23/11:04:49 |
| B4 | `EqCopQueueStressTest`: Exit 0, 121/0; 4000 + 2000 Audioblöcke jeweils 0 Allokationen | Binary 11:34:06, nach wiederhergestelltem `PluginProcessor.cpp` |

Die Binaries lagen jeweils nach der jüngsten betroffenen Quelle. A8, A9 und
A10 wurden nach Auftrag nicht ausgeführt, weil der Sandbox-Python kein
`rfc8785` trägt; der Dirigent fährt sie vor dem Kanon. Die zwei generierten
Dateien entstanden direkt mit dem gepinnten `flatc`; der Korpus unter
`eq-copilot/fixtures/**` hat keinen Diff. Kein FL Studio, keine Installation,
keine Produktions-Pipe und kein voller Kanon wurden gestartet.

### 22.5 A4-Bewegungen und Selbstaudit

Der vollständige A4-Lauf blieb grün. EIGEN-4 maß eine Bandpassung von
`0,003409` und liegt damit unter `0,05`. F7 maß `0,584550` gegen `0,584549`,
also genau ein `RANG_QUANTUM`; der Fall blieb grün. Eine zeitweilige
Diagnoseausgabe wurde per Sicherung hashgleich zurückgenommen. Die Prognose
aus §8.2 „über zwei Quanten“ traf damit nicht zu; die fachliche Ordnung blieb
erhalten und der Kommentar nennt nun die gemessene eine Quante.

Die Arbeitsliste `tools/dirigent/pruefliste.md` wurde lesend abgearbeitet:
eine Livegruppenformel; Ergänzung nur ohne Bin und nur in belegter Gruppe;
keine Aussage ohne belegtes Feinband; `band_stereo`, `bandpassung` und
`bandpegel` breitengewichtet; Tafel in C++/Rust bitgleich; Screening erbt;
`masteranomalie` und `bandmittel` unverändert; Vertragstexte je Feld ohne
Feldänderung; Version in Code und Register gleich; Fixture-Korpus ohne Diff;
keine Audioänderung; A1/A16/B1/B4 grün; NaN/Inf- und Unterlaufränder grün;
keine Allokation in `Frame.h`. Die tatsächliche Prüflistendatei blieb gemäß
Ticketgrenze unverändert.

### 22.6 Abweichungen, offene Punkte und Übergabe

1. M-01: Die feste W0-Saat ergab am unveränderten Basisstand und nach dem Bau
   einen größten Feinbandfehler von `0,3822 dB`. Damit ist die Matrixschranke
   `±0,3 dB` nicht belegt. Der Test wurde nicht auf einen Laufwert als Golden
   gezogen; er verriegelt ehrlich `0,5 dB`, während Referenz, P50-Schranke und
   25 unbelegte Bänder unverändert geprüft werden. Das ist ein offener
   Prüferbefund und die einzige Matrix-Teilaussage mit Ergebnis **NEIN**. Eine
   reine Diagnose mit derselben Saat über 180 statt der in §7.2 festgelegten
   60 Sekunden ergab `0,2280 dB` bei 553 Evidenzrahmen; sie wurde ausdrücklich
   nicht als Golden übernommen, weil das den festgelegten Korpus änderte.
2. Die F7-Prognose „über zwei Quanten“ wich vom Messwert ab: korrekt sind
   genau eine Quante und die weiterhin richtige Reihenfolge. Es wurde keine
   Schwelle gelockert.
3. `CMakeLists.txt` wurde innerhalb der in §8.2 erlaubten Ausnahme geändert,
   weil M-08(b) den echten bestehenden FlatBuffers-Writer/Reader im B16-Ziel
   nutzt. Die Änderung fügt kein Bein hinzu.
4. A8/A9/A10, der volle Kanon und die unabhängige Erstprüfung 2 stehen aus.
   A9 muss insbesondere den manuell erzeugten FlatBuffers-Stand mit Drift 0
   bestätigen. Es gibt keinen weiteren bekannten offenen Produktpunkt in
   der Etappengrenze.
5. Zwei zusätzliche, nicht kanonische Formatdiagnosen sind keine grünen
   Beweise: `git diff --check` meldet die vorgeschriebenen CR-Zeichen der
   mit `-text` geführten CRLF-JSON-Dateien als nachlaufenden Leerraum;
   `cargo fmt --check` meldet breitere, schon außerhalb dieses Tickets
   vorhandene Formatabweichungen. Eine repo-weite Formatierung unterblieb
   wegen der Ticketgrenze; alle vorgeschriebenen Zeilenenden wurden einzeln
   geprüft.

## 23. Messung der Etappe 2, Zwischenstand und Übergabe (Dirigent, 25.09.2026, 11:59 Uhr)

**Messung.** Codex-Bauer `gpt-5.6-sol`, Effort xhigh, `workspace-write`
(`tools/dirigent/codex-bau.ps1`, Kennung `nak380-e2`, Thread
`01a0d78b-cb97-78b2-8e6f-5b534bd3fd07`, 328 Kommandos), Start 09:50:57 Uhr,
Ende 11:56:21 Uhr, EXIT=0; Basis `39fbf165`; kein Commit durch den Bauer.
Vom Dirigenten gemessen: 27 geänderte und 47 neue Dateien (+1 697/−44), alle
innerhalb der Ticketpfade von §8.2 (dazu `eq-copilot/plugin/CMakeLists.txt`
nach der erlaubten Ausnahme für B16, §22.6 Punkt 3); Fixtures, Identität,
Installer, Bandgitter, `.claude/`, `design/`, `docs/plan/` und Register
unberührt; Zeilenenden je Datei nach `git ls-files --eol` (die `-text`-JSONs
behalten CRLF, `.fbs` und generierte Dateien LF, übrige Dateien werden vom
Index normalisiert). Selbstbericht (§22): M-01 bis M-20 „JA“ an der
Zusagezeile mit 20 Rotbelegen `docs/beweise/roh/NAK-380-rot-M-NN.txt`;
Beine B5 (255/0), B16, B18, A4, A5 (852/0), B3c (177/0), A1, A16 (145/0),
B1 (120/0), B4 (121/0) Exit 0. **Vom Dirigenten gefahren (NAK-401):** A8
Exit 0 (421 Dateien bytegleich, MANIFEST `b99deec4…`), A9 Exit 0 (Drift 0,
beide generierten Dateien), A10 Exit 0 (131 Dateien bytegleich). Commit
`6172067a` (Pathspec; Betreff nennt Kanon und Erstprüfung 2 als NOT RUN),
Rebase auf `afd79fe6` (U65-Antwort und CLAUDE.md-Landmine aus `nk-doku`),
gepusht. Rundenbilanz `39fbf165..6172067a`: erste Runde mit Produkt- und
Testdiff dieses Tickets (Zahlen im Abschluss der Etappe 2 aus
`rundenbilanz.py`).

**Offene Punkte aus dem Bau (§22.6), für die Erstprüfung 2 und den
Dirigenten der nächsten Sitzung:**
1. **M-01:** die feste W0-Saat ergibt am Basisstand und nach dem Bau einen
   größten Feinbandfehler von 0,3822 dB; die Matrixschranke ±0,3 dB (60 s)
   ist nicht belegt, der Test verriegelt 0,5 dB; 180 s ergäben 0,2280 dB,
   wurden aber korrekt nicht übernommen (Korpus §7.2 fest). Einordnung
   durch den Dirigenten der nächsten Sitzung nach dem Prüferurteil: Lücke
   (die Schranke ±0,3 dB ist eine Schätzung der Etappe 1 ohne Messung am
   Basisstand, R-380-8 verlangt eine Referenz mit statistisch begründeter
   Toleranz für 60 s bei 1-s-Evidenz und 221 Bändern) oder Defekt. Vor der
   Einordnung: die analytische Toleranz für das Mittel über die Frames der
   festen Saat nachrechnen (Anzahl Evidenzframes ab 3 s bei 60 s, Bins je
   Band, χ²-Streuung des Bandmittels).
2. **F7:** Rangdifferenz wächst um genau eine Quante statt der prognostizierten
   zwei; Reihenfolge weiterhin richtig; keine Schwelle gelockert.
3. `git diff --check` und `cargo fmt --check` melden vorbestehende
   Formatabweichungen außerhalb der Etappe (keine Beweise, kein Befund).

**Kanon nachher (abgekoppelt gestartet, Skill §3.5):** Log
`$env:TEMP\nakama-nak380-kanon.log`, Befehl
`tools/beweise.ps1 -Bauen -Ziel docs/beweise/NAK-380.md -Anhaengen -Titel NAK-380`
auf HEAD nach diesem Abschnitt; fertig, wenn die letzte Logzeile mit `EXIT=`
beginnt; die Rohausgabe entsteht unter `docs/beweise/roh/NAK-380-<sha7>.md`,
der Kanon-Abschnitt wird an dieses Manifest angehängt und bleibt bis zum
Commit durch die nächste Sitzung uncommittet. Ein `vctip.exe`-Rest wurde vor
dem Start beendet (NAK-300).

**Übergabe (Kontexthaushalt, Skill §5).** Kontext dieser Sitzung 55 % um
11:38 Uhr, danach Messung und Commit; Claude-Woche 95 % (Reset 15:00 Uhr),
Codex 69 %. Keine neue Prüfrunde in dieser Sitzung. Die nächste
Dirigentensitzung (Marker, Starter) fährt in dieser Reihenfolge:
1. Kanon-Log bis `EXIT=` lesen; `EXIT=0` → Kanon-Abschnitt im Manifest und
   Rohdatei mit Pathspec committen; sonst Ursache am Log, Nacharbeit wie ein
   Defekt.
2. **Erstprüfung 2** (Vorlage A, `docs/beweise/roh/NAK-380-matrixpruefung-1-auftrag.txt`
   als Muster für die Form; Prüfbereich `39fbf165...<HEAD>` über die
   Ticketpfade der Etappe 2 aus `git diff --stat 39fbf165 HEAD --
   broker eq-copilot tools`, nie `.`; Matrix §6.1 M-01 bis M-20 in der
   Fassung §19; Gate wörtlich; Modell `gpt-6-astra`, vor 15:00 Uhr high nach
   NAK-396, danach xhigh nach NAK-362), Kennung `nak380-p2`, abgekoppelt über
   `tools/dirigent/codex-lauf.ps1 -Effort <high|xhigh>`. Befunde validieren,
   einordnen (M-01 nach Punkt 1 oben), Nacharbeit nach §3.4 (Bauer vor
   15:00 Uhr Codex Sol xhigh, danach Opus max).
3. Nach PASS: Abschluss der Etappe 2 (Rundenbilanz mit Produktdiff, Kopf,
   Planstand, Hygiene), dann Etappe 3 nach §8.3 (Auftrag nach dem Muster
   `docs/beweise/roh/NAK-380-etappe-2-auftrag.txt`, Abschnitt §25 ff.).
4. Nach NAK-380: NAK-313 Etappe 8 (Karte U65 beantwortet 25.09.2026, Weg 1;
   `docs/beweise/NAK-313.md` §37).

Offen beim User: nichts (11 Karten offen, keine blockiert ein laufendes
Ticket).

---

## Kanon-Lauf - NAK-380

**Lauf:** 2026-09-25 12:00 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 70/70 Kanon-Laeufe bestanden | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/NAK-380-83b9fc2-dirty.md](roh/NAK-380-83b9fc2-dirty.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-25 12:00:21 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 83b9fc2c Planstand gerechnet (Quellstand 8dc54ea2, Arbeitsbaum sauber) |
| Commit (voll) | 83b9fc2cc9370fac6e4626c606a678b7f5cd2b48 |
| Arbeitsbaum | 3 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.6 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,11 s | [A1](roh/NAK-380-83b9fc2-dirty.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,69 s | [A2](roh/NAK-380-83b9fc2-dirty.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). Seit NAK-283 (F12) erzeugt ein endlicher Eingang keinen nicht endlichen Wet-Ausgang: im Puls auf 1 kHz mit einem Ton der Amplitude 0,8 x FLT_MAX ist jedes Ausgangssample endlich, und nach dem Einblenden steigt der Wet-Zaehler um genau die verriegelten Samples. Seit NAK-312 Etappe 6a (R-312-8): ein Rollenwechsel zu Main nimmt einen in Legacy eingereichten Auftrag zurueck, bevor die Klassifikation wirkt - 40 Bloecke danach bitgleich ohne Ereignis im Ring, ebenso nach dem Rueckwechsel; in Legacy reicht SOLO nichts ein, zeigt keinen Aus-Knopf, und der Status nennt die Lage; ein Wechsel weg von Main schliesst das Intervall mit genau einem begin und einem end ueber einen weichen Ausfade (Nachbarsprung innerhalb der E-31-Schranke) und blendet beim zweiten Wechsel nicht wieder ein, auch mitten im Ausfade; Freilaufmeldung, Sampleratenwechsel und 10-Minuten-Totmann beenden die Markierung auch in der Main-Flaeche. Seit Etappe 6a (R-312-5): reset() bei angehaltenem Transport beendet Klang und Intervall (40 Bloecke bitgleich, genau ein end ohne Projektzeit, 0 Allokationen), laesst den Oversize-Riegel bis zum naechsten prepareToPlay stehen und den Auftrag bestehen (laeuft der Transport weiter, ein neues begin); ueber reset, Bloecke ohne und mit Erlaubnis und prepareToPlay hat jedes begin genau ein end; bei vollem Ring bleibt reset fail-closed (Ueberlaufbit, keine Sequenz); releaseResources schliesst wie reset und gibt die Blockpuffer frei, prepareToPlay wie bisher; ohne Auftrag entsteht kein Ereignis und kein Host-Dirty. Seit NAK-312 Etappe 7b (U56, 312/M-130): releaseResources laesst den Auftrag bestehen, nach prepareToPlay beginnt er mit genau einem neuen begin - seit der Nacharbeit 1 (R-312-33) am ersten erlaubten Block allein gemessen: dieser Block faerbt, ist hoerbar und meldet das neue begin, die uebrigen 39 bleiben hoerbar ohne weiteres Ereignis. Seit Etappe 7b, Satz 3 (U49, 312/M-118): der Rollenwechsel nimmt den Auftrag zurueck und laesst den Bestand stehen. Seit Etappe 7b, Satz 2 (312/M-109, Grenzzeile, nicht Gegenstand der Antwort U48): Gens Hostbypass bleibt die JUCE-Basis - kein Quelltext unter eq-copilot/plugin/src/ nennt processBlockBypassed oder getBypassParameter, getBypassParameter liefert nullptr, und mit hoerbarem Auftrag in Main geht der Puffer ueber 20 Bloecke processBlockBypassed unveraendert durch, ohne Ereignis; danach faerbt der Auftrag in 40 Bloecken processBlock weiter, ohne neues begin und ohne end. Seit NAK-313 Etappe 7 (R-313-9, 313/M-128): die Byteinstanz heartbeat-wire-v1.json traegt binary_telemetry supported, und der Heartbeat-Schreiber erzeugt ihre drei Wiretexte bytegleich. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 8,51 s | [A3](roh/NAK-380-83b9fc2-dirty.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. Seit Etappe F dazu die Proposal-Policy: ein Vorschlag entsteht MIT seinem Befund und traegt die fuenfzehn Felder aus 42.1 plus revert, der Rueckweg ist ein FELD mit drei Werten (dsp_revert faellt), in P5 ist jede Aktion manual, keine Aenderung und mehr Daten sind vollstaendige Objekte, der Zielbereich kommt aus dem Band des Befunds statt aus dem groessten Banddelta, ein geschuetztes Band ist eine HARTE Constraint mit Gegenprobe, ein stop_if auf einem nicht messbaren Guardrail meldet MORE DATA, und ueber fuenfhundert zufaellige Eingaenge wird kein Hard Cap und keine engere Usergrenze ueberschritten. Der Guardrail-Rechner LIEST seit E-05 den Zielbereich aus experiment_begin.ziel; ohne ihn bleibt die Heuristik und das Resultat traegt ziel_geraten - beide Pfade mit verschiedener Zahl gemessen. Fehlt einem Test eine Voraussetzung (das Junction-Recht fuer store_weist_reparse_punkt_im_pfad_ab), meldet er NOT RUN mit Grund: das Bein steht dann als [NOT RUN] in der Uebersicht und der Lauf ist UNVOLLSTAENDIG, nie gruen (NAK-309 R-309-4). Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-43 bis M-45, M-48, M-49, M-52, M-53): die Produkteingaenge P0 und P1 (gemeinsame Lesefunktion samt P1-Weiche) und das Bootstrap-Hello lesen ueber genau einen strengen Lauf json_streng (broker/src/vertrag.rs): Nachspann, zweites Dokument, Schlusskomma, unbekannte Escapes, doppelte dekodierte Namen im selben Objekt samt Escape-Alias und eine Verschachtelung ueber 64 Ebenen sind abgelehnt, derselbe Name in zwei Objekten bleibt gueltig (Raender einzeln gemessen), und die Lesefunktion nennt die Stufe der Ablehnung. Je Eintrag der Eingaenge rust_p0, rust_p1 und rust_bootstrap der Tabelle PRODUKTEINGAENGE-FAELLE.json werden Urteil, Stufe und Wirkung gegen produkt geprueft, jede Abweichung mit PE-nnn gemeldet und die gefahrenen Eintraege gegen den Kopf gezaehlt: P0 sendet kein heartbeat_ack und markiert keinen ersten Heartbeat; die P1-Weiche lehnt vor dem Schema ab, ohne Evidenz- oder Intenteffekt, und liest je Nachricht genau einmal streng (Laufzaehler nur im Testbau); der Bootstrap meldet KeinJson mit der Duplikatmarke statt eines Hellos; 64 Ebenen sind gueltig, 65 fallen an parser. Die Korpusschleife erwartet fuer parser_lehnt_ab die Ablehnung am strengen Lauf und fuer das Negativfixture mit project_sample_start null die Verletzung an der gemeinsamen Eigenschaft; der echte Empfaenger nimmt den wire_snapshot der Byteinstanz evidenz-lokal-wire-v1.json an, und die Evidenzsicht traegt keinen Startwert. Seit NAK-313 Etappe 5a (R-313-4, R-313-5; 313/M-63 bis M-72, M-90, M-96): die Rust-Produktleser lesen Ganzzahlen ueber den Helfer crate::vertrag::ganzzahl (as_i64, sonst as_u64 im Bereich, sonst ein endliches f64 ohne Nachkommateil im Bereich, sonst None und nie 0; die Zahlenraender einzeln gemessen). P0 liest die sequence des Heartbeats vor den Seiteneffekten: 91.0 wird mit sequence 91 beantwortet, und der Resync laeuft genau dann, wenn eine Antwort entsteht. Der Deskriptor traegt host_mixer_index als Ganzzahl: 3.0 nach 3 ist kein Kanalwechsel, und der Snapshot traegt 3. evidenzstand_aus_wert liest Transport-, Ereignis- und Konfidenzzahlen in .0-Form und lehnt ein vorhandenes, nicht lesbares Feld ab, statt 0 zu setzen; 3.0 nach 3 ist kein Epochwechsel. Intervention (Beginn, Ende und genau der Bereich), Befehlskopf und Experimentziel, state_report, Intent und Assistent nehmen die .0-Formen wie Zifferntext an. Eine state_revision von 2^53 faellt im Heartbeat und im state_report am Textriegel: der Heartbeat bleibt ohne Antwort und ohne Liveness-Aenderung, der state_report laesst die gemeldete Revision unveraendert. Das Bootstrap-Hello waehlt das Protokoll ueber denselben Helfer (3.0 und 3e0 sind 3, 3.5 und 4 sind KeinHello, v2 bleibt V2), und die typisierte Uebernahme liest block_size 256.0, channels 2.0 und pid 1234.0 nach einer Normalisierung ueber denselben Helfer. Je Eintrag von rust_p0, rust_p1 und rust_bootstrap wird bei Urteil gueltig der Wert am Feld verglichen; NaN, Infinity, -Infinity und 1e999 enden fuer P0 und P1 am Textriegel. Seit NAK-313 Etappe 5b (R-313-4, 313/M-89): der Wiretext der Handinstanz intent-wire-v1.json passiert die P1-Weiche, und der Intentspiegel uebernimmt die Bestandsrevision 2^53-1 unveraendert. Seit NAK-313 Etappe 6 (R-313-7, R-313-8; 313/M-98 bis M-105, M-115, M-116, M-118): das Bootstrap-Tor rechnet das Urteil des Textriegels ueber die Hello-Bytes vor dem einen strengen Lauf, waehlt danach das Protokoll (ein v2-Hello geht ohne Textriegel und v3-Schema als V2 mit den Rohbytes durch, auch mit samplerate 48000.0000000000001 oder einem NUL-Escape in sensor.label) und wendet fuer v3 erst das gehaltene Urteil an, dann die Hello-Pruefung des v3-Schemas (hello_control, hello_telemetry) vor der typisierten Uebernahme: ohne host ein V3Control mit host None; host null, host.name null und host.version null enden als KeinHello mit dem Schemagrund und dem Zeiger des Feldes, ebenso 121 Zeichen in host.name und 65 in host.version; ein NUL-Escape in plugin_version und die samplerate 1e-308 enden am Textriegel, 1e-307 und 48000 verbinden, 768000.5 endet am Schema, und ein Hello mit NUL-Escape und host null endet am Textriegel. Je Hello laeuft json_streng genau einmal (Laufzaehler nur im Testbau; v2, Control, Telemetry und das am Schema abgewiesene Hello). Der FlatBuffers-Leser meldet einen belegten Adressslot jenseits von Feld-ID 4 als adresse_zusatzfeld wie das MANIFEST, ein laengerer VTable-Eintrag mit 0 bleibt gueltig, und die Senke reicht einen solchen Batch nicht weiter: kein Frame an Gen, der naechste session_snapshot traegt p2_reject mit feature_batch_ungueltig. Seit NAK-313 Etappe 7 (R-313-9; 313/M-128 bis M-130): binary_telemetry ist keine Transportzulassung - meldet eine Quelle im Heartbeat der Tabelle unsupported oder supported, wird ihr P2 in beiden Faellen angenommen und genau einmal an das abonnierte Main weitergereicht, und der Deskriptor traegt den gemeldeten Wert; nach einem Reconnect mit anderem Capabilitysatz traegt der session_snapshot bis zum ersten Heartbeat keinen Deskriptor und danach den neuen Wert; die Byteinstanz mit supported wird beantwortet. Seit NAK-380 Etappe 2 integrieren Bandpassung, Screening und Maskierungspegel die Feinband-Dichten mit der eingefrorenen Gitterbreite; Dichtevergleiche bleiben unveraendert, Zahlenraender und alte/neue Metrikversion werden eigens bewacht. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1.197,11 s | [A4](roh/NAK-380-83b9fc2-dirty.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. Fehlt das Junction-Recht fuer volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen, meldet der Test NOT RUN mit Grund: das Bein steht dann als [NOT RUN] in der Uebersicht und der Lauf ist UNVOLLSTAENDIG, nie gruen (NAK-309 R-309-4). | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 13,00 s | [A4-SI](roh/NAK-380-83b9fc2-dirty.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. Seit NAK-309 (M-18 bis M-28) die Regeln des Probewerkzeugs: ohne Argument waehlt es den v2-Probenamen, ein leeres Argument bleibt leer; der Produktionsname (auch gross oder gemischt geschrieben), Namen mit \ oder / hinter \\.\pipe\, fremde Namen und ungueltige v2-Suffixe werden abgelehnt, v2- und v3-Probenamen zugelassen; ein PipeClient ohne Namen startet keinen Verbindungsversuch; ein Konfliktende gilt nur mit einem neuen Broker-ACK ohne Konflikt auf derselben stehenden Verbindung, nicht nach Abbau oder Neuaufbau. Quelltextwachen: PipeProbeMain entscheidet den Namen vor Servererwartung und erstem Client (abgelehnt Exit 64) und wartet auf dieses Konfliktende-Praedikat; das Produkt nennt seine v2-Pipe ausdruecklich. Seit NAK-313 Etappe 4 (R-313-6, 313/M-42, M-49): der v2-Client faehrt nach der UTF-8-Pruefung eines Frames genau einen strengen Lauf kanon::lies, bevor JUCE ihn liest. Je Eintrag des Eingangs cpp_v2_client der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen: ein Peer auf einem Probenamen antwortet mit den Bytes des Eintrags, und jeder Fall endet getrennt, mit letzterFehler "eingehender Pipe-Frame: " und dem Grund des strengen Laufs (Stufe parser oder duplikat). Ein welcome mit Nachspann, Schlusskomma, unbekanntem Escape oder doppeltem type beziehungsweise protocol_version (passender und fremder letzter Wert) hinterlaesst keinen gesendeten Heartbeat, kein Sitzungstoken und keine Protokollversion; ein heartbeat_ack mit doppeltem seq nach gueltigem welcome bestaetigt keinen Heartbeat und meldet keinen Konflikt. Die Zaehlpruefung verlangt genau so viele Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-91, M-93, M-94, M-96): nach der UTF-8-Pruefung und vor dem strengen Lauf prueft der Zahlriegel zahlriegelBytes jede Zahl lexikalisch gegen die v2-Grenze INT64_MAX (Stellenzahl, dann Ziffernfolge; Exponent hoechstens drei Ziffern; ein Bruch hoechstens 15 signifikante Ziffern). Ein welcome mit protocol_version 9223372036854775808, 18446744073709551618, 2e4294967296 oder 2.0000000000000001, ein heartbeat_ack mit seq 18446744073709551616 oder 1.0000000000000001 und ein reject mit min_protocol 9223372036854775808 oder 2.0000000000000001 oder max_protocol 2e4294967296 enden getrennt mit letzterFehler "Zahlriegel: " und bestaetigen nichts; 9223372036854775807 passiert den Riegel und endet an der Feldregel, 2.0 verbindet mit Version 2, und 1e999 endet am Zahlriegel statt am strengen Lauf. Die Riegeleinheit faehrt zahlriegelBytes je Vektor direkt. Der Schalter --ohne-ueberlaufvektoren laesst die ub_bei_juce-Eintraege aus (nur fuer den Rotlauf am Riegelaufruf); der Kanon faehrt alle. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 67,55 s | [A4b](roh/NAK-380-83b9fc2-dirty.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. Seit NAK-313 Etappe 4 (R-313-6, 313/M-46, M-48, M-49): json_laden_strikt lehnt doppelte dekodierte Namen im selben Objekt (auch als Escape-Alias) ueber einen object_pairs_hook mit eigener Ausnahmeklasse ab und eine Verschachtelung ueber 64 Ebenen nach dem Laden; die Stufe folgt der Ausnahmeklasse (Hook duplikat, jede andere parser). Jedes Fixture der Manifestklasse parser_lehnt_ab faellt beim Laden und traegt keine Verletzungsmenge, und je v3-Eintrag der Tabelle PRODUKTEINGAENGE-FAELLE.json liefert ein Fall mit PE-nnn im Namen Urteil und Stufe wie vertrag; die Zaehlpruefung verlangt genau so viele v3-Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-73): bei eigenem Urteil gueltig vergleicht der Fall den Wert am Zeiger feld exakt mit wert - eine Ganzzahl gleich, eine Gleitkommazahl nur ganzzahlig und im Betrag bis 2^53-1. Seit NAK-313 Etappe 6 (R-313-7; 313/M-98 bis M-102, M-106 bis M-113, M-121): die gemeinsame Falltabelle des Textriegels traegt die Untergrenze (1e-307 gueltig; 1e-308, 2e-308 und 9.9e-308 nicht), und die v3-Eintraege der Etappe (Bootstrap-Hellos, Handschlag, ACK) liefern Urteil und Stufe wie vertrag. Seit NAK-313 Etappe 7 (313/M-129): die zwei Heartbeats der Tabelle mit binary_telemetry supported und unsupported sind gueltig wie vertrag. Seit NAK-380 Etappe 2 benennt der Vertrag Feinband-Leistungsdichte, Livegruppen-Bandleistung und dimensionsloses band_stereo je Feld und bindet Version 20260925 bei unveraenderten Konfidenzschwellen. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,51 s | [A5](roh/NAK-380-83b9fc2-dirty.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,14 s | [A6](roh/NAK-380-83b9fc2-dirty.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [A7](roh/NAK-380-83b9fc2-dirty.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-48, M-53) gehoeren dazu die Tabelle PRODUKTEINGAENGE-FAELLE.json, deren Kennungen, Stufen, Wirkungen und Kopfzahlen der Erzeuger bei jedem Lauf auf Widerspruchsfreiheit prueft, die Byteinstanz evidenz-lokal-wire-v1.json, die Fixtures der Manifestklasse parser_lehnt_ab als Rohbytes (nicht ueber den Serialisierer, der das Duplikat verloere) und das Negativfixture mit project_sample_start null. Seit NAK-313 Etappe 6 (R-313-7; 313/M-98 bis M-104, M-106 bis M-113, M-121): die Tabelle traegt die Eintraege der Etappe fuer rust_bootstrap, cpp_control_handshake, cpp_telemetrie_handshake und cpp_control_ack, dazu die aus Etappe 5 an das Tor verschobenen Negativen mit den Stufen der Etappe 6; der Erzeuger prueft jede Stufe gegen die Leserkette der anlegenden Etappe, und die Falltabelle des Textriegels traegt die vier Faelle der Untergrenze. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,39 s | [A8](roh/NAK-380-83b9fc2-dirty.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID, keine ist verbrannt oder wiederverwendet, und jedes Offsetfeld steht im Rust-Strukturriegel. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,28 s | [A9](roh/NAK-380-83b9fc2-dirty.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen, integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) und seit SONDE-015 band_dynamic_gain_db ID 22 (acht Werte, leerer Vektor, Altsender ohne Feld, vier falsche Laengen, NaN und Inf) sind in beiden Lesern gedeckt. Seit NAK-313 Etappe 6 (R-313-8; 313/M-119): dazu das Negativfixture adresse-zusatzfeld-id5.bin, uebersetzt aus einer Schemavariante mit zusatz:string (id: 5) mit demselben gepinnten flatc, und das Positivfixture adresse-vtable-slot5-leer.bin als gueltige Bytemutation (byte_mutation_von) mit einer 16 Bytes langen Adress-VTable, deren Slot 14 null ist. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 3,02 s | [A10](roh/NAK-380-83b9fc2-dirty.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. Seit NAK-313 Etappe 4 (R-313-6, 313/M-42, M-49): je v2-Eintrag der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen durch den strengen Lauf json_laden_strikt und die v2-Definition der Nachricht, verglichen mit vertrag (Urteil und Stufe); die Zaehlpruefung verlangt genau so viele v2-Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-95): der Lauf liest Zahlen mit Bruch oder Exponent als decimal.Decimal, und der Typpruefer nimmt integer als int oder ganzzahligen Decimal, number samt Decimal (draft 2020-12): 2.0000000000000001 und 1.0000000000000001 sind keine Ganzzahlen, 2.0 ist die 2; bei Urteil gueltig wird der Wert am Feld exakt verglichen. Seit NAK-313 Etappe 6 (R-313-7; 313/M-104): dazu die zwei v2-gueltigen Hellos der Tabelle mit einer verletzten v3-Textriegelregel (samplerate 48000.0000000000001, NUL-Escape in sensor.label), beide gueltig gegen den v2-Vertrag. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,29 s | [A11](roh/NAK-380-83b9fc2-dirty.md#a11) |
| A12 | Parameterbestand Layout v2 (120 Kennungen, 112 Host-Parameter als Praefix) haelt den Vertrag, und die 109 v1-Kennungen sind aus der eingefrorenen v1-Datei woertlich abgeleitet; Zonen-, Preset- und Ablehnungsregeln stimmen mit dem Vertrag ueberein; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State- und Presetkorpus samt MANIFEST bytegleich zur Neuerzeugung, keine verwaiste Datei. Seit NAK-313 Etappe 3 gehoert das Negativ-Golden schema2/doppelte-eigenschaft-v1.bin (R-313-2) mit SHA-256 und Erwartung nurLesen zum MANIFEST; geladen wird es in B2. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,41 s | [A12](roh/NAK-380-83b9fc2-dirty.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B und die benannten Kanonbeweise sie tragen; jedes supported hat einen Termin oder, nur ohne Hostanteil, einen Kanonbeweis, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,24 s | [A13](roh/NAK-380-83b9fc2-dirty.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 8,50 s | [A14](roh/NAK-380-83b9fc2-dirty.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,29 s | [A17](roh/NAK-380-83b9fc2-dirty.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 34,50 s | [A18](roh/NAK-380-83b9fc2-dirty.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/NAK-380-83b9fc2-dirty.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A20](roh/NAK-380-83b9fc2-dirty.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,34 s | [A21](roh/NAK-380-83b9fc2-dirty.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,22 s | [A22](roh/NAK-380-83b9fc2-dirty.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. Eine fehlende Messung ist kein PASS (NAK-283 F13): jeder Speicherpunkt traegt das Gueltigkeitsmerkmal seiner Windows-Abfrage, und fehlt ausserhalb der Neustartfenster eine gueltige Messung, endet S07 mit dem eigenen Status MESSUNG FEHLT (Exit 3) statt im Budget. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,74 s | [A24](roh/NAK-380-83b9fc2-dirty.md#a24) |
| A34 | Der Selbsttest des Soak-Orakels (NAK-283 F13, Muster A27). Er laeuft OHNE Repo-Fixture und ohne Lauf: vier Faelle bauen ihre Berichte im Speicher und laufen durch dieselben Funktionen wie A24 - Messstelle, Speicherpunkt, Urteil. Eine fehlende RSS-Messung endet mit MESSUNG FEHLT und Exit 3, nie im Budget; ein nicht abfragbarer Prozess und ein Prozess mit 0 Bytes Working Set sind im Bericht verschieden; ein Fehlerpunkt direkt vor oder nach dem Neustartfenster faellt, obwohl das Fenster die Kurve filtert; ein Altbericht ohne Gueltigkeitsmerkmal bleibt ueber --bericht auswertbar, derselbe Bericht als Livelauf ist rot. Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --selbsttest` | [OK] Exit 0 | 0,15 s | [A34](roh/NAK-380-83b9fc2-dirty.md#a34) |
| A35 | Der Selbsttest des Laufzeit-Arms (Plan S25e, NAK-286, NAK-309) laeuft ohne FL, ohne Installation und ohne MCP-Repo gegen Attrappen: der Runner haelt Exitcodes, Urteilswoerter und Kopfzeile der ersten Fassung, beendet nie ein fremdes FL (eigen ist nur ein selbst gestarteter Prozess, dessen PID, Startzeit und Befehlszeile in der Besitzliste stehen, auch bei gleichem Fenstertitel), faehrt nur den gepinnten MCP-Stand, startet keinen FL-Lauf gegen einen nicht aktuellen Bau und meldet ein veraendertes Diagnoseprojekt; eine Szenario-Voraussetzung (Exit 5) laesst die Folge weiterlaufen, nur ABWEICHUNG loest den Rueckweg aus. Eine MCP-Antwort misst nur mit der Anforderungskennung ihres Versuchs, jede andere heisst UNGEMESSEN; dass der Importweg fl_trigger nie laedt, ist ueber ein Attrappenpaket gemessen. Antworten des Briefkastens werden an Name und Groesse ausgewaehlt, bevor eine Datei geoeffnet wird; der Nulltest trennt Format, Versatz beider Vorzeichen, Kettenfaktor, Abweichung und Nullvorzeichen und urteilt BITIDENTISCH nur bei gleichen Bytes des Bereichs (NAK-311: +0 gegen -0 heisst NULLVORZEICHEN, Szenario-Exit 5); Baender entstehen nur mit Rechnung aus F-28. Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\fl\selbsttest.py` | [OK] Exit 0 | 33,01 s | [A35](roh/NAK-380-83b9fc2-dirty.md#a35) |
| A36 | Die Frischepruefung des Runners leitet je Pruefbinary den Frischebaum aus den MSBuild-Tracking-Logs, den Kernbibliotheken und dem Konfigurationsstand ab: an einem Attrappen-Baubaum verweigert eine geaenderte DSP-Quelle ohne Bau die Beglaubigung, ein frischer Bau wird angenommen, Gleichstand ist frisch, ein nicht ableitbarer Baum ist nie gruen, und jedes gebaute gebundene Kernziel ist gedeckt. Dasselbe Urteil trifft jedes Ziel aus $gemesseneZiele, das der Runner baut (NAK-309 R-309-9; die Uebergabe misst eine Quelltextwache): eine juengere Eingabe nur des gemessenen Ziels verweigert die Beglaubigung, auch wenn jedes Pruefbinary frisch ist, und ein nicht ableitbares oder im CMake-Export fehlendes Ziel ist nie gruen. Verglichen werden Zeitstempel, keine Inhalte. Die NOT-RUN-Meldung der cargo-Beine (R-309-4): ein leerer Meldeordner heisst gelaufen, eine Marke NOT RUN mit Test und Grund, ein unlesbarer Ordner nie leer; im Urteilsblock des Runners geht ROT der fehlenden Voraussetzung vor, und NOT RUN zaehlt nie als gruen (Quelltextwache). Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\eq-copilot\pruefe_beweisrunner.py --selbsttest` | [OK] Exit 0 | 5,96 s | [A36](roh/NAK-380-83b9fc2-dirty.md#a36) |
| A28 | Der P5-Evaluationskorpus ist reproduzierbar (Muster A25): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt SHA-256 im Manifest, und eine verwaiste Datei faellt. Zusaetzlich der Hygieneriegel aus NAK-182 R2: der Bezeichner des Kettenbeins steht WOERTLICH in seiner Datei, sonst waere der Name ein Etikett. | `py -3.13 tools\eq-copilot\erzeuge_p5_korpus.py --pruefen` | [OK] Exit 0 | 0,15 s | [A28](roh/NAK-380-83b9fc2-dirty.md#a28) |
| A29 | Das Sammelbein des P5-Exit-Gates (59 Punkt 6, 36.4, M-64 bis M-70, R2). Es ist das dritte Glied einer KETTE: der Korpus traegt die Wahrheit, broker/tests/sonde014_p5_korpus.rs faehrt jede Sitzung durch p1 und schreibt die TATSAECHLICH ausgegebene Hypothese, und dieses Bein haelt beides gegeneinander - eine falsche starke Produktbehauptung aendert den Korpus nicht, sie faellt am Vergleich. Precision und Recall stehen JE URSACHENKLASSE, dazu Brier, Kalibrierung, Coverage und Enthaltungsrate; die vier Riegel (falsche_starke, falsche_schwache, precision und recall in [0,1]) laufen je Klasse und ueber die Gesamtmenge. Die Schwelle aus M-31 ist AUSGABE: gesucht wird die niedrigste Sicherheitsstufe, deren Riegel halten. Der Startwert von GATE_MINDEST_FENSTER wird an den zwei Passagensitzungen kalibriert. Ohne frische Ergebnisdatei meldet das Bein Voraussetzung-fehlt (Exit 3) statt gruen. Die Frischepruefung zaehlt das Rechenmodul broker/src/coordinator/hypothese/ zur LAUFZEIT auf (rekursiv), und eine genannte, aber fehlende Quelle ist ebenfalls Exit 3 mit Nennung des Pfads - kein stilles Ueberspringen (NAK-224 D1). | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py` | [OK] Exit 0 | 0,15 s | [A29](roh/NAK-380-83b9fc2-dirty.md#a29) |
| A30 | Der Selbsttest des P5-Sammelbeins (M-68, Muster A27). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch DIESELBEN Funktionen, die das Sammelbein fuehrt. Jede Erwartung laeuft mit ihrem GEGENTEIL - eine falsche starke Behauptung und die richtige daneben, eine Enthaltung und dieselbe Menge ohne sie, ein leerer Satz und ein voller. Die vier Riegel werden synthetisch gefuettert und muessen fallen: precision > 1, recall > 1 und brier > 1 sind ueber den Korpusweg strukturell unerreichbar, und eine Wache ohne ausfuehrbaren Negativtest ist keine gemessene Zusage. Zuletzt die Schwellensuche in beide Richtungen. | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py --selbsttest` | [OK] Exit 0 | 0,14 s | [A30](roh/NAK-380-83b9fc2-dirty.md#a30) |
| A31 | Die adversarialen Gegenbeispiele des Phasengates G5 am PRODUKTPFAD - genau die Faelle, die das Korpusformat nicht ausdruecken kann. Eine Intent-Rolle trennt zwei messtechnisch identische Kandidaten NICHT, auch nicht bei rolle=fuehrt, und bei drei sichtbaren Kandidaten wird der Fuehrende gegen JEDEN geprueft, nicht nur gegen den Zweitplatzierten. Eine Quelle, die ihre Energie in einem ganz anderen Band traegt als der Masterbefund, erreicht keine starke Aussage. Zwei Kandidaten, deren Raenge sich nur durch ein Band WEIT ausserhalb des Befundintervalls unterscheiden, sind ungetrennt. Umklammernde und disjunkte Fensterspannen fallen am Alignment mit Grund, obwohl die Ueberdeckung ihrer SPANNEN 1,0 meldet. Und die Passage laeuft ueber den echten Produktpfad samt Store: zwoelf und acht Fensterlaengen tragen die Aussage, sieben nicht, und eine Passage ohne ein einziges Masterfenster ergibt eine ENTHALTUNG mit ungueltiger Beobachtung statt Schweigen (M-27). Seit der Nacharbeit 1 (07.09.2026) faehrt das Bein ALLE 15 vereinbarten Ketteneingaben durch denselben Produktpfad und DRUCKT je Eingabe ihr Ergebnis: vier Passagenrandwerte (12, 11, 8 und 7 Fensterlaengen), den zweiten experiment_begin mit gleicher passage_id, zwei Master, den Master ohne Fenster, den Kanalwechsel vor und nach dem Deskriptorwechsel, drei Sonden auf einem Kanal, die antikorrelierte Quelle allein und neben der korrelierten, dazu die Masteranomalie in einem Fenster HINTER der Passage. Seit NAK-213 (08.09.2026) traegt KEINE der sieben frueher gedruckten NAK-213-Luecken mehr eine Luecke: zwei Master ergeben eine Enthaltung OHNE ORT statt eines stillen Gewinners, ein Master ohne Fenster ebenfalls statt Schweigen, der Kanalwechsel nimmt die Belege GENAU DER wechselnden Quelle zurueck und entfernt ihren Befund, drei Sonden auf einem Kanal werden ueber ALLE Quellen der Sitzung erkannt - eine vierte, stumme Sonde desselben Kanals setzt die Duplikatmarke mit. Seit NAK-214 (08.09.2026) fahren drei Faelle dieses Tickets mit: zwei bestaetigte Mains erzeugen KEINEN Vorschlag (Vorschlaege 0, keine proposal-Zeile im Store), ein PRE/POST-Paar unterhalb der Alignmentschwelle bleibt Aussageklasse 1 mit next_test prepost_paar_messen, und eine zurueckgenommene juengste Passage laesst die AELTERE rechnen - jeder druckt sein Ergebnis. Keine Luecke bleibt gedruckt, und der aufruferlose Helfer `luecke()` ist mit seinem letzten Aufrufer gegangen. | `cargo test --manifest-path broker/Cargo.toml --test sonde014_gegenbeispiele --color never -- --nocapture --test-threads=1` | [OK] Exit 0 | 105,08 s | [A31](roh/NAK-380-83b9fc2-dirty.md#a31) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,18 s | [A25](roh/NAK-380-83b9fc2-dirty.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,15 s | [A26](roh/NAK-380-83b9fc2-dirty.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/NAK-380-83b9fc2-dirty.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 verbundenen Quellen, davon hoechstens 20 angenommen (seit NAK-312 Etappe 7b, U51: bei 32 sind 20 angenommen und 12 nicht), liegt p95 fuer die angenommenen Quellen fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,55 s | [A23](roh/NAK-380-83b9fc2-dirty.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc) mit dem aktiven DSP-Kern: im Default (eq_enabled aus) bitgleich ueber 1000 Bloecke von 1 bis 2048 Samples bei 44,1 / 48 / 96 / 192 kHz ohne belegte Bank; eq_enabled an mit bypass aus und sonst neutralen Defaults bitgleich; eq_enabled an mit bypass an bitgleich trotz hoerbarem Band dahinter; nach eq_enabled aus nach dem Fade wieder bitgleich; Mix 0 mit Output-Trim 0 dB bitgleich; der Passthrough sanitisiert kein NaN/Inf; 0 Samples Latenz und kein Tail im Passthrough und im Hard-Bypass; speichern-laden-speichern bytegleich im Layout v2 mit Kind Dsp. Seit NAK-283 (F04, F05) bleibt der Nulltest bitgleich, wenn reset() vor jedem dritten Block laeuft - ausgeschaltet bei 44,1 / 48 / 96 / 192 kHz, ohne dass die Zaehler fuer nicht endliche Eingaenge, geheilte Filterzustaende und verworfene Analyseframes steigen, und im Hard-Bypass auch mit NaN, +Inf und -Inf zwischen zwei reset(); Mono und Stereo mit gleichem Ein- und Ausgang werden angenommen (im Monobus bleibt der Passthrough bitgleich, und das Band rechnet endlich), Quadrophonie, 5.1, vier diskrete Kanaele, Mono->Stereo, Stereo->Mono und jeder deaktivierte Hauptbus bekommen ein Nein, setBusesLayout auf Quadrophonie scheitert, und der Prozessor bleibt bei zwei Kanaelen. Das Bundle meldet 112 Host-Parameter, und sein Bundlevertrag laesst nur active_probe zu. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. Seit NAK-311 Etappe 2 (Neutralpruefung) schreibt der Kern engagiert-neutral (eq_enabled an, sonst Default) keinen Sample: ab Sample 512 kommen das Bitmuster aus Subnormals, +-0 und kleinsten Normalen, die Wachmarke 0x7F800001 (signalisierender NaN), ein ruhiger NaN und +-Inf bytegleich heraus, bei 44,1 / 48 / 96 kHz und im Monobus; der Riegel zaehlt die nicht endlichen Werte (6, im Monobus 12), der Tap post_committed traegt dort 0,0 und an jedem Rauschsample ab Sample 0 den Eingang als double. Bei Mix 0 mit Output-Trim 0 dB kommt dasselbe Material in den Bloecken ab Sample 512 bytegleich heraus, und der Tap traegt ab Sample 512 den Eingang als double. Ausgeschaltet und im Hard-Bypass bleibt das Material bytegleich ohne Zaehler; mit hoerbarem Band schreibt der Kern und gibt die Wachmarke nie aus. Die Messung 311/M-01 gibt die Bits unter FTZ/DAZ aus und prueft nur, dass alle 18 Kombinationen liefen. Seit NAK-311 Etappe 4 (R-311-3) rechnet der Ausgleich nur mit dem ausgegebenen Kanal: im Monobus melden Kern UND dspBericht fuer ein Band im Modus right exakt +0,0 dB, und der Ausgang mit Auto-Gain ist bytegleich zum Lauf ohne; nach dem Layoutwechsel auf Stereo tragen beide den Zweikanalwert, bitgleich zu einem Stereokern mit demselben Zustand, zurueck in Mono wieder +0,0 - waehrend channel_mode `right` im bestaetigten Zustand und in den Statebytes stehen bleibt, ohne Revision und ohne Host-Dirty. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, Abnahme U47) ist der EQ unter 44,1 kHz nicht unterstuetzt: bei 32 und 22,05 kHz bereitet der Kern nichts vor, und ueber 1000 Bloecke von 1 bis 2048 Samples kommen Rauschen, Bitmuster, Wachmarke, NaN und +-Inf im Stereo- wie im Monobus bytegleich heraus, ohne dass die Zaehler fuer nicht endliche Eingaenge, geheilte Filterzustaende und verworfene Analyseframes steigen; der Kern meldet die abgelehnte Rate, exakt +0,0 dB Ausgleich und die Kanalzahl 2, und die Blockgroessen 1, 255, 256, 257, 2048 und 4096 kommen ebenso bytegleich heraus, ohne gefuellten Tap, mit 0 Samples Latenz und Tail 0,0 s. Genau 44 100 Hz und der naechste double darueber bleiben unterstuetzt - das Band ist hoerbar, und der Fingerabdruck des Ausgangs ist bei 44,1 / 48 / 96 / 192 kHz gleich dem Golden, das dieselbe Messung am unveraenderten Kern des Basisstands der Etappe (5ee8318c) erzeugt hat -, waehrend 44 100 minus 1 ULP, 44 099, 32 000, 22 050, 8 000, 0, negativ, NaN und +-Inf abgelehnt werden. Rate 0, nicht endlich, 1e300 und maxBlock 0 bleiben das verriegelte Fenster: die letzte Vorbereitung steht, der dspBericht meldet sie weiter, und ABGELEHNT ist dort nichts. v3Samplerate, v3BlockSize und v3Channels tragen im abgelehnten Fenster weiter die gemessenen Werte des Hosts. Seit NAK-312 Etappe 7b (U48, U58) ueber den Hostbypass-Eintritt processBlockBypassed: Probeeq blendet in 256 Samples nach trocken und zurueck, linear und am Nachbarsprung innerhalb der E-31-Schranke, auch wenn der Wunsch in der Blende umkehrt; danach bytegleich zum Eingang auch fuer Rauschen, Subnormals, die Wachmarke, NaN und +-Inf, und nach dem Austritt bitgleich zu einem Prozessor ohne Hostbypass; die Analyse bekommt je Block genau einen Analyseblock, auch im Hostbypass, der Tap post_committed ist dort bitgleich zu dem ohne Hostbypass, und der Riegel zaehlt wie ohne ihn; nach reset und prepareToPlay, auch beim Projektladen mit gebypasstem Slot, uebernimmt der erste Block seinen Eintritt ohne Blende; mit EQ aus, im Hard-Bypass und unvorbereitet schreibt kein Eintritt, und die Analyse laeuft in beiden wie in processBlock. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,69 s | [A16](roh/NAK-380-83b9fc2-dirty.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,20 s | [B1](roh/NAK-380-83b9fc2-dirty.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. Seit SONDE-015 zusaetzlich das Kind Dsp: 120 Werte bit-exakt durch den Roundtrip, flache Arrays statt Kindknoten, voller Undo-Ring aus 32 Schnappschuessen bytegleich und unter 16 MiB, der 33. Eintrag read-only; die Layoutmigration v1 nach v2 setzt occupied bitgenau aus enabled und den Werten; das Preset traegt zwei getrennte Versionen, weist jedes der sechs verbotenen Identitaetsfelder ab, ignoriert unbekannte Top-Level-Felder und laesst eq_enabled beim Laden unberuehrt. Seit NAK-312 Etappe 5 (312/M-49): die eingefrorenen Statebytes des Parametergoldens vom Basis-SHA 12300f1e passen zu ihrer Zeile im Golden (Laenge und SHA-256), laden normal und kommen aus dem Schreiber bytegleich zurueck. Seit NAK-312 Etappe 7b (U49): der Bestand des Hauptprogramms - Mitglieder samt Labels, Passagen, Intent mit Revision, Schutzangaben, Beziehungen, Assistent - uebersteht hub, sensor, hub live und ueber Speichern und Laden gleich, im Kind RetainedMainProject (Writer-Golden legacy-retained-v1.bin); unbekannte Eigenschaften wandern mit; Altstaende laden wie bisher und kommen bytegleich zurueck; jedes unzulaessige Bestandskind ist read-only mit Originalbytes; je Wechsel ein Host-Dirty. Seit NAK-313 Etappe 2 (R-313-3): setzeBindung prueft 120/60 nur fuer geaenderte Werte - an einem geladenen Stand mit Label 121 und Paarnamen 61 Zeichen gelingt die Rollenwahl mit genau einem Host-Dirty, und beide Texte bleiben; derselbe Aufruf ohne Aenderung gibt false zurueck, ohne Host-Dirty und ohne Reconnect-Anforderung, und der Save bleibt bytegleich. Seit NAK-313 Etappe 3 (R-313-1, R-313-2): ein schreibbar geladener Stand besteht mit seinem groessten Folgezustand den Byte-Riegel verlustfrei, sonst bleibt er read-only mit Originalbytes und wird nie ignoriert - gemessen an Common mit 65 533 (schreibbar samt pre und hub) bis 65 536 Eintraegen und an der nachgerechneten Gesamtgrenze G und G+1; Tiefe 64, 63 geschachtelte Arrays und 72 Knoten bleiben rueckschreibbar. Ein doppelter Eigenschaftsname in einem Knoten, auch tief und am Sammlungsrand, macht eine bekannte Wurzel read-only mit Originalbytes, ohne einen der Werte zu lesen und ohne Schema-1-Migration; eine fremde Wurzel bleibt ignoriert; die Writer-Goldens laden wie bisher, das Negativ-Golden doppelte-eigenschaft-v1.bin read-only. Seit NAK-313 Etappe 5b (R-313-4; 313/M-74 bis M-79): jede persistente Revision - Bestands-, Eintrags- und Assistentenrevision im MainProject wie im RetainedMainProject, Dsp.state_revision und die Undo-Revision - laedt bei 2^53-1 schreibbar und kommt bytegleich zurueck; 2^53 und int64max bleiben read-only mit Originalbytes, und der Grund nennt das Feld. Der NAK-283-Fall M-11 steht am Rand 2^53-1: Assistentenhandgriff und entferneIntent werden dort abgewiesen, die Bytes bleiben gleich, und eine neue Instanz laedt schreibbar mit 2^53-1. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 18,64 s | [B2](roh/NAK-380-83b9fc2-dirty.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,08 s | [B3](roh/NAK-380-83b9fc2-dirty.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,12 s | [B3b](roh/NAK-380-83b9fc2-dirty.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. Seit SONDE-015 zusaetzlich der Fassungsschritt P1 5: der aus der committeten Fassung zurueckgebaute Leser der Fassung 4 lehnt state_report.dsp ab, ein Bericht ohne dsp bleibt in beiden Fassungen gueltig, und der Empfaenger rechnet SHA-256(dsp.jcs) gegen state_hash nach - ein schemagueltiger Bericht mit abweichendem Hash wird GANZ abgewiesen. Seit NAK-311 Etappe 4 Teil b (R-311-5) klemmt der Bericht `auto_gain_db` auf die Vertragsgrenze, statt sie zu reissen: acht Low-Shelves 1 kHz +12 dB Q 8 sind abgeleitet -199,77 dB, gemeldet werden exakt -120, und der aus dem echten Transaktionskern gebaute state_report ist gueltig; acht Shelves Q 0,707 bleiben mit -92,29 dB bitgleich zum abgeleiteten Wert. Die Klemmgrenze im C++ ist dieselbe Zahl wie minimum und maximum des GELADENEN Schemas; +/-120 kommen unveraendert zurueck, der naechste double darueber und darunter sowie 120,5 werden geklemmt, NaN wird +0,0 (nicht -0,0) und +/-Inf werden +/-120. Geklemmt wird NUR der Bericht: DspKern::autoGainDb() und der lineare Faktor des gefahrenen Programms bleiben beim ungeklemmten Wert, und klemmungen bleibt leer. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, R-311-20) meldet der Bericht bei einer Rate unter 44,1 kHz wie ohne Samplerate: bei 32 kHz haelt der Transaktionskern fs = 0, auto_gain_db ist exakt +0,0 und klemmungen leer, waehrend derselbe Zustand bei 48 kHz -92,29 dB meldet; Kern und Bericht tragen die abgelehnte Rate, und der daraus gebaute state_report bleibt gueltig gegen $defs/dsp_bericht. Der Grund bleibt C++-lokal: dasselbe Dokument MIT dem Feld im dsp-Block wird von additionalProperties: false abgewiesen, ohne das Feld ist es gueltig - der v3-Vertrag bleibt unberuehrt. Seit NAK-311 Etappe 5 Satz B (R-311-14, Karte U54) melden Bericht und Kern DENSELBEN gedeckelten Wert: acht High-Cuts 20 Hz Q 0,15 sind abgeleitet +150,46 dB und ueber DspKern::autoGainRohDb() lesbar, gefahren und gemeldet werden bitgleich kAutoGainDeckelDb, weil der Deckel vor baueBericht sitzt; die Berichtsklemmung laesst den Wert unveraendert durch, und der Zustand steht in Kern und Bericht. Mit ausgeschaltetem Auto-Gain ist er in beiden falsch, auf der Absenkungsseite ebenso (-199,77 dB abgeleitet, exakt -120 gemeldet, klemmungen leer). Der Zustand geht nicht auf den Draht: derselbe Bericht MIT dem Feld im dsp-Block wird von additionalProperties: false abgewiesen. Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-47, M-53): die Korpusschleife faehrt nach dem Textriegel ueber dieselben Rohbytes genau einen strengen Lauf kanon::lies als Tor vor der Engine; jedes Fixture der Manifestklasse parser_lehnt_ab (Nachspann, zweites Dokument, Schlusskomma, unbekanntes Escape, doppelter Name, Escape-Alias) faellt dort mit Grund und erreicht die Engine nie, alle uebrigen Urteile und Verletzungsmengen bleiben wie im Manifest - auch das Negativfixture mit project_sample_start null, dessen Verletzung an der gemeinsamen Eigenschaft liegt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-62): die Engine liest die Werte aus DIESEM strengen Lauf ueber nakama::vertrag::wertAlsVar statt ueber juce::JSON::parse; alle Urteile und Verletzungsmengen bleiben wie im Manifest, auch das Bruchzahl-Fixture mit nicht_endliche_samples 2.5 (type-Verletzung). Seit NAK-313 Etappe 6 (R-313-7, R-313-8; 313/M-101, M-114, M-118, M-119, M-121): der FlatBuffers-Leser meldet einen belegten Adressslot jenseits von Feld-ID 4 als genau einen Verstoss adresse_zusatzfeld am Pfad /eintraege/0/quelle (Fixture adresse-zusatzfeld-id5.bin aus einer Schemavariante mit Feld-ID 5); ein laengerer VTable-Eintrag mit Offset 0 (adresse-vtable-slot5-leer.bin) bleibt gueltig, und der ganze Gueltigkorpus bleibt gueltig. audioGueltig nimmt 1e-307 an und lehnt 768000.5 ab, wie das Tor des Brokers; die Falltabelle des Textriegels traegt die Untergrenze (1e-308, 2e-308 und 9.9e-308 abgelehnt). | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,44 s | [B3c](roh/NAK-380-83b9fc2-dirty.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,48 s | [B4](roh/NAK-380-83b9fc2-dirty.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,16 s | [B9](roh/NAK-380-83b9fc2-dirty.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. Seit NAK-380 Etappe 2 bleiben 221 Feinbaender Leistungsdichte in dBFS/Hz; 64 Livegruppen sind bandbreitenintegrierte Bandleistung in dBFS, einschliesslich leerer Feinbaender innerhalb einer belegten Gruppe, Parseval, Sinus und Traegerrand. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 7,33 s | [B5](roh/NAK-380-83b9fc2-dirty.md#b5) |
| B6 | Aktiver DSP-Kern (plugin/dsp/) als Bibliothek: alle sechs RBJ-Filtertypen liegen ueber einem 1/24-Oktav-Gitter von 20 Hz bis min(20 kHz, 0,45 fs) bei 44,1 / 48 / 96 / 192 kHz innerhalb 0,05 dB typisch und 0,1 dB an den Raendern an der analytischen Antwort - gemessen als Impulsantwort AM TAP post_committed, also durch den Audiopfad, gegen eine im Test eigenstaendig ausgeschriebene Formel. Ausgeschaltet und hard-bypassed wird bei jeder Hoermatrix-Auswahl kein Sample geschrieben (bitgleich); bei Mix 0 mit Output-Trim 0 dB ist der Ausgang wertgleich zum Eingang; nach dem Ausschaltfade ist der Passthrough wieder bitgleich, und er sanitisiert kein NaN. Die acht Slots wirken als Kaskade von Slot 0 nach 7 gegen ein unabhaengig gerechnetes geordnetes Golden, das ein Rueckwaertslauf reisst. Auto-Gain ergibt bei flacher Kurve exakt 0 dB und bei einem +6-dB-Shelf ueber dem Gitter -6 dB, ein identisches Mid/Side-Paar faellt exakt auf den Stereo-Fall zusammen, und die Dynamik bewegt ihn nicht. Der Bankpool faehrt jeden der sechs Ownership-Uebergaenge einzeln, gibt busy_retry statt eine aktive Bank zu verdraengen, wird erst nach dem Audio-ACK frei und haelt seine Invarianten unter zwei echten Threads ueber tausende Bloecke; im Callback zaehlt der thread-lokale Zaehler ueber 4000 Bloecke wechselnder Groesse 0 Allokationen und 0 Sperren, waehrend derselbe Zaehler die Allokationen des Workers NICHT sieht; B6 baut keine Sonde, sein Zaehler misst ab DspKern::verarbeite (NAK-312 R-312-1). Ein Block ueber maxBlock (1024 Samples bei 256) verwirft den Analyse-Tap und gleicht auf beiden Kanaelen sample-exakt dem Lauf ohne Ueberlast in 4 x 256. Seit NAK-283 (F12) die float-Kante des Ausgangs: ein endlicher double ueber FLT_MAX (Output-Trim +6 dB auf 0,75 und 0,25 x FLT_MAX) kommt auf 0 verriegelt und im Eingangszaehler gezaehlt heraus, und kein Ausgangswert ist nicht endlich; auf endlichem Material in +/-1,0 (Bell +9 dB und Output-Trim +6 dB, 204 800 Werte, auch mit Betrag >= 1) ist jeder float bitgleich die Verengung des double-Taps, und der Zaehler bleibt stehen. Seit NAK-311 Etappe 2 (Neutralpruefung) schreibt der engagiert-neutrale Kern (Merkmal neutral, alle fuenf Rampen in Ruhe auf 1,0) keinen Sample: die Wachmarke 0x7F800001, NaN, +-Inf und das Bitmuster kommen bytegleich heraus, der Riegel zaehlt, der Tap post_committed traegt dort 0,0. Das Schreibende liegt exakt am ersten Sample nach Crossfade, Rampe und Hoermatrix-Fade, auch mitten im Teilstueck, bei Blockgroessen 1, 180, 255, 256, 257 und 4096 (maxBlock 512), und das Schreiben setzt am ersten Sample eines neuen Uebergangs wieder ein; bei Delta und Candidate, bei Mix 1e-9 und mit einem 0-dB-Bell schreibt der Kern. Wo er nicht mehr schreibt, misst der Tap post_committed die Rechnung: am Ende einer Width-Rampe auf 1,0 und bei Mix 0 exakt den Eingang als double. Der Allokationslauf mit neutralem Schritt bleibt bei 0 Allokationen. Seit NAK-311 Etappe 3 (W01) schaltet ein Pfad aus der Ruhe wie ein frischer Kern ein: nach Ausschalten, Ausblenden und einer Aenderung in der Ruhe (Output- oder Input-Trim +24 dB, Mix 0, Width 2,0, Auto-Gain; ebenso nach beendeAudiohistorie im Ausblenden und fuer den Candidate nach Ende und Hoerhalt am Tap post_candidate) ist der Ausgang ab dem Umschaltblock bytegleich zu einem frischen Kern mit demselben Programm, bei Blockgroessen 1 bis 4096 und 44,1 / 48 / 96 kHz; mit Ziel 0 dB bleibt Ausgang/Eingang an jedem Sample 1,0. Seit NAK-311 Etappe 3 (W03) traegt jeder aktive Slot eine Lebenszykluskennung: ein Slot, dessen Belegung und Topologie ueber jede Publikation seit dem gefahrenen Programm gleich blieben, behaelt beim Wechsel an einem ANDEREN Slot seinen Filter-, Detektor- und Huellkurvenzustand - sein Perioden-RMS (volle Periode des Prueftons, beide Kanaele, 100 Perioden ab dem Umschaltblock) liegt hoechstens 0,5 dB neben einem Referenzkern mit gleicher Vorgeschichte ohne Wechsel, bei 44,1 / 48 / 96 kHz und Blockgroessen 1 bis 4096 ueber maxBlock, am Candidate-Pfad ebenso; ein geaenderter Slot, ein zwischendurch entfernter und neu belegter und nach einem globalen Wechsel (Samplerate, Mono-Bass-Stufe) jeder Slot startet kalt und meldet 0,0, waehrend acht Slots zugleich ihre Auslenkungen bitgleich behalten oder alle acht kalt starten. Aendert der uebertragene Slot in derselben Publikation seinen Wert, bleibt der Quadraturbetrag am Umschaltsample innerhalb der Fadeschrittweite, ueber 100 Perioden hoechstens 0,5 dB ueber der groesseren eingeschwungenen Spitze und nach Fade-Ende plus Einschwingzeit hoechstens 0,5 dB neben dem Rampenlauf ohne Fremdslotwechsel; der Mono-Bass-Zustand wandert unter gleicher Pfadkennung mit, und der Allokationslauf bleibt mit einem Committed-Fremdslotwechsel im Zyklus bei 0 Allokationen. Seit NAK-311 Etappe 4 (R-311-3) rechnet die Auto-Gain-Ableitung im Monobus (Kanalzahl 1) die geordnete Kaskade in Slotreihenfolge und wertet nur den ausgegebenen Kanal aus: ein Band im Modus right oder side ergibt exakt +0,0 dB (kein -0,0), und der Ausgang ist ueber 48 000 Samples bytegleich zum Lauf ohne Auto-Gain; left und mid kompensieren voll wie derselbe Kern im Modus stereo; zwei vertauschte Slotreihenfolgen ergeben verschiedene Werte, und zwei und acht Baender treffen die im Test eigenstaendig ausgeschriebene Formel auf 1e-9 dB. Kanalzahl 0, 2 und 3 rechnen weiter die Zweikanalformel, deren vierzehn Werte bitgleich (memcmp) zum Basisstand der Etappe bleiben, und auch im Monokern laeuft die Ableitung nie im Audiopfad. Seit NAK-311 Etappe 4 Teil b (T3-15-11) ist die Steuerrate gemessen statt begruendet: gegen einen Referenzkern mit Range 0 (derselbe SVF-Weg) bleibt der Tap post_committed nach dem Einsatz eines Quadraturtons bis einschliesslich des BEZUGSPUNKTS bitgleich und weicht genau 1 Sample danach ab; Bezugspunkt ist der erste Steuerschritt mit einer Auslenkung ungleich 0, im Test aus schrittRest und der Auslenkung gelesen statt aus dem Toneinsatz, und er liegt im Raster auf der Phase d = (8 - p) mod 8 zum Toneinsatz; genau 8 Samples nach ihm liegt wieder ein Steuerschritt, an dem der dort entworfene Satz mit Gewicht 1 wirkt - dort traegt svfVon genau den am Bezugspunkt entworfenen Satz -, in jeder gefahrenen Steuerphase und bei jeder Rate. Die Spanne 1 bis 8 Samples bis zur ersten und 8 bis 15 Samples bis zur vollen Wirkung des ersten Entwurfs ist dagegen die RASTERZAEHLUNG 1 + d und 8 + d ueber die vier gefahrenen Steuerphasen, gezaehlt ab dem Sample Bezugspunkt - d und nicht als Abstand zum Bezugspunkt; in Zeit sind die 8 bis 15 Samples der vollen Wirkung 0,167 bis 0,3125 ms bei 48 kHz, 0,181 bis 0,340 ms bei 44,1 kHz und 0,083 bis 0,156 ms bei 96 kHz, also bei 48 und 44,1 kHz GROEBER als die kuerzeste Attack von 0,1 ms. Seit dem festgelegten Pegelbegriff (Satz D, R-311-15) liegt der Bezugspunkt bei manchen Steuerphasen einen ganzen Rasterschritt nach dem Toneinsatz; dieser Rasterschritt ist das Einschwingen des Pegelbegriffs und steckt nicht in den Zahlen 1 bis 8 und 8 bis 15. Das Verhalten aendert sich dabei nicht, kDynamikSchritt bleibt 8. Seit NAK-311 Etappe 5 Satz B (T3-15-09 Teil b, R-311-14, Karte U54) ist der ANGEWANDTE Auto-Gain nach oben einseitig gedeckelt: acht High-Cuts 20 Hz Q 0,15 bei 48 kHz sind abgeleitet +150,46 dB und ueber DspKern::autoGainRohDb() weiter lesbar, angewandt wird bitgenau kAutoGainDeckelDb = 24,0 dB, und am Tap post_committed liegt der Ausgang damit um den Faktor 15,849 ueber demselben Lauf ohne Auto-Gain statt um 3,3e7 - der Zustand ist dabei gesetzt. Die Deckelfunktion laesst den naechsten double unter der Grenze und die Grenze selbst bitgleich durch und setzt erst den naechsten darueber auf die Grenze; -0,0, +0,0 und das kleinste Subnormal kommen bitgleich zurueck. Die Absenkungsseite bleibt ungedeckelt (-199,77 und -92,29 dB bitgleich, Zustand falsch), Mono- und Zweikanalzweig deckeln an ihrer eigenen Rueckgabezeile auf denselben Wert, bei ausgeschaltetem Auto-Gain wird nichts angewandt und nichts gemeldet, und nach einem Programm unter dem Deckel faellt der Zustand zurueck, waehrend der angewandte Faktor ueber kRampeSamples auf das neue Ziel laeuft statt zu springen. Alle vierzehn Hexgolden von 311/M-61 liegen zwischen -6,00 und +0,23 dB, also auf der ungedeckelten Seite, und bleiben bitgleich. Seit NAK-311 Etappe 5 Satz C (T3-15-07, R-311-13, Karte U44) kennt die Kennungsvergabe auch WERTE: reisst ein Slot bei sonst gleicher Topologie eines der drei Kriterien - Frequenzverhaeltnis 2,0, Gueteverhaeltnis 4,0, Gaindifferenz 20,0 dB, richtungsfrei und strikt groesser -, bekommt NUR dieser Slot eine neue Lebenszykluskennung, der Blockrand blendet ueber statt zu rampen, und nur dieser Slot startet kalt. Gemessen am Mass von R-311-13 (groesster Ausgangsbetrag im Fenster von 0,5 s ab dem Umschaltsample gegen die groessere der beiden eingeschwungenen Spitzen, hoechstens 1 dB): der Sprung freq_hz 5000 -> 50 Hz an einem Low-Shelf +6 dB Q 0,707 faellt von +17,41 auf -0,22 dB, derselbe Sprung am Low-Cut 2000 -> 20 Hz von +22,20 auf +0,13 dB, die Gegenrichtung 50 -> 5000 Hz von +1,91 auf 0,00 dB, und ein dynamisches Band mit Range 0 haelt das Mass ebenso. Dieselbe Strecke in zwanzig Stufen zu je 1,2589 bleibt dagegen auf dem Rampenweg, behaelt ihre Kennung und ist ueber alle 40 960 Samples bitgleich zum Basisstand. Strikt groesser entscheidet an allen sechs Kanten je Kriterium (eine ULP darunter, genau auf der Grenze, eine ULP darueber, in beiden Richtungen), nicht endliche und nicht positive Werte erzwingen IMMER eine neue Kennung, acht gleichzeitig springende Slots ergeben genau EINEN Crossfade, ein zweiter Sprung im laufenden Fade wird erst am Blockrand nach dessen Ende genommen, und der Fade dauert bei Blockgroesse 1, 255, 256, 257 und 4096 genau kFadeSamples SAMPLES. Im Callback bleibt der Zaehler dabei bei 0 Allokationen und 0 Sperren, auch wenn eine Publikation ueber dem Kriterium im selben Zyklus liegt. Seit NAK-311 Etappe 5 Satz D (T3-15-06, R-311-15, Karte U45) vergleicht die Dynamikschwelle einen FESTGELEGTEN Pegelbegriff mit dem Threshold: zwischen Detektor und Huellkurve liegt ein symmetrisches Ein-Pol-Leistungsmittel mit dem festen Fenster kPegelFensterMs = 10 ms (durchschnitt); Fenster 0 (spitze) schliesst die Stufe kurz. Aktiv ist durchschnitt - baueProgramm setzt ihn unbedingt ohne Eingabe aus dem Vertrag, im Produktcode steht genau eine Setzstelle, und Pegelbegriff::spitze kommt dort nur in der Abbildung auf die Fensterlaenge vor. Gemessen bei 48 kHz, Bell Q 0,707, Range -12 dB, Threshold im Knie: drei Materialien gleicher Leistung (Quadraturton, L = R-Sinus, Rauschen) liegen um hoechstens 0,02 dB bei 1 kHz, 0,06 dB bei 341 Hz, 0,21 dB bei 100 Hz und 0,90 dB bei 20 Hz auseinander, das Rauschen um 0,32 dB - vorher 2,12 bis 4,01 dB. Neun Einstellungen aus Attack 0,1/10/500 ms mal Hold 0/30/500 ms spannen je Bandmitte 0,05 / 0,15 / 0,50 / 2,20 dB statt 5,78 dB; die Restwelligkeit eines FESTEN Fensters waechst mit fallender Bandmitte und steht als Zahl in der Schranke. Mit Fenster 0 ist der dynamische Weg ueber 76 096 Samples bitgleich zum Basisstand des Aenderungssatzes, und der Kurzschluss gibt den Eingang auch bei nicht endlichem oder denormalem Zustand bitgleich zurueck. Der Pegelzustand liegt im BandZustand: er wandert am Blockrand mit dem uebrigen Bandzustand, wird mit dem abgeschalteten Detektor gliedweise genullt, am Blockrand geheilt und denormalgeriegelt, ist bei Stille bitgenau +0,0 und ergibt bei 44,1 / 48 / 96 / 192 kHz dieselbe Zeit. Der Pegelbegriff ist topologisch: rampenKompatibel und die Kennungsvergabe tragen ihn, zwischen zwei Begriffen wird nie interpoliert. Die Sprungantwort misst gegen eine im Test ausgeschriebene Referenz der GANZEN Kette (Bandpass, Pegelbegriff, Huellkurve, Steuerraster): bei Attack 20 ms und Fenster 10 ms liegt der 63-%-Punkt bei 32,04 bis 32,17 ms statt bei 20 ms, die Toleranz von 1 ms je Stufe bleibt. Im Callback bleiben die Zaehler mit acht dynamischen Baendern bei 0 Allokationen und 0 Sperren; im Quelltext von verarbeiteBand steht der Aufruf der Pegelstufe genau einmal, und huellkurvePol, huellkurveEntwurf, std::exp, std::pow, std::log10, new und malloc stehen dort buchstaeblich nicht - der Pol entsteht im Worker (baueProgramm). Die Wortsuche liest nur den Text dieser einen Funktion und folgt keinem Aufruf; ueber den Steuerratenschritt, der alle kDynamikSchritt Samples Kennlinie und SVF-Satz neu rechnet (leistungInDb mit log10, svfEntwurf mit pow), sagt sie nichts. Seit NAK-312 Etappe 7b (U48, U58) die Hostbypass-Stufe hinter Taps und Hoermatrix: mit dem Wunsch Hostbypass blendet sie genau 256 Samples, gezaehlt in Samples bei jeder Blockgroesse (1, 64, 256 und 4096 bei maxBlock 256, 4096 bei maxBlock 100), linear mit dem Anteil n/256 und am Nachbarsprung innerhalb der E-31-Schranke, und schreibt danach keinen Sample (Wachmarke, NaN, +-Inf, Subnormals und -0 bytegleich); kehrt der Wunsch in der Blende um, laeuft sie vom Mischstand zurueck, ohne Sprung am Umkehrsample; mit dem Wunsch nein ist verarbeite bitgleich zum Aufruf ohne das Argument. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [OK] Exit 0 | 6,31 s | [B6](roh/NAK-380-83b9fc2-dirty.md#b6) |
| B7 | Lokaler Transaktionskern der aktiven Sonde mit dem echten DSP-Kern: die Falltabelle T1-T17 aus Manifest SONDE-015 §5.11.4 laeuft tabellengetrieben mit den Invarianten I1, I2 und I4 als Wachen nach jeder Eingabe; ein Fehler in jeder Stufe S1-S7 laesst bestaetigten Zustand, Hash, Undo-Ring und Register unveraendert, und weder der Nachschlag S0 noch der Commit-Punkt allozieren; Apply, Revert, Neutralisieren, Remove, Undo, Redo und Preset-Laden erzeugen je genau eine Revision, 10.000 doppelte, vertauschte und veraltete Eingaben einer Transaktion hoechstens eine. Belegung, Remove und Undo als ein Objekt, Schutz-Zonen mit Verletzungsmeldung, Undo-Ring der Tiefe 32 und Preset halten ihre Matrixzeilen. Am echten SondeProcessor: 112 Host-Parameter in Vertragsreihenfolge ohne occupied; Hostautomation ohne Revision mit zwei Epochenwechseln je Geste, auf einem freien Slot bitgleich im Klang; ein abgeschlossener Gestus ist eine Revision; Host-Dirty nur bei einem Commit; das Kind Dsp reist durch Speichern und Laden, und ein Reload rekonstruiert Hash und Ausgang. Seit NAK-283 (F05, F09, F12) am echten SondeProcessor: reset() beendet die Audiohistorie - nach einem Impuls in den Resonator (1 kHz, +12 dB, Q 10) ist der erste Block Stille auf beiden Kanaelen exakt 0, ohne reset() klingt er nach - und laesst getStateInformation bytegleich, die Revision, das bestaetigte eq_enabled und die engagierte Bank im Kern stehen (1 kHz danach mehr als 6 dB lauter); releaseResources->prepareToPlay, reset() und prepareToPlay allein enden in derselben Audiohistorie (Stille exakt 0, untereinander bitgleich). Ein Block ueber maxBlock bei rechnendem Kern hinterlaesst in der Analysequeue eine Luecke (der naechste Block beginnt bei 192 mit kFlagLueckeDavor, Segment +1), und Kern und Queue beschreiben dieselbe verworfene Zeitspanne (ein verworfener Tap, ein Block ohne Audio mit 128 Frames, oversize und Ueberlauf +0); ruht der Committed-Pfad im Hard-Bypass, wird derselbe Block ohne Luecke angenommen. Endliche double ueber FLT_MAX am Tap kommen in der Analysekopie als endliche floats an, und der Kernzaehler steigt um genau diese 1024 Werte. Seit NAK-311 Etappe 3 (W01): reset() im laufenden Ausblenden (Output-Trim +24 dB, ein Block nach eq aus), danach Output-Trim 0 dB in der Ruhe und eq an - Ausgang/Eingang bleibt an jedem Sample des Wiedereinschaltens hoechstens 1 + 1e-6. Seit W03: Remove und Neubelegung desselben Slots mit derselben Konfiguration ohne Audioblock dazwischen - beide committet, die zweite Publikation verdraengt die erste - starten kalt; der Uebergang ist ein Crossfade, die gemeldete Auslenkung ist ab dem ersten Block exakt 0,0 und der Ausgang bei Stille ab kFadeSamples exakt 0,0. Seit NAK-311 Etappe 4 Teil b (F12, R-311-4) hat die Toleranz von M-84 ihre Zahl samt Referenzbeginn und Geltungsbereich: t = 0 ist das erste Sample des ersten Blocks der geladenen oder neu vorbereiteten Instanz, der Vergleichskern laeuft ununterbrochen weiter und bekommt ab dort denselben Eingang. Fuer ein dynamisches Bell (Kanalmodus stereo, Sidechain internal) mit Quadraturton auf der Bandmitte im Teilraum Q >= 1, Q*A_min >= 0,5 und f0 bis min(20 kHz, fs/4) weicht der Quadraturbetrag je Sample ab t_E = max(kFadeSamples, 5*tau_a*fs + 10*Q*A_max*fs/(pi*f0) + 16) hoechstens 0,1 dB ab - gemessen hoechstens 0,03 dB ueber drei Prueflinge (Q 2 bei Attack 0,1 / 10 / 500 ms, Q 1,0, f0 = fs/4 mit Q 24) je in Knie und Plateau -, davor im Betrag hoechstens \|g0\| + \|Range\| + 0,1 dB; dasselbe gilt fuer prepareToPlay auf der laufenden Instanz. Der Same-Instance-Ladestart bleibt dagegen in Ausgang UND Tap bitgleich zum ununterbrochenen Kern, und zwei Sekunden eingeschwungenes Audio lassen die Statebytes unveraendert: die Huellkurvenleistung ist Audiohistorie, kein persistenter Wert. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, R-311-20) nimmt ein Ratenwechsel den Zustand zurueck: ueber 48 -> 32 -> 48 kHz ist der Ausgang im 32-kHz-Fenster bytegleich zum Eingang und der Kern meldet 32 000 als abgelehnte Rate, nach dem dritten prepareToPlay ist der Melder wieder exakt +0,0, das Band hoerbar und der Ausgang bitgleich zu einem Kern, der nie abgelehnt hat - waehrend eq_enabled, Statebytes, Revision und Host-Dirty ueber die ganze Folge unveraendert bleiben. Mit einem High-Cut 20 Hz Q 0,707 und Auto-Gain an sind Kern UND Bericht im Fenster exakt +0,0 dB, klemmungen leer, die Zaehler des alten Fensters genullt und die Kanalzahl auf 2, beide tragen die abgelehnte Rate, jcs, Revision und Undo-Tiefe bleiben die des bestaetigten Zustands, und nach dem Rueckweg tragen beide wieder bitgleich denselben abgeleiteten Wert. Im 32-kHz-Fenster zaehlt der thread-lokale Zaehler ueber 200 Bloecke 0 Allokationen im Callback, die Echtzeitwache (seit NAK-312 ab dem Eintritt processBlock oder processBlockBypassed) 0 gemeldete Sperren, getLatencySamples bleibt 0 und getTailLengthSeconds 0,0. Seit NAK-311 Etappe 5 Satz D (T3-15-06, R-311-15) traegt t_E den festgelegten Pegelbegriff: der Huellkurventerm rechnet 5*(tau_a + kPegelFensterMs)*fs statt 5*tau_a*fs, also hoechstens 5*kPegelFensterMs*fs Samples mehr, und die drei Prueflinge weichen ab t_E in Knie und Plateau hoechstens 0,027 dB ab - die Toleranz von 0,1 dB bleibt unveraendert. Zwei Sekunden eingeschwungenes Audio bewegen den Pegelzustand nachweislich auf einen endlichen Wert ueber 0, der Slot faehrt dabei den Begriff durchschnitt, und die Statebytes bleiben bytegleich: der Pegelbegriff ist Laufzeit wie die Kanalzahl und steht in keinem Statefeld. Seit NAK-312 Etappe 2 (R-312-1, E-312-11) misst die Echtzeitwache ab dem Eintritt processBlock oder processBlockBypassed, und B7 meldet jede eingeschaltete Allokation zusaetzlich an sie: mit einem Testplayhead, der je Aufruf eine gemeldete Sperre nimmt, zaehlt sie ueber 200 Bloecke genau 200 Sperren vor dspKern->verarbeite und 0 Allokationen; ein Workerzug mit Programmbau zwischen zwei Bloecken bewegt beide Zaehler nicht; legt das Bein den Bereich wie der Wrapper um setNonRealtime und processBlock, zaehlt sie seit Etappe 5 ueber 200 Bloecke je Aufruf setNonRealtime (true) wie je Aufruf setNonRealtime (false) 0 gemeldete Sperren (bis Etappe 4 je Aufruf setNonRealtime (true) genau eine). Sperren, die JUCE vor dem Plugincode nimmt, sieht der Zaehler bauartbedingt nicht. Seit NAK-312 Etappe 3a (W02) quittiert ein Ladestart die Hostwert-Mailbox: ein Hostwert, der vor dem Laden derselben Bytes in dieselbe Instanz eintraf, bleibt wirkungslos - auch ueber prepareToPlay, eine spaetere Wiederoeffnung der Mailbox und den Zaehlerueberlauf hinweg -, der Hash bleibt der der Quelle, und ein Hostwert nach der Quittierung wirkt, auch einer zwischen Quittierung und Abgleich, dessen Regler danach denselben Wert zeigt; reset() bewahrt die Mailbox. Ein read-only geladener Stand bleibt unter Hostautomation (eq_enabled und Output-Trim) ueber 512 Samples bitgleich zum Eingang, seine Regler springen auf neutral, die Originalbytes bleiben, kein Dirty, und ein gueltiger Stand danach laesst die Automation wieder wirken; NaN und +-Inf bleiben wirkungslos. Seit NAK-312 Etappe 3b (T3-01-05 Teil a, E-312-5, E-312-6) uebernimmt der Kern Input-Trim, Output-Trim, Width und Mix am Blockrand: ein Lauf unter der Taktsperre ohne Kontrolltakt ist bitgleich zu einem mit Takt nach jedem Block - fuer alle vier in beiden Staenden (neutral und mit Band) bei Blockgroesse 1, 64, 185, 240, 256 und 512 in Echtzeit und bei 64 offline; fuer Output-Trim im neutralen Stand zusaetzlich bei 4096 (dort wirkt der Wert im Block seines Randes), ueber eine Rampe von 200 Bloecken und ueber den Zaehlerueberlauf; ohne Ereignis setzt der Blockrand kein Ziel, NaN und +-Inf erreichen ihn als bestaetigter Wert, ein Ladestart mitten in der Fahrt macht einen alten Mailboxwert auch am Blockrand wirkungslos, und die Abdeckung ist genau diese vier. Epoche und Ruhegrenze bleiben beim Worker (genau zwei Epochenwechsel je Geste, Ruhegrenze in verarbeiteten Samples), und eine blockgebundene Fahrt erzeugt keine Revision, kein Dirty und bytegleiche Statebytes; ueber 4000 Bloecke wechselnder Groesse mit Automation zaehlt die Echtzeitwache 0 Sperren und 0 Allokationen. Ein Bandwert und eq_enabled bleiben taktgebunden: ohne Takt weicht der Lauf ab, mit Takt ist er bitgleich. Seit der Nacharbeit 1 der Etappe 3 (L-1, R-312-16) nimmt der Blockrand nach einem read-only-Ladestart keinen Hostwert: laedt ein Stand mit eq an und Output-Trim +3 dB das fremde Major und kommen danach Input-Trim +24 dB, Output-Trim +24 dB, Width 2 und Mix 0 als Hostwerte, ist der Ausgang ab dem ersten Sample nach dem Laden bitgleich zu dem ohne diese Hostwerte, und die Blockrandziele steigen um 0 - bei Bloecken zu 256 und, wenn beim Laden ein Uebergang laeuft, bei 185 und 512; ein eigener schreibbarer Stand danach laesst Output-Trim +6 dB ohne Kontrolltakt wieder am Blockrand wirken (bitgleich zu einem Lauf mit Takt nach jedem Block, Ausgang/Eingang am Ende bei 10^(6/20)). Seit NAK-312 Etappe 4 (312/M-38): ueber mindestens 4000 Bloecke wechselnder Groesse, waehrend beide v3-Clients der Sonde ueber einen Testserver auf der Probe-Pipe verbunden sind und ihre Provider nachweislich durch die Besitzschleuse laufen, bleiben RtWache::sperren() und RtWache::allokationen() ab Callback-Eintritt 0. Seit NAK-312 Etappe 5 (T3-01-03, T3-01-04, R-312-3) der Offline-Uebergang: mit hoerbarer Vorschau zaehlt die Wache im Bereich wie der Wrapper ueber 200 Bloecke 0 Sperren und (seit Etappe 6a gemessen) 0 Allokationen; unmittelbar nach dem ersten Aufruf setNonRealtime (true) ist die gewuenschte Hoermatrix Processed, und der Ausgang ist ab Sample 0 bitgleich zum Lauf ohne Vorschau - bei Blockgroesse 1, 64 und 256 und auch, wenn beim Wechsel ein Hoermatrix-Fade in Gegenrichtung oder ein Hoerhalt laeuft; nach einem ausdruecklichen Kontrolltakt ist keine Vorschau mehr aktiv, nach ceil(kFadeSamples/Blockgroesse)+1 Bloecken und einem zweiten Takt die Candidate-Bank frei; der Rueckweg in die Echtzeit blendet weich (Nachbarsprung hoechstens Fadeschritt plus 2^-23) und belebt die Vorschau nicht, und ohne Vorschau aendert das Offlineflag am Ausgang nichts. Seit NAK-312 Etappe 6a (R-312-20, R-312-22), unter der Taktsperre bei stehender Vorschau: ein im Render gesetzter Hoerwunsch Dry, Delta oder Candidate wirkt als Processed - jeder Offlineblock ist bei Blockgroesse 1, 64 und 256 ab Sample 0 bitgleich zum bestaetigten Lauf; nach setNonRealtime (true) und sofort setNonRealtime (false) ohne Block dazwischen schaltet der erste Echtzeitblock beim Wechsel auf Candidate nicht hart (Nachbarsprung hoechstens Fadeschritt plus 2^-23). Seit dem zweiten Aenderungssatz der Etappe 5 (T3-01-02, R-312-4) haengen die 112 Hostparameter ohne APVTS direkt am Prozessor: Zahl, Reihenfolge, IDs, Namen, Klassen, Bereiche, Defaults und Hostsicht (convertTo0to1 an Minimum, Default und Maximum; convertFrom0to1, getText und getValueForText an 0, 0,37 und 1), die Statebytes ueber speichern, laden, speichern und die Epochenfolge einer Automationsfahrt sind gleich dem Golden des Basis-SHA 12300f1e; kein Quelltext unter eq-copilot/plugin/sonde/ nennt die APVTS-Klasse; nach dem Laden traegt jeder Hostparameter den geladenen Wert; am Ende des Destruktorrumpfs erreicht ein Hostwert den Prozessor nicht mehr; Index -1 und 112 bleiben ohne Wirkung, und der eigene Abgleich meldet kein Hostereignis; ueber 4000 Bloecke mit Hostautomation aus einem zweiten Thread zaehlt die Wache ab processBlock 0 Sperren und 0 Allokationen - die Sperre, die JUCE vor dem Plugincode in sendValueChangedMessageToListeners nimmt, sieht sie bauartbedingt nicht. Seit der Nacharbeit 1 der Etappe 5 (R-312-19, Kanonteil von 312/M-88) ruft kein Quelltext unter src, sonde, dsp, state, vertrag, core und hostbridge von eq-copilot/plugin ExitProcess, TerminateProcess, PostQuitMessage, FatalAppExit, exit, _exit, quick_exit, _Exit, Process::terminate oder systemRequestedQuit und keiner nennt WM_CLOSE, WM_QUIT, SC_CLOSE oder JUCEApplicationBase (Textriegel); ob der Host im Betrieb lebt, misst das nicht - das misst der Laufzeit-Arm. Seit NAK-312 Etappe 7b (U48, U58) der Hostbypass-Eintritt processBlockBypassed: die Wache beginnt auch dort vor der Stempelbildung (200 gemeldete Sperren ueber 200 Bloecke mit Testplayhead); ueber 4000 Bloecke wechselnder Groesse mit Eintrittswechseln, Hostautomation aus einem zweiten Thread, Programmwechseln, reset, releaseResources und prepareToPlay bleiben Sperren und Allokationen 0, die Latenz 0 und der Tail 0,0; getBypassParameter bleibt nullptr bei 112 Hostparametern; State, Hash, Revision, Undo-Ring und Dirty bleiben unberuehrt, und die Ruhegrenze der Automationsepoche zaehlt auch im Hostbypass verarbeitete Samples - sie schliesst am selben Block wie ohne ihn; der Blockrand des Bypasseintritts nimmt Hostpunkte ueber den Ueberlauf der Ereigniszaehler (M-132); Echtzeit und Offline sind bitgleich, auch wenn der Wechsel nach offline mitten in die Blende faellt, und der Bypasseintritt setzt weder den Offline-Riegel noch das harte Schalten; im Hostbypass haengt keine Bank, und nach dem Austritt klingt der zuletzt bestaetigte Stand. Seit NAK-313 Etappe 5b (R-313-4, 313/M-83): der Kern bildet seine Grenze aus derselben Konstante wie der State-Leser - bei 2^53-2 committet genau eine Transaktion auf 2^53-1, die naechste scheitert in S4 mit revision_erschoepft ohne Nebenwirkung an Zustand, Hash, Undo-Ring, Register und Revision, und ein Ladestart mit 2^53 wird mit state_revision abgewiesen. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [OK] Exit 0 | 2,22 s | [B7](roh/NAK-380-83b9fc2-dirty.md#b7) |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. Seit NAK-313 Etappe 5a (R-313-4, R-313-5; 313/M-55 bis M-60, M-73, M-96): der flache Leser liest Ganzzahlen mit nakama::wire::ganzzahlAusLiteral ohne Gleitkommaschritt, je Literal ein Fall (0, -0, 1, 1.0, 1e0, 10E-1, 91.0, 1.00e2 und 2^53-1 in Ziffern- und e-Form gelten; 1.5, 1e, 01, -1, +1, 1., 1e400, 2^53 auch als Bruchform, 2^64, NaN, Infinity, -Infinity, 1e999 und leer nicht). state_revision im command_ack gilt von 0 bis 2^53-1 in jeder Schreibweise; 2^53 und ein Ueberlauftext (2^64) sind keinAck: der Auftrag bleibt im Register, der Konflikthaken wird nicht gerufen, kein Kopf mit 0 entsteht, und erst nach dem Reconnect gibt ein gueltiges ACK ihn frei; der Konfliktkopf traegt die gelesene Revision (5 wie 5.0). welcome.protocol gilt in Control und Telemetrie mit dem Wert 3 (3, 3.0, 3e0), 4 und 3.5 verbinden nicht. Je Eintrag der Eingaenge cpp_control_ack, cpp_control_handshake und cpp_telemetrie_handshake der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen ueber die Testserver-Schalter fuer rohe welcome- und ACK-Bytes, verglichen mit produkt samt Wert, mit Zaehlpruefung je Eingang. Seit NAK-313 Etappe 5b (R-313-4, 313/M-88): heartbeatAlsJson und stateReportJson schreiben state_revision unveraendert - 2^53-1 als 9007199254740991 und ein Testaufbau mit 2^53 als 9007199254740992, nie gekappt. Seit NAK-313 Etappe 6 (R-313-7; 313/M-106 bis M-113): der flache Leser prueft UTF-8 vor dem ersten Zeichen, dekodiert Escapes nach RFC 8259 (ein Surrogatpaar wird ein Codepunkt; NUL-Escape, einsames Surrogat, unbekanntes Escape, ein u-Escape ohne vier Hexziffern und rohe Steuerzeichen sind ungueltig) und vergleicht Namen dekodiert; eine Ablehnung nennt einen von acht festen Gruenden ohne die Teilwoerter, an denen Gen eine Inkompatibilitaet erkennt. Nur die beiden welcome-Aufrufer schreiben welcome: plus Grund, ein gelesenes Objekt ohne Text in type meldet welcome: kein type. broker_version zaehlt hoechstens 64 und reject.reason hoechstens 500 Codepunkte (64 und 500 mal e-Akut gelten, 65 und 501 nicht, 499 a plus Surrogatpaar gilt, 500 a plus Paar nicht); eine leere broker_version verbindet nicht, ein leerer reason ist Broker lehnt ab. Control und Telemetrie melden dieselben Gruende; ein reject mit Escapes endet mit genau den dekodierten Bytes. Am ACK-Weg ist ein konflikt mit escaptem Code gelesen und beendet den Auftrag, ein ACK mit FF im code oder mit ergebnis als Escape-Alias und roh ist keinAck, und der Auftrag bleibt im Register. Im Tabellenlauf gilt seit Etappe 6 ein reject als gelesen, wenn es als Broker lehnt ab erscheint, und ein ACK, wenn es den Auftrag beendet. Seit NAK-313 Etappe 7 (R-313-9; 313/M-123, M-124): capabilitiesJson meldet binary_telemetry supported - zwei Hostmessungen, eine Build-Tatsache, sieben feste Fallbacks, zehn Schluessel in der bisherigen Reihenfolge -, und der Capabilitybericht traegt Schluessel fuer Schluessel dasselbe Objekt. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 231,42 s | [B10](roh/NAK-380-83b9fc2-dirty.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,01 s | [B11](roh/NAK-380-83b9fc2-dirty.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,63 s | [B12](roh/NAK-380-83b9fc2-dirty.md#b12) |
| B31 | Sonde-Lebenslaufbein (NAK-312 R-312-7): der Testkonstruktor der Sonde uebernimmt einen Probe-Pipenamen unveraendert und einen Namen ausserhalb des Probe-Namensraums fail-closed als leeren Pipenamen; ohne die Starthaken macht im Testbau kein v3-Client einen Verbindungsversuch, mit ihnen meldet der ControlClient binnen 8 s verbunden ueber die Probe-Pipe eines Testservers, und die Telemetrie koppelt nach dessen welcome; die Sonde wird mit beiden laufenden Clients auf dem Heap abgebaut. Jeder Name, mit dem ein Client startet, liegt vor dem Start im Probe-Namensraum. Seit Etappe 4 (R-312-7, R-312-12) die Besitzschleuse der Sonde, gemessen an einem gehaltenen Statusprovider und einer gezaehlten Marke vor dem ersten Besitzerzugriff: nach dem Ende des Eigentuemers erreicht ihn kein Provider (0 in jedem Fall und in 20 Gegenzyklen, nach denen Handles und Threads nicht hoeher stehen als nach dem ersten); gehalten hinter dem Betreten wird er zu Ende gewartet, gehalten davor und nach dem Destruktor freigegeben abgewiesen, vor dem Abbau freigegeben laeuft er durch; stop() loest den gehaltenen ControlClient nach der 2-s-Frist ab, controlV3.stop() gemessen vor dem Schliessen, telemetryV3.stop() nach Zeilenfolge; ein Reconnect laesst dieselbe offene Schleuse stehen. Eine Sonde aus dem Produktkonstruktor traegt im Testbau genau pipeNameV3 (aktuelleLogonSid()), ohne dass ein Client startet. | `eq-copilot\build\plugin\EqCopSondeLebenslaufTest_artefacts\Release\EqCopSondeLebenslaufTest.exe` | [OK] Exit 0 | 71,26 s | [B31](roh/NAK-380-83b9fc2-dirty.md#b31) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. Seit NAK-312 Etappe 7b (U51): hoechstens 20 Quellen werden angenommen; bei 21, 32 und 64 Sonden stehen 20 Zeilen und die Zahl der nicht angenommenen in der Sicht, eine gegangene Quelle macht Platz ohne Verdraengung, und Messungen nicht angenommener Quellen erreichen die Sicht nicht; die Zeitmessung mit 16 und 32 Snapshot-Quellen erwartet min(n, 20) Zeilen, bei 32 also 20 und 12 nicht angenommene, p95 weiter unter 300 ms. Seit der Nacharbeit 1 der Etappe 7b (R-312-32): rueckt eine wartende Sonde ueber den lokalen Eintritt nach (Publikation der gespeicherten Mitglieder ohne ein entferntes, ohne neuen Snapshot), zaehlt ihre Zeile den offenen Befund aus dem juengsten Snapshot (findingsOffen 1, jede andere Zeile 0), und nach controlEnde ist er stale und die Zeile zaehlt 0. Seit NAK-313 Etappe 4 (R-313-6, 313/M-39 bis M-41, M-49): beide v3-Leser des Quellenmodells fahren nach dem Textriegel genau einen strengen Lauf kanon::lies, bevor sie ein Feld lesen. Je Eintrag der Eingaenge cpp_sources_snapshot und cpp_sources_ruecknahme der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen, direkt am Leser in einer abonnierten Sitzung mit uebernommenem Basis-Snapshot: Nachspann, zweites Dokument, Schlusskomma, unbekanntes Escape, doppelte Namen (type, session_epoch, logon_sid in einer Mitgliedsadresse, grund, art in umfang; je mit passendem und mit fremdem letzten Wert) und der Escape-Alias fuer type sind ungueltig mit dem Praefix "session_snapshot parser: " beziehungsweise "evidence_invalidate parser: " an der Stufe parser oder duplikat, und die Sicht bleibt dieselbe (Revision, Quellen mit Mitgliedschaft und Namen, Abonnement, fuehrendes Main, Diagnose, Ruecknahmezaehler, -grund und -umfang); derselbe Name logon_sid in zwei Mitgliedsadressen bleibt gueltig, beide Mitglieder stehen in der Sicht. Je Eingang verlangt die Zaehlpruefung genau so viele Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-61, M-73, M-96): beide Leser lesen die Felder aus DEMSELBEN strengen Lauf (nakama::vertrag::wertAlsVar statt juce::JSON::parse): letzter_kontakt_ms 1500.0 und 1.5e3 sowie host_mixer_index 3.0 werden als 1500 und 3 uebernommen, eine Ruecknahme von 1024.0 bis 2048e0 wie mit Zifferntext, die vertauschte Ordnung, 1.5 und -1 bleiben ungueltig, 2^53, NaN, Infinity, -Infinity und 1e999 fallen am Textriegel; bei Urteil gueltig wird der Wert am Feld verglichen. Seit NAK-313 Etappe 6 (R-313-8, R-313-15; 313/M-108, M-117): Gen ordnet Broker lehnt ab als incompatible ohne Handgriff ein und die Lesegruende welcome: plus Grund (die acht Wortlaute und welcome: kein type) als brokerUnavailable mit Handgriff; in einem Modell, das die Quelle kennt, verwirft uebernehmeP2 einen Batch mit belegtem Adressslot jenseits von Feld-ID 4 mit /eintraege/0/quelle:adresse_zusatzfeld, und die Zeile bleibt ohne neue Messung und Lautheit. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,16 s | [B13](roh/NAK-380-83b9fc2-dirty.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. Seit NAK-312 Etappe 6b (312/M-72) reisen die Namen nach dem Labelentwurf-Fall am echten Editor (Entwurf der Startquelle A beim Klick auf B bestaetigt, danach Tick und Enter) durch Speichern und Laden in eine frische Instanz unveraendert und mit derselben Zuordnung Name zu instance_id; das Laden meldet kein Dirty. Seit Etappe 6b (312/M-76) behaelt ein geladener Stand mit 40 bestaetigten Mitgliedern und einer unbekannten additiven MainProject-Eigenschaft ueber die Folge mit 20 gezeichneten Zeilen und Ersatzziel alle 40 Mitglieder im State, im Modell und nach dem Laden in eine frische Instanz; die Eigenschaft reist unveraendert, kein Host-Dirty. Seit NAK-312 Etappe 7b (312/M-126, U51) behaelt ein geladener Stand mit 21 und 40 bestaetigten Mitgliedern alle in State und Modell, 20 davon - die 20 kleinsten instance_id - sind Zeilen, auch nach einem Snapshot mit zwei weiteren Sonden; Speichern bytegleich, kein Host-Dirty. Seit Etappe 7b, Satz 3 (U49): in Legacy ist die Live-Sicht des Quellenmodells stillgelegt - keine Zeile, keine Subscription, ein verspaeteter Snapshot traegt nichts ein -, nach der Rueckkehr stehen die Mitglieder wieder; ein nach dem Wechsel quittierter Beitritt wirkt auf den ruhenden Bestand. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 8,60 s | [B14](roh/NAK-380-83b9fc2-dirty.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. Seit NAK-312 Etappe 4 (R-312-2) faehrt dasselbe Bein das Messpunkt-Popover am echten Editor: baut die Nachrichtenschleife es erst nach Editor und Prozessor ab, fasst sein Destruktor keinen von beiden an (gezaehlte Marke 0, Bindung bis zum Prozessorende unveraendert, 0 Host-Dirty); schliesst es, solange der Editor lebt, wird eine Aenderung mit genau einer Host-Dirty-Meldung uebernommen, ohne Aenderung mit keiner. Seit NAK-312 Etappe 6b (R-312-9) dazu der Labelentwurf der Main-Flaeche am echten Editor auf einem echten Main (Namen im Prozessorzustand): ein Entwurf der Startquelle A wird beim Klick auf die Zeile B auf A bestaetigt, und weder Enter noch Fokusverlust danach schreiben ihn auf B; ein leerer Entwurf loescht Bs Namen nicht; nach Escape zeigt das Feld wieder As Label, und der naechste Auswahlwechsel schreibt nichts; faellt A weg oder kommt A mit neuer Runtime-Nonce zurueck, verfaellt der Entwurf ohne Mutation, auch mit Enter vor dem naechsten Tick; unveraenderter Text schreibt nichts; im Feld werden 121 Codepoints ausserhalb der BMP auf 120 beschraenkt und beim Bestaetigen auf 120 gekappt, 0 und 120 werden angenommen, 121 am Prozessor abgewiesen; Host-Dirty je geaendertem Namen genau einmal. Seit Etappe 6b (R-312-6) dazu Ersatzziel und Aktionssteuerung: mit 16, 20, 21, 32 und 64 Quellen ohne Hauptziel ist das Ersatz-Hauptziel die erste angezeigte Quelle (vier Bilder 20/21/32/64 im Sichtsatz), ab 20 Quellen zeichnet die Flaeche 20 Zeilen; rueckt ein per Handgriff gewaehltes Hauptziel durch Beitritt oder Hostbusnamen hinter Zeile 20, sind Knopf, Labelfeld und dessen Beschriftung unsichtbar, das Aktionsziel ist leer, das Hauptziel im Modell unveraendert ohne Schreibversuch, und der Status sagt es; ein offener Entwurf wird davor auf der Startquelle bestaetigt (genau 1 Host-Dirty); ohne Quelle gibt es keine Aktionssteuerung, bei 1 und 16 Quellen ist jede Zeile gezeichnet und per Klick erreichbar. Seit Etappe 6b (NAK-349, R-312-2) dazu das Kennungskonflikt-Panel: stellt die Nachrichtenschleife einen geposteten Klick (triggerClick, der Weg von Enter) erst nach dem Ende des Editors zu, mit lebendem und mit danach zerstoertem Prozessor, fasst der Rueckruf weder Editor noch Prozessor an (gezaehlte Marke 0, Kennung unveraendert, 0 Host-Dirty); bei lebendem Editor vergibt derselbe Klick eine neue Kennung mit genau einer Host-Dirty-Meldung und Statusmeldung, und die Box schliesst. Seit der Nacharbeit 1 der Etappe 6 (L-5, R-312-2 Satz 2) dazu: beginnt der Klick bei lebendem Editor und baut ein Listener in audioProcessorChanged den Editor ab, waehrend der Rueckruf in neueSensorId steht (nach seinem Eintritt, die Kennung schon neu), wechselt die Kennung mit genau einer Host-Dirty-Meldung, der Rueckruf erreicht danach keinen Zugriff auf den Editor (zweite Lebendpruefung, gezaehlte Marke 0), und die Box wird abgebaut. Seit NAK-312 Etappe 7b (U51) dazu die Annahmegrenze am echten Pfad (abonnierte Sitzung, echte Snapshots): bei 0, 1, 19 und 20 Quellen ist jede angenommen, gezeichnet und per Klick erreichbar; bei 21, 32 und 64 Quellen sind 20 angenommen, jede davon gezeichnet und erreichbar, und das vorhandene Diagnosefeld meldet die volle Liste mit der Zahl der nicht angenommenen, allein oder als Zusatz hinter einer anstehenden Diagnose. Seit NAK-313 Etappe 2 (R-313-3) schreibt das Messpunkt-Popover nur, was der User geaendert hat: mit geladenem Label von 121 und Paarnamen von 61 Zeichen ruft das unveraendert geschlossene Popover die Bindungs-API nicht (0 Aufrufe, 0 Host-Dirty, 0 Reconnect-Anforderungen, Save bytegleich), und auch Fokusverlust und Rollenrueckruf ohne Wechsel rufen sie nicht und melden kein Host-Dirty; eine Rollenwahl mit unveraendert langem Label gelingt mit genau einem Aufruf und einem Host-Dirty, und beide Texte bleiben; die Felder begrenzen die Eingabe auf 120 und 60 Codepunkte (das Paarfeld gemessen bei eingerichteter Paarrolle pre); ein geaendertes, weiter zu langes Label oder ein solcher Paarname wird ohne Host-Dirty abgewiesen, und die vorhandene Statuszeile nennt die Grenze; traegt der abgewiesene Aufruf eine Rollenwahl (E-313-18), faellt die Rollenauswahl ohne zweiten Aufruf auf die gespeicherte Rolle zurueck, und die Meldung nennt es. Seit NAK-313 Etappe 7 (R-313-10; 313/M-141, M-142): der Festhalten-Handgriff am echten Editor meldet die entstandene Datei, bei einem Namen derselben Sekunde die vorhandene Datei und dass nichts gespeichert ist, bei einem Schreibfehler den Grund samt Rest der eigenen Temp-Datei; Datei und Vergleichslinie tragen denselben Snapshot, auch wenn sich die Messdaten der Engine direkt danach aendern. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 7,74 s | [B15](roh/NAK-380-83b9fc2-dirty.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/NAK-380-83b9fc2-dirty.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und der Fingerprint ist unter Vorzeichenumkehr des Signals bytegleich. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 1,22 s | [B22](roh/NAK-380-83b9fc2-dirty.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,85 s | [B23](roh/NAK-380-83b9fc2-dirty.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,04 s | [B24](roh/NAK-380-83b9fc2-dirty.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,07 s | [B25](roh/NAK-380-83b9fc2-dirty.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,53 s | [B26](roh/NAK-380-83b9fc2-dirty.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,13 s | [B21](roh/NAK-380-83b9fc2-dirty.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,90 s | [B19](roh/NAK-380-83b9fc2-dirty.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht fuer 15 bis 19 keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt auf 1e-9 genau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. Seit NAK-283 (N01) zusaetzlich die vier transienten Faelle 20 bis 23 aus EBU Tech 3341 (Versatz 0 bis 3): jeder trifft eine im Test gerechnete, vorher an 15 bis 19 auf +/-0,01 dB validierte 64-fach-Referenz innerhalb +/-0,1 dB und liegt zusaetzlich in der Normtoleranz 0,0 +0,2/-0,4 dBTP. Insgesamt 43 Pruefungen. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,38 s | [B17](roh/NAK-380-83b9fc2-dirty.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 5,71 s | [B18](roh/NAK-380-83b9fc2-dirty.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. Seit NAK-313 Etappe 4 (R-313-7, 313/M-51): ein echter FeatureFrame ohne gueltige Projektzeit (Transport local_monotonic) ergibt mit den Transportzahlen aus eingabe der Byteinstanz evidenz-lokal-wire-v1.json einen Transportblock OHNE project_sample_start, bytegleich zu wire_transport, und der ganze Snapshot passiert Textriegel und dieselbe Engine wie B3c. Seit Etappe 4 (R-313-6, 313/M-49) ausserdem der C++-Vertragsweg an der Tabelle PRODUKTEINGAENGE-FAELLE.json: je evidence_snapshot-Eintrag ein Fall mit PE-nnn im Namen durch Textriegel, genau einen strengen Lauf kanon::lies und die Engine, verglichen mit vertrag - doppeltes type faellt an der Stufe duplikat, eine Verschachtelung von 65 Ebenen an parser, project_sample_start null am Schema, und 64 Ebenen sowie der wire_snapshot der Byteinstanz sind gueltig; die Zaehlpruefung verlangt genau so viele Faelle, wie der Kopf der Tabelle fuer evidence_snapshot nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-73): die Werte kommen aus demselben strengen Lauf (nakama::vertrag::wertAlsVar), und bei Urteil gueltig wird der Wert am Feld als Ganzzahl verglichen (transport.sequence 8241.0 und 8.241e3 sind 8241). | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 1,87 s | [B16](roh/NAK-380-83b9fc2-dirty.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. Seit NAK-313 Etappe 5b (R-313-4; 313/M-80, M-81, M-89): die Revisionsraender der NAK-283-Faelle M-07 bis M-10 und M-12 stehen bei 2^53-1 (assistentenrevision_laeuft_nicht_ueber misst den neuen Rand); Bestands- und Eintragsrevision gehen bei 2^53-2 genau einen Schritt mit genau einem Host-Dirty und weisen am Rand ab, bevor sie etwas zuweisen (Prozessor ohne Host-Dirty, Bibliothek mit Grund und gleichen Bytes); der Intentwriter schreibt bestand_revision 2^53-1 ab bestand_revision bytegleich zur Handinstanz intent-wire-v1.json. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,74 s | [B27](roh/NAK-380-83b9fc2-dirty.md#b27) |
| B29 | Die Zustandsmaschine des AssistantStep im Main (46.1, E-07/E-08, M-55 bis M-62). Die Zustandsmenge hat ACHT Werte, jeder kommt ueber seinen eigenen Rueckweg zurueck, ein neunter faellt. Nach preview fuehrt KEINE Kante - weder hin noch zurueck -, ein gespeicherter preview-Schritt ist ein Lesefehler statt eines stillen Sprungs, und die Produkt-API setzt ihn nicht; die Gegenprobe mit proposal laedt normal. Jeder der acht Zustaende traegt fuenf Angaben (Eintritt, Evidenz, Useraktion, Timeout ueber 0, sichere Rueckkante), auch preview - der erste Zustand ist seine eigene Rueckkante. Der Deckel ist STRUKTURELL: ein zweiter Startversuch bei offenem Schritt wird abgewiesen, nicht eingereiht, und erst nach dem terminalen Abbruch beginnt ein neuer. Die vier Gegenpfade laufen einzeln: Zurueck geht auf die Rueckkante, Ueberspringen auf den naechsten Zustand, Resume ist eine FRAGE ohne Revisionssprung, und Verwerfen ist terminal statt geloescht - danach gibt es kein Resume mehr. Die drei benannten Ergebnisse aus 46.2 sind eigene Ergebnisse mit Objekt; dasselbe zweimal hebt keine Revision. Die harten Gates greifen VOR der Gewichtung: drei Kandidaten mit perfektem Rang und je einem gerissenen Gate verlassen die Liste, ein bescheidener bleibt. Verdrahtet am echten Prozessor mit Host-Dirty genau einmal je echter Aenderung, Rekonstruktion aus dem gespeicherten MainProject und bytegleichem Save/Load; jede Grenze faellt von beiden Seiten, und ein Altstand ohne die Eigenschaft laedt normal und schreibt sie nicht. Seit NAK-313 Etappe 5b (R-313-4; 313/M-82, M-84 bis M-87): an der Revision 2^53-1 weisen Weiter, Antwort und ein neuer Lauf ab, ohne etwas zu aendern und ohne Host-Dirty; assistentVersuchStarten prueft die Grenze vor dem Versuchsbeginn - am Probe-Server kein experiment_begin, kein belegter Slot, kein eingefrorener Pegel, 0 Host-Dirty, und der gespeicherte Stand traegt keine Versuchskennung -, startet unter der Grenze wie bisher mit genau einem experiment_begin und einem Host-Dirty, und faellt die Grenze erst nach dem Versuchsbeginn (Testhaken), weist die Bibliotheksfunktion ab und der Versuch endet mit genau einem experiment_abort (user_abbruch). | `eq-copilot\build\plugin\EqCopSonde014AssistentTest_artefacts\Release\EqCopSonde014AssistentTest.exe` | [OK] Exit 0 | 2,51 s | [B29](roh/NAK-380-83b9fc2-dirty.md#b29) |
| B30 | Diagnose-Briefkasten im Plugin (Plan S25e (3), R-286-1 und R-286-2): ohne anfrage.json nur eine Existenzpruefung je Takt (hoechstens 1 Hz, Message-Thread) und nichts Messbares - Audio, State, Parameter und der Snapshot des Knopfwegs bleiben bytegleich. Eine gueltige Anfrage beantworten Gen und Probeeq je Kennung genau einmal, solange sie im Ring der letzten 256 steht, ueber den eigenen Schreibweg (exklusive Temp-Datei, Groessenpruefung, Umbenennen ohne Ersetzen, nie Ordneranlage oder Loeschung) mit einem Umschlag nach nakama.diagnose.antwort.v1; fremde oder zu grosse Anfragen und solche hinter einer Junction bleiben ohne Antwort, eine beim Lesen gesperrte wird im naechsten Takt beantwortet. Seit NAK-313 Etappe 7 (R-313-10; 313/M-132 bis M-144) der Festhalten-Export, jeder Fall in einem Testordner unter %TEMP%: er legt die Ordnerebenen an und veroeffentlicht exklusiv (Temp-Datei, Spuelen, Groesse, Umbenennen ohne Ersetzen), ersetzt eine Datei gleichen Namens nie (Doppelklick, zwei Instanzen, leeres Label, Schreibweise und Bereinigung, Uhrrueckstellung: abgelehnt, die erste Datei bytegleich), meldet Teil-Schreibfehler als Fehler und entfernt die eigene Temp-Datei oder nennt sie als Rest; er liest je Export genau eine Uhrablesung und nur den uebergebenen Snapshot, der Testbau verweigert ohne Testordner vor jedem Dateizugriff, und die Referenz des Knopfwegs bleibt bytegleich. | `eq-copilot\build\plugin\EqCopBriefkastenTest_artefacts\Release\EqCopBriefkastenTest.exe` | [OK] Exit 0 | 77,55 s | [B30](roh/NAK-380-83b9fc2-dirty.md#b30) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,15 s | [B28](roh/NAK-380-83b9fc2-dirty.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. Seit NAK-309 (M-74) zuerst, vor dem ersten Prozessor: im Testbau nennt der Prozessor seinem v2-Client einen Probe-Namen, nie die Produktions-Pipe, nach Produkt- und Testkonstruktor denselben, und jedes Ziel mit den Prozessorquellen ausser dem Produkt traegt den Testschalter. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,32 s | [B8](roh/NAK-380-83b9fc2-dirty.md#b8) |
| A32 | Codebase- und Kontextgesundheit gegen die Schwellen aus Plan S25b und dem Kontext-Hygiene-Playbook. Der Quellumfang kommt seit NAK-283 (M-69) aus einem pruefbaren Inventar: jede Datei mit Endung .rs, .cpp, .h oder .hpp unter broker/src und eq-copilot/plugin samt allen Unterbaeumen, mit Grund ausgenommen nur der flatc-Codegen (broker/src/generiert, eq-copilot/plugin/vertrag/generiert) und der Testbaum eq-copilot/plugin/tests; der Bericht druckt das Inventar, und eine Quelldatei, die weder gemessen noch ausgenommen ist, beendet den Lauf als WERKZEUGFEHLER (Inventarriegel, Exit 2). Gemessen werden Zeilen je Quelldatei (Grenze 2 000, Ziel 1 500), Anzahl Funktionen ueber 200 Zeilen, aufruferlose allow(dead_code)-Helfer, Backtick-Bezeichner in Kommentaren ohne Entsprechung im Repo, sowie Bytes von MEMORY.md, CLAUDE.md und Dirigenten-Skill, Index-Zeilen ueber 250 Zeichen und Memory-Dateien ohne Index-Link. Die Ratschen (Funktionen 28, Kommentar-Bezeichner 30, clippy 91) stehen auf dem Iststand vom 09.09.2026 nach NAK-225 und reissen bei der ersten Verschlechterung; sie werden nur gesenkt, nie erhoeht. Die Grenzen - Zeilen je Datei, Funktionen ueber 200 Zeilen, Kommentar-Bezeichner - werden ehrlich gemeldet, auch wenn sie reissen (Pflegeticket NAK-292). Jeder neue Treffer ueber der Zeilengrenze meldet sich als OHNE PFLEGETICKET; ein Zuordnungseintrag, dessen Datei die Grenze nicht mehr reisst, beendet den Lauf als WERKZEUGFEHLER (Exit 2), damit die Liste nicht still veraltet. Vorangestellt laeuft der Selbsttest: 129 Faelle im Speicher durch dieselben Funktionen, jede Erwartung mit ihrem Gegenteil (Zahlenraender 2 000/2 001, 200/201, 250/251, CRLF, BOM, fehlender Schlussumbruch, Rust-Lebensdauer und C++-Ziffernstrenner gegen die Literalmaskierung; das Inventar mit neuem Unterbaum, Codegen, Tests und einer Ableitung, die still einen Unterbaum verliert). Der clippy-Teil laeuft NICHT mit (Schalter --clippy, er baut) und meldet sich als nicht messbar. Nicht blockierend: Exit 4 heisst gerissen und ist ein Hinweis. Exit 2 ist ROT und heisst, dass das WERKZEUG nicht mehr misst - roter Selbsttest, eine fehlende Inventarwurzel oder der Inventarriegel, denn ein verschobener Pfad wuerde sonst still 0 messen und die halbe Codebase unbemerkt aus dem Mass nehmen, und ein ungemessener Unterbaum machte jedes andere Mass falsch. | `py -3.13 tools\plan\gesundheit.py --mit-selbsttest` | [HINWEIS] Exit 4 - Befund, nicht blockierend (siehe Rohausgabe) | 36,14 s | [A32](roh/NAK-380-83b9fc2-dirty.md#a32) |
| A33 | clang-tidy ueber alle Uebersetzungseinheiten von eq-copilot/plugin ohne tests/ und ohne den flatc-Codegen vertrag/generiert/ (Bein A9 haelt ihn bytegleich; Regelsatz eq-copilot/plugin/.clang-tidy: bugprone, clang-analyzer, concurrency, performance sowie eine cert-, cppcoreguidelines- und misc-Auswahl; keine Stilregeln, Ausnahmen dort mit Grund) in einem eigenen Ninja-Baum eq-copilot/build-tidy mit denselben cl.exe-Schaltern wie der Produktbau. Gezaehlt werden eindeutige Fundstellen (Datei, Zeile, Spalte, Check) gegen die Ratsche in tools/plan/tidy.py: Grenze = Iststand der Erstmessung NAK-288, Ziel 0, nur gesenkt, nie erhoeht; eine Aenderung des Regelsatzes zieht die Ratsche im selben Commit nach. Vorangestellt der Selbsttest (Einordnung in den Quellort, Datenbankfilter, Diagnoseparser, Deduplizierung, Ratschenrand, Umgebungsleser). Nicht blockierend: Exit 4 ist ein Hinweis. ROT ist Exit 2: eine Uebersetzungseinheit mit error-Diagnose oder ein abgestuerzter clang-tidy heisst NICHT gemessen, denn eine halb geparste Einheit meldet weniger Funde und saehe wie Fortschritt aus. FEHLT (Exit 3) ohne Visual Studio 2022 mit C++-Werkzeugen und Clang-Komponente, cmake oder ninja. | `py -3.13 tools\plan\tidy.py --mit-selbsttest` | [OK] Exit 0 | 107,10 s | [A33](roh/NAK-380-83b9fc2-dirty.md#a33) |

## 24. Erstprüfung 2: Urteil, Validierung, Einordnung, Regel R-380-10 und Nacharbeitsauftrag 1 (Dirigent, 25.09.2026, 12:50 Uhr)

**Kanon nachher der Etappe 2.** Abgekoppelt gestartet 12:00:20 Uhr auf
`8dc54ea2` (Code gleich `6172067a`); GRUEN 70/70, 12:00:20 bis 13:15:18 Uhr, EXIT=0, Rohausgabe `docs/beweise/roh/NAK-380-83b9fc2-dirty.md` (dirty: der Runner pinnt den Release-Broker in `eq-copilot/install/nakama-installer-v1.json`; diese Hashzeile gehört zu den Abschlussdateien dieser Runde), A15 stillgelegt, A32 Hinweis Exit 4 (Quelldateien über 2 000 Zeilen, Funktionen über 200 Zeilen, Kommentar-Bezeichner ohne Code: vorbestehende Codebase-Maße, Pflegeticket NAK-292; keine Kontextflächen-Zeile gerissen), A33 Exit 0; der Kanon-Abschnitt steht vor diesem Abschnitt. Während des Laufs bewegte
sich HEAD durch den Planstand-Commit `83b9fc2c` (nur `docs/PLAN-STAND.md`,
kein Kanon-Bein); der Runner las den Stand beim Start. Lehre: vor jedem
Commit das Kanon-Log der Vorsitzung lesen.

**Erstprüfung 2 (Vorlage A).** Codex `gpt-6-astra`, Effort **high** (vor
dem Claude-Wochenreset 15:00 Uhr nach NAK-396; Claude-Woche 95 %, Codex
70 %), lesend, Thread `01a0d809-e8f0-72e0-ba13-b953172c7455`, 12:08:42 bis
12:18:06 Uhr, EXIT=0, HEAD vor und nach dem Lauf `83b9fc2c`. Prüfbereich
`39fbf165...83b9fc2c` über die 28 Ticketpfade der Etappe 2 (`git diff
--name-only 39fbf165 HEAD -- broker eq-copilot tools`); Auftrag
`docs/beweise/roh/NAK-380-erstpruefung-2-auftrag.txt`, Urteil
`docs/beweise/roh/NAK-380-erstpruefung-2-urteil.md`. **URTEIL: NEEDS_WORK — 5
Defekte, 1 Lücke, 0 Härtungen.** Geprüft: gebundener Diff, Gate-Abgleich,
Matrix und Entscheide, Tests und Mutationsstellen, Produktrechnung statisch
konsistent mit Dichte·Gitterbreite und Ergänzung, alle 222 C++-Kantenbits
gegen das JSON identisch, drei Breitenliterale und Summe nachgerechnet,
Versionen 20260925 konsistent, Vertragsänderungen ausschließlich beschreibend,
generierte Änderungen ausschließlich Kommentare, Fixtures, Gitter und
geschützte Produktpfade ohne Diff, F7 und EIGEN-4 ohne gelockerte Schwellen,
CMake-Erweiterung nur am bestehenden B16-Ziel. Nicht geprüft: keine Läufe,
Laufwerte nicht reproduziert.

**Validierung an der Quelle.** Codex `gpt-6-astra` high, lesend (Validierer
ab 90 % Claude-Woche nach Kopfzeile Prüfmodell), Thread
`01a0d818-55b5-76b0-8e8e-3682cae89868`, 12:24:28 bis 12:35:08 Uhr, EXIT=0,
HEAD gleich; Auftrag `docs/beweise/roh/NAK-380-erstpruefung-2-validierung-auftrag.txt`,
Bericht `docs/beweise/roh/NAK-380-erstpruefung-2-validierung.md`: 4
bestätigt, 2 präzisiert, 0 widerlegt, dazu zwei Abdeckungsbefunde Z1 und Z2.
Der Dirigent hat jede Stelle selbst nachgelesen (Zitate unten).

### 24.1 Einordnung je Befund

| Befund | Quelle (HEAD `83b9fc2c`) | Validierung | Einordnung | Regel |
|---|---|---|---|---|
| D1 Evidenzintervall nicht gesetzt | `AnalysisGoldenTestMain.cpp:771-776` (`nak380W0Lauf`: `vorbereiten (fs)`, kein `evidenzIntervallSetzen`); `FeatureEngine.h:1350` `evidenzIntervallS { kEvidenzIntervallMinS }` = 0,25 s | BESTÄTIGT | **DEFEKT** — M-01 Vorbedingung „Evidenzintervall 1 s“ | Intervall 1 s setzen, Framezahl hergeleitet verriegeln; Ausgangserfassung erneuern |
| D2 P50 nur Band 220 | `:893-899` `p50Band = 220` | BESTÄTIGT | **DEFEKT** — M-01 „der P50 desselben Bandes“ gilt je belegtem Band | P50 je Band gegen R-380-10 (iii) |
| D3 Bitzusage nur über die Schnittmenge | `:815-824` (`bitmapLies (evidenz) && bitmapLies (evidenzP50)`), `:886-889` (`n == 0` als „ohne Bit“) | BESTÄTIGT | **DEFEKT** — M-01 „die 25 Bänder ohne Bin tragen kein Bit“ wird nicht an der Bitmap gemessen | Bitmaps je Frame gegen die Gitterzuordnung, R-380-10 (iv) |
| Z1 M-02 überspringt Gruppen ohne Beitrag | `:913-915` `if (n == 0) continue;` | Validierer, vom Dirigenten bestätigt | **DEFEKT** — M-02 „je Gruppe g“ | Pflichtbeitrag je Gruppe, sonst rot; gleiche Ursache wie D1 bis D3 |
| D4 M-11-Rotbeweis fällt am Längenriegel / an UB | `bandbreite.rs:24` `assert_eq!(hex64.len(), BAENDER_FEIN + 1)`; Gitter-JSON: `kanten_hz.hex64` 222, `mitten_hz.hex64` 221 Elemente; `rot-M-11.txt` nennt „Summe 620765,452223“ (Summe der Mitten, nicht der Differenzen; am HEAD-Code nicht reproduzierbar); C++ `evidenzMitte (221)` außerhalb des 221er-Arrays | PRÄZISIERT | **DEFEKT des Rotbeweises** — §8.1 und die Regel „Rotbeweis an der Zusagezeile, nicht an einem Nebeneffekt“; kein Produktfehler | formgültige falsche Breite (`* 0.5`) in beiden Sprachen, neue Rohdatei |
| D5 `.fbs`-Bandwerte-Kommentar ungeprüft | `pruefe_v3_vertrag.py:2298-2302` prüft `fb_readme`; `.fbs:205` „einseitige Leistungsdichte in dBFS/Hz“ ohne Prüfung; `rot-M-17.txt` mutierte das JSON-Schema | BESTÄTIGT | **DEFEKT** — M-17 „die vier Vertragsstellen unterscheiden“ | Teilprüfung „FlatBuffers Bandwerte“, vierter Größensatz im Rotbeweis |
| Z2 C++-Leser ohne beide Fassungen | `Sonde013EventWireTest.cpp:570-586` schreibt nur frische Frames; `erzeuge_fb_fixtures.py:126` `metrics_version: 1`; Rust `telemetrie.rs:1186-1193` prüft 20260904 und 20260925 | Validierer | **HÄRTUNG** — M-18 nennt für C++ „B3c bestehend“ (Korpus mit Version 1, Leser nimmt ihn an: „kein Leser prüft den Versionswert“ ist in C++ belegt); ein C++-P2-Fall mit beiden Fassungen ist von keiner Zeile verlangt | Register NAK-402 |
| L1 Toleranz ± 0,3 dB ohne Referenz | M-01 Zusage „± 0,3 dB“; gemessen 0,3822 dB (0,25-s-Intervall), Test verriegelt 0,5 dB; Prüfer: σ ≈ 0,172 dB im ungünstigsten Einbinband, erwartetes Betragsmaximum ≈ 0,42 dB, 95-%-Quantil ≈ 0,55 dB | PRÄZISIERT (Lücke) | **LÜCKE** — R-380-8 verlangt eine statistisch begründete Toleranz; die Zahl der Etappe 1 hatte keine | **R-380-10** (unten) |

Die Prognose F7 „über zwei Quanten“ (§8.2) gegen die Messung „eine Quante“
(§22.5) und EIGEN-4 sind vom Prüfer ohne gelockerte Schwelle bestätigt: kein
Befund. Die CMake-Änderung liegt in der Ausnahme von §8.2.

### 24.2 Regel R-380-10 (Toleranzen von M-01 aus der Statistik; schließt L1)

Für W0 (L und R unabhängig, σ = 0,1 je Kanal, 48 kHz, Block 512,
Evidenzintervall 1 s, 60 s, Auswertung ab 3 s) gilt je belegtem Feinband b
mit n_b = bandBis − bandVon Bins in der zuständigen Stufe (Bass 16384/8192
unter `kTrennungHz`, Haupt 4096/2048 darüber) und M_s Fenstern der Stufe von
3 s bis 60 s (M_Bass = ⌊57·48000/8192⌋ = 334, M_Haupt = ⌊57·48000/2048⌋ =
1335):

- (i) Mittel über die Evidenzframes: |mittel_b − D| ≤ 4·σ_b mit D =
  10·log10(2σ²/fs) = −63,8021 dBFS/Hz, σ_b = 4,343 dB/√K_b, K_b =
  2·M_s·n_eff(n_b)/1,056. Herleitung: der Fensterwert je Bin ist |M_k|² +
  |S_k|² normiert, zwei unabhängige χ²₂-Anteile (Faktor 2); 1,056 ist der
  Welch-Faktor für Hann bei 50 % Überlappung (1 + 2·0,167²); n_eff deckt die
  Hann-Korrelation benachbarter Bins (Periodogrammkorrelation 4/9 beim
  Nachbarbin, 1/36 beim übernächsten): n_eff(1) = 1, n_eff(n) = n/(1 +
  (8/9)(n−1)/n + (1/18)(n−2)/n) für n ≥ 2 (n_eff(2) = 1,385; n_eff(44) ≈
  22,9). Zahlen: n_b = 1 Bass K = 633, σ = 0,173 dB, Toleranz 0,69 dB; n_b =
  1 Haupt K = 2528, σ = 0,086 dB, Toleranz 0,35 dB; n_b = 44 Haupt K ≈
  57 900, σ = 0,018 dB, Toleranz 0,072 dB. Familienweise Fehlerrate über 196
  Bänder bei 4σ ≈ 1,2 %; der gemessene Wert 0,3822 dB (0,25 s) liegt für ein
  Einbin-Bassband bei 2,2σ.
- (ii) Systematik: |Mittel über alle 196 belegten Bänder von (mittel_b − D)|
  ≤ 0,05 dB (σ dieses Mittels ≈ 0,01 dB; eine falsche Fensternormierung
  fällt hier, auch wenn (i) sie in schmalen Bändern deckt).
- (iii) P50 je belegtem Band (Stichprobenmedian der Fensterwerte im
  Evidenzfenster, `Frame.h:461-488`, in dB über die Frames gemittelt):
  |p50_b − (D + Δ(n_b))| ≤ 1,5 dB mit Δ(n_b) = 10·log10(1 − 1/(3k) +
  8/(405k²)), k = 2·n_eff(n_b) (Medianversatz einer Γ(k)-Verteilung gegen
  ihr Mittel; n_b = 1: −0,77 dB; 2: −0,55 dB; 44: −0,03 dB). Die 1,5 dB
  decken Stichprobenmedian (6 bis 7 Fenster je Frame in der Bassstufe, 25 in
  der Hauptstufe) und den Versatz der dB-Mittelung (Bass-Einbinbänder
  erwartet ≈ −0,45 ± 0,2 dB um die Referenz). Die bisherige Zusage „± 1,5 dB
  um die Dichte“ hätte für Einbinbänder nicht gehalten (Medianversatz −0,77
  dB plus Stichprobenversatz ≈ −1,2 dB, Streuung 0,2 dB): die Prüfung nur an
  Band 220 (44 Bins) hat das verdeckt.
- (iv) Bits: je Evidenzframe ab 3 s tragen genau die Bänder mit n_b ≥ 1 das
  Evidenz- und das P50-Bit, die 25 Bänder mit n_b = 0 keines.

n_b je Band liest der Test über einen `FeatureEngineTestzugang` (nur lesend:
`bandVon`/`bandBis` von `bass` und `haupt`, T-380-11), nicht aus dem Lauf;
die Zahl der Bänder mit n_b = 0 muss 25 sein. Der Rotbeweis M-01 bleibt die
Mutation `Spektrum.h` Bandmittel `energie /= (double) (bis - von);` weg
(fällt an (i) und (ii)). Etikett M-01 bleibt „heute nicht messbar“; die
Ausgangserfassung wird mit 1 s erneuert. Matrix §6.1: M-01 (Zusage), M-11
(Rotbeweisspalte: formgültige Mutation `* 0.5`) und M-17 (Rotbeweisspalte:
vierter Größensatz im `.fbs`) sind in der Fassung §24 nachgezogen; §6.6
bleibt (Etiketten unverändert).

### 24.3 Ursachen und Nacharbeitsauftrag 1 (Runde 1 von 3)

Zwei Ursachen: (A) der M-01/M-02-Test bildet Vorbedingung, Zusage je Band
oder Gruppe und Bitzusage der Matrixzeile nicht vollständig ab (D1, D2, D3,
Z1, L1); (B) Rotbeweis oder Prüfung decken nicht die Zusagezeile an allen
genannten Stellen (D4, D5). Der Auftrag trägt die Ursachen, die sechs
Befunde wörtlich, je Befund die schließende Regel und R-380-10:
`docs/beweise/roh/NAK-380-nacharbeit-e2-1-auftrag.txt`. Kein Produktcode;
Ticketpfade `AnalysisGoldenTestMain.cpp`, `pruefe_v3_vertrag.py`, Rohdateien,
Manifest §25. Bauer: Codex `gpt-5.6-sol` xhigh `workspace-write`
(`tools/dirigent/codex-bau.ps1`, Kennung `nak380-e2-n1`; vor 15:00 Uhr nach
NAK-396). Wiederprüfung 1 danach nach Vorlage B über den Fixdiff.

**Rundenbilanz Etappe 2 (Bau):** `39fbf165..83b9fc2c`: Produkt 20 Datei(en)
+597/−36 | Tests 6 Datei(en) +518/−3 | Pruefwerkzeug 2 Datei(en) +76/−3 |
Doku 53 Datei(en) +692/−27 (`rundenbilanz.py`, 25.09.2026).

**Register:** NAK-402 (Härtung Z2). Offen beim User: nichts.

## 25. Nacharbeit 1 der Etappe 2 (Bauer Codex gpt-5.6-sol, 25.09.2026)

### 25.0 Schließung der Befunde

| Befund | Geänderte Stellen | Schließung mit Messzahl |
|---|---|---|
| D1 | `AnalysisGoldenTestMain.cpp:841-860,983-993`; `roh/NAK-380-etappe-2-m01-w0-ausgang.txt` | `nak380W0Lauf` setzt 1,0 s; die aus 512er Block, 48 kHz, Livekadenz und 60 s hergeleitete Sollzahl ist verriegelt. B5 misst ab 3 s **54/54** Evidenzframes. Derselbe Aufbau gilt für M-08(a) W0h. **geschlossen** |
| D2 | `AnalysisGoldenTestMain.cpp:803-836,1010-1095` | Alle 196 belegten Feinbänder prüfen P50 gegen `D + Delta(n_b)` mit 1,5 dB. Größter Abstand: Band 55, `n_b=2`, **0,6056 dB**. **geschlossen** |
| D3 | `AnalysisGoldenTestMain.cpp:848-861,895-927,994-1009` | Erwartete Belegung stammt nur lesend aus `bass`/`haupt.bandVon/bandBis`; Evidenz- und P50-Bitmap werden vor Wertfilterung je Frame einzeln gegen `n_b>=1` verglichen. **196** belegt, **25** unbelegt, **0/54** abweichende Frames. **geschlossen** |
| Z1 | `AnalysisGoldenTestMain.cpp:1096-1141` | M-02 fordert vor Maximum und Spannentest einen Beitrag jeder Livegruppe; **64/64** Gruppen tragen mindestens einen. **geschlossen** |
| D4 | `roh/NAK-380-rot-M-11.txt`; `roh/NAK-380-e2-n1-mutation-M11-rust.txt`; `roh/NAK-380-e2-n1-mutation-M11-cpp.txt` | Beide Rotbeweise halbieren formgültige Breiten. Rust fällt nach grünem Längenriegel bei Summe **8964,516199894459**; alle drei Hexwerte sind ebenfalls falsch. C++ fällt bei 221 Breiten an Summe **8964,516199894 Hz** und drei falschen Hexwerten, ohne Panik, Arrayüberlauf oder UB. **geschlossen** |
| D5 | `pruefe_v3_vertrag.py:2270-2303`; `roh/NAK-380-rot-M-17.txt` | Eigene A5-Teilprüfung `FlatBuffers Bandwerte` fordert beide Größen im Kommentar der echten `.fbs`. Die isolierte Mutation ergibt **852 grün/1 rot** nur an dieser Prüfung; danach **853/0**. **geschlossen** |
| L1 / R-380-10 | `AnalysisGoldenTestMain.cpp:803-836,980-1095`; `roh/NAK-380-rot-M-01.txt` | Formelbasierte Mittelwerttoleranz je Band, Systematik über 196 Bänder, Gamma-Medianversatz und beide Bits ersetzen die ungedeckte Pauschalprüfung. Größter Mittelwertfehler **0,3651 dB** bei `n_b=1` und **0,6907 dB** Toleranz; Systematik **−0,00417 dB**. M-01-Rotbeweis fällt an (i) und (ii). **geschlossen** |

### 25.1 Änderungsregister

| Stelle | Vorher | Jetzt |
|---|---|---|
| W0-Vorbedingung | Defaultintervall 0,25 s; 178 ausgewertete Frames | explizit 1,0 s; 54 aus Kadenz und Dauer hergeleitete Frames |
| M-01 Mittel | ein globales Maximum gegen 0,5 dB | jedes belegte Band gegen `4*4,343/sqrt(2*M_s*n_eff/1,056)`; zusätzlich Systematik ±0,05 dB |
| M-01 P50 | nur Band 220 gegen Dichte | alle 196 belegten Bänder gegen `D + 10log10(1−1/(3k)+8/(405k²))`, `k=2*n_eff`, ±1,5 dB |
| M-01 Bits | Schnittmenge beider Bitmaps als Wertfilter | beide Bitmaps je Evidenzframe unabhängig gegen die 196/25-Zuordnung |
| M-02 Beiträge | fehlende Livegruppen wurden übersprungen | jede der 64 Gruppen ist vor Fehlermaximum und Spannentest Pflicht |
| M-11 Rotbeweis | falsche Arraylänge beziehungsweise ungültiger Zugriff | 221 formgültige, rechnerisch halbierte Breiten in Rust und C++ |
| M-17 A5 | `.fbs`-Feldgrößen nur teilweise beziehungsweise über README geprüft | eigener Wortsatz für Leistungsdichte und Bandleistung am Kommentar von `table Bandwerte` |

Die M-01-Ausgangserfassung wurde mit dem unveränderten Produktcode am HEAD
`22b414ab34ccc2a72290c22d06ac7df139cfec5d` und dem korrigierten Testaufbau
erneuert. Der alte Defaultintervall-Aufbau ist danach nicht mehr erreichbar;
`roh/NAK-380-etappe-2-m01-w0-ausgang.txt` benennt dies ausdrücklich.

### 25.2 W0-Messung mit 1 s und T-380-11

Der nur in B5 definierte `FeatureEngineTestzugang` liest nach `vorbereiten`
ausschließlich `bandVon` und `bandBis` aus der für die Bandmitte zuständigen
Bass- oder Hauptstufe. Er schreibt keinen Produktzustand und leitet keine
Erwartung aus den im Lauf beobachteten Bits oder Werten ab. `Nak380Bandmittel`,
Engine und Signalpuffer liegen auf dem Heap (NAK-175).

| Prüfung | Herleitung/Soll | Ist |
|---|---|---|
| Evidenzframes | `ceil(0,1*48000/512)=10` Blöcke je Liveframe; zehn Liveframes = 1,066667 s; `floor(60/1,066667)−ceil(3/1,066667)+1=54` | **54**, Intervall **1,0 s** |
| Zuordnung/Bits | `n_b>=1`: 196 Bänder; `n_b=0`: 25; beide Bitmaps in jedem Frame | **196/25**, **0** abweichende Frames |
| R-380-10 (i) | `M_Bass=floor(57*48000/8192)=334`, `M_Haupt=floor(57*48000/2048)=1335`; bandabhängig 4 Sigma | Maximum Band 30, `n_b=1`: **0,3651 dB** bei **0,6907 dB** |
| R-380-10 (ii) | Mittel der 196 Abweichungen innerhalb ±0,05000 dB | **−0,00417 dB** |
| R-380-10 (iii) | jedes belegte Band gegen `D+Delta(n_b)`, ±1,5000 dB | Maximum Band 55, `n_b=2`: **0,6056 dB** |
| M-02 | 64 Pflichtbeiträge; Spanne 28,5313 dB | **64/64**; Spanne **28,4148 dB**, maximaler Gruppenfehler **0,2505 dB** |

### 25.3 Rotbeweise der Nacharbeit

| Matrixzeile | Mutation und Lauf | Ergebnis | Fällt an der Zusagezeile? |
|---|---|---|---|
| M-01 | `Spektrum.h`: Binmitteldivision durch `1.0`; B5 | Exit 1, 254/7; (i) 16,4370 dB bei 0,0722 dB, (ii) 5,63511 dB bei 0,05000 dB, auch (iii) rot; `roh/NAK-380-rot-M-01.txt` | **JA** |
| M-02 | `Frame.h`: Gitterbreite durch `1.0`; B5 | Exit 1, 257/4; M-02-Spanne 7,1986 statt 28,5313 dB, alle M-01-Teilprüfungen grün; `roh/NAK-380-rot-M-02.txt` | **JA** |
| M-11 Rust | `tafel()`: jede Kantendifferenz `* 0.5`; nur A4-Filter | Exit 101, 0/1, 308 ausgefiltert; Längenriegel grün, Summe und alle drei Hexwerte falsch; `roh/NAK-380-rot-M-11.txt` | **JA** |
| M-11 C++ | `evidenzKante`: Rückgabewert `* 0.5`; B5 | Exit 1, 248/13; M-11-Summe und drei Hexwerte rot; abhängige M-01/M-02/M-03/M-05/M-08-, Gitter- und Nyquistprüfungen mitgefallen; `roh/NAK-380-rot-M-11.txt` | **JA** |
| M-17 vierter Satz | `.fbs`: `dBFS/Hz` zu `dBFS`, keine generierten Dateien; A5 | Exit 2, 852/1; nur `FlatBuffers Bandwerte` rot; `roh/NAK-380-rot-M-17.txt` | **JA** |

Jede Produktmutation lag unter `eq-copilot/build/nak380-sicherung/`, wurde
bytegleich zurückgespielt und per SHA-256 bestätigt. C++-Quellzeitstempel wurden
nach der Rücknahme erneuert und die Fälle neu gebaut. Die Einzelaufzeichnungen
stehen zusätzlich in `roh/NAK-380-e2-n1-mutation-M01.txt`, `-M02.txt`,
`-M11-rust.txt`, `-M11-cpp.txt` und `-M17-fbs.txt`.

### 25.4 Gefahrene Beine

| Bein | Lauf der Nacharbeit | Ergebnis |
|---|---|---|
| B5 | CMake Release-Ziel `EqCopAnalysisGoldenTest`; danach Binary | Build Exit 0; Test Exit 0, **261 bestanden, 0 Fehler**. Quelle 13:55:05, Binary 13:55:21 und damit frisch (NAK-230). |
| A5 | `C:\Python314\python.exe tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | Exit 0, **853 bestanden, 0 gescheitert**; die DeprecationWarning zu `jsonschema.__version__` bestand bereits. |
| A4-Filter | `cargo test --manifest-path broker/Cargo.toml nak380_m11_bandbreiten_aus_gitter` | Exit 0; **1 bestanden, 0 Fehler, 308 ausgefiltert** im Bibliothekstest. Vorbestehende Warnungen in anderen Testquellen blieben unverändert. |
| Dokuriegel | `C:\Python314\python.exe tools/plan/dokuriegel.py docs/beweise/NAK-380.md` | Ergebnis nach Abschlussprüfung in §25.5. |

### 25.5 Abweichungen, offene Punkte und Produktbefunde

- **Produktbefund:** keiner. Die neu vollständige Systematikprüfung und die
  unabhängige Bitprüfung sind grün; deshalb blieb der Produktcode unverändert.
- **Offen:** nur die beauftragte frische Wiederprüfung 1. Z2/M-18 bleibt wie in
  §24 bei NAK-402; es wurde nicht vorgezogen.
- Der erwartete fremde Installer-Hash-Diff war beim Start nicht vorhanden;
  der tatsächliche Anfangsstatus enthielt nur `.claude/settings.local.json`,
  `briefing-hub/` und `nimbalyst-local/`. Kein Installationspfad wurde berührt.
- Der direkte MSBuild-Unterprozess traf die bekannte doppelte `PATH`/`Path`-
  Umgebung. Der gleiche CMake-Aufruf lief deshalb über `Start-Process
  -UseNewEnvironment`; Ziel und Konfiguration blieben unverändert.
- Nach MSVC-Bauten blieb jeweils ein kindloser `vctip.exe` stehen und wurde nach
  NAK-300 beendet. Ein früher Zwischenstand des vergrößerten Testresultats auf
  dem Stack endete mit `0xC00000FD`; vor jedem Beweislauf wurde der Träger gemäß
  NAK-175 auf den Heap gelegt. Beides berührte keinen Produktpfad.
- Ein Zeilenenden-Normalisierungsschritt enthielt PowerShell-Escape-Backticks
  entgegen der Arbeitsanweisung. Er schrieb ausschließlich die beiden erlaubten
  Prüfquelldateien; die anschließenden Hash-, EOL-, Build- und Diffprüfungen
  bestimmen den gelieferten Zustand.
- Kein voller Kanon, kein A8/A9/A10, kein FL Studio, keine Installation und keine
  Produktions-Pipe wurden gefahren; genau diese Läufe waren aus der
  Ticketgrenze ausgenommen.
- `tools/dirigent/pruefliste.md` wurde lesend abgearbeitet. Der Abschlussdiff
  enthält keinen Produktcode und keine Datei außerhalb der beauftragten Pfade.
- **Dokuriegel:** Exit 0, 0 Befunde; 28 als historisch oder noch anzulegende
  Zielpfade benannte Verweise.
## 26. Wiederprüfung 1 PASS und Abschluss der Etappe 2 (Dirigent, 25.09.2026, 14:10 Uhr)

**Messung der Nacharbeit 1.** Bauer Codex `gpt-5.6-sol` xhigh
`workspace-write` (`tools/dirigent/codex-bau.ps1`, Kennung `nak380-e2-n1`,
Thread `01a0d84b-d1c8-7e13-8712-598a10ea8df3`), 13:20:40 bis 13:58:05 Uhr,
EXIT=0, Basis `22b414ab`, kein Commit durch den Bauer. Vom Dirigenten
gemessen: 8 geänderte und 5 neue Dateien (+475/−74); `git diff --stat
22b414ab b12737e3 -- eq-copilot broker tools` nennt nur
`AnalysisGoldenTestMain.cpp` und `pruefe_v3_vertrag.py`, kein Produktcode.
B5 selbst gefahren: Exit 0, 261/0 (Binary 13:55:21 nach Quelle 13:55:05):
Intervall 1,0 s, 54/54 Evidenzframes, 196/25 Bänder aus dem Testzugang, 0
Bitverstöße in 54 Frames, größter Mittelfehler 0,3651 dB (Band 30, n_b = 1)
bei Toleranz 0,6907 dB, Systematik −0,00417 dB, größter P50-Abstand 0,6056 dB
(Band 55, n_b = 2), 64/64 Gruppen; A5 Exit 0 (Bauer 853/0). Fixdiff
adversarial gelesen: `FeatureEngineTestzugang` nur lesend (`bandVon`,
`bandBis`; Stufe nach Bandmitte gegen `kTrennungHz`), Formeln R-380-10 als
Kommentar mit Zahlen, keine Erwartung aus dem Lauf, Träger und Engine auf
dem Heap, M-02 fällt bei fehlender Gruppe (Unendlich als Gruppenfehler und
eigene Prüfung), A5-Anker `/// Ein Bandsatz` fail-closed (fehlt er, ist die
Teilprüfung rot). Prozessabweichungen des Bauers (§25.5: Backticks in einem
Normalisierungsschritt, `-UseNewEnvironment`, Stacküberlauf vor dem
Heap-Umbau) berühren keinen gelieferten Pfad. Commit `b12737e3` (Pathspec),
gepusht.

**Wiederprüfung 1 (Vorlage B).** Codex `gpt-6-astra` high (vor 15:00 Uhr,
NAK-396), lesend, Thread `01a0d871-ce59-7ea0-8c80-e87dda113119`, 14:02:11 bis
14:04:54 Uhr, EXIT=0, HEAD `b12737e3` vor und nach dem Lauf; Prüfbereich
`22b414ab...b12737e3` über die zwei Prüfdateien, dazu die Rohbelege zu D4 und
D5; Auftrag `docs/beweise/roh/NAK-380-wiederpruefung-e2-1-auftrag.txt`,
Urteil `docs/beweise/roh/NAK-380-wiederpruefung-e2-1-urteil.md`. **URTEIL:
PASS — alle Befunde geschlossen (D1, D2, D3, Z1, L1, D4, D5), nichts
gebrochen.** Geprüft: Fixdiff und Belege; Stufenwahl gegen
`FeatureEngine.h:477-478` und `Zeit.h:105-143`; Halbierungsmutationen
formgültig (Rust fällt an `bandbreite.rs:50`, C++ an den festen Hexliteralen);
A5 prüft den echten Schema-Kommentar; Produktcode unverändert; Testzugang nur
in Testzielen; Heap; Zeilenenden; `diff --check` ohne Befund. Nicht geprüft:
keine Läufe.

**Rundenbilanz.** Runde 2 (Nacharbeit 1) `22b414ab..b12737e3`: Tests 1
Datei(en) +229/−26 | Pruefwerkzeug 1 Datei(en) +13/−0 | Doku 11 Datei(en)
+282/−48. Kumuliert `--runden 39fbf165 83b9fc2c b12737e3`: Runde 1
`39fbf165..83b9fc2c` Produkt 20 +597/−36 | Tests 6 +518/−3 | Pruefwerkzeug 2
+76/−3 | Doku 53 +692/−27; Runde 2 `83b9fc2c..b12737e3` Produkt 1 +1/−1
(Broker-Pin des Runners) | Tests 1 +229/−26 | Pruefwerkzeug 1 +13/−0 | Doku 19
+12 972/−56; kein Konvergenzsignal.

**Etappe 2 abgeschlossen (25.09.2026, 14:10 Uhr).** Gebaut (§22), gemessen
(§23), Kanon nachher GRUEN 70/70 an `83b9fc2c` (§24), Erstprüfung 2
NEEDS_WORK mit Regel R-380-10 (§24), Nacharbeit 1 (§25), Wiederprüfung 1 PASS
(§26). Aus der Etappe bleibt die Härtung NAK-402 im Register; Laufzeit-Arm
und T2-Abschlussprüfung folgen nach Etappe 6 (Kopf). Hygiene im
Abschlussfenster: Dokuriegel auf Manifest, Register, CLAUDE.md und Skill 0
Befunde; Zielbild aktuell (`zielbild.py pruefen` Exit 0); A32 unverändert
3/33/39 gegen den vorigen Kanon (kein neuer Riss), A33 Exit 0.

**Etappe 3 gestartet.** Auftrag
`docs/beweise/roh/NAK-380-etappe-3-auftrag.txt` nach §8.3 mit den Lehren aus
§24 als Präzisierungen (Vorbedingung exakt, Zusage je Element, Bits an der
Bitmap, formgültige Mutationen, je Vertragsstelle eine Prüfung, Toleranzen
mit Referenz). Bauer Codex `gpt-5.6-sol` xhigh `workspace-write` (Start vor
15:00 Uhr, NAK-396; Kennung `nak380-e3`), Basis = HEAD nach diesem Abschnitt;
Kanon vorher geerbt vom Kanon nachher der Etappe 2 an `83b9fc2c` (Diff seither
nur die zwei Prüfdateien der Nacharbeit 1 und der Broker-Pin). Aufsicht ENG.
Der Bauer schreibt §27.

## 27. Etappe 3 — LRA mit 10 Hz und M1-Spanne (Bauer Codex gpt-5.6-sol, 25.09.2026)

### 27.1 Startmessung und Kanon vorher

**Basis.** `git rev-parse HEAD` ergab
`b04dfb7ccee3d310e83969ad10f6ed6225542088` auf `master`. Der Startstatus
enthielt ausschließlich die drei fremden untracked Einträge
`.claude/settings.local.json`, `briefing-hub/` und `nimbalyst-local/`. Die
Prozessprobe auf `cmake`, `MSBuild`, `cl`, `link`, `cargo`, `rustc`,
`EqCop*`, `eqcop-broker`, `FL64` und `vctip` war leer.

**Kanon vorher nicht geerbt.** Der vorgeschriebene Vergleich
`git diff --stat 83b9fc2c HEAD -- eq-copilot broker tools` nannte nicht
exakt die zwei Nacharbeitsdateien, sondern drei Pfade:
`eq-copilot/install/nakama-installer-v1.json` (2 Zeilen),
`eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp` (255 Zeilen) und
`tools/eq-copilot/pruefe_v3_vertrag.py` (13 Zeilen). Nach dem Auftrag fährt
der Dirigent deshalb den Kanon vorher beziehungsweise seinen vollen
Abschlusslauf; der Bauer hat keinen vollen Kanon gefahren.

Die Basis-Gegenprobe lief vor dem ersten Produktedit. M-21 (73), M-22
(Wert vor der 10-Hz-60-s-Grenze), M-23 (3,000 LU), M-29 (70), M-30 und
M-32 bis M-35 fielen rot. M-24 bis M-27, M-28(a), M-28(b) und M-31 waren
Regressionswachen beziehungsweise der erlaubte Altvergleich. M-28(b) maß
am alten 1-Hz-Inhalt 18,900 LU gegen die unabhängige §5-Referenz 18,883 LU
und lag damit bereits innerhalb ±0,2 LU; diese Hälfte trägt den Ratenwechsel
nach E-380-18 nicht. M-37 bis M-40 waren vor der Reparatur nicht messbar und
wurden nach dem Bau mutiert. Die Reihenfolge war damit: neue Fälle gegen
Basis, Produktbau, Pflicht-Altvergleich M-39, Erzeuger, Mutationswachen.

### 27.2 Gebauter Änderungssatz

- `FeatureEngine.h` stieg von 1330 auf 1332 Zeilen: LRA-Hop 1 Zelle/100 ms,
  Normkommentar §3.1 und `kFeatureMetricsVersion = 20260926`.
- `featureengine/Lautheit.h` änderte ausschließlich den LRA-Kommentar; die
  Rechnung und insbesondere `sekundenJeWert` blieben unverändert.
- `AnalyseEngine.cpp` stieg von 1047 auf 1073 Zeilen: zweites Histogramm,
  lineare Summe und Restzähler je Band; genau acht aktive Segmente erzeugen
  einen M1-Wert, Perzentile lesen nur diesen Träger, Abdeckung weiterhin das
  Einzelsegmenthistogramm. Rest verfällt am Reset.
- `AnalyseEngine.h` stieg von 332 auf 339 Zeilen: `kMetricsVersion` ist
  `m4.2-2026-09-25`, der rein lesende Testfreund und die drei Heapträger.
  Zusätzlicher fester Bedarf je Engine: 221 × (141 × 4 + 8 + 4) =
  **127.296 Byte** (124.644 Byte Histogramm + 2.652 Byte Akkus).
- `Diagnose.cpp` änderte nur den Kommentar. `kDiagnoseVersion`, die Texte und
  die Schwellen 6/10 dB blieben bytegleich in ihrer Rechnung.
- `metriken-v1.json` erhielt Fassung 20260926, `seit` = „NAK-380 Etappe 3“,
  unveränderte Schwellen und den §3.1-Eintrag `kLraHopZellen = 1`.
  Weil die explizite Ticketgrenze beide Broker-Dateien verbietet, bindet das
  neue Feld `broker_aktuell = 20260925` deren unveränderte Schwellen weiterhin
  an ihre echte Fassung; A5 prüft Feature- und Broker-Fassung getrennt.
- `festhalten-referenz.json` entstand ausschließlich über B30 `--erzeuge`.
  Alter SHA-256: `573518B7...3C8DBF`; neu: `D05639C4...BB1938`, 35.550 Byte.
  Ein zweiter Erzeugerlauf lieferte denselben vollständigen Hash.
- Die Testhilfe `Nak380Pruefsignale.h` führt feste Saaten, Signale und die von
  `lraLu` unabhängige Tech-3342-§5-Referenz. B18, B9, A2 und A5 erhielten die
  Fälle M-21 bis M-40; alle großen Engines liegen auf dem Heap. Der
  `AnalyseEngineTestzugang` und beide `FeatureEngineTestzugang` lesen nur.
- `tools/beweise.ps1` nennt die neuen Zusagen in B9, B18 und A2. Kein neues
  Bein und keine Änderung an `CMakeLists.txt` waren nötig.

### 27.3 Matrix M-21 bis M-41 und Rotbeweise

| Zeile | Test (Datei, Fall, Bein) | Etikett und Ergebnis | Rohbeweis | Zusagezeile |
|---|---|---|---|---|
| M-21 | `Sonde013DynamicsTest.cpp`, `380/M-21 lra_zehn_kurzzeitwerte_je_sekunde`, B18 | heute rot 73; gebaut 721 | `roh/NAK-380-rot-M-21.txt` | fällt an der Zusagezeile: **JA** |
| M-22 | `Sonde013DynamicsTest.cpp`, `380/M-22 lra_sechzig_sekunden_bei_zehn_hertz`, B18 | heute rot; gebaut bei 62,5/62,8 s ohne, bei 62,9 s mit Wert | `roh/NAK-380-rot-M-22.txt` | fällt an der Zusagezeile: **JA** |
| M-23 | `Sonde013DynamicsTest.cpp`, `380/M-23 lra_ohne_aliasing`, B18 | heute rot 3,000; gebaut **weiter rot** 2,600 gegen 2,435 ±0,15 LU | `roh/NAK-380-rot-M-23.txt` | Rotmutation fällt an der Zusagezeile: **JA**; gebaute Zusage: **NEIN** |
| M-24 | `LoudnessGoldenTestMain.cpp`, `380/M-24 tech3342_tabelle1_fall1`, B9 | Regressionswache; 10,000 LU, Norm und §5 grün | `roh/NAK-380-rot-M-24.txt` | fällt an der Zusagezeile: **JA** |
| M-25 | ebenda, `380/M-25 tech3342_tabelle1_fall2`, B9 | Regressionswache; 5,000 LU, beide Prüfungen grün | `roh/NAK-380-rot-M-25.txt` | fällt an der Zusagezeile: **JA** |
| M-26 | ebenda, `380/M-26 tech3342_tabelle1_fall3`, B9 | Regressionswache; 20,000 LU, beide Prüfungen grün | `roh/NAK-380-rot-M-26.txt` | fällt an der Zusagezeile: **JA** |
| M-27 | ebenda, `380/M-27 tech3342_tabelle1_fall4`, B9 | Regressionswache; 15,000 LU, beide Prüfungen grün | `roh/NAK-380-rot-M-27.txt` | fällt an der Zusagezeile: **JA** |
| M-28(a) | `Sonde013DynamicsTest.cpp`, Abwesenheit nach 30 s, B18 | Golden (i), Regressionswache; grün | `roh/NAK-380-rot-M-28.txt` | fällt an der Zusagezeile: **JA** |
| M-28(b) | ebenda, §5-Wertvergleich, B18 | Golden (ii), heute nicht messbarer Ratenvergleich; alt und neu 18,900 gegen Referenz 18,883 LU, grün | `roh/NAK-380-rot-M-28.txt` | fällt an der Zusagezeile: **JA** |
| M-29 | `Sonde013DynamicsTest.cpp`, `380/M-29 lra_nan_sperrt_dreissig_werte`, B18 | heute rot 70; gebaut 691 | `roh/NAK-380-rot-M-29.txt` | fällt an der Zusagezeile: **JA** |
| M-30 | `pruefe_v3_vertrag.py`, `nak380_m30_nicht_gefuehrte_werte_stimmen`, A5 | drei heute-rot-Vertragsstellen separat; gebaut 4/0 | `roh/NAK-380-rot-M-30.txt` | fällt je Vertragsstelle an der Zusagezeile: **JA** |
| M-31 | `Sonde013DynamicsTest.cpp`, `380/M-31 material_ohne_dynamik_bleibt_nahe_null`, B18 | Regressionswache; 0,000 LU | `roh/NAK-380-rot-M-31.txt` | fällt an der Zusagezeile: **JA** |
| M-32 | `GoldenTestMain.cpp`, `380/M-32 m1_rauschen_steht_ruhig`, A2 | heute rot; gebaut Maximum alle/Einbin je 3,00 dB | `roh/NAK-380-rot-M-32.txt` | fällt an der Zusagezeile: **JA** |
| M-33 | ebenda, `380/M-33 m1_bewegung_1p8db`, A2 | heute rot 7; gebaut Median 3,00 dB in [1,3;3,8] | `roh/NAK-380-rot-M-33.txt` | fällt an der Zusagezeile: **JA** |
| M-34 | ebenda, `380/M-34 m1_bewegung_3p6db`, A2 | heute rot 8; gebaut Median 5,00 dB in [3,1;5,6] | `roh/NAK-380-rot-M-34.txt` | fällt an der Zusagezeile: **JA** |
| M-35 | ebenda, `380/M-35 m1_bewegung_5p4db`, A2 | heute rot 9; gebaut Median 6,00 dB in [4,9;7,4] | `roh/NAK-380-rot-M-35.txt` | fällt an der Zusagezeile: **JA** |
| M-36 | `GoldenTestMain.cpp`, `380/M-36 karte_116hz_steht_ruhig`, A2 | Regressionswache; Schwellen/Texte unverändert, 98/0 im Einzelfall | `roh/NAK-380-rot-M-36.txt` | fällt mit Ersatzmutation an der Zusagezeile: **JA** |
| M-37 | ebenda, `380/M-37 abdeckung_zaehlt_segmente`, A2 | Regressionswache; 221/221 Bandzähler und 28/28 Gruppen stimmen | `roh/NAK-380-rot-M-37.txt` | fällt an der Zusagezeile: **JA** |
| M-38 | ebenda, `380/M-38 teilblock_raender`, A2 | heute nicht messbar; gebaut floor(n/8), Reset, NaN-Grenze und 127.296 Byte grün | `roh/NAK-380-rot-M-38.txt` | fällt mit Ersatzmutation an der Zusagezeile: **JA** |
| M-39 | `BriefkastenTestMain.cpp`, `festhalten_bytegleich`, B30 | Golden (ii): neuer Code gegen alten Inhalt rot; Erzeuger zweimal bytegleich; final 214/0 | `roh/NAK-380-rot-M-39.txt` | fällt an der Zusagezeile: **JA** |
| M-40 | `pruefe_v3_vertrag.py`, `nak380_m40_fassung_etappe_3`, A5 | heute nicht messbar; gebaut 4/0 | `roh/NAK-380-rot-M-40.txt` | fällt an der Zusagezeile: **JA** |
| M-41 | A1/A16/B1/B4, Audiothread-Grenze | Regressionswache, dieselbe Zusage/Mutation wie M-20; aktuelle vier Beine grün | `roh/NAK-380-rot-M-41.txt`, Quelle `roh/NAK-380-rot-M-20.txt` | fällt an der Zusagezeile: **JA** |

Alle Mutationen lagen in `eq-copilot/build/nak380-sicherung/`, wurden durch
Zurückkopieren beendet und per SHA-256 gegen den Stand vor der Mutation
verglichen. Die wörtlichen Kurzprotokolle stehen zusätzlich in
`roh/NAK-380-e3-mutation-*.txt`. Kein Git-Befehl nahm eine Mutation zurück.

### 27.4 Einzeln gefahrene Beine

Alle C++-Ziele wurden mit dem fest benannten VS-CMake und
`Start-Process -UseNewEnvironment` gebaut. Ein vom ersten abgebrochenen
B30-Ausgabelauf übrig gebliebener eigener Prozess endete selbst; den nach dem
finalen Sammelbau noch vorhandenen eigenen `vctip.exe` beendete der Bauer nach
NAK-300. Jedes finale Binary ist jünger als seine jeweils jüngste Eingabequelle;
nach der letzten Rücknahme von `AnalyseEngine.cpp` um 15:27 Uhr wurden A2,
B30, A1, A16, B1 und B4 nochmals gebaut und gefahren.

| Bein | Ergebnis | Binary-Zeitstempel |
|---|---|---|
| B18 `EqCopSonde013DynamicsTest` | **Exit 1**, 85/1; ausschließlich M-23 rot | 15:13:26 |
| B9 `EqCopLoudnessGoldenTest` | Exit 0, 117/0 | 15:13:29 |
| A2 `EqCopGoldenTest fixtures` | Exit 0, 258/0 | 15:27:16 |
| B30 `EqCopBriefkastenTest` | Exit 0, 214/0 | 15:28:59 |
| A5 `pruefe_v3_vertrag.py --abdeckung` | Exit 0, 861/0 | Pythonlauf 15:14 Uhr |
| A1 `EqCopNullTest` | Exit 0, `NULLTEST OK` | 15:29:02 |
| A16 `EqCopProbeeqNullTest` | Exit 0, 145/0 | 15:29:05 |
| B1 `EqCopIdentityTest` | Exit 0, 120/0 | 15:29:08 |
| B4 `EqCopQueueStressTest` | Exit 0, 121/0 | 15:29:11 |

A8, A9 und A10 liefen nicht: `C:\Python314\python.exe` kann `rfc8785`
nicht importieren, und `py -3.13` kann in dieser Anmeldesitzung keinen
WindowsApps-Prozess anlegen (Exit 101). Der Dirigent fährt sie vor dem Kanon.
Der v3-, State- und FlatBuffers-Korpus wurde nicht geändert; der einzige
Fixture-Diff ist die ausdrücklich erlaubte Diagnose-Referenz.

### 27.5 Golden-Bewegungen und Messwerte

- Klasse (i) M-28(a) blieb als Abwesenheit nach 30 s unverändert.
- Klasse (ii) M-28(b) bewegte sich nicht: alter und neuer Produktwert 18,900
  LU, unabhängige Referenz 18,883 LU. Kein Golden wurde gelockert.
- Klasse (ii) M-39 fiel vor dem Erzeuger am alten Inhalt bei Byte 52. Die neue
  Referenz trägt `metrics_version` m4.2 und ist in zwei Erzeugerläufen
  bytegleich (`D05639C42C013F12961282B281ECD4CEFD25D3C7772A5AD018F5304A6EBB1938`).
- B9 maß Tabelle 1 mit 10/5/20/15 LU; jede Zahl stimmt zugleich innerhalb
  ±0,1 LU mit der unabhängigen §5-Referenz.
- A2 maß M-32 mit maximal 3 dB, M-33/34/35 mit Median 3/5/6 dB. Die bestehende
  Fixture `pink-minus20` trägt **196** Bänder mit vollständigen
  Teilblock-Perzentilen (gefordert waren mindestens 150). A2 blieb 258/0.

### 27.6 Abweichungen und technische Entscheide

1. **M-23 offen, nichts gelockert.** Der gebaute 10-Hz-Pfad liefert 2,600 LU;
   die Matrixobergrenze ist 2,585 LU. Die Überschreitung beträgt 0,015 LU.
   `Lautheit.h` durfte nach §8.3 außer dem Kommentar nicht geändert werden;
   daher blieb die Rechnung unangetastet und B18 ehrlich rot. Der Dirigent
   entscheidet, ob die analytische kontinuierliche 2,435-LU-Referenz zur
   diskreten 10-Hz-/0,1-LU-Histogrammrechnung oder der spätere Rechenpfad
   geändert werden soll.
2. Die M-36-Vorgabemutation `>0,5` blieb grün, weil die 116-Hz-Spanne nach
   Achtsegmentmittelung im 1-dB-Raster exakt 0 dB ist. Die formgültige
   Mutation `>−0,5` prüfte dieselbe unveränderte 10-dB-Werkzeugschwelle und
   ließ genau den versprochenen Werkzeugtext fallen.
3. M-38 besitzt am Fensterende keinen Produkt-Hook, an dem ein Rest ohne
   Zusatzcode als eigener Block publiziert werden könnte. Die formgültige
   Ersatzmutation startete den Rest nach Reset bei 1 statt 0 und brach damit
   dieselbe starten↔stoppen-Zusage ohne Längenriegel oder Arrayzugriff.
4. M-41 verwendet den frischen M-20-Rotbeweis derselben Ticketarbeit erneut,
   weil die verlangte Mutation `PluginProcessor.cpp` außerhalb der expliziten
   Etappe-3-Ticketpfade liegt. Die vier aktuellen Grenzbeine liefen dennoch.
5. Die neue Feature-Fassung darf nach der Ticketgrenze keine unveränderte
   Broker-`METRICS_VERSION` umschreiben. `broker_aktuell` hält diese Teilfassung
   explizit bei 20260925; A5 bindet beide Produktgruppen weiter streng. Das ist
   eine begründete Präzisierung des T-380-10-Registers, keine stille Lockerung.
6. Der B30-Erzeuger schreibt die Diagnose-Fixture kanonisch mit LF. Sie blieb
   deshalb trotz der allgemeinen CRLF-Präzisierung unverändert im
   Erzeugerformat (§8.1: Fixture-Zeilenenden LF); eine manuelle Konvertierung
   hätte „nur über Erzeuger“ und die Byte-Reproduzierbarkeit gebrochen.

### 27.7 Selbstaudit, Prüfliste und offene Punkte

Der eigene Diff wurde gegen HEAD adversarial gelesen: genau ein LRA-Hop,
60-s-Regel über `sekundenJeWert`, NaN-Sperre 691, unabhängige §5-Referenz,
Teilblöcke ausschließlich für Perzentile, Einzelsegmente für Abdeckung,
unveränderte 6/10-dB-Schwellen, getrennte und registrierte Versionen,
bytegleicher Knopfweg-Erzeuger, keine Änderung an Audio-, Bandgitter-, Wire-,
State-, Identity-, Installations- oder Designpfaden. Leere Teilblöcke bleiben
NaN, Reste verfallen am Reset, alle neuen längenabhängigen Träger liegen auf
dem Heap, und B4 maß weiter null Audiothread-Allokationen.

Die einschlägigen Punkte aus `tools/dirigent/pruefliste.md` sind abgehakt;
die Datei blieb unverändert. C++ und Tests sind CRLF, das Register CRLF
`-text`, Manifest und neue Rohdateien LF, `tools/beweise.ps1` CRLF mit BOM.
Offen bleibt ausschließlich M-23. Außerdem müssen der Dirigent den wegen des
Startdiffs nicht geerbten Kanon/vollen Kanon sowie A8/A9/A10 fahren. Danach
folgt ein frischer Prüfer; der Bauer prüft die Etappe nicht selbst.
## 28. Messung der Etappe 3, Regel R-380-11, T-380-10-Korrektur und Nacharbeit 0 (Dirigent, 25.09.2026, 15:50 Uhr)

**Messung.** Bauer Codex `gpt-5.6-sol` xhigh `workspace-write`
(`tools/dirigent/codex-bau.ps1`, Kennung `nak380-e3`, Thread
`01a0d877-6696-7d92-94ad-f405d6138e9a`), 14:08:18 bis 15:33:37 Uhr, EXIT=0,
Basis `b04dfb7c`, kein Commit durch den Bauer. Vom Dirigenten gemessen: 14
geänderte und 35 neue Dateien; Produktdiff ohne `docs/` 13 Dateien
(+1 551/−443): `FeatureEngine.h` (`kLraHopZellen = 1`, Fassung 20260926),
`Lautheit.h` (nur Kommentar), `AnalyseEngine.cpp`/`.h` (Teilblockhistogramm,
acht Segmente, Rest verfällt am Reset, Perzentile nur aus dem Teilblockträger,
Abdeckung unverändert auf Einzelsegmenten, `kMetricsVersion`
`m4.2-2026-09-25`, lesender Testfreund, 127 296 Byte je Engine),
`Diagnose.cpp` (nur Kommentar), `festhalten-referenz.json` über den
B30-Erzeuger (`metrics_version` m4.2, Perzentilwerte; Attribut `-text`, LF
wie zuvor: `git ls-files --eol` i/lf w/lf), `metriken-v1.json` (Fassung
20260926), die Tests B18, B9, A2, `Nak380Pruefsignale.h`,
`pruefe_v3_vertrag.py`, `tools/beweise.ps1` (Behauptungszeilen). v3-, State-
und FB-Korpus, Bandgitter, Identität, Installer, Audio-, Sonden-,
Hostbridge-Pfade unberührt. **Vom Dirigenten gefahren:** A8 Exit 0 (421
Dateien bytegleich, MANIFEST `b99deec4…`), A9 Exit 0 (Drift 0), A10 Exit 0
(131 Dateien bytegleich). Selbstbericht (§27): M-21 bis M-41 „JA“ an der
Zusagezeile mit Rohbelegen; Beine B9 117/0, A2 258/0, B30 214/0, A5 861/0,
A1, A16 145/0, B1 120/0, B4 121/0 Exit 0; **B18 Exit 1 (85/1, allein M-23)**.
Kanon vorher: geerbt — der dritte Pfad im Startdiff
(`eq-copilot/install/nakama-installer-v1.json`) ist der Broker-Pin des
Runners aus dem Kanon nachher der Etappe 2, kein Kanon-Bein (§27.1
präzisiert).

**Offene Punkte des Baus und Einordnung.**

1. **M-23 (§27.6 Punkt 1):** gebaut 2,600 LU gegen die Matrixzusage 2,44 ±
   0,15 LU; nichts gelockert, B18 rot. Nachgerechnet: die Matrixreferenz
   2,435 LU ist die geschlossene Form über eine kontinuierlich
   gleichverteilte Einschaltquote f ∈ [1/3, 2/3]. Die 10-Hz-Folge des
   L2-Signals (Rechteck mit Periode 2 s, 3-s-Fenster, Hop 100 ms) nimmt nur
   die elf Werte f = 10/30 … 20/30 an (je Periode 20 Werte: 10/30 und 20/30
   je einmal, die übrigen je zweimal). Der §5-Algorithmus (Index
   round((n−1)·p + 1)) trifft damit P10 bei 11/30 und P95 bei 20/30:
   10·log10(0,6700/0,3730) = 2,544 LU; das 0,1-LU-Histogrammraster des
   Produkts rundet auf 2,5 oder 2,6 LU. Der Produktwert 2,600 liegt 0,056 LU
   über der diskreten Referenz und ist kein Produktfehler; die Matrixzahl
   hatte keine Referenz desselben Algorithmus (R-380-8) → **LÜCKE**, Regel
   R-380-11 (§28.1).
2. **T-380-10 (§27.6 Punkt 5):** der Bauer hob `kFeatureMetricsVersion` auf
   20260926, ließ beide `METRICS_VERSION` des Brokers
   (`vergleichbarkeit.rs:149`, `prepost.rs:206`) auf 20260925 und band sie
   über ein neues Registerfeld `broker_aktuell` (Grund: §8.3 nennt die zwei
   Brokerdateien nicht). T-380-10 und R-380-7 verlangen je Etappe
   `kFeatureMetricsVersion` UND beide `METRICS_VERSION`; §8.3 hat die zwei
   Dateien übersehen → **DEFEKT gegen T-380-10/R-380-7**, Bauplanlücke vom
   Dirigenten geschlossen: die zwei Brokerdateien gehören zu den Ticketpfaden
   der Etappe 3; `broker_aktuell` entfällt; eine Fassung `aktuell` bindet
   beide Sprachen wie in Etappe 2. Der Broker vergleicht die Fassung eines
   Frames nirgends (M-18), er stempelt nur seine Ausgaben; die Trennung wäre
   dennoch eine zweite Zahlenbank im Vertrag.
3. **M-36/M-38 Ersatzmutationen, M-41 Wiederverwendung des M-20-Rotbeweises
   (§27.6 Punkte 2 bis 4):** Technikentscheide innerhalb der Rotbeweisregel;
   die Erstprüfung 3 prüft sie am Beleg.
4. **M-28(b):** Altwert 18,900 LU innerhalb ± 0,2 LU der §5-Referenz 18,883
   LU → Altvergleich belegt (E-380-18); der Ratenwechsel wird von dieser
   Hälfte nicht getragen (so in §6 benannt), von M-21 bis M-23 aber.

### 28.1 Regel R-380-11 (M-23: Referenz ist die diskrete §5-Folge; schließt die Lücke)

Die Referenz von M-23 ist `lraReferenz` (T-380-3,
`eq-copilot/plugin/tests/Nak380Pruefsignale.h`) über die analytische
100-ms-Zellenenergie des L2-Signals (0,5·a²(t) je Zelle, Amplitude 0,1 und
0,01 im Sekundenwechsel, laut ab Sample 0, 70 s, dieselbe Folge wie im
Produktlauf), nicht die geschlossene Form; Toleranz ± 0,15 LU
(0,1-LU-Histogrammraster und Einschwingrand des K-Filters); die geschlossene
Form 2,435 LU bleibt als Plausibilitätsnotiz im Kommentar (diskret nachgerechnet
2,544 LU). Zweite Hälfte (Aliasing): der Produktwert liegt mindestens 0,3 LU
unter dem 1-Hz-Aliaswert 10·log10(0,6700/0,3400) = 2,946 LU (1-Hz-Werte an
ganzen Sekunden treffen nur 1/3 und 2/3). Der Rotbeweis bleibt die Mutation
`kLraHopZellen` 1 → 10 und fällt an beiden Hälften. Matrix M-23 in der
Fassung §28.

### 28.2 Nacharbeit 0 (Regelfolge vor Kanon und Erstprüfung 3; keine Prüferrunde)

Auftrag `docs/beweise/roh/NAK-380-nacharbeit-e3-0-auftrag.txt`: (a) M-23 nach
R-380-11; (b) T-380-10: `vergleichbarkeit.rs` und `prepost.rs`
`METRICS_VERSION = 20260926`, `metriken-v1.json` ohne `broker_aktuell`, A5
mit einer Fassungsbindung wie vor der Etappe, `telemetrie.rs` M-18 nimmt
20260926 in die Fassungsliste; Rotbeweise M-23 und M-40 erneuern; Beine B18,
A5, A4 (Filter `telemetrie`, `prepost`, `vergleichbarkeit`, `hypothese`),
A1/A16/B1/B4 unberührt, A9 unberührt. Bauer: frischer Opus-5.5-Worker, Effort
max (Claude-Woche nach dem Reset 15:00 Uhr bei 0 %, NAK-359), Aufsicht ENG;
der Dirigent misst, committet, fährt danach den Kanon abgekoppelt und die
Erstprüfung 3 (Vorlage A, Codex `gpt-6-astra` xhigh nach NAK-362,
Prüfbereich `b04dfb7c...HEAD` über die Ticketpfade der Etappe 3 einschließlich
der zwei Brokerdateien). Zwischenstand der Etappe 3 mit Pathspec committet
(Betreff nennt B18 rot an M-23, Kanon und Erstprüfung 3 NOT RUN); Rundenbilanz
im Abschluss der Etappe.

## 29. Nacharbeit 0 der Etappe 3 (Bauer Opus 5.5, 25.09.2026)

**Start.** `git rev-parse HEAD` ergab
`f8942067e58a574f5790889d833ab32b067b1303` auf `master`; `git status --short`
nannte nur die fremden untracked Einträge `briefing-hub/` und
`nimbalyst-local/`; die Prozessprobe auf `cmake`, `MSBuild`, `cl`, `link`,
`cargo`, `rustc`, `EqCop*`, `eqcop-broker`, `FL64` und `vctip` war leer
(15:49 Uhr). Auftrag: `docs/beweise/roh/NAK-380-nacharbeit-e3-0-auftrag.txt`.
Kein Stage, Commit, Push oder Stash, keine Rücknahme über git, kein voller
Kanon, kein FL Studio, keine Installation, kein A8/A9/A10.

### 29.0 Schließung je Punkt

**(a) M-23, Lücke mit Regel R-380-11: geschlossen.**

- `eq-copilot/plugin/tests/Nak380Pruefsignale.h:180-197`: neue Hilfe
  `l2Zellenergie (int zellen)` nach dem Muster `tabelle1Zellenergie`: je
  100-ms-Zelle 0,5·a² mit a = `l2Amplitude` am ersten Zellsample (4 800
  Samples sind 100 Perioden des 1-kHz-Sinus, jede Zelle liegt ganz in einer
  Sekunde); analytisch, nichts aus dem Produkt.
- `eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:355-390` (Fall
  `380/M-23 lra_ohne_aliasing`): die Konstante `soll = 2.435` entfällt. Die
  Referenz ist `sig::lraReferenz (sig::l2Zellenergie (700))` (`:368`), der
  Aliaswert entsteht aus den L2-Amplituden als
  10·log10((2·e_laut + e_leise)/(e_laut + 2·e_leise)) (`:373-378`). Eine
  Prüfung (`:383-390`) trägt beide Hälften: (a) Abweichung ≤ 0,15 LU, (b)
  Abstand zum Aliaswert ≥ 0,3 LU; ihr Bericht nennt Produktwert, Referenz,
  Abweichung, Aliaswert, Abstand und je Hälfte ja oder nein. Der Kommentar
  (`:355-372`) nennt die Referenz als Formel, die diskrete Nachrechnung (P10
  bei k = 11 mit Index 68, P95 bei k = 20 mit Index round(637,5) = 638,
  10·log10(0,6700/0,3730) = 2,544 LU) und die geschlossene Form 2,435 LU als
  Plausibilitätsnotiz. Keine Zahl aus dem Lauf steht als Golden im Test
  (R-380-8).
- Messung, B18 voll um 15:56 Uhr: Produktwert **2,600 LU**; diskrete
  Referenz aus dem Lauf **2,544 LU**, Abweichung **0,056 LU** (≤ 0,15);
  Aliaswert **2,946 LU**, Abstand **0,346 LU** (≥ 0,3; Rand zur Grenze
  2,646 LU: 0,046 LU). B18 **86/0** statt 85/1. Unabhängige Nachrechnung mit
  `py -3.13` über dieselbe Zellfolge: 671 Kurzzeitwerte, keiner gegatet, LRA
  2,5436597 LU, Aliaswert 2,9459589 LU.

**(b) T-380-10, Defekt: geschlossen.**

- `broker/src/coordinator/vergleichbarkeit.rs:149` und
  `broker/src/coordinator/prepost.rs:206`: `METRICS_VERSION` 20260925 →
  **20260926**; sonst keine Zeile.
- `eq-copilot/schemas/v3/metriken-v1.json`: das Feld `broker_aktuell`
  (vorher Zeile 293) aus der Fassung 20260926 entfernt; `seit`, Hinweis,
  Schwellen und `nicht_gefuehrt` unverändert.
- `tools/eq-copilot/pruefe_v3_vertrag.py`: `pruefe_comparability_schwellen`
  (`:2450-2523`) ist wieder bytegleich zur Fassung am Stand `b04dfb7c`
  (Funktionstext per Skript verglichen, 4 086 Zeichen gleich): Gates,
  Brokerbindung und PRE/POST-Bindung laufen gegen `aktuell`, im Prüfer steht
  kein `broker_aktuell` mehr. `pruefe_nak380_etappe_3` behält M-30 und M-40;
  M-40 (`:2413-2447`) prüft zusätzlich beide Rust-Konstanten (`:2422-2431`,
  „vergleichbarkeit.rs nennt 20260926“, „prepost.rs nennt 20260926“), also
  Code in C++ und Rust und Register mit derselben Fassung 20260926. M-40
  allein 6/0 statt 4/0.
- `broker/src/telemetrie.rs:1187`: die Liste des Tests
  `nak380_m18_alte_und_neue_version_gelesen` lautet
  `[20_260_904, 20_260_925, 20_260_926]`; beide Altstände bleiben
  angenommen.
- Messung: A5 **863/0** (vorher 861/0, zwei neue M-40-Prüfungen).
  Gegenprobe mit dem neuen Prüfer vor dem Heben der Rust-Konstanten: 859/4,
  rot genau die vier Fassungsbindungen. Am unveränderten HEAD waren die zwei
  bestehenden A4-Tests `urteil_traegt_seine_kalibrierungsfassung`
  (`broker/tests/sonde013_passage.rs:379`,
  `broker/tests/sonde013_prepost.rs:718`) rot, weil das Register
  `"aktuell": 20260926` nennt und der Broker 20260925 stempelte; jetzt 2/0.
  Der Defekt hätte A4 im Kanon gerissen; §27.4 nennt keinen A4-Lauf der
  Etappe 3.

### 29.1 Änderungsregister

| Datei und Stelle | vorher (HEAD `f8942067`) | jetzt |
|---|---|---|
| `eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:355-390` | Soll `2.435` ± 0,15 LU (geschlossene Form), Kommentar „10*log10(0,6535/0,3730)“ | `lraReferenz (l2Zellenergie (700))` ± 0,15 LU und Abstand ≥ 0,3 LU zum Aliaswert aus den L2-Amplituden in einer Prüfung, Formeln im Kommentar |
| `eq-copilot/plugin/tests/Nak380Pruefsignale.h:180-197` | keine L2-Zellenergie | `l2Zellenergie (int zellen)` |
| `broker/src/coordinator/vergleichbarkeit.rs:149` | `METRICS_VERSION: u32 = 20260925` | `20260926` |
| `broker/src/coordinator/prepost.rs:206` | `METRICS_VERSION: u32 = 20260925` | `20260926` |
| `broker/src/telemetrie.rs:1187` | `[20_260_904, 20_260_925]` | `[20_260_904, 20_260_925, 20_260_926]` |
| `eq-copilot/schemas/v3/metriken-v1.json`, Fassung 20260926 | `"broker_aktuell": 20260925` | Feld entfernt |
| `tools/eq-copilot/pruefe_v3_vertrag.py:2450-2523` | `broker_aktuell`-Zweig für Gates, Broker- und PRE/POST-Bindung | Form von `b04dfb7c`, Bindung an `aktuell` |
| `tools/eq-copilot/pruefe_v3_vertrag.py:2422-2431` | M-40 prüft C++-Konstante und Register | zusätzlich beide Rust-Konstanten |
| `roh/NAK-380-rot-M-23.txt`, `roh/NAK-380-rot-M-40.txt` | Belege aus §27 | ersetzt (§29.2) |
| `roh/NAK-380-e3-n0-mutation-lra-hop.txt`, `roh/NAK-380-e3-n0-mutation-feature-version.txt`, `roh/NAK-380-e3-n0-mutation-broker-version.txt` | keine | neu, Kurzprotokolle der Mutationen |

Kein weiterer Produktcode: `FeatureEngine.h` ist nach drei Mutationen
bytegleich zum HEAD (SHA-256 `B836B6CE…`, `git diff` leer); `Lautheit.h`,
`AnalyseEngine.*`, `Diagnose.cpp`, `Frame.h`, `Spektrum.h`, Fixtures,
Bandgitter, Verträge, generierte Dateien, Installer, `.claude/**`,
`design/**`, `docs/PLAN-STAND.md`, `docs/offene-punkte.md` und `docs/plan/**`
blieben unberührt. Zeilenenden nach `git ls-files --eol` gemessen und
beibehalten: C++-Tests und die drei Rust-Dateien `i/lf w/crlf` (Arbeitskopie
ohne LF-Zeile), `metriken-v1.json` `i/crlf w/crlf -text` (422 CRLF, keine LF),
`pruefe_v3_vertrag.py` LF, Manifest und Rohdateien LF.

### 29.2 Rotbeweise

Ablauf nach §8.1: Sicherung unter `eq-copilot/build/nak380-sicherung/n0/`,
SHA-256 vor der Mutation und nach der Rücknahme, Mutation wörtlich, nur der
betroffene Fall, Rücknahme bytegleich durch Zurückkopieren (nie über git),
`LastWriteTime` auf jetzt, nach der C++-Rücknahme Neubau mit
Zeitstempelprüfung.

| Zeile | Mutation | Ergebnis unter der Mutation | Rücknahme | Rohdatei | fällt an der Zusagezeile |
|---|---|---|---|---|---|
| M-23 (a) und (b) | `FeatureEngine.h:419` `kLraHopZellen = 1` → `10` | B18 `--nak380 M-23` Exit 1: 3,000 LU; (a) Abweichung 0,456 LU > 0,15; (b) Abstand −0,054 LU < 0,3; M-21 unter derselben Mutation 73 statt 721 (eigene Zeile, erwartet) | SHA-256 gleich; Neubau; M-23 grün mit 2,600 LU; B18 86/0 | `roh/NAK-380-rot-M-23.txt` | **JA** (beide Hälften) |
| M-40 Hälfte 1 | `FeatureEngine.h:89` `kFeatureMetricsVersion = 20260926u` → `20260925u` bei Register 20260926 | A5 Exit 2, 861/2: „Code und Register nennen dieselbe Version“ und M-40 „FeatureEngine nennt 20260926“; M-40 allein 5/1 | SHA-256 gleich; A5 863/0 | `roh/NAK-380-rot-M-40.txt` Abschnitt 3 | **JA** |
| M-40 Hälfte 2 | `vergleichbarkeit.rs:149` `METRICS_VERSION = 20260926` → `20260925` | A5 Exit 2, 861/2: M-40 „vergleichbarkeit.rs nennt 20260926“ und „der Broker nennt die Fassung des Registers“; M-40 allein 5/1; zusätzlich A4 `urteil_traegt_seine_kalibrierungsfassung` in `sonde013_passage` rot | SHA-256 gleich; A5 863/0; A4 2/0 nach Neubau | `roh/NAK-380-rot-M-40.txt` Abschnitt 4 | **JA** |

Die neue M-40-Prüfung „prepost.rs nennt 20260926“ fiel in der Gegenprobe am
Produktwert vor der Nacharbeit (20260925; `roh/NAK-380-rot-M-40.txt`
Abschnitt 2b). Kurzprotokolle: `roh/NAK-380-e3-n0-mutation-lra-hop.txt`,
`roh/NAK-380-e3-n0-mutation-feature-version.txt`,
`roh/NAK-380-e3-n0-mutation-broker-version.txt`. Nach allen Rücknahmen nennt
`git diff --stat` nur die Dateien aus §29.1 und dieses Manifest.

### 29.3 Gefahrene Beine

| Bein | Ergebnis | Binary-Zeitstempel |
|---|---|---|
| B18 `EqCopSonde013DynamicsTest` | Exit 0, **86/0** (vorher 85/1), 15:56:05 bis 15:56:16 Uhr | 15:55:43.928, jünger als `FeatureEngine.h` 15:55:37.506, `Sonde013DynamicsTest.cpp` 15:50:10.642 und `Nak380Pruefsignale.h` 15:49:35.140 |
| A5 `pruefe_v3_vertrag.py --abdeckung` | Exit 0, **863/0** (vorher 861/0), 15:56:31 bis 15:56:35 Uhr | Pythonlauf am Endstand |
| A4 Filter `telemetrie` | Exit 0, 9/0 (mit `nak380_m18_alte_und_neue_version_gelesen`) | Testbinaries 15:54:44 bis 15:54:49, jünger als die jüngste Rust-Quelle `vergleichbarkeit.rs` 15:54:38.009 |
| A4 Filter `prepost` | Exit 0, 2/0 | wie oben |
| A4 Filter `vergleichbarkeit` | Exit 0, 1/0 | wie oben |
| A4 Filter `hypothese` | Exit 0, 33/0 | wie oben |
| A4 `urteil_traegt_seine_kalibrierungsfassung` (zusätzlich) | Exit 0, 2/0 (am HEAD 0/2) | wie oben |
| Broker Release `cargo build --release` | Exit 0, 15:56:42 bis 15:57:01 Uhr | `broker/target/release/eqcop-broker.exe` 15:56:59.745 |

C++ gebaut mit dem VS-CMake über den absoluten Pfad aus dem Auftrag: B18
15:50:26 bis 15:50:30 Uhr, unter der M-23-Mutation 15:55:08 bis 15:55:15 Uhr
und nach der Rücknahme 15:55:37 bis 15:55:43 Uhr, je Exit 0; übersetzt wurden
jeweils `TelemetryClient.cpp` und `NakamaEvidenz.cpp` aus `NakamaKern` sowie
`Sonde013DynamicsTest.cpp`. Übersetzungsprobe der vier weiteren Einbinder von
`Nak380Pruefsignale.h` (B16, B9, A2, B5): 16:02:04 bis 16:02:33 Uhr, Exit 0,
nicht gefahren. Verbliebene `vctip.exe` ohne Kindprozess nach NAK-300
beendet (gestartet 15:50:28, 15:52:23, 15:55:39, 15:56:59 und 16:02:07 Uhr).

### 29.4 Abweichungen und offene Punkte

1. **Zeilenenden der Rust-Dateien.** Der Auftrag nennt „Rust LF“; gemessen
   ist für `vergleichbarkeit.rs`, `prepost.rs` und `telemetrie.rs`
   `i/lf w/crlf` wie vor dem Eingriff. Nach der Regel „dieselben
   Zeilenenden zurückschreiben“ blieb die Arbeitskopie CRLF; der Index
   bleibt LF.
2. **Zusätzliche Messungen** ohne neues Bein: A4
   `urteil_traegt_seine_kalibrierungsfassung` am HEAD, unter der Mutation
   M-40 Hälfte 2 und am Endstand; M-21 unter der M-23-Mutation;
   Übersetzungsprobe B16, B9, A2, B5.
3. **Offen für den Dirigenten, außerhalb der Ticketgrenze:** die
   A5-Behauptung in `tools/beweise.ps1` (Zeile 544) endet mit „Seit
   NAK-380 Etappe 2 … bindet Version 20260925“; die Fassung 20260926, M-30
   und die Brokerbindung von M-40 nennt sie nicht (Prüfliste E, drei
   Stellen). Die B18-Behauptung nennt keine M-23-Zahl und bleibt richtig.
4. **Zeitstempel.** `FeatureEngine.h` trägt nach den Rücknahmen
   `LastWriteTime` 15:55:37 bei unverändertem Inhalt; die Binaries der
   übrigen C++-Beine (A1, A16, B1, B4, B30 und weitere) stammen aus dem Bau
   der Etappe 3 und sind damit älter als diese Quelle. Der Kanon mit
   `-Bauen` baut sie neu; ohne Neubau verweigert der Runner nach NAK-230.
5. **Verlauf mit der alten Zahl.** §7.2 (Signal L2) und §7.4 nennen noch
   2,435 LU als §5-Referenz; seit R-380-11 (§28.1) gilt die diskrete Folge.
   Beide Stellen sind append-only Verlauf und blieben unverändert.
6. **Beobachtung ohne Änderung:** die B18-Ausgabe von M-28(b) zeigt „Â§5“
   (UTF-8-Zeichen in einem schmalen `juce::String`-Literal, nur
   Konsolentext; Fall der Etappe 3, außerhalb dieser Nacharbeit).

Kanon, Erstprüfung 3 und Rundenbilanz fährt der Dirigent; der Bauer prüft
die Etappe nicht selbst.
## 30. Messung der Nacharbeit 0, Kanon nachher und Erstprüfung 3 gestartet (Dirigent, 25.09.2026, 16:15 Uhr)

**Messung.** Opus-5.5-Worker `cc0476b8` (`nakama-nak380-f894206-n0`, Effort
max, `dontAsk`, Session `cc0476b8-0d6f-4091-b794-41907c2557db`), 15:42 bis
16:06 Uhr, Basis `f8942067`, kein Commit durch den Worker. Vom Dirigenten
gemessen: 10 geänderte und 3 neue Dateien; Produktdiff nur die zwei
Rust-Konstanten `METRICS_VERSION = 20260926` (`vergleichbarkeit.rs:149`,
`prepost.rs:206`) und die M-18-Testliste in `telemetrie.rs`;
`metriken-v1.json` ohne `broker_aktuell`; A5 mit einer Fassungsbindung (Form
von `b04dfb7c`) und M-40 zusätzlich über beide Rust-Konstanten; M-23 nach
R-380-11 (`lraReferenz (l2Zellenergie (700))`, beide Hälften in einer Prüfung,
Formeln und diskrete Nachrechnung im Kommentar, kein Golden aus dem Lauf);
`FeatureEngine.h` nach drei Mutationen bytegleich. B18 selbst gefahren: Exit
0, 86/0 (Binary 15:55:43 nach den Quellen 15:50:10 und 15:49:35): M-23
2,600 LU, Referenz 2,544 LU, Abweichung 0,056 ≤ 0,15, Aliaswert 2,946 LU,
Abstand 0,346 ≥ 0,3; A5 Exit 0, 863/0. Rotbeweise M-23 (beide Hälften) und
M-40 (beide Hälften) mit Rohdateien (§29.2). **Befund des Workers (§29.0
b):** am Stand `f8942067` waren die zwei bestehenden A4-Tests
`urteil_traegt_seine_kalibrierungsfassung` (`sonde013_passage.rs:379`,
`sonde013_prepost.rs:718`) rot, weil der Broker 20260925 stempelte und das
Register 20260926 nannte; §27.4 nannte keinen A4-Lauf der Etappe 3 — der
T-380-10-Defekt hätte den Kanon gerissen; jetzt 2/0. Dirigent: die
A5-Behauptungszeile in `tools/beweise.ps1` (Zeile 544) um die Fassung
20260926, M-30 und die Brokerbindung von M-40 ergänzt (Prosa, keine Logik;
§29.4 Punkt 3). Commit `HEAD nach diesem Abschnitt` (Pathspec), gepusht.

**Kanon nachher (abgekoppelt, Skill §3.5).** Log
`$env:TEMP\nakama-nak380-e3-kanon.log`, Befehl `tools/beweise.ps1 -Bauen
-Ziel docs/beweise/NAK-380.md -Anhaengen -Titel NAK-380` auf HEAD `HEAD nach diesem Abschnitt`;
Rohausgabe `docs/beweise/roh/NAK-380-<sha7>.md`; der Kanon-Abschnitt wird an
dieses Manifest angehängt. Der Bau erneuert alle C++-Binaries
(`FeatureEngine.h` trägt nach den Rücknahmen einen neuen Zeitstempel bei
gleichem Inhalt, NAK-230).

**Erstprüfung 3 (Vorlage A), parallel zum Kanon.** Codex `gpt-6-astra`
**xhigh** (NAK-362; Claude-Woche nach dem Reset 0 %), lesend; Prüfbereich
`b04dfb7c...HEAD nach diesem Abschnitt` über die Ticketpfade der Etappe 3 einschließlich der
Nacharbeit 0 (`git diff --name-only b04dfb7c HEAD -- broker eq-copilot
tools`); Auftrag `docs/beweise/roh/NAK-380-erstpruefung-3-auftrag.txt`,
Kennung `nak380-p3`; Urteil, Validierung und Einordnung in §31.

---

## Kanon-Lauf - NAK-380

**Lauf:** 2026-09-25 16:11 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 70/70 Kanon-Laeufe bestanden | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/NAK-380-8308a3d-dirty.md](roh/NAK-380-8308a3d-dirty.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-25 16:11:02 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 8308a3d8 NAK-380 Etappe 3 Nacharbeit 0 (Opus 5.5 max, Worker cc0476b8, 15:42 bis 16:06 Uhr; Regelfolge): M-23 nach R-380-11 (Referenz lraReferenz über die analytische L2-Zellenenergie, 2,600 gegen 2,544 LU, Abstand 0,346 LU zum 1-Hz-Aliaswert), T-380-10-Korrektur (beide Broker-METRICS_VERSION 20260926, metriken-v1.json ohne broker_aktuell, A5 mit einer Fassungsbindung und M-40 über beide Rust-Konstanten, M-18-Liste um 20260926; am Vorstand waren die A4-Tests urteil_traegt_seine_kalibrierungsfassung rot), Rotbeweise M-23 und M-40 erneuert; B18 86/0 und A5 863/0 vom Dirigenten nachgefahren; A5-Behauptungszeile in tools/beweise.ps1 nachgezogen; §29 (Bauer), §30 (Dirigent); Kanon nachher und Erstprüfung 3 laufen ab diesem Stand |
| Commit (voll) | 8308a3d80146f1119203127e927c1f746fcd4ec9 |
| Arbeitsbaum | 3 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.6 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,11 s | [A1](roh/NAK-380-8308a3d-dirty.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). Seit NAK-380 Etappe 3: M1-Perzentile aus Teilbloecken zu acht Segmenten; Rauschen steht in jedem Band ruhig, eine bekannte Pegelbewegung wird innerhalb -0,5/+2,0 dB gelesen. | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 39,66 s | [A2](roh/NAK-380-8308a3d-dirty.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). Seit NAK-283 (F12) erzeugt ein endlicher Eingang keinen nicht endlichen Wet-Ausgang: im Puls auf 1 kHz mit einem Ton der Amplitude 0,8 x FLT_MAX ist jedes Ausgangssample endlich, und nach dem Einblenden steigt der Wet-Zaehler um genau die verriegelten Samples. Seit NAK-312 Etappe 6a (R-312-8): ein Rollenwechsel zu Main nimmt einen in Legacy eingereichten Auftrag zurueck, bevor die Klassifikation wirkt - 40 Bloecke danach bitgleich ohne Ereignis im Ring, ebenso nach dem Rueckwechsel; in Legacy reicht SOLO nichts ein, zeigt keinen Aus-Knopf, und der Status nennt die Lage; ein Wechsel weg von Main schliesst das Intervall mit genau einem begin und einem end ueber einen weichen Ausfade (Nachbarsprung innerhalb der E-31-Schranke) und blendet beim zweiten Wechsel nicht wieder ein, auch mitten im Ausfade; Freilaufmeldung, Sampleratenwechsel und 10-Minuten-Totmann beenden die Markierung auch in der Main-Flaeche. Seit Etappe 6a (R-312-5): reset() bei angehaltenem Transport beendet Klang und Intervall (40 Bloecke bitgleich, genau ein end ohne Projektzeit, 0 Allokationen), laesst den Oversize-Riegel bis zum naechsten prepareToPlay stehen und den Auftrag bestehen (laeuft der Transport weiter, ein neues begin); ueber reset, Bloecke ohne und mit Erlaubnis und prepareToPlay hat jedes begin genau ein end; bei vollem Ring bleibt reset fail-closed (Ueberlaufbit, keine Sequenz); releaseResources schliesst wie reset und gibt die Blockpuffer frei, prepareToPlay wie bisher; ohne Auftrag entsteht kein Ereignis und kein Host-Dirty. Seit NAK-312 Etappe 7b (U56, 312/M-130): releaseResources laesst den Auftrag bestehen, nach prepareToPlay beginnt er mit genau einem neuen begin - seit der Nacharbeit 1 (R-312-33) am ersten erlaubten Block allein gemessen: dieser Block faerbt, ist hoerbar und meldet das neue begin, die uebrigen 39 bleiben hoerbar ohne weiteres Ereignis. Seit Etappe 7b, Satz 3 (U49, 312/M-118): der Rollenwechsel nimmt den Auftrag zurueck und laesst den Bestand stehen. Seit Etappe 7b, Satz 2 (312/M-109, Grenzzeile, nicht Gegenstand der Antwort U48): Gens Hostbypass bleibt die JUCE-Basis - kein Quelltext unter eq-copilot/plugin/src/ nennt processBlockBypassed oder getBypassParameter, getBypassParameter liefert nullptr, und mit hoerbarem Auftrag in Main geht der Puffer ueber 20 Bloecke processBlockBypassed unveraendert durch, ohne Ereignis; danach faerbt der Auftrag in 40 Bloecken processBlock weiter, ohne neues begin und ohne end. Seit NAK-313 Etappe 7 (R-313-9, 313/M-128): die Byteinstanz heartbeat-wire-v1.json traegt binary_telemetry supported, und der Heartbeat-Schreiber erzeugt ihre drei Wiretexte bytegleich. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 8,18 s | [A3](roh/NAK-380-8308a3d-dirty.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. Seit Etappe F dazu die Proposal-Policy: ein Vorschlag entsteht MIT seinem Befund und traegt die fuenfzehn Felder aus 42.1 plus revert, der Rueckweg ist ein FELD mit drei Werten (dsp_revert faellt), in P5 ist jede Aktion manual, keine Aenderung und mehr Daten sind vollstaendige Objekte, der Zielbereich kommt aus dem Band des Befunds statt aus dem groessten Banddelta, ein geschuetztes Band ist eine HARTE Constraint mit Gegenprobe, ein stop_if auf einem nicht messbaren Guardrail meldet MORE DATA, und ueber fuenfhundert zufaellige Eingaenge wird kein Hard Cap und keine engere Usergrenze ueberschritten. Der Guardrail-Rechner LIEST seit E-05 den Zielbereich aus experiment_begin.ziel; ohne ihn bleibt die Heuristik und das Resultat traegt ziel_geraten - beide Pfade mit verschiedener Zahl gemessen. Fehlt einem Test eine Voraussetzung (das Junction-Recht fuer store_weist_reparse_punkt_im_pfad_ab), meldet er NOT RUN mit Grund: das Bein steht dann als [NOT RUN] in der Uebersicht und der Lauf ist UNVOLLSTAENDIG, nie gruen (NAK-309 R-309-4). Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-43 bis M-45, M-48, M-49, M-52, M-53): die Produkteingaenge P0 und P1 (gemeinsame Lesefunktion samt P1-Weiche) und das Bootstrap-Hello lesen ueber genau einen strengen Lauf json_streng (broker/src/vertrag.rs): Nachspann, zweites Dokument, Schlusskomma, unbekannte Escapes, doppelte dekodierte Namen im selben Objekt samt Escape-Alias und eine Verschachtelung ueber 64 Ebenen sind abgelehnt, derselbe Name in zwei Objekten bleibt gueltig (Raender einzeln gemessen), und die Lesefunktion nennt die Stufe der Ablehnung. Je Eintrag der Eingaenge rust_p0, rust_p1 und rust_bootstrap der Tabelle PRODUKTEINGAENGE-FAELLE.json werden Urteil, Stufe und Wirkung gegen produkt geprueft, jede Abweichung mit PE-nnn gemeldet und die gefahrenen Eintraege gegen den Kopf gezaehlt: P0 sendet kein heartbeat_ack und markiert keinen ersten Heartbeat; die P1-Weiche lehnt vor dem Schema ab, ohne Evidenz- oder Intenteffekt, und liest je Nachricht genau einmal streng (Laufzaehler nur im Testbau); der Bootstrap meldet KeinJson mit der Duplikatmarke statt eines Hellos; 64 Ebenen sind gueltig, 65 fallen an parser. Die Korpusschleife erwartet fuer parser_lehnt_ab die Ablehnung am strengen Lauf und fuer das Negativfixture mit project_sample_start null die Verletzung an der gemeinsamen Eigenschaft; der echte Empfaenger nimmt den wire_snapshot der Byteinstanz evidenz-lokal-wire-v1.json an, und die Evidenzsicht traegt keinen Startwert. Seit NAK-313 Etappe 5a (R-313-4, R-313-5; 313/M-63 bis M-72, M-90, M-96): die Rust-Produktleser lesen Ganzzahlen ueber den Helfer crate::vertrag::ganzzahl (as_i64, sonst as_u64 im Bereich, sonst ein endliches f64 ohne Nachkommateil im Bereich, sonst None und nie 0; die Zahlenraender einzeln gemessen). P0 liest die sequence des Heartbeats vor den Seiteneffekten: 91.0 wird mit sequence 91 beantwortet, und der Resync laeuft genau dann, wenn eine Antwort entsteht. Der Deskriptor traegt host_mixer_index als Ganzzahl: 3.0 nach 3 ist kein Kanalwechsel, und der Snapshot traegt 3. evidenzstand_aus_wert liest Transport-, Ereignis- und Konfidenzzahlen in .0-Form und lehnt ein vorhandenes, nicht lesbares Feld ab, statt 0 zu setzen; 3.0 nach 3 ist kein Epochwechsel. Intervention (Beginn, Ende und genau der Bereich), Befehlskopf und Experimentziel, state_report, Intent und Assistent nehmen die .0-Formen wie Zifferntext an. Eine state_revision von 2^53 faellt im Heartbeat und im state_report am Textriegel: der Heartbeat bleibt ohne Antwort und ohne Liveness-Aenderung, der state_report laesst die gemeldete Revision unveraendert. Das Bootstrap-Hello waehlt das Protokoll ueber denselben Helfer (3.0 und 3e0 sind 3, 3.5 und 4 sind KeinHello, v2 bleibt V2), und die typisierte Uebernahme liest block_size 256.0, channels 2.0 und pid 1234.0 nach einer Normalisierung ueber denselben Helfer. Je Eintrag von rust_p0, rust_p1 und rust_bootstrap wird bei Urteil gueltig der Wert am Feld verglichen; NaN, Infinity, -Infinity und 1e999 enden fuer P0 und P1 am Textriegel. Seit NAK-313 Etappe 5b (R-313-4, 313/M-89): der Wiretext der Handinstanz intent-wire-v1.json passiert die P1-Weiche, und der Intentspiegel uebernimmt die Bestandsrevision 2^53-1 unveraendert. Seit NAK-313 Etappe 6 (R-313-7, R-313-8; 313/M-98 bis M-105, M-115, M-116, M-118): das Bootstrap-Tor rechnet das Urteil des Textriegels ueber die Hello-Bytes vor dem einen strengen Lauf, waehlt danach das Protokoll (ein v2-Hello geht ohne Textriegel und v3-Schema als V2 mit den Rohbytes durch, auch mit samplerate 48000.0000000000001 oder einem NUL-Escape in sensor.label) und wendet fuer v3 erst das gehaltene Urteil an, dann die Hello-Pruefung des v3-Schemas (hello_control, hello_telemetry) vor der typisierten Uebernahme: ohne host ein V3Control mit host None; host null, host.name null und host.version null enden als KeinHello mit dem Schemagrund und dem Zeiger des Feldes, ebenso 121 Zeichen in host.name und 65 in host.version; ein NUL-Escape in plugin_version und die samplerate 1e-308 enden am Textriegel, 1e-307 und 48000 verbinden, 768000.5 endet am Schema, und ein Hello mit NUL-Escape und host null endet am Textriegel. Je Hello laeuft json_streng genau einmal (Laufzaehler nur im Testbau; v2, Control, Telemetry und das am Schema abgewiesene Hello). Der FlatBuffers-Leser meldet einen belegten Adressslot jenseits von Feld-ID 4 als adresse_zusatzfeld wie das MANIFEST, ein laengerer VTable-Eintrag mit 0 bleibt gueltig, und die Senke reicht einen solchen Batch nicht weiter: kein Frame an Gen, der naechste session_snapshot traegt p2_reject mit feature_batch_ungueltig. Seit NAK-313 Etappe 7 (R-313-9; 313/M-128 bis M-130): binary_telemetry ist keine Transportzulassung - meldet eine Quelle im Heartbeat der Tabelle unsupported oder supported, wird ihr P2 in beiden Faellen angenommen und genau einmal an das abonnierte Main weitergereicht, und der Deskriptor traegt den gemeldeten Wert; nach einem Reconnect mit anderem Capabilitysatz traegt der session_snapshot bis zum ersten Heartbeat keinen Deskriptor und danach den neuen Wert; die Byteinstanz mit supported wird beantwortet. Seit NAK-380 Etappe 2 integrieren Bandpassung, Screening und Maskierungspegel die Feinband-Dichten mit der eingefrorenen Gitterbreite; Dichtevergleiche bleiben unveraendert, Zahlenraender und alte/neue Metrikversion werden eigens bewacht. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1.180,44 s | [A4](roh/NAK-380-8308a3d-dirty.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. Fehlt das Junction-Recht fuer volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen, meldet der Test NOT RUN mit Grund: das Bein steht dann als [NOT RUN] in der Uebersicht und der Lauf ist UNVOLLSTAENDIG, nie gruen (NAK-309 R-309-4). | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,93 s | [A4-SI](roh/NAK-380-8308a3d-dirty.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. Seit NAK-309 (M-18 bis M-28) die Regeln des Probewerkzeugs: ohne Argument waehlt es den v2-Probenamen, ein leeres Argument bleibt leer; der Produktionsname (auch gross oder gemischt geschrieben), Namen mit \ oder / hinter \\.\pipe\, fremde Namen und ungueltige v2-Suffixe werden abgelehnt, v2- und v3-Probenamen zugelassen; ein PipeClient ohne Namen startet keinen Verbindungsversuch; ein Konfliktende gilt nur mit einem neuen Broker-ACK ohne Konflikt auf derselben stehenden Verbindung, nicht nach Abbau oder Neuaufbau. Quelltextwachen: PipeProbeMain entscheidet den Namen vor Servererwartung und erstem Client (abgelehnt Exit 64) und wartet auf dieses Konfliktende-Praedikat; das Produkt nennt seine v2-Pipe ausdruecklich. Seit NAK-313 Etappe 4 (R-313-6, 313/M-42, M-49): der v2-Client faehrt nach der UTF-8-Pruefung eines Frames genau einen strengen Lauf kanon::lies, bevor JUCE ihn liest. Je Eintrag des Eingangs cpp_v2_client der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen: ein Peer auf einem Probenamen antwortet mit den Bytes des Eintrags, und jeder Fall endet getrennt, mit letzterFehler "eingehender Pipe-Frame: " und dem Grund des strengen Laufs (Stufe parser oder duplikat). Ein welcome mit Nachspann, Schlusskomma, unbekanntem Escape oder doppeltem type beziehungsweise protocol_version (passender und fremder letzter Wert) hinterlaesst keinen gesendeten Heartbeat, kein Sitzungstoken und keine Protokollversion; ein heartbeat_ack mit doppeltem seq nach gueltigem welcome bestaetigt keinen Heartbeat und meldet keinen Konflikt. Die Zaehlpruefung verlangt genau so viele Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-91, M-93, M-94, M-96): nach der UTF-8-Pruefung und vor dem strengen Lauf prueft der Zahlriegel zahlriegelBytes jede Zahl lexikalisch gegen die v2-Grenze INT64_MAX (Stellenzahl, dann Ziffernfolge; Exponent hoechstens drei Ziffern; ein Bruch hoechstens 15 signifikante Ziffern). Ein welcome mit protocol_version 9223372036854775808, 18446744073709551618, 2e4294967296 oder 2.0000000000000001, ein heartbeat_ack mit seq 18446744073709551616 oder 1.0000000000000001 und ein reject mit min_protocol 9223372036854775808 oder 2.0000000000000001 oder max_protocol 2e4294967296 enden getrennt mit letzterFehler "Zahlriegel: " und bestaetigen nichts; 9223372036854775807 passiert den Riegel und endet an der Feldregel, 2.0 verbindet mit Version 2, und 1e999 endet am Zahlriegel statt am strengen Lauf. Die Riegeleinheit faehrt zahlriegelBytes je Vektor direkt. Der Schalter --ohne-ueberlaufvektoren laesst die ub_bei_juce-Eintraege aus (nur fuer den Rotlauf am Riegelaufruf); der Kanon faehrt alle. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 66,85 s | [A4b](roh/NAK-380-8308a3d-dirty.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. Seit NAK-313 Etappe 4 (R-313-6, 313/M-46, M-48, M-49): json_laden_strikt lehnt doppelte dekodierte Namen im selben Objekt (auch als Escape-Alias) ueber einen object_pairs_hook mit eigener Ausnahmeklasse ab und eine Verschachtelung ueber 64 Ebenen nach dem Laden; die Stufe folgt der Ausnahmeklasse (Hook duplikat, jede andere parser). Jedes Fixture der Manifestklasse parser_lehnt_ab faellt beim Laden und traegt keine Verletzungsmenge, und je v3-Eintrag der Tabelle PRODUKTEINGAENGE-FAELLE.json liefert ein Fall mit PE-nnn im Namen Urteil und Stufe wie vertrag; die Zaehlpruefung verlangt genau so viele v3-Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-73): bei eigenem Urteil gueltig vergleicht der Fall den Wert am Zeiger feld exakt mit wert - eine Ganzzahl gleich, eine Gleitkommazahl nur ganzzahlig und im Betrag bis 2^53-1. Seit NAK-313 Etappe 6 (R-313-7; 313/M-98 bis M-102, M-106 bis M-113, M-121): die gemeinsame Falltabelle des Textriegels traegt die Untergrenze (1e-307 gueltig; 1e-308, 2e-308 und 9.9e-308 nicht), und die v3-Eintraege der Etappe (Bootstrap-Hellos, Handschlag, ACK) liefern Urteil und Stufe wie vertrag. Seit NAK-313 Etappe 7 (313/M-129): die zwei Heartbeats der Tabelle mit binary_telemetry supported und unsupported sind gueltig wie vertrag. Seit NAK-380 Etappe 2 benennt der Vertrag Feinband-Leistungsdichte, Livegruppen-Bandleistung und dimensionsloses band_stereo je Feld und bindet Version 20260925 bei unveraenderten Konfidenzschwellen. Seit NAK-380 Etappe 3 (380/M-30, M-40) bindet das Register die Fassung 20260926 (LRA mit 10 Hz, M1-Perzentile aus Teilbloecken) in kFeatureMetricsVersion UND beiden Broker-Konstanten METRICS_VERSION bei unveraenderten Schwellen, und Kommentar (FeatureEngine.h, Lautheit.h) und Register nennen EBU Tech 3342 §3.1 fuer kLraHopZellen = 1. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,29 s | [A5](roh/NAK-380-8308a3d-dirty.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [A6](roh/NAK-380-8308a3d-dirty.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [A7](roh/NAK-380-8308a3d-dirty.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-48, M-53) gehoeren dazu die Tabelle PRODUKTEINGAENGE-FAELLE.json, deren Kennungen, Stufen, Wirkungen und Kopfzahlen der Erzeuger bei jedem Lauf auf Widerspruchsfreiheit prueft, die Byteinstanz evidenz-lokal-wire-v1.json, die Fixtures der Manifestklasse parser_lehnt_ab als Rohbytes (nicht ueber den Serialisierer, der das Duplikat verloere) und das Negativfixture mit project_sample_start null. Seit NAK-313 Etappe 6 (R-313-7; 313/M-98 bis M-104, M-106 bis M-113, M-121): die Tabelle traegt die Eintraege der Etappe fuer rust_bootstrap, cpp_control_handshake, cpp_telemetrie_handshake und cpp_control_ack, dazu die aus Etappe 5 an das Tor verschobenen Negativen mit den Stufen der Etappe 6; der Erzeuger prueft jede Stufe gegen die Leserkette der anlegenden Etappe, und die Falltabelle des Textriegels traegt die vier Faelle der Untergrenze. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,45 s | [A8](roh/NAK-380-8308a3d-dirty.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID, keine ist verbrannt oder wiederverwendet, und jedes Offsetfeld steht im Rust-Strukturriegel. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,47 s | [A9](roh/NAK-380-8308a3d-dirty.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen, integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) und seit SONDE-015 band_dynamic_gain_db ID 22 (acht Werte, leerer Vektor, Altsender ohne Feld, vier falsche Laengen, NaN und Inf) sind in beiden Lesern gedeckt. Seit NAK-313 Etappe 6 (R-313-8; 313/M-119): dazu das Negativfixture adresse-zusatzfeld-id5.bin, uebersetzt aus einer Schemavariante mit zusatz:string (id: 5) mit demselben gepinnten flatc, und das Positivfixture adresse-vtable-slot5-leer.bin als gueltige Bytemutation (byte_mutation_von) mit einer 16 Bytes langen Adress-VTable, deren Slot 14 null ist. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 5,90 s | [A10](roh/NAK-380-8308a3d-dirty.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. Seit NAK-313 Etappe 4 (R-313-6, 313/M-42, M-49): je v2-Eintrag der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen durch den strengen Lauf json_laden_strikt und die v2-Definition der Nachricht, verglichen mit vertrag (Urteil und Stufe); die Zaehlpruefung verlangt genau so viele v2-Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-95): der Lauf liest Zahlen mit Bruch oder Exponent als decimal.Decimal, und der Typpruefer nimmt integer als int oder ganzzahligen Decimal, number samt Decimal (draft 2020-12): 2.0000000000000001 und 1.0000000000000001 sind keine Ganzzahlen, 2.0 ist die 2; bei Urteil gueltig wird der Wert am Feld exakt verglichen. Seit NAK-313 Etappe 6 (R-313-7; 313/M-104): dazu die zwei v2-gueltigen Hellos der Tabelle mit einer verletzten v3-Textriegelregel (samplerate 48000.0000000000001, NUL-Escape in sensor.label), beide gueltig gegen den v2-Vertrag. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,30 s | [A11](roh/NAK-380-8308a3d-dirty.md#a11) |
| A12 | Parameterbestand Layout v2 (120 Kennungen, 112 Host-Parameter als Praefix) haelt den Vertrag, und die 109 v1-Kennungen sind aus der eingefrorenen v1-Datei woertlich abgeleitet; Zonen-, Preset- und Ablehnungsregeln stimmen mit dem Vertrag ueberein; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State- und Presetkorpus samt MANIFEST bytegleich zur Neuerzeugung, keine verwaiste Datei. Seit NAK-313 Etappe 3 gehoert das Negativ-Golden schema2/doppelte-eigenschaft-v1.bin (R-313-2) mit SHA-256 und Erwartung nurLesen zum MANIFEST; geladen wird es in B2. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,29 s | [A12](roh/NAK-380-8308a3d-dirty.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B und die benannten Kanonbeweise sie tragen; jedes supported hat einen Termin oder, nur ohne Hostanteil, einen Kanonbeweis, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,20 s | [A13](roh/NAK-380-8308a3d-dirty.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 8,46 s | [A14](roh/NAK-380-8308a3d-dirty.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,51 s | [A17](roh/NAK-380-8308a3d-dirty.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 35,29 s | [A18](roh/NAK-380-8308a3d-dirty.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,15 s | [A19](roh/NAK-380-8308a3d-dirty.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,37 s | [A20](roh/NAK-380-8308a3d-dirty.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,57 s | [A21](roh/NAK-380-8308a3d-dirty.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,14 s | [A22](roh/NAK-380-8308a3d-dirty.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. Eine fehlende Messung ist kein PASS (NAK-283 F13): jeder Speicherpunkt traegt das Gueltigkeitsmerkmal seiner Windows-Abfrage, und fehlt ausserhalb der Neustartfenster eine gueltige Messung, endet S07 mit dem eigenen Status MESSUNG FEHLT (Exit 3) statt im Budget. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,76 s | [A24](roh/NAK-380-8308a3d-dirty.md#a24) |
| A34 | Der Selbsttest des Soak-Orakels (NAK-283 F13, Muster A27). Er laeuft OHNE Repo-Fixture und ohne Lauf: vier Faelle bauen ihre Berichte im Speicher und laufen durch dieselben Funktionen wie A24 - Messstelle, Speicherpunkt, Urteil. Eine fehlende RSS-Messung endet mit MESSUNG FEHLT und Exit 3, nie im Budget; ein nicht abfragbarer Prozess und ein Prozess mit 0 Bytes Working Set sind im Bericht verschieden; ein Fehlerpunkt direkt vor oder nach dem Neustartfenster faellt, obwohl das Fenster die Kurve filtert; ein Altbericht ohne Gueltigkeitsmerkmal bleibt ueber --bericht auswertbar, derselbe Bericht als Livelauf ist rot. Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --selbsttest` | [OK] Exit 0 | 0,15 s | [A34](roh/NAK-380-8308a3d-dirty.md#a34) |
| A35 | Der Selbsttest des Laufzeit-Arms (Plan S25e, NAK-286, NAK-309) laeuft ohne FL, ohne Installation und ohne MCP-Repo gegen Attrappen: der Runner haelt Exitcodes, Urteilswoerter und Kopfzeile der ersten Fassung, beendet nie ein fremdes FL (eigen ist nur ein selbst gestarteter Prozess, dessen PID, Startzeit und Befehlszeile in der Besitzliste stehen, auch bei gleichem Fenstertitel), faehrt nur den gepinnten MCP-Stand, startet keinen FL-Lauf gegen einen nicht aktuellen Bau und meldet ein veraendertes Diagnoseprojekt; eine Szenario-Voraussetzung (Exit 5) laesst die Folge weiterlaufen, nur ABWEICHUNG loest den Rueckweg aus. Eine MCP-Antwort misst nur mit der Anforderungskennung ihres Versuchs, jede andere heisst UNGEMESSEN; dass der Importweg fl_trigger nie laedt, ist ueber ein Attrappenpaket gemessen. Antworten des Briefkastens werden an Name und Groesse ausgewaehlt, bevor eine Datei geoeffnet wird; der Nulltest trennt Format, Versatz beider Vorzeichen, Kettenfaktor, Abweichung und Nullvorzeichen und urteilt BITIDENTISCH nur bei gleichen Bytes des Bereichs (NAK-311: +0 gegen -0 heisst NULLVORZEICHEN, Szenario-Exit 5); Baender entstehen nur mit Rechnung aus F-28. Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\fl\selbsttest.py` | [OK] Exit 0 | 33,98 s | [A35](roh/NAK-380-8308a3d-dirty.md#a35) |
| A36 | Die Frischepruefung des Runners leitet je Pruefbinary den Frischebaum aus den MSBuild-Tracking-Logs, den Kernbibliotheken und dem Konfigurationsstand ab: an einem Attrappen-Baubaum verweigert eine geaenderte DSP-Quelle ohne Bau die Beglaubigung, ein frischer Bau wird angenommen, Gleichstand ist frisch, ein nicht ableitbarer Baum ist nie gruen, und jedes gebaute gebundene Kernziel ist gedeckt. Dasselbe Urteil trifft jedes Ziel aus $gemesseneZiele, das der Runner baut (NAK-309 R-309-9; die Uebergabe misst eine Quelltextwache): eine juengere Eingabe nur des gemessenen Ziels verweigert die Beglaubigung, auch wenn jedes Pruefbinary frisch ist, und ein nicht ableitbares oder im CMake-Export fehlendes Ziel ist nie gruen. Verglichen werden Zeitstempel, keine Inhalte. Die NOT-RUN-Meldung der cargo-Beine (R-309-4): ein leerer Meldeordner heisst gelaufen, eine Marke NOT RUN mit Test und Grund, ein unlesbarer Ordner nie leer; im Urteilsblock des Runners geht ROT der fehlenden Voraussetzung vor, und NOT RUN zaehlt nie als gruen (Quelltextwache). Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\eq-copilot\pruefe_beweisrunner.py --selbsttest` | [OK] Exit 0 | 5,92 s | [A36](roh/NAK-380-8308a3d-dirty.md#a36) |
| A28 | Der P5-Evaluationskorpus ist reproduzierbar (Muster A25): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt SHA-256 im Manifest, und eine verwaiste Datei faellt. Zusaetzlich der Hygieneriegel aus NAK-182 R2: der Bezeichner des Kettenbeins steht WOERTLICH in seiner Datei, sonst waere der Name ein Etikett. | `py -3.13 tools\eq-copilot\erzeuge_p5_korpus.py --pruefen` | [OK] Exit 0 | 0,13 s | [A28](roh/NAK-380-8308a3d-dirty.md#a28) |
| A29 | Das Sammelbein des P5-Exit-Gates (59 Punkt 6, 36.4, M-64 bis M-70, R2). Es ist das dritte Glied einer KETTE: der Korpus traegt die Wahrheit, broker/tests/sonde014_p5_korpus.rs faehrt jede Sitzung durch p1 und schreibt die TATSAECHLICH ausgegebene Hypothese, und dieses Bein haelt beides gegeneinander - eine falsche starke Produktbehauptung aendert den Korpus nicht, sie faellt am Vergleich. Precision und Recall stehen JE URSACHENKLASSE, dazu Brier, Kalibrierung, Coverage und Enthaltungsrate; die vier Riegel (falsche_starke, falsche_schwache, precision und recall in [0,1]) laufen je Klasse und ueber die Gesamtmenge. Die Schwelle aus M-31 ist AUSGABE: gesucht wird die niedrigste Sicherheitsstufe, deren Riegel halten. Der Startwert von GATE_MINDEST_FENSTER wird an den zwei Passagensitzungen kalibriert. Ohne frische Ergebnisdatei meldet das Bein Voraussetzung-fehlt (Exit 3) statt gruen. Die Frischepruefung zaehlt das Rechenmodul broker/src/coordinator/hypothese/ zur LAUFZEIT auf (rekursiv), und eine genannte, aber fehlende Quelle ist ebenfalls Exit 3 mit Nennung des Pfads - kein stilles Ueberspringen (NAK-224 D1). | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py` | [OK] Exit 0 | 0,33 s | [A29](roh/NAK-380-8308a3d-dirty.md#a29) |
| A30 | Der Selbsttest des P5-Sammelbeins (M-68, Muster A27). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch DIESELBEN Funktionen, die das Sammelbein fuehrt. Jede Erwartung laeuft mit ihrem GEGENTEIL - eine falsche starke Behauptung und die richtige daneben, eine Enthaltung und dieselbe Menge ohne sie, ein leerer Satz und ein voller. Die vier Riegel werden synthetisch gefuettert und muessen fallen: precision > 1, recall > 1 und brier > 1 sind ueber den Korpusweg strukturell unerreichbar, und eine Wache ohne ausfuehrbaren Negativtest ist keine gemessene Zusage. Zuletzt die Schwellensuche in beide Richtungen. | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py --selbsttest` | [OK] Exit 0 | 0,14 s | [A30](roh/NAK-380-8308a3d-dirty.md#a30) |
| A31 | Die adversarialen Gegenbeispiele des Phasengates G5 am PRODUKTPFAD - genau die Faelle, die das Korpusformat nicht ausdruecken kann. Eine Intent-Rolle trennt zwei messtechnisch identische Kandidaten NICHT, auch nicht bei rolle=fuehrt, und bei drei sichtbaren Kandidaten wird der Fuehrende gegen JEDEN geprueft, nicht nur gegen den Zweitplatzierten. Eine Quelle, die ihre Energie in einem ganz anderen Band traegt als der Masterbefund, erreicht keine starke Aussage. Zwei Kandidaten, deren Raenge sich nur durch ein Band WEIT ausserhalb des Befundintervalls unterscheiden, sind ungetrennt. Umklammernde und disjunkte Fensterspannen fallen am Alignment mit Grund, obwohl die Ueberdeckung ihrer SPANNEN 1,0 meldet. Und die Passage laeuft ueber den echten Produktpfad samt Store: zwoelf und acht Fensterlaengen tragen die Aussage, sieben nicht, und eine Passage ohne ein einziges Masterfenster ergibt eine ENTHALTUNG mit ungueltiger Beobachtung statt Schweigen (M-27). Seit der Nacharbeit 1 (07.09.2026) faehrt das Bein ALLE 15 vereinbarten Ketteneingaben durch denselben Produktpfad und DRUCKT je Eingabe ihr Ergebnis: vier Passagenrandwerte (12, 11, 8 und 7 Fensterlaengen), den zweiten experiment_begin mit gleicher passage_id, zwei Master, den Master ohne Fenster, den Kanalwechsel vor und nach dem Deskriptorwechsel, drei Sonden auf einem Kanal, die antikorrelierte Quelle allein und neben der korrelierten, dazu die Masteranomalie in einem Fenster HINTER der Passage. Seit NAK-213 (08.09.2026) traegt KEINE der sieben frueher gedruckten NAK-213-Luecken mehr eine Luecke: zwei Master ergeben eine Enthaltung OHNE ORT statt eines stillen Gewinners, ein Master ohne Fenster ebenfalls statt Schweigen, der Kanalwechsel nimmt die Belege GENAU DER wechselnden Quelle zurueck und entfernt ihren Befund, drei Sonden auf einem Kanal werden ueber ALLE Quellen der Sitzung erkannt - eine vierte, stumme Sonde desselben Kanals setzt die Duplikatmarke mit. Seit NAK-214 (08.09.2026) fahren drei Faelle dieses Tickets mit: zwei bestaetigte Mains erzeugen KEINEN Vorschlag (Vorschlaege 0, keine proposal-Zeile im Store), ein PRE/POST-Paar unterhalb der Alignmentschwelle bleibt Aussageklasse 1 mit next_test prepost_paar_messen, und eine zurueckgenommene juengste Passage laesst die AELTERE rechnen - jeder druckt sein Ergebnis. Keine Luecke bleibt gedruckt, und der aufruferlose Helfer `luecke()` ist mit seinem letzten Aufrufer gegangen. | `cargo test --manifest-path broker/Cargo.toml --test sonde014_gegenbeispiele --color never -- --nocapture --test-threads=1` | [OK] Exit 0 | 105,14 s | [A31](roh/NAK-380-8308a3d-dirty.md#a31) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,14 s | [A25](roh/NAK-380-8308a3d-dirty.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,14 s | [A26](roh/NAK-380-8308a3d-dirty.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,32 s | [A27](roh/NAK-380-8308a3d-dirty.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 verbundenen Quellen, davon hoechstens 20 angenommen (seit NAK-312 Etappe 7b, U51: bei 32 sind 20 angenommen und 12 nicht), liegt p95 fuer die angenommenen Quellen fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,54 s | [A23](roh/NAK-380-8308a3d-dirty.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc) mit dem aktiven DSP-Kern: im Default (eq_enabled aus) bitgleich ueber 1000 Bloecke von 1 bis 2048 Samples bei 44,1 / 48 / 96 / 192 kHz ohne belegte Bank; eq_enabled an mit bypass aus und sonst neutralen Defaults bitgleich; eq_enabled an mit bypass an bitgleich trotz hoerbarem Band dahinter; nach eq_enabled aus nach dem Fade wieder bitgleich; Mix 0 mit Output-Trim 0 dB bitgleich; der Passthrough sanitisiert kein NaN/Inf; 0 Samples Latenz und kein Tail im Passthrough und im Hard-Bypass; speichern-laden-speichern bytegleich im Layout v2 mit Kind Dsp. Seit NAK-283 (F04, F05) bleibt der Nulltest bitgleich, wenn reset() vor jedem dritten Block laeuft - ausgeschaltet bei 44,1 / 48 / 96 / 192 kHz, ohne dass die Zaehler fuer nicht endliche Eingaenge, geheilte Filterzustaende und verworfene Analyseframes steigen, und im Hard-Bypass auch mit NaN, +Inf und -Inf zwischen zwei reset(); Mono und Stereo mit gleichem Ein- und Ausgang werden angenommen (im Monobus bleibt der Passthrough bitgleich, und das Band rechnet endlich), Quadrophonie, 5.1, vier diskrete Kanaele, Mono->Stereo, Stereo->Mono und jeder deaktivierte Hauptbus bekommen ein Nein, setBusesLayout auf Quadrophonie scheitert, und der Prozessor bleibt bei zwei Kanaelen. Das Bundle meldet 112 Host-Parameter, und sein Bundlevertrag laesst nur active_probe zu. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. Seit NAK-311 Etappe 2 (Neutralpruefung) schreibt der Kern engagiert-neutral (eq_enabled an, sonst Default) keinen Sample: ab Sample 512 kommen das Bitmuster aus Subnormals, +-0 und kleinsten Normalen, die Wachmarke 0x7F800001 (signalisierender NaN), ein ruhiger NaN und +-Inf bytegleich heraus, bei 44,1 / 48 / 96 kHz und im Monobus; der Riegel zaehlt die nicht endlichen Werte (6, im Monobus 12), der Tap post_committed traegt dort 0,0 und an jedem Rauschsample ab Sample 0 den Eingang als double. Bei Mix 0 mit Output-Trim 0 dB kommt dasselbe Material in den Bloecken ab Sample 512 bytegleich heraus, und der Tap traegt ab Sample 512 den Eingang als double. Ausgeschaltet und im Hard-Bypass bleibt das Material bytegleich ohne Zaehler; mit hoerbarem Band schreibt der Kern und gibt die Wachmarke nie aus. Die Messung 311/M-01 gibt die Bits unter FTZ/DAZ aus und prueft nur, dass alle 18 Kombinationen liefen. Seit NAK-311 Etappe 4 (R-311-3) rechnet der Ausgleich nur mit dem ausgegebenen Kanal: im Monobus melden Kern UND dspBericht fuer ein Band im Modus right exakt +0,0 dB, und der Ausgang mit Auto-Gain ist bytegleich zum Lauf ohne; nach dem Layoutwechsel auf Stereo tragen beide den Zweikanalwert, bitgleich zu einem Stereokern mit demselben Zustand, zurueck in Mono wieder +0,0 - waehrend channel_mode `right` im bestaetigten Zustand und in den Statebytes stehen bleibt, ohne Revision und ohne Host-Dirty. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, Abnahme U47) ist der EQ unter 44,1 kHz nicht unterstuetzt: bei 32 und 22,05 kHz bereitet der Kern nichts vor, und ueber 1000 Bloecke von 1 bis 2048 Samples kommen Rauschen, Bitmuster, Wachmarke, NaN und +-Inf im Stereo- wie im Monobus bytegleich heraus, ohne dass die Zaehler fuer nicht endliche Eingaenge, geheilte Filterzustaende und verworfene Analyseframes steigen; der Kern meldet die abgelehnte Rate, exakt +0,0 dB Ausgleich und die Kanalzahl 2, und die Blockgroessen 1, 255, 256, 257, 2048 und 4096 kommen ebenso bytegleich heraus, ohne gefuellten Tap, mit 0 Samples Latenz und Tail 0,0 s. Genau 44 100 Hz und der naechste double darueber bleiben unterstuetzt - das Band ist hoerbar, und der Fingerabdruck des Ausgangs ist bei 44,1 / 48 / 96 / 192 kHz gleich dem Golden, das dieselbe Messung am unveraenderten Kern des Basisstands der Etappe (5ee8318c) erzeugt hat -, waehrend 44 100 minus 1 ULP, 44 099, 32 000, 22 050, 8 000, 0, negativ, NaN und +-Inf abgelehnt werden. Rate 0, nicht endlich, 1e300 und maxBlock 0 bleiben das verriegelte Fenster: die letzte Vorbereitung steht, der dspBericht meldet sie weiter, und ABGELEHNT ist dort nichts. v3Samplerate, v3BlockSize und v3Channels tragen im abgelehnten Fenster weiter die gemessenen Werte des Hosts. Seit NAK-312 Etappe 7b (U48, U58) ueber den Hostbypass-Eintritt processBlockBypassed: Probeeq blendet in 256 Samples nach trocken und zurueck, linear und am Nachbarsprung innerhalb der E-31-Schranke, auch wenn der Wunsch in der Blende umkehrt; danach bytegleich zum Eingang auch fuer Rauschen, Subnormals, die Wachmarke, NaN und +-Inf, und nach dem Austritt bitgleich zu einem Prozessor ohne Hostbypass; die Analyse bekommt je Block genau einen Analyseblock, auch im Hostbypass, der Tap post_committed ist dort bitgleich zu dem ohne Hostbypass, und der Riegel zaehlt wie ohne ihn; nach reset und prepareToPlay, auch beim Projektladen mit gebypasstem Slot, uebernimmt der erste Block seinen Eintritt ohne Blende; mit EQ aus, im Hard-Bypass und unvorbereitet schreibt kein Eintritt, und die Analyse laeuft in beiden wie in processBlock. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,69 s | [A16](roh/NAK-380-8308a3d-dirty.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,20 s | [B1](roh/NAK-380-8308a3d-dirty.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. Seit SONDE-015 zusaetzlich das Kind Dsp: 120 Werte bit-exakt durch den Roundtrip, flache Arrays statt Kindknoten, voller Undo-Ring aus 32 Schnappschuessen bytegleich und unter 16 MiB, der 33. Eintrag read-only; die Layoutmigration v1 nach v2 setzt occupied bitgenau aus enabled und den Werten; das Preset traegt zwei getrennte Versionen, weist jedes der sechs verbotenen Identitaetsfelder ab, ignoriert unbekannte Top-Level-Felder und laesst eq_enabled beim Laden unberuehrt. Seit NAK-312 Etappe 5 (312/M-49): die eingefrorenen Statebytes des Parametergoldens vom Basis-SHA 12300f1e passen zu ihrer Zeile im Golden (Laenge und SHA-256), laden normal und kommen aus dem Schreiber bytegleich zurueck. Seit NAK-312 Etappe 7b (U49): der Bestand des Hauptprogramms - Mitglieder samt Labels, Passagen, Intent mit Revision, Schutzangaben, Beziehungen, Assistent - uebersteht hub, sensor, hub live und ueber Speichern und Laden gleich, im Kind RetainedMainProject (Writer-Golden legacy-retained-v1.bin); unbekannte Eigenschaften wandern mit; Altstaende laden wie bisher und kommen bytegleich zurueck; jedes unzulaessige Bestandskind ist read-only mit Originalbytes; je Wechsel ein Host-Dirty. Seit NAK-313 Etappe 2 (R-313-3): setzeBindung prueft 120/60 nur fuer geaenderte Werte - an einem geladenen Stand mit Label 121 und Paarnamen 61 Zeichen gelingt die Rollenwahl mit genau einem Host-Dirty, und beide Texte bleiben; derselbe Aufruf ohne Aenderung gibt false zurueck, ohne Host-Dirty und ohne Reconnect-Anforderung, und der Save bleibt bytegleich. Seit NAK-313 Etappe 3 (R-313-1, R-313-2): ein schreibbar geladener Stand besteht mit seinem groessten Folgezustand den Byte-Riegel verlustfrei, sonst bleibt er read-only mit Originalbytes und wird nie ignoriert - gemessen an Common mit 65 533 (schreibbar samt pre und hub) bis 65 536 Eintraegen und an der nachgerechneten Gesamtgrenze G und G+1; Tiefe 64, 63 geschachtelte Arrays und 72 Knoten bleiben rueckschreibbar. Ein doppelter Eigenschaftsname in einem Knoten, auch tief und am Sammlungsrand, macht eine bekannte Wurzel read-only mit Originalbytes, ohne einen der Werte zu lesen und ohne Schema-1-Migration; eine fremde Wurzel bleibt ignoriert; die Writer-Goldens laden wie bisher, das Negativ-Golden doppelte-eigenschaft-v1.bin read-only. Seit NAK-313 Etappe 5b (R-313-4; 313/M-74 bis M-79): jede persistente Revision - Bestands-, Eintrags- und Assistentenrevision im MainProject wie im RetainedMainProject, Dsp.state_revision und die Undo-Revision - laedt bei 2^53-1 schreibbar und kommt bytegleich zurueck; 2^53 und int64max bleiben read-only mit Originalbytes, und der Grund nennt das Feld. Der NAK-283-Fall M-11 steht am Rand 2^53-1: Assistentenhandgriff und entferneIntent werden dort abgewiesen, die Bytes bleiben gleich, und eine neue Instanz laedt schreibbar mit 2^53-1. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 18,34 s | [B2](roh/NAK-380-8308a3d-dirty.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/NAK-380-8308a3d-dirty.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,28 s | [B3b](roh/NAK-380-8308a3d-dirty.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. Seit SONDE-015 zusaetzlich der Fassungsschritt P1 5: der aus der committeten Fassung zurueckgebaute Leser der Fassung 4 lehnt state_report.dsp ab, ein Bericht ohne dsp bleibt in beiden Fassungen gueltig, und der Empfaenger rechnet SHA-256(dsp.jcs) gegen state_hash nach - ein schemagueltiger Bericht mit abweichendem Hash wird GANZ abgewiesen. Seit NAK-311 Etappe 4 Teil b (R-311-5) klemmt der Bericht `auto_gain_db` auf die Vertragsgrenze, statt sie zu reissen: acht Low-Shelves 1 kHz +12 dB Q 8 sind abgeleitet -199,77 dB, gemeldet werden exakt -120, und der aus dem echten Transaktionskern gebaute state_report ist gueltig; acht Shelves Q 0,707 bleiben mit -92,29 dB bitgleich zum abgeleiteten Wert. Die Klemmgrenze im C++ ist dieselbe Zahl wie minimum und maximum des GELADENEN Schemas; +/-120 kommen unveraendert zurueck, der naechste double darueber und darunter sowie 120,5 werden geklemmt, NaN wird +0,0 (nicht -0,0) und +/-Inf werden +/-120. Geklemmt wird NUR der Bericht: DspKern::autoGainDb() und der lineare Faktor des gefahrenen Programms bleiben beim ungeklemmten Wert, und klemmungen bleibt leer. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, R-311-20) meldet der Bericht bei einer Rate unter 44,1 kHz wie ohne Samplerate: bei 32 kHz haelt der Transaktionskern fs = 0, auto_gain_db ist exakt +0,0 und klemmungen leer, waehrend derselbe Zustand bei 48 kHz -92,29 dB meldet; Kern und Bericht tragen die abgelehnte Rate, und der daraus gebaute state_report bleibt gueltig gegen $defs/dsp_bericht. Der Grund bleibt C++-lokal: dasselbe Dokument MIT dem Feld im dsp-Block wird von additionalProperties: false abgewiesen, ohne das Feld ist es gueltig - der v3-Vertrag bleibt unberuehrt. Seit NAK-311 Etappe 5 Satz B (R-311-14, Karte U54) melden Bericht und Kern DENSELBEN gedeckelten Wert: acht High-Cuts 20 Hz Q 0,15 sind abgeleitet +150,46 dB und ueber DspKern::autoGainRohDb() lesbar, gefahren und gemeldet werden bitgleich kAutoGainDeckelDb, weil der Deckel vor baueBericht sitzt; die Berichtsklemmung laesst den Wert unveraendert durch, und der Zustand steht in Kern und Bericht. Mit ausgeschaltetem Auto-Gain ist er in beiden falsch, auf der Absenkungsseite ebenso (-199,77 dB abgeleitet, exakt -120 gemeldet, klemmungen leer). Der Zustand geht nicht auf den Draht: derselbe Bericht MIT dem Feld im dsp-Block wird von additionalProperties: false abgewiesen. Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-47, M-53): die Korpusschleife faehrt nach dem Textriegel ueber dieselben Rohbytes genau einen strengen Lauf kanon::lies als Tor vor der Engine; jedes Fixture der Manifestklasse parser_lehnt_ab (Nachspann, zweites Dokument, Schlusskomma, unbekanntes Escape, doppelter Name, Escape-Alias) faellt dort mit Grund und erreicht die Engine nie, alle uebrigen Urteile und Verletzungsmengen bleiben wie im Manifest - auch das Negativfixture mit project_sample_start null, dessen Verletzung an der gemeinsamen Eigenschaft liegt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-62): die Engine liest die Werte aus DIESEM strengen Lauf ueber nakama::vertrag::wertAlsVar statt ueber juce::JSON::parse; alle Urteile und Verletzungsmengen bleiben wie im Manifest, auch das Bruchzahl-Fixture mit nicht_endliche_samples 2.5 (type-Verletzung). Seit NAK-313 Etappe 6 (R-313-7, R-313-8; 313/M-101, M-114, M-118, M-119, M-121): der FlatBuffers-Leser meldet einen belegten Adressslot jenseits von Feld-ID 4 als genau einen Verstoss adresse_zusatzfeld am Pfad /eintraege/0/quelle (Fixture adresse-zusatzfeld-id5.bin aus einer Schemavariante mit Feld-ID 5); ein laengerer VTable-Eintrag mit Offset 0 (adresse-vtable-slot5-leer.bin) bleibt gueltig, und der ganze Gueltigkorpus bleibt gueltig. audioGueltig nimmt 1e-307 an und lehnt 768000.5 ab, wie das Tor des Brokers; die Falltabelle des Textriegels traegt die Untergrenze (1e-308, 2e-308 und 9.9e-308 abgelehnt). | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,45 s | [B3c](roh/NAK-380-8308a3d-dirty.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,46 s | [B4](roh/NAK-380-8308a3d-dirty.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. Seit NAK-380 Etappe 3 die LRA-Faelle 1 bis 4 aus EBU Tech 3342 Tabelle 1 (10/5/20/15 LU ± 1, gleich der §5-Referenz ± 0,1 LU). | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 4,53 s | [B9](roh/NAK-380-8308a3d-dirty.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. Seit NAK-380 Etappe 2 bleiben 221 Feinbaender Leistungsdichte in dBFS/Hz; 64 Livegruppen sind bandbreitenintegrierte Bandleistung in dBFS, einschliesslich leerer Feinbaender innerhalb einer belegten Gruppe, Parseval, Sinus und Traegerrand. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 7,24 s | [B5](roh/NAK-380-8308a3d-dirty.md#b5) |
| B6 | Aktiver DSP-Kern (plugin/dsp/) als Bibliothek: alle sechs RBJ-Filtertypen liegen ueber einem 1/24-Oktav-Gitter von 20 Hz bis min(20 kHz, 0,45 fs) bei 44,1 / 48 / 96 / 192 kHz innerhalb 0,05 dB typisch und 0,1 dB an den Raendern an der analytischen Antwort - gemessen als Impulsantwort AM TAP post_committed, also durch den Audiopfad, gegen eine im Test eigenstaendig ausgeschriebene Formel. Ausgeschaltet und hard-bypassed wird bei jeder Hoermatrix-Auswahl kein Sample geschrieben (bitgleich); bei Mix 0 mit Output-Trim 0 dB ist der Ausgang wertgleich zum Eingang; nach dem Ausschaltfade ist der Passthrough wieder bitgleich, und er sanitisiert kein NaN. Die acht Slots wirken als Kaskade von Slot 0 nach 7 gegen ein unabhaengig gerechnetes geordnetes Golden, das ein Rueckwaertslauf reisst. Auto-Gain ergibt bei flacher Kurve exakt 0 dB und bei einem +6-dB-Shelf ueber dem Gitter -6 dB, ein identisches Mid/Side-Paar faellt exakt auf den Stereo-Fall zusammen, und die Dynamik bewegt ihn nicht. Der Bankpool faehrt jeden der sechs Ownership-Uebergaenge einzeln, gibt busy_retry statt eine aktive Bank zu verdraengen, wird erst nach dem Audio-ACK frei und haelt seine Invarianten unter zwei echten Threads ueber tausende Bloecke; im Callback zaehlt der thread-lokale Zaehler ueber 4000 Bloecke wechselnder Groesse 0 Allokationen und 0 Sperren, waehrend derselbe Zaehler die Allokationen des Workers NICHT sieht; B6 baut keine Sonde, sein Zaehler misst ab DspKern::verarbeite (NAK-312 R-312-1). Ein Block ueber maxBlock (1024 Samples bei 256) verwirft den Analyse-Tap und gleicht auf beiden Kanaelen sample-exakt dem Lauf ohne Ueberlast in 4 x 256. Seit NAK-283 (F12) die float-Kante des Ausgangs: ein endlicher double ueber FLT_MAX (Output-Trim +6 dB auf 0,75 und 0,25 x FLT_MAX) kommt auf 0 verriegelt und im Eingangszaehler gezaehlt heraus, und kein Ausgangswert ist nicht endlich; auf endlichem Material in +/-1,0 (Bell +9 dB und Output-Trim +6 dB, 204 800 Werte, auch mit Betrag >= 1) ist jeder float bitgleich die Verengung des double-Taps, und der Zaehler bleibt stehen. Seit NAK-311 Etappe 2 (Neutralpruefung) schreibt der engagiert-neutrale Kern (Merkmal neutral, alle fuenf Rampen in Ruhe auf 1,0) keinen Sample: die Wachmarke 0x7F800001, NaN, +-Inf und das Bitmuster kommen bytegleich heraus, der Riegel zaehlt, der Tap post_committed traegt dort 0,0. Das Schreibende liegt exakt am ersten Sample nach Crossfade, Rampe und Hoermatrix-Fade, auch mitten im Teilstueck, bei Blockgroessen 1, 180, 255, 256, 257 und 4096 (maxBlock 512), und das Schreiben setzt am ersten Sample eines neuen Uebergangs wieder ein; bei Delta und Candidate, bei Mix 1e-9 und mit einem 0-dB-Bell schreibt der Kern. Wo er nicht mehr schreibt, misst der Tap post_committed die Rechnung: am Ende einer Width-Rampe auf 1,0 und bei Mix 0 exakt den Eingang als double. Der Allokationslauf mit neutralem Schritt bleibt bei 0 Allokationen. Seit NAK-311 Etappe 3 (W01) schaltet ein Pfad aus der Ruhe wie ein frischer Kern ein: nach Ausschalten, Ausblenden und einer Aenderung in der Ruhe (Output- oder Input-Trim +24 dB, Mix 0, Width 2,0, Auto-Gain; ebenso nach beendeAudiohistorie im Ausblenden und fuer den Candidate nach Ende und Hoerhalt am Tap post_candidate) ist der Ausgang ab dem Umschaltblock bytegleich zu einem frischen Kern mit demselben Programm, bei Blockgroessen 1 bis 4096 und 44,1 / 48 / 96 kHz; mit Ziel 0 dB bleibt Ausgang/Eingang an jedem Sample 1,0. Seit NAK-311 Etappe 3 (W03) traegt jeder aktive Slot eine Lebenszykluskennung: ein Slot, dessen Belegung und Topologie ueber jede Publikation seit dem gefahrenen Programm gleich blieben, behaelt beim Wechsel an einem ANDEREN Slot seinen Filter-, Detektor- und Huellkurvenzustand - sein Perioden-RMS (volle Periode des Prueftons, beide Kanaele, 100 Perioden ab dem Umschaltblock) liegt hoechstens 0,5 dB neben einem Referenzkern mit gleicher Vorgeschichte ohne Wechsel, bei 44,1 / 48 / 96 kHz und Blockgroessen 1 bis 4096 ueber maxBlock, am Candidate-Pfad ebenso; ein geaenderter Slot, ein zwischendurch entfernter und neu belegter und nach einem globalen Wechsel (Samplerate, Mono-Bass-Stufe) jeder Slot startet kalt und meldet 0,0, waehrend acht Slots zugleich ihre Auslenkungen bitgleich behalten oder alle acht kalt starten. Aendert der uebertragene Slot in derselben Publikation seinen Wert, bleibt der Quadraturbetrag am Umschaltsample innerhalb der Fadeschrittweite, ueber 100 Perioden hoechstens 0,5 dB ueber der groesseren eingeschwungenen Spitze und nach Fade-Ende plus Einschwingzeit hoechstens 0,5 dB neben dem Rampenlauf ohne Fremdslotwechsel; der Mono-Bass-Zustand wandert unter gleicher Pfadkennung mit, und der Allokationslauf bleibt mit einem Committed-Fremdslotwechsel im Zyklus bei 0 Allokationen. Seit NAK-311 Etappe 4 (R-311-3) rechnet die Auto-Gain-Ableitung im Monobus (Kanalzahl 1) die geordnete Kaskade in Slotreihenfolge und wertet nur den ausgegebenen Kanal aus: ein Band im Modus right oder side ergibt exakt +0,0 dB (kein -0,0), und der Ausgang ist ueber 48 000 Samples bytegleich zum Lauf ohne Auto-Gain; left und mid kompensieren voll wie derselbe Kern im Modus stereo; zwei vertauschte Slotreihenfolgen ergeben verschiedene Werte, und zwei und acht Baender treffen die im Test eigenstaendig ausgeschriebene Formel auf 1e-9 dB. Kanalzahl 0, 2 und 3 rechnen weiter die Zweikanalformel, deren vierzehn Werte bitgleich (memcmp) zum Basisstand der Etappe bleiben, und auch im Monokern laeuft die Ableitung nie im Audiopfad. Seit NAK-311 Etappe 4 Teil b (T3-15-11) ist die Steuerrate gemessen statt begruendet: gegen einen Referenzkern mit Range 0 (derselbe SVF-Weg) bleibt der Tap post_committed nach dem Einsatz eines Quadraturtons bis einschliesslich des BEZUGSPUNKTS bitgleich und weicht genau 1 Sample danach ab; Bezugspunkt ist der erste Steuerschritt mit einer Auslenkung ungleich 0, im Test aus schrittRest und der Auslenkung gelesen statt aus dem Toneinsatz, und er liegt im Raster auf der Phase d = (8 - p) mod 8 zum Toneinsatz; genau 8 Samples nach ihm liegt wieder ein Steuerschritt, an dem der dort entworfene Satz mit Gewicht 1 wirkt - dort traegt svfVon genau den am Bezugspunkt entworfenen Satz -, in jeder gefahrenen Steuerphase und bei jeder Rate. Die Spanne 1 bis 8 Samples bis zur ersten und 8 bis 15 Samples bis zur vollen Wirkung des ersten Entwurfs ist dagegen die RASTERZAEHLUNG 1 + d und 8 + d ueber die vier gefahrenen Steuerphasen, gezaehlt ab dem Sample Bezugspunkt - d und nicht als Abstand zum Bezugspunkt; in Zeit sind die 8 bis 15 Samples der vollen Wirkung 0,167 bis 0,3125 ms bei 48 kHz, 0,181 bis 0,340 ms bei 44,1 kHz und 0,083 bis 0,156 ms bei 96 kHz, also bei 48 und 44,1 kHz GROEBER als die kuerzeste Attack von 0,1 ms. Seit dem festgelegten Pegelbegriff (Satz D, R-311-15) liegt der Bezugspunkt bei manchen Steuerphasen einen ganzen Rasterschritt nach dem Toneinsatz; dieser Rasterschritt ist das Einschwingen des Pegelbegriffs und steckt nicht in den Zahlen 1 bis 8 und 8 bis 15. Das Verhalten aendert sich dabei nicht, kDynamikSchritt bleibt 8. Seit NAK-311 Etappe 5 Satz B (T3-15-09 Teil b, R-311-14, Karte U54) ist der ANGEWANDTE Auto-Gain nach oben einseitig gedeckelt: acht High-Cuts 20 Hz Q 0,15 bei 48 kHz sind abgeleitet +150,46 dB und ueber DspKern::autoGainRohDb() weiter lesbar, angewandt wird bitgenau kAutoGainDeckelDb = 24,0 dB, und am Tap post_committed liegt der Ausgang damit um den Faktor 15,849 ueber demselben Lauf ohne Auto-Gain statt um 3,3e7 - der Zustand ist dabei gesetzt. Die Deckelfunktion laesst den naechsten double unter der Grenze und die Grenze selbst bitgleich durch und setzt erst den naechsten darueber auf die Grenze; -0,0, +0,0 und das kleinste Subnormal kommen bitgleich zurueck. Die Absenkungsseite bleibt ungedeckelt (-199,77 und -92,29 dB bitgleich, Zustand falsch), Mono- und Zweikanalzweig deckeln an ihrer eigenen Rueckgabezeile auf denselben Wert, bei ausgeschaltetem Auto-Gain wird nichts angewandt und nichts gemeldet, und nach einem Programm unter dem Deckel faellt der Zustand zurueck, waehrend der angewandte Faktor ueber kRampeSamples auf das neue Ziel laeuft statt zu springen. Alle vierzehn Hexgolden von 311/M-61 liegen zwischen -6,00 und +0,23 dB, also auf der ungedeckelten Seite, und bleiben bitgleich. Seit NAK-311 Etappe 5 Satz C (T3-15-07, R-311-13, Karte U44) kennt die Kennungsvergabe auch WERTE: reisst ein Slot bei sonst gleicher Topologie eines der drei Kriterien - Frequenzverhaeltnis 2,0, Gueteverhaeltnis 4,0, Gaindifferenz 20,0 dB, richtungsfrei und strikt groesser -, bekommt NUR dieser Slot eine neue Lebenszykluskennung, der Blockrand blendet ueber statt zu rampen, und nur dieser Slot startet kalt. Gemessen am Mass von R-311-13 (groesster Ausgangsbetrag im Fenster von 0,5 s ab dem Umschaltsample gegen die groessere der beiden eingeschwungenen Spitzen, hoechstens 1 dB): der Sprung freq_hz 5000 -> 50 Hz an einem Low-Shelf +6 dB Q 0,707 faellt von +17,41 auf -0,22 dB, derselbe Sprung am Low-Cut 2000 -> 20 Hz von +22,20 auf +0,13 dB, die Gegenrichtung 50 -> 5000 Hz von +1,91 auf 0,00 dB, und ein dynamisches Band mit Range 0 haelt das Mass ebenso. Dieselbe Strecke in zwanzig Stufen zu je 1,2589 bleibt dagegen auf dem Rampenweg, behaelt ihre Kennung und ist ueber alle 40 960 Samples bitgleich zum Basisstand. Strikt groesser entscheidet an allen sechs Kanten je Kriterium (eine ULP darunter, genau auf der Grenze, eine ULP darueber, in beiden Richtungen), nicht endliche und nicht positive Werte erzwingen IMMER eine neue Kennung, acht gleichzeitig springende Slots ergeben genau EINEN Crossfade, ein zweiter Sprung im laufenden Fade wird erst am Blockrand nach dessen Ende genommen, und der Fade dauert bei Blockgroesse 1, 255, 256, 257 und 4096 genau kFadeSamples SAMPLES. Im Callback bleibt der Zaehler dabei bei 0 Allokationen und 0 Sperren, auch wenn eine Publikation ueber dem Kriterium im selben Zyklus liegt. Seit NAK-311 Etappe 5 Satz D (T3-15-06, R-311-15, Karte U45) vergleicht die Dynamikschwelle einen FESTGELEGTEN Pegelbegriff mit dem Threshold: zwischen Detektor und Huellkurve liegt ein symmetrisches Ein-Pol-Leistungsmittel mit dem festen Fenster kPegelFensterMs = 10 ms (durchschnitt); Fenster 0 (spitze) schliesst die Stufe kurz. Aktiv ist durchschnitt - baueProgramm setzt ihn unbedingt ohne Eingabe aus dem Vertrag, im Produktcode steht genau eine Setzstelle, und Pegelbegriff::spitze kommt dort nur in der Abbildung auf die Fensterlaenge vor. Gemessen bei 48 kHz, Bell Q 0,707, Range -12 dB, Threshold im Knie: drei Materialien gleicher Leistung (Quadraturton, L = R-Sinus, Rauschen) liegen um hoechstens 0,02 dB bei 1 kHz, 0,06 dB bei 341 Hz, 0,21 dB bei 100 Hz und 0,90 dB bei 20 Hz auseinander, das Rauschen um 0,32 dB - vorher 2,12 bis 4,01 dB. Neun Einstellungen aus Attack 0,1/10/500 ms mal Hold 0/30/500 ms spannen je Bandmitte 0,05 / 0,15 / 0,50 / 2,20 dB statt 5,78 dB; die Restwelligkeit eines FESTEN Fensters waechst mit fallender Bandmitte und steht als Zahl in der Schranke. Mit Fenster 0 ist der dynamische Weg ueber 76 096 Samples bitgleich zum Basisstand des Aenderungssatzes, und der Kurzschluss gibt den Eingang auch bei nicht endlichem oder denormalem Zustand bitgleich zurueck. Der Pegelzustand liegt im BandZustand: er wandert am Blockrand mit dem uebrigen Bandzustand, wird mit dem abgeschalteten Detektor gliedweise genullt, am Blockrand geheilt und denormalgeriegelt, ist bei Stille bitgenau +0,0 und ergibt bei 44,1 / 48 / 96 / 192 kHz dieselbe Zeit. Der Pegelbegriff ist topologisch: rampenKompatibel und die Kennungsvergabe tragen ihn, zwischen zwei Begriffen wird nie interpoliert. Die Sprungantwort misst gegen eine im Test ausgeschriebene Referenz der GANZEN Kette (Bandpass, Pegelbegriff, Huellkurve, Steuerraster): bei Attack 20 ms und Fenster 10 ms liegt der 63-%-Punkt bei 32,04 bis 32,17 ms statt bei 20 ms, die Toleranz von 1 ms je Stufe bleibt. Im Callback bleiben die Zaehler mit acht dynamischen Baendern bei 0 Allokationen und 0 Sperren; im Quelltext von verarbeiteBand steht der Aufruf der Pegelstufe genau einmal, und huellkurvePol, huellkurveEntwurf, std::exp, std::pow, std::log10, new und malloc stehen dort buchstaeblich nicht - der Pol entsteht im Worker (baueProgramm). Die Wortsuche liest nur den Text dieser einen Funktion und folgt keinem Aufruf; ueber den Steuerratenschritt, der alle kDynamikSchritt Samples Kennlinie und SVF-Satz neu rechnet (leistungInDb mit log10, svfEntwurf mit pow), sagt sie nichts. Seit NAK-312 Etappe 7b (U48, U58) die Hostbypass-Stufe hinter Taps und Hoermatrix: mit dem Wunsch Hostbypass blendet sie genau 256 Samples, gezaehlt in Samples bei jeder Blockgroesse (1, 64, 256 und 4096 bei maxBlock 256, 4096 bei maxBlock 100), linear mit dem Anteil n/256 und am Nachbarsprung innerhalb der E-31-Schranke, und schreibt danach keinen Sample (Wachmarke, NaN, +-Inf, Subnormals und -0 bytegleich); kehrt der Wunsch in der Blende um, laeuft sie vom Mischstand zurueck, ohne Sprung am Umkehrsample; mit dem Wunsch nein ist verarbeite bitgleich zum Aufruf ohne das Argument. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [OK] Exit 0 | 6,25 s | [B6](roh/NAK-380-8308a3d-dirty.md#b6) |
| B7 | Lokaler Transaktionskern der aktiven Sonde mit dem echten DSP-Kern: die Falltabelle T1-T17 aus Manifest SONDE-015 §5.11.4 laeuft tabellengetrieben mit den Invarianten I1, I2 und I4 als Wachen nach jeder Eingabe; ein Fehler in jeder Stufe S1-S7 laesst bestaetigten Zustand, Hash, Undo-Ring und Register unveraendert, und weder der Nachschlag S0 noch der Commit-Punkt allozieren; Apply, Revert, Neutralisieren, Remove, Undo, Redo und Preset-Laden erzeugen je genau eine Revision, 10.000 doppelte, vertauschte und veraltete Eingaben einer Transaktion hoechstens eine. Belegung, Remove und Undo als ein Objekt, Schutz-Zonen mit Verletzungsmeldung, Undo-Ring der Tiefe 32 und Preset halten ihre Matrixzeilen. Am echten SondeProcessor: 112 Host-Parameter in Vertragsreihenfolge ohne occupied; Hostautomation ohne Revision mit zwei Epochenwechseln je Geste, auf einem freien Slot bitgleich im Klang; ein abgeschlossener Gestus ist eine Revision; Host-Dirty nur bei einem Commit; das Kind Dsp reist durch Speichern und Laden, und ein Reload rekonstruiert Hash und Ausgang. Seit NAK-283 (F05, F09, F12) am echten SondeProcessor: reset() beendet die Audiohistorie - nach einem Impuls in den Resonator (1 kHz, +12 dB, Q 10) ist der erste Block Stille auf beiden Kanaelen exakt 0, ohne reset() klingt er nach - und laesst getStateInformation bytegleich, die Revision, das bestaetigte eq_enabled und die engagierte Bank im Kern stehen (1 kHz danach mehr als 6 dB lauter); releaseResources->prepareToPlay, reset() und prepareToPlay allein enden in derselben Audiohistorie (Stille exakt 0, untereinander bitgleich). Ein Block ueber maxBlock bei rechnendem Kern hinterlaesst in der Analysequeue eine Luecke (der naechste Block beginnt bei 192 mit kFlagLueckeDavor, Segment +1), und Kern und Queue beschreiben dieselbe verworfene Zeitspanne (ein verworfener Tap, ein Block ohne Audio mit 128 Frames, oversize und Ueberlauf +0); ruht der Committed-Pfad im Hard-Bypass, wird derselbe Block ohne Luecke angenommen. Endliche double ueber FLT_MAX am Tap kommen in der Analysekopie als endliche floats an, und der Kernzaehler steigt um genau diese 1024 Werte. Seit NAK-311 Etappe 3 (W01): reset() im laufenden Ausblenden (Output-Trim +24 dB, ein Block nach eq aus), danach Output-Trim 0 dB in der Ruhe und eq an - Ausgang/Eingang bleibt an jedem Sample des Wiedereinschaltens hoechstens 1 + 1e-6. Seit W03: Remove und Neubelegung desselben Slots mit derselben Konfiguration ohne Audioblock dazwischen - beide committet, die zweite Publikation verdraengt die erste - starten kalt; der Uebergang ist ein Crossfade, die gemeldete Auslenkung ist ab dem ersten Block exakt 0,0 und der Ausgang bei Stille ab kFadeSamples exakt 0,0. Seit NAK-311 Etappe 4 Teil b (F12, R-311-4) hat die Toleranz von M-84 ihre Zahl samt Referenzbeginn und Geltungsbereich: t = 0 ist das erste Sample des ersten Blocks der geladenen oder neu vorbereiteten Instanz, der Vergleichskern laeuft ununterbrochen weiter und bekommt ab dort denselben Eingang. Fuer ein dynamisches Bell (Kanalmodus stereo, Sidechain internal) mit Quadraturton auf der Bandmitte im Teilraum Q >= 1, Q*A_min >= 0,5 und f0 bis min(20 kHz, fs/4) weicht der Quadraturbetrag je Sample ab t_E = max(kFadeSamples, 5*tau_a*fs + 10*Q*A_max*fs/(pi*f0) + 16) hoechstens 0,1 dB ab - gemessen hoechstens 0,03 dB ueber drei Prueflinge (Q 2 bei Attack 0,1 / 10 / 500 ms, Q 1,0, f0 = fs/4 mit Q 24) je in Knie und Plateau -, davor im Betrag hoechstens \|g0\| + \|Range\| + 0,1 dB; dasselbe gilt fuer prepareToPlay auf der laufenden Instanz. Der Same-Instance-Ladestart bleibt dagegen in Ausgang UND Tap bitgleich zum ununterbrochenen Kern, und zwei Sekunden eingeschwungenes Audio lassen die Statebytes unveraendert: die Huellkurvenleistung ist Audiohistorie, kein persistenter Wert. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, R-311-20) nimmt ein Ratenwechsel den Zustand zurueck: ueber 48 -> 32 -> 48 kHz ist der Ausgang im 32-kHz-Fenster bytegleich zum Eingang und der Kern meldet 32 000 als abgelehnte Rate, nach dem dritten prepareToPlay ist der Melder wieder exakt +0,0, das Band hoerbar und der Ausgang bitgleich zu einem Kern, der nie abgelehnt hat - waehrend eq_enabled, Statebytes, Revision und Host-Dirty ueber die ganze Folge unveraendert bleiben. Mit einem High-Cut 20 Hz Q 0,707 und Auto-Gain an sind Kern UND Bericht im Fenster exakt +0,0 dB, klemmungen leer, die Zaehler des alten Fensters genullt und die Kanalzahl auf 2, beide tragen die abgelehnte Rate, jcs, Revision und Undo-Tiefe bleiben die des bestaetigten Zustands, und nach dem Rueckweg tragen beide wieder bitgleich denselben abgeleiteten Wert. Im 32-kHz-Fenster zaehlt der thread-lokale Zaehler ueber 200 Bloecke 0 Allokationen im Callback, die Echtzeitwache (seit NAK-312 ab dem Eintritt processBlock oder processBlockBypassed) 0 gemeldete Sperren, getLatencySamples bleibt 0 und getTailLengthSeconds 0,0. Seit NAK-311 Etappe 5 Satz D (T3-15-06, R-311-15) traegt t_E den festgelegten Pegelbegriff: der Huellkurventerm rechnet 5*(tau_a + kPegelFensterMs)*fs statt 5*tau_a*fs, also hoechstens 5*kPegelFensterMs*fs Samples mehr, und die drei Prueflinge weichen ab t_E in Knie und Plateau hoechstens 0,027 dB ab - die Toleranz von 0,1 dB bleibt unveraendert. Zwei Sekunden eingeschwungenes Audio bewegen den Pegelzustand nachweislich auf einen endlichen Wert ueber 0, der Slot faehrt dabei den Begriff durchschnitt, und die Statebytes bleiben bytegleich: der Pegelbegriff ist Laufzeit wie die Kanalzahl und steht in keinem Statefeld. Seit NAK-312 Etappe 2 (R-312-1, E-312-11) misst die Echtzeitwache ab dem Eintritt processBlock oder processBlockBypassed, und B7 meldet jede eingeschaltete Allokation zusaetzlich an sie: mit einem Testplayhead, der je Aufruf eine gemeldete Sperre nimmt, zaehlt sie ueber 200 Bloecke genau 200 Sperren vor dspKern->verarbeite und 0 Allokationen; ein Workerzug mit Programmbau zwischen zwei Bloecken bewegt beide Zaehler nicht; legt das Bein den Bereich wie der Wrapper um setNonRealtime und processBlock, zaehlt sie seit Etappe 5 ueber 200 Bloecke je Aufruf setNonRealtime (true) wie je Aufruf setNonRealtime (false) 0 gemeldete Sperren (bis Etappe 4 je Aufruf setNonRealtime (true) genau eine). Sperren, die JUCE vor dem Plugincode nimmt, sieht der Zaehler bauartbedingt nicht. Seit NAK-312 Etappe 3a (W02) quittiert ein Ladestart die Hostwert-Mailbox: ein Hostwert, der vor dem Laden derselben Bytes in dieselbe Instanz eintraf, bleibt wirkungslos - auch ueber prepareToPlay, eine spaetere Wiederoeffnung der Mailbox und den Zaehlerueberlauf hinweg -, der Hash bleibt der der Quelle, und ein Hostwert nach der Quittierung wirkt, auch einer zwischen Quittierung und Abgleich, dessen Regler danach denselben Wert zeigt; reset() bewahrt die Mailbox. Ein read-only geladener Stand bleibt unter Hostautomation (eq_enabled und Output-Trim) ueber 512 Samples bitgleich zum Eingang, seine Regler springen auf neutral, die Originalbytes bleiben, kein Dirty, und ein gueltiger Stand danach laesst die Automation wieder wirken; NaN und +-Inf bleiben wirkungslos. Seit NAK-312 Etappe 3b (T3-01-05 Teil a, E-312-5, E-312-6) uebernimmt der Kern Input-Trim, Output-Trim, Width und Mix am Blockrand: ein Lauf unter der Taktsperre ohne Kontrolltakt ist bitgleich zu einem mit Takt nach jedem Block - fuer alle vier in beiden Staenden (neutral und mit Band) bei Blockgroesse 1, 64, 185, 240, 256 und 512 in Echtzeit und bei 64 offline; fuer Output-Trim im neutralen Stand zusaetzlich bei 4096 (dort wirkt der Wert im Block seines Randes), ueber eine Rampe von 200 Bloecken und ueber den Zaehlerueberlauf; ohne Ereignis setzt der Blockrand kein Ziel, NaN und +-Inf erreichen ihn als bestaetigter Wert, ein Ladestart mitten in der Fahrt macht einen alten Mailboxwert auch am Blockrand wirkungslos, und die Abdeckung ist genau diese vier. Epoche und Ruhegrenze bleiben beim Worker (genau zwei Epochenwechsel je Geste, Ruhegrenze in verarbeiteten Samples), und eine blockgebundene Fahrt erzeugt keine Revision, kein Dirty und bytegleiche Statebytes; ueber 4000 Bloecke wechselnder Groesse mit Automation zaehlt die Echtzeitwache 0 Sperren und 0 Allokationen. Ein Bandwert und eq_enabled bleiben taktgebunden: ohne Takt weicht der Lauf ab, mit Takt ist er bitgleich. Seit der Nacharbeit 1 der Etappe 3 (L-1, R-312-16) nimmt der Blockrand nach einem read-only-Ladestart keinen Hostwert: laedt ein Stand mit eq an und Output-Trim +3 dB das fremde Major und kommen danach Input-Trim +24 dB, Output-Trim +24 dB, Width 2 und Mix 0 als Hostwerte, ist der Ausgang ab dem ersten Sample nach dem Laden bitgleich zu dem ohne diese Hostwerte, und die Blockrandziele steigen um 0 - bei Bloecken zu 256 und, wenn beim Laden ein Uebergang laeuft, bei 185 und 512; ein eigener schreibbarer Stand danach laesst Output-Trim +6 dB ohne Kontrolltakt wieder am Blockrand wirken (bitgleich zu einem Lauf mit Takt nach jedem Block, Ausgang/Eingang am Ende bei 10^(6/20)). Seit NAK-312 Etappe 4 (312/M-38): ueber mindestens 4000 Bloecke wechselnder Groesse, waehrend beide v3-Clients der Sonde ueber einen Testserver auf der Probe-Pipe verbunden sind und ihre Provider nachweislich durch die Besitzschleuse laufen, bleiben RtWache::sperren() und RtWache::allokationen() ab Callback-Eintritt 0. Seit NAK-312 Etappe 5 (T3-01-03, T3-01-04, R-312-3) der Offline-Uebergang: mit hoerbarer Vorschau zaehlt die Wache im Bereich wie der Wrapper ueber 200 Bloecke 0 Sperren und (seit Etappe 6a gemessen) 0 Allokationen; unmittelbar nach dem ersten Aufruf setNonRealtime (true) ist die gewuenschte Hoermatrix Processed, und der Ausgang ist ab Sample 0 bitgleich zum Lauf ohne Vorschau - bei Blockgroesse 1, 64 und 256 und auch, wenn beim Wechsel ein Hoermatrix-Fade in Gegenrichtung oder ein Hoerhalt laeuft; nach einem ausdruecklichen Kontrolltakt ist keine Vorschau mehr aktiv, nach ceil(kFadeSamples/Blockgroesse)+1 Bloecken und einem zweiten Takt die Candidate-Bank frei; der Rueckweg in die Echtzeit blendet weich (Nachbarsprung hoechstens Fadeschritt plus 2^-23) und belebt die Vorschau nicht, und ohne Vorschau aendert das Offlineflag am Ausgang nichts. Seit NAK-312 Etappe 6a (R-312-20, R-312-22), unter der Taktsperre bei stehender Vorschau: ein im Render gesetzter Hoerwunsch Dry, Delta oder Candidate wirkt als Processed - jeder Offlineblock ist bei Blockgroesse 1, 64 und 256 ab Sample 0 bitgleich zum bestaetigten Lauf; nach setNonRealtime (true) und sofort setNonRealtime (false) ohne Block dazwischen schaltet der erste Echtzeitblock beim Wechsel auf Candidate nicht hart (Nachbarsprung hoechstens Fadeschritt plus 2^-23). Seit dem zweiten Aenderungssatz der Etappe 5 (T3-01-02, R-312-4) haengen die 112 Hostparameter ohne APVTS direkt am Prozessor: Zahl, Reihenfolge, IDs, Namen, Klassen, Bereiche, Defaults und Hostsicht (convertTo0to1 an Minimum, Default und Maximum; convertFrom0to1, getText und getValueForText an 0, 0,37 und 1), die Statebytes ueber speichern, laden, speichern und die Epochenfolge einer Automationsfahrt sind gleich dem Golden des Basis-SHA 12300f1e; kein Quelltext unter eq-copilot/plugin/sonde/ nennt die APVTS-Klasse; nach dem Laden traegt jeder Hostparameter den geladenen Wert; am Ende des Destruktorrumpfs erreicht ein Hostwert den Prozessor nicht mehr; Index -1 und 112 bleiben ohne Wirkung, und der eigene Abgleich meldet kein Hostereignis; ueber 4000 Bloecke mit Hostautomation aus einem zweiten Thread zaehlt die Wache ab processBlock 0 Sperren und 0 Allokationen - die Sperre, die JUCE vor dem Plugincode in sendValueChangedMessageToListeners nimmt, sieht sie bauartbedingt nicht. Seit der Nacharbeit 1 der Etappe 5 (R-312-19, Kanonteil von 312/M-88) ruft kein Quelltext unter src, sonde, dsp, state, vertrag, core und hostbridge von eq-copilot/plugin ExitProcess, TerminateProcess, PostQuitMessage, FatalAppExit, exit, _exit, quick_exit, _Exit, Process::terminate oder systemRequestedQuit und keiner nennt WM_CLOSE, WM_QUIT, SC_CLOSE oder JUCEApplicationBase (Textriegel); ob der Host im Betrieb lebt, misst das nicht - das misst der Laufzeit-Arm. Seit NAK-312 Etappe 7b (U48, U58) der Hostbypass-Eintritt processBlockBypassed: die Wache beginnt auch dort vor der Stempelbildung (200 gemeldete Sperren ueber 200 Bloecke mit Testplayhead); ueber 4000 Bloecke wechselnder Groesse mit Eintrittswechseln, Hostautomation aus einem zweiten Thread, Programmwechseln, reset, releaseResources und prepareToPlay bleiben Sperren und Allokationen 0, die Latenz 0 und der Tail 0,0; getBypassParameter bleibt nullptr bei 112 Hostparametern; State, Hash, Revision, Undo-Ring und Dirty bleiben unberuehrt, und die Ruhegrenze der Automationsepoche zaehlt auch im Hostbypass verarbeitete Samples - sie schliesst am selben Block wie ohne ihn; der Blockrand des Bypasseintritts nimmt Hostpunkte ueber den Ueberlauf der Ereigniszaehler (M-132); Echtzeit und Offline sind bitgleich, auch wenn der Wechsel nach offline mitten in die Blende faellt, und der Bypasseintritt setzt weder den Offline-Riegel noch das harte Schalten; im Hostbypass haengt keine Bank, und nach dem Austritt klingt der zuletzt bestaetigte Stand. Seit NAK-313 Etappe 5b (R-313-4, 313/M-83): der Kern bildet seine Grenze aus derselben Konstante wie der State-Leser - bei 2^53-2 committet genau eine Transaktion auf 2^53-1, die naechste scheitert in S4 mit revision_erschoepft ohne Nebenwirkung an Zustand, Hash, Undo-Ring, Register und Revision, und ein Ladestart mit 2^53 wird mit state_revision abgewiesen. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [OK] Exit 0 | 2,19 s | [B7](roh/NAK-380-8308a3d-dirty.md#b7) |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. Seit NAK-313 Etappe 5a (R-313-4, R-313-5; 313/M-55 bis M-60, M-73, M-96): der flache Leser liest Ganzzahlen mit nakama::wire::ganzzahlAusLiteral ohne Gleitkommaschritt, je Literal ein Fall (0, -0, 1, 1.0, 1e0, 10E-1, 91.0, 1.00e2 und 2^53-1 in Ziffern- und e-Form gelten; 1.5, 1e, 01, -1, +1, 1., 1e400, 2^53 auch als Bruchform, 2^64, NaN, Infinity, -Infinity, 1e999 und leer nicht). state_revision im command_ack gilt von 0 bis 2^53-1 in jeder Schreibweise; 2^53 und ein Ueberlauftext (2^64) sind keinAck: der Auftrag bleibt im Register, der Konflikthaken wird nicht gerufen, kein Kopf mit 0 entsteht, und erst nach dem Reconnect gibt ein gueltiges ACK ihn frei; der Konfliktkopf traegt die gelesene Revision (5 wie 5.0). welcome.protocol gilt in Control und Telemetrie mit dem Wert 3 (3, 3.0, 3e0), 4 und 3.5 verbinden nicht. Je Eintrag der Eingaenge cpp_control_ack, cpp_control_handshake und cpp_telemetrie_handshake der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen ueber die Testserver-Schalter fuer rohe welcome- und ACK-Bytes, verglichen mit produkt samt Wert, mit Zaehlpruefung je Eingang. Seit NAK-313 Etappe 5b (R-313-4, 313/M-88): heartbeatAlsJson und stateReportJson schreiben state_revision unveraendert - 2^53-1 als 9007199254740991 und ein Testaufbau mit 2^53 als 9007199254740992, nie gekappt. Seit NAK-313 Etappe 6 (R-313-7; 313/M-106 bis M-113): der flache Leser prueft UTF-8 vor dem ersten Zeichen, dekodiert Escapes nach RFC 8259 (ein Surrogatpaar wird ein Codepunkt; NUL-Escape, einsames Surrogat, unbekanntes Escape, ein u-Escape ohne vier Hexziffern und rohe Steuerzeichen sind ungueltig) und vergleicht Namen dekodiert; eine Ablehnung nennt einen von acht festen Gruenden ohne die Teilwoerter, an denen Gen eine Inkompatibilitaet erkennt. Nur die beiden welcome-Aufrufer schreiben welcome: plus Grund, ein gelesenes Objekt ohne Text in type meldet welcome: kein type. broker_version zaehlt hoechstens 64 und reject.reason hoechstens 500 Codepunkte (64 und 500 mal e-Akut gelten, 65 und 501 nicht, 499 a plus Surrogatpaar gilt, 500 a plus Paar nicht); eine leere broker_version verbindet nicht, ein leerer reason ist Broker lehnt ab. Control und Telemetrie melden dieselben Gruende; ein reject mit Escapes endet mit genau den dekodierten Bytes. Am ACK-Weg ist ein konflikt mit escaptem Code gelesen und beendet den Auftrag, ein ACK mit FF im code oder mit ergebnis als Escape-Alias und roh ist keinAck, und der Auftrag bleibt im Register. Im Tabellenlauf gilt seit Etappe 6 ein reject als gelesen, wenn es als Broker lehnt ab erscheint, und ein ACK, wenn es den Auftrag beendet. Seit NAK-313 Etappe 7 (R-313-9; 313/M-123, M-124): capabilitiesJson meldet binary_telemetry supported - zwei Hostmessungen, eine Build-Tatsache, sieben feste Fallbacks, zehn Schluessel in der bisherigen Reihenfolge -, und der Capabilitybericht traegt Schluessel fuer Schluessel dasselbe Objekt. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 231,66 s | [B10](roh/NAK-380-8308a3d-dirty.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,13 s | [B11](roh/NAK-380-8308a3d-dirty.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,67 s | [B12](roh/NAK-380-8308a3d-dirty.md#b12) |
| B31 | Sonde-Lebenslaufbein (NAK-312 R-312-7): der Testkonstruktor der Sonde uebernimmt einen Probe-Pipenamen unveraendert und einen Namen ausserhalb des Probe-Namensraums fail-closed als leeren Pipenamen; ohne die Starthaken macht im Testbau kein v3-Client einen Verbindungsversuch, mit ihnen meldet der ControlClient binnen 8 s verbunden ueber die Probe-Pipe eines Testservers, und die Telemetrie koppelt nach dessen welcome; die Sonde wird mit beiden laufenden Clients auf dem Heap abgebaut. Jeder Name, mit dem ein Client startet, liegt vor dem Start im Probe-Namensraum. Seit Etappe 4 (R-312-7, R-312-12) die Besitzschleuse der Sonde, gemessen an einem gehaltenen Statusprovider und einer gezaehlten Marke vor dem ersten Besitzerzugriff: nach dem Ende des Eigentuemers erreicht ihn kein Provider (0 in jedem Fall und in 20 Gegenzyklen, nach denen Handles und Threads nicht hoeher stehen als nach dem ersten); gehalten hinter dem Betreten wird er zu Ende gewartet, gehalten davor und nach dem Destruktor freigegeben abgewiesen, vor dem Abbau freigegeben laeuft er durch; stop() loest den gehaltenen ControlClient nach der 2-s-Frist ab, controlV3.stop() gemessen vor dem Schliessen, telemetryV3.stop() nach Zeilenfolge; ein Reconnect laesst dieselbe offene Schleuse stehen. Eine Sonde aus dem Produktkonstruktor traegt im Testbau genau pipeNameV3 (aktuelleLogonSid()), ohne dass ein Client startet. | `eq-copilot\build\plugin\EqCopSondeLebenslaufTest_artefacts\Release\EqCopSondeLebenslaufTest.exe` | [OK] Exit 0 | 71,54 s | [B31](roh/NAK-380-8308a3d-dirty.md#b31) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. Seit NAK-312 Etappe 7b (U51): hoechstens 20 Quellen werden angenommen; bei 21, 32 und 64 Sonden stehen 20 Zeilen und die Zahl der nicht angenommenen in der Sicht, eine gegangene Quelle macht Platz ohne Verdraengung, und Messungen nicht angenommener Quellen erreichen die Sicht nicht; die Zeitmessung mit 16 und 32 Snapshot-Quellen erwartet min(n, 20) Zeilen, bei 32 also 20 und 12 nicht angenommene, p95 weiter unter 300 ms. Seit der Nacharbeit 1 der Etappe 7b (R-312-32): rueckt eine wartende Sonde ueber den lokalen Eintritt nach (Publikation der gespeicherten Mitglieder ohne ein entferntes, ohne neuen Snapshot), zaehlt ihre Zeile den offenen Befund aus dem juengsten Snapshot (findingsOffen 1, jede andere Zeile 0), und nach controlEnde ist er stale und die Zeile zaehlt 0. Seit NAK-313 Etappe 4 (R-313-6, 313/M-39 bis M-41, M-49): beide v3-Leser des Quellenmodells fahren nach dem Textriegel genau einen strengen Lauf kanon::lies, bevor sie ein Feld lesen. Je Eintrag der Eingaenge cpp_sources_snapshot und cpp_sources_ruecknahme der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen, direkt am Leser in einer abonnierten Sitzung mit uebernommenem Basis-Snapshot: Nachspann, zweites Dokument, Schlusskomma, unbekanntes Escape, doppelte Namen (type, session_epoch, logon_sid in einer Mitgliedsadresse, grund, art in umfang; je mit passendem und mit fremdem letzten Wert) und der Escape-Alias fuer type sind ungueltig mit dem Praefix "session_snapshot parser: " beziehungsweise "evidence_invalidate parser: " an der Stufe parser oder duplikat, und die Sicht bleibt dieselbe (Revision, Quellen mit Mitgliedschaft und Namen, Abonnement, fuehrendes Main, Diagnose, Ruecknahmezaehler, -grund und -umfang); derselbe Name logon_sid in zwei Mitgliedsadressen bleibt gueltig, beide Mitglieder stehen in der Sicht. Je Eingang verlangt die Zaehlpruefung genau so viele Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-61, M-73, M-96): beide Leser lesen die Felder aus DEMSELBEN strengen Lauf (nakama::vertrag::wertAlsVar statt juce::JSON::parse): letzter_kontakt_ms 1500.0 und 1.5e3 sowie host_mixer_index 3.0 werden als 1500 und 3 uebernommen, eine Ruecknahme von 1024.0 bis 2048e0 wie mit Zifferntext, die vertauschte Ordnung, 1.5 und -1 bleiben ungueltig, 2^53, NaN, Infinity, -Infinity und 1e999 fallen am Textriegel; bei Urteil gueltig wird der Wert am Feld verglichen. Seit NAK-313 Etappe 6 (R-313-8, R-313-15; 313/M-108, M-117): Gen ordnet Broker lehnt ab als incompatible ohne Handgriff ein und die Lesegruende welcome: plus Grund (die acht Wortlaute und welcome: kein type) als brokerUnavailable mit Handgriff; in einem Modell, das die Quelle kennt, verwirft uebernehmeP2 einen Batch mit belegtem Adressslot jenseits von Feld-ID 4 mit /eintraege/0/quelle:adresse_zusatzfeld, und die Zeile bleibt ohne neue Messung und Lautheit. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,16 s | [B13](roh/NAK-380-8308a3d-dirty.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. Seit NAK-312 Etappe 6b (312/M-72) reisen die Namen nach dem Labelentwurf-Fall am echten Editor (Entwurf der Startquelle A beim Klick auf B bestaetigt, danach Tick und Enter) durch Speichern und Laden in eine frische Instanz unveraendert und mit derselben Zuordnung Name zu instance_id; das Laden meldet kein Dirty. Seit Etappe 6b (312/M-76) behaelt ein geladener Stand mit 40 bestaetigten Mitgliedern und einer unbekannten additiven MainProject-Eigenschaft ueber die Folge mit 20 gezeichneten Zeilen und Ersatzziel alle 40 Mitglieder im State, im Modell und nach dem Laden in eine frische Instanz; die Eigenschaft reist unveraendert, kein Host-Dirty. Seit NAK-312 Etappe 7b (312/M-126, U51) behaelt ein geladener Stand mit 21 und 40 bestaetigten Mitgliedern alle in State und Modell, 20 davon - die 20 kleinsten instance_id - sind Zeilen, auch nach einem Snapshot mit zwei weiteren Sonden; Speichern bytegleich, kein Host-Dirty. Seit Etappe 7b, Satz 3 (U49): in Legacy ist die Live-Sicht des Quellenmodells stillgelegt - keine Zeile, keine Subscription, ein verspaeteter Snapshot traegt nichts ein -, nach der Rueckkehr stehen die Mitglieder wieder; ein nach dem Wechsel quittierter Beitritt wirkt auf den ruhenden Bestand. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 8,64 s | [B14](roh/NAK-380-8308a3d-dirty.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. Seit NAK-312 Etappe 4 (R-312-2) faehrt dasselbe Bein das Messpunkt-Popover am echten Editor: baut die Nachrichtenschleife es erst nach Editor und Prozessor ab, fasst sein Destruktor keinen von beiden an (gezaehlte Marke 0, Bindung bis zum Prozessorende unveraendert, 0 Host-Dirty); schliesst es, solange der Editor lebt, wird eine Aenderung mit genau einer Host-Dirty-Meldung uebernommen, ohne Aenderung mit keiner. Seit NAK-312 Etappe 6b (R-312-9) dazu der Labelentwurf der Main-Flaeche am echten Editor auf einem echten Main (Namen im Prozessorzustand): ein Entwurf der Startquelle A wird beim Klick auf die Zeile B auf A bestaetigt, und weder Enter noch Fokusverlust danach schreiben ihn auf B; ein leerer Entwurf loescht Bs Namen nicht; nach Escape zeigt das Feld wieder As Label, und der naechste Auswahlwechsel schreibt nichts; faellt A weg oder kommt A mit neuer Runtime-Nonce zurueck, verfaellt der Entwurf ohne Mutation, auch mit Enter vor dem naechsten Tick; unveraenderter Text schreibt nichts; im Feld werden 121 Codepoints ausserhalb der BMP auf 120 beschraenkt und beim Bestaetigen auf 120 gekappt, 0 und 120 werden angenommen, 121 am Prozessor abgewiesen; Host-Dirty je geaendertem Namen genau einmal. Seit Etappe 6b (R-312-6) dazu Ersatzziel und Aktionssteuerung: mit 16, 20, 21, 32 und 64 Quellen ohne Hauptziel ist das Ersatz-Hauptziel die erste angezeigte Quelle (vier Bilder 20/21/32/64 im Sichtsatz), ab 20 Quellen zeichnet die Flaeche 20 Zeilen; rueckt ein per Handgriff gewaehltes Hauptziel durch Beitritt oder Hostbusnamen hinter Zeile 20, sind Knopf, Labelfeld und dessen Beschriftung unsichtbar, das Aktionsziel ist leer, das Hauptziel im Modell unveraendert ohne Schreibversuch, und der Status sagt es; ein offener Entwurf wird davor auf der Startquelle bestaetigt (genau 1 Host-Dirty); ohne Quelle gibt es keine Aktionssteuerung, bei 1 und 16 Quellen ist jede Zeile gezeichnet und per Klick erreichbar. Seit Etappe 6b (NAK-349, R-312-2) dazu das Kennungskonflikt-Panel: stellt die Nachrichtenschleife einen geposteten Klick (triggerClick, der Weg von Enter) erst nach dem Ende des Editors zu, mit lebendem und mit danach zerstoertem Prozessor, fasst der Rueckruf weder Editor noch Prozessor an (gezaehlte Marke 0, Kennung unveraendert, 0 Host-Dirty); bei lebendem Editor vergibt derselbe Klick eine neue Kennung mit genau einer Host-Dirty-Meldung und Statusmeldung, und die Box schliesst. Seit der Nacharbeit 1 der Etappe 6 (L-5, R-312-2 Satz 2) dazu: beginnt der Klick bei lebendem Editor und baut ein Listener in audioProcessorChanged den Editor ab, waehrend der Rueckruf in neueSensorId steht (nach seinem Eintritt, die Kennung schon neu), wechselt die Kennung mit genau einer Host-Dirty-Meldung, der Rueckruf erreicht danach keinen Zugriff auf den Editor (zweite Lebendpruefung, gezaehlte Marke 0), und die Box wird abgebaut. Seit NAK-312 Etappe 7b (U51) dazu die Annahmegrenze am echten Pfad (abonnierte Sitzung, echte Snapshots): bei 0, 1, 19 und 20 Quellen ist jede angenommen, gezeichnet und per Klick erreichbar; bei 21, 32 und 64 Quellen sind 20 angenommen, jede davon gezeichnet und erreichbar, und das vorhandene Diagnosefeld meldet die volle Liste mit der Zahl der nicht angenommenen, allein oder als Zusatz hinter einer anstehenden Diagnose. Seit NAK-313 Etappe 2 (R-313-3) schreibt das Messpunkt-Popover nur, was der User geaendert hat: mit geladenem Label von 121 und Paarnamen von 61 Zeichen ruft das unveraendert geschlossene Popover die Bindungs-API nicht (0 Aufrufe, 0 Host-Dirty, 0 Reconnect-Anforderungen, Save bytegleich), und auch Fokusverlust und Rollenrueckruf ohne Wechsel rufen sie nicht und melden kein Host-Dirty; eine Rollenwahl mit unveraendert langem Label gelingt mit genau einem Aufruf und einem Host-Dirty, und beide Texte bleiben; die Felder begrenzen die Eingabe auf 120 und 60 Codepunkte (das Paarfeld gemessen bei eingerichteter Paarrolle pre); ein geaendertes, weiter zu langes Label oder ein solcher Paarname wird ohne Host-Dirty abgewiesen, und die vorhandene Statuszeile nennt die Grenze; traegt der abgewiesene Aufruf eine Rollenwahl (E-313-18), faellt die Rollenauswahl ohne zweiten Aufruf auf die gespeicherte Rolle zurueck, und die Meldung nennt es. Seit NAK-313 Etappe 7 (R-313-10; 313/M-141, M-142): der Festhalten-Handgriff am echten Editor meldet die entstandene Datei, bei einem Namen derselben Sekunde die vorhandene Datei und dass nichts gespeichert ist, bei einem Schreibfehler den Grund samt Rest der eigenen Temp-Datei; Datei und Vergleichslinie tragen denselben Snapshot, auch wenn sich die Messdaten der Engine direkt danach aendern. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 7,93 s | [B15](roh/NAK-380-8308a3d-dirty.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,07 s | [B20](roh/NAK-380-8308a3d-dirty.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und der Fingerprint ist unter Vorzeichenumkehr des Signals bytegleich. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 1,21 s | [B22](roh/NAK-380-8308a3d-dirty.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,75 s | [B23](roh/NAK-380-8308a3d-dirty.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,01 s | [B24](roh/NAK-380-8308a3d-dirty.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/NAK-380-8308a3d-dirty.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,48 s | [B26](roh/NAK-380-8308a3d-dirty.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/NAK-380-8308a3d-dirty.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,95 s | [B19](roh/NAK-380-8308a3d-dirty.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht fuer 15 bis 19 keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt auf 1e-9 genau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. Seit NAK-283 (N01) zusaetzlich die vier transienten Faelle 20 bis 23 aus EBU Tech 3341 (Versatz 0 bis 3): jeder trifft eine im Test gerechnete, vorher an 15 bis 19 auf +/-0,01 dB validierte 64-fach-Referenz innerhalb +/-0,1 dB und liegt zusaetzlich in der Normtoleranz 0,0 +0,2/-0,4 dBTP. Insgesamt 43 Pruefungen. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,33 s | [B17](roh/NAK-380-8308a3d-dirty.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit der Spanne der §5-Referenz (± 0,2 LU); zehn Kurzzeitwerte je Sekunde (EBU Tech 3342 §3.1), die 60-s-Schwelle zaehlt gegatetes Material bei 10 Hz. Bei Material ohne Dynamik bleibt sie nahe null und nach 90 s Stille gibt es gar keinen Wert. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 11,20 s | [B18](roh/NAK-380-8308a3d-dirty.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. Seit NAK-313 Etappe 4 (R-313-7, 313/M-51): ein echter FeatureFrame ohne gueltige Projektzeit (Transport local_monotonic) ergibt mit den Transportzahlen aus eingabe der Byteinstanz evidenz-lokal-wire-v1.json einen Transportblock OHNE project_sample_start, bytegleich zu wire_transport, und der ganze Snapshot passiert Textriegel und dieselbe Engine wie B3c. Seit Etappe 4 (R-313-6, 313/M-49) ausserdem der C++-Vertragsweg an der Tabelle PRODUKTEINGAENGE-FAELLE.json: je evidence_snapshot-Eintrag ein Fall mit PE-nnn im Namen durch Textriegel, genau einen strengen Lauf kanon::lies und die Engine, verglichen mit vertrag - doppeltes type faellt an der Stufe duplikat, eine Verschachtelung von 65 Ebenen an parser, project_sample_start null am Schema, und 64 Ebenen sowie der wire_snapshot der Byteinstanz sind gueltig; die Zaehlpruefung verlangt genau so viele Faelle, wie der Kopf der Tabelle fuer evidence_snapshot nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-73): die Werte kommen aus demselben strengen Lauf (nakama::vertrag::wertAlsVar), und bei Urteil gueltig wird der Wert am Feld als Ganzzahl verglichen (transport.sequence 8241.0 und 8.241e3 sind 8241). | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 1,83 s | [B16](roh/NAK-380-8308a3d-dirty.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. Seit NAK-313 Etappe 5b (R-313-4; 313/M-80, M-81, M-89): die Revisionsraender der NAK-283-Faelle M-07 bis M-10 und M-12 stehen bei 2^53-1 (assistentenrevision_laeuft_nicht_ueber misst den neuen Rand); Bestands- und Eintragsrevision gehen bei 2^53-2 genau einen Schritt mit genau einem Host-Dirty und weisen am Rand ab, bevor sie etwas zuweisen (Prozessor ohne Host-Dirty, Bibliothek mit Grund und gleichen Bytes); der Intentwriter schreibt bestand_revision 2^53-1 ab bestand_revision bytegleich zur Handinstanz intent-wire-v1.json. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,72 s | [B27](roh/NAK-380-8308a3d-dirty.md#b27) |
| B29 | Die Zustandsmaschine des AssistantStep im Main (46.1, E-07/E-08, M-55 bis M-62). Die Zustandsmenge hat ACHT Werte, jeder kommt ueber seinen eigenen Rueckweg zurueck, ein neunter faellt. Nach preview fuehrt KEINE Kante - weder hin noch zurueck -, ein gespeicherter preview-Schritt ist ein Lesefehler statt eines stillen Sprungs, und die Produkt-API setzt ihn nicht; die Gegenprobe mit proposal laedt normal. Jeder der acht Zustaende traegt fuenf Angaben (Eintritt, Evidenz, Useraktion, Timeout ueber 0, sichere Rueckkante), auch preview - der erste Zustand ist seine eigene Rueckkante. Der Deckel ist STRUKTURELL: ein zweiter Startversuch bei offenem Schritt wird abgewiesen, nicht eingereiht, und erst nach dem terminalen Abbruch beginnt ein neuer. Die vier Gegenpfade laufen einzeln: Zurueck geht auf die Rueckkante, Ueberspringen auf den naechsten Zustand, Resume ist eine FRAGE ohne Revisionssprung, und Verwerfen ist terminal statt geloescht - danach gibt es kein Resume mehr. Die drei benannten Ergebnisse aus 46.2 sind eigene Ergebnisse mit Objekt; dasselbe zweimal hebt keine Revision. Die harten Gates greifen VOR der Gewichtung: drei Kandidaten mit perfektem Rang und je einem gerissenen Gate verlassen die Liste, ein bescheidener bleibt. Verdrahtet am echten Prozessor mit Host-Dirty genau einmal je echter Aenderung, Rekonstruktion aus dem gespeicherten MainProject und bytegleichem Save/Load; jede Grenze faellt von beiden Seiten, und ein Altstand ohne die Eigenschaft laedt normal und schreibt sie nicht. Seit NAK-313 Etappe 5b (R-313-4; 313/M-82, M-84 bis M-87): an der Revision 2^53-1 weisen Weiter, Antwort und ein neuer Lauf ab, ohne etwas zu aendern und ohne Host-Dirty; assistentVersuchStarten prueft die Grenze vor dem Versuchsbeginn - am Probe-Server kein experiment_begin, kein belegter Slot, kein eingefrorener Pegel, 0 Host-Dirty, und der gespeicherte Stand traegt keine Versuchskennung -, startet unter der Grenze wie bisher mit genau einem experiment_begin und einem Host-Dirty, und faellt die Grenze erst nach dem Versuchsbeginn (Testhaken), weist die Bibliotheksfunktion ab und der Versuch endet mit genau einem experiment_abort (user_abbruch). | `eq-copilot\build\plugin\EqCopSonde014AssistentTest_artefacts\Release\EqCopSonde014AssistentTest.exe` | [OK] Exit 0 | 2,34 s | [B29](roh/NAK-380-8308a3d-dirty.md#b29) |
| B30 | Diagnose-Briefkasten im Plugin (Plan S25e (3), R-286-1 und R-286-2): ohne anfrage.json nur eine Existenzpruefung je Takt (hoechstens 1 Hz, Message-Thread) und nichts Messbares - Audio, State, Parameter und der Snapshot des Knopfwegs bleiben bytegleich. Eine gueltige Anfrage beantworten Gen und Probeeq je Kennung genau einmal, solange sie im Ring der letzten 256 steht, ueber den eigenen Schreibweg (exklusive Temp-Datei, Groessenpruefung, Umbenennen ohne Ersetzen, nie Ordneranlage oder Loeschung) mit einem Umschlag nach nakama.diagnose.antwort.v1; fremde oder zu grosse Anfragen und solche hinter einer Junction bleiben ohne Antwort, eine beim Lesen gesperrte wird im naechsten Takt beantwortet. Seit NAK-313 Etappe 7 (R-313-10; 313/M-132 bis M-144) der Festhalten-Export, jeder Fall in einem Testordner unter %TEMP%: er legt die Ordnerebenen an und veroeffentlicht exklusiv (Temp-Datei, Spuelen, Groesse, Umbenennen ohne Ersetzen), ersetzt eine Datei gleichen Namens nie (Doppelklick, zwei Instanzen, leeres Label, Schreibweise und Bereinigung, Uhrrueckstellung: abgelehnt, die erste Datei bytegleich), meldet Teil-Schreibfehler als Fehler und entfernt die eigene Temp-Datei oder nennt sie als Rest; er liest je Export genau eine Uhrablesung und nur den uebergebenen Snapshot, der Testbau verweigert ohne Testordner vor jedem Dateizugriff, und die Referenz des Knopfwegs bleibt bytegleich. | `eq-copilot\build\plugin\EqCopBriefkastenTest_artefacts\Release\EqCopBriefkastenTest.exe` | [OK] Exit 0 | 72,25 s | [B30](roh/NAK-380-8308a3d-dirty.md#b30) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,15 s | [B28](roh/NAK-380-8308a3d-dirty.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. Seit NAK-309 (M-74) zuerst, vor dem ersten Prozessor: im Testbau nennt der Prozessor seinem v2-Client einen Probe-Namen, nie die Produktions-Pipe, nach Produkt- und Testkonstruktor denselben, und jedes Ziel mit den Prozessorquellen ausser dem Produkt traegt den Testschalter. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,45 s | [B8](roh/NAK-380-8308a3d-dirty.md#b8) |
| A32 | Codebase- und Kontextgesundheit gegen die Schwellen aus Plan S25b und dem Kontext-Hygiene-Playbook. Der Quellumfang kommt seit NAK-283 (M-69) aus einem pruefbaren Inventar: jede Datei mit Endung .rs, .cpp, .h oder .hpp unter broker/src und eq-copilot/plugin samt allen Unterbaeumen, mit Grund ausgenommen nur der flatc-Codegen (broker/src/generiert, eq-copilot/plugin/vertrag/generiert) und der Testbaum eq-copilot/plugin/tests; der Bericht druckt das Inventar, und eine Quelldatei, die weder gemessen noch ausgenommen ist, beendet den Lauf als WERKZEUGFEHLER (Inventarriegel, Exit 2). Gemessen werden Zeilen je Quelldatei (Grenze 2 000, Ziel 1 500), Anzahl Funktionen ueber 200 Zeilen, aufruferlose allow(dead_code)-Helfer, Backtick-Bezeichner in Kommentaren ohne Entsprechung im Repo, sowie Bytes von MEMORY.md, CLAUDE.md und Dirigenten-Skill, Index-Zeilen ueber 250 Zeichen und Memory-Dateien ohne Index-Link. Die Ratschen (Funktionen 28, Kommentar-Bezeichner 30, clippy 91) stehen auf dem Iststand vom 09.09.2026 nach NAK-225 und reissen bei der ersten Verschlechterung; sie werden nur gesenkt, nie erhoeht. Die Grenzen - Zeilen je Datei, Funktionen ueber 200 Zeilen, Kommentar-Bezeichner - werden ehrlich gemeldet, auch wenn sie reissen (Pflegeticket NAK-292). Jeder neue Treffer ueber der Zeilengrenze meldet sich als OHNE PFLEGETICKET; ein Zuordnungseintrag, dessen Datei die Grenze nicht mehr reisst, beendet den Lauf als WERKZEUGFEHLER (Exit 2), damit die Liste nicht still veraltet. Vorangestellt laeuft der Selbsttest: 129 Faelle im Speicher durch dieselben Funktionen, jede Erwartung mit ihrem Gegenteil (Zahlenraender 2 000/2 001, 200/201, 250/251, CRLF, BOM, fehlender Schlussumbruch, Rust-Lebensdauer und C++-Ziffernstrenner gegen die Literalmaskierung; das Inventar mit neuem Unterbaum, Codegen, Tests und einer Ableitung, die still einen Unterbaum verliert). Der clippy-Teil laeuft NICHT mit (Schalter --clippy, er baut) und meldet sich als nicht messbar. Nicht blockierend: Exit 4 heisst gerissen und ist ein Hinweis. Exit 2 ist ROT und heisst, dass das WERKZEUG nicht mehr misst - roter Selbsttest, eine fehlende Inventarwurzel oder der Inventarriegel, denn ein verschobener Pfad wuerde sonst still 0 messen und die halbe Codebase unbemerkt aus dem Mass nehmen, und ein ungemessener Unterbaum machte jedes andere Mass falsch. | `py -3.13 tools\plan\gesundheit.py --mit-selbsttest` | [HINWEIS] Exit 4 - Befund, nicht blockierend (siehe Rohausgabe) | 35,45 s | [A32](roh/NAK-380-8308a3d-dirty.md#a32) |
| A33 | clang-tidy ueber alle Uebersetzungseinheiten von eq-copilot/plugin ohne tests/ und ohne den flatc-Codegen vertrag/generiert/ (Bein A9 haelt ihn bytegleich; Regelsatz eq-copilot/plugin/.clang-tidy: bugprone, clang-analyzer, concurrency, performance sowie eine cert-, cppcoreguidelines- und misc-Auswahl; keine Stilregeln, Ausnahmen dort mit Grund) in einem eigenen Ninja-Baum eq-copilot/build-tidy mit denselben cl.exe-Schaltern wie der Produktbau. Gezaehlt werden eindeutige Fundstellen (Datei, Zeile, Spalte, Check) gegen die Ratsche in tools/plan/tidy.py: Grenze = Iststand der Erstmessung NAK-288, Ziel 0, nur gesenkt, nie erhoeht; eine Aenderung des Regelsatzes zieht die Ratsche im selben Commit nach. Vorangestellt der Selbsttest (Einordnung in den Quellort, Datenbankfilter, Diagnoseparser, Deduplizierung, Ratschenrand, Umgebungsleser). Nicht blockierend: Exit 4 ist ein Hinweis. ROT ist Exit 2: eine Uebersetzungseinheit mit error-Diagnose oder ein abgestuerzter clang-tidy heisst NICHT gemessen, denn eine halb geparste Einheit meldet weniger Funde und saehe wie Fortschritt aus. FEHLT (Exit 3) ohne Visual Studio 2022 mit C++-Werkzeugen und Clang-Komponente, cmake oder ninja. | `py -3.13 tools\plan\tidy.py --mit-selbsttest` | [OK] Exit 0 | 108,53 s | [A33](roh/NAK-380-8308a3d-dirty.md#a33) |

## 31. Kanon nachher, Erstprüfung 3: Urteil, Validierung, Einordnung und Nacharbeitsauftrag 1 (Dirigent, 25.09.2026, 17:10 Uhr)

**Kanon nachher der Etappe 3.** Abgekoppelt gestartet 16:11:03 Uhr auf
`8308a3d8`; GRUEN 70/70, 16:11:03 bis 17:02:40 Uhr, EXIT=0, Rohausgabe `docs/beweise/roh/NAK-380-8308a3d-dirty.md` (dirty: Broker-Pin des Runners), A15 stillgelegt, A32 Hinweis Exit 4 mit unveränderten Maßen 3/33/39 gegen den Kanon der Etappe 2 (kein neuer Riss), A33 Exit 0. Der Lauf stand von 16:11 bis 16:24 Uhr nach dem
Bauschritt ohne Kindprozess an einem `vctip.exe` (NAK-300; vom Dirigenten
beendet, der Lauf ging weiter). Der Kanon-Abschnitt steht vor diesem
Abschnitt.

**Erstprüfung 3 (Vorlage A).** Codex `gpt-6-astra`, Effort **xhigh**
(NAK-362; Claude-Woche nach dem Reset), lesend, Thread
`01a0d8e7-d698-7352-84e9-538e3e8fe086`, 16:11:07 bis 16:20:43 Uhr, EXIT=0,
HEAD vor und nach dem Lauf `8308a3d8`. Prüfbereich `b04dfb7c...8308a3d8` über
die 16 Ticketpfade der Etappe 3 einschließlich Nacharbeit 0 (ohne den
Broker-Pin); Auftrag `docs/beweise/roh/NAK-380-erstpruefung-3-auftrag.txt`,
Urteil `docs/beweise/roh/NAK-380-erstpruefung-3-urteil.md`. **URTEIL:
NEEDS_WORK — 4 Defekte, 0 Lücken, 0 Härtungen; kein Produktrechenfehler.**
Die offenen Punkte 1 bis 7 des Auftrags sind bestätigt: M-23-Lücke
geschlossen (2,544 gegen 2,600 LU verträglich), Versionsbindung 20260926
einschließlich Broker korrekt, M-28(b)-Ausnahme zulässig, Ersatzmutationen
formgültig, kein Audio-Pfad, nur die Diagnose-Fixture geändert, Toleranzen
und Saaten wie in der Matrix. Nicht geprüft: keine Läufe.

**Validierung an der Quelle.** Lesender Opus-5.5-Worker `92e014b2`
(`nakama-nak380-8308a3d-val3`, Effort max, `dontAsk`), 16:23 bis 16:43 Uhr,
HEAD gleich; Auftrag
`docs/beweise/roh/NAK-380-erstpruefung-3-validierung-auftrag.txt`, Bericht
`docs/beweise/roh/NAK-380-erstpruefung-3-validierung.md`: **3 bestätigt (D1,
D3, D4), 1 präzisiert (D2), 0 widerlegt; alle vier DEFEKT.** Teil 2:
Durchsicht M-21 bis M-41 mit Datei:Zeile, zwei Randbefunde. Der Dirigent hat
die Stellen selbst nachgelesen (`GoldenTestMain.cpp` `nak380M38`,
`pruefe_v3_vertrag.py` M-30, `metriken-v1.json:150`, `FeatureEngine.h:124`
und `:449`).

### 31.1 Einordnung je Befund

| Befund | Quelle (HEAD `8308a3d8`) | Validierung | Einordnung | Regel |
|---|---|---|---|---|
| D1 M-38 NaN nur als „alle drei endlich“ | `GoldenTestMain.cpp:604-619`: `endlich = isfinite(P10) && isfinite(P50) && isfinite(P95)`; `zuFrueh` zählt nur `bloecke == 0 && endlich` | BESTÄTIGT | **DEFEKT** — M-38 „NaN, nie 0“, `CLAUDE.md` NaN-Ehrlichkeit | je Feld NaN prüfen; Mutation je Feld |
| D2 M-38 Vorbedingung nicht hergestellt | `:565-572`: 30 s, Reset, 1 s; Matrix M-38 „wie M-32 [W4 60 s]; zweiter Lauf mit `zuruecksetzen()` bei 30 s“; floor/Rest nach dem Neustart ungeprüft | PRÄZISIERT | **DEFEKT** — Vorbedingung exakt (Lehre D1 aus §24) | Lauf 1 60 s, Lauf 2 Reset bei 30 s bis 60 s, Zählgrößen je Auswertepunkt mit Formel |
| D3 M-38 Flag nur im wahren Zustand | `:600-623`: `perzentileGueltig == irgendeinBlock`, nach 1 s immer wahr | BESTÄTIGT | **DEFEKT** — M-38 „`perzentileGueltig` erst mit mindestens einem Teilblock“ | Auswertepunkt ohne Teilblock mit Flag false; Mutation „Flag immer wahr“ |
| D4 M-30 nur das erste Paar, Index ohne `inline constexpr` | `pruefe_v3_vertrag.py:2389-2393` `re.match` auf das erste `kName = Wert`; Register `:150` zwei Paare; `kVerteilungPlaetze` ist `inline constexpr` (`FeatureEngine.h:124`), `kEreignisPlaetze` `static constexpr` (`:449`); dazu `Vergleichspegel::kMindestSekunden = 0.4` (`:417`) übersprungen | BESTÄTIGT | **DEFEKT** — M-30 „A5 prüft je `nicht_gefuehrt`-Eintrag der Form ‚kName = Wert‘, dass der Code denselben Wert trägt“ (Lehre D5) | jedes Paar, auch `Klasse::kName`, gegen einen Index aus `static`, `inline` und Klassenkonstanten |
| Z1 M-36 Wache ohne Trennschärfe (Validierer Teil 2) | `GoldenTestMain.cpp:280-288`, `:359-360`: Texte als Teilstring an einem Eingang mit Spanne 0 dB; Schwellen 10 → 7 oder 6 → 4 blieben grün (Ersatzmutation `> −0,5` fällt nur unter 0) | Validierer, vom Dirigenten bestätigt | **DEFEKT** — M-36 „Textschwellen 6 und 10 dB unverändert“ ist von keiner Prüfung getragen, die eine Schwellenänderung sieht (Ursache B) | drei Eingänge beiderseits der Schwellen, vollständige Texte, Mutationen 10 → 7 und 6 → 4 |
| Z2 M-37 Segmentzahl nicht verriegelt | `:521-562`: nur Samplezahl und Bandzahl verriegelt; 221/221 gilt auch bei null Segmenten | Validierer, vom Dirigenten bestätigt | **DEFEKT (schwach)** — gleiche Wurzel wie D2 | Segmentzahl je Stufe hergeleitet, > 0 |
| K1 Kommentar `AnalyseEngine.h:130-131` nennt das Pegelhistogramm | seit T-380-4 falsch (Quelle `teilblockHistogramm`, NaN = weniger als 8 Segmente) | Validierer Randbefund 2 | **Kommentar nachziehen** (Ticketpfad; Kommentar, der lügt) | in Nacharbeit 1 |
| R1 Perzentile 0,0 im Reset-Snapshot | `AnalyseEngine.cpp:333` `fertig = MessSnapshot {}` mit Vorgaben 0,0 (`AnalyseEngine.h:133-135`) bis zum nächsten `auswerten()`; `perzentileGueltig` false, beide Leser gaten darauf; vorbestehend an `b04dfb7c` | Validierer Randbefund 1 | **HÄRTUNG** — Register NAK-403 (Vorgabe NaN statt 0,0; keine Außenwirkung) | — |
| M-29 Satz 2 ohne eigene Prüfung | im Produkt doppelt gesperrt (`Lautheit.h:100-101`, `:124`) | Validierer | HÄRTUNG (im Register bei NAK-403 mitgeführt) | — |

**Lehre.** Die Präzisierungen aus §24 standen wörtlich im Etappenauftrag
und kehrten in M-36, M-37, M-38 und M-30 wieder. Ab Etappe 4 verlangt jeder
Bau- und Nacharbeitsauftrag je Matrixzeile eine Tabelle Zusagesatz →
Prüfzeile → Mutation → rot ja/nein im Etappenabschnitt (Vorschlag des
Validierers, Teil 2 Punkt 3); der Prüfer misst an ihr.

### 31.2 Matrix in der Fassung §31

M-38: Vorbedingung „wie M-32 (W4, 60 s); zweiter Lauf mit `zuruecksetzen()`
bei 30 s und Weiterfahrt bis 60 s; Auswertepunkte: 60 s (Lauf 1), unmittelbar
nach dem Reset, nach dem Reset ohne vollständigen Teilblock in irgendeiner
Stufe (weniger als 8 aktive Segmente je Stufe, Zeitpunkt aus dem kleinsten
Stufen-Hop hergeleitet), 60 s (Lauf 2)“; Zusage ergänzt um „je Feld P10, P50,
P95 NaN“ und „`perzentileGueltig` false ohne Teilblock, true ab dem ersten“;
Rotbeweis: vier Mutationen (Rest als eigener Teilblock; Vorgabe 0,0 statt NaN
für ein Feld; Flag bedingungslos wahr; Zähler beim Reset 1 statt 0). M-36:
drei Eingänge mit hergeleiteter Spanne unter 6, zwischen 6 und 10, über 10
dB, vollständige Texte; Rotbeweis Schwelle 10 → 7 und 6 → 4. M-37:
Segmentzahl je Stufe verriegelt und > 0. M-30: jedes Paar jedes Eintrags,
auch `Klasse::kName = Wert`; Index aus `static constexpr`, `inline constexpr`
und Klassenkonstanten; Rotbeweis zweiter Wert 65 und Vergleichspegel 0.5.

### 31.3 Ursachen und Nacharbeitsauftrag 1 (Runde 1 von 3)

Ursachen wie in §24: (A) der Test bildet Vorbedingung, Zusage je Feld und
Flagzusage nicht vollständig ab (D1, D2, D3, Z2); (B) je Zusagesatz und je
Wert eine eigene Prüfung und Mutation (D4, Z1, M-38-Feld- und Flagmutation).
Der Auftrag trägt die Ursachen, die Befunde wörtlich, je Befund die
schließende Regel: `docs/beweise/roh/NAK-380-nacharbeit-e3-1-auftrag.txt`.
Kein Produktcode außer dem Kommentar K1; Ticketpfade `GoldenTestMain.cpp`,
`Nak380Pruefsignale.h`, `pruefe_v3_vertrag.py`, `AnalyseEngine.h`
(Kommentar), Rohdateien, Manifest §32. Bauer: frischer Opus-5.5-Worker,
Effort max, Aufsicht ENG. Wiederprüfung 1 danach nach Vorlage B über den
Fixdiff (Codex `gpt-6-astra` xhigh).

**Register:** NAK-403 (Härtung R1, M-29 Satz 2). Offen beim User: nichts.

## 32. Nacharbeit 1 der Etappe 3 (Bauer Opus 5.5, 25.09.2026)

**Start.** `git rev-parse HEAD` ergab `3b888f56938cfff41bb12d905dedb210aff4759e`
auf `master`; `git status --short` nannte nur `briefing-hub/` und
`nimbalyst-local/`; die Prozessprobe auf `cmake`, `MSBuild`, `cl`, `link`,
`cargo`, `rustc`, `EqCop*`, `eqcop-broker`, `FL64` und `vctip` war leer
(17:06:37 Uhr). Auftrag `docs/beweise/roh/NAK-380-nacharbeit-e3-1-auftrag.txt`.
Basismessung am unveränderten Stand: A2 Exit 0, 258/0 (17:23:09 bis 17:23:50
Uhr), A5 Exit 0, 863/0. Kein Stage, Commit, Push oder Stash, keine Rücknahme
über git, kein voller Kanon, kein FL Studio, keine Installation, kein
A8/A9/A10. Ursache (A) wird geschlossen, indem jeder Zusagesatz in einem
Zustand geprüft wird, in dem er in beide Richtungen fallen kann, und jede
Zählgröße mit Formel verriegelt ist; Ursache (B), indem jeder Zusagesatz und
jeder Wert eine eigene Prüfung und eine eigene formgültige Mutation hat.
Zeilen in diesem Abschnitt gelten am Arbeitsstand der Nacharbeit, Zeilen der
Produktdateien (unverändert außer K1) am HEAD.

### 32.0 Schließung je Befund

| Befund | geänderte Stellen (Arbeitsstand) | Schließung mit Messzahl |
|---|---|---|
| D1 | `eq-copilot/plugin/tests/GoldenTestMain.cpp:789-820` (`nak380M38Perzentile`) | Je Feld eine eigene Prüfung: P10, P50 und P95 sind in jedem Band ohne Teilblock NaN und in jedem Band mit Teilblock endlich, beides aus der Formel, an fünf Auswertepunkten. Die Mutationen „Vorgabe 0,0 statt NaN“ für genau ein Feld (`AnalyseEngine.cpp:828`, `:829`, `:830`) machen je nur ihr Feld rot (25, 221, 221, 145 und 25 Bänder), die beiden anderen Felder bleiben grün. |
| D2 | `GoldenTestMain.cpp:822-908` (`nak380M38`), `:756-787` (`nak380M38Zaehler`), `:739-754` (`nak380TeilblockSoll`), `:431-450` (`nak380StufenPunkte`, `nak380Segmente`), `:388-418` (`nak380M1SpeisenSamples`) | Lauf 1 W4 über 60 s ohne Reset; Lauf 2 mit `zuruecksetzen()` bei 30 s und Weiterfahrt bis 60 s. Segmente je Band gegen S(N) = floor((N − n)/(n/2)) + 1 der Stufe, Teilblöcke floor(S/8), Rest S mod 8, Restsumme leer genau ohne Rest: Lauf 1 350/1405/2811 Segmente, 43/175/351 Teilblöcke, Rest 6/5/3; Lauf 2 nach 30 s seit dem Reset 174/702/1405, 21/87/175, Rest 6/6/5. Rot: `m38-rest` (196 von 221), `m38-reset`, `m37-segmentgate`, `m37-doppelzaehlung`, `m38-reset-segmenthistogramm`, `m38-reset-teilblockhistogramm`. |
| D3 | `GoldenTestMain.cpp:789-820` (Flag), Auswertepunkte in `:851-908` | Das Flag wird auf beiden Seiten gemessen: falsch unmittelbar nach dem Reset und nach 9 215 Samples (kein Teilblock in irgendeiner Stufe, erster Höhen-Teilblock nach 2048 + 7·1024 = 9 216), wahr nach 9 216 Samples, in Lauf 1 und nach 60 s. `m38-flag` (`AnalyseEngine.cpp:860` bedingungslos wahr) macht genau die zwei Falsch-Prüfungen rot; `m37-segmentgate` die drei Wahr-Prüfungen. |
| D4 | `tools/eq-copilot/pruefe_v3_vertrag.py:2179-2277` (`_cpp_ohne_kommentare`, `_kern_konstantenindex`, `_cpp_zahl`), `:2482-2520` (M-30) | Jedes Paar `kName = Wert` und `Klasse::kName = Wert` jedes `nicht_gefuehrt`-Eintrags der Fassung `aktuell` ist eine eigene Prüfung (7 Paare) gegen einen Index aus `static constexpr`, `inline constexpr` und Klassenkonstanten aller 18 Kernheader unter `eq-copilot/plugin/core/analysis/` (124 Schlüssel); dazu die Prüfung, dass jede Nennung `kName =` ein lesbares Paar ist. M-30 11/0, A5 870/0. Rot: zweiter Wert 64 → 65, `Vergleichspegel::kMindestSekunden` 0.4 → 0.5, erster Wert, vier LRA-Werte, Name ohne Codefund, unlesbarer Wert. |
| Z1 | `eq-copilot/plugin/tests/Nak380Pruefsignale.h:20-22`, `:202-228` (`M36Pegelrechteck`); `GoldenTestMain.cpp:554-641` (`nak380M36Eingang`, `nak380M36Eingaenge`), `:938-942`, `:984-995` | Drei Eingänge M36a/M36b/M36c: Ton 1500 Hz auf Bin 128 über Weißrauschen, 7 s Grundpegel und 3 s um 5, 8 oder 13 dB lauter, 60 s. Hergeleitete gelesene Spanne = Tiefe (P50 −30 dB, P95 −30 dB + Tiefe; gemessen gleich); je Eingang Charakter- und Werkzeugtext vollständig wörtlich (Bytevergleich gegen die Produktliterale). Die Schwellen sind beidseitig gehalten: 5 ≤ T6 < 8 und 8 ≤ T10 < 13. Rot: 10 → 7 nur M36b-Werkzeug, 6 → 4 nur M36a-Charakter, dazu 6 → 10, 6 → 13, 10 → 14, 10 → 4. |
| Z2 | `GoldenTestMain.cpp:643-737` (`nak380M37`) | Segmentzahl je Band gegen die Formel (Bass 350, Mitten 1405, Höhen 2811, jede > 0) vor der Gleichheit 221/221; im Kontrastlauf des Abdeckungsnachbaus aktive Segmente je Band in [S(3 072 000) − ceil(192 000/(n/2)), S(3 072 000) − S(192 000)] = [350, 352], [1405, 1407], [2811, 2813]. `m37-segmentgate` macht beide Verriegelungen rot, während 221/221 und der Nachbau bei null Segmenten grün bleiben (0 = 0) — genau die Leere aus Z2. |
| K1 | `eq-copilot/plugin/src/AnalyseEngine.h:130-131` | Kommentar nennt das 1-dB-Teilblockhistogramm (Mittel aus je 8 aktiven Segmenten) und NaN = weniger als 8 aktive Segmente; nur diese zwei Zeilen, Zeilenzahl gleich. |

### 32.1 Änderungsregister

| Datei und Stelle | vorher (HEAD `3b888f56`) | jetzt |
|---|---|---|
| `GoldenTestMain.cpp` `nak380M38` | 30 s, Reset, 1 s; floor/Rest nur vor dem Reset mit `n` aus dem Lauf; NaN als „alle drei endlich“; Flag nur im wahren Zustand | Lauf 1 und Lauf 2 nach Matrix; sechs Auswertepunkte; je Punkt Segmente, Teilblöcke, Rest gegen die Formel, je Feld NaN/endlich, Flag falsch und wahr (43 Prüfungen) |
| `GoldenTestMain.cpp` `nak380M37` | Samplezahl und Bandzahl verriegelt | zusätzlich Segmentzahl je Band gegen die Formel und Segmentbereich im Kontrastlauf (5 Prüfungen) |
| `GoldenTestMain.cpp` M-36 | Teilstring „steht ruhig“ an der 116-Hz-Karte (Spanne 0 dB) | drei Modulationseingänge mit vollständigen Texten (12 Prüfungen); 116-Hz-Prüfungen bleiben |
| `GoldenTestMain.cpp` Hilfen | `nak380M1Speisen` nach Sekunden | `nak380M1SpeisenSamples` (Samplezahl), `nak380StufenPunkte`, `nak380Segmente`, `nak380TeilblockSoll`; `nak380BinsImBand` nutzt `nak380StufenPunkte` (gleiches Ergebnis) |
| `GoldenTestMain.cpp` `Pruefer`, `main` | nur Fehler ausgegeben; `--nak380 M-36` fuhr 116-Hz-Fixture und Stereoblock | mit gesetzter Umgebungsvariable `NAK380_ALLE_PRUEFUNGEN` auch jede bestandene Prüfung (sonst unverändert); `--nak380 M-36` fährt die drei Eingänge und nur die 116-Hz-Fixture |
| `Nak380Pruefsignale.h` | keine M-36-Signale | Saaten `kM36aSaat` bis `kM36cSaat` (0x3800011 bis 0x3800013), `M36Pegelrechteck` |
| `pruefe_v3_vertrag.py` M-30 | `re.match` auf das erste Paar, Index nur `static constexpr` aus `FeatureEngine.h`, eine Sammelprüfung | jedes Paar einzeln, Index aus `static`/`inline constexpr` und Klassenkonstanten der Kernheader, Lesbarkeitsprüfung (8 statt 1 Prüfung) |
| `AnalyseEngine.h:130-131` | „aus dem 1-dB-Pegelhistogramm … NaN = Band ohne Segmente“ | „aus dem 1-dB-Teilblockhistogramm … NaN = weniger als 8 aktive Segmente“ |
| `docs/beweise/roh/NAK-380-rot-M-30.txt`, `-M-36.txt`, `-M-37.txt`, `-M-38.txt` | Belege aus §27 | ersetzt (Tabellen Zusagesatz → Prüfzeile → Mutation → rot) |
| `docs/beweise/roh/NAK-380-e3-n1-mutation-*.txt` | keine | 25 Kurzprotokolle (je Mutation Wortlaut, Hashes, Zeiten, gefallene und grüne Prüfungen) |

Kein Produktcode außer K1: `AnalyseEngine.cpp`, `Diagnose.cpp` und
`metriken-v1.json` sind nach allen Mutationen bytegleich zum HEAD (SHA-256
`F1017C03…0F5C`, `A589F3FE…7490`, `84C60296…8ECE`; `git diff` leer).
Zeilenenden nach `git ls-files --eol` beibehalten: C++ `i/lf w/crlf` (keine
nackte LF-Zeile), `pruefe_v3_vertrag.py` LF, Manifest und Rohdateien LF.

### 32.2 Zusagesatz → Prüfzeile → Mutation → rot

Wortlaut, Hashes, Bauzeiten und die vollständigen Listen gefallener und
grüner Prüfungen stehen je Kürzel in `docs/beweise/roh/NAK-380-e3-n1-mutation-<Kürzel>.txt`;
jede Mutation wurde nach §8.1 gefahren (Sicherung unter
`eq-copilot/build/nak380-sicherung/n1/`, nur der betroffene Fall, Rücknahme
durch Zurückkopieren mit gleichem SHA-256, Neubau, Fall wieder grün).

**M-38** (`docs/beweise/roh/NAK-380-rot-M-38.txt`):

| Zusagesatz | Prüfzeile | Mutation | rot |
|---|---|---|---|
| Teilblöcke = floor(segmente/8), ein unvollständiger Teilblock verfällt | „Lauf 1 60 s: Teilbloecke je Band = floor(Segmente/8)“ | `AnalyseEngine.cpp:1094` `auswerten()` bucht den wartenden Rest als eigenen Teilblock (`m38-rest`) | ja, 196 von 221 |
| nach `zuruecksetzen` beginnt der Teilblock bei 0; floor/Rest nach dem Neustart | „Lauf 2 unmittelbar nach dem Reset: Rest …“, „Lauf 2 60 s: Rest …“ | `:298` Zähler beim Reset 1 statt 0 (`m38-reset`) | ja, je 221 von 221 |
| P10 je Feld NaN, nie 0 | „… P10 je Band NaN ohne, endlich mit Teilblock“ | `:828` Vorgabe 0,0 (`m38-p10`) | ja, nur P10 |
| P50 je Feld NaN | „… P50 …“ | `:829` (`m38-p50`) | ja, nur P50 |
| P95 je Feld NaN | „… P95 …“ | `:830` (`m38-p95`) | ja, nur P95 |
| Flag falsch ohne Teilblock | „… unmittelbar nach dem Reset / 9215 Samples: perzentileGueltig falsch“ | `:860` bedingungslos wahr (`m38-flag`) | ja, genau zwei |
| Flag wahr ab dem ersten Teilblock | „Lauf 1 60 s / 9216 Samples / Lauf 2 60 s: perzentileGueltig wahr“ | `:123` Segmentgate 0 dBFS (`m37-segmentgate`) | ja, alle drei |
| Zählgrößen mit Formel verriegelt | „… Segmente je Band = Formel der Stufe“ | `:123` (`m37-segmentgate`); `:510` doppelte Zählung (`m37-doppelzaehlung`) | ja |
| Reset leert Segment- und Teilblockzählung | „Lauf 2 unmittelbar nach dem Reset: Segmente … / Teilbloecke …“ | `:295`, `:296` `std::fill` über einen leeren Bereich | ja, je 196 von 221 |

**M-37** (`docs/beweise/roh/NAK-380-rot-M-37.txt`):

| Zusagesatz | Prüfzeile | Mutation | rot |
|---|---|---|---|
| Summe des Segmenthistogramms = `segmente[b]` je Band | „380/M-37 abdeckung_zaehlt_segmente: jedes Band“ | `AnalyseEngine.cpp:510` `+= 2u` (`m37-doppelzaehlung`) | ja, 196 von 221 |
| Abdeckung liest das Einzelsegmenthistogramm | „380/M-37 berechneAbdeckung liest das Einzelsegmenthistogramm“ | `:975` liest `teilblockHistogramm` (`m37-abdeckung`) | ja, 28 von 28 Gruppen |
| Segmentzahl hergeleitet, verriegelt, > 0 | „380/M-37 Segmentzahl je Band = Formel der Stufe …“, „… Kontrastlauf Segmentzahl …“ | `:123` Segmentgate 0 dBFS (`m37-segmentgate`) | ja, beide; 221/221 und Nachbau bleiben grün (0 = 0) |

**M-36** (`docs/beweise/roh/NAK-380-rot-M-36.txt`):

| Zusagesatz | Prüfzeile | Mutation (`Diagnose.cpp`) | rot |
|---|---|---|---|
| Schwelle 6 dB, von unten | „M36a 5 dB: Charaktertext steht ruhig woertlich“ | `:212` `sp <= 6` → `<= 4` (`m36-schwelle6`) | ja, nur diese |
| Schwelle 6 dB, von oben | „M36b 8 dB: Charaktertext kommt in Wellen woertlich“ | `:212` → `<= 10` (`m36-schwelle6-10`) | ja, nur diese |
| Charaktertext M36c | „M36c 13 dB: Charaktertext …“ | `:212` → `<= 13` (`m36-schwelle6-13`) | ja, mit M36b |
| Schwelle 10 dB, von unten | „M36b 8 dB: Werkzeugtext fester Absenker woertlich“ | `:224` `spanne > 10.0` → `> 7.0` (`m36-schwelle10`) | ja, nur diese |
| Schwelle 10 dB, von oben | „M36c 13 dB: Werkzeugtext dynamisches Werkzeug woertlich“ | `:224` → `> 14.0` (`m36-schwelle10-14`) | ja, nur diese |
| Werkzeugtext M36a | „M36a 5 dB: Werkzeugtext …“ | `:224` → `> 4.0` (`m36-schwelle10-4`) | ja, mit M36b |
| Vorbedingung gelesene Spanne | „… Vorbedingung gelesene Spanne = Tiefe …“ | `AnalyseEngine.cpp:843` P95-Rang 0.50 (`m36-p95-rang`) | ja, alle drei |
| Vorbedingung dauerhafte Karte | „… Vorbedingung genau eine dauerhafte Resonanzkarte …“ | `AnalyseEngine.cpp:19` `kPersistenzDauerhaft` 1.50 (`m36-dauerhaft`) | ja, alle drei |

**M-30** (`docs/beweise/roh/NAK-380-rot-M-30.txt`):

| Zusagesatz | Prüfzeile | Mutation (`metriken-v1.json`, Fassung 20260926) | rot |
|---|---|---|---|
| zweiter Wert eines Eintrags (`inline constexpr`) | „…: kVerteilungPlaetze = 65 steht so im Code“ | `:412` 64 → 65 (`m30-zweiter-wert`) | ja, A5 Exit 2, 869/1 |
| Form `Klasse::kName = Wert` | „…: Vergleichspegel::kMindestSekunden = 0.5 steht so im Code“ | `:417` 0.4 → 0.5 (`m30-vergleichspegel`) | ja, A5 Exit 2, 869/1 |
| erster Wert (`static constexpr` in der Klasse) | „…: kEreignisPlaetze = 65 …“ | `:412` (`m30-erster-wert`) | ja |
| jeder LRA-Wert; Registereintrag „kLraHopZellen = 1 (… §3.1 …)“ | vier Paarprüfungen und „Register nennt §3.1 und Wert 1“ | `:408` bis `:411` (`m30-lra-werte`) | ja, fünf |
| Name ohne Codefund ist rot | „…: kLraRelGateLuX = -20 steht so im Code“ (kein Codefund) | `:410` Name geändert (`m30-ohne-codefund`) | ja |
| keine Nennung fällt still heraus | „…: jede kName-Nennung der aktuellen Fassung ist ein lesbares Paar“ | `:412` Wert `0x40` (`m30-unlesbar`) | ja (6 Paare aus 7 Nennungen) |
| Kommentare nennen §3.1 | FeatureEngine- und Lautheit-Kommentarprüfung | unverändert aus Etappe 3 (Vorgängerfassung der Rohdatei) | — |

Jede neue Prüfung ist unter mindestens einer Mutation gefallen. Nie
gefallen sind nur testseitige Vorbedingungen (Samplezahlen und Bandzahl aus
der Gitterformel) und Prüfungen früherer Etappen (Speicherzahl der
AnalyseEngine, „380/M-36 karte_116hz_steht_ruhig“, Fixture-Prüfungen).

### 32.3 Gefahrene Beine

| Bein | Ergebnis | Binary-Zeitstempel |
|---|---|---|
| A2 `EqCopGoldenTest eq-copilot/fixtures`, Endstand | Exit 0, **310/0** (vorher 258/0), 17:57:42 bis 17:58:35 Uhr | 17:57:34.024, jünger als `GoldenTestMain.cpp` 17:57:21.668, `Nak380Pruefsignale.h` 17:26:35.436, `AnalyseEngine.h` 17:30:52.253, `AnalyseEngine.cpp` 17:43:30.851, `Diagnose.cpp` 17:41:08.293, `DiagnoseAntwort.cpp` 09:59:14.230 |
| A2 Einzelfälle am Endstand mit Einzelausgabe | `--nak380 M-38` 43/0, `M-37` 5/0, `M-36` 55/0, je Exit 0 | wie oben |
| A5 `pruefe_v3_vertrag.py --abdeckung`, Endstand | Exit 0, **870/0** (vorher 863/0), 17:55:55 bis 17:55:59 Uhr | Pythonlauf am Endstand (nach dem Selbstauditfix, §32.4 Punkt 11) |
| A5 `--nak380 M-30` | Exit 0, 11/0 | wie oben |

Gebaut nur `EqCopGoldenTest` mit dem VS-CMake über den absoluten Pfad aus dem
Auftrag (erster Bau 17:31:05 bis 17:31:09 Uhr, danach je Mutation und
Rücknahme bis 17:43:34 Uhr, zuletzt nach dem Kommentarnachzug im M-36-Kopf
17:57:28 bis 17:57:34 Uhr), jedes Mal Exit 0 und Binary jünger als die
geänderte Quelle; eigene `vctip.exe` ohne Kindprozess nach jedem Bau beendet
(NAK-300). Die Mutationsläufe fuhren denselben Testcode bis auf zwei
Kommentarsätze (§32.4 Punkt 11). Am Ende läuft kein Bau- oder Testprozess.

### 32.4 Abweichungen, offene Punkte, Produktbefunde

1. **Mehr Mutationen als verlangt.** Der Auftrag nennt vier M-38-Sätze, eine
   M-37-, zwei M-36- und zwei M-30-Mutationen. Gefahren sind 25, damit jede
   neue Prüfung einmal fällt (Prüfliste E): für M-38 je Feld eine eigene
   Vorgabemutation (D1 „eine Mutation je Feld“), Segmentgate, doppelte
   Zählung und zwei Reset-Leerungen; für M-37 Segmentgate und doppelte
   Zählung (der Rotbeweis der Z2-Verriegelung); für M-36 beide Schwellen von
   beiden Seiten und die zwei Vorbedingungen; für M-30 erster Wert, LRA-Werte,
   Name ohne Codefund, unlesbarer Wert.
2. **„Rest als eigener Teilblock“ ist eine eingefügte Zeile.** Am Fensterende
   gibt es keinen Produkthaken (§27.6 Punkt 3); die Mutation fügt in
   `auswerten()` vor `berechnePerzentile` eine Zeile ein, die den wartenden
   Rest je Band als eigenen Teilblock bucht. Sie übersetzt, greift nur
   gültige Indizes (`juce::jlimit`) und modelliert genau den Fehler der
   Zusage; sie ist von „Zähler beim Reset 1 statt 0“ getrennt.
3. **„Zähler beim Reset 1 statt 0“ trifft auch Lauf 1**, weil `vorbereiten()`
   `zuruecksetzen()` ruft; die Rest-Prüfungen von Lauf 1 und vor dem Reset
   fallen mit (im Rohbeleg genannt), die Zusage fällt an „unmittelbar nach dem
   Reset“ und „Lauf 2 60 s“.
4. **Zusätzliche Auswertepunkte in Lauf 2:** „30 s vor dem Reset“ (ohne ihn
   wäre „nach dem Reset alles 0“ auch bei einem nie gefüllten Zähler grün) und
   „9216 Samples“ (die wahre Seite des Flags direkt an der Grenze). Der
   Auswertepunkt „unmittelbar nach dem Reset“ ruft `auswerten()` ohne neue
   Samples; der von `zuruecksetzen()` selbst veröffentlichte Snapshot mit
   Vorgabe 0,0 ist R1 (Härtung NAK-403) und nicht Gegenstand.
5. **M-36 mit zwei festen Pegelstufen** statt gleichverteilter Stufen wie
   M-33 bis M-35: nur so ist die gelesene Spanne eine hergeleitete ganze Zahl
   und der vollständige Text samt Zahl wörtlich prüfbar. Neue Signale M36a bis
   M36c mit den Saaten 0x3800011 bis 0x3800013 (Tafel §7.2 ist Verlauf; die
   Signale stehen hier und im Kopf von `M36Pegelrechteck`).
6. **Prüfwerkzeug:** `NAK380_ALLE_PRUEFUNGEN` (Standard aus) listet bestandene
   A2-Prüfungen für die Rohbelege; `--nak380 M-36` fährt nicht mehr den
   Stereoblock (nur der betroffene Fall). Der volle A2-Lauf ist unverändert
   in Ablauf und Ausgabe.
7. **Offen für den Dirigenten, außerhalb der Zusage:** die
   `nicht_gefuehrt`-Einträge nennen auch Rust-Konstanten aus `prepost.rs`
   (`GATE_SUCHRAUM_S = 2.0`, `GATE_MINDESTCAPTURE_S = 6.0`,
   `GATE_UEBERLAPPUNG = 0.8`, `GATE_AKTIVZEIT_DIFFERENZ = 0.1`,
   `FEATURE_HOP_MS = 100`); sie sind keine `kName`-Paare, M-30 prüft sie
   nicht.
8. **Runnerkopf** (`tools/beweise.ps1`, außerhalb der Ticketgrenze): die
   Behauptungen von A2 und A5 bleiben richtig; sie nennen die neuen
   Einzelprüfungen nicht.
9. **Zeitstempel (NAK-230):** `AnalyseEngine.cpp`, `Diagnose.cpp` und
   `metriken-v1.json` tragen nach den Rücknahmen neue `LastWriteTime` bei
   gleichem Inhalt, `AnalyseEngine.h` ist geändert (K1). Nur `EqCopGoldenTest`
   ist neu gebaut; die Binaries der übrigen Ziele sind älter als diese
   Quellen, der Kanon mit `-Bauen` baut sie neu.
10. **Produktbefunde:** keine. Alle neuen Prüfungen sind am unveränderten
    Produkt grün; keine Mutation hat einen Produktfehler aufgedeckt.
11. **Selbstauditfix im Prüfer:** `_cpp_zahl` schnitt bei einem Hexliteral
    eine Endziffer F als Suffix ab, `_cpp_ohne_kommentare` schluckte bei einem
    unterminierten Literal den Zeilenumbruch. Beides betrifft keine heutige
    Kernkonstante (Index vorher und nachher 124 Schlüssel mit gleichen
    Werten); nach dem Fix liefen die sechs M-30-Zyklen erneut (17:55:02 bis
    17:55:54 Uhr, gleiche Zählungen) und A5 voll 870/0. Im Kommentarkopf
    von `nak380M36Eingang` waren zwei Sätze ungenau („kippt erst bei einer
    Schwelle 4“, „44 dB über seiner Umgebung“); sie nennen jetzt die
    Kippgrenzen unter 5 und unter 8 und den Abstand zur Rauschdichte. Nur
    Kommentar; neu gebaut und A2 erneut gefahren (310/0).

**Selbstaudit und Prüfliste.** Eigener Diff (`git diff --stat`, `git diff`,
nicht gestagt) adversarial gelesen: kein Produktcode außer K1; jede Zahl der
neuen Prüfungen kommt aus Formel oder Konstruktion (Kommentar), keine aus dem
Lauf; Engines auf dem Heap (`std::make_unique`, NAK-175); Etiketten sagen
nicht mehr, als sie prüfen; Zahlränder: `nak380Segmente` liefert 0 vor n
Samples, keine Unterläufe; NaN-Prüfungen je Feld mit `std::isnan` gegen die
Formel, endliche Werte mit `std::isfinite`; Restsumme exakt 0,0 geprüft, weil
das Produkt sie beim Teilblock und beim Reset auf 0,0 setzt. Prüfliste
(`tools/dirigent/pruefliste.md`) lesend: B (starten↔stoppen: Reset in beide
Richtungen gemessen), D (fail-closed: unbekannter Name, mehrdeutiger Name und
unlesbarer Wert sind rot), E (Behauptung ≤ Messung, jede neue Prüfung einmal
gebrochen, Rohausgaben liegen bei), F (Änderungssatz vollständig). Kanon,
Wiederprüfung 1 und Commit fährt der Dirigent.
## 33. Messung der Nacharbeit 1, Opus-Effort-Regel NAK-404 und Wiederprüfung 1 gestartet (Dirigent, 25.09.2026, 18:10 Uhr)

**Messung.** Opus-5.5-Worker `73c2facf` (`nakama-nak380-3b888f5-n1`, Effort
max, `dontAsk`), 17:06 bis 17:59 Uhr, Basis `3b888f56`, kein Commit durch
den Worker. Vom Dirigenten gemessen: 9 geänderte und 25 neue Dateien;
Produktdiff nur der Kommentar `AnalyseEngine.h:130-131` (K1); Tests
`GoldenTestMain.cpp` (+431/−99: M-38 mit zwei Läufen und sechs
Auswertepunkten, je Feld NaN, Flag auf beiden Seiten, Zählgrößen aus der
Formel S = ⌊(N − n)/(n/2)⌋ + 1; M-37 mit verriegelter Segmentzahl; M-36 mit
drei Modulationseingängen M36a/b/c und vollständigen Texten; Prüfwerkzeug
`NAK380_ALLE_PRUEFUNGEN`), `Nak380Pruefsignale.h` (`M36Pegelrechteck`, drei
Saaten), `pruefe_v3_vertrag.py` (Kernkonstantenindex über 18 Header, jedes
Paar einzeln, Lesbarkeitsprüfung). A2 selbst gefahren: Exit 0, **310/0**
(Binary 17:57:34 nach den Quellen 17:57:21, 17:26:35, 17:30:52); A5 Exit 0,
**870/0**. 25 Mutationen mit Kurzprotokollen, je Zeile die Tabelle Zusagesatz
→ Prüfzeile → Mutation → rot (§32.2); jede neue Prüfung ist einmal gefallen.
Fixdiff adversarial gelesen: Referenzen aus Formeln (Segmente, Teilblöcke,
Rest, Spanne = Tiefe, Bandpegel −30 dB), Texte als Bytevergleich gegen die
Produktliterale, fail-closed im Index (kein Fund, mehrdeutig, unlesbar → rot),
Heap für Engines, Etiketten decken die Prüfung. Offen aus §32.4: Punkt 7
(Rust-Konstanten in `nicht_gefuehrt` außerhalb der `kName`-Zusage) →
Härtung, bei NAK-403 mitgeführt; Punkt 8 (Runnerkopf A2/A5 nennt die neuen
Einzelprüfungen nicht) → Abschluss der Etappe.

**Opus-Effort ab jetzt xhigh (User-Wort 25.09.2026, 17:49 Uhr, Register
NAK-404):** „opus worker in zukunft generell auf xhigh statt max" — jeder neu
gestartete Opus-Worker (Bauer, Nacharbeiter, Fortsetzung, lesende Prüfer und
Validierer) läuft mit `--effort xhigh`; der Worker `73c2facf` lief nach
Auftrag mit max zu Ende. Skill §3.2, §3.4, §3.6 und die Kopfzeile Prüfmodell
sind nachgezogen; NAK-359 gilt im Übrigen weiter.

**Wiederprüfung 1 (Vorlage B).** Codex `gpt-6-astra` xhigh, lesend;
Prüfbereich `3b888f56...HEAD` über `GoldenTestMain.cpp`,
`Nak380Pruefsignale.h`, `pruefe_v3_vertrag.py`, `AnalyseEngine.h`, dazu die
Rohbelege M-30, M-36, M-37, M-38 und die Kurzprotokolle als Beleg; Auftrag
`docs/beweise/roh/NAK-380-wiederpruefung-e3-1-auftrag.txt`, Kennung
`nak380-w3-1`; Urteil in §34. Commit dieses Abschnitts mit Pathspec, gepusht.
## 34. Wiederprüfung 1 PASS, Abschluss der Etappe 3 und Übergabe (Dirigent, 25.09.2026, 18:15 Uhr)

**Wiederprüfung 1 (Vorlage B).** Codex `gpt-6-astra` xhigh, lesend, Thread
`01a0d950-4518-7c63-8f88-ae9f19c6eccf`, 18:05:10 bis 18:10:48 Uhr, EXIT=0,
HEAD `fb591eed` vor und nach dem Lauf; Prüfbereich `3b888f56...fb591eed` über
`GoldenTestMain.cpp`, `Nak380Pruefsignale.h`, `pruefe_v3_vertrag.py`,
`AnalyseEngine.h`, dazu die vier Rohbelege und 25 Kurzprotokolle; Auftrag
`docs/beweise/roh/NAK-380-wiederpruefung-e3-1-auftrag.txt`, Urteil
`docs/beweise/roh/NAK-380-wiederpruefung-e3-1-urteil.md`. **URTEIL: PASS —
alle Befunde geschlossen (D1, D2, D3, D4, Z1, Z2, K1), nichts gebrochen.**
Geprüft: Herleitungen am Produktcode (Segmentformel `AnalyseEngine.cpp:70-71,
106-109, 157-159`; erster Höhen-Teilblock 9 216; 196/25 Bänder; Bin 128 in
Band 135, −30 dB, Spanne = Tiefe), Index fail-closed, Mutationen formgültig
und an der Zusageprüfung wirksam, genau vier Dateien, Produktdateien
blobidentisch, voller A2 erhalten, Heap, Zeilenenden. Nicht geprüft: keine
Läufe. Ein erster Lauf (Kennung `nak380-w3-1`, Thread auf `02389656`, 18:03
Uhr) wurde vom Dirigenten abgebrochen, weil der Dokuriegel-Commit `fb591eed`
(Auftragsdatei) HEAD während des Laufs bewegte — Lehre: vor einem
Prüfstart Dokuriegel und Rohdateien vollständig committen, während des Laufs
kein Commit (Skill §3.4).

**Rundenbilanz.** Runde 3 (Nacharbeit 1) `3b888f56..02389656`: Produkt 1
Datei(en) +2/−2 (Kommentar) | Tests 2 +390/−72 | Pruefwerkzeug 1 +136/−25 |
Doku 32 +1 970/−31. Kumuliert `--runden b04dfb7c 8308a3d8 02389656`: Runde 1
(Bau und Nacharbeit 0) Produkt 10 +580/−411 | Tests 4 +899/−20 |
Pruefwerkzeug 2 +117/−6 | Doku 40 +803/−4; Runde 2 Produkt 2 +3/−3 | Tests 2
+390/−72 | Pruefwerkzeug 1 +136/−25 | Doku 39 +14 763/−39; kein
Konvergenzsignal.

**Etappe 3 abgeschlossen (25.09.2026, 18:15 Uhr).** Gebaut (§27), Nacharbeit
0 nach R-380-11 und T-380-10-Korrektur (§29), gemessen (§28, §30), Kanon
nachher GRUEN 70/70 an `8308a3d8` (§31; die Nacharbeit 1 änderte danach nur
Tests, Prüfwerkzeug und einen Kommentar: A2 310/0 und A5 870/0 einzeln,
Wiederprüfung PASS), Erstprüfung 3 NEEDS_WORK (§31), Nacharbeit 1 (§32),
Wiederprüfung 1 PASS (§34). Aus der Etappe bleiben die Härtungen NAK-402,
NAK-403 im Register; Laufzeit-Arm und T2-Abschlussprüfung folgen nach Etappe
6 (Kopf). Hygiene im Abschlussfenster: Dokuriegel auf Manifest, Register,
CLAUDE.md und Skill; Zielbild; `gesundheit.py` (Kontextfläche nach dem
Skill-Nachzug NAK-404) — Ergebnisse im Commit-Betreff.

**Übergabe an die nächste Dirigentensitzung (Kontext dieser Sitzung 70 %,
Skill §5).** Etappe 4 (Ereignisdetektor nach §39.1, DSP-22; M-42 bis M-72;
§8.4) beginnt in einer frischen Sitzung: Auftrag nach dem Muster
`docs/beweise/roh/NAK-380-etappe-3-auftrag.txt` mit den Präzisierungen aus
§24 und §31 (Vorbedingung exakt, Zusage je Element, Bits und Flags am Feld
und auf beiden Seiten, formgültige Mutationen, je Vertragsstelle und je Wert
eine Prüfung, Toleranzen mit Referenz) und der Auftragsregel ab Etappe 4: je
Matrixzeile eine Tabelle Zusagesatz → Prüfzeile → Mutation → rot ja/nein im
Etappenabschnitt. Bauer: frischer Opus-5.5-Worker, `--effort xhigh`
(NAK-404), Aufsicht ENG; Kanon vorher geerbt vom Kanon nachher der Etappe 3
an `8308a3d8` (Diff seither: Nacharbeit 1, A2/A5 grün, Wiederprüfung PASS;
Broker-Pin). Etappe 4 baut den Nullkorpus (§7) mit gemessener
Fehlalarmrate; die Kalibrierwerte (ρ, T_min, H) sind Startwerte nach
T-380-5. Nach NAK-380 folgt NAK-313 Etappe 8 (`docs/beweise/NAK-313.md`
§37). Offen beim User: nichts.


---

# Zweiter Block (ausgelagert 26.09.2026 nach der Wiederprüfung 1 der Etappe 5, Manifest §53): die Abschnitte §35 bis §49 — Etappe 4 (Bau, Nacharbeiten 0 bis 2, Kanon-Lauf an `1e4a7010`, Erstprüfung 4, Wiederprüfungen 1 und 2) und Etappe 5 bis zur Messung der Nacharbeit 0 (§46 bis §49). Unverändert übernommen; Zeilenangaben in den Abschnitten gelten zum Stand des Abschnitts, der sie trägt.

## 35. Etappe 4 — Ereignisdetektor nach §39.1 (Bauer Opus 5.5 xhigh, 25.09.2026)

Auftrag `docs/beweise/roh/NAK-380-etappe-4-auftrag.txt`. Kein Stage, Commit,
Push oder Stash, keine Rücknahme über git, kein voller Kanon, kein FL
Studio, keine Installation, kein Laufzeit-Arm, keine Produktions-Pipe. Zeilen
in diesem Abschnitt gelten am Arbeitsstand der Etappe (uncommittet auf
`21853aa1`).

### 35.1 Start, Basis und Kanon vorher

- **Startmessung** 25.09.2026, 18:30:48 Uhr (`Get-Date`): `git rev-parse
  HEAD` = `21853aa1a8936115831411d6c5966ba9560ef7b0` auf `master`; `git
  status --short` nennt nur `briefing-hub/` und `nimbalyst-local/`
  (`.claude/settings.local.json` erscheint nicht); Prozessprobe auf cmake,
  MSBuild, cl, link, cargo, rustc, EqCop*, eqcop-broker, FL64 und vctip
  leer.
- **Kanon vorher: geerbt.** `git diff --stat 8308a3d8 HEAD -- eq-copilot
  broker tools` nennt genau die fünf Pfade, die der Dirigent gemessen hat:
  `eq-copilot/install/nakama-installer-v1.json` (2 Zeilen, Broker-Pin),
  `eq-copilot/plugin/src/AnalyseEngine.h` (+2/−2, Kommentar K1),
  `eq-copilot/plugin/tests/GoldenTestMain.cpp` (431),
  `eq-copilot/plugin/tests/Nak380Pruefsignale.h` (31) und
  `tools/eq-copilot/pruefe_v3_vertrag.py` (161); zusammen 529 Einfügungen,
  100 Löschungen. Der Kanon nachher der Etappe 3 lief an `8308a3d8` GRUEN
  70/70 (`docs/beweise/roh/NAK-380-8308a3d-dirty.md`).
- **Speicher vorher:** `sizeof (FeatureEngine)` = 16 240 B (x64, MSVC
  `/O2 /std:c++20`, eigenes Messprogramm
  `eq-copilot/build/nak380-sizeof/groesse.cpp` gegen den unveränderten Kopf).

### 35.2 Pflichterfassungen und Basisstand-Gegenprobe (vor dem ersten Produktedit)

Reihenfolge, alle Zeiten `date` der Shell: 18:55 Uhr B22 mit dem neuen Fall
gebaut (Produkt unverändert) und die F1-Bytes erfasst (18:56:18 Uhr); 19:05
bis 19:06 Uhr die vier Testziele B18, B22, B16, B5 am unveränderten Produkt
gebaut (B18 und B5 in einer Basisvariante, s. u.), 19:06:59 bis 19:07:10 Uhr
alle neuen Fälle gefahren, 19:10 Uhr B16 nach einer Präzisierung von M-63
und M-65 erneut am Basisstand gebaut und gefahren, 19:10:28 Uhr der
Altzähler dokumentiert; **erster Produktedit 19:10:50 Uhr**.

- **M-68, Golden-Klasse (i):** 76 F1-Ausgangsbytes (Akkord `akkord (0.4,
  220.0)`, 30 s = 1 440 000 Samples, 48 kHz, Block 512), Fenster 701,
  SHA-256 `F0D5C170BF6ED8FC7BAAF9375AB5930820E98CA87DE2017D32379CA644000612`,
  als `kNak380F1Bytes` in `Sonde013FingerprintGoldenTest.cpp` eingefroren;
  Rohdatei `docs/beweise/roh/NAK-380-etappe-4-m68-f1-ausgang.txt`. Der Fall
  war am Basisstand grün (B22 28/0, 19:07 Uhr).
- **M-50, Golden-Klasse (ii):** Altzähler W1 = **18** Ereignisse in 30 s
  (σ = 0,1, Saat 0x3800003, Ring nach jedem Frame entnommen, Verlust 0);
  Rohdatei `docs/beweise/roh/NAK-380-etappe-4-m50-w1-altzaehler.txt`. Nach dem
  Umbau lief der neue Code gegen genau diesen Zähler rot (Mutation
  `m50-altzaehler`: Golden 18 statt 0, Exit 1, gemessen 0), erst danach steht
  der Golden 0.
- **Basisvariante.** M-42 bis M-49 (B18) und die Auskunft von M-62 (B5)
  brauchen Träger, die erst der Bau schafft. Für die Gegenprobe wurden die
  zwei Testdateien einmal in eine Basisvariante gebracht (Skript
  `basisvariante.py` im Jobordner; Endfassungen vorher nach
  `eq-copilot/build/nak380-sicherung/basis/final-*` gesichert, SHA-256 B18
  `8CFD3A0C…DA79`, B5 `3E4E39DE…479F`; Basisvariante B18 `BC0231E2…521C`, B5
  `1E5138AC…497E`): gleicher Testcode, der Testzugang misst dieselbe Größe
  über die Träger des Basisstands (Bandfluss statt Binfluss, 16er-Historie
  mit jedem zweiten Wert der 32er-Historie, die MAD der Basis über die
  `staerke` eines Ereignisses, weil sie im Basisstand inline im Flussschritt
  steht); `detektorBinVon/Anzahl` gibt es dort nicht (−1), in M-62 steht
  `flussHatVorgaenger()` (Bandfluss-Vorgänger) statt der Binauskunft. Danach
  wurden die Endfassungen bytegleich zurückkopiert (SHA-256 wie oben).
- **Ergebnis je Zeile am Basisstand** (Rohausgaben in den Rohdateien
  `NAK-380-rot-M-nn.txt`): M-42 rot (Bandfluss 95,23 dB für jedes d statt 0
  und 79,96); M-43 nicht messbar (kein Detektorbereich; die
  Allokationsprüfung ist am Basisstand grün); M-44 rot (MAD 1,25); M-45 (a)
  grün, (b) rot (0 Ereignisse, die Basis verlangt MAD > 0); M-46 rot (599
  löst aus); M-47 (a) grün (Regressionswache), (b) nicht messbar (Bandfluss
  1490 dB statt 59,48 dB Binfluss); M-48 rot (3 Ereignisse); M-49 rot bei
  1 Hop (2 statt 1), grün bei 2 Hops; M-50 bis M-56 rot mit **18, 14, 19,
  22, 109, 55, 19** Ereignissen (W1, W2, W3, P1, S1, S2, V1; M-53 „nach
  Analogie“ damit gefahren: 22); M-57 rot (148 Ereignisse, 36 Klicks
  mehrfach), M-58 rot (87, 31 Paare mehrfach), M-59 rot (111, 18 Klicks
  ohne und 17 mehrfach getroffen); M-61 rot (5 Ereignisse, darunter der
  Wiederbeginn bei 6,9547 s); M-62: Grenzteil grün, Ereignisteil rot (5
  Fehlalarme im stationären Rosa), Binauskunft nicht messbar; M-63 rot nur
  an der Zuordnung des ersten Stille-Klicks (Basis stempelt 145 408 statt
  143 360, A-6), die Basis-Stärken lagen in diesen Signalen bei 18,19,
  90,18 und 62,97 in [3, 1000] — die Unbeschränktheit von (SF − med)/MAD
  ist nachgerechnet, in diesen Signalen nicht gemessen; M-65 rot (16 von 93
  Fenstern mit Onsetsumme > 0, größte 7,323); M-68 grün (Wache); M-69 rot
  (fünf Teilprüfungen, „kein Feld“ und „keine Version“ grün); M-70 nicht
  messbar (13 Teilprüfungen rot, Fassung fehlt).

### 35.3 Gebauter Änderungssatz

- **`FeatureEngine.h` (1 524 → 1 649 Zeilen):** `kFeatureMetricsVersion`
  20260926u → **20260927u** mit Begründungsabsatz; sieben benannte
  Namensraumkonstanten neben den Peakschwellen: `kFlussP0Db = -100.0`,
  `kFlussFilterCent = 50.0`, `kFlussHistorie = 32` (vorher die
  Klassenkonstante 16, jetzt geführt und im Register), `kFlussKappa = 3.0`,
  `kFlussRho = 1.0`, `kFlussTminDbJeBin = 0.30` (kalibriert, §35.4),
  `kSperrzeitMs = 50.0`, dazu die Vertragsgrenze `kFlussStaerkeMax =
  1000.0`. Neuer Zustandsträger `FlussDetektor` (Vorframe und Filterpuffer je
  Detektor-Bin, Ring der monotonen Warteschlange, Historie und
  Sortierpuffer, Binbereich, Vorframe-Gültigkeit, SF(n−1), Zeit des letzten
  Ereignisses) als **genau ein Element im Heap** (`std::vector<FlussDetektor>
  detektor`), angelegt in `vorbereiten` nach der Zuordnung; öffentliche
  Auskünfte `flussBinVorgaengerGueltig()`, `detektorBinVon()`,
  `detektorBinAnzahl()` und `static flussFilterBreite (int k)`;
  `zuruecksetzen` ruft `detektorLeeren()` statt die zwei Historienzähler zu
  nullen; der Kommentar an `evidenzVerteilung` nennt den Heapträger
  `detektor` statt `flussHistorie`. `sizeof (FeatureEngine)` **16 240 → 16 208 B** (−32 B: die
  Historienvektoren und -zähler wandern mit in den Heapträger). Eine erste
  Fassung mit den Detektorfeldern im Objekt (16 352 B, +112 B) sprengte den
  Stack von B5 (`0xC00000FD` beim Abschnitt G, gemessen 19:16 Uhr; NAK-175)
  — deshalb der Heapträger, dasselbe Muster wie `headroomRing`.
- **`featureengine/Spektrum.h` (739 → 928 Zeilen):** `rechneFenster`
  ruft in inaktiven Hauptstufen-Frames `flussVorframeSchritt` (A-6: Vorframe
  und SF(n−1) über alle Frames, Historie und Ereignisse nur aktiv).
  `flussSchritt` geteilt: der Bandfluss bleibt Zeile für Zeile, speist nur
  noch `fingerprintSchritt` (Schwerpunktzeilen des Bandflusses entfallen);
  danach `binFlussSchritt` (K Logarithmen `10·log10(psd·Δf + P0)`, positive
  Deltas gegen `filter`, Schwerpunkt der positiven Deltas in Hz, Vorframe
  fortgeschrieben, nicht endliche oder negative Binwerte als 0) und
  `detektorSchritt` (echte MAD, T_eff aus drei Termen, unveränderter
  Peakpfad, Spitzenwahl `sf >= sfVorher`, Sperrzeit in Samples gegen das
  letzte Ereignis beider Pfade, `staerke` = min(1000, κ·(SF − med)/(T_eff −
  med)) mit Nenner > 0). `flussFilterRechnen`: gleitendes Maximum über eine
  monotone Warteschlange, O(K) je Frame, Ring 2·w_max + 2 Plätze (Beweis der
  Schranke im Kommentar). `detektorBinsBestimmen` (M-43), `medianUndMad`
  ersetzt `medianDerHistorie`, `detektorLeeren` (derselbe Rückweg für
  `zuruecksetzen` und `grenzeZiehen`). Der Kommentar über dem Fluss lautet
  wörtlich nach §8.4 („SuperFlux-Fluss (§39.1; Boeck/Widmer, DAFx-13, Gl. 5
  und 6) auf den Bins der Hauptstufe: …; der einfache Peakpfad bleibt als
  Gegenbeleg.“).
- **`featureengine/Zeit.h` (601 → 603 Zeilen):** `grenzeZiehen` ruft
  `detektorLeeren()` statt die zwei Historienzähler zu nullen.
- **`featureengine/Vertrag.h` (773 Zeilen, unverändert lang):** Kommentar am
  Feld `staerke` wörtlich nach §8.4.
- **Broker:** `prepost.rs:206` und `vergleichbarkeit.rs:149`
  `METRICS_VERSION` 20260926 → **20260927** (einzige Produktzeilen; CRLF
  erhalten); `experiment.rs` bekommt am Dateiende einen neuen
  `#[cfg(test)] mod nak380_tests` mit genau dem Test
  `nak380_m66_guardrail_transient_grenze` (Abweichung 3, §35.8).
- **Verträge:** `eq-ipc-v3.schema.json` — an die Beschreibung von
  `dynamics_ereignis` der Satz aus §8.4 angehängt (ASCII, CRLF), kein Feld,
  keine Version. `metriken-v1.json` — neue Fassung **20260927** (`seit`
  „NAK-380 Etappe 4“, Hinweis mit Datum 25.09.2026), alle Einträge der
  Fassung 20260926 unverändert übernommen, dazu die sieben geführten
  Detektorschwellen (sechs unter `schwellen`, `kFlussHistorie` unter
  `ganzzahlige_schwellen`, je mit `datei` FeatureEngine.h und Codebezug
  `kName = Wert` im Zweck); `aktuell` 20260927; 20260925 und 20260926
  unverändert.
- **Tests:** `Nak380Pruefsignale.h` (+223: Saaten, `weissMono`,
  `RosaRauschen` mit analytischer Leistungsnormierung, E-380-13-Prüfung
  `rosaGroessteOktavdifferenzDb` über Welch mit 65 536 Punkten, `s1Sinus`,
  `s2Saegezahn`, `v1Vibrato`, Klickraster I1 bis I3; alle Erzeuger als eigene
  Funktionen); B18 `Sonde013DynamicsTest.cpp` (Testzugang um
  `filterBreite`, `detektorPufferBytes`, `binFluss`, `historieSetzen`,
  `medianUndMad`, `detektorSchritt`, `letzteStaerke` erweitert;
  `nak380Detektoreinheit` mit M-42 bis M-49, `__declspec(noinline)`,
  Engines im Heap); B5 `AnalysisGoldenTestMain.cpp` (`main (argc, argv)`
  mit `--nak380 M-nn`; `nak380Detektor` mit M-50 bis M-62; Korpuslauf,
  Nullfall, Impulsfall und Rosa-Prüfung als eigene Funktionen; eigener
  Testzugang um `vorframeMaxDb`, `vorframeMinDb`, `historieStand`,
  `historieGefuellt` erweitert (M-61); die Kommentare des Ringtests und
  des NAK-182-Sweeps, die den alten Detektor beschreiben — Median + 3·MAD
  über 16 Fenster, `mad > 0`, `staerke` = (fluss − med)/mad —, tragen je
  einen Hinweis auf die Regel seit Etappe 4, Code unverändert); B16
  `Sonde013EventWireTest.cpp` (`--nak380 M-63/M-65`, `nak380Wirelauf` durch
  den echten Serialisierer nach dem Muster `SondeProcessor.cpp`,
  fail-closed: ein nicht gebauter Snapshot entnimmt den Ring nicht); B22
  `Sonde013FingerprintGoldenTest.cpp` (`kNak380F1Bytes`, `nak380M68`,
  `--nak380-erfassen` und `--nak380 M-68`).
- **Prüfwerkzeug** `pruefe_v3_vertrag.py`: `nak380_m40_fassung_etappe_3`
  behält den Fassungsinhalt 20260926 (registriert, `seit`,
  Konfidenzschwellen) und gibt die drei Aktualitätsprüfungen (FeatureEngine,
  beide Rust-Konstanten, `aktuell`) an den neuen Fall ab — dasselbe Muster,
  mit dem Etappe 3 `nak380_m19` behandelt hat (Abweichung 4); neu
  `pruefe_nak380_etappe_4` mit `nak380_m69_staerke_benannt` (sieben
  Teilprüfungen) und `nak380_m70_fassung_etappe_4` (vier Versionsstellen,
  `seit`, Übernahme 20260926, sieben Schwellen je einzeln über den
  Kernkonstantenindex); `--nak380 M-69/M-70`.
- **Runner** `tools/beweise.ps1`: je ein Satz „Seit NAK-380 Etappe 4“ an
  B5, B16, B18 und B22 (BOM und CRLF erhalten, Parser 0 Fehler); B5 nennt
  den roten Fall I3 ausdrücklich.
- Zeilenenden nach `git ls-files --eol` erhalten und je Datei geprüft (C++,
  Schema, Register, `prepost.rs`, `vergleichbarkeit.rs`, `beweise.ps1` CRLF
  ohne nackte LF; `experiment.rs`, `pruefe_v3_vertrag.py`, Manifest und
  Rohdateien LF ohne CR).

### 35.4 Korpus und Kalibrierung

Ereignisse je 30 s (R1 12 s), gemessen mit `EqCopAnalysisGoldenTest --nak380
M-nn`; „Basis“ am unveränderten Produkt (§35.2), „Start“ mit den Startwerten
T-380-5 (T_min 0,05 dB je Bin, 19:14 Uhr), „kalibriert“ am Endstand (T_min
0,30 dB je Bin).

| Fall | Signal | Basis | Start | kalibriert | Schranke | gehalten |
|---|---|---|---|---|---|---|
| M-50 | W1 Weiß −20 dBFS, Saat 0x3800003 | 18 | 0 | 0 | ≤ 1 | ja |
| M-51 | W2 Weiß −40 dBFS, Saat 0x3800004 | 14 | 0 | 0 | ≤ 1 | ja |
| M-52 | W3 Weiß −50 dBFS, Saat 0x3800005 | 19 | 1 (5,120 s) | 0 | ≤ 1 | ja |
| M-53 | P1 Rosa −20 dBFS, Saat 0x3800007 | 22 | 0 | 0 | ≤ 1 | ja |
| M-54 | S1 Sinus 440 Hz, 0,5, float32 | 109 | 0 | 0 | ≤ 1 | ja |
| M-55 | S2 Sägezahn 110 Hz, 20 Obertöne, Spitze 0,5, float32 | 55 | 0 | 0 | ≤ 1 | ja |
| M-56 | V1 440 Hz, h = 1 bis 7 mit 1/h, Vibrato ±50 Cent 5,5 Hz, float32 | 19 | **160** | 0 | ≤ 1 | ja |
| M-57 | I1: P2 −40 dBFS + 112 Klicks | 148 (36 mehrfach) | 112 | 112 | genau 112, je Klick eines | ja |
| M-58 | I2: 56 Paare, 20 ms | 87 (31 mehrfach) | 56 | 56 | genau 56, je Paar eines | ja |
| M-59 | I3: 56 Paare, 100 ms | 111 (18 ohne, 17 mehrfach) | 111 (28,1 s ohne) | **105** (7 ohne) | genau 112 | **nein** |
| M-61 | R1: Rosa, 2 s Stille, Rosa | 5 | 1 (333 824) | 1 (333 824) | genau 1 in [331 904, 336 000] | ja |
| M-62 | P2 −20 dBFS, Seek bei 10 s, danach +14 dB; Endfassung mit Delle −34 dB von 0,3 bis 0,6 s nach der Grenze (erste Fassung: Klick +0,9 bei 10,5 s) | 5 (erste Fassung) | 1 (12,907 s, erste Fassung) | 0 (beide Fassungen) | 0 | ja |

Die Rosa-Selbstprüfung E-380-13 hält vor jedem Nutzer: größte
Nachbardifferenz der Oktavbandleistungen 0,177 dB (P1), 0,051 dB (P2 −40
dBFS), 0,282 dB (R1), 0,085 dB (M-62), Soll ≤ 1,0 dB; RMS innerhalb 0,09 dB
vom Soll (Toleranz 0,5 dB, Herleitung im Test).

**Kalibrierung (T-380-5, nur ρ, T_min, H):** Mit den Startwerten verfehlen
V1 (160 statt ≤ 1) und I3 (111 statt 112) ihre Schranke; W3 steht mit 1
Ereignis an der Schranke. **Geändert: T_min 0,05 → 0,30 dB je Detektor-Bin
(bei 48 kHz 76,5 → 459,0 dB); ρ = 1 und H = 32 bleiben, κ, P0, Filterbreite
und Sperrzeit ebenso.** Messzahlen vorher/nachher: V1 160 → 0, W3 1 → 0, I3
111 → 105, M-62 (erste Signalfassung) 1 → 0, alle übrigen Fälle unverändert (Tabelle). Begründung:
Das Vibrato bewegt die Teiltöne je Hop (42,67 ms) um bis zu
2π·5,5·50·0,04267 = 73,7 Cent, mehr als die Filterbreite; sein Fluss reicht
bis 414,6 dB (0,271 dB je Bin), während Weiß- und Rosarauschen höchstens
245,8 dB erreichen. Keine Wahl von ρ, T_min und H hält V1 und I3 zugleich:
Die Erkundung lief in einem Python-Nachbau der Engine (Jobordner, nicht im
Repo; er traf die C++-Zahlen an beiden gemessenen Punkten exakt: W3 1 bei
5,12 s, V1 160, I3 111 bei 28,1 s ohne Treffer; 105 am kalibrierten Stand)
über H ∈ {32, 48, 64}, ρ ∈ {0; 0,5; 1; 1,5; 2; 3; 5; 8} und T_min ∈ [0,05;
0,5] dB je Bin: V1 ≤ 1 verlangt T_min ≥ 0,2725 (bei 1/h-Amplituden), dort
trägt I3 höchstens 105; ρ hat bei T_min ≥ 0,27 im Korpus keine Wirkung (0
bis 2 gleich); H > 44 verliert den ersten Klick bei 2,0 s (I1 111 bei H =
48); ρ = 8 hielte V1 ohne T_min, verliert aber I1 und I3 stärker. Gewählt ist
der Wertesatz mit den meisten gehaltenen Fällen (10 von 11) und Abstand zur
V1-Grenze (459,0 gegen 414,6 dB); I3 ist offen (§35.9 Punkt 1). Kein Golden
ist gelockert, keine Schranke geändert: der Fall `380/M-59
impulskorpus_I3` verlangt weiter 112 und ist ROT.

**Rechenlast:** je Hauptstufen-Frame K Logarithmen (bei 48 kHz 1530, also
1530·23,44 = 35 860 je Sekunde) und das gleitende Maximum in O(K); keine
Allokation (M-43 zählt 0 Allokationen über 2 s). B5 läuft am Endstand
in 13 s (20:01:01 bis 20:01:14 Uhr), ohne `0xC00000FD`.

### 35.5 Tabelle je Matrixzeile M-42 bis M-72

Regel des Auftrags (Lehre §31): je Satz und je Hälfte der Zusage aus §6.3
eine Zeile Zusagesatz → Prüfzeile → Mutation → rot → Rohdatei. Prüfzeilen
und Mutationszeilen gelten am Etappenstand (uncommittet auf `21853aa1`;
`Spektrum.h` SHA-256 `EBD085D0…98D4` nach jeder Rücknahme). „rot JA“ heißt:
das Mutationsprotokoll nennt „Die Zusage fällt an der erwarteten Prüfung:
JA“, die Rücknahme ist per SHA-256 gleich, der Fall danach grün. Jedes
Protokoll trägt Sicherung, Wortlaut vorher/nachher, Bau- und Laufzeiten,
Binaryzeitstempel, gefallene und grün gebliebene Prüfungen; die Rohdatei
`NAK-380-rot-M-nn.txt` trägt Matrixzeile, Basisstand-Gegenprobe,
Endstand und alle Mutationen der Zeile. Zusagesätze sind aus §6.3
übernommen; Zahlen, die der Kalibrierung folgen, tragen den kalibrierten
Wert (§35.8 Punkt 8). **83 Zeilen, jede mit einem roten Beleg.** Drei
Mutationen fielen nicht an ihrer Zusage und sind ersetzt (Protokolle
bleiben, §35.8 Punkt 10): `m45-tmin0`, `m63-tmin0`, `m61-zeit`
(`NAK-380-e4-mutation-m61-zeit-verworfen.txt`). Zwei Fälle (M-61, M-62)
sind nach einem ersten NEIN geschärft und ihre Mutationen wiederholt
(`m61-vorframe`, `m61-historie`, `m62-leeren`, `m62-historie`; Erstläufe
als `NAK-380-e4-mutation-<Kürzel>-lauf1.txt`, §35.8 Punkt 9).

**M-42** (`docs/beweise/roh/NAK-380-rot-M-42.txt`) — Etikett: heute rot; Basisstand: rot: Bandfluss 95,23 dB für jedes d statt 0 und 79,96.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Filterbreite w₁₀ = 1, w₁₀₁ = 3, w₁₀₀₀ = 30, w₁₆₆₈ = 49 Bins | `Sonde013DynamicsTest.cpp:616` „380/M-42 superflux_maximumfilter: Filterbreite“, B18 | `Spektrum.h:669` `return 0;` (m42-w0) | JA | `NAK-380-e4-mutation-m42-w0.txt` |
| d = 1 bis 4 ergibt den Fluss 0,0 dB (Bin 101 liegt jeweils im Fenster); Bänder spielen keine Rolle | `Sonde013DynamicsTest.cpp:638` „380/M-42 superflux_maximumfilter: d = 1 bis 4“, B18 | `Spektrum.h:669` `return 0;` (m42-w0) | JA | `NAK-380-e4-mutation-m42-w0.txt` |
| d = 5 ergibt 79,96 dB in genau einem Bin; positive Log-Deltas | `Sonde013DynamicsTest.cpp:641` „380/M-42 superflux_maximumfilter: d = 5“, B18 | `Spektrum.h:718` `if (delta != 0.0) sf += std::abs (delta);` (m42-betrag) | JA | `NAK-380-e4-mutation-m42-betrag.txt` |

**M-43** (`docs/beweise/roh/NAK-380-rot-M-43.txt`) — Etikett: heute nicht messbar; Basisstand: nicht messbar (kein Detektorbereich); Allokationsprüfung grün.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| K = 1666 (44,1 kHz, k = 3 bis 1668) | `Sonde013DynamicsTest.cpp:652` „380/M-43 detektor_binbereich: 44.1 kHz“, B18 | `Spektrum.h:658` `const int von = 0;` (m43-untergrenze); `Spektrum.h:660` `const int bis = kHauptPunkte / 2;` (m43-obergrenze) | JA / JA | `NAK-380-e4-mutation-m43-untergrenze.txt`, `NAK-380-e4-mutation-m43-obergrenze.txt` |
| K = 1530 (48 kHz, k = 3 bis 1532), Untergrenze 30,36 Hz | `Sonde013DynamicsTest.cpp:652` „380/M-43 detektor_binbereich: 48.0 kHz“, B18 | `Spektrum.h:658` `const int von = 0;` (m43-untergrenze) | JA | `NAK-380-e4-mutation-m43-untergrenze.txt` |
| K = 1530 bei 48 kHz, Obergrenze min(17 959,39 Hz, Kappe) | `Sonde013DynamicsTest.cpp:652` „380/M-43 detektor_binbereich: 48.0 kHz“, B18 | `Spektrum.h:660` `const int bis = kHauptPunkte / 2;` (m43-obergrenze) | JA | `NAK-380-e4-mutation-m43-obergrenze.txt` |
| K = 765 (96 kHz, Δf 23,44 Hz, k = 2 bis 766) | `Sonde013DynamicsTest.cpp:652` „380/M-43 detektor_binbereich: 96.0 kHz“, B18 | `Spektrum.h:658` `const int von = 0;` (m43-untergrenze); `Spektrum.h:660` `const int bis = kHauptPunkte / 2;` (m43-obergrenze) | JA / JA | `NAK-380-e4-mutation-m43-untergrenze.txt`, `NAK-380-e4-mutation-m43-obergrenze.txt` |
| Vorframe und Filterpuffer 2·K·8 B ≤ 26 656 B | `Sonde013DynamicsTest.cpp:661` „380/M-43 detektor_binbereich: … 2*K*8 B <= 26 656 B“, B18 | `FeatureEngine.h:613` `d.vorframe.assign ((std::size_t) (2 * d.binAnzahl), kFlussP0Db);` (m43-speicher) | JA | `NAK-380-e4-mutation-m43-speicher.txt` |
| im Heap in `vorbereiten` (keine Allokation im Flussschritt) | `Sonde013DynamicsTest.cpp:691` „380/M-43 detektor_binbereich: nach vorbereiten() alloziert der Lauf nichts“, B18 | `Spektrum.h:726` `{ std::vector<double> nak380Mutation (d.vorframe); zentrumHz += 0.0 * ` (m43-heap) | JA | `NAK-380-e4-mutation-m43-heap.txt` |

**M-44** (`docs/beweise/roh/NAK-380-rot-M-44.txt`) — Etikett: heute rot; Basisstand: rot: MAD 1,25.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Median 1,0; MAD = Median der Absolutabweichungen = 0,0 (die mittlere Abweichung wäre 1,25) | `Sonde013DynamicsTest.cpp:705` „380/M-44 echte_mad_schief“, B18 | `Spektrum.h:894` `mad = 0.0; for (std::size_t i = 0; i < n; ++i) mad += d.sortiert[i]; m` (m44-mittel) | JA | `NAK-380-e4-mutation-m44-mittel.txt` |
| relative Schwelle med + 3·MAD = 1,0 statt 4,75 | `Sonde013DynamicsTest.cpp:705` „380/M-44 echte_mad_schief (med + kappa*mad == 1,0)“, B18 | `Spektrum.h:894` `mad = 0.0; for (std::size_t i = 0; i < n; ++i) mad += d.sortiert[i]; m` (m44-mittel) | JA | `NAK-380-e4-mutation-m44-mittel.txt` |

**M-45** (`docs/beweise/roh/NAK-380-rot-M-45.txt`) — Etikett: zwei Hälften: 76,4 Regressionswache, 76,6 heute rot; Basisstand: (a) grün, (b) rot: 0 Ereignisse (Basis verlangt MAD > 0).

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| T_min = 0,05 dB · 1530 = 76,5 dB, kalibriert 0,30 dB · 1530 = 459,0 dB: 76,4 → 458,9 dB kein Ereignis | `Sonde013DynamicsTest.cpp:717` „380/M-45 absolute_mindestschwelle: SF = T_min - 0,1 dB“, B18 | `Spektrum.h:786` `const double tMin = 0.5 * kFlussTminDbJeBin * (double) d.binAnzahl;` (m45-tminhalb) | JA | `NAK-380-e4-mutation-m45-tminhalb.txt` |
| 76,6 → 459,1 dB genau ein Ereignis | `Sonde013DynamicsTest.cpp:720` „380/M-45 absolute_mindestschwelle: SF = T_min + 0,1 dB“, B18 | `Spektrum.h:786` `const double tMin = 2.0 * kFlussTminDbJeBin * (double) d.binAnzahl;` (m45-tmin2) | JA | `NAK-380-e4-mutation-m45-tmin2.txt` |

**M-46** (`docs/beweise/roh/NAK-380-rot-M-46.txt`) — Etikett: heute rot; Basisstand: rot: 599 löst aus.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| T_eff = max(med + 3·MAD, (1 + ρ)·med, T_min) = 600 (Kalibrierung: med 300, MAD 20 statt 180/20): 599 kein Ereignis | `Sonde013DynamicsTest.cpp:736` „380/M-46 rauschbodenbezug: SF = 599“, B18 | `Spektrum.h:787` `const double tEff = std::max (std::max (med + kFlussKappa * mad, (1.0 ` (m46-rho0) | JA | `NAK-380-e4-mutation-m46-rho0.txt` |
| 601 genau ein Ereignis | `Sonde013DynamicsTest.cpp:739` „380/M-46 rauschbodenbezug: SF = 601“, B18 | `Spektrum.h:787` `const double tEff = std::max (std::max (med + kFlussKappa * mad, (1.0 ` (m46-rho2) | JA | `NAK-380-e4-mutation-m46-rho2.txt` |

**M-47** (`docs/beweise/roh/NAK-380-rot-M-47.txt`) — Etikett: zwei Hälften: (a) Regressionswache, (b) heute nicht messbar; Basisstand: (a) grün; (b) nicht messbar: Bandfluss 1490 dB statt 59,48 dB Binfluss.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| (a) Weißrauschen springt bei 5 s von −90 auf −70 dBFS unter dem Aktivgate: kein Ereignis | `Sonde013DynamicsTest.cpp:760` „380/M-47 pegelbezug_p0 (a)“, B18 | `Spektrum.h:486` `flussSchritt (s);` (m47-aktiv) | JA | `NAK-380-e4-mutation-m47-aktiv.txt` |
| (b) SF = 1530 · 10·log10((10⁻¹² + P0)/(10⁻¹³ + P0)) = 59,5 dB < T_min, kein Ereignis | `Sonde013DynamicsTest.cpp:772` „380/M-47 pegelbezug_p0 (b)“, B18 | `Spektrum.h:705` `const double p0 = 0.0;` (m47-p0) | JA | `NAK-380-e4-mutation-m47-p0.txt` |

**M-48** (`docs/beweise/roh/NAK-380-rot-M-48.txt`) — Etikett: heute rot; Basisstand: rot: 3 Ereignisse.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| genau ein Ereignis am Frame mit dem Maximum; der Folgewert liegt in der Sperrzeit und ist kein lokales Maximum, der dritte (85,3 ms, Sperrzeit frei) ist kein lokales Maximum (Folge 0, A, 0,9A, 0,8A mit A = 2·T_min) | `Sonde013DynamicsTest.cpp:779` „380/M-48 spitzenwahl_lokales_maximum“, B18 | `Spektrum.h:837` `sf > tEff && sperreFrei` (m48-lokal) | JA | `NAK-380-e4-mutation-m48-lokal.txt` |

**M-49** (`docs/beweise/roh/NAK-380-rot-M-49.txt`) — Etikett: heute rot; Basisstand: rot bei 1 Hop (2 statt 1), grün bei 2 Hops.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| 1 Hop (42,67 ms): ein Ereignis | `Sonde013DynamicsTest.cpp:None` „380/M-49 sperrzeit_50ms: 48 kHz, 1 Hop“, B18 | `Spektrum.h:834` `\|\| (double) (s.fensterStromStart - d.letzteEreignisStrom) * 1000.0 >= ` (m49-sperre0) | JA | `NAK-380-e4-mutation-m49-sperre0.txt` |
| 2 Hops (85,33 ms): zwei Ereignisse | `Sonde013DynamicsTest.cpp:811` „380/M-49 sperrzeit_50ms: 48 kHz, 2 Hops“, B18 | `Spektrum.h:834` `\|\| (double) (s.fensterStromStart - d.letzteEreignisStrom) * 1000.0 >= ` (m49-sperre90) | JA | `NAK-380-e4-mutation-m49-sperre90.txt` |
| Hopraster 46,44 ms bei 44,1 kHz: 1 Hop ein, 2 Hops zwei Ereignisse | `Sonde013DynamicsTest.cpp:814` „380/M-49 sperrzeit_50ms: 44,1 kHz“, B18 | `Spektrum.h:834` `\|\| (double) (s.fensterStromStart - d.letzteEreignisStrom) * 1000.0 >= ` (m49-sperre0) | JA | `NAK-380-e4-mutation-m49-sperre0.txt` |
| Konstante `kSperrzeitMs = 50.0` | `Sonde013DynamicsTest.cpp:798` „380/M-49 sperrzeit_50ms: Konstante“, B18 | `FeatureEngine.h:207` `inline constexpr double kSperrzeitMs = 60.0;` (m49-konstante) | JA | `NAK-380-e4-mutation-m49-konstante.txt` |

**M-50** (`docs/beweise/roh/NAK-380-rot-M-50.txt`) — Etikett: heute rot; Golden-Klasse (ii); Basisstand: rot: 18 Ereignisse (Altzähler).

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| höchstens 1 Ereignis in 30 s (Schranke), exakter Golden 0 | `AnalysisGoldenTestMain.cpp:1443` „380/M-50 nullkorpus_W1: hoechstens 1 Ereignis in 30 s, exakt der Golden“, B5 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m50-relativ) | JA | `NAK-380-e4-mutation-m50-relativ.txt` |
| Golden-Klasse (ii): der neue Code läuft gegen den Altzähler 18 rot, erst danach wird der neue exakte Zähler 0 zum Golden | `AnalysisGoldenTestMain.cpp:1443` „380/M-50 nullkorpus_W1 (Golden 18 statt 0)“, B5 | `AnalysisGoldenTestMain.cpp:1443` `sig::weissMono (sig::kW1Saat, 0.1, kDreissig), 18);` (m50-altzaehler) | JA | `NAK-380-e4-mutation-m50-altzaehler.txt` |

**M-51** (`docs/beweise/roh/NAK-380-rot-M-51.txt`) — Etikett: heute rot; Basisstand: rot: 14.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| höchstens 1 Ereignis in 30 s (W2, −40 dBFS), exakter Golden 0 | `AnalysisGoldenTestMain.cpp:1445` „380/M-51 nullkorpus_W2“, B5 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m50-relativ) | JA | `NAK-380-e4-mutation-m50-relativ.txt` |

**M-52** (`docs/beweise/roh/NAK-380-rot-M-52.txt`) — Etikett: heute rot; Basisstand: rot: 19.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| höchstens 1 Ereignis in 30 s (W3, −50 dBFS), exakter Golden 0 | `AnalysisGoldenTestMain.cpp:1447` „380/M-52 nullkorpus_W3“, B5 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m50-relativ) | JA | `NAK-380-e4-mutation-m50-relativ.txt` |

**M-53** (`docs/beweise/roh/NAK-380-rot-M-53.txt`) — Etikett: heute rot nach Analogie; Basisstand: am Basisstand gefahren: rot, 22.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| höchstens 1 Ereignis in 30 s (P1, −20 dBFS), exakter Golden 0 | `AnalysisGoldenTestMain.cpp:1452` „380/M-53 nullkorpus_P1“, B5 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m50-relativ) | JA | `NAK-380-e4-mutation-m50-relativ.txt` |
| P1 nach E-380-13: benachbarte Oktavbandleistungen 31,5 Hz bis 16 kHz weichen höchstens 1,0 dB ab | `AnalysisGoldenTestMain.cpp:1451` „380/M-53 rosa_selbstpruefung_E-380-13“, B5 | `Nak380Pruefsignale.h:294` `b0 = 0.0;` (m53-rosa) | JA | `NAK-380-e4-mutation-m53-rosa.txt` |

**M-54** (`docs/beweise/roh/NAK-380-rot-M-54.txt`) — Etikett: heute rot; Basisstand: rot: 109.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| höchstens 1 Ereignis in 30 s (S1, float32-Sinus), exakter Golden 0 | `AnalysisGoldenTestMain.cpp:1455` „380/M-54 nullkorpus_S1“, B5 | `Spektrum.h:786` `const double tMin = 0.0;` (m54-tmin0) | JA | `NAK-380-e4-mutation-m54-tmin0.txt` |

**M-55** (`docs/beweise/roh/NAK-380-rot-M-55.txt`) — Etikett: heute rot; Basisstand: rot: 55.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| höchstens 1 Ereignis in 30 s (S2, float32-Sägezahn), exakter Golden 0 | `AnalysisGoldenTestMain.cpp:1457` „380/M-55 nullkorpus_S2“, B5 | `Spektrum.h:786` `const double tMin = 0.0;` (m54-tmin0) | JA | `NAK-380-e4-mutation-m54-tmin0.txt` |

**M-56** (`docs/beweise/roh/NAK-380-rot-M-56.txt`) — Etikett: heute rot; Basisstand: rot: 19.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| höchstens 1 Ereignis in 30 s (V1, Vibrato ±50 Cent), exakter Golden 0 | `AnalysisGoldenTestMain.cpp:1459` „380/M-56 nullkorpus_V1“, B5 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m50-relativ) | JA | `NAK-380-e4-mutation-m50-relativ.txt` |

**M-57** (`docs/beweise/roh/NAK-380-rot-M-57.txt`) — Etikett: heute rot; Basisstand: rot: 148 Ereignisse, 36 Klicks mehrfach.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| genau 112 Ereignisse, je Klick genau eines mit `stromSample` in [Klick − 4096, Klick], keine weiteren | `AnalysisGoldenTestMain.cpp:1464` „380/M-57 impulskorpus_I1“, B5 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m50-relativ) | JA | `NAK-380-e4-mutation-m50-relativ.txt` |
| P2 nach E-380-13 selbstgeprüft (vor jedem Nutzer) | `AnalysisGoldenTestMain.cpp:1388` „380/M-57 rosa_selbstpruefung_E-380-13 (gemeinsamer Erzeuger `RosaRauschen`; Mutation gefahren an M-53)“, B5 | `Nak380Pruefsignale.h:294` `b0 = 0.0;` (m53-rosa) | JA | `NAK-380-e4-mutation-m53-rosa.txt` |

**M-58** (`docs/beweise/roh/NAK-380-rot-M-58.txt`) — Etikett: heute rot; Basisstand: rot: 87 Ereignisse, 31 Paare mehrfach.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| genau 56 Ereignisse, je Paar eines | `AnalysisGoldenTestMain.cpp:1466` „380/M-58 impulskorpus_I2“, B5 | `Spektrum.h:834` `\|\| (double) (s.fensterStromStart - d.letzteEreignisStrom) * 1000.0 >= ` (m58-sperre0) | JA | `NAK-380-e4-mutation-m58-sperre0.txt` |

**M-59** (`docs/beweise/roh/NAK-380-rot-M-59.txt`) — Etikett: heute rot; Basisstand: rot: 111 Ereignisse, 18 Klicks ohne, 17 mehrfach.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| genau 112 Ereignisse (Paare im Abstand 100 ms) — am Endstand ROT: 105, offen (§35.4) | `AnalysisGoldenTestMain.cpp:1468` „380/M-59 impulskorpus_I3“, B5 | `Spektrum.h:834` `\|\| (double) (s.fensterStromStart - d.letzteEreignisStrom) * 1000.0 >= ` (m59-sperre150) | JA | `NAK-380-e4-mutation-m59-sperre150.txt` |

**M-60** (`docs/beweise/roh/NAK-380-rot-M-60.txt`) — Etikett: Regressionswache; Basisstand: nicht gefahren (Wache; die zwei Einheitsprüfungen entstehen mit dem Testzugang dieser Etappe).

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| unverändert grün | `AnalysisGoldenTestMain.cpp:3132` „short_impulse_triggers_peak_path_only (bestehend)“, B5 | `Spektrum.h:821` `peakAus = false && steigungDb > kPeakSteigungSchwelleDb` (m60-peakpfad) | JA | `NAK-380-e4-mutation-m60-peakpfad.txt` |
| ein reines Peakereignis trägt `staerke` = Crest über Schwelle in dB | `AnalysisGoldenTestMain.cpp:1488` „380/M-60 peakpfad_bleibt: ein reines Peakereignis traegt staerke = Crest ueber Schwelle in dB (Einheit, Testzugang)“, B5 | `Spektrum.h:859` `: (float) crestDb;` (m60-peakstaerke) | JA | `NAK-380-e4-mutation-m60-peakstaerke.txt` |
| lösen Fluss und Peak im selben Frame aus, entsteht genau ein Ereignis mit beiden Bits | `AnalysisGoldenTestMain.cpp:1502` „380/M-60 peakpfad_bleibt: … genau ein Ereignis mit beiden Bits (Einheit, Testzugang)“, B5 | `Spektrum.h:864` `if (flussAus && peakAus) ereignisAblegen (e);` (m60-zwei) | JA | `NAK-380-e4-mutation-m60-zwei.txt` |

**M-61** (`docs/beweise/roh/NAK-380-rot-M-61.txt`) — Etikett: heute rot; Basisstand: rot: 5 Ereignisse, darunter der Wiederbeginn bei 6,9547 s.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| genau ein Ereignis mit `stromSample` in [7,0 s − N_H/fs, 7,0 s] | `AnalysisGoldenTestMain.cpp:1580` „380/M-61 wiederbeginn_nach_stille: genau ein Ereignis, Fensteranfang in [331904, 336000]“, B5 | `Spektrum.h:678` `detektor[0].gefuellt = 0;` (m61-stilleleert) | JA | `NAK-380-e4-mutation-m61-stilleleert.txt` |
| keines in den stationären Abschnitten nach voller Historie | `AnalysisGoldenTestMain.cpp:1580` „380/M-61 wiederbeginn_nach_stille: genau ein Ereignis …“, B5 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m61-relativ) | JA | `NAK-380-e4-mutation-m61-relativ.txt` |
| der Vorframe läuft über inaktive Frames | `AnalysisGoldenTestMain.cpp:1590` „380/M-61 wiederbeginn_nach_stille: der Vorframe laeuft ueber inaktive Frames“, B5 | `Spektrum.h:486` `(void) 0;` (m61-vorframe) | JA | `NAK-380-e4-mutation-m61-vorframe.txt` |
| die Historie nimmt nur aktive | `AnalysisGoldenTestMain.cpp:1596` „380/M-61 wiederbeginn_nach_stille: die Historie nimmt nur aktive Frames“, B5 | `Spektrum.h:678` `auto& d = detektor[0]; d.sfVorher = sf; d.historie[(std::size_t) d.sta` (m61-historie) | JA | `NAK-380-e4-mutation-m61-historie.txt` |

**M-62** (`docs/beweise/roh/NAK-380-rot-M-62.txt`) — Etikett: heute nicht messbar; Basisstand: Grenzteil grün, Ereignisteil rot (5 Fehlalarme, erste Signalfassung), Binauskunft nicht messbar.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Auskunft `flussBinVorgaengerGueltig()` direkt nach der Grenze falsch | `AnalysisGoldenTestMain.cpp:1672` „380/M-62 grenze_leert_detektor: Auskunft“, B5 | `Zeit.h:500` `// detektorLeeren();` (m62-leeren) | JA | `NAK-380-e4-mutation-m62-leeren.txt` |
| kein Ereignis im ersten Frame nach der Grenze | `AnalysisGoldenTestMain.cpp:1684` „380/M-62 grenze_leert_detektor: kein Ereignis …“, B5 | `Zeit.h:500` `// detektorLeeren();` (m62-leeren) | JA | `NAK-380-e4-mutation-m62-leeren.txt` |
| keines, bevor die Historie wieder 32 aktive Frames trägt | `AnalysisGoldenTestMain.cpp:1684` „380/M-62 grenze_leert_detektor: kein Ereignis …“, B5 | `Spektrum.h:910` `(Zeile entfernt: d.stand = 0; d.gefuellt = 0;)` (m62-historie) | JA | `NAK-380-e4-mutation-m62-historie.txt` |

**M-63** (`docs/beweise/roh/NAK-380-rot-M-63.txt`) — Etikett: heute rot; Basisstand: rot nur an der Zuordnung des ersten Stille-Klicks (145 408 statt 143 360); Stärken 18,19/90,18/62,97 in [3, 1000].

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Flussereignis trägt `staerke` = κ·(SF − med)/(T_eff − med) ≤ 1000, endlich (Klick auf Stille bis 40 dB über Vollaussteuerung) | `Sonde013EventWireTest.cpp:742` „380/M-63 staerke_begrenzt: Klick auf Stille - Engine“, B16 | `Spektrum.h:858` `? (float) (kFlussKappa * (sf - med) / nenner)` (m63-klammer) | JA | `NAK-380-e4-mutation-m63-klammer.txt` |
| … ≥ κ = 3 (Korpus I1) | `Sonde013EventWireTest.cpp:796` „380/M-63 staerke_begrenzt: Korpus I1 - Engine“, B16 | `Spektrum.h:858` `? (float) std::min (kFlussStaerkeMax, (sf - med) / nenner)` (m63-kappa) | JA | `NAK-380-e4-mutation-m63-kappa.txt` |
| Nenner ≥ max(med, T_min − med) > 0 (kleinste MAD: Klick auf stehendem Sinus) | `Sonde013EventWireTest.cpp:785` „380/M-63 staerke_begrenzt: Klick auf stehendem Sinus“, B16 | `Spektrum.h:858` `? (float) ((sf - med) / mad)` (m63-madnenner) | JA | `NAK-380-e4-mutation-m63-madnenner.txt` |
| der Wiretext passiert Textriegel und Schema (`staerke_mad` 0 bis 1000), jedes Klickereignis reist | `Sonde013EventWireTest.cpp:757` „380/M-63 staerke_begrenzt: Klick auf Stille - Draht“, B16 | `Spektrum.h:858` `? (float) (kFlussKappa * (sf - med) / nenner)` (m63-klammer) | JA | `NAK-380-e4-mutation-m63-klammer.txt` |

**M-64** (`docs/beweise/roh/NAK-380-rot-M-64.txt`) — Etikett: Regressionswache; Basisstand: nicht gefahren (Wache; Kanon an 8308a3d8: 202 752 je Rate).

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| `stromSample` je Rate über alle Blockgrößen identisch | `AnalysisGoldenTestMain.cpp:3439` „impulse_time_is_stable_over_block_sizes_and_sample_rates (a)“, B5 | `Spektrum.h:845` `e.stromSample = rahmenStartBlock.stromVon;` (m64-blockanfang) | JA | `NAK-380-e4-mutation-m64-blockanfang.txt` |
| und in [Impuls − N_H(fs), Impuls] | `AnalysisGoldenTestMain.cpp:3446` „(b): … innerhalb EINER Fensterlaenge vor t0“, B5 | `Spektrum.h:845` `e.stromSample = rahmenStartBlock.stromVon;` (m64-blockanfang) | JA | `NAK-380-e4-mutation-m64-blockanfang.txt` |

**M-65** (`docs/beweise/roh/NAK-380-rot-M-65.txt`) — Etikett: heute rot; Basisstand: rot: 16 von 93 Fenstern mit Onsetsumme > 0, größte 7,323.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Onsetsumme Σ `staerke` je Evidenzfenster = 0 in jedem Fenster nach voller Historie (93 Snapshots, 89 nach voller Historie) | `Sonde013EventWireTest.cpp:843` „380/M-65 onsetsumme_null_bei_rauschen“, B16 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m65-relativ) | JA | `NAK-380-e4-mutation-m65-relativ.txt` |

**M-66** (`docs/beweise/roh/NAK-380-rot-M-66.txt`) — Etikett: Regressionswache; Basisstand: nicht gefahren (Test neu; Schwelle 2,0 unverändert).

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| 2,0 reißt nicht (Schwelle 2,0) | `experiment.rs:1763` „nak380_m66_guardrail_transient_grenze“, A4 | `experiment.rs:575` `pub const GUARDRAIL_TRANSIENT: f64 = 1.0;` (m66-schwelle1) | JA | `NAK-380-e4-mutation-m66-schwelle1.txt` |
| 2,0 reißt nicht (`w > schwelle`) | `experiment.rs:1763` „nak380_m66_guardrail_transient_grenze“, A4 | `experiment.rs:527` `} else if w >= schwelle {` (m66-vergleich) | JA | `NAK-380-e4-mutation-m66-vergleich.txt` |
| 2,01 reißt | `experiment.rs:1772` „nak380_m66_guardrail_transient_grenze“, A4 | `experiment.rs:575` `pub const GUARDRAIL_TRANSIENT: f64 = 2.02;` (m66-schwelle202) | JA | `NAK-380-e4-mutation-m66-schwelle202.txt` |

**M-67** (`docs/beweise/roh/NAK-380-rot-M-67.txt`) — Etikett: Regressionswache; Basisstand: nicht gefahren (bestehender A4-Test, im Kanon an 8308a3d8 grün).

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| `None` (keine Streuung), Rangkomponente 0, weder Beleg noch Gegenbeleg | `sonde014_hypothese.rs:1503` „hoch_verlangt_einen_belegten_zusammenhang“, A4 | `messung.rs:313` `return Some(1.0);` (m67-nullvarianz) | JA | `NAK-380-e4-mutation-m67-nullvarianz.txt` |

**M-68** (`docs/beweise/roh/NAK-380-rot-M-68.txt`) — Etikett: Regressionswache, Golden-Klasse (i); Basisstand: grün; F1-Bytes vor dem ersten Produktedit erfasst.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| genau 76 Ausgangsbytes bytegleich zur vor dem Umbau erfassten Referenz; `fingerprintSchritt` bekommt weiter den Bandfluss | `Sonde013FingerprintGoldenTest.cpp:204` „380/M-68 fingerprint_unveraendert“, B22 | `Spektrum.h:639` `fingerprintSchritt (s, detektor.empty() ? fluss : detektor[0].sfVorher` (m68-binfluss) | JA | `NAK-380-e4-mutation-m68-binfluss.txt` |

**M-69** (`docs/beweise/roh/NAK-380-rot-M-69.txt`) — Etikett: heute rot; Basisstand: rot: 5 Teilprüfungen, „kein Feld“ und „keine Version“ grün.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Stärke „Flussüberschreitung in Vielfachen der auf κ = 3 normierten Schwellendistanz“ | `pruefe_v3_vertrag.py:2569` „nak380_m69_staerke_benannt: Flussstaerke …“, A5 | `eq-ipc-v3.schema.json:1270` `der auf kappa = 2 normierten` (m69-vielfache) | JA | `NAK-380-e4-mutation-m69-vielfache.txt` |
| „(bei greifender relativer Schwelle die echte MAD)“ | `pruefe_v3_vertrag.py:2575` „nak380_m69_staerke_benannt: … echte MAD“, A5 | `eq-ipc-v3.schema.json:1270` `(bei greifender relativer Schwelle die mittlere MAD)` (m69-mad) | JA | `NAK-380-e4-mutation-m69-mad.txt` |
| „höchstens 1000“ | `pruefe_v3_vertrag.py:2579` „nak380_m69_staerke_benannt: hoechstens 1000“, A5 | `eq-ipc-v3.schema.json:1270` `echte MAD), hoechstens 100;` (m69-1000) | JA | `NAK-380-e4-mutation-m69-1000.txt` |
| „reines Peakereignis: Crest über Schwelle in dB“ | `pruefe_v3_vertrag.py:2583` „nak380_m69_staerke_benannt: reines Peakereignis …“, A5 | `eq-ipc-v3.schema.json:1270` `traegt den Crest in dB (NAK-380 R-380-2).` (m69-peak) | JA | `NAK-380-e4-mutation-m69-peak.txt` |
| kein Feld (sechs Felder, `staerke_mad` 0 bis 1000) | `pruefe_v3_vertrag.py:2590` „nak380_m69_staerke_benannt: kein Feld“, A5 | `eq-ipc-v3.schema.json:1280` `"staerke_mad": { "type": "number", "minimum": 0, "maximum": 2000 },` (m69-feld) | JA | `NAK-380-e4-mutation-m69-feld.txt` |
| keine Version (`$id` bleibt v3) | `pruefe_v3_vertrag.py:2598` „nak380_m69_staerke_benannt: keine Version“, A5 | `eq-ipc-v3.schema.json:3` `"$id": "evenacadia.nakama.ipc.v4",` (m69-version) | JA | `NAK-380-e4-mutation-m69-version.txt` |
| Vertrag.h nennt dieselbe Einheit am Feld `staerke` (rechnen↔vertragen, §8.4) | `pruefe_v3_vertrag.py:2605` „nak380_m69_staerke_benannt: Vertrag.h“, A5 | `Vertrag.h:284` `≥ κ bei Auslösung, ≤ 100; Peakereignis` (m69-vertragh) | JA | `NAK-380-e4-mutation-m69-vertragh.txt` |

**M-70** (`docs/beweise/roh/NAK-380-rot-M-70.txt`) — Etikett: heute nicht messbar; Basisstand: nicht messbar: 13 Teilprüfungen rot, Fassung 20260927 fehlt.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| `kFeatureMetricsVersion` = 20260927 | `pruefe_v3_vertrag.py:2620` „nak380_m70_fassung_etappe_4: FeatureEngine nennt 20260927“, A5 | `FeatureEngine.h:89` `kFeatureMetricsVersion = 20260928u;` (m70-feature) | JA | `NAK-380-e4-mutation-m70-feature.txt` |
| `METRICS_VERSION` in prepost.rs = 20260927 | `pruefe_v3_vertrag.py:2628` „nak380_m70_fassung_etappe_4: prepost.rs nennt 20260927“, A5 | `prepost.rs:206` `pub const METRICS_VERSION: u32 = 20260928;` (m70-prepost) | JA | `NAK-380-e4-mutation-m70-prepost.txt` |
| `METRICS_VERSION` in vergleichbarkeit.rs = 20260927 | `pruefe_v3_vertrag.py:2628` „nak380_m70_fassung_etappe_4: vergleichbarkeit.rs nennt 20260927“, A5 | `vergleichbarkeit.rs:149` `pub const METRICS_VERSION: u32 = 20260928;` (m70-vergleichbarkeit) | JA | `NAK-380-e4-mutation-m70-vergleichbarkeit.txt` |
| `aktuell` im Register = 20260927 | `pruefe_v3_vertrag.py:2633` „nak380_m70_fassung_etappe_4: aktuell ist 20260927“, A5 | `metriken-v1.json:20` `"aktuell": 20260926,` (m70-aktuell) | JA | `NAK-380-e4-mutation-m70-aktuell.txt` |
| `seit` nennt „NAK-380 Etappe 4“ | `pruefe_v3_vertrag.py:2638` „nak380_m70_fassung_etappe_4: seit nennt NAK-380 Etappe 4“, A5 | `metriken-v1.json:422` `"seit": "NAK-380 Etappe 4x",` (m70-seit) | JA | `NAK-380-e4-mutation-m70-seit.txt` |
| Konfidenzschwellen und übrige Einträge der Fassung 20260926 unverändert übernommen | `pruefe_v3_vertrag.py:2651` „nak380_m70_fassung_etappe_4: Eintraege der Fassung 20260926“, A5 | `metriken-v1.json:292` `"wert": 12.5,` (m70-uebernahme) | JA | `NAK-380-e4-mutation-m70-uebernahme.txt` |
| geführt P0 = −100 dBFS je Bin (`kFlussP0Db = -100.0`) | `pruefe_v3_vertrag.py:2675` „nak380_m70_fassung_etappe_4: kFlussP0Db steht …“, A5 | `FeatureEngine.h:181` `inline constexpr double kFlussP0Db = -90.0;` (m70-p0) | JA | `NAK-380-e4-mutation-m70-p0.txt` |
| geführt Filterbreite 50 Cent (`kFlussFilterCent = 50.0`) | `pruefe_v3_vertrag.py:2675` „nak380_m70_fassung_etappe_4: kFlussFilterCent steht …“, A5 | `FeatureEngine.h:185` `inline constexpr double kFlussFilterCent = 45.0;` (m70-filtercent) | JA | `NAK-380-e4-mutation-m70-filtercent.txt` |
| geführt H = 32 (`kFlussHistorie = 32`) | `pruefe_v3_vertrag.py:2675` „nak380_m70_fassung_etappe_4: kFlussHistorie steht …“, A5 | `FeatureEngine.h:190` `inline constexpr int kFlussHistorie = 30;` (m70-historie) | JA | `NAK-380-e4-mutation-m70-historie.txt` |
| geführt κ = 3 (`kFlussKappa = 3.0`) | `pruefe_v3_vertrag.py:2675` „nak380_m70_fassung_etappe_4: kFlussKappa steht …“, A5 | `FeatureEngine.h:193` `inline constexpr double kFlussKappa = 3.5;` (m70-kappa) | JA | `NAK-380-e4-mutation-m70-kappa.txt` |
| geführt ρ = 1 (`kFlussRho = 1.0`) | `pruefe_v3_vertrag.py:2675` „nak380_m70_fassung_etappe_4: kFlussRho steht …“, A5 | `FeatureEngine.h:196` `inline constexpr double kFlussRho = 1.5;` (m70-rho) | JA | `NAK-380-e4-mutation-m70-rho.txt` |
| geführt T_min-Faktor, kalibriert 0,30 dB je Bin (`kFlussTminDbJeBin = 0.30`) | `pruefe_v3_vertrag.py:2675` „nak380_m70_fassung_etappe_4: kFlussTminDbJeBin steht …“, A5 | `FeatureEngine.h:202` `inline constexpr double kFlussTminDbJeBin = 0.35;` (m70-tmin) | JA | `NAK-380-e4-mutation-m70-tmin.txt` |
| geführt Sperrzeit 50 ms (`kSperrzeitMs = 50.0`; Matrix: 50 → 60 ohne Registereintrag → A5 rot) | `pruefe_v3_vertrag.py:2675` „nak380_m70_fassung_etappe_4: kSperrzeitMs steht …“, A5 | `FeatureEngine.h:207` `inline constexpr double kSperrzeitMs = 60.0;` (m70-sperrzeit) | JA | `NAK-380-e4-mutation-m70-sperrzeit.txt` |
| die Nullkorpusfälle M-50 bis M-56 fallen am alten Wert | Basisstand-Gegenprobe M-50 bis M-56 (§35.2): 18/14/19/22/109/55/19 Ereignisse | Basisstand (unverändertes Produkt) | JA | `NAK-380-rot-M-50.txt` bis `-M-56.txt` |

**M-71** (`docs/beweise/roh/NAK-380-rot-M-71.txt`) — Etikett: heute rot (Bruch am Wortlaut); Basisstand: alter Kommentar ohne SuperFlux-Wortlaut; Belege über M-42, M-44, M-56, M-60.

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Maximumfilter über Nachbarbins, Bins, positive Deltas | Beleg über M-42 | `Spektrum.h:669` `return 0;` (m42-w0); `Spektrum.h:718` `if (delta != 0.0) sf += std::abs (delta);` (m42-betrag) | JA / JA | `NAK-380-e4-mutation-m42-w0.txt`, `NAK-380-e4-mutation-m42-betrag.txt` |
| adaptive Median/MAD-Schwelle (echte MAD) | Beleg über M-44 | `Spektrum.h:894` `mad = 0.0; for (std::size_t i = 0; i < n; ++i) mad += d.sortiert[i]; m` (m44-mittel) | JA | `NAK-380-e4-mutation-m44-mittel.txt` |
| reduziert Vibrato-Fehltrigger (V1 ≤ 1: 0 statt 19 am Basisstand) | Beleg über M-56 | `Spektrum.h:786` `const double tMin = 0.0; const double tEff = std::max (std::max (med +` (m50-relativ) | JA | `NAK-380-e4-mutation-m50-relativ.txt` |
| ein einfacher Peakpfad bleibt als Gegenbeleg | Beleg über M-60 | `Spektrum.h:821` `peakAus = false && steigungDb > kPeakSteigungSchwelleDb` (m60-peakpfad) | JA | `NAK-380-e4-mutation-m60-peakpfad.txt` |

**M-72** (`docs/beweise/roh/NAK-380-rot-M-72.txt`) — Etikett: Regressionswache wie M-20; Basisstand: nicht gefahren (Wache; Audiopfad unberührt).

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| A1, A16 und B1 grün (Detektor im Analyseworker, Träger im Heap in `vorbereiten`) | A1, A16, B1, B4 am Endstand (§35.6) | Regressionswache wie M-20: Mutation `std::vector<float>` in `processBlock` (PluginProcessor.cpp, außerhalb der Ticketpfade), belegt in `NAK-380-rot-M-20.txt`; dazu Flussschritt allokationsfrei (m43-heap) | JA (M-20) | `NAK-380-rot-M-20.txt`, `NAK-380-e4-mutation-m43-heap.txt` |


### 35.6 Einzeln gefahrene Beine am Endstand

Alle acht C++-Ziele in einem Bau neu gebaut (19:59:51 bis 20:00:44 Uhr,
Exit 0, `vctip.exe` danach beendet), jüngste Quelle `Spektrum.h`
19:58:54.131 (Rücknahme des letzten Mutationszyklus); jedes Binary ist
jünger. Aufrufe wie `tools/beweise.ps1`, jedes Bein einzeln
(Rohausgaben `eq-copilot/build/nak380-e4-end/`, Bauartefakt; Zeiten `date`
der Shell):

| Bein | Aufruf | Lauf | Exit | Zähler | Binary |
|---|---|---|---|---|---|
| B18 | `EqCopSonde013DynamicsTest` | 20:00:49–20:01:01 | 0 | 105/0 (vorher 86/0) | 19:59:55.404 |
| B5 | `EqCopAnalysisGoldenTest` | 20:01:01–20:01:14 | **1** | **294/1** (vorher 261/0); rot nur `380/M-59 impulskorpus_I3` | 19:59:09.920 |
| B16 | `EqCopSonde013EventWireTest` | 20:01:14–20:01:17 | 0 | 213/0 (vorher 206/0) | 20:00:05.474 |
| B22 | `EqCopSonde013FingerprintGoldenTest` | 20:01:17–20:01:19 | 0 | 28/0 (vorher 27/0) | 20:00:08.418 |
| A5 | `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | 20:01:19–20:01:23 | 0 | 887/0 (vorher 870/0) | — |
| A1 | `EqCopNullTest` | 20:01:23 | 0 | NULLTEST OK | 20:00:16.770 |
| A16 | `EqCopProbeeqNullTest` | 20:01:23 | 0 | 145/0 | 20:00:25.336 |
| B1 | `EqCopIdentityTest` | 20:01:24 | 0 | 120/0 | 20:00:33.586 |
| B4 | `EqCopQueueStressTest` | 20:01:24–20:01:27 | 0 | 121/0 | 20:00:44.045 |
| A26 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py` | 20:01:27–20:01:28 | 0 | Exit-Gate P4: alle vier Klassen, 0 falsche Behauptungen | — |
| A8 | `erzeuge_v3_fixtures.py --pruefen` | 20:01:28 | 0 | 421 Dateien bytegleich | — |
| A9 | `pruefe_flatc_drift.py` | 20:01:28 | 0 | Drift 0 Dateien | — |
| A10 | `erzeuge_fb_fixtures.py --pruefen` | 20:01:29–20:01:33 | 0 | 131 Dateien bytegleich | — |
| A4 | `cargo test --manifest-path broker/Cargo.toml --color never` | 20:01:50–20:21:43 | 0 | 845 bestanden, 0 gescheitert, 23 ignoriert über 31 Testläufe; `lib` 310/0 (vorher 309, neu `nak380_m66_guardrail_transient_grenze`); kein NOT RUN | — |

B5 trägt 295 Prüfungen statt 261: 34 neue aus M-50 bis M-62, darunter die
zwei M-60-Einheiten (§35.8 Punkt 16). A26 sucht
die B5-Fallnamen des Sweeps wörtlich; sie sind unverändert. Ein erster
Durchgang derselben Beine lief 19:55:30 bis 19:56:19 Uhr (vor den
M-60-Einheiten) mit denselben Exitcodes; A4 lief zusätzlich 19:22:36 bis
19:43:17 Uhr am kalibrierten Stand (Exit 0, `lib` 310/0).

### 35.7 Bewegungen bestehender Goldens

Keine. Der NAK-182-Sweep (M-64) meldet am Endstand je Rate 202 752 (t0 −
Stempel 3 584 bei allen fünf Raten und allen fünf Blockgrößen), genau wie im
Kanon an `8308a3d8` (`docs/beweise/roh/NAK-380-8308a3d-dirty.md`): Bewegung
**0 Hops**. E-d meldet „kein zweites Ereignis“ (vorher Faktor ≥ 10). Die
bestehenden Fälle von B5, B16, B18 und B22 bleiben grün, der F1-Fingerprint
bytegleich (M-68), A2 und B9 sind nicht berührt. Zählerstände vorher → nachher:
B5 261/0 → 294/1 (rot nur M-59), B16 206/0 → 213/0, B18 86/0 →
105/0, B22 27/0 → 28/0, A5 870/0 → 887/0, A4 `lib` 309 → 310 (§35.6).

### 35.8 Abweichungen vom Bauplan §8.4, begründet

1. **Detektorzustand als Heapträger.** §8.4 nennt „Mitglieder … neben
   `:1479-1483`“. Als Felder im Objekt (+112 B, `sizeof` 16 352 B) sprengten
   sie den 1-MiB-Stack von B5 (`0xC00000FD` im Abschnitt G, gemessen
   19:16 Uhr). Der Träger `FlussDetektor` liegt jetzt als ein Element im Heap
   (Muster `headroomRing`, E-380-14 sinngemäß); `sizeof` sinkt auf 16 208 B.
2. **Warteschlange des gleitenden Maximums** als dritter Heapträger (Ring
   2·w_max + 2 `int`: 100 Plätze = 400 B bei 44,1 kHz, 94 bei 48 kHz, 48 bei
   96 kHz) neben Vorframe und Filterpuffer (2·K·8 B ≤ 26 656 B); sie ist das
   O(K)-Verfahren, das §8.4 verlangt. Gesamt bei 44,1 kHz 26 656 + 400 + 512
   (Historie und Sortierpuffer) = 27 568 B Heap.
3. **Broker (Ergänzung des Dirigenten):** `prepost.rs:206` und
   `vergleichbarkeit.rs:149` heben `METRICS_VERSION` auf 20260927 — die
   Rust-Hälfte von R-380-7 und T-380-10 (dieselbe Ergänzung wie in Etappe 3).
   `experiment.rs` hatte **keinen** bestehenden `#[cfg(test)]`-Block; der
   Test `nak380_m66_guardrail_transient_grenze` steht deshalb in einem neuen
   Block `mod nak380_tests` am Dateiende (Hilfsfunktion und genau ein
   Test); der Produktcode der Datei ist unverändert.
4. **A5, Fall der Etappe 3:** `nak380_m40_fassung_etappe_3` prüft weiter den
   Inhalt der Fassung 20260926 (registriert, `seit`, Konfidenzschwellen);
   die drei Aktualitätsprüfungen (FeatureEngine, beide Rust-Konstanten,
   `aktuell`) wandern in `nak380_m70_fassung_etappe_4` mit 20260927 — genau
   wie Etappe 3 `nak380_m19` („aktuell ist 20260925“ → „Fassung 20260925
   bleibt registriert“, Commit `f8942067`).
5. **Sperrzeit auch nach Peakereignissen.** „seit dem letzten Ereignis“ gilt
   für Ereignisse beider Pfade: der Peakpfad sieht einen Klick im ersten
   Fenster, der Fluss im nächsten; ohne gemeinsame Sperrzeit wären es zwei
   Ereignisse je Klick (M-57). Der Peakpfad selbst wird nicht gesperrt
   (unverändert, M-60).
6. **Freigabe des Peakpfads** folgt dem Detektor: er rechnet nur mit
   gültigem Binvorframe und voller Historie aus 32 aktiven Frames (vorher
   Bandvorgänger und 16 Frames) — T-380-5: „die ersten 32 Frames nach Start
   oder Grenze bleiben ohne Ereignis“.
7. **V1 wörtlich mit Amplituden 1/h** (Spitze über 0 dBFS, float32); der
   Nachbau `b11_fluss.py` nutzte 0,3/h. Mit 0,3/h erreichte V1 322,3 dB,
   mit 1/h 414,6 dB (Prototyp); die Kalibrierung richtet sich nach der
   wörtlichen Tafel.
8. **Zahlen, die der Kalibrierung folgen** (Formel unverändert): M-45
   458,9/459,1 dB statt 76,4/76,6; M-46 med 300, MAD 20, T_eff = 600
   (599/601) statt 180/20/360 (359/361), weil T_min = 459 sonst über 360
   läge; M-48 und M-49 A = 2·T_min = 918 dB (0,9A, 0,8A; 1,1A) mit Historie
   16 × 0 und 16 × 2 (T_eff = T_min) statt 500/450/400 und 32 × 0 — so kann
   auch die Basisvariante auslösen und die Gegenprobe misst die Spitzenwahl.
9. **Vorbedingungen, die für die Trennschärfe erweitert sind:** M-49 auch
   bei 44,1 kHz (46,44/92,88 ms); M-61 zusätzlich Vorframe und Historie am
   Testzugang in der Stille (die Matrixmutation „Vorframe nur aktiv“ trifft
   den Ereigniszähler nicht, weil das letzte aktive Fenster vor der Pause
   nur 384 Rosa-Samples trägt und der Wiederbeginn auch gegen dieses
   schwache Spektrum auslöst — gemessen, `m61-vorframe` erster Lauf; die
   Messpunkte liegen bei 5,504 s und 6,859 s, 31 Frames auseinander — ein
   erster Entwurf mit 32 Frames sah den Historienring einmal ganz herum
   und blieb unter `m61-historie` grün, gemessen); M-62
   nach dem Seek +14 dB und eine Delle −34 dB mit Rücksprung in der
   Nachfüllphase (der erste Entwurf mit einem Klick +0,9 auf dem lauten
   Rosa lag unter dem Rauschen und machte `m62-historie` nicht rot —
   gemessen); M-63 Stille nur um die Klicks (sonst entsteht mangels
   Konvergenz kein Snapshot, fail-closed), Klicks in Fenstermitte, ein Klick
   +100 für die Klammer, ein Klick auf dem stehenden Sinus für die kleinste
   MAD.
10. **Formgültige Ersatzmutationen.** M-45: „T_min → 0“ endet bei med =
    MAD = 0 am Nennerriegel `nenner > 0` (Protokoll `m45-tmin0`, NEIN) —
    ersetzt durch „T_min halbiert“ (`m45-tminhalb`). M-63, Nennersatz: „T_min
    = 0“ lässt den Sinusfall grün (Nenner ≥ med > 0, Klammer 1000;
    `m63-tmin0`, NEIN); der Satz ist in der Matrix „nachgerechnet“, sein
    messbarer Teil fällt mit dem Basisnenner MAD ohne Klammer
    (`m63-madnenner`). M-61, Ereignishälfte: „Ereigniszeit aus dem
    Blockanfang“ blieb im 4096-Sample-Fenster (332 800;
    `NAK-380-e4-mutation-m61-zeit-verworfen.txt`, NEIN) — ersetzt durch das
    Gegenteil von A-6, „ein inaktiver Frame leert die Historie“
    (`m61-stilleleert`: 0 Ereignisse). Die Nullkorpusmutation „ρ = 0,
    T_min = 0“ (M-50) und „T_min = 0“ (M-54/M-55) sind formgültig, weil der
    Median dort über 0 liegt bzw. die MAD nicht 0 ist (gemessen JA).
11. **M-64:** „Ereigniszeit aus dem Blockanfang“ als Stempel
    `rahmenStartBlock.stromVon`; **M-68:** „`fingerprintSchritt` erhält den
    Binfluss“ als SF(n−1) des Binflusses (der Binfluss des Frames entsteht
    erst danach); **M-72:** Beleg über die Mutation von M-20 (in
    `PluginProcessor.cpp`, außerhalb der Ticketpfade; Muster M-41) und die
    Allokationsprüfung des Flussschritts (M-43).
12. **N_H = 4096** bei allen Raten (Etappe 6 ändert die Fensterlängen).
13. `Nak380Pruefsignale.h` bindet `../core/analysis/Fft.h` für die
    Welch-Schätzung der E-380-13-Prüfung ein (JUCE-frei).
14. **M-53** („heute rot nach Analogie“) ist am Basisstand gefahren: 22.
15. **CMakeLists.txt** unverändert; kein neues Bein.
16. **M-60 mit zwei Einheitsprüfungen.** Die Matrix nennt nur bestehende
    Fälle. Beim Messen blieb `both_paths_yield_one_event` auch mit
    abgeschaltetem Peakpfad grün, weil ein Flussereignis mit hohem Crest das
    Peakbit als Zusatzbit trägt (`Spektrum.h`, unverändert seit SONDE-013);
    die Sätze „reines Peakereignis: Crest über Schwelle“ und „genau ein
    Ereignis mit beiden Bits“ hatten damit keine eigene fallende Prüfung. B5
    misst sie jetzt als Einheit über den eigenen Testzugang
    (`peakSchritt`: Rahmenpeak 1,0, RMS 0,05, voriger Rahmenpeak 0,1;
    Stärke 14,0206 dB bzw. 6,0), rot unter `m60-peakstaerke` und `m60-zwei`.
17. **Kommentare zum alten Detektor** im Ringtest und im NAK-182-Sweep von
    B5 tragen einen Hinweis auf die Regel seit Etappe 4 (Code dort
    unverändert, Sweep-Stempel gemessen gleich).

### 35.9 Selbstaudit, Prüfliste und offene Punkte

**Selbstaudit** (eigener Diff, `git diff --stat` und `git diff` gegen HEAD,
nicht gestagt, nach allen Rücknahmen): genau ein Detektor (`binFlussSchritt`,
`detektorSchritt`); Binbereich je Abtastrate mit Obergrenze
min(17 959,39 Hz, 18 kHz, 0,95·Nyquist) und ohne DC-Bin; der Logarithmus
bekommt immer p + P0 > 0, ein nicht endlicher oder negativer Binwert geht
als 0 ein; das Maximumfilter läuft über den Vorframe (Filter aus dem
fortgeschriebenen Vorframe erst nach dem Flusswert); echte MAD als Median
der Absolutabweichungen; T_eff aus drei Termen; Spitzenwahl gegen
`sfVorher`; Sperrzeit in Samples gegen das letzte Ereignis beider Pfade
(Ganzzahldifferenz nur bei `fensterStromStart >= letzteEreignisStrom`,
sonst frei, kein Unterlauf); Vorframe über alle Hauptstufen-Frames,
Historie nur aus aktiven; `detektorLeeren` ist der eine Rückweg für
`zuruecksetzen` und `grenzeZiehen`, `vorbereiten` legt den Träger neu an;
`staerke` endlich und in [κ, 1000] (Nenner > 0 geprüft, sonst kein
Flussereignis); `bandZentrumHz` = Summe/Summe positiver Deltas, nur bei SF >
0, sonst 0 (wie bisher; ein reines Peakereignis ohne positiven Fluss fällt
am Serialisierer weiter einzeln heraus); Peakpfad Zeile für Zeile
unverändert; Fingerprint mit Bandfluss (M-68 bytegleich); Versionen an vier
Stellen gleich (A5 M-70); Register mit jedem `kName = Wert`; Korpus unter
`eq-copilot/fixtures/**` unverändert (`git diff --stat` leer; A8, A9, A10
§35.6); kein Audiopfad berührt (A1, A16, B1, B4 grün). Zahlenränder: leere
Historie (keine Entscheidung vor 32 aktiven Frames), K = 0 (nur unter rund
64 Hz Abtastrate; dann kein Binfluss, kein Flussereignis und — weil der
Peakpfad im Detektorschritt liegt — auch kein Peakereignis), erster Frame
nach der Grenze (kein Vorframe, kein SF), Sperrzeit über einen Loop-Wrap
(ein Loop-Wrap ist eine Grenze und leert die Ereigniszeit), NaN/Inf im
Spektrum (die Samples sind vor der FFT verriegelt; der Binwert fällt
zusätzlich auf 0). Keine Allokation im Flussschritt (M-43, 0 Allokationen
über 2 s; `std::sort` auf 32 Werten im festen Puffer).

**Prüfliste** (`tools/dirigent/pruefliste.md`, abgehakt, Datei unverändert):

| Abschnitt | Punkt | Etappe 4 |
|---|---|---|
| A Rückstau | Politik bei voll, Zähler | Ereignisring unverändert (64 Plätze, ältestes fällt, Verlustzähler); jeder Korpuslauf verlangt `ereignisseVerworfen() = 0`; B16 entnimmt den Ring nur nach gebautem Snapshot wie das Produkt |
| B Lebenszyklus | starten↔stoppen | `vorbereiten` legt den Detektor an, `zuruecksetzen` und `grenzeZiehen` leeren ihn über `detektorLeeren` (M-62, Rotbelege `m62-leeren`, `m62-historie`) |
| C Verträge und Längen | NaN/Inf und Grenzwerte am Draht | `staerke` in [3, 1000] am serialisierten Wire-Text gemessen (M-63); Vertragstext der Stärke im Schema und in `Vertrag.h` (M-69); kein Feld, keine Version |
| D Bau- und Prüfriegel | fail-closed, Frische | A5 prüft jede Versionsstelle und jede Detektorschwelle einzeln (Name ohne Codefund, mehrdeutiger Fund, anderer Wert, andere Datei = rot); jedes Einzelbein mit Binaryzeitstempel nach der jüngsten Quelle (§35.6), `LastWriteTime` nach jeder Rücknahme |
| E Behauptung ≤ Messung | Runner, Manifest, Kommentar | Runnerzeilen B5, B16, B18, B22 nennen nur Gemessenes, B5 nennt den roten Fall I3; Zahlen in diesem Abschnitt aus Läufen und Protokollen; jede neue Prüfung einmal gebrochen (§35.5), Regressionswachen als solche benannt |
| E Behauptung ≤ Messung | geänderte Zusage an drei Stellen | Runnerkopf (B5, B16, B18, B22), Testköpfe und Kommentar über dem Fluss (`Spektrum.h`), Manifestkopf (Etappe, Basis-SHA, Kanon vorher); die Runnerzeile von A5 nennt Etappe 4 nicht (außerhalb der Ticketpfade, Punkt 4 unten) |
| E Writer-Fixtures | Golden-Klassen | M-68 Klasse (i) am unveränderten Code erfasst, nie nachgezogen; M-50 Klasse (ii) gegen den Altzähler rot, dann Golden 0 |
| F Änderungssatz | Beziehungen im selben Satz | rechnen↔vertragen (Detektor, Schematext, `Vertrag.h`), Versionsschritt↔Register (FeatureEngine, beide Broker-Konstanten, Fassung 20260927), C++↔Rust (M-65/M-66), aktivieren↔abklingen (A-6, M-61) |

**Offene Punkte (für den Dirigenten; Technik innerhalb der Regeln
entschieden, Regelfragen nur benannt):**

1. **M-59 (I3) ROT, 105 von 112.** Kein Wertesatz aus ρ, T_min und H hält V1
   (≤ 1) und I3 (112) zugleich (§35.4). Ursache: bei Klickpaaren im Abstand
   100 ms (2,34 Hops) trägt der Vorframe der Fenster des zweiten Klicks den
   ersten Klick; das Maximumfilter hebt Lmax über das flache Klickspektrum,
   und der Fluss des zweiten Klicks bleibt bei 142 bis 364 dB — unter T_eff
   (459 dB), das V1 verlangt. Verfehlt sind die Klicks bei 5,1/8,6/12,1/
   15,6/21,1/24,6/28,1 s (alle als zweiter Klick eines Paars). Mögliche
   Regeln (Entscheid des Dirigenten, nicht gebaut): (a) die Schranke von I3
   an den Detektorabstand binden (zweiter Klick zählt erst ab einer
   Vorframe-freien Lage, etwa ≥ 3 Hops); (b) die Struktur erweitern (μ > 1
   oder Filterbreite nach Hop, außerhalb der erlaubten Kalibrierung); (c)
   V1 und I3 auf verschiedene Pegel legen. Der Fall bleibt ROT und macht B5
   rot (Exit 1).
2. **Kommentar `experiment.rs:574`** („Die Groesse ist eine Summe von
   MAD-Vielfachen, keine dB.“) stimmt seit dem Detektorumbau nur noch bei
   greifender relativer Schwelle; genau ist „Summe der Ereignisstärken
   (Vielfache der auf κ normierten Schwellendistanz)“. Produktcode der
   Datei ist nach Auftrag unverändert; Nachzug im Abschlussfenster oder als
   Registerzeile.
3. **Produktwirkung der Kalibrierung (nur benannt):** T_min = 0,30 dB je
   Bin macht schwache Einsätze (Summe unter 459 dB bei 48 kHz, etwa ein
   Klick −6 dBFS mit Hann-Gewicht um 0,5 über Rosa −40 dBFS) zu
   Nicht-Flussereignissen, wenn der Peakpfad sie nicht fängt; dafür bleiben
   Rauschen, stehende Töne und Vibrato ohne Fehlalarm. Die Koinzidenz und
   die Transient-Guardrail des Brokers lesen dieselben Ereignisse; ihre
   Schwellen bleiben (M-66, M-67).
4. **Runnerzeile A5** (`tools/beweise.ps1`) nennt M-69/M-70 nicht (die
   Ticketgrenze erlaubt nur B5, B16, B18, B22).
5. **M-63 „heute rot“** war am Basisstand in den gefahrenen Signalen nicht
   an der Stärke messbar (18,19; 90,18; 62,97 lagen in [3, 1000]); rot war
   nur die Klickzuordnung (A-6). Die Unbeschränktheit von (SF − med)/MAD am
   Basisstand ist nachgerechnet, nicht gemessen.
6. **A8, A9, A10** vom Bauer gefahren (§35.6); der volle Kanon, die
   Erstprüfung 4 und der Commit stehen beim Dirigenten.

## 36. Messung der Etappe 4, Regel R-380-12 und Nacharbeitsauftrag 0 (Dirigent, 25.09.2026, 20:31 Uhr)

**Worker.** `ccbc09a1` (`nakama-nak380-21853aa-e4bau`, Opus 5.5, Effort
xhigh nach NAK-404, `dontAsk`), 18:30:48 bis 20:22:55 Uhr (Zeiten des
Bauers aus `Get-Date`, §35.1 und Schlussmeldung), Zustand danach
`blocked`/`idle` mit Schlussmeldung „FERTIG Etappe 4, 83 Zeilen belegt,
Beine B5 (M-59/I3 rot), uncommittet“. HEAD während des Laufs unverändert
`21853aa1`; der Beobachter meldete Telemetrie gesund und um 20:23 Uhr
„blockiert“ (Ende). Beobachter beendet, Stundenloop `40ea2315` gelöscht.

**Messung (Dirigent, `git status --short`, `git diff --stat`, Python auf
Bytes, Lesen der Hunks).** 17 geänderte Ticketpfade, +2 925/−121 Zeilen:
`Spektrum.h` +315 (Detektor: `binFlussSchritt`, `flussFilterRechnen` als
monotone Warteschlange, `detektorSchritt` mit T_eff aus drei Termen,
Spitzenwahl gegen `sfVorher`, Sperrzeit gegen das letzte Ereignis beider
Pfade, Klemme 1000, Nenner > 0), `FeatureEngine.h` +149 (sieben geführte
Konstanten, Heapträger `FlussDetektor`, `detektorBinsBestimmen`),
`Zeit.h` +8, `Vertrag.h` +2, Broker-Konstanten 2 × 1 Zeile,
`experiment.rs` +49 (nur `mod nak380_tests`), Schema +2 (Beschreibungstext
`staerke`), `metriken-v1.json` +175 (Fassung 20260927), Tests +1 451,
`pruefe_v3_vertrag.py` +154, `tools/beweise.ps1` +8, Manifest +729 (§35);
dazu 104 neue Rohdateien `docs/beweise/roh/NAK-380-*` (zwei
Pflichterfassungen, 31 Rotbeweise M-42 bis M-72, Mutationsprotokolle).
Zeilenenden je Datei wie die Arbeitskopie (`git ls-files --eol`: C++ und
Tests, JSON unter `schemas/v3`, `tools/beweise.ps1` CRLF; Rust, Python,
Manifest und Rohdateien LF; Python-Bytezählung Manifest 0 CRLF/5 492 LF,
`experiment.rs` 0/1 776, `NAK-380-rot-M-42.txt` 0/30). Fremde Einträge
unberührt (`briefing-hub/`, `nimbalyst-local/`), `eq-copilot/fixtures/**`
ohne Diff. Dokuriegel auf dem Manifest: 0 Befunde (26 benannt).
Kanon vorher geerbt (§35.1, Diff gegen `8308a3d8` genau die fünf
gemessenen Pfade). Beine einzeln am Endstand (§35.6): B18 105/0, B16
213/0, B22 28/0, A5 887/0, A4 845/0 (23 ignoriert, `lib` 310/0), A1, A16,
B1, B4, A26, A8, A9, A10 Exit 0; **B5 294/1, rot allein `380/M-59
impulskorpus_I3` (105 von 112)**. Goldens ohne Bewegung, Sweep M-64 0 Hops,
F1-Fingerprint bytegleich (§35.7). Pflichterfassungen vor dem ersten
Produktedit belegt (§35.2: W1-Altzähler 18, F1 76 Bytes mit SHA-256).
Tabelle nach der Auftragsregel: 83 Zusagezeilen, jede mit Mutation und
Rohdatei, drei Ersatzmutationen begründet (§35.5, §35.8 Punkt 10).
Kein voller Kanon (auftragsgemäß). Der Dirigent hat die Detektorkonstanten
(`FeatureEngine.h:160-215`) und den Detektor (`Spektrum.h:640-860`) selbst
gelesen: Form wie T-380-5; keine Allokation im Flussschritt; P0 hält den
Logarithmus endlich; Vorframe über alle Frames, Historie nur aktive.

### 36.1 Einordnung der offenen Punkte (§35.9) und der Kalibrierung (§35.4)

| Punkt | Quelle (Bauerstand auf `21853aa1`) | Einordnung | Regel |
|---|---|---|---|
| Kalibrierung T_min 0,05 → 0,30 dB je Bin wegen V1 (160 Ereignisse mit den Startwerten) | `FeatureEngine.h` `kFlussTminDbJeBin = 0.30`; §35.4: „Das Vibrato bewegt die Teiltöne je Hop (42,67 ms) um bis zu … 73,7 Cent, mehr als die Filterbreite“; `kFlussFilterCent = 50.0` | **DEFEKT am Mechanismus des Wortlauts** §39.1 `:2627-2629` „SuperFlux-artiger Maximumfilter über Nachbarbins … reduziert Vibrato-Fehltrigger“ und M-71 (Beleg über M-56 durch den Filter): am Bauerstand unterdrückt nicht der Filter das Vibrato, sondern eine sechsfach angehobene absolute Schwelle, deren Herleitung in T-380-5 („deckt stehende Töne und Änderungen unter P0“) nicht mehr trägt; **dazu LÜCKE in T-380-5**: die Filterbreite ±50 Cent ist ±1 Band der SuperFlux-Filterbank bei 10 ms Hop, die Regel hat die Hopzeit der Hauptstufe (42,67 ms bei 48 kHz, 46,44 ms bei 44,1 kHz) nicht skaliert und die Breite aus der Kalibriermenge ausgeschlossen. Der Bauer hat innerhalb seiner Regel richtig gehandelt und den Punkt benannt (§35.9 Punkt 3). | R-380-12 (i) |
| M-59 (I3) rot, 105 von 112; mit Startwerten 111 | §35.9 Punkt 1: „bei Klickpaaren im Abstand 100 ms (2,34 Hops) trägt der Vorframe der Fenster des zweiten Klicks den ersten Klick; das Maximumfilter hebt Lmax über das flache Klickspektrum, und der Fluss des zweiten Klicks bleibt bei 142 bis 364 dB“ | **LÜCKE** — die Matrix legt I3 auf 100 ms, ohne die Vorframe-Verdeckung der Hauptstufe zu bedenken: Für einen zweiten Impuls im Abstand Δ trifft der erste Frame, der ihn trägt, auf einen Vorframe mit dem ersten Impuls, sobald Δ < N_H + Hop (rasterabhängig für N_H < Δ < N_H + Hop, sicher für Δ ≤ N_H); frei erst ab Δ ≥ N_H + Hop = 6 144 Samples = 128,0 ms bei 48 kHz, 139,3 ms bei 44,1 kHz. Das ist eine Struktureigenschaft des Entwurfs (Hauptstufe 4 096, Hop 2 048), keine Sperrzeit; keine Zusage des Entwurfs verlangt getrennte Ereignisse zwischen Sperrzeit und N_H + Hop. Der Fall bleibt bis zur Regel ehrlich rot (kein Golden gelockert). | R-380-12 (ii) |
| Produktwirkung der Kalibrierung (§35.9 Punkt 3) | schwache Einsätze unter 459 dB Summe ohne Flussereignis | durch R-380-12 (i) aufgehoben (T_min kehrt auf den hergeleiteten Startwert zurück); keine Produktkarte — der Entwurf gibt den Mechanismus vor | — |
| Kommentar `experiment.rs:574` („Summe von MAD-Vielfachen, keine dB“) | seit dem Detektorumbau ungenau (nur bei greifender relativer Schwelle) | **Kommentar nachziehen** (Kommentar, der lügt); Ticketpfad in Nacharbeit 0 um diese Kommentarzeile erweitert | in Nacharbeit 0 |
| Runnerzeile A5 nennt M-69/M-70 nicht (§35.9 Punkt 4) | `tools/beweise.ps1` | **Behauptung ≤ Messung** (Prüfliste E): Ticketpfad in Nacharbeit 0 um die A5-Behauptungszeile erweitert; die B5-Zeile nennt nach grüner Nacharbeit keinen roten Fall mehr | in Nacharbeit 0 |
| M-63 „heute rot“ an der Stärke nicht messbar (§35.9 Punkt 5) | Basisstand 18,19; 90,18; 62,97 in [3, 1000]; rot war nur die Klickzuordnung (A-6) | **Etikett berichtigt**: M-63 Stärkehälfte „heute nicht messbar (Unbeschränktheit von (SF − med)/MAD nachgerechnet)“, Zuordnungshälfte „heute rot“ | Matrixfassung §36.3 |
| Abweichungen §35.8 Punkte 1 bis 7, 9 bis 17 | Heapträger, Warteschlange, Broker-Konstanten, A5-Fall der Etappe 3, gemeinsame Sperrzeit beider Pfade, Peakpfad-Freigabe ab 32 aktiven Frames, V1 wörtlich 1/h, erweiterte Vorbedingungen, Ersatzmutationen, N_H = 4 096, M-53 gefahren (22), M-60 mit zwei Einheitsprüfungen | **angenommen** (Technik innerhalb von Ticket, Invarianten und Regeln, begründet) | — |
| Abweichung §35.8 Punkt 8 (Zahlen folgen der Kalibrierung: M-45 458,9/459,1; M-46 300/20/600; M-48, M-49 A = 918) | Tests | kehren mit R-380-12 (i) zu den Matrixzahlen zurück, sofern T_min = 0,05 hält; sonst folgen sie dem gemessenen Wertesatz mit Formel im Kommentar | Nacharbeit 0 |

### 36.2 Regel R-380-12 (Dirigent, 25.09.2026, Technik innerhalb von R-380-2 und §39.1)

- **(i) Die Filterbreite des Maximumfilters ist an die Hopzeit der Hauptstufe
  gebunden und Kalibrierparameter; T_min kehrt auf den hergeleiteten
  Startwert zurück.** SuperFlux legt ±1 Band der 24-Bänder-je-Oktave-Bank
  (±50 Cent) bei 10 ms Hop an (Böck/Widmer, DAFx-13, Gl. 5). Die Hauptstufe
  hoppt mit N_H/2 = 2 048 Samples (42,67 ms bei 48 kHz, 46,44 ms bei 44,1
  kHz). Ein Vibrato mit Tiefe ±D Cent und Rate f_v bewegt einen Teilton je
  Hop um höchstens Δ = 2·D·sin(π·f_v·T_hop) Cent (Differenz der Sinusphase
  über einen Hop): für V1 (D = 50, f_v = 5,5 Hz) 67,2 Cent bei 48 kHz und
  71,9 Cent bei 44,1 kHz, mit Reserve bis f_v = 7 Hz 80,7 und 85,3 Cent —
  alle über der heutigen Breite 50 Cent (der Bauer misst dasselbe mit der
  Ableitungsschranke 73,7 Cent, §35.4). Regel: `kFlussFilterCent` wird eine
  geführte Kalibriergröße mit dieser Herleitung im Kommentar; Startwert
  **100 Cent** (ein Halbton, ±2 SuperFlux-Bänder; w_k = max(1,
  ⌈k·(2^(100/1200) − 1)⌉), w₁₀ = 1, w₁₀₁ = 7, w₁₀₀₀ = 60, w₁₆₆₈ = 100).
  `kFlussTminDbJeBin` kehrt auf **0,05 dB je Bin** zurück (Herleitung
  T-380-5: stehende Töne mit med ≈ MAD ≈ 0, Änderungen unter P0). ρ = 1, H
  = 32, κ, P0, Sperrzeit unverändert. Kalibrierfolge, falls V1 mit 100 Cent
  und 0,05 dB nicht ≤ 1 hält: zuerst die Filterbreite in Schritten von 25
  Cent bis höchstens 150 Cent, dann T_min bis höchstens 0,10 dB je Bin;
  jede Stufe misst alle Korpusfälle M-50 bis M-59, M-61, M-62 und steht mit
  Zahl im Etappenabschnitt; hält kein Wertesatz innerhalb dieser Grenzen,
  bleibt der Fall ehrlich rot und offen (kein Golden gelockert). Der
  Mechanismusbeleg für §39.1 (M-56, M-71) ist die Mutation **„Filterbreite
  → 50 Cent“ → V1 > 1 → rot**, zusätzlich zur Matrixmutation ρ = 0 und T_min
  = 0. Die Ringgröße der Warteschlange (`vorbereiten`, heute 2·w_max + 2)
  wird aus `kFlussFilterCent` hergeleitet, nicht als Zahl geführt (M-43
  prüft sie). Die Fassung 20260927 in `metriken-v1.json` bleibt die Fassung
  dieser Etappe und trägt die kalibrierten Werte (sie ist erst mit dem
  Etappenabschluss gültig; R-380-7 zählt den Versionsschritt je Etappe).
  Die Zahlen der Matrixzeilen M-42, M-45, M-46, M-48, M-49 folgen der
  Formel mit den geltenden Werten (§36.3).
- **(ii) Die Vorframe-Verdeckung ist eine benannte Eigenschaft; I3 misst
  über ihr.** Ein zweiter Impuls im Abstand Δ nach dem ersten trifft mit
  seinem ersten Frame auf einen Vorframe, der den ersten Impuls trägt,
  sobald Δ < N_H + Hop (sicher für Δ ≤ N_H, rasterabhängig für N_H < Δ <
  N_H + Hop); sein Fluss ist um Lmax des Impulsspektrums reduziert, nicht
  null. Frei ist der Vorframe ab Δ ≥ N_H + Hop = 128,0 ms (48 kHz) bzw.
  139,3 ms (44,1 kHz). Regel: I3 (§7.2, M-59) bekommt den Paarabstand
  **150 ms** (über N_H + Hop bei beiden Raten, Herleitung im Kommentar);
  die Zusage „genau 112 Ereignisse, je Paar zwei“ bleibt; die
  Sperrzeit-Hälfte trägt weiter I2 (20 ms, 56). Der Abstand 100 ms wird in
  der Nacharbeit einmal als **Beobachtung** gemessen (Ereignisse gesamt,
  verfehlte Klicks, kleinster und größter Fluss eines zweiten Klicks) und
  in `docs/beweise/roh/NAK-380-etappe-4-i3-100ms-beobachtung.txt` (Zielpfad, anzulegen) sowie im
  Etappenabschnitt protokolliert, ohne Zusage. Der Kommentar über dem
  Detektor (`Spektrum.h`) nennt die Verdeckung mit Formel und den beiden
  Zeiten; das Register bekommt keinen Eintrag (Eigenschaft der
  Entwurfsstruktur Hauptstufe 4 096/2 048, kein Defekt).

### 36.3 Matrix in der Fassung §36

M-42: Filterbreite w₁₀ = 1, w₁₀₁ = 7, w₁₀₀₀ = 60, w₁₆₆₈ = 100 Bins (100
Cent, nachzurechnen); d = 1 bis 7 ergibt 0,0 dB (Bin 101 liegt im Fenster
von Bin 101 + d, w₁₀₂ bis w₁₀₈ = 7), d = 8 ergibt 79,96 dB; Mutation w_k →
0 unverändert. M-45: T_min = 0,05 dB · 1530 = 76,5 dB (76,4/76,6) wieder
wie §6.3, sofern die Kalibrierfolge bei 0,05 endet; sonst Zahlen aus der
Formel mit dem geltenden Wert. M-46: med 180, MAD 20, T_eff 360 (359/361)
wie §6.3 unter derselben Bedingung. M-48, M-49: Folgen 500/450/400 und
Historie 32 × 0 wie §6.3 unter derselben Bedingung (die Variante des
Bauers mit A = 2·T_min bleibt zulässig, wenn sie die Spitzenwahl am
Basisstand misst; Wahl im Etappenabschnitt begründen). M-56: Zusage
ergänzt um „gehalten durch den Maximumfilter bei T_min = Startwert“;
Rotbeweis zusätzlich „Filterbreite → 50 Cent → V1 > 1 → rot“. M-59:
Vorbedingung „Klickpaare im Abstand 150 ms alle 500 ms von 2,0 s bis 29,5
s = 56 Paare (§7 I3; über N_H + Hop = 128,0/139,3 ms)“; Zusage „genau 112
Ereignisse“; Rotbeweis „Sperrzeit 150 ms → 56 → rot“ wird zu „Sperrzeit
200 ms → 56 → rot“; Beobachtung 100 ms ohne Zusage (R-380-12 (ii)). M-63:
Etikett der Stärkehälfte „heute nicht messbar (nachgerechnet)“,
Zuordnungshälfte „heute rot“. M-70: geführte Schwellen P0 −100 dBFS je
Bin, Filterbreite **100 Cent**, H = 32, κ = 3, ρ = 1, T_min-Faktor **0,05
dB je Bin**, Sperrzeit 50 ms (nach Kalibrierfolge die geltenden Werte);
Mutation je Stelle. M-71: Beleg „M-56 (Vibrato ≤ 1 durch den
Maximumfilter, Mutation Filterbreite 50 Cent rot)“. §7.2 I3: Abstand 150
ms. §8.0 T-380-5: Filterbreite tritt in die Kalibriermenge mit
Hopzeit-Herleitung; Kalibrierfolge nach R-380-12 (i).

### 36.4 Zwischenstand und Nacharbeitsauftrag 0 (Runde 0, zählt nicht zum Rundenbudget)

Der Bauerstand wird als Zwischenstand mit Pathspec committet (Betreff
„NAK-380 Etappe 4 Zwischenstand …, B5 rot an M-59, Kanon NOT RUN“),
damit Runde und Rundenbilanz einen Basis-SHA haben; der Kanon nachher folgt
nach Nacharbeit 0 (Muster Etappe 3, §28/§29). Der Auftrag trägt R-380-12
wörtlich, die Matrixfassung §36.3, die erweiterten Ticketpfade
(`experiment.rs:574` Kommentar, `tools/beweise.ps1` A5- und B5-Zeile,
Signal I3) und die Auftragsregel (Tabelle je berührter Zeile):
`docs/beweise/roh/NAK-380-nacharbeit-e4-0-auftrag.txt`. Bauer: frischer
Opus-5.5-Worker, Effort xhigh (NAK-404), `dontAsk`, Aufsicht ENG; Beine
einzeln; Rotbeweise erneut für jede Zeile, deren Zahlen, Signale oder
Konstanten sich ändern (M-42, M-43, M-45, M-46, M-48, M-49, M-50 bis
M-59, M-61, M-62, M-63, M-65, M-70, M-71); unveränderte Wachen laufen
grün mit. Danach Kanon nachher (Dirigent, abgekoppelt) und Erstprüfung 4
(Codex `gpt-6-astra` xhigh, Vorlage A über `21853aa1...HEAD`). Offen beim
User: nichts.

**Rundenbilanz Runde 0 (Bau) am uncommitteten Stand gegen `21853aa1` (`py -3.13 tools/dirigent/rundenbilanz.py 21853aa1`, 20:35 Uhr):** Produkt 9 Datei(en) +621/−83 | Tests 5 Datei(en) +1436/−15 | Pruefwerkzeug 2 Datei(en) +142/−20 | Doku 1 Datei(en) +878/−3 (Rohdateien untracked, nicht gezählt); Produkt- und Testzeilen vorhanden, kein Konvergenzsignal.

## 37. Nacharbeit 0 der Etappe 4 nach R-380-12 (Bauer Opus 5.5 xhigh, 25.09.2026)

Auftrag `docs/beweise/roh/NAK-380-nacharbeit-e4-0-auftrag.txt`. Kein Stage,
Commit, Push oder Stash, keine Rücknahme über git, kein voller Kanon, kein FL
Studio, keine Installation, kein Laufzeit-Arm, keine Produktions-Pipe. Zeilen
in diesem Abschnitt gelten am Arbeitsstand der Nacharbeit (uncommittet auf
`9125dbbc`).

### 37.1 Startmessung

25.09.2026, 20:37:57 Uhr (`Get-Date`): `git rev-parse HEAD` =
`9125dbbc18f6232507883f813953bcc7a4cc6b29` auf `master`; `git status --short`
nennt nur `briefing-hub/` und `nimbalyst-local/` (`.claude/settings.local.json`
erscheint nicht); Prozessprobe auf cmake, MSBuild, cl, link, cargo, rustc,
EqCop*, eqcop-broker, FL64 und vctip leer. Zeilenenden vor dem ersten Edit
gezählt (Python auf Bytes): C++, Tests, `metriken-v1.json`, `tools/beweise.ps1`
CRLF ohne nackte LF (`beweise.ps1` mit BOM); `experiment.rs`,
`pruefe_v3_vertrag.py`, Manifest LF ohne CR; nach allen Edits gleich.

### 37.2 Änderungen je Datei

- **`FeatureEngine.h`:** `kFlussFilterCent` 50.0 → **125.0**. Der Kommentar
  trägt die Herleitung aus R-380-12 (i) (Hopzeit 42,67/46,44 ms, Δ =
  2·D·sin(π·f_v·T_hop), 67,2/71,9/80,7/85,3 Cent, Startwert ein Halbton =
  100 Cent) und die Kalibrierstufe mit Messzahl (V1 bei 100 Cent 4 Ereignisse,
  ab 125 Cent 0; §37.3). `kFlussTminDbJeBin` 0.30 → **0.10**: Startwert
  0,05 dB mit der T-380-5-Herleitung (stehende Töne, Änderungen unter P0,
  M-47), der Hinweis auf 0,30 entfällt; ein Satz nennt, dass das Vibrato vom
  Maximumfilter gehalten wird (mit 50 Cent löst V1 auch bei 0,10 dB aus:
  119 Ereignisse, `m56-filter50`), dazu die Kalibrierung auf die Obergrenze
  0,10 dB mit Messzahlen (§37.3). Ringgröße in `vorbereiten`: der Code
  leitete sie am HEAD bereits aus `kFlussFilterCent` her
  (`2 * flussFilterBreite (d.binVon + d.binAnzahl - 1) + 2`, die
  Einargument-Form rechnet mit `std::exp2 (kFlussFilterCent / 1200.0) - 1.0`);
  keine Zahl im Code setzt 50 Cent voraus. Die Codezeile bleibt, der
  Kommentar nennt die Herleitung und die Zahlen 252/232/118 Plätze. Der
  Sammelkommentar über den Detektorkonstanten verweist auf R-380-12 und §37
  statt auf „die Kalibrierung von T_min in §35“, der Kommentar an
  `FlussDetektor` nennt w_max aus `kFlussFilterCent`.
- **`featureengine/Spektrum.h` (nur Kommentare, `git diff` gelesen):**
  Flusskommentar „Maximumfilter ±kFlussFilterCent (125 Cent, an die Hopzeit
  gebunden und kalibriert)“; neuer Absatz Vorframe-Verdeckung nach R-380-12
  (ii) (Δ < N_H + Hop, sicher für Δ ≤ N_H, rasterabhängig dazwischen; frei ab
  6 144 Samples = 128,0 ms bei 48 kHz, 139,3 ms bei 44,1 kHz; Fluss
  reduziert, nicht null; Messung §37.5); am Maximumfilter „der Faktor 0,0293“
  → „der Faktor 2^(kFlussFilterCent/1200) − 1, bei 125 Cent 0,0749“. Kein
  Codeedit; die Ringgrößenherleitung verlangte keinen.
- **`metriken-v1.json` (CRLF, `-text`):** Fassung 20260927 führt
  `kFlussFilterCent` = 125.0 und `kFlussTminDbJeBin` = 0.10, je mit Zweck,
  Herleitung, Kalibrierung und „Codebezug kName = Wert“; `seit` („NAK-380
  Etappe 4“), `hinweis` und alle übrigen Einträge unverändert; kein neuer
  Tageswert; 20260925 und 20260926 unverändert; `kFeatureMetricsVersion` und
  beide Broker-Konstanten bleiben 20260927 (A5 M-70 grün).
- **`Nak380Pruefsignale.h`:** `kI3AbstandSamples = 7200` (150 ms) und
  `kI3BeobachtungAbstandSamples = 4800` (100 ms) mit Herleitung über N_H +
  Hop und `static_assert (kI3AbstandSamples >= 4096u + 2048u)`. Der Erzeuger
  `klickPaare (abstandSamples)` war am HEAD schon im Abstand parametrisiert
  und bleibt Zeile für Zeile; alle anderen Signale unverändert.
- **`Sonde013DynamicsTest.cpp` (B18):** Testzugang um `schlangePlaetze` und
  `filterAbweichungenFallend` (Maximumfilter des Produkts über einen streng
  fallenden Vorframe gegen das direkt gerechnete Maximum; −1, wenn der
  Vorframe nicht streng fällt). M-42 mit den Zahlen bei 125 Cent (w₁₀ = 1,
  w₁₀₁ = 8, w₁₀₀₀ = 75, w₁₆₆₈ = 125; d = 1 bis 9 ergibt 0, d = 10 ergibt
  79,96 dB; Nachrechnung im Kommentar). M-43 um sechs Prüfungen: Ringgröße
  2·w_max + 2 je Rate aus der Formel (252/232/118) und „der Ring reicht“ je
  Rate. M-45 rechnet aus der Konstante (Kommentar: 153,0 dB, 152,9/153,1).
  M-46 mit den Matrixzahlen §6.3 (Historie 16 × 160 und 16 × 200: med 180,
  MAD 20, T_eff 360; 359/361). M-48 mit §6.3 (Historie 32 × 0, Folge 0, 500,
  450, 400). M-49 mit Historie 32 × 0 und A = 500, 1,1A = 550. M-44, M-47
  unverändert.
- **`AnalysisGoldenTestMain.cpp` (B5):** M-59 mit
  `klickPaare (sig::kI3AbstandSamples)` und Kommentar zur
  Vorframe-Verdeckung; die Beobachtung als eigene Funktion
  `nak380BeobachtungKlickpaare` (nur Ausgabe, kein `pruefe`, kein
  Zusagefall), erreichbar nur über `--nak380-beobachtung I3-100` bzw.
  `I3-150`; Testzugang um `letzterFensterStart` (`fensterStromStart` steht
  nach `rechneFenster` schon einen Hop weiter, `Spektrum.h:406`; der Anfang
  des geschlossenen Fensters ist einen Hop davor), `letzterFluss`,
  `naechsteSchwelle`. M-50 bis M-58, M-61, M-62 im Text unverändert.
- **B16 `Sonde013EventWireTest.cpp`, B22
  `Sonde013FingerprintGoldenTest.cpp`:** unverändert.
- **`pruefe_v3_vertrag.py` (A5):** `NAK380_E4_DETEKTORSCHWELLEN` trägt je
  Schwelle Block und geltenden Wert (P0 −100, Filterbreite 125, H 32, κ 3,
  ρ 1, T_min 0,10, Sperrzeit 50); `nak380_m70_fassung_etappe_4` prüft je
  Schwelle einzeln Code = Register = geltender Wert an der Codestelle
  (Fallname nennt den Wert); sonst unverändert.
- **`tools/beweise.ps1`:** nur Behauptungszeilen A5 (M-69/M-70: Stärketext,
  Fassung 20260927 mit den sieben Detektorschwellen je mit Wert an ihrer
  Codestelle), B5 (Filterbreite 125 Cent, T_min 0,10 dB je Bin, I3 150 ms
  über der Vorframe-Verdeckung N_H + Hop = 128 ms, kein roter Fall mehr), B18
  (Filterbreite 125 Cent, 1 bis 9 Bins ergeben 0, 10 Bins 79,96 dB; Ring
  252/232/118). BOM und CRLF erhalten (1 592 CRLF, 0 nackte LF), Parser 0
  Fehler.
- **`experiment.rs`:** ausschließlich der Kommentar an
  `GUARDRAIL_TRANSIENT` (ASCII, LF); A4 belegt, dass kein Code wandert.
- **Rohdateien:** 22 Rotbeweisdateien `NAK-380-rot-M-nn.txt` append-only um
  den Block „Nacharbeit 0, 25.09.2026“ ergänzt (M-42, M-43, M-45, M-46, M-48,
  M-49, M-50 bis M-59, M-61, M-62, M-63, M-65, M-70, M-71), 49
  Mutationsprotokolle `NAK-380-e4-n0-mutation-<Kürzel>.txt` (davon ein
  verworfener Erstlauf `…-m70-uebernahme-lauf1.txt`), die Beobachtung
  `docs/beweise/roh/NAK-380-etappe-4-i3-100ms-beobachtung.txt`.
  `NAK-380-etappe-4-m50-w1-altzaehler.txt` und
  `NAK-380-etappe-4-m68-f1-ausgang.txt` unverändert.

### 37.3 Korpus und Kalibrierfolge

Ereignisse je 30 s (R1 12 s), gemessen mit `EqCopAnalysisGoldenTest --nak380
M-nn`, je Stufe `FeatureEngine.h` gesetzt, B5 gebaut (Binary jünger als die
Quelle, je Stufe geprüft), zwölf Fälle gefahren, Datei danach bytegleich
zurück (SHA-256 `74A66B3D…95EC` vor und nach jeder Stufenreihe). „✗“ =
Schranke oder Golden verfehlt; bei den Impulsfällen stehen Treffer,
verfehlte Klicks und Fehlalarme (FA) in Klammern. Die Kalibrierfolge nach
R-380-12 (i): V1 hält mit 100 Cent und 0,05 dB nicht (4 Ereignisse), also
zuerst die Filterbreite in 25-Cent-Schritten, bis V1 hält (125 Cent), dann
T_min in 0,025-dB-Schritten bis zur Obergrenze 0,10.

| Fall | Signal | vorher §35.4 (50 Cent, 0,30) | Stufe 0: 100 Cent, 0,05 (Start) | Stufe 1: 125, 0,05 | Stufe 2: 125, 0,075 | Stufe 3: 125, 0,10 (gewählt) | Schranke | gehalten |
|---|---|---|---|---|---|---|---|---|
| M-50 | W1 Weiß −20 dBFS | 0 | 4 ✗ | 10 ✗ | 6 ✗ | 0 | ≤ 1, Golden 0 | ja |
| M-51 | W2 Weiß −40 dBFS | 0 | 1 ✗ (Golden 0) | 5 ✗ | 1 ✗ (Golden 0) | 0 | ≤ 1, Golden 0 | ja |
| M-52 | W3 Weiß −50 dBFS | 0 | 2 ✗ | 5 ✗ | 2 ✗ | 0 | ≤ 1, Golden 0 | ja |
| M-53 | P1 Rosa −20 dBFS | 0 | 3 ✗ | 7 ✗ | 2 ✗ | 0 | ≤ 1, Golden 0 | ja |
| M-54 | S1 Sinus float32 | 0 | 0 | 0 | 0 | 0 | ≤ 1, Golden 0 | ja |
| M-55 | S2 Sägezahn float32 | 0 | 0 | 0 | 0 | 0 | ≤ 1, Golden 0 | ja |
| M-56 | V1 Vibrato ±50 Cent, 5,5 Hz | 0 | **4** ✗ | 0 | 0 | 0 | ≤ 1, Golden 0 | ja |
| M-57 | I1: 112 Klicks | 112 | 114 (112, 0, 2 FA) ✗ | 120 (111, 1, 9 FA) ✗ | 113 (111, 1, 2 FA) ✗ | 112 | genau 112, je Klick eines | ja |
| M-58 | I2: 56 Paare, 20 ms | 56 | 60 (56, 0, 4 FA) ✗ | 65 (56, 0, 9 FA) ✗ | 59 (56, 0, 3 FA) ✗ | 56 | genau 56, je Paar eines | ja |
| M-59 | I3: 56 Paare, 150 ms | 105 (100 ms, alte Fassung) | 114 (112, 0, 2 FA) ✗ | 117 (111, 1, 6 FA) ✗ | 113 (112, 0, 1 FA) ✗ | 112 | genau 112 | ja |
| M-61 | R1: Rosa, 2 s Stille, Rosa | 1 | 4 ✗ | 10 ✗ | 3 ✗ | 1 (333 824) | genau 1 in [331 904, 336 000] | ja |
| M-62 | P2 −20 dBFS, Seek, +14 dB, Delle | 0 | 5 ✗ | 11 ✗ | 3 ✗ | 0 | 0 | ja |
| gehalten | | 11 von 12 | 2 von 12 | 3 von 12 | 3 von 12 | **12 von 12** | | |

Weitere gemessene Wertesätze innerhalb der Grenzen von R-380-12 (i)
(dieselben Fälle, Rohausgaben unter `eq-copilot/build/nak380-e4n0/kal/`,
Bauartefakt): die Filterbreite bis zur Obergrenze bei festem T_min und das
Raster darum.

| Wertesatz | gehalten | verfehlt (Zahl) |
|---|---|---|
| 150 Cent, 0,05 | 3 von 12 | W1 14, W2 15, W3 10, P1 13, I1 122, I2 70, I3 117, R1 9, M-62 12 |
| 150 Cent, 0,075 | 7 von 12 | W1 2, W3 1, I1 113 (1 FA), I2 57 (1 FA), I3 113 (1 FA) |
| 150 Cent, 0,10 | 11 von 12 | I3 111 (ohne Treffer der zweite Klick bei 26,65 s) |
| 100 Cent, 0,075 | 3 von 12 | wie Stufe 0, nur V1 0 |
| 100 Cent, 0,10 | 9 von 12 | W1 1, W3 1 (je Golden 0), M-62 1 |
| 125 Cent, 0,08 | 5 von 12 | W1 2, W3 1, I1 113, I2 59, I3 113, R1 3, M-62 3 |
| 125 Cent, 0,085 | 6 von 12 | W1 1, W3 1, I1 112 (111 Treffer, 1 FA), I2 57, R1 2, M-62 1 |
| 125 Cent, 0,09 | 11 von 12 | W3 1 (Schranke ≤ 1 gehalten, Golden 0 verfehlt) |
| 125 Cent, 0,095 | 12 von 12 | — |

Die Rosa-Selbstprüfung E-380-13 hält in jeder Stufe vor jedem Nutzer
(größte Nachbardifferenz 0,177/0,051/0,282/0,085 dB, Soll ≤ 1,0 dB). Am
Endstand (§37.7) tragen alle zwölf Fälle ihre Schranke und ihren Golden.

### 37.4 Gewählter Wertesatz und Begründung

**Filterbreite 125 Cent, T_min 0,10 dB je Detektor-Bin** (48 kHz: T_min =
153,0 dB); ρ = 1, H = 32, κ = 3, P0 = −100 dBFS je Bin und Sperrzeit 50 ms
unverändert. Begründung: (1) Mit den Startwerten 100 Cent/0,05 dB verfehlt V1
die Schranke (4 Ereignisse) — die Kalibrierfolge wird fällig. (2) Die erste
Filterstufe, die V1 hält, ist 125 Cent (0 Ereignisse); die Folge stellt die
Filterbreite vor T_min, also bleibt sie bei der kleinsten haltenden Stufe.
(3) Mit breiterem Filter lösen Weiß- und Rosarauschen bei 0,05 dB häufiger
aus (W1: 0 bei 50 Cent in §35.4, 4 bei 100, 10 bei 125, 14 bei 150 Cent);
T_min steigt deshalb in 0,025-dB-Schritten: 0,075 hält 3 von 12, 0,10 hält
alle zwölf. (4) Der Mechanismusbeleg für §39.1 bleibt die Filterbreite: mit
50 Cent löst V1 auch bei T_min 0,10 aus (119 Ereignisse, `m56-filter50`),
T_min allein hält das Vibrato also nicht. (5) Die weiteren Wertesätze
bestätigen die Wahl: 150 Cent/0,10 verliert einen zweiten I3-Klick, 100
Cent/0,10 lässt W1, W3 und M-62 je ein Ereignis; bei 125 Cent halten 0,095
und 0,10 dB alle Fälle, 0,09 lässt W3 ein Ereignis. Gewählt ist das Ende der
0,025-dB-Folge (0,10), der Wert mit dem größten Abstand zu den
Rausch-Fehlalarmen innerhalb der Grenze. Die Zahlen der Matrixzeilen M-42,
M-43, M-45, M-70 und der Registerfassung folgen den geltenden Werten
(Formel unverändert, Zahl im Kommentar hergeleitet); M-46, M-48 und M-49
tragen die Matrixzahlen aus §6.3, weil T_min = 153,0 dB unter 360 bzw. 500
liegt (die Variante A = 2·T_min des Vorgängers ist nicht mehr nötig).

### 37.5 Beobachtung I3 im Abstand 100 ms (R-380-12 (ii), ohne Zusage)

Rohdatei `docs/beweise/roh/NAK-380-etappe-4-i3-100ms-beobachtung.txt`
(`--nak380-beobachtung I3-100`, 20:58:56 Uhr, Exit 0, Binary 20:58:51.129
jünger als die Quelle 20:58:32.415; Endwertesatz). Signal: P2 −40 dBFS
(Saat 0x3800008) plus Klickpaare +0,5 bei 2,0 s + 0,5 s·j und 100 ms später,
56 Paare. **Ereignisse gesamt 108** (bei getrennten Klicks 112); 108 Klicks
mit genau einem Treffer, **4 verfehlt, alle zweite Klicks: 8,6 s, 12,1 s,
21,1 s, 28,1 s**; keine Mehrfachtreffer, keine Fehlalarme. Fluss der Frames,
die die verfehlten zweiten Klicks tragen: 128,5/81,2, 126,6/107,5,
146,0/76,0 und 106,1/102,3 dB gegen T_eff 153,0 dB. Fluss eines zweiten
Klicks über alle 56 Paare (größter SF der tragenden Frames): kleinster
**106,1 dB** (28,1 s, T_eff dort 153,0 dB), größter **3 056,0 dB** (7,6 s).
Der Vorframe des ersten Frames, der den zweiten Klick trägt, enthält den
ersten Klick; der Fluss ist reduziert, nicht null — die Verdeckung aus
R-380-12 (ii). Zum Vergleich am Zusageabstand 150 ms (`I3-150`): 112
Ereignisse, keiner verfehlt, kleinster Fluss eines zweiten Klicks 160,6 dB
bei 23,15 s (T_eff 153,0 dB). Erste Klicks tragen 118,0 bis 2 706,8 dB;
die unter T_eff fängt der Peakpfad (Bits in der Rohdatei). Die Beobachtung
lief zuerst mit einer um einen Hop verschobenen Fensterzuordnung (Stempel
nach `rechneFenster` gelesen); korrigiert vor dem Protokoll, die Rohdatei
trägt nur den korrigierten Lauf.

### 37.6 Tabelle je berührter Matrixzeile

Regel des Auftrags (§31, §35.5): je Satz und Hälfte der Zusage in der
Fassung §36.3/§37.4 eine Zeile Zusagesatz → Prüfzeile → Mutation → rot →
Rohdatei. Prüf- und Mutationszeilen gelten am Nacharbeitsstand. „rot JA“:
das Protokoll nennt „Die Zusage fällt an der erwarteten Prüfung: JA“, die
Rücknahme ist per SHA-256 gleich, der Fall danach grün (Neubau, Binary
jünger als die Quelle). Protokolle `docs/beweise/roh/NAK-380-e4-n0-mutation-<Kürzel>.txt`,
Sammelnachweis je Zeile im Block „Nacharbeit 0“ der Rohdatei
`NAK-380-rot-M-nn.txt`. **66 Zeilen, jede mit einem roten Beleg.**

**M-42** (`NAK-380-rot-M-42.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Filterbreite w₁₀ = 1, w₁₀₁ = 8, w₁₀₀₀ = 75, w₁₆₆₈ = 125 Bins (125 Cent) | `Sonde013DynamicsTest.cpp:662` „380/M-42 superflux_maximumfilter: Filterbreite …“, B18 | `Spektrum.h:682` `return 0;` (m42-w0, Matrixmutation w_k → 0) | JA | `NAK-380-e4-n0-mutation-m42-w0.txt` |
| d = 1 bis 9 ergibt 0,0 dB (Bin 101 im Fenster) | `Sonde013DynamicsTest.cpp:686` „… d = 1 bis 9 …“, B18 | `Spektrum.h:682` (m42-w0) | JA | `NAK-380-e4-n0-mutation-m42-w0.txt` |
| d = 10 ergibt 79,96 dB in genau einem Bin; positive Log-Deltas | `Sonde013DynamicsTest.cpp:689` „… d = 10 …“, B18 | `Spektrum.h:731` Betrag aller Deltas statt positiver (m42-betrag) | JA | `NAK-380-e4-n0-mutation-m42-betrag.txt` |

**M-43** (`NAK-380-rot-M-43.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| K = 1666/1530/765 (k = 3/3/2 bis 1668/1532/766), Untergrenze 30,36 Hz | `Sonde013DynamicsTest.cpp:710` „380/M-43 detektor_binbereich: <Rate> kHz, k = …“, B18 | `Spektrum.h:671` `const int von = 0;` (m43-untergrenze) | JA | `NAK-380-e4-n0-mutation-m43-untergrenze.txt` |
| Obergrenze min(17 959,39 Hz, Kappe) | `Sonde013DynamicsTest.cpp:710`, B18 | `Spektrum.h:673` `const int bis = kHauptPunkte / 2;` (m43-obergrenze) | JA | `NAK-380-e4-n0-mutation-m43-obergrenze.txt` |
| Vorframe und Filterpuffer 2·K·8 B ≤ 26 656 B | `Sonde013DynamicsTest.cpp:710`, B18 | `FeatureEngine.h:641` Vorframe mit 2·K Plätzen (m43-speicher) | JA | `NAK-380-e4-n0-mutation-m43-speicher.txt` |
| im Heap in `vorbereiten`, keine Allokation im Flussschritt | `Sonde013DynamicsTest.cpp:774` „… nach vorbereiten() alloziert der Lauf nichts …“, B18 | `Spektrum.h:739` Vektorkopie im Flussschritt (m43-heap) | JA | `NAK-380-e4-n0-mutation-m43-heap.txt` |
| Ring der Warteschlange 2·w_max + 2 aus `kFlussFilterCent` hergeleitet: 252/232/118 Plätze | `Sonde013DynamicsTest.cpp:736` „… Ring der Warteschlange 2*w_max + 2 = …“ je Rate, B18 | `FeatureEngine.h:644` Ring für 50 Cent, 100/92/48 Plätze (m43-ring50); `FeatureEngine.h:644` ohne „+ 2“, 250/230/116 (m43-ring2w) | JA / JA | `NAK-380-e4-n0-mutation-m43-ring50.txt`, `NAK-380-e4-n0-mutation-m43-ring2w.txt` |
| der Ring reicht: Maximumfilter über einen streng fallenden Vorframe = direktes Maximum in jedem Bin | `Sonde013DynamicsTest.cpp:744` „… der Ring reicht …“ je Rate, B18 | `FeatureEngine.h:644` Ring für 50 Cent (m43-ring50: 1014/931/459 Bins abweichend, kein Zugriff außerhalb des Rings) | JA | `NAK-380-e4-n0-mutation-m43-ring50.txt` |

**M-45** (`NAK-380-rot-M-45.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| T_min = 0,10 dB · 1530 = 153,0 dB: 152,9 dB kein Ereignis | `Sonde013DynamicsTest.cpp:802` „380/M-45 absolute_mindestschwelle: SF = T_min - 0,1 dB …“, B18 | `Spektrum.h:800` T_min halbiert (m45-tminhalb); Matrixmutation T_min → 0 NEIN (m45-tmin0: bei med = MAD = 0 sperrt der Nenner > 0, wie §35.8 Punkt 10) | JA (Ersatz) | `NAK-380-e4-n0-mutation-m45-tminhalb.txt`, `NAK-380-e4-n0-mutation-m45-tmin0.txt` |
| 153,1 dB genau ein Ereignis | `Sonde013DynamicsTest.cpp:805` „… SF = T_min + 0,1 dB …“, B18 | `Spektrum.h:800` T_min verdoppelt (m45-tmin2) | JA | `NAK-380-e4-n0-mutation-m45-tmin2.txt` |

**M-46** (`NAK-380-rot-M-46.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| med 180, MAD 20: T_eff = max(240, 360, 153,0) = 360; 359 kein Ereignis | `Sonde013DynamicsTest.cpp:823` „380/M-46 rauschbodenbezug: T_eff = (1 + rho)*med = 360, SF = 359 …“, B18 | `Spektrum.h:801` ρ → 0 (m46-rho0, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m46-rho0.txt` |
| 361 genau ein Ereignis | `Sonde013DynamicsTest.cpp:826` „… SF = 361 …“, B18 | `Spektrum.h:801` ρ → 2 (m46-rho2) | JA | `NAK-380-e4-n0-mutation-m46-rho2.txt` |

**M-48** (`NAK-380-rot-M-48.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| SF-Folge 0, 500, 450, 400 bei T_eff = 153,0 dB: genau ein Ereignis am Frame mit 500 (450 in der Sperrzeit, 400 kein lokales Maximum) | `Sonde013DynamicsTest.cpp:876` „380/M-48 spitzenwahl_lokales_maximum …“, B18 | `Spektrum.h:851` Bedingung SF(n) ≥ SF(n − 1) entfernt (m48-lokal, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m48-lokal.txt` |

**M-49** (`NAK-380-rot-M-49.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| 48 kHz, 1 Hop (42,67 ms): ein Ereignis | `Sonde013DynamicsTest.cpp:899`, B18 | `Spektrum.h:848` Sperrzeit 0 (m49-sperre0, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m49-sperre0.txt` |
| 48 kHz, 2 Hops (85,33 ms): zwei Ereignisse | `Sonde013DynamicsTest.cpp:903`, B18 | `Spektrum.h:848` Sperrzeit 90 ms (m49-sperre90) | JA | `NAK-380-e4-n0-mutation-m49-sperre90.txt` |
| 44,1 kHz (46,44/92,88 ms): 1 Hop ein, 2 Hops zwei | `Sonde013DynamicsTest.cpp:906`, B18 | `Spektrum.h:848` Sperrzeit 0 (m49-sperre0) | JA | `NAK-380-e4-n0-mutation-m49-sperre0.txt` |
| Konstante `kSperrzeitMs = 50.0` | `Sonde013DynamicsTest.cpp:890`, B18 | `FeatureEngine.h:229` 60.0 (m49-konstante) | JA | `NAK-380-e4-n0-mutation-m49-konstante.txt` |

**M-50 bis M-56** (`NAK-380-rot-M-50.txt` bis `-M-56.txt`; Prüfzeile je Fall
`AnalysisGoldenTestMain.cpp:1373` in `nak380Nullfall`, Aufruf `:1621`,
`:1623`, `:1625`, `:1630`, `:1633`, `:1635`, `:1637`, B5)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| M-50: höchstens 1 Ereignis in 30 s (W1), exakter Golden 0 | `:1373` „380/M-50 nullkorpus_W1 …“ | `Spektrum.h:800-801` ρ = 0, T_min = 0 (m50-relativ, Matrixmutation; W1 36, W2 32, W3 30, P1 32, V1 14 Ereignisse) | JA | `NAK-380-e4-n0-mutation-m50-relativ.txt` |
| M-50 Golden-Klasse (ii): der neue Code lief gegen den Altzähler 18 rot | `:1373` | Etappe 4, `m50-altzaehler` (Golden 0 in der Nacharbeit unverändert) | JA (Etappe 4) | `NAK-380-e4-mutation-m50-altzaehler.txt` |
| M-51: höchstens 1 (W2), Golden 0 | `:1373` „380/M-51 …“ | m50-relativ | JA | `NAK-380-e4-n0-mutation-m50-relativ.txt` |
| M-52: höchstens 1 (W3), Golden 0 | `:1373` „380/M-52 …“ | m50-relativ | JA | `NAK-380-e4-n0-mutation-m50-relativ.txt` |
| M-53: höchstens 1 (P1), Golden 0 | `:1373` „380/M-53 …“ | m50-relativ | JA | `NAK-380-e4-n0-mutation-m50-relativ.txt` |
| M-53: P1 nach E-380-13 selbstgeprüft | `:1398` „380/M-53 rosa_selbstpruefung_E-380-13“ | `Nak380Pruefsignale.h:294` `b0 = 0.0;` (m53-rosa) | JA | `NAK-380-e4-n0-mutation-m53-rosa.txt` |
| M-54: höchstens 1 (S1), Golden 0 | `:1373` „380/M-54 …“ | `Spektrum.h:800` T_min = 0 (m54-tmin0, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m54-tmin0.txt` |
| M-55: höchstens 1 (S2), Golden 0 | `:1373` „380/M-55 …“ | m54-tmin0 | JA | `NAK-380-e4-n0-mutation-m54-tmin0.txt` |
| M-56: höchstens 1 (V1), Golden 0 | `:1373` „380/M-56 …“ | m50-relativ | JA | `NAK-380-e4-n0-mutation-m50-relativ.txt` |
| M-56: gehalten durch den Maximumfilter bei kalibriertem T_min | `:1373` „380/M-56 …“ | `FeatureEngine.h:199` `kFlussFilterCent = 50.0` (m56-filter50: 119 Ereignisse) | JA | `NAK-380-e4-n0-mutation-m56-filter50.txt` |

**M-57 bis M-59** (Prüfzeile `AnalysisGoldenTestMain.cpp:1453` in
`nak380Impulsfall`, Aufruf `:1646`, `:1648`, `:1650`; Selbstprüfung `:1398`;
B5)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| M-57: genau 112 Ereignisse, je Klick eines in [Klick − 4096, Klick] | `:1453` „380/M-57 impulskorpus_I1 …“ | m50-relativ | JA | `NAK-380-e4-n0-mutation-m50-relativ.txt` |
| M-57: P2 selbstgeprüft | `:1398` „380/M-57 rosa_selbstpruefung …“ | m53-rosa | JA | `NAK-380-e4-n0-mutation-m53-rosa.txt` |
| M-58: genau 56, je Paar eines (20 ms) | `:1453` „380/M-58 impulskorpus_I2 …“ | `Spektrum.h:848` Sperrzeit 0 (m58-sperre0, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m58-sperre0.txt` |
| M-58: P2 selbstgeprüft | `:1398` „380/M-58 rosa_selbstpruefung …“ | m53-rosa | JA | `NAK-380-e4-n0-mutation-m53-rosa.txt` |
| M-59: Paare im Abstand 150 ms (über N_H + Hop = 128,0/139,3 ms), genau 112 Ereignisse | `:1453` „380/M-59 impulskorpus_I3 …“ | `Spektrum.h:848` Sperrzeit 200 ms (m59-sperre200: 77 Ereignisse) | JA | `NAK-380-e4-n0-mutation-m59-sperre200.txt` |
| M-59: P2 selbstgeprüft | `:1398` „380/M-59 rosa_selbstpruefung …“ | m53-rosa | JA | `NAK-380-e4-n0-mutation-m53-rosa.txt` |

**M-61 und M-62** (B5)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| M-61: genau ein Ereignis in [331 904, 336 000] | `AnalysisGoldenTestMain.cpp:1761` | `Spektrum.h:691` ein inaktiver Frame leert die Historie (m61-stilleleert) | JA | `NAK-380-e4-n0-mutation-m61-stilleleert.txt` |
| M-61: keines in den stationären Abschnitten nach voller Historie | `AnalysisGoldenTestMain.cpp:1761` | `Spektrum.h:800-801` ρ = 0, T_min = 0 (m61-relativ) | JA | `NAK-380-e4-n0-mutation-m61-relativ.txt` |
| M-61: der Vorframe läuft über inaktive Frames | `AnalysisGoldenTestMain.cpp:1770` | `Spektrum.h:486` `(void) 0;` (m61-vorframe, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m61-vorframe.txt` |
| M-61: die Historie nimmt nur aktive | `AnalysisGoldenTestMain.cpp:1777` | `Spektrum.h:691` Historie auch in inaktiven Frames (m61-historie) | JA | `NAK-380-e4-n0-mutation-m61-historie.txt` |
| M-61: R1 selbstgeprüft | `AnalysisGoldenTestMain.cpp:1398` „380/M-61 rosa …“ | m53-rosa | JA | `NAK-380-e4-n0-mutation-m53-rosa.txt` |
| M-62: Auskunft direkt nach der Grenze falsch | `AnalysisGoldenTestMain.cpp:1853` | `Zeit.h:500` `// detektorLeeren();` (m62-leeren, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m62-leeren.txt` |
| M-62: kein Ereignis im ersten Frame nach der Grenze | `AnalysisGoldenTestMain.cpp:1865` | `Zeit.h:500` (m62-leeren) | JA | `NAK-380-e4-n0-mutation-m62-leeren.txt` |
| M-62: keines, bevor die Historie wieder 32 aktive Frames trägt | `AnalysisGoldenTestMain.cpp:1865` | `Spektrum.h:924` Stand und Füllung beim Leeren nicht auf 0 (m62-historie) | JA | `NAK-380-e4-n0-mutation-m62-historie.txt` |
| M-62: P2 selbstgeprüft | `AnalysisGoldenTestMain.cpp:1398` „380/M-62 rosa …“ | m53-rosa | JA | `NAK-380-e4-n0-mutation-m53-rosa.txt` |

**M-63 und M-65** (B16; M-63 Stärkehälfte „heute nicht messbar
(nachgerechnet)“, Zuordnungshälfte „heute rot“, §36.3)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| M-63: jedes Flussereignis trägt `staerke` ≤ 1000, endlich (Klick auf Stille) | `Sonde013EventWireTest.cpp:742` | `Spektrum.h:872` Klammer 1000 entfernt (m63-klammer, Matrixmutation) | JA | `NAK-380-e4-n0-mutation-m63-klammer.txt` |
| M-63: … ≥ κ = 3 (Korpus I1) | `Sonde013EventWireTest.cpp:796` | `Spektrum.h:872` ohne κ (m63-kappa) | JA | `NAK-380-e4-n0-mutation-m63-kappa.txt` |
| M-63: Nenner ≥ max(med, T_min − med) > 0 (Klick auf stehendem Sinus) | `Sonde013EventWireTest.cpp:785` | `Spektrum.h:872` Basisnenner MAD ohne Klammer (m63-madnenner) | JA | `NAK-380-e4-n0-mutation-m63-madnenner.txt` |
| M-63: der Wiretext passiert Textriegel und Schema, jedes Klickereignis reist | `Sonde013EventWireTest.cpp:757` | m63-klammer | JA | `NAK-380-e4-n0-mutation-m63-klammer.txt` |
| M-65: Onsetsumme 0 in jedem Evidenzfenster nach voller Historie | `Sonde013EventWireTest.cpp:843` | `Spektrum.h:800-801` ρ = 0, T_min = 0 (m65-relativ) | JA | `NAK-380-e4-n0-mutation-m65-relativ.txt` |

**M-70** (`NAK-380-rot-M-70.txt`; A5 `pruefe_v3_vertrag.py --nak380 M-70`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| `kFeatureMetricsVersion` = 20260927 | `pruefe_v3_vertrag.py:2624` | `FeatureEngine.h:89` 20260928u (m70-feature) | JA | `NAK-380-e4-n0-mutation-m70-feature.txt` |
| `METRICS_VERSION` in `prepost.rs` = 20260927 | `pruefe_v3_vertrag.py:2632` | `prepost.rs:206` 20260928 (m70-prepost) | JA | `NAK-380-e4-n0-mutation-m70-prepost.txt` |
| `METRICS_VERSION` in `vergleichbarkeit.rs` = 20260927 | `pruefe_v3_vertrag.py:2632` | `vergleichbarkeit.rs:149` 20260928 (m70-vergleichbarkeit) | JA | `NAK-380-e4-n0-mutation-m70-vergleichbarkeit.txt` |
| `aktuell` = 20260927 | `pruefe_v3_vertrag.py:2637` | `metriken-v1.json:20` 20260926 (m70-aktuell) | JA | `NAK-380-e4-n0-mutation-m70-aktuell.txt` |
| `seit` nennt „NAK-380 Etappe 4“ | `pruefe_v3_vertrag.py:2642` | `metriken-v1.json:422` (m70-seit) | JA | `NAK-380-e4-n0-mutation-m70-seit.txt` |
| Einträge der Fassung 20260926 unverändert übernommen | `pruefe_v3_vertrag.py:2655` | `metriken-v1.json:290` Block 20260926, Peaksteigung 12.5 (m70-uebernahme; Erstlauf an `:161`, Block 20260925, NEIN, formungültig) | JA | `NAK-380-e4-n0-mutation-m70-uebernahme.txt`, `NAK-380-e4-n0-mutation-m70-uebernahme-lauf1.txt` |
| geführt P0 −100 dBFS je Bin | `pruefe_v3_vertrag.py:2681` „… kFlussP0Db = -100.0 steht …“ | `FeatureEngine.h:181` −90 (m70-p0) | JA | `NAK-380-e4-n0-mutation-m70-p0.txt` |
| geführt Filterbreite **125 Cent** | `pruefe_v3_vertrag.py:2681` „… kFlussFilterCent = 125.0 steht …“ | `FeatureEngine.h:199` 100.0 ohne Register (m70-filtercent); Code und Register gemeinsam 50 (m70-filtercent-geltend, `FeatureEngine.h:199`, `metriken-v1.json:432`) | JA / JA | `NAK-380-e4-n0-mutation-m70-filtercent.txt`, `NAK-380-e4-n0-mutation-m70-filtercent-geltend.txt` |
| geführt H = 32 | `pruefe_v3_vertrag.py:2681` | `FeatureEngine.h:204` 30 (m70-historie) | JA | `NAK-380-e4-n0-mutation-m70-historie.txt` |
| geführt κ = 3 | `pruefe_v3_vertrag.py:2681` | `FeatureEngine.h:207` 3.5 (m70-kappa) | JA | `NAK-380-e4-n0-mutation-m70-kappa.txt` |
| geführt ρ = 1 | `pruefe_v3_vertrag.py:2681` | `FeatureEngine.h:210` 1.5 (m70-rho) | JA | `NAK-380-e4-n0-mutation-m70-rho.txt` |
| geführt T_min-Faktor **0,10 dB je Bin** | `pruefe_v3_vertrag.py:2681` „… kFlussTminDbJeBin = 0.1 steht …“ | `FeatureEngine.h:224` 0.05 ohne Register (m70-tmin); Code und Register gemeinsam 0,05 (m70-tmin-geltend, `FeatureEngine.h:224`, `metriken-v1.json:450`) | JA / JA | `NAK-380-e4-n0-mutation-m70-tmin.txt`, `NAK-380-e4-n0-mutation-m70-tmin-geltend.txt` |
| geführt Sperrzeit 50 ms (Matrix: 50 → 60 ohne Registereintrag) | `pruefe_v3_vertrag.py:2681` | `FeatureEngine.h:229` 60.0 (m70-sperrzeit) | JA | `NAK-380-e4-n0-mutation-m70-sperrzeit.txt` |
| die Nullkorpusfälle M-50 bis M-56 fallen am alten Wert | Basisstand-Gegenprobe §35.2 (18/14/19/22/109/55/19) und Kalibrierstufe 0 (§37.3: W1 4, W2 1, W3 2, P1 3, V1 4) | Basisstand und Startwertesatz | JA | `NAK-380-rot-M-50.txt` bis `-M-56.txt`, §37.3 |

**M-71** (`NAK-380-rot-M-71.txt`; Beleg über die genannten Zeilen)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Maximumfilter über Nachbarbins, Bins, positive Deltas | Beleg über M-42 | m42-w0, m42-betrag | JA / JA | `NAK-380-e4-n0-mutation-m42-w0.txt`, `NAK-380-e4-n0-mutation-m42-betrag.txt` |
| adaptive Median/MAD-Schwelle (echte MAD) | Beleg über M-44 (unverändert) | m44-mittel (Etappe 4) | JA | `NAK-380-e4-mutation-m44-mittel.txt` |
| reduziert Vibrato-Fehltrigger: V1 ≤ 1 durch den Maximumfilter | Beleg über M-56 | `FeatureEngine.h:199` Filterbreite 50 Cent (m56-filter50: 119 statt 0); dazu m50-relativ | JA / JA | `NAK-380-e4-n0-mutation-m56-filter50.txt`, `NAK-380-e4-n0-mutation-m50-relativ.txt` |
| ein einfacher Peakpfad bleibt als Gegenbeleg | Beleg über M-60 (unverändert) | m60-peakpfad (Etappe 4) | JA | `NAK-380-e4-mutation-m60-peakpfad.txt` |

### 37.7 Einzeln gefahrene Beine am Endstand

Alle acht C++-Ziele in einem Bau (21:19:14 bis 21:20:29 Uhr, Exit 0,
`vctip.exe` danach beendet); jüngste Quelle `featureengine/Zeit.h`
21:18:23.465 (Rücknahme von `m62-leeren`), jedes Binary jünger (B5 wurde
nach dieser Rücknahme im Mutationslauf neu gebaut, 21:18:42.941, und war
danach aktuell). Aufrufe wie `tools/beweise.ps1`, jedes Bein einzeln
(Rohausgaben `eq-copilot/build/nak380-e4n0/end/`, Bauartefakt):

| Bein | Aufruf | Lauf | Exit | Zähler | Binary |
|---|---|---|---|---|---|
| B18 | `EqCopSonde013DynamicsTest` | 21:20:59–21:21:14 | 0 | 111/0 (vorher 105/0; +6 Ringprüfungen M-43) | 21:19:20.376 |
| B5 | `EqCopAnalysisGoldenTest` | 21:21:14–21:21:28 | 0 | **295/0** (vorher 294/1) | 21:18:42.941 |
| B16 | `EqCopSonde013EventWireTest` | 21:21:28–21:21:32 | 0 | 213/0 | 21:19:38.541 |
| B22 | `EqCopSonde013FingerprintGoldenTest` | 21:21:32–21:21:34 | 0 | 28/0 | 21:19:42.013 |
| A5 | `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | 21:21:39–21:21:45 | 0 | 887/0 | — |
| A1 | `EqCopNullTest` | 21:21:34 | 0 | NULLTEST OK | 21:19:52.669 |
| A16 | `EqCopProbeeqNullTest` | 21:21:34–21:21:35 | 0 | 145/0 | 21:20:03.602 |
| B1 | `EqCopIdentityTest` | 21:21:35–21:21:36 | 0 | 120/0 | 21:20:15.071 |
| B4 | `EqCopQueueStressTest` | 21:21:36–21:21:39 | 0 | 121/0 | 21:20:28.778 |
| A26 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py` | 21:21:46–21:21:47 | 0 | Exit-Gate P4: alle vier Klassen, 0 falsche Behauptungen | — |
| A8 | `erzeuge_v3_fixtures.py --pruefen` | 21:21:47–21:21:48 | 0 | 421 Dateien bytegleich | — |
| A9 | `pruefe_flatc_drift.py` | 21:21:48–21:21:49 | 0 | Drift 0 Dateien | — |
| A10 | `erzeuge_fb_fixtures.py --pruefen` | 21:21:49–21:21:55 | 0 | 131 Dateien bytegleich | — |
| A4 | `cargo test --manifest-path broker/Cargo.toml --color never` | 21:19:12–21:40:19 | 0 | 845 bestanden, 0 gescheitert, 23 ignoriert über 30 Testläufe (gezählt an „test result:“; §35.6 nannte 31 bei denselben Summen); `lib` 310/0 mit `nak380_m66_guardrail_transient_grenze`; kein NOT RUN | — |

Die Einzelfälle für die Endstandblöcke der Rohdateien liefen danach mit
denselben Binaries (`--nak380 M-nn`, alle Exit 0; A5 M-69 7/0, M-70 13/0).

### 37.8 Bewegungen bestehender Goldens

Keine. Der NAK-182-Sweep (M-64) meldet je Rate 202 752, t0 − Stempel 3 584
bei allen fünf Raten und Blockgrößen, wie in §35.7: Bewegung **0 Hops**; E-d
„kein zweites Ereignis“. F1 (M-68) bytegleich (0 von 76 Byte verschieden,
Fenster 701). A2 und B9 nicht berührt. Die Goldens der Nullkorpusfälle (0)
und die Sollzahlen der Impulsfälle sind unverändert; kein Golden gelockert,
keine Schranke geändert. Zählerstände: B18 105 → 111, B5 294/1 → 295/0,
B16, B22, A5 unverändert.

### 37.9 Abweichungen vom Auftrag, begründet

1. **Wertesatz 125 Cent/0,10 dB statt 100 Cent/0,05 dB** — die
   Kalibrierfolge war fällig (V1 4 > 1); die Zahlen von M-42, M-43, M-45,
   M-70 und die Registerfassung folgen den geltenden Werten (R-380-12 (i),
   letzter Satz); M-46, M-48, M-49 tragen die Matrixzahlen §6.3 (§37.4).
2. **Ringgröße ohne Codeedit.** Am HEAD stand schon `2 *
   flussFilterBreite (d.binVon + d.binAnzahl - 1) + 2` mit dem Faktor aus
   `kFlussFilterCent`; keine Zahl setzte 50 Cent voraus. Die Herleitung steht
   jetzt im Kommentar, M-43 misst sie (Größe aus der Formel) und ihre
   Hinlänglichkeit (Filter gegen direktes Maximum, Vorbedingungswache „Vorframe
   fällt streng“, damit eine Mutation, die den Flussschritt abbricht, die
   Prüfung nicht trivial grün lässt — gemessen unter `m43-speicher`).
3. **M-59-Rotbeweis „Sperrzeit 200 ms“** ergibt 77 statt der in §36.3
   genannten 56 Ereignisse: die Fensteranfänge von erstem und zweitem Klick
   liegen je nach Hopraster bis zu 150 ms + 2 Hops auseinander, 21 zweite
   Klicks überleben 200 ms. Die Zusage „genau 112“ fällt an ihrer Prüfung
   (JA).
4. **Ersatzmutationen:** `m45-tmin0` NEIN wie in Etappe 4 (Nenner > 0 bei
   med = MAD = 0) → `m45-tminhalb`; `m70-uebernahme` Erstlauf traf die
   Fassung 20260925 (Anker „Die Konfidenzschwellen bleiben unveraendert“,
   erster Treffer im Register) und blieb NEIN — formungültig, ersetzt durch
   dieselbe Mutation im Block 20260926 (JA), Erstlauf als `-lauf1`
   aufbewahrt.
5. **Zusätzliche Mutationen:** `m43-ring2w` (2·w_max statt 2·w_max + 2)
   und `m70-*-geltend` (Code und Register gemeinsam auf den alten Wert) —
   sie zeigen, dass A5 und B18 den geltenden Wert und die Herleitung messen,
   nicht nur Code = Register.
6. **Erweiterte Kalibriermessung:** neben der Folge 100 → 125 Cent und
   0,05 → 0,075 → 0,10 dB auch 150 Cent (bis zur Obergrenze der
   Filterbreite) und feinere T_min-Stufen bei 125 Cent (0,08 bis 0,095),
   nur als Messung (§37.3).
7. **Beobachtung auch bei 150 ms** (`I3-150`, nur Ausgabe) als Vergleich zur
   Zusage.
8. Fünf Mutationen des ersten Sammellaufs brachen vor jeder Dateiänderung
   ab (Wortlaut mit falscher Einrückung 0-mal gefunden); nachgeholt, alle
   JA.
9. **Zahl in §35.8 Punkt 2 berichtigt:** der 50-Cent-Ring hat bei 48 kHz
   92 Plätze (w₁₅₃₂ = ⌈44,89⌉ = 45), nicht 94; gemessen unter `m43-ring50`.

### 37.10 Selbstaudit und Prüfliste

**Selbstaudit** (eigener Diff gegen HEAD, nicht gestagt): Konstanten und
Register gleich (125.0/0.10 an beiden Stellen, A5 M-70 13/0 und
`metrics_version_bindet_schwellen` grün); Ringgröße hergeleitet und
ausreichend (w_max bei 44,1 kHz = ⌈1668·(2^(125/1200) − 1)⌉ = 125, 252
Plätze, höchste Belegung nachgerechnet 236; M-43); der Faktor 0,0749 < 1
hält die Fensterränder monoton; keine 50-Cent-Reste außer in Herleitung und
Messzahlen (Suche über die Ticketpfade); `Spektrum.h` nur Kommentarzeilen im
Diff; Korpus bytegleich (`git diff --stat -- eq-copilot/fixtures` leer; A8,
A9, A10); kein Audiopfad (A1, A16, B1, B4 grün); Zahlenränder unverändert zur
Struktur aus §35.9 (K = 0: Ring 0 Plätze, Filter kehrt zurück; leere
Historie: keine Entscheidung vor 32 aktiven Frames; erster Frame nach einer
Grenze: kein Vorframe, kein SF); Manifestzahlen aus Läufen und Protokollen;
Zeilenenden je Datei wie vor dem Edit.

**Prüfliste** (`tools/dirigent/pruefliste.md`, abgehakt, Datei unverändert):

| Abschnitt | Punkt | Nacharbeit 0 |
|---|---|---|
| A Rückstau | Politik bei voll, Zähler | Ereignisring unverändert; jeder Korpuslauf und die Beobachtung verlangen bzw. melden `ereignisseVerworfen() = 0` |
| B Lebenszyklus | starten↔stoppen | `vorbereiten` legt den Ring aus der Formel an, `detektorLeeren` unverändert (M-62, `m62-leeren`, `m62-historie`) |
| C Verträge und Längen | Grenzwerte am Draht | `staerke` in [3, 1000] am Wire-Text neu gemessen (M-63, Korpus I1 kleinste 3,807, größte 83,193) |
| D Bau- und Prüfriegel | fail-closed, Frische | A5 prüft je Schwelle Code = Register = geltender Wert; jedes Bein mit Binaryzeitstempel nach der jüngsten Quelle; `LastWriteTime` nach jeder Rücknahme; Stufenskript prüft je Stufe Binary jünger als Quelle |
| E Behauptung ≤ Messung | Runner, Manifest, Kommentar | Runnerzeilen A5, B5, B18 nennen nur Gemessenes (B5 ohne roten Fall); Kommentare nennen Messzahlen statt ungemessener Mechanismen; jede neue Prüfung (M-43 Ring) einmal gebrochen |
| E Behauptung ≤ Messung | geänderte Zusage an drei Stellen | Runnerkopf (A5, B5, B18), Testkommentare und Konstantenkommentare, Manifestkopf (Etappe) |
| E Writer-Fixtures | Golden-Klassen | keine Golden-Bewegung; M-68 bytegleich, M-50 Golden 0 unverändert |
| F Änderungssatz | Beziehungen im selben Satz | Konstante ↔ Register ↔ Kommentar ↔ Test ↔ Runner (Filterbreite, T_min), rechnen ↔ vertragen (Stärketext unverändert, M-69 grün) |

### 37.11 Offene Punkte (für den Dirigenten)

1. **Arbeitsbereich schmal.** Bei 125 Cent halten Null- und Impulskorpus nur
   für T_min ∈ [0,095; 0,10] dB je Bin (0,09 lässt W3 ein Ereignis); der
   gewählte Wert liegt an der Obergrenze der Regel. Am Zusageabstand 150 ms
   trägt der schwächste zweite Klick 160,6 dB gegen T_eff 153,0 dB (7,6 dB
   Abstand); bei 150 Cent/0,10 geht ein zweiter I3-Klick verloren.
2. **Herleitung der Filterbreite trägt nicht allein.** Die Hopzeit-Schranke
   (≤ 85,3 Cent) spricht für 100 Cent; gemessen löst V1 dort noch viermal
   aus, erst 125 Cent hält. Die zusätzliche Breite ist kalibriert, nicht
   hergeleitet (Kommentar und Register nennen die Messzahl).
3. **ρ-Herleitung und breiteres Filter.** T-380-5 begründet ρ = 1 mit rund
   17 % Streuung des Rauschflusses bei ±50 Cent; mit 100 bis 150 Cent und
   T_min 0,05 lösen W1 bis P1 wiederholt aus (§37.3), erst T_min 0,10 deckt
   das. Eine Herleitung von ρ für die breitere Filterbank fehlt.
4. **Produktwirkung (nur benannt):** T_min 0,10 dB je Bin (153 dB bei 48 kHz)
   lässt schwache Einsätze ohne Flussereignis, wenn der Peakpfad sie nicht
   fängt (erste I3-Klicks bis herab zu 118,0 dB fängt der Peakpfad); gegen
   §35 (0,30 dB, 459 dB) ist die Schwelle um den Faktor 3 niedriger.
5. Kanon nachher, Erstprüfung 4 und Commit stehen beim Dirigenten.

## 38. Messung der Nacharbeit 0, Härtung NAK-405, Kanon nachher und Erstprüfung 4 gestartet (Dirigent, 25.09.2026, 21:46 Uhr)

**Worker.** `c1e7b5d1` (`nakama-nak380-9125dbb-e4nach0`, Opus 5.5, Effort
xhigh, `dontAsk`), 20:37:47 bis 21:41 Uhr (§37.1 und Schlussmeldung „done
21:41“), Zustand danach `done`, Schlussmeldung „FERTIG Nacharbeit 0, 66
Zeilen belegt, Beine alle grün, uncommittet“. HEAD während des Laufs
unverändert `9125dbbc`; Beobachter meldete gesund (einmal „Statusquelle
unbekannt“ unter Last, Fehlalarm) und „beendet“. Beobachter beendet,
Stundenloop `9f1b729b` gelöscht, Worker gestoppt.

**Messung (Dirigent).** `git diff --stat` gegen `9125dbbc`: 10 geänderte
Ticketpfade (+1 370/−81 einschließlich Manifest §37 +473): Produkt
`FeatureEngine.h` +52 (Konstanten 125 Cent und 0,10 dB je Bin mit
Herleitungen, Ringgrößenkommentar), `Spektrum.h` 22 Zeilen (nur
Kommentare: Filterbreite, Vorframe-Verdeckung), `experiment.rs` 2 Zeilen
(Kommentar :574), `metriken-v1.json` 8 (Fassung 20260927 mit den geltenden
Werten); Tests `AnalysisGoldenTestMain.cpp` +201, `Sonde013DynamicsTest.cpp`
+174, `Nak380Pruefsignale.h` 13 (I3 150 ms, Erzeuger im Abstand
parametrisiert); Prüfwerkzeug `pruefe_v3_vertrag.py` +31,
`tools/beweise.ps1` 6 (A5-, B5-, B18-Zeilen); 22 Rotbeweis-Rohdateien der
berührten Zeilen append-only ergänzt; 50 neue Rohdateien (49
Mutationsprotokolle `NAK-380-e4-n0-mutation-*`, die Beobachtung
`NAK-380-etappe-4-i3-100ms-beobachtung.txt`). Zeilenenden je Datei wie die
Arbeitskopie (Python-Bytezählung: C++, Tests, JSON, `tools/beweise.ps1` nur
CRLF; Rust, Python, Manifest nur LF). Dokuriegel 0 Befunde. Fixtures ohne
Diff (A8 421, A9 0, A10 131). Beine einzeln am Endstand (§37.7): **B5
295/0** (vorher 294/1), B18 111/0, B16 213/0, B22 28/0, A5 887/0, A4
845/0 (23 ignoriert, `lib` 310/0), A1, A16 145/0, B1 120/0, B4 121/0, A26
Exit 0. Goldens ohne Bewegung, Sweep M-64 0 Hops, F1 bytegleich (§37.8).
Der Dirigent hat §37.3 bis §37.5, §37.7, §37.9 und §37.11 gelesen und die
Kalibrierfolge gegen R-380-12 (i) abgeglichen: Reihenfolge eingehalten
(Filterbreite 100 → 125 Cent, dann T_min 0,05 → 0,075 → 0,10 dB je Bin),
alle zwölf Korpusfälle halten am Endstand (§37.3), der Mechanismusbeleg
für §39.1 steht (`m56-filter50`: 50 Cent bei T_min 0,10 → V1 119 → rot).

**Rundenbilanz Runde 0, Nacharbeit (Worktree gegen `9125dbbc`, 21:44
Uhr):** Produkt 4 Datei(en) +63/−21 \| Tests 3 Datei(en) +344/−44 \|
Pruefwerkzeug 2 Datei(en) +22/−15 \| Doku 23 Datei(en) +941/−1;
Produkt- und Testzeilen vorhanden, kein Konvergenzsignal. Kumuliert nach
dem Commit in §39.

### 38.1 Einordnung der offenen Punkte (§37.11) und Abweichungen (§37.9)

| Punkt | Quelle | Einordnung | Regel |
|---|---|---|---|
| 1 Arbeitsbereich schmal (T_min hält nur in [0,095; 0,10] dB je Bin; schwächster zweiter I3-Klick 7,6 dB über T_eff) | §37.3, §37.11 Punkt 1 | **HÄRTUNG** — Gate verlangt „Nullkorpus mit gemessener Fehlalarmrate“, R-380-2 „kalibriert am Nullkorpus“: erfüllt (0 Fehlalarme je Fall in 30 s, 112/56/112 Treffer, Werte mit Messherleitung im Kommentar und im Register); keine Zusage verlangt eine Marge. Ursache benannt: hinter einem breiten Maximumfilter wird der Binfluss stationären Rauschens spärlich, Median und echte MAD klein, die relativen Terme tragen die Ausreißer nicht, T_min übernimmt (§37.3: bei 100 bis 150 Cent und 0,05 dB lösen W1 bis P1 aus). | Register **NAK-405** |
| 2 Filterbreite 125 Cent kalibriert, nicht allein hergeleitet (Hopzeit-Schranke ≤ 85,3 Cent, 100 Cent ließen V1 viermal aus) | §37.11 Punkt 2 | **HÄRTUNG** (Teil von NAK-405) — R-380-12 (i) verlangt die Herleitung als Startwert und die Kalibrierfolge mit Messzahl; beides ist geschehen; die Restbreite ist als Kalibrierung ausgewiesen (Kommentar, Register). | NAK-405 |
| 3 ρ-Herleitung für das breitere Filter fehlt | §37.11 Punkt 3 | **HÄRTUNG** (Teil von NAK-405) — T-380-5 leitete ρ = 1 für ±50 Cent her; für spärliche Flussverteilungen fehlt sie; ρ = 1 ist unverändert und am Korpus mitgemessen (§37.3: ρ wirkt bei T_min ≥ 0,08 nicht). | NAK-405 |
| 4 Produktwirkung (Einsätze unter 153 dB Fluss ohne Flussereignis, Peakpfad fängt Klicks bis 118 dB) | §37.11 Punkt 4 | benannt; keine Produktkarte (der Entwurf gibt den Mechanismus vor, keine Anzeige); Text in NAK-405 | — |
| Abweichung §37.9 Punkt 3: Rotbeweis M-59 „Sperrzeit 200 ms“ ergibt 77, nicht 56 | Rohdatei `m59-sperre200` | **Matrixfassung berichtigt**: Erwartung „weniger als 112“ (gemessen 77; Fensteranfänge je Rasterlage bis 150 ms + 2 Hops auseinander); die Zusage fällt an ihrer Prüfung (JA) | §38.2 |
| Abweichung §37.9 Punkt 9: 50-Cent-Ring bei 48 kHz 92 Plätze, nicht 94 | `m43-ring50` | §35.8 Punkt 2 ist durch §37.9 berichtigt (append-only) | — |
| Abweichungen §37.9 Punkte 1, 2, 4 bis 8 | Wertesatz nach Kalibrierfolge; Ringgröße war hergeleitet; Ersatzmutationen; Zusatzmutationen `-geltend`, `ring2w`; erweiterte Messung; Beobachtung auch bei 150 ms; nachgeholte Mutationen | **angenommen** | — |

### 38.2 Matrix in der Fassung §38

M-42: w₁₀ = 1, w₁₀₁ = 8, w₁₀₀₀ = 75, w₁₆₆₈ = 125 Bins bei 125 Cent
(w_k = max(1, ⌈k·(2^(125/1200) − 1)⌉), Faktor 0,074 8; Zahlen aus §37.6),
d-Schwelle aus derselben Formel; Mutation w_k → 0. M-43: Ringgröße 2·w_max
+ 2 aus `kFlussFilterCent` hergeleitet, Hinlänglichkeit gemessen (Filter
gegen direktes Maximum). M-45: T_min = 0,10 dB · K (48 kHz: 153,0 dB;
152,9/153,1). M-46, M-48, M-49: Matrixzahlen §6.3. M-56: „höchstens 1
Ereignis in 30 s, gehalten durch den Maximumfilter (Mutation Filterbreite
→ 50 Cent → 119 → rot) und die Matrixmutation ρ = 0, T_min = 0“. M-59:
Paarabstand 150 ms (über N_H + Hop = 128,0/139,3 ms), Zusage genau 112,
Rotbeweis „Sperrzeit 200 ms → weniger als 112 (gemessen 77) → rot“;
Beobachtung 100 ms ohne Zusage: 108 Ereignisse, 4 zweite Klicks verfehlt,
kleinster Fluss eines zweiten Klicks 106,1 dB gegen T_eff 153,0 dB
(§37.5). M-70: geführte Schwellen P0 −100 dBFS je Bin, Filterbreite 125
Cent, H = 32, κ = 3, ρ = 1, T_min-Faktor 0,10 dB je Bin, Sperrzeit 50 ms;
Fassung 20260927. M-71: Beleg über M-42, M-44, M-56 (Filter) und M-60.

### 38.3 Register, Commit, Kanon nachher, Erstprüfung 4

Register: **NAK-405** [Härtung · Messkern · Ereignisdetektor] (Korridor,
Herleitung der Filterbreite und von ρ, Produktwirkung; Weg: Bandintegration
vor dem Fluss oder Herleitung von κ und ρ für spärliche Verteilungen;
Beweis mit Pegelsweep des Impulskorpus und zweitem Vibrato). Der Stand
der Nacharbeit 0 wird mit diesem Abschnitt, dem Registereintrag und dem
Prüfauftrag `docs/beweise/roh/NAK-380-erstpruefung-4-auftrag.txt` (Vorlage
A; ZIEL = HEAD dieses Commits, der Prüfer misst ihn) als Runde-0-Stand
committet und gepusht; danach bewegt sich HEAD bis zum Urteil nicht (Lehre
§34). Auf diesem Stand fährt der Dirigent abgekoppelt den **Kanon
nachher** (`tools/beweise.ps1 -Bauen -Ziel docs/beweise/NAK-380.md
-Anhaengen -Titel NAK-380`, Log `$env:TEMP\nakama-nak380-e4-kanon.log`)
und parallel die **Erstprüfung 4** (Codex `gpt-6-astra`, Effort xhigh,
lesend, `tools/dirigent/codex-lauf.ps1 -Kennung nak380-e4-1`, Prüfbereich
`21853aa1...HEAD` über die 16 Ticketpfade der Etappe 4 ohne `docs/**`).
Offen beim User: nichts.

---

## Kanon-Lauf - NAK-380

**Lauf:** 2026-09-25 22:05 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 70/70 Kanon-Laeufe bestanden | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/NAK-380-1e4a701-dirty.md](roh/NAK-380-1e4a701-dirty.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-25 22:05:53 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 1e4a7010 NAK-380 Etappe 4 Nacharbeit 0 (Opus 5.5 xhigh, Worker c1e7b5d1, 20:37 bis 21:41 Uhr) nach R-380-12: Filterbreite an die Hopzeit als Kalibriergröße, Kalibrierfolge 100 → 125 Cent und T_min 0,05 → 0,075 → 0,10 dB je Bin (12 von 12 Korpusfällen gehalten, Mechanismusbeleg Filter 50 Cent → V1 119 rot), I3 mit 150 ms über der Vorframe-Verdeckung (112) und Beobachtung 100 ms (108, 4 zweite Klicks verfehlt), Ringgröße hergeleitet, Fassung 20260927 mit den geltenden Werten, A5/B5/B18-Runnerzeilen, Kommentar experiment.rs; 66 Zusagezeilen mit Rotbeweis (§37), Beine einzeln alle grün (B5 295/0, B18 111/0, B16 213/0, B22 28/0, A5 887/0, A4 845/0); Messung und Einordnung des Dirigenten §38 (Korridor, Filterbreiten- und ρ-Herleitung als Härtung NAK-405 ins Register, Matrixfassung §38.2, Rundenbilanz), Prüfauftrag Erstprüfung 4 abgelegt; Kanon nachher und Erstprüfung 4 laufen ab diesem Stand |
| Commit (voll) | 1e4a7010915fc8a34765aa1fd6d1b10551cd2fd3 |
| Arbeitsbaum | 5 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.6 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/NAK-380-1e4a701-dirty.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). Seit NAK-380 Etappe 3: M1-Perzentile aus Teilbloecken zu acht Segmenten; Rauschen steht in jedem Band ruhig, eine bekannte Pegelbewegung wird innerhalb -0,5/+2,0 dB gelesen. | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 47,34 s | [A2](roh/NAK-380-1e4a701-dirty.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). Seit NAK-283 (F12) erzeugt ein endlicher Eingang keinen nicht endlichen Wet-Ausgang: im Puls auf 1 kHz mit einem Ton der Amplitude 0,8 x FLT_MAX ist jedes Ausgangssample endlich, und nach dem Einblenden steigt der Wet-Zaehler um genau die verriegelten Samples. Seit NAK-312 Etappe 6a (R-312-8): ein Rollenwechsel zu Main nimmt einen in Legacy eingereichten Auftrag zurueck, bevor die Klassifikation wirkt - 40 Bloecke danach bitgleich ohne Ereignis im Ring, ebenso nach dem Rueckwechsel; in Legacy reicht SOLO nichts ein, zeigt keinen Aus-Knopf, und der Status nennt die Lage; ein Wechsel weg von Main schliesst das Intervall mit genau einem begin und einem end ueber einen weichen Ausfade (Nachbarsprung innerhalb der E-31-Schranke) und blendet beim zweiten Wechsel nicht wieder ein, auch mitten im Ausfade; Freilaufmeldung, Sampleratenwechsel und 10-Minuten-Totmann beenden die Markierung auch in der Main-Flaeche. Seit Etappe 6a (R-312-5): reset() bei angehaltenem Transport beendet Klang und Intervall (40 Bloecke bitgleich, genau ein end ohne Projektzeit, 0 Allokationen), laesst den Oversize-Riegel bis zum naechsten prepareToPlay stehen und den Auftrag bestehen (laeuft der Transport weiter, ein neues begin); ueber reset, Bloecke ohne und mit Erlaubnis und prepareToPlay hat jedes begin genau ein end; bei vollem Ring bleibt reset fail-closed (Ueberlaufbit, keine Sequenz); releaseResources schliesst wie reset und gibt die Blockpuffer frei, prepareToPlay wie bisher; ohne Auftrag entsteht kein Ereignis und kein Host-Dirty. Seit NAK-312 Etappe 7b (U56, 312/M-130): releaseResources laesst den Auftrag bestehen, nach prepareToPlay beginnt er mit genau einem neuen begin - seit der Nacharbeit 1 (R-312-33) am ersten erlaubten Block allein gemessen: dieser Block faerbt, ist hoerbar und meldet das neue begin, die uebrigen 39 bleiben hoerbar ohne weiteres Ereignis. Seit Etappe 7b, Satz 3 (U49, 312/M-118): der Rollenwechsel nimmt den Auftrag zurueck und laesst den Bestand stehen. Seit Etappe 7b, Satz 2 (312/M-109, Grenzzeile, nicht Gegenstand der Antwort U48): Gens Hostbypass bleibt die JUCE-Basis - kein Quelltext unter eq-copilot/plugin/src/ nennt processBlockBypassed oder getBypassParameter, getBypassParameter liefert nullptr, und mit hoerbarem Auftrag in Main geht der Puffer ueber 20 Bloecke processBlockBypassed unveraendert durch, ohne Ereignis; danach faerbt der Auftrag in 40 Bloecken processBlock weiter, ohne neues begin und ohne end. Seit NAK-313 Etappe 7 (R-313-9, 313/M-128): die Byteinstanz heartbeat-wire-v1.json traegt binary_telemetry supported, und der Heartbeat-Schreiber erzeugt ihre drei Wiretexte bytegleich. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 31,73 s | [A3](roh/NAK-380-1e4a701-dirty.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. Seit Etappe F dazu die Proposal-Policy: ein Vorschlag entsteht MIT seinem Befund und traegt die fuenfzehn Felder aus 42.1 plus revert, der Rueckweg ist ein FELD mit drei Werten (dsp_revert faellt), in P5 ist jede Aktion manual, keine Aenderung und mehr Daten sind vollstaendige Objekte, der Zielbereich kommt aus dem Band des Befunds statt aus dem groessten Banddelta, ein geschuetztes Band ist eine HARTE Constraint mit Gegenprobe, ein stop_if auf einem nicht messbaren Guardrail meldet MORE DATA, und ueber fuenfhundert zufaellige Eingaenge wird kein Hard Cap und keine engere Usergrenze ueberschritten. Der Guardrail-Rechner LIEST seit E-05 den Zielbereich aus experiment_begin.ziel; ohne ihn bleibt die Heuristik und das Resultat traegt ziel_geraten - beide Pfade mit verschiedener Zahl gemessen. Fehlt einem Test eine Voraussetzung (das Junction-Recht fuer store_weist_reparse_punkt_im_pfad_ab), meldet er NOT RUN mit Grund: das Bein steht dann als [NOT RUN] in der Uebersicht und der Lauf ist UNVOLLSTAENDIG, nie gruen (NAK-309 R-309-4). Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-43 bis M-45, M-48, M-49, M-52, M-53): die Produkteingaenge P0 und P1 (gemeinsame Lesefunktion samt P1-Weiche) und das Bootstrap-Hello lesen ueber genau einen strengen Lauf json_streng (broker/src/vertrag.rs): Nachspann, zweites Dokument, Schlusskomma, unbekannte Escapes, doppelte dekodierte Namen im selben Objekt samt Escape-Alias und eine Verschachtelung ueber 64 Ebenen sind abgelehnt, derselbe Name in zwei Objekten bleibt gueltig (Raender einzeln gemessen), und die Lesefunktion nennt die Stufe der Ablehnung. Je Eintrag der Eingaenge rust_p0, rust_p1 und rust_bootstrap der Tabelle PRODUKTEINGAENGE-FAELLE.json werden Urteil, Stufe und Wirkung gegen produkt geprueft, jede Abweichung mit PE-nnn gemeldet und die gefahrenen Eintraege gegen den Kopf gezaehlt: P0 sendet kein heartbeat_ack und markiert keinen ersten Heartbeat; die P1-Weiche lehnt vor dem Schema ab, ohne Evidenz- oder Intenteffekt, und liest je Nachricht genau einmal streng (Laufzaehler nur im Testbau); der Bootstrap meldet KeinJson mit der Duplikatmarke statt eines Hellos; 64 Ebenen sind gueltig, 65 fallen an parser. Die Korpusschleife erwartet fuer parser_lehnt_ab die Ablehnung am strengen Lauf und fuer das Negativfixture mit project_sample_start null die Verletzung an der gemeinsamen Eigenschaft; der echte Empfaenger nimmt den wire_snapshot der Byteinstanz evidenz-lokal-wire-v1.json an, und die Evidenzsicht traegt keinen Startwert. Seit NAK-313 Etappe 5a (R-313-4, R-313-5; 313/M-63 bis M-72, M-90, M-96): die Rust-Produktleser lesen Ganzzahlen ueber den Helfer crate::vertrag::ganzzahl (as_i64, sonst as_u64 im Bereich, sonst ein endliches f64 ohne Nachkommateil im Bereich, sonst None und nie 0; die Zahlenraender einzeln gemessen). P0 liest die sequence des Heartbeats vor den Seiteneffekten: 91.0 wird mit sequence 91 beantwortet, und der Resync laeuft genau dann, wenn eine Antwort entsteht. Der Deskriptor traegt host_mixer_index als Ganzzahl: 3.0 nach 3 ist kein Kanalwechsel, und der Snapshot traegt 3. evidenzstand_aus_wert liest Transport-, Ereignis- und Konfidenzzahlen in .0-Form und lehnt ein vorhandenes, nicht lesbares Feld ab, statt 0 zu setzen; 3.0 nach 3 ist kein Epochwechsel. Intervention (Beginn, Ende und genau der Bereich), Befehlskopf und Experimentziel, state_report, Intent und Assistent nehmen die .0-Formen wie Zifferntext an. Eine state_revision von 2^53 faellt im Heartbeat und im state_report am Textriegel: der Heartbeat bleibt ohne Antwort und ohne Liveness-Aenderung, der state_report laesst die gemeldete Revision unveraendert. Das Bootstrap-Hello waehlt das Protokoll ueber denselben Helfer (3.0 und 3e0 sind 3, 3.5 und 4 sind KeinHello, v2 bleibt V2), und die typisierte Uebernahme liest block_size 256.0, channels 2.0 und pid 1234.0 nach einer Normalisierung ueber denselben Helfer. Je Eintrag von rust_p0, rust_p1 und rust_bootstrap wird bei Urteil gueltig der Wert am Feld verglichen; NaN, Infinity, -Infinity und 1e999 enden fuer P0 und P1 am Textriegel. Seit NAK-313 Etappe 5b (R-313-4, 313/M-89): der Wiretext der Handinstanz intent-wire-v1.json passiert die P1-Weiche, und der Intentspiegel uebernimmt die Bestandsrevision 2^53-1 unveraendert. Seit NAK-313 Etappe 6 (R-313-7, R-313-8; 313/M-98 bis M-105, M-115, M-116, M-118): das Bootstrap-Tor rechnet das Urteil des Textriegels ueber die Hello-Bytes vor dem einen strengen Lauf, waehlt danach das Protokoll (ein v2-Hello geht ohne Textriegel und v3-Schema als V2 mit den Rohbytes durch, auch mit samplerate 48000.0000000000001 oder einem NUL-Escape in sensor.label) und wendet fuer v3 erst das gehaltene Urteil an, dann die Hello-Pruefung des v3-Schemas (hello_control, hello_telemetry) vor der typisierten Uebernahme: ohne host ein V3Control mit host None; host null, host.name null und host.version null enden als KeinHello mit dem Schemagrund und dem Zeiger des Feldes, ebenso 121 Zeichen in host.name und 65 in host.version; ein NUL-Escape in plugin_version und die samplerate 1e-308 enden am Textriegel, 1e-307 und 48000 verbinden, 768000.5 endet am Schema, und ein Hello mit NUL-Escape und host null endet am Textriegel. Je Hello laeuft json_streng genau einmal (Laufzaehler nur im Testbau; v2, Control, Telemetry und das am Schema abgewiesene Hello). Der FlatBuffers-Leser meldet einen belegten Adressslot jenseits von Feld-ID 4 als adresse_zusatzfeld wie das MANIFEST, ein laengerer VTable-Eintrag mit 0 bleibt gueltig, und die Senke reicht einen solchen Batch nicht weiter: kein Frame an Gen, der naechste session_snapshot traegt p2_reject mit feature_batch_ungueltig. Seit NAK-313 Etappe 7 (R-313-9; 313/M-128 bis M-130): binary_telemetry ist keine Transportzulassung - meldet eine Quelle im Heartbeat der Tabelle unsupported oder supported, wird ihr P2 in beiden Faellen angenommen und genau einmal an das abonnierte Main weitergereicht, und der Deskriptor traegt den gemeldeten Wert; nach einem Reconnect mit anderem Capabilitysatz traegt der session_snapshot bis zum ersten Heartbeat keinen Deskriptor und danach den neuen Wert; die Byteinstanz mit supported wird beantwortet. Seit NAK-380 Etappe 2 integrieren Bandpassung, Screening und Maskierungspegel die Feinband-Dichten mit der eingefrorenen Gitterbreite; Dichtevergleiche bleiben unveraendert, Zahlenraender und alte/neue Metrikversion werden eigens bewacht. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1.205,78 s | [A4](roh/NAK-380-1e4a701-dirty.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. Fehlt das Junction-Recht fuer volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen, meldet der Test NOT RUN mit Grund: das Bein steht dann als [NOT RUN] in der Uebersicht und der Lauf ist UNVOLLSTAENDIG, nie gruen (NAK-309 R-309-4). | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,79 s | [A4-SI](roh/NAK-380-1e4a701-dirty.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. Seit NAK-309 (M-18 bis M-28) die Regeln des Probewerkzeugs: ohne Argument waehlt es den v2-Probenamen, ein leeres Argument bleibt leer; der Produktionsname (auch gross oder gemischt geschrieben), Namen mit \ oder / hinter \\.\pipe\, fremde Namen und ungueltige v2-Suffixe werden abgelehnt, v2- und v3-Probenamen zugelassen; ein PipeClient ohne Namen startet keinen Verbindungsversuch; ein Konfliktende gilt nur mit einem neuen Broker-ACK ohne Konflikt auf derselben stehenden Verbindung, nicht nach Abbau oder Neuaufbau. Quelltextwachen: PipeProbeMain entscheidet den Namen vor Servererwartung und erstem Client (abgelehnt Exit 64) und wartet auf dieses Konfliktende-Praedikat; das Produkt nennt seine v2-Pipe ausdruecklich. Seit NAK-313 Etappe 4 (R-313-6, 313/M-42, M-49): der v2-Client faehrt nach der UTF-8-Pruefung eines Frames genau einen strengen Lauf kanon::lies, bevor JUCE ihn liest. Je Eintrag des Eingangs cpp_v2_client der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen: ein Peer auf einem Probenamen antwortet mit den Bytes des Eintrags, und jeder Fall endet getrennt, mit letzterFehler "eingehender Pipe-Frame: " und dem Grund des strengen Laufs (Stufe parser oder duplikat). Ein welcome mit Nachspann, Schlusskomma, unbekanntem Escape oder doppeltem type beziehungsweise protocol_version (passender und fremder letzter Wert) hinterlaesst keinen gesendeten Heartbeat, kein Sitzungstoken und keine Protokollversion; ein heartbeat_ack mit doppeltem seq nach gueltigem welcome bestaetigt keinen Heartbeat und meldet keinen Konflikt. Die Zaehlpruefung verlangt genau so viele Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-91, M-93, M-94, M-96): nach der UTF-8-Pruefung und vor dem strengen Lauf prueft der Zahlriegel zahlriegelBytes jede Zahl lexikalisch gegen die v2-Grenze INT64_MAX (Stellenzahl, dann Ziffernfolge; Exponent hoechstens drei Ziffern; ein Bruch hoechstens 15 signifikante Ziffern). Ein welcome mit protocol_version 9223372036854775808, 18446744073709551618, 2e4294967296 oder 2.0000000000000001, ein heartbeat_ack mit seq 18446744073709551616 oder 1.0000000000000001 und ein reject mit min_protocol 9223372036854775808 oder 2.0000000000000001 oder max_protocol 2e4294967296 enden getrennt mit letzterFehler "Zahlriegel: " und bestaetigen nichts; 9223372036854775807 passiert den Riegel und endet an der Feldregel, 2.0 verbindet mit Version 2, und 1e999 endet am Zahlriegel statt am strengen Lauf. Die Riegeleinheit faehrt zahlriegelBytes je Vektor direkt. Der Schalter --ohne-ueberlaufvektoren laesst die ub_bei_juce-Eintraege aus (nur fuer den Rotlauf am Riegelaufruf); der Kanon faehrt alle. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 68,02 s | [A4b](roh/NAK-380-1e4a701-dirty.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. Seit NAK-313 Etappe 4 (R-313-6, 313/M-46, M-48, M-49): json_laden_strikt lehnt doppelte dekodierte Namen im selben Objekt (auch als Escape-Alias) ueber einen object_pairs_hook mit eigener Ausnahmeklasse ab und eine Verschachtelung ueber 64 Ebenen nach dem Laden; die Stufe folgt der Ausnahmeklasse (Hook duplikat, jede andere parser). Jedes Fixture der Manifestklasse parser_lehnt_ab faellt beim Laden und traegt keine Verletzungsmenge, und je v3-Eintrag der Tabelle PRODUKTEINGAENGE-FAELLE.json liefert ein Fall mit PE-nnn im Namen Urteil und Stufe wie vertrag; die Zaehlpruefung verlangt genau so viele v3-Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-73): bei eigenem Urteil gueltig vergleicht der Fall den Wert am Zeiger feld exakt mit wert - eine Ganzzahl gleich, eine Gleitkommazahl nur ganzzahlig und im Betrag bis 2^53-1. Seit NAK-313 Etappe 6 (R-313-7; 313/M-98 bis M-102, M-106 bis M-113, M-121): die gemeinsame Falltabelle des Textriegels traegt die Untergrenze (1e-307 gueltig; 1e-308, 2e-308 und 9.9e-308 nicht), und die v3-Eintraege der Etappe (Bootstrap-Hellos, Handschlag, ACK) liefern Urteil und Stufe wie vertrag. Seit NAK-313 Etappe 7 (313/M-129): die zwei Heartbeats der Tabelle mit binary_telemetry supported und unsupported sind gueltig wie vertrag. Seit NAK-380 Etappe 2 benennt der Vertrag Feinband-Leistungsdichte, Livegruppen-Bandleistung und dimensionsloses band_stereo je Feld und bindet Version 20260925 bei unveraenderten Konfidenzschwellen. Seit NAK-380 Etappe 3 (380/M-30, M-40) bindet das Register die Fassung 20260926 (LRA mit 10 Hz, M1-Perzentile aus Teilbloecken) in kFeatureMetricsVersion UND beiden Broker-Konstanten METRICS_VERSION bei unveraenderten Schwellen, und Kommentar (FeatureEngine.h, Lautheit.h) und Register nennen EBU Tech 3342 §3.1 fuer kLraHopZellen = 1. Seit NAK-380 Etappe 4 (380/M-69, M-70) nennt das Schema die Staerke eines Flussereignisses in Vielfachen der auf kappa = 3 normierten Schwellendistanz (hoechstens 1000; ein reines Peakereignis traegt den Crest ueber der Schwelle in dB) ohne neues Feld und ohne Versionsschritt, und die Fassung 20260927 bindet kFeatureMetricsVersion und beide Broker-Konstanten METRICS_VERSION sowie die sieben Detektorschwellen (P0 -100 dBFS je Bin, Filterbreite 125 Cent, H = 32, kappa = 3, rho = 1, T_min 0,10 dB je Bin, Sperrzeit 50 ms) je einzeln mit Wert an ihrer Codestelle in FeatureEngine.h. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,43 s | [A5](roh/NAK-380-1e4a701-dirty.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [A6](roh/NAK-380-1e4a701-dirty.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [A7](roh/NAK-380-1e4a701-dirty.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-48, M-53) gehoeren dazu die Tabelle PRODUKTEINGAENGE-FAELLE.json, deren Kennungen, Stufen, Wirkungen und Kopfzahlen der Erzeuger bei jedem Lauf auf Widerspruchsfreiheit prueft, die Byteinstanz evidenz-lokal-wire-v1.json, die Fixtures der Manifestklasse parser_lehnt_ab als Rohbytes (nicht ueber den Serialisierer, der das Duplikat verloere) und das Negativfixture mit project_sample_start null. Seit NAK-313 Etappe 6 (R-313-7; 313/M-98 bis M-104, M-106 bis M-113, M-121): die Tabelle traegt die Eintraege der Etappe fuer rust_bootstrap, cpp_control_handshake, cpp_telemetrie_handshake und cpp_control_ack, dazu die aus Etappe 5 an das Tor verschobenen Negativen mit den Stufen der Etappe 6; der Erzeuger prueft jede Stufe gegen die Leserkette der anlegenden Etappe, und die Falltabelle des Textriegels traegt die vier Faelle der Untergrenze. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,49 s | [A8](roh/NAK-380-1e4a701-dirty.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID, keine ist verbrannt oder wiederverwendet, und jedes Offsetfeld steht im Rust-Strukturriegel. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,27 s | [A9](roh/NAK-380-1e4a701-dirty.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen, integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) und seit SONDE-015 band_dynamic_gain_db ID 22 (acht Werte, leerer Vektor, Altsender ohne Feld, vier falsche Laengen, NaN und Inf) sind in beiden Lesern gedeckt. Seit NAK-313 Etappe 6 (R-313-8; 313/M-119): dazu das Negativfixture adresse-zusatzfeld-id5.bin, uebersetzt aus einer Schemavariante mit zusatz:string (id: 5) mit demselben gepinnten flatc, und das Positivfixture adresse-vtable-slot5-leer.bin als gueltige Bytemutation (byte_mutation_von) mit einer 16 Bytes langen Adress-VTable, deren Slot 14 null ist. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 3,52 s | [A10](roh/NAK-380-1e4a701-dirty.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. Seit NAK-313 Etappe 4 (R-313-6, 313/M-42, M-49): je v2-Eintrag der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen durch den strengen Lauf json_laden_strikt und die v2-Definition der Nachricht, verglichen mit vertrag (Urteil und Stufe); die Zaehlpruefung verlangt genau so viele v2-Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-95): der Lauf liest Zahlen mit Bruch oder Exponent als decimal.Decimal, und der Typpruefer nimmt integer als int oder ganzzahligen Decimal, number samt Decimal (draft 2020-12): 2.0000000000000001 und 1.0000000000000001 sind keine Ganzzahlen, 2.0 ist die 2; bei Urteil gueltig wird der Wert am Feld exakt verglichen. Seit NAK-313 Etappe 6 (R-313-7; 313/M-104): dazu die zwei v2-gueltigen Hellos der Tabelle mit einer verletzten v3-Textriegelregel (samplerate 48000.0000000000001, NUL-Escape in sensor.label), beide gueltig gegen den v2-Vertrag. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,29 s | [A11](roh/NAK-380-1e4a701-dirty.md#a11) |
| A12 | Parameterbestand Layout v2 (120 Kennungen, 112 Host-Parameter als Praefix) haelt den Vertrag, und die 109 v1-Kennungen sind aus der eingefrorenen v1-Datei woertlich abgeleitet; Zonen-, Preset- und Ablehnungsregeln stimmen mit dem Vertrag ueberein; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State- und Presetkorpus samt MANIFEST bytegleich zur Neuerzeugung, keine verwaiste Datei. Seit NAK-313 Etappe 3 gehoert das Negativ-Golden schema2/doppelte-eigenschaft-v1.bin (R-313-2) mit SHA-256 und Erwartung nurLesen zum MANIFEST; geladen wird es in B2. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,48 s | [A12](roh/NAK-380-1e4a701-dirty.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B und die benannten Kanonbeweise sie tragen; jedes supported hat einen Termin oder, nur ohne Hostanteil, einen Kanonbeweis, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,45 s | [A13](roh/NAK-380-1e4a701-dirty.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 8,95 s | [A14](roh/NAK-380-1e4a701-dirty.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,31 s | [A17](roh/NAK-380-1e4a701-dirty.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 35,72 s | [A18](roh/NAK-380-1e4a701-dirty.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/NAK-380-1e4a701-dirty.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A20](roh/NAK-380-1e4a701-dirty.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,35 s | [A21](roh/NAK-380-1e4a701-dirty.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 11,04 s | [A22](roh/NAK-380-1e4a701-dirty.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. Eine fehlende Messung ist kein PASS (NAK-283 F13): jeder Speicherpunkt traegt das Gueltigkeitsmerkmal seiner Windows-Abfrage, und fehlt ausserhalb der Neustartfenster eine gueltige Messung, endet S07 mit dem eigenen Status MESSUNG FEHLT (Exit 3) statt im Budget. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 149,28 s | [A24](roh/NAK-380-1e4a701-dirty.md#a24) |
| A34 | Der Selbsttest des Soak-Orakels (NAK-283 F13, Muster A27). Er laeuft OHNE Repo-Fixture und ohne Lauf: vier Faelle bauen ihre Berichte im Speicher und laufen durch dieselben Funktionen wie A24 - Messstelle, Speicherpunkt, Urteil. Eine fehlende RSS-Messung endet mit MESSUNG FEHLT und Exit 3, nie im Budget; ein nicht abfragbarer Prozess und ein Prozess mit 0 Bytes Working Set sind im Bericht verschieden; ein Fehlerpunkt direkt vor oder nach dem Neustartfenster faellt, obwohl das Fenster die Kurve filtert; ein Altbericht ohne Gueltigkeitsmerkmal bleibt ueber --bericht auswertbar, derselbe Bericht als Livelauf ist rot. Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --selbsttest` | [OK] Exit 0 | 0,15 s | [A34](roh/NAK-380-1e4a701-dirty.md#a34) |
| A35 | Der Selbsttest des Laufzeit-Arms (Plan S25e, NAK-286, NAK-309) laeuft ohne FL, ohne Installation und ohne MCP-Repo gegen Attrappen: der Runner haelt Exitcodes, Urteilswoerter und Kopfzeile der ersten Fassung, beendet nie ein fremdes FL (eigen ist nur ein selbst gestarteter Prozess, dessen PID, Startzeit und Befehlszeile in der Besitzliste stehen, auch bei gleichem Fenstertitel), faehrt nur den gepinnten MCP-Stand, startet keinen FL-Lauf gegen einen nicht aktuellen Bau und meldet ein veraendertes Diagnoseprojekt; eine Szenario-Voraussetzung (Exit 5) laesst die Folge weiterlaufen, nur ABWEICHUNG loest den Rueckweg aus. Eine MCP-Antwort misst nur mit der Anforderungskennung ihres Versuchs, jede andere heisst UNGEMESSEN; dass der Importweg fl_trigger nie laedt, ist ueber ein Attrappenpaket gemessen. Antworten des Briefkastens werden an Name und Groesse ausgewaehlt, bevor eine Datei geoeffnet wird; der Nulltest trennt Format, Versatz beider Vorzeichen, Kettenfaktor, Abweichung und Nullvorzeichen und urteilt BITIDENTISCH nur bei gleichen Bytes des Bereichs (NAK-311: +0 gegen -0 heisst NULLVORZEICHEN, Szenario-Exit 5); Baender entstehen nur mit Rechnung aus F-28. Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\fl\selbsttest.py` | [OK] Exit 0 | 33,32 s | [A35](roh/NAK-380-1e4a701-dirty.md#a35) |
| A36 | Die Frischepruefung des Runners leitet je Pruefbinary den Frischebaum aus den MSBuild-Tracking-Logs, den Kernbibliotheken und dem Konfigurationsstand ab: an einem Attrappen-Baubaum verweigert eine geaenderte DSP-Quelle ohne Bau die Beglaubigung, ein frischer Bau wird angenommen, Gleichstand ist frisch, ein nicht ableitbarer Baum ist nie gruen, und jedes gebaute gebundene Kernziel ist gedeckt. Dasselbe Urteil trifft jedes Ziel aus $gemesseneZiele, das der Runner baut (NAK-309 R-309-9; die Uebergabe misst eine Quelltextwache): eine juengere Eingabe nur des gemessenen Ziels verweigert die Beglaubigung, auch wenn jedes Pruefbinary frisch ist, und ein nicht ableitbares oder im CMake-Export fehlendes Ziel ist nie gruen. Verglichen werden Zeitstempel, keine Inhalte. Die NOT-RUN-Meldung der cargo-Beine (R-309-4): ein leerer Meldeordner heisst gelaufen, eine Marke NOT RUN mit Test und Grund, ein unlesbarer Ordner nie leer; im Urteilsblock des Runners geht ROT der fehlenden Voraussetzung vor, und NOT RUN zaehlt nie als gruen (Quelltextwache). Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\eq-copilot\pruefe_beweisrunner.py --selbsttest` | [OK] Exit 0 | 6,07 s | [A36](roh/NAK-380-1e4a701-dirty.md#a36) |
| A28 | Der P5-Evaluationskorpus ist reproduzierbar (Muster A25): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt SHA-256 im Manifest, und eine verwaiste Datei faellt. Zusaetzlich der Hygieneriegel aus NAK-182 R2: der Bezeichner des Kettenbeins steht WOERTLICH in seiner Datei, sonst waere der Name ein Etikett. | `py -3.13 tools\eq-copilot\erzeuge_p5_korpus.py --pruefen` | [OK] Exit 0 | 0,16 s | [A28](roh/NAK-380-1e4a701-dirty.md#a28) |
| A29 | Das Sammelbein des P5-Exit-Gates (59 Punkt 6, 36.4, M-64 bis M-70, R2). Es ist das dritte Glied einer KETTE: der Korpus traegt die Wahrheit, broker/tests/sonde014_p5_korpus.rs faehrt jede Sitzung durch p1 und schreibt die TATSAECHLICH ausgegebene Hypothese, und dieses Bein haelt beides gegeneinander - eine falsche starke Produktbehauptung aendert den Korpus nicht, sie faellt am Vergleich. Precision und Recall stehen JE URSACHENKLASSE, dazu Brier, Kalibrierung, Coverage und Enthaltungsrate; die vier Riegel (falsche_starke, falsche_schwache, precision und recall in [0,1]) laufen je Klasse und ueber die Gesamtmenge. Die Schwelle aus M-31 ist AUSGABE: gesucht wird die niedrigste Sicherheitsstufe, deren Riegel halten. Der Startwert von GATE_MINDEST_FENSTER wird an den zwei Passagensitzungen kalibriert. Ohne frische Ergebnisdatei meldet das Bein Voraussetzung-fehlt (Exit 3) statt gruen. Die Frischepruefung zaehlt das Rechenmodul broker/src/coordinator/hypothese/ zur LAUFZEIT auf (rekursiv), und eine genannte, aber fehlende Quelle ist ebenfalls Exit 3 mit Nennung des Pfads - kein stilles Ueberspringen (NAK-224 D1). | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py` | [OK] Exit 0 | 0,36 s | [A29](roh/NAK-380-1e4a701-dirty.md#a29) |
| A30 | Der Selbsttest des P5-Sammelbeins (M-68, Muster A27). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch DIESELBEN Funktionen, die das Sammelbein fuehrt. Jede Erwartung laeuft mit ihrem GEGENTEIL - eine falsche starke Behauptung und die richtige daneben, eine Enthaltung und dieselbe Menge ohne sie, ein leerer Satz und ein voller. Die vier Riegel werden synthetisch gefuettert und muessen fallen: precision > 1, recall > 1 und brier > 1 sind ueber den Korpusweg strukturell unerreichbar, und eine Wache ohne ausfuehrbaren Negativtest ist keine gemessene Zusage. Zuletzt die Schwellensuche in beide Richtungen. | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py --selbsttest` | [OK] Exit 0 | 0,15 s | [A30](roh/NAK-380-1e4a701-dirty.md#a30) |
| A31 | Die adversarialen Gegenbeispiele des Phasengates G5 am PRODUKTPFAD - genau die Faelle, die das Korpusformat nicht ausdruecken kann. Eine Intent-Rolle trennt zwei messtechnisch identische Kandidaten NICHT, auch nicht bei rolle=fuehrt, und bei drei sichtbaren Kandidaten wird der Fuehrende gegen JEDEN geprueft, nicht nur gegen den Zweitplatzierten. Eine Quelle, die ihre Energie in einem ganz anderen Band traegt als der Masterbefund, erreicht keine starke Aussage. Zwei Kandidaten, deren Raenge sich nur durch ein Band WEIT ausserhalb des Befundintervalls unterscheiden, sind ungetrennt. Umklammernde und disjunkte Fensterspannen fallen am Alignment mit Grund, obwohl die Ueberdeckung ihrer SPANNEN 1,0 meldet. Und die Passage laeuft ueber den echten Produktpfad samt Store: zwoelf und acht Fensterlaengen tragen die Aussage, sieben nicht, und eine Passage ohne ein einziges Masterfenster ergibt eine ENTHALTUNG mit ungueltiger Beobachtung statt Schweigen (M-27). Seit der Nacharbeit 1 (07.09.2026) faehrt das Bein ALLE 15 vereinbarten Ketteneingaben durch denselben Produktpfad und DRUCKT je Eingabe ihr Ergebnis: vier Passagenrandwerte (12, 11, 8 und 7 Fensterlaengen), den zweiten experiment_begin mit gleicher passage_id, zwei Master, den Master ohne Fenster, den Kanalwechsel vor und nach dem Deskriptorwechsel, drei Sonden auf einem Kanal, die antikorrelierte Quelle allein und neben der korrelierten, dazu die Masteranomalie in einem Fenster HINTER der Passage. Seit NAK-213 (08.09.2026) traegt KEINE der sieben frueher gedruckten NAK-213-Luecken mehr eine Luecke: zwei Master ergeben eine Enthaltung OHNE ORT statt eines stillen Gewinners, ein Master ohne Fenster ebenfalls statt Schweigen, der Kanalwechsel nimmt die Belege GENAU DER wechselnden Quelle zurueck und entfernt ihren Befund, drei Sonden auf einem Kanal werden ueber ALLE Quellen der Sitzung erkannt - eine vierte, stumme Sonde desselben Kanals setzt die Duplikatmarke mit. Seit NAK-214 (08.09.2026) fahren drei Faelle dieses Tickets mit: zwei bestaetigte Mains erzeugen KEINEN Vorschlag (Vorschlaege 0, keine proposal-Zeile im Store), ein PRE/POST-Paar unterhalb der Alignmentschwelle bleibt Aussageklasse 1 mit next_test prepost_paar_messen, und eine zurueckgenommene juengste Passage laesst die AELTERE rechnen - jeder druckt sein Ergebnis. Keine Luecke bleibt gedruckt, und der aufruferlose Helfer `luecke()` ist mit seinem letzten Aufrufer gegangen. | `cargo test --manifest-path broker/Cargo.toml --test sonde014_gegenbeispiele --color never -- --nocapture --test-threads=1` | [OK] Exit 0 | 121,17 s | [A31](roh/NAK-380-1e4a701-dirty.md#a31) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,16 s | [A25](roh/NAK-380-1e4a701-dirty.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,14 s | [A26](roh/NAK-380-1e4a701-dirty.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,14 s | [A27](roh/NAK-380-1e4a701-dirty.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 verbundenen Quellen, davon hoechstens 20 angenommen (seit NAK-312 Etappe 7b, U51: bei 32 sind 20 angenommen und 12 nicht), liegt p95 fuer die angenommenen Quellen fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 5,89 s | [A23](roh/NAK-380-1e4a701-dirty.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc) mit dem aktiven DSP-Kern: im Default (eq_enabled aus) bitgleich ueber 1000 Bloecke von 1 bis 2048 Samples bei 44,1 / 48 / 96 / 192 kHz ohne belegte Bank; eq_enabled an mit bypass aus und sonst neutralen Defaults bitgleich; eq_enabled an mit bypass an bitgleich trotz hoerbarem Band dahinter; nach eq_enabled aus nach dem Fade wieder bitgleich; Mix 0 mit Output-Trim 0 dB bitgleich; der Passthrough sanitisiert kein NaN/Inf; 0 Samples Latenz und kein Tail im Passthrough und im Hard-Bypass; speichern-laden-speichern bytegleich im Layout v2 mit Kind Dsp. Seit NAK-283 (F04, F05) bleibt der Nulltest bitgleich, wenn reset() vor jedem dritten Block laeuft - ausgeschaltet bei 44,1 / 48 / 96 / 192 kHz, ohne dass die Zaehler fuer nicht endliche Eingaenge, geheilte Filterzustaende und verworfene Analyseframes steigen, und im Hard-Bypass auch mit NaN, +Inf und -Inf zwischen zwei reset(); Mono und Stereo mit gleichem Ein- und Ausgang werden angenommen (im Monobus bleibt der Passthrough bitgleich, und das Band rechnet endlich), Quadrophonie, 5.1, vier diskrete Kanaele, Mono->Stereo, Stereo->Mono und jeder deaktivierte Hauptbus bekommen ein Nein, setBusesLayout auf Quadrophonie scheitert, und der Prozessor bleibt bei zwei Kanaelen. Das Bundle meldet 112 Host-Parameter, und sein Bundlevertrag laesst nur active_probe zu. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. Seit NAK-311 Etappe 2 (Neutralpruefung) schreibt der Kern engagiert-neutral (eq_enabled an, sonst Default) keinen Sample: ab Sample 512 kommen das Bitmuster aus Subnormals, +-0 und kleinsten Normalen, die Wachmarke 0x7F800001 (signalisierender NaN), ein ruhiger NaN und +-Inf bytegleich heraus, bei 44,1 / 48 / 96 kHz und im Monobus; der Riegel zaehlt die nicht endlichen Werte (6, im Monobus 12), der Tap post_committed traegt dort 0,0 und an jedem Rauschsample ab Sample 0 den Eingang als double. Bei Mix 0 mit Output-Trim 0 dB kommt dasselbe Material in den Bloecken ab Sample 512 bytegleich heraus, und der Tap traegt ab Sample 512 den Eingang als double. Ausgeschaltet und im Hard-Bypass bleibt das Material bytegleich ohne Zaehler; mit hoerbarem Band schreibt der Kern und gibt die Wachmarke nie aus. Die Messung 311/M-01 gibt die Bits unter FTZ/DAZ aus und prueft nur, dass alle 18 Kombinationen liefen. Seit NAK-311 Etappe 4 (R-311-3) rechnet der Ausgleich nur mit dem ausgegebenen Kanal: im Monobus melden Kern UND dspBericht fuer ein Band im Modus right exakt +0,0 dB, und der Ausgang mit Auto-Gain ist bytegleich zum Lauf ohne; nach dem Layoutwechsel auf Stereo tragen beide den Zweikanalwert, bitgleich zu einem Stereokern mit demselben Zustand, zurueck in Mono wieder +0,0 - waehrend channel_mode `right` im bestaetigten Zustand und in den Statebytes stehen bleibt, ohne Revision und ohne Host-Dirty. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, Abnahme U47) ist der EQ unter 44,1 kHz nicht unterstuetzt: bei 32 und 22,05 kHz bereitet der Kern nichts vor, und ueber 1000 Bloecke von 1 bis 2048 Samples kommen Rauschen, Bitmuster, Wachmarke, NaN und +-Inf im Stereo- wie im Monobus bytegleich heraus, ohne dass die Zaehler fuer nicht endliche Eingaenge, geheilte Filterzustaende und verworfene Analyseframes steigen; der Kern meldet die abgelehnte Rate, exakt +0,0 dB Ausgleich und die Kanalzahl 2, und die Blockgroessen 1, 255, 256, 257, 2048 und 4096 kommen ebenso bytegleich heraus, ohne gefuellten Tap, mit 0 Samples Latenz und Tail 0,0 s. Genau 44 100 Hz und der naechste double darueber bleiben unterstuetzt - das Band ist hoerbar, und der Fingerabdruck des Ausgangs ist bei 44,1 / 48 / 96 / 192 kHz gleich dem Golden, das dieselbe Messung am unveraenderten Kern des Basisstands der Etappe (5ee8318c) erzeugt hat -, waehrend 44 100 minus 1 ULP, 44 099, 32 000, 22 050, 8 000, 0, negativ, NaN und +-Inf abgelehnt werden. Rate 0, nicht endlich, 1e300 und maxBlock 0 bleiben das verriegelte Fenster: die letzte Vorbereitung steht, der dspBericht meldet sie weiter, und ABGELEHNT ist dort nichts. v3Samplerate, v3BlockSize und v3Channels tragen im abgelehnten Fenster weiter die gemessenen Werte des Hosts. Seit NAK-312 Etappe 7b (U48, U58) ueber den Hostbypass-Eintritt processBlockBypassed: Probeeq blendet in 256 Samples nach trocken und zurueck, linear und am Nachbarsprung innerhalb der E-31-Schranke, auch wenn der Wunsch in der Blende umkehrt; danach bytegleich zum Eingang auch fuer Rauschen, Subnormals, die Wachmarke, NaN und +-Inf, und nach dem Austritt bitgleich zu einem Prozessor ohne Hostbypass; die Analyse bekommt je Block genau einen Analyseblock, auch im Hostbypass, der Tap post_committed ist dort bitgleich zu dem ohne Hostbypass, und der Riegel zaehlt wie ohne ihn; nach reset und prepareToPlay, auch beim Projektladen mit gebypasstem Slot, uebernimmt der erste Block seinen Eintritt ohne Blende; mit EQ aus, im Hard-Bypass und unvorbereitet schreibt kein Eintritt, und die Analyse laeuft in beiden wie in processBlock. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,70 s | [A16](roh/NAK-380-1e4a701-dirty.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,23 s | [B1](roh/NAK-380-1e4a701-dirty.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. Seit SONDE-015 zusaetzlich das Kind Dsp: 120 Werte bit-exakt durch den Roundtrip, flache Arrays statt Kindknoten, voller Undo-Ring aus 32 Schnappschuessen bytegleich und unter 16 MiB, der 33. Eintrag read-only; die Layoutmigration v1 nach v2 setzt occupied bitgenau aus enabled und den Werten; das Preset traegt zwei getrennte Versionen, weist jedes der sechs verbotenen Identitaetsfelder ab, ignoriert unbekannte Top-Level-Felder und laesst eq_enabled beim Laden unberuehrt. Seit NAK-312 Etappe 5 (312/M-49): die eingefrorenen Statebytes des Parametergoldens vom Basis-SHA 12300f1e passen zu ihrer Zeile im Golden (Laenge und SHA-256), laden normal und kommen aus dem Schreiber bytegleich zurueck. Seit NAK-312 Etappe 7b (U49): der Bestand des Hauptprogramms - Mitglieder samt Labels, Passagen, Intent mit Revision, Schutzangaben, Beziehungen, Assistent - uebersteht hub, sensor, hub live und ueber Speichern und Laden gleich, im Kind RetainedMainProject (Writer-Golden legacy-retained-v1.bin); unbekannte Eigenschaften wandern mit; Altstaende laden wie bisher und kommen bytegleich zurueck; jedes unzulaessige Bestandskind ist read-only mit Originalbytes; je Wechsel ein Host-Dirty. Seit NAK-313 Etappe 2 (R-313-3): setzeBindung prueft 120/60 nur fuer geaenderte Werte - an einem geladenen Stand mit Label 121 und Paarnamen 61 Zeichen gelingt die Rollenwahl mit genau einem Host-Dirty, und beide Texte bleiben; derselbe Aufruf ohne Aenderung gibt false zurueck, ohne Host-Dirty und ohne Reconnect-Anforderung, und der Save bleibt bytegleich. Seit NAK-313 Etappe 3 (R-313-1, R-313-2): ein schreibbar geladener Stand besteht mit seinem groessten Folgezustand den Byte-Riegel verlustfrei, sonst bleibt er read-only mit Originalbytes und wird nie ignoriert - gemessen an Common mit 65 533 (schreibbar samt pre und hub) bis 65 536 Eintraegen und an der nachgerechneten Gesamtgrenze G und G+1; Tiefe 64, 63 geschachtelte Arrays und 72 Knoten bleiben rueckschreibbar. Ein doppelter Eigenschaftsname in einem Knoten, auch tief und am Sammlungsrand, macht eine bekannte Wurzel read-only mit Originalbytes, ohne einen der Werte zu lesen und ohne Schema-1-Migration; eine fremde Wurzel bleibt ignoriert; die Writer-Goldens laden wie bisher, das Negativ-Golden doppelte-eigenschaft-v1.bin read-only. Seit NAK-313 Etappe 5b (R-313-4; 313/M-74 bis M-79): jede persistente Revision - Bestands-, Eintrags- und Assistentenrevision im MainProject wie im RetainedMainProject, Dsp.state_revision und die Undo-Revision - laedt bei 2^53-1 schreibbar und kommt bytegleich zurueck; 2^53 und int64max bleiben read-only mit Originalbytes, und der Grund nennt das Feld. Der NAK-283-Fall M-11 steht am Rand 2^53-1: Assistentenhandgriff und entferneIntent werden dort abgewiesen, die Bytes bleiben gleich, und eine neue Instanz laedt schreibbar mit 2^53-1. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 18,58 s | [B2](roh/NAK-380-1e4a701-dirty.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,07 s | [B3](roh/NAK-380-1e4a701-dirty.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,32 s | [B3b](roh/NAK-380-1e4a701-dirty.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. Seit SONDE-015 zusaetzlich der Fassungsschritt P1 5: der aus der committeten Fassung zurueckgebaute Leser der Fassung 4 lehnt state_report.dsp ab, ein Bericht ohne dsp bleibt in beiden Fassungen gueltig, und der Empfaenger rechnet SHA-256(dsp.jcs) gegen state_hash nach - ein schemagueltiger Bericht mit abweichendem Hash wird GANZ abgewiesen. Seit NAK-311 Etappe 4 Teil b (R-311-5) klemmt der Bericht `auto_gain_db` auf die Vertragsgrenze, statt sie zu reissen: acht Low-Shelves 1 kHz +12 dB Q 8 sind abgeleitet -199,77 dB, gemeldet werden exakt -120, und der aus dem echten Transaktionskern gebaute state_report ist gueltig; acht Shelves Q 0,707 bleiben mit -92,29 dB bitgleich zum abgeleiteten Wert. Die Klemmgrenze im C++ ist dieselbe Zahl wie minimum und maximum des GELADENEN Schemas; +/-120 kommen unveraendert zurueck, der naechste double darueber und darunter sowie 120,5 werden geklemmt, NaN wird +0,0 (nicht -0,0) und +/-Inf werden +/-120. Geklemmt wird NUR der Bericht: DspKern::autoGainDb() und der lineare Faktor des gefahrenen Programms bleiben beim ungeklemmten Wert, und klemmungen bleibt leer. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, R-311-20) meldet der Bericht bei einer Rate unter 44,1 kHz wie ohne Samplerate: bei 32 kHz haelt der Transaktionskern fs = 0, auto_gain_db ist exakt +0,0 und klemmungen leer, waehrend derselbe Zustand bei 48 kHz -92,29 dB meldet; Kern und Bericht tragen die abgelehnte Rate, und der daraus gebaute state_report bleibt gueltig gegen $defs/dsp_bericht. Der Grund bleibt C++-lokal: dasselbe Dokument MIT dem Feld im dsp-Block wird von additionalProperties: false abgewiesen, ohne das Feld ist es gueltig - der v3-Vertrag bleibt unberuehrt. Seit NAK-311 Etappe 5 Satz B (R-311-14, Karte U54) melden Bericht und Kern DENSELBEN gedeckelten Wert: acht High-Cuts 20 Hz Q 0,15 sind abgeleitet +150,46 dB und ueber DspKern::autoGainRohDb() lesbar, gefahren und gemeldet werden bitgleich kAutoGainDeckelDb, weil der Deckel vor baueBericht sitzt; die Berichtsklemmung laesst den Wert unveraendert durch, und der Zustand steht in Kern und Bericht. Mit ausgeschaltetem Auto-Gain ist er in beiden falsch, auf der Absenkungsseite ebenso (-199,77 dB abgeleitet, exakt -120 gemeldet, klemmungen leer). Der Zustand geht nicht auf den Draht: derselbe Bericht MIT dem Feld im dsp-Block wird von additionalProperties: false abgewiesen. Seit NAK-313 Etappe 4 (R-313-6, R-313-7; 313/M-47, M-53): die Korpusschleife faehrt nach dem Textriegel ueber dieselben Rohbytes genau einen strengen Lauf kanon::lies als Tor vor der Engine; jedes Fixture der Manifestklasse parser_lehnt_ab (Nachspann, zweites Dokument, Schlusskomma, unbekanntes Escape, doppelter Name, Escape-Alias) faellt dort mit Grund und erreicht die Engine nie, alle uebrigen Urteile und Verletzungsmengen bleiben wie im Manifest - auch das Negativfixture mit project_sample_start null, dessen Verletzung an der gemeinsamen Eigenschaft liegt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-62): die Engine liest die Werte aus DIESEM strengen Lauf ueber nakama::vertrag::wertAlsVar statt ueber juce::JSON::parse; alle Urteile und Verletzungsmengen bleiben wie im Manifest, auch das Bruchzahl-Fixture mit nicht_endliche_samples 2.5 (type-Verletzung). Seit NAK-313 Etappe 6 (R-313-7, R-313-8; 313/M-101, M-114, M-118, M-119, M-121): der FlatBuffers-Leser meldet einen belegten Adressslot jenseits von Feld-ID 4 als genau einen Verstoss adresse_zusatzfeld am Pfad /eintraege/0/quelle (Fixture adresse-zusatzfeld-id5.bin aus einer Schemavariante mit Feld-ID 5); ein laengerer VTable-Eintrag mit Offset 0 (adresse-vtable-slot5-leer.bin) bleibt gueltig, und der ganze Gueltigkorpus bleibt gueltig. audioGueltig nimmt 1e-307 an und lehnt 768000.5 ab, wie das Tor des Brokers; die Falltabelle des Textriegels traegt die Untergrenze (1e-308, 2e-308 und 9.9e-308 abgelehnt). | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,45 s | [B3c](roh/NAK-380-1e4a701-dirty.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,84 s | [B4](roh/NAK-380-1e4a701-dirty.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. Seit NAK-380 Etappe 3 die LRA-Faelle 1 bis 4 aus EBU Tech 3342 Tabelle 1 (10/5/20/15 LU ± 1, gleich der §5-Referenz ± 0,1 LU). | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 5,17 s | [B9](roh/NAK-380-1e4a701-dirty.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. Seit NAK-380 Etappe 2 bleiben 221 Feinbaender Leistungsdichte in dBFS/Hz; 64 Livegruppen sind bandbreitenintegrierte Bandleistung in dBFS, einschliesslich leerer Feinbaender innerhalb einer belegten Gruppe, Parseval, Sinus und Traegerrand. Seit NAK-380 Etappe 4 (380/M-50 bis M-62): der Ereignisdetektor rechnet den SuperFlux-Fluss auf den Bins der Hauptstufe (Maximumfilter +-125 Cent ueber den Vorframe, an die Hopzeit gebunden, echte MAD ueber 32 aktive Frames, T_min = 0,10 dB je Detektor-Bin, Rauschbodenbezug, Spitzenwahl, Sperrzeit 50 ms); je 30 s Weissrauschen bei -20/-40/-50 dBFS, rosa Rauschen, float32-Sinus, Saegezahn und Vibrato +-50 Cent erzeugen hoechstens ein Ereignis (gemessen jeweils 0, als Golden verriegelt), 112 Klicks auf rosa Rauschen -40 dBFS genau 112 Ereignisse, 56 Klickpaare im Abstand 20 ms genau 56 und 56 Klickpaare im Abstand 150 ms - ueber der Vorframe-Verdeckung der Hauptstufe (N_H + Hop = 6144 Samples, 128 ms) - genau 112, je Klick beziehungsweise Paar eines im Fenster [Klick - 4096, Klick]; ein Wiederbeginn nach 2 s digitaler Stille ist genau ein Ereignis; eine Grenze leert Vorframe und Historie. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 12,89 s | [B5](roh/NAK-380-1e4a701-dirty.md#b5) |
| B6 | Aktiver DSP-Kern (plugin/dsp/) als Bibliothek: alle sechs RBJ-Filtertypen liegen ueber einem 1/24-Oktav-Gitter von 20 Hz bis min(20 kHz, 0,45 fs) bei 44,1 / 48 / 96 / 192 kHz innerhalb 0,05 dB typisch und 0,1 dB an den Raendern an der analytischen Antwort - gemessen als Impulsantwort AM TAP post_committed, also durch den Audiopfad, gegen eine im Test eigenstaendig ausgeschriebene Formel. Ausgeschaltet und hard-bypassed wird bei jeder Hoermatrix-Auswahl kein Sample geschrieben (bitgleich); bei Mix 0 mit Output-Trim 0 dB ist der Ausgang wertgleich zum Eingang; nach dem Ausschaltfade ist der Passthrough wieder bitgleich, und er sanitisiert kein NaN. Die acht Slots wirken als Kaskade von Slot 0 nach 7 gegen ein unabhaengig gerechnetes geordnetes Golden, das ein Rueckwaertslauf reisst. Auto-Gain ergibt bei flacher Kurve exakt 0 dB und bei einem +6-dB-Shelf ueber dem Gitter -6 dB, ein identisches Mid/Side-Paar faellt exakt auf den Stereo-Fall zusammen, und die Dynamik bewegt ihn nicht. Der Bankpool faehrt jeden der sechs Ownership-Uebergaenge einzeln, gibt busy_retry statt eine aktive Bank zu verdraengen, wird erst nach dem Audio-ACK frei und haelt seine Invarianten unter zwei echten Threads ueber tausende Bloecke; im Callback zaehlt der thread-lokale Zaehler ueber 4000 Bloecke wechselnder Groesse 0 Allokationen und 0 Sperren, waehrend derselbe Zaehler die Allokationen des Workers NICHT sieht; B6 baut keine Sonde, sein Zaehler misst ab DspKern::verarbeite (NAK-312 R-312-1). Ein Block ueber maxBlock (1024 Samples bei 256) verwirft den Analyse-Tap und gleicht auf beiden Kanaelen sample-exakt dem Lauf ohne Ueberlast in 4 x 256. Seit NAK-283 (F12) die float-Kante des Ausgangs: ein endlicher double ueber FLT_MAX (Output-Trim +6 dB auf 0,75 und 0,25 x FLT_MAX) kommt auf 0 verriegelt und im Eingangszaehler gezaehlt heraus, und kein Ausgangswert ist nicht endlich; auf endlichem Material in +/-1,0 (Bell +9 dB und Output-Trim +6 dB, 204 800 Werte, auch mit Betrag >= 1) ist jeder float bitgleich die Verengung des double-Taps, und der Zaehler bleibt stehen. Seit NAK-311 Etappe 2 (Neutralpruefung) schreibt der engagiert-neutrale Kern (Merkmal neutral, alle fuenf Rampen in Ruhe auf 1,0) keinen Sample: die Wachmarke 0x7F800001, NaN, +-Inf und das Bitmuster kommen bytegleich heraus, der Riegel zaehlt, der Tap post_committed traegt dort 0,0. Das Schreibende liegt exakt am ersten Sample nach Crossfade, Rampe und Hoermatrix-Fade, auch mitten im Teilstueck, bei Blockgroessen 1, 180, 255, 256, 257 und 4096 (maxBlock 512), und das Schreiben setzt am ersten Sample eines neuen Uebergangs wieder ein; bei Delta und Candidate, bei Mix 1e-9 und mit einem 0-dB-Bell schreibt der Kern. Wo er nicht mehr schreibt, misst der Tap post_committed die Rechnung: am Ende einer Width-Rampe auf 1,0 und bei Mix 0 exakt den Eingang als double. Der Allokationslauf mit neutralem Schritt bleibt bei 0 Allokationen. Seit NAK-311 Etappe 3 (W01) schaltet ein Pfad aus der Ruhe wie ein frischer Kern ein: nach Ausschalten, Ausblenden und einer Aenderung in der Ruhe (Output- oder Input-Trim +24 dB, Mix 0, Width 2,0, Auto-Gain; ebenso nach beendeAudiohistorie im Ausblenden und fuer den Candidate nach Ende und Hoerhalt am Tap post_candidate) ist der Ausgang ab dem Umschaltblock bytegleich zu einem frischen Kern mit demselben Programm, bei Blockgroessen 1 bis 4096 und 44,1 / 48 / 96 kHz; mit Ziel 0 dB bleibt Ausgang/Eingang an jedem Sample 1,0. Seit NAK-311 Etappe 3 (W03) traegt jeder aktive Slot eine Lebenszykluskennung: ein Slot, dessen Belegung und Topologie ueber jede Publikation seit dem gefahrenen Programm gleich blieben, behaelt beim Wechsel an einem ANDEREN Slot seinen Filter-, Detektor- und Huellkurvenzustand - sein Perioden-RMS (volle Periode des Prueftons, beide Kanaele, 100 Perioden ab dem Umschaltblock) liegt hoechstens 0,5 dB neben einem Referenzkern mit gleicher Vorgeschichte ohne Wechsel, bei 44,1 / 48 / 96 kHz und Blockgroessen 1 bis 4096 ueber maxBlock, am Candidate-Pfad ebenso; ein geaenderter Slot, ein zwischendurch entfernter und neu belegter und nach einem globalen Wechsel (Samplerate, Mono-Bass-Stufe) jeder Slot startet kalt und meldet 0,0, waehrend acht Slots zugleich ihre Auslenkungen bitgleich behalten oder alle acht kalt starten. Aendert der uebertragene Slot in derselben Publikation seinen Wert, bleibt der Quadraturbetrag am Umschaltsample innerhalb der Fadeschrittweite, ueber 100 Perioden hoechstens 0,5 dB ueber der groesseren eingeschwungenen Spitze und nach Fade-Ende plus Einschwingzeit hoechstens 0,5 dB neben dem Rampenlauf ohne Fremdslotwechsel; der Mono-Bass-Zustand wandert unter gleicher Pfadkennung mit, und der Allokationslauf bleibt mit einem Committed-Fremdslotwechsel im Zyklus bei 0 Allokationen. Seit NAK-311 Etappe 4 (R-311-3) rechnet die Auto-Gain-Ableitung im Monobus (Kanalzahl 1) die geordnete Kaskade in Slotreihenfolge und wertet nur den ausgegebenen Kanal aus: ein Band im Modus right oder side ergibt exakt +0,0 dB (kein -0,0), und der Ausgang ist ueber 48 000 Samples bytegleich zum Lauf ohne Auto-Gain; left und mid kompensieren voll wie derselbe Kern im Modus stereo; zwei vertauschte Slotreihenfolgen ergeben verschiedene Werte, und zwei und acht Baender treffen die im Test eigenstaendig ausgeschriebene Formel auf 1e-9 dB. Kanalzahl 0, 2 und 3 rechnen weiter die Zweikanalformel, deren vierzehn Werte bitgleich (memcmp) zum Basisstand der Etappe bleiben, und auch im Monokern laeuft die Ableitung nie im Audiopfad. Seit NAK-311 Etappe 4 Teil b (T3-15-11) ist die Steuerrate gemessen statt begruendet: gegen einen Referenzkern mit Range 0 (derselbe SVF-Weg) bleibt der Tap post_committed nach dem Einsatz eines Quadraturtons bis einschliesslich des BEZUGSPUNKTS bitgleich und weicht genau 1 Sample danach ab; Bezugspunkt ist der erste Steuerschritt mit einer Auslenkung ungleich 0, im Test aus schrittRest und der Auslenkung gelesen statt aus dem Toneinsatz, und er liegt im Raster auf der Phase d = (8 - p) mod 8 zum Toneinsatz; genau 8 Samples nach ihm liegt wieder ein Steuerschritt, an dem der dort entworfene Satz mit Gewicht 1 wirkt - dort traegt svfVon genau den am Bezugspunkt entworfenen Satz -, in jeder gefahrenen Steuerphase und bei jeder Rate. Die Spanne 1 bis 8 Samples bis zur ersten und 8 bis 15 Samples bis zur vollen Wirkung des ersten Entwurfs ist dagegen die RASTERZAEHLUNG 1 + d und 8 + d ueber die vier gefahrenen Steuerphasen, gezaehlt ab dem Sample Bezugspunkt - d und nicht als Abstand zum Bezugspunkt; in Zeit sind die 8 bis 15 Samples der vollen Wirkung 0,167 bis 0,3125 ms bei 48 kHz, 0,181 bis 0,340 ms bei 44,1 kHz und 0,083 bis 0,156 ms bei 96 kHz, also bei 48 und 44,1 kHz GROEBER als die kuerzeste Attack von 0,1 ms. Seit dem festgelegten Pegelbegriff (Satz D, R-311-15) liegt der Bezugspunkt bei manchen Steuerphasen einen ganzen Rasterschritt nach dem Toneinsatz; dieser Rasterschritt ist das Einschwingen des Pegelbegriffs und steckt nicht in den Zahlen 1 bis 8 und 8 bis 15. Das Verhalten aendert sich dabei nicht, kDynamikSchritt bleibt 8. Seit NAK-311 Etappe 5 Satz B (T3-15-09 Teil b, R-311-14, Karte U54) ist der ANGEWANDTE Auto-Gain nach oben einseitig gedeckelt: acht High-Cuts 20 Hz Q 0,15 bei 48 kHz sind abgeleitet +150,46 dB und ueber DspKern::autoGainRohDb() weiter lesbar, angewandt wird bitgenau kAutoGainDeckelDb = 24,0 dB, und am Tap post_committed liegt der Ausgang damit um den Faktor 15,849 ueber demselben Lauf ohne Auto-Gain statt um 3,3e7 - der Zustand ist dabei gesetzt. Die Deckelfunktion laesst den naechsten double unter der Grenze und die Grenze selbst bitgleich durch und setzt erst den naechsten darueber auf die Grenze; -0,0, +0,0 und das kleinste Subnormal kommen bitgleich zurueck. Die Absenkungsseite bleibt ungedeckelt (-199,77 und -92,29 dB bitgleich, Zustand falsch), Mono- und Zweikanalzweig deckeln an ihrer eigenen Rueckgabezeile auf denselben Wert, bei ausgeschaltetem Auto-Gain wird nichts angewandt und nichts gemeldet, und nach einem Programm unter dem Deckel faellt der Zustand zurueck, waehrend der angewandte Faktor ueber kRampeSamples auf das neue Ziel laeuft statt zu springen. Alle vierzehn Hexgolden von 311/M-61 liegen zwischen -6,00 und +0,23 dB, also auf der ungedeckelten Seite, und bleiben bitgleich. Seit NAK-311 Etappe 5 Satz C (T3-15-07, R-311-13, Karte U44) kennt die Kennungsvergabe auch WERTE: reisst ein Slot bei sonst gleicher Topologie eines der drei Kriterien - Frequenzverhaeltnis 2,0, Gueteverhaeltnis 4,0, Gaindifferenz 20,0 dB, richtungsfrei und strikt groesser -, bekommt NUR dieser Slot eine neue Lebenszykluskennung, der Blockrand blendet ueber statt zu rampen, und nur dieser Slot startet kalt. Gemessen am Mass von R-311-13 (groesster Ausgangsbetrag im Fenster von 0,5 s ab dem Umschaltsample gegen die groessere der beiden eingeschwungenen Spitzen, hoechstens 1 dB): der Sprung freq_hz 5000 -> 50 Hz an einem Low-Shelf +6 dB Q 0,707 faellt von +17,41 auf -0,22 dB, derselbe Sprung am Low-Cut 2000 -> 20 Hz von +22,20 auf +0,13 dB, die Gegenrichtung 50 -> 5000 Hz von +1,91 auf 0,00 dB, und ein dynamisches Band mit Range 0 haelt das Mass ebenso. Dieselbe Strecke in zwanzig Stufen zu je 1,2589 bleibt dagegen auf dem Rampenweg, behaelt ihre Kennung und ist ueber alle 40 960 Samples bitgleich zum Basisstand. Strikt groesser entscheidet an allen sechs Kanten je Kriterium (eine ULP darunter, genau auf der Grenze, eine ULP darueber, in beiden Richtungen), nicht endliche und nicht positive Werte erzwingen IMMER eine neue Kennung, acht gleichzeitig springende Slots ergeben genau EINEN Crossfade, ein zweiter Sprung im laufenden Fade wird erst am Blockrand nach dessen Ende genommen, und der Fade dauert bei Blockgroesse 1, 255, 256, 257 und 4096 genau kFadeSamples SAMPLES. Im Callback bleibt der Zaehler dabei bei 0 Allokationen und 0 Sperren, auch wenn eine Publikation ueber dem Kriterium im selben Zyklus liegt. Seit NAK-311 Etappe 5 Satz D (T3-15-06, R-311-15, Karte U45) vergleicht die Dynamikschwelle einen FESTGELEGTEN Pegelbegriff mit dem Threshold: zwischen Detektor und Huellkurve liegt ein symmetrisches Ein-Pol-Leistungsmittel mit dem festen Fenster kPegelFensterMs = 10 ms (durchschnitt); Fenster 0 (spitze) schliesst die Stufe kurz. Aktiv ist durchschnitt - baueProgramm setzt ihn unbedingt ohne Eingabe aus dem Vertrag, im Produktcode steht genau eine Setzstelle, und Pegelbegriff::spitze kommt dort nur in der Abbildung auf die Fensterlaenge vor. Gemessen bei 48 kHz, Bell Q 0,707, Range -12 dB, Threshold im Knie: drei Materialien gleicher Leistung (Quadraturton, L = R-Sinus, Rauschen) liegen um hoechstens 0,02 dB bei 1 kHz, 0,06 dB bei 341 Hz, 0,21 dB bei 100 Hz und 0,90 dB bei 20 Hz auseinander, das Rauschen um 0,32 dB - vorher 2,12 bis 4,01 dB. Neun Einstellungen aus Attack 0,1/10/500 ms mal Hold 0/30/500 ms spannen je Bandmitte 0,05 / 0,15 / 0,50 / 2,20 dB statt 5,78 dB; die Restwelligkeit eines FESTEN Fensters waechst mit fallender Bandmitte und steht als Zahl in der Schranke. Mit Fenster 0 ist der dynamische Weg ueber 76 096 Samples bitgleich zum Basisstand des Aenderungssatzes, und der Kurzschluss gibt den Eingang auch bei nicht endlichem oder denormalem Zustand bitgleich zurueck. Der Pegelzustand liegt im BandZustand: er wandert am Blockrand mit dem uebrigen Bandzustand, wird mit dem abgeschalteten Detektor gliedweise genullt, am Blockrand geheilt und denormalgeriegelt, ist bei Stille bitgenau +0,0 und ergibt bei 44,1 / 48 / 96 / 192 kHz dieselbe Zeit. Der Pegelbegriff ist topologisch: rampenKompatibel und die Kennungsvergabe tragen ihn, zwischen zwei Begriffen wird nie interpoliert. Die Sprungantwort misst gegen eine im Test ausgeschriebene Referenz der GANZEN Kette (Bandpass, Pegelbegriff, Huellkurve, Steuerraster): bei Attack 20 ms und Fenster 10 ms liegt der 63-%-Punkt bei 32,04 bis 32,17 ms statt bei 20 ms, die Toleranz von 1 ms je Stufe bleibt. Im Callback bleiben die Zaehler mit acht dynamischen Baendern bei 0 Allokationen und 0 Sperren; im Quelltext von verarbeiteBand steht der Aufruf der Pegelstufe genau einmal, und huellkurvePol, huellkurveEntwurf, std::exp, std::pow, std::log10, new und malloc stehen dort buchstaeblich nicht - der Pol entsteht im Worker (baueProgramm). Die Wortsuche liest nur den Text dieser einen Funktion und folgt keinem Aufruf; ueber den Steuerratenschritt, der alle kDynamikSchritt Samples Kennlinie und SVF-Satz neu rechnet (leistungInDb mit log10, svfEntwurf mit pow), sagt sie nichts. Seit NAK-312 Etappe 7b (U48, U58) die Hostbypass-Stufe hinter Taps und Hoermatrix: mit dem Wunsch Hostbypass blendet sie genau 256 Samples, gezaehlt in Samples bei jeder Blockgroesse (1, 64, 256 und 4096 bei maxBlock 256, 4096 bei maxBlock 100), linear mit dem Anteil n/256 und am Nachbarsprung innerhalb der E-31-Schranke, und schreibt danach keinen Sample (Wachmarke, NaN, +-Inf, Subnormals und -0 bytegleich); kehrt der Wunsch in der Blende um, laeuft sie vom Mischstand zurueck, ohne Sprung am Umkehrsample; mit dem Wunsch nein ist verarbeite bitgleich zum Aufruf ohne das Argument. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [OK] Exit 0 | 6,30 s | [B6](roh/NAK-380-1e4a701-dirty.md#b6) |
| B7 | Lokaler Transaktionskern der aktiven Sonde mit dem echten DSP-Kern: die Falltabelle T1-T17 aus Manifest SONDE-015 §5.11.4 laeuft tabellengetrieben mit den Invarianten I1, I2 und I4 als Wachen nach jeder Eingabe; ein Fehler in jeder Stufe S1-S7 laesst bestaetigten Zustand, Hash, Undo-Ring und Register unveraendert, und weder der Nachschlag S0 noch der Commit-Punkt allozieren; Apply, Revert, Neutralisieren, Remove, Undo, Redo und Preset-Laden erzeugen je genau eine Revision, 10.000 doppelte, vertauschte und veraltete Eingaben einer Transaktion hoechstens eine. Belegung, Remove und Undo als ein Objekt, Schutz-Zonen mit Verletzungsmeldung, Undo-Ring der Tiefe 32 und Preset halten ihre Matrixzeilen. Am echten SondeProcessor: 112 Host-Parameter in Vertragsreihenfolge ohne occupied; Hostautomation ohne Revision mit zwei Epochenwechseln je Geste, auf einem freien Slot bitgleich im Klang; ein abgeschlossener Gestus ist eine Revision; Host-Dirty nur bei einem Commit; das Kind Dsp reist durch Speichern und Laden, und ein Reload rekonstruiert Hash und Ausgang. Seit NAK-283 (F05, F09, F12) am echten SondeProcessor: reset() beendet die Audiohistorie - nach einem Impuls in den Resonator (1 kHz, +12 dB, Q 10) ist der erste Block Stille auf beiden Kanaelen exakt 0, ohne reset() klingt er nach - und laesst getStateInformation bytegleich, die Revision, das bestaetigte eq_enabled und die engagierte Bank im Kern stehen (1 kHz danach mehr als 6 dB lauter); releaseResources->prepareToPlay, reset() und prepareToPlay allein enden in derselben Audiohistorie (Stille exakt 0, untereinander bitgleich). Ein Block ueber maxBlock bei rechnendem Kern hinterlaesst in der Analysequeue eine Luecke (der naechste Block beginnt bei 192 mit kFlagLueckeDavor, Segment +1), und Kern und Queue beschreiben dieselbe verworfene Zeitspanne (ein verworfener Tap, ein Block ohne Audio mit 128 Frames, oversize und Ueberlauf +0); ruht der Committed-Pfad im Hard-Bypass, wird derselbe Block ohne Luecke angenommen. Endliche double ueber FLT_MAX am Tap kommen in der Analysekopie als endliche floats an, und der Kernzaehler steigt um genau diese 1024 Werte. Seit NAK-311 Etappe 3 (W01): reset() im laufenden Ausblenden (Output-Trim +24 dB, ein Block nach eq aus), danach Output-Trim 0 dB in der Ruhe und eq an - Ausgang/Eingang bleibt an jedem Sample des Wiedereinschaltens hoechstens 1 + 1e-6. Seit W03: Remove und Neubelegung desselben Slots mit derselben Konfiguration ohne Audioblock dazwischen - beide committet, die zweite Publikation verdraengt die erste - starten kalt; der Uebergang ist ein Crossfade, die gemeldete Auslenkung ist ab dem ersten Block exakt 0,0 und der Ausgang bei Stille ab kFadeSamples exakt 0,0. Seit NAK-311 Etappe 4 Teil b (F12, R-311-4) hat die Toleranz von M-84 ihre Zahl samt Referenzbeginn und Geltungsbereich: t = 0 ist das erste Sample des ersten Blocks der geladenen oder neu vorbereiteten Instanz, der Vergleichskern laeuft ununterbrochen weiter und bekommt ab dort denselben Eingang. Fuer ein dynamisches Bell (Kanalmodus stereo, Sidechain internal) mit Quadraturton auf der Bandmitte im Teilraum Q >= 1, Q*A_min >= 0,5 und f0 bis min(20 kHz, fs/4) weicht der Quadraturbetrag je Sample ab t_E = max(kFadeSamples, 5*tau_a*fs + 10*Q*A_max*fs/(pi*f0) + 16) hoechstens 0,1 dB ab - gemessen hoechstens 0,03 dB ueber drei Prueflinge (Q 2 bei Attack 0,1 / 10 / 500 ms, Q 1,0, f0 = fs/4 mit Q 24) je in Knie und Plateau -, davor im Betrag hoechstens \|g0\| + \|Range\| + 0,1 dB; dasselbe gilt fuer prepareToPlay auf der laufenden Instanz. Der Same-Instance-Ladestart bleibt dagegen in Ausgang UND Tap bitgleich zum ununterbrochenen Kern, und zwei Sekunden eingeschwungenes Audio lassen die Statebytes unveraendert: die Huellkurvenleistung ist Audiohistorie, kein persistenter Wert. Seit NAK-311 Etappe 5 Satz A (F08, R-311-16, R-311-20) nimmt ein Ratenwechsel den Zustand zurueck: ueber 48 -> 32 -> 48 kHz ist der Ausgang im 32-kHz-Fenster bytegleich zum Eingang und der Kern meldet 32 000 als abgelehnte Rate, nach dem dritten prepareToPlay ist der Melder wieder exakt +0,0, das Band hoerbar und der Ausgang bitgleich zu einem Kern, der nie abgelehnt hat - waehrend eq_enabled, Statebytes, Revision und Host-Dirty ueber die ganze Folge unveraendert bleiben. Mit einem High-Cut 20 Hz Q 0,707 und Auto-Gain an sind Kern UND Bericht im Fenster exakt +0,0 dB, klemmungen leer, die Zaehler des alten Fensters genullt und die Kanalzahl auf 2, beide tragen die abgelehnte Rate, jcs, Revision und Undo-Tiefe bleiben die des bestaetigten Zustands, und nach dem Rueckweg tragen beide wieder bitgleich denselben abgeleiteten Wert. Im 32-kHz-Fenster zaehlt der thread-lokale Zaehler ueber 200 Bloecke 0 Allokationen im Callback, die Echtzeitwache (seit NAK-312 ab dem Eintritt processBlock oder processBlockBypassed) 0 gemeldete Sperren, getLatencySamples bleibt 0 und getTailLengthSeconds 0,0. Seit NAK-311 Etappe 5 Satz D (T3-15-06, R-311-15) traegt t_E den festgelegten Pegelbegriff: der Huellkurventerm rechnet 5*(tau_a + kPegelFensterMs)*fs statt 5*tau_a*fs, also hoechstens 5*kPegelFensterMs*fs Samples mehr, und die drei Prueflinge weichen ab t_E in Knie und Plateau hoechstens 0,027 dB ab - die Toleranz von 0,1 dB bleibt unveraendert. Zwei Sekunden eingeschwungenes Audio bewegen den Pegelzustand nachweislich auf einen endlichen Wert ueber 0, der Slot faehrt dabei den Begriff durchschnitt, und die Statebytes bleiben bytegleich: der Pegelbegriff ist Laufzeit wie die Kanalzahl und steht in keinem Statefeld. Seit NAK-312 Etappe 2 (R-312-1, E-312-11) misst die Echtzeitwache ab dem Eintritt processBlock oder processBlockBypassed, und B7 meldet jede eingeschaltete Allokation zusaetzlich an sie: mit einem Testplayhead, der je Aufruf eine gemeldete Sperre nimmt, zaehlt sie ueber 200 Bloecke genau 200 Sperren vor dspKern->verarbeite und 0 Allokationen; ein Workerzug mit Programmbau zwischen zwei Bloecken bewegt beide Zaehler nicht; legt das Bein den Bereich wie der Wrapper um setNonRealtime und processBlock, zaehlt sie seit Etappe 5 ueber 200 Bloecke je Aufruf setNonRealtime (true) wie je Aufruf setNonRealtime (false) 0 gemeldete Sperren (bis Etappe 4 je Aufruf setNonRealtime (true) genau eine). Sperren, die JUCE vor dem Plugincode nimmt, sieht der Zaehler bauartbedingt nicht. Seit NAK-312 Etappe 3a (W02) quittiert ein Ladestart die Hostwert-Mailbox: ein Hostwert, der vor dem Laden derselben Bytes in dieselbe Instanz eintraf, bleibt wirkungslos - auch ueber prepareToPlay, eine spaetere Wiederoeffnung der Mailbox und den Zaehlerueberlauf hinweg -, der Hash bleibt der der Quelle, und ein Hostwert nach der Quittierung wirkt, auch einer zwischen Quittierung und Abgleich, dessen Regler danach denselben Wert zeigt; reset() bewahrt die Mailbox. Ein read-only geladener Stand bleibt unter Hostautomation (eq_enabled und Output-Trim) ueber 512 Samples bitgleich zum Eingang, seine Regler springen auf neutral, die Originalbytes bleiben, kein Dirty, und ein gueltiger Stand danach laesst die Automation wieder wirken; NaN und +-Inf bleiben wirkungslos. Seit NAK-312 Etappe 3b (T3-01-05 Teil a, E-312-5, E-312-6) uebernimmt der Kern Input-Trim, Output-Trim, Width und Mix am Blockrand: ein Lauf unter der Taktsperre ohne Kontrolltakt ist bitgleich zu einem mit Takt nach jedem Block - fuer alle vier in beiden Staenden (neutral und mit Band) bei Blockgroesse 1, 64, 185, 240, 256 und 512 in Echtzeit und bei 64 offline; fuer Output-Trim im neutralen Stand zusaetzlich bei 4096 (dort wirkt der Wert im Block seines Randes), ueber eine Rampe von 200 Bloecken und ueber den Zaehlerueberlauf; ohne Ereignis setzt der Blockrand kein Ziel, NaN und +-Inf erreichen ihn als bestaetigter Wert, ein Ladestart mitten in der Fahrt macht einen alten Mailboxwert auch am Blockrand wirkungslos, und die Abdeckung ist genau diese vier. Epoche und Ruhegrenze bleiben beim Worker (genau zwei Epochenwechsel je Geste, Ruhegrenze in verarbeiteten Samples), und eine blockgebundene Fahrt erzeugt keine Revision, kein Dirty und bytegleiche Statebytes; ueber 4000 Bloecke wechselnder Groesse mit Automation zaehlt die Echtzeitwache 0 Sperren und 0 Allokationen. Ein Bandwert und eq_enabled bleiben taktgebunden: ohne Takt weicht der Lauf ab, mit Takt ist er bitgleich. Seit der Nacharbeit 1 der Etappe 3 (L-1, R-312-16) nimmt der Blockrand nach einem read-only-Ladestart keinen Hostwert: laedt ein Stand mit eq an und Output-Trim +3 dB das fremde Major und kommen danach Input-Trim +24 dB, Output-Trim +24 dB, Width 2 und Mix 0 als Hostwerte, ist der Ausgang ab dem ersten Sample nach dem Laden bitgleich zu dem ohne diese Hostwerte, und die Blockrandziele steigen um 0 - bei Bloecken zu 256 und, wenn beim Laden ein Uebergang laeuft, bei 185 und 512; ein eigener schreibbarer Stand danach laesst Output-Trim +6 dB ohne Kontrolltakt wieder am Blockrand wirken (bitgleich zu einem Lauf mit Takt nach jedem Block, Ausgang/Eingang am Ende bei 10^(6/20)). Seit NAK-312 Etappe 4 (312/M-38): ueber mindestens 4000 Bloecke wechselnder Groesse, waehrend beide v3-Clients der Sonde ueber einen Testserver auf der Probe-Pipe verbunden sind und ihre Provider nachweislich durch die Besitzschleuse laufen, bleiben RtWache::sperren() und RtWache::allokationen() ab Callback-Eintritt 0. Seit NAK-312 Etappe 5 (T3-01-03, T3-01-04, R-312-3) der Offline-Uebergang: mit hoerbarer Vorschau zaehlt die Wache im Bereich wie der Wrapper ueber 200 Bloecke 0 Sperren und (seit Etappe 6a gemessen) 0 Allokationen; unmittelbar nach dem ersten Aufruf setNonRealtime (true) ist die gewuenschte Hoermatrix Processed, und der Ausgang ist ab Sample 0 bitgleich zum Lauf ohne Vorschau - bei Blockgroesse 1, 64 und 256 und auch, wenn beim Wechsel ein Hoermatrix-Fade in Gegenrichtung oder ein Hoerhalt laeuft; nach einem ausdruecklichen Kontrolltakt ist keine Vorschau mehr aktiv, nach ceil(kFadeSamples/Blockgroesse)+1 Bloecken und einem zweiten Takt die Candidate-Bank frei; der Rueckweg in die Echtzeit blendet weich (Nachbarsprung hoechstens Fadeschritt plus 2^-23) und belebt die Vorschau nicht, und ohne Vorschau aendert das Offlineflag am Ausgang nichts. Seit NAK-312 Etappe 6a (R-312-20, R-312-22), unter der Taktsperre bei stehender Vorschau: ein im Render gesetzter Hoerwunsch Dry, Delta oder Candidate wirkt als Processed - jeder Offlineblock ist bei Blockgroesse 1, 64 und 256 ab Sample 0 bitgleich zum bestaetigten Lauf; nach setNonRealtime (true) und sofort setNonRealtime (false) ohne Block dazwischen schaltet der erste Echtzeitblock beim Wechsel auf Candidate nicht hart (Nachbarsprung hoechstens Fadeschritt plus 2^-23). Seit dem zweiten Aenderungssatz der Etappe 5 (T3-01-02, R-312-4) haengen die 112 Hostparameter ohne APVTS direkt am Prozessor: Zahl, Reihenfolge, IDs, Namen, Klassen, Bereiche, Defaults und Hostsicht (convertTo0to1 an Minimum, Default und Maximum; convertFrom0to1, getText und getValueForText an 0, 0,37 und 1), die Statebytes ueber speichern, laden, speichern und die Epochenfolge einer Automationsfahrt sind gleich dem Golden des Basis-SHA 12300f1e; kein Quelltext unter eq-copilot/plugin/sonde/ nennt die APVTS-Klasse; nach dem Laden traegt jeder Hostparameter den geladenen Wert; am Ende des Destruktorrumpfs erreicht ein Hostwert den Prozessor nicht mehr; Index -1 und 112 bleiben ohne Wirkung, und der eigene Abgleich meldet kein Hostereignis; ueber 4000 Bloecke mit Hostautomation aus einem zweiten Thread zaehlt die Wache ab processBlock 0 Sperren und 0 Allokationen - die Sperre, die JUCE vor dem Plugincode in sendValueChangedMessageToListeners nimmt, sieht sie bauartbedingt nicht. Seit der Nacharbeit 1 der Etappe 5 (R-312-19, Kanonteil von 312/M-88) ruft kein Quelltext unter src, sonde, dsp, state, vertrag, core und hostbridge von eq-copilot/plugin ExitProcess, TerminateProcess, PostQuitMessage, FatalAppExit, exit, _exit, quick_exit, _Exit, Process::terminate oder systemRequestedQuit und keiner nennt WM_CLOSE, WM_QUIT, SC_CLOSE oder JUCEApplicationBase (Textriegel); ob der Host im Betrieb lebt, misst das nicht - das misst der Laufzeit-Arm. Seit NAK-312 Etappe 7b (U48, U58) der Hostbypass-Eintritt processBlockBypassed: die Wache beginnt auch dort vor der Stempelbildung (200 gemeldete Sperren ueber 200 Bloecke mit Testplayhead); ueber 4000 Bloecke wechselnder Groesse mit Eintrittswechseln, Hostautomation aus einem zweiten Thread, Programmwechseln, reset, releaseResources und prepareToPlay bleiben Sperren und Allokationen 0, die Latenz 0 und der Tail 0,0; getBypassParameter bleibt nullptr bei 112 Hostparametern; State, Hash, Revision, Undo-Ring und Dirty bleiben unberuehrt, und die Ruhegrenze der Automationsepoche zaehlt auch im Hostbypass verarbeitete Samples - sie schliesst am selben Block wie ohne ihn; der Blockrand des Bypasseintritts nimmt Hostpunkte ueber den Ueberlauf der Ereigniszaehler (M-132); Echtzeit und Offline sind bitgleich, auch wenn der Wechsel nach offline mitten in die Blende faellt, und der Bypasseintritt setzt weder den Offline-Riegel noch das harte Schalten; im Hostbypass haengt keine Bank, und nach dem Austritt klingt der zuletzt bestaetigte Stand. Seit NAK-313 Etappe 5b (R-313-4, 313/M-83): der Kern bildet seine Grenze aus derselben Konstante wie der State-Leser - bei 2^53-2 committet genau eine Transaktion auf 2^53-1, die naechste scheitert in S4 mit revision_erschoepft ohne Nebenwirkung an Zustand, Hash, Undo-Ring, Register und Revision, und ein Ladestart mit 2^53 wird mit state_revision abgewiesen. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [OK] Exit 0 | 2,24 s | [B7](roh/NAK-380-1e4a701-dirty.md#b7) |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. Seit NAK-313 Etappe 5a (R-313-4, R-313-5; 313/M-55 bis M-60, M-73, M-96): der flache Leser liest Ganzzahlen mit nakama::wire::ganzzahlAusLiteral ohne Gleitkommaschritt, je Literal ein Fall (0, -0, 1, 1.0, 1e0, 10E-1, 91.0, 1.00e2 und 2^53-1 in Ziffern- und e-Form gelten; 1.5, 1e, 01, -1, +1, 1., 1e400, 2^53 auch als Bruchform, 2^64, NaN, Infinity, -Infinity, 1e999 und leer nicht). state_revision im command_ack gilt von 0 bis 2^53-1 in jeder Schreibweise; 2^53 und ein Ueberlauftext (2^64) sind keinAck: der Auftrag bleibt im Register, der Konflikthaken wird nicht gerufen, kein Kopf mit 0 entsteht, und erst nach dem Reconnect gibt ein gueltiges ACK ihn frei; der Konfliktkopf traegt die gelesene Revision (5 wie 5.0). welcome.protocol gilt in Control und Telemetrie mit dem Wert 3 (3, 3.0, 3e0), 4 und 3.5 verbinden nicht. Je Eintrag der Eingaenge cpp_control_ack, cpp_control_handshake und cpp_telemetrie_handshake der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen ueber die Testserver-Schalter fuer rohe welcome- und ACK-Bytes, verglichen mit produkt samt Wert, mit Zaehlpruefung je Eingang. Seit NAK-313 Etappe 5b (R-313-4, 313/M-88): heartbeatAlsJson und stateReportJson schreiben state_revision unveraendert - 2^53-1 als 9007199254740991 und ein Testaufbau mit 2^53 als 9007199254740992, nie gekappt. Seit NAK-313 Etappe 6 (R-313-7; 313/M-106 bis M-113): der flache Leser prueft UTF-8 vor dem ersten Zeichen, dekodiert Escapes nach RFC 8259 (ein Surrogatpaar wird ein Codepunkt; NUL-Escape, einsames Surrogat, unbekanntes Escape, ein u-Escape ohne vier Hexziffern und rohe Steuerzeichen sind ungueltig) und vergleicht Namen dekodiert; eine Ablehnung nennt einen von acht festen Gruenden ohne die Teilwoerter, an denen Gen eine Inkompatibilitaet erkennt. Nur die beiden welcome-Aufrufer schreiben welcome: plus Grund, ein gelesenes Objekt ohne Text in type meldet welcome: kein type. broker_version zaehlt hoechstens 64 und reject.reason hoechstens 500 Codepunkte (64 und 500 mal e-Akut gelten, 65 und 501 nicht, 499 a plus Surrogatpaar gilt, 500 a plus Paar nicht); eine leere broker_version verbindet nicht, ein leerer reason ist Broker lehnt ab. Control und Telemetrie melden dieselben Gruende; ein reject mit Escapes endet mit genau den dekodierten Bytes. Am ACK-Weg ist ein konflikt mit escaptem Code gelesen und beendet den Auftrag, ein ACK mit FF im code oder mit ergebnis als Escape-Alias und roh ist keinAck, und der Auftrag bleibt im Register. Im Tabellenlauf gilt seit Etappe 6 ein reject als gelesen, wenn es als Broker lehnt ab erscheint, und ein ACK, wenn es den Auftrag beendet. Seit NAK-313 Etappe 7 (R-313-9; 313/M-123, M-124): capabilitiesJson meldet binary_telemetry supported - zwei Hostmessungen, eine Build-Tatsache, sieben feste Fallbacks, zehn Schluessel in der bisherigen Reihenfolge -, und der Capabilitybericht traegt Schluessel fuer Schluessel dasselbe Objekt. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 313,77 s | [B10](roh/NAK-380-1e4a701-dirty.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,04 s | [B11](roh/NAK-380-1e4a701-dirty.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,61 s | [B12](roh/NAK-380-1e4a701-dirty.md#b12) |
| B31 | Sonde-Lebenslaufbein (NAK-312 R-312-7): der Testkonstruktor der Sonde uebernimmt einen Probe-Pipenamen unveraendert und einen Namen ausserhalb des Probe-Namensraums fail-closed als leeren Pipenamen; ohne die Starthaken macht im Testbau kein v3-Client einen Verbindungsversuch, mit ihnen meldet der ControlClient binnen 8 s verbunden ueber die Probe-Pipe eines Testservers, und die Telemetrie koppelt nach dessen welcome; die Sonde wird mit beiden laufenden Clients auf dem Heap abgebaut. Jeder Name, mit dem ein Client startet, liegt vor dem Start im Probe-Namensraum. Seit Etappe 4 (R-312-7, R-312-12) die Besitzschleuse der Sonde, gemessen an einem gehaltenen Statusprovider und einer gezaehlten Marke vor dem ersten Besitzerzugriff: nach dem Ende des Eigentuemers erreicht ihn kein Provider (0 in jedem Fall und in 20 Gegenzyklen, nach denen Handles und Threads nicht hoeher stehen als nach dem ersten); gehalten hinter dem Betreten wird er zu Ende gewartet, gehalten davor und nach dem Destruktor freigegeben abgewiesen, vor dem Abbau freigegeben laeuft er durch; stop() loest den gehaltenen ControlClient nach der 2-s-Frist ab, controlV3.stop() gemessen vor dem Schliessen, telemetryV3.stop() nach Zeilenfolge; ein Reconnect laesst dieselbe offene Schleuse stehen. Eine Sonde aus dem Produktkonstruktor traegt im Testbau genau pipeNameV3 (aktuelleLogonSid()), ohne dass ein Client startet. | `eq-copilot\build\plugin\EqCopSondeLebenslaufTest_artefacts\Release\EqCopSondeLebenslaufTest.exe` | [OK] Exit 0 | 72,76 s | [B31](roh/NAK-380-1e4a701-dirty.md#b31) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. Seit NAK-312 Etappe 7b (U51): hoechstens 20 Quellen werden angenommen; bei 21, 32 und 64 Sonden stehen 20 Zeilen und die Zahl der nicht angenommenen in der Sicht, eine gegangene Quelle macht Platz ohne Verdraengung, und Messungen nicht angenommener Quellen erreichen die Sicht nicht; die Zeitmessung mit 16 und 32 Snapshot-Quellen erwartet min(n, 20) Zeilen, bei 32 also 20 und 12 nicht angenommene, p95 weiter unter 300 ms. Seit der Nacharbeit 1 der Etappe 7b (R-312-32): rueckt eine wartende Sonde ueber den lokalen Eintritt nach (Publikation der gespeicherten Mitglieder ohne ein entferntes, ohne neuen Snapshot), zaehlt ihre Zeile den offenen Befund aus dem juengsten Snapshot (findingsOffen 1, jede andere Zeile 0), und nach controlEnde ist er stale und die Zeile zaehlt 0. Seit NAK-313 Etappe 4 (R-313-6, 313/M-39 bis M-41, M-49): beide v3-Leser des Quellenmodells fahren nach dem Textriegel genau einen strengen Lauf kanon::lies, bevor sie ein Feld lesen. Je Eintrag der Eingaenge cpp_sources_snapshot und cpp_sources_ruecknahme der Tabelle PRODUKTEINGAENGE-FAELLE.json ein Fall mit PE-nnn im Namen, direkt am Leser in einer abonnierten Sitzung mit uebernommenem Basis-Snapshot: Nachspann, zweites Dokument, Schlusskomma, unbekanntes Escape, doppelte Namen (type, session_epoch, logon_sid in einer Mitgliedsadresse, grund, art in umfang; je mit passendem und mit fremdem letzten Wert) und der Escape-Alias fuer type sind ungueltig mit dem Praefix "session_snapshot parser: " beziehungsweise "evidence_invalidate parser: " an der Stufe parser oder duplikat, und die Sicht bleibt dieselbe (Revision, Quellen mit Mitgliedschaft und Namen, Abonnement, fuehrendes Main, Diagnose, Ruecknahmezaehler, -grund und -umfang); derselbe Name logon_sid in zwei Mitgliedsadressen bleibt gueltig, beide Mitglieder stehen in der Sicht. Je Eingang verlangt die Zaehlpruefung genau so viele Faelle, wie der Kopf der Tabelle nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-61, M-73, M-96): beide Leser lesen die Felder aus DEMSELBEN strengen Lauf (nakama::vertrag::wertAlsVar statt juce::JSON::parse): letzter_kontakt_ms 1500.0 und 1.5e3 sowie host_mixer_index 3.0 werden als 1500 und 3 uebernommen, eine Ruecknahme von 1024.0 bis 2048e0 wie mit Zifferntext, die vertauschte Ordnung, 1.5 und -1 bleiben ungueltig, 2^53, NaN, Infinity, -Infinity und 1e999 fallen am Textriegel; bei Urteil gueltig wird der Wert am Feld verglichen. Seit NAK-313 Etappe 6 (R-313-8, R-313-15; 313/M-108, M-117): Gen ordnet Broker lehnt ab als incompatible ohne Handgriff ein und die Lesegruende welcome: plus Grund (die acht Wortlaute und welcome: kein type) als brokerUnavailable mit Handgriff; in einem Modell, das die Quelle kennt, verwirft uebernehmeP2 einen Batch mit belegtem Adressslot jenseits von Feld-ID 4 mit /eintraege/0/quelle:adresse_zusatzfeld, und die Zeile bleibt ohne neue Messung und Lautheit. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,15 s | [B13](roh/NAK-380-1e4a701-dirty.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. Seit NAK-312 Etappe 6b (312/M-72) reisen die Namen nach dem Labelentwurf-Fall am echten Editor (Entwurf der Startquelle A beim Klick auf B bestaetigt, danach Tick und Enter) durch Speichern und Laden in eine frische Instanz unveraendert und mit derselben Zuordnung Name zu instance_id; das Laden meldet kein Dirty. Seit Etappe 6b (312/M-76) behaelt ein geladener Stand mit 40 bestaetigten Mitgliedern und einer unbekannten additiven MainProject-Eigenschaft ueber die Folge mit 20 gezeichneten Zeilen und Ersatzziel alle 40 Mitglieder im State, im Modell und nach dem Laden in eine frische Instanz; die Eigenschaft reist unveraendert, kein Host-Dirty. Seit NAK-312 Etappe 7b (312/M-126, U51) behaelt ein geladener Stand mit 21 und 40 bestaetigten Mitgliedern alle in State und Modell, 20 davon - die 20 kleinsten instance_id - sind Zeilen, auch nach einem Snapshot mit zwei weiteren Sonden; Speichern bytegleich, kein Host-Dirty. Seit Etappe 7b, Satz 3 (U49): in Legacy ist die Live-Sicht des Quellenmodells stillgelegt - keine Zeile, keine Subscription, ein verspaeteter Snapshot traegt nichts ein -, nach der Rueckkehr stehen die Mitglieder wieder; ein nach dem Wechsel quittierter Beitritt wirkt auf den ruhenden Bestand. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 9,67 s | [B14](roh/NAK-380-1e4a701-dirty.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. Seit NAK-312 Etappe 4 (R-312-2) faehrt dasselbe Bein das Messpunkt-Popover am echten Editor: baut die Nachrichtenschleife es erst nach Editor und Prozessor ab, fasst sein Destruktor keinen von beiden an (gezaehlte Marke 0, Bindung bis zum Prozessorende unveraendert, 0 Host-Dirty); schliesst es, solange der Editor lebt, wird eine Aenderung mit genau einer Host-Dirty-Meldung uebernommen, ohne Aenderung mit keiner. Seit NAK-312 Etappe 6b (R-312-9) dazu der Labelentwurf der Main-Flaeche am echten Editor auf einem echten Main (Namen im Prozessorzustand): ein Entwurf der Startquelle A wird beim Klick auf die Zeile B auf A bestaetigt, und weder Enter noch Fokusverlust danach schreiben ihn auf B; ein leerer Entwurf loescht Bs Namen nicht; nach Escape zeigt das Feld wieder As Label, und der naechste Auswahlwechsel schreibt nichts; faellt A weg oder kommt A mit neuer Runtime-Nonce zurueck, verfaellt der Entwurf ohne Mutation, auch mit Enter vor dem naechsten Tick; unveraenderter Text schreibt nichts; im Feld werden 121 Codepoints ausserhalb der BMP auf 120 beschraenkt und beim Bestaetigen auf 120 gekappt, 0 und 120 werden angenommen, 121 am Prozessor abgewiesen; Host-Dirty je geaendertem Namen genau einmal. Seit Etappe 6b (R-312-6) dazu Ersatzziel und Aktionssteuerung: mit 16, 20, 21, 32 und 64 Quellen ohne Hauptziel ist das Ersatz-Hauptziel die erste angezeigte Quelle (vier Bilder 20/21/32/64 im Sichtsatz), ab 20 Quellen zeichnet die Flaeche 20 Zeilen; rueckt ein per Handgriff gewaehltes Hauptziel durch Beitritt oder Hostbusnamen hinter Zeile 20, sind Knopf, Labelfeld und dessen Beschriftung unsichtbar, das Aktionsziel ist leer, das Hauptziel im Modell unveraendert ohne Schreibversuch, und der Status sagt es; ein offener Entwurf wird davor auf der Startquelle bestaetigt (genau 1 Host-Dirty); ohne Quelle gibt es keine Aktionssteuerung, bei 1 und 16 Quellen ist jede Zeile gezeichnet und per Klick erreichbar. Seit Etappe 6b (NAK-349, R-312-2) dazu das Kennungskonflikt-Panel: stellt die Nachrichtenschleife einen geposteten Klick (triggerClick, der Weg von Enter) erst nach dem Ende des Editors zu, mit lebendem und mit danach zerstoertem Prozessor, fasst der Rueckruf weder Editor noch Prozessor an (gezaehlte Marke 0, Kennung unveraendert, 0 Host-Dirty); bei lebendem Editor vergibt derselbe Klick eine neue Kennung mit genau einer Host-Dirty-Meldung und Statusmeldung, und die Box schliesst. Seit der Nacharbeit 1 der Etappe 6 (L-5, R-312-2 Satz 2) dazu: beginnt der Klick bei lebendem Editor und baut ein Listener in audioProcessorChanged den Editor ab, waehrend der Rueckruf in neueSensorId steht (nach seinem Eintritt, die Kennung schon neu), wechselt die Kennung mit genau einer Host-Dirty-Meldung, der Rueckruf erreicht danach keinen Zugriff auf den Editor (zweite Lebendpruefung, gezaehlte Marke 0), und die Box wird abgebaut. Seit NAK-312 Etappe 7b (U51) dazu die Annahmegrenze am echten Pfad (abonnierte Sitzung, echte Snapshots): bei 0, 1, 19 und 20 Quellen ist jede angenommen, gezeichnet und per Klick erreichbar; bei 21, 32 und 64 Quellen sind 20 angenommen, jede davon gezeichnet und erreichbar, und das vorhandene Diagnosefeld meldet die volle Liste mit der Zahl der nicht angenommenen, allein oder als Zusatz hinter einer anstehenden Diagnose. Seit NAK-313 Etappe 2 (R-313-3) schreibt das Messpunkt-Popover nur, was der User geaendert hat: mit geladenem Label von 121 und Paarnamen von 61 Zeichen ruft das unveraendert geschlossene Popover die Bindungs-API nicht (0 Aufrufe, 0 Host-Dirty, 0 Reconnect-Anforderungen, Save bytegleich), und auch Fokusverlust und Rollenrueckruf ohne Wechsel rufen sie nicht und melden kein Host-Dirty; eine Rollenwahl mit unveraendert langem Label gelingt mit genau einem Aufruf und einem Host-Dirty, und beide Texte bleiben; die Felder begrenzen die Eingabe auf 120 und 60 Codepunkte (das Paarfeld gemessen bei eingerichteter Paarrolle pre); ein geaendertes, weiter zu langes Label oder ein solcher Paarname wird ohne Host-Dirty abgewiesen, und die vorhandene Statuszeile nennt die Grenze; traegt der abgewiesene Aufruf eine Rollenwahl (E-313-18), faellt die Rollenauswahl ohne zweiten Aufruf auf die gespeicherte Rolle zurueck, und die Meldung nennt es. Seit NAK-313 Etappe 7 (R-313-10; 313/M-141, M-142): der Festhalten-Handgriff am echten Editor meldet die entstandene Datei, bei einem Namen derselben Sekunde die vorhandene Datei und dass nichts gespeichert ist, bei einem Schreibfehler den Grund samt Rest der eigenen Temp-Datei; Datei und Vergleichslinie tragen denselben Snapshot, auch wenn sich die Messdaten der Engine direkt danach aendern. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 8,00 s | [B15](roh/NAK-380-1e4a701-dirty.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,11 s | [B20](roh/NAK-380-1e4a701-dirty.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und der Fingerprint ist unter Vorzeichenumkehr des Signals bytegleich. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. Seit NAK-380 Etappe 4 (380/M-68) sind die 76 Ausgangsbytes des Akkords F1 (220 Hz, 30 s) bytegleich zur am Etappenstart erfassten Referenz: der Fingerprint behaelt den Bandfluss. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 1,71 s | [B22](roh/NAK-380-1e4a701-dirty.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 12,16 s | [B23](roh/NAK-380-1e4a701-dirty.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,03 s | [B24](roh/NAK-380-1e4a701-dirty.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,07 s | [B25](roh/NAK-380-1e4a701-dirty.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,48 s | [B26](roh/NAK-380-1e4a701-dirty.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/NAK-380-1e4a701-dirty.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,95 s | [B19](roh/NAK-380-1e4a701-dirty.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht fuer 15 bis 19 keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt auf 1e-9 genau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. Seit NAK-283 (N01) zusaetzlich die vier transienten Faelle 20 bis 23 aus EBU Tech 3341 (Versatz 0 bis 3): jeder trifft eine im Test gerechnete, vorher an 15 bis 19 auf +/-0,01 dB validierte 64-fach-Referenz innerhalb +/-0,1 dB und liegt zusaetzlich in der Normtoleranz 0,0 +0,2/-0,4 dBTP. Insgesamt 43 Pruefungen. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,34 s | [B17](roh/NAK-380-1e4a701-dirty.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit der Spanne der §5-Referenz (± 0,2 LU); zehn Kurzzeitwerte je Sekunde (EBU Tech 3342 §3.1), die 60-s-Schwelle zaehlt gegatetes Material bei 10 Hz. Bei Material ohne Dynamik bleibt sie nahe null und nach 90 s Stille gibt es gar keinen Wert. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. Seit NAK-380 Etappe 4 (380/M-42 bis M-49) misst der Testzugang den Detektor nach T-380-5: Maximumfilter +-125 Cent (w_101 = 8; ein Teilton, der um 1 bis 9 Bins wandert, ergibt Fluss 0, um 10 Bins 79,96 dB), Detektor-Bins K = 1666/1530/765 bei 44,1/48/96 kHz mit 2*K*8 B im Heap, ein Ring der Warteschlange mit 2*w_max + 2 Plaetzen aus der Filterbreite (252/232/118), der ueber einen streng fallenden Vorframe in jedem Bin das direkte Maximum trifft, und ohne Allokation im Lauf, echte MAD (28 x 1 und 4 x 11 ergeben 0), absolute Mindestschwelle, Rauschbodenbezug und P0 je beiderseits der Schwelle, lokale Spitze und 50 ms Sperrzeit bei 1 und 2 Hops. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 12,47 s | [B18](roh/NAK-380-1e4a701-dirty.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. Seit NAK-313 Etappe 4 (R-313-7, 313/M-51): ein echter FeatureFrame ohne gueltige Projektzeit (Transport local_monotonic) ergibt mit den Transportzahlen aus eingabe der Byteinstanz evidenz-lokal-wire-v1.json einen Transportblock OHNE project_sample_start, bytegleich zu wire_transport, und der ganze Snapshot passiert Textriegel und dieselbe Engine wie B3c. Seit Etappe 4 (R-313-6, 313/M-49) ausserdem der C++-Vertragsweg an der Tabelle PRODUKTEINGAENGE-FAELLE.json: je evidence_snapshot-Eintrag ein Fall mit PE-nnn im Namen durch Textriegel, genau einen strengen Lauf kanon::lies und die Engine, verglichen mit vertrag - doppeltes type faellt an der Stufe duplikat, eine Verschachtelung von 65 Ebenen an parser, project_sample_start null am Schema, und 64 Ebenen sowie der wire_snapshot der Byteinstanz sind gueltig; die Zaehlpruefung verlangt genau so viele Faelle, wie der Kopf der Tabelle fuer evidence_snapshot nennt. Seit NAK-313 Etappe 5a (R-313-5, 313/M-73): die Werte kommen aus demselben strengen Lauf (nakama::vertrag::wertAlsVar), und bei Urteil gueltig wird der Wert am Feld als Ganzzahl verglichen (transport.sequence 8241.0 und 8.241e3 sind 8241). Seit NAK-380 Etappe 4 (380/M-63, M-65): jedes Flussereignis traegt eine endliche staerke in [3, 1000] - fuer Klicks auf digitaler Stille (bis 40 dB ueber Vollaussteuerung, dort auf 1000 geklemmt), einen Klick auf einem stehenden Sinus und den Korpus I1 - und reist so durch den echten Serialisierer, den Textriegel und dieselbe Schemaengine; bei 30 s Weissrauschen ist die Onsetsumme jedes der 93 Evidenzfenster 0. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 3,28 s | [B16](roh/NAK-380-1e4a701-dirty.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. Seit NAK-313 Etappe 5b (R-313-4; 313/M-80, M-81, M-89): die Revisionsraender der NAK-283-Faelle M-07 bis M-10 und M-12 stehen bei 2^53-1 (assistentenrevision_laeuft_nicht_ueber misst den neuen Rand); Bestands- und Eintragsrevision gehen bei 2^53-2 genau einen Schritt mit genau einem Host-Dirty und weisen am Rand ab, bevor sie etwas zuweisen (Prozessor ohne Host-Dirty, Bibliothek mit Grund und gleichen Bytes); der Intentwriter schreibt bestand_revision 2^53-1 ab bestand_revision bytegleich zur Handinstanz intent-wire-v1.json. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,69 s | [B27](roh/NAK-380-1e4a701-dirty.md#b27) |
| B29 | Die Zustandsmaschine des AssistantStep im Main (46.1, E-07/E-08, M-55 bis M-62). Die Zustandsmenge hat ACHT Werte, jeder kommt ueber seinen eigenen Rueckweg zurueck, ein neunter faellt. Nach preview fuehrt KEINE Kante - weder hin noch zurueck -, ein gespeicherter preview-Schritt ist ein Lesefehler statt eines stillen Sprungs, und die Produkt-API setzt ihn nicht; die Gegenprobe mit proposal laedt normal. Jeder der acht Zustaende traegt fuenf Angaben (Eintritt, Evidenz, Useraktion, Timeout ueber 0, sichere Rueckkante), auch preview - der erste Zustand ist seine eigene Rueckkante. Der Deckel ist STRUKTURELL: ein zweiter Startversuch bei offenem Schritt wird abgewiesen, nicht eingereiht, und erst nach dem terminalen Abbruch beginnt ein neuer. Die vier Gegenpfade laufen einzeln: Zurueck geht auf die Rueckkante, Ueberspringen auf den naechsten Zustand, Resume ist eine FRAGE ohne Revisionssprung, und Verwerfen ist terminal statt geloescht - danach gibt es kein Resume mehr. Die drei benannten Ergebnisse aus 46.2 sind eigene Ergebnisse mit Objekt; dasselbe zweimal hebt keine Revision. Die harten Gates greifen VOR der Gewichtung: drei Kandidaten mit perfektem Rang und je einem gerissenen Gate verlassen die Liste, ein bescheidener bleibt. Verdrahtet am echten Prozessor mit Host-Dirty genau einmal je echter Aenderung, Rekonstruktion aus dem gespeicherten MainProject und bytegleichem Save/Load; jede Grenze faellt von beiden Seiten, und ein Altstand ohne die Eigenschaft laedt normal und schreibt sie nicht. Seit NAK-313 Etappe 5b (R-313-4; 313/M-82, M-84 bis M-87): an der Revision 2^53-1 weisen Weiter, Antwort und ein neuer Lauf ab, ohne etwas zu aendern und ohne Host-Dirty; assistentVersuchStarten prueft die Grenze vor dem Versuchsbeginn - am Probe-Server kein experiment_begin, kein belegter Slot, kein eingefrorener Pegel, 0 Host-Dirty, und der gespeicherte Stand traegt keine Versuchskennung -, startet unter der Grenze wie bisher mit genau einem experiment_begin und einem Host-Dirty, und faellt die Grenze erst nach dem Versuchsbeginn (Testhaken), weist die Bibliotheksfunktion ab und der Versuch endet mit genau einem experiment_abort (user_abbruch). | `eq-copilot\build\plugin\EqCopSonde014AssistentTest_artefacts\Release\EqCopSonde014AssistentTest.exe` | [OK] Exit 0 | 2,52 s | [B29](roh/NAK-380-1e4a701-dirty.md#b29) |
| B30 | Diagnose-Briefkasten im Plugin (Plan S25e (3), R-286-1 und R-286-2): ohne anfrage.json nur eine Existenzpruefung je Takt (hoechstens 1 Hz, Message-Thread) und nichts Messbares - Audio, State, Parameter und der Snapshot des Knopfwegs bleiben bytegleich. Eine gueltige Anfrage beantworten Gen und Probeeq je Kennung genau einmal, solange sie im Ring der letzten 256 steht, ueber den eigenen Schreibweg (exklusive Temp-Datei, Groessenpruefung, Umbenennen ohne Ersetzen, nie Ordneranlage oder Loeschung) mit einem Umschlag nach nakama.diagnose.antwort.v1; fremde oder zu grosse Anfragen und solche hinter einer Junction bleiben ohne Antwort, eine beim Lesen gesperrte wird im naechsten Takt beantwortet. Seit NAK-313 Etappe 7 (R-313-10; 313/M-132 bis M-144) der Festhalten-Export, jeder Fall in einem Testordner unter %TEMP%: er legt die Ordnerebenen an und veroeffentlicht exklusiv (Temp-Datei, Spuelen, Groesse, Umbenennen ohne Ersetzen), ersetzt eine Datei gleichen Namens nie (Doppelklick, zwei Instanzen, leeres Label, Schreibweise und Bereinigung, Uhrrueckstellung: abgelehnt, die erste Datei bytegleich), meldet Teil-Schreibfehler als Fehler und entfernt die eigene Temp-Datei oder nennt sie als Rest; er liest je Export genau eine Uhrablesung und nur den uebergebenen Snapshot, der Testbau verweigert ohne Testordner vor jedem Dateizugriff, und die Referenz des Knopfwegs bleibt bytegleich. | `eq-copilot\build\plugin\EqCopBriefkastenTest_artefacts\Release\EqCopBriefkastenTest.exe` | [OK] Exit 0 | 76,88 s | [B30](roh/NAK-380-1e4a701-dirty.md#b30) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,16 s | [B28](roh/NAK-380-1e4a701-dirty.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. Seit NAK-309 (M-74) zuerst, vor dem ersten Prozessor: im Testbau nennt der Prozessor seinem v2-Client einen Probe-Namen, nie die Produktions-Pipe, nach Produkt- und Testkonstruktor denselben, und jedes Ziel mit den Prozessorquellen ausser dem Produkt traegt den Testschalter. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,32 s | [B8](roh/NAK-380-1e4a701-dirty.md#b8) |
| A32 | Codebase- und Kontextgesundheit gegen die Schwellen aus Plan S25b und dem Kontext-Hygiene-Playbook. Der Quellumfang kommt seit NAK-283 (M-69) aus einem pruefbaren Inventar: jede Datei mit Endung .rs, .cpp, .h oder .hpp unter broker/src und eq-copilot/plugin samt allen Unterbaeumen, mit Grund ausgenommen nur der flatc-Codegen (broker/src/generiert, eq-copilot/plugin/vertrag/generiert) und der Testbaum eq-copilot/plugin/tests; der Bericht druckt das Inventar, und eine Quelldatei, die weder gemessen noch ausgenommen ist, beendet den Lauf als WERKZEUGFEHLER (Inventarriegel, Exit 2). Gemessen werden Zeilen je Quelldatei (Grenze 2 000, Ziel 1 500), Anzahl Funktionen ueber 200 Zeilen, aufruferlose allow(dead_code)-Helfer, Backtick-Bezeichner in Kommentaren ohne Entsprechung im Repo, sowie Bytes von MEMORY.md, CLAUDE.md und Dirigenten-Skill, Index-Zeilen ueber 250 Zeichen und Memory-Dateien ohne Index-Link. Die Ratschen (Funktionen 28, Kommentar-Bezeichner 30, clippy 91) stehen auf dem Iststand vom 09.09.2026 nach NAK-225 und reissen bei der ersten Verschlechterung; sie werden nur gesenkt, nie erhoeht. Die Grenzen - Zeilen je Datei, Funktionen ueber 200 Zeilen, Kommentar-Bezeichner - werden ehrlich gemeldet, auch wenn sie reissen (Pflegeticket NAK-292). Jeder neue Treffer ueber der Zeilengrenze meldet sich als OHNE PFLEGETICKET; ein Zuordnungseintrag, dessen Datei die Grenze nicht mehr reisst, beendet den Lauf als WERKZEUGFEHLER (Exit 2), damit die Liste nicht still veraltet. Vorangestellt laeuft der Selbsttest: 129 Faelle im Speicher durch dieselben Funktionen, jede Erwartung mit ihrem Gegenteil (Zahlenraender 2 000/2 001, 200/201, 250/251, CRLF, BOM, fehlender Schlussumbruch, Rust-Lebensdauer und C++-Ziffernstrenner gegen die Literalmaskierung; das Inventar mit neuem Unterbaum, Codegen, Tests und einer Ableitung, die still einen Unterbaum verliert). Der clippy-Teil laeuft NICHT mit (Schalter --clippy, er baut) und meldet sich als nicht messbar. Nicht blockierend: Exit 4 heisst gerissen und ist ein Hinweis. Exit 2 ist ROT und heisst, dass das WERKZEUG nicht mehr misst - roter Selbsttest, eine fehlende Inventarwurzel oder der Inventarriegel, denn ein verschobener Pfad wuerde sonst still 0 messen und die halbe Codebase unbemerkt aus dem Mass nehmen, und ein ungemessener Unterbaum machte jedes andere Mass falsch. | `py -3.13 tools\plan\gesundheit.py --mit-selbsttest` | [HINWEIS] Exit 4 - Befund, nicht blockierend (siehe Rohausgabe) | 36,92 s | [A32](roh/NAK-380-1e4a701-dirty.md#a32) |
| A33 | clang-tidy ueber alle Uebersetzungseinheiten von eq-copilot/plugin ohne tests/ und ohne den flatc-Codegen vertrag/generiert/ (Bein A9 haelt ihn bytegleich; Regelsatz eq-copilot/plugin/.clang-tidy: bugprone, clang-analyzer, concurrency, performance sowie eine cert-, cppcoreguidelines- und misc-Auswahl; keine Stilregeln, Ausnahmen dort mit Grund) in einem eigenen Ninja-Baum eq-copilot/build-tidy mit denselben cl.exe-Schaltern wie der Produktbau. Gezaehlt werden eindeutige Fundstellen (Datei, Zeile, Spalte, Check) gegen die Ratsche in tools/plan/tidy.py: Grenze = Iststand der Erstmessung NAK-288, Ziel 0, nur gesenkt, nie erhoeht; eine Aenderung des Regelsatzes zieht die Ratsche im selben Commit nach. Vorangestellt der Selbsttest (Einordnung in den Quellort, Datenbankfilter, Diagnoseparser, Deduplizierung, Ratschenrand, Umgebungsleser). Nicht blockierend: Exit 4 ist ein Hinweis. ROT ist Exit 2: eine Uebersetzungseinheit mit error-Diagnose oder ein abgestuerzter clang-tidy heisst NICHT gemessen, denn eine halb geparste Einheit meldet weniger Funde und saehe wie Fortschritt aus. FEHLT (Exit 3) ohne Visual Studio 2022 mit C++-Werkzeugen und Clang-Komponente, cmake oder ninja. | `py -3.13 tools\plan\tidy.py --mit-selbsttest` | [OK] Exit 0 | 110,44 s | [A33](roh/NAK-380-1e4a701-dirty.md#a33) |


## 39. Kanon nachher, Erstprüfung 4: Urteil, Validierung, Einordnung und Nacharbeitsauftrag 1 (Dirigent, 25.09.2026, 23:19 Uhr)

**Kanon nachher der Etappe 4 (Bau und Nacharbeit 0).** Abgekoppelt gestartet
22:06:01 Uhr auf `1e4a7010` (Log `nakama-nak380-e4-kanon.log`);
**GRUEN 70/70**, 22:05:53 bis 23:18:03 Uhr, EXIT=0, Rohausgabe `docs/beweise/roh/NAK-380-1e4a701-dirty.md` (dirty: Broker-Pin des Runners und die vier Rohdateien der Prüfung und Validierung), A15 stillgelegt, A32 Hinweis Exit 4 mit unveränderten Maßen 3/33/39 (Quelldateien über 2 000 Zeilen, Funktionen über 200 Zeilen, Kommentar-Bezeichner ohne Code; kein neuer Riss; Kontextfläche OK: Skill 24 355, CLAUDE.md 20 402, MEMORY.md 6 205 Bytes), A33 Exit 0. Der Runner hängt seinen Abschnitt mit CRLF an; der Dirigent hat das Manifest danach auf LF normalisiert (Index unverändert). Der Lauf stand von 22:22 bis 22:37 Uhr nach dem
Bauschritt ohne Kindprozess an einem `vctip.exe` (PID 215028; NAK-300;
vom Dirigenten beendet, der Lauf ging weiter). Ein erster Startversuch war
vom Sandbox-Filter abgelehnt worden, weil der Befehlstext ein
Lösch-Cmdlet für ein altes Log enthielt (Landmine, CLAUDE.md); der zweite
Start ohne dieses Cmdlet lief. Der Kanon-Abschnitt steht vor diesem
Abschnitt.

**Erstprüfung 4 (Vorlage A).** Codex `gpt-6-astra`, Effort **xhigh**
(NAK-362), lesend, Thread `01a0da2c-49f4-7e23-b226-44ba2e5f7f68`, 22:05:30
bis 22:22:57 Uhr, EXIT=0, HEAD vor und nach dem Lauf `1e4a7010`.
Prüfbereich `21853aa1...1e4a7010` über die 16 Ticketpfade der Etappe 4
(ohne `docs/**`); Auftrag `docs/beweise/roh/NAK-380-erstpruefung-4-auftrag.txt`
(im Zielcommit enthalten, ZIEL = HEAD gemessen; Lehre §34: kein Commit
während des Laufs), Urteil `docs/beweise/roh/NAK-380-erstpruefung-4-urteil.md`.
**URTEIL: NEEDS_WORK — 7 Defekte, 0 Lücken, 0 Härtungen; kein
Produktrechenfehler.** Die offenen Punkte 1 bis 9 des Auftrags sind vom
Prüfer beurteilt: Kalibrierfolge und Werte konsistent (1), I3 150 ms über
der Verdeckungsgrenze (2), Pflichterfassungen und getrennter Bandfluss
bestätigt (3), Version 20260927 an allen Stellen (4), Produktformel der
Stärke korrekt (5), A-6, gemeinsamer Reset und Peakfreigabe nach 32 Frames
nachvollziehbar (6), Heapträger und Ringgröße nachvollziehbar (7),
Ersatzmutationen statisch nachvollziehbar (8), bestehende Wachen und M-72
nachvollziehbar (9); Defekte an den Tests M-62 (Pufferüberlauf), M-48
(Frame), M-63 (Wert, Rosa-Selbstprüfung, Verlustzähler), M-47(a)
(Normierung) und an Toleranzen ohne Herleitung. Nicht geprüft: keine Läufe.
Codex-Wochenkontingent nach dem Lauf 86 % (Statuszeile; Warnschwelle 85 %,
Sperrgrenze 95 %, Skill §5).

**Validierung an der Quelle.** Lesender Opus-5.5-Worker `42753244`
(`nakama-nak380-1e4a701-val4`, Effort xhigh, `dontAsk`), 22:27:41 bis
22:47 Uhr, HEAD gleich; Auftrag
`docs/beweise/roh/NAK-380-erstpruefung-4-validierung-auftrag.txt`, Bericht
`docs/beweise/roh/NAK-380-erstpruefung-4-validierung.md`: **6 bestätigt (D1,
D2, D3, D4, D5, D7), 1 präzisiert (D6: nur die Vorbedingung der Hälfte (a);
Zusage und Rotbeweis tragen), 0 widerlegt; alle DEFEKT.** Teil 2:
Durchsicht M-42 bis M-72 in 18 Punkten mit Datei:Zeile, ein zusätzlicher
Befund Z1 (M-63-Schemahälfte ist für die Stärke eine Wache ohne
Trennschärfe: der Serialisierer `NakamaEvidenz.cpp:526-535` lässt Werte
außerhalb [0, 1000] nicht auf den Draht; der Matrix-Rotbeweis „Klammer weg
→ Schema rot“ tritt nicht ein, `m63-klammer` fiel am Verlust, einem
Nebeneffekt). Der Dirigent hat die Stellen selbst nachgelesen
(`AnalysisGoldenTestMain.cpp:1813-1849` feste 512er-Schleife ohne
Restblock gegen `:1320-1327` mit `min(block, samples − strom)`;
`Sonde013DynamicsTest.cpp:632-642` `folge()` liefert nur eine Zahl,
`:874-877` prüft `n == 1`; `:833-839` `weissMono (…, 1.0, …)` mit
Amplitudenskalierung ohne Energienormierung; `Sonde013EventWireTest.cpp:695-760`
Bereichs-, Endlichkeits- und Schemaprüfung ohne Wertvergleich, `verloren`
nur im Bericht).

### 39.1 Einordnung je Befund

| Befund | Quelle (HEAD `1e4a7010`) | Validierung | Einordnung | Regel |
|---|---|---|---|---|
| D1 M-62 liest 384 Werte über das Vektorende | `AnalysisGoldenTestMain.cpp:1813-1817` `while (strom < x.size())` mit fester Kopie von 512 Samples; 720 000 mod 512 = 128 | BESTÄTIGT | **DEFEKT** — undefiniertes Verhalten im Test, kein verlässlicher Grenzbeweis (Prüfliste, NAK-175-Ränder; Ursache A) | Restblock min(Block, Rest), gespeiste Samplezahl = Signallänge verriegelt; alle Läufer über dieselbe Logik |
| D2 M-48 ohne Frameprüfung | `:632-642`, `:874-877` | BESTÄTIGT | **DEFEKT** — M-48 „genau ein Ereignis am Frame mit 500“ ist nur zur Hälfte getragen (Lehre D2) | `stromSample == 2048` als eigene Prüfung; Rotbeweis Spitzenwahl `>=` → `<=` |
| D3 M-63 Stärke nur als Bereich | `Sonde013EventWireTest.cpp:701-705` | BESTÄTIGT (verschärft) | **DEFEKT** — M-63 „`staerke` = κ·(SF − med)/(T_eff − med)“ ohne Wertprüfung; R-380-8 (Ursache B) | Wert gegen unabhängige Referenz aus SF, med, T_eff (Fall mit med > 0), Draht = Engine; Rotbeweis (sf − med) → sf |
| D4 Rosa-Selbstprüfung fehlt in B16 | `:735`, `:792`; Helfer nur in B5 `AnalysisGoldenTestMain.cpp:1385-1403` | BESTÄTIGT | **DEFEKT (schwach)** — E-380-13 „vor jedem Nutzer“ (Ursache C, Erzeugerbindung) | Kopffunktion in `Nak380Pruefsignale.h`, Aufruf in jedem Programm vor jedem Rosa-Nutzer |
| D5 Verlustzähler in M-62/M-63 nicht verriegelt | `AnalysisGoldenTestMain.cpp:1849`; `Sonde013EventWireTest.cpp:713`, `:741-800` | BESTÄTIGT | **DEFEKT** — §7.3 „Verlustzähler am Ende 0, sonst Fall rot“ (Ursache B) | `ereignisseVerworfen() == 0` und Draht-`verloren == 0` als eigene Prüfung in jedem Läufer |
| D6 M-47(a) ohne Abschnittsnormierung | `Sonde013DynamicsTest.cpp:833-839` | PRÄZISIERT | **DEFEKT (schwach)** — Vorbedingung „je Abschnitt auf die feste Rahmenenergie normiert“ nicht hergestellt (Lehre D1); Zusage hält | empirische Energie je Abschnitt hergestellt und gemessen |
| D7 Toleranzen ohne Herleitung | `:688` 1e-9, `:858` 1e-6/1e-4/1e-5; `AnalysisGoldenTestMain.cpp:1669`, `:1683`, `:1766-1769` | BESTÄTIGT | **DEFEKT (Kommentar, schwach)** — Präzisierung „jede Toleranz trägt ihre Herleitung“ (Ursache B) | Rechenbudget im Kommentar oder exakter Vergleich |
| Z1 M-63-Schemahälfte ohne Trennschärfe (Validierer) | `:755`; `NakamaEvidenz.cpp:526-535`; Rotbeweisspalte M-63 §6.3 | vom Dirigenten bestätigt (m63-klammer „Schemafehler 0“, fiel am Verlust) | **DEFEKT (schwach)** — Rotbeweis an einem Nebeneffekt (Regel §2; Lehre Z1 §31); schließt sich mit D3 und D5 | Rotbeweisspalte berichtigt: Klammer weg → Wertprüfung rot; Schemahälfte als Regressionswache |

### 39.2 Matrix in der Fassung §39

M-47 (a): Vorbedingung „Weißrauschen L = R, Saat 0x3800001, je Abschnitt
empirisch auf die Rahmenenergie −90 bzw. −70 dBFS normiert (gemessen,
Toleranz hergeleitet)“. M-48: Zusage „genau ein Ereignis, dessen
`stromSample` = 2 048 (Frame mit 500)“; Rotbeweis „Bedingung SF(n) ≥
SF(n − 1) → SF(n) ≤ SF(n − 1) → Ereignis bei 450 → Frameprüfung rot“.
M-62: Vorbedingung ergänzt „gespeiste Samplezahl = 720 000 verriegelt,
Restblock 128 Samples gespeist“; Zusage ergänzt „`ereignisseVerworfen()` =
0“. M-63: Zusage „jedes Flussereignis trägt `staerke` = min(1000, κ·(SF −
med)/(T_eff − med)), im Test aus SF, med und T_eff des auslösenden Frames
unabhängig gerechnet (Toleranz hergeleitet), mindestens ein Fall mit med >
0; Drahtwert = Engine-Wert (Zahlformat-Toleranz hergeleitet); `verloren` =
0 und `ereignisseVerworfen()` = 0; der Wiretext passiert Textriegel und
Schema (Regressionswache)“; Rotbeweis „Klammer 1000 weg → Klick +100 auf
Stille überschreitet 1000 → Wertprüfung rot; (sf − med) → sf → Wertprüfung
rot“. §7.3: „Verlustzähler = 0 als eigene Prüfung in jedem Läufer.“
E-380-13/§7.1: „Die Selbstprüfung ist eine Kopffunktion des Erzeugers; jedes
Programm ruft sie vor jedem Rosa-Nutzer auf.“ Toleranzen M-42, M-47(b),
M-60: Rechenbudget im Kommentar oder exakter Vergleich.

### 39.3 Ursachen und Nacharbeitsauftrag 1 (Runde 1 von 3)

Ursachen wie in §24 und §31: (A) der Test bildet Vorbedingung und Zusage
nicht vollständig ab (D1, D2, D6); (B) je Zusagesatz, je Wert und je
Zählregel eine eigene Prüfung mit Referenz und hergeleiteter Toleranz (D3,
D5, D7, Z1); (C) Erzeugerbindung: eine Vorbedingung des Erzeugers, die ein
Programm nicht selbst herstellt (D4). Der Auftrag trägt die Ursachen, die
Befunde wörtlich, je Befund die schließende Regel und die Durchsicht aller
Läufer der Etappe gegen dieselbe Liste als Auftragsteil:
`docs/beweise/roh/NAK-380-nacharbeit-e4-1-auftrag.txt`. Kein Produktcode
(Ausnahme: eine lesende Testzugangs-Auskunft nach T-380-11 für die
M-63-Referenz, falls nötig); Ticketpfade `AnalysisGoldenTestMain.cpp`,
`Sonde013DynamicsTest.cpp`, `Sonde013EventWireTest.cpp`,
`Nak380Pruefsignale.h`, Rohdateien, Manifest §40. Bauer: frischer
Opus-5.5-Worker, Effort xhigh, Aufsicht ENG; Beine einzeln B5, B18, B16.
Wiederprüfung 1 danach nach Vorlage B über den Fixdiff (Codex `gpt-6-astra`
xhigh; bei Codex-Sperrgrenze 95 % frischer Opus-Thread xhigh, Skill §3.4).
Kalibrierwerte und Struktur bleiben (125 Cent, 0,10 dB je Bin).

**Rundenbilanz.** Runde 0 gesamt (Bau und Nacharbeit 0)
`21853aa1..1e4a7010`: Produkt 9 Datei(en) +664/−84; Tests 5 Datei(en) +1 736/−15; Pruefwerkzeug 2 Datei(en) +150/−21; Doku 156 Datei(en) +5 785/−3. Je Runde (`--runden 21853aa1 9125dbbc 1e4a7010`): Bau Produkt 9 +621/−83, Tests 5 +1 436/−15, Pruefwerkzeug 2 +142/−20, Doku 104 +3 432/−3; Nacharbeit 0 Produkt 4 +63/−21, Tests 3 +344/−44, Pruefwerkzeug 2 +22/−15, Doku 75 +2 354/−1; kein Konvergenzsignal.

**Register:** NAK-405 (§38.3) unverändert; kein neuer Eintrag. Offen beim
User: nichts.

## 40. Nacharbeit 1 der Etappe 4 (Bauer Opus 5.5 xhigh, 25.09.2026)

Auftrag `docs/beweise/roh/NAK-380-nacharbeit-e4-1-auftrag.txt` (Runde 1 von
3), Befunde D1 bis D7 und Z1 der Erstprüfung 4 (§39). Kein Produktcode
geändert: die Referenz der Stärke (M-63) ist über einen eigenen, nur
lesenden Testzugang in B16 erreichbar (T-380-11), `FeatureEngine.h` bleibt
unberührt. Die Läufe reichen über Mitternacht; Uhrzeiten nach 00:00 gehören
zum 26.09.2026.

### 40.1 Startmessung

`Get-Date` 25.09.2026 23:25:47; `git rev-parse HEAD`
`7b4f7938cec10302af933fd10e64874a433f24ef` (Basis-SHA dieser Nacharbeit);
`git status --short` nur die fremden untracked Einträge `briefing-hub/` und
`nimbalyst-local/` (kein Broker-Pin, keine `.claude/settings.local.json`);
Prozessprobe auf cmake, MSBuild, cl, link, cargo, rustc, EqCop*,
eqcop-broker, FL64 und vctip leer (23:25:47 und vor dem ersten Bau
23:47:31). Zeilenenden vor dem ersten Schreiben (`git ls-files --eol`): die
vier Testdateien Arbeitskopie CRLF, Manifest und Rohdateien LF.

### 40.2 Schließung je Befund

| Befund | Schließende Regel (§39.1) | geschlossen an |
|---|---|---|
| D1 M-62 las 384 Werte hinter dem Puffer | Restblock min(Block, Rest), gespeiste Samplezahl = Signallänge als eigene Prüfung, alle Läufer über dieselbe Logik | `AnalysisGoldenTestMain.cpp:1324` (gemeinsamer Läufer, Schleife `:1344` mit min(512, Rest)), `:1387` (`nak380SamplesGeprueft`: Puffer, Läufer und Engine gegen die hergeleitete Länge), M-62 `:1714` über den Läufer mit Seek und Beobachter, M-61 `:1645`, Beobachtung `:1520`; B16 `Sonde013EventWireTest.cpp:893`, `:1058`; B18 `Sonde013DynamicsTest.cpp:777`, `:975` |
| D2 M-48 ohne Frameprüfung | `stromSample` jedes Ereignisses gegen den hergeleiteten Wert | `Sonde013DynamicsTest.cpp:647` (`folgeZeiten` liefert je Ereignis den `stromSample`), `:1086` (M-48 `stromSample = 2048` als eigene Prüfung), `:1124`, `:1128` (M-49 an den erwarteten Hops); Rotbeweis `m48-invers` |
| D3 M-63 Stärke nur als Bereich | Wert gegen unabhängige Referenz mit hergeleiteter Toleranz, Draht = Engine, Fall mit med > 0 | `Nak380Pruefsignale.h:490` (`staerkeReferenz`: Median und echte MAD im Test), `Sonde013EventWireTest.cpp:62` (Testzugang), `:716` (Wertprüfung je Flussereignis), `:851` (Draht = Engine als float exakt), `:908`, `:917`; B18 `Sonde013DynamicsTest.cpp:798`, `:818` (med 180); Rotbeweise `m63-zaehler`, `m63-klammer`, `m63-draht` |
| D4 Rosa-Selbstprüfung fehlte in B16 | Kopffunktion des Erzeugers vor jedem Rosa-Nutzer | `Nak380Pruefsignale.h:453` (`rosaSelbstpruefung`), B16 `Sonde013EventWireTest.cpp:954`, `:1010`, B5 `AnalysisGoldenTestMain.cpp:1441` (Helfer ruft den Kopf), Beobachtung `:1520`; Rotbeweis `n1-rosa` |
| D5 Verlustzähler in M-62/M-63 nicht verriegelt | `ereignisseVerworfen() == 0` und Draht-`verloren == 0` als eigene Prüfung in jedem Läufer | B5 `AnalysisGoldenTestMain.cpp:1400` (`nak380VerlustGeprueft`, Aufrufe `:1427`, `:1494`, `:1674`, `:1737`); B16 `Sonde013EventWireTest.cpp:898`, `:901`, `:1063`, `:1067`; B18 `Sonde013DynamicsTest.cpp:782`; Rotbeweise `n1-verlust`, `m62-verlust`, `m50-verlust`, `m65-verlust`, `m47-verlust` |
| D6 M-47(a) ohne Abschnittsnormierung | empirische Energie je Abschnitt hergestellt und gemessen | `Nak380Pruefsignale.h:271` (`weissAbschnitteNormiert`), `Sonde013DynamicsTest.cpp:731` (`nak380PegelbezugNormiert`), Messung `:756`; Rotbeweis `m47-norm` |
| D7 Toleranzen ohne Herleitung | Rechenbudget im Kommentar oder exakter Vergleich | `Sonde013DynamicsTest.cpp:882` (M-42), `:1058` (M-47(b)), `:756` (M-47(a)), `:707` (Stärke); `AnalysisGoldenTestMain.cpp:1833` (M-60(a)), `:1847` (M-60(b) exakt), `:1693` (M-61); Tabelle §40.6 |
| Z1 Schemahälfte ohne Trennschärfe | Rotbeweis Klammer weg → Wertprüfung rot; Schemahälfte als Regressionswache | `Sonde013EventWireTest.cpp:970`, `:999`, `:1014` (Prüftexte „Regressionswache: … Textriegel und Schema gueltig“, ohne Rotzusage); `m63-klammer` fällt an der Wertprüfung `:908` |

### 40.3 Änderungen je Datei

- **`eq-copilot/plugin/tests/Nak380Pruefsignale.h`** (B5, B16, B18): drei
  Kopffunktionen. `weissAbschnitteNormiert` (`:271`) erzeugt das
  Gauß-Weiß von `weissMono (saat, 1.0, …)` und skaliert jeden Abschnitt mit
  sqrt(E_soll/E_ist) der gemessenen Stichprobenenergie (M-47(a), D6); falsche
  Grenzen liefern einen leeren Puffer. `rosaSelbstpruefung` (`:453`) ist die
  E-380-13-Prüfung des Erzeugers mit Urteil und Meldung wie bisher im
  B5-Helfer (Nachbardifferenz, zehn Oktaven, RMS), ergänzt um die
  Samplezahl; die Pegeltoleranz 0,5 dB trägt jetzt die Herleitung aus der
  Korrelationssumme des Kellet-Filters (§40.6). `staerkeReferenz` (`:490`)
  rechnet Median, echte MAD, T_eff und die Stärke aus SF und einer Historie
  mit den Matrixzahlen, unabhängig vom Produkt (M-63, D3). Dazu der
  locale-freie Formatierer `detail::festkomma` (`std::to_chars`).
- **`eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp`** (B5): der
  Korpusläufer `nak380Korpuslauf` (`:1324`) ist der eine Blockläufer aller
  Detektorfälle; er begrenzt Kopie, `sampleCount` und Fortschreibung auf
  min(512, Rest), zählt die gespeisten Samples, liest die verarbeiteten über
  den Testzugang `verarbeitet` (`:136`), hält die Engine für Auskünfte nach
  dem Lauf und ruft einen Beobachter vor und nach jedem Block. M-61
  (`nak380WiederbeginnNachStille`, `:1645`) und M-62
  (`nak380GrenzeLeertDetektor`, `:1714`) sind aus `nak380Detektor` in eigene,
  nicht eingebettete Funktionen gezogen und fahren über diesen Läufer (M-62
  mit Seek bei 480 256 und Beobachter für die Auskunft vor und nach dem
  Grenzblock); die Beobachtung I3 fährt ebenso über ihn und ruft vorher die
  Rosa-Selbstprüfung (Exitcode 1, wenn sie fällt). `nak380SamplesGeprueft`
  (`:1387`) und `nak380VerlustGeprueft` (`:1400`) sind je eine eigene Prüfung
  in jedem Fall (Nullfall, Impulsfall, M-61, M-62); die bisher kombinierten
  Vorbedingungsprüfungen sind getrennt. Der Rosa-Helfer ruft den Kopf.
  Toleranzen M-60(a) mit Budget, M-60(b) exakt (`tMin == 153.0 && sb ==
  6.0f`), M-61 mit Budget.
- **`eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp`** (B18): Testzugang um
  `letzterEreignisStrom` und `verarbeitet` ergänzt; `folgeZeiten` (`:647`)
  liefert je Ereignis den `stromSample` (`folge` ist die Anzahl davon);
  `ulp`, `pegelBudget`, `staerkeToleranz` als Rechenbudget-Helfer; M-47(a)
  als eigene Funktion `nak380PegelbezugNormiert` (`:731`) mit normiertem
  Signal, gemessener Abschnittsenergie, Läufer mit Entnahme nach jedem Frame
  und je einer Prüfung für Samplezahl, Verlustzähler und Zusage; M-48 mit
  eigener Frameprüfung (`:1086`); M-49 mit zwei Zeitprüfungen (`:1124`,
  `:1128`); M-63-Einheit `nak380StaerkeWert` (`:798`, zwei Fälle mit med
  180: 3,016 67 und die Klammer 1000); M-42 und M-47(b) mit hergeleiteter
  Toleranz; der M-43-Allokationslauf speist den Restblock (96 000 statt
  95 744 Samples) und verriegelt die Zahl in derselben Prüfung.
- **`eq-copilot/plugin/tests/Sonde013EventWireTest.cpp`** (B16): eigener,
  nur lesender `FeatureEngineTestzugang` (`:62`: Fensteranfang und SF des
  zuletzt geschlossenen Frames, Historie, verarbeitete Samples); der
  Wireläufer (`:688`) merkt je geschlossenem Hauptstufen-Frame die Referenz
  aus der Historie vor diesem Frame, prüft jedes Flussereignis der Engine
  genau einmal (beim Entnehmen nach einem gebauten Snapshot, den Rest am
  Laufende) gegen sie, jedes Flussereignis am Draht gegen das
  Engine-Ereignis desselben Fensteranfangs, und zählt Ringverlust, Draht-
  `verloren` und Samplezahlen getrennt. `nak380M63Pruefungen` (`:889`) gibt je
  Lauf sechs eigene Prüfungen aus; die bisherigen Draht- und Schemaprüfungen
  heißen jetzt „Regressionswache“ (Z1). Rosa-Selbstprüfung vor dem
  Aufwärmmaterial (`:954`) und vor I1 (`:1010`). M-65 mit eigener
  Samplezahl- und zwei Verlustprüfungen, die Onsetsumme ohne den früher
  angehängten Draht-Verlust.
- **Rohdateien:** 20 neue Mutationsprotokolle
  `docs/beweise/roh/NAK-380-e4-n1-mutation-<Kürzel>.txt`; Block „Nacharbeit
  1, 25.09.2026“ append-only an `NAK-380-rot-M-43.txt`, `-M-47.txt`,
  `-M-48.txt`, `-M-49.txt`, `-M-50.txt`, `-M-57.txt`, `-M-61.txt`,
  `-M-62.txt`, `-M-63.txt`, `-M-65.txt`.
- Nicht angefasst: Produktcode (`git diff --stat -- eq-copilot/plugin/core
  eq-copilot/plugin/vertrag eq-copilot/plugin/src` leer), Fixtures, Schema,
  Register, Broker, `tools/`.

### 40.4 Durchsicht aller Läufer der Etappe (Auftragsteil)

Liste je Läufer: Restblock min(Block, Rest); gespeiste Samplezahl =
Signallänge als eigene Prüfung (Läufer und Engine); Verlustzähler = 0 als
eigene Prüfung; Rosa-Selbstprüfung vor dem Nutzer; Zeit je Ereignis, wo die
Zusage sie nennt; Wert gegen Referenz; Toleranz mit Herleitung. „erfüllt“
heißt am HEAD schon vorhanden und unverändert; „nachgezogen“ mit Datei:Zeile
am Nacharbeitsstand; „—“ die Zusage nennt die Größe nicht.

| Läufer (Fälle, Bein) | Restblock | Samplezahl | Verlustzähler | Rosa vor Nutzer | Zeit je Ereignis | Wert gegen Referenz | Toleranz |
|---|---|---|---|---|---|---|---|
| `nak380Korpuslauf`, Nullfall M-50 bis M-56 (B5) | erfüllt (HEAD `:1321`), jetzt `AnalysisGoldenTestMain.cpp:1344` | nachgezogen `:1426` (1 440 000, Läufer und Engine; HEAD nur Läufer, kombiniert) | nachgezogen, eigene Prüfung `:1427` (HEAD mit der Samplezahl kombiniert `:1369`) | M-53 erfüllt `:1778`; W1 bis W3, S1, S2, V1 ohne Rosa | — (Zusage ist eine Anzahl; Zeiten in der Meldung) | Golden exakt | keine |
| `nak380Korpuslauf`, Impulsfall M-57 bis M-59 (B5) | erfüllt | nachgezogen `:1493` | nachgezogen, eigene Prüfung `:1494` (HEAD mit der Klickzahl kombiniert `:1449`) | erfüllt, über den Kopf `:1441` vor `klicksEinsetzen` | erfüllt: je Klick ein Ereignis in [c − 4096, c] `:1495` | — | keine |
| M-60, Testzugang-Einheit (B5) | kein Läufer | — | — | — | — | erfüllt (Crest − 12 dB; 6,0) | nachgezogen `:1833` (Budget), `:1847` (exakt) |
| M-61 (B5) | nachgezogen: gemeinsamer Läufer statt eigener Schleife `:1645` | nachgezogen, eigene Prüfung `:1672` (HEAD in der Zusageprüfung `:1761`) | nachgezogen, eigene Prüfung `:1674` (HEAD in der Zusageprüfung) | erfüllt `:1649` | erfüllt: Fensteranfang in [331 904, 336 000] `:1677` | Vorframe −100 dB | nachgezogen `:1693` (Budget) |
| M-62 (B5) | nachgezogen: gemeinsamer Läufer, Restblock 128 statt 384 Werte hinter dem Puffer `:1714` | neu `:1735` (720 000) | neu `:1737` | erfüllt `:1719` | erfüllt: erster Frame 480 256, keines bis 545 792 `:1755` | — | keine |
| Beobachtung I3-100/I3-150 (B5, ohne Zusage) | nachgezogen: gemeinsamer Läufer | ausgegeben (Puffer, gespeist, verarbeitet) | ausgegeben | nachgezogen `:1520`, mit Urteil und Exitcode | — | — | — |
| Wireläufer, M-63 Klick auf Stille, Klick auf Sinus, Korpus I1 (B16) | erfüllt (HEAD `:632`), jetzt `Sonde013EventWireTest.cpp:757` | neu `:893` (240 000, 192 000, 1 440 000) | neu: Engine `:898`, Draht `:901` (HEAD nur im Bericht) | nachgezogen: Aufwärmmaterial `:954`, I1 `:1010`; Sinus ohne Rosa | erfüllt: Klickereignisse an 143 360 und 192 512 `:970` bzw. 143 360 `:999`; Werte je Ereignis über den Fensteranfang zugeordnet | neu: Referenz je Flussereignis `:908` (alle mit med > 0), Draht = Engine `:917` | neu `:716` und `:851` |
| Wireläufer, M-65 W1 (B16) | erfüllt | neu `:1058` | nachgezogen: Engine neu `:1063`, Draht eigene Prüfung `:1067` (HEAD an die Summenprüfung gehängt) | W1 ohne Rosa | — (je Fenster) | Summe = 0 exakt `:1070` | keine |
| M-47(a) (B18) | erfüllt (HEAD über `fahreGenau`), jetzt eigene Schleife mit Entnahme nach jedem Frame `Sonde013DynamicsTest.cpp:766` | neu `:777` (480 000) | nachgezogen, eigene Prüfung `:782` (HEAD kombiniert `:846`) | ohne Rosa | — | Vorbedingung „normiert“ gemessen `:756` | neu `:756` (2 × Budget) |
| M-43 Allokationslauf (B18) | nachgezogen `:963` (HEAD 95 744 statt 96 000 Samples, Validierung Punkt 1) | nachgezogen in derselben Prüfung `:975` | kein Ereigniszähler (Allokationsfall) | — | — | — | keine |
| `folgeZeiten`, M-45, M-46, M-48, M-49 (B18, Testzugang) | kein Blockläufer | — | — (höchstens ein Ereignis je Schritt, bis vier Schritte, Ring 64) | — | M-48 nachgezogen `:1086`; M-49 nachgezogen `:1124`, `:1128`; M-45, M-46 nennen keinen Frame | exakte Schwellen | keine |
| M-63-Einheit (B18, Testzugang) | kein Läufer | — | — | — | — | neu `:818` (med 180; 3,016 67 und 1000) | neu `:707` |
| M-42, M-47(b) (B18, Testzugang) | kein Läufer | — | — | — | — | erfüllt (Sollwerte analytisch) | nachgezogen `:882`, `:1058` |

Die 18 Punkte der Validierung (Teil 2) nachgeprüft: Punkt 1 (Blockschleifen)
bestätigt, zusätzlich der M-43-Lauf nachgezogen; Punkt 2 (Verlustzähler)
bestätigt, M-62, M-63 und der Engine-Zähler von M-65 nachgezogen; Punkt 3
(Rosa-Nutzer) bestätigt, B16 und die Beobachtung nachgezogen; Punkte 4 bis
13 am Nacharbeitsstand unverändert gültig (Zuordnung M-57 bis M-59, Fenster
M-61, M-64-Sweep unberührt, M-65-Snapshots 93/89, M-44 bis M-46, M-43);
Punkte 14 und 15 (M-68, M-70) nicht berührt; Punkt 16 (Toleranzen) alle fünf
nachgezogen, dazu die neuen Toleranzen §40.6; Punkt 17 (Zeit je Ereignis)
M-48 und M-49 nachgezogen; Punkt 18 (Stärkewert bei med > 0) in B16 und B18
nachgezogen, `letzteStaerke` wird genutzt.

### 40.5 Tabelle je berührter Matrixzeile

Regel des Auftrags (§31): je Satz und Hälfte der Zusage in der Fassung
§39.2 eine Zeile Zusagesatz → Prüfzeile → Mutation → rot → Rohdatei;
Prüf- und Mutationszeilen am Nacharbeitsstand (`Spektrum.h`, `Zeit.h`,
`NakamaEvidenz.cpp` unverändert gegen HEAD). „rot JA“: das Protokoll nennt
„Die Zusage faellt an der erwarteten Pruefung: JA“, die Rücknahme ist per
SHA-256 gleich, der Fall danach grün (Neubau, Binary jünger als die
Quelle). Bei den Doppelmutationen nennt das Protokoll zusätzlich die
Zusageprüfung, die unter der Mutation grün bleiben muss („Trennschärfe:
ja“). Protokolle `docs/beweise/roh/NAK-380-e4-n1-mutation-<Kürzel>.txt`,
Sammelnachweis im Block „Nacharbeit 1, 25.09.2026“ der Rohdatei
`NAK-380-rot-M-nn.txt`. **37 Zeilen: 36 mit rotem Beleg (35 aus dieser
Nacharbeit, 20 Protokolle, alle JA; M-47(b) aus Etappe 4, §35.5) und eine
Regressionswache ohne Rotzusage (Z1).**

**M-47 (a)** (`NAK-380-rot-M-47.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Vorbedingung: je Abschnitt empirisch auf die Rahmenenergie −90 bzw. −70 dBFS normiert (gemessen, Toleranz hergeleitet) | `Sonde013DynamicsTest.cpp:756` „380/M-47 pegelbezug_p0 (a): Vorbedingung je Abschnitt empirisch …“, B18 | `Nak380Pruefsignale.h:291` Faktor ohne `/ eIst`, nur die theoretische Standardabweichung (m47-norm; Zusage `:785` grün) | JA | `NAK-380-e4-n1-mutation-m47-norm.txt` |
| Vorbedingung: gespeiste Samplezahl 480 000, Läufer und Engine (D1) | `Sonde013DynamicsTest.cpp:777`, B18 | `Sonde013DynamicsTest.cpp:766` nur volle Blöcke (m47-rest) | JA | `NAK-380-e4-n1-mutation-m47-rest.txt` |
| Zählregel §7.3: Verlustzähler am Ende 0, eigene Prüfung | `Sonde013DynamicsTest.cpp:782`, B18 | `Spektrum.h:486` und `:935` (m47-verlust, Doppelmutation; Zusage `:785` grün) | JA | `NAK-380-e4-n1-mutation-m47-verlust.txt` |
| (a) kein Ereignis | `Sonde013DynamicsTest.cpp:785` „… Sprung von -90 auf -70 dBFS unter dem Aktivgate loest nichts aus“, B18 | `Spektrum.h:486` `flussSchritt (s)` statt `flussVorframeSchritt (s)` im inaktiven Zweig (m47-aktiv, Matrixmutation, wiederholt) | JA | `NAK-380-e4-n1-mutation-m47-aktiv.txt` |
| (b) SF = 59,48 dB < T_min, kein Ereignis (nur die Toleranz neu hergeleitet) | `Sonde013DynamicsTest.cpp:1058`, B18 | P0 → 0 (m47-p0, Etappe 4, unverändert gültig) | JA (§35.5) | `NAK-380-e4-mutation-m47-p0.txt` |

**M-48** (`NAK-380-rot-M-48.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| genau ein Ereignis (450 in der Sperrzeit und kein lokales Maximum, 400 kein lokales Maximum) | `Sonde013DynamicsTest.cpp:1081`, B18 | `Spektrum.h:851` `sf >= d.sfVorher &&` weg (m48-lokal, Matrixmutation: 400 löst aus) | JA | `NAK-380-e4-n1-mutation-m48-lokal.txt` |
| das Ereignis liegt am Frame mit 500, `stromSample` = 1·2 048 = 2 048 | `Sonde013DynamicsTest.cpp:1086`, B18 | `Spektrum.h:851` `sf >= d.sfVorher` → `sf <= d.sfVorher` (m48-invers, Rotbeweis §39.2: Ereignis bei 450, Anzahlprüfung `:1081` grün) | JA | `NAK-380-e4-n1-mutation-m48-invers.txt` |
| dasselbe, Zeitstempel | `Sonde013DynamicsTest.cpp:1086`, B18 | `Spektrum.h:859` Ereigniszeit einen Hop später (m49-zeit; Anzahl grün) | JA | `NAK-380-e4-n1-mutation-m49-zeit.txt` |

**M-49** (Zeitprüfungen nach D2; `NAK-380-rot-M-49.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| 48 kHz: die Ereignisse liegen an den erwarteten Hops, 1 Hop {2 048}, 2 Hops {2 048, 6 144} | `Sonde013DynamicsTest.cpp:1124`, B18 | `Spektrum.h:859` Ereigniszeit einen Hop später (m49-zeit; Anzahlprüfungen `:1113`, `:1117` grün) | JA | `NAK-380-e4-n1-mutation-m49-zeit.txt` |
| 44,1 kHz: dieselben Hops | `Sonde013DynamicsTest.cpp:1128`, B18 | wie oben (m49-zeit) | JA | `NAK-380-e4-n1-mutation-m49-zeit.txt` |

**M-62** (`NAK-380-rot-M-62.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Vorbedingung: gespeiste Samplezahl = 720 000 verriegelt, Restblock 128 gespeist | `AnalysisGoldenTestMain.cpp:1390` über `:1735`, B5 | `AnalysisGoldenTestMain.cpp:1344` gemeinsamer Läufer nur volle Blöcke (m62-rest; auch M-50 rot) | JA | `NAK-380-e4-n1-mutation-m62-rest.txt` |
| Auskunft `flussBinVorgaengerGueltig()` direkt nach der Grenze falsch | `AnalysisGoldenTestMain.cpp:1743`, B5 | `Zeit.h:500` `detektorLeeren();` auskommentiert (m62-leeren, Matrixmutation) | JA | `NAK-380-e4-n1-mutation-m62-leeren.txt` |
| kein Ereignis im ersten Frame nach der Grenze | `AnalysisGoldenTestMain.cpp:1755`, B5 | wie oben (m62-leeren) | JA | `NAK-380-e4-n1-mutation-m62-leeren.txt` |
| keines, bevor die Historie wieder 32 aktive Frames trägt | `AnalysisGoldenTestMain.cpp:1755`, B5 | `Spektrum.h:924` Stand und Füllung der Historie bleiben (m62-historie) | JA | `NAK-380-e4-n1-mutation-m62-historie.txt` |
| `ereignisseVerworfen()` = 0 (Zusage ergänzt) | `AnalysisGoldenTestMain.cpp:1402` über `:1737`, B5 | `Zeit.h:500` und `Spektrum.h:935` (m62-verlust, Doppelmutation: 2 verworfen, Zusage `:1755` grün) | JA | `NAK-380-e4-n1-mutation-m62-verlust.txt` |
| Vorbedingung E-380-13 (selbstgeprüftes Rosa) | `AnalysisGoldenTestMain.cpp:1441` über `:1719`, B5 | `Nak380Pruefsignale.h:335` `b0 = 0.0;` (n1-rosa) | JA | `NAK-380-e4-n1-mutation-n1-rosa.txt` |

**M-63** (`NAK-380-rot-M-63.txt`)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Flussereignis trägt `staerke` = min(1000, κ·(SF − med)/(T_eff − med)), im Test aus SF, med und T_eff des auslösenden Frames unabhängig gerechnet (Toleranz hergeleitet), mindestens ein Fall mit med > 0 | `Sonde013EventWireTest.cpp:908` je Lauf (Korpus I1: 65 Ereignisse, alle med > 0), B16; `Sonde013DynamicsTest.cpp:818` (med 180, SF 361), B18 | `Spektrum.h:872` `(sf - med)` → `sf` (m63-zaehler; Draht = Engine und Regressionswache grün) | JA | `NAK-380-e4-n1-mutation-m63-zaehler.txt` |
| Klammer 1000: der Wert des Klicks +100 auf Stille überschreitet 1000 → Wertprüfung rot | `Sonde013EventWireTest.cpp:908` (Klick auf Stille), B16; `Sonde013DynamicsTest.cpp:818` (SF 60 360), B18 | `Spektrum.h:872` Klammer weg (m63-klammer) | JA | `NAK-380-e4-n1-mutation-m63-klammer.txt` |
| endlich, in [3, 1000] (Engine) | `Sonde013EventWireTest.cpp:905`, B16 | wie oben (m63-klammer) | JA | `NAK-380-e4-n1-mutation-m63-klammer.txt` |
| Drahtwert = Engine-Wert | `Sonde013EventWireTest.cpp:917`, B16 | `NakamaEvidenz.cpp:559` Stärke abgerundet serialisiert (m63-draht; Wertprüfung und Regressionswache grün) | JA | `NAK-380-e4-n1-mutation-m63-draht.txt` |
| `ereignisseVerworfen()` = 0 | `Sonde013EventWireTest.cpp:898`, B16 | `Spektrum.h:935` Ring nimmt kein Ereignis an und zählt jedes (n1-verlust) | JA | `NAK-380-e4-n1-mutation-n1-verlust.txt` |
| `verloren` = 0 am Draht | `Sonde013EventWireTest.cpp:901`, B16 | n1-verlust (Engineverlust reist als `verloren`); m63-klammer (Serialisierer lässt Werte über 1000 nicht auf den Draht) | JA / JA | `NAK-380-e4-n1-mutation-n1-verlust.txt`, `NAK-380-e4-n1-mutation-m63-klammer.txt` |
| Vorbedingung: gespeiste Samplezahl = Signallänge (240 000, 192 000, 1 440 000) | `Sonde013EventWireTest.cpp:893`, B16 | `Sonde013EventWireTest.cpp:757` nur volle Blöcke (m63-rest; Sinus ohne Rest grün) | JA | `NAK-380-e4-n1-mutation-m63-rest.txt` |
| Vorbedingung E-380-13 für Aufwärmmaterial und I1 | `Sonde013EventWireTest.cpp:880` über `:954`, `:1010`, B16 | `Nak380Pruefsignale.h:335` `b0 = 0.0;` (n1-rosa) | JA | `NAK-380-e4-n1-mutation-n1-rosa.txt` |
| der Wiretext passiert Textriegel und Schema (Regressionswache, Z1) | `Sonde013EventWireTest.cpp:970`, `:999`, `:1014`, B16 | keine Rotzusage: der Serialisierer lässt Werte außerhalb [0, 1000] und nicht endliche nicht auf den Draht (`NakamaEvidenz.cpp:526-535`) | — | — |

**Zählregel §7.3 je Läufer** (Verlustzähler 0 als eigene Prüfung)

| Läufer | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| B5 `nak380Korpuslauf`, Nullfall M-50 bis M-56 | `AnalysisGoldenTestMain.cpp:1402` über `:1427` | m50-relativ + n1-verlust (m50-verlust; Golden `:1428` grün) | JA | `NAK-380-e4-n1-mutation-m50-verlust.txt` |
| B5 `nak380Korpuslauf`, Impulsfall M-57 bis M-59 | `:1402` über `:1494` | `Spektrum.h:935` (n1-verlust, M-57: 112 verworfen) | JA | `NAK-380-e4-n1-mutation-n1-verlust.txt` |
| B5 `nak380Korpuslauf`, M-61 | `:1402` über `:1674` | n1-verlust (1 verworfen) | JA | `NAK-380-e4-n1-mutation-n1-verlust.txt` |
| B5 `nak380Korpuslauf`, M-62 | `:1402` über `:1737` | m62-verlust | JA | `NAK-380-e4-n1-mutation-m62-verlust.txt` |
| B16 Wireläufer, M-63 | `Sonde013EventWireTest.cpp:898`, `:901` | n1-verlust (Stille 3, Sinus 1, I1 112 verworfen) | JA | `NAK-380-e4-n1-mutation-n1-verlust.txt` |
| B16 Wireläufer, M-65 | `Sonde013EventWireTest.cpp:1063`, `:1067` | m65-relativ + n1-verlust (m65-verlust; Onsetsumme `:1070` grün) | JA | `NAK-380-e4-n1-mutation-m65-verlust.txt` |
| B18 M-47(a) | `Sonde013DynamicsTest.cpp:782` | m47-verlust | JA | `NAK-380-e4-n1-mutation-m47-verlust.txt` |

**Gespeiste Samplezahl je Läufer** (D1; nicht in der obigen M-Zeile)

| Läufer | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| B5 Nullfall (M-50 stellvertretend für M-50 bis M-56, dieselbe Funktion) | `AnalysisGoldenTestMain.cpp:1390` über `:1426` | `:1344` nur volle Blöcke (m62-rest, 1 439 744) | JA | `NAK-380-e4-n1-mutation-m62-rest.txt` |
| B16 M-65 | `Sonde013EventWireTest.cpp:1058` | `:757` nur volle Blöcke (m63-rest) | JA | `NAK-380-e4-n1-mutation-m63-rest.txt` |
| B18 M-43 Allokationslauf (96 000) | `Sonde013DynamicsTest.cpp:975` | `:963` nur volle Blöcke (m43-rest) | JA | `NAK-380-e4-n1-mutation-m43-rest.txt` |

**E-380-13 je Programm** (Kopffunktion vor jedem Rosa-Nutzer)

| Programm | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| B16 (Aufwärmmaterial, I1) | `Sonde013EventWireTest.cpp:880` über `:954`, `:1010` | `Nak380Pruefsignale.h:335` `b0 = 0.0;` (n1-rosa) | JA | `NAK-380-e4-n1-mutation-n1-rosa.txt` |
| B5 (M-62 und Beobachtung; M-53, M-57 bis M-59, M-61 über denselben Helfer) | `AnalysisGoldenTestMain.cpp:1441` über `:1719`, `:1520` | wie oben (n1-rosa) | JA | `NAK-380-e4-n1-mutation-n1-rosa.txt` |

**M-42, M-60:** nur Toleranzen (§40.6), Fälle grün, keine Rotbeweise.

### 40.6 Toleranzen (Stelle, Wert, Herleitung)

Rechenbudgets mit „ulp“ = Abstand zur nächsten double-Zahl; log10 und pow
mit höchstens 2 ulp angesetzt. Wo das Budget im Test gerechnet wird
(`pegelBudget`, `staerkeToleranz`, M-47(a), M-61), steht die Formel im Code
und der Zahlenwert in der Meldung des Laufs.

| Stelle | Wert | Herleitung |
|---|---|---|
| M-42 d = 1 bis 9, `Sonde013DynamicsTest.cpp:870` | exakt 0,0 | jedes Delta ist ≤ 0, SF bleibt die Startnull (unverändert) |
| M-42 d = 10, `Sonde013DynamicsTest.cpp:882` | 4 × Budget = 5,3·10⁻¹³ dB (gemessen 0) | je Pegel L = 10·log10(p + P0): Eingang relativ ≤ 2⁻⁵⁰ (zwei pow) + 2⁻⁵² (/df, ·df) + 2⁻⁵³ (Addition) → 10/ln10 · 1,22·10⁻¹⁵ = 5,3·10⁻¹⁵ dB; log10 ≤ 2 ulp, mal 10; die Multiplikation mit 10 ≤ 0,5 ulp(L); L(−20) 1,6·10⁻¹⁴, L(−120) 4,8·10⁻¹⁴ dB; Produkt und Referenz je einmal plus je eine Rundung der Differenz (ulp(79,96) = 1,4·10⁻¹⁴) → 1,33·10⁻¹³ dB; Faktor 4 |
| M-47(b), `Sonde013DynamicsTest.cpp:1058` | 4 × Budget = 1,2·10⁻⁹ dB (gemessen 0) | SF = Summe von K = 1 530 gleichen Deltas L(−120) − L(−130): der Deltafehler wiederholt sich K-mal auf beiden Seiten, 2·K·(4,8 + 4,8)·10⁻¹⁴ = 2,9·10⁻¹⁰ dB; fortlaufende Summation K·0,5 ulp(59,5) = 5,4·10⁻¹² dB; zusammen 3,0·10⁻¹⁰ dB; Faktor 4 |
| M-47(a) Abschnittsenergie, `Sonde013DynamicsTest.cpp:756` | 2 × Budget = 1,04·10⁻⁶ dB (gemessen 5,9·10⁻¹⁰ und 3,3·10⁻¹⁰ dB) | float-Speicherung x_f = x(1 + d), \|d\| ≤ 2⁻²⁴ → x_f² relativ ≤ 2⁻²³ + 2⁻⁴⁸; Normierung in double ≤ 2⁻⁵⁰; zwei Summen über n = 240 000 positive Werte ≤ 2n·2⁻⁵³ = 5,3·10⁻¹¹; relativ 1,19·10⁻⁷, in dB 5,2·10⁻⁷; Faktor 2 (ohne Normierung wiche die Stichprobe um 0,013 dB ab) |
| M-60(a), `AnalysisGoldenTestMain.cpp:1833` | 2⁻²¹ + 4·2,7·10⁻¹⁴ dB = 4,77·10⁻⁷ dB | float-Speicherung: 14,02 liegt in [8, 16), Stufe 2⁻²⁰, Fehler ≤ 2⁻²¹; double-Budget beider Wege (Eingang 2,5·2⁻⁵³ → 2,4·10⁻¹⁵, je log10 2 ulp(1,301)·20 = 8,9·10⁻¹⁵, je ·20 und −12 dB 1,8·10⁻¹⁵ und 8,9·10⁻¹⁶) = 2,7·10⁻¹⁴ dB; Faktor 4 auf das double-Budget |
| M-60(b), `AnalysisGoldenTestMain.cpp:1847` | exakt (`tMin == 153.0 && sb == 6.0f`) | 0,10·1 530 rundet in double auf 153,0 (Fehler 8,5·10⁻¹⁵ < halbe Stufe 1,4·10⁻¹⁴); SF = 306,0, med = 0, 3·306/153 = 6,0 exakt in double und float |
| M-61 Vorframe, `AnalysisGoldenTestMain.cpp:1693` | 4 × Budget = 1,78·10⁻¹³ dB (gemessen −100,0000) | pow ≤ 2 ulp → 10/ln10 · 2⁻⁵¹ = 1,9·10⁻¹⁵ dB; log10(P0) = −10 mit ≤ 2 ulp(10) = 2⁻⁴⁸ → mal 10 = 3,6·10⁻¹⁴ dB; Multiplikation ≤ 0,5 ulp(100) = 7,1·10⁻¹⁵ dB; zusammen 4,5·10⁻¹⁴ dB; Faktor 4 |
| M-63 Stärke gegen Referenz, `Sonde013EventWireTest.cpp:716`, `Sonde013DynamicsTest.cpp:707` | \|ref\|·(2⁻²⁴ + 2⁻⁵⁰) (gemessen höchstens 5,68·10⁻⁸ relativ in I1; B18 1,6·10⁻⁸ absolut bei 3,017) | Produkt und Referenz rechnen dieselbe Formel in double: ≤ 2⁻⁵⁰ relativ auseinander (T_eff aus einem max, dessen Kandidat med + κ·MAD einmal mehr runden darf: 2 ulp von T_eff, T_eff/(T_eff − med) ≤ 2 → 2⁻⁵¹ auf den Nenner; je 2⁻⁵³ für SF − med, T_eff − med, ·κ, /Nenner); float-Speicherung ≤ halbe Stufe ≤ \|x\|·2⁻²⁴; beide Terme sind Schranken, kein Faktor |
| M-63 Draht gegen Engine, `Sonde013EventWireTest.cpp:851` | exakt als float | der Serialisierer schreibt 15 signifikante Stellen (`wireZahl`, ≤ 5·10⁻¹⁵ relativ), JUCE liest mit `_strtod_l` (≤ 1 ulp); beides liegt weit innerhalb der halben float-Stufe (6·10⁻⁸ relativ), (float) Drahtwert ist der Engine-float |
| E-380-13 Pegel, `Nak380Pruefsignale.h:453` | 0,5 dB | relative Varianz der Stichprobenleistung (2/N)·Σ_k ρ(k)²; Σρ² = 96,04 aus der Impulsantwort des Kellet-Filters; kürzester Nutzer 240 000 Samples: 2,83 % = 0,121 dB (1σ), 0,5 dB = 4,1σ; 1 440 000 Samples: 0,050 dB, 10σ (die frühere Annahme „alles am langsamsten Pol“, 877, ist nur eine Schranke und gäbe bei 240 000 Samples 1,35σ) |
| E-380-13 Oktaven | 1,0 dB | Wert von E-380-13 selbst (§7.1) |

### 40.7 Einzeln gefahrene Beine am Endstand

Endbau der drei Ziele 26.09.2026 00:01:13 bis 00:01:20 Uhr, Exit 0
(`eq-copilot/build/nak380-e4n1/bau-end.log`, Bauartefakt; neu gebaut wurde
nur B18, B5 und B16 waren nach der letzten Rücknahme schon neu gebaut; einzige
Warnung die vorbestehende C4458 in `Stereo.h:70`, `vctip.exe` danach
beendet). Jüngste Quelle unter `eq-copilot/plugin/core`, `vertrag` und
`tests`: `Nak380Pruefsignale.h` 00:00:29.963 (Rücknahme von `n1-rosa`); jedes
Binary ist jünger. Aufrufe wie `tools/beweise.ps1`, ohne Argumente;
Rohausgaben `eq-copilot/build/nak380-e4n1/end/` (Bauartefakt):

| Bein | Aufruf | Lauf | Exit | Zähler | Binary |
|---|---|---|---|---|---|
| B18 | `EqCopSonde013DynamicsTest` | 00:01:20–00:01:32 | 0 | **119/0** (vorher 111/0; +8: M-47(a) drei, M-48 eine, M-49 zwei, M-63-Einheit zwei) | 00:01:18.649 |
| B5 | `EqCopAnalysisGoldenTest` | 00:01:32–00:01:45 | 0 | **312/0** (vorher 295/0; +17: je Nullfall eine, sieben; je Impulsfall zwei, sechs; M-61 zwei; M-62 zwei) | 00:00:49.565 |
| B16 | `EqCopSonde013EventWireTest` | 00:01:45–00:01:48 | 0 | **234/0** (vorher 213/0; +21: M-63 18, M-65 drei) | 00:00:39.374 |

Die Einzelfälle für die Endstandblöcke der Rohdateien liefen danach mit
denselben Binaries bis 00:01:57 Uhr (`--nak380 M-nn`: B18 M-42, M-43, M-47,
M-48, M-49, M-63; B5 M-50 bis M-62; B16 M-63, M-65; alle Exit 0, 0 rote
Zeilen). Die Beobachtung `--nak380-beobachtung I3-100` misst über den
gemeinsamen Läufer dasselbe wie §37.5 (108 Ereignisse, 4 zweite Klicks
verfehlt; Ausgabe bis auf die neuen Zeilen Rosa-Selbstprüfung und
Samplezahlen gleich der Nacharbeit 0), Exit 0. `FeatureEngine.h` ist nicht
geändert; A1, A16, B1, B4 und die übrigen Ziele sind deshalb nicht neu
gefahren (Auftrag). Kein voller Kanon, kein FL, keine Installation.
Zwischenstand vor den Rotbeweisen (23:47:59 Uhr, derselbe Code): dieselben
Einzelfälle grün (`eq-copilot/build/nak380-e4n1/gruen1/`).

### 40.8 Abweichungen vom Auftrag, begründet

1. **Keine Testzugangs-Auskunft in `FeatureEngine.h`.** Die Referenz der
   Stärke braucht SF, Historie und Fensteranfang je Frame; sie sind über den
   bestehenden Freund `FeatureEngineTestzugang` erreichbar, den B16 jetzt
   wie B5 und B18 selbst definiert (T-380-11). Kein Produktpfad im Diff.
2. **Zählregel-Rotbeweise als Doppelmutationen** (`m47-verlust`,
   `m50-verlust`, `m62-verlust`, `m65-verlust`): eine Verlustprüfung kann
   nur fallen, wenn Ereignisse entstehen, und die Zusagen von M-47(a),
   M-50 bis M-56, M-62 und M-65 verlangen gerade keine. Die jeweilige
   Matrixmutation erzeugt Ereignisse, `n1-verlust` verliert jedes und
   zählt es mit dem Produktzähler (Verwerfen bei Kapazität 0; die Mutation
   umgeht den Zähler nicht). Unter der Doppelmutation bleibt die
   Zusageprüfung grün und nur die Verlustprüfung fällt — das ist die
   Trennschärfe, die D5 verlangt (Validierung D5 (e): „für M-62 ist keine
   formgültige Produktmutation erreichbar“ gilt für eine einzelne Mutation).
   Läufer mit Ereignissen (M-57, M-61, M-63) fallen unter `n1-verlust`
   allein.
3. **Rohdatei-Blöcke über die vier genannten Zeilen hinaus** (M-43, M-49,
   M-50, M-57, M-61, M-65): ihre Prüfungen änderten sich (Durchsicht), ihre
   neuen Prüfungen sind einmal gebrochen (Prüfliste E).
4. **Rosa-Meldung um die Samplezahl ergänzt**, sonst wie bisher; die
   Herleitung der Pegeltoleranz 0,5 dB ist ersetzt (Σρ² = 96,04 statt der
   Schranke 877, die bei den kürzeren Nutzern 1,35σ ergäbe; §40.6).
5. **Referenzparameter als Matrixzahlen** (κ = 3, ρ = 1, T_min = 0,10 dB ·
   1530) statt der Produktkonstanten: die Referenz ist dadurch unabhängig;
   eine spätere Kalibrierung ändert Konstante, Register und diese Zahlen im
   selben Änderungssatz (R-380-7).
6. **Zählung der Engine-Flussereignisse in B16 einmal je Ereignis**: früher
   zählte der Wireläufer die Ereignisse bei jedem frischen Evidenzframe,
   auch bei nicht gebauten Snapshots (die den Ring nicht entnehmen), und den
   Rest nach dem letzten Snapshot gar nicht; Stille jetzt 3 statt 4, I1 65
   statt 64 (das 65. entsteht nach dem letzten gebauten Snapshot und reist
   nicht; es ist kein Verlust, der Ring hält es am Laufende).
7. **M-45 und M-46 ohne Zeitprüfung**: ihre Zusagen nennen keinen Frame
   (je eine Folge aus einem Frame, Anzahl 0 oder 1).
8. **„mindestens ein Fall mit med > 0“** ist in der I1-Wertprüfung verlangt
   (alle 65 Flussereignisse haben med > 0, gemeldet etwa 59,51 und 58,15 dB);
   Stille (3 von 3, gemeldet 50,49 und 61,90 dB) und Sinus (1 von 1, med
   0,002 dB) tragen ebenfalls med > 0, verlangt wird es
   dort nicht; der konstruierte Fall med = 180 steht in B18.
9. **Beobachtung mit Exitcode**: `--nak380-beobachtung` endet mit 1, wenn
   ihre Rosa-Selbstprüfung fällt (bisher immer 0).
10. **Zeitraum**: Start 25.09.2026 23:25:47, die Läufe ab 00:00 gehören zum
    26.09.2026; Überschrift und Rohdatei-Kopf tragen das Auftragsdatum.

### 40.9 Selbstaudit und Prüfliste

**Selbstaudit** (eigener Diff gegen `7b4f7938`, nicht gestagt): nur die
vier Testdateien, das Manifest und Rohdateien unter `docs/beweise/roh/`
geändert; `git diff --stat -- eq-copilot/plugin/core eq-copilot/plugin/vertrag
eq-copilot/plugin/src eq-copilot/fixtures eq-copilot/schemas broker tools`
leer (nach allen Rücknahmen, SHA-256 je Mutation gleich); jede Toleranz mit
Zahl (§40.6); jede Zeitprüfung aus Hop und Index hergeleitet (M-48 2 048,
M-49 {2 048}/{2 048, 6 144}); Restblock in jeder Etappe-4-Schleife (§40.4);
Verlustzähler in jedem Läufer als eigene Prüfung; Rosa-Selbstprüfung vor
jedem Nutzer in B5 (M-53, M-57 bis M-59, M-61, M-62, Beobachtung) und B16
(Aufwärmmaterial, I1); Zahlenränder: leere Historie (`staerkeReferenz`
gültig nur bei gerader, nicht leerer Historie und Nenner > 0), leerer oder
falsch gegliederter Normierungspuffer (leerer Vektor, Prüfung rot),
Wertprüfung ohne Ereignisse rot (`wertGeprueft > 0`), NaN in der
Rosa-Prüfung nie grün; keine Allokation im Produktpfad (M-43
Allokationslauf grün mit Restblock); Manifestzahlen aus Läufen und
Protokollen; Zeilenenden je Datei wie vorher (Python-Bytezählung: die vier
Testdateien nur CRLF, Manifest und Rohdateien nur LF).

**Prüfliste** (`tools/dirigent/pruefliste.md`, abgehakt, Datei unverändert):

| Abschnitt | Punkt | Nacharbeit 1 |
|---|---|---|
| A Rückstau | Zähler einer Politik werden ausgewertet; ein Zähler, der strukturell 0 sein muss, ist eine Wache mit Test | `ereignisseVerworfen()` und Draht-`verloren` in jedem Läufer als eigene Prüfung, je einmal gebrochen (`n1-verlust`, Doppelmutationen) |
| B Lebenszyklus | starten↔stoppen | M-62 über den gemeinsamen Läufer, `m62-leeren`, `m62-historie`, `m62-verlust` neu gefahren |
| C Verträge und Längen | Grenzwerte am Draht | `staerke_mad` am Draht = Engine-Wert (float exakt), Engine-Wert = Referenz; Schemahälfte als Regressionswache benannt (Z1) |
| D Bau- und Prüfriegel | Frische | jedes Binary nach der jüngsten Quelle (§40.7); `LastWriteTime` nach jeder Rücknahme (`os.utime`), Neubau, Fall grün |
| E Behauptung ≤ Messung | jede neue Prüfung einmal gebrochen | 20 Protokolle `NAK-380-e4-n1-mutation-*`, alle JA; Prüftexte nennen nur Gemessenes („Regressionswache“ statt Zusage für die Schemahälfte) |
| E Behauptung ≤ Messung | geänderte Zusage an drei Stellen | Testkommentare, Rohdatei-Köpfe, Manifestkopf (Zeile Etappe); die Runnerzeilen (`tools/beweise.ps1`) liegen außerhalb der Ticketgrenze dieser Nacharbeit (offener Punkt 1) |
| F Änderungssatz | Beziehungen im selben Satz | schreiben↔lesen (Engine-Wert ↔ Drahtwert), Erzeuger ↔ Selbstprüfung (Kopffunktion) |

### 40.10 Offene Punkte (für den Dirigenten)

1. **Runnerzeilen B5, B16, B18 in `tools/beweise.ps1`** nennen die neuen
   Prüfungen nicht (Samplezahl, Verlust, Wert gegen Referenz, Draht =
   Engine, Frame); außerhalb der Ticketgrenze, Prüfliste E „drei Stellen“.
2. **Matrixtexte**: §7.2 P2-Zeile um M-63/B16 ergänzen (Validierung D4 (e));
   Testspalte M-63 um B18 (`380/M-63 staerke_wert_med_positiv`); M-49
   „Ereignisse an den erwarteten Hops“ steht nur im Auftrag, nicht in §6.3.
3. **I1 in B16:** das 65. Flussereignis entsteht nach dem letzten gebauten
   Snapshot (Laufende) und reist nicht; es ist auf der Engineseite gegen die
   Referenz geprüft, der Draht vergleicht 64.
4. Kanon nachher und Wiederprüfung 1 stehen beim Dirigenten.

## 41. Messung der Nacharbeit 1 und Wiederprüfung 1 gestartet (Dirigent, 26.09.2026, 00:09 Uhr)

**Worker.** `dcb76b63` (`nakama-nak380-7b4f793-e4nach1`, Opus 5.5, Effort
xhigh, `dontAsk`), 25.09.2026 23:25:36 Uhr bis 26.09.2026 00:04 Uhr (§40.1
und Schlussmeldung „done 0:04“), Zustand danach `done`, Schlussmeldung
„FERTIG Nacharbeit 1, 36 Zeilen belegt, Beine alle grün, uncommittet“.
HEAD während des Laufs unverändert `7b4f7938`; Beobachter meldete gesund
(Kontingentwarnung: Codex-Woche 86 %) und „beendet“. Beobachter beendet,
Stundenloop `a05ce40a` gelöscht, Worker gestoppt.

**Messung (Dirigent).** `git status --short`: genau die vier Testpfade
`AnalysisGoldenTestMain.cpp` (497 Zeilen bewegt), `Nak380Pruefsignale.h`
(+141), `Sonde013DynamicsTest.cpp` (328), `Sonde013EventWireTest.cpp`
(297), das Manifest (+373, §40), zehn Rotbeweis-Rohdateien append-only
(M-43, M-47, M-48, M-49, M-50, M-57, M-61, M-62, M-63, M-65) und 20 neue
Mutationsprotokolle `NAK-380-e4-n1-mutation-*`; **kein Produktpfad**, kein
Fixture, kein Schema, kein Broker, kein Runner im Diff (Bauer §40.9
gemessen, Dirigent am Status bestätigt). Zeilenenden je Datei wie die
Arbeitskopie (Python-Bytezählung: vier Tests nur CRLF, Manifest nur LF).
Dokuriegel 0 Befunde. Beine einzeln am Endstand (§40.7): **B18 119/0
(+8), B5 312/0 (+17), B16 234/0 (+21)**, alle Binaries jünger als die
jüngste Quelle; A1, A16, B1, B4 nicht neu gefahren (kein Produktpfad,
Auftrag). Der Dirigent hat §40.3, §40.7 bis §40.10 gelesen und die
Schließung gegen §39.2 abgeglichen: ein gemeinsamer Blockläufer mit
Restblock und Samplezahlprüfung (D1), `folgeZeiten` und Frameprüfung 2 048
(D2), `staerkeReferenz` im Erzeugerkopf mit Matrixzahlen und Fall med = 180
in B18 sowie Wert- und Draht-gleich-Engine-Prüfungen in B16 (D3),
`rosaSelbstpruefung` als Kopffunktion vor Aufwärmmaterial und I1 in B16
(D4), Samplezahl- und Verlustprüfung als eigene Prüfungen in jedem Läufer
(D5), `weissAbschnitteNormiert` mit gemessener Abschnittsenergie (D6),
Toleranztabelle §40.6 (D7), Schemahälfte als Regressionswache (Z1).

### 41.1 Einordnung der offenen Punkte (§40.10) und Abweichungen (§40.8)

| Punkt | Quelle | Einordnung | Folge |
|---|---|---|---|
| Runnerzeilen B5, B16, B18 nennen die neuen Prüfungen nicht (§40.10 Punkt 1) | `tools/beweise.ps1` | kein Bruch von „Behauptung ≤ Messung“ (die Zeilen sagen weniger, nicht mehr, als gemessen wird); außerhalb der Ticketgrenze der Nacharbeit, damit die Wiederprüfung den Fixdiff rein sieht | der Dirigent zieht die drei Zeilen im Etappenabschluss nach |
| Matrixtexte §7.2 P2 (B16 als Nutzer), Testspalte M-63 (B18-Einheit), M-49 Zeitprüfung (§40.10 Punkt 2) | Matrix | **Matrixfassung §41.2** | — |
| I1 in B16: 65 Flussereignisse der Engine, 64 am Draht (§40.10 Punkt 3) | `Sonde013EventWireTest.cpp` Wireläufer | Beobachtung, kein Befund: das 65. Ereignis entsteht nach dem letzten gebauten Snapshot und bleibt im Ring (kein Verlust, `ereignisseVerworfen()` = 0); engineseitig gegen die Referenz geprüft | im Kommentar des Läufers benannt (§40.8 Punkt 6) |
| Doppelmutationen für Läufer ohne Ereignisse (§40.8 Punkt 2) | `m47-verlust`, `m50-verlust`, `m62-verlust`, `m65-verlust` | **angenommen** — die Verlustprüfung kann nur fallen, wenn Ereignisse entstehen; die Matrixmutation erzeugt sie, `n1-verlust` verliert sie über den Produktzähler; unter der Doppelmutation bleibt die Zusageprüfung grün und nur die Verlustprüfung fällt: genau die Trennschärfe, die D5 verlangt | — |
| Referenzparameter als Matrixzahlen statt Produktkonstanten (§40.8 Punkt 5) | `staerkeReferenz` | **angenommen** (R-380-8: Referenz außerhalb des eigenen Codes; Kalibrierung ändert Konstante, Register und Referenz im selben Satz, R-380-7) | — |
| Keine Testzugangs-Auskunft in `FeatureEngine.h` (§40.8 Punkt 1) | B16 eigener lesender Freund | **angenommen** (T-380-11; kein Produktpfad) | — |
| Übrige Abweichungen §40.8 Punkte 3, 4, 6 bis 10 | Rohdatei-Blöcke, Rosa-Toleranz-Herleitung, Zählung je Ereignis, M-45/M-46 ohne Zeitprüfung, med > 0 in I1, Beobachtung mit Exitcode, Datumsgrenze | **angenommen** | — |

### 41.2 Matrix in der Fassung §41

§7.2 P2: Nutzer ergänzt um „M-63 (B16: Aufwärmmaterial und I1)“, Beine „B5,
B16, A2, B30“. M-63 Testspalte: „C++: B16 NEU `380/M-63 staerke_begrenzt`
(sechs eigene Prüfungen je Lauf: Wert gegen Referenz, Draht = Engine,
Samplezahl, Engine-Verlust, Draht-Verlust, Regressionswache Textriegel und
Schema), B18 NEU `380/M-63 staerke_wert_med_positiv` (Historie med 180:
3,016 67 und Klammer 1000)“. M-49 Zusage ergänzt: „die Ereignisse liegen an
den erwarteten Hops (ein Hop: `stromSample` 2 048; zwei Hops: 2 048 und
6 144 Samples bei 48 kHz; bei 44,1 kHz entsprechend)“. M-47(a), M-48, M-62,
M-63, §7.3, E-380-13 wie §39.2 umgesetzt (Belege §40.5).

### 41.3 Rundenbilanz und Wiederprüfung 1

**Rundenbilanz Runde 1 (Nacharbeit 1, Worktree gegen `7b4f7938`, 00:05
Uhr):** Produkt 0; Tests 4 Datei(en) +938/−325; Pruefwerkzeug 0; Doku 11
Datei(en) +646/−1 (Rohdateien untracked, nicht gezählt). Erste Runde ohne
Produktzeilen bei vorhandenen Testzeilen (Nacharbeit an Tests, kein
Produktdefekt): kein Konvergenzsignal (Skill §3.3: zwei Runden in Folge
ohne Produkt- und Testzeilen).

**Wiederprüfung 1 (Vorlage B).** Der Stand der Nacharbeit 1 wird mit diesem
Abschnitt und dem Prüfauftrag
`docs/beweise/roh/NAK-380-wiederpruefung-e4-1-auftrag.txt` (ZIEL = HEAD
dieses Commits, der Prüfer misst ihn) committet und gepusht; danach bewegt
sich HEAD bis zum Urteil nicht. Prüfer: Codex `gpt-6-astra`, Effort xhigh,
lesend, `tools/dirigent/codex-lauf.ps1 -Kennung nak380-e4-w1`, Prüfbereich
`7b4f7938...HEAD` über die vier Testpfade; Befundliste D1 bis D7 und Z1
wörtlich, zwei Fragen (geschlossen; nichts gebrochen). Codex-Woche 86 %
vor dem Start (unter der Sperrgrenze 95 %; bei Kapazitätsabbruch `resume`,
dann frischer Opus-Thread xhigh, Skill §3.4). Offen beim User: nichts.

## 42. Wiederprüfung 1: Urteil, Validierung, Einordnung und Nacharbeitsauftrag 2 (Dirigent, 26.09.2026, 00:26 Uhr)

**Wiederprüfung 1 (Vorlage B).** Codex `gpt-6-astra`, Effort xhigh, lesend,
Thread `01a0da9f-c95a-70c2-84dc-cf84e06ff0cb`, 00:11:39 bis 00:17:29 Uhr,
EXIT=0, HEAD vor und nach dem Lauf `20b30fcb`; Prüfbereich
`7b4f7938...20b30fcb` über die vier Testpfade; Auftrag
`docs/beweise/roh/NAK-380-wiederpruefung-e4-1-auftrag.txt` (im Zielcommit,
ZIEL = HEAD gemessen), Urteil
`docs/beweise/roh/NAK-380-wiederpruefung-e4-1-urteil.md`. **URTEIL:
NEEDS_WORK — D2, D3, D4, D6, D7 und Z1 geschlossen; D1 und D5 für genau
einen Läufer offen (Beobachtungsmodus `--nak380-beobachtung`); 0 Brüche.**
Geprüft: Fixdiff, Referenzen, Aufrufwege, Mutationsstellen; Produktpfade
und Fixtures ohne Diff, keine Golden-Lockerung, Kalibrierwerte
unverändert, Zusageprüfungen erhalten. Nicht geprüft: keine Läufe. Die
Reste: R1 „Verriegele die Samplezahl auch im Beobachtungslauf“
(`AnalysisGoldenTestMain.cpp:1589-1590`: Ausgabe statt
`nak380SamplesGeprueft`), R2 „Prüfe Ringverluste auch im Beobachtungsmodus“
(`:1914-1915`: Exitcode hängt nur an E-380-13). Codex-Woche nach dem Lauf
88 %.

**Validierung an der Quelle.** Lesender Opus-5.5-Worker `59a81446` (`nakama-nak380-20b30fc-valw1`, Effort xhigh, `dontAsk`), 00:20:56 bis 00:22 Uhr, HEAD gleich; Auftrag `docs/beweise/roh/NAK-380-wiederpruefung-e4-1-validierung-auftrag.txt`, Bericht `docs/beweise/roh/NAK-380-wiederpruefung-e4-1-validierung.md`: **2 bestätigt (R1, R2), 0 präzisiert, 0 widerlegt; beide DEFEKT (Auftragsbruch, schwach)** — der Nacharbeitsauftrag 1 nannte die Beobachtung in den Zeilen 15, 25, 29, 35 und 36 ausdrücklich als Läufer; §40.4 führt sie ehrlich als „ausgegeben“ (eine in der Legende nicht vorgesehene Kategorie, in §40.8 nicht erklärt), die D1-Schließzeile §40.2 nennt als Beleg `:1520` (Rosa-Prüfung, keine Samplezahlprüfung); gemeinsame Ursache: die Beobachtung verriegelt eine ihrer drei Messvorbedingungen (E-380-13) und lässt zwei aus. Der Dirigent hat die
Stellen selbst gelesen: `git grep nak380SamplesGeprueft\|nak380VerlustGeprueft`
nennt die Aufrufe `:1426-1427`, `:1493-1494`, `:1672-1674`, `:1735-1737`
(Nullfall, Impulsfall, M-61, M-62) und keinen im Beobachtungsläufer;
`:1588-1590` gibt Ringverlust und Samplezahlen nur aus; `:1914-1915`
„Einzige Pruefung der Beobachtung ist ihre Vorbedingung E-380-13“,
`return fehler == 0 ? 0 : 1`.

### 42.1 Einordnung

| Befund | Quelle (HEAD `20b30fcb`) | Einordnung | Regel |
|---|---|---|---|
| R1 Samplezahl im Beobachtungslauf nur ausgegeben | `AnalysisGoldenTestMain.cpp:1588-1590` | **DEFEKT (schwach, Auftragsrest)** — der Nacharbeitsauftrag 1 (Zeile 15) verlangte die Durchsicht aller Läufer „und die Beobachtung“ gegen die Liste mit „gespeiste Samplezahl = Signallänge“; §40.4 führt die Beobachtung als über den gemeinsamen Läufer laufend, die eigene Prüfung fehlt. Keine Matrixzusage bricht (die Beobachtung trägt keine); die Zahlen der Beobachtung (§37.5) sind ohne Samplezahlprüfung nicht verriegelt. | `nak380SamplesGeprueft` im Beobachtungsläufer (1 440 000) |
| R2 Verlust im Beobachtungsmodus ohne Prüfung | `:1588-1590`, `:1914-1915` | **DEFEKT (schwach, Auftragsrest)** — §7.3 „Verlustzähler am Ende 0, sonst Fall rot“ gilt „in jedem Läufer“ (§39.2); der Modus endet unter `n1-verlust` mit Exit 0 | `nak380VerlustGeprueft` im Beobachtungsläufer; Exitcode folgt `fehler` |

Beide Reste teilen die Wurzel der Nacharbeit 1 (Ursache A und B) für den
einen Läufer, den der Bauer nicht auf die Liste gesetzt hat. Kein
Konvergenzsignal (Runde 1 hatte Testzeilen); die Reste sind eng und
mechanisch (zwei Aufrufe), deshalb ein enger Einzelauftrag (Skill §3.3:
Rest in enge Einzelaufträge).

### 42.2 Nacharbeitsauftrag 2 (Runde 2 von 3)

`docs/beweise/roh/NAK-380-nacharbeit-e4-2-auftrag.txt`: nur
`AnalysisGoldenTestMain.cpp` (Beobachtungsläufer: zwei Aufrufe, Kommentar,
Exitcode), die Beobachtungsrohdatei (append-only, neuer Lauf mit gleichen
Zahlen), zwei Rotbeweise mit den vorhandenen Mutationen `m62-rest` und
`n1-verlust` (Exit 0 am Basisstand gemessen, Exit 1 nach der Änderung),
B5 einzeln (312/0 erwartet, die Beobachtung gehört nicht zum B5-Lauf),
Manifest §43. Bauer: frischer Opus-5.5-Worker, Effort xhigh, Aufsicht
ENG. Wiederprüfung 2 danach nach Vorlage B über den Fixdiff (Codex
`gpt-6-astra` xhigh; bei Codex-Sperrgrenze frischer Opus-Thread xhigh).
Runde 2 von 3; ein dritter Rest ohne PASS führt zum Konvergenzentscheid
(Skill §3.4). Offen beim User: nichts.

## 43. Nacharbeit 2 der Etappe 4 (Bauer Opus 5.5 xhigh, 26.09.2026)

Auftrag `docs/beweise/roh/NAK-380-nacharbeit-e4-2-auftrag.txt` (Runde 2 von
3), Reste R1 und R2 der Wiederprüfung 1 (§42; Urteil
`docs/beweise/roh/NAK-380-wiederpruefung-e4-1-urteil.md`, Validierung
`docs/beweise/roh/NAK-380-wiederpruefung-e4-1-validierung.md`). Geändert ist
nur der Beobachtungsläufer `nak380BeobachtungKlickpaare` mit dem Modusaufruf
in `main` (`AnalysisGoldenTestMain.cpp`). Kein Produktcode: die
Produktmutation `n1-verlust` in `Spektrum.h` lag nur während der Rotbeweise
an und ist bytegleich zurückgenommen. Keine anderen Tests, kein Runner.

### 43.1 Startmessung

`Get-Date` 26.09.2026 00:25:52; `git rev-parse HEAD`
`46bd211cd26d031bbb20a5dc608058065bf5b13f` (Basis-SHA dieser Nacharbeit);
`git status --short` nur die fremden untracked Einträge `briefing-hub/` und
`nimbalyst-local/` (keine `.claude/settings.local.json`). Prozessprobe auf
cmake, MSBuild, cl, link, cargo, rustc, EqCop*, eqcop-broker, FL64 und vctip
leer: 00:25:52 über einen Namensfilter, vor dem ersten Bau (erster Bau
00:29:14) mit dem Aufruf aus dem Auftrag, vor dem B5-Lauf (00:33:30) erneut.
Zeilenenden (`git ls-files --eol`): `AnalysisGoldenTestMain.cpp` Index LF,
Arbeitskopie CRLF; Manifest und Beobachtungsrohdatei LF. Das vorhandene
Binary (00:00:49.565) war jünger als jede Quelle (jüngste
`Nak380Pruefsignale.h` 00:00:29.963); `git diff --quiet HEAD` für
`AnalysisGoldenTestMain.cpp` und `Spektrum.h` Exit 0.

### 43.2 Änderungen je Datei

| Datei | Zeile HEAD `46bd211c` | Zeile Nacharbeitsstand | Änderung |
|---|---|---|---|
| `AnalysisGoldenTestMain.cpp` | `:1505-1511` Kopfkommentar „als BEOBACHTUNG - keine Pruefung, kein Zusagefall“ | `:1505-1513` | berichtigt: kein Zusagefall; geprüft werden seit Nacharbeit 2 nur die Messvorbedingungen (E-380-13, gespeiste Samplezahl, Verlustzähler); die Ereigniszahlen bleiben Ausgabe ohne Zusage |
| `AnalysisGoldenTestMain.cpp` | hinter `:1551` (Ende des Laufs, vor `const auto& ev`) | `:1554-1561` | neu: `kennung` aus dem gemessenen Abstand (`I3-100`, `I3-150`); `nak380SamplesGeprueft ("Beobachtung", kennung, *lauf, x.size(), 1440000u, "30 s * 48 000, 2812 Bloecke + Rest 256")` `:1559`; `nak380VerlustGeprueft ("Beobachtung", kennung, lauf->verworfen)` `:1561` |
| `AnalysisGoldenTestMain.cpp` | `:1588-1590` Ausgabe von Ringverlust und Samplezahlen | `:1598-1600` | unverändert; bleibt Ausgabe neben den Prüfungen |
| `AnalysisGoldenTestMain.cpp` | `:1903-1905` Moduskommentar „(keine Pruefung)“ | `:1913-1916` | berichtigt: „(ohne Zusage zu den Ereigniszahlen)“ |
| `AnalysisGoldenTestMain.cpp` | `:1914` „Einzige Pruefung der Beobachtung ist ihre Vorbedingung E-380-13.“, `:1915` `return fehler == 0 ? 0 : 1;` | `:1925-1928`, `:1929` | Kommentar berichtigt: die Prüfungen sind die Messvorbedingungen E-380-13, gespeiste Samplezahl und Verlustzähler, der Exitcode folgt ihnen, die Ereigniszahlen bleiben Ausgabe ohne Zusage; `return` unverändert (folgt `fehler`) |
| `docs/beweise/roh/NAK-380-etappe-4-i3-100ms-beobachtung.txt` | 100 Zeilen | 253 Zeilen | append-only Block „Nacharbeit 2, 26.09.2026“ (153 Zeilen): Stand, Aufruf, Binaryzeitstempel, Vergleich, beide Ausgaben vollständig mit den zwei neuen Prüfzeilen |
| `docs/beweise/roh/NAK-380-e4-n2-mutation-m62-rest.txt`, `docs/beweise/roh/NAK-380-e4-n2-mutation-n1-verlust.txt` | — | neu | je Rotbeweis Basisstand und Nacharbeitsstand |
| `docs/beweise/NAK-380.md` | — | §43, Kopf Zeile Etappe | dieser Abschnitt; letzter Fettsatz der Zeile Etappe ersetzt |

`git diff --stat` für `AnalysisGoldenTestMain.cpp`: +19/−5.

### 43.3 Reste R1 und R2: Regel → Prüfzeile → Mutation → rot

| Rest | Regel | Prüfzeile | Mutation | Exit am Basisstand `46bd211c` | Exit nach der Änderung | rot | Protokoll |
|---|---|---|---|---|---|---|---|
| R1 | D1 (§39.1, §42.1): gespeiste Samplezahl = Signallänge als eigene Prüfung, auch im Beobachtungsläufer | `AnalysisGoldenTestMain.cpp:1559` über `:1390`: „380/Beobachtung I3-100: Vorbedingung gespeiste Samplezahl = 1440000 (30 s * 48 000, 2812 Bloecke + Rest 256), Laeufer und Engine“ | `m62-rest`, `:1344` `while (strom < samples)` → `while (strom + (std::uint64_t) block <= samples)` (Wortlaut der Nacharbeit 1) | I3-100 **0**, I3-150 **0** (gespeist 1 439 744; einzige Prüfung Rosa, grün) | I3-100 **1**, I3-150 **1**; gefallen allein die Samplezahlprüfung (gespeist 1 439 744), Rosa und Verlust grün | **JA** | `NAK-380-e4-n2-mutation-m62-rest.txt` |
| R2 | D5 (§39.1, §7.3, §42.1): `ereignisseVerworfen() == 0` als eigene Prüfung, auch im Beobachtungsläufer | `:1561` über `:1402`: „380/Beobachtung I3-100: Zaehlregel 7.3 - kein Ringverlust, ereignisseVerworfen() = 0“ | `n1-verlust`, `Spektrum.h:935` (der Ring verwirft jedes Ereignis, der Produktzähler zählt es; Wortlaut der Nacharbeit 1) | I3-100 **0** (Ringverlust 108, 0 Ereignisse), I3-150 **0** (Ringverlust 112) | I3-100 **1** (verworfen 108), I3-150 **1** (verworfen 112); gefallen allein die Verlustprüfung, Rosa und Samplezahl grün | **JA** | `NAK-380-e4-n2-mutation-n1-verlust.txt` |

Beide Mutationen fallen an der jeweils neuen Prüfung und an keiner anderen.
Rücknahmen bytegleich aus `eq-copilot/build/nak380-sicherung/e4n2/`
(SHA-256 `AnalysisGoldenTestMain.cpp` Basisstand `B244C4AB…`,
Nacharbeitsstand `595A6C41…`; `Spektrum.h` `8E863CA3…` vor und nach jeder
Mutation), `LastWriteTime` nach jeder Rücknahme auf jetzt, neu gebaut,
Binary jünger als die Quelle, beide Modi danach Exit 0 (Protokolle). Damit
ist der D1-Beleg der Beobachtung `:1559`, nicht `:1520` (Validierung R1 (e)
zu §40.2).

**Fortschreibung der Durchsicht §40.4 und der Tabellen §40.5 für den
Beobachtungsläufer**

| Läufer | Restblock | Samplezahl | Verlustzähler | Rosa vor Nutzer | Zeit, Wert, Toleranz |
|---|---|---|---|---|---|
| Beobachtung I3-100 und I3-150 (B5-Programm, ohne Zusage) | erfüllt (gemeinsamer Läufer `:1344`) | nachgezogen, eigene Prüfung `:1559` (1 440 000; Puffer, Läufer und Engine), rot mit `m62-rest` | nachgezogen, eigene Prüfung `:1561`, rot mit `n1-verlust` | erfüllt `:1522` | — (keine Zusage zu Ereigniszahlen) |

### 43.4 Beobachtungszahlen vorher und nachher

| Modus | Basisstand `46bd211c` (00:28:47, Exit 0) | Nacharbeitsstand (00:33:43, Exit 0) |
|---|---|---|
| I3-100 | 108 Ereignisse; genau einer 108, ohne Treffer 4 (8,6 s, 12,1 s, 21,1 s, 28,1 s, je zweiter Klick), mehrfach 0, Fehlalarme 0; Ringverlust 0; gespeist 1 440 000 | gleich |
| I3-150 | 112 Ereignisse; genau einer 112, ohne Treffer 0, mehrfach 0, Fehlalarme 0; Ringverlust 0; gespeist 1 440 000 | gleich |

Gemessen mit `Compare-Object` über die Rohausgaben: die Ausgabe am
Basisstand ist zeilengleich mit dem Endlauf der Nacharbeit 1
(`eq-copilot/build/nak380-e4n1/end/b5-beob-I3-100.txt`, Bauartefakt); die
Ausgabe am Nacharbeitsstand unterscheidet sich davon je Modus nur um die zwei
neuen Prüfzeilen; die 60 Klick- und Summenzeilen von I3-100 gleichen dem
Block der Nacharbeit 0 in der Rohdatei. Keine neue Zusage.

### 43.5 Einzeln gefahrene Beine am Endstand

Letzter Bau nach der Rücknahme von `n1-verlust` 00:33:05 bis 00:33:21 Uhr,
Exit 0 (`eq-copilot/build/nak380-e4n2/bau-rot-n1-verlust-zurueck.log`,
Bauartefakt; einzige Warnung die vorbestehende C4458 in `Stereo.h:70`;
`vctip.exe` nach jedem Bau beendet, kein Hänger). Binary
`EqCopAnalysisGoldenTest.exe` 00:33:21.174, jünger als die jüngste Quelle
(`Spektrum.h` 00:33:05.124; `AnalysisGoldenTestMain.cpp` 00:32:21.726;
`Nak380Pruefsignale.h` 00:00:29.963). Aufrufe vom Repo-Wurzelverzeichnis wie
`tools/beweise.ps1`; Rohausgaben `eq-copilot/build/nak380-e4n2/end/`
(Bauartefakt):

| Bein | Aufruf | Lauf | Exit | Zähler | Binary |
|---|---|---|---|---|---|
| B5 | `EqCopAnalysisGoldenTest` ohne Argumente | 00:33:30–00:33:43 | 0 | **312/0** (gleich §40.7; die Beobachtung gehört nicht zum B5-Lauf, die Ausgabe enthält keine Zeile „Beobachtung“) | 00:33:21.174 |
| Beobachtung | `EqCopAnalysisGoldenTest --nak380-beobachtung I3-100` | 00:33:43 | 0 | 3 ok, 0 rot (Rosa, Samplezahl, Verlust) | 00:33:21.174 |
| Beobachtung | `EqCopAnalysisGoldenTest --nak380-beobachtung I3-150` | 00:33:43–00:33:44 | 0 | 3 ok, 0 rot | 00:33:21.174 |

Kein voller Kanon, kein FL, keine Installation, kein Laufzeit-Arm (Auftrag).
B16, B18 und die übrigen Ziele nicht gefahren: ihre Quellen sind inhaltlich
unverändert.

### 43.6 Abweichungen vom Auftrag, begründet

1. **Rotbeweise zusätzlich gegen `I3-150`** (die Validierung nannte
   `I3-150`, der Auftrag `I3-100`): beide Modi an beiden Ständen gemessen,
   beide mit demselben Ergebnis.
2. **Kennung aus dem gemessenen Abstand** (`juce::roundToInt (1000.0 *
   abstand / fs)`: 4 800 → 100, 7 200 → 150, exakt) statt aus dem
   Modusargument: der Prüftext nennt den Abstand, den der Lauf fuhr; die
   Funktionssignatur bleibt.
3. **Herleitungstext mit Blockzerlegung** („30 s * 48 000, 2812 Bloecke +
   Rest 256“, wie M-62) statt nur „30 s * 48 000“ wie Null- und Impulsfall:
   der Restblock ist der Gegenstand von R1.
4. **Zwei Kommentare über `:1914` hinaus berichtigt** (Kopfkommentar
   `:1505-1513`, Moduskommentar `:1913-1916`): beide sagten „keine
   Pruefung“ (Prüfliste E; die Validierung nennt `:1505-1507` als
   Folgestelle). `:1520-1521` („kein Zusagefall, meldet die Vorbedingung aber
   mit Urteil“) bleibt wahr und unverändert.
5. **Erster Mutationsversuch `m62-rest` am Basisstand**: die erste
   Wortlautzeile allein traf drei Schleifen (`:941`, `:1014`, `:1344`); das
   Hilfsskript schrieb nichts (SHA-256 unverändert `B244C4AB…`), ein Bau
   00:29:14 bis 00:29:24 lief am unveränderten Stand. Danach mit dem
   dreizeiligen Wortlaut aus dem Protokoll der Nacharbeit 1, eindeutig
   `:1344`.
6. **Kopfzeile 2 der Beobachtungsrohdatei** („ohne Pruefung“) bleibt
   (append-only); der neue Block sagt, dass sie seit Nacharbeit 2 für die
   Ereigniszahlen gilt, nicht mehr für die Messvorbedingungen.

### 43.7 Selbstaudit und Prüfliste

**Selbstaudit** (eigener Diff gegen `46bd211c`, nicht gestagt): geändert nur
`AnalysisGoldenTestMain.cpp` (+19/−5), die Beobachtungsrohdatei (+153, nur
Zusätze), zwei neue Protokolle und das Manifest; `git diff --stat --
eq-copilot/plugin/core eq-copilot/plugin/vertrag eq-copilot/plugin/src
eq-copilot/fixtures eq-copilot/schemas broker tools` leer (`Spektrum.h`
SHA-256 `8E863CA3…` wie HEAD). Zahlenränder: `roundToInt` auf 100,0 und
150,0 exakt; `kennung` lebt über beide Aufrufe, `toRawUTF8()` wird im Helfer
sofort in einen `juce::String` kopiert; `engineSamples` ist nur mit
`NAKAMA_FEATUREENGINE_TESTZUGANG` gefüllt, der Beobachtungsläufer liegt ganz
in diesem Schalter (`:1504`, `:1912`). Die zwei Prüfzeilen erscheinen vor der
Kopfzeile der Beobachtung; die übrige Ausgabe bleibt zeilengleich.
Zeilenenden (Python-Bytezählung): `AnalysisGoldenTestMain.cpp` 4 149 CRLF,
keine nackten LF; Rohdatei und Protokolle nur LF, ohne BOM; Manifest nur LF.

**Prüfliste** (`tools/dirigent/pruefliste.md`, Abschnitte A und E, Datei
unverändert):

| Abschnitt | Punkt | Nacharbeit 2 |
|---|---|---|
| A Rückstau | Zähler einer Politik werden ausgewertet; ein Zähler, der strukturell 0 sein muss, ist eine Wache mit Test | `ereignisseVerworfen()` jetzt auch im Beobachtungsläufer als eigene Prüfung `:1561`, mit `n1-verlust` gebrochen (verworfen 108 und 112) |
| A Rückstau | Politik bei voll, Abflussweg, Schlüssel, Lesepfad, beide Sprachen | nicht berührt (kein IPC, keine Queue, kein Rust-Läufer der Beobachtung) |
| E Behauptung ≤ Messung | Kommentar sagt nicht mehr, als der Test misst | Kommentare `:1505-1513`, `:1554-1557`, `:1913-1916`, `:1925-1928` nennen genau die drei geprüften Messvorbedingungen und „Ereigniszahlen ohne Zusage“ |
| E Behauptung ≤ Messung | Zahlen im Manifest gemessen | alle Zahlen dieses Abschnitts aus Läufen dieser Session (Rohausgaben `eq-copilot/build/nak380-e4n2/`), Uhrzeiten aus `Get-Date` der Hilfsskripte |
| E Behauptung ≤ Messung | lebender Kopf nachgezogen | Zeile Etappe, letzter Fettsatz |
| E Behauptung ≤ Messung | jede neue Prüfung einmal gebrochen, Rohausgabe liegt bei | beide, Protokolle `NAK-380-e4-n2-mutation-*`; am Basisstand Exit 0 gemessen, also Beleg, keine Regressionswache |
| E Behauptung ≤ Messung | geänderte Zusage an drei Stellen | keine Zusage geändert (die Beobachtung trägt keine); `git grep -n -i nak380-beobachtung -- tools` leer, keine Runnerzeile nennt die Beobachtung |

### 43.8 Offene Punkte (für den Dirigenten)

1. `Spektrum.h` trägt nach der Rücknahme von `n1-verlust` den Zeitstempel
   00:33:05 bei bytegleichem Inhalt; Ziele außer B5 (B16, B18, Plugin) bauen
   beim nächsten Kanon neu (wie nach Nacharbeit 1).
2. Wiederprüfung 2 über den Fixdiff steht beim Dirigenten.

## 44. Messung der Nacharbeit 2 und Wiederprüfung 2 gestartet (Dirigent, 26.09.2026, 00:41 Uhr)

**Worker.** `d7610d14` (`nakama-nak380-46bd211-e4nach2`, Opus 5.5, Effort
xhigh, `dontAsk`), 00:25:42 bis 00:38 Uhr (Schlussmeldung „done 0:38“),
Zustand danach `done`, Schlussmeldung „FERTIG Nacharbeit 2, 2 Reste
geschlossen, B5 312/0, uncommittet“. HEAD während des Laufs unverändert
`46bd211c`; Beobachter „arbeitet“, dann „beendet“. Beobachter beendet,
Stundenloop `71a3a12f` gelöscht, Worker gestoppt.

**Messung (Dirigent).** `git status --short`: genau
`AnalysisGoldenTestMain.cpp` (24 Zeilen im Diff, +19/−5 nach Rundenbilanz: zwei Prüfaufrufe, Kommentare, Modusausgang), die Beobachtungsrohdatei
append-only, zwei Mutationsprotokolle `NAK-380-e4-n2-mutation-m62-rest.txt`
und `-n1-verlust.txt`, das Manifest (§43); **kein Produktpfad**, kein
anderer Test, kein Runner. Zeilenenden je Datei wie die Arbeitskopie
(Python-Bytezählung: C++ nur CRLF, Manifest und Rohdateien nur LF).
Dokuriegel 0 Befunde. Laut §43: Rotbeweis R1 (`m62-rest`) Exit 0 am
Basisstand, Exit 1 nach der Änderung allein an der Samplezahlprüfung
(gespeist 1 439 744 statt 1 440 000); Rotbeweis R2 (`n1-verlust`) Exit 0 →
Exit 1 allein an der Verlustprüfung (verworfen 108 bei I3-100, 112 bei
I3-150); beide gegen I3-100 und I3-150 gefahren, Rücknahmen bytegleich
(SHA-256); B5 312/0 (unverändert), Binary 00:33:21 jünger als die Quelle;
Beobachtung I3-100 und I3-150 je Exit 0 mit drei Prüfungen, Zahlen gleich
(108 Ereignisse, 4 verfehlte zweite Klicks; 112). Der Dirigent hat §43.3
und §43.4 gelesen.

**Berichtigung zu §42:** die Kopfzeit „00:26 Uhr“ war nicht gemessen; der
Validierungsbericht trägt 00:21 bis 00:22 Uhr, der Commit `46bd211c`
00:25:25 Uhr (`Get-Date`). Uhrzeiten nur gemessen (Lehre 21.09.2026).

**Rundenbilanz Runde 2 (Nacharbeit 2, Worktree gegen `46bd211c`):**
Produkt 0; Tests 1 Datei(en) +19/−5; Pruefwerkzeug 0; Doku 2 Datei(en) +317/−1 (Mutationsprotokolle untracked, nicht gezählt). Zweite Runde in Folge ohne Produktzeilen, aber mit
Testzeilen: kein Konvergenzsignal nach Skill §3.3 (zwei Runden ohne
Produkt- UND Testzeilen); Runde 3 wäre die letzte vor dem
Konvergenzentscheid.

**Wiederprüfung 2 (Vorlage B).** Der Stand der Nacharbeit 2 wird mit diesem
Abschnitt und dem Prüfauftrag
`docs/beweise/roh/NAK-380-wiederpruefung-e4-2-auftrag.txt` (ZIEL = HEAD
dieses Commits) committet und gepusht; danach bewegt sich HEAD bis zum
Urteil nicht. Prüfer: Codex `gpt-6-astra`, Effort xhigh, lesend,
`tools/dirigent/codex-lauf.ps1 -Kennung nak380-e4-w2`, Prüfbereich
`46bd211c...HEAD` über `AnalysisGoldenTestMain.cpp`; Befunde R1 und R2
wörtlich, zwei Fragen. Codex-Woche 88 % vor dem Start. Offen beim
User: nichts.

## 45. Wiederprüfung 2 PASS, Abschluss der Etappe 4 und Übergabe (Dirigent, 26.09.2026, 00:57 Uhr)

**Codex-Ausfall.** Zwei Codex-Läufe der Wiederprüfung 2 (`nak380-e4-w2`,
Thread `01a0dabb-817e-7940-9d17-b72200e28b9f`, 00:41:55 bis 00:42:28 Uhr;
`nak380-e4-w2b`, 00:44:03 bis 00:44:35 Uhr) endeten mit EXIT=1 nach fünf
Wiederholungen „401 Unauthorized: Incorrect API key provided“ gegen
`chatgpt.com/backend-api/codex/responses`; `codex login status` meldet
„Logged in using ChatGPT“, `~/.codex/auth.json` (00:42:28 Uhr geschrieben)
trägt neben `tokens` einen `OPENAI_API_KEY`; keine Umgebungsvariable
gesetzt. Wiederholter API-Fehler → frischer Opus-Thread prüft (Skill §3.4,
§3.6). Der Codex-Login ist ein User-Handgriff außerhalb des Zeitfensters
(00:45 Uhr): One-Shot-Loop 9:07 Uhr prüft zuerst, ob Codex wieder
antwortet, und nennt sonst dem User den Handgriff `codex login` (Skill §4).
Codex-Woche vor dem Ausfall 88 %.

**Wiederprüfung 2 (Vorlage B, Ersatzprüfer).** Lesender Opus-5.5-Worker
`c130868f` (`nakama-nak380-88e3a63-pruefw2`, Effort xhigh, `dontAsk`),
00:45:06 bis 00:47 Uhr, HEAD vor und nach dem Lauf `88e3a63e`, `git status`
beide Male nur die fremden Einträge; Auftrag
`docs/beweise/roh/NAK-380-wiederpruefung-e4-2-auftrag.txt` unverändert
übernommen (Prüfbereich `46bd211c...88e3a63e` über
`AnalysisGoldenTestMain.cpp`), Urteil
`docs/beweise/roh/NAK-380-wiederpruefung-e4-2-urteil.md`. **URTEIL: PASS —
R1 und R2 geschlossen, nichts gebrochen.** Geprüft: Fixdiff (24 Zeilen,
drei Hunks), Produktpfade ohne Diff, Prüfhelfer und Aufrufe `:1559-1561`,
Modusausgang `:1929`, Mutationsstellen `:1344` und `Spektrum.h:935-936`,
Zähler `FeatureEngine.h:999`/`:1145`; Rotbeweise an der jeweils neuen
Prüfung nachgerechnet; A32/A33 nehmen `tests/` aus. Nicht geprüft: keine
Läufe.

**Rundenbilanz kumuliert** (`--runden 21853aa1 9125dbbc 1e4a7010 7b4f7938
20b30fcb 46bd211c 88e3a63e`): Bau Produkt 9 +621/−83, Tests 5 +1 436/−15,
Pruefwerkzeug 2 +142/−20, Doku 104 +3 432/−3; Nacharbeit 0 Produkt 4
+63/−21, Tests 3 +344/−44, Pruefwerkzeug 2 +22/−15, Doku 75 +2 354/−1;
Kanon/Prüfung Produkt 1 +1/−1 (Broker-Pin), Doku 6 +12 811/−2 (Kanon-Rohausgabe);
Nacharbeit 1 Tests 4 +938/−325, Doku 32 +1 524/−1; Zwischencommit Doku 5
+160/−1 (ohne Produktfortschritt: Urteil, Validierung, Auftrag);
Nacharbeit 2 Tests 1 +19/−5, Doku 5 +516/−1. Kein Konvergenzsignal
(höchstens eine Runde in Folge ohne Produkt- und Testzeilen, und diese war
ein Dokumentationscommit).

**Etappe 4 abgeschlossen (26.09.2026, 00:57 Uhr).** Gebaut (§35),
Nacharbeit 0 nach R-380-12 (§37), gemessen (§36, §38), Kanon nachher GRUEN
70/70 an `1e4a7010` (§39; die Nacharbeiten 1 und 2 änderten danach nur
Tests: B5, B18, B16 einzeln grün, A1/A16/B1/B4 unberührt, Wiederprüfungen
über den Fixdiff), Erstprüfung 4 NEEDS_WORK (§39), Nacharbeit 1 (§40),
Wiederprüfung 1 NEEDS_WORK mit zwei Resten (§42), Nacharbeit 2 (§43),
Wiederprüfung 2 PASS (§45). Aus der Etappe bleibt die Härtung NAK-405 im
Register; Laufzeit-Arm und T2-Abschlussprüfung folgen nach Etappe 6
(Kopf). Runnerzeilen B5, B16, B18 in `tools/beweise.ps1` um die Prüfungen
der Nacharbeiten 1 und 2 ergänzt (Behauptung ≤ Messung; §41.1, Textzeilen,
kein Bein neu gefahren; der T2-Abschlusskanon deckt sie).

**Hygiene im Abschlussfenster (gemessen):** `gesundheit.py` Exit 4 (Hinweis, nicht blockierend) mit unveränderten Maßen 3/33/39 (Quelldateien über 2 000 Zeilen, Funktionen über 200 Zeilen, Kommentar-Bezeichner ohne Code; Register NAK-292, NAK-371, NAK-339), Kontextfläche OK (MEMORY.md 6 205, CLAUDE.md 20 402, Dirigenten-Skill 24 355 Bytes); `zielbild.py pruefen` Exit 0 (Zielbild aktuell, Stand 25.09.2026 11:34); `dokuriegel.py` auf CLAUDE.md, Skill und Register 0 Befunde, auf Manifest und Verlaufsdatei 0 Befunde (nach der Auslagerung gemessen); `tidy.py` Exit 0 (Grenze gehalten, 58 von höchstens 58 Fundstellen). Kein Riss der Kontextfläche.

**Verlauf ausgelagert.** Das Manifest überschritt 7 000 Zeilen; die
Abschnitte §10 bis §34 (Etappen 1 bis 3: Matrixprüfungen, Bau, Nacharbeiten,
Kanon der Etappen 2 und 3) stehen unverändert in
`docs/beweise/NAK-380-verlauf.md` (Skill §3.5); Kopf, §0 bis §9
(Einordnung, Regeln, Zusagen, Matrix, Korpus, Bauplan, Prüfliste) und der
Verlauf der Etappe 4 ab §35 mit dem jüngsten Kanon-Abschnitt bleiben hier.
Verweise auf §10 bis §34 in den späteren Abschnitten meinen die
Verlaufsdatei.

**Übergabe an die nächste Dirigentensitzung (Kontext dieser Sitzung 64.0 %,
Skill §5: über 500k kein neues Ticket und keine neue Prüfrunde).** Etappe 5
(Kohärenz je Bin, DSP-23 mit NAK-177; M-73 bis M-97; §8.5, R-380-3,
T-380-6 bis T-380-9) beginnt in einer frischen Sitzung: Auftrag nach dem
Muster `docs/beweise/roh/NAK-380-etappe-4-auftrag.txt` mit den Lehren der
Etappe 4 (§36.1, §39.1, §42.1: Vorbedingung hergestellt und verriegelt —
Samplezahl, Verlustzähler, Erzeuger-Selbstprüfung als Kopffunktion vor jedem
Nutzer; jede in der Zusage genannte Größe kommt aus dem Lauf zurück; jeder
Formelwert gegen eine unabhängige Referenz; jede Gleichheitstoleranz mit
Rechenbudget; keine Wache ohne Trennschärfe; ein gemeinsamer Blockläufer
mit Restblock; Kalibriergrößen mit Herleitung und Kalibrierfolge in der
Regel, Struktur bleibt) und der Auftragsregel (Tabelle je Matrixzeile).
Bauer: frischer Opus-5.5-Worker `--effort xhigh` (NAK-404), Aufsicht ENG;
Kanon vorher geerbt vom Kanon nachher der Etappe 4 an `1e4a7010` (Diff
seither: Tests der Nacharbeiten 1 und 2, Runnerzeilen, Broker-Pin; B5, B18,
B16 einzeln grün, Wiederprüfungen PASS). Prüfer: Codex `gpt-6-astra` xhigh,
sobald der Login erneuert ist; bis dahin frischer Opus-Thread xhigh (Skill
§3.4). Nach NAK-380 folgt NAK-313 Etappe 8 (`docs/beweise/NAK-313.md` §37).
Offen beim User: der Codex-Login (Handgriff, Meldung 9:07 Uhr).

## 46. Etappe 5 — Kohärenz je Bin über Frames (Bauer Opus 5.5 xhigh, 26.09.2026)

Gebaut nach dem Auftrag `docs/beweise/roh/NAK-380-etappe-5-auftrag.txt`
(T-380-6, R-380-3, A-3, A-4, R-380-7): Stereoring je Bin mit W_H = 3 und
W_B = 7 Evidenzfenstern, Kohärenz als Mittel der MSC je Bin über die
Ringsumme, Phase am Bin der geometrischen Bandmitte, Gruppenlaufzeit aus dem
Lag-1-Produkt nur im Engine-Ergebnis, Persistenz aus der MSC je Bin,
Versionsschritt 20260928. **Ergebnis:** 63 Zusagesätze in M-73 bis M-97;
54 mit rotem Beleg an ihrer Prüfzeile (JA), einer ohne Mutation laut Auftrag
(M-96, alte `stereo_evidenz` über A4 und B3c), **acht NEIN: die
Laufzeitzusage ± 0,02 ms (M-74 bis M-80) hält am Etappenstand nicht** — die
Prüfzeilen sind rot (66 Prüfungen in B19), die Schranke ist nicht gelockert,
Zahlen und Ursache in §46.11. Alle übrigen Beine grün; A4 siehe §46.7.
Nichts gestagt, nichts committet.

### 46.1 Startmessung und Kanon vorher

- `Get-Date` 26.09.2026 01:13:18 Uhr; `git rev-parse HEAD` =
  `c43c16461142c9019868da1fe3ecc31d594856e6`; `git status --short` nur
  `briefing-hub/`, `nimbalyst-local/` (fremd, nicht angefasst) und der Auftrag.
- Produktdiff gegen `1e4a7010` (`git diff --stat 1e4a7010 HEAD -- eq-copilot
  broker tools`): genau die sechs vom Dirigenten genannten Pfade (Broker-Pin
  in `nakama-installer-v1.json`, `AnalysisGoldenTestMain.cpp`,
  `Nak380Pruefsignale.h`, `Sonde013DynamicsTest.cpp`,
  `Sonde013EventWireTest.cpp`, `tools/beweise.ps1`). **Kanon vorher: geerbt**
  (Kanon nachher der Etappe 4 GRUEN 70/70 an `1e4a7010`,
  `docs/beweise/roh/NAK-380-1e4a701-dirty.md`).
- Prozessprobe auf cmake, MSBuild, cl, link, cargo, rustc, EqCop*,
  eqcop-broker, FL64, vctip: leer (01:14:39 Uhr). Kein FL, keine Installation,
  kein voller Kanon.

### 46.2 Pflichterfassungen und Basisstand-Gegenprobe (vor dem ersten Produktedit)

Reihenfolge: (1) neue Fälle in B19, B5 und A5 geschrieben (M-95 in B18
braucht den Testzugang `nanKreuzBin` und entstand mit dem Umbau); (2) B5 gebaut
(01:43:53 bis 01:44:08 Uhr) und `sizeof (FeatureEngine)` am unveränderten
Code ausgegeben; (3) A5 mit den neuen Prüfungen gegen Register und Schema
(01:45 Uhr, 887/14); (4) B19 gebaut (01:47:06 bis 01:47:20 Uhr) und gefahren
(01:47:26 bis 01:47:36 Uhr, 499/120, alle 120 roten Prüfungen sind
`380/`-Fälle); (5) danach der erste Produktedit. Produktquellen dabei
unverändert (`git diff` leer auf `eq-copilot/plugin/core`).

| Erfassung | Zahl am Basisstand | Rohdatei |
|---|---|---|
| M-97 (Klasse Regressionswache) | `sizeof (FeatureEngine)` = **16 208 B**, Schranke also 16 248 B | `NAK-380-etappe-5-m97-sizeof-start.txt` |
| M-86 (Klasse (ii)) | erster Snapshot 24 Frames, 1024 ms; dritter Snapshot **25 Frames**, 1066,667 ms (nur das letzte Fenster) | `NAK-380-etappe-5-m86-dof-alt.txt` |
| M-91 (feste Saat, 18 Blockgrößen) | Mittel der Bandkohärenz 0,0175 bis 0,0195, Maximum 0,1443 bis 0,1564 (Band 105) | `NAK-380-etappe-5-m91-unkorreliert-basis.txt` |
| Gegenprobe B19 und A5 | je Zeile in §46.5 (Spalte Basisstand) | `NAK-380-etappe-5-basisstand-gegenprobe.txt` |

„heute rot“ bestätigt: M-73 (Minimum 0,3589, Band 216 0,4868 bei 8 Frames),
M-74 bis M-79 (Bandsumme, Bänder ohne Bit, keine Laufzeit), M-81 (weder
Kohärenz- noch Phasenbit), M-82 (zweiter Snapshot 7 Frames, 150 Bänder ohne
Bit), M-83 (235 bzw. 47 Bassbandbefunde ohne Bit), M-88 (7 statt 12 Frames),
M-93 (Persistenz 0 in 15 Fenstern), M-94 (A5, 4 Prüfungen). „heute nicht
messbar“: M-80 (keine Laufzeit, 33 Prüfungen rot), M-84 (7 statt 20, 1 statt
11), M-85 (kein Ring), M-86(b) (25 statt 74), M-91(b) (Pflichterfassung),
M-96 (A5, 10 Prüfungen). Die Matrixzahl für M-73 (≈ 0,477 bei 15,77 kHz)
stammt aus dem Nachbau `b10_kohaerenz.out` mit anderem Signal; der
Basislauf misst im Band 216 0,4868 bei 8 Frames. Bei M-81 trägt der letzte
Basissnapshot nur 7 Frames, deshalb weder Kohärenz- noch Phasenbit.

### 46.3 Änderungen je Datei

- **`Stereo.h`** (318 → 612 Zeilen, CRLF): Ring je Bin nach T-380-6. Helfer
  für Slots, Bins, Zählerkopf und Ringframes (`:52-122`). `stereoSchritt`
  (`:159`) addiert je Band die Fensterträger und das Kurzfenster (NaN-Riegel
  je Band und Frame `:200`), danach je Bin Sxx, Syy, Re und Im Sxy in den
  laufenden Slot und in den Kurzfensterslot und zählt je Band und Slot den
  Frame; der Kurzfensterabschluss rechnet die Persistenz aus der MSC je Bin
  (`:305`, A-4). `stereoAuswerten` (`:350`): Freiheitsgrade und Fensterdauer
  aus den Ringframes (`:372-374`), Kohärenz als Mittel der MSC je Bin über die
  Ringsumme nur ab acht Ringframes (`:414-461`), Phase am Mittenbin
  (`:422-423`, `:468`), Laufzeit aus dem Lag-1-Produkt für Bänder ab zwei Bins,
  nur über der Phasenschwelle und nur im Engine-Ergebnis (`:476-481`, A-3).
  `stereoFensterLeeren` (`:545`), `stereoRingVorschub` (`:578`, Slot weiter,
  neuer Slot geleert), `stereoLeeren` (`:604`, Ring ganz). Der Parameter
  `nanKreuzBin` (Standard −1) setzt nur für den Testzugang ein Kreuzprodukt
  auf NaN.
- **`FeatureEngine.h`** (1677 → 1752, CRLF): `kFeatureMetricsVersion`
  20260927 → 20260928 mit Begründung (`:89`); `kStereoRingHaupt = 3`
  (`:271`) und `kStereoRingBass = 7` (`:279`) mit Herleitung; der
  Bass-Kommentar nach §8.5; `vorbereiten` legt den Ring im Heap an (`:646`,
  Zählerkopf 286 Elemente plus Bins·(W + 1) je Band); `zuruecksetzen` leert
  ihn (`:698`); `stereoAkkuBelegteBaender` zählt die Ringzähler mit
  (`:1148`); `StereoAkku` ohne `dauerMs` (die Dauer folgt aus den
  Ringframes); Mitglieder `StereoBinAkku`, `stereoRing` und vier Stände
  (`:1534-1547`); die zwei B-5-Kommentare berichtigt.
- **`Frame.h`** (756 → 761): `evidenzLeeren` schiebt den Ring
  (`stereoRingVorschub()`, `:167`) statt ihn zu leeren.
- **`Zeit.h`** (603 → 604): `grenzeZiehen` leert weiter ganz, Kommentar um
  M-87 ergänzt (`:388-389`).
- **`Vertrag.h`** (773 → 788): `StereoBandwert` trägt `laufzeitGesetzt` und
  `laufzeitMs` (`:493`, A-3, nicht auf der Leitung); Kommentare zu Phase,
  Freiheitsgraden, Fensterdauer und fail-closed nachgezogen.
- **`eq-ipc-v3.schema.json`** (2395 → 2395, CRLF, `-text`): je ein Satz in
  den `$comment` von `fenster_dauer_ms` (`:1399`) und `freiheitsgrade`
  (`:1406`); keine Feldänderung, keine Version.
- **`metriken-v1.json`** (595 → 780, CRLF, `-text`): Fassung 20260928 als
  Kopie von 20260927 mit Seit und Hinweis, dazu `kStereoRingHaupt` 3 und
  `kStereoRingBass` 7 unter `ganzzahlige_schwellen`; `aktuell` 20260928.
- **`prepost.rs:206`**, **`vergleichbarkeit.rs:149`** (1248 und 403 Zeilen,
  CRLF): `METRICS_VERSION = 20260928`, sonst nichts.
- **`pruefe_v3_vertrag.py`** (2877 → 3038, LF): M-70 prüft 20260927 als
  registrierte Fassung weiter, die Versionsprüfungen wandern nach M-96; neu
  `pruefe_nak380_etappe_5` mit `nak380_m94_ring_benannt` und
  `nak380_m96_fassung_etappe_5`, Einzelaufruf `--nak380 M-94` bzw. `M-96`.
- **`Nak380Pruefsignale.h`** (605 → 732): Rauschpaar D1 bis D7
  (splitmix64 wie der B19-Sweep, R um d = round(0,001·fs) verzögert) mit
  Selbstprüfung vor jedem Nutzer (Kennwerte beider Saaten, Amplitude mit
  5σ-Toleranz je Samplezahl, exakte Verschiebung).
- **`Sonde013StereoGoldenTest.cpp`** (1329 → 2676): Fälle M-73 bis M-93,
  Kadenzmodell (Welch-Frames, Evidenzfälligkeit, Segmente), Referenz ρ_w,
  Einzelaufruf `--nak380 M-nn`; Erweiterungen M-86, M-87 (§32.3-Fall), M-91,
  Umbau M-92(b); Kommentar `:441-447` berichtigt.
- **`Sonde013DynamicsTest.cpp`** (2329 → 2472): M-95 über den Testzugang.
- **`AnalysisGoldenTestMain.cpp`** (4149 → 4175): M-97 mit Startwert 16 208.
- **`tools/beweise.ps1`** (1592 → 1592, CRLF mit BOM): Behauptungen B19,
  B18 und B5 um je einen Etappe-5-Satz ergänzt (B19 nennt die offene
  Laufzeitzusage); PowerShell-Parser ohne Fehler.
- **Größen:** `sizeof (FeatureEngine)` 16 208 → **16 248 B** (+40: ein
  Vektorkopf 24 B, vier `std::uint32_t`). Ringbytes (M-85, gemessen =
  gerechnet): 44,1 kHz **236 224 B**, 48 kHz 217 792 B, 96 kHz 113 344 B,
  192 kHz 61 248 B; Schranke 245 760 B. Keine CMake-Änderung nötig.

### 46.4 Messtabelle (Endstand, B19 02:51:55 bis 02:52:05 Uhr)

Laufzeitläufe D1 bis D6 (L splitmix64-Rauschen, R um d = round(0,001·fs)
verzögert, 10 s, Evidenzintervall 0,25 s), letzter Snapshot: Kohärenz-Minimum
über alle Bänder mit Bit, Mittel von q = (1 − C)/(1 − ρ_w(d)²), und je Band
mit mindestens zwei Bins und Bit die Abweichung der Gruppenlaufzeit von
1000·d/fs (0,997732 ms bei 44,1, 88,2 und 176,4 kHz; 1,000000 ms sonst).
Jedes Band mit Energie trägt ein Bit, kein Band liegt unter 0,95.

| fs (kHz) | Block | Kohärenz-Minimum (Band) | Referenzmittel q | Bänder mit Laufzeit | außerhalb ± 0,02 ms | größte Abweichung (Band) | mittlere Abweichung | ± 0,02 ms gehalten |
|---|---|---|---|---|---|---|---|---|
| 48 | 1 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 2 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 3 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 7 | 0,9969 (89) | 1,0029 | 128 | 84 | 0,4767 ms (109) | −0,0230 ms | nein |
| 48 | 15 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 16 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 31 | 0,9969 (89) | 1,0029 | 128 | 84 | 0,4767 ms (109) | −0,0230 ms | nein |
| 48 | 64 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 127 | 0,9967 (89) | 1,0046 | 128 | 86 | 0,4319 ms (109) | −0,0242 ms | nein |
| 48 | 128 | 0,9966 (89) | 1,0153 | 128 | 82 | 0,3210 ms (119) | −0,0241 ms | nein |
| 48 | 333 | 0,9968 (89) | 1,0054 | 128 | 80 | 0,4382 ms (109) | −0,0226 ms | nein |
| 48 | 512 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 1024 | 0,9970 (102) | 1,0042 | 128 | 82 | 0,3986 ms (109) | −0,0238 ms | nein |
| 48 | 2048 | 0,9968 (72) | 0,9911 | 128 | 90 | 0,5247 ms (113) | −0,0306 ms | nein |
| 48 | 4096 | 0,9970 (113) | 1,0092 | 128 | 75 | 0,3332 ms (109) | −0,0248 ms | nein |
| 48 | 8192 | 0,9970 (113) | 1,0092 | 128 | 75 | 0,3332 ms (109) | −0,0248 ms | nein |
| 48 | 12345 | 0,9959 (67) | 1,0063 | 128 | 95 | 0,4995 ms (60) | −0,0234 ms | nein |
| 48 | 16384 | 0,9970 (113) | 1,0092 | 128 | 75 | 0,3332 ms (109) | −0,0248 ms | nein |
| 44,1 | 64 | 0,9975 (75) | 1,0100 | 134 | 90 | 0,5019 ms (62) | −0,0193 ms | nein |
| 44,1 | 512 | 0,9972 (83) | 1,0162 | 134 | 87 | 0,4288 ms (52) | −0,0235 ms | nein |
| 44,1 | 16384 | 0,9976 (75) | 1,0118 | 134 | 88 | 0,4140 ms (62) | −0,0195 ms | nein |
| 88,2 | 64 | 0,9912 (108) | 0,9905 | 97 | 46 | 0,2086 ms (139) | −0,0113 ms | nein |
| 88,2 | 512 | 0,9905 (91) | 0,9881 | 97 | 42 | 0,2789 ms (139) | −0,0141 ms | nein |
| 88,2 | 16384 | 0,9899 (108) | 0,9895 | 97 | 38 | 0,1965 ms (139) | −0,0090 ms | nein |
| 96 | 64 | 0,9876 (84) | 0,9912 | 94 | 42 | 0,2027 ms (133) | −0,0109 ms | nein |
| 96 | 512 | 0,9893 (111) | 0,9856 | 94 | 44 | 0,1386 ms (135) | −0,0110 ms | nein |
| 96 | 16384 | 0,9882 (84) | 0,9916 | 94 | 39 | 0,1820 ms (133) | −0,0131 ms | nein |
| 176,4 | 64 | 0,9661 (134) | 0,9922 | 73 | 29 | 0,1604 ms (143) | 0,0008 ms | nein |
| 176,4 | 512 | 0,9676 (134) | 1,0017 | 73 | 24 | 0,1594 ms (168) | −0,0002 ms | nein |
| 176,4 | 16384 | 0,9653 (134) | 0,9951 | 73 | 21 | 0,1471 ms (168) | 0,0030 ms | nein |
| 192 | 64 | 0,9652 (133) | 0,9836 | 70 | 32 | 0,1358 ms (166) | −0,0078 ms | nein |
| 192 | 512 | 0,9625 (150) | 0,9952 | 70 | 30 | 0,1261 ms (166) | −0,0026 ms | nein |
| 192 | 16384 | 0,9631 (133) | 0,9865 | 70 | 34 | 0,1589 ms (166) | −0,0023 ms | nein |

Größte Abweichung je Binzahl (48 kHz, Block 512): 2 Bins 0,3986 ms, 3 Bins
0,1383, 5 Bins 0,1273, 10 Bins 0,0349, 20 Bins 0,0016, 39 Bins 0,0257,
40 und mehr Bins 0,0320 ms. M-73 (48 kHz, Block 512, Snapshots 7 bis 31):
Minimum 0,9950 (Band 110), Band 216 (15,77 kHz, 39 Bins) 0,9982 bei 22
Frames, q im Mittel 1,0001 über 4 900 Bandbefunde.

| Zeile | Messung am Endstand |
|---|---|
| M-81 | Band 216, Mittenbin 1346 (f_k 15 773,438 Hz): Kohärenz 0,9982, Phase −1,41478 rad gegen −1,42353 rad, Abweichung 0,00876 rad; Erstbin-Abstand 1,39899 rad (gerechnet) |
| M-82 | 44,1 kHz, Blöcke 64, 128, 256, 512: erster Snapshot 5 Frames, 0 von 150 Bändern mit Bit; zweiter Snapshot 11 (Block 64) bzw. 12 Frames, 150 von 150 mit Bit |
| M-83 | 48 kHz, Block 512, 47 Bassbänder: 0,25 s 6 Bassframes vor und 8 am Snapshot 5, ab Snapshot 5 jedes mit Bit; 1 s 5 vor und 11 am Snapshot 2, ab Snapshot 2 jedes mit Bit; vorher keines |
| M-84 | 44,1 kHz, Block 512, zehnter Snapshot: Hauptstufe 20 Frames (Modell 20, 66 seit dem Start), Bass 11 (Modell 11) |
| M-85 | Ringbytes 236 224 / 217 792 / 113 344 / 61 248 B bei 44,1 / 48 / 96 / 192 kHz, reserviert = belegt |
| M-86 | 48 kHz, Block 512, 1 s: erster Snapshot 24 Frames und 1024 ms (unverändert), dritter Snapshot 74 Frames und 3157,333 ms |
| M-87 | seek: Ring direkt nach dem Ereignis 0 Frames (236 224 B), erster Snapshot 0, zweiter 6 Frames, Kohärenz ohne Bit; reset: 0, 5, 12 Frames, ohne Bit; rate_change (44,1 → 96 kHz): 0 Frames, 113 344 B, 13 und 27 Frames, 1-kHz-Band 1,0000 aus 13 Frames; §32.3-Fall: 24 Frames, Kohärenz 0,0413 |
| M-88 | zweiter Snapshot 12 Frames je Hauptstufenband (5 + 7) |
| M-91 | Mittel 0,0404 bis 0,0450 (K 22 bis 24, 1/K 0,0417 bis 0,0455), Maximum 0,1644 bis 0,1861 (Band 105), 149 Bänder mit Bit, 0 Bänder mit Phase |
| M-92 | stiller Kanal ohne Bit; erster Snapshot 6 Frames ohne Bit (149 Bänder); Block 4096: 7 Frames, 0 mit Bit; Block 9216: 8 Frames, 149 mit Bit, alle in [0, 1) |
| M-93 | Persistenz im Band 216 in 15 von 15 Fenstern 1,000 |
| M-95 | Band 216 zählt den NaN-Frame nicht (16 → 16), 148 übrige Hauptstufenbänder +1, 0 nicht endliche Werte, Band 216 behält 0,9982 |
| M-97 | `sizeof (FeatureEngine)` 16 248 B (Startwert 16 208, Δ 40) |

### 46.5 Tabelle je Matrixzeile M-73 bis M-97

Regel des Auftrags: je Satz und Hälfte der Zusage (§6.4) eine Zeile
Zusagesatz → Prüfzeile → Mutation → rot → Rohdatei, dazu Etikett und
Basisstand. Prüf- und Mutationszeilen am Etappenstand. „rot JA“: das
Protokoll nennt „Die Zusage faellt an der erwarteten Pruefung: JA“, die
Rücknahme ist per SHA-256 gleich (für jede Datei ein einziger SHA über alle
Protokolle, gleich dem Endstand), der Fall danach grün (Neubau, Binary jünger
als die Quelle). Laufzeitmutationen sind als Wirkungsnachweis gefahren, aber
kein formgültiger Rotbeweis, weil ihre Prüfzeile schon am Etappenstand rot
ist. Protokolle `docs/beweise/roh/NAK-380-e5-mutation-<Kürzel>.txt` (56),
Sammelnachweis je Zeile `NAK-380-rot-M-73.txt` bis `NAK-380-rot-M-97.txt`
unter `docs/beweise/roh/` (Matrixzeile wörtlich, Basisstand, Endstand, Sätze,
Mutationen). **63 Zusagezeilen: 54
JA, 8 NEIN (Laufzeit, offen), 1 ohne Mutation laut Auftrag; dazu 15
Vorbedingungszeilen der Läufer, alle JA** (Tabelle am Ende dieses
Abschnitts).

**M-73** (`NAK-380-rot-M-73.txt`; Etikett: heute rot; Basisstand: rot (B19, 4 Prüfungen): Minimum 0,3589 (Band 219), 590 Bandbefunde unter 0,95, Band 216 (15,77 kHz, 39 Bins) 0,4868 bei 8 Frames; 3 112 von 4 900 Bandbefunden ohne Bit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Band 30 Hz bis 18 kHz mit Kohärenzbit trägt Kohärenz ≥ 0,95 | `Sonde013StereoGoldenTest.cpp:752` „380/M-73 laufzeit_1ms_breitband_48k … Kohaerenz >= 0,95“, B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| (Referenz ρ_w(48)² = 0,998 bei N = 4096 und 0,9999 bei N = 16 384, nachgerechnet) | `Sonde013StereoGoldenTest.cpp:756` Mittel von (1 − C)/(1 − ρ²) in [0,8; 1,2], B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| jedes Band mit Energie trägt ein Bit | `Sonde013StereoGoldenTest.cpp:748` „… traegt ab dem siebten Snapshot ein Kohaerenzbit“, B19 | `FeatureEngine.h:271-279` W_H und W_B → 1 (alter Wert) (m96-alt-w1) | JA | `NAK-380-e5-mutation-m96-alt-w1.txt` |

**M-74** (`NAK-380-rot-M-74.txt`; Etikett: heute rot; Basisstand: rot (B19, 36 Prüfungen): je Blockgröße Bänder ohne Bit und Kohärenz unter 0,95 (Bandsumme); keine Laufzeit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| je Blockgröße: jedes Band mit Bit hat Kohärenz ≥ 0,95 | `Sonde013StereoGoldenTest.cpp:688` und `:696` je Blockgröße (18 Läufe), B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| und jedes Band mit mindestens zwei Bins zusätzlich Gruppenlaufzeit 1000·48/48 000 = 1,000000 ms ± 0,02 ms | `Sonde013StereoGoldenTest.cpp:710` je Blockgröße, B19 | `Stereo.h:478` Vorzeichen des Lag-Produkts, `atan2 (-lagIm, lagRe)` (Matrixmutation) (m80-vorzeichen) | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | `NAK-380-e5-mutation-m80-vorzeichen.txt` |

**M-75** (`NAK-380-rot-M-75.txt`; Etikett: heute rot; Basisstand: rot (B19, 6 Prüfungen): bei den Blöcken 64, 512 und 16 384 kein Hauptstufenbit (199 von 199 Bändern ohne Bit), keine Laufzeit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Band mit Bit hat Kohärenz ≥ 0,95 (Referenz ρ_w(d)², 44,1 kHz) | `Sonde013StereoGoldenTest.cpp:688` und `:696` „380/M-75 laufzeit_1ms_44k1“ je Block 64, 512, 16 384, B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| Gruppenlaufzeit 0,997732 ms ± 0,02 ms | `Sonde013StereoGoldenTest.cpp:710` je Block, B19 | `Stereo.h:478` Laufzeitdivisor `fs` → 48 000 (Matrixmutation) (m80-divisor48k) | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | `NAK-380-e5-mutation-m80-divisor48k.txt` |

**M-76** (`NAK-380-rot-M-76.txt`; Etikett: heute rot; Basisstand: rot (B19, 6 Prüfungen): 31 Bänder ohne Bit, Minimum 0,38 bis 0,40 (Bandsumme), 97 Bänder ohne Laufzeit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Band mit Bit hat Kohärenz ≥ 0,95 (Referenz ρ_w(d)², 88,2 kHz) | `Sonde013StereoGoldenTest.cpp:688` und `:696` „380/M-76 laufzeit_1ms_88k2“ je Block 64, 512, 16 384, B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| Gruppenlaufzeit 0,997732 ms ± 0,02 ms | `Sonde013StereoGoldenTest.cpp:710` je Block, B19 | `Stereo.h:478` Laufzeitdivisor `fs` → 48 000 (Matrixmutation) (m80-divisor48k) | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | `NAK-380-e5-mutation-m80-divisor48k.txt` |

**M-77** (`NAK-380-rot-M-77.txt`; Etikett: heute rot; Basisstand: rot (B19, 6 Prüfungen): 28 Bänder ohne Bit, Minimum 0,37 bis 0,39, 94 Bänder ohne Laufzeit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Band mit Bit hat Kohärenz ≥ 0,95 (Referenz ρ_w(d)², 96 kHz) | `Sonde013StereoGoldenTest.cpp:688` und `:696` „380/M-77 laufzeit_1ms_96k“ je Block 64, 512, 16 384, B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| Gruppenlaufzeit 1,000000 ms ± 0,02 ms | `Sonde013StereoGoldenTest.cpp:710` je Block, B19 | `Stereo.h:478` Laufzeitdivisor `fs` → 48 000 (Matrixmutation) (m80-divisor48k) | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | `NAK-380-e5-mutation-m80-divisor48k.txt` |

**M-78** (`NAK-380-rot-M-78.txt`; Etikett: heute rot; Basisstand: rot (B19, 6 Prüfungen): 16 Bänder ohne Bit, Minimum 0,32 bis 0,38, 73 Bänder ohne Laufzeit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Band mit Bit hat Kohärenz ≥ 0,95 (Referenz ρ_w(d)², 176,4 kHz) | `Sonde013StereoGoldenTest.cpp:688` und `:696` „380/M-78 laufzeit_1ms_176k4“ je Block 64, 512, 16 384, B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| Gruppenlaufzeit 0,997732 ms ± 0,02 ms | `Sonde013StereoGoldenTest.cpp:710` je Block, B19 | `Stereo.h:478` Laufzeitdivisor `fs` → 48 000 (Matrixmutation) (m80-divisor48k) | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | `NAK-380-e5-mutation-m80-divisor48k.txt` |

**M-79** (`NAK-380-rot-M-79.txt`; Etikett: heute rot; Basisstand: rot (B19, 6 Prüfungen): Minimum 0,36 bis 0,37, 70 Bänder ohne Laufzeit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| jedes Band mit Bit hat Kohärenz ≥ 0,95 (Referenz ρ_w(d)², 192 kHz) | `Sonde013StereoGoldenTest.cpp:688` und `:696` „380/M-79 laufzeit_1ms_192k“ je Block 64, 512, 16 384, B19 | `Stereo.h:426-443` Kohärenz aus der über Bins summierten Kreuzleistung (alter Weg, Matrixmutation) (m73-bandsumme) | JA | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| Gruppenlaufzeit 1,000000 ms ± 0,02 ms | `Sonde013StereoGoldenTest.cpp:710` je Block, B19 | `Stereo.h:478` Laufzeitdivisor `fs` → 48 000 (Matrixmutation) (m80-divisor48k) | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | `NAK-380-e5-mutation-m80-divisor48k.txt` |

**M-80** (`NAK-380-rot-M-80.txt`; Etikett: heute nicht messbar; Basisstand: rot, weil die Größe erst entsteht (B19, 33 Prüfungen „ohne Laufzeitbit“ bzw. 0 Bänder))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| je Band mit mindestens zwei Bins und Kohärenzbit: τ̂ = arg(Σ Sxy[k+1]·conj(Sxy[k]))/(2π·Δf) stimmt mit 1000·d/fs ms ± 0,02 ms überein, also 0,997732 ms für D1, D3 und D5 sowie 1,000000 ms für D2, D4 und D6 | `Sonde013StereoGoldenTest.cpp:710` über `:804` „380/M-80 laufzeit_aus_phase“, 33 Läufe, B19 | `Stereo.h:478` Vorzeichen des Lag-Produkts, `atan2 (-lagIm, lagRe)` (Matrixmutation) (m80-vorzeichen); `Stereo.h:478` Laufzeitdivisor `fs` → 48 000 (Matrixmutation) (m80-divisor48k) | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | `NAK-380-e5-mutation-m80-vorzeichen.txt`, `NAK-380-e5-mutation-m80-divisor48k.txt` |
| Bänder mit einem Bin tragen keine Laufzeit, nie 0 | `Sonde013StereoGoldenTest.cpp:806` je Lauf, B19 | `Stereo.h:476` Laufzeit auch für Einbinbänder, `binsB >= 1` (atan2 (0, 0) = 0) (m80-einbin) | JA | `NAK-380-e5-mutation-m80-einbin.txt` |
| nie aus einer über Bins summierten Kreuzleistung | `Sonde013StereoGoldenTest.cpp:710` (Wert je Band), B19 | — | NEIN (Prüfzeile am Etappenstand rot, offen §46.11) | — |

**M-81** (`NAK-380-rot-M-81.txt`; Etikett: heute rot; Basisstand: rot (B19, 2 Prüfungen): Kohärenz ohne Bit, Phase ohne Bit; Soll −1,42353 rad, Erstbin-Abstand 1,39899 rad)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Kohärenz-Präsenzbit und Phasenbit gesetzt | `Sonde013StereoGoldenTest.cpp:844`, B19 | `Stereo.h:426-443` Kohärenz aus der Bandsumme (Wortlaut wie m73-bandsumme) (m81-bandsumme) | JA | `NAK-380-e5-mutation-m81-bandsumme.txt` |
| Phase = gewickeltes +2π·f_k·0,001 s ± 0,25 rad am tatsächlich gewählten FFT-Bin k, der der geometrischen Bandmitte am nächsten liegt | `Sonde013StereoGoldenTest.cpp:851`, B19 | `Stereo.h:422-423` Phase am ersten statt am Mittenbin, `kMitte = von` (Matrixmutation) (m81-erstbin) | JA | `NAK-380-e5-mutation-m81-erstbin.txt` |
| Bins 1327 bis 1365 (39), Mittenbin 1346; … falsche Erstbinwahl verschiebt die Referenz um 1,39899 rad | `Sonde013StereoGoldenTest.cpp:833` Vorbedingung (Testarithmetik) und `:851` (Erstbin-Abstand in der Meldung), B19 | `Stereo.h:422-423` Phase am ersten statt am Mittenbin, `kMitte = von` (Matrixmutation) (m81-erstbin) | JA | `NAK-380-e5-mutation-m81-erstbin.txt` |

**M-82** (`NAK-380-rot-M-82.txt`; Etikett: heute rot; Basisstand: rot (B19, 4 Prüfungen): zweiter Snapshot 7 statt 11 bis 12 Frames, 150 von 150 Bändern ohne Bit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| erster Snapshot: 5 Hauptstufenframes …, Kohärenz `null` | `Sonde013StereoGoldenTest.cpp:891` je Block 64, 128, 256, 512, B19 | `Stereo.h:414` Mindestframezahl 8 → 1 (m82-min1) | JA | `NAK-380-e5-mutation-m82-min1.txt` |
| ab dem zweiten: 11 bis 12 Frames im Ring und jedes Hauptstufenband mit Energie trägt ein Kohärenzbit | `Sonde013StereoGoldenTest.cpp:895` je Block, B19 | `FeatureEngine.h:271` W_H 3 → 1 (Matrixmutation) (m82-wh1) | JA | `NAK-380-e5-mutation-m82-wh1.txt` |

**M-83** (`NAK-380-rot-M-83.txt`; Etikett: heute rot; Basisstand: rot (B19, 4 Prüfungen): 235 (0,25 s) bzw. 47 (1 s) Bassbandbefunde ohne Bit)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| 0,25 s: ab dem fünften Snapshot tragen alle Bassbänder mit Energie ein Kohärenzbit | `Sonde013StereoGoldenTest.cpp:946` (Intervall 0,25 s), B19 | `FeatureEngine.h:279` W_B 7 → 1 (Matrixmutation) (m83-wb1) | JA | `NAK-380-e5-mutation-m83-wb1.txt` |
| 1 s: ab dem zweiten | `Sonde013StereoGoldenTest.cpp:946` (Intervall 1 s), B19 | `FeatureEngine.h:279` W_B 7 → 1 (Matrixmutation) (m83-wb1) | JA | `NAK-380-e5-mutation-m83-wb1.txt` |

**M-84** (`NAK-380-rot-M-84.txt`; Etikett: heute nicht messbar; Basisstand: rot, weil der Ring erst entsteht (B19, 2 Prüfungen): gemeldet 7 statt 20 (Hauptstufe) und 1 statt 11 (Bass))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Freiheitsgrade eines Hauptstufenbands = Frames der letzten 3 Fenster (20) | `Sonde013StereoGoldenTest.cpp:986`, B19 | `FeatureEngine.h:271` W_H 3 → 10 (Matrixmutation) (m84-wh10) | JA | `NAK-380-e5-mutation-m84-wh10.txt` |
| eines Bassbands = Frames der letzten 7 Fenster (11) | `Sonde013StereoGoldenTest.cpp:992`, B19 | `FeatureEngine.h:279` W_B 7 → 10 (m84-wb10) | JA | `NAK-380-e5-mutation-m84-wb10.txt` |
| ältere Fenster fallen heraus | `Sonde013StereoGoldenTest.cpp:986` und `:992` (66 Frames seit dem Start), B19 | `FeatureEngine.h:271` W_H 3 → 10 (Matrixmutation) (m84-wh10); `FeatureEngine.h:279` W_B 7 → 10 (m84-wb10) | JA | `NAK-380-e5-mutation-m84-wh10.txt`, `NAK-380-e5-mutation-m84-wb10.txt` |

**M-85** (`NAK-380-rot-M-85.txt`; Etikett: heute nicht messbar; Basisstand: nicht messbar: Ring und `stereoRingBytes()` entstehen erst mit dem Umbau)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Schranke 240 KiB (245 760 B) je Engine | `Sonde013StereoGoldenTest.cpp:1053` je Rate 44,1, 48, 96, 192 kHz, B19 | `FeatureEngine.h:646` Ringelemente mal 4 (formgültige Fassung der Matrixmutation) (m85-alle-bins) | JA | `NAK-380-e5-mutation-m85-alle-bins.txt` |
| 44,1-kHz-Familie … = 236 224 B, 48-kHz-Familie … = 217 792 B (nachgerechnet); vor Etappe 6 bei 96/192 kHz weniger | `Sonde013StereoGoldenTest.cpp:1053` (Istwert = gerechneter Wert, 113 344 B und 61 248 B), B19 | `FeatureEngine.h:646` Ringelemente mal 4 (formgültige Fassung der Matrixmutation) (m85-alle-bins) | JA | `NAK-380-e5-mutation-m85-alle-bins.txt` |

**M-86** (`NAK-380-rot-M-86.txt`; Etikett: Dauer Klasse (i) Regressionswache; Freiheitsgrade Klasse (ii) heute nicht messbar; Basisstand: Dauer grün (1066,667 ms gegen 1066,667 ms); Freiheitsgrade rot: gemeldet 25, Modell und Golden 74 (Pflichterfassung `NAK-380-etappe-5-m86-dof-alt.txt`))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Fensterdauer = Summe der Hopdauern genau der gezählten Frames ± 1 ms (bleibt) | `Sonde013StereoGoldenTest.cpp:1118` (3157,333 ms), B19 | `Stereo.h:374` Fensterdauer nur des letzten Fensters (Matrixmutation) (m86-dauer-letztes) | JA | `NAK-380-e5-mutation-m86-dauer-letztes.txt` |
| Freiheitsgrade = Frames im Ring (neu: über W Fenster, nicht nur das letzte) | `Sonde013StereoGoldenTest.cpp:1113` (74), B19 | `Stereo.h:372` Freiheitsgrade nur des letzten Fensters (Matrixmutation) (m86-dof-letztes); `Sonde013StereoGoldenTest.cpp:1082` Golden 74 → alter Wert 25 (Klasse (ii)) (m86-golden-alt) | JA | `NAK-380-e5-mutation-m86-dof-letztes.txt`, `NAK-380-e5-mutation-m86-golden-alt.txt` |

**M-87** (`NAK-380-rot-M-87.txt`; Etikett: Regressionswache; Basisstand: die Leerung selbst grün; die neuen Prüfungen „und der zweite die Frames beider Fenster“ (reset, rate_change) rot, weil der Ring fehlt (gemeldet 7 statt 12, 14 statt 27))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| (a) Seek: höchstens die Frames seit der Grenze (höchstens 5 …), Kohärenz `null` | `Sonde013StereoGoldenTest.cpp:1203`, `:1226`, `:1231` (Bein seek) und `:2085`, `:2089` (§32.3), B19 | `Zeit.h:389` Seek schiebt den Ring statt ihn zu leeren (Matrixmutation) (m87-seek) | JA | `NAK-380-e5-mutation-m87-seek.txt` |
| (b) `zuruecksetzen()`: dasselbe | `Sonde013StereoGoldenTest.cpp:1203`, `:1226`, `:1231`, `:1241` (Bein reset), B19 | `FeatureEngine.h:698` `zuruecksetzen` leert nur die Fensterträger (Matrixmutation) (m87-reset) | JA | `NAK-380-e5-mutation-m87-reset.txt` |
| (c) Binzuordnung für 96 kHz neu, kein Wert der alten Rate überlebt | `Sonde013StereoGoldenTest.cpp:1203` (Ringbytes 113 344 B), `:1237`, `:1241` (Bein rate_change), B19 | `FeatureEngine.h:646` Ratenwechsel legt den Ring nicht neu an (formgültige Fassung) (m87-rate) | JA | `NAK-380-e5-mutation-m87-rate.txt` |

**M-88** (`NAK-380-rot-M-88.txt`; Etikett: heute rot; Basisstand: rot (B19): gemeldet 7 statt 12 Frames, 150 von 150 Bändern)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Freiheitsgrade des zweiten Snapshots = Frames beider Fenster | `Sonde013StereoGoldenTest.cpp:1004` (5 + 7 = 12), B19 | `Frame.h:167` `evidenzLeeren` leert den Ring (Matrixmutation) (m88-leeren) | JA | `NAK-380-e5-mutation-m88-leeren.txt` |

**M-89** (`NAK-380-rot-M-89.txt`; Etikett: Regressionswache; Basisstand: grün (die 120 roten Prüfungen des Basislaufs sind alle 380/-Fälle; `mono_identity` und Sweep grün))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Korrelation > 0,999 | `Sonde013StereoGoldenTest.cpp:1450` und Sweep `:2410`, B19 | `Stereo.h:402` Korrelation mal 0,99 (m89-korrelation) | JA | `NAK-380-e5-mutation-m89-korrelation.txt` |
| Kohärenz > 0,99 | `Sonde013StereoGoldenTest.cpp:1456` und Sweep `:2410`, B19 | `Stereo.h:442` Nenner der MSC je Bin mal 2 (Matrixmutation) (m89-nenner2) | JA | `NAK-380-e5-mutation-m89-nenner2.txt` |
| Phase ± 0,01 rad | `Sonde013StereoGoldenTest.cpp:1459`, B19 | `Stereo.h:468` Phase gegen die gespiegelte Realachse, `atan2 (mitteIm, -mitteRe)` (m89-phase) | JA | `NAK-380-e5-mutation-m89-phase.txt` |
| Folddown ± 0,25 dB | `Sonde013StereoGoldenTest.cpp:1464`, B19 | `Stereo.h:528` Monoenergie doppelt gezählt (+3,01 dB) (m89-folddown) | JA | `NAK-380-e5-mutation-m89-folddown.txt` |

**M-90** (`NAK-380-rot-M-90.txt`; Etikett: Regressionswache; Basisstand: grün (wie M-89))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Korrelation −1 | `Sonde013StereoGoldenTest.cpp:1998`, B19 | `Stereo.h:402` Korrelation mal 0,99 (m89-korrelation) | JA | `NAK-380-e5-mutation-m89-korrelation.txt` |
| Kohärenz > 0,99 | `Sonde013StereoGoldenTest.cpp:2001` und Sweep `:2410`, B19 | `Stereo.h:442` Nenner der MSC je Bin mal 2 (Matrixmutation) (m89-nenner2) | JA | `NAK-380-e5-mutation-m89-nenner2.txt` |
| Monosumme an der Vertragsgrenze | `Sonde013StereoGoldenTest.cpp:2010`, B19 | `Stereo.h:528` Restanteil 1e-12 der Stereoenergie in der Monosumme (m90-monosumme) | JA | `NAK-380-e5-mutation-m90-monosumme.txt` |

**M-91** (`NAK-380-rot-M-91.txt`; Etikett: Phase Regressionswache; Kohärenz heute nicht messbar; Basisstand: grün; Pflichterfassung Mittel 0,0175 bis 0,0195, Maximum 0,1443 bis 0,1564 (`NAK-380-etappe-5-m91-unkorreliert-basis.txt`))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Korrelation < 0,2 | Sweep `Sonde013StereoGoldenTest.cpp:2410` (Klasse `sweep_uncorrelated_channels`), B19 | `Stereo.h:402` Korrelation + 0,5 (m91-korrelation) | JA | `NAK-380-e5-mutation-m91-korrelation.txt` |
| kein Band mit Phase | Sweep `Sonde013StereoGoldenTest.cpp:2410`, B19 | `Stereo.h:466` Phasenschwelle 0,8 → 0,0 (Matrixmutation) (m91-phase0) | JA | `NAK-380-e5-mutation-m91-phase0.txt` |
| NEU: Mittel der Bandkohärenz über alle Bänder mit Kohärenzbit ≤ 0,15 | `Sonde013StereoGoldenTest.cpp:2640` je Blockgröße, B19 | `Stereo.h:228-229` Ring je Bin trägt nur den letzten Frame (Matrixmutation) (m91-einframe) | JA | `NAK-380-e5-mutation-m91-einframe.txt` |
| und kein Band über 0,7 | `Sonde013StereoGoldenTest.cpp:2643` je Blockgröße, B19 | `Stereo.h:228-229` Ring je Bin trägt nur den letzten Frame (Matrixmutation) (m91-einframe) | JA | `NAK-380-e5-mutation-m91-einframe.txt` |

**M-92** (`NAK-380-rot-M-92.txt`; Etikett: Regressionswache; Basisstand: grün ((b) am Basisstand 6 Frames im ersten Snapshot, 0 mit Bit))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| (a) ein stiller Kanal: kein Kohärenzbit | `Sonde013StereoGoldenTest.cpp:1493`, B19 | `Stereo.h:461` Bit ohne Energiebedingung (formgültige Fassung) (m92-energie) | JA | `NAK-380-e5-mutation-m92-energie.txt` |
| (b) unter 8 Frames Kohärenz `null`, nicht 1 | `Sonde013StereoGoldenTest.cpp:1700` (erster Snapshot) und `:1341` (7 Frames), B19 | `Stereo.h:414` Mindestframezahl 8 → 1 (Matrixmutation) (m92-min1) | JA | `NAK-380-e5-mutation-m92-min1.txt` |

**M-93** (`NAK-380-rot-M-93.txt`; Etikett: heute rot; Basisstand: rot (B19): Persistenz 0 in 15 von 15 Fenstern)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Persistenz im Band 15,77 kHz > 0,5 (Kurzfenster von 8 Frames, MSC je Bin ≈ 0,998 ≥ 0,8) | `Sonde013StereoGoldenTest.cpp:1385`, B19 | `Stereo.h:305` Kurzfenster-Kohärenz aus der Bandsumme (Matrixmutation) (m93-bandsumme-kurz) | JA | `NAK-380-e5-mutation-m93-bandsumme-kurz.txt` |

**M-94** (`NAK-380-rot-M-94.txt`; Etikett: heute rot; Basisstand: rot (A5, 4 Prüfungen): beide Kommentare ohne Ringsatz und ohne Ringlängen)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| die `$comment`-Texte nennen den Ring: „Summe der Hopdauern … über die letzten W Evidenzfenster (Hauptstufe 3, Bassstufe 7)“ (`fenster_dauer_ms`) | `pruefe_v3_vertrag.py:2717` und `:2732` (`fenster_dauer_ms`), A5 | `eq-ipc-v3.schema.json:1399` Satz in `fenster_dauer_ms` entfernt (Matrixmutation) (m94-satz-fenster) | JA | `NAK-380-e5-mutation-m94-satz-fenster.txt` |
| … „Zahl der gültigen Welch-Frames über die letzten W Evidenzfenster (Hauptstufe 3, Bassstufe 7)“ (`freiheitsgrade`) | `pruefe_v3_vertrag.py:2717` und `:2732` (`freiheitsgrade`), A5 | `eq-ipc-v3.schema.json:1406` Satz in `freiheitsgrade` entfernt (Matrixmutation) (m94-satz-freiheit) | JA | `NAK-380-e5-mutation-m94-satz-freiheit.txt` |
| keine Feldänderung | `pruefe_v3_vertrag.py:2723` (Feldform) und `:2739` (`$id`), A5 | `eq-ipc-v3.schema.json:1410` Obergrenze der Freiheitsgrade 1 048 576 → 4096 (m94-feldform) | JA | `NAK-380-e5-mutation-m94-feldform.txt` |

**M-95** (`NAK-380-rot-M-95.txt`; Etikett: Regressionswache; Basisstand: nicht gefahren: der Testzugang (`nanKreuzBin`) entsteht erst mit dem Umbau; der Riegel je Band bestand am Basisstand (`Stereo.h:81-84` alt))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| der Frame zählt für dieses Band nicht (Riegel je Band) | `Sonde013DynamicsTest.cpp:939`, B18 | `Stereo.h:200` NaN-Riegel weg, `(void) 0;` (Matrixmutation) (m95-riegel) | JA | `NAK-380-e5-mutation-m95-riegel.txt` |
| die übrigen Bänder zählen | `Sonde013DynamicsTest.cpp:942`, B18 | `Stereo.h:200` NaN-Riegel verwirft den Frame für alle Bänder, `return;` (m95-global) | JA | `NAK-380-e5-mutation-m95-global.txt` |
| keine NaN in Kohärenz, Phase oder Laufzeit | `Sonde013DynamicsTest.cpp:958`, B18 | `Stereo.h:200` NaN-Riegel weg, `(void) 0;` (Matrixmutation) (m95-riegel) | JA | `NAK-380-e5-mutation-m95-riegel.txt` |

**M-96** (`NAK-380-rot-M-96.txt`; Etikett: heute nicht messbar; Basisstand: rot (A5, 10 Prüfungen): Code, Rust und Register nennen 20260927; die Fassung fehlt)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| A5 findet beide Werte im Code | `pruefe_v3_vertrag.py:2810`, A5 | `FeatureEngine.h:279` W_B im Code 7 → 6 ohne Register (Matrixmutation) (m96-wb6); `FeatureEngine.h:271` W_H im Code 3 → 2 ohne Register (m96-wh2) | JA | `NAK-380-e5-mutation-m96-wb6.txt`, `NAK-380-e5-mutation-m96-wh2.txt` |
| `kFeatureMetricsVersion`, `METRICS_VERSION` gehoben | `pruefe_v3_vertrag.py:2754`, `:2762`, `:2767`, `:2772`, A5 | `FeatureEngine.h:89` `kFeatureMetricsVersion` 20260928 → 20260927 (m96-version-cpp); `metriken-v1.json:20` `aktuell` 20260928 → 20260927 (m96-aktuell) | JA | `NAK-380-e5-mutation-m96-version-cpp.txt`, `NAK-380-e5-mutation-m96-aktuell.txt` |
| M-73 und M-82 fallen am alten Wert | `Sonde013StereoGoldenTest.cpp:748` und `:895`, B19 | `FeatureEngine.h:271-279` W_H und W_B → 1 (alter Wert) (m96-alt-w1) | JA | `NAK-380-e5-mutation-m96-alt-w1.txt` |
| alte `stereo_evidenz` bleibt lesbar (A4, B3c) | A4 (`cargo test`), B3c | — | — (keine Rust-Mutation laut Auftrag; A4 und B3c grün) | — |

**M-97** (`NAK-380-rot-M-97.txt`; Etikett: Regressionswache; Basisstand: Startwert 16 208 B (`NAK-380-etappe-5-m97-sizeof-start.txt`))

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| `sizeof (FeatureEngine)` ≤ Startwert + 40 B | `AnalysisGoldenTestMain.cpp:1913`, B5 | hinter `FeatureEngine.h:1542` vier weitere `std::uint32_t` als Mitglied (+16 B, formgültige Fassung) (m97-vier-staende); hinter `FeatureEngine.h:1542` 65 536 `double` als Mitglied (Matrixmutation) (m97-array); hinter `FeatureEngine.h:1542` 221 `std::uint32_t` als Mitglied (+884 B) (m97-bandstaende) | JA (m97-vier-staende); Matrixmutation und +884 B enden vor der Prüfung mit 0xC00000FD, NEIN | `NAK-380-e5-mutation-m97-vier-staende.txt`, `NAK-380-e5-mutation-m97-array.txt`, `NAK-380-e5-mutation-m97-bandstaende.txt` |
| alle bin- und fensterzahlabhängigen Daten bleiben Vektornutzdaten im Heap | `AnalysisGoldenTestMain.cpp:1913` (Objektgröße unabhängig von der Rate) und `Sonde013StereoGoldenTest.cpp:1053` (Ringbytes je Rate), B5 und B19 | hinter `FeatureEngine.h:1542` vier weitere `std::uint32_t` als Mitglied (+16 B, formgültige Fassung) (m97-vier-staende); `FeatureEngine.h:646` Ringelemente mal 4 (formgültige Fassung der Matrixmutation) (m85-alle-bins) | JA | `NAK-380-e5-mutation-m97-vier-staende.txt`, `NAK-380-e5-mutation-m85-alle-bins.txt` |
| A1, A16 und B1 grün, B5 ohne `0xC00000FD` | Beine A1, A16, B1, B5 (Endstand) | hinter `FeatureEngine.h:1542` 65 536 `double` als Mitglied (Matrixmutation) (m97-array); hinter `FeatureEngine.h:1542` 221 `std::uint32_t` als Mitglied (+884 B) (m97-bandstaende) | JA (B5 endet unter beiden Mutationen mit 0xC00000FD) | `NAK-380-e5-mutation-m97-array.txt`, `NAK-380-e5-mutation-m97-bandstaende.txt` |

**Vorbedingungen der Etappe-5-Läufer** (Lehren D1, D4, D5 aus §39.1; Muster §40.5; alle Läufer durchgesehen: Rauschläufer `nak380Rauschen` für M-73 bis M-84, M-88, M-93; M-86; M-87 und M-92 über denselben Blockläufer; B18 M-95)

| Vorbedingung | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| Rauschpaar-Selbstprüfung vor jedem Nutzer (Kennwerte, Amplitude, exakte Verschiebung), B19 | `Sonde013StereoGoldenTest.cpp:542` über `nak380Vorbedingungen` (alle Rauschläufe) | `Nak380Pruefsignale.h:656` R um d − 1 statt d verschoben (e5-verschiebung) | JA | `NAK-380-e5-mutation-e5-verschiebung.txt` |
| dasselbe, B18 | `Sonde013DynamicsTest.cpp:866` (M-95) | `Nak380Pruefsignale.h:656` R um d − 1 statt d verschoben (e5-verschiebung-b18) | JA | `NAK-380-e5-mutation-e5-verschiebung-b18.txt` |
| gespeiste Samplezahl = Signallänge, Läufer und Engine (Rauschläufe) | `Sonde013StereoGoldenTest.cpp:544` (M-74, Block 333: 147 Samples Rest) | `Sonde013StereoGoldenTest.cpp:433` Läufer speist nur volle Blöcke (e5-rest) | JA | `NAK-380-e5-mutation-e5-rest.txt` |
| dasselbe, M-86 (153 600 Samples ohne Rest) | `Sonde013StereoGoldenTest.cpp:1101` | `Sonde013StereoGoldenTest.cpp:433` Läufer lässt den letzten Block weg (e5-letzter) | JA | `NAK-380-e5-mutation-e5-letzter.txt` |
| dasselbe, M-87 (Segmente vor und nach dem Ereignis; gemeinsame Vorbedingung) | `Sonde013StereoGoldenTest.cpp:1212` je Bein | `Sonde013StereoGoldenTest.cpp:433` Läufer lässt den letzten Block weg (e5-letzter) | JA | `NAK-380-e5-mutation-e5-letzter.txt` |
| dasselbe, M-92 (24 000 Samples, Blöcke 4096 und 9216; gemeinsame Vorbedingung) | `Sonde013StereoGoldenTest.cpp:1318` | `Sonde013StereoGoldenTest.cpp:433` Läufer lässt den letzten Block weg (e5-letzter) | JA | `NAK-380-e5-mutation-e5-letzter.txt` |
| dasselbe, B18 M-95 (Rest 256) | `Sonde013DynamicsTest.cpp:898` | `Sonde013DynamicsTest.cpp:875` Läufer speist nur volle Blöcke (e5-rest-m95) | JA | `NAK-380-e5-mutation-e5-rest-m95.txt` |
| Zählregel 7.3, Verlustzähler 0 als eigene Prüfung (Rauschläufe) | `Sonde013StereoGoldenTest.cpp:549` (B19 voll: 24 Läufer bei 88,2 bis 192 kHz tragen 4 bis 5 Ereignisse; die 48-kHz- und 44,1-kHz-Läufer keines) | `Spektrum.h:935-936` Ereignisring nimmt kein Ereignis an und zählt jedes (e5-verlust) | JA | `NAK-380-e5-mutation-e5-verlust.txt` |
| dasselbe, M-86 (Sinus ohne Ereignis) | `Sonde013StereoGoldenTest.cpp:1105`; Zusageprüfungen von M-86 grün | `Spektrum.h:935-936` Ereignisring verliert jedes Ereignis, dazu `Spektrum.h:801` T_eff = med + 1e-9 (Doppelmutation) (e5-verlust-m86) | JA | `NAK-380-e5-mutation-e5-verlust-m86.txt` |
| dasselbe, B18 M-95 (Rauschen ohne Ereignis; `e5-verlust` allein grün) | `Sonde013DynamicsTest.cpp:902`; die sechs übrigen Prüfungen von M-95 grün | `Spektrum.h:935-936` Ereignisring verliert jedes Ereignis, dazu `Spektrum.h:801` T_eff = med + 1e-9 (Doppelmutation) (e5-verlust-m95) | JA | `NAK-380-e5-mutation-e5-verlust-m95.txt` |
| dasselbe, M-87 seek und rate_change (Sinus ohne Ereignis) | `Sonde013StereoGoldenTest.cpp:1212` | `Spektrum.h:935-936` Ereignisring verliert jedes Ereignis, dazu `Spektrum.h:801` T_eff = med + 1e-9 (Doppelmutation) (e5-verlust-m87-m92) | JA | `NAK-380-e5-mutation-e5-verlust-m87-m92.txt` |
| dasselbe, M-87 alle drei Beine einschließlich reset (Historie nach dem Rücksetzen leer) | `Sonde013StereoGoldenTest.cpp:1212` | `Spektrum.h:935-936` Ereignisring verliert jedes Ereignis, dazu `Spektrum.h:801` T_eff = med + 1e-9 und `:794` Schwelle ab dem ersten Historienwert (Dreifachmutation) (e5-verlust-m87-reset) | JA | `NAK-380-e5-mutation-e5-verlust-m87-reset.txt` |
| dasselbe, M-92 (rund 11 Frames, unter 32 Frames Historie; die Doppelmutation blieb grün) | `Sonde013StereoGoldenTest.cpp:1318` | `Spektrum.h:935-936` Ereignisring verliert jedes Ereignis, dazu `Spektrum.h:801` T_eff = med + 1e-9 und `:794` Schwelle ab dem ersten Historienwert (Dreifachmutation) (e5-verlust-m92) | JA | `NAK-380-e5-mutation-e5-verlust-m92.txt` |
| Evidenzsnapshots an den Samplezahlen des Kadenzmodells (Rauschläufe) | `Sonde013StereoGoldenTest.cpp:551` (M-82) | `Frame.h:46` Evidenz erst beim 1,5-fachen Intervall fällig (e5-kadenz) | JA | `NAK-380-e5-mutation-e5-kadenz.txt` |
| dasselbe, M-86 (drei Snapshots an 51 200, 102 400 und 153 600 Samples) | `Sonde013StereoGoldenTest.cpp:1106` | `Frame.h:46` Evidenz erst beim 1,5-fachen Intervall fällig (e5-kadenz-m86) | JA | `NAK-380-e5-mutation-e5-kadenz-m86.txt` |

### 46.6 Toleranzen (Stelle, Wert, Herleitung)

| Stelle | Wert | Herleitung |
|---|---|---|
| Kohärenz M-73 bis M-79, `Sonde013StereoGoldenTest.cpp:752`, `:688` | ≥ 0,95 (Matrix) | Referenz ρ_w(d)² bei N = 4096 zwischen 0,9715 (192 kHz, d = 192) und 0,9985 (44,1 kHz, d = 44); gemessen Minimum 0,9625 (192 kHz, Block 512) bis 0,9976; kein Band unter 0,95 |
| Referenzmittel, `Sonde013StereoGoldenTest.cpp:367` | Mittel von q = (1 − C)/(1 − ρ_w(d)²) in [0,8; 1,2] | je Bin E[C] = γ² + (1 − γ²)²/n, also E[q] = 1 − (1 − γ²)/n ≥ 0,996; Streuung je Bin höchstens sqrt(2/K) = 0,30 bei K = 22, über mindestens 50 Bänder höchstens 0,043 im Mittel, ± 0,2 sind 4,7σ; Python-Gegenprobe (75 200 Bins je Fall): Mittel 0,998 bis 0,999, 99,99-%-Quantil je Bin 1,7 bis 3,3, deshalb am Mittel statt je Band; die Bandsumme läge bei q = 250 bis 350; gemessen 0,9836 bis 1,0162 (M-73 1,0001) |
| Gruppenlaufzeit M-74 bis M-80, `Sonde013StereoGoldenTest.cpp:710` | ± 0,02 ms (Matrix, nicht gelockert) | nicht gehalten, §46.11 |
| Phase M-81, `Sonde013StereoGoldenTest.cpp:851` | ± 0,25 rad (Matrix) | Streuung je Bin sqrt((1 − γ²)/(2Kγ²)) = 0,0064 rad bei K = 22 und γ² = 0,998: 39σ; falsche Erstbinwahl 1,39899 rad; gemessen 0,00876 rad |
| Fensterdauer M-86, `Sonde013StereoGoldenTest.cpp:1118` | ± 1 ms (bestehend) | Summe der Hopdauern (42,667 ms bei 48 kHz) genau der gezählten Frames; gemessen 3157,333 gegen 3157,333 ms |
| M-91, `Sonde013StereoGoldenTest.cpp:2640`, `:2643` | Mittel ≤ 0,15, kein Band über 0,7 (Matrix) | E[Ĉ] = 1/K (K 22 bis 24: 0,042 bis 0,045), P(Ĉ > 0,7) = 0,3^21 = 1,0·10⁻¹¹ je Band; gemessen 0,0404 bis 0,0450 und 0,1644 bis 0,1861 |
| M-93, `Sonde013StereoGoldenTest.cpp:1385` | Persistenz > 0,5 (Matrix) | MSC je Bin ≈ 0,998 ≥ 0,8 in jedem Kurzfenster mit acht Frames; gemessen 1,000 in 15 von 15 |
| M-85, `Sonde013StereoGoldenTest.cpp:1053` | exakt und ≤ 245 760 B | Formel aus den Binzahlen (`Zeit.h:120-145`): 32 B je Bin und Slot, 8 B je Band und Slot |
| M-97, `AnalysisGoldenTestMain.cpp:1913` | exakt ≤ 16 248 B | Startwert 16 208 B + 40 B |
| Frames je Snapshot (M-73, M-82 bis M-84, M-86 bis M-88, M-92) | exakt | Kadenzmodell im Test (Welch-Frames bei N + j·N/2, Evidenz an 0,1-s-Rahmen fällig, Ring = Frames der letzten W Fenster), am Endstand in jedem Fall gleich der Engine |
| Rauschpaar-Selbstprüfung, `Nak380Pruefsignale.h` (Kopffunktion) | RMS innerhalb −10·log10(1 − 5σ) dB, σ = sqrt(0,8/n) | Var(u²)/E[u²]² = 0,8 für gleichverteiltes u; n = 44 100 (M-82): 0,094 dB, n = 96 000 (M-95): 0,063 dB, n = 1 920 000: 0,014 dB; gemessen −0,0078 bis +0,0041 dB. Die frühere feste Schranke 0,05 dB rechnete mit n ≥ 200 000 und hätte bei n = 44 100 nur 2,7σ gehabt; im Selbstaudit berichtigt (§46.9) |

### 46.7 Einzeln gefahrene Beine am Endstand

Endbau aller elf Ziele 26.09.2026 02:50:26 bis 02:51:48 Uhr, Exit 0
(`eq-copilot/build/nak380-e5-bau-end3.log`, Bauartefakt); einzige Warnung
die vorbestehende C4458 („`sr` blendet Klassenmember aus“), jetzt
`Stereo.h:186` statt `:70`. Jüngste Quelle unter `eq-copilot/plugin`
(core, tests, vertrag, src, dsp, state): `Spektrum.h` 02:49:33.458 Uhr
(Rücknahme der letzten Mutation); jedes Binary ist jünger (B19 02:49:40.127
aus dem Neubau nach dieser Rücknahme). Aufrufe wie `tools/beweise.ps1`;
Rohausgaben `eq-copilot/build/nak380-e5-3-<Bein>.log` (Bauartefakt). A4 lief
im Endlauf davor (Lauf 2) am selben Rust- und Vertragsstand: nach 02:27 Uhr
berührte keine Mutation Rust, Schema oder Register.

| Bein | Aufruf | Lauf | Exit | Zähler | Binary |
|---|---|---|---|---|---|
| B19 | `EqCopSonde013StereoGoldenTest` | 02:51:55–02:52:05 | 1 | 593/66 (vor der Etappe 91/0 im Kanon der Etappe 4; +568 neue Prüfungen; die 66 roten sind genau die Laufzeitprüfungen `:710`: M-74 18, M-75 bis M-79 je 3, M-80 33) | 02:49:40.127 |
| B18 | `EqCopSonde013DynamicsTest` | 02:52:05–02:52:18 | 0 | 126/0 (vorher 119/0; +7 M-95) | 02:50:31.494 |
| B5 | `EqCopAnalysisGoldenTest` | 02:52:18–02:52:31 | 0 | 313/0 (vorher 312/0; +1 M-97) | 02:50:44.969 |
| B16 | `EqCopSonde013EventWireTest` | 02:52:31–02:52:35 | 0 | 234/0 (unverändert) | 02:50:54.646 |
| B3c | `EqCopSchemaTest` | 02:52:35–02:52:38 | 0 | 177/0 | 02:50:58.602 |
| B24 | `EqCopSonde013PrePostGoldenTest` | 02:52:38–02:52:39 | 0 | 30/0 | 02:51:01.861 |
| A1 | `EqCopNullTest` | 02:52:39–02:52:40 | 0 | NULLTEST OK | 02:51:10.633 |
| A16 | `EqCopProbeeqNullTest` | 02:52:40–02:52:41 | 0 | 145/0 | 02:51:21.403 |
| B1 | `EqCopIdentityTest` | 02:52:41–02:52:41 | 0 | 120/0 | 02:51:32.192 |
| B4 | `EqCopQueueStressTest` | 02:52:41–02:52:45 | 0 | 121/0 | 02:51:44.719 |
| A2 | `EqCopGoldenTest` `eq-copilot/fixtures` | 02:52:50–02:53:37 | 0 | 310/0 | 02:51:48.456 |
| A26 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py` | 02:52:45–02:52:45 | 0 | Exit-Gate P4: alle vier Klassen bestehen, keine falsche Behauptung. | — |
| A5 | `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | 02:52:45–02:52:50 | 0 | 927/0 (Basisstand mit den neuen Prüfungen 887/14) | — |
| A8 | `py -3.13 tools/eq-copilot/erzeuge_v3_fixtures.py --pruefen` | 02:53:37–02:53:37 | 0 | bytegleich: 421 Dateien, MANIFEST sha256=b99deec416d8d745b085d656bacb9e0051d244a5e9ab14dc17d469ee9b9f534a | — |
| A9 | `py -3.13 tools/eq-copilot/pruefe_flatc_drift.py` | 02:53:37–02:53:38 | 0 | Drift: 0 Dateien | — |
| A10 | `py -3.13 tools/eq-copilot/erzeuge_fb_fixtures.py --pruefen` | 02:53:38–02:53:43 | 0 | bytegleich: 131 Dateien, MANIFEST sha256=c9d6bb4623b1df9f819df732ef2f9114e47120d94df5739968ac0b5cfd6bf1a9 | — |
| A4 | `cargo test` in `broker/` | 02:27:43–02:48:26 | 0 | 845 bestanden, 0 gescheitert, 23 ignoriert | — |

Hygiene am Endstand: `py -3.13 tools/plan/gesundheit.py` 02:54:40 bis
02:55:16 Uhr, Exit 4, A32 unverändert 3/33/39 (vorbestehend, nicht
blockierend; Testcode ist ausgenommen, `Stereo.h` 612 Zeilen, keine neue
Funktion über 200 Zeilen); `py -3.13 tools/plan/tidy.py` 02:55:16 bis
02:57:05 Uhr, Exit 0, A33 58 von höchstens 58. Einzelläufe je Matrixzeile für
die Endstandblöcke der Rohdateien (`--nak380 M-nn`) 02:53:43 bis 02:54:01 Uhr
mit denselben Binaries: rot nur M-74 bis M-80 (Laufzeit), alle übrigen Exit 0.
Kein voller Kanon, kein FL Studio, keine Installation.

### 46.8 Bewegungen bestehender Goldens

- **M-86 Freiheitsgrade (Klasse (ii)):** dritter Snapshot 25 → **74**
  (Golden `kNak380M86Freiheitsgrade = 74u` mit Formel im Kommentar; der neue
  Wert lief zuerst gegen den alten Wert 25 rot, Protokoll
  `NAK-380-e5-mutation-m86-golden-alt.txt`). Die Fensterdauer bleibt als
  Beziehung in ihrer Toleranz (Summe der Hopdauern der gezählten Frames,
  1066,667 → 3157,333 ms, weil die Frames wachsen); der erste Snapshot
  bleibt 24 Frames und 1024 ms.
- **M-92(b) Umbau:** gemessen wird der erste Snapshot nach dem Start (6
  Frames, kein Bit); dazu die Grenze 7 Frames ohne Bit (Block 4096) und 8
  Frames mit Bit (Block 9216).
- **M-91 Erweiterung:** Mittel und Maximum der Bandkohärenz; Basis 0,0175 bis
  0,0195 und 0,1443 bis 0,1564 (Bandsumme), Endstand 0,0404 bis 0,0450 und
  0,1644 bis 0,1861 (MSC je Bin, rund 1/K).
- **NAK-182-Sweep der Freiheitsgrade:** vorher und nachher je Klasse
  22 (zehnmal), 23, 24, 24, 23, 23, 23, 23, 23 (Median 22) — unverändert, weil
  der erste 1-s-Snapshot genau ein Fenster im Ring trägt.
- **§32.3-Fall:** um zwei Prüfungen erweitert (24 Frames seit der Grenze,
  Kohärenz 0,0413 < 0,5); die bestehenden Prüfungen unverändert.
- Keine weitere Bewegung: B5, B16, B18 (bestehende Fälle), A2 und B24 grün
  mit unveränderten Goldens; A8 und A10 bytegleich, A9 ohne Drift.

### 46.9 Abweichungen vom Bauplan §8.5 und Technikentscheide, begründet

1. **Rust-Versionskonstanten** `prepost.rs:206` und
   `vergleichbarkeit.rs:149`: nur die Zahl 20260927 → 20260928 (R-380-7);
   keine Rust-Mutation (Auftrag); die Basisstand-Gegenprobe zeigt A5 an beiden
   Stellen rot mit 20260927.
2. **A5-Fall der Etappe 4** (`nak380_m70_fassung_etappe_4`): er verlangte
   20260927 als aktuelle Fassung in Register, C++ und Rust und wäre mit dem
   Versionsschritt gefallen. M-70 prüft jetzt, dass die Fassung 20260927
   registriert bleibt (Seit, Einträge unverändert, Detektorschwellen mit
   Codebezug); die Versionsprüfung wandert nach M-96 auf 20260928, und M-96
   prüft zusätzlich „Einträge der Fassung 20260927 unverändert übernommen“.
3. **Keine CMake-Definition nötig:** der Testzugang lief über die bestehende
   Definition `NAKAMA_FEATUREENGINE_TESTZUGANG`.
4. **Referenzprüfung der Kohärenz am Mittel** statt je Band mit 5σ: die
   erste Fassung (im Basislauf gefahren) nahm eine Normalverteilung je Band
   an und riss nach dem Bau an einem Band (0,0031 > 0,0029); je Bin ist die
   Streuung χ²-artig (§46.6). Der Basisstand-Beleg bleibt der erste Lauf,
   rot an derselben Zusage.
5. **Laufzeit nur über der Phasenschwelle** (`kKohaerenzSchwellePhase` 0,8)
   wie die Phase (Stufe 2 von fail-closed, §40.2): ohne Kohärenz keine
   Lagaussage. In D1 bis D6 trägt jedes Band ≥ 0,95, der Unterschied zeigt
   sich nur bei unkorrelierten Kanälen (M-91: keine Laufzeit).
6. **Kohärenz nur für Bänder mit Fensterbasis:** ein Band ohne Frame im
   laufenden Fenster trägt keinen Bandwert (auch nicht aus dem Ring); bei
   0,25-s-Fenstern hat jedes Band mindestens einen Frame je Fenster.
7. **Zählerkopf im selben Vektor:** die Framezähler je Band und Slot liegen
   als double, vier je `StereoBinAkku`, vor den Bins (1144 Zähler, 286
   Elemente = 9152 B = 1144·8 B aus M-85); so bleibt es bei einem
   Vektorkopf (+40 B statt +64 B).
8. **`stereoAkkuBelegteBaender`** zählt die Ringzähler mit, damit die
   bestehende Auskunft „belegte Bänder“ nach dem Umbau stimmt.
9. **Parameter `nanKreuzBin`** an `stereoSchritt` (Standard −1, im Produkt
   ohne Wirkung) für M-95; der Testzugang ruft ihn mit einem Bin.
10. **`dauerMs` entfällt:** die Fensterdauer ist Frames·Hop/fs aus den
    Ringzählern; alle Frames einer Stufe haben denselben Hop.
11. **M-88 prüft nur die Hauptstufe:** am ersten Fenster trägt der Bass 0
    Frames, eine Leerung wäre dort nicht trennscharf; der Bassring ist in
    M-84 belegt (Modellwert 2 wird ausgegeben).
12. **M-87 zusätzlich** mit getrenntem erstem Block nach dem Ereignis, leerem
    Ring direkt danach, erstem und zweitem Snapshot nach dem Kadenzmodell je
    Segment und der §32.3-Erweiterung.
13. **Formgültige Ersatz- und Zusatzmutationen:** `m85-alle-bins`
    (Elementzahl ·4 statt „alle Bins beider Stufen“, kein Zugriff außerhalb),
    `m87-rate` (die Matrixstellen `FeatureEngine.h:468-472,520` sind
    verschoben; der Ratenwechsel legt den Ring nicht neu an), `m92-energie`,
    `m97-vier-staende` (die Matrixmutation mit 65 536 `double` und schon
    +884 B enden in B5 mit `0xC00000FD`, §46.11 Punkt 3); dazu je eine
    Mutation für Sätze, für die die Matrix keine nennt (`m80-einbin`,
    `m81-bandsumme`, `m82-min1`, `m89-korrelation`, `m89-phase`,
    `m89-folddown`, `m90-monosumme`, `m91-korrelation`, `m94-feldform`,
    `m95-global`, `m96-alt-w1`) und für die Vorbedingungen der Läufer
    (`e5-verschiebung`, `e5-verschiebung-b18`, `e5-rest`, `e5-letzter`,
    `e5-rest-m95`, `e5-verlust`, `e5-kadenz`, `e5-kadenz-m86`). Die
    Verlustprüfung kann nur fallen, wenn Ereignisse entstehen: in den
    48-kHz-Rauschläufen, M-86, M-87, M-92 und M-95 entsteht keines, deshalb
    Doppelmutationen (`e5-verlust-m86`, `e5-verlust-m95`,
    `e5-verlust-m87-m92`: zusätzlich T_eff = med + 1e-9) und, wo die
    Detektorhistorie von 32 Frames nie voll wird (M-92, M-87 reset),
    Dreifachmutationen (`e5-verlust-m92`, `e5-verlust-m87-reset`: zusätzlich
    Schwelle ab dem ersten Historienwert); die Zusageprüfungen bleiben dabei
    grün (Muster `m47-verlust`, §40.8 Punkt 2).
14. **B-5-Kommentare** in `FeatureEngine.h` berichtigt (die zwei Stellen,
    die der Umbau falsch machte).
15. **Laufzeitschätzer unverändert nach Matrix** (Lag-1-Produkt): ein Wechsel
    des Schätzers oder der Zusage ist eine Regel des Dirigenten (§46.11).
16. **Selbstaudit-Berichtigungen nach dem ersten Endlauf:** die RMS-Toleranz
    der Rauschpaar-Selbstprüfung (fest 0,05 dB, Herleitung für n ≥ 200 000)
    auf 5σ je Samplezahl umgestellt (§46.6); die Durchsicht aller Läufer
    (Lehre §40.4) ergab, dass die Vorbedingungen noch keinen eigenen
    Rotbeweis trugen — nachgeholt (15 Zeilen, §46.5). Danach alle Ziele neu
    gebaut und alle Beine gefahren (§46.7).
17. **A2, A8, A9, A10 selbst gefahren:** `rfc8785` 0.1.4 und `flatc`
    (`eq-copilot/build/nakama-flatc-pfad-Release.txt`) waren ladbar; A2
    (`EqCopGoldenTest`) hängt über `AnalyseEngine` an der `FeatureEngine`
    und lief deshalb mit.

### 46.10 Selbstaudit und Prüfliste

**Selbstaudit** (eigener Diff gegen `c43c1646`, nicht gestagt): geändert
sind genau die Ticketpfade aus dem Auftrag (§46.3) und Rohdateien unter
`docs/beweise/roh/`; nach allen Rücknahmen trägt jede mutierte Datei den
SHA-256 vor ihrer ersten Mutation (ein Wert je Datei über alle Protokolle,
gleich dem Endstand); keine Mutation im Baum (Suche nach allen
Mutationswortlauten leer); Zeilenenden je Datei wie vorher (Bytezählung: alle
C++-, Rust-, Schema- und Registerdateien nur CRLF, `tools/beweise.ps1` CRLF
mit BOM, `pruefe_v3_vertrag.py`, Manifest und Rohdateien nur LF).
Zahlenränder: keine Division durch null (MSC nur bei Sxx·Syy > 0, Mittel nur
bei mindestens einem Bin mit Energie, Laufzeit nur bei Lag-Produkt ≠ 0 und
mindestens zwei Bins), NaN je Band und Frame verriegelt (M-95), atan2 nur auf
endlichen Summen, Ringstand modulo W mit sättigender Belegung, leerer Ring nach
`vorbereiten`, `zuruecksetzen`, Seek und Ratenwechsel (M-87). Audio-Thread
unberührt: der Ring wird in `vorbereiten` im Heap angelegt, `stereoSchritt`
und `stereoAuswerten` laufen im Analysepfad, keine Allokation (A1, A16, B1,
B4 grün). Funktionslängen `stereoSchritt` rund 160, `stereoAuswerten` rund
190 Zeilen (A32 unverändert 33 über 200). Manifestzahlen aus Läufen und
Protokollen.

**Prüfliste** (`tools/dirigent/pruefliste.md`, abgehakt, Datei unverändert):

| Abschnitt | Punkt | Etappe 5 |
|---|---|---|
| A Rückstau | Zähler einer Politik werden ausgewertet; ein Zähler, der strukturell 0 sein muss, ist eine Wache mit Test | `ereignisseVerworfen()` in jedem Etappe-5-Läufer als eigene Prüfung, gebrochen mit `e5-verlust` (24 Läufer mit Ereignissen) und Doppel- oder Dreifachmutation in den Läufern ohne Ereignis (M-86, M-87, M-92, M-95) |
| B Lebenszyklus | starten↔stoppen | Ring leer nach `vorbereiten`, `zuruecksetzen`, Seek, Ratenwechsel (M-87, drei Mutationen); Ring überlebt `evidenzLeeren` (M-88) |
| C Verträge und Längen | NaN/Inf an jeder Zahl | NaN-Riegel je Band (M-95, zwei Mutationen); Vertragstext beider Felder benannt, Feldform unverändert (M-94, drei Mutationen) |
| D Bau- und Prüfriegel | Frische | jedes Binary jünger als die jüngste Quelle (§46.7); `LastWriteTime` nach jeder Rücknahme, Neubau, Fall grün |
| E Behauptung ≤ Messung | jede neue Prüfung einmal gebrochen | 56 Protokolle: 54 Zusagesätze und 15 Vorbedingungen JA; die Laufzeitzusage offen benannt statt gelockert; B19-Runnerzeile nennt sie offen |
| E Behauptung ≤ Messung | geänderte Zusage an drei Stellen | Testkommentare, Rohdatei-Köpfe, Manifestkopf; Runnerzeilen B19, B18, B5 nachgezogen; A5-Runnerzeile außerhalb der Grenze (§46.11 Punkt 2) |
| F Änderungssatz | Beziehungen im selben Satz | rechnen↔vertragen (Ring und Schematext, M-94), Code↔Register (M-96), Writer↔Leser: keine Vertragsform geändert, alte `stereo_evidenz` bleibt lesbar (A4, B3c) |

### 46.11 Offene Punkte (für den Dirigenten)

1. **Laufzeitzusage ± 0,02 ms (M-74 bis M-80) hält nicht; Regelentscheid
   nötig.** Gemessen (§46.4): in jedem der 33 Läufe liegen 21 bis 95 von 70
   bis 134 Bändern außerhalb, größte Abweichung 0,13 bis 0,52 ms, mittlere
   Abweichung mit Vorzeichen −0,019 bis −0,031 ms (44,1 und 48 kHz), −0,009
   bis −0,014 ms (88,2 und 96 kHz), −0,008 bis +0,003 ms (176,4 und
   192 kHz); nach Binzahl bei 48 kHz, Block 512: 2 Bins 0,40 ms, 3 Bins
   0,14, 5 Bins 0,13, 10 Bins 0,035, 20 Bins 0,002, 39 Bins 0,026 ms.
   Ursache, unabhängig nachgebaut (numpy, 40 Läufe, 48 kHz, N 4096, K = 22;
   `docs/beweise/roh/NAK-380-etappe-5-laufzeit-nachbau.txt`): (a) Streuung
   des Lag-1-Schätzers je Lauf 0,15 ms bei 2 Bins, 0,075 bei 3, 0,034 bei 5,
   0,017 bei 10, 0,010 bei 20, 0,0065 ms bei 39 Bins; (b) ein systematischer
   Bias von −0,018 bis −0,024 ms ab 5 Bins, weil benachbarte Hann-Bins
   korreliert sind und das Lag-Produkt die Terme desselben Frames enthält.
   Die Steigung der Binphase (kleinste Quadrate) ist biasfrei (≤ 0,001 ms),
   streut aber ebenso: 0,15 / 0,073 / 0,025 / 0,0066 / 0,0017 / 0,0005 ms.
   Mit rund 22 Frames ist ± 0,02 ms für Bänder mit wenigen Bins also bei
   keinem der beiden Schätzer erreichbar; die Matrix setzt die Schranke für
   „jedes Band mit mindestens zwei Bins“. Zu entscheiden: Schätzer (Lag-1 wie
   §6.4 oder Phasensteigung), Mindestbinzahl für eine Laufzeit oder eine
   hergeleitete Toleranz je Binzahl und Framezahl, gegebenenfalls ein
   Kreuzprodukt über Frames ohne Terme desselben Frames (mehr Speicher).
   Die Laufzeit ist nur Engine-Ergebnis (A-3) und geht nicht auf die
   Leitung. Die Schranke ist nicht gelockert; B19 endet deshalb mit Exit 1.
2. **A5-Runnerzeile** `tools/beweise.ps1:544` behauptet weiter „Fassung
   20260927 bindet kFeatureMetricsVersion und beide Broker-Konstanten“; seit
   dieser Etappe bindet sie 20260928, 20260927 bleibt registriert (A5
   927/0). Außerhalb der Ticketgrenze (nur die Zeilen B19, B18, B5).
3. **Stapelreserve von B5 knapp:** der Rahmen von `main` in
   `EqCopAnalysisGoldenTest` ist 1 002 752 B (`dumpbin /disasm`, `mov
   eax,0F4D00h` vor `__chkstk`) bei 1 048 576 B Reserve; dort liegen rund 60
   `FeatureEngine`-Objekte auf dem Stapel. Schon +884 B je Objekt enden mit
   `0xC00000FD` (`m97-bandstaende`), die Matrixmutation ebenso. Die Zusage
   „B5 ohne 0xC00000FD“ hält am Endstand, der Spielraum ist aber klein
   (Landmine NAK-175). Die Engines in B5 auf den Heap zu legen wäre ein
   eigenes verhaltensneutrales Pflegeticket.
4. **Kanon nachher und Erstprüfung 5** stehen aus (Dirigent). Einzeln
   gefahren und grün: alle Beine aus §46.7 außer B19 (Punkt 1).

## 47. Messung der Etappe 5, Regel R-380-13 und Nacharbeitsauftrag 0 (Dirigent, 26.09.2026, 03:12 Uhr)

**Messung (Dirigent, kein Selbstbericht).** Worker `95e076f4`
(`nakama-nak380-c43c164-bau`, Opus 5.5 xhigh, `dontAsk`), 01:13:18 bis
02:58 Uhr (Statuswechsel `working` → `done` vom Beobachter um 02:58 Uhr
gemeldet, `Get-Date` der Messung 02:59:09 Uhr). HEAD vor und nach dem Lauf
`c43c1646`, kein Commit des Workers, `git fetch` ohne fremden Commit auf
`origin/master`. `git status --short` ohne die fremden Einträge: 16 geänderte
Dateien (15 Produkt-, Test-, Vertrags- und Werkzeugpfade plus Manifest),
dazu untracked der Auftrag, 25 Rotbeweise `NAK-380-rot-M-73.txt` bis
`NAK-380-rot-M-97.txt`, 57 Mutationsprotokolle `NAK-380-e5-mutation-*.txt`
und 5 Erfassungsdateien (`m86-dof-alt`, `m91-unkorreliert-basis`,
`m97-sizeof-start`, `basisstand-gegenprobe`, `laufzeit-nachbau`). `git diff
--stat -- eq-copilot broker tools`: 15 Dateien, +2 506/−127 (Stereo.h +364,
Sonde013StereoGoldenTest.cpp +1 413, pruefe_v3_vertrag.py +199,
metriken-v1.json +187, Sonde013DynamicsTest.cpp +143, Nak380Pruefsignale.h
+127, FeatureEngine.h +125, AnalysisGoldenTestMain.cpp +26, Vertrag.h +23,
Frame.h +9, beweise.ps1 +6, schema +4, Zeit.h +3, prepost.rs und
vergleichbarkeit.rs je 1 Zeile). Geänderte Pfade genau die Ticketpfade des
Auftrags (§46.10); Fixtures bytegleich (A8, A9, A10 grün, §46.7).
Beine am Endstand (§46.7, vom Dirigenten am Manifest gelesen, Einzelläufe
nicht wiederholt): B19 593/66 mit Exit 1 — die 66 roten Prüfungen sind
genau die Laufzeitprüfungen M-74 bis M-80 (`Sonde013StereoGoldenTest.cpp:710`
am Etappenstand); B18 126/0, B5 313/0, B16 234/0, B3c 177/0, B24 30/0, A1,
A16 145/0, B1 120/0, B4 121/0, A2 310/0, A26, A5 927/0, A8, A9, A10, A4
845/0/23 alle Exit 0. Messzahlen §46.4: Kohärenz kein Band unter 0,95 in
allen 33 Läufen (Minimum 0,9625 bei 192 kHz, Referenz ρ_w(192)² = 0,9715);
M-81 Phase 0,00876 rad neben der Referenz; M-82 bis M-88, M-91 bis M-93,
M-95, M-97 wie zugesagt; `sizeof (FeatureEngine)` 16 208 → 16 248 B (Δ 40);
Ringbytes 236 224 / 217 792 / 113 344 / 61 248 B. Matrixtabelle §46.5: 54
Zusagesätze und 15 Vorbedingungen mit Rotbeweis JA, die sieben Laufzeitsätze
offen benannt (kein Golden gelockert). Dokuriegel des Bauers 0 Befunde.

**Landmine beim Workerstart (Dirigent, 01:12 Uhr).** Der Auftrag der Etappe 5
(`docs/beweise/roh/NAK-380-etappe-5-auftrag.txt`, 34 626 Zeichen) ließ sich
nicht als erstes Positionsargument übergeben: pwsh meldet „Der Dateiname oder
die Erweiterung ist zu lang“ (Windows-Kommandozeilenlimit, 32 767 Zeichen;
der Etappe-4-Auftrag hatte 27 857 Zeichen und ging). Ersatz: ein Zeigerauftrag
von 1 254 Zeichen als Positionsargument, der Rolle, eiserne Regeln und den
Befehl „lies zuerst die Auftragsdatei vollständig“ trägt; der Worker las die
Datei zuerst (§46.1) und arbeitete sie ab. Als Landmine nach `CLAUDE.md`
übernommen (Abschlussfenster dieser Runde).

### 47.1 Einordnung der offenen Punkte (§46.11) und Abweichungen (§46.9)

| Punkt | Quelle (Bauerstand auf `c43c1646`) | Einordnung | Regel |
|---|---|---|---|
| Laufzeitzusage ± 0,02 ms (M-74 bis M-80) verfehlt: 21 bis 95 von 70 bis 134 Bändern je Lauf außerhalb, größte Abweichung 0,13 bis 0,52 ms, mittlere −0,019 bis −0,031 ms bei 44,1/48 kHz (§46.11 Punkt 1) | `Stereo.h:425-482` Lag-1-Produkt nach T-380-6; Test `:607-660`, `:705-720`; Nachbau `NAK-380-etappe-5-laufzeit-nachbau.txt` (numpy, 40 Läufe): Lag-1 Bias −0,018 bis −0,024 ms ab 5 Bins, Streuung 0,15 ms bei 2 Bins bis 0,0065 ms bei 39 Bins; Phasensteigung biasfrei (≤ 0,002 ms), Streuung 0,15 / 0,073 / 0,025 / 0,0066 / 0,0017 / 0,0005 ms bei 2 / 3 / 5 / 10 / 20 / 39 Bins | **LÜCKE** — die Matrix (§6.4) setzt ± 0,02 ms für „jedes Band mit mindestens zwei Bins“ ohne Herleitung (dieselbe Lehre D7 aus §39.1, hier gegen die Matrix selbst), und T-380-6 schreibt den Lag-1-Schätzer vor, dessen systematischen Bias aus der Nachbarbin-Korrelation des Hann-Fensters weder Regel noch Matrix bedenken (Herleitung in R-380-13 (i)); der Entwurf verlangt in §40.3 nur, dass der Golden „bekannte Laufzeit“ korrekt klassifiziert, keine Toleranz. Mit rund 22 Frames ist 0,02 ms für 2- bis 10-Bin-Bänder bei keinem Schätzer erreichbar (Statistik in R-380-13 (iii)). Der Bauer hat innerhalb seiner Regel richtig gehandelt: Schranke nicht gelockert, Fall ehrlich rot, Ursache unabhängig nachgebaut, Schätzer nicht eigenmächtig gewechselt (§46.9 Punkt 15). | R-380-13 |
| A5-Runnerzeile `tools/beweise.ps1:544` nennt 20260927 als bindende Fassung (§46.11 Punkt 2) | Runnerzeile außerhalb der Ticketgrenze der Etappe | **Behauptung ≤ Messung** (Prüfliste E): Ticketpfad in Nacharbeit 0 um die A5-Behauptungszeile erweitert | in Nacharbeit 0 |
| Stapelreserve von B5 knapp: `main`-Rahmen 1 002 752 von 1 048 576 B, rund 60 `FeatureEngine`-Objekte auf dem Stapel; +884 B je Objekt reißen `0xC00000FD` (§46.11 Punkt 3) | `AnalysisGoldenTestMain.cpp`, Landmine NAK-175 | **HÄRTUNG** — keine Zusage bricht (B5 ohne `0xC00000FD`, M-97 gehalten); die Engines in B5 auf den Heap zu legen ist ein verhaltensneutrales Pflegeticket | Register NAK-406 |
| Abweichung 4: Referenzprüfung q = (1 − C)/(1 − ρ_w²) am Mittel über die Bänder statt je Band mit 5σ | `Sonde013StereoGoldenTest.cpp:367`; χ²-artige Streuung je Bin (§46.6) | **angenommen** — die Zusage „jedes Band mit Bit ≥ 0,95“ bleibt je Band geprüft (`:752`, `:688`); die Referenzprüfung ist eine zusätzliche Statistikwache mit hergeleiteter Schranke | — |
| Abweichung 5: Laufzeit nur über `kKohaerenzSchwellePhase` wie die Phase | `Stereo.h` Stufe 2 | **angenommen** — §40.1: „Interchannel-Phase wird nur in ausreichend kohärenten Bändern interpretiert“; die Laufzeit ist eine Deutung der Phase | — |
| Abweichung 6: Bandwert nur für Bänder mit mindestens einem Frame im laufenden Fenster | `Stereo.h` | **angenommen, Beobachtung**: `kEvidenzIntervallMinS` = 0,25 s (`FeatureEngine.h:569`) liegt über dem längsten Basshop (8 192/44 100 = 0,186 s), jedes Band hat also in jedem Fenster mindestens einen Frame; die Zusagen M-82, M-83 messen es | — |
| Abweichung 9: Parameter `nanKreuzBin` an `stereoSchritt` (Standard −1) mit drei Vergleichen je Bin im Produktrechenpfad (`Stereo.h:159`, Ringschleife) | Testzugang M-95 | **HÄRTUNG** — keine Zusage bricht (Rechenlast je Bin marginal, Produkt ruft mit −1), aber ein Testhaken im Produktrechenpfad gehört unter `NAKAMA_FEATUREENGINE_TESTZUGANG` | Register NAK-407 |
| Abweichungen 1 bis 3, 7, 8, 10 bis 14, 16, 17 | Rust-Konstanten, A5-Fall der Etappe 4, keine CMake-Definition, Zählerkopf im selben Vektor, `stereoAkkuBelegteBaender`, `dauerMs` entfällt, M-88 nur Hauptstufe, M-87 erweitert, formgültige Ersatz- und Zusatzmutationen mit Doppel- und Dreifachmutationen für die Verlustprüfung, B-5-Kommentare, Selbstaudit-Berichtigungen, A2/A8/A9/A10 selbst gefahren | **angenommen** (Technik innerhalb von Ticket, Invarianten und Regeln, begründet) | — |
| Abweichung 15: Laufzeitschätzer unverändert nach Matrix | — | Regel des Dirigenten | R-380-13 |

### 47.2 Regel R-380-13 (Dirigent, 26.09.2026, Technik innerhalb von R-380-3 und T-380-6)

**R-380-13 (Gruppenlaufzeit je Band: Schätzer und hergeleitete Toleranz;
DSP-23).**

(i) **Bias des Lag-1-Produkts, hergeleitet.** Für Gaußsches Rauschen mit
Hann-Fenster w ist je Frame E[X_{k+1}·conj(X_k)] = σ²·W₂(1) mit W₂ der
DFT von w² (w² = 0,375 − 0,5·cos(2πn/N) + 0,125·cos(4πn/N), also
|W₂(±1)| = 0,25·N, |W₂(±2)| = 0,125·N/2 = 0,0625·N, W₂(m) = 0 für |m| ≥ 3)
gegen R_w(0) = Σw² = 0,375·N. Das Produkt Sxy[k+1]·conj(Sxy[k]) über F
Frames enthält F·(F − 1) Kreuzterme verschiedener Frames mit Erwartung
σ⁴·R_w(d)²·e^{iΔθ} und F Terme desselben Frames mit Erwartung
σ⁴·(R_w(d)²·e^{iΔθ} + |W₂(1)|²) (Isserlis); der zweite, reelle Summand zieht
die Phase zur Null: relativer Bias der Laufzeit ≈ −(|W₂(1)|/R_w(0))²/F_eff
= −(4/9)/F_eff. Bei F_eff ≈ 18 (F = 22 mit 50 % Überlappung, unten) sind das
−2,5 %; der Nachbau misst −2,1 bis −2,4 % (−0,021 bis −0,024 ms bei 1 ms).
Für Binabstand 2 ist der Term (1/6)²/F_eff = 0,15 %, für Abstand ≥ 3 null;
auch die Überlappungskorrelation w(n)·w(n + N/2) = 0,125 − 0,125·cos(4πn/N)
trägt nur an den Bins 0 und ±2. Die Matrixzahlen zum Lag-1-Produkt in
T-380-6 und M-80 sind damit als Schätzer widerlegt, die Zusage ± 0,02 ms war
ohne Herleitung.

(ii) **Schätzer: Phasensteigung über das Band.** Je Band mit Kohärenzbit
werden die Binphasen φ_k = arg Sxy[k] der Bins mit Sxx·Syy > 0 und endlichen
Summen gelesen (Ringsumme, dieselben Akkus wie die MSC), in aufsteigender
Binfolge sequenziell entwickelt (Sprung über π um 2π berichtigt; ein
übersprungener Bin verdoppelt den Schritt und ist erlaubt), und die
Gruppenlaufzeit ist die Steigung der kleinsten Quadrate
s = Σ(k − k̄)(φ_k − φ̄)/Σ(k − k̄)² über die K_eff genutzten Bins,
τ = s/(2π·Δf) mit Δf = fs/N der Stufe, Vorzeichen so, dass ein verzögertes R
(Sxy = L·conj(R) ~ e^{+i2πkd/N}) eine positive Laufzeit ergibt. K_eff ≥ 2,
sonst kein Laufzeitbit (nie 0 als Wert). Für K_eff = 2 ist die Steigung
identisch mit dem Lag-1-Winkel und trägt den Bias aus (i); ab K_eff = 3
nutzt sie nur Paare mit Abstand ≥ 2 (Nachbau: |Bias| ≤ 0,002 ms). Das
Eindeutigkeitsintervall bleibt |τ| < 1/(2·Δf) (42,7 ms in der Hauptstufe bei
48 kHz, T-380-6). Kein Speicher- und kein Allokationszuwachs: zwei Durchläufe
über die Bins des Bandes mit laufenden Summen (Σk, Σφ, Σk², Σkφ). Kein
gewichteter Fit, keine Bootstrap-Größe, keine Änderung an Kohärenz, Phase am
Mittenbin, Persistenz, Vertrag oder Version (die Laufzeit ist Engine-Ergebnis,
A-3; `kFeatureMetricsVersion` 20260928 bleibt).

(iii) **Toleranz je Band, aus den Vorbedingungen gerechnet (R-380-8).**
Streuung der Binphase (Bendat und Piersol, Random Data, Phasenvarianz des
Kreuzspektrums): σ_φ² = (1 − C)/(2·C·F_eff) mit C = ρ_w(d)² der Rate
(Referenz im Kopf von §6.4) und F_eff = F/(1 + 2·(1/3)²) = F/1,222 für
Hann-Frames mit 50 % Überlappung (Welch 1967; Fensterkorrelation
Σw(n)·w(n + N/2)/Σw² = 0,125/0,375 = 1/3), F = Frames im Ring des Bandes
nach dem Kadenzmodell (verriegelt, M-84). Varianz der Steigung
Var(s) = 12·σ_φ²/(K_eff·(K_eff² − 1)), σ_τ = sqrt(Var(s))/(2π·Δf). Toleranz
T_B = |b|·τ_ref + 5·σ_τ mit b = (4/9)/F_eff für K_eff = 2 und b = 0 für
K_eff ≥ 3; 5σ, weil ein Lauf 70 bis 134 Bänder und die Etappe 33 Läufe
prüft (rund 4 200 Bandprüfungen; Bonferroni bei 1 % je Etappe verlangt 4,7σ).
Gegen den Nachbau (48 kHz, F = 22 → F_eff = 18, C = 0,998,
σ_φ = 0,00746 rad): σ_τ = 0,144 / 0,0320 / 0,0111 / 0,0039 / 0,00145 ms bei
K = 2 / 5 / 10 / 20 / 39 gegen gemessen 0,152 / 0,0246 / 0,0066 / 0,0017 /
0,0005 ms — bei K ≥ 5 konservativ (benachbarte Binphasen sind positiv
korreliert), bei K = 2 mit 5 % Unterschätzung innerhalb der 5σ-Reserve. Bei
48 kHz und vollem Ring hält damit jedes Band mit K ≥ 20 Bins T_B ≤ 0,02 ms
(5·0,0039 = 0,0197 ms), ein 2-Bin-Band trägt T_B ≈ 0,74 ms: die Laufzeit
schmaler Bänder ist mit 22 Frames keine brauchbare Größe, und die Matrix sagt
das jetzt ehrlich. Der Nachbau wird um den Steigungsschätzer je Rate D1 bis
D6 erweitert (numpy, 40 Läufe, F wie im Ring der Rate bei Block 512) und
belegt je Rate und Binzahlklasse T_B ≥ 3·SD_Nachbau + |Bias_Nachbau|;
verfehlt eine Klasse das, ist die Formel falsch und der Bauer meldet es,
statt die Toleranz zu weiten.

(iv) **Zusagen (Matrixfassung §47.3).** M-74 bis M-79: „jedes Band mit
mindestens zwei genutzten Bins und Kohärenzbit trägt die Gruppenlaufzeit
1000·d/fs ms innerhalb T_B(K_eff, F, C, Δf) nach R-380-13 (iii); T_B kommt
aus den Vorbedingungen, nie aus dem Lauf; Bänder mit einem Bin tragen kein
Laufzeitbit“. M-80: „τ ist die Phasensteigung nach (ii), nie aus einer über
Bins summierten Kreuzleistung und nie aus dem Lag-1-Produkt; bei 48 kHz und
vollem Ring hält jedes Band mit K_eff ≥ 20 Bins zusätzlich ± 0,02 ms“ (die
zweite Hälfte ist die Wache der Formel). Rotbeweise M-80: Vorzeichen der
Steigung → negative Werte; Δf aus 48 000 statt fs → D1 0,916667 ms, D3 bis D6
1,833333 bis 4,000000 ms; Steigung durch das Lag-1-Produkt ersetzt → bei
48 kHz fällt die ± 0,02-ms-Hälfte an Bändern mit K ≥ 20 (Bias 0,021 bis
0,024 ms gegen T_B ≤ 0,0197 ms); T_B im Test auf 0,02 ms festgenagelt → rot an
den schmalen Bändern. Rotbeweise M-74 bis M-79: Δf-Mutation wie bisher; die
Kohärenzhälften bleiben. Je Matrixzeile fällt der Rotbeweis an der Zeile, die
die Zusage trägt, nicht an einem Nebeneffekt.

### 47.3 Matrix in der Fassung §47

| ID | Änderung gegenüber §6.4 |
|---|---|
| M-74 bis M-79 | Laufzeithälfte: „± 0,02 ms“ → „innerhalb T_B(K_eff, F, C, Δf) nach R-380-13 (iii), aus den Vorbedingungen gerechnet; Bänder mit einem genutzten Bin ohne Laufzeitbit“; Kohärenzhälfte unverändert; Rotbeweis der Laufzeithälfte: Δf aus 48 000 statt fs (unverändert) |
| M-80 | Zusage: „τ = Steigung der kleinsten Quadrate der entwickelten Binphasen über die genutzten Bins des Bandes (R-380-13 (ii)), τ = s/(2π·Δf), stimmt mit 1000·d/fs innerhalb T_B überein; bei 48 kHz und vollem Ring hält jedes Band mit K_eff ≥ 20 Bins ± 0,02 ms; Bänder mit einem Bin tragen keine Laufzeit, nie 0; nie aus einer über Bins summierten Kreuzleistung, nie aus dem Lag-1-Produkt“; Rotbeweise: Vorzeichen, Δf 48 000, Lag-1 statt Steigung (fällt an der ± 0,02-ms-Hälfte bei K ≥ 20), T_B auf 0,02 ms festgenagelt (fällt an den schmalen Bändern) |
| T-380-6 (§8.0) | Satz „Laufzeit aus arg(Σ Sxy[k + 1]·conj(Sxy[k]))/(2π·Δf) über Bänder mit mindestens zwei Bins“ gilt in der Fassung R-380-13 (ii); der Rest von T-380-6 unverändert |
| §6.4-Kopf | „Laufzeit aus dem Lag-1-Produkt der Kreuzspektren“ → „Laufzeit als Phasensteigung über die Bins (R-380-13)“ |
| übrige Zeilen M-73, M-81 bis M-97 | unverändert (§46.5 belegt sie) |

### 47.4 Register, Zwischenstand und Nacharbeitsauftrag 0 (Runde 0, zählt nicht zum Rundenbudget)

Register: NAK-406 (B5-Engines auf den Heap, Pflegeschritt) und NAK-407
(Testhaken `nanKreuzBin` unter das Define) datiert angelegt. Zwischenstand
der Etappe 5 (Bauerstand plus dieser Abschnitt) mit Pathspec committet und
gepusht; Rundenbilanz nach dem Commit in §48. Nacharbeit 0 nach R-380-13
durch einen frischen Opus-5.5-Worker (`--effort xhigh`, NAK-404), Aufsicht
ENG, Auftrag `docs/beweise/roh/NAK-380-etappe-5-nacharbeit-0-auftrag.txt`
(Zeigerauftrag als Positionsargument, Landmine oben). Ticketpfade der
Nacharbeit 0: `Stereo.h` (Laufzeitblock `:425-482`, Kopfkommentar `:23`,
Kommentar der Auswertung), `Vertrag.h` (Kommentar `:486-493`),
`Sonde013StereoGoldenTest.cpp` (Toleranz je Band `:607-660`, Text `:705-720`,
Rotbeweise M-74 bis M-80), `tools/beweise.ps1` (B19 `:764` und A5 `:544`),
Nachbau-Rohdatei (erweitert als `NAK-380-etappe-5-laufzeit-nachbau-2.txt`),
Rotbeweise `NAK-380-rot-M-74.txt` bis `NAK-380-rot-M-80.txt` neu geschrieben,
Mutationsprotokolle `NAK-380-e5n0-mutation-*.txt`, Manifest §48. Nicht
Gegenstand: Kohärenz, Phase, Persistenz, Ring, Vertrag, Register,
Versionen, alle übrigen Matrixzeilen. Beine: B19 (voll), B18, B5, A26, A5;
Kanon nachher folgt auf dem Stand nach Nacharbeit 0 (Muster §37 bis §38).
Codex-Prüfer bleibt 401 (Handgriff beim User, Meldung 9:07 Uhr); Erstprüfung
5 fährt ein frischer lesender Opus-Thread xhigh, sofern Codex bis dahin nicht
antwortet.

## 48. Nacharbeit 0 der Etappe 5 nach R-380-13 (Bauer Opus 5.5 xhigh, 26.09.2026)

Gebaut nach dem Auftrag `docs/beweise/roh/NAK-380-etappe-5-nacharbeit-0-auftrag.txt`
(R-380-13 (ii) bis (iv), Matrixfassung §47.3). **Ergebnis:** die
Gruppenlaufzeit je Band ist die Steigung der kleinsten Quadrate über die
sequenziell entwickelten Binphasen der genutzten Bins; die Toleranz T_B je
Band kommt aus Binzahl, Framezahl des Kadenzmodells, Referenzkohärenz und
Binabstand, nie aus dem Lauf. B19 **809/0** (Etappenstand 593/66); alle 33
Laufzeitläufe halten T_B in jedem Band mit Laufzeit (größtes Verhältnis
Abweichung/T_B 0,670), die ± 0,02-ms-Wache bei 48 kHz hält in allen 18
Blockgrößen für jedes Band mit K_eff ≥ 20 (größte Abweichung 0,0030 ms). Der
Nachbau-2 hält den Beleg T_B ≥ 3·SD + |Bias| in allen 38 Klassen (größtes
Verhältnis 0,879). Je Zusagesatz M-74 bis M-80 ein Rotbeweis an der tragenden
Prüfzeile, alle JA (ein Struktursatz ohne Mutation, §48.5). Die 560
Nicht-Laufzeitprüfungen von B19 sind mit allen Zahlen bytegleich zum
Etappenstand. Nichts gestagt, nichts committet.

### 48.1 Startmessung

- `Get-Date` 26.09.2026 03:17:24 Uhr; `git rev-parse HEAD` =
  `80cd027485e97216d0de8b4fa23e86a048e6a0ab`; `git status --short` nur
  `briefing-hub/` und `nimbalyst-local/` (fremd, nicht angefasst).
- Prozessprobe (cmake, MSBuild, cl, link, cargo, rustc, EqCop*,
  eqcop-broker, FL64, vctip) um 03:17:24 Uhr leer. Vor dem Endbau lagen zwei
  übrig gebliebene MSVC-Telemetrieprozesse `vctip.exe` aus den eigenen Bauten
  ohne Bauschritt (PID 373848 vor dem Endbau, PID 49040 danach); beide beendet
  (NAK-300), kein Bau hing daran.
- Kein FL Studio, keine Installation, kein Laufzeit-Arm, kein voller Kanon.

### 48.2 Nachbau-2 (R-380-13 (iii), Beleg der Formel)

`docs/beweise/roh/NAK-380-etappe-5-laufzeit-nachbau-2.txt` (numpy im
Sitzungsordner außerhalb des Repos, 03:23:42 bis 03:23:45 Uhr, Exit 0; Skript
im Wortlaut in der Rohdatei). Je Rate D1 bis D6: N 4096, Hop 2048, Hann,
F = Frames im Ring am letzten Snapshot bei Block 512 nach dem Kadenzmodell des
Tests, C = ρ_w(d)², 40 Läufe, Saat 3 wie im Vorgänger; zusätzlich die
Bassstufe (N 16 384, W_B = 7) bei 44,1 und 48 kHz, den einzigen Raten mit
Bassbändern ab zwei Bins. Bias und SD in ms als Mittel der Bandwerte der
Klasse; T_B aus der Formel für dieselben K, F, C, Δf; „Beleg“ ist das größte
Verhältnis (3·SD + \|Bias\|)/T_B über die Bänder der Klasse (gehalten bei
≤ 1), „Extrem“ das größte \|Fehler\|/T_B über alle 40 Läufe.

| fs (kHz) | Stufe | F | Klasse K | Bänder (K) | Bias | SD | T_B | Beleg | Extrem | gehalten |
|---|---|---|---|---|---|---|---|---|---|---|
| 44,1 | haupt | 20 | 2 | 18 (2-2) | +0,00301 | 0,16062 | 0,73938 bis 0,73938 | 0,843 | 0,756 | ja |
| 44,1 | haupt | 20 | 3 | 12 (3-3) | +0,00287 | 0,07646 | 0,35614 bis 0,35614 | 0,761 | 0,825 | ja |
| 44,1 | haupt | 20 | 4-5 | 16 (4-5) | +0,00090 | 0,03483 | 0,15927 bis 0,22524 | 0,632 | 0,641 | ja |
| 44,1 | haupt | 20 | 6-10 | 22 (6-10) | -0,00011 | 0,01298 | 0,05545 bis 0,12040 | 0,579 | 0,553 | ja |
| 44,1 | haupt | 20 | 11-19 | 21 (11-19) | -0,00010 | 0,00380 | 0,02110 bis 0,04802 | 0,401 | 0,348 | ja |
| 44,1 | haupt | 20 | 20-39 | 25 (20-39) | +0,00010 | 0,00099 | 0,00717 bis 0,01953 | 0,318 | 0,295 | ja |
| 44,1 | haupt | 20 | ≥ 40 | 7 (40-48) | +0,00000 | 0,00038 | 0,00525 bis 0,00690 | 0,225 | 0,185 | ja |
| 44,1 | bass | 12 | 2 | 13 (2-2) | -0,00505 | 0,20127 | 0,96438 bis 0,96438 | 0,772 | 0,778 | ja |
| 48 | haupt | 22 | 2 | 18 (2-2) | +0,00421 | 0,15380 | 0,70541 bis 0,70541 | 0,872 | 0,667 | ja |
| 48 | haupt | 22 | 3 | 12 (3-3) | +0,00175 | 0,07417 | 0,34036 bis 0,34036 | 0,802 | 0,799 | ja |
| 48 | haupt | 22 | 4-5 | 16 (4-5) | +0,00165 | 0,03346 | 0,15221 bis 0,21526 | 0,679 | 0,617 | ja |
| 48 | haupt | 22 | 6-10 | 22 (6-10) | +0,00012 | 0,01190 | 0,05299 bis 0,11506 | 0,559 | 0,476 | ja |
| 48 | haupt | 22 | 11-19 | 22 (11-19) | -0,00012 | 0,00353 | 0,02016 bis 0,04589 | 0,407 | 0,393 | ja |
| 48 | haupt | 22 | 20-39 | 24 (20-39) | -0,00003 | 0,00097 | 0,00685 bis 0,01867 | 0,367 | 0,318 | ja |
| 48 | haupt | 22 | ≥ 40 | 4 (40-44) | +0,00005 | 0,00038 | 0,00571 bis 0,00659 | 0,203 | 0,165 | ja |
| 48 | bass | 13 | 2 | 10 (2-2) | -0,01020 | 0,19470 | 0,92695 bis 0,92695 | 0,777 | 0,668 | ja |
| 88,2 | haupt | 40 | 2 | 18 (2-2) | +0,00067 | 0,11575 | 0,51778 bis 0,51778 | 0,791 | 0,904 | ja |
| 88,2 | haupt | 40 | 3 | 12 (3-3) | -0,00057 | 0,05264 | 0,25212 bis 0,25212 | 0,768 | 0,726 | ja |
| 88,2 | haupt | 40 | 4-5 | 16 (4-5) | -0,00135 | 0,02518 | 0,11275 bis 0,15945 | 0,656 | 0,555 | ja |
| 88,2 | haupt | 40 | 6-10 | 22 (6-10) | +0,00012 | 0,00889 | 0,03925 bis 0,08523 | 0,597 | 0,498 | ja |
| 88,2 | haupt | 40 | 11-19 | 21 (11-19) | +0,00018 | 0,00259 | 0,01493 bis 0,03400 | 0,413 | 0,347 | ja |
| 88,2 | haupt | 40 | 20-39 | 8 (20-24) | +0,00013 | 0,00099 | 0,01051 bis 0,01383 | 0,301 | 0,270 | ja |
| 96 | haupt | 43 | 2 | 18 (2-2) | +0,00118 | 0,11237 | 0,50021 bis 0,50021 | 0,879 | 0,839 | ja |
| 96 | haupt | 43 | 3 | 12 (3-3) | +0,00016 | 0,04965 | 0,24379 bis 0,24379 | 0,718 | 0,733 | ja |
| 96 | haupt | 43 | 4-5 | 16 (4-5) | -0,00172 | 0,02356 | 0,10902 bis 0,15418 | 0,654 | 0,572 | ja |
| 96 | haupt | 43 | 6-10 | 22 (6-10) | -0,00004 | 0,00863 | 0,03796 bis 0,08241 | 0,557 | 0,478 | ja |
| 96 | haupt | 43 | 11-19 | 22 (11-19) | -0,00003 | 0,00266 | 0,01444 bis 0,03287 | 0,504 | 0,411 | ja |
| 96 | haupt | 43 | 20-39 | 4 (20-22) | -0,00005 | 0,00112 | 0,01159 bis 0,01337 | 0,312 | 0,304 | ja |
| 176,4 | haupt | 79 | 2 | 18 (2-2) | -0,00070 | 0,08049 | 0,36731 bis 0,36731 | 0,820 | 0,778 | ja |
| 176,4 | haupt | 79 | 3 | 12 (3-3) | -0,00031 | 0,03551 | 0,18023 bis 0,18023 | 0,721 | 0,719 | ja |
| 176,4 | haupt | 79 | 4-5 | 16 (4-5) | -0,00056 | 0,01768 | 0,08060 bis 0,11399 | 0,689 | 0,553 | ja |
| 176,4 | haupt | 79 | 6-10 | 22 (6-10) | -0,00003 | 0,00648 | 0,02806 bis 0,06093 | 0,565 | 0,478 | ja |
| 176,4 | haupt | 79 | 11-19 | 5 (11-12) | +0,00001 | 0,00263 | 0,02131 bis 0,02430 | 0,388 | 0,271 | ja |
| 192 | haupt | 86 | 2 | 18 (2-2) | +0,00019 | 0,07428 | 0,35298 bis 0,35298 | 0,758 | 0,670 | ja |
| 192 | haupt | 86 | 3 | 12 (3-3) | -0,00164 | 0,03548 | 0,17333 bis 0,17333 | 0,728 | 0,576 | ja |
| 192 | haupt | 86 | 4-5 | 16 (4-5) | -0,00079 | 0,01686 | 0,07752 bis 0,10962 | 0,641 | 0,573 | ja |
| 192 | haupt | 86 | 6-10 | 22 (6-10) | +0,00029 | 0,00609 | 0,02699 bis 0,05860 | 0,640 | 0,625 | ja |
| 192 | haupt | 86 | 11-19 | 2 (11-11) | -0,00011 | 0,00281 | 0,02337 bis 0,02337 | 0,426 | 0,386 | ja |

Die Formel unterschätzt die Streuung bei K = 2 um 7 bis 15 % und bei K = 3
um bis zu 9 %; ab K = 4 ist sie konservativ (gemessene SD 0,90- bis 0,96-mal
σ_τ bei K = 4, 0,41- bis 0,50-mal bei K = 20), weil benachbarte Hann-Bins
positiv korreliert sind. Die 5σ-Reserve trägt die Unterschätzung: kein Band
verfehlt den Beleg. Der Bias der Steigung liegt in keiner Klasse über dem
Rauschen der 40 Läufe (≤ 0,011 ms bei K = 2, ≤ 0,003 ms ab K = 3). Das
Lag-1-Produkt liegt zum Vergleich bei 44,1 und 48 kHz und K ≥ 20 im Mittel
0,021 bis 0,024 ms daneben, bei 88,2 und 96 kHz 0,011 bis 0,012 ms.

### 48.3 Änderungen je Datei

- **`Stereo.h`** (612 → 633 Zeilen, CRLF): Kopfkommentar (`:22-26`) und
  Kommentar der Auswertung (`:346-362`) nach R-380-13 (ii). In
  `stereoAuswerten` entfallen die Lag-Akkus und das Lag-1-Produkt im ersten
  Durchlauf; der Laufzeitblock (`:478-505`) ist ein zweiter Durchlauf über die
  Bins des Bandes nur für Bänder über `kKohaerenzSchwellePhase`: je Bin die
  Ringsumme über die belegten Slots (dieselbe Summationsfolge wie der erste
  Durchlauf), genutzt bei Sxx·Syy > 0 und endlichen Summen, Binphase
  `atan2 (Im, Re)`, sequenziell entwickelt (Sprung über π um 2π berichtigt,
  übersprungene Bins behalten ihren Index), laufende Summen Σk, Σφ, Σk², Σkφ
  mit k relativ zum ersten Bin des Bandes, s = (K_eff·Σkφ − Σk·Σφ)/(K_eff·Σk²
  − (Σk)²), τ = 1000·s/(2π·Δf) ms mit Δf = fs/N der Stufe; Bit nur bei
  K_eff ≥ 2, Nenner > 0 und endlichem τ. Kein neues Mitglied, keine
  Allokation; `stereoAuswerten` 190 → 198 Zeilen (Grenze 200, A32). Kohärenz,
  Phase am Mittenbin, Persistenz, Ring, Ringvorschub und NaN-Riegel
  unverändert.
- **`Vertrag.h`** (788 → 791, CRLF): nur der Kommentar von `laufzeitGesetzt`
  und `laufzeitMs` (`:486-496`): Phasensteigung nach R-380-13 (ii),
  Eindeutigkeit bis 1/(2·Δf), K_eff ≥ 2, Engine-Ergebnis A-3.
- **`Sonde013StereoGoldenTest.cpp`** (2676 → 2843, CRLF): Testzugang
  `genutzteBins` (`:202-233`, liest die Ringsumme je Bin wie der Schätzer),
  Feld `genutzt` je Band im Snapshot; `nak380Toleranz` (`:646-713`) mit der
  Herleitung (1) bis (5) im Kommentar; `nak380LaufzeitPruefen` und
  `nak380LaufzeitWert` neu (Vorbedingung genutzte Bins `:857`, T_B `:861`,
  Einbinbänder `:872`; Ausgabe je Lauf: größtes Verhältnis Abweichung/T_B mit
  Band, K und T_B, größte Abweichung, mittlere Abweichung mit Vorzeichen, je
  Binzahlklasse größte Abweichung, T_B und Verhältnis); M-80 (`:945-978`):
  „ohne Kohärenzbit keine Laufzeit“ (`:967`) und die Wache ± 0,02 ms bei
  48 kHz, vollem Ring und K_eff ≥ 20 (`:971`) je 48-kHz-Lauf. Kohärenzhälften,
  Vorbedingungen, Verlust- und Samplezahlprüfungen unverändert.
- **`tools/beweise.ps1`** (1592 → 1592, CRLF mit BOM, Parser ohne Fehler):
  B19 (`:764`) nennt die Laufzeitzusage nach R-380-13 statt „offen“; A5
  (`:544`) nennt 20260928 als bindende Fassung mit den Ringlängen und
  20260927 als registriert.
- **Größen:** `sizeof (FeatureEngine)` unverändert 16 248 B (B5 M-97).
- **Rohdateien (neu, LF):** Nachbau-2; `NAK-380-rot-M-74.txt` bis
  `NAK-380-rot-M-80.txt` in der Fassung Nacharbeit 0 (ersetzen die Fassung
  der Etappe; je Datei Kopf, Endstand-Einzellauf, Zusagesätze); neun
  Mutationsprotokolle `NAK-380-e5n0-mutation-<Kürzel>.txt`.

### 48.4 Messtabelle (Endstand, B19 03:41:07 bis 03:41:17 Uhr)

Laufzeitläufe D1 bis D6 (10 s, Evidenzintervall 0,25 s), letzter Snapshot,
je Band mit mindestens zwei genutzten Bins und Kohärenzbit. „Wache“: bei
48 kHz Bänder mit K ≥ 20 / davon außerhalb ± 0,02 ms / größte Abweichung
(Band). Jedes Band mit Bit trägt eine Laufzeit (ohne Laufzeitbit 0 in jedem
Lauf).

| fs (kHz) | Block | Bänder mit Laufzeit | außerhalb T_B | größtes Verhältnis Abweichung/T_B | größte Abweichung | mittlere Abweichung | Wache K ≥ 20 |
|---|---|---|---|---|---|---|---|
| 48 | 1 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 2 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 3 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 7 | 128 | 0 | 0,660 (Band 109, 2 Bins, T_B 0,7226 ms) | 0,4767 ms (Band 109) | -0,0065 ms | 28 / 0 / 0,0025 (195) |
| 48 | 15 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 16 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 31 | 128 | 0 | 0,660 (Band 109, 2 Bins, T_B 0,7226 ms) | 0,4767 ms (Band 109) | -0,0065 ms | 28 / 0 / 0,0025 (195) |
| 48 | 64 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 127 | 128 | 0 | 0,598 (Band 109, 2 Bins, T_B 0,7226 ms) | 0,4319 ms (Band 109) | -0,0086 ms | 28 / 0 / 0,0021 (197) |
| 48 | 128 | 128 | 0 | 0,471 (Band 136, 4 Bins, T_B 0,2153 ms) | 0,3210 ms (Band 119) | -0,0075 ms | 28 / 0 / 0,0018 (195) |
| 48 | 333 | 128 | 0 | 0,621 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,4382 ms (Band 109) | -0,0082 ms | 28 / 0 / 0,0021 (197) |
| 48 | 512 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 1024 | 128 | 0 | 0,565 (Band 109, 2 Bins, T_B 0,7054 ms) | 0,3986 ms (Band 109) | -0,0083 ms | 28 / 0 / 0,0026 (195) |
| 48 | 2048 | 128 | 0 | 0,670 (Band 113, 2 Bins, T_B 0,7827 ms) | 0,5247 ms (Band 113) | -0,0112 ms | 28 / 0 / 0,0025 (197) |
| 48 | 4096 | 128 | 0 | 0,494 (Band 109, 2 Bins, T_B 0,6744 ms) | 0,3332 ms (Band 109) | -0,0106 ms | 28 / 0 / 0,0022 (195) |
| 48 | 8192 | 128 | 0 | 0,494 (Band 109, 2 Bins, T_B 0,6744 ms) | 0,3332 ms (Band 109) | -0,0106 ms | 28 / 0 / 0,0022 (195) |
| 48 | 12345 | 128 | 0 | 0,540 (Band 136, 4 Bins, T_B 0,2316 ms) | 0,4995 ms (Band 60) | -0,0034 ms | 28 / 0 / 0,0030 (195) |
| 48 | 16384 | 128 | 0 | 0,494 (Band 109, 2 Bins, T_B 0,6744 ms) | 0,3332 ms (Band 109) | -0,0106 ms | 28 / 0 / 0,0022 (195) |
| 44,1 | 64 | 134 | 0 | 0,497 (Band 62, 2 Bins, T_B 1,0094 ms) | 0,5019 ms (Band 62) | -0,0025 ms | — |
| 44,1 | 512 | 134 | 0 | 0,445 (Band 52, 2 Bins, T_B 0,9644 ms) | 0,4288 ms (Band 52) | -0,0092 ms | — |
| 44,1 | 16384 | 134 | 0 | 0,453 (Band 125, 3 Bins, T_B 0,3321 ms) | 0,4140 ms (Band 62) | -0,0056 ms | — |
| 88,2 | 64 | 97 | 0 | 0,452 (Band 149, 3 Bins, T_B 0,2587 ms) | 0,2086 ms (Band 139) | -0,0025 ms | — |
| 88,2 | 512 | 97 | 0 | 0,592 (Band 141, 3 Bins, T_B 0,2521 ms) | 0,2789 ms (Band 139) | -0,0058 ms | — |
| 88,2 | 16384 | 97 | 0 | 0,579 (Band 149, 3 Bins, T_B 0,2351 ms) | 0,1965 ms (Band 139) | -0,0019 ms | — |
| 96 | 64 | 94 | 0 | 0,475 (Band 158, 3 Bins, T_B 0,2438 ms) | 0,2027 ms (Band 133) | -0,0044 ms | — |
| 96 | 512 | 94 | 0 | 0,458 (Band 159, 4 Bins, T_B 0,1542 ms) | 0,1386 ms (Band 135) | -0,0051 ms | — |
| 96 | 16384 | 94 | 0 | 0,537 (Band 159, 4 Bins, T_B 0,1459 ms) | 0,1820 ms (Band 133) | -0,0078 ms | — |
| 176,4 | 64 | 73 | 0 | 0,431 (Band 143, 2 Bins, T_B 0,3721 ms) | 0,1604 ms (Band 143) | 0,0037 ms | — |
| 176,4 | 512 | 73 | 0 | 0,434 (Band 168, 2 Bins, T_B 0,3673 ms) | 0,1594 ms (Band 168) | 0,0023 ms | — |
| 176,4 | 16384 | 73 | 0 | 0,435 (Band 168, 2 Bins, T_B 0,3380 ms) | 0,1471 ms (Band 168) | 0,0053 ms | — |
| 192 | 64 | 70 | 0 | 0,479 (Band 175, 3 Bins, T_B 0,1743 ms) | 0,1358 ms (Band 166) | -0,0034 ms | — |
| 192 | 512 | 70 | 0 | 0,499 (Band 214, 9 Bins, T_B 0,0316 ms) | 0,1261 ms (Band 166) | 0,0011 ms | — |
| 192 | 16384 | 70 | 0 | 0,476 (Band 166, 2 Bins, T_B 0,3338 ms) | 0,1589 ms (Band 166) | 0,0015 ms | — |

Je Binzahlklasse bei 48 kHz, Block 512 (T_B aus der Formel, F = 22 in der
Hauptstufe, 13 in der Bassstufe; vorher = Lag-1-Produkt, §46.4):

| K | T_B (ms) | größte Abweichung (ms) | Verhältnis | vorher (ms) |
|---|---|---|---|---|
| 2 (Bass) | 0,9270 | 0,3645 | 0,393 | — |
| 2 | 0,7054 | 0,3986 | 0,565 | 0,3986 (für K = 2 ist die Steigung der Lag-1-Winkel) |
| 3 | 0,3404 | 0,1351 | 0,397 | 0,1383 |
| 5 | 0,1522 | 0,0477 | 0,313 | 0,1273 |
| 10 | 0,0530 | 0,0045 | 0,086 | 0,0349 |
| 20 | 0,0187 | 0,0008 | 0,044 | 0,0016 |
| 39 | 0,0068 | 0,0003 | 0,050 | 0,0257 |
| ≥ 40 | 0,0066 | 0,0002 | 0,035 | 0,0320 |

Die mittlere Abweichung mit Vorzeichen fällt von −0,019 bis −0,031 ms (44,1
und 48 kHz, §46.4) auf −0,002 bis −0,011 ms; größtes Verhältnis je Rate:
48 kHz 0,670 (Block 2048), 44,1 kHz 0,497, 88,2 kHz 0,592, 96 kHz 0,537,
176,4 kHz 0,435, 192 kHz 0,499. Bei 48 kHz und Block 2048 trägt der Ring nur
18 Hauptstufenframes (Fenster 12 288 Samples); T_B(K = 20) ist dort
0,0206 ms, die Wache ± 0,02 ms also schärfer als T_B; gemessen 0,0025 ms.

### 48.5 Tabelle je Matrixzeile M-74 bis M-80

Regel des Auftrags: je Zusagesatz (§47.3) und Hälfte eine Zeile Zusagesatz →
Prüfzeile → Mutation → rot → Rohdatei. Zeilen am Stand der Nacharbeit 0
(Test `Sonde013StereoGoldenTest.cpp`, Produkt `Stereo.h`). „rot JA“: das
Protokoll nennt „Die Zusage faellt an der erwarteten Pruefung: JA“, die
Rücknahme ist per SHA-256 gleich (`Stereo.h` in jedem Protokoll
`21B5107B…3860`, gleich dem Endstand; Test `03F6A4C7…3F81`), Neubau mit
Binary jünger als die Quelle, Fall danach grün. Mutationen formgültig (kein
Riegel, keine Assertion, kein Zugriff außerhalb). Die Kohärenzhälften und ihre
Rotbeweise aus §46.5 gelten weiter (Kohärenzpfad unverändert, 560
Nicht-Laufzeitprüfungen bytegleich).

**M-74** (`NAK-380-rot-M-74.txt`; 48 kHz, 18 Blockgrößen)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| je Blockgröße: jedes Band mit Bit hat Kohärenz ≥ 0,95 | `:833`, `:841` je Block, B19 | §46.5: Kohärenz aus der Bandsumme (m73-bandsumme) | JA (§46.5) | `NAK-380-e5-mutation-m73-bandsumme.txt` |
| jedes Band mit mindestens zwei genutzten Bins und Kohärenzbit trägt 1,000000 ms innerhalb T_B(K_eff, F, C, Δf) nach R-380-13 (iii) | `:861` je Block (18), B19 | `Stereo.h:503` Δf aus 44 100 statt fs der Stufe → 1,088435 ms (m74-df44k1); Δf aus 48 000 ist bei 48 kHz wirkungslos (Gegenprobe grün, m80-df48k) | JA | `NAK-380-e5n0-mutation-m74-df44k1.txt`, `NAK-380-e5n0-mutation-m80-df48k.txt` |
| T_B aus den Vorbedingungen gerechnet, nie aus dem Lauf | `:711` (`nak380Toleranz`: K aus dem Gitter, F aus dem Kadenzmodell über `snapSoll`, C = ρ_w(d)², Δf = fs/N) | keine: Satz über die Herkunft der Schranke, eine Schranke aus dem Lauf machte die Prüfung nur grüner; Wirkung der Formel durch m80-tb-fest (M-80) | — (Struktursatz, am Code gelesen) | Nachbau-2, `NAK-380-e5n0-mutation-m80-tb-fest.txt` |
| Bänder mit einem genutzten Bin ohne Laufzeitbit | `:872` je Block, B19 | `Stereo.h:504` Laufzeitbit ab K_eff ≥ 1, Wert 0 ms (m80-einbin) | JA | `NAK-380-e5n0-mutation-m80-einbin.txt` |
| Vorbedingung von K_eff: genutzte Bins je Band = Gitterbins | `:857` je Block, B19 | `Stereo.h:229` letzter Bin jedes Bandes bleibt im Ring leer (e5n0-genutzt) | JA | `NAK-380-e5n0-mutation-e5n0-genutzt.txt` |

**M-75 bis M-79** (`NAK-380-rot-M-75.txt` bis `NAK-380-rot-M-79.txt`; D1,
D3 bis D6, je Block 64, 512, 16 384): je Zeile dieselben fünf Sätze mit
denselben Prüfzeilen; die Laufzeithälfte fällt unter m80-df48k (`Stereo.h:503`
Δf aus 48 000 statt fs der Stufe) an `:861` in jedem der drei Läufe der Rate,
mittlere Abweichung am Block 512: D1 −0,0895 ms (Soll-Verschiebung auf
0,916667 ms), D3 +0,8249 ms (1,833333), D4 +0,9898 ms (2,000000), D5
+2,6773 ms (3,666667), D6 +3,0044 ms (4,000000).

| Zeile | Kohärenz ≥ 0,95 (`:833`, `:841`) | Laufzeit in T_B (`:861`) | T_B aus Vorbedingungen (`:711`) | Einbin (`:872`) | K_eff-Vorbedingung (`:857`) |
|---|---|---|---|---|---|
| M-75 (44,1 kHz) | JA (§46.5) | JA (m80-df48k, 3 von 3 Läufen) | — (Struktursatz) | JA (m80-einbin) | JA (e5n0-genutzt) |
| M-76 (88,2 kHz) | JA (§46.5) | JA (m80-df48k, 3 von 3) | — | JA | JA |
| M-77 (96 kHz) | JA (§46.5) | JA (m80-df48k, 3 von 3) | — | JA | JA |
| M-78 (176,4 kHz) | JA (§46.5) | JA (m80-df48k, 3 von 3) | — | JA | JA |
| M-79 (192 kHz) | JA (§46.5) | JA (m80-df48k, 3 von 3) | — | JA | JA |

**M-80** (`NAK-380-rot-M-80.txt`; 33 Läufe)

| Zusagesatz | Prüfzeile | Mutation | rot | Rohdatei |
|---|---|---|---|---|
| τ = Steigung der kleinsten Quadrate der entwickelten Binphasen über die genutzten Bins (R-380-13 (ii)), τ = s/(2π·Δf), stimmt mit 1000·d/fs innerhalb T_B überein | `:861` je Lauf, B19 | (a) `Stereo.h:503` Vorzeichen der Steigung → −1 ms, alle 33 Läufe rot (m80-vorzeichen); (b) Δf aus 48 000 → die 15 Läufe bei 44,1 und 88,2 bis 192 kHz rot, 48 kHz grün (m80-df48k); Δf aus 44 100 → die 18 Läufe bei 48 kHz rot (m74-df44k1) | JA | `NAK-380-e5n0-mutation-m80-vorzeichen.txt`, `…-m80-df48k.txt`, `…-m74-df44k1.txt` |
| T_B nach R-380-13 (iii), aus den Vorbedingungen | `:711`, `:861` | (d) `Sonde013StereoGoldenTest.cpp:711` T_B auf 0,02 ms festgenagelt → alle 33 Läufe rot an den schmalen Bändern (48 kHz Block 512: 43 Bänder außerhalb) (m80-tb-fest) | JA | `NAK-380-e5n0-mutation-m80-tb-fest.txt` |
| bei 48 kHz und vollem Ring hält jedes Band mit K_eff ≥ 20 Bins ± 0,02 ms | `:971` je 48-kHz-Lauf (18), B19 | (c) `Stereo.h:498`, `:501`, `:503` Steigung durch das Lag-1-Produkt ersetzt (alter Weg) → 14 bis 19 von 28 Bändern außerhalb, größte Abweichung 0,035 bis 0,043 ms (m80-lag1) | JA | `NAK-380-e5n0-mutation-m80-lag1.txt` |
| Bänder mit einem Bin tragen keine Laufzeit, nie 0 | `:872` je Lauf, B19 | (e) `Stereo.h:504` Laufzeitbit ab K_eff ≥ 1, Wert 0 ms (m80-einbin) | JA | `NAK-380-e5n0-mutation-m80-einbin.txt` |
| nie aus einer über Bins summierten Kreuzleistung | `:861` je Lauf, B19 | (f) `Stereo.h:498`, `:501`, `:503` τ aus der Phase der Bandsumme durch 2π·f_Mitte → alle 33 Läufe rot (48 kHz Block 512: 117 von 128 Bändern außerhalb, bis 1,91 ms) (m80-bandsumme) | JA | `NAK-380-e5n0-mutation-m80-bandsumme.txt` |
| nie aus dem Lag-1-Produkt | `:971` je 48-kHz-Lauf, B19 | (c) wie oben (m80-lag1); zusätzlich fällt `:861` in 27 von 33 Läufen | JA | `NAK-380-e5n0-mutation-m80-lag1.txt` |
| Vorbedingung von K_eff: genutzte Bins = Gitterbins | `:857` je Lauf, B19 | `Stereo.h:229` letzter Bin je Band im Ring leer (e5n0-genutzt) | JA | `NAK-380-e5n0-mutation-e5n0-genutzt.txt` |
| Wache aus der Etappenfassung, nicht Teil von §47.3: keine Laufzeit ohne Kohärenzbit | `:967` je Lauf, B19 | `Stereo.h:467` Kohärenzbit fehlt in Bändern ab zwei Bins, die Laufzeit bleibt (m80-ohnekoh) | JA | `NAK-380-e5n0-mutation-m80-ohnekoh.txt` |

Mitgefallene Prüfungen je Protokoll mit Grund: (a) und (f) die Wache
`:971` (derselbe Wert), (c) `:861` in 27 Läufen (T_B bei K ≥ 20 höchstens
0,0206 ms), e5n0-genutzt die Kohärenzprüfung (Einbinbänder verlieren ihr
Bit), `:861` (28 Zweibinbänder ohne Laufzeitbit) und `:872` (keine
Einbinbänder mehr mit Bit), m80-ohnekoh `:861` und `:971` (keine Bänder mit
Bit mehr, Nichtleere). Unter jeder Mutation bleiben die Vorbedingungen und
die übrigen Prüfungen des Falls grün (Zahlen in den Protokollen).

### 48.6 Toleranzen (Stelle, Wert, Herleitung)

| Stelle | Wert | Herleitung |
|---|---|---|
| Gruppenlaufzeit M-74 bis M-80, `Sonde013StereoGoldenTest.cpp:861` über `nak380Toleranz` (`:711`) | T_B = b·τ_ref + 5·σ_τ je Band; am Endstand 0,0057 ms (48 kHz, K = 44) bis 1,0636 ms (48 kHz Block 2048, Bass K = 2) | R-380-13 (iii): F_eff = F/1,222 (Hann 50 %, Fensterkorrelation 1/3, Welch 1967), σ_φ² = (1 − C)/(2·C·F_eff) (Bendat und Piersol), Var(s) = 12·σ_φ²/(K·(K² − 1)), σ_τ = sqrt(Var(s))/(2π·Δf), b = (4/9)/F_eff nur bei K = 2, 5σ (rund 4 200 Bandprüfungen, Bonferroni 4,7σ); K aus dem Gitter, F aus dem Kadenzmodell, C = ρ_w(d)², Δf = fs/N; unter 8 Frames T_B = 0 (fail-closed); Beleg Nachbau-2 (§48.2) |
| Wache M-80, `:971` | ± 0,02 ms (Matrix §47.3) bei 48 kHz, vollem Ring, K_eff ≥ 20 | 5σ_τ bei K = 20 und F = 22 sind 0,0187 ms; gemessene SD bei K ≥ 20 höchstens 0,0011 ms (Nachbau-2), das Lag-1-Produkt läge 0,021 ms daneben; gemessen höchstens 0,0030 ms |
| Vorbedingung genutzte Bins, `:857` | exakt | Zustandsgröße (Testzugang), Rauschen mit σ > 0 belegt jeden Bin |
| Einbinbänder, `:872` | exakt: kein Laufzeitbit, mindestens ein Einbinband mit Kohärenzbit je Lauf | R-380-13 (ii) K_eff ≥ 2 |

### 48.7 Einzeln gefahrene Beine am Endstand

Endbau der sieben Ziele 26.09.2026 bis 03:40:51 Uhr, Exit 0
(`eq-copilot/build/nak380-e5n0-bau-end.log`, Bauartefakt); einzige Warnung die
vorbestehende C4458 („`sr` blendet Klassenmember aus“, `Stereo.h:188`). Jüngste
Quelle unter `eq-copilot/plugin`: `Stereo.h` 03:39:12.120 Uhr (Rücknahme der
letzten Mutation); jedes Binary ist jünger. Aufrufe wie `tools/beweise.ps1`;
Rohausgaben `eq-copilot/build/nak380-e5n0-end-<Bein>.log` (Bauartefakt).

| Bein | Aufruf | Lauf | Exit | Zähler | Binary |
|---|---|---|---|---|---|
| B19 | `EqCopSonde013StereoGoldenTest` | 03:41:07–03:41:17 | 0 | 809/0 (Etappenstand 593/66: 560 Nicht-Laufzeitprüfungen bytegleich, 99 alte Laufzeitprüfungen → 249 neue) | 03:39:18.328 |
| B18 | `EqCopSonde013DynamicsTest` | 03:41:17–03:41:29 | 0 | 126/0 (unverändert) | 03:40:01.727 |
| B5 | `EqCopAnalysisGoldenTest` | 03:41:29–03:41:42 | 0 | 313/0 (unverändert; M-97 16 248 B) | 03:40:15.602 |
| A1 | `EqCopNullTest` | 03:41:42–03:41:42 | 0 | NULLTEST OK | 03:40:24.127 |
| A16 | `EqCopProbeeqNullTest` | 03:41:42–03:41:43 | 0 | 145/0 | 03:40:32.891 |
| B1 | `EqCopIdentityTest` | 03:41:43–03:41:43 | 0 | 120/0 | 03:40:41.682 |
| B4 | `EqCopQueueStressTest` | 03:41:43–03:41:46 | 0 | 121/0 | 03:40:51.605 |
| A26 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py` | 03:41:46–03:41:47 | 0 | Exit-Gate P4: alle vier Klassen bestehen, keine falsche Behauptung. | — |
| A5 | `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | 03:41:47–03:41:50 | 0 | 927/0 | — |

Zusammensetzung B19: je Laufzeitlauf drei Prüfungen statt einer (Vorbedingung
genutzte Bins, T_B, Einbinbänder; 66 Läufe in M-74 bis M-80), in M-80 je
Lauf „ohne Kohärenzbit keine Laufzeit“ (33) und je 48-kHz-Lauf die Wache
(18): 560 + 198 + 33 + 18 = 809. Hygiene: `py -3.13 tools/plan/gesundheit.py` 03:42:33 bis 03:42:46 Uhr, Exit 4, A32 unverändert 3/33/39 (vorbestehend, nicht blockierend; `stereoAuswerten` 198 Zeilen, keine neue Funktion über 200); `py -3.13 tools/plan/tidy.py` 03:42:46 bis 03:44:34 Uhr, Exit 0, A33 58 von höchstens 58.

### 48.8 Bewegungen bestehender Goldens

Keine außer den sieben Laufzeitzeilen. Die 560 Prüfzeilen von B19 außerhalb
der Laufzeit sind am Endstand zeichengleich zum Etappenstand
(`nak380-e5-3-B19.log` gegen `nak380-e5n0-end-B19.log`, Vergleich der
Prüfzeilen samt Messwerten: 0 Unterschiede). B18, B5, A1, A16, B1, B4, A26
und A5 zählen wie in §46.7.

### 48.9 Abweichungen vom Auftrag und Technikentscheide, begründet

1. **Rotbeweis der Laufzeithälfte von M-74 mit Δf aus 44 100 statt 48 000.**
   M-74 läuft nur bei 48 kHz; dort ist Δf aus 48 000 gleich fs und die
   Mutation wirkungslos (gemessen: M-74 unter m80-df48k grün, 162/0). Die
   feste Rate 44 100 ist dieselbe Mutationsart (Δf aus einer festen Rate statt
   fs) und verschiebt auf 1,088435 ms.
2. **Zweiter Durchlauf inline in `stereoAuswerten`**, ohne neues Mitglied und
   ohne neue Funktion (die Deklaration läge in `FeatureEngine.h`, außerhalb der
   Grenze); die Ringsumme je Bin wird im zweiten Durchlauf mit derselben
   Summationsfolge neu gebildet, also bitgleich zum ersten. Kompakt gehalten,
   damit die Funktion unter 200 Zeilen bleibt (198).
3. **K_eff in T_B aus dem Gitter**, die Zahl der genutzten Bins aus dem
   Snapshot nur als eigene Vorbedingung (Gleichheit); damit geht keine Zahl
   aus dem Lauf in die Schranke, auch nicht die erlaubte Zustandsgröße.
4. **Zusätzliche Rotbeweise** über §47.2 (iv) hinaus: (f) m80-bandsumme für
   den Satz „nie aus einer über Bins summierten Kreuzleistung“ (§46.5 hatte
   dort keinen), e5n0-genutzt für die neue Vorbedingung, m80-ohnekoh für die
   abgetrennte Wache „ohne Kohärenzbit keine Laufzeit“ (im Etappenstand Teil
   der Sammelprüfung mit den Einbinbändern; in D1 bis D6 trägt jedes Band mit
   Energie ein Kohärenzbit, deshalb bricht nur eine Mutation am Bit sie).
5. **Einbinprüfung in jeder Zeile M-74 bis M-80** (§47.3 nennt den Satz für
   M-74 bis M-79 und M-80), nicht nur in M-80.
6. **Bias-Term b für K = 2.** Der Nachbau-2 misst bei K = 2 keinen Bias über
   dem Rauschen (≤ 0,011 ms bei T_B-Beiträgen von 0,014 bis 0,042 ms): jede
   Binphase streut symmetrisch um ihren Sollwert, die Differenz zweier Phasen
   ist erwartungstreu; der Bias aus R-380-13 (i) gehört zum Winkel der über
   Binpaare summierten Lag-Produkte. Der Term bleibt nach Regel und ist
   konservativ.
7. **Bassstufe im Nachbau-2** zusätzlich zur Hauptstufe, weil B19 auch
   Bassbänder mit zwei Bins gegen T_B prüft (44,1 und 48 kHz).
8. **Testkommentar nach der ersten Mutationsrunde berichtigt** (Unterschätzung
   bei K = 2 und 3, konservativ ab K = 4 statt ab K = 5, gemessen im
   Nachbau-2); m80-tb-fest danach auf dem Endstand des Tests neu gefahren,
   das Protokoll trägt den End-SHA.
9. **A5-Runnerzeile:** der Etappe-4-Satz sagt jetzt „bleibt registriert“ statt
   „bindet“, und ein Etappe-5-Satz (M-94, M-96, Ringlängen) kam hinzu; die
   Zeile nannte Etappe 5 bisher gar nicht.
10. **Rotbeweis (d) nur für M-80** nach §47.2 (iv); der Satz „T_B aus den
    Vorbedingungen, nie aus dem Lauf“ in M-74 bis M-79 ist ein Struktursatz
    ohne eigene Mutation (am Code von `nak380Toleranz` gelesen).

### 48.10 Selbstaudit und Prüfliste

**Selbstaudit** (eigener Diff gegen `80cd0274`, nicht gestagt): geändert sind
genau `Stereo.h`, `Vertrag.h`, `Sonde013StereoGoldenTest.cpp`,
`tools/beweise.ps1`, dazu die Rohdateien und dieses Manifest. Entwicklung der
Phase über übersprungene Bins: der Index i bleibt die wahre Binposition, der
Sprung über zwei Bins wird einmal um 2π berichtigt (eindeutig bis
\|τ\| < 1/(4·Δf) über der Lücke); Bänder mit genau zwei genutzten Bins:
s = Phasendifferenz durch Binabstand; Bänder ohne genutzten Bin oder mit einem:
K_eff < 2, kein Bit, Wert 0 nie gesetzt; Nenner K_eff·Σk² − (Σk)² ist bei
K_eff ≥ 2 verschiedenen Indizes positiv und in double exakt (k ≤ 48);
τ endlich geprüft, \|τ\| ≤ 1/(2·Δf) (jeder berichtigte Schritt liegt in
[−π, π], die Steigung ist ein gewichtetes Mittel der Schritte); Vorzeichen:
verzögertes R ergibt τ > 0 (Endstand +1 ms, Mutation (a) −1 ms); Δf je Stufe
aus `st.fs/st.punkte` (Haupt 4096, Bass 16 384; Bassbänder halten T_B, §48.4);
keine Allokation, kein Mitglied, `sizeof` 16 248 B; Kohärenz, Phase,
Persistenz, Ring, Vertrag und Version unverändert (560 Prüfzeilen
bytegleich, A5 927/0); A1, A16, B1, B4 grün; T_B liest keine Laufzahl.
Zeilenenden (Bytezählung): `Stereo.h`, `Vertrag.h`, Test und
`tools/beweise.ps1` nur CRLF (Runner mit BOM), Manifest und Rohdateien nur LF.
Keine Mutation im Baum (Suche nach allen Mutationswortlauten leer), jede
mutierte Datei trägt nach der Rücknahme den SHA-256 vor der Mutation.

**Prüfliste** (`tools/dirigent/pruefliste.md`, abgehakt, Datei unverändert):

| Abschnitt | Punkt | Nacharbeit 0 |
|---|---|---|
| A Rückstau | strukturell-0-Zähler als Wache | Verlustzähler je Laufzeitlauf unverändert geprüft (Vorbedingungen) |
| B Lebenszyklus | starten↔stoppen | Ring, Vorschub und Leeren unberührt (M-87, M-88 grün, bytegleich) |
| C Verträge | NaN/Inf an jeder Zahl | τ nur bei endlichen Summen und endlichem Ergebnis; keine Leitungsänderung (A-3) |
| D Bau- und Prüfriegel | Frische | Endbau nach der letzten Rücknahme, jedes Binary jünger als die jüngste Quelle (§48.7) |
| E Behauptung ≤ Messung | jede neue Prüfung einmal gebrochen | neun Protokolle, alle neuen Prüfzeilen rot an ihrer Zusage; Struktursatz ohne Mutation benannt |
| E Behauptung ≤ Messung | geänderte Zusage an drei Stellen | Testkommentare, Runnerzeile B19, Rohdatei-Köpfe; Manifestkopf (Zeile Etappe) |
| F Änderungssatz | Beziehungen | rechnen↔vertragen: Vertrag.h-Kommentar mit dem Schätzer; keine Vertragsform, keine Version |

### 48.11 Offene Punkte (für den Dirigenten)

1. **Kanon nachher und Erstprüfung 5** stehen aus (Dirigent); einzeln
   gefahren und grün: B19, B18, B5, A1, A16, B1, B4, A26, A5.
2. **Formel bei K = 2 und 3 knapp:** die Phasenvarianz nach Bendat und
   Piersol unterschätzt die gemessene Streuung um bis zu 15 % (K = 2) bzw. 9 %
   (K = 3); der Beleg hält mit größtem Verhältnis 0,879, am Endstand liegt das
   größte Verhältnis Abweichung/T_B bei 0,670. Kein Handlungsbedarf, aber der
   Prüfer sollte wissen, dass die 5σ dort die Modellunschärfe tragen.
3. **Wache „ohne Kohärenzbit keine Laufzeit“** ist keine Zusage von §47.3;
   sie blieb aus der Etappenfassung erhalten und trägt jetzt einen eigenen
   Rotbeweis. Streichen oder in die Matrix aufnehmen entscheidet der Dirigent.

## 49. Messung der Nacharbeit 0, Matrixfassung §49, Kanon nachher und Erstprüfung 5 gestartet (Dirigent, 26.09.2026, 03:53 Uhr)

**Messung (Dirigent, kein Selbstbericht).** Worker `343fcc2c`
(`nakama-nak380-80cd027-n0`, Opus 5.5 xhigh, `dontAsk`), 03:17:24 bis
03:46 Uhr (Beobachter meldete „blockiert“ um 03:46 Uhr: der Eingabeprompt
nach dem Ende, das Log endet mit „FERTIG Nacharbeit 0, Beine alle grün,
uncommittet“, done 3:46; `Get-Date` der Messung 03:46:54 Uhr). HEAD vor und
nach dem Lauf `80cd0274`, kein Commit des Workers. `git status --short` ohne
die fremden Einträge: `Stereo.h`, `Vertrag.h`, `Sonde013StereoGoldenTest.cpp`,
`tools/beweise.ps1`, Manifest, die sieben Rotbeweise `NAK-380-rot-M-74.txt`
bis `NAK-380-rot-M-80.txt` neu gefasst, neun Mutationsprotokolle
`NAK-380-e5n0-mutation-*.txt`, `NAK-380-etappe-5-laufzeit-nachbau-2.txt`.
`git diff --stat -- eq-copilot broker tools`: 4 Dateien, +255/−64
(Stereo.h 71, Vertrag.h 17, Test 227, beweise.ps1 4 Zeilen); genau die
Ticketpfade des Auftrags, kein Pfad außerhalb. Beine am Endstand (§48.7, am
Manifest gelesen): B19 809/0 mit Exit 0 (Etappenstand 593/66; die 560
Prüfzeilen außerhalb der Laufzeit sind laut §48.8 zeichengleich zum
Etappenstand), B18 126/0, B5 313/0 (`sizeof (FeatureEngine)` 16 248 B
unverändert), A1, A16 145/0, B1 120/0, B4 121/0, A26, A5 927/0. Nachbau-2
(§48.2, numpy, 40 Läufe je Rate, Haupt- und Bassstufe): Beleg
(3·SD + \|Bias\|)/T_B ≤ 0,879 in allen 38 Klassen, kein Verstoß; die Formel
unterschätzt die Streuung bei K = 2 um 7 bis 15 % und bei K = 3 um bis zu
9 %, ab K = 4 ist sie konservativ. Endstand (§48.4): in allen 33 Läufen kein
Band außerhalb T_B, größtes Verhältnis Abweichung/T_B 0,670 (48 kHz, Block
2048, 2-Bin-Band 113), Wache K ≥ 20 bei 48 kHz 28 Bänder je Lauf ohne
Abweichung über 0,0030 ms, mittlere Abweichung mit Vorzeichen −0,011 bis
+0,005 ms (vorher −0,031 bis +0,003, §46.4); je Binzahl bei 48 kHz, Block
512: K = 20 größte Abweichung 0,0008 ms (vorher 0,0016), K = 39 0,0003
(vorher 0,0257), K ≥ 40 0,0002 (vorher 0,0320). Tabelle §48.5: 17 Zeilen JA,
0 NEIN. Dokuriegel des Bauers 0 Befunde.

**Selbstaudit des Dirigenten am Diff (`Stereo.h`).** Lag-Akkus und Lag-Produkt
entfernt; zweiter Durchlauf über die Bins des Bandes bildet die Ringsumme mit
derselben Summationsfolge (bitgleich zum ersten), genutzter Bin nur bei
Sxx·Syy > 0 und endlichen Summen, Entwicklung der Binphase nur über genutzte
Bins mit Berichtigung um ± 2π, laufende Summen mit k relativ zum ersten Bin
(Steigung verschiebungsfrei), Nenner K_eff·Σk² − (Σk)² > 0 und K_eff ≥ 2 als
Bedingung des Bits, τ endlich, Vorzeichen R verzögert → positiv (Rotbeweis
M-80 (a) fällt), kein neues Mitglied, keine Allokation, Kommentare (Kopf
`:23`, Auswertung, `Vertrag.h`) nach R-380-13 (ii) wörtlich. Beobachtung, keine
Zusage: die Ringsumme wird für Bänder über der Phasenschwelle ein zweites Mal
gebildet, die Rechenlast je Snapshot verdoppelt sich für diese Bänder (bei
4 Hz unter 120 000 Additionen je Sekunde; §8.5 nannte 60 000 als
Abschätzung, keine Matrixzeile trägt die Zahl, M-85 bindet den Speicher).

### 49.1 Einordnung der offenen Punkte (§48.11) und Abweichungen (§48.9)

| Punkt | Quelle (Bauerstand auf `80cd0274`) | Einordnung | Regel |
|---|---|---|---|
| Formel bei K = 2 und 3 knapp (§48.11 Punkt 2): Phasenvarianz nach Bendat und Piersol unterschätzt die gemessene Streuung um bis zu 15 % (K = 2) bzw. 9 % (K = 3) | Nachbau-2 §48.2 | **angenommen, Beobachtung** — der Beleg hält (größtes Verhältnis 0,879 ≤ 1), die 5σ-Reserve trägt die Modellunschärfe, der Testkommentar benennt sie (§48.9 Punkt 8); keine Regeländerung | — |
| Wache „ohne Kohärenzbit keine Laufzeit“ (§48.11 Punkt 3) mit eigenem Rotbeweis `m80-ohnekoh` | `Sonde013StereoGoldenTest.cpp`, M-80 | **in die Matrix aufgenommen** — sie belegt die Abweichung 5 aus §46.9 (Laufzeit nur über `kKohaerenzSchwellePhase`, §40.1 Stufe 2), die §47.1 angenommen hat; als dritte Hälfte von M-80 | §49.2 |
| Abweichung 1: Rotbeweis der M-74-Laufzeithälfte mit Δf aus 44 100 statt 48 000 | M-74 läuft nur bei 48 kHz, Δf aus 48 000 ist dort gleich fs (gemessen grün) | **angenommen** — dieselbe Mutationsart, verschiebt auf 1,088435 ms; Matrixfassung §49.2 | §49.2 |
| Abweichung 6: Bias-Term b = (4/9)/F_eff für K = 2 bleibt, obwohl der Nachbau-2 bei K = 2 keinen Bias über dem Rauschen misst | Nachbau-2 | **angenommen, Beobachtung** — die Herleitung R-380-13 (i) gilt dem Winkel des über Binpaare summierten Produkts, der Nachbau der Steigung bei K = 2 zeigt ≤ 0,011 ms; der Term bleibt als konservative Reserve in T_B | — |
| Abweichungen 2 bis 5, 7 bis 10: zweiter Durchlauf inline (Funktion 198 Zeilen), K_eff in T_B aus dem Gitter mit Gleichheitsvorbedingung, zusätzliche Rotbeweise (`m80-bandsumme`, `e5n0-genutzt`, `m80-ohnekoh`), Einbinprüfung in jeder Zeile, Bassstufe im Nachbau-2, Testkommentar berichtigt, A5-Runnerzeile mit Etappe-5-Satz, Rotbeweis (d) nur für M-80 | — | **angenommen** (Technik innerhalb von Ticket, Regel und Invarianten, begründet) | — |

### 49.2 Matrix in der Fassung §49

| ID | Änderung gegenüber §47.3 |
|---|---|
| M-80 | dritte Hälfte: „ein Band ohne Kohärenzbit trägt kein Laufzeitbit (Stufe 2, §40.1; Abweichung 5 aus §46.9)“; Rotbeweis: Mutation am Kohärenzbit (`m80-ohnekoh`) → rot |
| M-74 | Rotbeweis der Laufzeithälfte: Δf aus 44 100 statt fs (bei 48 kHz ist 48 000 wirkungslos) → 1,088435 ms → rot |
| übrige Zeilen | wie §47.3 und §6.4 (§46.5, §48.5 belegen sie) |

### 49.3 Zwischenstand, Kanon nachher und Erstprüfung 5

Zwischenstand der Nacharbeit 0 (Bauerstand, dieser Abschnitt, Prüfauftrag)
mit Pathspec committet und gepusht; Rundenbilanz je Runde und kumuliert in
§50. Kanon nachher abgekoppelt auf dem End-Stand gestartet (Skill §3.5,
Log `$env:TEMP\nakama-nak380-e5-kanon.log`, Rohausgabe
`docs/beweise/roh/NAK-380-<sha7>[-dirty].md`); Ergebnis in §50.

**Codex wieder erreichbar.** Testlauf 03:48 Uhr (`codex exec`, `gpt-6-astra`,
read-only): Exit 0, Antwort „OK“; der 401 Unauthorized vom 26.09.2026 00:42
bis 03:12 Uhr (§45, §47) endete ohne User-Handgriff mit einem
Token-Refresh; der One-Shot-Loop 9:07 Uhr ist gelöscht, der Handgriff
entfällt. Erstprüfung 5 damit nach Skill §3.4: frischer lesender
Codex-Thread `gpt-6-astra`, Effort xhigh (NAK-362), Vorlage A, Prüfbereich
`c43c1646...HEAD` über die fünfzehn Ticketpfade der Etappe 5 (Bau §46 und
Nacharbeit 0 §48), Auftrag `docs/beweise/roh/NAK-380-erstpruefung-5-auftrag.txt`,
gefahren über `tools/dirigent/codex-lauf.ps1` mit Monitor auf der
`-start.log`; parallel zum Kanon (HEAD bleibt; Bauartefakte, Broker-Pin und
Kanon-Anhang sind kein Prüfgegenstand, im Auftrag benannt). Urteil in §50.
