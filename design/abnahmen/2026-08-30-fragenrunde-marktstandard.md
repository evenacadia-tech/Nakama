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
