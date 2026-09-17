# Tiefenaudit 3 · Phase 15 · Paket 2 — DSP-Numerik und dynamische Übergänge

> **Rolle dieses Berichts:** Rohbericht des Paketprüfers 2 (unabhängiger lesender Agent)
> im Tiefenaudit 3, Phase 15 vom 17.09.2026, Quellstand `e008811efa7650f3e0cc2332cc21f9e268cdd125`.
> Er ist Rohmaterial: abgeleitete Parameterbereiche, Bewertung der Referenztests,
> Herleitungen, Gegenargumente, verworfene Verdachte und gelesener Scope. Verbindlich ist
> die Einordnung in [../BEFUNDE.md](../BEFUNDE.md), Abschnitt „Phase 15“, zusammen mit
> [phase-15-zentral.md](phase-15-zentral.md) (zentrale Quellprüfung, unabhängige
> Nachrechnung, ID-Zuordnung, Zähler) und
> [phase-15-gegenpruefung-paket2.md](phase-15-gegenpruefung-paket2.md)
> (Widerlegungsversuch durch einen anderen Prüfer, mit eigener Nachrechnung).
>
> Die Kennungen `P15-D-…` sind lokale Arbeitsnummern dieses Laufs, keine Befund-IDs.
> Kategorie- und Schwerevorschläge sind der Stand **vor** der Gegenprüfung; wo diese
> herabgestuft hat, gilt ihr Urteil (P15-D-02 → LÜCKE/low, P15-D-06 und P15-D-07 →
> HÄRTUNG/low).
>
> **Zu den Zahlen:** Alle „Nachrechnung“-Werte sind Nachrechnungen der aus der Quelle
> abgeschriebenen Formeln, **keine Produktausführung**. Die im Text genannten Skripte
> `p15-paket2-*.py` lagen im Arbeitsordner des Laufs und sind nicht Teil des Repos; die
> tragenden Zahlen sind in [phase-15-zentral.md](phase-15-zentral.md) mit einem dort
> vollständig abgedruckten, unabhängig geschriebenen Rechenprogramm reproduziert.


Datum 17.09.2026. Quellstand HEAD `e008811efa7650f3e0cc2332cc21f9e268cdd125` (vor und nach der Prüfung gleich; `git status --short` unverändert: nur die Audit-Dokumente sowie `briefing-hub/`, `nimbalyst-local/`). Rein lesend, keine Datei im Repo geändert, kein Bau, kein Test, kein Kanon, kein FL-/MCP-Aufruf.

**Ergebnis:** 7 Kandidaten (2 DEFEKT-Vorschläge, 5 LÜCKE; 4 medium, 3 low), 6 offene Prüffelder, 19 verworfene Verdachte.

**Evidenz-Hinweis:** Alle Zahlen mit „Nachrechnung" stammen aus eigenständigen Python-Skripten im Scratchpad. Sie sind eine **Nachrechnung der aus der Quelle abgeschriebenen Formel, keine Produktausführung**. Jedes Skript nennt im Kopf die abgeschriebenen Stellen `pfad:zeile`:

| Skript | Gegenstand |
| --- | --- |
| `p15-paket2-detektor.py` | Detektorpegel eines Sinus gegen den RMS-Pegel |
| `p15-paket2-detektorrauschen.py` | Detektorpegel von bandpassgefiltertem Rauschen |
| `p15-paket2-rampen.py` | globale Rampen beim Wiedereinschalten aus der Ruhe |
| `p15-paket2-stabil.py` | Kontraktion der SVF-Mischung, Frozen-Time-Stabilität der Biquad-Rampe, Shelf-Spitzen bei Q 24 |
| `p15-paket2-zwischenentwurf.py` | Betragsgang gemischter Zwischenentwürfe, Rauschspitzen |
| `p15-paket2-rampenburst.py`, `-burstcheck.py`, `-burstscan.py` | Frequenzsprünge statischer Bänder im Rampenübergang |
| `p15-paket2-svfsprung.py` | dieselben Sprünge auf dem dynamischen SVF-Weg |
| `p15-paket2-kaltstart.py` | Einschwingen unveränderter Bänder nach einem topologischen Crossfade |
| `p15-paket2-cramping.py` | RBJ gegen analogen Prototyp nahe Nyquist |
| `p15-paket2-monoautogain.py` | Auto-Gain auf einem Mono-Bus |
| `p15-paket2-autogaingrenze.py` | Grenzen des abgeleiteten Auto-Gains |
| `p15-paket2-puls.py` | Puls-Stufen der Hörmarkierung |

---

## 1. Abgeleitete zulässige Bereiche und Konfigurationen

### 1.1 Parameter: Vertrag gegen Code-Klemmung

| Größe | Vertrag (`eq-copilot/schemas/state/nakama-parameter-v2.json`) | Code-Tabelle / Host | Laufzeit-Klemmung, abgeleitete Wirkung |
| --- | --- | --- | --- |
| `freq_hz` | 20–20000 Hz (`:154-165`) | `NakamaParameter.cpp:64`; APVTS mit Skew (`SondeProcessor.cpp:102-108`); Hostwert per `jlimit` auf Vertrag (`SondeProcessor.cpp:1377-1383`) | Kappung auf min(Wert, 0,45·fs) beim Programmbau (`DspFilter.h:109-114`, `DspProgramm.cpp:214-215`); persistenter Wert bleibt (M-12). Wirksam: 44,1 kHz ≤ 19 845 Hz, 48 kHz ≤ 20 000 Hz (Deckel 21 600), 96/192 kHz ≤ 20 000 Hz. w0 höchstens 0,9·π. Kappung wird nicht gemeldet (bekannt NAK-254). |
| `q` | 0,15–24 für **alle** Typen (`:166-178`) | `NakamaParameter.cpp:65` | Auch Shelves, Notch, Cuts und der Detektor-Bandpass (`DspProgramm.cpp:255`) nutzen dieselbe Güte. RBJ-Shelves überschwingen ab Q > 0,707: bei ±12 dB und Q 24 Spitze +37,1 dB / Senke −25,1 dB, bei wirksam +24 dB +51,0 / −27,0 dB (Nachrechnung, `p15-paket2-stabil.py` Teil C). |
| `gain_db` | ±12 dB (`:179-191`) | `NakamaParameter.cpp:66` | Wirkt nur bei bell/low_shelf/high_shelf (`DspSvf.h:149-152`, `DspFilter.h:240-242`). Wirksame Summe `gain_db + g_dyn` ungeklemmt bis ±24 dB (R7 Feinheit 2, `docs/beweise/SONDE-015.md:1710`). |
| `dynamic_range_db` | ±12 dB (`:219-231`) | `:69` | `g_dyn = r·min(1, max(0, e_db − t)/12)` (`DspFilter.h:355-361`), damit auf ±∣r∣ begrenzt; bei r = 0 kein Detektor (`DspProgramm.cpp:245`), Entwurf trotzdem vorhanden (E-29, `:253-260`). |
| `threshold_db` | −60…0 dBFS (`:232-244`) | `:70` | Kennlinie gegen `leistungInDb` mit Stillegrenze −240 dB (`DspFilter.h:334-341`). |
| `attack_ms` / `hold_ms` / `release_ms` | 0,1–500 / 0–500 / 5–5000 ms (`:245-283`) | `:71-73` | Pole `exp(−1/(fs·τ))` (`DspFilter.h:273-278`), Hold `llround(ms·fs/1000)` bis 384 000 Samples bei 768 kHz (`:286`). Steuerrate 8 Samples (`DspProgramm.h:70`). |
| `channel_mode` | stereo/left/right/mid/side, topologisch (`:193-208`) | `:67` | je Band lokal (`DspKern.cpp:505-513`, `:594-601`). |
| `type` | 6 Werte, topologisch (`:136-152`) | `:63` | statisch RBJ-DF2T `double`; dynamisch bell/Shelves als TPT-SVF (`DspProgramm.cpp:220-227`); dynamisch notch/cuts laufen statisch, Auslenkung 0 (E-3). |
| `enabled` | bool, blockrand, **nicht** topologisch (`:125-134`) | `:62` | Wechsel ändert `aktiv` → topologischer Crossfade (`DspProgramm.cpp:152`). Wirkt per Hostautomation (`NakamaTransaktion.cpp:340-348`). |
| `occupied` | bool, topologisch, kein Host-Parameter (`:299-308`) | `NakamaParameter.cpp:86-87` | nur über Transaktion oder geladenen Zustand (Register NAK-306, `docs/offene-punkte.md:314`). |
| `input_trim_db` / `output_trim_db` | ±24 dB (`:38-63`) | `:50-51` | 0 dB ist Kurzschluss (`DspProgramm.cpp:184-185`), sonst Rampe 256 Samples. |
| `width` / `mono_bass_hz` | 0–2 / 0–500 Hz (`:64-89`) | `:52-53` | M/S-Stufe nur bei width ≠ 1 oder mono_bass > 0 (`DspProgramm.cpp:190`); mono_bass > 0: Low-Cut Q 1/√2 (`:191-192`); 0 ↔ > 0 Crossfade (E-20). |
| `mix`, `eq_enabled`, `auto_gain`, `bypass` | 0–1; bool (`:28-122`) | `:49`, `:79-81` | eq_enabled/bypass blockrand mit Crossfade; auto_gain nicht in `rampenKompatibel` → Gainrampe (mathematisch gleich einem Crossfade, V-09). |
| Auto-Gain-Betrag | nicht persistent; Wire `auto_gain_db` in [−120; 120] (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2324-2329`) | — | Ableitung ungeklemmt (`DspProgramm.cpp:85-136`); erreichbar −275 dB bis +150 dB (P15-D-05). |

### 1.2 Technikkonstanten

`kFadeSamples` 256, `kRampeSamples` 256 (`DspProgramm.h:53`, `:57`), `kDynamikSchritt` 8 (`:70`), `kKniebreiteDb` 12 (`DspFilter.h:347`), `kStilleDb` −240 (`:334`), `kDenormalSchwelle` 1e-300 (`:64`), `kNyquistAnteil` 0,45 (`:34`), Auto-Gain-Gitter 121 Stellen 20 Hz–20 kHz (`DspProgramm.h:75-77`).

### 1.3 Unterstützte Konfigurationen

- **Sampleraten:** `prepareToPlay` nimmt jede endliche Rate in (0; 768 000] (`SondeProcessor.cpp:237-238`); v3 `audio_lage.samplerate` gleich (`eq-ipc-v3.schema.json:163`). Gemessen (B6/A16) werden 44,1/48/96/192 kHz (`DspGoldenTestMain.cpp:569`, `SondeNullTestMain.cpp:221`). R4 Feinheit 1 setzt „kleinste Rate 44,1 kHz" voraus (`docs/beweise/SONDE-015.md:1620`) — nicht erzwungen (O-02).
- **Blocklängen:** jede > 0; über `maxBlock` gestückelt, Übernahme nur einmal je äußerem Aufruf (`DspKern.cpp:837-866`); v3 `block_size` 1–65 536.
- **Kanäle:** Mono→Mono und Stereo→Stereo (`SondeProcessor.cpp:311-333`); der Kern kennt zwei Kanäle, Mono dupliziert L (`DspKern.cpp:893-895`) und schreibt nur Kanal 0 (`:1079-1085`).
- **Übergangsarten:** Rampe nur bei gleicher Topologie aller aktiven Slots, gleicher Samplerate, gleicher Mono-Bass-Anwesenheit, beide engagiert (`DspProgramm.cpp:139-159`); sonst Crossfade in eine kalt publizierte Bank (`DspKern.cpp:262-271`, `:397-414`); ENDE-Marke für eq_enabled = false (`:237-249`, `:374-395`).
- **Hostautomation:** wirkt nur für nicht topologische Parameter (`NakamaTransaktion.cpp:340-348`, samplegenau aus: `SondeProcessor.cpp:155`), übernommen im 5-ms-Takt des Workers (`SondeProcessor.cpp:541-545`, `:1413-1443`; bekannt T3-01-05).
- **Heutiger FL-Handgriff:** belegt kein Band (NAK-306). Alle Band-Kandidaten gelten deshalb am gebauten Transaktions-/Zustandseingang; nach einer Belegung wirken kontinuierliche Bandwerte und `enabled` auch per Hostautomation.

---

## 2. Gelesener Scope

### 2.1 Körperlektüre (vollständig gelesen)

| Datei | Zeilen |
| --- | --- |
| `AGENTS.md`; `CLAUDE.md` (im Kontext) | vollständig |
| `eq-copilot/plugin/dsp/DspFilter.h` | 1–369 |
| `eq-copilot/plugin/dsp/DspSvf.h` | 1–166 |
| `eq-copilot/plugin/dsp/DspKern.h` | 1–451 |
| `eq-copilot/plugin/dsp/DspKern.cpp` | 1–1104 |
| `eq-copilot/plugin/dsp/DspProgramm.h` / `.cpp` | 1–222 / 1–272 |
| `eq-copilot/plugin/dsp/DspBankPool.h` / `.cpp` | 1–309 / 1–285 |
| `eq-copilot/plugin/dsp/DspRtWache.h` | 1–157 |
| `eq-copilot/plugin/state/NakamaParameter.h` / `.cpp` | 1–233 / 1–701 |
| `eq-copilot/plugin/state/NakamaTransaktion.h` / `.cpp` | 1–393 / 1–820 |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp` | 1–1446 |
| `eq-copilot/plugin/src/HoerMarkierung.h` | 1–816 |
| `eq-copilot/schemas/state/nakama-parameter-v2.json` / `-v1.json` | 1–506 / 1–69 |
| `eq-copilot/plugin/tests/DspGoldenTestMain.cpp` | 1–600, 735–1117, 1120–1553, 1556–2135, 2380–2428, 2429–2436, 2900–2990, 3985–4060, 4400–4586 |
| `eq-copilot/plugin/tests/SondeNullTestMain.cpp` | 380–440 |
| `eq-copilot/plugin/tests/MarkierungTestMain.cpp` | 1440–1475 |
| `eq-copilot/schemas/v3/eq-ipc-v3.schema.json` | 158–167, 2300–2345 |
| `docs/beweise/SONDE-015.md` | 184–316, 1000–1119, 1207–1226, 1551–1576, 1599–1632, 1687–1760, 2085–2154, 2951, 3388–3500 |
| `docs/FL-Nakama-Sonden-Design-Entwurf.md` | 2970–2995 (dazu :3109 per Suche) |
| `docs/beweise/NAK-283.md` | 220–245, 1680–1700 |
| `docs/audits/2026-09-15-tiefenaudit/roh/phase-14-blindspots.md` | 1–136 |
| `docs/audits/2026-09-15-tiefenaudit/roh/phase-01-echtzeit.md` | 296–365 |
| `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` | 1–342 vollständig; Befundzeilen 343–606 als gekürzte Spaltenauszüge |
| `tools/eq-copilot/erzeuge_envelope_fixtures.py` | 1–25 (kein DSP-Referenzerzeuger) |

### 2.2 Gezielte Suchen (keine Körperlektüre)

- Rohberichte Phase 01/02/04/05/06/09/10/11 nach DSP-Begriffen (Kern, Filter, SVF, Biquad, Nyquist, Steuerrate, Hüllkurve, Auslenkung); `docs/offene-punkte.md` nach DSP-Begriffen, dazu Volltext NAK-254, NAK-283, NAK-306; `docs/audits/2026-09-10-code-review` (AUDIT.md, `packets/audio-dsp.md`) nach Detektor/Filter.
- Repo-weit: `gekapptFreq`/`freqHzWirksam`; `Decramp`/`Orfanidis`/`cramping`/`matched-analog`; `auslenkungenDb`/`band_dynamic_gain`; `auto_gain_db`/`autoGainDb`; Welligkeit/Quadratur/Stale-Rampe/DF2T in Audits.
- `TransactionTestMain.cpp` (Prüfungsliste), `SondeNullTestMain.cpp` (Prüfungsliste), `MarkierungTestMain.cpp` (Prüfungsliste), `DspGoldenTestMain.cpp` 4180–4400 (Konfiguration Abschnitt N), `eq-copilot/identity/host-capabilities-fl-v1.json` (Kanäle), `tools/eq-copilot/` (Inventar).
- **Nicht gelesen:** `NakamaState.cpp` (Ladeweg nur über NAK-306 und `SondeProcessor.cpp:1047-1063`), `NakamaPreset.*`, übrige Testkörper, JUCE-Wrapper, Broker.

---

## 3. Bewertung der Referenztests

| Test (Ort) | Referenz: Unabhängigkeit | Tatsächlich geprüfte Grenzen/Übergänge | Blind für dieses Paket |
| --- | --- | --- | --- |
| B6 C, M-10/M-11 (`DspGoldenTestMain.cpp:1120-1177`, Referenz `:182-253`) | Im Test eigenständig ausgeschriebene RBJ-Formel, Messung am Tap per Impulsantwort. Textlich unabhängig, gleiche Formelfamilie (RBJ digital). | 7 statische Prüflinge (Bell 1 kHz Q 1 +6; Bell 1 kHz Q 12 +12; LS 200 Q 0,707 +6; HS 5 kHz Q 0,707 −6; Notch 1 kHz Q 4; LC 100; HC 8 kHz) × 44,1/48/96/192 kHz, Gitter 1/24 Okt bis min(20 kHz; 0,45 fs). | Keine Eckfrequenz 20 Hz oder nahe 0,45 fs, kein Q 0,15/24, keine Shelf-Überschwinger, kein Vergleich mit dem analogen Prototyp (P15-D-06). |
| B6 C1 (`:1179-1206`) | Sinus gegen Impulsantwort und Referenz | ein Bell, drei Stellen, 48 kHz | — |
| B6 C2 (`:1208-1277`) | dieselbe RBJ-Referenz | 4 dynamische Ruhekonfigurationen × 4 Raten; −24-dB-High-Shelf im Plateau bei 48 kHz | keine Übergänge |
| B6 M-12 (`:1279-1312`) | Programmfeld | Kappung 20 kHz bei 44,1 kHz; 96 kHz ohne Kappung | Meldung nach außen (NAK-254) |
| B6 D, M-15 (`:1324-1406`) | Geordnete Kette im Test nachgerechnet (eigener Text) | Bell left + mid bei 48 kHz | — |
| B6 E, Output-Trim (`:1408-1480`) | Analytische Ideallinie, unabhängig | +6 dB Output-Trim, 48 kHz, Block 64, Wechsel am Blockrand | globale Rampen beim Wiedereinschalten aus der Ruhe (P15-D-01) |
| B6 E, Bandwert-Rampen (`:1482-1552`) | **Nicht unabhängig vom Verfahren:** Referenz ist derselbe DF2T mit linearer RBJ-Koeffizientenmischung und Zustandsübernahme. | nur Bell; gain 3→9 dB, freq 1000→1400 Hz, Q 1→3; 48 kHz; Block 64; Wechsel bei Sample 2048 | große Frequenzsprünge, Shelves, andere Raten, Vergleich mit parameterstetigem oder SVF-Weg (P15-D-03) |
| B6 F, M-19/M-20/M-22/M-23 (`:1560-1655`) | direkte Zustandsprüfung | 1e-6-Sinus (rund 110 dB unter Threshold −10 dB), Range 0, none, priority | Detektorpegel knapp unter Threshold (P15-D-02) |
| B6 F, M-18 Plateau (`:1657-1689`) | Sollwert Range | L = R-Sinus weit über Threshold+Knie | im Plateau pegelunempfindlich (P15-D-02) |
| B6 F, M-18 Knie (`:1691-1728`) | analytische Kennlinie | **Quadraturton** (konstante Momentanleistung), Attack = Release = 500 ms, Hold 0 | RMS-Treue bei schwankender Leistung; Bias hier konstruktionsbedingt 0 (P15-D-02) |
| B6 F, M-21 (`:1730-1776`) | relative Werte | 1 kHz gegen 60 Hz; −12-dB-Band vor/nach | — |
| B6 F, M-25 und G, M-29 (`:1778-1828`, `:2382-2428`) | zweite Differenz gegen ruhigen Lauf | je **ein** schnelles Band (900 Hz Q 2 bzw. 1 kHz Q 1), 512 Samples nach dem Wechsel | Einschwingen weiterer, unveränderter Bänder (P15-D-04) |
| B6 F, M-26 (`:1850-1892`, `:1894-1970`) | relativ bzw. Zeitmarken | Hold-Vergleich nur ∣a2∣ > ∣a1∣ + 0,5; Sprungantwort mit **Quadraturton**, A/H/R 20/30/50 ms, Toleranz 1 ms, 4 Raten | Attack 0,1 ms und Steuerrate (P15-D-07); Pegel-Bias |
| B6 F, W-2 (`:1972-2059`) | Idealrampe mit derselben Ballistik (eigener Text) | Quadraturton, 48 kHz, Block 64 | — |
| B6 F, W-3 (`:2061-2113`) | Idealrampe der Range | nur **Steuerphase 0** (Blocklänge 8 ab Vielfachem von 8), 48 kHz, Attack 0,1 ms | Phasen 1–7, gemischte Blocklängen (O-05) |
| B6 B, M-03/M-04/M-06/M-07 (`:736-1117`) | Sprunggrenze, Bitgleichheit; `kaltSchranke` (`:486-517`) aus eigener RBJ-Referenz | Einschalten aus **frischer** Ruhe; M-07 mit **identischem** Programm und anschließender Stille | geänderte globale Werte zwischen Aus und Ein (P15-D-01); Kaltstart unveränderter Bänder (P15-D-04) |
| B6 H, Auto-Gain (`:2433-2621`) | analytische Sollwerte ±0,1 dB | flach, 0-dB-Bell, +6-dB-Shelf, Mid/Side-Paar, Dynamik, Output-Trim; 48 kHz | Extremwerte und Vertragsgrenze ±120 dB (P15-D-05); Mono (O-01) |
| B6 J (`:2900-2990`) | Zähler | Allokationen/Sperren über 4000 Blöcke inkl. Aus/Ein mit identischem Programm | keine Klangprüfung |
| B6 M-121 (`:4010-4059`) | Sollwert 0 | Remove, Audio, Neubelegung mit statischem Notch | Übergänge ohne Audio dazwischen (bekannt T3-14-02) |
| B6 N, M-118 (`:4180-4400`) | Messwert | acht dynamische Bänder, Last | keine Numerik |
| B6 O, M-120 (`:4408-4570`) | derselbe Code zweimal (Zeitunabhängigkeit, keine Formelreferenz) | feste Blockfolge 256/128/480/64/512/1/333/256/200/17 | — |
| B7 `TransactionTestMain.cpp` | Transaktionsprotokoll | Auslenkung 0 nach Remove (`:1170`), Klemmliste und Auto-Gain-Unterschied (`:1576-1593`); Offline verwirft 8 Blöcke (bekannt T3-01-04) | keine Filternumerik |
| A16 `SondeNullTestMain.cpp` | Bitvergleich | 4 Raten, normale Zufallsfloats; M-04 nur Ausschalten (`:396-415`) | Subnormale (bekannt T3-01-01); Wiedereinschalten |
| A3 `MarkierungTestMain.cpp` | Lebenslauf, NaN, Oversize; Stufe 0 b == a (`:1446-1465`) | Solo „greift" nur als Abweichung (`:1531`) | keine analytische Antwort der Hörmarkierung |
| Werkzeuge `tools/eq-copilot/` | — | kein Referenz- oder Fixture-Erzeuger für den DSP-Kern | — |

---

## 4. Kandidaten

### P15-D-01 — Globale Rampen überdauern das Ausschalten: beim Wiedereinschalten wird mit dem Trim des alten Programms eingeblendet

**Vorschlag:** DEFEKT (alternativ LÜCKE, falls M-07 eng auf Filter- und Hüllkurvenzustand gelesen wird) · **medium** · **K4** · Evidenz: Quellenherleitung plus Nachrechnung (`p15-paket2-rampen.py`).

1. **Auslöser und Ablauf** (jede Samplerate, jede Blocklänge, keine Bandbelegung nötig):
   (a) `v2.global.eq_enabled` = true, `v1.global.output_trim_db` = +24 dB → die Output-Rampe steht auf 15,849.
   (b) `eq_enabled` = false → ENDE-Marke → Crossfade in die Ruhe; „Die Rampen behalten ihre Ziele". Nach 256 Samples ist der Pfad in Ruhe, die Rampe steht weiter auf 15,849.
   (c) Bei ausgeschaltetem EQ `output_trim_db` = 0 dB → nur ENDE-Marken; `blockrand` kehrt bei `alt < 0` zurück, die Rampen bleiben unberührt.
   (d) `eq_enabled` = true → neue Bank aus der Ruhe → `setzeZiel (1,0)` startet beim Ist-Stand 15,849. Die Rampe läuft über 256 Samples auf 1,0, gleichzeitig blendet der Crossfade von Dry ein. Ausgang/Eingang = (1 − t) + t·g(n).
   Nachrechnung: Maximum 4,683 (**+13,4 dB**) bei n = 128. Altstand +12 dB: +4,8 dB. Altstand +24 dB mit Ziel −24 dB: +13,0 dB über dem Eingang, 37 dB über dem Endwert. Altstand −24 dB: Delle −2,3 dB. Dauer 5,8 ms bei 44,1 kHz, 1,3 ms bei 192 kHz. Gleiches gilt für Input-Trim, Mix, Width und Auto-Gain. Das erste Einschalten nach `bereiteVor` läuft monoton ohne Überschwinger.
   Variante (nicht gezählt): Candidate-Pfad nach Preview-Ende; nur im Tap `post_candidate` sichtbar, weil die Hörmatrix das Einblenden abwartet (E-34).
2. **Quellstellen:** `DspKern.h:315-323` (`setzeSofort`; `setzeZiel` rampt ab `aktuell`); `DspKern.cpp:85-93` (einzige Rückstellung auf 1,0 in `ruhe()`, gerufen nur `:110` und `:138`); `:374-395` (ENDE; `:379` `if (alt < 0) return;`; `:383-384` Rampen behalten Ziele); `:410-424` (`setzeZiel` beim Einschalten); `:818-824` (Übergangsende ohne Rampenrückstellung); `:182-185` (Host-Reset setzt `setzeSofort (r->ziel)` und erhält den Altstand); `:651-662`, `:709-731` (Anwendung je Sample); `:796-813` (Crossfade).
3. **Produkteinstieg:** Hostparameter `v2.global.eq_enabled`, `v1.global.output_trim_db`/`input_trim_db`/`width`, `v2.global.mix`/`auto_gain` — alle nicht topologisch (`NakamaParameter.cpp:49-53`, `:79-81`). Weg: `SondeProcessor.cpp:1296-1304` → `:1413-1443` → `NakamaTransaktion.cpp:340-348` → `:753-763`. In FL heute per Automation oder generischem Plugin-Fenster erreichbar. Zusätzlich die Transaktions-API.
4. **Verletzte Zusage:**
   - M-07 (`docs/beweise/SONDE-015.md:1040`): „Beim erneuten Einschalten starten alle Filter- und Hüllkurvenzustände auf 0 — die Sonde beginnt neu, sie setzt nicht fort."
   - R2 Feinheit 4 (`:1573`): „**Ausgeschaltet heißt kalt.** … beim Einschalten starten alle Zustände auf 0 (M-07)."
   - `CLAUDE.md` Grundgesetz: „Verarbeitung nur, wenn der User sie einschaltet." Der +24-dB-Trim ist weder bestätigt noch wirksam.
5. **Gegenweg und stärkstes Gegenargument:** Geprüft: erstes Einschalten nach `prepareToPlay`/`bereiteVor` (1,0, korrekt); Wiedereinschalten mit unverändertem Programm (`setzeZiel` ohne Wirkung, korrekt); Hard-Bypass-Wechsel laufen über Bänke und ziehen die Ziele nach (kein Altstand). Stärkstes Gegenargument: M-07 nennt ausdrücklich Filter- und Hüllkurvenzustände; die Rampen gehören dem Pfad (E-16). Die Wirkung bleibt auf 256 Samples begrenzt, und am Umschaltsample gibt es keinen Sprung, das M-03-Kriterium hält also. Entgegnung: R2 Feinheit 4 sagt „alle Zustände" und „setzt nicht fort"; hörbar wird ein nicht bestätigter Gain.
6. **Vorhandene Tests und warum sie nicht genügen:** `DspGoldenTestMain.cpp:736-779` (M-03 schaltet nur aus frischer Ruhe ein); `:1089-1117` (M-07 schaltet mit identischem Programm wieder ein und misst danach Stille, 0·g = 0 ist blind); `:2933-2951` (Lauf J: Aus/Ein mit identischem Programm, misst nur Allokationen und Sperren); `SondeNullTestMain.cpp:396-415` (nur Ausschalten). Kein Test ändert einen globalen Wert während der Ruhe.
7. **Minimaler Folgebeweis:** B6 am Kern und dieselbe Folge in A16/B7 am Prozessor. Frische Instanz, 48 kHz, Block 64. EQ an mit Output-Trim +24 dB, 2048 Samples. EQ aus, 1024 Samples. Output-Trim 0 dB (EQ aus), 64 Samples. EQ an; DC 0,5 über 512 Samples. Heute erwartbar: max ∣aus/ein∣ ≈ 4,68 bei n ≈ 128. Erfolgskriterium: max ∣aus/ein∣ ≤ 1 + 1e-6 im Einblendfenster (Ideal exakt 1,0). Gegenfall: dieselbe Folge ohne Schritt (a) bleibt schon heute 1,0.
8. **Abgrenzung:** Kein T3-Befund betrifft die Pfadrampen. T3-01-09 (VST3-Hostbypass umgeht den Kern, anderer Eintritt); NAK-283 F05/R-283-3 (`beendeAudiohistorie` setzt die Rampen auf ihr Ziel, konserviert damit genau den Altstand und heilt P15-D-01 nicht); T3-01-05 (Worker fasst Werte zusammen; hier braucht es getrennte Takte); T3-14-02 (Bandzustand, nicht Pfadrampen).

### P15-D-02 — Der Detektorpegel ist bei schwankender Momentanleistung kein RMS: die Kennlinie greift 2 bis 9 dB zu früh

**Vorschlag:** DEFEKT (alternativ LÜCKE, falls R7 „RMS" als Detektorfamilie statt als Pegeldefinition gelesen wird) · **medium** · **K3** · Evidenz: Nachrechnung (`p15-paket2-detektor.py`, `p15-paket2-detektorrauschen.py`) plus Quellenherleitung.

1. **Auslöser und Ablauf:** Dynamisches Bell 1 kHz Q 1, `sidechain_source` internal, Range −9 dB, Threshold −30 dB, 48 kHz. Eingang: 1-kHz-Sinus mit RMS −24 dBFS auf L = R (Kanalmodus stereo; jeder Einkomponentenmodus ebenso). Je Sample gilt: steigt die Momentanleistung über die Hüllkurve, folgt die Attack und der Hold wird neu geladen, sonst Hold oder Release. Die Leistungswelligkeit bei 2f treibt die Hüllkurve deshalb Richtung Spitzenleistung.
   Nachrechnung, Sinus (Soll-Auslenkung −4,50 dB):

   | A/H/R | e_db gegen RMS | Auslenkung |
   | --- | --- | --- |
   | 10/0/100 ms (Vertragsdefault) | +2,12 dB | −6,09 dB |
   | 1/0/20 ms | +2,42 dB | −6,31 dB |
   | 0,1/0/5 ms | +2,67 dB | −6,50 dB |
   | Hold ≥ halbe Periode (z. B. 10/100/100, B6-O-Satz 5/10/80, 20/30/50) | +3,01 dB | −6,76 dB |
   | Quadraturton oder Attack = Release, Hold 0 | 0,00 dB | −4,50 dB |

   Nachrechnung, bandpassgefiltertes Rauschen: Defaults +4,16 dB (L = R) bzw. +3,22 dB (L/R unabhängig); 1/0/50 ms +6,26/+4,91 dB; 10/50/100 ms +9,19/+7,43 dB; 50/0/50 ms 0,00 dB.
   M-19-Folge (aus derselben Nachrechnung abgeleitet): RMS 2 dB unter Threshold mit Hold 100 ms ergibt e_db = Threshold + 1,01 dB, also eine Auslenkung von −0,76 dB statt exakt 0,0.
2. **Quellstellen:** `DspFilter.h:258-265` (Kommentar „RMS … das waere kein RMS"); `:273-288`; `:312-328` (Tick vergleicht mit der Momentanleistung, Hold-Neuladung je Anstieg); `:336-341`; `:355-361`; `DspKern.cpp:539-553` (Momentanleistung d², E-5-Mittel); `:570-572` (Kennlinie auf `leistungInDb (z.huelle.leistung)`).
3. **Produkteinstieg:** Dynamisches Band über die Transaktions-API (`apply`, `bandBelegen`, `presetLaden`; `NakamaTransaktion.cpp:519-535`) oder einen geladenen Zustand (`SondeProcessor.cpp:1047-1049`). `dynamic_enabled`, `sidechain_source` und `occupied` sind topologisch bzw. kein Host-Parameter; heute kein FL-Handgriff (NAK-306). Danach sind threshold/range/attack/hold/release per Hostautomation erreichbar. Der Gen-Fernweg (S29–31) ist noch nicht angeschlossen.
4. **Verletzte Zusage:**
   - R7 (`SONDE-015.md:1696-1697`): „Detektor = RMS mit Attack/Hold/Release des Bandes"
   - M-18 (`:1061`): „wobei `e_db` der RMS-Pegel des Detektors nach Attack/Hold/Release ist"
   - M-19 (`:1062`), Pegel unter Threshold: „Die dynamische Verstärkung ist **exakt 0,0** — nicht „nahe null"."
   - R7 Feinheit 3 (`:1711`): „quadratischer Mittelwert über eine Ein-Pol-Hüllkurve …"
5. **Gegenweg und stärkstes Gegenargument:** Geprüft: symmetrische Ballistik und Quadratursignale sind biasfrei; das B6-Plateau hält. Stärkstes Gegenargument: Der Entwurf nennt „RMS-/Peak-Hüllkurven" (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2984`), M-26 (`SONDE-015.md:1069`) beschreibt die Stufen je Sample genau so, wie sie gebaut sind, und E-25 (`:3419`) kennt die „Detektorwelligkeit" eines einkanaligen Sinus. Ein asymmetrischer Ein-Pol auf x² ist verbreitete Praxis. Entgegnung: M-18/M-19 knüpfen Zahlen an den RMS-Pegel; die Abweichung hängt am Material (Crest, Korrelation) und erreicht bis 9 dB.
6. **Vorhandene Tests und warum sie nicht genügen:** `DspGoldenTestMain.cpp:1691-1728` (Knie mit Quadraturton und 500/0/500 ms, Bias konstruktionsbedingt 0; Begründung E-25 in `:420-424`); `:1657-1689` (Plateau, pegelunempfindlich); `:1894-1970` (Sprungantwort mit Quadraturton); `:1850-1892` (nur relativer Hold-Vergleich); `:1972-2059` (W-2: Quadraturton, Referenz mit derselben Ballistik — prüft die Umsetzung, nicht die RMS-Treue); `:1560-1585` (M-19 mit 1e-6-Amplitude, rund 110 dB unter Threshold −10 dB).
7. **Minimaler Folgebeweis:** B6 mit der Einstellung aus Punkt 1 und Vertragsdefaults. Eingang L = R-Sinus mit RMS −24 dBFS über 3 s, dann Auslenkung und Wirkung (`tonAmplitude`). Gegenfall: Quadraturton gleichen RMS-Pegels. Heute laut Nachrechnung −6,09 gegen −4,50 dB. Erfolgskriterium: Differenz ≤ 0,1 dB bei Defaults und bei Hold 100 ms; M-19-Probe mit RMS −32 dBFS und Hold 100 ms ergibt Auslenkung == 0,0. Alternativ, falls Spitzencharakter gewollt ist: Zusage ausdrücklich ändern und datiert entscheiden.
8. **Abgrenzung:** Kein T3- oder NAK-Punkt. Phase 06 hat den DSP nur an der Messkante gelesen (`roh/phase-06-messkern.md:128`); T3-06-01 und T3-06-02 betreffen die Analyse-Engines.

### P15-D-03 — Rampenübergang statischer Bänder (E-19): lineare RBJ-Koeffizientenmischung erzeugt bei großen Frequenzsprüngen Ausgangsspitzen weit über beiden eingeschwungenen Zuständen

**Vorschlag:** LÜCKE · **medium** · **K3** (mit K5-Anteil: die Referenz bildet das Verfahren nach) · Evidenz: Nachrechnung (`p15-paket2-zwischenentwurf.py`, `-rampenburst.py`, `-burstcheck.py`, `-burstscan.py`, `-svfsprung.py`) plus Quellenherleitung.

1. **Auslöser und Ablauf:** Statisches Band, gleiche Topologie, nur `freq_hz` springt zwischen zwei Blockrändern → `rampenKompatibel` → Zustand wandert mit, Koeffizienten werden je Sample linear gemischt (erstes Sample 1/256). Fall: 48 kHz, Low-Shelf +6 dB Q 0,707, 5 kHz → 50 Hz, Eingang 1-kHz-Sinus 0,5. Nachrechnung: Spitze 7,39, also **+17,4 dB** über der größeren stationären Spitze (1,0), rund 9 ms nach dem Wechsel (die Rampe endet nach 5,3 ms); nach 20 ms wieder unter 1,0. Die gemischten Zwischenentwürfe haben bei 1 kHz höchstens +9,7 dB; der Ausschlag entsteht zeitvariant, weil die Bedeutung der DF2T-Zustände an den Koeffizienten hängt.
   Weitere Nachrechnungen (dB über der größeren stationären Spitze; in Klammern ein parameterstetiger Neuentwurf je Sample):
   - 48 kHz: Low-Shelf −6 dB 5 kHz → 50 Hz, 1-kHz-Ton +15,9 (+0,6); 2 kHz → 100 Hz: +2,8 bis +5,0 (≤ +0,3); Low-Shelf +12 dB 20 kHz → 20 Hz, Rauschen +13,4 (−4,6); High-Shelf −12 dB 20 kHz → 20 Hz, Rauschen +20,2 (−7,1).
   - 192 kHz: Low-Shelf −6 dB 5 kHz → 50 Hz, 3-kHz-Ton +17,3 (+1,8); 2 kHz → 100 Hz, 1-kHz-Ton +9,4 (+1,5); Bell +6 dB Q 1 5 kHz → 50 Hz, 3-kHz-Ton +22,9 (+9,0).
   - Dieselben Sprünge auf dem dynamischen SVF-Weg: Shelves −0,0 bis +0,9 dB, Bell +2,9 bis +5,2 dB.
   - Aufwärtssprünge und Sprünge ≤ 1:5: ≤ rund 1–2 dB.
   - Frozen-Time ist jeder Zwischenentwurf stabil; das Stabilitätsargument von E-19 stimmt.
2. **Quellstellen:** `DspKern.cpp:39-52` (Biquad-`mische` mit E-19-Konvexitätsargument); `:397-408` (Zustand wandert mit); `:410-414` (Rampe 256 Samples); `:488-502` (t je Sample); `:586-591` (statischer Weg mit gemischtem Biquad); `DspFilter.h:90-96` (DF2T), `:116-167` (RBJ); `DspProgramm.cpp:139-159` (`rampenKompatibel` ohne Wertegrenze); `DspSvf.h:12-16` (eigene Begründung: RBJ-Zustand „beim Koeffizientenwechsel nicht stetig - genau das erzeugt das Knacksen").
3. **Produkteinstieg:** Hostautomation `v1.band.<n>.freq_hz` mit Sprung zwischen zwei 5-ms-Takten (`SondeProcessor.cpp:1421-1443`), sobald das Band belegt ist (Belegung heute nur über Transaktion oder Zustand, NAK-306). Transaktions-API: undo/redo/revert/presetLaden/apply mit gleicher Topologie (`NakamaTransaktion.cpp:519-574`, `:697`, `:725-731`).
4. **Zusage:** Außerhalb des definierten Tests keine wörtlich verletzte.
   - Entwurf §44.5 (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3109`): „Automations-Zipperresiduen bleiben im definierten Ramp-Test unter −100 dBFS."
   - R8 (`SONDE-015.md:1725-1726`): „kontinuierliche Werte über Rampen; Zipperresiduen < −100 dBFS (§44.5)."
   - Entwurf §44.2 (`:2990-2991`): „Stetige Parameter werden geglättet".
   - M-17 (`SONDE-015.md:1055`), Rotbeweisspalte: „oder zwei Wege glätten verschieden, sodass derselbe Zielwert je nach Quelle anders klingt".
   Einordnung LÜCKE: Es fehlt eine Regel für die Übergangsspitze großer Sprünge; der definierte Ramp-Test prüft nur kleine Sprünge gegen dieselbe Mischung. Statisches und dynamisches Band klingen beim selben Sprung verschieden.
5. **Gegenweg und stärkstes Gegenargument:** Geprüft: kleine Sprünge (B6-Fälle) ≈ 0 dB; Aufwärtssprünge unkritisch; SVF-Weg ohne Überschuss bei Shelves; E-19 war die Abhilfe gegen den Kaltstart-Crossfade (B-4). Stärkstes Gegenargument: Jeder schnelle Filtersweep erzeugt physikalisch Transienten — auch parameterstetig zeigen Bells +4 bis +12 dB —, und der definierte Test ist bestanden. Entgegnung: Bei Shelves geht der Überschuss eindeutig auf das Verfahren zurück (stetig und SVF ≈ 0 dB gegenüber +16 bis +17 dB).
6. **Vorhandene Tests und warum sie nicht genügen:** `DspGoldenTestMain.cpp:1482-1552`: Die Referenz ist derselbe DF2T mit linearer RBJ-Koeffizientenmischung und Zustandsübernahme, also nicht unabhängig vom Verfahren. Geprüft wird nur ein Bell mit gain 3→9 dB, freq 1000→1400 Hz, Q 1→3, bei 48 kHz, Block 64, Wechsel am Blockrand 2048. `:1408-1480` prüft nur den Output-Trim. Keine Shelves, keine Sprünge ≥ 1:5, keine anderen Raten, kein Vergleich mit dem SVF-Weg.
7. **Minimaler Folgebeweis:** B6: Low-Shelf +6 dB Q 0,707 bei 5 kHz, 48 kHz, 1-kHz-Sinus 0,5, 1 s einschwingen; Wechsel nur `freq_hz` → 50 Hz; 0,5 s Tap aufzeichnen. Heute erwartbar: Spitze ≈ 7,39. Erfolgskriterium: Spitze ≤ max(stationär vorher, stationär nachher) + 1 dB. Gegenfälle: 50 Hz → 5 kHz (heute ≈ 0 dB) und derselbe Sprung als dynamisches Band mit Range 0 (heute ≈ 0 dB); zusätzlich 192 kHz.
8. **Abgrenzung:** T3-14-02 (Zustandsübernahme trotz Lebenszyklusgrenze — anderer Mangel an derselben Stelle `DspKern.cpp:397-408`); T3-01-05 (der Worker fasst Hostwerte zusammen und vergrößert dadurch Sprünge — eigener Mechanismus); NAK-250 (Performance). Keine Registerzeile.

### P15-D-04 — Ein topologischer Wechsel an einem Slot startet ALLE Bänder kalt: unveränderte tieffrequente, hochgütige und dynamische Bänder brechen lange nach dem 256-Sample-Fade ein

**Vorschlag:** LÜCKE · **medium** · **K4** · Evidenz: Nachrechnung (`p15-paket2-kaltstart.py`) plus Quellenherleitung.

1. **Auslöser und Ablauf:** Slot 0 ist ein eingeschwungenes statisches Bell 50 Hz Q 8 +12 dB (50-Hz-Ton), Slot 5 ist belegt. Nur `v1.band.5.enabled` wechselt — ebenso Typ, Kanalmodus, `dynamic_enabled`, `sidechain_source` oder `occupied` eines anderen Slots. `rampenKompatibel` liefert false → Crossfade in eine kalt publizierte Bank → Slot 0 schwingt neu ein.
   Nachrechnung (Ton auf Bandmitte, 48 kHz):

   | Unverändertes Band | tiefster Periodenpegel | wieder innerhalb 1 dB |
   | --- | --- | --- |
   | Bell 50 Hz Q 24 +12 dB | −9,6 dB bei 40 ms | ab 600 ms |
   | Bell 50 Hz Q 8 +12 dB | −6,6 dB bei 40 ms | ab 220 ms |
   | Bell 100 Hz Q 4 / Q 2 +6 dB | −1,9 / −1,7 dB | ab 30 / 20 ms |
   | Bell 60 Hz Q 1 +9 dB | −1,5 dB | ab 33 ms |
   | Bell 1 kHz Q 2 +6 dB | −0,2 dB | ab 3 ms |

   Dynamisches Band im Plateau (Range −12, Threshold −30; Quadraturton, vereinfachte Hüllkurvenrechnung ohne Detektor-Bandpass): Attack 50 ms, Pegel 1 dB über Knieende → Auslenkung am Fadeende −3,1 statt −12 dB, 1-dB-Nähe erst nach rund 50 ms. Attack 500 ms, 12 dB über Knieende → −4,3 dB, rund 26 ms. Attack 10 ms → −9,2 dB, 10 ms.
2. **Quellstellen:** `DspKern.cpp:262-271` (neue Bank kalt); `:397-414` (Zustandsübernahme nur bei `nurRampen`, sonst Crossfade); `:796-813` (Crossfade 256 Samples); `DspProgramm.cpp:148-157` (die Abweichung irgendeines aktiven Slots macht den ganzen Wechsel topologisch); `DspBankPool.cpp:38-42` (`zustaendeNullen`).
3. **Produkteinstieg:** Hostautomation `v1.band.<n>.enabled` (nicht topologisch, `NakamaParameter.cpp:62`; wirkt über `wirksam()`, `NakamaTransaktion.cpp:340-348`) an einem belegten Slot; Transaktionen (Typ-/Moduswechsel, bandBelegen, remove, presetLaden). Die Bandbelegung selbst heute nur über Transaktion oder Zustand (NAK-306).
4. **Zusage:** Keine ausdrückliche Zusage zur Historie unveränderter Slots, daher LÜCKE. E-8 (`SONDE-015.md:3400`) begründet den Kaltstart: „Der Crossfade über 256 Samples deckt die Transiente ab; ein übertragener Zustand wäre ein geteilter Zustand zwischen zwei Bänken, und §44.2 verbietet das ausdrücklich." Die Begründung trägt nicht: (a) Die Einschwingzeiten von 20 bis 600 ms liegen weit über 5,3 ms. (b) §44.2 (`SONDE-015.md:228-229`, ebenso M-46 `:1099`) verbietet das Teilen „zwischen beiden Pfaden", nicht zwischen alter und neuer Bank desselben Pfads — E-19 überträgt genau so (`DspKern.cpp:400-408`). M-07 verlangt den Kaltstart nur für eq_enabled aus/an.
5. **Gegenweg und stärkstes Gegenargument:** Geprüft: Rampenwechsel übertragen den Zustand; schnelle Bänder sind kaum betroffen (1 kHz Q 2: −0,2 dB); das geänderte Band selbst darf einschwingen. Stärkstes Gegenargument: E-8 ist ein Entscheid; der Crossfade zwischen zwei vollständigen Programmen ist R8-Wortlaut; ein warm übernommener Zustand des geänderten Slots wäre falsch (vgl. T3-14-02). Entgegnung: Beanstandet sind nur unveränderte Slots.
6. **Vorhandene Tests und warum sie nicht genügen:** `DspGoldenTestMain.cpp:2382-2428` (M-29 mit einem einzigen Band 1 kHz Q 1, 512 Samples, zweite Differenz); `:1778-1828` (M-25 mit einem Band 900 Hz Q 2); `:1045-1087` mit `:486-517` (B-5: `kaltSchranke` akzeptiert das Kalteinschwingen des gewechselten Bandes); `:4180-4400` (Abschnitt N: acht Bänder, nur Last). Kein Fall mit einem zweiten, unveränderten, langsam einschwingenden Band.
7. **Minimaler Folgebeweis:** B6 bei 48 kHz: Slot 0 Bell 50 Hz Q 8 +12 dB; Slot 5 Notch 5 kHz Q 4, belegt und ausgeschaltet; 50-Hz-Sinus 0,1 über 4 s; nur Slot 5 `enabled` → true; Periodenspitzen über 2 s aufzeichnen. Heute erwartbar: Minimum −6,6 dB, 1 dB erst nach 220 ms. Erfolgskriterium: Pegel bei 50 Hz nach dem Fade innerhalb ±0,5 dB des Zustands davor (der Notch bewegt 50 Hz um weniger als 0,01 dB). Gegenfall: Typwechsel an Slot 0 selbst darf einschwingen. Dynamische Variante: Slot 2 im Plateau mit Attack 50 ms, Auslenkung bleibt innerhalb ±0,5 dB.
8. **Abgrenzung:** T3-14-02 ist die Gegenrichtung (Historie wird bei Neubelegung zu Unrecht übernommen); beide Befunde brauchen eine Slot-Lebenszykluskennung. M-07/R2 bleiben unberührt. T3-01-09 und T3-01-10 sind andere Eintritte. Keine Registerzeile.

### P15-D-05 — Auto-Gain ist unbeschränkt: der abgeleitete Wert verlässt die Vertragsgrenze ±120 dB und verstärkt Inhalt außerhalb des Gitters extrem

**Vorschlag:** LÜCKE (künftiger Vertragsbruch, Sender noch nicht angeschlossen) · **low** · **K3** · Evidenz: Nachrechnung (`p15-paket2-autogaingrenze.py`).

1. **Auslöser:** 48 kHz stereo. Acht belegte Low-Shelves +12 dB Q 8 bei 1 kHz → Auto-Gain **−199,8 dB** (Q 24: −275,5 dB; Q 2: −117,5 dB). Acht High-Cuts 20 Hz Q 0,15 → **+150,5 dB**. Mit `auto_gain` = true liegt der Gesamtgang bei 1 Hz (DC/Infraschall) dann bei +147 dB. Schon ein einzelner High-Cut 20 Hz Q 0,707 hebt DC/Sub um +15,7 dB. Gegenwert: acht Low-Shelves +12 dB Q 0,707 → −95,9 dB (innerhalb).
2. **Quellstellen:** `DspProgramm.cpp:85-136` (keine Klemmung, nur `mittel > 0` und endlich), `:266-267`; `DspKern.cpp:424` (Ziel der Auto-Gain-Rampe), `:715-720` (Anwendung); `NakamaTransaktion.cpp:794-796` (Bericht übernimmt ungeklemmt); `eq-ipc-v3.schema.json:2324-2329`.
3. **Produkteinstieg:** Bandbelegung über Transaktion oder Zustand (NAK-306); `v2.global.auto_gain` als Hostparameter. `baueBericht` ist gebaut (`SondeProcessor.cpp:1281-1285`); der Sender von `state_report.dsp` ist Etappe 4b und **noch nicht angeschlossen** (`SONDE-015.md:4030`).
4. **Zusage:** Schema `eq-ipc-v3.schema.json:2324-2329` mit `minimum −120`, `maximum 120` und dem `$comment` „Die Grenze folgt aus dem Erreichbaren: acht Baender zu je 12 dB im selben Kanalmodus ergeben hoechstens 96 dB Plateau; 120 ist die Reserve darueber." (E2-7, `SONDE-015.md:2951`). Die Erreichbarkeitsaussage stimmt nicht (Shelf-Überschwinger bei Q > 0,707, Cuts mit Q < 0,707). Da der Sender fehlt, heute LÜCKE.
5. **Gegenweg und stärkstes Gegenargument:** R4 kompensiert bewusst ohne Klemmung; M-36/M-37 halten. Stärkstes Gegenargument: Die Konfigurationen sind absurd. Entgegnung: Das DTO ist vertragsgültig; ein Riegel gehört an die Ableitung oder an den Vertrag, sonst weist der Broker einen gültigen Zustand später als Vertragsfehler ab.
6. **Vorhandene Tests und warum sie nicht genügen:** `DspGoldenTestMain.cpp:2433-2621` (Abschnitt H prüft bei 48 kHz nur flach, 0-dB-Bell, +6-dB-Shelf, Mid/Side-Paar, Dynamik und Output-Trim); `TransactionTestMain.cpp:1576-1593` (Bericht ohne Grenzprüfung).
7. **Minimaler Folgebeweis:** B7: acht Low-Shelves +12 dB Q 8 bei 1 kHz belegen, `baueBericht` → `autoGainDb` ≈ −199,8; den gebauten `dsp_bericht` gegen `$defs/dsp_bericht` mit der A5-Vertragsengine prüfen → heute verletzt. Erfolgskriterium: eine festgelegte Regel (Klemmung mit Meldung oder angepasster Vertrag), Wert vertragsgültig. Gegenfall: acht Shelves Q 0,707 bleiben gültig. Audioseite: `auto_gain` an, acht High-Cuts 20 Hz Q 0,15, DC 1e-6 am Eingang → Ausgang nach festzulegender Regel begrenzt.
8. **Abgrenzung:** NAK-110 (DSP-Rückweg und Sender fehlen); NAK-254 (Nyquist-Kappung nicht gemeldet, andere Angabe); NAK-81 (Gain In/Out). Kein T3-Befund.

### P15-D-06 — „Nahe Nyquist wird Decramping geprüft" (Entwurf §44.2) hat weder Matrixzeile noch Entscheid noch Registerzeile; reines RBJ weicht im obersten Oktavbereich bis 6,5 dB vom analogen Prototyp ab

**Vorschlag:** LÜCKE · **low** · **K3** (Plan- und Vertragsabdeckung) · Evidenz: Nachrechnung (`p15-paket2-cramping.py`) plus Dokumentsuche.

1. **Auslöser:** Bell 16 kHz Q 2 +12 dB bei 19,2 kHz: 44,1 kHz −6,48 dB gegen den analogen Prototyp, 48 kHz −5,52 dB, 96 kHz −1,19 dB. Bell 12 kHz Q 1 +12 dB bei 0,45 fs (44,1 kHz): −5,22 dB. High-Shelf 16 kHz +12 dB bei 19,2 kHz (44,1 kHz): +3,56 dB. Derselbe gespeicherte Zustand klingt bei 44,1 und 96 kHz im Bereich 12–20 kHz um bis zu rund 5 dB verschieden.
2. **Quellstellen:** `DspFilter.h:116-167` (RBJ, Vorverzerrung auf f0), `:109-114` (Kappung 0,45 fs); `DspSvf.h:26-30` (SVF-Ruheantwort identisch zur RBJ-Antwort, also dieselbe Kompression).
3. **Produkteinstieg:** jede Bandbelegung (Transaktion oder Zustand) bei der Samplerate des Hosts.
4. **Zusage:** Entwurf §44.2 (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2980-2981`, als bindend zitiert in `SONDE-015.md:210-211`): „minimumphasige RBJ-Biquads als Referenz für statische Bell-, Shelf-, Notch- und Cut-Filter; nahe Nyquist wird ein matched-analog-/Orfanidis-artiges Decramping geprüft". Die Suche nach Decramp, Orfanidis und cramping findet nur den Entwurf, SONDE-015 §1.3 und das Archiv — keine M-Zeile, kein E-/R-Entscheid, keine Registerzeile. `CLAUDE.md` verlangt Widersprüche als Registerzeile.
5. **Gegenweg und stärkstes Gegenargument:** R15/M-10 legen RBJ als Referenz fest (`SONDE-015.md:2094-2097`, M-10 `:1048`), und B6 hält ±0,05 dB gegen RBJ. Stärkstes Gegenargument: „geprüft" verlangt keinen Bau, und die RBJ-Wahl schließt die Prüfung implizit ein. Entgegnung: Eine Prüfung ist nirgends dokumentiert, und die Größenordnung ist hörbar.
6. **Vorhandene Tests und warum sie nicht genügen:** `DspGoldenTestMain.cpp:1120-1177` hat das digitale RBJ als Referenz und Prüflinge mit Eckfrequenzen ≤ 8 kHz; kein Hochtonfall.
7. **Minimaler Folgebeweis:** datierter Entscheid (Decramping ja oder nein) mit Registerzeile. Bei „ja": B6-Fall Bell 16 kHz Q 2 +12 dB bei 44,1/48/96 kHz gegen den analogen Prototyp mit festgelegter Toleranz; Gegenfall 1 kHz unverändert.
8. **Abgrenzung:** NAK-254 (Meldung der Kappung, nicht Frequenzgang); T3-06-01 (Analyse-PSD). Kein T3-Befund.

### P15-D-07 — Die Steuerrate von 8 Samples ist bei 44,1/48 kHz gröber als die kürzeste Attack; die Begründung in E-6 und `DspProgramm.h` rechnet falsch

**Vorschlag:** LÜCKE · **low** · **K5** · Evidenz: Quellenherleitung (Arithmetik).

1. **Auslöser und Ablauf:** 48 kHz (44,1 kHz), `attack_ms` = 0,1 (Vertragsminimum), lauter Einsatz über Threshold + Knie in Steuertaktphase p ∈ 0…7. Die Hüllkurve erreicht das Plateau nach 1 Sample (Pol 0,812). Der Neuentwurf folgt erst am Steuerschritt bei p, voll angewandt ist er erst bei p + 8 (die Mischung beginnt bei t = 0 mit `svfVon`). Volle Verstärkung also nach 8 bis 15 Samples = 0,17 bis 0,31 ms (44,1 kHz: 0,18 bis 0,34 ms), halbe nach p + 4. Die Reaktion hängt von der Steuertaktphase relativ zum Bankstart ab.
2. **Quellstellen:** `DspProgramm.h:64-70` („Bei 48 kHz sind das 0,167 ms - feiner als die kuerzeste Attack (0,1 ms)"); `DspKern.cpp:557-581`; `DspBankPool.h:68-70`.
3. **Produkteinstieg:** dynamisches Band (Transaktion oder Zustand, NAK-306); `attack_ms` per Hostautomation.
4. **Zusage:** Keine Zahlenzusage zur Reaktionszeit des Gains. E-6 (`SONDE-015.md:3397`) behauptet: „8 Samples sind 0,167 ms bei 48 kHz und damit feiner als die kürzeste Attack (0,1 ms)" — 0,167 ms ist größer als 0,1 ms. Entwurf §44.2 (`:2984`) nennt „explizite Attack/Hold/Release-Smoothing". Daher LÜCKE mit falscher Begründung.
5. **Gegenweg und stärkstes Gegenargument:** Bei 96/192 kHz ist der Schritt mit 0,083/0,042 ms tatsächlich feiner; die Hüllkurve selbst folgt der Attack exakt (M-26); E-25 zählt die Steuerrate („höchstens 0,18 ms") zur Toleranz. Stärkstes Gegenargument: 0,2 ms Verzug sind praktisch unhörbar. Entgegnung: Der Einstellbereich 0,1 bis 0,2 ms wirkt gleich, und die Behauptung im Entscheid ist falsch.
6. **Vorhandene Tests und warum sie nicht genügen:** `DspGoldenTestMain.cpp:1894-1970` (Attack 20 ms, Toleranz 1 ms); `:1972-2059` (W-2 misst Leistung und Auslenkung am Steuerschritt, nicht die angewandte Verstärkung dazwischen); `:2061-2113` (Blocklänge 8 ab Phase 0).
7. **Minimaler Folgebeweis:** B6 bei 48 kHz: Bell 1 kHz, Range −12, Threshold −40, Attack 0,1 ms, Quadraturton 20 dB über Threshold, Einsatz bei Steuerphase 0, 3 und 7; angewandte Verstärkung je Sample aus dem Tap. Heute: 90 % nach 8 bis 15 Samples. Erfolgskriterium: eine festgelegte Regel (z. B. ≤ Attack + 2 Samples) oder berichtigte Behauptung in E-6 und `DspProgramm.h`. Gegenfall: 96 kHz.
8. **Abgrenzung:** kein T3- oder NAK-Punkt.

---

## 5. Offene Prüffelder (sinnvoll, nicht belegbar)

- **O-01 Mono-Bus.** Ein Mono-Layout wird angenommen (`SondeProcessor.cpp:311-333`); der Kern dupliziert L und schreibt nur Kanal 0 (`DspKern.cpp:893-895`, `:1079-1085`). Bänder auf `right` oder `side` sind hörbar wirkungslos, die Auto-Gain-Faltung (`DspProgramm.cpp:121-129`) kompensiert sie aber. Nachrechnung (`p15-paket2-monoautogain.py`, High-Shelf +12 dB über das ganze Gitter): right −9,02 dB netto, side −7,67 dB; left +2,73 dB, mid +4,08 dB unterkompensiert; Bell +6 dB Q 1 auf right −0,77 dB. Nicht belegbar: ob FL je Mono aushandelt (`host-capabilities-fl-v1.json` kennt nur zweikanalige Busse). Kleinster Folgeversuch: B6/A16 mit `numKanaele = 1`, einem Band auf right und `auto_gain` an; Host-Nachweis des Layouts.
- **O-02 Sampleraten unter 40 kHz.** R4 Feinheit 1 setzt „kleinste Rate 44,1 kHz" voraus (`SONDE-015.md:1620`), `prepareToPlay` nimmt aber jede Rate > 0 (`SondeProcessor.cpp:237-238`). Gitterstellen oberhalb fs/2 werten die gespiegelte Antwort aus (`DspProgramm.cpp:101-102`). Folgeversuch: 32 kHz, Bell +12 dB Q 4 bei 14 kHz, Auto-Gain mit und ohne Kappung des Gitters auf fs/2 vergleichen; Ratenliste von FL belegen.
- **O-03 Telemetrie `band_dynamic_gain_db`.** Die Kernquelle meldet den zuletzt entworfenen Zielwert (`DspKern.cpp:572-575`, `:618-624`), der erst 8 Samples später voll wirkt (`:579-581`): Vorlauf ≤ 15 Samples. Wert und Gain stammen aus demselben `g_dyn`, aber nicht aus demselben Sample. Der Sender ist Etappe 4b (`SONDE-015.md:4031`) und fehlt, heute also ohne Produktwirkung. Der gemeldete Wert ist konsistent mit dem angewandten Gain, nicht mit einer RMS-Kennlinie (P15-D-02). Folgeversuch bei 4b: Zuordnung Frame ↔ Analysefenster ↔ Generation (M-121) und Definition „momentan" gegen den angewandten Koeffizienten.
- **O-04 Güte bei Shelves.** Der Vertrag erlaubt q 0,15–24 für alle Typen (`nakama-parameter-v2.json:166-178`); RBJ-Shelves erreichen bei Q 24 +37,1/−25,1 dB (±12 dB) und +51,0/−27,0 dB (wirksam +24 dB). Keine Zusage zur Überhöhung; das ist eine Produktfrage (Q oder Slope). Folgeversuch: Entscheid, dann B6-Golden für einen Shelf mit Q 24.
- **O-05 Steuertaktphasen und Blockmischungen.** B6 W-3 prüft nur Phase 0 mit Blocklänge 8 (`DspGoldenTestMain.cpp:2061-2113`); die Herleitung zeigt keine Abhängigkeit von der Blockteilung (V-04). Folgeversuch: identische Rampenfolgen (Range ±12 ↔ 0, Frequenz, Q; statisch und dynamisch) mit Blockfolgen 1/7/64/333 und Startversatz 1 und 7; Tap bitgleich zwischen den Teilungen.
- **O-06 Projekt-Reload mit dynamischen Bändern.** Entwurf §44.5 zitiert in `SONDE-015.md:311-312`: „Projekt-Reload rekonstruiert denselben State-Hash und innerhalb numerischer Toleranz denselben Audioausgang". Nach dem Laden starten Detektor und Hüllkurve kalt (`DspKern.cpp:262-271`); bis zum Ende der Attack weicht der Ausgang um bis zu ∣Range∣ ab. Eine Toleranz für dieses Einschwingen ist nicht definiert. Folgeversuch: Reload-Golden mit einem dynamischen Band im Plateau, Abweichung über die Zeit ausweisen und Toleranzregel festlegen.

---

## 6. Verworfene Verdachte

- **V-01 SVF-Modulation instabil.** Gegenbeleg: `DspSvf.h:73-81`, `DspKern.cpp:25-37`. Herleitung: Die homogene Zustandsmatrix M = [[2a1−1, −2a2], [2a2, 1−2a3]] erfüllt für a2² ≤ a1·a3, a1, a3 ≥ 0 und s = 1 − a1 − a3 ≥ 0 die Beziehung I − MᵀM = [[4(a1 s + Δ), −4a2 s], [−4a2 s, 4(a3 s + Δ)]] ⪰ 0 mit Δ = a1a3 − a2² ≥ 0. Jeder Entwurf erfüllt a2² = a1·a3; lineare Mischungen erhalten a2² ≤ a1·a3 (Cauchy-Schwarz) und s ≥ 0, daher ‖M‖₂ ≤ 1 für jede Koeffizientenfolge der Steuerrate und der Rampe. Nachrechnung Teil A (`p15-paket2-stabil.py`): 40 000 Paare × 5 Mischwerte = 200 000 Matrizen, max ‖M‖₂ = 1,000000000000, max Spektralradius 0,999996; Randfälle max ‖M‖₂ = 1,0.
- **V-02 Biquad-Rampe frozen-time instabil.** Gegenbeleg: `DspKern.cpp:39-52` — das Stabilitätsdreieck ist konvex, E-19 stimmt. Nachrechnung Teil B: alle Zwischenentwürfe der 8 Randfälle bei 257 Mischwerten stabil; im Scan (`p15-paket2-zwischenentwurf.py`) 225 Kombinationen per Assertion stabil. Die zeitvariante Spitze ist P15-D-03.
- **V-03 Mischung der Hüllkurvenpole instabil.** Gegenbeleg: `DspKern.cpp:58-72`; Pole aus [0, 1) bleiben bei konvexer Mischung in [0, 1).
- **V-04 Ergebnis hängt von der Blockteilung ab.** Gegenbeleg: Steuerzähler, Rampen- und Übergangsrest liegen samplegenau im Zustand (`DspBankPool.h:68-70`; `DspKern.cpp:494-495`, `:579-581`, `:796-814`); Übernahme nur je äußerem Aufruf (`:837-838`); die Detektor-Nullung am Fensterende (`:608-612`) wirkt nicht auf den Ausgang, weil eine Reaktivierung nur an einem Blockrand kommt; der Denormal-Riegel bei 1e-300 ist unhörbar (`DspFilter.h:64`). Blockrandereignisse (E-17) und die Hörmatrix je Stück (E-34) sind gewollt.
- **V-05 Wirksame dynamische Summe ungeklemmt.** Gegenbeleg: gewollt (R7 Feinheit 2, `SONDE-015.md:1710`); der SVF bleibt bei ±24 dB kontrahierend (V-01); die Kennlinie begrenzt ∣g_dyn∣ ≤ ∣r∣ (`DspFilter.h:355-361`).
- **V-06 Nyquist-Kappung wird still angewandt.** Bekannt als NAK-254 (`docs/offene-punkte.md:264`).
- **V-07 Doppelte Genauigkeit reicht bei 20 Hz und 192–768 kHz nicht.** Koeffizientenabstände von rund 1e-7 bis 1e-8 liegen weit über 1e-16; DF2T-Zustände bleiben für Hoch- und Tiefpass in der Größenordnung des Eingangs; Rundungsrauschen nach Abschätzung unter −130 dBFS (Q 24).
- **V-08 0-dB-Bell wird übersprungen oder verändert Bits.** `Biquad::istEinheit` (`DspFilter.h:50-53`) greift für RBJ-0-dB-Entwürfe nie, weil b1 = a1 ≠ 0; das Band wird gerechnet. Bei leerem Zustand gilt y = x + 0 bitgleich (außer −0,0 → +0,0). Keine Zusage für belegte 0-dB-Bänder (M-02 betrifft freie Slots); M-36 hält über b == a (`DspProgramm.cpp:26-31`, `:134`). Die Kommentare `DspProgramm.h:87-90` und `DspKern.cpp:474-480` sind unzutreffend, aber wirkungslos — Pflegehinweis, kein Befund.
- **V-09 `auto_gain` (blockrand) rampt statt überzublenden.** `DspProgramm.cpp:139-159` prüft `autoGainAn` nicht; eine lineare Gainrampe 1 → g ist identisch mit einem linearen Crossfade zwischen y und g·y.
- **V-10 Range ≠ 0 → 0 hinterlässt eine Restauslenkung.** Der erste Steuerschritt nach Rampenende entwirft g_dyn = 0 (`DspKern.cpp:527`, `:570-575`); der Detektor wird genullt (`:608-612`); B6 `:2061-2113`.
- **V-11 Samplerate-Wechsel mit vorhandenen Bänken.** `bereiteVor` setzt Pool, Pfade und Rampen zurück (`DspKern.cpp:97-124`); `rampenKompatibel` verlangt gleiche Rate (`DspProgramm.cpp:145`).
- **V-12 Puls-Stufen der Hörmarkierung erzeugen Überschwinger.** Nachrechnung `p15-paket2-puls.py`: bei 48/192 kHz und f0 20 Hz bis 1 kHz keine Spitze über +4,5 dB (tiefe f0 erreichen nur +2,25 bis +3,1 dB).
- **V-13 Hörmarkierung mit extremer Bandbreite instabil.** `biquadStabil` (`HoerMarkierung.h:68-74`, `:246-252`) weist a2 = −1 und NaN ab; `std::max` mit NaN liefert NaN und fällt dort durch. Die Aussage im Kommentar `:31-34` („Pfad … wieder samplegleich") gilt bei tiefem f0 innerhalb der Ruhephase nicht; keine Zusage, keine Wirkung auf den Nulltest nach dem Ausfade (`:644-648`).
- **V-14 Telemetrie und angewandter Gain aus verschiedenen Werten.** Beide kommen aus demselben `g_dyn` desselben Steuerschritts (`DspKern.cpp:572-575`); es bleibt nur der Versatz (O-03).
- **V-15 Hold-Mischung rundet falsch.** `DspKern.cpp:63-70`: exakte Vielfache von 1/256; B6 `:2054-2057`.
- **V-16 Welligkeit der Hüllkurve aliasiert im Steuertakt zu hörbarer Modulation.** Die Welligkeit ist durch release ≥ 5 ms begrenzt: bei einer Leistungsperiode von 8 Samples (48 kHz) Abfall je Periode exp(−8/240) ≈ 0,967, also rund 0,15 dB; als Auslenkung ≤ 0,15·∣r∣/12 dB (Handrechnung).
- **V-17 −inf oder NaN erreichen die Kennlinie.** `leistungInDb` klemmt auf −240 dB (`DspFilter.h:336-341`); Parameter sind validiert endlich (`NakamaParameter.cpp:178-200`).
- **V-18 t von Crossfade, Rampe und Bandmischung falsch gezählt.** `DspKern.cpp:494-495` (1/256 … 1), `:800` (0 … 255/256), `Rampe::tick` (`DspKern.h:325-334`) sind konsistent; B6 `:781-820`.
- **V-19 `mono_bass_hz` extrem tief (z. B. 0,001 Hz) numerisch instabil.** Hochpass-DF2T mit Zuständen in der Größenordnung des Eingangs, Pole im Dreieck; die sehr lange Abklingzeit ist physikalisch gewollt.

---

## 7. Abgrenzung zu bekannten T3- und NAK-Punkten

| Bekannter Punkt | Inhalt | Verhältnis zu diesem Paket |
| --- | --- | --- |
| T3-01-01 | subnormale Samples/DAZ im neutralen Pfad | nicht berührt; V-07/V-08 betreffen double-Zustände und belegte 0-dB-Bänder |
| T3-01-04 | Offline nach Vorschau, Candidate im ersten Offline-Block | nicht berührt; P15-D-01 nennt nur eine Candidate-Variante im Tap, ohne Zählung |
| T3-01-05 | 5-ms-Worker fasst Hostwerte zusammen | vergrößert Sprunghöhen (P15-D-03); P15-D-01 braucht dagegen getrennte Takte |
| T3-01-09 / T3-01-10 | VST3-Hostbypass / Gen-Reset mit Markierung | andere Eintritte; P15-D-01 läuft über `eq_enabled`, P15-D-04 über Bandtopologie |
| T3-05-01 / T3-05-02 | Mailbox über Reload / read-only-Automation | nicht berührt; dieselben Hostwege dienen hier nur als Einstieg |
| T3-09-01 | Frischeriegel ohne `plugin/dsp/` | betrifft jeden späteren B6-Folgebeweis dieses Pakets (Bau vor Messung) |
| T3-09-02 | Host-Nulltest vergleicht numerisch statt bitweise | nicht berührt |
| T3-14-02 | Remove und Neubelegung ohne Audio dazwischen erbt Historie | gleiche Stelle `DspKern.cpp:397-408`, Gegenrichtung zu P15-D-04; P15-D-03 betrifft die Koeffizientenbahn, nicht den Lebenszyklus |
| Phase 14, Folgeprüfbereich 1 | Filtergrenzen und Audio bei dynamischen Rampen | hier bearbeitet: P15-D-02, -03, -04, -07, O-05, V-01 bis V-05 |
| NAK-81 | Gain In/Out, Planfragen | Randbezug zu P15-D-01 und P15-D-05, kein Duplikat |
| NAK-110 | DSP-Rückweg im v3-Vertrag, Sender 4b | P15-D-05 und O-03 betreffen den noch nicht angeschlossenen Sender |
| NAK-250 | Performance nicht gemessen | nicht berührt |
| NAK-254 | Nyquist-Kappung erreicht Gen nicht | V-06; P15-D-06 betrifft den Frequenzgang, nicht die Meldung |
| NAK-283 F04 / F05 | Mono/Stereo-Layout / Host-Reset | O-01 baut auf F04 auf; F05/R-283-3 konserviert den Altstand aus P15-D-01 |
| NAK-306 | Band heute nur über Gen/Transaktion belegbar | Reichweite von P15-D-02 bis -05 und -07 |
