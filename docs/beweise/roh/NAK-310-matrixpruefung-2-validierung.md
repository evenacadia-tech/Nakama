# NAK-310 — Validierung der Matrixprüfung 2 (lesender Opus-5.5-Thread, Effort max)

**HEAD vorher:** `abbc8ddaa1403168a15c9214f2caeb4bbd2a6c8f` (= `origin/master`, erste Messung des Laufs) · **HEAD nachher:** `abbc8ddaa1403168a15c9214f2caeb4bbd2a6c8f` (23.09.2026, 14:06:13 vor dem Schreiben dieser Datei und erneut danach, `git rev-parse HEAD`).
Manifest gelesen an `abbc8dda` (`git show abbc8dda:docs/beweise/NAK-310.md`; `git diff --stat abbc8dda -- docs/beweise/NAK-310.md` war leer); Code am Arbeitsbaum gleich `abbc8dda`. Nur Lesebefehle und vier Primärquellen im Netz (am Ende); kein Bau, kein Test, kein Python, kein FL, keine Pipe; geschrieben nur diese Datei.
Zeilen ohne Datei meinen `docs/beweise/NAK-310.md` an `abbc8dda`. Kürzel: MP1-/MP2-Auftrag = `docs/beweise/roh/NAK-310-matrixpruefung-1-auftrag.txt` / `…-2-auftrag.txt`, NA1-Auftrag = `docs/beweise/roh/NAK-310-matrixnacharbeit-1-auftrag.txt`, E1-Auftrag = `docs/beweise/roh/NAK-310-etappe-1-auftrag.txt`. Alle Zeilennummern des Prüfers stimmen.

---

## B-1 — M-111 prüft Riegel 3 über die lebende Installationsaufgabe (P1)

### 1. Quelle

- M-111 (`:792`): Zustand „nach dem Handgriff U60 am PC; das eigene Diagnose-FL läuft …"; Ereignis „einmaliger Start von `\Nakama\installieren` über die Aufgabenplanung, ohne das FL vorher zu beenden"; Zusage „Der Lauf als SYSTEM sieht das FL des Benutzers in dessen Sitzung und verweigert wie heute …, Ziele bytegleich …, Ergebnisjournal unverändert"; Test „Einmalmessung (live, kein Kanonbein) … statisch A18 `[0]` (M-20)"; Rotbeweis „**heute nicht messbar** … den Rotbeweis der Ruhe-Zeile trägt M-20 (A18 `[0]`)".
- Grenzsatz `:1055-1059`: „keine lebende Aufgabenplanung und kein `Program Files` in Tests (A18-Sandbox, A35-Attrappen; einzige Ausnahme ist die Einmalmessung M-111, die die lebende Aufgabe nach U60 einmal ohne Mutation startet)"; gleichsinnig die §6-Regel `:719-723` („… Einmalmessung M-111, die ohne Mutation endet").
- Beide Ausnahmen kamen mit diesem Diff. An `225784b7` stand `:617-618` „Keine Zeile berührt die lebende Aufgabenplanung, `Program Files` oder die Produktions-Pipe" und `:884-885` die Grenze ohne Ausnahme; der Bauer führt das als Abweichung §12.6 Punkt 2 (`:2485-2489`) und nennt M-111 selbst als tragende Stelle für die Prüfung (§12.8 Punkt 3, `:2579-2582`).
- M-20 (`:784`) bricht nur den Text („Ruhe-Zeile im Original umformulieren → `[0]` rot"). A18 ersetzt die `Get-Process`-Zeile von `Fl-Laeuft` in der Sandbox durch `$false` (`tools/eq-copilot/pruefe_installer_gegenpfad.py:36-41`, `:61-62`) und zählt in `[0]` nur, dass die Originalzeile genau einmal steht (`:115-125`).
- Die Stellen sagen, was der Prüfer behauptet.

### 2. Ablauf

1. Riegel 3 ist `eq-copilot/install/Install-Nakama.ps1:966`, `Fl-Laeuft` ist `:542-544` (`Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue`). Davor laufen Riegel 1 (`:886-891`), Riegel 2 (`:893-919`), die Signaturzeile (`:921-945`) und der `-Pruefen`-Ausgang (`:949-963`). M-111 erreicht Riegel 3 also nur mit hashgleichen Quellen — genau in dem Stand, in dem ein Fehlgriff eine vollständige Installation startet.
2. Sieht SYSTEM das FL nicht, folgt ohne Gleichstandsabkürzung: Sperrdatei unter Program Files (`:972`, `:609-623`), Sicherung aller Ziele (`:1051-1108`), Journal `VORBEREITET` in `aktive-transaktion.json` und `install-ergebnis.json` (`:1110-1119`, `Schreibe-Ergebnis` `:182-192`), dann je Artefakt `Remove-Item -Recurse -Force` auf das installierte Bundle und Kopie (`:501`, `:511`, `:1147`) — während das Diagnose-FL Gen und Probeeq geladen hält. Scheitert der Tausch, endet der Lauf in `ERROR_RUECKGEROLLT` oder `ERROR_TEILSTAND` (`:1228`); ein Teilstand sperrt jede weitere Installation bis `-Rueckweg` (`:990-991`), und der Rückweg prüft dieselbe Funktion (`:655`). Das ist der Schaden, gegen den Riegel 3 steht (`:29-31`: „Ein Host, der ein Bundle geladen haelt, bekommt sonst eine Datei unter den Fingern weggetauscht.").
3. Der reguläre Arm belegt den Riegel nie: er beendet das Diagnose-FL vor der Aufgabe (`tools/fl/laufzeit.ps1:599`, Aufgabe `:605-606`; so auch §7.1 Risiko 1, `:1315-1320`).
4. Form: Einmalmessung live, weder A18-Test noch Kanonbein. Für den Fehlerfall nennt die Zeile weder Abbruchkriterium noch Rückweg; SHA-256 vorher/nachher stellt ihn nur nachträglich fest. Auch im Gutfall schreibt der Lauf unter Program Files: das Log des Anker-Orts wird „je Lauf überschrieben" (`:1161-1162`) und soll danach die ABBRUCH-Zeile tragen.
5. Nebenwiderspruch: Zustand „vom Laufzeit-Arm gestartet" gegen Reihenfolge „Vor dem ersten Laufzeit-Arm danach" (§7.1 Schritt 4, `:1256-1258`); zwischen Commit A und U60 verweigert der Arm (M-15).
6. Wahrscheinlichkeit: `Get-Process` „gets all processes on the local computer", `Process.GetProcessesByName` liefert „all the process resources on the local computer" (Microsoft Learn). SYSTEM sieht das Benutzer-FL also sehr wahrscheinlich. Die Matrix führt die Frage aber als offen (Risiko 1), und die Mutationsfreiheit einer Messung darf nicht von ihrem Ergebnis abhängen.

### 3. Zusagenlage

- Grenzsatz `:1057-1059`: die Ausnahme gilt nur für einen Start „ohne Mutation". M-111 sichert das nicht zu (Punkt 2) und fällt damit nicht unter die Ausnahme.
- Dirigentenkriterium aus den Runden davor: MP1-Auftrag `:17` „ODER ein Rotbeweis oder Test verlangt die Produktions-Pipe, die lebende Aufgabenplanung, Program Files außerhalb der A18-Sandbox oder einen Angriffsweg"; MP2-Auftrag `:16` „verlangt keine neue Zeile … lebende Aufgabenplanung"; E1-Auftrag `:26` Rotbeweis „nie in der lebenden Aufgabenplanung". Der NA1-Auftrag lockert das nicht; die Lockerung in `:716-723` und `:1055-1059` stammt vom Bauer.
- Matrixregeln `:716-717` („Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt") und `:702-704` („**heute nicht messbar**: … die Zeile nennt die Mutation nach dem Bau"). M-111 nennt keine Mutation; M-20 misst die SYSTEM-Sicht nie.
- Zu beweisende Zusage: `CLAUDE.md:223` („Der Installer verweigert bei laufendem FL"). Gate BEWEIS verlangt die „Installation über die erhöhte Aufgabe nach der Reparatur" — das leistet M-16 mit beendetem FL, nicht ein Lauf bei offenem FL.
- Quellvalidierung Teil A §1 h (`:450-472`) nennt als Rotbeweisorte nur A17, A18 und die A35-Vorprüfung und schweigt zur SYSTEM-Sicht; der SYSTEM-Principal kam erst mit M-09 in Etappe 1.

### 4. Urteil: BESTÄTIGT

Zusatz: Die Ausnahme steht wörtlich im Grenzsatz, wurde aber in diesem Diff eingefügt und an eine Bedingung gebunden, die M-111 nicht garantieren kann. Der Fehlerfall ist nach Microsoft unwahrscheinlich; die Gefahr bleibt eine Eigenschaft des Verfahrens.

**Klassenvorschlag: DEFEKT** — Test verlangt die lebende Aufgabenplanung entgegen dem Dirigentenkriterium; Mutationsfreiheit nur bei Erfolg der Messung; die Zusage „SYSTEM sieht das FL" hat keinen Rotbeweis an der Zusage.

**Kleinster Fix am Text:** M-111 (`:792`) durch einen Beleg ohne Mutationsweg ersetzen — Mutationsfreiheit durch Konstruktion, nicht durch das Messergebnis. Entweder die SYSTEM-Sicht in §7.1 Risiko 1 (`:1315-1320`) als belegte Annahme mit Microsoft-Quelle führen, ehrlich ohne Rotbeweis, und A18 `[0]` als Textwache belassen; oder, nur auf ausdrückliches Dirigentenwort, ein Livelauf als SYSTEM in einem Modus ohne Sperre, Sicherung, Journal und Tausch (Bauinhalt Etappe 2). In beiden Fällen die Ausnahmesätze `:720-723` und `:1057-1059` auf den Wortlaut von `225784b7` zurücknehmen und §7.1 Schritt 4 (`:1256-1258`) nachziehen.

---

## B-2 — Die feste Schließfrist von 1 s hat keinen Beleg (P2)

### 1. Quelle

- M-97 (`:890`): Zusage „Spätestens 2 s nach dem bestätigten Ende meldet der Probelauf den Briefkasten geschlossen …"; Frist „Produktfrist eine Taktfrist (M-96); die Testfrist von 2 s lässt 1 s Planungsspielraum"; Rotbeweis „die Sitzungsprüfung vor dem Takt entfernen → K2 wird beantwortet → rot".
- M-96 (`:889`): Zustand „Testbühne mit Thread (`buehne(true, LANG)` …)", Ereignis „… danach Anfrage K2 und 5 Auslösungen". Die Frist steht nur in der Spalte Reihenfolge („der Thread endet spätestens eine Taktfrist später (`TAKTFRIST` 1 s, `:61`)"); Rotbeweis wie M-97.
- R-310-7 `:2115-2116`: „Endet der Prozess, schließt der Broker den Briefkasten binnen einer festen Frist (höchstens 1 s) dauerhaft für diesen Brokerlauf".
- Die Stellen sagen, was der Prüfer behauptet.

### 2. Ablauf

1. `TAKTFRIST` = 1 s steht in `broker/src/briefkasten.rs:61` und wird nur über `EchteUhr::taktfrist` gelesen (`:412-422`); `Kern::lauf` wartet per `wait_timeout` bis zum Fristende (`:554-591`). Kein Test liest die Konstante (`git grep TAKTFRIST`: nur `:61` und `:420`).
2. M-96: `buehne(true, LANG)` baut eine Testuhr mit `LANG` = 3600 s — „Takte kommen nur ueber Ausloesungen" (`:968-969`, `:1201-1219`, `ausloesen` `:1248-1253`). M-96 bleibt bei jeder `TAKTFRIST` grün.
3. M-97: der Probelauf nimmt `Fassaden::echt` (`:921`, echte Uhr `:436`). Bei `TAKTFRIST` 1,5 s endet der Thread etwa 1,5 s nach dem Ende, also innerhalb der 2 s. Rot wird M-97 erst ab rund 2 s.
4. Keine andere Zeile hält den Wert. M-102 (`:895`) prüft nur „keine Antwort" nach 3 s. §6 Zahlenränder (`:759`) behauptet „Schließfrist eine Taktfrist M-96, M-97"; die §6-Zeitregel (`:733-735`) verlangt Randwerte einer Frist „an reinen Funktionen mit Testuhr" — für die 1 s gibt es keinen.
5. Wirkung einer Regression: keine späte Antwort, denn die Prüfung steht vor jedem Takt (Rotbeweis M-96, M-97). Später kommen aber Threadende, `SitzungBeendet` und das Schließen des Sitzungsgriffs — genau das, was R-310-7 auf 1 s begrenzt.

### 3. Zusagenlage

- R-310-7 `:2115-2116` (oben) und NA1-Auftrag Punkt 1 (`:13`): „die Prozesswache — endet der markierende Prozess, schließt der Broker den Briefkasten binnen höchstens 1 s dauerhaft für diesen Lauf … Je Zeile: Zustand, Ereignis, Zusage, Test (Datei, Name, Bein), Rotbeweis an der Zusage."
- Muster im selben Manifest: M-47 (`:831`) hält die v2-Produktfristen in `v2_fristkonstanten` fest („eine Produktkonstante ändern → `v2_fristkonstanten` rot").
- §12.6 Punkt 3 (`:2490-2497`) legt die Lesart offen („Der Ende-zu-Ende-Test misst mit 2 s Toleranz (M-97), die Einheit ohne Uhr (M-96)"); §12.8 Punkt 3 (`:2579-2582`) reicht sie an die Prüfung weiter.

### 4. Urteil: BESTÄTIGT

**Klassenvorschlag: DEFEKT** — eine Zusage aus R-310-7, wörtlich, ohne messenden Test und ohne Rotbeweis. Die Produktwirkung ist klein, weil keine Antwort durchrutscht.

**Kleinster Fix am Text:** M-96 (`:889`) um einen Teilfall ergänzen, der die Produktfrist festhält (die echte Uhr liefert genau 1000 ms; Muster `v2_fristkonstanten`), mit Rotbeweis „`TAKTFRIST` auf 1500 ms → rot"; `:759` darauf verweisen. M-97 bleibt die tolerante Ende-zu-Ende-Zeile.

---

## B-3 — M-108 trägt eine Wache für R-310-8 (P2)

### 1. Quelle

- M-108 (`:871`): Zusage endet „… nicht mehr, als diese Zeilen messen. Kein Satz sagt, Etappe 4 begrenze, wer die Hörmarkierung über v2 setzt (R-310-8, Register NAK-377)"; Test zusätzlich „`git grep -n -i "hoermarkierung" -- tools/beweise.ps1` trifft die A4-Zeile nicht"; Rotbeweis „**zwei Hälften.** … Hörmarkierung: **Regressionswache** (heute trifft der zweite Befehl nur B4, `:709`); einmal gebrochen: ein Satz über die Hörmarkierung in der A4-Zeile → Treffer dort → rot"; Quelle „R-310-9; R-310-8; …".
- R-310-8 `:2132-2136`: „… Keine neue Matrixzeile." NA1-Auftrag Punkt 2 (`:14`): „Keine neue Matrixzeile, keine Zusage über eine Setzerbegrenzung."
- Die Fakten des Prüfers stimmen: eine eigene Hälfte mit Suche, Mutation und Quelle R-310-8.

### 2. Ablauf

1. Die Hälfte verspricht keine Setzerbegrenzung, sie verneint sie. Sie soll die A4-Behauptung klein halten (§12.6 Punkt 5, `:2502-2505`).
2. M-108 ist keine Zeile „wegen R-310-8", sondern die Behauptungszeile nach R-310-9 (§12.5 `:2455`); §6.8 (`:919-922`) zählt „R-310-8: keine Zeile". Den Wortlaut bricht sie nicht; sie holt aber Test und Rotbeweis mit Quelle R-310-8 in die Matrix.
3. Die Wache misst nicht, was sie vorgibt: `git grep -n -i "hoermarkierung" -- tools/beweise.ps1` trifft an `d72f9d84`, `225784b7` und `abbc8dda` nichts (am HEAD Exit 1). Der Kanon schreibt „Hoer-Markierung", in A3 (`:509`) und B4 (`:709`). Die Angabe „heute trifft der zweite Befehl nur B4, `:709`" ist am Ziel falsch, und ein Satz in dieser Hausschreibweise in der A4-Zeile bliebe unentdeckt.

### 3. Zusagenlage

- R-310-8 verbietet eine neue Zeile, nicht einen Satz in einer R-310-9-Zeile. „Nur in §7.0, §7.3 und NAK-377 verneint" ist die Folgerung des Prüfers, kein Wortlaut. Die Verneinung steht schon in §7.3 (`:1544-1545`: „und nichts darüber, wer die Hörmarkierung setzt (R-310-8)").
- MP2-Auftrag Frage 1 (`:16`): „stimmen die Codezeilen, die neue Zeilen als Bauinhalt oder als vorhandenes Verhalten nennen, am ZIEL" — die Textfundangabe der zweiten Hälfte stimmt nicht.

### 4. Urteil: PRÄZISIERT

Der Wortlaut von R-310-8 ist nicht verletzt. Die zweite Hälfte ist trotzdem fehlerhaft, aus einem anderen, prüfbaren Grund: falscher Textfund am Ziel, blinde Wache.

**Klassenvorschlag: DEFEKT (Text)**, geringes Gewicht; ohne den falschen Textfund wäre es Härtung.

**Kleinster Fix am Text** (derselbe wie beim Prüfer): in M-108 (`:871`) den Satz „Kein Satz sagt, Etappe 4 begrenze …", den zweiten `git grep`, die Hälfte „Hörmarkierung: Regressionswache …" und „R-310-8" in der Quelle streichen. Die Verneinung bleibt in §7.0 (`:978-986`), §7.3 (`:1544-1545`, `:1554-1568`) und im Register NAK-377.

---

## B-4 — Der Rotbeweis von M-98 kippt die Vorbedingung (P2)

### 1. Quelle

- M-98 (`:891`): Zustand „Probelauf wie M-97 vor dem Beenden des Hilfsprozesses; ein zweiter v2-Client mit anderer Sensor-ID"; Ereignis „Anfrage K"; Zusage „Genau eine Antwort der Rolle `broker` wie bisher; `aggregat` nennt beide Sensoren; …"; Test „A4 **NEU** Teilfall von `briefkasten_schliesst_mit_dem_sitzungsprozess`" (nach M-97 in `broker/tests/briefkasten.rs`); Rotbeweis „**Regressionswache** (antwortet heute schon); einmal gebrochen: die Sitzungsfassade meldet einen lebenden Prozess als beendet → keine Antwort → rot".
- R-310-7 `:2120-2126`: „Gegenfälle mit Rotbeweis, je in einer Matrixzeile: … (b) markierender Prozess lebt, zweites FL verbindet → Antwort wie bisher". Das „an der Zusage" steht nicht in R-310-7 selbst, sondern in §11.4 (`:2159-2160`: „je Matrixzeile fällt der Rotbeweis an der Zeile, die die Zusage trägt, nicht an einem Nebeneffekt"), im NA1-Auftrag Punkte 1 und 5 (`:13`, `:17`) und in der Matrixregel `:716-717`.

### 2. Ablauf

1. Die Mutation kippt den Zustand „Sitzungsprozess lebt", nicht die Wirkung des Ereignisses „zweite Verbindung". Sie macht den Fall auch ohne zweiten Client rot — dieselbe Richtung wie M-95 (c), M-96 und M-97.
2. Der Fall ist ein Integrationstest mit öffentlichem Probelauf und echter Fassade (`broker/tests/briefkasten.rs:65-125`, `broker/src/briefkasten.rs:921`); die Mutation sitzt deshalb in der echten Sitzungsfassade. Diese prüft schon beim Start (M-95; §7.4 `:1647-1655`): der Briefkasten startet nicht, und der Fall fällt an seiner Vorbedingung (Muster `broker/tests/briefkasten.rs:77-78`), bevor K gestellt wird.
3. Ungeprüft bleibt, ob eine zweite Verbindung bei lebender Sitzung die Antwort ändert. In M-98 tragen beide Clients die PID des Testprozesses, Sitzungsprozess ist der Hilfsprozess. Ein Bau, der die Antwort auf Sensoren der Sitzungs-PID beschränkt oder den Briefkasten bei einer weiteren v2-Verbindung schließt, bräche die Zusage; keine benannte Mutation zeigt, dass der Fall das fängt.

### 3. Zusagenlage

- R-310-7 (b) und die Rotbeweisregel aus §11.4, NA1-Auftrag und `:716-717` (oben).
- Eine Regressionswache wird nach `:704-706` „einmal absichtlich gebrochen"; M-99 (`:892`) zeigt die tragfähige Form mit dem Bruch am Ereignis („beim Sitzungsende den ganzen Probelauf stoppen → Heartbeat ohne `heartbeat_ack` → rot").

### 4. Urteil: BESTÄTIGT

Nur die Quellenangabe ist genauer zu fassen: „an der Zusage" tragen §11.4, der NA1-Auftrag und `:716-717`; R-310-7 verlangt „Gegenfälle mit Rotbeweis".

**Klassenvorschlag: DEFEKT** — der Rotbeweis fällt an einer Vorbedingung statt an der Zusage.

**Kleinster Fix am Text:** den Rotbeweis von M-98 (`:891`) bei gehaltener Vorbedingung neu fassen, etwa „die Antwort nur für Sensoren, deren `host_pid` der Sitzungs-PID gleicht → `aggregat` nennt keinen der beiden → rot" oder „den Briefkasten bei einer zweiten v2-Verbindung schließen → keine Antwort auf K → rot".

---

## B-5 — Die Griffzählung von M-100 läuft im parallelen A4 (P2)

### 1. Quelle

- M-100 (`:893`): Zustand „Testbühnen mit zählender Sitzungsfassade: (a) …; (b) …; (c) …; dazu 100 Zyklen Start und Stopp mit der echten Fassade …"; Zusage „… nach 100 Zyklen mit der echten Fassade ist die Griffzahl des Testprozesses (`GetProcessHandleCount`) wieder auf dem Ausgangswert"; Test „A4 **NEU** `sitzungsgriff_genau_einmal_geschlossen`"; Rotbeweis „das Schließen im Stopp streichen → (b) Öffnungen ungleich Schließungen → rot; den Griff beim Sitzungsende nicht schließen → (a) rot".
- `tools/beweise.ps1:515`: A4 hat `Argumente = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never')`, ohne `--test-threads=1`; nur A4-SI setzt es (`:524`, nur `store_crash_matrix`).
- Die Stellen sagen, was der Prüfer behauptet.

### 2. Ablauf

1. Cargo führt alle `#[test]` eines Testbinaries „in multiple threads" aus; mehrere Testziele laufen „serially" (Cargo Book, `cargo test`).
2. Die Testbühnen sind `#[cfg(test)]`-Innereien von `broker/src/briefkasten.rs` (`buehne_mit` `:1201-1212`); der Test liegt damit im Lib-Testbinary. Dieses hat am HEAD 284 Unit-Tests (`git grep -c "#\[test\]"` über `broker/src`, keiner unter `bin/`), darunter `server.rs` (24; benannte Pipes `:503`, `:559`, Threads), `briefkasten.rs` (18; Thread `nakama-briefkasten`) sowie Transport- und Storetests.
3. `GetProcessHandleCount` liefert „the number of open handles that belong to the specified process" (Microsoft Learn), also prozessweit. Parallele Tests verschieben den Wert in beide Richtungen: Rot bei korrekter Griffführung, Grün, wenn gleichzeitig freigegebene Griffe ein Leck verdecken.
4. Umfang: (a) bis (c) zählen bereits über die zählende Testfassade; betroffen ist nur die Hälfte mit echter Fassade. Für sie nennt die Zeile keinen Rotbeweis, denn beide genannten Mutationen fallen in der Testfassade.
5. Abhilfe: Ein Zähler in der echten Fassade zählt Aufrufe, nicht Betriebssystemgriffe. Die Aussage „Griffzahl zurück" braucht eine isolierte Probe: ein eigenes Testziel mit genau diesem Test (Cargo fährt Testziele nacheinander). Präzedenzen: 312/M-31 zählt Griffe in einem einfädigen C++-Binary und verlangt „nicht höher als" (`eq-copilot/plugin/tests/SondeLebenslaufTestMain.cpp:729-775`); `broker/tests/briefkasten.rs:14-15` serialisiert einen prozessweiten Zähler per Mutex, was nur gegen Tests hilft, die denselben Mutex nehmen.
6. Dasselbe Muster steht außerhalb des Prüfgegenstands dieser Runde in M-65 (`:856`, Etappe 4): „während der Verbindung ein zusätzlicher offener Griff, nach 100 Zyklen zurück auf den Ausgangswert".

### 3. Zusagenlage

- Prüfliste E (`tools/dirigent/pruefliste.md:62-64`): „Jede Behauptung … sagt nicht mehr, als der Test misst (… „isoliert gemessen" braucht die isolierte Probe)".
- Matrixregel `:716-717`; `CLAUDE.md:77-79` öffnen↔schließen (M-100 ist das Gegenstück, §6 Beziehungen `:739`). R-310-7 schweigt zu Griffen.

### 4. Urteil: PRÄZISIERT

Kern bestätigt: paralleles A4, prozessweite Zählung, die Zusage ist so nicht verlässlich messbar. Präzisiert sind der Umfang (nur die Hälfte mit echter Fassade, dort ohne Rotbeweis) und die Abhilfe (isoliertes Testziel; ein Fassadenzähler allein misst den Betriebssystemgriff nicht).

**Klassenvorschlag: DEFEKT** — Zusage ohne verlässlich messenden Test im erklärten Bein; ohne Isolierung entsteht ein schwankendes A4.

**Kleinster Fix am Text:** in M-100 (`:893`) die 100 Zyklen einem eigenen Testziel unter `broker/tests/` mit genau diesem Test zuweisen, Messung „nicht höher als der Ausgangswert" nach 312/M-31, dazu ein Rotbeweis an der echten Fassade („das Schließen in der echten Sitzungsfassade streichen → Griffzahl nach 100 Zyklen höher → rot"). M-65 in seiner Etappe ebenso.

---

## Übersicht

| Befund | Urteil | Klassenvorschlag | tragender Satz |
|---|---|---|---|
| B-1 · M-111 (`:792`) | BESTÄTIGT | DEFEKT | MP1-Auftrag `:17`: „ODER ein Rotbeweis oder Test verlangt … die lebende Aufgabenplanung …"; Grenzsatz `:1057-1059`: Ausnahme nur für einen Start „ohne Mutation" |
| B-2 · M-96, M-97 (`:889-890`) | BESTÄTIGT | DEFEKT | R-310-7 `:2115-2116`: „binnen einer festen Frist (höchstens 1 s)"; NA1-Auftrag Punkt 1: „Rotbeweis an der Zusage" |
| B-3 · M-108 (`:871`) | PRÄZISIERT | DEFEKT (Text), gering | R-310-8 `:2135-2136` „Keine neue Matrixzeile" wörtlich nicht verletzt; MP2-Auftrag Frage 1 (Angaben am ZIEL): Textfund „nur B4, `:709`" falsch |
| B-4 · M-98 (`:891`) | BESTÄTIGT | DEFEKT | §11.4 `:2159-2160`: „je Matrixzeile fällt der Rotbeweis an der Zeile, die die Zusage trägt, nicht an einem Nebeneffekt" |
| B-5 · M-100 (`:893`) | PRÄZISIERT | DEFEKT | Prüfliste E `:62-64`: eine isolierte Aussage braucht „die isolierte Probe"; Matrixregel `:716-717` |

## Gemeinsame Ursache

Ja. Alle fünf Befunde sitzen in Zeilen dieser Nacharbeit (M-96 bis M-100, M-108, M-111), und jede misst einen Stellvertreter statt der Zusage unter ihrem Ereignis. B-1 belegt die SYSTEM-Sicht mit einer Textwache aus einer Sandbox, in der der Mechanismus durch `$false` ersetzt ist, und sichert den Livelauf nur durch sein erhofftes Ergebnis. B-2 misst die Frist mit einer Testuhr von 3600 s und einer Ende-zu-Ende-Toleranz von 2 s, nie an der Produktkonstante. B-4 kippt die Vorbedingung statt das Ereignis. B-5 zählt prozessweit in einem parallel laufenden Binary. B-3 sucht eine Schreibweise, die der Kanon nicht benutzt. Das Selbstaudit (§12.7 `:2563-2564`: „jeder Rotbeweis fällt an der Zeile, die die Zusage trägt") prüfte den Ort der Mutation, nicht, ob sie bei gehaltener Vorbedingung das Ergebnis des Ereignisses ändert und ob die Messgröße im erklärten Bein isoliert ist. Bei B-1 kommt eine selbst gelockerte Grenze hinzu (`225784b7:617-618`, `:884-885` → `:716-723`, `:1055-1059`). Für die nächste Runde genügt je neuer Zeile: Ereignis, gehaltene Vorbedingung und eine Mutation, die unter dem Ereignis das Ergebnis ändert; für jede Frist einer Regel ein Test an der Produktkonstante; Betriebssystemzählungen nur im isolierten Testziel; Grenzen nur auf Dirigentenwort.

## Quellen im Netz (abgerufen 23.09.2026)

- Cargo Book, `cargo test`: https://doc.rust-lang.org/cargo/commands/cargo-test.html
- Microsoft Learn, `Get-Process` (PowerShell 7.5): https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.management/get-process?view=powershell-7.5
- Microsoft Learn, `Process.GetProcessesByName`: https://learn.microsoft.com/en-us/dotnet/api/system.diagnostics.process.getprocessesbyname
- Microsoft Learn, `GetProcessHandleCount`: https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesshandlecount

FERTIG Validierung Matrixprüfung 2 NAK-310, 3 bestätigt, 2 präzisiert, 0 widerlegt.
