# Werkzeug — Messinstrumente, keine Entwürfe

Hier liegen Hilfsmittel, mit denen die Vorstellung des Users **gemessen**
wird. Was hier liegt, ist ausdrücklich kein Gestaltungsvorschlag: die
gezeigten Spezimen sind Materialproben (ein Kurvenausschnitt, eine
Bandzeile, eine Listenzeile, eine Befundkarte) — nie ein Layout.
Layoutentscheidungen gehören in `prototyp/` und in Phase 1 des
Arbeitsplans.

## stilbefragung.html

Visuelle Alternative zum verbalen Interview: statt zu fragen „welche
Gefühle soll es auslösen", werden Bilder zur Wahl gestellt. Erkennen ist
für Menschen erheblich präziser als Erfinden.

**Aufruf:** Doppelklick. Kein Server, keine Build-Kette, keine
Netzverbindung. Alle Schriften sind lokal installiert.

**Prinzipien, die im Code stehen und nicht verhandelbar sind:**

1. **Eine Variable pro Runde.** Zwischen zwei Karten ändert sich genau ein
   Token. Nur so ist die Antwort einem Wert zuordenbar statt einer Stimmung.
2. **Kein Variantenname.** Ein Label wie „technisch" würde die Wahl
   vorwegnehmen. Die Karten heißen 1, 2, 3.
3. **Neutrales Mittelgrau als Umgebung.** Ein dunkles Werkzeug-Chrome
   ließe dunkle Spezimen gewinnen, ein helles die hellen.
4. **Echter Nakama-Inhalt**, Demo-Daten sichtbar gekennzeichnet — und die
   Kennzeichnung gehört zum Werkzeug, nicht zum Spezimen.
5. **Drei Rahmungen statt einer:** „4 Stunden am Stück", eine echte
   Suchaufgabe, und „einem Freund zeigen". Reines Hübsch-Wählen
   optimiert auf ein Poster, nicht auf ein Instrument.
6. **Ablehnung wird erfasst** („alle falsch — und zwar weil …"). Bei
   Abneigung sind Menschen präziser als bei Zuneigung; der Designvertrag
   braucht Antireferenzen ohnehin.
7. **Gegenprobe.** Zwei Entscheidungen kommen später gedreht und anders
   gerahmt wieder. Weicht die Antwort ab, entscheidet der Rahmen mit —
   das ist ein Befund, kein Fehler.

**Ergebnis:** ein JSON-Block mit dem fertigen Tokensatz, allen
Einzelentscheidungen samt Antwortzeit, den Ablehnungstexten und der
Gegenprobe. Der geht in den Chat und von dort als Abnahme nach
`abnahmen/`.
