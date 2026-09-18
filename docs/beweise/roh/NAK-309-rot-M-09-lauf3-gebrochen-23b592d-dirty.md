# Rohausgabe - NAK-309 Etappe 2 M-09 Lauf 3 Zusage gebrochen (beinfrei)

**Lauf:** 2026-09-18 22:07 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 0/0 Kanon-Laeufe bestanden | **Exitcode:** 0

**Manifest:** [NAK-309-rot-M-09-lauf3-gebrochen.md](NAK-309-rot-M-09-lauf3-gebrochen.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-18 22:07:29 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 23b592d7 NAK-309 Etappe 2: Kanon nachher GRUEN 69/69 auf 67e60dd3 (Code gleich d1676a63) |
| Commit (voll) | 23b592d75acc1390ec5dcf7cfb66ef353b2b6ddd |
| Arbeitsbaum | 4 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
?? briefing-hub/
?? docs/beweise/roh/NAK-309-rot-M-09-lauf3-vorher-23b592d-dirty.md
?? docs/beweise/roh/NAK-309-rot-M-09-lauf3-vorher.md
?? nimbalyst-local/
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand | juengste Eingabe |
|---|---|---|---|---|
| `EqCopNullTest` | 2026-09-18 21:24:13 | `7508C71B3DE6A23F` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopGoldenTest` | 2026-09-15 17:55:56 | `89BF95A245ECCB6A` | frisch | eq-copilot/plugin/core/analysis/TruePeak.h (2026-09-15 16:49:22) |
| `EqCopMarkierungTest` | 2026-09-18 21:24:15 | `F1284C9B456A6E4C` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopPipeClientTest` | 2026-09-18 21:24:16 | `4A37A324D5953C06` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopProbeeqNullTest` | 2026-09-18 21:24:17 | `2EA1F4D1B5DB3877` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopIdentityTest` | 2026-09-18 21:24:18 | `3D0CD22939F56385` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopStateMigrationTest` | 2026-09-18 21:24:19 | `4BA24E87F73436A4` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopHostContextTest` | 2026-09-12 03:36:28 | `B2AEC3C7B591F364` | frisch | eq-copilot/build/plugin/EqCopHostContextTest.vcxproj (2026-08-31 14:48:12) |
| `EqCopHostProbeTest` | 2026-09-12 03:36:55 | `4510CC89CD676948` | frisch | eq-copilot/build/plugin/EqCopHostProbeTest.vcxproj (2026-08-31 14:48:12) |
| `EqCopSchemaTest` | 2026-09-18 21:24:21 | `3F76DDB69C6BD83B` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopQueueStressTest` | 2026-09-18 21:24:22 | `64E2F0DED9EB5C9B` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopLoudnessGoldenTest` | 2026-09-12 03:37:40 | `FCCE494231C7EB19` | frisch | eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp (2026-09-07 04:41:30) |
| `EqCopAnalysisGoldenTest` | 2026-09-18 21:24:24 | `9AC099E92D611C97` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopDspGoldenTest` | 2026-09-18 21:24:25 | `3E79C18EF84479AB` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopTransactionTest` | 2026-09-18 21:24:26 | `1756CEC1EC6A1F33` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopIpcTest` | 2026-09-18 21:24:27 | `71C1AC34E3FC072C` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde012HostChannelContextTest` | 2026-09-18 21:24:28 | `19ACFEC5DC9F5EE5` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde012LoudnessSourceTest` | 2026-09-18 21:24:29 | `A27C1475B1FF5D49` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde012SourcesModelTest` | 2026-09-18 21:24:30 | `8074B936CD880253` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde012ProjectReloadTest` | 2026-09-18 21:24:31 | `D1390F3237D86E5F` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopShot` | 2026-09-18 21:24:33 | `BC31A1F449CD05DB` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013QualityClassTest` | 2026-09-18 21:24:33 | `46C7F7D7A412C797` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013FingerprintGoldenTest` | 2026-09-18 21:24:34 | `9D472876EB0A2E75` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013PassageStateTest` | 2026-09-18 21:24:35 | `7AE6842F7F0D1B8E` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013PrePostGoldenTest` | 2026-09-18 21:24:36 | `B7CBC133DD4E8F0C` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013ExperimentGoldenTest` | 2026-09-18 21:24:37 | `DCC6E36E32D48436` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013ExperimentUiTest` | 2026-09-18 21:24:38 | `89431D346FA49E09` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013InterventionRingTest` | 2026-09-18 21:24:39 | `375ECB03B648A426` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013StereoGoldenTest` | 2026-09-18 21:24:40 | `73B01725394F0B2E` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013TruePeakGoldenTest` | 2026-09-18 21:24:41 | `1210EE4367ECFF6E` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013DynamicsTest` | 2026-09-18 21:24:41 | `3E1603116F9221CD` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde013EventWireTest` | 2026-09-18 21:24:42 | `427C7DE760FC7839` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde014IntentTest` | 2026-09-18 21:24:44 | `FF0F354E3AA982BF` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde014AssistentTest` | 2026-09-18 21:24:45 | `4C2717900D3F2C9E` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopBriefkastenTest` | 2026-09-18 21:24:46 | `DE0FD871E86120D0` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopSonde014BefundTest` | 2026-09-18 21:24:47 | `7D4855A1655D0D5A` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |
| `EqCopLebenslaufTest` | 2026-09-18 21:24:48 | `3221B5E8C42836A7` | VERALTET | eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28) |

Frischebaum je Binary aus den MSBuild-Tracking-Logs (`tools/eq-copilot/pruefe_beweisrunner.py --baustand`, Exit 4): Leselogs des Zwischenordners, jede Bibliothek ueber die Logs ihres Erzeugers, Projektdatei und Konfigurationsstand; Kreuzprobe gegen den CMake-Export, Inventar ueber `git ls-files eq-copilot/plugin`. Verglichen werden Zeitstempel, keine Inhalte. `cargo test` uebersetzt selbst und ist damit immer frisch.

<details><summary>Ausgabe der Frischepruefung</summary>

```text
[Baustand] Frischebaum je Pruefbinary aus den MSBuild-Tracking-Logs (eq-copilot/build, Release)
  Grenze: Zeitstempel, kein Inhalt - eine per Copy-Item mit altem Zeitstempel zurueckgespielte Quelle gilt als frisch (NAK-230).

  Bein   Pruefbinary                            Stand                          juengste Eingabe
  A1     EqCopNullTest                          VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  A2     EqCopGoldenTest                        frisch                         eq-copilot/plugin/core/analysis/TruePeak.h (2026-09-15 16:49:22)
  A3     EqCopMarkierungTest                    VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  A4b    EqCopPipeClientTest                    VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  A15    EqCopSunaNullTest                      stillgelegt, nicht beurteilt
  A16    EqCopProbeeqNullTest                   VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B1     EqCopIdentityTest                      VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B2     EqCopStateMigrationTest                VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B3     EqCopHostContextTest                   frisch                         eq-copilot/build/plugin/EqCopHostContextTest.vcxproj (2026-08-31 14:48:12)
  B3b    EqCopHostProbeTest                     frisch                         eq-copilot/build/plugin/EqCopHostProbeTest.vcxproj (2026-08-31 14:48:12)
  B3c    EqCopSchemaTest                        VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B4     EqCopQueueStressTest                   VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B9     EqCopLoudnessGoldenTest                frisch                         eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp (2026-09-07 04:41:30)
  B5     EqCopAnalysisGoldenTest                VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B6     EqCopDspGoldenTest                     VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B7     EqCopTransactionTest                   VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B10    EqCopIpcTest                           VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B11    EqCopSonde012HostChannelContextTest    VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B12    EqCopSonde012LoudnessSourceTest        VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B13    EqCopSonde012SourcesModelTest          VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B14    EqCopSonde012ProjectReloadTest         VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B15    EqCopShot                              VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B20    EqCopSonde013QualityClassTest          VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B22    EqCopSonde013FingerprintGoldenTest     VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B23    EqCopSonde013PassageStateTest          VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B24    EqCopSonde013PrePostGoldenTest         VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B25    EqCopSonde013ExperimentGoldenTest      VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B26    EqCopSonde013ExperimentUiTest          VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B21    EqCopSonde013InterventionRingTest      VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B19    EqCopSonde013StereoGoldenTest          VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B17    EqCopSonde013TruePeakGoldenTest        VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B18    EqCopSonde013DynamicsTest              VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B16    EqCopSonde013EventWireTest             VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B27    EqCopSonde014IntentTest                VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B29    EqCopSonde014AssistentTest             VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B30    EqCopBriefkastenTest                   VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B28    EqCopSonde014BefundTest                VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)
  B8     EqCopLebenslaufTest                    VERALTET                       eq-copilot/plugin/dsp/DspKern.cpp (2026-09-18 22:07:28)

  Konfiguration: 7 Stempel, 29 Eingaben der Arbeitskopie, keine ausstehend

[Kreuzprobe] gebundene Ziele laut CMake-Export: 40 (2 statisch ueber ihre Endziele); gedeckt 38, NICHT GEDECKT 0, nicht gebaut 0, nicht im Kanon 2, nicht ableitbar 0; 21 Kernquellen
  EqCopPipeProbe                         nicht im Kanon, nicht beurteilt
  EqCopPaintBench                        nicht im Kanon, nicht beurteilt

[Inventar] 158 getrackte Dateien unter eq-copilot/plugin mit Endung .cpp .h .hpp .c .inl .in: im Frischebaum 149, ausgenommen mit Grund 9, Luecken 0
  ausgenommen: eq-copilot/plugin/hostprobe/HostProbeFactory.cpp - nur das VST3-Ziel EqCopHostProbe, das der Kanon nicht baut (B3b misst EqCopHostProbeTest)
  ausgenommen: eq-copilot/plugin/probe/PipeProbeMain.cpp - nur EqCopPipeProbe, ein Handwerkzeug ausserhalb des Kanons
  ausgenommen: eq-copilot/plugin/spike/AuxSpikeEditor.cpp - nur EqCopAuxSpike und EqCopAuxSpikeTest, nicht im Kanon (NAK-37)
  ausgenommen: eq-copilot/plugin/spike/AuxSpikeEditor.h - nur EqCopAuxSpike und EqCopAuxSpikeTest, nicht im Kanon (NAK-37)
  ausgenommen: eq-copilot/plugin/spike/AuxSpikeFactory.cpp - nur EqCopAuxSpike und EqCopAuxSpikeTest, nicht im Kanon (NAK-37)
  ausgenommen: eq-copilot/plugin/spike/AuxSpikeProcessor.cpp - nur EqCopAuxSpike und EqCopAuxSpikeTest, nicht im Kanon (NAK-37)
  ausgenommen: eq-copilot/plugin/spike/AuxSpikeProcessor.h - nur EqCopAuxSpike und EqCopAuxSpikeTest, nicht im Kanon (NAK-37)
  ausgenommen: eq-copilot/plugin/tests/AuxSpikeTestMain.cpp - nur EqCopAuxSpikeTest, nicht im Kanon (NAK-37)
  ausgenommen: eq-copilot/plugin/tests/PaintBenchMain.cpp - nur EqCopPaintBench, ein Messwerkzeug ausserhalb des Kanons

Baustand: NICHT FRISCH (Exit 4)
BAUSTAND-JSON {"exit": 4, "veraltet": true, "nicht_ableitbar": false, "luecken": [], "grund": "", "konfiguration_ausstehend": [], "binaries": [{"kuerzel": "A1", "name": "EqCopNullTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "A2", "name": "EqCopGoldenTest", "stand": "frisch", "juengste_eingabe": "eq-copilot/plugin/core/analysis/TruePeak.h", "juengste_zeit": "2026-09-15 16:49:22", "grund": ""}, {"kuerzel": "A3", "name": "EqCopMarkierungTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "A4b", "name": "EqCopPipeClientTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "A15", "name": "EqCopSunaNullTest", "stand": "stillgelegt, nicht beurteilt", "juengste_eingabe": "", "juengste_zeit": "", "grund": ""}, {"kuerzel": "A16", "name": "EqCopProbeeqNullTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B1", "name": "EqCopIdentityTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B2", "name": "EqCopStateMigrationTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B3", "name": "EqCopHostContextTest", "stand": "frisch", "juengste_eingabe": "eq-copilot/build/plugin/EqCopHostContextTest.vcxproj", "juengste_zeit": "2026-08-31 14:48:12", "grund": ""}, {"kuerzel": "B3b", "name": "EqCopHostProbeTest", "stand": "frisch", "juengste_eingabe": "eq-copilot/build/plugin/EqCopHostProbeTest.vcxproj", "juengste_zeit": "2026-08-31 14:48:12", "grund": ""}, {"kuerzel": "B3c", "name": "EqCopSchemaTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B4", "name": "EqCopQueueStressTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B9", "name": "EqCopLoudnessGoldenTest", "stand": "frisch", "juengste_eingabe": "eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp", "juengste_zeit": "2026-09-07 04:41:30", "grund": ""}, {"kuerzel": "B5", "name": "EqCopAnalysisGoldenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B6", "name": "EqCopDspGoldenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B7", "name": "EqCopTransactionTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B10", "name": "EqCopIpcTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B11", "name": "EqCopSonde012HostChannelContextTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B12", "name": "EqCopSonde012LoudnessSourceTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B13", "name": "EqCopSonde012SourcesModelTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B14", "name": "EqCopSonde012ProjectReloadTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B15", "name": "EqCopShot", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B20", "name": "EqCopSonde013QualityClassTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B22", "name": "EqCopSonde013FingerprintGoldenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B23", "name": "EqCopSonde013PassageStateTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B24", "name": "EqCopSonde013PrePostGoldenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B25", "name": "EqCopSonde013ExperimentGoldenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B26", "name": "EqCopSonde013ExperimentUiTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B21", "name": "EqCopSonde013InterventionRingTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B19", "name": "EqCopSonde013StereoGoldenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B17", "name": "EqCopSonde013TruePeakGoldenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B18", "name": "EqCopSonde013DynamicsTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B16", "name": "EqCopSonde013EventWireTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B27", "name": "EqCopSonde014IntentTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B29", "name": "EqCopSonde014AssistentTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B30", "name": "EqCopBriefkastenTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B28", "name": "EqCopSonde014BefundTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}, {"kuerzel": "B8", "name": "EqCopLebenslaufTest", "stand": "VERALTET", "juengste_eingabe": "eq-copilot/plugin/dsp/DspKern.cpp", "juengste_zeit": "2026-09-18 22:07:28", "grund": ""}], "kreuzprobe": [{"name": "EqCopilot", "typ": "STATIC_LIBRARY", "status": "gedeckt", "detail": ""}, {"name": "NakamaProbeeq", "typ": "STATIC_LIBRARY", "status": "gedeckt", "detail": ""}, {"name": "EqCopProbeeqNullTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde012HostChannelContextTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde012LoudnessSourceTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopBriefkastenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSessionSoak", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde012SourcesModelTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde014AssistentTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde014BefundTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde012ProjectReloadTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde012SourcesLatencyTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopPipeProbe", "typ": "EXECUTABLE", "status": "nicht im Kanon, nicht beurteilt", "detail": ""}, {"name": "EqCopPipeClientTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopNullTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopMarkierungTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopShot", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopPaintBench", "typ": "EXECUTABLE", "status": "nicht im Kanon, nicht beurteilt", "detail": ""}, {"name": "EqCopIdentityTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopStateMigrationTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013ExperimentGoldenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013ExperimentUiTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013PrePostGoldenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013PassageStateTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde014IntentTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopLebenslaufTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopQueueStressTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopIpcTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopIpcLast", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013EventWireTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013DynamicsTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013FingerprintGoldenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013InterventionRingTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013QualityClassTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013StereoGoldenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSonde013TruePeakGoldenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopAnalysisGoldenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopDspGoldenTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopTransactionTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}, {"name": "EqCopSchemaTest", "typ": "EXECUTABLE", "status": "gedeckt", "detail": ""}], "juengste_aller_baeume": "2026-09-18 22:07:28"}
```

</details>

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|

### Rohe Ausgaben

