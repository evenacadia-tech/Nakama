URTEIL: PASS — kein DEFEKT (Lücken und Härtungen sind benannt)

# NAK-312 S25k, Etappe 6 (6a und 6b) — Erstprüfung T2

- Prüfer: frischer, lesender Prüfer (Claude Opus 5.5), 22.09.2026; weder Bauer noch Prüfer oder Validierer einer früheren Runde.
- Prüfbereich: `git diff fd287d3c8ea277814c80e67365ea782e6af9994d...86c31f4673b84398c2a8c1ca4063dd5e2d757de8` über die 13 Pfade des Auftrags (Commits `ba32bc98`, `9aa776da`, `5f530041`, `db682244`, `53754c66`, `6fbe396a`, `86c31f46`). Geurteilt über den Zielstand. Für alle 13 Pfade ist der Arbeitsbaum gleich ZIEL (`git diff 86c31f46 HEAD -- eq-copilot/plugin/src eq-copilot/plugin/tests` leer, `git status` ohne Änderung an ihnen).
- Arbeitsweise: nur lesende Kommandos (`git rev-parse`, `status`, `log`, `diff`, `show`, `ls-files`, Suche, Lesen), JUCE-Quelle unter `eq-copilot/build/_deps/juce-src` gelesen; kein Compiler, kein Test, kein Python, kein FL. Einzige geschriebene Datei: dieser Bericht.
- HEAD zu Beginn: `87e8af15f6e748f2a14e8c7848084aae949ae60e`. HEAD am Ende: `87e8af15f6e748f2a14e8c7848084aae949ae60e`.

## Defekte

Keine. Am Zielstand bricht in keinem reproduzierbaren, vertragsgemäßen Ablauf eine Matrixzeile (M-55 bis M-77, M-84 bis M-86, M-89 bis M-92, M-40 erweitert), ein Satz des Gate-Textes oder eine Invariante aus CLAUDE.md.

## Lücken (6)

**L-1 — `reset()` gleichzeitig zu `process()`: Hostvertrag, für FL ungemessen.**
`markierungAbbrechen` (`eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:190-224`) und `brichAb` (`eq-copilot/plugin/src/HoerMarkierung.h:337-351`) setzen voraus, dass `reset()` nie neben `processBlock` läuft (Kommentar `Hostbruecke.cpp:198-199`). Der Wrapper ruft `reset()` aus `setProcessing (false)` ohne Sperre (`juce_audio_plugin_client_VST3.cpp:3612-3618`). JUCEs FL-Sperre (`:3815-3828`) umschließt `setActive` (`:2811`), `process` (`:3716`), `activateBus` (`:3316`) und `setBusArrangements` (`:3461`), nicht `setProcessing`. In FL laufen `prepareToPlay` und `releaseResources` also zusätzlich unter dieser Sperre gegen `process`, `reset()` nicht; der Satz „dieselbe Annahme, unter der prepareToPlay …" trägt für `reset()` nur über den VST3-Vertrag (setProcessing(false) nach dem letzten process). Gerechneter Ablauf bei Vertragsbruch (reset auf dem UI-Thread, während ein Block färbt): `brichAb` setzt `fade`, `warHoerbar`, `hoerbareSamples` und die Filterzustände zurück; der Block schreibt danach `fade = fEnde` zurück (`HoerMarkierung.h:641`) und meldet mit `warHoerbar == false` ein neues begin (`:659-663`) — der Klang endet nicht. Zwei Schreiber am SPSC-Ring (`eq-copilot/plugin/core/ipc/InterventionsRing.h:162-176`, `kopf` relaxed gelesen) können das end überschreiben; die Sequenzlücke setzt beim Broker unknown (fail-closed, keine scheinbar saubere Baseline). M-86 gilt damit nur im sequentiellen Vertrag; Gate, Matrix und Entwurf sagen zu gleichzeitigen Aufrufen nichts. Der Bauer hat die Annahme benannt (§36.16, §36.17). Wege, falls gewünscht: Thread und Reihenfolge von setProcessing und process im Laufzeit-Arm messen, oder `reset()` nur eine Anforderung setzen lassen, die der nächste Block einlöst (dann reist das end erst mit dem nächsten Block).

**L-2 — A-78 und A-79 gegen den Wortlaut von M-59, M-62 und M-64.**
Die messbaren Zusagen halten: M-59 (genau ein end im Ring, `project_sample_end` null, gezählte Hördauer), M-64 (dasselbe end auf der Leitung für reset, release und prepare: Kennung, null, tail), M-86. Wörtlich nicht erfüllt sind drei Stellen, die den ersetzten Bauplanweg beschreiben: M-59, Spalte „Reihenfolge und Frist" („derselbe Abschlussweg wie prepareToPlay … Hostbruecke.cpp:97-142"); M-64, Zusage (b) („der Fix verwendet den vorhandenen Weg wieder, statt einen zweiten zu bauen"); M-62, Zusage für ihren Zustand (zwei offene Intervalle → `interventionsRingUeberlauf` wird true). Mit dem Ringweg ist der Zustand von M-62 (a) kein Überlaufzustand mehr: beide Intervalle schließen lückenlos (gemessen begin@1 end@2 begin@3 end@4). Ein dort gesetztes Überlaufbit wäre ein falsches unknown und widerspräche R-312-5 („genau ein end je begin"). Der Kern von M-62 (fail-closed, keine Sequenz, die nicht reist) ist mit vollem Ring gemessen (M-62 (b)). Der Bauplanweg hätte M-59 selbst verfehlt: totes Ende im Sendezustand statt im Ring, Abholung erst bei einem Linkaufbau (A-78, Punkt 2). Kein Codefehler; ob die drei Stellen nachgezogen werden, entscheidet der Dirigent.

**L-3 — M-84 unter Produkttakt: das end nach „weg von Main" reist nicht.**
Der Sender leert und verwirft den Ring, sobald die Instanz nicht mehr „main" meldet (`eq-copilot/plugin/src/prozessor/Ipc.cpp:139-147`; `pluginKind` aus `zustand.common.klasse`, `:473`; seit BASIS unverändert, außerhalb des Änderungssatzes). Im Szenario S2 entsteht das end am Fadeende, rund 80 ms nach `setzeBindung ("sensor")`. Läuft der Worker vor einer Rückkehr zu Main, wird es verworfen; `offenesBegin` bleibt stehen, `sendeBeginOffen` bleibt true, die Instanz meldet danach keine Neutralität, und ein späterer Main-Aufbau setzt `replayFaellig` (`Ipc.cpp:1006-1023`) und replayt das alte begin ohne end (gerechnet, nicht gemessen). Das A3-Bein misst M-84 mit bestätigt ruhendem Sender (A-75) und erntet erst nach der Rückkehr zu hub (`eq-copilot/plugin/tests/MarkierungTestMain.cpp:1796-1817`); die A3-Zeile beschreibt genau diese Folge. Im Produkt heute nur über den Restfall von NAK-341 erreichbar (kein Auslöser und kein Rollenwähler in der Main-Fläche, „weg von Main" nur über `setStateInformation`); mit einem Auslöser in der Main-Fläche wird S2 regulär. Am BASIS-Stand derselbe Weg (Ausfade in Legacy, end verworfen). Gate und Matrix sagen zur Zustellung eines end nach dem Wechsel der Instanzklasse nichts.

**L-4 — Registervorschlag §38.28 (1): abgewiesener Entwurf im Umordnungsfall.**
Scheitert `benenneSourcesHauptziel` im Abschluss vor dem Leeren des Ziels (`eq-copilot/plugin/src/PluginEditor.cpp:1046-1047`), setzt `uebernehmeSourcesLabel` Fehlerbit und Fehlertext (`:1017-1022`); dieselbe Funktion löscht das Bit gleich danach, weil das Aktionsziel wechselt (`:1049-1050`), und der Status sagt „Main target is outside the drawn list - select a drawn source." (`:1062-1064`). Die Kennung fällt (`:1082-1086`); kehrt das Ziel in die Liste zurück, lädt das Feld den gespeicherten Namen — der Entwurf ist ohne Meldung weg. Keine Mutation, kein fremdes Schreiben, der sichtbare Status ist wahr. Keine Matrixzeile regelt den abgewiesenen Abschluss (M-85 setzt den Erfolg voraus). Erreichbar nur, wenn im selben Tick das Schreibrecht fällt oder das Mitglied im Zustand fehlt.

**L-5 — Registervorschlag §38.28 (2): Editorabbau innerhalb des Konfliktrückrufs.**
`PluginEditor.cpp:633-643`: der SafePointer wird einmal am Eintritt geprüft; die Zuweisung an `ed->statusMeldung` und `ed->statusMeldungBisMs` geschieht nach `ed->processor.neueSensorId()` ohne zweite Prüfung. Baute der Host den Editor synchron in seiner Reaktion auf Host-Dirty oder restartComponent ab, schriebe der Rückruf in einen toten Editor. R-312-2 deckt den Fall im Wortlaut („nach dem Ende seines Eigentümers … kein Zugriff"); M-91 misst nur den Abbau vor der Zustellung; ein Hostweg ist nicht belegt (`docs/beweise/roh/NAK-349-quellvalidierung.md`, V-2). Kleinster Nachzug: den SafePointer nach `neueSensorId()` erneut prüfen.

**L-6 — M-56 „der Grund steht im Status" ist in der Main-Fläche nicht sichtbar.**
Die drei Netze setzen `statusMeldung` über `markierungBeenden` (`PluginEditor.cpp:919-927`); gezeichnet wird `statusMeldung` nur in der Legacy-Fläche (`:1819-1831`), `paintMainFlaeche` zeigt allein `sourcesBedienstatus` (`:1428-1434`). Das Bein liest die Variable (`statusMeldungFuerTest`). In der Main-Fläche sieht der User damit weder Latch noch Endgrund — dieselbe Produktlücke wie NAK-341 (Auslöser, Aus-Knopf und Markierungsanzeige nur in Legacy); kein sichtbares Element behauptet etwas Falsches. Die Matrix verlangt nicht, dass der Grund in der Main-Fläche sichtbar ist.

## Härtungen (6)

**H-1 — Der Allokationszähler von M-60 (und B7) sieht nur `operator new` und `new[]`.**
`MarkierungTestMain.cpp:42-65` zählt thread-lokal je Aufrufthread die ersetzten `operator new` und `new[]`; `std::malloc`, `calloc`, `realloc` (juce::HeapBlock, juce::MemoryBlock, juce::Array), ausgerichtetes `new` und Heap-APIs sieht er nicht. `docs/beweise/roh/NAK-312-rot-M-60.txt` zeigt es: unter der Mutation „vorbereiten statt brichAb" meldet er 0 Allokationen, rot wird die Zeile allein über den Wet-Zuteilungszähler (A-80). „versuchTrocken.assign läuft nicht" ist für ein assign gleicher Größe nicht messbar (keine Zuteilung, gleiche Kapazität). Falsch rot ist ausgeschlossen (thread-lokal, synchron), falsch grün für C-Allokationen möglich. Der Pfad ist nach Lesen allokationsfrei (siehe A-78). Die Zeilen A3 (`tools/beweise.ps1:509`) und B7 (`:728`) sagen „0 Allokationen"; sie könnten die Zählart nennen.

**H-2 — Testzähler im Produktpfad von `HoerMarkierung.h`.**
`++zuteilungen` (`:307`), das Mitglied (`:855`) und die Leser `pufferZuteilungen()`, `pufferKapazitaet()` (`:743-744`) stehen ohne Testmakro. Alle übrigen neuen Testzugänge liegen unter `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` (`PluginEditor.h:60-122`, `:196-198`; `PluginProcessor.h:539-1071`; `PluginEditor.cpp:551-566`, `:636-639`, `:1105-1110`). Verhaltensneutral (Vorbereitungskontext, kein Audiothread); die Datei trägt schon unbedingte Testhaken. Unter das Makro stellbar.

**H-3 — M-68 (b): das Ereignis der Matrixzeile wird nicht gefahren.**
Die Zeile nennt „Fokusverlust durch Klick außerhalb der Liste"; das Bein fährt „Klick auf B, Tick, Fokusverlust" (`eq-copilot/plugin/tests/ShotTestMain.cpp:702-714`). Dort schreibt `mouseDown`, und der Fokusverlust ist ein No-op; kein anderer Fall ruft den Fokusverlust. Entfernte man die Verdrahtung `onFocusLost` (`PluginEditor.cpp:210`), bliebe jedes Bein grün, obwohl ein Entwurf beim Verlassen des Feldes dann im nächsten Tick still verfiele. Das Verhalten hält nach Lesen (dieselbe Funktion wie Enter, deren Schreiben M-71 (b) und (c) messen).

**H-4 — Die Anzeigeräumung fragt die Klassifikation nur zum Tick.**
`PluginEditor.cpp:942-949`: zwei Klassifikationswechsel innerhalb eines Ticks (Main, Legacy, Main — nur über `setStateInformation` möglich) lassen `markModus` stehen, obwohl die Rücknahme publiziert ist. Sichtbar allenfalls im Restfall-Popover von NAK-341; heilt beim nächsten Netz oder Rollenwechsel. Ein Wechselzähler der Klassifikation, im Tick verglichen, schlösse es.

**H-5 — Registervorschlag §38.28 (3): Meldung für einen still verfallenden Entwurf.**
M-69 verlangt ausdrücklich „verfällt ohne Mutation", ohne Meldung; eine Meldung ist Design (geparkt), keine Zusage.

**H-6 — Der neue Publisher-Aufruf in `spiegleKlassifikation` hängt am VST3-Threadmodell.**
`eq-copilot/plugin/src/prozessor/State.cpp:286-290` ruft `reicheAus()` (Tripelpuffer mit genau einem Publisher, `schreibIndex` nicht atomar, `HoerMarkierung.h:385-408`) auch aus `setStateInformation`. Das hält, solange der Host setState auf dem UI-Thread ruft (VST3-Vertrag; Kommentar `State.cpp:364-366`). Bisher berührte ein Aufruf von einem fremden Thread dort nur Atomics. Eine Threadwache im Testbau machte die Annahme prüfbar.

## Einordnung A-78 und A-81 (Produktcode abseits des Bauplans)

**A-78 — Abschluss aus `reset()` und `releaseResources()` über den RT-Ring: technische Entscheidung innerhalb der Gate-GRENZE, kein Zusagebruch.** Nachgerechnet: `markierungAbbrechen` nimmt keine Sperre und teilt nichts zu — `brichAb` ist noexcept ohne Zuteilung, das Ereignis ist POD auf dem Stack, `tailSamplesFuer` ist rein, der Ring schreibt in ein `std::array`. Die Overrides rufen keine JUCE-Basis (`AudioProcessor::reset()` leer, `juce_AudioProcessor.cpp:589`; `releaseResources()` rein virtuell, `juce_AudioProcessor.h:142`). Die Sequenz wird nur gezogen, wenn der Ring Platz hat (`Hostbruecke.cpp:216-221`). Der Worker setzt die Sequenz im Aufbau nur bei Neutralität auf 0 (`Ipc.cpp:956-971`); neutral ist der Prozessor in diesem Fenster nicht (begin im Ring oder `sendeBeginOffen`, `hoerbarAtomic` bleibt bis zum nächsten Block stehen). Auf der Leitung gleicht das end dem aus `prepareToPlay` (Kennung aus `nummer`, `project_sample_end` null, tail aus Dauer und letzter gültiger Rate). Der Bauplanweg hätte `sendeZustandMutex` genommen und das end ohne Linkaufbau liegen lassen. Folgen für den Matrixwortlaut: L-2; Nebenläufigkeit: L-1.

**A-81 — Eingriffsnummer in `prepareToPlay`: technische Entscheidung innerhalb der GRENZE, trägt die Zusage.** Nachgerechnet: ein offenes Intervall trägt immer die zuletzt gezogene Nummer (`PluginProcessor.cpp:1000-1003`; `warHoerbar` setzt einen begin-Übergang voraus). Hält der Sender ein älteres begin für lebend, dessen end schon im Ring liegt (aus `reset()` oder aus einem Block), weicht die Nummer ab, und der Übergang wartet (`Hostbruecke.cpp:118-157`); der Sender reiht ihn vor das erste Ringereignis mit höherer Sequenz oder ans Ringende (`Ipc.cpp:353-357`, `:395-401`) — Leitung `end:A begin:B end:B`. Ein berechtigtes end unterdrückt sie nicht: sie lenkt nur vom Anhängen an das offene Begin auf den Wartepfad; wartet schon ein Übergang, greift der vorbestehende fail-closed-Überlauf ohne gezogene Sequenz. Nebenbei schließt sie die vorbestehende Fehlzuordnung „Block-end im Ring, neues begin, prepareToPlay" (§36.17).

**Übrige Abweichungen mit Produktcode, geprüft:** A-89 (Bestätigung im Klick vor `waehleSourcesHauptziel`, `PluginEditor.cpp:1158-1160`); A-95 (die zweite Zustellung findet A nicht mehr als Hauptziel, `:1000-1008`; bei gescheitertem Zielwechsel No-op über `:1010`); A-90 (Escape leert die Kennung vor dem Neuladen, `:214-220`, schreibt nie); A-92 (Kennung = instance_id und Runtime-Nonce, Vergleich gegen `processor.sourcesSicht()`, `:1000-1008`); A-97 (eine Ordnungsfunktion für Sortierung und Ersatz, `SourcesModel.cpp:347-366`, `:1672-1675`, `:1793-1798`); A-99 und A-100 (Statustext in der vorhandenen Zeile, Beschriftung nur bei sichtbarem Feld, `PluginEditor.cpp:1062-1064`, `:1503-1510`) — je ein nötiger Handgriff ohne neues Element und ohne Geometrie.

## Prüfpunkte (a) bis (i)

**(a) Rollenwechsel.** Nachgerechnet: der Tausch in `reicheAus` (acq_rel, `HoerMarkierung.h:400-401`) ist vor dem Store `istMainKlassifiziert.store (main)` (seq_cst, damit release; `State.cpp:290`) sequenziert. Liest ein Block mit acquire (`PluginProcessor.cpp:877`) den neuen Wert, besteht synchronizes-with; der Tausch geschieht vor dem Briefkastenlesen in `verarbeite` (`HoerMarkierung.h:512`), und nach Schreib-Lese-Kohärenz sieht es den Tausch oder eine spätere Änderung (ein späterer Tausch des Audiothreads hat den Aus-Auftrag dann schon übernommen). Ein Block mit der neuen Klassifikation färbt den alten Auftrag nicht. Im Audiothread ändert sich nur die Leseordnung: keine Sperre, keine Zuteilung. Die Rücknahme ist ein Aus-Auftrag, die Rampe läuft weich zu Ende (`:604-641`), das end fällt am Fadeende. Beim zweiten Wechsel zu Main ist `zielGesetzt()` false, es gibt keine neue Publikation, `ausGewuenscht` bleibt, kein neues begin — auch mitten im Ausfade. M-57: der Aus-Knopf folgt `markModus` (`PluginEditor.cpp:295`); `wechsleFlaecheWennNoetig` macht ihn beim Wechsel nach Legacy kurz sichtbar, derselbe Tick blendet ihn vor jedem Zeichnen wieder aus. M-58: „Markierung nicht möglich — nur ein Main färbt hörbar, diese Instanz bleibt neutral." ist wahr (nichts eingereicht, audio-neutral nach §53.5).

**(b) Drei Netze.** `PluginEditor.cpp:261-269` stehen nach `wechsleFlaecheWennNoetig()` und vor der einzigen Abzweigung (`:271`); der einzige Rücksprung (`:283`) liegt dahinter, `wechsleFlaecheWennNoetig` kehrt nur aus sich selbst zurück. Kein weiterer früher Rücksprung. Sichtbarkeit des Grundes: L-6.

**(c) reset und releaseResources.** Siehe A-78, A-81, L-1, H-1. Der Oversize-Riegel bleibt (`brichAb` fasst `oversizeRiegel` nicht an). Nach `releaseResources` ist jeder Block ein Oversizeblock ohne Klang, die Pegelkopie greift wegen `versuchTrocken.size() == 0` nicht (`PluginProcessor.cpp:916`); `versuchTrocken` hat keinen weiteren Nutzer. Ohne Auftrag: kein Übergang, kein Ereignis, kein Host-Dirty, keine Änderung der Statebytes (M-65).

**(d) Labelentwurf.** Die Kennung entsteht an der Ladestelle (`PluginEditor.cpp:1087-1093`); geschrieben wird nur bei Kennung gleich Aktionsziel und gegen die frische Modellsicht (Hauptziel, Nonce, bestätigt); gleicher Text ist ein No-op (`:1010-1011`). Ein leerer Entwurf schreibt nur auf seine Startquelle; Escape verwirft; Fokusverlust ruft denselben Commit. `substring (0, 120)` und `setInputRestrictions (120)` zählen juce_wchar, also Codepoints; der Prozessor weist `length() > 120` ab (`Ipc.cpp:1208`). Save und Load laufen über den unveränderten Zustandsweg (M-72).

**(e) Ersatzziel und Aktionssteuerung.** Eine Ordnungsfunktion; `aktualisiereName` läuft in allen drei Modellpfaden vor `stelleZielSicher` (`SourcesModel.cpp:420-454`, `:503-532`, `:1336-1382`). Zeilenzahl nachgerechnet: Spalte 310 x 430, nach `reduced (8)` und `removeFromTop (48)` 366 px Höhe; bei 25 Quellen `jlimit (18, 34, 366 / 25 = 14)` = 18 und 366 / 18 = 20 Zeilen; ebenso 20 bei 20, 21, 32, 41 und 64 Quellen, 16 bei 16. Zeichnen (`PluginEditor.cpp:1311`), Klick (`:1148`) und Steuerung (`:1034`) rufen dieselbe `sourcesZeilen()`. Der Abschluss läuft vor dem Leeren des Ziels (`:1046-1048`). 0 Quellen: kein Ersatz, keine Steuerung. Der Status ist wahr, das Hauptziel wechselt nicht von selbst.

**(f) Konfliktrückruf.** SafePointer auf den Editor (`PluginEditor.cpp:621`), Prüfung vor dem ersten Zugriff (`:633-635`), Marke dahinter (`:636-639`). Variante (b) läuft ohne Zugriff auf Editor oder Prozessor (Rückkehr vor jedem Zugriff; die Box gehört dem Modal-Manager). M-92: neue Kennung, ein Host-Dirty, Statusmeldung, die Box schließt. Abbau innerhalb des Rückrufs: L-5.

**(g) Tests.** Gezählt wird nach ausdrücklichem Tick oder Block; das Freilaufsignal der Wanduhr wird vor jedem Tick verworfen (`MarkierungTestMain.cpp:1531-1535`); der Totmann wird relativ zum Millisekundenzähler gesetzt, auch über dessen Umlauf; die Konfliktfälle warten höchstens 5 s auf die Zustellung (obere Schranke, kein Zeitmaß). Die Mutationen der Rotbeweise sitzen an den Zusagezeilen (M-60 und M-86 in den Rohdateien nachgelesen). M-89 vergleicht A mit Vorschau gegen B ohne Vorschau bei Blockgröße 1, 64 und 256 für Dry, Delta und Candidate; M-90 misst ersten und größten Nachbarsprung gegen Fadeschritt plus 2⁻²³ und das Ziel — beide messen, was R-312-20 bis R-312-22 verlangen. Testzugänge: H-2; Ereignis von M-68 (b): H-3; Takt von M-84: L-3.

**(h) Behauptungszeilen.** A3 (`tools/beweise.ps1:509`), B7 (`:728`), B14 (`:751`), B15 (`:752`) Satz für Satz gegen die Prüfnamen gelesen: keine behauptet einen Fall, den ihr Bein nicht fährt. Genauigkeit von „0 Allokationen": H-1; A3 „ein Wechsel weg von Main schliesst das Intervall" beschreibt die gemessene Folge, zum Produkttakt L-3.

**(i) M-77.** `git diff --stat fd287d3c 86c31f46` über `eq-copilot/schemas`, `fixtures`, `identity`, `install`, `plugin/dsp`, `plugin/state`, `plugin/core`, `plugin/sonde`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/CMakeLists.txt`, `design` und `broker` ist leer; geändert sind genau die 13 Pfade. Keine neue Oberfläche, keine neue Stateversion, keine v3-Vertragsänderung, Identität und Legacy-Bezeichner unverändert; im Audiothread nur die Leseordnung. Ohne Auftrag kehrt `verarbeite` vor jedem Schreibzugriff zurück (Nulltest nach Lesen bitidentisch; A1 im Kanon grün).

## Punkt 3 — Produktverhalten in denselben Dateien

Selbst nachgerechnet: die Sortierung in `SourcesModel::sicht()` ist logisch gleich der vorigen Lambda; der Legacy-Tick wirkt in derselben Reihenfolge wie vorher (Netze vor dem Aus-Knopf); Messpunktpanel und Hinweisliste sind unberührt; `prepareToPlay` ändert sich nur in `anLebendes` (A-81) und entscheidet in jeder Lage ohne vorausgehendes end im Ring wie vorher; `verarbeite` (SONDE-013 M-34 bis M-38, Oversize M-36) ist unverändert; `spiegleKlassifikation` bekommt nur die Rücknahme vor dem Store; `releaseResources` berührt weder Pipes noch Clients (Besitzschleuse der Etappe 4 unberührt); die Blockbindung der Etappe 3 liegt in `sonde/` und `dsp/` (Diff leer). Nur gelesen, nicht ausgeführt: alle Beinergebnisse (Kanon GRÜN 70/70 an `86c31f46`, Rotbeweise; M-60 und M-86 in den Rohdateien stichprobenhaft), das Fokus- und Escape-Verhalten des JUCE-TextEditor (A-89, A-91 aus dem Manifest; für die Korrektheit unerheblich, weil ein Fokusverlust nach dem Klick ein No-op ist), der Besitz der CallOutBox, das Brokerverhalten bei einem geschlossenen Main-Link mit offenem begin.

## Matrixzeilen am Zielstand

| Zeile | Ergebnis | Hinweis |
|---|---|---|
| M-55, M-57, M-58 | halten | Speicherordnung nachgerechnet |
| M-56 | hält | Grund in der Main-Fläche nur als Variable: L-6 |
| M-59 | hält | Spalte „Reihenfolge und Frist": L-2 |
| M-60 | hält | Zählart: H-1 |
| M-61, M-63, M-65 | halten | |
| M-62 | Kern hält | Zustand (a): L-2 |
| M-64 | hält | Wortlaut (b): L-2 |
| M-66 bis M-72 | halten | Ereignis von M-68 (b): H-3 |
| M-73 bis M-76, M-85 | halten | Zeilenzahl nachgerechnet |
| M-77 | hält | geschützte Pfade ohne Diff |
| M-84 | hält im gemessenen Ablauf | Produkttakt: L-3 |
| M-86 | hält sequentiell | Nebenläufigkeit: L-1 |
| M-89, M-90, M-40 (erweitert) | halten | |
| M-91, M-92 | halten | Abbau im Rückruf: L-5 |

## Geprüft

Der vollständige Diff der 13 Pfade — Produktcode zeilenweise, die neuen Testfälle vollständig — am Zielstand im Kontext der unveränderten Nachbarn: Sender `Ipc.cpp` (118-440, 880-1034), Ring `InterventionsRing.h`, Lebenslauf `NakamaLebenslauf.h`, `State.cpp` (150-380), die Modellpfade in `SourcesModel.cpp`, `processBlock` (`PluginProcessor.cpp:825-1046`), die Zeichen- und Klickpfade des Editors. Dazu die JUCE-Aufrufwege von `reset`, `releaseResources`, `prepareToPlay` und `process` im VST3-Wrapper samt FL-Sperre und die JUCE-Basis von `reset` und `releaseResources`; die Matrixzeilen, Regeln, Entscheide und Abweichungen der Etappe 6 im Manifest als Kontext; zwei Rotbeweis-Rohdateien; die geschützten Pfade per Diff.

## Nicht geprüft

Kein Bau, kein Testlauf, kein Kanon, kein Laufzeit-Arm (lesender Auftrag); Beinergebnisse und die übrigen Rotbeweise nur aus Manifest und Rohdateinamen. Die Brokerseite (Rust) ist nicht gelesen. FLs Hostverhalten (Threads von setProcessing und setState, Zustellung geposteter Klicks) ist hier nicht messbar. Die JUCE-Interna von TextEditor und CallOutBox sind nicht nachgelesen, nur ihre Wirkung in den Fällen.

HEAD zu Beginn: `87e8af15f6e748f2a14e8c7848084aae949ae60e`. HEAD am Ende: `87e8af15f6e748f2a14e8c7848084aae949ae60e`.

FERTIG Erstprüfung Etappe 6, PASS, 0 Defekte, 6 Lücken, 6 Härtungen
