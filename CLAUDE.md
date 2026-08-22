# NAKAMA

Plugin-Familie für FL Studio (Windows 11, JUCE 8/C++20, CMake) mit
eigenständigem Rust-Broker (Named Pipe, `broker/`). Drei Apps, **eine**
Design-Identität: **Nakama Gen** (Main), **Nakama Probeeq** (aktive Sonde,
vollwertiger EQ), **Nakama Suna** (passive Sonde); Bundle-Name **Nakama
Studio**. Code, Bundle, Pipes und Schemas tragen heute noch den Legacy-Namen
**EQ-Copilot** (`EqCop*`, `Eqcp`) — Umbenennung ist ein eigenes
Identitäts-Ticket (NAK-30), kein Nebenbei-Refactor.

**Dieses Repo ist seit 18.08.2026 eigenständig** (Historie davor:
FL-Studio-Repo bis `7964777`). Remote `https://github.com/evenacadia-tech/Nakama`
(privat; User arbeitet wechselnd an Desktop und Laptop): vor Arbeitsbeginn
`git pull`, nach Commits pushen. Parallele Sessions sind möglich: eigene
Edits sofort per **explizitem Pathspec** committen (nie `git add -A`, nie
`--amend`), fremde uncommittete Dateien nie anfassen.

<!-- WAHRHEITSKERN:ANFANG — dieser Block wird von tools/hooks/nakama-primer.sh
     bei jedem Sessionstart und nach jeder Compaction injiziert. Er ist die
     EINZIGE Kopie; Hook und Memory dürfen ihn nicht nachbauen. -->
## Wahrheitskern (Stand 21.08.2026, aus dem Kontext-Interview mit dem User)

- **Produkt:** Nakama = Plugin-Familie. **Gen** (Main: Quellen-Übersicht,
  Befunde, Advisor) · **Probeeq** (aktive Sonde) · **Suna** (passive Sonde).
  Bundle „Nakama Studio". Alle drei Apps teilen EINE Design-Identität.
- **Grundgesetz, differenziert:** Gen und Suna **beraten nur** — setzen keine
  Parameter, schreiben keine Automation, Audio-Passthrough sampleidentisch
  (0 Samples Latenz, kein Tail; einzige Ausnahme die verriegelte
  Hör-Markierung von Gen). **Probeeq ist ein vollwertiger, hochwertiger EQ**:
  er setzt Anweisungen von Gen direkt um UND ist ganz normal manuell
  bedienbar. Audiothread überall: keine Sperren, Allokationen, Datei-/Pipe-/
  Netz-Zugriffe, kein Logging; Überlast verwirft Analyseframes, nie Audio.
- **Keine KI-/Claude-Erklärschicht** im Produkt (User 21.08.). Der Advisor ist
  regelbasiert.
- **Design-Quelle ist Figma (User).** Die Figma-Stände sind die Vorgabe;
  `Projekte\Nakama-Design` übersetzt sie in lebende Blätter (Zustände,
  Größen, Grenzfälle) — keine eigene Stilsuche, keine Varianten-Befragung.
  Produkt-Sprache **Englisch**; Docs, Commits, Gespräch Deutsch.
- **Material-Kit-Front** im heutigen Plugin = **Provisorium**, nie abgenommen;
  keine Arbeit mehr daran.
- **Prisma / Hörkompass / Glas-und-Licht** = **User-Idee, Studie, geparkt**
  (`eq-copilot/design/prisma-studie/`). Kein Produktteil, kein Bauplan; ihr
  Vokabular gehört nicht in die Plugin-UI. Das Geschmacksprofil dort bindet
  nur die Studie.
- **Produktzahlen** (16 sichtbare / 32 Vertrag Quellen, 8 Bänder, ±12 dB,
  Remote ±3 dB, 1,5/3 dB): vom User **hingenommene Startwerte**, änderbar —
  Regelfall einstellige Quellenzahl.
- **Pläne:** `docs/FL-Nakama-Sonden-Design-Entwurf.md` (mit Errata-Block)
  ist der technische Entwurf; die Recherche liegt als **Archiv** in `docs/archiv/`.
  Bauentscheidung erteilt 20.08. („okay dann fangen wir damit nächste
  session an"). Nächste Fläche ohne FL-Termine: S7 (`SONDE-006`).
- **Regel für Claude:** Ein Entscheid existiert nur mit **Datum + Zitat des
  Users** (Register unten). Eigene Vorschläge heißen „Vorschlag", nie
  „abgenommen"/„verbindlich". Keine zweite Kopie dieser Wahrheit in Hooks,
  Memory oder anderen Docs — verweisen, nicht abschreiben.
<!-- WAHRHEITSKERN:ENDE -->

## Entscheide des Users (Register — nur mit Zitat; Vorschläge stehen hier nicht)

| Datum | Entscheid | Wortlaut |
|---|---|---|
| 12.08. | Nakama berät nur (Rückweg vertagt); ein Referenztrack für jede Musik „ist Blödsinn"; Mess-Scope ist Pflicht | FL-Commits `77432e1` `882a964` `f13d2c9` — Paraphrasen im Commit-Body („USER-VORGABE"), Wortlaut nicht protokolliert |
| 16.08. | Umbenennung zu Nakama; Hör-Markierung als eigene Idee | FL-Commits `37aba8b`, `51021d2` — Paraphrasen im Commit-Body, Wortlaut nicht protokolliert |
| 16./17.08. | 3D-Papier-Shader, Tusche-Einzelmarken verworfen | „das sieht sehr schlecht aus" · „zerissenes Stroh", „Kochhut" |
| 17.08. | Kreativer Prozess wird nie übersprungen (Kreativ-Schleuse) | „man arbeitet eine idee aus, konkretisiert sie, solange bis man eine vorstellung hat … Kunst ist wertlos ohne ihren Entstehungsprozess." (Hook `tools/hooks/kreativ-schleuse.sh`) |
| 18.08. | Prisma-Objekt ✓, rohe Punktwolke als Dauerinhalt ✗ | „das prisma sieht top aus … wasserfall aus farbigen pixeln" |
| 19.08. | Nacht-Freistellung verworfen | „tut mir leid aber das war keine gute arbeit … eingebacken" |
| 20.08. | Bauentscheidung Sondenkern | „okay dann fangen wir damit nächste session an" |
| 20.08. | Prisma = eigenständige Begleit-App, Master-Plugin konventionell | Interview 21.08.: „Meine Entscheidung, so gesagt" |
| 20.08. | Design-Abnahmen (Größen 760×430 / 700×420 / 260×84, Overview+Detail, Vorhören nur markierte Zeile, Zustände nach Ausnahme-Prinzip) | `Nakama-Design/abnahmen/2026-08-20-*` |
| 21.08. | Nakama = Familie; Prisma nur Studie, geparkt | Interview: „Familie; Prisma nur Studie" · Prisma-Herkunft: „Meine Idee" · Hörkompass: „Alles nur Studie" |
| 21.08. | Probeeq ist ein EQ | „die active Probe fester Name : Nakama Probeeq ist ein vollwertiger hochwertiger EQ der mit Nakama kommuniziert. er kann von nakama direkt anweisungen umsetzen aber auch ganz normal manuell vom user benutzt werden" |
| 21.08. | Namen | „Nakama Gen = Main app · aktive sonde = Nakama Probeeq · passive sonde = Nakama Suna · Bundle = Nakama Studio" |
| 21.08. | KI-Erklärschicht raus | „Nein – raus aus dem Produkt" |
| 21.08. | Produktzahlen | „Hingenommen, passen aber" · Regelfall einstellige Quellenzahl: „ich habe schlicht 5 genommen weil 16 bedeutet 16 geladene proben auf instrumenten … 16 plugins nur für eq kostet auch massig leistung" (`Nakama-Design/abnahmen/2026-08-21-…`) |
| 21.08. | Design-Quelle | „das finale design wird aktuell in figma gemacht . alle 3 apps werden ein design haben mit der selben identität. alle alten sind alt. Ein Design entwickelt sich und ist nicht einfach da." · Rolle des Design-Repos: „Figma ist Quelle; Repo setzt um" |
| 21.08. | Material-Kit-Front | „Nie abgenommen – bleibt Provisorium" |
| 21.08. | Eigene Entwürfe (`Nakama-Design/…selfmade`, Figma-Bilder) | „Richtung für Look & Stimmung" · „Das ist ein Designprototyp keine technikanleitung, design passt sich am ende der funktion an." |
| 21.08. | Sprache | „Englisch – mein Wort" |
| 21.08. | Glas/Licht-Annahme raus; Geschmacksprofil nur Studie | „Glas/Licht raus; Profil nur Studie" |
| 21.08. | Recherche | „Archiv" |
| 21.08. | FL-Termine A/B bald; bis dahin S7 | „Termine bald; bis dahin S7" |
| 21.08. | Version 0.3.0 angleichen, noch nicht installieren | „Version 0.3.0 angleichen, noch nicht installieren" |
| 21.08. | „Lernsprache" und „Kernfunktion vor Verwaltung" gelten nicht mehr | „Keines mehr" |
| 21.08. | Aufräumen: alte Richtungen ins Archiv, Prisma-Studie parken, tote Memories löschen, Inspirationen + regenerierbare Renders raus | Interview Frage 20, alle vier gewählt |

**Was NICHT mehr gilt** (und nirgends mehr als gültig auftauchen darf):
Recherche als „kanonischer Plan" · Spectral Field / Bauplan 2.0 / Tiefenfeld /
Kunstwerk / Feld-Alphabet als Hauptansicht („alle alten sind alt") ·
Hörkompass als Zielvertrag der Plugin-UI · Geschmacksprofil für die Plugin-UI ·
Material-Kit-Front als „abgenommen" · Claude-Klick im Grundgesetz · Tauri-Hub-App
als Produktteil · „Lernsprache" / „Kernfunktion vor Verwaltung" als Regeln.

## Wo was liegt

| Was | Pfad |
|---|---|
| Plugin (JUCE 8 + CMake) | `eq-copilot/plugin/` (`src/` Produkt · `hostbridge/` · `hostprobe/` · `spike/` · `vertrag/` · `tests/`) |
| Schemas v2 (Vertrag des heutigen Plugins) / v3 (Sondenfamilie) | `eq-copilot/schemas/` · `eq-copilot/schemas/v3/` |
| State-Schema 2 + Parameterbestand (SONDE-006) | `eq-copilot/schemas/state/` (Vertrag) · `eq-copilot/plugin/state/` (Code) · `eq-copilot/fixtures/state/` (Korpus, drei Beine) |
| Identität (eingefroren, SONDE-001) | `eq-copilot/identity/plugin-identities-v1.json` |
| Broker (eigene Crate, `eqcop-broker.exe`) | `broker/` |
| Beweis-Runner + Manifeste | `tools/beweise.ps1` · `docs/beweise/` |
| Python-Erzeuger und -Prüfer | `tools/eq-copilot/` |
| Technischer Entwurf der Sondenfamilie (Fassung 0.4 + Errata) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` |
| Bauaufteilung (Sessions, Gates, Prüfstufen) | `docs/bauaufteilung-sonden.md` |
| Plugin-Architektur heute | `docs/plugin-wissen.md` |
| Offene Punkte (durabel, nie still löschen) | `docs/offene-punkte.md` |
| Einstieg nächste Session | `docs/NEXT-SESSION.md` |
| **Archiv** (Recherche, Mockups, alte Baupläne, alte Design-Docs — nur Verlauf) | `docs/archiv/` · `eq-copilot/design/archive/` |
| **Prisma-Studie** (geparkt, User-Idee) | `eq-copilot/design/prisma-studie/` (Statusblatt dort) |
| Material-Kit-Kette (Provisorium, technisch lebendig: `tokens.json` → `LeitstandTokens.h`) | `eq-copilot/design/` Wurzel |
| Design der drei Apps (Figma-Übersetzung, Abnahmen) | `C:\Users\phili\Projekte\Nakama-Design` |

## Hub — gemeinsames Briefing (Pflicht seit 22.08.2026)

Der User (Projektleiter, kein Coder) liest **eine** Seite: den Nakama-Hub
(Artefakt auf claude.ai; Adresse `artefakt_url` in `docs/hub/hub.json`). Sie
zeigt Plan erledigt/offen, was bei ihm liegt (Entscheide · Handgriffe · Wissen
vor dem Klick), den Figma-Stand der drei Apps mit Bild und nimmt Codex-Reviews
als `.md` entgegen. User-Wort 22.08.: „die pflicht für jeden claude dieses
dokument zu aktualisieren und bei session beginn anzusehen … wie ein
gemeinsames briefing und übersichtshub". Drei Pflichten je Session, Details in
`docs/hub/LIES-MICH.md`:

1. **Lesen** — der SessionStart-Hook (`tools/hooks/hub-primer.sh`) liest Kopf
   und Drift vor; bei Drift `hub.json` vor der Arbeit nachziehen.
2. **Eingang prüfen** — `Artifact(action='read', url=…)` → `py -3.13
   tools/hub/hub_eingang.py <Datei>` → jede Review Befund für Befund gegen die
   Quelldatei (T3-Regel, Bauaufteilung §2), Status in `hub.json`.
3. **Nachziehen** — `hub.json` fortschreiben (Klartext, kein Entscheid ohne
   Register-Zitat, „erledigt" nur mit Manifest) → `py -3.13
   tools/hub/baue_hub.py` → `Artifact(file_path=docs/hub/hub.html, url=…,
   capabilities={'artifact': {}})` → `hub.json` per Pathspec committen. Der
   Stop-Hook (`tools/hooks/hub-stop.sh`) blockt einmal je Session, wenn Commits
   ohne Hub-Update enden.

## Bauen & Beweisen (vom Workspace-Root)

```powershell
$cmake = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S eq-copilot -B eq-copilot/build -G "Visual Studio 17 2022" -A x64
& $cmake --build eq-copilot/build --config Release --target EqCopilot_VST3 EqCopShot EqCopPaintBench EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopPipeProbe EqCopIdentityTest EqCopHostContextTest EqCopHostProbe_VST3 EqCopHostProbeTest EqCopSchemaTest EqCopStateMigrationTest
```

**Ein Befehl für den ganzen Kanon** (ersetzt die bewusst nicht gebaute CI):

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-0NN.md -Anhaengen -Titel 'SONDE-0NN'
```

Er fährt alle Beine, schreibt die **rohe** Ausgabe ins Manifest und misst den
Baustand: sind Prüfbinaries älter als ihre Quellen, verweigert er mit Exitcode 4
die Beglaubigung (0 grün · 2 rot · 3 Voraussetzung fehlt · 4 nicht beglaubigt).
Vorlage `docs/beweise/VORLAGE.md`, Basislinie `docs/beweise/S0-basislinie.md`.

**Kanon (17 Beine, Tabelle in `tools/beweise.ps1`):** NullTest · Golden ·
Markierung · `cargo test` (seit 22.08. mit dem JCS-Bein) · sechs Python-Beine
des v3-Vertrags · A11 `pruefe_v2_schemas.py` · **A12
`erzeuge_state_fixtures.py --pruefen`** (SONDE-006) · Identität · **B2
`EqCopStateMigrationTest`** (SONDE-006) · Hostkontext · Host-Probe (zählt
89 nur mit PNG-Ziel, sonst 85 — NAK-34) · Schema. **Die Prüfzahlen stehen im
jüngsten Manifest in `docs/beweise/`, nicht hier** (zuletzt
`SONDE-006.md`: 17/17 grün). Nicht im Kanon, aber vorhanden:
`EqCopAuxSpikeTest` (NAK-37), Shot (`--state` lädt einen Host-State vor dem
Render), PaintBench, PipeProbe, `pluginval --strictness-level 8`. Vier Beine
stehen als „geplant" und werden Pflicht, sobald ihr Ticket sie baut.

- Golden-WAVs einmalig: `py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav`
  (Erzeuger der Referenz `tools/analyze-track.py` liegt noch im FL-Studio-Repo — NAK-31).
- Editor-Sichtprüfung ohne FL: `EqCopShot.exe <ziel.png> [breite]`.
- **Broker-Betrieb:** `broker\target\release\eqcop-broker.exe [--bindungen <pfad>]`
  (Standard `%APPDATA%\evenacadia\nakama\eq-copilot-bindungen.json`). Pipe
  Ende-zu-Ende: `eqcop-broker-probe.exe 30` + `EqCopPipeProbe.exe
  "\\.\pipe\evenacadia.eq-copilot.m2probe"` — **immer der Probe-Pipename**, nie
  die Produktion (`…eq-copilot.v1`; zwei Broker auf einem Namen stehlen sich
  still Clients; Produktion verweigert per FIRST_PIPE_INSTANCE).
- **Version:** `project(… VERSION 0.3.0)` und `kPluginVersion` müssen eins sagen —
  ein Configure-Riegel in `eq-copilot/CMakeLists.txt` bricht sonst ab.
  **Installiert ist das Bundle vom 16.08.** (Hash `74D86BD5…`); das gebaute vom
  21.08. (mit Hostbrücke) ist nicht installiert — Installation nur per
  User-Klick (`eq-copilot\install\Install-EQ-Copilot.ps1` als Admin; der Ordner
  ist gitignoriert und existiert nur auf dem Desktop — NAK-32).
- **FL-Termine A/B** (Klicklisten `eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md`,
  `…-B-HOSTZEIT.md`) sind **noch nicht gelaufen** (`%APPDATA%\evenacadia\nakama\spike\`
  leer); S4 und Gate G0 warten darauf. Die Wegwerf-Messgeräte `EqCopAuxSpike`
  (`NkSp`, 41 Prüfungen) und `EqCopHostProbe` (`NkHp`) sind gebaut.

## Technik-Zement (was heute gilt, weil Code und Beweis es sagen)

- **Identität (SONDE-001):** Bundle `EQ-Copilot`, Codes `Evna`/`Eqcp`, beide
  Class-IDs, `JUCE_VST3_CAN_REPLACE_VST2=0` eingefroren; `NkPr`/`NkAc` für
  Suna/Probeeq reserviert; `EqCopIdentityTest` misst das gebaute
  `moduleinfo.json` UND den CMake-Quelltext; Schema-1-Goldens
  `eq-copilot/fixtures/identity/` sind seit 22.08. eingefrorene **Lade**-Fixtures
  (das Plugin speichert Schema 2).
- **Hostbrücke (SONDE-003):** gevendorter JUCE-8.0.9-Wrapper per Patch
  (`third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`, CRLF in 149/163
  Zeilen, `.gitattributes` hält ihn per `-text` bytegleich) um drei
  Beobachtungen erweitert; Gate `eq-copilot/cmake/NakamaBruecke.cmake`
  (unberührt ⇒ patchen + nachmessen · gepatcht ⇒ No-Op · fremd ⇒ Bauabbruch).
  Gegenseite `plugin/hostbridge/NakamaHostBridge.h` (JUCE-frei, 0 Allokationen).
  Im Produkt kompiliert, aber **ungenutzt** — nur `HostProbeProcessor` ist eine
  `Senke`; der Verbraucher kommt mit SONDE-008/009. ⚠️ `_deps` nie von Hand
  editieren; bei jedem JUCE-Update Patch neu beweisen.
  🔑 Zwei teuer bezahlte Regeln: (1) ein Puffer, der beim Überlauf hinten
  abschneidet, darf nie die Quelle eines Wertes sein, den ein Vertrag überleben
  lässt; (2) ein Zähler beschreibt, was der HOST geliefert hat, nicht was in
  unsere Struktur passte.
- **v3-Vertragsbaum (SONDE-005a):** `eq-copilot/schemas/v3/`, gelesen von drei
  Beinen (`pruefe_v3_vertrag.py`, `EqCopSchemaTest`, `broker/tests/
  contract_cross_language.rs`) gegen ein **handgeschriebenes** Manifest (153
  Fixtures). 🔑 Ein Schema mit nicht implementiertem Schlüsselwort bricht den
  Ladevorgang. ⚠️ Bandgitter sind eingefrorene Hex-Zahlen, keine Rechenvorschrift.
  🔑 **Textriegel** = Stufe VOR dem Parser (8 Regeln, 59 Fälle in EINER Datei),
  weil JUCEs Zahlenleser überläuft. 🚨 Ein Riegel darf nie die Bibliothek
  befragen, gegen deren Verhalten er schützt.
- **FlatBuffers (SONDE-005b):** `.fbs` mit expliziten Feld-IDs, `flatc` auf
  COMMIT gepinnt, Codegen-Drift 0; zwei handgeschriebene Leser, weil FlatBuffers
  weder Enumbereiche noch Bitflags noch Feldbeziehungen verifiziert;
  `pruefe_fbs_feldids.py` schließt die Lücken „neue Tabelle ohne ids" und
  (T2-Runde 4) „neues Offsetfeld ohne Riegelzeile".
  🔑 Rusts Verifier kennt C++' Regel „May not point to itself" NICHT; der
  `strukturriegel` in `broker/src/telemetrie.rs` zieht sie über alle 15
  Offsetfelder nach — gemessen an 6215 Byte-Mutanten: 143 liefen auseinander,
  danach 0. 🔑 Ein Riegel, der STRENGER ist als das Bein, das er spiegelt,
  bricht denselben Vertrag wie einer, der schwächer ist.
- **State-Schema 2 (SONDE-006, 22.08.):** Vertrag
  `eq-copilot/schemas/state/nakama-state-v2.md`; das Plugin speichert
  `NakamaState{schema=2}` und migriert `EqCopilotState{schema=1}` **rein**
  (Goldens bytegleich). Unbekanntes Major / verletzte Kind-Matrix ⇒ read-only
  mit Originalbytes, keine Pipe, sichtbar im Editor. Jede persistente Änderung
  meldet Host-Dirty (`withNonParameterStateChanged`) — vorher kam
  `updateHostDisplay` im Plugin nicht vor. Parameterbestand 109 IDs
  handgeschrieben (`nakama-parameter-v1.json`), C++-Tabelle deckungsgleich
  gemessen; heute trägt kein Bundle Hostparameter. `state_hash` = SHA-256 über
  RFC-8785-Kanon mit **eigenem JSON-Leser** — 🔑 JUCEs Zahlenleser flusht
  Subnormale und verweigert `""` als Schlüssel; ein Hash, den drei Sprachen
  bilden, darf die Bibliothek nicht befragen, gegen die er schützt. Drei Beine
  (C++, Python `rfc8785`, Rust `serde_json_canonicalizer`) bytegleich gegen
  einen Korpus, dessen RFC-Zeilen den **vom RFC gedruckten** Text tragen.
  ⚠️ NAK-41: ein Schema-2-Projekt verliert im 16.08.-Build still seine
  Identität — vor der Installation wissen.
- **Hör-Markierung (0.3.0):** färbt auf Klick das Monitorsignal von Gen;
  Verriegelung im Code `(echtzeitOk ∨ test) ∧ (spielt ∨ ¬hatTransport) ∧
  ¬isNonRealtime ∧ (editorOffen ∨ test)`; Analyse-Abgriff davor; Render
  bitidentisch (MarkierungTest). Jede weitere Audio-Ausnahme von Gen/Suna
  braucht denselben Beweisstandard.
- **Editor heute:** Material-Kit-Front, festes Verhältnis 750:520, frei ziehbar
  600×416…1950×1352 (`PluginEditor.cpp:176-183`) — Provisorium; für die
  neue UI gilt die abgenommene Größe 760×430 („so oder so die zweitkleinste
  ist das vernünftigste", 20.08.); ob feste Stufen oder freie Skalierung,
  ist Claude-Empfehlung, kein Entscheid (User: „oder eine freie
  skalierungsform eben").

## Invarianten — tragend, jede Runde präsent

- **Schemas sind Verträge** (v2: `eq-ipc` · `eq-measurement` · `eq-report` ·
  `eq-snapshot` v3 · `eq-aggregat`; v3: `schemas/v3/`): neue Felder ⇒ ERST
  Versionierung; alte Snapshots laden ohne die Felder; unbekannte Felder
  zerstören alte Consumer nicht; Save + Load im selben Änderungssatz testen.
  In v3: `additionalProperties: true` nur mit `maxProperties`; Discriminator,
  Zieladresse, Revision, Capability nie additiv.
- **Engine kennt keine Optik:** AnalyseEngine liefert kohärente MessSnapshots
  (~20 Hz Leichtpfad + 250-ms-Schwerauswertung); der Editor hält NUR
  Anzeigezustand und malt nur bei neuer Snapshot-Revision.
- **Gegenpfade** (Invariante aus dem archivierten Bauplan 2.0 §11.5, gilt weiter): starten↔stoppen · öffnen↔schließen ·
  speichern↔laden · aktivieren↔abklingen · installieren↔Rollback — beide
  Hälften im selben Änderungssatz.
- **Zeit ist Aktivzeit:** Zonen-Ticks je 1 s AKTIVER Musikzeit (deterministisch,
  GoldenTest-beweisbar).
- **NaN-Ehrlichkeit:** NaN-Riegel mit Zähler, Nyquist-Kappe; < 5 endliche
  Nachbarn ⇒ keine Basislinie.
- **Probe-Pipe ≠ Produktions-Pipe** · **`ltasReferenzDb` ≠ Sollkurve** (keine
  globale Zielkurve; Befunde messen gegen ihre eigene Schulterlinie) ·
  **paint()-FPS ≠ Datenkadenz** (erst PaintBench, dann optimieren) ·
  **Musterblatt-/Demo-Daten ≠ Plugin-Daten** · **FL zeigt MIDI 60 als C5**.

## Design-Arbeitsmodell (seit 21.08.2026)

1. **Figma (User) ist die Quelle.** Aktuelle Stände liegen beim User
   (Downloads/Figma) und werden nach `Nakama-Design/assets/figma/` übernommen,
   sobald er sie gibt. Claude erfindet keine Richtung, keine Metapher, keine
   Farbwelt — Gesetz aus vier teuren Proben (17.08.): Claudes freie
   Bilderfindung ist Mode-Collapse; Vision kommt vom User.
2. **Nakama-Design übersetzt** Figma in lebende Blätter und prüft Zustände,
   Größen, Grenzfälle; Abnahmen dort sind bindend und tragen das User-Wort.
   Von dort spiegelt nichts hierher, bis eine Abnahme es sagt.
3. **Hier im Technik-Repo** gibt es keine Design-Arbeit mehr: `eq-copilot/
   design/` hält die Material-Kit-Kette (Provisorium), das Archiv und die
   geparkte Prisma-Studie. Der Hook `kreativ-schleuse.sh` blockt Edits darunter
   ohne frischen Freigabe-Marker — das ist gewollt.
4. **Geparkt, nie ungefragt reaktivieren:** Prisma-Studie (inkl. Hörkompass,
   Unicorn-Lichtwelt, ST-Map-Refraktion, Schlieren), Tiefenfeld, Bauplan 2.0,
   Kunstwerk-Studie, Feld-Alphabet, Proben P01–P04.

## Maschinen-Landminen

- ⚠️ **PowerShell `Start-Process -ArgumentList` quotiert NICHTS** — jedes
  Argument mit Leerzeichen selbst escapen (`Argument-Quoten` in `tools/beweise.ps1`).
- ⚠️ **Bash-Heredoc frisst Backslashes in Windows-Pfaden** (`\v` → 0x0B);
  Pipes maskieren Exitcodes (`${PIPESTATUS[0]}`).
- ⚠️ **`core.autocrlf` auf dem Zweitrechner**: `.gitattributes` hält Patch,
  v3-Baum und Fixtures per `-text` bytegleich — wer neue bytegleich geprüfte
  Dateien anlegt, trägt sie dort ein.
- 🌀 **Keine GPU-Batch-Render-Loops auf der Arc A770** (Lüfter-Failsafe klemmt
  auf 100 %; Fix `Win+Ctrl+Shift+B`, sonst Kaltstart). Renders nur Software/CPU.

## Read before working on

| Bereich | Zuerst lesen |
|---|---|
| Gemeinsames Briefing (Plan · bei dir · Design · Reviews) | `docs/hub/hub.json` (Quelle) · `docs/hub/LIES-MICH.md` |
| Einstieg, der eine nächste Schritt | `docs/NEXT-SESSION.md` |
| Plugin heute (Architektur, Datenfluss, IPC, Tests) | `docs/plugin-wissen.md` |
| Sondenfamilie: Technik + Phasen (Fassung 0.4 + Errata 21.08.) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` · `docs/bauaufteilung-sonden.md` |
| v3-Verträge, Textriegel, Bandgitter, Fixtures | `eq-copilot/schemas/v3/README.md` · `…/flatbuffers/README.md` |
| Beweise je Ticket (rohe Ausgabe) | `docs/beweise/` |
| FL-Termine A/B (liegen beim User) | `eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md` · `FL-TERMIN-B-HOSTZEIT.md` |
| Benchmark-Mechaniken (Median-Basislinie, Zonen) | `eq-copilot/docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md` |
| Offene Punkte | `docs/offene-punkte.md` |
| Design der drei Apps | `Nakama-Design/CLAUDE.md` + `abnahmen/` |
| Verlauf (nur zum Verstehen, nie als Vorgabe) | `docs/archiv/`, `eq-copilot/design/archive/`, `eq-copilot/design/prisma-studie/STATUS.md` |

## Arbeitsweise

- **Code ist die einzige Wahrheit** — Datei öffnen, bevor auf Doku-, Memory-
  oder Audit-Behauptungen gehandelt wird (~25 % der AI-Auditbefunde hier waren falsch).
- **Entscheide nur mit Zitat.** Was der User entschieden hat, steht mit Datum
  und Wortlaut im Register oben bzw. in `Nakama-Design/abnahmen/`. Alles
  andere ist Vorschlag, Arbeitsannahme oder Studie und heißt so — auch in
  Commit-Messages („Vorschlag:", nicht „Abnahme:").
- **Eine Wahrheit, ein Ort.** Zahlen (Testzahlen, Versionen, Fixturezahlen)
  stehen dort, wo sie gemessen werden (Manifeste, Code); Docs verweisen.
  Keine statischen Kopien in Hooks oder Memory.
- **Fortschritt nur mit Beleg aus dieser Session** (Testlauf, Render, Diff);
  Status kommt nach dem Beweis, nie davor. Selbstaudit nach jedem Commit.
- **Im Auftrag bleiben:** keine Neben-Refactors, keine Umbenennung der
  `EqCop*`-Legacy-Namen nebenbei (NAK-30 ist der Ort dafür).
- Out-of-scope-Funde → `docs/offene-punkte.md` (datierte Zeile mit ID).
- Nach großen Schritten `docs/plugin-wissen.md` nachziehen; Session-Memo ins
  Memory nur für Wissen, das NICHT im Repo steht (eine Lehre pro Datei, mit
  dem Warum; Falsches löschen statt stapeln).
- Alle Texte dieses Projekts (Docs, Commits) auf Deutsch; Produkt-Texte Englisch.
