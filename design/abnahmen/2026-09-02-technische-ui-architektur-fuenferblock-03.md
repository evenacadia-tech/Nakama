# 2026-09-02 — Technische UI-Architektur, Fünferblock 03

## Status

**2 von 5 Entscheidungen gesammelt.** Der Block beginnt mit Gen Fläche 1
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

## 2/5 — Die eingeklappte Global-Zeile nennt nur, was gerade wirkt

Technische Vorbedingung, vor der Frage geprüft: Der Parametervertrag
(`eq-copilot/schemas/state/nakama-parameter-v1.json`, `global`) kennt heute
Bypass, Input-Trim, Output-Trim, Width und Mono-Bass. Dazu kommen mit
Layout v2 in S26–28 die entschiedenen Erweiterungen Auto-Gain neben Output
(24.08.2026), Mix je Sonde (23.08.2026), das Preset-Objekt (U18, 30.08.2026)
und A/B; Revision/History ist der Undo-Ring des Entwurfs. Blueprint
Abschnitt 4.2 Zone 4 hält als **[U]** fest, dass die Global-Sektion im
Ruhezustand eingeklappt ist, und als **[A]** die Hypothese, dass aktive
Abweichungen am Einstieg sichtbar bleiben. Genau diese Hypothese war die
Frage.

Auf die Frage, was die eingeklappte Global-Zeile unter dem Graphen im
Ruhezustand zeigt, wählt der User am 02.09.2026:

> „Nur was gerade wirkt, beim Namen (Empfohlen)“

Damit ist für Gen Fläche 2 gesammelt:

- Die eingeklappte Zeile nennt die aktiven Abweichungen vom Standard beim
  Namen, zum Beispiel `GLOBAL · MIX 92 % · AUTO · B` oder `GLOBAL · BYPASS`.
  Sind alle globalen Werte Standard, steht dort nur `GLOBAL` mit dem
  Aufklapp-Control.
- Ein Zähler wie das heutige `GLOBAL · 2 ON` ist verworfen: er sagt nicht,
  was wirkt.
- Eine dauerhaft sichtbare Kurzzeile mit Bypass, Mix und A/B auch im
  Standardzustand ist verworfen: neutrale Werte belegen keinen Platz unter
  dem Graphen.
- Was als Abweichung gilt, folgt dem Vertrag: Bypass an, Mix ungleich 100 %,
  Auto-Gain an, Kurve B aktiv, Input- oder Output-Trim ungleich 0 dB, Width
  ungleich 1, Mono-Bass ungleich 0 Hz. Die Reihenfolge und Kurzform der
  Nennungen sind Dichteprüfung der Skizze, kein Beschluss.
- Aufgeklappt liegen dort alle globalen Handgriffe: Input, Output mit
  Auto-Gain daneben, Width, Mono-Bass, Bypass, Mix, A/B, Voreinstellungen
  und der Verlauf. Wie Voreinstellungen und Verlauf innerhalb der
  aufgeklappten Sektion geöffnet werden, ist noch nicht gefragt.
- Damit ist die Blueprint-Hypothese **[A]** aus Abschnitt 4.2 Zone 4 zu
  **[U]** geworden; Blueprint Abschnitt 15 Punkt 17 verliert den Teil
  „Global-Sektion nicht abgeleitet“ beim gemeinsamen Umbau nach 5/5. Der
  Wortlaut der Halten-Aktion und die Kurzform `DEL` bleiben dort offen.
