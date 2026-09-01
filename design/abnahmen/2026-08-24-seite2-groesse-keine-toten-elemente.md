# Entscheid 24.08.2026 — Seite-2-Größe und das Verbot toter Elemente

**Anlass:** Rückmeldung des Users auf den ersten Spielregeln-Umbau der
Gen-Seiten im Prototyp-Figma (23.08., Commit `8bea367`). Zweiter Durchgang
läuft in der Kopie-Datei `Probeeq-Nakama-Prototyp-Design (Copy)`
(Key `f9a5zdQ0tQ3e4dzrsF2k5y`).

## 1. Beide Seiten gleich groß (beantwortet die Größenfrage aus NAK-65)

> „da ist einiges falsch. page 2 des Gen , also die EQ fläche hat kleinere
> maße als die 1. seite. da es sich um 2 flächen der selben app handelt
> müssen die selbstverständlich gleich groß sein."

Folge: Gen Seite 2 (EQ-Zentrale) nutzt dieselbe Fläche wie Seite 1 —
**760×430** (Abnahme 20.08. galt der Übersicht, jetzt für beide Seiten).
Umgesetzt im 2. Durchgang: Seite 2 ist auf der Seite-1-Schale neu aufgebaut
(gleiche Chrome-Zeile, Wortmarke, Material).

## 2. Keine toten Elemente

> „zudem solltest du mal überlegen was fassade und was gebraucht wird. was
> ist der unterschied zu connected und paired ? was soll der global button
> bewirken ? WAS SOLL DER PLAY symbol button bewirken ? live ? overview ?
> die schlimmste ui/ux sünde sind sinnlose tote elemente . mach einen
> 2. durchgang über das gebaute eben und fokus auf logik."

Regel daraus (bindend für jede weitere Fläche): **Jedes sichtbare Element
bedient einen Handgriff oder meldet ehrlich einen Zustand.** Material/Schale
(Glas, Grain, Lichtsaum) ist Design-Identität des Users und zählt nicht als
totes Element; Beschriftungen zählen nur, wenn sie Information tragen
(Skalen, Analyzer-Abgriff).

Antworten des 2. Durchgangs auf die fünf genannten Sünder:

| Element (Durchgang 1) | Befund | Durchgang 2 |
|---|---|---|
| CONNECTED · PAIRED | zwei Wörter, ein Zustand — Fassade | EIN Link-Punkt; gesund = still (Ausnahme-Prinzip 20.08.); im Fehlerfall benennt der Text den gefallenen Abschnitt: BROKER OFFLINE ≠ PROBE OFFLINE |
| GLOBAL (Text + freier Pfeil) | Pfeil las sich als Play-Button, Text ohne erkennbare Funktion | EIN Aufklapp-Button „GLOBAL ⌄" (Chevron IM Button); klappt die vier globalen Regler aus (Regel 43) |
| „PLAY-Symbol" | war der frei schwebende Einklapp-Pfeil — Fehler | entfernt; Chevron sitzt im GLOBAL-Button |
| LIVE (Dauer-Chip) | Dauer-Status ist Rauschen | entfernt; nur Ausnahmen sprechen (STALE etc. erscheinen im Ausnahme-Slot) |
| OVERVIEW (toter Einzeltext) | Funktion (Seitenwechsel) nicht ablesbar, inkonsistent | Seiten-Tab-Paar **OVERVIEW · EQ CENTER** an identischer Position auf BEIDEN Seiten, aktive Seite hell |

**Status:** Der 2. Durchgang ist Claude-Umsetzung dieser Entscheide;
die Optik der Frames bleibt Vorschlag bis zum User-Urteil.

## Nachträgliche Quellenkorrektur 24.08.2026

Die Copy-Datei oben bleibt Teil der Entstehungsgeschichte, ist aber keine
aktuelle Designquelle. Der User hat anschließend bekräftigt, dass immer nur
`Nakama-Design` (`NPCQYSkoZEd4Av0NlKxBOd`) den aktuellen Stand enthält. Beim
ersten Live-Check stand das EQ Center dort noch auf 700×420 (`25:584`).

Danach korrigierte der User diesen Fehler direkt im verbindlichen Figma:

> „habe eq center aktualisiert, war ein fehler. ist jetzt das aktuelle drin
> mit 430“

Der erneute Live-Check bestätigt Wrapper `28:993` und Artboard `28:994`
mit 760×430. Die Größenentscheidung ist damit umgesetzt. Im sichtbaren
Hauptframe sind außerdem CONNECTED · PAIRED, der LIVE-Dauerchip und der freie
Pfeil nicht mehr vorhanden; GLOBAL ist ein beschrifteter Button mit Chevron
und OVERVIEW/EQ CENTER sind als Tab-Paar dargestellt. Die vollständige
Optik-, Komponenten- und Motion-Abnahme bleibt offen. Vollständiger Beleg:
`2026-08-24-figma-depot-immer-aktuell.md`.

## Fortschreibung vom 01.09.2026

Die Gleichheit beider Gen-Flächen bleibt bindend. Die damalige Festlegung auf
760×430 als einzige Größe ist jedoch durch
[`2026-09-01-gen-nur-standardgroesse.md`](2026-09-01-gen-nur-standardgroesse.md)
ersetzt: Beide Flächen verwenden im aktuellen Funktionsentwurf genau eine
logische Größe von 950×538. Compact und größenabhängiges Layout sind vertagt.
Die hier gemessenen Figma-Artboards bleiben historische Belege des damaligen
Stands und bestimmen nicht den aktuellen Funktionsentwurf.
