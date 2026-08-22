# 2026-08-20 — Korrektur: Formfaktor und Größe kommen zuerst

## Das wörtliche Urteil

> „jetzt hast du 50 % tote leere fläche, die dem nutzer platz in fl studio
> raubt. merkst du nicht dass du erstmal ganz an den anfang zurück musst?
> nämlich formfaktor, größe"

## Der Fehler

Alle bisherigen Entwürfe entstanden auf 840×560 — einer Leinwand, die ich mir
selbst ausgedacht und nie hinterfragt habe. In einem Plugin ist Fläche kein
freies Gut: jeder Pixel geht dem Arrangement ab. Größe und Form sind deshalb
keine späte Feinjustierung, sondern die erste Randbedingung. Sie entscheiden,
wieviel überhaupt sichtbar sein *kann* — und damit über jede
Disclosure-Entscheidung, die darauf aufbaut.

## Die gemessenen Randbedingungen

- **Bildschirm des Users: 1920×1080, ein Monitor, 100 % Skalierung**,
  Arbeitsfläche 1920×1032 (gemessen, nicht geschätzt).
  Mein 840×560-Fenster war damit **24 % des gesamten Bildschirms** — halb leer.
- **FL Studio multipliziert die native Plugingröße**:
  `final = original × GUI-Skalierung × Wrapper-Faktor`. Die native Größe ist
  also eine Untergrenze, keine Endgröße.
- **Frei ziehbare Plugin-GUIs sind in FL Studio unzuverlässig**: die Größe wird
  nicht gemerkt und springt nach dem Öffnen der Wrapper-Einstellungen zurück.
  → **Empfehlung: feste Größenstufen statt freiem Ziehen**, wie FabFilter es
  mit Mini/Small/Medium/Large/XL macht. Noch nicht abgenommen.

## Die entscheidende Messung

`werkzeug/formfaktor.html` legt denselben Pflichtinhalt in verschieden große
Fenster und misst auf jeder Ebene, ob etwas abgeschnitten wird:

| Inhaltsmenge | kleinste Größe ohne Verlust |
|---|---|
| Alles gleichzeitig | **980×560** (bei 760×430 fehlen 45 px, bei 520×300 fehlen 371 px) |
| Nur der aktuelle Handgriff | **~520×340** |

Das ist der Preis der alten Denkweise in Zahlen: **rund dreifache Fläche** für
dieselbe Aufgabe. Progressive Disclosure ist damit in diesem Projekt keine
Stilfrage, sondern die Bedingung dafür, dass Nakama neben FL Studio überhaupt
Platz hat.

## Was damit festliegt

1. **Formfaktor und Größe werden vor jedem Wireframe entschieden**, für alle
   drei Oberflächen. Der Arbeitsplan wird entsprechend geändert.
2. **Größenentscheidungen werden nur bei 1:1 beurteilt.** Ein skaliertes Bild
   taugt dafür nicht; das Werkzeug prüft und meldet, ob gerade wirklich 1:1
   gilt.
3. **Tote Fläche ist ein Fehler, kein Gestaltungsspielraum.** Ein Fenster, das
   halb leer ist, nimmt dem Arrangement Platz ohne Gegenleistung.

## Was ausdrücklich offen bleibt

- Die tatsächlichen Größen aller drei Oberflächen.
- Fest, Stufen oder frei ziehbar.
- Ob Main eine liegende Fläche, ein Breitband am unteren Rand oder eine Säule
  am Seitenrand ist — die Formfrage ist noch offen, nicht nur die Größe.
