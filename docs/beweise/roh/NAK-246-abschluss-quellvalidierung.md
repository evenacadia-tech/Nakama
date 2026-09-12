# NAK-246 Abschlussprüfung — Quellvalidierung P1/P2 (lesend, HEAD a88c87e8)

Alle Zeilenangaben am Stand a88c87e8. Keine Datei im Repo geändert, kein Build,
kein Test gefahren.

## 0. Die fünf tragenden Quellfakten (beide Befunde hängen daran)

| # | Fakt | Beleg |
|---|---|---|
| F1 | `v3SessionEpoch` ist `const std::string`, wird **einmal im Konstruktor** aus `uuidHex32()` gesetzt und danach nie geschrieben. Ein Reload ändert sie nicht. | `eq-copilot/plugin/src/PluginProcessor.h:1689`; `eq-copilot/plugin/src/PluginProcessor.cpp:134`; `git grep -n v3SessionEpoch` findet genau vier Nicht-Binär-Treffer: `PluginProcessor.cpp:134`, `PluginProcessor.h:1689`, `prozessor/Ipc.cpp:467` (Wire-Adresse), `:1373` (Riegel), `:1479` (Testhaken) |
| F2 | Der Reload-Riegel prüft `nurLesen`, Klasse, `projectBindingId`, `sessionEpoch` — **keine Generation, keine Sequenz**. Bei Reload derselben Bindung sind alle vier Prüfungen erfüllt. | `eq-copilot/plugin/src/prozessor/Ipc.cpp:1369-1374` |
| F3 | `setStateInformation` nimmt **`sourcesDrainMutex` nicht**. Es leert die beiden Listen unter `sourcesCommandMutex` (`State.cpp:102-106`) und tauscht `zustand` unter `bindungMutex` (`:128-134` read-only-Zweig, `:141-149` Vollrestore). Ein Drain, der den Swap (`Ipc.cpp:1422`) schon hinter sich hat, hält seinen Batch **lokal** — die Leerung erreicht ihn nicht. | `eq-copilot/plugin/src/prozessor/State.cpp:93-156`; `Ipc.cpp:1360-1366`, `:1420-1435` |
| F4 | Die Modellpublikation liegt **außerhalb beider Prozessorsperren**: `bindungMutex` wird an `Ipc.cpp:1405` freigegeben, `sourcesDrainMutex` an `:1435`; `setzePersistenteMitglieder(kopie)` läuft an `:1406` ohne jede Sperre des Prozessors und **ohne Bindungs-, Versions- oder Generationsprüfung**. | `eq-copilot/plugin/src/prozessor/Ipc.cpp:1396-1409`, `:1436-1437` |
| F5 | `SourcesModel::setzePersistenteMitglieder` hat **keinen** Bindungs-/Generationsparameter; die einzige Abwehr ist `if (neu == persistenteMitglieder) return;` — eine Idempotenzprüfung gegen **gleichen Inhalt**, keine Altersprüfung. Nach `projektReload` ist der Inhalt ein anderer, also greift sie nicht. | `eq-copilot/plugin/src/SourcesModel.cpp:349-399` (Riegelzeile `:360-361`), `SourcesModel.h:220-223` |

Nebenfakt: der Worker-Drain läuft im Produkt **immer** (`Analyse.cpp:346-347`,
Schalter `workerDrainAusFuerTest` steht im Produkt nie:
`PluginProcessor.h:839-843`, `:1663`). `getStateInformation` (`State.cpp:79-91`)
und `sourcesTick` (`Ipc.cpp:1111-1118`) laufen auf dem Message-Thread, also auf
demselben Thread wie `setStateInformation` — **nur der Analyse-Worker** kann mit
dem Reload verschränken.

---

## P1 — Bereits abgeholter Batch überlebt den Reload

### (a) Ist die Ereignisfolge möglich? — **Ja, möglich** (Produktcode, kein Testhaken nötig)

Schritt für Schritt, Threads in Klammern:

1. (IPC-Callback) Ein `command_ack` mit `erfolgreich` schiebt den Befehl nach
   `bestaetigteSourcesCommands` — `Ipc.cpp:1069-1076`.
2. (Worker) `Analyse.cpp:346-347` ruft `wendeBestaetigteSourcesCommandsAn()`.
3. (Worker) `Ipc.cpp:1421` nimmt `sourcesDrainMutex`; `:1422` swappt den Vektor
   unter `sourcesCommandMutex` leer (`:1360-1366`). **Ab hier liegt der Batch
   ausschließlich lokal auf dem Workerstack** (`const auto befehle`).
   `sourcesCommandMutex` ist wieder frei.
4. (Message) Der Host ruft `setStateInformation`. `State.cpp:103-105` leert
   `ausstehendeSourcesCommands` und `bestaetigteSourcesCommands` — **beide sind
   bereits leer**, der lokale Batch aus Schritt 3 wird nicht erreicht (F3).
   `:141-149` tauscht `zustand = geladen` unter `bindungMutex`; `:150`
   `sourcesModel.projektReload (geladen.mainProjectMitglieder)`.
   `sourcesDrainMutex` wird an keiner Stelle genommen.
5. (Worker) `Ipc.cpp:1429-1434`: je Befehl `bindungMutex`, dann
   `wendeSourcesCommandAnUnterBindung`. Der Riegel `:1371-1373` prüft
   `nurLesen` (false im Vollrestore), Klasse (`main`), `projectBindingId`
   (**identisch**, weil derselbe Projektstate zurückgeladen wurde) und
   `v3SessionEpoch` (**konstant**, F1). Alle vier Prüfungen passieren.
6. (Worker) `:1379-1391` mutiert `zustand.mainProjectMitglieder` des **frisch
   geladenen** States: `unbind_probe` löscht das geladene Mitglied wieder,
   `confirm_join` trägt eines nach.
7. (Worker) `:1436-1437` → `meldeSourcesMitgliederNachBefehl` meldet dafür
   zusätzlich Host-Dirty (`:1407`) und erhöht `v3StateRevision` (`:1408`) —
   unmittelbar nach einem Ladevorgang, der laut `State.cpp:155` bewusst kein
   Dirty meldet.

Zu den drei Einzelfragen des Auftrags:

- **Ist `v3SessionEpoch` beim Reload derselben Bindung konstant?** Ja, sie ist
  `const` und wird nie neu gesetzt (F1). Der Epochenteil des Riegels kann
  „vor dem Reload" von „nach dem Reload" **innerhalb derselben Prozessorinstanz
  grundsätzlich nicht** unterscheiden — er trennt nur fremde Instanzen.
- **Wird der lokale Batch beim Reload irgendwo verworfen?** Nein. Die einzige
  Verwerfstelle ist `State.cpp:103-105`, und sie greift auf die **geteilten**
  Listen, nicht auf den bereits geswappten lokalen Vektor (F3). Es gibt keinen
  zweiten Verwerfpfad (`git grep -n bestaetigteSourcesCommands` nennt nur
  `Ipc.cpp:1074`, `:1364`, `:1452` und `State.cpp:105`).
- **Bindungs-/Versionsprüfung vorhanden?** Nur der Riegel `Ipc.cpp:1371-1373`,
  und der ist gegen diesen Fall wirkungslos (F2).

Der Fall ist **nicht** erreichbar, wenn der geladene State `nurLesen` ist
(`Ipc.cpp:1371`), eine andere Klasse trägt oder eine **andere**
`projectBindingId` hat — genau der Fall, den M-13 misst.

### (b) Welche Zusage bricht

**Keine Matrixzeile nennt den Fall ausdrücklich.** M-13 ist auf den
komplementären Fall zugeschnitten und schließt ihn per Formulierung aus
(`docs/beweise/NAK-246.md:615`, Zusagespalte wörtlich):

> M-13 | Projekt wird neu geladen (`setStateInformation`), ein ACK des **alten**
> Laufs (andere `project_binding_id` oder `session_epoch`) liegt vor; danach
> Speichern ohne Tick | **Der Reload-Riegel gilt auch für den Speicher-Drain.**
> `setStateInformation` leert beide Listen (`State.cpp:79-82`); ein ACK, das
> nach dem Reload für einen alten Lauf eintrifft, mutiert den neuen State nie
> (`Ipc.cpp:1340-1343`). Der Speicher-Drain wendet nur an, was zur aktuellen
> Bindung und Epoche gehört.

Zwei Einschränkungen in dieser Zeile sind entscheidend: „**andere**
`project_binding_id` oder `session_epoch`" und „ein ACK, das **nach** dem
Reload … eintrifft". P1 ist der Fall *gleiche Bindung* und *ACK vor dem Reload,
Anwendung nach dem Reload*.

§5.3 Feinheit 6 (`docs/beweise/NAK-246.md:1168-1171`), wörtlich:

> 6. **Reload-Riegel bleibt** (`Ipc.cpp:1340-1343`) und gilt für alle drei
>    Drains; `setStateInformation` leert beide Listen weiterhin (`State.cpp:79-82`).
>    Ein bestätigter Befehl, dessen ACK **nach** dem Reload eintrifft, gehört
>    zum alten Lauf und wird verworfen — das ist heutige, richtige Semantik.

Auch hier: nur „ACK **nach** dem Reload". **Der Fall „Batch vor dem Reload
abgeholt, nach dem Reload angewandt" steht in Matrix, §5.3 und §10.2 nirgends.**
R-D3 (`:1109-1118`) sagt zum Reload überhaupt nichts.

Was trotzdem trägt, sind zwei allgemeinere Zusagen:

1. **Der Quelltext selbst behauptet die Eigenschaft, die hier fällt.**
   `PluginProcessor.h:1117-1121`: „Reload-Riegel (Klasse, Bindung, Epoche) wie
   bisher: **ein ACK eines vor dem Reload gueltigen Laufs mutiert den neuen
   State nie.**" Und `Ipc.cpp:1374` im Riegel selbst: „ACK eines vor Reload
   gueltigen Laufs mutiert den neuen State nie." Genau das tut er in P1. Das ist
   ein Kommentar, der lügt — nach Ticketauftrag Zeile 32 ist „UI copy that lies"
   nicht aufgeführt, aber ein Testhaken-/Riegelkommentar, der eine nicht
   gehaltene Eigenschaft behauptet, ist zumindest ein belegter Widerspruch
   Quelle↔Quelle.
2. **CLAUDE.md „State bleibt verlustfrei"** (Manifest §1.2, `:86-88`, wörtlich):
   > **State bleibt verlustfrei.** Unbekannte Major-Versionen oder unzulässige
   > Matrizen werden read-only mit Originalbytes gehalten. Jede persistente
   > Änderung meldet dem Host Dirty-State.

   Der **erste** Satz greift nicht (der Fall ist kein read-only-Stand). Der
   **zweite** Satz ist formal sogar erfüllt (die Mutation meldet Dirty,
   `Ipc.cpp:1407`). Der Verlust selbst — ein geladenes Mitglied verschwindet —
   wird vom Wortlaut der Invariante **nicht** wörtlich getroffen. Die Zusage,
   die inhaltlich fällt, ist die Entwurfszeile §57 (Manifest §1.3, `:113-116`):
   > **Exit-Gate:** Save/Reload, Duplicate, Bridge/PID-Wechsel, zwei offene
   > Projekte, Brokerneustart und 60-Minuten-Soak erhalten richtige
   > Mitgliedschaft ohne falsche Steueradresse.

   „Save/Reload … erhalten richtige Mitgliedschaft" ist genau die Eigenschaft,
   die P1 bricht — und sie ist die **Quelle des Gate-Befundes D3**
   (Ticketauftrag Zeile 12).

**Einordnungshilfe:** Matrix und §5.3 sagen zu „Reload während laufendem Drain"
ausdrücklich **nichts** — das spricht für LÜCKE. Dagegen steht, dass das Ticket
den Speicher- und Workerdrain überhaupt erst eingeführt hat und die Zusage des
Tickets die §57-Zeile „Save/Reload … erhalten richtige Mitgliedschaft" ist; der
neue Workerdrain eröffnet einen Weg, auf dem genau diese Zeile fällt. Das ist
der Entscheidungspunkt des Dirigenten. Ich würde den Befund als **LÜCKE mit
DEFEKT-Charakter gegen Entwurf §57** vorlegen: die Matrix deckt ihn nicht, die
Gate-Zeile D3 schon.

### (c) Erreichbarkeit im Produkt

**Im Produkt erreichbar, Fenster sehr eng.** Kein Testhaken nötig: der Haken
`Ipc.cpp:1427-1428` ist im Produkt `nullptr`, die Verschränkung entsteht rein
aus Thread-Preemption. Das Fenster ist `Ipc.cpp:1422` → `:1431` (Swap bis erste
Anwendung) bzw. zwischen zwei Schleifendurchläufen `:1429-1434`, also
Nanosekunden bis zu einem Scheduler-Quantum. Damit es überhaupt gefüllt ist,
muss ein bestätigter Join/Unbind unangewandt vorliegen — das ist nur in den
Millisekunden zwischen Broker-ACK und dem nächsten Drain (Worker alle ≤ 50 ms)
der Fall. Produktlage: der User klickt Join/Unbind und der Host lädt im selben
Augenblick Projektstate (Projekt-Reload, Undo des Plugin-States, Preset-Recall,
Freeze/Unfreeze) — real, aber sehr unwahrscheinlich.
**Deterministisch ist er mit den vorhandenen Haken herstellbar** (siehe (e)).

---

## P2 — Modellpublikation mit überholter Zustandskopie

### (a) Ist die Ereignisfolge möglich? — **Ja, möglich** (Produktcode, kein Testhaken nötig)

1. (Worker) Ein Drain hat einen Befehl **gültig** angewandt (`Ipc.cpp:1432`,
   `geaendert == 1`), verlässt die Klammer `:1435` (`sourcesDrainMutex` frei)
   und ruft `:1436-1437` `meldeSourcesMitgliederNachBefehl()`.
2. (Worker) `:1403-1405`: `kopie = zustand.mainProjectMitglieder` unter
   `bindungMutex`; die Sperre wird an `:1405` **freigegeben**. Ab hier hält der
   Worker **keine** Prozessorsperre mehr (F4) — die Antwort auf die Auftragsfrage
   „sind beide Prozessorsperren frei?" ist: **ja, beide**.
3. (Message) `setStateInformation`: `State.cpp:128-134` (read-only) bzw.
   `:141-149` (Vollrestore) tauscht `zustand`; `:135` `projektReload({})` bzw.
   `:150` `projektReload(geladen.mainProjectMitglieder)` leert `eintraege` und
   baut die persistenten Mitglieder des geladenen States neu
   (`SourcesModel.cpp:411-424`) plus `sitzungszustandLeeren()` (`:442`).
4. (Worker) `Ipc.cpp:1406` `setzePersistenteMitglieder (kopie)` mit der **alten**
   Kopie. `SourcesModel.cpp:360-361` lässt sie durch (Inhalt ≠ aktueller
   Inhalt), `:362` überschreibt `persistenteMitglieder`, `:363-385` wirft die
   gerade geladenen Einträge hinaus (sie stehen nicht in `kopie` und sind nicht
   `fluechtigImSnapshot`), `:386-396` legt die **alten** Mitglieder als
   `Mitgliedschaft::bestaetigt` neu an. Anschließend `:1407` Host-Dirty und
   `:1408` Revision.
5. Ergebnis: `sourcesModel` (Sichtwahrheit für die UI) und `zustand`
   (Persistenzwahrheit) divergieren. Weitere leere Drains korrigieren das nicht:
   `Ipc.cpp:1423-1424` kehrt bei leerem Batch **vor** jeder Nachführung zurück,
   `meldeSourcesMitgliederNachBefehl` wird also nie „zur Reparatur" gerufen.
   Der nächste Korrekturpunkt ist ein echter Befehl, ein Snapshot des Brokers
   oder ein erneutes `setStateInformation`.

**Gilt auch bei unterschiedlicher Bindung** — richtig, weil `setzePersistenteMitglieder`
gar keinen Bindungsbezug kennt (F5). Wichtige Präzisierung gegenüber dem
Codex-Text: bei **fremder** Bindung im geladenen State wird der Befehl selbst
gar nicht mehr angewandt; der Fall entsteht dadurch, dass der Befehl **vor** dem
Reload noch gültig war und nur seine Publikation den Reload überholt.

### (b) Welche Zusage bricht

**Auch hier nennt keine Matrixzeile den Fall.** Was am nächsten kommt, ist
M-31 (`docs/beweise/NAK-246.md:653`), Zustands- und Zusagespalte wörtlich:

> M-31 | Was `projektReload` **behält**: persistente Mitglieder aus dem State,
> `diagnose = brokerUnavailable` mit Handgriff, Neuaufbau des
> `subscribe_session`-Auftrags | **Unverändert.** Die bestehenden B14-Zeilen zur
> Reload-Semantik bleiben grün; die persistenten Mitglieder kommen als
> `bestaetigt · getrennt · missing` zurück.

M-31 sagt „persistente Mitglieder **aus dem State**". In P2 stehen nach dem
Reload Mitglieder im Modell, die **nicht** aus dem geladenen State stammen —
die Zusage ist inhaltlich verletzt, aber das gemessene Ereignis von M-31 ist
`projektReload` **allein**, ohne konkurrierenden Drain. Der Test
`projektwechsel_leert_experimente_paare_befunde_und_ruecknahmen`
(`Sonde012ProjectReloadTest.cpp:1448`) schaltet den Worker-Drain sogar
ausdrücklich ab, und der Aufbaukommentar benennt den Grund wörtlich
(`Sonde012ProjectReloadTest.cpp:1421-1422`):

> Der Worker-Drain ist aus: das Speichern ist der einzige Drain, und **kein
> zweiter Thread fuehrt das Modell nach dem Laden noch nach.**

Dasselbe im Manifest, §12.2 E-2 (`docs/beweise/NAK-246.md:4290-4293`):

> Der Worker-Drain ist aus (Muster M-10), **damit kein zweiter Thread das Modell
> nach dem Laden nachführt** (NAK-175: Prozessor auf dem Heap).

Der Erbauer kannte die Nachführung nach dem Laden also, hat sie im **Test**
abgeschaltet und im **Produkt** nicht verriegelt. Das ist der stärkste Beleg
für P2 — und zugleich der Beleg, dass keine Zusage sie regelt.

Die Ordnungsbehauptung, die P2 falsifiziert, steht in §10.2 Punkt 1
(`docs/beweise/NAK-246.md:2637-2644`), wörtlich:

> Preis der Freigabe vor der Nachführung: zwei Drains könnten ihre Nachführungen
> verschränken. Deshalb reicht `meldeSourcesMitgliederNachBefehl` dem Modell
> nicht die beim Anwenden gezogene Kopie, sondern liest `mainProjectMitglieder`
> unter `bindungMutex` **zum Zeitpunkt der Nachführung** — der zuletzt
> Nachführende reicht immer den zuletzt angewandten Stand, **ein älterer kann
> nie hinter einem neueren landen** (`setzePersistenteMitglieder` ist gegenüber
> gleichem Stand idempotent, `SourcesModel.cpp:359-361`).

Diese Begründung deckt nur **zwei Drains** gegeneinander ab. `projektReload` ist
ein **dritter** Schreiber desselben Modells, den das Argument nicht kennt; gegen
ihn ist der Satz „ein älterer kann nie hinter einem neueren landen" falsch.
§10.2 ist Etappen-Entscheid, nicht Gate-Text — ein Bruch dort ist Beleg, keine
Gate-Verletzung für sich.

CLAUDE.md „State bleibt verlustfrei" greift bei P2 **schwächer** als bei P1: der
persistente `zustand` bleibt korrekt, nur die Modell-/Sichtwahrheit kippt. Die
verletzte Regel ist eher „Keine toten/lügenden UI-Elemente" bzw.
„Engine kennt keine Optik / der Editor rendert nur neue Snapshot-Revisionen" —
beides keine Gate-Zeile dieses Tickets. Inhaltlich getroffen ist M-31.

### (c) Erreichbarkeit im Produkt

**Im Produkt erreichbar, Fenster etwas breiter als P1, aber immer noch eng.**
Kein Haken nötig; das Fenster ist `Ipc.cpp:1405` → `:1406` plus der
sperrfreie Vorlauf in `SourcesModel.cpp:352-357` (Map-Aufbau mit Allokationen)
bis zur Sperre `:359`. Dasselbe Vorbedingungsproblem wie bei P1: es braucht
einen gerade angewandten Befehl. **Deterministisch nur mit einem neuen Haken**
zwischen `Ipc.cpp:1405` und `:1406` — der vorhandene
`setzeSourcesDrainHakenFuerTest` liegt an der falschen Stelle (`:1427`, vor der
Anwendung, innerhalb der Klammer).

---

## (d) Gemeinsame Ursache

**Ein Satz:** Reload und Persistenzabschluss teilen keinen gemeinsamen Riegel
und keine gemeinsame Generation — `setStateInformation` nimmt `sourcesDrainMutex`
nie (`State.cpp:93-156`), der einzige Reload-Riegel prüft eine Epoche, die über
die Lebenszeit des Prozessors konstant ist (`PluginProcessor.h:1689`,
`Ipc.cpp:1373`), und beide Nachlaufschritte des Drains — die Anwendung des
abgeholten Batches (`Ipc.cpp:1429-1434`) und die Modellpublikation (`:1406`) —
laufen ohne jede Prüfung darauf, ob der Zustand, für den sie gedacht waren, noch
derselbe ist.

Kurz: **P1 = der abgeholte Batch überlebt den Reload; P2 = die Kopie für die
Publikation überlebt den Reload.** Zwei Ausprägungen derselben fehlenden
Ordnung.

---

## (e) Kleinste schließende Regel

### Vorschlag (Technik)

Eine **Reload-Generation** im Prozessor, `std::atomic<std::uint64_t>
reloadGeneration { 0 }` in `PluginProcessor.h` neben `bindungMutex`:

1. `setStateInformation` erhöht sie **unter `bindungMutex`, im selben Block wie
   `zustand = geladen`** — `State.cpp:128-134` und `:141-149` (beide Zweige;
   der read-only-Zweig ist genauso ein Projektwechsel, wie `State.cpp:116-118`
   für `vergleichszustandLeeren` schon feststellt).
2. `wendeBestaetigteSourcesCommandsAn` liest sie **innerhalb** von
   `sourcesDrainMutex` direkt nach dem Swap (`Ipc.cpp:1422`) in eine lokale
   `generationBeimAbholen`.
3. Vor der Anwendung je Befehl (`Ipc.cpp:1431-1432`, unter `bindungMutex`, wo
   die Generation ohnehin stabil ist): weicht die Generation ab, wird der
   **ganze** abgeholte Batch verworfen und in einem Zähler
   (`sourcesBefehleNachReloadVerworfen`) festgehalten — verworfen, nicht
   teilweise angewandt, weil die ACK-Reihenfolge sonst zerfiele.
4. `meldeSourcesMitgliederNachBefehl` bekommt die Generation als Parameter und
   prüft sie **innerhalb desselben `bindungMutex`-Blocks**, in dem es die Kopie
   zieht (`Ipc.cpp:1403-1405`): weicht sie ab, wird **nicht publiziert** (kein
   `setzePersistenteMitglieder`, kein Dirty, keine Revision) und ebenfalls
   gezählt. Damit sind Kopie und Gültigkeitsprüfung atomar zueinander; die
   Publikation selbst bleibt außerhalb der Sperren und die Deadlock-Begründung
   aus §10.2 Punkt 1 (`docs/beweise/NAK-246.md:2631-2637`: Hostaufruf unter
   eigener Sperre) bleibt unangetastet.
5. Derselbe Vergleich deckt den Speicher-Drain (`State.cpp:79-91`) mit; dort ist
   er trivial erfüllt, weil Speichern und Laden auf demselben Thread liegen.

**Warum keine der beiden naheliegenden Alternativen:**
`setStateInformation` einfach `sourcesDrainMutex` nehmen zu lassen, legte
`projektReload`, `pipe.stop/start` und `controlV3.reconnect()` unter den Riegel
und brächte genau die Sperrenordnung zurück, die §10.2 Punkt 1 als Deadlock
ausschließt (Host ruft aus `restartComponent` synchron `getState`).
`v3SessionEpoch` beim Reload neu zu würfeln, ändert eine **Wire-Größe**
(`Ipc.cpp:467`, `h.adresse.sessionEpoch`) und damit die v3-Zieladresse —
Vertragsfläche, unverhältnismäßig. Die interne Generation ist der kleinste
Schnitt.

### Rotbeweis und Haken

| | vorhandene Haken reichen? | Beleg |
|---|---|---|
| **P1** | **Ja, deterministisch ohne neuen Haken.** `setzeSourcesDrainHakenFuerTest` (`PluginProcessor.h:837-838`) feuert an `Ipc.cpp:1427-1428` genau zwischen Swap und Anwendung, innerhalb der Klammer; `merkeSourcesCommandFuerTest(art, id)` ohne Bindungs-/Epochenargument (`PluginProcessor.h:828-831`, `Ipc.cpp:1464-1481`) baut einen Befehl mit **aktueller** Bindung und Epoche; `v3AntwortFuerTest(ack(...))` bestätigt ihn über den echten Weg. Ablauf: Mitglied per Speicher-Drain bestätigen, State mit `getStateInformation` sichern, `unbind_probe` einschleusen + ACK, Worker-Drain am Haken festhalten, auf dem Testthread `setStateInformation` **derselben Bytes** fahren, Haken freigeben, Ruhe abwarten (`sourcesDrainEintritteFuerTest`, `PluginProcessor.h:851-852`). Rot: das geladene Mitglied ist weg. | `Sonde012ProjectReloadTest.cpp:954` zeigt das Haken-Muster (M-12); `:1621-1634` zeigt den Same-Instance-Reload |
| **P2** | **Nein — ein neuer Haken nötig**, zwischen `Ipc.cpp:1405` und `:1406` (bzw. als Parameterhaken in `meldeSourcesMitgliederNachBefehl`). Der vorhandene Haken liegt vor der Anwendung, nicht vor der Publikation. Der Rest (Einschleusen, ACK, `setzeWorkerDrainFuerTest`) ist vorhanden. | `Ipc.cpp:1427` gegen `:1403-1406`; Haken-Inventar `PluginProcessor.h:837-838`, `:1662` |

**Bein:** beide Fälle gehören nach **B14 `EqCopSonde012ProjectReloadTest`**
(`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp`), neben M-13
(`:1045-1114`) und M-12 (`:940-1039`); der `--nur`-Schalter existiert bereits
(`:1543`, Fallwähler `:740`, `:1275`, `:1532`) und nimmt zwei neue Kennungen
ohne Umbau auf.

**Es gibt heute keinen Fall, der Reload gegen einen laufenden Drain misst.**
`setzeSourcesDrainHakenFuerTest` wird in der ganzen Testdatei genau einmal
benutzt (`:954`, M-12, ohne Reload); `setStateInformation` erscheint an `:875`,
`:1067`, `:1101`, `:1268`, `:1462`, `:1501`, `:1625`, `:1640`, `:1770` — keine
dieser Stellen läuft gegen einen angehaltenen oder aktiven Drain (M-29 schaltet
den Worker sogar ab, `:1427`).

### Kollidiert das Verwerfen mit R-D4?

**Nein.** R-D4 lautet wörtlich (Manifest §5.4, `:1177-1187`, Ticketauftrag
Zeile 21): „… inFlight bekommt einen Deckel … und weist bei vollem Register
endgültig ab, statt unbegrenzt zu halten; **bereits angenommene Befehle werden
nie verworfen**." Der Satz steht im Zusammenhang des **Sendewegs**: die
Feinheiten §5.4 Punkt 1 (`:1191-1199`) bilden ihn ausschließlich auf
Rückgabewerte von `Nachrichten.cpp` ab (eingereiht / zur Wiederholung
angenommen / endgültig abgewiesen) und auf das Replay unter derselben
`command_id`. „Angenommen" heißt dort „zur Zustellung angenommen", nicht
„brokerseitig angewandt und lokal noch nicht übernommen".

Dass ein bestätigter, noch nicht angewandter Befehl beim Reload fällt, ist
**heute schon geltende und ausdrücklich gewollte Semantik**:
`State.cpp:103-105` leert `bestaetigteSourcesCommands` bei jedem akzeptierten
Reload, und §5.3 Feinheit 6 (`:1170-1171`) nennt das „heutige, **richtige**
Semantik". Der Vorschlag dehnt diese bestehende Regel lediglich auf den bereits
abgeholten Batch aus — er führt keine neue Verwerfklasse ein.

### Wer ist nach dem Reload die Wahrheit?

**Der geladene State**, nicht der bestätigte Befehl. Belege:

- Entwurf §57 Exit-Gate (`docs/FL-Nakama-Sonden-Design-Entwurf.md:4273-4275`,
  Manifest §1.3 `:113-116`): „Save/Reload … **erhalten richtige
  Mitgliedschaft** ohne falsche Steueradresse." Die Mitgliedschaft nach einem
  Reload ist die des geladenen Projekts.
- `Ipc.cpp:1374` (Riegelkommentar): „ACK eines vor Reload gueltigen Laufs
  mutiert den **neuen** State nie."
- `State.cpp:103-105` verwirft beide Listen; `State.cpp:155`: „Kein Host-Dirty:
  Laden und Migration sind keine Aenderung des Users."
- Der Abgleich mit dem Broker läuft ohnehin neu an: `State.cpp:151-154`
  (`pipe.start`/`reconnect`, `controlV3.reconnect()`), gemessen durch
  `project_reload_reconnect_path_builds_a_fresh_subscribe`
  (`Sonde012ProjectReloadTest.cpp:1661-1663`). Eine brokerseitig angewandte
  Mitgliedschaft, die im geladenen State fehlt, kommt über den frischen
  `subscribe_session` und den nächsten Sessionsnapshot als **flüchtige** Quelle
  zurück — sie geht nicht verloren, sie wird nur nicht mehr als *persistent*
  in ein fremdes Projekt geschrieben.

Bleibende, ehrlich zu benennende Folge: ein Unbind, den der Broker angewandt
hat und der lokal beim Reload verworfen wird, hinterlässt einen Unterschied
zwischen Broker und geladenem Projektstate, bis der User ihn erneut auslöst.
Das ist dieselbe Lage wie heute schon bei `State.cpp:105` und gehört, wenn der
Dirigent sie geschlossen sehen will, in eine eigene Zeile in
`docs/offene-punkte.md` — nicht in diese Nacharbeit.

---

## (f) Ticketpfade einer Nacharbeit

| Pfad | Warum | In der Ticketgrenze? |
|---|---|---|
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | Generation nach dem Swap lesen (`:1422`), vor Anwendung (`:1431`) und vor Publikation (`:1403-1406`) vergleichen; neuer Publikationshaken | **Ja** — Kopftabelle „Ticketpfade" (`docs/beweise/NAK-246.md:16`) nennt ihn wörtlich |
| `eq-copilot/plugin/src/prozessor/State.cpp` | Generation in beiden Reload-Zweigen unter `bindungMutex` erhöhen (`:128-134`, `:141-149`) | **Ja**, ebenda |
| `eq-copilot/plugin/src/PluginProcessor.h` | `reloadGeneration`, Verwerfzähler, Signatur von `meldeSourcesMitgliederNachBefehl`, neuer Testhaken; Kommentar `:1117-1121` berichtigen | **Ja**, ebenda |
| `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` | zwei neue Fälle plus `--nur`-Kennungen | **Ja**, ebenda |
| `eq-copilot/plugin/src/SourcesModel.cpp/.h` | **nur falls** der Dirigent die Prüfung ins Modell legen will statt in den Prozessor; der Vorschlag oben braucht sie **nicht** | **Ja**, ebenda (aber vermeidbar) |
| `eq-copilot/plugin/src/prozessor/Analyse.cpp` | **nicht nötig** — der Aufrufer `:346-347` bleibt unverändert | Ja (§4.7-Ergänzung, Etappe 4) |
| `docs/beweise/NAK-246.md`, `docs/beweise/roh/NAK-246-*` | Manifest und Rotbeweise | Ja, ebenda |

**Alle nötigen Pfade liegen in der Ticketgrenze.** Zusätzlich gehören die drei
lügenden Kommentarstellen in denselben Änderungssatz, weil sie sonst nach dem
Fix falsch bleiben bzw. unvollständig sind: `PluginProcessor.h:1117-1121`,
`Ipc.cpp:1374` und `docs/beweise/NAK-246.md:2637-2644` (§10.2 Punkt 1, „ein
älterer kann nie hinter einem neueren landen").

---

## Urteilszeilen

- **P1: möglich** — bricht Entwurf §57 Exit-Gate (`:4273-4275`, „Save/Reload …
  erhalten richtige Mitgliedschaft", Gate-Zeile D3) und die Eigenschaftszusage
  der Quelle selbst (`PluginProcessor.h:1117-1121`, `Ipc.cpp:1374`); Matrix
  (M-13, `NAK-246.md:615`) und §5.3 Feinheit 6 (`:1168-1171`) decken den Fall
  ausdrücklich **nicht** — Kandidat **LÜCKE mit DEFEKT-Charakter gegen §57**
  (Dirigentenentscheid: DEFEKT, wenn die Gate-Zeile D3 über die Matrixgrenze
  gestellt wird; sonst LÜCKE).
- **P2: möglich** — bricht M-31 inhaltlich („persistente Mitglieder **aus dem
  State**", `NAK-246.md:653`) und die Ordnungsbegründung §10.2 Punkt 1
  (`:2637-2644`); die Matrix misst M-31 aber nur ohne konkurrierenden Drain, und
  §12.2 E-2 (`:4290-4293`) schaltet den zweiten Thread im Test ausdrücklich ab,
  statt ihn im Produkt zu verriegeln — Kandidat **LÜCKE** (DEFEKT nur, wenn der
  Dirigent M-31 als bedingungslose Zusage liest).

Ausgabedatei:
`C:\Users\phili\AppData\Local\Temp\claude\C--Users-phili-Projekte-Nakama\0c396e8f-513e-4208-88b1-9033ee51f6b9\scratchpad\NAK-246-abschluss-quellvalidierung.md`
