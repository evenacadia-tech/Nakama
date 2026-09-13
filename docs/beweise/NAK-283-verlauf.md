# NAK-283 — Rundenverlauf §9 bis §44

Append-only. Diese Datei trägt die Abschnitte **§9 bis §44** des Manifests
`docs/beweise/NAK-283.md` **unverändert** (Messungen, Prüfungen, Nacharbeiten,
Einordnungen der Etappen 1 bis 6, dazu der Zwischenkanon `e6771fc9`). Sie sind
ausgelagert, weil der Lesetext des Manifests über 3 000 Zeilen lag; Kopf,
Einordnung (§0, §1), Regeln (§2), Etappenplan (§3), Zusagen (§4), Matrix (§5),
Bauplan (§6), Prüfliste (§7), Feinheiten (§8), der jüngste Kanonlauf und der
Abschluss (§45) bleiben dort, weil `tools/plan/planstand.py` die Kanonbilanz
genau dort liest. Rohausgaben liegen unverändert unter `docs/beweise/roh/`.

Ausgelagert am 2026-09-13 mit dem Ticketabschluss (Urteil T2 PASS).

---
## 9. Messung der Etappe 1 und Start der Matrixprüfung 1 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `c6152784` (`nakama-nak283-3c5ec9d-bau`, Opus max, `--permission-mode dontAsk`), Auftrag `docs/beweise/roh/NAK-283-etappe-1-auftrag.txt`; Start 13:06 Uhr, Ende 13:23 Uhr; Beobachter (`cockpit.ps1 -WatchWorker`, Aufsicht NORMAL) meldete Start, Telemetrie gesund, HEAD-Wechsel, Ende; Stundenloop gelöscht, `CronList` leer. |
| Commit | `2357826839ef0da10f6fcdb226e7bacc8195ff9c` „NAK-283 Etappe 1: Verhaltensmatrix M-01 bis M-70 und Bauplan der Etappen 2 bis 6", auf `origin/master`; Basis `3c5ec9dd` ist Vorfahr; `git status --short` leer. |
| Diff | `git diff --stat 3c5ec9dd..23578268`: nur `docs/beweise/NAK-283.md`, +1 356 / −2. Kein Produkt-, Test-, Schema- oder Werkzeugcode berührt. |
| Rundenbilanz | `3c5ec9dd..23578268: Doku 1 Datei(en) +1356/-2 — OHNE PRODUKTFORTSCHRITT` (Matrixrunde, zählt null; Skill §3.3). |
| Matrix | 70 Zeilen (§5.6): 55 Baulücke, 15 Regressionswache, 13 Zahlenrand, 6 Übergangszeilen, 0 OFFEN; jeder Befund und jede Regel gedeckt (Tabelle §5.6). |
| Quellenkorrekturen §8.2 | Übernommen: die Zusage „nie wieder sichtbar" zu F14 steht in `docs/beweise/SONDE-014.md:743` (M-24), nicht in SONDE-013 — §1.1 ist insoweit zu lesen; F01 hat vier Produktaufrufer von `setzePersistenteMitglieder` plus einen Testpfad. |
| Abweichungen §8.4 | **Beide genehmigt (Dirigent, Technik innerhalb des Tickets)** mit Grenze: Etappe 4 zusätzlich `eq-copilot/plugin/tests/MarkierungTestMain.cpp` (Bein A3) und `eq-copilot/plugin/core/StampedAudioQueue.h`, dort nur die Produzentenseite der Zeitbuchführung für M-35 — Kapazität, Vertrag und Verbraucherverhalten der Queue bleiben; Etappe 6 zusätzlich `docs/beweise/SONDE-013.md` nur als datierter Nachtrag in den Nachweisspalten `:471` und `:623` sowie der Wortlaut für den Nachtrag zu NAK-199, den der Dirigent einträgt. |
| Produktfragen §8.3 | Drei Karten angelegt: **U40** (Advisor auf leisem Material, P-283-3; M-44, M-46, M-47), **U41** (Headroom ohne markierte Passage, P-283-1; M-53 bis M-55), **U42** (Lautheit während der Sperrfrist, P-283-2; M-56 bis M-58). Keine blockiert eine Matrixzeile; gestellt über `/fragen` vor Etappe 5. |
| Matrixprüfung 1 | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only; Auftrag `docs/beweise/roh/NAK-283-matrixpruefung-1-auftrag.txt` (nach dem Muster der Matrixprüfung 1 von NAK-246); Kennung `nak283-matrix1`, abgekoppelt über `tools/dirigent/codex-lauf.ps1`; Prüfbereich `git diff 3c5ec9dd...23578268 -- docs/beweise/NAK-283.md`; Thread-ID und Urteil folgen in §10. |


## 10. Matrixprüfung 1 — NEEDS_WORK (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt über `tools/dirigent/codex-lauf.ps1` (Kennung `nak283-matrix1`); Thread `01a09561-a058-7982-8b9e-2648fa2fd273`; Lauf 13:29 bis 13:57 Uhr, Exit 0. |
| Prüfbereich | `git diff 3c5ec9dd...23578268 -- docs/beweise/NAK-283.md` (§4 bis §8); HEAD `5e68baf2` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. |
| Auftrag und Urteil | `docs/beweise/roh/NAK-283-matrixpruefung-1-auftrag.txt` (Muster NAK-246 Matrixprüfung 1); Urteil wörtlich `docs/beweise/roh/NAK-283-matrixpruefung-1-5e68baf.txt`. |
| Urteil | **NEEDS_WORK — 13 Befunde, alle P2**, noch nicht vom Dirigenten an der Quelle eingeordnet: (1) M-05/§6.3 Frischemarke wird bei inhaltsgleicher Publikation nicht nachgezogen (Rücksprung nach bestätigtem Unbind bleibt möglich); (2) M-01/§6.3 abgewiesene Publikation lässt Host-Dirty und Revision einer bereits angewandten Mutation ungezählt; (3) M-13/§6.4 Frischeprüfung nur bis zum Cache, nicht bis Persistenz und Publikation (Standlock vor der Persistenz frei); (4) M-11 Roundtrip ohne auslösende Änderung an der Obergrenze — nicht rot am Basisstand; (5) M-20 Queue-Begrenzung ist heute messbar (Regressionswache, keine Baulücke), Mutation „zwei Sperrabschnitte" erzeugt kein Wachstum; (6) M-22 Rollback-Fall widerspricht der Retention-Vorbedingung aus M-21 (E0 existiert nicht mehr); (7) M-31/§8.1 Feinheit 4 Mutation `vergissLetztePublikation` ändert keinen Zustand, Bytevergleich fällt nicht; (8) M-34 Reset-Nulltest im ausgeschalteten Pfad ist Regressionswache, kein Beweis für F05; (9) M-45 gewähltes Signal kippt das Gate unter der Mutation nicht (Pegel zwischen −66,02 und −60 dBFS nötig); (10) M-49/M-50/§6.6 `passVon` ist blocklokal, Reset bei `i == passVon` leert an jedem Blockanfang; (11) M-58 Erholung fünf Sekunden nach der Ersetzung erkennt eine zu frühe Freigabe nicht (Fensterrand messen); (12) M-65/§8.1 Feinheit 14 Beschränkung des ±0,1-dB-Budgets auf Sinus steht nicht in der Zusage (SONDE-013 M-02/M-81, Entwurf :2626-2627, :3599-3600); (13) §6.3 `State.cpp` als Änderungspfad der Etappe 2 ohne Freigabe. |
| Sofortentscheid | Befund 13 ist eine Ticketgrenze, keine Sachfrage: `State.cpp` (nur `:347`, Aufruf und Signatur) für Etappe 2 freigegeben — Nachtrag in §3. |
| Validierung | Lesender Opus-Agent (Effort max) an HEAD `5e68baf2`, Ergebnis `docs/beweise/roh/NAK-283-matrixpruefung-1-validierung.md`; Einordnung und Nacharbeitsauftrag in §10.1. |
| Rundenbilanz | Matrixrunde, zählt null (Skill §3.3); kumuliert `39e18d3f..5e68baf2`: nur `docs/`. |


### 10.1 Einordnung der 13 Befunde und Auftrag der Matrix-Nacharbeit 1 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Validierer | Lesender Opus-Agent (Effort max), 14:00 bis 14:10 Uhr an HEAD `5e68baf2` (Code = `ea6ddd57`); Rohdatei `docs/beweise/roh/NAK-283-matrixpruefung-1-validierung.md` (je Befund Quellzitat, Matrixzeile wörtlich, Status, schließende Änderung; gemeinsame Ursachen; Scope-Beweis). Die Zeilenangaben des Urteils liegen an HEAD um +3 höher (Kopfzeile „Gate"). |
| Ergebnis | **13 von 13 BESTÄTIGT, alle DEFEKT** — jede Zeile verletzt eine Zusage aus §4, eine Regel aus §2, den Ist-Stand des Codes oder die Beweisregel (Rotbeweis an der Zusagezeile; Klassifikation aus einer fallenden Messung, nicht aus einem fehlenden Mechanismus). Keine Lücke, keine Härtung. Befund 13 (Ticketgrenze) ist durch den Nachtrag in §3 sofort entschieden. |
| Gemeinsame Ursachen | (a) Klassifikation Baulücke ↔ Regressionswache aus dem fehlenden Mechanismus statt aus einer fallenden Messung (4, 5, 8, 11); (b) Rotbeweise, die eine Nachbargröße statt der zugesagten Messgröße kippen (4, 7, 9, 11); (c) Frischeprüfung nur bis zum Cache statt bis Persistenz und Publikation (1, 2, 3); (d) von der Nachbarzeile geerbte Bühne ohne Neuprüfung ihrer Invarianten (6, 9, 10); (e) Ticketgrenze nicht mitgeführt (13). Die Ursache ist der Auftrag: die Nacharbeit prüft alle 55 Baulücke-Zeilen, alle Rotbeweisspalten und alle geerbten Bühnen, nicht nur die 13 Zeilen. |
| Nacharbeit 1 | Frischer Opus-Worker (max), nur `docs/beweise/NAK-283.md`; Auftrag `docs/beweise/roh/NAK-283-matrix-nacharbeit-1-auftrag.txt` mit den 13 Defekten und je Defekt der schließenden Änderung, den Ursachen (a) bis (e) und der Prüfliste; Rundenbudget 1 von 3. Danach Matrixprüfung 2 (Wiederprüfung, Vorlage B) über den Diff dieser Runde. |

## 11. Matrix-Nacharbeit 1 (Bauer, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Runde | **1 von 3** nach der Matrixprüfung 1 (§10, §10.1). Basis `6c3b5948a1f75f15b48806e111086c67e4dd5db8`, `git status --short` beim Start leer; Produktcode unverändert seit `ea6ddd57`. |
| Ticketgrenze | Genau eine Datei: `docs/beweise/NAK-283.md`. Geändert wurden §4 bis §8 und die Kopfzeile „Etappe"; §0 bis §3, §9 und §10 sind unberührt. Kein Produktcode, kein Test, kein Schema, kein Werkzeug, keine andere Doku. Nicht gebaut, nichts gefahren; Git außer Commit und Push nur lesend. |
| Umfang | 13 Defekte geschlossen; Ursachen (a) bis (e) über **alle** Matrixzeilen geprüft, nicht nur über die 13. **28 Zeilen** berührt: 13 geändert wegen eines Defekts, 3 neu (M-71 bis M-73), 12 wegen einer Ursache oder des Selbstaudits. Keine ID wurde neu vergeben oder verschoben; keine Zeile gestrichen. |
| Zählung | §5.6 neu aus den Zeilen gezählt: **73 Zeilen, 53 Baulücke (44 heute rot, 7 heute nicht messbar, 2 halb/halb), 20 Regressionswache, 13 Zahlenrand, 6 Übergänge, 0 OFFEN.** Vorher: 70 / 55 / 15. |
| Produktfragen | Keine beantwortet. U40 bis U42 bleiben Karten; §8.3 ist unverändert. |

### 11.1 Die dreizehn Defekte

| Nr | Ort | Vorher | Jetzt | Warum das den Befund schließt |
|---|---|---|---|---|
| 1 | **M-05**, §6.3 (`SourcesModel`), §8.1 Feinheit 20 | M-05 war die Zahlenrandzeile („Folgenummer am Anschlag" plus Gleichheitsfall) und sagte: ein inhaltsgleicher Stand meldet Erfolg, **ohne die Folgenummer zu verbrauchen** | M-05 misst das Interleaving `[] → [A] (Join) → [] (Unbind)` mit angehaltener Worker-Kopie; **Nummernvergabe und Bestandsübernahme sind getrennt** — der frühe `return true` bei Gleichheit steht hinter der Übernahme der Nummer. Der Zahlenrand ist nach M-71 gewandert | Der Rücksprung war genau die Lücke: der `[]`-Stand ist inhaltsgleich zum leeren Modell, und eine dort nicht verbrauchte Nummer lässt die ältere Kopie `[A]` mit größerer Nummer danach wieder durch. Jetzt misst eine Zeile diesen Ablauf, und der Bauplan schließt ihn |
| 2 | **M-01**, **M-72** (neu), **M-04**, §6.3 (`Ipc.cpp`), §8.1 Feinheit 19 | §6.3 nannte nur die Folgenummern; der Ablehnungszweig `Ipc.cpp:1476-1481` blieb einer, und M-01 prüfte nur den Modellbestand | `setzePersistenteMitglieder` bekommt drei Ausgänge (übernommen · überholt · Reload abgewiesen); der Überholtfall meldet **weiterhin** Host-Dirty und Revision und zählt in einen eigenen Zähler. M-01 prüft Dirty-Zähler und Revisionsdelta mit, M-72 prüft die Unterscheidbarkeit beider Gründe, M-04 hält den Reloadfall (kein Dirty) fest | Eine angewandte persistente Änderung, deren Darstellung nur überholt wurde, bliebe sonst ungezählt — gegen `nakama-state-v2.md:139`, `CLAUDE.md` „jede persistente Änderung meldet dem Host Dirty-State" und den bestehenden Nachweis Dirty == 2 / Revisionsdelta 2 (`Sonde012ProjectReloadTest.cpp:1021-1026`) |
| 3 | **M-13**, §6.4 (`hypothese_verdrahtung.rs`), §8.1 Feinheit 18 | Der Riegel saß nur vor dem Cache-Eintrag; M-13 sagte an `stand.befunde` zu | M-13 trägt **zwei Fenster**: vor dem Cache-Eintrag und — neu — zwischen Cache-Eintrag und `befund_persistieren`. Dort vergleicht der Persister unter dem Standlock, ob sein Payload noch der Standeintrag ist; sonst schreibt er nicht. Gemessen wird an `event_log` **und** Projektion | R-283-1 und das Audit-Korrekturziel (`UEBERGABE-AN-CLAUDE.md:88`) reichen bis Persistenz und Publikation. Die zwei naheliegenden Alternativen sind an der Quelle geprüft und fallen aus: Standlock über die Persistenz = Deadlock (`:511-514`), Ordnungsmarke im Event erreicht die Projektion nicht (`store/writer.rs:476`) |
| 4 | **M-11** | Vorbedingung `int64max`, danach nur Speichern und Laden — der Wert lädt heute fehlerfrei (`NakamaState.cpp:1905-1909`), der Fall war grün | Vor dem Speichern werden **zwei inkrementierende Handgriffe an der Obergrenze tatsächlich gerufen**: `setzeAssistentenschritt` (`:2667`; die drei Inkrementstellen `:2662`, `:2707`, `:2810` haben heute keine obere Schranke) und `entferneIntent` (`:2275-2291`, heute Löschen vor `bestandsrevisionHeben`); geprüft wird der **geladene** Zustand einer neuen Instanz | Erst der ausgelöste Handgriff erzeugt am Basisstand den negativen Wert beziehungsweise die geänderten Bytes. Beide Hälften sind damit heute rot, und die Zeile trägt das persistente F11-Gegenbeispiel |
| 5 | **M-20**, §6.4 (`queues.rs`), §6.1 | Als Baulücke geführt („am Basis-SHA nicht messbar"), Atomaritätsmutation „zwei Sperrabschnitte" seriell | **Regressionswache (heute grün)** — `std::mem::replace` an alter Position (`:223-229`) hält die Deque schon heute bei drei Einträgen. Die Atomaritätszusage bekommt einen eigenen Bruch: `cfg(test)`-Haken zwischen Entfernen und Anhängen plus **erzwungenes konkurrierendes Einreihen** desselben Schlüssels | Die Klasse folgt jetzt aus einer Messung, nicht aus dem fehlenden Anhängen; und die zweite Zusage hat einen Bruch, der sie wirklich kippt — serielle Ersetzungen in zwei Sperrabschnitten wachsen nicht |
| 6 | **M-22** | „Wie M-21" — in dieser Bühne entfernt `pop_front()` gerade E0 (`evidenz.rs:186-189`), die Aussage „E0 trägt danach `ausschlussgrund = None`" ist dort nicht herstellbar | **Eigene Bühne ohne Retention-Abbau**: zehn Belege unter dem Deckel, **zwei** markierte Belege E2 und E5, kein zweiter Thread. Klasse: **Regressionswache**, Bruch = Rollback, der nach dem ersten Treffer abbricht | Die Bühne stellt den eigenen Fall jetzt her, und der von der Retention entfernte Beleg bleibt ausschließlich Gegenstand von M-23. Heute grün ist sie, weil `invalidierung_ruecknehmen` über **alle** `(key, index)`-Paare läuft (`:237-243`) und die Indizes ohne Retention-Abbau noch treffen |
| 7 | **M-31**, §8.1 Feinheit 4 | Mutation `vergissLetztePublikation()`; Feinheit 4 begründete das damit, der Aufruf „würde den bestätigten Zustand verwerfen" | Die technische Aussage ist **gestrichen** (der Aufruf setzt nur `zuletztGueltig = false`, `NakamaTransaktion.h:352`) und durch den tragenden Grund ersetzt. M-31 ist **Regressionswache**, ihr Bruch ist der neutrale Ladestart `transaktion->ladestart (…)` (`SondeProcessor.cpp:866`) | Der Bruch verwirft jetzt tatsächlich Zustand: `getStateInformation` liefert andere Bytes, die Bank ist nicht mehr engagiert, der nächste Block trägt keine Filterwirkung — alle drei zugesagten Messgrößen kippen. Die falsche Begründung steht nicht mehr im Manifest |
| 8 | **M-34** | Als Baulücke geführt, Rotbeweis „`reset()` den Ausgangspuffer nullen" | **Regressionswache (heute grün)** — keine `reset`-Überschreibung (`SondeProcessor.h:130-138`), der leere JUCE-Basispfad ändert nichts; ausdrücklich vermerkt: **kein Beleg für F05**, der liegt bei M-30 bis M-33 | Die Zeile behauptet nicht mehr, den aktiven Filterzustand zu belegen; sie schützt nur noch das Grundgesetz, und das sagt sie selbst |
| 9 | **M-45** | „Dasselbe Material" (−55 dBFS aus M-44); die Mutation `kHauptPunkte` hob die Bassenergie um 6,02 dB, ließ beide Stufen aber aktiv — die zugesagte Größe blieb erfüllt | **Zwei eigene Pegel bei 48 kHz**: L1 = −52,3 dBFS (am Basisstand entscheiden die Stufen verschieden: −56,97 gegen −62,99 dB) und L2 = −63,0 dBFS im Fenster −66,02 bis −60 (die Mutation hebt die Bassstufe auf −56,98 dB über das Gate, die Hauptstufe bleibt bei −63,00) | Jetzt kippt je Pegel genau die zugesagte Messgröße „gleiche Gate-Entscheidung beider Stufen" — L1 am Basisstand, L2 unter der Mutation. Die Pegel sind aus den gemessenen Konstanten gerechnet, nicht geschätzt |
| 10 | **M-49**, **M-50**, **M-73** (neu), §6.6, §8.1 Feinheit 17 | Reset „an der Grenze `i == passVon`" — der Index ist blocklokal (`Spektrum.h:75-79`) und in jedem späteren Block erneut 0 | Der Reset hängt an einem **einmaligen Startmerker**, dem Gegenstück zu `fensterEndetHier`: scharf beim Binden, gültig nur wenn `von == passagenfenster.startSample`, feuernd genau einmal je Passage. M-50 legt den Passagenanfang ins Blockinnere, M-73 misst die Blockgrößenunabhängigkeit (64 · 512 · 1024) | Der echte Beginn ist von späteren Blockanfängen unterschieden, und die falsche Fixfassung („Reset an jedem Blockanfang") hat mit M-73 eine Wache, die sie fängt — sie macht das Passagenmaximum blockgrößenabhängig, also genau den Fehler, den F07 beheben soll |
| 11 | **M-58** | Eine Messung 5 s nach dem Ende der Ersetzung; dann sind alle 30 Zellen ohnehin sauber und eine zu frühe Freigabe unsichtbar | **Vier Messpunkte am Fensterrand**: 0,3 s / 0,4 s (Momentary) und 2,9 s / 3,0 s (Short-term), gerechnet aus `kZelleSekunden = 0,1`, `kMomentZellen = 4`, `kKurzZellen = 30`. Je Richtung ein eigener Bruch (zu spät · zu früh) | Der Punkt 2,9 s ist am Basisstand rot, weil die Verriegelung heute mit dem Rahmen fällt; damit trägt die Zeile die Behauptung „ROT am Basis-SHA" wieder zu Recht, und beide Fehlrichtungen sind messbar |
| 12 | **M-65**, **M-66**, §6.7, §8.1 Feinheit 14 | Feinheit 14 beschränkte das ±0,1-dB-Budget auf analytische Sinus und setzte für 20 bis 23 die Normtoleranz an ihre Stelle | Die Beschränkung ist **gestrichen**: ±0,1 dB gegen eine validierte Referenz gilt für **alle** Fälle 15 bis 23; die Normtoleranz +0,2/−0,4 dBTP kommt als Mindestanforderung hinzu. Die Referenz für 20 bis 23 wird im Test gerechnet (64-fach-Überabtastung) und vorher an 15 bis 19 auf ±0,01 dB nachgewiesen | Die Zusage (SONDE-013 M-02/M-81, Entwurf `:2626-2627`) bindet das Budget an „Standard-Testmaterial … validierte Referenz", nicht an analytische Signale; eine Budgetänderung wäre nur nach Entwurf `:3599-3600` mit Messung, Evidenz und Userwirkung zulässig, und Etappe 1 misst nichts |
| 13 | **§8.4 Nummer 3**, §6.3 Ticketpfade | `State.cpp` stand als Änderungspfad in §6.3, ohne in §3 oder §8.4 geführt zu sein | §8.4 führt ihn als **dritte, datiert genehmigte Abweichung** (12.09.2026, §3 Nachtrag, §10 Sofortentscheid) mit der Grenze „nur `:347`, Aufruf und Signatur"; §6.3 nennt dieselbe Grenze wörtlich | Der Plan überschreitet die gebundene Ticketgrenze nicht mehr: die Erweiterung ist genannt, begrenzt und datiert genehmigt — der Defekt war die fehlende Führung, nicht die Änderung |

### 11.2 Die fünf Ursachen, über die ganze Matrix geprüft

**(a) Klassifikation Baulücke ↔ Regressionswache — alle 55 Baulücke-Zeilen
geprüft.** Maßstab: fällt die Messung am Code des Basisstands, oder fehlt nur
der Mechanismus? Ergebnis: **vier Zeilen umklassifiziert** (M-20, M-22, M-31,
M-34 — je weil die zugesagte Größe heute stimmt), **sieben Zeilen** tragen
jetzt ausdrücklich „am Basis-SHA nicht messbar" statt „ROT" (M-16, M-63, M-65,
M-66, M-68 und die beiden neuen M-71, M-72), **zwei Zeilen** sind halb rot,
halb nicht messbar und sagen es (M-01, M-13). Zusätzlich geändert, nicht durch
einen Defekt ausgelöst: **M-63**, **M-66**, **M-68** (Wortlaut), **M-67**
(fehlende Basisaussage ergänzt — die Statusspalten in SONDE-013 sind heute
tatsächlich falsch, die Zeile ist rot) und **M-26** (trug als einzige
Übergangszeile **gar keine** Basisaussage; sie ist über beide Sprachen rot,
weil in C++ die Folgenummer und in Rust der Mengenvergleich fehlt). §5.6 ist
aus den Zeilen neu gezählt
und trennt jetzt „heute rot" von „heute nicht messbar". **Je Befund bleibt
mindestens eine heute rote Zeile** — die Umklassifizierungen haben keinen
Befund ohne Beleg gelassen (§5.6 Schlussabsatz). *Zusätzlich geändert: 5
Zeilen.*

**(b) Rotbeweise, die den Fixfall nicht kippen — alle Rotbeweisspalten
geprüft.** Maßstab: kippt die Mutation genau die zugesagte Messgröße dieser
Zeile? Über die Defekte 4, 7, 9, 11 hinaus fielen drei Zeilen auf, deren
Zahlen die Größe nicht kippten oder nicht zur Quelle passten: **M-44** (die
Behauptung „44,1/48/96/192 kHz melden `abdeckung = 0`, 8 kHz meldet 1" ist an
der Quelle falsch — bei 44,1 und 48 kHz nimmt die Bassstufe das Gate; jetzt
stehen die acht gerechneten Stufenwerte), **M-46** (Pegel −58,0 und −62,0 dBFS
statt „knapp über und knapp unter"; die archivierte Beobachtung
`activity=1 coverage=0` ist damit reproduzierbar begründet) und **M-47**
(Pegel −58,0 dBFS bei 96 kHz mit den zwei Stufenwerten). *Zusätzlich geändert:
3 Zeilen.*

**(c) Reichweite bis Persistenz und Publikation — alle Zeilen der Etappen 2
und 3 geprüft.** Über die Defekte 1, 2, 3 hinaus: **M-04** nennt jetzt, dass
der Reloadfall Dirty und Revision unterdrückt und den Überholt-Zähler nicht
bewegt; **M-16** deckt **beide** Verwurfzweige (Cache und Persistenz) statt
einen. Neu entstanden ist **M-72** (Unterscheidbarkeit der Ablehnungsgründe).
Die übrigen Zeilen der Etappen 2 und 3 reichen bereits bis zu ihrer Zusage:
M-06 misst nach Ruhe State **und** Modell, M-11 misst am geladenen Zustand,
M-24 misst Cache **und** Projektion, M-21 und M-23 messen am Stand nach dem
Rollback. *Zusätzlich geändert: 2 Zeilen.*

**(d) Geerbte Bühnen — jede Zeile mit „Wie M-nn" oder „Dasselbe Material"
geprüft.** Geerbt wird in M-02, M-08, M-09, M-24, M-28, M-29, M-31, M-36,
M-38, M-54, M-57, M-62 (und bis zu dieser Runde in M-22 und M-45). Geprüft
wurde je Zeile, ob die geerbte Bühne den **eigenen** Fall herstellt. Neben
M-22 (Defekt 6) und M-45 (Defekt 9) fiel **M-57** durch: es erbte von M-56 das
Ereignis „der erste saubere Rahmen", in dem auch die Momentary-Frist noch
nicht abgelaufen ist — die zugesagte eigene Frist wäre dort unsichtbar. M-57
hat jetzt ein eigenes Ereignis (Rahmen bei 0,4 s und 2,9 s). Die übrigen zehn
erben nur Instanz oder Block und stellen ihren Fall her; M-08 und M-09 sind an
der Quelle nachgemessen (beide löschen vor `bestandsrevisionHeben`,
`NakamaState.cpp:2340-2346`, `:2455-2461`). *Zusätzlich geändert: 1 Zeile.*

**(e) Ticketgrenze mitgeführt.** §6.3 nennt die Grenze aus dem §3-Nachtrag
wörtlich, §8.4 führt sie als dritte datierte Abweichung. Gegenprobe über die
übrigen Bauetappen: §6.5 und §6.7 nennen ihre Abweichungen bereits und decken
sich mit §8.4 Nummer 1 und 2; §6.4 nennt neu ausdrücklich, dass
`broker/src/store/**` **nicht** angefasst wird — der Riegel sitzt im
Coordinator. Kein weiterer Pfad in §6 liegt außerhalb von §3 samt Nachtrag und
§8.4. *Zusätzlich geändert: 0 Zeilen (nur Bauplan- und Abweichungstext).*

### 11.3 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Zeile | Für diese Runde |
|---|---|
| **A** Rückstau und Prioritätsklassen | **Nicht zutreffend** — diese Runde baut keinen Code. Wo die Zeile in den Bauetappen fällt, steht unverändert in §7; neu hinzugekommen: M-20 misst die Atomarität der Ersetzung jetzt mit erzwungenem konkurrierendem Einreihen, M-72 die Trennung der zwei Ablehnungszähler. |
| **B** Lebenszyklus | **Nicht zutreffend.** Unverändert §7; die Beziehung `prepareToPlay`↔`releaseResources`↔`reset` bleibt bei M-32, und M-31 ist von der Baulücke zur Wache dieses Trios geworden. |
| **C** Verträge und Längen | **Nicht zutreffend.** Kein Feld reist neu über den Draht; M-11 misst den int64-Rand jetzt **mit** ausgelöstem Handgriff beidseitig (Writer und Reader). |
| **D** Bau- und Prüfriegel | **Nicht zutreffend.** Kein Bau, kein Bein, keine Beglaubigung in dieser Runde. |
| **E** Behauptung ≤ Messung | **Gilt hier.** Das Manifest behauptet weiterhin kein gemessenes Produktverhalten: Kopf „Urteil offen", Etappenzeile „Matrixprüfung 2 steht aus". Jede neue Zahl trägt Datei und Zeile oder ist aus benannten Konstanten gerechnet (§5 „Gerechnete Zahlen"). Zwei Behauptungen wurden **zurückgenommen**, weil sie mehr sagten als die Quelle trägt: die technische Begründung in Feinheit 4 und die Budgetbeschränkung in Feinheit 14. Die Klassenzählung in §5.6 ist aus den Zeilen gezählt, nicht fortgeschrieben. |
| **F** Änderungssatz | **Nicht zutreffend für Code.** Für das Manifest gilt: Zeile, Bauplanabsatz, Feinheit und Zählung sind je Defekt **gemeinsam** nachgezogen — keine Zeile ohne ihren Bauplanabsatz, kein Bauplanabsatz ohne seine Zeile. |

**Eigene Messungen dieser Runde** — alles andere ist gelesen, nicht gemessen:

- `git rev-parse HEAD` = `6c3b5948a1f75f15b48806e111086c67e4dd5db8`,
  `git status --short` leer.
- `SourcesModel.cpp:360-372`: früher `return true` bei Gleichheit **vor** jeder
  Zustandsfortschreibung; `Ipc.cpp:1461-1485`: ein Ablehnungszweig, Ausstieg vor
  `meldeHostDirty()`; `State.cpp:341-353`: vierter Produktaufrufer mit
  `else`-Zweig.
- `NakamaState.cpp:2207-2216`: `bestandsrevisionHeben` hat die obere Schranke
  **bereits**; `:2283-2288`, `:2340-2346`, `:2455-2461`: alle drei Entfern-
  Handgriffe löschen **vor** dieser Prüfung; die drei Inkrementstellen der
  Assistentenrevision (`:2662` in `schrittAendern`, `:2707`, `:2810`) haben
  **keine** obere Schranke.
- `hypothese_verdrahtung.rs:465-478`: der Standlock fällt vor
  `befund_persistieren`; `:494-515`: der Persister zieht seine `sequence` erst
  beim Schreiben und nimmt im Fehlerpfad den Standlock selbst (`:511-514`);
  `store/writer.rs:476`: `event_ord = tx.last_insert_rowid()`, gebunden in
  `:589-601` — **die Projektion ordnet nach Anhängereihenfolge, nicht nach
  Event-`sequence`**.
- `invalidierung_verdrahtung.rs:235-243`: der Rollback läuft über **alle**
  `(key, index)`-Paare; `queues.rs:220-238`: Ersetzung per `std::mem::replace`
  an alter Position, eine Operation unter einer Sperre.
- `NakamaTransaktion.h:352`: `vergissLetztePublikation()` setzt nur
  `zuletztGueltig = false`; `:226`: `schreibeIn` ist der Weg in den
  persistierten Stand; `SondeProcessor.cpp:820-827`, `:866`.
- `FeatureEngine.h:233-247`, `:249-259`, `:606-607`, `:611-622`;
  `featureengine/Spektrum.h:55-86` (`passVon` blocklokal, `fensterEndetHier`
  als Vorbild), `:107-113`, `:355-362`, `:383`;
  `featureengine/Lautheit.h:30-44`, `:70-72`, `:90-102`, `:109-119`;
  `featureengine/Frame.h:484-491` (`abdeckung` aus `evidenzFensterAktiv` durch
  `evidenzFensterGesamt`).
- Daraus gerechnet, nicht abgeschrieben: `10·log10 (fs / punkte)` = 4,30 /
  4,67 / 7,68 / 10,69 dB (Bassstufe bei 44,1 / 48 / 96 / 192 kHz) und 10,32 /
  10,69 / 13,70 / 16,71 dB (Hauptstufe); Differenz 6,02 dB. Daraus die Pegel
  −52,3 und −63,0 dBFS (M-45), −58,0 und −62,0 dBFS (M-46, M-47).
  Zellenfristen: 4 × 0,1 s = **0,4 s**, 30 × 0,1 s = **3,0 s**, Messpunkte
  eine Zelle davor (0,3 s, 2,9 s). Blockzahlen bei 48 kHz: 750 Blöcke je
  Sekunde bei 64 Samples, rund 47 bei 1024 (M-73).
- Klassen und Summen in §5.6 aus den 73 Zeilen ausgezählt: 53 + 20 = 73;
  44 + 7 + 2 = 53.

### 11.4 Was diese Runde nicht geändert hat

- **Dirigententext.** Die Kopfzeile „Gate" nennt weiterhin „M-01 bis M-70",
  §9 nennt „70 Zeilen … 55 Baulücke, 15 Regressionswache", §10 zitiert das
  Urteil. §0 bis §3, §9 und §10 gehören dem Dirigenten; nur die Zeile „Etappe"
  ist nachgezogen. Der Abgleich dieser drei Stellen auf 73 / 53 / 20 steht dem
  Dirigenten zu.
- **Produktfragen.** U40 bis U42 sind Karten und bleiben unbeantwortet; §8.3
  ist unverändert. Die neuen Pegel in M-45 bis M-47 verschieben keine
  Produktentscheidung — sie messen dieselbe Zusage an einem Punkt, an dem sie
  kippt.
- **Keine Zeile gestrichen.** Alle 70 IDs der Etappe 1 stehen weiter; drei
  sind hinzugekommen (M-71 bis M-73). Kein Rotbeweis wurde entfernt, ohne
  einen tragenden an seine Stelle zu setzen.
- **Nichts gebaut und nichts gefahren.** Kein Test, kein Bein, kein Repro,
  kein Skript; `planstand.py` und `docs/PLAN-STAND.md` unberührt. Was die
  Rotbeweise tatsächlich messen, entsteht erst in den Bauetappen.


## 12. Messung der Matrix-Nacharbeit 1 und Start der Matrixprüfung 2 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `31c9ebd4` (`nakama-nak283-6c3b594-matrix-nach1`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-matrix-nacharbeit-1-auftrag.txt`; 14:18 bis 14:56 Uhr; Beobachter (Aufsicht NORMAL) meldete Start, Telemetrie gesund, einmal „Statusquelle unbekannt" (Fehlalarm unter Last), HEAD-Wechsel, dann „blockiert" = leerer Prompt nach Abschluss; Worker gestoppt, Loop gelöscht, `CronList` leer. |
| Commit | `57aeae68c799dde633662728c286a825271fc524` „NAK-283 Matrix-Nacharbeit 1: 13 Defekte der Matrixprüfung 1 geschlossen, Ursachen a bis e über die Matrix geprüft", auf `origin/master`; Basis `6c3b5948` ist Vorfahr; `git status --short` leer. |
| Diff | `git diff --stat 6c3b5948..57aeae68`: nur `docs/beweise/NAK-283.md`, +512 / −112 (§5, §6, §8, neuer §11). Kein Code berührt. |
| Rundenbilanz | `6c3b5948..57aeae68: Doku 1 Datei(en) +512/-112 — OHNE PRODUKTFORTSCHRITT` (Matrixrunde, zählt null); kumuliert `39e18d3f..57aeae68`: nur `docs/`. |
| Selbstbericht des Bauers (§11, nicht als Beweis gezählt) | 13 von 13 geschlossen; 28 Zeilen berührt, 3 neu (M-71 bis M-73), keine gestrichen; §5.6 neu gezählt: 73 Zeilen — 53 Baulücke (45 heute rot, 7 nicht messbar, 1 gemischt), 20 Regressionswache, 13 Zahlenrand, 6 Übergänge, 0 OFFEN. Zwei technische Abweichungen zu Befund 3, je mit Quelle: Persistenz nicht unter den Standlock (Deadlock im Fehlerpfad, `hypothese_verdrahtung.rs:511-514`), keine Ordnungsmarke im Payload (`store/writer.rs:476`); stattdessen Wiedervergleich der Eingangsmenge unter dem Standlock unmittelbar vor `store.append`, `broker/src/store/**` unberührt. |
| Dirigent | Die Ticketpfade der Etappe 3 (§3) nennen `broker/src/store/` nicht; ob der Wiedervergleich vor `store.append` R-283-1 („bis zur Persistenz") genügt, entscheidet die Wiederprüfung an der Quelle. Genügt er nicht, entscheidet der Dirigent den Pfad (Store) vor Nacharbeit 2. Kopfzeilen „Gate" (M-01 bis M-73) und „Etappe" nachgezogen; §9 und §10 bleiben als Verlauf mit den Zahlen ihres Standes. |
| Matrixprüfung 2 | Wiederprüfung (Vorlage B): Codex `gpt-6-astra`, Effort **max**, lesend, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-matrix2`); Prüfbereich `git diff 6c3b5948...57aeae68 -- docs/beweise/NAK-283.md`, Befundliste = die 13 Befunde der Matrixprüfung 1; Auftrag `docs/beweise/roh/NAK-283-matrixpruefung-2-auftrag.txt`; Thread-ID und Urteil folgen in §13. |


## 13. Matrixprüfung 2 (Wiederprüfung) — NEEDS_WORK (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-matrix2`); Thread `01a095b4-a787-7c52-9ca2-c4a085b9299a`; Lauf 15:00 bis 15:15 Uhr, Exit 0. |
| Prüfbereich | Wiederprüfung (Vorlage B): `git diff 6c3b5948...57aeae68 -- docs/beweise/NAK-283.md` mit der Befundliste der Matrixprüfung 1; HEAD `2d30b41c` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. |
| Auftrag und Urteil | `docs/beweise/roh/NAK-283-matrixpruefung-2-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/NAK-283-matrixpruefung-2-2d30b41.txt`. |
| Urteil | **NEEDS_WORK — 11 von 13 Befunden geschlossen; Befunde 3 und 10 offen; drei neue Brüche** (alle P2): (1) Befund 3 offen — M-13/§6.4: A kann den Wiedervergleich bestehen, nach der Sperrfreigabe anhalten, B persistiert, dann schreibt A; bei gleicher `finding_id` gewinnt A durch die höhere Append-Ordnungszahl (`store/writer.rs:476`, `:589-593`); Vergleich und geordnete Store-Annahme müssen zusammengehören, eine Store-Änderung ist dafür nicht zwingend (`StoreHandle::append_einreihen`, `handle.rs:153-170`, trennt Annahme und Commit-Warten); (2) M-44: bei −55 dBFS ist `abdeckung` am Basisstand bei allen vier Raten 0, weil `Spektrum.h:369-383` nur die Hauptstufe zählt — die Bühne für R-283-4 muss die gezählte Hauptstufenentscheidung zwischen den Raten kippen; (3) M-73 verlangt zusätzlich Übereinstimmung mit der isolierten Passage, die der Basisstand verletzt (F07-Gegenbeispiel) — heute rot, keine Regressionswache; §5.6 (20 Regressionswachen) falsch; (4) Befund 10 offen — M-73: der 12-kHz-Ton (Periode vier Samples) ist gegen Blockgrenzen 64/512/1024 phasenneutral, die Mutation ändert das Passagenmaximum nicht (`TruePeak.h:75-78`, `:172-179`; `Spektrum.h:197-200`); ein blockgrenzensensitiver Verlauf ist nötig; (5) Feinheit 14 legt 64-fache Überabtastung und ±0,01 dB Referenzvalidierung ohne Quelle fest — verletzt „keine Zahlen ohne Quelle". |
| Rundenbudget | Nacharbeit 1 verbraucht; **Nacharbeit 2 = Runde 2 von 3**. |
| Validierung | Lesender Opus-Agent (Effort max) an HEAD `2d30b41c`, Ergebnis `docs/beweise/roh/NAK-283-matrixpruefung-2-validierung.md`; Einordnung, Regel zu Befund 3 und Nacharbeitsauftrag in §13.1. |


### 13.1 Einordnung der fünf Punkte, Feinheit zu R-283-1 und Auftrag der Matrix-Nacharbeit 2 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Validierer | Lesender Opus-Agent (Effort max), 15:18 bis 15:26 Uhr an HEAD `2d30b41c`/`cc957037` (Code = `ea6ddd57`); Rohdatei `docs/beweise/roh/NAK-283-matrixpruefung-2-validierung.md`. |
| Ergebnis | Punkte 1 bis 4 **BESTÄTIGT, DEFEKT** (R-283-1 verletzt; Code-Ist-Stand `Spektrum.h:369-383` zählt nur die Hauptstufe; M-73 trägt zwei Zusagen, deren zweite am Basisstand rot ist; der 12-kHz-Dauerton ist gegen die Blockraster phasenneutral). Punkt 5 **TEILWEISE**: die Zahlen (64-fache Überabtastung, ±0,01 dB) sind haltbar — Herleitung aus `TruePeak.h:215-220` (Restfehler −0,00065 dB bei fs/4) und Präzedenz NAK-180 B17 —, aber unbelegt notiert; Einordnung **DEFEKT gegen die Auftragsregel „jede Zahl trägt Datei:Zeile"**, schließende Änderung ist die Herleitung, kein Rückbau. |
| Feinheit zu R-283-1 (Dirigent) | **Frischevergleich und Store-Annahme unter demselben Standlock.** Der Vergleich der Eingangsmenge und `StoreHandle::append_einreihen` (Annahme mit Ordnungszahl, `broker/src/store/handle.rs:153-170`) laufen unter demselben Standlock; die Sperre wird erst vor dem Warten auf den Antwortkanal freigegeben; Annahmefehler werden am gehaltenen Guard gezählt; Commit-Warten und Fehlerbehandlung liegen außerhalb der Sperre. Kein Eingriff unter `broker/src/store/**`; die Ticketpfade der Etappe 3 bleiben. Wer später einreiht, gewinnt — damit ist das Fenster aus Befund 3 geschlossen, nicht verkleinert. |
| Gemeinsame Ursache | Runde 1 argumentierte aus dem zugesagten Mechanismus statt aus einer Messung am Basiscode (Annahmekante, Zählbedingung `&s == &haupt`, Reset am Bindezeitpunkt, Periode gegen Blockraster). Regel für Runde 2: jede Zahl, Messgröße und Sperrenaussage einer Zeile wird am Basiscode mit Datei:Zeile belegt; Zeilen mit zwei Zusagen werden je Zusage klassifiziert. |
| Nacharbeit 2 | Frischer Opus-Worker (max), nur `docs/beweise/NAK-283.md`; Auftrag `docs/beweise/roh/NAK-283-matrix-nacharbeit-2-auftrag.txt` (fünf Punkte mit schließender Änderung, Feinheit R-283-1, Belegregel über Etappen 3 und 5 und alle Runde-1-Zeilen); Runde 2 von 3. Danach Matrixprüfung 3 (Wiederprüfung 2) über den Diff dieser Runde mit der Liste dieser fünf Punkte; endet sie ohne PASS, folgt der Konvergenzentscheid nach Skill §3.4. |


## 14. Matrix-Nacharbeit 2 (Bauer, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Runde | **2 von 3** nach der Matrixprüfung 2 (§13, §13.1). Basis `168f2319c5634445b322517ad45a6d7598d14fc8`, `git status --short` beim Start leer; Produktcode unverändert seit `ea6ddd57`. |
| Ticketgrenze | Genau eine Datei: `docs/beweise/NAK-283.md`. Geändert wurden §5, §6, §8, die Kopfzeile „Etappe" und dieser neue §14; §0 bis §4, §7 und §9 bis §13 sind unberührt. Kein Produktcode, kein Test, kein Schema, kein Werkzeug, keine andere Doku. Nicht gebaut, nichts gefahren; Git außer Commit und Push nur lesend. |
| Umfang | Fünf von fünf Punkten geschlossen. **Neun Zeilen** berührt: fünf wegen eines Punktes (M-13, M-44, M-65, M-73 und die neue M-74), drei zusätzlich wegen der Belegregel (M-19, M-24, M-45) und eine als Folge der Teilung (M-49); in M-13 kam zur Kopplung eine Belegkorrektur hinzu. Eine neue ID (**M-74**), keine gestrichen, keine verschoben. Dazu §5 Vorspann, §5.6, §6.4, §6.6, §6.7, §8.1 Feinheit 14 und Feinheit 18. |
| Zählung | §5.6 neu aus den Zeilen gezählt: **74 Zeilen, 54 Baulücke (46 heute rot, 7 heute nicht messbar, 1 gemischt), 20 Regressionswache, 13 Zahlenrand, 6 Übergänge, 0 OFFEN.** Vorher: 73 / 53 / 20. |
| Produktfragen | Keine beantwortet. U40 bis U42 bleiben Karten; §8.3 ist unverändert. |

### 14.1 Die fünf Punkte

| Nr | Ort | Vorher | Jetzt | Warum das den Punkt schließt |
|---|---|---|---|---|
| 1 | **M-13** (Fenster 2), §6.4 (`hypothese_verdrahtung.rs`, Reihenfolge, Risiken, „Nicht angefasst"), §8.1 Feinheit 18 | Der Wiedervergleich saß „unmittelbar vor `store.append`" und gab den Standlock **vor** dem Append wieder frei — begründet mit einem Deadlock über `:511-514` | **Vergleich und Store-Annahme laufen unter derselben Sperre:** Standlock nehmen → Payload gegen den Standeintrag vergleichen → `StoreHandle::append_einreihen` (`broker/src/store/handle.rs:156-171`) am gehaltenen Guard → Sperre freigeben → `recv` und Fehlerbehandlung außerhalb; ein Annahmefehler zählt am Guard. Der Rotbeweis misst jetzt die Kopplung selbst: Sperre zwischen Vergleich und Annahme freigeben, B in der Lücke laufen lassen → A reiht nach B ein und gewinnt die Projektion | Das Fenster der Matrixprüfung 2 ist geschlossen, nicht verkleinert: die **Annahme** vergibt die Ordnungszahl (ein Writer-Thread über einen FIFO, `store/writer.rs:176-193`, `:298`, `:325-334`, `:476`, `:588-592`), also gewinnt, wer später einreiht — unter der gemeinsamen Sperre der, der den Vergleich später besteht. Kein Deadlock: die Annahme blockiert nie (`SyncSender::try_send`, `handle.rs:232`, `:239-242`; `STORE_KANAL_CAP = 256`, `store/mod.rs:48`), und der Lockpfad `:511-514` liegt hinter dem `recv`. Kein Eingriff unter `broker/src/store/**` — `append_einreihen` ist schon `pub` und wird schon von außen gerufen (`broker/tests/store_crash_matrix.rs:1527`, `:1532`, `:1676`, `:1705`, `:1712`) |
| 2 | **M-44**, §5 Vorspann | „`abdeckung` zählt die Fenster **beider** Stufen", Material −55 dBFS, und die Zeile sagte zusätzlich eine gleiche „Aktivitätsentscheidung" zu | „Beide Stufen" ist **gestrichen**: `abdeckung` zählt ausschließlich die Hauptstufe (`Spektrum.h:369-373` `if (&s == &haupt)`, Zähler `:382-383`, Quotient `Frame.h:486-490`). Der Pegel ist auf **−48,0 dBFS** gerechnet, sodass die gezählte Hauptstufenentscheidung unter der Mutation **1 / 1 / 0 / 0** liefert. Die Aktivitätszusage ist gestrichen (§14.2) | Der Rotbeweis fällt jetzt an der Größe, die die Zeile zusagt. Herleitung des Pegels aus den Konstanten: `−60 + 10·log10 (48000/4096) < L < −60 + 10·log10 (96000/4096)`, also −49,31 dBFS < L < −46,30 dBFS mit `kHauptPunkte = 4096` (`FeatureEngine.h:235`) und `kAktivGateDb = −60,0` (`:247`); −48,0 dBFS liegt 1,31 dB über der unteren und 1,70 dB unter der oberen Kante. Am Fixstand ist der integrierte Wert der Energiewert −48,0 dB, 12,0 dB über dem Gate, und jedes gezählte Fenster ist voll (`Spektrum.h:276-277`) — `abdeckung` ist bei allen vier Raten 1 |
| 3 | **M-73**, **M-74** (neu), §5.6, §6.6 | M-73 trug **zwei** Zusagen — Blockgrößenunabhängigkeit **und** Gleichheit mit dem isolierten Lauf — und war als reine Regressionswache geführt, obwohl die zweite am Basisstand fällt | Je Zusage eine Zeile: **M-73** sagt nur noch die Blockgrößenunabhängigkeit zu und bleibt **Regressionswache (heute grün)**; die Gleichheit mit dem isoliert ab Fensteranfang verarbeiteten Lauf steht als **M-74** und ist **Baulücke, ROT am Basis-SHA** | Die Klassifikation stimmt wieder mit der Definition aus §5.6 überein („*Regressionswache* heißt heute grün und messbar"). M-74 ist rot aus derselben Quelle, die M-49 als F07-Beleg führt: der Reset steht am Binden (`FeatureEngine.h:611-622`), das laute Vormaterial läuft durch den Interpolator, und `Spektrum.h:192-200` schreibt seinen Nachklang in `passagenTruePeakRahmen` — rund +0,157 dBTP statt −6,02 dBTP, also rund 6,2 dB daneben. §5.6 ist aus den Zeilen neu gezählt: die 20 Regressionswachen bleiben 20, die Baulücke steigt von 53 auf 54 |
| 4 | **M-73** (Bühne und Rotbeweis) | Passage und Vormaterial waren ein durchlaufender 12-kHz-Ton; bei 48 kHz ist seine Periode vier Samples, und 64, 512, 1024 sowie beide Fenstergrenzen sind Vielfache davon — jeder Reset fiel in dieselbe Signalphase, und jedes Teilstück schwang wieder auf denselben Wert ein | **Eigene Bühne mit einem sparsamen Verlauf:** 12-kHz-Grund mit Amplitude 0,001 und darin **ein lauter Abschnitt von zwölf Samples** an den passagenrelativen Versätzen **180 bis 191** — drei `fs/4`-Perioden mit 45°-Phasenversatz, Sample-Werte ±0,3536 (−9,03 dBFS), rekonstruierte Amplitude 0,500 (−6,02 dBTP) | Die Versätze sind aus dem Blockraster gerechnet: 48 000 = 750 × 64, 48 000 mod 512 = 384, 48 000 mod 1024 = 896 → mutierte Resetstellen {0, 64, 128, 192, …} bei 64, {0, 128, 640, …} bei 512 und {0, 128, 1152, …} bei 1024. Der Kern ist um `kTapsJePhase / 2 = 12` Ticks zentriert (`TruePeak.h:75-78`, `:172-179`, `:189-193`): bei 64 leert der Reset an Versatz 192 die Kette, **bevor** eines der lauten Samples die Kernmitte erreicht — es bleibt die Untergrenze `max (absL, absR)` = −9,03 dBFS (`Spektrum.h:192`); bei 512 und 1024 liegt der letzte Reset 52 Samples davor, beide rekonstruieren vollständig und liefern denselben Wert. Der Abstand ist die **ganze** Intersample-Überhöhung, im eingeschwungenen Fall 3,01 dB (`TruePeak.h:13-19`) — ein Vielfaches der ±0,1-dB-Schranke |
| 5 | **§8.1 Feinheit 14**, **M-65**, §6.7 (Golden-Absatz und Risiken) | 64-fache Überabtastung und ±0,01 dB Referenzvalidierung standen als Festlegung ohne Quelle; die zitierten Stellen (Entwurf `:2626-2627`, SONDE-013 M-02/M-81) tragen nur ±0,1 dB | Beide Zahlen tragen ihre **Herleitung** mit: `interpolationsfehlerDb` (`TruePeak.h:215-220`) liefert bei `f/fs = 0,25` −0,169 dB (Faktor 4), −0,042 dB (Faktor 8) und **−0,00065 dB (Faktor 64)**; die ±0,01 dB nennt derselbe Quelltext für sein Passband (`:75-77`, `:84-87`). Präzedenz: NAK-180 **B17** (`docs/beweise/NAK-180.md:1289`). Der Normbezug steht ausdrücklich getrennt | Kein Rückbau: die Grenze war haltbar, nur unbelegt notiert. Das Projektbudget ±0,1 dB bleibt für alle Fälle 15 bis 23 unverändert; die 64 und die ±0,01 dB sind projektinterne Rechengrenzen der **Referenz** und werden keiner Norm zugeschrieben — BS.1770 Annex 2 verlangt mindestens vierfach (`TruePeak.h:21-22`), EBU Tech 3341 v4.0 §2.6 nennt nur die Toleranz 0,0 +0,2/−0,4 dBTP |

### 14.2 Die Belegregel, über die Etappen 3 und 5 und alle Runde-1-Zeilen geprüft

**Die Regel dieser Runde** (§13.1): jede Zahl, jede Messgröße und jede
Sperrenaussage einer Matrixzeile wird am **Basiscode** mit Datei und Zeile
belegt; jede Zeile mit zwei Zusagen wird je Zusage klassifiziert oder geteilt.
Geprüft wurden **alle** Zeilen der Etappen 3 und 5 (M-13 bis M-26, M-44 bis
M-60, M-73) sowie **alle** in der Matrix-Nacharbeit 1 geänderten Zeilen (§11:
M-01, M-04, M-05, M-11, M-13, M-16, M-20, M-22, M-26, M-31, M-34, M-44, M-45,
M-46, M-47, M-49, M-50, M-57, M-58, M-63, M-65, M-66, M-67, M-68, M-71, M-72,
M-73) — nicht nur die fünf Punkte.

**Zwei Zusagen in einer Zeile.** Geteilt wurde **M-73** (Punkt 3). Sonst tragen
alle geprüften Zeilen entweder genau eine Zusage oder klassifizieren bereits je
Zusage: M-01 und M-11 je Hälfte mit eigener Mutation, M-13 je Fenster, M-20
(„Zwei Zusagen, beide heute erfüllt") je Hälfte, M-26 je Sprache, M-55 je
Teilfall — (a) Regressionswache, (b) Riegel —, M-59 ausdrücklich gemischt,
M-46 und M-48 mit zwei Richtungen **derselben** Zusage, M-49 und M-51 mit zwei
Mutationen an **derselben** Zusage. In **M-44** war die zweite Zusage keine
Teilung wert, sondern ein Fehler: `aktivitaet` ist eine Zeitbereichsgröße
(`Lautheit.h:70-72` über `zelleAktivEnergie`, `Spektrum.h:166`; Quotient
`Frame.h:548-551`) und von der Samplerate **ohnehin** unabhängig — der Δf-Fix
berührt sie nicht, und die Mutation kippt sie nicht. Sie ist deshalb aus M-44
gestrichen; ihre einzige tragende Rolle steht unverändert in M-46, wo die
Kohärenz von `aktivitaet` und `abdeckung` die Zusage **ist**.

**Vier Belegkorrekturen, je mit Grund** — drei an Zeilen außerhalb der fünf
Punkte, die vierte an der Zustandsspalte von M-13:

- **M-19** — die Hochwasserprüfung war mit `queues.rs:349` belegt; dort steht
  am Basisstand der Testhelfer `fn eintrag`. Gemessen liegt die Prüfung
  `eintrag.marke < hoch` an `:220-222` und die Positionsentscheidung an
  `:223-229`. Grund: Belegregel, Ortsaussage falsch belegt.
- **M-24** — die „heutige Prüfzeile `:3241`" ist die Meldungszeile des
  `assert!`; der `all` über `ausschlussgrund.is_none()` steht an `:3240`, das
  ganze `assert!` an `:3237-3243`. Grund: Belegregel.
- **M-13 (Zustandsspalte)** — `aufnahmen_sammeln` war mit
  `hypothese_verdrahtung.rs:72` belegt; dort steht `hypothesen_bilden`. Die
  Funktion beginnt an `:130` und nimmt den Standlock an `:131`. Grund:
  Belegregel — gerade bei einer Sperrenaussage darf der Anker nicht
  danebenliegen.
- **M-45** — die Zusage „beide Stufen entscheiden das Gate gleich" nannte
  **keine** beobachtbare Größe, und die naheliegende (`abdeckung`) kann sie
  nicht tragen, weil nur die Hauptstufe gezählt wird (`Spektrum.h:373`).
  Beobachtbar ist die Stufenentscheidung an den Bändern: die zwei Stufen
  teilen sich die Evidenzbänder überschneidungsfrei (`:394-397`, Selbstzusage
  `:417-424`, Trennung `kTrennungHz = 200,0` in `FeatureEngine.h:240`, erstes
  Hauptband `Zeit.h:107-111`), und ein Fenster ohne Gate liefert für seine
  Bänder gar keinen Wert (`:388-389`). Grund: Belegregel — dieselbe Ursache
  wie Punkt 2, eine Zeile weiter.

Dazu **M-49**: seine Testspalte nennt jetzt beide Blockgrößenproben — M-73 für
die Blockunabhängigkeit, M-74 für die Gleichheit mit dem isolierten Lauf. Reine
Folge der Teilung aus Punkt 3, keine eigene Aussage.

**Was geprüft und nicht geändert wurde.** In Etappe 3 halten M-14 bis M-18,
M-20 bis M-23, M-25 und M-26 ihre Zahlen und Ortsangaben am Basiscode;
nachgemessen wurden `queues.rs:152` (`VecDeque::with_capacity(16)`), `:202-238`,
`:228` (`std::mem::replace`), `evidenz.rs:39` (`EVIDENZ_RETENTION = 32`) und
`:179-189` (`push_back`, `pop_front`), `invalidierung_verdrahtung.rs:139`
(`Vec<(ClientKey, usize)>`), `:155-176` (Markierungsschleife, `zurueck.push` an
`:173`), `:230-243` (`invalidierung_ruecknehmen` samt Selbstzusage `:232-234`),
`:88-92`, `hypothese_verdrahtung.rs:91` (Rechenhaken), `:443-477`, `:478`,
`:479-481`, `:961` sowie `coordinator/hypothese/befund.rs:353`. In Etappe 5
halten M-46 bis M-48 und M-50 bis M-60 ihre Anker; nachgemessen wurden
`FeatureEngine.h:107-118`, `:235`, `:240`, `:247`, `:249`, `:251`, `:259`,
`:217-222`, `:611-622`, `Spektrum.h:55-86`, `:107-113`, `:133-141`, `:166`,
`:192-201`, `:276-277`, `:341-400`, `:388-389`, `:394-397`, `:473-479`,
`Lautheit.h:30-40`, `:70-72`, `:90-102`, `:109-119`, `Frame.h:99-109`,
`:486-490`, `:532-546`, `:548-551`, `:636` (Mindestbelegung vier Rahmen) und
`TruePeak.h:13-19`, `:21-33`, `:75-78`, `:172-179`, `:189-193`, `:201-205`,
`:215-220`. Außerhalb der zwei Etappen wurden M-31 und M-34 gegengelesen: beide
tragen genau eine Zusage, und M-34 sagt selbst, dass es die Behebung von F05
**nicht** belegt.

### 14.3 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Zeile | Für diese Runde |
|---|---|
| **A** Rückstau und Prioritätsklassen | **Nicht zutreffend** — diese Runde baut keinen Code. Neu für die Bauetappen: der Riegel in `befund_persistieren` hält den Standlock über die **Annahme**, nie über das Commit-Warten; die Kanalgrenze `STORE_KANAL_CAP = 256` wird dabei zum sichtbaren Fehlerfall am gehaltenen Guard statt zu einem stillen Warten. |
| **B** Lebenszyklus | **Nicht zutreffend.** Unverändert §7; M-73 und M-74 hängen beide am Paar Passagenanfang↔Passagenende (M-52), eine neue Beziehung entsteht nicht. |
| **C** Verträge und Längen | **Nicht zutreffend.** Kein Feld reist neu über den Draht; `abdeckung` behält Typ und Bedeutung, die Zeile sagt nur noch zu, was der Code auch zählt. |
| **D** Bau- und Prüfriegel | **Nicht zutreffend.** Kein Bau, kein Bein, keine Beglaubigung in dieser Runde. |
| **E** Behauptung ≤ Messung | **Gilt hier.** Kopf „Urteil offen", Etappenzeile „Matrixprüfung 3 steht aus". Jede neue Zahl trägt Datei und Zeile oder ist aus benannten Konstanten gerechnet (§5 „Gerechnete Zahlen" und „Zahlen des True-Peak-Pfads"). **Zwei Behauptungen sind zurückgenommen**, weil sie mehr sagten als die Quelle trägt: „`abdeckung` zählt beide Stufen" und die gleiche Aktivitätsentscheidung als eigene Zusage, beide in M-44. **Eine Behauptung ist absichtlich offengelassen:** wie groß der Abstand der drei M-73-Läufe unter der Mutation genau ist, misst der Etappenlauf — hier steht nur, dass er die **ganze** Intersample-Überhöhung betrifft und damit ein Vielfaches von ±0,1 dB. Die Klassenzählung in §5.6 ist aus den Zeilen gezählt, nicht fortgeschrieben. |
| **F** Änderungssatz | **Nicht zutreffend für Code.** Für das Manifest gilt: Matrixzeile, Bauplanabsatz, Feinheit und Zählung sind je Punkt **gemeinsam** nachgezogen — M-13 mit §6.4 und Feinheit 18, M-44 mit dem §5-Vorspann, M-73 und M-74 mit §5.6 und §6.6, M-65 mit §6.7 und Feinheit 14. |

**Eigene Messungen dieser Runde** — alles andere ist gelesen, nicht gemessen:

- `git rev-parse HEAD` = `168f2319c5634445b322517ad45a6d7598d14fc8`,
  `git status --short` leer.
- `broker/src/store/handle.rs`: `append` ist `append_einreihen` plus `recv`
  (`:148-151`); `append_einreihen` ist `pub` (`:156`) und nimmt nur
  store-eigene Sperren (`:234`, `:251`); `senden` benutzt
  `SyncSender::try_send` (`:232`) und meldet `KanalVoll` sofort (`:239-242`);
  der Kanal ist ein `sync_channel(STORE_KANAL_CAP)` (`store/writer.rs:53`,
  `store/mod.rs:48`). Kein Pfad unter `broker/src/store/**` nimmt den
  Standlock — das `stand` an `handle.rs:14`, `:25`, `:33` ist eine
  Teststartbarriere, nicht der Coordinator-Stand.
- `broker/src/store/writer.rs`: ein Writer-Thread über einen FIFO
  (`:176-193`), Nachschub per `aktiv.push_back` (`:298`), Commit in derselben
  Reihenfolge (`:325-334`), `event_ord = tx.last_insert_rowid()` (`:476`),
  Projektionsriegel `WHERE excluded.last_event_ord > …` (`:588-592`).
- `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h`: `abdeckung` wird
  ausschließlich innerhalb `if (&s == &haupt)` gezählt (`:373`, Zähler
  `:382-383`, Begründung im Quelltext `:369-372`); `summeBereich` summiert die
  PSD ohne Δf (`:473-479`); ein Fenster wird erst bei
  `++s.gefuellt == s.punkte` gerechnet (`:276-277`); `tpJetzt` hat den
  Sample-Peak als Untergrenze (`:192`) und geht nur `imPassagenfenster` ins
  Passagenmaximum (`:197-200`); `passVon` ist blocklokal (`:55`, `:61`,
  `:75-80`).
- `eq-copilot/plugin/core/analysis/TruePeak.h`: die Kernmitte liegt zwölf
  Ticks nach dem Sample (`:75-78`, `:172-179`, `:189-193`); 3,01 dB
  Überhöhung bei fs/4 und 45° Phase, Paar −9,03 dBFS gegen −6,02 dBTP
  (`:13-19`); BS.1770-4 Annex 2 „mindestens 4-fach" und die Fehlerformel
  (`:21-33`); `interpolationsfehlerDb` (`:215-220`). Aus dieser Formel bei
  `f/fs = 0,25` nachgerechnet: Faktor 4 → −0,169 dB und Faktor 8 → −0,042 dB
  (beide decken sich mit `:28-30` und mit NAK-180 B17), Faktor 64 →
  −0,00065 dB.
- `docs/beweise/NAK-180.md:1289`: B17 trägt die 8-fach-Entscheidung als Zahl
  mit genau diesen beiden Werten.

### 14.4 Was diese Runde nicht geändert hat

- **Keine Regel, kein Budget.** R-283-1 bis R-283-6 (§2) stehen unverändert;
  Feinheit 18 schärft nur, **wie weit** die Sperre reicht. Das
  True-Peak-Projektbudget bleibt ±0,1 dB gegen eine validierte Referenz für
  alle Fälle 15 bis 23.
- **Keine Ticketpfade.** `broker/src/store/**` bleibt außerhalb der Etappe 3;
  die Kopplung benutzt eine bereits öffentliche Kante. §3 und §8.4 sind
  unberührt.
- **Keine Umklassifizierung außer der Teilung.** M-73 bleibt Regressionswache,
  M-74 entsteht als Baulücke; keine andere Zeile wechselt ihre Klasse, und je
  Befund bleibt mindestens eine heute rote Zeile (§5.6 Schlussabsatz).
- **Zwei offene Punkte für den Dirigenten** (Kopf- und Verlaufspflege, nicht
  Bauersache; beide liegen außerhalb der Ticketgrenze dieser Runde — §13.1
  nennt nur die Kopfzeile „Etappe"): die Kopfzeile **„Gate"** nennt weiterhin
  „die Matrixzeilen M-01 bis M-73", und der **Nachtrag der Matrix-Nacharbeit 1
  in §7** nennt „**73** IDs (M-01 bis M-73) … 53 Baulücke / 20
  Regressionswache". Beide sind mit **74 IDs, 54 Baulücke / 20
  Regressionswache** nachzuziehen — so wie §12 es nach der Matrix-Nacharbeit 1
  schon einmal getan hat. Innerhalb der Grenze ist alles nachgezogen: §6.0
  nennt die Rotbeweisdateien jetzt bis `-M-74.txt`, §6.6 seine Zeilenliste,
  §8.1 Feinheit 17 seine Messzeilen.


## 15. Messung der Matrix-Nacharbeit 2 und Start der Matrixprüfung 3 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `c0087632` (`nakama-nak283-168f231-matrix-nach2`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-matrix-nacharbeit-2-auftrag.txt`; 15:32 bis 15:58 Uhr; Beobachter (Aufsicht NORMAL): Start, Telemetrie gesund, zwei HEAD-Wechsel (eigener Commit, dann Rebase beim Push auf zwei fremde Commits), Ende; Loop gelöscht. |
| Commit | `8f59bcccf729db0be20d80497dc981d9d26e424f` „NAK-283 Matrix-Nacharbeit 2: fünf Punkte der Matrixprüfung 2 geschlossen, Belegregel über Etappen 3 und 5 angewandt", auf `origin/master`; Basis `168f2319` ist Vorfahr; `git status --short` leer. Dazwischen fremd und unberührt: `82ed3e87` (NAK-285, Installation über erhöhte Aufgaben, andere Session) und `926a5d02` (NAK-284, Diagnoseprojekt `eq-copilot/fixtures/fl/Nakama-Diagnose.flp`, andere Session) — sie berühren dieses Manifest nicht. |
| Diff | `git show --stat 8f59bccc`: nur `docs/beweise/NAK-283.md`, +365 / −76 (§5, §6, §8, neuer §14). Kein Code berührt. |
| Rundenbilanz | `8f59bccc~1..8f59bccc: Doku 1 Datei(en) +365/-76 — OHNE PRODUKTFORTSCHRITT` (Matrixrunde, zählt null); kumuliert seit `39e18d3f`: nur `docs/` aus diesem Ticket. |
| Selbstbericht des Bauers (§14, nicht als Beweis gezählt) | fünf von fünf geschlossen, Belegregel über Etappen 3 und 5 angewandt; Matrix M-01 bis M-74; §5.6: 74 Zeilen — 54 Baulücke (46 heute rot, 7 nicht messbar, 1 gemischt), 20 Regressionswache. |
| Matrixprüfung 3 | Wiederprüfung 2 (Vorlage B): Codex `gpt-6-astra`, Effort **max**, lesend, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-matrix3`); Prüfbereich `git diff 168f2319...8f59bccc -- docs/beweise/NAK-283.md`, Befundliste = die fünf Punkte der Matrixprüfung 2; Auftrag `docs/beweise/roh/NAK-283-matrixpruefung-3-auftrag.txt`; Thread-ID und Urteil folgen in §16. Rundenbudget: nach dieser Wiederprüfung ist eine Nacharbeitsrunde übrig; endet die dritte Wiederprüfung ohne PASS, folgt der Konvergenzentscheid (Skill §3.4). |


## 16. Matrixprüfung 3 (Wiederprüfung 2) — NEEDS_WORK mit einem Restpunkt (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-matrix3`); Thread `01a095eb-360e-7721-b004-4a995cd09419`; Lauf 16:00 bis 16:11 Uhr, Exit 0. |
| Prüfbereich | Wiederprüfung 2 (Vorlage B): `git diff 168f2319...8f59bccc -- docs/beweise/NAK-283.md` mit der Fünf-Punkte-Liste der Matrixprüfung 2; HEAD `467e4534` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. |
| Auftrag und Urteil | `docs/beweise/roh/NAK-283-matrixpruefung-3-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/NAK-283-matrixpruefung-3-467e453.txt`. |
| Urteil | **NEEDS_WORK — vier von fünf Punkten geschlossen, nichts gebrochen, Zählung bestätigt (74 = 54 Baulücke + 20 Regressionswache).** Offen bleibt allein Punkt 1 (M-13, §6.4:995-997): Lockkopplung und Fehlerpfad sind belegt (§6.4:961-989) und schließen den ursprünglichen Fehler; der deterministische Rotbeweis fehlt — im Ablauf von M-13/Fenster 2 persistiert B vollständig, bevor A den Wiedervergleich ausführt, A wird schon dort verworfen, die Mutation „Lock zwischen Vergleich und `append_einreihen` freigeben" bleibt grün; der Haken sitzt weiter vor `befund_persistieren` (`hypothese_verdrahtung.rs:478-480`). Verlangt: ein zusätzlicher Ablauf nach bestandener Prüfung mit dem Erfolgskriterium „A wird zuerst angenommen, danach bestimmt B die jüngere Projektion". |
| Rundenbudget | Nacharbeiten 1 und 2 verbraucht; **Nacharbeit 3 = Runde 3 von 3**, als enger Einzelauftrag auf M-13 und §6.4. Endet die dritte Wiederprüfung ohne PASS, folgt der Konvergenzentscheid (Skill §3.4). |
| Validierung | Lesender Opus-Agent (Effort max) an HEAD `467e4534`, Ergebnis `docs/beweise/roh/NAK-283-matrixpruefung-3-validierung.md`; Einordnung und Auftrag in §16.1. |


### 16.1 Einordnung des Restpunkts und Auftrag der Matrix-Nacharbeit 3 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Validierer | Lesender Opus-Agent (Effort max), 16:14 bis 16:27 Uhr an HEAD `467e4534`/`e82572ef` (Code = `ea6ddd57`); Rohdatei `docs/beweise/roh/NAK-283-matrixpruefung-3-validierung.md`. |
| Ergebnis | **BESTÄTIGT, DEFEKT** — Belegdefekt der Matrix: die Zusage „Annahmeordnung unter derselben Sperre" (Feinheit §13.1 zu R-283-1) hat keinen deterministischen Rotbeweis, weil der Haken vor `befund_persistieren` (`hypothese_verdrahtung.rs:478-480`) das Fenster nach bestandenem Vergleich nicht erzwingt. Kein Rückbau der Lockkopplung. Nebenbefund: §6.4 nennt `broker/src/coordinator/mod.rs` (Hakenfeld) nicht ausdrücklich — liegt innerhalb der §3-Pfade „`broker/src/` (Coordinator, …)", keine neue Abweichung. |
| Schließende Änderung | Neue Zeile **M-75** (Fenster 3): zweiter Testhaken in `befund_persistieren` zwischen bestandenem Vergleich und `append_einreihen` unter gehaltenem Guard, Rendezvous nach dem Muster `CoordinatorFlushTestHaken`, Freigabe durch einen dritten Faden ohne Standlock (deadlockfrei, Frist im Test); Erfolgskriterium: A wird zuerst angenommen, B danach eingereiht, die Projektion trägt B; Rotbeweis: Guard vor `append_einreihen` freigeben. M-13 bleibt für Fenster 1 und 2; §6.4 belegt Haken, Freigabe und Deadlockfreiheit an der Quelle; §5.6 neu gezählt. |
| Nacharbeit 3 | Frischer Opus-Worker (max), nur `docs/beweise/NAK-283.md` (M-13, M-75, §5.6, §6.4, §17, Kopfzeile Etappe); Auftrag `docs/beweise/roh/NAK-283-matrix-nacharbeit-3-auftrag.txt`; **Runde 3 von 3**. Danach Matrixprüfung 4 (Wiederprüfung 3) über den Diff dieser Runde mit diesem einen Punkt; endet sie ohne PASS, folgt der Konvergenzentscheid (Skill §3.4): Einordnung am Gate-Wortlaut, höchstens eine weitere Runde mit der Ursache als Auftrag. |


## 17. Matrix-Nacharbeit 3 (Bauer, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Runde | **3 von 3** nach der Matrixprüfung 3 (§16, §16.1), enger Einzelauftrag auf den einen bestätigten Restpunkt. Basis `e5e56e9788b5fb386493475bc5c39801de73a9cc`, `git status --short` beim Start leer. Gemessen: `broker/**`, `eq-copilot/plugin/**` und `eq-copilot/schemas/**` sind seit der Audit-Basis `ea6ddd57` unverändert (`git diff --stat ea6ddd57..HEAD` darüber ist leer); hinzugekommen sind seither nur drei Dateien fremder Tickets, die dieses Manifest nicht berühren (`eq-copilot/fixtures/fl/LIES-MICH.md` und `Nakama-Diagnose.flp` aus NAK-284, `tools/dirigent/install-aufgaben-registrieren.ps1` aus NAK-285). |
| Ticketgrenze | Genau eine Datei: `docs/beweise/NAK-283.md`. Geändert wurden die Kopfzeile „Etappe", §5 (Überschrift und Vorspann als Zählungsaussage, M-13 und die neue M-75 in §5.2, §5.6), §6.4 (Überschrift, Ticketpfade, Fenster-3-Absatz, Testtabelle, Reihenfolge, Risiken) und dieser neue §17. §0 bis §4, §6.0 bis §6.3, §6.5 bis §6.8 und §7 bis §16 sind unberührt. Kein Produktcode, kein Test, kein Schema, kein Werkzeug, keine andere Doku. Nicht gebaut, nichts gefahren; Git außer Commit und Push nur lesend. |
| Umfang | Ein Punkt, ein Punkt geschlossen. **Zwei Zeilen** berührt: die neue **M-75** und M-13, das seine Fenster 1 und 2 behält und den Ordnungsteil abgibt. Eine neue ID (**M-75**), keine gestrichen, keine verschoben. Dazu §5 Überschrift und Vorspann, §5.6 und §6.4 (Überschrift, Ticketpfade, Fenster-3-Absatz, Testtabelle, Reihenfolge, Risiken). |
| Zählung | §5.6 neu aus den Zeilen gezählt: **75 Zeilen, 55 Baulücke (46 heute rot, 8 heute nicht messbar, 1 gemischt), 20 Regressionswache, 13 Zahlenrand, 6 Übergänge, 0 OFFEN.** Vorher: 74 / 54 (46 / 7 / 1) / 20. |
| Produktfragen | Keine beantwortet. U40 bis U42 bleiben Karten; §8.3 ist unverändert. |

### 17.1 Die neue Zeile M-75, wörtlich

Wiedergegeben ist der Text der Zeile aus §5.2, Spalte für Spalte und in
Anführungszeichen; die Spaltenmarken ersetzen die Tabellentrenner, der
abschließende Satzpunkt ist gesetzt. Sonst ist jede Spalte zeichengleich mit
der Matrixzeile.

**ID** M-75 · **Befund** F02 · R-283-1 · **Etappe** 3

**Zustand und Vorbedingung.** „**Fenster 3 derselben Naht, nach bestandenem
Wiedervergleich** (M-13 trägt die Fenster 1 und 2). Bühne mit Store wie
`coordinator_mit_store` (`broker/tests/sonde014_verdrahtung.rs:948-958`),
**eine** Sitzung und **genau ein** Befund je Sitzung — der Persistenzlauf ruft
`befund_persistieren` je Befund (`hypothese_verdrahtung.rs:479-481`), und nur
mit einem einzigen Befund trifft der einmalige Haken sicher den Lauf von A.
Rechnung A hat unter dem Standlock 12 stabile Evidence-IDs gesammelt
(`aufnahmen_sammeln` nimmt ihn an `:131`), ohne Lock gerechnet, die
Frischeprüfung bestanden und steht in `stand.befunde` (`:463-476`); der Guard
ist gefallen (`:478`). Drei Fäden: **A**, **B** (der zweite
`eqcop-v3-conn`-Faden) und der **Kontrollfaden** des Tests, der den Standlock
nie nimmt."

**Ereignis.** „A besteht in `befund_persistieren` (`:494-515`, nach dem Umbau)
den Wiedervergleich gegen den Standeintrag **unter dem Standlock** und hängt am
**neuen Annahmehaken unmittelbar vor** `StoreHandle::append_einreihen`
(`broker/src/store/handle.rs:156-171`), **am gehaltenen Guard**. Der
Kontrollfaden **startet darauf B** und wartet befristet auf dessen
Abschlussmarke, ohne selbst je den Standlock zu nehmen; **B** stellt die
13. Evidenz zu, sammelt, rechnet und will eintragen — und **blockiert am
Standlock** (schon die Zustellung braucht ihn, spätestens
`aufnahmen_sammeln:131`), solange A ihn hält. Läuft die Frist ab, gibt der
Kontrollfaden A frei."

**Zusage.** „**Wer den Vergleich später besteht, reiht später ein.** A wird
**zuerst** angenommen, B danach: das `event_log` trägt **beide**
`finding`-Events in Annahmereihenfolge — A mit der kleineren, B mit der
größeren `event_ord` —, und die Projektion `findings.state_jcs`
(`store/writer.rs:572`, `:588-593`) trägt den **13-ID-Payload von B**;
flüchtiger Stand und Projektion sind gleich. Dass beide dieselbe
Projektionszeile treffen, ist gemessen: `evidence_ids` steht im Payload
(`hypothese_verdrahtung.rs:597`), `finding_id`
(`coordinator/hypothese/befund.rs:353-373`) hasht Sitzung, Metrik, Band, Quelle
und `metrics_version` — **nicht** die Evidenzmenge."

**Reihenfolge und Frist.** „**Standlock über Vergleich und Annahme**
(`:494-515`), Freigabe **vor** dem `recv`. Die Annahme vergibt die
Ordnungszahl: ein einziger Writer-Thread liest den FIFO
(`store/writer.rs:176-193`), merkt Nachschub mit `aktiv.push_back` vor
(`:298`) und schreibt in derselben Reihenfolge (`:325-334`), sodass
`event_ord = tx.last_insert_rowid()` (`:476`) der Annahmereihenfolge folgt und
die Projektion die höhere behält (`:588-593`). **Der Haken:** Rendezvous nach
dem Muster `CoordinatorFlushTestHaken` (`broker/src/coordinator/mod.rs:164-209`,
`erreichen` `:200-208`, `freigeben` `:188-193`); Feld und Setter liegen in
`mod.rs` wie `flush_test_haken` (`:223`, `:381-390`). Er wird **vor** dem
`lock()` per `take()` herausgenommen (Muster `flush.rs:138-142`,
`mod.rs:369-378`) und erst unter dem Guard gefeuert — so nimmt kein
Coordinator-Mutex den Standlock als Vorgänger, die Bedingung aus
`mod.rs:224-228`. **Kein Deadlock:** der Freigeber ist der Kontrollfaden, der
den Standlock nie nimmt; die Gegenrichtung Haken → Standlock existiert nicht.
Ein Haken, der auf den **Abschluss von B** wartete, wäre in der Fixfassung
zirkuläres Warten. Die Frist (Vorschlag 500 ms) liegt im **Test**, nie im
Produkt: dort ist der Haken nie gesetzt."

**Test.** „`broker/tests/sonde014_verdrahtung.rs`, **NEU**
`annahmeordnung_folgt_der_vergleichsordnung` (drei Fäden, ein Rendezvous), Bein
**A4**; `event_ord` und `state_jcs` gelesen wie `user_verdict_ids_im_store`
(`:3341-3350`) und `ereignisse_im_log` (`:3353-3361`)."

**Rotbeweis.** „Mutation an der Zeile, die die Zusage trägt: in
`befund_persistieren` den Guard **zwischen bestandenem Vergleich und
`append_einreihen` freigeben** (die Fassung der Nacharbeit 1). Dann feuert der
Haken **ohne** Guard, B läuft in der Lücke vollständig durch und reiht als
Erster ein, A danach — A bekommt die höhere `event_ord`
(`store/writer.rs:476`) und gewinnt die Projektion (`:588-593`) mit dem
12-ID-Payload; Stand und Projektion stehen auseinander. **Am Basis-SHA nicht
messbar**, weil Wiedervergleich, Kopplung und Annahmehaken erst mit dieser
Etappe entstehen; **rot gegen die Fixfassung**. Der Lauf sichert seinen
**Freigabegrund** zu: Fixlauf `Frist` (der Beleg, dass B nicht durch die Lücke
kam), Mutantenlauf `Signal`. Ein Mutantenlauf mit `Frist` gilt als **nicht
gemessen** und wird mit längerer Frist wiederholt, nie als grün gezählt; bleibt
die Erfassungsmarke des Hakens aus, ist der Fall ebenfalls nicht gemessen —
dann hat A den Vergleich nicht bestanden, den die Bühne verlangt."

**Quelle.** „§2 R-283-1; §4 F02 (NAK-253); §8.1 Feinheit 18; Matrixprüfung 3
Punkt 1 (§16, §16.1)."

**Warum eine eigene Zeile und keine dritte Hälfte von M-13** — drei Gründe,
alle aus der Matrix selbst:

- **Andere Zusage.** M-13 sagt zu, dass die Frischeprüfung bis Persistenz und
  Publikation reicht: A schreibt **nicht**. M-75 sagt zu, dass die
  Annahmeordnung der Vergleichsordnung folgt: A schreibt, aber **vor** B.
  Beides in einer Zeile zu führen ist genau der Zeilenfehler, den die
  Matrixprüfung 2 an M-73 gerügt und die Matrix-Nacharbeit 2 mit der Teilung
  in M-73/M-74 geschlossen hat (§14.1 Punkt 3).
- **Andere Klasse.** M-13 ist „rot in einer Hälfte, nicht messbar in der
  anderen" (§5.6). M-75 ist **ganz** „heute nicht messbar", weil am Basisstand
  weder Wiedervergleich noch Kopplung noch Haken existieren.
- **Andere Erfolgsrichtung.** M-13 Fenster 2 misst ein **ausbleibendes**
  Schreiben, M-75 die **Reihenfolge** zweier tatsächlicher Schreibvorgänge.

### 17.2 Die Änderung an M-13

| Stelle | Vorher | Jetzt | Warum |
|---|---|---|---|
| Ereignisspalte | „**Zwei Fenster derselben Naht.**" | „**Zwei Fenster derselben Naht** — das dritte, der Wettlauf **nach bestandenem** Wiedervergleich, steht in **M-75**." | Der Verweis macht sichtbar, dass die Naht drei Fenster hat und welche Zeile welches misst. |
| Zusagespalte (2) | „**A schreibt nicht, und wer später einreiht, gewinnt:** … die Projektion trägt B's 13-ID-Payload **mit der höheren Ordnungszahl** …" | „**A schreibt nicht:** … die Projektion trägt B's 13-ID-Payload … **Die Ordnung zweier tatsächlich geschriebener Payloads sagt diese Zeile nicht zu — das ist die Zusage von M-75**" | In Fenster 2 schreibt nur **einer**; eine Ordnungszusage zwischen zwei Schreibern kann dieser Ablauf gar nicht herstellen. Sie stand hier als zweite Zusage ohne eigenen Rotbeweis — dieselbe Klasse Fehler wie bei M-73 vor der Teilung. |
| Rotbeweisspalte (b) | „In `befund_persistieren` die Sperre **zwischen Vergleich und `append_einreihen` freigeben** … und B in dieser Lücke laufen lassen → A besteht den Vergleich gegen den Stand vor B, reiht **nach** B ein …" | „In `befund_persistieren` (`:494-515`) **den Wiedervergleich entfernen** — die Sperre bleibt unberührt — und den Ablauf dieser Zeile fahren → A persistiert seinen 12-ID-Payload, reiht nach dem bereits fertigen B ein, bekommt die höhere Ordnungszahl (`store/writer.rs:476`) und gewinnt die Projektion (`:588-593`) …; das `event_log` trägt dann einen `finding`-Event mit A's Payload, und der Zähler des unterlassenen Schreibversuchs bleibt bei 0." | Die alte Mutation **fällt in diesem Ablauf nicht**: B persistiert vollständig, bevor A vergleicht, also verwirft der Vergleich A ohnehin, und die Lockfreigabe bliebe grün (Matrixprüfung 3 Punkt 1). Die neue Mutation trifft die Zeile, die Fenster 2 zusagt — den Wiedervergleich —, und kippt genau deren Messgrößen. |
| Rotbeweisspalte, Schlusssatz | — | „**Die Mutation „Sperre zwischen Vergleich und `append_einreihen` freigeben" steht nicht mehr hier:** … Sie ist der Rotbeweis von **M-75**." | Der Grund steht an der Zeile, nicht nur im Rundenabschnitt; ein späterer Leser findet ihn dort, wo er die Mutation sucht. |
| Quellspalte | „… Matrixprüfung 2 Punkt 1 (§13.1)" | „… Matrixprüfung 2 Punkt 1 (§13.1); Matrixprüfung 3 Punkt 1 (§16.1)" | Die Zeile trägt jetzt zwei Prüfrunden. |

Unverändert bleiben Zustandsspalte, Spalte „Reihenfolge und Frist" (der
Kopplungsmechanismus samt Deadlockargument), Testspalte und Etappe: die
Kopplung selbst ist richtig und wird nicht zurückgebaut — es fehlte allein der
Ablauf, der sie misst.

### 17.3 Die §6.4-Absätze

| Stelle | Vorher | Jetzt |
|---|---|---|
| **Überschrift** | „Etappe 3 — Broker (M-13 bis M-25, M-26 Hälfte 2)" | „… M-26 Hälfte 2, **M-75**" — dieselbe Pflege, die §6.6 nach der Matrix-Nacharbeit 2 für M-74 bekommen hat |
| **Ticketpfade** | `hypothese_verdrahtung.rs`, `invalidierung_verdrahtung.rs`, `evidenz.rs`, `queues.rs`, zwei Testdateien — **ohne** `broker/src/coordinator/mod.rs` | `broker/src/coordinator/mod.rs` ist ausdrücklich genannt, mit den Ankern der beiden Hakenfelder und ihrer Setter (`:223` mit `:381-390`, `:244` mit `:357-362` und `:369-378`, Rendezvousform `:164-209`) und dem Satz, dass der Pfad innerhalb von §3 „`broker/src/` (Coordinator, …)" liegt und **keine** neue Abweichung ist (§8.4 unberührt) |
| **Fenster-2-Absatz, Schlusssatz** | „Zwischen Cache-Eintrag und Persistenzlauf (`:479-481`) entsteht ein Testhaken für das erzwungene Interleaving, im Produkt leer — Muster `rechen_test_haken_ausloesen` (`:91`)." | Derselbe Satz, als „**Erster Haken (Fenster 2)**" benannt — damit der zweite unterscheidbar ist |
| **Fenster-3-Absatz** | fehlte | Neuer Listenpunkt „`hypothese_verdrahtung.rs` (**Fenster 3, Annahmeordnung — M-75**)" mit sechs belegten Teilen: **Ort** (unmittelbar vor `append_einreihen`, am gehaltenen Guard, und warum der Ort scharf ist), **Form** (Rendezvous `mod.rs:164-209`, ausdrücklich **nicht** das Closure-Muster, weil dessen Fälle `sonde014_verdrahtung.rs:575`, `:631` auf A's Faden in den Coordinator zurückrufen — Selbst-Deadlock, `std::sync::Mutex` nicht reentrant), **Sperrenordnung** (`take()` vor dem `lock()` nach `flush.rs:138-142` und `mod.rs:369-378`, Bedingung aus `mod.rs:224-228`; Präzedenz einer Naht unter dem Standlock `flush.rs:85-91`, die aber panisiert statt zu warten), **Deadlockfreiheit** (Freigeber ohne Standlock; ein Haken auf B's Abschluss wäre zirkuläres Warten), **Frist und Freigabegrund** (Frist im Test, `Frist` im Fixlauf, `Signal` im Mutantenlauf, sonst nicht gemessen), **Erfolgskriterium** (beide Events im `event_log`, A mit der kleineren `event_ord`, Projektion trägt B; `evidence_ids` im Payload `:597`, `finding_id` ohne Evidenzmenge `befund.rs:353-373`) samt benannter Nebenwirkung |
| **Testtabelle** | dreizehn Zeilen, M-13 bis M-25 | eine Zeile mehr: `annahmeordnung_folgt_der_vergleichsordnung`, `broker/tests/sonde014_verdrahtung.rs`, Bein **A4**, M-75 |
| **Reihenfolge** | „… M-13 Fenster 1 …, dann Fenster 2 …, dann M-16 …" | „… dann **Fenster 3** (zweiter Haken unmittelbar vor der Annahme → M-75, Annahmeordnung), dann M-16 …" |
| **Risiken** | „… eine Freigabe vor der Annahme öffnete das Fenster, das die Matrixprüfung 2 gefunden hat (M-13 Rotbeweis (b))." | Verweis auf **M-75** statt auf M-13, plus die neue Regel: „**Ein Testhaken unter dem gehaltenen Guard ist nur mit einem Freigeber zulässig, der den Standlock nie nimmt**", mit `take()` vor dem `lock()` und der Frist im Test |

### 17.4 Die neue Zählung, aus den Zeilen gezählt

| Größe | Vorher (§14) | Jetzt | Woher |
|---|---:|---:|---|
| Matrixzeilen gesamt | 74 | **75** | M-75 neu; keine gestrichen, keine verschoben |
| Baulücke | 54 | **55** | M-75 ist Baulücke |
| davon heute rot | 46 | **46** | M-75 ist am Basis-SHA nicht messbar, nicht rot |
| davon heute nicht messbar | 7 | **8** | M-16, M-63, M-65, M-66, M-68, M-71, M-72, **M-75** |
| davon gemischt | 1 | **1** | M-59, unverändert |
| reine Regressionswache | 20 | **20** | keine Zeile wechselt die Klasse |
| Zahlenrand | 13 | **13** | M-75 nennt keinen Zahlenrand in der Zustandsspalte |
| Übergangszeilen | 6 | **6** | M-75 ist keine Übergangszeile: sie liegt ganz in F02 |
| OFFEN | 0 | **0** | keine Produktfrage in der Matrix |

Probe: 55 Baulücke + 20 Regressionswache = **75**; 46 rot + 8 nicht messbar +
1 gemischt = **55**. In der Zuordnungstabelle bekommen **F02** und
**R-283-1** die M-75. Der Satz „Je Befund bleibt mindestens eine Zeile, die
heute rot ist" gilt unverändert: für F02 trägt M-13 Fenster 1 den heutigen
Rotbeweis, M-75 kommt als achte nicht messbare Zeile hinzu und nimmt keiner
Zeile ihren Beleg.

### 17.5 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Zeile | Für diese Runde |
|---|---|
| **A** Rückstau und Prioritätsklassen | **Nicht zutreffend** — diese Runde baut keinen Code. Neu für die Bauetappe 3 ist nur eine Testnaht, kein Produktverhalten: der Annahmehaken ist im Produkt nie gesetzt, und die Frist, die A den Standlock halten lässt, liegt im Test. |
| **B** Lebenszyklus | **Nicht zutreffend.** M-75 fügt kein Paar hinzu; sie hängt an derselben Naht wie M-13 (sammeln↔zurückschreiben) und bleibt in deren Beziehungssatz. |
| **C** Verträge und Längen | **Nicht zutreffend.** Kein Feld reist neu über den Draht; `evidence_ids` und `finding_id` behalten Form und Bedeutung, die Zeile sagt nur zu, was der Code schon schreibt (`hypothese_verdrahtung.rs:597`, `befund.rs:353-373`). |
| **D** Bau- und Prüfriegel | **Nicht zutreffend.** Kein Bau, kein Bein, keine Beglaubigung in dieser Runde. |
| **E** Behauptung ≤ Messung | **Gilt hier.** Kopf „Urteil offen", Etappenzeile „Matrixprüfung 4 steht aus". Jede Sperren-, Orts- und Ordnungsaussage von M-75 und §6.4 ist an der Quelle gelesen und trägt Datei und Zeile (§17.6). **Eine Behauptung ist zurückgenommen:** M-13 sagt die Annahmeordnung nicht mehr zu — sie stand dort ohne fallenden Rotbeweis. **Eine Behauptung ist absichtlich offengelassen:** die konkrete Frist (500 ms) ist ein Vorschlag; welcher Wert trägt, entscheidet der Etappenlauf, und der Freigabegrund macht ein stilles Kippen unmöglich. Die Klassenzählung in §5.6 ist aus den Zeilen gezählt, nicht fortgeschrieben. |
| **F** Änderungssatz | **Nicht zutreffend für Code.** Für das Manifest gilt: die neue Zeile, der Bauplanabsatz, die Testtabelle und die Zählung sind **gemeinsam** nachgezogen — M-75 mit §6.4 (Haken, Test, Reihenfolge, Risiken, Ticketpfade), M-13 mit demselben Absatz, beide mit §5.6 und dem §5-Vorspann. |

**Eigene Messungen dieser Runde** — alles andere ist gelesen, nicht gemessen:

- `git rev-parse HEAD` = `e5e56e9788b5fb386493475bc5c39801de73a9cc`,
  `git status --short` beim Start leer.
  `git diff --stat ea6ddd57..HEAD -- broker eq-copilot/plugin eq-copilot/schemas`
  ist leer; über `eq-copilot broker tools` stehen drei Dateien aus NAK-284 und
  NAK-285 (§17 Kopf).
- `broker/src/coordinator/hypothese_verdrahtung.rs`: `hypothesen_bilden`
  `:72-106` mit dem Rechenhaken an `:91`; `aufnahmen_sammeln` beginnt `:130`
  und nimmt den Standlock `:131`; `befunde_eintragen` `:443-486` mit dem
  Frischeriegel `:463-466`, dem Gleichheitszweig `:467-470`, dem Eintrag
  `:475`, dem Guardende `:478` und dem Persistenzlauf `:479-481`;
  `befund_persistieren` `:494-515` mit `store.append` `:511` und dem
  Fehlerpfad, der den Standlock erneut nimmt, `:512-513`; `befund_json` ab
  `:541`, `evidence_ids` im Payload `:597`.
- `broker/src/coordinator/mod.rs`: `CoordinatorFlushTestHaken` `:164-209`
  (`warten_bis_erfasst` `:180-186`, `freigeben` `:188-193`, `erreichen`
  `:200-208`); Felder `flush_test_haken` `:223`, `test_panik_unter_standlock`
  `:228` mit der Begründung „ohne dort einen zweiten Mutex zu nehmen"
  `:224-228`, `rechen_test_haken` `:244`; Setter `:357-362` und `:381-390`,
  Auslöser mit `take()` vor dem Lauf `:369-378`.
- `broker/src/coordinator/flush.rs`: die D12-Naht feuert **unter** dem
  Standlock `:85-91`; der `flush_test_haken` wird **außerhalb** des Guards
  gezogen und gefeuert `:138-145`.
- `broker/src/store/handle.rs`: `append` = `append_einreihen` plus `recv`
  `:148-151`; der Doc-Kommentar „kein Coordinator-Lock zwischen diesen
  Schritten" `:153-155`; `append_einreihen` `:156-171`; `senden` mit
  `try_send` `:232` und `KanalVoll` `:239-242`; store-eigene Sperren `:234`,
  `:251`.
- `broker/src/store/writer.rs`: ein Writer-Thread über den FIFO `:176-193`,
  Nachschub `aktiv.push_back` `:298`, Commit in derselben Reihenfolge
  `:325-334`, `event_ord = tx.last_insert_rowid()` `:476`, Zuordnung
  `"finding" => ("findings", "finding_id")` `:572`, Projektionsriegel
  `WHERE excluded.last_event_ord > …` `:588-593`.
- `broker/src/coordinator/hypothese/befund.rs`: `finding_id` hasht
  `session_epoch`, Metrik, Band, Quelle und `metrics_version` `:353-373` —
  keine Evidenzmenge.
- `broker/tests/sonde014_verdrahtung.rs`: `coordinator_mit_store` `:948-954`;
  die beiden Fälle, in denen der Rechenhaken auf A's Faden in den Coordinator
  zurückruft, `:575` und `:631`; Projektions- und Logleser
  `user_verdict_ids_im_store` `:3341-3350`, `ereignisse_im_log` `:3353-3361`,
  `zeilen_mit_nutzlast` `:3367-3375`.
- `broker/tests/store_crash_matrix.rs`: `CoordinatorFlushTestHaken` wird von
  außen gesetzt `:1128-1129`, `:1195-1196`; `MAX(event_ord)` als Testmuster
  `:682`, `:1260`.

### 17.6 Was diese Runde nicht geändert hat

- **Keine Regel, kein Rückbau.** R-283-1 bis R-283-6 (§2) stehen unverändert;
  die Kopplung aus der Matrix-Nacharbeit 2 bleibt, wie sie ist. M-75 fügt ihr
  den fehlenden Rotbeweis hinzu, sie ändert sie nicht.
- **Keine zweite Matrixzeile.** Außer M-13 und der neuen M-75 ist keine Zeile
  angefasst; keine wechselt ihre Klasse, keine verliert ihren Beleg.
- **Keine Ticketpfade über §3 hinaus.** `broker/src/coordinator/mod.rs` wird
  nur **ausdrücklich benannt**; es lag schon vorher unter §3 „`broker/src/`
  (Coordinator, …)". `broker/src/store/**` bleibt außerhalb der Etappe 3.
- **Vier offene Punkte für den Dirigenten** (Kopf- und Verlaufspflege, nicht
  Bauersache; alle außerhalb der Ticketgrenze dieser Runde — der Auftrag nennt
  M-13, M-75, §5.6, §6.4, §17 und die Kopfzeile „Etappe"). Sie sind hier
  benannt, nicht kaschiert, und mit **M-75 / 75 IDs / 55 Baulücke** nachzuziehen
  — so wie §12 und §15 es nach den Runden 1 und 2 schon getan haben:
  1. Die Kopfzeile **„Gate"** nennt weiterhin „die Matrixzeilen M-01 bis
     M-74".
  2. **§6.0** nennt die Rotbeweisdateien „`…-M-01.txt` bis `-M-74.txt`".
  3. **§6.1 Schritt 2** zählt die Nebenläufigkeitszeilen ohne M-75 auf und
     nennt als „neue Haken dieser Runde" nur den Persistenzhaken (M-13) und
     den `queues.rs`-Haken (M-20) — der Annahmehaken unter dem Guard gehört
     als dritter dazu.
  4. **§8.1 Feinheit 18** beschreibt den Ablauf von Fenster 3 bereits in Prosa
     („A besteht den Vergleich, hält an, B rechnet, reiht ein und
     persistiert …") und braucht den Zeiger auf **M-75** als die Zeile, die
     ihn misst.
- **Innerhalb der Grenze ist alles nachgezogen:** die Überschrift von §5 und
  ihr Vorspann tragen die Zählungsaussage „M-01 bis M-75" (sie ist dieselbe
  Zahl, die §5.6 und die Kopfzeile „Etappe" nennen, und wäre sonst sofort
  widersprüchlich), §5.6 ist aus den Zeilen neu gezählt, und §6.4 führt M-75 in
  seiner Überschrift, den Ticketpfaden, dem Hakenabsatz, der Testtabelle, der
  Reihenfolge und den Risiken.


## 18. Messung der Matrix-Nacharbeit 3 und Start der Matrixprüfung 4 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `579e8320` (`nakama-nak283-e5e56e9-matrix-nach3`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-matrix-nacharbeit-3-auftrag.txt`; 16:30 bis 16:48 Uhr; Beobachter (Aufsicht NORMAL): Start, Telemetrie gesund, zwei HEAD-Wechsel (eigener Commit, Rebase beim Push auf `d4581829`), Ende; Loop gelöscht, Worker entfernt. |
| Commit | `1446a3bd83ddab3af6f32a2ba750c3f7f4e5f454` „NAK-283 Matrix-Nacharbeit 3: M-75 Rotbeweis nach bestandenem Frischevergleich, M-13 und §6.4 nachgezogen", auf `origin/master`; Basis `e5e56e97` ist Vorfahr; `git status --short` leer. Dazwischen fremd und unberührt: `d4581829` (NAK-286, Laufzeit-Arm erste Fassung, `tools/fl/`, andere Session). |
| Diff | `git show --stat 1446a3bd`: nur `docs/beweise/NAK-283.md`, +353 / −19 (M-13, M-75, §5.6, §6.4, neuer §17). Kein Code berührt. |
| Rundenbilanz | `1446a3bd~1..1446a3bd: Doku 1 Datei(en) +353/-19 — OHNE PRODUKTFORTSCHRITT` (Matrixrunde, zählt null); kumuliert seit `39e18d3f`: nur `docs/` aus diesem Ticket. |
| Selbstbericht des Bauers (§17, nicht als Beweis gezählt) | M-75 neu (Fenster 3, zweiter Haken unter gehaltenem Guard, Rendezvous-Muster, Erfolgskriterium A vor B, Rotbeweis Guard-Freigabe), M-13 auf Fenster 1 und 2 begrenzt, §6.4 mit Quelle belegt, Matrix M-01 bis M-75. |
| Matrixprüfung 4 | Wiederprüfung 3 (Vorlage B): Codex `gpt-6-astra`, Effort **max**, lesend, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-matrix4`); Prüfbereich `git diff e5e56e97...1446a3bd -- docs/beweise/NAK-283.md`, Befundliste = der eine Punkt der Matrixprüfung 3; Auftrag `docs/beweise/roh/NAK-283-matrixpruefung-4-auftrag.txt`; Thread-ID und Urteil folgen in §19. **Rundenbudget ausgeschöpft:** endet diese Wiederprüfung ohne PASS, folgt der Konvergenzentscheid (Skill §3.4). |


## 19. Matrixprüfung 4 (Wiederprüfung 3) — PASS; Abschluss der Etappe 1; Start der Etappe 2 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-matrix4`); Thread `01a09619-243d-7bb2-ab0a-13303cc23863`; Lauf 16:50 bis 16:59 Uhr, Exit 0. |
| Prüfbereich | Wiederprüfung 3 (Vorlage B): `git diff e5e56e97...1446a3bd -- docs/beweise/NAK-283.md` mit dem einen Punkt der Matrixprüfung 3; HEAD `8cd32a9c` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. |
| Auftrag und Urteil | `docs/beweise/roh/NAK-283-matrixpruefung-4-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/NAK-283-matrixpruefung-4-8cd32a9.txt`. |
| Urteil | **PASS** — wörtlich: „Frage 1: ja, auf Matrixebene geschlossen: M-75 spezifiziert den Haken nach bestandenem Vergleich unmittelbar vor der Annahme unter gehaltenem Guard, deadlockfreie Drittfreigabe, Frist/Signal-Zusicherung und den Rotbeweis an Annahmeordnung und B-Projektion; Frage 2: nein, im Fixdiff nichts gebrochen." Geprüft u. a. `broker/src/coordinator/mod.rs:188-208`, `broker/src/store/writer.rs:476`, `:588-593`; aus den Zeilen gezählt: 75 IDs = 55 Baulücken (46 rot, 8 nicht messbar, 1 gemischt) + 20 Regressionswachen. |
| Etappe 1 | **Abgeschlossen auf `1446a3bd`.** Kette: Etappe 1 gebaut `23578268` → Matrixprüfung 1 NEEDS_WORK (13, alle bestätigt) → Nacharbeit 1 `57aeae68` → Matrixprüfung 2 NEEDS_WORK (5, bestätigt; Feinheit zu R-283-1) → Nacharbeit 2 `8f59bccc` → Matrixprüfung 3 NEEDS_WORK (1, bestätigt) → Nacharbeit 3 `1446a3bd` → Matrixprüfung 4 PASS. Drei Nacharbeitsrunden, Budget genau ausgeschöpft, kein Konvergenzentscheid. Die Matrix M-01 bis M-75 (§5) und der Bauplan (§6) sind ab jetzt Spezifikation der Etappen 2 bis 6; eine Abweichung eines Bauers steht begründet im Manifest, nie still im Code. |
| Rundenbilanz | Kumuliert `Matrixrunden 3c5ec9dd → 23578268 → 57aeae68 → 8f59bccc → 1446a3bd: je Runde nur docs/beweise/NAK-283.md, OHNE PRODUKTFORTSCHRITT (die Produkt- und Werkzeugzeilen in den Rundenbereichen stammen aus fremden Commits der Session nakama-a7: NAK-284, NAK-285, NAK-286); Konvergenz-Signal des Werkzeugs ist für Matrixrunden erwartet` — Matrixrunden zählen null; Produktfortschritt beginnt mit Etappe 2. |
| Etappe 2 | Frischer Opus-Worker (max), Auftrag `docs/beweise/roh/NAK-283-etappe-2-auftrag.txt` (Ticketpfade nach §3 samt Nachtrag und §6.3; Tests und Beine B14, B27, B2 aus §6.3; Rotbeweise nach §6.1; kein Kanon, kein Laufzeit-Arm — beides am Ende der Etappe 6). Aufsicht ENG (State, Persistenz). Danach Erstprüfung 2 durch Codex Astra max (Vorlage A über den Etappendiff). |
| Nebenläufig | Die Session `nakama-a7` hat parallel NAK-284 (Diagnoseprojekt), NAK-285 (Installation über erhöhte Aufgaben) und NAK-286 (Laufzeit-Arm, erste Fassung `d4581829`) geliefert; der Dirigent hat die vier Skill-Einfügungen aus `docs/gesundheit/skill-nachtrag-laufzeit-arm-2026-09-12.md` in diesem Fenster übernommen (Register NAK-286). Der Laufzeit-Arm läuft für NAK-283 nach dem grünen Kanon am Ende der Etappe 6. |


## 20. Bauetappe 2 — Plugin-Publikation und State (Bauer, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **2 von 6** (F01, F11). Basis `3cfa4176840d39629217c3844302cdadbaaf3dea`, `git status --short` beim Start **leer**. **Produktcode und Tests liegen auf `3606b0e9`** (Commit 1 der Etappe); Beweise, Skripte und dieser Abschnitt folgen im zweiten Commit. Auftrag `docs/beweise/roh/NAK-283-etappe-2-auftrag.txt` (§19). Spezifikation: die geprüfte Matrix §5.1 und der Bauplan §6.3. |
| Nebenläufig | Während des Baus hat der Dirigent im selben Checkout drei Commits gelegt (`93863296`, `4adbb0f7`, `bb49a612`); `3606b0e9` sitzt deshalb auf `bb49a612`. `git diff --stat 3cfa4176..bb49a612 -- eq-copilot broker` ist **leer** — die drei berühren nur `CLAUDE.md`, den Dirigenten-Skill, `docs/offene-punkte.md` und `tools/fl/LIES-MICH.md`. Der gemessene Produktcode ist der des Basis-SHA; kein Rotbeweis und kein Bein ist davon berührt. |
| Umfang | **14 Matrixzeilen** gebaut und gemessen: M-01 bis M-12, M-71, M-72; dazu die Übergangshälften M-26 (Hälfte 1, C++-Seite) und M-70 (Hälfte 1, C++-Seite). Zehn Dateien geändert, +1 493 / −98 Zeilen. |
| Beweise | **16 Rotbeweise** nach §6.1, jeder rot → bytegleiche Rücknahme (SHA-256 vorher == nachher) → grün; **12 Beine** einzeln gefahren, alle Exit 0; keine neue `warning C`. Kein Kanon, kein Laufzeit-Arm (beides am Ende der Etappe 6). |
| Abweichungen | **Sechs**, alle in §20.6 mit Grund und Quelle. Keine Matrixzeile geändert — die Prüfung entscheidet. |
| Produktfragen | Keine beantwortet. §8.3 ist unverändert. |

### 20.1 Was gebaut wurde

**F01 — die Ordnung der Mitgliederstände innerhalb einer Reload-Generation.**
Das Modell führte bis hierher nur `reloadGeneration`, einen Epochenzähler: er
ordnet Projekte, nicht die Mitgliederstände *innerhalb* eines Projekts. Zwei
Publikationen derselben Generation — ein Worker-Drain und ein Handgriff auf dem
Message-Thread — trugen dieselbe Zahl, und die später eintreffende gewann, auch
wenn ihr Stand älter war. Neu ist die **Folgenummer der Mitgliederstände**:

- `SourcesModel.h` / `.cpp`: das Modell führt `zuletztUebernommeneFolge` neben
  `reloadGeneration`; `setzePersistenteMitglieder` nimmt die Folgenummer als
  dritten Parameter und entscheidet unter demselben `mutex` in dieser
  Reihenfolge — Generationsvergleich (`:373`, erste Entscheidung, M-04),
  Folgenummernvergleich (`:383`, zweite Entscheidung, M-01), **Übernahme der
  Nummer** (`:395`), erst danach der frühe Ausstieg bei Inhaltsgleichheit
  (`:396-397`, M-05). Der Rückgabetyp ist dreiwertig (`Publikation`:
  übernommen, überholt, Reload abgewiesen — §8.1 Feinheit 19), und je
  Ablehnungsgrund führt das Modell einen eigenen Zähler. `projektReload` setzt
  die Folge **nicht** zurück — der Zähler im Prozessor läuft
  generationsübergreifend monoton, jede Publikation nach dem Reload trägt
  ohnehin eine größere Nummer, und jede davor fällt schon am
  Generationsvergleich (Kommentar an der Stelle).
- `PluginProcessor.h`: `sourcesMitgliederFolge` (`std::uint64_t`, nur unter
  `bindungMutex`), `sourcesPublikationUeberholt` (`std::atomic`), die
  Deklarationen von `naechsteSourcesFolgeUnterBindung` und
  `werteSourcesPublikationAus` sowie fünf Testzugänge.
- `Ipc.cpp`: `naechsteSourcesFolgeUnterBindung` (`:1446`) **sättigt** am
  Anschlag statt umzulaufen (M-71); `werteSourcesPublikationAus` (`:1462`) ist
  die eine Auswertung für alle vier Produktaufrufer und trennt die beiden
  Ablehnungsgründe: der Überholtfall meldet Host-Dirty und erhöht die Revision
  wie im Erfolgsfall (seine Änderung ist unter `bindungMutex` angewandt), nur
  der Reloadfall steigt vor beidem aus (M-72). Alle drei Publikationswege
  (`:1232`, `:1276`, `:1528`) ziehen ihre Folgenummer **im selben
  `bindungMutex`-Block wie ihre Kopie**.
- `State.cpp`: nur `:347` — der Aufruf, seine Signaturfolge (`folge` im selben
  Block) und der Kommentar, der „kein Dirty, keine Revision" jetzt ausdrücklich
  auf den Reloadfall eingrenzt. Kein weiteres Verhalten dieser Datei.

**F11 — die Revisionsränder.** In den drei Entfern-Handgriffen stand das
`erase` vor dem Riegel: an der Obergrenze war der Eintrag entfernt, der
Handgriff meldete `false` und `veraendert == false` — eine persistente Änderung
ohne Dirty und ohne Revision. Die Assistentenrevision hatte keine obere
Schranke; `a.revision += 1` an `int64max` ist signed-integer-UB, und der
erzeugte negative Wert wird vom eigenen Reader abgewiesen.

- `NakamaState.cpp`: `entferneIntent` (`:2295-2303`), `entferneSchutzangabe`
  (`:2360-2368`) und `entferneBeziehung` (`:2471-2482`) fragen erst, ob
  überhaupt etwas passt, dann läuft der Riegel, dann die Mutation. Der
  No-op-Pfad bleibt unverändert (nichts gefunden → `true`, keine Revision,
  kein Dirty). Kein Rollback (§8.1 Feinheit 12).
- `NakamaState.cpp`: `assistentenrevisionHeben` (`:2677`) ist neu und steht vor
  jeder Zuweisung an allen **drei** Inkrementstellen — `schrittAendern`
  (`:2703`), der neue Schritt in `setzeAssistentenschritt` (`:2751`) und
  `setzeAssistentenergebnis` (`:2856`). Der Reader-Riegel `revision < 1` bleibt
  unverändert.

**Kein Schemawechsel, kein neues persistentes Feld.** Die Folgenummer lebt im
Prozess, nicht auf der Leitung; Discriminator, Revision und Capability der
Verträge sind unberührt; das Layout des gespeicherten Zustands ist unverändert
(belegt durch B2 mit 287 Prüfungen und A12 bytegleich).

### 20.2 Je Matrixzeile: Test und Rotbeweis

Alle Rotbeweise nach §6.1: SHA-256 der unveränderten Quelle, Mutation an der
Zusagezeile, Zeitstempel, Bein rot, **bytegleiche Rücknahme der Originalbytes**,
Zeitstempel, Bein grün. Das Skript spielt die gesicherten Bytes zurück, nie eine
zweite Textersetzung — die Gleichheit ist strukturell, der Hash belegt sie.
Skript `docs/beweise/roh/NAK-283-etappe-2-rotskript.ps1`.

| Zeile | Test (Datei, Name) | Bein | Rotbeweis: Mutation | Rohdatei | rot / grün |
|---|---|---|---|---|---|
| M-01 (a) | `Sonde012ProjectReloadTest.cpp:1685` `aeltere_mitgliederpublikation_ersetzt_keine_juengere` | B14 | `SourcesModel.cpp:383-387`: Folgenummernprüfung entfernt | `NAK-283-rot-M-01a-etappe-2.txt` | Exit 1 / Exit 0 |
| M-01 (b) | derselbe Fall, Dirty- und Revisionshälfte | B14 | `Ipc.cpp:1477-1478`: Überholt-Ausstieg auf den Reload-Zweig | `NAK-283-rot-M-01b-etappe-2.txt` | Exit 1 / Exit 0 |
| M-02 | `:1764` `hauptziel_benennung_wird_nicht_von_aelterer_workerkopie_ueberholt` | B14 | wie M-01 (a) | `NAK-283-rot-M-02-etappe-2.txt` | Exit 1 / Exit 0 |
| M-03 | `:1811` `lokaler_unbind_wird_nicht_von_aelterer_workerkopie_ueberholt` | B14 | wie M-01 (a) | `NAK-283-rot-M-03-etappe-2.txt` | Exit 1 / Exit 0 |
| M-04 | **Regressionswache**: `gezogene_kopie_ueberholt_den_reload_nicht` (M-39), um den Folgenummernpfad erweitert (`:1524-1541`) | B14 | `SourcesModel.cpp:373-377`: Generationsvergleich entfernt — ein Riegel, der nur noch die Folgenummer prüft | `NAK-283-rot-M-04-etappe-2.txt` | Exit 1 / Exit 0 |
| M-05 | `:1863` `inhaltsgleiche_publikation_zieht_die_frischemarke_nach` | B14 | `SourcesModel.cpp:395-397`: früher `return` bei Gleichheit **vor** die Übernahme der Nummer | `NAK-283-rot-M-05-etappe-2.txt` | Exit 1 / Exit 0 |
| M-06 | `:1925` `state_und_modell_sind_nach_ruhe_gleich` | B14 | wie M-01 (a) | `NAK-283-rot-M-06-etappe-2.txt` | Exit 1 / Exit 0 |
| M-07 | `Sonde014IntentTest.cpp:1303` `m283_07_bis_09`, Block M-07 | B27 | `NakamaState.cpp:2295-2303`: Revisionsprüfung hinter das `erase` zurück | `NAK-283-rot-M-07-etappe-2.txt` | Exit 1 / Exit 0 |
| M-08 | derselbe Fall, Block M-08 | B27 | dieselbe Rückverlegung in `entferneSchutzangabe` | `NAK-283-rot-M-08-etappe-2.txt` | Exit 1 / Exit 0 |
| M-09 | derselbe Fall, Block M-09 | B27 | dieselbe Rückverlegung in `entferneBeziehung` | `NAK-283-rot-M-09-etappe-2.txt` | Exit 1 / Exit 0 |
| M-10 | `Sonde014IntentTest.cpp:1398` `m283_10` (alle drei Inkrementstellen) | B27 | `NakamaState.cpp:2677-2683`: obere Schranke entfernt | `NAK-283-rot-M-10-etappe-2.txt` | Exit 1 / Exit 0 |
| M-11 (a) | `StateMigrationTestMain.cpp:2343` `revisionsmaximum_ueberlebt_save_load`, Hälfte (a) | B2 | wie M-10 | `NAK-283-rot-M-11a-etappe-2.txt` | Exit 1 / Exit 0 |
| M-11 (b) | derselbe Fall, Hälfte (b) | B2 | wie M-07 | `NAK-283-rot-M-11b-etappe-2.txt` | Exit 1 / Exit 0 |
| M-12 | **Regressionswache**: `Sonde014IntentTest.cpp:1470` `m283_12` (vier Handgriffe plus Produktpfad) | B27 | `NakamaState.cpp:2209`: Bestandsschranke auf `int64max − 1` vorgezogen | `NAK-283-rot-M-12-etappe-2.txt` | Exit 1 / Exit 0 |
| M-71 | `Sonde012ProjectReloadTest.cpp:1995` `mitgliederfolge_wrappt_nicht` | B14 | `Ipc.cpp:1456-1457`: `++folge` ohne obere Schranke | `NAK-283-rot-M-71-etappe-2.txt` | Exit 1 / Exit 0 |
| M-72 | `:2047` `reloadablehnung_und_ueberholung_sind_unterscheidbar` (zwei Phasen) | B14 | wie M-01 (b) | `NAK-283-rot-M-72-etappe-2.txt` | Exit 1 / Exit 0 |

**Übergangszeilen, Hälfte 1.** **M-26** (F01 + F02, C++-Seite): die Regel
„Kopie unter Sperre, Rückschreiben mit Frischeprüfung" ist in C++ gebaut und
mit M-01 einmal absichtlich gebrochen (`NAK-283-rot-M-01a-etappe-2.txt`); die
Rust-Hälfte folgt in Etappe 3. **M-70** (F11 + F13, C++-Seite): „ein Scheitern
ist an einem eigenen Status erkennbar, nie an einem Wert aus dem
Erfolgsbereich" ist in C++ gebaut — Ablehnung ohne Mutation plus gesetzter
`grund` (M-07 bis M-09); die Python-Hälfte folgt in Etappe 6. Beide Zeilen
gelten erst als geschlossen, wenn **beide** Hälften stehen.

**Die vier Hashes** (unverändert vor und nach jedem Rotbeweis; gemessen am
Etappenstand):

| Datei | SHA-256 |
|---|---|
| `eq-copilot/plugin/src/SourcesModel.cpp` | `520A727382AAF6C0A8E1AA65D9BA973CC9E87B75825C8D2EB16CEDCB412373B3` |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | `B74AC525AD97CEEC36A0E0F56AA24599ECF7782B617164845A2B562C947B4B97` |
| `eq-copilot/plugin/state/NakamaState.cpp` | `A947AD28064B082C3DCF69E03DDF60E00539DBA6B43B9BE384D7993D88F4D8E9` |
| `eq-copilot/plugin/src/prozessor/State.cpp` | `C9A136D91E8B216B5B4FC8C266E28514D1CF8971F08D88D9E444A49A54DC158C` |

`State.cpp` trägt keinen eigenen Rotbeweis: ihre Etappenänderung ist die
Signaturfolge zu `:347`, und die Zusagen, die dort fallen, sind M-04 (gemessen
an `SourcesModel.cpp`) und M-72 (gemessen an `Ipc.cpp`).

### 20.3 Geänderte Dateien

| Datei | Zeilen (+/−) | Was |
|---|---:|---|
| `eq-copilot/plugin/src/SourcesModel.h` | +75 / −13 | dreiwertiger Rückgabetyp, dritter Parameter, zwei Zählerzugänge, `persistenteMitgliederKopie`, drei neue Felder |
| `eq-copilot/plugin/src/SourcesModel.cpp` | +56 / −5 | der dreistufige Riegel (`:361-398`), die drei Zugänge, der Reload-Kommentar |
| `eq-copilot/plugin/src/PluginProcessor.h` | +67 / −1 | `sourcesMitgliederFolge`, `sourcesPublikationUeberholt`, zwei Methodendeklarationen, fünf Testzugänge |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | +109 / −39 | `naechsteSourcesFolgeUnterBindung` (`:1446`), `werteSourcesPublikationAus` (`:1462`), drei Publikationswege, zwei Testzugänge, drei nachgezogene Kommentare |
| `eq-copilot/plugin/src/prozessor/State.cpp` | +13 / −7 | nur `:347` samt Signaturfolge und Kommentar |
| `eq-copilot/plugin/state/NakamaState.cpp` | +74 / −26 | drei Entfern-Handgriffe, `assistentenrevisionHeben`, drei Inkrementstellen, vier Aufrufer von `schrittAendern` |
| `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` | +618 / −2 | sieben neue Fälle, M-04-Erweiterung im M-39-Fall, Fallregister und Vollauf |
| `eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp` | +9 / −3 | nur die neue Signatur (drei Aufrufe) |
| `eq-copilot/plugin/tests/Sonde014IntentTest.cpp` | +358 / −2 | drei neue Fälle, `pruefe` um ein optionales Detail erweitert |
| `eq-copilot/plugin/tests/StateMigrationTestMain.cpp` | +114 / −0 | M-11 als eigener Abschnitt |

Nicht angefasst: `broker/`, `eq-copilot/schemas/**`, `eq-copilot/plugin/sonde/`,
`eq-copilot/plugin/dsp/`, `eq-copilot/plugin/core/analysis/`,
`eq-copilot/identity/`, `eq-copilot/install/`, alle anderen Tests, alle anderen
Manifeste, `docs/PLAN-STAND.md`, `docs/offene-punkte.md`, `design/`.
`eq-copilot/plugin/CMakeLists.txt` und `tools/beweise.ps1` blieben unberührt —
kein Test und kein Bein brauchte eine Änderung dort. Keine Fixture wurde
geschrieben; A12 belegt den Bestand bytegleich.

### 20.4 Gefahrene Beine

Skript `docs/beweise/roh/NAK-283-etappe-2-beinskript.ps1`, Rohausgabe
`docs/beweise/roh/NAK-283-etappe-2-beine.txt`. Gebaut aus pwsh (Release,
`eq-copilot/build`); die Binary-Zeitstempel sind gegen die jüngste geänderte
Quelle geprüft (alle **frisch**, NAK-230).

| Bein | Ziel | Exit | Messung |
|---|---|---:|---|
| A1 | `EqCopNullTest` | 0 | 57 Prüfungen |
| A16 | `EqCopProbeeqNullTest` | 0 | Nulltest und Passthrough bitidentisch |
| B1 | `EqCopIdentityTest` | 0 | 120 Prüfungen |
| B2 | `EqCopStateMigrationTest` | 0 | 287 Prüfungen (vorher 276; M-11 bringt 11) |
| B4 | `EqCopQueueStressTest` | 0 | Passthrough, 0 Allokationen im Audiothread |
| B8 | `EqCopLebenslaufTest` | 0 | 79 Prüfungen |
| B10 | `EqCopIpcTest` | 0 | v3-Vertrag und In-Flight-Register |
| B13 | `EqCopSonde012SourcesModelTest` | 0 | 88/88 (der Testpfad mit neuer Signatur) |
| B14 | `EqCopSonde012ProjectReloadTest` | 0 | 219/219 (vorher 149; M-01 bis M-06, M-71, M-72 bringen 70) |
| B27 | `EqCopSonde014IntentTest` | 0 | 254 Prüfungen (vorher 210; M-07 bis M-10, M-12 bringen 44) |
| B29 | `EqCopSonde014AssistentTest` | 0 | 193/193 (die Zustandsmaschine gegen die neue `schrittAendern`-Signatur) |
| A12 | `erzeuge_state_fixtures.py --pruefen` | 0 | Statekorpus und MANIFEST bytegleich, keine verwaiste Datei |

Zusätzlich **gebaut, nicht gefahren**: `EqCopilot_VST3` (das Bundle) und
`EqCopSessionSoak`. Der Bau meldet **zehn** `warning C` — alle dieselbe
vorbestehende `C4458` aus `core/analysis/featureengine/Stereo.h:70`, einmal je
Projekt; **keine neue Warnung, keine aus einer geänderten Datei**.

**Nulltest und Passthrough:** A1 und A16 übersetzen `NakamaState.cpp` und
`PluginProcessor.h` mit und sind grün — der Nulltest bleibt bitidentisch, der
Passthrough sampleidentisch. Kein Pfad dieser Etappe fügt Verarbeitung hinzu;
`bindungMutex` wird nie im Audiothread genommen, und `meldeHostDirty` läuft
unverändert außerhalb jeder eigenen Prozessorsperre.

### 20.5 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Abschnitt | Wo gemessen |
|---|---|
| **A** — Zähler einer Politik werden ausgewertet, nicht ignoriert | Alle vier neuen Zähler werden gemessen: `sourcesPublikationUeberholt` und `sourcesNachfuehrungNachReloadUnterblieben` in M-01, M-04, M-71, M-72; die beiden Modellzähler in M-04 und M-72. Keiner ist eine Behauptung ohne Messung. |
| **A** — dieselbe Regel in allen Sprachen | M-26 Hälfte 1: die C++-Seite ist gebaut und einmal gebrochen; die Rust-Seite ist Etappe 3. Die Zeile ist als Hälfte geführt, nicht als geschlossen. |
| **B** — Lebenszyklus | Der Publikationshaken lebt im Prozessor und kann bis zu dessen Destruktor aus dem Workerzug feuern. Die Testbühne deklariert Schranke und Zähler **vor** dem Prozessor, damit sie ihn überleben (umgekehrte Zerstörungsreihenfolge). |
| **C** — Zahlenränder | `std::uint64_t` am Anschlag (M-71, Sättigung statt Wrap), `int64max` und `int64max − 1` an vier Handgriffen (M-07 bis M-12). Kein Wert reist über den Draht: die Folgenummer lebt im Prozess. |
| **D** — Bau- und Prüfriegel | Beinskript und Rotskript prüfen die Binary-Zeitstempel gegen die Quellen und brechen ab, wenn ein Binary älter ist (NAK-230). Das Rotskript bricht ab, wenn das Mutationsmuster fehlt oder mehr als einmal vorkommt. |
| **E** — jede neue Prüfung einmal absichtlich gebrochen | 16 Rotbeweise, jeder mit Rohausgabe, SHA-256-Paar und beiden Exitcodes. Die zwei Regressionswachen (M-04, M-12) sind als solche benannt und **sind kein Beleg für ihren Befund** — die Belege liegen bei M-01 bis M-03, M-05, M-06 (F01) und M-07 bis M-11 (F11). |
| **E** — Behauptung ≤ Messung | M-71 sagt nur zu, was gemessen wird: kein Rücksprung des Zählers, Abweisung, Überholt-Zähler, Bestand bleibt. Die Aussage „eine ältere Publikation gewinnt nach einem Wrap wieder" steht **nicht** in der Zeile, weil der Fall am Anschlag nicht mehr geordnet, sondern angehalten wird. |
| **E** — Zahlen gemessen, nicht abgeschrieben | Alle Prüfzahlen in §20.4 stammen aus `NAK-283-etappe-2-beine.txt`; die Zeilenzahlen in §20.3 aus `git diff --numstat`. |
| **F** — speichern↔laden im selben Änderungssatz | M-11 misst Save **und** Load in einer neuen Instanz, und zwar am geladenen Zustand, nicht am Rückgabewert. A12 belegt, dass keine Fixture wandert. |
| **F** — Writer, Reader, Fixtures im selben Satz | Jeder Randzustand entsteht als **deklarierter Mutant eines Writer-Standes** mit genau einer bzw. zwei benannten Abweichungen, nie von Hand gesetzt. |
| **Dirty je persistenter Änderung** | M-01 (Delta 2 statt 1), M-05 (Delta 2 trotz inhaltsgleicher Publikation), M-11 (kein Dirty bei abgewiesenem Handgriff), M-12 Produktpfad (genau eine Meldung, folgenloser Aufruf schweigt), M-72 (nur der Reloadfall unterdrückt). |
| **Stale Closures** | Der Haken der Testbühne fängt `this`; die Struktur überlebt den Prozessor (siehe B). Kein Lambda hält einen Zeiger auf einen Wert, der früher stirbt. |
| **Sperrenordnung** (Lehre 12.09.2026) | Die Folgenummer wird **nur** unter `bindungMutex` vergeben und gelesen — unter derselben Sperre, unter der der Stand entsteht und die Kopie gezogen wird. Zuordnung und Operation liegen damit unter einer Sperre; eine Vergabe davor oder danach war genau die Lücke, die F01 beschreibt. `meldeHostDirty` bleibt außerhalb jeder eigenen Prozessorsperre. |
| **MSVC-Stack 1 MiB** | Alle Prozessorobjekte der neuen Fälle liegen auf dem Heap (`std::make_unique`), NAK-175. |

### 20.6 Abweichungen vom Bauplan §6.3

Sechs; keine ändert eine Matrixzeile. Die Prüfung entscheidet.

1. **`eq-copilot/plugin/src/PluginProcessor.h` ist geändert, §3 und §6.3 nennen
   ihn nicht.** Grund: `Ipc.cpp` und `State.cpp` implementieren Methoden von
   `EqCopilotProcessor`, und die Klasse wird in diesem Header deklariert. Ohne
   ihn gibt es weder das Feld `sourcesMitgliederFolge` noch die beiden neuen
   Methoden noch die Testzugänge, die M-71 und M-72 messen — die Änderung ist
   technisch unvermeidbar, sobald `Ipc.cpp` eine neue Methode bekommt. Der
   Umfang ist eng: zwei Felder, zwei Deklarationen, fünf Testzugänge, kein
   bestehendes Verhalten.
2. **Drei Testaufrufe statt einem in `Sonde012SourcesModelTest.cpp`.** §8.2
   Punkt 2 nennt „ein Testpfad (`Sonde012SourcesModelTest.cpp:342`)". Gemessen
   sind **drei** Aufrufe (`:342`, `:417`, `:712` zum Basisstand); alle drei
   bekamen die neue Signatur mit Folgenummer 1, jeder Fall baut ein frisches
   Modell mit zuletzt übernommener Nummer 0. Die Ticketgrenze („nur für die
   neue Signatur") deckt das; die Zusagen dieser Zeilen sind unverändert.
3. **Drei Beine mehr als §6.3 nennt: B1, B4, B29.** Sie rufen
   `assistentStarten` / `assistentWeiter` / `assistentAbbrechen` und messen
   damit die geänderte Signatur von `schrittAendern` — ein Bein, das eine
   geänderte Quelle nicht nur übersetzt, sondern ihre Zusage misst, gehört in
   die Etappe. Alle drei Exit 0. `EqCopSessionSoak` bindet `SourcesModel.cpp`
   und ist mitgebaut, aber nicht gefahren: sein Lauf ist ein Soak und gehört in
   den Kanon am Ende der Etappe 6.
4. **M-71: die Sättigung liegt im Prozessor, nicht im Modell.** Die Zeile sagt
   „das Modell hat die Folgenummer am Anschlag (im Test gesetzt)". Gebaut ist
   die Sättigung bei der **Vergabe** (`Ipc.cpp:1456`), weil die Nummer unter
   `bindungMutex` entsteht und nur dort angehalten werden kann, ohne eine
   zweite Ordnung neben der bestehenden zu erzeugen; das Modell weist eine
   nicht größere Nummer als überholt ab. Der Test setzt deshalb den
   **Prozessorzähler** auf `2^64 − 2` und erreicht den Anschlag über den echten
   Weg (ziehen, publizieren, übernehmen, abweisen) statt über einen gesetzten
   Modellzustand. Die vier zugesagten Größen sind unverändert gemessen: kein
   Rücksprung, Abweisung, Überholt-Zähler, Bestand bleibt.
5. **Vier Zähler statt zwei.** §6.3 verlangt „je Ablehnungsgrund führt das
   Modell einen eigenen Zähler", M-72 misst
   `sourcesNachfuehrungNachReloadUnterblieben` gegen „den Überholt-Zähler" —
   und der bestehende Reload-Zähler liegt im **Prozessor**. Gebaut sind
   deshalb beide Ebenen. Sie sind nicht redundant: der Prozessor-Reload-Zähler
   zählt auch den frühen Ausstieg in `meldeSourcesMitgliederNachBefehl`, der
   das Modell nie erreicht. M-72 misst alle vier; keiner ist eine Behauptung
   ohne Messung.
6. **M-72 fährt zwei Phasen mit je eigenem Prozessor.** Die Zeile sagt „beide
   Ablehnungsgründe in einem Lauf". Gebaut ist ein Testfall mit zwei Phasen
   (Reload, Überholung), jede mit eigenem Prozessor. Grund: „keiner steigt im
   Fall des anderen" ist so direkt messbar; eine verschränkte Bühne müsste
   beide Gründe im selben Prozessor auslösen und könnte die Zuordnung nur noch
   über Differenzen behaupten.

Drei kleinere Präzisierungen ohne Abweichungscharakter:

- **Die Dateinamen folgen dem Etappenauftrag, nicht §6.0.** §6.0 nennt
  `NAK-283-rot-M-01.txt` bis `-M-74.txt`, `NAK-283-etappe2-beine.txt` und
  `NAK-283-rotskript-etappe2.ps1`. Der Etappenauftrag vom 12.09.2026 (§19)
  schreibt stattdessen `NAK-283-rot-M-nn-etappe-2.txt`,
  `NAK-283-etappe-2-beine.txt` und `NAK-283-etappe-2-rotskript.ps1` vor; die
  jüngere Anweisung gilt. Zwei Zeilen tragen zusätzlich ein Halbsuffix (`-M-01a`
  und `-M-01b`, `-M-11a` und `-M-11b`), weil diese beiden Matrixzeilen je zwei
  Hälften mit **verschiedenen** Mutationen haben und eine gemeinsame Rohdatei
  den Beleg vermischte. §6.0 bleibt als Verlauf unangetastet.
- Die Fallnamen des neuen B14-Registers tragen das Präfix `283` (`283m01` und
  weitere), weil NAK-246 und NAK-283 je ein `m06` führen und ein Rotbeweis
  sonst den falschen Fall führe.
- `pruefe` in `Sonde014IntentTest.cpp` hat ein optionales drittes Argument für
  den gemessenen Wert bekommen (additiv, alle bestehenden Aufrufe unverändert,
  dieselbe Form wie in B14).

### 20.7 Offene Punkte dieser Etappe

1. **M-26 und M-70 sind je zur Hälfte gebaut.** M-26 braucht die Rust-Seite
   (Etappe 3, M-13), M-70 die Python-Seite (Etappe 6, M-61/M-62). Beide Zeilen
   gelten erst danach als geschlossen; sie stehen hier als Hälfte, nicht als
   Erfolg.
2. **Der fünfte Publikationsweg aus M-06 liegt in einem anderen Bein.** Die
   Zeile nennt „fünf Publikationswege (… Testpfad)". Der Testpfad ist der
   direkte Modellaufruf in `Sonde012SourcesModelTest.cpp` und kann in B14 nicht
   gefahren werden; der Fall misst die **vier Produktwege** in einem Lauf, und
   der Testpfad ist in B13 mit der neuen Signatur grün. §8.2 Punkt 2 führt ihn
   ausdrücklich als „keine eigene Zusage".
3. **Kein Kanon, kein Laufzeit-Arm** — auftragsgemäß am Ende der Etappe 6.
   `docs/PLAN-STAND.md` ist nicht angefasst, `planstand.py` nicht gelaufen.
4. **Nebenbefund, nicht im Register** (außerhalb des Auftrags, nichts
   geändert): der Kommentarkopf von `meldeSourcesMitgliederNachBefehl`
   behauptete bis zu dieser Etappe „zwei Drains … reichen dem Modell so nie
   einen aelteren Stand nach" — genau die Aussage, die F01 widerlegt. Sie ist
   jetzt an Ort und Stelle richtiggestellt; dieselbe Form von Selbstzusage
   („der Kommentar sagt mehr, als der Code hält") kann an weiteren Stellen
   stehen und wäre eine eigene Suche wert.


## 21. Messung der Etappe 2 und Start der Erstprüfung 2 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `0c08b136` (`nakama-nak283-3cfa417-etappe2`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-2-auftrag.txt` (§19); Start 17:04 Uhr, letzter Commit 17:55 Uhr, Ende danach. Dirigentensession bei 17:15 Uhr über den Marker neu gestartet (`bb49a612`); Beobachter um 17:20 Uhr neu gesetzt (Aufsicht ENG): Start, Telemetrie gesund, drei HEAD-Wechsel, Ende; Stundenloop `9d96a9f1` gelöscht, `CronList` leer. |
| Commits | `3606b0e9` (Produkt und Tests, 17:54 Uhr), `bead17e2` (Rotbeweise, Skripte, Beine, §20), `988dc44d` (Nebenläufigkeit im Kopf, 17:55 Uhr), alle auf `origin/master`; Basis `3cfa4176` ist Vorfahr; `git status --short` leer. Dazwischen eigene, unberührte Dirigentencommits `93863296`, `4adbb0f7`, `bb49a612` (nur `CLAUDE.md`, Skill, Register, `tools/fl/LIES-MICH.md`); `git diff --stat 3cfa4176..bb49a612 -- eq-copilot broker` leer, der gebaute Produktcode ist der des Basis-SHA. |
| Diff | `git diff --stat bb49a612..988dc44d`: 30 Dateien, +9 076 / −100 — sechs Produktdateien (+394/−91), vier Testdateien (+1 099/−7), Manifest §20 und 19 Rohdateien. Außerhalb der §3-Pfade nur `PluginProcessor.h` (Abweichung 1 in §20.6: die Klasse ist dort deklariert — Felder, zwei Deklarationen, Testzugänge, kein bestehendes Verhalten); `State.cpp` nur `setzeBindung` um `:347` (Signaturfolge `folge` im selben `bindungMutex`-Block, Aufruf, Kommentar). Broker, Schemas, Sonde, DSP, Analyse, Identität, Installer unberührt; `CMakeLists.txt` und `tools/beweise.ps1` unverändert. |
| Rundenbilanz | `bb49a612..988dc44d: Produkt 6 Datei(en) +394/-91 \| Tests 4 Datei(en) +1099/-7 \| Doku 20 Datei(en) +7583/-2` — die erste Runde dieses Tickets mit Produkt- und Testzeilen; kumuliert seit `39e18d3f` sind die Matrixrunden null (§19) und diese Etappe der gesamte Produktfortschritt. |
| Eigene Messung | Binaries frischer als Quellen (jüngste geänderte Quelle `NakamaState.cpp` 17:38:16 Uhr, ältestes gefahrenes Binary `EqCopNullTest.exe` 17:39:25 Uhr). Vom Dirigenten am Stand `988dc44d` um 17:57 Uhr selbst gefahren: B14 `EqCopSonde012ProjectReloadTest` 219/219 Exit 0; B27 `EqCopSonde014IntentTest` 254 Prüfungen Exit 0; B2 `EqCopStateMigrationTest` 287 Prüfungen Exit 0; B13 `EqCopSonde012SourcesModelTest` 88/88 Exit 0 — identisch mit `NAK-283-etappe-2-beine.txt` (dort zwölf Beine, alle Exit 0, zehn vorbestehende C4458 aus `Stereo.h:70`, keine neue Warnung). Rotbeweise stichprobenartig gelesen: M-01a rot 9/11 genau an „nach Ruhe trägt das Modell GENAU [A]" und am Überholt-Zähler (0/0), grün 11/11 nach bytegleicher Rücknahme, SHA-256 vorher = nachher; M-11a grün 287/287 nach Rücknahme, Hälfte (b) misst die Bytes (497 = 497) und den geladenen Eintrag der neuen Instanz. |
| Messabdeckung | 14 Matrixzeilen je mit Test und Rotbeweis (§20.2, 16 Rohdateien: M-01 und M-11 je zwei Hälften mit verschiedenen Mutationen); M-04 und M-12 als Regressionswachen absichtlich gebrochen und als solche benannt; M-26 und M-70 je Hälfte 1 (Rust in Etappe 3, Python in Etappe 6). Keine Zeile ohne Rotbeweis, kein Fortsetzungsauftrag nötig. |
| Abweichungen (Einordnung) | Sechs aus §20.6, an Diff und Quelle geprüft: 1 (`PluginProcessor.h`) und 2 (drei Testaufrufe statt einem) sind die Ticketgrenze in engster Form; 3 (B1, B4, B29 zusätzlich) und 5 (vier Zähler statt zwei) sind Mehrmessung; 4 (Sättigung bei der Vergabe unter `bindungMutex`, `Ipc.cpp` `naechsteSourcesFolgeUnterBindung`, statt im Modell) und 6 (M-72 in zwei Phasen mit je eigenem Prozessor) ändern die Zusage nicht — die vier Größen von M-71 und die Trennbarkeit beider Zählerpaare von M-72 sind gemessen. Dazu: `zuletztUebernommeneFolge` wird in `projektReload` bewusst nicht zurückgesetzt (Prozessorfolge läuft generationsübergreifend monoton; ältere Nummern fallen am Generationsvergleich). Keine ist ein Befund des Dirigenten; der Prüfauftrag nennt sie als gedeckt und lässt den Prüfer die Gründe an der Quelle prüfen. |
| Selbstbericht des Bauers (§20, nicht als Beweis gezählt) | Folgenummer der Mitgliederstände als zweite Entscheidung unter dem Modell-`mutex`, Nummernvergabe vor Inhaltsvergleich (M-05), dreiwertiger Rückgabewert `Publikation` mit einer Auswertung `werteSourcesPublikationAus` für alle vier Produktaufrufer (Überholt meldet Dirty und Revision, Reload nicht), Sättigung statt Wrap (M-71); drei Entfern-Handgriffe fragen → Riegel → Mutation, `assistentenrevisionHeben` vor allen drei Inkrementstellen, `schrittAendern` liefert `bool` mit `grund`; kein Schemawechsel, kein neues persistentes Feld. |
| Erstprüfung 2 | Vorlage A: Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`tools/dirigent/codex-lauf.ps1`, Kennung `nak283-etappe2-erst`); Prüfbereich `git diff 3cfa4176...988dc44d -- <die zehn Ticketpfade>`; Auftrag `docs/beweise/roh/NAK-283-etappe-2-erstpruefung-auftrag.txt` (Gate wörtlich, Matrix §5.1 samt M-26/M-70 Hälfte 1, drei Befundklassen, Ausschlüsse: `docs/**`, NAK-269, NAK-253, NAK-163, NAK-249, Etappen 3 bis 6, SONDE-015 §12, Nebenbefund §20.7 Punkt 4, C4458, SessionSoak, fünfter Publikationsweg; die sechs Abweichungen als gedeckt mit Prüfauftrag an der Quelle). Thread-ID und Urteil folgen in §22. |


## 22. Erstprüfung 2 — NEEDS_WORK mit drei Befunden; Validierung, Einordnung, Start der Nacharbeit 1 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-etappe2-erst`); Thread `01a0965b-5ebf-7c33-ae8e-917a8d356cf6`; Lauf 18:02 bis 18:11 Uhr, Exit 0. |
| Prüfbereich | Vorlage A: `git diff 3cfa4176...988dc44d -- <die zehn Ticketpfade>`; HEAD `92386c37` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. Auftrag `docs/beweise/roh/NAK-283-etappe-2-erstpruefung-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/NAK-283-erstpruefung-2-92386c3.txt`. |
| Urteil | **NEEDS_WORK** — wörtlich: „drei DEFEKTE in Tests beziehungsweise Messabdeckung." Geprüft: vollständiger gebundener Diff, Aufruf-/Sperrpfade, Revisionsgrenzen, Rot- und Bein-Rohbelege lesend; nicht geprüft: eigene Ausführung. Kein Befund gegen den Produktcode: Folgenummer, Sperrenordnung, dreiwertige Auswertung, Revisionsränder und Save/Load sind nicht beanstandet. |
| Befund 1 | M-06 (`Sonde012ProjectReloadTest.cpp:1944-1949`): nach Freigabe der alten Worker-Kopie publizieren Benennung und lokaler Unbind erneut den aktuellen State und heilen den zu messenden Rücksprung; in `NAK-283-rot-M-06-etappe-2.txt` bleibt der Bestandsvergleich grün, rot ist nur der Überholt-Zähler — Rotbeweis am Nebeneffekt, nicht an der Zusage. |
| Befund 2 | M-72, ebenso M-01/M-02 (`:2104-2110`, `:1697`, `:1775`): der Aufbau wartet nur auf den Mitgliederstand im State (sichtbar ab `Ipc.cpp:1585`), nicht auf Publikation, Dirty und Revision des Aufbau-Drains; Baselines und Scharfschalten fallen in dieses Fenster, der Haken kann `[B]` statt `[B, A]` festhalten — die korrekte Fassung zählte +3, MUT-B lieferte +2 und bestünde. |
| Befund 3 | M-12 (`Sonde014IntentTest.cpp:1526-1530`, `:1470-1549`): die vier MAX−1-Fälle laufen ohne Prozessor und Hostlistener, der Dirty-Block misst nur Intent bei Revision 1→2 am Normalstand; die Dirty-Zusage der Zeile hat keine Falsifikation (in `NAK-283-rot-M-12-etappe-2.txt` bleibt der Dirty-Block unter der Mutation grün). Dazu: M-08/M-09 „kein Host-Dirty" am MAX-Stand ohne Hostmessung. |
| Validierung | Drei lesende Opus-Agenten (18:13 bis 18:24 Uhr, Stand `988dc44d`, keine Repros, keine Injektionen, keine Builds), je Befund eine Datei: `docs/beweise/roh/NAK-283-erstpruefung-2-validierung-1.md`, `-2.md`, `-3.md` (Quellzitate, Teilbehauptungen, gebrochener Satz, schließende Regel, Scope-Beweis). Alle Teilbehauptungen der drei Befunde sind am Stand wahr. |
| Einordnung | **Drei DEFEKTE, eine LÜCKE mit Regel.** Befund 1: DEFEKT — bricht §6.1 Punkt 2 („Mutation genau an der Zeile, die die Zusage trägt — nie an einem Nebeneffekt"), das Gate („mit Rotbeweis an der Zusagezeile") und die M-06-Frist („Die Zusage muss ohne weiteren Befehl halten"). Befund 2: DEFEKT — bricht §6.1 Schritt 2 (erzwungenes Interleaving; „nur manchmal rot" ist kein Rotbeweis), die M-01-Vorbedingung („seine Kopie `[B, A]` gezogen") und die Dirty/Revision-Zusagen von M-01 und M-72; ein Test, der auf Timing hofft, ist laut Prüfauftrag ein Defekt. Befund 3: DEFEKT für M-12 — die Zusagespalte trägt „Host-Dirty wird genau einmal gemeldet", Host-Dirty ist nur über den Prozessor (`State.cpp:521-523`, `:577-579`, `:639-641`) beobachtbar, und der Test misst ihn am Grenzstand nicht; für M-08/M-09 steht „kein Host-Dirty, keine Revision" in Reihenfolge und Frist, nicht in der Zusage — **LÜCKE**. Kein Befund bricht Vertrag, Invariante oder Produktcode; F01 und F11 sind im Produkt erfüllt, die Messung ist es noch nicht. |
| Dirigentenregel (Lücke M-08/M-09) | „kein Host-Dirty, keine Revision" ist Teil der Zusage von M-08 und M-09 und wird am Grenzstand `int64max` über den Prozessor mit Hostlistener gemessen (null Meldungen, Revision unverändert); in die Zusagespalte der beiden Zeilen (§5.1) datiert eingetragen. Sie löst in derselben Runde Nacharbeit aus, weil Regel 3 die Grenzstände ohnehin durch den Prozessor führt. |
| Nacharbeit 1 | Runde 1 von 3. Frischer Opus-Worker (max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-auftrag.txt`: nur die drei Defekte wörtlich, je Defekt die schließende Regel aus der Validierung (Regel 1: alle vier Produktwege vor der Freigabe, letzter Schritt Freigabe plus Ruhe, Bestandsvergleich fällt unter der Mutation; Regel 2: vor Baselines und Scharfschalten muss der Aufbau-Drain über den Eintrittszähler des Drainrahmens nachweislich zurück sein, gilt für M-72, M-01, M-02 und wird an M-03, M-05, M-06 geprüft; Regel 3: Grenzstände `int64max−1` und `int64max` durch den Prozessor mit Hostlistener, genau eine bzw. null Dirty-Meldungen, Dirty-Erwartung fällt unter der M-12-Mutation) und die Prüfliste. Ticketpfade: nur die beiden Testdateien, §23 und Rohdateien; Produktcode bytegleich. Beine B14 und B27; Rotbeweise M-06, M-01b/M-72 (dreimal rot), M-01a/M-02, M-12, M-08/M-09 neu. Aufsicht ENG. Danach Wiederprüfung 1 (Vorlage B) über den Fixdiff durch Codex Astra max. |
| Rundenbilanz | Erstprüfung: keine Codeänderung; Stand vor der Runde der Commit dieses Abschnitts (Basis-SHA im Workernamen und in §23). |
| Hygiene des Fensters | Gemessen 18:05 Uhr: MEMORY.md 5 304 B, CLAUDE.md 18 959 B (≤ 20 480), Skill 24 369 B (≤ 24 576); eine MEMORY.md-Indexzeile über 250 Zeichen sofort gekürzt (rechnerlokal); Skill §0 nannte `--permission-mode auto`, der Starter nutzt `dontAsk` — berichtigt (`f8ce08a0`); `dokuriegel.py` auf CLAUDE.md, Skill, Register und Manifest ohne Befund; `gesundheit.py` Exit 4 (Funktionen über 200 Zeilen 29/28, Kommentar-Bezeichner 33/30, acht Dateien über 1 500 Zeilen, neu `Ipc.cpp` 1 663 durch diese Etappe) — bereits Registerpunkt NAK-255, Nachtrag datiert (`f8ce08a0`). Keine Fundstelle der Kommentar-Bezeichner liegt in einem Hunk der Etappe 2. |


## 23. Nacharbeit 1 der Etappe 2 (Bauer, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Runde | **Nacharbeit 1 von 3** zur Erstprüfung 2 (§22). Auftrag `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-auftrag.txt`. Geschlossen: die **drei** an der Quelle bestätigten Defekte, dazu die Dirigentenregel zur Lücke M-08/M-09. Nicht mehr und nicht weniger. |
| Basis-SHA | `f97500463b6419ca001c3f5a8bea7f33a6ddefad` (= `git rev-parse HEAD` beim Start; `git status --short` **leer**). |
| Umfang | **Zwei Testdateien**, +318 / −19 Zeilen (`git diff --numstat`): `Sonde012ProjectReloadTest.cpp` +91/−15, `Sonde014IntentTest.cpp` +227/−4. **Kein Produktcode** — der Produktdiff gegen den Basis-SHA ist leer, und die drei Quell-Hashes der Rotbeweise sind bytegleich mit §20.2. |
| Beweise | **9 Rotbeweise** nach §6.1, jeder rot → bytegleiche Rücknahme (SHA-256 vorher == nachher) → grün; MUT-B dreimal hintereinander rot. **2 Beine** einzeln gefahren, beide Exit 0; keine neue `warning C`. |
| Matrix | **Keine Matrixzeile geändert** (§22 hält die Dirigentenregel in den Zusagespalten von M-08 und M-09 bereits fest). Kein Kanon, kein Laufzeit-Arm, `docs/PLAN-STAND.md` und `docs/offene-punkte.md` nicht angefasst, `planstand.py` nicht gelaufen. |
| Produktfragen | Keine beantwortet. §8.3 ist unverändert. |

### 23.1 Defekt 1 — der M-06-Rotbeweis fällt jetzt am Bestandsvergleich

**Was war.** Der Fall gab die angehaltene Worker-Kopie bei `:1944` frei und ließ
danach noch zwei Handgriffe laufen — `benenneSourcesHauptziel` (`:1949`) und
`entferneSourcesHauptziel` (`:1958`). Beide ziehen ihre Kopie frisch aus
`zustand.mainProjectMitglieder` unter `bindungMutex` und publizieren sie; ohne
Folgenummernriegel wird jede von ihnen übernommen, und die letzte setzt das
Modell deterministisch auf den State. Sie heilten damit genau die Abweichung,
die die Zeile messen soll. Gemessen wurde deshalb nur der Überholt-Zähler — die
Zusage von M-01, M-71 und M-72, nicht die von M-06.

**Was jetzt ist** (`Sonde012ProjectReloadTest.cpp:1960-2044`,
`state_und_modell_sind_nach_ruhe_gleich`). Alle vier Produktwege laufen **vor**
der Freigabe, und der letzte Schritt vor der Ruhe ist die Freigabe plus
`ruheAbwarten()` — ohne jeden weiteren Handgriff, wie in M-01 (`:1743-1744`),
M-02 (`:1816-1817`), M-03 (`:1865-1866`) und M-05 (`:1919-1920`):

1. **Weg 4** — `setzeBindung` in `mainAnlegen`, im Konstruktor der Bühne.
2. **Aufbau (Weg 3, Workerzug, unscharf)** — `confirm_join A` und
   `confirm_join C`; Wartebedingung Zustandsbestand **plus** `ruheAbwarten()`
   (`:1978`, siehe Defekt 2).
3. **Weg 3, Workerzug, erzwungen angehalten** — `confirm_join B`; der Haken hält
   die Kopie `[A, C, B]` mit A's altem (leerem) Label. Der gehaltene Stand ist
   belegt (`:1988`: State `[A, C, B]`, Modell noch `[A, C]`).
4. **Weg 3, Message-Thread** — `unbind_probe B` über `sourcesTick()` in einem
   eigenen Faden; publiziert `[A, C]` mit größerer Folgenummer.
5. **Weg 1** — `benenneSourcesHauptziel (a, "Fluegel")`.
6. **Weg 2** — lokaler Unbind ohne Broker: C wird Hauptziel ohne gültige
   Runtime-Nonce, `entferneSourcesHauptziel (c)`.
7. **Freigabe plus Ruhe** (`:2021-2022`), dann nur noch Lesen.

Der Fall braucht dafür eine dritte Instanz `C`; sonst wäre der Endstand nach
dem lokalen Unbind leer und die Prüfzeile `! imState.empty()` nicht mehr
erfüllbar.

**Der Rotbeweis** (`NAK-283-rot-M-06-etappe-2-nacharbeit-1.txt`, MUT-A,
Mutation `SourcesModel.cpp:383-387`). Die Prüfzeile, die die Zusage trägt, geht
auf FEHLER, mit sichtbarer Divergenz in Schlüsselmenge **und** Label:

```
FEHLER  M-06: state_und_modell_sind_nach_ruhe_gleich - dieselbe Schluesselmenge,
        je Schluessel dasselbe Label
        [State: aaaa=Fluegel | Modell: aaaa=Eingeschleust bbbb=Eingeschleust cccc=Eingeschleust]
SONDE-012 ProjectReload (nur 283m06): 9/11 gruen
```

Die Zählerzeile fällt zusätzlich; sie ist damit Nebenbefund, nicht mehr der
Beleg (Validierung 1, schließende Regel Punkt 3).

### 23.2 Defekt 2 — der Aufbau ist vor Baseline und Scharfschalten nachweislich zurück

**Was war.** Die Wartebedingung las nur den Mitgliederstand im Prozessorstate.
Der wird sichtbar, sobald der Drain `bindungMutex` verlässt — Kopie,
Hakenruf, Modellübernahme, `meldeHostDirty()` und `v3StateRevision` stehen
dann noch aus. In diesem Fenster konnte der Haken den **Aufbau**-Ruf fangen
(Kopie `[B]` statt `[B, A]`), und die Baselines lagen im Nachlauf eines dritten
Befehls: die korrekte Fassung zählte +3 und wurde rot, MUT-B lieferte durch den
ausgelassenen Dirty-/Revisionspfad genau die erwarteten +2 und bestand — eine
mögliche **Invertierung** des Rotbeweises, nicht bloß Flakiness.

**Was jetzt ist.** Vor dem Lesen der Baselines und vor `scharf.store (true)`
läuft `ruheAbwarten()` — der Eintrittszähler des Drainrahmens
(`PluginProcessor.h`, „der nächste Eintritt des Workerzugs heisst, sein voriger
Aufruf ist samt Nachführung zurück"). Weil der Workerzug den Rahmen sequenziell
aufruft und der Zähler **vor** jedem Riegel steigt, beweist ein Stand größer als
der beim Eintreten der Zustandsbedingung gelesene, dass der Aufruf, der den
Aufbau angewandt hat, samt Publikation, Dirty und Revision zurück ist.

| Zeile | Fundstelle | Was ergänzt wurde |
|---|---|---|
| M-01 | `Sonde012ProjectReloadTest.cpp:1711` | `ruheAbwarten()` nach der Zustandsbedingung, vor Baseline und Scharfschalten |
| M-02 | `:1795` | dieselbe Wartebedingung; zusätzlich `:1808` — Beleg, **welchen** Stand der Haken hält (State `[A, B]`, Modell noch `[A]`) |
| M-06 | `:1978` | dieselbe Wartebedingung; Standbeleg `:1988` |
| M-72 Phase B | `:2177` | dieselbe Wartebedingung; Aufbau prüft jetzt auch das Modell (`:2165`); Standbeleg `:2187` (State `[B, A]`, Modell noch `[B]`) |

**M-03, M-05 geprüft, nichts nachzuziehen** (Auftrag Regel 2, Satz 3). Beide
haben **keinen** Aufbau-Drain, dessen Nachlauf noch ausstehen könnte: M-03
(`:1836-1876`) setzt nur eine Fixture und schaltet dann scharf — der einzige
nicht leere Batch ist der eigene `confirm_join A`, und ein leerer Batch ruft
den Publikationshaken nicht (`Ipc.cpp`, früher `return`). M-05 (`:1888-1940`)
liest seine Baselines, bevor überhaupt ein Befehl existiert; die Vorbedingung
ist „Modell und State sind leer". Ihre Rotbeweise bleiben damit gültig (§23.5).

**Der Rotbeweis** (`NAK-283-rot-M-01b-…` und `NAK-283-rot-M-72-etappe-2-nacharbeit-1.txt`,
MUT-B, Mutation `Ipc.cpp:1477-1478`), **dreimal hintereinander gefahren, alle
drei rot** — und in jedem Lauf an derselben Zusagezeile mit demselben Wert:

```
FEHLER  M-01: Dirty-Zaehler == Zahl der State-aendernden Befehle (2), nicht 1  [1]
FEHLER  M-01: Revision-Delta == 2  [1]
FEHLER  M-72 Phase B: ... der Ueberholtfall meldet Dirty und Revision ...  [1 Dirty, Revision-Delta 1]
```

Der gemessene Wert `[1]` ist der Beleg, dass die Baseline jetzt **nach** dem
Aufbau-Dirty liegt: das Fenster hätte `[2]` geliefert und die Mutante bestehen
lassen.

### 23.3 Defekt 3 — Host-Dirty an den zugesagten Revisionsgrenzen

**Was war.** Die vier MAX−1-Blöcke von M-12 liefen unter den
`state::`-Funktionen, die Host-Dirty gar nicht kennen; der Dirty-Block maß
einen Normalstand und einen einzigen Handgriff. Unter der Mutation, die die
Zeile trägt, blieb der ganze Dirty-Block grün — die Dirty-Zusage hatte keine
Falsifikation (§6.1 Schritt 4, Prüfliste E). Für M-08/M-09 fehlte die
Hostmessung am Grenzstand ganz.

**Was jetzt ist.** Ein Helfer `prozessorMit` (`Sonde014IntentTest.cpp:1309`)
lädt denselben deklarierten Writer-Mutanten in einen frischen Prozessor — keine
neue Fixture, nur ein zweiter Träger (Fixture-Regel §6.2; `StateMigrationTestMain.cpp`
führt den Weg für M-11 vor). Gemessen wird über die **Produkthandgriffe** mit
angehängtem `DirtyZaehler`:

| Block | Fundstelle | Vorzustand | Zusage, jetzt gemessen |
|---|---|---|---|
| M-12 Host (Intent) | `:1626-1657` | Bestandsrevision `int64max - 1`, ein Intent | `entferneQuellenrolle`: `true`, Eintrag weg, Revision `int64max`, **genau eine** Dirty-Meldung; folgenloser Zweitaufruf: `true`, Revision still, **null** weitere |
| M-12 Host (Schutz) | `:1660-1684` | dito, eine Schutzangabe | `hebeQuellenschutzAuf`, gleiche vier Größen |
| M-12 Host (Beziehung) | `:1687-1709` | dito, eine Beziehung | `entferneQuellenbeziehung`, gleiche vier Größen |
| M-12 Host (Assistent) | `:1713-1740` | Assistentenrevision `int64max - 1`, offener Schritt | `assistentUeberspringen`: Schritt wechselt, Revision `int64max`, **genau eine** Dirty-Meldung; der Zweitaufruf steht dann AM Maximum, wird abgewiesen und meldet nichts |
| M-08 Host | `:1439-1460` | Bestandsrevision `int64max`, eine Schutzangabe | `hebeQuellenschutzAuf`: `false`, Angabe steht noch, Revision still, **null** Dirty-Meldungen |
| M-09 Host | `:1465-1480` | dito, eine Beziehung | `entferneQuellenbeziehung`, gleiche vier Größen |

Die beiden MAX-Blöcke liegen in einer **eigenen** Funktion `m283_08_09_am_host`
(`:1430`, gerufen `:1801`), damit `m283_07_bis_09` unverändert bleibt und seine
Rotbeweise gültig bleiben.

**Die Rotbeweise.**

- `NAK-283-rot-M-12-etappe-2-nacharbeit-1.txt` (MUT-J, `NakamaState.cpp:2209`):
  **neun** Fehler statt bisher drei. Die Dirty-Erwartung fällt jetzt mit —
  „`M-12 Host (Intent): … Host-Dirty wird GENAU EINMAL gemeldet  [0 Dirty,
  Revision 9223372036854775806]`" — für alle drei Bestandshandgriffe, plus die
  drei Zweitaufruf-Zeilen.
- `NAK-283-rot-M-12b-etappe-2-nacharbeit-1.txt` (MUT-K, **neu**,
  `assistentenrevisionHeben`-Schranke auf `int64max - 1` vorgezogen): drei
  Fehler, darunter die Assistenten-Dirty-Zeile. MUT-J trägt nur
  `bestandsrevisionHeben` und lässt den Assistentenzweig unberührt; ohne MUT-K
  bliebe dessen Dirty-Zusage unfalsifiziert (Validierung 3, Regel d — die
  Entscheidung, einen zweiten Mutanten zu bauen statt den Zweig als unberührt zu
  bezeichnen, liegt dort ausdrücklich beim Bauer).
- `NAK-283-rot-M-08-…` und `-M-09-etappe-2-nacharbeit-1.txt` (MUT-G, MUT-H):
  je drei Fehler; die neuen Hostzeilen fallen an Container und Rückgabewert
  (`[0 Dirty, 0 Angabe(n)]` bzw. `[0 Dirty, 0 Kante(n)]`).

**Die Dirty-NULL-Hälfte der MAX-Messung ist eine Regressionswache** (Muster
M-04, M-12), ausdrücklich benannt statt mit einer erfundenen Mutation
gebrochen: Der Prozessor kehrt bei jeder Ablehnung mit `return false` zurück,
**bevor** er seinen `if (veraendert)`-Block erreicht (`State.cpp:575-577`,
`:637-639`). Es gibt deshalb keine Mutation an einer Zusagezeile, die genau
diese Null kippt — unter MUT-G/MUT-H fällt die Zeile an Container und
Rückgabewert, die Null bleibt richtig. Der Auftrag lässt diesen Fall
ausdrücklich zu (Regel 3, letzter Satz).

### 23.4 Je Matrixzeile: Test und Rotbeweis

Alle nach §6.1: SHA-256 der unveränderten Quelle, Mutation an der Zusagezeile,
Zeitstempel, Bein rot, **bytegleiche Rücknahme der Originalbytes**, Zeitstempel,
Bein grün. Skript `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-rotskript.ps1`;
es spielt die gesicherten Bytes zurück, nie eine zweite Textersetzung.

| Zeile | Test (Datei, Name) | Bein | Mutation | Rohdatei | rot / grün |
|---|---|---|---|---|---|
| M-01 (a) | `Sonde012ProjectReloadTest.cpp:1685` `aeltere_mitgliederpublikation_ersetzt_keine_juengere` | B14 `--nur 283m01` | MUT-A `SourcesModel.cpp:383-387` | `NAK-283-rot-M-01a-etappe-2-nacharbeit-1.txt` | Exit 1 / Exit 0 |
| M-01 (b) | derselbe Fall, Dirty- und Revisionshälfte | B14 `--nur 283m01` | MUT-B `Ipc.cpp:1477-1478` | `NAK-283-rot-M-01b-etappe-2-nacharbeit-1.txt` | Exit 1/1/1 / Exit 0 |
| M-02 | `:1778` `hauptziel_benennung_wird_nicht_von_aelterer_workerkopie_ueberholt` | B14 `--nur 283m02` | MUT-A | `NAK-283-rot-M-02-etappe-2-nacharbeit-1.txt` | Exit 1 / Exit 0 |
| M-06 | `:1960` `state_und_modell_sind_nach_ruhe_gleich` | B14 `--nur 283m06` | MUT-A | `NAK-283-rot-M-06-etappe-2-nacharbeit-1.txt` | Exit 1 / Exit 0 |
| M-72 | `:2107` `reloadablehnung_und_ueberholung_sind_unterscheidbar` | B14 `--nur 283m72` | MUT-B | `NAK-283-rot-M-72-etappe-2-nacharbeit-1.txt` | Exit 1/1/1 / Exit 0 |
| M-08 | `Sonde014IntentTest.cpp:1319` `m283_07_bis_09` **und neu** `:1430` `m283_08_09_am_host` | B27 | MUT-G `NakamaState.cpp` `entferneSchutzangabe` | `NAK-283-rot-M-08-etappe-2-nacharbeit-1.txt` | Exit 1 / Exit 0 |
| M-09 | dieselben zwei Fälle, Block M-09 | B27 | MUT-H `NakamaState.cpp` `entferneBeziehung` | `NAK-283-rot-M-09-etappe-2-nacharbeit-1.txt` | Exit 1 / Exit 0 |
| M-12 (Bestand) | `:1560` `m283_12`, vier Modul- plus drei Hostblöcke | B27 | MUT-J `NakamaState.cpp:2209` | `NAK-283-rot-M-12-etappe-2-nacharbeit-1.txt` | Exit 1 / Exit 0 |
| M-12 (Assistent) | derselbe Fall, Assistentenblock und Hostblock | B27 | MUT-K `assistentenrevisionHeben`-Schranke, **neu** | `NAK-283-rot-M-12b-etappe-2-nacharbeit-1.txt` | Exit 1 / Exit 0 |

**Die drei Quell-Hashes** (unverändert vor und nach jedem Rotbeweis) sind
**bytegleich mit §20.2** — der zweite unabhängige Beleg dafür, dass der
Produktcode dieser Runde nicht angefasst wurde:

| Datei | SHA-256 | = §20.2 |
|---|---|---|
| `eq-copilot/plugin/src/SourcesModel.cpp` | `520A727382AAF6C0A8E1AA65D9BA973CC9E87B75825C8D2EB16CEDCB412373B3` | ja |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | `B74AC525AD97CEEC36A0E0F56AA24599ECF7782B617164845A2B562C947B4B97` | ja |
| `eq-copilot/plugin/state/NakamaState.cpp` | `A947AD28064B082C3DCF69E03DDF60E00539DBA6B43B9BE384D7993D88F4D8E9` | ja |

### 23.5 Nicht neu gefahrene Rotbeweise, mit Grund

Alle übrigen Rotbeweise der Etappe 2 bleiben gültig, weil ihr Testfall
unverändert ist und ihre Mutation denselben Fall trifft:

| Zeile | Rohdatei (Etappe 2) | Grund |
|---|---|---|
| M-03 | `NAK-283-rot-M-03-etappe-2.txt` | Fall `lokaler_unbind_wird_nicht_von_aelterer_workerkopie_ueberholt` unverändert; kein Aufbau-Drain, also nichts nach Regel 2 nachzuziehen (§23.2) |
| M-04 | `NAK-283-rot-M-04-etappe-2.txt` | Fall `gezogene_kopie_ueberholt_den_reload_nicht` (M-39) unverändert |
| M-05 | `NAK-283-rot-M-05-etappe-2.txt` | Fall `inhaltsgleiche_publikation_zieht_die_frischemarke_nach` unverändert; kein Aufbau-Drain (§23.2) |
| M-07 | `NAK-283-rot-M-07-etappe-2.txt` | Fall `m283_07_bis_09` unverändert; die neue Hostmessung liegt in einer **eigenen** Funktion |
| M-10 | `NAK-283-rot-M-10-etappe-2.txt` | Fall `m283_10` unverändert |
| M-11 (a), (b) | `NAK-283-rot-M-11a-…`, `-M-11b-etappe-2.txt` | `StateMigrationTestMain.cpp` ist in dieser Runde nicht angefasst; B2 gehört nicht zu den Beinen des Auftrags |
| M-71 | `NAK-283-rot-M-71-etappe-2.txt` | Fall `mitgliederfolge_wrappt_nicht` unverändert; setzt `scharf` nie, also kein Hakenfenster |

Ersetzt sind damit ausschließlich die neun Rohdateien aus §23.4; die Zeilen
M-01, M-02, M-06, M-08, M-09, M-12 und M-72 lesen ab hier ihren Beleg aus der
`-nacharbeit-1`-Fassung, §20.2 bleibt als Verlauf unangetastet.

### 23.6 Gefahrene Beine

Skript `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-beinskript.ps1`,
Rohausgabe `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-beine.txt`. Gebaut
aus pwsh (Release, `eq-copilot/build`); die Binary-Zeitstempel sind gegen die
jüngste geänderte Quelle geprüft (beide **frisch**, NAK-230).

| Bein | Ziel | Exit | Messung |
|---|---|---:|---|
| B14 | `EqCopSonde012ProjectReloadTest` | 0 | **227/227** (vorher 219; die Wartebedingungen und Standbelege bringen 8) |
| B27 | `EqCopSonde014IntentTest` | 0 | **270 Prüfungen** (vorher 254; die Hostmessungen bringen 16) |

**Warnungen.** Der Lauf übersetzt beide Testquellen wirklich neu — die
Zeitstempel wurden vor dem Bau gesetzt, weil ein rein inkrementeller Bau nichts
kompiliert und „null `warning C`" dann keine Aussage wäre, sondern die
Abwesenheit einer Messung. Gemeldet werden **zwei** `warning C`: beide dieselbe
vorbestehende `C4458` aus `core/analysis/featureengine/Stereo.h:70`, einmal je
Projekt. **Keine neue Warnung, keine aus einer geänderten Datei.**

**Kein Nulltest, kein Passthrough-Bein, kein Kanon, kein Laufzeit-Arm** —
auftragsgemäß, weil kein Produktcode geändert ist; der leere Produktdiff belegt
es.

### 23.7 Produktdiff leer

```
git diff --stat f97500463b6419ca001c3f5a8bea7f33a6ddefad..HEAD -- eq-copilot/plugin/src eq-copilot/plugin/state broker eq-copilot/schemas eq-copilot/fixtures
```

Ausgabe: **leer** (keine Zeile). Der vollständige Diff gegen den Basis-SHA
berührt unter `eq-copilot/` ausschließlich die beiden Testdateien:

```
91      15      eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp
227     4       eq-copilot/plugin/tests/Sonde014IntentTest.cpp
```

Zweiter, unabhängiger Beleg: die drei Quell-Hashes in §23.4 sind bytegleich mit
denen aus §20.2.

### 23.8 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Abschnitt | Wo gemessen |
|---|---|
| **A** — Zähler einer Politik werden ausgewertet | Unverändert gegenüber §20.5; diese Runde ergänzt keinen Zähler. Der Überholt-Zähler ist in M-06 jetzt Nebenbefund neben der Zusagezeile, nicht mehr ihr Ersatz. |
| **B** — Lebenszyklus | Die Bühne ist unverändert: Schranke und Zähler stehen weiterhin **vor** dem Prozessor im Member-Block. Die neuen Prozessoren in B27 leben je in einem eigenen Block und sterben vor ihrem `DirtyZaehler`; `removeListener` läuft in jedem Block. |
| **C** — Zahlenränder | `int64max - 1` und `int64max` laufen jetzt **zusätzlich** durch den Prozessor — vier Handgriffe unter der Grenze, zwei an der Grenze. Kein Wert reist über den Draht. |
| **D** — Bau- und Prüfriegel | Beinskript und Rotskript prüfen die Binary-Zeitstempel gegen die Quellen und brechen ab, wenn ein Binary älter ist (NAK-230). Das Rotskript bricht ab, wenn das Mutationsmuster fehlt oder mehr als einmal vorkommt, und wenn die Rücknahme nicht bytegleich ist. Der Bau des Beinskripts wurde zu einer echten Neuübersetzung gezwungen (§23.6). |
| **E** — jede neue oder geänderte Prüfung einmal absichtlich gebrochen | Neun Rotbeweise mit Rohausgabe, SHA-256-Paar und beiden Exitcodes. Die **einzige** Ausnahme ist benannt und begründet: die Dirty-NULL-Hälfte der MAX-Messung von M-08/M-09 ist eine Regressionswache, weil der Prozessor bei jeder Ablehnung vor dem Dirty-Pfad aussteigt (§23.3). Sie zählt nicht als Beleg. |
| **E** — Behauptung ≤ Messung | Zwei eigene Kürzungen: (1) die Assistenten-Zweitaufrufzeile sagt „wird abgewiesen und meldet nichts", nicht „ist ein No-op" — nach dem ersten Überspringen steht die Revision am Maximum, der zweite Aufruf ist eine **Ablehnung**, kein folgenloser Aufruf; (2) die MAX-Dirty-Messung ist als Wache benannt, nicht als Beleg gezählt. |
| **E** — Zahlen gemessen, nicht abgeschrieben | 227 und 270 stammen aus `NAK-283-etappe-2-nacharbeit-1-beine.txt`, die Zeilenzahlen aus `git diff --numstat`, die Hashes aus den Rohdateien der Rotbeweise. |
| **F** — Änderungssatz | Kein Speicher-, Lebenszyklus- oder Vertragspaar berührt: die Runde ändert ausschließlich Messung, kein Verhalten. Save↔Load bleibt bei M-11 (B2), unverändert. |
| **Dirty je persistenter Änderung** | Neu am **Grenzstand** gemessen: genau eine Meldung je wirksamer Änderung bei `int64max - 1` (vier Handgriffe), null bei einem folgenlosen Zweitaufruf, null bei einer Ablehnung an `int64max` (Schutz und Beziehung; Intent und Assistent trägt M-11). |
| **Stale Closures** | Die Bühne ist unverändert. Neu geprüft ist die **Scharfschaltordnung** des gefangenen `scharf`-Flags — genau die Lücke, an der das Selbstaudit der Etappe 2 danebengeprüft hatte (§20.5 „Stale Closures" behandelte nur die Lebensdauer). |
| **Sperrenordnung** | Unberührt — kein Produktcode. Die neue Wartebedingung nimmt keine Sperre; `sourcesDrainEintritteFuerTest` liest ein `std::atomic`. |
| **MSVC-Stack 1 MiB** | Alle neuen Prozessoren liegen auf dem Heap (`std::make_unique` in `prozessorMit`), NAK-175. |
| **Unspezifizierte Auswertungsreihenfolge** (eigener Befund dieser Runde) | Im ersten Entwurf standen Handgriff und Messung als **zwei Argumente desselben** `pruefe`-Aufrufs; MSVC wertete die Detailspalte zuerst aus und zeigte den Zustand von **vor** dem Handgriff (`[0 Dirty, …]` bei grüner Prüfung). Im Rotlauf hätte die Diagnosespalte gelogen. Behoben: Handgriff ausführen, Messwerte in `const`-Lokale, dann `pruefe` — in allen sechs neuen Hostblöcken. |

### 23.9 Offene Punkte dieser Runde

1. **Die Dirty-NULL-Hälfte der MAX-Messung ist eine Wache, kein Beleg**
   (§23.3). Sie fällt nur zusammen mit Container und Rückgabewert. Wer sie
   einzeln falsifizieren wollte, müsste zwei Stellen zugleich ändern (den frühen
   `return false` **und** den `if (veraendert)`-Riegel in `State.cpp`) — das
   wäre eine erfundene Mutation, die der Auftrag ausdrücklich ausschließt.
2. **M-12 trägt jetzt zwei Mutanten** (MUT-J für den Bestand, MUT-K für den
   Assistenten). Die Matrixzeile nennt in der Rotbeweisspalte nur „eine Schranke
   bei `int64max - 1`"; das ist keine Abweichung, sondern die Auflösung der
   Zeile in ihre beiden Vorbedingungen („Bestandsrevision **bzw.**
   Assistentenrevision"). Keine Matrixzeile wurde geändert — die Prüfung
   entscheidet.
3. **M-26 und M-70 bleiben je zur Hälfte gebaut** (§20.7 Punkt 1), unverändert.
4. **Kein Kanon, kein Laufzeit-Arm, kein Planstand** — auftragsgemäß.


## 24. Messung der Nacharbeit 1 und Start der Wiederprüfung 1 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `65a9e0e3` (`nakama-nak283-f975004-e2nach1`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-auftrag.txt` (§22); Start 18:28 Uhr, letzter Commit 18:55 Uhr; Beobachter (Aufsicht ENG): Start, Telemetrie gesund, drei HEAD-Wechsel, Ende; Stundenloop `3791fe5e` gelöscht, `CronList` leer. |
| Commits | `3e7e0a12` (die zwei Testdateien), `e775007c` (neun Rotbeweise, Beine, Skripte, §23), `faea3c1a` (Kopfzeile), alle auf `origin/master`; Basis `f9750046` ist Vorfahr; `git status --short` leer. |
| Diff | `git diff --stat f9750046..faea3c1a`: 15 Dateien, +4 502 / −20 — zwei Testdateien (+318/−19), Manifest §23, zwölf Rohdateien. **Produktdiff leer:** `git diff --stat f9750046..faea3c1a -- eq-copilot/plugin/src eq-copilot/plugin/state broker eq-copilot/schemas eq-copilot/fixtures` ohne Zeile; die drei Quell-Hashes der Rotbeweise sind bytegleich mit §20.2. |
| Rundenbilanz | `f9750046..faea3c1a: Tests 2 Datei(en) +318/-19 \| Doku 13 Datei(en) +4184/-1`; kumuliert `--runden bb49a612 988dc44d faea3c1a`: Etappe 2 Produkt 6 Dateien +394/−91 und Tests 4 +1 099/−7, Nacharbeit 1 Tests 2 +318/−19 — „kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)". |
| Eigene Messung | Binaries frischer als Quellen (Quellen 18:48:14 Uhr, Binaries 18:48:21 und 18:48:27 Uhr). Vom Dirigenten am Stand `faea3c1a` um 18:58 Uhr selbst gefahren: B14 `EqCopSonde012ProjectReloadTest` 227/227 Exit 0; B27 `EqCopSonde014IntentTest` 270 Prüfungen Exit 0 — identisch mit `NAK-283-etappe-2-nacharbeit-1-beine.txt`. Rotbeweise gelesen: M-06 fällt an „dieselbe Schlüsselmenge, je Schlüssel dasselbe Label" (State `[aaaa=Fluegel]`, Modell `[aaaa, bbbb, cccc]`), 9/11 rot, 11/11 grün nach Rücknahme; M-01b und M-72 je drei Rotläufe, alle drei rot mit `[1]` an Dirty und Revision (Beleg, dass die Baseline nach dem Aufbau-Dirty liegt); M-12 unter MUT-J neun Fehler, darunter alle sechs Host-Dirty-Zeilen mit `[0 Dirty]`; M-12b unter MUT-K drei Fehler mit der Assistenten-Dirty-Zeile; SHA-256 vorher = nachher in allen vier Dateien. |
| Messabdeckung | Die drei Defekte je mit Test und Rotbeweis an der Zusagezeile; die Dirty-NULL-Hälfte der MAX-Messung (M-08/M-09) als Regressionswache benannt und begründet (§23.3, vom Auftrag zugelassen); M-03 und M-05 geprüft, kein Aufbau-Drain (§23.2); nicht neu gefahrene Rotbeweise mit Grund (§23.5). Zwei Mutanten für M-12 (MUT-J Bestand, MUT-K Assistent) sind Auflösung der Zeile in ihre beiden Vorbedingungen, keine Abweichung. |
| Selbstbericht des Bauers (§23, nicht als Beweis gezählt) | Vier Produktwege vor der Freigabe mit dritter Instanz C (M-06); `ruheAbwarten()` über den Eintrittszähler vor Baseline und Scharfschalten in M-01, M-02, M-06, M-72 samt Standbelegen; Helfer `prozessorMit` lädt die Writer-Mutanten in einen frischen Prozessor, sechs Hostblöcke mit `DirtyZaehler`; eigener Befund zur Auswertungsreihenfolge im `pruefe`-Aufruf behoben (Handgriff → Messwert → Prüfung). |
| Wiederprüfung 1 | Vorlage B: Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-etappe2-wieder1`); Prüfbereich `git diff f9750046...faea3c1a -- Sonde012ProjectReloadTest.cpp Sonde014IntentTest.cpp`, Befundliste = die drei Befunde der Erstprüfung 2; Auftrag `docs/beweise/roh/NAK-283-etappe-2-wiederpruefung-1-auftrag.txt`; Thread-ID und Urteil folgen in §25. Rundenbudget: Runde 1 von 3 geprüft. |


## 25. Wiederprüfung 1 — NEEDS_WORK mit einem neuen Bruch; Validierung, Einordnung, Start der Nacharbeit 2 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-etappe2-wieder1`); Thread `01a0968f-b45c-7c01-b8e6-47c2fe73057d`; Lauf 18:59 bis 19:07 Uhr, Exit 0. |
| Prüfbereich | Vorlage B: `git diff f9750046...faea3c1a -- Sonde012ProjectReloadTest.cpp Sonde014IntentTest.cpp`, Befundliste = die drei Befunde der Erstprüfung 2; HEAD `d5205c28` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. Auftrag `docs/beweise/roh/NAK-283-etappe-2-wiederpruefung-1-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/NAK-283-wiederpruefung-1-d5205c2.txt`. |
| Urteil | **NEEDS_WORK** — wörtlich: „die drei verlangten Reparaturen sind umgesetzt, aber der Fixdiff führt den beschriebenen Fehler in den Aufbauprüfungen ein." Frage 1: Befund 1 ja (vier Produktwege vor Freigabe, Bestandsvergleich im M-06-Rohbeleg rot), Befund 2 ja (Drainabschluss vor Baselines und Scharfschalten in M-01, M-02, M-06, M-72; M-03/M-05 ohne Aufbau-Drain; MUT-B dreimal rot), Befund 3 ja (sechs Host-Blöcke an MAX−1 und MAX, Dirty-Erwartung fällt unter MUT-J/MUT-K; Heap, Listener, Auswertungsreihenfolge eingehalten); Produktdiff leer. Frage 2: **ein Bruch** — die neue einmalige Modellprüfung `warteAuf(State) && modellTraegt(Modell)` im Aufbau von M-72 Phase B (`:2165-2168`) und M-06 (`:1975-1977`) steht **vor** `ruheAbwarten()`; wird der Worker beim Aufbau-Join nach Freigabe von `bindungMutex` und vor der Modellpublikation unterbrochen, zählt `pruefe` einen Fehler, den `ruheAbwarten()` nicht zurücknimmt — der Grünlauf ist timingabhängig. |
| Validierung | Ein lesender Opus-Agent (Stand `faea3c1a`), Datei `docs/beweise/roh/NAK-283-wiederpruefung-1-validierung.md`: State wird an Ipc.cpp:1585 unter bindungMutex sichtbar, das Modell erst an SourcesModel.cpp:361 unter eigener Sperre; dazwischen liefert warteAuf true und das einmalig ausgewertete modellTraegt false, pruefe (Sonde012ProjectReloadTest.cpp:48-55) zählt sofort, main:2761 gibt Exit 1. Gebrochen: §6.1 Punkt 5 („Erwartet: grün"), zweitrangig §6.1 Punkt 2 und die Regel aus §22 (ein Test, der auf Timing hofft, ist ein Defekt). Fundstellen: M-72 :2166 und M-06 :1976 (neu aus dem Fixdiff), dazu gleichartig und schon seit f9750046 vorhanden M-01 :1698 und M-02 :1790 — alle vier gehören in diese Runde. Regelkonformer Präzedenzfall in derselben Datei: M-71 (:2068, :2079, :2090). Standbelege (:1727, :1808, :1988, :2187) und Baselines bleiben stehen. |
| Einordnung | **DEFEKT**, vom Fixdiff der Nacharbeit 1 eingeführt: bricht §6.1 (Grünlauf nach bytegleicher Rücknahme muss deterministisch sein; ein Test, der auf Timing hofft, ist kein Beweis) — dieselbe Klasse wie Befund 2 der Erstprüfung, nur im Grünlauf statt im Rotlauf. Kein Bruch im Produktcode, keine Matrixzeile verletzt; M-01 (`:1698`) und M-02 (`:1790`) tragen laut Validierung dasselbe Muster (schon seit `f9750046`, in der Erstprüfung nicht erhoben) und werden in derselben Runde mitbehoben — dieselbe Ursache, ein Auftrag. |
| Nacharbeit 2 | Runde 2 von 3, enger Einzelauftrag. Frischer Opus-Worker (max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-auftrag.txt`: nur dieser Defekt wörtlich mit der schließenden Regel (Modellprüfung hinter State-Wartebedingung **und** `ruheAbwarten()`; Standbelege und Baselines bleiben; kein Timing als Ersatz), Ticketpfade nur `Sonde012ProjectReloadTest.cpp`, §26, Rohdateien; B14 einmal plus zehn Wiederholungen; Rotbeweise M-06 und M-72 (drei Rotläufe) neu, M-01b und M-02 ebenfalls, weil ihr Aufbau geändert wird. Aufsicht ENG. Danach Wiederprüfung 2 (Vorlage B) über den Fixdiff dieser Runde. |
| Rundenbilanz | Wiederprüfung: keine Codeänderung; Stand vor der Runde 2 ist der Commit dieses Abschnitts (Basis-SHA im Workernamen und in §26). |


## 26. Nacharbeit 2 der Etappe 2 (Bauer, 12.09.2026)

**Was war.** Der Fixdiff der Nacharbeit 1 zog in die Aufbauprüfung von M-72
Phase B und M-06 einen Modellvergleich ein, der mit der State-Wartebedingung in
**derselben kurzschließenden Bedingung** stand (`warteAuf (…) && modellTraegt
(…)`). State und Modell stehen aber unter **zwei** Sperren: der neue
Mitgliederstand wird sichtbar, sobald der Drain `bindungMutex` verlässt
(`Ipc.cpp`, Ende der Anwendungsklammer), das Modell erst, wenn
`setzePersistenteMitglieder` seine eigene Sperre genommen hat
(`SourcesModel.cpp`). Dazwischen liefert `warteAuf` bereits `true`, während das
genau einmal ausgewertete `modellTraegt` noch `false` liefert — und `pruefe`
(`:48-55`) kann einen gezählten Fehler nicht zurücknehmen, auch das folgende
`ruheAbwarten()` nicht. Der von §6.1 Punkt 5 geforderte Grünlauf nach
bytegleicher Rücknahme hing damit am Interleaving. M-01 und M-02 trugen
dieselbe Form schon seit `f9750046`. Einordnung des Dirigenten: **DEFEKT**
(§25), dieselbe Klasse wie Befund 2 der Erstprüfung 2, nur im Grünlauf statt im
Rotlauf.

**Was jetzt ist.** Der Modellvergleich ist aus der `&&`-Verbindung gelöst und
steht in allen vier Aufbaublöcken als **eigene Prüfzeile hinter beiden
Riegeln**: hinter der State-Wartebedingung **und** hinter `s.ruheAbwarten()`,
dem Eintrittszähler des Drainrahmens. Der Zähler steigt **vor** jedem Riegel
des Rahmens, und der Workerzug ruft den Rahmen sequenziell — ein Stand größer
als der beim Eintreten der Zustandsbedingung gelesene beweist deshalb, dass der
Aufruf, der den Aufbau angewandt hat, samt Publikation zurück ist. Der
Vergleich ist dort deterministisch. Die Reihenfolge je Bühne lautet jetzt:
State-Wartebedingung → `ruheAbwarten()` → Modellvergleich → Baselines →
`scharf.store (true)`. **Kein Schlaf, kein Timing, keine zweite Wartebedingung
auf das Modell** — der regelkonforme Präzedenzfall M-71 (`:2090`) stand schon
so in derselben Datei.

### 26.1 Start und Grenze

| Merkmal | Wert |
|---|---|
| Basis-SHA | `336ce254a9299ecc19550499b2b4f504b9ba5dec` (= `git rev-parse HEAD` beim Start) |
| Arbeitsbaum beim Start | `git status --short` **leer** |
| Rundenstand nach dem Rebase | `831fe896` — der `git pull --rebase` vor dem Push zog sechs Commits einer **parallelen Session** (Anti-Drift und Planstand, `65cb307e` bis `831fe896`) vor die drei dieser Runde. Der Diff **dieser Runde** ist deshalb `831fe896..HEAD` (§26.7); die Commits der Runde sind `b89174aa` (Testdatei), `4e62ccf7` (Beweise und dieser Abschnitt) und die Nachträge an der lebenden Kopfzeile |
| Geänderte Dateien | `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` (+40/−12), dieser Abschnitt, acht Rohdateien unter `docs/beweise/roh/` |
| Nicht angefasst | Produktcode (§26.7), `Sonde014IntentTest.cpp`, jede Matrixzeile aus §5.1, `docs/PLAN-STAND.md`, `docs/offene-punkte.md`; `planstand.py` lief nicht |

### 26.2 Die eine Änderung, je Fall

Je Fall wandert **genau ein** Teilausdruck: der `&& modellTraegt (…)`-Zweig der
Aufbauzeile wird zu einer eigenen `pruefe`-Zeile hinter `s.ruheAbwarten()`. Die
verbleibende Wartebedingung heißt fortan „im State" statt „in State UND
Modell" — die Beschriftung sagt damit genau das, was die Zeile misst
(Prüfliste E, „Behauptung ≤ Messung"). Zeilennummern zum Stand dieses
Abschnitts:

| Fall | State-Wartebedingung | Drainriegel `ruheAbwarten()` | Modellvergleich (neu, eigene Zeile) |
|---|---|---|---|
| M-01 | `:1697` | `:1710` | `:1724` |
| M-02 | `:1802` | `:1807` | `:1812` |
| M-06 | `:1992` | `:1994` | `:1999` |
| M-72 Phase B | `:2187` | `:2198` | `:2205` |

Der Diff besteht aus genau **vier Hunks**, einem je Fall; keine fünfte Stelle
der Datei ist berührt.

### 26.3 Was nicht verschoben wurde

| Was | M-01 | M-02 | M-06 | M-72 Phase B |
|---|---|---|---|---|
| **Standbeleg** (welchen Stand hält der Haken) | `:1740` | `:1825` | `:2010` | `:2215` |
| **Baselines** `dirtyVor` / `revisionVor` / `ueberholtVor` | `:1728-1730` | `:1815` | — (ohne Baseline) | `:2208-2209` |
| **`s.scharf.store (true)`** als letzter Aufbauschritt | `:1735` | `:1819` | `:2006` | `:2211` |

Alle drei stehen weiter **nach** `ruheAbwarten()` und **vor** dem ersten
scharfen Ereignis — genau die Ordnung, die §23.2 als Schließung von Defekt 2
eingeführt hat und die die Rotläufe `[1]` statt `[2]` messen lässt. Auch
**M-71** (`:2090`), **M-03** und **M-05** sind unberührt: M-71 war schon
regelkonform, M-03 und M-05 haben keinen Aufbau-Drain (§23.2).

### 26.4 Gefahrenes Bein

Skript `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-beinskript.ps1`,
Rohausgabe `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-beine.txt`. Gebaut
mit cmake aus pwsh (Release, `eq-copilot/build`); das Skript setzt vor dem Bau
den Zeitstempel der Quelle (NAK-230) und bricht ab, wenn das Binary älter ist
als sie. Gemessen: Quelle 19:31:35 Uhr, Binary 19:31:44 Uhr — **frisch**.

| Bein | Ziel | Läufe | Exit | Zählstand |
|---|---|---:|---:|---|
| B14 | `EqCopSonde012ProjectReloadTest` | 1 + 10 Wiederholungen | **0 in allen 11** | **231/231 grün in jedem Lauf** |

**Ein einziger Zählstand über elf Läufe** (das Skript vergleicht sie und bricht
bei mehr als einem ab). 231 gegenüber 227 in §23.6: **+4**, genau die vier neuen
Aufbau-Prüfzeilen. Die zehn Wiederholungen sind der eigentliche Punkt dieser
Runde: gegen ein Fenster von Mikrosekunden sagt ein einzelner grüner Lauf
nichts, erst die Wiederholung ohne Ausreißer stützt die Zusage „Erwartet:
grün".

**Warnungen.** Der Lauf übersetzt die Testquelle wirklich neu (Zeitstempel vor
dem Bau gesetzt, sonst wäre „null `warning C`" die Abwesenheit einer Messung).
Gemeldet wird **eine** eindeutige `warning C`: die vorbestehende `C4458` aus
`core/analysis/featureengine/Stereo.h:70`, dieselbe wie in §23.6. **Keine neue
Warnung, keine aus der geänderten Datei.**

**Kein Nulltest, kein Passthrough-Bein, kein Kanon, kein Laufzeit-Arm** —
auftragsgemäß, weil kein Produktcode geändert ist.

### 26.5 Rotbeweise

Alle nach §6.1: SHA-256 der unveränderten Quelle, Mutation an der Zusagezeile,
Zeitstempel, Bein rot, **bytegleiche Rücknahme der Originalbytes**, Zeitstempel,
Bein grün. Skript
`docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-rotskript.ps1`; es spielt die
gesicherten Bytes zurück, nie eine zweite Textersetzung. Neu gefahren wurden
die Rotbeweise **aller vier Fälle, deren Aufbau diese Runde geändert hat** —
auch M-01a, dessen Testfall derselbe ist wie der von M-01b.

| Zeile | Bein | Mutation | Rohdatei | rot / grün |
|---|---|---|---|---|
| M-01 (a) | B14 `--nur 283m01` | MUT-A `SourcesModel.cpp:383-387` | `NAK-283-rot-M-01a-etappe-2-nacharbeit-2.txt` | Exit 1 / Exit 0 |
| M-01 (b) | B14 `--nur 283m01` | MUT-B `Ipc.cpp:1477-1478` | `NAK-283-rot-M-01b-etappe-2-nacharbeit-2.txt` | Exit 1/1/1 / Exit 0 |
| M-02 | B14 `--nur 283m02` | MUT-A | `NAK-283-rot-M-02-etappe-2-nacharbeit-2.txt` | Exit 1 / Exit 0 |
| M-06 | B14 `--nur 283m06` | MUT-A | `NAK-283-rot-M-06-etappe-2-nacharbeit-2.txt` | Exit 1 / Exit 0 |
| M-72 | B14 `--nur 283m72` | MUT-B | `NAK-283-rot-M-72-etappe-2-nacharbeit-2.txt` | Exit 1/1/1 / Exit 0 |

**Zwei neue Riegel im Rotskript**, gegen genau den Befund dieser Runde
gerichtet, je Rohdatei unter „RIEGEL DIESER RUNDE" protokolliert:

1. **Die Zusagezeile fällt in JEDEM Rotlauf.** Gemessen wird das Vorkommen der
   tragenden `FEHLER`-Zeile, nicht nur der Exitcode — M-01a
   `aeltere_mitgliederpublikation_ersetzt_keine_juengere`, M-01b `Dirty-Zaehler
   == …` **und** `Revision-Delta == 2`, M-02
   `hauptziel_benennung_wird_nicht_von_aelterer_workerkopie_ueberholt`, M-06
   `state_und_modell_sind_nach_ruhe_gleich`, M-72
   `reloadablehnung_und_ueberholung_sind_unterscheidbar`. **Ergebnis: in allen
   Rotläufen erfüllt.**
2. **Keine `FEHLER`-Zeile eines Rotlaufs enthält „Aufbau:".** Das ist die
   direkte Messung des behobenen Defekts: eine mitfallende Aufbauzeile wäre ein
   Rotbeweis an einer Zeile, die die Zusage nicht trägt (§6.1 Punkt 2).
   **Ergebnis: in allen fünf Rotbeweisen keine einzige gefallene Aufbauzeile.**
   Zusätzlich trägt jeder Grünlauf **keine** `FEHLER`-Zeile.

**Die zwei Quell-Hashes** (unverändert vor und nach jedem Rotbeweis) sind
**bytegleich mit §23.4** — das Rotskript prüft sie als Erwartungswert und bricht
vor der ersten Messung ab, wenn einer abweicht:

| Datei | SHA-256 | = §23.4 |
|---|---|---|
| `eq-copilot/plugin/src/SourcesModel.cpp` | `520A727382AAF6C0A8E1AA65D9BA973CC9E87B75825C8D2EB16CEDCB412373B3` | ja |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | `B74AC525AD97CEEC36A0E0F56AA24599ECF7782B617164845A2B562C947B4B97` | ja |

`NakamaState.cpp` steht nicht in dieser Runde: MUT-G/MUT-H/MUT-J/MUT-K
gehören zu B27, und `Sonde014IntentTest.cpp` ist unberührt.

### 26.6 Nicht neu gefahrene Rotbeweise, mit Grund

| Zeile | Gültige Rohdatei | Grund |
|---|---|---|
| M-03, M-04, M-05, M-07, M-10, M-11 (a), (b), M-71 | `…-etappe-2.txt` (§20.2, unverändert seit §23.5) | Testfall unverändert; keiner dieser Fälle hat einen Aufbau-Modellvergleich, der diese Runde betrifft. M-71 prüft das Modell schon regelkonform (`:2068` in der Wartebedingung, `:2090` hinter dem Drainriegel) |
| M-08, M-09, M-12 (Bestand), M-12 (Assistent) | `…-etappe-2-nacharbeit-1.txt` (§23.4) | liegen in `Sonde014IntentTest.cpp` / Bein B27; diese Runde fasst weder die Datei noch das Bein an — der Auftrag nennt B27 nicht, und ohne Änderung an Testfall oder Produktcode misst ein erneuter Lauf nichts Neues |

### 26.7 Produktdiff leer

```
git diff --stat 831fe896..HEAD -- eq-copilot/plugin/src eq-copilot/plugin/state broker eq-copilot/schemas eq-copilot/fixtures
```

Ausgabe: **leer** (keine Zeile).

**Warum gegen `831fe896` und nicht gegen den Basis-SHA.** Dasselbe Kommando
gegen `336ce254` war bei der Messung vor dem Push ebenfalls leer — beides ist
protokolliert. Der `git pull --rebase` vor dem Push zog danach sechs Commits
einer parallelen Session zwischen Basis und Runde (§26.1); einer davon ändert
`broker/Cargo.toml` (+2/−2). Gegen den Basis-SHA zeigt das Kommando seither
genau diese fremde Zeile und ist als Rundenbeleg nicht mehr geeignet. Der
Rundenstand `831fe896` isoliert diese Runde — **keine Zeile Produktcode.**

Der vollständige Diff der Runde (`git diff --numstat 831fe896..HEAD`) berührt
unter `eq-copilot/` ausschließlich die eine Testdatei:

```
40      12      eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp
```

Zweiter, unabhängiger Beleg: die zwei Quell-Hashes in §26.5 sind bytegleich mit
§23.4 — und das Rotskript hätte vor der ersten Messung abgebrochen, wären sie
es nicht.

### 26.8 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Abschnitt | Wo gemessen |
|---|---|
| **A** — Zähler einer Politik werden ausgewertet | Unverändert gegenüber §23.8: diese Runde ergänzt keinen Zähler und wertet keinen anders aus. Der Eintrittszähler `sourcesDrainEintritteFuerTest` wird jetzt an vier Stellen zusätzlich als Riegel **vor** dem Modellvergleich genutzt — dieselbe Rolle, die er seit §23.2 vor Baseline und Scharfschalten hat. |
| **B** — Lebenszyklus | Unberührt: keine neue Bühne, kein neuer Prozessor, kein neuer Listener. Schranke und Zähler stehen weiterhin vor dem Prozessor im Member-Block; `removeListener` läuft wie bisher. |
| **C** — Zahlenränder | Unberührt — diese Runde ändert keine Zahl, keinen Grenzwert und keinen Vergleich am Zahlenrand. M-71 (Anschlag `2^64 − 1`) ist nicht angefasst. |
| **D** — Bau- und Prüfriegel | Beinskript und Rotskript setzen den Quellzeitstempel vor dem Bau und brechen ab, wenn ein Binary älter ist als seine Quelle (NAK-230). Das Rotskript bricht zusätzlich ab, wenn das Mutationsmuster fehlt oder mehr als einmal vorkommt, wenn die Rücknahme nicht bytegleich ist **und neu: wenn ein Quell-Hash von §23.4 abweicht**. Das Beinskript bricht ab, wenn die elf Läufe mehr als einen Zählstand liefern. |
| **E** — jede neue oder geänderte Prüfung einmal absichtlich gebrochen | Die fünf Rotbeweise dieser Runde decken **jede geänderte Zusagezeile** ab, mit Rohausgabe, SHA-256-Paar und beiden Exitcodes; neu wird je Lauf geprüft, dass die tragende Zeile wirklich fällt. **Benannte Ausnahme:** die vier **neuen** Aufbau-Modellzeilen (`:1724`, `:1812`, `:1999`, `:2205`) sind **Regressionswachen, kein Beleg** — sie fallen unter keiner Mutation dieser Matrixzeilen, weil der Aufbau-Drain in keiner von ihnen überholt wird. Das ist kein Mangel, sondern ihr Zweck: sie belegen die Vorbedingung, genau wie die Standbelege (`:1740`, `:1825`, `:2010`, `:2215`) und die `ruheAbwarten()`-Zeilen seit §23.2. Eine Mutation, die sie bricht, müsste die Publikationsschleife selbst entfernen — eine erfundene Mutation außerhalb der Matrix (Muster §23.9 Punkt 1). Dass das gemessene Prädikat falsifizierbar **ist**, zeigt derselbe Vergleich als Zusagezeile von M-06: unter MUT-A fällt er. |
| **E** — Behauptung ≤ Messung | Drei eigene Kürzungen: (1) die verbliebenen Wartebedingungen heißen „ist Mitglied **im State**" statt „in State UND Modell" — sie messen nur noch den State; (2) der Nebenbefund der Validierung zur M-02-Beschriftung („A ist Mitglied", ohne die Modellhälfte zu nennen) ist damit mit geschlossen; (3) die neuen Zeilen behaupten „hinter dem Drainriegel", nicht „deterministisch bewiesen" — der Riegel ist der Eintrittszähler, und genau das steht im Kommentar. |
| **E** — Zahlen gemessen, nicht abgeschrieben | 231/231 und die elf Exitcodes stammen aus `NAK-283-etappe-2-nacharbeit-2-beine.txt`, die Zeilenzahlen `+40/−12` aus `git diff --numstat`, die Zeilennummern aus der Datei am Stand dieses Abschnitts, die Hashes aus den Rohdateien der Rotbeweise, die Frischezeiten aus dem Kopf der Beinrohausgabe. |
| **F** — Änderungssatz | Kein Speicher-, Lebenszyklus- oder Vertragspaar berührt: die Runde ändert ausschließlich die **Reihenfolge einer Messung**, kein Verhalten. Save↔Load, verbinden↔trennen und starten↔stoppen sind nicht im Änderungssatz, weil keine ihrer Hälften angefasst wurde. |
| **Stale Closures** | Die Bühne ist unverändert. Der gefangene `scharf`-Flag wird weiterhin erst nach dem vollständigen Aufbau gesetzt — und jetzt zusätzlich erst **nach** dem Modellvergleich; das Fenster, in dem der Haken den Aufbau-Ruf fangen könnte, wird durch diese Runde nicht größer, sondern bleibt unverändert geschlossen. |
| **Sperrenordnung** | Unberührt — kein Produktcode. Die neue Prüfzeile nimmt keine Sperre: `modellTraegt` liest über `sourcesPersistenteMitgliederFuerTest()`, `ruheAbwarten` über ein `std::atomic`. Der ganze Befund dieser Runde war, dass die Testbühne **keine** der beiden Sperren hält und deshalb den Zwischenstand sehen kann — die Antwort ist der Eintrittszähler, nicht eine dritte Sperre. |
| **MSVC-Stack 1 MiB** | Unberührt: kein neues Prozessorobjekt, keine neue Bühne (NAK-175). |
| **Unspezifizierte Auswertungsreihenfolge** | Die vier neuen `pruefe`-Aufrufe werten in der Detailspalte `modellBestand (*s.p)` aus — denselben Modellstand, den die Bedingung prüft, und **nach** beiden Riegeln. Anders als bei den Hostblöcken der Nacharbeit 1 findet zwischen Bedingung und Detailspalte kein Handgriff statt; die Reihenfolge ihrer Auswertung ändert die Anzeige deshalb nicht. |

### 26.9 Offene Punkte dieser Runde

1. **Die vier neuen Aufbau-Modellzeilen sind Wachen, kein Beleg** (§26.8 E).
   Sie fallen nur mit einer erfundenen Mutation, die der Auftrag ausschließt.
2. **M-26 und M-70 bleiben je zur Hälfte gebaut** (§20.7 Punkt 1), unverändert.
3. **Kein Kanon, kein Laufzeit-Arm, kein Planstand** — auftragsgemäß. Der volle
   Kanon steht am Ende der letzten Bauetappe aus.


## 27. Messung der Nacharbeit 2 und Start der Wiederprüfung 2 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `8ef94d26` (`nakama-nak283-336ce25-e2nach2`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-2-auftrag.txt` (§25); Start 19:15 Uhr, letzter Commit 19:40 Uhr; Beobachter (Aufsicht ENG): Start, Telemetrie gesund, fünf HEAD-Wechsel (vier eigene Commits, dazu der Rebase auf fremde Commits beim Push), Ende; Stundenloop `d9dfe27b` gelöscht, `CronList` leer. |
| Commits | `b89174aa` (Testdatei), `4e62ccf7` (fünf Rotbeweise, Beine, Skripte, §26), `199c29ca` und `3d6bbfb4` (Kopfzeile, Rebase-Nachtrag §26.1/§26.7), alle auf `origin/master`; Basis `336ce254` ist Vorfahr; eigene Pfade sauber. **Fremd und unberührt:** sechs Commits der parallelen Session `nakama-76` (`65cb307e` bis `831fe896`, „Anti-Drift 12.09.2026" und Planstand, 19:27 bis 19:28 Uhr) liegen durch den Rebase vor den Rundencommits; sie berühren keine Ticketpfade (`git diff --stat 336ce254..831fe896 -- eq-copilot broker docs/beweise/NAK-283.md docs/beweise/roh`: nur `broker/Cargo.toml` Kommentar +2/−2, `eq-copilot/README.md`, Archivbilder, Kalibrierprotokoll — kein Code, kein Test, kein Manifest dieses Tickets). Dieselben Commits entfernten die `.gitignore`-Regel für `nimbalyst-local/automations/*/` und tracked Dateien unter `briefing-hub/`; seither zeigt `git status --short` die fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` — nicht angefasst, Sache der Session `nakama-76`. |
| Diff | Rundenstand nach Rebase `831fe896..3d6bbfb4`: 10 Dateien, +1 319 / −13 — `Sonde012ProjectReloadTest.cpp` +40/−12, §26, acht Rohdateien. **Produktdiff leer:** `git diff --stat 831fe896..3d6bbfb4 -- eq-copilot/plugin/src eq-copilot/plugin/state broker eq-copilot/schemas eq-copilot/fixtures eq-copilot/plugin/tests/Sonde014IntentTest.cpp` ohne Zeile; die zwei Quell-Hashes der Rotbeweise sind gleich §23.4. |
| Rundenbilanz | Rein `831fe896..3d6bbfb4: Tests 1 Datei(en) +40/-12 \| Doku 9 Datei(en) +1279/-1` (die Zeile `336ce254..HEAD` trägt zusätzlich die fremden Anti-Drift-Commits: Prüfwerkzeug −2 474, Sonstiges −16 212, Produkt +2/−2 Cargo-Kommentar — nicht dieses Ticket); kumuliert `--runden bb49a612 988dc44d faea3c1a 3d6bbfb4`: „kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)". Zwei Nacharbeitsrunden ohne Produktzeilen sind hier erwartet: beide Befundrunden betrafen ausschließlich Messung, der Produktcode der Etappe ist seit `3606b0e9` unverändert und unbeanstandet. |
| Eigene Messung | Binary `EqCopSonde012ProjectReloadTest.exe` 19:31:44 Uhr, gebaut aus der Quelle mit Stand 19:31:35 Uhr (Beinrohausgabe: frisch); die Quelle trägt am Zielstand den Zeitstempel 19:38:39 Uhr allein durch den Rebase beim Push — ihr Blob ist identisch mit dem gebauten Commit (`git rev-parse b89174aa:…` = `HEAD:…` = `22880aae`). Vom Dirigenten am Stand `3d6bbfb4` um 19:41 Uhr dreimal gefahren: 231/231 grün, Exit 0 in allen drei Läufen — identisch mit den elf Läufen in `NAK-283-etappe-2-nacharbeit-2-beine.txt`. Rotbeweise gelesen: M-06 fällt am Bestandsvergleich (10/12 rot, 12/12 grün); M-01b und M-72 je drei Rotläufe, alle rot mit `[1]` an Dirty und Revision; in keinem Rotlauf fällt eine Aufbauzeile; SHA-256 vorher = nachher = §23.4. |
| Messabdeckung | Vier Fundstellen (§26.2) je mit Modellvergleich hinter State-Wartebedingung und `ruheAbwarten()`; Standbelege, Baselines, `scharf.store(true)` unverschoben (§26.3); die vier neuen Aufbau-Modellzeilen als Regressionswachen benannt und begründet (§26.8 E); M-71, M-03, M-05 unberührt; nicht neu gefahrene Rotbeweise mit Grund (§26.6). |
| Selbstbericht des Bauers (§26, nicht als Beweis gezählt) | Der Modellvergleich ist aus der `&&`-Verbindung gelöst und als eigene Zeile hinter den Drainriegel gestellt; Wartebedingungen heißen jetzt „im State"; Rotskript prüft je Lauf das Fallen der tragenden Zeile und den Hash gegen §23.4; Beinskript bricht bei abweichenden Zählständen ab. |
| Wiederprüfung 2 | Vorlage B: Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-etappe2-wieder2`); Prüfbereich `git diff 336ce254...3d6bbfb4 -- Sonde012ProjectReloadTest.cpp`, Befundliste = der eine Befund der Wiederprüfung 1 (vier Fundstellen); Auftrag `docs/beweise/roh/NAK-283-etappe-2-wiederpruefung-2-auftrag.txt`; Thread-ID und Urteil folgen in §28. **Rundenbudget:** Runde 2 von 3 geprüft; endet auch Runde 3 ohne PASS, folgt der Konvergenzentscheid (Skill §3.4). |


## 28. Wiederprüfung 2 — PASS; Abschluss der Etappe 2; Start der Etappe 3 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-etappe2-wieder2`); Thread `01a096b8-ef03-7610-a837-23698a413e8f`; Lauf 19:44 bis 19:49 Uhr, Exit 0. |
| Prüfbereich | Vorlage B: `git diff 336ce254...3d6bbfb4 -- Sonde012ProjectReloadTest.cpp` mit dem einen Befund der Wiederprüfung 1 (vier Fundstellen); HEAD `456f21b5` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. Auftrag `docs/beweise/roh/NAK-283-etappe-2-wiederpruefung-2-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/NAK-283-wiederpruefung-2-456f21b.txt`. |
| Urteil | **PASS** — wörtlich: „der Befund ist an allen vier Fundstellen geschlossen; kein Bruch im Prüfbereich festgestellt." Geprüft: alle vier Modellvergleiche eigenständig und deterministisch hinter State-Wartebedingung und vollständigem Aufbau-Drain, kein vorgezogener Modellvergleich, Standbelege/Baselines/Scharfschalten unverschoben, Produktdiff leer, M-06 am Bestandsvergleich rot, M-01b/M-72 je dreimal rot mit Delta 1, keine gefallene Aufbauzeile, Hashes identisch, Grünläufe grün, B14 elfmal 231/231. |
| Etappe 2 | **Abgeschlossen auf `3d6bbfb4`** (Produktcode seit `3606b0e9` unverändert und in drei Prüfungen unbeanstandet). Kette: Etappe 2 gebaut `988dc44d` → Erstprüfung 2 NEEDS_WORK (3 Befunde in Tests und Messabdeckung, alle bestätigt; Lücke M-08/M-09 mit Dirigentenregel) → Nacharbeit 1 `faea3c1a` → Wiederprüfung 1 NEEDS_WORK (die drei geschlossen, ein vom Fixdiff eingeführter Bruch, bestätigt, gleichartig in M-01/M-02) → Nacharbeit 2 `3d6bbfb4` → Wiederprüfung 2 PASS. Zwei Nacharbeitsrunden, Budget nicht ausgeschöpft, kein Konvergenzentscheid. Gemessen sind M-01 bis M-12, M-71, M-72 vollständig; M-26 und M-70 je Hälfte 1 (Rust in Etappe 3, Python in Etappe 6). Lehren, die in den Etappe-3-Auftrag eingehen: Rotbeweis an der Zusagezeile statt am Nebeneffekt; Aufbau eines Nebenläufigkeitsfalls deterministisch über einen Eintrittszähler, nie über Zustandsbestand oder einmaligen Modellvergleich; Host-Dirty am Grenzstand über den Prozessor. |
| Rundenbilanz | Kumuliert `--runden bb49a612 988dc44d faea3c1a 3d6bbfb4`: Etappe 2 Produkt 6 Dateien +394/−91, Tests 4 +1 099/−7; Nacharbeit 1 Tests 2 +318/−19; Nacharbeit 2 Tests 1 +40/−12 (rein `831fe896..3d6bbfb4`); „kein Konvergenz-Signal". Produktfortschritt des Tickets bisher: Etappe 2. |
| Hygiene des Fensters (19:50 Uhr) | MEMORY.md 5 358 B, CLAUDE.md 19 728 B (≤ 20 480; +769 B durch die Anti-Drift-Commits der Session `nakama-76`), Skill 24 369 B (≤ 24 576); keine Indexzeile über 250 Zeichen, keine Memory-Datei ohne Indexlink; `dokuriegel.py` auf CLAUDE.md, Skill, Register ohne Befund; `gesundheit.py` Exit 4 — Funktionen über 200 Zeilen und Kommentar-Bezeichner unverändert gerissen, bereits NAK-255 mit Nachtrag von 18:05 Uhr, keine neue Fundstelle aus den Nacharbeiten (nur Tests). Kein Riss der Kontextfläche; kein neuer Registerpunkt. Fremde untracked Ordner `briefing-hub/` und `nimbalyst-local/` (§27) bleiben Sache der Session `nakama-76`. |
| Etappe 3 | Frischer Opus-Worker (max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-3-auftrag.txt` (Ticketpfade nach §3 und §6.4 wörtlich; Matrix M-13 bis M-25, M-26 Hälfte 2, M-75 in §5.2/§5.3; Regeln R-283-1 und R-283-2; Beine A4, A4-SI, A4b, A21, A5, A8, B3c, B13; Nebenläufigkeitszeilen dreimal rot, M-75 mit Freigabegrund als Zusicherung; Manifest §29; kein Kanon, kein Laufzeit-Arm — beides am Ende der Etappe 6). Aufsicht ENG (Nebenläufigkeit, Persistenz, Sperrenordnung). Danach Erstprüfung 3 durch Codex Astra max (Vorlage A über den Etappendiff). |


## 29. Bauetappe 3 — Broker (Bauer, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **3 von 6** (F02, F03, F14). Basis `a754a99214a265fcfeee3bbccc3a844184a4bd51`; `git status --short` beim Start zeigt nur die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` (§27) — nicht angefasst, nicht committet. **Produktcode und Tests liegen auf `6ce14429`** (Commit 1 der Etappe; vor `git pull --rebase` auf `32334eb5` war es `19bcf01c`, die Blobs sind gleich); Beweise, Skripte und dieser Abschnitt folgen im zweiten Commit `f9b0c6d1`, der SHA-Nachtrag nach dem Rebase im dritten. Auftrag `docs/beweise/roh/NAK-283-etappe-3-auftrag.txt` (§28). Spezifikation: die geprüfte Matrix §5.2 (M-13 bis M-25, M-75) und §5.3 (M-26 Hälfte 2), der Bauplan §6.4, die Regeln R-283-1 und R-283-2 (§2). |
| Umfang | **14 Matrixzeilen** gebaut und gemessen: M-13 bis M-25 und M-75; dazu die Übergangshälfte M-26 (Hälfte 2, Rust-Seite). Sieben Dateien unter `broker/` geändert, +1 829 / −83 Zeilen (§29.3). |
| Beweise | **20 Rotbeweise** aus 15 Mutationen nach §6.1, jeder rot → bytegleiche Rücknahme (SHA-256 vorher = nachher) → grün; jede Nebenläufigkeitszeile aus §6.1 und dem Auftrag dreimal rot, M-75 mit Freigabegrund je Lauf (§29.2). **16 Schritte** der Beinbilanz (zwei Bauten, 13 Beine, Riegel M-25) Exit 0 (§29.4). Kein Kanon, kein Laufzeit-Arm (Ende der Etappe 6). |
| Abweichungen | **Vierzehn**, alle in §29.6 mit Grund und Quelle. Keine Matrixzeile geändert — die Prüfung entscheidet. |
| Produktfragen | Keine beantwortet. |

### 29.1 Was gebaut wurde

**F02 — die Frischeprüfung reicht bis zur Persistenz (R-283-1).** Die Rechnung
sammelt unter dem Standlock, rechnet ohne Lock und schreibt unter dem Standlock
zurück; neu ist, dass jeder Rückschreibeschritt gegen die **Eingangsmenge** der
Rechnung prüft und nicht mehr nur gegen Generation und verwendete IDs.

- `hypothese_verdrahtung.rs` `aufnahmen_sammeln` (`:195`): neben jeder Aufnahme
  entsteht ihre Eingangsmenge — die stabilen Evidence-IDs der Sitzung, die beim
  Sammeln gültig und nicht ausgeschlossen sind (`:454`, aus
  `gueltige_evidenz_ids_locked` `:1227`) — unter demselben Standlock. Im selben
  Lock löst das Sammeln den Merker `befunde_neu_bilden` ein (`:204`,
  Abweichung 12).
- **Fenster 1** (Cache): `ergebnis_ist_noch_gueltig` (`:1183`) prüft nach
  Generation und verwendeten IDs (NR-03, unverändert `Veraltet`) die
  Eingangsmenge gegen den Stand (`:1213`) — jede Abweichung, hinzugekommen oder
  verworfen, ist `Rueckschreibung::Ueberholt`. `befunde_eintragen` (`:526`)
  setzt in diesem Zweig den Merker (`:565`) und meldet den Verwurf.
- **Fenster 2** (Persistenz): der Persistenzhaken sitzt zwischen Cache-Eintrag
  und Persistenzlauf (`:590`, ohne Standlock). `befund_persistieren` (`:634`)
  nimmt den Standlock, vergleicht den Standeintrag der Sitzung in Wireform mit
  dem eingetragenen Ergebnis (`standeintrag_ist_noch` `:720`, Aufruf `:668`);
  weicht er ab, zählt es `befund_schreibversuche_unterlassen`, setzt den Merker
  (`:669-671`) und schreibt nicht. Sonst reiht es **am gehaltenen Guard** ein
  (`append_einreihen` `:678`, Annahmefehler dort gezählt) und wartet erst nach
  der Freigabe auf den Commit (`recv` `:689`).
- **Fenster 3** (Annahme, M-75): der Annahmehaken wird vor dem `lock()` per
  `take()` herausgenommen (`:657-661`) und unmittelbar vor der Annahme am
  gehaltenen Guard erreicht (`:676`) — Rendezvous nach `CoordinatorFlushTestHaken`.
- **Heilungstakt** (M-16): `hypothesen_bilden` (`:107`) ruft nach einem Verwurf
  nach R-283-1 `hypothesen_bei_bedarf_bilden` (`:109`); das ist eine Schleife
  (`:169`), die je Durchlauf den Merker einlöst und genau einmal rechnet. Ein
  Verwurf nach NR-03 setzt nur den Merker (M-14, M-15 unverändert).
- `mod.rs`: die zwei Testhakenfelder `persistenz_test_haken` (`:253`) und
  `annahme_test_haken` (`:264`) mit Settern (`:419`, `:431`), `#[doc(hidden)]`,
  im Produkt nie gesetzt. `zustand.rs`: das Zählerfeld
  `befund_schreibversuche_unterlassen` (`:399`, Abweichung 1), Zugang
  `befund_schreibversuche_unterlassen_zaehler` (`hypothese_verdrahtung.rs:1286`).

**F03 — die Ersetzung wandert ans Ende (R-283-2).** `queues.rs`
`einreihen_eintrag`: die Hochwasserprüfung je Schlüssel bleibt die erste
Entscheidung (`:234`, unverändert); die Ersetzung ist `remove(position)` plus
`push_back` unter derselben Sperre (`:251-252`). Der `cfg(test)`-Ersetzungshaken
(`:132`, `:346`) feuert nach dem Sperrabschnitt (`:283-286`, Abweichung 5).

**F14 — der Rollback benennt seine Belege über die stabile ID.**
`invalidierung_verdrahtung.rs`: `Invalidierungswirkung.zurueck` ist
`Vec<(ClientKey, String)>` (`:51`); `invalidierung_vorbereiten` merkt die
`evidence_id` (`:147`, `:182`); `invalidierung_ruecknehmen` sucht über sie und
überspringt, was die Retention entfernt hat (`:250-264`). Die Lock-Lücke
zwischen Vorbereitung und Persistenz bleibt — ihre Überbrückung ist das Merkmal.
`evidenz.rs` ist unverändert.

**Kein Schemawechsel, kein neues Feld auf der Leitung**; `broker/src/store/**`
und `coordinator/hypothese/befund.rs` unberührt (`append_einreihen` benutzt,
nicht geändert).

### 29.2 Je Matrixzeile: Test und Rotbeweis

Alle Rotbeweise nach §6.1, gefahren von
`docs/beweise/roh/NAK-283-etappe-3-rotskript.ps1`: Originalbytes und SHA-256
der Quelle sichern, Mutation an der Zusagezeile (jedes Muster genau einmal,
sonst Abbruch), Zeitstempel setzen, Neubau mit „Compiling eqcop-broker" als
Beleg, Fall fahren — ein Rotlauf zählt nur, wenn die tragende Meldung der
Zusage fällt —, Originalbytes zurückspielen (nie eine zweite Textersetzung),
Hash gegen vorher, Zeitstempel, Neubau, Fall grün. Jede Rohdatei trägt
Mutation, beide Hashes, beide Neubauten, jeden Rotlauf mit Exitcode und den
Grünlauf; die Bilanz steht in `NAK-283-etappe-3-rotbilanz.txt`.

| Zeile | Test (Datei, Name) | Bein | Rotbeweis: Mutation | Rohdatei | rot / grün |
|---|---|---|---|---|---|
| M-13 (1) | `sonde014_verdrahtung.rs:741` `neue_evidenz_waehrend_der_rechnung_verwirft_das_aeltere_ergebnis` | A4 | MUT-13a, `hypothese_verdrahtung.rs:1213-1215`: der Vergleich der Eingangsmenge entfällt (Stand am Basis-SHA) → A setzt den Bestand von B auf 12 IDs zurück | `NAK-283-rot-M-13a-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-13 (2) | `:887` `aelterer_payload_wird_nach_dem_cacheeintrag_nicht_persistiert` | A4 | MUT-13b, `hypothese_verdrahtung.rs:668`: der Wiedervergleich entfällt, die Sperre bleibt → A persistiert seinen 12-ID-Payload nach B | `NAK-283-rot-M-13b-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-75 | `:977` `annahmeordnung_folgt_der_vergleichsordnung` (drei Fäden) | A4 | MUT-75, `hypothese_verdrahtung.rs:676-678`: der Guard fällt zwischen bestandenem Vergleich und `append_einreihen`, die Annahme nimmt den Standlock neu (Fassung der Nacharbeit 1) → B reiht zuerst ein, A gewinnt die Projektion | `NAK-283-rot-M-75-etappe-3.txt` | 3× Exit 101, Freigabegrund Signal (371, 423, 777 ms) / Exit 0, Freigabegrund Frist (1 021 ms) |
| M-14 | **Regressionswache:** `:534` `veraltetes_rechenergebnis_wird_nicht_veroeffentlicht` (NR-03, unverändert) | A4 | MUT-14, `hypothese_verdrahtung.rs:1233-1235`: der Gültigkeitsfilter lässt ausgeschlossene Belege gelten | `NAK-283-rot-M-14-etappe-3.txt` | 1× Exit 101 / Exit 0 |
| M-15 | **Regressionswache:** derselbe Fall | A4 | MUT-15, `hypothese_verdrahtung.rs:1199-1204`: die Generationsprüfung entfällt | `NAK-283-rot-M-15-etappe-3.txt` | 1× Exit 101 / Exit 0 |
| M-16 (1) | `:1100` `verworfene_rechnung_zieht_genau_eine_neurechnung_nach`, Fenster 1 | A4 | MUT-16a, `hypothese_verdrahtung.rs:565`: der Merker im Zweig `Ueberholt` wird nicht gesetzt → der Stand bleibt beim älteren Ergebnis | `NAK-283-rot-M-16a-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-16 (2) | derselbe Fall, Fenster 2 | A4 | MUT-16b, `hypothese_verdrahtung.rs:671`: der Merker im Zweig des unterlassenen Schreibversuchs wird nicht gesetzt | `NAK-283-rot-M-16b-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-16 (Obergrenze) | derselbe Fall, Zählzeile von Fenster 1 | A4 | MUT-16c, `hypothese_verdrahtung.rs:108-110`: nach dem Heilungstakt läuft eine zweite Rechnung → zwei statt einer Neurechnung (Abweichung 13) | `NAK-283-rot-M-16c-etappe-3.txt` | 1× Exit 101 / Exit 0 |
| M-17 | `queues.rs:663` `koaleszierung_erhaelt_die_ordnung_ueber_schluessel` | A4 | MUT-17, `queues.rs:251-252`: die Ersetzung wieder an der alten Position (`std::mem::replace`, Stand am Basis-SHA) → Snapshot 3 vor Invalidierung 2 | `NAK-283-rot-M-17-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-18 | **Regressionswache:** `queues.rs:411` `writerqueue_snapshot_koalesziert_nach_objektschluessel` | A4 | MUT-18, `queues.rs:251-252`: anhängen statt ersetzen (Abweichung 10) | `NAK-283-rot-M-18-etappe-3.txt` | 1× Exit 101 / Exit 0 |
| M-19 | **Regressionswache:** `queues.rs:439` `aelterer_nachzuegler_ersetzt_keinen_neueren_snapshot` (nachgezogen, Abweichung 11) | A4 | MUT-19, `queues.rs:234`: `marke <= hoch` — Gleichheit wird Nachzügler | `NAK-283-rot-M-19-etappe-3.txt` | 1× Exit 101 / Exit 0 |
| M-20 (1) | **Regressionswache:** `queues.rs:701` `ersetzung_ans_ende_laesst_die_queue_nicht_wachsen` | A4 | MUT-18 (anhängen statt ersetzen) → die Deque wächst | `NAK-283-rot-M-20a-etappe-3.txt` | 1× Exit 101 / Exit 0 |
| M-20 (2) | **Regressionswache:** `queues.rs:768` `konkurrierendes_einreihen_findet_kein_fenster_ohne_schluessel` | A4 | MUT-20b, `queues.rs:251-252`: Entfernen und Anhängen in zwei Sperrabschnitten, der Haken dazwischen → zwei Einträge desselben Schlüssels | `NAK-283-rot-M-20b-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-21 | `sonde013_verdrahtung.rs:3579` `rollback_ueber_stabile_evidence_ids_laesst_fremden_ausschluss_stehen` | A4 | MUT-21, `invalidierung_verdrahtung.rs:51`, `:147`, `:166`, `:182`, `:252-262`: `zurueck` wieder über den Deque-Index (Stand am Basis-SHA) → E1 verliert seinen Ausschluss | `NAK-283-rot-M-21-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-22 | **Regressionswache:** `:3636` `rollback_nimmt_den_eigenen_ausschluss_vollstaendig_zurueck` (eigene Bühne) | A4 | MUT-22, `invalidierung_verdrahtung.rs:263`: der Rollback bricht nach dem ersten Treffer ab | `NAK-283-rot-M-22-etappe-3.txt` | 1× Exit 101 / Exit 0 |
| M-23 | `:3708` `retention_am_deckel_verschiebt_keine_rollbackzuordnung` (Stufen 31 und 32) | A4 | MUT-23, `invalidierung_verdrahtung.rs:260-261`: eine von der Retention entfernte ID bricht den Rollback ab, statt übersprungen zu werden | `NAK-283-rot-M-23-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-24 | `:3753` `cache_und_projektion_sind_nach_dem_storefehler_gleich`; dazu B16 `:3137` erweitert | A4 | MUT-21 → E1 im Cache gültig, in der Projektion ausgeschlossen | `NAK-283-rot-M-24-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-25 (Weg 1) | `sonde014_verdrahtung.rs:1204` `kein_rueckschreibeweg_des_coordinators_benennt_sein_ziel_ueber_eine_position`; dazu der Riegel im Bein (§29.4) | A4 | MUT-13a — die implizite Menge statt der Eingangsmenge | `NAK-283-rot-M-25a-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-25 (Weg 2) | derselbe Fall | A4 | MUT-21 — der Index statt der `evidence_id` | `NAK-283-rot-M-25b-etappe-3.txt` | 3× Exit 101 / Exit 0 |
| M-26 (Hälfte 2) | wie M-13 (1) | A4 | MUT-13a | `NAK-283-rot-M-26-etappe-3.txt` | 3× Exit 101 / Exit 0 |

**Die drei Hashes** (vor und nach jedem Rotbeweis gleich; SHA-256 der Bytes im
Arbeitsbaum, die Blob-IDs stehen im Kopf von `NAK-283-etappe-3-beine.txt`):

| Datei | SHA-256 |
|---|---|
| `broker/src/coordinator/hypothese_verdrahtung.rs` | `902A1B39BE793AB1BA4D376D969D2DBFBCEC644AD7E95FD9819650D60D76B40E` |
| `broker/src/coordinator/invalidierung_verdrahtung.rs` | `276D5F532BC7F5A7F2B87D89A4FC09C42203084449530DE44E59A2EFF8644719` |
| `broker/src/transport/server_v3/queues.rs` | `A2D6BCBFD937F7462F17486D5B36D3629ECAF2F937B31E0BC8F7CDFC8B2E36C6` |

**Übergangszeile, Hälfte 2.** **M-26** (F01 + F02, Rust-Seite): „Kopie unter
Sperre, Rückschreiben mit Frischeprüfung" steht in Rust über Generation plus
Eingangsmenge (M-13) und ist mit MUT-13a einmal absichtlich gebrochen; Hälfte 1
(C++, Mitgliederfolge) steht in §20.2. Beide Hälften sind damit gebaut und je
einmal gebrochen.

**Drei volle Rotläufe, gezählt wird der dritte.** Der erste (Bilanz 21:45 Uhr)
bestand 17 von 19: M-16a und M-16b blieben 0/3 grün — daraus Abweichung 12. Der
zweite (nach der Korrektur, Bilanz 22:19 Uhr) bestand 19 von 19. Danach kamen
der neu scharf stellende Zählhaken und MUT-16c (Abweichung 13) und die
zusätzlichen Rotläufe von M-17 und M-26 (Abweichung 14); der dritte Lauf
(Bilanz 22:37 Uhr) misst den Etappenstand, und nur seine Rohdateien liegen hier. Die
Rohdateien der ersten beiden sind überschrieben.

### 29.3 Geänderte Dateien

| Datei | Zeilen (+/−) | Was |
|---|---:|---|
| `broker/src/coordinator/hypothese_verdrahtung.rs` | +308 / −55 | Eingangsmenge und Merker beim Sammeln, die drei Rückschreibefenster, `Rueckschreibung`, Heilungsschleife, Wireform-Wiedervergleich, Zählerzugang |
| `broker/src/coordinator/mod.rs` | +46 / −0 | die zwei Testhakenfelder mit Settern |
| `broker/src/coordinator/zustand.rs` | +6 / −0 | Zählerfeld `befund_schreibversuche_unterlassen` (Abweichung 1) |
| `broker/src/coordinator/invalidierung_verdrahtung.rs` | +32 / −11 | `zurueck` über `evidence_id`, Rücknahme über die ID mit Übersprung |
| `broker/src/transport/server_v3/queues.rs` | +242 / −10 | Ersetzung ans Ende, `cfg(test)`-Ersetzungshaken, drei neue Fälle, M-19-Fall nachgezogen |
| `broker/tests/sonde013_verdrahtung.rs` | +493 / −6 | M-21 bis M-24 mit F14-Bühne, B16 erweitert |
| `broker/tests/sonde014_verdrahtung.rs` | +702 / −1 | M-13 (zwei Fenster), M-75, M-16, M-25 mit F02-Bühne und Zählhaken |
| `docs/beweise/NAK-283.md` | +377 / −2 | dieser Abschnitt, Kopfzeilen „Etappe" und „Basis-SHA" |

Neu unter `docs/beweise/roh/`: `NAK-283-etappe-3-beinskript.ps1`,
`NAK-283-etappe-3-beine.txt`, `NAK-283-etappe-3-beine-lauf-2.txt`,
`NAK-283-etappe-3-rotskript.ps1`, `NAK-283-etappe-3-rotbilanz.txt` und die 20
Rohdateien `NAK-283-rot-M-nn-etappe-3.txt` aus §29.2.

Nicht angefasst: `broker/src/store/**`, `coordinator/hypothese/befund.rs`,
`coordinator/evidenz.rs`, `eq-copilot/**` (Schemas bytegleich), `tools/`, alle
anderen Tests und Manifeste, `docs/PLAN-STAND.md`, `docs/offene-punkte.md`,
`design/`, die fremden Ordner `briefing-hub/` und `nimbalyst-local/`.

### 29.4 Gefahrene Beine

Skript `docs/beweise/roh/NAK-283-etappe-3-beinskript.ps1`, Rohausgabe
`docs/beweise/roh/NAK-283-etappe-3-beine.txt` (22:37 bis 23:05 Uhr). Der Kopf trägt
`git status`, `git diff --numstat` und je geänderte Quelle Blob-ID, SHA-256 und
Zeitstempel; die Release-Binaries sind gegen die jüngste geänderte Quelle
geprüft (alle frisch, NAK-230). Die C++-Beine übersetzen keine geänderte Quelle;
sie laufen, weil der Auftrag sie nennt.

| Schritt | Ziel | Exit | Messung |
|---|---|---:|---|
| BAU-cargo | `cargo build --release`: `eqcop-broker-v3probe`, `eqcop-broker-sonde012-probe`, `eqcop-broker` | 0 | Neubau (`Compiling eqcop-broker`), die drei Binaries frisch |
| BAU-cmake | sieben C++-Ziele, Release | 0 | kein Neubau nötig (keine C++-Quelle geändert), 0 `warning C` |
| A4 | `cargo test` über den Broker | 0 | 29 Testblöcke: 782 bestanden, 0 fehlgeschlagen, 23 ignoriert (1 296 s); nur die vorbestehenden Warnungen (§29.7 Punkt 5) |
| A29 | `pruefe_p5_korpus.py` (nach A4) | 0 | P5-Korpus grün (27 Sitzungen, 38 Befunde) |
| A4-SI | `store_crash_matrix -- --ignored --test-threads=1` | 0 | 23 bestanden |
| A21 | `transport_fuzz` | 0 | 12 bestanden |
| A31 | `sonde014_gegenbeispiele` | 0 | 20 bestanden |
| A5 | `pruefe_v3_vertrag.py --abdeckung` | 0 | 631 Prüfungen bestanden, 0 gescheitert |
| A8 | `erzeuge_v3_fixtures.py --pruefen` | 0 | 406 Dateien bytegleich |
| A22 | `pruefe_ipc_last.py` | 0 | GRÜN |
| A23 | `pruefe_sonde012_sources_latency.py` | 0 | GRÜN, sechs p95-Grenzen |
| A24 | `pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | 0 | GRÜN; S08: 0 Ganzblockdrops bei 220 672 Blöcken |
| A4b | `EqCopPipeClientTest` | 0 | 0 Fehler |
| B3c | `EqCopSchemaTest` | 0 | 152 bestanden, 0 gescheitert |
| B13 | `EqCopSonde012SourcesModelTest` (Consumerprobe) | 0 | 88/88 |
| Riegel M-25 | `git grep -n "zurueck: Vec<(ClientKey, usize)>" broker/src/` | 0 | leer (`git grep` Exit 1) |

**Drei Beinläufe, gezählt wird der dritte.** Der erste (vor der
Merker-Korrektur, Ende 21:34 Uhr) endete 16/16 Exit 0; seine Rohausgabe ist
überschrieben und zählt nicht, weil Produktcode und Tests danach geändert
wurden. Der zweite (nach der Merker-Korrektur, 21:52 bis 22:05 Uhr) endete
13/16: A4 fiel am Fristfall
`abgeloestes_telemetrie_getrennt_haelt_control_getrennt_nicht_auf` (265 von 266
Lib-Tests grün; `cargo test` bricht nach dem Lib-Ziel ab), A29 folgerichtig mit
„Voraussetzung fehlt", A24 an S08 (1 489 Ganzblockdrops durch Überlauf bei
221 040 Blöcken) — beides lastabhängig, während eine andere Sitzung den
C++-Vollbau über alle Ziele in `eq-copilot/build` fuhr (§29.7 Punkt 4);
Rohausgabe `docs/beweise/roh/NAK-283-etappe-3-beine-lauf-2.txt`. Der dritte Lauf
(22:37 bis 23:05 Uhr) fuhr den Etappenstand des Rotlaufs 3 nach dem Ende des fremden
Baus: 16/16 Exit 0, darunter A4 mit dem NAK-113-Fall grün und A24 mit 0 Ganzblockdrops.

### 29.5 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Abschnitt | Wo gemessen |
|---|---|
| **Sperrenordnung** (Lehre 12.09.2026) | Die Eingangsmenge entsteht unter dem Standlock (`aufnahmen_sammeln` `:454`) und wird unter dem Standlock verglichen (`ergebnis_ist_noch_gueltig`, Aufruf aus `befunde_eintragen` `:551`); der Merker wird im selben Lock eingelöst, in dem die Menge entsteht (`:204`). `befund_persistieren`: Annahmehaken vor dem `lock()` herausgenommen (`:657-661`), Wiedervergleich (`:668`) und Annahme (`:678`) unter **einem** Guard, `recv` nach der Freigabe (`:689`); der Commitfehler nimmt den Standlock danach neu, nicht geschachtelt. Unter dem Standlock nimmt kein zweiter Coordinator-Mutex: Rechen- und Persistenzhaken laufen ohne Standlock (`:140`, `:590`, `:700`); `erreichen` (`:676`) wartet nur am Hakenobjekt, und dessen Freigeber (der Kontrollfaden in M-75) nimmt den Standlock nie. Kein Pfad unter `broker/src/store/**` nimmt den Standlock (§8.1 Feinheit 18). `queues.rs`: Entfernen und Anhängen unter einer Sperre (`:251-252`), der Haken danach (`:283-286`). F14: Markierung und Rücknahme je unter dem Standlock, dazwischen die erlaubte Lücke; die Zuordnung überlebt sie über die ID. |
| **Zahlenränder** | `befund_schreibversuche_unterlassen` und `store_verweigerungen` sättigen (`saturating_add`); der Rollback zählt `invalidierungen` und `evidenz_ausgeschlossen` sättigend zurück (`invalidierung_verdrahtung.rs:280-284`); die Event-Sequenz bleibt auf `i64::MAX` begrenzt. Retention am Rand 31/32 (M-23), 100 Ersetzungen je Schlüssel über drei Schlüssel (M-20 Hälfte 1). |
| **NaN** | Der Wiedervergleich in Fenster 2 vergleicht die Wireform (`befund_json` macht Nicht-Endliches zur Vertragszahl 0). Ein Strukturvergleich wäre mit NaN mit sich selbst ungleich: jeder Schreibversuch unterbliebe, und jeder Heilungstakt verwürfe erneut — eine Endlosschleife (Abweichung 2). |
| **Stale Closures, Lebensdauer der Haken** | Jeder Coordinator-Haken der Tests fängt `Arc::downgrade` — kein Zyklus Coordinator → Haken → Coordinator. Rechen- und Persistenzhaken nehmen sich vor dem Lauf heraus und fallen genau einmal; der Zählhaken `f02_zaehlhaken` stellt sich bei jedem Lauf neu scharf und endet mit dem Coordinator. Der Annahmehaken wird vor dem Lock verbraucht; bleibt der Wiedervergleich aus, fällt er ungezogen, und der Fall meldet nach 30 s „NICHT GEMESSEN", statt zu hängen. Der Ersetzungshaken ist `FnOnce` und existiert nur im Testbau. Im Produkt ist keiner gesetzt. |
| **A** — Zähler werden ausgewertet | `befund_schreibversuche_unterlassen` in M-13 (2) und M-16 (2); die Neurechnungen in M-13, M-16 (mit Obergrenze), M-22 und M-24. |
| **A** — dieselbe Regel in allen Sprachen | M-26: Hälfte 1 (C++, §20.2) und Hälfte 2 (Rust, §29.2) sind gebaut und je einmal gebrochen. |
| **B** — Lebenszyklus | Die Fäden von M-75 und M-20 (2) enden per `join` bzw. `thread::scope`; ihre Fristen trennen nur „nicht gemessen" von einem hängenden Lauf und liegen im Test, nie im Produkt. |
| **C** — Verträge und Längen | Kein Schemawechsel, kein neues Feld auf der Leitung; A5 und A8 grün, `eq-copilot/schemas/v3/**` bytegleich. |
| **D** — Bau- und Prüfriegel | Das Beinskript bricht ab, wenn ein Release-Binary älter ist als die jüngste geänderte Quelle. Das Rotskript setzt vor jedem Neubau den Zeitstempel, verlangt „Compiling eqcop-broker" in jedem Neubau, bricht ab, wenn ein Muster nicht genau einmal vorkommt oder ein Hash abweicht, und zählt einen Rotlauf nur, wenn die tragende Meldung fällt. |
| **E** — Behauptung ≤ Messung | Die Regressionswachen M-14, M-15, M-18, M-19, M-20 und M-22 sind benannt und kein Beleg für ihren Befund. „Genau eine Neurechnung" misst die Obergrenze erst mit dem neu scharf stellenden Zählhaken und ist mit MUT-16c einmal gebrochen (Abweichung 13). M-22 misst nur, dass der Takt rechnete, und sagt nur das. B16 trägt keinen Rotbeweis (§29.6, Präzisierungen). Der erste Rotlauf von M-16 war 0/3 — daraus folgte Abweichung 12 im Produkt, kein angepasster Test. |
| **E** — Zahlen gemessen | Alle Zahlen in §29.2 und §29.4 stammen aus den Rohdateien, die Zeilen in §29.3 aus `git diff --numstat`. |
| **F** — Änderungssatz | starten↔stoppen der Rechnung (Verwurf ↔ Heilungstakt, M-16), markieren↔zurücknehmen der Invalidierung (M-21 bis M-24) und Cache↔Projektion (M-13 (2), M-16 (2), M-24, M-75) liegen je im selben Satz. |

### 29.6 Abweichungen vom Bauplan §6.4

Vierzehn; keine ändert eine Matrixzeile. Die Prüfung entscheidet.

1. **`broker/src/coordinator/zustand.rs` ist geändert; §3 und §6.4 nennen ihn
   nicht.** Grund: M-13 (2) sagt zu, dass A's unterlassener Schreibversuch
   gezählt ist, und §6.4 verlangt das Zählen am gehaltenen Guard. Der Zähler
   gehört in `Stand`, und `Stand` ist in `zustand.rs` deklariert. Umfang: ein
   Feld mit Doc und sein Default (+6/−0), kein Verhalten.
2. **Wiedervergleich in Wireform statt Strukturgleichheit.** M-13 nennt
   „derselbe Vergleich wie `:465`" — das ist `PartialEq` auf dem Befund. Grund:
   mit einem nicht-endlichen Wert wäre ein Befund mit sich selbst ungleich,
   jeder Schreibversuch unterbliebe, und jeder Heilungstakt verwürfe erneut.
   `standeintrag_ist_noch` (`:720`) vergleicht deshalb `befund_json` — die Form,
   die persistiert wird.
3. **Der Heilungstakt folgt nur den zwei Verwurfzweigen nach R-283-1.** Ein
   Verwurf nach NR-03 (Generation, zurückgenommene verwendete ID) setzt wie bisher
   nur den Merker, und das nächste Material rechnet (M-14, M-15; `intent.rs:592-600`
   „ohne dass Zahlen nachgerechnet werden"). **Folge in Fenster 2:** landet eine
   Evidenzrücknahme oder eine Intentänderung genau zwischen Cache-Eintrag und
   Persistenz, verändert sie den Standeintrag, der Schreibversuch unterbleibt,
   und der Heilungstakt rechnet sofort neu. Für die Rücknahme ist das gemessen
   (M-16 (2): der Befund steht danach ohne den Beleg 105 und nicht `stale`), für
   die Intentänderung ist es Analyse. Außerhalb des Fensters bleibt ein
   zurückgenommener Befund `stale`, bis Material kommt (SONDE-014 M-24,
   `invalidierung_verdrahtung.rs:204-211`). M-16 verlangt die Neurechnung für den
   unterlassenen Schreibversuch ausdrücklich; offen für die Prüfung (§29.7).
4. **Die Mengenprüfung gilt auch für ein leeres Ergebnis.** Generation und
   verwendete IDs prüft `ergebnis_ist_noch_gueltig` wie bisher nur bei
   Befunden; die Eingangsmenge dagegen immer (`:1213`). Grund: ein leeres
   Ergebnis über einer überholten Menge entfernte sonst den jüngeren Befund aus
   dem Stand (`befunde_eintragen` `:579-581`).
5. **M-20: der Ersetzungshaken feuert in der Fixfassung nach dem
   Sperrabschnitt**, §6.4 sagt „zwischen Entfernen und Anhängen". Grund: in der
   Fixfassung gibt es dieses „zwischen" nicht; ein Haken unter der Sperre hielte
   den zweiten Faden am Lock fest, und der Fall käme nur über eine Frist frei —
   ein Interleaving, das nicht erzwungen, sondern abgewartet wäre. Die gebrochene
   Fassung (MUT-20b) feuert ihn zwischen ihren zwei Sperrabschnitten.
6. **Die F14-Bühnen erzwingen die Lücke ohne SQLite BUSY.** M-21 bis M-24 nennen
   „der Store-Append scheitert (SQLite BUSY)". Gebaut ist das Interleaving über
   den Flush-Haken am Sessionschloss, die Vorschau (P0) und die erzwungene
   Storenaht (`append_naht_setzen`, der bestehende B16-Weg): der Flush hält das
   Sessionschloss, die Vorschau markiert vorläufig und wartet, der nächste Beleg
   löst die Retention aus, der Append scheitert. Grund: BUSY ist nicht
   deterministisch erzwingbar. Der Takt nach dem Storefehler (M-24,
   `invalidierung_verdrahtung.rs:126`) kommt aus einer zweiten, trefferlosen
   Invalidierung an derselben Naht, weil der Vorschau-Fehlerzweig keinen Takt
   anstößt (§29.7 Punkt 5).
7. **M-22: „`genommene_ids` ist leer" ist nicht beobachtbar** — die Menge ist
   eine lokale Variable von `invalidierung_vorbereiten`, kein Stand. Gemessen
   sind beide Belege, `evidenz_ausgeschlossen`, `invalidierungen` und der Takt
   nach dem Storefehler.
8. **Fünf Beine mehr, als §6.4 nennt: A22, A23, A24, A29, A31.** A22 bis A24
   fahren Release-Probes, die Coordinator und `queues.rs` übersetzen; A31
   übersetzt den Coordinator; A29 liest das Ergebnis, das A4 über den
   Produktpfad schreibt. Der Auftrag verlangt jedes Bein, das eine geänderte
   Quelle übersetzt oder linkt.
9. **Die Mutanten der Wachen M-14 und M-15.** M-14 bricht den
   Gültigkeitsfilter (`gueltige_evidenz_ids_locked` ohne Ausschlussprüfung),
   nicht nur die Prüfung der verwendeten IDs: die neue Mengenprüfung umfasst
   diese, und eine allein entfernte ID-Prüfung bliebe grün. M-15 entfernt die
   Generationsprüfung; der bestehende NR-03-Fall kann an zwei Zeilen fallen, beide
   sind als tragende Meldung zugelassen.
10. **M-18 fällt am Warten auf das `false` des Ersetzten**
    (`queues.rs:420`, `recv_timeout(..).unwrap()` mit `Timeout`): mit Anhängen
    statt Ersetzen kommt es nie. Das ist die Zusage „der ersetzte bekommt
    `false`" des bestehenden Falls, keine Nebenzeile.
11. **M-19: der bestehende Fall ist nachgezogen.**
    `aelterer_nachzuegler_ersetzt_keinen_neueren_snapshot` prüfte die Position
    der Ersetzung — genau die Ordnung, die R-283-2 ändert. Die erwarteten
    Positionen nach den Ersetzungen und die Schreibreihenfolge folgen jetzt der
    Ersetzung ans Ende; die Nachzüglerzusage ist unverändert.
12. **Der Merker wird beim Sammeln eingelöst** (`aufnahmen_sammeln` `:196-204`,
    Produkt, nach dem ersten Rotlauf). **Gemessen:** MUT-16a und MUT-16b blieben
    im ersten vollen Rotlauf 0/3 grün (Bilanz 21:45 Uhr). **Ursache:** jede
    rechnende Sitzung braucht die Vollständigkeitsmarke (NR-01), und ihre
    Übernahme setzt `befunde_neu_bilden` (`intent.rs:626`); das unbedingte Rechnen
    aus `evidenz.rs:232` löste ihn nie ein. Der Heilungstakt fand deshalb in jeder
    Sitzung einen liegengebliebenen Merker, und nicht das Flag des Verwurfs trug
    die Neurechnung. **Jetzt** löst das Sammeln ihn unter demselben Standlock ein,
    unter dem die Aufnahme entsteht; jede spätere Änderung setzt ihn neu. Andere
    Leser gibt es nicht: `hypothesen_bei_bedarf_bilden` ist der einzige, und seine
    beiden Aufrufer (`:109`, `invalidierung_verdrahtung.rs:126`) folgen einem
    Zweig, der den Merker selbst setzt. Danach je 3/3 rot.
13. **Der Zählhaken stellt sich neu scharf** (Tests). Der Rechenhaken fällt
    genau einmal (`mod.rs:241-243`); ein einmaliger Zähler maß in M-13 und M-16
    nur „mindestens eine" Neurechnung, obwohl die Meldungen „genau eine"
    sagten. `f02_zaehlhaken` zählt jede Rechnung nach dem Setzen; die Obergrenze
    ist mit MUT-16c (eine zweite Rechnung nach dem Heilungstakt) einmal
    gebrochen. M-22 fährt auf dem Harnisch ohne `Arc` und kann nicht neu scharf
    stellen; seine Meldung sagt deshalb nur noch, dass der Takt rechnete.
14. **Mehr Rotläufe, als der Auftrag nennt:** M-17 und M-26 dreimal, weil §6.1
    sie zu den Nebenläufigkeitszeilen zählt; dazu MUT-16c als eigener Rotbeweis
    der Obergrenze aus Abweichung 13.

Präzisierungen ohne Abweichungscharakter:

- **Dateinamen** folgen dem Etappenauftrag (`NAK-283-rot-M-nn-etappe-3.txt`),
  nicht §6.0. Zeilen mit mehreren Fällen oder Mutationen tragen ein Suffix
  (M-13a/b, M-16a/b/c, M-20a/b, M-25a/b).
- **Quellvalidierung:** der Auftrag nennt „Teil B"; F02, F03 und F14 stehen in
  `NAK-283-quellvalidierung.md` **Teil A** (`:12`, `:193`, `:326`). Gelesen wurde
  der Teil, der sie trägt.
- **B16** (`preview_und_ruecknahme_liegen_in_einem_append`, M-24) ist erweitert:
  eigene Positionen für die Belege 10 bis 12, eine frühere Rücknahme von Beleg
  11, und statt des `all` über `ausschlussgrund.is_none()` je Beleg der Vergleich
  mit der Projektion. B16 trägt keinen eigenen Rotbeweis: ohne
  Retention-Verschiebung trifft auch ein Index die richtigen Belege (Analyse);
  der Rotbeweis von M-24 liegt am neuen Fall.
- Die Zeilenangaben der Matrix beziehen sich auf den Basis-SHA, die Zeilen in
  diesem Abschnitt auf den Etappenstand `6ce14429`.

### 29.7 Offene Punkte dieser Etappe

1. **M-26 ist in beiden Hälften gebaut** (C++ §20.2, Rust §29.2); als
   geschlossen gilt die Zeile erst nach der Prüfung.
2. **Fenster-2-Heilung gegen SONDE-014 M-24 und §37.3** (Abweichung 3): eine
   Rücknahme oder Intentänderung, die genau zwischen Cache-Eintrag und
   Persistenz landet, wird sofort neu gerechnet; außerhalb des Fensters bleibt der
   Befund bis zum nächsten Material `stale`. Ob das Fenster die Regel „terminal,
   ohne Nachrechnen" ausnehmen darf, entscheidet die Prüfung.
3. **Heilungstakt unter dichtem Evidenzstrom (Analyse, nicht gemessen).** Die
   Rechnung läuft synchron auf dem Faden, der den Beleg annimmt
   (`evidenz.rs:232`), und der Heilungstakt ist eine Schleife auf demselben
   Faden. Liefern mehrere Links einer Sitzung so dicht, dass während fast jeder
   Rechnung ein anderer Beleg eintrifft, wird fast jede Rechnung überholt: jeder
   Verwurf zieht eine Neurechnung nach sich (M-16), der Faden nimmt so lange
   keinen eigenen Beleg an, und der Stand bekommt bis zu einer Pause kein neues
   Ergebnis. Die Schleife endet mit der ersten Rechnung ohne Eingangsänderung;
   ein Verwurf durch einen neuen Beleg bräuchte die Neurechnung nicht, weil der
   annehmende Faden selbst rechnet. Die Bühnen dieser Etappe erzwingen je genau
   einen Verwurf; Rechendauer gegen Belegabstand aller Links einer Sitzung ist
   nirgends gemessen. Ob R-283-1 eine Grenze der Heilungstakte verlangt oder die
   Neurechnung nach einem Verwurf durch einen selbst rechnenden Beleg auslassen
   darf, entscheidet die Prüfung.
4. **Lastabhängige Beine außerhalb dieser Etappe.** Im zweiten Beinlauf fielen
   A4 am Fristfall `abgeloestes_telemetrie_getrennt_haelt_control_getrennt_nicht_auf`
   (`broker/src/transport/server_v3/tests_kopplung.rs:260`, Register NAK-113:
   ein Fehlschlag in 13 Suite-Läufen, gemessen im G1-Gate) und A24 an S08
   (Ganzblockdrops durch Überlauf auf dem C++-Sondenpfad), während eine andere
   Sitzung den C++-Vollbau in `eq-copilot/build` fuhr. Keinen der beiden Pfade
   ändert diese Etappe; im dritten Lauf ohne fremden C++-Bau waren beide grün (§29.4).
   NAK-113 nennt als Fix, den Zähler
   abzuwarten statt sofort zu lesen — der Fall wartet inzwischen bis
   `SENKE_FRIST * 3` (`tests_kopplung.rs:260-267`) und fiel trotzdem. Das ist ein
   Nachtrag für das Register und Sache des Dirigenten.
5. **Nebenbefunde, nichts geändert.** (a) `befehl.rs:912-917`: scheitert der
   Append einer Vorschau, nimmt der Fehlerzweig den Ausschluss zurück und setzt
   den Merker, stößt aber keinen Takt an — die zurückgenommenen Befunde bleiben
   `stale` bis zum nächsten Material, genau der Zustand, den
   `invalidierung_verdrahtung.rs:121-126` für den anderen Weg ausschließt.
   (b) `befunde_eintragen` vergleicht `alt == neu` weiter mit `PartialEq`
   (`:573`): mit einem nicht-endlichen Wert würde ein gleicher Stand erneut
   eingetragen und persistiert — ohne Verwurf, also ohne Heilungsschleife.
   (c) Die Warnungen in `sonde013_verdrahtung` (5) und `sonde014_gegenbeispiele`
   (1) sind vorbestehend: keine ihrer Stellen (`clock`, `sonde_mit_evidenz`,
   dreimal `let mut haelfte`, `Buehne::snapshot`) liegt im Diff dieser Etappe
   (`git diff -U0` gemessen). (d) Die Annahme eines Belegs (`evidenz.rs:179-189`)
   prüft nicht, ob seine `evidence_id` schon in der Historie steht. Käme dieselbe
   ID zweimal hinein, fände die Rücknahme über `find` nur den ersten Eintrag. Die
   Projektion führt die ID als Schlüssel (`store/writer.rs:571`), und der
   bestehende Ingest-Rollback (`evidenz.rs:218`) behandelt sie ebenso als
   Identität; ob ein Sender eine ID wiederholen kann, ist hier nicht geprüft.
   Eindeutig je Eintrag wäre die Ankunftsfolge `empfangsfolge` (`evidenz.rs:184`).
6. **Kein Kanon, kein Laufzeit-Arm** — auftragsgemäß am Ende der Etappe 6.
   `docs/PLAN-STAND.md` ist nicht angefasst, `planstand.py` nicht gelaufen.


## 30. Messung der Etappe 3 und Dirigenten-Übergabe vor der Erstprüfung 3 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `d69358bd` (`nakama-nak283-a754a99-etappe3`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-3-auftrag.txt` (§28); Start 19:51 Uhr, Abschlussmeldung 23:12 Uhr, vom Dirigenten um 23:16 Uhr gestoppt und entfernt; Beobachter (Aufsicht ENG): Start, Telemetrie gesund, wiederholt „Statusquelle unbekannt: Zeitlimit" unter Bau-Last (Fehlalarme, `claude agents` antwortete während cargo-Bauten verzögert), vier HEAD-Wechsel, Ende; Stundenloop `b706f7cc` gelöscht, `CronList` leer. Laufzeit 3 h 21 min — an der Grenze aus Skill §3.3; die Abschlussmeldung ist vollständig und deckt sich mit §29. |
| Commits | `6ce14429` (Produkt und Tests; vor dem Rebase `19bcf01c`, Blobs gleich), `f9b0c6d1` (20 Rotbeweise, zwei Beinläufe, Skripte, Rotbilanz, §29), `30a0b1bf` (Kopfzeile, SHA-Nachtrag), alle auf `origin/master`; Basis `a754a992` ist Vorfahr; eigene Pfade sauber (nur die fremden untracked Ordner aus §27). **Fremd und unberührt:** `ac1ea6e8` („Designteil geparkt (User-Wort 12.09.2026)") und `32334eb5` (Planstand) einer parallelen Session um 20:07 Uhr — nur `docs/plan/`, `docs/offene-punkte.md`, `design/`; kein Ticketpfad. |
| Diff | `git diff --stat 32334eb5..30a0b1bf`: 33 Dateien, +8 692 / −85 — fünf Produktdateien unter `broker/src/` (+634/−76: `hypothese_verdrahtung.rs` +363, `queues.rs` +252, `mod.rs` +46, `invalidierung_verdrahtung.rs` +43, `zustand.rs` +6 — letzterer außerhalb der §6.4-Liste, Abweichung 1 in §29.6), zwei Testdateien (+1 195/−7), §29 und 26 Rohdateien. `eq-copilot/**`, `broker/src/store/**`, `eq-copilot/schemas/**`, `tools/**` unberührt (gemessen, leer). |
| Rundenbilanz | `32334eb5..30a0b1bf: Produkt 5 Datei(en) +634/-76 \| Tests 2 Datei(en) +1195/-7 \| Doku 26 Datei(en) +6863/-2` — zweite Produktrunde des Tickets. |
| Eigene Messung | Rotbilanz `NAK-283-etappe-3-rotbilanz.txt` gelesen: 20 Zeilen BESTANDEN, Nebenläufigkeitszeilen 3/3 rot (Exit 101/101/101), M-75 dreimal „Signal", grün „Frist", drei Quell-Hashes vorher = nachher. Beine `NAK-283-etappe-3-beine.txt`: 16 von 16 Schritten Exit 0 (A4 782 bestanden, 0 fehlgeschlagen, 23 ignoriert); `…-beine-lauf-2.txt` ist ein roter Zwischenlauf unter fremder C++-Bau-Last (A4 Fristfall NAK-113, A24 S08 Audio-Drops), im dritten Lauf ohne fremde Last grün — Verlauf, kein Beleg. Vom Dirigenten am Stand `30a0b1bf` selbst gefahren (23:16 bis 23:33 Uhr, `cargo test --release` im Broker, 1 060 s): Exit 0, alle Suiten grün (u. a. 67 bestanden/23 ignoriert in der Lib-Suite, 62, 37, 21, 20, 12 in den Integrationssuiten; 0 fehlgeschlagen) — ohne fremde Last kein Fristfall. |
| Messabdeckung | 14 Matrixzeilen (M-13 bis M-25, M-75) plus M-26 Hälfte 2 je mit Test und Rotbeweis; Wachen (M-14, M-15, M-18, M-19, M-20a, M-22, M-16c) als solche benannt; M-26 damit in beiden Hälften gebaut (C++ §20.2, Rust §29.2) — abgeschlossen erst mit PASS der Erstprüfung 3; M-70 Hälfte 2 bleibt Etappe 6. |
| Noch nicht eingeordnet (nächste Session, vor der Erstprüfung 3) | (1) Die **vierzehn Abweichungen** in §29.6 — je gedeckt / Lücke mit Regel / Befund; (2) **§29.7 Punkt 2**: die Fenster-2-Heilung (Rücknahme oder Intentänderung zwischen Cache-Eintrag und Persistenz wird sofort neu gerechnet) gegen SONDE-014 M-24 und §37.3 („terminal, ohne Nachrechnen") — Lücke mit Dirigentenregel, Widerspruchszeile in `docs/offene-punkte.md` oder Befund gegen M-16; (3) `zustand.rs` (+6) an der Quelle gegen §6.4 lesen. Der Prüfauftrag liegt vorbereitet als `docs/beweise/roh/NAK-283-etappe-3-erstpruefung-auftrag.ENTWURF.txt` mit Platzhaltern für genau diese drei Punkte und die Zeilennummern aus §29.2. |
| Register | NAK-113 datiert nachgetragen (Fristfall fiel im Beinlauf 2 unter fremder Last trotz `SENKE_FRIST * 3`, §29.7 Punkt 4); NAK-283 Nachtrag: Etappe 2 abgeschlossen, Etappe 3 gebaut, Übergabe. |
| Übergabe | Kontext dieser Session 49 % vor dieser Messung (Skill §5: ab 500k keine neue Prüfrunde). Nächste Session: `git pull --ff-only`, Anker `NAK-283`, §29 lesen, Punkte oben einordnen, Prüfauftrag füllen und als `NAK-283-etappe-3-erstpruefung-auftrag.txt` ablegen, §31 „Start der Erstprüfung 3" schreiben, committen, `codex-lauf.ps1 -Kennung nak283-etappe3-erst` (Astra max), Monitor auf das Startlog; danach Validierung, Einordnung, Nacharbeit oder Etappe 4 (§6.5: Hostvertrag und DSP-Kante, M-27 bis M-43; Aufsicht ENG). Kanon und Laufzeit-Arm erst am Ende der Etappe 6. Kein Worker läuft; `CronList` leer; Beobachter beendet; Worker `d69358bd` entfernt. |


## 31. Einordnung der Etappe 3 und Start der Erstprüfung 3 (Dirigent, 12.09.2026)

| Merkmal | Wert |
|---|---|
| Übernahme | Neue Dirigentensession 23:35 Uhr (Marker-Neustart nach §30); `git pull --ff-only` ohne Änderung; HEAD `1e2822ed` = Etappenstand `30a0b1bf` plus Planstand- und Registercommits (`git diff --stat 30a0b1bf..1e2822ed -- broker eq-copilot` leer, gemessen); `git status --short` nur die fremden untracked Ordner (§27); `claude agents --json`: kein Worker, eine fremde interaktive Session `nakama-f7` (seit 20:59 Uhr, keine Änderung an tracked Dateien) — nicht angefasst; `CronList` leer; Anker `NAK-283` gesetzt. Telemetrie 23:36 Uhr: Kontext 10 %, Claude 5 h 14 %, Claude 7 Tage 38 %, Codex 7 Tage 32 %. |
| Abweichungen §29.6 (Einordnung) | **Elf gedeckt, drei Lücken mit Regel, kein Befund** — je an Diff und Quelle gelesen (`hypothese_verdrahtung.rs`, `zustand.rs`, `intent.rs`, `invalidierung_verdrahtung.rs`, `queues.rs`; Stand `1e2822ed`, Produktcode `6ce14429`). **Gedeckt:** 1 (`zustand.rs` +6: nur das Zählerfeld `befund_schreibversuche_unterlassen` mit Doc und Default 0, `:395-399`, `:614`; §6.4 verlangt „eigenen Zähler erhöhen", M-13 (2) „A's unterlassener Schreibversuch ist gezählt", der Zähler lebt in `Stand`, §3 nennt `broker/src/` (Coordinator) — Ticketgrenze in engster Form, kein Verhalten); 4 (Mengenprüfung auch für ein leeres Ergebnis — genau R-283-1 „nie über ein jüngeres Ergebnis geschrieben"; Generation und verwendete IDs weiter nur bei Befunden, NR-03 unverändert); 5 (Ersetzungshaken nach dem Sperrabschnitt — §6.4 selbst: „in der Fixfassung liegt zwischen beiden nichts, was ihn feuern ließe"; M-20 Hälfte 2 dreimal rot gegen die Zweisperren-Fassung); 6 (F14-Bühnen erzwingen den Storefehler über Flush-Haken, Vorschau und `append_naht_setzen` statt SQLite BUSY — die Zusage ist der Rollback nach dem Storefehler, die Ursache des Fehlers ist Bühne, §6.1 verlangt Determinismus); 7 (`genommene_ids` ist lokal und nicht beobachtbar; beide Belege, `evidenz_ausgeschlossen`, `invalidierungen` und der Takt sind gemessen); 8 (fünf Beine mehr — Mehrmessung, vom Auftrag verlangt); 9 (M-14-Mutant bricht den Gültigkeitsfilter statt nur die ID-Prüfung, weil die Mengenprüfung `:1213` eine allein entfernte ID-Prüfung deckt; die ID-Prüfung `:1205-1211` ist nicht tot — sie bestimmt die Verwurfklasse `Veraltet` ohne Heilungstakt, M-14/M-15, gegen `Ueberholt`); 10 (M-18 fällt am `false` des Ersetzten — die Zusage der Zeile); 11 (M-19-Fall nachgezogen: die alte Fassung kodierte die Position der Ersetzung, genau die Ordnung, die R-283-2 ändert; Nachzüglerzusage unverändert — der Prüfer prüft, dass nur die Positionserwartung geändert ist); 13 (Zählhaken stellt sich neu scharf — misst die Obergrenze „genau eine" aus M-16, mit MUT-16c gebrochen); 14 (mehr Rotläufe — §6.1). **Lücke mit Regel, keine Nacharbeit:** 2, 3, 12 (nächste Zeilen). |
| Regel zu Abweichung 2 (Wireform-Wiedervergleich) | M-13 (2) nennt „derselbe Vergleich wie `:465`" (`PartialEq`) als Mechanik; die Zusage ist „A schreibt nicht … flüchtiger Stand und Projektion sind gleich". Mit einem nicht-endlichen Wert wäre ein Befund per `PartialEq` mit sich selbst ungleich, jeder Schreibversuch unterbliebe, und jeder Heilungstakt verwürfe erneut — eine Endlosschleife (`CLAUDE.md` NaN-Ehrlichkeit). **Regel (12.09.2026):** der Wiedervergleich in Fenster 2 vergleicht die Wireform (`befund_json`, `standeintrag_ist_noch` `:720-728`) — die Form, die persistiert wird, Länge und Reihenfolge eingeschlossen; sie trägt `zustand` (`:869`) und `evidence_ids` (`:810`) und macht Nicht-Endliches zur Vertragszahl 0. Gilt zu M-13 (2). |
| Regel zu Abweichung 12 (Merker beim Sammeln eingelöst) | Matrix und §6.4 nennen keinen Einlöseort für `befunde_neu_bilden`; ohne das Einlösen beim Sammeln blieb der Merker der Vollständigkeitsmarke (`intent.rs:626`) über jede Rechnung stehen, und der Heilungstakt fand ihn in jeder Sitzung (MUT-16a/b 0/3 grün im ersten Rotlauf). **Gemessen** (`git grep -n befunde_neu_bilden -- broker/src`): fünf Setzer (`hypothese_verdrahtung.rs:554`, `:565`, `:671`, `intent.rs:626`, `invalidierung_verdrahtung.rs:279`), zwei Leser (`:173` Heilungsschleife per `take`, `:204` Sammeln), kein weiterer. **Regel (12.09.2026):** der Merker wird unter demselben Standlock eingelöst, unter dem die Aufnahme entsteht (`aufnahmen_sammeln` `:204`); jeder Setzer nach dem Sammeln setzt ihn neu; einziger Verbraucher ist `hypothesen_bei_bedarf_bilden`. Außerhalb der Naht verhaltensgleich: den Merker der Vollständigkeitsmarke löste vorher wie nachher erst das nächste Material oder ein Takt ein. Gilt zu M-16. |
| §29.7 Punkt 2 — Fenster-2-Heilung (Abweichung 3) | **LÜCKE mit Dirigentenregel R-283-7; kein Widerspruch für `docs/offene-punkte.md`; kein Befund gegen M-16.** An der Quelle: eine Rücknahme im Fenster setzt den Standeintrag auf `stale` (`befunde_invalidieren_locked` `:906`), eine Intentänderung ebenso (`befunde_veralten_locked` `:1248`); die Wireform trägt `zustand` (`:869`), der Wiedervergleich fällt (`:668`), der Schreibversuch unterbleibt, der Merker steht (`:671`), und `hypothesen_bilden` (`:107-111`) rechnet sofort neu — gemessen für die Rücknahme (M-16 Fenster 2, `sonde014_verdrahtung.rs:1134-1183`: Befund ohne Beleg 105, nicht `stale`, genau eine Neurechnung, Stand gleich Projektion). M-13 (2) und M-16 unterscheiden die Ursache der Standänderung nicht und verlangen die Neurechnung „ohne dass neue Evidenz eintreffen muss"; SONDE-014 M-24 („terminal invalidieren") und Entwurf §37.3 („stale, nicht still umgerechnet"; `invalidierung_verdrahtung.rs:204-211`, `intent.rs:592-600`) beschreiben den Zustand eines veröffentlichten Befunds nach Rücknahme oder Rollenänderung, nicht eine noch nicht persistierte Rechnung in der Naht. Ohne die Neurechnung stünde im Fenster ein nie persistierter `stale`-Standeintrag gegen die Projektion — genau die Ungleichheit, die M-13 (2) ausschließt. **R-283-7 (Technik, 12.09.2026):** Innerhalb der Rückschreibenaht (zwischen Cache-Eintrag und Persistenz) gilt R-283-1 vollständig — jeder unterlassene Schreibversuch zieht genau eine Neurechnung nach sich, gleich ob der Standeintrag durch jüngere Evidenz, eine Rücknahme oder eine Intentänderung ersetzt wurde; die Neurechnung steht auf dem jüngsten Eingangsstand (ohne den zurückgenommenen Beleg, unter der aktuellen Generation, nur mit Vollständigkeitsmarke) und trägt nur ein, was vom Standeintrag abweicht. Außerhalb der Naht bleiben SONDE-014 M-24 und §37.3 unverändert: `stale` bis neues Material, kein Nachrechnen. Für die Intentänderung ist die Regel Analyse, nicht gemessen — der Prüfer prüft sie lesend; eine Messung ist Härtung, kein Defekt. Produktwirkung: keine — außerhalb eines Fensters von Millisekunden ändert sich nichts Sichtbares. |
| §29.7 Punkte 3 bis 5 | Punkt 3 (Heilungstakt unter dichtem Evidenzstrom): **LÜCKE** — R-283-1 verlangt keine Grenze der Heilungstakte; die Schleife (`:169-179`) endet mit der ersten Rechnung ohne Eingangsänderung, ein selbst rechnender Beleg macht die Heilung des überholten Fadens zur Doppelrechnung, nicht zum Fehler; Rechendauer gegen Belegabstand ist nirgends gemessen → **Härtung**, Register **NAK-287**. Punkt 4: NAK-113 bereits nachgetragen (§30). Punkt 5 (a) Vorschau-Fehlerzweig `befehl.rs:912-917` ohne Heilungstakt, (b) `PartialEq`-Bestandsvergleich `:573` bei Nicht-Endlichem, (d) doppelte `evidence_id` in der Annahme ungeprüft: **Härtung**, alle vorbestehend und außerhalb der Ticketpfade dieser Etappe → NAK-287; (c) vorbestehende Warnungen: keine Fundstelle im Diff, kein Punkt. |
| Erstprüfung 3 | Vorlage A: Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`tools/dirigent/codex-lauf.ps1`, Kennung `nak283-etappe3-erst`); Prüfbereich `git diff a754a992...30a0b1bf -- <die sieben Ticketpfade>`; Auftrag `docs/beweise/roh/NAK-283-etappe-3-erstpruefung-auftrag.txt` (aus dem Entwurf §30 gefüllt: Riegelzeilen am Stand `1e2822ed` gemessen und gleich §29.2; die vierzehn Abweichungen mit dieser Einordnung; R-283-7; Quellvalidierung Teil A; Ausschlüsse `docs/**`, NAK-269, NAK-249, NAK-113, NAK-287, Etappen 2, 4, 5, 6, SONDE-015 §12, §29.7 Punkte 3 bis 5; der Entwurf bleibt als Verlauf liegen). Thread-ID und Urteil folgen in §32. |


## 32. Erstprüfung 3 — NEEDS_WORK mit einem Befund; Validierung, Einordnung als Lücke, Abschluss der Etappe 3; Start der Etappe 4 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-etappe3-erst`); Thread `01a09799-2733-7db0-b878-4450b8ee1fd8`; Lauf 12.09.2026 23:49 bis 13.09.2026 00:02 Uhr, Exit 0. |
| Prüfbereich | Vorlage A: `git diff a754a992...30a0b1bf -- <die sieben Ticketpfade>`; HEAD `34a127e5` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. Auftrag `docs/beweise/roh/NAK-283-etappe-3-erstpruefung-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/NAK-283-erstpruefung-3-34a127e.txt`. |
| Urteil des Prüfers | **NEEDS_WORK** — wörtlich: „ein DEFEKT im sitzungsübergreifenden Heilungstakt." Geprüft: vollständiger Sieben-Dateien-Diff, Sperrenordnung, Haken, Merker, Queue, Rollback, Matrixbezug, 20 Rotprotokolle und Beinrohdaten, geschützte Quellpfade diffleer; nicht geprüft: Builds, Testläufe, Laufzeit. Kein Befund gegen Eingangsmenge, Fenster 2, Annahmeordnung (M-75), Queue-Ordnung, Rollback über stabile IDs, Wachen oder Rotbeweise. |
| Befund 1 | `hypothese_verdrahtung.rs:108-109`: bei zwei Sitzungen mit Vollständigkeitsmarke löst ein `Ueberholt`-Verwurf in S1 den Heilungstakt aus, dessen `aufnahmen_sammeln` alle Sitzungen aufnimmt; S2, deren Befund durch eine Intentänderung `stale` wurde (M-15, `Veraltet`), wird ohne eigenes Material frisch gerechnet und verliert `stale` — gegen R-283-7 („außerhalb der Naht stale bis neues Material, kein Nachrechnen"); BASIS hatte den Folgeaufruf nicht. |
| Validierung | Ein lesender Opus-Agent (00:04 bis 00:24 Uhr, Stand `34a127e5`, keine Repros, keine Builds): `docs/beweise/roh/NAK-283-erstpruefung-3-validierung.md`. Alle Teilbehauptungen des Ablaufs am Ziel sind wahr (globaler Merker `zustand.rs:363`, ODER über Sitzungen `:141-157`, `aufnahmen_sammeln` ohne Sitzungsfilter `:208-231`, Takt `:107-111`, eine frische Rechnung ist nie veraltet). **Am BASIS `a754a992` rechnete jeder Materialtakt (`evidenz.rs:232`) ebenso alle Sitzungen mit Marke, ein `stale`-Befund von S2 verlor `stale` beim nächsten Material von S1, und der Storefehler-Rückweg (`invalidierung_verdrahtung.rs:118`) rechnete ohne Materialeingang alle Sitzungen** — verschärft dadurch, dass der Merker am BASIS beim Sammeln nie gelöscht wurde. SONDE-014 M-10, M-24, M-29 und Entwurf §37.3 normieren die Dauer von `stale` nicht und kennen keinen Sitzungsbezug; „bis neues Material" steht allein im Codekommentar `invalidierung_verdrahtung.rs:204-211`; `intent.rs:592-600` bindet „kein Neurechnen" auf den Veraltungsschritt; der Test `sonde014_befund.rs:317-358` misst nur diesen Schritt. Der Aufruf `:108-109` ist neu, die Sitzungsreichweite vorbestehend. **Gebrochen wird genau ein Satz: R-283-7 aus §31** — Gate, R-283-1, M-13 bis M-16 (M-16 verlangt den Takt), SONDE-014 und §37.3 brechen nicht. |
| Einordnung | **LÜCKE, kein Defekt, keine Nacharbeit.** Der Befund trifft die vom Dirigenten in §31 formulierte Lückenregel, deren Satz „außerhalb der Naht kein Nachrechnen" eine sitzungsscharfe Norm aufstellte, die weder Entwurf noch SONDE-014-Matrix noch der Bestandscode je trugen: am BASIS heilte jeder Materialeingang und jeder Storefehler-Rückweg irgendeiner Sitzung S2s `stale`. Die Codefassung (Merker je Sitzung, Sitzungsfilter im Sammeln, mengenwertiger Rückgabewert, drei Zähltests plus Zwei-Sitzungs-Fall) verkürzte nur den zusätzlichen Heilungstakt, nicht die Dauer von `stale`, weil der Materialtakt sitzungsübergreifend bliebe — eine halbe Sitzungsschärfe; sie ist Härtung (NAK-287 (e)), ein sitzungsscharfer Rechenpfad insgesamt ein eigener Umbau außerhalb von F02. Neu gegenüber BASIS bleibt genau eines: ein Verwurf nach R-283-1 ohne Materialeingang löst einen zusätzlichen Lauf über alle Sitzungen mit Marke aus; das Ergebnis ist idempotent (ein unveränderter Stand erzeugt keinen Event, `:522-524`). Produktwirkung: keine. |
| R-283-7 (präzisiert, 13.09.2026) | „Innerhalb der Rückschreibenaht (zwischen Cache-Eintrag und Persistenz) gilt R-283-1 vollständig — jeder unterlassene Schreibversuch zieht genau eine Neurechnung nach sich, gleich ob der Standeintrag durch jüngere Evidenz, eine Rücknahme oder eine Intentänderung ersetzt wurde; die Neurechnung steht auf dem jüngsten Eingangsstand (ohne den zurückgenommenen Beleg, unter der aktuellen Generation, nur mit Vollständigkeitsmarke) und trägt nur ein, was vom Standeintrag abweicht. Der Rechenlauf umfasst wie am Bestand alle Sitzungen mit Vollständigkeitsmarke. Außerhalb der Naht bleiben SONDE-014 M-24 und §37.3 unverändert: der Veraltungsschritt selbst rechnet nicht nach; ein zurückgenommener oder überholter Befund bleibt `stale` bis zum nächsten Rechenlauf, den Material irgendeiner Sitzung, ein Storefehler-Rückweg oder ein Verwurf nach R-283-1 auslöst." Diese Fassung ersetzt die aus §31; geändert ist allein der letzte Satz („stale bis neues Material, kein Nachrechnen" → die beiden letzten Sätze). Für die Abschlussprüfung gilt R-283-7 in dieser Fassung neben R-283-1 bis R-283-6. |
| Etappe 3 | **Abgeschlossen auf `30a0b1bf`** (Produktcode `6ce14429`, seit dem Bau unverändert): Erstprüfung 3 NEEDS_WORK (1) → validiert → LÜCKE mit präzisierter Regel → kein Defekt; nach Skill §3.4 („Ein Lauf ohne Defekt ist PASS") ist die Etappe geprüft, ohne Nacharbeitsrunde. Gemessen und geprüft sind M-13 bis M-25, M-75 und M-26 in beiden Hälften (Hälfte 1 §20.2, Hälfte 2 §29.2); M-70 Hälfte 2 bleibt Etappe 6. NAK-253 und NAK-163 werden mit dem Ticketabschluss geschlossen (Abschlussprüfung über `39e18d3f...HEAD`). |
| Rundenbilanz | Erstprüfung: `30a0b1bf..34a127e5: Doku 5 Datei(en) +106/-9` — keine Codeänderung; kumuliert `--runden bb49a612 988dc44d faea3c1a 3d6bbfb4 30a0b1bf`: „kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)". Produktfortschritt des Tickets: Etappen 2 und 3. |
| Hygiene des Fensters (00:10 bis 00:27 Uhr) | MEMORY.md 5 338 B, CLAUDE.md 19 020 B (≤ 20 480), Skill 24 369 B (≤ 24 576); eine MEMORY.md-Indexzeile über 250 Zeichen (311) sofort auf 216 gekürzt (rechnerlokal, Inhalt bleibt in der Memory-Datei); keine Memory-Datei ohne Indexlink; `dokuriegel.py` auf CLAUDE.md, Skill, Register und Manifest ohne Befund; `gesundheit.py` Exit 4 — Funktionen über 200 Zeilen 29/28 (darunter `aufnahmen_sammeln` `hypothese_verdrahtung.rs:195` mit 281 Zeilen, durch die Eingangsmenge dieser Etappe gewachsen) und Kommentar-Bezeichner 34/30 (+1 gegenüber §22: `rowid` im Doc-Kommentar `hypothese_verdrahtung.rs:625` dieser Etappe; die übrigen Fundstellen vorbestehend) — NAK-255 datiert nachgetragen, kein stilles Nachbessern. Kein Riss der Kontextfläche mehr offen. |
| Etappe 4 | Frischer Opus-Worker (max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-4-auftrag.txt` (Ticketpfade nach §3 und §6.5 wörtlich; Matrix M-27 bis M-43 in §5.3; Regeln R-283-3 und R-283-6; Zusagen F04, F05, F09, F12 in §4; Manifest §33; kein Kanon, kein Laufzeit-Arm — beides am Ende der Etappe 6). Aufsicht ENG (Audio-Thread, Hostvertrag, Nulltest, State). Danach Erstprüfung 4 durch Codex Astra max (Vorlage A über den Etappendiff). |


## 33. Bauetappe 4 — Hostvertrag und DSP-Kante der Sonde (Bauer, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **4 von 6** (F04, F05, F09, F12; Matrixzeilen M-27 bis M-43). Basis `0f0bc1edbd0554eede8976f05a3eb3f4c376e19c` (= `git rev-parse HEAD` beim Start); `git status --short` beim Start zeigt nur die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` (§27) — nicht angefasst, nicht committet. **Produktcode und Tests liegen auf `fe6ee58f`** (Commit 1 der Etappe; vor `git pull --rebase` auf `e4554125` war es `7c139163`, die Blobs sind gleich); Beweise, Skripte und dieser Abschnitt auf `f5011dfe` (vorher `aaaf32cf`), die Messung von A33 nach dem Rebase im dritten Commit. Auftrag `docs/beweise/roh/NAK-283-etappe-4-auftrag.txt` (§32). Spezifikation: die geprüfte Matrix §5.3, der Bauplan §6.5, die Regeln R-283-3 und R-283-6 (§2). |
| Nebenläufig | Anweisung des Dirigenten per Sitzungsnachricht (13.09.2026, 00:4x Uhr): bis 01:30 Uhr kein cmake-Bau und kein Testlauf im Checkout, weil eine parallele Session in einer Arbeitskopie auf demselben Rechner einen Kanon fuhr (fremde Bau- und Testlast hatte Beine reißen lassen, §29.7 Punkt 4). Bis dahin nur Lesen, Bauplan je Datei und dieser Kopf. Um 01:25 Uhr hob der Dirigent die Sperre auf (fremder Kanon beendet) und meldete `origin/master` auf `63df1e95` — fremde Commits zu NAK-288 (clang-tidy-Werkzeug, Register, Planstand), kein Ticketpfad dieser Etappe; der Basis-SHA bleibt `0f0bc1ed`. Bau ab 01:26 Uhr. Vor dem Push stand `origin/master` auf `e4554125` (NAK-288 T2 PASS und Planstand, wieder kein Ticketpfad dieser Etappe); `git pull --rebase` lief ohne Konflikt. Mit NAK-288 kam das Kanon-Bein A33 in den Kanon (clang-tidy-Ratsche über `eq-copilot/plugin`); es analysiert die geänderten Produktquellen und lief deshalb auf dem rebasten Stand (§33.4). |
| Umfang | **17 Matrixzeilen** gebaut und gemessen: M-27 bis M-43. Zehn Dateien geändert, +956 / −33 Zeilen (§33.3), dazu dieser Abschnitt, die Kopfzeilen und die Rohdateien. |
| Beweise | **20 von 20 Rotbeweisen** rot → grün mit bytegleicher Rücknahme — 18 Mutationen, dazu der Riegel M-43 gegen den Basis-SHA (Bilanz `docs/beweise/roh/NAK-283-etappe-4-rotbilanz.txt`, §33.2). **38 von 38 Beinen Exit 0**, 42 Ziele gebaut ohne Warnung auf einer geänderten Zeile (`docs/beweise/roh/NAK-283-etappe-4-beine.txt`, §33.4). Nach dem Rebase **A33 (clang-tidy) Exit 0**: 83 Fundstellen bei Grenze 83, dieselbe Menge wie vor der Etappe (`docs/beweise/roh/NAK-283-etappe-4-tidy.txt`, §33.4). Riegel M-43 am Arbeitsbaum 12 von 12. Kein Kanon, kein Laufzeit-Arm (Ende der Etappe 6). |
| Abweichungen | **Dreizehn**, alle in §33.6 mit Grund und Quelle; dazu sieben Entscheide innerhalb der Regeln (§33.5). Keine Matrixzeile geändert — die Prüfung entscheidet. |
| Produktfragen | Keine beantwortet. |

### 33.1 Was gebaut wurde

Zeilenangaben zum Etappenstand (Rotbeweise stellen die Bytes bytegleich wieder her, §33.2).

**F04 — das Buslayout folgt der Regel von Gen (M-27 bis M-29).**
`SondeProcessor::isBusesLayoutSupported` (`SondeProcessor.cpp:291-313`): die
Prüfung deaktivierter Hauptbusse bleibt, danach `ein != aus` → Nein, und
angenommen wird nur Mono oder Stereo — dieselbe Regel wie
`EqCopilotProcessor::isBusesLayoutSupported` (`src/prozessor/Hostbruecke.cpp:149-158`).
Keine Mehrkanalverarbeitung. Der Selbstkommentar nennt den rechnenden Kern mit
zwei Kanälen und führt die alte Passthrough-Begründung ausdrücklich als
überholt (Riegel M-43).

**F05 — der dritte Hosteintritt (R-283-3, M-30 bis M-34).**
`SondeProcessor::reset()` ist neu (`SondeProcessor.h:139-145`,
`SondeProcessor.cpp:267-289`): `getCallbackLock()`, danach `zustandSchloss` —
dieselbe Ordnung wie `releaseResources` (§8.1 Feinheit 4) —, dann
`dspKern->beendeAudiohistorie()`. Kein `vergissLetztePublikation`,
`publikationOffen` bleibt, `getTailLengthSeconds()` bleibt 0,0 (SONDE-015 M-51).
`DspKern::beendeAudiohistorie()` (`DspKern.h:91-103`, `DspKern.cpp:144` ff.)
setzt je Pfad die aktive und die Quellbank kalt wie eine frisch publizierte
Bank in `publiziereVorbau` (Zustände genullt, beide SVF-Enden auf der Ruhe),
beendet jeden laufenden Übergang — Crossfade, Rampe, Hörhalt — auf seinem Ziel
und dient die Quellbank über den regulären ACK aus (wie das natürliche Ende in
`verarbeitePfad`), stellt die fünf Rampen auf ihren Zielwert, meldet die
Auslenkungen mit 0,0 und beendet den Fade der Hörmatrix. Programm,
Publikation, gewünschte Hörmatrix-Auswahl und alle Zähler bleiben. Der
Kommentar von `releaseResources` (`SondeProcessor.cpp:251-265`) nennt das
Lebenszyklus-Trio.

**F09 — die Zeit des verworfenen Taps bleibt sichtbar (M-35 bis M-38).**
`StampedAudioQueue::verwirfOhneAudio (frames)` (`StampedAudioQueue.h:492-521`)
ist die Produzentenfläche aus §6.5: der lokale Strom rückt um `frames` vor, der
eigene Zähler `ohneAudioDrops` steigt (`:641`, Zugang `dropsOhneAudio()` `:576-579`),
`verworfeneFrames` steigt um `frames`, `lueckeOffen` wird gesetzt — die Wirkung
des `verlust`-Pfads von `veroeffentliche`, ohne Ring, ohne Allokation, nur
Produzentenzustand und Atomics. `oversizeDrops` und `dropsGesamt()` bleiben
unverändert. `SondeProcessor::processBlock` ruft sie im dritten Zustand — Tap
ungültig und Kern rechnet — als eigenen Zweig nach dem ruhenden Passthrough
(`SondeProcessor.cpp:423-434`); die Reihenfolge gültiger Tap → ruhender
Passthrough → Verwurf bleibt, die Stückelung des Kerns (M-48) ist unberührt.

**F12 — Endlichkeit am float-Ausgang (R-283-6, M-39 bis M-42).** An allen drei
Verengungsstellen wird zuerst verengt, dann geprüft:
- Ausgangsschleife des Kerns (`DspKern.cpp:1053-1071`): ein float, der nicht
  endlich ist, obwohl sein double endlich war, wird auf 0,0f verriegelt und in
  `zaehlerEingaenge` gezählt. Die Schleife läuft nur, wenn eine engagierte Bank
  rechnet (`committedRuht` kehrt vorher zurück) — innerhalb des
  `engagiert`-Rahmens des Eingangsriegels.
- `HoerMarkierung.h` `tdf2Lauf` (`:739-749`): `const float f = (float) y;`,
  dann `isfinite (f)`; der Wet-Zähler sieht den Fall; die Blockrandprüfung von
  `s1`/`s2` bleibt. Die Selbstzusage (`:711-733`) nennt die Verengung.
- Analysekopie der Sonde (`SondeProcessor.cpp:388-409`): dieselbe Prüfung je
  Wert, lokal gezählt und je Block **eine** Addition über den neuen
  Zählerzugang `DspKern::zaehleVerriegelteVerengungen` (`DspKern.h:246-255`)
  in denselben sichtbaren Zähler; die Doku von `nichtEndlicheEingaenge`
  (`DspKern.h:239-244`) nennt beide neuen Quellen.

**Riegel M-43.** Die vier Selbstzusagen nennen den rechnenden Kern
(`SondeProcessor.cpp:293-302`, `:253-259`, `:414-431`, `HoerMarkierung.h:711-733`);
dazu der Kopf von `SondeProcessor.h` (`:17-25`) und die Doku von
`klassifikation()` (`:227-230`), die sich noch auf „heute Passthrough" beriefen
(§33.6 Abweichung 4). Das Riegelskript
`docs/beweise/roh/NAK-283-etappe-4-riegel-M-43.ps1` misst alle Stellen am
Arbeitsbaum oder an einer Revision; die Runner-Behauptung von A16
(`tools/beweise.ps1:657`) nennt den aktiven Kern bereits am Basis-SHA („mit dem
aktiven DSP-Kern").

**Testzugänge** (nur `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3`, `SondeProcessor.h:302-324`):
`analyseDropsOhneAudioFuerTest`, `analyseVerloreneFramesFuerTest` und
`mitAngehaltenerAnalyseFuerTest (f)` — hält `analyseSchloss`, unter dem allein
der Worker die Queue liest, und reicht `f` die Queue als einzigem Consumenten;
der Audiopfad nimmt dieses Schloss nie (§33.6 Abweichung 3).

**Kein Schemawechsel, kein neues persistentes Feld, kein Hostparameter, keine
Identitätsänderung.** M-31 misst `getStateInformation` bytegleich vor und nach
`reset()`.

### 33.2 Je Matrixzeile: Test und Rotbeweis

Alle Rotbeweise nach §6.1, gefahren von
`docs/beweise/roh/NAK-283-etappe-4-rotskript.ps1`: Originalbytes und SHA-256
jeder Quelle der Mutation sichern, Mutation an der Zusagezeile (jedes Muster
genau einmal, sonst Abbruch), Zeitstempel, Neubau des Ziels mit Frischeprüfung,
Bein rot — **ein Rotlauf zählt nur mit Exit ungleich 0 und einer FEHLER-Zeile,
die den Testnamen der Zusage trägt** —, Originalbytes zurück (nie eine zweite
Textersetzung), Hash gegen vorher, Zeitstempel, Neubau, Bein grün mit ok-Zeile
desselben Testnamens. Keine Zeile dieser Etappe steht in der
Nebenläufigkeitsliste aus §6.1; die Analysefälle halten den Worker
deterministisch an. Bilanz: `docs/beweise/roh/NAK-283-etappe-4-rotbilanz.txt`.

| Zeile | Test (Datei:Zeile, Name) | Bein | Rotbeweis: Mutation | Rohdatei | rot (Exit 1) · grün (Exit 0), Messwerte der Trägerzeile |
|---|---|---|---|---|---|
| M-27 | `SondeNullTestMain.cpp:824` `mehrkanallayout_wird_abgelehnt` | A16 | MUT-27, `SondeProcessor.cpp:309-312`: die Mono/Stereo-Einschränkung entfernt, nur `ein == aus` (Stand am Basis-SHA) | `NAK-283-rot-M-27-etappe-4.txt` | rot: Quad, 5.1 und vier diskrete Kanäle angenommen und gesetzt, Kanäle 4/4 · grün: alle drei Nein, `setBusesLayout` scheitert, Kanäle 2/2 |
| M-28 | **Regressionswache:** `:861` `mono_und_stereo_bleiben_angenommen` (mit Monoblock) | A16 | MUT-28, `SondeProcessor.cpp:312`: nur Stereo zugelassen | `NAK-283-rot-M-28-etappe-4.txt` | rot: Mono Nein, Monolayout nicht gesetzt · grün: Stereo und Mono Ja, Monolayout gesetzt, Monoblock bitgleich, Band rechnet |
| M-29 | **Regressionswache:** `:874` `ungleiche_und_deaktivierte_layouts_bleiben_abgelehnt` | A16 | MUT-29, `SondeProcessor.cpp:309-312`: die Gleichheit von Ein- und Ausgang entfernt (Abweichung 9) | `NAK-283-rot-M-29-etappe-4.txt` | rot: Mono→Stereo und Stereo→Mono angenommen · grün: alle fünf Layouts Nein |
| M-30 | `TransactionTestMain.cpp:2349` `host_reset_beendet_die_audiohistorie` (Gegenprobe ohne reset im selben Fall) | B7 | MUT-30, `SondeProcessor.h:145` und `SondeProcessor.cpp:267-289`: die `reset`-Überschreibung entfernt (leere JUCE-Basis, Stand am Basis-SHA) | `NAK-283-rot-M-30-etappe-4.txt` | rot: Peak nach `reset()` 0,0192088876, gleich dem Lauf ohne `reset()` · grün: 0 gegen 0,0192088876 |
| M-31 | **Regressionswache:** `:2371` `host_reset_laesst_parameter_und_zustand_unberuehrt` | B7 | MUT-31, `SondeProcessor.cpp:288`: `reset()` ruft zusätzlich den neutralen Ladestart (Rotbeweisspalte wörtlich); MUT-31b zusätzlich mit Publikation (Abweichung 7) | `NAK-283-rot-M-31-etappe-4.txt`, `NAK-283-rot-M-31b-etappe-4.txt` | rot MUT-31: Bytes ungleich, Revision 1 → 0, `eq_enabled` 0, Bank im Kern 1, 1 kHz +12 dB; MUT-31b: dazu Bank im Kern 0, 1 kHz 0 dB · grün: 4 704 Bytes gleich, Revision 1 → 1, `eq_enabled` 1, Bank im Kern 1, +12 dB |
| M-32 | `:2403` `die_drei_hosteintritte_enden_in_derselben_audiohistorie` | B7 | MUT-30 (Weg b leer) | `NAK-283-rot-M-32-etappe-4.txt` | rot: Peaks 0 / 0,0192088876 / 0, nicht bitgleich · grün: 0 / 0 / 0, bitgleich |
| M-33 | **Regressionswache:** `SondeNullTestMain.cpp:318` `latenz_bleibt_null` (bestehend) | A16 | MUT-33, `SondeProcessor.h:170`: `getTailLengthSeconds()` liefert 0,05 | `NAK-283-rot-M-33-etappe-4.txt` | rot: Latenz 0, Tail 0,05 s · grün: Latenz 0, Tail 0 |
| M-34 | **Regressionswache:** `:249` `nulltest_bleibt_bitidentisch_mit_host_reset` (vier Raten, Hard-Bypass, NaN/Inf; `fahreNull` mit `resetAlle`, `:153`) | A16 | MUT-34, `SondeProcessor.cpp`: `reset()` setzt eine Marke, der nächste `processBlock` nullt seinen Puffer (Abweichung 8) | `NAK-283-rot-M-34-etappe-4.txt` | rot: bei allen vier Raten und im Hard-Bypass 152 379 von 460 000 Samples abweichend · grün: 0 abweichend, Zähler 0/0/0 |
| M-35 | `TransactionTestMain.cpp:2496` `ungueltiger_tap_bei_rechnendem_kern_hinterlaesst_eine_luecke` | B7 | MUT-35, `SondeProcessor.cpp:433`: der Verwerfpfad entfernt (Stand am Basis-SHA: der Block fällt durch) | `NAK-283-rot-M-35-etappe-4.txt` | rot: zwei Deskriptoren, der dritte Block bei `stromVon 64` ohne Lücke, Segment 1 · grün: der dritte Block bei `stromVon 192` mit `kFlagLueckeDavor`, Segment 2 |
| M-36 | `:2503` `dsp_und_queuezaehler_beschreiben_dieselbe_verworfene_zeitspanne` | B7 | MUT-36, `StampedAudioQueue.h:518-519`: der Verwerfpfad ohne Zähler | `NAK-283-rot-M-36-etappe-4.txt` | rot: Kern +1, ohne Audio +0, Frames +0 · grün: Kern +1, ohne Audio +1, Frames +128, oversize +0, Überlauf +0 |
| M-37 | **Regressionswache:** `:2539` `ruhender_passthrough_erzeugt_keine_luecke` | B7 | MUT-37, `SondeProcessor.cpp:412`: ein Verwerfpfad vor dem Passthrough-Zweig | `NAK-283-rot-M-37-etappe-4.txt` | rot: zwei Deskriptoren — der übergroße Block fehlt, der dritte trägt eine Lücke, Segment 2, ohne Audio +1 · grün: drei Deskriptoren, Block 2 `stromVon 64` mit 128 Frames ohne Lücke, Segment 1, ohne Audio +0 |
| M-38 | **Regressionswache:** `DspGoldenTestMain.cpp:3080` `der_uebergrosse_block_gleicht_sample_exakt_dem_lauf_ohne_ueberlast` (bestehend, M-48/B-22) | B6 | MUT-38, `DspKern.cpp:850-861`: der übergroße Block wird zurückgewiesen statt gestückelt | `NAK-283-rot-M-38-etappe-4.txt` | rot: FEHLER am Träger (SONDE-015 M-48), dazu SONDE-015 M-25 · grün: ok |
| M-39 | `:3346` `endlicher_eingang_erzeugt_keinen_nichtendlichen_floatausgang` | B6 | MUT-39, `DspKern.cpp:1062-1070`: der Ausgangsriegel entfernt (Stand am Basis-SHA) | `NAK-283-rot-M-39-etappe-4.txt` | rot: 512 von 1 024 Werten am Ausgang nicht endlich, Zähler +0 · grün: 0 nicht endlich, 0 abweichend, Zähler +512 |
| M-40 | `MarkierungTestMain.cpp:800` `NAK-283 M-40: endlicher_eingang_erzeugt_keinen_nichtendlichen_wetausgang` (`nak283WetVerengung` `:747`, gerufen `:1574`) | A3 | MUT-40, `HoerMarkierung.h:742-744`: die Prüfung wieder auf `y` (double) | `NAK-283-rot-M-40-etappe-4.txt` | rot: 4 696 Samples nicht endlich, Wet-Zähler +0 · grün: 0 nicht endlich, Wet-Zähler +4 228 bei 4 228 verriegelten Samples |
| M-41 | `TransactionTestMain.cpp:2583` `analysekopie_verengt_nicht_unbemerkt` | B7 | MUT-41, `SondeProcessor.cpp:401`: die Prüfung aus der Kopierschleife entfernt | `NAK-283-rot-M-41-etappe-4.txt` | rot: Analyse nicht endlich, Zähler +0 · grün: Analyse endlich, Zähler +1 024 |
| M-42 (a) | **Regressionswache:** `DspGoldenTestMain.cpp:3392` `ausgangsriegel_beruehrt_endliches_material_nicht` | B6 | MUT-42a, `DspKern.cpp:1063`: der Riegel greift jeden linken float mit Betrag ≥ 1 (Abweichung 10) | `NAK-283-rot-M-42a-etappe-4.txt` | rot: 52 608 von 204 800 Werten nicht bitgleich, Zähler +52 608; dazu fallen SONDE-015 M-02, M-05, M-06, M-17, M-35, M-54 und NAK-283 M-39 (endliches Material mit Betrag ≥ 1 wird genullt) · grün: 0 nicht bitgleich, 105 396 Werte mit Betrag ≥ 1, Zähler +0 |
| M-42 (b) | **Regressionswache:** `SondeNullTestMain.cpp:352` `passthrough_sanitisiert_nichts` (bestehend) | A16 | MUT-42b, `DspKern.cpp` Ruhezweig von `verarbeiteStueck`: ein Ausgangsriegel außerhalb des `engagiert`-Rahmens (Rotbeweisspalte wörtlich) | `NAK-283-rot-M-42b-etappe-4.txt` | rot: Zähler 6; dazu fällt Abschnitt 3b im Hard-Bypass (NaN/Inf nicht roh) · grün: Zähler 0 |
| M-43 | Riegel `NAK-283-etappe-4-riegel-M-43.ps1`, Prüfzeilen R1 bis R6 | Riegel | MUT-43, `SondeProcessor.cpp:293-302`: die alte Passthrough-Begründung zurück; dazu derselbe Riegel gegen den Basis-SHA (rot) und den Arbeitsbaum (grün) | `NAK-283-rot-M-43-etappe-4.txt`, `NAK-283-rot-M-43-basis-etappe-4.txt` | rot MUT-43: 2 von 12 Prüfungen FEHLER (R1: 0 Treffer, alte Fassung steht); Basis-SHA: 11 von 12 FEHLER (R6 steht schon am Basis-SHA) · grün: 12 von 12 ok, 1 Treffer mit Marker |

**Regressionswachen sind kein Beleg für ihren Befund** (`tools/dirigent/pruefliste.md` E):
M-28, M-29, M-31, M-33, M-34, M-37, M-38, M-42. Die Belege liegen bei M-27
(F04), M-30 und M-32 (F05), M-35 und M-36 (F09), M-39 bis M-41 (F12) und beim
Riegel M-43 (Übergang).

**Die fünf Hashes der mutierten Produktdateien** (SHA-256 der Bytes im
Arbeitsbaum, gemessen vor dem Rotlauf; jeder Rotbeweis stellt sie bytegleich
wieder her, die Rohdateien tragen je Mutation vorher und nachher):

| Datei | SHA-256 |
|---|---|
| `eq-copilot/plugin/sonde/SondeProcessor.cpp` | `0D517BC8AA8270F3AF29D656E1F4F8D195B4F5EF837A6FAA4B55400AA3070CCC` |
| `eq-copilot/plugin/sonde/SondeProcessor.h` | `6B6A4CADA6E2783CC92EF2B337D3A26487176A8E10F3DA12C18BBBAD35EED898` |
| `eq-copilot/plugin/dsp/DspKern.cpp` | `DB7F930002D154BDB9890434AC884AA517481CFC39E3409201973D6FB8DDA20E` |
| `eq-copilot/plugin/core/StampedAudioQueue.h` | `CF3A29F3FDB8C17567ABC58E22DA10A84B91CF687A5EAAABA0E7FBED038140A9` |
| `eq-copilot/plugin/src/HoerMarkierung.h` | `D8E034F0459697403611BC04872523C259B93751120578F9B939FB89404894DC` |

**Gemessene Werte der grünen Läufe** (aus den Beinläufen, §33.4): M-30 Peak nach
`reset()` exakt 0, ohne `reset()` 0,0192088876 — derselbe Wert, den der Audit am
VST3-Modul maß (Übergabe §4 F05, 0,019208887592); M-31 4 704 Bytes bytegleich,
Revision 1 → 1, 1 kHz +12 dB; M-32 drei Peaks 0, bitgleich; M-35 zwei
Deskriptoren, Block 1 `stromVon 0`, Segment 1, Block 3 `stromVon 192`,
`kFlagLueckeDavor`, Segment 2; M-36 Kern +1, Queue +1 Block ohne Audio,
+128 Frames, oversize +0; M-37 drei Deskriptoren, Block 2 `stromVon 64` mit
128 Frames, keine Lücke, ein Segment; M-39 512 von 1 024 Werten über dem
float-Rand, alle verriegelt, keiner abweichend, Zähler +512; M-40 Wet-Zähler
+4 228 bei 4 228 verriegelten Samples; M-41 Zähler +1 024, Analyse endlich;
M-42 (a) 204 800 Werte verglichen, keiner abweichend, 105 396 davon mit
Betrag ≥ 1 (beide Kanäle), Zähler +0. Die Prüfzeilen von M-35 und M-37 nennen
alle gelesenen Deskriptoren, M-39 und M-42 (a) zählen ohne Abbruch der
Messschleife — auch die Rotwerte der Tabelle sind vollständig.

### 33.3 Geänderte Dateien

| Datei | Zeilen (+/−) | Was |
|---|---:|---|
| `eq-copilot/plugin/sonde/SondeProcessor.cpp` | +77 / −13 | `isBusesLayoutSupported`, `reset()`, Kommentar `releaseResources`, Analysekopie mit Riegel, dritter Zweig |
| `eq-copilot/plugin/sonde/SondeProcessor.h` | +37 / −5 | `reset()`-Deklaration, drei Testzugänge, Kopf und `klassifikation()`-Doku (M-43) |
| `eq-copilot/plugin/dsp/DspKern.cpp` | +64 / −2 | `beendeAudiohistorie`, Ausgangsriegel |
| `eq-copilot/plugin/dsp/DspKern.h` | +26 / −1 | Deklaration `beendeAudiohistorie`, Zählerzugang `zaehleVerriegelteVerengungen`, Zählerdoku |
| `eq-copilot/plugin/core/StampedAudioQueue.h` | +37 / −1 | `verwirfOhneAudio`, `dropsOhneAudio`, Zähler, Klassenkopf |
| `eq-copilot/plugin/src/HoerMarkierung.h` | +16 / −8 | `tdf2Lauf` verengt, dann prüft; Riegeldoku |
| `eq-copilot/plugin/tests/SondeNullTestMain.cpp` | +154 / −2 | Abschnitt 3b (M-34), Abschnitt 12 (M-27 bis M-29), `fahreNull (resetAlle)`, Kopf |
| `eq-copilot/plugin/tests/TransactionTestMain.cpp` | +375 / −1 | Abschnitte P (M-30 bis M-32) und Q (M-35 bis M-37, M-41), Helfer, Kopf |
| `eq-copilot/plugin/tests/DspGoldenTestMain.cpp` | +97 / −0 | M-39 und M-42 (a) am Ende von Abschnitt K |
| `eq-copilot/plugin/tests/MarkierungTestMain.cpp` | +73 / −0 | `nak283WetVerengung` (M-40) und sein Aufruf |

Zeilenzahlen aus `git diff --numstat` gegen den Basis-SHA. Nicht angefasst:
`broker/**`, `eq-copilot/plugin/state/**`, `eq-copilot/plugin/core/analysis/**`,
`eq-copilot/schemas/**`, `eq-copilot/identity/**`, `eq-copilot/install/**`,
`tools/**`, `eq-copilot/plugin/src/prozessor/**`, `SourcesModel`, alle anderen
Tests und Manifeste, `docs/PLAN-STAND.md`, `docs/offene-punkte.md`, `design/`;
`eq-copilot/plugin/CMakeLists.txt` blieb unberührt. Keine Fixture geschrieben.

### 33.4 Gefahrene Beine

Gefahren von `docs/beweise/roh/NAK-283-etappe-4-beinskript.ps1` aus pwsh; Rohausgabe
`docs/beweise/roh/NAK-283-etappe-4-beine.txt` (Lauf 3, 13.09.2026, 02:33 Uhr; HEAD = Basis
`0f0bc1ed`, der Etappenstand im Arbeitsbaum mit den Hashes aus §33.2).

**Bau.** Vor dem Bau stempelt das Skript die zehn geänderten Quellen neu (SHA-256 vorher =
nachher), damit jede Übersetzungseinheit, die eine davon liest, in diesem Bau übersetzt wird.
`cmake --build eq-copilot/build --config Release` über 42 Ziele — jedes nicht stillgelegte
Plugin-Ziel des Kanons, die Artefakte der Python-Beine, `EqCopilot_VST3` und
`NakamaProbeeq_VST3` —: **Exit 0**. `warning C`: 49 eindeutige Zeilen an 14 Stellen, **keine auf
einer Zeile, die diese Etappe hinzugefügt oder geändert hat** (gemessen gegen `git diff -U0` zum
Basis-SHA). Alle 14 stehen auf unverändertem Code: `core/analysis/featureengine/Stereo.h:70`
C4458 (28 Ziele), `tests/DspGoldenTestMain.cpp` Zeilen 953, 969, 3138 und 3140 C4456,
`tests/MarkierungTestMain.cpp:1074` C4457, `tests/IpcTestMain.cpp:3705` und `:3987` C4456,
`tests/Sonde012HostChannelContextTest.cpp:295` und `:450` C4244,
`tests/StateMigrationTestMain.cpp:1348` und `:1636` C4456. Die beiden Bundles tragen damit keine
neue Warnung.

**Auswahl** (Abweichung 6). Die Tabelle der Rohausgabe nennt je Kanon-Bein aus
`tools/beweise.ps1` die geänderten Quellen, die sein Ziel liest oder linkt. Gefahren: die neun
Beine aus §6.5 ohne das stillgelegte A15, 24 weitere Plugin-Beine laut Trackinglog und fünf
Python-Beine mit Artefakt aus `eq-copilot/build/plugin`. Nicht gefahren: B3, B3b und B9 (lesen und
linken keine geänderte Quelle), die übrigen 23 Python- und Cargo-Beine (berühren kein Artefakt aus
`eq-copilot/build/plugin`), A15 (stillgelegt).

**Frische** (Abweichung 13). Jedes der 38 gefahrenen Artefakte ist jünger als die jüngste
geänderte Quelle, die sein Ziel liest oder linkt; `EqCopGoldenTest` (A2) liest und linkt keine
(Gegenprobe). Kein Artefakt veraltet.

| Bein | Ziel | Grund | Exit | s |
|---|---|---|---:|---:|
| A16 | `EqCopProbeeqNullTest` | §6.5 | 0 | 0,3 |
| A1 | `EqCopNullTest` | §6.5 | 0 | 0,1 |
| B7 | `EqCopTransactionTest` | §6.5 | 0 | 0,5 |
| B6 | `EqCopDspGoldenTest` | §6.5 | 0 | 4,8 |
| A3 | `EqCopMarkierungTest` | §6.5 | 0 | 7,5 |
| B4 | `EqCopQueueStressTest` | §6.5 | 0 | 3,3 |
| B5 | `EqCopAnalysisGoldenTest` | §6.5 | 0 | 5,2 |
| B21 | `EqCopSonde013InterventionRingTest` | §6.5 | 0 | 0,1 |
| A2 | `EqCopGoldenTest eq-copilot/fixtures` | §6.5, Gegenprobe | 0 | 9,6 |
| A4b | `EqCopPipeClientTest` | Trackinglog | 0 | 65,7 |
| B1 | `EqCopIdentityTest` | Trackinglog | 0 | 0,1 |
| B2 | `EqCopStateMigrationTest` | Trackinglog | 0 | 1,2 |
| B3c | `EqCopSchemaTest` | Trackinglog | 0 | 0,4 |
| B8 | `EqCopLebenslaufTest` | Trackinglog | 0 | 0,3 |
| B10 | `EqCopIpcTest` | Trackinglog | 0 | 220,5 |
| B11 | `EqCopSonde012HostChannelContextTest` | Trackinglog | 0 | 1,0 |
| B12 | `EqCopSonde012LoudnessSourceTest` | Trackinglog | 0 | 1,6 |
| B13 | `EqCopSonde012SourcesModelTest` | Trackinglog | 0 | 0,1 |
| B14 | `EqCopSonde012ProjectReloadTest` | Trackinglog | 0 | 8,3 |
| B15 | `EqCopShot --sonde012-suite eq-copilot/build/sonde012-shots` | Trackinglog | 0 | 2,2 |
| B16 | `EqCopSonde013EventWireTest` | Trackinglog | 0 | 0,5 |
| B17 | `EqCopSonde013TruePeakGoldenTest` | Trackinglog | 0 | 0,1 |
| B18 | `EqCopSonde013DynamicsTest` | Trackinglog | 0 | 3,9 |
| B19 | `EqCopSonde013StereoGoldenTest` | Trackinglog | 0 | 1,8 |
| B20 | `EqCopSonde013QualityClassTest` | Trackinglog | 0 | 0,1 |
| B22 | `EqCopSonde013FingerprintGoldenTest` | Trackinglog | 0 | 0,9 |
| B23 | `EqCopSonde013PassageStateTest` | Trackinglog | 0 | 11,6 |
| B24 | `EqCopSonde013PrePostGoldenTest` | Trackinglog | 0 | 1,0 |
| B25 | `EqCopSonde013ExperimentGoldenTest` | Trackinglog | 0 | 0,1 |
| B26 | `EqCopSonde013ExperimentUiTest` | Trackinglog | 0 | 0,5 |
| B27 | `EqCopSonde014IntentTest` | Trackinglog | 0 | 0,4 |
| B28 | `EqCopSonde014BefundTest` | Trackinglog | 0 | 0,1 |
| B29 | `EqCopSonde014AssistentTest` | Trackinglog | 0 | 0,8 |
| A14 | `py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py` | Artefakt (`NakamaKern.lib`, Gen-VST3) | 0 | 8,4 |
| A17 | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | Artefakt (`EqCopIpcTest`) | 0 | 1,0 |
| A22 | `py -3.13 tools/eq-copilot/pruefe_ipc_last.py` | Artefakt (`EqCopIpcLast`) | 0 | 10,1 |
| A23 | `py -3.13 tools/eq-copilot/pruefe_sonde012_sources_latency.py` | Artefakt (`EqCopSonde012SourcesLatencyTest`) | 0 | 4,8 |
| A24 | `py -3.13 tools/eq-copilot/pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | Artefakt (`EqCopSessionSoak`) | 0 | 148,7 |

**38 von 38 Beinen Exit 0.** Der Nulltest bleibt bitidentisch grün (A16 mit Abschnitt 3b, A1).

**Frühere Läufe.** Lauf 1 (01:50 Uhr) brach nach grünem Bau an der Frischeprüfung gegen die
jüngste Quelle überhaupt ab (Abweichung 13; Rohdatei `NAK-283-etappe-4-beine-lauf-1.txt`). Lauf 2
(02:17 Uhr, Skript mit Trackinglog-Auswahl, noch ohne Neustempeln) endete mit 38 von 38 Exit 0,
übersetzte im Bau aber nur die Einheiten, die seit den Neubauten des Rotskripts neu zu übersetzen
waren — `DspKern.cpp`, `DspGoldenTestMain.cpp` und `MarkierungTestMain.cpp` nicht, und deren
Warnungen (die vorbestehenden C4456 und C4457 oben) fehlten in seiner Ausgabe. Lauf 3 ersetzt
seine Rohausgabe.

**A33 nach dem Rebase.** Mit NAK-288 kam das Kanon-Bein A33 in den Kanon (`tools/plan/tidy.py`,
clang-tidy über die Übersetzungseinheiten von `eq-copilot/plugin` ohne `tests/`, Ratsche 83). Es
analysiert die geänderten Produktquellen und lief deshalb auf dem rebasten Stand `f5011dfe`
(Rohausgabe `docs/beweise/roh/NAK-283-etappe-4-tidy.txt`): `py -3.13 tools/plan/tidy.py
--mit-selbsttest --jobs 3` — Selbsttest 39/39, 41 Einheiten fehlerfrei geparst, **83 Fundstellen
bei Grenze 83, Exit 0** in 198,6 s. Die Fundmenge ist dieselbe wie der Iststand von NAK-288
(`docs/beweise/roh/NAK-288-tidy-iststand-83.txt`; beim Vergleich zählen in den geänderten Dateien
Datei und Check, nicht die verschobene Zeile). Die fünf Fundstellen in `DspKern.cpp` (Zeilen 63,
503, 512, 520 und 573) stehen auf Code, den diese Etappe nicht berührt. Parallelität 3 statt der
Vorgabe 12: der erste Versuch mit zwölf Prozessen wurde wegen Speichermangels beendet (danach
12,4 GB von 31,7 GB frei); die Fundzahl hängt nicht von der Parallelität ab.

### 33.5 Entscheide innerhalb der Regeln und Selbstaudit

**Sieben Entscheide**, je innerhalb von R-283-3, R-283-6 und §6.5 — keine
Produktfrage:

1. **`reset()` behält die gewünschte Hörmatrix-Auswahl.** Sie ist transient und
   kein Verlauf (R10); zurückgesetzt wird nur ihr laufender Fade. `prepareToPlay`
   stellt sie weiter auf Processed (`DspKern::bereiteVor`) — das ist Neuanlage,
   kein Reset.
2. **Übergänge enden auf ihrem Ziel, Rampen stehen auf ihrem Zielwert.** Das
   Ziel ist das publizierte Programm; der Weg dorthin ist Audiohistorie. Die
   Quellbank dient über den regulären ACK aus — kein zweiter Freigabeweg
   (`DspBankPool.h:37-39`).
3. **Der Ausgangsriegel verriegelt, was die Verengung nicht endlich macht.**
   Gezählt und verriegelt wird ein float, dessen double endlich war. Ein schon
   als double nicht-endlicher Wert kann in der Ausgangsschleife nur roh aus dem
   Dry-Anteil kommen (Dry, Delta, Passthrough-Seite eines Crossfades); er bleibt
   roh (M-42 „der Dry-Zweig bleibt roh"), und der Eingangsriegel hat ihn bereits
   gezählt, weil die Schleife nur bei engagierter Bank läuft. So zählt jeder
   nicht-endliche Wert genau einmal an seinem Ursprung.
4. **Die Analysekopie zählt je Riegelstelle.** Überschreitet derselbe Wert am
   Ausgang (Hörmatrix Processed) und in der Kopie den float-Bereich, zählen
   beide Stellen. M-41 isoliert die Kopie über die Hörmatrix Dry und misst die
   Zahl exakt (+1 024).
5. **Der Verwerfpfad der Queue übernimmt keinen wartenden Neuanlauf.** Der
   nächste angenommene Block übernimmt ihn; weil `lueckeOffen` schon steht,
   trägt er dieselbe Lücke und `startFolge` wie nach einer Übernahme im
   Verwurfszug. `veroeffentliche` bleibt dadurch bytegleich.
6. **`dropsGesamt()` bleibt Überlauf plus Oversize.** B4 misst ihn; die neue
   Ursache steht getrennt in `dropsOhneAudio()`, ihre Frames in
   `verloreneFrames()`.
7. **Die Analysefälle messen den Deskriptor selbst.** Statt am Consumer auf den
   Worker zu warten, hält der Test den Worker über `analyseSchloss` an und liest
   die Queue als einziger Consument — der Aufbau ist deterministisch, der
   Beleg liegt am exakten Frame (§5 „Zeitbeweise am exakten Frame").

**Selbstaudit nach `tools/dirigent/pruefliste.md`:**

| Abschnitt | Wo gemessen |
|---|---|
| **Audio-Thread-Regel** | Der Verwerfpfad (`verwirfOhneAudio`) schreibt nur Produzentenfelder und zwei Atomics; der Ausgangsriegel ist ein Vergleich und bei Treffer eine atomare Addition; die Analysekopie rechnet in einem Lambda ohne Allokation und addiert einmal je Block. Keine Sperre, keine Allokation, kein Datei-, Pipe- oder Netzzugriff, kein Logging. `reset()` läuft nicht in `processBlock`; zu seinem Thread §33.7 Punkt 3. |
| **Sperrenordnung in `reset()`** | `getCallbackLock()` → `zustandSchloss`, wie `releaseResources` und `prepareToPlay`; auch der VST3-Wrapper nimmt das Callback-Schloss vor `setNonRealtime` (das `zustandSchloss` nimmt) — dieselbe Ordnung, kein Gegenlauf. `dspKontrollTakt`, `fuehreTransaktionAus` und `setStateInformation` nehmen nur `zustandSchloss`. |
| **B** — Lebenszyklus | M-32 misst das Trio in einem Zeilensatz; M-30 den VST3-Weg `setProcessing (false)` → `reset()` deterministisch. |
| **C** — Zahlenränder | `FLT_MAX`: 0,75 und 0,25 × `FLT_MAX` bei +6 dB (M-39, M-41), 0,8 × `FLT_MAX` im Puls (M-40); endliches Material in ±1,0 mit Beträgen ≥ 1 (M-42 a). `frames <= 0` ist in `verwirfOhneAudio` kein Verlust. Die Zähler sind `uint64` und steigen um Blöcke beziehungsweise Frames. |
| **NaN am float-Rand** | Ein nicht-endlicher Eingang bleibt im Passthrough roh (M-42 b, M-34 mit NaN/Inf); am aktiven Ausgang verriegelt der Riegel nur, was die Verengung erzeugt (Entscheid 3). |
| **D** — Bau- und Prüfriegel | Das Beinskript misst jedes gefahrene Artefakt gegen die jüngste geänderte Quelle, die sein Ziel laut MSBuild-Trackinglog liest oder linkt (Abweichung 13), und gleicht die Beinauswahl bei jedem Lauf mit den Kanonzeilen aus `tools/beweise.ps1` ab (Abweichung 6). Das Rotskript prüft je Mutation den Neubau des Ziels, bricht bei fehlendem oder mehrfachem Muster ab und zählt Rot nur mit Trägerzeile. |
| **E** — jede neue Prüfung einmal gebrochen | 17 Matrixzeilen, 20 Rohdateien aus 18 Mutationen und einem Basislauf; Regressionswachen als solche benannt. |
| **E** — Behauptung ≤ Messung | M-31 berichtet, welche Messgrößen die wörtliche Mutation kippt (Abweichung 7). Die Runner-Behauptungen in `tools/beweise.ps1` nennen die neuen Zusagen noch nicht (§33.7 Punkt 1). |
| **F** — speichern↔laden | Unberührt: kein neues Feld; M-31 misst die Bytes vor und nach `reset()`; A16 misst Speichern–Laden–Speichern weiter bytegleich. |
| **MSVC-Stack 1 MiB** | Jeder Prozessor und die Hörmarkierung der neuen Fälle liegen auf dem Heap (`std::make_unique`), NAK-175. |

### 33.6 Abweichungen vom Bauplan §6.5

Dreizehn; keine ändert eine Matrixzeile. Die Prüfung entscheidet.

1. **Die Hostwert-Mailbox wird in `reset()` nicht zurückgesetzt.** R-283-3,
   §6.5 und M-30 nennen sie. Die Mailbox (`hostWert`, `hostEreignis`,
   `hostEreignisOffen`; SONDE-015 §12.2 Ursache A) trägt Hostwerte, keine
   Audiohistorie: der Worker leert sie alle 5 ms (`dspKontrollTakt`), unabhängig
   vom Audiocallback. Ungelesene Werte zu verwerfen ließe den wirksamen Zustand
   vom Hostparameter abweichen — gegen „ohne … Parameter … zu ändern" (R-283-3)
   und die Parameterhoheit (`CLAUDE.md`). Sie in den Overlay zu übernehmen,
   verlangte `publikationOffen = true` — gegen „`publikationOffen` unberührt"
   (§6.5). Ihre Bereinigung bei Abweisung und Ladestart ist Regel R-4a-A der
   vertagten Nacharbeit 1 (SONDE-015 §12.2), die der Auftrag ausdrücklich nicht
   vorziehen lässt. Keine Matrixzeile misst die Mailbox.
2. **`DspKern.h`/`.cpp` bekommen die Methode `beendeAudiohistorie`.** Der
   Auftrag nennt für `DspKern.h` den Zählerzugang und für `DspKern.cpp`
   Ausgangsschleife und Riegelrahmen. Pfadzustände, Rampen und der Fade der
   Hörmatrix sind privat; ohne eine Kernmethode könnte `reset()` sie nicht
   zurücksetzen, und `freigeben` plus `bereiteVor` verwürfe Programm und
   Publikation und allozierte.
3. **`SondeProcessor.h` bekommt drei Testzugänge** (nur unter
   `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3`). Ohne sie misst B7 weder den neuen
   Queue-Zähler (M-36) noch den Deskriptor am echten Prozessor (M-35, M-37,
   M-41); `analyseQueue` ist privat.
4. **Über die vier Stellen aus M-43 hinaus zwei weitere Kommentare in
   `SondeProcessor.h`.** Der Kopf sagte „es tut deshalb dasselbe wie Suna:
   nichts am Audio", die Doku von `klassifikation()` „beide Bundles sind heute
   Passthrough". Die Zusage von M-43 — keine Fläche begründet sich mehr mit
   einem Passthrough, den es nicht mehr gibt — deckt beide; der Riegel misst sie
   als R5.
5. **Der Ausgangsriegel verriegelt nur verengungsbedingt nicht-endliche Werte**
   (Entscheid 3). R-283-6 sagt „nicht-endliche float-Werte werden verriegelt";
   M-42 bindet den Riegel an den `engagiert`-Rahmen mit rohem Dry-Zweig. Ein
   Riegel auf jeden nicht-endlichen float verriegelte auch den rohen Dry-Anteil
   und zählte dessen Eingangs-NaN doppelt.
6. **Beine über die Liste aus §6.5 hinaus, gewählt nach den Trackinglogs von
   MSBuild.** Der Auftrag verlangt „jedes weitere Bein, das eine geänderte Quelle
   übersetzt oder linkt". Das Beinskript liest je Ziel `CL.read.*.tlog` (jede
   Datei, die `cl.exe` geöffnet hat) und `link.read.*.tlog` (jede Bibliothek aus
   dem Baum, samt deren eigenem `CL.read`) und gleicht die Auswahl bei jedem Lauf
   mit den Kanonzeilen aus `tools/beweise.ps1` ab (PowerShell-AST; Abbruch, wenn
   ein betroffenes Bein fehlt oder Ziel oder Argumente abweichen). Gefahren
   werden danach zusätzlich A4b, B1, B2, B3c, B8, B10 bis B20, B22 bis B29 und
   die Python-Beine A14, A17, A22, A23 und A24, deren Skript ein Artefakt aus
   `eq-copilot/build/plugin` fährt oder liest. Jedes Plugin-Ziel außer A2, B3,
   B3b und B9 linkt `NakamaKern.lib` und damit `DspKern.cpp` und
   `StampedAudioQueue.h`; B3, B3b und B9 lesen und linken keine geänderte Quelle
   und laufen nicht, A2 ebenso, läuft aber als Gegenprobe aus §6.5. Die erste
   Fassung des Beinskripts (Rohdatei `NAK-283-etappe-4-beine-lauf-1.txt`)
   wählte nach Include-Ketten und übersah die reinen Verbraucher von
   `NakamaKern.lib` (A4b, B13, B17, B20, B25, B26, B28) sowie die Python-Beine
   A17, A22 und A24. **A15 ist stillgelegt** (`tools/beweise.ps1:652`, Ziel
   `EqCopSunaNullTest` existiert nicht) — weder gebaut noch gefahren, nicht
   gezählt.
7. **M-31: die wörtliche Mutation kippt nicht alle drei Messgrößen.**
   `Transaktionskern::ladestart` publiziert nichts an den Kern
   (`NakamaTransaktion.cpp:234-270`); unter MUT-31 fallen Bytes und bestätigtes
   `eq_enabled`, der Kern filtert weiter. MUT-31b ergänzt die Publikation, wie
   der neutrale Weg von `setStateInformation` (`SondeProcessor.cpp`, Ladestart
   und `publiziereWirksam`) — dann kippen alle drei. Beide Rohdateien tragen die
   Messwerte der Prüfzeile.
8. **M-34: „eine reset-Fassung, die den Ausgangspuffer nullt"** ist als Marke
   gebaut, die `reset()` setzt und der nächste `processBlock` einlöst — `reset()`
   hat keinen Puffer.
9. **M-29: die Rotbeweisspalte nennt keine Mutation.** Gebrochen ist die Zeile,
   die ihre Zusage trägt: die Gleichheit von Ein- und Ausgang.
10. **M-42 hat zwei Hälften mit eigener Mutation.** Die Rotbeweisspalte nennt den
    Riegel außerhalb des `engagiert`-Rahmens (Hälfte b, A16); der neue B6-Fall der
    Hälfte a ist mit einem Riegel gebrochen, der endliches Material greift.
11. **M-43: der Riegel liest zusätzlich den gefalteten Wortlaut.** Am Basis-SHA
    steht „waere / kein Passthrough" über einen Zeilenumbruch; `git grep` allein
    fände dort keine Stelle. Dazu ein Lauf gegen den Basis-SHA.
12. **Dateinamen:** die Rohdateien folgen dem Auftrag
    (`NAK-283-rot-M-nn-etappe-4.txt`, `-etappe-4-beine.txt`, `-beinskript.ps1`,
    `-rotskript.ps1`); zusätzlich `-rotbilanz.txt`, `-riegel-M-43.ps1`,
    `-beine-lauf-1.txt` (der abgebrochene erste Beinlauf, Abweichung 13) und die
    Halbsuffixe `M-31b`, `M-42a`, `M-42b`, `M-43-basis`. Der Nulltest mit
    `reset()` (M-34) steht als eigener Abschnitt 3b neben dem unveränderten
    M-01-Lauf, damit dessen Prüfzeilen bleiben, wie sie sind.
13. **Frische je Artefakt statt gegen die jüngste geänderte Quelle überhaupt.**
    Der Auftrag sagt „vor jedem Lauf den Binary-Zeitstempel gegen die jüngste
    geänderte Quelle prüfen". Jeder Rotbeweis stempelt die Quelle nach dem
    Rückspielen neu (NAK-230); danach ist die zuletzt zurückgespielte Quelle die
    jüngste überhaupt, und Ziele, die sie weder lesen noch linken, baut MSBuild
    zu Recht nicht neu. Der erste Beinlauf (13.09.2026, 01:50 Uhr,
    `NAK-283-etappe-4-beine-lauf-1.txt`) brach deshalb nach grünem Bau mit
    „B6 A3 VERALTET" ab: beide Binaries lagen vor der jüngsten Quelle (01:49:29
    Uhr, `SondeProcessor.cpp` aus MUT-43), die keines der beiden Ziele liest
    oder linkt. Das Beinskript misst jetzt jedes gefahrene Artefakt gegen die
    jüngste geänderte Quelle, die sein Ziel laut Trackinglog liest oder linkt,
    und nennt sie je Artefakt in der Rohausgabe; den Artefaktpfad liest es aus
    `link.write` beziehungsweise `Lib-link.write` und gleicht ihn für die
    Plugin-Beine mit dem gefahrenen Pfad ab. Das ist strenger als der
    kanonische Runner, der nach einem erfolgreichen Bau dem Urteil des
    Buildsystems folgt (`tools/beweise.ps1:982-987`).

### 33.7 Offene Punkte dieser Etappe

1. **Runner-Behauptungen.** `tools/beweise.ps1` nennt für A16 weder die
   Layoutablehnung noch den Nulltest mit `reset()`, für B7 weder Host-Reset noch
   Analysezuführung, für B6 und A3 nicht die float-Kante (Prüfliste E,
   „Geänderte Zusage — drei Stellen"). `tools/**` liegt außerhalb der
   Ticketgrenze; Skriptköpfe der vier Tests sind nachgezogen.
2. **Statusbericht.** `v3Status().framesDropped` summiert weiter Überlauf und
   Oversize; die neue Ursache `dropsOhneAudio` reist nicht über den Draht
   (Leitungssemantik unverändert). Kandidat für eine Härtung.
3. **Thread von `reset()`.** Die VST3-API erlaubt `setProcessing` auf dem
   UI- oder dem Processing-Thread („could be called in the UI or in Processing
   Thread", `IAudioProcessor::setProcessing`, VST 3 API Documentation,
   https://steinbergmedia.github.io/vst3_doc/vstinterfaces/classSteinberg_1_1Vst_1_1IAudioProcessor.html);
   M-30 setzt den Hostthread voraus, und `reset()` nimmt nach §8.1 Feinheit 4
   zwei Sperren. Ob FL `setProcessing (false)` auf dem Audiothread ruft, ist
   ungemessen — Laufzeit-Arm oder Termin.
4. **Hostwert-Mailbox** (Abweichung 1): Entscheid des Dirigenten; R-4a-A folgt in
   der Nacharbeit 1 der Etappe 4a.
5. **Zählername.** `nichtEndlicheEingaenge` zählt jetzt auch die
   Verengungsriegel am Ausgang und in der Analysekopie; die Doku sagt es, der
   Name bleibt (keine Umbenennung nebenbei).
6. **Vorbestehende Warnungen in geänderten Testdateien:** C4456 in
   `DspGoldenTestMain.cpp` (Zeilen 953, 969, 3138, 3140) und C4457 in
   `MarkierungTestMain.cpp:1074` stehen auf Code, den diese Etappe nicht berührt
   (§33.4); nicht nebenbei behoben.
7. **Kein Kanon, kein Laufzeit-Arm** — auftragsgemäß am Ende der Etappe 6.
   `docs/PLAN-STAND.md` nicht angefasst, `planstand.py` nicht gelaufen.


## 34. Messung der Etappe 4, Einordnung der Abweichungen und Dirigenten-Übergabe vor der Erstprüfung 4 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `e01e87c3` (`nakama-nak283-0f0bc1e-etappe4`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-4-auftrag.txt` (§32); Start 00:32 Uhr, auf Anweisung des Dirigenten bis zur Aufhebung um 01:25 Uhr nur lesend (fremder Kanon in einer Arbeitskopie, §33 „Nebenläufig"), Bau ab 01:26 Uhr, letzter Push 02:59 Uhr, Ende 03:0x Uhr; Beobachter (Aufsicht ENG): Start, Telemetrie gesund, zweimal einmalig „Statusquelle unbekannt" unter Bau-Last (Fehlalarme nach Skill §3.2), fünf HEAD-Wechsel, Ende; Stundenloop `ec04109e` gelöscht, `CronList` leer. Laufzeit 2 h 3x min. |
| Commits | `fe6ee58f` (Produkt und Tests; vor dem Rebase `7c139163`, Blobs gleich — `git diff --stat 7c139163 fe6ee58f -- eq-copilot` zeigt nur die per Rebase hereingekommene `.clang-tidy` aus NAK-288), `f5011dfe` (20 Rotbeweise, Beinläufe, Skripte, Riegel M-43, §33), `270167fb` (A33 nach dem Rebase), `6f0d35d7` (Kopfzeile), alle auf `origin/master`; Basis `0f0bc1ed` ist Vorfahr; eigene Pfade sauber (nur die fremden untracked Ordner aus §27). **Fremd und unberührt, durch den Rebase davor:** die NAK-288-Commits der Session `nakama-f7` (`4e70e369` bis `63df1e95`) und die Dirigentencommits `289360c5`, `e4554125` (NAK-288 T2 PASS, Planstand) — kein Ticketpfad dieser Etappe. |
| Diff | `git diff --stat e4554125..6f0d35d7`: 38 Dateien — sechs Produktdateien (+257/−30: `SondeProcessor.cpp` 90, `.h` 42, `DspKern.cpp` 66, `.h` 27, `HoerMarkierung.h` 24, `StampedAudioQueue.h` 38), vier Testdateien (+699/−3), §33 und 28 Rohdateien. `broker/**`, `eq-copilot/plugin/state/**`, `core/analysis/**`, `schemas/**`, `identity/**`, `install/**`, `src/prozessor/**`, `tools/**` unberührt (gemessen, leer). |
| Rundenbilanz | `e4554125..6f0d35d7: Produkt 6 Datei(en) +257/-30 \| Tests 4 Datei(en) +699/-3 \| Doku 28 Datei(en) +13816/-2` — dritte Produktrunde des Tickets; kumuliert `--runden bb49a612 988dc44d faea3c1a 3d6bbfb4 30a0b1bf 6f0d35d7`: „kein Konvergenz-Signal". |
| Eigene Messung | Rotbilanz `NAK-283-etappe-4-rotbilanz.txt` gelesen: 20 von 20 BESTANDEN, je rot Exit 1 mit Trägerzeile, grün Exit 0 mit Trägerzeile, Hash gleich; Beine `NAK-283-etappe-4-beine.txt`: 38 von 38 Exit 0, 42 Ziele, keine Warnung auf einer geänderten Zeile; A33 nach dem Rebase Exit 0, 83 Fundstellen (Ratsche gehalten, `NAK-283-etappe-4-tidy.txt`). Vom Dirigenten am Stand `6f0d35d7` um 03:02 Uhr selbst gefahren (Binaries 02:34 bis 02:38 Uhr, Quellblobs gleich `fe6ee58f`): A16 `EqCopProbeeqNullTest` 65 Prüfungen, 0 Fehler, Exit 0 (mit Abschnitt 3b); A1 `EqCopNullTest` 11 ok, Exit 0; B7 `EqCopTransactionTest` 174 geprüft, 0 Fehler, Exit 0; B6 `EqCopDspGoldenTest` 259 geprüft, 0 Fehler, Exit 0; A3 `EqCopMarkierungTest` 116 ok, Exit 0 — identisch mit §33.4. Riegelzeilen der Tests am Stand gemessen und gleich §33.2. Ausgangsriegel an der Quelle gelesen: `DspKern.cpp:1063`/`:1068` `!isfinite(float) && isfinite(double)`, Eingangsriegel `:907-908`; `reset()` `SondeProcessor.cpp:267-289` nimmt `getCallbackLock()` vor `zustandSchloss` und ruft nur `beendeAudiohistorie`. |
| Messabdeckung | 17 Matrixzeilen (M-27 bis M-43) je mit Test und Rotbeweis; Regressionswachen (M-28, M-29, M-31, M-33, M-34, M-37, M-38, M-42) als solche benannt; keine Nebenläufigkeitszeile nach §6.1. Kein Fortsetzungsauftrag nötig. |
| Abweichungen §33.6 (Einordnung) | **Elf gedeckt, zwei Lücken mit Regel, kein Befund** — an Diff und Quelle gelesen. **Gedeckt:** 2 (`beendeAudiohistorie` als Kernmethode — Ticketgrenze in engster Form, §3 nennt `DspKern.cpp`/`.h`; ohne sie erreichte `reset()` die privaten Pfadzustände nicht), 3 (drei Testzugänge nur unter `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3`, Muster Etappe 2 Abweichung 1), 4 (zwei weitere Passthrough-Kommentare — von der M-43-Zusage gedeckt, Riegel R5), 6 (Beine per MSBuild-Trackinglog — Mehrmessung, vom Auftrag verlangt), 7 (M-31: die wörtliche Mutation kippt nur zwei Messgrößen, MUT-31b mit Publikation kippt alle drei — Messung verschärft), 8 (M-34: „reset-Fassung, die den Ausgangspuffer nullt" als Marke, weil `reset()` keinen Puffer hat — gleiche Wirkung), 9 (M-29 ohne genannte Mutation — gebrochen ist die Zusagezeile Ein- gleich Ausgang), 10 (M-42 in zwei Hälften mit je eigener Mutation), 11 (M-43-Riegel liest den gefalteten Wortlaut, dazu der Basislauf), 12 (Dateinamen), 13 (Frische je Artefakt statt gegen die jüngste Quelle überhaupt — strenger als der Runner, `tools/beweise.ps1:982-987`). **Lücke mit Regel, keine Nacharbeit:** 1 und 5 (nächste Zeilen). |
| Regel zu Abweichung 1 — R-283-3 präzisiert (13.09.2026) | R-283-3 und die Reihenfolge-Spalte von M-30 nennen die Hostwert-Mailbox unter den zurückzusetzenden Dingen; dieselbe Regel verlangt „ohne Programm, Parameter oder Zustand zu ändern". Die Mailbox (`hostWert`, `hostEreignis`, `hostEreignisOffen`) trägt ungelesene Hostparameterwerte, keine Audiohistorie; der Kontrolltakt (`dspKontrollTakt`, 5 ms) löst sie unabhängig vom Audiocallback ein. Ein `reset()`, das sie leerte, ließe den wirksamen Zustand vom Hostparameter abweichen (Parameterhoheit, `CLAUDE.md`); sie in den Overlay zu übernehmen, verlangte `publikationOffen = true` gegen §6.5. **Regel:** `reset()` setzt allein die Audiohistorie des Kerns zurück (Filterzustände, Rampen, Crossfades, laufender Hörmatrix-Fade) und lässt die Hostwert-Mailbox unberührt; ihre Bereinigung bei Abweisung und Ladestart bleibt R-4a-A der vertagten Nacharbeit 1 (SONDE-015 §12). Die Zusage von M-30 (Stille → Stille, Peak exakt 0) ist davon unberührt und gemessen. |
| Regel zu Abweichung 5 — R-283-6 präzisiert (13.09.2026) | R-283-6 sagt „nicht-endliche float-Werte werden verriegelt"; M-42 verlangt zugleich „der Dry-Zweig bleibt roh" und dass der Riegel den Nulltest und den Passthrough nicht berührt. Gebaut ist `!isfinite(float) && isfinite(double)` (`DspKern.cpp:1063`, `:1068`): verriegelt und in `zaehlerEingaenge` gezählt wird, was erst die Verengung nicht-endlich macht. **Regel:** ein schon als double nicht-endlicher Wert in der Ausgangsschleife ist rohes Dry-Material (Dry, Delta, Passthrough-Seite eines Crossfades), bleibt roh und ist vom Eingangsriegel (`:907-908`) bereits gezählt; die Endlichkeit des Wet-Pfads trägt SONDE-015 M-49 (Eingangsriegel je Sample, Zustandsriegel am Blockrand, `DspFilter.h` `istEndlich`). Jeder nicht-endliche Wert zählt genau einmal an seinem Ursprung. Ob ein Wet-Wert innerhalb eines Blocks als double nicht-endlich werden kann (Zustandsüberlauf zwischen zwei Blockrandprüfungen), prüft die Erstprüfung 4 lesend an der Quelle (Prüffrage 2 (d)); ein konkreter Pfad wäre ein Defekt gegen R-283-6, ein theoretischer Randfall ohne Pfad Härtung. |
| §33.7 offene Punkte | 1 (Runner-Behauptungen A16, B7, B6, A3 in `tools/beweise.ps1` nennen die neuen Zusagen nicht): `tools/**` liegt außerhalb der Etappengrenze; Nachzug beim Ticketabschluss oder in Etappe 6 (dort ist `tools/beweise.ps1` „nur für Beine" Ticketpfad) — im lebenden Kopf unter „Ticketpfade" vermerkt. 2 (`dropsOhneAudio` reist nicht über den Draht, `framesDropped` summiert weiter) und 3 (Thread von `reset()`: `setProcessing` darf laut VST3-API auf dem UI- oder Processing-Thread kommen; ob FL `setProcessing(false)` auf dem Audiothread ruft, ist ungemessen — dann nähme `reset()` zwei Sperren auf dem Audio-Thread): **Härtung/Messung**, Register **NAK-291** (Laufzeit-Arm-Szenario nach NAK-286 oder Termin). 4 (Mailbox): entschieden, Regel oben. 5 (Zählername `nichtEndlicheEingaenge` zählt jetzt auch die Verengungsriegel; Doku sagt es): kein Punkt. 6 (vorbestehende C4456/C4457 in Testdateien): NAK-276. |
| Hygiene des Fensters (03:03 Uhr) | MEMORY.md 5 338 B, CLAUDE.md 19 434 B (≤ 20 480; +414 B durch die NAK-288-Commits: Arbeitsregel `tidy.py`, Landmine NAK-290), Skill 24 369 B (≤ 24 576, Worktree LF); keine Indexzeile über 250 Zeichen, keine Memory-Datei ohne Indexlink; `dokuriegel.py` auf CLAUDE.md, Skill, Register und Manifest ohne Befund; `gesundheit.py` Exit 4 — Funktionen über 200 Zeilen 29/28 (Anzahl unverändert gegen 00:27 Uhr, `HoerMarkierung.h:431 verarbeite` 230 Zeilen), Kommentar-Bezeichner 32/30 (gegen 34 um 00:27 Uhr), MEMORY.md und Skill-Bytes OK — NAK-255 unverändert, kein neuer Riss aus dieser Etappe. **Werkzeugbefund (NAK-288 NB-3):** in einem Klon mit `autocrlf` misst A32 die Skill-Bytes mit CRLF (≈ 24 800 B) und meldet einen Riss, der im LF-Worktree nicht besteht — `gesundheit.py` sollte LF-normalisiert messen; Registernachtrag NAK-255. Skill §3.5 um `tidy.py`/A33 ergänzt (Registerbedarf NAK-288). |
| Erstprüfung 4 (vorbereitet, nicht gestartet) | Kontext dieser Session **50 %** vor dieser Messung (Skill §5: ab 500k keine neue Prüfrunde) — Übergabe. Der Prüfauftrag liegt **fertig** (kein Entwurf) als `docs/beweise/roh/NAK-283-etappe-4-erstpruefung-auftrag.txt`: Vorlage A, Prüfbereich `git diff 0f0bc1ed...6f0d35d7 -- <die zehn Ticketpfade>`, Gate wörtlich, Matrix §5.3, Riegelzeilen gemessen, Rotbeweise und Beine, die dreizehn Abweichungen mit dieser Einordnung, beide Regelpräzisierungen, Ausschlüsse (`docs/**`, NAK-269, NAK-249, NAK-113, NAK-276, NAK-287 bis NAK-291, Etappen 2, 3, 5, 6, SONDE-015 §12, §33.7 Punkte 1 und 5, Trackinglog-Auswahl, Frische je Artefakt), Prüffragen (a) bis (e). |
| Übergabe | Nächste Session (Marker-Neustart nach Skill §5): `git pull --ff-only`, Anker `NAK-283` bleibt gesetzt, HEAD muss `6f0d35d7` plus die Commits dieses Abschlusses sein; dann sofort `pwsh -NoProfile -File tools/dirigent/codex-lauf.ps1 -Kennung nak283-etappe4-erst -Prompt <Auftrag> -HeadSha <HEAD>` (Astra max), Monitor auf das Startlog bis `EXIT=`, §35 „Erstprüfung 4" mit Thread-ID und Urteil; danach Validierung durch einen lesenden Opus-Agenten, Einordnung, Nacharbeit (Runde 1 von 3) oder Abschluss der Etappe 4 → Etappe 5 (§6.6: Analysefachmodelle M-44 bis M-60, M-73, M-74; Aufsicht ENG). **Vor Etappe 5** sind laut Register NAK-283 die Karten U40 bis U42 zu stellen (Produktfragen, nur 9:00–23:00 Uhr; U40 vertagt bis zur Messung, NAK-284) — blockieren sie, Etappe 6 (§6.7, `tools/eq-copilot/`, unabhängig von Etappe 5) vorziehen. Kanon und Laufzeit-Arm erst am Ende der Etappe 6. Kein Worker läuft; `CronList` leer; Beobachter beendet; Worker `e01e87c3` entfernt. |

## 35. Erstprüfung 4 — PASS; Abschluss der Etappe 4; Etappe 6 vorgezogen und gestartet (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Erstprüfung 4 | Vorlage A: Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`tools/dirigent/codex-lauf.ps1`, Kennung `nak283-etappe4-erst`); Auftrag `docs/beweise/roh/NAK-283-etappe-4-erstpruefung-auftrag.txt` (§34, unverändert gestartet); Prüfbereich `git diff 0f0bc1ed...6f0d35d7 -- <die zehn Ticketpfade>`; Start 03:10:34 Uhr, Ende 03:18:53 Uhr; HEAD `ae12272b` vor und nach dem Lauf identisch, Arbeitsbaum unverändert (nur die zwei fremden untracked Ordner, §27). Thread `01a09850-e91c-7880-b4e4-3b5a5c1078e9`; 52 lesende Kommandos im JSONL (712 KB), keine Kapazitäts- oder API-Fehler. Rohdatei `docs/beweise/roh/NAK-283-erstpruefung-4-ae12272.txt`. |
| Urteil | **PASS** — Codex wörtlich: „kein DEFEKT im gebundenen Änderungssatz festgestellt." Geprüft: „vollständiger Zehn-Dateien-Diff, M-27 bis M-43, präzisierte Regeln, Reset-/Bank-Lebenszyklus, Queue-Zeitbuchführung, Wet-/Dry-Verengungen, Testverdrahtung und Riegel; 20 Rotbelege samt Rücknahme-Hashes sowie relevante Ergebnisse der 38 Beine lesend abgeglichen; ausgeschlossene Produktpfade unverändert". Nicht geprüft: „eigene Bau-, Test-, Compiler-, Python- oder Produktläufe sowie voller Kanon und FL-Laufzeit — auftragsgemäß nicht ausgeführt". Keine Lücke und keine Härtung benannt; die Prüffrage 2 (d) des Auftrags (kann in der Ausgangsschleife ein als double nicht-endlicher Wet-Wert entstehen) endet ohne Befund — die Regel zu Abweichung 5 (§34) bleibt, wie eingetragen. |
| Einordnung | Kein Befund zu validieren, keine Nacharbeit. Die Präzisierungen von R-283-3 (Hostwert-Mailbox bleibt in `reset()` unberührt) und R-283-6 (verriegelt und gezählt wird, was erst die Verengung nicht-endlich macht) aus §34 sind damit geprüft und gelten für die Abschlussprüfung. |
| Rundenbilanz | Erstprüfung: keine Codeänderung. `6f0d35d7..ae12272b: Doku 5 Datei(en) +65/-9` — ohne Produktfortschritt (Übergabe §34, Register, Planstand). Kumuliert unverändert gegen §34 (`--runden bb49a612 988dc44d faea3c1a 3d6bbfb4 30a0b1bf 6f0d35d7`: „kein Konvergenz-Signal"). |
| Etappe 4 | **Abgeschlossen auf `6f0d35d7`** (Produktcode und Tests `fe6ee58f`): Erstprüfung 4 PASS in der ersten Runde, null Nacharbeitsrunden. Stand des Tickets: Etappe 1 PASS nach drei Matrix-Nacharbeiten (§19), Etappe 2 PASS nach zwei Nacharbeiten (§28), Etappe 3 PASS ohne Nacharbeit mit einer Lücke (§32), Etappe 4 PASS ohne Nacharbeit (§35). Offen: Etappen 5 und 6, Abschlussprüfung, Kanon, Laufzeit-Arm. Der Nachzug der Runner-Behauptungen A16, B7, B6, A3 in `tools/beweise.ps1` (§33.7 Punkt 1) bleibt beim Ticketabschluss (Kopfzeile „Ticketpfade"). |
| Hygiene des Fensters (03:2x Uhr) | MEMORY.md 5 338 B, CLAUDE.md 19 434 B (≤ 20 480), Skill 24 465 B (≤ 24 576; +96 B gegen §34 durch den Nachtrag §3.5 tidy.py/A33, Worktree LF); keine Indexzeile über 250 Zeichen, keine Memory-Datei ohne Indexlink; `dokuriegel.py` auf CLAUDE.md und Skill ohne Befund; `gesundheit.py` Exit 4 — dieselben zwei Maße wie §34 (Funktionen über 200 Zeilen, Kommentar-Bezeichner ohne Code; NAK-255), seit `6f0d35d7` kein Produktcode geändert, kein neuer Riss; A33 (`tidy.py`) am Stand `270167fb` gehalten (83 von 83), seither keine Übersetzungseinheit geändert. Keine Kontextfläche über der Grenze, nichts zu beheben. |
| Reihenfolge | **Etappe 6 vor Etappe 5** (Übergabe §34, Register NAK-283): die Karten U41 (Headroom-Verteilung ohne markierte Passage) und U42 (Kurzzeit-Lautheit in der Sperrfrist) sind Produktfragen zur Anzeige und werden nur im Zeitfenster 9:00–23:00 Uhr gestellt (User 11.09.2026); One-Shot-Cron `cca43804` um 09:04 Uhr stellt sie über `/fragen U41`, dann `/fragen U42`; U40 bleibt vertagt bis zur Messung (NAK-284). Etappe 6 (§6.7: `tools/eq-copilot/pruefe_session_soak.py`, `erzeuge_p4_korpus.py`, `Sonde013TruePeakGoldenTest.cpp`, `p4-korpus` über den Erzeuger, `tools/plan/gesundheit.py`, `tools/beweise.ps1` nur für Beine) teilt keine Datei mit Etappe 5 (`core/analysis/**`) und hängt an keiner der drei Karten. |
| Etappe 6 | Frischer Opus-Worker (max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-6-auftrag.txt` (Ticketpfade nach §3 und §6.7 wörtlich; Matrix M-61 bis M-70 in §5.5; Feinheiten 13 bis 16 samt der Herleitung von 64-fach und ±0,01 dB; Beine A24, neues Selbsttestbein, B17, A25, A26, A27, A32, B9, B18; Rotbeweise nach §6.1 — M-61, M-62, M-67, M-69, M-70 rot am Basis-SHA, M-63, M-65, M-66, M-68 gegen die Fixfassung, M-64 Regressionswache; kein Kanon, kein Laufzeit-Arm; Wortlaut des NAK-199-Nachtrags im Etappenabschnitt, Register bleibt Dirigentensache). Basis-SHA = HEAD nach diesem Commit (steht im Workernamen). Aufsicht **NORMAL** (Werkzeuge, Tests, Nachweise; kein Audio-, State-, Vertrags- oder Nebenläufigkeitscode); Beobachter und Stundenloop nach Skill §3.2. Danach Erstprüfung 6 (Vorlage A) und das Pflegeticket für die durch M-69 ehrlich gerissenen Gesundheitswerte (Muster S25b–d). |

## 36. Bauetappe 6 — Nachweise und Werkzeuge (Bauer, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **6 von 6, vor Etappe 5 vorgezogen** (F13, N01, NAK-249; Matrixzeilen M-61 bis M-70). Basis `7b1f38b7bddcf67965d1bbc2609fd74464cfaaea` (= `git rev-parse HEAD` beim Start); `git status --short` beim Start zeigt nur die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` (§27) — nicht angefasst, nicht committet. **Werkzeuge, Test, Korpus und Runner auf `7d65adc7`** (Commit 1); Riegel, Rot- und Beinskript, Rohdateien, der SONDE-013-Nachtrag und dieser Abschnitt im Commit 2, die Kopfzeile „Etappe" im Commit 3. Auftrag `docs/beweise/roh/NAK-283-etappe-6-auftrag.txt` (§35). Spezifikation: Matrix §5.5, Bauplan §6.7, Feinheiten 13 bis 16 (§8.1). |
| Umfang | **10 Matrixzeilen** gebaut und gemessen: M-61 bis M-70. Sieben Dateien im Commit 1 mit +1 165 / −77 Zeilen, dazu `docs/beweise/SONDE-013.md` mit +2 / −2 (nur Zuwachs in zwei Nachweiszellen) — §36.4. Kein Produktcode, kein Schema, kein State-Feld, kein Hostparameter, keine Identitätsänderung. |
| Beweise | **20 von 20 Rotbeweisen** rot → grün, jede Rücknahme bytegleich (Bilanz `docs/beweise/roh/NAK-283-etappe-6-rotbilanz.txt`, §36.2); dazu vier Beobachtungsläufe, darunter die wörtliche Matrixmutation von M-65 (Abweichung 1). **20 von 20 Beinläufen mit erwartetem Exit** (`docs/beweise/roh/NAK-283-etappe-6-beine.txt`, §36.5): A24, A34, B17, A25, A26, A27, B9, B18 und B27 Exit 0, A32 Exit 4 (nicht blockierend, ehrlich gerissen), dazu die Riegel M-66, M-67, M-68 und M-70 und `dokuriegel.py`. Die Referenz liegt an 15 bis 19 höchstens 0,000005 dB neben der analytischen Wahrheit, die Fälle 20 bis 23 höchstens 0,039 dB neben der Referenz und zwischen −0,164 und −0,153 dBTP (§36.3). Kein Kanon, kein Laufzeit-Arm (§6.8). |
| Abweichungen | **Zwölf**, alle in §36.6 mit Grund und Quelle. Keine Matrixzeile geändert — die Prüfung entscheidet. |
| Produktfragen | Keine beantwortet. |

### 36.1 Was gebaut wurde

Zeilenangaben zum Commit `7d65adc7`; Rotbeweise stellen die Bytes bytegleich wieder her (§36.2).

**F13 — eine fehlende Messung ist kein PASS (M-61 bis M-64).** Die Messstelle
`rss_messung` (`tools/eq-copilot/pruefe_session_soak.py:363` ff.) liefert bei
gelungenem Abruf `{"rss_bytes": n, "rss_gueltig": true}` — auch für n = 0 —
und bei gescheitertem `OpenProcess` oder `GetProcessMemoryInfo`
`{"rss_bytes": null, "rss_gueltig": false, "rss_fehler": "<Aufruf>: Fehler <n>"}`
(`_fehlmessung` `:358-360`). Die drei Windows-Aufrufe liegen in
`WindowsSpeicherApi` (`:310` ff.; Fehlercode über `use_last_error`, Handle als
`HANDLE`), damit der Selbsttest dieselbe Messstelle mit einem Ersatz fahren
kann. `speicherpunkt` (`:390` ff.) legt Zeitangaben und Messung unverändert
zusammen; `probe()` baut jeden Punkt darüber (`:530-532`). In `urteile`
(`:814` ff.) prüft `merkmal_vollstaendig` (`:1146` ff.) die Form: das Merkmal
ist an jedem Punkt ein Wahrheitswert, jeder gemessene Punkt trägt eine ganze
Zahl ≥ 0; nur `--bericht` lässt einen Altbericht zu, dessen Punkte das Merkmal
alle nicht tragen („Merkmal unbekannt", `:1720`). `gueltig_gemessen`
(`:1177-1201`) trennt die zwei Prüfungen — das Neustartfenster filtert die
Kurve, das Merkmal entscheidet über den Lauf: ein ungültiger Punkt im Fenster
wird genannt, ein ungültiger tragender Punkt meldet
`FEHLT … Messung fehlt` (`messung_fehlt` `:854` ff.) und die Budgetrechnung
entfällt (Aufrufe vor `wachstum`: `:1211`, `:1245`). Der Schluss
(`:1267-1277`): ein roter Prüfpunkt → ROT, Exit 2; sonst eine fehlende Messung →
MESSUNG FEHLT, Exit 3; sonst GRUEN. `--selbsttest` (Fälle ab `:1378`, Lauf
`:1656` ff.) baut einen vollständigen grünen A24-Bericht im Speicher aus den
Zahlen des Kontrolllaufs `4ff6f248` (abgeschrieben, nie gelesen) und fährt die
vier Fälle der Testtabelle §6.7 durch `rss_messung`, `speicherpunkt` und
`urteile`; jede Erwartung läuft mit ihrem Gegenteil. Neuer Mutant `rss_fehlt`
(`:263` ff., `:801` ff.) für den echten Bericht. Der Skriptkopf nennt die fünfte
Regel und die Exitcodes.

**N01 — die transienten EBU-Fälle 20 bis 23 (M-65, M-66).**
`eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp`, Kopf `:39-52`,
Helfer `:131-332`, Abschnitt 7 `:538` ff. Die Synthese bei 4·fs = 192 kHz
(`ebuTransientBei4fs` `:182`): fs/6-Sinus mit 0,50, darin eine fs/4-Periode
mit 1,00 ab dem 4·fs-Index 19 200 — ein Nulldurchgang des fs/6-Sinus und ein
Vielfaches von 4 (`static_assert` `:156`), also phasen- und wertstetig an
beiden Seiten; die Phase ist je Abschnitt geschlossen gerechnet. Der
Anti-Aliasing-Tiefpass (`aaKoeffizienten` `:199`): Kaiser-Sinc, 257 Taps,
6-dB-Punkt 0,45·fs, β = 10, laufzeitfrei angewandt (`aaGefiltert` `:229`);
dezimiert wird `x[versatz + 4·m]` (`dezimiert` `:249`). Die Referenz
(`referenzKerne` `:259`, `referenzTruePeak` `:289`): 64-fach, Kaiser-Sinc über
128 fs-Samples je Phase, β = 12, jede Phase auf Summe 1, Maximum im Inneren
[64, N − 64). Der Prüfling ist der unveränderte 8-fach-Pfad (`detektorTruePeak`
`:319`: beide Kanäle gleich, Einschwingen wie Abschnitt 1, Nachlauf).
Reihenfolge: 7a die Referenz an 15 bis 19 auf ±0,01 dB (`:549`), 7b der
Tiefpass gemessen (`:578`), 7c je Fall die Gegenprobe der Referenz am
4·fs-Signal (`:613`), der Referenzvergleich ±0,1 dB nur mit validierter
Referenz (`:616`) und die Normtoleranz (`:623`). Der P4-Korpus trägt den zweiten
Eintrag der Klasse `referenz` (`tools/eq-copilot/erzeuge_p4_korpus.py:97-106`),
über den Erzeuger neu geschrieben.

**NAK-249 — der Quellumfang aus dem Inventar (M-69).** `tools/plan/gesundheit.py`:
`INVENTARWURZELN` (`:208`), `AUSGENOMMEN` mit Grund (`:212`, dazu der
flatc-Codegen `eq-copilot/plugin/vertrag/generiert`), `TESTBAEUME` (`:219`);
`inventar` (`:461`), `quellorte` (`:494-496`), `inventarluecken` (`:499`, zählt
unabhängig von der Ableitung jede Quelldatei unter den Wurzeln) und
`sammle_quellen` (`:524`: fehlende Wurzel → Werkzeugfehler; danach der
Inventarriegel → Werkzeugfehler, Exit 2). Bericht und JSON drucken das Inventar
(`:1040` ff., `:1676`). Selbsttestfall `kein_quellbaum_bleibt_ungemessen`
(`:1509` ff., acht Prüfungen); der bestehende Fall „fehlender Messort" misst jetzt
eine umbenannte Wurzel. Ratschen und Grenzen unverändert — die neuen Werte
reißen ehrlich (§36.5).

**Runner** (`tools/beweise.ps1:614`, `:616`): die A24-Behauptung nennt die
fehlende Messung; neues Bein **A34** (`pruefe_session_soak.py --selbsttest`,
AbPhase P3) direkt hinter A24.

**SONDE-013** (`docs/beweise/SONDE-013.md:471`, `:623`): je ein datierter
Nachtrag „13.09.2026 (NAK-283, N01)" am Ende der Nachweisspalte.

### 36.2 Je Matrixzeile: Test und Rotbeweis

Alle Rotbeweise nach §6.1, gefahren von `docs/beweise/roh/NAK-283-etappe-6-rotskript.ps1`:
Originalbytes und SHA-256 jeder betroffenen Datei sichern, Mutation an der
Zusagezeile (jedes Muster genau einmal, sonst Abbruch), Zeitstempel, Neubau des
Ziels mit Frischeprüfung, roter Lauf — **ein Rotlauf zählt nur mit dem
erwarteten Exit und einer Trägerzeile (rote Marke plus Träger der Zusage)** —,
Originalbytes zurück (nie eine zweite Textersetzung), Hash gegen vorher,
Zeitstempel, Neubau, grüner Lauf mit Trägerzeile. Basis- und Registerläufe fahren
denselben Riegel rot am Basis-SHA (`--revision`) oder am Registerstand
(`--registerstand`) und grün am Arbeitsbaum. Keine Zeile dieser Etappe
ist Nebenläufigkeitszeile. Bilanz: `docs/beweise/roh/NAK-283-etappe-6-rotbilanz.txt`
(Commit 1 als HEAD).

| Zeile | Test (Datei:Zeile, Name) | Bein | Rotbeweis: Mutation | Rohdatei | rot · grün |
|---|---|---|---|---|---|
| M-61 | `pruefe_session_soak.py:1523` Fall `fehlende_rss_messung_ist_kein_pass` | A34 | MUT-61, `:1193-1200`: die Erfolgsbedingung in `gueltig_gemessen` entfernt · Basislauf: das Orakel des Basis-SHA (Riegel `basis-M-61-M-62 --revision 7b1f38b7`) | `NAK-283-rot-M-61-etappe-6.txt`, `NAK-283-rot-M-61-basis-etappe-6.txt` | rot Exit 2: das Szenario mit `null` bricht mit einer Ausnahme ab (Exit None), das Szenario mit `rss_bytes` 0 endet Exit 0 „0.0 -> 0.0 MB" im Budget · Basislauf rot Exit 1: das Orakel des Basis-SHA endet mit Exit 0, S07 „0.0 -> 0.0 MB" im Budget · grün Exit 0: Exit 3, MESSUNG FEHLT, nie im Budget |
| M-62 | `:1545` Fall `messfehler_und_nullmessung_sind_unterscheidbar` | A34 | MUT-62, `:360`: `_fehlmessung` liefert wieder `rss_bytes` 0 mit gültigem Merkmal · Basislauf wie M-61 | `NAK-283-rot-M-62-etappe-6.txt`, `NAK-283-rot-M-62-basis-etappe-6.txt` | rot Exit 2: PID 0 und die gescheiterte Speicherabfrage ergeben `{"rss_bytes": 0, "rss_gueltig": true}` wie die Nullmessung, im Urteil beide Exit 0 · Basislauf rot Exit 1: am Basis-SHA sind Messfehler und Nullmessung derselbe Punkt mit `rss_bytes` 0 · grün Exit 0: Messfehler `rss_bytes` null, `rss_gueltig` false, `rss_fehler` „OpenProcess: Fehler 87"; Nullmessung `rss_bytes` 0, `rss_gueltig` true |
| M-63 | `:1597` Fall `fehlerpunkt_ausserhalb_des_fensters_faellt` | A34 | MUT-63, `:1191-1192`: das Merkmal nur an Punkten im Neustartfenster geprüft | `NAK-283-rot-M-63-etappe-6.txt` | rot Exit 2: ein Fehlerpunkt direkt vor oder direkt nach dem Fenster endet mit `rss_bytes` 0 in Exit 0 und mit `null` in einer Ausnahme (Exit None); das Gegenteil, der Fehler im Fenster, endet mit Exit 3 · grün Exit 0: vor und nach dem Fenster Exit 3, im Fenster Exit 0 mit Hinweis |
| M-64 | **Regressionswache:** `:1628` Fall `altbericht_ohne_merkmal_bleibt_auswertbar` | A34 | MUT-64, `:1144`: `merkmal()` wertet ein fehlendes Merkmal als Messfehler | `NAK-283-rot-M-64-etappe-6.txt` | rot Exit 2: der Altbericht endet mit Exit 3 statt 0 · grün Exit 0: Exit 0, „121.7 -> 122.6 MB", HINWEIS „Merkmal unbekannt"; am echten Kontrollbericht §36.5 |
| M-65 | `Sonde013TruePeakGoldenTest.cpp:545` Abschnitt `true_peak_matches_ebu_transient_set`, Prüfzeilen `:616` (Referenz) und `:623` (Norm) | B17 | MUT-65b, `TruePeak.h:77`: 12 statt 24 Taps je Phase (Abweichung 1) · beobachtet: MUT-65-MATRIX, `TruePeak.h:74`: Faktor 4 statt 8, die Rotbeweisspalte wörtlich | `NAK-283-rot-M-65-etappe-6.txt`, `NAK-283-rot-M-65-matrix-etappe-6.txt` | MUT-65b rot Exit 1 mit genau einer roten Zeile (42 bestanden, 1 gescheitert): Fall 22 gemessen −0,25972 dBTP gegen die Referenz −0,12552, Abstand −0,13420 dB, die Normtoleranz hält; 15 bis 19, Sweep, Schranke und alle übrigen Zeilen grün · grün Exit 0: Fall 22 −0,16416, Abstand −0,03864. MUT-65-MATRIX: Exit 1 (41 bestanden, 2 gescheitert), rot nur „8-fach: die ausgerechnete Schranke" (−0,1685 dB) und „Ueberabtastfaktor ist 8", alle vier transienten Fälle unverändert grün — kein Rotbeweis |
| M-66 | `erzeuge_p4_korpus.py:101-106` zweiter Eintrag; A25 `--pruefen`; Riegel `NAK-283-etappe-6-riegel.py M-66` (K1 bis K3) | A25, A26, A27 | MUT-66, `erzeuge_p4_korpus.py:97-106`: zweiter Eintrag weg, `hinweis` des ersten auf 15-23 (Rotbeweisspalte wörtlich), Korpus unverändert · MUT-66b: dieselbe Mutation, der Korpus danach über den mutierten Erzeuger neu geschrieben | `NAK-283-rot-M-66-etappe-6.txt`, `NAK-283-rot-M-66-riegel-etappe-6.txt`, `NAK-283-rot-M-66b-etappe-6.txt`; Beobachtungen `NAK-283-rot-M-66b-A25-etappe-6.txt`, `-A26-`, `-A27-` | MUT-66: A25 rot Exit 2 „nicht bytegleich: referenz.json", Riegel rot Exit 1 (K1, K2 an der Neuerzeugung) · MUT-66b: Riegel rot Exit 1 (K1, K2 an Neuerzeugung und committetem Korpus), A25, A26 und A27 unter der Mutation Exit 0 (Abweichung 2) · grün je Exit 0, Riegel 5 von 5 |
| M-67 | Riegel `NAK-283-etappe-6-riegel.py M-67` (R1 bis R6) über `docs/beweise/SONDE-013.md:471` und `:623` | Riegel | MUT-67, `SONDE-013.md:471`: die Statusspalte umgeschrieben statt die Nachweisspalte ergänzt · Basislauf: derselbe Riegel am Basis-SHA | `NAK-283-rot-M-67-etappe-6.txt`, `NAK-283-rot-M-67-basis-etappe-6.txt` | MUT-67 rot Exit 1: R2 M-02 „veraendert: Zelle [5]" · Basislauf rot Exit 1: R4 M-02 und R4 M-81, kein Nachtrag · grün Exit 0, 14 von 14 |
| M-68 | Riegel `NAK-283-etappe-6-riegel.py M-68` (R0 bis R5) über den Wortlaut §36.7 und die Registerzeile NAK-199 | Riegel | MUT-68, §36.7: der Wortlaut behält den alten Umfang („True Peak über 15 bis 19") · Registerstand: derselbe Riegel an der Zeile, wie sie im Register steht | `NAK-283-rot-M-68-etappe-6.txt`, `NAK-283-rot-M-68-register-etappe-6.txt` | MUT-68 rot Exit 1: R1 und R3 · Registerstand rot Exit 1: R3 — der Nachtrag steht aus, Dirigentensache · grün Exit 0: Fixfassung 7 von 7 |
| M-69 | `gesundheit.py:1509` Selbsttestfall `kein_quellbaum_bleibt_ungemessen`; Basislauf Riegel `basis-M-69` | A32 | MUT-69, `gesundheit.py:496`: `quellorte` nimmt den Unterbaum `state` still aus dem Umfang · Basislauf am Basis-SHA | `NAK-283-rot-M-69-etappe-6.txt`, `NAK-283-rot-M-69-A32-etappe-6.txt`, `NAK-283-rot-M-69-basis-etappe-6.txt` | MUT-69 rot: `--selbsttest` Exit 2, 128 von 129 — der erste Fall fällt mit „WERKZEUGFEHLER: Inventarriegel: … eq-copilot/plugin/state (1 Datei(en))", `--mit-selbsttest` Exit 2 „ABBRUCH: Selbsttest rot" · Basislauf rot Exit 1: am Basis-SHA gemessen 152 Dateien, ungemessen acht Unterbäume mit 44 Dateien, der Selbsttest des Basiswerkzeugs grün (121 von 121) · grün: Exit 0 (129 von 129) und Exit 4 (A32 gerissen); am Arbeitsbaum gemessen 196 Dateien, kein Unterbaum ungemessen |
| M-70 | Riegel `NAK-283-etappe-6-riegel.py M-70`: C1 bis C3 (B27, M-07 bis M-09) und P1, P2 (A34, M-61 und M-62) | B27, A34 | MUT-62 bricht die Python-Hälfte · Basisläufe am Basis-SHA der Etappe 6 und der Etappe 1 | `NAK-283-rot-M-70-etappe-6.txt`, `NAK-283-rot-M-70-basis-etappe-6.txt`, `NAK-283-rot-M-70-etappe-1-basis-etappe-6.txt` | MUT-62 rot Exit 1: P2 (Selbsttest Exit 2, M-61 und M-62 rot) · `7b1f38b7` rot Exit 1: P1, C1 ok · `3c5ec9dd` rot Exit 1: C1 und P1 · grün Exit 0, 5 von 5 |

**Regressionswachen sind kein Beleg für ihren Befund** (`tools/dirigent/pruefliste.md` E): M-64. Die Belege liegen bei M-61, M-62 und M-63 (F13), M-65 bis M-68 (N01), M-69 (NAK-249) und beim Riegel M-70 (Übergang).

**SHA-256 der mutierten Dateien** (Bytes im Arbeitsbaum, vor jedem Rotlauf und nach jeder Rücknahme gleich; die Rohdateien tragen je Mutation vorher und nachher; `docs/beweise/NAK-283.md` trägt seinen Hash zum Stand des Rotlaufs in `NAK-283-rot-M-68-etappe-6.txt`):

| Datei | SHA-256 |
|---|---|
| `tools/eq-copilot/pruefe_session_soak.py` | `054EF7F2A0B4FE861B45BF59105EDE0F8165B5AB6BB05C58FE135B56188B602E` |
| `eq-copilot/plugin/core/analysis/TruePeak.h` | `39C8C79B2DF406E0AFAC12F7371A0FB2671DD91793C3E2E85A8E5C3B968B6D2F` |
| `tools/eq-copilot/erzeuge_p4_korpus.py` | `93A0049387641362A904C1D9F490F5ECDB37D5CE7DB2896473628D8AFD787143` |
| `eq-copilot/fixtures/p4-korpus/MANIFEST.json` | `8DC286E37FF938D9394324805C095CB517BA8F2EF8811FA90E985F5CF7EAA2D4` |
| `eq-copilot/fixtures/p4-korpus/referenz.json` | `8ACD00972BF63BDCD55F739CDBB13C6E7AC76B930F1566D8DD6AFDB53C836A51` |
| `docs/beweise/SONDE-013.md` | `07B3477958E231486EDE95DEF0E92E7E6C9A788C6A049D64D9B80D0A5D8CFE44` |
| `tools/plan/gesundheit.py` | `2F1B279F813E3212ABB44F8322FAC216BA23B9F717838E03FFF88711BB47A60A` |

### 36.3 Referenzvalidierung und die Fälle 20 bis 23

Gemessen von B17 (`docs/beweise/roh/NAK-283-etappe-6-beine.txt`, Abschnitt B17); dieselben Werte stehen in den grünen Läufen von `docs/beweise/roh/NAK-283-rot-M-65-etappe-6.txt`.

**7a — die 64-fach-Referenz an den analytisch bekannten Fällen** (Zusage: ±0,01 dB):

| Fall | Signal | Referenz | analytisch | Abstand |
|---|---|---:|---:|---:|
| 15 | fs/4, 0,50 FFS, 0° | −6,020600 dBTP | −6,020600 | −0,000000 dB |
| 16 | fs/4, 0,50 FFS, 45° | −6,020601 dBTP | −6,020600 | −0,000001 dB |
| 17 | fs/6, 0,50 FFS, 60° | −6,020604 dBTP | −6,020600 | −0,000005 dB |
| 18 | fs/8, 0,50 FFS, 67,5° | −6,020600 dBTP | −6,020600 | −0,000000 dB |
| 19 | fs/4, 1,41 FFS, 45° | +2,984381 dBTP | +2,984382 | −0,000001 dB |

**7b — der Tiefpass:** Durchlass bis 0,4·fs höchstens 0,000101 dB neben 0 dB, Sperrbereich 0,5·fs bis 2·fs höchstens −100,04 dB (Prüfgrenzen 0,001 dB und −90 dB).

**7c — die vier Fälle** (Normabstand = Messwert − 0,0 dBTP, Toleranz +0,2/−0,4 dB; Gegenprobe = größte Abweichung der Referenz vom gefilterten 4·fs-Signal an dessen Stützstellen, Grenze 1e-4):

| Fall (Versatz) | Referenz | Messwert 8-fach | Abstand zur Referenz | Normabstand | Sample-Peak | Gegenprobe |
|---|---:|---:|---:|---:|---:|---:|
| 20 (0) | −0,12552 dBTP | −0,15843 dBTP | −0,03292 dB | −0,15843 dB | −0,1584 dBFS | 0,000001470 |
| 21 (1) | −0,12552 dBTP | −0,15306 dBTP | −0,02755 dB | −0,15306 dB | −0,5581 dBFS | 0,000001375 |
| 22 (2) | −0,12552 dBTP | −0,16416 dBTP | −0,03864 dB | −0,16416 dB | −2,6495 dBFS | 0,000001249 |
| 23 (3) | −0,12551 dBTP | −0,15306 dBTP | −0,02755 dB | −0,15306 dB | −0,5581 dBFS | 0,000001216 |

Der Sample-Peak zeigt, dass die Fälle zwischen den Samples messen: bei Versatz 2 liegt das größte Sample 2,52 dB unter der Referenz. Der Messwert liegt 0,028 bis 0,039 dB unter der Referenz; Abstand zu den Grenzen 0,061 dB (±0,1 dB zur Referenz) und 0,236 dB (untere Normgrenze −0,4 dBTP). Der Interpolator hält sein Passband laut Kopf bis 0,4·fs (`eq-copilot/plugin/core/analysis/TruePeak.h:75-77`), der Tiefpass der Synthese hat seinen 6-dB-Punkt bei 0,45·fs; dass der Abstand aus diesem Band stammt, ist eine Erklärung, keine Messung. Unter den Mutationen: 4-fach (MUT-65-MATRIX) misst die vier Werte auf fünf Stellen gleich; 12 Taps je Phase (MUT-65b) misst Fall 22 mit −0,25972 dBTP (Abstand −0,13420 dB), die Fälle 21 und 23 mit −0,13441 dBTP (Abstand −0,00890 dB) und Fall 20 unverändert.

### 36.4 Geänderte Dateien

| Datei | Zeilen (+/−) | Was |
|---|---:|---|
| `tools/eq-copilot/pruefe_session_soak.py` | +573 / −36 | Messstelle mit Merkmal, `speicherpunkt`, S07-Form und -Erfolgsbedingung, eigener Status MESSUNG FEHLT, `--selbsttest` mit vier Fällen, Mutant `rss_fehlt`, Kopf |
| `tools/plan/gesundheit.py` | +253 / −37 | Inventar, Inventarriegel, Ausnahmen mit Grund, Bericht und JSON, Selbsttestfall, angepasster Messortfall, Kopf |
| `eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp` | +311 / −0 | Abschnitt 7 mit Synthese, Tiefpass, Referenz, Validierung und vier Fällen; Kopf |
| `tools/eq-copilot/erzeuge_p4_korpus.py` | +10 / −0 | zweiter Eintrag der Klasse `referenz` |
| `eq-copilot/fixtures/p4-korpus/referenz.json` | +12 / −0 | über den Erzeuger (`-text`, `i/lf w/lf` vorher und nachher) |
| `eq-copilot/fixtures/p4-korpus/MANIFEST.json` | +3 / −3 | über den Erzeuger (Hash, Bytes, 26 Fälle) |
| `tools/beweise.ps1` | +3 / −1 | A24-Behauptung, Bein A34 |
| `docs/beweise/SONDE-013.md` | +2 / −2 | Nachtrag am Ende der Nachweisspalten M-02 und M-81 (Riegel M-67) |

Zahlen aus `git diff --numstat 7b1f38b7 7d65adc7` beziehungsweise gegen den Basis-SHA. Neu unter `docs/beweise/roh/`: `NAK-283-etappe-6-riegel.py`, `NAK-283-etappe-6-rotskript.ps1`, `NAK-283-etappe-6-beinskript.ps1`, `NAK-283-etappe-6-rotbilanz.txt`, `NAK-283-etappe-6-beine.txt` und 24 Rohdateien `NAK-283-rot-M-*-etappe-6.txt`. Nicht angefasst: jeder Produktcode (`eq-copilot/plugin/**` außer der Testdatei, `broker/**`), `eq-copilot/plugin/core/analysis/TruePeak.h` (nur im Rotbeweis mutiert, bytegleich zurück), `eq-copilot/schemas/**`, `eq-copilot/identity/**`, `eq-copilot/install/**`, `tools/plan/tidy.py`, `tools/plan/planstand.py`, `tools/plan/dokuriegel.py`, `tools/eq-copilot/pruefe_p4_korpus.py`, alle anderen Tests und Manifeste, `docs/offene-punkte.md`, `docs/PLAN-STAND.md`, `docs/plan/`, `design/`.

### 36.5 Gefahrene Beine

Gefahren von `docs/beweise/roh/NAK-283-etappe-6-beinskript.ps1` aus pwsh am Commit `7d65adc7` (dazu im Arbeitsbaum dieser Abschnitt, der SONDE-013-Nachtrag und die Rohdateien); Rohausgabe `docs/beweise/roh/NAK-283-etappe-6-beine.txt`. Vor dem Bau gleicht das Skript Kürzel, Name, Art, Ordner und Argumente jedes Beins mit `tools/beweise.ps1` ab (PowerShell-AST) und prüft vor dem Bau und vor A24 auf fremde Bau-, Test- und Kanonprozesse (NAK-290): keine. Bau `cmake --build eq-copilot/build --config Release` über B17, B18, B9, B27 und `EqCopSessionSoak`: Exit 0 in 3,6 s ohne Warnung (nichts neu zu übersetzen; die Binaries stammen aus dem Rotskript). Die Broker-Probe ist jünger als jede Broker-Quelle; jedes Binary ist jünger als seine jüngste Eingabe (Zeitstempel mit Datum in der Rohausgabe).

| Lauf | Befehl | Exit (erwartet) | Befund |
|---|---|---|---|
| A34 | `py -3.13 tools/eq-copilot/pruefe_session_soak.py --selbsttest` | 0 (0) | 4 von 4 Fällen |
| B17 | `EqCopSonde013TruePeakGoldenTest` | 0 (0) | 43 bestanden, 0 gescheitert (§36.3) |
| A25 | `py -3.13 tools/eq-copilot/erzeuge_p4_korpus.py --pruefen` | 0 (0) | 5 Dateien, 26 Fälle, bytegleich |
| A26 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py` | 0 (0) | 26 Fälle, 0 falsche starke und 0 falsche schwache Behauptungen, Kalibrierung 0,0619; alle vier Klassen bestehen |
| A27 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py --selbsttest` | 0 (0) | 42 bestanden |
| B9 | `EqCopLoudnessGoldenTest` | 0 (0) | 109 Prüfungen, 0 Fehler (Klasse `referenz` des P4-Korpus) |
| B18 | `EqCopSonde013DynamicsTest` | 0 (0) | 46 bestanden, 0 gescheitert (Klasse `referenz`) |
| B27 | `EqCopSonde014IntentTest` | 0 (0) | 270 Prüfungen, 0 Fehler (C++-Hälfte M-70) |
| A32 | `py -3.13 tools/plan/gesundheit.py --mit-selbsttest` | 4 (4) | Selbsttest 129 von 129; gerissen, Tabelle unten |
| A32 vorher | Werkzeug des Basis-SHA am selben Arbeitsbaum | 4 (4) | Selbsttest 121 von 121; Tabelle unten |
| A24 | `py -3.13 tools/eq-copilot/pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | 0 (0) | GRUEN; alle zehn Speicherpunkte des Berichts `rss_gueltig` true; S07 Client 121,7 → 122,4 MB, Broker Generation 0 14,7 → 14,9 MB, Generation 1 14,5 → 15,1 MB (Grenze je 16,8 MB); P0 2 336 von 2 336 beantwortet, ACK-p95 21,8 ms; 148,5 s |
| A24 `--bericht` | derselbe frische Bericht, nur geurteilt | 0 (0) | GRUEN |
| A24 `--bericht --mutant rss_fehlt` | derselbe Bericht, jede tragende Messung die eines nicht abfragbaren Prozesses | 3 (3) | dreimal FEHLT, „MESSUNG FEHLT - 3 Speicherkurve(n)", nie im Budget (M-61 am echten Bericht) |
| NAK-246 `--bericht` | der Kontrollbericht `docs/beweise/roh/NAK-246-4ff6f24.md` | 0 (0) | GRUEN, dreimal HINWEIS „Merkmal unbekannt", Client 121,7 → 122,6 MB (M-64 an der Kontrollquelle) |
| Riegel M-66 | `NAK-283-etappe-6-riegel.py M-66` | 0 (0) | 5 von 5 |
| Riegel M-67 | `NAK-283-etappe-6-riegel.py M-67` | 0 (0) | 14 von 14 |
| Riegel M-68 | `NAK-283-etappe-6-riegel.py M-68` | 0 (0) | Fixfassung 7 von 7 |
| Riegel M-68 Registerstand | `NAK-283-etappe-6-riegel.py M-68 --registerstand` | 1 (1) | R3 rot: das Register trägt den Nachtrag noch nicht (Dirigentensache) |
| Riegel M-70 | `NAK-283-etappe-6-riegel.py M-70` | 0 (0) | 5 von 5 |
| dokuriegel | `docs/beweise/SONDE-013.md`, `docs/beweise/NAK-283.md` | 0 (0) | 0 Befunde |

**A32 vorher und nachher** — derselbe Arbeitsbaum, einmal mit dem Werkzeug des Basis-SHA, einmal mit dem Inventar; Produktcode in beiden Fällen gleich dem Basis-SHA:

| Maß | Grenze | vorher | nachher | neu sichtbar |
|---|---:|---:|---:|---|
| Quellumfang | — | 3 Orte, 152 Dateien | 15 von 17 Inventareinträgen, 196 Dateien | acht Unterbäume mit 44 Dateien |
| Quelldateien über 2 000 Zeilen | 0 | 0 | **1** | `eq-copilot/plugin/state/NakamaState.cpp` 2 911 Zeilen, OHNE PFLEGETICKET |
| Quelldateien über 1 500 Zeilen (Ziel) | — | 8 | 8 | — |
| Funktionen über 200 Zeilen | 28 | 29 | **35** | `NakamaState.cpp:1445` `leseSchema2` 593, `NakamaTransaktion.cpp:404` `Transaktionskern::fuehreAus` 305, `NakamaState.cpp:806` `synchronisiert` 279, `HostProbeProcessor.cpp:188` `HostProbeProcessor::nakamaBlockEmpfangen` 261, `DspKern.cpp:856` `DspKern::verarbeiteStueck` 230, `NakamaVertrag.cpp:474` `textriegel` 207 |
| allow(dead_code) ohne Aufrufer | 0 | 0 | 0 | — |
| Kommentar-Bezeichner ohne Code | 30 | 32 | **36** | `NakamaTransaktion.h:18` und `:154` `automation_epoch`, `NakamaEvidenz.h:6` `ed9bbf7`, `NakamaTelemetrie.cpp:161` `stringsGueltig` |

Nichts davon ist repariert (Auftrag); die Ratschen stehen unverändert.
### 36.6 Abweichungen vom Bauplan §6.7

Zwölf; keine ändert eine Matrixzeile. Die Prüfung entscheidet.

1. **M-65: die Matrixmutation „8-fach auf 4-fach" macht die transienten
   Fälle nicht rot.** Gemessen im Rotskript (MUT-65-MATRIX, 41 bestanden,
   2 gescheitert): mit Faktor 4 messen die Fälle 20 bis 23 auf fünf Stellen
   dieselben Werte, 15 bis 19 bleiben grün; rot werden nur die Schranke
   `interpolationsfehlerDb` (−0,1685 dB) und die Faktorprüfung. Erklärung: die
   Synthese legt den Scheitel der Einzelperiode auf das 4·fs-Raster (Einsatz bei
   19 200, Scheitel bei +4 und +12), und ein 4-fach-Interpolator rechnet bei
   jedem Versatz genau auf diesem Raster; der Rasterfehler, der 4-fach von
   8-fach trennt, trifft diese Signale nicht — ebenso wenig 15 bis 19, deren
   Scheitel auf dem 4-fach-Raster liegen. Die
   Norm nennt als Gegenstand der Fälle „pass-band ripple in the upsampling
   filter" und „under-read" (§2.6, zitiert in der Quellvalidierung Teil C).
   Den Rotbeweis an der Zusagezeile trägt deshalb **MUT-65b** — ein kürzerer
   Interpolator (12 Taps je Phase), also genau der Durchlassfehler, den die
   Fälle messen: Fall 22 reißt ±0,1 dB (−0,134 dB), die Normtoleranz hält
   (−0,260 dBTP), 15 bis 19, Sweep und Schranke bleiben grün — die Reihenfolge,
   die die Matrix zusagt („die transienten Fälle reißen zuerst … bevor die
   weitere Normtoleranz reißt"). Die Matrixmutation lief wörtlich und steht als
   Beobachtung in der Bilanz. Quellen: `TruePeak.h:21-33` (4-fach genügt
   BS.1770; die 8-fach-Entscheidung trägt die Schranke), §8.1 „Woher die 64".
2. **M-66: A26 liest `hinweis` nicht.** Die in der Rotbeweisspalte beschriebene
   Wirkung („A26 misst eine Behauptung, die kein Fall trägt") ist keine rote
   Zeile von A26 — sie ist genau der unsichtbare Fehler. Rot fällt an A25
   (Erzeuger gegen committeten Korpus, MUT-66) und am neuen Riegel M-66
   (Korpusinhalt gegen den Wortlaut der Zusage); MUT-66b zeigt, dass nach dem
   Neuschreiben über den mutierten Erzeuger A25, A26 und A27 grün bleiben und
   nur der Riegel fällt. `tools/eq-copilot/pruefe_p4_korpus.py` liegt außerhalb der Ticketgrenze
   (offener Punkt 4).
3. **Status und Exit der fehlenden Messung.** MESSUNG FEHLT endet mit Exit 3 —
   im Runner `[FEHLT]`, Kanonurteil UNVOLLSTAENDIG, nie GRUEN —; ein zugleich
   roter Prüfpunkt gewinnt (Exit 2). M-61 verlangt „einen von 0 verschiedenen
   Exitcode" und „Messung fehlt" statt „im Budget", M-63 „rot enden"; gelesen als „nie grün, Exit ≠ 0". Quellen:
   `tools/dirigent/pruefliste.md` D („meldet Voraussetzung-fehlt (Exit 3) statt
   grün", in §4 F13 zitiert), M-70 („an einem eigenen Status erkennbar"),
   `tools/beweise.ps1:1096-1117` (Exit 3 wird `[FEHLT]`) und `:1199-1202` (UNVOLLSTAENDIG).
4. **Der Wert eines gescheiterten Abrufs ist `null`, nicht 0** (M-62: „den
   Fehlerwert wieder als Zahl 0 kodieren" setzt das voraus). Die Szenarien von
   M-61 und M-63 fahren zusätzlich die deklarierte Abweichung „`rss_bytes` 0,
   Merkmal ungültig" (Prüfliste E: „deklarierte Mutanten genau eines Writer-Fixtures mit benannter Abweichung"), damit
   die Erfolgsbedingung ihr einziger Träger ist: unter MUT-61 endet genau dieses
   Szenario mit „0.0 -> 0.0 MB" im Budget und Exit 0 — die Erwartung der
   Rotbeweisspalte („`0,0 → 0,0 MB` besteht grün") —, das `null`-Szenario bricht mit einer Ausnahme ab.
5. **M-64 präzisiert.** Die Rückwärtskompatibilität gilt für gespeicherte
   Berichte (`--bericht`) und nur, wenn kein Punkt das Merkmal trägt; der
   Livelauf und ein gemischter Bericht verlangen es (Pflichtfeld, ROT). Ein
   Livelauf ohne Merkmal kann nur aus einer kaputten Messstelle stammen — genau
   F13. Beide Gegenteile laufen im Fall M-64 mit.
6. **Über die Matrix hinaus gebaut, als Gegenprobe, nicht als Zusage:** Mutant
   `rss_fehlt` (F13 am echten A24-Bericht, §36.5); die Ersatz-API für die
   Messstelle (einen echten Prozess mit 0 Bytes gibt es nicht); in B17 die
   Messung des Tiefpasses, die Gegenprobe der Referenz am 4·fs-Signal und die
   Indexprüfung für Versatz 3.
7. **NAK-249: Ausnahmen und Riegelklasse.** `eq-copilot/plugin/vertrag/generiert`
   steht neu in `AUSGENOMMEN` — flatc-Codegen, A9 hält ihn bytegleich, dieselbe
   Begründung wie `broker/src/generiert` (M-69: „Generiertes bleibt
   ausgeschlossen") —, `tests` als `TESTBAEUME`. Der Inventarriegel meldet als
   Werkzeugfehler (Exit 2) wie der Ortsriegel, weil ein ungemessener Unterbaum
   jedes andere Maß falsch machte. Der bestehende Selbsttestfall „fehlender
   Messort" misst eine umbenannte Wurzel, weil ein umbenannter Unterbaum kein
   fehlender Messort mehr ist; die Fallzahl steigt von 121 auf 129.
8. **Zahlen zu M-69.** Die Matrix nennt „31 handgeschriebene Dateien mit
   13 560 Zeilen" und `NakamaState.cpp` mit 2 863 Zeilen (Stand Etappe 1).
   Gemessen am Basis-SHA der Etappe 6: 44 Dateien mit 17 825 Zeilen in denselben
   acht Unterbäumen — `state` 7 010, `dsp` 3 320, `vertrag` 3 027, `sonde` 1 828,
   `hostprobe` 1 285, `spike` 682, `hostbridge` 486, `probe` 187 —,
   `NakamaState.cpp` 2 911 Zeilen (`NAK-283-rot-M-69-basis-etappe-6.txt`).
9. **M-68.** Der Wortlaut ist auf den Messtag 13.09.2026 datiert statt auf den
   Planungstag „12.09.2026" der Matrix. Der Riegel misst die Fixfassung
   (Registerzeile plus Wortlaut), weil das Register Dirigentensache ist; sein
   Lauf am Registerstand ist rot, bis der Nachtrag steht.
10. **M-70.** Am Basis-SHA der Etappe 6 steht die C++-Hälfte schon (Etappe 2);
    rot ist dort die Python-Hälfte. „ROT am Basis-SHA für beide" (Matrix, Stand
    Etappe 1) belegt der zusätzliche Lauf am Basis-SHA der Etappe 1 `3c5ec9dd`.
11. **Beine über §6.7 hinaus:** B27 (C++-Hälfte M-70), A32 vorher mit dem
    Werkzeug des Basis-SHA, der frische A24-Bericht über `--bericht` mit und
    ohne `rss_fehlt`, der Kontrollbericht NAK-246 über `--bericht`, die Riegel.
    Kürzel **A34**: die nächste freie Nummer (A33 ist `tidy.py`), eingetragen
    direkt hinter A24 (Muster A27).
12. **Dateinamen:** die Rohdateien folgen dem Auftrag
    (`NAK-283-rot-M-nn-etappe-6.txt`) mit Suffixen für Basisläufe, Matrix-,
    Riegel-, Register- und A32-Läufe; dazu `NAK-283-etappe-6-rotbilanz.txt` und
    das Riegelskript `NAK-283-etappe-6-riegel.py`.

### 36.7 Wortlaut des NAK-199-Nachtrags (M-68)

**M-68 · Wortlaut für die Registerzeile NAK-199** — einzufügen vom Dirigenten am
Ende der Kurzgrundzelle von NAK-199 in `docs/offene-punkte.md`, vor dem
schließenden Tabellenstrich; der bestehende Wortlaut der Zeile bleibt, wie er
ist:

```text
**Nachtrag 13.09.2026 (NAK-283, N01):** True Peak jetzt über 15 bis 23 gedeckt — B17 misst seit NAK-283 Etappe 6 auch die transienten EBU-Tech-3341-Fälle 20 bis 23 (`true_peak_matches_ebu_transient_set`, Versatz 0 bis 3: je ±0,1 dB gegen eine im Test gerechnete, an den Fällen 15 bis 19 validierte 64-fach-Referenz plus Normtoleranz 0,0 +0,2/−0,4 dBTP; eigener P4-Korpuseintrag; `docs/beweise/NAK-283.md` §36). Der LUFS-I-Teil dieser Zeile bleibt offen.
```

Reihenfolge nach M-68: erst der Nachweis (M-65, M-66 — gemessen, §36.2), dann
die Behauptung. Der Riegel M-68 prüft genau diese Reihenfolge (R0) und die
Fixfassung (R3 bis R5).

### 36.8 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Abschnitt | Wo gemessen |
|---|---|
| **C** — Zahlenränder | Versatz 3 am Rand: `x[3 + 4·(N − 1)]` ist der Index 38 399 von 38 399 (Prüfzeile `:597`). Filterlänge gegen Signallänge: die Einzelperiode beginnt beim fs-Sample 4 800 und endet 4 796 fs-Samples vor dem Signalende, der Tiefpass reicht ±32 fs-Samples (±128 Taps bei 4·fs), der Referenzkern ±64; das Referenzmaximum läuft nur im Inneren [64, 9 536). `static_assert` auf Nulldurchgang und fs-Raster; `db (0)` liefert −400 statt eines Logarithmus von 0. Das Merkmal ist ein Wahrheitswert (Zahl oder Text sind rot), ein gemessener Wert eine ganze Zahl ≥ 0 und kein bool. |
| Altbericht ohne Merkmal | Fall M-64 im Selbsttest und der echte Kontrollbericht über `--bericht`: Exit 0, „Merkmal unbekannt" (§36.5). |
| Inventar bei leerem Unterbaum | Selbsttest: ein leerer Ordner und ein Ordner nur mit Doku stehen nicht im Inventar; ein Unterbaum nur aus Codegen oder Tests steht als ausgenommen; geschachtelter Codegen zählt im gemessenen Unterbaum als ausgenommen; am echten Baum trägt `vertrag` 8 Dateien, davon 1 ausgenommen (Rohausgabe A32). |
| Exit-Codes je Status | Soak-Orakel: 0 GRUEN, 2 ROT (auch Pflichtfeld), 3 MESSUNG FEHLT (nur ohne ROT), `--selbsttest` 0 oder 2. Gesundheit: 0 gehalten, 4 gerissen, 2 Werkzeugfehler (Selbsttest rot, Messort fehlt, Inventarriegel), 3 `--clippy` ohne cargo. B17: 0 oder 1. Riegel: 0 oder 1. |
| **D** — Bau- und Prüfriegel | Beinskript: Kanonabgleich per AST, fremde Prozesse vor Bau und vor A24 (NAK-290), Frische je Artefakt gegen seine jüngste Eingabe. Rotskript: Muster genau einmal, Frische nach jedem Neubau, rot nur mit erwartetem Exit und Trägerzeile, Originalbytes zurück, Hash gleich. |
| **E** — jede neue Prüfung einmal gebrochen | 20 Rotbeweise, 13 aus zehn Mutationen und 7 aus fünf Basis- oder Registerläufen; M-64 als Regressionswache benannt; die Matrixmutation von M-65 als Beobachtung. |
| **E** — Behauptung ≤ Messung | Die A24-Behauptung nennt die fehlende Messung; Skriptköpfe von `pruefe_session_soak.py`, `gesundheit.py` und des Goldens nachgezogen. Die Runner-Behauptungen von B17 (nennt nur 15 bis 19) und A32 („121 Faelle", „Zeilengrenze HAELT") sagen weniger beziehungsweise Veraltetes — außerhalb der Runnergrenze dieser Etappe, offener Punkt 1. |
| **E** — Writer-Fixtures | Korpus nur über den Erzeuger, Zeilenenden vorher und nachher `i/lf w/lf -text`; die Altberichtsform im Selbsttest ist ein deklarierter Mutant des Writer-Punkts (genau `rss_gueltig` fehlt). |
| **F** — Änderungssatz | Messstelle, Speicherpunkt, Bewertung, Mutant, Selbsttest und Runnerbehauptung in einem Commit, ebenso Test und Korpuseintrag; der SONDE-013-Nachtrag steht mit diesem Abschnitt im Commit 2, weil er auf die gemessenen Werte verweist. |
| NaN/Inf, stale Zustand | Referenz und Detektor bekommen nur endliche Samples; die Budgetrechnung sieht nie einen `null`-Wert. Der Selbsttest ersetzt `inventar`, der Basislauf `ctypes.windll` — beide nur in `try/finally`. |
| Audio-Thread-Regel | Nicht berührt: kein Produktcode. |

### 36.9 Offene Punkte dieser Etappe

1. **Runner-Behauptungen B17 und A32.** B17 nennt nur die Fälle 15 bis 19; A32
   nennt „121 Faelle", „`broker/src/generiert/` ausgenommen" und „Die
   Zeilengrenze HAELT seit NAK-225" — seit dem Inventar gilt das nicht mehr
   (`NakamaState.cpp` 2 911 Zeilen). `tools/beweise.ps1` war in dieser Etappe
   nur für A24 und A34 frei; Nachzug beim Ticketabschluss zusammen mit A16, B7,
   B6 und A3 (§33.7 Punkt 1, Kopfzeile „Ticketpfade").
2. **Register (Dirigentensache).** Der NAK-199-Nachtrag (Wortlaut §36.7; der
   Riegel am Registerstand ist bis dahin rot); NAK-249 ist durch M-69 gebaut; das
   Pflegeticket für die ehrlich gerissenen Werte (Muster S25b–d): eine Datei über
   2 000 Zeilen ohne Pflegeticket, Funktionen über 200 Zeilen 35 bei Ratsche 28,
   Kommentar-Bezeichner 36 bei Ratsche 30 — die neu sichtbaren Treffer stehen in
   §36.5.
3. **EBU-Synthese ohne Abgleich mit den offiziellen Testdateien.** Die Norm legt
   Tiefpass und Einsatzstelle der Einzelperiode nicht fest; die Werte gelten für
   die dokumentierte Synthese. Ein Vergleich mit den EBU-Dateien der Fälle 20 bis
   23 ist nicht gemacht (keine Datei im Repo) — Härtung oder Messtermin.
4. **A26 prüft `hinweis` nicht gegen die Fälle** (Abweichung 2). Härtung;
   `pruefe_p4_korpus.py` außerhalb dieser Grenze.
5. **Vorbestehend, nicht in dieser Etappe geändert:**
   `eq-copilot/plugin/core/analysis/TruePeak.h:33` nennt den Fallnamen
   `interpolationsfehler_folgt_der_formel`, den es nicht gibt (das
   Kommentar-Bezeichner-Maß meldet ihn vorher und nachher); Prüftexte mit „§"
   erscheinen in der Ausgabe von B17
   (`eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp:438`), B18 und B27
   als „Â§", weil `juce::String` das UTF-8-Literal byteweise liest — die neuen
   Prüftexte in Abschnitt 7 sind ASCII.
6. **Audit-Beleg.** `docs/audits/2026-09-12-code-review-2/evidence/repro_soak_missing_rss.py`
   ruft `module.rss_bytes(0)`; die Funktion heißt seit Commit 1 `rss_messung`
   und liefert ein Objekt statt einer Zahl — der Beleg läuft gegen das
   reparierte Orakel so nicht mehr (gelesen, nicht gefahren; `docs/audits/`
   liegt außerhalb der Ticketgrenze).
7. **Kein Kanon, kein Laufzeit-Arm** — Dirigentensache nach dieser Etappe
   (§6.8); A34 ist ein neues Kanonbein. `docs/PLAN-STAND.md` nicht angefasst,
   `planstand.py` nicht gelaufen.

## 37. Messung der Etappe 6, Einordnung der Abweichungen und Start der Erstprüfung 6 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `f8b76182` (`nakama-nak283-7b1f38b-etappe6`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-6-auftrag.txt` (§35); Start 03:2x Uhr, Ende 05:0x Uhr (Laufzeit 1 h 44 min); Beobachter (Aufsicht NORMAL): Start, Telemetrie gesund, drei HEAD-Wechsel, Ende; Stundenloop `d1507ea5` gelöscht. Kein Halt, kein `needs input`. |
| Commits | `7d65adc7` (Werkzeuge, Test, Korpus über den Erzeuger, Runner), `4182cc0e` (§36, Riegel, Rot- und Beinskript, 24 Rohdateien, SONDE-013-Nachtrag), `32c09cb4` (Kopfzeile), alle auf `origin/master`; Basis `7b1f38b7` ist Vorfahr; `git status --short` zeigt nur die zwei fremden untracked Ordner (§27). |
| Diff | `git diff --stat 7b1f38b7..32c09cb4 -- eq-copilot tools`: sieben Dateien, +1 165/−77 (`pruefe_session_soak.py` +609, `gesundheit.py` +290, `Sonde013TruePeakGoldenTest.cpp` +311, `erzeuge_p4_korpus.py` +10, `referenz.json` +12, `MANIFEST.json` 6, `beweise.ps1` 4); dazu `docs/beweise/SONDE-013.md` +2/−2. Fremde Pfade gemessen leer: `eq-copilot/plugin/{src,core,dsp,sonde,state}`, `broker`, `schemas`, `identity`, `install`, `docs/offene-punkte.md`, `docs/PLAN-STAND.md`, `docs/plan`, `design`. `TruePeak.h` am Zielstand bytegleich zum Basis-SHA (nur im Rotbeweis mutiert, Hash in §36.2). |
| Rundenbilanz | `7b1f38b7..32c09cb4: Produkt 2 Datei(en) +15/-3 \| Tests 1 Datei(en) +311/-0 \| Pruefwerkzeug 4 Datei(en) +839/-74 \| Doku 31 Datei(en) +6218/-4` (die zwei „Produkt"-Dateien sind die Korpusfixtures unter `eq-copilot/fixtures/`, über den Erzeuger geschrieben); kumuliert `--runden bb49a612 988dc44d faea3c1a 3d6bbfb4 30a0b1bf 6f0d35d7 32c09cb4`: „kein Konvergenz-Signal". |
| Eigene Messung | Am Stand `32c09cb4` um 05:0x Uhr selbst gefahren: A34 `pruefe_session_soak.py --selbsttest` 4 von 4, Exit 0; A25 `--pruefen` bytegleich, Exit 0; A26 „alle vier Klassen bestehen", Exit 0; A27 Exit 0; A32 `--mit-selbsttest` Exit 4 mit genau den Werten aus §36.5 (Quelldateien über 2 000 Zeilen 1 von 0, Funktionen über 200 Zeilen 35 von 28, Kommentar-Bezeichner 36 von 30); B17 `EqCopSonde013TruePeakGoldenTest` 43 bestanden, 0 gescheitert, Exit 0 (Binary 04:31 Uhr, Quelle 04:03 Uhr), Fälle 20 bis 23 mit denselben Werten wie §36.3 (Fall 22 −0,16416 dBTP, Abstand −0,03864 dB). Rotbilanz gelesen: 20 von 20 BESTANDEN, je rot mit erwartetem Exit und Trägerzeile, grün Exit 0 (A32 grün Exit 4), Hash gleich; vier Beobachtungsläufe als solche gekennzeichnet. Zeilen der Tests und Riegel an HEAD gemessen: `pruefe_session_soak.py` `_fehlmessung :358`, `rss_messung :363`, `messung_fehlt :854`, `merkmal_vollstaendig :1146`, `gueltig_gemessen :1177`; `Sonde013TruePeakGoldenTest.cpp` Abschnitt 7 `:545`, Prüfzeilen `:617` und `:624`; `gesundheit.py` `INVENTARWURZELN :208`, `AUSGENOMMEN :212`, `TESTBAEUME :219`, `inventar :461`, `quellorte :494`, `inventarluecken :499`; `erzeuge_p4_korpus.py :101-103`; `beweise.ps1` A24 `:614`, A34 `:616`. A24 (148 s Soak) nicht selbst wiederholt; Rohdatei gelesen (zehn Speicherpunkte `rss_gueltig` true, S07 dreimal im Budget, `--mutant rss_fehlt` Exit 3). |
| Messabdeckung | Zehn Matrixzeilen (M-61 bis M-70) je mit Test und Rotbeweis; M-64 als Regressionswache benannt; M-63, M-65, M-66, M-68 gegen die Fixfassung rot (am Basis-SHA nicht messbar, §5.6), M-61, M-62, M-67, M-69, M-70 zusätzlich mit Basislauf rot am Basis-SHA. Keine Nebenläufigkeitszeile. Kein Fortsetzungsauftrag nötig. |
| Abweichungen §36.6 (Einordnung) | **Zehn gedeckt, zwei Lücken (eine mit Regel, eine als Härtung), kein Befund** — an Diff, Rotbilanz und Quelle gelesen. **Gedeckt:** 1 (M-65: die wörtliche Matrixmutation „8-fach auf 4-fach" lief und macht die Fälle 20 bis 23 nicht rot — gemessen auf fünf Stellen gleich, Beobachtung in der Bilanz; der Rotbeweis fällt mit MUT-65b, 12 statt 24 Taps je Phase, an der Zusagezeile `:617`: Fall 22 reißt ±0,1 dB, die Normtoleranz hält — genau die von M-65 zugesagte Reihenfolge. Die Matrixmutation ist Messvorschrift, keine Zusage; die Zusage ist gemessen. Nebenbefund für die Prüfung: die Erklärung — Scheitel der Einzelperiode auf dem 4·fs-Raster, Rasterfehler null für jeden 4-fach-Interpolator — ist an der Quelle zu prüfen (Prüffrage (b)); die 8-fach-Entscheidung aus NAK-180 bleibt von der Schranke `interpolationsfehlerDb` getragen, nicht von den EBU-Fällen), 3 (MESSUNG FEHLT mit Exit 3 — im Runner `[FEHLT]`, Kanon UNVOLLSTAENDIG, nie GRUEN; M-61 verlangt „von 0 verschiedener Exitcode" und „Messung fehlt" statt „im Budget", M-70 „eigener Status"; ein zugleich roter Punkt gewinnt mit Exit 2), 4 (`null` statt 0 für den gescheiterten Abruf, zusätzlich das deklarierte Szenario „0 mit ungültigem Merkmal", damit die Erfolgsbedingung einziger Träger ist), 6 (Gegenproben über die Matrix hinaus: Mutant `rss_fehlt` am echten Bericht, Ersatz-API der Messstelle, Tiefpassmessung, Gegenprobe der Referenz am 4·fs-Signal, Indexprüfung Versatz 3 — Mehrmessung), 7 (`eq-copilot/plugin/vertrag/generiert` ausgenommen wie `broker/src/generiert` — M-69 „Generiertes bleibt ausgeschlossen"; Inventarriegel als Werkzeugfehler Exit 2 wie der Ortsriegel, NAK-223; Fallzahl 121 → 129), 8 (Zahlen zu M-69 am Basis-SHA neu gemessen: 44 Dateien, 17 825 Zeilen, `NakamaState.cpp` 2 911), 9 (Datum des Wortlauts = Messtag), 10 (M-70 mit Basislauf am Basis-SHA der Etappe 1), 11 (Beine über §6.7 hinaus, Kürzel A34 = nächste freie Nummer hinter A33), 12 (Dateinamen). **Lücke mit Regel:** 5 (M-64, nächste Zeile). **Lücke als Härtung:** 2 (A26 liest `hinweis` nicht — die Rotbeweisspalte von M-66 nahm an, A26 messe „eine Behauptung, die kein Fall trägt"; gemessen misst es A26 nicht, `pruefe_p4_korpus.py` liegt außerhalb der Ticketgrenze; die Zusage M-66 fällt an A25 (Erzeuger gegen committeten Korpus) und am Etappenriegel M-66, der nicht im Kanon lebt → Registerpunkt im Abschluss der Etappe: A26 prüft `fall` und `hinweis` jedes `referenz`-Eintrags gegen die im Golden existierenden Abschnittsnamen). |
| Regel zu Abweichung 5 — M-64 präzisiert (13.09.2026) | M-64 sagt „ein Bericht ohne das neue Merkmal wird als Merkmal unbekannt behandelt und nicht pauschal rot", damit Altberichte auswertbar bleiben. Ein Livelauf schreibt seinen Bericht mit derselben Messstelle, die das Merkmal an jedem Punkt setzt; fehlt es dort oder trägt nur ein Teil der Punkte es, ist die Messstelle gebrochen — genau F13. **Regel:** „Merkmal unbekannt" gilt allein für gespeicherte Berichte über `--bericht` und nur, wenn kein Punkt das Merkmal trägt; im Livelauf und in einem gemischten Bericht ist das fehlende Merkmal ein Pflichtfeldfehler (ROT, Exit 2). Beide Gegenteile laufen im Selbsttestfall M-64 mit; der Kontrollbericht `NAK-246-4ff6f24.md` bleibt GRUEN mit Hinweis (§36.5). |
| §36.9 offene Punkte | 1 (Runner-Behauptungen B17 und A32 nennen den neuen Umfang nicht): Ticketabschluss, Kopfzeile „Ticketpfade" ergänzt. 2 (Register): nach PASS der Erstprüfung 6 im Abschluss der Etappe — NAK-199-Nachtrag mit dem Wortlaut §36.7 (der Riegel M-68 am Registerstand ist bis dahin rot), NAK-249-Nachtrag „durch M-69 gebaut", Pflegeticket für die ehrlich gerissenen Werte (eine Datei über 2 000 Zeilen, Funktionen über 200 Zeilen 35/28, Kommentar-Bezeichner 36/30; Muster S25b–d, Ergänzung zu NAK-255). 3 (EBU-Synthese ohne Abgleich mit den offiziellen Testdateien der Fälle 20 bis 23): **Härtung/Messung**, Registerpunkt im Abschluss der Etappe. 4 (A26 und `hinweis`): Härtung, Registerpunkt (oben). 5 (vorbestehend: `TruePeak.h:33` nennt einen Fallnamen, den es nicht gibt; „§" in Prüftexten von B17/B18/B27 als „Â§"): Registernachtrag NAK-255 beziehungsweise Härtung, keine Etappenarbeit. 6 (Audit-Beleg `repro_soak_missing_rss.py` ruft `rss_bytes(0)`, das es seit `7d65adc7` nicht mehr gibt): `docs/audits/` ist Verlauf, kein Ticketpfad; Hinweis im Register bei NAK-283. 7 (Kanon, Laufzeit-Arm): nach der letzten Etappe (§6.8). |
| Hygiene des Fensters (05:1x Uhr) | Kontextfläche unverändert gegen §35 (MEMORY.md 5 338 B, CLAUDE.md 19 434 B, Skill 24 465 B); `gesundheit.py` Exit 4 mit **drei** Maßen — das dritte (eine Quelldatei über 2 000 Zeilen) und die Sprünge 29 → 35 und 32 → 36 sind das ehrlich gewordene Maß aus M-69, kein neuer Codebase-Riss dieser Etappe (Produktcode unverändert); das Pflegeticket folgt im Abschluss der Etappe (Registerpunkt, nie stilles Nachbessern). A33 (`tidy.py`) am Stand `270167fb` gehalten; diese Etappe ändert keine Übersetzungseinheit des Plugins außer der Testdatei (kein Ratschenpfad). |
| Erstprüfung 6 | Vorlage A: Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`codex-lauf.ps1`, Kennung `nak283-etappe6-erst`); Auftrag `docs/beweise/roh/NAK-283-etappe-6-erstpruefung-auftrag.txt`: Prüfbereich `git diff 7b1f38b7...32c09cb4 -- <die sieben Ticketpfade>`, Gate wörtlich, Matrix §5.5, Riegelzeilen gemessen, Rotbeweise und Beine, die zwölf Abweichungen mit dieser Einordnung, die Regel zu M-64, Ausschlüsse (`docs/**`, NAK-199, NAK-249, NAK-255, NAK-276, NAK-287 bis NAK-291, Etappen 2 bis 5, §36.9 Punkte 1 und 3 bis 6), Prüffragen (a) bis (e). HEAD beim Start = der Commit dieses Abschnitts; Thread-ID und Urteil in §38. |

## 38. Erstprüfung 6 — PASS; Abschluss der Etappe 6; Registernachträge; Zwischenkanon (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Erstprüfung 6 | Vorlage A: Codex `gpt-6-astra`, Effort **max**, lesend, Sandbox read-only, abgekoppelt (`tools/dirigent/codex-lauf.ps1`, Kennung `nak283-etappe6-erst`); Auftrag `docs/beweise/roh/NAK-283-etappe-6-erstpruefung-auftrag.txt` (§37); Prüfbereich `git diff 7b1f38b7...32c09cb4 -- <die sieben Ticketpfade>`; Start 05:15:34 Uhr, Ende 05:28:01 Uhr; HEAD `f74e7d0b` vor und nach dem Lauf identisch, Arbeitsbaum unverändert. Thread `01a098c3-5b5e-7920-8240-07f7a349aabb`; 60 lesende Kommandos im JSONL (1,0 MB), keine Kapazitäts- oder API-Fehler. Rohdatei `docs/beweise/roh/NAK-283-erstpruefung-6-f74e7d0.txt`. |
| Urteil | **PASS** — Codex wörtlich: „kein DEFEKT im gebundenen Prüfbereich." Geprüft: „vollständiger Sieben-Dateien-Diff, M-61 bis M-70 anhand Implementierung, Tests, Riegeln und Rotbelegen, einschließlich Verbraucher, Probe-Pipe und Änderungsgrenzen; Hashes und LF/-text selbst lesend überprüft". Nicht geprüft: „keine eigenen Build-, Python-, Produkt- oder Testläufe; Laufresultate einschließlich A32 Exit 4 ausschließlich aus vorhandenen Rohbelegen ausgewertet". Keine Lücke und keine Härtung benannt; die Prüffrage (b) zur Abweichung 1 (4-fach-Mutation ohne Wirkung auf die Fälle 20 bis 23) endet ohne Befund. |
| Einordnung | Kein Befund zu validieren, keine Nacharbeit. Die Regel zu M-64 (§37) gilt für die Abschlussprüfung. |
| Rundenbilanz | Erstprüfung: keine Codeänderung (`32c09cb4..f74e7d0b`: nur Doku — §37, Register, Prüfauftrag). Kumuliert unverändert gegen §37. |
| Etappe 6 | **Abgeschlossen auf `32c09cb4`** (Werkzeuge, Test, Korpus und Runner `7d65adc7`): Erstprüfung 6 PASS in der ersten Runde, null Nacharbeitsrunden. Stand des Tickets: Etappen 1, 2, 3, 4 und 6 abgeschlossen (§19, §28, §32, §35, §38); offen Etappe 5 (nach den Antworten U41/U42, One-Shot 09:04 Uhr), danach Abschlussprüfung über `39e18d3f...HEAD`, voller Kanon, Laufzeit-Arm, Nachzug der Runner-Behauptungen A16, B7, B6, A3, B17, A32 (Kopfzeile „Ticketpfade"). |
| Register (13.09.2026, 05:4x Uhr) | **NAK-199:** Nachtrag mit dem Wortlaut aus §36.7 (True Peak jetzt über 15 bis 23 gedeckt; LUFS-I bleibt offen) — damit ist der Riegel M-68 am Registerstand grün (`NAK-283-etappe-6-riegel.py M-68 --registerstand`, nach dem Commit gemessen). **NAK-249:** geschlossen durch M-69 (Quellumfang aus dem Inventar, Inventarriegel, Selbsttestfall; §36.1). **NAK-255:** Nachtrag — durch M-69 ehrlich gewordene Werte (eine Quelldatei über 2 000 Zeilen, Funktionen über 200 Zeilen 35/28, Kommentar-Bezeichner 36/30) und die neuen Fundstellen (§36.5), dazu `TruePeak.h:33` (§36.9 Punkt 5). **NAK-292 neu [Planarbeit · Pflegeschritt]:** Pflegeticket für die durch das Inventar sichtbar gewordenen Risse (Muster S25b–d; `NakamaState.cpp` 2 911 Zeilen, sechs weitere Funktionen über 200 Zeilen, vier weitere Kommentar-Bezeichner; nie stilles Nachbessern). **NAK-293 neu [Härtung · Messung · P4]:** Abgleich der synthetisierten EBU-Fälle 20 bis 23 mit den offiziellen EBU-Testdateien (§36.9 Punkt 3; die Norm legt Tiefpass und Einsatzstelle nicht fest). **NAK-294 neu [Härtung · Prüfwerkzeug]:** A26 prüft `fall` und `hinweis` jedes `referenz`-Eintrags gegen die im Golden existierenden Abschnittsnamen (§36.6 Abweichung 2, §36.9 Punkt 4); dazu die byteweise gelesenen UTF-8-Prüftexte („Â§") in B17/B18/B27 (§36.9 Punkt 5). **NAK-283:** Nachtrag (Erstprüfung 6 PASS, Etappe 6 abgeschlossen, Registerpunkte, Zwischenkanon). Der Audit-Beleg `repro_soak_missing_rss.py` (§36.9 Punkt 6) steht als Hinweis bei NAK-283; `docs/audits/` ist Verlauf. |
| Hygiene des Fensters (05:4x Uhr) | MEMORY.md 5 338 B, CLAUDE.md 19 434 B, Skill 24 465 B — unverändert gegen §35; keine Indexzeile über 250 Zeichen, keine Memory-Datei ohne Indexlink; `dokuriegel.py` auf Manifest, Register, CLAUDE.md und Skill ohne Befund (nach dem Commit gemessen). `gesundheit.py` Exit 4 (drei Maße, §37) — jetzt mit Pflegeticket NAK-292, kein neuer Riss. A33 am Stand `270167fb` gehalten; seit dem Rebase keine Übersetzungseinheit außer Testdateien geändert. |
| Zwischenkanon | Der jüngste volle Kanon (`224d18d1` GRÜN 65/65, NAK-288 Lauf 3, 00:37 Uhr) liegt **vor** den Etappen 4 und 6. Bis zum Zeitfenster der Produktfragen (09:04 Uhr) läuft deshalb ein voller abgekoppelter Kanon auf dem Stand dieses Abschlusses als Zwischenmessung (`tools/beweise.ps1 -Bauen -Ziel docs/beweise/NAK-283.md -Anhaengen -Titel NAK-283`; Rohausgabe `docs/beweise/roh/NAK-283-<sha>.md`, Kopfzeile in §38.1 nach dem Lauf). Er ersetzt nicht den Kanon am Ticketende (§6.8), er misst A34 erstmals im Kanon (66 Beine erwartet: 65 aus NAK-288 plus A34) und macht einen Riss aus Etappe 4 oder 6 vor Etappe 5 sichtbar. Während des Laufs kommt nichts in den Worktree; Etappe 5 startet erst nach dem Kanon und den Antworten. |

---

### 38.1 Zwischenkanon auf dem Abschlussstand der Etappe 6 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Lauf | `tools/beweise.ps1 -Bauen -Ziel docs/beweise/NAK-283.md -Anhaengen -Titel NAK-283`, abgekoppelt, 05:34:15 bis 06:0x Uhr, auf `e6771fc9` (HEAD = `origin/master`); während des Laufs nichts im Worktree; Beobachter auf dem Log bis `EXIT=`, Stundenloop `a8e9a2c4` gelöscht. Rohausgabe `docs/beweise/roh/NAK-283-e6771fc-dirty.md`; der Runner-Anhang „Kanon-Lauf - NAK-283" folgt unten. |
| Urteil | **GRÜN — 66/66 Kanon-Läufe bestanden, 1 stillgelegt (A15), Exit 0.** 42 Bauziele; A34 (`pruefe_session_soak.py --selbsttest`, Etappe 6) erstmals im Kanon — 66 Beine gegen 65 im jüngsten beglaubigten Lauf `224d18d1` (NAK-288 Lauf 3); A32 `[HINWEIS]` Exit 4, nicht blockierend (drei Maße, Pflegeticket NAK-292); A33 Exit 0 (Ratsche gehalten); A24 GRÜN mit Gültigkeitsmerkmal. |
| Arbeitsbaum | „2 unbestätigte Dateien — dieser Lauf beweist NICHT allein den Commit": die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` (§27), kein Ticketpfad, nie angefasst; alle gemessenen Quellen sind committet (`git status --short` zeigt nur diese zwei Einträge). Der Rohdateiname trägt deshalb das Suffix `-dirty`. |
| Reichweite | Erster voller Kanon, der die Etappen 4 (`fe6ee58f`) und 6 (`7d65adc7`) enthält; der jüngste vorherige (`224d18d1`, 00:37 Uhr) lag davor. Kein Riss aus den Etappen 4 und 6. Kein Ersatz für den Kanon am Ticketende (§6.8) und nicht die Abschlussmessung. |
| Laufzeit-Arm | Nicht gefahren: der Laufzeit-Arm gehört zum Ticketabschluss (Skill §3.3, §3.5), und sein erster Lauf mit Installation ist Teil von S25e / NAK-286 (`tools/fl/LIES-MICH.md`, „noch nicht im Lauf gemessen") — er folgt nach dem Kanon am Ticketende. |

---

### 38.2 Runner-Behauptungen nachgezogen (Worker, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Auftrag | Enger Einzelauftrag des Dirigenten vom 13.09.2026 (Commit `f2827441`): die Behauptungstexte von A16, B7, B6 und A3 (§33.7 Punkt 1, §34) sowie B17 und A32 (§36.9 Punkt 1, §37) in `tools/beweise.ps1` nennen den gemessenen Umfang der Etappen 4 und 6. Grenze: nur die sechs Zeichenketten `Behauptung`, dieser Abschnitt, die Kopfzeile „Ticketpfade“ und die Rohausgabe. Kein Kanon, kein Laufzeit-Arm, kein Produktcode; `docs/PLAN-STAND.md` und `docs/offene-punkte.md` nicht angefasst. |
| Basis-SHA | `f2827441f8ae23bcb7f8c7ff4f8dc406e7811ad6` (= `git rev-parse HEAD` beim Start); `git status --short` beim Start zeigt nur die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` (§27) — nicht angefasst, nicht committet. |
| Geändert | `tools/beweise.ps1`, sechs Zeilen (`git diff --numstat` 6/6): 487 (A3), 659 (A16), 694 (B6), 699 (B7), 730 (B17), 751 (A32), je nur die Zeichenkette `Behauptung`. BOM und CRLF wie vorher (`i/lf w/crlf`). Keine Runner-Logik, kein Argument, kein neues Bein, keine Umbenennung. |
| Parser und AST | `[System.Management.Automation.Language.Parser]::ParseFile` auf den Arbeitsbaum: **0 Parse-Fehler**. Beintabelle `$kanon` (Zuweisung Zeile 464) gegen `f2827441:tools/beweise.ps1`: 67 gegen 67 Einträge, keine doppelten Kürzel; die Behauptung ist genau bei A3, A16, B6, B7, B17 und A32 geändert, bei 61 Einträgen gleich; Kürzel, Name, Art, Argumente, AbPhase und alle weiteren Schlüssel aller 67 Einträge sind im Quelltext je Wert gleich. Zeilenvergleich 1 442 gegen 1 442 Zeilen: verschieden sind genau die sechs Behauptungszeilen. Keine neue Nicht-ASCII-Stelle, kein Backtick (A32 verliert seine zwei), kein Umbruch, kein Apostroph. |
| Beine | Direkt gefahren, weil der Runner keine Einzelbeinform kennt; Aufruf aus dem Workspace-Root wie im Runner, 13.09.2026 ab 06:52:47 Uhr. Die fünf Binaries sind bytegleich die des Zwischenkanons (SHA-256-Präfix je Binary gleich seinem Baustand in `docs/beweise/roh/NAK-283-e6771fc-dirty.md`, dort „frisch (Bau bestaetigt)“; gebaut 05:34 bis 05:36 Uhr, B17 schon 04:31:24 Uhr) und jünger als die jüngste Quelle unter `eq-copilot/plugin` (04:31:20 Uhr, `TruePeak.h`); seit `e6771fc9` kein Commit unter `eq-copilot`, `broker` oder `tools`, kein fremder Bau- oder Testprozess (NAK-290), kein Neubau. **A16 Exit 0** (65 Prüfungen ok, 0 Fehler), **B7 Exit 0** (174 geprüft, 0 Fehler), **B6 Exit 0** (259 geprüft, 0 Fehler), **A3 Exit 0** (MARKIERUNGSTEST OK), **B17 Exit 0** (43 bestanden, 0 gescheitert), **A32 Exit 4 wie erwartet** (Selbsttest 129/129; gerissen: Quelldateien über 2 000 Zeilen 1 von 0, Funktionen über 200 Zeilen 35 von 28, Kommentar-Bezeichner ohne Code 36 von 30 — NAK-292, nicht blockierend). |
| Rohausgabe | `docs/beweise/roh/NAK-283-runner-behauptungen-beine.txt`: Parser- und AST-Vergleich, alte und neue Behauptungen wörtlich, Frische, je Bein Befehl, Exit, Trägerzeilen und letzte Zeilen, im Nachtrag `dokuriegel.py` auf dieses Manifest, der Hashvergleich der fünf Binaries mit dem Baustand des Zwischenkanons und der Zitatabgleich von §38.2.1, am Ende das Prüfskript wörtlich (gelaufen aus dem Job-Ordner der Session; nur dort liegt es als Datei). |
| Abweichungen | Sechs, §38.2.3. |

#### 38.2.1 Alte und neue Behauptungen, wörtlich

Alt = `f2827441:tools/beweise.ps1`, neu = Stand dieses Commits; die Zeilennummer ist in beiden Fassungen dieselbe. Der Zitatabgleich der Rohausgabe (Abschnitt 6) vergleicht jeden Block zeichengleich mit dem Parserwert.

**A16** (`tools/beweise.ps1:659`) — alt:

```text
Nakama Probeeq (NkAc) mit dem aktiven DSP-Kern: im Default (eq_enabled aus) bitgleich ueber 1000 Bloecke von 1 bis 2048 Samples bei 44,1 / 48 / 96 / 192 kHz ohne belegte Bank; eq_enabled an mit bypass aus und sonst neutralen Defaults bitgleich; eq_enabled an mit bypass an bitgleich trotz hoerbarem Band dahinter; nach eq_enabled aus nach dem Fade wieder bitgleich; Mix 0 mit Output-Trim 0 dB bitgleich; der Passthrough sanitisiert kein NaN/Inf; 0 Samples Latenz und kein Tail im Passthrough und im Hard-Bypass; speichern-laden-speichern bytegleich im Layout v2 mit Kind Dsp. Das Bundle meldet 112 Host-Parameter, und sein Bundlevertrag laesst nur active_probe zu. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2.
```

neu:

```text
Nakama Probeeq (NkAc) mit dem aktiven DSP-Kern: im Default (eq_enabled aus) bitgleich ueber 1000 Bloecke von 1 bis 2048 Samples bei 44,1 / 48 / 96 / 192 kHz ohne belegte Bank; eq_enabled an mit bypass aus und sonst neutralen Defaults bitgleich; eq_enabled an mit bypass an bitgleich trotz hoerbarem Band dahinter; nach eq_enabled aus nach dem Fade wieder bitgleich; Mix 0 mit Output-Trim 0 dB bitgleich; der Passthrough sanitisiert kein NaN/Inf; 0 Samples Latenz und kein Tail im Passthrough und im Hard-Bypass; speichern-laden-speichern bytegleich im Layout v2 mit Kind Dsp. Seit NAK-283 (F04, F05) bleibt der Nulltest bitgleich, wenn reset() vor jedem dritten Block laeuft - ausgeschaltet bei 44,1 / 48 / 96 / 192 kHz, ohne dass die Zaehler fuer nicht endliche Eingaenge, geheilte Filterzustaende und verworfene Analyseframes steigen, und im Hard-Bypass auch mit NaN, +Inf und -Inf zwischen zwei reset(); Mono und Stereo mit gleichem Ein- und Ausgang werden angenommen (im Monobus bleibt der Passthrough bitgleich, und das Band rechnet endlich), Quadrophonie, 5.1, vier diskrete Kanaele, Mono->Stereo, Stereo->Mono und jeder deaktivierte Hauptbus bekommen ein Nein, setBusesLayout auf Quadrophonie scheitert, und der Prozessor bleibt bei zwei Kanaelen. Das Bundle meldet 112 Host-Parameter, und sein Bundlevertrag laesst nur active_probe zu. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2.
```

**B7** (`tools/beweise.ps1:699`) — alt:

```text
Lokaler Transaktionskern der aktiven Sonde mit dem echten DSP-Kern: die Falltabelle T1-T17 aus Manifest SONDE-015 §5.11.4 laeuft tabellengetrieben mit den Invarianten I1, I2 und I4 als Wachen nach jeder Eingabe; ein Fehler in jeder Stufe S1-S7 laesst bestaetigten Zustand, Hash, Undo-Ring und Register unveraendert, und weder der Nachschlag S0 noch der Commit-Punkt allozieren; Apply, Revert, Neutralisieren, Remove, Undo, Redo und Preset-Laden erzeugen je genau eine Revision, 10.000 doppelte, vertauschte und veraltete Eingaben einer Transaktion hoechstens eine. Belegung, Remove und Undo als ein Objekt, Schutz-Zonen mit Verletzungsmeldung, Undo-Ring der Tiefe 32 und Preset halten ihre Matrixzeilen. Am echten SondeProcessor: 112 Host-Parameter in Vertragsreihenfolge ohne occupied; Hostautomation ohne Revision mit zwei Epochenwechseln je Geste, auf einem freien Slot bitgleich im Klang; ein abgeschlossener Gestus ist eine Revision; Host-Dirty nur bei einem Commit; das Kind Dsp reist durch Speichern und Laden, und ein Reload rekonstruiert Hash und Ausgang.
```

neu:

```text
Lokaler Transaktionskern der aktiven Sonde mit dem echten DSP-Kern: die Falltabelle T1-T17 aus Manifest SONDE-015 §5.11.4 laeuft tabellengetrieben mit den Invarianten I1, I2 und I4 als Wachen nach jeder Eingabe; ein Fehler in jeder Stufe S1-S7 laesst bestaetigten Zustand, Hash, Undo-Ring und Register unveraendert, und weder der Nachschlag S0 noch der Commit-Punkt allozieren; Apply, Revert, Neutralisieren, Remove, Undo, Redo und Preset-Laden erzeugen je genau eine Revision, 10.000 doppelte, vertauschte und veraltete Eingaben einer Transaktion hoechstens eine. Belegung, Remove und Undo als ein Objekt, Schutz-Zonen mit Verletzungsmeldung, Undo-Ring der Tiefe 32 und Preset halten ihre Matrixzeilen. Am echten SondeProcessor: 112 Host-Parameter in Vertragsreihenfolge ohne occupied; Hostautomation ohne Revision mit zwei Epochenwechseln je Geste, auf einem freien Slot bitgleich im Klang; ein abgeschlossener Gestus ist eine Revision; Host-Dirty nur bei einem Commit; das Kind Dsp reist durch Speichern und Laden, und ein Reload rekonstruiert Hash und Ausgang. Seit NAK-283 (F05, F09, F12) am echten SondeProcessor: reset() beendet die Audiohistorie - nach einem Impuls in den Resonator (1 kHz, +12 dB, Q 10) ist der erste Block Stille auf beiden Kanaelen exakt 0, ohne reset() klingt er nach - und laesst getStateInformation bytegleich, die Revision, das bestaetigte eq_enabled und die engagierte Bank im Kern stehen (1 kHz danach mehr als 6 dB lauter); releaseResources->prepareToPlay, reset() und prepareToPlay allein enden in derselben Audiohistorie (Stille exakt 0, untereinander bitgleich). Ein Block ueber maxBlock bei rechnendem Kern hinterlaesst in der Analysequeue eine Luecke (der naechste Block beginnt bei 192 mit kFlagLueckeDavor, Segment +1), und Kern und Queue beschreiben dieselbe verworfene Zeitspanne (ein verworfener Tap, ein Block ohne Audio mit 128 Frames, oversize und Ueberlauf +0); ruht der Committed-Pfad im Hard-Bypass, wird derselbe Block ohne Luecke angenommen. Endliche double ueber FLT_MAX am Tap kommen in der Analysekopie als endliche floats an, und der Kernzaehler steigt um genau diese 1024 Werte.
```

**B6** (`tools/beweise.ps1:694`) — alt:

```text
Aktiver DSP-Kern (plugin/dsp/) als Bibliothek: alle sechs RBJ-Filtertypen liegen ueber einem 1/24-Oktav-Gitter von 20 Hz bis min(20 kHz, 0,45 fs) bei 44,1 / 48 / 96 / 192 kHz innerhalb 0,05 dB typisch und 0,1 dB an den Raendern an der analytischen Antwort - gemessen als Impulsantwort AM TAP post_committed, also durch den Audiopfad, gegen eine im Test eigenstaendig ausgeschriebene Formel. Ausgeschaltet und hard-bypassed wird bei jeder Hoermatrix-Auswahl kein Sample geschrieben (bitgleich); bei Mix 0 mit Output-Trim 0 dB ist der Ausgang wertgleich zum Eingang; nach dem Ausschaltfade ist der Passthrough wieder bitgleich, und er sanitisiert kein NaN. Die acht Slots wirken als Kaskade von Slot 0 nach 7 gegen ein unabhaengig gerechnetes geordnetes Golden, das ein Rueckwaertslauf reisst. Auto-Gain ergibt bei flacher Kurve exakt 0 dB und bei einem +6-dB-Shelf ueber dem Gitter -6 dB, ein identisches Mid/Side-Paar faellt exakt auf den Stereo-Fall zusammen, und die Dynamik bewegt ihn nicht. Der Bankpool faehrt jeden der sechs Ownership-Uebergaenge einzeln, gibt busy_retry statt eine aktive Bank zu verdraengen, wird erst nach dem Audio-ACK frei und haelt seine Invarianten unter zwei echten Threads ueber tausende Bloecke; im Callback zaehlt der thread-lokale Zaehler ueber 4000 Bloecke wechselnder Groesse 0 Allokationen und 0 Sperren, waehrend derselbe Zaehler die Allokationen des Workers NICHT sieht.
```

neu:

```text
Aktiver DSP-Kern (plugin/dsp/) als Bibliothek: alle sechs RBJ-Filtertypen liegen ueber einem 1/24-Oktav-Gitter von 20 Hz bis min(20 kHz, 0,45 fs) bei 44,1 / 48 / 96 / 192 kHz innerhalb 0,05 dB typisch und 0,1 dB an den Raendern an der analytischen Antwort - gemessen als Impulsantwort AM TAP post_committed, also durch den Audiopfad, gegen eine im Test eigenstaendig ausgeschriebene Formel. Ausgeschaltet und hard-bypassed wird bei jeder Hoermatrix-Auswahl kein Sample geschrieben (bitgleich); bei Mix 0 mit Output-Trim 0 dB ist der Ausgang wertgleich zum Eingang; nach dem Ausschaltfade ist der Passthrough wieder bitgleich, und er sanitisiert kein NaN. Die acht Slots wirken als Kaskade von Slot 0 nach 7 gegen ein unabhaengig gerechnetes geordnetes Golden, das ein Rueckwaertslauf reisst. Auto-Gain ergibt bei flacher Kurve exakt 0 dB und bei einem +6-dB-Shelf ueber dem Gitter -6 dB, ein identisches Mid/Side-Paar faellt exakt auf den Stereo-Fall zusammen, und die Dynamik bewegt ihn nicht. Der Bankpool faehrt jeden der sechs Ownership-Uebergaenge einzeln, gibt busy_retry statt eine aktive Bank zu verdraengen, wird erst nach dem Audio-ACK frei und haelt seine Invarianten unter zwei echten Threads ueber tausende Bloecke; im Callback zaehlt der thread-lokale Zaehler ueber 4000 Bloecke wechselnder Groesse 0 Allokationen und 0 Sperren, waehrend derselbe Zaehler die Allokationen des Workers NICHT sieht. Ein Block ueber maxBlock (1024 Samples bei 256) verwirft den Analyse-Tap und gleicht auf beiden Kanaelen sample-exakt dem Lauf ohne Ueberlast in 4 x 256. Seit NAK-283 (F12) die float-Kante des Ausgangs: ein endlicher double ueber FLT_MAX (Output-Trim +6 dB auf 0,75 und 0,25 x FLT_MAX) kommt auf 0 verriegelt und im Eingangszaehler gezaehlt heraus, und kein Ausgangswert ist nicht endlich; auf endlichem Material in +/-1,0 (Bell +9 dB und Output-Trim +6 dB, 204 800 Werte, auch mit Betrag >= 1) ist jeder float bitgleich die Verengung des double-Taps, und der Zaehler bleibt stehen.
```

**A3** (`tools/beweise.ps1:487`) — alt:

```text
Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport).
```

neu:

```text
Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). Seit NAK-283 (F12) erzeugt ein endlicher Eingang keinen nicht endlichen Wet-Ausgang: im Puls auf 1 kHz mit einem Ton der Amplitude 0,8 x FLT_MAX ist jedes Ausgangssample endlich, und nach dem Einblenden steigt der Wet-Zaehler um genau die verriegelten Samples.
```

**B17** (`tools/beweise.ps1:730`) — alt:

```text
True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten.
```

neu:

```text
True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht fuer 15 bis 19 keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt auf 1e-9 genau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. Seit NAK-283 (N01) zusaetzlich die vier transienten Faelle 20 bis 23 aus EBU Tech 3341 (Versatz 0 bis 3): jeder trifft eine im Test gerechnete, vorher an 15 bis 19 auf +/-0,01 dB validierte 64-fach-Referenz innerhalb +/-0,1 dB und liegt zusaetzlich in der Normtoleranz 0,0 +0,2/-0,4 dBTP. Insgesamt 43 Pruefungen.
```

**A32** (`tools/beweise.ps1:751`) — alt:

```text
Codebase- und Kontextgesundheit gegen die Schwellen aus Plan S25b und dem Kontext-Hygiene-Playbook: Zeilen je Quelldatei (Grenze 2 000, Ziel 1 500, `broker/src/generiert/` ausgenommen), Anzahl Funktionen ueber 200 Zeilen, aufruferlose allow(dead_code)-Helfer, Backtick-Bezeichner in Kommentaren ohne Entsprechung im Repo, sowie Bytes von MEMORY.md, CLAUDE.md und Dirigenten-Skill, Index-Zeilen ueber 250 Zeichen und Memory-Dateien ohne Index-Link. Die Ratschen (Funktionen 28, Kommentar-Bezeichner 30, clippy 91) stehen auf dem Iststand vom 09.09.2026 nach NAK-225 und reissen bei der ersten Verschlechterung; sie werden nur gesenkt, nie erhoeht. Die Zeilengrenze HAELT seit NAK-225 (S25d): keine Quelldatei liegt mehr ueber 2 000 Zeilen, und die Pflegeticket-Zuordnung ist leer. Jeder neue Treffer meldet sich als OHNE PFLEGETICKET; ein Zuordnungseintrag, dessen Datei die Grenze nicht mehr reisst, beendet den Lauf als WERKZEUGFEHLER (Exit 2), damit die Liste nicht still veraltet. Vorangestellt laeuft der Selbsttest: 121 Faelle im Speicher durch dieselben Funktionen, jede Erwartung mit ihrem Gegenteil (Zahlenraender 2 000/2 001, 200/201, 250/251, CRLF, BOM, fehlender Schlussumbruch, Rust-Lebensdauer und C++-Ziffernstrenner gegen die Literalmaskierung). Der clippy-Teil laeuft NICHT mit (Schalter --clippy, er baut) und meldet sich als nicht messbar. Nicht blockierend: Exit 4 ist ein Hinweis. Exit 2 ist ROT und heisst, dass das WERKZEUG nicht mehr misst - roter Selbsttest oder ein fehlender Messort, denn ein verschobener Pfad wuerde sonst still 0 messen und die halbe Codebase unbemerkt aus dem Mass nehmen.
```

neu:

```text
Codebase- und Kontextgesundheit gegen die Schwellen aus Plan S25b und dem Kontext-Hygiene-Playbook. Der Quellumfang kommt seit NAK-283 (M-69) aus einem pruefbaren Inventar: jede Datei mit Endung .rs, .cpp, .h oder .hpp unter broker/src und eq-copilot/plugin samt allen Unterbaeumen, mit Grund ausgenommen nur der flatc-Codegen (broker/src/generiert, eq-copilot/plugin/vertrag/generiert) und der Testbaum eq-copilot/plugin/tests; der Bericht druckt das Inventar, und eine Quelldatei, die weder gemessen noch ausgenommen ist, beendet den Lauf als WERKZEUGFEHLER (Inventarriegel, Exit 2). Gemessen werden Zeilen je Quelldatei (Grenze 2 000, Ziel 1 500), Anzahl Funktionen ueber 200 Zeilen, aufruferlose allow(dead_code)-Helfer, Backtick-Bezeichner in Kommentaren ohne Entsprechung im Repo, sowie Bytes von MEMORY.md, CLAUDE.md und Dirigenten-Skill, Index-Zeilen ueber 250 Zeichen und Memory-Dateien ohne Index-Link. Die Ratschen (Funktionen 28, Kommentar-Bezeichner 30, clippy 91) stehen auf dem Iststand vom 09.09.2026 nach NAK-225 und reissen bei der ersten Verschlechterung; sie werden nur gesenkt, nie erhoeht. Die Grenzen - Zeilen je Datei, Funktionen ueber 200 Zeilen, Kommentar-Bezeichner - werden ehrlich gemeldet, auch wenn sie reissen (Pflegeticket NAK-292). Jeder neue Treffer ueber der Zeilengrenze meldet sich als OHNE PFLEGETICKET; ein Zuordnungseintrag, dessen Datei die Grenze nicht mehr reisst, beendet den Lauf als WERKZEUGFEHLER (Exit 2), damit die Liste nicht still veraltet. Vorangestellt laeuft der Selbsttest: 129 Faelle im Speicher durch dieselben Funktionen, jede Erwartung mit ihrem Gegenteil (Zahlenraender 2 000/2 001, 200/201, 250/251, CRLF, BOM, fehlender Schlussumbruch, Rust-Lebensdauer und C++-Ziffernstrenner gegen die Literalmaskierung; das Inventar mit neuem Unterbaum, Codegen, Tests und einer Ableitung, die still einen Unterbaum verliert). Der clippy-Teil laeuft NICHT mit (Schalter --clippy, er baut) und meldet sich als nicht messbar. Nicht blockierend: Exit 4 heisst gerissen und ist ein Hinweis. Exit 2 ist ROT und heisst, dass das WERKZEUG nicht mehr misst - roter Selbsttest, eine fehlende Inventarwurzel oder der Inventarriegel, denn ein verschobener Pfad wuerde sonst still 0 messen und die halbe Codebase unbemerkt aus dem Mass nehmen, und ein ungemessener Unterbaum machte jedes andere Mass falsch.
```

#### 38.2.2 Je Behauptungssatz die Prüfzeile

Nur die neuen und die geänderten Sätze; der übrige Text steht unverändert und wird hier nicht neu geprüft. Zeilen zum Basis-SHA (die Testdateien sind seit `e6771fc9` unverändert), Messwerte aus den Beinläufen dieses Abschnitts.

**A16** — `eq-copilot/plugin/tests/SondeNullTestMain.cpp`

| Satz | Prüfzeile (Datei:Zeile, Fall) | gemessen |
|---|---|---|
| Nulltest bitgleich mit reset() vor jedem dritten Block, ausgeschaltet bei 44,1 / 48 / 96 / 192 kHz; die Zähler für nicht endliche Eingänge, geheilte Filterzustände und verworfene Analyseframes steigen nicht | `SondeNullTestMain.cpp:247-249` `nulltest_bleibt_bitidentisch_mit_host_reset (M-34)`; Lauf `:245` mit `resetAlle` 3, Reset `:160-161` | je Rate 460 000 Samples, 0 abweichend, Zähler 0/0/0 |
| im Hard-Bypass auch mit NaN, +Inf und -Inf zwischen zwei reset() | `:281-283` (zweiter Fall in Abschnitt 3b, Reset `:273` und `:275`) | 460 000 Samples, 0 abweichend, NaN/Inf roh |
| Mono und Stereo mit gleichem Ein- und Ausgang angenommen; im Monobus Passthrough bitgleich, Band rechnet endlich | `:859-861` `mono_und_stereo_bleiben_angenommen (M-28)` | Stereo ja, Mono ja, gesetzt, bitgleich, Band rechnet |
| Quadrophonie, 5.1, vier diskrete Kanäle: Nein; setBusesLayout auf Quadrophonie scheitert; zwei Kanäle bleiben | `:822-824` `mehrkanallayout_wird_abgelehnt (M-27)` | alle drei nein, gesetzt nein, Kanäle 2/2 |
| Mono->Stereo, Stereo->Mono und jeder deaktivierte Hauptbus: Nein | `:873-874` `ungleiche_und_deaktivierte_layouts_bleiben_abgelehnt (M-29)` | fünf Layouts nein |

**B7** — `eq-copilot/plugin/tests/TransactionTestMain.cpp`

| Satz | Prüfzeile (Datei:Zeile, Fall) | gemessen |
|---|---|---|
| reset() beendet die Audiohistorie: nach dem Impuls in den Resonator (1 kHz, +12 dB, Q 10) ist der erste Block Stille auf beiden Kanälen exakt 0, ohne reset() klingt er nach | `TransactionTestMain.cpp:2347-2349` `host_reset_beendet_die_audiohistorie (M-30)` | Peak nach reset 0, ohne reset 0,0192088876 |
| reset() lässt getStateInformation bytegleich, die Revision, das bestätigte eq_enabled und die engagierte Bank im Kern stehen (1 kHz danach mehr als 6 dB lauter) | `:2369-2371` `host_reset_laesst_parameter_und_zustand_unberuehrt (M-31)` | 4 704 Bytes gleich, Revision 1 → 1, eq_enabled 1, Bank im Kern 1, 1 kHz +12 dB |
| releaseResources->prepareToPlay, reset() und prepareToPlay allein enden in derselben Audiohistorie | `:2402-2403` `die_drei_hosteintritte_enden_in_derselben_audiohistorie (M-32)` | Peaks 0 / 0 / 0, untereinander bitgleich |
| Block über maxBlock bei rechnendem Kern: Lücke, der nächste Block beginnt bei 192 mit kFlagLueckeDavor, Segment +1 | `:2493-2496` `ungueltiger_tap_bei_rechnendem_kern_hinterlaesst_eine_luecke (M-35)` | zwei Deskriptoren: `stromVon` 0 in Segment 1, `stromVon` 192 mit Lücke in Segment 2 |
| Kern und Queue beschreiben dieselbe verworfene Zeitspanne | `:2501-2503` `dsp_und_queuezaehler_beschreiben_dieselbe_verworfene_zeitspanne (M-36)` | Kern +1, ohne Audio +1, Frames +128, oversize +0, Überlauf +0 |
| im Hard-Bypass wird derselbe Block ohne Lücke angenommen | `:2535-2539` `ruhender_passthrough_erzeugt_keine_luecke (M-37)` | drei Deskriptoren, Block 2 `stromVon` 64 mit 128 Frames, keine Lücke, ein Segment |
| endliche double über FLT_MAX am Tap kommen in der Analysekopie als endliche floats an; Kernzähler +1 024 | `:2582-2583` `analysekopie_verengt_nicht_unbemerkt (M-41)` | Tap über FLT_MAX, Analyse endlich, Ausgang roh, Zähler +1 024 |

**B6** — `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`

| Satz | Prüfzeile (Datei:Zeile, Fall) | gemessen |
|---|---|---|
| ein Block über maxBlock (1 024 bei 256) verwirft den Analyse-Tap | `DspGoldenTestMain.cpp:3076-3077` `ueberlast_verwirft_analyse (M-48)` | Tap verworfen und gezählt |
| und gleicht auf beiden Kanälen sample-exakt dem Lauf ohne Überlast in 4 x 256 | `:3079-3080` `der_uebergrosse_block_gleicht_sample_exakt_dem_lauf_ohne_ueberlast (M-48, B-22)` | alle 1 024 Samples beider Kanäle bitgleich |
| endlicher double über FLT_MAX kommt auf 0 verriegelt und im Eingangszähler gezählt heraus; kein Ausgangswert nicht endlich | `:3344-3346` `endlicher_eingang_erzeugt_keinen_nichtendlichen_floatausgang (M-39)`; Erwartung 0,0f für jeden Wert über dem Rand `:3339` | 512 von 1 024 über dem Rand, 0 nicht endlich, 0 abweichend, Zähler +512 |
| auf endlichem Material jeder float bitgleich die Verengung des double-Taps, Zähler steht | `:3390-3392` `ausgangsriegel_beruehrt_endliches_material_nicht (M-42 a)` | 204 800 verglichen, 0 abweichend, 105 396 mit Betrag ≥ 1, Zähler +0 |

**A3** — `eq-copilot/plugin/tests/MarkierungTestMain.cpp`

| Satz | Prüfzeile (Datei:Zeile, Fall) | gemessen |
|---|---|---|
| endlicher Eingang erzeugt keinen nicht endlichen Wet-Ausgang: Puls auf 1 kHz, Ton 0,8 x FLT_MAX, jedes Ausgangssample endlich; nach dem Einblenden steigt der Wet-Zähler um genau die verriegelten Samples | `MarkierungTestMain.cpp:798-800` `NAK-283 M-40: endlicher_eingang_erzeugt_keinen_nichtendlichen_wetausgang` (`nak283WetVerengung` `:747`, gerufen `:1574`) | 0 nicht endlich, Zähler +4 228 bei 4 228 verriegelten Samples |

**B17** — `eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp`

| Satz | Prüfzeile (Datei:Zeile, Fall) | gemessen |
|---|---|---|
| für 15 bis 19 keine zweite eigene Implementierung als Vergleich (Geltungsbereich ergänzt) | `Sonde013TruePeakGoldenTest.cpp:357` Erwartung `db (f.amplitude)`, Prüfung `:359` | analytische Erwartung je Fall |
| ein Gleichanteil bleibt auf 1e-9 genau er selbst (vorher „bitgenau“) | `:498` Grenze `1e-9` | Abweichung 0,000000000000 in der Anzeige mit zwölf Stellen |
| die vier transienten Fälle 20 bis 23, Versatz 0 bis 3 | `:545` Abschnitt `true_peak_matches_ebu_transient_set`, Schleife `:600` | vier Fälle |
| gegen eine im Test gerechnete, vorher an 15 bis 19 auf +/-0,01 dB validierte 64-fach-Referenz | `:565-569` (`referenzValidiert` `:568`) | Abstand höchstens 0,000005 dB |
| innerhalb +/-0,1 dB | `:616-618`, nur mit validierter Referenz | Abstand −0,03292 / −0,02755 / −0,03864 / −0,02755 dB |
| zusätzlich in der Normtoleranz 0,0 +0,2/-0,4 dBTP | `:623-625` | −0,15843 / −0,15306 / −0,16416 / −0,15306 dBTP |
| insgesamt 43 Prüfungen | Zählzeile `:632` | 43 bestanden, 0 gescheitert |

**A32** — `tools/plan/gesundheit.py`

| Satz | Prüfzeile (Datei:Zeile, Fall) | gemessen |
|---|---|---|
| Quellumfang aus einem prüfbaren Inventar: jede Datei mit Endung .rs, .cpp, .h oder .hpp unter broker/src und eq-copilot/plugin samt allen Unterbäumen | `gesundheit.py:208-209` `INVENTARWURZELN`, `QUELLENDUNGEN`; `inventar` `:461`, `sammle_quellen` `:524`; Selbsttest `:1540` „jede nicht ausgenommene Quelldatei wird gemessen, auch im neuen Unterbaum“ | 15 von 17 Inventareinträgen gemessen, 196 Dateien |
| mit Grund ausgenommen nur der flatc-Codegen und der Testbaum | `:212-221` `AUSGENOMMEN`, `TESTBAEUME`; Selbsttest `:1553` und `:1560` | `broker/src/generiert` 2 und `eq-copilot/plugin/tests` 42 Dateien ausgenommen, `eq-copilot/plugin/vertrag` 8 Dateien, davon 1 ausgenommen |
| der Bericht druckt das Inventar | `:1045-1057` | je Inventareintrag eine Zeile |
| eine Quelldatei, die weder gemessen noch ausgenommen ist, beendet den Lauf als WERKZEUGFEHLER (Inventarriegel, Exit 2) | `inventarluecken` `:499`, Riegel `:555-561`, Exit 2 `:1654-1656`; Selbsttest `:1542`, `:1544`, `:1548`, `:1577` | Selbsttest ok |
| die Grenzen werden ehrlich gemeldet, auch wenn sie reißen (NAK-292) | `bewerte` `:1012-1020`, Urteil `:1091-1093` | URTEIL GRENZE GERISSEN: 1 von 0, 35 von 28, 36 von 30 |
| jeder neue Treffer über der Zeilengrenze meldet sich als OHNE PFLEGETICKET | `:1113-1115` | `NakamaState.cpp` 2 911 Zeilen, OHNE PFLEGETICKET |
| 129 Fälle, dazu das Inventar mit neuem Unterbaum, Codegen, Tests und einer Ableitung, die still einen Unterbaum verliert | Zählzeile `:1605`; Fall `kein_quellbaum_bleibt_ungemessen` `:1509-1580` (acht Prüfungen) | Selbsttest 129/129 |
| Exit 4 heißt gerissen und ist ein Hinweis | `:1688`; im Runner `HinweisExits=@(4)` am Eintrag A32 | Exit 4 |
| Exit 2: roter Selbsttest, fehlende Inventarwurzel oder Inventarriegel | `:1638-1641`, `:538-543`, `:555-561`, `:1654-1656`; Selbsttest `:1507` | Selbsttest ok |

#### 38.2.3 Abweichungen vom Auftrag

1. **A16: „jedes andere Layout abgelehnt“ steht enger.** Gemessen wird eine feste Liste: Quadrophonie, 5.1 und vier diskrete Kanäle (`SondeNullTestMain.cpp:818-822`), Mono->Stereo, Stereo->Mono und drei deaktivierte Kombinationen (`:868-873`). Die Behauptung nennt genau diese; „jedes andere“ wäre mehr als gemessen (`tools/dirigent/pruefliste.md` E).
2. **A16: „Latenz 0“ und „Passthrough sanitisiert nichts“ nicht doppelt.** Beide stehen schon im alten Text („0 Samples Latenz und kein Tail …“, „der Passthrough sanitisiert kein NaN/Inf“) und werden weiter gemessen (`:318` `latenz_bleibt_null`, `:352` `passthrough_sanitisiert_nichts`; NAK-283 M-33 und M-42 (b)).
3. **B7: „Programm, Parameter, Zustand und publikationOffen unberührt“ steht als die gemessenen Größen.** Kein Test liest `publikationOffen` oder die Hostparameter einzeln. M-31 misst `getStateInformation` bytegleich, die Revision, das bestätigte `eq_enabled`, die engagierte Bank im Kern und die Filterwirkung (`TransactionTestMain.cpp:2369-2370`); genau das nennt die Behauptung.
4. **A3: „tdf2Lauf verengt zuerst, prüft dann“ nicht aufgenommen.** Das ist die Bauweise (`HoerMarkierung.h`, §33.1), keine Prüfung; der Test misst ihre Wirkung am Ausgang und am Wet-Zähler. Dass die Wirkung an der Bauweise hängt, zeigt der Rotbeweis MUT-40 (§33.2).
5. **B17: zwei Präzisierungen im bestehenden Text.** „also steht keine zweite eigene Implementierung als Vergleich dahinter“ gilt seit den Fällen 20 bis 23 nur noch für 15 bis 19 und sagt das jetzt. „ein Gleichanteil bleibt bitgenau er selbst“ misst `:498` mit der Grenze `1e-9` und heißt jetzt „auf 1e-9 genau“; der Prüftext des Tests sagt weiter „exakt“ (`:499`) — Testdatei außerhalb dieser Grenze, nicht geändert. Tiefpassmessung, Gegenprobe der Referenz am 4·fs-Signal und Indexprüfung für Versatz 3 sind nach §36.6 Abweichung 6 Gegenproben, keine Zusage; sie stehen nicht in der Behauptung (Prüfliste E: Wachen und Vorstufen gehören in den Skriptkopf) und zählen in den 43 Prüfungen mit.
6. **A32: vier Stellen ersetzt statt ergänzt.** „`broker/src/generiert/` ausgenommen“ allein wird zum Inventarsatz mit beiden Codegen-Orten und dem Testbaum; „Die Zeilengrenze HAELT seit NAK-225 … und die Pflegeticket-Zuordnung ist leer“ wird zum Satz über ehrlich gemeldete Grenzen — die leere Zuordnung (`PFLEGETICKETS`, `gesundheit.py:285`) ist Code, keine Prüfung; „121 Faelle“ wird 129; aus dem Werkzeugkopf „Bericht und JSON drucken das Inventar“ steht nur „der Bericht druckt das Inventar“, weil das Bein kein `--json` fährt. „Jeder neue Treffer“ heißt jetzt „Jeder neue Treffer ueber der Zeilengrenze“, weil der Bezugssatz entfällt. Die Backticks der alten Fassung entfallen.

### 38.3 Messung des Einzelauftrags Runner-Behauptungen (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `2a4615d6` (`nakama-nak283-f282744-runner`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-runner-behauptungen-auftrag.txt`; 06:3x bis 07:03 Uhr (31 min); Beobachter (Aufsicht LOCKER): Start, Telemetrie gesund, ein HEAD-Wechsel, Ende; Stundenloop `e09f8321` gelöscht. Commit `7781b316` auf `origin/master`, Basis `f2827441` Vorfahr, Baum sauber (nur die zwei fremden Ordner, §27). |
| Diff | `git diff --numstat f2827441..7781b316`: `tools/beweise.ps1` 6/6 (genau die sechs Behauptungszeichenketten A3, A16, B6, B7, B17, A32), `docs/beweise/NAK-283.md` (§38.2, Kopfzeile „Ticketpfade"), Rohdatei `NAK-283-runner-behauptungen-beine.txt`. Rundenbilanz: `Pruefwerkzeug 1 Datei(en) +6/-6 \| Doku 2 Datei(en) +851/-1` — ohne Produktfortschritt, wie beauftragt; kumuliert (`--runden … 32c09cb4 7781b316`): „kein Konvergenz-Signal". |
| Eigene Messung | `Parser::ParseFile` auf `tools/beweise.ps1`: 0 Parse-Fehler (am Stand `7781b316`); Rohdatei gelesen: AST-Vergleich 67 gegen 67 Einträge, genau sechs Behauptungen geändert, alle anderen Schlüssel gleich; Beinläufe 06:52 Uhr auf den Binaries des Zwischenkanons (Hash gleich): A16 65 Prüfungen Exit 0, B7 Exit 0, B6 Exit 0, A3 Exit 0, B17 43 bestanden Exit 0, A32 Exit 4 (erwartet; 129 von 129 Selbsttests, drei Maße gerissen, NAK-292). |
| Abweichungen §38.2.3 (Einordnung) | **Sechs, alle gedeckt** durch `tools/dirigent/pruefliste.md` E („Behauptung ≤ Messung"): 1 (A16 nennt die gemessene Layoutliste statt „jedes andere Layout"), 2 (Latenz 0 und „sanitisiert nichts" standen schon im Text), 3 (B7 nennt die gemessenen Größen — State-Bytes, Revision, bestätigtes `eq_enabled`, engagierte Bank, Filterwirkung — statt `publikationOffen` und Hostparameter, die kein Test einzeln liest), 4 (A3: Bauweise ist keine Prüfung), 5 (B17: „für 15 bis 19" präzisiert, „auf 1e-9 genau" statt „bitgenau"; Gegenproben nicht als Zusage), 6 (A32: vier Sätze ersetzt, weil der alte Wortlaut seit M-69 falsch wäre). Der Auftrag hatte an diesen Stellen mehr verlangt, als gemessen ist; der Worker hat richtig enger formuliert. Kein Befund. |
| Prüfung | Keine eigene Codex-Runde: geändert sind Behauptungstexte des Prüfwerkzeugs, keine Zusage, kein Test, kein Produktcode; `tools/beweise.ps1` liegt im Bereich der Abschlussprüfung `39e18d3f...HEAD` (Kopfzeile „Ticketpfade"), die den Nachzug mitprüft. §33.7 Punkt 1 und §36.9 Punkt 1 sind damit erledigt. |
| Offen im Ticket | Etappe 5 (nach U41/U42, One-Shot 09:04 Uhr), Abschlussprüfung, voller Kanon, Laufzeit-Arm. |
### 38.4 Antworten U41 und U42; Start der Etappe 5 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Zeitfenster | Der One-Shot-Cron für 09:04 Uhr feuerte um 12:03 Uhr (die Session war bis dahin in Läufen gebunden); beide Fragen im Fenster 9:00–23:00 Uhr gestellt (User 11.09.2026), Skill `/fragen`, je Karte ein Commit. |
| U41 (Headroom ohne Passage) | User-Wort 13.09.2026, 12:03 Uhr, Option wörtlich: „Gleitendes Fenster mit sichtbarer Länge (Empfohlen)". Abnahme `design/abnahmen/2026-09-13-headroom-ohne-passage-u41.md`, `docs/plan/fragen.json` (`beantwortet.U41`), Commit `002235e7`. Folge für Etappe 5: mit Passage das Histogramm über die ganze Passage (M-53 bis M-55); ohne Passage bleibt der 64-Rahmen-Ring (6,4 s) als gleitendes Fenster, kein Sammeln seit der Transportgrenze; `headroomFenster` nennt in beiden Fällen die tatsächliche Zahl der eingegangenen Rahmen; die sichtbare Beschriftung der Fensterlänge ist Gen-Oberfläche (S31b, geparkt). |
| U42 (Kurzzeit-Lautheit in der Sperrfrist) | User-Wort 13.09.2026, 12:0x Uhr, Option wörtlich: „Nichts zeigen — Feld leer (Empfohlen)". Abnahme `design/abnahmen/2026-09-13-lautheit-sperrfrist-u42.md`, `docs/plan/fragen.json` (`beantwortet.U42`), Commit dieses Abschlusses. Folge für Etappe 5: in der Sperrfrist kein Präsenzbit (M-56 bis M-58, R-283-5), sonst nichts — kein zusätzliches Feld, kein letzter gültiger Wert, keine Marke auf der Leitung; die Anzeige braucht über „ohne Präsenzbit kein Wert" hinaus keine Änderung. |
| U40 | Bleibt vertagt bis zur Messung (NAK-284, Zwischenantwort 12.09.2026); für Etappe 5 gilt der Kartendefault Weg 1: die Korrektur des Aktivitätsgates ohne zusätzliche Sichtbarkeitsgrenze (M-44 bis M-48, R-283-4). Offen bleiben die Karten U11, U13, U24, U25, U26, U40. |
| Etappe 5 | Frischer Opus-Worker (max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-5-auftrag.txt` (Ticketpfade nach §3 und §6.6 wörtlich; Matrix M-44 bis M-60, M-73, M-74 in §5.4; Regeln R-283-4 und R-283-5; Feinheiten 5 bis 8 und 17; Beine B16, B18, B17, B19, B20, B22, B23, B24, B25, B9, B5, A10, B3c, A26, A16, A1; Rotbeweise nach §6.1 mit Basisläufen; Goldenverschiebungen über den Erzeuger mit Zahl und Grund; Antworten U41/U42 und der U40-Default wörtlich im Auftrag; kein Kanon, kein Laufzeit-Arm). Basis-SHA = HEAD nach diesem Commit (steht im Workernamen). Aufsicht **ENG** (Analysefachmodelle: NaN-Verriegelung, Zahlenränder, Vertragsköpfe); Beobachter und Stundenloop nach Skill §3.2. Bauer-Abschnitt §39; danach Erstprüfung 5 (Vorlage A), Abschlussprüfung `39e18d3f...HEAD`, voller Kanon, Laufzeit-Arm. |
| Kontext | Dirigentensession bei 43 % vor dem Start (Telemetrie 07:1x Uhr); ab 50 % keine neue Prüfrunde — Übergabe nach Skill §5, sobald die Etappe eine saubere Grenze erreicht. |

## 39. Bauetappe 5 — Analysefachmodelle (Bauer, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **5 von 6** (F06, F07, F08, F10; Matrixzeilen M-44 bis M-60, M-73, M-74). Basis `bc0941e51bcdf8d3752362c3ca10cda628bd1438` (= `git rev-parse HEAD` beim Start); `git status --short` beim Start zeigt nur die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` (§27) — nicht angefasst, nicht committet. Auftrag `docs/beweise/roh/NAK-283-etappe-5-auftrag.txt` (§38.4). Spezifikation: die geprüfte Matrix §5.4, der Bauplan §6.6, die Regeln R-283-4 und R-283-5 (§2), §8.1 Feinheiten 5 bis 8 und 17. Produktfragen: U41 und U42 (User 13.09.2026) wie beantwortet umgesetzt, U40 mit dem Kartendefault Weg 1 (Gate ohne zusätzliche Sichtbarkeitsgrenze); keine Produktfrage beantwortet. |
| Vorher | Zu Beginn auf den Binaries des Basisstands (12:35 Uhr): A16 65 Prüfungen ok, A1 `NULLTEST OK`, B16 153/0, B18 46/0, alle Exit 0. Vor jedem Bau kein fremder Bau-, Test- oder Kanonprozess (NAK-290). |
| Umfang | **19 Matrixzeilen** gebaut und gemessen; fünf Produktköpfe und zwei Testdateien, +1 519/−58 Zeilen (§39.3), dazu dieser Abschnitt, die Kopfzeilen und die Rohdateien. |
| Beweise | **56 von 56 Rotbeweisläufen bestanden** (`docs/beweise/roh/NAK-283-etappe-5-rotbilanz.txt`): 37 Läufe aus 22 Mutationen an den Zusagezeilen, davon eine Beobachtung (die wörtliche Matrixmutation von M-48 macht nicht rot, Abweichung 1), dazu der Basislauf der fünf Produktköpfe am Basis-SHA mit 18 Läufen (15 rot, 3 Wachen grün) und der Riegel M-55 (b) am Basis-SHA. Jede Rücknahme bytegleich (SHA-256 vorher = nachher in jeder Rohdatei). |
| Beine | **41 von 41 Beinen Exit 0** im Endlauf nach den Rotbeweisen (`docs/beweise/roh/NAK-283-etappe-5-beine.txt`, §39.4): die 16 des Auftrags mit A16 und A1 als Wache, 19 weitere C++-Beine nach Trackinglog und 6 Python-Beine an neu gebauten Artefakten. Release-Bau von 42 Zielen Exit 0, keine `warning C` auf einer geänderten Zeile; `EqCopilot_VST3` und das Probeeq-Bundle ohne neue Warnung. Nulltest A16 und A1 vor der Etappe und im Endlauf grün. **A33:** Grenze gehalten, 83 von höchstens 83 Fundstellen, Exit 0. |
| Goldens | **Keine Verschiebung, keine Fixture geschrieben.** Alle Goldens der Etappe laufen unverändert grün (B5, B9, B17, B19, B22, B24, B25; A10 `--pruefen` bytegleich; A26); die Headroomverteilung der erzeugten Frames steckt in keiner eingefrorenen Fixture (`erzeuge_fb_fixtures.py` setzt die Headroomwerte synthetisch). |
| Abweichungen | **Elf**, alle in §39.7 mit Grund und Quelle; keine Matrixzeile geändert — die Prüfung entscheidet. |
| Offene Punkte | Vier Nebenbefunde (§39.9), nicht ins Register geschrieben. |

### 39.1 Was gebaut wurde

Zeilenangaben zum Etappenstand (die Rotbeweise stellen die Bytes bytegleich wieder her, §39.2).

**F06 — Energie gegen Energie am Gate (R-283-4, M-44 bis M-48).**
`FeatureEngine::rechneFenster` (`featureengine/Spektrum.h:431-451`) integriert die
Dichtesumme über `Δf = fs / s.punkte`, bevor sie in dB gegen `kAktivGateDb` steht:
`binBreiteHz = s.fs / (double) s.punkte`, `gesamt = summeBereich (s, 0, bins) * binBreiteHz`.
`s.psd` bleibt eine Dichte (Bandakkumulation, Fluss, Stereo lesen sie unverändert, §8.1
Feinheit 5); `Δf` entsteht je Stufe aus ihrer eigenen Punktzahl (Feinheit 6);
`kAktivGateDb` bleibt −60,0; die Nullprüfung `gesamt > 0.0 &&` steht weiter vor dem
Logarithmus. Das Zeitbereichsgate in `zelleSchliessen` (`Lautheit.h`) war schon Energie gegen
Energie und blieb unverändert; der Vertragskopf zu `abdeckung` (`Vertrag.h:712-725`) nennt
die Einheit.

**F07 — der Anfang ist ein Sample (R-283-5, §8.1 Feinheit 17; M-49 bis M-52, M-73, M-74).**
`setzePassagenfenster` (`FeatureEngine.h:749-812`) leert den Polyphasenfilter nicht mehr und
verwirft keinen Nachlauf mehr; es stellt den Startmerker `passagenStartScharf`
(`FeatureEngine.h:1406`) scharf. `verarbeiteSamples` (`Spektrum.h:76-156`) setzt im
Blockausschnitt neben `fensterEndetHier` das Gegenstück
`fensterBeginntHier = passagenStartScharf && von == passagenfenster.startSample` und lässt den
Merker fallen, sobald `b0 > startSample` (der Anfang liegt ungesehen zurück). Im Samplelauf
feuert er bei `i == passVon` genau einmal, vor dem ersten Passagensample:
`rest = tp.nachlauf()` geht in `rahmenTruePeak` (nicht in `passagenTruePeakRahmen`, M-51),
dann `tp.zuruecksetzen()`, `kurzfensterNeuBeginnen()`, und der Merker fällt.
`loeschePassagenfenster` und `zuruecksetzen` lassen ihn fallen (öffnen↔schließen, M-52).
Das Kurzzeitfenster der Passage (SONDE-013 R04) beginnt damit am echten Anfang;
`kurzfensterNeuBeginnen()` (`Lautheit.h:182-197`) läuft beim Binden nur noch, wenn
`passagenanfangNochVorn (startSample)` (`Spektrum.h:39-50`) meldet, dass kein Merker den
Anfang mehr sieht (Abweichung 3).

**F08 — Headroom über die Passage (R-283-5, §8.1 Feinheit 7; U41; M-53 bis M-55).**
Neuer Träger `HeadroomVerteilung` (`FeatureEngine.h:226-354`) als Element von `headroomRing`:
der bisherige `VerteilungsRing` (ohne Passage, U41) und ein speicherfestes Histogramm
(`kHeadroomKlassen = 26000` Klassen zu 0,01 dB über [−200, +60) dBTP, `uint32`, Heap aus
`vorbereiten()`), dazu `rahmen` (eingegangene Rahmen, sättigt bei `0xFFFFFFFF`, danach nimmt
das Histogramm nichts mehr auf), der belegte Klassenbereich und `nichtDarstellbar`.
`baueFrame` (`Frame.h:99-120`) schiebt den Rahmen mit Passage ins Histogramm, ohne Passage in
den Ring; `fuelleSkalare` (`Frame.h:657-693`) bildet P10/P50/P95 mit Passage über das
Histogramm (Rang p·(n−1), linear zwischen Klassenmitten — dieselbe Konvention wie
`perzentil`), `headroomFenster = rahmen`, ab vier Rahmen und nur, wenn kein Rahmenwert außerhalb
des Klassenbereichs lag; ohne Passage unverändert der Ring und seine Belegung. Jede Stelle,
die den Träger leert (`zuruecksetzen`, `grenzeZiehen` in `Zeit.h`, Binden, Lösen), trifft
beide Spannen mit einem `leeren()` — `Zeit.h` blieb unverändert. `kVerteilungPlaetze` bleibt
64. Der Vertragskopf (`Vertrag.h:670-688`) nennt dieselbe Bedeutung wie Entwurf, Matrix und
`nakama_telemetry_v1.fbs` (dort am Basis-SHA schon „Verteilung ueber die Rahmen der Passage",
„`fenster` ist die Zahl der WIRKLICH eingegangenen Rahmen"); Writer (`TelemetryClient.cpp`)
und die Leser in C++ und Rust bleiben unverändert — kein Feld wechselt Typ oder Bedeutung.

**F10 — Gültigkeit je Zelle (R-283-5, §8.1 Feinheit 8; M-56 bis M-59).**
Der NaN-Riegel in `verarbeiteSamples` (`Spektrum.h:200-212`) merkt sich die Stromposition
des jüngsten ersetzten Samples (`letztesErsetztesSample`); `zelleSchliessen`
(`Lautheit.h:30-54`) setzt je Zelle das Merkmal `kurzZellenErsetzt[kurzStand]`, wenn diese
Position in der schließenden Zelle liegt (sie umfasst genau die letzten `zellenSamples`
verarbeiteten Samples). `zellenFensterSauber (anzahl)` (`Lautheit.h:162-171`) prüft die
letzten `anzahl` Zellen; `kurzLufs` und `kurzTruePeak` (damit PSR und Crest über 3 s) fragen
alle 30, `momentanLufs` die letzten vier. Die LRA-Werte entstehen über `kurzLufs` und nehmen
damit kein kontaminiertes Fenster mehr auf. Die Rahmenverriegelung in `fuelleSkalare`
(`Frame.h:546-569`) bleibt als erste Stufe; die Zähler `rahmenNichtEndlich` und
`evidenzNichtEndlich` sättigen weiter und werden für die Marke nicht gelesen (M-59).

**Testzugang (M-59):** `friend struct FeatureEngineTestzugang;` und
`#define NAKAMA_FEATUREENGINE_TESTZUGANG 1` (`FeatureEngine.h:357-371`); die Struktur ist nur
in `Sonde013DynamicsTest.cpp` definiert und setzt die zwei Zähler. Kein Verhalten, kein
öffentlicher Setter (Abweichung 6).

**Kein Schemawechsel, kein persistentes Feld, kein Hostparameter, keine Identitätsänderung,
keine Legacy-Umbenennung; `kFeatureMetricsVersion` unverändert.**

### 39.2 Je Matrixzeile: Test und Rotbeweis

Alle Rotbeweise nach §6.1, gefahren von `docs/beweise/roh/NAK-283-etappe-5-rotskript.ps1`
(13:24 bis 13:38 Uhr): Originalbytes und SHA-256 jeder Quelle sichern, Mutation an der
Zusagezeile (jedes Muster genau einmal, sonst Abbruch; vorab mit `-Muster` geprüft: 25 von 25
Mustern je ein Treffer), Zeitstempel, Neubau des Ziels mit Frischeprüfung, Bein fahren — **ein
Rotlauf zählt nur mit Exit ungleich 0 und einer `[ROT]`-Zeile, die den Träger der Zusage
nennt** —, Originalbytes zurück (nie eine zweite Textersetzung), Hash gegen vorher,
Zeitstempel, Neubau, Bein grün mit `[ok]`-Zeile desselben Trägers. Eine **Beobachtung** (die
wörtliche Matrixmutation, die nicht rot macht) zählt nur mit `[ok]`-Zeile im mutierten Stand.
Der **Basislauf** ersetzt die fünf Produktköpfe byteweise durch ihren Stand am Basis-SHA
(Git-Blob mit den Zeilenenden des Arbeitsbaums) und fährt die neuen Tests des Etappenstands
dagegen: B16 167 bestanden / 12 gescheitert, B18 57 / 11 — der Testzugang für M-59 fehlt dort,
sein Fall meldet das und zählt nicht als Rotbeweis. Keine Zeile dieser Etappe steht in der
Nebenläufigkeitsliste aus §6.1: jede Bühne ist ein deterministischer Sampleplan auf dem
Workerpfad der Engine. Bilanz: `docs/beweise/roh/NAK-283-etappe-5-rotbilanz.txt`.

| Zeile | Test (Datei:Zeile, Name) | Bein | Rotbeweis: Mutation · Basislauf | Rohdatei(en) | rot · grün, Messwerte der Trägerzeile |
|---|---|---|---|---|---|
| M-44 | `Sonde013EventWireTest.cpp:2058` `aktivgate_ist_samplerateunabhaengig` | B16 | MUT-44, `Spektrum.h:448-449`: Δf aus der Gate-Rechnung entfernt (Stand am Basis-SHA) · Basislauf | `NAK-283-rot-M-44-etappe-5.txt`, `-M-44-basis-` | rot: Abdeckung 1,000 / 1,000 / 0,000 / 0,000 bei 44,1 / 48 / 96 / 192 kHz, Eingang −48,000 dB, je 9 Evidenzframes (MUT-44 und Basislauf gleich) · grün: 1,000 bei allen vier Raten |
| M-45 | `:2084` `aktivgate_ist_stufenunabhaengig` (L1 und L2 in einem Fall) | B16 | (a) MUT-44 · Basislauf, bei L1; (b) MUT-45, `Spektrum.h:448`: Δf fest aus `kHauptPunkte`, bei L2 | `-M-45a-`, `-M-45b-`, `-M-45-basis-` | (a) rot: 47 Bassbänder mit Beitrag, 0 Hauptbänder, Abdeckung 0 · grün: 47 und 149, Abdeckung 1; (b) rot: 47 Bassbänder, 0 Hauptbänder · grün: 0 und 0 |
| M-46 | `:2110` `aktivitaet_und_abdeckung_widersprechen_sich_nicht` (beide Richtungen) | B16 | MUT-44 · Basislauf | `-M-46-`, `-M-46-basis-` | rot: (a) Aktivität 1,000 bei Abdeckung 0,000, Widerspruch in 9 von 9 Evidenzframes · grün: (a) 1 und 1, (b) 0 und 0, kein Widerspruch |
| M-47 | `:2132` `aktives_fenster_liefert_bandwerte` | B16 | MUT-44 · Basislauf | `-M-47-`, `-M-47-basis-` | rot: `stereo_bands` 0, 0 Evidenzbänder mit Bit, Abdeckung 0 · grün: 161, 16, Abdeckung 1 |
| M-48 | **Regressionswache:** `:2144` `stille_bleibt_inaktiv_ohne_nichtendlichen_zwischenwert` ((a) digitale Stille, (b) Dichtesumme 0 nach Ton) | B16 | MUT-48 wörtlich, `Spektrum.h:450-451`: Nullprüfung entfällt — Beobachtung; MUT-48b an derselben Zeile: Dichtesumme 0 zählt als aktiv (Abweichung 1); Basislauf grün | `-M-48-beobachtung-`, `-M-48a-`, `-M-48b-`, `-M-48-basis-` | MUT-48 Exit 0: (a) Abdeckung 0,000, endlich, 6 Evidenzframes · MUT-48b rot: (a) Abdeckung 1,000, (b) 1,000 in 4 Evidenzframes · grün: 0,000 und 0,000 |
| M-49 | `:2177` `vorab_gebundene_passage_traegt_kein_material_davor` (Gegenlauf isoliert) | B16 | (a) MUT-49a, `Spektrum.h:148-156` und `FeatureEngine.h:786`: Nachlauf und Reset wieder beim Binden (Stand am Basis-SHA) · Basislauf; (b) MUT-49b fällt an M-73 | `-M-49-`, `-M-49-basis-` | rot: vorab +0,157 dBTP gegen isoliert −59,843 dBTP (die Zahlen des Audits, Übergabe §4 F07) · grün: −59,843 gegen −59,843 |
| M-50 | `:2202` `zwoelf_samples_vor_dem_fensteranfang_gehen_nicht_ein` (Anfang bei Blockversatz 384) | B16 | MUT-50, `Spektrum.h:148`: Reset bei `i == passVon + 1`; dazu MUT-49a · Basislauf | `-M-50b-`, `-M-50-`, `-M-50-basis-` | rot MUT-50: vorab −1,383 dBTP; MUT-49a und Basislauf: −0,815 dBTP; jeweils gegen isoliert −60,000 (Kante davor −0,92 dBTP) · grün: −60,000 gegen −60,000 |
| M-51 | `:2227` `nachlauf_beim_binden_geht_in_die_livemetrik_nicht_in_die_passage` ((A) Binden mit Spitze in der Kette, (B) Spitze direkt vor dem Anfang) | B16 | (A) MUT-49a · Basislauf; (B) MUT-51a, `Spektrum.h:151`: Nachlauf am Startsample verworfen; MUT-51b: Nachlauf zusätzlich in die Passage (Abweichung 4) | `-M-51A-`, `-M-51-basis-`, `-M-51Ba-`, `-M-51Bb-` | (A) rot: Rahmen mit Bindung −9,031 dBTP (Samplescheitel) gegen −5,920 ohne · grün: −5,920 gegen −5,920, Passagenbit leer; (B) rot MUT-51a: Rahmen −9,031 gegen −5,920; rot MUT-51b: Passage −5,920 gegen isoliert −60,000 · grün: Rahmen −5,920, Passage −60,000 |
| M-52 | `:2326` `passagenanfang_und_passagenende_wirken_auf_dieselben_historien` (vier Prüfungen, ein Lauf) | B16 | True-Peak-Kette Anfang: MUT-49a · Basislauf; Ende: MUT-52b, `Spektrum.h:177-178`: Nachlauf an `passBis` nicht für die Passage; 3-s-Zellen Anfang: MUT-52c, `Spektrum.h:153`: kein Neubeginn am Startsample; Headroom beide Grenzen: MUT-52d, `Frame.h:119-120`: Rahmen davor und dahinter gehen ein, und MUT-53 (Abweichung 5) | `-M-52a-`, `-M-52b-`, `-M-52c-`, `-M-52d-`, `-M-52e-`, `-M-52-basis-` | rot: Anfang −0,736 dBTP (Kanten −0,92); Ende −29,031 = Samplescheitel; 3-s-Zellen 27 Rahmen mit 3-s-Crest vor 2,9 s, erster ab 0,093 s mit 5,105 dB (Basislauf: 18, ab 1,053 s, 10,060 dB); Headroom Fenster 187 bei 132 Rahmen, P95 −6,025 (MUT-52d), Fenster 64 (MUT-53, Basislauf) · grün: −25,918 dBTP; 0 Rahmen vor 2,9 s, erster ab 3,080 s mit 3,010 dB; Fenster 132 bei 132, P10 = P50 = P95 = −40,005 |
| M-53 | `Sonde013DynamicsTest.cpp:825` `headroomverteilung_umfasst_die_ganze_passage` | B18 | MUT-53, `Frame.h:116-119` und `:671-673`: Verteilung wieder aus dem 64er-Ring (Stand am Basis-SHA) · Basislauf | `-M-53-`, `-M-53-basis-` | rot: P10 = P50 = P95 = −66,021 dBTP, Fenster 64 · grün: P10 −66,025, P50 −6,025, P95 −6,025, Fenster 140 |
| M-54 | `:825` `headroomfenster_nennt_die_passagenlaenge` (derselbe Fall) | B18 | MUT-53 · Basislauf; MUT-54, `Frame.h:679`: `headroomFenster` höchstens Ringbelegung bei richtigen Perzentilen | `-M-54-`, `-M-54b-`, `-M-54-basis-` | rot: Fenster 64 (MUT-53, Basislauf); MUT-54: Fenster 64 bei P95 −6,025 — die Zahl lügt über die Rechnung · grün: 140 |
| M-55 | **Regressionswache:** `:881` `headroom_unter_vier_rahmen_bleibt_ungesetzt` (a) mit Speicherseite (b), dazu Riegel `NAK-283-etappe-5-riegel-M-55.ps1` | B18, Riegel | (a) MUT-55, `Frame.h:673`: Mindestbelegung drei; (b) MUT-55b, `FeatureEngine.h:118`: `kVerteilungPlaetze = 65`; Basislauf und Riegel am Basis-SHA grün | `-M-55-`, `-M-55b-`, `-M-55-basis-`, `-M-55b-basis-` | (a) rot: gesetzt, Fenster 3 · grün: kein Bit, Gegenprobe vier Rahmen Fenster 4; (b) rot: R2 fällt · grün: 2 von 2; 0 Allokationen in 20 s Passage (Basis und Etappe) |
| M-56 | `:952` `kurzzeitlautheit_bleibt_ungesetzt_solange_ihr_fenster_kontaminiert_ist` | B18 | MUT-56, `Lautheit.h:54`: Zellenmerkmal entfernt (Stand am Basis-SHA) · Basislauf | `-M-56-`, `-M-56-basis-` | rot: erster sauberer Rahmen (0,1 s) Momentary und Short-term gesetzt · grün: beide leer; bei 3,0 s Short-term gesetzt |
| M-57 | `:952` `momentan_und_kurzzeit_erholen_sich_mit_eigenen_fristen` (derselbe Fall) | B18 | MUT-57, `Lautheit.h:140`: Momentary an die 3-s-Frist; MUT-56 · Basislauf | `-M-57b-`, `-M-57-`, `-M-57-basis-` | rot MUT-57: 0,4 s beide leer; MUT-56 und Basislauf: 0,4 s beide gesetzt · grün: 0,4 s und 2,9 s Momentary gesetzt, Short-term leer |
| M-58 | `:611` M-07-Fall `non_finite_never_reaches_wire_for_new_metrics`, um die Erholungsstrecke erweitert (`:680-731`) | B18 | (a) MUT-58a, `Lautheit.h:115-117`: Verriegelung eine Zelle länger; (b) MUT-58b, `Lautheit.h:115-116` und `:140-141`: Fenster ohne Zellenmarken, nur Rahmenlatch (Stand am Basis-SHA) · Basislauf | `-M-58a-`, `-M-58b-`, `-M-58c-`, `-M-58-basis-` | rot (a): 2,9 s und 3,0 s Short-term leer; (b): 0,3 s Momentary gesetzt, 2,9 s Short-term gesetzt · grün: 0,3 s leer, 0,4 s gesetzt, 2,9 s leer, 3,0 s gesetzt; bei 3,0 s S und M −7,9935 gegen Referenz −7,9935 LUFS, Zähler 0/0 |
| M-59 | `:1015` `nichtendlich_zaehler_saettigen_und_die_verriegelung_bleibt` (Testzugang) | B18 | Sättigung (**Regressionswache**): MUT-59a, `Spektrum.h:204-205`: ohne Sättigungsprüfung; Gültigkeitsmarke: MUT-59b, `Lautheit.h:53`: Marke aus dem Zählerstand | `-M-59a-`, `-M-59b-` | rot MUT-59a: Zähler 0xE / 0xE (gewrappt) · grün: 0xFFFFFFFF / 0xFFFFFFFF, Rahmen verriegelt; rot MUT-59b: 0,1 s Momentary und Short-term gesetzt, 2,9 s Short-term gesetzt · grün: 0,1 s leer, 0,4 s Momentary, 3,0 s Short-term |
| M-60 | `:1089` `jede_analysehistorie_traegt_ihre_eigene_spanne` (ein Lauf, drei Prüfungen) | B18 | True Peak MUT-49a, Headroom MUT-53, Loudness MUT-56 · Basislauf (alle drei rot) | `-M-60a-`, `-M-60b-`, `-M-60c-`, `-M-60-basis-` | rot: Passagenmaximum −0,736 dBTP; P10 = P95 = −66,021, Fenster 64; 0,3 s Momentary und 2,9 s Short-term gesetzt · grün: −6,021 dBTP; P10 −66,025, P95 −6,025, Fenster 140; 0,3 s leer, 0,4 s Momentary, 2,9 s leer, 3,0 s Short-term |
| M-73 | **Regressionswache:** `Sonde013EventWireTest.cpp:2409` `passagenmaximum_haengt_nicht_an_der_blockgroesse` | B16 | MUT-49b, `Spektrum.h:148`: ohne Startmerker Reset an jedem Blockanfang in der Passage; Basislauf grün | `-M-73-`, `-M-73-basis-` | rot: 64 → −9,031 dBTP (der Samplescheitel des lauten Abschnitts), 512 und 1 024 → −5,919; Spanne 3,111 dB · grün: dreimal −5,919, Spanne 0,000; Basislauf: dreimal +0,157 |
| M-74 | `:2409` `passagenmaximum_gleicht_dem_isolierten_lauf_bei_jeder_blockgroesse` (derselbe Testaufbau) | B16 | MUT-49a (Startmerker wandert ans Binden) · Basislauf | `-M-74-`, `-M-74-basis-` | rot: dreimal +0,157 dBTP gegen isoliert −5,919, Abweichung 6,077 dB · grün: dreimal −5,919, Abweichung 0,000 |

Rohdateien vollständig `docs/beweise/roh/NAK-283-rot-M-<nn>-etappe-5.txt`; die Spalte kürzt
den gemeinsamen Präfix. **Regressionswachen sind kein Beleg für ihren Befund**
(`tools/dirigent/pruefliste.md` E): M-48, M-55, M-59 (Sättigung) und M-73. Die Belege liegen
bei M-44 bis M-47 (F06), M-49 bis M-51 und M-74 (F07), M-53 und M-54 (F08), M-56 bis M-58 und
M-59 (Marke, gegen die Fixfassung) (F10) und bei den Übergängen M-52 und M-60.

**M-73 misst die Matrixrechnung wörtlich.** Unter MUT-49b erreicht der 64er-Lauf am lauten
Abschnitt genau den Samplescheitel −9,031 dBFS und keine Überhöhung; 512 und 1 024 sehen
denselben Kettenzustand und rekonstruieren −5,919 dBTP. Der Abstand 3,111 dB ist die ganze
Intersample-Überhöhung dieses drei Perioden langen Abschnitts — etwas mehr als die 3,01 dB des
eingeschwungenen Falls, weil der Kern zwölf Samples nach jeder Seite in den −60-dBFS-Grund
greift.

**Die fünf Hashes der Produktköpfe am Etappenstand** (SHA-256 im Arbeitsbaum; jede Rohdatei
trägt je Mutation vorher, mutiert und nachher):

| Datei | SHA-256 |
|---|---|
| `eq-copilot/plugin/core/analysis/FeatureEngine.h` | `C4F57568B5A596920E64F88E016FC98BA4BBDA27F0AC8D1750BE66E8167A18FD` |
| `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h` | `1D1E5CA27AEE5647AE2DF7B41EE0E3F888E799EA546079934BA350BD6AE36BBA` |
| `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h` | `7DA9740FB8FFF98EBFFC5F33AED23B6BDC2BC96A2C6D5CC9CAE0914CB18978BD` |
| `eq-copilot/plugin/core/analysis/featureengine/Frame.h` | `9B626DA98F3DC80EEDA33F91BCA794FD4BCC02368DDDEB4E299EDFFFE7E7A001` |
| `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h` | `F006B56135665145D99F1ECF042716029BFE086441A905DDB4F3BF77741AE289` |

### 39.3 Geänderte Dateien

| Datei | Zeilen (+/−) | Was |
|---|---:|---|
| `eq-copilot/plugin/core/analysis/FeatureEngine.h` | +227 / −35 | `kHeadroomKlassen`-Konstanten und `HeadroomVerteilung`, Testzugang, `vorbereiten` (Histogramm, Markenring), `zuruecksetzen`, `setzePassagenfenster` (Startmerker statt Reset, R04 nur für zurückliegende Anfänge), `loeschePassagenfenster`, Deklarationen, Felder `passagenStartScharf`, `kurzZellenErsetzt`, `letztesErsetztesSample`, `headroomRing` |
| `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h` | +90 / −1 | `passagenanfangNochVorn`, Startmerker im Ausschnitt und im Samplelauf, Samplestelle der Ersetzung, Gate über Δf |
| `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h` | +77 / −0 | Zellenmarke in `zelleSchliessen`, `zellenFensterSauber`, `kurzfensterNeuBeginnen`, Markenprüfung in `kurzLufs`, `momentanLufs`, `kurzTruePeak` |
| `eq-copilot/plugin/core/analysis/featureengine/Frame.h` | +58 / −11 | Headroomeinschub und -perzentile je Spanne, Kommentar der zweistufigen Verriegelung, `rahmenLeeren` |
| `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h` | +18 / −7 | Köpfe Headroomverteilung und Abdeckung |
| `eq-copilot/plugin/tests/Sonde013EventWireTest.cpp` | +595 / −0 | Helfer `fahreGate`, `fahrePassage`, `fs4Abschnitt`; Fälle M-44 bis M-52, M-73/M-74 |
| `eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp` | +454 / −4 | Allokationszähler, Testzugang, M-58 im M-07-Fall, Fälle M-53/M-54 mit U41-Wache, M-55, M-56/M-57, M-59, M-60 |

Zeilenzahlen aus `git diff --numstat bc0941e5` am Etappenstand. Zeilenenden wie vorgefunden
zurückgeschrieben (`git ls-files --eol`: fünf Produktköpfe und `Sonde013EventWireTest.cpp`
`w/crlf`, `Sonde013DynamicsTest.cpp` `w/lf`). Gemessen leer: `git diff --stat bc0941e5 --`
`broker`, `eq-copilot/plugin/{sonde,dsp,state,src}`, `core/StampedAudioQueue.h`,
`core/analysis/TruePeak.h`, `featureengine/{Zeit,Stereo,Fingerprint}.h`,
`eq-copilot/{schemas,identity,install,fixtures}`, `eq-copilot/plugin/CMakeLists.txt`, `tools`,
`docs/PLAN-STAND.md`, `docs/offene-punkte.md`, `docs/plan`, `design`. Neu unter
`docs/beweise/roh/`: `NAK-283-etappe-5-beinskript.ps1`, `-beine.txt` (Endlauf),
`-beine-lauf-2.txt`, `-rotskript.ps1`, `-rotbilanz.txt`, `-riegel-M-55.ps1`, `-pegel.py`,
`-pegel.txt`, `-tidy.txt` und 56 Rohdateien `NAK-283-rot-M-*-etappe-5.txt`.

### 39.4 Gefahrene Beine

Gefahren von `docs/beweise/roh/NAK-283-etappe-5-beinskript.ps1`. Maßgeblich ist der **Endlauf**
nach den Rotbeweisen: Rohausgabe `docs/beweise/roh/NAK-283-etappe-5-beine.txt` (13:38 Uhr; HEAD =
Basis `bc0941e5`, der Etappenstand liegt im Arbeitsbaum).

**Bau.** Vor dem Aufruf kein fremder Bau-, Test- oder Kanonprozess (NAK-290; `Get-Process` auf
`cl`, `link`, `MSBuild`, `cargo`, `rustc`, `cmake` und `EqCop*`, sonst Abbruch). Das Skript stempelt die
sieben geänderten Quellen neu (SHA-256 vorher = nachher; die fünf Produktköpfe tragen die Hashes
aus §39.2) und baut Release 42 Ziele: die 36 C++-Beine des Kanons, die Artefakte der Python-Beine
(`EqCopIpcLast`, `EqCopSessionSoak`, `EqCopSonde012SourcesLatencyTest`), `NakamaKern`,
`EqCopilot_VST3` und `NakamaProbeeq_VST3` — Exit 0 in 409,4 s. `warning C`: 43 eindeutige Zeilen
an 8 Stellen, **keine auf einer Zeile, die diese Etappe hinzugefügt oder geändert hat**. Alle acht
stehen in Dateien, die diese Etappe nicht ändert: `featureengine/Stereo.h:70` C4458,
`IpcTestMain.cpp:3705` und `:3987` C4456, `MarkierungTestMain.cpp:1074` C4457,
`Sonde012HostChannelContextTest.cpp:295` und `:450` C4244, `StateMigrationTestMain.cpp:1348` und
`:1636` C4456. `EqCopilot_VST3` und das Probeeq-Bundle bringen damit keine neue Warnung.

**Auswahl.** Das Skript liest die Beinliste aus `tools/beweise.ps1` (A15 stillgelegt, nicht
gezählt), entscheidet am MSBuild-Trackinglog jedes C++-Ziels (`CL.read`, `link.read`), ob es eine
geänderte Quelle übersetzt oder linkt, und bei Python- und Cargo-Beinen, ob sie ein Artefakt aus
`eq-copilot/build/plugin` berühren (Abweichung 11). Gefahren: **41 Beine** — die 16 des Auftrags
(A16 und A1 als Wache des Audiopfads), 19 weitere C++-Beine nach Trackinglog und 6 Python-Beine an
einem neu gebauten Artefakt. Nicht gefahren: A2, B3 und B3b (lesen und linken keine geänderte
Quelle), die Cargo-Beine A4, A4-SI, A21 und A31 und die übrigen Python-Beine (berühren kein
Artefakt des Plugin-Baus); A33 lief getrennt wie in Etappe 4 (unten), A32 lief nicht. B9 steht im
Auftrag, liest aber keine geänderte Quelle; sein Binary vom 12.09.2026 ist die Gegenprobe der
Auswahl.

**Frische.** Vor dem ersten Bein prüft das Skript jedes gebaute Artefakt gegen seine jüngste
geänderte Abhängigkeit (`Vertrag.h`, 13:38:20 Uhr; bei B16 und B18 die eigene Testdatei) und
bricht sonst ab: alle 37 frisch (13:38:25 bis 13:44:31 Uhr).

| Bein | Ziel | Grund | Exit | s | Schlusszeile des Beins |
|---|---|---|---:|---:|---|
| A1 | `EqCopNullTest` | §6.6, Wache Audiopfad | 0 | 0,2 | `NULLTEST OK` |
| A3 | `EqCopMarkierungTest` | Trackinglog | 0 | 7,7 | `MARKIERUNGSTEST OK` |
| A4b | `EqCopPipeClientTest` | Trackinglog | 0 | 66,2 | `PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler` |
| A16 | `EqCopProbeeqNullTest` | §6.6, Wache Audiopfad | 0 | 0,4 | `SONDE-NULLTEST OK - 65 Pruefungen ok, 0 Fehler` |
| B1 | `EqCopIdentityTest` | Trackinglog | 0 | 0,2 | `IDENTITY-TEST OK - 120 Pruefungen ok, 0 Fehler` |
| B2 | `EqCopStateMigrationTest` | Trackinglog | 0 | 1,7 | `STATE-MIGRATION-TEST OK - 287 Pruefungen ok, 0 Fehler` |
| B3c | `EqCopSchemaTest` | §6.6 | 0 | 0,6 | `152 bestanden, 0 gescheitert` |
| B4 | `EqCopQueueStressTest` | Trackinglog | 0 | 3,4 | `QUEUE-STRESSTEST OK` |
| B9 | `EqCopLoudnessGoldenTest` | §6.6 | 0 | 0,1 | `LOUDNESS-GOLDEN OK` |
| B5 | `EqCopAnalysisGoldenTest` | §6.6 | 0 | 6,4 | `Ergebnis: 248 bestanden, 0 Fehler.` |
| B6 | `EqCopDspGoldenTest` | Trackinglog | 0 | 5,9 | `DSP-GOLDEN OK` |
| B7 | `EqCopTransactionTest` | Trackinglog | 0 | 0,4 | `TRANSAKTION OK` |
| B10 | `EqCopIpcTest` | Trackinglog | 0 | 221,3 | `ALLE PRUEFUNGEN GRUEN — 432 Pruefungen, 0 Fehler` |
| B11 | `EqCopSonde012HostChannelContextTest` | Trackinglog | 0 | 1,1 | `SONDE-012 HOST CHANNEL CONTEXT: 21 bestanden, 0 fehlgeschlagen` |
| B12 | `EqCopSonde012LoudnessSourceTest` | Trackinglog | 0 | 1,7 | `SONDE-012 LOUDNESS SOURCE: 4 bestanden, 0 fehlgeschlagen` |
| B13 | `EqCopSonde012SourcesModelTest` | Trackinglog | 0 | 0,2 | `SONDE-012 SourcesModel: 88/88 gruen` |
| B14 | `EqCopSonde012ProjectReloadTest` | Trackinglog | 0 | 8,7 | `SONDE-012 ProjectReload: 231/231 gruen` |
| B15 | `EqCopShot --sonde012-suite eq-copilot/build/sonde012-shots` | Trackinglog | 0 | 2,9 | `SONDE-012 SHOTS 25/25 gruen` |
| B20 | `EqCopSonde013QualityClassTest` | §6.6 | 0 | 0,4 | `24 bestanden, 0 gescheitert` |
| B22 | `EqCopSonde013FingerprintGoldenTest` | §6.6 | 0 | 1,1 | `24 bestanden, 0 gescheitert` |
| B23 | `EqCopSonde013PassageStateTest` | §6.6 | 0 | 11,7 | `409 bestanden, 0 gescheitert` |
| B24 | `EqCopSonde013PrePostGoldenTest` | §6.6 | 0 | 1,1 | `30 bestanden, 0 gescheitert` |
| B25 | `EqCopSonde013ExperimentGoldenTest` | §6.6 | 0 | 0,1 | `27 bestanden, 0 gescheitert` |
| B26 | `EqCopSonde013ExperimentUiTest` | Trackinglog | 0 | 0,5 | `52 bestanden, 0 gescheitert` |
| B21 | `EqCopSonde013InterventionRingTest` | Trackinglog | 0 | 0,2 | `22 bestanden, 0 gescheitert` |
| B19 | `EqCopSonde013StereoGoldenTest` | §6.6 | 0 | 2,2 | `91 bestanden, 0 gescheitert` |
| B17 | `EqCopSonde013TruePeakGoldenTest` | §6.6 | 0 | 0,4 | `43 bestanden, 0 gescheitert` |
| B18 | `EqCopSonde013DynamicsTest` | §6.6 | 0 | 6,0 | `70 bestanden, 0 gescheitert` |
| B16 | `EqCopSonde013EventWireTest` | §6.6 | 0 | 2,1 | `179 bestanden, 0 gescheitert` |
| B27 | `EqCopSonde014IntentTest` | Trackinglog | 0 | 0,6 | `SONDE-014-INTENT-TEST OK - 270 Pruefungen ok, 0 Fehler` |
| B29 | `EqCopSonde014AssistentTest` | Trackinglog | 0 | 0,9 | `SONDE-014 AssistantStep: 193/193 gruen` |
| B28 | `EqCopSonde014BefundTest` | Trackinglog | 0 | 0,1 | `SONDE-014 Befund und Maskierung: 81/81 gruen` |
| B8 | `EqCopLebenslaufTest` | Trackinglog | 0 | 0,3 | `LEBENSLAUF-TEST OK - 79 Pruefungen ok, 0 Fehler` |
| A10 | `py -3.13 tools/eq-copilot/erzeuge_fb_fixtures.py --pruefen` | §6.6 | 0 | 5,8 | `bytegleich: 129 Dateien, MANIFEST sha256=342f9d91…` |
| A14 | `py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py` | Artefakt (NakamaKern, EqCopilot_VST3) | 0 | 12,9 | `25 ok, 0 Fehler` |
| A17 | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | Artefakt (EqCopIpcTest) | 0 | 3,2 | `99 ok, 0 Fehler` |
| A22 | `py -3.13 tools/eq-copilot/pruefe_ipc_last.py` | Artefakt (EqCopIpcLast) | 0 | 10,7 | `GRUEN` |
| A24 | `py -3.13 tools/eq-copilot/pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | Artefakt (EqCopSessionSoak) | 0 | 148,9 | `GRUEN` |
| A34 | `py -3.13 tools/eq-copilot/pruefe_session_soak.py --selbsttest` | Artefakt (EqCopSessionSoak) | 0 | 0,2 | `Selbsttest: 4 von 4 Faellen bestanden` |
| A26 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py` | §6.6 | 0 | 0,3 | `Exit-Gate P4: alle vier Klassen bestehen, keine falsche Behauptung.` |
| A23 | `py -3.13 tools/eq-copilot/pruefe_sonde012_sources_latency.py` | Artefakt (EqCopSonde012SourcesLatencyTest) | 0 | 4,7 | `GRUEN - sechs p95-Grenzen und Anzeige-Revisionen` |

**Drei Läufe.** Lauf 1 (vor den Rotbeweisen) baute und brach in der Auswahl vor der
Frischeprüfung ab — „A34 pruefe_session_soak.py beruehrt eq-copilot/build/plugin und hat keine
Artefaktzuordnung“; kein Bein lief, die Rohausgabe ist nicht aufbewahrt. A34 ist dasselbe Skript
wie A24 mit `--selbsttest` und seither dem Soak-Artefakt zugeordnet. **Lauf 2** (13:08 Uhr): 41
von 41 Exit 0, Bau 343,6 s, dieselben acht Warnstellen, keine auf einer geänderten Zeile
(`docs/beweise/roh/NAK-283-etappe-5-beine-lauf-2.txt`). Danach mutierten die Rotbeweise die
Produktköpfe und spielten sie bytegleich zurück (13:24 bis 13:38 Uhr); der Endlauf belegt den
Stand danach.

**Nulltest.** A1 (`NULLTEST OK`, darin „1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)“) und
A16 (`SONDE-NULLTEST OK - 65 Pruefungen ok, 0 Fehler`) sind grün vor der ersten Änderung
(Basisbinaries, 12:35 Uhr), in Lauf 2 und im Endlauf; die Ausgaben von Lauf 2 und Endlauf sind Zeile für Zeile gleichlautend (A1 11, A16 80 Zeilen).

**A33 (clang-tidy-Ratsche).** `py -3.13 tools/plan/tidy.py --mit-selbsttest --jobs 3` am
Etappenstand (13:54 Uhr, 277,3 s): Selbsttest 39 von 39, **Grenze gehalten — 83 Fundstellen von
höchstens 83**, Exit 0 (`docs/beweise/roh/NAK-283-etappe-5-tidy.txt`). Sieben Fundstellen liegen
in geänderten Dateien (`FeatureEngine.h:1104`, `Spektrum.h:393-401`, alle
`bugprone-misplaced-widening-cast`), keine auf einer geänderten Zeile: es sind die Fundstellen
`FeatureEngine.h:939` und `Spektrum.h:322-330` der Etappe 4
(`docs/beweise/roh/NAK-283-etappe-4-tidy.txt`, dort ebenfalls 83 von 83), verschoben um die davor
eingefügten 165 und 71 Zeilen (`git diff -U0 bc0941e5`).

### 39.5 Bühnenpegel M-44 bis M-47, gerechnet und gemessen

**Größe.** „dBFS RMS" meint in M-44 bis M-47 die mittlere Kanalenergie
`10·log10 (mean (x²))` — dieselbe Größe, die `zelleAktivEnergie` (`Lautheit.h`) und seit
R-283-4 das integrierte Spektralgate gegen `kAktivGateDb = −60,0` halten. Für einen
Stereosinus mit gleicher Amplitude `A` auf beiden Kanälen ist sie `20·log10 (A/√2)`; die
Tests setzen `A = √2·10^(L/20)` (`amplitudeFuerPegelDb`) und messen den Eingang mit: bei
allen vier Raten −48,000 dB (M-44).

**Versatz ohne Integration.** Mit `norm = 2/(fs·Σw²)` ist die Dichtesumme einer Stufe
`gesamt = P · punkte/fs` (Parseval über das Hann-Fenster), liegt also um
`10·log10 (fs/punkte)` unter dem Energiewert `P`:

| fs | Hauptstufe (`kHauptPunkte = 4096`) | Bassstufe (`kBassPunkte = 16384`) |
|---|---:|---:|
| 44,1 kHz | 10,32 dB | 4,30 dB |
| 48 kHz | 10,69 dB | 4,67 dB |
| 96 kHz | 13,70 dB | 7,68 dB |
| 192 kHz | 16,71 dB | 10,69 dB |

Die Stufen trennen `10·log10 (16384/4096) = 6,02 dB`. Nachgerechnet mit
`docs/beweise/roh/NAK-283-etappe-5-pegel.py`, Ausgabe `NAK-283-etappe-5-pegel.txt`.

**M-44, −48,0 dBFS.** Die gezählte Hauptstufenentscheidung muss ohne Δf bei 48 kHz über
und bei 96 kHz unter dem Gate liegen: `−60 + 10,69 < L < −60 + 13,70`, also
−49,31 dBFS < L < −46,30 dBFS; −48,0 liegt 1,31 dB über der unteren und 1,70 dB unter der
oberen Kante. Ohne Δf: −58,32 / −58,69 / −61,70 / −64,71 dB → Abdeckung 1 / 1 / 0 / 0
(Basislauf und MUT-44 gemessen: 1,000 / 1,000 / 0,000 / 0,000); mit Δf −48,00 dB bei
allen Raten → 1 / 1 / 1 / 1.

**M-45, L1 = −52,3 und L2 = −63,0 dBFS bei 48 kHz.** Ohne Δf bei L1: Bass
−52,3 − 4,67 = −56,97 dB (aktiv), Haupt −52,3 − 10,69 = −62,99 dB (inaktiv) — die Stufen
entscheiden verschieden (gemessen: 47 Bassbänder mit Beitrag, 0 Hauptbänder). Mutation
„Δf fest aus kHauptPunkte" bei L2: Bass −63,0 + 6,02 = −56,98 dB (aktiv), Haupt −63,00
(inaktiv); das Fenster, in dem nur diese Mutation kippt, ist −66,02 bis −60,00 dBFS. Mit
Δf: L1 beide aktiv (47 Bass-, 149 Hauptbänder mit Beitrag, Abdeckung 1), L2 beide inaktiv.

**M-46, −58,0 und −62,0 dBFS bei 44,1 kHz.** Ohne Δf bei −58,0: Bass −62,30 dB, Haupt
−68,32 dB, beide inaktiv, während das Zeitbereichsgate bei −58,00 dB aktiv meldet —
`aktivitaet` 1 bei `abdeckung` 0 in 9 von 9 Evidenzframes (Basislauf). Mit Δf: (a) 1 und 1,
(b) 0 und 0, kein Widerspruch.

**M-47, −58,0 dBFS bei 96 kHz.** Ohne Δf: Bass −65,68 dB, Haupt −71,70 dB, beide unter dem
Gate — `stereo_bands` 0, kein Evidenzband mit Bit (Basislauf). Mit Δf: `stereo_bands` 161,
16 Evidenzbänder mit Bit, Abdeckung 1. Dass 161 Bänder eine Stereobasis tragen, aber nur 16
ein Evidenzbit, zeigt nebenbei, warum M-45 die Stufenentscheidung an `freiheitsgrade` statt
an Evidenzbits liest (Abweichung 2).

### 39.6 Goldens

Keine Verschiebung. Die Headroomrechnung, das Gate und die Zellenmarke ändern keinen
eingefrorenen Wert: B5 (`EqCopAnalysisGoldenTest`, Zwillingsprobe G13 und Grenzen), B9, B17,
B19, B22, B24 und B25 laufen unverändert grün (§39.4), A10 findet den FlatBuffers-Korpus
bytegleich zur Neuerzeugung (die Headroomtabelle dort setzt `erzeuge_fb_fixtures.py` mit
festen Zahlen, nicht aus der Engine), A26 hält den P4-Korpus. Kein Erzeuger lief schreibend,
keine Fixture ist geändert (`git diff --stat bc0941e5 -- eq-copilot/fixtures` leer).

### 39.7 Abweichungen vom Bauplan §6.6 und Entscheide innerhalb der Zeilen

Keine Matrixzeile geändert — die Prüfung entscheidet.

1. **M-48: die wörtliche Matrixmutation macht nicht rot (Beobachtung, Muster §36.6
   Abweichung 1).** Rotbeweisspalte: „absichtlich gebrochen, indem die Nullprüfung entfällt:
   `10·log10 (0)` ergibt `-inf` und der Vergleich wird unentscheidbar". Nach IEEE 754 ist
   `log10 (+0) = -inf` und `-inf > -60` falsch, also entscheidbar: die Stille bleibt inaktiv,
   MUT-48 endet mit Exit 0 und `[ok]` am Träger (`NAK-283-rot-M-48-beobachtung-etappe-5.txt`).
   Der Rotbeweis fällt mit MUT-48b an derselben Zusagezeile: die Nullprüfung als Freigabe
   (`gesamt >= 0.0 && (gesamt == 0.0 || …)`) — digitale Stille nimmt das Gate, Abdeckung 1,
   beide Hälften rot. Dass `log10 (0)` nicht gerechnet wird, ist von außen nicht beobachtbar;
   der Test misst die Folge (Abdeckung 0, endlich, kein Band mit Stereobasis).
2. **M-45: die Stufenentscheidung wird an `freiheitsgrade` je Band gelesen.** Die Zeile sagt
   „an den Bändern" und schließt `abdeckung` aus. `StereoBandwert::freiheitsgrade` zählt die
   Fenster der zuständigen Stufe, die das Gate genommen haben (`Stereo.h`, gezählt nach dem
   frühen `return` in `rechneFenster`, vor jeder Plausibilitätsgrenze). Evidenzbits taugen dafür
   nicht: M-47 misst bei 96 kHz 161 Bänder mit Stereobasis, aber nur 16 mit Evidenzbit — ein
   Band ohne Bit sagt über die Entscheidung seiner Stufe nichts. Die Trennung der Bänder folgt
   `trennIndex()` (Bandmitte ≥ `kTrennungHz`), im Test nachgebaut, weil die Zuordnung privat
   ist. Evidenzintervall 1 s, damit jedes gewertete Evidenzfenster mehrere Bassfenster trägt
   (Hop 8 192).
3. **R04 wandert an den Startmerker (der Bauplan nennt es nicht).** §6.6 zählt für
   `setzePassagenfenster` die bleibenden Aufgaben auf (Fenstergrenzen, Epoche, Nullen der
   Passagenmaxima, `hatSampleAusserhalb`) und schweigt zum Leeren der 3-s-Zellen (SONDE-013
   R04). Beim Binden geleert, füllt das Material bis zu einem späteren Anfang die Zellen wieder
   — derselbe Fehler wie F07 am Filter, gemessen an M-52 „3-s-Zellen" (Basislauf: 18 Rahmen mit
   3-s-Crest vor 2,9 s, erster ab 1,053 s mit 10,060 dB; MUT-52c: 27, ab 0,093 s). Deshalb
   beginnt das Kurzzeitfenster am Startmerker neu und beim Binden nur noch, wenn der Anfang
   schon zurückliegt (`passagenanfangNochVorn`) — so behält ein Binden mitten in der Passage
   das R04-Verhalten, und die Live-Fenster fallen bei einem vorab gebundenen Anfang nicht mehr
   vorzeitig (Übergabe §4 F07, Korrekturziel: „die kontinuierlichen Live-Metriken nicht durch
   einen Reset der falschen Historie verfälschen"). Die K-Filter bleiben unberührt (Übergabe §5:
   Filtervorgeschichte an einer Passage „nicht mit F07 gleichsetzen").
4. **M-51: zwei Hälften statt „Nachlauf beim Binden".** Beim Binden gibt es keinen Nachlauf
   mehr (§6.6: der Reset „entfällt hier"); er läuft am Startsample. Die Zusage „die Live-Metrik
   verliert nichts; der Nachlauf geht in `rahmenTruePeak`, nicht in `passagenTruePeakRahmen`"
   ist deshalb zweimal gemessen: (A) gebunden wird, während die Intersample-Spitze in der Kette
   steht und die Passage später beginnt — der Rahmen behält seine Spitze (Basislauf rot:
   −9,031 statt −5,920 dBTP, „ROT am Basis-SHA für die erste Hälfte"); (B) die Spitze steht
   direkt vor dem Anfang — der Nachlauf am Startsample geht in den Rahmen (MUT-51a rot) und
   nicht in die Passage (MUT-51b rot). Beide Richtungen der Rotbeweisspalte fallen je an ihrer
   Trägerzeile.
5. **M-52: die 3-s-Zellen am Passagenende sind nicht gemessen.** Die Zeile verlangt beide
   Grenzen für alle drei Historien. Am Ende tragen die 3-s-Zellen keine Passagengröße über das
   Ende hinaus: LRA nimmt nur Zellen, die ganz im Fenster lagen (`zelleImFenster`), Momentary,
   Short-term, PSR und Crest sind Live-Größen — und LRA entsteht erst ab 60 s, eine 14-s-Bühne
   kann es nicht zeigen. Gemessen sind: True-Peak-Kette am Anfang (MUT-49a) und am Ende
   (MUT-52b), Headroom an beiden Grenzen (MUT-52d: Rahmen davor und dahinter gehen ein;
   MUT-53: Ring statt Passage), 3-s-Zellen am Anfang (MUT-52c).
6. **Testzugang für M-59.** Die Zähler erreichen über die öffentliche Schnittstelle nie ihren
   Anschlag (Rahmen höchstens rund 5 000 Samples, Evidenzfenster höchstens 1 s). Die Klasse
   erklärt `FeatureEngineTestzugang` zum Freund und definiert
   `NAKAMA_FEATUREENGINE_TESTZUGANG`; der Test definiert die Struktur nur mit diesem Makro und
   meldet sonst „Testzugang fehlt" (so im Basislauf). Kein öffentlicher Setter, kein Verhalten;
   Muster: die Testzugänge der Etappe 4 (§33.1).
7. **M-60: jedes zweite Sample ersetzt.** Die Zeile nennt „eine Ersetzung im letzten Drittel"
   ohne Muster. Jedes zweite Sample zwischen 10,8 und 11,0 s der Passage: so trägt jeder der
   140 Rahmen einen True Peak über 0, `headroomFenster` ist genau 140, und die Loudnessmarke
   fällt wie bei einer vollständigen Ersetzung. Ein ganz ersetzter Rahmen hätte
   `passagenTruePeakRahmen` 0 und ginge — wie ein stiller — nicht ein (offener Punkt 3).
8. **M-58 im bestehenden M-07-Fall mit 480er-Blöcken.** Nach den 750 vergifteten
   512er-Blöcken (384 000 Samples, ein Zellen- und Rahmenrand) laufen beide Aufbauten mit
   480er-Blöcken weiter, damit jeder Rahmen auf einem Zellenrand endet und die vier Messpunkte
   exakt fallen. Der Prüftext der Gegenprobe „der Riegel ist ein Latch des Rahmens" wird zu
   „kein Dauerschweigen" — die alte Fassung wäre seit F10 falsch; der Kommentar
   `:590-596` ist auf die zweistufige Verriegelung nachgezogen (§6.6).
9. **U41-Wache ohne eigene Matrixzeile.** Der M-53/M-54-Fall fährt dieselbe Bühne einmal ohne
   Passage: 64 Rahmen, P10 = P95 = −66,021 dBTP. Das ist der User-Entscheid vom 13.09.2026
   (§38.4), keine erfundene Zusage.
10. **M-55 (b) zusätzlich gemessen.** Neben dem Riegel zählt B18 die Allokationen:
    `vorbereiten()` legt am Etappenstand 2 536 859 Byte in 68 Allokationen an, am Basis-SHA
    2 432 750 Byte in 66 — Differenz 104 109 Byte in den zwei neuen Vektoren (Histogramm
    26 000 × 4 = 104 000 Byte, Markenring 30 Byte) und dem größeren Trägerelement (79 Byte, nicht
    einzeln gemessen). 20 s einer gebundenen Passage legen danach 0 Byte an, an beiden Ständen.
11. **Beinauswahl gerechnet.** Das Beinskript bildet die Trackinglog-Auswahl aus den
    MSBuild-Logs statt aus einer Liste (Etappe 4 hielt sie als Liste) und ordnet A34 (dasselbe
    Skript wie A24, `--selbsttest`) dem Soak-Artefakt zu; Lauf 1 brach an genau dieser fehlenden
    Zuordnung vor dem ersten Bein ab.

### 39.8 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Punkt | Gemessen oder gelesen |
|---|---|
| Zahlenrand Δf | `s.fs / (double) s.punkte`: `punkte` ist 4 096 oder 16 384, `fs` nach `vorbereiten()` endlich, > 0 und ≤ 768 000 (sonst `vorbereitet = false`, `nimmBlock` kehrt vorher zurück) — Δf liegt in (0; 187,5] Hz. Halb gefüllte Fenster gibt es nicht: `schiebeStufe` rechnet erst bei vollem Ring. |
| `log10 (0)` | `gesamt > 0.0 &&` steht vor dem Logarithmus (M-48; selbst ohne sie bliebe Stille nach IEEE 754 inaktiv, MUT-48). `passageSchiebe` bekommt nur `20·log10 (x)` mit `x > 0` (`passagenTruePeakRahmen > 0.0`). |
| Histogrammgrenzen | Genommen wird nur `0 ≤ pos < 26 000` mit `pos = (db + 200) / 0,01` — fängt NaN und ±inf; sonst `nichtDarstellbar` und keine Verteilung bis zum Neuanfang der Passage, nie ein still gekappter Rand. Leeren nur über `[kleinste, groesste]`. Ränge `lo = ⌊p·(n−1)⌋`, `hi = min (lo+1, n−1)`; die Suche endet spätestens bei `groesste`, weil die Klassensumme `rahmen` ist. P10 ≤ P50 ≤ P95 folgt aus aufsteigenden Rängen und Klassenmitten — der Leserriegel `headroom_unsortiert` greift nie. Quantisierung gemessen: −6,025 gegen −6,021 dBTP (Ring), unter 0,005 dB. |
| Sättigung `uint32` | `rahmen` stoppt bei `0xFFFFFFFF` und mit ihm das Histogramm. `rahmenNichtEndlich`/`evidenzNichtEndlich` sättigen wie bisher (M-59 Hälfte 1, MUT-59a gewrappt 0xE rot); die Zellenmarke hängt an der Samplestelle (M-59 Hälfte 2, MUT-59b rot). `verarbeiteteSamples − zellenSamples` ist gegen Unterlauf geschützt. |
| Passagenanfang am Blockrand, im Blockinneren, in der Vergangenheit | Blockrand: M-73 bei 64 (48 000 = 750 × 64); die Bestandsfälle B08, R04, R05 in B16 und R03 in B23 binden am Abspielkopf — der Merker feuert bei `i = 0`, dieselbe Wirkung wie vorher (alle grün). Blockinneres: M-49, M-50 (Versatz 384), M-51 (B), M-52, M-73/M-74 bei 512 und 1 024. Vergangenheit: `b0 > startSample` lässt den Merker fallen, das Binden beginnt das Kurzzeitfenster neu, `hatSampleAusserhalb` trägt die Unvollständigkeit. `b0 + n` sättigt (M-17-Regel). Eine Grenze bricht das Fenster, der Ausschnitt wird nicht mehr ausgewertet, der Merker bleibt wirkungslos bis zum neuen Binden. |
| NaN am Analysepfad | Der Riegel ersetzt und zählt wie bisher; neu ist nur die Stelle. Eine Zelle umfasst genau die letzten `zellenSamples` verarbeiteten Samples: nach jedem Neuanfang (Grenze in `Zeit.h`, Startmerker, Binden, `zuruecksetzen`) zählt `zelleStand` von 0; verworfene Straddle-Blöcke erhöhen `verarbeiteteSamples` nicht. `grenzeZiehen` (`Zeit.h`, kein Ticketpfad) leert die Markenringe nicht — sie werden nie gelesen, bevor 4 bzw. 30 neue Zellen sie überschrieben haben (`kurzGefuellt` beginnt dort bei 0); die Zwillingsprobe G13 in B5 bleibt grün. |
| öffnen↔schließen | `setzePassagenfenster` stellt den Merker scharf und leert Headroom (beide Spannen), LRA und Fingerprint; `verarbeiteSamples` handelt Anfang (`fensterBeginntHier`) und Ende (`fensterEndetHier`) im selben Block; `loeschePassagenfenster` und `zuruecksetzen` lassen den Merker fallen und leeren den Träger (M-52). |
| aktivieren↔abklingen | Die Marke setzt die schließende Zelle, der Ring überschreibt sie — Momentary nach vier, Short-term nach 30 sauberen Zellen (M-58: 0,3 / 0,4 / 2,9 / 3,0 s exakt; MUT-58a eine Zelle länger rot, MUT-58b Rahmenlatch rot). |
| Leitung | Kein Feld wechselt Typ oder Bedeutung: `headroomFenster` bleibt `uint32` „Zahl der wirklich eingegangenen Rahmen" (`.fbs` am Basis-SHA wortgleich), die Leser in C++ und Rust sind unverändert (endlich, `fenster ≠ 0`, sortiert); B3c, A10, B10 und A4b grün (§39.4). |
| Speicher je Instanz | Gemessen (Abweichung 10): +104 109 Byte in +2 Allokationen aus `vorbereiten()`, 0 Allokationen im Lauf; `kVerteilungPlaetze` bleibt 64 (Riegel). |
| Audio-Thread | `FeatureEngine` rechnet im Analyseworker; A16 und A1 vor der Etappe (Basisbinaries) und im Endlauf bitidentisch grün. |
| Behauptung ≤ Messung | Die Prüftexte nennen den gemessenen Wert; die Regressionswachen M-48, M-55, M-59 (Sättigung) und M-73 sind als solche benannt; die Kommentare nennen nur, was ein Test misst (`Frame.h` zweistufige Verriegelung, `Vertrag.h` Köpfe). |

### 39.9 Offene Punkte dieser Etappe

Nebenbefunde, nicht gebaut, weil keine Matrixzeile sie zusagt; zur Einordnung durch die
Prüfung und den Dirigenten (nicht ins Register geschrieben).

1. **R-283-5 wörtlich reicht weiter als die Loudnesszellen.** „Das Präsenzbit einer Metrik
   bleibt gelöscht, bis ihr Fenster frei von ersetzten Samples ist." Gebaut ist es für jede
   Metrik über die 3-s-Zellen (Momentary, Short-term, PSR, Crest über 3 s, LRA-Werte). An der
   Quelle gelesen, nicht gebaut: (a) `aktivitaet` zählt die in einem Rahmen geschlossenen
   100-ms-Zellen; eine Zelle mit ersetzten Samples, die erst im nächsten sauberen Rahmen
   schließt, geht dort ein (höchstens eine Zelle). (b) Passagenmaximum und Headroomverteilung
   nehmen den Beitrag eines Rahmens mit ersetzten Samples auf (`baueFrame` schiebt vor der
   Rahmenverriegelung in `fuelleSkalare`); ein Ersatz durch Stille kann ein Maximum nur senken,
   die Verteilung bekommt aber einen Wert aus stillgelegtem Audio. Wörtlich hieße die Regel
   hier: keine Passagenverteilung bis zum Neuanfang der Passage — eine Produktwirkung. (c) Ein
   FFT-Fenster (bis 341 ms), das ersetzte Samples trägt und erst im nächsten Evidenzfenster
   schließt, geht dort mit `nichtEndlichEvidenz == 0` ein. (d) Die Verzögerungskette des
   True-Peak-Interpolators trägt ersetzte Samples zwölf Samples in den nächsten Rahmen.
2. **K-Filtervorgeschichte am Passagenanfang** bleibt wie vor NAK-283 (Übergabe §5
   ausdrücklich „nicht mit F07 gleichsetzen"); der Startmerker lässt `kL`/`kR` unberührt.
3. **Rahmen digitaler Stille in einer Passage** gehen wie bisher nicht in die
   Headroomverteilung ein (`passagenTruePeakRahmen > 0.0`); `headroomFenster` zählt dann weniger
   Rahmen, als die Passage lang ist. Das deckt U41 („die tatsächliche Zahl der eingegangenen
   Rahmen"), weicht aber von der Matrixformulierung „die Zahl der Rahmen der Passage" ab, sobald
   eine Passage Stille enthält.
4. **Quantisierung der Passagenverteilung.** Mit Passage liefern die Perzentile
   Klassenmitten (0,01 dB), ohne Passage exakte Ringwerte; gemessen −6,025 gegen −6,021 dBTP.
   Kein Leser vergleicht die zwei Spannen; festgehalten, damit ein künftiger Golden über die
   Passage nicht an der dritten Nachkommastelle scheitert.

## 40. Messung der Etappe 5, Einordnung der Abweichungen, Regeln und Dirigenten-Übergabe vor der Erstprüfung 5 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `1e232a5a` (`nakama-nak283-bc0941e-etappe5`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-5-auftrag.txt` (§38.4); Start 12:2x Uhr, Meldung 14:12 Uhr, Push 14:4x Uhr, vom Dirigenten um 15:0x Uhr gestoppt (Prozess blieb nach der Meldung idle); Beobachter (Aufsicht ENG): Start, Telemetrie gesund, dreimal einmalig „Statusquelle unbekannt" unter Bau-Last (Fehlalarme nach Skill §3.2, je selbst nachgemessen), drei HEAD-Wechsel; Stundenloop `8bbd225a` gelöscht. |
| Commits | `4679eb3b` (Produkt und Tests), `08fc9493` (§39, Rotbeweise, Beine, Riegel, Pegelrechnung — 66 Rohdateien), `32c3f297` (Kopfzeile), alle auf `origin/master`; Basis `bc0941e5` ist Vorfahr; `git status --short` zeigt nur die zwei fremden untracked Ordner (§27). |
| Diff | `git diff --stat bc0941e5..32c3f297`: fünf Analyseköpfe (`FeatureEngine.h` +262, `Frame.h` 69, `Lautheit.h` +77, `Spektrum.h` 91, `Vertrag.h` 25) und zwei Tests (`Sonde013DynamicsTest.cpp` +458, `Sonde013EventWireTest.cpp` +595), gesamt +1 519/−58; dazu §39 und Rohdateien. Fremde Pfade gemessen leer: `broker`, `plugin/{sonde,dsp,state,src}`, `schemas`, `identity`, `install`, `tools`, `TruePeak.h`. Keine Fixture geschrieben, keine Goldenverschiebung (§39 Kopf „Goldens"). |
| Rundenbilanz | `bc0941e5..32c3f297: Produkt 5 Datei(en) +470/-54 \| Tests 2 Datei(en) +1049/-4 \| Doku 66 Datei(en) +34328/-2` — vierte Produktrunde des Tickets; kumuliert (`--runden … 7781b316 32c3f297`): „kein Konvergenz-Signal". |
| Eigene Messung | Am Stand `32c3f297` um 15:0x Uhr auf den Binaries des Worker-Endlaufs (13:41 bis 13:43 Uhr, jünger als die jüngste Quelle `Vertrag.h` 13:38 Uhr): B16 `EqCopSonde013EventWireTest` 179 bestanden, 0 gescheitert; B18 `EqCopSonde013DynamicsTest` 70 bestanden, 0 gescheitert; A16 `EqCopProbeeqNullTest` 65 Prüfungen ok; A1 `EqCopNullTest` NULLTEST OK; B17 43 bestanden — alle Exit 0, identisch mit §39.4. Rotbilanz gelesen: 56 von 56 Läufen bestanden (37 Mutationsläufe aus 22 Mutationen, davon eine Beobachtung MUT-48; 18 Basisläufe am Basis-SHA, 15 rot, 3 Wachen grün; Riegel M-55 (b) am Basis-SHA), jede Rücknahme bytegleich. Beine §39.4: 41 von 41 Exit 0, A33 gehalten (83). |
| Messabdeckung | 19 Matrixzeilen (M-44 bis M-60, M-73, M-74) je mit Test und Rotbeweis; Regressionswachen M-48, M-55, M-73 als solche benannt; M-59 in beiden Hälften (Sättigung als Wache, Marke gegen die Fixfassung); Basisläufe für alle heute roten Zeilen. Keine Nebenläufigkeitszeile. Kein Fortsetzungsauftrag nötig. |
| Abweichungen §39.7 (Einordnung) | **Zehn gedeckt, eine Lücke mit Regel, kein Befund** — an Diff, Rotbilanz und Quelle gelesen. **Gedeckt:** 1 (M-48: die wörtliche Matrixmutation macht nicht rot, weil `log10 (+0) = -inf` und `-inf > -60` nach IEEE 754 entscheidbar falsch ist — die Zusage „Stille bleibt inaktiv, kein NaN, Abdeckung 0" ist gemessen; der Rotbeweis fällt mit MUT-48b an derselben Zusagezeile; Muster §36.6 Abweichung 1), 2 (M-45: Stufenentscheidung an `freiheitsgrade` je Band statt an Evidenzbits, `trennIndex()` im Test nachgebaut — Messweg innerhalb der Zeile „an den Bändern"; Prüffrage an die Erstprüfung), 4 (M-51 in zwei Hälften, beide Richtungen je an ihrer Trägerzeile rot), 5 (M-52: die 3-s-Zellen tragen am Passagenende keine Passagengröße über das Ende hinaus — `zelleImFenster` nimmt nur Zellen ganz im Fenster, LRA entsteht erst ab 60 s; die Zeile verlangt Grenzen nur für „Historien, die sie überschreiten"; die lange Bühne wird Härtung NAK-295), 6 (Testzugang M-59 unter Makro, Muster Etappe 4), 7 (M-60: jedes zweite Sample ersetzt, damit alle 140 Rahmen eingehen), 8 (M-58 mit 480er-Blöcken auf Zellenrand; Prüftext „kein Dauerschweigen"), 9 (U41-Wache ohne Matrixzeile — User-Entscheid 13.09.2026, §38.4), 10 (M-55 (b): `vorbereiten()` legt am Etappenstand 104 109 Byte mehr an — Histogramm 26 000 × 4 Byte und Markenring —, im Lauf 0 Byte in 20 s an beiden Ständen; die Zusage „die 64-Platz-Grenze der Bandverteilungen bleibt, der feste Speicher wächst nicht" meint den Ring der Bandverteilungen, das Histogramm ist der von Feinheit 7 verlangte speicherfeste Ersatz des Passagenrings; Prüffrage: keine Allokation im Lauf), 11 (Beinauswahl aus den Trackinglogs gerechnet, Mehrmessung). **Lücke mit Regel:** 3 (nächste Zeile). |
| Regel zu Abweichung 3 — M-52 präzisiert (13.09.2026) | Der Bauplan §6.6 nennt für `setzePassagenfenster` die bleibenden Aufgaben und schweigt zum Leeren der 3-s-Zellen (SONDE-013 R04, bisher beim Binden). Beim Binden geleert, füllt das Material bis zu einem späteren Passagenanfang die Zellen wieder — derselbe Fehler wie F07 am Filter (gemessen an M-52 „3-s-Zellen", Basislauf gegen MUT-52c). **Regel:** Jede Passagenhistorie — True-Peak-Kette, Headroomverteilung, 3-s-Zellen — beginnt am **Startsample** der Passage (Startmerker, Feinheit 17), nicht am Bindezeitpunkt; liegt der Anfang beim Binden bereits zurück, beginnt sie sofort (das bisherige R04-Verhalten bleibt für ein Binden mitten in der Passage; `passagenanfangNochVorn`). Die K-Filter (`kL`/`kR`) bleiben unberührt (Übergabe §5: Filtervorgeschichte „nicht mit F07 gleichsetzen"). SONDE-013 R04 gilt damit am Passagenanfang statt am Bindezeitpunkt; die Erstprüfung 5 prüft, dass die Live-Fenster (Momentary, Short-term) bei einem vorab gebundenen Anfang nicht mehr vorzeitig fallen und dass kein Kurzzeitfenster eine Passagengrenze überbrückt. |
| §39.9 offene Punkte (Einordnung) | 1 (R-283-5 reicht wörtlich weiter als die Loudnesszellen): **(b) Lücke mit Regel — Nacharbeit** (nächste Zeile); (a), (c), (d) Zahlenränder von höchstens einer 100-ms-Zelle, einem FFT-Fenster (≤ 341 ms) beziehungsweise zwölf Samples der Verzögerungskette, die je ein Rahmen später eingehen — **Härtung NAK-295**, keine Zusage verlangt sie. 2 (K-Filtervorgeschichte am Passagenanfang unberührt): bewusst, Übergabe §5; kein Punkt. 3 (Rahmen digitaler Stille zählen nicht in `headroomFenster`): gedeckt durch U41 („die tatsächliche Zahl der eingegangenen Rahmen", §38.4) und `Vertrag.h:681` („in beiden Fällen die Zahl der Rahmen, die …"); die Matrixformulierung „Zahl der Rahmen der Passage" (M-54) gilt für Passagen ohne Stille — Prüffrage, ob der Vertragskopf die Bedeutung vollständig nennt. 4 (Passagenperzentile auf 0,01 dB gerastert, ohne Passage exakte Ringwerte; −6,025 gegen −6,021 dBTP): Härtung NAK-295 (Notiz für künftige Goldens). |
| Regel zu §39.9 Punkt 1 (b) — R-283-5 präzisiert (13.09.2026) | R-283-5 sagt „das Präsenzbit einer Metrik bleibt gelöscht, bis ihr Fenster frei von ersetzten Samples ist" und ist für die fensterbasierten Loudnessmetriken über die 3-s-Zellen gebaut (M-56 bis M-60). Passagenmaximum und Headroomverteilung nehmen heute den Beitrag eines Rahmens mit ersetzten Samples auf (`baueFrame` schiebt vor der Rahmenverriegelung in `fuelleSkalare`): ein durch 0 ersetztes Sample senkt das Maximum, und die Verteilung bekommt einen Wert aus stillgelegtem Audio — derselbe Fehler wie F10 („der kontaminierte Wert reist mit"), nur ohne Präsenzbit als Träger. **Regel:** Ein Rahmen, der ersetzte Samples trägt (`rahmenNichtEndlich > 0`), geht **nicht** in Passagenmaximum und Headroomverteilung ein — wie ein Rahmen digitaler Stille (§39.9 Punkt 3); `headroomFenster` zählt ihn nicht; die Passage selbst bleibt gültig, ihre Verteilung beschreibt nur gemessenes Material. Das ist Technik innerhalb von R-283-5 (Gültigkeit hängt an der tatsächlich verwendeten Historie) und `CLAUDE.md` NaN-Ehrlichkeit; keine Produktwirkung über „ein verriegelter Rahmen fehlt in der Verteilung" hinaus. **Sie löst Nacharbeit aus** (Nacharbeit 1 zur Erstprüfung 5, zusammen mit etwaigen Defekten): eine Matrixzeile **M-76** (Zustand: 14-s-Passage, ein Rahmen mit ersetzten Samples in der Mitte; Ereignis: Perzentile am Passagenende; Zusage: `headroomFenster` = 139, Passagenmaximum und Verteilung ohne den Rahmen; Test in `Sonde013DynamicsTest.cpp`, Bein B18; Rotbeweis: die Bedingung entfernen → Fenster 140 und ein Wert aus Stille in der Verteilung) mit Rotbeweis an der Zusagezeile. |
| Register (13.09.2026, 15:1x Uhr) | **NAK-283:** Nachtrag (Etappe 5 gebaut, Einordnung, Regeln, Übergabe). **NAK-295 neu [Härtung · Analyse · P4/P5]:** die Zahlenränder aus §39.9 Punkt 1 (a), (c), (d); die lange Bühne für die 3-s-Zellen am Passagenende (LRA ≥ 60 s, §39.7 Abweichung 5); die 0,01-dB-Rasterung der Passagenperzentile (§39.9 Punkt 4). **Plan:** NAK-289 (83 clang-tidy-Fundstellen abbauen, Ziel 0) als eigener Pflegeschritt **S25g** in `docs/plan/plan.json` (Übergabe der NAK-288-Session vom 13.09.2026: „eigener verhaltensneutraler Pflegeschritt im Plan"); S31c-Text um NAK-292 ergänzt (die durch das Inventar sichtbar gewordenen Risse). |
| Hygiene des Fensters (15:1x Uhr) | MEMORY.md 5 338 B, CLAUDE.md 19 434 B, Skill 24 465 B — unverändert; `dokuriegel.py` auf Manifest, Register, Plan ohne Befund (nach dem Commit gemessen); `gesundheit.py` Exit 4 (drei Maße, NAK-292), Etappe 5 ändert Analyseköpfe unter `core/analysis` — Funktionen über 200 Zeilen und Kommentar-Bezeichner am Stand `32c3f297` gemessen (Werte im Rohlauf des Abschlusses, §40.1); A33 gehalten (83, §39 Kopf). |
| Erstprüfung 5 (vorbereitet, nicht gestartet) | Kontext dieser Session **50 %** (Telemetrie 15:05 Uhr; Skill §5: ab 500k keine neue Prüfrunde) und PC-Neustart des Users nach diesem Abschluss (User-Wort 13.09.2026: „wenn der nächste worker fertig ist und du einen klaren abschluss machst, dann stopppen wir kurz und ich starte den pc neu"). Der Prüfauftrag liegt **fertig** als `docs/beweise/roh/NAK-283-etappe-5-erstpruefung-auftrag.txt`: Vorlage A, Prüfbereich `git diff bc0941e5...32c3f297 -- <die sieben Ticketpfade>`, Gate wörtlich, Matrix §5.4, Riegelzeilen aus §39.2, Rotbeweise und Beine, die elf Abweichungen mit dieser Einordnung, beide Regeln, Ausschlüsse, Prüffragen (a) bis (f). |
| Übergabe | Nächste Session (nach dem PC-Neustart über `tools/dirigent/start-dirigent.ps1`, `/dirigent`): `git pull --ff-only`; Anker `pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Anker -Ticket NAK-283` neu setzen; HEAD muss `32c3f297` plus die Commits dieses Abschlusses sein; dann sofort `pwsh -NoProfile -File tools/dirigent/codex-lauf.ps1 -Kennung nak283-etappe5-erst -Prompt <Auftrag> -HeadSha <HEAD>` (Astra max), Monitor auf das Startlog bis `EXIT=` (Prozessprüfung über `Get-CimInstance Win32_Process`, nicht `Get-Process` — kein `CommandLine`-Feld), §41 „Erstprüfung 5" mit Thread-ID und Urteil; danach Validierung der Befunde durch einen lesenden Opus-Agenten, Einordnung, **Nacharbeit 1** (bestätigte Defekte plus die Lückenregel zu §39.9 Punkt 1 (b) als M-76, Aufsicht ENG) oder — ohne Defekt — Nacharbeit 1 nur mit M-76; Wiederprüfung (Vorlage B); dann Abschluss des Tickets: Abschlussprüfung `39e18d3f...HEAD` (Vorlage A über die Ticketpfade aller Etappen 2 bis 6 und `tools/beweise.ps1`), voller Kanon auf dem End-Stand, Laufzeit-Arm (`tools/fl/laufzeit.ps1 -Ticket NAK-283 -Basis 39e18d3f`; erster Lauf mit Installation, S25e), Urteil T2, Planstand. Alle Session-Loops und Beobachter sind beendet (`CronList` leer), kein Worker läuft, Worker `1e232a5a` entfernt; der Ticketanker lebt nur in der Session und ist neu zu setzen. Offene Karten: U11, U13, U24, U25, U26, U40 (vertagt). |

### 40.1 Gesundheitsmaß am Stand `32c3f297` (Dirigent, 13.09.2026, 15:1x Uhr)

`py -3.13 tools/plan/gesundheit.py` Exit 4: Quelldateien über 2 000 Zeilen 1 von 0 (`NakamaState.cpp`, NAK-292), Funktionen über 200 Zeilen 35 von 28 (unverändert gegen §36.5), Kommentar-Bezeichner ohne Code 36 von 30 (unverändert); Quelldateien über dem Ziel 1 500 Zeilen 9 (gegen 8 vor der Etappe — `FeatureEngine.h` ist mit +262 Zeilen über das Ziel gewachsen; Ziel, keine Grenze; Zuschnitt gehört zu S31c). MEMORY.md 5 338 B, CLAUDE.md 19 434 B, Dirigenten-Skill 24 465 B, keine Indexzeile über 250 Zeichen, keine Memory-Datei ohne Indexlink — alles OK. Kein neuer Grenzriss aus Etappe 5; A33 (`tidy.py`) im Worker-Endlauf gehalten (83, §39 Kopf).

---## Kanon-Lauf - NAK-283

**Lauf:** 2026-09-13 05:34 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 66/66 Kanon-Laeufe bestanden | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/NAK-283-e6771fc-dirty.md](roh/NAK-283-e6771fc-dirty.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-13 05:34:15 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | e6771fc9 Planstand nach NAK-283 Etappe 6 (Erstpruefung 6 PASS) neu gerechnet |
| Commit (voll) | e6771fc927a130360a80b075b37afac4ab9715d4 |
| Arbeitsbaum | 2 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.6 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,21 s | [A1](roh/NAK-283-e6771fc-dirty.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,48 s | [A2](roh/NAK-283-e6771fc-dirty.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,50 s | [A3](roh/NAK-283-e6771fc-dirty.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. Seit Etappe F dazu die Proposal-Policy: ein Vorschlag entsteht MIT seinem Befund und traegt die fuenfzehn Felder aus 42.1 plus revert, der Rueckweg ist ein FELD mit drei Werten (dsp_revert faellt), in P5 ist jede Aktion manual, keine Aenderung und mehr Daten sind vollstaendige Objekte, der Zielbereich kommt aus dem Band des Befunds statt aus dem groessten Banddelta, ein geschuetztes Band ist eine HARTE Constraint mit Gegenprobe, ein stop_if auf einem nicht messbaren Guardrail meldet MORE DATA, und ueber fuenfhundert zufaellige Eingaenge wird kein Hard Cap und keine engere Usergrenze ueberschritten. Der Guardrail-Rechner LIEST seit E-05 den Zielbereich aus experiment_begin.ziel; ohne ihn bleibt die Heuristik und das Resultat traegt ziel_geraten - beide Pfade mit verschiedener Zahl gemessen. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1.183,17 s | [A4](roh/NAK-283-e6771fc-dirty.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 13,19 s | [A4-SI](roh/NAK-283-e6771fc-dirty.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 66,21 s | [A4b](roh/NAK-283-e6771fc-dirty.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,72 s | [A5](roh/NAK-283-e6771fc-dirty.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,32 s | [A6](roh/NAK-283-e6771fc-dirty.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/NAK-283-e6771fc-dirty.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,37 s | [A8](roh/NAK-283-e6771fc-dirty.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID, keine ist verbrannt oder wiederverwendet, und jedes Offsetfeld steht im Rust-Strukturriegel. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,56 s | [A9](roh/NAK-283-e6771fc-dirty.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen, integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) und seit SONDE-015 band_dynamic_gain_db ID 22 (acht Werte, leerer Vektor, Altsender ohne Feld, vier falsche Laengen, NaN und Inf) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 5,13 s | [A10](roh/NAK-283-e6771fc-dirty.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,26 s | [A11](roh/NAK-283-e6771fc-dirty.md#a11) |
| A12 | Parameterbestand Layout v2 (120 Kennungen, 112 Host-Parameter als Praefix) haelt den Vertrag, und die 109 v1-Kennungen sind aus der eingefrorenen v1-Datei woertlich abgeleitet; Zonen-, Preset- und Ablehnungsregeln stimmen mit dem Vertrag ueberein; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State- und Presetkorpus samt MANIFEST bytegleich zur Neuerzeugung, keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A12](roh/NAK-283-e6771fc-dirty.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,23 s | [A13](roh/NAK-283-e6771fc-dirty.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 8,26 s | [A14](roh/NAK-283-e6771fc-dirty.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,11 s | [A17](roh/NAK-283-e6771fc-dirty.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 34,90 s | [A18](roh/NAK-283-e6771fc-dirty.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [A19](roh/NAK-283-e6771fc-dirty.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,34 s | [A20](roh/NAK-283-e6771fc-dirty.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,34 s | [A21](roh/NAK-283-e6771fc-dirty.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,45 s | [A22](roh/NAK-283-e6771fc-dirty.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. Eine fehlende Messung ist kein PASS (NAK-283 F13): jeder Speicherpunkt traegt das Gueltigkeitsmerkmal seiner Windows-Abfrage, und fehlt ausserhalb der Neustartfenster eine gueltige Messung, endet S07 mit dem eigenen Status MESSUNG FEHLT (Exit 3) statt im Budget. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,54 s | [A24](roh/NAK-283-e6771fc-dirty.md#a24) |
| A34 | Der Selbsttest des Soak-Orakels (NAK-283 F13, Muster A27). Er laeuft OHNE Repo-Fixture und ohne Lauf: vier Faelle bauen ihre Berichte im Speicher und laufen durch dieselben Funktionen wie A24 - Messstelle, Speicherpunkt, Urteil. Eine fehlende RSS-Messung endet mit MESSUNG FEHLT und Exit 3, nie im Budget; ein nicht abfragbarer Prozess und ein Prozess mit 0 Bytes Working Set sind im Bericht verschieden; ein Fehlerpunkt direkt vor oder nach dem Neustartfenster faellt, obwohl das Fenster die Kurve filtert; ein Altbericht ohne Gueltigkeitsmerkmal bleibt ueber --bericht auswertbar, derselbe Bericht als Livelauf ist rot. Jede Erwartung laeuft mit ihrem Gegenteil. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --selbsttest` | [OK] Exit 0 | 0,16 s | [A34](roh/NAK-283-e6771fc-dirty.md#a34) |
| A28 | Der P5-Evaluationskorpus ist reproduzierbar (Muster A25): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt SHA-256 im Manifest, und eine verwaiste Datei faellt. Zusaetzlich der Hygieneriegel aus NAK-182 R2: der Bezeichner des Kettenbeins steht WOERTLICH in seiner Datei, sonst waere der Name ein Etikett. | `py -3.13 tools\eq-copilot\erzeuge_p5_korpus.py --pruefen` | [OK] Exit 0 | 0,13 s | [A28](roh/NAK-283-e6771fc-dirty.md#a28) |
| A29 | Das Sammelbein des P5-Exit-Gates (59 Punkt 6, 36.4, M-64 bis M-70, R2). Es ist das dritte Glied einer KETTE: der Korpus traegt die Wahrheit, broker/tests/sonde014_p5_korpus.rs faehrt jede Sitzung durch p1 und schreibt die TATSAECHLICH ausgegebene Hypothese, und dieses Bein haelt beides gegeneinander - eine falsche starke Produktbehauptung aendert den Korpus nicht, sie faellt am Vergleich. Precision und Recall stehen JE URSACHENKLASSE, dazu Brier, Kalibrierung, Coverage und Enthaltungsrate; die vier Riegel (falsche_starke, falsche_schwache, precision und recall in [0,1]) laufen je Klasse und ueber die Gesamtmenge. Die Schwelle aus M-31 ist AUSGABE: gesucht wird die niedrigste Sicherheitsstufe, deren Riegel halten. Der Startwert von GATE_MINDEST_FENSTER wird an den zwei Passagensitzungen kalibriert. Ohne frische Ergebnisdatei meldet das Bein Voraussetzung-fehlt (Exit 3) statt gruen. Die Frischepruefung zaehlt das Rechenmodul broker/src/coordinator/hypothese/ zur LAUFZEIT auf (rekursiv), und eine genannte, aber fehlende Quelle ist ebenfalls Exit 3 mit Nennung des Pfads - kein stilles Ueberspringen (NAK-224 D1). | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py` | [OK] Exit 0 | 0,13 s | [A29](roh/NAK-283-e6771fc-dirty.md#a29) |
| A30 | Der Selbsttest des P5-Sammelbeins (M-68, Muster A27). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch DIESELBEN Funktionen, die das Sammelbein fuehrt. Jede Erwartung laeuft mit ihrem GEGENTEIL - eine falsche starke Behauptung und die richtige daneben, eine Enthaltung und dieselbe Menge ohne sie, ein leerer Satz und ein voller. Die vier Riegel werden synthetisch gefuettert und muessen fallen: precision > 1, recall > 1 und brier > 1 sind ueber den Korpusweg strukturell unerreichbar, und eine Wache ohne ausfuehrbaren Negativtest ist keine gemessene Zusage. Zuletzt die Schwellensuche in beide Richtungen. | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py --selbsttest` | [OK] Exit 0 | 0,12 s | [A30](roh/NAK-283-e6771fc-dirty.md#a30) |
| A31 | Die adversarialen Gegenbeispiele des Phasengates G5 am PRODUKTPFAD - genau die Faelle, die das Korpusformat nicht ausdruecken kann. Eine Intent-Rolle trennt zwei messtechnisch identische Kandidaten NICHT, auch nicht bei rolle=fuehrt, und bei drei sichtbaren Kandidaten wird der Fuehrende gegen JEDEN geprueft, nicht nur gegen den Zweitplatzierten. Eine Quelle, die ihre Energie in einem ganz anderen Band traegt als der Masterbefund, erreicht keine starke Aussage. Zwei Kandidaten, deren Raenge sich nur durch ein Band WEIT ausserhalb des Befundintervalls unterscheiden, sind ungetrennt. Umklammernde und disjunkte Fensterspannen fallen am Alignment mit Grund, obwohl die Ueberdeckung ihrer SPANNEN 1,0 meldet. Und die Passage laeuft ueber den echten Produktpfad samt Store: zwoelf und acht Fensterlaengen tragen die Aussage, sieben nicht, und eine Passage ohne ein einziges Masterfenster ergibt eine ENTHALTUNG mit ungueltiger Beobachtung statt Schweigen (M-27). Seit der Nacharbeit 1 (07.09.2026) faehrt das Bein ALLE 15 vereinbarten Ketteneingaben durch denselben Produktpfad und DRUCKT je Eingabe ihr Ergebnis: vier Passagenrandwerte (12, 11, 8 und 7 Fensterlaengen), den zweiten experiment_begin mit gleicher passage_id, zwei Master, den Master ohne Fenster, den Kanalwechsel vor und nach dem Deskriptorwechsel, drei Sonden auf einem Kanal, die antikorrelierte Quelle allein und neben der korrelierten, dazu die Masteranomalie in einem Fenster HINTER der Passage. Seit NAK-213 (08.09.2026) traegt KEINE der sieben frueher gedruckten NAK-213-Luecken mehr eine Luecke: zwei Master ergeben eine Enthaltung OHNE ORT statt eines stillen Gewinners, ein Master ohne Fenster ebenfalls statt Schweigen, der Kanalwechsel nimmt die Belege GENAU DER wechselnden Quelle zurueck und entfernt ihren Befund, drei Sonden auf einem Kanal werden ueber ALLE Quellen der Sitzung erkannt - eine vierte, stumme Sonde desselben Kanals setzt die Duplikatmarke mit. Seit NAK-214 (08.09.2026) fahren drei Faelle dieses Tickets mit: zwei bestaetigte Mains erzeugen KEINEN Vorschlag (Vorschlaege 0, keine proposal-Zeile im Store), ein PRE/POST-Paar unterhalb der Alignmentschwelle bleibt Aussageklasse 1 mit next_test prepost_paar_messen, und eine zurueckgenommene juengste Passage laesst die AELTERE rechnen - jeder druckt sein Ergebnis. Keine Luecke bleibt gedruckt, und der aufruferlose Helfer `luecke()` ist mit seinem letzten Aufrufer gegangen. | `cargo test --manifest-path broker/Cargo.toml --test sonde014_gegenbeispiele --color never -- --nocapture --test-threads=1` | [OK] Exit 0 | 104,23 s | [A31](roh/NAK-283-e6771fc-dirty.md#a31) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,14 s | [A25](roh/NAK-283-e6771fc-dirty.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,14 s | [A26](roh/NAK-283-e6771fc-dirty.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/NAK-283-e6771fc-dirty.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,71 s | [A23](roh/NAK-283-e6771fc-dirty.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc) mit dem aktiven DSP-Kern: im Default (eq_enabled aus) bitgleich ueber 1000 Bloecke von 1 bis 2048 Samples bei 44,1 / 48 / 96 / 192 kHz ohne belegte Bank; eq_enabled an mit bypass aus und sonst neutralen Defaults bitgleich; eq_enabled an mit bypass an bitgleich trotz hoerbarem Band dahinter; nach eq_enabled aus nach dem Fade wieder bitgleich; Mix 0 mit Output-Trim 0 dB bitgleich; der Passthrough sanitisiert kein NaN/Inf; 0 Samples Latenz und kein Tail im Passthrough und im Hard-Bypass; speichern-laden-speichern bytegleich im Layout v2 mit Kind Dsp. Das Bundle meldet 112 Host-Parameter, und sein Bundlevertrag laesst nur active_probe zu. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,37 s | [A16](roh/NAK-283-e6771fc-dirty.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,16 s | [B1](roh/NAK-283-e6771fc-dirty.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. Seit SONDE-015 zusaetzlich das Kind Dsp: 120 Werte bit-exakt durch den Roundtrip, flache Arrays statt Kindknoten, voller Undo-Ring aus 32 Schnappschuessen bytegleich und unter 16 MiB, der 33. Eintrag read-only; die Layoutmigration v1 nach v2 setzt occupied bitgenau aus enabled und den Werten; das Preset traegt zwei getrennte Versionen, weist jedes der sechs verbotenen Identitaetsfelder ab, ignoriert unbekannte Top-Level-Felder und laesst eq_enabled beim Laden unberuehrt. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,24 s | [B2](roh/NAK-283-e6771fc-dirty.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,07 s | [B3](roh/NAK-283-e6771fc-dirty.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,23 s | [B3b](roh/NAK-283-e6771fc-dirty.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. Seit SONDE-015 zusaetzlich der Fassungsschritt P1 5: der aus der committeten Fassung zurueckgebaute Leser der Fassung 4 lehnt state_report.dsp ab, ein Bericht ohne dsp bleibt in beiden Fassungen gueltig, und der Empfaenger rechnet SHA-256(dsp.jcs) gegen state_hash nach - ein schemagueltiger Bericht mit abweichendem Hash wird GANZ abgewiesen. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,43 s | [B3c](roh/NAK-283-e6771fc-dirty.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,53 s | [B4](roh/NAK-283-e6771fc-dirty.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/NAK-283-e6771fc-dirty.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,22 s | [B5](roh/NAK-283-e6771fc-dirty.md#b5) |
| B6 | Aktiver DSP-Kern (plugin/dsp/) als Bibliothek: alle sechs RBJ-Filtertypen liegen ueber einem 1/24-Oktav-Gitter von 20 Hz bis min(20 kHz, 0,45 fs) bei 44,1 / 48 / 96 / 192 kHz innerhalb 0,05 dB typisch und 0,1 dB an den Raendern an der analytischen Antwort - gemessen als Impulsantwort AM TAP post_committed, also durch den Audiopfad, gegen eine im Test eigenstaendig ausgeschriebene Formel. Ausgeschaltet und hard-bypassed wird bei jeder Hoermatrix-Auswahl kein Sample geschrieben (bitgleich); bei Mix 0 mit Output-Trim 0 dB ist der Ausgang wertgleich zum Eingang; nach dem Ausschaltfade ist der Passthrough wieder bitgleich, und er sanitisiert kein NaN. Die acht Slots wirken als Kaskade von Slot 0 nach 7 gegen ein unabhaengig gerechnetes geordnetes Golden, das ein Rueckwaertslauf reisst. Auto-Gain ergibt bei flacher Kurve exakt 0 dB und bei einem +6-dB-Shelf ueber dem Gitter -6 dB, ein identisches Mid/Side-Paar faellt exakt auf den Stereo-Fall zusammen, und die Dynamik bewegt ihn nicht. Der Bankpool faehrt jeden der sechs Ownership-Uebergaenge einzeln, gibt busy_retry statt eine aktive Bank zu verdraengen, wird erst nach dem Audio-ACK frei und haelt seine Invarianten unter zwei echten Threads ueber tausende Bloecke; im Callback zaehlt der thread-lokale Zaehler ueber 4000 Bloecke wechselnder Groesse 0 Allokationen und 0 Sperren, waehrend derselbe Zaehler die Allokationen des Workers NICHT sieht. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [OK] Exit 0 | 4,86 s | [B6](roh/NAK-283-e6771fc-dirty.md#b6) |
| B7 | Lokaler Transaktionskern der aktiven Sonde mit dem echten DSP-Kern: die Falltabelle T1-T17 aus Manifest SONDE-015 §5.11.4 laeuft tabellengetrieben mit den Invarianten I1, I2 und I4 als Wachen nach jeder Eingabe; ein Fehler in jeder Stufe S1-S7 laesst bestaetigten Zustand, Hash, Undo-Ring und Register unveraendert, und weder der Nachschlag S0 noch der Commit-Punkt allozieren; Apply, Revert, Neutralisieren, Remove, Undo, Redo und Preset-Laden erzeugen je genau eine Revision, 10.000 doppelte, vertauschte und veraltete Eingaben einer Transaktion hoechstens eine. Belegung, Remove und Undo als ein Objekt, Schutz-Zonen mit Verletzungsmeldung, Undo-Ring der Tiefe 32 und Preset halten ihre Matrixzeilen. Am echten SondeProcessor: 112 Host-Parameter in Vertragsreihenfolge ohne occupied; Hostautomation ohne Revision mit zwei Epochenwechseln je Geste, auf einem freien Slot bitgleich im Klang; ein abgeschlossener Gestus ist eine Revision; Host-Dirty nur bei einem Commit; das Kind Dsp reist durch Speichern und Laden, und ein Reload rekonstruiert Hash und Ausgang. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [OK] Exit 0 | 0,37 s | [B7](roh/NAK-283-e6771fc-dirty.md#b7) |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 220,54 s | [B10](roh/NAK-283-e6771fc-dirty.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,04 s | [B11](roh/NAK-283-e6771fc-dirty.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,60 s | [B12](roh/NAK-283-e6771fc-dirty.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/NAK-283-e6771fc-dirty.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 8,39 s | [B14](roh/NAK-283-e6771fc-dirty.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,16 s | [B15](roh/NAK-283-e6771fc-dirty.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/NAK-283-e6771fc-dirty.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,86 s | [B22](roh/NAK-283-e6771fc-dirty.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,69 s | [B23](roh/NAK-283-e6771fc-dirty.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 0,97 s | [B24](roh/NAK-283-e6771fc-dirty.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/NAK-283-e6771fc-dirty.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,47 s | [B26](roh/NAK-283-e6771fc-dirty.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/NAK-283-e6771fc-dirty.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,76 s | [B19](roh/NAK-283-e6771fc-dirty.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,29 s | [B17](roh/NAK-283-e6771fc-dirty.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 3,89 s | [B18](roh/NAK-283-e6771fc-dirty.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,51 s | [B16](roh/NAK-283-e6771fc-dirty.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,41 s | [B27](roh/NAK-283-e6771fc-dirty.md#b27) |
| B29 | Die Zustandsmaschine des AssistantStep im Main (46.1, E-07/E-08, M-55 bis M-62). Die Zustandsmenge hat ACHT Werte, jeder kommt ueber seinen eigenen Rueckweg zurueck, ein neunter faellt. Nach preview fuehrt KEINE Kante - weder hin noch zurueck -, ein gespeicherter preview-Schritt ist ein Lesefehler statt eines stillen Sprungs, und die Produkt-API setzt ihn nicht; die Gegenprobe mit proposal laedt normal. Jeder der acht Zustaende traegt fuenf Angaben (Eintritt, Evidenz, Useraktion, Timeout ueber 0, sichere Rueckkante), auch preview - der erste Zustand ist seine eigene Rueckkante. Der Deckel ist STRUKTURELL: ein zweiter Startversuch bei offenem Schritt wird abgewiesen, nicht eingereiht, und erst nach dem terminalen Abbruch beginnt ein neuer. Die vier Gegenpfade laufen einzeln: Zurueck geht auf die Rueckkante, Ueberspringen auf den naechsten Zustand, Resume ist eine FRAGE ohne Revisionssprung, und Verwerfen ist terminal statt geloescht - danach gibt es kein Resume mehr. Die drei benannten Ergebnisse aus 46.2 sind eigene Ergebnisse mit Objekt; dasselbe zweimal hebt keine Revision. Die harten Gates greifen VOR der Gewichtung: drei Kandidaten mit perfektem Rang und je einem gerissenen Gate verlassen die Liste, ein bescheidener bleibt. Verdrahtet am echten Prozessor mit Host-Dirty genau einmal je echter Aenderung, Rekonstruktion aus dem gespeicherten MainProject und bytegleichem Save/Load; jede Grenze faellt von beiden Seiten, und ein Altstand ohne die Eigenschaft laedt normal und schreibt sie nicht. | `eq-copilot\build\plugin\EqCopSonde014AssistentTest_artefacts\Release\EqCopSonde014AssistentTest.exe` | [OK] Exit 0 | 0,84 s | [B29](roh/NAK-283-e6771fc-dirty.md#b29) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,22 s | [B28](roh/NAK-283-e6771fc-dirty.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,29 s | [B8](roh/NAK-283-e6771fc-dirty.md#b8) |
| A32 | Codebase- und Kontextgesundheit gegen die Schwellen aus Plan S25b und dem Kontext-Hygiene-Playbook: Zeilen je Quelldatei (Grenze 2 000, Ziel 1 500, `broker/src/generiert/` ausgenommen), Anzahl Funktionen ueber 200 Zeilen, aufruferlose allow(dead_code)-Helfer, Backtick-Bezeichner in Kommentaren ohne Entsprechung im Repo, sowie Bytes von MEMORY.md, CLAUDE.md und Dirigenten-Skill, Index-Zeilen ueber 250 Zeichen und Memory-Dateien ohne Index-Link. Die Ratschen (Funktionen 28, Kommentar-Bezeichner 30, clippy 91) stehen auf dem Iststand vom 09.09.2026 nach NAK-225 und reissen bei der ersten Verschlechterung; sie werden nur gesenkt, nie erhoeht. Die Zeilengrenze HAELT seit NAK-225 (S25d): keine Quelldatei liegt mehr ueber 2 000 Zeilen, und die Pflegeticket-Zuordnung ist leer. Jeder neue Treffer meldet sich als OHNE PFLEGETICKET; ein Zuordnungseintrag, dessen Datei die Grenze nicht mehr reisst, beendet den Lauf als WERKZEUGFEHLER (Exit 2), damit die Liste nicht still veraltet. Vorangestellt laeuft der Selbsttest: 121 Faelle im Speicher durch dieselben Funktionen, jede Erwartung mit ihrem Gegenteil (Zahlenraender 2 000/2 001, 200/201, 250/251, CRLF, BOM, fehlender Schlussumbruch, Rust-Lebensdauer und C++-Ziffernstrenner gegen die Literalmaskierung). Der clippy-Teil laeuft NICHT mit (Schalter --clippy, er baut) und meldet sich als nicht messbar. Nicht blockierend: Exit 4 ist ein Hinweis. Exit 2 ist ROT und heisst, dass das WERKZEUG nicht mehr misst - roter Selbsttest oder ein fehlender Messort, denn ein verschobener Pfad wuerde sonst still 0 messen und die halbe Codebase unbemerkt aus dem Mass nehmen. | `py -3.13 tools\plan\gesundheit.py --mit-selbsttest` | [HINWEIS] Exit 4 - Befund, nicht blockierend (siehe Rohausgabe) | 13,99 s | [A32](roh/NAK-283-e6771fc-dirty.md#a32) |
| A33 | clang-tidy ueber alle Uebersetzungseinheiten von eq-copilot/plugin ohne tests/ und ohne den flatc-Codegen vertrag/generiert/ (Bein A9 haelt ihn bytegleich; Regelsatz eq-copilot/plugin/.clang-tidy: bugprone, clang-analyzer, concurrency, performance sowie eine cert-, cppcoreguidelines- und misc-Auswahl; keine Stilregeln, Ausnahmen dort mit Grund) in einem eigenen Ninja-Baum eq-copilot/build-tidy mit denselben cl.exe-Schaltern wie der Produktbau. Gezaehlt werden eindeutige Fundstellen (Datei, Zeile, Spalte, Check) gegen die Ratsche in tools/plan/tidy.py: Grenze = Iststand der Erstmessung NAK-288, Ziel 0, nur gesenkt, nie erhoeht; eine Aenderung des Regelsatzes zieht die Ratsche im selben Commit nach. Vorangestellt der Selbsttest (Einordnung in den Quellort, Datenbankfilter, Diagnoseparser, Deduplizierung, Ratschenrand, Umgebungsleser). Nicht blockierend: Exit 4 ist ein Hinweis. ROT ist Exit 2: eine Uebersetzungseinheit mit error-Diagnose oder ein abgestuerzter clang-tidy heisst NICHT gemessen, denn eine halb geparste Einheit meldet weniger Funde und saehe wie Fortschritt aus. FEHLT (Exit 3) ohne Visual Studio 2022 mit C++-Werkzeugen und Clang-Komponente, cmake oder ninja. | `py -3.13 tools\plan\tidy.py --mit-selbsttest` | [OK] Exit 0 | 92,73 s | [A33](roh/NAK-283-e6771fc-dirty.md#a33) |


---

## 41. Erstprüfung 5 — Urteil, Einordnung, M-76 und Auftrag der Nacharbeit 1 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Lauf | `tools/dirigent/codex-lauf.ps1 -Kennung nak283-etappe5-erst`, Codex `gpt-6-astra`, Effort **max**, Sandbox read-only, Prompt `docs/beweise/roh/NAK-283-etappe-5-erstpruefung-auftrag.txt` (Vorlage A, §40). Start 18:20:23 Uhr, Ende 18:28:35 Uhr, Exit 0; HEAD vorher und nachher `0989d738` (`bf5b503a` plus Planstandcommit), Worktree unverändert (nur die zwei fremden untracked Ordner, §27). Thread `01a09b91-e561-7e02-9c2b-4280e3610498`. Monitor und Stundenloop `da386651` beendet, `CronList` leer. Session neu nach dem PC-Neustart (§40 Übergabe), Anker NAK-283 gesetzt, Kontext beim Start 10 %. |
| Urteil | **PASS — kein DEFEKT im gebundenen Prüfbereich.** Wörtlich in `docs/beweise/roh/NAK-283-erstpruefung-5-0989d73.txt`. Geprüft (laut Prüfer): der vollständige Sieben-Dateien-Diff, Gate-Einheiten und Bühnenpegel, Start-/Endgrenzen, Histogramm samt Sättigung, getrennte Loudnessfristen, M-60, Worker-/Audiothread-Abgrenzung, Writer/Reader; 56 Rotbeweisdateien mit passenden Zusageprüfungen und 133 übereinstimmenden protokollierten Hashpaaren, sieben Quellhashes passend zum Endlauf, ausgeschlossene Produktpfade und Fixtures unverändert. Nicht geprüft: keine eigenen Bau-, Test-, Mutations- oder Kanonläufe (Leseauftrag); B16/B18, Nulltests, Goldens und A10 als gelesene Rohbelege. Keine Lücke, keine Härtung benannt. |
| Prüffrage (f) | Bestätigt, wörtlich: „`Spektrum.h:200–211,268–270` übernimmt ersetztes Material in den Rahmenbeitrag, `Frame.h:112–122` sammelt ihn vor der Verriegelung `Frame.h:569–570`" — der Pfad der Lückenregel zu §39.9 Punkt 1 (b) (§40) ist an der Quelle belegt; die angeordnete Nacharbeit bleibt separat. |
| Einordnung | Kein Befund, also keine Validierung durch einen lesenden Opus-Agenten; die Einordnung der elf Abweichungen (§40) hält. Etappe 5 ist erstgeprüft PASS mit null Defekten; offen bleibt allein die vom Dirigenten angeordnete Nacharbeit 1 (M-76). Runde 1 von 3 wird damit nicht von einem Defekt, sondern von der Lückenregel ausgelöst. |
| M-76 eingetragen | Matrix §5.4 um **M-76** ergänzt (Zustand, Ereignis, Zusage, Reihenfolge, Test, Rotbeweis, Quelle: Regel §40); §5-Überschrift und §5.6-Zählung nachgezogen (76 Zeilen, 56 Baulücken, 47 heute rot); Gate-Kopfzeile „M-01 bis M-76". Die Zusage ist bewusst an **zwei** Größen messbar (`headroomFenster` 139 statt 140; Passagenmaximum rund −6,02 statt −0,9 dBTP), damit der Rotbeweis an `Frame.h:112` nicht an der Robustheit der Perzentile gegen einen einzelnen Ausreißer vorbeigeht; ein vollständig ersetzter Rahmen trägt heute schon nicht bei (`passagenTruePeakRahmen > 0.0`), deshalb trägt der Testrahmen endliche Samples mit eigenem, lauterem Beitrag. |
| Nacharbeit 1 | Auftrag `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-auftrag.txt` (nur M-76; Ticketpfade `Frame.h` und `Sonde013DynamicsTest.cpp`, Manifest §42, Rohdateien `NAK-283-etappe-5-nacharbeit-1-*` und `NAK-283-rot-M-76-etappe-5-nacharbeit-1*.txt`; Beine B18 als Ziel, dazu B16, B17, B19, B20, B22, B23, B24, B25, B9, B5, A10, B3c, A26, A16, A1, A33; kein Kanon, kein Laufzeit-Arm — beides am Ticketende, §6.8). Worker: frischer Opus max, dontAsk, Aufsicht **ENG** (Analysekern). Basis-SHA der Nacharbeit = HEAD nach dem Commit dieses Abschnitts (im Workernamen und in §42). Danach Wiederprüfung 1 (Vorlage B, nur der Fixdiff: M-76 geschlossen, nichts gebrochen). |
| Hygiene des Fensters (18:3x Uhr) | MEMORY.md 5 338 B, CLAUDE.md 19 434 B, Skill 24 465 B (Grenzen 22 528 / 20 480 / 24 576 B) — unverändert gegen §40, kein Riss; `dokuriegel.py` auf dem Manifest nach diesem Eintrag ohne Befund (vor dem Commit gemessen); `gesundheit.py` und `tidy.py` im Abschlussfenster der Nacharbeit 1 (A33 fährt der Bauer). |

## 42. Nacharbeit 1 der Etappe 5 (Bauer, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Runde | **Nacharbeit 1 (Runde 1 von 3) der Etappe 5** mit genau einem Punkt: **M-76**, die Lückenregel zu §39.9 Punkt 1 (b) (§40), Matrixzeile §5.4. Auftrag `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-auftrag.txt` (§41). Keine Matrixzeile geändert, keine Produktfrage beantwortet. |
| Basis-SHA | `d8c3f74c656bb88a08999edbf6705440a4f2e71a` (= `git rev-parse HEAD` beim Start); `git status --short` beim Start: nur `?? briefing-hub/` und `?? nimbalyst-local/` (§27) — nicht angefasst, nicht committet. |
| Commits | `afef1077` (Produkt und Test: `Frame.h`, `Vertrag.h`, `Sonde013DynamicsTest.cpp`, +190/−8). Dieser Abschnitt mit Rot- und Beinskript und den Rohdateien sowie die Kopfzeile „Etappe" folgen in eigenen Commits, die nur `docs/beweise/` tragen. |
| M-76 | **Gebaut und gemessen.** Test `rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein` (B18): in der Bühne `headroomFenster` **139** (Gegenlauf 140), Passagenmaximum **−6,021 dBTP**, P10/P50/P95 −66,025/−6,025/−6,025 dBTP wie im Gegenlauf, größte Klasse der Verteilung −6,025 dBTP, Passage intakt. B18 im Endlauf 77 bestanden, 0 gescheitert (Etappe 5: 70). |
| Rotbeweis | **MUT-76 an `Frame.h:122`** (die Bedingung `rahmenNichtEndlich == 0` entfernt): mutiert Exit 1 und rot an **Fenster (140) und Maximum (−0,915 dBTP)**, dazu an der Verteilung (größte Klasse −0,915 dBTP); zurückgespielt Exit 0 und grün; `Frame.h` SHA-256 vorher = nachher. **Basislauf am Basis-SHA** rot an denselben drei Trägern mit denselben Werten. Rotbilanz **19 von 19 Läufen bestanden**, darunter die vier Prüfungsbrüche PB-76a bis d und die Neufahrten an `Frame.h` und M-60 (§42.3). |
| Beine | **41 von 41 Beinen Exit 0** im Endlauf nach dem vollen Rotskript (Lauf 1 davor ebenfalls 41 von 41): die 16 des Auftrags mit A16 und A1 als Wache, 19 weitere C++-Beine nach Trackinglog, 6 Python-Beine an neu gebauten Artefakten. Release-Bau von 42 Zielen Exit 0, keine `warning C` auf einer geänderten Zeile; Goldens unverändert grün, A10 bytegleich (§42.5). |
| A33 | `py -3.13 tools/plan/tidy.py --mit-selbsttest --jobs 3` nach dem Endlauf: Selbsttest 39 von 39, **Grenze gehalten — 83 Fundstellen von höchstens 83**, Exit 0 (`docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-tidy.txt`, §42.5). |
| Produktdiff | Außerhalb der Ticketpfade **leer** (§42.6). |
| Abweichungen | Fünf, alle in §42.7 mit Grund; keine Matrixzeile geändert. |
| Offene Punkte | Zwei (§42.9), nicht ins Register geschrieben. |

### 42.1 Was geändert wurde

Zeilenangaben zum Stand `afef1077`.

- **`eq-copilot/plugin/core/analysis/featureengine/Frame.h:122` — die Sperre.** `if (passagenTruePeakRahmen > 0.0 && rahmenNichtEndlich == 0 && ! headroomRing.empty())` statt `if (passagenTruePeakRahmen > 0.0 && ! headroomRing.empty())`. Sie steht vor dem Einschub in `passageTruePeak`, `passageSchiebe` und `ring.schiebe` (`:124-129`), damit vor `fuelleSkalare` (`:134`) und vor `rahmenLeeren()` (`:139`), das den Zähler zurücksetzt. Sie gilt für beide Spannen des Trägers (Histogramm mit Passage, gleitender Ring ohne Passage) — an derselben Stelle, an der ein Rahmen digitaler Stille (`passagenTruePeakRahmen == 0`) schon vorher ausblieb. Kommentar `:112-121`, ein Verweissatz in der Verriegelung von `fuelleSkalare` (`:577-578`). Unverändert: `headroomGesetzt` und die Mindestbelegung (`:685`), die zwei Zähler als gezählte Hälfte (`:579-580`), die Rahmenverriegelung (`:581-582`).
- **`eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:675-679` — nur Kommentar** (Abweichung 4).
- **`eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp`** — Lesezugang `FeatureEngineTestzugang::headroomGroessteKlasseDb` (`:206-212`, nur unter `NAKAMA_FEATUREENGINE_TESTZUGANG`); M-60 (`:1101`) erwartet `headroomFenster == 138u` (`:1152`, Abweichung 3); neuer Fall M-76 (`:1173-1321`).

Kein neues Feld auf der Leitung (U42), kein Schema, keine Fixture, keine Identität; `kFeatureMetricsVersion` unverändert. `FeatureEngine.h`, `Spektrum.h`, `Lautheit.h`, `TruePeak.h` und `Sonde013EventWireTest.cpp` sind unverändert (§42.6).

### 42.2 M-76: Test und Rotbeweis

**Test** `Sonde013DynamicsTest.cpp:1173`, Bein **B18**. Drei Läufe je auf einer frischen Engine im Heap, 48 kHz, 480er-Blöcke, Passage `[0, 672 000)` vor dem ersten Block gebunden, je 140 Rahmen:

- **Bühne** wie M-53: 1-kHz-Ton, erste Hälfte Amplitude 0,5, zweite 0,0005. Der Rahmen 6,9 bis 7,0 s (Samples 331 200 bis 335 999, der letzte der lauten Hälfte, ganz im Passagenfenster) trägt 2 400 Samples Ton mit Amplitude 0,9, dann 2 352 NaN, die der Riegel durch 0 ersetzt und zählt, dann 48 Samples regulären Ton seiner Hälfte (Abweichung 1). Jeder Wechsel liegt auf einem Nulldurchgang.
- **Gegenlauf** (Matrixzeile): derselbe Rahmen mit dem regulären Ton seiner Hälfte.
- **Gegenprobe** (Abweichung 2): die Bühne mit echten Nullen statt NaN — nach dem Riegel dieselben Samples, nur ohne Zählung und Zellenmarke.

| Prüfung (Träger) | grün, Stand `afef1077` | MUT-76 und Basislauf |
|---|---|---|
| Bühne: je 140 Rahmen, genau ein Rahmen mit ersetzten Samples (endet bei Sample 336 000), Gegenlauf und Gegenprobe keinen | 140/140/140 Rahmen; 1/0/0 | grün; gebrochen mit PB-76a |
| Gegenprobe: derselbe Rahmen mit echten Nullen geht ein | Fenster 140, Maximum −0,915 dBTP, größte Klasse −0,915 dBTP | grün; gebrochen mit PB-76b |
| **Fenster:** `headroomFenster` zählt den Rahmen nicht | 139 (Gegenlauf 140) | **rot:** 140 |
| **Maximum:** bleibt beim gemessenen Material | −6,021 dBTP | **rot:** −0,915 dBTP |
| Perzentile: P10, P50, P95 gleichen dem Gegenlauf (Abstand < 0,01 dB) | −66,025 / −6,025 / −6,025 gegen dieselben | grün — ein einzelner Ausreißer bewegt kein Perzentil (§41); gebrochen mit PB-76c |
| **Verteilung:** keine Klasse aus dem teilweise stillgelegten Rahmen | größte Klasse −6,025 (Gegenlauf −6,025, Gegenprobe −0,915) | **rot:** −0,915 |
| Passage bleibt gültig: Fenster intakt, Verteilung und Maximum gesetzt | ja | grün; gebrochen mit PB-76d |

Rohdateien `docs/beweise/roh/NAK-283-rot-M-76-etappe-5-nacharbeit-1.txt` (MUT-76) und `docs/beweise/roh/NAK-283-rot-M-76-etappe-5-nacharbeit-1-basis.txt` (Basislauf). `Frame.h` SHA-256 vorher = nachher `1427F90780EBE62736FFECACE0E285C3EF9EC81D026236CC7266808B92FFE39A`, mutiert `8F5AED294014FB7378A56C0692831A2D129F97B81D0C0FA70C4DCF179C64C5C1`. Im Basislauf tragen `Frame.h` und `Vertrag.h` `9B626DA98F3DC80EEDA33F91BCA794FD4BCC02368DDDEB4E299EDFFFE7E7A001` und `F006B56135665145D99F1ECF042716029BFE086441A905DDB4F3BF77741AE289` — die Endhashes der Etappe 5 (§39.2): der Basislauf misst genau den Pfad, den die Erstprüfung 5 unter Prüffrage (f) bestätigt hat. Dieselbe Mutation macht M-60 an seiner Headroomprüfung rot (`headroomFenster` 140 statt 138).

### 42.3 Rotbilanz

Gefahren von `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-rotskript.ps1` (HEAD `afef1077`; Rohdateien 19:24:44 bis 19:29:49 Uhr). Ablauf §6.1; Muster vorab mit `-Muster` geprüft (14 von 14 je ein Treffer). Ein Rotlauf zählt nur mit Exit ungleich 0 **und** einer `[ROT]`-Zeile an **jedem** Träger des Laufs; die Rücknahme schreibt die gesicherten Bytes zurück (SHA-256 vorher = nachher in jeder Rohdatei), Neubau mit Frischeprüfung, zurückgespielt Exit 0 mit `[ok]` an jedem Träger. Keine Zeile steht in der Nebenläufigkeitsliste aus §6.1. Bilanz `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-rotbilanz.txt`: **19 von 19 Läufen bestanden.** Ein erster Lauf ohne die Prüfungsbrüche (Bilanz 19:09:11 Uhr, 15 von 15 bestanden, dieselben Messwerte) ging voraus; der vollständige zweite Lauf hat dieselben Rohdateien überschrieben. Nach dem Lauf ist `git diff --stat -- eq-copilot` leer. Die Spalte „Rohdatei" nennt den Namen unter `docs/beweise/roh/` gekürzt: `M76…` steht für `NAK-283-rot-M-76-etappe-5-nacharbeit-1…`, `N1-…` für `NAK-283-etappe-5-nacharbeit-1-rot-…`.

| Lauf | Art | Mutation | Träger | mutiert, Messwert der Trägerzeile | Rohdatei |
|---|---|---|---|---|---|
| MUT-76 | Rotbeweis | `Frame.h:122`: Bedingung `rahmenNichtEndlich == 0` entfernt | M-76 Fenster, Maximum, Verteilung | 140; −0,915 dBTP; größte Klasse −0,915 | `M76.txt` |
| MUT-76 | Folge | dieselbe | M-60 Headroom | Fenster 140 | `N1-M-60-folge-M-76.txt` |
| BASIS-N1 | Basislauf | `Frame.h`, `Vertrag.h` am Basis-SHA | M-76 Fenster, Maximum, Verteilung | wie MUT-76 | `M76-basis.txt` |
| BASIS-N1 | Basislauf | dieselbe | M-60 Headroom | Fenster 140 | `N1-M-60-basis-n1.txt` |
| BUEHNE-76 | Bühnenprobe | Test `:1215-1217`: der ersetzte Rahmen endet mit dem Ton 0,9 | M-76 Maximum, Verteilung | −0,915 dBTP; −0,915 (Fenster 139 bleibt) | `M76-buehnenprobe.txt` |
| PB-76a | Prüfungsbruch | Test `:1219`: echte Nullen auch in der Bühne | M-76 Bühnenprüfung | 0 Rahmen mit ersetzten Samples in der Bühne | `M76-bruch-a.txt` |
| PB-76b | Prüfungsbruch | Test `:1219`: NaN auch in der Gegenprobe | M-76 Gegenprobe | Gegenprobe Fenster 139, Maximum −6,021 dBTP, größte Klasse −6,025 | `M76-bruch-b.txt` |
| PB-76c | Prüfungsbruch | nach `Frame.h:130`, falscher Fix: der ersetzte Rahmen als Stille (unterste Klasse) in die Verteilung | M-76 Perzentile | P50 −36,025 gegen −6,025 im Gegenlauf (Fenster 140, Maximum −6,021 bleibt) | `M76-bruch-c.txt` |
| PB-76d | Prüfungsbruch | vor `Frame.h:122`, falscher Fix (§39.9 Punkt 1 (b) wörtlich gelesen): die Passage bricht am ersetzten Rahmen | M-76 Passage gültig | Fenster nicht intakt; Verteilung nur über 69 Rahmen, P10 −6,025 | `M76-bruch-d.txt` |
| MUT-52d | Rotbeweis der Etappe 5, neu gefahren | nach `Frame.h:130` | M-52 Headroom (B16) | Fenster 187 bei 132 Rahmen, P95 −6,025 | `N1-M-52d.txt` |
| MUT-53 | Rotbeweis, neu gefahren | `Frame.h:126-129`, `:683-685` | M-53 | P10 = P50 = P95 = −66,021, Fenster 64 | `N1-M-53.txt` |
| MUT-53 | Rotbeweis, neu gefahren | dieselbe | M-54 | Fenster 64 | `N1-M-54.txt` |
| MUT-53 | Rotbeweis, neu gefahren | dieselbe | M-60 Headroom | P10 = P95 = −66,021, Fenster 64 | `N1-M-60b.txt` |
| MUT-53 | Rotbeweis, neu gefahren | dieselbe | M-52 Headroom, Ring (B16) | Fenster 64 bei 132 Rahmen | `N1-M-52e.txt` |
| MUT-54 | Rotbeweis, neu gefahren | `Frame.h:691` | M-54 | Fenster 64 bei P95 −6,025 | `N1-M-54b.txt` |
| MUT-55 | Regressionswache, neu gefahren | `Frame.h:685` | M-55 (a) | gesetzt, Fenster 3 | `N1-M-55.txt` |
| MUT-49a | Rotbeweis, neu gefahren | `Spektrum.h:148-156`, `FeatureEngine.h:786` | M-60 True Peak | −0,736 dBTP | `N1-M-60a.txt` |
| MUT-56 | Rotbeweis, neu gefahren | `Lautheit.h:54` | M-60 Loudness | 0,3 s Momentary und 2,9 s Short-term gesetzt | `N1-M-60c.txt` |
| BASIS-E5 | Basislauf, neu gefahren | fünf Köpfe am Basis-SHA der Etappe 5 | M-60 True Peak, Headroom, Loudness | −0,736 dBTP; Fenster 64; 0,3 s M und 2,9 s S gesetzt | `N1-M-60-basis-e5.txt` |

Jeder neu gefahrene Rotbeweis der Etappe 5 trifft denselben Messwert wie in §39.2; der geänderte Block in `Frame.h` hat keinen von ihnen geschwächt.

### 42.4 Nicht neu gefahrene Rotbeweise und neue Zeilenlagen

**Nicht neu gefahren, mit Grund.** MUT-44, MUT-45, MUT-48 (Beobachtung), MUT-48b, MUT-49b (M-73), MUT-50, MUT-51a, MUT-51b, MUT-52b und MUT-52c (Zusagezeilen in `Spektrum.h`, Fälle in B16), MUT-49a für M-49, M-50, M-51 (A), M-52 Anfang und M-74 (B16), MUT-55b mit dem Riegel (`FeatureEngine.h:118`), MUT-56 für M-56 und M-57, MUT-57, MUT-58a, MUT-58b und MUT-59b (Zusagezeilen in `Lautheit.h`), MUT-59a (`Spektrum.h`) sowie der Basislauf der Etappe 5 für alle Zeilen außer M-60. Für jede gilt: Zusagezeile und Testfall sind unverändert. `FeatureEngine.h`, `Spektrum.h` und `Lautheit.h` tragen nach allen Mutationen dieser Runde byteweise die Endhashes der Etappe 5 (`C4F57568B5A596920E64F88E016FC98BA4BBDA27F0AC8D1750BE66E8167A18FD`, `1D1E5CA27AEE5647AE2DF7B41EE0E3F888E799EA546079934BA350BD6AE36BBA`, `7DA9740FB8FFF98EBFFC5F33AED23B6BDC2BC96A2C6D5CC9CAE0914CB18978BD`; gemessen nach dem Rotskript), `Sonde013EventWireTest.cpp` hat keinen Diff, und in `Sonde013DynamicsTest.cpp` berührt der Diff nur den Testzugang, den Fall M-60 und den neuen Fall M-76. Die Fälle M-56/M-57 und M-59 und die M-58-Strecke im M-07-Fall tragen ersetzte Samples; keiner vergleicht einen Wert von Passagenmaximum oder Headroom (der M-07-Fall prüft von beiden nur die Endlichkeit gesetzter Werte), und B18 bleibt an allen grün (§42.5).

**Neue Zeilenlagen** zum Stand `afef1077` (§39.2 nennt den Etappenstand):

| Zeile | §39.2, Etappe 5 | jetzt |
|---|---|---|
| M-53 (MUT-53) | `Frame.h:116-119`, `:671-673` | `Frame.h:126-129`, `:683-685` |
| M-54 (MUT-54) | `Frame.h:679` | `Frame.h:691` |
| M-55 (MUT-55) | `Frame.h:673` | `Frame.h:685` |
| M-52 Headroom (MUT-52d) | `Frame.h:119-120` | `Frame.h:129-130` |
| M-59 Rahmenverriegelung (Zähler und `return`) | `Frame.h:567-570` | `Frame.h:579-582` |
| M-60 Headroom (MUT-53) | wie M-53 | wie M-53 |
| M-76 | — | `Frame.h:122` |
| Kopf `headroomFenster` | `Vertrag.h:670-688` | `Vertrag.h:670-690` |
| Fälle in `Sonde013DynamicsTest.cpp` | M-07/M-58 `:611`, M-53/M-54 `:825`, M-55 `:881`, M-56/M-57 `:952`, M-59 `:1015`, M-60 `:1089` | `:623`, `:837`, `:893`, `:964`, `:1027`, `:1101` (je +12 durch den Lesezugang); M-76 `:1173` |

### 42.5 Gefahrene Beine

Gefahren von `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-beinskript.ps1` (Bauform wie §39.4). Maßgeblich ist der **Endlauf** nach dem vollen Rotskript: Rohausgabe `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-beine.txt` (Start 19:30:03 Uhr, HEAD `afef1077`). Lauf 1 vor den Rotbeweisen (Start 19:09:40 Uhr) liegt als `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-beine-lauf-1.txt` bei: ebenfalls 41 von 41 Exit 0, Bau 328,7 s, dieselben acht Warnstellen und dieselben Schlusszeilen.

**Bau.** Vor dem Bau prüft das Skript, dass kein fremder Bau-, Test- oder Kanonprozess läuft (NAK-290). Es stempelt die drei geänderten Quellen neu (SHA-256 vorher = nachher: `Frame.h` `1427F907…E39A`, `Vertrag.h` `0E411C87…73D4`, `Sonde013DynamicsTest.cpp` `02EC9CC3…383B`) und baut Release 42 Ziele — die 36 C++-Beine des Kanons, die Artefakte der Python-Beine, `NakamaKern`, `EqCopilot_VST3` und `NakamaProbeeq_VST3` —, Exit 0 in 301,1 s. `warning C`: 43 eindeutige Zeilen an 8 Stellen, **keine auf einer Zeile, die diese Nacharbeit hinzugefügt oder geändert hat**; es sind die acht Stellen aus §39.4 (`featureengine/Stereo.h:70` C4458, `IpcTestMain.cpp:3705` und `:3987` C4456, `MarkierungTestMain.cpp:1074` C4457, `Sonde012HostChannelContextTest.cpp:295` und `:450` C4244, `StateMigrationTestMain.cpp:1348` und `:1636` C4456). `EqCopilot_VST3` und das Probeeq-Bundle bringen keine neue Warnung.

**Auswahl.** Die Beinliste kommt aus `tools/beweise.ps1` (A15 stillgelegt, nicht gezählt); je C++-Ziel entscheidet das MSBuild-Trackinglog, ob es eine geänderte Quelle übersetzt oder linkt, je Python-Bein die Artefaktzuordnung. Gefahren: die 16 Beine des Auftrags, 19 weitere C++-Beine nach Trackinglog und 6 Python-Beine an einem neu gebauten Artefakt — **41**. Nicht gefahren: A2, B3 und B3b (lesen und linken keine geänderte Quelle), die Cargo-Beine A4, A4-SI, A21, A31 und die übrigen Python-Beine (berühren kein Artefakt des Plugin-Baus); A33 lief getrennt (unten), A32 lief nicht. B9 steht im Auftrag, liest aber keine geänderte Quelle; sein Binary vom 12.09.2026 ist die Gegenprobe der Auswahl.

**Frische.** 38 Artefakte vor dem ersten Bein geprüft: 37 frisch (nach dem Stempeln um 19:30:03 Uhr gebaut; B18 gegen seine eigene Testdatei), eines als Gegenprobe (`EqCopLoudnessGoldenTest`), keines veraltet.

| Bein | Ziel | Grund | Exit | s | Schlusszeile des Beins |
|---|---|---|---:|---:|---|
| A1 | `EqCopNullTest` | Auftrag, Wache Audiopfad | 0 | 0,1 | `NULLTEST OK` |
| A3 | `EqCopMarkierungTest` | Trackinglog | 0 | 7,9 | `MARKIERUNGSTEST OK` |
| A4b | `EqCopPipeClientTest` | Trackinglog | 0 | 65,7 | `PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler` |
| A16 | `EqCopProbeeqNullTest` | Auftrag, Wache Audiopfad | 0 | 0,3 | `SONDE-NULLTEST OK - 65 Pruefungen ok, 0 Fehler` |
| B1 | `EqCopIdentityTest` | Trackinglog | 0 | 0,1 | `IDENTITY-TEST OK - 120 Pruefungen ok, 0 Fehler` |
| B2 | `EqCopStateMigrationTest` | Trackinglog | 0 | 1,4 | `STATE-MIGRATION-TEST OK - 287 Pruefungen ok, 0 Fehler` |
| B3c | `EqCopSchemaTest` | Auftrag | 0 | 0,4 | `152 bestanden, 0 gescheitert` |
| B4 | `EqCopQueueStressTest` | Trackinglog | 0 | 3,5 | `QUEUE-STRESSTEST OK` |
| B9 | `EqCopLoudnessGoldenTest` | Auftrag | 0 | 0,1 | `LOUDNESS-GOLDEN OK` |
| B5 | `EqCopAnalysisGoldenTest` | Auftrag | 0 | 5,5 | `Ergebnis: 248 bestanden, 0 Fehler.` |
| B6 | `EqCopDspGoldenTest` | Trackinglog | 0 | 4,9 | `DSP-GOLDEN OK` |
| B7 | `EqCopTransactionTest` | Trackinglog | 0 | 0,4 | `TRANSAKTION OK` |
| B10 | `EqCopIpcTest` | Trackinglog | 0 | 221,8 | `ALLE PRUEFUNGEN GRUEN — 432 Pruefungen, 0 Fehler` |
| B11 | `EqCopSonde012HostChannelContextTest` | Trackinglog | 0 | 1,0 | `SONDE-012 HOST CHANNEL CONTEXT: 21 bestanden, 0 fehlgeschlagen` |
| B12 | `EqCopSonde012LoudnessSourceTest` | Trackinglog | 0 | 1,6 | `SONDE-012 LOUDNESS SOURCE: 4 bestanden, 0 fehlgeschlagen` |
| B13 | `EqCopSonde012SourcesModelTest` | Trackinglog | 0 | 0,1 | `SONDE-012 SourcesModel: 88/88 gruen` |
| B14 | `EqCopSonde012ProjectReloadTest` | Trackinglog | 0 | 8,4 | `SONDE-012 ProjectReload: 231/231 gruen` |
| B15 | `EqCopShot --sonde012-suite eq-copilot/build/sonde012-shots` | Trackinglog | 0 | 2,2 | `SONDE-012 SHOTS 25/25 gruen` |
| B20 | `EqCopSonde013QualityClassTest` | Auftrag | 0 | 0,1 | `24 bestanden, 0 gescheitert` |
| B22 | `EqCopSonde013FingerprintGoldenTest` | Auftrag | 0 | 0,9 | `24 bestanden, 0 gescheitert` |
| B23 | `EqCopSonde013PassageStateTest` | Auftrag | 0 | 11,6 | `409 bestanden, 0 gescheitert` |
| B24 | `EqCopSonde013PrePostGoldenTest` | Auftrag | 0 | 1,0 | `30 bestanden, 0 gescheitert` |
| B25 | `EqCopSonde013ExperimentGoldenTest` | Auftrag | 0 | 0,1 | `27 bestanden, 0 gescheitert` |
| B26 | `EqCopSonde013ExperimentUiTest` | Trackinglog | 0 | 0,5 | `52 bestanden, 0 gescheitert` |
| B21 | `EqCopSonde013InterventionRingTest` | Trackinglog | 0 | 0,1 | `22 bestanden, 0 gescheitert` |
| B19 | `EqCopSonde013StereoGoldenTest` | Auftrag | 0 | 1,9 | `91 bestanden, 0 gescheitert` |
| B17 | `EqCopSonde013TruePeakGoldenTest` | Auftrag | 0 | 0,3 | `43 bestanden, 0 gescheitert` |
| **B18** | `EqCopSonde013DynamicsTest` | Auftrag, **Ziel** | 0 | 5,6 | `77 bestanden, 0 gescheitert` |
| B16 | `EqCopSonde013EventWireTest` | Auftrag | 0 | 1,6 | `179 bestanden, 0 gescheitert` |
| B27 | `EqCopSonde014IntentTest` | Trackinglog | 0 | 0,5 | `SONDE-014-INTENT-TEST OK - 270 Pruefungen ok, 0 Fehler` |
| B29 | `EqCopSonde014AssistentTest` | Trackinglog | 0 | 0,8 | `SONDE-014 AssistantStep: 193/193 gruen` |
| B28 | `EqCopSonde014BefundTest` | Trackinglog | 0 | 0,1 | `SONDE-014 Befund und Maskierung: 81/81 gruen` |
| B8 | `EqCopLebenslaufTest` | Trackinglog | 0 | 0,3 | `LEBENSLAUF-TEST OK - 79 Pruefungen ok, 0 Fehler` |
| A10 | `py -3.13 tools/eq-copilot/erzeuge_fb_fixtures.py --pruefen` | Auftrag | 0 | 1,7 | `bytegleich: 129 Dateien, MANIFEST sha256=342f9d91…` |
| A14 | `py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py` | Artefakt (NakamaKern, EqCopilot_VST3) | 0 | 8,3 | `25 ok, 0 Fehler` |
| A17 | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | Artefakt (EqCopIpcTest) | 0 | 1,1 | `99 ok, 0 Fehler` |
| A22 | `py -3.13 tools/eq-copilot/pruefe_ipc_last.py` | Artefakt (EqCopIpcLast) | 0 | 10,0 | `GRUEN` |
| A24 | `py -3.13 tools/eq-copilot/pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | Artefakt (EqCopSessionSoak) | 0 | 148,5 | `GRUEN` |
| A34 | `py -3.13 tools/eq-copilot/pruefe_session_soak.py --selbsttest` | Artefakt (EqCopSessionSoak) | 0 | 0,1 | `Selbsttest: 4 von 4 Faellen bestanden` |
| A26 | `py -3.13 tools/eq-copilot/pruefe_p4_korpus.py` | Auftrag | 0 | 0,1 | `Exit-Gate P4: alle vier Klassen bestehen, keine falsche Behauptung.` |
| A23 | `py -3.13 tools/eq-copilot/pruefe_sonde012_sources_latency.py` | Artefakt (EqCopSonde012SourcesLatencyTest) | 0 | 4,6 | `GRUEN - sechs p95-Grenzen und Anzeige-Revisionen` |

**Goldens.** Keine Verschiebung, keine Fixture geschrieben: B5, B9, B17, B19, B22, B24 und B25 laufen unverändert grün mit denselben Zählungen wie in §39.4, A10 findet den FlatBuffers-Korpus bytegleich zur Neuerzeugung (dieselbe MANIFEST-Summe), A26 hält den P4-Korpus; `git diff --stat d8c3f74c..HEAD -- eq-copilot/fixtures` ist leer (§42.6).

**Nulltest.** A1 (`NULLTEST OK`, darin „1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)") und A16 (`SONDE-NULLTEST OK - 65 Pruefungen ok, 0 Fehler`) sind in Lauf 1 und im Endlauf grün.

**A33 (clang-tidy-Ratsche).** `py -3.13 tools/plan/tidy.py --mit-selbsttest --jobs 3` am Stand `afef1077` (Ende 19:47:33 Uhr, 205,4 s), gestartet erst nach Rotskript und Endlauf und nicht parallel zu ihnen — so las clang-tidy keinen mutierten Kopf, und die Zeitbeine des Endlaufs liefen ohne diese Last: Selbsttest 39 von 39, 41 Übersetzungseinheiten fehlerfrei geparst, **Grenze gehalten — 83 Fundstellen von höchstens 83** (Ziel 0), Exit 0 (`docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-tidy.txt`). Keine Fundstelle in `Frame.h` oder `Vertrag.h`; der Test liegt außerhalb des Umfangs von A33 (`eq-copilot/plugin` ohne `tests/`). Die sieben Fundstellen in geänderten Köpfen der Etappe 5 (`FeatureEngine.h:1104`, `Spektrum.h:393-401`, alle `bugprone-misplaced-widening-cast`) stehen unverändert an denselben Zeilen wie in §39.4.

### 42.6 Produktdiff außerhalb der Ticketpfade

`git diff --stat d8c3f74c656bb88a08999edbf6705440a4f2e71a..HEAD -- eq-copilot/plugin/core/analysis/TruePeak.h eq-copilot/plugin/core/analysis/featureengine/Spektrum.h eq-copilot/plugin/core/analysis/featureengine/Lautheit.h eq-copilot/plugin/src eq-copilot/plugin/sonde eq-copilot/plugin/dsp eq-copilot/plugin/state broker eq-copilot/schemas eq-copilot/identity eq-copilot/fixtures tools` am Stand `afef1077`: **Ausgabe leer**, Exit 0. Ebenso leer: `git diff --stat d8c3f74c..HEAD -- eq-copilot/plugin/core/analysis/FeatureEngine.h eq-copilot/plugin/tests/Sonde013EventWireTest.cpp`. Der ganze Diff `git diff --stat d8c3f74c..afef1077`: `Frame.h` 14 (+13/−1), `Vertrag.h` 8 (+5/−3), `Sonde013DynamicsTest.cpp` 176 (+172/−4). Die Commits nach `afef1077` tragen nur `docs/beweise/NAK-283.md` und `docs/beweise/roh/`.

### 42.7 Abweichungen und Entscheide innerhalb der Zeile

Keine Matrixzeile geändert — die Prüfung entscheidet.

1. **Bühne: der ersetzte Rahmen endet mit einer Periode regulären Tons.** Die Zeile sagt „in seinen Samples steht der Ton mit Amplitude 0,9 … und ein Teil der Samples ist NaN". Der True-Peak-Interpolator gibt ein Sample zwölf Ticks später aus und liest dafür 25 Samples zurück (`TruePeak.h:75-78`, `:172-179`); stünde der laute Ton bis zum Rahmenende, trüge der nächste, saubere Rahmen dessen Nachklang ins Passagenmaximum — der Zahlenrand §39.9 Punkt 1 (d), Härtung NAK-295, keine Zusage von M-76. **Gemessen** mit BUEHNE-76: ohne die Schlussperiode endet der Fall bei unverändertem Produkt mit Fenster 139, aber Maximum −0,915 dBTP und größter Klasse −0,915 — er fiele an einer Stelle, die die Zeile nicht zusagt. Mit ihr rechnet der Interpolator in jedem anderen Rahmen über dieselben Samples wie im Gegenlauf, und die Zusage fällt unter MUT-76 weiter an beiden Größen, Fenster **und** Maximum. Lage und Anteil innerhalb des Spielraums des Auftrags: Rahmen 6,9 bis 7,0 s (der letzte der lauten Hälfte), 2 352 von 4 800 Samples NaN, Wechsel auf Vielfachen von 48 Samples (Nulldurchgänge), damit der laute Abschnitt ohne Sprung beginnt und endet.
2. **Gegenprobe und Verteilungsträger zusätzlich zu Bühne und Gegenlauf.** Die Zeile nennt „Bühne plus Gegenlauf in einem Fall". Die Perzentile zeigen einen einzelnen Ausreißer nicht (§41), die Rotbeweisspalte nennt aber „die Verteilung trägt einen Wert aus dem teilweise stillgelegten Rahmen". Ein Lesezugang liest deshalb die größte belegte Klasse der Verteilung (nur unter `NAKAMA_FEATUREENGINE_TESTZUGANG`, nur lesend, vom Produkt nie gerufen) und misst diesen Satz direkt (MUT-76 rot: −0,915). Die Gegenprobe mit echten Nullen rechnet nach dem Riegel über dieselben Samples wie die Bühne: sie belegt, dass der Rahmen an seinen ersetzten Samples ausbleibt und nicht an seinem Material (Fenster 140, Maximum −0,915 dBTP), und dass der Lesezugang einen lauten Wert sieht, wenn er in der Verteilung steht.
3. **M-60 erwartet 138 statt 140 Rahmen.** M-60 ersetzt jedes zweite Sample zwischen 10,8 und 11,0 s der Passage (Samples 614 400 bis 623 999, genau die Rahmen 128 und 129 des Stroms); beide tragen `rahmenNichtEndlich > 0` und zählen nach der Regel §40 nicht mehr. §39.7 Abweichung 7 („so trägt jeder der 140 Rahmen einen True Peak über 0, `headroomFenster` ist genau 140") beschreibt den Stand vor der Regel. Die Zeile M-60 nennt keine Rahmenzahl; P10 −66,025 und P95 −6,025 halten, beide Hälften bleiben in der Verteilung. Prüftext und Kommentar des Falls sind nachgezogen, der Träger-Teilsatz „Headroom: die Verteilung beschreibt beide Haelften der ganzen Passage" blieb wortgleich. Alle drei Rotbeweise von M-60 und sein Basislauf sind neu gefahren (§42.3).
4. **`Vertrag.h`: Kommentar des Headroomkopfs nachgezogen**, wie vom Auftrag zugelassen. „ein Wert je publiziertem Rahmen" und „mit markierter Passage gehen ALLE Rahmen der Passage ein" widersprachen nach M-76 einer gemessenen Zusage; jetzt: „hoechstens ein Wert je publiziertem Rahmen (10 Hz): ein Rahmen ohne True Peak (digitale Stille) oder mit ersetzten Samples (NaN-Riegel, NAK-283 M-76) traegt keinen. … mit markierter Passage gehen ALLE uebrigen Rahmen der Passage ein". Der Satz „`headroomFenster` nennt in beiden Faellen die Zahl der Rahmen, die wirklich eingegangen sind" ist wortgleich und stimmt weiter. Kein Feld wechselt Typ oder Bedeutung auf der Leitung; B3c und A10 grün (§42.5).
5. **Mehr gemessen als verlangt.** Neben M-76 sind die vier Etappe-5-Mutationen an `Frame.h` neu gefahren (ihre Zusagezeilen stehen in oder an dem geänderten Block), die vier neuen Prüfungen, die MUT-76 grün lässt, sind mit PB-76a bis d je einmal gebrochen (Prüfliste E), und die Beine laufen wie in Etappe 5 zusätzlich über jedes Kanon-Bein, dessen Binary eine geänderte Quelle übersetzt oder linkt (`Frame.h` hängt über `FeatureEngine.h` an jedem Analyseverbraucher). Das Beinskript lief zweimal: Lauf 1 vor, der Endlauf nach dem vollen Rotskript.

### 42.8 Selbstaudit nach `tools/dirigent/pruefliste.md`

| Punkt | Gemessen oder gelesen |
|---|---|
| E — jede neue oder geänderte Prüfung einmal gebrochen | Fenster, Maximum, Verteilung: MUT-76 und Basislauf. Perzentile: PB-76c. Passage gültig: PB-76d. Bühnenprüfung: PB-76a. Gegenprobe: PB-76b. M-60 Headroom (geändert auf 138): MUT-76 und Basislauf der Nacharbeit (je 140), MUT-53 und Basislauf der Etappe 5 (je 64). Die Rohausgabe jedes Rots liegt bei (§42.3). |
| E — Behauptung ≤ Messung | `Frame.h:112-121` nennt nur Gemessenes oder an der Quelle Gelesenes: „wie ein Rahmen digitaler Stille" (dieselbe Bedingung), „bleibt auch am Anschlag > 0" (M-59 misst die Sättigung), „`headroomFenster` zaehlt den Rahmen damit nicht, die Passage bleibt gueltig" (M-76). Der Testkommentar zur Schlussperiode ist mit BUEHNE-76 gemessen. Die Regressionswache M-55 heißt so. |
| Zahlenränder | `rahmenNichtEndlich` ist `uint32`, sättigt bei `0xFFFFFFFF` (M-59) und sperrt dort weiter; `== 0` ist exakt. `passageSchiebe` bekommt weiter nur `20·log10 (x)` mit `x > 0`. Der Lesezugang liefert NaN für eine leere Verteilung, die Prüfung verlangt `isfinite`. Toleranzen: Perzentile < 0,01 dB (eine Klassenbreite), Maximum ±0,1 dB (wie M-53), Klassenmitten < 0,005 dB. |
| aktivieren↔abklingen | Die Sperre lebt genau einen Rahmen (`rahmenLeeren` setzt den Zähler zurück); der Rahmen danach ist sauber und geht ein — in der Bühne fehlt genau 1 von 140 Rahmen. |
| öffnen↔schließen | Unverändert: Binden, Lösen, `zuruecksetzen` und `grenzeZiehen` leeren den Träger mit einem `leeren()`; M-52 Headroom an beiden Grenzen neu gefahren und rot (MUT-52d, MUT-53). |
| Randfall, gelesen, nicht gemessen | Ein Rahmen, der eine Passagengrenze überspannt und nur außerhalb des Fensters ersetzte Samples trägt, bleibt ebenfalls aus der Verteilung — regelkonform (die Regel liest `rahmenNichtEndlich`) und fail-closed wie die Rahmenverriegelung, die denselben Rahmen sperrt (offener Punkt 2). |
| Leitung, Speicher, Audio-Thread | Kein Feld, kein Typ, keine Bedeutung auf der Leitung (B3c, A10 bytegleich). Keine neue Allokation: B18 M-55 (b) im Endlauf 0 Allokationen in 20 s gebundener Passage, `vorbereiten()` legt 2 536 859 Byte in 68 Allokationen an — dieselben Zahlen wie am Etappenstand (§39.7 Abweichung 10). `FeatureEngine` rechnet im Analyseworker; A16 und A1 grün (§42.5). |
| Stack (NAK-175) | Der neue Fall hält die Engine je Lauf im Heap und statt ganzer `FeatureFrame`-Kopien eine kleine Ergebnisstruktur. |
| F — Änderungssatz | Schreiber (`baueFrame`), Leser (`fuelleSkalare`), Vertragskopf (`Vertrag.h`) und Test im selben Commit `afef1077`. |

### 42.9 Offene Punkte dieser Runde

Nicht gebaut, weil keine Zeile sie zusagt; zur Einordnung durch Wiederprüfung und Dirigent.

1. **Kopf von `HeadroomVerteilung` (`FeatureEngine.h:247-251`)** — „Jeder publizierte Rahmen der Passage geht als eine dBTP-Klasse in ein speicherfestes Histogramm ein" nennt mehr, als der Aufrufer einschiebt: ein Rahmen ohne True Peak blieb schon vorher aus, einer mit ersetzten Samples jetzt auch. Die Ausnahmen stehen an der Sperre (`Frame.h:112-121`) und im Vertragskopf (`Vertrag.h:675-679`). Nicht geändert, weil der Auftrag `FeatureEngine.h` nur für eine Änderung zulässt, ohne die die Zeile nicht schließbar ist; ein reiner Kommentarsatz.
2. **Randfall an einer Passagengrenze** (§42.8): ein überspannender Rahmen mit ersetzten Samples nur außerhalb des Fensters bleibt aus der Verteilung, obwohl seine Fenstersamples sauber sind. Regelkonform und fail-closed; ohne eigene Bühne. Kandidat für NAK-295 wie §39.9 Punkt 1 (a), (c), (d).

---

## 43. Messung der Nacharbeit 1 (Etappe 5), Einordnung und Auftrag der Wiederprüfung 1 (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Worker | `79ed3c00` (`nakama-nak283-d8c3f74-e5-nacharbeit1`, Opus max, dontAsk), Auftrag `docs/beweise/roh/NAK-283-etappe-5-nacharbeit-1-auftrag.txt` (§41); Start 18:4x Uhr, Meldung 19:53 Uhr, Prozess danach idle (ein veralteter Wecker des Workers lief 19:54 Uhr leer), vom Dirigenten 20:0x Uhr gestoppt. Beobachter (Aufsicht ENG): Start, Telemetrie gesund, einmal „Statusquelle unbekannt" unter Bau-Last (Fehlalarm nach Skill §3.2, selbst nachgemessen: `working`, HEAD unverändert, drei Ticketdateien in Änderung), drei HEAD-Wechsel; Stundenloop `dabcbc5a` gelöscht, Beobachter beendet, `CronList` leer. |
| Commits | `afef1077` (Produkt und Test), `d364b6a1` (§42, Rot- und Beinskript, 26 Rohdateien), `6195c143` (Kopfzeile), alle auf `origin/master`; Basis `d8c3f74c` ist Vorfahr; `git status --short` zeigt nur die zwei fremden untracked Ordner (§27). |
| Diff | `git diff --stat d8c3f74c..6195c143`: `Frame.h` 14 (+13/−1), `Vertrag.h` 8 (+5/−3, nur Kommentar), `Sonde013DynamicsTest.cpp` 176 (+172/−4); dazu §42 und die Rohdateien. Fremde Pfade gemessen leer: `TruePeak.h`, `Spektrum.h`, `Lautheit.h`, `FeatureEngine.h`, `plugin/{src,sonde,dsp,state}`, `broker`, `schemas`, `identity`, `fixtures`, `tools`. Produkt-Hunks gelesen: die Sperre `rahmenNichtEndlich == 0` an `Frame.h:122` vor dem Einschub in `passageTruePeak`, `passageSchiebe` und `ring.schiebe`, gelesen vor `rahmenLeeren()`; Kommentare an `:112-121` und `:577-578`; `Vertrag.h:675-679` Kommentar. Test: Lesezugang unter Makro (`:206-212`), M-60 auf 138 Rahmen (`:1152`), neuer Fall M-76 (`:1173-1321`, drei Läufe im Heap). |
| Rundenbilanz | `d8c3f74c..6195c143: Produkt 2 Datei(en) +18/-4 \| Tests 1 Datei(en) +172/-4 \| Doku 26 Datei(en) +17632/-1` — fünfte Produktrunde des Tickets; kumuliert (`--runden bb49a612 988dc44d faea3c1a 3d6bbfb4 30a0b1bf 6f0d35d7 32c09cb4 7781b316 32c3f297 6195c143`): „kein Konvergenz-Signal". |
| Eigene Messung | 20:0x Uhr auf den Binaries des Worker-Endlaufs (19:33 Uhr, jünger als `Frame.h` und `Sonde013DynamicsTest.cpp`, 19:30 Uhr): B18 `EqCopSonde013DynamicsTest` 77 bestanden, 0 gescheitert; B16 `EqCopSonde013EventWireTest` 179 bestanden, 0 gescheitert; beide Exit 0, identisch mit §42.5. Rotbilanz gelesen: 19 von 19 Läufen; MUT-76 rot an Fenster (140), Maximum (−0,915 dBTP) und größter Klasse (−0,915); Basislauf am Basis-SHA rot an denselben drei Trägern; Rücknahme bytegleich (SHA-256 vorher = nachher); die vier Etappe-5-Mutationen an `Frame.h` und die drei M-60-Rotbeweise neu gefahren mit den Werten aus §39.2. |
| Messabdeckung | M-76 mit Test, Rotbeweis an der Zusagezeile und Basislauf; vier Prüfungsbrüche PB-76a bis d (Prüfliste E); M-60 mit geänderter Erwartung neu falsifiziert. Kein Fortsetzungsauftrag nötig. |
| Abweichungen §42.7 (Einordnung) | **Fünf gedeckt, kein Befund** — an Diff, Rotbilanz und Quelle gelesen: 1 (Schlussperiode regulären Tons im ersetzten Rahmen — innerhalb des Spielraums, den der Auftrag einräumt; gemessen mit BUEHNE-76; die Zusage fällt weiter an beiden Größen; die Matrixzeile verlangt „ein Teil der Samples ist NaN" und ist erfüllt), 2 (Gegenprobe mit echten Nullen und Lesezugang für die größte Klasse — Mehrmessung; Testzugang unter `NAKAMA_FEATUREENGINE_TESTZUGANG` wie M-59, vom Produkt nie gerufen), 3 (M-60 erwartet 138 statt 140 — Folge der Regel §40 für die zwei Rahmen mit ersetzten Samples; die Zeile M-60 in §5.4 nennt keine Rahmenzahl, gemessen; §39.7 Abweichung 7 beschreibt den Stand vor der Regel; Rotbeweise und Basislauf von M-60 neu gefahren), 4 (`Vertrag.h` nur Kommentar, vom Auftrag zugelassen; der Satz zur Bedeutung von `headroomFenster` wortgleich, kein Feld auf der Leitung geändert, B3c und A10 grün), 5 (mehr gemessen als verlangt). Prüffragen an die Wiederprüfung. |
| §42.9 offene Punkte (Einordnung) | 1 (Kommentar `FeatureEngine.h:247-251` nennt mehr als eingeschoben wird): Kommentarpflege außerhalb des Fixdiffs, kein Befund; wird beim Ticketabschluss datiert ins Register geschrieben (Nachzug im nächsten Änderungssatz, der `FeatureEngine.h` anfasst, spätestens Pflegeschritt S25g). 2 (Randfall eines Rahmens über einer Passagengrenze mit ersetzten Samples nur außerhalb des Fensters — regelkonform, fail-closed): **Härtung NAK-295**, Nachtrag beim Abschluss. |
| Wiederprüfung 1 | Auftrag `docs/beweise/roh/NAK-283-etappe-5-wiederpruefung-1-auftrag.txt` (Vorlage B): Prüfbereich `git diff d8c3f74c...6195c143 -- Frame.h Vertrag.h Sonde013DynamicsTest.cpp`, die Lückenregel und M-76 als zu schließender Punkt, zwei Fragen (M-76 geschlossen; nichts gebrochen — darunter M-60 auf 138 und M-55 (a)), Ausschlüsse mit NAK-295 und dem Kommentar §42.9 Punkt 1. Codex `gpt-6-astra`, Effort **max**, read-only, über `tools/dirigent/codex-lauf.ps1 -Kennung nak283-etappe5-wieder1`; Thread-ID und Urteil in §44. HEAD vor und nach dem Lauf = der Commit dieses Abschnitts. |
| Hygiene des Fensters | Bytes der Kontextfläche unverändert gegen §41 (MEMORY.md 5 338 B, CLAUDE.md 19 434 B, Skill 24 465 B); `dokuriegel.py` auf dem Manifest vor dem Commit ohne Befund; `gesundheit.py` und `tidy.py` im Abschlussfenster nach der Wiederprüfung (A33 vom Bauer: 83 von 83, §42.5). |

---

## 44. Wiederprüfung 1 der Etappe 5 — Urteil, Etappenabschluss und Start des Ticketabschlusses (Dirigent, 13.09.2026)

| Merkmal | Wert |
|---|---|
| Lauf | `tools/dirigent/codex-lauf.ps1 -Kennung nak283-etappe5-wieder1`, Codex `gpt-6-astra`, Effort **max**, Sandbox read-only, Prompt `docs/beweise/roh/NAK-283-etappe-5-wiederpruefung-1-auftrag.txt` (Vorlage B, §43). Start 20:11:33 Uhr, Ende 20:17:35 Uhr, Exit 0; HEAD vorher und nachher `e5e0155a`, Worktree unverändert. Thread `01a09bf7-a64f-7470-80f0-b1def9213558`. Monitor und Stundenloop `fe2a3ca1` beendet, `CronList` leer. |
| Urteil | **PASS — M-76 geschlossen, nichts gebrochen.** Wörtlich in `docs/beweise/roh/NAK-283-wiederpruefung-1-etappe-5-e5e0155.txt`. Geprüft (laut Prüfer): die Sperre `Frame.h:122-137` vor Passagenmaximum, Histogramm und gleitendem Ring, vor `fuelleSkalare` und `rahmenLeeren`; der Test hält den Rahmen ganz in der Passage, prüft 139 gegen 140, gleiche Perzentile und gültige Passage; `TruePeak.h:75-78,172-179` bestätigt die Nachklangbegründung (12 Samples Verzögerung, bis 25 gelesene Samples — 48 reguläre Schlusssamples reichen); MUT-76 und Basislauf gegen die Produktquellen von `d8c3f74c` mit denselben roten Trägern (140; −0,915 dBTP; −0,915), danach Exit 0 und identische Rücknahmehashes, nachvollzogen an Rotskript und Rohdateien; M-60 zählt regelkonform zwei Rahmen weniger (138) ohne Bruch der Spannen-Zusage; M-55 bleibt bei mindestens vier Beiträgen; `Vertrag.h:675-690`, Writer und C++-/Rust-Leser behalten Bedeutung „wirklich eingegangene Rahmen" und `uint32`; keine zusätzliche Allokation, keine Verletzung der NaN-, Audio-, State- oder Identitätsinvarianten; der Lesezugang existiert nur im Test unter Makro. Nicht geprüft: keine eigenen Läufe; die Laufresultate (41/41, B18 77/0, B16 179/0, M-55 (b), B3c, A10, A33) aus den Rohdateien gelesen. Keine Lücke, keine Härtung benannt. |
| Einordnung | Kein Befund. Hinweis des Prüfers ohne Befundcharakter: das Maximum wird gegen den gerechneten Sollwert −6,0206 dBTP geprüft, nicht direkt gegen das Gegenlaufmaximum — die Rohwerte beider Läufe zeigen −6,021, die Zeile sagt „rund −6,02 dBTP". **Etappe 5 abgeschlossen auf `6195c143`** (Produktcode und Tests der Etappe `4679eb3b`, der Nacharbeit `afef1077`): Erstprüfung 5 PASS mit null Defekten → Nacharbeit 1 (Lückenregel §40, M-76) → Wiederprüfung 1 PASS; eine Nacharbeitsrunde, durch die Dirigentenregel ausgelöst, nicht durch einen Defekt. |
| Ticketstand | Alle Bauetappen abgeschlossen: Etappe 2 auf `3d6bbfb4` (zwei Nacharbeitsrunden), Etappe 3 auf `30a0b1bf` (Lücke R-283-7, keine Runde), Etappe 4 auf `6f0d35d7` (keine Runde), Etappe 6 auf `32c09cb4` (keine Runde), Etappe 5 auf `6195c143` (eine Runde aus Regel). Matrix (Etappe 1): drei Nacharbeitsrunden bis PASS. Kumulierte Rundenbilanz (§43): „kein Konvergenz-Signal"; Produktfortschritt in den Etappen 2, 3, 4, 6, 5 und der Nacharbeit 1. |
| Ticketabschluss (gestartet 20:2x Uhr) | (1) **Abschlussprüfung** (Vorlage A) über `39e18d3f...6195c143 -- <41 Ticketpfade der Etappen 2 bis 6 samt tools/beweise.ps1>` (Auftrag `docs/beweise/roh/NAK-283-abschlusspruefung-auftrag.txt`: Gate wörtlich mit M-76, je Etappe Gate-Teil, Riegel und Prüffragen wörtlich aus dem Erstprüfungsauftrag, Wechselwirkungen (a) Vertrag Plugin↔Broker, (b) DSP-Kante↔Analyse, (c) EBU-Fälle↔True-Peak-Kette; Ausschlüsse je Etappe plus NAK-295, Kommentar §42.9 Punkt 1, Lücke R-283-7 und die drei ticketfremden Commits `4e70e369`, `8aeec95d`, `74d9c28f` in `tools/beweise.ps1` und `tools/plan/gesundheit.py`); Codex `gpt-6-astra` max, read-only, Kennung `nak283-abschluss`; HEAD = der Commit dieses Abschnitts. (2) **Voller Kanon** auf dem End-Stand, abgekoppelt und parallel (`pwsh -NoProfile -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/NAK-283.md -Anhaengen -Titel NAK-283`, Log `$env:TEMP\nakama-nak283-kanon.log`; kein anderer Bau- oder Testprozess im Checkout, Worker `79ed3c00` gestoppt, NAK-290). (3) Danach der **Laufzeit-Arm** `tools/fl/laufzeit.ps1 -Ticket NAK-283 -Basis 39e18d3f` (erster Lauf mit Installation, S25e). Urteil, Thread, Kanonbilanz, Laufzeit, Register (NAK-253, NAK-163, NAK-269, NAK-249, NAK-199, NAK-295-Nachtrag, Kommentarpflege §42.9 Punkt 1) und Urteil T2 in §45. |
| Hygiene des Fensters | Bytes der Kontextfläche unverändert gegen §41; `dokuriegel.py` auf dem Manifest vor dem Commit; `gesundheit.py` (A32) und `tidy.py` (A33) laufen im Kanon. |

---

