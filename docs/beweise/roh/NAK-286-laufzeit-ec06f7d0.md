# Laufzeit-Arm NAK-286 - GEMESSEN

Zeit: 2026-09-14 19:29:53 | Basis: a2887462 | HEAD: ec06f7d0 | Repo: C:\Users\phili\Projekte\Nakama | Projekt: Nakama-Diagnose.flp
Kopfzeile: LAUFZEIT NAK-286 ec06f7d0 GEMESSEN installation=installiert; pruefen Exit 0 szenarien=1 verfehlt=0 [bereitschaft.json=0] fl=Producer Edition v26.1.4 [build 5589]

## Protokoll

```text
[2026-09-14 19:29:29] Laufzeit-Arm NAK-286 auf ec06f7d0 (Basis a2887462), Repo C:\Users\phili\Projekte\Nakama
[2026-09-14 19:29:29] Lohnt es? True - Produktpfade im Diff: 8 Datei(en), z. B. eq-copilot/install/nakama-installer-v1.json
[2026-09-14 19:29:29] Manifest-Hashes nachziehen (--hashen)
[2026-09-14 19:29:29]   hashen: Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
[2026-09-14 19:29:29]   hashen:                  Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[2026-09-14 19:29:29]   hashen: [0] Struktur vor dem mutierenden Release-Schritt
[2026-09-14 19:29:29]   hashen:   ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
[2026-09-14 19:29:29]   hashen:   ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
[2026-09-14 19:29:29]   hashen:   ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
[2026-09-14 19:29:29]   hashen:   ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
[2026-09-14 19:29:29]   hashen:   ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
[2026-09-14 19:29:29]   hashen:   ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
[2026-09-14 19:29:29]   hashen:   ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
[2026-09-14 19:29:29]   hashen:   ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
[2026-09-14 19:29:29]   hashen:   ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
[2026-09-14 19:29:29]   hashen:   ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
[2026-09-14 19:29:29]   hashen:   ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
[2026-09-14 19:29:29]   hashen:   ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
[2026-09-14 19:29:29]   hashen:   ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
[2026-09-14 19:29:29]   hashen:   ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
[2026-09-14 19:29:29]   hashen:   ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)
[2026-09-14 19:29:29]   hashen: [hashen] Artefakte gegen den gebauten Stand festschreiben
[2026-09-14 19:29:29]   hashen:   ok      main = C4EDA82C35CA0A7DD300B9465803476F10ECBCAAED71BFF665C0CEDB80008756
[2026-09-14 19:29:29]   hashen:   ok      active-probe = BF640FB57C79BB6562860243F140D255418D10AD6E92CDEA92C0060D3F71C3F7
[2026-09-14 19:29:29]   hashen:   ok      eqcop-broker.exe = F8F67718BF5A797D7DE7F13820D162C7150F57A0A99C975CF873B5510B047BE3
[2026-09-14 19:29:29]   hashen: 
[2026-09-14 19:29:29]   hashen: geschrieben: eq-copilot/install/nakama-installer-v1.json
[2026-09-14 19:29:29] Aufgabe \Nakama\installieren starten
[2026-09-14 19:29:33]   installieren: Exit 0, Zustand Ready, Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3] |   ok      Nakama Probeeq installiert  [C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3] |   ok      eqcop-broker.exe installiert  [C:\Program Files\evenacadia\Nakama\eqcop-broker.exe] | INSTALLATION OK
[2026-09-14 19:29:35]   pruefen: Exit 0, Log: Installierter Stand gegen das Manifest: |   aktuell          C:\Program Files\Common Files\VST3\EQ-Copilot.vst3 |   aktuell          C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3 |   aktuell          C:\Program Files\evenacadia\Nakama\eqcop-broker.exe
[2026-09-14 19:29:35] loopMIDI laeuft
[2026-09-14 19:29:35] FL gestartet: PID 26236 (C:\Program Files\Image-Line\FL Studio 2026\FL64.exe) mit C:\Users\phili\AppData\Local\evenacadia\nakama-laufzeit\projekt\Nakama-Diagnose.flp
[2026-09-14 19:29:40] Boot-Marke nach 5 s: { "script_version": "2026-09-12", "loaded_at": "2026-09-14T19:29:40", "program_title": "FL Studio 2026" }
[2026-09-14 19:29:49] Ping ok: FL Producer Edition v26.1.4 [build 5589] ('FL Studio 2026'), Fenster 'Nakama-Diagnose.flp - FL Studio 2026', Port loopMIDI Port 1
[2026-09-14 19:29:53] Szenario bereitschaft.json: Exit 0
```
## Szenarien


<!-- szenario.py 2026-09-14T17:29:50+00:00 -->
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
| 11 | `mixer.getPeaks` | `{"track": 1}` | `{"name": "Insert 1", "peak_left": 0.4983249604701996, "peak_max": 0.4983249604701996, "peak_right": 0.4755004644393921, "success": true, "track": 1}` | ok |
| 12 | `mixer.getPeaks` | `{"track": 0}` | `{"name": "Master", "peak_left": 0.5602249503135681, "peak_max": 0.5602249503135681, "peak_right": 0.5197994709014893, "success": true, "track": 0}` | ok |
| 13 | `transport.getStatus` | `{}` | `{"is_playing": true, "is_recording": false, "loop_mode": "song", "position": "3:16:07", "success": true}` | ok |
| 14 | `transport.stop` | `{}` | `{"stopped": true, "success": true}` | ok |

**Ergebnis:** 14 von 14 Schritten bestanden.


