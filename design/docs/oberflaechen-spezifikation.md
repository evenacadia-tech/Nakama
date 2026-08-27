# Oberflächen-Spezifikation — was jede Ansicht zeigen MUSS

**Ergänzung für Bedienung und Bewegung:** Die
[Interaktions- und Motion-Spezifikation](interaktions-und-motion-spezifikation.md)
konkretisiert Gesten und Zustandswechsel, ohne die aktuelle User-Referenz
umzudeuten.

Diese Datei ist die zentrale Sachquelle des Projekts für Inhalte und
Zustände. Die verlinkte Interaktions- und Motion-Spezifikation ergänzt
nur die Bediengesten und Übergänge dieser festgelegten Elemente. Sie
erfindet keine zusätzlichen Messwerte oder Produktzustände. Die Gestaltung
kommt aus den drei aktuellen Rework-Bildern und neueren User-Entscheiden; sie
wird hier nicht beschrieben. Was hier nicht steht, existiert nicht — keine
zusätzlichen Messwerte oder Zustände erfinden.

**Aktuelle Produktstruktur (User, 2026-08-27):** Gen hat zwei Oberflächen;
„Overview" und „EQ-Zentrale" beziehungsweise „EQ Center" sind nur
Arbeitsnamen. Probeeq vereint die frühere passive Suna-Messrolle und die
aktive Probeeq-Rolle: passiv messen und Gen beliefern, bei zugeschaltetem EQ
Gens Fernsteuerung auf dem jeweiligen Bus ausführen. Wortlaut:
`../abnahmen/2026-08-27-arbeitsnamen-und-probeeq-doppelrolle.md`.

**Sprache (User, 2026-08-21: „Englisch – mein Wort"):** alles, was im
Produkt steht — Beschriftungen, Werte, Einheiten, Zustände, Meldungen —
ist englisch. Diese Datei beschreibt auf Deutsch, was dort englisch steht.

**Zahlen** (16 sichtbare Quellen, 8 Bänder, ±12 dB) sind vom User
**hingenommene Startwerte** („Hingenommen, passen aber", 2026-08-21),
änderbar. Das frühere ±3-dB-Sonderlimit für Fernsteuerung ist seit dem
Entscheid vom 23.08. aufgehoben. Der Regelfall ist eine einstellige
Quellenzahl; 16 ist die Belastungsprobe
(`abnahmen/2026-08-20-karte-alle-quellen.md`, Nachtrag 2026-08-21).

## Produktidee in einem Satz

Nakama sieht die Summe eines Mixes und seine wichtigsten Instrumenten-Busse
gleichzeitig, findet den wahrscheinlichen Verursacher eines Problems, beweist
ihn hörbar, schlägt eine kleine konkrete Änderung am richtigen Bus vor und
prüft danach, was sie gebracht hat. Probeeq misst dort im Grundzustand passiv
und beliefert Gen. Schaltet der User den EQ zu, führt dieselbe Instanz Gens
ferngesteuerte Änderung auf diesem Bus aus; Gen besitzt zusätzlich den
Master-EQ. Der User entscheidet und hört immer selbst.

## 1. Nakama Gen — zwei große Oberflächen

**Zweck:** tägliche Arbeits-App. Hier liest der User den Mix, versteht
Befunde und bedient die EQs zentral. Die sichtbaren Namen beider Flächen sind
noch nicht festgelegt.

### Fläche 1 (Arbeitsname „Overview")

**Inhalte:**
- **Quellenliste**: bis 16 sichtbare Quellen (Busse) mit Name, Rolle,
  Aktivität und Frische. Suche, Pinning und Fokusgruppen begrenzen die
  Last; ein Minimap-/Zoomprinzip schlägt „alles gleichzeitig".
- **Vergleichende Heatmap (Entwurfs-Annahme §35.1; Abnahme 20.08. = Overview + Detail; der Figma-Stand 21.08. zeigt ein Quellen×Band-Gitter — Klärung in der Übersetzung, NAK-38 im Technik-Repo)**: welche Quelle wo im Frequenzraum Energie
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

### Fläche 2 (Arbeitsname „EQ-Zentrale" / „EQ Center")

- **Sondenwahl direkt auf der Fläche:** zwischen den verbundenen
  Probeeq-Instanzen und dem Master wechseln, ohne Umweg über Fläche 1.
- **Zentraler EQ-Graph:** den EQ der gewählten Probeeq-Instanz fernbedienen;
  die DSP- und Zustands-Hoheit bleibt auf ihrem Bus. Optional zwei EQ-Spuren
  farblich unterscheidbar im selben Graph zeigen.
- **Master-EQ:** vollwertiger eigener EQ-Pfad der Gen-Instanz.
- **Ehrliche Zustände:** bestätigter Stand, flüchtiger Draft/Preview,
  Verbindungsausfall, Bypass, Schutzbereiche und eigener Revisions-/Undo-Weg.
  Die vollständigen technischen Regeln stehen in
  `ui-spielregeln-eq-zentrale.md`.

**Vier Bedienebenen, sichtbar getrennt** (nie vermischen):
Beobachten (nur messen) · Beraten (erklären) · Vorhören (flüchtig hörbar,
nur solange gehalten) · Anwenden (bewusst bestätigt, rückgängig machbar).

**Zustände, die immer ablesbar sein müssen:** Quelle frisch / veraltet /
getrennt; Messung läuft / unvollständig; eine Funktion ist auf diesem
System nicht verfügbar (degradiert) — ehrlich zeigen, nie kaschieren.

## 2. Nakama Probeeq — kompakte Bus-Sonde mit Doppelrolle

**Grundzustand:** Probeeq misst den Bus passiv und liefert Gen Messwerte,
Frische, Identität und Zustände. Bei ausgeschaltetem EQ verändert sie den
Audiopfad nicht.

**Zugeschalteter Zustand:** Dieselbe Instanz führt den von Gen
ferngesteuerten EQ auf ihrem Bus aus. Sie ist dann weiterhin Messsonde; die
Rollen schließen sich nicht aus. Parameter- und Zustands-Hoheit bleiben in
Probeeq, Gen ist die zentrale Bedien- und Ansichtsfläche.

**Kompakte Oberfläche:** Link-/Busidentität, Mess- und Verbindungszustand,
EQ-Status beziehungsweise geänderte Bänder sowie die dafür abgenommenen
direkten Handgriffe. Die aktuelle sichtbare Grundlage ist
`../assets/rework-basis-2026-08-25/probeeq.png`; ein zweiter vollflächiger
EQ-Editor ist ausdrücklich ausgeschlossen. Die vollständige Band- und
Kurvenbedienung liegt nur in Gens zweiter Oberfläche.

**Implementierungsstand:** Der heutige Code baut noch getrennte Suna- und
Probeeq-Hüllen; beide sind derzeit audio-neutral. Probeeq trägt bereits die
aktive Produktklasse und ihren Parameter-State, aber noch keine EQ-DSP. Das
ist Baustand, nicht die Produktstruktur (`../../docs/plugin-wissen.md`).

## Für alle drei Oberflächen

- **Editor-Panel** (für den User, im Prototyp): Look-Parameter live
  verstellbar, Zustands-Schalter für alle oben genannten Zustände,
  Varianten nebeneinander.
- **Demo-Daten**: realistisch (z. B. Klavier/Chor/Drums/Bass als
  Quellen), sichtbar als Demo markiert.
- **Maßstab**: ein Profi mit 20 Spuren muss in 2 Sekunden ablesen können,
  was Sache ist. Information vor Dekoration.
