# Laufzeit-Arm NAK-283 - GEMESSEN

Zeit: 2026-09-13 21:14:12 | Basis: 39e18d3f | HEAD: 7ad6b6f2 | Repo: C:\Users\phili\Projekte\Nakama | Projekt: Nakama-Diagnose.flp
Kopfzeile: LAUFZEIT NAK-283 7ad6b6f2 GEMESSEN installation=installiert; pruefen Exit 0 szenarien=1 verfehlt=0 [bereitschaft.json=0] fl=Producer Edition v26.1.4 [build 5589]

## Protokoll

```text
[2026-09-13 21:13:48] Laufzeit-Arm NAK-283 auf 7ad6b6f2 (Basis 39e18d3f), Repo C:\Users\phili\Projekte\Nakama
[2026-09-13 21:13:49] Lohnt es? True - Produktpfade im Diff: 34 Datei(en), z. B. broker/src/coordinator/hypothese_verdrahtung.rs
[2026-09-13 21:13:49] Manifest-Hashes nachziehen (--hashen)
[2026-09-13 21:13:49]   hashen: Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
[2026-09-13 21:13:49]   hashen:                  Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[2026-09-13 21:13:49]   hashen: [0] Struktur vor dem mutierenden Release-Schritt
[2026-09-13 21:13:49]   hashen:   ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
[2026-09-13 21:13:49]   hashen:   ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
[2026-09-13 21:13:49]   hashen:   ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
[2026-09-13 21:13:49]   hashen:   ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
[2026-09-13 21:13:49]   hashen:   ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
[2026-09-13 21:13:49]   hashen:   ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
[2026-09-13 21:13:49]   hashen:   ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
[2026-09-13 21:13:49]   hashen:   ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
[2026-09-13 21:13:49]   hashen:   ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
[2026-09-13 21:13:49]   hashen:   ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
[2026-09-13 21:13:49]   hashen:   ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
[2026-09-13 21:13:49]   hashen:   ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
[2026-09-13 21:13:49]   hashen:   ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
[2026-09-13 21:13:49]   hashen:   ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
[2026-09-13 21:13:49]   hashen:   ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)
[2026-09-13 21:13:49]   hashen: [hashen] Artefakte gegen den gebauten Stand festschreiben
[2026-09-13 21:13:49]   hashen:   ok      main = AFCB797283BC7B28BF406C0699B44413275E16F938908EDBEDB9D779216C1FC4
[2026-09-13 21:13:49]   hashen:   ok      active-probe = 31CC951BDDCEFED297E0B8D1D0E1DFFF0CCB8D87DDEEA3AC5BFB1ECCD4BC4FE7
[2026-09-13 21:13:49]   hashen:   ok      eqcop-broker.exe = F8F67718BF5A797D7DE7F13820D162C7150F57A0A99C975CF873B5510B047BE3
[2026-09-13 21:13:49]   hashen: 
[2026-09-13 21:13:49]   hashen: geschrieben: eq-copilot/install/nakama-installer-v1.json
[2026-09-13 21:13:49] Aufgabe \Nakama\installieren starten
[2026-09-13 21:13:53]   installieren: Exit 0, Zustand Ready, Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3] |   ok      Nakama Probeeq installiert  [C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3] |   ok      eqcop-broker.exe installiert  [C:\Program Files\evenacadia\Nakama\eqcop-broker.exe] | INSTALLATION OK
[2026-09-13 21:13:54]   pruefen: Exit 0, Log: Installierter Stand gegen das Manifest: |   aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3 |   aktuell          C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3 |   aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe
[2026-09-13 21:13:54] loopMIDI laeuft
[2026-09-13 21:13:54] FL gestartet: PID 4028 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-13 21:13:59] Boot-Marke nach 5 s: { "script_version": "2026-09-12", "loaded_at": "2026-09-13T21:13:59", "program_title": "FL Studio 2026" }
[2026-09-13 21:14:08] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1
[2026-09-13 21:14:12] Szenario bereitschaft.json: Exit 0
```
## Szenarien


<!-- szenario.py 2026-09-13T19:14:08+00:00 -->
## Szenario `bereitschaft` — FL antwortet, das Diagnoseprojekt mit beiden Plugins ist offen, Ton fließt durch Sondenspur und Master

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `{"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `{"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `{"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `{"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125}` | ok |
| 5 | `transport.getLength` | `{}` | `{"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok |
| 6 | `transport.setLoopMode` | `{"mode": "song"}` | `{"mode": "song", "success": true}` | ok |
| 7 | `transport.stop` | `{}` | `{"stopped": true, "success": true}` | ok |
| 8 | `transport.setPosition` | `{"mode": 1, "position": 2}` | `{"position": "1:01:00", "success": true}` | ok |
| 9 | `transport.start` | `{}` | `{"is_playing": true, "success": true}` | ok |
| 10 | warte | 3 s | — | — |
| 11 | `mixer.getPeaks` | `{"track": 1}` | `{"name": "Insert 1", "peak_left": 0.4949530065059662, "peak_max": 0.4949530065059662, "peak_right": 0.4754391014575958, "success": true, "track": 1}` | ok |
| 12 | `mixer.getPeaks` | `{"track": 0}` | `{"name": "Master", "peak_left": 0.4949530065059662, "peak_max": 0.4949530065059662, "peak_right": 0.4754391014575958, "success": true, "track": 0}` | ok |
| 13 | `transport.getStatus` | `{}` | `{"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "3:15:20", "success": true}` | ok |
| 14 | `transport.stop` | `{}` | `{"stopped": true, "success": true}` | ok |

**Ergebnis:** 14 von 14 Schritten bestanden.


