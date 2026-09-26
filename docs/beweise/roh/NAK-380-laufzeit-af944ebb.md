# Laufzeit-Arm NAK-380 - VORAUSSETZUNG

Zeit: 2026-09-26 15:05:11 | Basis: c58a2714 | HEAD: af944ebb | Repo: C:\Users\phili\Projekte\Nakama | Projekt: Nakama-Diagnose.flp
Kopfzeile: LAUFZEIT NAK-380 af944ebb VORAUSSETZUNG installation=installiert; pruefen Exit 0, 3 Artefakt(e) aktuell szenarien=6 verfehlt=1 [bereitschaft.json=0,fenster.json=0,nulltest-host.json=5,schleife-dauerlauf.json=0,snapshot-runde01.json=0,u40-aktivitaetsgate.json=0] fl=Producer Edition v26.1.4 [build 5589]

## Protokoll

```text
[2026-09-26 14:53:47] Laufzeit-Arm NAK-380 auf af944ebb (Basis c58a2714), Repo C:\Users\phili\Projekte\Nakama
[2026-09-26 14:53:48] Lohnt es? True - Produktpfade im Diff: 47 Datei(en), z. B. broker/src/coordinator/experiment.rs
[2026-09-26 14:53:48] MCP-Stand: Revision 15b3b1a133ad37aa75e63e2d98a7aac0b74a62ae, Zweig evenacadia-local (Pin evenacadia-local), sauber, 10 gepinnte Datei(en) gleich, uv.lock SHA-256 356B1391E1F32DF927D0CF5B9D6F7FE19240241F1C768EC3E1241DED68EC0AD7
[2026-09-26 14:53:48] Manifest-Hashes nachziehen (--hashen)
[2026-09-26 14:53:49]   hashen: Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
[2026-09-26 14:53:49]   hashen:                  Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[2026-09-26 14:53:49]   hashen: [0] Struktur vor dem mutierenden Schritt --hashen
[2026-09-26 14:53:49]   hashen:   ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
[2026-09-26 14:53:49]   hashen:   ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
[2026-09-26 14:53:49]   hashen:   ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
[2026-09-26 14:53:49]   hashen:   ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
[2026-09-26 14:53:49]   hashen:   ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
[2026-09-26 14:53:49]   hashen:   ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
[2026-09-26 14:53:49]   hashen:   ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
[2026-09-26 14:53:49]   hashen:   ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
[2026-09-26 14:53:49]   hashen:   ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
[2026-09-26 14:53:49]   hashen:   ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
[2026-09-26 14:53:49]   hashen:   ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
[2026-09-26 14:53:49]   hashen:   ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
[2026-09-26 14:53:49]   hashen:   ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
[2026-09-26 14:53:49]   hashen:   ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
[2026-09-26 14:53:49]   hashen:   ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)
[2026-09-26 14:53:49]   hashen: [hashen] Artefakte gegen den gebauten Stand festschreiben
[2026-09-26 14:53:49]   hashen:   ok      main = 256F936781D26E1FFECB46B85AB815E4946B05467134EC65DCB11E3675B8C449
[2026-09-26 14:53:49]   hashen:   ok      active-probe = 317CBF13490EF11FECCA931E1ECB2E5FDFA15CA7DAE20C8F405E8178534C7C9F
[2026-09-26 14:53:49]   hashen:   ok      eqcop-broker.exe = 93310E116FF2EAC6CC938FF79708504949A92BA6CC57A7A58AD7A644ADACC6A2
[2026-09-26 14:53:49]   hashen: 
[2026-09-26 14:53:49]   hashen: geschrieben: eq-copilot/install/nakama-installer-v1.json
[2026-09-26 14:53:49]   hashen: 
[2026-09-26 14:53:49]   hashen: [hashen] Startbindung gegen das geschriebene Manifest ([4c], [4c+])
[2026-09-26 14:53:49]   hashen: 
[2026-09-26 14:53:49]   hashen: [4c] Manifestgebundene Broker-Startwerte
[2026-09-26 14:53:49]   hashen:   ok      generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests  [ist=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', '93310E116FF2EAC6CC938FF79708504949A92BA6CC57A7A58AD7A644ADACC6A2', '') soll=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', '93310E116FF2EAC6CC938FF79708504949A92BA6CC57A7A58AD7A644ADACC6A2', '')]
[2026-09-26 14:53:49]   hashen:   ok      Installer-Manifest ist CMake-Configure-Dependency
[2026-09-26 14:53:49]   hashen: 
[2026-09-26 14:53:49]   hashen: [4c+] Broker-Pin im Header gegen die gebaute Release-Broker-Datei (hart)
[2026-09-26 14:53:49]   hashen:   ok      Broker-Pin im erzeugten Header entspricht dem SHA-256 der gebauten Release-Broker-Datei  [Header 93310E116FF2EAC6CC938FF79708504949A92BA6CC57A7A58AD7A644ADACC6A2 | Datei 93310E116FF2EAC6CC938FF79708504949A92BA6CC57A7A58AD7A644ADACC6A2]
[2026-09-26 14:53:49] Aufgabe \Nakama\installieren starten
[2026-09-26 14:53:52]   installieren: Exit 0, Zustand Ready, Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3] |   ok      Nakama Probeeq installiert  [C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3] |   ok      eqcop-broker.exe installiert  [C:\Program Files\evenacadia\Nakama\eqcop-broker.exe] | INSTALLATION OK
[2026-09-26 14:53:54]   pruefen: Exit 0, Zustand Ready, Log: Installierter Stand gegen das Manifest: |   aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3 |   aktuell          C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3 |   aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe
[2026-09-26 14:53:54] Controller-Skript: SHA-256 Repo 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB, installiert 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB (C:\Users\phili\OneDrive\Dokumente\Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\device_FLStudioMCP.py), script_version 2026-09-18
[2026-09-26 14:53:54] Diagnoseprojekt: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
[2026-09-26 14:53:54] Referenzprojekt Nakama-Diagnose-Verarbeitung.flp: fehlt neben dem Diagnoseprojekt (C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\fl\Nakama-Diagnose-Verarbeitung.flp; Karte U43), nicht kopiert
[2026-09-26 14:53:54] Referenzprojekt Nakama-Diagnose-Referenz.flp: SHA-256 Repo 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Arbeitskopie 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D
[2026-09-26 14:53:54] Briefkasten: C:\Users\phili\AppData\Local\evenacadia\nakama\diagnose bereit, 41 Antwortdatei(en) frueherer Laeufe entfernt
[2026-09-26 14:53:54] Renderfolge vor dem FL-Start: 1 Nakama-Diagnose.flp (Auslieferungszustand), 2 Nakama-Diagnose-Verarbeitung.flp (Referenzprojekt fehlt, kein Render), 3 Nakama-Diagnose-Referenz.flp (Referenzprojekt)
[2026-09-26 14:53:55] Besitz: PID 21920 (render) eingetragen
[2026-09-26 14:53:55] Render gestartet: PID 21920 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe /R /Ewav /O"C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render" "C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp")
[2026-09-26 14:54:00] Besitz: PID 21920 ausgetragen (Render beendet)
[2026-09-26 14:54:00] Render: Exit 0, Dauer 5,2 s, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\Nakama-Diagnose.wav, SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A, Grund 
[2026-09-26 14:54:00] Referenzrender Nakama-Diagnose-Verarbeitung.flp: kein Render - Referenzprojekt fehlt (Karte U43, K-286-1); Renderstatus C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Verarbeitung.flp\render.json
[2026-09-26 14:54:00] Besitz: PID 191520 (render) eingetragen
[2026-09-26 14:54:00] Referenzrender Nakama-Diagnose-Referenz.flp gestartet: PID 191520 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe /R /Ewav /O"C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp" "C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose-Referenz.flp")
[2026-09-26 14:54:03] Besitz: PID 191520 ausgetragen (Render beendet)
[2026-09-26 14:54:03] Referenzrender Nakama-Diagnose-Referenz.flp: Exit 0, Dauer 3,1 s, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp\Nakama-Diagnose-Referenz.wav, SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A, SHA-256 Projekt 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Grund 
[2026-09-26 14:54:03] loopMIDI laeuft
[2026-09-26 14:54:03] Besitz: PID 12520 (fl) eingetragen
[2026-09-26 14:54:03] FL gestartet: PID 12520 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-26 14:54:06] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-26T14:54:05", "program_title": "FL Studio 2026" }
[2026-09-26 14:54:16] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 12520, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-26 14:54:19] Szenario bereitschaft.json: Exit 0
[2026-09-26 14:54:21] Szenario fenster.json: Exit 0
[2026-09-26 14:54:25] Szenario nulltest-host.json: Exit 5
[2026-09-26 14:54:25] Szenario schleife-dauerlauf.json: frischer_start - Diagnose-FL neu starten
[2026-09-26 14:54:25] Diagnose-FL beenden: PID 12520 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-26 14:54:26] Besitz: PID 12520 ausgetragen (Ende bestaetigt)
[2026-09-26 14:54:26] Besitz: PID 275924 (fl) eingetragen
[2026-09-26 14:54:26] FL gestartet: PID 275924 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-26 14:54:29] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-26T14:54:28", "program_title": "FL Studio 2026" }
[2026-09-26 14:54:38] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 275924, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-26 14:58:42] Szenario schleife-dauerlauf.json: Exit 0
[2026-09-26 14:58:42] Szenario snapshot-runde01.json: frischer_start - Diagnose-FL neu starten
[2026-09-26 14:58:42] Diagnose-FL beenden: PID 275924 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-26 14:58:43] Besitz: PID 275924 ausgetragen (Ende bestaetigt)
[2026-09-26 14:58:43] Besitz: PID 315912 (fl) eingetragen
[2026-09-26 14:58:43] FL gestartet: PID 315912 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-26 14:58:46] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-26T14:58:45", "program_title": "FL Studio 2026" }
[2026-09-26 14:58:55] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 315912, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-26 15:04:28] Szenario snapshot-runde01.json: Exit 0
[2026-09-26 15:05:09] Szenario u40-aktivitaetsgate.json: Exit 0
[2026-09-26 15:05:10] Diagnose-FL beenden: PID 315912 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-26 15:05:11] Besitz: PID 315912 ausgetragen (Ende bestaetigt)
[2026-09-26 15:05:11] Diagnoseprojekt am Ende: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
[2026-09-26 15:05:11] Referenzprojekt Nakama-Diagnose-Referenz.flp am Ende: SHA-256 Repo 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Arbeitskopie 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D
```
## Szenarien


<!-- szenario.py 2026-09-26T12:54:16+00:00 -->
## Szenario `bereitschaft` — FL antwortet, das Diagnoseprojekt mit beiden Plugins ist offen, Ton fließt durch Sondenspur und Master

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=a327dc1c570cf6179609869f29efe9dc {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `request_id=4e70d151f24bed6e280fbbe84bbd6d0a {"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `request_id=33de5fbec5e63cb5675da76b770d4cce {"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `request_id=e014472ed5ec947f322a2b52a2509a61 {"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok |
| 5 | `transport.getLength` | `{}` | `request_id=010dd2862e6b69075e18c052211e42d1 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok |
| 6 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=dcc7b9a9242b5fef39a6668d27abd960 {"mode": "song", "success": true}` | ok |
| 7 | `transport.stop` | `{}` | `request_id=66294ebbfe87d4b6712cd831c328aa4a {"stopped": true, "success": true}` | ok |
| 8 | `transport.setPosition` | `{"mode": 1, "position": 2}` | `request_id=413380660bab84255b64d29a59cb88df {"mode": 1, "requested_position": 2, "success": true}` | ok |
| 9 | `transport.start` | `{}` | `request_id=dfc2d1e4c0d5a211ea7609b3298af4af {"is_playing": true, "success": true}` | ok |
| 10 | warte | 3 s | — | — |
| 11 | `mixer.getPeaks` | `{"track": 1}` | `request_id=0e91127b96beedb65a97936b211bed3a {"name": "Insert 1", "peak_left": 0.4983249604701996, "peak_max": 0.4983249604701996, "peak_right": 0.4754391014575958, "success": true, "track": 1}` | ok |
| 12 | `mixer.getPeaks` | `{"track": 0}` | `request_id=29edd4e7292a8ee47ca1a2accfada3e5 {"name": "Master", "peak_left": 0.48372113704681396, "peak_max": 0.4920049011707306, "peak_right": 0.4920049011707306, "success": true, "track": 0}` | ok |
| 13 | `transport.getStatus` | `{}` | `request_id=ce61ea7b29b2b27f8dbeecc1ebeb3c84 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "3:16:07", "success": true}` | ok |
| 14 | `transport.stop` | `{}` | `request_id=92735e69786094c0e6a2978d270ec073 {"stopped": true, "success": true}` | ok |

**Ergebnis:** 14 von 14 Schritten bestanden.


<!-- szenario.py 2026-09-26T12:54:20+00:00 -->
## Szenario `fenster` — Das FL-Hauptfenster und das Fenster von Nakama Probeeq werden ohne Vordergrundwechsel als Bild erfasst; ein einfarbiges Bild ist verfehlt

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=3d8097b63215a1ca2bf0a7d70544cea0 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `lokal.fenster` | `{"ziel": "fl"}` | {"breite": 1920, "dauer_s": 0.312, "einfarbig": false, "fenster": "Nakama-Diagnose.flp - FL Studio 2026", "hoehe": 1032, "klasse": "TFruityLoopsMainForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\af944ebb-fl-20260926-145420.png", "sha256": "319CC0C8A60A714C9EC4AA0CCE9CEB3E83EC381F1FF1FE3B0B31B363B84CAAD4"} | ok (Details unten) |
| 3 | `mixer.focusEditor` | `{"slot_index": 0, "track": 1}` | `request_id=fbee4004f2eb1cc505b7c03aa7c300d9 {"focused": true, "plugin": "Nakama Probeeq", "slot_index": 0, "success": true, "track": 1}` | ok |
| 4 | warte | 1 s | — | — |
| 5 | `lokal.fenster` | `{"plugin": "Nakama Probeeq", "ziel": "plugin"}` | {"breite": 571, "dauer_s": 0.125, "einfarbig": false, "fenster": "Nakama Probeeq (Insert 1)", "hoehe": 487, "klasse": "TPluginForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\af944ebb-plugin-20260926-145421.png", "sha256": "465D90CF6A8F7223E84595DF46F8A3F9F28369184DF21C7E1DACD86CFBF6E7A3"} | ok (Details unten) |

**Ergebnis:** 5 von 5 Schritten bestanden.


### Schritt 2 `lokal.fenster`

- Antwort {"breite": 1920, "dauer_s": 0.312, "einfarbig": false, "fenster": "Nakama-Diagnose.flp - FL Studio 2026", "hoehe": 1032, "klasse": "TFruityLoopsMainForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\af944ebb-fl-20260926-145420.png", "sha256": "319CC0C8A60A714C9EC4AA0CCE9CEB3E83EC381F1FF1FE3B0B31B363B84CAAD4"}

### Schritt 5 `lokal.fenster`

- Antwort {"breite": 571, "dauer_s": 0.125, "einfarbig": false, "fenster": "Nakama Probeeq (Insert 1)", "hoehe": 487, "klasse": "TPluginForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\af944ebb-plugin-20260926-145421.png", "sha256": "465D90CF6A8F7223E84595DF46F8A3F9F28369184DF21C7E1DACD86CFBF6E7A3"}

<!-- szenario.py 2026-09-26T12:54:22+00:00 -->
## Szenario `nulltest-host` — Der Render des Diagnoseprojekts im Auslieferungszustand ist bitidentisch zur Quelle (Weg R1); Render mit Verarbeitung und ohne Slots brauchen Karte U43

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=7b47b3586e36d1f67e9cbf45f5908585 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `request_id=271bf131d750de982f18d6b713a8b125 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `request_id=60ba6ad8a0d7b820d21294b7fe922127 {"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok · roh: volume=0.78125; volume_db=-5.176800727844238 |
| 4 | `mixer.getTrackInfo` | `{"track": 1}` | `request_id=81cb1cebe1507465309457c8bea513b0 {"color": "-0xbdbdc0", "index": 1, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Insert 1", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume=0.7999999998137355; volume_db=0.0 |
| 5 | `mixer.getTrackInfo` | `{"track": 0}` | `request_id=44aa7a1a2be97d6c9a3930537c7bc9f2 {"color": "-0xbdbdc0", "index": 0, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Master", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume=0.7999999998137355; volume_db=0.0 |
| 6 | `mixer.getRouting` | `{"track": 1}` | `request_id=6cf6926aa27d8f7fe4617f104a764b83 {"routes": [{"dest": 0, "dest_name": "Master", "level": 0.8, "source": 1, "source_name": "Insert 1"}], "scanned": 1, "success": true, "track_count": 18}` | ok · roh: routes.*.dest=[0]; routes.*.level=[0.8] |
| 7 | `lokal.nulltest` | `{"vergleich": "auslieferung"}` | BITIDENTISCH v=0 g=1.0 Abweichungen=0 | ok (Details unten) |
| 8 | `lokal.nulltest` | `{"projekt": "Nakama-Diagnose-Verarbeitung.flp", "vergleich": "verarbeitung_ein"}` | VORAUSSETZUNG verarbeitung_ein: Referenzprojekt fehlt (Karte U43, K-286-1) | VORAUSSETZUNG (Details unten) |
| 9 | `lokal.nulltest` | `{"projekt": "Nakama-Diagnose-Referenz.flp", "vergleich": "ohne_slots"}` | GEMESSEN ohne_slots: GLEICH v=0 g_db=0.0 Abweichungen=0 | ok (Details unten) |

**Ergebnis:** 8 von 9 Schritten bestanden.


### Schritt 7 `lokal.nulltest`

- `nulltest.py` Exit 0, Quelle `C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav`, Songlaenge 45696 ms
  - NULLTEST Urteil BITIDENTISCH · Exit 0 · N 2015193 · v 0 · abweichungen 0 · nullvorzeichen 0 · g 1.0 · g_db 0.0 · ueberlappend 2015193 Frames [0, 2015193) · sha256_render_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · sha256_quelle_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · Stichprobenspitzen Datei {'render': [0.3984098434448242, 0.4039571285247803], 'quelle': [0.3984098434448242, 0.4039571285247803]} · Stichprobenspitzen Bereich {'render': [0.3984098434448242, 0.4039571285247803], 'quelle': [0.3984098434448242, 0.4039571285247803]} · Render {'art': 'PCM', 'bits': 24, 'kanaele': 2, 'rate': 44100, 'frames': 2015409} SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A
- ergebnis.json: `{"N": 2015193, "abweichungen": 0, "erste": null, "erzeugt_utc": "2026-09-26T12:54:24+00:00", "exit": 0, "format": "nakama.laufzeit.nulltest.v1", "g": 1.0, "g_db": 0.0, "hinweis": "Offline-Latenz des Plugins 0 (Kanon A1/A16); Clip-Lage oder Latenz trennt nur Weg R2 (Karte U43)", "letzte": null, "nullvorzeichen": 0, "nullvorzeichen_erste": null, "nullvorzeichen_letzte": null, "quelle": {"format": {"art": "PCM", "bits": 24, "frames": 5470096, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\Projekte\\Nakama\\eq-copilot\\kalibration\\Testtrack.wav", "sha256_datei": "D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF"}, "render": {"format": {"art": "PCM", "bits": 24, "frames": 2015409, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "sha256_datei": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A"}, "renderstatus": {"aufruf": "C:\\Program Files\\Image-Line\\FL Studio 2026\\FL64.exe /R /Ewav /O\"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\" \"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose.flp\"", "datei": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "dauer_s": 5.2, "exit": 0, "fenstertitel": "", "format": "nakama.laufzeit.render.v1", "grund": null, "head": "af944ebb", "pid": 21920, "projekt": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose.flp", "sha256": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A", "sha256_projekt": "B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229", "zeit": "2026-09-26 14:54:00"}, "sha256_quelle_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "sha256_render_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "songlaenge_ms": 45696, "spitzen_bereich": {"quelle": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "spitzen_datei": {"quelle": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "ueberlappend": {"bis": 2015193, "frames": 2015193, "von": 0}, "urteil": "BITIDENTISCH", "v": 0}`

### Schritt 8 `lokal.nulltest`

- Renderstatus `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Verarbeitung.flp\render.json`: projekt `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose-Verarbeitung.flp`, grund "Referenzprojekt fehlt (Karte U43, K-286-1)" - kein Render dieses Zustands (M-64), nie still

### Schritt 9 `lokal.nulltest`

- Referenzprojekt `Nakama-Diagnose-Referenz.flp`: Renderstatus `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp\render.json`, SHA-256 Projekt 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Renderdauer 3.1 s, Render `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp\Nakama-Diagnose-Referenz.wav`
- `nulltest.py --vergleich ohne_slots` Exit 0, Songlaenge 45696 ms
  - NULLTEST Vergleich ohne_slots · Urteil GEMESSEN · Exit 0 · Befund GLEICH · N 2015193 · v 0 · abweichungen 0 · g 1.0 · g_db 0.0 · ueberlappend 2015193 Frames [0, 2015193) · sha256_render_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · sha256_auslieferung_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · Stichprobenspitzen Bereich {'render': [0.3984098434448242, 0.4039571285247803], 'auslieferung': [0.3984098434448242, 0.4039571285247803]} · Render {'art': 'PCM', 'bits': 24, 'kanaele': 2, 'rate': 44100, 'frames': 2015409} SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A · Projekt Nakama-Diagnose-Referenz.flp · SHA-256 Projekt 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D · Renderdauer 3.1 s
- ergebnis.json (ohne_slots): `{"N": 2015193, "abweichungen": 0, "auslieferung": {"ergebnis": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ergebnis.json", "render": {"format": {"art": "PCM", "bits": 24, "frames": 2015409, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "sha256_datei": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A"}}, "befund": "GLEICH", "erste": null, "erzeugt_utc": "2026-09-26T12:54:25+00:00", "exit": 0, "format": "nakama.laufzeit.nulltest.referenz.v1", "g": 1.0, "g_db": 0.0, "letzte": null, "projekt": "Nakama-Diagnose-Referenz.flp", "render": {"format": {"art": "PCM", "bits": 24, "frames": 2015409, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\referenz\\Nakama-Diagnose-Referenz.flp\\Nakama-Diagnose-Referenz.wav", "sha256_datei": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A"}, "renderdauer_s": 3.1, "renderstatus": {"aufruf": "C:\\Program Files\\Image-Line\\FL Studio 2026\\FL64.exe /R /Ewav /O\"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\referenz\\Nakama-Diagnose-Referenz.flp\" \"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose-Referenz.flp\"", "datei": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\referenz\\Nakama-Diagnose-Referenz.flp\\Nakama-Diagnose-Referenz.wav", "dauer_s": 3.1, "exit": 0, "fenstertitel": "", "format": "nakama.laufzeit.render.v1", "grund": null, "head": "af944ebb", "pid": 191520, "projekt": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose-Referenz.flp", "sha256": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A", "sha256_projekt": "4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D", "zeit": "2026-09-26 14:54:03"}, "sha256_auslieferung_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "sha256_projekt": "4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D", "sha256_render_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "songlaenge_ms": 45696, "spitzen_bereich": {"auslieferung": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "ueberlappend": {"bis": 2015193, "frames": 2015193, "von": 0}, "urteil": "GEMESSEN", "v": 0, "vergleich": "ohne_slots"}`

<!-- szenario.py 2026-09-26T12:54:38+00:00 -->
## Szenario `schleife-dauerlauf` — Der Host lebt und antwortet ueber mindestens fuenf Songumlaeufe Echtzeit-Wiedergabe des Diagnoseprojekts; nach jedem Umlauf antworten FL und beide Rollen im Briefkasten

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=d224d828e7f7421b75d4438e4463af9b {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `request_id=e87c9cc2dba04364bdb9a89ad187afc8 {"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `request_id=339b34f9e6b5a2f33839471bedb83474 {"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `transport.getLength` | `{}` | `request_id=2a06a2c114d7f59dc3ef6728da4edd38 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 5 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=fb6f1e42a27fb618f7da49378e05450f {"mode": "song", "success": true}` | ok |
| 6 | `transport.stop` | `{}` | `request_id=3b9189fbb9da1cfcdcd4d5105eaa8978 {"stopped": true, "success": true}` | ok |
| 7 | `transport.setPosition` | `{"mode": 0, "position": 0}` | `request_id=5146fb1e56c27cda402c6782ab926a7f {"mode": 0, "requested_position": 0, "success": true}` | ok |
| 8 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 9 | `transport.start` | `{}` | `request_id=e62b44678c24c7b5906b2fb7213d3de6 {"is_playing": true, "success": true}` | ok |
| 10 | warte | 46 s | — | — |
| 11 | `transport.getStatus` | `{}` | `request_id=37e487e25cff70cd64990d096b843c45 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "1:04:00", "success": true}` | ok · roh: position="1:04:00" |
| 12 | `transport.getPosition` | `{}` | `request_id=12a15fcfc35b69d82fa9c524606fd2d9 {"hint": "1:04:10", "ms": 366, "seconds": 0, "success": true}` | ok · roh: ms=366 |
| 13 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 14 | warte | 46 s | — | — |
| 15 | `transport.getStatus` | `{}` | `request_id=9dc8f15e36389e64b0c826520512da3d {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "2:11:19", "success": true}` | ok · roh: position="2:11:19" |
| 16 | `transport.getPosition` | `{}` | `request_id=5f3caf6a5beafb34f076cace2288d5f3 {"hint": "2:11:19", "ms": 2871, "seconds": 3, "success": true}` | ok · roh: ms=2871 |
| 17 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 18 | warte | 46 s | — | — |
| 19 | `transport.getStatus` | `{}` | `request_id=a13704b11838d524fb202a52c6989b2f {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "4:03:14", "success": true}` | ok · roh: position="4:03:14" |
| 20 | `transport.getPosition` | `{}` | `request_id=0c27e39461fe89bc80dca27fcb9479c6 {"hint": "4:03:14", "ms": 5420, "seconds": 5, "success": true}` | ok · roh: ms=5420 |
| 21 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 22 | warte | 46 s | — | — |
| 23 | `transport.getStatus` | `{}` | `request_id=0629c1f2f7cf4ea7ebd567b4e27ae7e9 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "5:11:10", "success": true}` | ok · roh: position="5:11:10" |
| 24 | `transport.getPosition` | `{}` | `request_id=8ccb11d3f3b6890b9b867bada4a485aa {"hint": "5:11:10", "ms": 7973, "seconds": 8, "success": true}` | ok · roh: ms=7973 |
| 25 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 26 | warte | 46 s | — | — |
| 27 | `transport.getStatus` | `{}` | `request_id=d76bab6a80c3216e63c13a0a04de48f1 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "7:02:18", "success": true}` | ok · roh: position="7:02:18" |
| 28 | `transport.getPosition` | `{}` | `request_id=688855cab8dbe956860587f1b63ffa85 {"hint": "7:03:05", "ms": 10522, "seconds": 11, "success": true}` | ok · roh: ms=10522 |
| 29 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 30 | `transport.stop` | `{}` | `request_id=d6d7a486b76d36e3dc47db989a6b285f {"stopped": true, "success": true}` | ok |

**Ergebnis:** 30 von 30 Schritten bestanden.


### Schritt 8 `lokal.briefkasten`

- Anfrage `d778307cecb438cc7c45633132090e44`, erwartet {"gen": [275924], "probeeq": [275924]}, Positionsklammer None → None ms
  - gen: `d778307cecb438cc7c45633132090e44.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json` (22745 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.66 s nach dem Schreiben)
  - probeeq: `d778307cecb438cc7c45633132090e44.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json` (15917 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.70 s nach dem Schreiben)
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 103
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 113

### Schritt 13 `lokal.briefkasten`

- Anfrage `b6c4ac3404b107c897ddf35cff37a630`, erwartet {"gen": [275924], "probeeq": [275924]}, Positionsklammer None → None ms
  - gen: `b6c4ac3404b107c897ddf35cff37a630.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json` (22663 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.42 s nach dem Schreiben)
  - probeeq: `b6c4ac3404b107c897ddf35cff37a630.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json` (22601 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.45 s nach dem Schreiben)
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 571
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 581

### Schritt 17 `lokal.briefkasten`

- Anfrage `14829caf79e71c7286f82d47c145c8fe`, erwartet {"gen": [275924], "probeeq": [275924]}, Positionsklammer None → None ms
  - gen: `14829caf79e71c7286f82d47c145c8fe.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json` (22782 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.20 s nach dem Schreiben)
  - probeeq: `14829caf79e71c7286f82d47c145c8fe.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json` (22772 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.23 s nach dem Schreiben)
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1039
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1049

### Schritt 21 `lokal.briefkasten`

- Anfrage `51a4e9993d6875e25b1e3ec59bd7aea9`, erwartet {"gen": [275924], "probeeq": [275924]}, Positionsklammer None → None ms
  - gen: `51a4e9993d6875e25b1e3ec59bd7aea9.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json` (22823 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.98 s nach dem Schreiben)
  - probeeq: `51a4e9993d6875e25b1e3ec59bd7aea9.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json` (22775 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.02 s nach dem Schreiben)
  - roh fremd: andere Kennung: `14829caf79e71c7286f82d47c145c8fe.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `14829caf79e71c7286f82d47c145c8fe.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1518
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1518

### Schritt 25 `lokal.briefkasten`

- Anfrage `3807ba291aa249083d5b89874855b3ec`, erwartet {"gen": [275924], "probeeq": [275924]}, Positionsklammer None → None ms
  - gen: `3807ba291aa249083d5b89874855b3ec.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json` (22854 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.78 s nach dem Schreiben)
  - probeeq: `3807ba291aa249083d5b89874855b3ec.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json` (22799 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.81 s nach dem Schreiben)
  - roh fremd: andere Kennung: `14829caf79e71c7286f82d47c145c8fe.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `14829caf79e71c7286f82d47c145c8fe.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `51a4e9993d6875e25b1e3ec59bd7aea9.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `51a4e9993d6875e25b1e3ec59bd7aea9.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1986
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1996

### Schritt 29 `lokal.briefkasten`

- Anfrage `c3672e93d3c1e03105306ba0d882cad7`, erwartet {"gen": [275924], "probeeq": [275924]}, Positionsklammer None → None ms
  - gen: `c3672e93d3c1e03105306ba0d882cad7.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json` (22865 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.58 s nach dem Schreiben)
  - probeeq: `c3672e93d3c1e03105306ba0d882cad7.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json` (22790 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.61 s nach dem Schreiben)
  - roh fremd: andere Kennung: `14829caf79e71c7286f82d47c145c8fe.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `14829caf79e71c7286f82d47c145c8fe.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `3807ba291aa249083d5b89874855b3ec.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `3807ba291aa249083d5b89874855b3ec.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `51a4e9993d6875e25b1e3ec59bd7aea9.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `51a4e9993d6875e25b1e3ec59bd7aea9.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `b6c4ac3404b107c897ddf35cff37a630.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.gen.275924.2f869d2ee2d9099f.8d7a2c3494414243a30bdd85f2ce413d.json`
  - roh fremd: andere Kennung: `d778307cecb438cc7c45633132090e44.probeeq.275924.d387a78151edaa16.2f241e106bfb40a9b15a912a6029884c.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 2454
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 2464

<!-- szenario.py 2026-09-26T12:58:55+00:00 -->
## Szenario `snapshot-runde01` — Snapshot beider Rollen ueber den Briefkasten innerhalb eines Umlaufs gegen den Referenzausschnitt der Quelle; die Runde-01-Anker stehen nur als Rohvergleich daneben

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=825f5e57d84f2a878055200db420db18 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `request_id=2e7187649556dd006db01d8edf4d2287 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `request_id=80e8c1c4f1971498f77318de30415140 {"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok · roh: volume=0.78125; volume_db=-5.176800727844238 |
| 4 | `mixer.getTrackInfo` | `{"track": 1}` | `request_id=51a27ee8db2a01f226a366e17c6ab6a3 {"color": "-0xbdbdc0", "index": 1, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Insert 1", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume_db=0.0 |
| 5 | `mixer.getTrackInfo` | `{"track": 0}` | `request_id=e48d8a09dd01a526699cdde076f113a5 {"color": "-0xbdbdc0", "index": 0, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Master", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume_db=0.0 |
| 6 | `mixer.getRouting` | `{"track": 1}` | `request_id=a45e0555b4d17e5f1f039833c2456493 {"routes": [{"dest": 0, "dest_name": "Master", "level": 0.8, "source": 1, "source_name": "Insert 1"}], "scanned": 1, "success": true, "track_count": 18}` | ok · roh: routes.*.level=[0.8] |
| 7 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=ccac2b180fa979642b385741744f29a3 {"mode": "song", "success": true}` | ok |
| 8 | `transport.stop` | `{}` | `request_id=faace76c0ee89ae9062ec7bec9bb6aa3 {"stopped": true, "success": true}` | ok |
| 9 | `transport.setPosition` | `{"mode": 0, "position": 0}` | `request_id=fca817ea23116d6d3d342325e7c01f5b {"mode": 0, "requested_position": 0, "success": true}` | ok |
| 10 | `transport.getPosition` | `{}` | `request_id=337565c869191ca26845c52f87d42ff2 {"hint": "1:01:00", "ms": 0, "seconds": 0, "success": true}` | ok · roh: ms=0; hint="1:01:00" |
| 11 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq", "broker"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 12 | `transport.start` | `{}` | `request_id=1be58b1781df935947ac7578433b868e {"is_playing": true, "success": true}` | ok |
| 13 | `lokal.umlauf` | `{"rollen": ["gen", "probeeq"], "vergleich": {"baender": [{"anker": "schwer", "delta_k": true, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "plus_s_lufs": true, "rechnung": "F-28 LUFS integriert: R_LUFS + Delta_K +- (0,07 LU + S_LUFS) ueber den Referenzausschnitt des schweren Ankers n_S; 0,07 LU KALIBRIER-PROTOKOLL.md:71", "referenz": "lufs", "rollen": ["gen", "probeeq"], "toleranz…` | gemessen: gen `ff6e2254`, probeeq `ff6e2254` | ok (Details unten) |

**Ergebnis:** 13 von 13 Schritten bestanden.


### Schritt 11 `lokal.briefkasten`

- Anfrage `f1cd68ba6c41d8f69a4e85c894ae7a44`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer None → None ms
  - broker: Broker laeuft nicht - keine Erwartung an die Rolle broker
  - gen: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22745 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.75 s nach dem Schreiben)
  - probeeq: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (15917 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.78 s nach dem Schreiben)
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 103
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 113

### Schritt 13 `lokal.umlauf`

- Messfassung aus C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\src\AnalyseEngine.h: m4.3-2026-09-26
- Nulltesturteil und Kettenverschiebung: {"delta_k_db": 0.0, "g": 1.0, "urteil": "BITIDENTISCH", "v": 0}

| # | Kennung | p_vor ms | p_nach ms | gen: zustand · aktiv s · gesamt s · E · schwer s | probeeq: zustand · aktiv s · gesamt s · E · schwer s |
|---|---|---|---|---|---|
| 1 | `0b99c56b` | 0 | 1580 | "sammelt" · 0.4 · 0.495532879818594 · 21853 · 0.263401360544218 | "sammelt" · 0.5 · 0.584829931972789 · 25791 · 0.584829931972789 |
| 2 | `cd8d0de7` | 1625 | 3576 | "sammelt" · 2.4 · 2.491065759637188 · 109856 · 2.397324263038549 | "sammelt" · 2.5 · 2.584829931972789 · 113991 · 2.584829931972789 |
| 3 | `a48cd66f` | 3621 | 5571 | "sammelt" · 4.4 · 4.486598639455782 · 197859 · 4.254467120181406 | "sammelt" · 4.5 · 4.593741496598639 · 202584 · 4.593741496598639 |
| 4 | `6668a6f4` | 5621 | 7616 | "sammelt" · 6.4 · 6.482131519274376 · 285862 · 6.392857142857143 | "sammelt" · 6.5 · 6.593741496598639 · 290784 · 6.593741496598639 |
| 5 | `ace9d4d9` | 7616 | 9567 | "sammelt" · 8.5 · 8.526780045351474 · 376031 · 8.526780045351474 | "sammelt" · 8.5 · 8.575895691609977 · 378197 · 8.575895691609977 |
| 6 | `914e3c48` | 9612 | 11562 | "sammelt" · 10.4 · 10.477687074829932 · 462066 · 10.383922902494332 | "sammelt" · 10.5 · 10.58482993197279 · 466791 · 10.58482993197279 |
| 7 | `c27b0ae9` | 11612 | 13562 | "sammelt" · 12.5 · 12.522312925170068 · 552234 · 12.522312925170068 | "sammelt" · 12.5 · 12.58482993197279 · 554991 · 12.58482993197279 |
| 8 | `04ac1af2` | 13607 | 15603 | "sammelt" · 14.4 · 14.473219954648526 · 638269 · 14.379455782312926 | "sammelt" · 14.5 · 14.580362811791383 · 642994 · 14.580362811791383 |
| 9 | `1971f0b2` | 15603 | 17603 | "messbereit" · 16.5 · 16.51340136054422 · 728241 · 16.51340136054422 | "messbereit" · 16.6 · 16.607142857142858 · 732375 · 16.607142857142858 |
| 10 | `e8b9136b` | 17603 | 19598 | "messbereit" · 18.5 · 18.51340136054422 · 816441 · 18.375011337868482 | "messbereit" · 18.6 · 18.607142857142858 · 820575 · 18.607142857142858 |
| 11 | `8fe0d0c3` | 19598 | 21643 | "messbereit" · 20.400000000000002 · 20.464285714285715 · 902475 · 20.464285714285715 | "messbereit" · 20.6 · 20.60267573696145 · 908578 · 20.60267573696145 |
| 12 | `987f6661` | 21643 | 23638 | "messbereit" · 22.5 · 22.504467120181406 · 992447 · 22.321428571428573 | "messbereit" · 22.5 · 22.598208616780045 · 996581 · 22.598208616780045 |
| 13 | `08e6cde8` | 23638 | 25589 | "messbereit" · 24.5 · 24.549115646258503 · 1082616 · 24.455351473922903 | "messbereit" · 24.5 · 24.59374149659864 · 1084584 · 24.59374149659864 |
| 14 | `9769b60e` | 25634 | 27634 | "messbereit" · 26.5 · 26.5 · 1168650 · 26.31249433106576 | "messbereit" · 26.6 · 26.607142857142858 · 1173375 · 26.607142857142858 |
| 15 | `150f8d2c` | 27634 | 29629 | "messbereit" · 28.400000000000002 · 28.495532879818594 · 1256653 · 28.450884353741497 | "messbereit" · 28.6 · 28.60267573696145 · 1261378 · 28.60267573696145 |
| 16 | `992d04ec` | 29629 | 31625 | "messbereit" · 30.400000000000002 · 30.495532879818594 · 1344853 · 30.308027210884354 | "messbereit" · 30.6 · 30.60267573696145 · 1349578 · 30.60267573696145 |
| 17 | `b806928d` | 31625 | 33621 | "messbereit" · 32.4 · 32.49106575963719 · 1432856 · 32.441972789115646 | "messbereit" · 32.5 · 32.59820861678005 · 1437581 · 32.59820861678005 |
| 18 | `30afbdbd` | 33621 | 35621 | "messbereit" · 34.5 · 34.535714285714285 · 1523025 · 34.30358276643991 | "messbereit" · 34.5 · 34.59374149659864 · 1525584 · 34.59374149659864 |
| 19 | `7f68c54b` | 35621 | 37616 | "messbereit" · 36.4 · 36.482131519274375 · 1608862 · 36.43750566893424 | "messbereit" · 36.6 · 36.62498866213152 · 1615162 · 36.62498866213152 |
| 20 | `2e4bd82f` | 37616 | 39612 | "messbereit" · 38.5 · 38.52678004535147 · 1699031 · 38.2946485260771 | "messbereit" · 38.5 · 38.589297052154194 · 1701788 · 38.589297052154194 |
| 21 | `43df56ab` | 39661 | 41612 | "messbereit" · 40.5 · 40.52678004535147 · 1787231 · 40.43303854875283 | "messbereit" · 40.6 · 40.616077097505666 · 1791169 · 40.616077097505666 |
| 22 | `cf635e13` | 41612 | 43607 | "messbereit" · 42.5 · 42.52231292517007 · 1875234 · 42.290181405895694 | "messbereit" · 42.6 · 42.616077097505666 · 1879369 · 42.616077097505666 |
| 23 | `ff6e2254` | 43652 | 45603 | "messbereit" · 44.5 · 44.517868480725625 · 1963238 · 44.42410430839002 | "messbereit" · 44.6 · 44.611609977324264 · 1967372 · 44.611609977324264 |
| 24 | `177a27fc` | 45603 | 1902 | "sammelt" · 0.8 · 0.861609977324263 · 37997 · 0.861609977324263 | "sammelt" · 0.9 · 0.924104308390023 · 40753 · 0.924104308390023 |

- Ende der Folge: wrap; 24 Anfragen, 48 Positionen
- Anfrage `0b99c56bd746e0c58f05f489553192b6`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 0 → 1580 ms
  - gen: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (21733 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.50 s nach dem Schreiben)
  - probeeq: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22663 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.55 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `cd8d0de7d26b4ec361bdda1ad902f983`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 1625 → 3576 ms
  - gen: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22704 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22760 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `a48cd66ff19f746e334868d2811bfdb0`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 3621 → 5571 ms
  - gen: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22814 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.89 s nach dem Schreiben)
  - probeeq: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22786 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `6668a6f432543c07a616c6530f318af6`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 5621 → 7616 ms
  - gen: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22803 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22782 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `ace9d4d931407f978b318c5036ffbdbd`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 7616 → 9567 ms
  - gen: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22840 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22785 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.94 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `914e3c487a0b0425fdfddd8165243755`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 9612 → 11562 ms
  - gen: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22865 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22815 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `c27b0ae92af9170308b40d71e7375bc0`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 11612 → 13562 ms
  - gen: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22860 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22820 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `04ac1af2d5b32f100e23e4dc942305dd`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 13607 → 15603 ms
  - gen: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22864 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22822 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `1971f0b290cd523997a9bc9da2f9069d`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 15603 → 17603 ms
  - gen: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (26151 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23740 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `e8b9136bcf4066d4c7cd168fb7139e01`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 17603 → 19598 ms
  - gen: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25997 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23534 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `8fe0d0c315fff1e956caf48d7c2089cc`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 19598 → 21643 ms
  - gen: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25852 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23441 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `987f6661df4cf19006167d8004dc87ed`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 21643 → 23638 ms
  - gen: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25692 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23266 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `08e6cde87a39cbeb122ff6517c04ec49`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 23638 → 25589 ms
  - gen: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25708 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23259 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `9769b60e26d3a3fa46b0661d3d04769b`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 25634 → 27634 ms
  - gen: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25688 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23293 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `150f8d2c5d9125065425a036f4423391`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 27634 → 29629 ms
  - gen: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25693 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23285 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.94 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `992d04ecdf3064e3771565e4b3af8142`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 29629 → 31625 ms
  - gen: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25691 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23287 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `b806928d55840c645de8bc33fc0ec974`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 31625 → 33621 ms
  - gen: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25647 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23261 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.94 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `30afbdbdca74fb44863cfddda3338081`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 33621 → 35621 ms
  - gen: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25676 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23274 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `7f68c54b82bc7e13e31ed3010e82ea8f`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 35621 → 37616 ms
  - gen: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25735 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23274 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `2e4bd82ff1ecdeaad498cac65d935930`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 37616 → 39612 ms
  - gen: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25728 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23271 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `43df56ab849f1c368d1d5da95ad3af5a`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 39661 → 41612 ms
  - gen: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25721 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23275 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `cf635e13f2ecb4b79295ee582e056773`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 41612 → 43607 ms
  - gen: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25724 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23303 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `ff6e2254b2d896cec250b6e7f575c608`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 43652 → 45603 ms
  - gen: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (25729 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (23277 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `177a27fc2266abdb21d79bf7ab687e79`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 45603 → 1902 ms
  - gen: `177a27fc2266abdb21d79bf7ab687e79.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22702 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `177a27fc2266abdb21d79bf7ab687e79.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22641 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `04ac1af2d5b32f100e23e4dc942305dd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `08e6cde87a39cbeb122ff6517c04ec49.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `0b99c56bd746e0c58f05f489553192b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `150f8d2c5d9125065425a036f4423391.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `1971f0b290cd523997a9bc9da2f9069d.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2e4bd82ff1ecdeaad498cac65d935930.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `30afbdbdca74fb44863cfddda3338081.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `43df56ab849f1c368d1d5da95ad3af5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `6668a6f432543c07a616c6530f318af6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `7f68c54b82bc7e13e31ed3010e82ea8f.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `8fe0d0c315fff1e956caf48d7c2089cc.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `914e3c487a0b0425fdfddd8165243755.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9769b60e26d3a3fa46b0661d3d04769b.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `987f6661df4cf19006167d8004dc87ed.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `992d04ecdf3064e3771565e4b3af8142.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `a48cd66ff19f746e334868d2811bfdb0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ace9d4d931407f978b318c5036ffbdbd.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `b806928d55840c645de8bc33fc0ec974.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `c27b0ae92af9170308b40d71e7375bc0.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cd8d0de7d26b4ec361bdda1ad902f983.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `cf635e13f2ecb4b79295ee582e056773.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e8b9136bcf4066d4c7cd168fb7139e01.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f1cd68ba6c41d8f69a4e85c894ae7a44.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- gen: `177a27fc` E 37997 < 1963238: Anlaufdaten des naechsten Umlaufs, nie gewertet
- gen: gewertet `ff6e2254b2d896cec250b6e7f575c608.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (p_vor 43652 ms), Anker {"bloecke_max_samples": 197, "e": 1963238, "fortlaufend": 1963238, "k": 0, "leicht": [0, 1963238], "n_l": 1963238, "n_s": 1959103, "rate": 44100.0, "s0": 0, "schwer": [0, 1959103], "stillstand": 0}
- probeeq: `177a27fc` E 40753 < 1967372: Anlaufdaten des naechsten Umlaufs, nie gewertet
- probeeq: gewertet `ff6e2254b2d896cec250b6e7f575c608.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (p_vor 43652 ms), Anker {"bloecke_max_samples": 197, "e": 1967372, "fortlaufend": 1967372, "k": 0, "leicht": [0, 1967372], "n_l": 1967372, "n_s": 1967372, "rate": 44100.0, "s0": 0, "schwer": [0, 1967372], "stillstand": 0}
- Rechnung F-28 (282.7 s): `{"analyze": "C:\\Users\\phili\\FL-Studio\\tools\\analyze-track.py", "anker": [{"analyze_s": 18.25, "art": "leicht", "ausschnitt": [0, 1963238], "centroid_mag": 828.5258947850594, "corr": 0.5039285811199503, "low_frac": 0.7815396977706075, "low_frac_kanal": 0.7593726124897107, "lufs": -22.380774337530216, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-af944ebb-0-1963238.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "sha256": "2F88386C422FF4967ED798C1AC5299223FF166A88DD2D4D74E42C463E91576D3", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3303634821112413}, {"analyze_s": 2.86, "art": "schwer", "ausschnitt": [0, 1959103], "centroid_mag": 828.4898807392367, "corr": 0.5041019540009194, "low_frac": 0.781830556250053, "low_frac_kanal": 0.7596879001110316, "lufs": -22.37693629413697, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-af944ebb-0-1959103.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "s_lufs": {"arbeiter": 12, "dauer_s": 115.16, "max": -22.370761701932484, "min": -22.37693629413697, "nicht_endlich": 0, "phi0": -22.37693629413697, "phi_max": 4409, "phi_min": 0, "spanne": 0.006174592204487794, "versaetze": 4410}, "sha256": "A8F1A20919770F15D66F4FE6A12FA755893CE593434ADC12E3940DADADC2210C", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3302151645797923}, {"analyze_s": 3.38, "art": "leicht", "ausschnitt": [0, 1967372], "centroid_mag": 828.501939386266, "corr": 0.5038705227494916, "low_frac": 0.781279697394338, "low_frac_kanal": 0.7590869167154394, "lufs": -22.385016854426702, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-af944ebb-0-1967372.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "sha256": "7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3304148640349438}, {"analyze_s": 3.38, "art": "schwer", "ausschnitt": [0, 1967372], "centroid_mag": 828.501939386266, "corr": 0.5038705227494916, "low_frac": 0.781279697394338, "low_frac_kanal": 0.7590869167154394, "lufs": -22.385016854426702, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-af944ebb-0-1967372.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "s_lufs": {"arbeiter": 12, "dauer_s": 141.72, "max": -22.37887414486701, "min": -22.38503200234511, "nicht_endlich": 0, "phi0": -22.385016854426702, "phi_max": 4390, "phi_min": 1293, "spanne": 0.0061578574781009365, "versaetze": 4410}, "sha256": "7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3304148640349438}], "dauer_s": 281.88, "quelle_sha256": "D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF", "u_unten": [{"delta_db": 4.2536484549686325e-12, "eps": 9.79438752324313e-13, "frames_ausserhalb_quelle": 0, "max_zellen": 445, "min_zellen": 444, "n_l": 1963238, "phi_min": 789, "rolle": "gen", "s0": 0, "schwelle_db": -59.999999999995744, "u_unten_s": 44.4, "zellen_phi0": 445}, {"delta_db": 4.2536484549686325e-12, "eps": 9.79438752324313e-13, "frames_ausserhalb_quelle": 0, "max_zellen": 446, "min_zellen": 445, "n_l": 1967372, "phi_min": 513, "rolle": "probeeq", "s0": 0, "schwelle_db": -59.999999999995744, "u_unten_s": 44.5, "zellen_phi0": 446}]}`
- Referenzausschnitt gen leicht: Frames [0, 1963238), K 0, v 0, SHA-256 2F88386C422FF4967ED798C1AC5299223FF166A88DD2D4D74E42C463E91576D3, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-af944ebb-0-1963238.wav
- Referenzausschnitt gen schwer: Frames [0, 1959103), K 0, v 0, SHA-256 A8F1A20919770F15D66F4FE6A12FA755893CE593434ADC12E3940DADADC2210C, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-af944ebb-0-1959103.wav
- Referenzausschnitt probeeq leicht: Frames [0, 1967372), K 0, v 0, SHA-256 7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-af944ebb-0-1967372.wav
- Referenzausschnitt probeeq schwer: Frames [0, 1967372), K 0, v 0, SHA-256 7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-af944ebb-0-1967372.wav
- gen: U_unten 44.4 s = 0,1 s x min Z_phi (phi 789; phi 0: 445 Zellen; Schwelle -59.999999999995744 dB, Abstand 4.2536484549686325e-12 dB), aktiv_sekunden 44.5
- gen roh `snapshot.spektral.centroid_mag_hz` = [886.6302174060226]
- gen roh `snapshot.loudness.lufs_integriert` = [-22.37693629413699]
- gen roh `snapshot.loudness.true_peak_dbtp` = [-7.867515515804746]
- gen roh `snapshot.stereo.width` = [0.330215164579874]
- gen roh `snapshot.stereo.corr` = [0.504101953999153]
- gen roh `snapshot.spektral.low_frac` = [0.759687901691955]
- gen roh `snapshot.resonanzen.*.freq_hz` = [688.6962107092802, 459.64957473531535, 919.2991494706307]
- gen roh `snapshot.zustand` = ["messbereit"]
- gen roh `snapshot.aktiv_sekunden` = [44.5]
- gen roh `snapshot.gesamt_sekunden` = [44.517868480725625]
- gen roh `frame.schwer_sekunden` = [44.42410430839002]
- gen roh `frame.material_ende_projektsample` = [1963238]
- gen roh `frame.hostzeit_fortlaufend_samples` = [1963238]
- gen roh `frame.hostzeit_stillstand_bloecke` = [0]
- gen roh `frame.bloecke_max_samples` = [197]
- probeeq: U_unten 44.5 s = 0,1 s x min Z_phi (phi 513; phi 0: 446 Zellen; Schwelle -59.999999999995744 dB, Abstand 4.2536484549686325e-12 dB), aktiv_sekunden 44.6
- probeeq roh `snapshot.spektral.centroid_mag_hz` = [886.6096345840435]
- probeeq roh `snapshot.loudness.lufs_integriert` = [-22.385016854426723]
- probeeq roh `snapshot.loudness.true_peak_dbtp` = [-7.867515515804746]
- probeeq roh `snapshot.stereo.width` = [0.330414864035028]
- probeeq roh `snapshot.stereo.corr` = [0.503870522747712]
- probeeq roh `snapshot.spektral.low_frac` = [0.759086918280695]
- probeeq roh `snapshot.resonanzen.*.freq_hz` = [688.6962107092802, 459.64957473531535, 919.2991494706307]
- probeeq roh `snapshot.zustand` = ["messbereit"]
- probeeq roh `snapshot.aktiv_sekunden` = [44.6]
- probeeq roh `snapshot.gesamt_sekunden` = [44.611609977324264]
- probeeq roh `frame.schwer_sekunden` = [44.611609977324264]
- probeeq roh `frame.material_ende_projektsample` = [1967372]
- probeeq roh `frame.hostzeit_fortlaufend_samples` = [1967372]
- probeeq roh `frame.hostzeit_stillstand_bloecke` = [0]
- probeeq roh `frame.bloecke_max_samples` = [197]
- Band LUFS gen: {"ausschnitt": [0, 1959103], "breite": 0.0761745922044878, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "mitte": -22.37693629413697, "referenz": -22.37693629413697, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": -22.37693629413699}
- Band TP gen: {"feld": "snapshot.loudness.true_peak_dbtp", "grund": "kein Band fuer diese Rolle", "kurz": "TP", "rolle": "gen", "status": "roh", "wert": -7.867515515804746}
- Band width gen: {"ausschnitt": [0, 1959103], "breite": 0.01, "feld": "snapshot.stereo.width", "kurz": "width", "mitte": 0.3302151645797923, "referenz": 0.3302151645797923, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.330215164579874}
- Band corr gen: {"ausschnitt": [0, 1959103], "breite": 0.01, "feld": "snapshot.stereo.corr", "kurz": "corr", "mitte": 0.5041019540009194, "referenz": 0.5041019540009194, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.504101953999153}
- Band low_frac gen: {"ausschnitt": [0, 1959103], "breite": 0.02, "feld": "snapshot.spektral.low_frac", "kurz": "low_frac", "mitte": 0.7596879001110316, "referenz": 0.7596879001110316, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.759687901691955}
- Band resonanzen gen: {"ausschnitt": [0, 1959103], "feld": "snapshot.resonanzen.*.freq_hz", "kurz": "resonanzen", "referenz": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": [688.6962107092802, 459.64957473531535, 919.2991494706307]}
- Band LUFS probeeq: {"ausschnitt": [0, 1967372], "breite": 0.07615785747810094, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "mitte": -22.385016854426702, "referenz": -22.385016854426702, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": -22.385016854426723}
- Band TP probeeq: {"ausschnitt": [0, 1967372], "breite": 0.12, "feld": "snapshot.loudness.true_peak_dbtp", "kurz": "TP", "mitte": -7.867515649360563, "referenz": -7.867515649360563, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": -7.867515515804746}
- Band width probeeq: {"ausschnitt": [0, 1967372], "breite": 0.01, "feld": "snapshot.stereo.width", "kurz": "width", "mitte": 0.3304148640349438, "referenz": 0.3304148640349438, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.330414864035028}
- Band corr probeeq: {"ausschnitt": [0, 1967372], "breite": 0.01, "feld": "snapshot.stereo.corr", "kurz": "corr", "mitte": 0.5038705227494916, "referenz": 0.5038705227494916, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.503870522747712}
- Band low_frac probeeq: {"ausschnitt": [0, 1967372], "breite": 0.02, "feld": "snapshot.spektral.low_frac", "kurz": "low_frac", "mitte": 0.7590869167154394, "referenz": 0.7590869167154394, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.759086918280695}
- Band resonanzen probeeq: {"ausschnitt": [0, 1967372], "feld": "snapshot.resonanzen.*.freq_hz", "kurz": "resonanzen", "referenz": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": [688.6962107092802, 459.64957473531535, 919.2991494706307]}
- Rohvergleich Runde 01 (nur daneben, F-28): {"centroid_offline": 829, "centroid_plugin": [876, 879], "corr": [0.5, 0.507], "low_frac_offline": 0.78, "lufs_offline": -22.41, "lufs_plugin": [-22.45, -22.48], "quelle": "eq-copilot/kalibration/KALIBRIER-PROTOKOLL.md:68-69", "resonanzen_offline": [229.8, 354.4, 459.6, 546.6, 688.7, 919.3], "resonanzen_plugin": [688.7, 459.6, 919.3], "tp_offline": -7.87, "tp_plugin": [-7.75, -6.47], "width": [0.33, 0.328]}

<!-- szenario.py 2026-09-26T13:04:28+00:00 -->
## Szenario `u40-aktivitaetsgate` — Karte U40 als Messung: Aktivitaet, Abdeckung, LUFS-Short, True Peak und gezaehlte Fenster und Rahmen je leiser Stelle des Testtracks - Rohdaten, keine Deutung

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=aa42ca507f92edd4ed51e97d6efa1254 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `request_id=68661699227f3ebc60dc00ef01bcb1a8 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=7efc2759da460fc242a57a82e2c6605c {"mode": "song", "success": true}` | ok |
| 4 | `lokal.stellen` | `{"rollen": ["gen", "probeeq"], "stellen": [{"bis_s": 3.692, "energie": {"anteil_66_40": 0.0, "max_db": -22.628, "min_db": -27.205, "mittel_db": -25.205, "zellen": 36}, "id": "S1", "name": "Intro", "takte": "1-2", "von_s": 0.0}, {"bis_s": 11.077, "energie": {"anteil_66_40": 0.0, "max_db": -22.147, "min_db": -27.558, "mittel_db": -25.112, "zellen": 73}, "id": "S2", "name": "ruhige Passage", "takte"…` | gemessen, Rohdaten unten | ok (Details unten) |

**Ergebnis:** 4 von 4 Schritten bestanden.


### Schritt 4 `lokal.stellen`

- Quelle `C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav` SHA-256 D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF, erwartet D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF
- Versatz v = 0 Frames; Nulltest {"delta_k_db": 0.0, "g": 1.0, "urteil": "BITIDENTISCH", "v": 0}

#### Stelle S1 Intro: Quelle [0.0, 3.692) s, Host [0, 162817) Frames
- Stopp `{"request_id": "59a475e90e79fc4c7ca0a3653b4f0f5a", "stopped": true, "success": true}`; setPosition 0 ms `{"mode": 0, "request_id": "d67049c26aeab900808ae122eff2e0b2", "requested_position": 0, "success": true}`; getPosition roh `{"hint": "1:01:00", "ms": 0, "request_id": "b8743f9525a37b0470e464cc54882624", "seconds": 0, "success": true}` nach 0.032 s
- Phasenmessung (nicht gezaehlt) `322e5b12`: je Rolle {"gen": 0.549, "probeeq": 1.52} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.562, "probeeq": 1.531} s; gemeinsamer Zeitplan: 8 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle; getrennte Durchlaeufe je Rolle
- Durchlauf gen: Zeitplan erste Anfrage -0.79 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.957 s
- Durchlauf gen: Start `{"is_playing": true, "request_id": "14a54c53ca78576b493581bbdaf63210", "success": true}` nach 0.016 s (geplant ab +0.003 s), erste Anfrage geplant -0.79 s zum Start
- Durchlauf gen: Ende der Anfragen: Stellenende erreicht; Stopp `{"request_id": "e16ebceaf3de608e9a7dcdc233fb50ce", "stopped": true, "success": true}`
- Durchlauf probeeq: Stopp `{"request_id": "933fbfed7c78937fb5fb26e8ebad5ba9", "stopped": true, "success": true}`; setPosition 0 ms `{"mode": 0, "request_id": "743764e3889805d8c800178f86010d2b", "requested_position": 0, "success": true}`; getPosition roh `{"hint": "1:01:00", "ms": 0, "request_id": "e6d01e310564fa0c9301ba7554e75d7c", "seconds": 0, "success": true}` nach 0.032 s
- Durchlauf probeeq: Zeitplan erste Anfrage -0.62 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.425 s
- Durchlauf probeeq: Start `{"is_playing": true, "request_id": "89e7bb580f8fc6e3add957b80b83e84a", "success": true}` nach 0.031 s (geplant ab +0.004 s), erste Anfrage geplant -0.62 s zum Start
- Durchlauf probeeq: Ende der Anfragen: Stellenende erreicht; Stopp `{"request_id": "03eb794f1b4b96fbd1c98e08a3cd672c", "stopped": true, "success": true}`
- Anfrage `322e5b1287e5297e4ca27e759c5ddeba`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer None → None ms
  - gen: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22249 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.56 s nach dem Schreiben)
  - probeeq: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22199 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.53 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `244d6ece98ea245048f53377f0206336`, erwartet {"gen": [315912]}, Positionsklammer 0 → 0 ms
  - gen: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22249 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.81 s nach dem Schreiben)
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `92d1ad68f2507dfcf3503efd044dfc80`, erwartet {"gen": [315912]}, Positionsklammer 138 → 696 ms
  - gen: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22659 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.52 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `52170b5714b8492e937a31362507d631`, erwartet {"gen": [315912]}, Positionsklammer 1205 → 1670 ms
  - gen: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22670 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.48 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `75f298e94f6a2a793b8056eb3ffaf3c4`, erwartet {"gen": [315912]}, Positionsklammer 2228 → 2692 ms
  - gen: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22808 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.47 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `e24490b7f84793ed328baf45c4c4a9a7`, erwartet {"gen": [315912]}, Positionsklammer 3205 → 3714 ms
  - gen: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22816 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.45 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `d2421e6ea5b8857214434e92cbbb4ec4`, erwartet {"gen": [315912]}, Positionsklammer 4272 → 4692 ms
  - gen: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22854 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.44 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: Rolle probeeq ohne Erwartung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `e00570f447eebfec862835bb18d9f462`, erwartet {"probeeq": [315912]}, Positionsklammer 0 → 1205 ms
  - probeeq: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (15969 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.89 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: Rolle gen ohne Erwartung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `39b2baa369670e84403a069f0f131c5a`, erwartet {"probeeq": [315912]}, Positionsklammer 1254 → 3205 ms
  - probeeq: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22779 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: Rolle gen ohne Erwartung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `9fa21bb0629f402f9e5b14f58fc2c0b6`, erwartet {"probeeq": [315912]}, Positionsklammer 3250 → 5201 ms
  - probeeq: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22759 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: Rolle gen ohne Erwartung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S1 | gen | getrennt | 16/16 | `244d6ece` | Materialausschnitt null: zaehlt nie | false | 0.0 | null | null | null | null | null | null | 1033 | 7423 | 0 | 7 | 3371 | 4528 | 0 | null | null | false | [0, 162817) v = 0 | 0 | 0 |
| S1 | gen | getrennt | 16/16 | `92d1ad68` | gezaehlt | true | 1.0 | 1.0 | 0.788220286369324 | null | -17.323007583618164 | -17.323062896728516 | 2 | 1044 | 7448 | 1 | 1 | 3381 | 4528 | 0 | 0 | 25594 | true | [0, 162817) v = 0 | 138 | 696 |
| S1 | gen | getrennt | 16/16 | `52170b57` | gezaehlt | false | 1.0 | null | null | null | -14.135613441467285 | -14.137906074523926 | null | 1064 | 7468 | 3 | 3 | 3391 | 4528 | 0 | 63488 | 7387 | true | [0, 162817) v = 0 | 1205 | 1670 |
| S1 | gen | getrennt | 16/16 | `75f298e9` | gezaehlt | false | 1.0 | null | null | null | -15.566499710083008 | -15.5695219039917 | null | 1084 | 7488 | 4 | 4 | 3401 | 4528 | 0 | 98304 | 17852 | true | [0, 162817) v = 0 | 2228 | 2692 |
| S1 | gen | getrennt | 16/16 | `e24490b7` | gezaehlt | true | 1.0 | 1.0 | 0.866093814373016 | -24.45303726196289 | -14.812978744506836 | -14.815757751464844 | 1 | 1110 | 7514 | 0 | 0 | 3411 | 4529 | 0 | 139264 | 22174 | true | [0, 162817) v = 0 | 3205 | 3714 |
| S1 | gen | getrennt | 16/16 | `d2421e6e` | Ausschnitt [188416, 206719) nicht ganz in [0, 162817) | false | 1.0 | null | null | -25.188722610473633 | -16.528411865234375 | -16.529401779174805 | null | 1130 | 7534 | 2 | 2 | 3421 | 4528 | 0 | 188416 | 18303 | true | [0, 162817) v = 0 | 4272 | 4692 |
| S1 | probeeq | getrennt | 16/16 | `e00570f4` | gezaehlt | true | 1.0 | 1.0 | 0.8194819688797 | null | -14.590160369873047 | -14.591312408447266 | 2 | 1139 | 7593 | 0 | 0 | 3450 | 4528 | 0 | 24576 | 28383 | true | [0, 162817) v = 0 | 0 | 1205 |
| S1 | probeeq | getrennt | 16/16 | `39b2baa3` | gezaehlt | false | 1.0 | null | null | -24.89150047302246 | -14.225686073303223 | -14.230125427246094 | null | 1179 | 7633 | 3 | 3 | 3469 | 4528 | 0 | 131072 | 7922 | true | [0, 162817) v = 0 | 1254 | 3205 |
| S1 | probeeq | getrennt | 16/16 | `9fa21bb0` | Ausschnitt [204800, 225028) nicht ganz in [0, 162817) | false | 1.0 | null | null | -25.31037139892578 | -14.770365715026855 | -14.771775245666504 | null | 1219 | 7673 | 6 | 6 | 3488 | 4528 | 0 | 204800 | 20228 | true | [0, 162817) v = 0 | 3250 | 5201 |

- Stelle S1 gen (durchlauf getrennt, kombinationen 16/16): {"antworten": 4, "d_frames_gebaut": 30, "d_summe_fenster_aktiv": 66, "d_summe_fenster_gesamt": 66, "erster_ausschnitt": [0, 25594], "letzter_ausschnitt": [139264, 161438]}
- Stelle S1 probeeq (durchlauf getrennt, kombinationen 16/16): {"antworten": 2, "d_frames_gebaut": 19, "d_summe_fenster_aktiv": 40, "d_summe_fenster_gesamt": 40, "erster_ausschnitt": [24576, 52959], "letzter_ausschnitt": [131072, 138994]}

#### Stelle S2 ruhige Passage: Quelle [3.692, 11.077) s, Host [162817, 488496) Frames
- Stopp `{"request_id": "90fff109a46b49507d3ee88252165d00", "stopped": true, "success": true}`; setPosition 3692 ms `{"mode": 0, "request_id": "54c4a6a0d21359da70b4948af72f1c52", "requested_position": 3692, "success": true}`; getPosition roh `{"hint": "3:03:11", "ms": 3692, "request_id": "1ddea8e1366db1b1b0c9ed68390baef6", "seconds": 4, "success": true}` nach 0.031 s
- Phasenmessung (nicht gezaehlt) `4a641bb8`: je Rolle {"gen": 0.714, "probeeq": 1.681} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.719, "probeeq": 1.703} s; gemeinsamer Zeitplan: 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle
- Durchlauf gen und probeeq: Zeitplan erste Anfrage -0.30 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 1.819 s
- Durchlauf gen und probeeq: Start `{"is_playing": true, "request_id": "cdb3d9d78a94ca486236072663c5d64c", "success": true}` nach 0.031 s (geplant ab -0.010 s), erste Anfrage geplant -0.30 s zum Start
- Durchlauf gen und probeeq: Ende der Anfragen: Stellenende erreicht; Stopp `{"request_id": "8e08ef3223c721915477fb12250be8cf", "stopped": true, "success": true}`
- Anfrage `4a641bb8c2c2b556f91e0d90c11a1c51`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer None → None ms
  - gen: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (15951 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.72 s nach dem Schreiben)
  - probeeq: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (15906 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.70 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `2cc2d1d3f519d93f4422f3c1d818cd62`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 3692 → 5223 ms
  - gen: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22559 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.89 s nach dem Schreiben)
  - probeeq: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22504 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.84 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `862ad6997100aff090ca640216055288`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 5223 → 7223 ms
  - gen: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22682 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.00 s nach dem Schreiben)
  - probeeq: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22731 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `57cde9242aefb9083d37597417c87d14`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 7268 → 9263 ms
  - gen: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22842 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.00 s nach dem Schreiben)
  - probeeq: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22791 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `49b4dc0521311a18c81eb707ac343120`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 9263 → 11263 ms
  - gen: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22838 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.98 s nach dem Schreiben)
  - probeeq: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22757 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
- Anfrage `41ac235862cad8a46f379f88ccfeec13`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer 11263 → 13259 ms
  - gen: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22840 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.98 s nach dem Schreiben)
  - probeeq: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22789 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S2 | gen | gemeinsam | 16/16 | `2cc2d1d3` | gezaehlt | true | 1.0 | 1.0 | 0.809194624423981 | null | -17.663484573364258 | -17.663484573364258 | 1 | 1251 | 7756 | 0 | 0 | 3528 | 4528 | 0 | 162817 | 23230 | true | [162817, 488496) v = 0 | 3692 | 5223 |
| S2 | gen | gemeinsam | 16/16 | `862ad699` | gezaehlt | false | 1.0 | null | null | null | -15.137248039245605 | -15.137248039245605 | null | 1291 | 7796 | 2 | 2 | 3547 | 4528 | 0 | 252929 | 19152 | true | [162817, 488496) v = 0 | 5223 | 7223 |
| S2 | gen | gemeinsam | 16/16 | `57cde924` | gezaehlt | true | 1.0 | 1.0 | 0.824372112751007 | -24.563261032104492 | -16.149852752685547 | -16.151063919067383 | 2 | 1337 | 7842 | 0 | 0 | 3567 | 4528 | 0 | 334849 | 27401 | true | [162817, 488496) v = 0 | 7268 | 9263 |
| S2 | gen | gemeinsam | 16/16 | `49b4dc05` | gezaehlt | false | 1.0 | null | null | -24.485902786254883 | -15.791512489318848 | -15.791512489318848 | null | 1377 | 7882 | 3 | 3 | 3586 | 4528 | 0 | 441345 | 6939 | true | [162817, 488496) v = 0 | 9263 | 11263 |
| S2 | gen | gemeinsam | 16/16 | `41ac2358` | Ausschnitt [515073, 538847) nicht ganz in [162817, 488496) | true | 1.0 | 1.0 | 0.769561052322388 | -25.157520294189453 | -12.947218894958496 | -12.947218894958496 | 1 | 1423 | 7928 | 0 | 0 | 3606 | 4528 | 0 | 515073 | 23774 | true | [162817, 488496) v = 0 | 11263 | 13259 |
| S2 | probeeq | gemeinsam | 16/16 | `2cc2d1d3` | gezaehlt | true | 1.0 | 1.0 | 0.867649555206299 | null | -14.770365715026855 | -14.771775245666504 | 1 | 1256 | 7776 | 1 | 1 | 3537 | 4591 | 0 | 203777 | 23023 | true | [162817, 488496) v = 0 | 3692 | 5223 |
| S2 | probeeq | gemeinsam | 16/16 | `862ad699` | gezaehlt | false | 1.0 | null | null | -24.958499908447266 | -12.061075210571289 | -12.0626802444458 | null | 1296 | 7816 | 4 | 4 | 3557 | 4528 | 0 | 310273 | 6892 | true | [162817, 488496) v = 0 | 5223 | 7223 |
| S2 | probeeq | gemeinsam | 16/16 | `57cde924` | gezaehlt | true | 1.0 | 1.0 | 0.871445059776306 | -24.111417770385742 | -15.413409233093262 | -15.41372299194336 | 2 | 1342 | 7862 | 1 | 1 | 3576 | 4528 | 0 | 375809 | 27194 | true | [162817, 488496) v = 0 | 7268 | 9263 |
| S2 | probeeq | gemeinsam | 16/16 | `49b4dc05` | Ausschnitt [474113, 493565) nicht ganz in [162817, 488496) | false | 1.0 | null | null | -24.24235725402832 | -16.860294342041016 | -16.861051559448242 | null | 1382 | 7902 | 4 | 4 | 3596 | 4528 | 0 | 474113 | 19452 | true | [162817, 488496) v = 0 | 9263 | 11263 |
| S2 | probeeq | gemeinsam | 16/16 | `41ac2358` | Ausschnitt [556033, 579600) nicht ganz in [162817, 488496) | true | 1.0 | 1.0 | 0.737211108207703 | -24.795475006103516 | -13.75872802734375 | -13.764945983886719 | 1 | 1428 | 7948 | 1 | 1 | 3615 | 4528 | 0 | 556033 | 23567 | true | [162817, 488496) v = 0 | 11263 | 13259 |

- Stelle S2 gen (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 4, "d_frames_gebaut": 58, "d_summe_fenster_aktiv": 126, "d_summe_fenster_gesamt": 126, "erster_ausschnitt": [162817, 186047], "letzter_ausschnitt": [441345, 448284]}
- Stelle S2 probeeq (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 3, "d_frames_gebaut": 39, "d_summe_fenster_aktiv": 86, "d_summe_fenster_gesamt": 86, "erster_ausschnitt": [203777, 226800], "letzter_ausschnitt": [375809, 403003]}

#### Stelle S3 Ausklang: Quelle [42.462, 45.596) s, Host [1872574, 2010784) Frames
- Stopp `{"request_id": "28036a417f65cabde097d9fb0378ba7c", "stopped": true, "success": true}`; setPosition 42462 ms `{"mode": 0, "request_id": "cceb95f529a3783e8ecbb34310f20824", "requested_position": 42462, "success": true}`; getPosition roh `{"hint": "25:13:07", "ms": 42460, "request_id": "4fe75f4bedde3a8f8c94226a8e671e81", "seconds": 42, "success": true}` nach 0.016 s
- Phasenmessung (nicht gezaehlt) `085b6f4a`: je Rolle {"gen": 0.707, "probeeq": 1.676} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.718, "probeeq": 1.703} s; gemeinsamer Zeitplan: 2 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle; getrennte Durchlaeufe je Rolle
- Durchlauf gen: Zeitplan erste Anfrage -1.09 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.674 s
- Durchlauf gen: Start `{"is_playing": true, "request_id": "b4101637bbd6d72bf94a8e24d6d35787", "success": true}` nach 0.015 s (geplant ab +0.000 s), erste Anfrage geplant -1.09 s zum Start
- Durchlauf gen: Ende der Anfragen: wrap; Stopp `{"request_id": "ea1e21e5cec6f60ec1946fa6f3b990ea", "stopped": true, "success": true}`
- Durchlauf probeeq: Stopp `{"request_id": "e9ca8f3accbe809ea80e8d8c54de1e0f", "stopped": true, "success": true}`; setPosition 42462 ms `{"mode": 0, "request_id": "49f93f8214e9c732a280803bf8beacec", "requested_position": 42462, "success": true}`; getPosition roh `{"hint": "25:13:07", "ms": 42460, "request_id": "c1861bb926df4d3a9dcc56fc41e83a5f", "seconds": 42, "success": true}` nach 0.016 s
- Durchlauf probeeq: Zeitplan erste Anfrage -0.55 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.162 s
- Durchlauf probeeq: Start `{"is_playing": true, "request_id": "e35fd4f7a1b69017714bf25ba6004bf8", "success": true}` nach 0.016 s (geplant ab +0.006 s), erste Anfrage geplant -0.55 s zum Start
- Durchlauf probeeq: Ende der Anfragen: wrap; Stopp `{"request_id": "d0657bb3cc9eed9483d99ec694a236f9", "stopped": true, "success": true}`
- Anfrage `085b6f4addf6ee8cf797ea849421df15`, erwartet {"gen": [315912], "probeeq": [315912]}, Positionsklammer None → None ms
  - gen: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (15957 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.72 s nach dem Schreiben)
  - probeeq: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (15907 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.70 s nach dem Schreiben)
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `26ad647bf80434b218a715cdaf7c8818`, erwartet {"gen": [315912]}, Positionsklammer 42460 → 42509 ms
  - gen: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (15952 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.09 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `847ab994d33698cc622048d6281bc8f7`, erwartet {"gen": [315912]}, Positionsklammer 42509 → 42879 ms
  - gen: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (21569 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.36 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `5bea81184744c82d757d3a3e02c2cd92`, erwartet {"gen": [315912]}, Positionsklammer 43531 → 43902 ms
  - gen: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22555 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.36 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `37cfdc19a13e9e562f8655a2d981686d`, erwartet {"gen": [315912]}, Positionsklammer 44554 → 44924 ms
  - gen: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (22747 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.33 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `5bb9a55712cd75fc35975a58e8b7e28b`, erwartet {"gen": [315912]}, Positionsklammer 45571 → 196 ms
  - gen: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json` (13772 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.30 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `bf652fc0faf10a27c9b8d68fd7d8e332`, erwartet {"probeeq": [315912]}, Positionsklammer 42460 → 43438 ms
  - probeeq: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (1278 B, Version '0.3.0', gruende ['noch_keine_messdaten', 'rolle_ohne_aggregat'], 1.51 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: Rolle gen ohne Erwartung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `eefa4cdaa9a33766ebff53e1f3e049fa`, erwartet {"probeeq": [315912]}, Positionsklammer 43438 → 45433 ms
  - probeeq: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22657 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: Rolle gen ohne Erwartung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
- Anfrage `f07fd8e8ba118b089ba1e62230023060`, erwartet {"probeeq": [315912]}, Positionsklammer 45433 → 1732 ms
  - probeeq: `f07fd8e8ba118b089ba1e62230023060.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json` (22650 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `085b6f4addf6ee8cf797ea849421df15.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `244d6ece98ea245048f53377f0206336.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `26ad647bf80434b218a715cdaf7c8818.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `2cc2d1d3f519d93f4422f3c1d818cd62.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `322e5b1287e5297e4ca27e759c5ddeba.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `37cfdc19a13e9e562f8655a2d981686d.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `39b2baa369670e84403a069f0f131c5a.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `41ac235862cad8a46f379f88ccfeec13.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `49b4dc0521311a18c81eb707ac343120.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `4a641bb8c2c2b556f91e0d90c11a1c51.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `52170b5714b8492e937a31362507d631.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `57cde9242aefb9083d37597417c87d14.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `5bb9a55712cd75fc35975a58e8b7e28b.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `5bea81184744c82d757d3a3e02c2cd92.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `75f298e94f6a2a793b8056eb3ffaf3c4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `847ab994d33698cc622048d6281bc8f7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `862ad6997100aff090ca640216055288.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `92d1ad68f2507dfcf3503efd044dfc80.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `9fa21bb0629f402f9e5b14f58fc2c0b6.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `bf652fc0faf10a27c9b8d68fd7d8e332.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `d2421e6ea5b8857214434e92cbbb4ec4.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `e00570f447eebfec862835bb18d9f462.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: andere Kennung: `e24490b7f84793ed328baf45c4c4a9a7.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`
  - roh fremd: andere Kennung: `eefa4cdaa9a33766ebff53e1f3e049fa.probeeq.315912.d387a78151edaa16.c174f858b4d84d46b9a51fdc2f37822c.json`
  - roh fremd: Rolle gen ohne Erwartung: `f07fd8e8ba118b089ba1e62230023060.gen.315912.2f869d2ee2d9099f.9e3fafed076d4fba9ec60650010532f1.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S3 | gen | getrennt | 16/16 | `26ad647b` | Materialausschnitt null: zaehlt nie | true | 0.0 | 0.0 | null | null | null | null | 0 | 1443 | 8008 | 0 | 0 | 3645 | 4496 | 0 | null | null | false | [1872574, 2010784) v = 0 | 42460 | 42509 |
| S3 | gen | getrennt | 16/16 | `847ab994` | gezaehlt | true | 1.0 | 1.0 | 0.745826005935669 | null | -16.252164840698242 | -16.252670288085938 | 5 | 1448 | 8019 | 2 | 2 | 3654 | 4528 | 0 | 1872574 | 14079 | true | [1872574, 2010784) v = 0 | 42509 | 42879 |
| S3 | gen | getrennt | 16/16 | `5bea8118` | gezaehlt | false | 1.0 | null | null | null | -15.007675170898438 | -15.007857322692871 | null | 1468 | 8039 | 3 | 3 | 3664 | 4528 | 0 | 1913534 | 18400 | true | [1872574, 2010784) v = 0 | 43531 | 43902 |
| S3 | gen | getrennt | 16/16 | `37cfdc19` | gezaehlt | false | 1.0 | null | null | null | -25.128005981445312 | -25.134159088134766 | null | 1488 | 8059 | 5 | 5 | 3674 | 4529 | 0 | 1968830 | 8189 | true | [1872574, 2010784) v = 0 | 44554 | 44924 |
| S3 | gen | getrennt | 16/16 | `5bb9a557` | Ausschnitt [0, 2363) nicht ganz in [1872574, 2010784) | false | null | null | null | null | -15.642861366271973 | -15.643216133117676 | null | 1508 | 8079 | 2 | 2 | 3683 | 2363 | 0 | 0 | 2363 | true | [1872574, 2010784) v = 0 | 45571 | 196 |
| S3 | probeeq | getrennt | 16/16 | `bf652fc0` | gezaehlt | false | 1.0 | null | null | null | -15.911407470703125 | -15.9158935546875 | null | 1510 | 8112 | 3 | 3 | 3703 | 4528 | 0 | 1903294 | 8559 | true | [1872574, 2010784) v = 0 | 42460 | 43438 |
| S3 | probeeq | getrennt | 16/16 | `eefa4cda` | gezaehlt | false | 1.0 | null | null | null | -35.821502685546875 | -35.835269927978516 | null | 1550 | 8152 | 6 | 6 | 3722 | 4528 | 0 | 1979070 | 18817 | true | [1872574, 2010784) v = 0 | 43438 | 45433 |
| S3 | probeeq | getrennt | 16/16 | `f07fd8e8` | Ausschnitt [65536, 73238) nicht ganz in [1872574, 2010784) | false | 1.0 | null | null | null | -14.135613441467285 | -14.137906074523926 | null | 1589 | 8191 | 3 | 3 | 3742 | 4528 | 0 | 65536 | 7702 | true | [1872574, 2010784) v = 0 | 45433 | 1732 |

- Stelle S3 gen (durchlauf getrennt, kombinationen 16/16): {"antworten": 3, "d_frames_gebaut": 20, "d_summe_fenster_aktiv": 40, "d_summe_fenster_gesamt": 40, "erster_ausschnitt": [1872574, 1886653], "letzter_ausschnitt": [1968830, 1977019]}
- Stelle S3 probeeq (durchlauf getrennt, kombinationen 16/16): {"antworten": 2, "d_frames_gebaut": 19, "d_summe_fenster_aktiv": 40, "d_summe_fenster_gesamt": 40, "erster_ausschnitt": [1903294, 1911853], "letzter_ausschnitt": [1979070, 1997887]}

