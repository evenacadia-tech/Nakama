# NAK-380 Wiederprüfung 1 der Etappe 4 — Urteil (Codex gpt-6-astra, Effort xhigh, lesend)

Thread `01a0da9f-c95a-70c2-84dc-cf84e06ff0cb`, 26.09.2026, 00:11:39 bis 00:17:29 Uhr, EXIT=0, HEAD vor und nach dem Lauf `20b30fcb0f351705cafe2bc990b7ddaa0b1bb44d`; Auftrag `docs/beweise/roh/NAK-380-wiederpruefung-e4-1-auftrag.txt` (Vorlage B, Fixdiff `7b4f7938...20b30fcb` über vier Testpfade). Wortlaut des Prüfers unverändert:

---

URTEIL: NEEDS_WORK — D1 und D5 nicht vollständig geschlossen, keine zusätzlichen Brüche festgestellt.

Geprüft: vollständiger gebundener Fixdiff, geltende Referenzen, Aufrufwege und Mutationsstellen; ausgeschlossene Produktpfade in beiden gemessenen Diffs leer, Fixtures unverändert, keine Golden-Lockerung, Kalibrierwerte 125 Cent und 0,10 dB je Bin unverändert, bestehende Zusageprüfungen erhalten.

Nicht geprüft: auftragsgemäß keine Builds, Testläufe, Mutationen oder Produktprozesse ausgeführt; vorhandene Laufbelege nur gelesen.

Schließungsbelege am ZIEL; folgende Dateinamen relativ zu eq-copilot/plugin/tests/:
D1 — nein: Restblocklogik vorhanden in AnalysisGoldenTestMain.cpp:1344-1370, Sonde013DynamicsTest.cpp:766-777 und :963-975 sowie Sonde013EventWireTest.cpp:757-777; M-62 abgesichert bei AnalysisGoldenTestMain.cpp:1735-1736, Beobachtung jedoch ohne Samplezahlprüfung bei :1589-1590
D2 — ja: eigene Frameprüfung auf 2048 in Sonde013DynamicsTest.cpp:1086-1089, zusätzliche Hopprüfungen bei :1124-1131
D3 — ja: unabhängige Median-/MAD-/Schwellenreferenz in Nak380Pruefsignale.h:490-508; Engine-Wertprüfung in Sonde013EventWireTest.cpp:737-743, Drahtgleichheit bei :851-855, positiver Median bei :908-912 verlangt; konstruierter Fall med=180 in Sonde013DynamicsTest.cpp:798-825
D4 — ja: Kopffunktion in Nak380Pruefsignale.h:453-471, B16-Aufrufe vor beiden Nutzern in Sonde013EventWireTest.cpp:954 und :1010; B5 einschließlich Beobachtung über AnalysisGoldenTestMain.cpp:1441-1442 und :1520
D5 — nein: eigene Verlustprüfungen für M-62 in AnalysisGoldenTestMain.cpp:1737, M-63/M-65 in Sonde013EventWireTest.cpp:898-904 und :1063-1069 sowie weitere Zusageläufer vorhanden; Beobachtung weiterhin nur Ausgabe bei AnalysisGoldenTestMain.cpp:1588-1590
D6 — ja: empirische Abschnittsnormierung in Nak380Pruefsignale.h:285-293, unabhängige Energiemessung und Prüfung in Sonde013DynamicsTest.cpp:739-760
D7 — ja: numerische Rechenbudgets in Sonde013DynamicsTest.cpp:687-709, :723-730, :873-882 und :1044-1058 sowie AnalysisGoldenTestMain.cpp:1680-1693 und :1809-1826; exakter M-60-Vergleich bei :1847
Z1 — ja: Wertprüfung vor der Drahtfilterung in Sonde013EventWireTest.cpp:737-743 und :815-816, Schemahälfte ausdrücklich Regressionswache bei :944-951; Klammermutation trifft die Wertprüfung

HEAD Beginn: 20b30fcb0f351705cafe2bc990b7ddaa0b1bb44d
HEAD Ende: 20b30fcb0f351705cafe2bc990b7ddaa0b1bb44d

FERTIG Wiederprüfung 1 Etappe 4, NEEDS_WORK, 2 offene Befunde, 0 Brüche

Full review comments:

- [P2] Verriegele die Samplezahl auch im Beobachtungslauf — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:1589-1590
  Bei `--nak380-beobachtung I3-100` und `I3-150` werden Samplezahlen lediglich ausgegeben; `nak380SamplesGeprueft` wird nicht aufgerufen. Die vorhandene Gegenprobe `m62-rest` am gemeinsamen Läufer würde deshalb trotz nur 1 439 744 statt 1 440 000 gespeister Samples weiterhin Exit 0 ergeben: Die Rosa-Prüfung untersucht den unveränderten vollständigen Puffer, und weitere Prüfungen fehlen. Das ist statisch nachvollzogen, nicht ausgeführt. D1 bleibt hinsichtlich der ausdrücklich verlangten Samplezahlverriegelung der Beobachtung offen ([Nacharbeitsauftrag:15](docs/beweise/roh/NAK-380-nacharbeit-e4-1-auftrag.txt#L15)); ergänze dort die vorhandene Samplezahlprüfung.

- [P2] Prüfe Ringverluste auch im Beobachtungsmodus — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:1914-1915
  Im Beobachtungsmodus hängt der Exitcode ausschließlich von E-380-13 ab; der gemeldete Ringverlust erreicht keine Prüfung. Unter der vorhandenen Produktmutation `n1-verlust` würden die Klickereignisse verworfen und gezählt, die unveränderte Rosa-Selbstprüfung bliebe jedoch erfolgreich und der Modus endete mit Exit 0. Diese Gegenprobe ist lesend nachvollzogen, nicht ausgeführt. Damit bleibt D5 für diesen ausdrücklich eingeschlossenen Läufer offen ([Nacharbeitsauftrag:36](docs/beweise/roh/NAK-380-nacharbeit-e4-1-auftrag.txt#L36)); rufe auch dort `nak380VerlustGeprueft` auf, ohne eine neue Zusage zur beobachteten Trefferrate einzuführen.