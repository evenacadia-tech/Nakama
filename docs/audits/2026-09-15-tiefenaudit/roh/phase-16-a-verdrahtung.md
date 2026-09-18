# Phase 16 · Paket A — Produktverdrahtung und verschobene Ergebnisse

Datum: **18.09.2026**. Geprüfte Produktquelle: **`aff2d8188f33a9525aec1869449773bfdbe305a6`** in `C:\na-audit16-aff2d818`. Der Produktcode dieser Kopie wird nicht verändert. Schreibbereich dieses Pakets sind dieser Bericht und [phase-16-nachweise/a](phase-16-nachweise/a/). Die abschließende ID-Vergabe und Gegenprüfung liegen bei der Hauptsession; `A-01` ist eine Paketkennung.

Verfahren: `AGENTS.md` und `CLAUDE.md` vollständig gelesen; Profil `deep` aus `nakama-plugin-quality`, ohne Reparaturen entsprechend dem engeren Userauftrag. Gelesen wurden insbesondere Phase 14, die Phase-15-Felder 1 bis 6, Paket 1 einschließlich Gegenprüfung, die einschlägigen Zeilen von `BEFUNDE.md`, NAK-213/214/283 und die tatsächlichen Sender, Empfänger und Verbraucher. Sicherheit, Gestaltung, Installation und laufende Hosts bleiben außerhalb dieses Pakets. Ein Quellenbefund wird nicht als echter Hostlauf bezeichnet.

## Ergebnis der übernommenen Ausgangsfelder

| Feld | Bisheriger Nachweis | Ergebnis dieses Pakets | Verbindlicher Status für Phase 16 |
| --- | --- | --- | --- |
| F01 · Hypothesenrechnung ohne Prüfung aller Eingaben beim Rückschreiben | Phase 15 OF-1, NAK-283-Riegel für Intentgeneration und Evidence-IDs | Unbind im echten Rechenhaken lässt ein Ergebnis mit den Belegen der inzwischen entbundenen Quelle eintragen; die nächste Evidenz leert es. Eine vor dem Intentwechsel gesammelte leere Rechnung entfernt den zwischenzeitlich auf `stale` gesetzten Bestand. Beide Folgen ausgeführt. K-20 legt für Beitritt die Neuberechnung ab neuer Evidenz fest; keine allgemeine Sofortregel für sämtliche Graphänderungen gefunden. | **Benötigt Produktentscheidung** für den Zeitpunkt der Invalidierung/Neuberechnung bei Mitgliedschaft und Führung. Kein zusätzlicher T3-Defekt allein aus einem fehlenden Generationsfeld. Die Leer-Ergebnis-Variante gehört als konkret belegte Grenze in dieselbe Entscheidung; sie verletzt die allgemeine Formulierung der Generationserhaltung, liefert hier aber keinen falschen handelbaren Befund. |
| F03 · Vorschläge ohne Rückschreibprüfung | Phase 15 OF-3: verschiedene Lock-Abschnitte | Sammeln, Rechnen und Cache-Einsetzen sind weiter getrennt. `capability_vorhanden=false` verhindert jedes `draft_offer`; die Snapshot-Sicht enthält keine Vorschläge. Die `proposals`-Projektion ist ein realer gebauter API-Nebenweg; das Überholen zweier Schreibvorgänge wurde hier nicht ausgeführt. A-01 verhindert zusätzlich die normale Masterhypothese. | **Derzeit nicht produktiv erreichbar** als veraltetes Angebot an Gen. Der Store-Rest ist vor der Angebotsfreigabe gezielt zu beweisen; keine heutige fernwirksame Fehlaktion behauptet. |
| F04 · Experimentresultat in zwei Lock-Abschnitten | Phase 15 OF-4 | Der Empfänger misst sogar zweimal: Vorprüfung `:168`, danach die tatsächlich übernommene Messung `:224`. Eine Invalidierung vor dieser zweiten Messung wird erfasst. Nur das Fenster **nach** der zweiten Messung und **vor** `:275` bleibt. Kein äußerer Session-Guard schließt es; der Commit-Guard entsteht erst innerhalb der anschließenden Persistenz. | **Benötigt einen konkret benannten weiteren Nachweis**: echte `experiment_manual_result`-Nachricht, Barriere exakt nach `resultatmessung` an `:227`, dort einen tatsächlich verwendeten Baseline-Beleg über Intervention zurücknehmen, dann Terminal, Store und Snapshot auf diese ID prüfen. Kein Haken an dieser Naht vorhanden; ein Test nur vor der ersten Messung wäre kein Nachweis. Gebauter API-Vertrag ohne angeschlossene Bedienung. |
| F05 · PRE/POST-Paarurteil ohne Rückschreibprüfung | Phase 15 OF-5; T3-08-04 | Die Hälfte wird unter Lock kopiert, dann nach einer Lock-Lücke unter dem neuen Lock beurteilt und eingefügt. Präzisierung zu Phase 15: `bilde_paare` läuft **unter dem zweiten Lock**, nicht außerhalb beider Locks. Die kopierten Halbzeuge können trotzdem alt sein. Produktiver Descriptor führt weiterhin keine `pair_id`. | **Derzeit nicht produktiv erreichbar**; die Anschlusslücke ist **T3-08-04**. Alte Halbzeuge nach Invalidierung/Disconnect gehören in dessen Reparaturmatrix, ohne weitere ID für eine noch nicht produktiv erreichbare Folge. |

F02 (Replay nach Same-Instance-Reload) und F06 (DSP-Zustand/Evidenzsegment) sind mit Paket C/Hauptsession abgestimmt; dort steht ihr endgültiges Urteil. Dieses Paket zählt sie nicht nochmals.

## A-01 · Gen liefert dem v3-Hypothesenweg keine eigene Master-Evidenz

**Vorschlag für die zentrale Einordnung:** eigenständige funktionale Anschlusslücke; **high / DEFEKT / K4, K8** unter der zugesagten gebauten Masterbefund-Kette. Falls die Hauptprüfung einen bindenden expliziten Aufschub des Gen-Senders findet, ist die Einordnung entsprechend herabzustufen. Das geparkte Design ist kein solcher Nachweis.

### Wirkung und Erreichbarkeit

Eine normale Sitzung mit Gen als `main` und messender Probeeq bekommt über die v3-Kette keine Masteranomalie. Auch beliebig viele weitere Probeeq-Belege ersetzen die fehlende Main-Historie nicht. Der Broker kann deshalb nur eine Enthaltung `DatenReichenNicht` / `MoreData` mit dem Grund „Das fuehrende Main hat noch keinen verwertbaren Beleg gesendet“ bilden. Sein nächster Schritt lautet „Dieselbe Passage laenger messen“, obwohl längeres Messen den fehlenden Sender nicht ergänzt. Das lokale Gen-Messen und die alten lokalen Hinweise sind davon getrennte Ausgänge und funktionieren dadurch nicht automatisch falsch.

**Erreichbarkeit 1:** Gen-/Probeeq-Produktweg nach Rollenwahl und normalem Verbindungsaufbau. Hier als Quellenkette und Ausführung des echten **Broker-Coordinators** geprüft; keine gestartete C++-Plugininstanz und kein echter Host. Die positive Gegenprobe mit zusätzlich eingespeister Main-Evidenz zeigt den vorgesehenen Eingang, stellt aber keine heute vorhandene Gen-Sendefunktion dar.

### Auslöser → Sender → Annahme → Rechnung → Rückmeldung → Verbraucher

1. Gen verarbeitet Audio und erzeugt FeatureEngine-Frames: `eq-copilot/plugin/src/prozessor/Analyse.cpp:307-310`. Seine lokalen Snapshots werden an `:345-360` ausgewertet. In diesem Worker gibt es keinen Aufruf von `evidenceSnapshotAlsJson`, keinen Evidenz-P1-Producer und keine Übergabe eines Featureframes an `telemetryV3`.
2. Gen meldet bei v3-Verbindungsaufbau Subscription und Intent-Vollbestand: `eq-copilot/plugin/src/prozessor/Ipc.cpp:1031-1059`. Seine tatsächlichen P1-Sender im gesamten `src/` sind Subscription, Intent und AssistantStep. Der einzige Produktaufrufer von `evidenceSnapshotAlsJson` ist **Probeeq**, `eq-copilot/plugin/sonde/SondeProcessor.cpp:843-849`.
3. Auch der Main-Descriptor hat keinen Zulieferer: `EqCopilotProcessor::v3Status`, `eq-copilot/plugin/src/prozessor/Ipc.cpp:484-517`, lässt `ControlStatus.runtime` ungesetzt. Default `gemeldet=false`: `eq-copilot/plugin/core/ipc/ControlClient.h:132-142`; `runtimeJson` lässt den Block dann weg: `core/ipc/controlclient/Intern.h:412-419`. Diese Nebenlücke betrifft spätere Kanal-/Messpunktvergleiche; die fehlende **Master-Evidenz** allein reicht bereits für A-01.
4. Der Broker nimmt eine `evidence_snapshot` ausschließlich über den P1-Evidenzzweig an: `broker/src/coordinator/senke.rs:103-108`; `evidenz.rs:179-189` schreibt die Historie, `:232` rechnet Hypothesen. Der P2-Livepfad schreibt keine `stand.evidenz`-Historie und bildet keine Masterfenster als Ersatz.
5. `hypothese_verdrahtung.rs:295-340` bildet jedes Quellprofil ausschließlich aus der jeweiligen Evidenzhistorie. `:414-418` wählt das führende Main als Master; eine Probeeq auf dem Masterbus wird nicht stellvertretend zum Main.
6. `hypothese/rechnung.rs:92-100` fällt ohne Masteranomalie in `KeinMasterbeleg`; `hypothese/enthaltung.rs:91-153` baut die allgemeine Enthaltung mit den vorhandenen Belegen der Sonde. Diese kann normal persistiert und in den Sitzungssnapshot übertragen werden (`hypothese_verdrahtung.rs:143-155`, `:737-751`). `SourcesModel` übernimmt Befunde aus diesem Snapshot; ein lokaler Gen-Hinweis ersetzt diese v3-Historie nicht.

### Reproduzierbare Gegenprobe

[verdrahtung.rs](phase-16-nachweise/a/verdrahtung.rs), Test `a01_current_gen_messages_never_supply_master_evidence`, verwendet den **unveränderten** Broker als Cargo-Path-Dependency. Die Eingaben verwenden das eingecheckte Evidenzfixture und Nachrichtengestalt des aktuellen Heartbeat-Writers. Capabilities sind genau zwei `supported` und acht `unsupported` wie `Intern.h:388-402`. Die Neutralbestätigung wird über einen echten `Senke::p0`-Heartbeat transportiert; kein `descriptor_setzen` und kein direktes `resync_bestaetigen`.

Vorbedingung: Hello für Main und Probeeq, Main-Heartbeat ohne Runtime, Probe-Heartbeat mit Runtime, vollständiger leerer Intent, elf angenommene Probe-Evidenzen. Assertions messen, dass alle elf Belege angenommen wurden, die Main-Historie leer ist und genau ein Befund mit `beobachtung.gueltig=false` entsteht. Ausführung: **bestätigt**.

Gegenfall: dieselbe Bühne, zusätzlich ein Main-Descriptor über Heartbeat und dreizehn Main-Belege über die P1-Senke. Nun entsteht eine gültige Beobachtung mit `QuelleResonanz` an Probeeq. Ausführung: **bestätigt**. Dies ist ein Gegenbeleg gegen „der Fixtureinhalt reicht fachlich grundsätzlich nicht“; die im Gegenfall ergänzte Masterzuspeisung ist ausdrücklich eine heute fehlende Produktvorbedingung.

### Zusage, Gegenargument und Testgrenze

- Konkrete technische Lieferzusage: `docs/FL-Nakama-Sonden-Design-Entwurf.md:4314-4334` ordnet Evidenzgraph, CauseHypothesis, Alternativen, Proposal und manuellen Versuch **P5** zu; `:2379-2383` verlangt im Screening die Verbindung von Master-Anomalien und Quellenergie. P6 (`:4338-4348`) verschiebt den aktiven Audiokern, nicht diesen passiven Mastereingang. `docs/beweise/SONDE-014.md:737` (M-18) bindet das Screening nochmals an Masteranomalien. `docs/beweise/NAK-213.md:435` sichert zwar die korrekte Enthaltung bei **fehlendem** Main-Beleg, sagt aber nicht, dass der Main im Produkt niemals Belege senden soll. In den gezielt geprüften P5/P6-Quellen wurde kein ausdrücklicher Aufschub des Gen-Evidenzsenders gefunden; eine Vollsuche sämtlicher historischer Plandokumente wird nicht beansprucht.
- Stärkstes Gegenargument: Gen besitzt eine lokale Analyse und einen v2-PipeClient; fehlende Ergebnisse können deshalb als nur noch nicht angeschlossene Oberfläche erscheinen. Die Aufrufsuche ergibt jedoch keinen Weg von deren Messwerten zu `Coordinator::stand.evidenz`. Das ist die technische Lücke **vor** jeder geparkten Designfrage. Ein lokaler M3-Hinweis ist keine Master-Evidenz im v3-Hypothesenweg.
- Zweites Gegenargument: Enthaltung ist ehrlich. Der Broker sagt den fehlenden Beleg korrekt an; das widerlegt eine falsche Messbehauptung, aber nicht den fehlenden Sender und den dadurch dauerhaft unwirksamen „mehr messen“-Schritt.
- Vorhandene Tests: `broker/tests/sonde014_verdrahtung.rs:119-166` setzen den Descriptor direkt; `:685-708` speisen Masterbelege selbst ein. Sie prüfen die Rechnung ab einer **manuell hergestellten** Vorbedingung. Die Sondertests für echten Heartbeat an `:2621-2662` verbessern die Descriptor-Ebene, führen jedoch ebenfalls keinen Gen-Evidenzsender aus.
- Die Harnisch-Nachrichten sind nach aktueller Quelle zusammengesetzt, **nicht durch C++ serialisiert**. Daher lautet die Nachweisart: **Quellenherleitung + Ausführung echter Brokerkomponente**. Ein Ende-zu-Ende-Nachweis ab einem echten `EqCopilotProcessor` bleibt erforderlich.

### Abgrenzung, kleinster Korrekturumfang und Regression

Kein Duplikat von T3-08-04: dort fehlt die Paar-ID für PRE/POST; hier fehlt die gesamte eigene Main-Evidenz für gewöhnliche Masterhypothesen. Kein Duplikat von T3-07-06: dort bleiben lokale Mess-/Freeze-Werte über Reload stehen; hier erreicht kein aktueller Masterwert die Brokerhistorie. NAK-213-K35 regelt korrekt den fehlenden Eingang, repariert dessen Erzeuger nicht.

Kleinster kohärenter Umfang: den bereits vorhandenen Gen-Analyseworker an den bestehenden Evidenzserialisierer und P1-Transport anbinden; Main-Runtime/Hostkanal aus den vorhandenen aktuellen Zuständen ehrlich melden. Identität, Epoche, Segment, Ereignisentnahme, Ablehnung/Queueverlust und Reload müssen dieselbe Semantik wie am Probeeq-Sender behalten. Keine zweite Formel und kein Kopieren einer Probeeq-Klasse als Main-Ersatz.

Regression: echte Gen- und Probeeq-Prozessoren mit derselben isolierten Brokerinstanz, bekanntes Audio, Gen-Rolle Main, mindestens eine angenommene **Gen-eigene** Evidence-ID, dann gültige Masterbeobachtung. Gegenfälle: fehlendes oder nicht verwertbares Main-Audio bleibt `KeinMasterbeleg`; Probeeq auf dem Masterbus darf Main nicht ersetzen; Reconnect/Reload veröffentlicht keine alte Generation; P1-Druck löscht keine noch unbestätigten Ereignisse. F01/F03-Folgekombinationen werden nach Anschluss dieser Eingangshälfte erneut erreichbar und gehören in denselben Testplan.

## Präzisierung zu T3-15-02: der Kanalgegenweg maskiert den Positionsbruch und verwirft beim unveränderten Reconnect

**Keine neue Befund-ID; gleiche Ursache `Hello → descriptor=None`.** Die bisherige Aussage „alte Evidenz bleibt gültig“ gilt nicht für jede Hostkonstellation. Diese Phase ergänzt erstmals einen ausgeführten Gegenweg und dessen entgegengesetzte Wirkung.

`broker/src/coordinator/link.rs:253-257` löscht den Descriptor beim Same-Nonce-Hello, behält aber die Evidenz. Der Positionsvergleich verlangt `Some(alt)` und `Some(neu)` (`liveness.rs:411-424`) und fällt damit aus. Der benachbarte **Kanalvergleich** arbeitet anders: er vergleicht zwei `Option<i64>` direkt (`:442-460`). Ein weiter gemeldeter Mixerindex 3 erscheint nun als **None→3**, selbst wenn er vor Reconnect ebenfalls 3 war. Die Kanalinvalidierung sammelt alle noch gültigen Evidence-IDs dieser Quelle und nimmt sie als `MesspunktWechsel` zurück (`invalidierung_verdrahtung.rs:510-555`). Der Positionswechsel gilt der Sitzung, der Kanalwechsel nur der Quelle (`liveness.rs:325-329`); beide sind also auch im Umfang kein vollständiger Ersatz füreinander.

Ausführung mit echter Coordinator-Komponente, `f15_02_reload_position_with_and_without_mixer_context`:

| Folge | Belege vorher | Belege nachher gültig | Ergebnis |
| --- | --- | --- | --- |
| pre → Same-Nonce-Neu-Hello → pre, kein Hostkanal geliefert | 3 | 3 | Richtiger unveränderter Gegenfall |
| pre → Same-Nonce-Neu-Hello → post, kein Hostkanal geliefert | 3 | 3 | T3-15-02 bestätigt: notwendige Rücknahme fehlt |
| pre → Same-Nonce-Neu-Hello → post, Hostkanal weiterhin 3 | 3 | 0 | Kanalgegenweg nimmt alle drei zurück; breite Aussage aus Phase 15 eingeschränkt |
| pre → Same-Nonce-Neu-Hello → pre, Hostkanal weiterhin 3 | 3 | 0 | Unnötige Rücknahme bei unveränderter Quelle; neu bestätigte Gegenwirkung derselben Ursache |

Alle vier Varianten halten Adresse, Runtime-Nonce und Hostkanal soweit geliefert konstant. Der Positionswert wird nur im markierten Fall gewechselt. Alle eingehenden Belege sind vor dem Wechsel nachweislich angenommen und nicht ausgeschlossen. Die drei Ausschlussgründe in den beiden Fällen mit Kanal sind jeweils `messpunkt_wechsel`.

**Erreichbarkeit präzisiert:** Die heutige Probeeq setzt `hostMixerIndexGemeldet` ausdrücklich **false** (`eq-copilot/plugin/sonde/SondeProcessor.cpp:1162-1163`). Deshalb sind die beiden Varianten mit bekanntem Index ein **gebauter Broker-API-Gegenweg ohne heutigen Probeeq-Sender** (Stufe 2), kein heutiger Benutzerfehler. Sie gehören vor Anschluss eines Mixerindex-Senders in dessen Regression. Der heutige Probeeq-Weg entspricht der Variante ohne Hostindex, in der T3-15-02 bestehen bleibt. Der technische Positionswechsel selbst bleibt Host-State-API-Eingang; kein heute vorhandener lokaler Positionseditor wird behauptet. Der Beweis führt Coordinator-Hello/Heartbeat aus, keinen Host-Recall und keinen tatsächlichen C++-Serializer.

**Stärkstes Gegenargument:** Nach Disconnect ist Evidenz ohnehin vorsichtig zu behandeln. Dagegen spricht der explizite Erhalt der Historie bei gleicher Nonce in `link.rs:131-143`, der getrennte Liveness-Zustand sowie NAK-213s unveränderter Kanalgegenfall (`broker/tests/sonde014_verdrahtung.rs:3113`); der Test schließt allerdings keinen Reconnect dazwischen ein. Der Same-Nonce-Reconnect ist kein nachgewiesener Klang- oder Messpunktwechsel. Die genaue Historiepolitik darf Fable nicht aus der neuen Rücknahme implizit ableiten.

**Kleinster Korrekturumfang:** Vergleichsbasis für Position **und** Kanal über den Same-Instance-Reconnect bewahren beziehungsweise beim Hello mit dem Vorgänger vergleichen. Nicht den Kanalgegenweg entfernen: ein echter Kanalwechsel muss seine Quellenevidenz weiter zurücknehmen. Regression ist die Vierermatrix oben, zusätzlich neuer Nonce/Klon sowie ein echter Positionswechsel mit sitzungsweitem Umfang. Diese Arbeiten gehören zu T3-15-02, keine zweite ID für denselben Reset.

## F01: Grenzen der bestehenden Generationswache, ausgeführt

`hypothese_verdrahtung.rs:195-470` sammelt Intent, Führung, Clients, Descriptor, Evidenz und Passage unter dem Standlock. Die Rechnung läuft außerhalb; `:1183-1216` vergleicht nur die Evidence-ID-Menge und bei nichtleerem Ergebnis die verwendeten IDs/Intentgeneration. Der P0-`unbind_probe` läuft über `befehl.rs:575-576` nach `mitgliedschaft.rs:163-176`, setzt aber nur den Mitgliedsstatus und `dirty_sessions`. Die nächste Snapshot-Rückmeldung enthält deshalb noch die vorherige Hypothese. Der Gate-Text K-20 (`docs/beweise/NAK-213.md:407`) nennt für späteren Beitritt ausdrücklich neue Evidenz als Anlass der Neurechnung.

Der Auditfall setzt keinen Descriptor direkt, verwendet keinen direkten Mitgliedschaftssetter und kein direktes `intent_update_json`: Hello, Heartbeat, Intent, Unbind und Evidenz laufen durch die gebauten Nachrichtenannahmen. Nur der vorhandene Rechenhaken hält die Lücke deterministisch offen.

- `f01_unbind_during_calculation_retains_old_membership_until_new_evidence`: laufende Rechnung sammelt zwölf Belege, Unbind wird über `Senke::p0` positiv quittiert, alte Rechnung trägt zwölf IDs ein. Ein weiterer Evidenzsnapshot leert die Enthaltung. Das entspricht der noch nicht allgemeinen Graphwechselregel; **kein selbstheilender Soforttakt nach Unbind** wurde behauptet oder gemessen.
- `f01_empty_calculation_clears_stale_after_intent_revision`: nach Unbind sammelt die Rechnung leer; der Haken meldet einen jüngeren vollständigen Intent. Im Haken steht genau ein `stale`-Befund, nach Rückschreiben null. `:1193` überspringt den Generationsvergleich für die leere Ausgabe, `:579-580` löscht den Bestand. Damit ist die weite Formulierung „ein Rechenergebnis einer älteren Generation wird nicht eingetragen“ (`SONDE-014.md:3503`) an der Löschseite nicht geschlossen. **Kein falscher neuer READY-Befund**, kein Audioverlust und keine sichtbare Produktbedienung des Intent-Handgriffs behauptet. Die fachliche gewünschte Wirkung nach Wegfall aller Kandidaten muss zur Regel passen.

Weitere nicht ausgeführte F01-Kombinationen sind Führung, Kanal/Descriptor und Passagewahl. Der Kanalwechsel hat bereits einen wirklichen Invalidierungsweg; ein bloßer fehlender Graphzähler wäre deshalb dort kein ausreichender Fehlerbeweis. Die Mitgliedschaftsfälle dürfen nicht pauschal auf sämtliche Graphänderungen übertragen werden.

## F03, F04 und F05: geprüfte Gegenwege und konkrete Beweisgrenzen

**Vorschläge:** `proposal_verdrahtung.rs:37-60` und `:77-88` trennen Eingang, Cache und Persistenz. `:108` setzt Capability fest falsch; `proposal.rs:942-959` benutzt diese Wache wirklich. Der Vorschlag braucht außerdem eine benannte Passage und vollständige Gate-Felder (`proposal_verdrahtung.rs:450-480`). Kein heutiger Drahtkonsument bekommt daraus ein veraltetes Angebot. Tests für fehlenden/falschen Ziel-Main führen nach dem Wechsel neue Evidenz zu (`sonde014_verdrahtung.rs:3446-3474`); sie erzwingen keinen Wechsel zwischen Kopie und Einsetzen. Ein alter **bereits korrekt entstandener** persistierter Vorschlag darf dort ausdrücklich als Verlauf bleiben, weshalb seine bloße Existenz in `proposals` kein Defekt ist.

Vor Freigabe des Angebotswegs muss die echte `vorschlaege_bilden`-Naht mit Store geprüft werden: A sammelt Befund X, B bildet und persistiert nach Intent-/Führungswechsel Befund Y und dessen Proposal, A setzt danach ein; Cache, Projektion und mögliche Zustellschuld dürfen keine ältere **aktuelle** Empfehlung zeigen. Zusätzlich prüfen: leere Befundmenge (`hypothese_verdrahtung.rs:579-580`) gegen den frühen `None`-Rückweg von `vorschlaege_bilden` (`:40-44`), sowie Append-Ablehnung nach vorheriger Cacheübernahme. Diese zwei benachbarten Formen wurden quellseitig gefunden, aber nicht als neue heutige Benutzerfehler aufgenommen.

**Experimentresultate:** Das Plugin sendet über `sendeVersuchP0`/`sendePersistenzP0` (`prozessor/Analyse.cpp:777-786`, `:995-1009`). Die APIs sind gebaut, ihre Bedienung ausdrücklich noch nicht angeschlossen (`prozessor/State.cpp:434-443`). `experiment_p0` prüft bei `manual_result` die Resultatmessung nach Freigabe des ersten Standlocks (`experiment_verdrahtung.rs:165-171`); `experiment_p0_weiter` misst an `:224` **erneut**. Diese zweite Messung widerlegt einen Test, der nur die Lücke nach der ersten Vorprüfung angreift. Danach übernimmt `:275` das Ergebnis unter neuem Lock, `:309` persistiert, `:356-360` flusht den Snapshot. Der Session-Commit-Guard in `befehl.rs:170-174` liegt erst im Persistenzaufruf und schützt das vorherige Mess-/Anwendungsfenster nicht. `resultatmessung` filtert ausgeschlossene IDs korrekt (`:918-920`), das Terminal kopiert die gemessenen IDs (`experiment.rs:1244-1245`). Ohne erzwungenen Zwischenzeitpunkt an der zweiten Naht bleibt die End-to-End-Race-Reproduktion **NOT RUN**. Die bekannte frühe lokale Versuchsbeendigung vor ACK ist **NAK-169**, kein neuer Befund.

**PRE/POST:** Produktiver Eingang ohne `pair_id` bleibt T3-08-04 (`liveness.rs:741-755`, `prepost_verdrahtung.rs:85-91`). Die später nötige Interleaving-Prüfung muss die echten Halbzeuge vor Invalidierung halten und erst danach in den zweiten Lock lassen. `bilde_paare` läuft dort unter dem Lock (`:146-154`), aber auf der Kopie aus `:75-119`. Der vorhandene Ausschlusstest (`sonde013_verdrahtung.rs:2671-2761`) setzt die Pair-ID selbst und schickt nach der Invalidierung einen weiteren Snapshot; das ist ein gültiger sequenzieller Algorithmusbeweis, kein Beweis dieser Lock-Lücke und kein Anschlussnachweis. Beim Reparieren von T3-08-04 beide Grenzen zusammen prüfen.

## Weitere abgeglichene Wege

| Weg | Ergebnis und konkrete Grenze |
| --- | --- |
| Positiver ACK gegen Reconnect/Produktcallback | **T3-15-01 bleibt quellseitig bestätigt**: `Verbindung.cpp:834-841` verbraucht vor Generationsprüfung; `Nachrichten.cpp:363` löscht aus Inflight, `:264` spielt nur verbliebene Einträge; `Ipc.cpp:1067-1076` ist der lokale ACK-Verbraucher. Abschluss-Hook hat weiterhin nur Testaufrufer (`IpcTestMain.cpp:6951`, `:7009`, `:7138`). Kein neuer C++-Rennlauf in diesem Paket. |
| Negativer Experiment-ACK / frühes lokales Terminal | **NAK-169 abgedeckt**. Keine zweite ID; vor Bedienanschluss mit dem Ergebnis-/Ablehnungspfad prüfen. |
| Neue Evidenz während Hypothesenrechnung | Bestehender Evidence-ID-Mengenriegel vorhanden; NAK-253/NAK-283. Dieser Paketlauf beansprucht keine erneute Gesamtabnahme ihrer bestehenden Tests. |
| Mixerkanalwechsel auf bestehendem Link | Wirklicher produktiver Gegenweg in `liveness.rs:447-460` und `invalidierung_verdrahtung.rs:510-555`; verhindert eine pauschale Behauptung „Descriptoränderungen invalidieren nie“. Same-Nonce-Neu-Hello ist die oben neu geprüfte Kombination. |
| `state_report` als denkbarer Master-Evidenzersatz | Widerlegt an der Datenform: Revision/Hash/Recordstatus sind keine Featurehistorie. Keine Konversion in die von `fenster_aus_historie` benötigten Evidenzfenster. |
| Stärkerer Masterbefund allein durch mehr Probeeq-Belege | Im A-01-Gegenfall widerlegt: elf angenommene Probe-Belege, leere Main-Historie und weiter `KeinMasterbeleg`; die Rechenquelle verlangt ausdrücklich ein Main-Profil. |

## Ausgeführte Prüfungen und dauerhafte Artefakte

- [Cargo.toml](phase-16-nachweise/a/Cargo.toml), [Cargo.lock](phase-16-nachweise/a/Cargo.lock), [verdrahtung.rs](phase-16-nachweise/a/verdrahtung.rs).
- [lauf-pinned.txt](phase-16-nachweise/a/lauf-pinned.txt): finaler Lauf **4 Tests bestanden**, Assertions bestätigen die beschriebenen Fehl-/Gegenfolgen. Dies bedeutet **kein Produkt-PASS**.
- Eigener Buildbaum: `C:\na-audit16-aff2d818\build-a`; keine Pipe, kein Brokerprozess, keine Installation und kein Eingriff in FL.
- Der erste Probelauf löste Abhängigkeiten neu auf. Er ist **nicht** der verwendete Nachweis: danach wurde der Produkt-Lock übernommen, der Broker mit dessen Versionen neu gebaut und die endgültigen Fälle mit `--locked --offline` ausgeführt. Mengenvergleich von Name/Version/Source/Checksum: keine entfallene oder geänderte Produktabhängigkeit; genau das Audit-Rootpaket zusätzlich.
- [quellmanifest.json](phase-16-nachweise/a/quellmanifest.json) dokumentiert Hashes der gelesenen Kernquellen, Auditdateien und die Abhängigkeitsgleichheit. Produktdiff in der festen Kopie bleibt leer.

Reproduktion bei wiederhergestellter unveränderter Kopie desselben HEAD an demselben Pfad:

```powershell
cargo test --locked --offline --manifest-path C:\Users\phili\Projekte\Nakama\docs\audits\2026-09-15-tiefenaudit\roh\phase-16-nachweise\a\Cargo.toml --target-dir C:\na-audit16-aff2d818\build-a --test verdrahtung -- --nocapture --test-threads=1
```

**Nicht ausgeführt:** C++-Prozessor-/Transportharnisch, reale Plugin-Instanzen, Host-/FL-Nachweis, Proposal-Race mit Store, Experimentresultat-Race an der zweiten Messnaht, PRE/POST-Halbzeug-Race, vollständiger Kanon. Die Nachweisgrenzen gehören in die zentrale Integration. Dieser Bericht beansprucht keine Neuvalidierung aller Altbefunde.
