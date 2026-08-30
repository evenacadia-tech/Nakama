# 2026-08-30 — Fragenrunde Marktstandard: U20, U16, U17, U15, U18

Gestellt in der Dirigenten-Sitzung am 30.08.2026, nachdem der Marktvergleich
`docs/beweise/PR1.md` fünf Lücken gegenüber vergleichbaren Produkten belegt
hatte. Jede Antwort ist eine Auswahl des Users und steht hier **wörtlich**.

Diese Datei ist der bindende Entscheidort für **U20, U16, U17, U15 und U18**
(Aussehen und Verhalten). Die sechste Karte derselben Runde, **U19**
(Referenz-Nachbilden — Antwort: „Bleibt hinten (Empfohlen)"), ist eine reine
Reihenfolge-Entscheidung ohne Gestaltungsfolge und steht nur in
`docs/plan/fragen.json` und im Plan (S16–17).

## U20 — was der Sonden-Durchschalter zeigt, wenn FL den Spurnamen nicht hergibt

**Frage:** Du hast am 23.08. festgelegt, dass der Durchschalter die
FL-Mixer-Reihenfolge hält und je Sonde den Bus-Namen zeigt. Beides kann nur FL
selbst liefern, und ob FL das überhaupt herausgibt, ist nie gemessen worden.
Wir messen es. Die Frage an dich ist der Fall danach: **wenn FL nichts
hergibt** — was soll dann im Durchschalter stehen? Möglich wären ein Name, den
du selbst vergibst, die Nummer des Mixerkanals, sofern die ankommt, oder eine
Reihenfolge nach dem Zeitpunkt des Verbindens.

**Antwort (Auswahl):** „Eigener Name (Empfohlen)" — du vergibst je Sonde einen
Namen (wie heute das Label); liefert FL den Bus-Namen doch, ersetzt er ihn
automatisch.

**Folge:** Der Rückfall ist der vom User je Sonde vergebene Name — genau das
heutige `label`-Feld. Es bleibt User-Wort und damit untrusted: Länge begrenzen,
nie als Hostwahrheit auszeichnen, nie als Pfad oder Markup deuten. Ein vom Host
gelieferter Bus-Name hat Vorrang und ersetzt den selbst vergebenen Namen
automatisch; der eigene Name bleibt gespeichert und trägt wieder, sobald der
Host nichts mehr liefert. Mixer-Nummer und Verbindungsreihenfolge werden
ausdrücklich **nicht** als Rückfall gebaut. Messung, Capabilitybit und
Vertragsort liegen bei S18–19, die Anzeige bei S31b.

## U16 — Überdeckungen direkt im Kurvenbild

**Frage:** Geplant ist heute, dass Nakama dir in Worten sagt, dass zwei Quellen
sich im selben Bereich streiten, mit Ort, Beleg und dem kleinsten Gegentest.
Nicht geplant ist, dass du beim Arbeiten am EQ direkt im Bild siehst, wo es
sich zudeckt — etwa als farbige Zone auf der Kurve. Soll das in die erste
nützliche Fassung, oder reicht dir der Befund in Worten?

**Antwort (Auswahl):** „Ja, erste Fassung (Empfohlen)" — die farbige
Überdeckungszone kommt mit der EQ-Fläche in die erste nützliche Fassung.

**Folge:** Die Überdeckung wird nicht nur gemeldet, sondern gezeigt. Sie ist
Lieferumfang der ersten nützlichen Fassung der EQ-Fläche, kein Ausbau. Der
Datenweg liegt bei S23–25 (die Überdeckung als laufender, je Frequenzbereich
auflösbarer Wert mit Anzeigekadenz — der Befundtext bleibt unverändert
daneben), die farbige Zone selbst bei S31b. Die Zone zeigt nur, was der Befund
belegt hat, und setzt keine eigene Schwelle: sie ist keine zweite Wahrheit
neben dem Befundtext. Zeigt die Bildvorlage vom 25.08. keine solche Zone, wird
das als Lücke einzeln vorgelegt und nicht erfunden.

## U17 — fünf einzelne Handgriffe fürs Kurvenbild

**Frage:** Fünf Dinge, die vergleichbare EQs können und Nakama heute nirgends
vorsieht. Sie hängen nicht voneinander ab — jeder Punkt wurde einzeln gefragt
und einzeln beantwortet; eine Sammelantwort gibt es bewusst nicht, weil dein
Wort vom 23.08. die Überladen-Grenze „button für button" klären wollte.

**U17.1 — Aus dem Spektrum greifen.** Auf einen Berg im Frequenzbild fahren,
ihn herunterziehen, und daraus entsteht das Filterband.

> **Antwort (erste Reaktion, wörtlich):** „VERSTEHE Die frage nicht"
>
> **Antwort nach erneuter Erklärung (Auswahl):** „Ja"

Beide Schritte gehören zum Wortlaut: die Frage war beim ersten Mal nicht
verständlich gestellt. Erst als derselbe Handgriff gegenständlich erklärt war —
Berg anfahren, herunterziehen, Band entsteht daraus —, kam das Ja.

**U17.2 — Das Bild festhalten.** Das Spektrum hört auf zu fallen und sammelt
die Spitzenwerte.

> **Antwort (Auswahl):** „Ja"

**U17.3 — Die Anzeige einstellbar machen.** Wie hoch der Ausschlag geht, wie
fein und wie schnell das Bild ist, und ob die Höhen optisch angehoben
dargestellt werden.

> **Antwort (Auswahl):** „Ja"

**U17.4 — Tonnamen zeigen.** Eine Klaviatur unter dem Bild.

> **Antwort (Auswahl):** „Nein"

**U17.5 — Die Lautheit sichtbar machen.** Nakama misst sie bereits, angezeigt
wird sie nirgends.

> **Antwort (Auswahl):** „Ja, auf der Übersicht" — je Quelle ein Lautheitswert
> auf Gens erster Seite.

**Folge:** Die EQ-Fläche (S31b) liefert U17.1, U17.2 und U17.3. Bei U17.3 gilt
ausdrücklich: das ist Darstellung und nichts sonst — die Frequenzraster und
alles, was gemessen wird, bleiben unverändert, und keine Anzeigeeinstellung
darf einen Befund, eine Schwelle oder eine Kurve verschieben. U17.4 ist
abgelehnt: keine Tonnamen, keine Klaviatur — damit ist auch die
FL-Oktavschreibweise (FL nennt MIDI 60 C5) keine Anzeigefrage mehr, sondern
bleibt Innenwahrheit ohne sichtbare Fläche. U17.5 gehört nicht zur EQ-Fläche,
sondern zu S18–19: je Quelle ein Lautheitswert auf Gens erster Seite, mit der
Konfidenz, die derselbe Messer schon führt (Entwurf §0.4/§1.4 verlangt Frische,
Konfidenz und Degradation in jeder UI-Fassung). Für diesen einen Wert ist die
Anzeigefrage aus NAK-57 beantwortet; NAK-57 bleibt im Übrigen offen. Zeigt die
Bildvorlage vom 25.08. einen der drei angenommenen Handgriffe nicht, wird das
als Lücke einzeln vorgelegt und nicht erfunden.

## U15 — zweite Betriebsart ohne Phasendrehung (linearphasig)

**Frage:** Geplant ist heute ein einziger Filtertyp, der sofort reagiert und
keine Verzögerung einbaut — die Bauart, die fast jeder EQ als Grundeinstellung
hat. Pro-Q, dein genanntes Vorbild, bietet zusätzlich eine Betriebsart an, die
beim Filtern nichts im Zeitverlauf verschiebt. Sie klingt beim harten
Beschneiden sauberer, kostet aber eine feste Verzögerung (alles wird träger,
Live-Vorhören ungenauer) und kann vor sehr steilen Eingriffen ein leises
Vorecho erzeugen. Dass FL diese Verzögerung sauber herausrechnet, ist bei uns
nie nachgemessen worden. Soll Nakama diese zweite Betriebsart bekommen?

**Antwort (Auswahl):** „Nein, dauerhaft ohne" — Nakama bleibt bewusst
minimalphasig.

**Folge:** Kein zweiter Betriebsmodus — nicht in der ersten Fassung und nicht
als späterer Ausbau. Es wird dafür auch kein Name reserviert; die Tür bleibt
nicht angelehnt. Am Bau ändert sich dadurch nichts, und das ist der Punkt: die
an FL gemeldete Verzögerung bleibt null, der gespeicherte Zustand bekommt
keinen Phasenmodus-Wert, die Oberfläche keinen Umschalter. Die Karte hatte
S26–28, S28b und das Prüftor G6 als betroffen genannt — alle drei bleiben
unverändert. Der Abstand zum Vorbild ist damit bewusst und datiert: Pro-Q kann
das, Nakama nicht, und niemand muss die Stelle später neu aufrollen. Die in der
Karte genannte Abhängigkeit von der ungemessenen FL-Latenzverrechnung (Termin
A2, Karte U11) entfällt für den EQ; ob U11 aus anderen Gründen offen bleibt,
entscheidet U11 selbst.
