# Nakama — zweiter Codeaudit: Übergabe an Claude

**Datum: 12.09.2026 · Urteil: Nacharbeit erforderlich · Keine Freigabe zur weiteren Featureimplementierung aus diesem Audit.**

Diese Datei ist die eigenständig lesbare Übergabe des zweiten Audits. Sie enthält die Ergebnisse der erneuten Prüfung nach NAK-246 und der anschließend ausgeweiteten Prüfung. Die verlinkten Rohbelege dienen zur Nachvollziehbarkeit; die Befunde und ihre Grenzen sind hier vollständig beschrieben.

## 1. Auftrag, Stand und aktuelle Arbeitsgrenze

Der User hat am 12.09.2026 beauftragt, den abgearbeiteten Audit mit maximaler Tiefe erneut durchzuführen und anschließend Bereiche zu vertiefen, die bisher zu kurz kamen. Sein Wortlaut zur Reihenfolge: „der implement wird erst weitergeführt ,wenn der aktuelle stand in perfektem shape ist“. Gewünscht ist eine neue separate Markdown-Datei zur Übergabe an Claude.

**Spätere, maßgebliche Einschränkung:** Nach wiederholten automatischen Blockaden einzelner Agentenläufe bat der User, die auslösenden Handlungen zu unterlassen. Daraufhin wurden weitere Agentenexperimente, Sicherheitsprüfungen, Fuzzing sowie gezielte Fehler- und Grenzfallinjektionen eingestellt. Auch der noch laufende kanonische Beweislauf wurde abgebrochen. Danach erfolgten ausschließlich Quellen- und Ergebnislektüre, Archivierung vorhandener Belege und Erstellung dieser Übergabe. Die hier enthaltenen früheren Reproduktionen sind historische Ergebnisse dieser Session und kein Auftrag, die gestoppten Prüfformen automatisch wieder aufzunehmen. Es wurde keine Blockade umgangen.

Ausgangsrevision war **ea6ddd57c6658429ccb283f5d207e26341606267**. Die vorgeschriebene reine Planrechnung wurde als **91bb5fa34fa9c949611656c787c343cf7ffee0b7** separat committed. Auf diesem Produktstand entstanden die nachfolgenden Belege. Beim abschließenden Quellenabgleich lag HEAD auf **3225b1e7e45a4b44dad27a4452cb3379688b55dd**. Die zwischenzeitlichen fremden Commits änderten Plan-, Register-, Beweis- und Orchestrierungsdokumente, keinen Produktcode. Aussagen aus anderen Sessions über den nächsten Planschritt ersetzen weder diesen Audit noch seine Nachweise.

**Änderungsumfang dieses Audits:** Planstandrechnung und neue Auditdokumente samt Belegen. Kein Produktfix, keine Installation, kein Featurebau. Produktquellen und fremde Änderungen blieben unverändert. Das Quellinventar umfasst 327 ausgewählte eigene Quell-, Vertrags-, Test- und Werkzeugdateien mit 221.045 Zeilen. Das ist eine Bestandsaufnahme, keine Behauptung, jede Zeile vollständig geprüft zu haben.

## 2. Ergebnis und Priorisierung

Es liegen **14 Befundgruppen** mit konkreten Gegenbeispielen vor. Zwei davon sind bereits registrierte Punkte mit verbessertem Beweisstatus: **NAK-253** und **NAK-163**. Dazu kommt eine belegte Lücke in der behaupteten EBU-Testabdeckung. Die Beweistiefe reicht vom echten VST3-Modul im kleinen Testhost bis zum unveränderten Einzelmodul; sie wird ausdrücklich unterschieden.

P2 bezeichnet relevante Korrektheits- oder Nachweisprobleme im vorhandenen Funktionsumfang. P3 bezeichnet hier die zwei seltenen numerischen Grenzbereiche; ihre Gegenbeispiele sind belegt, normale Musikpegel beziehungsweise gewöhnliche Revisionsstände lösen sie nicht aus. Die Audit-IDs sind keine neuen NAK-Ticketnummern.

| ID | Priorität | Befund | Nachweis / Registerbezug |
|---|---|---|---|
| F01 | P2 | Sources-Modell übernimmt nach erfolgreichem Unbind wieder einen älteren Mitgliederstand | Echter Prozessor und Worker; fünf gleiche Ergebnisse, serieller Kontrollfall korrekt |
| F02 | P2 | Ältere Hypothesenrechnung überschreibt bereits veröffentlichtes neueres Ergebnis | Original-Broker, Projektion und Snapshot beobachtet; **NAK-253**, bisher Verdacht |
| F03 | P2 | Koaleszierung vertauscht die Reihenfolge von Vollsnapshot und Rücknahme | Original-Queue reproduziert; Verbraucherwirkung quellbelegt; Nachbarschaft zu NAK-269 |
| F04 | P2 | VST3 akzeptiert vier Kanäle, verarbeitet aber nur die ersten zwei | Tatsächliches VST3-Modul und gültiger Vierkanalbus |
| F05 | P2 | Host-Reset lässt Filterzustand hörbar weiterlaufen | Tatsächliches VST3-Modul; danach bei Stille noch Peak 0,0192089 |
| F06 | P2 | Aktivitätsgate der Spektralanalyse verwechselt Leistungsdichte mit Energie | Original-FeatureEngine; gleiches Signal abhängig von Samplerate unterschiedlich bewertet |
| F07 | P2 | Vorab markierte Passage übernimmt True Peak aus vorherigem Material | Original-FeatureEngine; 60 dB Unterschied zur isolierten Passage |
| F08 | P2 | Headroom-Verteilung beschreibt nur die letzten 64 Rahmen statt die ganze Passage | Original-FeatureEngine; 14-Sekunden-Passage, P95 um 60 dB abweichend |
| F09 | P2 | Verworfener DSP-Tap hinterlässt keine Lücke auf der Analysezeitachse | Original-DSP und Queue mit nachgebildeter Sonde-Abzweigung; kein vollständiger Prozessortest |
| F10 | P2 | Loudness wird wieder als gültig ausgegeben, obwohl ihr Zeitfenster ersetzte Samples enthält | Original-FeatureEngine; alte Ersetzung lebt länger als Ungültigkeitszähler |
| F11 | P3 | Revisionsgrenze verletzt Ablehnungsatomarität und eigenen State-Roundtrip | Echter Prozessor; akzeptierter maximaler int64-State |
| F12 | P3 | Endliche double-Werte werden beim float-Ausgang ungezählt zu Inf | Original-DSP und Hörmarkierung; extreme, endliche Eingangspegel |
| F13 | P2 | Fehlende RSS-Messwerte können den Speichertest grün bestehen lassen | Aktuelles Soak-Orakel mit historischem Kontrollbericht; kein neuer Soak |
| F14 | P2 | Rollback einer Invalidierung entfernt den Ausschluss eines anderen Belegs | Original-Broker und SQLite; **NAK-163**, bereits registriert |
| N01 | Nachweislücke | Als EBU-Testset ausgewiesener True-Peak-Nachweis umfasst nur Fälle 15–19 | Quell- und Primärquellenabgleich; Fälle 20–23 nicht nachgewiesen |

**Entscheidende Folgerung:** Die Nacharbeit des ersten Audits hat die dort beschriebenen Ursachen erkennbar verändert. Daraus folgt keine Freigabe des gesamten Bestands. Insbesondere Publikationsreihenfolge, Host-Lebenszyklus und die Bedeutung der Analysezahlen brauchen weitere Korrekturarbeit. Ein Zustand „perfekt“ ist durch die vorliegenden Belege ausdrücklich nicht erreicht.

## 3. Wiederprüfung des Audits vom 10.09.2026

Verglichen wurden der damalige Bericht, die aktuellen Produktpfade und die ergänzten Tests. Der frühere Abschluss von NAK-246 mit 64/64 ist **historische Evidenz einer anderen Runde**. Der eigene vollständige Lauf dieser Session wurde nicht abgeschlossen. „Quellseitig behoben“ in der folgenden Tabelle ist deshalb kein frischer vollständiger PASS.

| Alter Punkt | Ergebnis der aktuellen Quellenprüfung | Grenze / Zusammenhang |
|---|---|---|
| D1 — Markierungsring überschreibt gelesenen Slot | Besitz der Slots ist jetzt explizit; Producer und Consumer verwenden getrennte Besitzindizes. Die ergänzten Tests prüfen konkurrierenden Slotbesitz. | Alter Mechanismus in HoerMarkierung.h geändert; kein frischer Sanitizer-/Gesamtlauf |
| D2 — Callback überlebt Prozessor | Gemeinsame Callback-Schleuse mit Eintritt/Austritt und Warten vor Besitzerzerstörung ist in den v3-Callbacks angeschlossen. | Alte Lebensdauerursache quellseitig behandelt; kein neuer Shutdown-Stresstest abgeschlossen |
| D3 — ACK-Persistenz hängt am Editor | Worker und Speicherpfad übernehmen bestätigte Befehle unabhängig vom Editor. | F01 betrifft die spätere Publikation zweier gültiger Drains, nicht die frühere fehlende Übernahme |
| D4 — Ablehnung und spätere Ausführung | Die Unterscheidung von angenommen, vorgemerkt und hart abgewiesen ist angeschlossen; harte Ablehnung behält keinen später auszuführenden Befehl. | Kein frischer kompletter P0-E2E-Lauf |
| D5 — alter Snapshot ersetzt neueren | Die Queue führt ein Hochwasser pro Objektschlüssel und verwirft ältere Marken. | Acht bestehende Original-Queue-Tests frisch bestanden; F02 betrifft die Berechnung vor der Queue, F03 die Reihenfolge zwischen Schlüsseln |
| D6 — Reload lässt fremden Sitzungszustand stehen | Erweiterte Sitzungslisten werden im gemeinsamen Reset geleert. | Kein neues vollständiges Reload-/Hostzertifikat |
| D7 — Stereo erreicht 400 ms schon nach 200 ms | Vergleichspegel übernimmt jetzt den vollständigen Hostblock und zählt Frames kanalunabhängig. Die aufrufende Audiokette und Zeitgrenztests passen dazu. | Alter Einzelkanal-Aufruf beseitigt; hier kein neuer eigenständiger 400-ms-Gesamtlauf |
| D8 — Konvertierung vor Bereichsprüfung | Zonenwert wird als double auf den erlaubten Bereich geprüft, bevor die Ganzzahlkonvertierung erfolgt. | Quellen und ergänzte State-Tests geprüft; kein neuer UBSan-Lauf |
| D9 — Rücknahme teilt Snapshot-Schlüssel | Getrennte Objektschlüssel sind vorhanden. | Der frühere Verdrängungsmechanismus ist behoben; F03 ist die verbleibende Reihenfolge über beide Schlüssel |
| D10 — Slotbelegung ersetzt Vier-Bank-Rechenbeweis | B6 erzeugt jetzt tatsächlich verschiedene Bankprogramme auf beiden Pfaden, zählt vier reale Bankläufe und misst die DSP-Verarbeitung mit Mittelwert, p99 und Maximum. | Aussagekräftigerer Test vorhanden; in dieser Runde keine frischen abgeschlossenen B6-/Host-Performancezahlen |

Einstiegspunkte: [alter Audit](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-10-code-review/AUDIT.md), [NAK-246](C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-246.md), [Hörmarkierung](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/HoerMarkierung.h:345), [Vergleichspegel-Aufruf](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/PluginProcessor.cpp:903), [Vier-Bank-Test](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4081).

## 4. Befunde im Detail

### F01 — Mitgliederpublikation kann innerhalb derselben Reload-Generation zurückspringen

**Auslöser:** B ist bereits bestätigt. Ein Join von A wird bestätigt und vom echten Worker in den Projektstate übernommen. Nach dem Ziehen der Mitgliederkopie, aber vor deren Modellpublikation, übernimmt der Message-Thread einen ebenfalls bestätigten Unbind von B. Der Projektstate und die zunächst veröffentlichte Sicht enthalten jetzt nur A. Danach publiziert der Worker seine ältere Kopie mit B und A.

**Beobachtet:** In fünf kontrollierten Läufen blieb der gespeicherte State korrekt bei genau A, während das Modell B wieder als bestätigt zeigte: 13/14 Erwartungen erfüllt, Exit 1. Der seriell ausgeführte identische Handgriffablauf erfüllte 14/14 Erwartungen, Exit 0. Der gespeicherte State hatte 333 Bytes und ließ sich mit genau einem Mitglied laden. Das ist eine belegte Abweichung zwischen Persistenz und Modell, kein behaupteter Verlust des gespeicherten Unbinds.

**Ursache:** Der Drain-Riegel schützt Übernahme und Anwendung, endet aber vor der Modellpublikation. Die Kopie wird unter bindungMutex gezogen, anschließend ohne diese Sperre publiziert. SourcesModel prüft nur reloadGeneration. Beide Kopien gehören derselben Generation an; eine Reihenfolge der Mitgliederstände wird nicht geprüft. Ein leerer weiterer Drain korrigiert das Modell nicht. Der Quellenkommentar, die aktuelle Kopie könne nie älter als die vorige Nachführung werden, berücksichtigt diese Überholung nicht.

**Orte:** [Ipc.cpp:1444](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/prozessor/Ipc.cpp:1444), dort 1461–1484 und 1498–1534; [SourcesModel.cpp:351](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/SourcesModel.cpp:351), insbesondere 368–372. Der Harness verwendet die öffentlichen Handgriffe, vorhandene ACK-Testzugänge und den bestehenden Publikationshaken. Keine Produktionslogik wurde durch eine Kopie ersetzt.

**Korrekturziel:** Neben der Reload-Generation muss auch die Reihenfolge der angewandten Mitgliederstände bis in die atomare Modellübernahme gelten. Eine ältere Publikation darf keine neuere ersetzen. Host-Dirty-Aufrufe dabei weiterhin außerhalb eigener Prozessorsperren halten. Erfolgreiches Join/Unbind muss nach Abschluss in State und Modell denselben Bestand ergeben, auch wenn die Publikationsreihenfolge vertauscht ist.

**Abgrenzung:** Kein Reload im Gegenbeispiel. Daher keine Dublette der dokumentierten Reload-Rekonziliation NAK-278 oder der dortigen Härtungsreste NAK-279.

**Belege:** [Harness](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/state/repro.cpp), [Lauf 1](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/state/run-1.txt), weitere Läufe run-2.txt bis run-5.txt im selben Ordner, [serielle Kontrolle](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/state/run-serial.txt), [Build-Provenienz](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/state/input-provenance.json).

### F02 — NAK-253 ist vom Verdacht zum beobachteten Ergebnisrücksprung geworden

**Auslöser:** Rechnung A sammelt ältere, weiterhin gültige Evidenz. Am bestehenden Rechenhaken kommen neue gültige Belege hinzu; Rechnung B wird vollständig veröffentlicht. Anschließend setzt A seine ältere Rechnung ein. Die Intentgeneration bleibt gleich, keine benutzte alte Evidenz wurde invalidiert.

**Beobachtet:** B trug 13 Evidence-IDs und einen Beobachtungswert von rund −3,30 dB. Der endgültige Bestand wurde durch A auf 12 IDs und rund −8,30 dB zurückgesetzt. Der neue Quellbeleg war weiterhin in der Historie vorhanden. Auch der letzte Session-Snapshot-Payload und die SQLite-Projektion enthielten das ältere Ergebnis als ready_to_send. Der Vergleich schlug fehl, Cargo-Exit 101.

**Ursache:** [hypothese_verdrahtung.rs:72](C:/Users/phili/Projekte/Nakama/broker/src/coordinator/hypothese_verdrahtung.rs:72) sammelt unter Sperre und rechnet außerhalb. [befunde_eintragen:443](C:/Users/phili/Projekte/Nakama/broker/src/coordinator/hypothese_verdrahtung.rs:443) prüft vor dem Ersetzen nur den Riegel aus [961–996](C:/Users/phili/Projekte/Nakama/broker/src/coordinator/hypothese_verdrahtung.rs:961): Intentgeneration und weitere Gültigkeit der verwendeten IDs. Er erkennt weder neue Evidenz noch ein bereits publiziertes Ergebnis eines jüngeren Eingangsstands.

**Korrekturziel:** Der Eingangsstand einer Rechnung muss bis zu Ergebnisübernahme, Persistenz und Publikation identifizierbar bleiben. Veraltete Rechnungen dürfen einen jüngeren vollständigen Stand nicht verdrängen. Ein nachfolgender Neuberechnungstakt kann den Fehler eventuell heilen; er verhindert den beobachteten falschen aktuellen Stand nicht und darf nicht als Ordnungsgarantie dienen.

**Beweisgrenze:** Original-Broker und Store mit deterministisch verschachtelten Produktionsaufrufen am vorhandenen Testhaken. Kein freilaufender Lasttest und keine reale Pipe-Zustellung. Der Snapshot-Payload wurde am Senderzugang aufgezeichnet.

**Register:** Bestehenden **NAK-253** aktualisieren, nicht als neuen unabhängigen NAK-Punkt duplizieren. Die alte Formulierung „ohne Laufzeit unentscheidbar“ ist durch diesen Beleg überholt.

**Belege:** [broker_repros.rs:382](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/broker/tests/broker_repros.rs:382), [v2.log](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/broker/v2.log).

### F03 — Koaleszierung erhält nicht die Reihenfolge zwischen Snapshot und Invalidierung

**Auslöser:** Die Writerqueue enthält einen session_snapshot mit Marke 1 und danach evidence_invalidate mit Marke 2. Ein neuer session_snapshot mit Marke 3 ersetzt den ersten Eintrag an dessen bisheriger Position.

**Beobachtet:** Die unveränderte Produktionsqueue gibt **Snapshot 3, danach Invalidierung 2** aus. Erwartet wurde die Reihenfolge 2, 3. Beide tatsächlich entnommenen Einträge erhalten erfolgreiche Schreibbestätigungen; nur der supersedierte alte Snapshot wird korrekt als ersetzt zurückgemeldet. Der neue Reihenfolgefall schlägt fehl, die acht bereits vorhandenen Queue-Tests bestehen.

**Ursache und Wirkung:** [queues.rs:223](C:/Users/phili/Projekte/Nakama/broker/src/transport/server_v3/queues.rs:223) ersetzt im vorhandenen Deque-Slot. Das Hochwasser ist pro Schlüssel korrekt, bildet jedoch keine Reihenfolge über abhängige Objektschlüssel ab. Auf der C++-Seite setzt eine angenommene Rücknahme sämtliche vorhandenen Messachsen auf invalid und Befunde auf stale, ohne deren Zusammenhang mit einem bereits neueren Vollstand prüfen zu können: [SourcesModel.cpp:1453](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/SourcesModel.cpp:1453). So kann ein jüngerer Vollstand anschließend durch eine ältere Rücknahme wieder ungültig erscheinen.

**Beweisgrenze:** Die Reihenfolge ist direkt reproduziert. Die Queue-Payloads des Harness sind beschriftete Bytes, keine gültigen vollständigen Wire-Nachrichten. Die anschließende Modellwirkung ist aus dem tatsächlichen Consumer abgeleitet, nicht als durchgängiges Broker-zu-Plugin-E2E ausgeführt. Dieser Unterschied ist beim Schließen des Punkts beizubehalten.

**Korrekturziel:** Ersetzen und Zustellen abhängiger Snapshots/Rücknahmen muss ihre fachliche Reihenfolge bewahren. Die vorhandenen Zusagen zu P0-Priorität, Queuegrenze, Schreibantworten und schlüsselbezogenem Hochwasser bleiben dabei gültig. Eine bloß globale Zahl auf der Consumerseite oder ein pauschales Wegwerfen aller älteren Rücknahmen ist ohne vollständigen Vertragsabgleich kein hinreichender Fix.

**Registerbezug:** D5/D9 sind in ihrem bisherigen Umfang tatsächlich korrigiert. **NAK-269** beschreibt einen fehlenden Consumer-Altersriegel bisher als wirkungslos, weil der Broker die Ordnung vollständig sichere. Dieses Gegenbeispiel widerlegt die allgemeine Ordnungsannahme bereits in der heutigen Queue. Es braucht dazu weder einen zweiten Sender noch einen anderen Transport.

**Belege:** [queue_repro.rs](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/broker/tests/queue_repro.rs), [Fehlerlauf](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/broker/queue.log), [acht bestehende Kontrollfälle](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/broker/queue-controls.log).

### F04 — Angenommener Vierkanalbus wird nur teilweise bearbeitet

**Auslöser:** Der Host richtet für Nakama Probeeq einen symmetrischen k40Music-Vierkanalbus ein. EQ ist eingeschaltet, Output-Trim steht auf +6 dB. Auf jedem Kanal liegt derselbe Wert 0,1 an.

**Beobachtet am tatsächlichen VST3-Modul:** Erzeugen, Initialisieren, Busannahme, Busaktivierung, Setup mit 48 kHz/512 Samples und Processing melden Erfolg. Der Inputbus meldet vier Kanäle. Die vier Ausgangswerte sind **0,199526235461 / 0,199526235461 / 0,10000000149 / 0,10000000149**. Die letzten zwei Kanäle bleiben also unbearbeitet, obwohl das Plugin den Bus angenommen hat.

**Ursache:** [SondeProcessor.cpp:265](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:265) akzeptiert jede gleiche, nicht deaktivierte Ein-/Ausgangsbelegung. [processBlock:282](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:282) und der DSP-Aufruf begrenzen die Verarbeitung auf zwei Kanäle. Die frühere reine Passthrough-Begründung deckt den inzwischen aktiven EQ nicht mehr.

**Korrekturziel:** Der angekündigte Hostvertrag muss der tatsächlich implementierten Kanalverarbeitung entsprechen. Nicht unterstützte Buslayouts sind verlässlich abzulehnen; daraus soll keine ungefragte Mehrkanalerweiterung entstehen. Jede angenommene Konfiguration muss auf allen zugehörigen Kanälen die zugesagte Bearbeitung ausführen.

**Beweisgrenze:** Echtes VST3-Modul mit kleinem lokalem Testhost, keine FL-Studio-Sitzung. Der entsprechende direkte Prozessorfall zeigt dieselben Werte. Das Ergebnis behauptet keine Installation oder DAW-Zertifizierung.

**Belege:** [VST3-Harness](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/host/vst3_contract.cpp), [VST3-Ausgabe](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/host/vst3_full.log), [Prozessor-Harness](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/host/host_audit.cpp).

### F05 — Reset des Hosts leert den DSP-Zustand nicht

**Auslöser:** Ein gültiger Bell-Filter bei 1 kHz, +12 dB, Q 10 erhält nach dem Einschwingen einen Impuls am Ende des Blocks. Der Host ruft setProcessing(false) und anschließend setProcessing(true) auf; der nächste Audioblock enthält nur Nullen.

**Beobachtet:** Trotz Reset beträgt der nächste Peak **0,019208887592**, ungefähr −34,33 dBFS. Das Modul meldet gleichzeitig null Tail-Samples. Beide Processing-Aufrufe melden Erfolg. Im direkten Prozessortest ergeben releaseResources beziehungsweise erneutes prepareToPlay anschließend exakt null.

**Ursache:** SondeProcessor überschreibt reset nicht. Die JUCE-Basisimplementierung ist leer. Der eingebundene VST3-Wrapper ruft in setProcessing(false) ausschließlich den Reset des Plugins auf. Der normale Hostweg kann daher ohne erneutes release/prepare den alten Filterzustand weiterverwenden. JUCE beschreibt reset ausdrücklich als Hostanweisung zum Beenden weiterlaufender Stimmen beziehungsweise Tails.

**Orte:** [SondeProcessor.h:135](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.h:135), [releaseResources:251](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:251), eingebundenes JUCE: juce_AudioProcessor.h:935–941, juce_AudioProcessor.cpp:589 und juce_audio_plugin_client_VST3.cpp:3612–3617 unter eq-copilot/build/_deps/juce-src/modules/.

**Korrekturziel:** Ein Host-Reset muss die für den nächsten Block relevanten Audiohistorien konsistent beenden, ohne den bestätigten Parametersatz zu verlieren. Reset, release und prepare als zusammengehörigen Lebenszyklus behandeln. Die Tail-Angabe und die reale Verarbeitung müssen zusammenpassen; nur die Tail-Zahl zu ändern beseitigt den beobachteten Resetfehler nicht.

**Belege:** [VST3-Ausgabe](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/host/vst3_full.log), [direkter Prozessorlauf mit Kontrollen](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/host/run.log). Beide Fälle verwenden unveränderte Produktimplementierungen.

### F06 — Das Spektralgate verwendet eine Leistungsdichte wie eine Energie

**Auslöser und Beobachtung:** Drei Sekunden identischer 1-kHz-Stereosinuston mit gemessen −55 dBFS RMS. Die Zeitbereichsaktivität beträgt bei allen untersuchten Raten 1. Die Spektralabdeckung beträgt bei 44,1 / 48 / 96 / 192 kHz dagegen 0. Im 8-kHz-Komponenten-Kontrollfall beträgt sie 1. Der Evidenzframe wird trotzdem als frisch ausgegeben. Die 8-kHz-Kontrolle ist keine Aussage über offiziell unterstützte Pluginraten.

**Ursache:** [Spektrum.h:355](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:355) normiert das FFT-Leistungsspektrum mit 2/(fs · Fensterenergie) zu einer Dichte. summeBereich addiert die Dichten ohne Binbreite Δf = fs/N. Diese Summe wird bei 360–362 gegen dasselbe −60-dB-Gate gehalten, das [Lautheit.h:70](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:70) mit wirklicher mittlerer Sampleenergie verwendet. So hängt die effektive Schwelle von Rate und FFT-Länge ab. Das frühe return überspringt außerdem die anschließende Bandakkumulation.

**Wirkung:** Ein normales, oberhalb des definierten Aktivitätsgates liegendes Signal kann als aktiv gelten und gleichzeitig keine aktive spektrale Abdeckung erhalten. Daraus entstehende Konfidenz- und Bandmessungen sind nicht über Sampleraten vergleichbar.

**Korrekturziel:** Dichte, integrierte Energie und dB-Referenz am Gate sauber auseinanderhalten. Die PSD für weitere Fachrechnungen nicht pauschal umskalieren, ohne ihre Verbraucher zu prüfen. Gleiche Signalenergie oberhalb beziehungsweise unterhalb derselben fachlichen Schwelle muss unabhängig von Samplerate und Stufengröße konsistent bewertet werden.

**Beweisgrenze:** Direkter Lauf der unveränderten FeatureEngine mit gültigen StampedBlocks, kein vollständiges Plugin-/Wire-E2E. Der Harness gibt Messwerte aus und endet regulär; die falsche Abdeckung ist aus Messwerten und Einheitenrechnung belegt, nicht durch einen künstlich roten Prozess-Exit.

**Belege:** [models.cpp](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models.cpp), Zeilen mit ACTIVITY in [models-results.txt](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models-results.txt).

### F07 — Reset beim Binden einer zukünftigen Passage kommt zu früh

**Auslöser:** Vor dem Abspielen wird ein gültiges Passagenfenster von Sample 48.000 bis 96.000 bei 48 kHz gesetzt. Vor der Passage läuft eine Sekunde 12-kHz-Ton mit Amplitude 1; innerhalb der Passage eine Sekunde desselben Tons mit Amplitude 0,001.

**Beobachtet:** Der aktuelle Rahmen meldet rund −59,84262 dBTP. Das Passagenmaximum meldet aber **+0,157058 dBTP**. Die isoliert ab Beginn verarbeitete leise Passage meldet rund −59,84262 dBTP, also 60 dB weniger. Das Passagenfenster und die Epochennummer sind gültig.

**Ursache:** [FeatureEngine.h:589](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/FeatureEngine.h:589) setzt den True-Peak-Interpolator beim Setzen des Fensters zurück. Beginnt das Fenster erst später, füllt das Material davor den Filter wieder. [Spektrum.h:192](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:192) taktet den Interpolator auch außerhalb der Passage. Am tatsächlichen Start wird dessen verzögerte Vorgeschichte ohne passende Startgrenze in passagenTruePeakRahmen übernommen. Das bereits korrigierte Auslaufen am Passagenende löst diese Startseite nicht.

**Korrekturziel:** Passagenwerte müssen sich auf das definierte Materialintervall beziehen, unabhängig davon, wann die Bindung des Fensters erfolgt. Der eigentliche Anfang, das Ende und die verzögerte Filterantwort brauchen eine gemeinsame Zuordnung. Dabei die kontinuierlichen Live-Metriken nicht durch einen Reset der falschen Historie verfälschen.

**Beweisgrenze:** Akzeptierter Engine-Aufruf mit Vorabfenster; keine vollständige UI-/Hostkette zum Setzen dieses Fensters ausgeführt. Der Gegensatz zu SONDE-013 M-03/M-25 und zum eigenen Startgrenzenkommentar ist im Komponentenpfad direkt belegt.

**Belege:** FUTURE_PASSAGE und ISOLATED_REFERENCE in [models-results.txt](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models-results.txt), [Harness](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models.cpp).

### F08 — Headroom-P95 verliert den Anfang einer längeren Passage

**Auslöser:** Eine gültige Passage dauert 14 Sekunden. Die erste Hälfte enthält einen 1-kHz-Ton mit Amplitude 0,5, die zweite Hälfte denselben Ton mit Amplitude 0,0005. Das sind zwei normale Pegel mit 60 dB Abstand.

**Beobachtet:** Das vollständige Passagenmaximum bleibt korrekt bei etwa −6,0206 dBTP. P10, P50 und P95 stehen am Ende sämtlich bei etwa **−66,0206 dBTP**, headroomFenster ist 64. Das P95 über sämtliche Rahmen dieser Passage müsste bei rund −6,0206 dBTP liegen.

**Ursache:** [FeatureEngine.h:118](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/FeatureEngine.h:118) begrenzt den gemeinsamen Verteilungsring auf 64 Werte; schiebe überschreibt bei 211–221 die ältesten Werte. [Frame.h:105](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Frame.h:105) legt pro 100-ms-Rahmen einen Headroomwert ab. Damit verbleiben ungefähr die letzten 6,4 Sekunden, nicht die gesamte Passage.

**Vertragsabgleich:** SONDE-013 **M-03** verlangt eine Headroom-Verteilung über die Passage. [featureengine/Vertrag.h:670](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:670) wiederholt diese Aussage und nennt kurz danach widersprüchlich ein laufendes Evidenzfenster. Der Ring wird hier aber auch nicht bei jedem Evidenzfenster neu begonnen. Die drei Bedeutungen dürfen nicht gleichgesetzt werden. Eine vorhandene Zahl für die Anzahl der behaltenen Rahmen ersetzt keine korrekte Benennung ihres zeitlichen Umfangs.

**Korrekturziel:** Die geltende Passagenaussage korrekt erfüllen oder eine ausdrücklich beschlossene andere Produktdefinition sauber durch Vertrag, Writer, Reader und Anzeige ziehen. Ein Kommentarwechsel allein darf aus einer Passagenverteilung kein gleitendes Fenster machen. Für die derzeit dokumentierte Bedeutung müssen frühe und späte Abschnitte derselben Passage in die Verteilung eingehen.

**Belege:** HEADROOM in [models-extra-results.txt](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models-extra-results.txt), [models_extra.cpp](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models_extra.cpp).

### F09 — Analyseverlust im DSP überspringt den Zeit- und Lückenvertrag der Queue

**Auslöser:** Der DSP ist auf 48 kHz und 64 Samples vorbereitet, ein gültiger EQ mit +6 dB ist aktiv. Aufeinander folgen Hostblöcke mit 64, 128 und 64 Samples. Für den übergroßen Mittelblock verarbeitet der DSP Audio in Teilstücken, verwirft aber den vollständigen Analyse-Tap.

**Beobachtet:** DSP-Dropzähler 1, Queue-Dropzähler 0. Der dritte Block beginnt auf der lokalen Analysezeitachse bei **64 statt 192**. Segment unverändert, Lücke-davor-Flag 0, Queue-Verlustframes 0. Die Blockquarantäne akzeptiert die beiden verbleibenden Blöcke als zusammenhängend.

**Ursache:** [DspKern.cpp:788](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/dsp/DspKern.cpp:788) verwirft bei der Stückelung den Tap. [SondeProcessor.cpp:345](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:345) ruft bei ungültigem Tap und aktivem EQ die Analysequeue gar nicht auf. Deren lokale Zeit- und Verlustbuchführung wird deshalb übersprungen. Die in StampedAudioQueue.h dokumentierte Zeitachse zählt nur die tatsächlich zugeführten Blöcke weiter.

**Wirkung und Einschränkung:** Analysefenster können Material vor und nach einem verlorenen Block ohne sichtbare Grenze zusammenfügen. Das Gegenbeispiel hat keine verwertbar fortschreitende Hostzeit. Eine zuverlässige externe Projektzeit kann eine Diskontinuität zusätzlich erkennen; sie heilt nicht den fehlenden internen Dropvertrag. Der Befund betrifft Analyse, nicht verlorenes Audio.

**Korrekturziel:** Ein bewusst nicht publizierter Tap muss seine Sampledauer und Diskontinuität trotzdem in der Analysezuführung hinterlassen. Die Queue darf dabei kein nicht existentes Audio liefern. DSP- und Queue-Zähler sowie nachfolgende Segment-/Lückenlogik müssen dieselbe verworfene Zeitspanne beschreiben.

**Beweisgrenze:** Original-DSP und Original-Queue mit der nachgebildeten aktiven Tap-Abzweigung aus SondeProcessor. Parameterprüfung ergibt 1. Kein vollständig instanziierter SondeProcessor in diesem speziellen Fall. Der zeitliche Widerspruch ist im Komponentenverbund direkt beobachtet, Exit 1.

**Belege:** [gap-validated.cpp](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/audio/gap-validated.cpp), [gültiger Gegenbeleg](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/audio/gap-validated-log.txt). Frühere Scratchfassungen mit unvollständigem Parametersatz wurden ausdrücklich nicht übernommen.

### F10 — Gültigkeitsdauer entspricht nicht der Dauer des Loudnessfensters

**Früher in dieser Session ausgeführter Fall:** Nach drei Sekunden sauberem 1-kHz-Ton folgen 0,2 Sekunden nicht endliche Samples und danach wieder sauberes Material. Die Engine ersetzt die betroffenen Samples durch Nullen und zählt sie zunächst korrekt. Diese Eingabeprüfung wurde nach dem Stoppsignal nicht wiederholt.

**Beobachtet:** Schon der erste saubere Rahmen trägt erneut Momentary- und Short-term-Präsenzbits. Nach insgesamt 4,1 Sekunden sind auch beide ausgegebenen Ungültigkeitszähler null. LUFS-S beträgt weiterhin **−14,313746** statt des sauberen Referenzwerts von rund **−14,014145**, weil das 3-s-Fenster weiterhin die Ersetzungsperiode enthält. Es geht um einen als sauber präsentierten kontaminierten Messwert, nicht um ein NaN auf der Leitung.

**Ursache:** [Spektrum.h:120](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:120) ersetzt und zählt. [Lautheit.h:30](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:30) speichert die Zellenergien ohne entsprechende Zellgültigkeit, kurzLufs bei 90–101 mittelt alle 30 Zellen. [Frame.h:534](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/featureengine/Frame.h:534) verriegelt nur bei rahmenNichtEndlich im aktuellen Rahmen. Rahmen- und Evidenzzähler fallen früher zurück als die betroffene Loudnesshistorie.

**Korrekturziel:** Gültigkeit muss an der tatsächlich verwendeten Historie hängen. Jede Metrik muss so lange ungesetzt oder anderweitig vertraglich ungültig bleiben, wie ersetztes Material ihren Wert beeinflusst. Das betrifft die unterschiedlichen 400-ms- und 3-s-Fenster getrennt; bloßes Verlängern eines globalen Warnzählers löst die Aussage der Präsenzbits nicht sauber.

**Anforderung:** SONDE-013 M-07 und der eigene Kommentar an Frame.h:534–542 verlangen, ersetztes Material nicht als saubere Messung auszugeben. Kein zusätzlicher externer Produktstandard wird eingeführt.

**Belege:** NAN_BEFORE bis NAN_TWO_CLEAN_FRAMES in [models-results.txt](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models-results.txt), NAN_4p1_SECOND in [models-extra-results.txt](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/models/models-extra-results.txt). Original-Engine, kein vollständiger Wirelauf.

### F11 — Akzeptierte maximale Revision verletzt Mutation und Roundtrip

**Einordnung:** Seltene numerische Vertragsgrenze, P3. Keine Behauptung, dass ein normaler User durch gewöhnliche Bedienung rund 9 × 10^18 Revisionen erreicht. Die Reader akzeptieren diesen Zustand jedoch ausdrücklich als schreibbar. Der vorhandene Gegenbeleg entstand vor dem Stoppsignal.

**Beobachtet:** Bei maximaler int64-Bestandsrevision entfernen die drei öffentlichen Handgriffe für Intent, Schutz und Beziehung den Eintrag trotzdem, melden aber false und erzeugen kein Dirty. Bei maximaler Assistentenrevision wird Überspringen angenommen; der MSVC-Lauf erzeugt −9223372036854775808. Ein danach gespeicherter State lädt nur noch read-only mit dem Grund, die Assistentenrevision müsse mindestens 1 sein. Die vier Fälle bei MAX−1 funktionieren; die vier MAX-Fälle verletzen die Erwartung, insgesamt 20/24 erfüllt, Exit 1.

**Ursache:** In [NakamaState.cpp:2283](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/state/NakamaState.cpp:2283), 2341–2345 und 2456–2461 steht das Entfernen vor dem möglicherweise fehlschlagenden bestandsrevisionHeben. Die Assistentenpfade erhöhen bei 2662, 2707 und 2810 ohne entsprechende obere Schranke. Der Reader erlaubt MAX, aber keine negative Folgerevision.

**Korrekturziel:** Ablehnung muss vor der ersten Mutation feststehen oder den Zustand vollständig unverändert zurücklassen. Jeder angenommene persistente Folgezustand muss wieder vom eigenen Reader akzeptiert werden. Die Revision darf nicht unkontrolliert überlaufen. Keine neue Revisionssemantik oder stille Rebasierung erfinden.

**Belege:** [repro_numbers.cpp](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/state/repro_numbers.cpp), [run-numbers.txt](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/state/run-numbers.txt). Öffentliche Prozessor- und Speicherzugänge, gleiche unveränderte Objektdateien wie F01.

### F12 — float-Verengung liegt hinter der Endlichkeitsprüfung

**Einordnung:** Numerischer Rand weit außerhalb gewöhnlicher Audiopegel, P3. Der bereits ausgeführte Lauf verwendete endliche Werte nahe FLT_MAX, keinen normalen 0-dBFS-Mix. Er wurde nach der Einschränkung des Users nicht wiederholt.

**Beobachtet:** Der gültige, öffentlich validierte DSP-Parametersatz mit +6 dB liefert bei 0,5 × FLT_MAX noch einen endlichen Ausgang. Bei 0,75 × FLT_MAX wird der float-Ausgang Inf, während der double-Tap endlich bleibt; Eingang-Nichtendlich- und Filterheilungszähler bleiben null. Der gültig erzeugte Hörmarkierungs-Puls liefert bei einem 1-kHz-Ton mit Amplitude 0,8 × FLT_MAX auf dem ausgewerteten Kanal 2.348 nicht endliche Ausgabesamples in 12.800 Frames, erstes bei Sample 2.796; der zugehörige Wet-Zähler bleibt null.

**Ursache:** [HoerMarkierung.h:735](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/HoerMarkierung.h:735) prüft die double-Zwischengröße, konvertiert erst danach nach float. [DspKern.cpp:1007](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/dsp/DspKern.cpp:1007) verengt den Ausgang ebenfalls ohne einen passenden Ausgangsriegel. Die Analyse-Tap-Kopie in SondeProcessor.cpp:353–355 hat dieselbe Verengungsstelle; deren Wirkung wurde hier nicht separat durch die volle Sonde gemessen.

**Korrekturziel:** Die Zusage zu endlichen Werten und Zählern muss die tatsächlich ausgegebene Darstellung einschließen. Das ist kein Auftrag zum Einbau eines musikalischen Limiters oder zur Änderung des neutralen Passthroughs. Die Ursache liegt im darstellbaren Zahlenbereich am aktiven Ausgang.

**Beweisgrenze:** Original-DSP und Original-Hörmarkierung; öffentlich gültige Parameter, keine Ersatzimplementierung. Der Harness druckt die Widersprüche und endet regulär, nicht mit einer fehlgeschlagenen Assertion. Alte Scratchfassungen mit ungeeigneten Parametern oder Hilfs-Stubs wurden verworfen.

**Belege:** [repro-validated.cpp](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/audio/repro-validated.cpp), [Ausgabe](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/audio/repro-validated-log.txt), [Eingangshashes](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/audio/hashes.txt).

### F13 — Der Speichernachweis unterscheidet Messfehler nicht von null Bytes

**Auslöser:** rss_bytes liefert bei fehlgeschlagenem Prozesszugriff oder fehlgeschlagener Windows-Speicherabfrage den Wert 0. Derselbe Wert wird als gewöhnlicher Messpunkt in den Bericht geschrieben.

**Vor dem Stoppsignal beobachtet:** Ein existierender realer A24-Bericht wurde ausschließlich als Kontrollfixture durch das heutige Orakel ausgewertet: Exit 0. Die aktuelle Funktion liefert für den nicht abfragbaren Prozess 0 den Fehlerwert 0. Ersetzt man im Kontrollbericht ausschließlich die RSS-Werte durch diesen Fehlerwert, meldet dasselbe Orakel weiterhin Exit 0 und sämtliche S07-Speicherprüfungen grün, nun mit 0,0 → 0,0 MB. Der kleine Gegenbeleg endet deshalb mit Exit 1.

**Ursache:** [pruefe_session_soak.py:284](C:/Users/phili/Projekte/Nakama/tools/eq-copilot/pruefe_session_soak.py:284) bis 300 kodiert Scheitern als Zahl. Die Aufzeichnung bei 429–431 übernimmt sie unverändert. Die Bewertung ab 992 hält Anzahl, Zeitabstand, Generationen und Wachstum gegen das Budget, verlangt aber keinen erfolgreichen Speicherabruf.

**Korrekturziel:** Fehlende Messung braucht einen eigenen Status und darf kein Budget bestätigen. Ein Lauf mit fehlender tragender RSS-Messung muss als unvollständig oder fehlgeschlagen enden. Die gültigen historischen Messwerte bleiben gültige Kontrollwerte; aus dem Orakelfehler folgt nicht, dass deren damalige Windows-Abfragen tatsächlich gescheitert waren.

**Beweisgrenze:** Ausschließlich ein Orakelgegenbeispiel mit historischem Bericht und aktueller Funktion. **Kein neuer Live-Soak, kein beobachteter Speicherlecknachweis und kein Nachweis fehlerhafter RSS-Erfassung im alten A24-Lauf.**

**Belege:** [repro_soak_missing_rss.py](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/repro_soak_missing_rss.py), [Auswertung](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/repro-soak-missing-rss.txt). Historische Kontrollquelle: docs/beweise/roh/NAK-246-4ff6f24.md.

### F14 — NAK-163: Fehlgeschlagene Invalidierung nimmt den falschen Ausschluss zurück

**Bereits registrierter Mechanismus, jetzt konkret belegt:** Die Historie steht bei 32 Belegen. E1 ist durch eine frühere erfolgreiche Intervention ausgeschlossen. Eine neue Invalidierung von E0 wartet auf den Store. Währenddessen wird E32 aufgenommen und E0 durch Retention vorne entfernt. Der Index von E1 verschiebt sich auf die vorherige Position von E0. Nach dem SQLite-Fehler macht der Rückweg den Eintrag an dieser Position wieder gültig.

**Beobachtet vor dem Stoppsignal:** Der Original-Broker trifft den regulären SQLite-BUSY-Fehlerweg. Anschließend hat E1 im Cache ausschlussgrund = None, in der SQLite-Projektion weiterhin "intervention". Der Vergleich schlägt fehl, Cargo-Exit 101. Der neu hinzugefügte Beleg wird wegen desselben Storefehlers ebenfalls verworfen; das verhindert die zwischenzeitliche Verschiebung der Historie nicht.

**Ursache:** [invalidierung_verdrahtung.rs:158](C:/Users/phili/Projekte/Nakama/broker/src/coordinator/invalidierung_verdrahtung.rs:158) speichert für die Rücknahme ClientKey und Deque-Index. [invalidierung_ruecknehmen:235](C:/Users/phili/Projekte/Nakama/broker/src/coordinator/invalidierung_verdrahtung.rs:235) löscht den Grund am späteren Index. [evidenz.rs:186](C:/Users/phili/Projekte/Nakama/broker/src/coordinator/evidenz.rs:186) verschiebt beim Retention-Abbau die Positionen.

**Korrekturziel:** Rollback an die stabile Evidence-ID und die eigene vorläufige Änderung binden. Weder fremde frühere Ausschlüsse noch spätere berechtigte Änderungen dürfen dadurch aufgehoben werden. Cache und persistente Projektion müssen nach dem Fehler denselben ausgeschlossenen Beleg zeigen.

**Register:** **NAK-163** beschreibt diese Kombination bereits als Härtung/Struktur. Keine neue Dublette anlegen. Der konkrete Lauf belegt, dass die beschriebene Kombination eine falsche aktuelle Gültigkeit erzeugt; die Priorisierung als bloße Strukturpflege sollte anhand dieses Belegs neu beurteilt werden.

**Beweisgrenze:** Isolierter lokaler Original-Broker und SQLite-Datei, bewusst hergestellter Storefehler. Kein Produktionsbroker, keine Produktionspipe, keine echte Projektdatei betroffen. Keine Wiederaufnahme dieses Fehlerexperiments aus dieser Übergabe ableiten.

**Belege:** [broker_repros.rs:435](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/broker/tests/broker_repros.rs:435), [rollback.log](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/broker/rollback.log).

## 5. Erweiterte Nachweisprüfung und nicht bestätigte Kandidaten

### N01 — Der EBU-True-Peak-Testumfang ist kleiner als die dokumentierte Zusage

SONDE-013 M-02 und M-81 verlangen das offizielle EBU-Testset. [Sonde013TruePeakGoldenTest.cpp:127](C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp:127) führt fünf analytische Sinusfälle 15–19 aus; der erzeugte P4-Korpuseintrag nennt denselben Umfang. Die anschließenden generierten Sampleratenfälle und weitere eigene Randtests ergänzen diese Auswahl.

Die am 12.09.2026 gelesene Primärquelle [EBU Tech 3341, Version November 2023](https://tech.ebu.ch/files/live/sites/tech/files/shared/tech/tech3341v4_0.pdf) fordert in §2.6 ausdrücklich die True-Peak-Fälle **15–23**. Tabelle 1 auf Seite 9 enthält zusätzlich vier transiente Fälle 20–23 mit unterschiedlichen Abtastversätzen. Die strengere Sinustoleranz von ±0,1 dB ersetzt diese zusätzliche Signalform nicht. In den untersuchten Test- und Korpusquellen wurde kein vollständiger entsprechender Nachweis für 20–23 gefunden.

**Folgerung:** Belegt ist eine Lücke zwischen dem nachgewiesenen Umfang und der Aussage „offizielles Testset bestanden“. Nicht belegt ist ein numerisches Versagen des True-Peak-Kerns an den fehlenden Fällen; diese wurden hier nicht gerechnet. Auch folgt daraus kein automatischer Auftrag zum Austausch des DSP-Kerns. NAK-199 betrifft bisher das fehlende externe LUFS-I-Referenzmaterial und behauptet True Peak als gedeckt; diesen Teil der Dokumentation bei der Nacharbeit mit abgleichen.

**Abschlussziel:** Die Behauptung muss ihren tatsächlichen Umfang nennen. Eine vollständige Zusage benötigt passende Belege für den vollständigen referenzierten Satz. Diese Übergabe startet keinen weiteren Prüflauf.

### Offene Kandidaten — ausdrücklich keine weiteren bestätigten Defekte

| Thema | Was vorhanden ist | Was zur Entscheidung fehlt |
|---|---|---|
| NAK-252 / alter V1: Writer-Headroom und Reader-Sammlungsgrenzen | Bytegrenze und zusätzliche Readergrenzen sind quellbelegt. Ein lokaler Harness wurde vorbereitet. | Kein abgeschlossener Build-/Read-Write-Read-Beleg. **Kein bestätigter Stateverlust.** Nach dem Stoppsignal nicht weiterverfolgt. |
| Reload während einer bereits geparsten Evidenzrücknahme | Früher Bindungscheck und spätere Modellmutation sind getrennte Stellen in SourcesModel. | Kein abgeschlossener Lauf; ein Entwurf wurde nicht ausgeführt. Erreichbarkeit und wirksame Generationenbarriere nicht abschließend entschieden. |
| Passagenbindung zwischen Message-Thread und Analyseworker | Übergabepfad in Analyse.cpp und mehrere Änderungsmerker gelesen. | Kein abgeschlossener Gegenbeleg für eine stale Bindung. |
| K-Filterhistorie an einer Passage | Komponentenwerte unterscheiden sich für leise Passage nach tiefem lautem Material und isoliertem Start. | Exakte fachliche Definition der Filtervorgeschichte und Bezug zur tatsächlichen Produzentenkette. Nicht mit F07 gleichsetzen. |
| Einheit der Stereo-Mid/Side-Bandwerte | Auffälliger absoluter dB-Wert im Headerlauf; relative Größen können anders definiert sein. | Vollständiger Einheitenvertrag mit Quantisierung und Consumer. Kein gesicherter zusätzlicher Fehler. |
| Fingerprint ohne Onset-Information | Gleichheit eines stationären Signals ergibt im beobachteten Fall keine positive Ähnlichkeitsaussage. | Klären, ob dies der beabsichtigte konservative Fall ohne ausreichende Information ist. Nicht als falsche Erkennung zählen. |
| Ungültiges erneutes prepare ohne vorausgehendes release | Quellenfrage nach altem DSP-Zustand. | Regulärer untersuchter Hostpfad deaktiviert und gibt Ressourcen vorher frei. Kein neuer normaler Hostdefekt belegt. |
| Installer/Repair/Rückweg mit unterschiedlichen Manifestständen | Vertrauensbindung zwischen Manifest, Journal und installierten Dateien gelesen. | Kein abgeschlossener Upgrade-/Rollback-Nachweis. Ablehnung unterschiedlicher Stände kann beabsichtigt sein; kein Defekt daraus abgeleitet. |

Die zu diesen Kandidaten teilweise sichtbaren zusätzlichen Werte in den archivierten Modelllogs sind keine stillschweigend bestätigten Befunde.

## 6. Abdeckung und tatsächliche Prüfungen

Die Arbeit lief in zwei Wellen mit insgesamt sechs Agentenpaketen und Root-Integration. Maximal drei Unteragenten arbeiteten parallel. Nicht jeder Agentenlauf wurde erfolgreich beendet: Einzelne brachen wegen automatischer Sicherheitsfilter ab; die verbleibenden Experimente wurden auf die letzte User-Anweisung hin gestoppt. Unfertige Pakete werden nicht nachträglich zu abgeschlossenen Reviews erklärt.

| Bereich | Tatsächlich bearbeitet | Wesentliche Grenze |
|---|---|---|
| Audio/DSP | Markierung, Slot-/Bankbesitz, aktiver DSP, Tapverlust, Zahlenverengung, alte D1/D7/D10 | Kein neuer kompletter Nulltest-, Sanitizer- oder Host-Performanceabschluss |
| State/IPC | Quellenbefehle, ACK-Drain, Save/Load, Reload-Generation, Modellpublikation, Revisionen, alte D2/D3/D4/D6/D8 | Sammlungsgrenzen und spätere Rücknahme-/Reloadkandidaten unentschieden |
| Broker | Hypothesenberechnung, Snapshotqueue, Store/Cache-Rollback, alte D5/D9 | Keine neue vollständige Broker-E2E-Serie; Queuefolge nicht bis zum Plugin ausgeführt |
| Analysefachmodelle — ausgeweitet | Aktivitäts-/Coveragegate, Passagen-True-Peak, Headroom, Loudnesshistorie, ausgewählte Fingerprint-/Stereofragen | Komponentenläufe; mehrere fachliche Kandidaten bewusst nicht hochgestuft |
| Host-Lebenszyklus — ausgeweitet | Echtes VST3: Busvertrag, Audioverarbeitung, Reset; direkter Prozessor als Kontrolle | Kein FL Studio, kein physischer Audiotreiber-/XRun-Beleg, keine Installation |
| Werkzeuge und Nachweisqualität — ausgeweitet | Gesundheit samt Selbsttest, RSS-Orakel, Quellinventar, EBU-Testumfang | Gesundheitsfilter unvollständig; kein vollständiger aktueller Kanon |
| Lieferung/Installer — ausgeweitet, begrenzt | Quellenlektüre zu Installation, Repair und Rückweg | Keine Installation und kein abgeschlossener Upgrade-/Rollbacklauf |

| Prüfung dieser Session | Ergebnis | Was daraus folgt |
|---|---|---|
| Planstandrechnung | 33/45 abgenommen; Planblatt separat aktualisiert | Router aktuell zum damaligen Rechenstand, kein Produktbeweis |
| Dokuriegel für das Planblatt | Exit 1: geplante fehlende NAK-226/NAK-227-Manifeste und historischer Pfad | Keine neuen Produktfehler aus diesen bekannten Dokumentverweisen abgeleitet |
| Qualitäts-Preflight | Benötigte lokalen Werkzeuge vorhanden, kein Preflight-Fund | Ausführbarkeit, keine Qualitätsfreigabe |
| Gesundheit mit Selbsttest | **121/121 Selbsttests bestanden; Gesamtauswertung Exit 4** | Prüfer läuft, Codebase-Maße bleiben gerissen |
| Gesundheitsmaße | 29 Funktionen über 200 Zeilen bei Grenze 28; 33 Kommentar-Bezeichner bei Grenze 30; acht Dateien über Ziel 1.500 Zeilen | Bestehende Pflegebaustelle NAK-255, keine neue Funktionsfehlerzählung |
| Umfang Gesundheit | QUELLORTE weiterhin nur broker/src, plugin/src und plugin/core | „0 Dateien über 2.000 Zeilen“ gilt nur darin; **NAK-249** bleibt relevant |
| Clippy | Nicht angefordert / nicht gemessen | Kein Clippy-PASS |
| Bereits ausgeführter Cargo-Advisory-Abgleich | Exit 0, 66 Abhängigkeiten, 0 gemeldete Advisories im verwendeten Datenbankstand | Nur dieser begrenzte Abhängigkeitsabgleich; keine allgemeine Sicherheitsfreigabe |
| Original-Queue-Kontrollen | Acht bestehende Tests bestanden | Per-Key-Fixes D5/D9 bestätigt; F03 bleibt daneben bestehen |
| Gezielte Gegenbeispiele | Resultate und Unterschiede in F01–F14 dokumentiert | Manche Programme melden normale Exits und drucken Messwerte; ein grüner Exit widerlegt diese Werte nicht |
| Voller kanonischer Beweislauf mit Build | **Abgebrochen, Exit 1 nach Unterbrechung** | Kein vollständiges frisches Manifest, keine aktuelle 64/64-Aussage |

Der abgebrochene Aufruf lautete:

    pwsh -NoProfile -File tools/beweise.ps1 -Bauen -Titel 'Zweiter unabhaengiger Codeaudit 2026-09-12'

Diese Aufzeichnung dient der Provenienz. Der Befehl wird durch die Übergabe nicht erneut gestartet. Auch das bloße Vorhandensein frisch gebauter Einzelbinaries beweist keinen abgeschlossenen Gesamtkanon.

Die Gesundheitswerte stammen aus dem Messzeitpunkt vor den späteren reinen Orchestrierungsänderungen. Die damals gemessenen Kontextdateigrößen sind daher keine frische Größenbehauptung über den zuletzt geänderten Dirigenten-Skill. Der Codeumfang blieb im verglichenen Produktstand unverändert.

## 7. Empfohlene Übernahme durch Claude

Diese Reihenfolge ist ein **Nacharbeitsvorschlag**, keine automatische Ausführung oder Wiederaufnahme gestoppter Experimente. Vor konkreten Änderungen den dann aktuellen CLAUDE-/AGENTS-Kontext, Worktree und betroffene Tests lesen. Den hier geprüften Stand gegen späteren Produktcode abgleichen. Fremde Änderungen erhalten.

1. **Publikationskette schließen:** F01, F02/NAK-253 und F03. Den jeweils letzten gültigen fachlichen Stand von State beziehungsweise Eingangsrevision über Modell, Store und Sender erhalten. Die drei Ursachen getrennt behandeln, danach ihre Übergänge gemeinsam beurteilen.
2. **Hostvertrag korrigieren:** F04 und F05. Angenommene Buslayouts und Resetverhalten müssen zur bestehenden DSP-Funktion passen. Keine ungefragte Mehrkanal- oder Tail-Funktion hinzufügen.
3. **Bedeutung der Analysewerte korrigieren:** F06, F07, F08 und F09. Einheiten, Passagenanfang/-ende, vollständiger Auswerteumfang und verlorene Zeit sind fachliche Invarianten. F10 gehört in dieselbe Betrachtung der Lebensdauer von Analysehistorien; seine vorhandenen Eingabefehlerbelege bleiben unter der oben genannten Prüfgrenze archiviert.
4. **Persistenz- und Zahlenränder einordnen:** F14/NAK-163, F11 und F12 anhand des belegten Effekts behandeln. Seltene Zahlenränder nicht zu normalem Benutzerverhalten umdeuten. Ablehnung ohne Mutation und eigener Reader-kompatibler Output bleiben die Korrekturziele.
5. **Nachweisbehauptungen berichtigen:** F13, N01, NAK-249 und NAK-255. Fehlende Messung ist kein Budget-PASS; Teilkorpus ist kein vollständiges Testset; ein begrenzter Quellfilter beschreibt nicht das ganze Repository. Reine Kommentar- oder Grenzwertänderungen dürfen die zugrunde liegende Lücke nicht unsichtbar machen.

Für jeden übernommenen Punkt sollten Ursache, kleinster kohärenter Fix, tatsächlich ausgeführter Nachweis und Restgrenze gemeinsam dokumentiert werden. Bereits registrierte Punkte aktualisieren und die Audit-ID angeben. Neue Register-IDs aus dem dann aktuellen Register beziehen, keine Nummern aus dieser Übergabe erfinden.

Eine spätere Behauptung, der Bestand sei freigegeben, braucht ein neues, abgeschlossenes Urteil mit tatsächlich ausgeführten passenden Prüfungen auf dem dann aktuellen Stand. Dieser Bericht liefert weder eine solche Freigabe noch einen Blankoauftrag für die vom User gestoppten Prüfhandlungen.

## 8. Belegarchiv und Nachvollziehbarkeit

Die Harnessquellen, vorhandenen Buildargumente und Rohlogs wurden unverändert aus den lokalen Scratchverzeichnissen übernommen. Es wurden keine EXE-, OBJ- oder LIB-Dateien in das Auditarchiv kopiert. Die zwei kleinen BIN-Dateien im Hostordner sind durch öffentliche Produktmethoden erzeugte Parameter-Statefixtures, keine Programme.

Die archivierten Builddateien enthalten die absoluten Scratchpfade des damaligen Laufs. Sie sind eine Aufzeichnung der Zusammenstellung und **keine ungeprüfte portable Ein-Befehl-Anleitung**. Nicht abgeschlossene Harnessentwürfe wurden nicht als erfolgreiche Belege archiviert. Der Brokerharness bindet das Original-Crate per Pfad ein; der Queueharness bindet zusätzlich das Original-Queuemodul ein. Die C++-Headerharnesses verwenden die Produktheader, die Prozessortests unveränderte Objektdateien. Bei F09 ist die kleine Sonde-Abzweigung ausdrücklich nachgebildet.

Das Belegarchiv wird ohne automatische Zeilenendenumwandlung versioniert. Ursprüngliche Leerzeilen, Konsolenzeichen und die Zeilenenden der damaligen Builddateien bleiben erhalten; kosmetische Whitespacehinweise im Archiv sind in der Verifikationsnotiz von der eigentlichen Markdown-Prüfung getrennt.

| Beleggruppe | Inhalt |
|---|---|
| [Quellinventar](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/source-inventory.json) | Dateiliste, Zeilen-/Bytezahlen, SHA-256 und Basisrevision |
| [Gesundheit](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/gesundheit.json) / [vollständige Ausgabe](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/gesundheit-raw.txt) | Strukturierte Maße und 121 Selbsttestresultate |
| [Cargo-Abgleich](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/evidence/cargo-audit.json) | Verwendete Advisory-Datenbank: 1.243 Einträge, Commit b50980aad8b8f14f77e25a97b32dd94bf008b0af, aktualisiert 09.09.2026 |
| evidence/state/ | F01 und F11, fünf Publikationsläufe, serielle Kontrolle und Input-Provenienz |
| evidence/broker/ | F02, F03, F14, Cargo-Konfiguration, Harnessquellen und Original-Queue-Kontrollen |
| evidence/audio/ | Ausschließlich die validierten Endfassungen zu F09 und F12, Logs und Hashes |
| evidence/models/ | F06, F07, F08, F10; zusätzlich sichtbar gebliebene, nicht als Defekte bestätigte Fachbeobachtungen |
| evidence/host/ | F04/F05 über echten VST3-Wrapper und direkte Prozessorkontrollen |
| [Verifikationsnotiz](C:/Users/phili/Projekte/Nakama/docs/audits/2026-09-12-code-review-2/VERIFIKATION.md) | Zusammenstellung, Quellenstand, Grenzen und Abschluss der Dokumentprüfung |

SHA-256 des untersuchten und beim Quellenabschluss erneut nur lesend abgeglichenen Probeeq-VST3-Moduls:

    0004D7B8C9F29ADE4710BB0FCFA25B8A622966B314A4630592594BDE816925A9

SHA-256 der für die validierten Audioharnesses verwendeten NakamaKern.lib laut archiviertem Hashbeleg:

    F06D907835CCD02C6B8639036E3A7C90C4BDD583A64F0D6B9A0CEBD2024759E5

**Offen bleiben:** vollständiger aktueller Kanon, vollständige Host-/DAW- und Callback-Performance, Sanitizer, Clippy, Installation/Upgrade/Rückweg und die ausdrücklich unentschiedenen Kandidaten. Nicht gelaufen bedeutet nicht bestanden. Die vorliegende Übergabe ist fertiggestellt; die Produktnacharbeit ist es nicht.
