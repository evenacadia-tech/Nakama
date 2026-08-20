# Beweismanifest — SONDE-004a «Wegwerf-Target mit zwei Aux-Bussen + Impulsharness»

| Feld | Wert |
|---|---|
| Ticket | `SONDE-004a` (Vorzug nach Pruefbericht NAK-22) |
| Phase / Session | P0 / S1 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §65: *„FL-Aux-/PDC-/Recall-Spike und Capabilityreport — jede Aux-Capability eindeutig supported/unsupported"* |
| Abgrenzung | S1 baut das **Messgeraet**. Der Capabilityreport selbst ist S4, nach User-Termin A und B. |
| Datum | 2026-08-20 |
| Pruefstufen | T1 ✅ · T2 laut Bauaufteilung §3 fuer S1 **nicht gefordert** · T3 am Gate G0 |

---

## 1. Ticket-Behauptungen

| # | Behauptung | Befehl | Ergebnis | Rohausgabe |
|---|---|---|---|---|
| B1 | Das Target deklariert Main-I/O plus GENAU ZWEI Aux-Eingaenge mit den Vertragsnamen `priority_sidechain` und `compare_pre`, beide Vorgabe-inaktiv. | `EqCopAuxSpikeTest.exe` | ✅ | [↓ B1](#b1) |
| B2 | Der Impulsversatz wird sampledgenau gemessen — inklusive Blockgrenze, Versatz 0 und fehlendem Impuls. | `EqCopAuxSpikeTest.exe` | ✅ | [↓ B1](#b1) |
| B3 | Laeuft statt Stille ein Dauersignal, verweigert der Spike den Versatz mit Klartextgrund, statt eine Zahl zu erfinden. | `EqCopAuxSpikeTest.exe` | ✅ | [↓ B1](#b1) |
| B4 | Der Main-Ausgang bleibt bitgleich, auch mit zwei aktiven Aux-Bussen. | `EqCopAuxSpikeTest.exe` | ✅ | [↓ B1](#b1) |
| B5 | Gegenpfade gefahren: Bericht schreiben ↔ wieder einlesen, Messung ↔ Reset. Fehlende Werte sind `null`, nicht 0. | `EqCopAuxSpikeTest.exe` | ✅ | [↓ B1](#b1) |
| B6 | Das Wegwerf-VST3 baut in Release und liegt als ladbares Bundle vor. | `cmake --build … --target EqCopAuxSpike_VST3` | ✅ | [↓ B2](#b2) |
| B7 | Die Impuls-Fixtures sind deterministisch — zwei Laeufe, gleiche Hashes. | `erzeuge_aux_spike_fixtures.py` (2×) | ✅ | [↓ B3](#b3) |
| B8 | Die eingefrorene Identitaet bleibt unberuehrt: keine Produktquelle angefasst, Plugin-Code `NkSp` ausserhalb von `Eqcp`/`NkPr`/`NkAc`. | `git diff --name-only HEAD -- eq-copilot/plugin/src` | ✅ | [↓ B4](#b4) |

---

## 2. Rohe Ausgaben

<a id="b1"></a>
### B1 · Selbsttest des Messgeraets

**Befehl:** `eq-copilot\build\plugin\EqCopAuxSpikeTest_artefacts\Release\EqCopAuxSpikeTest.exe` · **Exitcode:** 0

```text
  ok      drei Eingangsbusse deklariert  [3]
  ok      Aux 1 traegt den Vertragsnamen priority_sidechain
  ok      Aux 2 traegt den Vertragsnamen compare_pre
  ok      Aux-Busse sind Vorgabe-inaktiv (Host muss zuschalten)
  ok      Main stereo + zwei Aux stereo wird angenommen
  ok      Main allein (beide Aux aus) wird angenommen
  ok      Main mono wird angenommen
  ok      5.1 am Main wird klar abgelehnt (kein stilles Umdeuten)
  ok      5.1 auf einem Aux-Bus wird abgelehnt
  ok      Layout mit zwei aktiven Aux-Bussen gesetzt
  ok      Messstand kennt drei Busse
  ok      Main-Impuls exakt bei Sample 100  [100]
  ok      Aux-1-Impuls exakt bei Sample 612  [612]
  ok      Aux-2-Impuls exakt auf der Blockgrenze (512)  [512]
  ok      Versatz Aux 1 = +512 Samples  [512]
  ok      Versatz Aux 2 = +412 Samples  [412]
  ok      Samplezaehler stimmt  [2048]
  ok      Stille + ein Impuls gilt als eingehaltenes Messprotokoll
  ok      Reset loescht alle Impulsbefunde
  ok      Reset setzt den Samplezaehler zurueck  [512]
  ok      Main-Ausgang bleibt bitgleich, auch mit aktiven Aux-Bussen
  ok      Dauersignal auf Main wird als Protokollverletzung erkannt  [0.980]
  ok      bei verletztem Protokoll gibt es KEINEN Versatzwert
  ok      der Grund steht im Klartext da  [Dauersignal statt Stille+Impuls â Messprotokoll verletzt]
  ok      der stille Aux-Bus bleibt davon unberuehrt bewertet
  ok      Impuls bei Sample 0 wird als 0 gemeldet
  ok      Versatz 0 ist ein Messwert, kein Fehlen
  ok      Signal unter der Schwelle loest KEINEN Impuls aus
  ok      Kleinsignal wird trotzdem als angekommen gemessen  [0.100]
  ok      ohne Impuls gibt es keinen Versatz (nicht 0)
  ok      abgeschaltete Aux-Busse melden sich als inaktiv
  ok      inaktiver Bus liefert keine erfundenen Messwerte
  ok      Bericht als Datei geschrieben
  ok      Bericht wieder eingelesen und geparst
  ok      Bericht traegt alle drei Busse
  ok      Bericht: Main-Impuls 64 unveraendert durch den Roundtrip
  ok      Bericht: Versatz Aux 1 = 256 Samples
  ok      Bericht: Versatz auch in Millisekunden korrekt
  ok      Bericht: fehlender Impuls ist null, nicht 0
  ok      Bericht: fehlender Versatz ist null, nicht 0
  ok      Bericht: fehlender Versatz nennt seinen Grund

AUXSPIKE-TEST OK — 41 Pruefungen ok, 0 Fehler
```

<a id="b2"></a>
### B2 · Gebautes Bundle

```text
Pfad:   eq-copilot\build\plugin\EqCopAuxSpike_artefacts\Release\VST3\EqCop-Aux-Spike.vst3\Contents\x86_64-win\EqCop-Aux-Spike.vst3
Groesse: 6250 KB
Gebaut:  2026-08-20 19:37:04
SHA-256: E451D91F729C14B7B4D292E7CBA0CBD0EEFFC645DF7E81B51430619BA098B0C6
```

<a id="b3"></a>
### B3 · Impuls-Fixtures, zwei Laeufe

```text
--- Lauf 1 ---
impuls-44100.wav  rate=44100  impuls_ab_sample=22050  dauer_samples=88200  sha256=6EEC38186549BD9D19922E018B76EB17B79BF6C8300AE05AA49E0D064647FDCC
impuls-48000.wav  rate=48000  impuls_ab_sample=24000  dauer_samples=96000  sha256=D4E4D119D73D29AE2DF7125BA3FC685A7FC9A00A0DF9B742BA7CEDF689AFB5A1
--- Lauf 2 ---
impuls-44100.wav  rate=44100  impuls_ab_sample=22050  dauer_samples=88200  sha256=6EEC38186549BD9D19922E018B76EB17B79BF6C8300AE05AA49E0D064647FDCC
impuls-48000.wav  rate=48000  impuls_ab_sample=24000  dauer_samples=96000  sha256=D4E4D119D73D29AE2DF7125BA3FC685A7FC9A00A0DF9B742BA7CEDF689AFB5A1
```

<a id="b4"></a>
### B4 · Produktquellen unberuehrt

**Befehl:** `git diff --name-only HEAD -- eq-copilot/plugin/src`

```text
(leer — keine Produktquelle angefasst)
```

---

## 4. T1 — Selbstaudit

| # | Punkt | Befund |
|---|---|---|
| 1 | Numerische Raender | NaN/Inf loesen keinen Impuls aus (jeder Vergleich mit NaN ist false) und veraendern die Spitze nicht. `-1` ist reserviert fuer „kein Impuls" und wird nie mit 0 vermischt. `lautAnteil` ist `-1`, solange nichts gesehen wurde — die Division durch `samplesGesehen` ist gegen 0 abgesichert. Zaehler sind `int64`; bei 44,1 kHz reicht das fuer ~6,6 Mio. Jahre. |
| 2 | Gegenpfad vorhanden? | Messung ↔ `messungZuruecksetzen()` (getestet) · Bericht schreiben ↔ wieder einlesen (getestet) · Suchpfad hinzufuegen ↔ entfernen (Schritt 2 und 6 der Klickliste). Save/Load des Plugin-States: bewusst leer — ein Wegwerf-Messgeraet hat keinen Zustand, der eine Migration verdient. |
| 3 | Behauptungs-Integritaet | §65 und §48.2 im Entwurf geoeffnet und woertlich uebernommen (§65 = „Erste Ticketfolge", Zeile 3788). JUCE-API an der gepinnten 8.0.9-Quelle nachgelesen: `BusesProperties::withInput` (juce_AudioProcessor.h:1451), `getBusBuffer` rechnet den Kanalversatz selbst (Z. 669–676), jeder weitere Input-Bus wird `kAux` (VST3-Wrapper Z. 3230). |
| 4 | Luegt der UI-Text? | Nein — und das war Arbeit: „kein Impuls" steht als `—`, nie als 0; ein inaktiver Bus zeigt `INAKTIV` statt „kein Signal"; ohne Audio steht „Noch kein Audio verarbeitet" statt einer stillen Null; bei Dauersignal steht der Grund im Klartext statt einer Zahl. |
| 5 | Anzeige-Pflichten (§0.4) | Sinngemaess erfuellt, soweit sie auf ein Messgeraet zutreffen: Frische (laufender Samplezaehler), Konfidenz (Protokollzustand je Bus), Degradation (inaktiver Bus), „welche Aussage gerade NICHT gilt" (Versatzgrund im Klartext). `arming`/`audible_ready` gibt es hier nicht — der Spike hat keine hoerbare Aktion. |
| 6 | Audiothread | Keine Allokation (nur `std::atomic`-Zugriffe und Stack-Variablen), keine Sperre (Reset laeuft ueber `exchange` einer Flagge), kein Datei-/Pipe-I/O, kein Logging. Der JSON-Bericht wird ausschliesslich vom Nachrichtenthread geschrieben. |

---

## 6. Befunde aus diesem Ticket

| Befund | Quelle | Gegen die Quelldatei verifiziert? | Ausgang |
|---|---|---|---|
| Die erste Fassung erkannte „erstes Sample ueber der Schwelle" als Impuls. Auf einer laufenden Mischung meldete sie deshalb den Impuls bei Sample 0 — eine glaubwuerdige, falsche Zahl. | T1 / Selbsttest, noch vor dem FL-Termin | ja (Testlauf: `Main-Impuls exakt bei Sample 100 [0]`) | **gefixt**: der Spike zaehlt jetzt den Anteil lauter Samples und verweigert bei Dauersignal den Versatz mit Klartextgrund. Zwei Tests decken es ab. |
| Ein vorzeitiges `break` in der Kanalschleife haette „spitze_gesamt" nur bis zum Impuls gemessen und damit einen falschen Spitzenwert berichtet. | T1 / Wiederlesen | ja (Quelltext) | **gefixt** vor dem ersten Bau: die Schleife laeuft vollstaendig durch. |
| `Start-Process -ArgumentList` quotiert nicht — betrifft jedes Werkzeug dieses Workspaces. | S0 | ja (`git log --format=%h %s` → Exit 128) | **gefixt** in `tools/beweise.ps1`, als Landmine in `CLAUDE.md` notiert. |

Kein Befund offen; keine NAK-Zeile noetig.

---

---

## Kanon-Lauf — SONDE-004a

**Lauf:** 2026-08-20 19:43 · **Runner:** `tools/beweise.ps1` · **Urteil:** GRUEN — 4/4 Kanon-Laeufe bestanden · 7 geplante Pruefung(en) noch nicht gebaut · **Exitcode:** 0

### Kopf — woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-20 19:43:02 +02:00 |
| Rechner | SCHUBBINATOR200 · Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 3d820c6 Doku: Basislinien-SHA berichtigt (734cf50 -> b7d37ce) |
| Commit (voll) | 3d820c6443b6c15b8c732736ce307810e0cf0751 |
| Arbeitsbaum | 9 unbestaetigte Datei(en) — dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9 |
| FL Studio | FL Studio 2025 25.2.5.5319 · FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M .gitignore
 M eq-copilot/plugin/CMakeLists.txt
 M tools/beweise.ps1
?? docs/beweise/SONDE-004a.md
?? eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md
?? eq-copilot/fixtures/aux-spike/
?? eq-copilot/plugin/spike/
?? eq-copilot/plugin/tests/AuxSpikeTestMain.cpp
?? tools/eq-copilot/erzeuge_aux_spike_fixtures.py
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/tests`, CMakeLists): **2026-08-20 19:35:42**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,89 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,24 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,33 s | [↓ A4](#a4) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [GEPLANT] geplant (ab P0) | — | — |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | — | — |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [GEPLANT] geplant (ab P2) | — | — |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | — | — |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | — | — |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | — | — |
| B7 | Apply/Revert ist transaktional — kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | — | — |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 · EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` · **Exitcode:** 0 · **Dauer:** 0,06 s

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
#### A2 · EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` · **Exitcode:** 0 · **Dauer:** 9,89 s

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
#### A3 · EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` · **Exitcode:** 0 · **Dauer:** 6,24 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
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
#### A4 · broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` · **Exitcode:** 0 · **Dauer:** 0,33 s

stdout:

```text

running 36 tests
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
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
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 36 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.11s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-a60741d6aa4f32af.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-b91f868b743e27b1.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-c17ffc214d5eff74.exe)
   Doc-tests eqcop_broker
```

### Bau vor dem Lauf (`-Bauen`)

**build** · Exit 0 · 2,11 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
```

</details>

