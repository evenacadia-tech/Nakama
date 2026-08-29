# NEXT-SESSION — Einstieg für die nächste Runde

> ## ✅ NACHTRAG — 29.08.2026, S9 `SONDE-007b` T3-Nacharbeit
>
> Die Marke von `SONDE-007b` ist nicht mehr `offen`, sondern
> **`T3 NEEDS_WORK 2026-08-29 nachgearbeitet`**. Von den zwei G1-Befunden ist
> **§4.2 geschlossen**. Was das genau heißt — und nicht mehr: der Riegel gegen
> `post_fader_contribution` sperrt **klassenunabhängig**, gemessen auf
> `lade()`-Ebene für alle vier Klassen inklusive `passive_probe` mit
> Bundlevertrag `nkpr` (Bein `B2`); der **Durchgriff durch die Sondenschale**
> (`setStateInformation` → `lade` → `positionErlaubt`) ist am **einzigen
> gebauten Sondenbundle** Nakama Probeeq gemessen, Klasse `active_probe`
> (Block 5b im `EqCopProbeeqNullTest`); und der ursprüngliche G1-Träger — eine
> passive Sonde als Produkt — existiert seit S9b/`SONDE-007c` nicht mehr.
> Beide Riegelhälften sind per Mutationsprobe rot vorgeführt. Kanon GRÜN;
> Läufe und Zahlen stehen im Manifest.
>
> **Der Satz weiter unten gilt weiter**, nur genauer: **§55 Klausel 1** ist
> weiterhin ungemessen und damit der einzige offene Punkt an diesem Ticket.
> Seit S9b/`SONDE-007c` betrifft er **zwei** Ziele — Nakama Gen und Nakama
> Probeeq —, nicht drei. Datiert als **`NAK-87`**. Nur der User kann ihn
> schließen: Installation als Admin-Klick, dann FL starten und beide Bundles
> laden. **P1 bleibt bis dahin nicht exit-fähig.**

> ## 🛑 ÜBERGABE — 24.08.2026, Dirigentenlauf beendet (Kontext voll)
>
> **Stand: 12 von 36 abgenommen.** An diesem Tag abgenommen: **S12–13**
> (`T2 PASS`, drei Prüf- und zwei Nacharbeitsrunden) und **S10–11**
> (`T2 PASS`). Danach **Gate G1** gefahren, Urteil **NEEDS_WORK**, Nacharbeit
> vollständig gefahren.
>
> ### Was G1 ergab und was daraus geschlossen wurde
>
> **Gate 1 hielt** („kein Pfad gefunden"), **Gate 7 fiel** — von zwei Modellen
> unabhängig, auf zwei Ebenen. Der v3-Vertrag verlangte
> `measurement_position` und `aussageklasse`, **koppelte sie aber nicht**:
> `insert` + `beitrag` validierte fehlerfrei. Alle fünf Befunde plus §55
> Klausel 4 (NAK-42) sind geschlossen, Kanon **28/28 Exit 0**.
> Manifest: `docs/beweise/G1.md` §1–§9 (Gate) und §10 (Nacharbeit).
>
> Marken heute: `SONDE-005a`, `SONDE-005b`, `SONDE-007a` = `T3 NEEDS_WORK
> nachgearbeitet` · **`SONDE-007b` = `offen`**, bewusst, siehe unten.
>
> ### 👤 Das Einzige, was noch fehlt — und nur der User kann es
>
> **§55 Klausel 1: „Alle drei Ziele scannen und laden in FL."** Nie gemessen;
> installiert ist weiterhin das Bundle vom 16.08. Braucht **Installation als
> Admin + einen FL-Lauf**. ⚠️ **NAK-41**: ein Projekt, das der neue Build
> speichert, verliert im 16.08.-Build still seine Messpunkt-Identität — nicht
> zwischen alt und neu springen. Deshalb steht `SONDE-007b` auf `offen`: sein
> zweiter Befund ist kein Code, sondern ein Termin.
>
> ### ❓ Offene Frage an den User (per Routine gestellt, Antwort steht aus)
>
> **A** *(Empfehlung)* — erst FL-Termin, dann **G1 Runde 2** in einem Zug; dann
> ist alles Prüfbare geprüft und das Gate kann durchgehen.
> **B** — G1 Runde 2 sofort, um die sechs Reparaturen unabhängig prüfen zu
> lassen; die FL-Klausel fällt dabei zwangsläufig wieder auf.
>
> ⚠️ **G1 Runde 2 ist ein Gate** — nach Skill §4.1 **nie ohne den User starten.**
>
> ### Kanäle (Entscheid 24.08., Register + Skill §3.6)
>
> Berichte **und Rückfragen** gehen über die claude.ai-Routine
> **„Nakama: Dirigent-Meldung"** (`trig_01BUKf1i5Y9ztqGkA6Ev4eff`, kein Cron,
> `push: true`): Meldung zwischen die `--- MELDUNG ---`-Marker im Prompt
> schreiben, dann `run`. Das ist derzeit der **einzige** Kanal.
>
> ⚠️ **WhatsApp ist abgeschafft** (User 24.08.) — MCP, Daemon, Autostart und
> Gerüst sind **entfernt**, es gibt dort nichts mehr aufzurufen.
>
> 🔑 **Der Messenger ist Matrix (E2EE), gebaut und belegt am 24.08.**
> Werkzeug: `C:\Users\phili\.claude\matrix-bridge\` — **außerhalb des Repos**,
> weil dort Zugangsdaten liegen; nie hierher kopieren.
>
> - **Melden:** `py -3.13 melden.py "Ticket · Ergebnis · was als Nächstes"`
>   (aus dem Brücken-Ordner). `--status` zeigt den Zustand.
> - **Bild/Datei an den User:** `py -3.13 melden.py --datei <pfad> ["Text"]`
>   — verschlüsselt, mit Bildmaßen, damit die Vorschau auf dem Handy stimmt.
>   Das ist der Weg für `/fragen`: Design-Stand hinschicken, Antwort kommt
>   zurück. ⚠️ Anhänge kann **nur der Dienst** senden; läuft er nicht, bricht
>   `melden.py` mit Exit 3 ab (Text hat einen Notweg, Dateien bewusst nicht —
>   verschlüsselten Upload zweimal zu pflegen hieße zwei Kopien, die
>   auseinanderlaufen).
> - **Vom User kommen Text, Bilder und Dateien an.** Anhänge landen
>   entschlüsselt unter `eingang/`, der Pfad steht im zugestellten Prompt.
>   Ein Nachhol-Lauf beim Start reicht nach, was ein Absturz verschluckt hat.
> - **Rückweg:** Antworten des Users landen als Prompt in der Session, die in
>   `config.json` unter `nimbalyst.session_id` steht. Ist die weg, sucht der
>   Dienst **selbst** die neueste lebende claude-code-Session, liefert dorthin,
>   schreibt die Kennung fest und sagt es dem User im Raum — von Hand
>   nachziehen ist also nicht nötig. Wer sie doch setzt: es ist die
>   **Nimbalyst**-Kennung aus `list_recent_sessions`, **nicht** die
>   Claude-Code-Kennung aus dem Scratchpad-Ordnernamen (die liefert
>   „Session not found").
> - **Der Dienst** (`dienst.py`) startet mit Windows und muss laufen, sonst
>   kommt nichts an. Er ist der **einzige** Prozess, der mit Matrix spricht;
>   `melden.py` übergibt ihm nur Aufträge. Nie einen zweiten Klienten
>   aufmachen — zwei Prozesse auf einem Kryptospeicher legen sich Megolm-
>   Sitzungen an, die der andere nicht kennt.
>
> ### Neue offene Punkte dieses Tages
>
> **NAK-68** (Plausibilität der Rahmenskalare) · **NAK-69** (veröffentlichte
> Frames tragen unter `/O2` unbestimmte Füllbytes aus `Transportstempel` —
> heute folgenlos, ab **`SONDE-010`** eine nicht reproduzierbare Zahl im
> Wire-Vertrag) · **NAK-70** (grundabhängige Auslassung in `grenzeZiehen()` bei
> `loopWrap`/`moeglicherStraddle` ohne Bein) · **NAK-71** · **NAK-72**
> (`beweise.ps1` kann still auf `VCTIP.EXE` warten) · ⚠️ **NAK-73**:
> `git checkout` ist hier wegen `core.autocrlf=true` **keine** byteweise
> Rückstellung — die Formel „SHA-256 vorher = nachher" trägt in keinem Manifest
> mehr ohne Nachweis.
>
> **Nach dem Gate** wäre `S14–15` (`SONDE-010`) die nächste Baufläche — normale
> Bau-Runde, kein Gate. Dort werden NAK-69, NAK-59, NAK-40 und der
> v3-Schema-Schnitt aus NAK-29 fällig.
>
> Vollständige Begründungen je Runde: `docs/dirigent/protokoll.md`.

> ## ✅ G1-NACHARBEIT IST GEFAHREN — 24.08.2026 · **kein PASS erklärt**
>
> Die fünf Befunde aus `docs/beweise/G1.md` §4 **und** die §55-Klausel 4
> (NAK-42) sind **geschlossen**, jeder erst an der Quelle nachgemessen (alle
> fünf hielten) und jeder neue Riegel **beim Fallen vorgeführt**. Neuer
> Abschnitt **§10/§11** im Gate-Manifest; §1–§9 sind unangetastet.
> Kanon **28/28 grün, Exit 0, beglaubigt**.
>
> | Befund | Commit | Kern |
> |---|---|---|
> | §4.1 Gate 7, Vertrag | `ebfe02e` | `probe_descriptor` = diskriminierte Union über `measurement_position` |
> | §4.2 Gate 7, State | `a2fe0f5` | `post_fader_contribution` für **keine** Klasse |
> | §4.3 `ppq` | `fa976fd` | beide Leser + **Manifest-Eintrag** (das war der eigentliche Fix) |
> | §4.4 K2b | `fac67f4` | beide Richtungen + Wertwiderspruch |
> | §4.5 Alphabet | `ebfe02e` | Muster für `state_hash` und beide Bitmaps |
> | §55 Klausel 4 | `5ffcc60` | `broker/src/dto.rs` — Rust klassifiziert alle 15 ungültigen DTOs |
>
> **Die Entwurfsentscheidung, ohne Versionsanhebung** (`G1.md` §10.0): keine
> neue Fläche, der Vertragstext trug die Regel bereits, kein Erzeuger im Baum —
> und tragend: **eine Anhebung würde die Lücke als unterstützte v3.0-Variante
> einfrieren. Wer einen Bug versioniert, konserviert ihn.**
>
> **Marken:** `S5` · `S6` · `S8` stehen auf `T3 NEEDS_WORK … nachgearbeitet`
> (= warten auf einen frischen Prüfer). **`S9` bleibt auf `offen`** — von
> seinen zwei Befunden ist einer nicht geschlossen, siehe unten.
>
> ### 🚨 Was P1 weiterhin blockiert — und nur ein Mensch kann es
>
> 1. 👤 **§55 Klausel 1: „Alle drei Ziele scannen und laden in FL" ist
>    ungemessen.** Braucht Installation (Admin-Klick) + FL-Lauf. **NAK-41 gilt
>    davor.** **P1 ist NICHT exit-fähig**, egal wie grün alles andere ist.
> 2. 👤 **NAK-79 — offene Produktfrage:** *welche* `plugin_kind` darf
>    `post_fader_contribution` führen, wenn es den diskreten Aux-Bus gibt? Der
>    Entwurf sagt, wer der **Receiver** ist (§32.2 `:1610`), aber nicht, wer die
>    **Position trägt**. Heute kostet die Frage nichts (die Position ist
>    unerreichbar, `contribution_aux` ist `unsupported`); im Code steht dafür
>    eine **fail-closed-Vorgabe, kein Urteil**. Sie wird fällig, sobald ein Bau
>    den Bus bekommt (`SONDE-011`).
> 3. **Ein frischer Gate-Lauf.** Dass der geschlossene Weg zu Gate 7 zu ist, ist
>    gemessen; dass es **keinen anderen** gibt, ist nicht gezeigt — das kann nur
>    ein neuer Gate-Lauf, und diese Session hat nicht geurteilt.
>
> **Neu im Offen-Set:** **NAK-79** (Produktfrage oben) · **NAK-80** (K2b/K2c
> leben nur im Configure-Schritt und sind in keiner der 28 Kanonzeilen sichtbar
> — heute folgenlos, aber eine Annahme über das Buildsystem statt einer
> Messung). **NAK-78** bleibt unberührt (war nicht Auftrag).
> ⚠️ **NAK-73 in der Praxis:** Rückstellungen liefen **nie** über
> `git checkout`, sondern über zurückgeschriebene Bytes mit SHA-256- und
> `cmp`-Beleg. Das Muster steht in `G1.md` §10.2–§10.6 und ist nachnutzbar.
> ⚠️ **Berichtigt:** der Riegelkopf in `NakamaKern.cmake` behauptete seit dem
> 22.08. eine gemessene JUCE_USE_CURL-Abweichung. Sie existierte nie — sie war
> ein Artefakt eines Generatorausdrucks, den `get_target_property` nicht
> auflöst (`G1.md` §10.4).


> ## 🚨 GATE G1 IST GEFAHREN — 24.08.2026 · Urteil **NEEDS_WORK**
>
> Die gebündelte T3-Runde über **P1** ist durch, alle drei Prüfer sind gelaufen:
> `/c-review` (13 Cluster-Worker + 2 Richter über `eq-copilot/plugin`),
> `/rust-review` (21 Worker + 2 Richter über `broker`) und **Codex**
> (`gpt-5.6-sol`, `xhigh`) als zweites Modell. Manifest: **`docs/beweise/G1.md`**.
> Kanon **28/28 grün, Exit 0, beglaubigt**.
>
> **Gate 1 hielt** — beide Modelle antworteten wörtlich „kein Pfad gefunden".
> **Gate 7 fiel** — und zwar *zweimal, unabhängig, auf verschiedenen Ebenen*:
> Claude über das **v3-Schema** (`insert` + `beitrag` validiert; selbst
> nachgefahren, 0 Fehler in allen 4×2 Kombinationen), Codex über den
> **gespeicherten Zustand** (eine Sonde mit nur einem Stereo-Bus darf sich
> `post_fader_contribution` nennen). Entwurf §49.2 ist dort unbedingt — ein
> gefallenes Gate kann kein PASS bekommen.
>
> **Vier Ticketmarken sind gesetzt** (`T3 NEEDS_WORK 2026-08-24 offen`), damit
> der gerechnete Planstand sie sieht: `SONDE-005a` · `SONDE-005b` ·
> `SONDE-007a` · `SONDE-007b`. **`SONDE-006` behält sein `T2 PASS`** — der
> Befund an `NakamaState.cpp:100` ist erfasst, aber nicht dem Ticket
> zurechenbar: die Regel, die er durchsetzt, steht im Entwurf gar nicht
> (Begründung `G1.md` §7.2).
>
> ### Was als Nächstes ansteht — in dieser Reihenfolge
>
> ✅ **Die vormals hier stehende User-Frage ist beantwortet** (24.08., im Chat):
> *„EINE Sonde kann nur dort messen wo sie steckt. und sie kann nur dort EQ
> anwenden durch GEN wo sie steckt. das ist kein prinzip das ist die einzige
> realität."* Damit ist der Fix eindeutig statt zweideutig — und **`SONDE-006`
> hat nachträglich ebenfalls `T3 NEEDS_WORK` bekommen** (`G1.md` §7.2):
> `positionErlaubt` erlaubt die Beitragsposition ausgerechnet der Sonde.
>
> ✅ **Erledigt in der G1-Nacharbeit** (`a2fe0f5`, am Code nachgemessen): keine
> Klasse darf die Position mehr führen, zwei unabhängige Riegel. **NAK-79 ist
> mit derselben User-Aussage beantwortet** — und zwar so, dass die fail-closed-
> Vorgabe im Klassen-Riegel die *richtige Dauerantwort* ist, keine Platzhalter-
> entscheidung: der Beitrag gehört gar nicht auf die Instanzposition, sondern
> auf die Frage, aus welchem Gen-Eingang eine Messung stammt. Was bleibt, ist
> eine Vertragsfrage für `SONDE-018` — meine Entscheidung, nicht deine.
>
> 1. **Gate 7 schließen** (NAK-74): Kopplung im Schema per `oneOf` — das
>    Schlüsselwort ist im Engine-Subset und wird in derselben Datei viermal
>    benutzt — **plus** ein Fixture `ungueltig/insert-mit-beitragsklasse.json`,
>    das ohne den Fix grün wäre.
> 3. **NAK-75** `start_ppq`/`end_ppq` in **beiden** Lesern prüfen, je ein
>    Binärfixture; die Prüfung so bauen, dass schon *ein* fehlender Leser rot wird.
> 4. **NAK-76** K2b in beide Richtungen messen und beim Fallen vorführen.
> 5. **NAK-42** schließen — jetzt ein **Phasen**-Blocker: §55 verlangt am
>    P1-Exit, dass C++ und Rust *jedes* Fixture gleich klassifizieren; Rust liest
>    die 15 ungültigen State-DTOs nicht (gemessen: 6 gültig, 15 ungültig,
>    `contract_cross_language.rs:458` liest nur `dto_gueltig`).
> 6. 👤 **§55-Klausel 1 ist ein User-Termin:** „Alle drei Ziele scannen und laden
>    in FL" ist **ungemessen** — installiert ist weiterhin nur das
>    16.08.-Bundle, und Installation bleibt ein Admin-Klick. **NAK-41 gilt davor.**
>
> **Neu im Offen-Set:** NAK-74 (Gate-7-Bruch) · NAK-75 (`ppq` ungeprüft) ·
> NAK-76 (K2b einseitig) · NAK-77 (`state_hash`/Bitmap: Länge geprüft, Alphabet
> nicht) · NAK-78 (Sammelzeile: 27 + 20 überlebende Prüferbefunde).
> ⚠️ **NAK-72 berichtigt:** `VSCMD_SKIP_SENDTELEMETRY=1` war in diesem Lauf
> gesetzt und hat **nicht** geholfen — VCTIP kam trotzdem, der Lauf stand still,
> `Stop-Process -Name vctip` löste ihn. Es bleiben die beiden anderen Wege.
> ⚠️ **Neue Landmine:** `build_run_plan.py` von *rust-review* liest `context.md`
> in der Windows-Codepage und **stürzt ab** (`⚠️` enthält das Byte `0x8f`) —
> `PYTHONUTF8=1` davor.

> ## ✅ NACHTRAG 24.08.2026 — `S10–11` ist abgenommen
>
> Die **Ausnahme**, die der Dirigent im Block darunter empfohlen hat („es lohnt,
> `S10–11` sein `T2` einzeln zu geben"), ist gefahren: ein zweiter frischer
> Prüfer hat die Nacharbeit (`334f185..0b23cf5`) beurteilt und gibt
> **`T2 PASS`** — Bericht `docs/beweise/SONDE-008.md` **§10**, Marke im
> Manifestkopf. Alle vier Befunde aus §8 sind geschlossen **und** jeder ist von
> einem Bein gedeckt, das der Prüfer mit einer *eigenen* Mutation zum Fallen
> gebracht hat (M1–M8). Kanon **28/28 grün, Exit 0, beglaubigt**.
>
> **Damit warten noch VIER Stände auf ein Urteil, nicht fünf:** `S5` · `S6` ·
> `S8` · `S9` — alle vier gehören zu **P1** und damit zu **G1**. Die Empfehlung
> „B, mit einer Ausnahme" unten hat ihre Ausnahme verbraucht; was bleibt, ist
> schlicht **B**. Der gerechnete Planstand steht auf **12 von 36**.
>
> 🛑 **Der Dirigent hält hier erneut an (§4.1).** Der gerechnete Planstand nennt
> als Nächstes **G1** — ein Gate: `T3`, fremde Prüfer, Bruchaufträge. Das
> startet er nicht blind. Beim User liegt damit nur noch **eine** Frage, und sie
> ist eine Aufwandsfrage: `S5`, `S6`, `S8`, `S9` einzeln nachholen oder am Gate
> bündeln. ⚠️ Bündeln heißt: die Gate-Session muss eine Urteilsmarke **je
> Ticketmanifest** setzen — ein `PASS` allein in `G1.md` bewegt den gerechneten
> Planstand nicht (`tools/plan/planstand.py` liest je Ticket).
>
> **Neu offen aus dieser Prüfung** (keiner ein heute erreichbarer Fehler):
> **NAK-71** (die Ordnungszusage in `neustartAnfordern()` hat kein Bein; ihr
> beschriebener Schadensfall ist zu harmlos; zwei Texte stimmen nicht mit der
> Quelle überein) · **NAK-72** (`tools/beweise.ps1` kann still hängen, weil
> `Start-Process -Wait` auf `VCTIP.EXE` wartet — hat diesen Lauf 13 Minuten
> gekostet) · **NAK-73** (`git checkout` ist hier **keine** byteweise
> Rückstellung, `core.autocrlf=true` — betrifft jeden Mutationsbeweis, der
> „SHA-256 vorher = nachher" behauptet).

> ## 🛑 HALT DES DIRIGENTEN — 24.08.2026 nachts: der nächste Schritt ist ein Gate
>
> **S12–13 (`SONDE-009`) ist ABGENOMMEN** — `T2 PASS` von einem frischen Prüfer
> in Runde 3 (`65613f6`, Manifest §13). Der Planstand steht damit auf
> **11 von 36**. Der Weg dorthin waren drei Prüf- und zwei Nacharbeitsrunden in
> einer Nacht; das Protokoll steht in `docs/dirigent/protokoll.md`.
>
> **Der Dirigent hält hier an, nach §4.1 seines Skills:** der gerechnete
> Planstand nennt als Nächstes **G1** — ein Gate. Gates sind `T3`, verlangen
> **fremde** Prüfer (`/c-review`, `/rust-review`, `/security-review`, Codex) und
> Bruchaufträge. Das ist keine Runde, die ein Dirigent blind startet.
>
> ### 👤 Die Entscheidung, die beim User liegt
>
> **Fünf Stände warten weiter auf ein frisches Urteil:** `S5` · `S6` · `S8` ·
> `S9` · `S10–11`. Alle sind gebaut, nachgearbeitet und grün — keinem fehlt
> Arbeit, allen fehlt ein **PASS eines Prüfers, der sie nicht gebaut hat**.
>
> | Weg | Was er kostet | Was er bringt |
> |---|---|---|
> | **A — einzeln nachholen** | fünf frische T2-Sessions, je eigener Basispunkt; jede kann eine Nacharbeitsrunde nach sich ziehen | präzise; jeder Stand bekommt sein eigenes Urteil, unabhängig vom Gate |
> | **B — am Gate bündeln** *(Empfehlung)* | eine G1-Runde deckt `S5`, `S6`, `S8`, `S9` (alle P1); `S10–11` gehört zu P2 und fiele erst bei **G2** | `T3` ist **strenger** als das fehlende `T2`; fremde Prüfer sind eine andere Art Auge als eine weitere Claude-Runde; der Kontext wird einmal aufgebaut statt viermal |
>
> **Empfehlung: B, mit einer Ausnahme.** `S10–11` ist der jüngste Stand und
> hätte bei G2 den weitesten Weg — es lohnt, ihm sein `T2` einzeln zu geben,
> während die vier P1-Stände auf G1 warten.
>
> ⚠️ **Ein technisches Detail, das über Erfolg oder Schein entscheidet:** ein
> `PASS` in `docs/beweise/G1.md` allein bewegt den Planstand **nicht**. Der
> Rechner liest die Urteilsmarke **je Ticketmanifest**
> (`tools/plan/planstand.py`). Wer G1 fährt, muss in **jedes** geprüfte
> Ticketmanifest eine Marke setzen — sonst bleiben S5/S6/S8/S9 auf „gebaut",
> obwohl sie geprüft wurden. Das ist keine Entscheidung, sondern eine
> Arbeitsanweisung an die Gate-Session.
>
> ### Was S12–13 offen lässt (aus dem PASS selbst, §13.9)
>
> Das PASS deckt die **Deckungsfrage**, nicht die Bedeutungsfrage und nicht den
> Lauf im Host. Drei benannte Punkte reisen mit:
> **NAK-68** (Plausibilität der Rahmenskalare — „ist ein Crest über 10 ms noch
> dieselbe Größe?") · **NAK-69** (ein veröffentlichter Frame trägt unter `/O2`
> unbestimmte Füllbytes aus `Transportstempel`; heute folgenlos, ab
> **`SONDE-010`** wäre es eine nicht reproduzierbare Zahl im Wire-Vertrag) ·
> **NAK-70** (eine *grundabhängige* Auslassung in `grenzeZiehen()` ist bei
> `loopWrap` und `moeglicherStraddle` von keinem Bein gedeckt — keine heutige
> Fehlfunktion, eine Lücke gegen eine künftige Änderungsform).
>
> **Nach dem Gate** wäre `S14–15` (`SONDE-010`) die nächste Baufläche — dort
> werden NAK-69, NAK-59, NAK-40 und der v3-Schema-Schnitt aus NAK-29 fällig.

> ## 🎨 Design-Strang — Quellenkorrektur und Live-Stand 24.08.2026
>
> Der User hat verbindlich bekräftigt: **Immer nur** `Nakama-Design`
> (`NPCQYSkoZEd4Av0NlKxBOd`) enthält die aktuellsten Varianten; alle anderen
> Figma-Dateien, Kopien und Node-IDs sind Verlauf. Entscheid:
> `design/abnahmen/2026-08-24-figma-depot-immer-aktuell.md`.
>
> Direkt per Figma-API gelesen: Overview-Wrapper `25:443` mit Artboard
> `25:444` = 760×430; nach der User-Korrektur EQ-Wrapper `28:993` mit
> Artboard `28:994` = 760×430; Zustandstafeln `25:805` und `25:901`.
> In den vier Bereichen: 554 Nodes, 0 Reaktionen,
> 0 Components/Component Sets/Instances. Die Größenabweichung ist geschlossen;
> das Gesamtdesign ist weiterhin nicht abgenommen.
>
> **Nächster Design-Schritt:** Den korrigierten EQ-Stand zusammen mit Overview
> und den beiden Zustandstafeln sichten/abnehmen, danach Komponentenvarianten
> und Motion-Prototypen ergänzen. Die Größenkorrektur sowie die Entfernung der
> konkret beanstandeten toten Elemente sind im aktuellen EQ-Hauptframe sichtbar
> umgesetzt. Die historische Copy-Fassung wird nicht als aktuelle Wahrheit
> übernommen. Ausführliche
> Restarbeiten: `design/docs/figma-restarbeiten-nakama-gen.md`. Bereits
> entschieden, aber noch zu gestalten: zweite Vergleichsquelle,
> PROTECTED-Detailgesten, zweistufiger Mix und der kleine Revisionsverlauf
> auf Seite 1. Sidechain bleibt im ersten Release unsichtbar. Für die
> Probeeq-Rückfallfläche ist der vorhandene Suna-Entwurf `6:2864` die
> verbindliche visuelle Basis. Offen ist nur noch die visuelle Abnahme der
> ausgearbeiteten Gen- und Probeeq-Flächen.

> ## ⚠ DER EINE NÄCHSTE SCHRITT — Stand 24.08.2026, nach **T2 Runde 2** und der zweiten Nacharbeit
>
> **Runde 2 ist gefahren (Manifest §11) und lautete NEEDS_WORK; die Nacharbeit
> dazu steht in §12.** Vier Befunde, alle vier an der Quelle nachgemessen und
> bestätigt, alle vier geschlossen. Beweislauf **GRÜN 28/28, Exit 0,
> beglaubigt**; B5 ist von **155 auf 212 Prüfungen** gewachsen.
>
> **Der tragende Befund T2R2-1 war eine Deckungslücke, kein Ausfall:** die zehn
> Rahmen-Skalare (`rahmenPeak`, `rahmenSamples`, `rahmenL/R/L2/R2/LR`, …) werden
> von `grenzeZiehen()` korrekt geleert — aber die Leerung ließ sich **entfernen,
> ohne dass eine einzige Zeile rot wurde** (`155 bestanden, 0 Fehler`,
> selbst gefahren). Dieselbe Gestalt wie T2-1, ein Feldbündel weiter.
>
> 🔑 **Die Antwort ist bewusst KEINE längere Liste.** Die Ursache war nie eine
> vergessene Zeile, sondern die Prüffrage *„welche Auskunft steht auf null?"* —
> die kann nur Felder sehen, die eine Auskunft **haben**, und ein künftiges Feld
> hat keine. Neu ist deshalb **G13, die Zwillingsprobe**: zwei Engines, dieselbe
> Blockfolge Zug um Zug, gegensätzlicher Inhalt davor (laut / Stille), dieselbe
> Grenze, danach **bitgleiches** Audio in beide — ab da muss jeder Frame
> **feldgleich** sein. Verglichen wird mit `FeatureFrame::operator== = default`,
> also memberweise und **vom Compiler gepflegt**: ein neues Feld ist automatisch
> dabei. Vorgeführt an einem eigens eingebauten **neuen** Träger (Mutation Q2) —
> gefangen, **ohne dass an irgendeiner Liste etwas geändert wurde**.
> ⚠️ Nicht `memcmp`: gemessen tragen die Füllbytes des Frames unter `/O2`
> unbestimmte Werte (**NAK-69**, trifft SONDE-010).
>
> **Der Entwurfsentscheid zu T2-1 ist gefallen** (§10.1, Begründung dort):
> *die Grenze schneidet den Inhalt, nicht die Uhr.* `grenzeZiehen()` leert
> jetzt auch die drei Bandakkus und die zwei Zellzähler; `liveSamples` und
> `evidenzSamples` bleiben **bewusst** stehen, weil sie kein Messwert sind,
> sondern der Fahrplan — würden sie mitgenullt, verstummte die Telemetrie
> vollständig, sobald Grenzen dichter kommen als die Kadenz (Queue-Drops unter
> Last, enge Schleife). Das ist dieselbe Todesart, die §4.4 für den Straddle
> schon einmal ausdrücklich verworfen hat.
>
> **Das Bein ist mitgewachsen** — das war der eigentliche Fehler:
> `keinFensterUeberbrueckt()` fragte fünf Füllstände und keinen Akkumulator.
> Neu: acht Auskünfte an der Engine, `keineAkkusUeberleben()` in beiden
> Kernfragen, eine Vorbedingung je Grenzfall und **Abschnitt G12**, der nicht
> den Füllstand misst, sondern den **Frame**. Mutation **M8** (Fix ganz zurück)
> macht **14 Zeilen** rot und reproduziert die Prüferzahlen **23 / 80 / 40 / 43**
> Ziffer für Ziffer.
>
> Die zwei kleinen Befunde am Text sind hier berichtigt, **nicht** in §10:
> die positive Hälfte gab es in **6 von 9** Grenzfällen (nicht neun), der Wert
> ist **690** je Fall (die Zahl **1435** kommt im Lauf nicht vor) — G3, G4, G5
> sind nachgerüstet, jetzt sind es **9 von 9**; und das Aktivitätsgate steht bei
> `if (! aktiv)` in `rechneFenster()` — im geprüften Stand `FeatureEngine.h:1144`,
> nicht `:992`; heute (nach dieser Nacharbeit) `:1198`. Die Diagnosezeile druckt `Breite` jetzt
> als Bänderzahl + Exponent (`196B/2.660e-04` statt `0.000`) — eine Diagnose,
> die bei echtem Bruch alle Träger unschuldig aussehen lässt, ist schlimmer als
> keine.
>
> 🔑 **Trotzdem steht kein PASS da, und das ist Absicht:** wer repariert,
> spricht sich nicht selbst frei. Die jüngste Urteilsmarke lautet
> `T2 NEEDS_WORK 2026-08-24 nachgearbeitet` — der Stand wartet auf einen
> **frischen Prüfer**, der weder gebaut noch nachgearbeitet hat. Das ist jetzt
> die **dritte** Runde, die er anschauen würde (§9 → §10 → §11 → §12).
>
> | Stand | Manifest | Urteil heute | Was fehlt |
> |---|---|---|---|
> | **S9** / `SONDE-007b` | §5 (Bericht), **§6** (Nacharbeit) | NEEDS_WORK | **kein PASS** — nie ein zweites Mal geprüft |
> | **S10–11** / `SONDE-008` | §8 (Bericht), **§9** (Nacharbeit) | NEEDS_WORK | **kein PASS** — nie ein zweites Mal geprüft |
> | **S12–13** / `SONDE-009` | §9 (T2 R1), §10 (Nacharbeit 1), §11 (**T2 R2**), **§12** (Nacharbeit 2, 24.08.) | NEEDS_WORK · nachgearbeitet | **kein PASS** — erneuter T2 durch einen frischen Prüfer |
>
> **Damit hat sich der eine nächste Schritt verschoben:** der gerechnete
> Planstand nennt jetzt **G1** (`Gate` — Prüfrunde über P1, eigene Session).
> Sechs Schritte warten auf ein Urteil, keiner auf Bauarbeit. Wer prüft, prüft
> mit frischem Kontext und **schließt keine eigenen Befunde**.
>
> **Was die zweite Nacharbeit ausdrücklich NICHT geprüft hat** (§12.7, damit
> niemand mehr Deckung annimmt als da ist):
> - **G13 deckt keinen Träger, der im Messfenster kein Frame-Feld erreicht** —
>   gemessen, nicht vermutet: `liveBreiteAkku` erreicht den Frame nur über
>   Bänder mit `liveAkku[b].n > 0`, und der erste Frame nach der Grenze fällt
>   ~1–2 Blöcke später, während das 4096-Punkte-Fenster **8** Blöcke braucht.
>   Deshalb macht Mutation Q3 die **neun G-Fälle** rot, aber nicht G13. Die
>   zwei Beine sind **komplementär**: bandgetorte Träger deckt
>   `keineAkkusUeberleben()`, per-Sample-Träger deckt G13.
> - **`loopWrap`, `moeglicherStraddle` und der gestoppte `zeitSprung` (G3)**
>   laufen nicht als Zwillingspaar (PPQ-Führung in jedem Block nötig).
> - **Plausibilität** der Rahmenskalare bleibt **NAK-68** (mit NAK-59 in
>   denselben `.fbs`-Änderungssatz in `SONDE-010`); **NAK-69** ist neu (Frame
>   nicht byteweise vergleichbar/hashbar).
> - Der G12-Sweep fährt weiterhin nur den **Seek** auf Frame-Ebene; M1–M14
>   wurden nicht wiederholt (außer P1 als Q1 und P2 als Q3); kein FL-Lauf, kein
>   Thread-Sanitizer, keine Serialisierung.
>
> ### ⚙️ Neu am 23.08. (Werkzeug, nicht Produkt): der Planstand wird GERECHNET
>
> Die Briefing-Seite ist **abgeschafft** (User-Wort im Register; NAK-50
> geschlossen). Was das für dich ändert: **nichts nachziehen.**
> `docs/PLAN-STAND.md` entsteht aus `docs/plan/plan.json` (Text) und den
> **Urteilsmarken** in den Manifesten (Status); `tools/hooks/planstand.sh`
> rechnet nach jedem Befehl neu und committet das Blatt allein.
>
> **Deine eine neue Pflicht als Prüfer:** nach einem Urteil eine Zeile in den
> Kopf des Manifests setzen —
> `<!-- NAKAMA-URTEIL: T2 PASS 2026-08-23 -->` bzw.
> `<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-23 offen|nachgearbeitet -->`.
> Ohne Marke bleibt der Schritt „gebaut" (fail-closed, Vergessen
> untertreibt). Das vierte Wort steuert, was das Blatt als Nächstes nennt:
> `offen` ⇒ Nacharbeit, `nachgearbeitet` ⇒ wartet auf einen frischen Prüfer.
> Offene Fragen an den User stellt der Skill **`/fragen`**, nie improvisiert
> im Chat. Hausordnung: `docs/plan/LIES-MICH.md`.
>
> ### S12–13 (`SONDE-009`) ist GEBAUT und T2-GEPRÜFT — 23.08.2026
>
> Commits `f14924a` · `357786e` · `133526e` · `f1e4a08`. Beweislauf **GRÜN
> 28/28, Exit 0, beglaubigt**; Kanon **26 → 28** (B5 `EqCopAnalysisGoldenTest`
> mit 120 Prüfungen gebaut, A19 `erzeuge_bandgitter_header.py --pruefen` neu).
> `EqCopNullTest`/`GoldenTest`/`MarkierungTest`/`QueueStressTest` unverändert
> grün — **kein Sample hat sich geändert**, und `schliesstAn()` ist nicht
> angefasst.
>
> **Was das Gate verlangte** („Drop/Seek/Loop trennt jedes offene Fenster"):
> neun Grenzarten, jede mit eigener Ursache und eigenem Prüffall — Drop (zählt
> als **Segment**, nicht als Epoche), Seek laufend, Seek **gestoppt** (das
> T2-4-Erbe aus S10–11), Loop-Wrap, möglicher Straddle, Transportkante,
> Sampleratewechsel, Neuanlauf, Beweislagewechsel. Getrennt wird alles:
> FFT-Fenster **beider** Auflösungsstufen, Loudness-Zelle, 3-s-Historie,
> Korrelationsfenster, Fluss-Vorgänger **und der K-Filterzustand**.
>
> **Was der T2 beantwortet hat** (Manifest §9.2–§9.9), damit die Nacharbeit es
> nicht noch einmal fragt:
> - **Der Kern hält.** Bass-/Hauptfenster, Loudnesszelle, 3-s-Historie,
>   Flussvorgänger **und die K-Filterzustände** fallen bei allen neun
>   Grenzarten — kein Pfad gefunden. Auch `flussHistorie` und `vorigesSpektrum`
>   sind unschädlich (ihre Werte werden vor der nächsten Nutzung überschrieben).
> - **Realtime:** keine Allokation, keine Sperre, kein I/O, kein Logging im
>   `processBlock`-Pfad; die FeatureEngine ist von dort gar nicht erreichbar.
>   NaN/Inf, DC, Nyquist, Subnormale, 1-Sample-Blöcke und ein 65536-Sample-Block
>   selbst gefahren — alles sauber.
> - **K-Gewichtung:** gegen die *gedruckten* BS.1770-Koeffizienten nachgerechnet,
>   0,04312 dB exakt reproduziert. Der Umzug nach `KGewichtung.h` ist wirklich
>   ein Umzug.
> - **Bitidentität:** A1/A2/A3 in zwei unabhängigen Läufen grün.
> - **NAK-56-Werkbankhälfte:** das Bein fährt `nakamaBlockEmpfangen()` wirklich
>   und kann scheitern. ⚠️ Präzisiert: „Ersatzweg nur zwei Bits" gilt für den
>   minimalen Playhead des Beins — ein Host-Playhead mit Schleifenpunkten
>   erreicht **drei** von sieben.
> - **A19** selbst gebrochen: es schlägt an, mit Zeilenangabe.
> - **§5.1/G11 ist wirklich scharf geworden**, nicht anders blind — der Lauf
>   endet beim ersten LUFS-Frame und vergleicht bitgleich.
>
> **Was offen blieb** (Manifest §9.12): die sieben Mutationsproben M1–M7 sind
> **nicht** nachgemessen, `FeatureFrame` ist **nicht** Feld für Feld gegen
> `nakama_telemetry_v1.fbs` gelegt, kein Thread-Sanitizer, kein FL-Lauf, und
> die Bandgitter-Fixtures sind nicht gegen IEC 61260-1 nachgerechnet (nur
> Header ↔ Fixture ↔ Bein auf Deckungsgleichheit).
>
> ⚠️ **Was S12–13 NICHT geprüft hat**, vollständig in Manifest §8. Die zwei
> wichtigsten: die FeatureEngine ist **an keinen Sender angeschlossen** (die
> Felder sind gegen `nakama_telemetry_v1.fbs` gelegt, aber nie serialisiert —
> die Cross-Language-Probe fehlt, das ist `SONDE-010`), und die
> **Ereignis-Erkennung ist nicht gegen einen annotierten Korpus gemessen** —
> §39.3 („bekannte Impulsereignisse bleiben über Blockgrößen und Sampleraten
> zeitlich stabil") ist damit **nicht** erfüllt und gehört zu `SONDE-013`.
>
> ✅ **Zwei offene Punkte geschlossen:** NAK-29 (bedingte Feldpflichten, im
> **Erzeuger**) und die **Werkbankhälfte** von NAK-56 (`nakamaBlockEmpfangen()`
> am echten Prozessor gefahren, mit Gegenprobe Brücke 0x7f gegen Playhead 0x3).
> Neu: **NAK-59** — Band-Stereo wird berechnet, hat aber keinen Platz im
> v3-Binärvertrag (`SONDE-010`).
> ⚠️ **T2-Nachtrag zu NAK-29:** der Riegel steht am richtigen Ort, aber er ist
> aus `baueStempel()` konstruktiv nie auslösbar, und der beschriebene Meldeweg
> („die `sequence` springt") existiert im Code nicht — `++sequenz` steht hinter
> dem Ablehnungszweig. Beides gehört in die Nacharbeit, bevor `SONDE-010` den
> ersten echten Leser anhängt (Manifest §9.10).
>
> ⚠️ **`CLAUDE.md` trägt jetzt wieder eine veraltete Zahl:** „Kanon (24 Beine)"
> bzw. was dort nach der Hygiene-Runde steht — gemessen sind es **28**
> (`tools/beweise.ps1`, jüngstes Manifest `SONDE-009.md`). Diese Session hat
> `CLAUDE.md` auftragsgemäß **nicht** angefasst. Dieselbe Ursache wie in NAK-55
> beschrieben: eine gemessene Zahl, die in eine Zusammenfassung abgeschrieben
> wurde, altert dort.
>
> ---
>
> ## S10–11 — Stand 23.08.2026 abends
>
> | Stand | Manifest | Urteil heute | Was fehlt |
> |---|---|---|---|
> | **S9** / `SONDE-007b` | §5 (Bericht), **§6** (Nacharbeit) | NEEDS_WORK | nie geprüft |
> | **S10–11** / `SONDE-008` | §8 (Bericht), **§9** (Nacharbeit) | NEEDS_WORK | nie geprüft |
>
> **Die S10–11-Nacharbeit ist gefahren** (Commits `f88f8c4` · `90c387a` ·
> `a563d4c` · `0373c51` · `bbe4b3f`; Manifest **§9**, NAK-58 geschlossen). Alle vier
> T2-Befunde geschlossen — jeder erst an der **Quelldatei** nachgemessen, und
> **alle vier haben sich bestätigt, keiner war ein Fehlalarm**. Jeder neue oder
> erweiterte Riegel ist **beim Fallen vorgeführt** worden, mit echtem Bau und
> byteweiser Rückstellung (SHA-256 vorher = nachher). Beweislauf **GRÜN 26/26,
> Exit 0, beglaubigt**, kein Exit-4-Fehlalarm.
>
> - **T2-1** — nicht den Eimer *melden*, sondern die **Voraussetzung**
>   reparieren: der Satz „unsicher ist allein die Zugehörigkeit **eines
>   einzigen** Bins" stimmt genau dann, wenn jede endliche Blocklautheit in
>   einem Bin **endlicher Breite** liegt. Über dem Feingitter liegt jetzt ein
>   **Oberband** (3070 Bins à 1 LU bis +3100,01 LUFS, also über
>   `lautheit(DBL_MAX)` = +3081,86). Kosten 36 KB, Nutzen: der Prüfer-Korpus
>   liefert **d = 0,000000000 LU** statt 2,917909. Bein **B9 66 → 108**.
> - **T2-2** — `EqCopMarkierungTest` **T11**. Die Prüfer-Mutation macht dieses
>   Bein jetzt rot; die drei anderen bleiben zu Recht grün.
> - **T2-3 / T2-4** — Anlauf steigt sofort; Seek bei **bekannt** gestopptem
>   Transport ist eine Grenze. Bein **B4 69 → 81** (Abschnitte O und P).
>
> Der **Kanon bleibt bei 26 Beinen** — kein neues Bein, sondern zwei vorhandene
> sehend gemacht.
>
> **Was ein Prüfer bei S10–11 zuerst ansehen sollte:**
> - `plugin/core/analysis/LoudnessAccumulator.h` — der Indexraum ist neu
>   (`binIndex`, `grenzBin`, `binMitte` über zwei Auflösungen). Die Frage, an
>   der alles hängt: **durchschneidet Γ_r wirklich immer genau einen Bin?**
> - `plugin/tests/LoudnessGoldenTestMain.cpp` §G — kann G3 scheitern, oder ist
>   auch er konstruktiv grün? Genau daran ist §F gescheitert.
> - `plugin/core/StampedAudioQueue.h` — `neustartAnfordern()` hat jetzt **zwei**
>   Schreiber auf `aktuelleStartFolge`. Die Ordnungsaussage in §9.6 Punkt 3 ist
>   **Argument, nicht Messung**.
> - `plugin/tests/MarkierungTestMain.cpp` T11 — deckt (c) wirklich ab, dass (a)
>   nicht aus einem anderen Grund grün ist?
>
> ⚠️ **Die größte ungedeckte Fläche, unverändert — vor SONDE-009 lesen:** die
> **Brückenhälfte des Zeitstempels hat kein Bein.** Jede Messung, auch die des
> Prüfers und auch T11, läuft über den **Playhead-Rückfallweg**;
> `nakamaBlockEmpfangen()` ist gelesen, nie gefahren. Ebenso ungemessen: **kein
> Lauf in FL** (NAK-56). ⚠️ **Neu daran seit T2-4:** derselbe Zähler
> `analyseKontinuitaetsbrueche()`, an dem NAK-56 beim FL-Termin abgelesen werden
> soll, steigt jetzt **auch** bei einem Seek mit gestopptem Transport — beim
> Messen die Ursachen trennen. Vollständige Liste des Ungeprüften: §8.9 und §9.7.
>
> 🔑 **Die teuerste Lehre dieser Runde, jetzt dreimal bestätigt:** immer fand der
> *adversariale* Teil den Fehler, nie der Normalfall. Der Erbauer fand so das
> Klemmen überlauter Blöcke; der Prüfer fand direkt daneben T2-1 — im **Fix**
> dieses Bugs; und die Nacharbeit fand beim Bauen von §O noch, dass zwei
> Neuanläufe sich verschluckten. Drei Sätze, die das zusammenfassen:
> **ein Test, der nicht scheitern KANN, beweist nichts** (§F mit einem Pegel) ·
> **ein Bein kann am richtigen Ort stehen und trotzdem blind sein**
> (§N fährt genau den `prepareToPlay`-Gegenpfad von T2-3, sah ihn aber nie,
> weil dort zwischen Neuanlauf und Drain immer ein Audioblock läuft) ·
> **ein Fix, der näher am Wortlaut liegt, ist nicht automatisch näher an der
> Absicht** (`bbe4b3f`: die erste T2-4-Regel brach bei *jeder* Zeitbewegung im
> Stopp — §32.3 nennt „einen Sprung" als Grenze, nicht „eine Bewegung").
>
> ⚠️ **`CLAUDE.md` trägt DREI veraltete Zahlen** — „(12)" Kern-Verbraucher
> (gemessen **14**), „Kanon (23 Beine)" (jetzt **26**) und die Formel der
> Hör-Markierung (`∨ ¬hatTransport` ist gefallen). **Nicht** berichtigt: die
> Datei hat uncommittete Änderungen einer anderen Session. Nachliste in
> **NAK-55**, dieselbe Sperre wie NAK-54.
>
> **Danach:** ~~S12–13 (`SONDE-009`)~~ — **gebaut am 23.08.**, siehe den Block
> ganz oben. Sie baut direkt auf der Grenze auf, die S10–11 liefert; das
> T2-4-Erbe (Seek bei bekannt gestopptem Transport) hat dort mit B5 §G3 sein
> eigenes Bein bekommen, und die Mutationsprobe M3 zeigt, dass es **genau
> zwei** Zeilen deckt — den Riegel und nichts sonst.
> **Nächste Fläche nach dem Prüfer:** S14–15 (`SONDE-010`) — v3-Control-/
> Telemetry-Clients und Rust-Envelopeparser. Dort werden NAK-59 (Band-Stereo
> ohne Wire-Platz), der v3-Schema-Schnitt aus NAK-29 und der Doppelpuffer für
> `merkmalFrame()` fällig.
>
> ---
>
> ## S9-Nacharbeit — Stand 23.08.2026 nachmittags
>
> **Ein frischer T2-Prüfer auf den NACHGEBESSERTEN S9-Stand.** Nicht diese
> Session, nicht die beiden bisherigen Prüfer: die vier Befunde aus §5.7 sind
> geschlossen, und **wer nacharbeitet, urteilt nicht** (S8-Präzedenz). Das
> Urteil in `docs/beweise/SONDE-007b.md` §5 steht weiterhin auf
> **NEEDS_WORK**; ein PASS auf S9 fehlt.
>
> **Was die Nacharbeit getan hat** (Commits `d93d536`, `9573546`, `0d5ec3d`;
> Manifest **§6**): alle vier Befunde geschlossen — jeder erst an der
> Quelldatei nachgemessen (alle vier bestätigten sich, kein Fehlalarm), jeder
> neue Riegel **beim Fallen vorgeführt** mit roher Ausgabe. **T1 ist gefahren**
> (§6.6, feste Sechserliste über `git diff 4b500a4..HEAD`), Kopf-Kästchen auf
> ☑. Beweislauf **GRÜN 24/24, Exit 0, beglaubigt**.
>
> **Was ein Prüfer zuerst ansehen sollte** — genau das, was noch niemand mit
> fremdem Kontext gelesen hat:
> - `eq-copilot/schemas/installer/nakama-installer-v1.md` **§2.1** (Ordner-Hash v1),
>   **§2.2**, **§5.0/§5.1** und **§5.2** (warum das v1 bleibt) — neuer Vertragstext,
> - `eq-copilot/install/NakamaOrdnerHash.ps1` und
>   `tools/eq-copilot/pruefe_installer_gegenpfad.py` — **beide ganz neu**,
> - `eq-copilot/install/Install-Nakama.ps1` — Ordner statt Datei, plus drei
>   Verhaltensänderungen im Rückweg,
> - `eq-copilot/cmake/NakamaIdentitaet.cmake` und
>   `eq-copilot/plugin/tests/IdentityTestMain.cpp` — die zwei Riegel aus T2-2/T2-3.
>
> 🔑 **Die teuerste Lehre dieser Runde:** der Gegenpfad `installieren ↔ Rückweg`
> stand vollständig im selben Änderungssatz — und war trotzdem kaputt. Drei
> Leser (Autor, T2 Teil A, T2 Teil B) hatten ihn gelesen; **beim ersten
> wirklichen Lauf fielen zwei echte Fehler heraus.** Geschrieben ≠ gefahren.
> Deshalb läuft er ab jetzt als **Kanon-Bein A18** bei jedem Beweislauf mit
> (Sandbox unter `%TEMP%`, es wird nichts installiert). **Kanon 23 → 24.**
>
> ⚠️ **`CLAUDE.md` trägt zwei veraltete Zahlen** — „(12)" Kern-Verbraucher
> (gemessen 13) und „Kanon (23 Beine)" (jetzt 24). Beide **nicht** berichtigt:
> die Datei hat uncommittete Änderungen einer anderen Session. Nachliste in
> **NAK-55**, dieselbe Sperre wie NAK-54.
>
> ~~**Danach:** S10–11 (`SONDE-008`)~~ — **erledigt am 23.08. abends**, siehe
> den Block ganz oben.
>
> ---
>
> ## ⚠ Zuerst lesen — Stand 23.08.2026 mittags
>
> **P0 ist geschlossen.** Gate G0 ist gefahren, Urteil **PASS**
> (`docs/beweise/G0.md`): beide Bruchauftraege (Gate 1, Gate 5) gescheitert,
> die P0-Kernflaeche traegt keinen Befund. Damit faellt der
> Schliessungsvorbehalt §65 fuer `SONDE-005`.
>
> **S8 (`SONDE-007a`) ist gebaut** (22.08. spaet, Manifest
> `docs/beweise/SONDE-007a.md`, Commits `5d0e9fd` + `06913aa`): `NakamaKern`
> ist eine echte Static-Lib, einmal uebersetzt statt je Ziel, ohne eine einzige
> `JucePlugin_*`-Konstante. Kanon 18 → **19** Beine (A14).
>
> **T2 ist gefahren** (23.08., Commit `26b94c5`, Manifest §5/§6): Urteil
> **NEEDS_WORK**. Die zentrale Behauptung hielt unter eigener Messung (0 von 74
> `JucePlugin_`-Defines im Kern, Fassade traegt, alle acht nachgezaehlten
> Zahlen stimmten), aber fuenf Befunde — allen voran eine **echte Regression
> des Aenderungssatzes**: als eigene Lib erbt der Kern die PUBLIC-Schalter
> seiner Verbraucher nicht mehr und uebersetzte als einziger Code im Baum unter
> `/W1` statt `/W4`. Alle fuenf am selben Tag geschlossen, fuenfter Riegel
> **K2c** gebaut und beim Fallen vorgefuehrt, Kanon danach 19/19, `pluginval` 8
> erneut SUCCESS. **Kein PASS auf den nachgebesserten Stand** — S8 steht damit
> wie S5 und S6 auf „gebaut", nicht „abgenommen".
> 🔑 Lehre fuer S9: **eine Static-Lib erbt die PUBLIC-Schalter ihrer
> Verbraucher nicht** — wer in S9 eine zweite Lib anlegt, haengt ihr die
> Empfehlungsschalter selbst an; K2c faellt sonst zur Configure-Zeit.
> **S9 (`SONDE-007b`): Abschnitte 1+2 von 3 gebaut** (23.08., Commits
> `8e32baf` + `fdece75`, Manifest `docs/beweise/SONDE-007b.md`).
> **Es gibt jetzt drei Bundles:** `EQ-Copilot.vst3` (`Eqcp`),
> **`Nakama Suna.vst3`** (`NkPr`) und **`Nakama Probeeq.vst3`** (`NkAc`).
> Die Identitaet kommt aus `identity/plugin-identities-v1.json` — kein Literal
> mehr im Bauskript (**NAK-52 geschlossen**). Die beiden neuen entstehen aus
> EINER Quelle `plugin/sonde/` ueber duenne Target-Schichten; ihre CIDs sind
> erstmals an Artefakten gemessen, kein Bundle traegt eine fremde Ziel-CID.
> Auch entschieden und gebaut: K2b/K2c messen gegen **jeden** Kern-Verbraucher
> (12 damals, **13** seit Abschnitt 3 — vom T2-Teil-B-Pruefer nachgezaehlt),
> nicht gegen eine Stichprobe. Kanon 19 → **21** (A15/A16), `pluginval` 8
> an beiden neuen SUCCESS.
>
> ⚠️ **Beide neuen Bundles sind heute Passthrough** — keine Hostparameter,
> `hasEditor() == false`. Probeeqs EQ-DSP gehoert zu P6, die Oberflaechen
> kommen aus Figma. Wenn P6 die DSP bringt, **muss** das Kanon-Bein A16
> umgeschrieben werden.
> 🔑 Zwei Fehler fand der Gegenpfad speichern↔laden, nicht das Nachdenken:
> `active_probe` verlangt laut Kind-Matrix §2.1 genau ein `Parameters`-Kind,
> und `Zustand::parameters` war `{}` — also **Nullen statt `standardSatz()`**,
> und 0 Hz liegt ausserhalb von `band.0.freq_hz`.
>
> **S9 Abschnitt 3 ist gebaut** (23.08., `42bfe6e` + `ff0e0b8`). Damit stehen
> alle drei Bauabschnitte von `SONDE-007b`:
> - `state::Lebenslauf` im gemeinsamen Kern ist §53.5 als Code. Neu ist nicht
>   der Automat, sondern dass er **Zaehne** hat: die Hoer-Markierung faellt
>   unter „audio-neutral", und `EqCopLebenslaufTest` misst das an AUDIO —
>   dieselbe Markierung, die `EqCopMarkierungTest` faerben laesst, faerbt vor
>   der Klassifikation **kein einziges Sample**. `darfBrokerStarten()` ist der
>   eine Haken, an den SONDE-010 den Spawn haengt.
> - Installer-Manifest: Vertrag (`schemas/installer/`), Auslieferung
>   (`install/nakama-installer-v1.json`) und `Install-Nakama.ps1` mit beiden
>   Haelften (installieren ↔ Rueckweg) samt NAK-41-Riegel. **NAK-32 ist damit
>   geschlossen**; NAK-41 bleibt als Sachlage bestehen, ist aber kein Blocker
>   des Installer-Schritts mehr. Der committete Stand traegt bewusst
>   `sha256: null` = nicht ausliefer-bar.
> - Kanon 21 → **23** (A17 Installer-Manifest, B8 Lebenslauf), 23/23 gruen.
>
> ⚠️ **Verhaltensaenderung, die ein Pruefer zuerst ansehen sollte:** eine
> `legacy`-Instanz faerbt nicht mehr. §53.5 nennt `legacy` „immer passiv" —
> gewollt, aber es ist der einzige Punkt, an dem dieser Aenderungssatz das
> Verhalten des Produkts aendert.
> ✅ **`pluginval` 8 SUCCESS auch am Main-Bundle** (`SONDE-007b.md` B13) —
> gerade dort beruehrt der Aenderungssatz den Audiopfad.
> ⚠️ **Selbstaudit-Korrektur, damit sie niemand wiederholt:** dieses Blatt und
> das Manifest trugen kurzzeitig „pluginval nicht auffindbar" samt einem neuen
> Punkt NAK-53. Falsch — das Binary liegt unter `%TEMP%\pluginval.exe`, und
> **NAK-26 sagt das seit dem 21.08. woertlich**. Der Fehler war nicht die
> Suche, sondern einen bestehenden offenen Punkt nicht zu lesen, bevor ein
> neuer angelegt wird. NAK-53 ist zurueckgenommen, NAK-26 hat den Nachtrag.
>
> **T2 fuer S9 ist zur HAELFTE gefahren** (23.08. nachts, Commit `d351d4a`,
> Manifest §5): Urteil **NEEDS_WORK** auf Teil A. Gemessen und gehalten haben
> der §53.5-Automat, der Scannerlauf-Pfad (unabhaengig nachgezogen: der Automat
> hat genau vier Aufrufer, die Atomic genau einen Schreiber und einen Leser),
> Behauptung 13 an echtem Audio, der Installer-Rueckweg samt NAK-41-Riegel und
> die NAK-32-Buchfuehrung. Eigener Kanon-Lauf mit `-Bauen`: **23/23 gruen,
> beglaubigt**.
> **Ein Befund, im selben Zug geschlossen (T2-1):** `SondeProcessor` las
> `klassifikation()`/`darfBrokerStarten()` ohne `zustandSchloss`, waehrend
> `setStateInformation` den Automaten unter genau diesem Schloss schreibt — und
> waehrend das Main-Bundle dieselben zwei Methoden mutex-schuetzt
> (`PluginProcessor.cpp:531-540`). Heute folgenlos, aber Behauptung 15 bewirbt
> `darfBrokerStarten()` als die Stelle, an die **SONDE-010** den Spawn haengt.
> ⚠️ Ein Fehlalarm ist als solcher protokolliert (§5.2): der Beglaubigungsriegel
> vergleicht die neueste Quelle im GANZEN Baum gegen jede Binaerdatei und
> schlaegt an, sobald ein Ziel zu Recht nicht neu gelinkt wird. Kein Beweis —
> nachmessen, welche Quelle wirklich zu welchem Ziel gehoert.
>
> **T2 Teil B ist gefahren** (23.08. mittags, Manifest §5.5–§5.9, zweite frische
> Session): Urteil **NEEDS_WORK**. Die fuenf offenen Dateien sind jetzt gegen
> die Quelle gemessen — nicht gelesen, sondern **gefahren**: acht
> Configure-Laeufe mit verdorbenem Identitaetsmanifest, zwei Mutationsproben am
> Bauskript, zwei Regelproben am Installer-Bein, jede Mutation byteweise
> zurueckgenommen (SHA-256 gegengeprueft). Eigener Kanon-Lauf mit `-Bauen`:
> **23/23 gruen, Exit 0, beglaubigt**.
> **Gehalten haben**: die Verweigerung unvergebener Identitaeten (vier Proben,
> viermal Exit 1 an `NakamaIdentitaet.cmake:74/90/101/116`) · der umgedrehte
> Quellfrost **fuer `main`, beim Fallen vorgefuehrt** · die Zwei-Wege-Messung
> der Class-IDs ist **keine Tautologie** (kein `.cmake` im Baum enthaelt eine
> CID; JUCE rechnet, der Test misst am Artefakt) · alle drei Bundles gemessen
> (93/93 selbst gefahren) · A17 24/24 samt Gegenprobe, beide Leermengen-Fallen
> zu · `NAKAMA_BUNDLE_MAIN` deckt genau die acht Ziele, die `src/` uebersetzen.
> **Vier Befunde, alle offen (T2-2…T2-5, Sammelzeile NAK-55):**
> - **T2-2** — `NakamaIdentitaet.cmake` prueft `null` bei `produktname`,
>   `bundle`, `plugin_code`, **nicht** bei `hersteller.name`/`hersteller.code`.
>   Gemessen: `hersteller.code: null` konfiguriert **gruen** durch, JUCE setzt
>   seinen Vorgabewert `Manu` — beide Class-IDs **aller drei** Bundles haengen
>   daran. Der Dateikopf sagt zu, dass genau das nicht passiert.
> - **T2-3** — der Quellfrost deckt **eines von drei** Zielen. Vorgefuehrt:
>   dasselbe Literal faellt im `EqCopilot`-Block auf und bleibt in
>   `nakama_sonde_ziel` unsichtbar. Ein zusaetzlicher Blockfrost genuegt.
> - **T2-4** — ein `art` ausserhalb `vst3|broker` faellt durch alle zwoelf
>   A17-Regeln und landet im Broker-Zweig des Skripts.
> - **T2-5** — ausgeliefert wird die innere Binaerdatei, nicht das Bundle;
>   `-Rueckweg` laesst nach einer Erstinstallation ein leeres `.vst3`-Gehaeuse.
> ⚠️ Eine Zahl war falsch und ist berichtigt: K2b/K2c messen gegen **13**
> Verbraucher, nicht 12 (§5.8). Dieselbe 12 steht noch in `CLAUDE.md` —
> **nicht** angefasst, weil dort eine parallele Session uncommittete Aenderungen
> hat (gleiche Sperre wie NAK-54).
>
> **Naechster Schritt: T2-2 und T2-3 schliessen** — beides sind kleine, klar
> umrissene Ergaenzungen, und beide sitzen auf dem Riegel, der NAK-52
> ueberhaupt begruendet hat. T2-4/T2-5 sind Vertragsarbeit und koennen mit
> `SONDE-010` laufen. **Kein PASS auf S9, solange T2-2/T2-3 offen sind; T1
> steht weiterhin ganz aus.** Danach S10–11 (`SONDE-008`).
>
> **Aber vorher zwei Dinge, die Vorrang haben:**
>
> 1. ~~35 unbearbeitete Antworten~~ — **eingearbeitet am 22.08. spaet**
>    (alle 35 mit Status `eingearbeitet`; heute in `docs/plan/fragen.json`
>    unter `beantwortet`, lesbar als `docs/ANTWORTEN-OFFEN.md`). Jede traegt Wortlaut
>    und Ergebnis in `design/abnahmen/2026-08-22-hub-antworten-35.md`; fuenf
>    Entscheide stehen im Register in `CLAUDE.md` (Rangfolge Implementplan vor
>    Figma · Wortmarken aus dem Figma-Export · Hoer-Markierung nur mit Signal ·
>    Quellen×Band-Gitter als Gen-Uebersicht · Entscheidungstor Gen offen).
>    **Was daraus offen ist: zwoelf Folgefragen** (Abnahme, Abschnitt 6) —
>    ueberall dort, wo der User den Entwurf verworfen hat, der Alternativtext
>    aber zwei Formen anbot, und dort, wo eine Frage schlecht gestellt war
>    (`U6.8`, `U9.3`, `U9.4`: „macht kein sinn die frage", „verstehe frage
>    nicht"). **Nichts davon wurde geraten.**
>    ✅ **Erledigt sich mit dem 23.08.:** die zwoelf Folgefragen brauchen keinen
>    Deploy mehr. Sie stehen als Karten in `docs/plan/fragen.json` und werden
>    mit dem Skill **`/fragen`** einzeln im Chat gestellt. (Das alte Hindernis
>    war genau diese Kette: `briefing-app.tsx` filterte jede Karte still weg,
>    die nicht auch in `briefing-hub/data/friendly-copy.ts` stand — eine neue
>    Frage brauchte Katalog-Eintrag **und** Deploy.)
> 2. ~~**NAK-50: Der User will den Hub im Nimbalyst-GUI statt auf der Seite**~~
>    — **entschieden und gebaut am 23.08.** Aus der Praeferenz vom 22.08.
>    („EIGENTLICH ist die seite unnötig noise") wurde ein Entscheid: „*saeite
>    ganz weg, alles im repo + Skill bauen, der automatisch die offenen fragen
>    nach und nach an mich stellt und gleich einarbeitet in den plan*". Die
>    asynchrone Antwortfunktion ist **nicht** verlorengegangen, sie hat den Ort
>    gewechselt: Karten in `docs/plan/fragen.json`, gestellt vom Skill
>    `/fragen`, Antwort woertlich mit Datum. Der Planstand wird seither
>    gerechnet. Register-Zeile in `CLAUDE.md`, Punkt geschlossen in
>    `docs/offene-punkte.md`.
>
> ## 👤 Was beim User liegt (nur er kann es)
>
> ~~**Der OpenWiki-Schluessel.**~~ — **erledigt sich am 23.08.**, auf dein Wort
> („*verwerfe das mit dem API key wir machen das manuell*"). Der taegliche
> Workflow ist geloescht, der Schluessel wird nicht mehr gebraucht; das Wiki
> wird im Lauf einer Session von Hand aufgefrischt (NAK-51 verworfen).
>
> **Die Planblatt-Automation einschalten** (optional):
> `nimbalyst-local/automations/planstand-nakama.md`, taeglich 09:00 — angelegt,
> aber nicht aktiviert; das macht der User selbst ueber die Kopfleiste.
>
> **Klarstellungen vom 22.08. abends, damit sie niemand wieder falsch liest:**
> der Fremdmodell-Riegel bindet **nur Gemini** („gpt IST WAS anderes. das galt
> nur für gemini. gpt vertraue ich.").
>
> ~~**OpenWiki ist das Kontextsystem, auf dem der Workspace aufbaut**
> („das workspace hier baut auf openwiki als kontextsystem auf") — nicht gegen
> `docs/` abwaegen, nicht als Zusatzquelle behandeln, nicht abschalten
> vorschlagen.~~ — **AUFGEHOBEN am 25.08.**, neues User-Wort: *„entferne das
> komplette openwiki konstrukt"*. Vorausgegangen ist eine Messung der
> tatsaechlichen Nutzung (Transkripte beider Werkzeuge): Claude hat das Wiki
> **nie** als Kontext gelesen, Codex genau **einmal**, und mehrfach aktiv aus
> Suchen ausgeschlossen (`--glob '!openwiki/**'`). Das Konstrukt ist restlos
> entfernt — Ordner, MCP-Registrierungen, Skills, Server, Prozesse. Nichts
> davon suchen, nichts davon wiederbeleben. Wortlaut und Umfang:
> `design/abnahmen/2026-08-25-openwiki-entfernt.md`.
>
> **Neu seit 23.08.:** `docs/PLAN-STAND.md` wird **gerechnet**, nicht gepflegt
> (`tools/plan/planstand.py` aus `docs/plan/plan.json` + den Urteilsmarken in
> `docs/beweise/`); `tools/hooks/planstand.sh` haelt es nach jedem Befehl
> frisch und committet es allein. **Keine Zahl mehr von Hand hier eintragen** —
> das Blatt sagt den Stand, dieses Kapitel sagt das Warum. Die
> Nimbalyst-Automation (`nimbalyst-local/automations/planstand-nakama.md`,
> taeglich 09:00) ist nur noch das Netz fuer Zeiten ohne Session.
>
> **Unverbuchte Fremd-Aenderungen im Baum** (der User 22.08.: „alle anderen
> sitzungen sind lange schon fertig und ruhen" — sie sind also verwaist, nicht
> in Arbeit): vier Dateien unter `briefing-hub/` und zwei **Loeschungen** unter
> `eq-copilot/design/` (`ASSET-KIT.md`, `eq-copilot-material-preview.png`).
> ~~Nicht blind committen — erst pruefen, ob die Loeschungen gewollt waren.~~
> **Geklaert 23.08.:** die briefing-hub-Arbeit ist geborgen (`6e67662`), die
> Loeschungen sind auf User-Wort committet („ASSET KIT is alt und längst
> verworfen"), die Motion-Spezifikation ebenso (`7020f06`).


> Stand: **22.08.2026, nach S7 (`SONDE-006`).** Davor: 21.08., Kontext-Inventur und Interview mit dem User.
> Die Wahrheit steht in `CLAUDE.md` (Wahrheitskern + Register der User-Entscheide
> mit Zitat). Dieses Blatt sagt nur, wo wir stehen und was als Nächstes kommt —
> es wiederholt die Wahrheit nicht. Wenn hier und in CLAUDE.md etwas
> Verschiedenes steht, gilt CLAUDE.md; dann dieses Blatt berichtigen.

> ~~**Seit 22.08. gibt es den Nakama-Hub**: die Seite
> <https://nakama-briefing.philipld.chatgpt.site>~~ — **abgeschafft am
> 23.08.2026** (User-Wort im Register; NAK-50 geschlossen). Sie hatte keine
> automatische Aktualisierung und driftete deshalb, samt Planstand. An ihre
> Stelle treten zwei Dinge im Repo: der **gerechnete** Planstand
> (`docs/PLAN-STAND.md`, Quelle `docs/plan/` + Urteilsmarken in
> `docs/beweise/`) und der Skill **`/fragen`**, der die offenen Punkte einzeln
> im Chat stellt und die Antwort wörtlich einarbeitet
> (`docs/plan/LIES-MICH.md`). `briefing-hub/` bleibt als Verlauf liegen —
> nicht deployen, nicht füttern. Dieses Blatt bleibt der technische Einstieg.

## Was heute passiert ist

- Neun Prüfer haben beide Repos, drei Memory-Verzeichnisse, Hooks, Docs,
  Design-Artefakte, Code und Git-Historie inventarisiert (Kontextkarte: 20
  Widersprüche, 15 Design-Richtungen in 10 Tagen, fünf parallel lebend, ein
  Produktentscheid in fünf Kopien ohne Zitat). Der User hat 20 Fragen beantwortet.
- Daraus: neue `CLAUDE.md`, Hooks lesen den Wahrheitskern aus CLAUDE.md (keine
  zweite Kopie), alte Design-Richtungen im Archiv, Prisma als Studie geparkt,
  Recherche im Archiv, Memories bereinigt, Nakama-Design auf „Figma ist Quelle".
- Zement-Brüche repariert: `eq-snapshot.schema.json` war seit 15.08. kein JSON
  (neues Kanon-Bein A11); CMake-Version 0.1.0 → 0.3.0 mit Configure-Riegel.

## Wo der Bau steht

- **P0 gebaut:** Beweis-Runner (S0), Aux-/PDC-Messgerät (S1), Identität (S2),
  Hostbrücke + Hostkontext-Test (S3), Termin-B-Messgerät (S3b), v3-Vertrag JSON
  (S5) und FlatBuffers (S6). **P1 begonnen:** State-Schema 2 (S7, 22.08.).
  Manifeste in `docs/beweise/`.
- **Offen an S5/S6:** nur noch der Schließungsvorbehalt §65 (erst nach S4).
  T2 lief in vier Runden; **Runde 4 ist am 21.08. geschlossen** — der
  Selbstbezug (`uoffset` 0) ist nicht mehr an einem Fixture, sondern an der
  **Klasse** belegt (6215 Byte-Mutanten, 143 → 0 Abweichungen), die stale Zahl
  in P10 ist nachgemessen (207) und der tote Teilausdruck in `utf8Gueltig` ist
  raus. Alles in `docs/beweise/SONDE-005b.md` §6.5 + P14/P15.
  **Offen bleibt das Urteil selbst:** eine T2-Runde 5 mit frischem Kontext gegen
  `git diff 4cf05b9..HEAD` ist nicht gelaufen — Befunde schliesst, wer nacharbeitet,
  urteilen darf nur ein Pruefer, der den Umbau nicht geschrieben hat.
- **Termin A ist gemessen (22.08. 00:17 + 00:27): geht, recall-stabil** — zwei
  getrennte Nebenwege samplegenau (Versatz 0), Beweis `docs/beweise/termin-a/`.
  **Termin B ist gemessen (22.08. 12:45–13:27) und S4 gebaut** — Capabilityreport
  `docs/beweise/SONDE-004.md`, Bits in `eq-copilot/identity/host-capabilities-fl-v1.json`
  (Kanon A13). **Nur Gate G0 (T3) steht in P0 noch aus.** Beide Messgeräte sind
  in `C:/Program Files/Common Files/VST3/` installiert (FL scannt VST3 nur dort).
- **Kanon:** 19 Beine in `tools/beweise.ps1` (seit 22.08.: B2 `EqCopStateMigrationTest`,
  A12 `erzeuge_state_fixtures.py --pruefen`, A13 `pruefe_host_capabilities.py`,
  A14 `pruefe_kern_identitaetsfrei.py`);
  letzter vollständiger Lauf siehe jüngstes Manifest in `docs/beweise/` (`SONDE-007a.md`: 19/19).

## ▶ Erledigt am 22.08. (Termin B + S4)

1. ~~Termin B über den FL-MCP fahren~~ — **gemessen 12:45–13:27**: der User legte
   `EqCop-Host-Probe` auf den Piano-Kanal, zeichnete den Automationsclip,
   schaltete Smart disable ein, exportierte und sicherte den Bericht; Claude
   fuhr Live, Seeks (ohne/mit Stop), Pattern-Schleife, Stummschaltung über den
   MCP. **Lehre:** `fl_set_song_position` — Modus 0 = ms, 1 = s, 2 = absolute
   Ticks (die Werkzeugbeschreibung sagt 1 = ms, 2 = s — falsch; im Fork
   `fl-studio-mcp` berichtigen). `fl_save_project` fehlt dem Fork weiterhin —
   Speichern blieb Strg+S beim User.
2. ~~Klicklisten A und B nachziehen~~ — beide tragen einen Stand-Hinweis mit den
   Berichtigungen (VST3 nur unter `Common Files\VST3`; Nebenwege als
   „Sidechain to this track"; Song ≥ 16 Takte; Arbeitsteilung).
3. **Entschieden (User 24.08., 13:26): „wird gelöscht, ich bin nicht am pc. muss
   vertagt werden"** — `C:/Program Files/Common Files/VST3/eq-copilot/`
   (845 MB alte Repo-Kopie vom 13.08. mit zwei alten `EQ-Copilot.vst3`, die FL
   mitscannt) wird entfernt, sobald der User am PC ist. Fertiger Befehl,
   PowerShell **als Administrator**:
   ```powershell
   Remove-Item -LiteralPath "C:\Program Files\Common Files\VST3\eq-copilot" -Recurse -Force
   ```
   Danach `docs/offene-punkte.md`/hier als erledigt markieren (kein NAK-Eintrag
   vorhanden, war ein reiner Handgriff — U7 in `docs/plan/fragen.json`).
4. ~~S4~~ — **gebaut**: `docs/beweise/SONDE-004.md`,
   `eq-copilot/identity/host-capabilities-fl-v1.json` (**2 supported, 8
   unsupported** — T2 hat `presentation_latency`, `aux_priority_sidechain` und
   `aux_compare_pre` herabgestuft: Golden nicht erbracht; Runde 2 PASS), Kanon A13.
   **Offen: Gate G0** (T3, eigene Session: `/c-review` auf den Bridge-Patch +
   Codex, Bruchaufträge Gate 1 und Gate 5) — danach fällt der
   Schließungsvorbehalt §65 für `SONDE-005`. **Termin A2** (NAK-44: PDC-Last,
   Bus-Identität, L/R) hebt beide Aux-Bits, wenn der User ihn fährt.

## ▶ So startet der Projektleiter eine Bau-Session

Neue Session in diesem Workspace aufmachen, diese zwei Zeilen einfügen, fertig.
**Stand 23.08. abends ist das die T2-Prüfung von S9**, nicht der nächste Bau:

```
Pruefe S9 (SONDE-007b) als T2 Teil B mit frischem Kontext gegen git diff 4b500a4..HEAD.
Teil A ist gefahren (Manifest §5). Ungeprueft: tests/IdentityTestMain.cpp,
cmake/NakamaIdentitaet.cmake, tools/eq-copilot/pruefe_installer_manifest.py,
schemas/installer/nakama-installer-v1.md, plugin/CMakeLists.txt.
Urteil nach docs/beweise/SONDE-007b.md, Abschnitt 5.
```

⚠️ **Der Basispunkt ist `4b500a4`, nicht `657fe57`.** Bis zum 23.08. stand hier
der falsche — `657fe57` ist S9s *Hub*-Commit, also die Mitte des Tickets; ein
Prüfer, der ihn kopiert, misst nur Abschnitt 3 und sieht weder die
Identitätsumstellung (NAK-52) noch die beiden neuen Bundles. S9 sind fünf
Commits: `8e32baf` · `fdece75` · `657fe57` · `42bfe6e` · `ff0e0b8`; der Stand
davor ist `4b500a4`.

Die drei Stellen, an denen ein Prüfer bei **S9 Abschnitt 3** zuerst graben
sollte (aus dem Selbstaudit, ehrlich benannt statt versteckt):
1. **Die Verhaltensänderung an der Hör-Markierung.** `legacy` färbt nicht mehr.
   Ist der Term an der richtigen Stelle (vor `markierung.verarbeite`, nach dem
   Analyse-Abgriff), und kann die Atomic-Spiegelung `istMainKlassifiziert`
   veralten? Sie wird nur unter `bindungMutex` geschrieben — reicht
   `memory_order_relaxed` für einen Audiothread, der sie liest?
2. **Die explizite Initialisierung hängt an `setzeBindung`.** Das ist heute der
   einzige User-Akt, der die Klasse setzt. Gibt es einen zweiten Weg, auf dem
   `zustand.common.klasse` `main` wird, ohne dass der Automat es erfährt?
   (`neueSensorId`, `setStateInformation`, Editor-Pfade.)
3. **A17s Ableitung.** Der Quellpfad wird aus `cmake_ziel` + Bundlename
   gebildet. Stimmt die Formel noch, wenn ein Ziel andere JUCE-Formate baut,
   und was passiert bei einem Ziel ohne `cmake_ziel`-Feld?

Das ist keine Wahl, sondern die Sessionregel (`docs/bauaufteilung-sonden.md`
§0: „1 Session = 1 Ticket + sein Beweismanifest + **sein Frischkontext-Prüfer**").
S8 hat Manifest und T1, aber kein Prüferurteil — es ist damit **offen**. Ein
T2-Prüfer darf den Umbau nicht selbst geschrieben haben; den frischen Kontext
liefert genau das Aufmachen einer neuen Session, und diese Gelegenheit ist
verbraucht, sobald dieselbe Session anfängt zu bauen. Der Rückstand wächst
sonst weiter: S5, S6 und S8 tragen alle „T2 offen".

Danach S9, mit denselben zwei Zeilen in der Bauform:

```
Baue S9 (SONDE-007b) nach docs/bauaufteilung-sonden.md.
Manifest nach docs/beweise/SONDE-007b.md, T1 + T2.
```

Die drei Stellen, an denen ein Prüfer bei S8 zuerst graben sollte (aus dem
Selbstaudit, ehrlich benannt statt versteckt):
1. **Die Kopf-Fassade** leitet Includes und Defines per Generatorausdruck ab.
   K2 kann Generatorausdrücke zur Konfigurierzeit **nicht** auswerten — steht
   so im Modulkopf. Ist die Lücke wirklich nur durch K3 gedeckt?
2. **K2b vergleicht gegen genau ein Ziel** (`EqCopilot`). Ob sein
   Ausschlusssatz trägt, ist nachgemessen (22.08., Manifest B8): von
   `JUCE_SHARED_CODE`, `JUCE_STANDALONE_APPLICATION` und
   `JUCE_VST3_CAN_REPLACE_VST2` kommt **keines** in irgendeinem Header der vier
   Kernmodule vor; `JUCE_MODULE_AVAILABLE_` trifft zweimal, beide in
   `juce_core/native/juce_BasicNativeHeaders.h` auf `juce_opengl` gegated —
   ein Modul, das weder der Kern noch `EqCopilot` hat, beide sehen es also
   gleich undefiniert. Offen bleibt die Frage für **drei** Ziele: welches ist
   dann die Referenz?
3. **A14s Nadelliste** kommt aus `plugin-identities-v1.json`. Abgeleitete
   Identitätsträger wie `JucePlugin_AAXIdentifier` / `_CFBundleIdentifier`
   (`com.evenacadia.EqCopilot`) stehen dort nicht — geprüft und für gedeckt
   befunden: der Firmenname ist Teilstring jeder abgeleiteten ID, und A14
   sucht Teilstrings. Gegengemessen 22.08.: `com.evenacadia` steht ohnehin
   **auch im gebauten Bundle nicht**, weil beide Makros macOS-/AAX-Wege sind,
   die der Windows-VST3-Bau nie übersetzt. Ein Prüfer sollte trotzdem fragen,
   ob es einen Identitätsträger gibt, der *nicht* den Firmennamen enthält.

Mehr braucht es nicht: die SessionStart-Hooks legen Wahrheitskern, Hub-Stand,
Design-Stand und Git-Stand von selbst vor. **Nur das Ticket muss genannt
werden** — „go" ohne Ticketgrenze widerspricht der Sessionregel in
`docs/bauaufteilung-sonden.md` („1 Session = 1 Ticket + sein Beweismanifest
+ sein Frischkontext-Prüfer") und endet in Code ohne Manifest.

**Prüfstein für den User:** liegt am Ende ein neues Manifest in
`docs/beweise/`? Wenn nein, ist die Session **offen**, egal was sie meldet.
Für das jeweils nächste Ticket dieselben zwei Zeilen mit der nächsten Nummer
aus der Tabelle in `docs/bauaufteilung-sonden.md`.

## ▶ Der eine nächste Schritt (Technik): S9 — `SONDE-007b`

**S8 / `SONDE-007a` ist gebaut (22.08., Manifest `docs/beweise/SONDE-007a.md`):**
`NakamaKern` ist eine echte Static-Lib mit den vier geteilten Quellen
(`state/*.cpp` + `vertrag/NakamaVertrag.cpp`), **einmal** übersetzt statt je
Ziel; angebunden über `nakama_kern_anbinden()`. Vier Riegel, jeder beim Fallen
vorgeführt (K1 Präprozessor · K2 Linkhülle · K2b gleiche JUCE-Konfiguration ·
K3 = Kanon-Bein **A14**, misst das Artefakt). Kanon 18 → **19**.
**T2 ist offen** — kein Frischkontext-Prüferurteil im Manifest.

**Drei Dinge, die S9 aus S8 mitnehmen muss** (sonst wird es teuer):

1. **Der Kern übersetzt gegen JUCE-KÖPFE, nicht gegen JUCE-Module.** Die
   Implementierung kommt vom verbrauchenden Ziel. Die drei Bundles müssen
   daher `juce_core`, `juce_events`, `juce_data_structures` und
   `juce_cryptography` selbst linken — genau das tut `nakama_kern_anbinden()`;
   ein Ziel, das den Kern ohne diese Funktion anbindet, verliert zur Linkzeit
   Symbole. Grund und Messung: Kopf von `eq-copilot/cmake/NakamaKern.cmake`.
2. **K2b vergleicht heute gegen `EqCopilot`.** Kommen Probeeq und Suna dazu,
   muss entschieden werden, ob der Kern gegen *alle drei* geprüft wird oder
   die drei untereinander gleich konfiguriert sein müssen — die Funktion
   `nakama_kern_konfig_pruefen(kern referenz)` nimmt eine Referenz.
3. **NAK-52:** §53.4 verlangt Identität ausschließlich aus
   `plugin-identities-v1.json`. S8 hat nur die Kern-Hälfte gebaut; die
   Target-Schichten lesen die Werte weiterhin nicht, sondern tragen sie als
   CMake-Literale. Das ist **S9s Auftrag** — inklusive Umbau von
   `EqCopIdentityTest`, das die CMake-Quelle heute in genau dieser Textform
   einfriert (`tests/IdentityTestMain.cpp:257-263`).

Vorher lesen: `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Errata-Block zuerst,
§53.4 und §53.5), `docs/bauaufteilung-sonden.md` S9-Zeile,
`docs/beweise/SONDE-007a.md` §1 („Was dieses Ticket ausdrücklich NICHT
behauptet").

**Vor der Installation des neuen Bundles:** NAK-41 — ein Projekt, das der neue
Build speichert (Schema 2), verliert im 16.08.-Build seine Messpunkt-Identität.
Installation bleibt User-Klick (NAK-32).

Beim Bauen gilt: 1 Session = 1 Ticket + Beweismanifest + Frischkontext-Prüfer;
Code ohne Manifest heißt *offen*. Beweislauf:
`pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/<Ticket>.md -Anhaengen -Titel '<Ticket>'`.

## Was beim User liegt

| Was | Wo |
|---|---|
| ~~FL-Termin B~~ — beide Termine erledigt 22.08. | Optional offen: Teil 3 der Klickliste B (`nakama-altprojekt.flp` = Legacy-Fixture für SONDE-002) und das Wegräumen von `Common Files\VST3\eq-copilot\` (Admin-Klick) |
| Figma-Stände der drei Apps (Gen, Probeeq, Suna) | per Figma-MCP nach `design/assets/figma/` holen; `design/` übersetzt sie |
| Installation des 21.08.-Bundles (mit Hostbrücke, Version 0.3.0) | erst mit dem nächsten bewiesenen Stand, per UAC-Klick |

## Offen-Set

`docs/offene-punkte.md` — neu seit 22.08.: NAK-40 (`instance_id` bytegleich vs.
hex32 der v3-Adresse, SONDE-010), NAK-41 (Schema-2-Stand im alten Build = stiller
Identitätsverlust; vor der Installation wissen), NAK-42 (Rust klassifiziert die
ungültigen DTOs noch nicht — vor G1), NAK-43 (`presentation_latency`: ein
verworfener Wertwechsel ohne Protokoll — vor SONDE-009), NAK-44 (Termin A2:
PDC-Golden für `aux_priority_sidechain`, Bus-Identität/L/R, FL-Version). NAK-33 trägt den Nachtrag „State
trägt 13, UI-Frage offen". Neu seit 21.08.: NAK-30 (Umbenennung zu Nakama
Studio / Gen / Probeeq / Suna = Identitätsticket), NAK-31 (`analyze-track.py`
liegt im FL-Repo), NAK-32 (`install/` unversioniert), NAK-33 (12 oder 13
Parameter je Band), NAK-34–37 (HostProbe-Zählung, Hör-Markierungs-Verriegelung,
Broker-Binaries, AuxSpikeTest nicht im Kanon).

## Parallele Sessions

Am 21.08. lief parallel die SONDE-005b-Session (T2 Runde 3, Commit
`4f7182b` 21:57 — Fund: der FlatBuffers-Verifier prüft in C++ kein UTF-8,
C++ stürzte ab, wo Rust sauber ablehnte; gefixt, Binärkorpus 47). Der
geteilte Index hat dabei **elf gestagte Dateien dieser Inventur-Session in
jenen Commit mitgenommen** (CLAUDE.md, NEXT-SESSION, plugin-wissen,
Bauaufteilung, Entwurf-Errata (l)/(m), offene-punkte NAK-38/39, lib.rs- und
Probe-Kommentare, eq-aggregat-Schema, tokens.json, depth-primer): Inhalt
vollständig und richtig, Zuordnung nicht — keine Historie umschreiben.
Lehre steht in `reference_werkzeug-landminen-windows` (Memory) Nr. 6:
bei paralleler Session `git add <dateien>` + `git commit` ohne Pfade, und
zwischen beidem keine Pause.
