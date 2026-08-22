# 2026-08-20 — Zustände: nur die Ausnahme spricht

**Abnahme.** Beurteilt an drei Blättern auf 760×430 mit durchschaltbarer
Zustandsmatrix — sechs Szenarien, drei Messstände, drei Arbeitsschritte.

## Das wörtliche Urteil

> „3. ist nicht perfekt aber das einzige was sinn ergibt"

## Was damit festliegt

**Frische Quellen sehen normal aus. Nur Abweichung wird gezeichnet.**

| Zustand | in der Karte | im Streifen |
|---|---|---|
| frisch | gefüllte Zellen | schweigt (siehe Nachtrag) |
| veraltet | **hohle** Zellen — Daten da, aber alt | „2 stale" |
| getrennt | **Strichlinie**, keine Zellen — keine Daten | „1 offline" |

Zwei Regeln, die dabei mit festliegen:

1. **Keine Zeile ändert ihre Höhe.** Sonst läse man Energie, wo Alter
   gemeint ist — die Karte ist ein Vergleichsinstrument.
2. **Die drei Sorten Zustand bleiben getrennt:** je Quelle · global
   (Messung) · die App (degradierte Funktion).

## Nachtrag: „nicht perfekt" war die Aufgabe

Der genannte Preis dieser Mechanik lautete: *„alles in Ordnung" wird durch
Abwesenheit gezeigt — man muss darauf vertrauen, dass nichts zu sehen auch
wirklich nichts heißt und nicht ein Ausfall der Anzeige.* Zwei Eingriffe,
beide gemessen:

**1. Abwesenheit bekommt eine positive Aussage.** Ist nichts zu melden,
sagt der Streifen **„All 16 fresh"** — eine einzige Zeile, genau dann, wenn
sonst nichts dort stünde. Sie kostet nichts im Ernstfall, weil sie dort
verschwindet. Das ist nicht das „13 fresh" der Sammelzeile: das zählte
immer mit, dieses spricht nur, wenn es sonst still wäre.

**2. Die Ausnahme spricht mit Namen.** Eine hohle Zeile in einer namenlosen
Karte sagt nur die Position. Jetzt: **„2 stale (Room Mic, Backing Vox) ·
1 offline (Shaker)"**.

Die Grenze dafür ist gemessen, nicht gedreht: bei „viel veraltet" (8) sprengt
die Namensliste den Streifen um **42 px**. Regel deshalb: **Namen, solange es
eine Aufzählung ist** — bis zwei je Sorte. Acht Namen sind keine Aufzählung
mehr, sondern eine Zahl.

Gemessen, was der gewählte Entwurf je Lage sagt:

| Szenario | Streifen |
|---|---|
| alles frisch | `All 16 fresh · Measuring 68 % · Width: unavailable` |
| gemischt | `2 stale (Room Mic, Backing Vox) · 1 offline (Shaker) · …` |
| viel veraltet | `8 stale · 2 offline (Backing Vox, Shaker) · …` |
| alles getrennt | `16 offline · Measurement stalled — no source reachable` |
| keine Quelle | `No source connected · Nothing to measure` |

## Was der schlimme Fall vorher ans Licht gebracht hat

Beim Umschalten auf *alles getrennt* zeigte das Blatt weiter drei Befunde
über Bass, Choir und Drums, als wäre gerade gemessen worden. Behoben, und
zwar als Regel: **„ehrlich zeigen, nie kaschieren" gilt nicht nur für die
Zustandsanzeige, sondern für alles, was aus dem Zustand folgt** — Befunde
sind Schlüsse aus Daten, Knöpfe sind Versprechen über Wirkung, ein
Prozentwert ist eine Behauptung über laufende Arbeit.

- Befunde tragen den Zustand ihrer Quelle.
- Der Assistent lässt auf einer getrennten Quelle nichts mehr zu.
- Messen setzt Kontakt voraus.

## Was ausdrücklich offen bleibt

- **Wie man von einer hohlen Zeile zur Quelle kommt.** Die Karte ist
  namenlos; der Name steht im Streifen, solange es wenige sind, sonst in der
  Quellenliste einen Schritt entfernt. Ob das reicht, zeigt erst der Gebrauch.
- Ob **veraltet** eine Abstufung braucht (seit 10 s / seit 10 min).
- Mehrere angewandte Änderungen (Zähler ohne Verlauf).
- Alles Visuelle.

## Damit ist Main auf Wireframe-Ebene vollständig

Größe · Disclosure-Mechanik · Inhalt der dauerhaften Karte · Aufteilung ·
Pflichtinhalt der Arbeitsfläche · Zeichenbild der Bedienebenen · Zustände.

Als Nächstes steht laut `docs/arbeitsplan.md` der **Active-Probe-Editor**
(700×420) an, danach die **Passive-Probe-Kachel** (260×84).

**Kein Prüfpunkt ausgelöst:** Welt und Fassung bleiben Arbeitsannahmen. Der
Prüfpunkt lautet „sobald das Layout steht" — Main steht, aber Stufe 2
Schritt 2 („Early Visual Layer") ist nicht begonnen und zwei Oberflächen
fehlen.

## Bezug

- `werkzeug/LIES-MICH.md`, Abschnitt `zustaende.html`
- [vorhoeren-markierte-zeile](2026-08-20-vorhoeren-markierte-zeile.md)
- [karte-alle-quellen](2026-08-20-karte-alle-quellen.md)
