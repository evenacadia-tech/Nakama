# 2026-09-01 — Technische UI-Architektur, Fünferblock 01

## Status

**5 von 5 Entscheidungen gesammelt und gemeinsam integriert.** Skizze,
Detailabnahmen, Verhaltensvertrag und Blueprint wurden nach Entscheidung 5
zusammen aktualisiert und im internen Browser geprüft.

Für diesen Block gilt der neue User-Entscheid:

> „WÄRE ES nicht sinnvoller wir sammeln wieder ein paar entscheidungen? nach
> jeder frage direkt umbauen ist doch sehr umständlich ? wir können das fest
> machen. immer 5 fragen dann umbauen“

## 1/5 — Dynamic beim ersten Öffnen aktivieren

Auf die unmittelbar zuvor gestellte Architekturfrage wählt der User:

> „C“

Damit ist für Gen Fläche 2 gesammelt:

- Ein Klick auf `DYN · OFF` setzt `dynamic_enabled` für das ausgewählte Band
  auf `true` und öffnet im selben Handgriff die bereits entschiedene
  zweireihige Dynamic-Werteansicht.
- Ein Klick auf `DYN · ON` öffnet beziehungsweise verlässt nur diese Ansicht;
  er schaltet den Klangzustand nicht aus.
- Das Verlassen oder Schließen der Werteansicht lässt Dynamic aktiv.
- Der ausdrückliche Bedienort zum späteren Ausschalten bleibt eine echte
  offene Architekturfrage dieses Blocks.

Technisch bleiben `dynamic_enabled` und die fünf Werteparameter getrennte
persistente Parameter. Ein späteres Ausschalten darf `dynamic_range_db`,
`threshold_db`, `attack_ms`, `hold_ms` und `release_ms` daher nicht still auf
Defaults zurücksetzen.

Vor Eingang der Taktänderung war dieser erste Übergang in der laufenden Skizze
bereits begonnen und an einem zuvor inaktiven Band geprüft worden. Der
kohärente Fünfer-Umbau samt vollständigem Sichtbeleg und Repo-Integration bleibt
trotzdem bis 5/5 zurückgestellt.

## 2/5 — Ausschalten am Anfang der Dynamic-Ansicht

Auf die Frage nach dem ausdrücklichen Bedienort zum Ausschalten eines aktiven
Dynamic-Bands wählt der User:

> „A“

Damit ist für Gen Fläche 2 gesammelt:

- Am Anfang der geöffneten Dynamic-Werteansicht sitzt ein kompakter
  `dynamic_enabled`-Zustandscontrol für das ausgewählte Band.
- Dieser Control ist der ausdrückliche Ein-/Ausschalter. Der bereits
  entschiedene `DYN`-Control im Panelkopf bleibt Einstieg und Disclosure und
  schaltet ein aktives Dynamic-Band nicht durch bloßes Öffnen oder Verlassen
  der Ansicht aus.
- Ausschalten ändert ausschließlich `dynamic_enabled`; die fünf gespeicherten
  Dynamic-Werte bleiben erhalten.
- Welche Ansicht unmittelbar nach dem Ausschalten bestehen bleibt und wohin
  der Fokus geht, bleibt als nächste Architekturfrage offen.

## 3/5 — Nach dem Ausschalten zu den Grundwerten zurückkehren

Auf die Frage nach Ansicht und Fokus unmittelbar nach dem Ausschalten von
Dynamic wählt der User:

> „A“

Damit ist für Gen Fläche 2 gesammelt:

- Das Ausschalten über den Control am Anfang der Dynamic-Ansicht setzt
  `dynamic_enabled` auf `false`, lässt das objektgebundene Mini-Panel jedoch
  geöffnet.
- Im selben Panelkörper erscheinen wieder `Frequency`, `Gain` und `Q`.
- Der Tastaturfokus geht auf den weiterhin sichtbaren Einstieg `DYN · OFF`.
- Die fünf gespeicherten Dynamic-Werte bleiben unverändert und werden erst
  nach einer erneuten Aktivierung wieder eingeblendet.

## 4/5 — Pointer schließt nur über den eigenen Panel-Control

Auf die Frage nach dem Schließen des Mini-Panels per Zeiger wählt der User:

> „B“

Damit ist für Gen Fläche 2 gesammelt:

- Das Mini-Panel behält einen eigenen, sichtbaren Schließen-Control.
- Ein einfacher Klick auf eine freie Stelle des EQ-Graphen lässt das Panel
  geöffnet und verändert weder Auswahl noch Parameterzustand.
- `Escape` bleibt der technisch festgelegte Tastaturweg: Eine laufende
  Werteingabe wird zuerst abgebrochen; andernfalls schließt es das oberste
  nichtmodale Panel.
- Schließen verändert weder den Bandzustand noch `dynamic_enabled` und gibt
  den Tastaturfokus an den zugehörigen Bandpunkt zurück.

## 5/5 — Ruhige Kontur plus tatsächliche Dynamic-Auslenkung

Auf die Frage, wie ein aktives Dynamic-Band bei geschlossenem Mini-Panel
dauerhaft erkennbar bleibt, wählt der User Variante A und ergänzt:

> „A ; DAS BANDSCHWINGT ja dann eh sichtbar mit, daran erkennt man es auch“

Damit ist für Gen Fläche 2 entschieden:

- Ein aktives Dynamic-Band besitzt am Bandpunkt eine zweite, ruhige Kontur.
  Sie belegt den aktiven Zustand auch in stillen Passagen und ohne Farbe oder
  ausgeschriebenen Status.
- Die Kontur bleibt an der eingestellten Gain-Position und bildet zugleich das
  stabile Zeigerziel. Der innere Bandpunkt und der zugehörige Kurvenzug gehen
  mit der tatsächlichen dynamischen Gain-Auslenkung mit. Dadurch wird die
  Bewegung ablesbar, ohne dass der direkte Bedienpunkt selbst zum wandernden
  Ziel wird.
- Die Bewegung ergänzt den persistenten Aktivbeleg, ersetzt ihn aber nicht:
  Unterhalb des Thresholds oder bei `dynamic_range_db = 0` kann ein korrekt
  aktives Band zeitweise stillstehen.
- Es gibt keine dekorative Endlosschwingung. Die native Bewegung muss aus
  einer autoritativen, bandbezogenen Live-Auslenkung stammen.

## Technische Grenze der Live-Bewegung

Der aktuelle Parametervertrag enthält `dynamic_enabled`,
`dynamic_range_db`, `threshold_db`, `attack_ms`, `hold_ms` und `release_ms`.
Der Name `Frame.band_dynamic_gain_db` ist inzwischen für S26–28
(`SONDE-015`) reserviert. Feld-ID und Nutzlast existieren im aktuellen
Runtime-Vertrag jedoch noch nicht: Probeeq liefert den Wert später mit
Anzeigekadenz über den Featureframe, Gens Master-EQ lokal ohne IPC. Die Skizze
zeigt deshalb beim Aktivieren genau eine
Bewegungsprobe als ausdrücklich bezeichnetes **Zielverhalten**. Für den
nativen Editor bleibt die fortlaufende Bewegung bis zu einem versionierten,
autoritativen Telemetrieweg technisch offen; sie darf nicht aus den
Einstellwerten erfunden werden.

## Gemeinsamer Umbau und Sichtbeleg nach 5/5

Die laufende technische Skizze unter
`C:\Users\phili\.codex\visualizations\2026\08\31\01a056ed-2436-7fc1-9caf-fe883e3c1467\nakama-ui-technical-sketch.html`
setzt alle fünf Entscheide gemeinsam um. Im internen Browser wurden am
01.09.2026 geprüft:

- `DYN · OFF` aktiviert und öffnet in einem Schritt; `Range` erhält den Fokus.
- Der kompakte `DYNAMIC · ON`-Control steht als erstes Element der zweireihigen
  Ansicht. Ausschalten erhält alle fünf Werte, stellt `Frequency`, `Gain` und
  `Q` wieder her und fokussiert `DYN · OFF`.
- Verlassen der Dynamic-Ansicht und explizites Schließen lassen
  `dynamic_enabled` unverändert.
- Ein einfacher Leerklick im Graph lässt Panel, Bandauswahl und Parameterstand
  unverändert.
- Eine geänderte Zahleneingabe wird mit dem ersten `Escape` auf ihren
  Ausgangswert zurückgesetzt; ein weiteres `Escape` schließt das Panel und
  fokussiert den Bandpunkt.
- Der sichtbare Schließen-Control schließt das Panel und fokussiert ebenfalls
  den Bandpunkt.
- Dynamic-Konturen waren für die aktiven Bänder B3 und B5 auch bei
  geschlossener Werteansicht sichtbar. Beim erneuten Aktivieren lief die
  einmalige Bewegungsprobe; danach endete die Animation vollständig.
- Der 238 × 114 Pixel große Dynamic-Panelkörper blieb bei B3 und B5 vollständig
  innerhalb des Graphen und ohne Kollision zum Bandpunkt. Ein nahe am unteren
  Graphrand erzeugtes B7 wurde oberhalb platziert und blieb ebenfalls
  vollständig innerhalb des Graphen.
- Die Browserkonsole enthielt weder Fehler noch Warnungen.

Farben, Material, Typografie und visuelle Feinheiten waren nicht Teil dieser
Prüfung.

## Nachtrag 02.09.2026 — Ablageort der Skizze

Der oben genannte Pfad im Codex-Visualisierungsordner war der Arbeitsort am
01.09.2026 und ist auf dem Laptop nicht erreichbar. Dieselbe Datei liegt seit
dem 02.09.2026 byteidentisch im Repo unter
[`design/skizze/nakama-ui-technical-sketch.html`](../skizze/nakama-ui-technical-sketch.html);
Sichtbelege des Stands vom 02.09.2026 liegen unter `design/skizze/belege/`.
Die Skizze ist der derzeitige Stand, nicht der finale (User-Wort 02.09.2026 in
[`2026-08-31-technische-ui-architektur-arbeitsmodus.md`](2026-08-31-technische-ui-architektur-arbeitsmodus.md)).
