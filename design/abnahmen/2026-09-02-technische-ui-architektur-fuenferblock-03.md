# 2026-09-02 — Technische UI-Architektur, Fünferblock 03

## Status

**5 von 5 Entscheidungen gesammelt; gemeinsamer Umbau läuft.** Der Block
begann mit Gen Fläche 1 (Karte U21 aus `docs/plan/fragen.json`); danach
folgten Gen Fläche 2 (Blueprint Abschnitt 15, Punkte 17 und 18) und Probeeq
(Punkt 19). Skizze, Detailabnahmen und Blueprint werden nach der fünften
Antwort gemeinsam umgebaut (Arbeitsmodus vom 31.08.2026, Taktänderung vom
01.09.2026); der Sichtbeleg steht am Ende dieser Datei.

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

## 3/5 — Der Kopf schaltet die Flächen; SEND DRAFT ist nicht der einzige Hinweg

Auf die Frage nach dem Rückweg von Gen Fläche 2 zu Fläche 1, die als
Voraussetzung nannte, dass es nur über `SEND DRAFT` hin geht, korrigiert der
User am 02.09.2026:

> „nein hin geht es nicht nur über send draft, sondern über den header wo EQ
> steht“

Damit ist für beide Gen-Flächen gesammelt:

- Der Kopf von Gen trägt den Flächenwechsel: Dort, wo der Name der zweiten
  Fläche steht (in der Skizze der Seitentitel `EQ CENTER`; der Produktname
  ist Karte U23), führt ein Handgriff von Fläche 1 zur EQ-Fläche. Der Kopf
  ist damit Produkt-UI und kein Inspektionswerkzeug mehr.
- `SEND DRAFT → EQ` bleibt als Handgriff am Befund erhalten: Er übergibt den
  Draft und wechselt zur EQ-Fläche. Er ist aber nicht mehr der einzige
  Hinweg; die EQ-Fläche ist auch ohne Befund und ohne Draft erreichbar, zum
  Beispiel für Master-EQ und Sonden-Durchschaltung.
- Der Entscheid vom 31.08.2026 „keine funktion um auf die 2. seite zu
  wechseln abgesehen von send draft“ ist damit **überholt**. Er bleibt im
  Arbeitsmodus-Protokoll stehen und ist dort mit Datum als überholt
  markiert; Blueprint Abschnitt 4.1 („Übergang“) und das Skizzen-Leseblatt
  werden beim gemeinsamen Umbau nach 5/5 nachgezogen.
- Wie der Rückweg im selben Kopf aussieht und ob Gen nach Abschluss einer
  Transaktion von selbst zurückkehrt, ist die nächste Frage dieses Blocks.

Nachtrag zum Stand der Skizze im Repo: Der User sagt dazu „die funktiopn
ist schon in der skizze, man kann über den header wo eq steht auf die 2.
seite schalten“. In der committeten Skizze (`design/skizze/`, Stand
02.09.2026) ist der klickbare Wechsel `GEN · SOURCES / GEN · EQ / PROBEEQ`
die Inspektionsleiste **oberhalb** des Gen-Fensters; der Seitentitel `EQ
CENTER` im Kopf des Fensters selbst ist dort nur Text. Mit diesem Entscheid
wird der Flächenwechsel beim Umbau nach 5/5 in den Kopf des Gen-Fensters
(950×538) übernommen; die Inspektionsleiste bleibt Werkzeug für `PROBEEQ`
und `SCENARIO`. Liegt auf dem PC eine neuere Skizze aus einer
Codex-Sitzung, die den Wechsel schon im Fensterkopf trägt, gilt sie erst,
wenn sie in `design/skizze/` committet ist.

## 4/5 — Rückweg über denselben Kopf, keine automatische Rückkehr

Auf die Frage, ob der Kopf auch für den Weg zurück gilt und ob Gen nach
einer abgeschlossenen Änderung (`APPLY` fest oder `REJECT`) von selbst
zurückspringt, wählt der User am 02.09.2026:

> „Nur der Kopf, nichts automatisch (Empfohlen)“

Damit ist für beide Gen-Flächen gesammelt:

- Der Kopf trägt beide Flächennamen als stabilen Umschalter; die aktive
  Fläche ist dort erkennbar. Zurück geht es über den Namen der ersten
  Fläche, hin über den Namen der EQ-Fläche.
- Nach `APPLY` fest oder `REJECT` bleibt Gen auf der EQ-Fläche. Es gibt
  keine automatische Rückkehr, auch nicht nach `SEND DRAFT`.
- Ein offener Draft bleibt beim Flächenwechsel offen und verändert sich
  nicht. Fläche 1 zeigt ihn am betroffenen Befund als Zustand (in der
  Skizze `DRAFT OPEN · EQ`); der Handgriff am Befund führt zurück zur
  EQ-Fläche. Ein laufender 10-Sekunden-Kandidat zählt auf Fläche 1 sichtbar
  weiter und bietet dort keinen zweiten Draft an — das zeigt die Skizze
  heute bereits (`EQ CANDIDATE · n s`, `SEND DRAFT` gesperrt).
- Technische Grenze, unverändert: Der Flächenwechsel ist reine Anzeige. Er
  berührt weder den Draft noch Kandidat, Bestätigung oder Revision; die
  Transaktionslogik bleibt bei S29–31 und dem Kandidatenvertrag (Blueprint
  Abschnitt 15, Punkt 12).
- Blueprint Abschnitt 15 Punkt 18 (Rückweg) ist damit entschieden und wird
  beim Umbau nach 5/5 auf **[U]** gesetzt; Abschnitt 4.1 „Übergang“ und die
  Skizze folgen im selben Umbau.

## Prozesswort vor 5/5 — Fundament jetzt, visuelles Design danach in Figma

Auf die erste Fassung der Frage nach der Größenklasse von Probeeq antwortet
der User ohne Auswahl mit einer Anmerkung:

> „so dass wir später wenn layout /skizze steht daraus auch ein schönes
> design in figma entwickeln können. (wie bei GEN) aktuell machen wir ja das
> fundament danach kommt das visuelle design in figma.“

Bindende Konsequenz für den Arbeitsmodus:

- Die laufende Phase liefert das Fundament: Layout, Hierarchie, Handgriffe
  und Zustände in der technischen Skizze, für beide Gen-Flächen und für
  Probeeq.
- Die visuelle Phase folgt danach und findet in Figma statt, wie beim
  früheren Gen-Stand. Farben, Material, Typografie und Feinheiten werden
  weiterhin nicht in der Skizze entschieden. Welche Figma-Datei dann gilt,
  benennt der User beim Wechsel in diese Phase; die Figma-Stände bis zum
  25.08.2026 bleiben Verlauf.
- Die Größenklasse von Probeeq gehört zum Fundament (wie 950×538 bei Gen)
  und wird deshalb hier entschieden, nicht erst in Figma.

## 5/5 — Probeeq ist eine flache Kachel mit ein bis zwei Zeilen

Technische Vorbedingung, vor der Frage geprüft: Der Inhalt der
Probeeq-Rückfallfläche ist seit dem 23.08.2026 entschieden
([`2026-08-23-interview-struktur.md`](2026-08-23-interview-struktur.md),
Frage 13: „EQ mit rotem oder Eq mit grünem punkt. Connected oder
disconnected, bypass und bei eq on die werte + den mode der probe“), und
Blueprint Abschnitt 4.3 führt ihn. Die Größe war nie entschieden: Die
Formate vom 20.08.2026 ([`2026-08-20-groessen-alle-drei.md`](2026-08-20-groessen-alle-drei.md))
galten drei Apps, darunter eine Passiv-Kachel von 260×84 mit der
Begründung, dass diese Fläche vielfach gleichzeitig offen ist; die Skizze
trug 760×430 nur als Werkzeugannahme.

Auf die Frage nach der Größenklasse wählt der User am 02.09.2026:

> „Flache Kachel, ein bis zwei Zeilen (Empfohlen)“

Damit ist für Probeeq gesammelt:

- Die Rückfallfläche ist eine flache Kachel. Zeile 1 trägt dauerhaft Name
  und Bus, Verbindung, den EQ-Punkt (an/aus), Bypass und Mix. Zeile 2
  erscheint nur bei zugeschaltetem EQ und trägt die laufende Änderung
  (Band, Frequenz, Gain, Q) und den Modus der Sonde.
- Ein kompaktes Feld halber Gen-Größe und die heutige 760×430-Bühne sind
  verworfen. Die Begründung vom 20.08.2026 gilt weiter: Probeeq ist auf
  vielen Bussen gleichzeitig offen, jeder Pixel Höhe zählt mal Instanzen.
- Die konkreten Maße der Kachel in der Skizze sind eine Dichteprüfung des
  Umbaus, kein Beschluss; sie werden unten gemessen genannt. Ob die zweite
  Zeile die Fensterhöhe verändert oder in fester Höhe ein- und
  ausgeblendet wird, ist eine technische Frage des Host-Fensters (JUCE
  `setSize` unter FL) und bleibt als Vorgriff im Skizzen-Leseblatt benannt.
- Blueprint Abschnitt 15 Punkt 19 ist damit für die Größenklasse
  entschieden; Abschnitt 4.3 erhält beim Umbau die Kachelform als **[U]**.
