# NAK-286 — Validierung der Codex-Matrixprüfung 1, Teil A (lesend, 14.09.2026)

Ticket NAK-286 (Plan S25e). HEAD `a4c7128be5fce1e3e74fdd9f8d0810ec8f40906e`,
Branch `master`. **Nur gelesen** — nichts gebaut, kein Test, kein Skript, keine
FL-Instanz, Git nur lesend; geschrieben wurde ausschließlich diese Datei. Die
untracked Ordner `briefing-hub/` und `nimbalyst-local/` blieben unberührt.

Teil A prüft die Befunde **1, 5 und 9** des Urteils
`docs/beweise/roh/NAK-286-matrixpruefung-1-0523c07.txt` (Analyse, Toleranzen).

**Zeilenversatz: null.** Das Urteil zitiert `docs/beweise/NAK-286.md` zum Stand
`0523c079`. `git diff 0523c079..HEAD -- docs/beweise/NAK-286.md` ändert genau
eine Zeile innerhalb der Kopftabelle (Zeile 10, „Etappe") und hängt danach §9
mit 14 Zeilen an. Vor §9 verschiebt sich keine Zeile; alle Zeilenangaben des
Urteils gelten unverändert an HEAD. Stichprobe: Urteil `:578` = M-67 = HEAD 578;
Urteil `:587` = M-71 = HEAD 587; Urteil `:404-408` = F-28 = HEAD 404-408.

---

## Befund 1 — „Passe die Snapshot-Vorbedingung an die Loop-Resets an"

**Wörtlich (Urteil), `docs/beweise/roh/NAK-286-matrixpruefung-1-0523c07.txt:9-10`:**

> `[P1] Passe die Snapshot-Vorbedingung an die Loop-Resets an —
> C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:578-578`
> DEFEKT M-67/F-28: Beide Worker setzen ihre Analyse-Engine an Loopgrenzen
> zurück (`eq-copilot/plugin/src/prozessor/Analyse.cpp:267–273`,
> `eq-copilot/plugin/sonde/SondeProcessor.cpp:595–600`); dabei werden
> Lautheitsintegration und `aktiveZellen` geleert
> (`eq-copilot/plugin/src/AnalyseEngine.cpp:311–312`). Zwei Umläufe des
> 45,696-s-Loops ergeben deshalb keine ≥60 aktiven Messsekunden. Unmittelbar
> nach dem zweiten Umlauf liegen erneut Anlaufdaten vor. Das Szenario würde
> auch mit korrekten Plugins scheitern; Messsequenz und Kalibrier-Vorbedingung
> müssen diese bestehende Reset-Semantik berücksichtigen.

### Quelle an HEAD

**(1) Gen — `eq-copilot/plugin/src/prozessor/Analyse.cpp:262-274`:**

```cpp
                    const auto grenzenVorher = merkmale.getrennteFenster();
                    const auto straddlesVorher = merkmale.straddleVerworfen();
                    if (merkmale.nimmBlock (*frei.block, frei.audio))
                        merkmalFrames.fetch_add (1);

                    const bool featureGrenze = merkmale.getrennteFenster() != grenzenVorher;
                    const bool blockVerworfen = merkmale.straddleVerworfen() != straddlesVorher;
                    if (featureGrenze)
                    {
                        engine.zuruecksetzen();
                        unverarbeitet = 0;
                        kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
                    }
```

**(2) Probeeq — `eq-copilot/plugin/sonde/SondeProcessor.cpp:592-601`:** derselbe
Aufbau, `analyseEngine.zuruecksetzen()` plus `producerStandLeeren()`.

**(3) Was der Reset leert — `eq-copilot/plugin/src/AnalyseEngine.cpp:309-312`:**

```cpp
    zellenStand = 0;
    zelleKEnergie = zelleAktivEnergie = 0.0;
    loudness.zuruecksetzen();     // SONDE-008: leert Histogramm und Ring, ohne Allokation
    aktiveZellen = 0;
```

**(4) Woran die Zusage hängt — `eq-copilot/plugin/src/AnalyseEngine.cpp:748-752`:**

```cpp
    const double aktivS = (double) aktiveZellen * 0.1;
    s.aktivSekunden = aktivS;
    s.zustand = samplesGesamt == 0        ? MessZustand::keineDaten
              : aktivS < kMindestAktivS   ? MessZustand::sammelt
                                          : MessZustand::messbereit;
```

mit `kMindestAktivS = 15.0` (`eq-copilot/plugin/src/AnalyseEngine.cpp:17`),
`aktiveZellen` = „Zellen über dem −60-dBFS-Gate", je Zelle 100 ms
(`eq-copilot/plugin/src/AnalyseEngine.h:344`). Beide Felder gehen genau so in
die Antwort: `aktiv_sekunden` (`eq-copilot/plugin/src/prozessor/Analyse.cpp:1303`)
und `lufs_integriert` aus demselben `loudness`-Akku
(`eq-copilot/plugin/src/AnalyseEngine.cpp:703-705`,
`eq-copilot/plugin/src/prozessor/Analyse.cpp:1311`).

**(5) Dass der Loop-Wrap wirklich eine Grenze ist —
`eq-copilot/plugin/core/analysis/FeatureEngine.h:651-654`:**

```cpp
        const Grenzgrund grund = habeVorigen ? grenzeZwischen (vorigerBlock, block)
                                             : Grenzgrund::keine;
        if (grund != Grenzgrund::keine)
            grenzeZiehen (grund);
```

`grenzeZwischen` liefert für den Rücksprung
`Grenzgrund::loopWrap` beziehungsweise `zeitSprung`
(`eq-copilot/plugin/core/analysis/featureengine/Zeit.h:213-222`), und
`grenzeZiehen` zählt jede Grenze in genau den Zähler, den die beiden Worker
oben abfragen: `++zGetrennteFenster`
(`eq-copilot/plugin/core/analysis/featureengine/Zeit.h:337`).

**(6) Zweiter, unabhängiger Weg zum selben Reset:** liegt die Schleifengrenze im
Hostblock und sind Cycle- und PPQ-Felder gültig, fällt zusätzlich
`grenzeZiehen (Grenzgrund::moeglicherStraddle)` und der Block wird verworfen
(`eq-copilot/plugin/core/analysis/FeatureEngine.h:663-670`); daneben zieht ein
Kontinuitätsbruch der Quarantäne denselben `engine.zuruecksetzen()`
(`eq-copilot/plugin/src/prozessor/Analyse.cpp:244-252`). Ein Umlauf trifft die
Engine also nicht auf einem, sondern auf bis zu drei Wegen.

**(7) Dass FL das liefert — `eq-copilot/identity/host-capabilities-fl-v1.json:47,49`
(Messtermin B, gemessen, nicht angenommen):**

> „gueltig_immer.project_time_samples = true (ebenso continous_time_samples,
> tempo, ppq_position, bar_position, cycle_bounds, sample_rate);
> projektzeit.spruenge_rueckwaerts = 51 … **Loop-Wraps tragen exakt die
> Songlaenge (-539634/-539635)**"

**(8) Loop-Länge — gemessen, nicht angenommen:**
`docs/beweise/roh/NAK-283-laufzeit-7ad6b6f2.md:57`

> `| 5 | transport.getLength | {} | {"bars": 27, "milliseconds": 45696,
> "seconds": 46, "success": true, "ticks": 10236} | ok |`

Die 45,696 s aus §2.4 F-21, §2.3 F-19 und M-67 sind damit belegt.

### Gegenstand

**M-67, HEAD-Zeile 578** (Vorbedingung und Zusage, gekürzt):

> „frischer Start (M-59); Wiedergabe ab 0 mit Loop Song; **zwei volle Umläufe**
> (`lokal.umlauf` über `transport.getPosition`, M-18); dann eine Anfrage …
> `snapshot.zustand` = `messbereit`; **`snapshot.aktiv_sekunden` ≥ 60**;
> `snapshot.loudness.lufs_integriert` ∈ [−22,58; −22,31] + Δ_K … **Anfrage
> höchstens 1 s nach dem zweiten Umlauf**"

**F-28, HEAD-Zeile 410:** „`snapshot.zustand` = `messbereit`,
`snapshot.aktiv_sekunden` ≥ 60 (K5, `KALIBRIER-PROTOKOLL.md:135-138`)."

**Die Herkunft der 60 — `eq-copilot/kalibration/KALIBRIER-PROTOKOLL.md:135-138`:**

> „**K5 Protokollregel Material-Identität:** Für jeden Vergleich MUSS die App
> exakt das gerenderte Audio hören — **den Render selbst in FL loopen**
> (≥60 s aktiv), Messung nach dem Render neu starten."

### Status: BESTÄTIGT — und schärfer als das Urteil

1. **Der Reset an der Loopgrenze ist belegt**, auf beiden Wegen und für beide
   Rollen (Quellen 1 bis 3), die Grenzerkennung ebenso (5, 6), und dass FL den
   Wrap als Rücksprung meldet, ist am Capability-Bericht gemessen (7). Der
   Prüfer hat hier nichts nachgeplappert und nichts übertrieben.
2. **Die Vorbedingung ist nicht nur nach zwei Umläufen, sondern auf diesem
   Projekt überhaupt nicht erreichbar.** `aktiv_sekunden` zählt 100-ms-Zellen
   über dem −60-dBFS-Gate; ein Umlauf dauert 45,696 s, also höchstens 457
   Zellen = **45,7 s**, und real weniger, weil Intro und Ausklang Zellen unter
   dem Gate enthalten (genau das misst §2.4 F-21 als „Anteil der Zellen
   zwischen −66 und −40 dB"). ≥ 60 s verlangt zwingend Material über zwei
   Umläufe hinweg — genau das, was jede Grenze wegräumt. Das gilt unabhängig
   davon, ob die Anfrage nach dem zweiten Umlauf oder irgendwann sonst kommt.
3. **Der vom Auftrag erwogene Ausweg „ohne Loop" existiert nicht.**
   `fl_set_loop_mode` schaltet zwischen **Pattern- und Song-Modus**, nicht die
   Schleife ab (MCP-Repo, `src/fl_studio_mcp/tools/transport.py:127-143`:
   „Set the loop mode between pattern and song", `mode` ∈ {`pattern`, `song`},
   Befehl `transport.setLoopMode`). Und Wiedergabe über das Loop-Ende hinaus
   hilft nicht: hinter der Musik steht Stille, und Stille erzeugt keine aktive
   Zelle.
4. **M-67 verfehlt mit einem korrekten Bau drei seiner fünf Zusagen, nicht
   eine.** Eine Anfrage ≤ 1 s nach dem Wrap trifft eine Engine mit
   `aktiveZellen` ≈ 0 bis 10:
   `aktiv_sekunden` ≥ 60 verfehlt; `zustand` = `messbereit` verfehlt, weil
   `kMindestAktivS` = 15 s (Quelle 4) den Zustand auf `sammelt` hält; und
   `lufs_integriert` integriert unter einer Sekunde des leisen Intros und liegt
   damit weit außerhalb von [−22,58; −22,31] + Δ_K. Ergebnis: Exit 4 bei
   fehlerfreiem Produkt. Der Rotbeweis der Zeile („Bandmitte LUFS-I um 1,0 LU
   verschoben") kann das nicht auffangen, weil er an einer Attrappenantwort
   hängt, nicht an der Messsequenz.
5. **Die tiefere Ursache steht in K5.** Die ≥ 60 s sind aus einer
   Protokollregel vom 16.08.2026 übernommen, die ausdrücklich das Loopen des
   Renders verlangt. Diese Regel ist unter der heutigen Grenzsemantik nicht
   erfüllbar; die Matrix hat sie übernommen, ohne sie gegen die Quelle zu
   messen. Damit ist die Zusage nicht nur unerreichbar, sie widerspricht auch
   der eigenen Arbeitsregel von §2 („je Matrixzeile fällt der Rotbeweis an der
   Zeile, die die Zusage trägt") und der Regel aus F-17, Voraussetzungen zu
   messen statt anzunehmen.
6. Offene Nebenfrage, die die schließende Änderung mitnehmen sollte: das
   Kalibrier-Protokoll hält für Runde 01 **keine** aktiven Sekunden fest
   (`KALIBRIER-PROTOKOLL.md:62-69`; Runde 02a nennt „209,3 s aktiv" und
   „36,6 s aktiv", Runde 01 nichts). Die Vergleichsbasis der Anker ist also
   selbst nicht dokumentiert — wer die Vorbedingung neu setzt, sollte das
   ausdrücklich als Rohzeile mitschreiben statt es erneut anzunehmen.

**Einordnung (Vorschlag): DEFEKT.** Der Beweisweg trägt die Zusage nicht: das
Szenario endet mit korrektem Produkt Exit 4, und die Vorbedingung ist auf dem
Diagnoseprojekt strukturell unerreichbar. Kein Fall von „Matrix schweigt".

**Schließende Änderung (Vorschlag).** Messpunkt in **einen** Umlauf legen statt
hinter zwei: frischer Start (M-59), Wiedergabe ab 0, Anfrage an einer über
`transport.getPosition` geprüften Position **kurz vor dem Loopende** (Vorschlag:
Position ≥ 40 s und ≤ 45,0 s, Anfrage und erwartete Antwort vollständig vor dem
Wrap — dieselbe Positionsklammer-Mechanik wie F-22). Vorbedingung wird
`snapshot.zustand` = `messbereit` plus eine aus dem ersten Lauf gemessene
Untergrenze für `aktiv_sekunden` (Größenordnung 30 bis 45 s, Zahl aus der
Messung, nicht aus K5). Die Übernahme von K5 in F-28 fällt weg, mit datiertem
Satz im Etappenabschnitt, dass K5 aus einer Fassung vor der heutigen
Grenzsemantik stammt (Zeile für `docs/offene-punkte.md`, damit das
Kalibrier-Protokoll nicht still weiter das Gegenteil sagt). Zusätzlich bekommt
M-67 einen Rotbeweis an der eigenen Zusage: eine Antwort, die nach einem Wrap
eingeholt wird, muss die Vorbedingung **verfehlen** — das ist die Mutation, die
ein Zurückrutschen des Messpunkts fängt. Folge für §2.6: der Zuschlagsterm (b)
in F-28 („Blockraster über zwei Umläufe") entfällt ersatzlos, siehe Befund 9.

---

## Befund 5 — „Verwende kumulative Evidenzzähler für die U40-Differenzen"

**Wörtlich (Urteil), `…-0523c07.txt:21-22`:**

> `[P2] Verwende kumulative Evidenzzähler für die U40-Differenzen —
> … NAK-286.md:587-587`
> DEFEKT M-71/F-6/F-23: Die vorgesehenen Getter in
> `eq-copilot/plugin/core/analysis/FeatureEngine.h:939–940` liefern Zähler des
> aktuell offenen Evidenzfensters, keine kumulativen Summen.
> `featureengine/Frame.h:137–154` leert beide bei jedem Evidenzabschluss.
> Differenzen zwischen Antworten im Sekundentakt können daher null, negativ
> oder bloße Teilfensterdifferenzen sein, obwohl dazwischen viele Fenster
> gemessen wurden. Damit liefern sie nicht die nach R-286-4 zugesagten
> Messzahlen je Passage. Der Bauplan braucht kumulative beziehungsweise beim
> Abschluss erfasste Zähler und einen Test über mehrere Evidenzabschlüsse.

### Quelle an HEAD

**(1) Die vorgesehenen Getter — `eq-copilot/plugin/core/analysis/FeatureEngine.h:936-940`:**

```cpp
    /** Fensterzaehler der Abdeckung: gesamt und aktiv. Beide muessen an einer
        Grenze fallen, sonst waere die Abdeckung ein Anteil ueber zwei
        Epochen. */
    std::uint64_t evidenzFensterGesamtJetzt() const noexcept { return evidenzFensterGesamt; }
    std::uint64_t evidenzFensterAktivJetzt()  const noexcept { return evidenzFensterAktiv; }
```

**(2) Geleert bei jedem Evidenzabschluss —
`eq-copilot/plugin/core/analysis/featureengine/Frame.h:137-154`:**

```cpp
    rahmenLeeren();
    if (f.evidenzFrisch)
        evidenzLeeren();
    …
inline void FeatureEngine::evidenzLeeren() noexcept
{
    …
    evidenzFensterGesamt = 0;
    evidenzFensterAktiv = 0;
```

**(3) Die Engine sagt es selbst —
`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:462-473`:**

```cpp
    if (&s == &haupt)
    {
        // Saettigend statt umlaufend: … Beide Zaehler werden bei jedem
        // Evidenzframe geleert, der Fall ist also theoretisch; …
        constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
        if (evidenzFensterGesamt < kMax) ++evidenzFensterGesamt;
        if (aktiv && evidenzFensterAktiv < kMax) ++evidenzFensterAktiv;
    }
```

**(4) Wie oft das passiert:** `kEvidenzIntervallMinS = 0.25` (4 Hz),
`kEvidenzIntervallMaxS = 1.0`, Voreinstellung `evidenzIntervallS { kEvidenzIntervallMinS }`
(`FeatureEngine.h:438-439`, `:1335`). Zwischen zwei Antworten im Sekundentakt
(F-22) liegen also **ein bis vier** Abschlüsse — die Zähler stehen bei der
zweiten Antwort auf dem Füllstand eines frisch begonnenen Fensters. Zusätzlich
nullt sie jede Grenze (`featureengine/Zeit.h:382-383`).

**(5) Kumulative Zähler existieren — aber nicht für Evidenzfenster.** Die
monotonen Diagnosezähler der Engine stehen in `FeatureEngine.h:1485-1489`
(`zGetrennteFenster`, `zEpochenwechsel`, `zSegmentwechsel`, `zStraddleVerworfen`,
`zEreignisseVerworfen`, `zNak29Abgelehnt`, `zBloecke`, `zVerworfeneBandfenster`)
mit Auskünften in `:863-870` und `:1036`. Für „Fenster gesamt" und „Fenster
aktiv" gibt es keinen. Gesucht und nichts gefunden: `evidenzFensterGesamt` und
`evidenzFensterAktiv` kommen im ganzen Analysekern nur an den oben zitierten
Stellen vor.

**(6) Was dagegen kumulativ ist:** `merkmalFrames` wird nur hochgezählt
(`eq-copilot/plugin/src/PluginProcessor.h:1633`,
`eq-copilot/plugin/src/prozessor/Analyse.cpp:265`) und nirgends genullt. Δ
`frames_gebaut` aus M-71 ist also tragfähig.

**(7) Was die Engine bereits fertig liefert —
`eq-copilot/plugin/core/analysis/featureengine/Frame.h:507-514`:**

```cpp
inline void FeatureEngine::fuelleAbdeckungUndKonvergenz (FeatureFrame& f) const noexcept
{
    if (evidenzFensterGesamt > 0)
    {
        f.abdeckungGesetzt = true;
        f.abdeckung = (float) ((double) evidenzFensterAktiv
                             / (double) evidenzFensterGesamt);
    }
```

Der Anteil aktiver Fenster **je abgeschlossenem Evidenzfenster** steht also
schon im Rahmen und schon in der Feldliste von F-6/F-23.

### Gegenstand

**M-71, HEAD-Zeile 587:** „je Stelle und Rolle Zahl der Antworten,
Δ `frame.frames_gebaut`, **Δ `frame.evidenz_fenster_aktiv` und
Δ `frame.evidenz_fenster_gesamt`**".

**F-23, HEAD-Zeile 389:** „Δ `evidenz_fenster_aktiv` gegen
Δ `evidenz_fenster_gesamt` (Analysefenster, die das Aktivgate genommen haben,
gegen alle) … Geprüft wird nur Plausibilität: … **Δ aktiv ≤ Δ gesamt**".

**§7.1 T-15, HEAD-Zeile 887** — hier steht die Annahme ausgeschrieben:
„Gezählt werden beide ehrlich benannt: Δ `evidenz_fenster_aktiv` gegen
Δ `evidenz_fenster_gesamt` (Fenster) und Δ `frames_gebaut` (Rahmen)".

**R-286-4, HEAD-Zeile 385:** „… dazu **die Zahl der Rahmen, die als gemessen
gelten**". **Karte U40** (`docs/plan/fragen.json`, Kennung U40) fragt, ob der
Advisor nach der F06-Korrektur auf leisem Material sprechen soll; vertagt mit
dem User-Wort vom 12.09.2026 „bis die Wirkung der Korrektur gemessen ist … der
Dirigent misst selbst … und legt Messwerte vor, keine Blindwahl". Die Zahl, die
diese Karte entscheidet, ist genau der Anteil der Analysefenster, die an den
leisen Stellen das Aktivgate nehmen.

### Status: BESTÄTIGT — mit drei Ergänzungen

1. **Der Kern stimmt wörtlich.** Die Getter sind Fensterzähler; sie fallen bei
   jedem Evidenzabschluss, und die Engine schreibt das selbst als Kommentar hin
   (Quelle 3). Eine Differenz zweier Antworten im Sekundentakt beschreibt
   nichts, was zwischen den Antworten gemessen wurde.
2. **Nur zwei der drei Differenzen sind betroffen.** Δ `frames_gebaut` ist
   tragfähig (Quelle 6); das Urteil sagt das nicht, und die Nacharbeit sollte
   die Zeile nicht pauschal umbauen.
3. **Der Schaden ist größer als „unbrauchbare Zahlen": die Zeile kann einen
   korrekten Bau rot machen.** F-23 und M-72 prüfen „Δ aktiv ≤ Δ gesamt" als
   Erwartung, und ein verfehltes Szenario ist Exit 4 (R-286-5). Beispiel mit
   echten Wertebereichen: Antwort A mitten im Fenster (gesamt 3, aktiv 2),
   Antwort B eine Sekunde später kurz nach einem Abschluss (gesamt 1, aktiv 1)
   ergibt Δ gesamt = −2, Δ aktiv = −1, und −1 ≤ −2 ist falsch → Exit 4 ohne
   jeden Produktfehler. Die Zusage ist damit nicht nur leer, sie ist in sich
   widersprüchlich.
4. **F-6 mischt in derselben Antwort zwei Zeitachsen.** `abdeckung` und
   `evidenz_fenster` stammen aus dem zuletzt **gebauten** Rahmen
   (`merkmale.frame()` liefert `aktuell`, gesetzt in `Frame.h:135` **vor**
   `evidenzLeeren()`), also aus dem gerade abgeschlossenen Fenster; die beiden
   Zähler kommen dagegen live aus den Gettern und beschreiben das frisch
   begonnene Fenster. Die drei Felder nebeneinander sind kein zusammengehöriges
   Tripel.
5. **R-286-4s „Zahl der Rahmen, die als gemessen gelten" liefert heute kein
   einziger Zähler.** `frames_gebaut` zählt gebaute Rahmen, nicht
   gate-nehmende; die gate-nehmende Größe lebt ausschließlich in den
   Fensterzählern. Ohne Änderung bleibt die Karte U40 ohne die Zahl, für die
   der User sie vertagt hat.

**Einordnung (Vorschlag): DEFEKT.** Die Zusage in M-71/F-23 ist am Code nicht
haltbar, sie kann Exit 4 auslösen, und sie verfehlt die Zusage von R-286-4 und
die Erwartung der Karte U40. Kein Fall von „Matrix schweigt".

**Schließende Änderung (Vorschlag).** Zwei Wege tragen; sie schließen einander
nicht aus, und die Wahl ist Technik:
*(a)* Neben die vorhandenen monotonen Diagnosezähler (`FeatureEngine.h:1485-1489`)
ein kumulatives Paar stellen, das in `Spektrum.h:471-472` mitläuft und **weder**
in `evidenzLeeren()` **noch** in `grenzeZiehen()` fällt — dieselbe Sorte
Auskunft wie `zGetrennteFenster`, ohne Messsemantik anzufassen; F-6 nimmt die
neuen Namen auf, M-71 differenziert nur noch kumulative Zähler. Das liegt
innerhalb dessen, was der Bauplan ohnehin tut (F-6 sieht für Probeeq schon
einen neuen atomaren Rahmenzähler vor) und rührt keine Zeile aus §5.9 an.
*(b)* Ohne Engine-Änderung: je gezählter Antwort `frame.abdeckung` und
`frame.evidenz_fenster` roh mitschreiben (beide stehen schon in F-23) und je
Stelle als mit `evidenz_fenster` gewichtetes Mittel ausweisen; das beantwortet
U40 aus dem, was die Engine bereits fertig rechnet (Quelle 7).
In beiden Fällen: die Erwartung „Δ aktiv ≤ Δ gesamt" aus F-23/M-72 fällt oder
wird auf die kumulativen Zähler umgestellt, und M-71 bekommt einen Rotbeweis
über **mehrere** Evidenzabschlüsse (Attrappenfolge mit mindestens zwei
Abschlüssen zwischen zwei Antworten), nicht nur den heutigen
`roh_schreibt_null_als_null`.

---

## Befund 9 — „Begründe die zusätzlichen Snapshot-Toleranzen"

**Wörtlich (Urteil), `…-0523c07.txt:33-34`:**

> `[P2] Begründe die zusätzlichen Snapshot-Toleranzen —
> … NAK-286.md:404-408`
> DEFEKT F-28/M-67/M-68: Die behauptete LUFS-Schranke folgt nicht aus der
> angegebenen Rechnung: Das relative −10-LU-Gate ist eine untere Aufnahmegrenze,
> kein oberer Energiedeckel für die sechs weggelassenen Blöcke. Zusätzlich
> werden ≤0,05 LU, ±10 Hz und weitere Zuschläge ausdrücklich ohne Herleitung
> als Annahmen eingeführt. Diese Zahlen entscheiden in M-67/M-68 dennoch über
> Exit 4. Die Kalibrieranker ersetzen keinen Beleg für diese zusätzlichen
> Toleranzen; sie müssen vor ihrer Verwendung als Abnahmekriterien aus
> Messdaten oder einer gültigen Abschätzung hergeleitet werden.

### Gegenstand — die Rechnung, HEAD-Zeile 404

> „**LUFS integriert** ∈ [−22,58; −22,31] + Δ_K. Anker −22,41 (offline, Datei)
> und −22,45/−22,48 (Plugin, Livespiel); Spannweite der Anker enthält die
> Mess-Integrität 0,07 LU. Zuschlag je Seite 0,10 LU: (a) Loop-Kürzung — der
> Loop endet bei 45,696 s, die Musik bei rund 46,3 s (124 s − 77,7 s); die
> fehlenden rund 0,6 s sind höchstens 6 der rund 457 Blöcke (400 ms, Schritt
> 100 ms) eines Umlaufs; **selbst am relativen Gate (−10 LU) trägt ein Block
> ein Zehntel der mittleren Energie, zusammen ≤ 6 × 0,1/457 = 0,13 % =
> 0,006 dB**; (b) Blockraster über zwei Umläufe (Versatz 0,96 Schritt je
> Umlauf): **Annahme ≤ 0,05 LU, nicht hergeleitet**; (a) + (b) = 0,056,
> aufgerundet 0,10."

dazu HEAD-Zeile 406 („Zuschlag ±10 Hz = **Annahme** Livespiel gegen Render
≤ 1,1 %, nicht hergeleitet"), 407 („Zuschlag 0,015 = Integrität 0,01 plus
**Annahme** 0,005 für den Loop"), 408 („Integrität 0,02; Zuschlag 0,005") und
die Schlussregel 411 („Verfehlt ein Wert sein Band, ist das Exit 4 und ein
Befund an der Quelle …; die zwei Annahmen oben werden im ersten Lauf der
Etappe 4 gegen die Messung gelesen und im Etappenabschnitt bestätigt oder mit
Beleg ersetzt").

### Quelle an HEAD

**(1) Das relative Gate — `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:283-307`:**

```cpp
    bool integriert (double& heraus) const noexcept
    {
        if (anzahlAbs == 0)
            return false;
        const double gammaR = lautheit (summeAbs.wert() / (double) anzahlAbs) - 10.0;
        …
            if (binMitte (b) > gammaR)
            {
                s2.addiere (binSumme[(std::size_t) b]);
                n2 += binAnzahl[(std::size_t) b];
            }
        …
        heraus = lautheit (s2.wert() / (double) n2);
```

Das Gate ist eine **Aufnahmebedingung** (`binMitte > gammaR`) und das Ergebnis
ein **Mittelwert über die aufgenommenen Blöcke** (`s2 / n2`), keine Summe. Der
Kopfkommentar sagt dasselbe (`:15-17`: „1. absolutes Gate … 2. relatives Gate:
Gamma_r = Loudness(Mittel der absolut-gegateten z_j) …").

**(2) Die Anker und die Integrität — `eq-copilot/kalibration/KALIBRIER-PROTOKOLL.md:68-73`:**

> „Offline LUFS −22,41 · TP −7,87 · Centroid 829 Hz · low_frac 0,78 ·
> width 0,33 · corr 0,50 …
> Plugin LUFS −22,45/−22,48 · TP −7,75/−6,47 · Centroid 876/879 · width 0,328 ·
> corr 0,507 …
> **Mess-Integrität: BESTANDEN.** LUFS Δ≤0,07 LU · TP Δ0,12 dB (Sensor) ·
> width/corr Δ<0,01 · low_frac Δ0,02"

**(3) Die Bandkonstruktion ist rechnerisch konsistent** (nachgerechnet):
LUFS [−22,58; −22,31] = [−22,48 − 0,10; −22,41 + 0,10]; TP [−7,99; −6,35] =
[−7,87 − 0,12; −6,47 + 0,12]; Centroid [866; 889] = [876 − 10; 879 + 10];
width [0,313; 0,345] = 0,33/0,328 ± 0,015; corr [0,485; 0,522] = 0,50/0,507 ±
0,015; low_frac [0,755; 0,805] = 0,78 ± 0,025. Die **Integritätshälften** sind
also gemessen (Quelle 2); die **Zuschläge darüber** sind es nicht.

### Status: BESTÄTIGT, mit einer Verschärfung und einer Einschränkung

**Verschärfung 1 — die Richtung ist falsch.** Der Prüfer hat recht: `> gammaR`
ist eine untere Aufnahmegrenze. Ein Block, der das relative Gate nimmt, trägt
**mindestens** ein Zehntel der mittleren Energie; nach oben ist er unbeschränkt.
Die Rechnung in F-28 verwendet den kleinstmöglichen Beitrag als wäre er der
größtmögliche.

**Verschärfung 2 — die Rechnung ist auch bei ihrer eigenen Annahme
arithmetisch falsch.** LUFS-I ist ein Mittel über die gegateten Blöcke, nicht
eine Summe (Quelle 1); fallen 6 von 457 Blöcken weg, sinkt auch der Nenner.
Mit N = 457 und einem Energiefaktor f der weggelassenen Blöcke, bezogen auf das
Mittel der übrigen, ist die Verschiebung 10·log10(457 / (451 + 6·f)):

- f = 0,1 (F-28s eigene Annahme, Blöcke gerade am Gate): **+0,052 dB** — das
  Neunfache der behaupteten 0,006 dB und schon mehr als die Hälfte des ganzen
  Zuschlags.
- f = 1 (Blöcke auf dem Mittel): 0,000 dB.
- f = 2,8 (Blöcke 4,4 dB über dem Mittel): −0,10 dB, der Zuschlag ist
  aufgebraucht.
- f = 10 (Blöcke 10 dB über dem Mittel, mit dem Gate vereinbar): −0,49 dB.

Der Zuschlag 0,10 LU ist damit weder nach unten noch nach oben belegt.

**Verschärfung 3 — auch die Eingangszahl der Rechnung ist eine Ableitung, keine
Messung.** „die Musik bei rund 46,3 s (124 s − 77,7 s)" folgt aus der
Protokollnotiz „124 s — davon 77,7 s Stille"
(`KALIBRIER-PROTOKOLL.md:66`) unter der stillschweigenden Annahme, die Stille
liege vollständig am Ende. Das ist plausibel (24 durchgehende Takte,
`tools/eq-copilot/erzeuge_testsong.py:10-12`), aber es ist **messbar**: §2.4
F-21 rechnet für S3 ohnehin das Zellenenergieprofil der Quelle, aus dem die
letzte Zelle über dem Gate direkt abfällt.

**Verschärfung 4 — Term (b) hat nach Befund 1 keine Grundlage mehr.** „Blockraster
über zwei Umläufe" setzt eine Lautheitsintegration voraus, die über den
Loop-Wrap hinweg läuft. Genau das tut die Engine nicht (Befund 1, Quellen 1
bis 3). Mit dem korrigierten Messpunkt (eine Anfrage innerhalb eines Umlaufs)
entfällt der Term ersatzlos, und (a) beschreibt dann nicht mehr die
Loop-Kürzung, sondern den Abstand zwischen Anfragezeitpunkt und Musikende.

**Einschränkung — „ohne Herleitung eingeführt" ist kein Vorwurf des Verbergens.**
F-28 benennt (b) und die ±10 Hz selbst wörtlich als „Annahme … nicht
hergeleitet", und Zeile 411 verlangt ausdrücklich, sie im ersten Lauf der
Etappe 4 gegen die Messung zu lesen und nie still zu ändern. Der Prüfer
referiert hier also den Text der Matrix, keinen versteckten Mangel. Der
tragende Teil des Befundes ist ein anderer und bleibt bestehen: **die
unbelegten Zahlen entscheiden über Exit 4 in genau dem Lauf, der sie erst
belegen soll.** Ein Fehlschlag ist dann nicht zuzuordnen — zu enges Band oder
echte Abweichung —, und das widerspricht der Schlussregel von F-28 selbst („ein
Befund an der Quelle — Referenz oder Messung").

**Sind die Toleranzen aus Messdaten herleitbar? Ja, teilweise sofort.**

- Die Integritätshälften **sind** aus Messdaten: LUFS ≤ 0,07 LU, TP 0,12 dB,
  width/corr < 0,01, low_frac 0,02 (`KALIBRIER-PROTOKOLL.md:71-73`, Runde 01,
  15.08.2026). Sie bleiben.
- Der Rasterterm ist eine reine Offline-Rechnung über
  `eq-copilot/kalibration/Testtrack.wav`: dieselbe gegatete Blockfolge bei
  mehreren 100-ms-Rasterversätzen auswerten, die Spannweite ist die Zahl. Sie
  braucht kein FL und keinen Bau. (Der historische Offline-Anker stammt aus
  `tools/analyze-track.py`, das seit dem Workspace-Umzug außerhalb dieses Repos
  im FL-Studio-Repo liegt, Register NAK-31 — die Rechnung kann alternativ im
  vorhandenen Prüfstand des `LoudnessAccumulator` laufen.)
- Der Kürzungsterm wird aus dem Zellenenergieprofil von F-21 gemessen, sobald
  der Messpunkt nach Befund 1 feststeht.
- Die ±10 Hz auf dem Centroid sind heute **nicht** herleitbar: es gibt keinen
  gemessenen Vergleich Livespiel gegen Render. Diese Zahl gehört deshalb im
  ersten Lauf der Etappe 4 als `roh` mitgeschrieben und erst danach als Band
  gesetzt.

**Einordnung (Vorschlag): DEFEKT** für die LUFS-Herleitung in F-28 (falsche
Richtung, dazu Mittel statt Summe, dazu ein nach Befund 1 gegenstandsloser
Term) — der Bandwert entscheidet Exit 4 in M-67. Für die übrigen Zuschläge
(±10 Hz, 0,005) ist es kein neuer Befund gegen die Matrix, sondern ein bereits
in F-28 Zeile 411 offengelegter Punkt, der jetzt geschlossen statt vertagt
werden muss; wer die Einordnung enger fassen will, kann diesen Teil als
**LÜCKE** führen (die Matrix schweigt dazu, wie ein Bandfehlschlag von einer
echten Abweichung unterschieden wird).

**Schließende Änderung (Vorschlag).** F-28 Zeile 404 neu rechnen, mit dem
Messpunkt aus Befund 1: Term (b) streichen; Term (a) als gemessene Zahl aus dem
Zellenenergieprofil und der Rasterrechnung über `Testtrack.wav` einsetzen, mit
der Formel 10·log10(N / (N − k + Σf)) statt der Energieanteilsrechnung, und die
Herkunft jeder Zahl in der Zeile nennen. Für jede Toleranz, die nach dieser
Rechnung noch eine Annahme bleibt (heute: ±10 Hz Centroid, 0,005 auf
width/corr und low_frac): das Feld im ersten Lauf der Etappe 4 als `roh`
mitschreiben statt als Band prüfen, und M-67/M-68 erst nach dem gemessenen
Wert auf ein Band setzen. Dazu eine Zeile in M-67/M-68, die einen
Bandfehlschlag von der Toleranzfrage trennt: verfehlt genau ein Feld sein Band,
während die übrigen sitzen, ist der Befund gegen die Toleranz zu führen, nicht
gegen den Bau (heute steht nur „ein Befund an der Quelle — Referenz oder
Messung" ohne Entscheidungsregel).

---

## Gemeinsame Ursachen

Der Verdacht des Dirigenten stimmt zur Hälfte und ist an einer Stelle
zu korrigieren:

- **Bestätigt (1 und 9):** dieselbe Wurzel. Sowohl die Vorbedingung von M-67
  („zwei volle Umläufe", ≥ 60 aktive Sekunden) als auch der Zuschlagsterm (b)
  in F-28 („Blockraster über zwei Umläufe") setzen voraus, dass die Messung
  über Loopgrenzen hinweg akkumuliert. Der Code tut das ausdrücklich nicht.
  Eine Nacharbeit, die nur M-67 anfasst, lässt die Rechnung in F-28 mit einer
  toten Voraussetzung stehen.
- **Zu korrigieren (5):** nicht der Loop-Reset ist die Ursache, sondern ein
  Nachbar davon. Die Evidenzzähler fallen bei **jedem Evidenzabschluss** (alle
  0,25 bis 1 s), unabhängig von jeder Loopgrenze; die Grenze nullt sie
  zusätzlich. Die gemeinsame Klasse mit Befund 1 ist allgemeiner und trägt beide:
  **die Matrix liest Engine-Zustand als kumulativ, der auslegungsgemäß je
  Fenster beziehungsweise je Grenze fällt.** Wer die Nacharbeit an dieser Klasse
  aufhängt statt am Loop, fängt beide Zeilen und die Rechnung in F-28 mit.
- **Eigenständig (9, zweiter Teil):** die Toleranzen ohne Herleitung sind eine
  eigene Ursache — eine Abnahmegrenze, die aus einer Annahme statt aus einer
  Messung stammt, und die kein Kriterium mitbringt, mit dem ein Fehlschlag
  zugeordnet werden kann.

---

## Schlusstabelle

| Befund | Status | Einordnung (Vorschlag) | Schließende Änderung (Kurz) |
|---|---|---|---|
| 1 — M-67/F-28, Snapshot-Vorbedingung gegen Loop-Reset | BESTÄTIGT, schärfer: die Vorbedingung ist auf dem Diagnoseprojekt überhaupt unerreichbar (ein Umlauf = höchstens 45,7 aktive Sekunden), `fl_set_loop_mode` schaltet nur Pattern/Song, und M-67 verfehlt mit korrektem Bau drei Zusagen (auch `zustand` = `messbereit`, `kMindestAktivS` = 15 s) | DEFEKT | Messpunkt in einen Umlauf legen (Anfrage kurz vor dem Loopende, Position geprüft); Vorbedingung `messbereit` plus gemessene Untergrenze statt ≥ 60; K5-Übernahme streichen, datierte Zeile nach `docs/offene-punkte.md`; Rotbeweis „Antwort nach dem Wrap muss verfehlen" |
| 5 — M-71/F-6/F-23, kumulative Evidenzzähler | BESTÄTIGT, mit drei Ergänzungen: Δ `frames_gebaut` ist tragfähig; die Prüfung „Δ aktiv ≤ Δ gesamt" kann einen korrekten Bau auf Exit 4 werfen; F-6 mischt in einer Antwort das abgeschlossene und das frisch begonnene Fenster | DEFEKT | Kumulatives Zählerpaar neben `zGetrennteFenster` (fällt weder bei `evidenzLeeren` noch bei `grenzeZiehen`) oder Auswertung über das gewichtete `frame.abdeckung`; „Δ aktiv ≤ Δ gesamt" fällt oder wandert auf die kumulativen Zähler; Rotbeweis über mehrere Evidenzabschlüsse |
| 9 — F-28/M-67/M-68, Toleranzen ohne Herleitung | BESTÄTIGT, mit Verschärfung und Einschränkung: das relative Gate ist eine Aufnahme-, keine Deckelgrenze; zusätzlich rechnet F-28 mit einer Summe statt dem Mittel und unterschätzt schon die eigene Annahme um das Neunfache (+0,052 statt 0,006 dB); Term (b) entfällt nach Befund 1; die Annahmen sind aber in F-28 Zeile 404-411 selbst offengelegt | DEFEKT für die LUFS-Herleitung; für ±10 Hz und 0,005 alternativ LÜCKE (keine Regel, die einen Bandfehlschlag von der Toleranzfrage trennt) | F-28 Zeile 404 neu rechnen (Term (b) streichen, Term (a) aus Zellenenergieprofil und Rasterrechnung über `Testtrack.wav`, Formel über das Mittel); jede verbleibende Annahme im ersten Lauf der Etappe 4 als `roh` statt als Band; Zuordnungsregel für einen Bandfehlschlag in M-67/M-68 |
