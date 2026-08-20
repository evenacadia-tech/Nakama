# Oberflächen-Spezifikation — was jede Ansicht zeigen MUSS

Diese Datei ist die einzige Sachquelle des Projekts. Sie beschreibt
Inhalte und Zustände, keine Gestaltung. Die Gestaltung ist frei und wird
mit dem User entwickelt. Was hier nicht steht, existiert nicht — keine
zusätzlichen Messwerte oder Zustände erfinden.

## Produktidee in einem Satz

Nakama sieht die Summe eines Mixes und seine wichtigsten Instrumenten-
Busse gleichzeitig, findet den wahrscheinlichen Verursacher eines
Problems, beweist ihn hörbar, schlägt eine kleine konkrete Änderung am
richtigen Bus vor und prüft danach, was sie gebracht hat. Es berät —
der User entscheidet und hört immer selbst.

## 1. Main — die Arbeitsfläche (groß)

**Zweck:** tägliche Arbeitsfläche im Master-Plugin. Hier liest der User
den Mix, versteht Befunde und führt den nächsten Schritt.

**Inhalte:**
- **Quellenliste**: bis 16 sichtbare Quellen (Busse) mit Name, Rolle,
  Aktivität und Frische. Suche, Pinning und Fokusgruppen begrenzen die
  Last; ein Minimap-/Zoomprinzip schlägt „alles gleichzeitig".
- **Vergleichende Heatmap**: welche Quelle wo im Frequenzraum Energie
  trägt; Dichtestufen Liste → Heatmap → Detail.
- **Detailansicht** einer Quelle: Spektrum, Lautheit, Dynamik,
  Stereobreite, Verlauf.
- **Befunde**: je Befund Ort (Bus + Frequenzbereich), Beobachtung,
  Begründung, **Sicherheit (hoch / mittel / noch unklar)**, mögliche
  Alternativursachen und der billigste nächste Beweisschritt.
  „Mehr Daten nötig" und „keine Änderung empfohlen" sind normale,
  vollwertige Ergebnisse.
- **Assistent**: führt genau EINEN Schritt zur Zeit in fester Form:
  Priorität → wahrscheinliche Ursache → kleinster Versuch → worauf hören
  → danach dieselbe Passage erneut prüfen.
- **Vorher/Nachher-Vergleich**: Baseline gegen Änderung, level-gematcht;
  Urteil des Users: behalten / verwerfen / noch unklar.
- **Musikalische Prioritäten**: der User kann Quellen Rollen geben
  (führt / trägt / begleitet / geschützt / bewusst verschmolzen).

**Vier Bedienebenen, sichtbar getrennt** (nie vermischen):
Beobachten (nur messen) · Beraten (erklären) · Vorhören (flüchtig hörbar,
nur solange gehalten) · Anwenden (bewusst bestätigt, rückgängig machbar).

**Zustände, die immer ablesbar sein müssen:** Quelle frisch / veraltet /
getrennt; Messung läuft / unvollständig; eine Funktion ist auf diesem
System nicht verfügbar (degradiert) — ehrlich zeigen, nie kaschieren.

## 2. Active-Probe-Editor (mittel)

**Zweck:** vollwertiger lokaler EQ-Editor der aktiven Sonde. Muss allein
funktionieren, auch ohne Main.

**Inhalte:**
- **8 Band-Slots**, je: an/aus, Typ (Bell, Shelf, Notch, Hoch-/Tiefpass),
  Frequenz, Q, Gain (manuell bis ±12 dB), Kanalmodus (Stereo / L-R / M-S),
  dynamisch an/aus mit Schwelle, Bereich, Attack, Hold, Release.
- **Global:** Eingangs-/Ausgangs-Trim, Stereobreite, Mono-Bass, Bypass.
- **EQ-Kurve** als zentrale Anzeige (Summenkurve + Einzelbänder).
- **Draft/Preview vom Main**: ein von außen vorgeschlagener Zustand
  erscheint als eigener, klar getrennter flüchtiger Zustand — gehalten =
  hörbar, losgelassen = weg. Anwenden und Verwerfen sind bewusste Klicks.
  Ferngesteuerte Vorschläge sind enger begrenzt (±3 dB) als manuelle.
- **Undo-Verlauf** der bestätigten Zustände + „Neutralisieren".
- **Schutzbereiche des Users** (gesperrte Frequenzbereiche/Eigenschaften)
  sichtbar im Kurvenfeld.
- **Statuszeile:** verbunden/getrennt, gekoppelt (Pairing) ja/nein.

## 3. Passive-Probe-Kachel (klein)

**Zweck:** reine Status-/Identitätskachel. Die passive Sonde verändert
nie Klang und hat keine Bedienfläche außer:
- Name und Rolle des Busses (bestätigen/ändern),
- Messposition (Insert / PRE / POST einer Kette),
- Verbindung + Frische der Messung,
- Hinweis bei Problemen (doppelte Instanz, kein Kontakt).

## Für alle drei Blätter

- **Editor-Panel** (für den User, im Prototyp): Look-Parameter live
  verstellbar, Zustands-Schalter für alle oben genannten Zustände,
  Varianten nebeneinander.
- **Demo-Daten**: realistisch (z. B. Klavier/Chor/Drums/Bass als
  Quellen), sichtbar als Demo markiert.
- **Maßstab**: ein Profi mit 20 Spuren muss in 2 Sekunden ablesen können,
  was Sache ist. Information vor Dekoration.
