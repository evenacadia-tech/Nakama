# 2026-08-23 — Gen wird EQ-Zentrale: eine Sonde, Master-EQ, Seite 2

> ## Entscheid (bindend, User-Wort 23.08.2026)
>
> Der User hat die Produktarchitektur umgestellt. Das Gespräch begann
> ausdrücklich als Idee („das war keine anweisung, sondern ich will das
> durchsprechen mit dir") und endete mit „ja festhalten". Alle Zitate
> wörtlich, in der Reihenfolge des Gesprächs.

## 1. Die Idee (Ausgangswortlaut)

> „aktuell ist es ja so Probeeq wird ein vollwertiger EQ verbunden mit Gen.
> Meine idee die ich hatte war: STATT mehrere eq auf den instrumenten liegen
> zu haben, könnte man das EQ UI des Probeeq auf eine neue 2. seite auf den
> Gen legen. der probeeq fällt entweder ganz weg oder wird mit dem Suna
> gemergt. Im GEN kann man dann jede Sonde im EQ mode auswählen und
> einstellen. so könnte man super intuitiv an einem ort jeden EQ steuern,
> statt sich von Probeeq zu Probeeq mühsam durchzuarbeiten. Also im Prinzip
> liegt der EQ auf den Bussen wird aber eingestellt auf dem Gen. Das wäre
> eine echte innovation, die es so nicht gibt auf dem markt. dazu erhält der
> Gen noch einen EQ für den master selbst auf dem er sitzt, das wäre
> maximale kontrolle mit maximalem workflow gewinn."

## 2. Merge der Sonden

Frage: „Was wird aus Probeeq und Suna, wenn die EQ-Bedienung in den Gen
wandert?" — Wahl des Users: **„Merge: eine Sonde"** (Claudes Vorschlag,
vom User gewählt): Suna und Probeeq werden EINE Sonde, EQ per Klick in Gen
zuschaltbar; aus „beobachten" wird „korrigieren" ohne Plugin-Tausch im
Mixer; EQ aus = beweisbar bitidentisch (NullTest-Standard).

## 3. Gen als Hub mit Master-EQ (voller Wortlaut)

> „Gen wird ein vollwertiges HUB oder Studio , dass auf einer 2. neuen
> Fläche nahtlos zwischen allen eingebauten Sonden durchschalten kann. und
> hier kommt das besondere. es ist ein vollwertiger eq für den master wie
> andere eq auf dem markt, aber man a) durch intuitive UI zwischen den
> Instrumenteneq durchklicken, man kann aber auch die masterspur + parallel
> eine andere spur anzeigen lassen. Man kann im Mastereq ,wenn man eine
> frequenz bearbeitet den angewendeten effekt solo stellten und durch
> wechseln auf sonden hören inwiefern das das instrument beeinflusst. das
> ziel ist maximale kontrolle ohne überkomplexe workflow"

## 4. Festhalten + Design-Vorgaben (voller Wortlaut)

> „ja festhalten aber wichtige dinge zum design: auf der 2. seite dem EQ
> gibt eine UI an dem man die sonden durchklicken kann, nicht durch die
> 1. seite. das wäre wieder nicht intuitiv. in der musikproduktion ist
> workflow alles. 1 klick oder 3 klicks dazwischen liegen welten. zudem bei
> 2 EQ spuren liegen sie nicht nebeneinander, sondern sind in EINEM Graph
> visuell unterscheidbar durch zum beispiel farben."

## 5. Name der verschmolzenen Sonde

Wahl des Users: **„Nakama Probeeq"** — Suna entfällt als App-Name.

## Was damit festliegt

1. **Zwei Apps statt drei:** Gen (Hub/„Studio") und Probeeq (DIE Sonde).
   Bundle bleibt „Nakama Studio".
2. **Der EQ rechnet auf den Bussen (in der Sonde) und in Gen (Master),
   bedient wird zentral auf Gens Seite 2.**
3. **Design-Vorgaben Seite 2:** Sonden-Durchschalter direkt AUF der
   EQ-Seite, nie über Seite 1 (1 Klick, nicht 3); zwei EQ-Spuren in EINEM
   Graph, visuell unterscheidbar z. B. durch Farben.
4. **Grundgesetz-Folge:** „Gen und Suna beraten nur" in der Absolutform ist
   überholt. Es gilt: **nichts Ungefragtes** — Audio wird nur verarbeitet,
   wo der User es einschaltet; aus = beweisbar bitidentisch (dasselbe
   verriegelte-und-bewiesene Muster wie die Hör-Markierung).

## Folgen, Zuordnung Entscheid / Vorschlag

- **Entscheid** (oben, wörtlich): Merge · Name Probeeq · Master-EQ in Gen ·
  Seite-2-Durchschalter · Ein-Graph-Overlay mit Farben.
- **Vorschlag Claude, im Gespräch unwidersprochen, NICHT abgenommen:**
  (a) Minimal-Rückfallfläche der Sonde (Bypass, Kurve lesbar, „In Gen
  öffnen") statt gar keiner UI; (b) „Effekt solo + Wirkung je Instrument"
  zuerst als Attribution über die vorhandene Sonden-Telemetrie (sehen, wen
  man anfasst) plus Sprung zur Sonde mit vorgeladener Frequenz
  (Remote-Preview aus P7) — echtes Bus-Audio-Streaming nach Gen als spätere
  Ausbaustufe geparkt; (c) Neudeutung des ±3-dB-Caps: es bindet künftig
  Advisor-Vorschläge, User-Edits über Gen sind der manuelle Weg (±12 dB) —
  braucht User-Wort (NAK-64); (d) Parameter- und Zustands-Hoheit bleibt in
  der Sonde, Gen ist Fernbedienung (Host-Automation ist instanzgebunden —
  Befund, folgt aus der DAW-Mechanik).
- **Markt-Befund** (Claude, 23.08., Web-Recherche): Fernbedienung anderer
  Instanzen existiert (FabFilter Pro-Q 4 „Instance List", sonible smart:EQ 4
  „Group View") — die „echte Innovation" ist nicht die Fernsteuerung,
  sondern der dedizierte Hub mit Advisor: Befund sehen → im selben Fenster
  korrigieren, plus Sonden-Upgrade ohne Plugin-Tausch.

## Bezug

- Register in `Nakama/CLAUDE.md` (drei neue Zeilen 23.08.)
- Entwurf-Erratum (n) in `docs/FL-Nakama-Sonden-Design-Entwurf.md`
- Umschnitt-Arbeit: NAK-64 · Figma-Flächen: NAK-65 (`docs/offene-punkte.md`)
- Fernsteuer-Mechanik, die das trägt: Entwurf Kernfunktion 16, §44, P7
  (Pairing, HMAC, Revision, Lease, Revert)
