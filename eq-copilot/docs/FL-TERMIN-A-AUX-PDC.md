# FL-Termin A — koennen zwei getrennte Aux-Wege in FL sauber ankommen?

> **Stand 22.08.2026: gemessen, Ergebnis „geht"** — Rohdaten und Aufbau in
> `docs/beweise/termin-a/`, Capabilityreport `docs/beweise/SONDE-004.md`.
> Zwei Berichtigungen an dieser Liste, gelernt beim Messen: (1) FL scannt VST3
> **nur** unter `C:\Program Files\Common Files\VST3\` — ein Suchpfad gilt
> nur fuer VST2; das Messgeraet muss dorthin (Admin-Klick), die Zeile „kein
> Admin noetig" war falsch. (2) Nebenwege als **„Sidechain to this track"**
> legen, nicht als Mixroute — sonst wird der Ton in der Zielspur mitgemischt
> (Spitze 2,70 statt 0,90). Der Text unten ist unveraendert.

**Dauer:** ca. 20 Minuten. **Du brauchst:** FL Studio, sonst nichts.

## Worum es geht

Nakama soll spaeter sagen koennen: *„dieser Beitrag kommt von DIESER Spur"* und
*„so klingt es mit und ohne"*. Beides haengt daran, dass FL zwei getrennte
Nebenwege (Aux) in ein Plugin schicken kann — **gleichzeitig**, **in der
richtigen Reihenfolge** und **zeitlich exakt** zum Hauptweg.

Ob FL das kann, weiss ich nicht. Ich habe es nicht geraten, sondern ein
Messgeraet gebaut: ein Wegwerf-Plugin, das nur zaehlt und anzeigt. Es
veraendert deinen Klang nicht — es reicht das Signal unveraendert durch.

Am Ende steht eine von drei Antworten: **geht** · **geht mit festem Versatz** ·
**geht nicht**. Alle drei sind gute Ergebnisse. Nur „weiss nicht" waere schlecht,
denn danach wuerde ich Funktionen bauen, die FL nicht traegt.

> Ehrlich dazu: die Klickwege unten habe ich aus der FL-Dokumentation und aus
> Image-Line-Foren zusammengetragen, nicht selbst in FL geklickt. Wenn ein Menue
> bei dir anders heisst, ist das kein Fehler von dir — sag mir wie es heisst,
> dann ziehe ich die Anleitung nach.

---

## 1. Vorbereitung (5 Minuten)

### Schritt 1 — Impulsdatei erzeugen

**Tu:** In der Kommandozeile im Nakama-Ordner:

```powershell
py -3.13 tools/eq-copilot/erzeuge_aux_spike_fixtures.py
```

**Warum:** Das legt zwei kurze WAVs an — zwei Sekunden Stille mit **einem
einzigen Knacks** in der Mitte. Der Knacks ist die Stoppuhr: wir schauen
nachher, ob er auf allen Wegen zur gleichen Zeit ankommt.

**Sieh:** Zwei Zeilen mit `sha256=…`. Die Dateien liegen in
`eq-copilot\fixtures\aux-spike\`. Nimm die, deren Zahl zu deiner Projektrate
passt — dein FL laeuft auf **44100**, also `impuls-44100.wav`.

### Schritt 2 — Messgeraet fuer FL sichtbar machen

**Tu:** In FL: *Options → File settings → Manage plugins*. Bei den
VST-Suchpfaden diesen Ordner hinzufuegen:

```
C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAuxSpike_artefacts\Release\VST3
```

Dann *Find more plugins* / *Start scan*.

**Warum:** So muss nichts installiert werden — kein Administrator, keine
Systemordner, nichts, was hinterher aufzuraeumen waere ausser diesem einen
Pfad. Das Ding ist ein Wegwerfwerkzeug, es soll deine Plugin-Sammlung nicht
anfassen.

**Sieh:** In der Liste erscheint **EqCop-Aux-Spike**. Falls nicht: den Pfad
nochmal pruefen (er endet auf `\VST3`) und *Rescan previously verified
plugins* anhaken.

---

## 2. Der Aufbau (10 Minuten)

Wir bauen drei Spuren, die **dieselbe** Impulsdatei zur **gleichen Zeit**
abspielen. Eine geht direkt ins Messgeraet, zwei ueber Nebenwege.

### Schritt 3 — Hauptweg

**Tu:** Impulsdatei in die Playlist ziehen, Takt 1. Diesen Kanal auf
**Mixer-Insert 1** legen. Auf Insert 1 das **EqCop-Aux-Spike** laden.

**Warum:** Insert 1 ist der Hauptweg — der Bezugspunkt, gegen den alles
gemessen wird.

**Sieh:** Oeffne das Plugin. Oben steht der Hostname, darunter *„Noch kein
Audio verarbeitet"*. Druecke Play: die Zeile wird gruen und zaehlt Samples.
Unter **MAIN** erscheint nach dem Knacks *„Impuls: Sample …"*.

> Bleibt die Zeile grau, kommt kein Audio an — dann stimmt das Routing des
> Kanals auf Insert 1 nicht.

### Schritt 4 — Erster Nebenweg

**Tu:** Dieselbe Impulsdatei ein zweites Mal in die Playlist, **exakt auf
denselben Takt**. Diesen Kanal auf **Mixer-Insert 2** legen. Dann im Mixer
Insert 2 anklicken, auf Insert 1 **rechtsklick auf den Pfeil unten** →
*Sidechain to this track*.

**Warum:** „Sidechain" heisst in FL nichts anderes als: schick dieses Signal
zusaetzlich als Nebenweg in das Plugin auf der Zielspur. FL vergibt dabei eine
Nummer — die brauchen wir gleich.

**Sieh:** Am Pfeil von Insert 2 nach Insert 1 erscheint eine kleine Ziffer,
typischerweise **1**.

### Schritt 5 — Nebenweg auf die richtigen Stifte legen

**Tu:** Am Plugin-Fenster oben die **Wrapper-Leiste** oeffnen (das schmale
Band ueber der Plugin-Oberflaeche, Zahnrad/Pfeil) → Seite **VFX / Pins** bzw.
*Processing → Inputs*. Dort **Sidechain 1** auf die **Plugin-Eingaenge 3 und 4**
legen.

**Warum:** Das ist die Stelle, an der solche Versuche fast immer scheitern.
Das Plugin zaehlt seinen Hauptweg als Eingang **1 und 2**. Der erste Nebenweg
ist deshalb **3 und 4** — nicht 1 und 2. Der zweite waere 5 und 6.

**Sieh:** Im Messgeraet wird die Zeile **AUX 1 · priority_sidechain** gruen und
zeigt *„2 Kanaele aktiv"*. Steht dort weiter *INAKTIV*, hat FL den Bus nicht
zugeschaltet — dann bitte genau so notieren, das ist bereits ein Messergebnis.

### Schritt 6 — Zweiter Nebenweg

**Tu:** Dasselbe nochmal: dritte Spur mit derselben Datei auf demselben Takt,
**Insert 3**, Sidechain auf Insert 1, im Wrapper auf **Eingaenge 5 und 6**.

**Warum:** Die eigentliche Frage ist nicht „geht ein Nebenweg", sondern „gehen
**zwei getrennte**, ohne sich zu vermischen". Genau das braucht Nakama spaeter.

**Sieh:** Auch **AUX 2 · compare_pre** wird gruen.

---

## 3. Die Messung (2 Minuten)

### Schritt 7 — Alles still, dann einmal abspielen

**Tu:** Alles andere im Projekt stumm schalten. Im Messgeraet **„Messung
zuruecksetzen"** druecken. Dann von vorn abspielen, bis der Knacks durch ist.

**Warum:** Das Geraet sucht den ersten lauten Moment. Laeuft nebenher Musik,
findet es irgendeinen lauten Moment — und das waere Zufall, keine Messung. Es
merkt das selbst und sagt dann *„Dauersignal …"* statt eine Zahl zu erfinden.

**Sieh:** Drei Zeilen mit *Impuls: Sample …*. Und darunter das Ergebnis:

- **Versatz: 0 Samples** → FL gleicht den Nebenweg vollstaendig aus. Bestes
  Ergebnis.
- **Versatz: +N Samples** → fester Zeitunterschied. Auch brauchbar, wenn N
  gleich bleibt.
- **Versatz: — (…)** → in der Klammer steht der Grund im Klartext.

### Schritt 8 — Bleibt es nach Speichern und Laden gleich?

**Tu:** **„Bericht schreiben"** druecken. Projekt speichern, FL schliessen,
Projekt wieder oeffnen. Ohne etwas zu aendern: zuruecksetzen, abspielen,
nochmal **„Bericht schreiben"**.

**Warum:** Ein Zeitunterschied, der sich beim naechsten Oeffnen aendert, ist
schlimmer als ein grosser fester — auf so etwas kann man nichts bauen. Deshalb
messen wir zweimal.

**Sieh:** Unten im Fenster steht jeweils der Pfad der geschriebenen Datei, in
`%APPDATA%\evenacadia\nakama\spike\`.

---

## 4. Was ich von dir brauche

1. Die **zwei JSON-Dateien** aus `%APPDATA%\evenacadia\nakama\spike\`
   (Explorer-Adresszeile: `%APPDATA%\evenacadia\nakama\spike`).
2. Ein Satz dazu, ob im Wrapper etwas anders hiess als hier beschrieben.
3. Falls ein Nebenweg **INAKTIV** blieb: bei welchem Schritt es haengen blieb.

Mehr nicht — die Zahlen lese ich aus den Dateien.

---

## 5. Wenn etwas nicht passt

| Was du siehst | Was es heisst | Was zu tun ist |
|---|---|---|
| *„Noch kein Audio verarbeitet"* trotz Play | Insert 1 bekommt kein Signal | Kanal-Routing auf Insert 1 pruefen |
| AUX bleibt *INAKTIV* | FL bietet den Bus nicht an | notieren — das ist ein Messergebnis, kein Fehler |
| AUX aktiv, aber *„Signal: nein"* | Bus da, Pins falsch | im Wrapper auf 3/4 bzw. 5/6 legen |
| *„Dauersignal …"* | es lief noch etwas mit | alles stumm, zuruecksetzen, nochmal |
| Sidechain-Nummer ist nicht 1 | die Nummern sind projektweit | die Nummer nehmen, die dasteht — beim Loeschen rutschen sie nach |

---

## 6. Aufraeumen danach

**Tu:** Den in Schritt 2 hinzugefuegten Suchpfad wieder entfernen und einmal
neu scannen. Das Testprojekt kannst du loeschen.

**Warum:** Das Messgeraet ist Wegwerfware und hat in deiner Plugin-Liste nichts
verloren. Es wurde nie installiert, deshalb genuegt es, den Pfad wieder
wegzunehmen — es bleibt nichts zurueck.
