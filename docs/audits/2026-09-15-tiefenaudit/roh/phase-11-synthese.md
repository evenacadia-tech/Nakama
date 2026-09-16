URTEIL: BÜNDEL 36 (P1 24 · P2 4 · P3 4 · P4 4) — Die Codebase ist in ihren Bausteinen breit geprüft, enthält aber freigabeblockierende Fehler an den Übergängen zwischen Host, State, Transport und Beweis.

# Tiefenaudit 3 — Phase 11: Synthese

| Feld | Wert |
| --- | --- |
| Phase / Auftrag | 11 — Gesamtbild, Priorisierung und Abarbeitung; gebunden 2026-09-16 04:03 |
| Datum / Zeitzone | 2026-09-16, Europe/Berlin (MESZ) |
| HEAD vorher / nachher | `d53e0a370b9976d8ebe87e87eed25096237b8e53` / `d53e0a370b9976d8ebe87e87eed25096237b8e53` |
| Branch | master |
| Modell / Effort | `gpt-6-astra` / `max`, laut Startlog |
| Start | 04:03:51 MESZ laut Startlog; erste Uhrabfrage 04:04:08 |
| Abschlussstand / Dauer | 04:39:27 MESZ / 35 min 35 s seit Startlog |
| Methode | Lesende Synthese der vollständigen Befundpopulation; Quellen-/Test-/Absichts-/Registerabgleich aus Phase 10 übernommen und an den Synthesenähten gezielt nachgelesen |
| Schreibziele | Nur dieser Bericht und Abschnitt `## Kopf` in `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` |
| Ausführung | Kein Bau, Test, Kanon, Planstand-Generator, Installation, Staging, Commit oder Push; kein Subagent |
| Skill | `.agents/skills/nakama-plugin-quality/SKILL.md` gelesen; nur lesender Qualitäts-/Beweisrahmen, nachgeordnet dem engeren Phasenauftrag |

Startlog nur gelesen: `C:/Users/phili/AppData/Local/Temp/nakama-tiefenaudit-p11-d53e0a370b9976d8ebe87e87eed25096237b8e53-start.log`; Start `2026-09-16T04:03:51.5871404+02:00`, Modell/Effort wie oben.
Das andere Modell aus Phase 12 ist `gpt-5.6-sol` / `max`; seine fünf Einstufungen werden nicht verändert.

## Synthese und Abarbeitungsplan

Nach dem Skeptikerpass bleiben 59 eigenständige Befunde: 40 Defekte, 15 Lücken und vier Härtungen; alle acht critical und alle 40 Defekte liegen in P1-Bündeln.
Weitere Bauarbeit braucht zuerst diese P1-Reparaturen und verlässliche Detektoren an den betroffenen Grenzen, denn dieser lesende Lauf liefert weder einen neuen Kanonbeweis noch eine Garantie für die gesamte Codebase.

**Stand:** Phase 11, Auftrag 2026-09-16 04:03, HEAD `d53e0a370b9976d8ebe87e87eed25096237b8e53`; Synthese von Phase 01–10 und 12, Modell `gpt-6-astra`, Effort `max`.
Vorschlag für den Dirigenten ab **18.09.2026**, keine Ticketfreigabe und keine Reparatur in diesem Lauf.
Die vorhandenen Phasenabschnitte bleiben historische Einzelurteile; für Summen gilt dieser Kopf mit dem Skeptikerstand aus Phase 10.
Die alte Phasentabelle oberhalb von „Kopf“ bleibt wegen der engen Schreibgrenze unverändert.

### Zähler nach Skeptiker

| Schwere | DEFEKT | LÜCKE | HÄRTUNG | Gesamt |
| --- | --- | --- | --- | --- |
| critical | 7 | 1 | 0 | 8 |
| high | 24 | 0 | 0 | 24 |
| medium | 9 | 14 | 0 | 23 |
| low | 0 | 0 | 4 | 4 |
| Gesamt | 40 | 15 | 4 | 59 |

Phase 10 bestätigt 56 Befunde und stuft drei herab; **0 widerlegt, 0 Duplikate, 0 unentschieden** innerhalb der 59 aufgenommenen Befunde.
Geändert werden nur T3-04-04 → **low/HÄRTUNG**, T3-07-03 → **medium/LÜCKE**, T3-07-05 → **medium/LÜCKE**.
Die fünf Phase-12-Befunde des anderen Modells bleiben mit **2 critical/DEFEKT, 1 high/DEFEKT, 2 medium/LÜCKE** unverändert.
Phase 11 eröffnet **keine neue Befund-ID** und zählt die Skeptikerzeilen nicht erneut.

Klassenpopulation: K1 **2**, K2 **6**, K3 **16**, K4 **18**, K5 **6**, K6 **5**, K7 **0**, K8 **4**; dazu je **1** „K-neu: Privileggrenze“ und „K-neu: Diagnose-Vertrauensgrenze“.
Die Ursprungsphasen enthalten **136 Einträge verworfener Verdachte** (einschließlich Dubletten/„bekannt“), keine 136 zusätzlich widerlegten eindeutigen Fehler.
Aus **20 Frageeinträgen** entstehen **17 Karten**; PF11-07 hält eine durch den Skeptiker bereits geklärte Absicht fest und ist keine erneut zu stellende Frage.

### Ursachenbündel

**Leseregel:** genau **ein** Bündel je Befund, zusammen 59/59 Mitglieder ohne Überschneidung.
Ein Bündel ist ein vorgeschlagener späterer Ticketauftrag mit eigener Verhaltensmatrix; die angegebenen Beine sind **zu erweiternde oder gezielt zu fahrende Beweise**, keine hier ausgeführten Tests.
„Ticketpfade“ schätzt die Zahl berührter Quell-, Vertrags-, Fixture- und Testdateien; Manifest-/Rohlogpfade kommen nach Ticketanlage hinzu, Pfade verschiedener Tickets können sich überschneiden.
Große Bündel B08/B12 brauchen eine Matrix je realem Eintritt, keine einzige Sammelassertion.
Priorität und Schwere sind verschieden: P1 umfasst auch sechs mit denselben Risiken gekoppelte Lücken, ohne deren Kategorie oder Schwere zu erhöhen.

| Bündel | Priorität | Mitglieder | Ursache | Kleinster Fix | Beweis | Ticketpfade |
| --- | --- | --- | --- | --- | --- | --- |
| B01 — Frische der tatsächlich geprüften Binaries | P1: Sonst können die nachfolgenden Reparaturen mit alten Testbinaries bewertet werden. | T3-09-01 | Der Frischeriegel kennt nicht alle produktiven Quellabhängigkeiten. | DSP-Unterbaum und abhängige Testziele im Frischenachweis schließen. | Frische-Gegenfall mit normaler DSP-Änderung und altem B6/B7; A14 darf das nicht heilen. | 3–5: tools/beweise.ps1, Plugin-CMake, gezielter Frische-Selbsttest |
| B02 — Vertrauensanker der erhöhten Installation | P1: Der bestätigte Privilegbruch liegt vor jeder weiteren Installation. | T3-12-01 | Die erhöhte Aufgabe führt eine vom nicht erhöhten Benutzer veränderbare Ausführungskette aus. | Aufgabenaktion samt Helfern an unabhängig geschützten Code binden und Artefaktautorisierung außerhalb der beschreibbaren Quelle verankern. | A17/A18 erweitern: veränderte Aktion, Helfer oder Manifest dürfen keine erhöhte Fremdausführung auslösen; tatsächliche Aufgaben-/Dateirechte getrennt prüfen. | 4–7: Aufgabenregistrierung, Installer, Manifestprüfung, Gegenpfadtests |
| B03 — Broker-Pin über Bau und Installation | P1: Ein konsistenter Dateisatz allein garantiert keinen startbaren installierten Broker. | T3-05-03 | Plugin-Pin und installierter Broker können aus verschiedenen Baugenerationen stammen. | Broker vor Plugin-Pin erzeugen und die Generation bis Manifest und Installation vergleichen. | A14/A17/A18 plus B8: H0-Plugin/H1-Broker ablehnen; vollständige H1-Kette akzeptieren. | 4–6: Plugin-CMake, tools/beweise.ps1, tools/fl/laufzeit.ps1, Manifestprüfung, B8 |
| B04 — Authentisierte Verbindungsidentität | P1: Der Sicherheitsbruch bleibt trotz enger Pipe-ACL erreichbar. | T3-12-02 | OS-SID-Prüfung autorisiert die behauptete Plugin-/Sensor-/Prozessidentität nicht. | Verifizierten Peer und zulässige Anmeldung/Befehlsziele gemeinsam binden; v2-Token darf keine fremde Instanzrolle freischalten. | A4 security_vectors und B10: richtige/falsche SID, PID, Sensorrolle, Token, Ziel sowie v2→v3-Übergang; Altstände nach versioniertem Vertrag. | 6–10: server_v3/auth/bootstrap/verbindung, Coordinator-Link, v2-Tokenpfad, C++-Handshake, Verträge/Fixtures |
| B05 — v2 darf v3 nicht stilllegen | P1: Eine fremde v2-Belegung kann den benötigten Produkttransport verhindern. | T3-12-03; T3-12-05 | Der unzureichend begrenzte Legacy-Eingang ist eine globale Voraussetzung der v3-Verfügbarkeit. | v2-Namenskollision von v3-Start entkoppeln und v2-Handshake/Leerlauf/Rate mit expliziten Grenzen versehen. | A4/B8/A22: besetzte v2-Pipe bei freier SID-v3-Pipe; 64 langsame v2-Clients; v3 bleibt erreichbar und v2 gibt Ressourcen frei. | 4–7: broker/src/lebenslauf.rs, server.rs, framing.rs, Lifecycle-/Lasttests |
| B06 — Sonde-Callbacks besitzen ihren Lebenslauf | P1: Der Quellenpfad erlaubt einen Zugriff nach Prozessorfreigabe. | T3-04-01 | Abgelöste Clientläufe behalten drei Provider mit rohem SondeProcessor-Besitzbezug. | Besessene Callback-Schleuse wie bei Gen auf sämtliche Sonde-Provider samt Abbaureihenfolge anwenden. | B10/B14: jeden der drei Provider über Stop/Detach festhalten, Prozessor abbauen, Callback freigeben; kein Besitzerzugriff. | 3–5: SondeProcessor.cpp/.h, IPC-Lifecycle-/Reloadtests |
| B07 — Messpunkt-Panel: expliziter, lebender Commit | P1: Derselbe Destruktor erzeugt Stateverlust und den kritischen Besitz-Sonderpfad. | T3-02-04; T3-04-03 | Der Panel-Destruktor speichert ungefragt über eine ungeprüfte Prozessorreferenz. | Speichern an einen ausdrücklichen Commit mit Lebenslaufprüfung binden; Schließen/Abbau verändert keine Daten. | B2/B14/B15: nur öffnen/schließen mit 121/61 Zeichen, ungebundener State, Editor-vor-Prozessor-Abbau bei offenem Popout; Dirty nur bei Änderung. | 3–5: PluginEditor.cpp, Bindungszugriff, State-/Reload-/Shot-Gegenfälle |
| B08 — Hostereignisse am Audio- und Reload-Rand | P1: Audio-Sperren, falscher Offline-Inhalt und trotz read-only wirksame Automation verletzen bestehende Zusagen. | T3-01-02; T3-01-03; T3-01-04; T3-01-05; T3-05-01; T3-05-02 | Hostautomation und Offline-Wechsel besitzen keinen durchgängig lockfreien, generationsgebundenen Übernahmezeitpunkt. | Hostereignisse ohne APVTS-/State-Sperren am definierten Blockrand übernehmen; Reload verwirft alte Mailboxen, read-only sperrt Aktuation und Offline beginnt sofort mit Committed. | B6/B7 plus Wrapper-Gegenfälle: 112 Parameter, Automation in wechselnden Blockgrößen ohne Worker-Handtakt, erstes Offline-Sample, ausstehender Wert vor Load, unbekannte Major mit enable/trim; RT-Wache vor Parameterverarbeitung. | 8–14: SondeProcessor, Hostparameter-Anbindung, Transaktionskern/DspKern, B6/B7, Wrapper-Prüfzugriff |
| B09 — Bitidentität einschließlich Float-Randmustern | P1: Ein neutral eingeschalteter EQ bricht den Nulltest; der Hostnachweis übersieht ein anderes Bitmuster. | T3-01-01; T3-09-02 | Numerische Gleichheit ersetzt an Kern- und Hostgrenze die geforderte Erhaltung der Samplebits. | Neutralen stabilen Pfad ohne verlustbehafteten Zahlenrücklauf führen und R1 mit Bitvergleich bewerten. | A16/B6 und A35/R1: Subnormale, +0/−0 und normale endliche Muster durch vollständigen SondeProcessor; R2-Bitprüfung als vorhandenes Muster. | 4–6: DspKern.cpp, SondeProcessor.cpp, SondeNullTestMain.cpp, tools/fl/nulltest.py, Selbsttest |
| B10 — State-Annahme und verlustfreier Rückweg | P1: Heute kann der Writer einen angenommenen State für den eigenen Reader unlesbar machen. | T3-02-02; T3-02-05 | Annahmegrenzen, Writer-Ergänzungen und doppelte Propertynamen haben keinen geschlossenen verlustfreien Rückweg. | Jeder angenommene Baum muss nach Writer-Ergänzungen wieder ladbar bleiben; doppelte Propertynamen nach geklärter Regel verlustfrei erhalten oder vor mutierbarem Laden abweisen. | A12/B2/B14: 65 536 Common-Properties mit fehlendem Pflichtwert, Save→Load, doppelte Namen mit verschiedenen Werten und unveränderte Originalbytes. | 3–5: NakamaState.cpp, State-Reader/Writer-Gegenfälle, Fixtures |
| B11 — Revisionen: ein Wertebereich, atomare Annahme | P1: Vertragsgrenzen werden beim Speichern, Handeln und Bestätigen verschieden ausgelegt. | T3-02-01; T3-02-03; T3-03-03 | Persistenz, Assistenten-Inkrement und ACK-Verbrauch benutzen unterschiedliche Revisionsgrenzen und Fehlersemantik. | Zulässigen Bereich vor Nebenwirkung prüfen und gemeinsam für Inkrement, Wire und ACK verwenden; Grenzfehler dürfen keinen Erfolg erzeugen. | A5/A12/B2/B3c/B10/A4: 2^53−1, 2^53, INT64_MAX und Überlauftext; kein begonnener Schritt/entfernter Inflight bei Fehler. | 5–9: NakamaState, prozessor/Analyse, IPC-ACK, C++/Rust-Vertrag, Schema-/State-Fixtures |
| B12 — JSON-Vertrag an jedem Produkteingang | P1: Gültige Nachrichten werden verworfen und ungültige angenommen; v2 riskiert vor dem Riegel signed overflow. | T3-03-01; T3-03-02; T3-03-04; T3-03-05; T3-03-06; T3-03-08; T3-03-09 | Die produktiven Writer und Parser umgehen dieselbe Schema-/Zahlen-/Textsemantik jeweils anders. | Geprüfte Annahmesemantik vor DTO-Übernahme vereinheitlichen: mathematische Integer, vollständiges JSON, sichere Grenzen, Unicode und Pflichtwerte; Writer daran messen und Duplikatregel festlegen. | A5/A8/A11/B3c/B10/B13/A4: identische Positiv-/Negativvektoren durch echte C++-/Rust-Reader und Writer; 1.0/1e0, Resttext, Escapes, UTF-8, NUL, null, Extremzahlen, doppelte Schlüssel. | 8–14: SourcesModel, controlclient/Intern, NakamaVertrag/Evidenz, PipeClient, Rust-bootstrap/vertrag, Fixtures und Produktreader-Tests |
| B13 — Geschlossene FlatBuffers-Zieladresse | P1: Die Zieladresse ist laut Invariante nicht additiv erweiterbar. | T3-03-07 | Generische FlatBuffers-Vorwärtskompatibilität wird auf ausdrücklich geschlossene Zieladressen angewandt. | Zusätzliche Address-Slots in beiden Lesern vor Zielauflösung ablehnen und Vertragsfixture spiegeln. | A9/A10/B3c/A4: bekannte Adresse akzeptieren, ansonsten identische Nachricht mit Extra-Slot ablehnen. | 4–6: NakamaTelemetrie, broker/src/telemetrie.rs, beide Fixture-Generatoren, Reader-Tests |
| B14 — Domain-Atomarität im Store | P1: Ein bestätigter beziehungsweise deduplizierter Befehl kann seine dauerhafte Wirkung verlieren. | T3-08-01 | Group-Commit zählt Append-Aufträge statt der zusammengehörigen Befehlswirkung. | Befehl und Wirkung als unteilbare Domain-Transaktion an den Writer übergeben und zusammen wiederherstellen. | A4-SI: 63 vorherige Einträge, gepaarter Befehl/Wirkung über Grenze 64, Crash an beiden Seiten, Retry nach Restart genau einmal wirksam. | 3–5: store/writer.rs, coordinator/befehl.rs, store_crash_matrix.rs |
| B15 — Commit, Invalidierung und Veröffentlichung | P1: Lokale Queueordnung beweist weder gültigen Restore noch Reihenfolge der vorherigen Commits. | T3-08-02; T3-08-03 | Epochengültigkeit und Reihenfolge enden vor Persistenzannahme beziehungsweise vor Eintritt in die Ausgangsqueue. | Commit-/Epochennummer bis Annahme und Publikation mitführen; vor Persistenz Gültigkeit und beim Publizieren die Commitordnung sichern. | A4/A4-SI: Evidenz bekannt→invalidiert→gespeichert→Restart; N festhalten, N+1 veröffentlichen, N lösen; weder gültige Alt-Evidenz noch Entwertung des neuen Stands. | 5–8: evidenz, invalidierung, flush, store/writer, Coordinator-/Crash-Gegenfälle |
| B16 — Reload-Generation durch die gesamte Übernahme | P1: Die bereits zugesagte Reload-Sperre hat eine weitere ungeschützte Veröffentlichungsstelle. | T3-04-02; T3-07-06 | Ein vor Reload erzeugtes Resultat beziehungsweise lokaler Vergleich kann in der neuen Identität weiterwirken. | Generation bis zur letzten Sources-Übernahme prüfen; lokale Messung/Freeze entsprechend der geklärten Reload-Regel derselben Generation zuordnen. | B13/B14/A2: Invalidierung vor Reload parsen, danach übernehmen; Main-/Legacy-Reload mit alter Messung und Freeze, neue Messung bleibt gültig. | 4–7: SourcesModel, prozessor/State/Analyse/Ipc, Reload-/Analyse-Gegenfälle |
| B17 — Gültigkeit numerischer Ergebnisse bis zum Verbraucher | P1: NaN-Ehrlichkeit und Invalidierungsregeln gelten auch nach Zwischenrechnung und Overlay. | T3-01-06; T3-01-07; T3-01-08; T3-06-02 | Überläufe und invalidierte Messungen verlieren ihre Ungültigkeitsinformation vor Zählung beziehungsweise späterem Overlay. | Zwischenergebnisse sicher berechnen und Gültigkeit samt Zähler bis Frame/LUFS/Delay-Übernahme erhalten; kein Overlay ohne gültigen Ursprung. | A2/B9/B12/B3c und AuxSpikeTest: endliche 2e19/1e38, Inf-Spike, invalidierter Frame mit vorhandener LUFS-Historie; Ausgabe verriegelt und gezählt statt scheinbar gültig. | 6–9: PluginProcessor, AnalyseEngine, AuxSpikeProcessor, FeatureEngine/Frame, Analyse-/Delay-Tests |
| B18 — PSD-Randgewichte | P1: Die versprochene Leistung und daraus abgeleitete Aktivität sind im erreichbaren Randfall falsch. | T3-06-01 | Bandintegration behandelt DC und Nyquist wie innere einseitige PSD-Bins. | Randbin-Gewichte in der Bandintegration an den festgelegten PSD-Bezug anpassen. | B5/A2: analytische DC-/Nyquist-/Innensinusleistung, Mischsignal 5A²/3-Gegenfall und Aktivität um −61 dBFS über Fenstergrößen. | 3–4: FeatureEngine/Spektrum, spektrale Goldens |
| B19 — Markierungsaktion und Rollenwechsel | P1: Ein sichtbarer Handgriff kann wirkungslos sein oder seinen Abschaltweg verlieren. | T3-07-02 | Sichtbarer Markierungsauftrag und seine Aufhebung folgen nicht derselben Rollenbedingung wie der Audiopfad. | Aktion nur für wirksame Rolle zulassen und beim Rollenwechsel laufenden Auftrag mit erreichbarem Aus-Zustand beenden. | A3/B14/B15: Legacy Solo/Puls, Wechsel nach hub bei aktivem Auftrag, Rückweg; Anzeige, Audio und Stop müssen übereinstimmen. | 3–5: PluginEditor, PluginProcessor/HoerMarkierung, Rollen-/Markierungstests |
| B20 — Userabsicht vor Passageableitung | P1: Die zugesagte Userführung wird auf dem produktiven Weg überschrieben. | T3-07-01 | Abgeleitete Passageabsicht umgeht die bestehende globale Userpriorität. | Den verbindlichen Prioritätsentscheid im produktiven State-/Brokerpfad benutzen. | B27/A4: globale Userabsicht und Veto gegen abweichende abgeleitete Passage; Save/Load und Brokerübernahme eingeschlossen. | 3–5: NakamaState, coordinator/intent, Intent-/Verdrahtungstests |
| B21 — PRE/POST-Paarung produktiv und lebenslauffest | P1: Die zugesagte Paarfunktion ist heute produktiv unerreichbar; ihr Anschluss aktiviert zwei bekannte Komponentenfenster. | T3-08-04 | Pair-ID erreicht den produktiven Descriptor nicht; isolierte Tests setzen die Vorbedingung selbst. | Persistente Paarmetadaten versioniert bis Descriptor/Coordinator führen und bei Anschluss zugleich Disconnect-/Epochenneuprüfung schließen. | A5/A8/B10/B24/A4/A24: echte Sonde-Anmeldung mit pair_id ohne Testsetter, Paarurteil, Disconnect/Stale und alte Aufnahme nach Invalidierung. | 6–10: Sonde-Statebericht/Handshake, Vertrag/Fixtures, liveness, prepost_verdrahtung, Integration |
| B22 — Transportepoche versus gültiger Loop | P1: Ein zulässiger Loop invalidiert den Befund entgegen der bestehenden Matrix. | T3-08-05 | Der allgemeine Epochenwechselpfad benutzt die bereits definierte Loop-Ausnahme nicht. | Produktive Invalidierung an dieselbe Passage-/Loop-Entscheidung binden wie den geprüften Helfer. | A4/B23/A24: Schleifenrücksprung an Passagegrenze erhält erlaubte Evidenz, tatsächlicher Seek außerhalb invalidiert. | 3–5: invalidierung_verdrahtung, invalidierung, Feature-Zeit-/Passage-Gegenfälle |
| B23 — Entfernen einer nicht mehr verbundenen Quelle | P1: Ein vorhandener Bedienhandgriff erreicht seine zugesagte Wirkung nicht. | T3-07-04 | Persistente Quellenverwaltung hängt an einer veralteten Live-Zielnonce und verschluckt den negativen ACK. | Persistentes Entfernen vom Live-Ziel trennen beziehungsweise korrekt adressieren und jeden Reject ehrlich abschließen. | B13/B14/B10/A4: offline nach echter Verbindung, alte Nonce, unknown_target, erfolgreicher Remove und Reload; nie stiller Erfolg. | 4–6: SourcesModel, prozessor/Ipc, coordinator/befehl, Sources-/Reloadtests |
| B24 — Hostbypass und Gen-reset | P2: Vor weiterem EQ-Bau muss die Wirkung dieser normalen Hostaktionen feststehen. | T3-01-09; T3-01-10 | Host-Lebenslaufübergänge haben keine vollständige Regel für volatile Historie und Markierungsaufträge. | Entschiedene Bypass-/Reset-Regeln am Hosteintritt einschließlich Wiederanschluss implementieren; persistenten State erhalten. | A1/A3/A16/B6/B14: Hostbypass A→B→A, reset während Markierung, Folgesample und Interventionszeitraum nach definierter Regel. | 4–6: Gen-/Sonde-Prozessoren, HoerMarkierung, Host-/Markierungstests |
| B25 — Rollenwechsel erhält eine einheitliche State-Regel | P2: Neue Main-EQ-Daten dürfen nicht auf einer ungeklärten Rollen-Lebensdauer aufsetzen. | T3-02-06 | Live-Rückwechsel und serialisierter Rückweg besitzen verschiedene MainProject-Bestände. | Die beschlossene Erhalten-/Löschen-Regel symmetrisch für Wechsel, Save und Load umsetzen. | A12/B2/B14: alle sechs Main-Sammlungen, Main→Legacy→Main unmittelbar und nach Reload; fremde Daten bleiben erhalten. | 3–5: prozessor/State, NakamaState, Rollenfixtures/-Tests |
| B26 — Operative Bedeutung von binary_telemetry | P2: Vor neuer EQ-Telemetrie muss das bestehende Capabilitysignal verbindlich sein. | T3-03-10 | Gemeldeter Unterstützungsstand und produktive Wahl des Telemetrieformats haben keinen gemeinsamen Entscheid. | Bedeutung des Bits und Formatwahl gemäß bestehender JSON-Rückfallzusage verbinden und versioniert beidseitig prüfen. | A13/A5/B10/A4: unsupported/supported samt tatsächlich gesendetem Format und Reconnect; keine erfundene Hostfähigkeit. | 4–7: controlclient/Intern, TelemetryClient, Rust-senke, Capabilityvertrag/-Fixtures |
| B27 — Statistische Auflösung schmaler Änderungen | P2: Vor weiterer EQ-Wirkungsbewertung darf geringe Auflösung nicht still als Unverändert gelten. | T3-06-03 | Bootstrap-Auflösung und Mehrfachtestschwelle passen nicht zu jeder gültigen Bandfamilie. | Nach festgelegter kleinster Aussagebreite Auflösung/FDR abstimmen oder unentscheidbare Fälle ausdrücklich kennzeichnen. | B25/A4/A26: tatsächliche 400 Ziehungen in 221-/64-Bandfamilien, eine schmale bekannte Änderung und Nullfall; keine unerreichbaren handgesetzten p-Werte. | 3–6: experiment, experiment_verdrahtung, Experiment-Goldens/Korpus |
| B28 — P4-Kalibrierung misst Produktresultate | P3: Das Releasegate braucht Wirksamkeitsmessung, nicht nur konsistente Datenherstellung. | T3-09-03 | P4-Qualitätszahlen vergleichen vorbereitete Annotationen statt den zu bewertenden Algorithmus. | Tatsächlichen Produktoutput mit unabhängiger Wahrheit verbinden; Generator-Determinismus als getrennte Aussage erhalten. | A25/A26/A27 und B25/A4: bewusst falscher Produktbefund muss Brier/Abdeckung verschlechtern; P5-A29 liefert das vorhandene Strukturmuster. | 4–6: P4-Generator/-Prüfer/-Selbsttest, Produktresultat-Exporter, Korpus |
| B29 — Rückwirkende LUFS-Invalidierung | P3: Vor G6 muss klar sein, welche Aussage der alte Messwert nach seiner Rücknahme noch macht. | T3-07-03 | Rücknahme gültiger Evidenz und Altersanzeige teilen keine eindeutige Regel für das separate LUFS-Paar. | Nach Entscheidung LUFS-Gültigkeit an Evidenzrücknahme binden oder den ausdrücklich erlaubten Altzustand konsistent tragen. | B12/B13/B15: valid→invalidate→Timer/Disconnect→neuer gültiger Frame; kein ungewolltes Wiedererscheinen. | 3–5: SourcesModel, PluginEditor, Sources-/Loudness-Gegenfälle |
| B30 — Diagnosezugriff mit definierter Berechtigung | P1: Die benannte Sicherheitslücke muss vor weiteren automatischen Diagnose-/Installationsläufen geklärt werden. | T3-12-04 | Die Diagnose-ID ist ein Dateinamensformat, aber kein Berechtigungsnachweis für Messwerte und Tokeninhalt. | Diagnoseanfrage an eine kontrollierte Berechtigung binden und wiederverwendbare Token aus Antworten entfernen oder maskieren. | B30/A4/A35: gültiger Dateiname ohne Berechtigung erhält keine Diagnose; berechtigter Lauf bleibt messbar; Antworten ohne vollständiges v2-Token. | 4–7: C++-/Rust-Briefkasten, Diagnose-Writer, tools/fl-Client, Briefkasten-Gegenfälle |
| B31 — Alle angenommenen Quellen erreichbar | P3: Vor G6 darf die Annahme einer Quelle ihren technischen Bedienzugriff nicht verlieren. | T3-07-05 | Modellkapazität und technische Erreichbarkeit der Quellen besitzen verschiedene Grenzen. | Entschiedene Erreichbarkeits- oder Annahmegrenze umsetzen, ohne vorhandene persistente Quellen zu verlieren. | B13/B15: Quelle 16/20/21/64 sowie Mehrfachanmeldung; jedes angenommene Ziel erreichbar oder Annahme ausdrücklich begrenzt. | 2–4: SourcesModel, PluginEditor, Sources-/Shot-Gegenfälle |
| B32 — Junction-Voraussetzung sichtbar im Prüfergebnis | P3: Vor G6 darf ein Sicherheitsgegenpfad ohne Messung keinen grünen Beweis liefern. | T3-09-04 | Nicht ausgeführte Gegenfälle werden als bestandene Testprozesse zusammengezählt. | Voraussetzungsmangel als eigene nicht-gemessene beziehungsweise fehlerhafte Ergebnisart bis in den Kanon tragen. | A4/A4-SI plus Runner-Selbsttest: fehlendes mklink-Recht erreicht den finalen Befund als nicht gemessen; erfolgreiche Junction läuft wirklich. | 2–4: store_crash_matrix.rs, tools/beweise.ps1, Ergebnis-Selbsttest |
| B33 — Eingefrorener Common-Bindungsvektor | P4: Eine zusätzliche Kompatibilitätswache gehört als bekannte NAK-267-Erweiterung in die Pflege. | T3-02-07 | Das Feld ist roundtripgeprüft, aber in einem unabhängigen eingefrorenen Common-Golden nicht festgehalten. | Common.project_binding_id im unabhängigen Writer-/Migrationsvektor ergänzen. | A12/B2/B14: eingefrorene Bytes und Gegenwert; vorhandenen Roundtrip nicht als neuen unabhängigen Beweis zählen. | 2–4: State-Fixturegenerator, Fixture, B2-Golden |
| B34 — Welcome-Minor-Grenze | P4: Der Skeptiker bestätigt Härtung, keine heutige zerstörende unbekannte Nutzlast. | T3-03-11 | Zwei Welcome-Verbraucher haben die Envelope-Minor-Grenze anderer Verbraucher nicht übernommen. | Zulässige Welcome-Minors beidseitig explizit prüfen und Altversionen erhalten. | A5/B10: höchste erlaubte Minor akzeptieren, nächste ablehnen; Control und Telemetrie gleich behandeln. | 2–4: IpcVerbindung, TelemetryClient, IPC-Fixtures/-Tests |
| B35 — Diagnoseabschluss nach Stop begrenzen | P4: Absichtskommentar und M-45 tragen die Herabstufung auf Härtung. | T3-04-04 | Ein bereits begonnener Diagnoseauftrag darf nach der Join-Frist fertig werden, besitzt aber keinen gezielten Abschlussgegenfall. | Erlaubten Restabschluss und seine Ressourcen-/Zeitgrenze explizit beweisen, ohne daraus ungefragt ein Schreibverbot abzuleiten. | A4 briefkasten: Schreiben über Stop/Join hinaus halten, Abschluss beziehungsweise Freigabe beobachten; kein neuer Takt nach Stop. | 2–3: broker/src/briefkasten.rs, Briefkasten-Lebenslauftests |
| B36 — Fingerprint-Phaseninvarianz wirklich vergleichen | P4: Ein konkretes fehlendes Oracle rechtfertigt einen kleinen Pflegeauftrag. | T3-09-05 | Ein als Phasenvergleich beschrifteter Erfolgsmarker vergleicht das Gegenresultat nicht. | Vorzeicheninvertiertes Gegenresultat erzeugen und die zugesagte Invarianz numerisch prüfen. | B22: Original gegen invertiertes Signal; gezielter nicht äquivalenter Fehler muss die Assertion brechen. | 1–2: Sonde013FingerprintGoldenTest.cpp, gegebenenfalls getrennte Gegenfixture |

### Geordnete Abarbeitung ab 18.09.2026

Der Dirigent validiert die Mitglieder an den angegebenen Quellen, gleicht den aktuellen Registerstand ab und vergibt erst dann NAK-/Ticketnummern.
**Jedes der folgenden 36 Bündel: Vorschlag T2 mit Codex-Erstprüfung**, Quellenvalidierung vor Bau, eine Matrixzeile je Mitglied, gezielte Rot-/Grünbelege am Produkteingang und passende Integrationsbeine.
Bekannte Punkte bleiben unter ihrer NAK-Nummer; das Bündel repariert nur den in der T3-Zeile belegten Zusatz.
Die Liste ist eine zulässige Reihenfolge; „nach“ benennt die sachliche Abhängigkeit zusätzlich zur gemeinsamen Beweisgrundlage B01.

1. **B01 Frische der tatsächlich geprüften Binaries — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 3–5: tools/beweise.ps1, Plugin-CMake, gezielter Frische-Selbsttest.
2. **B02 Vertrauensanker der erhöhten Installation — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 4–7: Aufgabenregistrierung, Installer, Manifestprüfung, Gegenpfadtests.
3. **B03 Broker-Pin über Bau und Installation — P1, T2/Codex-Erstprüfung.** Nach B01, B02. Umfang: 4–6: Plugin-CMake, tools/beweise.ps1, tools/fl/laufzeit.ps1, Manifestprüfung, B8.
4. **B06 Sonde-Callbacks besitzen ihren Lebenslauf — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 3–5: SondeProcessor.cpp/.h, IPC-Lifecycle-/Reloadtests.
5. **B07 Messpunkt-Panel: expliziter, lebender Commit — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 3–5: PluginEditor.cpp, Bindungszugriff, State-/Reload-/Shot-Gegenfälle.
6. **B09 Bitidentität einschließlich Float-Randmustern — P1, T2/Codex-Erstprüfung.** Nach B01. Umfang: 4–6: DspKern.cpp, SondeProcessor.cpp, SondeNullTestMain.cpp, tools/fl/nulltest.py, Selbsttest.
7. **B10 State-Annahme und verlustfreier Rückweg — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 3–5: NakamaState.cpp, State-Reader/Writer-Gegenfälle, Fixtures.
8. **B11 Revisionen: ein Wertebereich, atomare Annahme — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 5–9: NakamaState, prozessor/Analyse, IPC-ACK, C++/Rust-Vertrag, Schema-/State-Fixtures.
9. **B12 JSON-Vertrag an jedem Produkteingang — P1, T2/Codex-Erstprüfung.** Nach B11. Umfang: 8–14: SourcesModel, controlclient/Intern, NakamaVertrag/Evidenz, PipeClient, Rust-bootstrap/vertrag, Fixtures und Produktreader-Tests.
10. **B13 Geschlossene FlatBuffers-Zieladresse — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 4–6: NakamaTelemetrie, broker/src/telemetrie.rs, beide Fixture-Generatoren, Reader-Tests.
11. **B04 Authentisierte Verbindungsidentität — P1, T2/Codex-Erstprüfung.** Nach B12. Umfang: 6–10: server_v3/auth/bootstrap/verbindung, Coordinator-Link, v2-Tokenpfad, C++-Handshake, Verträge/Fixtures.
12. **B05 v2 darf v3 nicht stilllegen — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 4–7: broker/src/lebenslauf.rs, server.rs, framing.rs, Lifecycle-/Lasttests.
13. **B14 Domain-Atomarität im Store — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 3–5: store/writer.rs, coordinator/befehl.rs, store_crash_matrix.rs.
14. **B15 Commit, Invalidierung und Veröffentlichung — P1, T2/Codex-Erstprüfung.** Nach B14. Umfang: 5–8: evidenz, invalidierung, flush, store/writer, Coordinator-/Crash-Gegenfälle.
15. **B16 Reload-Generation durch die gesamte Übernahme — P1, T2/Codex-Erstprüfung.** Nach B15. Umfang: 4–7: SourcesModel, prozessor/State/Analyse/Ipc, Reload-/Analyse-Gegenfälle.
16. **B08 Hostereignisse am Audio- und Reload-Rand — P1, T2/Codex-Erstprüfung.** Nach B06, B09, B11. Umfang: 8–14: SondeProcessor, Hostparameter-Anbindung, Transaktionskern/DspKern, B6/B7, Wrapper-Prüfzugriff.
17. **B17 Gültigkeit numerischer Ergebnisse bis zum Verbraucher — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 6–9: PluginProcessor, AnalyseEngine, AuxSpikeProcessor, FeatureEngine/Frame, Analyse-/Delay-Tests.
18. **B18 PSD-Randgewichte — P1, T2/Codex-Erstprüfung.** Nach B17. Umfang: 3–4: FeatureEngine/Spektrum, spektrale Goldens.
19. **B19 Markierungsaktion und Rollenwechsel — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 3–5: PluginEditor, PluginProcessor/HoerMarkierung, Rollen-/Markierungstests.
20. **B20 Userabsicht vor Passageableitung — P1, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 3–5: NakamaState, coordinator/intent, Intent-/Verdrahtungstests.
21. **B21 PRE/POST-Paarung produktiv und lebenslauffest — P1, T2/Codex-Erstprüfung.** Nach B12, B13, B15, B16. Umfang: 6–10: Sonde-Statebericht/Handshake, Vertrag/Fixtures, liveness, prepost_verdrahtung, Integration.
22. **B22 Transportepoche versus gültiger Loop — P1, T2/Codex-Erstprüfung.** Nach B15. Umfang: 3–5: invalidierung_verdrahtung, invalidierung, Feature-Zeit-/Passage-Gegenfälle.
23. **B23 Entfernen einer nicht mehr verbundenen Quelle — P1, T2/Codex-Erstprüfung.** Nach B04, B15, B16. Umfang: 4–6: SourcesModel, prozessor/Ipc, coordinator/befehl, Sources-/Reloadtests.
24. **B30 Diagnosezugriff mit definierter Berechtigung — P1, T2/Codex-Erstprüfung.** Nach B04. Umfang: 4–7: C++-/Rust-Briefkasten, Diagnose-Writer, tools/fl-Client, Briefkasten-Gegenfälle.
25. **B24 Hostbypass und Gen-reset — P2, T2/Codex-Erstprüfung.** Nach B08, B16, B19. Umfang: 4–6: Gen-/Sonde-Prozessoren, HoerMarkierung, Host-/Markierungstests.
26. **B25 Rollenwechsel erhält eine einheitliche State-Regel — P2, T2/Codex-Erstprüfung.** Nach B10, B16. Umfang: 3–5: prozessor/State, NakamaState, Rollenfixtures/-Tests.
27. **B26 Operative Bedeutung von binary_telemetry — P2, T2/Codex-Erstprüfung.** Nach B12, B13. Umfang: 4–7: controlclient/Intern, TelemetryClient, Rust-senke, Capabilityvertrag/-Fixtures.
28. **B27 Statistische Auflösung schmaler Änderungen — P2, T2/Codex-Erstprüfung.** Nach B17, B18. Umfang: 3–6: experiment, experiment_verdrahtung, Experiment-Goldens/Korpus.
29. **B28 P4-Kalibrierung misst Produktresultate — P3, T2/Codex-Erstprüfung.** Nach B17, B18, B27. Umfang: 4–6: P4-Generator/-Prüfer/-Selbsttest, Produktresultat-Exporter, Korpus.
30. **B29 Rückwirkende LUFS-Invalidierung — P3, T2/Codex-Erstprüfung.** Nach B16, B17. Umfang: 3–5: SourcesModel, PluginEditor, Sources-/Loudness-Gegenfälle.
31. **B31 Alle angenommenen Quellen erreichbar — P3, T2/Codex-Erstprüfung.** Nach B23. Umfang: 2–4: SourcesModel, PluginEditor, Sources-/Shot-Gegenfälle.
32. **B32 Junction-Voraussetzung sichtbar im Prüfergebnis — P3, T2/Codex-Erstprüfung.** Nach B01. Umfang: 2–4: store_crash_matrix.rs, tools/beweise.ps1, Ergebnis-Selbsttest.
33. **B33 Eingefrorener Common-Bindungsvektor — P4, T2/Codex-Erstprüfung.** Nach B10. Umfang: 2–4: State-Fixturegenerator, Fixture, B2-Golden.
34. **B34 Welcome-Minor-Grenze — P4, T2/Codex-Erstprüfung.** Nach B12. Umfang: 2–4: IpcVerbindung, TelemetryClient, IPC-Fixtures/-Tests.
35. **B35 Diagnoseabschluss nach Stop begrenzen — P4, T2/Codex-Erstprüfung.** Nach B30. Umfang: 2–3: broker/src/briefkasten.rs, Briefkasten-Lebenslauftests.
36. **B36 Fingerprint-Phaseninvarianz wirklich vergleichen — P4, T2/Codex-Erstprüfung.** Keine zusätzliche Bündelabhängigkeit. Umfang: 1–2: Sonde013FingerprintGoldenTest.cpp, gegebenenfalls getrennte Gegenfixture.

**Anschluss an den bestehenden Plan:** P1 zuerst, einschließlich sicherer Installation vor einem neuen FL-Nachweis.
S25g/NAK-289 Etappe 1 ist bereits durch Phase 00 abgenommen; der verbleibende tidy-Abbau bleibt ein eigener verhaltensneutraler Auftrag und nimmt keine der Produktreparaturen verdeckt auf.
Vor der Fortsetzung von **S26–28** und dem neuen Master-Pfad **S28b**: P1/P2 und die K3-/K4-Detektoren aus Aufbau-Schritt 4 schließen; vorhandener EQ-Code und B6/B7 werden dabei ausdrücklich als Bestand behandelt.
Vor **G6**: P3, Echtzeit-Tor, gezielte Mutanten und die gemeinsame Laufzeitkette schließen; G6 bleibt **T3**, mit dem geltenden Prüfsystem-Audit vor dem Codex-Gate-Audit.
U39 hat den früheren User-Handgriff `/code-review ultra` ersetzt; dieser Auftrag reaktiviert ihn nicht.
**S29–31** baut auf Identitätsbindung, verlässlichem State, Paarung und ACK-/Reload-Ordnung auf; das weiterhin bekannte NAK-269 gehört spätestens vor seiner neuen Transport-/Fernsteuerungsnutzung an die Grenze.
**S31c/S35b** nehmen P4 sowie NAK-292/298/299 nach ihrem bestehenden Pflegeauftrag auf; P4 darf als begründete Ergänzung früher an ein passendes Bündel angeschlossen werden.
**S32–33** behält die eigene offene Aux-/PDC-Voraussetzung U11/NAK-44/79; ein Nulltest ersetzt sie nicht.
**S34–35** behält reale Signierung, Migration und Rückweg; NAK-30-Identität bleibt eingefroren und NAK-119 ist keine Erlaubnis für B02s offene Privileggrenze.
Planbasis: `docs/plan/plan.json`, Schritte S25g, S26–28, S28b, G6, S29–31, S31c, S32–33, S34–35, S35b.

### Detektorlücken und Aufbauplan des Prüfsystems

`docs/gesundheit/KONZEPT.md:606` ordnet die Arbeiten: **2 Prüfgang/NAK-260 → 5 Mutanten/NAK-263 und 7 Echtzeit-Tor/NAK-265**; **4 Zustand/Vertrag/NAK-262 vor S26–28**; **6 Wächter/NAK-264**, **8 Zwecktreue/NAK-266**, **9 Bedien-/Laufzeit-/Gate-Audit**.
Der Befundscan dieses Laufs findet unter `tools/pruefung/` nur `codex-audit-lauf.ps1`, `tiefenaudit_phase.py`, `auftrag-lebenslauf-audit.md`; der dort geplante `pruefgang.py` und `.claude/agents/` fehlen.
Damit ist ein Auditstarter vorhanden, aber die geplante Detektorkette nicht vollständig eingebaut.
Schritt 9 ist **teilweise vorhanden**: der Laufzeit-Arm NAK-286 ist abgenommen; die noch fehlende Verbindung seiner Messungen mit den folgenden Gegenfällen bleibt eigener Auftrag.
Phasen 02/03/07 liefern bereits inhaltliche Populationen für Schritte 4/8, sind aber allein kein regelmäßig laufendes Tor.

| Klasse | Bündel / Register | Aufbauschritt | Kleinstes wirksames Bein | Heute fehlende Aussage |
| --- | --- | --- | --- | --- |
| K1 Rückstau/Ordnung | B05, B15 | 2 Prüfgang (NAK-260), 6 Beziehungswächter (NAK-264), 5 Mutanten (NAK-263) | A4/A4-SI mit Barrieren vor Queueeintritt: N+1 vor N; v2-Half-Hello/Leerlauf bis Cap in A22. Verlust und maximale Belegung als Ergebnis prüfen. | Queue-Lokaltests besitzen keinen Beweis über vorangehende Commitordnung oder v2-Fristen. |
| K2 Lebenslauf/Besitz | B05–B07, B16, B35 | 3 vorhandene Vorlage weiterverwenden; 2/6/5 ergänzen | B10/B14 hält jeden realen Sonde-Provider über Detach und Besitzerabbau; Panel-Abbau und begonnener Briefkastenschreibweg separat halten. | Gen-Schleuse und reiner API-Stop-Test erkennen keinen neuen rohen Besitzer in der Sonde. |
| K3 Vertrag/Zahlen | B11–B13, B17–B18, B26–B27, B34 | 4 Zustand/Vertrag (NAK-262), 6 Vertragswächter, 5 Mutanten | Ein Vektorsatz durch die echten C++-/Rust-Produkteingänge in B10/B13/A4 plus A5/A8/A10; analytische Zahlenränder in B5/B25. | SchemaTest allein prüft nicht die daneben laufenden Produktparser; mathematisch unmögliche Test-p-Werte messen die Pipeline nicht. |
| K4 Zustand/Paarung | B07–B08, B10–B11, B14–B16, B20–B22, B25, B33 | 4 Zustandstreue, 6 Zustands-/Beziehungswächter, 5 Mutanten | B2/B7/B14: Writer ergänzt am Annahmelimit, Automation wartet während Load; A4-SI: Domainpaar über Commitgrenze und Restore nach Invalidierung; B24/A24 echte Paaranmeldung. | Feldinventar plus Roundtrip ohne Mutation oder identische neue Testinstanz decken diese Übergänge nicht ab. |
| K5 Behauptung/Messung | B01, B09, B28, B32, B36 | 2 Prüfgang mit Frische/Vakuum und 5 Mutanten; 9 Laufzeit-Arm erweitern | Runner-Selbsttest für fehlende DSP-Abhängigkeit und nicht gemessene Junction; A35/R1 Bitmuster; A26 tatsächlicher falscher Produktoutput; B22 Gegenresultat. | Ein grüner Prozess, identische Generatorbytes oder eine konstante Erfolgsassertion sind keine Messung der benannten Eigenschaft. |
| K6 Echtzeit | B08–B09, B24 | 2 Tor zunächst HINWEIS, 7 Kalibrierung zur GRENZE (NAK-265), 6 Echtzeitwächter | Wrapper-Eintritt vor processParameterChanges instrumentieren: Locks/Allokationen einschließlich malloc; B6/B7 identische Automation mit veränderten Workerzeiten und erster Offline-Block. | Die heutige DspKern-RtWache beginnt zu spät; NAK-267 A-4/A-5 und NAK-48 sind bekannte zusätzliche Grenzen, keine neuen Phase-11-Befunde. |
| K7 Größe/Kontext | 0 neue T3-Befunde; bekannt NAK-292/298/299 | 2 W-Riss/Clippy-Anschluss; S31c/S35b; S25g getrennt | Vorhandene A32/A33 samt Inventar und registrierten Rissen verbrauchen; Clippy-Messung ausdrücklich zuordnen. | Null neue K7-Befunde ist keine neue Gesundmessung; bestehende Pflegeschritte werden nicht zu Produktreparaturen umetikettiert. |
| K8 Bedienehrlichkeit | B19, B23, B29, B31 | 8 Zwecktreue (NAK-266), 9 technische Bedien-/Laufzeitprüfung (NAK-281/282/286), 6 Beziehungswächter | B13/B14/B15 beziehungsweise vorhandener FL-Arm: echter Handgriff, ACK/Reject, Modell und sichtbarer Zustand; Quelle 21 und Rollenwechsel. | Screenshot allein belegt keine Wirkung, Modelltest allein keine erreichbare Aktion; geparktes Design bleibt außerhalb. |
| K-neu Privileg-/Diagnosegrenze; K4 Identitätsbindung | B02, B04, B30 | 2 Sicherheitsprüfung einhängen; 6 um ausdrücklichen Vertrauensgrenzen-Auftrag ergänzen; 9 Runtime nach sicherem Installationspfad | A17/A18 prüfen unabhängig geschützte Aufgabenaktion; A4 security_vectors realer Peer gegen behauptete Identität; B30/A35 unberechtigte Diagnose und Tokenfreiheit. | Die vier geplanten Wächter haben keinen expliziten Besitzer für eine erhöhte Skriptkette; Pipe-ACL und Hash gegen eigenes Manifest ersetzen diesen nicht. |

Ein Wächter liefert einen lesenden Gegencheck, keinen Laufzeitbeweis.
Ein statischer Treffer auf eine Sperre ist zunächst HINWEIS; erst nach Aufrufgraph, kalibrierten Gegenfällen und sauberer Ausnahmeliste trägt er GRENZE.
Die Mutanten müssen die späteren Matrixzeilen am produktiven Weg töten; ein Testhelfer, der die im Produkt fehlende Pair-ID oder einen unerreichbaren p-Wert selbst setzt, bleibt eine Beweislücke.
Diese Detektortickets ergänzen die Reparaturen; die P1-Arbeit wartet nicht auf den vollständigen Bau des gesamten Prüfsystems.

### Blind Spots und nach Reparatur verbleibende Nahtstellen

1. **Bekannt NAK-269 bleibt eigenständig:** B15 schließt Commit → Writer/Queue, jedoch fehlt weiterhin ein Alters-/Ordinalvergleich im C++-Snapshot-Verbraucher. `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1070` trägt kein entsprechendes Feld; `SourcesModel.cpp:1235` prüft Sitzung/Bindung und übernimmt bei `:1335` den Bestand. Eine gleiche Sitzung ist kein Monotonienachweis. Der Registerpunkt ist Härtung, kein neuer Phase-11-Defekt; kleinster dauerhafter Schutz: versionierte Ordnungsmarke und N+1→N-Gegenfall in A5/A8/B3c/B13 sowie Brokerintegration.
2. **Host → Wrapper → Worker → Reload → Broker → Anzeige ist als gemeinsame Kette ungemessen.** Phase 01/05 lesen die Hostseite, 04/08 die Lebensläufe, 07 den Verbraucher, 09 die Beweise. A24 benutzt echte Sonde/Broker/SourcesModel, jedoch keinen echten FL-VST3-Wrapper; A35 hat Selbsttests, R1 misst Hostaudio. Ein nach allen Einzelreparaturen verbleibender Detektor braucht ein gemeinsames Szenario mit echter Automation, erstem Offline-Block, Same-Instance-Reload, Brokerneustart, Invalidierung und ehrlicher Anzeige. Testbroker bleiben auf Probe-Pipes; keine Produktions-Pipe für E2E.
3. **Externer FL-MCP-Server und Controller:** `tools/fl/` wurde gelesen, die Gegenimplementierung im separaten `fl-studio-mcp`-Repo gehört nicht zu einem dokumentierten Inhaltsscope dieser Phasen. Dessen tatsächlicher Pin, Antwortsemantik und Boot-/Projektidentität sind deshalb eine einseitig gelesene Naht. Die externe Audio-Referenz aus Phase 06 ist dagegen ausdrücklich gelesen worden und wird nicht als Lücke behauptet.
4. **Bau-/Werkzeugkette ohne dokumentierte eigene Inhaltslektüre:** `broker/Cargo.lock`; `eq-copilot/cmake/NakamaFlatBuffers.cmake`, `NakamaFlatBuffersProjektHook.cmake`, `NakamaFlatcBeleg.cmake`, `NakamaKern.cmake`, `pruefe_nakama_kern_genex.cmake`; `eq-copilot/plugin/probe/PipeProbeMain.cpp`. Diese sieben Dateien fehlen auch als Dateiname in allen elf Rohberichten. Das belegt eine Scope-Lücke, keinen Fehler in ihnen. In einer Inventarpopulation von 297 getrackten Pfaden finden sich insgesamt 14 solcher nicht ausdrücklich genannten Dateien; die sieben anderen sind vier kleine Diagnose-/Identitäts-/Stilheader und drei Referenz-/Kontextwerkzeuge, siehe Scope-Beweis im Synthesebericht.
5. **Inventar ist keine vollständige Körperlektüre:** Phase 03 zählt 641 Pfade mit verschiedenen Lesemodi; Phase 08 unterscheidet bei 112 Rust-Dateien 74 Körper-/Quellbereiche von 38 reinen Suchzugriffen. Spätere Phasen überdecken Teile davon, insbesondere Tests und Sicherheit, aber daraus folgt keine vollständige Zeilenlektüre des Brokers oder seiner Abhängigkeiten. Die 11 Scope-Beweise werden deshalb nicht zu einer Repository-Prozentzahl addiert.
6. **Aktivierte Vorbedingungen brauchen beide Seiten:** Der Pair-ID-Fix B21 macht erst die in Phase 08 V01/V02 beschriebenen Disconnect-/Stale- und verspäteten PRE/POST-Aufnahmen erreichbar. Diese Fälle gehören in dieselbe Matrix, obwohl sie am heutigen HEAD zurecht keine zusätzlichen Befunde sind. Dasselbe gilt für B04/B30: eine geschlossene ACL allein beweist weder Rollenautorisation noch Diagnoseberechtigung.
7. **Laufzeit-/OS-Grenzen bleiben offen:** Keine Phase dieses Leselaufs hat reale Task-ACLs, erhöhte Ausführung, Host-Scheduling, Crash/Stromverlust, Racefreiheit oder Worst-Case-CPU frisch gemessen. B02s Quellenbefund bleibt gültig; er ist kein behaupteter ausgeführter Exploit. Vollständige JUCE-/VST3-SDK-/SQLite-/Toolchain-Audits fehlen ebenfalls.
8. **Noch nicht angeschlossener Produktumfang** S28b, S29–31 und spätere Sidechain-/Releasearbeit kann nur gegen Verträge, nicht gegen eine fertige Laufzeit geprüft sein. Reservierte Preview-/Draft-Wege und geparktes Design werden nicht als heutige fehlende Controls beanstandet.

**Weiterer Leselauf, falls der Dirigent die Restfläche schließen will:** keine neue „Phase 12“, denn diese Nummer ist mit der erhaltenen Pipe-Sicherheitsprüfung belegt.
Vorschlag **Phase 13 — Bau- und Hostnachweiskette**: die sieben Dateien aus Punkt 4, die externen FL-MCP-Server-/Controllerdateien am festgehaltenen Commit und ihre Aufrufpaare zu `tools/fl/` vollständig inventarisieren und lesen; Grenzen von Build-Pin, Codegen-Frische, Probe-Pipe und Hostantwort jeweils beidseitig nachweisen.
Das ersetzt die anschließend nötigen Host-/OS-Messungen nicht und erweitert diesen Lauf nicht um Ausführung oder fremde Schreibziele.

### Produktfragen als Kartenentwürfe

Die Kennungen PF11-* sind lokale Übergabekarten, keine NAK- oder U-Nummern.
**Default bedeutet Vorschlag, nicht beschlossene Produktregel.** Technische Vertrags- und Mechanikfragen entscheidet der Dirigent nach Quellenvalidierung; nur echte Produktwahl wird als solche vorgelegt.
Es wurde weder `fragen.json` geändert noch eine Frage an den User gestellt.
Bestehende offene/geparkte Karten bleiben bestehen; insbesondere U11 und die geparkte Designrichtung werden nicht neu entschieden.
Alle Frageabschnitte von 01–10 und 12 sind abgebildet; Phasen 05/08/09/12 melden keine neuen Produktfragen.

**PF11-01 — Hostbypass und Wiedereinstieg**

- **Herkunft / Auftrag:** 01:P01; 10:P10-01; B24.
- **Was:** Welche Fade- und Historienregel gilt für den VST3-Hostbypass?
- **Optionen:** Wie global.bypass; eigener ausdrücklich definierter Hostübergang.
- **Default:** Gleiche Übergangsregel wie global.bypass, persistenten State erhalten; vor B24 bestätigen.

**PF11-02 — Gen-reset bei aktiver Markierung**

- **Herkunft / Auftrag:** 01:P02; 10:P10-01; B24.
- **Was:** Was überlebt reset: volatile Audiohistorie, Markierungsauftrag, Interventionszeitraum?
- **Optionen:** Nur Historie leeren; zusätzlich Auftrag und Zeitraum sauber beenden.
- **Default:** Volatile Historie leeren und hörbaren Auftrag kontrolliert beenden, Parameter/State behalten; Entscheidung erforderlich.

**PF11-03 — Main-Bestände beim Rollenwechsel**

- **Herkunft / Auftrag:** 02:P-01; 10:P10-02; B25.
- **Was:** Soll Main → Messpunkt → Main dieselben Bestände mit und ohne Reload ergeben?
- **Optionen:** Bestände behalten; bewusst löschen; separater ausdrücklicher Löschhandgriff.
- **Default:** Bestände erhalten; Löschen nur ausdrücklich, nicht als Nebenwirkung des Rollenwechsels.

**PF11-04 — Bedeutung von binary_telemetry**

- **Herkunft / Auftrag:** 03:Frage 1; 10:P10-04; B26.
- **Was:** Ist unsupported eine operative Transportsperre oder ein historischer Mess-/Freigabestatus?
- **Optionen:** Operative Sperre mit bestehendem JSON-Rückfall; getrennter Evidenzstatus mit eindeutigem Transportentscheid.
- **Default:** Operative Fähigkeit und Evidenz sauber trennen; tatsächlichen Transport nach bestätigter Regel wählen. JSON-Rückfall ist bereits §53.6, keine neue Produktoption.

**PF11-05 — Zahlenbereich von host.pid**

- **Herkunft / Auftrag:** 03:Frage 2; B12.
- **Was:** Soll der Vertrag Windows-u32-PIDs oder den bisher schemaerlaubten größeren Integerbereich annehmen?
- **Optionen:** Vertrag versioniert auf u32 festlegen; alle Leser vertragskonform für bisherigen Bereich halten.
- **Default:** Bis zur versionierten Festlegung den geltenden Bereich konsistent lesen; technische Entscheidung des Dirigenten.

**PF11-06 — Fassung im steuerkopf**

- **Herkunft / Auftrag:** 03:Frage 3; B12 / S29–31.
- **Was:** Ist steuerkopf.schema_minor eine eigene DTO-Fassung oder an die Envelope-Minor gekoppelt?
- **Optionen:** Unabhängige DTO-Fassung; gekoppelte, versioniert festgelegte Fassung.
- **Default:** Eigene DTO-Fassung explizit benennen, solange keine Kopplung zugesagt ist; technische Entscheidung, kein heutiger Defekt.

**PF11-07 — Begonnene Diagnose beim Stop**

- **Herkunft / Auftrag:** 04:P04-1; 10:Herabstufung T3-04-04; B35.
- **Was:** Darf eine bereits laufende Antwort nach der Join-Frist noch fertig werden?
- **Optionen:** Erlaubten Restabschluss begrenzen; neue strengere Abbruchregel einführen.
- **Default:** Bestehende erlaubte Restarbeit erhalten und beweisen. Durch Absichtskommentar und Skeptiker geklärt: keine erneut zu stellende Produktfrage.

**PF11-08 — Absoluter M/S-Bezug**

- **Herkunft / Auftrag:** 06:P-06-01; Zwecktreue / S26–28.
- **Was:** Welchen absoluten Bezug sagen midDb/sideDb zu?
- **Optionen:** Normierte Leistung; Leistungsdichte; nur relative Auswertung zusagen.
- **Default:** Bis zur ausdrücklichen Entscheidung nur den belegten relativen Bezug verwenden; keine absolute Pegelzusage ergänzen.

**PF11-09 — Stationäres Material ohne Onsets**

- **Herkunft / Auftrag:** 06:P-06-02; Zwecktreue / B27.
- **Was:** Bleibt ein degenerierter Onset-Vergleich eine konservative Gesamtsperre, oder erhält er einen eigenen Ergebnisgrund?
- **Optionen:** Konservative Gesamtsperre; ausdrücklich nicht beurteilbarer Onset-Teil.
- **Default:** Konservative Sperre beibehalten, Beurteilbarkeitsgrund klar benennen; bestehende Absicht nicht als Defekt umdeuten.

**PF11-10 — Lokale Messung und Freeze nach Reload**

- **Herkunft / Auftrag:** 07:PF-01; 10:P10-01; B16.
- **Was:** Dürfen Legacy-Messung, Befunde oder Freeze in einer neu geladenen Identität weiterleben?
- **Optionen:** Gemeinsame neue Messgeneration; bewusst laufübergreifende gekennzeichnete Referenz.
- **Default:** Neue Messgeneration; alte Referenz nur als ausdrücklich gewollter, separat gekennzeichneter Zustand.

**PF11-11 — Nur transportierte Modellfelder**

- **Herkunft / Auftrag:** 07:PF-02; Schritt 8 / S29–31.
- **Was:** Wer verbraucht pre_post, Rangkomponenten, Descriptor-/Paarmetadaten und Sichtbestände künftig?
- **Optionen:** Geplanten Verbraucher und Anschlussticket benennen; ausschließlich lokalen unbenutzten DTO-Bestand gezielt reduzieren.
- **Default:** Zunächst Besitzer und geplanten Anschluss benennen; keine Wirefelder unversioniert entfernen und keine geparkte Oberfläche reaktivieren.

**PF11-12 — Besitzer von Assistentenfrist und Priorität**

- **Herkunft / Auftrag:** 07:PF-03; Schritt 8 / S29–31.
- **Was:** Wer setzt timeoutMs und Schrittpriorisierung im späteren realen Workflow durch?
- **Optionen:** Laufzeitbesitzer samt Rückkante bestimmen; lokale ungenutzte Laufzeitversprechen gezielt reduzieren.
- **Default:** Vor Anschluss einen Besitzer und Abbruchweg festlegen; reserviertes P5-preview bleibt reserviert.

**PF11-13 — Rollenwahl nach hub**

- **Herkunft / Auftrag:** 07:PF-04; B19 / S29–31.
- **Was:** Ist die Rollenwahl eine einmalige Initialisierung oder technisch umkehrbar?
- **Optionen:** Ausdrücklich einmalig; klarer technischer Rückweg.
- **Default:** Umkehrbaren technischen Weg vorsehen, sofern nicht ausdrücklich einmalig beschlossen; B19 braucht dafür keine neue Designrichtung.

**PF11-14 — Doppelte Propertynamen und JSON-Schlüssel**

- **Herkunft / Auftrag:** 10:P10-03; B10 / B12.
- **Was:** Wie werden doppelte Schlüssel angenommen und fremde Werte erhalten?
- **Optionen:** Ungültig und verlustfrei abweisen; gemeinsame deterministische Auswahl-/Erhaltungsregel.
- **Default:** Wire-JSON abweisen; binären State vor mutierbarer Übernahme abweisen beziehungsweise Originalbytes read-only erhalten, niemals still einen Wert verlieren. Technik legt den kompatiblen Weg fest.

**PF11-15 — Auflösung einer schmalen Änderung**

- **Herkunft / Auftrag:** 10:P10-05; B27.
- **Was:** Welche kleinste Änderung soll in welcher gültigen Bandfamilie eine gerichtete Aussage erlauben?
- **Optionen:** Mindestbreite und statistische Auflösung festlegen; darunter ausdrücklich unentscheidbar.
- **Default:** Keine willkürliche Breite festsetzen; unter unzureichender Auflösung unentscheidbar statt unverändert. Zahlenregel vor B27 festlegen.

**PF11-16 — LUFS nach rückwirkender Rücknahme**

- **Herkunft / Auftrag:** 10:P10-06; B29.
- **Was:** Entwertet evidence_invalidate ein früher gültiges separates LUFS-I-Paar bis zu neuen gültigen Daten?
- **Optionen:** Messwert bis zu neuen Daten verriegeln; ausdrücklich alter Messwert mit Alter.
- **Default:** Verriegeln bis neue gültige Evidenz vorliegt; Vorschlag, keine aus dem heutigen Bad-frame-Vertrag erfundene Pflicht.

**PF11-17 — Quellen oberhalb der sichtbaren Zeilen**

- **Herkunft / Auftrag:** 10:P10-07; B31.
- **Was:** Was gilt oberhalb zugesagter 16 und tatsächlich erreichbarer 20 Zeilen?
- **Optionen:** Alle angenommenen Quellen technisch erreichbar halten; Annahme ausdrücklich begrenzen.
- **Default:** Angenommene Quellen erreichbar halten; vorhandenen State nie zur Anpassung an eine UI-Grenze löschen.


## Befundpopulation: vollständige Zuordnung nach Skeptiker

Die **Bündeltabelle oben ist wortgleich mit dem neuen BEFUNDE-Kopf**.
Diese zusätzliche Tabelle ist ein Rückverfolgungsindex, keine neue Befundliste und keine erneute Quellenvalidierung jedes Defekts.
Beleg, gebrochener Satz, ursprünglicher Vorschlag und Status **offen** stehen unter derselben ID im unveränderten Phasenabschnitt von BEFUNDE.md und im Rohbericht.
Die Einstufung folgt jeweils der gleichnamigen Zeile in `roh/phase-10-skeptiker.md:35` ff.; dort steht auch der jeweilige Gegencheck an Test, Register und Absicht.
Insbesondere bleiben alle „bekannt NAK-…“-Abgrenzungen erhalten: eine zusätzliche Ursache, Gegenpopulation oder fehlende Wache wird nicht als neu entdeckter alter Registerpunkt verkauft.

| ID | Schwere | Klasse | Kategorie nach Skeptiker | Bündel | Quellorte des Ursprungsbefunds |
| --- | --- | --- | --- | --- | --- |
| T3-01-01 | critical | K6 | DEFEKT | B09 | eq-copilot/plugin/sonde/SondeProcessor.cpp:337; eq-copilot/plugin/dsp/DspKern.cpp:893; eq-copilot/plugin/dsp/DspKern.cpp:1077; eq-copilot/build/_deps/juce-src/modules/juce_audio_basics/buffers/juce_FloatVectorOperations.cpp:1548 |
| T3-01-02 | high | K6 | DEFEKT | B08 | eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3687; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessorParameter.cpp:113; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/utilities/juce_AudioProcessorValueTreeState.cpp:185; eq-copilot/plugin/sonde/SondeProcessor.cpp:163 |
| T3-01-03 | high | K6 | DEFEKT | B08 | eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3892; eq-copilot/plugin/sonde/SondeProcessor.cpp:1287; eq-copilot/plugin/sonde/SondeProcessor.cpp:992; eq-copilot/plugin/sonde/SondeProcessor.cpp:1415 |
| T3-01-04 | high | K4 | DEFEKT | B08 | eq-copilot/plugin/sonde/SondeProcessor.cpp:1292; eq-copilot/plugin/dsp/DspKern.cpp:391; eq-copilot/plugin/dsp/DspKern.cpp:980; eq-copilot/plugin/dsp/DspKern.cpp:1062; eq-copilot/plugin/tests/TransactionTestMain.cpp:2219 |
| T3-01-05 | high | K6 | DEFEKT | B08 | eq-copilot/plugin/sonde/SondeProcessor.cpp:1296; eq-copilot/plugin/sonde/SondeProcessor.cpp:541; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421; eq-copilot/plugin/sonde/SondeProcessor.cpp:1443; eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4416 |
| T3-01-06 | medium | K3 | DEFEKT | B17 | eq-copilot/plugin/src/PluginProcessor.cpp:627; eq-copilot/plugin/src/PluginProcessor.cpp:633; eq-copilot/plugin/src/PluginProcessor.cpp:636; eq-copilot/plugin/src/prozessor/Ipc.cpp:1128 |
| T3-01-07 | medium | K3 | DEFEKT | B17 | eq-copilot/plugin/src/AnalyseEngine.cpp:131; eq-copilot/plugin/src/AnalyseEngine.cpp:133; eq-copilot/plugin/src/AnalyseEngine.cpp:153; eq-copilot/plugin/src/AnalyseEngine.cpp:437; eq-copilot/build/_deps/juce-src/modules/juce_dsp/frequency/juce_FFT.cpp:394 |
| T3-01-08 | medium | K5 | DEFEKT | B17 | eq-copilot/plugin/spike/AuxSpikeProcessor.cpp:195; eq-copilot/plugin/spike/AuxSpikeProcessor.cpp:206; eq-copilot/plugin/spike/AuxSpikeProcessor.cpp:278; eq-copilot/plugin/spike/AuxSpikeProcessor.cpp:16 |
| T3-01-09 | medium | K6 | LÜCKE | B24 | eq-copilot/plugin/sonde/SondeProcessor.h:139; eq-copilot/plugin/src/PluginProcessor.h:111; eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3906; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.cpp:592 |
| T3-01-10 | medium | K2 | LÜCKE | B24 | eq-copilot/plugin/src/PluginProcessor.h:111; eq-copilot/plugin/src/HoerMarkierung.h:731; eq-copilot/plugin/src/HoerMarkierung.h:753; eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3615; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.cpp:589 |
| T3-02-01 | high | K4 | DEFEKT | B11 | eq-copilot/plugin/src/prozessor/Analyse.cpp:1299; eq-copilot/plugin/src/prozessor/Analyse.cpp:1315; eq-copilot/plugin/state/NakamaState.cpp:1905 |
| T3-02-02 | critical | K4 | DEFEKT | B10 | eq-copilot/plugin/state/NakamaState.cpp:85; eq-copilot/plugin/state/NakamaState.cpp:821; eq-copilot/plugin/state/NakamaState.cpp:1116; eq-copilot/plugin/state/NakamaState.cpp:1503 |
| T3-02-03 | high | K3 | DEFEKT | B11 | eq-copilot/plugin/state/NakamaState.cpp:1696; eq-copilot/plugin/src/prozessor/Ipc.cpp:586; eq-copilot/plugin/src/prozessor/Ipc.cpp:601; eq-copilot/plugin/src/prozessor/Ipc.cpp:674; eq-copilot/plugin/core/ipc/controlclient/Intern.h:443; broker/src/vertrag.rs:244 |
| T3-02-04 | critical | K4 | DEFEKT | B07 | eq-copilot/plugin/src/PluginEditor.cpp:683; eq-copilot/plugin/src/PluginEditor.cpp:688; eq-copilot/plugin/src/PluginEditor.cpp:705; eq-copilot/plugin/src/prozessor/State.cpp:307; eq-copilot/plugin/state/NakamaState.cpp:1503 |
| T3-02-05 | medium | K4 | LÜCKE | B10 | eq-copilot/plugin/state/NakamaState.cpp:281; eq-copilot/plugin/state/NakamaState.cpp:2117; eq-copilot/build/_deps/juce-src/modules/juce_data_structures/values/juce_ValueTree.cpp:1069; eq-copilot/build/_deps/juce-src/modules/juce_core/containers/juce_NamedValueSet.cpp:187 |
| T3-02-06 | medium | K4 | LÜCKE | B25 | eq-copilot/plugin/src/prozessor/State.cpp:320; eq-copilot/plugin/state/NakamaState.cpp:1009; eq-copilot/plugin/state/NakamaState.cpp:2018 |
| T3-02-07 | low | K4 | HÄRTUNG | B33 | eq-copilot/plugin/state/NakamaState.cpp:824; eq-copilot/plugin/tests/StateMigrationTestMain.cpp:1003; eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2639; tools/eq-copilot/erzeuge_state_fixtures.py:1084 |
| T3-03-01 | high | K3 | DEFEKT | B12 | eq-copilot/plugin/vertrag/NakamaEvidenz.cpp:204; eq-copilot/schemas/v3/eq-ipc-v3.schema.json:307; eq-copilot/schemas/v3/eq-ipc-v3.schema.json:360; eq-copilot/plugin/sonde/SondeProcessor.cpp:843 |
| T3-03-02 | high | K3 | DEFEKT | B12 | broker/src/coordinator/befehl.rs:759; broker/src/coordinator/evidenz.rs:267; eq-copilot/plugin/src/SourcesModel.cpp:101; eq-copilot/plugin/core/ipc/controlclient/Intern.h:81; broker/src/transport/bootstrap.rs:242 |
| T3-03-03 | high | K3 | DEFEKT | B11 | eq-copilot/plugin/core/ipc/controlclient/Intern.h:213; eq-copilot/plugin/core/ipc/controlclient/Intern.h:244; eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:301; eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:363 |
| T3-03-04 | high | K3 | DEFEKT | B12 | eq-copilot/plugin/src/SourcesModel.cpp:640; eq-copilot/plugin/src/SourcesModel.cpp:646; eq-copilot/plugin/src/SourcesModel.cpp:1387; eq-copilot/plugin/vertrag/NakamaVertrag.cpp:563 |
| T3-03-05 | high | K3 | DEFEKT | B12 | broker/src/transport/bootstrap.rs:95; broker/src/transport/bootstrap.rs:111; broker/src/transport/bootstrap.rs:219; broker/src/transport/bootstrap.rs:265 |
| T3-03-06 | medium | K3 | DEFEKT | B12 | eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:213; eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:228; eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:450 |
| T3-03-07 | high | K3 | DEFEKT | B13 | eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:159; broker/src/telemetrie.rs:247; eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h:252; broker/src/generiert/nakama_telemetry_v1_generated.rs:515 |
| T3-03-08 | high | K3 | DEFEKT | B12 | eq-copilot/plugin/src/PipeClient.cpp:493; eq-copilot/plugin/src/PipeClient.cpp:589; eq-copilot/plugin/src/PipeClient.cpp:690; eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:263 |
| T3-03-09 | medium | K3 | LÜCKE | B12 | broker/src/coordinator/schema.rs:424; eq-copilot/plugin/src/SourcesModel.cpp:646; eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:515; broker/src/transport/bootstrap.rs:254 |
| T3-03-10 | medium | K3 | LÜCKE | B26 | eq-copilot/plugin/core/ipc/controlclient/Intern.h:401; eq-copilot/plugin/sonde/SondeProcessor.cpp:669; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:780; broker/src/coordinator/senke.rs:125 |
| T3-03-11 | low | K3 | HÄRTUNG | B34 | eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:638; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:653; eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:198 |
| T3-04-01 | critical | K2 | DEFEKT | B06 | eq-copilot/plugin/sonde/SondeProcessor.cpp:139; eq-copilot/plugin/sonde/SondeProcessor.cpp:228; eq-copilot/plugin/core/ipc/ControlClient.cpp:133; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:328 |
| T3-04-02 | medium | K2 | DEFEKT | B16 | eq-copilot/plugin/src/SourcesModel.cpp:1375; eq-copilot/plugin/src/SourcesModel.cpp:1504; eq-copilot/plugin/src/SourcesModel.cpp:479; eq-copilot/plugin/src/prozessor/Ipc.cpp:1087 |
| T3-04-03 | critical | K2 | LÜCKE | B07 | eq-copilot/plugin/src/PluginEditor.cpp:627; eq-copilot/plugin/src/PluginEditor.cpp:688; eq-copilot/plugin/src/PluginEditor.cpp:707; eq-copilot/plugin/src/PluginEditor.cpp:734 |
| T3-04-04 | low | K2 | HÄRTUNG | B35 | broker/src/briefkasten.rs:570; broker/src/briefkasten.rs:743; broker/src/briefkasten.rs:750; broker/src/briefkasten.rs:876; broker/src/briefkasten.rs:890 |
| T3-05-01 | high | K4 | DEFEKT | B08 | eq-copilot/plugin/sonde/SondeProcessor.cpp:1047; eq-copilot/plugin/sonde/SondeProcessor.cpp:1296; eq-copilot/plugin/sonde/SondeProcessor.cpp:1388; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421 |
| T3-05-02 | high | K4 | DEFEKT | B08 | eq-copilot/plugin/sonde/SondeProcessor.cpp:1047; eq-copilot/plugin/sonde/SondeProcessor.cpp:1202; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421; eq-copilot/plugin/state/NakamaTransaktion.cpp:345; eq-copilot/plugin/dsp/DspProgramm.cpp:173 |
| T3-05-03 | high | K4 | DEFEKT | B03 | eq-copilot/plugin/CMakeLists.txt:56; tools/beweise.ps1:886; tools/beweise.ps1:895; tools/fl/laufzeit.ps1:408; tools/eq-copilot/pruefe_installer_manifest.py:1150; eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:309 |
| T3-06-01 | medium | K3 | DEFEKT | B18 | eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:426; eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:449; eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:89; eq-copilot/plugin/src/AnalyseEngine.cpp:138 |
| T3-06-02 | medium | K4 | DEFEKT | B17 | eq-copilot/plugin/src/AnalyseEngine.cpp:352; eq-copilot/plugin/src/AnalyseEngine.cpp:387; eq-copilot/plugin/sonde/SondeProcessor.cpp:638; eq-copilot/plugin/core/analysis/featureengine/Frame.h:590; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:163 |
| T3-06-03 | medium | K3 | LÜCKE | B27 | broker/src/coordinator/experiment.rs:585; broker/src/coordinator/experiment.rs:1606; broker/src/coordinator/experiment.rs:417; broker/src/coordinator/experiment.rs:493; broker/src/coordinator/experiment_verdrahtung.rs:974 |
| T3-07-01 | high | K4 | DEFEKT | B20 | eq-copilot/plugin/state/NakamaState.cpp:2316; eq-copilot/plugin/state/NakamaState.cpp:2497; broker/src/coordinator/intent.rs:155 |
| T3-07-02 | high | K8 | DEFEKT | B19 | eq-copilot/plugin/src/PluginEditor.cpp:438; eq-copilot/plugin/src/PluginEditor.cpp:800; eq-copilot/plugin/src/PluginEditor.cpp:245; eq-copilot/plugin/src/PluginEditor.cpp:840; eq-copilot/plugin/src/PluginProcessor.cpp:848 |
| T3-07-03 | medium | K8 | LÜCKE | B29 | eq-copilot/plugin/src/SourcesModel.cpp:1505; eq-copilot/plugin/src/SourcesModel.cpp:1832; eq-copilot/plugin/src/PluginEditor.cpp:1299 |
| T3-07-04 | high | K8 | DEFEKT | B23 | eq-copilot/plugin/src/SourcesModel.cpp:1319; eq-copilot/plugin/src/prozessor/Ipc.cpp:1257; broker/src/coordinator/befehl.rs:549; eq-copilot/plugin/src/prozessor/Ipc.cpp:1073 |
| T3-07-05 | medium | K8 | LÜCKE | B31 | eq-copilot/plugin/src/PluginEditor.cpp:858; eq-copilot/plugin/src/PluginEditor.cpp:951; eq-copilot/plugin/src/PluginEditor.cpp:974; eq-copilot/plugin/tests/ShotTestMain.cpp:114 |
| T3-07-06 | medium | K4 | LÜCKE | B16 | eq-copilot/plugin/src/prozessor/State.cpp:167; eq-copilot/plugin/src/prozessor/Analyse.cpp:576; eq-copilot/plugin/src/PluginEditor.cpp:287; eq-copilot/plugin/src/PluginProcessor.h:1051 |
| T3-08-01 | critical | K4 | DEFEKT | B14 | broker/src/store/writer.rs:278; broker/src/coordinator/befehl.rs:409; broker/src/coordinator/befehl.rs:189 |
| T3-08-02 | high | K4 | DEFEKT | B15 | broker/src/coordinator/evidenz.rs:186; broker/src/coordinator/evidenz.rs:450; broker/src/coordinator/invalidierung_verdrahtung.rs:179; broker/src/store/writer.rs:656; broker/src/coordinator/experiment_verdrahtung.rs:1441 |
| T3-08-03 | high | K1 | DEFEKT | B15 | broker/src/coordinator/invalidierung_verdrahtung.rs:389; broker/src/coordinator/invalidierung_verdrahtung.rs:428; broker/src/coordinator/flush.rs:191; broker/src/transport/server_v3/queues.rs:228; broker/src/transport/server_v3/queues.rs:258 |
| T3-08-04 | high | K4 | DEFEKT | B21 | broker/src/coordinator/liveness.rs:741; broker/src/coordinator/liveness.rs:472; broker/src/coordinator/prepost_verdrahtung.rs:90; broker/tests/sonde013_verdrahtung.rs:1130 |
| T3-08-05 | medium | K4 | DEFEKT | B22 | broker/src/coordinator/invalidierung_verdrahtung.rs:628; broker/src/coordinator/invalidierung.rs:146; broker/tests/sonde013_taint.rs:127 |
| T3-09-01 | high | K5 | DEFEKT | B01 | tools/beweise.ps1:969; tools/beweise.ps1:1016; eq-copilot/plugin/CMakeLists.txt:199; tools/eq-copilot/pruefe_kern_identitaetsfrei.py:289 |
| T3-09-02 | medium | K5 | DEFEKT | B09 | tools/fl/nulltest.py:206; tools/fl/nulltest.py:225; tools/fl/nulltest.py:252; tools/fl/nulltest.py:616 |
| T3-09-03 | medium | K5 | LÜCKE | B28 | tools/eq-copilot/erzeuge_p4_korpus.py:51; tools/eq-copilot/erzeuge_p4_korpus.py:90; tools/eq-copilot/pruefe_p4_korpus.py:160; tools/eq-copilot/pruefe_p4_korpus.py:219 |
| T3-09-04 | medium | K5 | LÜCKE | B32 | broker/tests/store_crash_matrix.rs:3238; broker/tests/store_crash_matrix.rs:3364; tools/beweise.ps1:490; tools/beweise.ps1:498; tools/beweise.ps1:1164 |
| T3-09-05 | low | K5 | HÄRTUNG | B36 | eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp:263; eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp:270 |
| T3-12-01 | critical | K-neu: Privileggrenze | DEFEKT | B02 | tools/dirigent/install-aufgaben-registrieren.ps1:57; tools/dirigent/install-aufgaben-registrieren.ps1:66; tools/dirigent/install-aufgaben-registrieren.ps1:74; eq-copilot/install/Install-Nakama.ps1:87; eq-copilot/install/Install-Nakama.ps1:101; eq-copilot/install/Install-Nakama.ps1:215; eq-copilot/install/Install-Nakama.ps1:896; eq-copilot/install/Install-Nakama.ps1:908; eq-copilot/install/Install-Nakama.ps1:1147 |
| T3-12-02 | critical | K4 | DEFEKT | B04 | broker/src/transport/server_v3/auth.rs:45; broker/src/transport/server_v3/auth.rs:103; broker/src/transport/server_v3/auth.rs:168; broker/src/transport/bootstrap.rs:149; broker/src/transport/server_v3/verbindung.rs:253; broker/src/transport/server_v3/verbindung.rs:300; broker/src/coordinator/zustand.rs:10; broker/src/coordinator/link.rs:80; broker/src/coordinator/mod.rs:457; broker/src/lebenslauf.rs:142; broker/src/server.rs:691; broker/src/server.rs:786 |
| T3-12-03 | high | K2 | DEFEKT | B05 | broker/src/lebenslauf.rs:38; broker/src/lebenslauf.rs:152; broker/src/lebenslauf.rs:159; broker/src/server.rs:502; broker/src/server.rs:505; broker/src/main.rs:50 |
| T3-12-04 | medium | K-neu: Diagnose-Vertrauensgrenze | LÜCKE | B30 | eq-copilot/plugin/core/diagnose/Briefkasten.h:3; eq-copilot/plugin/core/diagnose/Briefkasten.h:76; eq-copilot/plugin/core/diagnose/Briefkasten.cpp:620; broker/src/briefkasten.rs:663; broker/src/briefkasten.rs:710; broker/src/briefkasten.rs:787; broker/src/briefkasten.rs:823; eq-copilot/plugin/src/prozessor/Analyse.cpp:1344; eq-copilot/plugin/sonde/SondeProcessor.cpp:921 |
| T3-12-05 | medium | K1 | LÜCKE | B05 | broker/src/lebenslauf.rs:152; broker/src/server.rs:45; broker/src/server.rs:549; broker/src/server.rs:673; broker/src/server.rs:782; broker/src/framing.rs:34; broker/src/framing.rs:65; broker/src/lebenslauf.rs:316 |

Die Standübernahme hat einen aktuellen Quellenbeleg: `git diff --name-only c8c920e2bbdd099e5185337070a094f78f1ae5c4 d53e0a370b9976d8ebe87e87eed25096237b8e53 -- eq-copilot broker tools/beweise.ps1 tools/eq-copilot tools/fl tools/plan tools/dirigent/install-aufgaben-registrieren.ps1` liefert **keinen Pfad**.
Damit sind die für diese Übernahme relevanten getrackten Produkt-/Prüfquellen zwischen dem ersten Rohbericht und dem gebundenen HEAD unverändert.
Das ist kein Hashbeweis für die gesamte lokale Build-Abhängigkeit oder installierte Binaries; Phase 10 beschreibt seinen JUCE-/FlatBuffers-Abgleich ausdrücklich separat.

## Widerlegte Verdachte

**12 Syntheseverdachte verworfen**, mit den folgenden Grenzen; keine zusätzliche vollständige Ursprungsbefundzeile wurde gestrichen.
Die 136 Verwerfungen der Ursprungsphasen bleiben in deren eigenen Listen und werden hier weder als neue Befunde noch als 136 eindeutige Defekte gezählt.

| Nr. | Verdacht | Gegenbeleg / verbleibende Grenze |
| --- | --- | --- |
| S11-W01 | Die 59 Skeptikerzeilen seien weitere Befunde. | Identische ID-Menge in 59 Ursprungszeilen und 59 Phase-10-Urteilen; Phase 10 legt keine T3-10-ID an (phase-10-skeptiker.md:96). Zählung bleibt 59. |
| S11-W02 | Jede Diagnose-Restarbeit nach Stop breche die Zusage. | broker/src/briefkasten.rs:887 erlaubt die abgelöste Restarbeit; M-45 verlangt keine neu ausgelösten Takte (docs/beweise/NAK-286.md:575). T3-04-04 bleibt nur low/HÄRTUNG. |
| S11-W03 | Wieder sichtbares LUFS nach Rücknahme sei bewiesen eine aktuelle Falschanzeige. | PluginEditor.cpp:1270 trägt stale/Alter; SONDE-012 L14/A06 erlaubt alten gültigen Wert mit Alter. Die rückwirkende Zuordnung ist unbestimmt: T3-07-03 bleibt LÜCKE (phase-10:104). |
| S11-W04 | 64 Modellplätze sagten 64 sichtbare Quellen zu. | Entwurf:4277 und ShotTestMain.cpp:114 tragen 16; die 20-Zeilen-Grenze ist ein ungelöster Annahme-/Bedienfall, keine gebrochene 64-Zusage (phase-10:105). |
| S11-W05 | Der korrekte lokale Queuepfad widerlege die vertauschten Commits. | Phase 04 W05/Phase 08 V05–V06 prüfen Ordnung nach Queueeintritt; T3-08-03 liegt davor (coordinator/invalidierung.rs:100; flush.rs:58; server_v3/queues.rs:228). Beide Aussagen können gleichzeitig wahr sein. |
| S11-W06 | Paar-ID anschließen genüge allein für das PRE/POST-Ticket. | Phase 08 V01/V02 beschreibt Disconnect-/Stale- und verspätete Aufnahme bei heute fehlender produktiver Vorbedingung (liveness.rs:211; prepost_verdrahtung.rs:146). Kein weiterer heutiger Befund, aber notwendige Gegenfälle in B21. |
| S11-W07 | NAK-246s Callbackschutz schließe den Sonde-Besitzfehler bereits. | Gen schließt seine Schleuse in PluginProcessor.cpp:474; SondeProcessor.cpp:139/:216/:1105 liefert eigene rohe Provider (phase-04 W01; phase-10 T3-04-01). Verschiedene Besitzer, keine Dublette. |
| S11-W08 | P4s deterministischer Generator beweise die Produktkalibrierung; P5 habe denselben Fehler. | Phase 09 W05 und broker/tests/sonde014_p5_korpus.rs:349 belegen echten P5-Output; P4s pruefe_p4_korpus.py:160/:219 verwendet Annotationen. B28 bleibt auf P4 begrenzt. |
| S11-W09 | Enge Pipe-ACL und geprüfter Plugin-Spawn widerlegten alle Sicherheitsbefunde. | Phase 12 V01/V04 belegt beides; T3-12-02 betrifft Identitäts-/Rollenbindung desselben OS-Benutzers, T3-12-01 die andere erhöhte Skriptkette. Kein behaupteter ACL-Durchbruch für einen Fremdbenutzer. |
| S11-W10 | Alle Scope-Zahlen ließen sich zu vollständig geprüften Repository-Dateien addieren. | Phase 03 Scope:1256 benennt Inventar-/Lesemodi; Phase 08 Scope trennt 74 Quellbereichs- von 38 Suchlektüren. Überschneidungen und generierte Fixtures sind keine zusätzliche semantische Abdeckung. |
| S11-W11 | B15 beseitige zugleich den bekannten Consumer-Altersriegel. | SourcesModel.cpp:1235 prüft Sitzung, :1335 ersetzt den Bestand; das Schema:1070 hat keine Ordnungsmarke. NAK-269 führt den bewusst nicht gebauten Consumer-Riegel ausdrücklich als Härtung. |
| S11-W12 | Weil Prüfgang und Wächter fehlen, sei auch der gesamte Laufzeit-Arm ungebaut. | NAK-286 ist abgenommen; tools/fl/LIES-MICH.md und Phase 09 inventarisieren A35 und reale Szenarien. Fehlend ist deren gemeinsame Grenzprüfung, nicht die Existenz des Arms. |

Die tragfähigen Teile der Codebase sind damit ebenfalls sichtbar: Gen-Callbackbesitz, lokale Queueordnung, enge Pipe-ACL, absoluter geprüfter Broker-Spawn, P5-Produktkalibrierung und die bestehenden State-/Schema-/Hostgegenproben sind konkrete vorhandene Schutzmechanismen.
Die überlebenden Fehler liegen häufig einen Übergang vor oder nach deren Geltungsbereich.
Das erklärt die Bündelbildung, rechtfertigt aber weder das Entfernen vorhandener Wachen noch eine pauschale Neuentwicklung.

## Nicht geprüft

- Keine Ausführung der vorgeschlagenen Reparaturen oder Beweise; insbesondere keine frischen Nulltests, Race-/Crashversuche, Host-/OS-/CPU-Messungen und kein Kanon.
- Keine erneute vollständige semantische Lektüre des gesamten Produktcodes durch Phase 11; die früheren Lesetiefen werden unten erhalten, nicht in eigene Lektüre umgerechnet.
- Keine Prüfung real installierter Task-/Datei-ACLs, Signaturen oder eines funktionierenden Privileg-Exploits; Phase 12 bleibt ein Quellenurteil.
- Kein vollständiger Abhängigkeiten-/Toolchainaudit, kein Audit des externen FL-MCP-Servers, kein Durchsehen aller alten Rohlogs oder Bild-/Audiogoldens.
- Manifeste, Register, Plan und Dokumentation sind Zusage-/Statuskontext, keine Befundfläche; keine neuen Prosa-, Zeilen- oder Trefferzahlbefunde.
- Geparkte Design-/Studienarbeit, Legacy-Umbenennungen und noch nicht gebaute spätere Produktfunktionen bleiben außerhalb.
- `planstand.py` und `dokuriegel.py` wurden entsprechend dem Leseauftrag nicht gefahren. Der gelesene Planstand ist Kontext; die vorgeschlagene Reihenfolge stützt sich auf Planquelle, Register und Auditpopulation.
- Ein lesender `py -3.13 -c`-Zugriff scheiterte am Windows-Launcher/Store-Python-Zugriff. Die Inventur und Zählung wurden anschließend ohne Schreiben mit PowerShell/.NET und JavaScript im Arbeitsspeicher durchgeführt; kein Pflichtinhalt blieb deshalb unlesbar.
- Git meldet fehlenden Zugriff auf die globale Ignore-Datei `C:/Users/phili/.config/git/ignore`; HEAD, Repository-Status und Repository-Dateien waren lesbar.

## Scope-Beweis

### Populationsschluss

Vor dem Schreiben vorhanden: **11/11 beauftragte Rohberichte** (01–10 und 12) und **1/1 BEFUNDE.md**.
Alle **59/59 Ursprungsbefund-IDs**, **59/59 Skeptikerurteile**, sämtliche Frageabschnitte und alle **11 Scope-Beweise** sind in der Synthese erfasst.
Die Zuordnung besitzt **36 Bündel, 59 Mitglieder, 0 fehlende IDs, 0 Mehrfachzuordnungen**; P1/P2/P3/P4 = **24/4/4/4**.
Keine neue Befund-ID; 12 eigene verworfene Syntheseverdachte; 17 Karten aus 20 Frageeinträgen, davon eine bereits durch den Skeptiker geklärt.
Kein Pflichtpfad fehlt.

### Lesetiefe der übernommenen Phasen

Pfade dieser Tabelle sind relativ zu `docs/audits/2026-09-15-tiefenaudit/`.
Zahlen bleiben populations- und phasenbezogen; überlappende Lesesichten werden nicht addiert.
Ein Suchzugriff, ein strukturierter Fixturelesevorgang und eine vollständige Implementierungslektüre bleiben unterschiedliche Belegarten.

| Phase | Scope-Quelle | Population / Dateizahl | Beweisgrenze |
| --- | --- | --- | --- |
| 01 | roh/phase-01-echtzeit.md:359 | 47/47 Implementierungen, zusätzlich fünf vollständig gelesene Abhängigkeiten; JUCE/Testkontext separat | Körperlektüre der Audio-/Analysepopulation; kein gemessener kompletter Wrapper-RT-Pfad. |
| 02 | roh/phase-02-zustand.md:447 | 101 Kernpfade: 13 State, vier Schema, 79 State-Fixtures, vier Identity-Fixtures, ein Generator; 28 weitere gezielt | Binärfixtures strukturell gelesen, nicht durch Produktreader ausgeführt. |
| 03 | roh/phase-03-vertrag.md:1252 | 641 Primärpfade; 775 JSON-Deklarationen und 64 IDL-Felder einzeln | Unterschiedliche Lesemodi, viele Fixtures/Codegen; kein 641-Dateien-Vollbeweis. |
| 04 | roh/phase-04-lebenslauf.md:242 | 64/64 Primärpfade nach Lebenslaufmustern; weitere Aufruf-/Testpfade getrennt | Population der Besitz-/Stop-/Queue-Muster, nicht jedes Fachalgorithmus. |
| 05 | roh/phase-05-hostkante.md:219 | 35 Primärpfade, davon 32 gelesen/durchsucht; zusätzliche Quellen/Beweisartefakte getrennt | Host-/Installationsnähte; drei Editorpfade ausgenommen, keine reale FL-Messung. |
| 06 | roh/phase-06-messkern.md:203 | 98 unterschiedliche Lese-/Kontextpfade plus 18 strukturierte Fixtures; 18 Kernheader mit 234 Funktionen vollständig | Messkernpopulation umfassend; übrige Quellen unterschiedlich tief, externe Referenz ausdrücklich enthalten. |
| 07 | roh/phase-07-modell.md:740 | 34/35 Primärdateien inhaltlich, insgesamt 83 benannte Pfade; generierte Datei inventarisiert | Technische Wirkung/Verbraucher; kein visueller Designaudit. |
| 08 | roh/phase-08-broker.md:212 | 113 Pfade: 89 Rust-src, 23 Rust-Tests, Cargo.toml; bei Rust 74 Körper-/Quelllektüren, 38 Suchzugriffe | Vollständiges Funktionsinventar ist keine vollständige Semantikprüfung aller 112 Rust-Dateien. |
| 09 | roh/phase-09-beweisluecken.md:1126 | 150 Pfade; 43/43 Plugin-Testdateien, 27/27 Python-Werkzeuge, 23/23 Broker-Integrationstests | Bein-/Behauptungsinventar und gezielte Körperlektüre; keine Testausführung. |
| 10 | roh/phase-10-skeptiker.md:145 | 59/59 Befunde, zehn Rohberichte, 152 explizite Dateien plus Werkzeugkontext | Jede Behauptung samt Test/Absicht/Register gegengeprüft; keine zweite Vollprüfung der Codebase. |
| 12 | roh/phase-12-pipesicherheit.md:70 | 61/61 Dateien an Vertrauensgrenzen plus zwölf zusätzliche Pfade | Anderes Modell gpt-5.6-sol/max; keine reale Task-ACL-/Exploitmessung. |

### Eigener benannter Inhaltsscope

**46 unterschiedliche Repository-Dateien**, nachstehend vollständig benannt; dazu **ein externer Startlog**, oben mit absolutem Pfad.
Alle 46 existierten beim Metadaten-/Hashabgleich.
Große Textbestände wurden eingelesen und nach Befund-, Fragen-, Scope- und Registerpopulation verarbeitet; reine Anhangstabellen sind nicht als erneuter Produktquellbeweis ausgegeben.
Zusätzliche `rg`-/`git ls-files`-Inventuren dienen nur der Pfad-/Trefferermittlung, nicht als behauptete Volllektüre.
Die neu erzeugte Synthesedatei zählt nicht zur Eingabepopulation.

### Auditpopulation — 12 Dateien

Vollständiger Textbestand eingelesen; alle 59 Befundzeilen und 59 Skeptikerurteile einzeln abgeglichen, sämtliche Frage-/Scope-Abschnitte; große Anlagen als strukturierte Population ausgewertet.

- `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-01-echtzeit.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-02-zustand.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-03-vertrag.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-04-lebenslauf.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-05-hostkante.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-06-messkern.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-07-modell.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-08-broker.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-09-beweisluecken.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-10-skeptiker.md`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-12-pipesicherheit.md`

### Einstieg und fachlicher Kontext — 14 Dateien

AGENTS/CLAUDE und Skill vollständig; übriger Textbestand eingelesen, fachlich nur die beauftragten Abschnitte, Registerzeilen und Plan-/Fragenpopulation ausgewertet; keine Prosa als Befundfläche.

- `AGENTS.md`
- `CLAUDE.md`
- `.agents/skills/nakama-plugin-quality/SKILL.md`
- `docs/plugin-wissen.md`
- `docs/gesundheit/KONZEPT.md`
- `docs/gesundheit/abdeckungskarte.md`
- `docs/gesundheit/lebenslauf-audit.md`
- `docs/offene-punkte.md`
- `docs/FL-Nakama-Sonden-Design-Entwurf.md`
- `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md`
- `docs/plan/plan.json`
- `docs/plan/fragen.json`
- `docs/PLAN-STAND.md`
- `docs/plan/LIES-MICH.md`

### Zusage-/Gegenbelegkontext — 8 Dateien

Gezielte Matrix-, Gate- und Gegenbelegsuche; keine vollständige Loglektüre und kein Manifestaudit.

- `docs/beweise/SONDE-012.md`
- `docs/beweise/SONDE-013.md`
- `docs/beweise/SONDE-014.md`
- `docs/beweise/SONDE-015.md`
- `docs/beweise/NAK-123.md`
- `docs/beweise/NAK-246.md`
- `docs/beweise/NAK-283.md`
- `docs/beweise/NAK-286.md`

### Direkte Anschlusslektüre — 10 Dateien

Gezielte Quell-/Vertragsbereiche und Aufrufkontext; keine neue Vollprüfung dieser Implementierungen.

- `eq-copilot/schemas/v3/README.md`
- `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`
- `eq-copilot/identity/host-capabilities-fl-v1.json`
- `tools/beweise.ps1`
- `tools/fl/LIES-MICH.md`
- `eq-copilot/plugin/src/SourcesModel.cpp`
- `eq-copilot/plugin/src/PluginEditor.cpp`
- `eq-copilot/plugin/core/ipc/controlclient/Intern.h`
- `broker/src/briefkasten.rs`
- `tools/pruefung/codex-audit-lauf.ps1`

### Auftrags-/Werkzeugkontext — 2 Dateien

Gezielte Textsuche; kein neuer Quellbefund.

- `tools/hooks/agent-reminder-nakama.sh`
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-11-auftrag.txt`

### Nicht gelesene und nur einseitig gelesene Restfläche

Der mechanische Quercheck benutzt **297 getrackte Pfade** aus `eq-copilot/plugin`, `broker`, `eq-copilot/cmake`, `tools/fl`, `tools/pruefung` und `tools/plan`.
Er vergleicht sowohl vollständigen Pfad als auch Dateinamen mit dem vollständigen Text aller elf Rohberichte.
**14 Dateien besitzen keinen ausdrücklichen Dateinamensbezug**; das ist ein Indiz fehlender dokumentierter Inhaltslektüre, kein Beweis, dass niemals ein Include oder eine Verzeichnisgruppe berührt wurde:

- `broker/Cargo.lock`
- `broker/sensoruebersicht-referenz/Sensoruebersicht.svelte`
- `eq-copilot/cmake/NakamaFlatBuffers.cmake`
- `eq-copilot/cmake/NakamaFlatBuffersProjektHook.cmake`
- `eq-copilot/cmake/NakamaFlatcBeleg.cmake`
- `eq-copilot/cmake/NakamaKern.cmake`
- `eq-copilot/cmake/pruefe_nakama_kern_genex.cmake`
- `eq-copilot/plugin/probe/PipeProbeMain.cpp`
- `eq-copilot/plugin/src/Diagnose.h`
- `eq-copilot/plugin/src/EqCopilotAssetKit.h`
- `eq-copilot/plugin/src/EqCopilotIds.h`
- `eq-copilot/plugin/src/LeitstandTokens.h`
- `tools/plan/antworten_blatt.py`
- `tools/pruefung/auftrag-lebenslauf-audit.md`

Davon bilden Cargo.lock, fünf CMake-Helfer und PipeProbeMain.cpp die **sieben konkreten Restpfade der Bau-/Hostprüfung**.
Die vier Header `Diagnose.h`, `EqCopilotAssetKit.h`, `EqCopilotIds.h`, `LeitstandTokens.h` werden nicht zu neuen Stil-/Identitätsbefunden.
Svelte-Referenz, `antworten_blatt.py` und alter Auditauftrag sind Referenz-/Kontextflächen und außerhalb der Produktfehlersuche.
Dazu bleiben die externe FL-MCP-Gegenseite, vollständige Abhängigkeitskörper und tatsächliche OS-/Hostzustände ungemessen beziehungsweise ungelesen, wie oben unter Blind Spots abgegrenzt.
Die 38 Suchlektüren aus Phase 08 sind kein pauschaler Rest von 38 ungelesenen Dateien: andere Phasen überdecken Teilmengen, ohne einen vollständigen semantischen Vereinigungsbeweis zu liefern.

### Fehlende Pfade

- **Pflichtpopulation: 0 fehlend.**
- Geplanter, noch nicht vorhandener Detektor: `tools/pruefung/pruefgang.py`.
- Geplantes Wächterverzeichnis: `.claude/agents/` existiert am Lesestand nicht.
- Diese beiden fehlenden Aufbauteile sind Kontext zu NAK-260/264 und kein neuer T3-11-Defekt gegen Dokumentation.

### Integrität der Ausgabe

Ausgangs-BEFUNDE: **129190 Bytes**, SHA-256 `C3784453ED2135254E72645C34B8AF589D29F1105471077CAE68FAE4237D7547`.
Erlaubter Ersatzbereich: ausschließlich der Inhalt ab `## Kopf` bis vor `## Phase 00`; dadurch bleiben zusätzlich zur verlangten Phase-01-Grenze auch Phase 00 und alles oberhalb von Kopf erhalten.

Zu erhaltende SHA-256-Werte:

- Präfix vor `## Kopf`: `900C0CA32CC5315A0B2E0E8A12345C14141D39DAD87102C067DFCE60BFF67DB6`.
- Gesamter Suffix ab `## Phase 00`: `D5880FE81502B54BA8379FD5DA10E83356E40DB1F65FACEDF9A6106E4E34F8F2`.
- Gesamter Suffix ab `## Phase 01`: `842C9D30E00AF60A5273E7B26606F41F52553AF8EA04DAAFD2B289280B371F85`.

Abschlusskontrolle am 16.09.2026, 04:39:27 MESZ: Alle drei Präfix-/Suffixhashes stimmen mit dem Ausgang überein; damit sind alles oberhalb von Kopf und ab Phase 00 einschließlich sämtlicher Phase-01-bis-12-Abschnitte bytegleich erhalten. Die 45 sonstigen benannten Eingabedateien sind im erneuten SHA-256-Abgleich unverändert, darunter alle elf Rohberichte. Beide Bündeltabellen sind wortgleich; 36 Bündel mit 59 eindeutigen Mitgliedern, Zähler 24/4/4/4, 59 korrekte Klassifikationen, fünf unveränderte Phase-12-Einstufungen, 17 Karten und zwölf Synthese-Verwerfungen sind am geschriebenen Text nachgezählt. `git diff --check` meldet keinen Fehler; LF bleibt erhalten. Der eigene Diff enthält ausschließlich den Ersatz der Kopf-Platzhalterzeile (297 Zeilen hinzu, eine entfernt) und den neuen Bericht. Die schon vorher vorhandenen vier ungetrackten Pfade `.claude/settings.local.json`, `briefing-hub/`, `roh/phase-11-auftrag.txt` im Auditordner und `nimbalyst-local/` bleiben unberührt. Schluss-HEAD: `d53e0a370b9976d8ebe87e87eed25096237b8e53`; Branch master. Kein Produktcode geändert, kein Staging, Commit oder Push.
