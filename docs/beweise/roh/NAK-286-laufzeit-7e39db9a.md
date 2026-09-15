# Laufzeit-Arm NAK-286 - VORAUSSETZUNG

Zeit: 2026-09-15 10:41:19 | Basis: 7e39db9ab14866932cc7ddcfbd132df2bf141094 | HEAD: 7e39db9a | Repo: C:\Users\phili\Projekte\Nakama | Projekt: Nakama-Diagnose.flp
Kopfzeile: LAUFZEIT NAK-286 7e39db9a VORAUSSETZUNG installation=installiert; pruefen Exit 0, 3 Artefakt(e) aktuell szenarien=5 verfehlt=2 [bereitschaft.json=0,fenster.json=5,nulltest-host.json=5,snapshot-runde01.json=0,u40-aktivitaetsgate.json=0] fl=Producer Edition v26.1.4 [build 5589]

## Protokoll

```text
[2026-09-15 10:35:24] Laufzeit-Arm NAK-286 auf 7e39db9a (Basis 7e39db9ab14866932cc7ddcfbd132df2bf141094), Repo C:\Users\phili\Projekte\Nakama
[2026-09-15 10:35:24] Lohnt es? True - erzwungen (-Erzwingen)
[2026-09-15 10:35:25] Diagnose-FL beenden: PID 27792 'Nakama-Diagnose.flp - FL Studio 2026' (Projekt wird nie gespeichert)
[2026-09-15 10:35:25] FL-Restprozess PID 27792: beendet
[2026-09-15 10:35:25] Manifest-Hashes nachziehen (--hashen)
[2026-09-15 10:35:25]   hashen: Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
[2026-09-15 10:35:25]   hashen:                  Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[2026-09-15 10:35:25]   hashen: [0] Struktur vor dem mutierenden Release-Schritt
[2026-09-15 10:35:25]   hashen:   ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
[2026-09-15 10:35:25]   hashen:   ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
[2026-09-15 10:35:25]   hashen:   ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
[2026-09-15 10:35:25]   hashen:   ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
[2026-09-15 10:35:25]   hashen:   ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
[2026-09-15 10:35:25]   hashen:   ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
[2026-09-15 10:35:25]   hashen:   ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
[2026-09-15 10:35:25]   hashen:   ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
[2026-09-15 10:35:25]   hashen:   ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
[2026-09-15 10:35:25]   hashen:   ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
[2026-09-15 10:35:25]   hashen:   ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
[2026-09-15 10:35:25]   hashen:   ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
[2026-09-15 10:35:25]   hashen:   ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
[2026-09-15 10:35:25]   hashen:   ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
[2026-09-15 10:35:25]   hashen:   ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)
[2026-09-15 10:35:25]   hashen: [hashen] Artefakte gegen den gebauten Stand festschreiben
[2026-09-15 10:35:25]   hashen:   ok      main = 35BA2C4895614209CB7A2408B33FFD9982370D4E46A1EC71E18CEC4BF34B21A2
[2026-09-15 10:35:25]   hashen:   ok      active-probe = 7D7F9BAA28B52BE848F5330AC9664947A6DB700800CE0B527331A9BA1E01AC46
[2026-09-15 10:35:25]   hashen:   ok      eqcop-broker.exe = CA85733027ACCEF5E7D98EA4FBCFFD12E7BCFECAE3DA3E2284834D759AD2B306
[2026-09-15 10:35:25]   hashen: 
[2026-09-15 10:35:25]   hashen: geschrieben: eq-copilot/install/nakama-installer-v1.json
[2026-09-15 10:35:25] Aufgabe \Nakama\installieren starten
[2026-09-15 10:35:29]   installieren: Exit 0, Zustand Ready, Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3] |   ok      Nakama Probeeq installiert  [C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3] |   ok      eqcop-broker.exe installiert  [C:\Program Files\evenacadia\Nakama\eqcop-broker.exe] | INSTALLATION OK
[2026-09-15 10:35:31]   pruefen: Exit 0, Zustand Ready, Log: Installierter Stand gegen das Manifest: |   aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3 |   aktuell          C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3 |   aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe
[2026-09-15 10:35:31] Controller-Skript: SHA-256 Repo 704DECFF3689D618E48CF0B8F175A67E3B73312C0A9BA1DAE6F5CF9206D834EC, installiert 704DECFF3689D618E48CF0B8F175A67E3B73312C0A9BA1DAE6F5CF9206D834EC (C:\Users\phili\OneDrive\Dokumente\Image-Line\FL Studio\Settings\Hardware\FLStudioMCP\device_FLStudioMCP.py), script_version 2026-09-15
[2026-09-15 10:35:31] Diagnoseprojekt: SHA-256 Repo 65FA78F821EA03CD944427F5470EA42C8BAF47C1CF65A397DB9B205E5907A335, Arbeitskopie 65FA78F821EA03CD944427F5470EA42C8BAF47C1CF65A397DB9B205E5907A335
[2026-09-15 10:35:31] Referenzprojekt Nakama-Diagnose-Verarbeitung.flp: fehlt neben dem Diagnoseprojekt (C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\fl\Nakama-Diagnose-Verarbeitung.flp; Karte U43), nicht kopiert
[2026-09-15 10:35:31] Referenzprojekt Nakama-Diagnose-Referenz.flp: fehlt neben dem Diagnoseprojekt (C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\fl\Nakama-Diagnose-Referenz.flp; Karte U43), nicht kopiert
[2026-09-15 10:35:31] Briefkasten: C:\Users\phili\AppData\Local\evenacadia\nakama\diagnose bereit, 40 Antwortdatei(en) frueherer Laeufe entfernt
[2026-09-15 10:35:31] Render gestartet: PID 24532 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe /R /Ewav /O"C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render" "C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp")
[2026-09-15 10:35:35] Render: Exit 0, Dauer 4,1 s, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\Nakama-Diagnose.wav, SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A, Grund 
[2026-09-15 10:35:35] loopMIDI laeuft
[2026-09-15 10:35:35] FL gestartet: PID 17516 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-15 10:35:38] Boot-Marke nach 3 s: { "script_version": "2026-09-15", "loaded_at": "2026-09-15T10:35:37", "program_title": "FL Studio 2026" }
[2026-09-15 10:35:46] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 17516
[2026-09-15 10:35:50] Szenario bereitschaft.json: Exit 0
[2026-09-15 10:35:51] Szenario fenster.json: Exit 5
[2026-09-15 10:35:54] Szenario nulltest-host.json: Exit 5
[2026-09-15 10:35:54] Szenario snapshot-runde01.json: frischer_start - Diagnose-FL neu starten
[2026-09-15 10:35:54] Diagnose-FL beenden: PID 17516 'Nakama-Diagnose.flp - FL Studio 2026' (Projekt wird nie gespeichert)
[2026-09-15 10:35:54] FL-Restprozess PID 17516: beendet
[2026-09-15 10:35:55] FL gestartet: PID 29068 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-15 10:35:58] Boot-Marke nach 3 s: { "script_version": "2026-09-15", "loaded_at": "2026-09-15T10:35:57", "program_title": "FL Studio 2026" }
[2026-09-15 10:36:06] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1, Diagnose-PID 29068
[2026-09-15 10:40:36] Szenario snapshot-runde01.json: Exit 0
[2026-09-15 10:41:19] Szenario u40-aktivitaetsgate.json: Exit 0
[2026-09-15 10:41:19] Diagnoseprojekt am Ende: SHA-256 Repo 65FA78F821EA03CD944427F5470EA42C8BAF47C1CF65A397DB9B205E5907A335, Arbeitskopie 65FA78F821EA03CD944427F5470EA42C8BAF47C1CF65A397DB9B205E5907A335
```
## Szenarien


<!-- szenario.py 2026-09-15T08:35:47+00:00 -->
## Szenario `bereitschaft` — FL antwortet, das Diagnoseprojekt mit beiden Plugins ist offen, Ton fließt durch Sondenspur und Master

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `{"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `plugins.getName` | `{"index": 0, "slot_index": 0, "use_global": true}` | `{"name": "EQ-Copilot", "success": true}` | ok |
| 3 | `plugins.getName` | `{"index": 1, "slot_index": 0, "use_global": true}` | `{"name": "Nakama Probeeq", "success": true}` | ok |
| 4 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `{"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok |
| 5 | `transport.getLength` | `{}` | `{"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok |
| 6 | `transport.setLoopMode` | `{"mode": "song"}` | `{"mode": "song", "success": true}` | ok |
| 7 | `transport.stop` | `{}` | `{"stopped": true, "success": true}` | ok |
| 8 | `transport.setPosition` | `{"mode": 1, "position": 2}` | `{"mode": 1, "requested_position": 2, "success": true}` | ok |
| 9 | `transport.start` | `{}` | `{"is_playing": true, "success": true}` | ok |
| 10 | warte | 3 s | — | — |
| 11 | `mixer.getPeaks` | `{"track": 1}` | `{"name": "Insert 1", "peak_left": 0.4872153699398041, "peak_max": 0.4872153699398041, "peak_right": 0.4683002531528473, "success": true, "track": 1}` | ok |
| 12 | `mixer.getPeaks` | `{"track": 0}` | `{"name": "Master", "peak_left": 0.48372113704681396, "peak_max": 0.4920049011707306, "peak_right": 0.4920049011707306, "success": true, "track": 0}` | ok |
| 13 | `transport.getStatus` | `{}` | `{"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "3:16:07", "success": true}` | ok |
| 14 | `transport.stop` | `{}` | `{"stopped": true, "success": true}` | ok |

**Ergebnis:** 14 von 14 Schritten bestanden.


<!-- szenario.py 2026-09-15T08:35:50+00:00 -->
## Szenario `fenster` — Das FL-Hauptfenster und das Fenster von Nakama Probeeq werden ohne Vordergrundwechsel als Bild erfasst; ein einfarbiges Bild ist verfehlt

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `{"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `lokal.fenster` | `{"ziel": "fl"}` | {"breite": 1920, "dauer_s": 0.125, "einfarbig": false, "fenster": "Nakama-Diagnose.flp - FL Studio 2026", "hoehe": 1032, "klasse": "TFruityLoopsMainForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\7e39db9a-fl-20260915-103550.png", "sha256": "C7944C10A9ED564B14E5FB07A8B40CE70B46D9DDFC66C49296BC26004578B534"} | ok (Details unten) |
| 3 | `mixer.focusEditor` | `{"slot_index": 0, "track": 1}` | `{"focused": true, "plugin": "Nakama Probeeq", "slot_index": 0, "success": true, "track": 1}` | ok |
| 4 | warte | 1 s | — | — |
| 5 | `lokal.fenster` | `{"plugin": "Nakama Probeeq", "ziel": "plugin"}` | {"breite": 67, "dauer_s": 0.078, "einfarbig": false, "fenster": "Nakama Probeeq (Insert 1)", "hoehe": 31, "klasse": "TPluginForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\7e39db9a-plugin-20260915-103551.png", "sha256": "CA1B55FF171215B781484FEF1A268967ADE327B180C380B769D72CEEC4AB91F1"} · VORAUSSETZUNG: eingeklappt: 67 x 31 Pixel (Plugin-Fenster unter 200 x 100; Handgriff K-286-2, Karte U43) | VORAUSSETZUNG (Details unten) |

**Ergebnis:** 4 von 5 Schritten bestanden.


### Schritt 2 `lokal.fenster`

- Antwort {"breite": 1920, "dauer_s": 0.125, "einfarbig": false, "fenster": "Nakama-Diagnose.flp - FL Studio 2026", "hoehe": 1032, "klasse": "TFruityLoopsMainForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\7e39db9a-fl-20260915-103550.png", "sha256": "C7944C10A9ED564B14E5FB07A8B40CE70B46D9DDFC66C49296BC26004578B534"}

### Schritt 5 `lokal.fenster`

- Antwort {"breite": 67, "dauer_s": 0.078, "einfarbig": false, "fenster": "Nakama Probeeq (Insert 1)", "hoehe": 31, "klasse": "TPluginForm", "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\bilder\\7e39db9a-plugin-20260915-103551.png", "sha256": "CA1B55FF171215B781484FEF1A268967ADE327B180C380B769D72CEEC4AB91F1"}

<!-- szenario.py 2026-09-15T08:35:52+00:00 -->
## Szenario `nulltest-host` — Der Render des Diagnoseprojekts im Auslieferungszustand ist bitidentisch zur Quelle (Weg R1); Render mit Verarbeitung und ohne Slots brauchen Karte U43

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `{"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `{"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `{"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok · roh: volume=0.78125; volume_db=-5.176800727844238 |
| 4 | `mixer.getTrackInfo` | `{"track": 1}` | `{"color": "-0xbdbdc0", "index": 1, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Insert 1", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume=0.7999999998137355; volume_db=0.0 |
| 5 | `mixer.getTrackInfo` | `{"track": 0}` | `{"color": "-0xbdbdc0", "index": 0, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Master", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume=0.7999999998137355; volume_db=0.0 |
| 6 | `mixer.getRouting` | `{"track": 1}` | `{"routes": [{"dest": 0, "dest_name": "Master", "level": 0.8, "source": 1, "source_name": "Insert 1"}], "scanned": 1, "success": true, "track_count": 18}` | ok · roh: routes.*.dest=[0]; routes.*.level=[0.8] |
| 7 | `lokal.nulltest` | `{"vergleich": "auslieferung"}` | BITIDENTISCH v=0 g=1.0 Abweichungen=0 | ok (Details unten) |
| 8 | `lokal.nulltest` | `{"projekt": "Nakama-Diagnose-Verarbeitung.flp", "vergleich": "verarbeitung_ein"}` | VORAUSSETZUNG verarbeitung_ein: Referenzprojekt fehlt (Karte U43, K-286-1) | VORAUSSETZUNG (Details unten) |
| 9 | `lokal.nulltest` | `{"projekt": "Nakama-Diagnose-Referenz.flp", "vergleich": "ohne_slots"}` | VORAUSSETZUNG ohne_slots: Referenzprojekt fehlt (Karte U43, K-286-1) | VORAUSSETZUNG (Details unten) |

**Ergebnis:** 7 von 9 Schritten bestanden.


### Schritt 7 `lokal.nulltest`

- `nulltest.py` Exit 0, Quelle `C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav`, Songlaenge 45696 ms
  - NULLTEST Urteil BITIDENTISCH · Exit 0 · N 2015193 · v 0 · abweichungen 0 · g 1.0 · g_db 0.0 · ueberlappend 2015193 Frames [0, 2015193) · sha256_render_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · sha256_quelle_bereich 6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717 · Stichprobenspitzen Datei {'render': [0.3984098434448242, 0.4039571285247803], 'quelle': [0.3984098434448242, 0.4039571285247803]} · Stichprobenspitzen Bereich {'render': [0.3984098434448242, 0.4039571285247803], 'quelle': [0.3984098434448242, 0.4039571285247803]} · Render {'art': 'PCM', 'bits': 24, 'kanaele': 2, 'rate': 44100, 'frames': 2015409} SHA-256 FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A
- ergebnis.json: `{"N": 2015193, "abweichungen": 0, "erste": null, "erzeugt_utc": "2026-09-15T08:35:54+00:00", "exit": 0, "format": "nakama.laufzeit.nulltest.v1", "g": 1.0, "g_db": 0.0, "hinweis": "Offline-Latenz des Plugins 0 (Kanon A1/A16); Clip-Lage oder Latenz trennt nur Weg R2 (Karte U43)", "letzte": null, "quelle": {"format": {"art": "PCM", "bits": 24, "frames": 5470096, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\Projekte\\Nakama\\eq-copilot\\kalibration\\Testtrack.wav", "sha256_datei": "D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF"}, "render": {"format": {"art": "PCM", "bits": 24, "frames": 2015409, "kanaele": 2, "rate": 44100}, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "sha256_datei": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A"}, "renderstatus": {"aufruf": "C:\\Program Files\\Image-Line\\FL Studio 2026\\FL64.exe /R /Ewav /O\"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\" \"C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose.flp\"", "datei": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\Nakama-Diagnose.wav", "dauer_s": 4.1, "exit": 0, "fenstertitel": "", "format": "nakama.laufzeit.render.v1", "grund": null, "head": "7e39db9a", "pid": 24532, "projekt": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\projekt\\Nakama-Diagnose.flp", "sha256": "FAE3676F98AF2575A6806873676CE616279D62F6E2BB246BF3ABB3B61FF8C87A", "zeit": "2026-09-15 10:35:35"}, "sha256_quelle_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "sha256_render_bereich": "6F9AA3E5C970D217DEA37750F3B940D05BCB767D3663D823C4B3A71770C96717", "songlaenge_ms": 45696, "spitzen_bereich": {"quelle": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "spitzen_datei": {"quelle": [0.3984098434448242, 0.4039571285247803], "render": [0.3984098434448242, 0.4039571285247803]}, "ueberlappend": {"bis": 2015193, "frames": 2015193, "von": 0}, "urteil": "BITIDENTISCH", "v": 0}`

### Schritt 8 `lokal.nulltest`

- erwartet: `C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\fl\Nakama-Diagnose-Verarbeitung.flp` - ohne das Projekt kein Render dieses Zustands (M-64), nie still

### Schritt 9 `lokal.nulltest`

- erwartet: `C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\fl\Nakama-Diagnose-Referenz.flp` - ohne das Projekt kein Render dieses Zustands (M-64), nie still

<!-- szenario.py 2026-09-15T08:36:06+00:00 -->
## Szenario `snapshot-runde01` — Snapshot beider Rollen ueber den Briefkasten innerhalb eines Umlaufs gegen den Referenzausschnitt der Quelle; die Runde-01-Anker stehen nur als Rohvergleich daneben

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `{"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `{"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `channels.getInfo` | `{"index": 0, "use_global": false}` | `{"color": "-0x7f808a", "index": 0, "is_muted": false, "is_selected": true, "is_solo": false, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": true, "target_fx_track": 1, "volume": 0.78125, "volume_db": -5.176800727844238}` | ok · roh: volume=0.78125; volume_db=-5.176800727844238 |
| 4 | `mixer.getTrackInfo` | `{"track": 1}` | `{"color": "-0xbdbdc0", "index": 1, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Insert 1", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume_db=0.0 |
| 5 | `mixer.getTrackInfo` | `{"track": 0}` | `{"color": "-0xbdbdc0", "index": 0, "is_armed": false, "is_muted": false, "is_solo": false, "name": "Master", "pan": 0.0, "stereo_separation": 0.0, "success": true, "volume": 0.7999999998137355, "volume_db": 0.0}` | ok · roh: volume_db=0.0 |
| 6 | `mixer.getRouting` | `{"track": 1}` | `{"routes": [{"dest": 0, "dest_name": "Master", "level": 0.8, "source": 1, "source_name": "Insert 1"}], "scanned": 1, "success": true, "track_count": 18}` | ok · roh: routes.*.level=[0.8] |
| 7 | `transport.setLoopMode` | `{"mode": "song"}` | `{"mode": "song", "success": true}` | ok |
| 8 | `transport.stop` | `{}` | `{"stopped": true, "success": true}` | ok |
| 9 | `transport.setPosition` | `{"mode": 0, "position": 0}` | `{"mode": 0, "requested_position": 0, "success": true}` | ok |
| 10 | `transport.getPosition` | `{}` | `{"hint": "1:01:00", "ms": 0, "seconds": 0, "success": true}` | ok · roh: ms=0; hint="1:01:00" |
| 11 | `lokal.briefkasten` | `{"rollen": ["gen", "probeeq", "broker"]}` | beantwortet: gen, probeeq | ok (Details unten) |
| 12 | `transport.start` | `{}` | `{"is_playing": true, "success": true}` | ok |
| 13 | `lokal.umlauf` | `{"rollen": ["gen", "probeeq"], "vergleich": {"baender": [{"anker": "schwer", "delta_k": true, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "plus_s_lufs": true, "rechnung": "F-28 LUFS integriert: R_LUFS + Delta_K +- (0,07 LU + S_LUFS) ueber den Referenzausschnitt des schweren Ankers n_S; 0,07 LU KALIBRIER-PROTOKOLL.md:71", "referenz": "lufs", "rollen": ["gen", "probeeq"], "toleranz…` | gemessen: gen `aa201f68`, probeeq `aa201f68` | ok (Details unten) |

**Ergebnis:** 13 von 13 Schritten bestanden.


### Schritt 11 `lokal.briefkasten`

- Anfrage `7856209da76011fefc83a8387e20e317`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer None → None ms
  - broker: Broker laeuft nicht - keine Erwartung an die Rolle broker
  - gen: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (24047 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.89 s nach dem Schreiben)
  - probeeq: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (15916 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.94 s nach dem Schreiben)
  - gen: snapshot.zustand "sammelt", frame.frames_gebaut 103
  - probeeq: snapshot.zustand "sammelt", frame.frames_gebaut 113

### Schritt 13 `lokal.umlauf`

- Nulltesturteil und Kettenverschiebung: {"delta_k_db": 0.0, "g": 1.0, "urteil": "BITIDENTISCH", "v": 0}

| # | Kennung | p_vor ms | p_nach ms | gen: zustand · aktiv s · gesamt s · E · schwer s | probeeq: zustand · aktiv s · gesamt s · E · schwer s |
|---|---|---|---|---|---|
| 1 | `3c99e9c5` | 0 | 1719 | "sammelt" · 0.6 · 0.633922902494331 · 27956 · 0.584829931972789 | "sammelt" · 0.7 · 0.727687074829932 · 32091 · 0.727687074829932 |
| 2 | `5be38cb3` | 1763 | 3714 | "sammelt" · 2.6 · 2.629455782312925 · 115959 · 2.441972789115646 | "sammelt" · 2.6 · 2.691972789115646 · 118716 · 2.691972789115646 |
| 3 | `7855b1bb` | 3763 | 5714 | "sammelt" · 4.5 · 4.580362811791383 · 201994 · 4.535714285714286 | "sammelt" · 4.7 · 4.718752834467121 · 208097 · 4.718752834467121 |
| 4 | `84e64102` | 5759 | 7710 | "sammelt" · 6.600000000000001 · 6.624988662131519 · 292162 · 6.392857142857143 | "sammelt" · 6.7 · 6.714285714285714 · 296100 · 6.714285714285714 |
| 5 | `2c4db809` | 7710 | 9705 | "sammelt" · 8.6 · 8.620544217687074 · 380166 · 8.526780045351474 | "sammelt" · 8.700000000000001 · 8.727687074829932 · 384891 · 8.727687074829932 |
| 6 | `6834a321` | 9705 | 11701 | "sammelt" · 10.600000000000001 · 10.616077097505668 · 468169 · 10.383922902494332 | "sammelt" · 10.700000000000001 · 10.70981859410431 · 472303 · 10.70981859410431 |
| 7 | `08790edf` | 11750 | 13701 | "sammelt" · 12.600000000000001 · 12.616077097505668 · 556369 · 12.522312925170068 | "sammelt" · 12.700000000000001 · 12.705351473922903 · 560306 · 12.705351473922903 |
| 8 | `656c3d27` | 13746 | 15741 | "sammelt" · 14.600000000000001 · 14.65625850340136 · 646341 · 14.65625850340136 | "sammelt" · 14.700000000000001 · 14.71875283446712 · 649097 · 14.71875283446712 |
| 9 | `04d95b33` | 15741 | 17741 | "messbereit" · 16.6 · 16.607142857142858 · 732375 · 16.51340136054422 | "messbereit" · 16.7 · 16.714285714285715 · 737100 · 16.714285714285715 |
| 10 | `96ad80db` | 17741 | 19737 | "messbereit" · 18.6 · 18.607142857142858 · 820575 · 18.607142857142858 | "messbereit" · 18.7 · 18.714285714285715 · 825300 · 18.714285714285715 |
| 11 | `75f0d15e` | 19737 | 21732 | "messbereit" · 20.6 · 20.64732426303855 · 910547 · 20.464285714285715 | "messbereit" · 20.700000000000003 · 20.70981859410431 · 913303 · 20.70981859410431 |
| 12 | `73701f4c` | 21732 | 23732 | "messbereit" · 22.6 · 22.64732426303855 · 998747 · 22.598208616780045 | "messbereit" · 22.700000000000003 · 22.705351473922903 · 1001306 · 22.705351473922903 |
| 13 | `5fbe4a69` | 23732 | 25728 | "messbereit" · 24.6 · 24.642857142857142 · 1086750 · 24.455351473922903 | "messbereit" · 24.700000000000003 · 24.705351473922903 · 1089506 · 24.705351473922903 |
| 14 | `e016af62` | 25728 | 27723 | "messbereit" · 26.6 · 26.638390022675736 · 1174753 · 26.544648526077097 | "messbereit" · 26.700000000000003 · 26.700884353741497 · 1177509 · 26.700884353741497 |
| 15 | `cf142a4b` | 27772 | 29723 | "messbereit" · 28.6 · 28.63392290249433 · 1262756 · 28.450884353741497 | "messbereit" · 28.700000000000003 · 28.72768707482993 · 1266891 · 28.72768707482993 |
| 16 | `4a0e2f31` | 29768 | 31719 | "messbereit" · 30.6 · 30.678571428571427 · 1352925 · 30.54018140589569 | "messbereit" · 30.700000000000003 · 30.72768707482993 · 1355091 · 30.72768707482993 |
| 17 | `0f05d06d` | 31763 | 33714 | "messbereit" · 32.6 · 32.629455782312924 · 1438959 · 32.39732426303855 | "messbereit" · 32.7 · 32.72321995464853 · 1443094 · 32.72321995464853 |
| 18 | `a5e507a0` | 33763 | 35714 | "messbereit" · 34.6 · 34.62498866213152 · 1526962 · 34.535714285714285 | "messbereit" · 34.7 · 34.736598639455785 · 1531884 · 34.736598639455785 |
| 19 | `b430f1a9` | 35759 | 37710 | "messbereit" · 36.6 · 36.66963718820862 · 1617131 · 36.66963718820862 | "messbereit" · 36.7 · 36.71875283446712 · 1619297 · 36.71875283446712 |
| 20 | `3ac44a6b` | 37754 | 39705 | "messbereit" · 38.6 · 38.66517006802721 · 1705134 · 38.52678004535147 | "messbereit" · 38.7 · 38.72768707482993 · 1707891 · 38.72768707482993 |
| 21 | `e7a2a12e` | 39754 | 41701 | "messbereit" · 40.6 · 40.616077097505666 · 1791169 · 40.616077097505666 | "messbereit" · 40.7 · 40.709818594104306 · 1795303 · 40.709818594104306 |
| 22 | `e2347ddd` | 41750 | 43701 | "messbereit" · 42.6 · 42.66072562358276 · 1881338 · 42.47321995464853 | "messbereit" · 42.7 · 42.7544671201814 · 1885472 · 42.7544671201814 |
| 23 | `aa201f68` | 43746 | 45 | "messbereit" · 44.6 · 44.65625850340136 · 1969341 · 44.611609977324264 | "messbereit" · 44.7 · 44.71875283446712 · 1972097 · 44.71875283446712 |

- Ende der Folge: wrap; 23 Anfragen, 46 Positionen
- Anfrage `3c99e9c5433491d0820a4b1b18d7a5d4`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 0 → 1719 ms
  - gen: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23744 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.64 s nach dem Schreiben)
  - probeeq: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23665 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.69 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `5be38cb3e3839371d4ab78da935bd50c`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 1763 → 3714 ms
  - gen: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23679 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23635 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `7855b1bb88299136775fbdbefa949b79`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 3763 → 5714 ms
  - gen: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23684 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23590 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `84e64102eb3fdcb316eb998a720335a9`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 5759 → 7710 ms
  - gen: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23689 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23651 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.94 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `2c4db8091833adbfac13235a018af234`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 7710 → 9705 ms
  - gen: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23708 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23687 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `6834a321c4aea60c6e927446f7282801`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 9705 → 11701 ms
  - gen: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23760 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23688 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `08790edff42ea8404d212cb87d6ef6e9`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 11750 → 13701 ms
  - gen: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23746 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23710 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `656c3d27de06c607c4d675314a4b0284`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 13746 → 15741 ms
  - gen: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23746 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23707 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `04d95b33dee0f42a4e39b6fffa0a12d2`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 15741 → 17741 ms
  - gen: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26890 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24636 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `96ad80dbfe1546580dd0a9f510f2a436`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 17741 → 19737 ms
  - gen: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26669 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24464 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `75f0d15e7c818c1af4c6a8c797e85c16`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 19737 → 21732 ms
  - gen: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26564 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24320 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `73701f4c9a54b7b5b872b3637129722f`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 21732 → 23732 ms
  - gen: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26439 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24170 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `5fbe4a69aa12791a16f8537ca7abf8df`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 23732 → 25728 ms
  - gen: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26428 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24151 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `e016af620a7d9cf8d3ab125f9a758730`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 25728 → 27723 ms
  - gen: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26435 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24156 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `cf142a4b41ce7c7d175950be7fefdb29`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 27772 → 29723 ms
  - gen: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26438 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24157 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `4a0e2f318e62c7b4672f09784c406ac2`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 29768 → 31719 ms
  - gen: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26419 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24149 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `0f05d06d293292f015d85a893059e29f`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 31763 → 33714 ms
  - gen: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26433 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24162 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `a5e507a05c786a53e23e707c7880da2e`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 33763 → 35714 ms
  - gen: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26435 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24161 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `b430f1a9200ac656db993ab0f2a1b8a7`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 35759 → 37710 ms
  - gen: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26441 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24146 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `3ac44a6b5919ef355855d03f8470ba30`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 37754 → 39705 ms
  - gen: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26452 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24156 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `e7a2a12ed75296b619fb3db3a77bde7f`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 39754 → 41701 ms
  - gen: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26433 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24169 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `e2347ddd804d1c0e64725c942a5dfb6f`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 41750 → 43701 ms
  - gen: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26453 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.91 s nach dem Schreiben)
  - probeeq: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24168 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `aa201f681617a4f78085baceff6cf225`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 43746 → 45 ms
  - gen: `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (26445 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24173 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `04d95b33dee0f42a4e39b6fffa0a12d2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `08790edff42ea8404d212cb87d6ef6e9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `0f05d06d293292f015d85a893059e29f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2c4db8091833adbfac13235a018af234.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3ac44a6b5919ef355855d03f8470ba30.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `3c99e9c5433491d0820a4b1b18d7a5d4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4a0e2f318e62c7b4672f09784c406ac2.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5be38cb3e3839371d4ab78da935bd50c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5fbe4a69aa12791a16f8537ca7abf8df.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `656c3d27de06c607c4d675314a4b0284.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6834a321c4aea60c6e927446f7282801.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `73701f4c9a54b7b5b872b3637129722f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `75f0d15e7c818c1af4c6a8c797e85c16.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7855b1bb88299136775fbdbefa949b79.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `7856209da76011fefc83a8387e20e317.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `84e64102eb3fdcb316eb998a720335a9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `96ad80dbfe1546580dd0a9f510f2a436.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `a5e507a05c786a53e23e707c7880da2e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b430f1a9200ac656db993ab0f2a1b8a7.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `cf142a4b41ce7c7d175950be7fefdb29.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e016af620a7d9cf8d3ab125f9a758730.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e2347ddd804d1c0e64725c942a5dfb6f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `e7a2a12ed75296b619fb3db3a77bde7f.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- gen: gewertet `aa201f681617a4f78085baceff6cf225.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (p_vor 43746 ms), Anker {"bloecke_max_samples": 197, "e": 1969341, "fortlaufend": 1969341, "k": 0, "leicht": [0, 1969341], "n_l": 1969341, "n_s": 1967372, "rate": 44100.0, "s0": 0, "schwer": [0, 1967372], "stillstand": 0}
- probeeq: gewertet `aa201f681617a4f78085baceff6cf225.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (p_vor 43746 ms), Anker {"bloecke_max_samples": 197, "e": 1972097, "fortlaufend": 1972097, "k": 0, "leicht": [0, 1972097], "n_l": 1972097, "n_s": 1972097, "rate": 44100.0, "s0": 0, "schwer": [0, 1972097], "stillstand": 0}
- Rechnung F-28 (221.7 s): `{"analyze": "C:\\Users\\phili\\FL-Studio\\tools\\analyze-track.py", "anker": [{"analyze_s": 8.9, "art": "leicht", "ausschnitt": [0, 1969341], "centroid_mag": 828.501939386266, "corr": 0.5038671704454135, "low_frac": 0.781279697394338, "low_frac_kanal": 0.7590869167154394, "lufs": -22.390984273301687, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-7e39db9a-0-1969341.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "sha256": "53AA875B05BB061019C12C2B64B0B5880A66A1C77CC51FAE66B7642AB3A7A237", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.33041779769467955}, {"analyze_s": 2.84, "art": "schwer", "ausschnitt": [0, 1967372], "centroid_mag": 828.501939386266, "corr": 0.5038705227494916, "low_frac": 0.781279697394338, "low_frac_kanal": 0.7590869167154394, "lufs": -22.385016854426702, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-7e39db9a-0-1967372.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "s_lufs": {"arbeiter": 12, "dauer_s": 103.29, "max": -22.37887414486701, "min": -22.38503200234511, "nicht_endlich": 0, "phi0": -22.385016854426702, "phi_max": 4390, "phi_min": 1293, "spanne": 0.0061578574781009365, "versaetze": 4410}, "sha256": "7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.3304148640349438}, {"analyze_s": 2.86, "art": "leicht", "ausschnitt": [0, 1972097], "centroid_mag": 828.4764882043261, "corr": 0.5038841426697283, "low_frac": 0.7811472879206683, "low_frac_kanal": 0.7589359094222629, "lufs": -22.390860551570047, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-7e39db9a-0-1972097.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "sha256": "B492D1BE4AF278DF9DD19195326FA580F2A59C9C6A993A393A859B3A3812B7BF", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.33040295565684347}, {"analyze_s": 2.86, "art": "schwer", "ausschnitt": [0, 1972097], "centroid_mag": 828.4764882043261, "corr": 0.5038841426697283, "low_frac": 0.7811472879206683, "low_frac_kanal": 0.7589359094222629, "lufs": -22.390860551570047, "pfad": "C:\\Users\\phili\\AppData\\Local\\evenacadia\\nakama-laufzeit\\render\\ausschnitt-7e39db9a-0-1972097.wav", "resonanzen": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "s_lufs": {"arbeiter": 12, "dauer_s": 102.94, "max": -22.384600803113845, "min": -22.391770034096634, "nicht_endlich": 0, "phi0": -22.390860551570047, "phi_max": 3139, "phi_min": 3002, "spanne": 0.007169230982789543, "versaetze": 4410}, "sha256": "B492D1BE4AF278DF9DD19195326FA580F2A59C9C6A993A393A859B3A3812B7BF", "stichprobenspitze": [0.3984098434448242, 0.4039571285247803], "tp_dbtp": -7.867515649360563, "width": 0.33040295565684347}], "dauer_s": 221.27, "quelle_sha256": "D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF", "u_unten": [{"delta_db": 4.2536484549686325e-12, "eps": 9.79438752324313e-13, "frames_ausserhalb_quelle": 0, "max_zellen": 446, "min_zellen": 445, "n_l": 1969341, "phi_min": 2482, "rolle": "gen", "s0": 0, "schwelle_db": -59.999999999995744, "u_unten_s": 44.5, "zellen_phi0": 446}, {"delta_db": 4.2536484549686325e-12, "eps": 9.79438752324313e-13, "frames_ausserhalb_quelle": 0, "max_zellen": 447, "min_zellen": 446, "n_l": 1972097, "phi_min": 828, "rolle": "probeeq", "s0": 0, "schwelle_db": -59.999999999995744, "u_unten_s": 44.6, "zellen_phi0": 447}]}`
- Referenzausschnitt gen leicht: Frames [0, 1969341), K 0, v 0, SHA-256 53AA875B05BB061019C12C2B64B0B5880A66A1C77CC51FAE66B7642AB3A7A237, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-7e39db9a-0-1969341.wav
- Referenzausschnitt gen schwer: Frames [0, 1967372), K 0, v 0, SHA-256 7A0EA739EE2FE94791E29AFC948119E6700F2EBD7069B1B288EF812F0606E34A, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-7e39db9a-0-1967372.wav
- Referenzausschnitt probeeq leicht: Frames [0, 1972097), K 0, v 0, SHA-256 B492D1BE4AF278DF9DD19195326FA580F2A59C9C6A993A393A859B3A3812B7BF, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-7e39db9a-0-1972097.wav
- Referenzausschnitt probeeq schwer: Frames [0, 1972097), K 0, v 0, SHA-256 B492D1BE4AF278DF9DD19195326FA580F2A59C9C6A993A393A859B3A3812B7BF, Datei C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\render\ausschnitt-7e39db9a-0-1972097.wav
- gen: U_unten 44.5 s = 0,1 s x min Z_phi (phi 2482; phi 0: 446 Zellen; Schwelle -59.999999999995744 dB, Abstand 4.2536484549686325e-12 dB), aktiv_sekunden 44.6
- gen roh `snapshot.spektral.centroid_mag_hz` = [886.6096345840435]
- gen roh `snapshot.loudness.lufs_integriert` = [-22.385016854426723]
- gen roh `snapshot.loudness.true_peak_dbtp` = [-7.867515515804746]
- gen roh `snapshot.stereo.width` = [0.330414864035028]
- gen roh `snapshot.stereo.corr` = [0.503870522747712]
- gen roh `snapshot.spektral.low_frac` = [0.759086918280695]
- gen roh `snapshot.resonanzen.*.freq_hz` = [688.6962107092802, 459.64957473531535, 919.2991494706307]
- gen roh `snapshot.zustand` = ["messbereit"]
- gen roh `snapshot.aktiv_sekunden` = [44.6]
- gen roh `snapshot.gesamt_sekunden` = [44.65625850340136]
- gen roh `frame.schwer_sekunden` = [44.611609977324264]
- gen roh `frame.material_ende_projektsample` = [1969341]
- gen roh `frame.hostzeit_fortlaufend_samples` = [1969341]
- gen roh `frame.hostzeit_stillstand_bloecke` = [0]
- gen roh `frame.bloecke_max_samples` = [197]
- probeeq: U_unten 44.6 s = 0,1 s x min Z_phi (phi 828; phi 0: 447 Zellen; Schwelle -59.999999999995744 dB, Abstand 4.2536484549686325e-12 dB), aktiv_sekunden 44.7
- probeeq roh `snapshot.spektral.centroid_mag_hz` = [886.5780462091899]
- probeeq roh `snapshot.loudness.lufs_integriert` = [-22.39086055157007]
- probeeq roh `snapshot.loudness.true_peak_dbtp` = [-7.867515515804746]
- probeeq roh `snapshot.stereo.width` = [0.330402955656926]
- probeeq roh `snapshot.stereo.corr` = [0.50388414266794]
- probeeq roh `snapshot.spektral.low_frac` = [0.758935910980525]
- probeeq roh `snapshot.resonanzen.*.freq_hz` = [688.6962107092802, 459.64957473531535, 919.2991494706307]
- probeeq roh `snapshot.zustand` = ["messbereit"]
- probeeq roh `snapshot.aktiv_sekunden` = [44.7]
- probeeq roh `snapshot.gesamt_sekunden` = [44.71875283446712]
- probeeq roh `frame.schwer_sekunden` = [44.71875283446712]
- probeeq roh `frame.material_ende_projektsample` = [1972097]
- probeeq roh `frame.hostzeit_fortlaufend_samples` = [1972097]
- probeeq roh `frame.hostzeit_stillstand_bloecke` = [0]
- probeeq roh `frame.bloecke_max_samples` = [197]
- Band LUFS gen: {"ausschnitt": [0, 1967372], "breite": 0.07615785747810094, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "mitte": -22.385016854426702, "referenz": -22.385016854426702, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": -22.385016854426723}
- Band TP gen: {"feld": "snapshot.loudness.true_peak_dbtp", "grund": "kein Band fuer diese Rolle", "kurz": "TP", "rolle": "gen", "status": "roh", "wert": -7.867515515804746}
- Band width gen: {"ausschnitt": [0, 1967372], "breite": 0.01, "feld": "snapshot.stereo.width", "kurz": "width", "mitte": 0.3304148640349438, "referenz": 0.3304148640349438, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.330414864035028}
- Band corr gen: {"ausschnitt": [0, 1967372], "breite": 0.01, "feld": "snapshot.stereo.corr", "kurz": "corr", "mitte": 0.5038705227494916, "referenz": 0.5038705227494916, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.503870522747712}
- Band low_frac gen: {"ausschnitt": [0, 1967372], "breite": 0.02, "feld": "snapshot.spektral.low_frac", "kurz": "low_frac", "mitte": 0.7590869167154394, "referenz": 0.7590869167154394, "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": 0.759086918280695}
- Band resonanzen gen: {"ausschnitt": [0, 1967372], "feld": "snapshot.resonanzen.*.freq_hz", "kurz": "resonanzen", "referenz": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "gen", "seite": "im Band", "status": "im Band", "wert": [688.6962107092802, 459.64957473531535, 919.2991494706307]}
- Band LUFS probeeq: {"ausschnitt": [0, 1972097], "breite": 0.07716923098278955, "feld": "snapshot.loudness.lufs_integriert", "kurz": "LUFS", "mitte": -22.390860551570047, "referenz": -22.390860551570047, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": -22.39086055157007}
- Band TP probeeq: {"ausschnitt": [0, 1972097], "breite": 0.12, "feld": "snapshot.loudness.true_peak_dbtp", "kurz": "TP", "mitte": -7.867515649360563, "referenz": -7.867515649360563, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": -7.867515515804746}
- Band width probeeq: {"ausschnitt": [0, 1972097], "breite": 0.01, "feld": "snapshot.stereo.width", "kurz": "width", "mitte": 0.33040295565684347, "referenz": 0.33040295565684347, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.330402955656926}
- Band corr probeeq: {"ausschnitt": [0, 1972097], "breite": 0.01, "feld": "snapshot.stereo.corr", "kurz": "corr", "mitte": 0.5038841426697283, "referenz": 0.5038841426697283, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.50388414266794}
- Band low_frac probeeq: {"ausschnitt": [0, 1972097], "breite": 0.02, "feld": "snapshot.spektral.low_frac", "kurz": "low_frac", "mitte": 0.7589359094222629, "referenz": 0.7589359094222629, "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": 0.758935910980525}
- Band resonanzen probeeq: {"ausschnitt": [0, 1972097], "feld": "snapshot.resonanzen.*.freq_hz", "kurz": "resonanzen", "referenz": [546.6, 919.3, 354.4, 459.6, 229.8, 688.7], "rolle": "probeeq", "seite": "im Band", "status": "im Band", "wert": [688.6962107092802, 459.64957473531535, 919.2991494706307]}
- Rohvergleich Runde 01 (nur daneben, F-28): {"centroid_offline": 829, "centroid_plugin": [876, 879], "corr": [0.5, 0.507], "low_frac_offline": 0.78, "lufs_offline": -22.41, "lufs_plugin": [-22.45, -22.48], "quelle": "eq-copilot/kalibration/KALIBRIER-PROTOKOLL.md:68-69", "resonanzen_offline": [229.8, 354.4, 459.6, 546.6, 688.7, 919.3], "resonanzen_plugin": [688.7, 459.6, 919.3], "tp_offline": -7.87, "tp_plugin": [-7.75, -6.47], "width": [0.33, 0.328]}

<!-- szenario.py 2026-09-15T08:40:36+00:00 -->
## Szenario `u40-aktivitaetsgate` — Karte U40 als Messung: Aktivitaet, Abdeckung, LUFS-Short, True Peak und gezaehlte Fenster und Rahmen je leiser Stelle des Testtracks - Rohdaten, keine Deutung

| # | Aktion | Parameter | Antwort | Urteil |
|---|---|---|---|---|
| 1 | `system.ping` | `{}` | `{"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": false, "program_title": "FL Studio 2026", "success": true}` | ok |
| 2 | `transport.getLength` | `{}` | `{"bars": 27, "milliseconds": 45696, "seconds": 46, "success": true, "ticks": 10236}` | ok · roh: milliseconds=45696 |
| 3 | `transport.setLoopMode` | `{"mode": "song"}` | `{"mode": "song", "success": true}` | ok |
| 4 | `lokal.stellen` | `{"rollen": ["gen", "probeeq"], "stellen": [{"bis_s": 3.692, "energie": {"anteil_66_40": 0.0, "max_db": -22.628, "min_db": -27.205, "mittel_db": -25.205, "zellen": 36}, "id": "S1", "name": "Intro", "takte": "1-2", "von_s": 0.0}, {"bis_s": 11.077, "energie": {"anteil_66_40": 0.0, "max_db": -22.147, "min_db": -27.558, "mittel_db": -25.112, "zellen": 73}, "id": "S2", "name": "ruhige Passage", "takte"…` | gemessen, Rohdaten unten | ok (Details unten) |

**Ergebnis:** 4 von 4 Schritten bestanden.


### Schritt 4 `lokal.stellen`

- Quelle `C:\Users\phili\Projekte\Nakama\eq-copilot\kalibration\Testtrack.wav` SHA-256 D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF, erwartet D8B851CCF27E8F60B877D4BE3555184F2D5D6A4E7AA047482E4A694ED1F1D0BF
- Versatz v = 0 Frames; Nulltest {"delta_k_db": 0.0, "g": 1.0, "urteil": "BITIDENTISCH", "v": 0}

#### Stelle S1 Intro: Quelle [0.0, 3.692) s, Host [0, 162817) Frames
- Stopp `{"stopped": true, "success": true}`; setPosition 0 ms `{"mode": 0, "requested_position": 0, "success": true}`; getPosition roh `{"hint": "1:01:00", "ms": 0, "seconds": 0, "success": true}` nach 0.032 s
- Phasenmessung (nicht gezaehlt) `8b1b4754`: je Rolle {"gen": 0.74, "probeeq": 1.764} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.765, "probeeq": 1.781} s; gemeinsamer Zeitplan: 8 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle; getrennte Durchlaeufe je Rolle
- Durchlauf gen: Zeitplan erste Anfrage -0.79 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.959 s
- Durchlauf gen: Start `{"is_playing": true, "success": true}` nach 0.031 s (geplant ab +0.002 s), erste Anfrage geplant -0.79 s zum Start
- Durchlauf gen: Ende der Anfragen: Stellenende erreicht; Stopp `{"stopped": true, "success": true}`
- Durchlauf probeeq: Stopp `{"stopped": true, "success": true}`; setPosition 0 ms `{"mode": 0, "requested_position": 0, "success": true}`; getPosition roh `{"hint": "1:01:00", "ms": 0, "seconds": 0, "success": true}` nach 0.032 s
- Durchlauf probeeq: Zeitplan erste Anfrage -0.61 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.433 s
- Durchlauf probeeq: Start `{"is_playing": true, "success": true}` nach 0.032 s (geplant ab -0.010 s), erste Anfrage geplant -0.61 s zum Start
- Durchlauf probeeq: Ende der Anfragen: Stellenende erreicht; Stopp `{"stopped": true, "success": true}`
- Anfrage `8b1b47542c373ff7394dd76acd2b9718`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer None → None ms
  - gen: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (24225 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.76 s nach dem Schreiben)
  - probeeq: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (24176 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.78 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `089ce2f738c0b8a8dd28b30ce8aa3992`, erwartet {"gen": [29068]}, Positionsklammer 0 → 0 ms
  - gen: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (24224 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.83 s nach dem Schreiben)
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `82a9f707745328dd9e2ec1a4680c0933`, erwartet {"gen": [29068]}, Positionsklammer 188 → 696 ms
  - gen: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23682 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.51 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `6f359cf21cfd2d8d2ddd6961a76a5e9d`, erwartet {"gen": [29068]}, Positionsklammer 1161 → 1670 ms
  - gen: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23686 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.50 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `ece258a0683cfb5348aedeca464ba978`, erwartet {"gen": [29068]}, Positionsklammer 2228 → 2692 ms
  - gen: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23584 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.48 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `802b464bdca2b41a41ffd43b6f1e2718`, erwartet {"gen": [29068]}, Positionsklammer 3205 → 3670 ms
  - gen: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23707 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.45 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `ab4aa456b3e8b02398b58280f3b9a2bc`, erwartet {"gen": [29068]}, Positionsklammer 4228 → 4692 ms
  - gen: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23709 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.45 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: Rolle probeeq ohne Erwartung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `d1661b7b76a610b2f5914a050f12ea56`, erwartet {"probeeq": [29068]}, Positionsklammer 0 → 1254 ms
  - probeeq: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (22466 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.88 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: Rolle gen ohne Erwartung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `848fea379c417effc9d375dc205fb752`, erwartet {"probeeq": [29068]}, Positionsklammer 1299 → 3299 ms
  - probeeq: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23644 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: Rolle gen ohne Erwartung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `6ccf1613a7e57b139eeab0c3e210a584`, erwartet {"probeeq": [29068]}, Positionsklammer 3299 → 5246 ms
  - probeeq: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23653 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: Rolle gen ohne Erwartung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S1 | gen | getrennt | 16/16 | `089ce2f7` | Materialausschnitt null: zaehlt nie | false | 0.0 | null | null | null | null | null | null | 998 | 6067 | 0 | 6 | 2756 | 4528 | 0 | null | null | false | [0, 162817) v = 0 | 0 | 0 |
| S1 | gen | getrennt | 16/16 | `82a9f707` | gezaehlt | true | 1.0 | 1.0 | 0.771860361099243 | null | -17.36280632019043 | -17.36280632019043 | 2 | 1010 | 6092 | 0 | 0 | 2766 | 4528 | 0 | 0 | 26775 | true | [0, 162817) v = 0 | 188 | 696 |
| S1 | gen | getrennt | 16/16 | `6f359cf2` | gezaehlt | true | 1.0 | 1.0 | 0.831815540790558 | null | -14.675589561462402 | -14.676325798034668 | 2 | 1029 | 6111 | 2 | 2 | 2775 | 4528 | 0 | 40960 | 26568 | true | [0, 162817) v = 0 | 1161 | 1670 |
| S1 | gen | getrennt | 16/16 | `ece258a0` | gezaehlt | false | 1.0 | null | null | null | -16.465740203857422 | -16.4705867767334 | null | 1049 | 6131 | 4 | 4 | 2785 | 4528 | 0 | 104448 | 8361 | true | [0, 162817) v = 0 | 2228 | 2692 |
| S1 | gen | getrennt | 16/16 | `802b464b` | gezaehlt | false | 1.0 | null | null | -24.535213470458984 | -14.812978744506836 | -14.815757751464844 | null | 1069 | 6151 | 6 | 6 | 2795 | 4529 | 0 | 139264 | 18827 | true | [0, 162817) v = 0 | 3205 | 3670 |
| S1 | gen | getrennt | 16/16 | `ab4aa456` | Ausschnitt [180224, 203372) nicht ganz in [0, 162817) | true | 1.0 | 1.0 | 0.830727517604828 | -25.188722610473633 | -16.528411865234375 | -16.529401779174805 | 1 | 1096 | 6178 | 1 | 1 | 2805 | 4528 | 0 | 180224 | 23148 | true | [0, 162817) v = 0 | 4228 | 4692 |
| S1 | probeeq | getrennt | 16/16 | `d1661b7b` | gezaehlt | true | 1.0 | 1.0 | 0.836983025074005 | null | -14.173524856567383 | -14.179786682128906 | 2 | 1106 | 6232 | 0 | 0 | 2835 | 4528 | 0 | 24576 | 29958 | true | [0, 162817) v = 0 | 0 | 1254 |
| S1 | probeeq | getrennt | 16/16 | `848fea37` | gezaehlt | false | 1.0 | null | null | -24.89150047302246 | -14.225686073303223 | -14.230125427246094 | null | 1146 | 6272 | 3 | 3 | 2854 | 4528 | 0 | 122880 | 17492 | true | [0, 162817) v = 0 | 1299 | 3299 |
| S1 | probeeq | getrennt | 16/16 | `6ccf1613` | Ausschnitt [204800, 230934) nicht ganz in [0, 162817) | true | 1.0 | 1.0 | 0.851385593414307 | -25.235836029052734 | -15.158549308776855 | -15.159322738647461 | 2 | 1192 | 6318 | 0 | 0 | 2874 | 4528 | 0 | 204800 | 26134 | true | [0, 162817) v = 0 | 3299 | 5246 |

- Stelle S1 gen (durchlauf getrennt, kombinationen 16/16): {"antworten": 4, "d_frames_gebaut": 29, "d_summe_fenster_aktiv": 59, "d_summe_fenster_gesamt": 59, "erster_ausschnitt": [0, 26775], "letzter_ausschnitt": [139264, 158091]}
- Stelle S1 probeeq (durchlauf getrennt, kombinationen 16/16): {"antworten": 2, "d_frames_gebaut": 19, "d_summe_fenster_aktiv": 40, "d_summe_fenster_gesamt": 40, "erster_ausschnitt": [24576, 54534], "letzter_ausschnitt": [122880, 140372]}

#### Stelle S2 ruhige Passage: Quelle [3.692, 11.077) s, Host [162817, 488496) Frames
- Stopp `{"stopped": true, "success": true}`; setPosition 3692 ms `{"mode": 0, "requested_position": 3692, "success": true}`; getPosition roh `{"hint": "3:03:11", "ms": 3692, "seconds": 4, "success": true}` nach 0.016 s
- Phasenmessung (nicht gezaehlt) `44df32a8`: je Rolle {"gen": 0.637, "probeeq": 1.667} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.672, "probeeq": 1.688} s; gemeinsamer Zeitplan: 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle
- Durchlauf gen und probeeq: Zeitplan erste Anfrage -1.29 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 1.802 s
- Durchlauf gen und probeeq: Start `{"is_playing": true, "success": true}` nach 0.016 s (geplant ab -0.001 s), erste Anfrage geplant -1.29 s zum Start
- Durchlauf gen und probeeq: Ende der Anfragen: Stellenende erreicht; Stopp `{"stopped": true, "success": true}`
- Anfrage `44df32a883d2d0d937e72df33a488fbb`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer None → None ms
  - gen: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (15950 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.67 s nach dem Schreiben)
  - probeeq: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (15905 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.69 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `b08037a100e63b68d700f342c47d88f5`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 3692 → 4344 ms
  - gen: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (15966 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.30 s nach dem Schreiben)
  - probeeq: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (1287 B, Version '0.3.0', gruende ['noch_keine_messdaten', 'rolle_ohne_aggregat'], 1.89 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `fddf05e17c2d0e8df6c192ece5fbed1d`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 4344 → 6339 ms
  - gen: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23625 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23571 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
- Anfrage `d6bcf3bfce17f3d9a9d418dfce27685c`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 6339 → 8335 ms
  - gen: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23683 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.92 s nach dem Schreiben)
  - probeeq: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23637 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `dc1afa272482ebd1d857d34a9756f1b4`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 8335 → 10335 ms
  - gen: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23705 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23632 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `bb8781fcfbfa400947f12fdbfd084681`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer 10335 → 12281 ms
  - gen: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23674 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.94 s nach dem Schreiben)
  - probeeq: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23681 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.95 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S2 | gen | gemeinsam | 16/16 | `b08037a1` | Materialausschnitt null: zaehlt nie | true | 0.0 | 0.0 | null | null | null | null | 0 | 1207 | 6399 | 0 | 0 | 2913 | 4528 | 0 | null | null | false | [162817, 488496) v = 0 | 3692 | 4344 |
| S2 | gen | gemeinsam | 16/16 | `fddf05e1` | gezaehlt | false | 1.0 | null | null | null | -15.158549308776855 | -15.159322738647461 | null | 1236 | 6435 | 3 | 3 | 2932 | 4529 | 0 | 211969 | 18375 | true | [162817, 488496) v = 0 | 4344 | 6339 |
| S2 | gen | gemeinsam | 16/16 | `d6bcf3bf` | gezaehlt | false | 1.0 | null | null | -24.958499908447266 | -12.061075210571289 | -12.0626802444458 | null | 1276 | 6475 | 5 | 5 | 2951 | 4528 | 0 | 308225 | 8153 | true | [162817, 488496) v = 0 | 6339 | 8335 |
| S2 | gen | gemeinsam | 16/16 | `dc1afa27` | gezaehlt | false | 1.0 | null | null | -24.058902740478516 | -14.138407707214355 | -14.138729095458984 | null | 1322 | 6521 | 3 | 3 | 2971 | 4528 | 0 | 398337 | 8603 | true | [162817, 488496) v = 0 | 8335 | 10335 |
| S2 | gen | gemeinsam | 16/16 | `bb8781fc` | Ausschnitt [474113, 492778) nicht ganz in [162817, 488496) | false | 1.0 | null | null | -24.24235725402832 | -16.860294342041016 | -16.861051559448242 | null | 1362 | 6561 | 6 | 6 | 2990 | 4528 | 0 | 474113 | 18665 | true | [162817, 488496) v = 0 | 10335 | 12281 |
| S2 | probeeq | gemeinsam | 16/16 | `b08037a1` | gezaehlt | true | 1.0 | 1.0 | 0.812662422657013 | null | -17.7799129486084 | -17.7799129486084 | 1 | 1201 | 6415 | 2 | 2 | 2922 | 4528 | 0 | 162817 | 22245 | true | [162817, 488496) v = 0 | 3692 | 4344 |
| S2 | probeeq | gemeinsam | 16/16 | `fddf05e1` | gezaehlt | false | 1.0 | null | null | null | -15.495652198791504 | -15.495652198791504 | null | 1241 | 6455 | 5 | 5 | 2942 | 4528 | 0 | 267265 | 8360 | true | [162817, 488496) v = 0 | 4344 | 6339 |
| S2 | probeeq | gemeinsam | 16/16 | `d6bcf3bf` | gezaehlt | true | 1.0 | 1.0 | 0.824372112751007 | -24.563261032104492 | -16.149852752685547 | -16.151063919067383 | 2 | 1288 | 6502 | 1 | 1 | 2961 | 4528 | 0 | 334849 | 26810 | true | [162817, 488496) v = 0 | 6339 | 8335 |
| S2 | probeeq | gemeinsam | 16/16 | `dc1afa27` | gezaehlt | false | 1.0 | null | null | -24.64156150817871 | -16.245967864990234 | -16.254230499267578 | null | 1327 | 6541 | 5 | 5 | 2981 | 4536 | 0 | 433153 | 18872 | true | [162817, 488496) v = 0 | 8335 | 10335 |
| S2 | probeeq | gemeinsam | 16/16 | `bb8781fc` | Ausschnitt [515073, 538059) nicht ganz in [162817, 488496) | true | 1.0 | 1.0 | 0.769561052322388 | -25.157520294189453 | -12.947218894958496 | -12.947218894958496 | 1 | 1374 | 6588 | 1 | 1 | 3000 | 4554 | 0 | 515073 | 22986 | true | [162817, 488496) v = 0 | 10335 | 12281 |

- Stelle S2 gen (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 3, "d_frames_gebaut": 39, "d_summe_fenster_aktiv": 86, "d_summe_fenster_gesamt": 86, "erster_ausschnitt": [211969, 230344], "letzter_ausschnitt": [398337, 406940]}
- Stelle S2 probeeq (durchlauf gemeinsam, kombinationen 16/16): {"antworten": 4, "d_frames_gebaut": 59, "d_summe_fenster_aktiv": 126, "d_summe_fenster_gesamt": 126, "erster_ausschnitt": [162817, 185062], "letzter_ausschnitt": [433153, 452025]}

#### Stelle S3 Ausklang: Quelle [42.462, 45.596) s, Host [1872574, 2010784) Frames
- Stopp `{"stopped": true, "success": true}`; setPosition 42462 ms `{"mode": 0, "requested_position": 42462, "success": true}`; getPosition roh `{"hint": "25:13:07", "ms": 42460, "seconds": 42, "success": true}` nach 0.031 s
- Phasenmessung (nicht gezaehlt) `5ddf861e`: je Rolle {"gen": 0.657, "probeeq": 1.68} s nach dem Schreiben der Anfrage ({"gen": "Schreibzeit", "probeeq": "Schreibzeit"}), Eingang beim Runner {"gen": 0.656, "probeeq": 1.703} s; gemeinsamer Zeitplan: 4 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle; getrennte Durchlaeufe je Rolle
- Durchlauf gen: Zeitplan erste Anfrage -1.08 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.670 s
- Durchlauf gen: Start `{"is_playing": true, "success": true}` nach 0.032 s (geplant ab -0.002 s), erste Anfrage geplant -1.08 s zum Start
- Durchlauf gen: Ende der Anfragen: wrap; Stopp `{"stopped": true, "success": true}`
- Durchlauf probeeq: Stopp `{"stopped": true, "success": true}`; setPosition 42462 ms `{"mode": 0, "requested_position": 42462, "success": true}`; getPosition roh `{"hint": "25:13:07", "ms": 42460, "seconds": 42, "success": true}` nach 0.031 s
- Durchlauf probeeq: Zeitplan erste Anfrage -0.90 s zum Start, 16 von 16 Kombinationen des Zeitmodells tragen zwei Rahmen je Rolle, Marge 0.148 s
- Durchlauf probeeq: Start `{"is_playing": true, "success": true}` nach 0.016 s (geplant ab +0.003 s), erste Anfrage geplant -0.90 s zum Start
- Durchlauf probeeq: Ende der Anfragen: wrap; Stopp `{"stopped": true, "success": true}`
- Anfrage `5ddf861ec29880578e655e954351dd2a`, erwartet {"gen": [29068], "probeeq": [29068]}, Positionsklammer None → None ms
  - gen: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (15956 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.66 s nach dem Schreiben)
  - probeeq: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (15906 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.70 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `0851ddef6a6b0d2955b8183f5931fb37`, erwartet {"gen": [29068]}, Positionsklammer 42460 → 42509 ms
  - gen: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (15951 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.11 s nach dem Schreiben)
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `4a818767dd9f525ff3c593082e9fa6b6`, erwartet {"gen": [29068]}, Positionsklammer 42509 → 42879 ms
  - gen: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (22332 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.38 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `17f81b60b37c0444d8173e259c5e95ba`, erwartet {"gen": [29068]}, Positionsklammer 43531 → 43902 ms
  - gen: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23584 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.34 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `866e500ecce754bc47b32e1bc83556e3`, erwartet {"gen": [29068]}, Positionsklammer 44554 → 44875 ms
  - gen: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (23632 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.31 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `bd4942fa4b2a902df58d9283650ec5d6`, erwartet {"gen": [29068]}, Positionsklammer 45571 → 196 ms
  - gen: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json` (13771 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 0.28 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: Rolle probeeq ohne Erwartung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `2fb4c0763d34f097fe4cb00e9a15425e`, erwartet {"probeeq": [29068]}, Positionsklammer 42460 → 43344 ms
  - probeeq: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (1277 B, Version '0.3.0', gruende ['noch_keine_messdaten', 'rolle_ohne_aggregat'], 1.86 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: Rolle gen ohne Erwartung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `429e62b80ae80a9ba41a25e869dc4fd9`, erwartet {"probeeq": [29068]}, Positionsklammer 43393 → 45388 ms
  - probeeq: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23540 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: Rolle gen ohne Erwartung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
- Anfrage `4b55f549c95ced4ef3cb17555e655578`, erwartet {"probeeq": [29068]}, Positionsklammer 45388 → 1683 ms
  - probeeq: `4b55f549c95ced4ef3cb17555e655578.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json` (23700 B, Version '0.3.0', gruende ['rolle_ohne_aggregat'], 1.97 s nach dem Schreiben)
  - roh fremd: andere Kennung: `0851ddef6a6b0d2955b8183f5931fb37.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `089ce2f738c0b8a8dd28b30ce8aa3992.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `17f81b60b37c0444d8173e259c5e95ba.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `2fb4c0763d34f097fe4cb00e9a15425e.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `429e62b80ae80a9ba41a25e869dc4fd9.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `44df32a883d2d0d937e72df33a488fbb.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `4a818767dd9f525ff3c593082e9fa6b6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: Rolle gen ohne Erwartung: `4b55f549c95ced4ef3cb17555e655578.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `5ddf861ec29880578e655e954351dd2a.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `6ccf1613a7e57b139eeab0c3e210a584.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `6f359cf21cfd2d8d2ddd6961a76a5e9d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `802b464bdca2b41a41ffd43b6f1e2718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `82a9f707745328dd9e2ec1a4680c0933.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `848fea379c417effc9d375dc205fb752.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `866e500ecce754bc47b32e1bc83556e3.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `8b1b47542c373ff7394dd76acd2b9718.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `ab4aa456b3e8b02398b58280f3b9a2bc.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `b08037a100e63b68d700f342c47d88f5.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bb8781fcfbfa400947f12fdbfd084681.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `bd4942fa4b2a902df58d9283650ec5d6.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d1661b7b76a610b2f5914a050f12ea56.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `d6bcf3bfce17f3d9a9d418dfce27685c.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `dc1afa272482ebd1d857d34a9756f1b4.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`
  - roh fremd: andere Kennung: `ece258a0683cfb5348aedeca464ba978.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.gen.29068.2f869d2ee2d9099f.1e77b6e43b6a4b3582ab3407e9c496bc.json`
  - roh fremd: andere Kennung: `fddf05e17c2d0e8df6c192ece5fbed1d.probeeq.29068.d387a78151edaa16.13208ec5b3994704b8a6bb7a694554c4.json`

| Stelle | Rolle | durchlauf | kombinationen | Kennung | Zaehlung | evidenz_frisch | aktivitaet | abdeckung | konvergenz | lufs_s | true_peak_db | peak_db | evidenz_fenster | summe_fenster_aktiv | summe_fenster_gesamt | offen_fenster_aktiv | offen_fenster_gesamt | frames_gebaut | integration_samples | nicht_endlich_rahmen | projekt_sample_start | sample_count | spielt | lage im Host | p_vor ms | p_nach ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| S3 | gen | getrennt | 16/16 | `0851ddef` | Materialausschnitt null: zaehlt nie | true | 0.0 | 0.0 | null | null | null | null | 0 | 1389 | 6647 | 0 | 0 | 3030 | 4528 | 0 | null | null | false | [1872574, 2010784) v = 0 | 42460 | 42509 |
| S3 | gen | getrennt | 16/16 | `4a818767` | gezaehlt | true | 1.0 | 1.0 | 0.714650928974152 | null | -15.939704895019531 | -15.939704895019531 | 6 | 1395 | 6660 | 1 | 1 | 3039 | 4528 | 0 | 1872574 | 15260 | true | [1872574, 2010784) v = 0 | 42509 | 42879 |
| S3 | gen | getrennt | 16/16 | `17f81b60` | gezaehlt | false | 1.0 | null | null | null | -14.028632164001465 | -14.028903007507324 | null | 1415 | 6680 | 2 | 2 | 3049 | 4528 | 0 | 1913534 | 19581 | true | [1872574, 2010784) v = 0 | 43531 | 43902 |
| S3 | gen | getrennt | 16/16 | `866e500e` | gezaehlt | false | 1.0 | null | null | null | -24.583023071289062 | -24.585853576660156 | null | 1435 | 6700 | 3 | 3 | 3058 | 4528 | 0 | 1954494 | 19178 | true | [1872574, 2010784) v = 0 | 44554 | 44875 |
| S3 | gen | getrennt | 16/16 | `bd4942fa` | Ausschnitt [0, 3544) nicht ganz in [1872574, 2010784) | false | null | null | null | null | -15.642861366271973 | -15.643216133117676 | null | 1455 | 6720 | 2 | 2 | 3068 | 3544 | 0 | 0 | 3544 | true | [1872574, 2010784) v = 0 | 45571 | 196 |
| S3 | probeeq | getrennt | 16/16 | `2fb4c076` | gezaehlt | false | 1.0 | null | null | null | -15.911407470703125 | -15.9158935546875 | null | 1455 | 6771 | 5 | 5 | 3098 | 4528 | 0 | 1903294 | 7771 | true | [1872574, 2010784) v = 0 | 42460 | 43344 |
| S3 | probeeq | getrennt | 16/16 | `429e62b8` | gezaehlt | true | 1.0 | 1.0 | 0.744721293449402 | null | -35.821502685546875 | -35.835269927978516 | 2 | 1501 | 6817 | 1 | 1 | 3117 | 4528 | 0 | 1970878 | 26222 | true | [1872574, 2010784) v = 0 | 43393 | 45388 |
| S3 | probeeq | getrennt | 16/16 | `4b55f549` | Ausschnitt [65536, 72450) nicht ganz in [1872574, 2010784) | false | 1.0 | null | null | null | -14.135613441467285 | -14.137906074523926 | null | 1537 | 6853 | 6 | 6 | 3137 | 4528 | 0 | 65536 | 6914 | true | [1872574, 2010784) v = 0 | 45388 | 1683 |

- Stelle S3 gen (durchlauf getrennt, kombinationen 16/16): {"antworten": 3, "d_frames_gebaut": 19, "d_summe_fenster_aktiv": 40, "d_summe_fenster_gesamt": 40, "erster_ausschnitt": [1872574, 1887834], "letzter_ausschnitt": [1954494, 1973672]}
- Stelle S3 probeeq (durchlauf getrennt, kombinationen 16/16): {"antworten": 2, "d_frames_gebaut": 19, "d_summe_fenster_aktiv": 46, "d_summe_fenster_gesamt": 46, "erster_ausschnitt": [1903294, 1911065], "letzter_ausschnitt": [1970878, 1997100]}

