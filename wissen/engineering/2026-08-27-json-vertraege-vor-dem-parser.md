---
id: w-2026-08-27-json-vertraege-vor-dem-parser
typ: wissenseintrag
status: aktiv
datum: 2026-08-27
aktualisiert: 2026-08-27
hauptbereich: engineering
weitere_bereiche: [fl-studio]
pruefstand: mehrfach-belegt
---

# JSON-Verträge vor dem Parser beweisen

## Menschliches Warum

Projektzustand, Messdaten und Steuerbefehle dürfen ihre Bedeutung nicht ändern,
nur weil sie gerade von C++, Rust oder Python gelesen werden. Für einen
Produzenten wäre das kein technischer Sonderfall: Ein Projekt lädt anders,
eine Messung wird falsch bestätigt oder ein authentisierter Befehl meint auf
der Gegenseite etwas anderes.

Die Kausalkette lautet:

> Weil ein Produzent beim Laden, Vergleichen und Anwenden auf dieselbe Bedeutung
> seiner Daten angewiesen ist, muss Nakama bereits die zulässige Byte- und
> Zahlenform eindeutig festlegen, damit unterschiedliche Parser keinen
> scheinbar gültigen, aber abweichenden Zustand erzeugen.

## Belegtes Wissen

RFC 8259 beschreibt JSON absichtlich breiter, als es ein sicherer
Mehrsprachenvertrag verträgt. Objektnamen sollen eindeutig sein; bei Duplikaten
ist das beobachtete Verhalten verschiedener Implementierungen unvorhersehbar.
Zahlen dürfen Bruch- und Exponentenschreibweise verwenden. Implementierungen
dürfen außerdem Grenzen für Größe, Tiefe und Zahlenbereich setzen. Exakte
Ganzzahl-Interoperabilität wird nur für den Bereich von `-(2^53)+1` bis
`(2^53)-1` zugesichert.

RFC 8785 schränkt kanonisierbare JSON-Daten deshalb auf I-JSON ein: doppelte
Namen sind verboten, ungültige Unicode-Surrogate müssen scheitern und Zahlen
folgen IEEE-754-Binär64. Zeichenketten werden nicht nachträglich Unicode-
normalisiert. Höherpräzise oder sehr große Zahlen sollen als Zeichenketten
transportiert werden. `NaN` und Unendlich sind keine zulässigen JSON-Zahlen.

Der entscheidende Schluss ist: Ein JSON-Schema allein beweist noch keinen
identischen Vertrag. Wenn ein Parser zuerst `1.0000000000000001` nach Binär64
rundet und erst danach auf "integer" prüft, kann er ein anderes Ergebnis
liefern als ein Parser, der die lexikalische Zahl betrachtet. Dieselbe Gefahr
gilt für doppelte Schlüssel, `null` gegenüber fehlend und still akzeptierte
Zusatzfelder.

## Fehlerlexikon und Gegenbeweis

| Fehlerquelle | Warum ein normaler Test grün bleiben kann | Erforderlicher Gegenbeweis |
|---|---|---|
| doppelter Schlüssel | jeder Parser wählt scheinbar erfolgreich einen Wert | identische Ablehnung in allen Sprachbeinen |
| lexikalisch gebrochene "Ganzzahl" | Binär64 rundet vor der Typprüfung | gemeinsame Fixtures mit Bruch, Exponent und Grenzmantisse |
| `null`, fehlend und Zusatzfeld | DTO-Defaults verdecken den Unterschied | explizite Negativfixtures und `deny unknown` |
| NUL, ungültiges UTF-8, einzelner Surrogate | C-String- und Unicode-APIs schneiden oder ersetzen | bytegenaue Eingabe und identische Ablehnung |
| `-0`, `NaN`, `Inf`, Überlauf | Sprachbibliotheken normalisieren unterschiedlich | nicht-finite und Grenzwert-Fixtures vor Geschäftslogik |
| Tiefe oder Größe ohne Obergrenze | kleine Happy-Path-Dateien zeigen keinen Ressourcenangriff | feste Limits mit Exakt-am-Limit- und Eins-darüber-Test |
| JCS vor oder nach Validierung | beide Seiten signieren unterschiedliche Bedeutung | ein kanonischer Ablauf und sprachübergreifende Signaturgoldens |
| neue Schema-Version bei altem Verbraucher | unbekannte Felder werden still ignoriert | Versionsmatrix Producer × Consumer mit Fail-closed-Regel |

## Bedeutung für Nakama

Die Debugging-Session fand genau diese Klasse mehrfach: C++ schnitt
längenbegrenztes UTF-8 am rohen NUL anders ab als Rust und Python; State-Bytes
konnten mit gültigem Präfix und Restdaten akzeptiert werden; v2-Nachrichten
nahmen unbekannte Felder und teilweise ungültige Messungen an. Besonders
wertvoll war der spätere Gegenbeweis mit lexikalisch gebrochenen JSON-Zahlen:
Ein Schema-"integer" war nach Parserrundung nicht in allen drei Sprachen
dasselbe. Die Reparatur trennt deshalb lexikalische Schema-Prüfung von der
exakten DTO-/JCS-Auswertung und hält die Mehrsprachenfixtures gemeinsam.

Für die geplanten v3-Control- und Telemetry-Verträge sowie HMAC/JCS bedeutet
das: Bytes, Unicode, Schlüsselmenge, Zahlendomäne, Größenlimit,
Kanonisierungszeitpunkt und Versionsfehler gehören in einen gemeinsamen
Vertrag. Ein erfolgreicher Parse ist kein Akzeptanzbeweis.

## Grenzen, Widersprüche und offene Fragen

RFC 8259 erlaubt bewusst mehr als Nakama sicher verwenden sollte. Die engeren
Regeln hier sind daher eine Nakama-Hypothese für robuste Verträge, keine
Behauptung über jedes JSON-System. Die konkrete v3-Feldform bleibt Eigentum des
technischen Bauplans und der Schemas. Dieser Eintrag ersetzt weder Code noch
Fixtures.

## Quellen

- Titel: *The JavaScript Object Notation (JSON) Data Interchange Format*
- Autor: Tim Bray
- Jahr: 2017
- URL: https://www.rfc-editor.org/rfc/rfc8259.html
- relevante Abschnitte: 4, 6, 8 und 9
- Titel: *JSON Canonicalization Scheme (JCS)*
- Autoren: Anders Rundgren, Benjamin Jordan und Samuel Erdtman
- Jahr: 2020
- DOI: https://doi.org/10.17487/RFC8785
- URL: https://www.rfc-editor.org/rfc/rfc8785.html
- relevante Abschnitte: 3.1, 3.2.2.2 und 3.2.2.3
- Eingang: `web-recherche`
- geprüft: Original-RFCs sowie Nakamas aktuelle Verträge, Fixtures und
  Debugging-Beweise

## Verknüpfungen

- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/integration.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/integration.md)
- [`../../docs/FL-Nakama-Sonden-Design-Entwurf.md`](../../docs/FL-Nakama-Sonden-Design-Entwurf.md)
- [`../../eq-copilot/schemas/v3/README.md`](../../eq-copilot/schemas/v3/README.md)
- [`2026-08-27-windows-pipes-sicherheit-und-lifecycle.md`](2026-08-27-windows-pipes-sicherheit-und-lifecycle.md)
