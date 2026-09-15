URTEIL: DEFEKTE 5 · LÜCKEN 0 · HÄRTUNGEN 0 — Der Broker besitzt fünf statisch belegte Brüche an Transaktions-, Evidenz- und Publikationsgrenzen sowie an der produktiven Paar- und Loop-Verarbeitung.

# Tiefenaudit 3 — Phase 08: Broker

| Kopf | Wert |
|---|---|
| Auftrag | Gebundener Prüfauftrag des Dirigenten, 2026-09-16 01:17 |
| Datum | 2026-09-16, Europe/Berlin |
| Branch / HEAD | master / 979c4b94e9a139f726252cc74b8309fe029b0e83 |
| Modell / Effort | gpt-6-astra / max, aus dem Startlog dieses Laufs |
| Start | 2026-09-16 01:17:24.6097829 +02:00 |
| Berichtsstand | 2026-09-16T01:52:12.084688+02:00 |
| Dauer bis Berichtsstand | 34 min 47 s; anschließende Artefakt-/HEAD-Kontrolle separat |
| Abschlusskontrolle | 2026-09-16 01:53:13 +02:00; 35 min 49 s seit Start; HEAD identisch, Tabellen wortgleich, BEFUNDE-Präfix bytegleich |
| Modus | Frischer lesender Prüfer; keine Unteragenten; ausschließlich zwei Audit-Schreibziele |
| Sicherheitsabgrenzung | Pipe-Sicherheit ist Phase 12; hier nur Korrektheit, Rückstau, Besitz und Lebensdauer |

## Beweistiefe und Prüfweg

Lesender Quellenaudit am festgelegten HEAD. Keine Ausführung von Rust-, C++-, Python-Produkt- oder Testfunktionen; kein Build, Clippy, Kanon, Brokerstart, SQLite-Schreibzugriff oder Installationslauf. Python wurde ausschließlich zum Lesen, lexikalischen Zählen und Aufbereiten von Quelltext verwendet. Die späteren Schreibzugriffe beschränken sich auf die zwei beauftragten Markdown-Artefakte. Es gibt keine neuen Laufzeitmesswerte und kein PASS aus alten Manifesten.

Die ganze vorgegebene Rust-Population wurde für Funktionen, unsafe, Sperren, Kanäle, Spawn-Stellen und Lint-Ausnahmen erfasst. Die fachliche Tiefenlektüre folgt den Transport-, Store-, Wiederanlauf- und Coordinator-Ketten. Das vollständige Stelleninventar und die Grenzen der inhaltlichen Lektüre stehen im abschließenden Scope-Beweis. Der Skeptikerpass lief durch denselben frischen Prüfer; es wurden keine Unteragenten gestartet.

Die Regeln stammen aus CLAUDE.md, den angenommenen Verhaltensmatrizen und dem aktuellen Code. Register, Manifeste und Gesundheitsdokumente wurden nur als Kontext und Gegenbeleg gelesen. Weder Dokumentationsabweichungen noch Legacy-Bezeichner sind Befunde.

## Befunde

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
|---|---|---|---|---|---|---|---|---|---|
| T3-08-01 | critical | K4 | DEFEKT | broker/src/store/writer.rs:278; broker/src/coordinator/befehl.rs:409; broker/src/coordinator/befehl.rs:189 | bekannt NAK-157: Ein gemeinsamer Append macht Befehlsriegel und Domänenwirkung nicht atomar; der Group-Commit kann auch einen Auftrag mit nur zwei Ereignissen an der gemeinsamen 64er-Grenze teilen, sodass nach einem Crash die Wirkung dauerhaft fehlt. | Statische Folge: 63 bereits gesammelte Ereignisse + [command, evidence_invalidate] → Commit 1 enthält command → Abbruch vor Commit 2 → command_event_lesen liefert beim Retry idempotent_wiederholt ohne erneute Wirkung; store_crash_matrix.rs:1701–1721 belegt im Testquelltext ausdrücklich die Aufteilung 63+2. | „B14 — eine Preview committet ihre Ruecknahme MIT sich, oder gar nicht.“ (broker/tests/sonde013_verdrahtung.rs:3126; angenommene Regel SONDE-013 §13.4/B14) | Fachlich atomare Append-Aufträge ungeteilt in eine Transaktion legen und die 63+2-Grenze mit Crash und Retry der echten Domänenwirkung absichern. | offen |
| T3-08-02 | high | K4 | DEFEKT | broker/src/coordinator/evidenz.rs:186; broker/src/coordinator/evidenz.rs:450; broker/src/coordinator/invalidierung_verdrahtung.rs:179; broker/src/store/writer.rs:656; broker/src/coordinator/experiment_verdrahtung.rs:1441 | Ein bereits im Coordinator ausgeschlossener Beleg kann anschließend ohne Ausschluss gespeichert werden und beim Wiederanlauf als gültig zurückkehren. | Statische Folge: P1 trägt E vorläufig in die Historie ein → paralleles preview_begin schließt E aus und committet die Rücknahme, während E noch nicht in SQLite steht → P1 persistiert nur ID, Empfangsfolge und Originalsnapshot → Restore findet keinen ausschlussgrund. Ein Rücksprung der Projektzeit zeigt dieselbe Cache/Store-Divergenz auch ohne Nebenläufigkeit. | „Bereits eingegangene Evidenz wird per ID oder Bereich invalidiert.“ (SONDE-013 M-52, docs/beweise/SONDE-013.md:547); „jede Zeile der Sitzung traegt den Grund - haltbar, nicht nur fluechtig“ (broker/tests/sonde013_verdrahtung.rs:1484) | Evidenzannahme und nachfolgende Invalidierungen pro Sitzung in derselben verbindlichen Reihenfolge persistieren, sodass keine betroffene Evidenz hinter ihrer Rücknahme unmarkiert eingefügt werden kann. | offen |
| T3-08-03 | high | K1 | DEFEKT | broker/src/coordinator/invalidierung_verdrahtung.rs:389; broker/src/coordinator/invalidierung_verdrahtung.rs:428; broker/src/coordinator/flush.rs:191; broker/src/transport/server_v3/queues.rs:228; broker/src/transport/server_v3/queues.rs:258 | bekannt NAK-269: Zwischen Commit und Einreihen kann eine ältere Rücknahme weiterhin erst hinter einem neueren Vollsnapshot eingereiht werden; die reparierte Koaleszierung schützt nur die bereits eingehaltene Einreihreihenfolge. | Statische Folge: Rücknahme N committet, ihr Thread pausiert vor snapshot_schreiben; ein anderer Thread committet und reiht Snapshot N+1 ein; anschließend wird Rücknahme N unter ihrem anderen Objektschlüssel angenommen. Ausgabe N+1,N; SourcesModel.cpp:1505–1518 setzt den jüngeren Stand wieder invalid/stale. | „Die Koaleszierung erhält die Entstehungsreihenfolge über Schlüssel: eine Ersetzung wandert ans Ende der Deque (Reihenfolge ist die Zeit der jüngsten Marke), sodass Vollsnapshot und Rücknahme desselben Objekts in ihrer Entstehungsreihenfolge zugestellt werden.“ (R-283-2, docs/beweise/NAK-283.md:112) | Commit und Publikationsauftrag für abhängige Schlüssel gemeinsam ordnen und erst das Warten auf den Pipe-Write außerhalb dieser Ordnung ausführen. | offen |
| T3-08-04 | high | K4 | DEFEKT | broker/src/coordinator/liveness.rs:741; broker/src/coordinator/liveness.rs:472; broker/src/coordinator/prepost_verdrahtung.rs:90; broker/tests/sonde013_verdrahtung.rs:1130 | Der v3-PRE/POST-Join bekommt produktiv keine pair_id: Der einzige Runtime-Descriptor-Writer lässt sie aus; nur Tests füllen sie über descriptor_setzen. Gültig gespeicherte PRE/POST-Paarzuordnungen erreichen dadurch weder den Join noch session_snapshot.paare. | Vollständige Suche nach pair_id und Descriptor-Zuweisungen in broker/src: Link startet mit descriptor=None; heartbeat.runtime ersetzt ihn ohne pair_id; der Join überspringt solche Quellen. Gegenprüfung des Senders: SondeProcessor.cpp:1100 und controlclient/Intern.h:412 liefern ebenfalls keine Paar-ID; der v2-Registerpfad ist getrennt. | „Reihenfolge: Descriptor validieren → Paarkandidat bilden → Vollständigkeit prüfen → erst dann eine Paaraussage erzeugen.“ (SONDE-013 M-13, docs/beweise/SONDE-013.md:488); „B25: der PRE/POST-Join liefert ein Urteil je Paar (M-13/M-14)“ (broker/tests/sonde013_verdrahtung.rs:1154) | Die vorhandene Paarzuordnung über einen versionierten v3-Eingang bis zum Descriptor führen und den Join über echte Hello-/Heartbeat-/Evidenznachrichten statt den Testsetter prüfen. | offen |
| T3-08-05 | medium | K4 | DEFEKT | broker/src/coordinator/invalidierung_verdrahtung.rs:628; broker/src/coordinator/invalidierung.rs:146; broker/tests/sonde013_taint.rs:127 | Auch ein Loop-Wrap genau an der gebundenen Passagengrenze invalidiert im Broker die abgeschlossene Evidenz; die vorhandene Ausnahme LoopWrapAnGrenze wird im produktiven Pfad nie benutzt. | Statische Folge: vollständiger Durchlauf [0,L), dann nächster gültiger Evidenzsnapshot nach dem Wrap mit höherer transport_epoch → invalidierung_aus_transportbruch wählt bedingungslos Epochwechsel und Bereich [0,altes Ende). rg findet grund_fuer_bruch nur als Definition und in sonde013_taint.rs; der echte Frame-Erzeuger erhöht beim Loop-Wrap die Epoche (Zeit.h:222/:516, Frame.h:221). | „Ein Loop-Wrap **an** der Passagengrenze (Loop deckt die Passage vollständig) beendet einen Durchlauf und beginnt den nächsten — das ist **keine** Invalidierung und **keine** Herabstufung (Abgrenzung zu M-23).“ (SONDE-013 M-53, docs/beweise/SONDE-013.md:548) | Die Transportbruchentscheidung im Evidenzempfänger mit der gebundenen Passage und dem belegten Loopgrund verknüpfen und die Grenzausnahme dort prüfen. | offen |

Zähler: **critical 1 · high 3 · medium 1 · low 0**. Kategorien: **DEFEKT 5 · LÜCKE 0 · HÄRTUNG 0**. Alle fünf offen; kein Fix ausgeführt.

## Herleitungen und Skeptikerpass je Befund

### T3-08-01 — Ein Append ist keine Transaktionsgrenze

**Erreichbare Eingabe.** Ein regulärer persistenter P0-Auftrag mit eigener Domänenwirkung, beispielsweise `preview_begin`, läuft gleichzeitig mit anderen Store-Produzenten. Es braucht keinen großen einzelnen P0-Auftrag: Zwei zusammengehörige Ereignisse genügen. Der gemeinsame Writer darf innerhalb seines 50-ms-Fensters bereits 63 Ereignisse anderer Aufträge gesammelt haben.

1. `befehl.rs:409–435` erzeugt zuerst das Ereignis vom Typ `command` mit `command_id`, danach die Domänenereignisse. `:437` übergibt den Vektor gemeinsam.
2. `store/writer.rs:278–283` nimmt aus **jedem** Auftrag nur so viele Ereignisse, wie bis zur gemeinsamen Grenze 64 noch passen. Bei 63+2 wandert nur der Befehlsriegel in den ersten Commit.
3. `:325` führt diese Gruppe als eigene Transaktion aus. Erst ein späterer Schleifendurchlauf persistiert die Restwirkung.
4. Ein Brokerabbruch zwischen beiden Commits hinterlässt einen gültigen Command-Eintrag ohne Rücknahme. Der erfolgreiche erste Commit wird durch den zweiten Auftragsteil nicht zurückgerollt.
5. Beim Retry findet `befehl.rs:189–209` die gespeicherte Command-Wirkung und quittiert sofort `idempotent_wiederholt`. Der Domänenappend wird nicht nochmals ausgeführt.

**Wirkung.** Dauerhafter Verlust einer fachlichen Wirkung bei gleichzeitig dauerhaftem Idempotenzriegel. Der Satz in `befehl.rs:414–419`, ein unter 64 Ereignissen liegender Auftrag sei deshalb unteilbar, wird von der Writer-Schleife widerlegt. Ein Fehler des zweiten Commits kann denselben Teilzustand hinterlassen; dafür wird hier keine Laufzeitreproduktion behauptet.

**Gegenbelege gelesen.** `store_crash_matrix.rs:1689–1722` verlangt die harte 64er-Grenze und konstruiert sogar 63+2; die Ereignisse dort bilden keine fachliche Command/Wirkungs-Einheit. `sonde013_verdrahtung.rs:3137–3184` misst benachbarte Ordinale nach störungsfreiem Abschluss. Zwei benachbarte Ordinale können in zwei Transaktionen stehen. Auch `experimentbefehl_und_wirkung_sind_ein_append` prüft den abgeschlossenen Zustand, nicht den Zwischencommit.

**Registerabgleich.** NAK-157 kennt den fehlenden Killmatrix-Beweis und bezeichnet den Produktpfad bislang als korrekt. Neu ist der konkrete, aus dem unveränderten Writer zwingend folgende Teilcommit. Keine zweite Meldung allein für den fehlenden Test.

**Kleinste spätere Prüfung.** Original-Writer mit 63 vorangehenden Ereignissen, echter Preview mit Rücknahme, Halt unmittelbar nach erstem Commit, Restart und Retry; Command und Rücknahme müssen gemeinsam vorhanden oder gemeinsam abwesend sein. Nicht ausgeführt.

### T3-08-02 — Vorläufige Evidenz wird nach ihrer Rücknahme sauber eingefügt

**Erreichbare Nebenläufigkeit.** Der Transport besitzt einen P0-Verbraucher und einen getrennten P1/P2-Verbraucher (`verbindung.rs:593/:622`); mehrere Links liefern zusätzlich parallel. Der Standmutex schützt die Aufnahme in den Cache, aber nicht die Reihenfolge der späteren Store-Aufträge.

| Schritt | P1-Evidenzpfad A | P0-Pfad B / Store |
|---|---|---|
| 1 | Vertragsgültiges E mit neuer ID passiert das freie Taint-Gate und wird unter dem Standmutex eingefügt (`evidenz.rs:176–191`). Danach Pause. | E steht noch nicht in SQLite. |
| 2 | — | Ein erlaubtes `preview_begin` bereitet die Rücknahme der Sitzung vor (`befehl.rs:890–898`); `invalidierung_vorbereiten:166–179` markiert E. |
| 3 | — | Die Rücknahme committet. `writer.rs:656–665` liest ausschließlich schon vorhandene Evidence-Zeilen; E fehlt noch. |
| 4 | A läuft weiter. `evidenz_persistieren:450–467` schreibt ID, Empfangsfolge und unveränderten Wire-Snapshot. | E wird **ohne** den inzwischen gesetzten Ausschlussgrund angelegt. |
| 5 | Cache: E ausgeschlossen. | SQLite: E ohne Ausschluss. Restore `experiment_verdrahtung.rs:1438–1455` macht daraus wieder einen gültigen Historieneintrag. |

Der P1-Pfad prüft nach dem Store-Append weder die inzwischen erfolgte Rücknahme noch ergänzt er den Ausschluss in der gespeicherten Zeile. Der Wiederanlauf rekonstruiert die Evidenz aus der Projektion; er spielt die früheren Invalidierungen nicht nachträglich auf eine später eingefügte Zeile an.

**Serieller Gegenfall derselben Persistenznaht.** Alter Beleg `[48000,96000)`, danach ein neuer Beleg `[0,48000)` in neuer Transportepoche, beide mit verschiedenen gültigen IDs und 48 kHz: `evidenz.rs:197` löst die Bruchinvalidierung bereits **vor** dem Append des neuen Belegs aus. Der Bereich `[0,96000)` aus `invalidierung_verdrahtung.rs:648–650` trifft im Cache auch den schon eingefügten neuen Beleg, im Store aber nur den alten. Die nachfolgende Ablage enthält den neuen Ausschluss wiederum nicht. Hier ist außerdem die Auswahl nach Projektbereich statt nach alter Epoche sichtbar; sie ist kein zweiter Befund zur selben Divergenz.

**Skeptikerpass.** Der gemeinsame Lock aus M-63 hält Taintprüfung und Cache-Insert zusammen; er reicht nicht über den Store-Auftrag. `invalidierung_projiziert_ihren_grund_in_die_evidenzzeilen` (`sonde013_verdrahtung.rs:1458`) invalidiert drei **bereits gespeicherte** Belege. `transportbruch_invalidiert_mit_seinem_grund` (`:1072`) prüft Zähler und irgendeinen ausgeschlossenen Beleg, nicht Cache/DB-Gleichheit des gerade aufgenommenen Belegs. NAK-287(c) braucht doppelte Evidence-IDs; dieser Befund ausdrücklich nicht. F14/NAK-163 betraf den Fehler-Rollback auf verschobenen Positionen; hier gelingen alle Appends und die IDs sind eindeutig.

**Kleinste spätere Prüfung.** Halt nach Cache-Insert und vor Evidence-Append; echte Preview dazwischen; anschließend Cache, Evidence-Projektion und zweiter Coordinator auf derselben DB vergleichen. Zusätzlich den seriellen Rücksprung fahren. Nicht ausgeführt.

### T3-08-03 — Commit-Ordnung geht vor dem Einreihen über Schlüssel verloren

Die Reparatur von F03 ist an der Queue vorhanden: Bei Ersetzung wird der alte Eintrag entfernt und der neue hinten angehängt. Sie setzt voraus, dass die Publikationsaufträge überhaupt in fachlicher Reihenfolge in dieser Queue ankommen.

1. A committet `evidence_invalidate` mit Ordinal N (`invalidierung_verdrahtung.rs:389–395`).
2. A pausiert vor `invalidierung_zustellen`/`snapshot_schreiben` (`:405–428`).
3. B erzeugt einen späteren Vollstand derselben Sitzung, committet ihn und reiht `session_snapshot` mit N+1 ein (`flush.rs:158/:191/:213`). Der Session-Flush-Mutex gilt nur dem Flush und endet vor dem Pipe-Auftrag.
4. A setzt fort. Das Hochwasser in `queues.rs:228` ist nach **Objektschlüssel** getrennt. N ist deshalb kein Nachzügler zum Hochwasser N+1 des anderen Schlüssels.
5. Die Queue liefert N+1, N. Auch wenn N+1 bereits geschrieben wurde, wird N noch angenommen. Ein bloßes Sortieren der aktuell wartenden Einträge wäre deshalb kein vollständiger Fix.

**Verbraucherwirkung als Referenz.** `SourcesModel.cpp:1494–1518` kennt keinen Altersvergleich für diese Rücknahme und setzt Messungen sowie Befunde zurück. Der Vollsnapshot darf zwischenzeitlich frische, nach der Rücknahme gerechnete Befunde enthalten. Die Zustellung des älteren Ereignisses kann diese erneut als stale markieren. Die Pipe schreibt zwar seriell, aber bereits in der falschen Einreihfolge.

**Skeptikerpass.** R-283-2, M-17 bis M-20 und die zugehörigen Queue-Tests decken Ersetzung innerhalb der Queue ab. `tests_abonnement.rs:1039–1052` wartet ausdrücklich, bis die Rücknahme eingereiht ist, und startet **danach** den Vollsnapshot; genau das strittige Fenster fehlt. Die Same-Key-Hochwasserwache bleibt korrekt. Der Befund braucht weder Store-Degradation (NAK-274 E5-2), fehlgeschlagenen Resubscribe (NAK-165) noch einen neuen Transport.

**Registerabgleich.** Bekannt NAK-269 ist der noch offene Consumer-Altersriegel. Neu ist eine weitere konkrete brokerseitige Umordnung vor der reparierten Queue. Der minimale Fix muss abhängige Publikationen ordnen; eine pauschale globale Verwerfung älterer Rücknahmen wäre ohne vollständigen Umfangsvertrag nicht ausreichend.

### T3-08-04 — Der v3-Paarpfad beginnt erst in den Tests

Die Population der Descriptor-Schreibwege ist klein und vollständig verfolgt:

| Schreibweg | Ort | Paar-ID |
|---|---|---|
| Control-Anmeldung / Reconnect | `link.rs:253–257` | Descriptor wird `None`. |
| Produktiver Runtime-Heartbeat | `liveness.rs:407–408/:471–472` | Vollständiger Ersatz durch `descriptor_aus_heartbeat`. |
| Konstruktion aus Runtime | `liveness.rs:714–763` | Adresse, Kind, Messpunkt, Betrieb, Label, Capabilities, Frische, optional Hostname/Index; keine `pair_id`. |
| Öffentlicher Descriptor-Setter | `liveness.rs:804–909` | Übernimmt eine `pair_id`, hat aber unter den produktiven Broker-Aufrufern keinen Verwender. |
| PRE/POST-Eingang | `prepost_verdrahtung.rs:90–92` | Überspringt jede Quelle ohne `pair_id`. |

**Produzent und Vertrag gegengeprüft.** Der echte `SondeProcessor::v3Status` (`SondeProcessor.cpp:1100–1140`) übernimmt den Messpunkt und das Label, nicht die gespeicherte Paar-ID. `runtimeJson` (`core/ipc/controlclient/Intern.h:412–434`) schreibt dieselbe reduzierte Form. Das geschlossene `heartbeat_runtime`-Schema (`eq-ipc-v3.schema.json:744–755`) hat auch kein Feld dafür. Dagegen führen die Probe-Descriptor-Zweige `pair_id` bereits. Ein bloßes zusätzliches JSON-Feld ohne Versionsarbeit wäre daher kein zulässiger Fix.

Der alte v2-Sensorpfad besitzt die Paar-ID (`protokoll.rs:54`, `register.rs:121`, `paar.rs:81`); seine separate Aggregatauswertung befüllt nicht den v3-Coordinator. Die dort funktionierende Paarung widerlegt die fehlende v3-Verbindung nicht.

**Skeptikerpass.** `prepost_join_laeuft_im_produktpfad` setzt beide Descriptoren direkt (`sonde013_verdrahtung.rs:1130/:1134`) und prüft dann ein Urteil (`:1154`). Dasselbe Muster tragen die anderen Paar-Verdrahtungstests. Die Suche nach `descriptor_setzen`, `pair_id` und allen Descriptor-Zuweisungen findet keinen alternativen produktiven Writer. Das ist keine neue Designforderung: M-13 nimmt den analytischen v3-Paarpfad einschließlich Rückweg über `session_snapshot.paare` bereits ab.

**Grenze.** Kein FL-/UI-Lauf. Aus der Quellenkette folgt das Ausbleiben der v3-Paarmetadaten; nicht behauptet wird, die getrennte v2-Paaranzeige sei ebenfalls leer. Die unten festgehaltenen Cache-Verdachte werden wegen dieser fehlenden produktiven Vorbedingung nicht als zusätzliche Defekte gezählt.

### T3-08-05 — Die Passagengrenzen-Ausnahme erreicht den Empfänger nicht

Die angenommene Regel unterscheidet drei Fälle: Seek, Loop-Wrap **innerhalb** einer Passage und Loop-Wrap **an** ihrer Grenze. Nur der letzte beendet einen gültigen Durchlauf ohne Invalidierung.

Der echte Erzeuger klassifiziert eine rückspringende, gültige Projektzeit als `loopWrap` (`featureengine/Zeit.h:213–222`) und hebt dafür die Transportepoche (`:509–518`). `Frame.h:221` überträgt diese Epoche. Der Broker vergleicht dagegen in `invalidierung_aus_transportbruch:626–635` ausschließlich Epoche und Segment. Er liest weder Passagengrenze noch Loop-Klassifikation und erzeugt bei jeder neuen Epoche eine Rücknahme. Der Bereich bis zum Ende des alten Fensters erfasst den abgeschlossenen Durchlauf.

**Skeptikerpass.** `grund_fuer_bruch(LoopWrapAnGrenze)` liefert korrekt `None` (`invalidierung.rs:146–154`). Die vollständige Aufrufsuche findet jedoch ausschließlich `sonde013_taint.rs`; dessen `:127` misst die reine Zuordnung. Der produktive Verdrahtungstest `sonde013_verdrahtung.rs:1072` behandelt jede neue Epoche als rückzunehmenden Bruch. Es gibt keine Weitergabe der bereits formulierten Ausnahme an den Eingang. NAK-296 betrifft die zurückgesetzte lokale Lautheits-/Kalibrationsdauer des Diagnoseprojekts, nicht diese Broker-Rücknahme.

**Abgrenzung zu T3-08-02.** Dieser Befund gilt auch bei perfekt geordneter und atomarer Persistenz: Bereits vollständig gespeicherte Evidenz wird fachlich fälschlich ausgeschlossen. T3-08-02 betrifft die unterschiedliche Wirkung derselben Rücknahme in Cache und SQLite.

## Querschnitt: Antworten auf die acht Prüffragen

| Prüffrage | Quellenbefund dieser Phase | Grenze |
|---|---|---|
| 1. Ordnung, Commit, Rücknahme, Heilung | T3-08-01/02/03. F02-Frischevergleich und F14-ID-Rückweg sind vorhanden; R-283-7/NAK-287 bleiben bekannter Vorlauf. | Keine gemessene Heilungsrate unter dichtem Mehrinstanzstrom. |
| 2. Store und Persistenz | Ein Writer besitzt die Write-Connection; Transaktionen, Projektionen, Outbox-Kompaktierung, Migration, Checkpoint und Dateiersatz gelesen. Der entscheidende neue Atomaritätsbruch liegt oberhalb der einzelnen SQLite-Transaktion. | Keine echte Crash-/WAL-Recovery oder Plattenfehlerinjektion; kein Lauf der Abbruchmatrix. |
| 3. Nebenläufigkeit | Vollständiges Stelleninventar: 350 `.lock()`-Aufrufe, 111 `Mutex<…>`-Typstellen, kein `RwLock`; Stand-/Alias-/Flush-/Writer-/Transport-Sperren und Aufrufgrenzen verfolgt. P0 und P1 dürfen denselben Coordinator parallel betreten. | Stelleninventar ist kein formaler Deadlockfreiheitsbeweis; bekannte fristlose Stopppfade bleiben NAK-268. |
| 4. Frames und Rückstau | Bootstrap, Stromleser, Envelope, Familienwahl, Ratengrenze, Bytebudget, P0/P1/P2-Politik und Writerantwort geprüft. Volle P0/P1-Eingänge trennen; P2-Verwurf wird gezählt. | Kein Fuzz-, Last- oder Reconnect-Sturm-Lauf; Schema-Nachbefunde der Phase 03 nicht dupliziert. |
| 5. unsafe und FFI | 108 Blöcke, 12 unsafe-impls und 5 unsafe-Funktionsdefinitionen erfasst; Eigentümer, Drop, Cancel/Drain, Overlapped-Puffer und Fehlerpfade gegengelesen. Kein zusätzlicher zwingend belegter Lebensdauer-/UB-Befund. | Keine Aussage zur Pipe-Sicherheit; Auth-/ACL-Kanten nur auf Besitz und Freigabe betrachtet. Keine Miri-/Sanitizer-/Handle-Laufzeitmessung. |
| 6. Zahlen und Verträge | Rust-Schemaengine, DTO-/JSON-Lesegrenzen, FlatBuffer-Verifier plus Strukturriegel, Endlichkeitsprüfungen, Zähler-/Ordinaldarstellung und leere Mengen geprüft. T3-08-04/05 zeigen fehlende fachliche Informationen am Eingang. | NAK-287(b)/(c) bleibt bekannt; kein neuer erreichbarer NaN-PartialEq-Fall oder doppelter Produktionssender belegt. Keine komplette mathematische Falsifikation aller Fachalgorithmen. |
| 7. Beobachtbarkeit | `Register::fehler_merken:438–443` hält 50 Fehler; Diagnose-Umschlag verwendet bewusst das ungefilterte Aggregat. Verweigerungs-, Überlauf-, Cancel- und Storezähler an ihren Besitzern gelesen. | Der fehlende produktive Leser des Queue-Nachzüglerzählers ist NAK-274 E5-1; sensible Inhalte gehören Phase 12. |
| 8. Cargo/Clippy | Rustc-`unsafe_op_in_unsafe_fn=deny`; Clippy-`undocumented_unsafe_blocks=deny`. Der optionale Gesundheitslauf nutzt `--cap-lints warn` und die Ratsche 91. Alle acht lokalen Allow-Attribute unten ausgewiesen. | Nicht ausgeführt. Die historische Zahl 94 wird nur als NAK-299 referenziert, nicht als heutige Messung übernommen. |

### Sperren, Kanäle und Lebensdauer

- **Coordinator → AliasRegister:** Anmeldung, Heartbeat und Dispatch halten stellenweise den Standmutex und nehmen den Aliasmutex. Das AliasRegister ruft nicht zurück in den Coordinator (`instance_alias.rs:123–378`). In dieser Kante kein belegter inverser Erwerb.
- **Session-Flush → Stand / Store:** `flush.rs:60–63` serialisiert die Sessionaufnahme und den Commit; `:191` gibt vor externem Push frei. Das vermeidet eine Sperre um reentrante Pipe-Senken, ermöglicht aber T3-08-03 über die getrennten Publikationspfade.
- **Hypothesen-Übernahme → nicht blockierende Store-Annahme:** `hypothese_verdrahtung.rs:663–686` hält Wiedervergleich und `append_einreihen` zusammen. Auf die Writerantwort wird nach Freigabe gewartet. Das ist die konkrete R-283-1-Wache.
- **Handle-Register:** Austragen und Cancel-Zuordnung laufen unter derselben Sperre (`win_handles.rs:490/:625/:637`); Schließen folgt dem Austrag. Der rohe Windows-Wert ist damit nicht allein die Lebensdaueridentität.
- **Kanäle:** Der Store hat einen begrenzten Arbeitskanal (`writer.rs:53`), seine fünf Antwortkanäle in `handle.rs:165/:174/:187/:201/:211` tragen je eine Antwort. Writer- und Bootstrap-Antwortkanäle sind begrenzt (`queues.rs:191`, `verbindung.rs:60`). Die übrigen Kanalstellen sind Testbarrieren oder STOP-Eingänge der Probe-Binaries.
- **Threads:** Acceptor, Wachhund, Verbindungsreader/-writer, P0/P1-Verbraucher, Storewriter, Supervisor und Diagnosebriefkasten haben explizite Besitzer und Stoppwege. Die bekannten fehlenden Gesamtfristen werden nicht durch die begrenzten inneren Waits automatisch bewiesen; NAK-268 bleibt stehen. Die Spawn-Liste enthält auch Testprozesse und ist deshalb keine Threadanzahl.
- **Persistente Löschung:** Outbox-Kompaktierung ist von Ereignis-/Domänenbestand getrennt. Im geprüften Storepfad kein neuer Nachweis einer ungefragten Archivlöschung. Datenbankwachstum und lange Wiederanlaufzeit wurden nicht gemessen.

### Alle lokalen Lint-Ausnahmen

| Ort | Abgeschaltete Regel | Begründung an der Stelle |
|---|---|---|
| `broker/src/coordinator/experiment.rs:831` | `clippy::too_many_arguments` | Fachlicher Restore erklärt; kein eigener Satz, warum gerade die Argumentzahl ausgenommen wird. |
| `broker/src/coordinator/experiment.rs:1026` | `clippy::too_many_arguments` | Begin-Reihenfolge erklärt; kein eigener Ausnahmegrund zur Argumentzahl. |
| `broker/src/coordinator/hypothese/befund.rs:172` | `clippy::too_many_arguments` | Kein lokaler Ausnahmegrund. |
| `broker/src/transport/server_v3/verbindung.rs:147` | `clippy::too_many_arguments` | Kein lokaler Ausnahmegrund. |
| `broker/src/coordinator/hypothese/screening.rs:224` | `clippy::neg_cmp_op_on_partial_ord` | Explizit mit dem Fail-closed-Verhalten von NaN begründet, `:219–223`. |
| `broker/src/transport/server_v3/griff.rs:440` | `dead_code` | Der gehaltene WorkerPlatz führt über Drop die Freigabe aus; Besitzgrund steht `:436–438`. |
| `broker/tests/sonde014_gegenbeispiele.rs:219` | `dead_code` | Gehaltenes Writer-Objekt im Testharnisch; kein ausdrücklicher Ausnahmegrund am Feld. |
| `broker/tests/sonde014_gegenbeispiele.rs:221` | `dead_code` | Gehaltener Testordner im selben Harnisch; kein ausdrücklicher Ausnahmegrund am Feld. |

Die vier Argumentzahl-Ausnahmen und die zwei Testfelder ohne ausdrücklichen Kommentar sind als Antwort auf Prüffrage 8 dokumentiert, **keine kosmetischen Härtungsbefunde**. Die generierte Clippy-Freistellung gehört nicht zur eigenen Codepopulation. Das Herabsetzen von Clippy-Deny auf Warnungen im optionalen Runner ist bereits NAK-237/NAK-299, nicht ein neuer Fund gegen einen Runner.

## Widerlegte Verdachte

„Widerlegt“ bedeutet hier: Der jeweilige Verdacht trägt **keinen weiteren heutigen Produktbefund**. Bei V01/V02 bleibt der Komponentenmechanismus für die spätere Reparatur von T3-08-04 relevant; die nötige produktive Vorbedingung fehlt heute.

| Nr. | Verdacht | Gegenbeleg / Grenze |
|---|---|---|
| V01 | Ein produktives v3-Paar bleibt nach Disconnect oder Stale als gültig stehen. | `liveness.rs:211–223` und `link.rs:559–567` setzen tatsächlich keinen Paarmerker; `sicht.rs:622–635` würde den Cache ausgeben. Der produktive Descriptor trägt aber keine Paar-ID (T3-08-04), somit fehlt das vorausgesetzte Paar. Nur über Testsetter herstellbar; keine weitere heutige Defektzeile. |
| V02 | Eine ältere PRE/POST-Aufnahme überschreibt nach Invalidierung das neue Paarurteil. | Aufnahme `prepost_verdrahtung.rs:76–119`, spätere ungeprüfte Übernahme `:146–155` ergeben dieses Komponentenfenster. Wieder fehlt produktiv die Paar-ID; nicht als erreichbarer Normalpfad verkauft. Beim Anschließen des Paarpfads Generation/Revalidierung und Lebenslauf mitprüfen. |
| V03 | Der alte F02-Gegenfall mit neuer Evidenz überschreibt weiterhin die jüngere Hypothese. | `hypothese_verdrahtung.rs:1213–1214` vergleicht jetzt die ganze Eingangs-ID-Menge; `:668–678` prüft den Standeintrag nochmals unter dem Guard vor Writerannahme. Die NAK-283-Tests in `sonde014_verdrahtung.rs` erzwingen beide Fenster. Kein Laufzeit-PASS daraus abgeleitet. |
| V04 | Retention verschiebt weiterhin das Rollbackziel auf einen anderen Beleg (alter F14). | `invalidierung_verdrahtung.rs:180–182/:252–263` speichert und sucht stabile Evidence-IDs statt Deque-Positionen. Doppelte IDs sind NAK-287(c); T3-08-02 benutzt eindeutige IDs und einen anderen Mechanismus. |
| V05 | Die alte F03-Ersetzung bleibt an der alten Deque-Position. | `queues.rs:251–252` entfernt und hängt hinten an, unter derselben Sperre. T3-08-03 beginnt schon vor dem Queuezugang und widerlegt diesen Fix nicht. |
| V06 | Ein älterer Snapshot desselben Schlüssels kann nach Leerung der Queue wieder angenommen werden. | Hochwasser `queues.rs:228–236` bleibt getrennt vom Deque-Inhalt erhalten; kleinere Marke wird gezählt und abgewiesen. Der ausdrücklich bekannte Degradationssonderfall NAK-274 E5-2 ist davon getrennt. |
| V07 | Overlapped-Read gibt den Stackpuffer beim Cancel sofort frei. | `win_handles.rs:297–307` wartet nach Cancel mit GetOverlappedResult auf das Operationsende. Listener-Overlapped liegt in einem besessenen Box-Puffer; die Verbindungsgriffe bleiben über Arc lebend. Kein bloßes Cancel-ohne-Drain an dieser Kante. |
| V08 | Ein recycelter Handlewert wird über einen alten Registereintrag gecancelt. | Austrag `win_handles.rs:490–494`, danach Close `:513`; Cancel `:625–641` unter demselben Registermutex. Die Zuordnung wird nicht vor der Operation kopiert und danach ungeschützt benutzt. |
| V09 | Die Ausgangsqueue liefert nach Schließen noch ihre alten Einträge aus. | `queues.rs:328–338` setzt Schließflag und entnimmt den gesamten Inhalt atomar; offene Schreibantworten werden false. `entnehmen` kann daher nicht Schließflag plus alten Restbestand beobachten. Der fehlende Spiegeltest ist bekannt NAK-268 B-3. |
| V10 | „Neu anlegen“ ersetzt bei einer Namenskollision eine bestehende Datei oder veröffentlicht Teildaten. | `persistenz.rs:60–68` schreibt und synchronisiert den Temp-Inhalt; `:98–104` setzt REPLACE_EXISTING nur bei ausdrücklich gewünschtem Ersetzen. Der Neuanlagepfad verwendet diese Option nicht. Keine Stromausfallmessung. |
| V11 | Ein fehlgeschlagener Proposal-Append führt heute zu einem produktiven Draft-Angebot. | `proposal_verdrahtung.rs:108` setzt Capability im aktuellen Produktpfad false; `proposal.rs:942–954` sperrt damit jedes Draft-Angebot. Das isolierte spätere Sendefenster ist aktuell unerreichbar; vorhandener Vorlauf NAK-207. |
| V12 | Wiederholte Fehler lassen die Register-Fehlerliste unbegrenzt wachsen. | `register.rs:438–443` entfernt bei 50 Einträgen den ältesten, bevor der nächste hinzukommt. Die begrenzte Liste ist kein unbegrenztes Journal und behauptet dies auch nicht. |

**Zähler widerlegte Verdachte: 12.**

## Bekannter Vorlauf — nicht erneut gezählt

| Register / Vorphase | Abgleich am aktuellen Quellpfad |
|---|---|
| NAK-160 | `experiment.rs:1339–1340` nimmt Loglänge und globale Folge zurück; `experiment_verdrahtung.rs` wartet außerhalb des Standlocks. Das ist bereits genau der registrierte sitzungsübergreifende Rollbackfall. |
| NAK-149, NAK-268 B-1/B-2/B-4 | Store-Antwort und Supervisor-Join besitzen keine gemeinsame Gesamtfrist; `lebenslauf.rs:78`, `store/writer.rs:128`, Stopp-Callbacks des V3-Servers. Kein neuer Laufzeitnachweis. |
| NAK-165 | Outbox-Replay nach fehlgeschlagenem absolutem Resubscribe bleibt ein bekannter Fall. Getrennte Queue-Schlüssel sind dagegen gebaut. |
| NAK-274 E5-1 bis E5-4 | Nachzüglerzähler nicht in V3Statistik, Degradationsrand, Hochwasserkartenwachstum und alter Link/Timeout-Weg sind im Register. Diese Phase fügt dafür keine Schätzwerte oder theoretischen Härtungen hinzu. |
| NAK-287(a) | Preview-Rollback `befehl.rs:912–917` hat weiterhin keinen eigenen Heilungsanstoß; der allgemeine Invalidierungsrückweg ruft ihn (`invalidierung_verdrahtung.rs:126`). |
| NAK-287(b)/(c) | PartialEq-NaN und wiederholte Evidence-ID sind bekannte Bedingungen. Der angenommene Evidenzweg und seine Zahlenriegel wurden gelesen; keine zusätzliche reguläre Erzeugerkette zu diesen beiden Randfällen belegt. |
| NAK-287(d)/(e), R-283-7 | Keine neue Lastmessung. Heilung nach Material/Verwurf kann weiterhin mehrere Sitzungen rechnen; kein erfundener sitzungsscharfer Vertrag. |
| NAK-153/NAK-237/NAK-299 | Cargo-Deny, Kommentarwache und optionaler Clippy-Zählweg sind verschiedene Detektoren. Ratsche 91 aus Code, historische Überschreitung aus Register; Clippy nicht gelaufen. |
| T3-02-03, Phase 03 | Bereits berichtete Zahlen-/Schemaengine-/Bootstrap-Differenzen bleiben dort; keine Neunummerierung. |
| T3-04-04, T3-07-03 | Diagnose-Stoppfenster und spätere Modellalterung einer Rücknahme bleiben in den Vorphasen. T3-08-03 betrifft die vorgelagerte brokerseitige Reihenfolge. |

## Produktfragen

Keine neue Produktfrage. Die fünf aufgenommenen Fälle haben bereits eine angenommene Verhaltensregel. Die Wahl eines technisch geeigneten Publikations- oder Atomaritätsmechanismus ist keine Produktentscheidung.

**Zähler Produktfragen: 0.**

## Nicht geprüft

- Keine frische Ausführung von Tests, Crashmatrix, Clippy, Gesundheitsmaß, Fuzzing, Last-/Soaklauf, Kanon oder Installation; auch keine manuellen SQLite-Schreibexperimente. Alle vorgeschlagenen Gegenläufe bleiben Arbeit für die Nacharbeit.
- Pipe-Sicherheit, ACL, Security-Descriptor-Inhalt, Impersonation-Autorisierung, Session-Token/Client-Identität, Signaturen und Broker-Spawn-Vertrauen: Phase 12. Lebensdauer der dort verwendeten Handles ist im unsafe-Inventar enthalten.
- Kein vollständiger semantischer Einzelbeweis aller 2.573 Funktionsdeklarationen und aller Testassertionen. Die vollständige Population wurde gezählt und verortet; vollständige mathematische und statistische Falsifikation aller Hypothesen-/PREPOST-Algorithmen sowie freilaufende Nebenläufigkeit sind damit nicht bewiesen.
- Generierter FlatBuffer-Code nur als Bezug zum handgeschriebenen Verifier; keine Neugenerierung und kein vollständiger Audit der Drittbibliotheken oder des Win32-Kernels.
- Kein Audio-/Nulltest-, Host-/FL- oder UI-Sichtbeleg. Die referenzierten C++-Stellen dienen ausschließlich dem Nachweis der Broker-Eingänge und -Ausgänge; Design bleibt geparkt.

## Scope-Beweis

### Umfang und Zählregel

**113 beauftragte Dateien vorhanden und lesbar:** 89 Rust-Dateien unter `broker/src/` ohne `generiert/`, 23 unter `broker/tests/`, dazu `broker/Cargo.toml`. Rust zusammen **83.389 Zeilen**, Cargo **82 Zeilen**, insgesamt **83.471 Zeilen**. Das ist eine Quellpopulation, keine Zahl geprüfter Assertions oder fehlerfreier Zeilen.

| Teilbaum | Dateien | Rust-Zeilen | Benannte fn-Deklarationen |
|---|---:|---:|---:|
| `broker/src/ (direkt)` | 20 | 11150 | 454 |
| `broker/src/bin/` | 4 | 550 | 18 |
| `broker/src/coordinator/` | 39 | 23979 | 598 |
| `broker/src/store/` | 5 | 2695 | 90 |
| `broker/src/transport/ (ohne server_v3/)` | 6 | 2318 | 124 |
| `broker/src/transport/server_v3/` | 15 | 7749 | 284 |
| `broker/tests/` | 23 | 34948 | 1005 |
| **Rust gesamt** | **112** | **83389** | **2573** |

Die ganze Population wurde per `rg --files broker/src broker/tests` und lesendem `python -X utf8 -c` erfasst. Der lexikalische Zähler maskiert Kommentare einschließlich Blockkommentaren, normale/raw Strings und Zeichenliterale unter Erhaltung der Zeilennummern. Gezählt sind benannte `fn`-Deklarationen inklusive Traitmethoden, Hilfsfunktionen und Tests; keine Closures oder Makroexpansionen. Bei unsafe sind **108 Blöcke + 12 impls + 5 benannte Funktionen = 125 Stellen** gezählt. Der `unsafe extern "C" fn(...)`-Typalias in `store/writer.rs:858` ist keine sechste Funktionsdefinition.

Weitere vollständige Suchpopulationen: **111 Mutex-Typstellen** (einschließlich wiederholter Signaturtypen, nicht 111 unabhängige Sperren), **350 .lock()-Aufrufe**, **0 RwLock-Treffer**, **22 Kanalkonstruktionen**, **59 Spawn-Aufrufstellen** (Thread- und Testprozessstarts), **8 lokale Allow-Attribute**. Datei und Zeile jeder Stelle folgen unten.

**Lesetiefe:** Die ganze Rust-Population wurde als Text für diese Muster gelesen. Zusätzlich sind für **74 Rust-Pfade** gezielte nummerierte Quell-/Testauszüge im Lesejournal dieses Laufs vorhanden (Marke B). S bezeichnet die übrigen **38 Pfade** mit Populations-/Funktions-/Aufrufsuche; kurze Treffer- und unsafe-Kontexte können auch dort gelesen sein. Nicht jede Funktion dieser Dateien wurde semantisch vollständig abgearbeitet. Die Tabellen erheben deshalb keinen formalen Vollständigkeitsbeweis aller möglichen Fehler.

### Vollständige Funktionspopulation

Die Zeilen gehören jeweils zur links genannten Datei; zusammen ergeben beide Spalten eindeutige Datei:Zeile-Orte. B/S beschreibt nur die oben definierte Lesetiefe. Cargo wurde vollständig gelesen.

| Datei | Zeilen gesamt | fn-Anzahl | Zeilen sämtlicher fn-Deklarationen | Tiefe |
|---|---:|---:|---|---|
| `broker/src/aggregat.rs` | 379 | 14 | 16, 138, 146, 151, 184, 192, 208, 221, 240, 263, 281, 299, 322, 367 | B |
| `broker/src/bin/eqcop-broker-probe.rs` | 35 | 1 | 16 | B |
| `broker/src/bin/eqcop-broker-sonde012-probe.rs` | 73 | 2 | 14, 19 | B |
| `broker/src/bin/eqcop-broker-v3probe.rs` | 144 | 1 | 28 | B |
| `broker/src/bin/eqcop-store-crash-worker.rs` | 298 | 14 | 12, 18, 46, 90, 98, 99, 100, 101, 103, 130, 131, 132, 136, 164 | B |
| `broker/src/bindung.rs` | 156 | 8 | 22, 40, 51, 60, 74, 86, 100, 142 | B |
| `broker/src/briefkasten.rs` | 1995 | 142 | 74, 78, 82, 91, 95, 108, 128, 135, 170, 183, 194, 195, 196, 197, 198, 199, 202, 205, 213, 237, 258, 266, 270, 275, 285, 290, 317, 321, 326, 333, 338, 343, 348, 354, 360, 369, 375, 384, 395, 399, 408, 409, 415, 419, 432, 499, 504, 554, 594, 607, 663, 682, 694, 728, 754, 762, 779, 787, 853, 859, 896, 914, 928, 934, 940, 946, 983, 991, 1000, 1005, 1029, 1039, 1044, 1049, 1054, 1061, 1066, 1071, 1076, 1085, 1096, 1100, 1120, 1128, 1133, 1139, 1145, 1156, 1170, 1174, 1178, 1184, 1201, 1215, 1223, 1228, 1232, 1237, 1241, 1248, 1256, 1273, 1279, 1287, 1295, 1302, 1309, 1313, 1317, 1321, 1326, 1333, 1340, 1349, 1358, 1377, 1398, 1420, 1434, 1455, 1469, 1495, 1500, 1509, 1519, 1568, 1581, 1600, 1611, 1647, 1702, 1717, 1745, 1776, 1791, 1811, 1817, 1846, 1876, 1917, 1962, 1977 | B |
| `broker/src/coordinator/assistent.rs` | 236 | 7 | 83, 155, 188, 194, 203, 215, 230 | B |
| `broker/src/coordinator/befehl.rs` | 1332 | 21 | 41, 66, 85, 111, 129, 146, 157, 474, 626, 661, 665, 987, 996, 1006, 1012, 1017, 1031, 1046, 1100, 1147, 1245 | B |
| `broker/src/coordinator/evidenz.rs` | 528 | 10 | 119, 253, 349, 386, 432, 477, 486, 500, 510, 522 | B |
| `broker/src/coordinator/experiment.rs` | 1727 | 44 | 166, 176, 181, 350, 357, 480, 650, 661, 671, 687, 819, 832, 866, 882, 894, 904, 953, 965, 979, 983, 989, 993, 997, 1006, 1036, 1105, 1144, 1176, 1204, 1265, 1289, 1321, 1344, 1354, 1360, 1435, 1472, 1484, 1487, 1509, 1572, 1629, 1675, 1706 | B |
| `broker/src/coordinator/experiment_verdrahtung.rs` | 1892 | 38 | 39, 206, 372, 385, 403, 435, 521, 556, 577, 622, 639, 659, 668, 682, 694, 703, 712, 719, 743, 825, 860, 1258, 1302, 1332, 1372, 1482, 1588, 1598, 1602, 1628, 1664, 1673, 1681, 1769, 1825, 1838, 1873, 1882 | B |
| `broker/src/coordinator/flush.rs` | 318 | 12 | 18, 24, 31, 50, 58, 223, 235, 247, 281, 289, 300, 311 | B |
| `broker/src/coordinator/hypothese/befund.rs` | 484 | 8 | 45, 132, 164, 173, 353, 375, 385, 429 | B |
| `broker/src/coordinator/hypothese/determinismus.rs` | 120 | 2 | 88, 103 | S |
| `broker/src/coordinator/hypothese/eingang.rs` | 179 | 2 | 48, 170 | S |
| `broker/src/coordinator/hypothese/enthaltung.rs` | 203 | 3 | 46, 75, 158 | S |
| `broker/src/coordinator/hypothese/klassenwahl.rs` | 240 | 7 | 47, 55, 70, 75, 112, 148, 175 | S |
| `broker/src/coordinator/hypothese/messung.rs` | 556 | 20 | 18, 34, 48, 64, 81, 108, 128, 149, 180, 198, 218, 226, 240, 283, 332, 386, 450, 478, 502, 540 | S |
| `broker/src/coordinator/hypothese/mod.rs` | 72 | 0 | — | S |
| `broker/src/coordinator/hypothese/rechnung.rs` | 523 | 4 | 56, 334, 424, 504 | B |
| `broker/src/coordinator/hypothese/screening.rs` | 813 | 15 | 60, 81, 127, 138, 161, 196, 204, 209, 225, 351, 399, 449, 493, 512, 711 | B |
| `broker/src/coordinator/hypothese/testhilfe.rs` | 132 | 7 | 14, 33, 44, 54, 80, 104, 123 | S |
| `broker/src/coordinator/hypothese/vertrag.rs` | 354 | 14 | 56, 61, 76, 92, 117, 125, 144, 176, 187, 211, 261, 265, 288, 321 | S |
| `broker/src/coordinator/hypothese/zusammenhang.rs` | 630 | 23 | 47, 64, 109, 114, 121, 132, 146, 163, 190, 227, 238, 276, 307, 329, 359, 376, 407, 419, 439, 519, 540, 574, 610 | S |
| `broker/src/coordinator/hypothese_verdrahtung.rs` | 1330 | 21 | 107, 120, 169, 195, 484, 526, 634, 700, 720, 737, 754, 906, 984, 1040, 1183, 1227, 1248, 1274, 1286, 1312, 1322 | B |
| `broker/src/coordinator/intent.rs` | 921 | 19 | 136, 155, 175, 190, 212, 229, 293, 299, 317, 668, 736, 742, 751, 762, 780, 794, 810, 833, 866 | B |
| `broker/src/coordinator/intervention.rs` | 674 | 28 | 28, 32, 40, 48, 54, 88, 104, 123, 165, 189, 273, 299, 310, 316, 331, 367, 401, 442, 471, 495, 520, 558, 570, 599, 610, 632, 656, 669 | B |
| `broker/src/coordinator/invalidierung.rs` | 285 | 10 | 43, 74, 87, 96, 119, 146, 164, 203, 229, 272 | B |
| `broker/src/coordinator/invalidierung_verdrahtung.rs` | 781 | 24 | 62, 90, 138, 250, 293, 312, 350, 405, 444, 456, 474, 510, 563, 590, 617, 666, 686, 701, 722, 735, 748, 761, 767, 775 | B |
| `broker/src/coordinator/link.rs` | 648 | 15 | 11, 29, 34, 52, 61, 80, 364, 384, 397, 431, 480, 499, 619, 628, 635 | B |
| `broker/src/coordinator/liveness.rs` | 911 | 13 | 15, 58, 77, 99, 211, 225, 317, 545, 549, 653, 714, 782, 804 | B |
| `broker/src/coordinator/maskierung.rs` | 334 | 11 | 92, 133, 173, 195, 213, 227, 247, 262, 276, 309, 322 | S |
| `broker/src/coordinator/mitgliedschaft.rs` | 269 | 9 | 9, 57, 95, 119, 139, 163, 180, 213, 231 | B |
| `broker/src/coordinator/mod.rs` | 737 | 37 | 147, 159, 180, 188, 196, 200, 277, 287, 309, 366, 374, 381, 393, 405, 419, 431, 444, 451, 457, 464, 478, 496, 506, 513, 557, 574, 592, 608, 616, 626, 636, 649, 660, 672, 681, 692, 705 | B |
| `broker/src/coordinator/prepost.rs` | 1248 | 20 | 386, 419, 439, 467, 528, 551, 582, 637, 697, 715, 743, 752, 778, 850, 879, 945, 981, 1016, 1052, 1224 | B |
| `broker/src/coordinator/prepost_verdrahtung.rs` | 382 | 7 | 64, 74, 186, 317, 332, 356, 375 | B |
| `broker/src/coordinator/proposal.rs` | 960 | 29 | 75, 86, 104, 125, 141, 145, 164, 195, 199, 268, 360, 382, 409, 466, 509, 525, 542, 585, 608, 653, 775, 827, 836, 844, 862, 880, 898, 920, 942 | B |
| `broker/src/coordinator/proposal_verdrahtung.rs` | 741 | 23 | 37, 99, 139, 176, 210, 231, 254, 301, 376, 390, 411, 424, 429, 442, 474, 503, 510, 538, 542, 552, 596, 622, 692 | B |
| `broker/src/coordinator/schema.rs` | 856 | 25 | 188, 205, 227, 261, 318, 325, 377, 413, 422, 428, 432, 458, 465, 481, 522, 562, 608, 636, 651, 707, 734, 776, 804, 836, 852 | B |
| `broker/src/coordinator/senke.rs` | 264 | 11 | 10, 23, 41, 58, 72, 80, 84, 88, 92, 125, 263 | B |
| `broker/src/coordinator/sicht.rs` | 662 | 21 | 41, 49, 111, 118, 125, 132, 218, 233, 237, 246, 256, 264, 274, 282, 289, 296, 304, 345, 375, 467, 519 | B |
| `broker/src/coordinator/subscription.rs` | 495 | 13 | 9, 14, 18, 74, 251, 310, 342, 375, 418, 447, 468, 485, 490 | B |
| `broker/src/coordinator/uhr.rs` | 54 | 6 | 10, 19, 27, 41, 45, 51 | B |
| `broker/src/coordinator/vergleichbarkeit.rs` | 403 | 7 | 54, 83, 204, 232, 262, 282, 316 | S |
| `broker/src/coordinator/zustand.rs` | 685 | 12 | 18, 27, 48, 223, 262, 481, 529, 536, 543, 557, 583, 645 | B |
| `broker/src/dto.rs` | 591 | 24 | 84, 130, 137, 218, 222, 227, 254, 258, 261, 264, 267, 270, 273, 276, 280, 288, 302, 312, 439, 510, 529, 555, 566, 584 | B |
| `broker/src/framing.rs` | 118 | 7 | 23, 34, 70, 80, 91, 104, 113 | B |
| `broker/src/instance_alias.rs` | 539 | 29 | 15, 26, 48, 55, 87, 124, 128, 132, 144, 224, 245, 265, 274, 285, 302, 336, 359, 364, 372, 400, 407, 416, 430, 440, 448, 468, 486, 503, 520 | B |
| `broker/src/lebenslauf.rs` | 418 | 13 | 75, 122, 130, 232, 253, 266, 280, 284, 299, 332, 353, 377, 404 | B |
| `broker/src/lib.rs` | 63 | 0 | — | B |
| `broker/src/main.rs` | 81 | 2 | 15, 24 | B |
| `broker/src/paar.rs` | 433 | 10 | 41, 64, 78, 276, 298, 323, 354, 375, 400, 413 | B |
| `broker/src/persistenz.rs` | 186 | 11 | 20, 25, 31, 60, 72, 78, 84, 112, 131, 137, 158 | B |
| `broker/src/probe.rs` | 117 | 3 | 29, 66, 87 | B |
| `broker/src/protokoll.rs` | 718 | 23 | 100, 109, 164, 242, 318, 326, 334, 342, 362, 380, 398, 431, 463, 468, 488, 508, 574, 590, 629, 642, 666, 677, 692 | B |
| `broker/src/register.rs` | 996 | 28 | 118, 142, 146, 226, 236, 331, 340, 353, 360, 368, 379, 383, 420, 434, 438, 448, 468, 482, 520, 539, 556, 603, 693, 739, 831, 895, 930, 958 | B |
| `broker/src/server.rs` | 1841 | 58 | 76, 87, 98, 106, 121, 218, 222, 248, 254, 261, 274, 283, 314, 339, 353, 383, 384, 395, 415, 429, 446, 464, 663, 913, 923, 977, 980, 984, 1021, 1049, 1057, 1072, 1080, 1091, 1099, 1151, 1170, 1192, 1222, 1238, 1246, 1262, 1271, 1331, 1406, 1478, 1507, 1550, 1574, 1596, 1638, 1663, 1687, 1725, 1758, 1779, 1793, 1823 | B |
| `broker/src/sitzung.rs` | 71 | 2 | 25, 58 | B |
| `broker/src/store/handle.rs` | 509 | 29 | 18, 24, 32, 76, 86, 101, 130, 134, 138, 144, 148, 156, 173, 182, 196, 210, 219, 250, 256, 277, 301, 336, 380, 408, 434, 446, 483, 495, 504 | B |
| `broker/src/store/migration.rs` | 384 | 8 | 152, 158, 228, 242, 252, 304, 323, 339 | B |
| `broker/src/store/mod.rs` | 414 | 16 | 74, 88, 125, 156, 164, 190, 218, 234, 238, 243, 247, 256, 295, 316, 345, 405 | B |
| `broker/src/store/pfad.rs` | 368 | 14 | 9, 15, 32, 37, 42, 82, 99, 119, 166, 180, 263, 299, 321, 360 | B |
| `broker/src/store/writer.rs` | 1020 | 23 | 16, 36, 111, 115, 119, 123, 135, 176, 406, 436, 500, 641, 724, 752, 790, 825, 866, 872, 879, 893, 940, 960, 971 | B |
| `broker/src/telemetrie.rs` | 1096 | 19 | 75, 89, 90, 125, 159, 230, 247, 330, 401, 408, 453, 477, 514, 548, 553, 572, 762, 928, 1037 | B |
| `broker/src/testhilfe.rs` | 61 | 4 | 13, 33, 44, 52 | S |
| `broker/src/transport/bootstrap.rs` | 704 | 31 | 145, 149, 175, 190, 204, 327, 331, 335, 339, 346, 370, 394, 406, 415, 426, 432, 441, 448, 456, 467, 478, 492, 568, 579, 592, 602, 617, 628, 659, 688, 696 | B |
| `broker/src/transport/legacy_v2.rs` | 93 | 4 | 28, 44, 72, 84 | B |
| `broker/src/transport/mod.rs` | 24 | 0 | — | S |
| `broker/src/transport/pipetoken.rs` | 199 | 12 | 38, 55, 66, 87, 96, 106, 120, 141, 166, 173, 182, 190 | S |
| `broker/src/transport/server_v3/auth.rs` | 189 | 4 | 18, 22, 40, 45 | S |
| `broker/src/transport/server_v3/griff.rs` | 495 | 26 | 53, 65, 81, 108, 124, 134, 213, 231, 238, 248, 264, 269, 273, 277, 285, 294, 306, 311, 319, 371, 382, 398, 447, 456, 464, 477 | B |
| `broker/src/transport/server_v3/listener.rs` | 703 | 10 | 63, 67, 79, 83, 130, 197, 205, 220, 242, 265 | B |
| `broker/src/transport/server_v3/mod.rs` | 231 | 1 | 208 | B |
| `broker/src/transport/server_v3/queues.rs` | 831 | 31 | 16, 23, 46, 51, 55, 77, 85, 160, 174, 185, 201, 210, 305, 328, 346, 359, 374, 391, 397, 403, 411, 439, 508, 536, 572, 614, 630, 644, 663, 701, 768 | B |
| `broker/src/transport/server_v3/senke.rs` | 140 | 22 | 16, 24, 45, 52, 53, 54, 55, 57, 58, 59, 63, 66, 69, 74, 94, 98, 101, 104, 108, 128, 131, 134 | B |
| `broker/src/transport/server_v3/tests_abonnement.rs` | 1567 | 53 | 27, 79, 153, 187, 335, 453, 461, 471, 477, 498, 549, 560, 572, 577, 587, 603, 620, 647, 673, 677, 685, 697, 704, 710, 719, 726, 732, 740, 750, 771, 781, 787, 793, 807, 824, 832, 861, 873, 946, 1039, 1108, 1188, 1270, 1350, 1357, 1363, 1378, 1384, 1404, 1443, 1486, 1509, 1532 | B |
| `broker/src/transport/server_v3/tests_fristen.rs` | 179 | 6 | 19, 52, 82, 102, 146, 165 | S |
| `broker/src/transport/server_v3/tests_hilfe.rs` | 454 | 38 | 23, 42, 46, 76, 114, 131, 135, 139, 160, 166, 172, 180, 189, 199, 208, 220, 229, 233, 261, 270, 286, 305, 309, 313, 324, 346, 356, 368, 384, 416, 427, 430, 433, 436, 439, 443, 447, 451 | S |
| `broker/src/transport/server_v3/tests_kopplung.rs` | 498 | 20 | 36, 54, 57, 63, 66, 77, 82, 86, 90, 94, 97, 100, 103, 112, 154, 192, 228, 284, 361, 448 | S |
| `broker/src/transport/server_v3/tests_lebenszyklus.rs` | 422 | 16 | 22, 66, 127, 158, 216, 274, 285, 289, 292, 295, 298, 301, 304, 307, 318, 369 | S |
| `broker/src/transport/server_v3/tests_rueckstau.rs` | 240 | 12 | 24, 63, 118, 154, 157, 160, 163, 166, 169, 178, 181, 191 | S |
| `broker/src/transport/server_v3/trennung.rs` | 192 | 7 | 50, 60, 77, 89, 102, 149, 185 | B |
| `broker/src/transport/server_v3/verbindung.rs` | 890 | 9 | 32, 42, 49, 83, 95, 111, 136, 140, 148 | B |
| `broker/src/transport/server_v3/win_handles.rs` | 718 | 29 | 19, 29, 35, 58, 106, 132, 136, 152, 161, 206, 217, 223, 228, 234, 240, 258, 270, 319, 409, 474, 527, 534, 543, 601, 620, 624, 632, 649, 676 | B |
| `broker/src/transport/v3.rs` | 586 | 26 | 70, 83, 92, 144, 165, 171, 200, 216, 224, 326, 375, 379, 383, 388, 417, 426, 446, 451, 460, 471, 485, 499, 518, 543, 564, 574 | B |
| `broker/src/transport/warteschlange.rs` | 712 | 51 | 60, 64, 72, 90, 94, 98, 102, 106, 110, 118, 174, 178, 188, 192, 196, 203, 216, 233, 268, 278, 302, 306, 311, 315, 319, 323, 334, 368, 374, 387, 394, 398, 402, 406, 413, 417, 428, 477, 489, 499, 514, 529, 546, 570, 590, 605, 622, 634, 653, 670, 690 | B |
| `broker/src/vertrag.rs` | 1250 | 54 | 38, 62, 74, 84, 91, 109, 179, 214, 221, 240, 322, 340, 347, 351, 377, 381, 560, 571, 578, 591, 624, 697, 720, 740, 754, 778, 795, 816, 820, 826, 1038, 1059, 1071, 1081, 1087, 1093, 1099, 1110, 1116, 1122, 1128, 1136, 1141, 1161, 1169, 1175, 1180, 1203, 1209, 1215, 1221, 1227, 1234, 1240 | B |
| `broker/src/zeit.rs` | 41 | 3 | 12, 21, 35 | B |
| `broker/tests/briefkasten.rs` | 125 | 6 | 17, 27, 41, 46, 55, 66 | S |
| `broker/tests/broker_idle.rs` | 235 | 7 | 18, 42, 48, 64, 76, 99, 188 | S |
| `broker/tests/contract_cross_language.rs` | 811 | 19 | 16, 20, 26, 31, 45, 144, 177, 239, 296, 316, 349, 384, 394, 408, 429, 525, 575, 675, 803 | S |
| `broker/tests/coordinator_model.rs` | 1873 | 76 | 18, 22, 32, 60, 75, 90, 103, 109, 132, 203, 209, 214, 233, 237, 241, 247, 253, 264, 276, 317, 367, 388, 440, 460, 477, 493, 508, 513, 532, 538, 557, 571, 626, 634, 662, 676, 719, 766, 806, 887, 902, 916, 928, 973, 974, 1009, 1058, 1098, 1103, 1108, 1113, 1130, 1138, 1211, 1215, 1247, 1258, 1266, 1319, 1361, 1421, 1449, 1474, 1504, 1544, 1596, 1597, 1650, 1668, 1713, 1762, 1789, 1800, 1820, 1834, 1860 | S |
| `broker/tests/security_vectors.rs` | 1259 | 46 | 31, 55, 60, 68, 79, 93, 99, 108, 118, 122, 131, 145, 154, 161, 167, 178, 186, 207, 228, 259, 272, 301, 329, 360, 370, 389, 418, 447, 455, 473, 490, 499, 525, 582, 603, 665, 709, 726, 738, 745, 751, 784, 836, 1018, 1023, 1095 | S |
| `broker/tests/sonde012_sources_slice.rs` | 1197 | 44 | 14, 18, 28, 49, 64, 80, 105, 116, 165, 174, 179, 185, 197, 217, 305, 309, 315, 327, 336, 345, 387, 412, 439, 481, 496, 519, 555, 577, 604, 635, 657, 702, 727, 773, 833, 852, 933, 956, 1004, 1029, 1081, 1097, 1149, 1174 | S |
| `broker/tests/sonde013_experiment.rs` | 1074 | 33 | 27, 62, 66, 85, 99, 112, 127, 138, 188, 226, 256, 287, 307, 337, 390, 437, 532, 579, 601, 638, 677, 717, 761, 789, 820, 872, 913, 918, 926, 947, 980, 1015, 1049 | S |
| `broker/tests/sonde013_passage.rs` | 511 | 14 | 32, 46, 60, 136, 239, 258, 276, 302, 327, 341, 357, 393, 429, 470 | S |
| `broker/tests/sonde013_prepost.rs` | 1335 | 29 | 33, 48, 59, 89, 102, 110, 147, 178, 242, 288, 404, 432, 479, 546, 586, 666, 709, 725, 754, 851, 896, 952, 993, 1032, 1051, 1100, 1138, 1180, 1223 | B |
| `broker/tests/sonde013_taint.rs` | 662 | 22 | 25, 29, 48, 57, 81, 106, 150, 186, 217, 249, 291, 305, 318, 344, 374, 401, 411, 432, 444, 483, 520, 576 | B |
| `broker/tests/sonde013_verdrahtung.rs` | 6575 | 141 | 24, 28, 38, 66, 86, 92, 108, 119, 133, 151, 160, 175, 189, 215, 225, 231, 249, 256, 263, 294, 298, 327, 340, 352, 366, 376, 380, 384, 388, 398, 420, 454, 479, 516, 541, 590, 645, 683, 740, 755, 783, 932, 977, 993, 1033, 1072, 1120, 1176, 1213, 1240, 1262, 1311, 1337, 1363, 1402, 1458, 1506, 1567, 1634, 1685, 1774, 1826, 1864, 1913, 1939, 1963, 1983, 2061, 2176, 2224, 2274, 2315, 2466, 2541, 2610, 2671, 2776, 2790, 2910, 2980, 3067, 3137, 3280, 3359, 3366, 3372, 3386, 3404, 3421, 3431, 3445, 3457, 3481, 3579, 3608, 3636, 3708, 3753, 3811, 3875, 3958, 4049, 4123, 4241, 4326, 4383, 4430, 4488, 4607, 4618, 4631, 4647, 4688, 4714, 4778, 4825, 4866, 4910, 5122, 5164, 5199, 5246, 5264, 5277, 5330, 5374, 5438, 5513, 5573, 5609, 5755, 5798, 5836, 5917, 5935, 5969, 6151, 6155, 6306, 6387, 6501 | B |
| `broker/tests/sonde014_befund.rs` | 495 | 21 | 28, 32, 42, 63, 67, 76, 98, 112, 124, 146, 152, 168, 189, 216, 237, 286, 318, 367, 404, 433, 469 | S |
| `broker/tests/sonde014_gegenbeispiele.rs` | 2125 | 50 | 68, 72, 82, 103, 112, 119, 125, 132, 148, 188, 196, 204, 210, 227, 240, 255, 278, 307, 319, 334, 352, 364, 403, 409, 442, 464, 516, 533, 599, 769, 809, 866, 914, 981, 1032, 1083, 1154, 1202, 1238, 1250, 1457, 1538, 1578, 1639, 1702, 1827, 1923, 1953, 1991, 2067 | B |
| `broker/tests/sonde014_hypothese.rs` | 2622 | 61 | 37, 41, 51, 72, 76, 89, 121, 145, 182, 213, 252, 269, 308, 313, 327, 341, 369, 420, 439, 454, 501, 534, 565, 598, 648, 718, 772, 871, 921, 974, 1006, 1121, 1146, 1179, 1233, 1281, 1331, 1374, 1490, 1538, 1568, 1648, 1708, 1799, 1826, 1841, 1849, 1853, 1867, 1937, 2019, 2083, 2104, 2163, 2300, 2359, 2414, 2491, 2525, 2550, 2611 | S |
| `broker/tests/sonde014_maskierung.rs` | 389 | 16 | 22, 26, 36, 57, 61, 70, 91, 109, 131, 146, 159, 181, 220, 274, 319, 357 | S |
| `broker/tests/sonde014_nacharbeit1.rs` | 805 | 30 | 37, 41, 51, 72, 81, 102, 132, 142, 148, 163, 177, 194, 201, 240, 259, 269, 275, 289, 294, 336, 390, 442, 493, 561, 574, 625, 665, 723, 729, 758 | S |
| `broker/tests/sonde014_nacharbeit2.rs` | 766 | 28 | 33, 37, 47, 73, 88, 109, 137, 147, 153, 165, 172, 181, 208, 233, 280, 295, 308, 322, 349, 359, 364, 370, 393, 398, 418, 510, 641, 714 | S |
| `broker/tests/sonde014_nacharbeit3.rs` | 699 | 31 | 42, 46, 56, 77, 92, 109, 146, 156, 162, 173, 180, 189, 215, 240, 278, 292, 304, 317, 326, 331, 354, 361, 381, 429, 441, 457, 466, 481, 521, 603, 684 | S |
| `broker/tests/sonde014_p5_korpus.rs` | 474 | 16 | 30, 34, 44, 65, 74, 95, 103, 109, 124, 164, 174, 180, 187, 195, 204, 349 | S |
| `broker/tests/sonde014_proposal.rs` | 1495 | 43 | 28, 32, 42, 74, 86, 103, 108, 123, 131, 140, 173, 191, 218, 230, 259, 287, 302, 354, 419, 431, 459, 487, 534, 593, 625, 697, 727, 739, 766, 827, 904, 940, 991, 1045, 1070, 1129, 1131, 1215, 1216, 1296, 1320, 1420, 1470 | S |
| `broker/tests/sonde014_verdrahtung.rs` | 4889 | 120 | 30, 34, 44, 65, 69, 76, 84, 90, 95, 108, 119, 135, 168, 178, 209, 219, 239, 321, 344, 398, 451, 534, 673, 679, 685, 718, 741, 796, 822, 837, 868, 887, 977, 1100, 1204, 1365, 1406, 1463, 1530, 1551, 1593, 1615, 1625, 1631, 1649, 1674, 1746, 1787, 1858, 1928, 1988, 2008, 2020, 2061, 2092, 2125, 2142, 2185, 2226, 2272, 2283, 2324, 2350, 2394, 2433, 2492, 2597, 2612, 2621, 2641, 2653, 2692, 2700, 2707, 2716, 2774, 3009, 3113, 3133, 3203, 3234, 3248, 3290, 3327, 3351, 3446, 3485, 3526, 3585, 3616, 3672, 3724, 3776, 3800, 3822, 3840, 3868, 3897, 3924, 3943, 3985, 4020, 4031, 4042, 4054, 4068, 4093, 4204, 4288, 4356, 4460, 4509, 4545, 4553, 4571, 4636, 4664, 4706, 4746, 4850 | S |
| `broker/tests/store_crash_matrix.rs` | 3981 | 134 | 28, 38, 44, 49, 58, 71, 83, 89, 106, 118, 133, 141, 148, 164, 174, 185, 193, 199, 207, 216, 227, 239, 258, 297, 301, 311, 332, 347, 362, 383, 410, 416, 423, 427, 433, 441, 449, 481, 520, 521, 603, 735, 916, 1043, 1076, 1157, 1175, 1238, 1324, 1361, 1385, 1399, 1411, 1432, 1439, 1468, 1489, 1494, 1515, 1546, 1559, 1579, 1616, 1632, 1645, 1671, 1689, 1725, 1735, 1759, 1771, 1820, 1883, 1902, 1907, 1925, 1945, 1989, 2015, 2031, 2044, 2068, 2078, 2099, 2126, 2166, 2179, 2192, 2197, 2202, 2214, 2221, 2228, 2235, 2240, 2268, 2298, 2325, 2352, 2399, 2478, 2570, 2580, 2590, 2619, 2639, 2668, 2689, 2717, 2723, 2729, 2767, 2810, 2853, 2904, 2944, 2968, 2997, 3046, 3148, 3220, 3274, 3346, 3437, 3467, 3514, 3531, 3572, 3590, 3672, 3721, 3806, 3891, 3930 | B |
| `broker/tests/transport_fuzz.rs` | 551 | 18 | 21, 24, 33, 40, 45, 67, 85, 95, 122, 140, 188, 237, 256, 282, 293, 315, 375, 462 | S |
| `broker/Cargo.toml` | 82 | 0 | — | vollständig |

### Vollständige unsafe-Population

Die Zahlen sind Zeilen im jeweiligen Pfad. Auth-/Token-/Security-Descriptor-Stellen wurden ausschließlich auf Besitz, Lebensdauer und Freigabe bezogen; ihre Sicherheitswirkung ist Phase 12 vorbehalten.

| Datei | unsafe-Blöcke | unsafe-impls | unsafe-Funktionsdefinitionen |
|---|---|---|---|
| `broker/src/briefkasten.rs` | 221, 281, 988, 993 | 982 | 983, 991 |
| `broker/src/persistenz.rs` | 104 | — | — |
| `broker/src/server.rs` | 81, 109, 125, 127, 136, 140, 151, 161, 170, 177, 182, 190, 192, 198, 200, 206, 231, 289, 296, 502, 516, 562, 580, 590, 594, 613, 917, 941 | 70, 73, 115 | — |
| `broker/src/store/pfad.rs` | 52, 67, 108, 154, 162, 194, 232, 242, 277, 335, 355 | — | — |
| `broker/src/store/writer.rs` | 876, 883, 888, 906, 911, 914, 920, 923, 926, 944 | — | 872, 879, 893 |
| `broker/src/transport/server_v3/auth.rs` | 30, 54, 63, 72, 76, 88, 103, 107, 120, 127, 140, 147, 149, 151, 156, 165, 177, 179, 184 | — | — |
| `broker/src/transport/server_v3/listener.rs` | 460 | — | — |
| `broker/src/transport/server_v3/tests_hilfe.rs` | 53, 81, 118, 142, 162 | 39 | — |
| `broker/src/transport/server_v3/win_handles.rs` | 22, 37, 68, 84, 89, 93, 109, 145, 173, 182, 209, 220, 225, 236, 243, 275, 323, 417, 448, 456, 510, 513, 604, 609, 654 | 16, 55, 198, 203, 384, 386, 585 | — |
| `broker/tests/broker_idle.rs` | 21, 43 | — | — |
| `broker/tests/security_vectors.rs` | 34, 56 | — | — |

### Vollständige Sperrenpopulation

`Mutex<…>` ist eine Typfundstelle, `.lock()` eine Aufrufstelle. Gleiche Zeilennummern bleiben wiederholt stehen, falls die Zeile mehrere syntaktische Treffer enthält. Diese Aufstellung ersetzt keine Behauptung, es gebe eine global dokumentierte Sperrenordnung.

| Datei | Mutex-Typstellen | lock-Aufrufe |
|---|---|---|
| `broker/src/bin/eqcop-broker-sonde012-probe.rs` | — | 61 |
| `broker/src/bin/eqcop-broker-v3probe.rs` | — | 77, 129 |
| `broker/src/briefkasten.rs` | 78, 531, 542, 543, 549, 779, 787, 909, 1025, 1196, 1340, 1349 | 79 |
| `broker/src/coordinator/assistent.rs` | — | 122, 181, 220, 232 |
| `broker/src/coordinator/befehl.rs` | — | 173, 241, 268, 487, 677, 1230, 1255, 1325 |
| `broker/src/coordinator/evidenz.rs` | — | 141, 212, 468, 501, 511, 524 |
| `broker/src/coordinator/experiment_verdrahtung.rs` | — | 85, 245, 314, 357, 640, 865, 1830, 1840, 1887 |
| `broker/src/coordinator/flush.rs` | — | 26, 32, 62, 81, 140, 161, 202, 209, 256, 282, 301, 312 |
| `broker/src/coordinator/hypothese_verdrahtung.rs` | — | 172, 196, 533, 659, 663, 690, 703, 739, 989, 1014, 1279, 1288 |
| `broker/src/coordinator/intent.rs` | — | 472, 654, 673, 730, 763, 781, 796 |
| `broker/src/coordinator/intervention.rs` | — | 133, 200, 274, 300, 311, 317, 332, 368, 402, 500, 521, 560, 571, 600, 611, 657, 670 |
| `broker/src/coordinator/invalidierung_verdrahtung.rs` | — | 146, 251, 416, 449, 457, 521, 619, 762, 769, 777 |
| `broker/src/coordinator/link.rs` | — | 90, 368, 385, 432, 484, 501, 621 |
| `broker/src/coordinator/liveness.rs` | — | 247, 331, 607, 783, 817 |
| `broker/src/coordinator/mitgliedschaft.rs` | — | 106, 150, 191, 214 |
| `broker/src/coordinator/mod.rs` | 170, 212, 218, 222, 223, 244, 253, 264 | 182, 190, 202, 375, 384, 396, 412, 422, 434, 516 |
| `broker/src/coordinator/prepost_verdrahtung.rs` | — | 66, 76, 146, 318, 333, 357, 377 |
| `broker/src/coordinator/proposal_verdrahtung.rs` | — | 39, 50, 161, 177, 232, 261, 292, 416, 431, 444, 626, 662 |
| `broker/src/coordinator/senke.rs` | — | 43, 60, 74, 138, 142 |
| `broker/src/coordinator/sicht.rs` | — | 113, 120, 127, 139, 238, 248, 258, 266, 276, 284, 291, 298, 314, 362 |
| `broker/src/coordinator/subscription.rs` | — | 24, 29, 34, 86, 221, 271, 312, 376, 382, 419, 448, 486 |
| `broker/src/instance_alias.rs` | 120 | 153, 225, 251, 287, 303, 338, 360, 366, 374 |
| `broker/src/lebenslauf.rs` | 85, 87, 89, 98, 103, 112, 116, 253, 267, 268, 269, 270, 271 | 254, 309, 318, 325, 343, 365, 380 |
| `broker/src/probe.rs` | — | 41, 73, 98 |
| `broker/src/server.rs` | 248, 254, 262, 274, 283, 315, 316, 345, 390, 391, 431, 448, 466, 665, 923, 1080 | 249, 255, 267, 319, 354, 405, 636, 717, 841, 1007, 1082, 1132, 1143, 1302, 1446, 1462, 1470, 1497, 1531, 1540, 1568, 1586, 1624, 1655, 1658, 1679, 1716 |
| `broker/src/store/handle.rs` | 14, 115, 495 | 26, 34, 131, 234, 251, 496 |
| `broker/src/store/writer.rs` | 179 | 45, 228, 335 |
| `broker/src/transport/server_v3/griff.rs` | 22, 23, 24, 26, 44, 45, 60, 61, 399 | 94, 115, 240, 250, 265, 334, 351, 353, 403 |
| `broker/src/transport/server_v3/listener.rs` | 58, 134, 283, 286, 287, 288 | 68, 85, 345, 379, 392, 575, 651, 670 |
| `broker/src/transport/server_v3/queues.rs` | 11, 108, 132 | 25, 56, 79, 87, 213, 306, 330, 349, 361, 377, 725, 784 |
| `broker/src/transport/server_v3/senke.rs` | 90 | 136 |
| `broker/src/transport/server_v3/tests_abonnement.rs` | 765, 766, 767, 1346 | 523, 674, 688, 788, 794, 810, 826, 842, 849, 878, 1358, 1371 |
| `broker/src/transport/server_v3/tests_kopplung.rs` | 25, 30, 31 | 38, 42, 50, 55, 68, 115, 231, 287, 364, 451 |
| `broker/src/transport/server_v3/trennung.rs` | 41, 150, 151 | 51, 62, 67, 79, 84, 91, 96, 109, 118, 164, 171 |
| `broker/src/transport/server_v3/verbindung.rs` | 132, 152, 154, 155, 162, 163 | 141, 195, 305, 388, 410, 434, 454, 461, 476, 482, 531, 699, 817, 824, 859, 886 |
| `broker/src/transport/server_v3/win_handles.rs` | 372, 523, 624, 633, 690 | 490, 538, 548, 625, 637, 692 |
| `broker/tests/briefkasten.rs` | 15 | 56, 67 |
| `broker/tests/coordinator_model.rs` | 1204, 1646 | 1212, 1219, 1659, 1730, 1740 |
| `broker/tests/security_vectors.rs` | 701, 702 | 712, 716, 728, 740 |
| `broker/tests/sonde012_sources_slice.rs` | 300, 301 | 306, 310, 323, 329 |
| `broker/tests/sonde013_verdrahtung.rs` | 244 | 251, 275 |
| `broker/tests/sonde014_nacharbeit2.rs` | 161 | 167, 174, 190 |
| `broker/tests/sonde014_nacharbeit3.rs` | 169 | 175, 182, 198 |
| `broker/tests/sonde014_proposal.rs` | 1041 | 1054, 1103 |
| `broker/tests/sonde014_verdrahtung.rs` | 2686, 2687 | 760, 769, 915, 921, 1217, 1221, 2702, 2709, 2728, 2733 |
| `broker/tests/store_crash_matrix.rs` | 397, 402, 405, 406 | 418, 424, 429, 430, 435, 443, 457, 463, 467, 474 |

### Vollständige Kanal- und Spawn-Population

Die Quelle unterscheidet Arbeitskanäle, einmalige Antworten und Testbarrieren; ihre Anzahl ist kein Rückstaubudget. `.spawn()` an einem Test-Command ist ein Prozessstart, kein Thread.

| Datei | channel/sync_channel | spawn |
|---|---|---|
| `broker/src/aggregat.rs` | — | 331 |
| `broker/src/bin/eqcop-broker-sonde012-probe.rs` | 57 | 58 |
| `broker/src/bin/eqcop-broker-v3probe.rs` | 73 | 74 |
| `broker/src/bindung.rs` | — | 110 |
| `broker/src/briefkasten.rs` | — | 836 |
| `broker/src/lebenslauf.rs` | — | 180 |
| `broker/src/server.rs` | 1766 | 532, 623, 941, 1767 |
| `broker/src/store/handle.rs` | 165, 174, 187, 201, 211 | — |
| `broker/src/store/writer.rs` | 53 | 62 |
| `broker/src/transport/server_v3/listener.rs` | — | 371, 440, 614 |
| `broker/src/transport/server_v3/queues.rs` | 191, 780, 781 | 790, 798 |
| `broker/src/transport/server_v3/tests_abonnement.rs` | — | 557, 574, 897, 1044, 1560 |
| `broker/src/transport/server_v3/tests_fristen.rs` | 42 | 43 |
| `broker/src/transport/server_v3/tests_lebenszyklus.rs` | 189, 232, 333, 397 | 190, 233, 334, 398 |
| `broker/src/transport/server_v3/verbindung.rs` | 60 | 63, 497, 593, 622 |
| `broker/src/transport/server_v3/win_handles.rs` | — | 438 |
| `broker/tests/broker_idle.rs` | — | 111 |
| `broker/tests/coordinator_model.rs` | 1228 | 1229, 1390 |
| `broker/tests/security_vectors.rs` | — | 197, 763, 795 |
| `broker/tests/sonde013_taint.rs` | — | 611, 630 |
| `broker/tests/sonde013_verdrahtung.rs` | — | 3530, 3538 |
| `broker/tests/sonde014_verdrahtung.rs` | 1004, 1019 | 1000, 1006, 1021, 1297, 1301 |
| `broker/tests/store_crash_matrix.rs` | 1201 | 170, 181, 234, 278, 1132, 1198, 1202, 1348, 1653, 1781, 2085, 2883, 2920 |

### Gelesene Kontext- und Referenzpfade

**Kanonischer Einstieg und Methodik:** `AGENTS.md` und `CLAUDE.md` vollständig; relevante Brokerabschnitte aus `docs/plugin-wissen.md`, `docs/gesundheit/KONZEPT.md` §3/§4.3, `docs/gesundheit/abdeckungskarte.md` K1/K2 und Nachbarzusagen, `docs/gesundheit/lebenslauf-audit.md` B-1 bis B-4; thematische Registersuche in `docs/offene-punkte.md`. Das sind Kontextquellen, keine Befundflächen.

**Anforderungen/Vorlauf, gezielt:** `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/plan/plan.json`, `docs/beweise/SONDE-011.md`, `docs/beweise/SONDE-013.md`, `docs/beweise/SONDE-014.md`, `docs/beweise/NAK-246.md`, `docs/beweise/NAK-283.md`, `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md`, vorhandene Abschnitte von `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` und gezielte Treffer in den Rohberichten der Phasen 03/06. Alte Manifestaussagen sind nicht als neue Messergebnisse übernommen.

**Vertragsbezug, gezielt:** `eq-copilot/schemas/v3/README.md`, `eq-copilot/schemas/v3/flatbuffers/README.md`, `eq-copilot/schemas/v3/eq-ipc-v3.schema.json` und die im Broker eingebundenen Vertragsdefinitionen; `broker/src/generiert/` nur als Generator-/Accessor-Bezug.

**C++-Referenzen der neuen Befunde:** `eq-copilot/plugin/src/SourcesModel.cpp:1494`; `eq-copilot/plugin/sonde/SondeProcessor.cpp:967/:1100`; `eq-copilot/plugin/core/ipc/controlclient/Intern.h:412`; `eq-copilot/plugin/src/prozessor/Analyse.cpp:1333`; `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:222/:516`; `eq-copilot/plugin/core/analysis/featureengine/Frame.h:221`. Diese Auszüge sind Referenzen und kein zweiter Plugin-Audit.

**Werkzeug-/Startbezug:** `tools/plan/gesundheit.py:255/:676` nur gelesen; `tools/pruefung/codex-audit-lauf.ps1` zur Startlog-Zuordnung; Startlog `C:/Users/phili/AppData/Local/Temp/nakama-tiefenaudit-p08-979c4b94e9a139f726252cc74b8309fe029b0e83-start.log` nur gelesen. Geladene Skills: `C:/Users/phili/.codex/skills/unsafe-checker/SKILL.md` und `.agents/skills/nakama-plugin-quality/SKILL.md`; die ausdrückliche Lesebeschränkung dieses Auftrags ging vor deren ausführenden Workflows.

### Nicht vollständig gelesene und fehlende Pfade

- Die mit **S** markierten 38 Rust-Pfade wurden nicht als vollständige Funktionskörper-Lektüre behandelt; alle sind in der Population und den Suchläufen enthalten. Auch B bedeutet Auszüge, nicht automatisch Volllektüre der ganzen Datei.
- Nicht vollständig gelesen: `broker/src/generiert/**`, Drittbibliotheken, Buildartefakte und alte Rohlogs; weitere Dokumentationsbäume, Design/Studien, andere Plugin-Quellbäume außerhalb der gezielten Referenzen.
- **Fehlende Pfade der beauftragten Population: 0.** Falsch angesetzte Suchpfade wurden auf die vorhandenen Dateinamen korrigiert; sie wurden nicht als fehlende Produktdateien gewertet.
- Der Python-3.13-Launcher war in dieser Sitzung nicht verfügbar; die ausschließlich lesenden Inventare liefen mit vorhandenem `python -X utf8` (Python 3.14). Kein dadurch ausgelassener Test, weil Tests ausdrücklich nicht beauftragt waren.
- HEAD vor dem Lauf: `979c4b94e9a139f726252cc74b8309fe029b0e83`; beim Abschluss erneut derselbe HEAD. Branch `master`. Kein Stage, Commit oder Push.
- Vorbestehende unversionierte Pfade: `.claude/settings.local.json`, `briefing-hub/`, `docs/audits/2026-09-15-tiefenaudit/roh/phase-08-auftrag.txt`, `nimbalyst-local/`. Unberührt.
- Eigene Schreibfläche ausschließlich `roh/phase-08-broker.md` und der neue Phase-08-Anhang in `BEFUNDE.md`. Der vorherige BEFUNDE-Präfix umfasst **59.452 Bytes**, SHA-256 **a28f261245bead17d451118f17acdc98abb1eccfcf98547c4b901b16e1613987**; bei der Abschlusskontrolle bytegleich bestätigt. Die Befundtabellen sind wortgleich, alle fünf IDs eindeutig, die Zähler stimmen. `git diff --check` meldet keinen Fehler; es gibt keine gestagten Änderungen und die einzige Änderung an einem zuvor versionierten Pfad sind 14 angehängte Zeilen in `BEFUNDE.md`.
