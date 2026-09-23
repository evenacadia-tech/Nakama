# NAK-312 Etappe 7b Teil 1 — Quellvalidierung der Erstprüfung (D-1 bis D-3)

- Validierer: frischer, lesender Opus-5.5-Thread, Effort max (nicht Bauer 4c811fa4, nicht Prüfer 01a0cc61), 23.09.2026, Messung 06:15 Uhr (`date`). Nur gelesen: kein Compiler, kein Test, kein FL, kein Python; nichts gestaged oder committet; einzige geschriebene Datei ist diese.
- HEAD vorher: `09a1ae4ac7e021d91ee700697b55abb26fe98af5`. HEAD nachher: `09a1ae4ac7e021d91ee700697b55abb26fe98af5`.
- Der Auftrag nennt `f636d1a3` als HEAD; inzwischen steht dort `09a1ae4a` (Dirigent, nur `docs/**`: Rohurteil, Validierungsauftrag, Manifest §56). `git diff --stat 19bb1803 HEAD` über die 18 Pfade ist leer, ebenso `git diff --stat 19bb1803` (Arbeitsbaum) über die 18 Pfade samt `PluginEditor.h`, `Hostbruecke.cpp` und `HoerMarkierung.h`. Alle Zeilenangaben unten gelten deshalb am ZIEL `19bb1803`.

## D-1 — Befundzähler nach dem Nachrücken über den lokalen Eintritt

**1. Quelle.** Die Nachrückschleife steht in `setzePersistenteMitglieder`. Kopf: `SourcesModel::Publikation SourcesModel::setzePersistenteMitglieder (const std::vector<nakama::state::MainProjectMitglied>& mitglieder, std::uint64_t generation, std::uint64_t folge, nakama::state::Klasse klasse)` (`eq-copilot/plugin/src/SourcesModel.cpp:370-374`, `mutex` ab `:383`). Die Schleife steht an `:453-478`: `for (const auto& id : nimmAn (fluechtige))` (`:461`), `e = std::move (wartend->second);` (`:468`), `klassifiziere (e);` (`:476`) und `eintraege.emplace (id, std::move (e));` (`:477`). Danach folgen nur `stelleZielSicher(); revidiere();` (`:479-480`). Die Zeilen des Prüfers (`:476-477`) und des Auftrags (`:461-477`) stimmen. Die Schleife ist neu in 7b (`git diff e1bd9c6d 19bb1803`; vorher legte die Stelle nur frische Einträge für gespeicherte Mitglieder an).
- Aufrufer von `zaehleOffeneFindings()`: direkt nur `uebernehmeSessionSnapshot` (`:1441`), dazu `setzeAlleBefundeStale` (`:1804`), gerufen von `controlEnde` (`:654`) und `uebernehmeEvidenzruecknahme` (`:1605`). `uebernehmeP2` ruft sie nicht (`:1613-1693`). `sitzungszustandLeeren` (`:589-611`) leert `befunde` und setzt jeden Zähler auf 0. Es läuft in `projektReload` (`:562`), `beginneSubscription` (`:623`) und `legeLiveSichtStill` (`:586`). `setzePersistenteMitglieder` hat keinen dieser Aufrufe.
- Grundlage der Zählung: `zaehleOffeneFindings` (`:1771-1790`) setzt alle Einträge auf 0 und zählt dann jeden nicht `stale` Befund aus `befunde` auf `eintraege.find (b.candidateSource)`. Findet sie keinen Eintrag, fehlt der Befund in der Zählung (`:1783-1784`). `befunde` ist „der zuletzt empfangene Befundstand dieser Sitzung“ (`SourcesModel.h:442-443`) und wird ganz ersetzt (`:1425`). Der Leser prüft `candidate_source` nur auf hex32 (`:966-974`), nicht auf Annahme. Ein Befund für ein wartendes X bleibt also in `befunde`, und `Sicht::befunde` trägt ihn (`:1731`).
- Der wartende Eintrag entsteht im Snapshot (`:1338-1403`), als Default oder als Kopie von `alt` (`:1339-1342`). Ein wartendes X hatte vorher keinen Eintrag, denn `nimmAn` lässt Angenommene ihren Platz behalten (`:1865-1866`). Deshalb ist `findingsOffen` = 0 (Default `SourcesModel.h:80`). `klassifiziere` (`:434-441`) setzt nur Mitgliedschaft, Label und Name.
- Keine Korrektur: `tick()` (`:1695-1713`) führt über `aktualisiereAbgeleiteteZustaende` (`:1916-1960`) nur Control, Messung und Alter nach. `revidiere()` ist `++revision` (`SourcesModel.h:462`). `stelleZielSicher()` (`:1879-1895`) setzt nur `hauptziel`.
- Anzeige: `PluginEditor.cpp:1529-1531` rechnet `ziel->findingsOffen == 0 ? "Findings: 0 - no findings yet" : …`, mit `ziel` = Zeile mit `hauptziel` (`:1429-1431`). Dazu zeichnet jede Zeile ihre Zahl (`:1383-1384`, Spaltenkopf „Findings“ `:1334`). Beide Anzeigen lesen `Zeile::findingsOffen` aus `sicht()` (`:1737-1738`).

**2. Ablauf, nachgerechnet.** Beispiel mit 20 gespeicherten Quellen G0 bis G19; mit 21 gespeicherten (X darunter) läuft es gleich.
1. `setzePersistenteMitglieder (G, …)`: 20 Zeilen, `Control::getrennt`, ohne Runtime-Nonce (`:471-475`).
2. `beginneSubscription`, danach ein Snapshot mit der flüchtigen Sonde X und `findings: [{candidate_source: X, zustand: ready_to_send}]`. `nimmAn` behält die 20 (`:1865-1866`), X kommt nach `wartend` (`:1402-1403`, `:1422`). `befunde` enthält den Befund (`:1425`). `zaehleOffeneFindings` findet X nicht (`:1784`). Alle Zeilen zeigen 0, `nichtAngenommen` ist 1. Bis hierher gilt M-128.
3. Der User wählt G0 (offline, ohne Nonce) und entfernt es: `entferneSourcesHauptziel` (`Ipc.cpp:1247`) nimmt ohne hex32-Nonce den lokalen Weg (`:1259`, `erase` `:1277`, Publikation `:1286-1287`).
4. Im Modell: G0 ist weder gespeichert noch flüchtig und fällt, dabei wird `hauptziel` geleert (`:444-447`). `fluechtige` enthält X aus `nichtAngenommene` (`:459-460`), `nimmAn` nimmt X an (`:1873-1874`). X rückt über `:468` mit `findingsOffen` 0 ein (`:476-477`). Es folgt keine Neuzählung.
5. `stelleZielSicher` wählt das Ersatzziel in Anzeigeordnung. Wird X Hauptziel, automatisch oder per `waehleHauptziel` (`:1748-1756`), zeigt der Editor „Findings: 0 - no findings yet“, obwohl `Sicht::befunde` einen offenen Befund mit `candidate_source` X trägt. Die Zeile X zeigt „0“ auch ohne Auswahl (`PluginEditor.cpp:1383`).
6. Der Zustand hält bis zum nächsten Snapshot (`:1441`) oder einem anderen Sitzungsschnitt: `controlEnde` oder Evidenzrücknahme (alle Befunde `stale`, 0 ist dann wahr), Reload, neue Subscription, Wechsel nach `legacy`. Der lokale Handgriff löst selbst keinen Snapshot aus, denn er läuft ohne Broker.
- Ergänzung: Dieselbe Schleife läuft auch in der Nachführung nach einem ACK (`Ipc.cpp:1553`), etwa bei `unbindProbe` für ein Y mit Nonce (`Ipc.cpp:1290`). Ob der Broker danach sofort einen Snapshot schickt, ist nicht geprüft.
- Snapshot-Eintritt (M-125): Dort folgt die Zählung, ja. X rückt über `neu` ein (`:1402-1403`), danach laufen `eintraege = std::move (neu)` (`:1421`), `befunde = …` (`:1425`) und `zaehleOffeneFindings()` (`:1441`). Nur der Eintritt `setzePersistenteMitglieder` ist betroffen. `projektReload` leert Wartende und Befunde (`:528`, `:562`) und bleibt damit stimmig.
- Rotbeweis mit vorhandenen Zugängen (B13, Muster `Sonde012SourcesModelTest.cpp`): Die Schritte 1 bis 4 über die Testform `setzePersistenteMitglieder (…, 0, 1)` (`SourcesModel.h:300-310`, benutzt an `:980`), `beginneSubscription`, `uebernehme (m, snapshot (q, hex (1), hex (2), hex (8), hex (10), false, 0, { befundJson (…, x.id, "ready_to_send") }), t)` (Muster `:1052-1055`, Helfer `:141-158`, `:160-192`, `:215-220`), dann `setzePersistenteMitglieder (G ohne G0, 0, 2)`. Erwartet: X ist Zeile, `nichtAngenommen` 0, `befunde.size() == 1`, Zeile X hat `findingsOffen == 1`. Am ZIEL steht dort 0, der Test ist also heute rot (nach Lesart, nicht gefahren). Kein Prozessor und kein Broker nötig.

**3. Zusagenlage.**
- M-125 (`NAK-312.md:9115`), Zusage: „**X rückt im selben Snapshot nach: 20 Zeilen, X darunter, `nichtAngenommen` = 0, die Meldung verschwindet. …**“ Das betrifft nur den Snapshot-Eintritt; dort gilt die Zählung.
- M-126 (`:9116`), Reihenfolge: „`projektReload` … und `setzePersistenteMitglieder` (`:370-457`) nehmen dieselbe Regel“. Gemeint sind Zeilen und Bestand, der Zähler kommt nicht vor.
- M-128 (`:9118`), Zusage: „keine Zeile zählt einen Befund für X“, Reihenfolge: „`zaehleOffeneFindings` zählt nur Einträge (`:1716-1717`) — die nicht angenommene Quelle hat keinen“. Das gilt für den Wartezustand und hält. Die Zeile sagt nichts zu X nach dem Nachrücken.
- §47.2 (`:9432-9434`): „Wer keinen Platz bekommt, ist nicht angenommen: kein Eintrag, keine Zeile, keine Messung …, kein Befundzähler“. Auch das betrifft nur den Wartezustand. Das Nachrücken ohne Snapshot ist die Abweichung A-116 des Bauers (`:11224-11231`: „Damit ein Platz, der ohne neuen Snapshot frei wird (Unbind eines gespeicherten Mitglieds), ohne Wartezeit an eine verbundene Sonde gehen kann …“). Nach §54.2 (`:11057-11059`) rückt die Sonde „mit ihrem Stand aus dem jüngsten Snapshot“ nach; zu diesem Stand gehört der offene Befund.
- U51 (`2026-09-21-quellenannahme-auf-20-begrenzt-u51.md:24-25`): „Eine weitere meldet ehrlich, dass die Liste voll ist, und misst nicht mit, bis Platz ist.“ M-128 ordnet den Befundzähler unter „misst nicht mit“ ein. Dass X mit Platz sofort mitzählt, sagt U51 aber nicht wörtlich.
- Prüffrage (c): „Ist die Annahmeregel im Modell … in allen drei Eintritten … unter demselben mutex deterministisch und ohne Wandzeit …“. Die Annahme hält; den Zähler erfasst die Frage nicht. R-312-25 bis R-312-31 (§50.4) und Gate-Text schweigen zum Zähler.
- Invariante `CLAUDE.md:44-45`: „**Keine toten UI-Elemente** (User 24.08.2026): jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand.“ Der Prüfauftrag nennt „keine toten UI-Elemente“ ausdrücklich als Defektkriterium (Auftrag Zeile 15). Das Modell selbst nennt die Abweichung des Zählers von `befunde` genau dieses tote Element: `SourcesModel.cpp:1426-1431` (M-84: ein Zähler, der immer 0 zeigt, ist „genau das sinnlose tote Element, das CLAUDE.md ausschliesst“) und `:1796-1800` (die Gegenrichtung: ein Zähler, der „Arbeit behauptet, die es nicht gibt“).
- Register: NAK-367 betrifft die Brokerseite, NAK-371 die Codebase-Pflege. Keiner deckt den Fall.

**4. Urteil: BESTÄTIGT.** Quelle, Funktion, fehlende Neuzählung, Anzeige und Ablauf stimmen wie beschrieben. Ergänzungen ohne Widerspruch:
- Y braucht für den lokalen Weg keine hex32-Nonce.
- Dieselbe Lücke entsteht über die ACK-Nachführung.
- Die Zeilenzahl „0“ steht auch ohne Auswahl von X.
- Korrigiert wird auch durch `controlEnde`, Evidenzrücknahme, Reload, Subscription oder Legacy-Wechsel.

**Klassenvorschlag: DEFEKT** gegen `CLAUDE.md:44-45` („meldet ehrlich einen Zustand“), ein Defektkriterium des Prüfauftrags. Die sichtbare Aussage „no findings yet“ widerspricht dem gehaltenen Befundstand. Eingeführt hat sie der Änderungssatz. Matrix und Gate schweigen zum Zähler beim Nachrücken über `setzePersistenteMitglieder`. Hält der Dirigent die Invariante für einen vorübergehenden Anzeigezustand nicht für tragend, bleibt LÜCKE mit Dirigentenregel.

## D-2 — Prüfdeckung von M-130 „beim ersten erlaubten Block“

**1. Quelle.** `eq-copilot/plugin/tests/MarkierungTestMain.cpp:2268-2275` (Zweig `Weg::release` in `nak312M64`): `r.auftragNachBloecken = s.p->markierungZielGesetztFuerTest(); s.p->prepareToPlay (fs, bs); r.mitErlaubnis = s.bloecke (40); r.nachPrepare = s.ernte(); r.hoerbarDanach = s.p->markierungHoerbar();`. Die Prüfung steht an `:2298-2300`: `a.auftragNachBloecken && a.nachPrepare.begins == 1 && a.nachPrepare.ends == 0 && neuesBegin.isNotEmpty() && neuesBegin != a.beginId && a.mitErlaubnis.hoerbar > 0 && a.hoerbarDanach`. Die Zeilen des Prüfers (`:2298-2300`) stimmen.
- `bloecke (n)` (`:1546-1578`) liefert nur Summen: `abweichend`, `hoerbar` (Blöcke, nach denen `markierungHoerbar()` gilt, `:1571-1575`), `letzteAbweichung` (Index des LETZTEN abweichenden Blocks), `sprungAmAnfang` und `groessterSprung` (`Lauf`, `:1481-1488`). `ernte()` (`:1582-1599`) liefert alle Ereignisse seit der letzten Ernte.
- Den ersten Block kann derselbe Test einzeln beobachten. Muster 312/M-86 in derselben Datei: `const auto ersterMit = s.bloecke (1); const int nachErstem = s.p->interventionsRingFuellstandFuerTest(); const auto mit = s.bloecke (39);` mit `ersterMit.abweichend == 1 && nachErstem == 2` („der erste Block mit Erlaubnis meldet sein begin im selben Block, in dem er faerbt“, `:2117-2119`, `:2128-2136`). Alternativen: `ernte()` nach `bloecke (1)`, oder die Summe schärfen. `klingtInMain` verlangt `an.hoerbar == 40` (`:1902`); die Prüfung verlangt nur `> 0`.
- Der Text der Prüfung sagt mehr, als die Bedingung misst: „… beginnt er beim ersten erlaubten Block neu …“ (`:2301-2304`, ebenso der Kommentar `:2265-2267`).

**2. Ablauf.** Nach `prepareToPlay` laufen 40 Blöcke mit Erlaubnis; geprüft werden nur Summen über alle 40. Ein Auftrag, der erst in Block 2 bis 40 beginnt und am Ende hörbar ist, liefert `begins == 1`, `ends == 0`, ein neues `begin`, `hoerbar` zwischen 1 und 39 (also `> 0`) und `markierungHoerbar()` wahr. Die Prüfung bleibt grün. Die Aussage des Prüfers stimmt.
- Rotbeweis `docs/beweise/roh/NAK-312-rot-M-130.txt`: Die Mutation ist `markierung.reicheAus()` nach `markierungAbbrechen()` in `releaseResources()` (Zeilen 14-21). Rot wird „Auftrag nach den Bloecken WEG, 0 begin, 0 end, hoerbar 0 von 40“ (Zeile 28). Er fällt also an „Auftrag bleibt“ und dem ausbleibenden `begin`, nicht am Zeitpunkt „erster Block“.
- Produktcode, nur gelesen: `releaseResources` (`Hostbruecke.cpp:178-188`) ruft `brichAb`. Das setzt nur Fade, `warHoerbar` und `hoerbareSamples` zurück; Auftrag, Briefkasten und Oversize-Riegel bleiben (`HoerMarkierung.h:334-351`). `gibPufferFrei` setzt `wetKapazitaet = 0` (`:356-360`). Die 5 Blöcke danach sind Oversizeblöcke: der Riegel steht, nichts klingt (`:490-500`). `prepareToPlay` (`Hostbruecke.cpp:79-80`) löst den Riegel in `setzeSamplerate` (`HoerMarkierung.h:717`) und `vorbereiten` (`:313`) und teilt den Wet-Puffer neu zu (`:305-306`).
- Im ersten Block danach ist `zielAn` wahr, sobald `erlaubt` gilt. Die übrigen Terme (`gelesenNr != 0`, `lokal`, `fs`) bleiben unverändert (`:533-539`). Die Rampe hebt `fade` schon beim ersten Sample über 0 (`:630`), und `begann` fällt im selben Block (`:658-663`). `erlaubt` (`PluginProcessor.cpp:877-884`) ist im Test ab dem ersten Block wahr: `testEchtzeit`, Editor offen, `spielt` kommt je Block frisch über die Brücke (`LaufenderTransport::vorBlock`, `MarkierungTestMain.cpp:146-164`).
- Einen Weg, auf dem der Auftrag bei `erlaubt` erst im zweiten Block beginnt, habe ich im Code nicht gefunden. Ein anderes `fs` oder ein Oversize-Startblock verhindern den Neubeginn ganz; beides fiele an `begins == 1`. Im Produkt ist der erste erlaubte Block der erste nach dem Echtzeitbeleg (`echtzeitOk`, `Hostbruecke.cpp:88`). Das deckt die Zusage „erlaubt“.
- Eine künftige Verzögerung um einen oder mehrere Blöcke bemerkte der Test nicht. Die Zusage gilt heute nach Lesart; sie ist nicht gemessen.

**3. Zusagenlage.**
- M-130 (`NAK-312.md:9154`), Zusage wörtlich: „**Der Auftrag bleibt über `releaseResources()` eingereicht (`markierungZielGesetztFuerTest()` wahr nach den 5 Blöcken), und nach dem nächsten `prepareToPlay` beginnt er beim ersten erlaubten Block neu: genau ein neues `begin`, kein zweites `end` zum alten Intervall, `markierungHoerbar()` wird wahr**“. Test-Spalte: „A3 bestehend `312/M-64 (a)` …, um die zwei Prüfungen ergänzt“.
- Prüfauftrag, Prüffrage (a): „Trägt jede Zeile M-110 bis M-131 … einen Test am ZIEL, der ihre Zusage misst, und fällt der im Diff erkennbare Rotbeweis an der Zusage statt an einem Nebeneffekt?“ Der Satz „Je Matrixzeile fällt der Rotbeweis an der Zeile, die die Zusage trägt, nicht an einem Nebeneffekt“ steht wörtlich in Aufträgen anderer Tickets (etwa `NAK-283.md:109`), nicht in diesem Prüfauftrag; hier trägt ihn Prüffrage (a).
- U56 (`…-u56.md:26-28`): „der Auftrag bleibt bestehen, und die Markierung beginnt beim Weiterlaufen von vorn.“ „Erster erlaubter Block“ steht dort nicht; die Präzisierung kommt aus M-130 und dem Kommentar `HoerMarkierung.h:336`.
- Die A3-Behauptungszeile (`tools/beweise.ps1:509`) sagt nur „releaseResources laesst den Auftrag bestehen, nach prepareToPlay beginnt er mit genau einem neuen begin.“ Das ist nicht mehr, als der Test misst; Prüffrage (d) hält hier.
- Kontext außerhalb des Prüfbereichs: Das ältere `312/M-63` (Weg `reset`, `:2086-2093`, Zusage „beim naechsten erlaubten Block NEU“) prüft ebenso nur `nach.hoerbar > 0`.

**4. Urteil: BESTÄTIGT.** Test, Summenprüfung, Zusage und Rotbeweis stimmen wie beschrieben. Das Produkt hält den Zeitpunkt nach Lesart; ein Produktfehler liegt nicht vor.

**Klassenvorschlag: DEFEKT (Prüfdeckung)** gegen M-130 (Zusageteil „beim ersten erlaubten Block“) in Verbindung mit Prüffrage (a). Der zugewiesene Test misst diesen Teil nicht, und sein Prüftext behauptet ihn.

## D-3 — B15-Behauptungszeile „die bisherigen Bilder bleiben bytegleich“

**1. Quelle.** `tools/beweise.ps1:752`, B15, Nachsatz wörtlich (in 7b hinzugefügt, `git diff --word-diff e1bd9c6d 19bb1803`):

> Seit NAK-312 Etappe 7b (U51) dazu die Annahmegrenze am echten Pfad (abonnierte Sitzung, echte Snapshots): bei 0, 1, 19 und 20 Quellen ist jede angenommen, gezeichnet und per Klick erreichbar; bei 21, 32 und 64 Quellen sind 20 angenommen, jede davon gezeichnet und erreichbar, und das vorhandene Diagnosefeld meldet die volle Liste mit der Zahl der nicht angenommenen, allein oder als Zusatz hinter einer anstehenden Diagnose; die bisherigen Bilder bleiben bytegleich.

- `schreibeShot` (`eq-copilot/plugin/tests/ShotTestMain.cpp:120-140`) löscht die Zieldatei (`:132`), schreibt das PNG und gibt `ok && bild.getWidth() == 760 && bild.getHeight() == 430` zurück (`:139`). `sonde012Suite` zählt nur diese Rückgaben (`:274-279`).
- Pixelvergleiche in B15 am ZIEL:
  - Diagnosefeld gegen eine unabhängig gezeichnete Referenz (`diagnosefeldReferenz`, `:1418-1429`; `abweichendePixel`, `:1431-1439`; Rechteck `{324, 49, 422, 28}`, `:1311`) in M-123 (`:1506`), M-124 (`:1533`) und M-129 (b), (c), (d) (`:1597`, `:1608`, `:1621`).
  - M-129 (a): Spalte Sources `{0, 0, 310, 430}` zwischen dem 20er- und dem 21er-Bild desselben Laufs (`:1578`).
  - `feldGezeichnet`: ein Pixel (`:1443-1446`).
- Einen Vergleich der bisherigen Bilder gegen eine Referenz gibt es nicht: keine SHA-256-Liste, keine committeten Referenzbilder (`git ls-files` zählt 0 Treffer für `sonde012*.png`), kein Vergleich zweier Läufe. Außer `ShotTestMain.cpp` und `tools/beweise.ps1` nennt keine Datei außerhalb von `docs/**` den Bildsatz. Der Runner wertet Plugin-Beine nur am Exitcode aus (`tools/beweise.ps1:1205` ff.); gehasht wird nur die Binary für die Frische (`:1088`).
- Die Aussage des Prüfers stimmt. Sein Beispiel trägt: Der Titel „Gen / Surface 1“ (`PluginEditor.cpp:1395-1399`, y 14 bis 46) liegt außerhalb des Diagnosefelds (ab y 49) und außerhalb der Spalte (x < 310).

**2. Ablauf.** Eine Änderung eines bisherigen Bildes, im Fixture-Sichtsatz (`:161-272`) oder außerhalb der verglichenen Rechtecke, lässt B15 mit Exit 0 enden; der Kanon beglaubigt die Zeile trotzdem. Beleg: Die B15-Zeile des Kanonmanifests (`docs/beweise/NAK-312.md:11750`) trägt den Nachsatz mit „[OK] Exit 0“. Gemessen wurde die Bytegleichheit einmal vom Bauer, nicht im Kanonlauf:
- Rotbeweistabelle M-129: „die 31 Bilder bytegleich zum Basisstand (SHA-256)“ (`:11146`)
- §54.5 (`:11174`), §54.8 (`:11281`), §54.14 (`:11503`), §54.19 (`:11649`)

Nachgemessen habe ich diese Bytegleichheit nicht.

**3. Zusagenlage.**
- Prüfauftrag, Defektkriterien (Zeile 15): „… die bestehenden Bilder des B15-Sichtsatzes sind bytegleich; die Behauptungszeilen sagen nicht mehr, als der jeweilige Test misst.“
- Prüffrage (d): „Sagen die geänderten Behauptungszeilen in tools/beweise.ps1 (B13, A23, B15, B14, A3, B2) nicht mehr, als der Test misst?“
- M-129 (`NAK-312.md:9119`), Zusage: „… Die bestehenden Bilder des B15-Sichtsatzes (0, 1, 16 Quellen) bleiben bytegleich“, Test-Spalte: „B15 bestehend (Sichtsatz)“. Die Matrix weist diesen Teil damit einem Bein zu, das ihn nicht misst.
- Kontext: Der Rotbeweis von M-131 (`:9160`, „→ der bestehende B15-Sichtsatz weicht ab … → rot“) setzt denselben Vergleich voraus. Gefallen ist er laut §54.19 an M-123 und M-124.

**4. Urteil: BESTÄTIGT.** Zeile, Nachsatz, `schreibeShot`, Umfang der Pixelvergleiche und Einmaligkeit des Bauervergleichs stimmen wie beschrieben.

**Klassenvorschlag: DEFEKT** gegen den Prüfauftragssatz „die Behauptungszeilen sagen nicht mehr, als der jeweilige Test misst“ (Prüffrage (d)). Mitbetroffen ist die Test-Zuweisung des Bytegleich-Teils von M-129. Schließen lässt sich das auf zwei Wegen: den Nachsatz streichen oder einen Referenzvergleich im Bein anschließen. Wird nur gestrichen, bleibt der Bytegleich-Teil von M-129 im Kanon ungemessen.

## Übersicht

| Befund | Urteil | Klassenvorschlag | tragender Satz |
|---|---|---|---|
| D-1 `SourcesModel.cpp:461-477` | BESTÄTIGT (Ergänzungen: Nonce-Bedingung für den lokalen Weg, ACK-Nachführung `Ipc.cpp:1553`, Zeilenzahl ohne Auswahl, weitere Sitzungsschnitte) | DEFEKT; falls die Invariante nicht trägt: LÜCKE mit Regel | `CLAUDE.md:44-45` „meldet ehrlich einen Zustand“ (Defektkriterium „keine toten UI-Elemente“); M-125, M-126, M-128, Gate und R-312-25 bis R-312-31 schweigen zum Zähler beim Nachrücken |
| D-2 `MarkierungTestMain.cpp:2298-2300` | BESTÄTIGT (Produkt hält den Zeitpunkt nach Lesart; Rotbeweis fällt an „Auftrag bleibt“) | DEFEKT (Prüfdeckung) | M-130 „beginnt er beim ersten erlaubten Block neu“ mit Prüffrage (a) „einen Test am ZIEL, der ihre Zusage misst“ |
| D-3 `tools/beweise.ps1:752` | BESTÄTIGT | DEFEKT | Prüfauftrag „die Behauptungszeilen sagen nicht mehr, als der jeweilige Test misst“ (Prüffrage (d)); M-129 Test-Spalte „B15 bestehend (Sichtsatz)“ |

FERTIG Validierung Etappe 7b Teil 1, 3 bestätigt, 0 präzisiert, 0 widerlegt
