# eq-copilot — Plugin-Bauwurzel (Nakama)

Hier liegen Plugin (JUCE 8 + CMake), Verträge (v2 + v3), Identität, Fixtures,
Install-Skript (gitignoriert, nur auf dem Desktop) und die Material-Kit-Kette.
Produkt, Namen, Grundgesetz und Beweis-Kanon stehen in **einer** Quelle:
`../CLAUDE.md` (Wahrheitskern + Register der User-Entscheide). Dieses README
wiederholt sie nicht.

**Legacy-Name:** Bundle, Codes und Pipes heißen noch `EQ-Copilot` / `EqCop*`;
die Umbenennung zu „Nakama Studio" ist NAK-30 (Identitäts-Ticket), kein
Nebenbei-Refactor. Die kompilierte Material-Kit-Front ist ein **Provisorium**
(User 21.08.: „Nie abgenommen – bleibt Provisorium"); die neue UI aller drei
Apps kommt aus Figma über `design/` (Repo-Wurzel).

## Layout

```
eq-copilot/
  CMakeLists.txt   Bauwurzel; JUCE 8.0.9 gepinnt; Versions-Riegel (CMake == kPluginVersion)
  cmake/           NakamaBruecke.cmake (Quellhash-Gate des JUCE-Patches), FlatBuffers-Pin
  plugin/
    src/           Produkt: AnalyseEngine · Diagnose · PluginProcessor/-Editor · PipeClient · HoerMarkierung · AssetKit
    hostbridge/    Gegenseite des Bridge-Patches (JUCE-frei, 0 Allokationen)
    hostprobe/     Wegwerf-Messgerät Termin B (NkHp)
    spike/         Wegwerf-Messgerät Termin A, Aux/PDC (NkSp)
    vertrag/       v3-Vertragsengine C++ (JSON + FlatBuffers)
    tests/         Prüfbinaries (Null, Golden, Markierung, Identität, Hostkontext, HostProbe, Schema, AuxSpike, Shot, PaintBench, PipeProbe)
  schemas/         v2-Verträge des heutigen Plugins · v3/ Sondenfamilie (Bandgitter, Textriegel, FlatBuffers)
  identity/        plugin-identities-v1.json — eingefrorene Bundle-Identität
  fixtures/        Golden-Referenz, Identity-Goldens, v3-Korpus (JSON + Binär), Aux-Spike-Impulse
  design/          tokens.json → gen-tokens.mjs → plugin/src/LeitstandTokens.h (Provisorium) · archive/ · prisma-studie/
  docs/            Befunde M0–M3a, CS1, Benchmark-Studie, FL-Testanleitung, FL-Termine A/B, Hör-Markierungs-Konzept
  install/         (gitignoriert) Install-Skript + Rollback-Bundles
```

## Bauen und Prüfen

Der vollständige Bau- und Kanonbefehl steht in `../CLAUDE.md` („Bauen &
Beweisen"): `pwsh -File tools/beweise.ps1 -Bauen …` fährt alle 15 Beine und
schreibt die rohe Ausgabe ins Manifest. Einzelbefehle ebenfalls dort.

Artefakt: `build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3`.
Installation nur per User-Klick (`install/Install-EQ-Copilot.ps1` als Admin);
installiert ist zurzeit das Bundle vom 16.08.

Pipe Ende-zu-Ende (immer der eigene Probe-Pipename, nie die Produktion):

```powershell
# Terminal 1 (Broker-Probe, 30 s):
..\broker\target\release\eqcop-broker-probe.exe 30
# Terminal 2 (C++-Client-Probe gegen den Probe-Namen):
build\plugin\EqCopPipeProbe_artefacts\Release\EqCopPipeProbe.exe "\\.\pipe\evenacadia.eq-copilot.m2probe"
# Erwartet: PROBE OK v2 · KONFLIKT OK · KONFLIKT-ENDE OK (Exit 0)
```

## Meilensteine (Verlauf der EQ-Copilot-Linie bis 0.3.0 — Beweisgeschichte, keine Roadmap)

M0: Beweis-Spike — Passthrough-VST3, Named-Pipe-Handshake, Schemas, Tokens,
Nulltest + Probe (`docs/M0-BEFUND.md`).
M1: Einzelinstanz-Messung — AnalyseEngine (Mehrfachauflösung §5.10.1,
BS.1770-LUFS, True Peak, Abdeckung, Resonanzkandidaten), Editor-Graph,
lokaler Snapshot, Kreuzvalidierung **GOLDEN OK 88/88** (`docs/M1-BEFUND.md`).
**M2 (dieser Stand): Multi-Instanz + Projektbindung** — Protokoll v2
(Messstand im Heartbeat, heartbeat_ack mit Konflikt-Flag), Projektzeit-Fenster
mit Sprungzähler, PRE/POST-Paar-Auswertung mit ehrlicher Herabstufung (§5.7),
persistente Profilbindung, Aggregat-Snapshot (`schemas/eq-aggregat.schema.json`),
Sensorübersicht in der Hub-App, Konflikt-UI „neue Kennung" (§8.4),
Pipe-Squatting-Härtung (`docs/M2-BEFUND.md`). FL-Prüfliste:
`docs/FL-TESTANLEITUNG.md` §11–15. Danach M3 Ziele + deterministische
Diagnose (Plan §11).
**Änderungssatz 1 (2026-08-14, metrics `m3-2026-08-14`):** P0-Rust-FFI-Fix
(Alignment/RAII/SAFETY) · Spektrum+Aktivität+Crest auf Kanalenergie
(Antiphase-Fix) · NaN-Riegel mit Zähler · Nyquist-Kappe der LTAS · Fixtures
v2 (L==R) + 6 adversariale GoldenTest-Fälle — **GOLDEN OK 178/178**
(`docs/CS1-BEFUND.md`; Auftrag: `../docs/archiv/EQ-COPILOT-TECHNIK-UX-UEBERGABE.md`, Archiv).
**EQ-Copilot Material Kit (2026-08-15):** vollständige eigenständige
Vektorfront aus `plugin/src/EqCopilotAssetKit.h` (Tokens-Gruppe `copilot`; das
Vertragsblatt `design/ASSET-KIT.md` ist am 23.08.2026 auf User-Wort gelöscht —
„alt und längst verworfen"; Historie in git). Warme Metallfläche + schwarzes Analyseglas + sparsamer
Ember-Akzent; keine übernommenen Fremd-Assets und keine Fake-Potis. LUFS Short
und True Peak sind eigene Anzeigeschienen, Werkzeuge echte physische Tasten,
Kennzahlen getrennte Statuszellen. Jedes Bauteil ist unabhängig verschiebbar
und skaliert ohne Bitmap-Unschärfe; Fensterformat 750:520, Standard 1200:832.
Sichtprüfung ohne FL: `EqCopShot.exe <ziel.png> [breite]` rendert den Editor
mit echter 20-s-Messung offscreen als PNG.
**M3a + FPS-Fix (Nacht 15.08., metrics/diagnose m4-2026-08-15):** Sichtbare
Datenrate 4 Hz → ~20 Hz (`auswertenLeicht()` im Worker-Takt + Editor malt nur
bei neuer Snapshot-Revision, ruht im Leerlauf; `EqCopPaintBench` bewies
vorher, dass paint() mit ~2,4 ms unschuldig war). Kern-Mechaniken aus der
Benchmark-Studie (`docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md`): Median-Basislinie
±1/2 Okt statt Mittel, Zonen-Zeitverlauf (1-s-Aktiv-Ticks, geteiltes
`ZonenRegeln.h`) → Zonen-Karten sagen „in NN % der Musikzeit" und erreichen
ehrlich „hoch", Band-Perzentile P10/50/95 → Charakter-Zeile (ruhig ↔ Wellen),
Konvergenz-Anzeige **„KURVE STEHT"** (rein informativ), Notennamen in
FL-Zählung (116 Hz = A#3). Snapshot-Datei v3. Beweise: **GOLDEN OK 239/239**
(u. a. Halbzeit-Mulm ≈ 0,5 · wandernder Ton = zeitweise) · NULLTEST ·
pluginval 8 · SHOT (`docs/M3A-BEFUND.md`).
**Hör-Markierung (2026-08-16, Plugin 0.3.0, metrics/diagnose m4.1-2026-08-15):**
[Solo]/[Puls] je Befundkarte färben das Monitorsignal hörbar (Resonanz =
Bandpass-Kaskade, Zone = Butterworth-Paar; Puls im 250/375-ms-Raster,
Headroom-bewusst aus dem eigenen True Peak). Message-Thread baut den
kompletten Auftrag, Audiothread liest lock-frei aus dem 4-Slot-Ring;
Totmann-Timer 10 min, `Markierung aus` immer sichtbar; Analyse-Abgriff
sitzt VOR der Färbung. m4.1-Kalibrierfix: Zonen-Ticks brauchen ≥ −80 dB
je Region (vorher zählte Quasi-Stille als „Härte"-Zeit). Beweise:
**MARKIERUNGSTEST 30/30** (inkl. Freewheel/Render bitidentisch) · GOLDEN
weiter 239/239 (`docs/HOER-MARKIERUNG-KONZEPT.md`,
`kalibration/KALIBRIER-PROTOKOLL.md`).
