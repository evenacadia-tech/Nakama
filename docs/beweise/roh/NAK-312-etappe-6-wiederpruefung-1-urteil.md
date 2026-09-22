URTEIL: PASS — beide Befunde geschlossen, nichts gebrochen

# NAK-312 Etappe 6 — Wiederprüfung 1 (Nacharbeit 1, `6cc849eb...6ff660ed`)

Prüfer: frischer, lesender Thread (Opus 5.5), 22.09.2026, gebundener Auftrag
`docs/beweise/roh/NAK-312-etappe-6-wiederpruefung-1-auftrag.txt` (Vorlage B).
Prüfbereich: `git diff 6cc849eb3a10860c8d48d46e0336edf916778cbb...6ff660edd36fdefb224d944f0d07e70a7addee17 -- eq-copilot/plugin/src/PluginEditor.cpp eq-copilot/plugin/tests/ShotTestMain.cpp tools/beweise.ps1`
(ein Commit `6ff660ed`). Gelesen mit git, den Lesewerkzeugen und `sha256sum`
beziehungsweise `stat`; kein Compiler, kein Test, kein FL Studio, kein Python;
nichts gestagt, committet oder gepusht. Einzige geschriebene Datei ist dieser
Bericht. Zeilenangaben gelten zum HEAD, JUCE-Zeilen zum gepinnten Stand unter
`eq-copilot/build/_deps/juce-src/modules`.

- **HEAD zu Beginn:** `412e93bdb24617ad55983eaf49aa480f2a4a6222`. Das ist
  `6ff660ed` plus der Auftragscommit `412e93bd`, der nur die Auftragsdatei
  anlegt; `git diff --stat 6ff660ed 412e93bd` nennt keine der drei
  Prüfdateien. Der Prüfgegenstand ist an beiden Ständen derselbe.
- **HEAD am Ende:** `412e93bdb24617ad55983eaf49aa480f2a4a6222`, gemessen nach
  dem Schreiben dieses Berichts; `git status --short` zeigt daneben nur
  `briefing-hub/`, `nimbalyst-local/` und diese Datei.

## Befund 1 — L-5 (DEFEKT gegen R-312-2 Satz 2): geschlossen

### (a) Kein Zugriff auf `ed` zwischen Hostaufruf und zweiter Prüfung

Der Rückruf am HEAD (`eq-copilot/plugin/src/PluginEditor.cpp:639-669`):

- `:641-643` erste Lebendprüfung, `:644-647` Marke (unter dem Makro).
- `:656` `auto& proz = ed->processor;` ist das einzige `ed->processor` im
  Rückruf und steht hinter der ersten Prüfung.
- `:657` `const bool neueKennung = proz.neueSensorId();` ist der Hostaufruf.
- `:658-660` `ed = safe.getComponent(); if (ed == nullptr) return;`. `:658`
  überschreibt nur den lokalen Zeiger und dereferenziert ihn nicht; `safe` ist
  die Kopie im Lambda.
- `:661-664` Marke (unter dem Makro), `:665-668` die zwei Schreibzugriffe. Der
  read-only-Zweig liest `proz.holeStateGrund()` in `:667`, also hinter der
  zweiten Prüfung.

Zwischen `:657` und `:658` steht keine Anweisung. Zwischen `:656` und `:665`
steht kein `ed->`. Damit ist die Forderung aus §41.6 erfüllt: zuerst die
Kennung, eine zweite Lebendprüfung vor jedem Schreibzugriff und zwischen dem
Hostaufruf und der zweiten Prüfung kein Zugriff auf `ed`.

**Die Prozessorreferenz vor dem Hostaufruf ist nach R-312-2 zulässig:**

1. `EqCopilotEditor::processor` ist eine Referenz (`PluginEditor.h:174`,
   `EqCopilotProcessor& processor;`). `auto& proz` bindet also an den
   Prozessor selbst, nicht an Speicher im Editor. Genommen wird sie hinter der
   ersten Lebendprüfung. In diesem Moment lebt der Editor und damit auch der
   Prozessor, denn JUCE baut den Editor vor dem Prozessor ab (`~AudioProcessor`
   verlangt `activeEditor == nullptr`, `juce_AudioProcessor.cpp:60-67`).
2. Nach dem Hostaufruf berührt der Rückruf den Prozessor nur in `:667`: im
   read-only-Zweig und hinter der zweiten Prüfung. Dieser Zweig ruft den Host
   überhaupt nicht, weil `State.cpp:58-59` vor `meldeHostDirty` zurückkehrt.
   Scheitert die zweite Prüfung, fasst der Rückruf weder Editor noch Prozessor
   an. R-312-2 Satz 2 („kein Zugriff, keine Mutation, keine Dirty-Meldung")
   gilt damit für alles, was der Rückruf nach dem Ende seines Eigentümers tut.
3. Dass der Prozessor den Hostaufruf überlebt, setzt nicht erst der Rückruf
   voraus. `neueSensorId()` arbeitet nach `meldeHostDirty()` selbst auf `this`
   weiter (`State.cpp:63-65`). Die eine Prozessoroperation läuft zu Ende: neue
   Kennung, genau ein Host-Dirty, Revision und Neuverbindung. Genau das sagt
   M-93 nach §41.6 zu (Lesart A-112).
4. Die Kopie von `safe` im Lambda überlebt den Abbau. Die CallOutBox ist ein
   Kind des Editors (`juce_CallOutBox.cpp:43-47`), gehört aber
   `CallOutBoxCallback` (`:62-94`) und damit dem ModalComponentManager.
   `~Component` des Editors entfernt seine Kinder, ohne sie zu löschen
   (`juce_Component.cpp:277-278`). Der ModalItem-Beobachter stößt dabei nur
   `triggerAsyncUpdate` an (`juce_ModalComponentManager.cpp:70-90`). Gelöscht
   wird erst in `handleAsyncUpdate` (`:196-215`). Panel, Knopf, Lambda und
   `safe` leben deshalb über das Ende des Rückrufs hinaus. `safe.getComponent()`
   liest die schwache Referenz, die in `juce_Component.cpp:280` geleert wird,
   und liefert null. Das anschließende `box->dismiss()` im `onClick`
   (`PluginEditor.cpp:599-600`) trifft die lebende Box und postet über eine
   `WeakReference` (`juce_Component.cpp:2096-2103`).

Die Hostkette ist nachgelesen: `State.cpp:54-67` → `Hostbruecke.cpp:237-240` →
`juce_AudioProcessor.cpp:431-436` (synchron, `getListenerLocked` je Index) →
`juce_audio_plugin_client_VST3.cpp:1502-1559` → `juce_VST3Common.h:1639-1656`
(auf dem Nachrichtenthread direkt `handleAsyncUpdate`) →
`juce_audio_plugin_client_VST3.cpp:1613-1622` (`setDirty (true)`,
`restartComponent`). Der gepostete Klick kommt über `Button::triggerClick` →
`handleCommandMessage` → `sendClickMessage`, und `onClick` ist dort der letzte
Aufruf (`juce_Button.cpp:359-362`, `:390-434`). Die Kommentare
`PluginEditor.cpp:648-655` und am Übergabepunkt `:615-628` beschreiben das
zutreffend. Der Satz „prueft ihn VOR jedem Zugriff" ist jetzt wahr: `:656`
steht hinter `:641-643`, `:665-668` hinter `:658-660`.

### (b) 312/M-93 misst Ereignis und Zusage, der Rotbeweis fällt an der zweiten Prüfung, kein undefiniertes Verhalten

Test in `eq-copilot/plugin/tests/ShotTestMain.cpp`:

- **Ereignis.** `KonfliktAbbauer` (`:1101-1113`) reagiert nur auf
  `nonParameterStateChanged` und läuft höchstens einmal, weil die Funktion vor
  dem Aufruf herausbewegt wird. Angemeldet wird er nur für M-93 und erst nach
  dem Öffnen des Popovers (`:1155-1169`). Die Aufbauprüfung legt fest, dass
  der Abbau innerhalb von `neueSensorId()` geschieht:
  - `abgebaut == 1`,
  - `eintritteBeimAbbau == 1`: Der Rückruf war eingetreten, und die Marke wurde
    genau einmal bei lebendem Editor gefragt (`:1130`),
  - `kennungBeimAbbau != kennungVorher`: Die Kennung war schon neu.

  Ein Abbau vor dem Eintritt (0 Eintritte), ein Abbau vor der Kennungsvergabe
  (alte Kennung) oder ein fehlender Abbau macht den Fall rot. Der Listener
  sitzt an derselben Stelle, an der der VST3-Wrapper als Listener gerufen wird.
  Der Testweg ist damit gleichwertig.
- **Zusage** (`:1262-1264`):
  - `marke == 0`: Die Stelle vor den Schreibzugriffen wird bei totem Editor
    nicht erreicht.
  - `dirty == 1`: `PanelDirty` (`:336-344`) zählt über den ganzen Lauf, auch
    unabhängig von der Reihenfolge der Listener.
  - `kennungBeimEnde != kennungVorher`, und die Kennung ist nicht leer.
  - `panelWeg`: Die Box ist abgebaut.
- **Rotbeweis** (`docs/beweise/roh/NAK-312-rot-M-93.txt`). Die Mutation
  entfernt genau `:658-660`. Unter ihr ist allein M-93 rot, mit „Marke 1" bei
  identischem Aufbau: Abbau 1 nach 1 Eintritt, Kennung beim Abbau neu,
  Host-Dirty 1, Kennung NEU, Box abgebaut. M-91 (a), M-91 (b) und M-92 bleiben
  grün, und die Rücknahme ist bytegleich. Der einzige abweichende Wert ist die
  Marke: Der Rotbeweis fällt an der Zusagezeile, nicht an einem Nebeneffekt.
  Nachgerechnet: `sha256sum` von `PluginEditor.cpp` am HEAD ergibt
  `c6c2ef90…f3cd`. Das ist der Wert, den die Rohdatei vor der Mutation und
  nach der Rücknahme nennt. Der grüne Lauf nach der Rücknahme lief also auf
  genau der committeten Datei.
- **Undefiniertes Verhalten, gelesen.** Im grünen Lauf wird `ed` nach dem Abbau
  nicht dereferenziert, weil `:658` ihn überschreibt. Panel und Lambda leben
  (Punkt 4 oben). `updateHostDisplay` verträgt den Abbau im Listener, weil es
  über `getListenerLocked` je Index liest. `bindungMutex` ist beim Hostaufruf
  frei, denn `State.cpp:56-61` gibt ihn vor `:62` zurück. Deshalb blockiert
  weder `holeZustandKopie()` im Abbauer noch `setzeEditorOffen (false)` im
  Editordestruktor (`Hostbruecke.cpp:247-257`). Der Editor entstand über
  `createEditor()`, `activeEditor` bleibt null, und die Assertion in
  `juce_AudioProcessorEditor.cpp:54` hält. In Rotbeweis und Gegenprobe kehrt
  die Marke vor der ersten Dereferenzierung zurück (R-312-12, R-312-23; die
  Mutationstexte beider Rohdateien sind gelesen).

### (c) M-91 (a), M-91 (b) und M-92 unverändert

- **Zusagen.** §35.2 (`docs/beweise/NAK-312.md:6389-6390`) liegt in keinem
  Hunk des Commits.
- **Tests.** Die drei Prüfausdrücke (`ShotTestMain.cpp:1243`, `:1249`,
  `:1255`) sind unverändert. Neu in ihrem Aufbau sind nur drei Dinge:
  - der ungeprüfte Zähler `eintritte`,
  - der `KonfliktAbbauer`, der in diesen Fällen nie angemeldet wird,
  - seine Abmeldung (`:1215`), nach `juce_AudioProcessor.cpp:344-348` ein
    No-op.

  M-91 kehrt wie bisher an der ersten Prüfung zurück. In M-92 passiert der
  Rückruf beide Prüfungen, und die zweite Marke antwortet bei lebendem Editor
  `true`. Statustext und genau ein Host-Dirty bleiben wie bisher.
- **Grün**, gelesen und nicht gefahren: Rohdatei Rotbeweis Abschnitt 5 und
  §42.7 „NAK-312 KONFLIKT 4 geprueft, 0 Fehler".

## Befund 2 — L-2 (LÜCKE, Regel R-312-24): geschlossen (Manifestprüfung)

| Zeile | neuer Wortlaut | gebauter Weg | Zusage |
|---|---|---|---|
| M-59, Frist | Abschluss aus `reset()` und `releaseResources()` über `markierungAbbrechen` und den RT-Ring (`Hostbruecke.cpp:190-224`), sperrfrei; der Block unter `sendeZustandMutex` bleibt allein `prepareToPlay` | `reset()` `:165-176` und `releaseResources()` `:178-188` rufen `markierungAbbrechen` `:190-224`; dort nur Atomics und der Ring (`eq-copilot/plugin/core/ipc/InterventionsRing.h:149-230`, feste Plätze, keine Sperre); `sendeZustandMutex` steht in `Hostbruecke.cpp` nur in `:104` (`prepareToPlay`) | „genau EIN `end` … ein zweites `end` entsteht nicht" unverändert |
| M-62 | Zustand voller Ring; das frühere (a) ausdrücklich kein Überlauffall; Frist `:216-221` | `:216-220` setzen das Überlaufbit und kehren vor dem `fetch_add` in `:221` zurück | fail-closed bei vollem Ring: Bit vorher nein, nachher ja; Status unbekannt; keine Sequenz, die nicht reist; das nächste begin ist die letzte Sequenz plus 1. Test (b) misst jedes Glied (`MarkierungTestMain.cpp:2143-2146`, auch `interventionStateUnknown`) |
| M-64 (b) | dasselbe `end` auf der Leitung (Kennung, `project_sample_end` null, tail) wie bei `reset()` und `releaseResources()`, deren `end` über den Ring reist; `prepareToPlay` schließt über den Block unter `sendeZustandMutex` | `prepareToPlay` `:102-158`; das Bein holt dessen totes Ende über den Linkaufbau (`MarkierungTestMain.cpp:2176-2183`) und prüft alle drei Wege mit demselben Prädikat `gleich` (`:2196-2200`) an den gesendeten JSON-Ereignissen (`ernte`, `:1577-1591`) | „dasselbe end auf der Leitung" bleibt |

- **Alle drei Stellen ersetzt.** Die Erstprüfung hatte unter L-2 drei Stellen
  genannt, deren Wortlaut den alten Bauplanweg beschreibt. Keine davon steht
  noch, und keine Verhaltenszusage ist schwächer geworden.
- **M-62 (a) zu Recht gestrichen.** Der frühere Zustand verlangte ein
  Überlaufbit in einer Lage, in der der Ring beide Intervalle schließt. Die
  Streichung erfüllt R-312-5 („genau ein end je begin") und ist in §41.6
  angeordnet.
- **Abweichung A-110 nötig.** Wörtlich aus §41.6 an (b) gehängt, hätte der
  Satz für `prepareToPlay` den Ringweg behauptet. Den geht der Code nicht:
  `prepareToPlay` schließt in `Hostbruecke.cpp:102-158` unter
  `sendeZustandMutex`.
- **§7.6 stimmt.** Der neue Vermerk dort stimmt mit dem Code überein.

## Nichts gebrochen

**Nachgerechnet oder an der Quelle geprüft:**

- **Diffumfang.** An Code und Runner ändert der Commit genau die drei
  Prüfpfade. Kein Audio-Pfad, kein State-, Schema-, Identitäts- oder
  Installpfad, kein `PluginEditor.h` und kein `neueSensorId` ist berührt.
- **Audio-Thread.** Der geänderte Rückruf läuft auf dem Nachrichtenthread
  (geposteter Klick). `processBlock` und `markierungAbbrechen` sind unberührt.
  Der Nulltest misst Audio, und dieser Pfad liegt nicht darin.
- **State und Host-Dirty.** `neueSensorId()` wird je Klick wie bisher genau
  einmal gerufen. Der Code enthält keine zusätzliche Meldung. M-92 und M-93
  verlangen je genau 1.
- **Oberfläche.** Es gibt kein neues Element. Beide Statustexte (`:666-667`)
  und der Handgriff sind unverändert.
- **Testzugänge.** Beide Marken im Rückruf (`PluginEditor.cpp:644-647`,
  `:661-664`) und die Markenfunktion (`:551-568`) stehen unter
  `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3`. Abbauer, Zähler und der neue Fall stehen
  nur im Test. Im Produktbau wirkt der Rückruf bei lebendem Editor wie vorher:
  Der Hostaufruf kommt vor der Zuweisung, dieselbe Reihenfolge wie in der
  alten Einzelanweisung.
- **Runner.** `tools/beweise.ps1:752` ändert nur die B15-Behauptung. Jedes
  Glied des neuen Satzes hat im Prüfausdruck `ShotTestMain.cpp:1262-1264` ein
  Maß: Abbau durch den Listener 1, nach dem Eintritt (1 Eintritt), Kennung
  schon neu, genau 1 Host-Dirty, Marke 0, Box abgebaut, Kennung am Ende neu.
  Einen Stillstand des Prozessors behauptet er nicht. Nachgerechnet am Blob:
  BOM `ef bb bf` vorher und nachher, die geänderte Zeile ist ASCII, und ihre
  14 einfachen Anführungszeichen sind nur die Begrenzer. Die Zeichenkette ist
  nicht gebrochen.
- **Kommentare (R-312-14).** Übergabepunkt, Rückruf, Markendoku und Testkopf
  beschreiben den Endstand zutreffend.

**Nur gelesen, nicht gemessen:**

- **Grüne Läufe.** B15 4/4, B14, A1 Nulltest und A3, die 31 bytegleichen Bilder
  und die clang-tidy-Teilmessung stammen aus §42.7 und den zwei Rohdateien.
  Ich habe nichts gebaut und nichts gefahren.
- **Fenster zwischen Hostaufruf und zweiter Prüfung.** Dass dort kein Zugriff
  steht, ist gelesen (`PluginEditor.cpp:656-660`). Die Marke steht nach
  R-312-23 hinter der Prüfung und sieht dieses Fenster konstruktionsbedingt
  nicht. Der Rotbeweis deckt das Entfernen der Prüfung ab, nicht einen später
  davor eingefügten Zugriff.
- **Kanon und Laufzeit-Arm.** Beide sind in dieser Runde nicht gefahren
  (Auftrag, §42.1). Sie gehören zum Abschluss der Etappe.

**Hinweis ohne Befundklasse** (docs/\*\*, außerhalb der Befundfläche):

- **Hash der Gegenprobe weicht ab.** Die Gegenprobe nennt für
  `ShotTestMain.cpp` SHA-256 `7DBFC214…DE08` und „Tests in der Endfassung". Am
  HEAD ergibt `sha256sum` `aa43307f…7190`.
- **Zeitfolge.** Laut `stat` wurde die Datei zuletzt 20:50:37 geschrieben:
  nach dem Gegenprobelauf (20:48:22) und vor dem Fixbau (20:51:03).
- **Was gleich ist.** Prüfname, Prüftext und Detailfelder im Log der
  Gegenprobe gleichen dem HEAD. Den Inhalt der Zwischenfassung kann ich nicht
  lesen.
- **Folge.** Rotbeweis (Bau 20:56:41) und Endstandläufe liefen mit der
  HEAD-Fassung; `EqCopShot.exe` (20:56:57) ist jünger als beide Quellen. Das
  Urteil hängt daran nicht.

## Zählung

Offene Befunde: 0. Brüche: 0. Neue Befunde aus dieser Wiederprüfung: keine.

FERTIG Wiederprüfung 1 Etappe 6, PASS, 0 offene Befunde, 0 Brüche.
