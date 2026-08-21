# Änderungssatz 1 — Sicherheit und Messkorrektheit (Befund)

**Datum:** 2026-08-14 · **Commits:** `41e9424` (P0 Rust) + `dca9a40` (Engine m3)
**Auftrag:** `docs/archiv/EQ-COPILOT-TECHNIK-UX-UEBERGABE.md` (Archiv) §15 „Änderungssatz 1" (Pakete A, B, C §8.2, D)
**Metrics-Version:** `m1-2026-08-13` → **`m3-2026-08-14`** (Messsemantik geändert — nie still)

---

## 1. Was gebaut wurde

### P0 — Rust-FFI (`plugin-hub-app/src-tauri/src/eq_copilot/server.rs`)

| Befund der Übergabe | Fix |
|---|---|
| `Vec<u8>` als `*const TOKEN_USER` dereferenziert (Alignment 1 vs. 8 = UB) | `Vec<u64>`-Puffer + Kompilezeit-Riegel `align_of::<TOKEN_USER>() <= align_of::<u64>()` |
| `CloseHandle` überschrieb `GetLastError` vor der Fehlerformatierung | Fehlercode wird SOFORT nach jedem Win32-Aufruf gesichert |
| Thread-Spawn-Fehler leakte die erste Pipe-Instanz | `HandleGuard` (RAII) für Token- und Pipe-Handles; `uebernehmen()` übergibt Ownership an Thread/File |
| unsafe-Blöcke und `unsafe impl Send` unbegründet | überall konkrete `// SAFETY:`-Begründungen |
| Kleinwarnungen (doppeltes `cfg(windows)`, `&mut attrs`, unnötiges `mut`, `contains`) | bereinigt |

Neuer Test: `user_sid_ist_wohlgeformt` (SID-Format `S-1-…` über den alignment-sicheren Pfad).

### Paket B — Stereo-/Spektralanalyse (`AnalyseEngine`)

Spektrum, Segment-Aktivgate, §5.10.2-Aktivität und Crest-RMS rechnen auf der
**mittleren Kanalenergie** `(PSD_L+PSD_R)/2` bzw. `0.5·(L²+R²)` statt des
Mid-Mix `0.5·(L+R)`. Vorher löschte sich Antiphase-/Side-Material komplett aus
und galt als „keine Daten" — für den Stil des Users (breite Chöre, Hallfelder,
Pads) der kritischste Messfehler. `width`/`corr` bleiben echte M/S-Maße
(`summeMono2` heißt jetzt ehrlich `summeMid2`). Bei L==R ist die neue Rechnung
**bitidentisch** zur alten (0.5·(x+x)=x, FFT deterministisch) — deshalb bleibt
analyze-track der exakte Kreuzvalidierungs-Maßstab (s. Fixtures).

### Paket C §8.2 — NaN/Inf-Robustheit

Die **Engine** (nicht der Audiothread) ersetzt nicht-endliche Samples in ihrer
Analysekopie durch Stille und zählt sie: `MessSnapshot.nanErsetzt` (seit
Messstart; Reset nullt), `nan_ersetzt_samples` in der Snapshot-Datei
(+ Schema), Statuszeile nennt beide Zeiträume („gesehen" = seit Pluginstart,
Zähler = seit Messstart). Akkus/Biquads/EMA können damit nicht mehr vergiftet
werden; das Audio bleibt unberührt (Passthrough = Nichtstun). Ort der
Sanitisierung ist bewusst die Engine: derselbe Riegel gilt für Plugin UND
Golden-/Direktfütterungs-Tests, und der Audiothread bleibt unverändert schlank.
Ein zusätzlicher „Filterzustands-Heiler" wäre toter Code: bei endlichen
Eingaben (float ≤ 3.4e38, quadriert als double) kann die Kette kein NaN/Inf
mehr erzeugen — die Recovery IST die Eingangs-Sanitisierung (Test `nan-recovery`).

### Paket D — Nyquist-Gültigkeit

LTAS (Komposit, Referenzachse, Live-EMA) endet bei
`min(18 kHz, 0.95·Nyquist)`: Befüllung, Interpolation und Anzeigekurve kappen
dort, darüber bleibt **NaN = „nicht messbar"** — vorher wurde der letzte
Randwert fortgeschrieben (erfundene Evidenz bei fs < ~37,9 kHz). Teilbänder,
die Nyquist überlappen, zählen ebenfalls als nicht messbar (ein
Rest-Bin-Pegel wäre kein Bandwert). Resonanz-Glättung ist NaN-fest
(−∞-Ersatz: nie Kandidat, blockiert aber kein echtes Randmaximum). Bei
fs ≥ 44,1 kHz liegt die Grenze über 18 kHz ⇒ Verhalten unverändert.
Centroid/Rolloff bleiben bewusst auf der vollen analyze-track-Achse
(samplerate-abhängig wie der Maßstab selbst — dokumentierte Eigenschaft,
keine stille Abweichung von der Kreuzvalidierung).

### Fixtures v2 + adversariale Tests

Alle 7 WAVs sind jetzt **kanalidentisch (L==R)**: analyze-track misst spektral
den Mid-Mix — nur bei L==R ist Mid ≡ Kanalenergie und die eingefrorene
Referenz ein exakter Maßstab (das alte dekorrelierte Pink hätte nach der
Korrektur ~3 dB daneben gelegen; die Übergabe verbot genau dieses
Konservieren). Referenz neu eingefroren (`golden-referenz.json`; pink jetzt
width 0.0/corr 1.0). Die Stereo-Korrektheit beweisen 6 neue Fälle im
GoldenTest mit **mathematisch zwingenden Prädiktionen** aus dem Pink-Fixture:

| Fall | Erwartung (alle bestanden) |
|---|---|
| `stereo-antiphase` (R=−L) | messbereit, LTAS/LUFS/Crest/TP exakt wie Referenz, corr −1, width side-dominant — vorher „keine Daten" |
| `stereo-nur-links` / `-rechts` | Kurve+LUFS = Referenz −3,01 dB, Crest +3,01, TP unverändert, width 1 |
| `stereo-dekorr` (R = 5 s rotiertes L) | Kurve = Referenz, corr ≈ 0, width ≈ 1 (der Hall-/Chor-Fall) |
| `nan-recovery` (Sekunde 5–6 NaN) | messbereit, Zähler exakt 96 000, alle Werte endlich, Kurve erholt |
| `nyquist-32k` | unterhalb der Kappe lückenlos, oberhalb nur NaN, kein Kandidat oberhalb |

## 2. Beweise (alle selbst ausgeführt, 2026-08-14)

| Prüfung | Ergebnis |
|---|---|
| `EqCopNullTest` | **NULLTEST OK** (bittransparent, NaN-Durchreichung unverändert) |
| `EqCopGoldenTest` | **GOLDEN OK 178/178** (vorher 135; +43 aus den neuen Fällen) |
| pluginval `--strictness-level 8` | **SUCCESS** |
| Pipe E2E (frischer Broker-Build) | **PROBE OK v2 · KONFLIKT OK · KONFLIKT-ENDE OK** |
| `cargo test --lib eq_copilot` | **36/36** (neu: `user_sid_ist_wohlgeformt`) |
| `cargo clippy -W undocumented_unsafe_blocks -W cast_ptr_alignment` | im eq_copilot-Scope warnungsfrei |
| `cargo test` (voller Workspace) | **734/735** — der eine Fehler ist der dokumentierte fremde Vorbefund `scanner::tests::scans_real_db` („3x Osc"; parallele Harness-Baustelle, liest die echte FL-DB). `bausteine::scan::…kindprozess…` war diesmal grün (lastabhängig). |
| `npm run check` / `npm run build` | 0 Fehler/0 Warnungen · Build OK |

## 3. Bewusste Abgrenzungen (nicht vergessen, nur verschoben)

- **Mid-/Side-LTAS als eigene Kurven, bandweise Kohärenz, Low-End-Mono-Check**
  (Paket B „zusätzlich sinnvoll"): kommen mit Diagnose v2 (Änderungssatz 3),
  wo sie Konsumenten haben (Masking §5.10.5) — jetzt wären es tote Felder.
- **Integrated-Loudness-Wachstum, Worker-Polling, Pipe-Deadlines/CancelIoEx**:
  Änderungssatz 2 (Übergabe §15).
- **Broker-Protokollvalidierung (`is_finite`, Wertebereiche)**: Paket F,
  Änderungssatz 2.
- `analyze-track.py` bleibt Mid-basiert: historischer Maßstab aller
  Stil-Profile des Users — eine Umstellung wäre ein eigener, sichtbarer
  Entscheid mit Re-Profilierungs-Folgen, kein Nebeneffekt dieses Satzes.

## 4. Offene Risiken

- Die Bitidentitäts-Aussage (L==R) ist mathematisch begründet und durch die
  engen Golden-Toleranzen (±0,3 LU, Kurve 0,5/2,0 dB) gestützt, aber nicht
  bit-für-bit gegen den m1-Stand diffbar (Referenz wurde neu eingefroren, das
  alte Pink war dekorreliert).
- `stereo-dekorr` nutzt zyklische Rotation (5 s): ein Segment enthält den
  Wrap-Sprung (~1/234 der Segmente, < 0,05 dB Effekt) — bewusst toleriert.
- ~~Live in FL noch nicht neu abgenommen~~ **Installiert 2026-08-14** per
  UAC-Helfer, Hash-Beweis von außen: DLL-SHA `9599EC6CA1EF7B5AD8E98765
  57415DB23134E0976D02F37D802B3C5382755CEC` == Staging, moduleinfo identisch.
  Die FL-Prüfschritte (§16 der Testanleitung) bleiben als Live-Abnahme offen.
