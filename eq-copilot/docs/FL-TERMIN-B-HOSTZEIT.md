# FL-Termin B — was sagt FL dem Plugin ueber Zeit, Transport und Automation?

> **Stand 22.08.2026: gemessen** — Berichte, Aktionsprotokoll und Lesart in
> `docs/beweise/termin-b/`, Capabilityreport `docs/beweise/SONDE-004.md`.
> **Arbeitsteilung, wie sie wirklich lief:** der User legte Plugin, Piano und
> Automationsclip an, schaltete Smart disable ein, exportierte und klickte
> „Bericht sichern"; Abspielen, Springen, Schleife und Stummschalten fuhr
> Claude ueber den FL-MCP (`fl-studio`: `fl_play`, `fl_stop`,
> `fl_set_song_position` **nur im Modus 1 = Millisekunden** — Modus 2
> „Sekunden" landet nahe Songanfang —, `fl_set_loop_mode`, `fl_mute_channel`).
> Berichtigungen: (1) Schritt 1 — FL scannt VST3 nur unter
> `C:\Program Files\Common Files\VST3\`, Suchpfade gelten nur fuer VST2;
> das Messgeraet liegt seit 21.08. dort. (2) Der Song muss **mindestens 16
> Takte** lang sein (Clips nach rechts ziehen) — bei 2 Takten loopt FL im
> Sekundentakt, und Live (Schritt 3) ist von Schleife (Schritt 5) nicht mehr
> zu trennen. (3) Ein Vorwaertssprung braucht eine Zielposition hinter der
> aktuellen — die erste Sprungrunde war versehentlich rein rueckwaerts und
> wurde nachgemessen. Der Text unten ist unveraendert.

**Dauer:** ca. 25 Minuten. **Du brauchst:** FL Studio, sonst nichts.

## Worum es geht

Nakama soll spaeter sagen koennen: *„dieser Befund gehoert zu DIESER Stelle im
Song"* und *„dieser Reglerweg ist genau hier passiert"*. Beides haengt daran,
was FL dem Plugin ueberhaupt mitteilt — und das ist mit dem Auge nicht
ablesbar. VST3 schickt neben dem Ton eine Handvoll Zusatzangaben mit, und zu
jeder gehoert ein Haekchen „dieser Wert ist gueltig". Welche Haekchen FL setzt,
steht in keiner Oberflaeche.

Deshalb habe ich ein zweites Wegwerf-Plugin gebaut, das genau das anzeigt. Es
**veraendert deinen Klang nicht** — es reicht das Signal unveraendert durch und
zaehlt nur mit.

Am Ende beantwortet es sieben Fragen. **Jede Antwort ist ein gutes Ergebnis,
auch ein Nein.** Nur „weiss nicht" waere schlecht, denn danach wuerde ich
Funktionen bauen, die FL nicht traegt.

| Frage | Warum sie zaehlt |
|---|---|
| Schickt FL ueberhaupt eine Projektzeit mit? | Ohne sie kann Nakama einen Befund keiner Stelle im Song zuordnen. |
| Merkt das Plugin, wenn du **springst**? | Sonst rechnet es nach einem Sprung mit einer Zeitachse, die es nicht mehr gibt. |
| Was passiert an der **Schleifen-Grenze**? | Der Sprung zurueck darf nicht wie ein neuer Song aussehen. |
| Was macht **Smart Disable**? | Wenn FL das Plugin bei Stille pausiert, entsteht eine Luecke in der Zeitachse. Nakama muss sie von einem Sprung unterscheiden koennen — und wenn es das nicht kann, muss ich das WISSEN. |
| Kommt **Automation samplegenau** an? | Dafuer habe ich den JUCE-Patch gebaut. Wenn ja, kann Nakama Reglerwege exakt lesen; wenn nein, nur grob — und sagt das dann auch. |
| Was ist beim **Export/Render** anders? | Ein Befund aus dem Render muss von einem Live-Befund unterscheidbar sein. |
| Meldet FL eine **Verzoegerung je Bus**? | Wenn nicht, darf Nakama zwei Messpunkte nicht rechnerisch aneinander ausrichten. |

> Ehrlich dazu: die Klickwege unten stammen aus der FL-Dokumentation, nicht aus
> eigener Klickerfahrung. Heisst ein Menue bei dir anders, ist das kein Fehler
> von dir — sag mir wie es heisst, dann ziehe ich die Anleitung nach.

---

## 1. Vorbereitung (3 Minuten)

### Schritt 1 — Messgeraet fuer FL sichtbar machen

**Tu:** In FL: *Options → File settings → Manage plugins*. Bei den
VST-Suchpfaden diesen Ordner hinzufuegen:

```
C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbe_artefacts\Release\VST3
```

Dann *Find more plugins* / *Start scan*.

**Warum:** So muss nichts installiert werden — kein Administrator, keine
Systemordner. Am Ende nimmst du den einen Pfad wieder weg, und deine
Plugin-Sammlung ist unberuehrt.

**Sieh:** In der Liste erscheint **EqCop-Host-Probe**.

### Schritt 2 — Auf eine Spur legen und Fenster offen lassen

**Tu:** Neues, leeres Projekt. Irgendein Instrument oder Sample auf einen
Mixer-Kanal, dort **EqCop-Host-Probe** als Effekt einfuegen. Das Fenster offen
lassen.

**Warum:** Das Plugin misst nur, waehrend FL es aufruft — also waehrend Ton
durch diesen Kanal laeuft.

**Sieh:** Ganz oben steht **„Hostbruecke liefert: JA"** in Gruen.

> ⚠️ **Steht dort NEIN in Rot, hoer hier auf und sag mir Bescheid.** Dann laeuft
> ein Bau ohne den JUCE-Patch, und alles Weitere waere Messung ohne Messgeraet.

---

## 2. Die Messungen (etwa 18 Minuten)

Zwischen den Messungen musst du **nichts** zuruecksetzen — das Plugin merkt sich
alles nebeneinander. Nur wenn du komplett neu anfangen willst, drueckst du
*Messung zuruecksetzen*.

### Schritt 3 — Live: einfach abspielen

**Tu:** Etwa 20 Sekunden abspielen lassen.

**Warum:** Das ist der Normalfall. Alles Weitere wird dagegen verglichen.

**Sieh:** *Bloecke verarbeitet* zaehlt hoch. In der Liste „Was FL im Context
ausfuellt" steht bei den Zeilen entweder **immer**, **manchmal** oder **nie** —
das ist schon die halbe Antwort auf Frage 1.

### Schritt 4 — Springen, waehrend es LAEUFT

**Tu:** Abspielen lassen und **ohne zu stoppen** ins Playlist-Lineal klicken —
einmal weiter hinten, einmal weiter vorn. Zwei bis drei Spruenge reichen.

**Warum:** Ein Sprung ist fuer das Plugin ein Bruch in der Zeitachse. Es muss
ihn erkennen, ohne bei normalem Abspielen staendig Fehlalarm zu geben.

⚠️ **Wichtig ist das „ohne zu stoppen".** Ein Positionswechsel ueber ein
Stop/Play hinweg ist etwas anderes — dort ist jede neue Position legitim, und
das Plugin zaehlt ihn deshalb in einer eigenen Zeile. Wuerdest du nur so
springen, blieben die beiden Sprung-Zeilen auf 0, und ich koennte „FL meldet
keine Spruenge" nicht von „wir haben es nicht ausgeloest" unterscheiden.

**Sieh:** *Zeitspruenge vorwaerts* und *rueckwaerts* zaehlen hoch.

### Schritt 4b — Und einmal MIT Stop, zum Vergleich

**Tu:** Stoppen, im Lineal woanders hinklicken, wieder abspielen.

**Warum:** Das ist der Gegenfall. Er darf die Sprung-Zeilen NICHT hochzaehlen —
sonst wuerde Nakama spaeter jeden Neustart fuer einen Zeitbruch halten.

**Sieh:** *Positionswechsel ueber Stop/Play* zaehlt um eins hoch, die beiden
Sprung-Zeilen bleiben unveraendert.

### Schritt 5 — Schleife

**Tu:** Einen kurzen Bereich als Schleife markieren (Loop) und zwei- bis
dreimal durchlaufen lassen.

**Warum:** An der Schleifen-Grenze springt die Zeit zurueck. Ich muss wissen, ob
das anders aussieht als ein Sprung von Hand.

**Sieh:** *Transport / Schleife* zeigt **Schleife an**, und
*Zeitspruenge rueckwaerts* zaehlt bei jedem Durchlauf einen dazu.

### Schritt 6 — Automation (die wichtigste Messung)

**Tu:** Im Plugin-Fenster den Regler **„Testwert (nur Messung)"** suchen (er
steht in FLs Plugin-Kopfzeile unter *Browse parameters*). Rechtsklick →
*Create automation clip*. In den Automationsclip eine **schnelle Kurve** malen —
kein langsamer Anstieg, sondern deutliche Zacken. Dann abspielen.

**Warum:** Das ist die Frage, fuer die der ganze JUCE-Patch gebaut wurde. Eine
langsame Kurve schickt pro Block nur einen Wert; eine schnelle Kurve schickt
mehrere — und nur wenn mehrere ankommen, kann Nakama Reglerwege exakt lesen.

**Sieh:** Unter „Samplegenaue Automation":
- *Automationspunkte* zaehlt hoch,
- *Punkte im dichtesten Block* steht auf einer Zahl **groesser als 1**,
- *Mehr als EIN Punkt je Block?* sagt **JA**.

> Steht dort „bisher nie", zeichne die Kurve steiler und spiel nochmal.
> Bleibt es dabei, ist das ein **gueltiges Ergebnis** — dann sagt Nakama spaeter
> ehrlich „nur grob" statt „samplegenau" zu behaupten.

### Schritt 6b — Smart Disable

**Tu:** In FLs Wrapper-Menue des Messgeraets (das kleine Dreieck oben links im
Plugin-Fenster) **Smart disable** einschalten. Dann etwa 10 Sekunden **Stille**
laufen lassen (Abspielen, aber die Spur still — z. B. Pattern leer) und danach
wieder Ton.

Wenn du *Smart disable* nicht findest: ueberspring den Schritt und sag mir das.
Das ist selbst eine Antwort.

**Warum:** Bei Stille darf FL das Plugin pausieren. Danach geht es an einer
spaeteren Stelle weiter — fuer das Plugin sieht das aus wie ein Sprung nach
vorn. Ich muss wissen, ob das in FL wirklich passiert.

**Sieh:** *Zeitspruenge vorwaerts* zaehlt hoch, obwohl du nirgends hingeklickt
hast.

> Ehrlich dazu: **aus den Daten allein kann ich einen Smart-Disable-Sprung nicht
> von einem Seek unterscheiden** — beide sehen gleich aus. Auseinander halte ich
> sie nur, weil du sie in verschiedenen Schritten ausloest und das Plugin die
> Reihenfolge protokolliert. Deshalb ist es wichtig, dass Schritt 4 und dieser
> Schritt nicht durcheinandergehen.

### Schritt 7 — Export (Render)

**Tu:** *File → Export → WAV*. Kurzen Bereich exportieren, egal wohin.
**Wichtig:** das Plugin-Fenster danach wieder oeffnen, ohne das Projekt zu
schliessen.

**Warum:** Beim Export laeuft FL schneller als Echtzeit. Ich muss wissen, ob das
Plugin das ueberhaupt mitgeteilt bekommt.

**Sieh:** *Offline-Bloecke (Render)* steht nicht mehr auf **0**, sondern auf
einer Zahl groesser als 0.

### Schritt 8 — Der Rest ergibt sich von selbst

**Tu:** Nichts Zusaetzliches.

**Sieh:** Zwei Stellen haben sich waehrend der Messungen nebenbei beantwortet:

- *float / double* — welche Rechengenauigkeit FL benutzt.
- Der Block **„Presentation-Latency je Bus"** ganz unten. Dort stehen zwei
  Zeilen, *Eingang* und *Ausgang*, und beide haben genau zwei moegliche
  Ausgaenge:
  - **„nicht gemeldet"** (gelb) — FL sagt zu dieser Richtung nichts. Das ist ein
    vollstaendiges Ergebnis, keine fehlende Messung; viele Hosts melden es nicht.
    Solange **beide** Richtungen so stehen, erscheint darunter zusaetzlich der
    Satz „bisher NIE gemeldet - das ist ein gueltiger Befund".
  - **„Bus 0 = <Zahl>"** (gruen) — FL meldet eine Verzoegerung. Dann brauche ich
    die Zahl; sie steht ohnehin im Bericht.

  Die beiden Richtungen sind unabhaengig: „Eingang nicht gemeldet · Ausgang
  Bus 0 = 1024" ist ein voellig normales Ergebnis.

---

## 3. Zusatz: die alte Projektdatei (5 Minuten, optional aber wertvoll)

Dieser Teil hat mit dem Messgeraet nichts zu tun. Er sichert, dass ein
**heutiges** Projekt spaeter unveraendert laedt, wenn Nakama in drei Teile
zerfaellt.

**Tu:** Neues Projekt. Vier Mixer-Kanaele, auf jeden **eine Instanz des
normalen EQ-Copilot** (nicht das Messgeraet). In jeder Instanz oben bei
*AUFGABE DIESES MESSPUNKTS* eine andere Auswahl treffen:

1. **Misst diese Spur**
2. **Sammelpunkt (Master)**
3. **VOR dem EQ (Paar)**
4. **NACH dem EQ (Paar)**

Speichern als `nakama-altprojekt.flp`, irgendwo wo du es wiederfindest.

**Warum:** Das ist die Vergleichsdatei. Wenn ich spaeter die Aufteilung baue,
muss genau dieses Projekt sich weiterhin **bitgleich** laden lassen. Ohne die
Datei von heute gaebe es nichts zum Vergleichen.

**Sieh:** Vier Instanzen, jede mit einer anderen Aufgabe im Kopf.

---

## 4. Was ich von dir zurueckbrauche

**Tu:** Im Plugin-Fenster unten auf **„Bericht sichern"** klicken.

**Sieh:** Unten erscheint ein Dateipfad in
`%APPDATA%\evenacadia\nakama\spike\` — die Datei heisst `host-probe-<Datum>.json`.

Schick mir:

1. diese **eine JSON-Datei**,
2. die **`nakama-altprojekt.flp`** aus Teil 3 (falls du ihn gemacht hast),
3. einen Satz dazu, ob dir etwas komisch vorkam,
4. und — falls du Schritte ausgelassen oder in anderer Reihenfolge gemacht hast
   — welche. Das Ereignisprotokoll im Bericht ist zeitlich geordnet; mit deiner
   Reihenfolge kann ich Seek und Smart Disable auseinanderhalten, ohne sie
   raten zu muessen.

Mehr nicht. Alles Weitere lese ich aus der Datei.

---

## 5. Wenn etwas nicht passt

- **„Hostbruecke liefert: NEIN"** → abbrechen, mir sagen. Der Bau stimmt nicht.
- **Ein Menue heisst anders** → sag mir wie, ich ziehe die Anleitung nach.
- **Das Plugin erscheint nicht in der Liste** → Suchpfad pruefen (er endet auf
  `\VST3`) und *Rescan previously verified plugins* anhaken.
- **Alle Zahlen bleiben auf 0** → laeuft wirklich Ton durch diesen Kanal? Das
  Plugin misst nur, waehrend FL es aufruft.
- **Etwas stuerzt ab** → sag es mir mit dem, was du zuletzt getan hast. Das ist
  selbst ein Messergebnis.

---

## 6. Aufraeumen danach

**Tu:** In *Manage plugins* den Suchpfad aus Schritt 1 wieder entfernen, dann
einmal *Start scan*.

**Warum:** Das Messgeraet ist Wegwerfware — es wird nach dem Capabilityreport
nicht weitergepflegt und soll nicht in deiner Plugin-Sammlung liegenbleiben.

**Sieh:** **EqCop-Host-Probe** verschwindet aus der Liste. Der normale
EQ-Copilot bleibt unberuehrt — er war nie Teil dieses Termins.
