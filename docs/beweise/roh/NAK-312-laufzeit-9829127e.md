# Laufzeit-Arm NAK-312 - VORAUSSETZUNG

Zeit: 2026-09-23 11:30:38 | Basis: d72f9d84 | HEAD: 9829127e | Repo: C:\Users\phili\Projekte\Nakama | Projekt: Nakama-Diagnose.flp
Kopfzeile: LAUFZEIT NAK-312 9829127e VORAUSSETZUNG installation=installiert; pruefen Exit 0, 3 Artefakt(e) aktuell szenarien=6 verfehlt=1 [bereitschaft.json=0,fenster.json=0,nulltest-host.json=5,schleife-dauerlauf.json=0,snapshot-runde01.json=0,u40-aktivitaetsgate.json=0] fl=Producer Edition v26.1.4 [build 5589]

## Protokoll

```text
[2026-09-23 11:20:35] Laufzeit-Arm NAK-312 auf 9829127e (Basis d72f9d84), Repo C:\Users\phili\Projekte\Nakama
[2026-09-23 11:20:35] Lohnt es? True - Produktpfade im Diff: 8 Datei(en), z. B. eq-copilot/plugin/dsp/DspKern.cpp
[2026-09-23 11:20:35] MCP-Stand: Revision 15b3b1a133ad37aa75e63e2d98a7aac0b74a62ae, Zweig evenacadia-local (Pin evenacadia-local), sauber, 10 gepinnte Datei(en) gleich, uv.lock SHA-256 356B1391E1F32DF927D0CF5B9D6F7FE19240241F1C768EC3E1241DED68EC0AD7
[2026-09-23 11:20:36] Manifest-Hashes nachziehen (--hashen)
[2026-09-23 11:20:36]   hashen: Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
[2026-09-23 11:20:36]   hashen:                  Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[2026-09-23 11:20:36]   hashen: [0] Struktur vor dem mutierenden Schritt --hashen
[2026-09-23 11:20:36]   hashen:   ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
[2026-09-23 11:20:36]   hashen:   ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
[2026-09-23 11:20:36]   hashen:   ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
[2026-09-23 11:20:36]   hashen:   ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
[2026-09-23 11:20:36]   hashen:   ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
[2026-09-23 11:20:36]   hashen:   ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
[2026-09-23 11:20:36]   hashen:   ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
[2026-09-23 11:20:36]   hashen:   ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
[2026-09-23 11:20:36]   hashen:   ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
[2026-09-23 11:20:36]   hashen:   ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
[2026-09-23 11:20:36]   hashen:   ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
[2026-09-23 11:20:36]   hashen:   ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
[2026-09-23 11:20:36]   hashen:   ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
[2026-09-23 11:20:36]   hashen:   ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
[2026-09-23 11:20:36]   hashen:   ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)
[2026-09-23 11:20:36]   hashen: [hashen] Artefakte gegen den gebauten Stand festschreiben
[2026-09-23 11:20:36]   hashen:   ok      main = 202D0B37751A15C3D1254AFAC0C88F93A01D740FF86BB59D20941D2E12C34DC4
[2026-09-23 11:20:36]   hashen:   ok      active-probe = A0DFA2F32D8AE95E682AD1045AA5D3E44DAC39F9C59FD683952514A3D6755D51
[2026-09-23 11:20:36]   hashen:   ok      eqcop-broker.exe = FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D
[2026-09-23 11:20:36]   hashen: 
[2026-09-23 11:20:36]   hashen: geschrieben: eq-copilot/install/nakama-installer-v1.json
[2026-09-23 11:20:36]   hashen: 
[2026-09-23 11:20:36]   hashen: [hashen] Startbindung gegen das geschriebene Manifest ([4c], [4c+])
[2026-09-23 11:20:36]   hashen: 
[2026-09-23 11:20:36]   hashen: [4c] Manifestgebundene Broker-Startwerte
[2026-09-23 11:20:36]   hashen:   ok      generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests  [ist=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', 'FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D', '') soll=('C:/Program Files/evenacadia/Nakama/eqcop-broker.exe', 'FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D', '')]
[2026-09-23 11:20:36]   hashen:   ok      Installer-Manifest ist CMake-Configure-Dependency
[2026-09-23 11:20:36]   hashen: 
[2026-09-23 11:20:36]   hashen: [4c+] Broker-Pin im Header gegen die gebaute Release-Broker-Datei (hart)
[2026-09-23 11:20:36]   hashen:   ok      Broker-Pin im erzeugten Header entspricht dem SHA-256 der gebauten Release-Broker-Datei  [Header FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D | Datei FBE413395BC610C0211E0845D7104196023BE607F863EF38A2693B1C8C5F720D]
[2026-09-23 11:20:36] Aufgabe \Nakama\installieren starten
[2026-09-23 11:20:39]   installieren: Exit 0, Zustand Ready, Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3] |   ok      Nakama Probeeq installiert  [C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3] |   ok      eqcop-broker.exe installiert  [C:\Program Files\evenacadia\Nakama\eqcop-broker.exe] | INSTALLATION OK
[2026-09-23 11:20:41]   pruefen: Exit 0, Zustand Ready, Log: Installierter Stand gegen das Manifest: |   aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3 |   aktuell          C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3 |   aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe
[2026-09-23 11:20:41] Controller-Skript: SHA-256 Repo 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB, installiert 6640525B954B373E01C7D9C5C587D8C2AA4FA090E3CB0E176D5BD7B44F8802CB (C:\Users\phili\OneDrive\Dokumente\Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\device_FLStudioMCP.py), script_version 2026-09-18
[2026-09-23 11:20:41] Diagnoseprojekt: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
[2026-09-23 11:20:41] Referenzprojekt Nakama-Diagnose-Verarbeitung.flp: fehlt neben dem Diagnoseprojekt (C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\fl\Nakama-Diagnose-Verarbeitung.flp; Karte U43), nicht kopiert
[2026-09-23 11:20:41] Referenzprojekt Nakama-Diagnose-Referenz.flp: SHA-256 Repo 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Arbeitskopie 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D
[2026-09-23 11:20:41] Briefkasten: C:\Users\phili\AppData\Local\evenacadia\nakama\diagnose bereit, 34 Antwortdatei(en) frueherer Laeufe entfernt
[2026-09-23 11:20:41] Renderfolge vor dem FL-Start: 1 Nakama-Diagnose.flp (Auslieferungszustand), 2 Nakama-Diagnose-Verarbeitung.flp (Referenzprojekt fehlt, kein Render), 3 Nakama-Diagnose-Referenz.flp (Referenzprojekt)
[2026-09-23 11:20:41] Besitz: PID 53084 (render) eingetragen
[2026-09-23 11:20:41] Render gestartet: PID 53084 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe /R /Ewav /O"C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render" "C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp")
[2026-09-23 11:20:46] Besitz: PID 53084 ausgetragen (Render beendet)
[2026-09-23 11:20:46] Render: Exit 0, Dauer 5,2 s, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\Nakama-Diagnose.wav, SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A, Grund 
[2026-09-23 11:20:46] Referenzrender Nakama-Diagnose-Verarbeitung.flp: kein Render - Referenzprojekt fehlt (Karte U43, K-286-1); Renderstatus C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Verarbeitung.flp\render.json
[2026-09-23 11:20:46] Besitz: PID 183960 (render) eingetragen
[2026-09-23 11:20:46] Referenzrender Nakama-Diagnose-Referenz.flp gestartet: PID 183960 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe /R /Ewav /O"C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp" "C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose-Referenz.flp")
[2026-09-23 11:20:49] Besitz: PID 183960 ausgetragen (Render beendet)
[2026-09-23 11:20:50] Referenzrender Nakama-Diagnose-Referenz.flp: Exit 0, Dauer 3,1 s, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp\Nakama-Diagnose-Referenz.wav, SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A, SHA-256 Projekt 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Grund 
[2026-09-23 11:20:50] loopMIDI laeuft
[2026-09-23 11:20:50] Besitz: PID 218568 (fl) eingetragen
[2026-09-23 11:20:50] FL gestartet: PID 218568 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-23 11:20:53] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-23T11:20:52", "program_title": "FL Studio 2026" }
[2026-09-23 11:21:01] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 218568, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-23 11:21:05] Szenario bereitschaft.json: Exit 0
[2026-09-23 11:21:07] Szenario fenster.json: Exit 0
[2026-09-23 11:21:09] Szenario nulltest-host.json: Exit 5
[2026-09-23 11:21:09] Szenario schleife-dauerlauf.json: frischer_start - Diagnose-FL neu starten
[2026-09-23 11:21:09] Diagnose-FL beenden: PID 218568 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-23 11:21:11] Besitz: PID 218568 ausgetragen (Ende bestaetigt)
[2026-09-23 11:21:11] Besitz: PID 225312 (fl) eingetragen
[2026-09-23 11:21:11] FL gestartet: PID 225312 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-23 11:21:14] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-23T11:21:13", "program_title": "FL Studio 2026" }
[2026-09-23 11:21:22] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 225312, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-23 11:25:26] Szenario schleife-dauerlauf.json: Exit 0
[2026-09-23 11:25:26] Szenario snapshot-runde01.json: frischer_start - Diagnose-FL neu starten
[2026-09-23 11:25:26] Diagnose-FL beenden: PID 225312 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-23 11:25:27] Besitz: PID 225312 ausgetragen (Ende bestaetigt)
[2026-09-23 11:25:27] Besitz: PID 242392 (fl) eingetragen
[2026-09-23 11:25:27] FL gestartet: PID 242392 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-23 11:25:30] Boot-Marke nach 3 s: { "script_version": "2026-09-18", "loaded_at": "2026-09-23T11:25:29", "program_title": "FL Studio 2026" }
[2026-09-23 11:25:38] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 242392, Module fl_studio_mcp, fl_studio_mcp.utils, fl_studio_mcp.utils.connection, fl_studio_mcp.utils.fl_paths, fl_studio_mcp.utils.midi_connection
[2026-09-23 11:29:59] Szenario snapshot-runde01.json: Exit 0
[2026-09-23 11:30:37] Szenario u40-aktivitaetsgate.json: Exit 0
[2026-09-23 11:30:37] Diagnose-FL beenden: PID 242392 'Nakama-Diagnose.flp - FL Studio 2026' (eigen, Projekt wird nie gespeichert)
[2026-09-23 11:30:38] Besitz: PID 242392 ausgetragen (Ende bestaetigt)
[2026-09-23 11:30:38] Diagnoseprojekt am Ende: SHA-256 Repo B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229, Arbeitskopie B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229
[2026-09-23 11:30:38] Referenzprojekt Nakama-Diagnose-Referenz.flp am Ende: SHA-256 Repo 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Arbeitskopie 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D
```
## Szenarien


<!-- szenario.py 2026-09-23T09:21:02+00:00 -->
## Szenario `bereitschaft` — FL antwortet, das Diagnoseprojekt mit beiden Plugins ist offen, Ton fließt durch Sondenspur und Master

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=367565c90df701177ce0264622a78651 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `request_id=3841f53b339500bb2d7dff7055990e9f {"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `request_id=6acdf6fc37062300bd2def352c76bb13 {"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `request_id=ebdcc3ee0e6ee603eef517f7dfec18b0 {"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok |
| 5 | `transport.getLength` | `{}` | `request_id=f85d8b96de74884e6d77857396505ecb {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok |
| 6 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=b165bfdcc90e1a084b9bc638bb827b05 {"mode": "song", "success": true}` | ok |
| 7 | `transport.stop` | `{}` | `request_id=93c127fbdab8b7ab19efd06f6640706f {"stopped": true, "success": true}` | ok |
| 8 | `transport.setPosition` | `{"mode": 1, "position": 2}` | `request_id=19b99d157e29c9180ab106572327879a {"mode": 1, "requested_position": 2, "success": true}` | ok |
| 9 | `transport.start` | `{}` | `request_id=cf9411ba982becd796fb6154e6c46422 {"is_playing": true, "success": true}` | ok |
| 10 | warte | 3 s | — | — |
| 11 | `mixer.getPeaks` | `{"track": 1}` | `request_id=9ad421eed1c6df27c3cc0b6372a3d9b3 {"name": "Insert 1", "peak_left": 0.48343729972839355, "peak_max": 0.48343729972839355, "peak_right": 0.4635325074195862, "success": true, "track": 1}` | ok |
| 12 | `mixer.getPeaks` | `{"track": 0}` | `request_id=764e0ad2b4dff2520df90f41da0d2b09 {"name": "Master", "peak_left": 0.4983249604701996, "peak_max": 0.4983249604701996, "peak_right": 0.4755004644393921, "success": true, "track": 0}` | ok |
| 13 | `transport.getStatus` | `{}` | `request_id=c09875d41586aba459ebb2977742e850 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "3:15:20", "success": true}` | ok |
| 14 | `transport.stop` | `{}` | `request_id=cce6b3399d651a8401eaa9989a573928 {"stopped": true, "success": true}` | ok |

**Ergebnis:** 14 von 14 Schritten bestanden.


<!-- szenario.py 2026-09-23T09:21:05+00:00 -->
## Szenario `fenster` — Das FL-Hauptfenster und das Fenster von Nakama Probeeq werden ohne Vordergrundwechsel als Bild erfasst; ein einfarbiges Bild ist verfehlt

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=99ff2d6276ba1b8fb1025533380defea {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `lokal.fenster` | `{"ziel": "fl"}` | {"breite": 1920, "dauer_s": 0.125, "einfarbig": false, "fenster": "Nakama-Diagnose.flp - FL Studio 2026", "hoehe": 1032, "klasse": "TFruityLoopsMainForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\9829127e-fl-20260923-112105.png", "sha256": "62F14BD915C0E1C0303C6BE35CABA2C80CB520741A304DC1C019E6108E1C91D1"} | ok (Details unten) |
| 3 | `mixer.focusEditor` | `{"slot_index": 0, "track": 1}` | `request_id=e90d7207ad68cc39946fa7ff88a90922 {"focused": true, "plugin": "Nakama Probeeq", "slot_index": 0, "success": true, "track": 1}` | ok |
| 4 | warte | 1 s | — | — |
| 5 | `lokal.fenster` | `{"plugin": "Nakama Probeeq", "ziel": "plugin"}` | {"breite": 571, "dauer_s": 0.109, "einfarbig": false, "fenster": "Nakama Probeeq (Insert 1)", "hoehe": 487, "klasse": "TPluginForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\9829127e-plugin-20260923-112106.png", "sha256": "465D90CF6A8F7223E84595DF46F8A3F9F28369184DF21C7E1DACD86CFBF6E7A3"} | ok (Details unten) |

**Ergebnis:** 5 von 5 Schritten bestanden.


### Schritt 2 `lokal.fenster`

- Antwort {"breite": 1920, "dauer_s": 0.125, "einfarbig": false, "fenster": "Nakama-Diagnose.flp - FL Studio 2026", "hoehe": 1032, "klasse": "TFruityLoopsMainForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\9829127e-fl-20260923-112105.png", "sha256": "62F14BD915C0E1C0303C6BE35CABA2C80CB520741A304DC1C019E6108E1C91D1"}

### Schritt 5 `lokal.fenster`

- Antwort {"breite": 571, "dauer_s": 0.109, "einfarbig": false, "fenster": "Nakama Probeeq (Insert 1)", "hoehe": 487, "klasse": "TPluginForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\9829127e-plugin-20260923-112106.png", "sha256": "465D90CF6A8F7223E84595DF46F8A3F9F28369184DF21C7E1DACD86CFBF6E7A3"}

<!-- szenario.py 2026-09-23T09:21:07+00:00 -->
## Szenario `nulltest-host` — Der Render des Diagnoseprojekts im Auslieferungszustand ist bitidentisch zur Quelle (Weg R1); Render mit Verarbeitung und ohne Slots brauchen Karte U43

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=ac885800a03c732fd06d10d55342a00d {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `request_id=aa86e5e1c1f86b3019af610684252fe5 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `request_id=7c5fe6ab738f1d47e3846c6d574e5bda {"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok · roh: volume=0.78125; volume_db=-5.176800727844238 |
| 4 | `mixer.getTrackInfo` | `{"track": 1}` | `request_id=0bb56c63a9cf237cc9beb558dc41807c {"color": "-0xbdbdc0", "index": 1, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Insert 1", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume=0.7999999998137355; volume_db=0.0 |
| 5 | `mixer.getTrackInfo` | `{"track": 0}` | `request_id=c04787be9494e7b370012c132edbb338 {"color": "-0xbdbdc0", "index": 0, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Master", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume=0.7999999998137355; volume_db=0.0 |
| 6 | `mixer.getRouting` | `{"track": 1}` | `request_id=14814459926834bba75a41cb1d97a5b3 {"routes": [{"dest": 0, "dest_name": "Master", "level": 0.8, "source": 1, "source_name": "Insert 1"}], "scanned": 1, "success": true, "track_count": 18}` | ok · roh: routes.*.dest=[0]; routes.*.level=[0.8] |
| 7 | `lokal.nulltest` | `{"vergleich": "auslieferung"}` | BITIDENTISCH v=0 g=1.0 Abweichungen=0 | ok (Details unten) |
| 8 | `lokal.nulltest` | `{"projekt": "Nakama-Diagnose-Verarbeitung.flp", "vergleich": "verarbeitung_ein"}` | VORAUSSETZUNG verarbeitung_ein: Referenzprojekt fehlt (Karte U43, K-286-1) | VORAUSSETZUNG (Details unten) |
| 9 | `lokal.nulltest` | `{"projekt": "Nakama-Diagnose-Referenz.flp", "vergleich": "ohne_slots"}` | GEMESSEN ohne_slots: GLEICH v=0 g_db=0.0 Abweichungen=0 | ok (Details unten) |

**Ergebnis:** 8 von 9 Schritten bestanden.


### Schritt 7 `lokal.nulltest`

- `nulltest.py` Exit 0, Quelle `C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav`, Songlaenge 45696 ms
  - NULLTEST Urteil BITIDENTISCH · Exit 0 · N 2015193 · v 0 · abweichungen 0 · nullvorzeichen 0 · g 1.0 · g_db 0.0 · ueberlappend 2015193 Frames [0, 2015193) · sha256_render_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · sha256_quelle_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · Stichprobenspitzen Datei {'render': [0.3984098434448242, 0.4039571285247803], 'quelle': [0.3984098434448242, 0.4039571285247803]} · Stichprobenspitzen Bereich {'render': [0.3984098434448242, 0.4039571285247803], 'quelle': [0.3984098434448242, 0.4039571285247803]} · Render {'art': 'PCM', 'bits': 24, 'kanaele': 2, 'rate': 44100, 'frames': 2015409} SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A
- ergebnis.json: `{"N": 2015193, "abweichungen": 0, "erste": null, "erzeugt_utc": "2026-09-23T09:21:09+00:00", "exit": 0, "format": "nakama.laufzeit.nulltest.v1", "g": 1.0, "g_db": 0.0, "hinweis": "Offline-Latenz des Plugins 0 (Kanon A1/A16); Clip-Lage oder Latenz trennt nur Weg R2 (Karte U43)", "letzte": null, "nullvorzeichen": 0, "nullvorzeichen_erste": null, "nullvorzeichen_letzte": null, "quelle": {"format": {"art": "PCM", "bits": 24, "frames": 5470096, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\Projekte\\Nakama\\eq-copilot\\kalibration\\Testtrack.wav", "sha256_datei": "D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF"}, "render": {"format": {"art": "PCM", "bits": 24, "frames": 2015409, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "sha256_datei": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A"}, "renderstatus": {"aufruf": "C:\\Program Files\\Image-Line\\FL Studio 2026\\FL64.exe /R /Ewav /O\"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\" \"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose.flp\"", "datei": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "dauer_s": 5.2, "exit": 0, "fenstertitel": "", "format": "nakama.laufzeit.render.v1", "grund": null, "head": "9829127e", "pid": 53084, "projekt": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose.flp", "sha256": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A", "sha256_projekt": "B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229", "zeit": "2026-09-23 11:20:46"}, "sha256_quelle_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "sha256_render_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "songlaenge_ms": 45696, "spitzen_bereich": {"quelle": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "spitzen_datei": {"quelle": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "ueberlappend": {"bis": 2015193, "frames": 2015193, "von": 0}, "urteil": "BITIDENTISCH", "v": 0}`

### Schritt 8 `lokal.nulltest`

- Renderstatus `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Verarbeitung.flp\render.json`: projekt `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose-Verarbeitung.flp`, grund "Referenzprojekt fehlt (Karte U43, K-286-1)" - kein Render dieses Zustands (M-64), nie still

### Schritt 9 `lokal.nulltest`

- Referenzprojekt `Nakama-Diagnose-Referenz.flp`: Renderstatus `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp\render.json`, SHA-256 Projekt 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D, Renderdauer 3.1 s, Render `C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\referenz\Nakama-Diagnose-Referenz.flp\Nakama-Diagnose-Referenz.wav`
- `nulltest.py --vergleich ohne_slots` Exit 0, Songlaenge 45696 ms
  - NULLTEST Vergleich ohne_slots · Urteil GEMESSEN · Exit 0 · Befund GLEICH · N 2015193 · v 0 · abweichungen 0 · g 1.0 · g_db 0.0 · ueberlappend 2015193 Frames [0, 2015193) · sha256_render_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · sha256_auslieferung_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · Stichprobenspitzen Bereich {'render': [0.3984098434448242, 0.4039571285247803], 'auslieferung': [0.3984098434448242, 0.4039571285247803]} · Render {'art': 'PCM', 'bits': 24, 'kanaele': 2, 'rate': 44100, 'frames': 2015409} SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A · Projekt Nakama-Diagnose-Referenz.flp · SHA-256 Projekt 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D · Renderdauer 3.1 s
- ergebnis.json (ohne_slots): `{"N": 2015193, "abweichungen": 0, "auslieferung": {"ergebnis": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ergebnis.json", "render": {"format": {"art": "PCM", "bits": 24, "frames": 2015409, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "sha256_datei": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A"}}, "befund": "GLEICH", "erste": null, "erzeugt_utc": "2026-09-23T09:21:09+00:00", "exit": 0, "format": "nakama.laufzeit.nulltest.referenz.v1", "g": 1.0, "g_db": 0.0, "letzte": null, "projekt": "Nakama-Diagnose-Referenz.flp", "render": {"format": {"art": "PCM", "bits": 24, "frames": 2015409, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\referenz\\Nakama-Diagnose-Referenz.flp\\Nakama-Diagnose-Referenz.wav", "sha256_datei": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A"}, "renderdauer_s": 3.1, "renderstatus": {"aufruf": "C:\\Program Files\\Image-Line\\FL Studio 2026\\FL64.exe /R /Ewav /O\"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\referenz\\Nakama-Diagnose-Referenz.flp\" \"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose-Referenz.flp\"", "datei": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\referenz\\Nakama-Diagnose-Referenz.flp\\Nakama-Diagnose-Referenz.wav", "dauer_s": 3.1, "exit": 0, "fenstertitel": "", "format": "nakama.laufzeit.render.v1", "grund": null, "head": "9829127e", "pid": 183960, "projekt": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose-Referenz.flp", "sha256": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A", "sha256_projekt": "4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D", "zeit": "2026-09-23 11:20:50"}, "sha256_auslieferung_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "sha256_projekt": "4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D", "sha256_render_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "songlaenge_ms": 45696, "spitzen_bereich": {"auslieferung": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "ueberlappend": {"bis": 2015193, "frames": 2015193, "von": 0}, "urteil": "GEMESSEN", "v": 0, "vergleich": "ohne_slots"}`

<!-- szenario.py 2026-09-23T09:21:22+00:00 -->
## Szenario `schleife-dauerlauf` — Der Host lebt und antwortet ueber mindestens fuenf Songumlaeufe Echtzeit-Wiedergabe des Diagnoseprojekts; nach jedem Umlauf antworten FL und beide Rollen im Briefkasten

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=009a8840c20af1e082a6ce04868688e1 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `request_id=1c2a134f3d15d9b7725d91f9b5d15945 {"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `request_id=d6107159057ea25f8a7f192d33bf1051 {"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `transport.getLength` | `{}` | `request_id=cb948c760b7fa8cd6f71971a1fd9be73 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 5 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=508e6a6cfd3f0add5760021932826fe4 {"mode": "song", "success": true}` | ok |
| 6 | `transport.stop` | `{}` | `request_id=973eb5173e1999731d25f6eef134c01e {"stopped": true, "success": true}` | ok |
| 7 | `transport.setPosition` | `{"mode": 0, "position": 0}` | `request_id=221ad5a2781509d1531ef2803577c079 {"mode": 0, "requested_position": 0, "success": true}` | ok |
| 8 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 9 | `transport.start` | `{}` | `request_id=3984e865b80225c65107d4d67381c472 {"is_playing": true, "success": true}` | ok |
| 10 | warte | 46 s | — | — |
| 11 | `transport.getStatus` | `{}` | `request_id=da976d7bd7e057f40c85b89af789eca7 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "1:04:00", "success": true}` | ok · roh: position="1:04:00" |
| 12 | `transport.getPosition` | `{}` | `request_id=0b2a7364bcddfa8ba84a6235fd6b5017 {"hint": "1:04:00", "ms": 321, "seconds": 0, "success": true}` | ok · roh: ms=321 |
| 13 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 14 | warte | 46 s | — | — |
| 15 | `transport.getStatus` | `{}` | `request_id=e9398a9e75ff8e24a4f27041d7f85969 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "2:11:09", "success": true}` | ok · roh: position="2:11:09" |
| 16 | `transport.getPosition` | `{}` | `request_id=bc42b2b5724dfd0b578f297be81504e9 {"hint": "2:11:19", "ms": 2871, "seconds": 3, "success": true}` | ok · roh: ms=2871 |
| 17 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 18 | warte | 46 s | — | — |
| 19 | `transport.getStatus` | `{}` | `request_id=f378c8c5fb0b07fadaf081a92e7f5581 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "4:03:04", "success": true}` | ok · roh: position="4:03:04" |
| 20 | `transport.getPosition` | `{}` | `request_id=94553eac51d6fe9a477903f26431d9de {"hint": "4:03:14", "ms": 5420, "seconds": 5, "success": true}` | ok · roh: ms=5420 |
| 21 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 22 | warte | 46 s | — | — |
| 23 | `transport.getStatus` | `{}` | `request_id=d7ebb8397352ab23de7519f828c9869a {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "5:10:23", "success": true}` | ok · roh: position="5:10:23" |
| 24 | `transport.getPosition` | `{}` | `request_id=6f453ba2d36dfcdf8c3f98e8fc20e512 {"hint": "5:11:10", "ms": 7973, "seconds": 8, "success": true}` | ok · roh: ms=7973 |
| 25 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 26 | warte | 46 s | — | — |
| 27 | `transport.getStatus` | `{}` | `request_id=cf08bb7e078b84c5b18b0c26e3ef0e84 {"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "7:02:18", "success": true}` | ok · roh: position="7:02:18" |
| 28 | `transport.getPosition` | `{}` | `request_id=3439be036c79bc754dd19db5f5e51ca3 {"hint": "7:02:18", "ms": 10473, "seconds": 10, "success": true}` | ok · roh: ms=10473 |
| 29 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 30 | `transport.stop` | `{}` | `request_id=f109e6098ef6ceeed26ba61d5057cb6c {"stopped": true, "success": true}` | ok |

**Ergebnis:** 30 von 30 Schritten bestanden.


### Schritt 8 `lokal.briefkasten`

- Anfrage `e3c3559d1c00ee4ede28e81d90997e65`, erwartet {"gen": [225312], "probeeq": [225312]}, Positionsklammer None → None ms
  - gen: `e3c3559d1c00ee4ede28e81d90997e65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json` (24048 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.84 s nach dem Schreiben)
  - probeeq: `e3c3559d1c00ee4ede28e81d90997e65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json` (15917 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.86 s nach dem Schreiben)
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 103
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 113

### Schritt 13 `lokal.briefkasten`

- Anfrage `96ffe1fb8d8e6e57bd5512cdd7d99db1`, erwartet {"gen": [225312], "probeeq": [225312]}, Positionsklammer None → None ms
  - gen: `96ffe1fb8d8e6e57bd5512cdd7d99db1.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json` (23678 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.61 s nach dem Schreiben)
  - probeeq: `96ffe1fb8d8e6e57bd5512cdd7d99db1.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json` (23681 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.61 s nach dem Schreiben)
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 571
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 581

### Schritt 17 `lokal.briefkasten`

- Anfrage `3d5bb5898fb7ceb54abec8ce2c61d61c`, erwartet {"gen": [225312], "probeeq": [225312]}, Positionsklammer None → None ms
  - gen: `3d5bb5898fb7ceb54abec8ce2c61d61c.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json` (23694 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.36 s nach dem Schreiben)
  - probeeq: `3d5bb5898fb7ceb54abec8ce2c61d61c.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json` (23648 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.39 s nach dem Schreiben)
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1039
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1049

### Schritt 21 `lokal.briefkasten`

- Anfrage `c40eb02fa1cab2f82adee942754b9c65`, erwartet {"gen": [225312], "probeeq": [225312]}, Positionsklammer None → None ms
  - gen: `c40eb02fa1cab2f82adee942754b9c65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json` (23702 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.17 s nach dem Schreiben)
  - probeeq: `c40eb02fa1cab2f82adee942754b9c65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json` (23664 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.19 s nach dem Schreiben)
  - roh fremd: andere Kennung: `3d5bb5898fb7ceb54abec8ce2c61d61c.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `3d5bb5898fb7ceb54abec8ce2c61d61c.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1508
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1518

### Schritt 25 `lokal.briefkasten`

- Anfrage `37d49a4c8f0ac561b55b786cc449d9a1`, erwartet {"gen": [225312], "probeeq": [225312]}, Positionsklammer None → None ms
  - gen: `37d49a4c8f0ac561b55b786cc449d9a1.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json` (23736 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.95 s nach dem Schreiben)
  - probeeq: `37d49a4c8f0ac561b55b786cc449d9a1.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json` (23673 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `3d5bb5898fb7ceb54abec8ce2c61d61c.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `3d5bb5898fb7ceb54abec8ce2c61d61c.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `c40eb02fa1cab2f82adee942754b9c65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `c40eb02fa1cab2f82adee942754b9c65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 1986
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 1996

### Schritt 29 `lokal.briefkasten`

- Anfrage `7931c6c49f248ee399ed8f040a835ed0`, erwartet {"gen": [225312], "probeeq": [225312]}, Positionsklammer None → None ms
  - gen: `7931c6c49f248ee399ed8f040a835ed0.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json` (23759 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.75 s nach dem Schreiben)
  - probeeq: `7931c6c49f248ee399ed8f040a835ed0.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json` (23664 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.75 s nach dem Schreiben)
  - roh fremd: andere Kennung: `37d49a4c8f0ac561b55b786cc449d9a1.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `37d49a4c8f0ac561b55b786cc449d9a1.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `3d5bb5898fb7ceb54abec8ce2c61d61c.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `3d5bb5898fb7ceb54abec8ce2c61d61c.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `96ffe1fb8d8e6e57bd5512cdd7d99db1.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `c40eb02fa1cab2f82adee942754b9c65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `c40eb02fa1cab2f82adee942754b9c65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.gen.225312.2f869d2ee2d9099f.fb228652cb224f22a0c00e0885646de7.json`
  - roh fremd: andere Kennung: `e3c3559d1c00ee4ede28e81d90997e65.probeeq.225312.d387a78151edaa16.46593bff475b4c90a0513480368339a9.json`
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 2454
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 2464

<!-- szenario.py 2026-09-23T09:25:39+00:00 -->
## Szenario `snapshot-runde01` — Snapshot beider Rollen ueber den Briefkasten innerhalb eines Umlaufs gegen den Referenzausschnitt der Quelle; die Runde-01-Anker stehen nur als Rohvergleich daneben

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=a854a7d7b54b7dcd53c9f1b6caa24348 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `request_id=df3a168998fbd27051e0b40720120932 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `request_id=22c39f8d4d8867fcb6d1dd667e814a8b {"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok · roh: volume=0.78125; volume_db=-5.176800727844238 |
| 4 | `mixer.getTrackInfo` | `{"track": 1}` | `request_id=57d6e18068d9c31015a0cabca65ea1bc {"color": "-0xbdbdc0", "index": 1, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Insert 1", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume_db=0.0 |
| 5 | `mixer.getTrackInfo` | `{"track": 0}` | `request_id=25b90f0b07a4ce77098319de8b80a038 {"color": "-0xbdbdc0", "index": 0, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Master", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume_db=0.0 |
| 6 | `mixer.getRouting` | `{"track": 1}` | `request_id=2c0b5635d83dc740f79942b0ff7d6575 {"routes": [{"dest": 0, "dest_name": "Master", "level": 0.8, "source": 1, "source_name": "Insert 1"}], "scanned": 1, "success": true, "track_count": 18}` | ok · roh: routes.*.level=[0.8] |
| 7 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=80b415441cb995f1d395b07ea4342aab {"mode": "song", "success": true}` | ok |
| 8 | `transport.stop` | `{}` | `request_id=8ca15151712ec2d86ab24be93a4556e8 {"stopped": true, "success": true}` | ok |
| 9 | `transport.setPosition` | `{"mode": 0, "position": 0}` | `request_id=afbb4bca8c27521c99915813a899f786 {"mode": 0, "requested_position": 0, "success": true}` | ok |
| 10 | `transport.getPosition` | `{}` | `request_id=147dc1449dc941060f400a43b9a1b30d {"hint": "1:01:00", "ms": 0, "seconds": 0, "success": true}` | ok · roh: ms=0; hint="1:01:00" |
| 11 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq", "broker"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 12 | `transport.start` | `{}` | `request_id=5271f6bc4e4652682332798fb2a5274e {"is_playing": true, "success": true}` | ok |
| 13 | `lokal.umlauf` | `{"rollen": ["gen", "probeeq"], "vergleich": {"baender": [{"anker": "schwer", "delta_k": true, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "plus_s_lufs": true, "rechnung": "F-28 LUFS integriert: R_LUFS + Delta_K +- (0,07 LU + S_LUFS) ueber den Referenzausschnitt des schweren Ankers n_S; 0,07 LU KALIBRIER-PROTOKOLL.md:71", "referenz": "lufs", "rollen": ["gen", "probeeq"], "toleranz…` | gemessen: gen `3564d357`, probeeq `3564d357` | ok (Details unten) |

**Ergebnis:** 13 von 13 Schritten bestanden.


### Schritt 11 `lokal.briefkasten`

- Anfrage `1ce0fd13f4554957283ec4b864832087`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer None → None ms
  - broker: Broker laeuft nicht - keine Erwartung an die Rolle broker
  - gen: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (24047 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.75 s nach dem Schreiben)
  - probeeq: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (15912 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.78 s nach dem Schreiben)
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 103
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 114

### Schritt 13 `lokal.umlauf`

- Nulltesturteil und Kettenverschiebung: {"delta_k_db": 0.0, "g": 1.0, "urteil": "BITIDENTISCH", "v": 0}

| # | Kennung | p_vor ms | p_nach ms | gen: zustand · aktiv s · gesamt s · E · schwer s | probeeq: zustand · aktiv s · gesamt s · E · schwer s |
|---|---|---|---|---|---|
| 1 | `3209b422` | 45 | 1580 | "sammelt" · 0.4 · 0.495532879818594 · 21853 · 0.263401360544218 | "sammelt" · 0.5 · 0.584829931972789 · 25791 · 0.584829931972789 |
| 2 | `629f7842` | 1580 | 3576 | "sammelt" · 2.4 · 2.491065759637188 · 109856 · 2.397324263038549 | "sammelt" · 2.5 · 2.584829931972789 · 113991 · 2.584829931972789 |
| 3 | `e516c24e` | 3576 | 5621 | "sammelt" · 4.4 · 4.486598639455782 · 197859 · 4.486598639455782 | "sammelt" · 4.5 · 4.580362811791383 · 201994 · 4.580362811791383 |
| 4 | `cc8d6170` | 5621 | 7616 | "sammelt" · 6.4 · 6.482131519274376 · 285862 · 6.343741496598639 | "sammelt" · 6.5 · 6.593741496598639 · 290784 · 6.593741496598639 |
| 5 | `4b3645a5` | 7616 | 9612 | "sammelt" · 8.4 · 8.482131519274377 · 374062 · 8.482131519274377 | "sammelt" · 8.5 · 8.575895691609977 · 378197 · 8.575895691609977 |
| 6 | `8525324e` | 9612 | 11612 | "sammelt" · 10.4 · 10.477687074829932 · 462066 · 10.339274376417233 | "sammelt" · 10.5 · 10.58482993197279 · 466791 · 10.58482993197279 |
| 7 | `fc8752cb` | 11612 | 13607 | "sammelt" · 12.5 · 12.522312925170068 · 552234 · 12.473219954648526 | "sammelt" · 12.5 · 12.58482993197279 · 554991 · 12.58482993197279 |
| 8 | `cd96ac31` | 13607 | 15603 | "sammelt" · 14.5 · 14.517868480725623 · 640238 · 14.330362811791383 | "sammelt" · 14.600000000000001 · 14.611609977324264 · 644372 · 14.611609977324264 |
| 9 | `d030be03` | 15603 | 17603 | "messbereit" · 16.400000000000002 · 16.46875283446712 · 726272 · 16.424104308390024 | "messbereit" · 16.5 · 16.575895691609976 · 730997 · 16.575895691609976 |
| 10 | `2525ea96` | 17603 | 19598 | "messbereit" · 18.5 · 18.51340136054422 · 816441 · 18.28124716553288 | "messbereit" · 18.5 · 18.575895691609976 · 819197 · 18.575895691609976 |
| 11 | `ce06a6c0` | 19643 | 21594 | "messbereit" · 20.5 · 20.508934240362812 · 904444 · 20.415170068027212 | "messbereit" · 20.6 · 20.60267573696145 · 908578 · 20.60267573696145 |
| 12 | `83027537` | 21594 | 23589 | "messbereit" · 22.5 · 22.504467120181406 · 992447 · 22.504467120181406 | "messbereit" · 22.5 · 22.598208616780045 · 996581 · 22.598208616780045 |
| 13 | `1cb6459a` | 23589 | 25589 | "messbereit" · 24.5 · 24.504467120181406 · 1080647 · 24.361609977324264 | "messbereit" · 24.5 · 24.59374149659864 · 1084584 · 24.59374149659864 |
| 14 | `bb026806` | 25589 | 27585 | "messbereit" · 26.5 · 26.5 · 1168650 · 26.5 | "messbereit" · 26.6 · 26.607142857142858 · 1173375 · 26.607142857142858 |
| 15 | `16b8c9ad` | 27634 | 29580 | "messbereit" · 28.5 · 28.544648526077097 · 1258819 · 28.357142857142858 | "messbereit" · 28.6 · 28.60267573696145 · 1261378 · 28.60267573696145 |
| 16 | `5ed3ec28` | 29629 | 31580 | "messbereit" · 30.5 · 30.54018140589569 · 1346822 · 30.495532879818594 | "messbereit" · 30.6 · 30.60267573696145 · 1349578 · 30.60267573696145 |
| 17 | `86d33685` | 31625 | 33576 | "messbereit" · 32.4 · 32.49106575963719 · 1432856 · 32.35267573696145 | "messbereit" · 32.5 · 32.59820861678005 · 1437581 · 32.59820861678005 |
| 18 | `53fa9b0a` | 33621 | 35621 | "messbereit" · 34.4 · 34.486598639455785 · 1520859 · 34.441972789115646 | "messbereit" · 34.5 · 34.59374149659864 · 1525584 · 34.59374149659864 |
| 19 | `812ab54f` | 35621 | 37616 | "messbereit" · 36.5 · 36.53124716553288 · 1611028 · 36.29911564625851 | "messbereit" · 36.5 · 36.59374149659864 · 1613784 · 36.59374149659864 |
| 20 | `4d6f4ec2` | 37616 | 39612 | "messbereit" · 38.5 · 38.52678004535147 · 1699031 · 38.43303854875283 | "messbereit" · 38.5 · 38.589297052154194 · 1701788 · 38.589297052154194 |
| 21 | `2de412d7` | 39661 | 41612 | "messbereit" · 40.400000000000006 · 40.47768707482993 · 1785066 · 40.2946485260771 | "messbereit" · 40.5 · 40.58482993197279 · 1789791 · 40.58482993197279 |
| 22 | `5cff38e8` | 41656 | 43607 | "messbereit" · 42.5 · 42.52231292517007 · 1875234 · 42.42857142857143 | "messbereit" · 42.6 · 42.616077097505666 · 1879369 · 42.616077097505666 |
| 23 | `3564d357` | 43652 | 45603 | "messbereit" · 44.5 · 44.517868480725625 · 1963238 · 44.517868480725625 | "messbereit" · 44.6 · 44.611609977324264 · 1967372 · 44.611609977324264 |
| 24 | `fb1af799` | 45652 | 1902 | "sammelt" · 0.8 · 0.861609977324263 · 37997 · 0.861609977324263 | "sammelt" · 0.9 · 0.90625850340136 · 39966 · 0.90625850340136 |

- Ende der Folge: wrap; 24 Anfragen, 48 Positionen
- Anfrage `3209b422d630ca72aec003beabd1c260`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 45 → 1580 ms
  - gen: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (22508 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.52 s nach dem Schreiben)
  - probeeq: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23697 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.53 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `629f7842454e28d7aef8290ad863e492`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 1580 → 3576 ms
  - gen: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23581 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23648 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `e516c24e7299cd238d3142825efdf2d2`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 3576 → 5621 ms
  - gen: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23716 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.00 s nach dem Schreiben)
  - probeeq: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23660 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `cc8d617099785d0c7d0e8f3dc1f50666`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 5621 → 7616 ms
  - gen: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23675 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23661 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `4b3645a5679faf4689588bba8961bf5c`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 7616 → 9612 ms
  - gen: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23712 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23659 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `8525324e72b07b217a92d1a21cc9780c`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 9612 → 11612 ms
  - gen: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23738 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23706 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `fc8752cbffabf889dab624d11cfb17f7`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 11612 → 13607 ms
  - gen: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23743 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23688 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `cd96ac31ceeee8d365ce69df4e7abf8f`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 13607 → 15603 ms
  - gen: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23726 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23696 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `d030be03d83ea11c0e048926b08bd1cb`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 15603 → 17603 ms
  - gen: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26700 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24601 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `2525ea9616bedd7cc9ed30b4339dc112`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 17603 → 19598 ms
  - gen: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26715 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24450 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `ce06a6c039fc8b0cff7c6055522017f6`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 19643 → 21594 ms
  - gen: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26731 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24321 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `830275377acf62c7ed8c1efcac8615dc`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 21594 → 23589 ms
  - gen: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26432 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24157 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `1cb6459af9a4d1e360354c4133519caa`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 23589 → 25589 ms
  - gen: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26441 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24137 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `bb026806048b4958d882271de660fb2f`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 25589 → 27585 ms
  - gen: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26390 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24182 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `16b8c9ad08dbacadd8023eb0549302f9`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 27634 → 29580 ms
  - gen: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26426 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24165 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `5ed3ec287f5fc39c01b99ac213b7cdd4`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 29629 → 31580 ms
  - gen: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26415 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24171 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `86d33685ecfe00bd68099000722f86d0`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 31625 → 33576 ms
  - gen: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26443 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24142 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `53fa9b0a6f732d44ac1604e8c5aedc15`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 33621 → 35621 ms
  - gen: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26431 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24163 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `812ab54f7ae2682763626bc2ba6c8a75`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 35621 → 37616 ms
  - gen: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26442 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24154 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.94 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `4d6f4ec2b31f7437d220b8c205ac5ee9`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 37616 → 39612 ms
  - gen: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26426 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24157 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `2de412d7925eeae6e9b13ba5faefb344`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 39661 → 41612 ms
  - gen: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26442 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24137 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `5cff38e8f21e09e748f7c0e6ba1cb30f`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 41656 → 43607 ms
  - gen: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26459 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24184 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `3564d357f1a9cd3a13cba02bc619778b`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 43652 → 45603 ms
  - gen: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (26450 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24161 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `fb1af799c1ba44c6c9af63505a7a2d4c`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 45652 → 1902 ms
  - gen: `fb1af799c1ba44c6c9af63505a7a2d4c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23724 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `fb1af799c1ba44c6c9af63505a7a2d4c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23669 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `16b8c9ad08dbacadd8023eb0549302f9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1cb6459af9a4d1e360354c4133519caa.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `1ce0fd13f4554957283ec4b864832087.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2525ea9616bedd7cc9ed30b4339dc112.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2de412d7925eeae6e9b13ba5faefb344.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3209b422d630ca72aec003beabd1c260.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4b3645a5679faf4689588bba8961bf5c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `4d6f4ec2b31f7437d220b8c205ac5ee9.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `53fa9b0a6f732d44ac1604e8c5aedc15.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5cff38e8f21e09e748f7c0e6ba1cb30f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5ed3ec287f5fc39c01b99ac213b7cdd4.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `629f7842454e28d7aef8290ad863e492.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `812ab54f7ae2682763626bc2ba6c8a75.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `830275377acf62c7ed8c1efcac8615dc.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `8525324e72b07b217a92d1a21cc9780c.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `86d33685ecfe00bd68099000722f86d0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `bb026806048b4958d882271de660fb2f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cc8d617099785d0c7d0e8f3dc1f50666.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `cd96ac31ceeee8d365ce69df4e7abf8f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `ce06a6c039fc8b0cff7c6055522017f6.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `d030be03d83ea11c0e048926b08bd1cb.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e516c24e7299cd238d3142825efdf2d2.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `fc8752cbffabf889dab624d11cfb17f7.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- gen: `fb1af799` E 37997 < 1963238: Anlaufdaten des naechsten Umlaufs, nie gewertet
- gen: gewertet `3564d357f1a9cd3a13cba02bc619778b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (p_vor 43652 ms), Anker {"bloecke_max_samples": 197, "e": 1963238, "fortlaufend": 1963238, "k": 0, "leicht": [0, 1963238], "n_l": 1963238, "n_s": 1963238, "rate": 44100.0, "s0": 0, "schwer": [0, 1963238], "stillstand": 0}
- probeeq: `fb1af799` E 39966 < 1967372: Anlaufdaten des naechsten Umlaufs, nie gewertet
- probeeq: gewertet `3564d357f1a9cd3a13cba02bc619778b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (p_vor 43652 ms), Anker {"bloecke_max_samples": 197, "e": 1967372, "fortlaufend": 1967372, "k": 0, "leicht": [0, 1967372], "n_l": 1967372, "n_s": 1967372, "rate": 44100.0, "s0": 0, "schwer": [0, 1967372], "stillstand": 0}
- Rechnung F-28 (209.5 s): `{"analyze": "C:\\Users\\phili\\FL-Studio\\tools\\analyze-track.py", "anker": [{"analyze_s": 8.61, "art": "leicht", "ausschnitt": [0, 1963238], "centroid_mag": 828.5258947850594, "corr": 0.5039285811199503, "low_frac": 0.7815396977706075, "low_frac_kanal": 0.7593726124897107, "lufs": -22.380774337530216, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-9829127e-0-1963238.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "sha256": "2F88386C422FF4967ED798C1AC5299223FF166A88DD2D4D74E42C463E91576D3", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3303634821112413}, {"analyze_s": 8.61, "art": "schwer", "ausschnitt": [0, 1963238], "centroid_mag": 828.5258947850594, "corr": 0.5039285811199503, "low_frac": 0.7815396977706075, "low_frac_kanal": 0.7593726124897107, "lufs": -22.380774337530216, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-9829127e-0-1963238.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "s_lufs": {"arbeiter": 12, "dauer_s": 98.41, "max": -22.37461343987253, "min": -22.380774337530216, "nicht_endlich": 0, "phi0": -22.380774337530216, "phi_max": 4388, "phi_min": 0, "spanne": 0.006160897657686348, "versaetze": 4410}, "sha256": "2F88386C422FF4967ED798C1AC5299223FF166A88DD2D4D74E42C463E91576D3", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3303634821112413}, {"analyze_s": 2.65, "art": "leicht", "ausschnitt": [0, 1967372], "centroid_mag": 828.501939386266, "corr": 0.5038705227494916, "low_frac": 0.781279697394338, "low_frac_kanal": 0.7590869167154394, "lufs": -22.385016854426702, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-9829127e-0-1967372.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "sha256": "7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3304148640349438}, {"analyze_s": 2.65, "art": "schwer", "ausschnitt": [0, 1967372], "centroid_mag": 828.501939386266, "corr": 0.5038705227494916, "low_frac": 0.781279697394338, "low_frac_kanal": 0.7590869167154394, "lufs": -22.385016854426702, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-9829127e-0-1967372.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "s_lufs": {"arbeiter": 12, "dauer_s": 99.05, "max": -22.37887414486701, "min": -22.38503200234511, "nicht_endlich": 0, "phi0": -22.385016854426702, "phi_max": 4390, "phi_min": 1293, "spanne": 0.0061578574781009365, "versaetze": 4410}, "sha256": "7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3304148640349438}], "dauer_s": 209.11, "quelle_sha256": "D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF", "u_unten": [{"delta_db": 4.2536484549686325e-12, "eps": 9.79438752324313e-13, "frames_ausserhalb_quelle": 0, "max_zellen": 445, "min_zellen": 444, "n_l": 1963238, "phi_min": 789, "rolle": "gen", "s0": 0, "schwelle_db": -59.999999999995744, "u_unten_s": 44.4, "zellen_phi0": 445}, {"delta_db": 4.2536484549686325e-12, "eps": 9.79438752324313e-13, "frames_ausserhalb_quelle": 0, "max_zellen": 446, "min_zellen": 445, "n_l": 1967372, "phi_min": 513, "rolle": "probeeq", "s0": 0, "schwelle_db": -59.999999999995744, "u_unten_s": 44.5, "zellen_phi0": 446}]}`
- Referenzausschnitt gen leicht: Frames [0, 1963238), K 0, v 0, SHA-256 2F88386C422FF4967ED798C1AC5299223FF166A88DD2D4D74E42C463E91576D3, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-9829127e-0-1963238.wav
- Referenzausschnitt gen schwer: Frames [0, 1963238), K 0, v 0, SHA-256 2F88386C422FF4967ED798C1AC5299223FF166A88DD2D4D74E42C463E91576D3, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-9829127e-0-1963238.wav
- Referenzausschnitt probeeq leicht: Frames [0, 1967372), K 0, v 0, SHA-256 7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-9829127e-0-1967372.wav
- Referenzausschnitt probeeq schwer: Frames [0, 1967372), K 0, v 0, SHA-256 7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-9829127e-0-1967372.wav
- gen: U_unten 44.4 s = 0,1 s x min Z_phi (phi 789; phi 0: 445 Zellen; Schwelle -59.999999999995744 dB, Abstand 4.2536484549686325e-12 dB), aktiv_sekunden 44.5
- gen roh `snapshot.spektral.centroid_mag_hz` = [886.6567596735559]
- gen roh `snapshot.loudness.lufs_integriert` = [-22.38077433753024]
- gen roh `snapshot.loudness.true_peak_dbtp` = [-7.867515515804746]
- gen roh `snapshot.stereo.width` = [0.330363482111323]
- gen roh `snapshot.stereo.corr` = [0.503928581118176]
- gen roh `snapshot.spektral.low_frac` = [0.759372614062002]
- gen roh `snapshot.resonanzen.*.freq_hz` = [688.6962107092802, 459.64957473531535, 919.2991494706307]
- gen roh `snapshot.zustand` = ["messbereit"]
- gen roh `snapshot.aktiv_sekunden` = [44.5]
- gen roh `snapshot.gesamt_sekunden` = [44.517868480725625]
- gen roh `frame.schwer_sekunden` = [44.517868480725625]
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
- Band LUFS gen: {"ausschnitt": [0, 1963238], "breite": 0.07616089765768636, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "mitte": -22.380774337530216, "referenz": -22.380774337530216, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": -22.38077433753024}
- Band TP gen: {"feld": "snapshot.loudness.true_peak_dbtp", "grund": "kein Band fuer diese Rolle", "kurz": "TP", "rolle": "gen", "status": "roh", "wert": -7.867515515804746}
- Band width gen: {"ausschnitt": [0, 1963238], "breite": 0.01, "feld": "snapshot.stereo.width", "kurz": "width", "mitte": 0.3303634821112413, "referenz": 0.3303634821112413, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.330363482111323}
- Band corr gen: {"ausschnitt": [0, 1963238], "breite": 0.01, "feld": "snapshot.stereo.corr", "kurz": "corr", "mitte": 0.5039285811199503, "referenz": 0.5039285811199503, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.503928581118176}
- Band low_frac gen: {"ausschnitt": [0, 1963238], "breite": 0.02, "feld": "snapshot.spektral.low_frac", "kurz": "low_frac", "mitte": 0.7593726124897107, "referenz": 0.7593726124897107, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.759372614062002}
- Band resonanzen gen: {"ausschnitt": [0, 1963238], "feld": "snapshot.resonanzen.*.freq_hz", "kurz": "resonanzen", "referenz": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": [688.6962107092802, 459.64957473531535, 919.2991494706307]}
- Band LUFS probeeq: {"ausschnitt": [0, 1967372], "breite": 0.07615785747810094, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "mitte": -22.385016854426702, "referenz": -22.385016854426702, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": -22.385016854426723}
- Band TP probeeq: {"ausschnitt": [0, 1967372], "breite": 0.12, "feld": "snapshot.loudness.true_peak_dbtp", "kurz": "TP", "mitte": -7.867515649360563, "referenz": -7.867515649360563, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": -7.867515515804746}
- Band width probeeq: {"ausschnitt": [0, 1967372], "breite": 0.01, "feld": "snapshot.stereo.width", "kurz": "width", "mitte": 0.3304148640349438, "referenz": 0.3304148640349438, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.330414864035028}
- Band corr probeeq: {"ausschnitt": [0, 1967372], "breite": 0.01, "feld": "snapshot.stereo.corr", "kurz": "corr", "mitte": 0.5038705227494916, "referenz": 0.5038705227494916, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.503870522747712}
- Band low_frac probeeq: {"ausschnitt": [0, 1967372], "breite": 0.02, "feld": "snapshot.spektral.low_frac", "kurz": "low_frac", "mitte": 0.7590869167154394, "referenz": 0.7590869167154394, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.759086918280695}
- Band resonanzen probeeq: {"ausschnitt": [0, 1967372], "feld": "snapshot.resonanzen.*.freq_hz", "kurz": "resonanzen", "referenz": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": [688.6962107092802, 459.64957473531535, 919.2991494706307]}
- Rohvergleich Runde 01 (nur daneben, F-28): {"centroid_offline": 829, "centroid_plugin": [876, 879], "corr": [0.5, 0.507], "low_frac_offline": 0.78, "lufs_offline": -22.41, "lufs_plugin": [-22.45, -22.48], "quelle": "eq-copilot/kalibration/KALIBRIER-PROTOKOLL.md:68-69", "resonanzen_offline": [229.8, 354.4, 459.6, 546.6, 688.7, 919.3], "resonanzen_plugin": [688.7, 459.6, 919.3], "tp_offline": -7.87, "tp_plugin": [-7.75, -6.47], "width": [0.33, 0.328]}

<!-- szenario.py 2026-09-23T09:29:59+00:00 -->
## Szenario `u40-aktivitaetsgate` — Karte U40 als Messung: Aktivitaet, Abdeckung, LUFS-Short, True Peak und gezaehlte Fenster und Rahmen je leiser Stelle des Testtracks - Rohdaten, keine Deutung

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `request_id=b5cf4f80c154ff9a7bf6e0aa54d65ab6 {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `request_id=772f5ca2241c9ed83052d00cd25c92c9 {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `transport.setLoopMode` | `{"mode": "song"}` | `request_id=2f07a44afee62114867eabf1b8d38aa1 {"mode": "song", "success": true}` | ok |
| 4 | `lokal.stellen` | `{"rollen": ["gen", "probeeq"], "stellen": [{"bis_s": 3.692, "energie": {"anteil_66_40": 0.0, "max_db": -22.628, "min_db": -27.205, "mittel_db": -25.205, "zellen": 36}, "id": "S1", "name": "Intro", "takte": "1-2", "von_s": 0.0}, {"bis_s": 11.077, "energie": {"anteil_66_40": 0.0, "max_db": -22.147, "min_db": -27.558, "mittel_db": -25.112, "zellen": 73}, "id": "S2", "name": "ruhige Passage", "takte"…` | gemessen, Rohdaten unten | ok (Details unten) |

**Ergebnis:** 4 von 4 Schritten bestanden.


### Schritt 4 `lokal.stellen`

- Quelle `C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav` SHA-256 D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF, erwartet D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF
- Versatz v = 0 Frames; Nulltest {"delta_k_db": 0.0, "g": 1.0, "urteil": "BITIDENTISCH", "v": 0}

#### Stelle S1 Intro: Quelle [0.0, 3.692) s, Host [0, 162817) Frames
- Stopp `{"request_id": "555086c8c7aa51236a847d3b6f89a55c", "stopped": true, "success": true}`; setPosition 0 ms `{"mode": 0, "request_id": "9312ecb796e9aaabdbab3c48f2f68025", "requested_position": 0, "success": true}`; getPosition roh `{"hint": "1:01:00", "ms": 0, "request_id": "b4ff60897181d4f0b4d0e87d02b6a9ee", "seconds": 0, "success": true}` nach 0.016 s
- Phasenmessung (nicht gezaehlt) `5e43c59b`: je Rolle {"gen": 0.008, "probeeq": 1.934} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.016, "probeeq": 1.953} s; gemeinsamer Zeitplan: 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle
- Durchlauf gen und probeeq: Zeitplan erste Anfrage -0.18 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.000 s
- Durchlauf gen und probeeq: Start `{"is_playing": true, "request_id": "dc3d8ea4ba7d380d4e6a729076050528", "success": true}` nach 0.031 s (geplant ab +0.004 s), erste Anfrage geplant -0.18 s zum Start
- Durchlauf gen und probeeq: Ende der Anfragen: Stellenende erreicht; Stopp `{"request_id": "828777e171fe9c316458d630d10b5678", "stopped": true, "success": true}`
- Anfrage `5e43c59b30da38353e5c50924440000e`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer None → None ms
  - gen: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (24235 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.02 s nach dem Schreiben)
  - probeeq: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (24191 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `9a2084b6ab1615afdfbf961cccc1a36b`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 0 → 1670 ms
  - gen: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23685 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23708 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.86 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `3754c4f35edbb91427007f3ceadcbf51`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 1719 → 3670 ms
  - gen: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23679 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.05 s nach dem Schreiben)
  - probeeq: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23642 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `027458b2afd5dd2f13d185f98d0688f8`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 3714 → 5665 ms
  - gen: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23705 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.05 s nach dem Schreiben)
  - probeeq: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23657 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `74c5f9fcdff613a6cf7db13ac0465d6f`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 5714 → 7710 ms
  - gen: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23721 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.03 s nach dem Schreiben)
  - probeeq: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23675 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S1 | gen | gemeinsam | 16/16 | `9a2084b6` | gezaehlt | true | 1.0 | 1.0 | 0.780126988887787 | null | -16.65938949584961 | -16.661378860473633 | 2 | 1043 | 5868 | 2 | 2 | 2670 | 4528 | 0 | 0 | 28547 | true | [0, 162817) v = 0 | 0 | 1670 |
| S1 | gen | gemeinsam | 16/16 | `3754c4f3` | gezaehlt | false | 1.0 | null | null | null | -14.135613441467285 | -14.137906074523926 | null | 1063 | 5888 | 3 | 3 | 2680 | 4428 | 0 | 57344 | 16384 | true | [0, 162817) v = 0 | 1719 | 3670 |
| S1 | gen | gemeinsam | 16/16 | `027458b2` | gezaehlt | false | 1.0 | null | null | -24.45303726196289 | -14.812978744506836 | -14.815757751464844 | null | 1103 | 5928 | 6 | 6 | 2699 | 4528 | 0 | 139264 | 20402 | true | [0, 162817) v = 0 | 3714 | 5665 |
| S1 | gen | gemeinsam | 16/16 | `74c5f9fc` | Ausschnitt [229376, 250228) nicht ganz in [0, 162817) | false | 1.0 | null | null | -24.92220115661621 | -15.90353775024414 | -15.90353775024414 | null | 1149 | 5974 | 3 | 3 | 2719 | 4528 | 0 | 229376 | 20852 | true | [0, 162817) v = 0 | 5714 | 7710 |
| S1 | probeeq | gemeinsam | 16/16 | `9a2084b6` | gezaehlt | true | 1.0 | 1.0 | 0.858825981616974 | null | -15.207754135131836 | -15.211620330810547 | 2 | 1048 | 5888 | 2 | 2 | 2679 | 4528 | 0 | 40960 | 28340 | true | [0, 162817) v = 0 | 0 | 1670 |
| S1 | probeeq | gemeinsam | 16/16 | `3754c4f3` | gezaehlt | false | 1.0 | null | null | -24.45303726196289 | -14.812978744506836 | -14.815757751464844 | null | 1088 | 5928 | 5 | 5 | 2699 | 4528 | 0 | 139264 | 20402 | true | [0, 162817) v = 0 | 1719 | 3670 |
| S1 | probeeq | gemeinsam | 16/16 | `027458b2` | Ausschnitt [221184, 245700) nicht ganz in [0, 162817) | true | 1.0 | 1.0 | 0.869620025157928 | -24.927047729492188 | -17.87503433227539 | -17.88389015197754 | 1 | 1134 | 5974 | 2 | 2 | 2718 | 4528 | 0 | 221184 | 24516 | true | [0, 162817) v = 0 | 3714 | 5665 |
| S1 | probeeq | gemeinsam | 16/16 | `74c5f9fc` | Ausschnitt [319488, 336262) nicht ganz in [0, 162817) | false | 1.0 | null | null | -24.235475540161133 | -13.52647876739502 | -13.527809143066406 | null | 1174 | 6014 | 5 | 5 | 2738 | 4528 | 0 | 319488 | 16774 | true | [0, 162817) v = 0 | 5714 | 7710 |

- Stelle S1 gen (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 3, "d_frames_gebaut": 29, "d_summe_fenster_aktiv": 60, "d_summe_fenster_gesamt": 60, "erster_ausschnitt": [0, 28547], "letzter_ausschnitt": [139264, 159666]}
- Stelle S1 probeeq (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 2, "d_frames_gebaut": 20, "d_summe_fenster_aktiv": 40, "d_summe_fenster_gesamt": 40, "erster_ausschnitt": [40960, 69300], "letzter_ausschnitt": [139264, 159666]}

#### Stelle S2 ruhige Passage: Quelle [3.692, 11.077) s, Host [162817, 488496) Frames
- Stopp `{"request_id": "4966a3c1f4cde9ee6b796a123b52540a", "stopped": true, "success": true}`; setPosition 3692 ms `{"mode": 0, "request_id": "c755fbe6a8f933bf0dc219fb9b49506b", "requested_position": 3692, "success": true}`; getPosition roh `{"hint": "3:03:11", "ms": 3692, "request_id": "b18c23d351739c889937847857b88202", "seconds": 4, "success": true}` nach 0.016 s
- Phasenmessung (nicht gezaehlt) `246afef0`: je Rolle {"gen": 0.74, "probeeq": 1.665} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.75, "probeeq": 1.687} s; gemeinsamer Zeitplan: 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle
- Durchlauf gen und probeeq: Zeitplan erste Anfrage -0.38 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 1.810 s
- Durchlauf gen und probeeq: Start `{"is_playing": true, "request_id": "ca7b0b098d5058a5c58516c05ba000a6", "success": true}` nach 0.016 s (geplant ab -0.007 s), erste Anfrage geplant -0.38 s zum Start
- Durchlauf gen und probeeq: Ende der Anfragen: Stellenende erreicht; Stopp `{"request_id": "8c13c0082986e8cb6204803b62eb63e2", "stopped": true, "success": true}`
- Anfrage `246afef0e31123d9366b9b0d672852d3`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer None → None ms
  - gen: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (15951 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.75 s nach dem Schreiben)
  - probeeq: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (15901 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.69 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `2a05a42d251b7d69e12916f730ea6c81`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 3692 → 5179 ms
  - gen: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (22343 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23533 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.86 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `242e059b2a16e620baa0dc55109a7ed5`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 5223 → 7223 ms
  - gen: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23629 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.02 s nach dem Schreiben)
  - probeeq: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23598 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `314da53e746f2ee146b827ad935a332e`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 7223 → 9219 ms
  - gen: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23684 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.03 s nach dem Schreiben)
  - probeeq: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23663 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `a7536b3198533ebe8140097cdcfd7b43`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 9219 → 11170 ms
  - gen: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23714 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.03 s nach dem Schreiben)
  - probeeq: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23669 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `e27a4821625662ad990238d052e4a134`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer 11214 → 13165 ms
  - gen: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23676 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.05 s nach dem Schreiben)
  - probeeq: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23672 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S2 | gen | gemeinsam | 16/16 | `2a05a42d` | gezaehlt | true | 1.0 | 1.0 | 0.844633460044861 | null | -17.149717330932617 | -17.149717330932617 | 1 | 1199 | 6090 | 0 | 0 | 2778 | 4528 | 0 | 162817 | 24017 | true | [162817, 488496) v = 0 | 3692 | 5179 |
| S2 | gen | gemeinsam | 16/16 | `242e059b` | gezaehlt | true | 1.0 | 1.0 | 0.867649555206299 | null | -14.770365715026855 | -14.771775245666504 | 1 | 1219 | 6110 | 1 | 1 | 2787 | 4528 | 0 | 203777 | 23810 | true | [162817, 488496) v = 0 | 5223 | 7223 |
| S2 | gen | gemeinsam | 16/16 | `314da53e` | gezaehlt | false | 1.0 | null | null | -24.73749542236328 | -12.061075210571289 | -12.0626802444458 | null | 1259 | 6150 | 5 | 5 | 2807 | 4528 | 0 | 310273 | 7680 | true | [162817, 488496) v = 0 | 7223 | 9219 |
| S2 | gen | gemeinsam | 16/16 | `a7536b31` | gezaehlt | true | 1.0 | 1.0 | 0.873827517032623 | -24.111417770385742 | -15.413409233093262 | -15.41372299194336 | 2 | 1305 | 6196 | 1 | 1 | 2826 | 4528 | 0 | 375809 | 28178 | true | [162817, 488496) v = 0 | 9219 | 11170 |
| S2 | gen | gemeinsam | 16/16 | `e27a4821` | Ausschnitt [474113, 494550) nicht ganz in [162817, 488496) | false | 1.0 | null | null | -24.220050811767578 | -13.392220497131348 | -13.392547607421875 | null | 1345 | 6236 | 4 | 4 | 2846 | 4528 | 0 | 474113 | 20437 | true | [162817, 488496) v = 0 | 11214 | 13165 |
| S2 | probeeq | gemeinsam | 16/16 | `2a05a42d` | gezaehlt | true | 1.0 | 1.0 | 0.867649555206299 | null | -14.770365715026855 | -14.771775245666504 | 1 | 1204 | 6110 | 0 | 0 | 2787 | 4528 | 0 | 203777 | 23810 | true | [162817, 488496) v = 0 | 3692 | 5179 |
| S2 | probeeq | gemeinsam | 16/16 | `242e059b` | gezaehlt | false | 1.0 | null | null | -24.958499908447266 | -13.595952033996582 | -13.599640846252441 | null | 1244 | 6150 | 3 | 3 | 2806 | 4528 | 0 | 293889 | 19536 | true | [162817, 488496) v = 0 | 5223 | 7223 |
| S2 | probeeq | gemeinsam | 16/16 | `314da53e` | gezaehlt | true | 1.0 | 1.0 | 0.873827517032623 | -24.111417770385742 | -15.413409233093262 | -15.41372299194336 | 2 | 1290 | 6196 | 0 | 0 | 2826 | 4528 | 0 | 375809 | 28178 | true | [162817, 488496) v = 0 | 7223 | 9219 |
| S2 | probeeq | gemeinsam | 16/16 | `a7536b31` | Ausschnitt [482305, 490022) nicht ganz in [162817, 488496) | false | 1.0 | null | null | -24.24235725402832 | -16.337068557739258 | -16.338058471679688 | null | 1330 | 6236 | 3 | 3 | 2845 | 4528 | 0 | 482305 | 7717 | true | [162817, 488496) v = 0 | 9219 | 11170 |
| S2 | probeeq | gemeinsam | 16/16 | `e27a4821` | Ausschnitt [556033, 580584) nicht ganz in [162817, 488496) | true | 1.0 | 1.0 | 0.725998401641846 | -24.795475006103516 | -13.75872802734375 | -13.764945983886719 | 1 | 1376 | 6282 | 0 | 0 | 2865 | 4528 | 0 | 556033 | 24551 | true | [162817, 488496) v = 0 | 11214 | 13165 |

- Stelle S2 gen (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 4, "d_frames_gebaut": 48, "d_summe_fenster_aktiv": 106, "d_summe_fenster_gesamt": 106, "erster_ausschnitt": [162817, 186834], "letzter_ausschnitt": [375809, 403987]}
- Stelle S2 probeeq (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 3, "d_frames_gebaut": 39, "d_summe_fenster_aktiv": 86, "d_summe_fenster_gesamt": 86, "erster_ausschnitt": [203777, 227587], "letzter_ausschnitt": [375809, 403987]}

#### Stelle S3 Ausklang: Quelle [42.462, 45.596) s, Host [1872574, 2010784) Frames
- Stopp `{"request_id": "d91e9ad226153dc86dad5cd8dabb8984", "stopped": true, "success": true}`; setPosition 42462 ms `{"mode": 0, "request_id": "b375193cd2a138f14988e25f0cb36bcb", "requested_position": 42462, "success": true}`; getPosition roh `{"hint": "25:13:07", "ms": 42460, "request_id": "ba2ed8307a3a05726c0d26b2d69681d1", "seconds": 42, "success": true}` nach 0.016 s
- Phasenmessung (nicht gezaehlt) `047ba8f7`: je Rolle {"gen": 0.751, "probeeq": 1.677} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.766, "probeeq": 1.687} s; gemeinsamer Zeitplan: 6 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle; getrennte Durchlaeufe je Rolle
- Durchlauf gen: Zeitplan erste Anfrage -1.08 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.689 s
- Durchlauf gen: Start `{"is_playing": true, "request_id": "4f821ef5a5dccdf4e23fd9f973a425bc", "success": true}` nach 0.016 s (geplant ab +0.002 s), erste Anfrage geplant -1.08 s zum Start
- Durchlauf gen: Ende der Anfragen: wrap; Stopp `{"request_id": "2edf355b740dcc923de96d3d5ef74d2e", "stopped": true, "success": true}`
- Durchlauf probeeq: Stopp `{"request_id": "686f0102a77ec42fc9470fdbae715134", "stopped": true, "success": true}`; setPosition 42462 ms `{"mode": 0, "request_id": "d9f563e5a6d33764cc5a93aef1259b3e", "requested_position": 42462, "success": true}`; getPosition roh `{"hint": "25:13:07", "ms": 42460, "request_id": "9bd5e17734d3b3c81051cd19ff7cc7f9", "seconds": 42, "success": true}` nach 0.032 s
- Durchlauf probeeq: Zeitplan erste Anfrage -0.89 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.154 s
- Durchlauf probeeq: Start `{"is_playing": true, "request_id": "6fa1d1024e8132c3d39ca667aef3a674", "success": true}` nach 0.015 s (geplant ab -0.002 s), erste Anfrage geplant -0.89 s zum Start
- Durchlauf probeeq: Ende der Anfragen: wrap; Stopp `{"request_id": "724c8f99377d5316dc3224c39f6cac23", "stopped": true, "success": true}`
- Anfrage `047ba8f7723fbd0e1a63fb9d5f943eee`, erwartet {"gen": [242392], "probeeq": [242392]}, Positionsklammer None → None ms
  - gen: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (15957 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.77 s nach dem Schreiben)
  - probeeq: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (15907 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.69 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
- Anfrage `8e1161348c4a4935c652a5c1f0243344`, erwartet {"gen": [242392]}, Positionsklammer 42460 → 42460 ms
  - gen: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (15957 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.09 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
- Anfrage `630a9ceec79339d79984c0abf71d932c`, erwartet {"gen": [242392]}, Positionsklammer 42509 → 42879 ms
  - gen: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (22333 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.39 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
- Anfrage `fe0dab26bbce293874e2d2abcdbc6a22`, erwartet {"gen": [242392]}, Positionsklammer 43531 → 43902 ms
  - gen: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23580 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.34 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
- Anfrage `dc96c4551b4659ab9fda705e06f5daa0`, erwartet {"gen": [242392]}, Positionsklammer 44554 → 44875 ms
  - gen: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (23633 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.33 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
- Anfrage `e8694285b8778080982066b4be59c9b0`, erwartet {"gen": [242392]}, Positionsklammer 45571 → 196 ms
  - gen: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json` (13772 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.31 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: Rolle probeeq ohne Erwartung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
- Anfrage `478ac87d5920f1abb5da76261479be26`, erwartet {"probeeq": [242392]}, Positionsklammer 42460 → 43438 ms
  - probeeq: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (1278 B, Version '0.3.0', gruende ['noch_keine_messdaten', 'rolle_ohne_aggregat'], 1.88 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: Rolle gen ohne Erwartung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
- Anfrage `6adcaf46213b7054111c2e08d1a48367`, erwartet {"probeeq": [242392]}, Positionsklammer 43438 → 45433 ms
  - probeeq: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23541 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: Rolle gen ohne Erwartung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
- Anfrage `507b7aa815d30ff93e8c7d26ac21c431`, erwartet {"probeeq": [242392]}, Positionsklammer 45433 → 1732 ms
  - probeeq: `507b7aa815d30ff93e8c7d26ac21c431.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json` (23686 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.98 s nach dem Schreiben)
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `027458b2afd5dd2f13d185f98d0688f8.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `047ba8f7723fbd0e1a63fb9d5f943eee.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `242e059b2a16e620baa0dc55109a7ed5.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `246afef0e31123d9366b9b0d672852d3.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `2a05a42d251b7d69e12916f730ea6c81.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `314da53e746f2ee146b827ad935a332e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `3754c4f35edbb91427007f3ceadcbf51.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `478ac87d5920f1abb5da76261479be26.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: Rolle gen ohne Erwartung: `507b7aa815d30ff93e8c7d26ac21c431.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `5e43c59b30da38353e5c50924440000e.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `630a9ceec79339d79984c0abf71d932c.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `6adcaf46213b7054111c2e08d1a48367.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `74c5f9fcdff613a6cf7db13ac0465d6f.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `8e1161348c4a4935c652a5c1f0243344.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `9a2084b6ab1615afdfbf961cccc1a36b.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `a7536b3198533ebe8140097cdcfd7b43.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `dc96c4551b4659ab9fda705e06f5daa0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e27a4821625662ad990238d052e4a134.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`
  - roh fremd: andere Kennung: `e8694285b8778080982066b4be59c9b0.probeeq.242392.d387a78151edaa16.3a7c165d4e364962b127052f58f076d5.json`
  - roh fremd: andere Kennung: `fe0dab26bbce293874e2d2abcdbc6a22.gen.242392.2f869d2ee2d9099f.adf4d399201d4778b9b39c89ec5ce65c.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S3 | gen | getrennt | 16/16 | `8e116134` | Materialausschnitt null: zaehlt nie | false | 0.0 | null | null | null | null | null | null | 1391 | 6333 | 0 | 6 | 2894 | 4416 | 0 | null | null | false | [1872574, 2010784) v = 0 | 42460 | 42460 |
| S3 | gen | getrennt | 16/16 | `630a9cee` | gezaehlt | true | 1.0 | 1.0 | 0.714650928974152 | null | -16.252164840698242 | -16.252670288085938 | 6 | 1397 | 6352 | 1 | 1 | 2904 | 4528 | 0 | 1872574 | 14473 | true | [1872574, 2010784) v = 0 | 42509 | 42879 |
| S3 | gen | getrennt | 16/16 | `fe0dab26` | gezaehlt | false | 1.0 | null | null | null | -14.028632164001465 | -14.028903007507324 | null | 1416 | 6371 | 3 | 3 | 2914 | 4528 | 0 | 1913534 | 18794 | true | [1872574, 2010784) v = 0 | 43531 | 43902 |
| S3 | gen | getrennt | 16/16 | `dc96c455` | gezaehlt | false | 1.0 | null | null | null | -21.91321563720703 | -21.91814422607422 | null | 1436 | 6391 | 4 | 4 | 2923 | 4528 | 0 | 1954494 | 18587 | true | [1872574, 2010784) v = 0 | 44554 | 44875 |
| S3 | gen | getrennt | 16/16 | `e8694285` | Ausschnitt [0, 3150) nicht ganz in [1872574, 2010784) | false | null | null | null | null | -15.642861366271973 | -15.643216133117676 | null | 1456 | 6411 | 2 | 2 | 2933 | 3150 | 0 | 0 | 3150 | true | [1872574, 2010784) v = 0 | 45571 | 196 |
| S3 | probeeq | getrennt | 16/16 | `478ac87d` | gezaehlt | false | 1.0 | null | null | null | -15.911407470703125 | -15.9158935546875 | null | 1459 | 6465 | 3 | 3 | 2962 | 4528 | 0 | 1905342 | 6905 | true | [1872574, 2010784) v = 0 | 42460 | 43438 |
| S3 | probeeq | getrennt | 16/16 | `6adcaf46` | gezaehlt | true | 1.0 | 1.0 | 0.738255441188812 | null | -36.642024993896484 | -36.644187927246094 | 1 | 1505 | 6511 | 0 | 0 | 2982 | 4528 | 0 | 1979070 | 23739 | true | [1872574, 2010784) v = 0 | 43438 | 45433 |
| S3 | probeeq | getrennt | 16/16 | `507b7aa8` | Ausschnitt [65536, 73631) nicht ganz in [1872574, 2010784) | false | 1.0 | null | null | null | -14.135613441467285 | -14.137906074523926 | null | 1537 | 6543 | 4 | 4 | 3001 | 4528 | 0 | 65536 | 8095 | true | [1872574, 2010784) v = 0 | 45433 | 1732 |

- Stelle S3 gen (durchlauf getrennt, kombinationen 16/16): {"antworten": 3, "d_frames_gebaut": 19, "d_summe_fenster_aktiv": 39, "d_summe_fenster_gesamt": 39, "erster_ausschnitt": [1872574, 1887047], "letzter_ausschnitt": [1954494, 1973081]}
- Stelle S3 probeeq (durchlauf getrennt, kombinationen 16/16): {"antworten": 2, "d_frames_gebaut": 20, "d_summe_fenster_aktiv": 46, "d_summe_fenster_gesamt": 46, "erster_ausschnitt": [1905342, 1912247], "letzter_ausschnitt": [1979070, 2002809]}

