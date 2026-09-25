# NAK-380 Erstprüfung 4 — Urteil (Codex gpt-6-astra, Effort xhigh, lesend)

Thread `01a0da2c-49f4-7e23-b226-44ba2e5f7f68`, 25.09.2026, 22:05:30 bis 22:22:57 Uhr, EXIT=0, HEAD vor und nach dem Lauf `1e4a7010915fc8a34765aa1fd6d1b10551cd2fd3`; Auftrag `docs/beweise/roh/NAK-380-erstpruefung-4-auftrag.txt` (Vorlage A, Prüfbereich `21853aa1...1e4a7010` über 16 Ticketpfade). Wortlaut des Prüfers unverändert:

---

URTEIL: NEEDS_WORK

Geprüft: vollständiger freigegebener Diff, geltende Referenzen, Test- und Mutationsstellen, Versionsgleichheit und Zeilenenden; keine Änderungen an den ausgeschlossenen Produktpfaden, Fixtures, Bandgittern oder FlatBuffers und keine uncommitteten Änderungen im Prüfbereich festgestellt.

Nicht geprüft: keine Builds, Testläufe, Mutationen oder Produktprozesse ausgeführt; Laufzahlen und Rotprotokolle ausschließlich gelesen, daher kein eigener Kanon- oder Laufzeitnachweis.

Offene Punkte: (1) Kalibrierfolge und Werte konsistent, M-62-Beleg durch Pufferüberlauf beeinträchtigt; (2) I3 mit 150 ms korrekt über der Verdeckungsgrenze, I2 unverändert; (3) Pflichterfassungen gelesen, unveränderter Bandfluss und getrennte Fingerprint-Zuführung bestätigt; (4) Version 20260927 und einzelne A5-Konstantenprüfungen konsistent; (5) Produktformel korrekt, M-63-Prüfungen unvollständig; (6) A-6, gemeinsamer Reset und Peakfreigabe nach 32 Frames nachvollziehbar, M-62-Test defekt; (7) Heapträger und Queue-Größe nachvollziehbar, Allokationsmessung nur fremdbelegt; (8) benannte Ersatzmutationen statisch nachvollziehbar, M-62 wegen undefinierten Verhaltens nicht belastbar; (9) bestehende Wachen und M-72-Wiederverwendung nachvollziehbar, aktuelle Laufresultate nicht selbst bestätigt.

HEAD Beginn: 1e4a7010915fc8a34765aa1fd6d1b10551cd2fd3
HEAD Ende: 1e4a7010915fc8a34765aa1fd6d1b10551cd2fd3

FERTIG Erstprüfung 4, NEEDS_WORK, 7 Defekte, 0 Lücken, 0 Härtungen

Full review comments:

- [P2] Begrenze den letzten M-62-Block auf die verbleibenden Samples — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:1813-1817
  DEFEKT: Der Signalpuffer enthält 720000 Samples, aber die Schleife verarbeitet ausschließlich vollständige 512er-Blöcke. Beim letzten Durchlauf ist `strom = 719872`; nur 128 Samples bleiben, anschließend werden 384 Werte außerhalb des Vektors gelesen. Damit enthält bereits der unveränderte M-62-Lauf undefiniertes Verhalten und liefert keinen verlässlichen Grenzbeweis. Reproduktion zur Nacharbeit: `EqCopAnalysisGoldenTest --nak380 M-62` mit Speicherprüfung; hier ausschließlich statisch festgestellt. Wie im benachbarten Korpusläufer müssen Kopierlänge, `sampleCount` und Fortschreibung auf die Restlänge begrenzt werden.

- [P2] Prüfe in M-48 auch den Frame der einzigen Auslösung — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:874-877
  DEFEKT gegen [M-48](docs/beweise/NAK-380.md#L938): `folge()` verwirft die Ereigniszeiten, sodass nur „genau eines“, nicht „am Frame mit 500“ geprüft wird. Die formgültige Gegenprobe `sf >= d.sfVorher` → `sf <= d.sfVorher` in `Spektrum.h` würde beim Aufruf `EqCopSonde013DynamicsTest --nak380 M-48` weiterhin genau ein Ereignis liefern, allerdings bei 450 statt 500: 500 scheitert am Vergleich, 450 löst aus und 400 liegt danach innerhalb der Sperrzeit. Diese Kontrollflussfolge ist statisch nachvollzogen, nicht ausgeführt. Prüfe zusätzlich den erwarteten Frame beziehungsweise `stromSample == 2048`.

- [P2] Vergleiche die serialisierte Stärke mit der zugesagten Formel — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde013EventWireTest.cpp:701-705
  DEFEKT gegen [M-63](docs/beweise/NAK-380.md#L953): Die Wire-Auswertung prüft ausschließlich Endlichkeit und `[3,1000]`, nicht den zugesagten Zahlenwert `min(1000, κ·(SF−med)/(T_eff−med))`. Beispielsweise bliebe die formgültige Änderung des Produktzählers von `(sf - med)` zu `sf` innerhalb dieser Grenzen, obwohl Ereignisse mit positivem Median falsche Stärken tragen. Auch die zusätzliche M-60-Einheit erkennt das nicht, weil dort `med == 0` gilt. Gegenprobe zur Nacharbeit: diese Änderung mit `EqCopSonde013EventWireTest --nak380 M-63`; hier nicht ausgeführt. Es fehlt der Vergleich jedes Wire-Wertes mit einer unabhängig hergeleiteten Referenz.

- [P2] Prüfe die Rosa-Signale vor ihren B16-Nutzern — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde013EventWireTest.cpp:791-794
  DEFEKT gegen [E-380-13 und §7.1](docs/beweise/NAK-380.md#L1099-L1104): Sowohl das Rosa-Material zum Aufwärmen als auch I1 werden unmittelbar verändert und verarbeitet, ohne vorher ihre integrierten Oktavbandleistungen zu prüfen. `rosaMono()` führt diese Prüfung nicht selbst aus; die vorhandene Selbstprüfung liegt ausschließlich im anderen Testprogramm B5. Ein isoliertes `EqCopSonde013EventWireTest --nak380 M-63` trägt deshalb nicht die geforderte Vorbedingung „vor jedem Nutzer“. Lesereproduktion: `git grep -n rosaGroessteOktavdifferenzDb -- eq-copilot/plugin/tests/Sonde013EventWireTest.cpp` liefert keinen Aufruf. Prüfe beide unveränderten Rosa-Puffer vor dem Einsetzen von Stille und Klicks.

- [P2] Verriegele Ringverluste auch in M-62 und M-63 — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde013EventWireTest.cpp:800-803
  DEFEKT gegen [§7.3](docs/beweise/NAK-380.md#L1154-L1158): M-63 erfasst Verluste, verwendet sie aber lediglich im Bericht; die Zusageprüfungen können trotz verlorener Ereignisse grün bleiben, solange genügend gültige Ereignisse übrig sind. Auch der eigene M-62-Läufer prüft `ereignisseVerworfen()` nicht. Die ausdrücklich verlangte Bedingung „Verlustzähler am Ende 0, sonst Fall rot“ fehlt somit in beiden neuen Läufern. Lesereproduktion: `git grep -n -E 'verloren|ereignisseVerworfen' -- eq-copilot/plugin/tests/Sonde013EventWireTest.cpp`; die neue Nullprüfung steht nur im M-65-Zweig. Ergänze die Verlustprüfung unabhängig von Ereigniszahl und Stärke.

- [P2] Normiere die beiden Rauschabschnitte des M-47-Signals — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:836-839
  DEFEKT gegen die Vorbedingung von [M-47(a)](docs/beweise/NAK-380.md#L937): Verlangt sind zwei Abschnitte mit fest normierter Rahmenenergie. Hier werden dagegen unnormierte Box-Muller-Samples lediglich mit den Soll-Amplituden multipliziert; `sigma = 1` bezeichnet die theoretische Standardabweichung, nicht die tatsächliche Energie der endlichen Abschnitte. Weder eine Abschnittsnormierung noch eine Energieprüfung findet statt. Lesereproduktion: `git grep -n -A 8 'inline std::vector<float> weissMono' -- eq-copilot/plugin/tests/Nak380Pruefsignale.h`. Damit misst `--nak380 M-47` nicht die exakt vereinbarte Vorbedingung; stelle die feste Energie für beide Abschnitte ausdrücklich her.

- [P2] Leite die neuen numerischen Prüftoleranzen her — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:688-690
  DEFEKT gegen die verbindliche Präzisierung [„Jede Toleranz … trägt im Testkommentar ihre Herleitung“](docs/beweise/roh/NAK-380-etappe-4-auftrag.txt#L27): Bei M-42 ist zwar der Sollfluss hergeleitet, nicht aber die Toleranz `1e-9`; dasselbe gilt für `1e-6` in M-47(b) sowie `1e-4` und `1e-5` in den neuen M-60-Einheiten. Diese Grenzen sind damit nicht als referenzbasierte Fehlerbudgets nachvollziehbar. Lesereproduktion: `git grep -n -E '1\.0e-(9|6|4|5)' -- eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp`. Begründe die jeweiligen Grenzen anhand der tatsächlich ausgeführten double-Rechnung beziehungsweise float-Konvertierung.