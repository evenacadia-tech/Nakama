# Oberflächen-Spezifikation — was jede Ansicht zeigen MUSS

Diese Datei ist die einzige Sachquelle des Projekts. Sie beschreibt
Inhalte und Zustände, keine Gestaltung. Die Gestaltung kommt aus Figma
(User, Designvertrag 2026-08-21: „Figma ist Quelle; Repo setzt um");
sie wird hier nicht beschrieben. Was hier nicht steht, existiert nicht —
keine zusätzlichen Messwerte oder Zustände erfinden.

**Namen (User, 2026-08-21):** „Nakama Gen = Main app · aktive sonde =
Nakama Probeeq · passive sonde = Nakama Suna · Bundle = Nakama Studio".
Die Abschnitte unten tragen beide Namen; neue Texte verwenden die neuen.

**Sprache (User, 2026-08-21: „Englisch – mein Wort"):** alles, was im
Produkt steht — Beschriftungen, Werte, Einheiten, Zustände, Meldungen —
ist englisch. Diese Datei beschreibt auf Deutsch, was dort englisch steht.

**Zahlen** (16 sichtbare Quellen, 8 Bänder, ±12 dB, ferngesteuert ±3 dB)
sind vom User **hingenommene Startwerte** („Hingenommen, passen aber",
2026-08-21), änderbar. Der Regelfall ist eine einstellige Quellenzahl; 16
ist die Belastungsprobe (`abnahmen/2026-08-20-karte-alle-quellen.md`,
Nachtrag 2026-08-21).

## Produktidee in einem Satz

Nakama sieht die Summe eines Mixes und seine wichtigsten Instrumenten-
Busse gleichzeitig, findet den wahrscheinlichen Verursacher eines
Problems, beweist ihn hörbar, schlägt eine kleine konkrete Änderung am
richtigen Bus vor und prüft danach, was sie gebracht hat. Gen und Suna
beraten — der User entscheidet und hört immer selbst; Probeeq ist der
EQ, der die Änderung ausführt, auf Anweisung von Gen oder von Hand.

## 1. Nakama Gen (Main) — die Arbeitsfläche (groß)

**Zweck:** tägliche Arbeitsfläche der Main-App. Hier liest der User
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

## 2. Nakama Probeeq (Active-Probe-Editor, mittel)

**Zweck:** die aktive Sonde ist **ein vollwertiger, hochwertiger EQ, der
mit Nakama kommuniziert** — er setzt Anweisungen von Gen direkt um **und**
wird ganz normal manuell vom User benutzt (User-Wort, 2026-08-21). Beides
ist gleichwertig. Muss allein funktionieren, auch ohne Gen.

**Inhalte:**
- **8 Band-Slots**, je: an/aus, Typ (Bell, Shelf, Notch, Hoch-/Tiefpass),
  Frequenz, Q, Gain (manuell bis ±12 dB), Kanalmodus (Stereo / L-R / M-S),
  dynamisch an/aus mit Schwelle, Bereich, Attack, Hold, Release —
  **zwölf Parameter**, so zeigt sie auch der Figma-Stand
  (`assets/figma/2026-08-21-probeeq.png`: `BAND TYPE FREQ GAIN Q MODE |
  DYN THRESH RANGE ATTACK HOLD RELEASE`).
  > **Offen (NAK-33 im Technik-Repo), hier nicht entschieden:** der
  > technische Entwurf zählt je Band **dreizehn** Parameter — zusätzlich
  > eine **Sidechain-Quelle** für die Dynamik. Ob sie im Produkt sichtbar
  > wird und wo, ist offen. Bis zur Entscheidung bleibt das Blatt bei
  > zwölf und trägt den Vermerk; kein Ort wird dafür erfunden.
- **Global:** Eingangs-/Ausgangs-Trim, Stereobreite, Mono-Bass, Bypass.
- **EQ-Kurve** als zentrale Anzeige (Summenkurve + Einzelbänder).
- **Draft/Preview von Gen**: ein von außen vorgeschlagener Zustand
  erscheint als eigener, klar getrennter flüchtiger Zustand — gehalten =
  hörbar, losgelassen = weg. Anwenden und Verwerfen sind bewusste Klicks.
  Ferngesteuerte Vorschläge sind enger begrenzt (±3 dB) als manuelle.
- **Undo-Verlauf** der bestätigten Zustände + „Neutralisieren".
- **Schutzbereiche des Users** (gesperrte Frequenzbereiche/Eigenschaften)
  sichtbar im Kurvenfeld.
- **Statuszeile:** verbunden/getrennt, gekoppelt (Pairing) ja/nein.

## 3. Nakama Suna (Passive-Probe-Kachel, klein)

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
