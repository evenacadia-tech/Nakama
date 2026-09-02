# 2026-09-02 — Technische UI-Architektur, Fünferblock 03

## Status

**0 von 5 Entscheidungen gesammelt.** Der Block beginnt mit Gen Fläche 1
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
