# NAK-286 — Validierung der Codex-Matrixprüfung 2, Teil B (lesend)

**Ticket:** NAK-286 (Plan S25e, Laufzeit-Arm), Etappe 1 — Verhaltensmatrix und
Bauplan in `docs/beweise/NAK-286.md`.
**HEAD:** `611979c7a7885947039ed93ff66f303c31238a5a` (Zweig `master`).
**Datum:** 14.09.2026.
**Umfang:** Befunde 4, 5, 6 und 7 der Rohdatei
`docs/beweise/roh/NAK-286-matrixpruefung-2-611979c.txt` — die vier
Briefkasten-Befunde (M-33, M-77, M-51, M-53) der Matrixprüfung 2 zum Fixdiff
`c6c59170..7d9e8a79`.
**Vorgehen:** nur gelesen. Nichts gebaut, kein Test, kein Skript, keine
Installation; Git nur lesend (`git log`, `git status --short`, `git show
c6c59170:docs/beweise/NAK-286.md`, `git ls-files --eol`, `git check-attr`);
geschrieben wurde ausschließlich diese Datei. Die fremden untracked Ordner
`briefing-hub/` und `nimbalyst-local/` blieben unberührt.

**Zeilenversatz: null.** Das Urteil ist am selben Stand entstanden, den es
zitiert (`611979c7…`, Schlussabsatz der Rohdatei: „HEAD blieb vor/nach der
Prüfung identisch"). Jede Zeilenangabe des Prüfers gilt an HEAD unverändert;
nachgemessen für `:532` (M-33), `:538` (M-77), `:567` (M-51), `:569` (M-53),
`:374` (F-14), `:357` (F-4), `:1029` (P-1), `:1045` (Leitsatz).

**Bezugsstand für die Ursachenfrage.** Der Fixdiff hat das Namensmodell
geändert. Die Vorher-Fassungen der drei betroffenen Zeilen stehen in
`c6c59170:docs/beweise/NAK-286.md` (M-33 `:514`, M-51 `:547`, M-53 `:549`) und
werden unten wörtlich gegen HEAD gestellt; ohne sie ist die gemeinsame Ursache
nicht entscheidbar.

---

## Befund 4 — „Erzwinge für M-33(2) eine echte Zielnamenskollision"

**Wörtlich (Urteil):** `[P2] Erzwinge für M-33(2) eine echte Zielnamenskollision
— C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:532-532 — DEFEKT im
zweiten Rotbeweis: In Lage (b) erhalten die beiden Instanzen nun ausdrücklich
verschiedene `laufzeit32` und damit verschiedene Zielnamen. Allein das
Einschalten von Ersetzen beim Umbenennen lässt deshalb weiterhin zwei Dateien
entstehen; die behauptete Reduktion auf eine Datei findet nicht statt. Der
UUID-Fix beseitigt den Deckel, macht aber den übernommenen Ersetzungs-Rotbeweis
wirkungslos. Für die Zusage „keine Datei wird je ersetzt" braucht die Probe
einen bereits belegten identischen Zielnamen und eine Beobachtung seiner
Erhaltung, entsprechend dem Beweisleitsatz (NAK-286.md#L1045).`

**Gegenstand — M-33, `docs/beweise/NAK-286.md:532`** (Zustand, Zusage,
Rotbeweis):

> Zustand: „Probeeq-Instanzen in einem Prozess: (a) zwei mit verschiedenen
> persistenten Instanzkennungen; (b) zwei mit gleicher Kennung
> (Kennungskonflikt); (c) zehn mit gleicher Kennung …" · Zusage: „je
> Instanzobjekt genau eine Datei: (a) zwei Dateien mit verschiedenem
> `instanz16`; (b) zwei Dateien mit gleichem `instanz16` und verschiedenem
> `laufzeit32`; (c) zehn Dateien, keine Instanz gibt die Anfrage auf
> (Fehlerzähler aller zehn 0); **keine Datei wird je ersetzt.** Mechanismus:
> Laufzeitkennung im Namen; Geltungsrahmen: alle lebenden Instanzobjekte des
> Prozesses (F-3, §7.1 T-3)" · Rotbeweis: „(1) Laufzeitkennung aus dem Namen
> entfernt → in (c) findet nur eine Instanz einen freien Namen, neun
> Umbenennungen scheitern, nach drei Takten stehen neun Fehlerzähler auf 1 →
> rot; **(2) Umbenennen mit Ersetzen → in (b) bleibt eine Datei → rot**".

**Gegenstand — F-3, `:356`** (die Zusage, die Rotbeweis (2) tragen soll):

> „**Keine Suffixleiter, kein Deckel:** zwei lebende Instanzobjekte eines
> Prozesses tragen verschiedene Laufzeitkennungen, auch bei gleicher
> persistenter Kennung … Keine Instanz gibt eine gültige Anfrage wegen ihres
> Namens auf; **scheitert das Umbenennen dennoch an einem vorhandenen Namen,
> ist das ein gescheiterter Schreibversuch nach F-13, nie ein Ersetzen.**"

**Quelle an HEAD — tragen zwei Instanzobjekte wirklich verschiedene
Laufzeitkennungen?** Ja, beide Kennungen entstehen je Objekt im Konstruktor.

`eq-copilot/plugin/sonde/SondeProcessor.cpp:132-138` (Probeeq, die Rolle aus
M-33):

```cpp
SondeProcessor::SondeProcessor()
    : juce::AudioProcessor (BusesProperties()
          …
      v3RuntimeNonce (uuidHex32()),
```

und `:20-30` bildet `uuidHex32()` aus `juce::Uuid().toString()`. Gen ebenso,
`eq-copilot/plugin/src/PluginProcessor.cpp:242-243`:

```cpp
    zustand = nakama::state::frisch (juce::Uuid().toString());
    instanceNonce = juce::Uuid().toString();
```

Zwei Probeeq-Objekte eines Prozesses bekommen damit zwei verschiedene
`v3RuntimeNonce` — auch dann, wenn ihr geladener `zustand.common.instanceId`
identisch ist (kopierter Slot, `eq-copilot/plugin/state/NakamaState.cpp:1482`,
zitiert in T-3, `:917`). Der Antwortname nach F-3 ist
`<anfrage_id>.<rolle>.<pid>.<instanz16>.<laufzeit32>.json`; in Lage (b) sind
`anfrage_id`, `rolle`, `pid` und `instanz16` gleich, `laufzeit32` verschieden —
also **zwei verschiedene Zielnamen**.

**Quelle an HEAD — was tut „Umbenennen mit Ersetzen" dann?** Das in F-8 Schritt
(5) übernommene Muster ist `broker/src/persistenz.rs:78-109`:

```rust
    let mut flags = MOVEFILE_WRITE_THROUGH;
    if ersetzen {
        flags |= MOVEFILE_REPLACE_EXISTING;
    }
```

`MOVEFILE_REPLACE_EXISTING` wirkt nur, wenn der Zielname belegt ist; ist er es
nicht, ist der Aufruf mit und ohne Flag derselbe Erfolg. In Lage (b) ist keiner
der beiden Zielnamen belegt.

**Vorher-Stand (der die Ursache zeigt), `c6c59170:docs/beweise/NAK-286.md:514`:**

> Zusage: „(a) zwei Dateien mit verschiedenem `instanz16`; **(b) zwei Dateien,
> die zweite mit Suffix `.2`**; keine Datei wird je ersetzt" · Rotbeweis:
> „Umbenennen mit Ersetzen → in (b) bleibt eine Datei → rot".

Vor dem Fix rechneten beide Instanzen in Lage (b) **denselben** Basisnamen; nur
die Suffixleiter trennte sie. Mit Ersetzen schrieb die zweite Instanz über die
erste — der Rotbeweis fiel. Die Nacharbeit hat den Namenskonflikt beseitigt und
den Rotbeweis wörtlich stehen lassen.

**Status: BESTÄTIGT.** Rotbeweis (2) ist an HEAD wirkungslos: die Mutation
verändert in Lage (b) kein beobachtbares Ergebnis, weil sie einen Zweig
einschaltet, den die Lage nie erreicht. Die Zusage „keine Datei wird je
ersetzt" hat damit **keinen** Beweis mehr; Rotbeweis (1) trägt eine andere
Zusage (Eindeutigkeit des Namens je Instanzobjekt, Lage (c)) und ist intakt.
Nach dem Leitsatz (`:1045`) und der Matrixregel §4 „Dateisystem und
Fehlerinjektion" (`:479`) fällt der Rotbeweis damit neben der Zusage.
*Einschränkung:* die Zusage selbst ist richtig und bleibt; der Defekt liegt
allein im Beweisweg.

**Welche Lage erzeugt einen bereits belegten identischen Zielnamen?** Alle fünf
Namensglieder müssen gleich sein; `laufzeit32` ist je Instanzobjekt eindeutig,
also muss es **dieselbe Instanz** sein, die ein zweites Mal auf denselben Namen
schreibt. Drei Wege stehen im Manifest:

1. **Dieselbe Instanz, dieselbe Kennung, Ring umgangen.** Das ist bereits
   M-25s Rotbeweis, `:523`: „Ringabfrage entfernt → die Fassade zählt für K
   einen zweiten Anlegeversuch (**das Umbenennen ohne Ersetzen scheitert danach
   am vorhandenen Namen**)". Der Weg braucht eine Mutation, um überhaupt
   entstehen zu können — als Bühne für eine *Zusage* taugt er deshalb nicht,
   nur als Beleg, dass das Modell die Kollision kennt.
2. **Verdrängte Kennung erneut vorgelegt** (M-51, F-9) — das ist Befund 6, dort
   ist es kein Rotbeweis, sondern der Grund, warum eine Zusage nicht erfüllbar
   ist.
3. **Vorab angelegte Datei mit dem Zielnamen** — die einzige Bühne, die *ohne*
   Mutation und ohne Regelbruch entsteht und deren Beobachtung genau die Zusage
   trägt.

**Beobachtung bei Weg 3.** Der Zielname ist für den Test ohne Testzugang
ableitbar: die Instanz beantwortet zuerst Kennung K1 und schreibt
`K1.probeeq.<pid>.<instanz16>.<laufzeit32>.json`; der Zielname für die nächste
Kennung K2 entsteht daraus durch Austausch des ersten Namensglieds. Der Test
legt unter diesem Namen eine Markendatei an und legt dann K2 vor. Erwartet:

- die Markendatei bleibt **byteweise erhalten** (Hash vor und nach den Takten
  gleich) — das ist „keine Datei wird je ersetzt";
- der Anlegeversuch scheitert nicht, das **Umbenennen** scheitert (F-8 Schritt
  5, Muster `persistenz.rs:78-109`, Fehler `AlreadyExists` beziehungsweise
  Win32 80/183 wie `ist_bereits_vorhanden`, `:72-75`);
- die Temp-Datei `<Antwortname>.tmp-<pid>` bleibt liegen, **Löschzähler der
  Fassade 0** (F-8 Schritt 6);
- die beiden Folgetakte scheitern schon am exklusiven Anlegen derselben
  Temp-Datei (F-8 Schritt 2: „ein vorhandener Name ist ein gescheiterter
  Versuch"), nach drei Takten ist die Kennung aufgegeben und der Fehlerzähler
  steht auf 1 (F-13, `:373`) — genau der Ausgang, den F-3 zusagt („ein
  gescheiterter Schreibversuch nach F-13, nie ein Ersetzen");
- **Mutation:** Umbenennen mit Ersetzen → die Markendatei wird überschrieben,
  ihr Hash ändert sich, Fehlerzähler bleibt 0 → **rot**.

**Einordnung (Vorschlag): DEFEKT** (der Beweisweg trägt die Zusage nicht; §4
`:479`, Leitsatz `:1045`).

**Schließende Änderung (Vorschlag).** M-33 bekommt eine vierte Lage **(d)**:
„eine Instanz; der Zielname der zweiten Kennung ist im Antwortordner bereits
durch eine Markendatei belegt". Zusage der Lage: die Markendatei bleibt
byteweise erhalten, Löschzähler 0, genau eine Temp-Datei
`<Antwortname>.tmp-<pid>`, nach drei Takten Fehlerzähler 1, keine `.json` mit
dem Zielnamen aus dem Umschlag. Rotbeweis (2) wandert von (b) auf (d):
„Umbenennen mit Ersetzen → Hash der Markendatei ändert sich → rot". Lage (b)
behält Rotbeweis (1), der sie mitträgt (Laufzeitkennung aus dem Namen entfernt
→ beide Instanzen rechnen denselben Namen → eine Datei plus ein gescheiterter
Versuch → rot). Der Fall `instanzen_je_eine_datei` in `BriefkastenTestMain.cpp`
(B30) bekommt die Lage (d) als vierten Unterfall; die Rust-Hälfte in A4 braucht
sie nicht (ein Brokerlauf je Prozess, T-3 `:917`), es sei denn, der Bauer will
denselben Wortlaut in beiden Sprachen (`tools/dirigent/pruefliste.md` A).

---

## Befund 5 — „Trenne in M-77 Startzugriffe und nachfolgende Taktzugriffe"

**Wörtlich (Urteil):** `[P2] Trenne in M-77 Startzugriffe und nachfolgende
Taktzugriffe — C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:538-538 —
DEFEKT im Reparse-Durchlauf: Das Ereignis umfasst die Konstruktion, gleichzeitig
verlangt die Zusage insgesamt 0 Fassadenaufrufe. Um `evenacadia` beim Start als
Reparse-Punkt zu erkennen, muss jedoch bereits eine Attributprüfung erfolgen;
nach F-14 (NAK-286.md#L374) muss auch diese über die gezählte
Dateisystem-Fassade laufen. Eine regelkonforme Implementierung verletzt somit
schon ohne gestarteten Takt den Nullzähler. Die Startprüfungen müssen getrennt
ausgewiesen und die Nullzugriffe als Differenz für die anschließenden fünf
Taktauslösungen geprüft werden; ein ungezählter Startzugriff wäre kein
zulässiger Ausweg.`

**Gegenstand — M-77, `docs/beweise/NAK-286.md:538`:**

> Zustand: „Gen- und Probeeq-Prozessor im Produktzweig; die Wurzel-Fassade
> meldet für `windowsLocalAppData` keinen vorhandenen Ordner; **zweiter
> Durchlauf: `evenacadia\` ist beim Start ein Reparse-Punkt (F-11)**" ·
> Ereignis: „**Konstruktion**, danach fünf Taktauslösungen über den Testzugang
> und eine gültige Anfrage in der Temp-Wurzel" · Zusage: „kein Takt gestartet
> (Startzähler 0), **0 Fassadenaufrufe**, keine Antwort; der Grund steht nur
> instanzlokal (Testzugang); State-Bytes, Parameter und Host-Dirty unverändert
> wie M-37; kein Logeintrag auf dem Audio-Thread; der Ausfall zeigt sich im
> Runner als fehlende Antwort der Rolle innerhalb der Frist (M-56)" ·
> Rotbeweis: „Wurzel ungeprüft benutzt → der Takt startet, die Fassade zählt
> Existenzprüfungen größer 0 → rot".

**Gegenstand — F-14, `:374`** (die Regel, die den Zähler füllt):

> „Je Takt genau eine Existenzprüfung (Dateiattribute von `anfrage.json`), kein
> Öffnen, kein Verzeichnislesen. **Jeder** Dateizugriff des Briefkastens läuft
> durch eine Dateisystem-Fassade — **Existenz- und Attributprüfung (auch
> Reparse, F-11)**, Lesen, exklusives Anlegen, Schreiben und Spülen, Größe
> lesen, Umbenennen ohne Ersetzen (F-8); **sie zählt Existenzprüfungen**,
> Leseöffnungen, gelesene Bytes, Anlegeversuche, Schreibvorgänge,
> Umbenennungen, Ordneranlagen und Löschungen … Ein Zugriff an ihr vorbei …
> ist ein Befund, weil ihn kein Zähler sieht (§10.3 (d))."

**Gegenstand — wann wird der Reparse-Punkt geprüft?** F-11 selbst (`:364`) sagt
**nichts** über den Zeitpunkt:

> „Ist `anfrage.json`, `antwort\`, `diagnose\`, `nakama\` oder `evenacadia\`
> ein Reparse-Punkt (Symlink oder Junction), liest und schreibt die Instanz
> nichts; `anfrage.json` wird geöffnet, ohne einem Link zu folgen (§7.1 T-6)."

Den Zeitpunkt setzen zwei andere Stellen, und sie setzen ihn verschieden:

- **F-1, `:354` — beim Start:** „oder ist **eine beim Start vorhandene Ebene ab
  `evenacadia\`** ein Reparse-Punkt (F-11), startet kein Takt; der Grund bleibt
  instanzlokal (§10.2 P-2; M-76 Broker, M-77 Plugin)."
- **M-49, `:560` — vor jedem Lesen und Schreiben:** Reihenfolge und Frist:
  „**Reparse-Prüfung von `evenacadia\` bis `anfrage.json` vor Lesen und
  Schreiben**".

Beides zusammen ergibt die einzige regelkonforme Buchführung:

| Zeitpunkt | Zugriffe über die F-14-Fassade |
|---|---|
| Start (F-1) | 1 bis 4 Attributprüfungen — die beim Start vorhandenen Ebenen `evenacadia\`, `nakama\`, `diagnose\`, `antwort\`; von oben geprüft endet der zweite Durchlauf von M-77 nach der ersten (`evenacadia\` ist der Reparse-Punkt) |
| Takt ohne Anfrage (F-14) | genau 1 (Dateiattribute von `anfrage.json`, die den Reparse-Zustand der Datei mittragen) |
| Takt mit Anfrage (M-49) | zusätzlich die Ebenenprüfung vor Lesen und Schreiben |

**Status: BESTÄTIGT**, für den zweiten Durchlauf zwingend: `evenacadia\` als
Reparse-Punkt zu erkennen verlangt mindestens eine Attributprüfung, F-14 zählt
sie, das Ereignis von M-77 schließt die Konstruktion ausdrücklich ein — ein
regelkonformer Bau steht damit vor dem ersten Takt bei Zähler ≥ 1 und wird an
„0 Fassadenaufrufe" rot. Das ist dieselbe Fehlerklasse wie §10.1 Befund 5
(„wirft einen korrekten Bau auf Exit 4"), nicht bloß eine unscharfe Formulierung.

**Verschärfung gegenüber dem Prüfer (erster Durchlauf).** Die Zeile lässt offen,
ob die genannte „Wurzel-Fassade" Teil der gezählten F-14-Fassade ist. Ist sie
es, hat auch der **erste** Durchlauf (`windowsLocalAppData` liefert keinen
vorhandenen Ordner) mindestens eine gezählte Existenzprüfung, und „0
Fassadenaufrufe" ist auch dort für einen regelkonformen Bau unerreichbar. Ist
sie ein eigener, ungezählter Prüfnaht, widerspricht das F-14 („**Jeder**
Dateizugriff … Ein Zugriff an ihr vorbei … ist ein Befund"). Die Zeile muss
sagen, welches gilt; der Prüfer nennt nur den zweiten Durchlauf.

**Zweiter Fundort derselben Ursache (vom Prüfer nicht genannt): M-23, `:521`.**
Zusage: „je Prozessor **genau 600 Existenzprüfungen**, 0 Leseöffnungen …" bei
Ereignis „600 Takte", Zustand „Gen- und Probeeq-Prozessor mit Briefkasten auf
Temp-Wurzel und Dateisystem-Fassade". Zählt die Fassade ab der Konstruktion,
liegt sie mit den Startprüfungen bei 601 bis 604, und auch diese Zeile wirft
einen korrekten Bau rot. Die Broker-Schwester M-41 (`:546`) ist bereits sauber
— sie sagt ausdrücklich „**laufender** Briefkasten-Griff … Die Zeile misst nur
den laufenden Griff; den Startpfad ohne `LOCALAPPDATA` misst M-76". M-23 fehlt
genau dieser Satz. Ebenso trägt M-76 (`:547`, die Rust-Schwester von M-77)
**keine** Zusage über Fassadenzähler, sondern misst die Registerfehlerliste —
die Überbestimmung steht allein in M-77.

**Einordnung (Vorschlag): DEFEKT** (Zusage widerspricht der eigenen Feinheit
F-14; ein regelkonformer Bau wird an der Zeile rot — Verstoß gegen §10.2 P-2
insofern, als der Startpfad „nichts, was ein anderer Weg lesen kann" ändern
soll, der instanzlokale Testzähler aber genau das Gegenteil zusagt).

**Schließende Änderung (Vorschlag).** M-77 weist die beiden Fenster getrennt aus:

1. **Startfenster:** Durchlauf 1 (keine Wurzel) — Zahl der Fassadenaufrufe
   ausdrücklich benannt (0, wenn die Wurzelbestimmung außerhalb der F-14-Fassade
   liegt und die Zeile das sagt; sonst 1); Durchlauf 2 (Reparse auf
   `evenacadia\`) — **genau eine Attributprüfung**, danach Abbruch, keine
   weitere Ebene geprüft (die Prüfung läuft von `evenacadia\` abwärts und endet
   am ersten Reparse-Punkt), 0 Leseöffnungen, 0 Anlegeversuche, 0
   Schreibvorgänge, 0 Ordneranlagen, 0 Löschungen.
2. **Taktfenster:** die fünf Taktauslösungen ergeben **0 zusätzliche
   Fassadenaufrufe jeder Art** (Differenz gegen den Zählerstand nach der
   Konstruktion) und keine Antwort, obwohl eine gültige Anfrage in der
   Temp-Wurzel liegt.
3. **Rotbeweis** bleibt inhaltlich, misst aber die Differenz: „Wurzel
   beziehungsweise Ebenen ungeprüft benutzt → der Takt startet, das Taktfenster
   zählt Existenzprüfungen größer 0 → rot"; zweiter Rotbeweis für den
   Startpfad: „Ebenenprüfung an der Fassade vorbei (etwa `juce::File::isSymbolicLink`
   direkt) → das Startfenster zählt 0 statt 1 → rot" (deckt §10.3 (d), „ein
   ungezählter Startzugriff ist kein zulässiger Ausweg").
4. **M-23** bekommt denselben Schnitt wie M-41: „die Zeile misst nur die 600
   Takte; die Startprüfungen nach F-1 stehen in M-77" — oder die Zusage lautet
   „600 Existenzprüfungen **im Taktfenster** (Differenz gegen den Stand nach der
   Konstruktion)".
5. **F-1** nennt die Zahl der Startprüfungen ausdrücklich (Ebenen ab
   `evenacadia\`, von oben, Abbruch am ersten Reparse-Punkt), damit M-77 und
   M-23 dieselbe Zahl meinen; **F-11** bekommt den Zeitpunkt, den F-1 und M-49
   heute getrennt tragen (Start: Ebenen; je Takt: Attribute von `anfrage.json`;
   vor Lesen und Schreiben: Ebenen erneut).

---

## Befund 6 — „Bereite den erneuten Antwortpfad im FIFO-Test vor"

**Wörtlich (Urteil):** `[P2] Bereite den erneuten Antwortpfad im FIFO-Test vor —
C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:567-567 — DEFEKT an
M-51: Nach den 257 Antworten liegt die Datei der ältesten Kennung weiterhin vor,
denn F-9 verbietet der Instanz das Löschen. Bei erneuter Vorlage bleiben
Anfragekennung, PID, Instanz- und Laufzeitkennung identisch; anders als vor dem
Fix gibt es nach F-3 keine Suffixleiter mehr. Das vorgeschriebene Umbenennen
ohne Ersetzen aus F-8 scheitert deshalb am alten Antwortnamen, obwohl die
Kennung korrekt aus dem Ring verdrängt wurde. Die zugesagte erneute Antwort kann
so nicht entstehen; lediglich die vorhandene Datei zu zählen würde den FIFO-Pfad
nicht beweisen. Der Test-Anfragende muss alte Antworten vor der Wiederholung
ausdrücklich entfernen oder die Zusage auf den beobachteten neuen Schreibversuch
ausrichten, ohne den Geltungsrahmen von P-1 (NAK-286.md#L1029) zu ändern.`

**Gegenstand — M-51, `docs/beweise/NAK-286.md:567`:**

> Zustand: „257 verschiedene gültige Kennungen nacheinander — die 257.
> verdrängt die älteste aus dem FIFO … —, danach die zweitjüngste und die
> älteste erneut" · Zusage: „Geltungsrahmen nach §10.2 P-1: eine Kennung im
> Ring bekommt nie eine zweite Antwort — die zweitjüngste erneut vorgelegt: 0
> Anlegeversuche an der Fassade; **eine verdrängte Kennung ist für die Instanz
> neu — die älteste erneut vorgelegt: eine Antwort**; Ring fest 256 Einträge,
> nach dem Anlauf keine Allokation je Takt; im Runnerbetrieb legt der Runner
> keine Kennung zweimal vor (F-2)" · Rotbeweis: „(1) Ringgröße: Ring mit einem
> Eintrag → die zweitjüngste Kennung wird erneut beantwortet → rot; (2)
> Einmaligkeit: Ring vor dem Schreiben nicht gefragt → zweite Antwort auf die
> noch gemerkte zweitjüngste Kennung, Anlegeversuche 1 → rot".

**Gegenstand — F-9, `:362`:**

> „Eine Kennung kommt erst nach erfolgreichem Umbenennen in den Kennungsring.
> **Instanzen löschen weder `anfrage.json` noch eine Antwort** (Zähler
> `loeschen` der Fassade bleibt in jedem Fall 0). Der Runner legt keine Kennung
> zweimal vor und **räumt zuerst die Anfrage, dann die Antworten ab** (F-2,
> M-55); damit sieht keine Instanz im Runnerbetrieb eine verdrängte Kennung
> wieder."

**Gegenstand — F-8 Schritt 5, `:361`:** „ohne Ersetzen in den Antwortnamen
umbenennen (Windows: `MoveFileExW` ohne `MOVEFILE_REPLACE_EXISTING`, Muster
`broker/src/persistenz.rs:78-109`)".

**Quelle an HEAD — scheitert das Umbenennen wirklich?** Ja.
`broker/src/persistenz.rs:72-75` und `:158-166`:

```rust
fn ist_bereits_vorhanden(fehler: &io::Error) -> bool {
    fehler.kind() == io::ErrorKind::AlreadyExists
        || cfg!(windows) && matches!(fehler.raw_os_error(), Some(80 | 183))
}
…
    match verschieben(&temp.pfad, ziel, false) {
        Ok(()) => Ok(true),
        Err(e) if ist_bereits_vorhanden(&e) => Ok(false),
```

Ein belegter Zielname ist der dokumentierte Ausgang „saubere Namenskollision",
kein Ersetzen. Nach F-3 (`:356`) ist er „ein gescheiterter Schreibversuch nach
F-13, nie ein Ersetzen" — also einer von drei Versuchen, danach Kennung
aufgegeben und Fehlerzähler plus 1 (F-13, `:373`).

**Vorher-Stand, `c6c59170:docs/beweise/NAK-286.md:547`:** Zusage „die 256
jüngsten beantworteten Kennungen bekommen nie eine zweite Antwort; **die
verdrängte älteste bekommt eine zweite**". Im alten Namensmodell (Suffixleiter
`.2` bis `.9`, siehe M-33 vorher) konnte diese zweite Antwort als eigene Datei
entstehen. Die Nacharbeit hat die Suffixleiter gestrichen (§11.1 Punkt 7,
`:1071`) und die Zusage — inzwischen um den P-1-Geltungsrahmen erweitert —
stehen gelassen.

**Status: BESTÄTIGT.** Für die zweite Vorlage der ältesten Kennung sind alle
fünf Namensglieder identisch (dieselbe Instanz, derselbe Prozess, dieselbe
Kennung); die erste Antwortdatei liegt noch, weil F-9 das Löschen verbietet und
im Test niemand abräumt. Ein regelkonformer Bau erreicht Schritt 5 und
scheitert dort — die Zusage „eine Antwort" ist für ihn unerfüllbar. Wird
stattdessen nur „eine Datei mit diesem Namen existiert" gemessen, besteht der
Fall **vakuum**, weil die Datei aus dem ersten Durchgang stammt: der FIFO-Pfad
wäre nicht bewiesen. Beide Lesarten sind Defekte, die zweite die gefährlichere
(stiller Grünstand).

**Verlangt M-51, dass die Bühne die alten Antworten entfernt?** Nein. Der
Zustand der Zeile nennt nur die Kennungsfolge; von Abräumen steht nichts.
**P-1, `:1029`,** sagt dazu: „Der Runner legt eine Kennung nie zweimal vor
(F-2, 128 Bit Zufall) und **räumt zuerst die Anfrage, dann die Antworten ab**."
Das ist eine Pflicht des **Anfragenden**, nicht der Instanz — und im Testfall
`kennungsring_fifo_256` (B30/A4) ist die Testbühne der Anfragende. Die Bühne
zum Abräumen zu verpflichten, wendet P-1 an, statt seinen Geltungsrahmen zu
ändern; das Namensmuster dafür steht bereits in M-55 (`:576`): „nur
Namensmuster `<32 hex>.<rolle>.<pid>.<16 hex>.<32 hex>.json` und Namen mit
`.tmp-`", und zwar „zuerst die Anfrage, dann die Antworten".

**Welche Zusage fällt, und welche Beobachtung trägt sie?** Die Zusage ist „eine
verdrängte Kennung ist für die Instanz **neu**" — also: der Ring hält sie nicht
mehr zurück. Die Beobachtung, die genau das misst und nicht am Dateisystemrest
hängt, ist der **Anlegeversuch an der Fassade**, symmetrisch zur
Schwesterzusage:

- zweitjüngste (noch im Ring) erneut vorgelegt → **0 Anlegeversuche** (steht
  schon so in der Zeile);
- älteste (verdrängt) erneut vorgelegt → **1 Anlegeversuch und 1 Schreibvorgang**
  für diese Kennung.

Räumt die Bühne vorher ab, kommt die stärkere Beobachtung dazu: eine Datei mit
dem Zielnamen und **neuem Inhalt** (anderes `erzeugt_utc`), Fehlerzähler 0.
Ohne Abräumen entsteht dagegen die Kaskade aus F-8/F-13 (Temp bleibt liegen,
zwei weitere Takte scheitern schon am exklusiven Anlegen, Fehlerzähler 1) —
sie kontaminiert die Zeile mit einem Nebenschauplatz, den M-33 (d) und M-29
bereits messen.

**Einordnung (Vorschlag): DEFEKT** (Zusage für einen regelkonformen Bau nicht
erfüllbar beziehungsweise nur vakuum bestätigbar; §10.3 (c), Leitsatz `:1045`).

**Schließende Änderung (Vorschlag).** Im Zustand von M-51 ergänzen: „die
Testbühne ist der Anfragende und räumt vor jeder Wiederholung **zuerst die
Anfrage, dann die Antworten** ab (P-1, Namensmuster wie M-55, einschließlich
`.tmp-`)". In der Zusage die Beobachtung an die Fassade binden: „die älteste
erneut vorgelegt: **1 Anlegeversuch und 1 Schreibvorgang an der Fassade**, danach
eine Antwortdatei mit neuem `erzeugt_utc`; Löschzähler der Instanz bleibt 0 (das
Abräumen tut die Bühne, nicht die Instanz — F-9)". Rotbeweis (1) bleibt
(Ringgröße 1), Rotbeweis (2) bleibt (Ring nicht gefragt); ein dritter ist nicht
nötig, weil die neue Beobachtung Rotbeweis (1) schärft: mit Ringgröße 1 steht die
zweitjüngste bei 1 statt 0 Anlegeversuchen. Ergänzend in **T-2** (`:916`) einen
Satz, dass der Test-Anfragende dieselbe Abräumpflicht trägt wie der Runner —
sonst wiederholt sich der Fehler in jedem künftigen Fall, der eine Kennung
erneut vorlegt.

---

## Befund 7 — „Verwende für M-53(c) einen wirksamen Größen-Rotbeweis"

**Wörtlich (Urteil):** `[P2] Verwende für M-53(c) einen wirksamen
Größen-Rotbeweis — C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:569-569
— DEFEKT im neuen Runner-Rotbeweis: Eine 0-Byte-Datei wird nach Entfernung
allein des Größenriegels weiterhin nicht als Antwort gewertet, weil sie kein
JSON-Objekt und damit keinen gültigen Umschlag nach F-4 (NAK-286.md#L357)
enthält. Die nachgelagerte Verarbeitung verwirft sie weiterhin; der behauptete
Zustandswechsel tritt nicht ein. Die korrigierten Injektionen (a) und (b) tragen
ihre Zusagen, aber (c) braucht beispielsweise eine ansonsten gültige übergroße
Antwort oder einen Zähler für das unerlaubte Lesen vor der Größenprüfung, damit
tatsächlich der Größenriegel falsifiziert wird.`

**Gegenstand — M-53, `docs/beweise/NAK-286.md:569`, Lage (c):**

> Zustand: „… (c) Runner: im Antwortordner liegen eine Datei
> `<Antwortname>.tmp-<pid>`, eine `.json` mit 0 Bytes und eine mit 17 MiB" ·
> Zusage: „(c) der Runner ignoriert Namen mit `.tmp-` und **verwirft `.json`
> unter 2 Bytes oder über 16 MiB als unbrauchbar (verfehlt, nie gewertet)**" ·
> Rotbeweis: „(c) **Größenriegel im Runner entfernt → die 0-Byte-Datei wird
> gewertet → rot**" · Test: „Selbsttest `antwortgroessen` für (c) (A35)".

**Gegenstand — F-4, `:357`:** „ein Umschlag mit genau den Pflichtschlüsseln
`format`, `anfrage_id`, `rolle`, `instanz_id`, `laufzeit_id` …, `pid`,
`erzeugt_utc`, `version`, `snapshot`, `frame`, `aggregat`, `gruende`
(`additionalProperties: false`)". Eine 0-Byte-Datei ist kein JSON-Objekt und
erfüllt keinen einzigen Pflichtschlüssel.

**Gegenstand — M-56, `:577`** (die Stelle, die den Umschlag tatsächlich liest):

> „gewertet wird je Rolle `gen` und `probeeq` genau die Antwort mit aktueller
> Kennung und Diagnose-PID …; zwei Antworten derselben Rolle und PID mit
> verschiedener **`instanz_id`** → beide roh, Schritt verfehlt „mehrdeutig";
> zwei Antworten derselben Rolle und PID mit gleicher `instanz_id` und
> verschiedener **`laufzeit_id`** … sind ebenso mehrdeutig".

`instanz_id` und `laufzeit_id` mit Unterstrich sind Umschlagfelder (F-4), nicht
Namensglieder (F-3 schreibt `instanz16` und `laufzeit32`). Die Auswahl des
Runners liest also den Umschlag; eine Datei, die sich nicht als JSON-Objekt
lesen lässt, kann diese Auswahl nicht bestehen — mit oder ohne Größenriegel.

**Quelle an HEAD — was tut der Runner heute?** Nichts davon. `tools/fl/szenario.py`
(185 Zeilen) kennt weder Briefkasten noch Antwortordner noch Größen: der
Bezeichner `antwort` steht dort ausschließlich für die MCP-Kommandoantwort
(`:57-63`, `:66-72`, `:124-125`), es gibt keine `lokal.`-Aktion, keinen
Dateizugriff auf `%LOCALAPPDATA%\evenacadia\nakama\diagnose\` und keinen
`--selbsttest`; die Exitcodes stehen bei `:38`
(`EXIT_OK, EXIT_DATEI, EXIT_VORAUSSETZUNG, EXIT_VERFEHLT = 0, 2, 3, 4`).
`tools/fl/laufzeit.ps1` (331 Zeilen) enthält keine Zeile zu Antwortordner oder
Briefkasten. Beides bestätigt das Etikett „heute nicht messbar": die
Runner-Hälfte von M-53 entsteht erst in Etappe 4 (Bauplan §5.7 Punkt 2, `:826`:
„Antwortauswahl und Größenriegel (M-53, M-56, M-57)"). Es gibt an HEAD keinen
Code, der den Rotbeweis (c) heute wirksam oder unwirksam machen könnte — die
Prüfung ist eine reine Spezifikationsprüfung, und sie fällt gegen F-4 und M-56.

**Status: BESTÄTIGT.** Die Mutation „Größenriegel entfernt" ändert für die
0-Byte-Datei nichts Beobachtbares: sie wird danach zwar nicht mehr wegen ihrer
Größe verworfen, aber weiterhin wegen ihres Umschlags — der Schritt endet in
beiden Fällen „verfehlt". Damit unterscheidet der Beweis Mutation und Bau
nicht; er misst nur, dass irgendein Riegel greift. *Verschärfung:* Die Lage (c)
enthält überhaupt keine gültige Antwort, ihre Zusage endet also ohnehin bei
„verfehlt" — der Rotbeweis müsste die **Auswahl** beobachten („welche Datei
wurde zur Antwort der Rolle"), nicht das Urteil; genau das leistet er nicht,
weil die Auswahl nach M-56 den Umschlag liest. *Einschränkung gegenüber dem
Prüfer:* die 17-MiB-Hälfte derselben Zusage ist von dem Argument **nicht**
betroffen — eine übergroße, sonst gültige Antwort ist ein echter
Unterscheidungsfall. Der Prüfer nennt sie nur als Vorschlag, nicht als bereits
in der Zeile stehende Lage; in der Zeile steht sie tatsächlich nicht als
*gültige* Datei, nur als „eine mit 17 MiB".

**Was falsifiziert den Größenriegel wirklich?** Zwei Wege, die zusammen beide
Hälften der Zusage decken:

1. **Übergroße, sonst gültige Antwort** (deckt die 16-MiB-Obergrenze): ein
   vollständiger, schemakonformer Umschlag nach F-4 mit korrektem Namen nach
   F-3, durch JSON-Leerraum auf über 16 MiB gebracht (Leerraum zwischen Tokens
   ist gültiges JSON und lässt den Umschlag unverändert). Mit Riegel: verworfen,
   Rolle ohne Antwort, Schritt verfehlt. **Mutation** „Größenriegel entfernt" →
   die Datei wird geparst und als Antwort der Rolle **gewertet**, der Schritt
   besteht → rot. Hier kippt der Zustand wirklich.
2. **Zähler für das Lesen vor der Größenprüfung** (deckt beide Grenzen und
   insbesondere die 2-Byte-Untergrenze): die Datei-Attrappe des Selbsttests
   (`laufzeit.ps1 -Selbsttest` mit Attrappe für Dateien, §5.7 Punkt 1, `:825`;
   `szenario.py --selbsttest`, Punkt 2) zählt Leseöffnungen je Datei. Zusage:
   für die `.tmp-`-Datei, die 0-Byte- und die 17-MiB-Datei **0 Leseöffnungen** —
   der Runner entscheidet an Name und Größe, ohne Inhalt zu lesen. **Mutation**
   „Größenriegel entfernt" → Leseöffnungen größer 0 → rot. Das ist die
   Runner-Entsprechung zur F-14-Fassade des Produkts und die einzige
   Beobachtung, die der 2-Byte-Untergrenze überhaupt einen eigenen Inhalt gibt.

Ohne (2) hat die Untergrenze „unter 2 Bytes" keine eigene Zusage: sie ist durch
die Umschlagprüfung nach F-4 vollständig gedeckt und damit ein redundanter
Riegel. Dann gehört sie entweder gestrichen oder mit (2) begründet.

**Einordnung (Vorschlag): DEFEKT** (der Beweisweg trägt die Zusage nicht; §4
„Dateisystem und Fehlerinjektion" `:479`, Leitsatz `:1045`). Kein Bruch einer
Regel R-286-n und keine Lücke — die Zusage steht, nur ihr Beweis fällt neben
ihr.

**Schließende Änderung (Vorschlag).** Lage (c) von M-53 bekommt eine vierte
Datei und zwei Beobachtungen: Zustand „(c) Runner: im Antwortordner liegen eine
Datei `<Antwortname>.tmp-<pid>`, eine `.json` mit 0 Bytes, eine mit 17 MiB und
**eine gültige, mit JSON-Leerraum auf über 16 MiB gebrachte Antwort mit
korrektem Namen und Umschlag nach F-3/F-4**". Zusage: „der Runner ignoriert
Namen mit `.tmp-`; er verwirft `.json` unter 2 Bytes oder über 16 MiB **ohne sie
zu lesen** (Leseöffnungen an der Datei-Attrappe 0), auch die sonst gültige
übergroße; keine von ihnen wird je die Antwort einer Rolle (verfehlt, nie
gewertet)". Rotbeweis (c) zweiteilig: „(c1) Größenriegel entfernt → die gültige
übergroße Antwort wird gewertet, der Schritt besteht → rot; (c2) Größenriegel
entfernt → die Attrappe zählt Leseöffnungen für die 0-Byte- und die 17-MiB-Datei
→ rot". Der Selbsttest `antwortgroessen` (A35) trägt beide; Bauplan §5.7 Punkt 2
(`:826`) nennt den Größenriegel bereits, hier kommt nur „ohne Lesen" dazu.

---

## Gemeinsame Ursache

Der Prüfer sieht bei 4, 6 und 7 dieselbe Ursache: „Rotbeweis aus dem alten
Namens-/Lesemodell übernommen, obwohl der Fix das Modell geändert hat". Der
Vorher-Nachher-Vergleich stützt das **für 4 und 6** und **nicht für 7**.

**Bestätigt für Befund 4 und Befund 6 — und zwar belegbar aus dem Fixdiff.**
Vor `7d9e8a79` rechneten zwei Instanzobjekte mit gleicher persistenter Kennung
**denselben** Antwortnamen; die Suffixleiter `.2` bis `.9` trennte sie (M-33
vorher, `c6c59170:…:514`: „zwei Dateien, die zweite mit Suffix `.2`"). In diesem
Modell funktionierten beide jetzt gefallenen Sätze:

- M-33 (2) „Umbenennen mit Ersetzen → in (b) bleibt eine Datei" — weil beide
  Instanzen denselben Basisnamen ansteuerten;
- M-51 „die verdrängte älteste bekommt eine zweite [Antwort]" — weil die zweite
  Antwort derselben Instanz auf dieselbe Kennung als `.2` entstehen konnte.

Der Fix (Laufzeitkennung im Namen, keine Suffixleiter, F-3 `:356`, §11.1 Punkt 7
`:1071`) hat beide Voraussetzungen entfernt und beide Sätze wörtlich stehen
lassen. Die Nacharbeit hat die Ursache selbst benannt — §11.2 (c), `:1114-1117`:
„M-25 brauchte dafür einen neuen Rotbeweis: mit der Laufzeitkennung im Namen
scheitert eine zweite Antwort am Umbenennen ohne Ersetzen, der alte Beweis
„Datei mit Suffix entsteht" war nicht mehr herstellbar" — und sie hat diese
Prüfung auf M-25 angewandt, aber **nicht** auf M-33 und M-51, die dieselbe
Namensmechanik tragen. Das ist eine unvollständig ausgerollte Ursache, nicht ein
neuer Fehler: Ursache §10.3 (c) („Eindeutigkeit über verdrängende Behälter statt
über vorhandene Produktmechanismen") ist der Ort, an dem die Nacharbeit den
Rundlauf abgebrochen hat.

**Widerspruch bei Befund 7.** Der Rotbeweis (c) ist **nicht** übernommen — es
gab ihn vorher nicht. `c6c59170:…:549` trägt genau einen Rotbeweis für die ganze
Zeile M-53: „direktes Schreiben in den Zielnamen → die Injektion hinterlässt eine
`.json` mit 0 Bytes → rot"; die Runner-Hälfte hatte keinen. Die Nacharbeit hat
(c) **neu erfunden** (§11.1 Punkt 12, `:1076`: „Lage (c): Runner-Hälfte wie
bisher, mit eigenem Rotbeweis") und dabei die Beobachtung an einer Stelle
angesetzt, die die Umschlagprüfung nach F-4 ohnehin abfängt. Das ist Ursache
§10.3 (d) („Beweis beziehungsweise Injektion neben der Zusage"), nicht (c): ein
neuer Beweis am falschen Beobachtungspunkt, dazu ein Riegel (die 2-Byte-Grenze),
der ohne eine eigene Beobachtung inhaltlich leer ist.

**Befund 5 steht für sich** und gehört zu keiner der beiden Gruppen. M-77 ist
eine **neue** Zeile der Nacharbeit (§11.3, `:1165`), und ihr Fehler ist eine
Überbestimmung: die Zusage nennt einen Zähler („0 Fassadenaufrufe"), den die
eigene Feinheit F-14 im gemessenen Fall zwangsläufig füllt. Verwandt ist sie mit
§10.1 Befund 5 („Δ aktiv ≤ Δ gesamt wirft einen korrekten Bau auf Exit 4") — die
Klasse „Zusage, an der ein regelkonformer Bau rot wird" —, und sie hat mit M-23
(`:521`) einen zweiten, bisher ungemeldeten Fundort; die Rust-Schwester M-76
(`:547`) und M-41 (`:546`) sind bereits sauber geschnitten.

**Nebenbefund (Form, außerhalb der vier Befunde).** Zeile `:1045` endet mit
einem alleinstehenden `|` hinter dem Leitsatz — ein Rest der darüberstehenden
Tabelle. Kein inhaltlicher Befund, aber die Zeile, auf die vier Urteile
verweisen; eine Nacharbeit, die §10 ohnehin nicht anfasst, sollte ihn nur
mitnehmen, wenn sie den Abschnitt aus einem anderen Grund berührt.

---

## Übersicht

| Befund | Kurzname | Status | Einordnung (Vorschlag) | Schließende Änderung (Kurz) |
|---|---|---|---|---|
| 4 | Zielnamenskollision für M-33(2) (`:532`) | BESTÄTIGT | DEFEKT (Beweisweg trägt die Zusage nicht) | Rotbeweis (2) von Lage (b) auf eine neue Lage (d) „Zielname bereits durch eine Markendatei belegt" verlegen; Beobachtung: Markendatei byteweise erhalten, Löschzähler 0, eine Temp-Datei `<Antwortname>.tmp-<pid>`, nach drei Takten Fehlerzähler 1; Mutation „Umbenennen mit Ersetzen" ändert den Hash der Markendatei → rot; Lage (b) trägt Rotbeweis (1) |
| 5 | Start- und Taktzugriffe in M-77 (`:538`) | BESTÄTIGT (zweiter Durchlauf zwingend; erster Durchlauf offen, weil die Zeile nicht sagt, ob die Wurzel-Fassade gezählt wird) | DEFEKT (Zusage widerspricht F-14; ein regelkonformer Bau wird rot) | Startfenster und Taktfenster getrennt ausweisen: Start Durchlauf 2 = genau 1 Attributprüfung (`evenacadia\`, Abbruch am ersten Reparse-Punkt), Takte = 0 zusätzliche Aufrufe als Differenz; zweiter Rotbeweis „Ebenenprüfung an der Fassade vorbei → Startfenster zählt 0 statt 1"; F-1 nennt die Zahl der Startprüfungen, F-11 den Zeitpunkt; M-23 bekommt denselben Schnitt wie M-41 |
| 6 | Erneuter Antwortpfad im FIFO-Test (`:567`) | BESTÄTIGT (Zusage für einen korrekten Bau unerfüllbar; naiv gemessen vakuum grün) | DEFEKT | Testbühne als Anfragende verpflichten, vor jeder Wiederholung zuerst die Anfrage, dann die Antworten abzuräumen (P-1, Namensmuster wie M-55 samt `.tmp-`); Zusage an die Fassade binden: verdrängte Kennung erneut → 1 Anlegeversuch und 1 Schreibvorgang, danach Antwort mit neuem `erzeugt_utc`, Löschzähler der Instanz 0; T-2 nennt die Abräumpflicht der Bühne |
| 7 | Wirksamer Größen-Rotbeweis für M-53(c) (`:569`) | BESTÄTIGT für die 0-Byte-Hälfte (F-4 fängt sie ohnehin ab); die 16-MiB-Hälfte ist tragfähig, steht aber nicht als gültige Datei in der Lage | DEFEKT (Beweisweg trägt die Zusage nicht) | Lage (c) um eine gültige, mit JSON-Leerraum auf über 16 MiB gebrachte Antwort ergänzen; Zusage „verwirft ohne zu lesen" mit Leseöffnungszähler der Datei-Attrappe; Rotbeweis zweiteilig — (c1) Riegel entfernt → die gültige übergroße Antwort wird gewertet → rot; (c2) Riegel entfernt → Leseöffnungen für 0-Byte- und 17-MiB-Datei größer 0 → rot; sonst die 2-Byte-Grenze streichen |
| — | Gemeinsame Ursache | 4 und 6 BESTÄTIGT (altes Namensmodell, im Fixdiff belegbar: `c6c59170:…:514`, `:547`); 7 WIDERSPRUCH (der Rotbeweis ist neu, `c6c59170:…:549` hatte keinen); 5 einzeln | — | Die Prüfung aus §11.2 (c), die für M-25 gemacht wurde („der alte Beweis „Datei mit Suffix entsteht" war nicht mehr herstellbar"), über **alle** Zeilen ziehen, die Antwortnamen, Kollision oder Ersetzen nennen (M-24, M-25, M-26, M-29, M-33, M-42, M-43, M-48, M-51, M-55, M-56); für 7 gilt §10.3 (d), für 5 die Klasse „Zusage, an der ein regelkonformer Bau rot wird" (zweiter Fundort M-23) |
