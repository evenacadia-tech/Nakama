# 2026-09-01 — Gen erhält vorerst genau eine Standardgröße

## User-Wortlaut

Der User vereinfacht den zuvor am selben Tag festgehaltenen Zwei-Größen- und
Resize-Ansatz ausdrücklich:

> „compact und standard wird zu kompliziert. es ist schon schwer genug ein
> gutes UI layout zu erstellen. es wird eine größe nämlich standard geben.
> compact kann eventuell iwann wenn alles fertig ist angegangen werden.“

Dieser Entscheid ist neuer und ersetzt für den aktuellen Funktionsentwurf die
Größen- und Resize-Regeln aus
[`2026-09-01-gen-fenstergroesse-und-resize.md`](2026-09-01-gen-fenstergroesse-und-resize.md).

## Bindender aktueller Größenvertrag

- Beide Gen-Flächen verwenden genau **eine logische Layoutgröße: 950×538**.
- Es gibt im aktuellen Ziel keine Compact-Fassung, keine zweite
  Layoutvariante und kein größenabhängiges Reflow.
- Fenster-Resize als zusätzliche Informationsfläche wird nicht parallel zum
  Standardlayout entworfen. Es ist zusammen mit Compact vertagt.
- Compact darf frühestens neu betrachtet werden, wenn die Standardoberfläche
  vollständig ausgearbeitet und abgenommen ist. Das ist eine mögliche spätere
  Erweiterung, keine heutige Zusage.
- Die UI-Skalierungsstufen 100/125/150/200 Prozent bleiben davon getrennt. Sie
  vergrößern dieselbe logische 950×538-Geometrie für Lesbarkeit und Bedienung;
  sie erzeugen kein zweites Layout und dürfen keine Funktionen verändern.
- Die Entscheidung betrifft beide Gen-Flächen. Sie dimensioniert die
  Probeeq-Rückfallfläche nicht neu.

## Konsequenz für Skizze und Implementierung

Die technische Skizze enthält keinen Größenumschalter mehr und kennzeichnet
die eine Gen-Bühne als `ONE LOGICAL SIZE 950×538`; der bisherige
Compact-Umschalter entfällt. Der äußere Inspektionsrahmen darf die komplette
950×538-Bühne proportional verkleinern, damit sie in der schmalen internen
Browserspalte vollständig sichtbar bleibt. Dieses Fit-to-view ist Werkzeug,
kein Produkt-Resize und kein alternatives Layout.

Der aktuelle funktionale JUCE-Pfad läuft noch fest auf 760×430. Die spätere
native Übernahme muss deshalb auf genau 950×538 umgestellt und dort in FL
Studio geprüft werden. Eine Resize-Implementierung gehört ausdrücklich nicht
zu diesem Schritt.

## Abnahmebeweis

Am 01.09.2026 um 18:21 Uhr im internen Browser auf der laufenden technischen
Skizze geprüft:

- Gen Overview und Gen EQ melden jeweils eine logische Bühne von exakt
  **950×538**; der Wechsel erzeugt keine zweite Layoutgröße.
- In Overview bleiben Sources-Rail 180 Pixel breit, akustische Evidenz
  738×312 und Finding-Fläche 738×142 groß.
- Der EQ-Graph misst 928×370. Das frei verankerte Band-Panel misst im normalen
  Bandzustand 276×80 und bleibt sowohl bei B1 links als auch bei B6 rechts
  vollständig innerhalb des Graphen; der Dynamic-Zustand B3 misst 276×112.
- Es existiert kein `data-gen-size`-Control mehr. Der schmale interne Browser
  zeigt die unveränderte 950×538-Bühne lediglich mit Fit-to-view-Faktor
  0,6674.
- Die Browser-Konsole blieb leer. Der maschinenlesbare UI-Vertrag bestand
  `node design/prototyp/tests/contract.test.mjs`; der Nakama-Adaptercheck des
  Audio-Plugin-UX-Skills bestand mit `failedCount: 0` und Vertragsversion
  `0.3.0`.

Noch nicht bewiesen ist die spätere native 950×538-Umsetzung in JUCE und FL
Studio; der aktuelle funktionale Pfad steht weiterhin fest auf 760×430.
