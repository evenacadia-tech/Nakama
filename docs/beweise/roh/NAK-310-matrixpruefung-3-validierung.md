# NAK-310 — Validierung der Matrixprüfung 3 (lesender Opus-5.5-Thread, Effort max)

**HEAD vorher:** `4a831da3d62e3df3b17caad450503e71781c0e2a` (= `origin/master`, erste Messung des Laufs) · **HEAD nachher:** `4a831da3d62e3df3b17caad450503e71781c0e2a` (23.09.2026, 16:05:46 vor dem Schreiben dieser Datei und 16:11:14 danach, je `git rev-parse HEAD`).

Manifest gelesen ausschließlich über `git show 4a831da3:docs/beweise/NAK-310.md` (der Arbeitsbaum trägt währenddessen den Anhang des Dirigenten); Code am Arbeitsbaum gleich `4a831da3` (außer dem Manifest keine geänderte getrackte Datei). Nur Lesebefehle und drei Primärquellen im Netz (am Ende); kein Bau, kein cargo, kein Test, kein Python, kein FL, keine Pipe; geschrieben nur diese Datei. Zeilen ohne Datei meinen `docs/beweise/NAK-310.md` an `4a831da3`; MP3-Auftrag = `docs/beweise/roh/NAK-310-matrixpruefung-3-auftrag.txt`, V1/V2 = `docs/beweise/roh/NAK-310-matrixpruefung-1-validierung.md` / `-2-validierung.md`. Alle Zeilennummern des Prüfers stimmen (M-36 `:941`, M-95 `:1015`, M-109 `:1023`, §7.4 `:1892-1895`). Randnotizen: die Grenze „in jeder Etappe" steht an `4a831da3` in `:1194-1199` (`:1055-1059` ist heute §6.8; die Angabe stammt vom Stand `abbc8dda`); das Rohurteil trägt weder URTEIL- noch FERTIG-Zeile (MP3-Auftrag `:23-27`).

Alle drei Befunde liegen im Prüfbereich: M-36, M-95 und M-109 tragen „(Nacharbeit 2)", stehen im Fixdiff (vorher `6fefac1b:814`, `:888`, `:896`) und in der Durchsicht §14.6 (`:3269`, `:3287`, `:3311`); MP3-Auftrag Frage 2 (`:19`) fragt genau danach.

---

## F-1 — Startmeldung: gemessen wird die reine Funktion, nicht die Ausgabe von `main.rs` (M-36, `:941`)

### 1. Quelle

- M-36 (`:941`): Ereignis „Statusabfrage; Startmeldung"; Zusage, zweite Hälfte: „die Startmeldung von `main.rs` nennt nur Endpunkte, die bedient werden (v3 immer, v2 nur mit Griff)"; Reihenfolge: „die Meldung bildet eine reine Funktion der Bibliothek aus dem Status des Laufs, `main.rs` gibt sie nur aus (Bauinhalt)"; Test: „A4 **NEU** `komposition_status_nennt_v2_belegung`; A4 **NEU** `startmeldung_nennt_nur_bediente_endpunkte` an der reinen Funktion (Status mit und ohne v2-Griff); A4 **NEU** Quelltextwache nur dafür, dass `broker/src/main.rs` diese Funktion ruft"; Rotbeweis: „`:388` wieder bedingungslos → rot; die Funktion nennt den v2-Namen auch ohne Griff → `startmeldung_nennt_nur_bediente_endpunkte` rot".
- §7.2 `:1565-1568` gleichlautend; Durchsicht `:3269`: „das Verhalten der Startmeldung von `main.rs` maß nur eine Quelltextwache · reine Funktion der Bibliothek mit Test `startmeldung_nennt_nur_bediente_endpunkte`; die Wache prüft nur den Aufruf".
- Die Stelle sagt, was der Prüfer behauptet.

### 2. Ablauf

1. Messung je Teil der Zusage: Z1 (`broker_status`: `pipe_name` nur mit Griff, `fehler` mit Belegung) misst `komposition_status_nennt_v2_belegung` an der Naht. Z2 (Startmeldung) misst `startmeldung_nennt_nur_bediente_endpunkte` nur als Rückgabewert der Funktion. Die Wache misst, dass der Quelltext von `main.rs` den Aufruf enthält (Aufbau). Keine Zeile liest, was `main.rs` schreibt; beide Rotbeweise brechen Status oder Funktion, keiner die Ausgabe.
2. Prüferfall nachgerechnet: `main.rs` ruft die Funktion, verwirft das Ergebnis und gibt wie heute `PIPE_NAME` aus → Funktionstest grün (Funktion unverändert), Wache grün (Aufruf steht), Statustest grün (Status unberührt). Z2 ist verletzt, nichts wird rot. Am Text reproduzierbar.
3. Heute (`broker/src/main.rs:52-59`): „EQ-Copilot-Broker läuft auf {`PIPE_NAME`} (Bindungen: …)" — nur der v2-Name, bedingungslos; den v3-Endpunkt nennt sie nie; dazu `:67` die Idle-Zeile. `broker_status` hat am HEAD keinen Aufrufer (`git grep broker_status -- broker eq-copilot tools`: nur `broker/src/lebenslauf.rs:13` und `:377`); die Startmeldung ist die einzige Endpunktauskunft des Binaries. Sichtbar ist sie nur beim Start von Hand: Gen startet verborgen, ohne Umleitung der Standardausgabe (`eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:612-621`, `SW_HIDE`, `CREATE_NO_WINDOW`).
4. Testnaht am HEAD: keine. `git grep CARGO_BIN_EXE -- broker` trifft nur `eqcop-broker-v3probe` (`broker/tests/broker_idle.rs:105`, `broker/tests/security_vectors.rs:187`) und `eqcop-store-crash-worker`; kein Test startet `eqcop-broker` oder liest `main.rs` (`git grep "main\.rs" -- broker`: nur ein Kommentar, `broker/src/bin/eqcop-broker-probe.rs:4`). Cargo baut das Binary für die Integrationstests mit, fährt es aber nie („Binary targets are automatically built if there is an integration test."). `echter_brokerprozess_beendet_sich_nach_letztem_client_selbst` (`broker/tests/broker_idle.rs:98-174`) startet trotz des Namens `eqcop-broker-v3probe` auf einem Probenamen und liest `BEREIT` und den JSON-Bericht. Dieses Binary hat eine eigene `main` (`broker/src/bin/eqcop-broker-v3probe.rs:28-144`) mit eigenen Argumenten (`<pipe-name> [sekunden] [--idle-self-exit]`) und eigener Ausgabe; es startet nur einen v3-Listener (`:50-61`): kein `broker_starten`, kein v2, kein Briefkasten, keine Startmeldung. `broker/Cargo.toml` hat kein `[[bin]]`; `src/main.rs` ist das Binary `eqcop-broker`. `main.rs` selbst ist in keinem Test startbar, denn `broker_starten` nimmt `PIPE_NAME` und den produktiven v3-Namen (`broker/src/lebenslauf.rs:152-169`); so auch die Durchsicht `:3289`: „`main.rs` wird nie gestartet, er öffnete die Produktions-Pipe".
5. Die reine Funktion ist ein Stellvertreter im Sinn von §8.4 (iii): ihr Rückgabewert ist Vorbedingung der Ausgabe, nicht das Ergebnis des Ereignisses „Startmeldung".

### 3. Zusagenlage

- M-36 (`:941`), Zusage wie oben; §12.4 `:2664`: „§7.2 v2-Griff als Option, Status, Startmeldung · M-36 · Zusage" — die Startmeldung ist als Zusage gebucht, nicht als Bauinhalt.
- §8.4 (iii) `:2218-2224`: „Misst die Zeile die Zusage unter ihrem Ereignis — oder eine Sandbox mit ersetztem Mechanismus, eine Testuhr ohne Produktwert, eine Textwache für ein Verhalten …? … eine Textwache trägt nur Aussagen über Text oder Aufbau".
- MP3-Auftrag Frage 2 (`:19`): „misst eine Zeile der Durchsicht (§14, vier Fragen) weiterhin einen Stellvertreter statt der Zusage unter ihrem Ereignis?"
- R-310-12 `:3017-3018`: „Ein Rotbeweis hält Zustand und Vorbedingung seiner Zeile und bricht das Ergebnis des Ereignisses" — für Z2 bricht keiner das Ergebnis des Ereignisses.
- R-310-3 (`:146-149`) sagt „im Status ehrlich gemeldet", §1 (`:102-103`) „ehrlicher `broker_status`"; die Startmeldung nennen beide nicht. Sie ist Zusage der Matrix seit Etappe 1 (`6fefac1b:814`). Der Gate-Text schweigt.

### 4. Urteil: BESTÄTIGT

Nur die Quelle genauer: die Zusage „ehrliche Startmeldung" steht in M-36 und §7.2; R-310-3 spricht vom Status. Produktgewicht gering (nur beim Start von Hand sichtbar).

**Klassenvorschlag: DEFEKT** — Stellvertreter statt Zusage unter ihrem Ereignis (§8.4 (iii), MP3-Auftrag Frage 2).

**Kleinster Fix am Text:** gemeinsam mit F-2 über die Naht `lauf` (F-2, Punkt 2.7): M-36 misst die Meldung an einer übergebenen Ausgabe. Ohne diese Naht bliebe nur, die Startmeldung aus der Zusage von M-36 zu nehmen und in §7.2 als Bauinhalt ohne Zusage zu führen.

---

## F-2 — Sitzungskennung: der Weg vom Argument bis `starten_mit` ist im Binary ungemessen (M-95, `:1015`)

### 1. Quelle

- M-95 (`:1015`): Zustand „Brokerbriefkasten auf einer Testbühne (Muster `buehne_mit`, …) mit Marke `1` in der Umgebungsfassade und der echten Sitzungsfassade"; Zusage (a) „Startgrund `Gestartet`, genau ein Sitzungsgriff offen", (e) „Die ganze Befehlszeile liest eine reine Lesefunktion der Bibliothek, die `broker/src/main.rs` ruft: ein ungültiger Wert von `--diagnose-sitzung` ergibt einen Aufruf ohne Sitzung (der Broker startet dann wie ohne Argument), … ein unbekanntes Argument ergibt weiter Exit 2"; Test „`sitzung_fehlt_oder_tot_kein_briefkasten` …; `sitzungsargument_rand` und `aufruf_lesen_rand` (reine Lesefunktionen); A4 **NEU** Quelltextwache `main_ruft_aufruf_lesen` nur dafür, dass `broker/src/main.rs` die Lesefunktion ruft und ihren Exitcode übernimmt".
- §7.4 `:1866-1873`: „… `main.rs` ruft nur sie und übernimmt ihren Exitcode. … `broker_starten` (`broker/src/lebenslauf.rs:130`) und die Kompositionsnaht der Etappe 3 reichen die Kennung an `briefkasten_starten` (heute `:173`) durch."; §7.2 `:1573-1574`; Durchsicht `:3287`.
- Die Stelle sagt, was der Prüfer behauptet.

### 2. Ablauf

1. Wer verspricht, dass die Kennung ankommt: M-94 (`:1014`) endet an den Argumenten von `CreateProcessW` (Plugin-Seite). M-95 misst Anfang (Lesefunktion) und Ende (`starten_mit` auf der Bühne, Kennung direkt übergeben); das Durchreichen dazwischen steht nur in §7.4 `:1869-1873`, und §12.4 bucht das Durchreichen durch die Naht als Bauinhalt (`:2668`: „§7.2 ab Etappe 5 reicht die Naht die Sitzungskennung durch · M-95 · Bauinhalt der Etappe 5"). M-97 (`:1017`) geht nicht durch `main.rs`: der Test ruft `briefkasten_probe_lauf_starten` im Testprozess (`broker/tests/briefkasten.rs:76`), der Probelauf setzt sich selbst aus `server_starten` und `starten_mit` zusammen (`broker/src/briefkasten.rs:914-924`) und bekommt die Kennung als Parameter (§7.4 `:1904-1905`) — ohne `main.rs`, `aufruf_lesen`, `broker_starten` und Naht; `eqcop-broker-v3probe` ist dort nur der Sitzungsprozess. Unter den Zeilen verspricht allein M-84 (`:1009`), dass der gestartete Broker die Kennung „erhält … und antwortet" — eine Livezeile ohne eigenen Bruch und heute ohne Wirkung: „Im Diagnoseprojekt antwortet kein Broker, solange Gen dort `legacy` ist" (`tools/fl/LIES-MICH.md:207-209`; M-84: „dann gilt M-85 (c)").
2. Kette im Produkt: Befehlszeile → `main.rs` → `aufruf_lesen` → `broker_starten(…, sitzung)` → Naht → `briefkasten_starten(…, sitzung)` → `starten_mit`. Gemessen: `aufruf_lesen` (rein) und `starten_mit` (Bühne, Probelauf). Ungemessen: (a) `main.rs` → `broker_starten`, nur die Aufruf-Wache; (b) `broker_starten` → Naht → `briefkasten_starten`, weder Zeile noch Wache. Heute nimmt keine der beiden Funktionen eine Kennung (`broker/src/lebenslauf.rs:130`, `:173`).
3. Prüferfall nachgerechnet: `main.rs` ruft `aufruf_lesen`, übernimmt den Exitcode, startet aber ohne Kennung → `aufruf_lesen_rand`, die Bühnenfälle und die Wache bleiben grün; der Broker startet mit `OhneSitzung`, der Briefkasten der Rolle `broker` bleibt den ganzen Lauf zu. Wirkung: fail-closed. Verletzt sind im Produktpfad M-95 (a), M-84 und die positive Hälfte von R-310-7; der Satz „Marke ohne gültige, lebende Prozesskennung → kein Briefkasten" hält in diesem Fall.
4. Gegenrichtung, ebenso ungemessen: erreicht über (a) oder (b) eine falsche, lebende Kennung `starten_mit` — etwa die des eigenen Prozesses, wie sie die Tests für den Testprozess benutzen (M-81, M-95 (a), M-100 (d)) —, bindet der Briefkasten an die Lebensdauer des Brokers statt an die des FL und bleibt über das FL-Ende hinaus offen, gegen R-310-7 („Endet der Prozess, schließt der Broker den Briefkasten …"). Die Nachprobe M-102 sähe das nur live, und live startet heute kein Broker (Punkt 1).
5. R-310-7 im gebauten Binary: die fail-closed-Entscheidung selbst ist an `starten_mit` gemessen (M-95 (b) bis (e)); welche Kennung im Binary dort ankommt, misst keine Zeile, in keiner Richtung.
6. Zeilen, deren Verdrahtung nur eine Wache oder nichts trägt (Muster „Quelltextwache nur dafür, dass …" trifft genau M-36 und M-95):

| Zeile | Wache | ungemessene Verdrahtung |
|---|---|---|
| M-36 `:941` | „nur dafür, dass `broker/src/main.rs` diese Funktion ruft" | Funktionsergebnis → Standardausgabe (F-1) |
| M-95 `:1015` | `main_ruft_aufruf_lesen` „nur dafür, dass … die Lesefunktion ruft und ihren Exitcode übernimmt" | gelesene Kennung → Brokerstart (F-2 a) |
| keine Zeile (§7.4 `:1871-1873`, §12.4 `:2668`) | keine | `broker_starten` → Naht → `briefkasten_starten` (F-2 b) |
| M-39 `:944` (gleiche Klasse) | Startfolge im Quelltext | dass `broker_starten` die Naht ruft (§7.2 `:1557-1558`), trägt keine Zeile; das Verhalten messen M-34, M-35, M-37 an der Naht auf Testnamen |
| M-40 `:945`, M-56 `:967` (gleiche Klasse) | keine | dass `broker_soll_idle_enden` die neue Zählfunktion ruft; der als „bestehend" genannte `echter_brokerprozess_…` fährt die eigene Leerlaufschleife von `eqcop-broker-v3probe` (`broker/src/bin/eqcop-broker-v3probe.rs:95-104`), nicht die des Brokers |

Die übrigen Wachen tragen Aufbau neben einem Laufzeittest und fallen nicht unter das Muster: M-55 `:966` (Schließweg), M-74 `:992` (Orte des Tokenvergleichs), M-93 `:1034` (Name und Token), M-99 `:1019` (die Leerlaufzählung liest keine Sitzung).

7. Technische Prüfung des Wegs „`main.rs` ohne Logik, `lauf(argumente, ausgabe, brokerstart)` → Exitcode":
   - Machbar. `main.rs` hält heute Argumentschleife (`:25-42`), Bindungsordner (`:43-49`), Start (`:50`), Meldung (`:52-59`), Warteschleife (`:60-70`), Stopp und Exit (`:71-79`); die Schleife fragt Funktionen über der Prozesszelle `BROKER` ab (`broker/src/lebenslauf.rs:120`, `:299`, `:332`). Die Fassade `brokerstart` muss deshalb mehr tragen als den Start: Start mit Bindungen und Kennung samt Status für die Meldung, fataler Listenerfehler, Leerlaufende, geordneter Stopp. Mit Attrappen für Ausgabe und Fassade läuft `lauf` im Testprozess ohne Pipe; `main.rs` wird ein einziger Ausdruck, seine Wache eine Aussage über Aufbau, die §8.4 (iii) zulässt (Muster vorhandener Wachen auf `lebenslauf.rs`: `broker/tests/broker_idle.rs:178-235`, `broker/src/briefkasten.rs:1920-1930`).
   - F-1 schließt er ganz: `lauf_startmeldung_nennt_nur_bediente_endpunkte` liest die Ausgabe-Attrappe bei Status mit und ohne v2-Griff; Rotbeweis: „`lauf` schreibt statt des Funktionsergebnisses die feste Meldung mit `PIPE_NAME` → ohne v2-Griff nennt die Ausgabe den v2-Namen → rot" (hält „Lauf aus M-35", bricht das Ergebnis der Startmeldung).
   - F-2 schließt er für Glied (a): `lauf_reicht_sitzung_an_den_brokerstart` — die Fassaden-Attrappe zeichnet Bindungen und Kennung auf: gültige Kennung → dieselbe Kennung; ungültige → keine, Start erfolgt; unbekanntes Argument → Exit 2, kein Start; Rotbeweis: „`lauf` reicht keine Kennung weiter → die Attrappe erhält bei gültigem Argument keine → rot". Glied (b) bleibt offen: die echte Fassade und `broker_starten` sind je ein Aufruf (Aufbau-Wache, die den Parameter mitliest); das Durchreichen Naht → `briefkasten_starten` braucht eine Zeile an der Naht auf Testnamen (Etappe 5): Marke und Kennung des Testprozesses → Briefkasten des Laufs `Gestartet` mit einem Sitzungsgriff, Kennung eines beendeten Hilfsprozesses → `SitzungBeendet`; Rotbeweis: „die Naht reicht keine Kennung an `briefkasten_starten` → `OhneSitzung` → rot". Voraussetzung: die Naht nimmt die Umgebungsfassade des Briefkastens als Parameter; heute baut `briefkasten_starten` die echte (`broker/src/briefkasten.rs:779-781`), im Test fehlte die Marke (M-80), und der Fall unterschiede Durchreichen und Verlieren nicht. Gleichwertig: der Probelauf von M-97 setzt sich über die Naht zusammen statt selbst.
   - Matrix: M-36 Test und Rotbeweis wie oben, die Wache „nur Aufruf" wird eine gemeinsame Aufbau-Wache (Name Bauinhalt; Bruch: eine Ausgabezeile oder ein eigenes Argumentlesen in `main.rs` → rot); M-95 Test und Rotbeweis wie oben, Zusage (e) ergänzt um „`lauf` reicht die gelesene Kennung unverändert an den Brokerstart", Wache wie M-36; eine neue Zeile oder der Zustand von M-97 für Glied (b); nachziehen §7.2 `:1565-1568`, §7.4 `:1866-1873`, `:1904-1905`, §6.8, §8.2 (A4, Skriptkopf `broker/src/main.rs:1-11`) und die Durchsicht. Die Startmeldung kommt in Etappe 3 (M-36), die Kennung in Etappe 5 (M-95), beide über dieselbe Naht.
8. Start des gebauten Binarys auf einem Probe-Pipenamen: entbehrlich und am HEAD nicht zulässig. `eqcop-broker` kennt nur die Produktionsnamen; ein Start im Test verletzte die GRENZE (Gate `:6` „Probe-Pipe nie Produktions-Pipe"; `CLAUDE.md:225-226`; §7.0 `:1194-1195`). Ein Pipenamen-Schalter im Produktbinary wäre eine neue Fläche derselben Klasse wie NAK-376 (§1 `:126-128`). `eqcop-broker-v3probe` ist kein Ersatz (eigene `main`, F-1 Punkt 2.4). Ist `main.rs` ein einziger Aufruf, bewiese ein Binarystart nur, dass diese Zeile läuft. Die Ende-zu-Ende-Bestätigung bleibt der Laufzeit-Arm (M-84), sobald Gen im Diagnose-FL einen Broker startet; bis dahin hat der Argumentweg des Brokers auch live keinen Beleg.

### 3. Zusagenlage

- R-310-7 `:2416-2420`: „Der Briefkasten des Brokers ist offen, solange der Prozess lebt, der den Broker mit Marke gestartet hat (das markierte FL); der Broker erhält die Kennung dieses Prozesses beim Start (Startargument des Startpfads in `BrokerLifecycle.cpp`, kein v2- oder v3-Vertragsfeld) und wacht über sein Ende."; `:2424-2425`: „Marke ohne gültige, lebende Prozesskennung → kein Briefkasten (fail-closed)."
- M-95 (a), (e) und M-84 wie oben; §7.4 `:1869-1873`.
- §7.0 `:1085-1087`: „nennt jeder Satz, der ein Verhalten verspricht, die Matrixzeile, die ihn trägt, oder er ist als Bauinhalt ohne Zusage, Rückfallweg oder Grenze gekennzeichnet" — das Durchreichen (b) ist als Bauinhalt gebucht, trägt aber den Teil von R-310-7, den keine Zeile misst.
- §8.4 (iii) `:2218-2224` und MP3-Auftrag Frage 2 (`:19`), wörtlich unter F-1.
- Grenzen für den Fix: Gate `:6` „Probe-Pipe nie Produktions-Pipe"; `CLAUDE.md:225-226` „Broker-Ende-zu-Ende immer über den Probe-Pipenamen; nie ein Testbroker auf der Produktions-Pipe."

### 4. Urteil: PRÄZISIERT

Kern bestätigt: der Weg der Kennung durch `main.rs` ist nur durch Stellvertreter belegt. Präzisiert: (1) der Prüferfall wirkt fail-closed — verletzt sind M-95 (a), M-84 und die positive Hälfte von R-310-7, nicht der fail-closed-Satz; (2) ungemessen ist auch die Gegenrichtung (falsche, lebende Kennung, fail-open); (3) ein zweites Glied, `broker_starten` → Naht → `briefkasten_starten`, hat weder Zeile noch Wache, und M-97 umgeht beide Glieder; (4) die Livezeile M-84 fängt es heute nicht.

**Klassenvorschlag: DEFEKT** — für Glied (a) Stellvertreter statt Zusage (§8.4 (iii), MP3-Auftrag Frage 2); für Glied (b) eine Zusage (R-310-7 „der Broker erhält die Kennung dieses Prozesses beim Start") ohne Zeile mit messendem Test.

**Kleinster Fix am Text:** Punkt 2.7 — Naht `lauf` mit je einem Test und Rotbeweis in M-36 und M-95, eine Aufbau-Wache für `main.rs`, eine Zeile (oder der Probelauf von M-97 über die Naht) für das Durchreichen Naht → Briefkasten; kein Binarystart.

---

## F-3 — Der A4-Satz „binnen der Schliessfrist von 500 ms" sagt mehr, als M-96, M-97 und M-113 messen (M-109, `:1023`)

### 1. Quelle

- M-109 (`:1023`), A4-Satz: „Seit NAK-310 Etappe 5 startet der Briefkasten des Brokers nur mit der Marke und der Sitzungskennung eines lebenden Prozesses und schliesst sich dauerhaft binnen der Schliessfrist von 500 ms, wenn dieser Prozess endet (Produktwert an der echten Uhr festgehalten, Ende zu Ende mit einem Hilfsprozess des Tests auf einer Probe-Pipe binnen 1,5 s; den Sitzungsgriff gibt jeder Lauf frei, gezaehlt im eigenen Testziel briefkasten_sitzungsgriff)." Kernsatz des Textfunds: „schliesst sich dauerhaft binnen der Schliessfrist"; Schlusssatz der Zusage: „Jede Behauptung sagt nicht mehr, als ihre Zeilen messen".
- §7.4 `:1890-1895`: „`Kern::lauf` wartet mit dem kleineren Wert aus Taktrest und Schließfrist …; nach dem Ende des Sitzungsprozesses vergeht damit höchstens eine Schließfrist, bis der Thread es bemerkt, zuzüglich eines Takts, der vor dem Ende begann (M-113). 500 ms statt der erlaubten 1 000 ms lassen einem solchen Takt Raum innerhalb der zugesagten Sekunde."
- M-96 (`:1016`), Reihenfolge: „… der Thread endet spätestens eine Schließfrist (`SITZUNG_SCHLIESSFRIST` 500 ms, M-113) nach dem Ende; ein Takt, der vor dem Ende begann, schreibt noch zu Ende."
- M-113 (`:1024`): (a) „`EchteUhr` liefert als Schließfrist genau `SITZUNG_SCHLIESSFRIST` = 500 ms …"; (b) Zustand „Testuhr mit Taktfrist `LANG` (3 600 s, `:969`) und Schließfrist 20 ms, … keine Auslösung", Zusage „… endet der Thread binnen der Testfrist von 2 s mit `SitzungBeendet` …". M-97 (`:1017`): „Spätestens 1,5 s nach dem bestätigten Ende …"; Frist: „… die Testfrist von 1,5 s ist Schließfrist plus eine Taktfrist … als Spielraum für einen laufenden Takt und die Planung".
- Die Stellen sagen, was der Prüfer behauptet; `:1892-1895` trifft den Satz mit „zuzüglich eines Takts".

### 2. Ablauf

1. Ein Takt (`broker/src/briefkasten.rs:594-661`): unter der Zustandssperre (`:608`) eine Existenzprüfung (`:611`); bei neuer Anfrage drei Ebenenprüfungen, Öffnen und Lesen bis 1 025 Bytes (`:669`, `:673`, `lies` `:237-256`); bei unbeantworteter Kennung `beantworte` (`:682-689`): Registersperre (`:684`), Umschlag, dann `schreibe` (`:728-751`): vier Ebenenprüfungen (`:733`), exklusives Anlegen (`:739`), `write_all` mit `sync_all` (`:743`, `:267`), Größe (`:746`), `MoveFileExW` mit `MOVEFILE_WRITE_THROUGH` (`:750`, `:281`).
2. Obere Grenze: keine. Keiner dieser Aufrufe hat eine Frist. `sync_all` versucht vor der Rückkehr sicherzustellen, dass alle Daten das Dateisystem erreichen („attempt to ensure that all in-memory data reaches the filesystem before returning"); `MOVEFILE_WRITE_THROUGH`: „The function does not return until the file is actually moved on the disk."; die Registersperre wartet auf jeden Halter. Der Code rechnet selbst mit Takten über 2 s: der Stoppweg wartet `JOIN_FRIST` 2 s (`:63`, `:874-879`) und überlässt den Thread danach sich selbst (`:886-891`, `join_frist_verfehlt`; Modulkopf `:20-23`: „der Takt wartet nie auf ihn"). Ohne Anfrage ist ein Takt kurz (eine Attributabfrage); mit Anfrage bestimmt der Datenträger die Dauer.
3. Geplanter Ablauf (§7.4 `:1883-1893`, Reihenfolge M-113): Sitzungsprüfung nach jedem Wecken, nach dem Stoppflag und vor jedem Takt; nach einem Takt führt `continue` (`:575`) zur Prüfung. Endet der Prozess, während der Thread wartet, bemerkt er es binnen der Schließfrist; endet er während eines Takts, erst an dessen Ende. Die Schließzeit ist das Größere aus Schließfrist und Restdauer des laufenden Takts (die Summe in §7.4 ist eine lockerere obere Schranke) — ohne Obergrenze, weil der Takt keine hat.
4. Wer misst einen laufenden Takt: M-113 (a) keinen (Konstantenwert, ohne Toleranz); (b) keinen (keine Auslösung, Taktfrist 3 600 s, Schließfrist 20 ms, Testfrist 2 s, also das Hundertfache); M-97 keinen gezielt (echte Uhr, ein Takt je Sekunde, nach K1 kurz, Toleranz 1,5 s); M-96 misst keine Zeit (Bühne mit `LANG`). Prüferfall 600 bis 1 400 ms: alle grün, der Satz „binnen 500 ms" falsch. Nachgerechnet.
5. Der A4-Satz sagt mehr: gemessen sind der Wert 500 ms (M-113 (a)), die Verdrahtung mit 20 ms (M-113 (b)), 1,5 s Ende zu Ende (M-97) und „kein Takt nach dem Ende" (M-96) — nicht „geschlossen binnen 500 ms". Er widerspricht §7.4 `:1892-1893`, M-96 („schreibt noch zu Ende") und §14.2 `:3168-3171`. Der Vorgänger an `6fefac1b:896` sagte „schliesst sich dauerhaft vor dem naechsten Takt, wenn dieser Prozess endet (Hilfsprozess des Tests auf einer Probe-Pipe, geschlossen binnen 2 s)" — das maßen M-96 und die damalige M-97; die Durchsicht (`:3311`) ersetzte ihn als „Stellvertreter der Produktfrist" durch die Konstante als Schließzeit.
6. Dieselbe Aussage ohne laufenden Takt steht auch in M-96 (Reihenfolge, oben; sie widerspricht dort dem eigenen Nachsatz), §6 Beziehungen `:872-874` („binnen der Schließfrist"), §7.4 Restlücke 1 `:1976-1977` und Risiko 2 `:1996-1998`. Die Begriffsbestimmung `:781-784` („die längste Wartezeit des Briefkastenthreads zwischen zwei Sitzungsprüfungen") ist richtig.
7. Die Rechnung §7.4 hält am Code nicht: „500 ms … lassen einem solchen Takt Raum innerhalb der zugesagten Sekunde" setzt einen Takt von höchstens 500 ms voraus, den weder Code noch Zeile sichert. Liest man R-310-7 wie §14.2 (der laufende Takt zählt in die Sekunde), trägt keine Zeile die Sekunde. §12.6 Punkt 3 (`:2795-2802`) liest anders („Ein Takt, der vor dem Ende begann, schreibt noch zu Ende; das ist eine Antwort aus der Sitzung"); V2 `:61` rechnet Threadende, `SitzungBeendet` und das Schließen des Sitzungsgriffs in die Sekunde.

### 3. Zusagenlage

- R-310-9 `:2442-2445`: „… trägt im selben Änderungssatz ihre Behauptungszeile im Kanon (`tools/beweise.ps1`, Bein und Zeile benannt), die nicht mehr sagt, als der Test misst"; M-109 `:1023`: „Jede Behauptung sagt nicht mehr, als ihre Zeilen messen"; Prüfliste E (`tools/dirigent/pruefliste.md:62-63`): „Jede Behauptung im Runner, Manifest oder Kommentar sagt nicht mehr, als der Test misst".
- §7.4 `:1891-1895` und M-96 nennen den laufenden Takt; §14.2 `:3167-3171` begründet die 500 ms mit ihm.
- R-310-7 `:2420-2421`: „schließt der Broker den Briefkasten binnen einer festen Frist (höchstens 1 s) dauerhaft"; R-310-11 `:3015-3016`: „M-97 bleibt die tolerante Ende-zu-Ende-Zeile mit Toleranz höchstens Frist plus ein Takt". Zu einem Takt, der beim Ende läuft, schweigt R-310-7.

### 4. Urteil: PRÄZISIERT

Kern bestätigt: der A4-Satz sagt mehr, als seine Zeilen messen, und widerspricht §7.4 und M-96. Präzisiert: (1) dieselbe Aussage steht auch in M-96, §6 `:872-874` und §7.4 `:1976-1977`, `:1996-1998`; (2) Abhilfe: „Ende zu Ende abdecken" geht nicht, denn der Code begrenzt einen Takt nicht — der Satz muss schrumpfen; (3) die Rechnung §7.4 selbst setzt eine ungemessene Taktdauer von höchstens 500 ms voraus.

**Klassenvorschlag: DEFEKT** (R-310-9, Prüfliste E: Behauptung größer als Messung). Dazu **LÜCKE** zur Lesart von R-310-7 bei laufendem Takt (R-310-7 schweigt; §12.6 Punkt 3 und §14.2 lesen verschieden) — Dirigentenwort.

**Kleinster Fix am Text:**
- M-109, A4-Satz: den Hauptsatz auf das Gemessene zurücknehmen, etwa „… und schliesst sich dauerhaft, wenn dieser Prozess endet: danach beginnt kein Takt mehr, zwischen zwei Sitzungspruefungen wartet der Thread hoechstens die Schliessfrist von 500 ms (Produktwert an der echten Uhr festgehalten), ein Takt, der vor dem Ende begann, schreibt zu Ende (Ende zu Ende mit einem Hilfsprozess des Tests auf einer Probe-Pipe binnen 1,5 s; …)"; Kernsatz des Textfunds entsprechend (etwa „wartet der Thread hoechstens die Schliessfrist").
- M-96 Reihenfolge, §6 `:872-874`, §7.4 `:1976-1977`, `:1996-1998`: „binnen der Schließfrist" → „binnen der Schließfrist; ein laufender Takt kommt hinzu".
- §7.4 `:1893-1895`: den Satz „500 ms statt der erlaubten 1 000 ms lassen einem solchen Takt Raum innerhalb der zugesagten Sekunde" ersetzen: eine Obergrenze der Taktdauer gibt der Code nicht (`sync_all`, `MOVEFILE_WRITE_THROUGH`, Registersperre ohne Frist; der Stoppweg rechnet mit Takten über 2 s); die Lesart von R-310-7 für einen laufenden Takt setzt der Dirigent.
- Wahlweise, falls der Zusatz „zuzüglich eines Takts" gemessen werden soll: Teilfall an M-113 mit dem vorhandenen Testhaken `haken_takt` (`broker/src/briefkasten.rs:548-549`, `:595-600`), der einen Takt über das Sitzungsende hält; nach der Freigabe endet der Thread ohne weiteres Warten, K2 bleibt ohne Antwort (gezählt in Takten der Instanz). Das misst den Zusatz, nicht seine Länge.

---

## Übersicht

| Befund | Urteil | Klassenvorschlag | tragender Satz |
|---|---|---|---|
| F-1 · M-36 (`:941`) | BESTÄTIGT | DEFEKT | §8.4 (iii) `:2218-2224` „Misst die Zeile die Zusage unter ihrem Ereignis …? … eine Textwache trägt nur Aussagen über Text oder Aufbau"; MP3-Auftrag Frage 2 (`:19`) |
| F-2 · M-95 (`:1015`) | PRÄZISIERT | DEFEKT | R-310-7 `:2417-2420` „der Broker erhält die Kennung dieses Prozesses beim Start … und wacht über sein Ende"; §8.4 (iii); MP3-Auftrag Frage 2 |
| F-3 · M-109 (`:1023`) | PRÄZISIERT | DEFEKT; dazu LÜCKE zur Lesart von R-310-7 | R-310-9 `:2442-2445` „die nicht mehr sagt, als der Test misst"; §7.4 `:1892-1893` „zuzüglich eines Takts, der vor dem Ende begann" |

## Gemeinsame Ursache

Ja, und es ist die Ursache der Runden 1 und 2: F-1, F-2 (a) und F-3 messen einen Stellvertreter statt der Zusage unter ihrem Ereignis (V2 `:178`) — die reine Funktion für die Ausgabe von `main.rs`, Lesefunktion und Bühnentests für den Weg der Kennung im Binary, die Konstante für die Schließzeit; F-2 (b) ist zusätzlich der Fall aus Runde 1, ein versprechender Satz ohne tragende Zeile (V1 `:191`), hier als Bauinhalt gebucht (`:2668`). Neu ist die Form. Die Durchsicht beantwortete Frage (iii) je Zeile, indem sie den Stellvertreter tauschte: aus „Textwache für ein Verhalten" wurde „reine Funktion plus Wache nur für den Aufruf" (`:3269`, `:3287`) — die Wache hält §8.4 (iii) jetzt dem Wortlaut nach, aber zwischen gemessener Funktion und Wirkung misst niemand; und aus dem gemessenen „vor dem naechsten Takt … binnen 2 s" wurde eine Konstante als Schließzeit (`:3311`), also eine Behauptung größer als ihre Messung. Technische Wurzel für F-1 und F-2: die Produktkomposition (`main.rs`, `broker_starten` mit Produktionsnamen und der Prozesszelle `BROKER`) läuft in keinem Test (`:3289`; `broker/tests/broker_idle.rs:182-186`), die Matrix ersetzt sie durch Quelltextwachen, und jede Zusage, die durch diese Schichten läuft, erbt die Lücke (auch M-39, M-40, M-56, Tabelle in F-2). Abhilfe als Klasse: die Produktschicht auf einen bewachten Aufruf verkürzen und alles mit Wirkung hinter Fassaden legen, die der Test ersetzt — wie der Probelauf schon die Wurzel ersetzt (`broker/src/briefkasten.rs:921`). Für die vier Fragen genügen zwei Zusätze zu (iii): „Erreicht das gemessene Ergebnis die Wirkung im Produkt, und misst eine Zeile den Weg dazwischen?" und „Nennt die Behauptung die gemessene Größe oder nur einen ihrer Parameter?"

## Nicht geprüft

- Kein Lauf; die Dauer realer Takte ist nicht gemessen, nur ihre fehlende Obergrenze am Code gelesen.
- Der Weg `lauf` ist auf Machbarkeit am heutigen Code geprüft, nicht entworfen; Namen und Fassadenzuschnitt sind Bauinhalt.
- §5.1, §5.2, §5.5, §6.1 bis §6.5 außer M-34 bis M-42, M-55, M-56, M-74 und die Nacharbeit-1-Abschnitte außer §12.1, §12.4 (Ausschnitt) und §12.6 habe ich nicht gelesen.

## Quellen im Netz (abgerufen 23.09.2026)

- Cargo Book, Cargo Targets (Binaries, Integration tests): https://doc.rust-lang.org/cargo/reference/cargo-targets.html
- Rust std, `File::sync_all`: https://doc.rust-lang.org/std/fs/struct.File.html
- Microsoft Learn, `MoveFileExW` (`MOVEFILE_WRITE_THROUGH`): https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw

FERTIG Validierung Matrixprüfung 3 NAK-310, 1 bestätigt, 2 präzisiert, 0 widerlegt.
