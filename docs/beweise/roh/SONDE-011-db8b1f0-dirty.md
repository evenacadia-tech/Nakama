# Rohausgabe - SONDE-011 Phase A

**Lauf:** 2026-08-31 06:16 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 33/33 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0

**Manifest:** [../SONDE-011.md](../SONDE-011.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-31 06:16:44 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | db8b1f0 SONDE-011 Phase A Runde 3: Kern identitaetsfrei — Domain-Tag und FlatBuffers-Namespace ohne Herstellernadel |
| Commit (voll) | db8b1f0eb9828e80993d2ee44ccac3327b53a387 |
| Arbeitsbaum | 2 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/beweise/SONDE-011.md
?? docs/beweise/roh/SONDE-011-b327c1d.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-31 06:16:47 | `7AD87EF1236F7F78` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-29 19:34:14 | `CC3B4291A48EBFA6` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-31 06:16:51 | `122BA35B45F8ADAC` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-29 19:34:53 | `349EB193563ED663` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-31 06:16:55 | `B8FABE423F558674` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-31 06:16:58 | `055FA8AD4E6CC03F` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-31 06:17:00 | `2A54899C17883EB9` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-29 19:36:27 | `676D474494BB10C5` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-29 19:36:55 | `C80BBF20C87F1522` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-31 06:17:04 | `88B424353B34388E` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-31 06:17:07 | `7B1EA411065D2F9C` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-29 19:37:41 | `3BC12A458CB9D51E` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-31 06:17:10 | `D7740539ACF15DF8` | frisch (Bau bestaetigt) |
| `EqCopIpcTest` | 2026-08-31 06:17:12 | `5FA948679ACF8680` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-31 06:17:14 | `D6AE6896383B10D8` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-31 06:01:06**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,09 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,64 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,70 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server, v3-Coordinator, NAK-40-Alias/Kollisionsquarantaene und sticky Interventionszustand). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,16 s | [↓ A4](#a4) |
| A4-SI | SONDE-011/28-B Server-Integration auf Probe-Pipenamen: subscribe_session ist an Adresse, Session und eigenen Control-Link gebunden; Fremdadressen fallen; EOF, Protokoll-/Writefehler, Timeout-Hook und Serverstopp entfernen nur den eigenen Besitz atomar vor jedem spaeteren Push. | `cargo test --manifest-path broker/Cargo.toml --color never subscription_` | [OK] Exit 0 | 0,73 s | [↓ A4-SI](#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,07 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,80 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,19 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen und band_stereo ID 10 samt saturated-/Grenzfaellen sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,49 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,22 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 5,42 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen (Zielkollision, state_schema-Drift, Stilllegung umgangen, Marke unlesbar, ungueltige ziel_id-Typen, Pfadtraversal) fallen einzeln. [4] vergleicht im Kanon weich (Abweichung nach Relink = Hinweis, fehlendes Artefakt oder nicht bildbarer Ordner-Hash = Fehler), hart nur mit --release; [4b] berichtet den installierten Stand ohne Urteil, ok nur bei Journalstatus OK; [5] Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide ab. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft, jede unerwartete Ausnahme endet als Klartext mit Exit 2 statt als Traceback. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,00 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 34,96 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A19](#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [↓ A20](#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,33 s | [↓ A21](#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,50 s | [↓ A22](#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,09 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,10 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,32 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,25 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,45 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,11 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 51,19 s | [↓ B10](#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,12 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,09 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,64 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,70 s

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
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 3,16 s

stdout:

```text

running 175 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test coordinator::tests::heartbeat_ack_meldet_alias_quarantaene_fuer_beide_links ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test coordinator::tests::hoermarkierung_tail_sperrt_bis_ende ... ok
test coordinator::tests::hoermarkierung_v2_v3_gleicher_zustand ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test coordinator::tests::hoermarkierung_vor_evidence_dispatch ... ok
test coordinator::tests::instance_alias_gleiche_id_in_verschiedenen_sitzungsadressraeumen_kollidiert_nicht ... ok
test coordinator::tests::intervention_control_disconnect_setzt_sticky_unknown ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test coordinator::tests::intervention_falsche_end_id_bleibt_gesperrt ... ok
test coordinator::tests::intervention_neutral_resync_entsperrt ... ok
test coordinator::tests::intervention_overflow_setzt_sticky_unknown ... ok
test coordinator::tests::intervention_sequenzluecke_setzt_sticky_unknown ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test coordinator::tests::intervention_ueberlappung_bleibt_bis_alle_end ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test coordinator::tests::subscription_cleanup_vor_weiterem_push ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test instance_alias::tests::instance_address_alias_vectors ... ok
test instance_alias::tests::instance_address_empty_uses_fresh_uuid ... ok
test instance_alias::tests::instance_alias_collision_abgeleitet_dann_native_quarantaenisiert_beide ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test instance_alias::tests::instance_alias_collision_native_dann_abgeleitet_quarantaenisiert_beide ... ok
test instance_alias::tests::instance_alias_collision_zwei_ableitungen_quarantaenisiert_beide ... ok
test instance_alias::tests::instance_alias_target_recomputed_and_matched ... ok
test instance_alias::tests::instance_alias_unknown_fail_closed ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test transport::bootstrap::tests::binaerframe_statt_hello_wird_abgelehnt ... ok
test transport::bootstrap::tests::hello_ueber_16_kib_faellt_vor_dem_parser ... ok
test transport::bootstrap::tests::hex32_riegel ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test transport::bootstrap::tests::host_haelt_die_laengen_des_vertrags ... ok
test transport::bootstrap::tests::koppeln_und_entkoppeln_sind_symmetrisch ... ok
test transport::bootstrap::tests::kopplung_verlangt_alle_drei_merkmale ... ok
test transport::bootstrap::tests::offene_kopplungen_sind_gedeckelt ... ok
test transport::bootstrap::tests::unbekanntes_feld_im_hello_faellt ... ok
test transport::bootstrap::tests::v2_hello_bleibt_v2 ... ok
test transport::bootstrap::tests::v3_control_wird_vollstaendig_validiert ... ok
test transport::legacy_v2::tests::echtes_v2_json_geht_weiterhin_durch ... ok
test transport::legacy_v2::tests::v2_json_ist_kein_gueltiger_v3_envelope ... ok
test transport::legacy_v2::tests::v3_binaerframe_wird_nie_als_v2_json_gedeutet ... ok
test transport::pipetoken::tests::base32_gegen_rfc_4648 ... ok
test transport::pipetoken::tests::golden_aus_dem_entwurf ... ok
test transport::pipetoken::tests::probe_namensraum_laesst_nur_probe_namen_durch ... ok
test transport::pipetoken::tests::schreibweise_der_sid_ist_egal ... ok
test transport::pipetoken::tests::tokenform_ist_stabil ... ok
test transport::pipetoken::tests::verschiedene_sids_verschiedene_token ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test transport::server_v3::tests::geschlossener_eingang_liefert_nichts_mehr ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test transport::server_v3::tests::blockierende_senke_haelt_den_leser_nicht_auf ... ok
test transport::server_v3::tests::kopplung_faellt_mit_dem_leserende_nicht_erst_nach_den_joins ... ok
test transport::server_v3::tests::p0_auf_der_telemetriepipe_wird_abgewiesen ... ok
test transport::server_v3::tests::p0_wird_beantwortet_waehrend_p1_die_senke_blockiert ... ok
test transport::server_v3::tests::beendete_verbindungen_werden_geerntet ... ok
test transport::server_v3::tests::p2_auf_der_controlpipe_wird_abgewiesen ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok
test transport::server_v3::tests::subscription_ist_an_eigenen_control_link_gebunden ... ok
test transport::server_v3::tests::connect_callbacks_je_kopplung_genau_einmal ... ok
test transport::server_v3::tests::control_ende_beendet_die_telemetrie ... ok
test transport::server_v3::tests::p0_ueberlauf_trennt_die_verbindung ... ok
test transport::v3::tests::crc32c_gegen_bekannte_vektoren ... ok
test transport::v3::tests::grenzen_des_rahmens ... ok
test transport::v3::tests::jeder_regelname_ist_rueckuebersetzbar ... ok
test transport::v3::tests::json_rundlauf ... ok
test transport::v3::tests::p2_traegt_pflicht_crc ... ok
test transport::v3::tests::ratengrenze_haelt_das_fenster ... ok
test transport::v3::tests::stromleser_vertraegt_byteweise_zustellung ... ok
test transport::v3::tests::stufe_drei_sammelt_und_sortiert ... ok
test transport::v3::tests::u32_ueberlauf_wird_nicht_akzeptiert ... ok
test transport::warteschlange::tests::ingress_droppt_p2_zuerst_und_trennt_bei_p0 ... ok
test transport::warteschlange::tests::ingress_liefert_p0_getrennt_vom_rest ... ok
test transport::warteschlange::tests::ingress_voll_ohne_p2_trennt_auch_bei_p1 ... ok
test transport::warteschlange::tests::p0_verwirft_nie_und_meldet_ueberlauf ... ok
test transport::warteschlange::tests::p1_haelt_ereignisse_fuer_den_reconnect_vor ... ok
test transport::warteschlange::tests::p1_koalesziert_snapshots_an_ihrer_position ... ok
test transport::warteschlange::tests::p1_neuzugang_ueberholt_keine_wiederholung ... ok
test transport::warteschlange::tests::p1_wiederholpuffer_fliesst_ohne_reconnect_ab ... ok
test transport::warteschlange::tests::p1_wiederholpuffer_haelt_den_schluessel ... ok
test transport::warteschlange::tests::p2_ersetzt_den_aeltesten_ungesendeten ... ok
test transport::warteschlange::tests::p2_flut_hungert_p0_nicht_aus ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::discriminator_boolean_falscher_typ ... ok
test vertrag::tests::discriminator_boolean_fehlt ... ok
test vertrag::tests::discriminator_boolean_true_false ... ok
test vertrag::tests::discriminator_json_pointer_boolean ... ok
test vertrag::tests::discriminator_json_pointer_segment_fehlt ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test transport::server_v3::tests::subscription_cleanup_vor_weiterem_push ... ok
test transport::server_v3::tests::trennreihenfolge_je_callback_genau_einmal ... ok
test transport::server_v3::tests::stop_im_fenster_vor_der_bedienung_haengt_nicht ... ok
test transport::server_v3::tests::welcome_folgt_dem_abgeschlossenen_control_verbunden ... ok
test transport::server_v3::tests::acceptor_ueberlebt_die_verbindungsgrenze ... ok
test transport::server_v3::tests::telemetrie_endet_zuerst_control_getrennt_folgt_trotzdem ... ok
test transport::server_v3::tests::trennreihenfolge_haelt_im_kopplungs_callback_fenster ... ok
test transport::server_v3::tests::abgeloestes_telemetrie_getrennt_haelt_control_getrennt_nicht_auf ... ok
test transport::server_v3::tests::abgeloestes_telemetrie_gekoppelt_laesst_control_getrennt_nicht_warten ... ok
test transport::server_v3::tests::stoppen_endet_auch_bei_haengendem_lebenszyklusaufruf ... ok
test transport::server_v3::tests::stoppen_endet_auch_bei_haengender_senke ... ok

test result: ok. 175 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 2.41s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 9 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test envelope_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.16s


running 9 tests
test ein_kaputter_frame_im_strom_beendet_den_strom ... ok
test feindliche_laengen_enden_in_der_erwarteten_fehlerklasse ... ok
test ratengrenze_haelt_unter_flut ... ok
test zufallsbytes_bringen_den_pruefer_nie_aus_dem_tritt ... ok
test jedes_gekippte_payloadbit_eines_p2_frames_faellt_an_der_crc ... ok
test payload_an_der_paketgrenze_wird_angenommen_darueber_nicht ... ok
test schreiben_und_pruefen_sind_zueinander_invers ... ok
test fragmentierung_aendert_das_ergebnis_nie ... ok
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.16s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.06s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-95fc09d40727420b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-af1d3f20e690c9d6.exe)
     Running unittests src\bin\eqcop-broker-v3probe.rs (broker\target\debug\deps\eqcop_broker_v3probe-469dcf6e3c80a0af.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-0cc75d7ee20a9bba.exe)
     Running tests\transport_fuzz.rs (broker\target\debug\deps\transport_fuzz-c05695697064a160.exe)
   Doc-tests eqcop_broker
```

<a id="a4-si"></a>
#### A4-SI | subscription_server_integration

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never subscription_` | **Exitcode:** 0 | **Dauer:** 0,73 s

stdout:

```text

running 3 tests
test coordinator::tests::subscription_cleanup_vor_weiterem_push ... ok
test transport::server_v3::tests::subscription_ist_an_eigenen_control_link_gebunden ... ok
test transport::server_v3::tests::subscription_cleanup_vor_weiterem_push ... ok

test result: ok. 3 passed; 0 failed; 0 ignored; 0 measured; 172 filtered out; finished in 0.52s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 9 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 9 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-95fc09d40727420b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-af1d3f20e690c9d6.exe)
     Running unittests src\bin\eqcop-broker-v3probe.rs (broker\target\debug\deps\eqcop_broker_v3probe-469dcf6e3c80a0af.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-0cc75d7ee20a9bba.exe)
     Running tests\transport_fuzz.rs (broker\target\debug\deps\transport_fuzz-c05695697064a160.exe)
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,07 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [328 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,80 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
53 Definitionen, 17 Nachrichtenfamilien, 220 Fixtures
Pruefungen: 333 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties     10 / 50  
  const                    12 / 12   HART
  enum                     13 / 13   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 5 / 21  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                    10 / 16  
  pattern                   4 / 9   
  required                 46 / 252 
  type                     12 / 172 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:1081: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

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

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

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

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
48 gueltige, 172 ungueltige Fixtures
  bytegleich: 222 Dateien, MANIFEST sha256=7b1baba87ec4c2d4016ccc1bd7b44426550e57cf71823f52fcbf67a527cfcec6
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=7393d08f318a1aa599c10a12851f377db1b764ba1387501678f72b6f792bab2a  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1202 Zeilen  sha256=b929657011afd5e4...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2042 Zeilen  sha256=4cfefcf6bad8e267...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 1,49 s

stdout:

```text
14 gueltige, 72 ungueltige Fixtures
  bytegleich: 87 Dateien, MANIFEST sha256=77f0508c02983bd0092211464b94ea5f65d55805c7971f6a92cad3542cdb2bd2
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

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

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,25 s

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

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,22 s

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

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 5,42 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (1404806 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7107584 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - der Kern wurde fuer diese Messung neu gebaut
  ok      Kernartefakte geloescht und in 4.9s neu erzeugt (22 Dateien entfernt, 9 Uebersetzungseinheiten, 9 Objekte, Lib neu gelinkt)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii,fourcc-int-be]
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
  ok      Archivmitglieder sind genau die 9 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

25 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 1,00 s

stdout:

```text
Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
                 Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
  ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an verdorbener Eingabe
  ok      faellt an verdorbener Eingabe: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt an verdorbener Eingabe: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt an verdorbener Eingabe: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt an verdorbener Eingabe: Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag
  ok      faellt an verdorbener Eingabe: jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      faellt an verdorbener Eingabe: jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
  ok      faellt an verdorbener Eingabe: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt an verdorbener Eingabe: kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
  ok      faellt an verdorbener Eingabe: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt an verdorbener Eingabe: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt an verdorbener Eingabe: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt an verdorbener Eingabe: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt an verdorbener Eingabe: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt an verdorbener Eingabe: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt an verdorbener Eingabe: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird
  ok      faellt, wenn ein stillgelegtes Ziel nirgends benannt ist
  ok      faellt, wenn die Stilllegungsmarke null ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke null ist
  ok      faellt, wenn die Stilllegungsmarke eine Zeichenkette ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke eine Zeichenkette ist
  ok      faellt, wenn die Stilllegungsmarke ein leeres Array ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein leeres Array ist
  ok      faellt, wenn die Stilllegungsmarke eine Zahl ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke eine Zahl ist
  ok      faellt, wenn die Stilllegungsmarke ein Boolean ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Boolean ist
  ok      faellt, wenn die Stilllegungsmarke ein leeres Objekt ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein leeres Objekt ist
  ok      faellt, wenn die Stilllegungsmarke ein Objekt mit leerem `am` ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Objekt mit leerem `am` ist
  ok      faellt, wenn die Stilllegungsmarke ein Objekt ohne `entscheid` ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Objekt ohne `entscheid` ist
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine leere Liste
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist ein Objekt
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine leere Zeichenkette
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist nur Leerraum
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine Zahl
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id fehlt ganz
  ok      faellt kontrolliert bei gemischten ziel_id-Typen in einer Liste
  ok      faellt, wenn `seit` null ist
  ok      faellt, wenn `seit` ein leeres Array ist
  ok      faellt, wenn `seit` ein leeres Objekt ist
  ok      faellt, wenn `seit` leer ist
  ok      faellt, wenn `seit` nur Leerraum ist
  ok      faellt, wenn `seit` eine Zahl ist
  ok      faellt, wenn `warum` null ist
  ok      faellt, wenn `warum` ein leeres Array ist
  ok      faellt, wenn `warum` ein leeres Objekt ist
  ok      faellt, wenn `warum` leer ist
  ok      faellt, wenn `warum` nur Leerraum ist
  ok      faellt, wenn `warum` eine Zahl ist
  ok      faellt, wenn `umgang_mit_altbestand` null ist
  ok      faellt, wenn `umgang_mit_altbestand` ein leeres Array ist
  ok      faellt, wenn `umgang_mit_altbestand` ein leeres Objekt ist
  ok      faellt, wenn `umgang_mit_altbestand` leer ist
  ok      faellt, wenn `umgang_mit_altbestand` nur Leerraum ist
  ok      faellt, wenn `umgang_mit_altbestand` eine Zahl ist
  ok      faellt, wenn `kennung_bleibt` null ist
  ok      faellt, wenn `kennung_bleibt` ein leeres Array ist
  ok      faellt, wenn `kennung_bleibt` ein leeres Objekt ist
  ok      faellt, wenn `kennung_bleibt` leer ist
  ok      faellt, wenn `kennung_bleibt` nur Leerraum ist
  ok      faellt, wenn `kennung_bleibt` eine Zahl ist
  ok      faellt, wenn ein aktives Ziel still aus der Auslieferung faellt
  ok      faellt an einem Produktnamen ausserhalb der Pfade
  ok      faellt an einem Bundlenamen ausserhalb der Pfade
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand  - Kanon: eine Abweichung ist ein Hinweis, kein Fehler
  hinweis main: Bau weicht vom festgeschriebenen Paket ab (nach Relink erwartet; vor einer Auslieferung --hashen)  [Manifest AC8102F23EDC7D7C | gebaut 52407773584D2A5B]
  hinweis active-probe: Bau weicht vom festgeschriebenen Paket ab (nach Relink erwartet; vor einer Auslieferung --hashen)  [Manifest 1DDC92E3B8525F1F | gebaut 61BB12BDD9D31385]
  ok      eqcop-broker.exe: gebautes Artefakt stimmt mit dem festgeschriebenen Hash  [21C7A8DC985BCA16]

[4b] Installierter Stand  - Bericht, kein Urteil
  Journal: status='OK'  zeit='2026-08-29T09:46:53.0057417Z'
  hinweis main: installierter Stand ist ein anderer als der im Manifest festgeschriebene  [installiert 4E0BED966D834BC1 | Manifest AC8102F23EDC7D7C]  C:\Program Files\Common Files\VST3\EQ-Copilot.vst3
  hinweis active-probe: installierter Stand ist ein anderer als der im Manifest festgeschriebene  [installiert AD7678B7C34A64FE | Manifest 1DDC92E3B8525F1F]  C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3
  hinweis eqcop-broker.exe: installierter Stand ist ein anderer als der im Manifest festgeschriebene  [installiert 53808359C59B5D09 | Manifest 21C7A8DC985BCA16]  C:\Program Files\evenacadia\Nakama\eqcop-broker.exe

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

95 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 34,96 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-c6u7b5cw\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-c6u7b5cw\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: m_null).
  ok      Nakama Suna : stillgelegt seit 2026-08-28, nicht installiert
  ok      EQ-Copilot installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-c6u7b5cw\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-c6u7b5cw\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-c6u7b5cw\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle 3 Artefakte melden `aktuell`  [3x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[3b] Ein stillgelegtes Bundle wird gemeldet, nicht geloescht
  ok      das vorgefundene stillgelegte Bundle wird mit vollem Pfad gemeldet
  ok      die Meldung nennt den Handgriff, statt nur zu klagen
  ok      und der Installer hat es NICHT angefasst - bytegleich vorgefunden
  ok      ohne Altbestand bleibt die Zeile ehrlich (`nicht installiert`)

[3c] Eine Auslieferung ganz OHNE stillgelegtes Ziel laeuft weiter
  ok      ohne stillgelegtes Ziel laeuft -Pruefen durch und schweigt darueber  [Exit 0]

[3d] Eine unlesbare Stilllegungsmarke sperrt, statt 'aktiv' zu heissen
  ok      Marke vom Typ null bricht ab und benennt sich  [Exit 1]
  ok      Marke vom Typ String bricht ab und benennt sich  [Exit 1]
  ok      Marke vom Typ Array bricht ab und benennt sich  [Exit 1]
  ok      Marke vom Typ Zahl bricht ab und benennt sich  [Exit 1]
  ok      auch MIT geschmuggeltem Artefakteintrag sperrt die Marke zuerst  [Exit 1]

[3e] Ein unbrauchbares Pflichtfeld bricht ab, statt Luecken zu drucken
  ok      `seit` als null bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = null)
  ok      `seit` als Array bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = Array)
  ok      `seit` als Objekt bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = Objekt)
  ok      `seit` als leer bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = leer)
  ok      `seit` als Leerraum bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = Leerraum)
  ok      `warum` als null bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (warum = null)
  ok      `warum` als Array bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (warum = Array)
  ok      `warum` als Objekt bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (warum = Objekt)
  ok      `warum` als leer bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (warum = leer)
  ok      `warum` als Leerraum bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (warum = Leerraum)
  ok      `umgang_mit_altbestand` als null bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (umgang_mit_altbestand = null)
  ok      `umgang_mit_altbestand` als Array bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (umgang_mit_altbestand = Array)
  ok      `umgang_mit_altbestand` als Objekt bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (umgang_mit_altbestand = Objekt)
  ok      `umgang_mit_altbestand` als leer bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (umgang_mit_altbestand = leer)
  ok      `umgang_mit_altbestand` als Leerraum bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (umgang_mit_altbestand = Leerraum)
  ok      `kennung_bleibt` als null bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (kennung_bleibt = null)
  ok      `kennung_bleibt` als Array bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (kennung_bleibt = Array)
  ok      `kennung_bleibt` als Objekt bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (kennung_bleibt = Objekt)
  ok      `kennung_bleibt` als leer bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (kennung_bleibt = leer)
  ok      `kennung_bleibt` als Leerraum bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (kennung_bleibt = Leerraum)

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

127 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a20"></a>
#### A20 | erzeuge_envelope_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,31 s

stdout:

```text
8 gueltige, 29 ungueltige Envelope-Fixtures; 14 Regeln, jede mit Negativfall
  bytegleich: 38 Dateien, MANIFEST sha256=efa50c0384865d8a3d4eb560c7037937b60af6a4f32731f7d7dc5e953cf13307
```

stderr:

_(leer)_

<a id="a21"></a>
#### A21 | transport_fuzz

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | **Exitcode:** 0 | **Dauer:** 0,33 s

stdout:

```text

running 9 tests
test ein_kaputter_frame_im_strom_beendet_den_strom ... ok
test feindliche_laengen_enden_in_der_erwarteten_fehlerklasse ... ok
test ratengrenze_haelt_unter_flut ... ok
test zufallsbytes_bringen_den_pruefer_nie_aus_dem_tritt ... ok
test jedes_gekippte_payloadbit_eines_p2_frames_faellt_an_der_crc ... ok
test payload_an_der_paketgrenze_wird_angenommen_darueber_nicht ... ok
test schreiben_und_pruefen_sind_zueinander_invers ... ok
test fragmentierung_aendert_das_ergebnis_nie ... ok
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.16s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running tests\transport_fuzz.rs (broker\target\debug\deps\transport_fuzz-c05695697064a160.exe)
```

<a id="a22"></a>
#### A22 | pruefe_ipc_last.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | **Exitcode:** 0 | **Dauer:** 10,50 s

stdout:

```text
  ok      beide Probeprogramme verweigern Golden- und v1-Pipename
Probe-Pipe: \\.\pipe\evenacadia.nakama.v3.probe.last.21288.1788149961
  BEREIT \\.\pipe\evenacadia.nakama.v3.probe.last.21288.1788149961
SONDE-010 | Lastbein: 32 Sonden, 8 s, Pipe \\.\pipe\evenacadia.nakama.v3.probe.last.21288.1788149961
  verbunden: 32 Control, 32 Telemetrie
{"sonden":32,"sekunden":8,"p2_veroeffentlicht":128832,"p2_gesendet":94840,"p2_ersetzt":33993,"p2_abgelehnt":0,"p2_zu_gross":0,"p2_kollisionsloecher":0,"p2_neueste_verworfen":0,"p0_gesendet":9952,"p0_beantwortet":9952,"p0_latenz_max_ms":21,"p0_latenz_p99_ms":20,"p0_ueberlaeufe":0,"envelope_abweisungen":0}
  ok      P0-Befehle wurden ueberhaupt gesendet  [9952]
  ok      KEIN P0-Frame geht verloren  [9952/9952]
  ok      P0-Antwortlatenz bleibt beschraenkt  [max 21 ms, p99 20 ms, Schranke 1000 ms]
  ok      es lag WIRKLICH Rueckstau an (Cap 2 hat ersetzt)  [33993 ersetzte P2-Frames]
  ok      und dabei faellt NIE der neueste Frame (replace-oldest)  [0 neueste verworfen, 0 Positionen uebersprungen]
  ok      jede abgelehnte Veroeffentlichung hat einen gezaehlten Grund  [0 abgelehnt = 0 zu gross + 0 ohne Platz]
  ok      kein Envelope wurde abgewiesen  [0]
  ok      keine P0-Queue lief ueber  [0]
LASTBEIN GRUEN
Brokerbericht: {"abweisungen": 0, "acceptor_wartet_auf_instanz": 0, "angenommen": 64, "control_getrennt": 32, "control_verbindungen": 32, "geschlossen_bootstrap": 0, "geschlossen_envelope": 0, "geschlossen_familie": 0, "geschlossen_kopplung": 0, "geschlossen_p0_ueberlauf": 0, "geschlossen_rate": 0, "geschlossen_writer": 0, "ingress_hoechststand": 3, "ingress_p1_ueberlauf_trennt": 0, "ingress_p2_verworfen": 0, "letzter_grund": "", "p0": 9952, "p0_beantwortet": 9952, "p1": 0, "p2": 94840, "pipe": "\\\\.\\pipe\\evenacadia.nakama.v3.probe.last.21288.1788149961", "telemetrie_getrennt": 32, "telemetrie_verbindungen": 32}
  ok      das C++-Lastbein selbst ist gruen  [Exit 0]
  ok      der Rust-Broker hat einen Bericht geliefert  [23 Felder]
  ok      32 Control-Verbindungen im Broker angekommen  [32]
  ok      32 Telemetrieverbindungen GEKOPPELT (nicht nur verbunden)  [32]
  ok      der Broker hat P0 gesehen und beantwortet  [9952 empfangen / 9952 beantwortet]
  ok      und P2 ueber die zweiten Verbindungen  [94840]
  ok      keine Verbindung wegen Envelope, Rate oder P0-Ueberlauf geschlossen  [envelope=0 rate=0 p0=0]
  ok      kein P0-Frame geht verloren (Clientseite)  [9952/9952]
  ok      es lag wirklich Rueckstau an  [33993 ersetzte P2-Frames]
  ok      und dabei fiel nie der neueste Frame (replace-oldest)  [0 neueste verworfen, 0 Positionen uebersprungen]
  ok      jede abgelehnte Veroeffentlichung hat einen gezaehlten Grund  [0 abgelehnt = 0 zu gross + 0 ohne Platz]
GRUEN
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,09 s

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
  ok      die Probebytes tragen wirklich measurement_position=post_fader_contribution  [post_fader_contribution]
  ok      der Stand kommt als read-only zurueck, nicht als eigener  [measurement_position post_fader_contribution is not allowed for active_probe]
  ok      read-only nennt seinen Grund
  ok      das Bundle FUEHRT die verbotene Position nicht  [insert]
  ok      die Instanz klassifiziert NICHT auf die Produktklasse, sie bleibt neutral  [unclassified]
  ok      eine read-only-Sonde darf den Broker nicht starten
  ok      Vertrag 53.8: dieselben Originalbytes gehen unveraendert an den Host zurueck  [3456 Bytes]
  ok      der Rueckweg waescht nichts: erneut geladen bleibt read-only  [measurement_position post_fader_contribution is not allowed for active_probe]
  ok      und erneut geladen bleibt die Instanz neutral  [unclassified]
  ok      Gegenprobe: derselbe Stand mit erlaubter Position laedt normal
  ok      Gegenprobe: der geladene Common ist derselbe (Position insert)  [insert]
  ok      Gegenprobe: nach gueltigem Stand traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      Nachreichen: die Instanz ist VOR dem verbotenen Stand klassifiziert  [active_probe]
  ok      Nachreichen: der verbotene Stand kommt auch bei einer laufenden Instanz read-only  [measurement_position post_fader_contribution is not allowed for active_probe]
  ok      Nachreichen: die Klassifikation faellt zurueck auf neutral, alte Rechte bleiben nicht  [unclassified]
  ok      Nachreichen: der Host bekommt genau die Bytes zurueck, die er gab - kein stiller Tausch  [3456 Bytes]
  ok      positionErlaubt (Riegel 1 und Klassenmatrix gemeinsam) lehnt post_fader_contribution ab fuer 'main'
  ok      positionErlaubt (Riegel 1 und Klassenmatrix gemeinsam) lehnt post_fader_contribution ab fuer 'passive_probe'
  ok      positionErlaubt (Riegel 1 und Klassenmatrix gemeinsam) lehnt post_fader_contribution ab fuer 'active_probe'
  ok      positionErlaubt (Riegel 1 und Klassenmatrix gemeinsam) lehnt post_fader_contribution ab fuer 'legacy'
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 82 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,10 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt mindestens ein Ziel  [3]
  ok      mindestens ein Ziel ist nicht stillgelegt  [2 aktiv, 1 stillgelegt]
  ok      jede vorhandene Stilllegungsmarke ist ein lesbares Objekt  [0 unlesbar]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes AKTIVE Ziel im Manifest hat hier eine Zeile  [2 vs 2]
  ok      jedes STILLGELEGTE Ziel im Manifest hat hier eine Zeile  [1 vs 1]
  ok      main: steht als AKTIVES Ziel im Manifest
  ok      active-probe: steht als AKTIVES Ziel im Manifest
  ok      passive-probe: die Kennung steht weiter im Manifest
  ok      passive-probe: und ist dort als stillgelegt markiert
  ok      passive-probe: plugin_code ist nicht geloescht  [NkPr]
  ok      passive-probe: bundle ist nicht geloescht  [Nakama Suna.vst3]
  ok      passive-probe: produktname ist nicht geloescht  [Nakama Suna]
  ok      passive-probe: component_cid ist nicht geloescht  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: controller_cid ist nicht geloescht  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: die Stilllegung nennt Datum und Entscheid  [2026-08-28 / design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md]
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
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      NakamaSuna: stillgelegt - kein Aufruf von nakama_sonde_ziel() mehr
  ok      NakamaSuna: stillgelegt - auch kein eigener juce_add_plugin-Block
  ok      NakamaSuna: das Bauskript erklaert die Stilllegung an Ort und Stelle
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [31 Aug 2026 6:17:40am vs 31 Aug 2026 3:14:22am]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [31 Aug 2026 6:18:06am vs 31 Aug 2026 3:14:22am]
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

IDENTITY-TEST OK - 116 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,32 s

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
  ok      Matrix Klasse=main, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=main, Position=pre: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position pre is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=legacy, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for legacy', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=passive_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for passive_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=active_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for active_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      post_fader_contribution_bleibt_fuer_alle_sondenklassen_gesperrt
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      instance_address_alias_vectors  [6/6]
  ok      instance_alias_target_recomputed_and_matched
  ok      instance_alias_unknown_fail_closed
  ok      instance_address_empty_uses_fresh_uuid  [ce77cec816cf48d5ac534fe4d33d0b8b]
  ok      legacy_instance_id_save_load_bytegleich
  ok      == NAK-40 Aliasvektoren, Zielvergleich und Save+Load
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [3ea74be3db5a4bea889af9cffe6d9f2b]
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
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [37a46f41f0c245d7a10145c1bd1fb1a7]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 176 Pruefungen ok, 0 Fehler
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

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260831-061932.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (20013 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
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
[ok]   discriminator_boolean_true_false
[ok]   discriminator_boolean_falscher_typ
[ok]   discriminator_boolean_fehlt
[ok]   discriminator_json_pointer_boolean
[ok]   discriminator_json_pointer_segment_fehlt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (86 Fixtures)
[ok]   Binaerkorpus hat Substanz  [86 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   band_stereo_featureframe_to_flatbuffer
[ok]   nak29_encoder_project_samples_ohne_project_time_bit
[ok]   nak29_encoder_local_monotonic_mit_project_sample_start
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (220 Fixtures)
[ok]   Korpus hat Substanz  [220 Fixtures]
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
71 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,45 s

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
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
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
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
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
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3853 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
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
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
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
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,11 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 1: project_samples + project_sample_start, aber KEIN project_time-Bit
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 2: local_monotonic MIT project_sample_start (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [33 Bloecke]
[ok]   und mindestens einen Frame gebaut  [3 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 237 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b10"></a>
#### B10 | EqCopIpcTest

**Befehl:** `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | **Exitcode:** 0 | **Dauer:** 51,19 s

stdout:

```text
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  ok      MANIFEST.json gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\v3\envelope\MANIFEST.json]
  ok      Regelmenge und ihre Reihenfolge stimmen mit dem Manifest  [14 Regeln]
  ok      die SAMMELNDE Stufe 3 ist in Deklarationsreihenfolge namenssortiert
  ok      alle Fixtures klassifiziert wie das Manifest (Urteil UND Verstossmenge)  [8 gueltig / 29 ungueltig]
  ok      Korpus ist substanziell  [37 Fixtures]
  ok      jede der 14 Regeln hat mindestens ein Negativfixture  [14/14]

== B · CRC32C und Envelope-Rundlauf ==
  ok      CRC32C("") == 0
  ok      CRC32C("123456789") == 0xE3069283 (RFC 3720)
  ok      CRC32C(32x 0x00) == 0x8A9136AA
  ok      CRC32C(32x 0xFF) == 0x62A8AB43
  ok      P0-Frame geschrieben
  ok      P0 ist JSON und traegt CRC exakt 0
  ok      P2 traegt die Pflicht-CRC ueber genau die Payloadbytes
  ok      ein gekipptes Payloadbit faellt an der CRC
  ok      Payload an der Paketgrenze wird angenommen
  ok      ein Byte darueber wird nicht einmal geschrieben

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  ok      kein angenommener Zufallsframe verletzt eine Kopfregel  [0 angenommen / 40000 abgelehnt]
  ok      ein gekipptes Bit in einem GUELTIGEN Frame: was durchkommt, haelt jede Kopfregel  [7671 angenommen / 12329 abgelehnt]
  ok      3000 gekippte P2-Payloadbits: jedes einzelne faellt an der CRC
  ok      byteweise Zustellung liefert exakt dieselben 40 Frames  [40 Frames, Rest 0]
  ok      ein kaputter Frame im Strom wird gemeldet, nicht uebersprungen

== D · v3-Pipename als reine Funktion ==
  ok      Golden aus Entwurf §48.3
  ok      vollstaendiger Pipename
  ok      Schreibweise der SID ist egal (Grossschreibung vor dem Hashen)
  ok      verschiedene SIDs, verschiedene Token
  ok      der Golden-Pipename wird als Probe-Name VERWEIGERT
  ok      die v1-Produktions-Pipe ebenso
  ok      und ein v3-Name ausserhalb des Probe-Namensraums
  ok      `probe.` allein ist keine Pipe, sondern nur der Namensraum
  ok      der eigene Testname liegt im Probe-Namensraum
  ok      SHA-256("abc") wie FIPS 180-4
  ok      SHA-256 des 56-Byte-Vektors (Zweiblockfall)
  ok      Base32 gegen die Testvektoren aus RFC 4648 §10
  ok      eigene Logon-SID ist wohlgeformt  [S-1-5-21-369263037-1611576211-2081231688-1001]

== E · Backpressure: P0 · P1 · P2 ==
  ok      P0 nimmt 64 Nachrichten
  ok      die 65. meldet Ueberlauf (nichts verwerfen ⇒ Verbindung schliessen)
  ok      alle 64 kommen unveraendert und in Reihenfolge zurueck
  ok      der Sender entnimmt den ersten Befehl und reserviert seinen Platz
  ok      64 neue Befehle waehrend des Writes finden KEINEN Platz — ehrlich gezaehlt  [0 angenommen, 64 Ueberlaeufe]
  ok      der gescheiterte Write legt ihn zurueck; die Queue traegt wieder genau 64  [64 Eintraege]
  ok      und zwar VORN — kein P0-Befehl ist verlorengegangen
  ok      P1 nimmt Snapshot a
  ok      P1 nimmt Ereignis
  ok      P1 nimmt Snapshot b
  ok      ein zweiter Snapshot desselben Objekts koalesziert, ohne zu wachsen
  ok      Koaleszieren behaelt die Position, tauscht nur den Inhalt
  ok      nicht koaleszierbare Ereignisse gehen in den Wiederholpuffer
  ok      ein voller Wiederholpuffer weist das NEUE Ereignis ab, gezaehlt, nie still  [2 vorgehalten, 1 abgewiesen]
  ok      ein einziges Senden zieht eine Wiederholung nach — ohne Reconnect  [1 noch vorgehalten]
  ok      beim Reconnect ist nichts mehr nachzuholen — es floss schon ab
  ok      und zwar JEDES angenommene Ereignis, in der urspruenglichen Reihenfolge  [3,4]
  ok      ein Neuzugang findet keinen Platz, solange eine Wiederholung wartet
  ok      und er ueberholt sie nicht — Annahmereihenfolge ueber beide Puffer  [2,3]
  ok      ein Snapshot im Wiederholpuffer behaelt seinen Schluessel und koalesziert dort  [1 vorgehalten, 0 abgewiesen]
  ok      und der NEUERE geht raus, nicht der aeltere  [fremd,neu]
  ok      entnehmen liefert Objektschluessel UND Nachricht
  ok      ein gescheiterter Write legt das Ereignis an seinen Platz zurueck
  ok      und es steht wieder VORN, nicht hinten
  ok      ein zurueckgelegter Snapshot weicht dem neueren, statt ihn zu verdraengen
  ok      und der neuere geht raus
  ok      der entnommene P1-Eintrag zaehlt weiter gegen die Kapazitaet
  ok      ein neues Ereignis findet deshalb keinen Platz und wartet auf den Reconnect
  ok      und der unterwegs gewesene Eintrag kommt an SEINEN Platz zurueck
  ok      ein nicht geschriebener P0-Befehl geht ebenfalls zurueck
  ok      und behaelt seine Reihenfolge

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  ok      der dritte Frame ersetzt den aeltesten ungesendeten (Cap 2)
  ok      abgeholt wird der zweite, nicht der erste  [2]
  ok      danach der dritte
  ok      danach ist die Schleuse leer
  ok      ein Frame ueber der Slotgroesse wird gezaehlt verworfen, nie halb geschrieben
  ok      100 000 Uebergaben mit 0 Allokationen  [0]
  ok      Gegenprobe: derselbe Zaehler sieht eine echte Allokation  [1]
  ok      2 000 000 kleine Frames unter Flut: kein zerrissener Frame  [379406 geholt, 0 zerrissen, 1990263 ersetzt]
  ok      300 000 grosse Frames (8000 B) ebenso: kein zerrissener Frame  [22841 geholt, 0 zerrissen]
  ok      und keine ruecklaeufige oder doppelte Folgenummer — die Loecher der Kollision brechen die Reihenfolge nicht  [0 ruecklaeufig]
  ok      der Erzeuger traf den beanspruchten Slot WIRKLICH — und hat ihn nicht beschrieben  [10395 uebersprungene Positionen]
  ok      der NEUESTE Frame faellt dabei NIE — es weicht der aelteste wartende (replace-oldest, §53.9)  [0 neueste wegen fremden Anspruchs verworfen]
  ok      Testhaken: die zwei naechsten Plaetze sind beansprucht
  ok      erzwungene Slot-Kollision: der neueste Frame findet immer einen Platz  [0 verworfen, 2 Loecher]
  ok      und die beiden beanspruchten Positionen sind LOECHER, nicht Verluste  [2]
  ok      der Verbraucher ueberspringt die Loecher und bekommt genau diesen Frame  [64 Bytes]
  ok      Kollisionsloecher lassen den Verbraucher wirklich zurueckfallen (schreib - boden waechst)  [6 Positionen Rueckstand nach 5 Runden]
  ok      der Verbraucher ist zwischen JEDEM Versuch weitergerueckt — kein statisch freier dritter Platz  [6 Spruenge]
  ok      und der Erzeuger bekommt TROTZDEM einen Platz — garantiert, nicht begrenzt versucht  [0 verworfen, 6 Loecher in dieser Veroeffentlichung]
  ok      der neueste Frame liegt danach wirklich da, hinter allen Loechern  [64 Bytes]

== F · Ratengrenze ==
  ok      5000 Nachrichten in derselben Millisekunde: 1000 gehen durch  [1000]
  ok      ein volles Fenster spaeter wieder 1000
  ok      gleitend: 1 Nachricht je 10 ms laeuft unbegrenzt

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  ok      Control verbindet und bekommt ein v3-gerahmtes welcome
  ok      welcome liefert link_id und challenge
  ok      Telemetry koppelt mit link_id + challenge + derselben runtime_nonce
  ok      nak29_sender_project_samples_ohne_project_time_bit
  ok      nak29_sender_local_monotonic_mit_project_sample_start
  ok      band_stereo_sender_emittiert_erhoehten_schema_minor
  ok      P0 kommt beim Server als P0-Familie an
  ok      und die Antwort erreicht den Client
  ok      P1 kommt als P1-Familie an
  ok      P2 fliesst ueber die zweite Verbindung  [21 Frames]
  ok      beide Bootstrap-Hellos tragen dieselbe runtime_nonce
  ok      instance_address_alias_is_on_actual_wire
  ok      das Bootstrap-Hello ist v3 und nicht v2
  ok      stop() trennt beide Verbindungen und kehrt zurueck

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  ok      der Server weist die ungekoppelte Telemetrieverbindung ab
  ok      der Client gilt nicht als verbunden

== G3 · Reconnect nach Serverneustart ==
  ok      erste Verbindung steht
  ok      nach dem Serverende faellt der Client auf getrennt
  ok      der Client verbindet von selbst wieder (Backoff 500..8000 ms)
  ok      und zaehlt die Versuche ehrlich mit  [2]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  ok      der Client weist den Frame ab und verbindet nicht
  ok      Status bleibt ungleich 'verbunden'

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  ok      64 P0-Nachrichten passen
  ok      die 65. meldet Ueberlauf an den Aufrufer
  ok      der Ueberlauf ist gezaehlt
  ok      Verbindung steht (der Server liest nur nicht mehr)
  ok      ein 200-KiB-Befehl wird eingereiht
  ok      die Queue laeuft ueber, weil der Peer nicht liest
  ok      die Verbindung wird deswegen geschlossen, nicht stillschweigend gekuerzt  [P0-Ueberlauf: Verbindung wird geschlossen]

== G6 · ein P1-Ereignis ueberlebt einen gescheiterten Write ==
  ok      Verbindung steht (der Server liest nur nicht mehr)
  ok      der Write bricht, sobald der Server weg ist
  ok      einige Ereignisse gingen raus, eines blieb im Sender haengen  [p1Gesendet = 77]
  ok      das Ereignis, dessen Write scheiterte, kommt nach dem Reconnect an  [gesucht "id":77, · 1 P1 nach dem Reconnect empfangen]

== G7 · der TelemetryClient prueft sein welcome vollstaendig ==
  ok      ein welcome in der P2-Familie gilt NICHT als Kopplung  [welcome kam nicht als P0]
  ok      ein welcome mit fremder link_id gilt nicht als Kopplung  [unerwartete Antwort auf das Telemetry-Hello]
  ok      ein welcome ohne `protocol` gilt nicht als Kopplung  [unerwartete Antwort auf das Telemetry-Hello]
  ok      Gegenprobe: mit vollstaendigem welcome koppelt derselbe Client

== G8 · nicht endliche Audiofelder werden VOR der Wandlung verriegelt ==
  ok      48 kHz / 512 / 2 halten den Vertrag
  ok      NaN, ±Inf, 1e300 und Grenzverstoesse fallen
  ok      mit NaN/±Inf/1e300 verbindet der Client gar nicht erst  [Audiolage haelt den v3-Vertrag nicht (samplerate/block_size/channels)]
  ok      und nennt den Grund ehrlich beim Namen  [Audiolage haelt den v3-Vertrag nicht (samplerate/block_size/channels)]
  ok      kein Hello mit einer Nicht-Zahl hat den Server je erreicht

== G9 · ein P0-Befehl ueberlebt einen gescheiterten Write bei voller Queue ==
  ok      Verbindung steht (der Server liest nur nicht mehr)
  ok      der erste, grosse Befehl wird eingereiht
  ok      genau 64 passen — der unterwegs befindliche belegt seinen Platz weiter  [64 angenommen]
  ok      und der Ueberlauf ist oeffentlich gezaehlt  [1]
  ok      die Verbindung wird deswegen geschlossen  [P0-Ueberlauf: Verbindung wird geschlossen]
  ok      nach dem Reconnect kommt JEDER angenommene Befehl an — auch der, dessen Write scheiterte

== G10 · beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag ==
  ok      Telemetry: `broker_version` als `null` ist kein gueltiges welcome  [unerwartete Antwort auf das Telemetry-Hello]
  ok      Telemetry: ein Zusatzfeld verletzt additionalProperties:false  [unerwartete Antwort auf das Telemetry-Hello]
  ok      Telemetry: `broker_version` ueber 64 Zeichen faellt an der Laenge  [unerwartete Antwort auf das Telemetry-Hello]
  ok      Control: dieselbe Strenge — `null` statt String verbindet nicht  [unerwartete Antwort auf hello]
  ok      Control: ein Zusatzfeld verbindet nicht  [unerwartete Antwort auf hello]
  ok      Control: eine zu lange `broker_version` verbindet nicht  [unerwartete Antwort auf hello]

== G11 · ein P2-Frame auf der Control-Verbindung wird abgewiesen ==
  ok      der Client weist den P2-Frame ab und schliesst die Verbindung
  ok      und er hat die Binaerpayload NIE an den Aufrufer weitergereicht  [0 Rueckrufe]

== G12 · die Nachrichtenratengrenze gilt auch im Client ==
  ok      ein Peer, der schneller pipelined als die Rate erlaubt, wird getrennt
  ok      und hoechstens die erlaubte Zahl Frames hat den Aufrufer erreicht  [4000 von 8000]

== G13 · die Telemetrie merkt im Leerlauf, dass ihre Kopplung fort ist ==
  ok      Control steht
  ok      Telemetry koppelt
  ok      ohne eine einzige Veroeffentlichung bemerkt der Client den Pipe-Abschluss
  ok      und koppelt binnen Frist wieder
  ok      Control koppelt neu und traegt eine frische link_id
  ok      die Telemetrie bemerkt die neuen Kopplungswerte im Leerlauf  [Kopplung gewechselt — Telemetrie koppelt neu]
  ok      und koppelt sich mit ihnen neu — ohne dass je ein P2-Frame floss  [Kopplung gewechselt — Telemetrie koppelt neu]
  ok      Gegenprobe: es wurde in diesem Abschnitt wirklich nichts veroeffentlicht  [0 gesendet]

== G14 · was der TelemetryClient auf SEINER Verbindung annimmt ==
  ok      ein P0-Frame auf der Telemetrieverbindung schliesst sie
  ok      auch hier gilt die Nachrichtenratengrenze
  ok      die vertragsgemaessen P2-Frames sind gezaehlt, nicht still verworfen  [4000 von 8000 gezaehlt]

== G15 · zu grosse Nachrichten werden an der TUER abgewiesen ==
  ok      ein P0 ueber der Paketgrenze wird gar nicht erst eingereiht  [0]
  ok      und ein P1 ebenso — der Aufrufer erfaehrt es sofort
  ok      genau auf der Grenze wird eingereiht, nicht abgewiesen

== G16 · ein P0-ACK kommt an, waehrend P1 rueckstaut ==
  ok      Verbindung steht
  ok      der Heartbeat wird eingereiht
  ok      der ACK erreicht beiAntwort, obwohl P1 rueckstaut  [1 ACKs, 1 P1 gesendet, ]

== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  ok      Verbindung steht, und der Peer liest kein einziges Byte
  ok      ein vor dem Verbindungsende empfangener ACK geht nicht verloren — auch nach einem Write-Zeitlimit  [1 ACKs, empfangen=1, Pipe-Schreiben: Zeitlimit ueberschritten]

== G17 · stop() kehrt in JEDEM Zustand zurueck ==
  ok      Verbindung steht
  ok      stop() aus beiAntwort kehrt zurueck — kein Self-Join, kein terminate
  ok      und danach ist der Client getrennt
  ok      Verbindung steht
  ok      der Callback blockiert wirklich
  ok      stop() kehrt trotz blockierendem Callback binnen Frist zurueck  [2000 ms]
  ok      und die Fristueberschreitung ist sichtbar gezaehlt  [1]
  ok      Verbindung steht
  ok      der Callback blockiert wirklich
  ok      nach dem Abloesen verbindet ein neuer start() wieder  [2 Versuche]
  ok      und zwar GENAU EIN neuer Lauf — der abgeloeste Vorgaenger faehrt nicht daneben weiter  [2 Versuche, 1 Fristueberschreitungen]
  ok      Verbindung steht
  ok      der Callback blockiert wirklich
  ok      der neue Lauf steht NACHGEWIESEN — welcome geprueft, nicht nur ein Versuch gezaehlt
  ok      der zurueckkehrende alte Lauf trennt die neue Verbindung NICHT  [2 Versuche (vorher 2), 2 Serververbindungen (vorher 2), Status verbunden]
  ok      und der neue Lauf beantwortet danach weiter P0  [1 ACKs]
  ok      die Telemetrie steht
  ok      der helloProvider blockiert wirklich
  ok      der neue Telemetrielauf steht nachgewiesen
  ok      dieselbe Zusage gilt fuer den TelemetryClient  [2 Versuche (vorher 2), 2 Serververbindungen (vorher 2), Status verbunden]
  ok      die Telemetrie wartet auf ihre Kopplung
  ok      stop() im Zustand wartetAufKopplung kehrt zurueck  [1 ms]
  ok      die Telemetrie steht
  ok      stop() waehrend eines blockierenden P2-Writes kehrt binnen Frist zurueck, nicht erst nach kIoFristMs  [2 ms]

== G18 · die Telemetrie verliert die Verbindung mitten im Write ==
  ok      die Telemetrie steht
  ok      der Verbindungsverlust im Write wird bemerkt  [3 ms]
  ok      und der Client zaehlt einen neuen Versuch — er koppelt von selbst neu  [1 Versuche]
  ok      die Schleuse nimmt weiter an — der Erzeuger merkt vom Abbruch nichts

== H · Bootstrapgrenze und JSON-Riegel ==
  ok      16 KiB sind noch ein Bootstrap-Hello
  ok      ein Byte darueber nicht mehr
  ok      flaches Objekt wird gelesen
  ok      Zahlen kommen als Text zurueck
  ok      ein String kommt als String
  ok      eine Zahl ist KEIN String
  ok      und ein String ist kein Literal
  ok      `null` ist ein Literal und wird nie als Text `null` durchgereicht
  ok      der STRING "null" dagegen ist einer — beide sind unterscheidbar
  ok      die exakte Feldmenge wird erkannt
  ok      ein Zusatzfeld faellt auf (additionalProperties:false)
  ok      und ein fehlendes Pflichtfeld ebenso
  ok      Verschachtelung wird ABGELEHNT, nicht geraten
  ok      Arrays ebenso
  ok      ein Escape wird abgelehnt statt interpretiert
  ok      doppelter Schluessel ist eine Ablehnung, keine 'letzter gewinnt'-Regel
  ok      Text hinter dem Objekt wird abgelehnt
  ok      das leere Objekt ist gueltig

ALLE PRUEFUNGEN GRUEN — 211 Pruefungen, 0 Fehler
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,12 s

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

**build** | Exit 0 | 81,53 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
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
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIpcTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIpcTest_rc_lib.dir\Release\EqCopIpcTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIpcTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
     Bibliothek "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.lib" und Objekt "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.exp" werden erstellt.
  Code wird generiert.
  Codegenerierung ist abgeschlossen.
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIpcLast_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIpcLast_rc_lib.dir\Release\EqCopIpcLast_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIpcLast.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIpcLast_artefacts\Release\EqCopIpcLast.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
     Bibliothek "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.lib" und Objekt "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.exp" werden erstellt.
  Code wird generiert.
  Codegenerierung ist abgeschlossen.
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>

**cargo-release** | Exit 0 | 5,49 s

<details><summary>Rohe Ausgabe</summary>

```text

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `release` profile [optimized] target(s) in 5.43s
```

</details>

