---
id: w-2026-08-27-beweise-muessen-reproduzierbar-sein
typ: wissenseintrag
status: aktiv
datum: 2026-08-27
aktualisiert: 2026-08-27
hauptbereich: engineering
weitere_bereiche: [dsp, ui-motion]
pruefstand: mehrfach-belegt
---

# Beweise müssen aus versionierten Quellen neu entstehen

## Menschliches Warum

Eine Zahl oder ein grüner Test hilft nur, wenn ein anderer Lauf erklären und
erneut zeigen kann, wie sie entstanden ist. Sonst trifft der Produzent eine
Design-, Klang- oder Releaseentscheidung auf Basis eines einmal sichtbaren
Zustands, der weder überprüft noch nach einer Änderung widerlegt werden kann.

Die Kausalkette lautet:

> Weil ein Produzent und das Team Entscheidungen später noch überprüfen
> müssen, braucht jeder relevante Beleg versionierte Eingaben, Ablauf und
> Werkzeugstand, damit eine Behauptung neu erzeugt und nicht nur weitererzählt
> werden kann.

## Belegtes Wissen

Das Reproducible-Builds-Projekt nennt einen Build reproduzierbar, wenn derselbe
Quellcode, dieselbe relevante Umgebung und dieselben Bauanweisungen
bitidentische Artefakte erzeugen können. Der Vergleich erfolgt bytegenau,
üblicherweise über kryptografische Hashes. Damit gehören Abhängigkeiten,
Versionen, Flags, Locale und weitere wirksame Umgebungswerte zum Beweis und
nicht zum unsichtbaren Hintergrund.

SLSA beschreibt Provenienz als verifizierbare Information darüber, wo, wann
und wie ein Artefakt erzeugt wurde. Die aktuelle Spezifikation verlangt einen
konsistenten Buildprozess und eine Provenienz, die das Ausgabeobjekt über
seinen Digest eindeutig identifiziert und seine Entstehung beschreibt.

Für Messungen ist nicht immer ein identisches Ergebnis zu erwarten. Die
übertragbare Regel lautet dann: Eingabe, Messaufbau, Version, Zielumgebung,
Stichprobe und Auswertung müssen erneut ausführbar sein; erlaubte Streuung und
Urteilsschwelle müssen vorher feststehen. Ein Screenshot oder Bericht ist ein
Ausgabeobjekt, nicht der Messaufbau.

## Fehlerlexikon und Gegenbeweis

| Fehlerquelle | Warum die Behauptung nicht belastbar ist | Erforderlicher Gegenbeweis |
|---|---|---|
| sitzungslokales Blatt oder Skript | der vermessene Gegenstand ist verschwunden | Messbank und Eingaben im Repo, Neubau aus sauberem Checkout |
| Bericht ohne Runner | Zahlen können nicht neu entstehen | ausführbarer Befehl mit versionierter Konfiguration |
| "neueste" Toolversion | gleicher Quellstand erzeugt anderes Ergebnis | konkrete Version und Digest des Werkzeugs |
| generierte Quelle manuell formatiert | Generator und eingecheckte Bytes driften | neu generieren und bytegenau gegen Git prüfen |
| alter grüner Beleg nach Quelländerung | Ergebnis gehört zu anderem Stand | Quellcommit/Digests im Manifest und Frischeprüfung |
| Runner verändert seine Eingabe | der Test beweist den Zustand nach Selbstreparatur | Mutationsprüfung und sauberer Worktree vor/nach Lauf |
| Golden und Istwert gemeinsam aktualisiert | falsches Verhalten wird zur neuen Erwartung | unabhängige Vertragsquelle und adversariales Review |
| Timingzahl ohne Umweltprofil | Hardware, Host und Last sind unbekannt | Plattform-, Rate-, Block-, Instanz- und Lastmatrix |
| nur Pass-Zähler | offen, gebaut und abgenommen werden vermischt | Urteil, Prüfstufe und nicht gelaufene Grenzen getrennt |
| Log ohne Rohdaten oder Auswertungsregel | Schwelle wurde nach Sichtung gewählt | vorab definierte Regel und ausreichend kleine, prüfbare Rohbelege |

## Bedeutung für Nakama

Der historische UI-Sondenplan liefert den klarsten Gegenbeweis: Die erste
Messung lebte nur in einer Session. Nach dem Bau einer versionierten Messbank
waren 10 von 19 nachprüfbaren Angaben falsch. Die falsche Grundannahme von
20 statt 15,4 Pixel Zeilenhöhe zog mehrere darauf aufgebaute Layoutwerte mit.
Der heutige Vermerk "historisch" verhindert zusätzlich, dass diese alte
Oberflächenrichtung versehentlich wieder zum Arbeitsauftrag wird.

Die letzte technische Debugging-Session zeigte dieselbe Regel in anderer Form.
Der erste Abschlusslauf war ehrlich 28/29 rot, weil die eingecheckte generierte
Rust-FlatBuffers-Datei bei den Zeilenenden vom Generator abwich. Erst die
Neugenerierung aus demselben `.fbs` mit dem gepinnten `flatc 25.12.19`, ein
bytegenauer Drift-0-Vergleich und der erneute Gesamtlauf ergaben 29/29.

Für alle weiteren Sonden folgt daraus: Messbank, Fixture, Schema, Generator,
Werkzeugversion, Hostmatrix und Urteil gehören zum versionierten Beweisweg.
Pläne und Berichte dürfen den aktuellen Lauf erklären, aber nie dessen
Ausführung ersetzen. Ein generiertes Artefakt ist nur dann Wahrheit, wenn es
aus der kanonischen Quelle reproduzierbar entsteht und der Drift-Riegel grün
ist.

## Grenzen, Widersprüche und offene Fragen

SLSA ist eine Lieferketten-Spezifikation; die Übertragung auf Nakamas
Design- und DSP-Messungen ist eine begründete Analogie. Bitidentität ist für
generierten Code und Fixtures passend, nicht für verrauschte Laufzeit- oder
Audiomessungen. Dort ist ein reproduzierbares Protokoll mit vorab festgelegter
Toleranz der ehrlichere Gegenbeweis.

## Quellen

- Titel: *Definitions – When is a build reproducible?*
- Autor: Reproducible Builds Project
- URL: https://reproducible-builds.org/docs/definition/
- relevante Abschnitte: Definition, Umgebung und bytegenauer Vergleich
- Titel: *SLSA v1.2 – Provenance*
- Autor: SLSA Community / Linux Foundation
- Jahr: 2026
- URL: https://slsa.dev/spec/v1.2/provenance
- Titel: *SLSA v1.2 – Requirements for producing artifacts*
- Autor: SLSA Community / Linux Foundation
- Jahr: 2026
- URL: https://slsa.dev/spec/v1.2/build-requirements
- Eingang: `web-recherche`
- geprüft: aktuelle Spezifikationen sowie Nakamas versionierte Messbank,
  historischer Sondenplan und finaler Debugging-Beweis

## Verknüpfungen

- [`../../design/docs/sondenplan.md`](../../design/docs/sondenplan.md)
- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/verification.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/verification.md)
- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/integration.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/integration.md)
- [`../../docs/PLAN-STAND.md`](../../docs/PLAN-STAND.md)
- [`2026-08-27-realtime-worst-case-statt-durchschnitt.md`](2026-08-27-realtime-worst-case-statt-durchschnitt.md)
