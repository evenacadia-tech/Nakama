# Laufzeit-Arm NAK-312 - VERFEHLT

Zeit: 2026-09-21 17:27:24 | Basis: 12300f1e429c38354f9d216d7ade395b71f71b70 | HEAD: 5f1ca5a6 | Repo: C:\Users\phili\Projekte\Nakama | Projekt: Nakama-Diagnose.flp
Kopfzeile: LAUFZEIT NAK-312 5f1ca5a6 VERFEHLT installation=installiert; pruefen Exit 0, 3 Artefakt(e) aktuell szenarien=1 verfehlt=1 [schleife-dauerlauf.json=4] fl=Producer Edition v26.1.4 [build 5589]

## Protokoll

```text
[2026-09-21 17:21:35] Laufzeit-Arm NAK-312 auf 5f1ca5a6 (Basis 12300f1e429c38354f9d216d7ade395b71f71b70), Repo C:\Users\phili\Projekte\Nakama
[2026-09-21 17:21:35] Besitz: veralteter Eintrag PID 176680 (fl) entfernt - Prozess lebt nicht mehr
[2026-09-21 17:21:35] Lohnt es? True - Produktpfade im Diff: 7 Datei(en), z. B. eq-copilot/install/nakama-installer-v1.json
[2026-09-21 17:21:36] MCP-Stand: Revision 15b3b1a133ad37aa75e63e2d98a7aac0b74a62ae, Zweig evenacadia-local (Pin evenacadia-local), sauber, 10 gepinnte Datei(en) gleich, uv.lock SHA-256 356B1391E1F32DF927D0CF5B9D6F7FE19240241F1C768EC3E1241DED68EC0AD7
[2026-09-21 17:21:36] Manifest-Hashes nachziehen (--hashen)
[2026-09-21 17:21:36]   hashen: Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
[2026-09-21 17:21:36]   hashen:                  Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[2026-09-21 17:21:36]   hashen: [0] Struktur vor dem mutierenden Schritt --hashen
[2026-09-21 17:21:36]   hashen:   ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
[2026-09-21 17:21:36]   hashen:   ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
[2026-09-21 17:21:36]   hashen:   ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
[2026-09-21 17:21:36]   hashen:   ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
[2026-09-21 17:21:36]   hashen:   ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
[2026-09-21 17:21:36]   hashen:   ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
[2026-09-21 17:21:36]   hashen:   ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
[2026-09-21 17:21:36]   hashen:   ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
[2026-09-21 17:21:36]   hashen:   ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
[2026-09-21 17:21:36]   hashen:   ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
[2026-09-21 17:21:36]   hashen:   ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
[2026-09-21 17:21:36]   hashen:   ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
[2026-09-21 17:21:36]   hashen:   ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
[2026-09-21 17:21:36]   hashen:   ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
[2026-09-21 17:21:36]   hashen:   ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)
[2026-09-21 17:21:36]   hashen: [hashen] Artefakte gegen den gebauten Stand festschreiben
[2026-09-21 17:21:36]   hashen:   ok      main = 723AB44F5BB0B36E73151E7017C653BA23D19EAFCB09BA8D0A1322A36F3DB1D7
[2026-09-21 17:21:36]   hashen:   ok      active-probe = 9BE64F30518961AFBCC23918A23380BAD5C2C13D8DA91B2EACCAC95442F61AEF
[2026-09-21 17:21:36]   hashen:   ok      eqcop-broker.exe = FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D
[2026-09-21 17:21:36]   hashen: 
[2026-09-21 17:21:36]   hashen: geschrieben: eq-copilot/install/nakama-installer-v1.json
[2026-09-21 17:21:36]   hashen: 
[2026-09-21 17:21:36]   hashen: [hashen] Startbindung gegen das geschriebene Manifest ([4c], [4c+])
[2026-09-21 17:21:36]   hashen: 
[2026-09-21 17:21:36]   hashen: [4c] Manifestgebundene Broker-Startwerte
[2026-09-21 17:21:36]   hashen:   ok      generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests  [ist=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', 'FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D', '') soll=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', 'FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D', '')]
[2026-09-21 17:21:36]   hashen:   ok      Installer-Manifest ist CMake-Configure-Dependency
[2026-09-21 17:21:36]   hashen: 
[2026-09-21 17:21:36]   hashen: [4c+] Broker-Pin im Header gegen die gebaute Release-Broker-Datei (hart)
[2026-09-21 17:21:36]   hashen:   ok      Broker-Pin im erzeugten Header entspricht dem SHA-256 der gebauten Release-Broker-Datei  [Header FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D | Datei FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D]
[2026-09-21 17:21:36] Aufgabe \Nakama\installieren starten
[2026-09-21 17:21:40]   installieren: Exit 0, Zustand Ready, Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3] |   ok      Nakama Probeeq installiert  [C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3] |   ok      eqcop-broker.exe installiert  [C:\Program Files\evenacadia\Nakama\eqcop-broker.exe] | INSTALLATION OK
[2026-09-21 17:21:42]   pruefen: Exit 0, Zustand Ready, Log: Installierter Stand gegen das Manifest: |   aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3 |   aktuell          C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3 |   aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe
[2026-09-21 17:21:42] Controller-Skript: SHA-256 Repo 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB, installiert 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB (C:\Users\phili\OneDrive\Dokumente\Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\device_FLStudioMCP.py), script_version 2026-09-18
[2026-09-21 17:21:42] Diagnoseprojekt: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
[2026-09-21 17:21:42] Briefkasten: C:\Users\phili\AppData\Local\evenacadia\nakama\diagnose bereit, 48 Antwortdatei(en) frueherer Laeufe entfernt
[2026-09-21 17:21:42] loopMIDI laeuft
[2026-09-21 17:21:42] Besitz: PID 231732 (fl) eingetragen
[2026-09-21 17:21:42] FL gestartet: PID 231732 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-21 17:21:45] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-21T17:21:45", "program_title": "FL Studio 2026" }
[2026-09-21 17:21:54] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 231732, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-21 17:21:54] Szenario schleife-dauerlauf.json: frischer_start - Diagnose-FL neu starten
[2026-09-21 17:21:55] Diagnose-FL beenden: PID 231732 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-21 17:21:56] Besitz: PID 231732 ausgetragen (Ende bestaetigt)
[2026-09-21 17:21:56] Besitz: PID 284032 (fl) eingetragen
[2026-09-21 17:21:56] FL gestartet: PID 284032 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-21 17:21:59] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-21T17:21:59", "program_title": "FL Studio 2026" }
[2026-09-21 17:22:08] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 284032, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-21 17:27:24] Szenario schleife-dauerlauf.json: Exit 4
[2026-09-21 17:27:24] Diagnoseprojekt am Ende: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
```
## Szenarien


<!-- szenario.py 2026-09-21T15:22:08+00:00 -->
## Szenario `schleife-dauerlauf` — Der Host lebt und antwortet ueber mindestens fuenf Songumlaeufe Echtzeit-Wiedergabe des Diagnoseprojekts; nach jedem Umlauf antworten FL und beide Rollen im Briefkasten

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=da89854d64bc679a02312a36a22736a4 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `request_id=75367677c8be72ef434887717a8b22c0 {"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `request_id=2b2fd7d0bd8b9333556856f003f9b1ae {"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `transport.getLength` | `{}` | `request_id=bfb0d7cf75554c6139f6630e0a0cbf14 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 5 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=d5c4d8c9fa3cb6e8a2bf31b574cb7ae9 {"mode": "song", "success": true}` | ok |
| 6 | `transport.stop` | `{}` | `request_id=b63976fcdcd554c0aa6f0b943fc411c2 {"stopped": true, "success": true}` | ok |
| 7 | `transport.setPosition` | `{"mode": 0, "position": 0}` | `request_id=1478b4f263f47a5d6945392f1c60b99c {"mode": 0, "requested_position": 0, "success": true}` | ok |
| 8 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 9 | `transport.start` | `{}` | `request_id=fd2479ceb71a887499b65763174577f9 {"is_playing": true, "success": true}` | ok |
| 10 | warte | 46 s | — | — |
| 11 | `transport.getStatus` | `{}` | `request_id=0d886b03c2eaaed1025b7536a4843b6f {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "1:04:00", "success": true}` | ok · roh: position="1:04:00" |
| 12 | `transport.getPosition` | `{}` | `request_id=99006c206867e5995be6a69b7273720d {"hint": "1:04:10", "ms": 366, "seconds": 0, "success": true}` | ok · roh: ms=366 |
| 13 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 14 | warte | 46 s | — | — |
| 15 | `transport.getStatus` | `{}` | `request_id=ce9805007372500dd4c9db22e8f3c6a5 {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 16 | `transport.getPosition` | `{}` | `request_id=ce467c0f15e7d9cbac072a52aafdd4c6 {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 17 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | Rolle ohne Antwort nach 10 s: gen, probeeq | VERFEHLT (Details unten) |
| 18 | warte | 46 s | — | — |
| 19 | `transport.getStatus` | `{}` | `request_id=01bbd9140f9fe66e9b5e6a9608192f15 {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 20 | `transport.getPosition` | `{}` | `request_id=af1c29dd4cadc33c036ed1c49b2995e6 {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 21 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | Rolle ohne Antwort nach 10 s: gen, probeeq | VERFEHLT (Details unten) |
| 22 | warte | 46 s | — | — |
| 23 | `transport.getStatus` | `{}` | `request_id=7664acade23cb8d4379616d6c36d7cda {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 24 | `transport.getPosition` | `{}` | `request_id=2ade73c78fba61451a87e322740c505a {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 25 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | Rolle ohne Antwort nach 10 s: gen, probeeq | VERFEHLT (Details unten) |
| 26 | warte | 46 s | — | — |
| 27 | `transport.getStatus` | `{}` | `request_id=8fe9a04dde09cf4769cdd4fc41590acc {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 28 | `transport.getPosition` | `{}` | `request_id=f92cb1b8b12e6564333d053adb796610 {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |
| 29 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | Rolle ohne Antwort nach 10 s: gen, probeeq | VERFEHLT (Details unten) |
| 30 | `transport.stop` | `{}` | `request_id=5306c766e94196476887178b4c76fb1a {"_wiederholt": true, "error": "Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings.", "success": false}` | VERFEHLT: Antwort ohne Erfolg: Timeout waiting for FL Studio response after 2.0s. Make sure FL Studio is running and the MCP controller is enabled in MIDI Settings. |

**Ergebnis:** 17 von 30 Schritten bestanden.


### Schritt 8 `lokal.briefkasten`

- Anfrage `7aa4c612c1e339f8997dab5471fa9bd2`, erwartet {"gen": [284032], "probeeq": [284032]}, Positionsklammer None → None ms
  - gen: `7aa4c612c1e339f8997dab5471fa9bd2.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json` (24027 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.72 s nach dem Schreiben)
  - probeeq: `7aa4c612c1e339f8997dab5471fa9bd2.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json` (15917 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.77 s nach dem Schreiben)
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 102
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 112

### Schritt 13 `lokal.briefkasten`

- Anfrage `a882f170517abdb8a6eccfe94bbabdff`, erwartet {"gen": [284032], "probeeq": [284032]}, Positionsklammer None → None ms
  - gen: `a882f170517abdb8a6eccfe94bbabdff.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json` (23687 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.48 s nach dem Schreiben)
  - probeeq: `a882f170517abdb8a6eccfe94bbabdff.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json` (23676 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.52 s nach dem Schreiben)
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 570
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 581

### Schritt 17 `lokal.briefkasten`

- Anfrage `a5f8c35c84e8fd40abcbbbf4619d7a24`, erwartet {"gen": [284032], "probeeq": [284032]}, Positionsklammer None → None ms
  - gen: fehlt
  - probeeq: fehlt
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`

### Schritt 21 `lokal.briefkasten`

- Anfrage `28ca4eb4a902eb1054e028263be5ded2`, erwartet {"gen": [284032], "probeeq": [284032]}, Positionsklammer None → None ms
  - gen: fehlt
  - probeeq: fehlt
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`

### Schritt 25 `lokal.briefkasten`

- Anfrage `96b3a2da862bb481ce2563a61bcffc1c`, erwartet {"gen": [284032], "probeeq": [284032]}, Positionsklammer None → None ms
  - gen: fehlt
  - probeeq: fehlt
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`

### Schritt 29 `lokal.briefkasten`

- Anfrage `2dec2f340dfe64b70487c47db92c48ff`, erwartet {"gen": [284032], "probeeq": [284032]}, Positionsklammer None → None ms
  - gen: fehlt
  - probeeq: fehlt
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `7aa4c612c1e339f8997dab5471fa9bd2.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.gen.284032.2f869d2ee2d9099f.f3ae25495f4b4b1291c09c7aa4125deb.json`
  - roh fremd: andere Kennung: `a882f170517abdb8a6eccfe94bbabdff.probeeq.284032.d387a78151edaa16.ccb826698a1b41a3b4f77322794409d8.json`

