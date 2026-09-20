VALIDIERUNG: V-1 BESTÄTIGT · V-2 BESTÄTIGT · V-3 BESTÄTIGT

# NAK-311 — Validierung der Matrixprüfung 5 (Etappe 5, Planschritt S25j)

Frischer, lesender Validierer. Auftrag
`docs/beweise/roh/NAK-311-matrixpruefung-5-validierung-auftrag.txt`.
**HEAD zu Beginn `1cfbc3f916bdcda7234b7cec0cda2e8dd818368d`.** Nur lesende
Kommandos (`git rev-parse`, `git diff`, `git show`, `git status`, `grep`/`rg`,
zeilenweises Lesen). Kein Bau, kein Test, kein FL Studio, kein Repo-Skript,
kein Python. Geschrieben ist genau diese Datei.

`docs/beweise/NAK-311.md` ist seit `d4b40aa6` nur um §45 gewachsen
(`git diff --stat d4b40aa6 HEAD -- docs/beweise/NAK-311.md` = 58 Einfügungen,
1 Löschung, alle ab Zeile 8185). Die Zeilennummern von §38 bis §41
(`:5836-7549`) gelten deshalb am HEAD unverändert wie im Fixdiff. Der Code ist
seit `34953955` unverändert (`git diff --stat 34953955 HEAD -- eq-copilot/` ist
für die vier Testdateien und `dsp/` leer).

---

## V-1 — D-1 (a) und (d): die „vollständige" Aufzählung der Bestandsfälle

### 1. Die betroffenen Sätze des Manifests, wörtlich, mit Zeile am HEAD

- §40.3 `:6844-6846`: „**Die Bestandsfälle auf dem Rampenweg, vollständig**
  (nachgemessen in der Matrixnacharbeit 3, D-1 (a); gezählt über jede Zuweisung
  an `kGainDb`, `kFreqHz` und `kQ` und jeden `belege`-Aufruf in den vier
  Testdateien)."
- §40.3 `:6847-6850`: „Ein Rampenfall verlangt zwei Publikationen desselben
  Pfades, beide mit `eqEngagiert` und ohne `hardBypass`, gleicher Samplerate und
  gleicher Topologie je Slot (`eq-copilot/plugin/dsp/DspProgramm.cpp:207-227`);
  alles andere geht schon heute über den Crossfade und ist von den Kriterien
  unberührt."
- §40.3 `:6862-6863`: „Kein weiterer Fall der vier Testdateien publiziert zwei
  rampenkompatible Programme mit geändertem `freq_hz`, `q` oder `gain_db`."
- §40.3 `:6866-6868`: „Alle sieben Fälle liegen unter den drei Kriterien; der
  Satz belegt es mit dem Zeilenvergleich vorher/nachher."
- §40.3 `:6872-6873`: „Alle drei liegen in `TransactionTestMain.cpp` (Bein B7),
  und **keine von ihnen vergleicht Audio**: `Stand` bereitet zwar einen echten
  `DspKern` vor (`eq-copilot/plugin/tests/TransactionTestMain.cpp:130-144`),
  aber `fahre` (`:156-161`) ruft nie `verarbeite`."
- §40.3 `:6888`: „Weitere Fälle über einem Kriterium gibt es in den vier
  Testdateien nicht".
- §40.3 `:6889-6892`: „`SondeNullTestMain.cpp:202-203` gegen `:662-663` erreicht
  15,0 dB (Gain) und Faktor 1,41 (Güte) und bleibt darunter, und die Vorschau in
  `TransactionTestMain.cpp:2222` erreicht 18,0 dB, liegt aber auf dem
  **Candidate**-Pfad, dessen Merkzettel leer startet."
- §40.3 „Golden- und Fixture-Regel" `:6969-6971`: „die **sieben** oben
  aufgezählten Rampenfälle liegen unter den Kriterien — der engste ist
  `uebernahme_nur_am_aeusseren_blockrand` mit 18,0 dB gegen 20,0 —".
- §40.3 „Risiken und Grenze" `:6979-6981`: „die zwei Aufzählungen oben nennen
  **jeden** Fall der vier Testdateien mit Fundstelle und Wert — sieben auf dem
  Rampenweg unter den Kriterien, drei ohne Audio darüber —".
- M-103 (§39.1, `:6300`): „Die vollständige Aufzählung der Bestandsfälle auf dem
  Rampenweg steht in §40.3".
- §38.1 `:5932-5933`: „Sie sind nicht die größten Bestandsfälle auf dem
  Rampenweg; die vollständige Aufzählung steht in §40.3."

### 2. Die Codebehauptungen des Prüfers an der Quelle

**Fall 1 — `TransactionTestMain.cpp:2069` → `:2081`.** Jede Angabe stimmt.

- `:2069` `        setzeBand (z, 0, 1000.0, 9.0);`
- `:2070` `        setze (*mit, z); setze (*ohne, z);`
- `:2071` `        hostSchreibt (*mit, iBand (0, param::kType), 1.0f);         // bell -> low_shelf: topologisch`
- `:2072` `        hostSchreibt (*mit, iBand (0, param::kChannelMode), 3.0f);  // stereo -> mid: topologisch`
- `:2073` `        mit->kontrollTaktFuerTest();`
- `:2074` `        const auto a = fahreAudio (*mit, 200, 128, 10);`
- `:2078` `        pruefe (typParam.convertFrom0to1 (typParam.getValue()) == 1.0f && w.werte[(size_t) iBand (0, param::kType)].enumIndex == 0`
- `:2081` `        hostSchreibt (*mit, iBand (0, param::kGainDb), -9.0f);`
- `:2082` `        mit->kontrollTaktFuerTest();`
- `:2083` `        const auto c = fahreAudio (*mit, 50, 128, 11);`
- `:2085-2086` `        pruefe (gain (mit->wirksamerZustand(), 0) == -9.0 && ! bitgleich (c, d),` / `                "  Gegenprobe: kontinuierliche Automation (gain_db) wirkt blockweise");`
- `prozessor()` `:1722`: `std::unique_ptr<Prozessor> prozessor (double rate = 48000.0, int block = 512)` — 48 kHz stimmt.

**Fall 2 — `TransactionTestMain.cpp:1313` → `:1316`/`:1317`.** Jede Angabe
stimmt.

- `grundzustand` `:276-281`, darin `:279` `    setzeBand (z, 0, freqHz, 3.0);`
  mit Vorgabe `double freqHz = 1000.0` (`:276`).
- `:1313` `        auto raus = st.tk->bestaetigt(); raus.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 500.0;`
- `:1315` `        const auto er = fahre (st, s, apply (*st.tk, raus, 3));`
- `:1316` `        rein = st.tk->bestaetigt(); rein.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 1000.0;`
- `:1317` `        const auto ei = fahre (st, s, apply (*st.tk, rein, 4));`
- `:1318-1319` sichern beide Commits zu:
  `        pruefe (e1.ausgang == tx::Ausgang::commit && st.tk->revision() == r1 + 3 && keinerVerletzt` /
  `                    && er.ausgang == tx::Ausgang::commit && ei.ausgang == tx::Ausgang::commit,`
- `Stand` `:130-144`, darin `:139` `        if (fs > 0.0) kern->bereiteVor (fs, 512);`
- `fahre` `:156-161` ruft nur `st.tk->fuehreAus (a)` — kein `verarbeite`.

**Die Mechanik, an der Quelle.** Der Commitweg publiziert immer durch
`vergebeKennungen`: `DspKernAusfuehrung::baueVor`
(`eq-copilot/plugin/state/NakamaTransaktion.cpp:715-727`) ruft
`kern.baueVor (sichtCommit, dsp::Pfad::committed)`, `publiziereVorbau`
(`:729-735`) ruft `kern.publiziereVorbau (dsp::Pfad::committed)`, und dort steht
`DspKern.cpp:347` `    vergebeKennungen (p, bank.programm);`. Danach
`:353` `    bank.zustaendeNullen();`. Am Blockrand entscheidet
`DspKern.cpp:507` `    const bool nurRampen = kennungenGleich && rampenKompatibel (*pAlt, pNeu);`
und `:515` `        bankNeu.baender         = baenke.bank (alt).baender;`.
`rampenKompatibel` (`DspProgramm.cpp:207-227`) nennt in `:222-223` genau
`typ`, `modus`, `dynamisch`, `nutztSvf`, `quelle` — kein Wertfeld;
`vergebeKennungen` (`DspKern.cpp:286-287`) dieselben fünf Felder.

### 3. Die Herleitung, selbst nachvollzogen

**Fall 1.** Ja, zwei nacheinander im selben Kern publizierte,
rampenkompatible Programme auf dem Committed-Pfad.

- Zwischen `:2070` und `:2081` entsteht **keine** dritte Publikation: `:2071`
  und `:2072` schreiben topologische Hostparameter;
  `Transaktionskern::wirksam` (`NakamaTransaktion.cpp:340-346`) übernimmt ein
  Overlayfeld nur, wenn `samplegenau \|\| ! t[i].topologisch` — bei
  unsupported `sample_accurate_automation` bleiben beide draußen. Damit ist
  `wirksam()` bitgleich zum bestätigten Stand, und
  `DspKernAusfuehrung::publiziereWirksam` (`:757-767`) bricht an
  `:762` `    if (! erzwingen && zuletztGueltig && werteGleich (sichtWeg.werte, zuletzt)) return true;`
  ab. Der Test beweist dasselbe von außen an `:2078` (`kType`-enumIndex bleibt
  0).
- `rampenKompatibel` ist wahr: beide Programme `eqEngagiert` (`mitEq (true)`),
  kein `hardBypass`, gleiche Rate (dieselbe Instanz, 48 kHz), `monoBassHz` beide
  0, Slot 0 in beiden aktiv, `typ` bell, `modus` stereo, `dynamisch` falsch,
  `nutztSvf` falsch, `quelle` gleich. Die Kennungen bleiben, weil `bleibt`
  (`DspKern.cpp:286-287`) dieselben fünf Felder prüft.
- **Audio läuft durch den Kern.** `:2074` fährt 200 Blöcke à 128 Samples über
  `Prozessor::processBlock` (`fahreAudio` `:1751-1769`), `:2083` weitere 50.
  Der Blockrand nach `:2082` ist damit ein echter Rampenübergang von +9,0 auf
  −9,0 dB, Differenz **18,0 dB** — gleichauf mit dem in M-103 und §40.3 als
  „der größte" benannten Fall.
- **Was die Prüfung wirklich vergleicht:** `! bitgleich (c, d)` —
  der Ausgang der automatisierten Instanz gegen den der nicht automatisierten.
  Sie behauptet einen **Unterschied**, keine Zahl.
- **Bliebe ihr Urteil nach dem Bau gleich?** Ja, in beiden Richtungen. Mit
  18,0 < `kSprungGainDb` = 20,0 bleibt der Fall auf dem Rampenweg und ändert
  nichts. Verließe er ihn (Kriterium unter 18,0), liefe statt der Rampe ein
  Crossfade; `c` und `d` unterschieden sich dann erst recht, und
  `gain (wirksamerZustand(), 0) == -9.0` hängt gar nicht am DSP. Die Prüfung
  merkt einen Wegwechsel **nicht** — und weil ihr `pruefe`-Aufruf `:2085-2086`
  keinen Detailtext mit Zahlen trägt, merkt ihn auch der in §40.3
  („Risiken und Grenze", `:6981`) als Abwehr genannte **Zeilenvergleich über B7
  vorher/nachher nicht**. Das ist der eigentliche Riss: die Wache, die den Fall
  auffangen soll, ist für ihn blind.

**Fall 2.** Ja, zwei nacheinander publizierte rampenkompatible Programme,
Verhältnis **genau 2,0** in beiden Richtungen.

- Publikationsfolge im Block `:1302-1330`: `grundzustand` (1000 Hz) → `mitZone`
  (`:1307`, keine Bandwertänderung) → `aus` (`:1311`, nur Zonen) → `raus`
  500 Hz (`:1315`) → `rein` 1000 Hz (`:1317`).
- Beide Commits sind vom Test zugesichert (`:1318-1320`); ein Commit setzt
  voraus, dass S5 (`baueVor`) eine freie Bank sah — sonst wäre der Ausgang
  `busy_retry`.
- `kSprungFrequenzVerhaeltnis` = 2,0. M-101 (`:6298`) sagt zur Kante:
  „**Strikt größer entscheidet: unter der Grenze und GENAU auf der Grenze
  bleibt der Slot auf dem Rampenweg und behält seine Kennung, über der Grenze
  bekommt er eine neue**" — der Vergleich ist also `>`, und ein Verhältnis von
  genau 2,0 nimmt danach den **Rampenweg**.
- 1000,0 und 500,0 sind in `double` exakt (1000 = 125·2³, 500 = 125·2²);
  1000,0/500,0 ist exakt 2,0 und 500,0/1000,0 exakt 0,5. Die richtungsfreie
  Bildung (größerer durch kleineren) liefert beide Male exakt 2,0. Der Abstand
  zur Grenze ist **0**.
- **Liest irgendeine Prüfung dieses Falls den Weg oder das Audio?** Nein.
  `fahre` (`:156-161`) ruft kein `verarbeite`; der Block prüft
  `ausgang == commit`, `revision() == r1 + 3` und `keinerVerletzt`
  (`:1318-1320`). Ohne Blockrand läuft weder Rampe noch Crossfade. Das Urteil
  hält so oder so.

### 4. Gegenargument

Gesucht und gefunden — es trägt aber nicht:

- §40.3 `:6872-6873` fängt Fall 1 **nicht** auf, sondern behauptet das
  Gegenteil: „keine von ihnen vergleicht Audio … aber `fahre` (`:156-161`) ruft
  nie `verarbeite`". Das gilt für die Ebene 1 von B7 (`Stand`), nicht für die
  Ebenen ab `prozessorParameter()` (`:1811` ff.), in denen `fahreAudio`
  (`:1751`) echte Blöcke durch `Prozessor::processBlock` schiebt. Fall 1 liegt
  in Abschnitt O (`:2041`), also in der Prozessorebene.
- Für Fall 2 gibt es kein auffangendes Wort. §40.3 zählt in der zweiten Tabelle
  ausdrücklich Publikationspaare **ohne** Audio (`:6872-6883`), Fall 2 ist also
  nach der eigenen Definition des Abschnitts zählpflichtig — er liegt nur nicht
  *über*, sondern *exakt auf* dem Kriterium und gehört damit in die **erste**
  Tabelle.
- Der Satz `:6888` („Weitere Fälle über einem Kriterium gibt es … nicht")
  bleibt nach meiner Messung wahr; gebrochen sind `:6862-6863`, `:6866-6868`,
  `:6969-6971` und `:6979-6981`.
- **Selbstwiderspruch im Manifest:** `:6889-6890` nennt
  `SondeNullTestMain.cpp:202-203` gegen `:662-663` als Paar mit 15,0 dB und
  Faktor 1,41, das „darunter bleibt" — das ist wörtlich ein weiterer Fall, der
  „zwei rampenkompatible Programme mit geändertem `freq_hz`, `q` oder
  `gain_db`" publiziert, und er steht nicht in der ersten Tabelle. `:6862-6863`
  widerspricht damit `:6889-6890` innerhalb derselben Seite.

### 5. Urteil

**BESTÄTIGT.** Beide vom Prüfer genannten Fälle halten an der Quelle; die
Aufzählung in §40.3 ist unvollständig. Meine eigene Suche zeigt die Lücke
**größer** als vom Prüfer benannt: die Tabelle unten führt **31** Zeilen, davon
stehen **20 in keiner der zwei Tabellen** von §40.3 und eine weitere (Nr. 30)
nur in der falschen. Zwei davon fahren echtes Audio über einen Blockrand
(Nr. 27 mit **18,0 dB** und Nr. 28), und **zwei** Bestandsfälle liegen mit
Abstand **0** auf dem Frequenzkriterium (Nr. 24 und Nr. 13). Der Prüfer hat die
Vollständigkeit seiner Suche ausdrücklich nicht behauptet; seine Aussage ist
damit nicht nur richtig, sondern vorsichtig.

### 6. Kleinste Korrektur (Vorschlag an den Dirigenten, kein Entscheid)

1. **Die Zählvorschrift ersetzen.** `:6845-6846` („gezählt über jede Zuweisung
   an `kGainDb`, `kFreqHz` und `kQ` und jeden `belege`-Aufruf") durch die
   Vorschrift unter Nr. 3 dieses Abschnitts ersetzen: gezählt wird über jeden
   Weg, auf dem ein Test `freq_hz`, `q` oder `gain_db` eines Bandes setzt —
   direkte Zuweisung, `belege`, `setzeBand`, `grundzustand`, `mitGain`,
   `bandWerte`, `setzeHoerbaresBand`, `nak311AchtShelves`, `nak311DynBand`,
   `resonator`, `hostSchreibt`/`setValueNotifyingHost`, Auftragsnutzlast
   (`apply`, `bandBelegen`, `presetLaden`, `undo`, `redo`, `revert`,
   `neutralisieren`) und `ladestart`/`setStateInformation`.
2. **Den Geltungsbereich der ersten Tabelle schärfen.** Entweder die Tabelle um
   die Paare aus `TransactionTestMain.cpp` und `SondeNullTestMain.cpp`
   ergänzen (dann trägt `:6862-6863`), oder die erste Tabelle ausdrücklich auf
   „Rampenfälle **mit Audio über einem Blockrand**" beschränken und einen
   zweiten, kurzen Satz für die Paare ohne Blockrand führen. Der zweite Weg ist
   der kleinere Eingriff und deckt sich mit dem, was die Golden-Regel
   (`:6966-6975`) wirklich braucht.
3. **Zwei Zeilen sind in jedem Fall aufzunehmen**, weil sie Audio über einen
   Blockrand fahren:
   - `TransactionTestMain.cpp:2069`/`:2081`, Gain +9,0 → −9,0 dB = **18,0 dB**,
     Abstand 2,0 dB — gleichauf mit dem heute als „der größte" benannten Fall.
     Der Satz `:6971` („der engste ist `uebernahme_nur_am_aeusseren_blockrand`")
     wird dadurch zu „zwei Fälle teilen sich den engsten Abstand".
   - `TransactionTestMain.cpp:2091`/`:2097`, Gain 3,0 → −6,0 dB = **9,0 dB** als
     Publikationspaar (verdrängt vor dem ersten Blockrand), danach 50 Schritte
     zu 0,2 dB je Blockrand mit Audio.
4. **Den Rand-0-Fall benennen.** `:6867` („Alle sieben Fälle liegen **unter**
   den drei Kriterien") stimmt für die sieben; zwei Bestandsfälle liegen jedoch
   mit Abstand 0 **auf** dem Frequenzkriterium
   (`TransactionTestMain.cpp:1313`→`:1316` und `:524-531`). §43.2 hat für den
   Gain genau diesen Zustand zum Anlass genommen, 18,0 auf 20,0 zu heben; für
   die Frequenz steht er unbenannt da. Ein Satz, der ihn nennt und auf M-101
   („strikt größer") zurückbindet, schließt.
5. **Die Wache nachziehen.** Der `git grep` über `kFreqHz`, `kQ`, `kGainDb`
   (`:6982-6983`) findet Fall 1 nur halb (`:2081` trägt `kGainDb`, `:2069`
   nicht) und Fall 2 gar nicht (`:1313` trägt `kFreqHz`, aber der
   Ausgangswert kommt aus `grundzustand`). Die Wache muss die Helfernamen
   mitsuchen.
6. **§38.1 `:5933` und M-103 `:6300`** verweisen auf „die vollständige
   Aufzählung"; sie tragen den Wortlaut der Korrektur aus Nr. 2 mit.

### Die Suchvorschrift, wiederholbar

```
# 1. direkte Zuweisungen
rg -n "kFreqHz|kGainDb|kQ\b" eq-copilot/plugin/tests/DspGoldenTestMain.cpp \
   eq-copilot/plugin/tests/TransactionTestMain.cpp \
   eq-copilot/plugin/tests/SondeNullTestMain.cpp \
   eq-copilot/plugin/tests/SchemaTestMain.cpp
# 2. Helferdefinitionen je Datei
rg -n "^[A-Za-z_].*\b(belege|machDynamisch|setzeBand|grundzustand|mitGain|bandWerte|hostSchreibt|nak311DynBand|resonator|setzeHoerbaresBand|nak311AchtShelves|machSatz|mitEq)\s*\(" <die vier Dateien>
# 3. deren Aufrufstellen
rg -n "belege \(|setzeBand \(|grundzustand \(|mitGain \(|bandWerte \(|hostSchreibt \(|nak311DynBand \(|resonator \(\)|setzeHoerbaresBand \(|nak311AchtShelves \(" <die vier Dateien>
# 4. Host-Automationsweg
rg -n "setValueNotifyingHost|automationSchreiben|hostSchreibt \(" <die vier Dateien>
# 5. Publikationswege
rg -n "uebernehmeZustand|publiziereVorbau|setzePreview|ladestart|setStateInformation|prepareToPlay|fuehreTransaktionAus|fahre \(|setze \(\*" <die vier Dateien>
```

**Helfer je Datei, mit Fundstelle der Definition**

\| Datei \| Helfer, die `freq_hz`, `q` oder `gain_db` setzen \|
\|---\|---\|
\| `DspGoldenTestMain.cpp` \| `machSatz` `:150`, `setzeGlobal` `:157`, `setzeGlobalBool` `:163`, **`belege` `:171`** (setzt `kFreqHz`, `kQ`, `kGainDb`), `machDynamisch` `:183` (setzt KEIN Kriterienfeld — `kDynamicEnabled`, `kDynamicRangeDb`, `kThresholdDb`, `kAttackMs`, `kHoldMs`, `kReleaseMs`, `kSidechainSource`) \|
\| `TransactionTestMain.cpp` \| `mitEq` `:246`, **`setzeBand` `:253`**, **`bandWerte` `:266`** (über `setzeBand`), **`grundzustand` `:276`** (über `setzeBand`, Vorgabe 1000 Hz / 3,0 dB), **`mitGain` `:284`**, **`hostSchreibt` `:1795`** (`setValueNotifyingHost`), `resonator` `:2243` (nutzt `setzeBand` `:2246` und `kQ` `:2247`), **`nak311DynBand` `:2828`** (`kFreqHz` `:2837`, `kQ` `:2838`, `kGainDb` `:2839`) \|
\| `SondeNullTestMain.cpp` \| **`setzeHoerbaresBand` `:197`** (`kFreqHz` `:202`, `kGainDb` `:203`), das lokale `mitBand` `:1129` (`kFreqHz` `:1136`, `kQ` `:1137`, `kGainDb` `:1138`), `setze` `:184`, `vorbereitet` `:175` \|
\| `SchemaTestMain.cpp` \| **`nak311AchtShelves` `:1424`** (`kFreqHz` `:1435`, `kQ` `:1436`, `kGainDb` `:1437`), `nak311KernAutoGain` `:1484` \|

`belege` existiert nur in `DspGoldenTestMain.cpp` — die bisherige
Zählvorschrift konnte die drei anderen Dateien also nur über direkte
Zuweisungen sehen.

### Tabelle — jedes Paar nacheinander im selben Kern und auf demselben Pfad publizierter Programme mit geändertem `freq_hz`, `q` oder `gain_db`

Kein Paar ohne Wertänderung; Schleifen mit ihrem größten Schritt in einer
Zeile. „rk" = rampenkompatibel nach `DspProgramm.cpp:207-227`.

\| # \| Datei:Zeilen (Paar) \| Prüfname (wie ausgegeben) \| Kriterium \| Werte \| Differenz / Verhältnis \| rk (Grund) \| Audio \| was die Prüfung liest \| in §40.3? \|
\|---\|---\|---\|---\|---\|---\|---\|---\|---\|---\|
\| 1 \| `DspGoldenTestMain.cpp:1875` (sa→sb `:1894`/`:1900`) \| `bandwert_gain_db_rampt_ohne_zustandsreset (M-17, R8, B-4)` (`:1931-1933`) \| Gain \| 3,0 → 9,0 dB \| 6,0 dB \| ja \| ja \| Residuum gegen die ausgeschriebene Koeffizientenrampe, < 1e−5 \| **ja** (Zeile 1) \|
\| 2 \| `:1876` \| `bandwert_freq_hz_rampt_ohne_zustandsreset` \| Frequenz \| 1000 → 1400 Hz \| 1,4 \| ja \| ja \| wie 1 \| **ja** \|
\| 3 \| `:1877` \| `bandwert_q_rampt_ohne_zustandsreset` \| Güte \| 1,0 → 3,0 \| 3,0 \| ja \| ja \| wie 1 \| **ja** \|
\| 4 \| `:4013` → `:4014` \| `null_allokationen_im_callback_samt_programmwechseln (M-41, M-47, B-25)` (`:4075-4076`) \| Gain \| 6,0 → 2,0 dB \| 4,0 dB \| ja \| ja \| Allokations- und Sperrzähler, Übernahmen \| **ja** \|
\| 5 \| `:4199` → `:4205` \| `uebernahme_nur_am_aeusseren_blockrand (M-25, R9, B-11)` (`:4225`) \| Gain \| +9,0 → −9,0 dB \| **18,0 dB** \| ja \| ja \| Umschaltzeitpunkt am äußeren Blockrand \| **ja** („der größte") \|
\| 6 \| `:5670` → `:5686` \| `realtime_und_offline_gleich (M-120)` (`:5799`, `:5816`) \| Gain \| +5,0 → −2,0 dB \| 7,0 dB \| ja \| ja \| zwei Läufe desselben Kerns gegeneinander \| **ja** \|
\| 7 \| `:6825` → `:6831` \| `311/M-94 uebertragung_mit_wertaenderung_haelt_e31_spitze_und_referenz` (`:6876-6887`) \| Gain \| +12,0 → +6,0 dB \| 6,0 dB \| ja \| ja \| E-31-Spitze, Perioden-RMS gegen Referenzkern \| **ja** \|
\| 8 \| `TransactionTestMain.cpp:398` → `:400` → `:402` \| `T1 wiederholung_im_fenster_liefert_dasselbe_ergebnis` (`:409`) \| Gain \| 3,0 → 6,0 → −6,0 dB \| 3,0 dB; **12,0 dB** \| ja \| nein \| Ausgang, Revision, Register \| **nein** \|
\| 9 \| `:417` → `:421` \| `T2 veraltete_base_revision_ist_konflikt_ohne_wirkung` (`:422`) \| Gain \| 3,0 → 6,0 dB \| 3,0 dB \| ja \| nein \| Ausgang, Stufe, Revision \| **nein** \|
\| 10 \| `:435` → `:443` \| `T3 ungueltige_nutzlast_faellt_und_wird_nicht_gemerkt` (`:438`) \| Gain \| 3,0 → 6,0 dB (18,0 wird abgewiesen, keine Publikation) \| 3,0 dB \| ja \| nein \| Ausgang, Register \| **nein** \|
\| 11 \| `:476` → `:478` \| `T6 commit_erzeugt_genau_eine_revision_und_einen_eintrag` (`:481`) \| Gain \| 3,0 → 6,0 dB \| 3,0 dB \| ja \| nein \| Revision, Registereintrag \| **nein** \|
\| 12 \| `:491` → `:494` \| `T7 wiederholung_nach_bankfreigabe_committet` (`:500`) \| Gain \| 3,0 → 6,0 dB \| 3,0 dB \| ja \| nein \| Ausgang, Revision \| **nein** \|
\| 13 \| `:524-531` (`zonenVorlauf` `:372-381`, darin `:375` 500 Hz und `:379` 1000 Hz; `rebasiert`) \| `T9 retry_nach_zonenloesung_mit_aktueller_base_revision_committet` (`:532`) \| **Frequenz** \| 500 → 1000 Hz \| **genau 2,0 — Abstand 0** \| ja \| nein \| Ausgang, Stufe, Revision \| **nein** \|
\| 14 \| `:574-579` (Schleife i = 1-33) \| `dreiunddreissigCommits`, Vorlauf für `T10`/`T11`/`T17` \| Gain \| `(i % 24) − 12 + 0,25`, Umlauf 23 → 24 \| **23,0 dB** \| ja \| nein \| Fensterrand, Revision, Hash, Wachen I1/I2/I4 \| **ja** (zweite Tabelle) \|
\| 15 \| `:609` → `:610` → `:612` \| `T12 abweisungen_verdraengen_keinen_committeten_eintrag` (`:623`) \| Gain \| 3,0 → 6,0 → −6,0 dB \| 3,0 dB; **12,0 dB** \| ja \| nein \| Registereintrag, Ausgang \| **nein** \|
\| 16 \| `:742` → `:762` \| `fehler_hinterlaesst_keinen_registereintrag` (`:764`) \| Gain \| 3,0 → 6,0 dB \| 3,0 dB \| ja \| nein \| Ausgang, Revision \| **nein** \|
\| 17 \| `:785-805` (Schleife i = 0-39, Slot `i % 8`) \| `hinter_dem_commit_punkt_wird_nicht_alloziert` (`:817`) \| Gain / Frequenz \| je Slot `(i%12)` → `((i+8)%12)`; `200 + 10i` → `200 + 10(i+8)` \| **8,0 dB**; **1,4** (200 → 280 Hz) \| ab i = 8 ja (davor wird je ein Slot erst aktiv) \| nein \| Allokationszähler hinter dem Commit-Punkt \| **nein** \|
\| 18 \| `:806-811` (undo, undo, redo, revert) \| dieselbe Prüfung (`:817`) \| Gain / Frequenz \| Rücknahme je eines Slots \| ≤ 8,0 dB; ≤ 1,16 \| ja (soweit Topologie gleich bleibt) \| nein \| wie 17 \| **nein** \|
\| 19 \| `:832` → `:848` \| `apply_erzeugt_genau_eine_revision` (`:843`) \| Gain \| 3,0 → 6,0 dB \| 3,0 dB \| ja \| nein \| Revision, Ring, Register \| **nein** \|
\| 20 \| `:891-897` (Schleife i = 1-7) \| Vorlauf zu `E` (`:890`) \| Gain \| `(double) i`, 1 → 7 \| 1,0 dB je Schritt \| ja \| nein \| nur Vorlauf, gespeichert nach `:899` \| **nein** \|
\| 21 \| `:1004` → `:1009` → `:1013` \| `wiederholung_unter_einer_zone_bekommt_nicht_user_schutz` (`:1019`) \| Frequenz \| 1000 → 1050 → 500 Hz \| 1,05; **2,1 (ÜBER)** \| ja \| nein \| Zonenein-/-austritt, Ausgang, Revision, Hash \| **ja** (zweite Tabelle, `:1009`/`:1013`) \|
\| 22 \| `:1042` → `:1043` \| `register_ist_nicht_teil_des_zustands` (`:1053`) \| Gain \| 3,0 → 6,0 dB \| 3,0 dB \| ja \| nein \| Hash, Revision, Registerbelegung \| **nein** \|
\| 23 \| `:1273` → `:1282` → `:1285` \| `verletztes_band_bleibt_bedienbar (M-69)` (`:1288`) \| Gain, Frequenz \| 3,0 → −4,0 dB; 1000 → 950 Hz \| **7,0 dB**; 1,0526 \| ja \| nein \| Ausgang, bestätigter Gain und Frequenz \| **nein** \|
\| 24 \| `:1304` → `:1315` → `:1317` \| `zone_deaktivieren_ist_eine_transaktion (M-71)` (`:1318-1320`) \| **Frequenz** \| 1000 → 500 → 1000 Hz \| **genau 2,0 in beiden Richtungen — Abstand 0** \| ja \| nein \| Ausgänge, Revision, `verletzteBaender` \| **nein** \|
\| 25 \| `:1393-1397` (Schleife i = 1-40) \| `undoRing`-Vorlauf (Abschnitt I, `:1389`) \| Gain \| `((i % 24) − 12) + 0,5`, Umlauf 23 → 24 \| **23,0 dB** \| ja \| nein \| Ringtiefe, älteste/jüngste Revision, Speichern/Laden \| **ja** (zweite Tabelle, `:1396`) \|
\| 26 \| `:1411` → `:1412` → `:1413` → `:1414` → `:1416` \| `neue_transaktion_schneidet_den_redo_zweig_ab (M-79)` (`:1421`) \| Gain \| 3,0 → 1,0 → 2,0 → (undo) 1,0 → 9,0 dB \| 2,0; 1,0; 1,0; **8,0 dB** \| ja \| nein \| Ring, Cursor, Ausgang \| **nein** \|
\| 27 \| `:2069` → `:2081` \| `  Gegenprobe: kontinuierliche Automation (gain_db) wirkt blockweise` (`:2086`), im Test `capability_entscheidet_den_automationspfad (M-119)` \| Gain \| +9,0 → −9,0 dB \| **18,0 dB** \| ja \| **ja** (200 + 50 Blöcke à 128) \| `! bitgleich (c, d)` und `gain (wirksam,0) == −9.0` \| **nein** \|
\| 28 \| `:2091` → `:2097` (Schleife i = 0-49) \| `automation_erzeugt_keine_revision (M-81, Prozessor)` (`:2109`) und `epoche_wechselt_genau_zweimal_je_geste (M-81, Prozessor)` (`:2111`) \| Gain \| 3,0 → −6,0; dann `−6,0 + 0,2·i` \| **9,0 dB** als Publikationspaar (vor dem ersten Blockrand verdrängt), danach 0,2 dB je Blockrand \| ja \| **ja** (je 1 Block à 480) \| Revision, Undo-Ring, Automationsepoche \| **nein** \|
\| 29 \| `:2118` → `:2135` (Gestusende) \| `abgeschlossener_gestus_ist_eine_revision (M-82)` (`:2140`) \| Gain, Frequenz \| 3,0 → ≈ −11,0 dB; 1000 → ≈ 1680 Hz \| **14,0 dB**; 1,68 \| ja \| nein \| Revision, Ringgröße, Endwerte \| **nein** \|
\| 30 \| `SondeNullTestMain.cpp:202-203` → `:662-663` \| `speichern_laden_speichern_bytegleich_in_v2: Kind Dsp mit Revision, Zone und Undo-Ring` (`:685`), Abschnitt `:653` \| Gain, Güte \| 12,0 → −3,0 dB; 1,0 → 0,7071 \| **15,0 dB**; 1,4142 \| ja \| nein \| Bytegleichheit des gespeicherten Baums \| genannt, aber in der **zweiten** Tabelle (`:6889-6890`) statt in der ersten \|
\| 31 \| `:662-663` → Undo `:665-669` \| dieselbe Prüfung (`:685`) \| Gain, Güte \| −3,0 → 12,0 dB; 0,7071 → 1,0 \| 15,0 dB; 1,4142 \| ja \| nein \| wie 30 \| **nein** \|

**Nicht in der Tabelle, mit Grund.** `TransactionTestMain.cpp:2222` (+6 → −12
= 18,0 dB) liegt auf dem **Candidate**-Pfad (`setzePreview` `:2224`) — anderer
Pfad, eigener Merkzettel; im Manifest richtig ausgenommen. Die Abschnitte R
(`:2621`) und S (`:2673`) wechseln `eqEngagiert` beziehungsweise die Belegung —
schon heute Crossfade. `:903-927` belegt Slot 3 neu (`a.aktiv != n.aktiv`,
`DspProgramm.cpp:220`). `:1064-1073`, `:1096-1113`, `:1120-1133`, `:1522-1536`
(`dynamisch` ist Topologiefeld), `:1547-1560`, `:1662-1667`, `:1368-1379` und
`:1426-1438` ändern Topologie. `:459-466` (T5), `:631-638` (T13-T15),
`:1193-1208`, `:1246-1249`, `:1259-1266`, `:1351-1358`, `:1507-1512` weisen
ab — keine Publikation. `:1450-1468` und `:1292-1300` schreiben nur das
Automationsoverlay: `Transaktionskern::automationSchreiben`
(`NakamaTransaktion.cpp:316-324`) publiziert **nicht**. `:2045-2055` schreibt
auf den freien Slot 5, den `setzeFreieSlotsNeutral` vor der Publikation
neutralisiert — der Test misst das selbst (`uebernahmenMit == uebernahmenOhne`,
`:2062`). `SchemaTestMain.cpp` publiziert je `Nak311Stand` zweimal (`ladestart`
`:1526`/`:1568` und `nak311KernAutoGain` `:1551`/`:1574`) — beide Male
**denselben** Satz, also ohne Wertänderung. `:2906-2913`, `:2983-2986`,
`:3088-3089` und `:2940` (`prepareToPlay` in `fahreRecall`) publizieren
denselben Zustand erneut.

**Grenze meiner Suche.** `TransactionTestMain.cpp`, `SondeNullTestMain.cpp` und
`SchemaTestMain.cpp` habe ich Block für Block durchgegangen (jede `Stand`- und
`prozessor()`-Instanz, jeden `fahre`/`setze`/`hostSchreibt`/`ladestart`-Aufruf);
die Tabelle ist für diese drei Dateien vollständig. In
`DspGoldenTestMain.cpp` (227 Publikationsstellen) habe ich die sieben Zeilen des
Manifests an der Quelle bestätigt und zusätzlich jede Stelle geprüft, an der
zwei `uebernehmeZustand`-Aufrufe denselben Kern treffen und ein Bandwert im
Spiel ist (`:1019`-`:1045`, `:1078`-`:1081`, `:1126`-`:1144`, `:1196`-`:1211`,
`:1261`-`:1270`, `:1303`-`:1345`, `:1382`-`:1408`, `:1429`-`:1439`,
`:1801`-`:1818`, `:1894`-`:1900`, `:2251`-`:2276`, `:2330`-`:2331`,
`:2473`-`:2483`, `:2543`-`:2547`, `:2653`, `:2779`-`:2782`, `:2978`-`:2981`,
`:3043`, `:3083`, `:5274`-`:5276`, `:5397`-`:5401`, `:5479`-`:5481`,
`:6485`, `:6629`-`:6648`, `:6706`, `:6821`-`:6831`); alle übrigen wechseln
Topologie, `eqEngagiert`, Rate, Kanalmodus oder nur globale Werte. Dass es in
den nicht einzeln aufgeführten Stellen dieser einen Datei keinen achten Fall
gibt, behaupte ich nicht.

---

## V-2 — D-5: die neue Fundstelle in M-99

### 1. Der betroffene Satz, wörtlich

M-99 (§39.1, `:6296`), Rotbeweisspalte: „… und der Fall ist am Basisstand mit
vorhandenen Mitteln messbar: der SVF-Weg besteht, `detektorLaeuft` ist bei
Range 0 falsch (`DspProgramm.cpp:318`), `nutztSvf` wahr, **`machDynamisch` mit
Range 0 steht im Bein (`DspGoldenTestMain.cpp:5677`, `:6821`)**, und die
Übergangsspitze über 0,5 s wird am Ausgang abgelesen wie bei M-96."

### 2. Die Quelle

- `DspGoldenTestMain.cpp:5677`
  `        machDynamisch (basis, 5, -6.0, -20.0, 5.0, 10.0, 80.0);`
  → Range **−6,0 dB** auf Slot 5. **Stimmt nicht.**
- `DspGoldenTestMain.cpp:6821`
  `                    machDynamisch (vorher, 0, -12.0, -40.0, 0.1, 0.0, 100.0);`
  → Range **−12,0 dB**. **Stimmt nicht.**
- Signatur `:183-185`
  `void machDynamisch (param::DspSatz& s, int slot, double rangeDb, double thresholdDb,` /
  `                    double attackMs = 10.0, double holdMs = 0.0, double releaseMs = 100.0,` /
  `                    Sidechain quelle = Sidechain::internal)`
  und `:188` `    s.werte[(size_t) param::indexBandV1 (slot, param::kDynamicRangeDb)].zahl = rangeDb;`
  — das dritte Argument nach `s` ist die Range. Der Prüfer schreibt „`:183-187`";
  richtig ist die Signatur `:183-185`, der Beweis für „drittes Argument = Range"
  steht an `:188`.
- **Richtig wären** drei Stellen, nicht zwei:
  - `:2057` `            machDynamisch (s, 0, 0.0, -60.0);   // Range 0` — der einzige
    Aufruf mit einem Literal 0.
  - `:2540` `            machDynamisch (sa, 0, rVon, -60.0, 0.1, 0.0, 100.0);` mit
    `:2536` `            const double rVon = richtung == 0 ? -12.0 : 0.0, rNach = …`
    — trifft Range 0 in der Richtung `richtung == 1` (Schleife `:2534`).
  - `:2652` `                        machDynamisch (s, 0, range, -60.0, 0.1, 0.0, 100.0);`
    über die Lambda `machKern` (`:2647`), aufgerufen mit
    `:2657` `                    auto kRef = machKern (0.0);` — der Referenzkern
    der Steuerratenprüfung `311/M-73` bis `311/M-75`. Diese dritte Stelle
    nennt der Prüfer nicht; sein Satz „der einzige Aufruf mit Range 0 … steht
    an `:2057`" ist damit zu eng.
- `machDynamisch` existiert nur in `DspGoldenTestMain.cpp` (38 Aufrufe);
  `SondeNullTestMain.cpp` und `SchemaTestMain.cpp` tragen **null** Treffer für
  `kDynamicEnabled`/`kDynamicRangeDb`, `TransactionTestMain.cpp` setzt die
  Felder direkt (`:1100-1101`, `:1150-1151`, `:1523-1524`, `:1575-1576`,
  `:2694-2695`, `:2841-2842`) und nie mit Range 0.

### 3. Trägt die Umetikettierung ohne den falschen Satz?

Ja. Die Umetikettierung von M-99 auf „heute rot" steht auf vier anderen
Beinen, die alle an der Quelle halten:

- `rampenKompatibel` kennt kein Wertfeld (`DspProgramm.cpp:222-223`).
- `bleibt` vergleicht nur Topologiefelder (`DspKern.cpp:286-287`).
- `DspProgramm.cpp:318`
  `        b.detektorLaeuft = b.nutztSvf && detektorGewuenscht && b.rangeDb != 0.0;`
  — bei Range 0 ist `detektorLaeuft` falsch, `nutztSvf` bleibt wahr.
- Der SVF-Weg interpoliert je Steuerschritt (`DspKern.cpp:690-710`), und die
  Spitze wird am Ausgang abgelesen.

Der Satz ist also entbehrlich, aber falsch. Ohne ihn bleibt das Etikett
unberührt.

### 4. Jede im Fixdiff neu eingeführte Pfad:Zeile-Angabe in §38 bis §41

Ermittelt als Mengendifferenz: alle Angaben der Form
`Datei.endung:Zeile[-Zeile]` im neuen §38-§41 (`:5836-7549`) minus alle im
alten §38-§41 (`137672b4`, `:5836-7207`). 125 gegen 97; neu sind die folgenden
33. §44 ist ausgenommen.

\| Angabe \| Quelle am HEAD \| Urteil \|
\|---\|---\|---\|
\| `DspFilter.h:314-318` \| `if (leistungEin > leistung) { leistung = k.attackPol * leistung + (1.0 - k.attackPol) * leistungEin; holdRest = k.holdSamples; }` \| stimmt \|
\| `DspFilter.h:355-361` \| `inline double dynamischeKennlinie (double pegelDb, double thresholdDb, double rangeDb)` bis `return rangeDb * u;` \| stimmt \|
\| `DspGoldenTestMain.cpp:1875` \| `{ "gain_db", param::kGainDb, 3.0,    9.0 },` \| stimmt \|
\| `DspGoldenTestMain.cpp:2564` \| `const double leistungNachRampe = k->pool().bank (cA).baender[0].huelle.leistung;` \| stimmt \|
\| **`DspGoldenTestMain.cpp:5677`** \| `machDynamisch (basis, 5, -6.0, -20.0, 5.0, 10.0, 80.0);` — Range −6,0, Slot 5 \| **stimmt nicht** (richtig: `:2057`, `:2540`, `:2652`/`:2657`) \|
\| `DspKern.cpp:741-745` \| `if (b.nutztSvf && ! detektorZuletzt) { z.huelle.nullen(); for (auto& d : z.detektor) d.nullen(); }` \| stimmt \|
\| `DspKern.cpp:743` \| `z.huelle.nullen();` \| stimmt \|
\| `DspKern.h:312` \| `double autoGainDb() const noexcept { return autoGainBericht[0].load (std::memory_order_relaxed); }` \| stimmt \|
\| `DspProgramm.cpp:212` \| `if (! alt.eqEngagiert \|\| ! neu.eqEngagiert \|\| alt.hardBypass \|\| neu.hardBypass) return false;` \| stimmt \|
\| `DspProgramm.cpp:318` \| `b.detektorLaeuft = b.nutztSvf && detektorGewuenscht && b.rangeDb != 0.0;` \| stimmt \|
\| `DspProgramm.h:95-97` \| `kAutoGainStellen = 121`, `kAutoGainVonHz = 20.0`, `kAutoGainBisHz = 20000.0` \| stimmt \|
\| `SondeNullTestMain.cpp:202-203` \| `kFreqHz = 1000.0` / `kGainDb = 12.0` in `setzeHoerbaresBand` \| stimmt \|
\| `TransactionTestMain.cpp:2222` \| `vorschau.werte[(size_t) iBand (0, param::kGainDb)].zahl = -12.0;` (Candidate, `setzePreview` `:2224`) \| stimmt \|
\| `TransactionTestMain.cpp:577` \| `z.werte[(size_t) iBand (0, param::kGainDb)].zahl = (double) (i % 24) - 12.0 + 0.25;`, Schleife `:574` `i = 1 … 33` \| stimmt \|
\| `docs/beweise/SONDE-015.md:3423` \| Zeile E-29 „Wie der Übergang `dynamic_range_db` auf 0 und von 0 weg rampt (W-3)" \| stimmt \|
\| `eq-copilot/plugin/dsp/DspKern.cpp:120` \| `for (auto& a : autoGainBericht) a.store (0.0, std::memory_order_relaxed);` \| stimmt \|
\| `eq-copilot/plugin/dsp/DspKern.cpp:127-154` \| `void DspKern::freigeben()` … `letzteKanaele = 0; }` \| stimmt \|
\| `eq-copilot/plugin/dsp/DspKern.cpp:140` \| `kanalzahl       = 2;` \| stimmt \|
\| `eq-copilot/plugin/dsp/DspKern.cpp:507` \| `const bool nurRampen = kennungenGleich && rampenKompatibel (*pAlt, pNeu);` \| stimmt \|
\| `eq-copilot/plugin/dsp/DspKern.cpp:741-745` \| wie oben \| stimmt \|
\| `eq-copilot/plugin/dsp/DspProgramm.cpp:120` \| `const double f = autoGainGitterHz (i);` im ersten Gitterlauf \| stimmt \|
\| `eq-copilot/plugin/state/NakamaTransaktion.cpp:350-357` \| `void Transaktionskern::setzeSamplerate (double samplerate, int kanaele) noexcept` … `kanalzahl = kanaele; }` \| stimmt \|
\| `eq-copilot/plugin/state/NakamaTransaktion.cpp:817` \| `dsp::baueProgramm (sicht, fs, 0, *prog, tk.kanaele());` \| stimmt \|
\| `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1875` \| wie oben \| stimmt \|
\| `…/DspGoldenTestMain.cpp:1931-1933` \| `pruefe (maxRes < 1e-5,` / `"bandwert_" + sp.name + "_rampt_ohne_zustandsreset (M-17, R8, B-4)",` / Detailzeile \| stimmt \|
\| `…/DspGoldenTestMain.cpp:2432` \| `pruefe (innerhalb, "attack_hold_release_als_sprungantwort_bei_vier_raten (M-26, B-15)",` \| stimmt \|
\| `…/DspGoldenTestMain.cpp:2531-2578` \| Prüfstand `range_…_rampt_ueber_die_volle_rampe`, Kommentarbeginn `:2531`, `pruefe` `:2572` \| stimmt \|
\| `…/DspGoldenTestMain.cpp:2564` \| wie oben \| stimmt \|
\| `…/DspGoldenTestMain.cpp:4199` \| `belege (ein, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0);` \| stimmt \|
\| `…/SondeNullTestMain.cpp:522-540` \| Block `6. M-02 eq_an_bypass_aus_alles_neutral_ist_bitidentisch` \| stimmt \|
\| `…/SondeNullTestMain.cpp:523` \| `abschnitt ("6. M-02 eq_an_bypass_aus_alles_neutral_ist_bitidentisch");` \| stimmt \|
\| `…/TransactionTestMain.cpp:130-144` \| `struct Stand { … explicit Stand (double fs = 48000.0) { … } };` \| stimmt \|
\| `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2325` \| die `$comment`-Zeile von `auto_gain_db` \| stimmt \|

Zusätzlich habe ich die neu eingeführten **fortsetzenden** Angaben der Form
`:Zeile` geprüft, soweit sie eine Zusage tragen (124 Stück neu; geprüft: alle
Fundstellen der zwei Tabellen von §40.3, alle Fundstellen der zwei Listen von
§40.4, die `freigeben`/`bereiteVor`-Kette und die Prüfstandsbereiche). Genau
**zwei** stimmen nicht:

- **`:6821`** in M-99 (`:6296`) — dieselbe Behauptung wie `:5677`,
  `machDynamisch (vorher, 0, -12.0, …)`, Range −12,0.
- **`:2119`** in §40.4 (`:7104`, Fundstelle für „der persistente Wert bleibt
  priority_sidechain (M-23)") — `:2119` ist
  `                    == (int) Sidechain::prioritySidechain,`, also die
  Fortsetzung der Bedingung. Der `pruefe`-Aufruf beginnt an `:2118`, der
  Prüfname steht an `:2120`. Alle übrigen Fundstellen dieser Liste nennen die
  `pruefe`-Zeile (`:2049`, `:2116`, `:4075`, `:4288`, `:4340`, `:5130`, …);
  richtig wäre `:2118`.

Alle anderen geprüften Fortsetzungsangaben stimmen, unter anderem `:1876`,
`:1877`, `:1931-1933`, `:2049`, `:2067`, `:2079`, `:2097`, `:2116`, `:2150`,
`:2153`, `:2175-2176`, `:2187`, `:2191`, `:2226`, `:2239`, `:2291`, `:2312`,
`:2356`, `:2432`, `:2434`, `:2497`, `:2510`, `:2521-2523`, `:2564`, `:2569`,
`:2572-2578`, `:2598`, `:2630-2632`, `:2889`, `:2908`, `:3183`, `:3188`,
`:3202`, `:3252`, `:3379`, `:4013`, `:4014`, `:4075`, `:4081`, `:4199`,
`:4205`, `:4225`, `:4288`, `:4295`, `:4307`, `:4340`, `:5130`, `:5192`,
`:5200`, `:5206`, `:5242`, `:5256`, `:5397-5401`, `:5414`, `:5479-5481`,
`:5635`, `:5639`, `:5670`, `:5686`, `:5799`, `:5816`, `:6529`, `:6533`,
`:6557`, `:6654`, `:6672`, `:6692`, `:6715`, `:6752`, `:6756`, `:6782`,
`:6825`, `:6831`, `:1009`, `:1013`, `:1015-1021`, `:1095-1107`, `:1145-1156`,
`:1396`, `:1402-1408`, `:1523-1541`, `:1575`, `:156-161`, `:164-201`,
`:2685-2700`, `:120`, `:140`, `:507`, `:547`, `:686`, `:743`, `:822`,
`:662-663`.

### 5. Gegenargument

Keines. Der Satz behauptet eine Fundstelle im Bein, und beide genannten Stellen
tragen einen anderen Wert. Es gibt auch keine frühere Fassung, in der die
Zeilennummern gestimmt hätten: der Satz ist neu im Fixdiff.

### 6. Urteil

**BESTÄTIGT.** Beide Fundstellen sind falsch; die Umetikettierung trägt ohne
den Satz. Präzisierung am Rand: der Prüfer nennt zwei richtige Stellen,
tatsächlich sind es drei (`:2057`, `:2540`, `:2652`/`:2657`), und die Signatur
steht an `:183-185`, nicht `:183-187`.

**Kleinste Korrektur.** In M-99 (`:6296`) die Klammer
„(`DspGoldenTestMain.cpp:5677`, `:6821`)" durch
„(`DspGoldenTestMain.cpp:2057`; über eine Laufvariable auch `:2540` und
`:2652`)" ersetzen. In §40.4 (`:7104`) `:2119` auf `:2118` setzen.

---

## V-3 — R-311-18: die zwei Listen von §40.4

### 1. Die betroffenen Sätze, wörtlich

- Die Regel, §43 (`:7665-7668`): „**R-311-18 (L-2).** Jede bestehende Prüfung
  eines dynamischen Bands steht mit Testnamen und Fundstelle in genau einer von
  zwei Listen („verschiebt sich" mit Richtung und Neubelegung, „verschiebt sich
  nicht" mit ihrem eigenen Grund); **Umschreibungen ohne Testnamen entfallen**."
- §40.4 `:7089-7092`: „**Was sich NICHT verschiebt, je Prüfung mit ihrem eigenen
  Grund** (vollständig aufgezählt in der Matrixnacharbeit 3, R-311-18…).
  **Gezählt sind alle Prüfungen der vier Testdateien, die ein dynamisches Band
  anfassen.**"
- §40.4 `:7086` (erste Liste): „\| B7 `311/M-77` und `311/M-78` (F12, R-311-4) \|
  t_E \| **größer** um höchstens 5·`kPegelFensterMs`·fs Samples … \|"
- §40.4 `:7134-7141`: „*Andere Beine:* **Abschnitt C von B6 (RBJ-Goldens, M-82)**
  und die Abschnitte G, H und H2 … **tragen kein dynamisches Band in der
  gemessenen Größe**; A16 fährt kein dynamisches Band im Nulltestmaterial; …
  In `TransactionTestMain.cpp` fassen `:1095-1107`, `:1145-1156`, `:1523-1541`,
  `:1575` und `:2685-2700` ein dynamisches Band an, **messen aber Register,
  Hash, Revision und Schemagültigkeit — keine Pegelgröße**."
- §40.4 `:7172` (Tests und Beine): „B7 `311/M-79` erweitert und die Prüflinge
  von `311/M-77` und `311/M-78`".
- M-128 (§39.3, `:6358`): „**jede** bestehende Prüfung eines dynamischen Bands
  in den vier Testdateien, die §40.4 in der Liste „Was sich NICHT verschiebt"
  mit Testnamen, Fundstelle und eigenem Grund führt".

### 2. Die Codebehauptungen des Prüfers

Alle vier stimmen.

1. `311/M-76 gleicher_ladestart_bleibt_warm (NAK-311 F12, M-84)` steht an
   `TransactionTestMain.cpp:3008` (Prüfstand `:2981-3012`, Kommentar ab
   `:2976`). Der Prüfling ist `nak311DynBand (1000.0, 2.0, 0.0, -12.0, -15.0,
   500.0)` (`:2982`): `kDynamicEnabled` = true (`:2841`), `kDynamicRangeDb` =
   **−12,0** (`:2842`), also ≠ 0. In §40.4 (`:6991-7212`) kommt weder `M-76`
   noch `gleicher_ladestart` vor.
2. `311/M-79 audiohistorie_ist_kein_state (NAK-311 F12, R-311-4)` steht an
   `:3103`, Prüfling `nak311DynBand (1000.0, 2.0, 0.0, -12.0, -40.0, 500.0)`
   (`:3087`), Range −12,0 ≠ 0. In §40.4 steht es nur an `:7172` unter „Tests und
   Beine" — ohne Richtung, ohne eigenen Grund, in keiner der zwei Listen.
3. Die Sammelzeile `:7138-7141` führt fünf Fundstellen ohne Testnamen. Die
   Testnamen dazu:
   - `:1095-1107` → `undo_stellt_das_entfernte_band_als_ein_objekt_her (M-61)`
     (`:1116`)
   - `:1145-1156` → `freier_slot_verarbeitet_nichts (M-58)` (`:1182`)
   - `:1523-1541` → `dynamicwerte_ueberleben_aus_und_ein_bitgleich (M-24, ueber
     zwei Transaktionen)` (`:1542`) — genau der vom Prüfer genannte
   - `:1575` → `klemmliste_erscheint_im_bericht (M-23, E2-7)` (`:1587`)
   - `:2685-2700` → `311/M-40 remove_und_belegen_ohne_audio_startet_kalt
     (NAK-311 T3-14-02, SONDE-015 M-121)` (`:2761`)
4. `:7086` nennt Matrixkennungen statt Testnamen und ohne Fundstelle. Die
   Testnamen werden an `:3063-3067` gebildet:
   `311/M-77 recall_in_neue_instanz_innerhalb_der_toleranz_pruefling_<1..3>`
   und `311/M-78 recall_nach_preparetoplay_innerhalb_der_toleranz_pruefling_<1..3>`,
   `pruefe`-Aufruf `:3067-3074`.

### 3. Eigene vollständige Suche

**Suchvorschrift.**

```
rg -n "kDynamicEnabled|kDynamicRangeDb" <die vier Testdateien>
rg -n "machDynamisch|nak311DynBand" <die vier Testdateien>
rg -n "auslenkungenDb|dynamikVorhanden|detektorLaeuft|huelle\.|auslenkung" <die vier Testdateien>
```

**Helfer, die ein dynamisches Band bauen:** `machDynamisch`
(`DspGoldenTestMain.cpp:183`, 38 Aufrufe — nur in dieser Datei),
`nak311DynBand` (`TransactionTestMain.cpp:2828`, Aufrufe `:2904`, `:2982`,
`:3087`), sonst direkte Zuweisungen (`TransactionTestMain.cpp:1100-1101`,
`:1150-1151`, `:1523-1524`, `:1575-1576`, `:2694-2695`).
`SondeNullTestMain.cpp` und `SchemaTestMain.cpp` tragen **null** Treffer — der
Satz „A16 fährt kein dynamisches Band im Nulltestmaterial" (`:7136`) stimmt.

**Ergebnis: neun Prüfungen mit dynamischem Band (Range ≠ 0) oder mit gelesenem
Detektor-/Hüllkurven-/Dynamikzustand stehen in KEINER der zwei Listen.** Die
Tabelle führt diese neun und die fünf von der Sammelzeile nur umschriebenen
Prüfungen; die 15 Zeilen des Abschnitts F und die 13 Einträge außerhalb, die
korrekt mit Testnamen und Fundstelle stehen, sind nicht wiederholt.

\| Datei:Zeile des Prüfnamens \| Prüfname \| Material \| was gelesen wird \| Liste 1 / Liste 2 / keiner \| Kann ein vorgeschalteter Ein-Pol von 10 ms auf der Detektorleistung Zahl oder Urteil verschieben? \|
\|---\|---\|---\|---\|---\|---\|
\| `DspGoldenTestMain.cpp:1625` \| `dynamisches_<typ>_ruhe_gegen_rbj (M-19, B-2) <gain> dB @<fs>` (16 Teilfälle) \| Impuls (`impulsantwort`, `:1613`); Band dynamisch, Range −12,0, **Sidechain::none** (`:1610`) \| Impulsantwort gegen RBJ, Schranken 0,05 / 0,1 dB \| **keiner** \| **nein** — `detektorLaeuft` ist schon wegen `Sidechain::none` falsch (`DspProgramm.cpp:318`); die Pegelstufe wird nie getickt \|
\| `DspGoldenTestMain.cpp:1659` \| `dynamischer_high_shelf_bei_minus_24_db_gegen_rbj (M-19, U15, B-2)` \| **Quadraturton** `fahreStereoTon` (L sin / R cos, `:447-449`), 0,5 bei 250 / 1000 / 4000 Hz, Threshold −60 \| `w[0] != -12.0` → `plateau` (Absolutzahl der Dynamik) **und** der gemessene Gang gegen RBJ −24 dB, Schranke 0,05 dB \| **keiner** \| **nein** — der Quadraturton hält die Detektorleistung konstant auf a²/2; der Fixpunkt eines Ein-Pol-Mittels bei konstantem Eingang ist bitgenau dieser Eingang, der Pegel bleibt 36 dB über Threshold, die Auslenkung exakt −12,0. Der Satz `:7134-7135` („Abschnitt C … trägt kein dynamisches Band in der gemessenen Größe") ist für diese Zeile **falsch**: die gemessene Größe sind −24 dB = `gain_db` −12 **plus** Auslenkung −12 \|
\| `DspGoldenTestMain.cpp:4376` \| `im_regulaeren_betrieb_ist_jede_auslenkung_endlich (M-113)` \| L = R-Sinus 0,9 bei 2000 Hz, 80 Blöcke; dynamisches Band Slot 4, Range −6,0 (`:4277`) \| `auslenkungenDb` — Endlichkeit **jedes** Werts und `nichtEndlicheAuslenkungen() == 0` \| **keiner** \| **nein** — geprüft werden Endlichkeit und ein Zähler, keine Zahl. Ein Ein-Pol mit endlichen Koeffizienten erzeugt aus endlichem Eingang keinen nicht-endlichen Zustand \|
\| `DspGoldenTestMain.cpp:4401` \| `nichtendliche_auslenkung_wird_null_und_gezaehlt (M-113, B-21)` \| wie oben, dazu `rangeDb = NaN` ins gefahrene Programm injiziert (`:4387`) \| `nachher[4] == 0.0`, kein Vorzeichenbit, Zähler +1 je Block \| **keiner** \| **nein** — die Kennlinie liefert mit NaN-Range in jedem Block NaN, unabhängig davon, ob ihr Eingang die Momentanleistung oder ein Pegel ist; der Riegel meldet exakt 0,0 \|
\| `DspGoldenTestMain.cpp:5298` \| `311/M-41 verdraengte_zwischenpublikation_bricht_den_lebenszyklus (NAK-311 T3-14-02, M-121, R-311-1)` \| Quadraturton 0,5 bei 1 kHz, 48 000 Samples (`:5278-5280`), dann Stille; Range −12,0, Threshold −40 \| `vorherA[0] == -12.0` (**Absolutzahl**), `uebernahmen == 1`, `slot0Max == 0.0`, `spitzeNachFade == 0.0` \| **keiner** (Liste 2 nennt den M-121-Block nur mit `:5242` und `:5256`; `311/M-43` bis `311/M-56` decken M-41 nicht) \| **nein** — Quadraturton, Plateau 31 dB über Threshold; der Fixpunkt eines Ein-Pol-Mittels bei konstantem Eingang ist bitgenau dieser Eingang, nach 48 000 Samples ist ein 10-ms-Pol (τ = 480) hundertfach eingeschwungen \|
\| `DspGoldenTestMain.cpp:5628` \| `jede Publikation des Messlaufs kam durch (kein busy_retry)` \| Sinus 0,5 / 0,4 (`:5501-5503`), acht dynamische Bänder je Programm, Range −6,0 (`:5481`) \| Zahl der abgewiesenen Publikationen \| **keiner** \| **nein** — gezählt wird `busy_retry`, keine Pegelgröße \|
\| `DspGoldenTestMain.cpp:5632` \| `worst_case_vier_baenke_laeuft (M-118)` \| wie oben \| Zahl gleichzeitig rechnender Bänke je Stichprobe \| **keiner** \| **nein** — Bankzählung, keine Pegelgröße \|
\| `DspGoldenTestMain.cpp:5643` \| `die Uebergangslast wurde gemessen, nicht gedeckelt (M-118, R15)` \| wie oben \| Mittelwert und Maximum der Echtzeitlast, Endlichkeit \| **keiner** \| **nein** — eine Lastzahl ohne Deckel; die zusätzliche Stufe kostet Rechenzeit, aber der Test prüft nur „gemessen und endlich" \|
\| `TransactionTestMain.cpp:1116` \| `undo_stellt_das_entfernte_band_als_ein_objekt_her (M-61)` \| **kein Audio** \| Undo-Objekt, Ausgang, Revision, `bestaetigt() == vorher` \| nur als Fundstelle `:1095-1107` in der Sammelzeile, **ohne Testnamen** \| **nein** — kein Kern läuft; der Grund der Sammelzeile („Register, Hash, Revision") trifft hier zu \|
\| `TransactionTestMain.cpp:1182` \| `freier_slot_verarbeitet_nichts (M-58)` \| **Rauschen** (`juce::Random w (58)`, 200 Blöcke à 256, zwei echte Kerne) \| Ausgang **bitgleich** zwischen zwei Kernen, `auslenkung[2] == 0.0`, `! progMit->baender[2].aktiv`, Auto-Gain bitgleich \| nur als Fundstelle `:1145-1156`, **ohne Testnamen** \| **nein** — Slot 2 ist **nicht belegt**, das Band ist inaktiv, es läuft kein Detektor. Der Grund der Sammelzeile („Register, Hash, Revision und Schemagültigkeit") ist für diese Zeile jedoch **falsch**: sie vergleicht Audio bitweise und liest eine Auslenkung \|
\| `TransactionTestMain.cpp:1542` \| `dynamicwerte_ueberleben_aus_und_ein_bitgleich (M-24, ueber zwei Transaktionen)` \| **kein Audio** \| fünf Parameterzellen bitweise, Revision, `kDynamicEnabled` \| nur als Fundstelle `:1523-1541`, **ohne Testnamen** \| **nein** — kein Kern läuft \|
\| `TransactionTestMain.cpp:1587` \| `klemmliste_erscheint_im_bericht (M-23, E2-7)` \| **kein Audio** \| Klemmliste des Berichts, `gekapptFreq`, persistenter Wert \| nur als Fundstelle `:1575`, **ohne Testnamen** \| **nein** — kein Kern läuft \|
\| `TransactionTestMain.cpp:2761` \| `311/M-40 remove_und_belegen_ohne_audio_startet_kalt (NAK-311 T3-14-02, SONDE-015 M-121)` \| **Quadraturton** 0,5 bei 1 kHz, 94 Blöcke à 512 (`:2708-2715`), dann Stille; Range −12,0, Threshold −40 \| `vorher[0] == -12.0` (**Absolutzahl**), `uebernahmen == 1`, Crossfade, `slot0Max == 0.0`, `spitzeNachFade == 0.0` \| nur als Fundstelle `:2685-2700`, **ohne Testnamen** \| **nein** — Quadraturton, Plateau; Fixpunkt bitgenau. Der Grund der Sammelzeile ist für diese Zeile **falsch**: sie fährt Audio und liest eine Absolutzahl der Dynamik \|
\| `TransactionTestMain.cpp:3008` \| `311/M-76 gleicher_ladestart_bleibt_warm (NAK-311 F12, M-84)` \| Quadraturton 0,5 bei 1 kHz, 240 000 Samples Vorlauf, dann 48 000 gemessen; Range −12,0, Threshold −15 (Knie) \| **Bitgleichheit** von Ausgang und Tap gegen einen ununterbrochenen Vergleichskern \| **keiner** \| **nein** — verglichen werden zwei gleich gebaute Läufe desselben Codes; eine zusätzliche Stufe wirkt in beiden identisch, und der Ladestart überträgt den `BandZustand` (siehe L-1) \|
\| `TransactionTestMain.cpp:3067-3074` \| `311/M-77 recall_in_neue_instanz_innerhalb_der_toleranz_pruefling_<1..3>` \| Quadraturton 0,5, drei Prüflinge, je Knie (−15) und Plateau (−40) \| **Toleranz ab t_E**: ab t_E ≤ 0,1 dB, davor ≤ \|g0\| + \|Range\| + 0,1 dB \| Liste 1, aber als Matrixkennung `311/M-77` ohne Testnamen und ohne Fundstelle (`:7086`) \| **ja, in Richtung „später"** — die neue Instanz startet die Pegelstufe kalt, der Einschwingvorgang verlängert sich; `:7086` beziffert das als „größer um höchstens 5·`kPegelFensterMs`·fs Samples". Die Zahl 0,1 dB bleibt, t_E wächst \|
\| `TransactionTestMain.cpp:3067-3074` \| `311/M-78 recall_nach_preparetoplay_innerhalb_der_toleranz_pruefling_<1..3>` \| wie M-77, `prepareToPlay` auf A selbst gegen die ununterbrochene Instanz C \| wie M-77 \| wie M-77 \| **ja, „später"**, aus demselben Grund \|
\| `TransactionTestMain.cpp:3103` \| `311/M-79 audiohistorie_ist_kein_state (NAK-311 F12, R-311-4)` \| Quadraturton 0,5 bei 1 kHz, 96 000 Samples; Range −12,0, Threshold −40 (**Plateau**) \| Statebytes vorher == nachher **und** `werte[0] == -12.0` (**Absolutzahl**) \| **keiner** (nur `:7172` unter „Tests und Beine") \| **nein** — Plateau, Quadraturton, Fixpunkt bitgenau; nach 2 s ist ein 10-ms-Pol eingeschwungen. Die Bytegleichheit hängt ohnehin nicht am Pegel, solange der neue Zustand — wie §40.4 `:7154` es vorsieht — im `BandZustand` und nicht im State liegt \|

**Die Recall-Prüfungen des Abschnitts T (`311/M-76` bis `M-80`), woran ihr
Urteil hängt**

\| Zeile \| Prüfung \| Das Urteil hängt an \|
\|---\|---\|---\|
\| `311/M-76` \| `gleicher_ladestart_bleibt_warm` (`:3008`) \| **Bitgleichheit zweier gleich gebauter Läufe** (Ausgang und Tap gegen den ununterbrochenen Kern) \|
\| `311/M-77` \| `recall_in_neue_instanz_…_pruefling_<n>` (`:3067`) \| **Toleranz ab t_E** (≤ 0,1 dB ab t_E, davor ≤ \|g0\|+\|Range\|+0,1 dB) \|
\| `311/M-78` \| `recall_nach_preparetoplay_…_pruefling_<n>` (`:3067`) \| **Toleranz ab t_E**, dieselbe Formel \|
\| `311/M-79` \| `audiohistorie_ist_kein_state` (`:3103`) \| **Bytegleichheit des States** UND einer **Absolutzahl der Dynamik** (`werte[0] == -12.0`) \|
\| `311/M-80` \| `reload_rekonstruiert_denselben_audioausgang (M-84)` (`:2198`), Matrixzeile `:692` \| **Hashgleichheit und Bitgleichheit zweier Läufe**; der Prüfling `:2181-2196` trägt **kein** dynamisches Band (zwei statische Bänder, Width 1,3, Auto-Gain, eine Zone) — er gehört deshalb nicht in die zwei Listen \|

### 4. Gegenargument

Gesucht:

- §40.4 `:7134-7136` will Abschnitt C, G, H, H2 und A16 sammelweise ausnehmen
  („tragen kein dynamisches Band in der gemessenen Größe"). Für A16, G, H und
  H2 halte ich das an der Quelle für richtig; für **Abschnitt C** ist es
  falsch (`:1659` liest `w[0] != -12.0` und misst −24 dB Gesamtgain), und die
  Form ist ohnehin genau die von R-311-18 verbotene Umschreibung ohne
  Testnamen.
- Die Sammelzeile `:7138-7141` nennt für die fünf `TransactionTestMain`-Stellen
  einen gemeinsamen Grund. Er trifft für drei (`:1095-1107`, `:1523-1541`,
  `:1575` — kein Audio), nicht für `:1145-1156` (bitweiser Audiovergleich,
  gelesene Auslenkung) und nicht für `:2685-2700` (Audio, Absolutzahl −12,0,
  Stillespitze).
- Ein Satz, der `311/M-76` oder `311/M-79` auffinge, existiert in §40.4 nicht;
  `:7172` nennt `311/M-79` nur als erweitertes Bein.
- Zählung: die zweite Liste führt 15 Tabellenzeilen des Abschnitts F (mit 17
  benannten Prüfungen) und 13 Zeilen außerhalb — die Zahlen in `:7919-7921`
  und M-128 (`:6358`) stimmen als Zeilenzahl. Gebrochen ist nicht die Zahl,
  sondern das Wort „jede".

### 5. Urteil

**BESTÄTIGT.** Alle vier Teilbefunde halten. Darüber hinaus: die Regel wird an
mindestens **neun** weiteren Prüfungen verfehlt, die in keiner der zwei Listen
stehen (`:1625`, `:1659`, `:4376`, `:4401`, `:5298`, `:5628`, `:5632`,
`:5643` in `DspGoldenTestMain.cpp`, dazu `311/M-76` und `311/M-79` in
`TransactionTestMain.cpp`), und **zwei** der fünf sammelweise begründeten
Fundstellen tragen einen sachlich falschen Grund.

### 6. Kleinste Korrektur (Vorschlag)

1. **Die Sammelzeile `:7138-7141` auflösen** in fünf Zeilen der zweiten Liste,
   je mit Testname, Fundstelle und eigenem Grund:
   `undo_stellt_das_entfernte_band_als_ein_objekt_her (M-61)` (`:1116`) — kein
   Audio; `freier_slot_verarbeitet_nichts (M-58)` (`:1182`) — Slot 2 ist frei,
   kein Detektor, deshalb `auslenkung[2] == 0,0` und bitgleicher Ausgang;
   `dynamicwerte_ueberleben_aus_und_ein_bitgleich (M-24, ueber zwei
   Transaktionen)` (`:1542`) — kein Audio;
   `klemmliste_erscheint_im_bericht (M-23, E2-7)` (`:1587`) — kein Audio;
   `311/M-40 remove_und_belegen_ohne_audio_startet_kalt` (`:2761`) — Plateau am
   Quadraturton, Fixpunkt bitgenau.
2. **`:7086` mit Testnamen und Fundstelle versehen:**
   `311/M-77 recall_in_neue_instanz_innerhalb_der_toleranz_pruefling_<1..3>`
   und `311/M-78 recall_nach_preparetoplay_innerhalb_der_toleranz_pruefling_<1..3>`
   (`eq-copilot/plugin/tests/TransactionTestMain.cpp:3067-3074`, Prüfstand
   `:3026-3077`).
3. **Zwei Zeilen in die zweite Liste aufnehmen:**
   `311/M-76 gleicher_ladestart_bleibt_warm (NAK-311 F12, M-84)` (`:3008`) —
   Grund: zwei gleich gebaute Läufe gegeneinander, bitgleich; und
   `311/M-79 audiohistorie_ist_kein_state (NAK-311 F12, R-311-4)` (`:3103`) —
   Grund: Plateau am Quadraturton, Fixpunkt bitgenau, und der neue Zustand
   liegt im `BandZustand`, nicht im State.
4. **Acht weitere Zeilen aufnehmen** (`:1625`, `:1659`, `:4376`, `:4401`,
   `:5298`, `:5628`, `:5632`, `:5643`) mit den Gründen aus der Tabelle oben;
   den Satz `:7134-7135` über Abschnitt C entweder streichen oder auf
   „Abschnitt C außer `dynamisches_<typ>_ruhe_gegen_rbj` und
   `dynamischer_high_shelf_bei_minus_24_db_gegen_rbj`" einschränken.
5. M-128 (`:6358`) trägt die Zählung mit: „fünfzehn Prüfungen des Abschnitts F"
   bleibt, die Zahl der Einträge außerhalb steigt.

---

## Fakten zur Lücke L-1 (keine Einordnung)

- Der Same-Instance-Ladestart läuft `setStateInformation` →
  `Transaktionskern::ladestart` (`SondeProcessor.cpp:1064-1069`) →
  `dspAusfuehrung->publiziereWirksam (transaktion->wirksam(), true)`
  (`:1080`, `erzwingen` = true) → `kern.uebernehmeZustand (…, Pfad::committed)`
  (`NakamaTransaktion.cpp:763`).
- `ladestart` (`NakamaTransaktion.cpp:234-270`) berührt keinen DSP-Zustand: es
  setzt `committed`, `r`, `r0`, Ring, Cursor, leert Register und Overlays.
- Die Publikation läuft durch `DspKern::publiziereVorbau`; dort wird die **neue
  Bank** genullt: `DspKern.cpp:353` `    bank.zustaendeNullen();`. Der laufende
  `BandZustand` bleibt davon unberührt.
- **Übernommen wird am Blockrand des Audiothreads**, nicht vom Worker. Bei
  gleicher Belegung und Topologie sind Pfad- und Slotkennungen gleich
  (`vergebeKennungen`, `DspKern.cpp:286-288`), `nurRampen` ist wahr
  (`:507`), und `DspKern.cpp:515` kopiert den Zustand **als Ganzes**:
  `        bankNeu.baender         = baenke.bank (alt).baender;`
- `BandZustand` (`DspBankPool.h:58-78`) enthält `statisch[2]`, `svf[2]`,
  `detektor[2]`, `huelle`, `schrittRest`, `svfVon`/`svfNach`, `auslenkungDb`.
  Ein neuer Pegelzustand als Feld dieses Structs — so sieht es §40.4 `:7154`
  vor („`DspBankPool.h` bekommt den Zustand im `BandZustand`") — wandert bei
  `:515` ohne weiteren Code mit, ebenso im Crossfade-Zweig je Slot mit gleicher
  Kennung (`:536-537`).
- Er wird am Ladestart also **weder genullt noch neu gebaut, sondern übernommen**
  — an genau derselben Stelle wie `huelle` heute. Zusätzlich greift das
  ausdrückliche Nullen in `DspKern.cpp:741-745`, wenn der Detektor am
  Fensterende nicht mehr läuft; §40.4 `:7156-7158` nimmt den Pegelzustand dort
  bereits auf (M-141).

---

## Beifang

- §40.4 `:7134-7135` („Abschnitt C von B6 … trägt kein dynamisches Band in der
  gemessenen Größe") ist an
  `DspGoldenTestMain.cpp:1659` falsch: der Test prüft `w[0] != -12.0` und misst
  den Gang bei −24 dB Gesamtgain, also `gain_db` −12 **plus** Auslenkung −12.
- §40.3 `:6862-6863` widerspricht `:6889-6890` derselben Seite (siehe V-1
  Nr. 4).

---

## Selbst nachgemessen

- `rampenKompatibel` (`DspProgramm.cpp:207-227`), `vergebeKennungen`
  (`DspKern.cpp:261-292`), der Blockrand (`:491-547`), `publiziereVorbau`
  (`:315-363`), das Nullen (`:741-745`), `detektorLaeuft` (`DspProgramm.cpp:318`).
- Der ganze Publikationsweg der Transaktion: `DspKernAusfuehrung::baueVor`,
  `publiziereVorbau`, `publiziereWirksam`, `publizierePreview`
  (`NakamaTransaktion.cpp:715-767`), `Transaktionskern::ladestart` (`:234-270`),
  `automationSchreiben` (`:316-324`), `wirksam` (`:340-346`),
  `SondeProcessor.cpp:263-272`, `:1040-1080`, `:1460`.
- Jede `Stand`- und `prozessor()`-Instanz in `TransactionTestMain.cpp` mit ihrer
  Publikationsfolge; jeder `setze`-Aufruf in `SondeNullTestMain.cpp`; beide
  `Nak311Stand`-Blöcke in `SchemaTestMain.cpp`; die sieben Zeilen der ersten
  und die drei Zeilen der zweiten Tabelle von §40.3 in
  `DspGoldenTestMain.cpp`.
- Alle 38 `machDynamisch`-Aufrufe samt Laufvariablen (`:2131`, `:2170`,
  `:2206`, `:2466`, `:2540`/`:2541`, `:2652`/`:2657`), die Signatur `:183-185`
  und das Zuweisungspaar `:187-188`.
- Alle `kDynamicEnabled`/`kDynamicRangeDb`-Stellen der vier Testdateien; alle
  `auslenkungenDb`-, `dynamikVorhanden`- und `huelle.`-Lesestellen.
- Die 33 neu eingeführten Angaben der Form `Datei:Zeile` in §38 bis §41 als
  Mengendifferenz gegen `137672b4`, dazu 124 neu eingeführte
  Fortsetzungsangaben `:Zeile`, davon alle zusageführenden Zeile für Zeile.
- `BandZustand` (`DspBankPool.h:58-78`), `zustaendeNullen` (`:87`).
- Nachgerechnet ohne Werkzeug: 1000/500 = 2,0 und 500/1000 = 0,5 sind in
  `double` exakt (beide Zahlen sind 125 mal eine Zweierpotenz); 9,0 − (−9,0) =
  18,0 dB; 20,0 − 18,0 = 2,0 dB; 12,0 − (−3,0) = 15,0 dB; 1,0/0,7071067811865476
  = 1,41421…; `(23 % 24) − 12 + 0,25 = 11,25` gegen `(24 % 24) − 12 + 0,25 =
  −11,75` → 23,0 dB; dieselbe Rechnung mit `+ 0,5` → 23,0 dB; `(i%12)` gegen
  `((i+8)%12)` → maximal 8,0; `280/200 = 1,4`.

## Nur gelesen / nicht geprüft

- Der Rohbericht der Matrixprüfung 5 in Gänze (D-2, D-3, D-4, R-311-17,
  R-311-19, R-311-20, Frage 2, Härtungen) — außerhalb meines Auftrags.
- §44 als Text; §1 bis §37, §42, §45; der unberührte Bereich von §38 bis §41.
- Die Zahlen der Regeln als solche (`kSprungGainDb`, `kSprungFrequenzVerhaeltnis`,
  `kSprungGueteVerhaeltnis`, `kPegelFensterMs`, `kAutoGainDeckelDb`); die
  Härtungen H-1 bis H-3; T3-12-*; Legacy-Bezeichner, Design, Stil.
- Jede Zahl, die erst die Bauetappe messen kann. Kein Bau, kein Test, kein
  FL Studio, kein Repo-Skript gefahren.
- Die Vollständigkeit meiner Publikationspaar-Suche in `DspGoldenTestMain.cpp`
  außerhalb der oben einzeln genannten Stellen (Abschnitt V-1, „Grenze meiner
  Suche").
- Ob `311/M-40` und `311/M-41` mit ihrer Absolutzahl −12,0 dB nach dem Bau
  wirklich halten: das misst die Bauetappe, nicht dieser Lauf.

---

**HEAD zu Beginn `1cfbc3f916bdcda7234b7cec0cda2e8dd818368d`,
HEAD am Ende `1cfbc3f916bdcda7234b7cec0cda2e8dd818368d`** — unverändert. Nichts
gestaget, nichts committet, nichts gepusht; die zwei fremden untracked Ordner
`briefing-hub/` und `nimbalyst-local/` sind unberührt.
