# Nakama, wenn es fertig ist

<!-- Pflege (für Sessions, im Plan-Tab unsichtbar): Dieser Text ist autoriert, seine
Aktualität gerechnet — `py -3.13 tools/plan/zielbild.py pruefen` nennt jede neue Karte,
Abnahme, Produktzeile im Register und Planänderung seit dem letzten Abgleich. Eingearbeitet
wird im selben Änderungssatz wie die Entscheidung, danach `zielbild.py abgleichen`
(docs/plan/LIES-MICH.md, Abschnitt „Das Zielbild"). Schreibweise: Alltagssprache für den
User als Projektleiter; nur Entschiedenes und der gültige Plan, nie Vorschläge. Eine
Planmarke steht als eigene Zeile unter einer Überschrift oder am Ende einer Listenzeile. -->

> So würde Nakama für dich als Musiker arbeiten, wenn der heutige Plan mit allen bisher
> getroffenen Entscheidungen fertig gebaut wäre — ein Bild vom Ziel, kein Baustand. Die
> Zeichen zeigen, wie weit die Technik einer Funktion gebaut und geprüft ist:
> ✓ fertig · ◐ läuft · ○ offen.

## Auf einen Blick

Nakama findet in deinem FL-Mix die Spur, die ein Problem wahrscheinlich verursacht, lässt
dich den Befund hören, schlägt den kleinsten sinnvollen Eingriff vor und führt ihn — nur
wenn du ihn bestätigst — mit dem eigenen EQ genau auf dieser Spur aus; danach prüft es
dieselbe Stelle noch einmal.

- **Zwei Plugins:** Nakama Gen auf dem Master mit zwei Oberflächen (Übersicht und
  EQ-Zentrale), Nakama Probeeq auf den Instrumentenbussen.
- **Ablauf:** einsetzen → abspielen → Befund lesen → hören → eingreifen → prüfen.
- **Grundgesetz:** nichts passiert ungefragt; ausgeschaltet bleibt dein Klang bitgenau
  unverändert.
- **Die zwölf Kernfunktionen:**
  - Mix-Landkarte <!-- plan: P3 -->
  - Ursachenfinder <!-- plan: P4–P5 -->
  - Musikalische Prioritäten <!-- plan: P4–P5 -->
  - PRE/POST-Kettenprüfung <!-- plan: P4–P5 -->
  - Dynamik und Headroom <!-- plan: P4–P5 -->
  - Stereo und Phase <!-- plan: P4–P5 -->
  - Hörbarer Beweis <!-- plan: P4–P5 · P6–P7 -->
  - Konkrete Bus-Empfehlung <!-- plan: P4–P5 · P6–P7 -->
  - Vorher/Nachher-Prüfung <!-- plan: P4–P5 · P6–P7 -->
  - EQ auf jedem Bus, bedient aus Gen <!-- plan: P6–P7 -->
  - Dynamisches Entmaskieren, hängt an Termin A2 <!-- plan: P8–P9 -->
  - Zentraler Mix-Assistent <!-- plan: P4–P5 -->
- **Bedienoberfläche:** kommt für alle Funktionen gesammelt am Ende des Plans; bis dahin
  läuft die Technik auf einer provisorischen Oberfläche <!-- plan: S31b -->
- **Was noch deine Entscheidung braucht,** steht am Ende unter „Noch offen".

## Ein Beispiel

Statt „im Master stimmt bei 900 Hz etwas nicht" sagt Nakama: „Im Refrain verdeckt das
Klavier den Chor zwischen 700 Hz und 1,2 kHz. Hör es dir an, probier eine kleine Absenkung
nur auf dem Klavierbus und prüf danach dieselbe Stelle."

## Die zwei Plugins

- **Nakama Gen** liegt auf dem Master und ist die Zentrale. Sie hat zwei Oberflächen, die
  du oben im Kopf jederzeit wechselst:
  - die **Übersicht** (Arbeitsname): alle Quellen deines Mixes, das Frequenzbild, die
    Befunde und der Advisor;
  - die **EQ-Zentrale** (Arbeitsname): ein vollwertiger EQ für den Master und die
    Fernbedienung für den EQ jeder Probeeq — alles an einem Ort.
- **Nakama Probeeq** liegt auf den Instrumenten- und Gruppenbussen (Klavier, Chor, Drums,
  Bass …). Im Regelfall sind es eine Handvoll; Gen nimmt bis zu 20 an. Probeeq misst, was
  auf ihrem Bus passiert, und liefert es an Gen. Schaltest du ihren EQ zu, führt sie auf
  diesem Bus aus, was du in Gen einstellst. Eine Probeeq misst und wirkt nur dort, wo sie
  steckt.
- Im eigenen Fenster zeigt Probeeq nur eine flache Kachel: Name und Bus, Verbindung zu Gen,
  EQ an oder aus, Bypass und Mix — diese drei kannst du dort auch selbst schalten, etwa wenn
  Gen nicht läuft. Darunter steht bei eingeschaltetem EQ die laufende Änderung, sonst was
  sie gerade misst. Einen zweiten vollständigen EQ-Editor hat sie bewusst nicht.
- Ein unsichtbarer Hintergrunddienst verbindet beide. Du startest nichts von Hand und
  brauchst kein Terminal.
- Im Plugin-Menü heißen sie „Nakama Gen" und „Nakama Probeeq", das Paket heißt „Nakama
  Studio"; der alte Name „EQ-Copilot" verschwindet vor der Auslieferung. Die Oberfläche
  ist englisch.

## Ein Arbeitsdurchgang

1. **Einsetzen.** Gen auf den Master, Probeeq auf die wichtigen Busse. Beim ersten Mal
   öffnest du Gen und bestimmst sie im Fenster als Zentrale; den Hintergrunddienst startet
   sie dann selbst. Je Projekt führt genau eine Gen. Nichts verändert deinen Klang: jede
   Verarbeitung ist aus, bis du sie einschaltest.
2. **Quellen erkennen.** Gen findet die Probeeq-Instanzen dieses Projekts selbst. Jede
   Quelle zeigt ihren Namen, ob sie Signal hat, wie frisch ihre Messung ist, ihre Lautheit
   und wie viele Befunde offen sind. Name und Reihenfolge kommen aus deinem FL-Mixer, soweit
   FL sie herausgibt (Karte U26); sonst trägt jede Sonde den Namen, den du ihr gibst.
3. **Abspielen.** Du spielst eine Passage. Die Mix-Landkarte füllt sich: wer wo im
   Frequenzbild Energie trägt, wie laut, wie dynamisch, wie breit.
4. **Befund lesen.** Der Advisor nennt je Befund genau drei Dinge: wahrscheinliche Ursache,
   kleinster Test, worauf du hören sollst. Nur ein sicherer Befund lässt sich weiterschicken
   (READY TO SEND); ein unsicherer sagt „mehr Daten nötig" (MORE DATA), ein veralteter
   „veraltet" (STALE). Die Belegstelle ist im Frequenzbild markiert; wo sich zwei Spuren
   zudecken, ist die Zone farbig.
5. **Hören.** Am Befund hältst du die Taste gedrückt und hörst die Änderung; loslassen heißt
   weg (HOLD TO AUDITION). Mit SEND DRAFT schickst du den Vorschlag in die EQ-Zentrale,
   direkt auf die betroffene Spur.
6. **Eingreifen.** In jeder EQ-Ansicht gilt die Drei-Stufen-Geste: halten zum Hören; der
   erste Klick probiert die Änderung zehn Sekunden lang aus und springt dann von selbst
   zurück; der zweite Klick übernimmt sie. Mit dem Mausrad oder per Klick schaltest du durch
   die Sonden, setzt Filter und springst zum Master zurück. Eine zweite EQ-Kurve lässt sich
   farbig unterscheidbar ins selbe Bild legen.
7. **Prüfen.** Du spielst dieselbe Passage noch einmal; Nakama misst sie erneut und
   vergleicht lautheitsangeglichen. Du entscheidest: behalten, verwerfen oder weiter prüfen.
   Erst dann führt der Advisor zum nächsten Schritt; es läuft immer nur ein Klangversuch.

## Die zwölf Kernfunktionen

Bestätigter Kernumfang, deine Entscheidung vom 15.09.2026: „passt so mit den 12".

### 1 · Mix-Landkarte
<!-- plan: P3 -->

Du siehst deinen Mix als Gruppe benannter Quellen statt als eine Masterkurve: welche Busse
gerade spielen, wo jede Quelle im Frequenzbild Energie trägt, wie laut sie ist (mit
ehrlicher Unsicherheit) und welche Sonde fehlt, schweigt oder veraltet ist. Beispiel: „Die
Tiefmitten kommen überwiegend vom Klavierbus; der Chor trägt leiser bei, die Drums sind in
dieser Passage nicht beteiligt." Hinter Sättigung oder Limiter auf dem Master ist die Summe
keine einfache Addition — Nakama zeigt dann Beiträge und Wahrscheinlichkeiten, keine
erfundene Gewissheit.

### 2 · Ursachenfinder
<!-- plan: P4–P5 -->

Führt einen Masterbefund auf den wahrscheinlichen Bus oder die Effektkette zurück: eine
Quelle mit Resonanz, zwei Quellen im selben Bereich, eine Kette, die das Problem erst
erzeugt, Spitzen von einem Drumbus, Breitenverlust durch eine Kette — oder ehrlich „die
Daten reichen noch nicht". Du siehst die wahrscheinliche Ursache; der Beleg ist die
markierte Zone im Frequenzbild, andere mögliche Ursachen stehen als weitere Befunde in der
Liste.

### 4 · Musikalische Prioritäten
<!-- plan: P4–P5 -->

Du sagst Nakama, was eine Quelle in der Passage soll — genau fünf Rollen: führt, trägt,
begleitet, geschützt, bewusst verschmolzen. Kurze Anschläge schützt du über eine
Schutzangabe, Hall und Atmosphäre dürfen über eine Verschmelzungs-Erlaubnis verschmelzen.
Grundregel: deine Absicht schlägt die statistisch „sauberere" Lösung. Ohne Rolle misst
Nakama trotzdem, formuliert aber vorsichtiger.

### 5 · PRE/POST-Kettenprüfung
<!-- plan: P4–P5 -->

Zwei Probeeq rahmen eine Effektkette ein, eine davor, eine dahinter; in Gen markierst du
sie als Paar. Nakama beschreibt, was die Kette wirklich tut: welche Frequenzen sie hebt oder
senkt, ob sie nur lauter macht, Anschläge stärker drückt als gedacht, das Signal verengt
oder die Mono-Verträglichkeit verschlechtert — und ob sie das ursprüngliche Problem löst
oder nur verschiebt. Arbeiten fremde Plugins mit Verzögerung oder stark nichtlinear, heißt
das Ergebnis „wahrscheinliche Wirkung" statt Behauptung.

### 9 · Dynamik und Headroom
<!-- plan: P4–P5 -->

Zeigt, wo Dynamik entsteht und verloren geht und welcher Bus den Master in den Limiter
treibt — etwa „nicht der Bass, sondern drei einzelne Drumspitzen". Dazu Lautheit (auch
kurzfristig), Spitzen und True Peak je Bus und Master. Ohne markierte Passage blickt die
Headroom-Verteilung auf ein gleitendes Zeitfenster, dessen Länge dabeisteht. Nakama
optimiert nie auf maximale Lautheit; erhaltene Bewegung ist ein Ziel, kein Fehler.

### 10 · Stereo und Phase
<!-- plan: P4–P5 -->

Breite, Korrelation und Mono-Verlust je Quelle und im Gesamtmix, zu breiter Tiefbass,
Seitenenergie, die nur aus einem Effektbus kommt — und welcher Bus oder welche Kette das
verursacht. „Breiter" ist nicht automatisch „besser": bewertet werden Stabilität,
Mono-Verträglichkeit und musikalische Rolle zusammen.

### 12 · Hörbarer Beweis
<!-- plan: P4–P5 · P6–P7 -->

Du hörst einen Befund, bevor du ihm glaubst. Ohne zugeschalteten EQ hebt Gen den
auffälligen Bereich der Summe hörbar hervor — allein oder pulsierend, nur beim Abspielen
und nie im Export — oder sagt dir, welche Spur du kurz solo oder im Bypass hören sollst.
Mit zugeschaltetem EQ hörst du den Eingriff direkt auf dem Bus: im lautheitsangeglichenen
A/B-Vergleich, damit „lauter" nicht als „besser" durchgeht, nur im betroffenen Bereich oder
in Mono. Jede Hörprobe ist flüchtig: Loslassen, Ablauf, Stopp oder Verbindungsverlust
beendet sie sanft. Das hörbare Delta — nur das, was sich ändert — hängt an Termin A2
(Karte U11).

### 13 · Konkrete Bus-Empfehlung
<!-- plan: P4–P5 · P6–P7 -->

Statt „Tiefmitten prüfen" bekommst du einen kleinen, ausführbaren Versuch am wahrscheinlich
richtigen Ort, etwa: Klavierbus, breite Absenkung um 850 Hz, Start −1,5 dB, aufhören, sobald
das Klavier hohl klingt. Immer zuerst der kleinste plausible Eingriff, nie fünf Busse auf
einmal. Du führst ihn selbst in deinem gewohnten Werkzeug aus oder als Entwurf im eigenen
EQ; Vorschläge sind im EQ frei nachjustierbar. Soll eine Absenkung nur wirken, während eine
andere Spur spielt, ist das Punkt 17.

### 14 · Vorher/Nachher-Prüfung
<!-- plan: P4–P5 · P6–P7 -->

Nach einer Änderung spielst du dieselbe Passage noch einmal, und Nakama vergleicht: was sich
am Bus und am Master verändert hat, welche Nebenwirkungen bei Dynamik, Breite und Lautheit
auftreten, dazu der Hörvergleich bei angeglichener Lautheit. Es gibt ein Blind-Urteil: erst
nach deinem Urteil deckt Nakama auf, welche Fassung du gehört hast (ob es Standard ist oder
zugeschaltet wird, ist geparkt, Karte U30). Ist das Material nicht vergleichbar, warnt oder
sperrt es. Nakama sagt nie „jetzt objektiv besser" — das Urteil ist deins.

### 16 · EQ auf jedem Bus, bedient aus Gen
<!-- plan: P6–P7 -->

Jede Probeeq trägt einen eigenen EQ; bedient wird er zentral in Gens EQ-Zentrale. Gen hat
zusätzlich einen eigenen, vollwertigen Master-EQ. Nakama steuert nie fremde Plugins oder
FL-Fader — für die bleibt eine Empfehlung eine verständliche Anleitung. Was der EQ kann,
steht unten unter „Der EQ".

### 17 · Dynamisches Entmaskieren
<!-- plan: P8–P9 -->

Eine Hintergrundspur macht nur dann und nur dort Platz, wenn eine priorisierte Spur ihn
braucht: das Klavier gibt zwischen 700 Hz und 1,2 kHz etwas nach, aber nur, während der
Chor singt. Dafür leitest du in FL die führende Spur als Sidechain in die Probeeq der
nachgebenden; jede Spur weicht höchstens einer anderen. Du bestätigst die Beziehung — wer
führt, wer weicht — ausdrücklich. Die Stärke ist begrenzt und sichtbar (ab Werk höchstens
1,5 dB; die genaue Obergrenze ist noch zu klären), eine automatische Anhebung gibt es nie.
Fällt das Steuersignal weg, fährt die Absenkung sanft auf null; der übrige EQ bleibt. Kein
globaler Auto-Ducker über alle Spuren. **Hängt an einer FL-Messung:** ob FLs Nebenwege mit
Latenzausgleich sicher arbeiten, klärt Termin A2 (Karte U11); ohne sie beginnt dieser Bau
nicht. Bis dahin bleibt es beim Hinweis auf die Überdeckung und einem festen, statischen
Vorschlag; fällt die Messung endgültig aus, entscheidest du, ob Nakama ohne diese Funktion
erscheint.

### 18 · Zentraler Mix-Assistent (Advisor)
<!-- plan: P4–P5 -->

Führt alle Befunde zu einer Arbeitsreihenfolge zusammen: wenige nächste Schritte, einer
nach dem anderen. Er unterscheidet zwischen EQ-, Dynamik-, Stereo-, Pegel- und „lieber
nichts tun"-Lösung, verlangt erst einen Hörbeweis, wenn er unsicher ist, führt nach dem
Eingriff zum Vergleich zurück und lässt sich verlassen und an derselben Stelle fortsetzen.
Er arbeitet nach festen Regeln und nachvollziehbar — keine KI-Schicht. Er spricht auch auf
leisem Material wie Intro oder Ausklang.

## Der EQ
<!-- plan: P6–P7 · S31b -->

- **Bänder:** acht je EQ; Glocke, Tief- und Hochregal, Notch, Hoch- und Tiefpass;
  20 Hz bis 20 kHz, Güte 0,15 bis 24, ±12 dB. Jedes Band wahlweise auf Stereo, links,
  rechts, Mitte oder Seite.
- **Dynamisch:** jedes Band kann dynamisch arbeiten, mit Schwelle, Bereich, Attack, Hold
  und Release; die Schwelle misst je Band Spitze oder Durchschnitt (ein neues Band startet
  auf Durchschnitt). Ein dynamisches Band bewegt sich im Bild sichtbar mit der tatsächlichen
  Absenkung oder Anhebung.
- **Global je EQ:** Ein- und Ausgangspegel, Breite, Mono-Bass, Bypass und
  AUTO-Lautstärkeausgleich (Schalter, standardmäßig aus, mit Obergrenze). Mix (Dry/Wet) gibt
  es für die ganze App und je Spur; ein Klick am Regler wählt, welchen du einstellst — wie
  beide zusammenwirken, ist noch offen. Die globale Sektion ist eingeklappt und nennt nur,
  was vom Standard abweicht.
- **Bedienen wie bei Pro-Q:** Doppelklick ins Bild legt ein Band an; ein kleines Panel am
  Bandpunkt zeigt Typ, Frequenz, Gain, Güte und Dynamik; Mausrad über einem Wert ändert ihn
  (mit Shift fein), Strg-Klick setzt ihn zurück. Ein Band lässt sich direkt aus dem Spektrum
  ziehen: Berg anfahren, herunterziehen.
- **Spektrum:** live nur für die gerade bearbeitete Spur, einfrierbar (Spitzen sammeln),
  Anzeige einstellbar (Höhe, Feinheit, Geschwindigkeit, Neigung). Die Überdeckungszone
  zweier Spuren liegt farbig im Bild.
- **Schutzzonen:** ein Frequenzbereich als harte Sperre für alles, was du in Nakama
  bedienst, auch für deine eigenen Bänder. Gen darf eine Zone vorschlagen, anlegen tust du
  sie.
- **Voreinstellungen und Kopie:** mitgelieferte und eigene Voreinstellungen; eine fertige
  Kurve lässt sich von einer Sonde auf eine andere kopieren. Rückgängig und Wiederholen
  gelten je Spur.
- **Automation:** EQ-Werte lassen sich in FL automatisieren. Läuft eine Automation, hat sie
  Vorrang, und Gen zeigt das, statt still zu überschreiben. Während FL aufnimmt, gibt es aus
  Gen keine Hörprobe und kein Übernehmen. Im Export klingt die Automation genauso wie beim
  Abhören.
- **Klang und Sicherheit:** minimalphasig und ohne Latenz; ausgeschaltet bitgenau
  unverändert. Große Sprünge eines Bands werden kurz überblendet. Bricht die Verbindung zu
  Gen ab, bleibt der bestätigte Klang stehen; nur eine laufende Hörprobe endet.
- **Bewusst nicht:** kein linearphasiger Modus (dauerhaft), keine Klaviatur mit Tonnamen,
  kein Nachbilden des Klangs einer Referenzaufnahme (bleibt für später), keine Steuerung von
  Lautstärke oder Panorama anderer Spuren.

## Installation und Projektsicherheit
<!-- plan: S34–35 -->

- Ein Installer bringt beide Plugins samt Hintergrunddienst; Update, Reparatur und Rückweg
  auf die vorige Fassung gehören dazu.
- Alte FL-Projekte laden weiter. Gespeicherte Einstellungen gehen nie verloren — auch ein
  unbekannter, neuerer Stand bleibt unverändert erhalten; der EQ läuft dann neutral und sagt
  warum. Deshalb auf PC und Laptop dieselbe Fassung installieren.
- Ein bestätigter EQ-Klang rendert auch ohne laufenden Hintergrunddienst.
- Deine lokalen Messverläufe kannst du exportieren oder löschen, getrennt vom Projekt.

## Was Nakama nie tut

- **Nichts Ungefragtes:** es verarbeitet nur, was du einschaltest; ausgeschaltet läuft der
  Klang bitgenau unverändert und ohne Verzögerung durch.
- **Keine KI-Schicht:** der Advisor schlägt nur vor, er greift nie selbst ein.
- Kein Auto-Mixer, keine Einheitskurve für jeden Mix, kein Loudness-Maximierer, kein
  „objektiv besser".
- Kein Zugriff auf fremde Plugins, FL-Fader, Lautstärke, Panorama oder Mixer-Routing.
- **Keine toten Knöpfe:** jedes sichtbare Element tut etwas oder meldet ehrlich einen
  Zustand.
- Dein Audio bleibt auf deinem Rechner. Wird es eng, verwirft Nakama Messdaten, nie Audio.
- Sonden aus einem anderen FL-Projekt landen nie still in deiner Sitzung.

## Ehrliche Grenzen

- Ohne Probeeq auf einem Bus kann Nakama nicht sicher sagen, welches Instrument einen
  Masterbefund verursacht; Gen allein liefert eine ehrliche Masterdiagnose.
- Hinter nichtlinearer Masterbearbeitung lässt sich kein Quellenbeitrag exakt
  zurückrechnen.
- Ob eine Überdeckung künstlerisch gewollt ist, kann keine Messung beweisen — deshalb gibt
  es die Rollen.
- Wenn Arrangement, Klangwahl oder Spiel das eigentliche Problem sind, hilft kein EQ;
  Nakama garantiert keinen guten Mix.

## Im Detail festgelegt

- Gens Fenster hat genau eine Größe (950 × 538) und lässt sich per UI-Skalierung
  vergrößern; eine kleinere Fassung ist vertagt (01.09.2026).
- Messlücken-Zähler und ähnliche Technikdiagnose bekommst du nicht zu sehen — ein
  Audiowerkzeug, kein Entwicklerwerkzeug (U27).
- Kurzzeit-Lautheit bleibt nach einer Störung kurz leer, statt einen alten Wert zu zeigen
  (U42).
- FLs eigener Bypass-Knopf am Mixer-Slot blendet weich, Probeeq misst währenddessen weiter,
  und nach dem Loslassen laufen die Filter ohne Einschwingen durch (U48, U58; Bau in NAK-312).
- Projekte unter 44,1 kHz: Messung ja, EQ nein — er bleibt neutral und sagt warum (U47).
- Setzt FL die Audioverarbeitung aus (etwa beim Wechsel der Audioeinstellungen, nicht beim
  normalen Stopp), verstummt eine laufende Hörmarkierung sauber und beginnt beim
  Weiterlaufen von vorn (U56).
- Liegen mehrere Gen im Projekt, führt genau eine; die Führung wechselt nur, wenn du es
  bestätigst. Stellst du eine Gen-Instanz auf Messpunkt und wieder zurück, bleibt alles
  erhalten (U49).
- Mehr als 20 Probeeq: die 21. meldet ehrlich, dass die Liste voll ist, und misst erst mit,
  wenn Platz ist; gespeicherte Quellen werden dafür nie gelöscht (U51).

## Noch offen

Diese Punkte brauchen deine Antwort oder eine Messung in FL. Die offenen Karten stellt dir
der Skill `/fragen`; die geparkten erst, wenn du den Designteil wieder öffnest. Solange du
nicht antwortest, gilt jeweils der genannte Weg.

- **Termin A2 (U11):** eine Messung in FL mit einem Plugin, das Verzögerung meldet. Ohne
  sie beginnt der Bau des dynamischen Entmaskierens nicht, und es gibt kein hörbares Delta;
  ob Nakama ohne diese Funktionen erscheint, entscheidest du ausdrücklich.
- **Kurztermin C (U13):** eine Zeit-Annahme des Messkerns im echten FL ablesen; blockiert
  nichts.
- **Busnamen aus FL (U26):** ob FL Namen und Reihenfolge der Mixer-Busse herausgibt. Bis
  dahin trägt jede Sonde den Namen, den du ihr gibst.
- **Geklonte Spur (U50):** wer bleibt die bekannte Quelle, wenn du eine Spur samt Probeeq
  klonst? Ohne Antwort: die zuerst verbundene; der Klon wird eine neue Quelle.
- **Lautheit nach einer ungültigen Messung (U52):** ausblenden, bis wieder gültige Daten da
  sind, oder den alten Wert mit Alter zeigen? Ohne Antwort: ausblenden.
- **Getippter Name beim Schließen (U57):** ein noch nicht bestätigter Name im
  Messpunkt-Feld verfällt, wenn du das Fenster schließt. Ohne Antwort bleibt es so.
- **Geparkt, bis die Technik fertig ist (Designteil, 12.09.2026):** das Aussehen beider
  Gen-Oberflächen und der Probeeq-Kachel, die Namen der beiden Gen-Oberflächen (U23) und
  die Bedienfragen vor dem Oberflächenbau: Passage markieren (U28), laufender Versuch (U29),
  Blind-Urteil als Standard (U30), Reihenfolge der Vergleichsergebnisse (U31),
  PRE/POST-Paar in der Liste (U32), Passagennamen (U33), Befund ohne markierte Passage
  (U34), wie viele Befunde gleichzeitig (U35), zurückgenommene Befunde (U36), Advisor holt
  Befunde in den Fokus (U37).
- **Vertagt:** die Höhenkorrektur des digitalen EQ (U46, eigene Karte, wenn der Kern fertig
  ist) und die kleinere Fenstergröße.

## Später, nicht im Kern

Nach dem Kern liegen acht Ideen als Roadmap bereit: eine Überdeckungsanalyse über alle
Spurpaare, automatische Abschnittserkennung (Intro, Strophe, Refrain), Arrangement-Beratung,
ein Tiefbass-Manager, Referenz-Abgleich je Rolle, ein Verlauf über Projektstände, ein eng
begrenzter Autopilot (nur mit deiner ausdrücklichen Entscheidung) und das Lernen deiner
Vorlieben.

<!-- ZIELBILD-ABGLEICH — geschrieben von `py -3.13 tools/plan/zielbild.py abgleichen`, nie von Hand.
stand: 2026-09-23 09:09
karten-beantwortet: U10=270797 U12=5adee9 U14=ea3de5 U15=74821d U16=661dec U17.1=9d0438 U17.2=02d885 U17.3=02d885 U17.4=beefc4 U17.5=c7a6e3 U18=6292e7 U19=1af907 U2=23433a U2.0=99b9d1 U2.1=678ed3 U2.10=3d74d7 U2.11=602c50 U2.12=602c50 U2.13=602c50 U2.14=c102c7 U2.15=b19ebb U2.16=2b6c01 U2.17=2a3dc9 U2.2=678ed3 U2.3=a9f0cd U2.4=d319c0 U2.5=9dcd3d U2.6=825118 U2.7=f70e04 U2.8=8383eb U2.9=5ee95d U20=ab54ab U21=3c9eab U22=79104d U24=5a6eb4 U25=48371a U27=9b4f9a U38=a8f063 U39=745483 U40=e91456 U41=7f6f2a U42=e0eb1d U43=8394ae U44=234869 U45=046708 U46=9a5e23 U47=0286f0 U48=604d90 U49=9762b7 U5=f71092 U51=b9be76 U53=c6fb14 U54=ec5b6c U55=ced656 U56=3ef749 U58=acbea1 U6=23433a U6.1=28ce08 U6.10=99b9d1 U6.2=2a8a34 U6.3=2a8a34 U6.4=9350ae U6.5=a5d9ab U6.6=9abfbd U6.7=574cdc U6.8=8abbc0 U6.9=a2145e U7=56c6da U8=79a4a5 U9=23433a U9.1=96cb06 U9.2=18d84a U9.3=ada30f U9.4=306119 U9.5=9c0088
karten-offen: U11 U13 U26 U50 U52 U57
karten-geparkt: U23 U28 U29 U30 U31 U32 U33 U34 U35 U36 U37
abnahmen: 0928f1 0c52de 107b19 11ce7d 12da63 146c14 162dc0 17b40b 1fd475 281f0c 2c0763 2c17d8 2ca56d 2df88d 2ffda7 3075a8 3183bf 322bd7 3c2ae2 3d6d83 3e3f05 40101e 4375dd 44d299 45aa05 48e998 49e0ee 4cd564 56cdff 5eaca5 6336d1 69197c 6b3715 6bf037 6c9196 6edeeb 6f4b71 726f52 7e353d 8438e4 851e96 876993 8ba768 8d06b5 9af315 9ec278 a2a1ce a32fc2 a717fd ac02f8 b2aceb b45329 ba820b bb73e5 bcaf8d bfa7e5 c01fab c59c1c ce4242 d41a3f d4960d d4de1e dba4c9 df38a4 e842b5 e8919c f2635a f67e43 f86c28 f8d069 f9fdee
register-produkt: NAK-111=179ff6 NAK-122=ec3396 NAK-179=a6b56f NAK-306=e9be8e NAK-331=afa1a0 NAK-332=03a7d9 NAK-341=eac434 NAK-39=707c3b NAK-44=b95a79 NAK-56=69e61e
planschritte: G0=3c6c4a G1=ccbdf5 G2=3de1eb G3=8d76ab G4=02c378 G5=b6d4d1 G6=6221ef G7=c138a2 G8=54f587 G9=7f7123 PR1=f7be81 PR2=29b00f S0=2b1293 S1=3f4715 S10–11=2271a8 S12–13=4a1b75 S14–15=56a6b4 S16–17=cf677a S18–19=4d754d S19b=b49639 S2=1d7603 S20–22=528685 S23–25=2b623b S25b=0bee3a S25c=69d275 S25d=3bc1c1 S25e=95856f S25f=0d9ba6 S25g=b4a2b4 S25h=30e8ec S25i=eae234 S25j=2e3c71 S25k=e4e92a S25l=28ac5b S25m=3e9457 S25n=bb6661 S25o=83afad S25p=4721e6 S26–28=acfad9 S28b=ec5221 S29–31=467ebc S3=dab03b S31b=1a55ff S31c=2c7823 S32–33=642430 S34–35=aca31e S35b=c7df0b S3b=bed185 S4=3c6fe8 S5=3dd9a8 S6=6413ed S7=fa71e8 S8=176639 S9=824636 S9b=6d3982 Termin%20A=9818b7 Termin%20B=1d196f
-->
