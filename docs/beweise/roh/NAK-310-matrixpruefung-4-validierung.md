# NAK-310 — Validierung der Matrixprüfung 4 (lesender Opus-5.5-Thread, Effort max)

**HEAD vorher:** `709ee97a0c1759dbacf0f66db9fea79cb0520a81` (= `origin/master`, erste Messung des Laufs; erneut 23.09.2026, 18:07:41 vor dem Schreiben dieser Datei) · **HEAD nachher:** `709ee97a0c1759dbacf0f66db9fea79cb0520a81` (je `git rev-parse HEAD`, nach dem Schreiben dieser Datei).

Manifest ausschließlich über `git show 709ee97a:docs/beweise/NAK-310.md`; Zeilen ohne Datei meinen dieses Manifest an `709ee97a`. Code am Arbeitsbaum; `git status --short` zeigt keine geänderte getrackte Datei, der Code gleicht HEAD. Nur Lesebefehle und zwei Primärquellen im Netz (am Ende); kein Bau, kein cargo, kein Test, kein Python, kein FL, keine Pipe; geschrieben nur diese Datei. MP4-Auftrag = `docs/beweise/roh/NAK-310-matrixpruefung-4-auftrag.txt`, V3 = `docs/beweise/roh/NAK-310-matrixpruefung-3-validierung.md`. Alle Zeilennummern des Prüfers stimmen: M-116 `:1141`, M-97 `:1211`, M-102 `:1216`, M-109 `:1217`, M-113 `:1218`. Alle vier Befunde liegen im Prüfbereich: M-116 ist neu, M-97, M-102, M-109 und M-113 tragen „(Nacharbeit 3)" und R-310-15; MP4-Auftrag Frage 1 (F-3 geschlossen?) und Frage 2 („eine Regel R-310-1 bis R-310-15" gebrochen? „misst eine Zeile der Durchsicht … weiterhin einen Stellvertreter …, oder sagt eine Behauptung mehr, als ihre Zeilen messen?") fragen danach.

---

## G-1 — Die Warteschleife von `lauf` misst nur Stoppzahl und Exitcode (M-116, `:1141`)

### 1. Quelle

- M-116 `:1141`, Zustand (a): „Fassaden-Attrappe, die jeden Aufruf mit seinen Parametern aufzeichnet, … und vorgibt, wann die Warteschleife endet; die Pause der Warteschleife kostet an der Attrappe keine Zeit". Zusage: „(a6) Genau ein geordneter Stopp, danach Exit 1 (heute `:62-65`, `:71-74`). (a7) Genau ein geordneter Stopp, danach Exit 0 (heute `:66-69`, `:71`)." Reihenfolge: „… → Warteschleife, je Durchlauf fataler Listenerfehler vor dem Leerlaufende → geordneter Stopp → Exitcode". Rotbeweis (a): „`lauf` stoppt beim Leerlaufende nicht → (a7) 0 Stopps → rot; `lauf` gibt beim fatalen Listenerfehler 0 zurück → (a6) rot; `lauf` startet nach einem unbekannten Argument → (a1) 1 Startaufruf → rot." Zusage (b) zählt die Methoden der echten Fassade auf: Start, Startauskunft, fataler Listenerfehler, Leerlaufende, Stopp — die Pause fehlt, ebenso im Rotbeweis (b).
- §7.2 `:1821-1822` („fragt alle 250 ms den fatalen v3-Listenerfehler und das Leerlaufende ab"), `:1828-1831` („alles mit Wirkung geht über die übergebene Fassade — … Leerlaufende, Pause der Warteschleife, geordneter Stopp; je Durchlauf bleibt der fatale Listenerfehler vor dem Leerlaufende"), `:1836-1844` („Die echte Fassade reicht jede Methode an genau einen Aufruf weiter: … Pause an `std::thread::sleep` mit 250 ms, Bindungsordner an `std::fs::create_dir_all`, Standardort an `%APPDATA%` wie heute. Den Aufbau liest die Aufbau-Wache der Produktschicht (M-116 (b)), den Ablauf misst M-116 (a) an Attrappen").
- Code: `broker/src/main.rs:60-70` — je Durchlauf zuerst `sleep(250 ms)` (`:61`), dann der fatale Fehler (`:62-65`, `break true`), dann das Leerlaufende (`:66-69`, `break false`); `:71` genau ein Stopp; `:72-74` Exit 1, sonst Rückkehr aus `main` (Exit 0). `broker_soll_idle_enden` (`broker/src/lebenslauf.rs:299-327`) misst ab `idle_seit` (gesetzt beim Start, `:203`) und wird frühestens nach 60 s wahr: im Produkt läuft die Schleife viele Male nichtterminal durch.
- Die Stelle sagt, was der Prüfer behauptet.

### 2. Ablauf

1. Mutant des Prüfers: `lauf` fragt nach der Startmeldung genau einmal den fatalen Fehler; `true` → Stopp, Exit 1; sonst sofort Stopp, Exit 0.
2. (a1) bis (a5): unberührt, grün. (a6): meldet die Attrappe den Fehler bei der ersten Abfrage, ein Stopp, Exit 1 → grün; meldete sie ihn erst in einem späteren Durchlauf, wäre (a6) rot — die Zeile legt das nicht fest („vorgibt, wann die Warteschleife endet"), ein zeilentreuer Test besteht den Mutanten also. (a7): in jeder Lage grün (fatal nein → ein Stopp, Exit 0), denn keine Zusage zählt Leerlaufabfragen, Pausen oder Durchläufe.
3. M-36 (b) und M-95 (f): ihre Attrappe meldet „danach sofort das Leerlaufende" → grün. M-40 (`:1137`) und M-56 (`:1161`) messen die Zählfunktion an der Naht, nicht `lauf` → grün. Die Aufbau-Wache (b) liest `main.rs`, echte Fassade und `lebenslauf.rs`, nicht `lauf` → grün (R-310-14 erlaubt ihr auch nur Aufbau).
4. Weitere Mutanten, ebenso grün: Priorität vertauscht (Leerlauf vor fatal) — (a6) mit Leerlauf „nein" endet mit Exit 1, (a7) mit Exit 0, keine Lage setzt beide im selben Durchlauf; Schleife ohne Pause (Dauerschleife) — keine Zusage liest den Pausenaufruf; Pause 2 500 ms statt 250 ms in der echten Fassade — (b) nennt die Pause nicht.
5. Produktwirkung des Mutanten aus Punkt 1: der Broker endet unmittelbar nach dem Start mit Exit 0, weil der fatale Fehler im Normalfall falsch ist. Kein Bein sähe es: kein Test startet `eqcop-broker` (R-310-14; §5.3 `:538-541`), und im Diagnoseprojekt startet Gen keinen Broker (M-84; `tools/fl/LIES-MICH.md:207-209`).
6. Schließende Spur „wie heute" (`main.rs:60-70`, die Pause steht am Anfang jedes Durchlaufs): Leerlaufende nach einem nichtterminalen Durchlauf: Start → Startmeldung → Pause → fatal nein → Leerlauf nein → Pause → fatal nein → Leerlauf ja → Stopp → Exit 0; fataler Fehler nach einem nichtterminalen Durchlauf: … → Pause → fatal nein → Leerlauf nein → Pause → fatal ja → Stopp → Exit 1, ohne Leerlaufabfrage im letzten Durchlauf; neue Lage (a8): beide im selben Durchlauf → Exit 1, ohne Leerlaufabfrage. Die Spur im Validierungsauftrag (fatal nein → Leerlauf nein → Pause → …) lässt die erste Pause aus; „wie heute" steht sie vorn. Rotbeweise bei gehaltener Befehlszeile und Fassadenlage: zweite Abfrage fehlt → Spur endet nach „fatal nein" mit Stopp → rot; Priorität vertauscht → im nichtterminalen Durchlauf Leerlauf vor fatal, in (a8) Exit 0 → rot; Pause fehlt → keine Pause zwischen den Durchläufen → rot.
7. Den Wert 250 ms trägt die echte Fassade (§7.2 `:1840-1841`), nicht `lauf`; die Spur an der Attrappe sieht nur den Aufruf. Die 250 ms bindet deshalb nur die Aufbau-Wache (b), wenn ihre Liste „Pause an `std::thread::sleep` mit 250 ms" nennt (Rotbeweis „Pause mit 2 500 ms → Wache rot").
8. Nebenbeobachtung, kein Befund dieses Laufs: dieselbe Liste (b) nennt auch Bindungsordner und Standardort nicht, die §7.2 `:1841-1842` der echten Fassade zuweist.

### 3. Zusagenlage

- R-310-14 (§17.3 `:4099-4107`): „Alles mit Wirkung liegt in `lauf` hinter einer übergebenen Ausgabe und einer Fassade, die der Test durch Attrappen ersetzt: … Warteschleife mit fatalem Listenerfehler und Leerlaufende, geordneter Stopp, Exitcode. … Jede Zusage über diese Schicht hat eine Zeile, deren Test die Wirkung an der Attrappe liest (ausgegebener Text, übergebene Bindungen und Kennung, Stoppaufruf, Exitcode), und einen Rotbeweis, der die Wirkung bricht"; `:4112-4115`: „die echte Fassade und `broker_starten` sind je ein Aufruf, dessen Parameter die Wache mitliest". R-310-14 nennt weder 250 ms noch die Priorität; beide sind Zusagen aus §7.2 (`:1830-1831`, `:1840-1841`) und der Reihenfolge-Spalte von M-116.
- §7 Vorspann `:1291-1293`: „nennt jeder Satz, der ein Verhalten verspricht, die Matrixzeile, die ihn trägt" — §7.2 nennt M-116 (a) und (b), die Schleife und Pause nicht messen.
- §6 Beziehungen `:1039-1041`: „M-116 (`lauf`: Start, dann Leerlaufende oder fataler Listenerfehler, dann genau ein geordneter Stopp …)"; M-40 und M-56, Reihenfolge: „`lauf` fragt das Leerlaufende über die Fassade …(M-116 (a7))" — (a7) misst die Abfrage nicht.
- §8.4 (iii) mit Zusatz (v) (`:2549-2562`); MP4-Auftrag Frage 2. Mittelbar Gate T3-12-03 („v2-Namenskollision vom v3-Start entkoppeln"): der Mutant legt den ganzen Lauf still.

### 4. Urteil: PRÄZISIERT

Kern bestätigt: kein Test fängt ein `lauf` ohne Schleife, mit vertauschter Priorität oder ohne Pause. Präzisiert: (1) die 250 ms sind Parameter der echten Fassade (§7.2 `:1840-1841`), also Sache der Aufbau-Wache (b); die vom Prüfer verlangte Aufrufspur allein bindet sie nicht. (2) R-310-14 legt die Warteschleife hinter die Fassade und verlangt je Zusage eine messende Zeile; Priorität und Pause stehen als Zusagen in §7.2 und M-116, nicht in R-310-14. (3) „Wie heute" beginnt jeder Durchlauf mit der Pause.

**Klassenvorschlag: DEFEKT** (§8.4 (iii) und (v): Stoppzahl und Exitcode als Stellvertreter der Schleife; §7.2 nennt eine Zeile, die nicht misst). Produktgewicht hoch: der Mutant beendet den Broker gleich nach dem Start, ohne dass ein Bein rot wird.

**Kleinster Fix am Text:** M-116 (a6) und (a7) prüfen die vollständige Aufrufspur mit einem nichtterminalen Durchlauf (Punkt 6), neue Lage (a8) „beide im selben Durchlauf → Exit 1 ohne Leerlaufabfrage", Rotbeweise wie Punkt 6; (b) nennt „Pause an `std::thread::sleep` mit 250 ms" (dazu Bindungsordner und Standardort) mit Rotbeweis „2 500 ms → rot"; M-40, M-56 und §6 Beziehungen verweisen auf die Spur.

---

## G-2 — Anfragen nach dem Ende ohne beobachtbaren Schluss (M-97 `:1211`, M-102 `:1216`)

### 1. Quelle

- M-97 `:1211`: Ereignis „Hilfsprozess beenden (`Child::kill`, Ende über `Child::wait` bestätigt), danach Anfrage K2"; Zusage „Spätestens 1,5 s nach dem bestätigten Ende meldet der Probelauf den Briefkasten geschlossen (Thread beendet, Grund `SitzungBeendet`); K2 bleibt 5 s lang ohne Antwort der Rolle `broker`"; Reihenfolge „… einen laufenden Takt, der hier nach der beantworteten K1 nur seine Existenzprüfung macht"; Rotbeweis „die Sitzungsprüfung vor dem Takt entfernen → K2 wird beantwortet → rot".
- M-102 `:1216`: Ereignis „Nachprobe: eine Anfrage nach dem bestätigten Ende, 3 s Wartezeit (… die Schließfrist zählt hier nicht, denn nach dem bestätigten Ende beginnt kein Takt mehr, M-96, R-310-15 …)"; Zusage (b) „Exit 4 VERFEHLT mit dem Grund „Briefkasten antwortet nach dem Ende des Diagnose-FL (R-310-7)"". §7.4 `:2247-2250`: „legt der Arm nach `Beende-Diagnose-FL` … eine Anfrage, wartet 3 s … und erwartet keine Antwort; eine Antwort ist VERFEHLT".
- §18.6 Punkt 2 `:4438-4441`: „ein gehaltener Takt läse eine während des Haltens gelegte K2 nach der Freigabe selbst und beantwortete sie — nach R-310-15 zu Recht, denn er begann vor dem Ende"; gleichlautend M-113, Reihenfolge (`:1218`). Maßgeblich ist der Beginn des Takts, nicht die Entstehung der Anfrage. Die Stelle sagt, was der Prüfer behauptet.

### 2. Ablauf

1. Geplante Prüfstellen (M-96 Reihenfolge `:1210`, §7.4 `:2170-2173`): die Sitzungsprüfung steht in `Kern::lauf` am Schleifenanfang nach dem Stoppflag (`broker/src/briefkasten.rs:560`), also nach jedem Wecken (`:579-582` → Schleifenanfang), vor jedem Takt (`:564-571`) und nach jedem Takt (`continue`, `:575`). Heute gibt es keine Sitzungsprüfung (Bauinhalt).
2. Folge im Takt: `drop(weck)` `:570` → `takt()` `:571` → Testhaken `:595-600` (nur `cfg(test)`) → Taktzähler `:601` → `rumpf` → Zustandssperre `:608` → Existenzprüfung `:611` → Merker oder Lesen (`:626`, `lies` `:673`) → Ring `:641` → `beantworte` (`:653`, `:682-689`) → `schreibe` (`:728-751`). Sitzungsprüfung (unter `weck`) und Existenzprüfung (unter `zustand`) liegen unter verschiedenen Sperren, `weck` ist dazwischen frei; eine Anfragedatei des Tests ordnet ohnehin keine Sperre des Brokers.
3. Fenster: vom Bestehen der Sitzungsprüfung bis zur Existenzprüfung — normal wenige Anweisungen, vom Code nicht begrenzt; eine Verdrängung des Threads dehnt es beliebig.
4. M-97: K2 entsteht sofort nach `Child::wait`. Bestand die Prüfung kurz vor dem Ende und steht der Thread bis nach dem Schreiben von K2 vor seiner Existenzprüfung, sieht er K2, liest und beantwortet sie — nach §18.6 Punkt 2 korrekt; M-97 wird rot („K2 … ohne Antwort"). Derselbe Lauf kann auch die 1,5 s reißen, weil der Takt dann schreibt (`sync_all`, `MOVEFILE_WRITE_THROUGH`, Dauer ohne Obergrenze, R-310-15); dann stimmt auch der Reihenfolgesatz „nur seine Existenzprüfung" nicht.
5. M-102: `Beende-Diagnose-FL` (`tools/fl/laufzeit.ps1:545-553`) bestätigt das Ende über `Warte-Ende` (`:537-543`), das die Prozessliste mit 1-s-Schlaf abfragt; danach legt der Arm die Nachprobe an, die 3 s folgen dem Anlegen. Ein korrekter Broker antwortet nur, wenn ein vor dem Ende begonnener Takt bis nach Bestätigung und Anlegen vor seiner Existenzprüfung steht. Heute live unerreichbar: im Diagnoseprojekt startet Gen keinen Broker (M-84); der A35-Selbsttest `310/M-102` arbeitet mit einer Attrappe.
6. Wahrscheinlichkeit: nicht null, lastabhängig, vom Code nicht begrenzt, nicht gemessen (kein Lauf). M-97: das Rennen braucht nur eine Verdrängung länger als der Abstand zwischen Prozessende und K2; selten, aber möglich — ein roter Kanon belegte dann keinen Defekt. Als Test unzulässig, nicht nur unscharf. M-102: formal dasselbe, praktisch vernachlässigbar (Bestätigung im Sekundenraster), heute nicht erreichbar.
7. Synchronisation, die der Probelauf heute bietet: nur `briefkasten_gestartet` (`:934-937`: Startgrund, Threadgriff, Startzähler der Instanz = 1), `sensor_ids`, `laufzeit_id`; keine Meldung „geschlossen", kein Taktzähler nach außen, kein Haken — `haken_takt` ist ein privates Feld des privaten `Kern` unter `#[cfg(test)]`, ein Integrationstest sieht nur die öffentliche API (Quellen). Die Meldung „Schluss und Grund" ist Bauinhalt (§7.4 `:2208-2209`) und für die Zusage von M-97 ohnehin nötig.
8. Kleinster Fix M-97: K2 erst anlegen, nachdem der Probelauf „geschlossen (Thread beendet, Grund `SitzungBeendet`)" meldet; dann läuft kein Takt mehr, und keiner beginnt. Einen taktfreien Zustand vor dem Beenden zu synchronisieren ist unnötig und im Integrationstest nicht herstellbar (kein Haken; zwischen Synchronisation und `kill` kann ein neuer Takt beginnen). Der Rotbeweis „Sitzungsprüfung entfernen" fällt dann an der Schlussmeldung (keine Meldung binnen 1,5 s → rot); K2 misst „öffnet nie wieder".
9. Kleinster Fix M-102: eine beobachtbare Schlussgrenze hat der Runner nicht (`BrokerStatus`, `broker/src/lebenslauf.rs:384-396`, führt keinen Briefkastenzustand; `erzeugt_utc` entsteht erst nach dem Lesen, `broker/src/briefkasten.rs:695`, `:717`). Deterministisch ist eine eingeschränkte Erwartung: eine Antwort auf die erste Nachprobe allein ist kein Mangel (Rohzeile „ein vor dem Ende begonnener Takt kann antworten, R-310-15"); danach legt der Arm eine zweite Nachprobe, und eine Antwort auf sie ist VERFEHLT. Ein korrektes Produkt beantwortet nach dem Ende höchstens eine Anfrage (ein Takt, danach Threadende); ein offen gebliebener Briefkasten beantwortet beide. „Nach dem Ende beginnt kein Takt" misst M-96 deterministisch an der Bühne. Eine bloße Wartezeit vor der Nachprobe senkte nur die Wahrscheinlichkeit.

### 3. Zusagenlage

- R-310-15 (`:4133-4135`): „Ein Takt, der vor dem Ende begann, schreibt zu Ende — eine Antwort aus der Sitzung —, danach endet der Thread ohne weiteres Warten"; §18.6 Punkt 2 und M-113 (Reihenfolge) wenden das auf eine während des Haltens gelegte Anfrage an.
- Dagegen M-97 („K2 bleibt 5 s lang ohne Antwort") und M-102 (b) ohne Ausnahme; R-310-7 Gegenfall (a) (`:2793-2795`, „markierender Prozess endet → Anfrage danach unbeantwortet") ist die Fassung vor der Lesart R-310-15.
- MP4-Auftrag Frage 2: „Hat der Fixdiff etwas gebrochen — … eine Regel R-310-1 bis R-310-15 …; widerspricht eine neue Zeile einer bestehenden": M-97 und M-102 sind im Fixdiff unter R-310-15 geändert und halten deren Lesart nicht.
- §8.4 fragt es nicht: (i) prüft, ob der Rotbeweis das Ergebnis bricht, nicht, ob ein korrektes Produkt die Zeile immer besteht. Das Gate schweigt.

### 4. Urteil: PRÄZISIERT

Kern bestätigt: beide Zeilen können ein nach R-310-15 korrektes Produkt rot melden. Präzisiert: (1) das Fenster liegt zwischen Sitzungsprüfung und Existenzprüfung und öffnet sich nur durch Verdrängung; M-97 selten, M-102 vernachlässigbar und live heute unerreichbar. (2) Für M-97 genügt „K2 erst nach der Schlussmeldung"; der taktfreie Zustand vor dem Beenden ist unnötig und im Integrationstest nicht herstellbar. (3) Für M-102 gibt es keine beobachtbare Schlussgrenze, nur die eingeschränkte Erwartung.

**Klassenvorschlag: DEFEKT** (Test meldet ein nach R-310-15 korrektes Produkt rot) — M-97 voll, M-102 formal, mit geringem Gewicht.

**Kleinster Fix am Text:** Punkte 8 und 9.

---

## G-3 — Die Wartegrenze ist nur an der Uhr festgehalten (M-113 (b) `:1218`, M-109 `:1217`)

### 1. Quelle

- M-113 `:1218`: (a) „Die Uhr der echten Fassaden liefert als Schließfrist genau 500 ms (`SITZUNG_SCHLIESSFRIST`, als Zahl verglichen)"; (b) Zustand „Testuhr mit Taktfrist `LANG` (3 600 s …) und Schließfrist 20 ms …, keine Auslösung", Zusage „Ohne jede Auslösung endet der Thread binnen der Testfrist von 2 s mit `SitzungBeendet` …; ohne die Schließfrist im Warten wartete er bis zum Taktende nach 3 600 s"; Reihenfolge „`Kern::lauf` wartet mit dem kleineren Wert aus Taktrest und Schließfrist"; Rotbeweis (b) „die Schließfrist aus dem Warten nehmen (Warten nur bis zum Taktende) → (b) der Thread wartet 3 600 s → rot an der Testfrist".
- M-109 `:1217`, A4-Satz: „zwischen zwei Sitzungspruefungen wartet der Thread hoechstens die Schliessfrist von 500 ms (Produktwert an der echten Uhr festgehalten)"; Kernsatz des Textfunds „wartet der Thread hoechstens die Schliessfrist". §7.4 `:2180-2182`: „die feste Frist von R-310-7 bindet das Warten zwischen zwei Sitzungsprüfungen — höchstens die Schließfrist (M-113 (a), (b))"; gleichlautend M-96 (Reihenfolge), §6 Beziehungen `:1056-1058`, §7.4 Restlücke 1 `:2295-2297` und Risiko 2 `:2319-2322`.
- Die Stellen sagen, was der Prüfer behauptet.

### 2. Ablauf

1. `Kern::lauf` wartet heute am echten `Condvar` (`broker/src/briefkasten.rs:579-580`, `wait_timeout(weck, frist_ende - jetzt)`) mit einer Dauer aus `Instant::now()` (`:557`, `:563`); die Uhr-Fassade liefert nur Werte (`jetzt_utc_ms`, `taktfrist`, `:407-410`; die Schließfrist wird als zweite Methode Bauinhalt, §7.4 `:2175-2178`). Die Testuhr (`:1090-1103`) hält eine UTC-Zeit für den Umschlag und die Taktfrist; sie stellt keine monotone Zeit vor, zeichnet kein Warten auf und weckt nicht (geweckt wird über `ausloesen`, `:1248-1253`, und den Stoppweg).
2. Mutant des Prüfers, Warten mit `min(Taktrest, min(2 × Schließfrist, 1 s))`: (a) grün, die Uhr liefert weiter 500 ms; (b) wartet 40 ms und endet weit binnen 2 s → grün; M-96 misst keine Zeit (`LANG`, Auslösungen) → grün; M-97: im Produkt 1 s Warten, Schluss binnen 1 s plus Planung, unter 1,5 s → grün. Nachgerechnet.
3. Stärker: `Kern::lauf` wartet mit `TAKTFRIST` und benutzt die Schließfrist der Uhr gar nicht → (a) grün, (b) 1 s < 2 s → grün, M-97 grün. Damit ist auch R-310-11 („nicht `TAKTFRIST`") am Warten ungemessen. Ein Warten von 1,5 s (dreifache Schließfrist) verletzt R-310-7 („höchstens 1 s"); (b) sähe 60 ms → grün, M-97 wird nach eigenem Wortlaut „nur zufällig rot, je nachdem, wo im Warten das Ende liegt".
4. Was der Code hergibt: ein deterministischer Fristrand bräuchte eine vorstellbare monotone Uhr und ein Warten über die Fassade — beides fehlt. Möglich ist ein Testhaken am Kern nach dem Muster `haken_takt` (`:548-549`), der jede an `wait_timeout` übergebene Dauer und jede Sitzungsprüfung in eine Spur der Instanz schreibt. (b) liest dann: jede übergebene Dauer ≤ 20 ms, und zwischen zwei Warten liegt eine Sitzungsprüfung. Rotbeweise bei gehaltenem Zustand: doppelte Schließfrist → 40 ms → rot; `TAKTFRIST` statt Schließfrist → 1 000 ms → rot; Prüfung nur nach jedem zweiten Wecken → zwei Warten ohne Prüfung → rot. „An der Testuhr" ginge nur, wenn das Warten eine Methode der Uhr-Fassade würde; der Haken ist die kleinere Änderung.

### 3. Zusagenlage

- R-310-15 (`:4129-4133`): „Die feste Frist von R-310-7 (höchstens 1 s) bindet das Warten des Briefkastenthreads zwischen zwei Sitzungsprüfungen (Schließfrist `SITZUNG_SCHLIESSFRIST`, Produktwert 500 ms, M-113 (a))"; `:4143-4148`: „Jede Behauptung … nennt die gemessene Größe: „… zwischen zwei Sitzungsprüfungen wartet der Thread höchstens die Schließfrist von 500 ms (Produktwert an der echten Uhr) …"". R-310-7 (`:2788-2789`): „binnen einer festen Frist (höchstens 1 s)"; R-310-11 (`:3379-3384`): „eine eigene Konstante im Broker (höchstens 1 000 ms, nicht `TAKTFRIST`)".
- R-310-9 (`:2813`): „die nicht mehr sagt, als der Test misst"; §7.0 Punkt 6 (`:1390-1391`); §8.4 (v) und (vi) (`:2557-2566`): „Nennt die Behauptung die gemessene Größe oder nur einen ihrer Parameter?" — gemessen ist der Parameter an der Uhr, der Weg zum Warten nur mit Faktor 100 (20 ms gegen 2 s).
- MP4-Auftrag Frage 1 („nennt der A4-Satz von M-109 und jede gleichlautende Stelle … nur die gemessene Größe") und Frage 2.
- Die Regel schreibt den Satz selbst vor. Zu ändern ist nicht die Regel, sondern (b), damit der vorgeschriebene Satz gemessen ist.

### 4. Urteil: BESTÄTIGT

**Klassenvorschlag: DEFEKT** (Behauptung größer als Messung: R-310-9, R-310-15, §8.4 (vi); Weg Uhr → Warten ungemessen: §8.4 (v)). Sicherheitsnah: die Frist von R-310-7, die Gate-Satz T3-12-04 trägt (§17.2 `:4051-4054`), ist am Warten nur zufällig gemessen.

**Kleinster Fix am Text:** M-113 (b) liest eine Warte- und Prüfspur an der Instanz (Bauinhalt, Muster `haken_takt`): jede übergebene Dauer ≤ Schließfrist, nach jedem Wecken eine Sitzungsprüfung; Rotbeweise „doppelte Schließfrist → 40 ms → rot" und „`TAKTFRIST` statt Schließfrist → 1 000 ms → rot"; §7.4 `:2182` und M-109 nennen (b) als Messung des Wartens.

---

## G-4 — „Schreibt zu Ende" hat keine Zeile (M-113 (c) `:1218`, M-109 `:1217`)

### 1. Quelle

- M-113 (c) `:1218`: Zustand „… der Testhaken `haken_takt` der Instanz … hält den ersten Takt an einer Schranke …"; Ereignis „… es liegt die Anfrage K2, deren erstes Lesen die Dateisystem-Attrappe scheitern lässt (Injektion `lesen`, `:1004-1016`) …"; Zusage „Der gehaltene Takt läuft nach der Freigabe zu Ende (eine Existenzprüfung, ein gescheitertes Lesen, keine Antwort); danach endet der Thread ohne weiteres Warten …"; Rotbeweis „nach dem gehaltenen Takt wird die Sitzung nicht geprüft und der nächste Takt beginnt … → rot".
- Die Zusage „schreibt zu Ende" steht in R-310-15 (`:4133-4134`), im A4-Satz von M-109 („ein Takt, der vor dem Ende begann, schreibt zu Ende"), in §7.4 `:2183-2185` („… schreibt zu Ende — eine Antwort aus der Sitzung —, danach endet der Thread ohne weiteres Warten (M-113 (c))"), in M-96 (Reihenfolge, „(M-113 (c))"), §6 Beziehungen `:1058-1059`, §7.4 Restlücke 1 und Risiko 2.
- §18.6 Punkt 2 (`:4436-4446`) begründet die Leseinjektion: ohne sie beantwortete der gehaltene Takt K2 selbst.
- Die Stellen sagen, was der Prüfer behauptet.

### 2. Ablauf

1. Mutant des Prüfers: nach erfolgreichem Lesen (`Lesung::Kennung`, `broker/src/briefkasten.rs:633-637`) prüft `rumpf` die Sitzung erneut und kehrt vor `beantworte` (`:653`) zurück.
2. M-113 (c): das erste Lesen scheitert (`:674` → `Lesung::Spaeter` → `return`, `:627`), `beantworte` wird nie erreicht → grün. (a), (b): kein Takt → grün. M-96: K1 bei lebender Sitzung beantwortet, danach beginnt kein Takt → grün. M-97: K2 nach dem Ende ohne Antwort → grün. M-98, M-117: die Sitzung lebt, die erneute Prüfung sieht „lebt" → grün. Keine Zeile hält einen Takt mit gültiger Anfrage über das Sitzungsende. Nachgerechnet.
3. Vorbild im Code: `nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal` (`:1816-1896`, NAK-309 M-66) misst dieselbe Eigenschaft für den Stopp (Haken `beim_schreiben`, nach der Freigabe genau eine Antwortdatei, kein neuer Takt). Für das Sitzungsende fehlt das Gegenstück.
4. Teilfall (d): K1 gültig vor der Auslösung, keine Injektion; `haken_takt` hält den Takt vor der Existenzprüfung (`:595-600`); währenddessen meldet die Sitzungsfassade „beendet"; nach der Freigabe genau eine Antwort auf K1 (Datei, Zähler `antworten` = 1), Taktzähler 1, Threadende ohne weiteres Warten binnen der Testfrist von 2 s mit `SitzungBeendet` (Takt- und Schließfrist `LANG`), Sitzungsgriff geschlossen. Der Halt muss vor dem Lesen liegen: ein Halt im Schreibschritt (`beim_schreiben`) läge hinter der erneuten Prüfung des Mutanten und fände ihn nicht. Rotbeweis, der die Vorbedingung hält und das Ergebnis bricht (R-310-12): „nach dem Lesen die Sitzung erneut prüfen und vor `beantworte` abbrechen → keine Antwort auf K1 → rot".
5. Verträglichkeit: (d) läuft auf einer eigenen Bühne ohne Injektion; (c) behält ihre Injektion auf ihrer Bühne, die Injektion gehört je Bühne der Attrappe (`TestDateisystem`, `:1020-1026`). (c) misst „kein weiteres Warten, kein weiterer Takt", (d) „schreibt zu Ende". Kein Widerspruch.

### 3. Zusagenlage

- R-310-15 (`:4133-4134`, oben); R-310-9 (`:2810-2813`): die Behauptung „schreibt zu Ende" in M-109 hat keine messende Zeile; §7 Vorspann `:1291-1293`: §7.4 und M-96 nennen M-113 (c) als tragende Zeile, (c) misst anderes; §8.4 (iii); MP4-Auftrag Frage 1 („trägt M-113 den Teilfall (c)") und Frage 2.
- R-310-15 schreibt (c) mit genau diesem Inhalt vor und ordnet „schreibt zu Ende" keinen Teilfall zu. Das Gate schweigt (die Lesart erlaubt eine Antwort mehr, sie verlangt keine).

### 4. Urteil: BESTÄTIGT

**Klassenvorschlag: DEFEKT** (Zusage ohne messende Zeile; Behauptung größer als Messung, R-310-9). Produktgewicht gering: der Mutant wirkt fail-closed (eine Antwort weniger).

**Kleinster Fix am Text:** M-113 Teilfall (d) wie Punkt 4; M-96, §7.4 `:2185` und M-109 nennen (d) neben (c).

---

## Übersicht

| Befund | Urteil | Klassenvorschlag | tragender Satz | kleinster Fix |
|---|---|---|---|---|
| G-1 · M-116 `:1141` | PRÄZISIERT | DEFEKT | R-310-14 `:4099-4107` „Warteschleife mit fatalem Listenerfehler und Leerlaufende … Jede Zusage über diese Schicht hat eine Zeile, deren Test die Wirkung an der Attrappe liest"; §7.2 `:1830-1831`, `:1840-1841` | (a6)/(a7) mit vollständiger Aufrufspur samt nichtterminalem Durchlauf (Pause vorn), (a8) beide zugleich → Exit 1 ohne Leerlaufabfrage; (b) nennt die Pause mit 250 ms, Rotbeweis 2 500 ms |
| G-2 · M-97 `:1211`, M-102 `:1216` | PRÄZISIERT | DEFEKT (Test meldet korrektes Produkt rot; M-102 mit geringem Gewicht) | R-310-15 `:4133-4135` mit §18.6 Punkt 2 `:4438-4441`; MP4-Auftrag Frage 2 | M-97: K2 erst nach der Meldung „geschlossen, `SitzungBeendet`"; M-102: eine Antwort auf die erste Nachprobe ist Rohzeile, eine Antwort auf eine danach gelegte zweite ist VERFEHLT |
| G-3 · M-113 (b) `:1218`, M-109 `:1217` | BESTÄTIGT | DEFEKT | R-310-15 `:4129-4133`, `:4143-4148`; R-310-9 `:2813`; §8.4 (v), (vi) | (b) liest die an `wait_timeout` übergebenen Dauern und die Sitzungsprüfungen an einer Spur der Instanz (Muster `haken_takt`); Rotbeweis doppelte Schließfrist → 40 ms → rot |
| G-4 · M-113 (c) `:1218`, M-109 `:1217` | BESTÄTIGT | DEFEKT | R-310-15 `:4133-4134` „schreibt zu Ende — eine Antwort aus der Sitzung"; R-310-9 `:2813` | Teilfall (d): gültige K1, Halt vor der Existenzprüfung, Sitzungsende, genau eine Antwort, Threadende ohne Warten; Rotbeweis Abbruch vor `beantworte` → rot |

## Gemeinsame Ursache

G-1, G-3 und G-4 haben dieselbe Ursache wie die Runden 1 bis 3 — Stellvertreter statt Zusage, Zusage ohne messende Zeile —, in neuer Form: **Verweis statt Messung.** Die Zeilen messen jetzt am richtigen Ort (Attrappe, Bühne), aber nur die Beispiele, die die Regeln nennen. R-310-14 zählt als Wirkungen „Stoppaufruf, Exitcode" auf — M-116 (a6)/(a7) prüfen genau diese, nicht die Warteschleife, die dieselbe Regel hinter die Fassade legt. R-310-15 nennt für die Frist „Produktwert 500 ms, M-113 (a)" und für den laufenden Takt Teilfall (c) mit „ohne weiteres Warten" — gemessen wurde genau das, nicht das Warten und nicht „schreibt zu Ende". Die weitergehenden Sätze stehen anderswo (§7.2, §7.4, §6 Beziehungen, Reihenfolge von M-40, M-56 und M-96, A4-Satz von M-109) und verweisen auf die Zeile; die Durchsicht §18.4 prüfte je Zeile deren eigene Zusage, nicht die Sätze, die auf sie verweisen. G-2 ist eine **neue Klasse: ein Test, der ein korrektes Produkt rot melden kann.** R-310-15 erlaubt einem laufenden Takt die Antwort; die Ereignisfolgen der Zeilen, die gleich nach dem Ende eine Anfrage legen (M-97, M-102), wurden nicht neu abgeleitet, nur ihre Zeitbegründung. Keine der sechs Fragen §8.4 fragt, ob ein nach den Regeln korrektes Produkt die Zeile immer besteht.

## Registerreife

Ja. Jeder Fix ist eine abgeschlossene Textänderung an einer Zeile; die Mechanik liegt vor oder ist schon Bauinhalt, neu ist nur die Wartespur von M-113 (b). **M-116:** (a6) und (a7) prüfen die vollständige Aufrufspur mit einem nichtterminalen Durchlauf wie `main.rs:60-70`, die neue Lage (a8) setzt fatalen Fehler und Leerlaufende im selben Durchlauf und erwartet Exit 1 ohne Leerlaufabfrage, und (b) nennt die Pause mit 250 ms (dazu Bindungsordner und Standardort), je mit Rotbeweis. **M-97:** K2 wird erst angelegt, wenn der Probelauf „geschlossen (Thread beendet, `SitzungBeendet`)" meldet — die Meldung verlangt §7.4 `:2208-2209` ohnehin —, und der Rotbeweis fällt an dieser Meldung. **M-102:** eine Antwort auf die erste Nachprobe ist Rohzeile, eine Antwort auf eine danach gelegte zweite ist VERFEHLT; der A35-Satz in M-109 zieht nach. **M-113:** (b) liest eine Warte- und Prüfspur an der Instanz (neuer Testhaken nach dem Muster `haken_takt`) mit „jede Dauer ≤ Schließfrist", und der neue Teilfall (d) hält einen Takt mit gültiger K1 über das Sitzungsende und erwartet genau eine Antwort, danach Threadende ohne Warten. Folgestellen: M-40, M-56, §6 Beziehungen, §7.2 (G-1); M-96, §7.4 `:2182`, `:2185`, M-109 (G-3, G-4).

## Nicht geprüft

- Kein Lauf. Die Wahrscheinlichkeit des Rennens in M-97 und M-102 ist nicht gemessen; gelesen ist nur, dass der Code das Fenster nicht begrenzt.
- Gelesen: Kopf, §2, §5.3 (Randbedingungen), §5.6 (R-310-11, R-310-15), §6 Kopf und Regeln, §6.3, M-55 bis M-57, §6.6, §7 Vorspann, §7.0 (Punkt 6, Grenzen), §7.2 bis `:1870`, §7.4, §8.4, §11.3, §13.4, §17, §18.1 bis §18.6, §18.8, §19; MP4-Auftrag, Rohurteil, V3. Nicht gelesen: §5.0 bis §5.2, §5.4, §5.5, §6.1, §6.2, §6.5, §6.7, §6.8, §7.1, §7.3, §7.5, §8.1 bis §8.3, §8.5, §8.6, §9 bis §16, §18.7, §18.9, die Validierungen der Runden 1 und 2.
- Namen, Fassadenzuschnitt und Ort der Wartespur sind Bauinhalt; geprüft ist nur, dass der Code sie hergibt.

## Quellen im Netz (abgerufen 23.09.2026)

- Rust Reference, Conditional compilation, Option `test`: „Enabled when compiling the test harness. Done with `rustc` by using the `--test` flag." — https://doc.rust-lang.org/reference/conditional-compilation.html
- Cargo Book, Cargo Targets, Integration tests: „Integration tests can use the public API of the package's library." — https://doc.rust-lang.org/cargo/reference/cargo-targets.html

FERTIG Validierung Matrixprüfung 4 NAK-310, 2 bestätigt, 2 präzisiert, 0 widerlegt.
