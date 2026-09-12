# Integration des zweiten Audits

Abschlussnotiz vom 12.09.2026. Der eigenständig lesbare Befundbericht ist UEBERGABE-AN-CLAUDE.md in diesem Ordner. Diese Notiz dokumentiert die Zusammenführung, keine zusätzliche Fehlerliste.

## Annahmen und Widersprüche

- Der erste Audit und NAK-246 sind Ausgangsmaterial. D1–D10 wurden gegen die aktuellen Aufrufpfade und Tests gelesen. Historische 64/64-Ergebnisse wurden nicht als aktueller eigener Nachweis übernommen.
- F01 wurde vom vermuteten Publikationsrennen auf einen tatsächlich verwendeten Ablauf eingegrenzt: B ist bestätigt, Join A läuft über den echten Worker, Unbind B über den Message-Thread. Der identische serielle Ablauf funktioniert. Der alte Reload-Generationsfix greift hier nicht, weil kein Reload stattfindet.
- F02 verwendet den Original-Broker mit vorhandenem Rechenhaken. Der verspätete Stand ist nicht nur im Cache sichtbar: aufgezeichneter Snapshot-Payload und SQLite-Projektion stimmen mit dem alten Ergebnis überein. Bestehender Registerpunkt NAK-253, nicht erneut als neu zählen.
- F03 bindet die Original-Queue ein. Labels statt vollständiger Wire-Nachrichten beschränken den Laufbeleg auf die Ausgabeordnung. Die Wirkung einer angenommenen Invalidierung wurde separat im C++-Consumer gelesen. Die acht Original-Kontrollen belegen, dass schlüsselbezogene D5-/D9-Fixes daneben funktionieren. NAK-269 behauptet derzeit zu weitgehend vollständige Brokerordnung.
- F04/F05 wurden von direkten Prozessormessungen bis zum echten VST3-Modul verfolgt. Der lokale Testhost aktiviert einen gültigen symmetrischen Vierkanalbus; die Wrapper-Aufrufe melden Erfolg. Das ist stärker als bloße Headerinspektion, aber kein FL-Studio-Beleg.
- F06 ist ein Einheitenfehler an einem Gate mit messbarer Sampleratenabhängigkeit, kein Anspruch auf eine neue Pegeldefinition. PSD-Verbraucher müssen bei der Korrektur getrennt beurteilt werden.
- F07 betrifft den tatsächlichen Start eines vorab gebundenen Fensters. Der vorhandene Reset beim Binden und die bereits behandelte Endgrenze widerlegen den Fall nicht. Die vollständige UI-/Hostkette wurde für diesen Fall nicht ausgeführt.
- F08 ist nicht durch stilles Umbenennen in „Evidenzfenster“ erledigt: der aktuelle Ring entspricht weder der vollen Passage noch einem jeweils neu begonnenen Evidenzfenster. M-03 und der Kommentar in Vertrag.h wurden nebeneinander gelesen.
- F09/F12: Erste Audioharnessfassungen mit unvollständigen Parametern beziehungsweise einem Hilfs-Stub wurden verworfen. Archiviert und ausgewertet wurden ausschließlich die abschließenden Fassungen mit erfolgreicher öffentlicher Parameterprüfung und originalen Produktbibliotheken.
- F09: Der kleine Tap-Verzweigungsteil wurde nachgebildet; diese Grenze ist im Bericht sichtbar. Kein vollständiger Sonde-Prozessorlauf für diesen speziellen Fall behauptet.
- F10: Die Ungültigkeit des aktuellen Rahmens und des Evidenzintervalls reichen nicht bis zum Ende der betroffenen 3-s-Historie. Genau diese unterschiedlichen Lebensdauern wurden getrennt gelesen; kein nicht endlicher Wirewert behauptet.
- F11/F12 sind belegte akzeptierte Zahlenränder, aber keine normalen musikalischen Betriebsfälle. Daher P3 statt künstlicher Dringlichkeit. Ihre vorliegenden Tests werden nach dem Stoppsignal nicht erneut gestartet.
- F13: Der historische A24-Bericht ist nur Kontrollinput. Der heutige Orakelpfad akzeptiert den Fehlerwert null; das ist kein Nachweis, dass der historische Soak seine RSS-Abfragen tatsächlich verlor.
- F14 ist die bereits registrierte NAK-163-Kombination. Der Store-/Cache-Unterschied ist jetzt gemessen, die alte Klassifizierung damit neu zu beurteilen. Keine Dublette.
- N01 wurde nach dem Agentenhinweis ausschließlich lesend gegen Originaltest, Korpuserzeuger und EBU Tech 3341 §2.6/Tabelle 1 überprüft. Fehlende Fälle 20–23 bedeuten eine Abdeckungslücke, nicht automatisch einen fehlerhaften True-Peak-Kern.

## Nicht in bestätigte Befunde übernommen

State-Collection-Headroom (NAK-252), spätere Evidenzrücknahme über Reload, Passagen-Worker-Bindung, Einheit der Stereo-Mid/Side-Zahl, Fingerprint ohne Onsets, fachliche K-Filterhistorie sowie unterschiedliche Installer-/Journalstände. Gründe und verbleibende Entscheidungsfragen stehen in Abschnitt 5 der Übergabe.

## Grenzen der Unabhängigkeit

Sechs unterschiedlich abgegrenzte Agentenpakete plus Root. Root hat vorhandene Harnessquellen, Rohwerte, relevante Produktstellen und Registerbezüge zusammengeführt. Nicht jeder Befund wurde von einem zweiten Agenten vollständig reproduziert. Mehrere Agentenläufe wurden automatisch blockiert. Nach der letzten User-Anweisung erfolgten keine neuen Experimente; offene Fragen wurden eingegrenzt statt durch ein erfundenes PASS geschlossen.

## Schreibgrenze

Produktquellen unverändert. Nur Planrechnung und neue Auditdokumentation einschließlich archivierter Belege. Kein offenes Register nebenbei umgeschrieben, keine neuen NAK-IDs reserviert, keine Produktinstallation und keine externe Nachricht an Claude gesendet.
