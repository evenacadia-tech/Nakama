# Dirigenten-Protokoll

Was hier steht: **warum** eine Runde als fertig galt und die nächste startete.
Nicht *dass* sie fertig war — das steht in den Beweismanifesten
(`docs/beweise/`) und in der Git-Historie.

Der User hat am 23.08.2026 entschieden, dass der Dirigent **durchläuft, bis der
Plan leer ist**, und dass er dabei **nicht an einer harten Regel scheitern**
soll: *„die regel ist einfach das individuell zu entscheiden mit bestem
wissen"*. Dieses Blatt ist die Gegenleistung dafür. Freie Urteilskraft ohne
Belegpflicht wäre eine Erlaubnis, sich selbst zu überreden; mit ihr steht
morgens schwarz auf weiß, worauf sich jede Weiterschaltung stützte — und
**was dabei ungeprüft blieb**.

Ein Eintrag ohne die Zeile „Nicht geprüft" ist unvollständig.

Regeln und Zyklus: `.claude/skills/dirigent/SKILL.md`. Jüngster Eintrag oben.

---

## 2026-08-23 spät · HALT · User-Stopp

- **Anlass:** User-Wort 23.08.: „bin wieder da, kannst das Dirigieren stoppen jetzt" — §4, expliziter Stopp. Es wird nichts mehr gespawnt.
- **Letzter Rundenstand, am Repo gemessen:** Das T2-Urteil zu SONDE-009 ist gelandet (`d2144a1`): **NEEDS_WORK — die Bandakkumulatoren überbrücken jede Epochengrenze.** Der Kernbefund trifft genau den Gate-Satz; Details im Manifest-T2-Abschnitt. Positiv aus derselben Prüfung: die BS.1770-Koeffizienten sind unabhängig aus den publizierten ITU-Werten nachgerechnet (0,04312 dB, deckungsgleich mit dem Manifest). Die Prüfsession (`f870959a`) schrieb beim Stopp noch ihre Abschluss-Doku (A19-Falsifikationsprobe, Manifest, Hub, NEXT-SESSION) — sie darf fertig schreiben; das ist Abschluss ihrer Runde, keine neue.
- **Bewusst NICHT angesetzt:** die Nacharbeit für die SONDE-009-T2-Befunde (Stopp kam vor §3.5). Die Befunde stehen protokolliert im Manifest und warten auf die nächste Session.
- **Offen bei Übergabe:** (1) Nacharbeit SONDE-009-T2-Befunde · (2) drei frische Prüfer-PASS für S9, S10–11, S12–13 — Vorschlag: am Gate G2 bündeln · (3) S14–15/`SONDE-010` (dort NAK-59, NAK-40, Broker wird Spawn-Ziel) · (4) danach **Gate G2** = T3-HALT mit fremden Prüfern · (5) User-Karte **U13** (FL-Kurztermin C / NAK-56-Ablesung, Installation = User-Klick; Seite zeigt die Karte erst nach Deploy, `/api/hub` trägt sie) · (6) NAK-57 (Telemetrie-Anzeige, wartet auf Figma-Übersetzung) · (7) die dokumentiert liegengelassenen verwaisten Stände (`design/docs`-Paar + untracked Spezifikation, zwei `eq-copilot/design`-Löschungen, „Untitled Workspace").
- **Bilanz des Laufs (23.08., ein Tag):** S9-T2 Teil B gefahren + Nacharbeit (4 Befunde zu, Kanon-Bein A18 neu) · S10–11 gebaut + T2 + Nacharbeit (4 Befunde zu, U10-Wächter, Oberband-Fix) · S12–13 gebaut + T2 gefahren (Urteil da, Nacharbeit offen) · Hygiene-Runde (CLAUDE.md-Register vollständig, NAK-54/55 zu) · Kanon 23 → **28** Beine · Plan 14 → **16/34** Zeilen · geschlossene Punkte: NAK-24, NAK-29, NAK-32*, NAK-35, NAK-54, NAK-55, NAK-58, NAK-56-Werkbankhälfte (*NAK-32 schon vor dem Lauf) · alles gepusht, **nichts installiert**, kein PASS behauptet, der nicht von einem frischen Prüfer kam.

## 2026-08-23 spät · SONDE-009 · bau (S12–13)

- **Session:** „S12–13 SONDE-009 — FeatureEngine v2" (`7e47fcfb-d9c2-45bc-8cfe-f74e5ceb3c1c`), Opus 5 mit 1M.
- **Ergebnis:** Gebaut in sieben Commits (`f14924a` Bandvertrag/K-Gewichtung/FFT im Realtime-Core · `357786e` FeatureEngine v2 + erweiterter Zeitstempel · `133526e` Kanon-Beine B5+A19 · `f1e4a08` Manifest, **NAK-29 und die NAK-56-Werkbankhälfte geschlossen**, NAK-59 neu · `aed5fcf` Docs · `39b4cdd` Hub · `48fcd9c` Selbstaudit). Der Gate-Satz „Drop/Seek/Loop trennt jedes offene Fenster" ist als **neun** unterscheidbare Epochengrenzen gebaut, je mit eigenem Testfall; der Filterzustands-Flush ist gemessen (ohne ihn >9 LU Fehler). **Sieben Vorführungen, eine davon ehrlich protokolliert zunächst blind** (der Test schaute erst nach 3,5 s — nachgeschärft, dann schlug er an; derselbe Fehlertyp wie T2-1 gestern). Die Brücken-Gegenprobe zeigt, dass der `nakamaBlockEmpfangen`-Pfad gebraucht wird: 7 Zeitangaben über die Brücke, 2 über den Ersatzweg. Kanon 26 → **28**; **maßgeblich ist der vierte Lauf** (17:07, 28/28, Exit 0) — der dritte galt, bis der Selbstaudit einen numerischen Rand am Ursachenzähler fand (`48fcd9c`). Bitidentität gemessen, kein Sample geändert. NAK-59 (Band-Stereo hat keinen Platz im v3-Binärvertrag) korrekt bei SONDE-010 verortet — Wire-Format-Versionierung gehört in das Ticket, das den Vertrag anfasst („Schemas sind Verträge").
- **Gestützt auf:** Repo-Messung — Commits gepusht, Manifest-Rohblöcke (16:56 + 17:07 je 28/28 Exit 0), §5 „Jedes neue Bein beim Fallen vorgeführt", NAK-Buchführung in `offene-punkte.md`.
- **Nicht geprüft:** Das T1-Kästchen im Manifestkopf habe ich nicht explizit verifiziert (geht als Prüfauftrag an den T2-Prüfer); den Code selbst misst der frische Prüfer — insbesondere die **FFT im Realtime-Core** (heikelste neue Fläche) und die neun Grenzen einzeln.
- **Weiter mit:** T2-Prüf-Session für SONDE-009 — Basispunkt **`ff24908`** (selbst gerechnet: Elterncommit von `f14924a`). Danach warten DREI Stände auf frische Prüfer-PASS (S9, S10–11, S12–13) — die G2-Bündelung wird konkreter.

## 2026-08-23 abends · Baumhygiene · hygiene (parallel zu S12–13)

- **Session:** „Baumhygiene + Registerpflege (NAK-54/55)" (`1d1f86d1-c131-4695-a05e-acd4ed9c40e6`), Opus 5 mit 1M — parallel zur S12–13-Bau-Session auf strikt disjunkten Territorien; Anstoß war der Stop-Hook (Fund = Fix statt „sobald die Datei frei ist").
- **Ergebnis:** Fünf Commits — `6e67662` (die verwaiste Briefing-Seiten-Arbeit war fertige Arbeit und ist committet: alle 18 Gen-Lesarten sichtbar, U2.16 geklärt) · `ccc6c19` (.gitignore-Hygiene: Automatik-Laufergebnisse, TS-Buildcache) · `09c5899` (**CLAUDE.md nachgezogen**: die zwei Register-Einträge vom 23.08. mit Wortlaut; drei gealterte Zahlen an der Quelle nachgemessen — Kanon **26** [Tabelle + drei Manifest-Rohblöcke, zwei Wege], Verbraucher **14** [GLOBAL-Property; NAK-55s „13" war von vor `EqCopQueueStressTest`]; die fail-open-Passage berichtigt, an `PluginProcessor.cpp:300-306` gegengelesen) · `803f0f0` (NAK-54 und NAK-55 geschlossen; drei weiterhin unklare Stände **dokumentiert statt geraten**: `design/docs`-Paar, die zwei `eq-copilot/design`-Löschungen, „Untitled Workspace") · `0efffa6` (Formatierungs-Selbstaudit). **Nichts verworfen.**
- **Gestützt auf:** Commits im Repo, gepusht; die Zahlen tragen Zwei-Wege-Belege im Commit/Insight der Session.
- **Nicht geprüft:** Die CLAUDE.md-Diffs habe ich nicht Zeile für Zeile gegengelesen (die Session hat je Zahl die Quelle genannt); die bewusst liegengelassenen verwaisten Stände bleiben mit datierter Zeile im Offen-Set — gewollt, kein Raten über fremde Absicht.
- **Weiter mit:** S12–13-Bau läuft weiter (aktive uncommittete Edits an `StampedAudioQueue.h`, `PluginProcessor.*`, neu `BandGrid.h`).

## 2026-08-23 abends · SONDE-008 · nacharbeit

- **Session:** „SONDE-008 Nacharbeit — T2-1…T2-4 schließen" (`62326e9e-fc5b-49e7-ae6f-d97b734f1e3f`), Opus 5 mit 1M.
- **Ergebnis:** Alle vier Befunde geschlossen (Manifest §9.1–§9.4; Commits `f88f8c4`, `90c387a`, `a563d4c`, `0373c51`), alle vier vorher an der Quelle bestätigt — 4 von 4 echt, gegen die ~25-%-Fehlbefundquote des Projekts bemerkenswert. T2-1 wurde auf dem **richtigen** Weg geschlossen (Gitter nach oben erweitert statt Schranke gemeldet; der Prüfer-Testfall fällt von 2,918 LU auf 0,000000000 LU; Zwei-Pegel-Korpus jetzt echtes Bein). Der U10-Wächter existiert und wurde beim Fallen vorgeführt (fail-open rein ⇒ Bein rot ⇒ byteweise zurück). Drei Selbstaudit-Nachträge derselben Session: T2-4-Regel war zu scharf (`bbe4b3f`), zweiter Beweislauf (`1d86083`), Exit-4-Fehlalarm nachgemessen statt geglaubt (`0b23cf5`) — außerdem fand ihr Selbstaudit im eigenen G4-Testfall einen `DBL_MAX`-Überlauf und zog den erreichbaren Rand nach. Drei Kanon-Läufe 26/26, Exit 0 (14:41, 15:01, 15:42). Hub und NEXT-SESSION nachgezogen (`87fe56b`). Kein PASS erklärt — regelkonform.
- **Gestützt auf:** Repo-Messung (Manifest §9 vollständig, Kanon-Rohblöcke, Commits gepusht, Baum sauber bis auf die bekannten fremden Dateien).
- **Nicht geprüft:** Die Fixes selbst an den Quellen (Rolle des ausstehenden frischen Prüfers — S9 UND S10–11 warten jetzt beide auf ihr PASS, Vorschlag G2-Bündelung). Der FL-Transportpfad bleibt nie live gefahren — geht jetzt als „bei dir"-Karte an den User.
- **Weiter mit:** S12–13 (`SONDE-009`, FeatureEngine v2 — Gate-Text: „Drop/Seek/Loop trennt jedes offene Fenster") + FL-Transportpfad-Frage an den User.

## 2026-08-23 spät · SONDE-008 · pruefung (T2)

- **Session:** „SONDE-008 T2 — Frischkontext-Prüfung" (`4bb58934-da4e-4249-9672-94f3eaaa2337`), Opus 5 mit 1M.
- **Ergebnis:** **NEEDS_WORK — der Kern hält, vier Befunde** (Manifest §8, Commits `3c4d215` + `014248f`; Hub von der Session nachgezogen). Die Queue überstand einen härteren Stress als ihr eigenes Bein: 4 Mio. Samples mit parallelem Schreiber/Leser und absichtlich zu kleinem Ring (~8000 erzwungene Drops) — 0 zerrissene Blöcke, 0 Umordnungen, jede Lücke markiert; die Loop-Rückhaltung hält. Befunde: **T2-1** `unsicherheitLu()` meldet 0 LU, wo 2,918 LU Fehler stehen (Über-Gitter-Eimer = zweite, unbegrenzte, ungemeldete Quantisierung; das Golden prüft dort nur EINEN Pegel und kann nicht scheitern) · **T2-2** der U10-Riegel ist wirksam (Positivfall gemessen), aber ungedeckt — fail-open zurückgebaut ⇒ 4/4 Beine weiter grün · **T2-3** Anlaufwechsel: 0/3 Altblöcke an `startFolge` erkennbar, anders als der Kommentar behauptet · **T2-4** Seek bei gestopptem Transport erzeugt keine Epochengrenze, Entwurf §32.3 führt Seek als Grenze.
- **Gestützt auf:** Repo-Messung — Manifest-Kopf (T2 ☑, NEEDS_WORK), §8-Befundtabelle mit konkreten Messwerten je Befund, beide Commits gepusht; die Methode des Prüfers (Mutationen byteweise zurückgestellt, U10-Positivfall ausdrücklich gemessen) ist im Manifest protokolliert.
- **Nicht geprüft:** Die vier Befunde habe ich nicht selbst an den Quellen nachgemessen — die Nacharbeit misst beim Schließen (T3-Regel). Der Prüfer benennt als größte ungedeckte Fläche den **echten FL-Transportpfad** (nichts lief in FL Studio; der Weg, über den das Plugin live seine Zeitinformation bekommt, wurde gelesen, nie gefahren) — das live zu messen braucht absehbar eine Installation des neuen Builds = User-Klick (NAK-41). Diese Entscheidung stelle ich mit der S12-13-Runde, nicht jetzt.
- **Weiter mit:** Nacharbeit-Session für T2-1…T2-4 — alle vier (Fund = Fix); T2-1/T2-2 blockieren ein PASS laut Prüfer.

## 2026-08-23 abends · SONDE-008 · bau (S10–11)

- **Session:** „S10–11 SONDE-008 — StampedAudioQueue" (`0fdf9dc4-e27e-4dcc-90af-1461081b1106`), Opus 5 mit 1M.
- **Ergebnis:** Gebaut in vier Commits — `7fa1cf5` (StampedAudioQueue mit Ganz-oder-gar-nicht-Semantik statt Teilblöcken, Ein-Block-Quarantäne mit Halterückgabe für Loop-Sprünge, fixed-memory Loudness), `2bd6d6e` (Manifest + Fix am Beglaubigungsriegel: **drei Quellorte fehlten der Frische-Liste**, zwei davon seit zwei Arbeitsschritten — die gedruckte Ortsliste erzeugt sich jetzt selbst), `c3e9f2d` (Hub), `fb437e8` (Selbstaudit, drei Nachlese-Funde geschlossen). Manifest-Kopf **T1 ☑ / T2 ☐**; Kanon 24 → **26**, Lauf 14:41: **26/26, Exit 0, beglaubigt**; Bitidentität ausdrücklich belegt (A1 NullTest, A2 Golden, A3 Markierung, alle Exit 0). **U10 umgesetzt** (§5 des Manifests): Hör-Markierung nur noch mit gültigem „spielt" — **NAK-24/NAK-35 geschlossen**; Grundlage ist der Register-Entscheid vom 22.08. („Nein, nur mit Signal"), kein eigenmächtiger Produktentscheid. Der Stress-Testfall fand vor dem Merge einen echten Fehler (durchweg extrem lautes Material → kein Loudness-Wert) und eine eigene Falschbehauptung („exakt gleich" statt ULP-gleich) — beide im selben Änderungssatz berichtigt.
- **Gestützt auf:** Repo-Messung, nicht Selbstbericht — git log, Manifest-Kopf, Kanon-Rohblock (26/26, Exit 0), A1/A2/A3-Zeilen, §5-U10-Abschnitt; alles gepusht; Hub von der Session selbst nachgezogen (`c3e9f2d`).
- **Nicht geprüft:** Der Code selbst — Queue-Semantik, Halterückgabe, Quarantäneklasse, U10-Verriegelung sind Behauptungen des Erbauers, bis der frische Prüfer sie gemessen hat (genau dafür startet jetzt T2). Die drei Nachlese-Funde aus `fb437e8` habe ich nicht einzeln nachgemessen.
- **Weiter mit:** T2-Prüf-Session für SONDE-008 — Basispunkt **`e330052`** (selbst gerechnet: Elterncommit von `7fa1cf5`), Bruchauftrag Blockgrößen-Fuzz über `maximumExpectedSamplesPerBlock` hinaus (Bauaufteilung-Vorgabe für dieses Ticket).

## 2026-08-23 nachmittags · SONDE-007b · nacharbeit

- **Session:** „S9 Nacharbeit — T2-Befunde schließen + T1" (`cb6005c4-e646-424a-8d7e-c87db4cd1d51`), Opus 5 mit 1M.
- **Ergebnis:** Alle vier Befunde geschlossen (§6.1–§6.4; Commits `d93d536`, `9573546`), Auslieferungseinheit auf den Bundle-**Ordner** gehoben, neues Kanon-Bein **A18** (Installer-Gegenpfad läuft in einer Sandbox bei jedem Beweislauf mit), T1-Sechserliste gefahren mit zwei behobenen Funden (§6.6). Beim Reparieren fünf weitere echte Fehler gefunden und geschlossen — darunter tote `NOTFOUND`-Vergleichszweige im Identitätsriegel und zwei Rückweg-Fehler beim **ersten tatsächlichen Lauf** des Rückwegs überhaupt (wäre ab der zweiten Installation abgestürzt). Kanon 23 → **24**; Lauf 13:25: **24/24, Exit 0, beglaubigt** (Rohblock im Manifest). Manifest `0d5ec3d`, Hub/NEXT-SESSION `7557165`, Selbstaudit-Nachtrag `e2d2a9e`. Kein PASS erklärt — regelkonform.
- **Gestützt auf:** Commits, Manifest-§6 und Kanon-Rohzeile im Repo gelesen; alles gepusht (origin eingeholt); keine fremde RUNNING-Session mehr (OpenWiki fertig, `e8b71a0`).
- **Nicht geprüft:** Die Fixes selbst habe ich nicht an den Quelldateien nachgemessen — das ist die Rolle des noch ausstehenden frischen Prüfers (§6.7); die CLAUDE.md-Zeile 12→13 bleibt unter NAK-55 offen (fremde uncommittete Änderungen liegen weiter darauf).
- **Abwägung vor S10:** Die Bauaufteilung warnt nach der S10-Zeile: „Kein Harness-Autopilot für die Audiothread-Tickets (S10–11, S26–28) … er gehört unter direkte Beobachtung." Das ist eine Arbeitsregel aus der Planungszeit (kein User-Entscheid mit Zitat) und zielte auf den unbeaufsichtigten Bau-Harness. Der Dirigent ist kein Autopilot-Loop: je Ticket eine dedizierte Session mit Manifestpflicht und Frischkontext-T2, alles im Hub sichtbar. Der jüngere User-Entscheid (23.08., „Durchlaufen bis der Plan leer ist") trägt; ich fahre S10–11 — mit verschärften Audio-Beweispflichten im Auftrag und vorheriger Handy-Meldung, damit der User eingreifen kann, wenn er es doch beobachten will.
- **Weiter mit:** S10–11 (`SONDE-008`) Bau-Session. Offen für S9: frisches Prüfer-PASS auf den nachgebesserten Stand — Vorschlag: mit den gleichartigen Rückständen (S5-Runde-5, S8) am Gate G2 bündeln, dort laufen ohnehin fremde Prüfer.

## 2026-08-23 mittags · SONDE-007b · pruefung (T2 Teil B)

- **Session:** „S9 T2 Teil B — Frischkontext-Prüfung" (`2e747d9a-d613-463f-a2d9-5ef6e6c8fc9e`), Opus 5 mit 1M-Kontext (User-Zusatz vom 23.08.; xhigh ist über die Spawn-API nicht setzbar — nur Basisvariante + `-1m`).
- **Ergebnis:** Urteil **NEEDS_WORK** — vier Befunde T2-2…T2-5, alle offen (Absicht: wer prüft, repariert nicht). Manifest §5.5–§5.9 in `docs/beweise/SONDE-007b.md` (Commit `51b4fb2`), Hub nachgezogen (`1c99708`, Drift 0), Sammelzeile NAK-55 im Offen-Set, eine abgeschriebene Zahl berichtigt (12→13 Kern-Verbraucher, §5.8). Eigener Kanon-Lauf des Prüfers: 23/23 grün, Exit 0 — beglaubigt.
- **Gestützt auf:** Commits und Manifest im Repo gelesen, nicht den Selbstbericht; beide Commits gepusht (origin eingeholt). Basispunkt `4b500a4` vor dem Spawn selbst nachgerechnet (Elterncommit von `8e32baf`). Methode des Prüfers nachvollziehbar protokolliert: Mutationsproben mit byteweiser SHA-256-Rückstellung, der eigene mtime-Eingriff offen benannt und durch echten `-Bauen`-Lauf geschlossen statt durch Zeitstempel.
- **Nicht geprüft:** Die vier Befunde habe ich nicht selbst an den Quelldateien nachgemessen (der Dirigent baut und prüft nicht; die Nacharbeit misst beim Schließen an der Quelle, T3-Regel). Teil A wurde von Teil B nicht erneut kontrolliert (§5.9, ehrlich benannt). T1 für S9 steht weiter aus — geht in den Nacharbeit-Auftrag (Sechserliste aus Bauaufteilung §2).
- **Parallellauf, beurteilt statt gewartet:** Während der Runde lief die fremde Codex-Session „OpenWiki-Aktualisierung" (Commit `e8b71a0`). §3.1 sagt „warte"; der Grund der Regel ist der geteilte Build-Ordner, und diese Session schrieb nachweislich nur `openwiki/`-Doku ohne Build — als unkritisch beurteilt, nichts kollidierte.
- **Entscheid des Dirigenten für die Nacharbeit** (Technik, kein User-Entscheid): alle vier Befunde schließen, nicht nur T2-2/T2-3 — T2-4 ist eine Regelzeile, und für T2-5 gilt: Auslieferungseinheit ist der Bundle-**Ordner** (moduleinfo.json fährt mit, der Rückweg lässt keine leere Hülle). Die Defer-Empfehlung „T2-4/T2-5 mit SONDE-010" ist damit überstimmt (Hausregel: Fund = Fix). `CLAUDE.md` (die 12→13-Zeile, NAK-55) bleibt unangetastet, solange fremde uncommittete Änderungen darauf liegen.
- **Weiter mit:** Nacharbeit-Session für T2-2…T2-5 + T1; danach S10–11 (`SONDE-008`).

## 2026-08-23 vormittags · SONDE-007b · rundenstart T2 Teil B

- **Session:** Dirigent (`85577774-aa79-47c8-b5a5-3ec14ec5fa4e`) spawnt die Prüf-Session; User-Zusatz beim Aufruf: „spawne immer einner opus 5 model in xhigh 1m".
- **Vorher gemessen:** `git status` (verwaiste Fremddateien benannt, nicht angefasst) · Hub `holen` („Nichts Neues") · Basispunkt selbst gerechnet · fremde RUNNING-Session als reine Doku-Session eingestuft.
- **Meldekanal:** claude.ai-Routine „Nakama: Dirigent-Meldung" (`trig_01BUKf1i5Y9ztqGkA6Ev4eff`, Push) angelegt und getestet — User ist afk („schreib mir falls etwas wichtiges ist über die claude routine").
