URTEIL: NEEDS_WORK — vier DEFEKTE in den zugesagten Prüfungen, kein zusätzlicher Produktrechenfehler festgestellt.

Geprüft: vollständiger gebundener Diff, Gate-Abgleich, Matrix und Quellen; Punkte 1–7: (1) M-23-Lücke geschlossen, diskrete Referenz 2,544 LU und berichtete 2,600 LU verträglich; (2) Versionsbindung 20260926 einschließlich Broker korrekt, Altlesbarkeit im Code erhalten; (3) M-28(b)-Ausnahme zulässig, Ratenwechsel anderweitig getragen; (4) Ersatzmutationen formgültig, M-38-Prüfung jedoch unvollständig; (5) kein Audio-Pfad geändert, Wiederverwendung des M-20-Rotbelegs nachvollziehbar; (6) ausschließlich Diagnose-Fixture geändert, Diagnoseversion unverändert, selbst gelesener SHA-256 entspricht dem Erzeugerbericht; (7) Toleranzfenster und Saaten stimmen mit der Matrix überein, 196 belegte Pink-Bänder sind ein gelesener Messbericht.

Nicht geprüft: keine Builds, Tests, Mutationen, Erzeuger oder Produktprozesse ausgeführt; Kanon-Erfolg und ausschließliche Erzeugerherkunft nicht unabhängig nachgewiesen, historische Laufberichte nur gelesen.

HEAD Beginn: 8308a3d80146f1119203127e927c1f746fcd4ec9
HEAD Ende: 8308a3d80146f1119203127e927c1f746fcd4ec9
FERTIG Erstprüfung 3, NEEDS_WORK, 4 Defekte, 0 Lücken, 0 Härtungen

Full review comments:

- [P2] Prüfe jedes unbelegte Perzentil einzeln auf NaN — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/GoldenTestMain.cpp:610-613
  DEFEKT gegen M-38 („NaN, nie 0“): Bei einem leeren Teilblockhistogramm meldet diese Prüfung erst dann einen Fehler, wenn alle drei Perzentile endlich sind. Beispielsweise wird P95 = 0 bei weiterhin NaN in P10/P50 akzeptiert. Eine versehentliche Initialisierung von `perzentilP95` mit 0 statt NaN würde den Fall `EqCopGoldenTest.exe eq-copilot/fixtures --nak380 M-38` deshalb nicht rot machen (statisch nachvollzogen, nicht ausgeführt). Für unbelegte Bänder muss jedes der drei Felder ausdrücklich NaN sein; siehe auch [CLAUDE.md:189–190](CLAUDE.md#L189-L190).

- [P2] Stelle die vorgeschriebenen M-38-Läufe vollständig her — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/GoldenTestMain.cpp:571-572
  DEFEKT gegen die exakte Vorbedingung von M-38: Gefordert sind W4 über 60 Sekunden und ein zweiter Lauf mit Reset bei 30 Sekunden. Tatsächlich läuft dieser Test nur 30 Sekunden, setzt zurück und speist anschließend eine Sekunde; außerdem wird `floor(n/8)` nach dem Neustart nicht erneut geprüft. Damit fehlt der zugesagte vollständige Start/Stop-Vergleich, nicht lediglich zusätzliche Härtung; dazu passt die Lebenszyklusregel [AGENTS.md:50–52](AGENTS.md#L50-L52). Lesereproduktion: `git grep -n -A63 'void nak380M38' 8308a3d8 -- eq-copilot/plugin/tests/GoldenTestMain.cpp`.

- [P2] Prüfe das Gültigkeitsflag auch vor dem ersten Teilblock — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/GoldenTestMain.cpp:620-623
  DEFEKT gegen [M-38](docs/beweise/NAK-380.md#L913): „perzentileGueltig erst mit mindestens einem Teilblock“ wird nur auf der bereits gültigen Seite geprüft. Nach der eingespeisten Sekunde besitzen Mitten- und Höhenstufe schon vollständige Teilblöcke; `irgendeinBlock` ist daher immer true. Selbst ein bedingungslos gesetztes Produktflag würde `EqCopGoldenTest.exe eq-copilot/fixtures --nak380 M-38` bestehen (statischer Gegenbeleg, nicht ausgeführt). Ergänze eine Auswertung ohne vollständigen Teilblock, beispielsweise nach 0,1 Sekunden, und fordere dort ausdrücklich `perzentileGueltig == false`.

- [P2] Erfasse alle kName-Werte eines Registereintrags — C:/Users/phili/Projekte/Nakama/tools/eq-copilot/pruefe_v3_vertrag.py:2389-2393
  DEFEKT gegen [M-30](docs/beweise/NAK-380.md#L905): Der vorhandene Registereintrag enthält `kEreignisPlaetze = 64, kVerteilungPlaetze = 64`, aber das verankerte `re.match` prüft ausschließlich das erste Paar. Eine versehentliche Abweichung beim zweiten Wert bleibt somit unbemerkt; dessen `inline constexpr`-Definition fehlt zusätzlich im ausschließlich auf `static constexpr` begrenzten Codeindex. Gegenprobe, nicht ausgeführt: Den zweiten Registerwert auf 65 setzen und `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --nak380 M-30` aufrufen; keine der vier Prüfungen erfasst diese Abweichung. Beide Paare müssen gegen ihre Codekonstanten geprüft werden.
