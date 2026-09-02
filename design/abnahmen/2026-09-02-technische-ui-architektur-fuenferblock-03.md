# 2026-09-02 — Technische UI-Architektur, Fünferblock 03

## Status

**1 von 5 Entscheidungen gesammelt.** Der Block beginnt mit Gen Fläche 1
(Karte U21 aus `docs/plan/fragen.json`); danach folgen die offenen
Bedienbereiche von Gen Fläche 2 (Blueprint Abschnitt 15, Punkte 17 und 18)
und Probeeq (Punkt 19). Skizze, Detailabnahmen und Blueprint werden erst nach
der fünften Antwort gemeinsam umgebaut (Arbeitsmodus vom 31.08.2026,
Taktänderung vom 01.09.2026).

## Leitsatz vor der ersten Frage — weniger zeigen

Auf die erste Fassung der Frage U21, die den Befund um Beobachtung,
Sicherheit und Alternativen ergänzen wollte, antwortet der User:

> „wenn wir 6 werte für eine messung anzeigen, ist das schlichtweg schlechtes
> design. das liest keiner, das ist zuviel und zeigt ,dass wir nicht
> v erstehen worauf es ankommt“

Bindende Konsequenz für Gen Fläche 1:

- Die sechs Teile eines Ursachenbefunds aus dem Entwurf (Ort, Beobachtung,
  Zusammenhang, Alternativen, Sicherheit, nächster Beweisschritt; Entwurf
  Abschnitt 8, „Ergebnisform“) sind das Datenmodell und die Regelbasis des
  Advisors. Sie sind **keine** Vorgabe, sechs Werte sichtbar zu machen.
- Die Findings-Fläche zeigt nur, was der aktuelle Handgriff braucht. Eine
  Anzeige, die einen Befund als Liste seiner Datenfelder ausbreitet, ist
  verworfen.
- Die Frage U21 wird deshalb neu gestellt: nicht „wo erscheinen drei weitere
  Angaben“, sondern ob Sicherheit, Beleg und Alternativen überhaupt als
  eigene Angaben erscheinen oder in bereits vorhandenen Elementen aufgehen
  (Zustand des Befunds, markierte Zone im Graphen, Liste der Befunde).

Der Wortlaut der Antwort folgt als Entscheidung 1/5.

## 1/5 — Sicherheit, Beleg und Alternativen stecken schon drin (U21)

Auf die neu gestellte Frage, ob Sicherheit, Beleg und Alternativen auf Gen
Fläche 1 überhaupt als eigene Angaben zu den drei Zeilen (wahrscheinliche
Ursache, kleinster Test, worauf hören) dazukommen, wählt der User am
02.09.2026 um 09:35 Uhr:

> „Nein, sie stecken schon drin (Empfohlen)“

Damit ist für Gen Fläche 1 gesammelt:

- Die Findings-Fläche bleibt bei genau drei Zeilen je Befund: `LIKELY
  CAUSE`, `SMALLEST TEST`, `LISTEN FOR`. Es kommt keine vierte Zeile, kein
  Umschalter und kein Panel für Beleg, Sicherheit oder Alternativen dazu.
- **Sicherheit** ist der Zustand des Befunds, nicht ein eigener Wert: Nur ein
  sicherer Befund steht auf `READY TO SEND` und bietet `HOLD TO AUDITION`
  und `SEND DRAFT → EQ` an. Ein unsicherer Befund meldet `MORE DATA`, ein
  veralteter `STALE`; beide bieten keinen Draft an. Die Stufen hoch,
  mittel, unklar aus dem Entwurf werden im Datenweg auf diese Zustände
  abgebildet; die Abbildung ist Technik von S23–25.
- **Beleg** ist die markierte Zone im Frequenzgraph mit den beiden Kurven
  von Ziel und Referenz. Er wird nicht zusätzlich als Text wiederholt.
- **Alternativen** sind weitere Befunde in der Liste (`ALL FINDINGS`), nicht
  ein Feld im fokussierten Befund. Ein zweiter möglicher Verursacher wird
  damit als eigener Befund geführt, mit eigenem Zustand.
- Das Wort `CONFIDENCE` unter dem Graphen bleibt die Messqualität der
  Passage (Blueprint Abschnitt 4.1, Zone 3), nicht die Sicherheit des
  Befunds; beide dürfen nicht vermischt werden.
- Die technische Skizze zeigt diesen Stand bereits: Die drei Zeilen, die
  Zustände `READY TO SEND`, `MORE DATA`, `STALE` mit gesperrten
  Draft-Aktionen und die Befundliste sind vorhanden. Im gemeinsamen Umbau
  nach 5/5 wird nur geprüft, dass die Skizze keine weiteren Angaben
  vorgreift.

Karte U21 ist damit beantwortet (`docs/plan/fragen.json`, 02.09.2026). Der
Planschritt S23–25 trägt den Datenweg als datierten Nachtrag. Gen Fläche 1
hat nach dem Arbeitsmodus vom 31.08.2026 keinen offenen Punkt mehr; die
nächste Frage dieses Blocks gilt Gen Fläche 2.
