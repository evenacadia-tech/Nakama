# EQ-Copilot — Probefahrt in FL Studio (M0 ≈ 10 min · M1 ≈ 10 min · M2 ≈ 10 min · M3-Kern ≈ 5 min · M3a ≈ 5 min)

Das Plugin ist der **Messfühler und Berater** des EQ-Copilot. Es verändert
nichts am Klang (bewiesen per Nulltest). Seit M1 zeigt es die echte
Spektralkurve; seit M2 melden alle Instanzen an die Hub-App. **Seit M3-Kern
sagt es dir, was an deinem Graphen schlecht ist — mit konkreten
Umsetzungsempfehlungen** (Befundkarten hinter dem Hinweis-Knopf). Kein Setup
nötig: laden, Musik spielen, Befund lesen.

> **Aktueller Build: M3a + FPS-Fix (Nacht 15.08.)** — das fertige Bundle liegt
> in `eq-copilot\install\EQ-Copilot.vst3` (SHA siehe
> `install\install-result.txt` nach dem Einspielen; Skript prüft ihn selbst).
> Die Systemkopie trägt noch den Material-Kit-Stand `8FB5EE05…` — einmal
> `install\Install-EQ-Copilot.ps1` als Administrator ausführen (oder §1 per
> Explorer). **Was der neue Stand ändert:** Der Graph läuft flüssig (~20
> Bilder-Datenrate statt 4 — das war dein „extrem wenig FPS") und ruht im
> Stillstand komplett; Resonanzkarten nennen die **Note in FL-Zählung** (116 Hz
> = A#3) und ob der Pegel dort „ruhig steht" oder „in Wellen kommt";
> Zonen-Karten sagen ehrlich, **in wie viel % der Musikzeit** das Problem da
> war; oben rechts im Glas erscheint **„KURVE STEHT"**, sobald sich die Kurve
> ~10 s lang kaum noch bewegt (ab da ist sie belastbar lesbar).
> **Sehen:** warme helle Metallfront, schwarze Kopfleiste und großes
> Analyseglas; links LUFS Short, rechts True Peak; darunter echte Tasten und
> getrennte Statuszellen. Keine Fremd-Assets, keine Fake-Potis. Das Fenster
> behält beim Ziehen immer 750:520. Alle Audio-/Analysefunktionen sind dieselben.
> Darunter steckt weiterhin der Messstand von Änderungssatz 1: breites
> Side-/Antiphase-Material (Chöre, Hall) zählt voll statt als Stille, kaputte
> Zahlen (NaN) vergiften nichts mehr, oberhalb der messbaren Grenze zeigt die
> Kurve ehrlich eine Lücke.
> (Der M2-Kasten von früher — App neu starten für den 🎚-Knopf — gilt
> weiterhin, falls noch nicht geschehen.)

## 1 · Einmalig: Plugin installieren

**Tu:** Kopiere den Ordner `eq-copilot\install\EQ-Copilot.vst3`
nach `C:\Program Files\Common Files\VST3\` (Explorer fragt einmal nach
Administrator-Erlaubnis — bestätigen).
**Warum:** Das ist der Ordner, den FL beim Plugin-Suchlauf immer ansieht.
**Dann:** FL Studio → Add-Menü → „Mehr Plugins…" → Suchlauf (Find plugins).
**Sehen:** „EQ-Copilot" erscheint unter Effekte (Hersteller evenacadia).

## 2 · Erster Klangtest — hörst du GENAU nichts?

**Tu:** Lade EQ-Copilot als **letzten** Effekt auf den Master eines vollen
Projekts, spiele den Song, schalte das Plugin mehrfach an/aus (Bypass).
**Warum:** Ein Messfühler darf den Klang nicht einmal um ein Haar verändern.
**Hören:** Kein Unterschied — auch keiner in der Lautstärke. Wenn doch etwas
anders klingt, ist das ein Fehler: bitte notieren, bei welcher Stelle.

## 3 · Fenster und Pegel

**Tu:** Plugin-Fenster öffnen, an der Ecke größer/kleiner ziehen, Song spielen.
**Sehen:** Das Gitter (20 Hz–20 kHz) skaliert sauber mit; rechts bewegen sich
zwei Pegelbalken L/R im Takt der Musik; unten steht der Status als Wort.
**Auch prüfen:** Windows-Skalierung (125 %/150 %): bleibt alles scharf und
bedienbar?

## 4 · Verbindung zur App

**Tu:** Starte die Plugin-Hub-App (`pnpm tauri dev` oder die installierte App).
**Sehen:** Im Plugin springt der Statuspunkt auf Grün: „App verbunden".
**Dann:** App beenden. **Sehen:** Punkt wird rot, Text sagt „App nicht
erreichbar — misst lokal weiter (Versuch n)", und die Zahl n zählt langsam
hoch. App wieder starten → nach wenigen Sekunden wieder Grün, ohne dass du
etwas tust. **Warum:** Der Fühler darf nie von der App abhängen.

## 5 · Rolle und Speichern

**Tu:** Setze im Plugin Rolle „POST (Paar)", Label „PIANO-Bus POST", Paar-ID
„paar-a". Projekt speichern, FL komplett schließen, Projekt neu öffnen.
**Sehen:** Alle drei Angaben stehen wieder da.
**Warum:** Die Bindung ist die Identität des Messpunkts — sie muss jeden
Neustart überleben.

## 6 · Stresstest (für den M0-Nachweis)

**Tu:** Lege das Plugin auf ~16 Spuren/Busse gleichzeitig, spiele den Song.
**Hören/Sehen:** keine Aussetzer/Knackser; FLs CPU-Anzeige bleibt unauffällig;
in der Statuszeile bleibt „verworfene Analyseframes" niedrig oder stabil.

## 7 · M1: Die Kurve lebt

**Tu:** Plugin auf den Master, Song abspielen, Fenster offen lassen.
**Sehen:** Nach wenigen Sekunden erscheint eine blaugrüne Kurve (der
Langzeit-Durchschnitt) mit einer leiseren grauen Linie dahinter (das, was
GERADE klingt). Solange unten „sammelt · n s aktiv" steht, hält sich das
Plugin mit Befunden zurück — ab 15 Sekunden aktiver Musik steht „messbereit".
**Warum:** Zu wenig Material ⇒ keine Behauptung (das ist Absicht, kein Fehler).

## 8 · M1: Resonanz-Gegenprobe (der 116-Hz-Beweis am lebenden Objekt)

**Tu:** Lege auf eine leere Spur FLs **3x Osc**, spiele einen tiefen
Dauerton (**A#3** in FLs Piano-Roll ≈ 116 Hz) ÜBER laufendem Rauschen/Pad,
mindestens 20 s.
**Sehen:** Im Plugin auf dieser Spur (oder dem Master) erscheint ein
Bernstein-Dreieck an der Spitze mit „116 Hz +xx dB dauerhaft".
**Warum:** Genau diese Aussage („schmaler Dauerton sticht aus dem Bett") ist
der Kern von M1 — und sie braucht das lange Bassfenster.
**Hören:** Ton weg → nach einer Weile verschwindet der Kandidat (er ist an
die Messung gebunden, nicht an den Moment). „Neu messen" setzt sofort zurück.

## 9 · M1: Snapshot festhalten

**Tu:** Bei laufender Musik auf **Snapshot** klicken.
**Sehen:** Unten rechts erscheint kurz der Dateipfad
(`…\evenacadia\EQ-Copilot\snapshots\snapshot-….json`), und im Graph liegt ab
jetzt eine zweite, ruhige Vergleichslinie. **Dann:** Etwas am Mix ändern
(z. B. EQ am Klavier) → die Hauptlinie wandert, die Vergleichslinie bleibt.
„Vergleich aus" blendet sie weg.
**Warum:** Vorher/Nachher-Hören mit festgehaltenem Vorher ist der spätere
Kern des Hörzyklus (M4) — hier ist sein Fundament.

## 10 · M1: Ansicht-Spielereien ändern nie die Messung

**Tu:** Glättung umschalten (1/3 ↔ roh), Ansicht auf „Neigung", Fenster
skalieren — währenddessen weiterspielen.
**Sehen:** Die Kurve wird glatter/rauer bzw. kippt (in „Neigung" liegt
Rauschen waagrecht) — aber Zustand, Sekunden-Zähler und Kandidaten bleiben
unbeeindruckt. **Warum:** Anzeige und Messung sind getrennte Welten; das ist
eine M1-Abnahmebedingung.

## 11 · M2: Die Sensorübersicht in der App

**Tu:** Hub-App öffnen (Broker startet mit), in FL das Plugin auf 2–3 Spuren
laden und jedem im Plugin einen **Namen** geben (z. B. „PIANO-Bus",
„CHOR-Bus"). Dann in der App auf den **🎚-Knopf** in der Werkzeugleiste.
**Sehen:** Jeder Messpunkt als Karte — Name, Aufgabe, Zustand („hört zu …" /
„messbereit"), Lautheit und Spitze live. Der Knopf selbst zeigt die Anzahl
verbundener Messpunkte.
**Dann:** FL schließen. **Sehen:** Die Karten werden grau „getrennt (vor
x min)" — sie verschwinden NICHT still. **Warum:** Ein weg-gewischter Sensor
wäre gelogen; du sollst sehen, was fehlt.

## 12 · M2: Einem Song zuordnen

**Tu:** In der Sensorübersicht bei einem Messpunkt unter „Song:" dein
Projektprofil wählen.
**Warum:** So gerät nie ein Messpunkt aus einem ANDEREN Song in die
Auswertung dieses Songs.
**Sehen:** Die Zuordnung bleibt — auch nach App-Neustart und nach
FL-Neustart (sie hängt an der inneren Kennung des Plugins, nicht am Namen).

## 13 · M2: Kennungs-Konflikt durch Duplizieren

**Tu:** In FL eine Spur mit EQ-Copilot **duplizieren** (die Kopie trägt
dieselbe innere Kennung), beide spielen lassen.
**Sehen:** In BEIDEN Plugin-Fenstern erscheint nach wenigen Sekunden ein
roter Knopf **„Kennung doppelt!"**; auch die App färbt den 🎚-Knopf rot.
**Dann:** In EINEM der beiden Fenster auf den roten Knopf → „Dieser Instanz
eine neue Kennung geben".
**Sehen:** Der rote Knopf verschwindet in beiden Fenstern; in der App sind
jetzt zwei getrennte Messpunkte. **Warum:** Niemand entscheidet still, wer
wer ist — das tust du, einmal, sichtbar.

## 14 · M2: Vorher/Nachher-Paar

**Tu:** Auf EINER Spur zwei EQ-Copilot-Instanzen laden: eine VOR deinem EQ
(Aufgabe „VOR dem EQ"), eine DANACH („NACH dem EQ"), beiden dieselbe
**Paar-Kennung** geben (z. B. „piano-eq"). Song ein Stück am Stück spielen.
**Sehen:** In der Sensorübersicht erscheint unter „Vorher/Nachher-Paare" eine
Zeile mit Ampel: **grün = Passagen deckungsgleich** (gleiche Stelle
gemessen), **bernstein = wahrscheinlich** (z. B. nach Loopen/Springen),
**rot/grau = nicht vergleichbar** — mit dem Grund im Klartext.
**Dann:** Wild im Song herumspringen und weitermessen. **Sehen:** Die Ampel
fällt auf „wahrscheinlich" mit Grund „Sprünge" — die Messung LÜGT nicht,
wenn die Passagen nicht mehr sauber übereinanderliegen. „Neu messen" auf
beiden setzt frisch auf.

## 15 · M2: Gemeinsamer Snapshot

**Tu:** Bei laufender Messung in der Sensorübersicht auf **„Gemeinsamen
Snapshot festhalten"**.
**Sehen:** Unten erscheint der Dateipfad
(`…\evenacadia\EQ-Copilot\snapshots\aggregat-….json`) — EINE Datei mit dem
Stand ALLER Messpunkte samt Warnungen, falls etwas nicht zusammenpasst.
**Warum:** Das ist die Momentaufnahme „wie klingt der ganze Mix an allen
Punkten gleichzeitig" — die Grundlage der späteren Diagnose (M3).

## 16 · M3-Kern: Die Befundkarten — das Herzstück

**Tu:** Plugin auf den Master (oder einen Bus), deinen Song mindestens ~30
Sekunden am Stück spielen lassen. Dann auf den Hinweis-Knopf (er heißt jetzt
z. B. „2 Auffälligkeiten" oder „keine Auffälligkeit").
**Sehen:** Karten statt einer Liste: Jede Karte sagt getrennt **Gemessen**
(die Zahlen), **Wirkung** (was das fürs Hören bedeuten kann), **Tu** (der
konkrete Versuch mit Startwerten und deinem Werkzeug — Fruity PEQ2 oder
Smooth Operator Pro), **Warum**, **Hören** (worauf achten + wann aufhören)
und **Sicherheit**.
**Warum:** Genau das ist die Kernfunktion — nicht nur „da ist was", sondern
„das ist es, so probierst du es, daran hörst du, ob es besser wird".
**Dann als Gegenprobe:** Der 116-Hz-Trick aus §8 (3x Osc, A#3 über laufendem
Pad) → nach ~20 s erscheint eine Karte „Ein Ton sticht dauerhaft heraus"
mit einem konkreten Absenk-Startwert. Ton weg, „Neu messen" → Karte weg.
**Und:** Ein „Snapshot" (Festhalten-Knopf) schreibt die Karten jetzt MIT in
die Datei — die kannst du mir (Claude) zeigen, wenn du eine zweite Meinung
willst.

## 17 · M3a: Flüssiger Graph, Notennamen, Zeitverlauf, „Kurve steht"

**Tu:** Song abspielen und einfach auf die Live-Kurve schauen; dann Wiedergabe
stoppen.
**Sehen:** Die Kurve und die beiden Pegelschienen bewegen sich jetzt **weich**
(vorher ruckelte alles im Viertelsekunden-Takt — das war dein FPS-Befund).
Beim Stopp friert die Anzeige ein und das Plugin verbraucht sichtbar keine
Arbeit mehr (nichts flackert).
**Dann:** Nach ~30 s Musik ohne großen Szenenwechsel erscheint oben rechts im
Glas **„KURVE STEHT"**.
**Warum:** Das ist das Zeichen „die Messung hat sich eingeschwungen — ab jetzt
kannst du den Karten trauen". Bewegt sich der Song stark (neuer Teil), bleibt
das Zeichen weg — auch das ist Information.
**Dann:** Der 116-Hz-Trick (§8/§16): Die Karte heißt jetzt „… (116 Hz · A#3)"
— dieselbe Note, die du im 3x Osc gespielt hast, und im Gemessen steht, ob der
Pegel dort ruhig steht (fester Absenker sicher) oder in Wellen kommt
(dynamisches Werkzeug).
**Und:** Spiel eine Weile NUR Strophen-artiges (ruhig), dann laut — Zonen-
Karten (z. B. „Untenrum staut es sich") sagen jetzt „Das war in NN % der
Musikzeit so." — Halbzeit-Probleme werden nicht mehr wie Dauerprobleme
behandelt.

## 18 · Hör-Markierung: Probleme EINFÄRBEN und hören, wo sie sind

**Tu:** Musik laufen lassen, Hinweis-Knopf öffnen. Auf einer Befundkarte
stehen jetzt unter „Hören:" zwei kleine Tasten: **[Solo]** und **[Puls]**
(beim Mitten-Loch nur Solo — ein Loch kann nicht pulsieren). Drück **Puls**.
**Hören:** Der volle Mix läuft weiter, aber der Problembereich **schwillt im
Takt an und ab** (~1,6× pro Sekunde) — dein Ohr findet die Stelle von selbst.
Im Graph pulsiert die Tönung **exakt gleichzeitig** mit dem Hörbaren.
**Warum:** Das ist die Erfolgskontrolle beim Beheben: Öffne deinen EQ
(PEQ 2 / Smooth Operator Pro), zieh die empfohlene Glocke — **das Schwellen
wird leiser, bis es nichts mehr zu fassen hat.** Dann sitzt der Griff.
**Dann:** Drück **Solo**: nur noch der Problembereich spielt — so lernst du
seinen Klangcharakter isoliert kennen („DAS ist das Dröhnen"). Nochmal
drücken = aus; „Markierung aus" links in der Statuszeile geht immer.
**Wichtig zu wissen (bitte gegenprüfen):**
- Die Markierung startet erst nach ~1 s Wiedergabe („wartet auf
  Wiedergabe" steht in der Mikroleiste, bis die Echtzeit bewiesen ist).
- Transport-Stopp = sofort still; Fenster zu = Markierung endet; nach
  10 min ohne Bedienung endet sie von selbst.
- **Render-Beweis:** Lass eine Markierung absichtlich AN und exportiere den
  Song — die Ausspielung muss **komplett sauber** sein (die Markierung kann
  einen Offline-Render konstruktionsbedingt nicht bestehen). Bitte einmal
  bewusst prüfen und notieren.
- **Ehrliche Grenze:** Ein ECHTZEIT-Mitschnitt (z. B. Edison auf dem Master
  aufnehmen, während der Puls läuft) enthält die Markierung — das ist
  physikalisch nicht erkennbar. Darum: Markierung aus vor Edison-Aufnahmen.
- Der Graph zeigt während Solo weiter den ECHTEN Mix (die Kurve ist das
  Messgerät, nicht der Kopfhörer) — nur Tönung + Statuszeile zeigen, was
  gerade hörbar ist.

**Alles, was abweicht, bitte einfach formlos notieren (welcher Schritt, was
war anders) — daraus wird die Abnahme.**

*(Weiterhin Alltags-Beobachtung, kein eigener Schritt: 16 Instanzen unter
Last, Verhalten mit Latenz-Plugins vor einem Messpunkt, Offline-Render.)*
