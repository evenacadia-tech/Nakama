# Laufzeit-Arm NAK-312 - GEMESSEN

Zeit: 2026-09-21 16:31:15 | Basis: 12300f1e429c38354f9d216d7ade395b71f71b70 | HEAD: 5f1ca5a6 | Repo: C:\Users\phili\Projekte\Nakama | Projekt: Nakama-Diagnose.flp
Kopfzeile: LAUFZEIT NAK-312 5f1ca5a6 GEMESSEN installation=installiert; pruefen Exit 0, 3 Artefakt(e) aktuell szenarien=1 verfehlt=0 [schleife-dauerlauf.json=0] fl=Producer Edition v26.1.4 [build 5589]

## Protokoll

```text
[2026-09-21 16:26:39] Laufzeit-Arm NAK-312 auf 5f1ca5a6 (Basis 12300f1e429c38354f9d216d7ade395b71f71b70), Repo C:\Users\phili\Projekte\Nakama
[2026-09-21 16:26:39] Besitz: veralteter Eintrag PID 316548 (fl) entfernt - Prozess lebt nicht mehr
[2026-09-21 16:26:39] Lohnt es? True - Produktpfade im Diff: 7 Datei(en), z. B. eq-copilot/install/nakama-installer-v1.json
[2026-09-21 16:26:40] MCP-Stand: Revision 15b3b1a133ad37aa75e63e2d98a7aac0b74a62ae, Zweig evenacadia-local (Pin evenacadia-local), sauber, 10 gepinnte Datei(en) gleich, uv.lock SHA-256 356B1391E1F32DF927D0CF5B9D6F7FE19240241F1C768EC3E1241DED68EC0AD7
[2026-09-21 16:26:40] Manifest-Hashes nachziehen (--hashen)
[2026-09-21 16:26:40]   hashen: Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
[2026-09-21 16:26:40]   hashen:                  Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[2026-09-21 16:26:40]   hashen: [0] Struktur vor dem mutierenden Schritt --hashen
[2026-09-21 16:26:40]   hashen:   ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
[2026-09-21 16:26:40]   hashen:   ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
[2026-09-21 16:26:40]   hashen:   ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
[2026-09-21 16:26:40]   hashen:   ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
[2026-09-21 16:26:40]   hashen:   ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
[2026-09-21 16:26:40]   hashen:   ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
[2026-09-21 16:26:40]   hashen:   ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
[2026-09-21 16:26:40]   hashen:   ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
[2026-09-21 16:26:40]   hashen:   ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
[2026-09-21 16:26:40]   hashen:   ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
[2026-09-21 16:26:40]   hashen:   ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
[2026-09-21 16:26:40]   hashen:   ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
[2026-09-21 16:26:40]   hashen:   ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
[2026-09-21 16:26:40]   hashen:   ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
[2026-09-21 16:26:40]   hashen:   ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)
[2026-09-21 16:26:40]   hashen: [hashen] Artefakte gegen den gebauten Stand festschreiben
[2026-09-21 16:26:40]   hashen:   ok      main = 194C2BBA4892C82652AC6634CD3DAE522D49C0B3F65DC571ED7E9CBE78BC25E6
[2026-09-21 16:26:40]   hashen:   ok      active-probe = E5881DAF42BBA3384EF61C9A4274896DA241E8A83AEDB53E4E01FD3F1115BAEE
[2026-09-21 16:26:40]   hashen:   ok      eqcop-broker.exe = FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D
[2026-09-21 16:26:40]   hashen: 
[2026-09-21 16:26:40]   hashen: geschrieben: eq-copilot/install/nakama-installer-v1.json
[2026-09-21 16:26:40]   hashen: 
[2026-09-21 16:26:40]   hashen: [hashen] Startbindung gegen das geschriebene Manifest ([4c], [4c+])
[2026-09-21 16:26:40]   hashen: 
[2026-09-21 16:26:41]   hashen: [4c] Manifestgebundene Broker-Startwerte
[2026-09-21 16:26:41]   hashen:   ok      generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests  [ist=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', 'FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D', '') soll=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', 'FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D', '')]
[2026-09-21 16:26:41]   hashen:   ok      Installer-Manifest ist CMake-Configure-Dependency
[2026-09-21 16:26:41]   hashen: 
[2026-09-21 16:26:41]   hashen: [4c+] Broker-Pin im Header gegen die gebaute Release-Broker-Datei (hart)
[2026-09-21 16:26:41]   hashen:   ok      Broker-Pin im erzeugten Header entspricht dem SHA-256 der gebauten Release-Broker-Datei  [Header FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D | Datei FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D]
[2026-09-21 16:26:41] Aufgabe \Nakama\installieren starten
[2026-09-21 16:26:44]   installieren: Exit 0, Zustand Ready, Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3] |   ok      Nakama Probeeq installiert  [C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3] |   ok      eqcop-broker.exe installiert  [C:\Program Files\evenacadia\Nakama\eqcop-broker.exe] | INSTALLATION OK
[2026-09-21 16:26:46]   pruefen: Exit 0, Zustand Ready, Log: Installierter Stand gegen das Manifest: |   aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3 |   aktuell          C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3 |   aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe
[2026-09-21 16:26:46] Controller-Skript: SHA-256 Repo 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB, installiert 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB (C:\Users\phili\OneDrive\Dokumente\Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\device_FLStudioMCP.py), script_version 2026-09-18
[2026-09-21 16:26:46] Diagnoseprojekt: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
[2026-09-21 16:26:46] Briefkasten: C:\Users\phili\AppData\Local\evenacadia\nakama\diagnose bereit, 48 Antwortdatei(en) frueherer Laeufe entfernt
[2026-09-21 16:26:46] loopMIDI laeuft
[2026-09-21 16:26:46] Besitz: PID 324712 (fl) eingetragen
[2026-09-21 16:26:46] FL gestartet: PID 324712 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-21 16:26:49] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-21T16:26:49", "program_title": "FL Studio 2026" }
[2026-09-21 16:26:58] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 324712, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-21 16:26:58] Szenario schleife-dauerlauf.json: frischer_start - Diagnose-FL neu starten
[2026-09-21 16:26:58] Diagnose-FL beenden: PID 324712 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-21 16:26:59] Besitz: PID 324712 ausgetragen (Ende bestaetigt)
[2026-09-21 16:26:59] Besitz: PID 231228 (fl) eingetragen
[2026-09-21 16:26:59] FL gestartet: PID 231228 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-21 16:27:03] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-21T16:27:02", "program_title": "FL Studio 2026" }
[2026-09-21 16:27:11] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 231228, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-21 16:31:15] Szenario schleife-dauerlauf.json: Exit 0
[2026-09-21 16:31:15] Diagnoseprojekt am Ende: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
```
## Szenarien



<!-- szenario.py 2026-09-21T14:27:11+00:00 -->
## Szenario `schleife-dauerlauf` — Der Host lebt und antwortet ueber mindestens fuenf Songumlaeufe Echtzeit-Wiedergabe des Diagnoseprojekts; nach jedem Umlauf antworten FL und beide Rollen im Briefkasten

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=c4d4994010cc3e2d6ac27b4f97e97da0 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `request_id=d6e66c8fbebaececd956cd774d1f02a7 {"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `request_id=286af1f815d4e9a7d55da0f4da228b34 {"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `transport.getLength` | `{}` | `request_id=3d1d6e8adeb9b165adf08ca9ad11422d {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 5 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=0028bdc3cee8110b2b895c68f7f08ebd {"mode": "song", "success": true}` | ok |
| 6 | `transport.stop` | `{}` | `request_id=fb2ba0d8489b8acb13ae1662241b127f {"stopped": true, "success": true}` | ok |
| 7 | `transport.setPosition` | `{"mode": 0, "position": 0}` | `request_id=89cfcfafb2f8c757d8bd05f9ebdda008 {"mode": 0, "requested_position": 0, "success": true}` | ok |
| 8 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 9 | `transport.start` | `{}` | `request_id=0e4e3548c15c491a6b7da51831d08506 {"is_playing": true, "success": true}` | ok |
| 10 | warte | 46 s | — | — |
| 11 | `transport.getStatus` | `{}` | `request_id=ec863373971646721b6eb03c4cf423d5 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "1:03:14", "success": true}` | ok · roh: position="1:03:14" |
| 12 | `transport.getPosition` | `{}` | `request_id=49bc2186cb08abb455f51dadbe879a9a {"hint": "1:04:00", "ms": 321, "seconds": 0, "success": true}` | ok · roh: ms=321 |
| 13 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 14 | warte | 46 s | — | — |
| 15 | `transport.getStatus` | `{}` | `request_id=f15f41c9e9af117321fef919ac1ec6ac {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "2:11:09", "success": true}` | ok · roh: position="2:11:09" |
| 16 | `transport.getPosition` | `{}` | `request_id=3532758b6579d9b367f70057edbb7da5 {"hint": "2:11:09", "ms": 2826, "seconds": 3, "success": true}` | ok · roh: ms=2826 |
| 17 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 18 | warte | 46 s | — | — |
| 19 | `transport.getStatus` | `{}` | `request_id=111d07ef6ea0b3e2d16bd940fbcd5dc3 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "4:03:04", "success": true}` | ok · roh: position="4:03:04" |
| 20 | `transport.getPosition` | `{}` | `request_id=26e042286a2abd45c2536f8adcf04782 {"hint": "4:03:14", "ms": 5420, "seconds": 5, "success": true}` | ok · roh: ms=5420 |
| 21 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 22 | warte | 46 s | — | — |
| 23 | `transport.getStatus` | `{}` | `request_id=12999e2de6b29e475c8e187620029717 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "5:10:23", "success": true}` | ok · roh: position="5:10:23" |
| 24 | `transport.getPosition` | `{}` | `request_id=b28d19db461dcaf8bec7e8028b18e06c {"hint": "5:10:23", "ms": 7924, "seconds": 8, "success": true}` | ok · roh: ms=7924 |
| 25 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 26 | warte | 46 s | — | — |
| 27 | `transport.getStatus` | `{}` | `request_id=04c8151d569294a81a3669faedcbefb5 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "7:02:18", "success": true}` | ok · roh: position="7:02:18" |
| 28 | `transport.getPosition` | `{}` | `request_id=04490626b6501d8c67a6065e6e338ca3 {"hint": "7:02:18", "ms": 10473, "seconds": 10, "success": true}` | ok · roh: ms=10473 |
| 29 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 30 | `transport.stop` | `{}` | `request_id=7df5b64d488d7c2b1c8d7af1c1a9c60a {"stopped": true, "success": true}` | ok |

**Ergebnis:** 30 von 30 Schritten bestanden.


### Schritt 8 `lokal.briefkasten`

- Anfrage `6aeb9157b5a85fbfa92827f364f7ee51`, erwartet {"gen": [231228], "probeeq": [231228]}, Positionsklammer None → None ms
  - gen: `6aeb9157b5a85fbfa92827f364f7ee51.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json` (24045 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.05 s nach dem Schreiben)
  - probeeq: `6aeb9157b5a85fbfa92827f364f7ee51.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json` (15910 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.09 s nach dem Schreiben)
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 92
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 102

### Schritt 13 `lokal.briefkasten`

- Anfrage `8171f2abd1bd222d64d7fc83adc0ba74`, erwartet {"gen": [231228], "probeeq": [231228]}, Positionsklammer None → None ms
  - gen: `8171f2abd1bd222d64d7fc83adc0ba74.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json` (23720 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.81 s nach dem Schreiben)
  - probeeq: `8171f2abd1bd222d64d7fc83adc0ba74.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json` (23558 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.86 s nach dem Schreiben)
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 570
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 580

### Schritt 17 `lokal.briefkasten`

- Anfrage `83c566822b16b0ab63ed37277817a63f`, erwartet {"gen": [231228], "probeeq": [231228]}, Positionsklammer None → None ms
  - gen: `83c566822b16b0ab63ed37277817a63f.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json` (23709 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.59 s nach dem Schreiben)
  - probeeq: `83c566822b16b0ab63ed37277817a63f.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json` (23635 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.64 s nach dem Schreiben)
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1039
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1049

### Schritt 21 `lokal.briefkasten`

- Anfrage `c4423436860ad776235e8d0aed49c3a7`, erwartet {"gen": [231228], "probeeq": [231228]}, Positionsklammer None → None ms
  - gen: `c4423436860ad776235e8d0aed49c3a7.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json` (23708 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.39 s nach dem Schreiben)
  - probeeq: `c4423436860ad776235e8d0aed49c3a7.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json` (23632 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.42 s nach dem Schreiben)
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `83c566822b16b0ab63ed37277817a63f.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `83c566822b16b0ab63ed37277817a63f.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1507
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1517

### Schritt 25 `lokal.briefkasten`

- Anfrage `ea88025e5044c0c69e37f0ef09916771`, erwartet {"gen": [231228], "probeeq": [231228]}, Positionsklammer None → None ms
  - gen: `ea88025e5044c0c69e37f0ef09916771.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json` (23696 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.17 s nach dem Schreiben)
  - probeeq: `ea88025e5044c0c69e37f0ef09916771.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json` (23675 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.20 s nach dem Schreiben)
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `83c566822b16b0ab63ed37277817a63f.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `83c566822b16b0ab63ed37277817a63f.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `c4423436860ad776235e8d0aed49c3a7.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `c4423436860ad776235e8d0aed49c3a7.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1975
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1985

### Schritt 29 `lokal.briefkasten`

- Anfrage `c792225ba3f1509683e7ad8d954303ac`, erwartet {"gen": [231228], "probeeq": [231228]}, Positionsklammer None → None ms
  - gen: `c792225ba3f1509683e7ad8d954303ac.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json` (23735 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.97 s nach dem Schreiben)
  - probeeq: `c792225ba3f1509683e7ad8d954303ac.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json` (23672 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 2.00 s nach dem Schreiben)
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `6aeb9157b5a85fbfa92827f364f7ee51.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `8171f2abd1bd222d64d7fc83adc0ba74.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `83c566822b16b0ab63ed37277817a63f.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `83c566822b16b0ab63ed37277817a63f.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `c4423436860ad776235e8d0aed49c3a7.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `c4423436860ad776235e8d0aed49c3a7.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - roh fremd: andere Kennung: `ea88025e5044c0c69e37f0ef09916771.gen.231228.2f869d2ee2d9099f.5149794162be4e00ae0eacf9b313d9fd.json`
  - roh fremd: andere Kennung: `ea88025e5044c0c69e37f0ef09916771.probeeq.231228.d387a78151edaa16.e8b3b256a08f4a5da9ae298492c63791.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 2453
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 2463
