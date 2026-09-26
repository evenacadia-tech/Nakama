# NAK-380 Wiederprüfung 1 der Etappe 5 — Urteil des Codex-Prüfers

Prüfer: Codex `gpt-6-astra`, Effort xhigh, lesend (`read-only`), Thread `01a0dbe6-d13f-7551-9418-ed9f9d85f0fe`; Lauf 26.09.2026 06:08:51 bis 06:13:56 Uhr über `tools/dirigent/codex-lauf.ps1` (Kennung `nak380-e5-w1`); HEAD vor und nach dem Lauf `76c9b665247043ca7b3f81978eca2fb3e357a318`; Auftrag `docs/beweise/roh/NAK-380-wiederpruefung-e5-1-auftrag.txt` (Vorlage B, Prüfbereich `11ad06ba...76c9b665` über `Sonde013StereoGoldenTest.cpp` und `Sonde013DynamicsTest.cpp`, Befunde D1, D2, D3, Z1). Bericht wörtlich:

URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.

Geprüft: ausschließlich der gebundene Fixdiff samt Aufrufketten, Matrixfassung §50.2 und Mutationsbelegen; Dateikürzel G = eq-copilot/plugin/tests/Sonde013StereoGoldenTest.cpp, D = eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp; D1: ja — G:915-942 prüft sämtliche Snapshots 7 bis n mit verriegelter Anzahl, Bit-, Kohärenz- und Referenzprüfung, m75-frueh trifft ausschließlich frühere Snapshots bei grünem M-74, Bandsumme und Vorbedingungsmutation fallen an ihren Zusageprüfungen, Laufzeit bleibt am letzten Snapshot; D2: ja — D:912 verriegelt die Ausgangsbedingungen, D:922-923 sichert den Vergleichswert, D:974 und D:979 trennen Endlichkeit und Biterhalt, die Doppelmutation ergibt „nicht endlich 1“, beide Einzelgegenproben bestätigen die getrennte Schutzwirkung; D3: ja — G:1410-1424 vergleicht alle 221 Bänder beider Stufen, G:1561 und G:1570 prüfen Ausgangszuordnung und neue Zuordnung, G:1550 und G:1554 trennen Ringinhalt und Größe, Zuordnungsmutation sowie Ring-Doppelmutation fallen jeweils an der zugesagten Eigenschaft und die Einzelgegenproben passen; Z1: ja — G:1480-1481 legt den Seek nach den modellierten Snapshot, G:1604-1605 verriegelt Lage und 1–7 neue Frames, m87-seek verletzt G:1631 tatsächlich mit gesetztem Kohärenzbit bei zwölf statt fünf Frames; kein Bruch der mitbetroffenen Matrixzeilen, Etappenzusagen, Gate-Sätze oder CLAUDE-Invarianten erkennbar, Produktdiff leer, lesender Logvergleich bestätigt 773 unveränderte B19-Prüfzeilen und ausschließlich die erwartete M-95-Aufteilung in B18.

Nicht geprüft: keine eigenen Build-, Test-, Mutations-, Kanon- oder Produktläufe gemäß Prüfauftrag; vorhandene Laufprotokolle ausschließlich gelesen, übriger Ticketbereich und ausgeschlossene Nachbarthemen nicht erneut geprüft, keine Dateien verändert.

VORHER: 11ad06ba7bfdc828024000de2b5a80d125a0daba
HEAD zu Beginn: 76c9b665247043ca7b3f81978eca2fb3e357a318
HEAD am Ende: 76c9b665247043ca7b3f81978eca2fb3e357a318 — identisch

FERTIG Wiederprüfung 1 Etappe 5, PASS, 0 offene Befunde, 0 Brüche