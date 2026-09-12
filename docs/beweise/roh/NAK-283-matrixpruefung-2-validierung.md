# NAK-283 Matrixprüfung 2 — Quellvalidierung der fünf Punkte

Lesender Validierer, 12.09.2026. Repo `C:\Users\phili\Projekte\Nakama`.
Stand beim Lauf: HEAD `2d30b41c` beim Start, während des Lesens auf
`cc957037` gewandert (nur `docs/beweise/NAK-283.md` §13 und die Urteilsdatei,
+35/−1); alle unten zitierten Ankerzeilen sind an beiden Ständen identisch,
`git status --short` ist leer, Produktcode unverändert (Audit-Basis
`ea6ddd57`). Nichts gebaut, nichts gefahren, Git nur lesend.

Gegenstand: Urteil `docs/beweise/roh/NAK-283-matrixpruefung-2-2d30b41.txt`
(Codex `gpt-6-astra` max, Thread `01a095b4-a787-7c52-9ca2-c4a085b9299a`),
NEEDS_WORK mit fünf Punkten über dem Fixdiff `6c3b5948...57aeae68` an
`docs/beweise/NAK-283.md`.

---

## Punkt 1 — „Kopple den Frischevergleich atomar an die Store-Annahme"

**Befund wörtlich.** Anker `docs/beweise/NAK-283.md:1007-1010`.
Titel: „Kopple den Frischevergleich atomar an die Store-Annahme".
Kern: „Befund 3 bleibt offen: A kann den neuen Wiedervergleich bestehen und
unmittelbar nach der Sperrfreigabe anhalten; B persistiert anschließend sein
jüngeres Ergebnis vollständig, danach schreibt A. Bei gleicher `finding_id`
gewinnt weiterhin A wegen seiner höheren Append-Ordnungszahl
(`broker/src/store/writer.rs:476,589–593`). … Vergleich und geordnete
Store-Annahme müssen zusammengehören; eine Änderung unter
`broker/src/store/**` ist dafür nicht zwingend, denn
`StoreHandle::append_einreihen` (`handle.rs:153–170`) trennt bereits die
Annahme vom Commit-Warten."

### Quellzitate an HEAD

`docs/beweise/NAK-283.md:1004-1010` (§6.4 „Risiken", die angegriffene Stelle):

```
**Risiken.** Sperrenordnung: die Menge der Eingangs-IDs wird **unter dem
Standlock** gebildet und **unter dem Standlock** verglichen; eine Bildung
außerhalb wäre derselbe Fehler in neuem Gewand. Der Wiedervergleich vor dem
Append nimmt den Standlock ein zweites Mal und **nicht geschachtelt**: er wird
vor `store.append` genommen und vor dem Append wieder freigegeben, weil
`befund_persistieren` ihn im Fehlerpfad erneut nimmt (`:511-514`) — eine
gehaltene Sperre wäre dort ein Deadlock.
```

`broker/src/coordinator/hypothese_verdrahtung.rs:494-515` (Fehlerpfad und
Sperre, der Deadlock-Grund des Bauers):

```
494	    fn befund_persistieren(&self, session: &SessionKey, befund: &CauseHypothesis) {
495	        let Some(store) = self.store.as_ref() else {
496	            return;
497	        };
498	        let payload = Self::befund_json(befund);
...
510	        event.event_type = "finding".into();
511	        if store.append(vec![event]).is_err() {
512	            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
513	            stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
514	        }
515	    }
```

`broker/src/store/handle.rs:148-170` (die Annahmekante, die Codex benennt):

```
148	    pub fn append(&self, events: Vec<StoreEvent>) -> Result<Vec<AppendAusgang>, StoreFehler> {
149	        let antwort_rx = self.append_einreihen(events)?;
150	        antwort_rx.recv().map_err(|_| StoreFehler::Beendet)?
151	    }
152
153	    /// Trennt die begrenzte Annahmekante vom spaeteren Commit-Warten. Der
154	    /// Aufrufer darf zwischen diesen Schritten insbesondere keinen
155	    /// Coordinator-Lock halten.
156	    pub fn append_einreihen(
157	        &self,
158	        events: Vec<StoreEvent>,
159	    ) -> Result<mpsc::Receiver<Result<Vec<AppendAusgang>, StoreFehler>>, StoreFehler> {
...
164	        let (antwort_tx, antwort_rx) = mpsc::channel();
165	        self.senden(WriterBefehl::AppendBatch {
166	            events,
167	            antwort: antwort_tx,
168	        })?;
169	        Ok(antwort_rx)
170	    }
```

`broker/src/store/handle.rs:219-247` (`senden` blockiert nicht — `try_send`):

```
219	    fn senden(&self, befehl: WriterBefehl) -> Result<(), StoreFehler> {
...
232	        match sender.try_send(befehl) {
233	            Ok(()) => { ... Ok(()) }
239	            Err(TrySendError::Full(_)) => {
240	                self.verweigerung_zaehlen();
241	                Err(StoreFehler::KanalVoll)
242	            }
```

`broker/src/store/writer.rs:475-477` und `:588-593` (Ordnungszahl und
Projektionsriegel):

```
475	    )?;
476	    let event_ord = tx.last_insert_rowid();
477	    projektionen_anwenden(tx, event, event_ord)?;
...
588	            "INSERT INTO {tabelle}({id_feld},project_binding_id,session_epoch,last_event_ord,state_jcs) \
589	             VALUES(?1,?2,?3,?4,?5) ON CONFLICT({id_feld}) DO UPDATE SET \
590	             project_binding_id=excluded.project_binding_id,session_epoch=excluded.session_epoch,\
591	             last_event_ord=excluded.last_event_ord,state_jcs=excluded.state_jcs \
592	             WHERE excluded.last_event_ord > {tabelle}.last_event_ord"
```

`broker/src/store/writer.rs:176-193` und `:294-303`, `:325-334` (ein einziger
Writer-Thread, FIFO-Kanal, geordnete Abarbeitung):

```
176	pub(super) fn writer_lauf(
177	    mut conn: Connection,
178	    receiver: Receiver<WriterBefehl>,
...
188	    loop {
189	        let befehl = if let Some(v) = vorgemerkt.pop_front() {
190	            Ok(v)
191	        } else {
192	            receiver.recv_timeout(Duration::from_millis(COMMIT_FENSTER_MS))
193	        };
...
294	                        match receiver.recv_timeout(rest) {
295	                            Ok(WriterBefehl::AppendBatch { events, antwort })
296	                                if !barriere_gesehen =>
297	                            {
298	                                aktiv.push_back(AppendStand { ... });
...
325	                    let ergebnisse = append_gruppe(&mut conn, &commit_jobs, test_haken.as_ref());
326	                    match ergebnisse {
327	                        Ok(pro_job) => {
328	                            for (index, ergebnis) in zuordnung.into_iter().zip(pro_job) {
```

### Bewertung

**Status: BESTÄTIGT.** Das Fenster, das Codex beschreibt, existiert am
geplanten Fixstand: `befunde_eintragen` (`:443-482`) hält den Standlock nur
über Prüfung und Cache-Eintrag und ruft `befund_persistieren` erst **nach**
der Freigabe (`:481-483`). Der neue Wiedervergleich sitzt innerhalb von
`befund_persistieren` **vor** `store.append` und gibt die Sperre vor dem
Append wieder frei (§6.4:1007-1009 sagt das ausdrücklich). Zwischen
Freigabe und Append kann B den ganzen Weg gehen; die Reihenfolge in der
Projektion entscheidet allein `last_event_ord` = `tx.last_insert_rowid()`
(`writer.rs:476`) gegen `WHERE excluded.last_event_ord > …`
(`writer.rs:592`), und die `finding_id` trägt die Eingangsmenge nicht.
A gewinnt die Projektion. R-283-1 („nie über ein jüngeres Ergebnis
schreiben", §2:111) bleibt verletzt.

**Frage des Dirigenten: Ist die Kopplung ohne Deadlock möglich und schließt
sie das Fenster? — Ja, beides.**

1. *Kein Deadlock.* Der Deadlock des Bauers entsteht **nur**, weil
   `befund_persistieren` bei `store.append` (blockierendes `recv`) den
   Standlock im Fehlerpfad **erneut** nimmt (`:511-514`, `std::sync::Mutex`,
   nicht reentrant). Mit `append_einreihen` zerfällt der Aufruf in eine
   nicht blockierende Annahme und ein separates `recv`. Die Annahme
   blockiert nachweislich nie: `senden` benutzt `try_send` und meldet bei
   vollem Kanal sofort `StoreFehler::KanalVoll` (`handle.rs:232-242`,
   `STORE_KANAL_CAP = 256` in `store/mod.rs:48`). Der Fehlerzweig der
   Annahme läuft also **unter der bereits gehaltenen Sperre** und darf den
   Zähler direkt am gehaltenen Guard erhöhen statt neu zu sperren; der
   Fehlerzweig des `recv` läuft nach der Freigabe und darf neu sperren. Der
   Doc-Kommentar der Annahmekante verlangt genau diese Aufteilung: „Der
   Aufrufer darf **zwischen diesen Schritten** insbesondere keinen
   Coordinator-Lock halten" (`handle.rs:153-155`) — gemeint ist das Warten,
   nicht die Annahme.
2. *Sie schließt das Fenster.* Die Annahmereihenfolge bestimmt die
   Ordnungszahl: ein einziger Writer-Thread liest einen FIFO-Kanal
   (`writer.rs:178`, `:188-193`), sammelt weitere Aufträge in
   `aktiv.push_back` (`:298`) und schreibt sie in derselben Reihenfolge über
   `commit_jobs`/`zuordnung` (`:325-334`), sodass `last_insert_rowid()`
   monoton der Annahmereihenfolge folgt. Vergleich und `append_einreihen`
   unter **demselben** Standlock heißt deshalb: besteht A den Vergleich,
   hat B seinen Cache-Eintrag noch nicht gesetzt; B nimmt die Sperre später,
   reiht später ein und bekommt die höhere Ordnungszahl — B gewinnt die
   Projektion, wie zugesagt. Besteht A den Vergleich nicht, schreibt A gar
   nicht.
3. *Ohne Änderung unter `broker/src/store/**`.* `append_einreihen` ist
   bereits `pub` (`handle.rs:156`) und wird heute schon von außen benutzt
   (`broker/tests/store_crash_matrix.rs:1527`, `:1532`, `:1676`, `:1705`,
   `:1712`). Die Pfadtrennung der Etappe 3 (§3, §6.4 „Nicht angefasst")
   bleibt unverletzt.

**Vorschlag Einordnung: DEFEKT** (gegen Regel R-283-1, §2:111, und gegen
das Audit-Korrekturziel „bis zu Ergebnisübernahme, Persistenz und
Publikation", `UEBERGABE-AN-CLAUDE.md:88`). Kein LÜCKE-Fall: die Regel
existiert und ist eindeutig; der Bauplan erfüllt sie nur nicht.

**Schließende Änderung.** §6.4 Risiken (`:1006-1010`), M-13 Fenster 2
(`:595`-Block, Spalte „Reihenfolge und Frist") und §8.1 Feinheit 18
(`:1620-1634`) schreiben die Kopplung: Standlock nehmen → Payload gegen den
Standeintrag vergleichen → `StoreHandle::append_einreihen` **unter derselben
Sperre** → Sperre freigeben → `recv` und Fehlerzählung außerhalb; der
Annahmefehler zählt am gehaltenen Guard, nicht über ein zweites `lock()`.

---

## Punkt 2 — „Leite M-44 aus der tatsächlich gezählten Hauptstufe ab"

**Befund wörtlich.** Anker `docs/beweise/NAK-283.md:640`.
Kern: „Bei dem festgelegten −55-dBFS-Signal ist `abdeckung` am Basisstand bei
allen vier Raten 0, nicht bei 44,1/48 kHz größer als 0:
`featureengine/Spektrum.h:369–383` zählt ausdrücklich ausschließlich die
Hauptstufe (`&s == &haupt`), deren Gate überall inaktiv bleibt. Die neue
Aussage, beide Stufen gingen in die Abdeckung ein, widerspricht deshalb dem
Code-Ist-Stand."

### Quellzitate an HEAD

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:360-384`:

```
360	    const double gesamt = summeBereich (s, 0, bins);
361	    const bool aktiv = gesamt > 0.0
362	                    && 10.0 * std::log10 (gesamt) > kAktivGateDb;
...
369	    // Gezaehlt wird nur die HAUPTstufe. Die Bassstufe hat ein anderes
370	    // Fenster- und Hopmass; beide zusammen zu zaehlen ergaebe ein
371	    // Verhaeltnis aus zwei verschiedenen Zeitachsen. Dieselbe Begruendung
372	    // steht beim Ereignisdetektor weiter unten.
373	    if (&s == &haupt)
374	    {
...
381	        constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
382	        if (evidenzFensterGesamt < kMax) ++evidenzFensterGesamt;
383	        if (aktiv && evidenzFensterAktiv < kMax) ++evidenzFensterAktiv;
384	    }
```

`eq-copilot/plugin/core/analysis/featureengine/Frame.h:486-490`:

```
486	    if (evidenzFensterGesamt > 0)
...
488	        f.abdeckungGesetzt = true;
489	        f.abdeckung = (float) ((double) evidenzFensterAktiv
490	                             / (double) evidenzFensterGesamt);
```

`eq-copilot/plugin/core/analysis/FeatureEngine.h:233-247`:

```
233	    static constexpr int kBassPunkte = 16384;
235	    static constexpr int kHauptPunkte = 4096;
247	    static constexpr double kAktivGateDb = -60.0;
```

Manifest `:640` (M-44, die beanstandeten Sätze, wörtlich):

> … und `abdeckung` ist bei allen vier **1** (jedes Fenster beider Stufen
> nimmt das Gate) … `abdeckung` zählt die Fenster **beider** Stufen
> (`evidenzFensterAktiv` / `evidenzFensterGesamt`, `Spektrum.h:383`,
> `Frame.h:486-491`) … bei 44,1 kHz nimmt nur die Bassstufe das Gate
> (−55 − 4,30 = **−59,30 dB**, Hauptstufe −55 − 10,32 = **−65,32 dB**) …
> `abdeckung` fällt dort auf 0, während sie bei 44,1 und 48 kHz über 0 liegt.

### Bewertung

**Status: BESTÄTIGT.** Der Zähler steht innerhalb von `if (&s == &haupt)`
(`Spektrum.h:373-384`), und der Kommentar `:369-372` begründet das
ausdrücklich. Die Bassstufe geht in `abdeckung` nicht ein. Mit den vom
Manifest selbst gerechneten Stufenwerten liegt die **Hauptstufe** unter der
Mutation bei −65,32 / −65,69 / −68,70 / −71,71 dB, also bei allen vier Raten
unter `kAktivGateDb = −60,0` — `abdeckung` ist überall 0. Die Sätze „jedes
Fenster beider Stufen nimmt das Gate", „`abdeckung` zählt die Fenster beider
Stufen" und „während sie bei 44,1 und 48 kHz über 0 liegt" widersprechen dem
Code-Ist-Stand.

Teilentlastung, die den Befund nicht aufhebt: die Zeile bleibt am Basisstand
**rot**, weil die Zusage „`abdeckung` ist bei allen vier 1" unter der
Mutation überall auf 0 fällt. Gebrochen ist nicht die Klassifikation, sondern
der **Rotbeweis als Beleg für R-283-4**: die Regel verlangt „dasselbe Signal
bei 44,1 kHz und 96 kHz erhält dieselbe Bewertung" (§2:114), und genau dieser
Ratenunterschied ist an der gezählten Größe nicht beobachtbar. Der Fehler
stammt aus der Nacharbeit selbst: §11.2 (b) (`:1819-1822`) hat die alte
Behauptung durch die Bassstufen-Aussage ersetzt.

**Vorschlag Einordnung: DEFEKT** (gegen Code-Ist-Stand und gegen die
Beweisregel „ein Rotbeweis fällt an der Zeile, die die Zusage trägt",
§5:517-522; zugleich gegen den Wortlaut von R-283-4).

**Schließende Änderung.** M-44 (`:640`) streicht die Aussage „beide Stufen"
und wählt einen Pegel, bei dem die **Hauptstufen**-Entscheidung unter der
Mutation zwischen den Raten kippt: aus den Konstanten gerechnet muss
`L − 10·log10 (fs / kHauptPunkte)` bei 48 kHz über und bei 96 kHz unter
−60 dB liegen, also **−49,31 dBFS < L < −46,30 dBFS**; mit L = **−48,0 dBFS**
ergibt sich unter der Mutation −58,32 / −58,69 / −61,70 / −64,71 dB
→ `abdeckung` 1 / 1 / 0 / 0, gegen die Fixfassung 1 / 1 / 1 / 1.

---

## Punkt 3 — „Klassifiziere M-73 nach ihrer vollständigen Zusage"

**Befund wörtlich.** Anker `docs/beweise/NAK-283.md:657`.
Kern: „M-73 verlangt neben gleicher Bewertung aller Blockgrößen ausdrücklich
die Übereinstimmung jedes Laufs mit der isolierten Passage. Diese zweite
Zusage verletzt bereits der Basisstand: Die vorab gebundene Passage übernimmt
über die laufende Filterhistorie das laute Vormaterial
(`FeatureEngine.h:621–622`, `featureengine/Spektrum.h:192–200`), genau das
F07-Gegenbeispiel aus M-49. Die gesamte Zeile ist daher heute rot und keine
Regressionswache … die ausgewiesenen 20 reinen Regressionswachen sind sonst
inhaltlich falsch."

### Quellzitate an HEAD

Manifest `:657` (M-73, Zusage und Klassifikation, wörtlich):

> **Das Passagenmaximum hängt nicht an der Blockgröße.** Die drei Läufe
> liefern denselben Wert innerhalb ±0,1 dB, und jeder gleicht dem isoliert ab
> Fensteranfang verarbeiteten Lauf aus M-49 … **Regressionswache (heute
> grün)** — am Basis-SHA gibt es an `passVon` keinen Reset, die
> Polyphasenkette läuft über die Blockgrenzen durch, und das (falsche)
> Passagenmaximum ist bei allen drei Blockgrößen dasselbe.

Manifest `:645` (M-49, dieselbe Bühne, gegenteilige Klassifikation):

> … (a) An `FeatureEngine.h:604-625`: `tp.zuruecksetzen()` wieder ans Binden
> legen → das Passagenmaximum springt um rund 60 dB. **ROT am Basis-SHA.**

`eq-copilot/plugin/core/analysis/FeatureEngine.h:611-622` (der Reset hängt am
**Bindezeitpunkt**):

```
611	        // 🔑 Der Polyphasenfilter wird geleert. Ein Passagenanfang IST eine
612	        // Fenstergrenze (§32.3): seine 24 Taps je Phase reichen zwoelf Samples
613	        // vor den Anfang zurueck, und ohne diesen Reset trug der erste
614	        // Passagenrahmen den Nachklang des Materials DAVOR. Genau daran hing
615	        // der Befund B08: eine leise Passage nach einem lauten Abschnitt
616	        // uebernahm dessen Spitze — nicht ueber einen Puffer, sondern ueber den
617	        // Filterzustand, die subtilste Form desselben Fehlers.
...
621	        (void) tp.nachlauf();
622	        tp.zuruecksetzen();
```

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:192-201` (die Kette
läuft über die Passagengrenze durch):

```
192	        const double tpJetzt = std::max (tp.tick (l, r), std::max (absL, absR));
193	        rahmenTruePeak = std::max (rahmenTruePeak, tpJetzt);
194	        // M-03/M-25: das PASSAGENmaximum zaehlt nur, was im Fenster lag.
...
197	        if (imPassagenfenster)
198	        {
199	            passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, tpJetzt);
200	            ++zelleImFensterSamples;
201	        }
```

Manifest `:707-711` (§5.6, die Liste der zwanzig):

> Die zwanzig Regressionswachen: M-04, M-12, M-14, M-15, M-18, M-19, **M-20**,
> **M-22**, M-28, M-29, **M-31**, M-33, **M-34**, M-37, M-38, M-42, M-48,
> M-55, M-64, **M-73**.

### Bewertung

**Status: BESTÄTIGT.** M-73 trägt zwei Zusagen. Die erste
(Blockgrößenunabhängigkeit) ist am Basisstand grün. Die zweite („jeder
gleicht dem isoliert ab Fensteranfang verarbeiteten Lauf aus M-49") ist am
Basisstand **rot** — und zwar aus demselben Grund, den M-49 als F07-Beleg
führt: Der Reset steht am Binden (`FeatureEngine.h:621-622`), die Passage von
M-73 wird **vorab** gebunden, danach läuft das laute 12-kHz-Vormaterial durch
den Interpolator, und `Spektrum.h:192-200` schreibt dessen Nachklang in
`passagenTruePeakRahmen` (M-49 beziffert den Sprung mit rund 60 dB). Eine
Zeile, deren Zusage am Basisstand fällt, ist nach der eigenen Definition in
§5.6 (`:678-681`: „*Regressionswache* heißt heute grün und messbar") keine
reine Regressionswache. Damit stimmt auch die Zählung nicht: 20 reine
Regressionswachen / 53 Baulücke.

**Vorschlag Einordnung: DEFEKT** (gegen die eigene Klassifikationsregel
§5.6:678-681 und gegen die Zählzusage §5.6; die Zeile behauptet „heute grün"
für eine Zusage, die heute fällt).

**Schließende Änderung.** M-73 (`:657`) trennt die beiden Zusagen — die
Blockgrößen-Gleichheit bleibt Regressionswache, die Übereinstimmung mit dem
isolierten Lauf wandert als Baulücke in M-49 beziehungsweise eine eigene
Hälfte — und §5.6 (`:685-711`) wird neu gezählt (Regressionswache 19,
Baulücke 54, M-73 als gemischte Zeile geführt).

---

## Punkt 4 — „Wähle für M-73 einen blockgrenzensensitiven Verlauf"

**Befund wörtlich.** Anker `docs/beweise/NAK-283.md:657`.
Kern: „Der übernommene 12-kHz-Ton hat bei 48 kHz eine Periode von vier
Samples; 64, 512 und 1024 sowie beide Passagengrenzen sind Vielfache davon.
Beim mutierten Reset an jedem Blockanfang entstehen deshalb dieselben
Einschwingantworten in derselben Signalphase; alle Teilstücke sind länger als
die FIR-Historie (`TruePeak.h:75–78,172–179`). Ihre häufigere Wiederholung
ändert das Passagenmaximum nicht (`featureengine/Spektrum.h:197–200`). Die
behauptete Abweichung über ±0,1 dB folgt somit nicht aus dieser Mutation."

### Quellzitate an HEAD

`eq-copilot/plugin/core/analysis/TruePeak.h:72-82` (Historienlänge):

```
72	    /** Ueberabtastfaktor.  Siehe Kopf: 8 ist nicht Geschmack, sondern die
73	        kleinste Zweierpotenz, deren Restfehler bei fs/4 unter 0,1 dB bleibt. */
74	    static constexpr int kFaktor = 8;
75	    /** Taps je Phase.  24 ergibt 193 Gesamttaps (ungerade, symmetrisch um die
76	        Mitte) und haelt das Passband bis 0,4*fs innerhalb 0,01 dB. */
77	    static constexpr int kTapsJePhase = 24;
78	    static constexpr int kTaps = kFaktor * kTapsJePhase + 1;   // 193
79	    /** Ringlaenge: die naechste Zweierpotenz ueber `kTapsJePhase`, damit die
80	        Indexrechnung eine Maske statt eines Modulo ist. */
81	    static constexpr int kRing = 32;
```

`eq-copilot/plugin/core/analysis/TruePeak.h:167-183` (das Maximum je Sample
über acht Phasen; nur die letzten ≤ 24 Eingangssamples wirken):

```
167	        const double* const h0 = koeffizienten();
168	        double groesster = 0.0;
169	        for (int phase = 0; phase < kFaktor; ++phase)
170	        {
171	            double accL = 0.0, accR = 0.0;
172	            for (int k = 0; phase + kFaktor * k < kTaps; ++k)
173	            {
174	                const double h = h0[(std::size_t) (phase + kFaktor * k)];
175	                const int idx = (stand - k) & (kRing - 1);
176	                accL += h * verlaufL[(std::size_t) idx];
177	                accR += h * verlaufR[(std::size_t) idx];
178	            }
179	            groesster = std::max (groesster, std::max (std::abs (accL), std::abs (accR)));
180	        }
```

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:55-86` (die
Blocklokalität, auf die sich die falsche Fixfassung stützt):

```
55	    int passVon = 0, passBis = n;
...
75	            const std::int64_t von = std::max (passagenfenster.startSample, b0);
76	            const std::int64_t bis = std::min (passagenfenster.endeSample, bEnde);
77	            if (bis > von)
78	            {
79	                passVon = (int) (von - b0);
80	                passBis = (int) (bis - b0);
```

Manifest `:657` (der beanstandete Bruch, wörtlich):

> … absichtlich gebrochen mit einem Reset an jedem `i == passVon` ohne
> Startmerker → bei 64 Samples je Block wird die Kette 750-mal je Sekunde
> geleert, bei 1024 nur rund 47-mal, und die drei Läufe fallen um mehr als
> ±0,1 dB auseinander.

### Bewertung

**Status: BESTÄTIGT.** Nachgerechnet an den Konstanten: 12 kHz bei 48 kHz
ergibt genau vier Samples je Periode. 64, 512 und 1024 sind Vielfache von 4,
und beide Fenstergrenzen (48 000, 96 000) ebenfalls — jeder Reset fällt
deshalb in dieselbe Signalphase. Die wirksame Historie ist auf
`kTapsJePhase = 24` Eingangssamples begrenzt (`TruePeak.h:77`, Schleife
`:172-178` mit `kTaps = 193 = 8·24+1`), also kürzer als das kürzeste
Teilstück (64 Samples = 16 volle Perioden). Jedes Teilstück erreicht damit
denselben eingeschwungenen Zustand und liefert dasselbe Maximum; das
Passagenmaximum ist ein `std::max` über alle Teilstücke
(`Spektrum.h:197-200`) und bleibt unverändert. Die Behauptung „die drei Läufe
fallen um mehr als ±0,1 dB auseinander" folgt aus dieser Mutation nicht — die
Wache fängt die falsche Fixfassung, gegen die sie geschrieben ist, nicht.

**Vorschlag Einordnung: DEFEKT** (gegen die Beweisregel „Ein Rotbeweis fällt
an der Zeile, die die Zusage trägt", §5:517-522; der Bruch kippt die
zugesagte Messgröße nicht — dieselbe Klasse wie die Defekte 4, 7, 9, 11 der
Matrixprüfung 1, §11.2 Ursache (b)).

**Schließende Änderung.** M-73 (`:657`) legt in die Passage einen **sparsamen
Transienten** statt eines Dauertons — etwa eine einzelne `fs/4`-Periode mit
Amplitude 1,00 auf leisem Grund, deren entscheidende Intersample-Spitze an
einem passagenrelativen Versatz liegt, der Vielfaches von 64, aber mehr als
`kTapsJePhase / 2` = 12 Samples von der nächsten 512er- und 1024er-Blockgrenze
entfernt ist; dann kappt nur der 64-Sample-Lauf die Historie vor der Spitze,
und die drei Läufe fallen messbar auseinander.

---

## Punkt 5 — „Belege die neu festgelegten Referenzparameter"

**Befund wörtlich.** Anker `docs/beweise/NAK-283.md:1583-1584`.
Kern: „Feinheit 14 schreibt erstmals 64-fache Überabtastung und eine
Referenzvalidierung auf ±0,01 dB verbindlich vor, ohne diese Zahlen durch eine
Quelle oder Herleitung zu begründen. Die zitierten Stellen in
Entwurf:2626–2627 und SONDE-013 M-02/M-81 tragen das Projektbudget ±0,1 dB,
nicht diese zusätzlichen Referenzvorgaben … Damit verletzt der Fix die
ausdrückliche Grenze „keine neuen Deckel oder Zahlen ohne Quelle"."

### Quellzitate an HEAD

Manifest `:1581-1584` (die beanstandete Festlegung):

```
1581	und darf deshalb nichts ändern. Die fehlende analytische Referenz ist damit
1582	kein Grund für eine andere Grenze, sondern ein Bauauftrag: die Referenz wird
1583	im Test gerechnet (64-fach-Überabtastung mit langem Fenster-Sinc) und vorher
1584	an den Fällen 15 bis 19 auf ±0,01 dB nachgewiesen (§6.7, M-65).
```

`docs/FL-Nakama-Sonden-Design-Entwurf.md:2624-2627` (die zitierte Quelle —
sie trägt nur ±0,1 dB):

```
2624	### 39.3 Abnahmekriterien
2625
2626	- Loudness und True Peak stimmen auf Standard-Testmaterial mit einer validierten Referenz
2627	  innerhalb ±0,1 LU beziehungsweise ±0,1 dB überein.
```

`docs/FL-Nakama-Sonden-Design-Entwurf.md:3599-3600`:

```
3599	Diese Budgets sind Hypothesen mit Abnahmetest. Die ausführende Phase darf sie nach Messung ändern,
3600	muss Änderung, Evidenz und Userwirkung aber versioniert dokumentieren.
```

`eq-copilot/plugin/core/analysis/TruePeak.h:72-87` und `:215-220` (die
vorhandene, benannte Herleitung im Produktcode):

```
72	    /** Ueberabtastfaktor.  Siehe Kopf: 8 ist nicht Geschmack, sondern die
73	        kleinste Zweierpotenz, deren Restfehler bei fs/4 unter 0,1 dB bleibt. */
74	    static constexpr int kFaktor = 8;
75	    /** Taps je Phase.  24 ergibt 193 Gesamttaps (ungerade, symmetrisch um die
76	        Mitte) und haelt das Passband bis 0,4*fs innerhalb 0,01 dB. */
77	    static constexpr int kTapsJePhase = 24;
...
84	    /** Kaiser-Beta.  8,0 ergibt rund -80 dB Sperrdaempfung; die Passbandwelligkeit
85	        liegt damit bei 24 Taps je Phase unter 0,01 dB — eine Groessenordnung
86	        unter der Toleranz, die dieser Pfad einhalten muss. */
87	    static constexpr double kKaiserBeta = 8.0;
...
215	    static constexpr double interpolationsfehlerDb (double fRelFs) noexcept
216	    {
217	        const double halbeStufe = 1.0 / (2.0 * (double) kFaktor);
218	        const double c = cosKonst (2.0 * kPi * fRelFs * halbeStufe);
219	        return c > 0.0 ? 20.0 * log10Konst (c) : -400.0;
220	    }
```

`docs/beweise/NAK-180.md:1289` (bereits beglaubigter Präzedenzfall derselben
Herleitung, Bein B17):

> Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene
> Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB)
> und reisst mit Faktor 4 (-0,169 dB).

### Bewertung

**Status: TEILWEISE.** Codex hat recht, dass die **zitierten** Stellen die
Zahlen nicht tragen: Entwurf `:2626-2627` und SONDE-013 M-02/M-81 nennen
ausschließlich ±0,1 dB gegen eine validierte Referenz, und Feinheit 14
(`:1583-1584`) sowie M-65 (`:667`) schreiben 64-fach und ±0,01 dB ohne
Herleitung hin. Damit verletzt der Fixdiff die Auftragsgrenze und die eigene
Manifestregel „jede neue Zahl trägt Datei und Zeile oder ist aus benannten
Konstanten gerechnet" (§5, §11.3 E).

**Nicht bestätigt** ist der implizite Schluss, es gebe keine tragfähige
Quelle. Sie existiert im Produktcode und ist bereits beglaubigt:
`interpolationsfehlerDb` (`TruePeak.h:215-220`) ist die geschlossene
Fehlerschranke, mit der NAK-180 B17 schon die 8-fach-Entscheidung als Zahl
getroffen hat. Nachgerechnet mit derselben Formel bei `fRelFs = 0,25`:
Faktor 4 → −0,169 dB, Faktor 8 → −0,042 dB (beide decken sich exakt mit der
beglaubigten B17-Aussage), Faktor **64 → −0,00065 dB**, also gut eine
Größenordnung unter dem geforderten ±0,01 dB und zwei unter dem
Projektbudget ±0,1 dB. Die ±0,01 dB selbst stehen als Passband- und
Welligkeitsgrenze bereits im selben Quelltext (`:75-77`, `:84-87`).
Extern tragen ITU-R BS.1770 (Mindestforderung 4-fache Überabtastung für True
Peak) und EBU Tech 3341 (Toleranz 0,0 +0,2/−0,4 dBTP für die Fälle 15 bis 23)
diese beiden Zahlen **nicht** — beide Normen kennen weder 64-fach noch
±0,01 dB; die Begründung ist projektintern und rechnerisch, nicht normativ.

**Vorschlag Einordnung: LÜCKE** — es fehlt die Regel, dass eine Zahl, die
nicht aus der zitierten Zusage stammt, ihre Herleitung mitführt. Kein
Rückbau auf ±0,1 dB nötig: die Grenze ist haltbar, nur unbelegt notiert.

**Schließende Änderung.** Feinheit 14 (`:1583-1584`) und M-65 (`:667`)
führen die Herleitung mit: 64-fach und ±0,01 dB sind aus
`TruePeak.h:215-220` (`interpolationsfehlerDb`, Restfehler bei fs/4:
Faktor 64 → −0,00065 dB) plus `:75-77`, `:84-87` gerechnet, Präzedenz
`docs/beweise/NAK-180.md:1289` (B17) — ausdrücklich als projektinterne
Rechengrenze der Referenz, nicht als Normforderung.

---

## Gemeinsame Ursache

Alle fünf Punkte entstehen aus **einem** Muster: die Nacharbeit 1 hat aus dem
*zugesagten Mechanismus* argumentiert statt aus einer *Messung am
Basiscode*.

- Punkt 1: die Reichweite der Frischeprüfung wurde am Coordinator entschieden,
  ohne die Annahmekante des Stores zu lesen (`handle.rs:153-170`,
  `try_send` nicht blockierend, Writer FIFO) — deshalb galt „Sperre über den
  Append = Deadlock" als Ausschluss der einzigen tragenden Kopplung.
- Punkt 2: die Messgröße `abdeckung` wurde beschrieben, ohne die
  Zählbedingung `if (&s == &haupt)` zu lesen (`Spektrum.h:373`).
- Punkte 3 und 4: die Wache M-73 wurde aus der erwarteten Wirkung der falschen
  Fixfassung abgeleitet, ohne die zweite eigene Zusage am Basisstand zu
  prüfen (Reset am Binden, `FeatureEngine.h:621-622`) und ohne Periode,
  Blockraster und Historienlänge nachzurechnen (`TruePeak.h:77`).
- Punkt 5: eine Zahl wurde als bindend gesetzt, obwohl ihre Herleitung im
  Quelltext daneben lag und in NAK-180 B17 schon einmal beglaubigt wurde.

Daraus folgt eine übertragbare Regel für Runde 2: **jede Zahl, jede
Messgröße und jede Sperrenaussage einer Matrixzeile wird an der Zeile des
Basiscodes belegt, die sie trägt** — Zählbedingung, Sperrenordnung,
Kanalsemantik und Signalperiode eingeschlossen; und jede Zeile mit zwei
Zusagen wird je Zusage klassifiziert.

## Scope-Beweis

- Alle fünf Punkte liegen innerhalb der zwei Fragen des Prüfauftrags
  (`docs/beweise/roh/NAK-283-matrixpruefung-2-auftrag.txt:27-28`): Punkt 1
  ist Frage 1 (Befund 3 der Matrixprüfung 1), Punkt 4 ist Frage 1
  (Befund 10), die Punkte 2, 3 und 5 sind Frage 2 (gebrochene Matrixzeile,
  Zählung §5.6, Feinheit ohne Quelle). Kein Punkt greift auf §0 bis §4, §7,
  §9, §10 oder auf Produktfragen U40 bis U42 über; keine ausgeschlossene
  Härtung (NAK-255, NAK-258, NAK-269, NAK-270 bis NAK-280) ist berührt.
- Geändert wurde nichts: `git status --short` ist leer, HEAD `cc957037`
  (beim Start `2d30b41c`; der Unterschied sind ausschließlich §13 des
  Manifests und die Urteilsdatei, +35/−1, durch den Dirigenten). Alle oben
  benutzten Ankerzeilen (`:640`, `:657`, `:1007`, `:1583`) sind an beiden
  Ständen identisch. Produktcode unverändert seit `ea6ddd57`; nicht gebaut,
  nichts gefahren, Git nur lesend. Einzige Schreibstelle ist diese Datei im
  Scratchpad.
- Nebenbeobachtung, **kein** eigener Befund und nicht Teil der fünf Punkte:
  die Zählzeile in §11 (`:1775`) nennt „44 heute rot … 2 halb/halb", §5.6
  (`:686-693`) nennt „45 heute rot … 1 gemischt" bei gleicher Summe 53. Die
  Neuzählung nach Punkt 3 berührt beide Stellen ohnehin.
