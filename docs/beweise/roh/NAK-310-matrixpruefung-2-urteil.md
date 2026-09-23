V-1 bleibt wegen Frist- und Gegenfallbeleg offen; zusätzlich verletzen M-108 und M-111 ausdrückliche Matrixgrenzen. Geprüft wurden nur der gebundene Diff und die genannten Quellen, ohne Builds, Tests, Produktprozesse, Pipes oder Aufgaben. HEAD war zu Beginn und am Ende abbc8ddaa1403168a15c9214f2caeb4bbd2a6c8f (= origin/master); einzig die fremde Auftragsdatei blieb untracked.

Full review comments:

- [P1] Prüfe Riegel 3 nicht über die lebende Installationsaufgabe — C:\Users\phili\nk-doku\docs\beweise\NAK-310.md:792-792
  Wenn SYSTEM das Benutzer-FL nicht sieht – genau der von M-111 zu prüfende Fehlerfall –, läuft `\Nakama\installieren` an Riegel 3 vorbei und kann den echten Program-Files-Stand sowie Journal und Log verändern, während FL geöffnet ist. Damit verlangt die neue Zeile entgegen der Grenze in `docs/beweise/NAK-310.md:1055-1059` einen potenziell mutierenden Produktionslauf; zudem liegt ihr Rotbeweis nur bei der A18-Sandbox M-20, nicht an der SYSTEM-Zusage selbst. Diese Prüfung muss fail-safe außerhalb der lebenden Installationsaufgabe erfolgen.

- [P2] Belege die feste Ein-Sekunden-Schließfrist — C:\Users\phili\nk-doku\docs\beweise\NAK-310.md:890-890
  Wenn `TAKTFRIST` auf etwa 1,5 s regressiert, bleibt M-96 wegen `buehne(true, LANG)` und manueller Auslösungen grün, während M-97 weiterhin innerhalb seiner 2-s-Toleranz besteht. Damit erzwingt kein Rotbeweis die in R-310-7 zugesagten höchstens 1 s (`docs/beweise/NAK-310.md:2115-2116`); die Produktfrist muss unabhängig von der toleranteren Ende-zu-Ende-Frist festgehalten werden.

- [P2] Entferne die R-310-8-Wache aus M-108 — C:\Users\phili\nk-doku\docs\beweise\NAK-310.md:871-871
  R-310-8 verlangt ausdrücklich „Keine neue Matrixzeile“ (`docs/beweise/NAK-310.md:2132-2136`), M-108 fügt aber eine eigenständige zweite Hälfte samt `hoermarkierung`-Suche und Mutation genau für diese Regel hinzu. M-108 kann als von R-310-9 geforderte A4-Behauptungszeile bestehen bleiben, die Setzerbegrenzung darf jedoch nur in §7.0, §7.3 und NAK-377 verneint werden.

- [P2] Halte die Vorbedingung im Rotbeweis von M-98 stabil — C:\Users\phili\nk-doku\docs\beweise\NAK-310.md:891-891
  Der beschriebene Bruch lässt die Sitzungsfassade „beendet“ statt „lebt“ melden und verändert damit die Vorbedingung des Gegenfalls (b), nicht dessen Zusage. Er beweist folglich den Fail-closed-Fall, aber nicht, dass eine zweite Verbindung bei weiterhin lebendem markierendem Prozess beantwortet bleibt, obwohl R-310-7 für jeden Gegenfall einen Rotbeweis an der Zusage verlangt (`docs/beweise/NAK-310.md:2120-2126`).

- [P2] Isoliere die Handle-Leckprüfung vom parallelen A4-Lauf — C:\Users\phili\nk-doku\docs\beweise\NAK-310.md:893-893
  Im kanonischen A4-Lauf startet `cargo test` ohne `--test-threads=1` (`tools/beweise.ps1:515`), während andere Broker-Tests parallel Windows-Handles öffnen und schließen. Der prozessweite `GetProcessHandleCount` kann deshalb nach 100 Zyklen trotz korrekter Sitzungsgriff-Lebensdauer vom Ausgangswert abweichen oder ein Leck durch gegenläufige Änderungen verdecken; hierfür ist ein eigener Fassadenzähler oder ein tatsächlich isolierter Test erforderlich.