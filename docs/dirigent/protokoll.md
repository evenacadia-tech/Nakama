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
