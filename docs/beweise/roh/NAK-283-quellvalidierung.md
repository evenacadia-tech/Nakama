# NAK-283 — Quellvalidierung der Befunde F01 bis F14 und N01 des zweiten Codeaudits (12.09.2026)

Drei lesende Opus-Agenten (Effort max), gestartet vom Dirigenten am 12.09.2026 um
12:40 Uhr an HEAD `39e18d3f` (Produktcode identisch mit der Audit-Basis `ea6ddd57`).
Quelle: `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md` §2, §4, §5, §8
und `pakete/`. Keine Repros, keine Injektionen, keine Builds; Git nur lesend. Die
Einordnung Defekt / Lücke / Härtung trifft der Dirigent in `docs/beweise/NAK-283.md` §1;
die Vorschläge der Agenten stehen hier je Befund.

| Teil | Befunde |
|---|---|
| A | F01, F02, F03, F11, F13, F14 |
| B | F04, F05, F09, F12 |
| C | F06, F07, F08, F10, N01 |

---

# Teil A — F01, F02, F03, F11, F13, F14

# NAK-283 — Quellvalidierung A (F01, F02, F03, F11, F13, F14)

**Lauf:** lesende Quellvalidierung am HEAD `39e18d3f`, 12.09.2026.
**Basisabgleich:** `git diff --stat ea6ddd57 HEAD -- eq-copilot/ broker/ tools/` ist **leer** —
der Produktcode am HEAD ist bytegleich mit der Auditbasis. Alle vom Audit genannten
Zeilennummern gelten damit unverändert.
**Prüfform:** ausschließlich Quellenlektüre und lesendes Git. Kein Bau, kein Test, kein Repro,
keine Fehlerinjektion (User-Stopp 12.09.2026). Die Repro-Logs des Audits sind hier historische
Belege, keine Beweisgrundlage — jede Aussage unten steht auf dem Quelltext am HEAD.

---

## F01 — „Mitgliederpublikation kann innerhalb derselben Reload-Generation zurückspringen"

*§2-Titel: „Sources-Modell übernimmt nach erfolgreichem Unbind wieder einen älteren
Mitgliederstand" · Priorität laut Audit: **P2***

### Quellzitat an HEAD

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1444-1447` (Kommentarkopf, der die Zusage behauptet):

```
void EqCopilotProcessor::meldeSourcesMitgliederNachBefehl (std::uint64_t generationBeimAbholen)
{
    // Der AKTUELLE Stand, nicht eine beim Anwenden gezogene Kopie: zwei
    // Drains, die ihre Nachfuehrung nacheinander fahren, reichen dem Modell
    // so nie einen aelteren Stand als den zuletzt angewandten nach.
```

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1461-1477` (Kopie unter Sperre, Publikation ohne Sperre):

```
    std::vector<nakama::state::MainProjectMitglied> kopie;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (reloadGeneration.load() != generationBeimAbholen)
        {
            sourcesNachfuehrungNachReloadUnterblieben.fetch_add (1);
            return;
        }
        kopie = zustand.mainProjectMitglieder;
    }
    // NAK-246 M-39: der Testhaken zwischen Kopie und Publikation, ohne
    // gehaltene Sperre. Im Produkt leer.
    if (sourcesPublikationHakenFuerTest)
        sourcesPublikationHakenFuerTest (kopie.size());
    if (! sourcesModel.setzePersistenteMitglieder (kopie, generationBeimAbholen))
```

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1500-1535` (Rahmen: Riegel endet vor der Publikation):

```
        std::lock_guard<std::mutex> drain (sourcesDrainMutex);
        const auto abgeholt = bestaetigteSourcesCommandsAbholen();
        const auto& befehle = abgeholt.befehle;
        if (befehle.empty())
            return;
        …
        std::lock_guard<std::mutex> l (bindungMutex);
        …
        for (const auto& befehl : befehle)
            if (wendeSourcesCommandAnUnterBindung (befehl))
                ++geaendert;
    }
    for (std::size_t i = 0; i < geaendert; ++i)
        meldeSourcesMitgliederNachBefehl (generationBeimAbholen);
```

`eq-copilot/plugin/src/SourcesModel.cpp:360-372` (der einzige Riegel im Modell):

```
    std::lock_guard<std::mutex> l (mutex);
    // 🔑 NAK-246 Abschluss Nacharbeit 1 Fortsetzung (R-A1 Punkt 4', M-39): der
    // Generationsvergleich VOR jeder Aenderung und unter DEMSELBEN `mutex`, den
    // `projektReload` haelt. …
    if (generation != reloadGeneration)
        return false;
    if (neu == persistenteMitglieder)
        return true;    // Publikation hat stattgefunden, nur ohne Aenderung.
    persistenteMitglieder = std::move (neu);
```

**Befund der Lektüre:** Der Riegel im Modell vergleicht ausschließlich `reloadGeneration`.
Es existiert kein Ordnungsmerkmal für die *Mitgliederstände* innerhalb einer Generation.
Der `sourcesDrainMutex` klammert Abholen und Anwenden, endet aber vor der Schleife, die
publiziert (`:1533-1535`). Copy und Publikation liegen damit außerhalb jeder Klammer.

### Zusätzliche, vom Audit nicht genannte Publikationsstellen

`setzePersistenteMitglieder` hat am HEAD **fünf** Aufrufer, nicht zwei:
`Ipc.cpp:1233` (`benenneSourcesHauptziel`), `Ipc.cpp:1275` (lokaler Unbind ohne Broker),
`Ipc.cpp:1475` (Drain-Nachführung), `State.cpp:347` (`projektReload`), plus Testpfad.
Die beiden Message-Thread-Handgriffe ziehen ihre Kopie ebenfalls unter `bindungMutex` und
publizieren danach **ohne** Sperre; ihr Quellenkommentar deckt ausdrücklich nur den Reload ab:

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1229-1233`:

```
    // Wache, kein gemessener Fall: dieser Handgriff und `setStateInformation`
    // laufen heute beide auf dem Message-Thread, ein Reload kann zwischen Block
    // und Publikation nicht liegen. Kommt er doch, gilt dieselbe Regel wie im
    // Drain - Publikation unterblieben, kein Dirty, keine Revision.
    if (sourcesModel.setzePersistenteMitglieder (kopie, generation))
```

Der Worker-Drain ist in dieser Wache nicht bedacht. Der Befund ist damit **breiter** als vom
Audit beschrieben: auch `benenneSourcesHauptziel` und der lokale Unbind können von einer
älteren Worker-Kopie überholt werden.

### Anforderungsquelle

- **Keine Matrixzeile deckt den Fall.** `docs/beweise/NAK-246.md` M-12 (Zeile 614) sagt zu:
  „Genau einmal je Befehl, in ACK-Reihenfolge. … Join-dann-Unbind desselben Mitglieds endet
  **ohne** Mitglied." Gemessen wird dort aber nur der **Prozessorstate**:
  `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:1017-1020`
  („M-12: … endet OHNE Mitglied", Prüfgröße `mitglieder (*p)`), `:1021-1026` (Dirty, Revision).
  Das **Modell** kommt in M-12 nicht vor.
- M-38 / M-39 (`docs/beweise/NAK-246.md:5733`, `:5342`) decken ausschließlich den
  **Reload** gegen Drain bzw. gegen die gezogene Kopie — nicht zwei gültige Publikationen
  derselben Generation. Das Gegenbeispiel enthält keinen Reload.
- **CLAUDE.md-Invariante (User-Gesetz 24.08.2026):** „Keine toten UI-Elemente: jedes sichtbare
  Element führt einen Handgriff aus oder meldet ehrlich einen Zustand." Das Modell speist die
  Quellenliste; ein nach erfolgreichem Unbind weiterhin als `bestaetigt` gezeigtes Mitglied ist
  ein unehrlicher Zustand.
- **CLAUDE.md-Invariante:** „Engine kennt keine Optik. Sie liefert kohärente Mess-Snapshots;
  der Editor hält Anzeigezustand" — Modell und State dürfen nach Ruhe nicht auseinanderfallen.

### Auslösekette und Erreichbarkeit im Produkt

1. Audio-/Analyseworker (`eq-copilot/plugin/src/prozessor/Analyse.cpp:347`, Zug spätestens alle
   50 ms, ab Konstruktor bis Destruktor) drain’t `confirm_join (A)`, wendet unter `bindungMutex`
   an → `zustand = [B, A]`, verlässt `sourcesDrainMutex`.
2. Worker ruft `meldeSourcesMitgliederNachBefehl`, zieht die Kopie `[B, A]` unter `bindungMutex`,
   **gibt die Sperre frei**.
3. Message-Thread (Editor-Timer `PluginEditor.cpp:247` → `sourcesTick` → `Ipc.cpp:1113`)
   drain’t `unbind_probe (B)` → `zustand = [A]`, publiziert `[A]` ins Modell.
4. Worker publiziert danach seine ältere Kopie `[B, A]`. `setzePersistenteMitglieder` findet
   dieselbe `reloadGeneration` und übernimmt.

Beide Threads existieren im Produkt (der Workerzug ist seit NAK-246 D3 der prozessoreigene Takt,
Testhaken sind im Produkt leer und weiten das Fenster nur auf). Der Testhaken ist **nicht**
Voraussetzung des Rennens, nur seiner Determinierung.

**Keine Selbstheilung:** `wendeBestaetigteSourcesCommandsAn` kehrt bei leerem Batch vor jeder
Nachführung zurück (`Ipc.cpp:1503-1504`), und `meldeSourcesMitgliederNachBefehl` läuft nur
`geaendert`-mal. Ein leerer Folgedrain korrigiert das Modell also nicht; der falsche Stand bleibt
bis zum nächsten state-ändernden Befehl. Ein eingehender `session_snapshot` schreibt `eintraege`,
nicht `persistenteMitglieder`.

### Status

**BESTÄTIGT** — allein aus der Quelle ableitbar: Kopie unter Sperre, Publikation ohne Sperre,
und der einzige Vergleich im Modell ist die Reload-Generation. Der Quellenkommentar `:1444-1447`
beschreibt ausdrücklich nur den seriellen Fall („nacheinander fahren"); genau der ist im Audit
die grüne Kontrolle.

### Vorschlag Einordnung

**DEFEKT.** Grund: verletzt die CLAUDE.md-Invariante „jedes sichtbare Element meldet ehrlich
einen Zustand" (24.08.2026) und die Kohärenz von State und Anzeigemodell; nicht selbstheilend.
Keine Dublette zu NAK-278/NAK-279 (dort geht es um Reload-Generation; hier liegt kein Reload vor).
Korrekturziel sollte alle **fünf** Publikationsstellen umfassen, nicht nur den Drain.

### Test, der die Zusage tragen müsste

`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:930-1039`
(`dirty_und_revision_genau_einmal_je_bestaetigtem_befehl`, M-12) — misst State, Dirty, Revision
und den Riegel, **nicht** `sourcesModel`. **Würde heute nicht rot.**
`:1437-1530` (M-39) hält den Publikationshaken, fährt aber einen Reload und prüft gegen den
geladenen State — deckt den Gleich-Generation-Fall nicht ab. **Würde heute nicht rot.**

---

## F02 — „NAK-253 ist vom Verdacht zum beobachteten Ergebnisrücksprung geworden"

*§2-Titel: „Ältere Hypothesenrechnung überschreibt bereits veröffentlichtes neueres Ergebnis" ·
Priorität laut Audit: **P2***

### Quellzitat an HEAD

`broker/src/coordinator/hypothese_verdrahtung.rs:66-93`:

```
    /// Der Weg ist dreiteilig und in dieser Reihenfolge zwingend:
    /// **einsammeln unter dem Lock → rechnen ohne Lock → eintragen,
    /// persistieren, zustellen**. …
    pub(super) fn hypothesen_bilden(&self) {
        let aufnahmen = self.aufnahmen_sammeln();
        if aufnahmen.is_empty() {
            return;
        }
        let mut ergebnisse: Vec<(SessionKey, Vec<CauseHypothesis>)> = Vec::new();
        for (session, aufnahme) in aufnahmen {
            let ergebnis = hypothesen(&aufnahme);
            ergebnisse.push((session, ergebnis.befunde));
        }
        …
        self.rechen_test_haken_ausloesen();
        for (session, befunde) in ergebnisse {
            let geaendert = self.befunde_eintragen(&session, befunde);
```

`broker/src/coordinator/hypothese_verdrahtung.rs:961-1000` (der ganze Riegel):

```
    fn ergebnis_ist_noch_gueltig(
        stand: &Stand,
        session: &SessionKey,
        befunde: &[CauseHypothesis],
    ) -> bool {
        if befunde.is_empty() {
            return true;
        }
        let aktuelle_generation = stand
            .intent
            .get(session)
            .map(|b| b.generation)
            .unwrap_or_default();
        if befunde
            .iter()
            .any(|b| b.intent_generation != aktuelle_generation)
        {
            return false;
        }
        …
        befunde
            .iter()
            .flat_map(|b| b.evidence_ids.iter())
            .all(|id| gueltig.contains(id.as_str()))
    }
```

`broker/src/coordinator/hypothese_verdrahtung.rs:465-477` (bedingungsloses Ersetzen nach dem Riegel):

```
            let alt = stand.befunde.get(session);
            if alt.map(Vec::as_slice) == Some(befunde.as_slice()) {
                return false;
            }
            …
                stand.befunde.insert(session.clone(), befunde.clone());
```

**Befund der Lektüre:** Der Riegel prüft (a) Intentgeneration und (b) Fortbestand der
*verwendeten* Evidenz-IDs. Er kennt **weder** die Menge der zum Sammelzeitpunkt vorhandenen
Evidenz **noch** ein Merkmal des bereits eingetragenen Bestands. Eine Rechnung A mit 12 weiterhin
gültigen IDs unter derselben Intentgeneration passiert ihn und ersetzt den Bestand aus Rechnung B
mit 13 IDs. `stand.befunde.insert` überschreibt ohne Altersvergleich.

### Anforderungsquelle

- **Registerzeile `docs/offene-punkte.md:265` (NAK-253):** „**[Verdacht · Broker ·
  Hypothesen-Ordering]** Eine ältere Hypothesenrechnung kann ein neueres Ergebnis überschreiben:
  der Riegel prüft Intentgeneration und Gültigkeit der verwendeten Evidenz-IDs, nicht neu
  hinzugekommene Evidenz; A (alte, weiter gültige Evidenz) setzt nach B ein. Mildernd:
  `befunde_neu_bilden` heilt am nächsten Takt, sofern das Flag nicht schon verbraucht war.
  **Ohne Laufzeit unentscheidbar**; … Wer es angeht: Konkurrenztest mit weiterhin gültigen alten
  Evidenz-IDs."
- **Matrix schweigt.** `docs/beweise/SONDE-014.md` M-10 (`:724`) sagt nur zu, dass abhängige
  Befunde bei steigender Intent-Revision `STALE` werden; M-24 (`:743`) nur, dass eine Rücknahme
  abhängige Hypothesen deterministisch invalidiert. Kein Satz verlangt, dass eine Rechnung mit
  *älterem Eingangsstand* eine neuere nicht verdrängt.

### Auslösekette und Erreichbarkeit im Produkt

Auslöser ist `broker/src/coordinator/evidenz.rs:232` (`self.hypothesen_bilden();` direkt im
Annahmeweg jedes `evidence_snapshot`) und `invalidierung_verdrahtung.rs:118`.
Der Broker fährt **einen Thread je Verbindung**
(`broker/src/transport/server_v3/listener.rs:612-614`, Threadname `eqcop-v3-conn`), und Nakama
betreibt bestimmungsgemäß mehrere Probeeq-Instanzen an einem Gen. Zwei Sonden, die dicht
hintereinander Evidenz liefern, betreten `hypothesen_bilden` gleichzeitig; der Standlock ist
zwischen `aufnahmen_sammeln` und `befunde_eintragen` ausdrücklich frei (Doku-Kommentar `:66-69`).
**Der Pfad ist im heutigen Produkt erreichbar**, ohne Testhaken; der Haken (`:91`) determiniert
nur die Verschachtelung.

Die im Register genannte Milderung ist quellseitig bestätigt, aber unvollständig:
`befunde_neu_bilden` wird nur im **Verwerffall** (`:464`) und beim Invalidierungs-Rollback
(`invalidierung_verdrahtung.rs:258`) gesetzt — im hier beschriebenen Fall wird nichts verworfen,
also kein Heilungstakt angestoßen. Der falsche Stand bleibt bis zur nächsten Evidenz.

### Status

**BESTÄTIGT** — der fehlende Vergleich ist direkt im Riegel lesbar, und die Nebenläufigkeit der
Aufrufer ist quellseitig belegt. Der Auditlauf ist historischer Zusatz, nicht Grundlage.

### Vorschlag Einordnung

**LÜCKE**, bereits registriert als **NAK-253**. Grund: weder Matrix noch Entwurf verlangen eine
Eingangsstands-Ordnung der Hypothesenrechnung; der Riegel ist bewusst auf Intentgeneration und
Evidenzgültigkeit beschnitten (NR-03, 07.09.2026).
**Änderung gegenüber der Registerzeile:** (a) „Ohne Laufzeit unentscheidbar" ist überholt — die
fehlende Prüfung ist ohne Laufzeit direkt aus `ergebnis_ist_noch_gueltig` ablesbar; (b) die
Milderung „`befunde_neu_bilden` heilt am nächsten Takt" trägt in genau diesem Fall **nicht**,
weil nichts verworfen wird und das Flag deshalb nie gesetzt wird; (c) die Klasse sollte von
`[Verdacht]` auf eine zu entscheidende Regel (Lücke) gehoben werden.

### Test, der die Zusage tragen müsste

`broker/tests/sonde014_verdrahtung.rs:533-620`
(`veraltetes_rechenergebnis_wird_nicht_veroeffentlicht`, NR-03) — prüft am selben Rechenhaken
genau zwei Interleavings: steigende Intent-Revision und während der Rechnung zurückgenommene
Evidenz. Der Fall „während der Rechnung kommt **neue** gültige Evidenz dazu" fehlt.
**Würde heute nicht rot.**

---

## F03 — „Koaleszierung erhält nicht die Reihenfolge zwischen Snapshot und Invalidierung"

*§2-Titel: „Koaleszierung vertauscht die Reihenfolge von Vollsnapshot und Rücknahme" ·
Priorität laut Audit: **P2***

### Quellzitat an HEAD

`broker/src/transport/server_v3/queues.rs:202-238`:

```
    pub(super) fn einreihen_eintrag(&self, mut eintrag: AusgangEintrag) -> bool {
        let mut ersetzt = None;
        let ok = {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            if g.1 {
                false
            } else if let Ausgangsart::Snapshot(objekt_schluessel) = &eintrag.art {
                // 🔑 NAK-246 D5 (R-D5, §5.5 Feinheit 2, R-M2-1): die Annahmeregel.
                // Verworfen wird nur, was KLEINER ist als das Hochwasser seines
                // Schluessels …
                let schluessel = objekt_schluessel.clone();
                let hochwasser = g.2.get(&schluessel).copied();
                …
                if hochwasser.is_some_and(|hoch| eintrag.marke < hoch) {
                    self.nachzuegler_verworfen.fetch_add(1, Ordering::SeqCst);
                    false
                } else if let Some(position) = g.0.iter().position(|alt| {
                    matches!(&alt.art, Ausgangsart::Snapshot(alt_schluessel)
                        if *alt_schluessel == schluessel)
                }) {
                    g.2.insert(schluessel, eintrag.marke);
                    ersetzt = Some(std::mem::replace(&mut g.0[position], eintrag));
                    true
```

**Befund der Lektüre:** `std::mem::replace(&mut g.0[position], eintrag)` setzt den neuen Eintrag
an die **alte** Position der Deque. Ein später eingereihter `session_snapshot` erbt damit die
Position eines früheren und überholt jeden dazwischen eingereihten Eintrag eines **anderen**
Schlüssels. Das Hochwasser (`g.2`) ist je Schlüssel geführt und ordnet nicht über Schlüssel hinweg.

Verbraucherseite, `eq-copilot/plugin/src/SourcesModel.cpp:1447-1467`:

```
    // Die WIRKUNG. Gen fuehrt keine Evidenz-IDs - es fuehrt Zeilen mit einem
    // Messzustand. … Fail-closed heisst hier
    // invalidieren: jede Zeile, die eine Messaussage traegt, verliert sie.
    std::lock_guard<std::mutex> l (mutex);
    for (auto& [_, e] : eintraege)
        if (e.zeile.messung != Messung::missing)
            e.zeile.messung = Messung::invalid;
    …
    setzeAlleBefundeStale();
    evidenzRuecknahmen = evidenzRuecknahmen + 1;
```

Kein Altersvergleich; die Rücknahme wirkt unbedingt auf den zuletzt übernommenen Stand.

### Anforderungsquelle

- **Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md:4122`:** „P1 Zustand/Evidenz | 128 |
  Snapshots nach Objektschlüssel koaleszieren; nicht koaleszierbare Events bei Überlauf über
  Reconnect/Outbox wiederholen." — sagt nichts über die Reihenfolge **zwischen** Objektschlüsseln.
- **Matrixzeile `docs/beweise/NAK-246.md:643` (M-26):** „Der Queue-Schlüssel ist der
  Objektschlüssel des Ziels. … drei Einträge liegen gleichzeitig in der Queue, keiner ersetzt
  einen anderen; **die Writerreihenfolge ist die Einreihreihenfolge**." Die Zusage gilt für das
  Szenario **ohne** Koaleszierung; der Satz selbst ist im F03-Fall verletzt, sein Szenario
  aber nicht getroffen.
- **Matrixzeile `docs/beweise/NAK-246.md:645` (M-28):** „Innerhalb eines Schlüssels bleibt die
  Koaleszierungsregel (jüngere Marke ersetzt, M-21)" — ausdrücklich nur innerhalb eines Schlüssels.
- **Matrixzeile `docs/beweise/NAK-246.md:637` (M-25):** „Kein Schemawechsel. … der Consumer
  (`SourcesModel.cpp:1152-1159`) prüft weiterhin kein Alter — dieser Riegel ist Härtung."
- **Registerzeile `docs/offene-punkte.md:281` (NAK-269):** „**[Härtung · Vertrag]** Der C++-Consumer
  prüft bei `session_snapshot` weiterhin kein Alter. … **Wirkung heute keine — die Ordnung entsteht
  vollständig im Broker**; ein zweiter Sender, ein anderer Transportweg oder ein Consumer ohne
  diese Queue hätte den Schutz nicht."

### Auslösekette und Erreichbarkeit im Produkt

Alle drei Einreihungen laufen über `V3Sender::snapshot_schreiben` → `griff.rs` →
`snapshot_einreihen_mit_antwort` in denselben `Ausgang` eines Links. Ein Verbraucher, der kurz
nicht liest (langsamer Leser, blockierter Pipe-Write), hält zwei Einträge in der Queue; ein
dazwischen eingereihter neuer Snapshot desselben Links koalesziert an die vordere Position.
Auslöser sind zwei gewöhnliche Coordinator-Ereignisse: ein Flush/Commit (`session_snapshot`) und
eine Evidenzrücknahme (`evidence_invalidate`, z. B. aus `invalidierung_aus_transportbruch`).
**Erreichbar im Produkt**, ohne zweiten Sender und ohne anderen Transport — genau das, was
NAK-269 als „Wirkung heute keine" ausschließt.

**Wirkungsgrenze, ehrlich:** Die Verbraucherwirkung ist fail-closed (Messachsen `invalid`,
Befunde `stale`) und wird vom nächsten Vollsnapshot wieder überschrieben. Es geht kein
persistenter Stand verloren; die Anzeige zeigt für ein Fenster weniger, als gedeckt wäre.
Ein durchgängiger Broker-zu-Plugin-Beleg fehlt (Auditgrenze; hier nicht nachgeholt).

### Status

**BESTÄTIGT** für die Queue-Reihenfolge (direkt aus `einreihen_eintrag` ablesbar) und für die
fehlende Altersprüfung im Consumer. **BESTÄTIGT** außerdem, dass die Annahme in NAK-269
(„die Ordnung entsteht vollständig im Broker") quellseitig falsch ist.

### Vorschlag Einordnung

**LÜCKE.** Grund: Matrix (M-21/M-22/M-26/M-27/M-28) und Entwurf (`:4122`) regeln die Ordnung
ausdrücklich **je Objektschlüssel** und schweigen zur Ordnung über Schlüssel hinweg; R-D5 hat
den Consumer-Altersriegel bewusst nicht gebaut (M-25). Der Dirigent muss die Regel entscheiden
(Ordnung über abhängige Schlüssel im Broker **oder** Altersriegel im Consumer), bevor eine
Nacharbeit ausgelöst wird.
**Änderung gegenüber NAK-269:** der dort tragende Satz „Wirkung heute keine — die Ordnung entsteht
vollständig im Broker" fällt. NAK-269 ist damit nicht mehr reine Härtung; die Zeile ist an der
neuen Ursache (Koaleszierung, nicht nur fehlender Consumer-Riegel) neu zu fassen.
*Alternative Lesart:* Wer den Zusagesatz aus M-26 („die Writerreihenfolge ist die
Einreihreihenfolge") als allgemein liest, kommt zu **DEFEKT**. Die Entscheidung gehört dem
Dirigenten.

### Test, der die Zusage tragen müsste

`broker/src/transport/server_v3/queues.rs:366-381`
(`writerqueue_snapshot_koalesziert_nach_objektschluessel`, M-28) und `:388-455`
(`aelterer_nachzuegler_ersetzt_keinen_neueren_snapshot`, M-21) — beide messen nur **innerhalb**
eines Schlüssels bzw. die Annahme/Verwerfung, nie die Ausgabereihenfolge über zwei Schlüssel bei
Koaleszierung. **Würden heute nicht rot.** Auch M-27
(`ruecknahme_und_vollsnapshot_liegen_zugleich_in_der_angehaltenen_queue`) fährt nur zwei
Einreihungen ohne Ersetzung. **Würde heute nicht rot.**

---

## F11 — „Akzeptierte maximale Revision verletzt Mutation und Roundtrip"

*§2-Titel: „Revisionsgrenze verletzt Ablehnungsatomarität und eigenen State-Roundtrip" ·
Priorität laut Audit: **P3***

### Quellzitat an HEAD

`eq-copilot/plugin/state/NakamaState.cpp:2207-2216`:

```
bool bestandsrevisionHeben (Zustand& z, juce::String& grund)
{
    if (z.intentBestandRevision >= std::numeric_limits<juce::int64>::max())
    {
        grund = "intent revision would overflow";
        return false;
    }
    ++z.intentBestandRevision;
    return true;
}
```

`eq-copilot/plugin/state/NakamaState.cpp:2275-2291` (stellvertretend für alle drei Handgriffe;
`entferneSchutzangabe :2334-2348` und `entferneBeziehung :2447-2463` sind formgleich):

```
bool entferneIntent (Zustand& z, const juce::String& quelleId, const juce::String& passageId,
                     bool& veraendert, juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (quelleId))             { grund = "source id is not hex32"; return false; }
    if (! passageScopeGueltig (passageId)) { grund = "passage scope is neither empty nor hex32"; return false; }

    const auto vorher = z.sourceIntents.size();
    z.sourceIntents.erase (std::remove_if (z.sourceIntents.begin(), z.sourceIntents.end(),
        [&] (const SourceIntent& s) { return s.quelleId == quelleId && s.passageId == passageId; }),
        z.sourceIntents.end());
    if (z.sourceIntents.size() == vorher)
        return true;
    if (! bestandsrevisionHeben (z, grund)) return false;
    veraendert = true;
    return true;
}
```

→ Die Mutation (`erase`) steht **vor** dem Riegel. Bei `intentBestandRevision == int64max`
liefert der Handgriff `false`, `veraendert` bleibt `false` (also **kein** Host-Dirty), der
Eintrag ist aber entfernt.

`eq-copilot/plugin/state/NakamaState.cpp:2656-2665` (Assistentenpfad, ohne obere Schranke;
`:2707` und `:2810` sind formgleich):

```
void schrittAendern (Assistentenzustand& a, Assistentenschritt neu, bool offen,
                     bool& veraendert)
{
    if (a.gesetzt && a.schritt == neu && a.offen == offen)
    {
        veraendert = false;
        return;
    }
    a.schritt = neu;
    a.offen = offen;
    a.revision += 1;
    veraendert = true;
}
```

Reader-Gegenstück `eq-copilot/plugin/state/NakamaState.cpp:1905-1909`:

```
            if (! (revision.isInt() || revision.isInt64())
                || static_cast<juce::int64> (revision) < 1)
            {
                grund = "MainProject.assistant_step_v1 revision must be at least 1";
                return false;
            }
```

Dass `int64max` als Bestand **ausdrücklich schreibbar** gilt, steht im Produktcode selbst
(`NakamaState.cpp:1189` und `:1196`, Headroomkandidat):
`kandidat.intentBestandRevision = std::numeric_limits<juce::int64>::max();` und
`… Assistentenschritt::verdict, std::numeric_limits<juce::int64>::max(), …`.

### Anforderungsquelle

- **Vertragssatz `eq-copilot/schemas/state/nakama-state-v2.md:139`:** „`intent_revision_v1` |
  `int64` ≥ 1 | Revision des **ganzen** Bestands, **steigt bei jeder persistenten Änderung genau
  einmal**. Fehlt sie, ist der Bestand nie beschrieben worden; **Inhalt ohne Revision** ist ein
  ungültiger Stand …"
- **Vertragssatz `:136`:** `source_intents_v1` … „`revision` `int64` ≥ 1".
- **CLAUDE.md-Invariante:** „**State bleibt verlustfrei.** … jede persistente Änderung meldet dem
  Host Dirty-State." — Im MAX-Fall gibt es eine persistente Änderung **ohne** Dirty und **ohne**
  Revisionserhöhung.
- **CLAUDE.md-Invariante:** „Schemas sind Verträge. … Altstände laden; … Save und Load gemeinsam
  testen." — Ein selbst erzeugter Stand mit negativer Assistentenrevision wird vom eigenen Reader
  abgewiesen.

### Auslösekette und Erreichbarkeit im Produkt

Erreichbar nur nach ~9,2 × 10^18 Bestandsänderungen bzw. Assistentenschritten. Durch normale
Bedienung nicht erreichbar; die Audit-Einordnung P3 ist zutreffend. **Erreichbar ist der Zustand
aber über den Ladeweg**: der Reader akzeptiert `int64max` (kein oberer Riegel im Leser), und der
eigene Headroomkandidat (`:1189`) erzeugt ihn ausdrücklich. Ein geladenes Projekt mit
`intent_revision_v1 = int64max` bringt die Instanz sofort in den Zustand, in dem der nächste
Entfern-Handgriff still mutiert. Der Überlauf auf dem Assistentenpfad ist zusätzlich
**signed-integer-UB** (`a.revision += 1` ohne Schranke), also nicht bloß ein falscher Wert.

### Status

**BESTÄTIGT** für beide Hälften: Mutation-vor-Riegel ist an drei Stellen wörtlich lesbar, die
fehlende obere Schranke der Assistentenrevision an drei weiteren, und der Reader-Riegel `< 1`
steht an `:1907`.

### Vorschlag Einordnung

**DEFEKT.** Grund: verletzt den Vertragssatz „steigt bei jeder persistenten Änderung genau
einmal" (`nakama-state-v2.md:139`) und die CLAUDE.md-Invariante „jede persistente Änderung meldet
dem Host Dirty-State"; der Assistentenpfad erzeugt zusätzlich UB und einen vom eigenen Reader
abgewiesenen Stand („State bleibt verlustfrei"). Die Seltenheit ist ein Prioritäts-, kein
Klassenargument — P3 bleibt richtig.

### Test, der die Zusage tragen müsste

`eq-copilot/plugin/tests/Sonde014IntentTest.cpp` — führt `intentBestandRevision` an `:565-570`,
`:623-632`, `:747-759`, `:814`, `:1020`, `:1058`, prüft aber keinen Grenzwert und keinen
Entfern-Handgriff an der Obergrenze. Kein Testkörper im Repo enthält
`std::numeric_limits<juce::int64>::max()` für Bestands- oder Assistentenrevision
(geprüft über `eq-copilot/plugin/tests/*.cpp`). **Kein bestehender Test würde rot.**

---

## F13 — „Der Speichernachweis unterscheidet Messfehler nicht von null Bytes"

*§2-Titel: „Fehlende RSS-Messwerte können den Speichertest grün bestehen lassen" ·
Priorität laut Audit: **P2***

### Quellzitat an HEAD

`tools/eq-copilot/pruefe_session_soak.py:284-299`:

```
def rss_bytes(pid: int) -> int:
    """Working Set eines Prozesses. 0, wenn er nicht (mehr) greifbar ist."""
    k32, psapi = ctypes.windll.kernel32, ctypes.windll.psapi
    h = k32.OpenProcess(_PROCESS_QUERY_LIMITED_INFORMATION | _PROCESS_VM_READ,
                        False, pid)
    if not h:
        return 0
    try:
        pmc = _PMC()
        pmc.cb = ctypes.sizeof(_PMC)
        if not psapi.GetProcessMemoryInfo(h, ctypes.byref(pmc), pmc.cb):
            return 0
        return int(pmc.WorkingSetSize)
    finally:
        k32.CloseHandle(h)
```

`tools/eq-copilot/pruefe_session_soak.py:421-431` (Fehlerwert wandert unverändert in den Bericht):

```
    def probe(minute: int, sekunden: float, im_fenster: bool) -> None:
        """Eine Speicherprobe beider Prozesse. …"""
        eintrag = {"minute": minute, "sekunden": round(sekunden, 1),
                   "im_neustartfenster": im_fenster}
        speicher["client"].append({**eintrag, "rss_bytes": rss_bytes(klient.pid)})
        speicher["broker"].append({**eintrag, "generation": generation,
                                   "rss_bytes": rss_bytes(broker.pid)})
```

`tools/eq-copilot/pruefe_session_soak.py:992-1021` (Bewertung ohne Erfolgsbedingung):

```
    def wachstum(punkte, budget_p, budget_b):
        gueltig = [p for p in punkte if not p.get("im_neustartfenster")]
        if len(gueltig) < 2:
            return None
        basis, ende = gueltig[0]["rss_bytes"], gueltig[-1]["rss_bytes"]
        grenze = max(basis * budget_p / 100.0, budget_b)
        spanne = gueltig[-1].get("sekunden", 0) - gueltig[0].get("sekunden", 0)
        return basis, ende, ende - basis, grenze, spanne
    …
        pruefe(delta <= grenze, "S07",
               f"{name}-Working-Set bleibt im Budget",
               f"{basis/1e6:.1f} -> {ende/1e6:.1f} MB, +{delta/1e6:.1f} MB, "
               f"Grenze {grenze/1e6:.1f} MB ueber {spanne:.0f} s")
```

**Befund der Lektüre:** `basis = ende = 0` ⇒ `delta = 0`, `grenze = max(0, BUDGET_BYTES) ≥ 0`
⇒ `delta <= grenze` ist wahr. Geprüft werden Punktanzahl, Zeitspanne, Generationenzahl und
Wachstum; **keine** Prüfung verlangt einen erfolgreichen Speicherabruf. Der Fehlerwert der
WinAPI-Abfrage ist vom Messwert 0 nicht unterscheidbar.

### Anforderungsquelle

- **Kanonische Zusage `tools/beweise.ps1:614` (Bein A24), wörtlich:** „… **Working Set beider
  Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB)** …" — die Zusage wird als
  erfüllt gemeldet, obwohl kein Working Set gemessen wurde.
- **Bewertungsschlüssel `tools/eq-copilot/pruefe_session_soak.py:141`:**
  `"S07": "A24:speicherkurve_im_budget"`.
- **CLAUDE.md-Arbeitsregel:** „Fortschritt erst nach Beleg aus der laufenden Session behaupten"
  und „Der Runner … verweigert die Beglaubigung, wenn Prüfbinaries älter sind als ihre Quellen" —
  eine Beglaubigung ohne Messung widerspricht demselben Prinzip.
- **Kein Sabotagefall:** Der Dirigenten-Skill (§3.4) stuft nur solche Runner-Befunde als Härtung
  ein, die „Sabotage in repo-eigenen Quellen voraussetzt". `OpenProcess`-Verweigerung, beendeter
  Prozess oder fehlgeschlagenes `GetProcessMemoryInfo` sind gewöhnliche Laufzeitfälle.

### Auslösekette und Erreichbarkeit

`probe()` läuft während des Laufs periodisch; `rss_bytes` trifft die Null-Rückgabe, wenn der
Broker im Neustartfenster gerade beendet ist, wenn der PID recycelt/nicht mehr greifbar ist oder
wenn die Rechteprüfung fehlschlägt. Der Bericht schreibt den Wert unverändert; die Bewertung
akzeptiert ihn. **Erreichbar ohne Manipulation** — das Neustartfenster des Brokers ist im Ablauf
des Beins selbst vorgesehen (`--neustarts 1`), und Punkte im Fenster werden zwar aus der Kurve
gefiltert, Punkte **um** das Fenster herum nicht zwingend.

### Status

**BESTÄTIGT** als Orakelbefund. **NICHT** bestätigt (und vom Audit auch nicht behauptet): dass im
historischen A24-Lauf tatsächlich Messungen fehlgeschlagen sind, oder dass ein Speicherleck
existiert. Es bleibt ein Nachweisloch, kein Produktbefund.

### Vorschlag Einordnung

**DEFEKT** (Nachweis/Werkzeug). Grund: die wörtliche Gate-nahe Zusage des kanonischen Runners
(`tools/beweise.ps1:614`) kann grün gemeldet werden, ohne dass die zugesagte Größe je gemessen
wurde; der Ausfallpfad ist gewöhnlich und nicht sabotagebedingt.
*Hinweis für die Einordnung:* Wenn der Dirigent Werkzeugbefunde konsequent als Registerklasse
`[Werkzeug]` führt (Muster NAK-249, `docs/offene-punkte.md:261`), gehört F13 dorthin statt in die
Nacharbeit — die Wirkung (ein Bein beglaubigt ohne Messung) bleibt dieselbe.

### Test, der die Zusage tragen müsste

Es existiert **kein** Selbsttest des Orakels: `pruefe_session_soak.py` kennt keine
`--selbsttest`-Betriebsart (geprüft; nur Mutationsschalter wie `"s07"` an `:185`, der das Budget
auf 0 setzt). Das Mutationsmittel `:185/:630-631` mutiert das **Budget**, nicht die Messung —
ein Lauf ohne RSS-Messwerte bleibt dort ebenfalls grün. **Kein bestehender Prüfkörper würde rot.**

---

## F14 — „NAK-163: Fehlgeschlagene Invalidierung nimmt den falschen Ausschluss zurück"

*§2-Titel: „Rollback einer Invalidierung entfernt den Ausschluss eines anderen Belegs" ·
Priorität laut Audit: **P2***

### Quellzitat an HEAD

`broker/src/coordinator/invalidierung_verdrahtung.rs:139` und `:155-176` (Index wird gespeichert):

```
        let mut zurueck: Vec<(ClientKey, usize)> = Vec::new();
        …
        for key in keys {
            let Some(historie) = stand.evidenz.get_mut(&key) else {
                continue;
            };
            for (index, eintrag) in historie.iter_mut().enumerate() {
                if eintrag.ausschlussgrund.is_some() {
                    // Schon ausgeschlossen: der ERSTE Grund bleibt stehen.
                    continue;
                }
                …
                if invalidierung
                    .umfang
                    .erfasst(&eintrag.evidence_id, von, bis)
                {
                    eintrag.ausschlussgrund = Some(invalidierung.grund.wort().to_string());
                    genommene_ids.insert(eintrag.evidence_id.clone());
                    zurueck.push((key.clone(), index));
                }
            }
        }
```

`broker/src/coordinator/invalidierung_verdrahtung.rs:230-259` (Rollback über denselben Index):

```
    /// Zurueckgesetzt wird GENAU, was diese Invalidierung markiert hat — nicht
    /// jeder Ausschluss der Sitzung. Ein aelterer Grund gehoert einer anderen
    /// Ruecknahme und bleibt stehen.
    pub(super) fn invalidierung_ruecknehmen(&self, wirkung: Invalidierungswirkung) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        for (key, index) in &wirkung.zurueck {
            if let Some(historie) = stand.evidenz.get_mut(key) {
                if let Some(eintrag) = historie.get_mut(*index) {
                    eintrag.ausschlussgrund = None;
                }
            }
        }
        …
        stand.befunde_neu_bilden = true;
```

`broker/src/coordinator/evidenz.rs:179-189` (die Verschiebung):

```
            let historie = stand.evidenz.entry(key.clone()).or_default();
            let mut eintrag = stand_neu;
            eintrag.empfangsfolge = self.evidenz_folge.fetch_add(1, Ordering::SeqCst);
            let folge = eintrag.empfangsfolge;
            historie.push_back(eintrag);
            while historie.len() > EVIDENZ_RETENTION {
                historie.pop_front();
            }
```

`broker/src/coordinator/invalidierung_verdrahtung.rs:88-92` (Persistenz **ohne** Standlock):

```
        let Some(wirkung) = self.invalidierung_vorbereiten(session, invalidierung) else {
            return Ok(0);
        };
        let betroffen = wirkung.betroffen;
        match self.invalidierung_alleine_persistieren(session, &wirkung) {
```

**Befund der Lektüre:** `invalidierung_vorbereiten` gibt den Standlock frei, bevor persistiert
wird; das Rollback nimmt ihn neu. Zwischen beiden Sperrabschnitten kann ein anderer
Verbindungsthread Evidenz aufnehmen und `pop_front()` auslösen. Die gespeicherten Deque-Indizes
zeigen danach auf **andere** Einträge. Der Kommentar `:232-234` beschreibt exakt die Zusage,
die der Code nicht hält.

### Anforderungsquelle

- **Datierter Konvergenzentscheid `docs/beweise/SONDE-013.md:4342`, wörtlich:**
  „| B16-Index **„Rolle Invalidierungen über stabile Evidence-IDs zurück"** | Rollback über
  Indizes; nur wenn die Historie am Retention-Deckel steht UND während eines **fehlgeschlagenen**
  Appends ein Snapshot eintrifft, verschieben sich die Indizes. | **HÄRTUNG** — Fehlerpfad ×
  Randlage. | NAK-163 |"
- **Registerzeile `docs/offene-punkte.md:191` (NAK-163):** „**[Härtung/Struktur · S20–22]**
  **Der Fehler-Rollback der Invalidierung arbeitet über Indizes.** … verschiebt `pop_front()` die
  Indizes, und der Rollback löscht den Ausschluss eines anderen Snapshots oder lässt den eigenen
  stehen. Fehlerpfad × Randlage. Wer es angeht: wie beim Evidence-Append über `evidence_id`
  zurücknehmen."
- **Matrixzeile `docs/beweise/SONDE-014.md:743` (M-24):** „Entfernen eines Evidenzobjekts
  invalidiert abhängige Hypothesen **deterministisch**. … „Deterministisch" heißt: dieselbe
  Rücknahme in anderer Reihenfolge trifft dieselbe Menge." Der Produktkommentar zieht daraus
  wörtlich (`hypothese_verdrahtung.rs:956-957`): „Eine waehrend der Rechnung zurueckgenommene ID
  darf **nie wieder sichtbar werden** (M-24, M-28)."
- **Zusage im bestehenden Test** `broker/tests/sonde013_verdrahtung.rs:3237-3243`: „B16: und AUCH
  KEIN lokaler Ausschluss - **der fluechtige Stand bleibt gleich dem persistierten**."

### Auslösekette und Erreichbarkeit im Produkt

1. E1 ist durch eine frühere **erfolgreiche** Intervention ausgeschlossen (`ausschlussgrund =
   "intervention"`, auch in der SQLite-Projektion).
2. Eine neue Invalidierung markiert E0 unter dem Standlock, merkt sich `(ClientKey, index)`,
   gibt den Lock frei.
3. Ein anderer `eqcop-v3-conn`-Thread nimmt Evidenz auf; die Historie steht am
   `EVIDENZ_RETENTION`-Deckel, `pop_front()` verschiebt alle Indizes um eins.
4. Der Store-Append scheitert (SQLite BUSY, degradierter Store, volle Platte) → `Err(())` →
   `invalidierung_ruecknehmen` löscht `ausschlussgrund` am **verschobenen** Index, also bei E1.
5. `stand.befunde_neu_bilden = true` (`:258`) stößt `hypothesen_bei_bedarf_bilden()` an
   (`:118`); die Neurechnung nimmt E1 wieder als gültigen Beleg auf. Cache und SQLite-Projektion
   sind ab hier uneins.

Alle vier Voraussetzungen sind Produktionspfade; keine davon braucht Testhaken. Der Storefehler
ist der einzige seltene Teil — der Konvergenzentscheid nennt ihn zutreffend „Fehlerpfad ×
Randlage".

### Status

**BESTÄTIGT** — Indexspeicherung, Rollback über Index, Retention-`pop_front` und die Lock-Lücke
sind wörtlich lesbar. Neu gegenüber der Registerzeile ist die **belegte Wirkung**: ein
*fremder*, berechtigt ausgeschlossener Beleg wird im Cache wieder gültig und geht in die
Neurechnung ein, während die Projektion ihn weiter als ausgeschlossen führt.

### Vorschlag Einordnung

**DEFEKT.** Grund: ein durch eine frühere erfolgreiche Rücknahme ausgeschlossener Beleg wird
wieder sichtbar und fließt in die Befundrechnung ein — gegen den Produktkommentar zu M-24
(„darf nie wieder sichtbar werden") und gegen die Testzusage „der flüchtige Stand bleibt gleich
dem persistierten" (`sonde013_verdrahtung.rs:3241-3242`).
**Änderung gegenüber NAK-163:** die Zeile führt den Punkt als `[Härtung/Struktur]` nach dem
dated Entscheid vom 05.09.2026, der ihn als „Fehlerpfad × Randlage" einstufte. Diese Einstufung
begründete sich auf Wahrscheinlichkeit, nicht auf fehlende Zusage. Mit dem jetzt gezeigten
Effekt (Cache ≠ Projektion, wiederauferstandener Ausschluss, verfälschte Neurechnung) ist die
Wirkung benannt; die Umstufung von Härtung auf Defekt ist Dirigentensache, aber sachlich
gestützt. **Keine neue Register-ID** — NAK-163 aktualisieren.

### Test, der die Zusage tragen müsste

`broker/tests/sonde013_verdrahtung.rs:3189-3244` (B16, gescheiterter Append hinterlässt keinen
lokalen Ausschluss) — die Bühne enthält keinen **vorbestehenden** Ausschluss aus einer früheren
Intervention und keine Historie am Retention-Deckel. Die Prüfzeile
`.all(|e| e.ausschlussgrund.is_none())` würde durch F14 sogar *bestätigt* statt gebrochen.
`broker/tests/sonde014_verdrahtung.rs:2382-2388` prüft dieselbe Richtung.
**Kein bestehender Test würde rot.**

---

## Gemeinsame Ursache

**Die Auditbehauptung „F01, F02 und F03 sind drei getrennte Ursachen" trifft für den Fix zu,
nicht für das Muster.**

1. **F01 und F02 teilen dasselbe Entwurfsmuster** (unterschiedliche Sprache, Prozess und Datei):
   *Zustand unter Sperre kopieren → Arbeit ohne Sperre → Ergebnis zurückschreiben, ohne zu
   prüfen, ob der Eingangsstand noch der jüngste ist.* In F01 ist der Eingangsstand die
   Mitgliederliste (`Ipc.cpp:1461-1475`), in F02 die Evidenzaufnahme
   (`hypothese_verdrahtung.rs:72-93` + `:961`). Beide Riegel prüfen ein **anderes** Merkmal
   (Reload-Generation bzw. Intentgeneration) und halten es irrtümlich für hinreichend. Ein
   gemeinsamer Fix ist nicht möglich, eine gemeinsame **Regel** schon: „der Eingangsstand einer
   Rechnung/Publikation muss bis zur Übernahme identifizierbar sein und eine ältere Übernahme
   muss fallen".
2. **F02 und F14 teilen eine engere Ursache im Broker-Coordinator**: beide schreiben nach einer
   Lock-Lücke anhand eines Merkmals zurück, das sich in der Lücke ändern kann — F02 anhand einer
   implizit angenommenen Evidenzmenge, F14 anhand einer Deque-**Position**. Die für F14 längst
   formulierte Regel („über stabile Evidence-IDs zurückrollen", SONDE-013 §14) ist genau die
   Regel, die F02 auf der Eingangsseite fehlt. Eine gemeinsame Nacharbeitsrunde im Coordinator
   ist sinnvoll.
3. **F03 hat eine eigene, davon unabhängige Ursache**: eine Ersetzung *an alter Position* in
   einer geordneten Struktur (`queues.rs:228`). Kein Lock-Gap, kein veralteter Eingangsstand —
   ein Ordnungsfehler der Koaleszierung. Die Auditbehauptung ist hier bestätigt.
4. **F11 und F13 teilen ein drittes Muster**: *Scheitern ohne unterscheidbaren Status.* F11 gibt
   `false` zurück, nachdem bereits mutiert wurde (Ablehnung ist keine Ablehnung); F13 kodiert das
   Scheitern einer Messung als gültigen Messwert 0. In beiden Fällen ist das Fehlersignal im
   Wertebereich des Erfolgs versteckt.
5. **Übergreifend:** vier der sechs Befunde (F01, F02, F03, F14) betreffen dieselbe fachliche
   Kette — „letzter gültiger Stand über Modell, Store und Sender erhalten". Der Vorschlag in §7.1
   der Übergabe (F01, F02, F03 gemeinsam betrachten) ist tragfähig; F14 gehört sachlich dazu,
   auch wenn die Übergabe es unter Punkt 4 führt.

---

## Scope-Beweis

### Gelesen (Produkt- und Vertragsquellen an HEAD `39e18d3f`)

- `eq-copilot/plugin/src/prozessor/Ipc.cpp` — `:1085-1120`, `:1205-1290`, `:1420-1545`
- `eq-copilot/plugin/src/prozessor/Analyse.cpp` — `:320-360`
- `eq-copilot/plugin/src/SourcesModel.cpp` — `:330-400`, `:1415-1480`
- `eq-copilot/plugin/src/PluginProcessor.h` — `:838`, `:866`, `:1122`, `:1133`, `:1177-1178`,
  `:1778-1782` (Testhaken, Drainkommentar)
- `eq-copilot/plugin/src/PluginEditor.cpp:247` (Editor-Tick als Drain)
- `eq-copilot/plugin/state/NakamaState.cpp` — `:979-982`, `:1175-1200`, `:1690-1700`,
  `:1898-1915`, `:2020-2025`, `:2205-2470`, `:2650-2820`
- `broker/src/coordinator/hypothese_verdrahtung.rs` — `:55-120`, `:420-520`, `:945-1005`
- `broker/src/coordinator/evidenz.rs` — `:170-240`
- `broker/src/coordinator/invalidierung_verdrahtung.rs` — `:55-180`, `:220-265`
- `broker/src/transport/server_v3/queues.rs` — `:175-265`, `:314-400`
- `broker/src/transport/server_v3/listener.rs:600-635`, `verbindung.rs:55-80` (Threadmodell)
- `tools/eq-copilot/pruefe_session_soak.py` — `:119-200`, `:266-305`, `:415-440`, `:600-640`,
  `:980-1035`
- `tools/beweise.ps1:614` (A24-Behauptung)
- `eq-copilot/schemas/state/nakama-state-v2.md` — `:26-62`, `:136-140`, `:230-262`
- `docs/FL-Nakama-Sonden-Design-Entwurf.md` — `:3551`, `:4115-4130`, `:4240-4255`, `:4374`
- `docs/beweise/NAK-246.md` — §2-Matrix (M-11 bis M-13, M-21 bis M-28, M-31, M-38, M-39),
  `:1488-1489`, `:2810-2825`, `:3285`, `:4389`, `:5007-5008`, `:5342`, `:5733`
- `docs/beweise/SONDE-013.md` — `:20-28`, `:471-507`, `:4342`
- `docs/beweise/SONDE-014.md` — `:724`, `:743`, `:784`
- `docs/offene-punkte.md` — `:191` (NAK-163), `:261` (NAK-249), `:265` (NAK-253), `:267`
  (NAK-255), `:281` (NAK-269), `:343-345` (Nachzugsabsätze), `:439` (NAK-246)
- `.claude/skills/dirigent/SKILL.md` — `:228-250` (Klassenregeln), `:310-318`
- `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md` (vollständig),
  `pakete/02-state-ipc.md`, `03-broker.md`, `04-state-publication.md`, `07-root-integration.md`
- Tests: `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` (`:930-1045`, `:1322-1545`,
  `:1838-1841`), `Sonde014IntentTest.cpp` (Revisionsstellen),
  `broker/src/transport/server_v3/queues.rs` (`mod tests` `:325-530`),
  `broker/tests/sonde014_verdrahtung.rs:515-620`, `broker/tests/sonde013_verdrahtung.rs:3189-3250`

### Bewusst nicht gelesen (außerhalb der sechs Befunde)

- Alle Quellen zu F04–F10 und F12 (`sonde/SondeProcessor.*`, `dsp/DspKern.cpp`,
  `core/analysis/featureengine/*`, `src/HoerMarkierung.h`) sowie N01
  (`Sonde013TruePeakGoldenTest.cpp`).
- Die Rohbelege unter `docs/audits/2026-09-12-code-review-2/evidence/**` (Harnessquellen und
  Logs) — der Auftrag stellt die Quellenlektüre ausdrücklich über die historischen Repros; die
  Belege wurden nur in der Übergabe referenziert gelesen, nicht ausgewertet.
- `broker/src/transport/server_v3/griff.rs` (Schlüsselvergabe) — für F03 nicht nötig, weil die
  Ordnungsumkehr vollständig in `einreihen_eintrag` liegt; die Schlüsseltrennung selbst ist durch
  M-26 bereits gebaut und war nicht Prüfgegenstand.
- `eq-copilot/plugin/src/prozessor/State.cpp` über `:347` hinaus (fünfter Publikationsaufrufer;
  der Reloadpfad ist durch M-38/M-39 gedeckt und nicht Teil von F01).

### Fehlend / nicht entscheidbar ohne Ausführung

- **F03 Verbraucherwirkung durchgängig:** ob ein aus dem Broker in vertauschter Reihenfolge
  zugestelltes Paar im Plugin tatsächlich als `invalid/stale` ankommt, ist quellseitig
  konsistent, aber nicht durchgängig belegt (Wire-Ebene). Der Auditbeleg nutzt beschriftete
  Bytes, keine gültigen Wire-Nachrichten. Unverändert offen.
- **F13:** ob die historischen A24-Läufe echte RSS-Werte trugen, ist aus dem Repo nicht
  entscheidbar, ohne ein Rohmanifest auszuwerten; für die Einordnung des Orakelfehlers
  unerheblich.
- **F11:** der genaue MSVC-Überlaufwert (`-9223372036854775808`) ist UB und hier nicht
  nachgemessen; der Befund steht ohne diesen Wert, weil die fehlende obere Schranke und der
  Reader-Riegel `< 1` allein tragen.
- **Häufigkeit** aller Nebenläufigkeitsbefunde (F01, F02, F14) im Normalbetrieb — eine
  Wahrscheinlichkeitsaussage ist ohne Lauf nicht möglich und wird hier nicht behauptet.

---

# Teil B — F04, F05, F09, F12

# NAK-283 — Quellvalidierung B: F04, F05, F09, F12

Lesende Validierung der Befunde des zweiten unabhängigen Codeaudits
(Codex Astra, 12.09.2026, `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md`)
an der Quelle. Repo-HEAD beim Lesen: `39e18d3f`; Produktcode identisch mit der
Auditbasis `ea6ddd57`. Keine Änderung, kein Bau, kein Lauf, keine Repro.
Die Repro-Logs des Audits sind als historische Belege zitiert, nicht als Beweis
verwendet — jede Aussage unten steht auf der Quellenlesung an HEAD.

Klassenregeln aus `.claude/skills/dirigent/SKILL.md` (Zeilen 236–241):

> - **Defekt** — verletzt Matrix, Gate, Entwurf oder `CLAUDE.md`-Invariante →
> - **Lücke** — Matrix und Entwurf schweigen → Dirigent entscheidet die Regel
> - **Härtung** — von keiner Zusage verlangt → datiert ins Register.

---

## Übersicht

| ID | Status | Vorschlag Klasse | HEAD-Zeilen |
|---|---|---|---|
| F04 | BESTÄTIGT | DEFEKT | `sonde/SondeProcessor.cpp:265-277`, `:279-285`, `:340-342`, `:373-376` |
| F05 | BESTÄTIGT (Kern) · Tail-Teil NICHT BESTÄTIGT | LÜCKE | `sonde/SondeProcessor.h:134-137`, `:161`; `sonde/SondeProcessor.cpp:251-263` |
| F09 | BESTÄTIGT | DEFEKT | `dsp/DspKern.cpp:786-805`, `sonde/SondeProcessor.cpp:345-371` |
| F12 | BESTÄTIGT | LÜCKE | `src/HoerMarkierung.h:724-743`, `dsp/DspKern.cpp:1007-1008`, `sonde/SondeProcessor.cpp:353-355` |

---

## F04 — „Angenommener Vierkanalbus wird nur teilweise bearbeitet"

**Audit-Titel §2 wörtlich:** „VST3 akzeptiert vier Kanäle, verarbeitet aber nur die ersten zwei"
**Audit-Titel §4 wörtlich:** „F04 — Angenommener Vierkanalbus wird nur teilweise bearbeitet"
**Priorität laut Audit:** **P2**

### Quellzitat an HEAD

`eq-copilot/plugin/sonde/SondeProcessor.cpp:265-277` (Audit nennt `:265` — trifft an HEAD):

```cpp
bool SondeProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    // Ein Passthrough, der Kanaele erfinden oder verschlucken muesste, waere
    // kein Passthrough. Deshalb nur gleiche Ein-/Ausgangsbelegung, und keine
    // deaktivierten Hauptbusse.
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();

    if (ein.isDisabled() || aus.isDisabled())
        return false;

    return ein == aus;
}
```

`eq-copilot/plugin/sonde/SondeProcessor.cpp:279-285` (Audit nennt `:282` als processBlock — trifft):

```cpp
void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals keineDenormals;
    const int kanaele = std::min (puffer.getNumChannels(), 2);
    const int samples = puffer.getNumSamples();
```

`eq-copilot/plugin/sonde/SondeProcessor.cpp:340-342` — der DSP-Aufruf sieht nur zwei Zeiger:

```cpp
        float* kanalZeiger[2] = { puffer.getWritePointer (0),
                                  kanaele > 1 ? puffer.getWritePointer (1) : nullptr };
        dspKern->verarbeite (kanalZeiger, kanaele, samples);
```

`eq-copilot/plugin/sonde/SondeProcessor.cpp:373-376` — die JUCE-Sicherheitsschleife fasst
die überzähligen Kanäle bei gleicher Belegung ausdrücklich **nicht** an:

```cpp
    // Bei der erlaubten gleichen Busbelegung laeuft diese
    // JUCE-Sicherheitsschleife null Mal.
    for (int k = getTotalNumInputChannels(); k < getTotalNumOutputChannels(); ++k)
        puffer.clear (k, 0, puffer.getNumSamples());
```

Deklariert wird ein Stereopaar — `eq-copilot/plugin/sonde/SondeProcessor.cpp:133-135`:

```cpp
    : juce::AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
```

**Ergebnis der Zeilenprüfung:** alle vom Audit genannten Orte treffen an HEAD zu.
Kanal 3 und 4 eines angenommenen k40Music-Busses werden weder verarbeitet noch
geleert; sie laufen roh durch, während Kanal 1 und 2 EQ und Output-Trim erhalten.

### Anforderungsquelle

`docs/FL-Nakama-Sonden-Design-Entwurf.md:3451-3454` (§48.2 FL-Studio- und Hostvertrag), wörtlich:

> - Probe-Buslayouts werden in Mono und Stereo getestet. Active deklariert getrennte feste
>   `priority_sidechain`- und `compare_pre`-Aux-Busse; Main deklariert nur bei validierter
>   Contribution-Capability seine festen diskreten Aux-Busse. **Unpassende oder vertauschte Layouts
>   werden verständlich verweigert, nie automatisch umgedeutet.**

`docs/FL-Nakama-Sonden-Design-Entwurf.md:3646` (§50.1, Paket B) wörtlich:
„ladbares Main, passive und aktive Probe mit sicheren State-Migrationen, **festen Buslayouts**
und Float-/Double-Capability".

**Ein Vierkanalbus ist nirgends zugesagt.** Die Zusage lautet ausdrücklich, ihn
**abzulehnen**. Das ist keine Ableitung: die Schwesterklasse im selben Repo hält
die Regel bereits — `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:149-158`:

```cpp
bool EqCopilotProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    // V1: Mono und Stereo, Eingang == Ausgang. Anderes wird nicht still
    // heruntergemischt (Plan §9.4) — der Host bekommt ein klares Nein.
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();
    if (ein != aus)
        return false;
    return ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo();
}
```

Dazu `CLAUDE.md`-Wahrheitskern: „**Grundgesetz: nichts Ungefragtes.**" — ein Bus, den
der User nicht bestellt hat und den das Produkt nicht bedient, darf nicht angenommen werden.

### Auslösekette

Host-Aufruf: VST3 `IAudioProcessor::setBusArrangements` → JUCE-Wrapper →
`AudioProcessor::checkBusesLayoutSupported` → `SondeProcessor::isBusesLayoutSupported`
(Message-/Hostthread, vor `setActive`). Danach `process` → `processBlock`
(Audio-Thread) mit vier Kanälen im Puffer.
Bedingung: der Host verlangt eine Ein-/Ausgangsbelegung mit gleichem, aber
mehr als zweikanaligem ChannelSet.

**Erreicht FL Studio den Pfad heute?** Nein.
`eq-copilot/identity/host-capabilities-fl-v1.json` führt jeden gemessenen FL-Bus
mit `kanaele = 2` (Zeilen 70 und 78: „busse[2] (compare_pre): aktiv = true, **kanaele = 2**",
„busse[1] (priority_sidechain): aktiv = true, **kanaele = 2**"); ein Mehrkanalbus kommt
in keinem Rohfeld vor. Der Pfad ist ein Vertragsloch an der VST3-Grenze (fremde
Hosts, Validatoren, spätere Layouts), kein heutiger FL-Nutzerweg.

### Status

**BESTÄTIGT.** Quelle an HEAD trägt den Befund vollständig; die vom Audit genannten
Zeilen stimmen; das Messlog `evidence/host/vst3_full.log` („vst3_quad_set_result=0
vst3_input_channels=4 … vst3_quad_samples=0.199526235461,0.199526235461,0.10000000149,0.10000000149")
ist mit der Quellenlesung konsistent, wird aber nicht als Beweis gebraucht.

### Vorschlag Einordnung

**DEFEKT.** Verletzt den Entwurf §48.2 wörtlich („nie automatisch umgedeutet") und
§50.1 Paket B („festen Buslayouts").

**Korrekturrichtung — ausdrücklich „Ablehnen", nicht „Mehrkanal hinzufügen":**
`isBusesLayoutSupported` muss zusätzlich auf Mono oder Stereo einschränken, wie
`Hostbruecke.cpp:157` es für Gen bereits tut. Eine Erweiterung des DSP auf vier
Kanäle wäre Verarbeitung, die niemand eingeschaltet hat (Grundgesetz), und ist
durch keine Zusage gedeckt. Das Korrekturziel des Audits sagt dasselbe:
„Nicht unterstützte Buslayouts sind verlässlich abzulehnen; daraus soll keine
ungefragte Mehrkanalerweiterung entstehen."

### Tragender Test

Der Bestand hat die Prüfung — aber nur für die Schwesterklasse:
- `eq-copilot/plugin/tests/HostProbeTestMain.cpp:159-162` — „5.1 wird klar abgelehnt,
  nicht still gemischt" (fährt `HostProbeProcessor`, Zeile 119 ff.).
- `eq-copilot/plugin/tests/AuxSpikeTestMain.cpp:132-144` — vier Layoutfälle inkl.
  „5.1 auf einem Aux-Bus wird abgelehnt".
- `eq-copilot/plugin/tests/NullTestMain.cpp:71-74` — Mono-Layout am `EqCopilotProcessor`.

Für `SondeProcessor` existiert **kein** Buslayout-Fall: `SondeNullTestMain.cpp`
(734 Zeilen, fährt laut `:31` und `:52` die echte Produktklasse) enthält weder
`checkBusesLayoutSupported` noch `setBusesLayout`. **Heute würde kein Test rot.**
Der nächste Ort für den Rotbeweis ist `SondeNullTestMain.cpp`.

---

## F05 — „Reset des Hosts leert den DSP-Zustand nicht"

**Audit-Titel §2 wörtlich:** „Host-Reset lässt Filterzustand hörbar weiterlaufen"
**Audit-Titel §4 wörtlich:** „F05 — Reset des Hosts leert den DSP-Zustand nicht"
**Priorität laut Audit:** **P2**

### Quellzitat an HEAD

`eq-copilot/plugin/sonde/SondeProcessor.h:134-137` — die überschriebene
Lebenszyklusfläche; ein `reset()` fehlt (Audit nennt `:135`, an HEAD
`releaseResources()`, also genau die Stelle, an der `reset` stünde):

```cpp
    void prepareToPlay (double samplerate, int maxBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layout) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
```

`eq-copilot/plugin/sonde/SondeProcessor.h:161`:

```cpp
    double getTailLengthSeconds() const override        { return 0.0; }
```

Gegenprobe: `releaseResources` **räumt** tatsächlich auf —
`eq-copilot/plugin/sonde/SondeProcessor.cpp:251-263` (Audit nennt `:251`, trifft):

```cpp
void SondeProcessor::releaseResources()
{
    // Gegenstueck zu prepareToPlay (Beziehungen mitpruefen): der Kern gibt
    // seine Blockpuffer frei und setzt seine Baenke zurueck. Ein Block ohne
    // neue Vorbereitung laeuft dann unberuehrt durch (`verarbeiteStueck`).
    // Der bestaetigte Zustand bleibt im Transaktionskern; die naechste
    // Vorbereitung publiziert ihn wieder.
    const juce::ScopedLock callback (getCallbackLock());
    const juce::ScopedLock l (zustandSchloss);
    dspKern->freigeben();
    dspAusfuehrung->vergissLetztePublikation();
    publikationOffen = false;
}
```

Eingebundenes JUCE, `eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.h:936-941`
(Audit nennt `:935-941`, trifft):

```cpp
    /** A plugin can override this to be told when it should reset any playing voices.

        The default implementation does nothing, but a host may call this to tell the
        plugin that it should stop any tails or sounds that have been left running.
    */
    virtual void reset();
```

Eingebundenes JUCE, `eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3612-3618`
(Audit nennt `:3612-3617`, trifft):

```cpp
    tresult PLUGIN_API setProcessing (TBool state) override
    {
        if (! state)
            getPluginInstance().reset();

        return kResultTrue;
    }
```

**Ergebnis der Zeilenprüfung:** die Ursachenkette stimmt Zeile für Zeile.
`setProcessing(false)` ruft ausschließlich `reset()`, `reset()` ist die leere
JUCE-Basis, und `releaseResources`/`prepareToPlay` laufen auf diesem Weg nicht
(`setActive(false)` ruft `releaseResources` — `:2837` —, `setProcessing(false)`
ruft es nicht).

### Anforderungsquelle

**Zum Tail-Teil des Befunds existiert eine datierte Zusage — und sie sagt das Gegenteil.**
`docs/beweise/SONDE-015.md:1104`, Matrixzeile **M-51**, wörtlich:

> | M-51 | Der Host fragt Latenz und Tail | Die gemeldete Latenz bleibt **konstant 0 Samples** —
> `setLatencySamples` wird nie mit einem Wert ungleich 0 gerufen —, `getTailLengthSeconds()`
> bleibt 0,0, und es gibt kein Lookahead und keine lineare Phase. Der **Tailbeweis** gilt für
> den Passthrough- und Bypasszustand; **ein eingeschalteter resonanter Filter klingt
> naturgemäß aus**, und die Runner-Behauptung sagt genau das und nicht mehr. |
> **BELEGT** · **ENTSCHIEDEN (R9-Feinheit, §5.9)** · BAULÜCKE | **A16**, Fälle
> `latenz_bleibt_null` und `kein_tail_im_passthrough` |

`CLAUDE.md`-Wahrheitskern, wörtlich: „Ausgeschaltet ist der Pfad im Nulltest
bitidentisch; sonst Passthrough sampleidentisch, **ohne Latenz oder Tail**." —
die Zusage „ohne Tail" ist auf den Passthrough begrenzt, nicht auf den
eingeschalteten EQ.

**Zum Reset-Teil existiert keine Zusage.** Gesucht und nicht gefunden:
- `docs/beweise/SONDE-015.md` — keine Matrixzeile zu `reset`, `setProcessing`,
  Suspend oder Smart Disable auf DSP-Zustandsebene (M-41 bis M-56 decken Pool,
  Ownership, ACK, Echtzeit, Hörmatrix; M-51 deckt Latenz/Tail).
- `docs/FL-Nakama-Sonden-Design-Entwurf.md:3449-3450` — „Smart Disable/Suspend wird
  als fehlende **Coverage** behandelt; nach Resume oder Discontinuity beginnt eine
  neue Transportepoche." Das ist die Analysezeitachse, nicht der Filterzustand.
- `docs/FL-Nakama-Sonden-Design-Entwurf.md:3030-3031` (§44.3) — „Läuft kein Callback,
  bleibt Main sichtbar bei `gespeichert · wird bei Audio fortgesetzt aktiv`. Der nächste
  `prepareToPlay`/Audioblock übernimmt den bestätigten Zustand **vor** seiner Ausgabe."
  Das betrifft die **Parameter**hoheit, nicht die Audiohistorie.

Die nächstliegende Regel ist keine Matrixzeile, sondern eine `CLAUDE.md`-Arbeitsregel:

> Beziehungen mitprüfen: speichern↔laden, **starten↔stoppen**, öffnen↔schließen,
> verbinden↔trennen, **aktivieren↔abklingen**, installieren↔Rückweg gehören in
> denselben Änderungssatz.

`setProcessing(true)`↔`setProcessing(false)` ist genau „starten↔stoppen", und
„aktivieren↔abklingen" benennt die Reset-/Tail-Beziehung wörtlich. Der DSP-Kern
kam mit SONDE-015 Etappe 4a hinzu; das Paar `prepareToPlay`↔`releaseResources`
wurde mitgebaut (`SondeProcessor.cpp:252-256` sagt das selbst), der dritte
Hosteintritt `reset` nicht.

### Auslösekette

Host-Aufruf: VST3 `IAudioProcessor::setProcessing(false)` → JUCE-Wrapper `:3614-3615`
→ `AudioProcessor::reset()` (leere Basis). Thread: Hostthread außerhalb des
Audio-Callbacks. Bedingung: EQ eingeschaltet, engagierte Bank mit Filterhistorie
(resonanter Bell). Der nächste `setProcessing(true)` und der folgende Block lesen
`Zust::s1/s2` der Biquads unverändert weiter.

**Erreicht FL Studio den Pfad?** Sehr wahrscheinlich ja, und zwar häufiger als jeder
andere hier geprüfte Weg: `setProcessing` ist der reguläre VST3-Transportweg, und
FL fährt zusätzlich Smart Disable — der Entwurf nennt es in §48.2 ausdrücklich als
Hostverhalten (`:3449`) und §44.3 bindet das zweistufige ACK genau daran
(„Wegen Stop und FL Smart Disable besitzt das ACK zwei Stufen", `:3024`).
Ein FL-Beleg für den Audioeffekt liegt nicht vor (Audit: „kein FL Studio,
kein physischer Audiotreiber-/XRun-Beleg").

### Status

**BESTÄTIGT für den Resetkern** — `reset()` ist an HEAD nicht überschrieben, die
JUCE-Basis ist leer, der VST3-Wrapper ruft auf diesem Weg nichts anderes.

**NICHT BESTÄTIGT für den Tail-Teil.** Der Satz des Audits „Das Modul meldet
gleichzeitig null Tail-Samples" beschreibt kein Abweichen, sondern die eingehaltene
Zusage M-51 (ENTSCHIEDEN, R9-Feinheit §5.9): `getTailLengthSeconds()` **bleibt** 0,0,
und „ein eingeschalteter resonanter Filter klingt naturgemäß aus" ist dort
ausdrücklich mitentschieden. `SondeProcessor.h:161` erfüllt die Zeile.
Der Audit zieht dieselbe Grenze in seinem Korrekturziel („nur die Tail-Zahl zu
ändern beseitigt den beobachteten Resetfehler nicht").

### Vorschlag Einordnung

**LÜCKE.** Matrix und Entwurf schweigen zu `reset()`; M-51 regelt nur die
Tail-*Meldung* und deckt den Resetweg nicht. Nach Skillregel ist das die
Lückenklasse — der Dirigent entscheidet die Regel („Ein Host-Reset beendet die
Audiohistorie des aktiven Pfades, ohne den bestätigten Parametersatz zu verlieren"),
danach braucht sie eine Matrixzeile und einen Rotbeweis.

Gegen DEFEKT spricht: keine Matrix-, Gate- oder Entwurfszeile bricht.
Für die Behandlung als lückenschließende Nacharbeit (nicht als bloßer
Registereintrag) spricht die `CLAUDE.md`-Beziehungsregel starten↔stoppen /
aktivieren↔abklingen und dass der hörbare Rest ein echtes Produktverhalten ist.

**Korrekturrichtung — ausdrücklich „Zurücksetzen", nicht „Tail hinzufügen":**
`SondeProcessor::reset()` überschreiben und die Filter-/Fadehistorie des aktiven
Pfades beenden (der Kern hat die Mittel bereits: `HoerMarkierung.h:745-757`
`resetZustaende()`/`hartAus()`, `DspKern` heilt Zustände in `heileZustaende`).
`getTailLengthSeconds()` bleibt 0,0 — jede Änderung dort widerspräche M-51.
Kein Lookahead, keine lineare Phase, keine gemeldete Latenz.

### Tragender Test

Es gibt **keinen**. `reset()` kommt in keinem Test unter
`eq-copilot/plugin/tests/` als Aufruf vor (geprüft über `\.reset()` / `->reset()`
in `DspGoldenTestMain.cpp` und `SondeNullTestMain.cpp`, und `reset` überhaupt in
`SondeNullTestMain.cpp`). `SondeNullTestMain.cpp:103` fährt nur
`prepareToPlay (rate, maxBlock)`.
Die Zusage M-51 trägt heute **A16** (`latenz_bleibt_null`, `kein_tail_im_passthrough`)
— beide messen den Passthrough und würden nicht rot. **Heute würde kein Test rot.**

---

## F09 — „Analyseverlust im DSP überspringt den Zeit- und Lückenvertrag der Queue"

**Audit-Titel §2 wörtlich:** „Verworfener DSP-Tap hinterlässt keine Lücke auf der Analysezeitachse"
**Audit-Titel §4 wörtlich:** „F09 — Analyseverlust im DSP überspringt den Zeit- und Lückenvertrag der Queue"
**Priorität laut Audit:** **P2**

### Quellzitat an HEAD

`eq-copilot/plugin/dsp/DspKern.cpp:786-805` (Audit nennt `:788`, trifft):

```cpp
    // M-48: verworfen wird die ANALYSE, nie Audio. Ein Block groesser als
    // `maxBlock` laeuft deshalb in Stuecken durch statt zurueckzuweisen.
    if (maxBlockGroesse > 0 && numSamples > maxBlockGroesse)
    {
        zaehlerVerworfen.fetch_add (1, std::memory_order_relaxed);
        int versatz = 0;
        while (versatz < numSamples)
        {
            const int n = std::min (maxBlockGroesse, numSamples - versatz);
            float* teil[2] = { kanaele[0] + versatz,
                               numKanaele > 1 ? kanaele[1] + versatz : nullptr };
            verarbeiteStueck (teil, numKanaele, n);
            versatz += n;
            if (versatz < numSamples && teilstueckHaken != nullptr)
                teilstueckHaken (teilstueckKontext);
        }
        // Der Tap dieses Blocks ist keine kohaerente Messung mehr.
        tapGueltig = 0;
        return;
    }
```

`eq-copilot/plugin/sonde/SondeProcessor.cpp:345-371` (Audit nennt `:345`, trifft):

```cpp
        // Die Analyse misst `post_committed` (§44.2: Session-Landkarte und
        // Recall beziehen sich darauf), nie den Hoermatrix-Ausgang (M-57).
        AnalyseQueue::TapQuelle abgriff;
        const double* tapL = dspKern->tap (nakama::dsp::Tap::postCommitted, 0);
        const double* tapR = dspKern->tap (nakama::dsp::Tap::postCommitted, 1);
        if (tapL != nullptr && tapR != nullptr && dspKern->tapLaenge() == samples
            && (size_t) samples <= analyseL.size())
        {
            for (int i = 0; i < samples; ++i) analyseL[(size_t) i] = (float) tapL[i];
            if (kanaele > 1)
                for (int i = 0; i < samples; ++i) analyseR[(size_t) i] = (float) tapR[i];
            abgriff.links  = analyseL.data();
            abgriff.rechts = kanaele > 1 ? analyseR.data() : nullptr;
            // Ganz oder gar nicht. Rueckstau verwirft Analyse, niemals Audio.
            analyseQueue.veroeffentliche (&abgriff, 1, kanaele, samples, stempel);
        }
        else if (committedRuhtImPassthrough())
        {
            // Ohne gueltigen Tap (unvorbereitet oder uebergrosser Block) ist der
            // ruhende Passthrough-Ausgang der unberuehrte Eingang: die Analyse
            // liest ihn wie bisher. Rechnet der Kern, ist der Tap dieses Blocks
            // verworfen und gezaehlt (M-48) - die Analyse faellt, nie Audio.
            abgriff.links  = puffer.getReadPointer (0);
            abgriff.rechts = kanaele > 1 ? puffer.getReadPointer (1) : nullptr;
            analyseQueue.veroeffentliche (&abgriff, 1, kanaele, samples, stempel);
        }
    }
```

Die Schaltbedingung des zweiten Zweigs, `eq-copilot/plugin/sonde/SondeProcessor.cpp:1214-1225`:

```cpp
bool SondeProcessor::committedRuhtImPassthrough() const noexcept
{
    int aktiv = -1, quelle = -1, candidate = -1, candidateQuelle = -1;
    dspKern->gefahreneSlots (aktiv, quelle, candidate, candidateQuelle);
    const auto passthrough = [this] (int slot) noexcept
    {
        if (slot < 0) return true;
        const auto& p = dspKern->pool().bank (slot).programm;
        return ! p.eqEngagiert || p.hardBypass;
    };
    return passthrough (aktiv) && passthrough (quelle);
}
```

Und `eq-copilot/plugin/dsp/DspKern.h:188-190` plus `DspKern.cpp:272`:

```cpp
        nullptr. Immer `tapLaenge()` Samples lang. */
    const double* tap (Tap t, int kanal) const noexcept;
    int tapLaenge() const noexcept { return tapGueltig; }
```
```cpp
    if (tapGueltig <= 0 || kanal < 0 || kanal > 1) return nullptr;
```

**Ergebnis der Zeilenprüfung:** die Kette schließt. Bei übergroßem Block und
engagiertem EQ liefert `tap()` `nullptr` (erster Zweig fällt aus) und
`committedRuhtImPassthrough()` liefert `false` (zweiter Zweig fällt aus).
`analyseQueue.veroeffentliche` wird **gar nicht** gerufen. Die Buchführung der
Queue — lokaler Strom, `kFlagLueckeDavor`, `segment`, `oversizeDrops`,
`verloreneFrames` — läuft deshalb für diesen Block nicht an.

### Anforderungsquelle

`eq-copilot/plugin/core/StampedAudioQueue.h:32-43`, wörtlich (Kopfvertrag der Klasse):

```
// EHRLICHKEIT STATT SANIERUNG (dieselbe Regel wie in NakamaHostBridge.h):
//   * Der lokale Strom (`stromVon`) zaehlt JEDEN Hostblock mit, auch den
//     verworfenen. Ein Zaehler, der nur die angenommenen Frames zaehlt, koennte
//     "hier fehlt Zeit" nicht mehr ausdruecken - genau der Fehler, den der
//     AbstractFifo-Weg macht.
//   * `projectSampleStart` traegt ein eigenes Gueltigkeitsbit. Ohne Hostkontext
//     ist Projektzeit unbewiesen (§32.3) - dann gilt nur der lokale monotone
//     Verlauf (§53.6, Verhalten ohne Beweis von `project_time_samples`).
//   * Ein Hostblock ueber `maxBlockFrames` wird als GANZES nur fuer die Analyse
//     verworfen (`oversizeDrops`) - `maximumExpectedSamplesPerBlock` ist bei
//     JUCE nur ein Hinweis (§48.1), FL zerteilt Puffer sogar bis auf 1 Sample
//     (Capabilityreport S4). Der Audiopfad beruehrt das nicht.
```

`docs/FL-Nakama-Sonden-Design-Entwurf.md:1938-1940`, wörtlich — der Lückenvertrag
nennt den Oversize-Block ausdrücklich:

> `continuity_segment` trennt innerhalb derselben echten Transportepoche lokale Analyselücken wie
> Queue-Drop oder **Oversize-Block**. Es steigt nach einem verlorenen Ganzblock; **kein Fenster darf die
> Grenze überbrücken.** Die Host-Zeitachse wird dadurch nicht fälschlich als Seek bezeichnet.

`docs/beweise/SONDE-014.md:4026` (Runner-Behauptung des Beins **B5**, gemessen und beglaubigt):

> … Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/
> Neuanlauf/Beweislagewechsel **trennen JEDES offene Fenster** … und **ein Drop zaehlt als Segment,
> nicht als Epoche**.

`CLAUDE.md`-Wahrheitskern: „Überlast verwirft Analyseframes, nie Audio." — die
Audio-Hälfte hält (der Block läuft gestückelt bitgleich durch, siehe Test unten);
verletzt ist die Ehrlichkeitshälfte, dass der Verlust auf der Analysezeitachse sichtbar bleibt.

Die Entwurfszeile nennt den Fall beim Namen; **der Vertrag schweigt hier nicht.**

### Auslösekette

Host-Aufruf: `processBlock` mit `numSamples > maxBlockGroesse`, wobei
`maxBlockGroesse` aus `prepareToPlay (samplerate, maxBlock)` stammt
(`SondeProcessor.cpp:232-236`: `dspKern->bereiteVor (sichereRate, maxBlock)`).
Thread: Audio-Thread. Bedingung: EQ eingeschaltet und mindestens eine engagierte,
nicht hart gebypasste Bank (sonst greift der Passthrough-Zweig `:361` und die Queue
läuft korrekt).
Der Host muss also mehr Samples liefern, als er selbst als
`maxSamplesPerBlock` deklariert hat. Der JUCE-Wrapper übergibt den deklarierten
Wert bei `setActive(true)` (`juce_audio_plugin_client_VST3.cpp:2829-2833`);
`setupProcessing` allein ruft `prepareToPlay` nicht (`:3607`, `CallPrepareToPlay::no`).

**Erreicht FL Studio den Pfad?** Nicht abschließend entscheidbar aus den gelesenen
Quellen. Der Entwurf und der Queue-Kopf halten den Fall ausdrücklich für real
(„`maximumExpectedSamplesPerBlock` ist bei JUCE nur ein Hinweis (§48.1)"), und die
Existenz des Stückelungspfads plus des B-22-Tests zeigt, dass das Produkt ihn
erwartet. Der Capabilityreport belegt für FL die Gegenrichtung (Zerteilung bis
1 Sample), nicht das Überschreiten. **Auslöser belegt reachable im Vertragssinn,
nicht in einer FL-Messung.**

### Status

**BESTÄTIGT.** Die Auslassung steht an HEAD in beiden Dateien, die Bedingung ist
aus der Quelle allein ableitbar, und die verletzte Zusage steht wörtlich im Entwurf
und im Klassenkopf der Queue. Das Auditlog `evidence/audio/gap-validated-log.txt`
(„beforeLocal=0 beforeFrames=64 afterLocal=64 expectedAfterLocal=192 sameSegment=1
gapFlag=0 quarantineAccepts=1 queueDroppedFrames=0") ist konsistent; die
Einordnung hängt nicht daran.
Einschränkung des Audits übernommen: „Kein vollständig instanziierter
SondeProcessor in diesem speziellen Fall" — die Sonde-Abzweigung war nachgebildet.
Die Quellenlesung ersetzt das hier: der fehlende Aufruf ist in
`SondeProcessor.cpp:347-370` direkt sichtbar und braucht keine Instanz.

### Vorschlag Einordnung

**DEFEKT.** Verletzt `docs/FL-Nakama-Sonden-Design-Entwurf.md:1938-1940` wörtlich
(„Es steigt nach einem verlorenen Ganzblock; kein Fenster darf die Grenze
überbrücken") und den Klassenvertrag `StampedAudioQueue.h:33-35`
(„Der lokale Strom zaehlt JEDEN Hostblock mit, auch den verworfenen").
Die Runner-Behauptung von B5 (SONDE-014) behauptet die Trennung als gemessen —
sie misst sie aber nur am Consumer, nicht an diesem Produzentenweg.

Kein Audioverlust: `verarbeiteStueck` läuft über den ganzen Block. Der Befund ist
ein Analyse-Ehrlichkeitsdefekt, kein Grundgesetzverstoß gegen „nie Audio".

### Tragender Test

Die Zusage müsste hier tragen:
- `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:3060-3081` — **M-48 / B-22**.
  Prüft `k->verworfeneAnalyseframes() == verworfenVorher + 1 && k->tapLaenge() == 0`
  und die Bitgleichheit aller 1024 Samples gegen 4×256. Fährt **nur `DspKern`**,
  nie `SondeProcessor` und nie `analyseQueue`. **Würde nicht rot.**
- `eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:1380-1385` — „G1 Drop
  (kFlagLueckeDavor) … §32.3: SEGMENT, nicht Epoche". Baut das Flag von Hand
  (`++s.segment; auto b = s.bauen (rt::kFlagLueckeDavor);`) und misst die
  Consumerreaktion. **Würde nicht rot.**
- `eq-copilot/plugin/tests/QueueStressTestMain.cpp:92, :234, :280` — fährt die Queue
  direkt mit `maxBlockFrames 256`. Sie wird im Befundfall nie gerufen. **Würde nicht rot.**

**Heute würde kein Test rot.** Der fehlende Fall ist ein Prozessorfall
(`SondeNullTestMain.cpp` oder ein neuer B6-Fall): übergroßer Block bei
engagiertem EQ → nächster angenommener Block trägt `kFlagLueckeDavor` und ein
erhöhtes `segment`.

---

## F12 — „float-Verengung liegt hinter der Endlichkeitsprüfung"

**Audit-Titel §2 wörtlich:** „Endliche double-Werte werden beim float-Ausgang ungezählt zu Inf"
**Audit-Titel §4 wörtlich:** „F12 — float-Verengung liegt hinter der Endlichkeitsprüfung"
**Priorität laut Audit:** **P3** („Numerischer Rand weit außerhalb gewöhnlicher Audiopegel")

### Quellzitat an HEAD

`eq-copilot/plugin/src/HoerMarkierung.h:724-743` (Audit nennt `:735`, trifft — dort
steht die `isfinite`-Prüfung auf der double-Zwischengröße):

```cpp
    static void tdf2Lauf (float* d, int n, const BiquadKoeff& c, Zust& z,
                          std::uint64_t& nichtEndlich)
    {
        double s1 = z.s1, s2 = z.s2;
        for (int i = 0; i < n; ++i)
        {
            double x = (double) d[i];
            if (! std::isfinite (x)) { x = 0.0; ++nichtEndlich; }
            const double y = c.b0 * x + s1;
            s1 = c.b1 * x - c.a1 * y + s2;
            s2 = c.b2 * x - c.a2 * y;
            if (std::isfinite (y))
                d[i] = (float) y;
            else
                { d[i] = 0.0f; ++nichtEndlich; }
        }
        if (! std::isfinite (s1)) { s1 = 0.0; ++nichtEndlich; }
        if (! std::isfinite (s2)) { s2 = 0.0; ++nichtEndlich; }
        z.s1 = s1; z.s2 = s2;
    }
```

`eq-copilot/plugin/dsp/DspKern.cpp:991-1008` (Audit nennt `:1007`, trifft):

```cpp
    for (size_t i = 0; i < schreibBis; ++i)
    {
        double l = 0.0, r = 0.0;
        hoerWert (hoerLaufend, i, l, r);

        if (hoerFadeRest > 0)
        {
            // Linear, wie jeder andere Uebergang des Kerns (§5.2 Feinheit 2).
            double lv = 0.0, rv = 0.0;
            hoerWert (hoerVorher, i, lv, rv);
            const double t = 1.0 - (double) hoerFadeRest / (double) kFadeSamples;
            l = lv * (1.0 - t) + l * t;
            r = rv * (1.0 - t) + r * t;
            --hoerFadeRest;
        }

        kanaele[0][i] = (float) l;
        if (numKanaele > 1) kanaele[1][i] = (float) r;
    }
```

Der Riegel, den die Verengung überholt — `eq-copilot/plugin/dsp/DspKern.cpp:849-865`:

```cpp
    // --- der Nicht-Endlich-Riegel, EINMAL je Stueck ----------------------
    // R9: verriegelt und gezaehlt, VOR jedem Filterzustand - und nur, wenn
    // ueberhaupt eine engagierte Bank rechnet. Der Dry-Zweig bleibt roh.
    const auto engagiert = [this] (const PfadZustand& z) noexcept
    {
        return (z.aktiv >= 0 && ! istPassthrough (z.aktiv)) || (z.quelle >= 0 && ! istPassthrough (z.quelle));
    };
    if (engagiert (pfade[0]) || (candRechnet && engagiert (pfade[1])))
    {
        for (size_t i = 0; i < n; ++i)
        {
            double l = dryL[i], r = dryR[i];
            if (! std::isfinite (l)) { l = 0.0; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }
            if (! std::isfinite (r)) { r = 0.0; zaehlerEingaenge.fetch_add (1, std::memory_order_relaxed); }
            eingL[i] = l; eingR[i] = r;
        }
    }
```

Die dritte Verengungsstelle, `eq-copilot/plugin/sonde/SondeProcessor.cpp:353-355`
(Audit nennt `:353-355`, trifft):

```cpp
            for (int i = 0; i < samples; ++i) analyseL[(size_t) i] = (float) tapL[i];
            if (kanaele > 1)
                for (int i = 0; i < samples; ++i) analyseR[(size_t) i] = (float) tapR[i];
```

Zähler-Verdrahtung der Hörmarkierung, `eq-copilot/plugin/src/HoerMarkierung.h:657-658`
und `:690-692`:

```cpp
        if (nichtEndlichImBlock != 0)
            wetNichtEndlich.fetch_add (nichtEndlichImBlock, std::memory_order_relaxed);
```

**Ergebnis der Zeilenprüfung:** alle drei vom Audit genannten Orte treffen an HEAD.
An jeder Stelle wird ein `double` nach `float` verengt, nachdem (oder ohne dass) die
Endlichkeit auf der double-Darstellung geprüft wurde. Ein endlicher `double` über
`FLT_MAX` wird dabei zu `inf`, ohne dass ein Zähler steigt.
Nebenbefund aus derselben Lesung: die Konvertierung eines nicht darstellbaren
Werts ist in C++ undefiniert ([conv.double]); der Beobachtungswert `inf` ist das
MSVC-Verhalten, nicht eine Garantie.

### Anforderungsquelle

`docs/beweise/SONDE-015.md:1102`, Matrixzeile **M-49**, wörtlich:

> | M-49 | Ein nicht-endlicher **Eingangssample** erreicht den **aktiven** Pfad | Er wird
> **vor jedem Filterzustand** durch 0,0 ersetzt und gezählt; der Zähler ist lockfrei lesbar.
> Zusätzlich prüft der Kern am **Blockrand** die **Filterzustände** … Damit bleiben die
> Filterzustände endlich … | **ENTSCHIEDEN (R9-Feinheit, §5.9)** |

`docs/beweise/SONDE-015.md:2147`, R9 im Wortlaut der Abweichungstabelle:

> | 4 | **R9:** „Nicht-endliche **Eingangssamples** … werden verriegelt und gezählt
> (**Filterzustände** bleiben endlich)" | Je Sample ein `isfinite` auf dem **Eingang**;
> je **Blockrand** eine Prüfung der Filterzustände mit Nullung und eigenem Zähler. |

`docs/beweise/NAK-180.md:309`, Matrixzeile **N-14** — die einzige Zeile, die den
Fall „alle Eingangssamples endlich" ausdrücklich regelt:

> | N-14 | wie N-13, aber **alle Eingangssamples sind endlich** | normaler Betrieb |
> Der Riegel ändert **kein Bit**: `isfinite(y)` ist wahr, die Konvertierung ist dieselbe
> wie heute. **Der Zähler bleibt 0.** Goldens und Nulltest bleiben unverändert. |

`docs/beweise/NAK-180.md:308`, **N-13**, regelt den Gegenfall (nicht-endlicher Eingang).
`docs/beweise/SONDE-015.md:1103`, **M-50**, regelt den Passthrough
(„sanitisiert nichts … kein Zähler steigt").

**Keine dieser Zeilen sagt etwas über die ausgegebene float-Darstellung eines
endlichen double.** N-14 ist im Gegenteil buchstäblich erfüllt: `isfinite(y)` **ist**
wahr, und der Zähler **bleibt** 0 — genau wie zugesagt. Die Zeile setzt stillschweigend
voraus, dass aus endlichem `y` ein endliches `float` wird; diese Voraussetzung
benennt sie nicht als Zusage.

Die nächstliegende ausdrückliche Regel steht in einer Nachbarzeile,
`docs/beweise/NAK-180.md:311`, **N-16**: „Die Konvertierung **sättigt und ist nie UB**."
Sie gilt dort für `samplerate → int`, nicht für den Audioausgang.

`CLAUDE.md`-Wahrheitskern: „**NaN-Ehrlichkeit.** Nicht-endliche Werte werden verriegelt
und gezählt; Nyquist gekappt; ohne genügend endliche Nachbarn keine Basislinie."
Diese Formulierung ist breiter als R9; die datierte Konkretisierung R9-Feinheit §5.9
(ENTSCHIEDEN) bindet sie jedoch ausdrücklich an Eingang und Filterzustand.

### Auslösekette

Host-Aufruf: `processBlock` → `DspKern::verarbeite` → `verarbeiteStueck`
(Audio-Thread), bzw. `HoerMarkierungDsp::verarbeite` → `tdf2Lauf`.
Bedingung: ein **endlicher** Eingang so groß, dass Filter- oder Trimverstärkung den
Zwischenwert über `FLT_MAX` (≈3,4·10^38) hebt. Der Auditlauf brauchte dafür
0,75 × FLT_MAX am Eingang bei +6 dB; das sind rund 10^38 in einem Wertebereich,
der bei Audio ±1,0 heißt.

**Erreicht FL Studio den Pfad?** Praktisch nein. FL liefert float-Audio aus realen
Quellen; um 10^38 in den Puffer zu bekommen, müsste eine vorgeschaltete Kette bereits
absurd entgleist sein — und ein `inf` davor fängt der Eingangsriegel `:861-862` ab.
Der Befund ist ein Zahlenrand, kein Betriebsweg. Das sagt auch der Audit selbst
(„Seltene numerische Grenzbereiche … normale Musikpegel … lösen sie nicht aus").

### Status

**BESTÄTIGT** als Quellenbefund: an allen drei Orten liegt die Verengung hinter
bzw. ohne die Endlichkeitsprüfung, und der jeweilige Zähler kann den Fall nicht sehen.
Das Auditlog `evidence/audio/repro-validated-log.txt` („DSP +6dB 0.75-FLT_MAX
finite=0 value=inf inputNonfinite=0 stateHealed=0 doubleTapFinite=1" /
„markerOutputNonfinite=2348 firstSample=2796 counter=0") ist mit der Quelle
konsistent.
**Nicht bestätigt** ist dagegen, dass damit eine bestehende Zusage bricht — siehe
Anforderungsquelle.
Für den Tap-Weg `SondeProcessor.cpp:353-355` gilt die Beweisgrenze des Audits
unverändert: „deren Wirkung wurde hier nicht separat durch die volle Sonde gemessen."

### Vorschlag Einordnung

**LÜCKE.** Matrix (M-49, M-50, N-13, N-14) und Entwurf regeln den nicht-endlichen
**Eingang** und den **Filterzustand**; die ausgegebene float-Darstellung eines
endlichen double ist ungeregelt. Nach Skillregel („Matrix und Entwurf schweigen")
ist das die Lückenklasse; der Dirigent entscheidet die Regel — naheliegend nach
dem Muster N-16: „Die Konvertierung sättigt und ist nie UB", plus ein eigener
Ausgangszähler oder eine Erweiterung des Wet-/Eingangszählers.

Argument für die Alternative **DEFEKT**: liest man `CLAUDE.md` „Nicht-endliche Werte
werden verriegelt und gezählt" als Zusage über jeden nicht-endlichen Wert
einschließlich des ausgegebenen, bricht sie hier. Ich schlage das nicht vor, weil
die datierte Konkretisierung R9-Feinheit §5.9 den Umfang ausdrücklich auf Eingang
und Filterzustand legt und N-14 den Fall „endlicher Eingang" mit „Zähler bleibt 0"
positiv regelt.

Argument gegen **HÄRTUNG**: der Kommentar an `HoerMarkierung.h:715-718` erklärt
den Riegel selbst damit, dass „`y` … auch bei endlichem `x` nicht endlich werden"
kann — die gebaute Prüfung verfehlt also ihren eigenen erklärten Zweck um eine
Darstellungsstufe. Das ist mehr als „von keiner Zusage verlangt", auch wenn keine
Vertragszeile es fordert.

**Korrekturrichtung:** Endlichkeit an der ausgegebenen Darstellung prüfen bzw.
sättigen, Zähler mitführen. Ausdrücklich **kein** musikalischer Limiter und keine
Änderung am neutralen Passthrough (M-50: „Der Passthrough sanitisiert nichts") —
der Riegel darf nur greifen, wo eine engagierte Bank ohnehin rechnet, sonst bricht
er M-01/M-05/M-50 und den Nulltest.

### Tragender Test

- `eq-copilot/plugin/tests/MarkierungTestMain.cpp:564-628` — **NAK-180 R4, Matrix
  N-13 bis N-15**, „der Wet-Pfad latcht nie einen nicht-endlichen Zustand".
  Der Fall injiziert `std::numeric_limits<float>::infinity()` (`:624`) und prüft
  danach `nichtEndlicheWetSamples() > 0` (`:626`). Ein **endlicher** Eingang nahe
  `FLT_MAX` kommt nicht vor. **Würde nicht rot** — im Gegenteil: der Befundfall
  erfüllt die Erwartung von `:618` („bei endlichem Material zaehlt der Riegel NICHTS").
- `eq-copilot/plugin/tests/DspGoldenTestMain.cpp` — M-49-Fälle
  (`nichtendlicher_eingang_wird_verriegelt_und_gezaehlt`,
  `nichtendlicher_filterzustand_wird_am_blockrand_geheilt`) messen den Eingang und
  den Zustand. **Würden nicht rot.**

**Heute würde kein Test rot.** Der fehlende Fall ist ein endlicher Eingang, dessen
Zwischenwert `FLT_MAX` überschreitet, mit Erwartung an Ausgang **und** Zähler.

---

## Gemeinsame Ursache

Alle vier Befunde sitzen an derselben Bruchkante: **SONDE-015 Etappe 4a hat aus
Probeeq einen rechnenden DSP gemacht, aber die Hostvertrags- und
Ehrlichkeitsflächen, die für den reinen Passthrough geschrieben waren, nicht
mitgezogen.** Der Bestand vor 4a ist in `docs/beweise/SONDE-015.md:719-730`
(§2.2 (a)) beschrieben: „Probeeq heute: ein Passthrough ohne Parameter, ohne DSP,
ohne Editor … Er kopiert Leseblöcke in die vorallokierte Analysequeue … und
schreibt an keiner Stelle in den Audiopuffer."

Konkret:

1. **F04** — `isBusesLayoutSupported` begründet sich selbst noch mit „Ein Passthrough,
   der Kanaele erfinden oder verschlucken muesste, waere kein Passthrough"
   (`:267-269`). Für einen Passthrough war „Eingang == Ausgang" hinreichend; für
   einen rechnenden EQ, der nur zwei Kanäle kennt, ist es das nicht mehr. Gen hat
   die strengere Regel seit je (`Hostbruecke.cpp:157`).
2. **F05** — der Lebenszyklus wurde um `prepareToPlay`↔`releaseResources` erweitert
   (`SondeProcessor.cpp:252-256` nennt die Beziehungsregel im Kommentar), der dritte
   Hosteintritt `reset` blieb auf der leeren JUCE-Basis. Ein Passthrough hat nichts
   zurückzusetzen; ein Biquad schon.
3. **F09** — der Analysepfad hatte vor 4a genau einen Zweig („kopiert Leseblöcke").
   Mit dem Kern kam ein zweiter Zweig und mit ihm ein **dritter, unbehandelter
   Zustand**: Tap ungültig **und** Kern rechnet. Genau dort fällt die Queue-Zuführung
   aus — und damit die gesamte Zeit- und Lückenbuchführung, die nur im Aufruf lebt.
4. **F12** — die Endlichkeitsriegel (R9/N-13) wurden für die `double`-Rechnung
   entworfen; die `double→float`-Verengung am Ausgang ist mit dem Kern neu und an
   drei Stellen dieselbe unbehandelte Kante.

**Regelmuster für die Nacharbeit:** jede Fläche, die Probeeq vor 4a als „reiner
Passthrough" begründet hat, ist verdächtig. Drei davon sind hier belegt; die
vierte Gruppe (Zahlenrand) ist dieselbe Bewegung von `double` nach `float`.
Eine Nacharbeit sollte §2.2 (a) systematisch gegen den heutigen aktiven Kern
durchgehen, statt nur diese vier Punkte zu schließen.

**Zweites, kleineres Muster (F09 und F12):** ein Zähler, der im falschen
Repräsentationsraum bzw. an der falschen Schichtgrenze steht, sieht seinen eigenen
Fall nicht. `zaehlerVerworfen` (DspKern) und `oversizeDrops`/`kFlagLueckeDavor`
(Queue) beschreiben dieselbe verworfene Zeitspanne, sind aber nicht gekoppelt;
`wetNichtEndlich` zählt auf `double`, während der Schaden auf `float` entsteht.

---

## Scope-Beweis

### Gelesene Pfade (Produktcode an HEAD 39e18d3f)

- `eq-copilot/plugin/sonde/SondeProcessor.cpp` — `:133-135` (BusesProperties),
  `:195-245` (Konstruktor-Ende/`prepareToPlay`), `:251-263` (`releaseResources`),
  `:265-277` (`isBusesLayoutSupported`), `:279-377` (`processBlock` vollständig),
  `:379-411` (`nakamaBlockEmpfangen`), `:1214-1225` (`committedRuhtImPassthrough`)
- `eq-copilot/plugin/sonde/SondeProcessor.h` — `:118-166` (Klassenkopf, überschriebene
  Methoden, `getTailLengthSeconds`), `:452` (Analyse-Tap-Puffer)
- `eq-copilot/plugin/dsp/DspKern.cpp` — `:755-830` (`verarbeite`/Stückelung/`verarbeiteStueck`-Kopf),
  `:843-880` (Nicht-Endlich-Riegel, Pfadaufrufe), `:975-1023` (Ausgangsschleife,
  Schreibverzicht, `beendeHoerHalt`), `:115/140/272-274/803/898` (`tapGueltig`)
- `eq-copilot/plugin/dsp/DspKern.h` — `:188-190` (`tap`, `tapLaenge`), `:422` (`tapGueltig`)
- `eq-copilot/plugin/src/HoerMarkierung.h` — `:530-560` (`tdf2Lauf`-Aufrufe),
  `:657-658` und `:690-692` und `:804` (Zählerverdrahtung), `:715-757`
  (Riegelkommentar, `tdf2Lauf`, `resetZustaende`, `hartAus`)
- `eq-copilot/plugin/core/StampedAudioQueue.h` — `:1-75` (Klassenvertrag, Flags),
  Trefferliste zu `Luecke`/`segment`/`stromVon`/`oversize` über die ganze Datei
  (`:165-167`, `:295-297`, `:309-374`, `:392-481`, `:543-552`, `:590-605`, `:621-729`)
- `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp` — `:1-40` (Dateikopf/Zuständigkeit),
  `:149-158` (`EqCopilotProcessor::isBusesLayoutSupported`)
- `eq-copilot/plugin/src/PluginProcessor.cpp` / `.h` — Kopf und Zuständigkeitskarte,
  Suche nach `reset` (kein Treffer außer einem Kommentar `PluginProcessor.h:1336`)
- `eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.h:925-950`
- `eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp`
  — `:2744-2753`, `:2820-2840` (`setActive`), `:3595-3630` (`setupProcessing`,
  `setProcessing`, `getTailSamples`), `:3948-3962` (`preparePlugin`)

### Gelesene Tests

- `eq-copilot/plugin/tests/SondeNullTestMain.cpp` — `:31-52` (Prüfling),
  `:103` (`prepareToPlay`), `:488-530`; vollständige Suche nach
  `checkBusesLayoutSupported`, `BusesLayout`, `reset`, `getTotalNum` (keine Treffer)
- `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:3060-3090` (M-48 / B-22),
  `:2910` (Kommentar Oversize)
- `eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:414-448`, `:1380-1385`,
  `:1819-1820`, `:1900-1935`
- `eq-copilot/plugin/tests/MarkierungTestMain.cpp:431-477` (M-36 Oversize),
  `:564-632` (NAK-180 R4), `:671`, `:700-714`
- `eq-copilot/plugin/tests/HostProbeTestMain.cpp:119-162`,
  `eq-copilot/plugin/tests/AuxSpikeTestMain.cpp:50-150`,
  `eq-copilot/plugin/tests/NullTestMain.cpp:26`, `:71-74`,
  `eq-copilot/plugin/tests/QueueStressTestMain.cpp:92`, `:216`, `:234`, `:280`

### Gelesene Anforderungs- und Auditquellen

- `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md` — §1 bis §8 vollständig
- `docs/audits/2026-09-12-code-review-2/evidence/host/vst3_full.log`,
  `evidence/audio/gap-validated-log.txt`, `evidence/audio/repro-validated-log.txt`,
  Inhaltsverzeichnis von `evidence/` und `pakete/`
- `CLAUDE.md` (Wahrheitskern, Arbeitsregeln, Tragende technische Invarianten)
- `docs/FL-Nakama-Sonden-Design-Entwurf.md` — `:1925-1940` (Zeitachse/Lückenvertrag),
  `:3016-3034` (§44.3), `:3440-3470` (§48.2), `:3640-3650` (§50.1);
  Trefferlisten zu Kanal/Bus/Layout/reset/Suspend über die ganze Datei
- `docs/beweise/SONDE-015.md` — `:700-740` (§2.2 (a)), Matrixzeilen M-01, M-05, M-40
  bis M-57 (`:1034-1116`), M-51 (`:1104`), R9-Abweichung `:1755` und `:2147`,
  Rotbeweistabellen `:3438-3494`, `:3660-3680`
- `docs/beweise/SONDE-014.md` — `:9-10` (Urteil/Grenzen), `:4026` (Runner-Behauptung B5),
  Trefferliste zu Lücke/Zeitachse/Segment
- `docs/beweise/NAK-180.md:308-311` (N-13 bis N-17), `:364`, `:446-447`
- `eq-copilot/identity/host-capabilities-fl-v1.json` — vollständige Trefferliste zu
  Kanal/Bus/Layout (`:17`, `:60-86`, `:125-130`)
- `docs/offene-punkte.md` — Registerzeilen NAK-163 (`:191`), NAK-249 (`:261`),
  NAK-253 (`:265`), NAK-255 (`:267`), NAK-269 (`:281`), NAK-283 (`:295`), NAK-246 (`:440`);
  gezielte Suche nach Vierkanal / Mehrkanal / Buslayout / Reset / Tail / Oversize /
  Verengung / float über alle Registerzeilen
- `.claude/skills/dirigent/SKILL.md:231-276` (Klassenregeln §3.4, Nacharbeit)

**Registerbefund:** zu keinem der vier Punkte existiert heute eine Registerzeile.
F04, F05, F09 und F12 sind neu; NAK-283 ist die Übernahmezeile des Audits.

### Nicht gelesen (bewusst außerhalb dieses Auftrags)

- `broker/` vollständig (F02, F03, F14 gehören zu Validierung A bzw. anderen Paketen)
- `eq-copilot/plugin/core/analysis/` außer der Suche nach der Zeitachse
  (Spektrum.h, Lautheit.h, Frame.h, FeatureEngine.h betreffen F06 bis F10)
- `eq-copilot/plugin/state/NakamaState.cpp` (F11)
- `tools/eq-copilot/pruefe_session_soak.py` (F13)
- `eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp` (N01)
- `eq-copilot/plugin/src/SourcesModel.cpp`, `src/prozessor/Ipc.cpp` (F01)
- `docs/audits/2026-09-12-code-review-2/AUDIT.md`, `ARBEITSPLAN.md`,
  `INTEGRATION.md`, `VERIFIKATION.md` — die Übergabe ist laut §1 eigenständig lesbar
- `docs/audits/2026-09-12-code-review-2/pakete/01-audio-dsp.md` und `06-host-delivery.md`
  sind 9 bzw. 11 Zeilen lang (Paketaufträge, keine Befundinhalte); der Befundinhalt
  steht in der Übergabe §4

### Fehlend / nicht entscheidbar aus der Quelle

1. **F09, FL-Erreichbarkeit.** Ob FL Studio jemals einen Block größer als das
   deklarierte `maxSamplesPerBlock` liefert, steht in keiner gelesenen Quelle.
   Der Capabilityreport belegt nur die Gegenrichtung (Zerteilung bis 1 Sample).
   Ein FL-Termin oder ein Hostprobe-Lauf könnte es messen; ohne ihn bleibt die
   Vertragsverletzung belegt, die Häufigkeit unbekannt.
2. **F05, hörbare Wirkung in FL.** Der Audit misst den Rest am kleinen Testhost,
   nicht in FL. Wie oft FL `setProcessing(false)` ohne `setActive(false)` fährt
   (Transportstop vs. Smart Disable), ist aus dem Repo nicht belegbar.
3. **F12, Verhalten der Verengung.** `(float)` eines nicht darstellbaren `double`
   ist in C++ undefiniert; der beobachtete Wert `inf` ist MSVC-Verhalten. Ob ein
   anderer Compiler/Optimierungsstand denselben Wert liefert, ist nicht geprüft
   und für die Einordnung nicht nötig.
4. **F04, Verhalten anderer Hosts.** Ob ein realer Host tatsächlich ein
   Vierkanallayout gegen ein stereo deklarierendes Plugin setzt, ist aus dem Repo
   nicht belegbar; der Auditharness hat es getan, ein VST3-Validator täte es
   vermutlich auch.

---

# Teil C — F06, F07, F08, F10, N01

# NAK-283 — Quellvalidierung C: F06, F07, F08, F10, N01

**Rolle:** lesender Validierer. HEAD `39e18d3f`; Produktcode identisch mit der
Audit-Basis `ea6ddd57` (die Zwischen-Commits änderten Plan-, Register-, Beweis-
und Auditdokumente). Kein Bau, kein Test, kein Repro, keine Fehlerinjektion,
kein Schreiben im Repo. Die Repro-Logs des Audits sind hier nur Gegenprobe zur
eigenen Quellenlesung, nie Ersatz.

**Einordnungsregeln** (`.claude/skills/dirigent/SKILL.md:236-241`, wörtlich):

> - **Defekt** — verletzt Matrix, Gate, Entwurf oder `CLAUDE.md`-Invariante →
>   Nacharbeit.
> - **Lücke** — Matrix und Entwurf schweigen → Dirigent entscheidet die Regel
>   (Technik) in derselben Runde, trägt sie in Matrix und Manifest ein; erst die
>   Regel darf Nacharbeit auslösen.
> - **Härtung** — von keiner Zusage verlangt → datiert ins Register.

---

## F06 — „Das Spektralgate verwendet eine Leistungsdichte wie eine Energie"

**Audit-Titel §2:** „Aktivitätsgate der Spektralanalyse verwechselt
Leistungsdichte mit Energie" · **Priorität: P2** · §4 Überschrift: „F06 — Das
Spektralgate verwendet eine Leistungsdichte wie eine Energie".

### Quelle an HEAD

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:352-362`:

```cpp
    // Kanalenergie-Mittel: (|L|² + |R|²)/2 = |M|² + |S|².  Ein reiner
    // Mono-Mix loeschte Antiphase komplett aus (Uebergabe-Paket B) — dieselbe
    // Falle, die `AnalyseEngine` im Kopf beschreibt.
    const double norm = 2.0 / (s.fs * s.fensterEnergie);
    const int bins = s.punkte / 2 + 1;
    for (int k = 0; k < bins; ++k)
        s.psd[(std::size_t) k] = (s.fftM.leistung (k) + s.fftS.leistung (k)) * norm;

    const double gesamt = summeBereich (s, 0, bins);
    const bool aktiv = gesamt > 0.0
                    && 10.0 * std::log10 (gesamt) > kAktivGateDb;
```

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:473-479`:

```cpp
inline double FeatureEngine::summeBereich (const Stufe& s, int von, int bis) noexcept
{
    double su = 0.0;
    for (int k = von; k < bis; ++k)
        su += s.psd[(std::size_t) k];
    return su;
}
```

`eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:68-71` — dasselbe
Gate, andere Größe:

```cpp
    const double aktivMittel = zelleAktivEnergie / (double) zellenSamples;
    if (aktivMittel > 0.0 && 10.0 * std::log10 (aktivMittel) > kAktivGateDb)
        ++rahmenAktivZellen;
    ++rahmenZellen;
```

`eq-copilot/plugin/core/analysis/FeatureEngine.h:246-247`, die EINE Konstante:

```cpp
    /** Aktivitaetsgate, wie M1. */
    static constexpr double kAktivGateDb = -60.0;
```

`eq-copilot/plugin/core/analysis/FeatureEngine.h:941-946` (in
`Stufe::vorbereiten`, `fensterEnergie` = Σ w²):

```cpp
            fensterEnergie = 0.0;
            ...
                fensterEnergie += w * w;
```

`eq-copilot/plugin/core/analysis/FeatureEngine.h:233-235`:
`kBassPunkte = 16384`, `kHauptPunkte = 4096`.

### Einheitenrechnung (nachvollzogen, nicht übernommen)

`norm = 2/(fs · Σw²)` ist die Standard-Welch-Normierung auf eine **einseitige
Leistungsdichte**: `psd[k]` trägt Amplitude²/Hz. Die Gesamtleistung (also die
mittlere Sampleenergie) ist `Σ psd[k] · Δf` mit `Δf = fs/N`. `summeBereich`
addiert die Dichten **ohne** `Δf`. Damit gilt

    gesamt = P · N / fs           (P = mittlere Kanalenergie, N = s.punkte)

und die effektive Schwelle ist nicht −60 dB auf P, sondern

    P_schwelle = −60 dB − 10·log10(N/fs)

Für die Hauptstufe (N = 4096):

| fs | 10·log10(N/fs) | effektive RMS-Schwelle | −55 dBFS RMS → `gesamt` |
|---|---|---|---|
| 8 kHz | −2,91 dB | −57,1 dBFS | −57,9 dB → **aktiv** |
| 44,1 kHz | −10,32 dB | −49,7 dBFS | −65,3 dB → inaktiv |
| 48 kHz | −10,69 dB | −49,3 dBFS | −65,7 dB → inaktiv |
| 96 kHz | −13,70 dB | −46,3 dBFS | −68,7 dB → inaktiv |
| 192 kHz | −16,71 dB | −43,3 dBFS | −71,7 dB → inaktiv |

Das deckt sich Ziffer für Ziffer mit dem archivierten Lauf
`evidence/models/models-results.txt:1-5`
(`sr=8000 … coverage=1`, `sr=44100/48000/96000/192000 … coverage=0`, jeweils
`activity=1`). Die Größe ist damit **samplerateabhängig UND stufenabhängig**
(Bassstufe N = 16384 → +6,02 dB andere Schwelle); die Zeitbereichsgröße in
`Lautheit.h:68` ist es nicht. Beide werden gegen dieselbe Konstante gehalten.

**Zusatzwirkung an HEAD:** das `if (! aktiv) return;` steht VOR der
Bandakkumulation (`Spektrum.h:386-388`), also fehlen dem Evidenzfenster bei
leisem Material auch die Bandwerte — im Lauf sichtbar als `stereo_bands=0` bei
96/192 kHz.

### Anforderungsquelle

`eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:706-714`:

```cpp
    /** Abdeckung und Konvergenz des Evidenzfensters, beide in [0, 1].

        `abdeckung` ist der Anteil der Analysefenster, die das Aktivgate
        genommen haben — die tatsaechlich gemessene Signalabdeckung, nicht
        die Wanddauer (§48.2). …  */
    bool  abdeckungGesetzt  { false };  float abdeckung  { 0.0f };
```

Der Vertrag kennt **ein** Aktivgate; `FeatureEngine.h:247` deklariert es einmal
(„Aktivitaetsgate, wie M1"). Eine **Zahl** für die Schwelle steht weder in
`docs/FL-Nakama-Sonden-Design-Entwurf.md` §39/§48.2 noch in einer Matrixzeile:
§48.2 (`:3446-3459`) spricht nur von „Smart Disable/Suspend wird als fehlende
Coverage behandelt". Die Einheit des Gates ist damit nirgends festgeschrieben —
das ist der Lückenanteil dieses Befundes.

Verletzt ist dagegen die CLAUDE.md-Invariante **„Begriffe nicht vermischen"**
(eine Leistungsdichte ist keine Energie; die Aufzählung „Probe-Pipe ist nie
Produktions-Pipe; `ltasReferenzDb` ist keine globale Sollkurve; Paint-FPS ist
nicht Datenkadenz" ist ausdrücklich exemplarisch) sowie die Kohärenzzusage
„Engine … liefert kohärente Mess-Snapshots": derselbe Frame trägt
`aktivitaet = 1` und `abdeckung = 0`.

**Wirkung im Produkt:** `abdeckung` steuert die Konfidenzklasse,
`eq-copilot/plugin/core/analysis/Konfidenz.h:162-170` — bei `abdeckung == 0`
fällt der Beleg auf die unterste Klasse. Ein sauber messbares leises Material
(Ausklang, Intro, ruhige Passage) wird damit als nicht belegbar gemeldet.

### Auslösekette / Erreichbarkeit

Eingabe: beliebiges Material unter rund −49 dBFS RMS bei 48 kHz (bei 96 kHz
schon unter −46 dBFS). Bedingung: keine. Der Pfad ist der normale
Analysepfad jeder Sonde — `verarbeiteSamples` → `schiebeStufe` →
`rechneFenster`. **Im heutigen Produkt uneingeschränkt erreichbar**, ohne
Passage, ohne Sonderzustand.

**Unabhängige Bestätigung im Repo (kein Auditartefakt):**
`docs/beweise/SONDE-013.md:2619-2621` beschreibt denselben Effekt, ohne ihn als
Einheitenfehler zu erkennen: „**Der Boden lag unter dem Aktivgate.** Bei ±0,005
läuft der Detektor gar nicht erst (`if (! aktiv) return;` vor dem Flussschritt)".
±0,005 Amplitude sind rund −52 dBFS RMS — nach der dokumentierten Gate-Absicht
(−60 dB) klar aktiv, nach der gebauten Rechnung bei 48 kHz nicht.

### Status

**BESTÄTIGT.** Die Einheitenrechnung ist an der Quelle nachvollzogen und
reproduziert die fünf archivierten Messzeilen exakt; sie steht ohne das Repro.

### Vorschlag Einordnung

**DEFEKT** — Grund: der Frame trägt zwei einander widersprechende
Aktivitätsaussagen über dasselbe Signal, der Vertrag zu `abdeckung`
(`Vertrag.h:706-714`) beschreibt eine „tatsächlich gemessene Signalabdeckung",
die sich tatsächlich mit Samplerate und FFT-Länge verschiebt, und
CLAUDE.md „Begriffe nicht vermischen" ist verletzt.
**Regelbedarf in derselben Runde:** die Einheit und der Bezug des Gates
(mittlere Sampleenergie vs. integrierte Bandleistung) sind nirgends zugesagt —
der Dirigent muss die Regel setzen, bevor die Nacharbeit sie schließen kann.
Ein Umskalieren der PSD ist kein alleiniger Fix: `s.psd` wird von
Bandakkumulation, Fluss und Stereo mitgelesen.

### Test, der die Zusage tragen müsste

`eq-copilot/plugin/tests/Sonde013EventWireTest.cpp:485-517` ist der einzige
Abdeckungsfall. Er läuft **nur bei 48 kHz** und **nur mit Amplitude 0,25**
(rund −15 dBFS RMS), also weit über jeder Gate-Variante, und vergleicht
Dauerton gegen halbe digitale Stille. Er würde heute **nicht rot**. Kein Test
im Repo hält `aktivitaet` und `abdeckung` desselben Frames gegeneinander, und
kein Test fährt die Abdeckung über mehrere Sampleraten.

---

## F07 — „Reset beim Binden einer zukünftigen Passage kommt zu früh"

**Audit-Titel §2:** „Vorab markierte Passage übernimmt True Peak aus vorherigem
Material" · **Priorität: P2**.

### Quelle an HEAD

`eq-copilot/plugin/core/analysis/FeatureEngine.h:589-625` (Auszug, der Reset
beim Binden):

```cpp
    bool setzePassagenfenster (std::int64_t startSample, std::int64_t endeSample,
                               std::uint64_t transportEpocheDerPassage) noexcept
    {
        if (endeSample <= startSample)
            return false;
        …
        if (transportEpocheDerPassage != transportEpoche)
            return false;
        passagenfenster.gesetzt = true;
        …
        passageTruePeak = 0.0;
        passagenTruePeakRahmen = 0.0;
        zelleImFensterSamples = 0;
        // 🔑 Der Polyphasenfilter wird geleert. Ein Passagenanfang IST eine
        // Fenstergrenze (§32.3): seine 24 Taps je Phase reichen zwoelf Samples
        // vor den Anfang zurueck, und ohne diesen Reset trug der erste
        // Passagenrahmen den Nachklang des Materials DAVOR. …
        (void) tp.nachlauf();
        tp.zuruecksetzen();
```

Der Kommentar zitiert die Zusage („Ein Passagenanfang IST eine Fenstergrenze")
und setzt sie am **Bindezeitpunkt** um, nicht am **Fensteranfang**.

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:190-201` — der
Interpolator wird für JEDES Sample getaktet, auch außerhalb der Passage, und
sein verzögerter Ausgang fällt ungeprüft in das Passagenmaximum:

```cpp
        const double tpJetzt = std::max (tp.tick (l, r), std::max (absL, absR));
        rahmenTruePeak = std::max (rahmenTruePeak, tpJetzt);
        // M-03/M-25: das PASSAGENmaximum zaehlt nur, was im Fenster lag.
        // Ohne Fenster ist `imPassagenfenster` fuer jedes Sample wahr, und
        // die zwei Groessen sind identisch.
        if (imPassagenfenster)
        {
            passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, tpJetzt);
            ++zelleImFensterSamples;
        }
```

`imPassagenfenster` entsteht aus dem Projektzeitausschnitt
(`Spektrum.h:57-87`), nicht aus dem Bindezeitpunkt. Für das Ende gibt es die
Gegenmaßnahme (`Spektrum.h:107-113`, Nachlauf am Index `passBis`); für den
**Anfang** gibt es keine.

### Anforderungsquelle

`docs/beweise/SONDE-013.md:3794` (Verhaltensmatrix, Befund B08):

> | B08 | DEFEKT M-03/M-25 | Die `FeatureEngine` kennt das Fenster der
> markierten Passage (`setzePassagenfenster`); Passagenmaximum, Headroomring,
> LRA und Fingerprint hängen daran statt an der letzten Transportgrenze. …

`docs/beweise/SONDE-013.md:505` (M-25) und `:472` (M-03) binden die
Passagenmetriken an das markierte Fenster; Entwurf §39.2:2610-2612 („Headroom
wird in dBTP und als Verteilung über die Passage dargestellt").
Dazu die Selbstzusage im Quelltext (`FeatureEngine.h:604-612`, oben zitiert).

### Auslösekette / Erreichbarkeit

`eq-copilot/plugin/src/prozessor/State.cpp:359-395` (`merkeManuellePassage`)
prüft nur:

```cpp
    if (! nakama::ipc::istHex32 (passageId.toStdString()) || label.length() > 120)
        return false;
    if (projektStart < 0 || projektEnde <= projektStart)
        return false;
```

**Keine Prüfung gegen den Abspielkopf.** Ein Fenster, dessen `projektStart`
noch in der Zukunft liegt, wird angenommen, über
`bindePassagenfenster` als Wunsch hinterlegt und vom Analyseworker unter seiner
Steuersperre eingelöst (`eq-copilot/plugin/src/prozessor/Analyse.cpp:180-193`).
Der Epochenriegel greift nicht, solange kein Seek dazwischenliegt.

Kette: User markiert bei laufendem Transport eine Passage **vor** dem
Abspielkopf (oder bindet vor dem Start) → `tp.zuruecksetzen()` läuft sofort →
das Material bis zum Fensteranfang füllt die 24-Tap-Kette je Phase wieder →
beim ersten Sample im Fenster trägt `tpJetzt` die verzögerte Antwort der
letzten zwölf Samples DAVOR → `passagenTruePeakRahmen` übernimmt sie.
Bei einer lauten/leisen Kante am Fensteranfang ist der Fehler so groß wie die
Kante. Archivierter Lauf: `evidence/models/models-results.txt:7-8` —
`FUTURE_PASSAGE … TP=-59,84 passageTP=+0,157` gegen
`ISOLATED_REFERENCE … passageTP=-59,84`.

**Erreichbar im heutigen Produkt: ja**, über den einzigen Produktaufrufer
`merkeManuellePassage` → `bindePassagenfenster` → Analyseworker.
Der Audit hat diese Kette nicht durchgefahren (Engine-Direktaufruf); die
Kette ist hier an der Quelle gelesen und enthält keinen Riegel, der den Fall
ausschließt.

**Nebenbefund an derselben Stelle:** der Reset verwirft den Nachlauf des
laufenden Materials (`(void) tp.nachlauf();`). Bei einem Fenster in der Zukunft
wird damit eine echte Intersample-Spitze der LIVE-Metrik `rahmenTruePeak`
unterschlagen. Gehört in denselben Fix, nicht in einen zweiten.

### Status

**BESTÄTIGT.**

### Vorschlag Einordnung

**DEFEKT** — verletzt M-03/M-25 in der Fassung des Befundes B08 und die
ausdrückliche Selbstzusage im Quellkommentar („Ein Passagenanfang IST eine
Fenstergrenze"). Die Startgrenze existiert nur für den Fall
„Bindezeitpunkt = Fensteranfang".

### Test, der die Zusage tragen müsste

`eq-copilot/plugin/tests/Sonde013EventWireTest.cpp:1004-1035` (B08) — bindet
mit `const std::int64_t start = s.projekt;`, also **genau am Abspielkopf**;
`:1084` (R04) ebenso. `eq-copilot/plugin/tests/Sonde013PassageStateTest.cpp:632ff`
prüft Epoche und ID, nicht die Startgrenze. Kein Fall bindet ein Fenster, das
erst später beginnt. Keiner dieser Tests würde heute **rot**.

---

## F08 — „Headroom-P95 verliert den Anfang einer längeren Passage"

**Audit-Titel §2:** „Headroom-Verteilung beschreibt nur die letzten 64 Rahmen
statt die ganze Passage" · **Priorität: P2**.

### Quelle an HEAD

`eq-copilot/plugin/core/analysis/FeatureEngine.h:107-118`:

```cpp
/** Wie viele Analysefenster hoechstens in P10/P50/P95 eines Bandes eingehen.

    SONDE-013 M-05. Der Wert ist eine RESSOURCENGRENZE, keine Messaussage:
    er deckelt den festen Speicher (221 Baender x 64 float = rund 57 KiB je
    Instanz) und damit den Sortieraufwand am Rahmenende. …  */
inline constexpr int kVerteilungPlaetze = 64;
```

`eq-copilot/plugin/core/analysis/FeatureEngine.h:217-222`:

```cpp
    void schiebe (float db) noexcept
    {
        werte[(std::size_t) stand] = db;
        stand = (stand + 1) % kVerteilungPlaetze;
        if (gefuellt < kVerteilungPlaetze) ++gefuellt;
    }
```

`eq-copilot/plugin/core/analysis/featureengine/Frame.h:99-109` — genau ein Wert
je publiziertem Rahmen (Live-Kadenz 10 Hz, M-01):

```cpp
    // SONDE-013 M-02/M-03: das Passagenmaximum und die Headroomverteilung
    // wachsen um GENAU DIESEN Rahmen, bevor die Skalare sie lesen …
    if (passagenTruePeakRahmen > 0.0 && ! headroomRing.empty())
    {
        passageTruePeak = std::max (passageTruePeak, passagenTruePeakRahmen);
        headroomRing[0].schiebe ((float) (20.0 * std::log10 (passagenTruePeakRahmen)));
    }
```

`eq-copilot/plugin/core/analysis/featureengine/Frame.h:636-645`:

```cpp
    if (! headroomRing.empty() && headroomRing[0].gefuellt >= 4)
    {
        float sortiert[kVerteilungPlaetze];
        const int n = ringInZeitfolge (headroomRing[0], sortiert);
        std::sort (sortiert, sortiert + n);
        f.headroomGesetzt = true;
        f.headroomP10Db = (float) perzentil (sortiert, n, 0.10);
        …
        f.headroomFenster = (std::uint32_t) n;
    }
```

64 Plätze × 100 ms = **6,4 s**. Der Ring wird nur bei `zuruecksetzen`
(`FeatureEngine.h:428`), `setzePassagenfenster` (`:623`),
`loeschePassagenfenster` (`:662`) und `grenzeZiehen` (`Zeit.h:468`) geleert —
**nicht** in `evidenzLeeren` (`Frame.h:122-140`).

### Anforderungsquelle

Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md` §39.2 (Zeile 2620), wörtlich:

> Headroom wird in dBTP und als Verteilung über die Passage dargestellt. Ein
> Peak darf nicht als Problem gelten, nur weil er hoch ist.

Matrix `docs/beweise/SONDE-013.md:472` (M-03), Zusagespalte wörtlich:
„Headroom reist in dBTP und als Verteilung über die Passage, nicht als
Einzelspitze."; Anforderungsspalte „Entwurf §39.1:2583-2586; §39.2:2610-2612.
**BELEGT, gemessen** (Etappe C, 2026-09-04)".

Der Vertragskopf sagt an derselben Stelle etwas Drittes,
`eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:670-682`:

```cpp
    /** Headroom als VERTEILUNG, nicht als Einzelspitze (§39.2 woertlich:
        "Headroom wird in dBTP und als Verteilung ueber die Passage
        dargestellt. …")

        Die drei Punkte sind P10/P50/P95 der RAHMEN-True-Peaks im laufenden
        Evidenzfenster; `headroomFenster` sagt, ueber wie viele Rahmen sie
        gehen …  */
```

Damit stehen drei Bedeutungen nebeneinander — „über die Passage" (Entwurf und
Matrix), „im laufenden Evidenzfenster" (Vertragskopf) und faktisch „die letzten
höchstens 64 Rahmen" (Code, der weder bei jedem Evidenzfenster noch je Passage
begrenzt wird). Der Befund des Audits, dass diese drei nicht gleichgesetzt
werden dürfen, ist an der Quelle belegt.

### Auslösekette / Erreichbarkeit

Eingabe: eine markierte Passage **länger als 6,4 s** mit ungleich verteiltem
Pegel. Bedingung: keine weitere. Jede musikalisch übliche Passage (ein Refrain,
ein Achttakter) überschreitet 6,4 s. Archivierter Lauf
`evidence/models/models-extra-results.txt:1`:
`HEADROOM t=14 window_set=1 kept_frames=64 p10=p50=p95=-66,02 expected_p95=-6,02
passage_max=-6,02`. Das Passagenmaximum bleibt korrekt; nur die Verteilung ist
auf das Ende beschnitten.
**Erreichbar im heutigen Produkt: ja** — sobald ein Fenster gesetzt ist, und
auch ohne Fenster für die Strecke seit der letzten Transportgrenze.

### Status

**BESTÄTIGT.**

### Vorschlag Einordnung

**DEFEKT** — verletzt Entwurf §39.2:2620 und die Matrixzeile M-03 wörtlich
(„Verteilung über die Passage"), deren Anforderungsspalte auf genau diese
Entwurfszeilen zeigt und mit **BELEGT, gemessen** geschlossen ist.
Anmerkung für die Nacharbeit: `headroomFenster` ist bereits ehrlich (es meldet
64), eine Kommentarkorrektur allein macht aus einer Passagenverteilung kein
zugesagtes gleitendes Fenster; der kleinste kohärente Fix muss Entwurf, Matrix,
Vertragskopf, Writer/Reader und Anzeige gemeinsam tragen.

### Test, der die Zusage tragen müsste

`eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:413-452`
(`M-03 headroom_is_distribution_not_single_peak`). Er setzt **kein**
Passagenfenster und fährt 5,0 s beziehungsweise 3,0 + 0,3 + 1,7 s — beides
unter 6,4 s, der Ring läuft also nie über. `headroomFenster` wird im
Freitext ausgegeben, aber nie geprüft. Er würde heute **nicht rot**.

---

## F10 — „Gültigkeitsdauer entspricht nicht der Dauer des Loudnessfensters"

**Audit-Titel §2:** „Loudness wird wieder als gültig ausgegeben, obwohl ihr
Zeitfenster ersetzte Samples enthält" · **Priorität: P2**.

### Quelle an HEAD

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:133-141` — Ersetzung
und Zählung:

```cpp
        if (! std::isfinite (l) || ! std::isfinite (r))
        {
            if (! std::isfinite (l)) l = 0.0;
            if (! std::isfinite (r)) r = 0.0;
            if (rahmenNichtEndlich < 0xFFFFFFFFu) ++rahmenNichtEndlich;
            if (evidenzNichtEndlich < 0xFFFFFFFFu) ++evidenzNichtEndlich;
            ++nichtEndlicheSamplesGesamt;
        }
```

`eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:30-40` — die Zelle
wird **ohne Gültigkeitsmerkmal** abgelegt:

```cpp
inline void FeatureEngine::zelleSchliessen() noexcept
{
    const double mittel = zelleKEnergie / (double) zellenSamples;
    kurzZellen[(std::size_t) kurzStand] = mittel;
    …
    kurzStand = (kurzStand + 1) % kKurzZellen;
    if (kurzGefuellt < kKurzZellen) ++kurzGefuellt;
```

`eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:90-102` — gemittelt
werden alle 30 Zellen, also 3 s zurück:

```cpp
inline bool FeatureEngine::kurzLufs (double& heraus) const noexcept
{
    if (kurzGefuellt < kKurzZellen)
        return false;
    double su = 0.0;
    for (int i = 0; i < kKurzZellen; ++i)
        su += kurzZellen[(std::size_t) i];
    const double mittel = su / (double) kKurzZellen;
```

`eq-copilot/plugin/core/analysis/featureengine/Frame.h:532-546` — die
Verriegelung greift nur auf den **aktuellen Rahmen**:

```cpp
inline void FeatureEngine::fuelleSkalare (FeatureFrame& f) const noexcept
{
    // 🔑 SONDE-013 M-07, DIE VERRIEGELUNG. Sie steht vor allem anderen.
    //
    // Ein nicht-endliches Eingangssample wurde durch Stille ersetzt. Jede
    // Zahl, die dieser Rahmen daraus rechnet, ist damit eine Aussage ueber
    // Audio, das die Engine nie so gesehen hat — endlich, plausibel und
    // falsch. CLAUDE.md verlangt genau hier "verriegelt UND gezaehlt" …
    f.nichtEndlichRahmen  = rahmenNichtEndlich;
    f.nichtEndlichEvidenz = evidenzNichtEndlich;
    if (rahmenNichtEndlich > 0)
        return;
```

`rahmenNichtEndlich` wird in `rahmenLeeren` (`Frame.h:142-163`) je Rahmen
genullt, `evidenzNichtEndlich` in `evidenzLeeren` (`Frame.h:122-135`) je
Evidenzfenster. Beide Lebensdauern sind kürzer als die 3 s der `kurzZellen`
(und die 400 ms von `momentanLufs`, `Lautheit.h:105ff`).

### Anforderungsquelle

`docs/beweise/SONDE-013.md:476` (M-07), Zusagespalte wörtlich:

> Nichtendliches wird beim **Erzeugen** in Wert 0 mit `gueltig=false` übersetzt
> und gezählt, nie sanitisiert auf die Leitung gelassen. … Bei den
> Rahmenskalaren fehlt statt dessen das Präsenzbit. Der Zähler wird
> ausgewertet, nicht ignoriert.

Anforderungsspalte derselben Zeile: „`.fbs`:208-215; `quantisierung-v1.json`;
`LoudnessAccumulator.h`:383; CLAUDE.md NaN-Ehrlichkeit. **BELEGT, gemessen**".
CLAUDE.md-Invariante: „**NaN-Ehrlichkeit.** Nicht-endliche Werte werden
verriegelt und gezählt". Dazu der eigene Quellkommentar `Frame.h:536-540`
(oben zitiert), der die Regel ausdrücklich an „Audio, das die Engine nie so
gesehen hat" bindet — nicht an den Rahmen.

### Auslösekette / Erreichbarkeit

Eingabe: ein nicht endliches Sample im Zulauf (defekte Quelle, denormale
Explosion, fremdes Plugin davor). Nach dem letzten beschädigten Rahmen trägt
bereits der **erste** saubere Rahmen wieder `lufsMGesetzt`/`lufsSGesetzt`,
obwohl 400 ms bzw. 3 s seiner Zellen aus ersetztem Material stammen.
Archivierte Läufe: `evidence/models/models-results.txt:11-12`
(`NAN_ONE_CLEAN_FRAME … lufsS_set=1 S=-14,3137 … nonfinite_frame=0`) gegen
`:9` (`NAN_BEFORE … S=-14,0141`) und
`evidence/models/models-extra-results.txt:2`
(`NAN_4p1_SECOND … S=-14,3137 … nonfinite_frame=0 nonfinite_evidence=0`) —
nach 4,1 s sind beide Zähler null und der Wert weiter kontaminiert.
**Erreichbar im heutigen Produkt: ja**, derselbe Erzeugerpfad wie im Betrieb.

Bewertungshinweis: Die 0,2-s-Schadensphase des Audits ist eine
Eingabefehlerinjektion; sie ist am 12.09.2026 gestoppt und wird hier nicht
wiederholt. Die Aussage hängt aber nicht am Repro — die Lebensdauern der zwei
Zähler und die Fensterlänge von `kurzLufs` sind unmittelbar aus der Quelle
ablesbar.

### Status

**BESTÄTIGT** (Quellenlesung; die Zahlenwerte stammen aus dem archivierten
Lauf und sind nicht nachgemessen worden).

### Vorschlag Einordnung

**DEFEKT** — verletzt M-07 („nie sanitisiert auf die Leitung gelassen") und die
CLAUDE.md-Invariante NaN-Ehrlichkeit: der Wert ist endlich und plausibel, seine
Herkunft ist stillgelegtes Audio, und er reist mit gesetztem Präsenzbit.
**Regelbedarf:** M-07 legt die **Reichweite** der Verriegelung nicht fest; der
Quellkommentar und der bestehende Test verstehen sie ausdrücklich als
Rahmenlatch („kein Dauerschweigen: der naechste saubere Rahmen traegt wieder
alles", `Sonde013DynamicsTest.cpp:595-596`). Der Dirigent muss die Regel
„Gültigkeit hängt an der tatsächlich verwendeten Historie, je Fenster getrennt
(400 ms / 3 s / Passage)" setzen; erst dann ist der kleinste kohärente Fix
beschreibbar.

### Test, der die Zusage tragen müsste

`eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:553-613`
(`M-07 non_finite_never_reaches_wire_for_new_metrics`). Er vergiftet **dauernd**
(jedes 997./991. Sample über 8 s), so dass jeder Rahmen den Latch zieht, und
prüft danach nur den Gegenfall mit komplett sauberem Material. Die
Erholungsstrecke — erste saubere Rahmen nach Ende der Vergiftung — wird nie
gefahren. Der Test würde heute **nicht rot**; seine Zeilen 590-596 beschreiben
den Rahmenlatch sogar als gewollt.

---

## N01 — „Der EBU-True-Peak-Testumfang ist kleiner als die dokumentierte Zusage"

**Audit-Titel §2:** „Als EBU-Testset ausgewiesener True-Peak-Nachweis umfasst
nur Fälle 15–19" · **Priorität: „Nachweislücke"** (keine P-Stufe vergeben).

### Quelle an HEAD

`eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp:121-136`:

```cpp
    // ── 1. Die fuenf True-Peak-Faelle aus EBU Tech 3341 ───────────────────
    …
    abschnitt ("true_peak_matches_ebu_test_set - EBU Tech 3341, Testfaelle 15 bis 19 (Toleranz aus §49.3: +/-0,1 dB)");
    {
        const struct { const char* name; double amplitude; int teiler; double phase; } faelle[] = {
            { "15  fs/4, 0,50 FFS, 0 Grad",    0.50, 4, 0.0 },
            { "16  fs/4, 0,50 FFS, 45 Grad",   0.50, 4, 45.0 },
            { "17  fs/6, 0,50 FFS, 60 Grad",   0.50, 6, 60.0 },
            { "18  fs/8, 0,50 FFS, 67,5 Grad", 0.50, 8, 67.5 },
            { "19  fs/4, 1,41 FFS, 45 Grad",   1.41, 4, 45.0 },
        };
```

Fünf Fälle, alle stationäre Sinus, gemessen gegen `20·log10(Amplitude)`.
Die Fälle 20–23 kommen im Repo nirgends vor (`grep` über
`eq-copilot/plugin/tests/`, `eq-copilot/schemas/`, `tools/eq-copilot/`).

### Wer „EBU-Testset" behauptet — und wie eng es wirklich ist

| Ort | Behauptung | tatsächlicher Umfang |
|---|---|---|
| `eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp:127` | Fallname `true_peak_matches_ebu_test_set` | 15–19 (im Fallnamen daneben ehrlich benannt) |
| `eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp:5-10` | zitiert M-02 „das offizielle EBU-Testset bei 48 kHz" als erfüllte Bedingung | 15–19 |
| `tools/eq-copilot/erzeuge_p4_korpus.py:92-96` | Korpusfall `true_peak_matches_ebu_test_set`, `hinweis`: „EBU Tech 3341 Faelle 15-19" | ehrlich im `hinweis`, breit im Fallnamen |
| `docs/beweise/SONDE-013.md:471` (M-02) | Statusspalte „**BELEGT, gemessen** … der 8-fach-Pfad besteht die Prüfung und bleibt" | Nachweisspalte nennt „die fünf True-Peak-Fälle 15 bis 19" |
| `docs/beweise/SONDE-013.md:623` (M-81) | „**BELEGT, gemessen**" | dieselben fünf Fälle |
| `docs/beweise/SONDE-014.md:4043` u. a. (B17-Zeilen) | „die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19)" | ehrlich |
| `docs/offene-punkte.md:221` (NAK-199) | „für True Peak ist es gedeckt (B17, EBU Tech 3341 Fälle 15–19)" | schließt den True-Peak-Teil ausdrücklich als gedeckt |

### Anforderungsquelle — Matrix

`docs/beweise/SONDE-013.md:471` (M-02), wörtlich:

> True Peak ist BS.1770-konform. Der vorhandene 8-fach-Pfad bleibt **nur**,
> wenn er das offizielle EBU-Testset bei 48 kHz und generierte
> Mehrsampleraten-Goldens besteht; sonst wird der BS.1770-Polyphase-
> Referenzpfad verwendet. Toleranz ±0,1 dB (§49.3).

`docs/beweise/SONDE-013.md:623` (M-81), wörtlich:

> Der 8-fach-True-Peak-Pfad besteht das offizielle EBU-Testset bei 48 kHz
> **plus** äquivalente generierte Mehrsampleraten-Goldens — oder er wird durch
> den BS.1770-Polyphase-Referenzpfad ersetzt (M-02).

Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md`, §39.1 (Zeilen 2598-2601):

> Der vorhandene 8×-True-Peak-Pfad bleibt nur, wenn er das vollständige
> offizielle EBU-Testset bei 48 kHz sowie äquivalente generierte
> Mehrsampleraten-Goldens besteht; sonst wird der BS.1770-Polyphase-
> Referenzpfad verwendet.

### Anforderungsquelle — Primärquelle (am 12.09.2026 erneut gelesen)

`wissen/INDEX.md` und `wissen/dsp/2026-08-27-messwerte-brauchen-support-und-zeitanker.md:99`
führen Tech 3341 nur als URL, ohne Fallliste. Deshalb die Norm selbst geprüft:
**EBU Tech 3341, Fassung 4.0 (2023)**,
`https://tech.ebu.ch/files/live/sites/tech/files/shared/tech/tech3341v4_0.pdf`.

§2.6 „True peak measurement" (Seite 6), wörtlich:

> In addition to the specifications of ITU-R BS.1770 Annex 2, the true-peak
> measure implemented in an 'EBU Mode' meter shall comply with the tolerances
> specified in Table 1 (signals 15-23), intended as 'minimum requirements'
> tests. The total measurement error of the true-peak level must comply with
> these tests, including any pass-band ripple in the upsampling filter and the
> 'under-read' described in ITU-R BS.1770, Appendix 1 to Annex 2.

Table 1 „Minimum requirements test signals" (Seite 9) führt 15–19 als
stationäre Sinus (fs/4, fs/6, fs/8; 0,50 und 1,41 FFS; Erwartung −6,0 bzw.
+3,0 dBTP, Toleranz +0,2/−0,4) und zusätzlich:

> **20** — Stereo sine wave with frequency *fs/6* Hz, amplitude 0.50 FFS,
> containing a single period of a sine wave with frequency *fs/4*, amplitude
> 1.00; the signal being continuous in phase at both sides of the single
> period. The signal is synthesized at *4\*fs* (for example, 192 kHz), and then
> lowpass (anti-aliasing) filtered and downsampled to *fs* with a *0* samples
> offset. … Max. true-peak level = 0.0 +0.2/−0.4 dBTP
> **21 / 22 / 23** — As #20, but downsampled with a 1 / 2 / 3 samples offset
> (at the *4\*fs* rate). Max. true-peak level = 0.0 +0.2/−0.4 dBTP

Der Audit-Verweis auf §2.6 und die Fallspanne 15–23 sind damit an der
Primärquelle **bestätigt**. Die vier fehlenden Fälle sind nicht „mehr vom
selben": sie sind transient, an vier Abtastversätzen, und §2.6 nennt
ausdrücklich „pass-band ripple in the upsampling filter" und „under-read" als
das, was sie prüfen sollen. Die schärfere Nakama-Toleranz ±0,1 dB an
stationären Sinus ersetzt diese Signalform nicht.

### Auslösekette / Erreichbarkeit

Keine Laufzeitkette — es handelt sich um eine Nachweis- und Beschriftungslücke.
**Nicht belegt** ist ein numerisches Versagen des 8×-Pfades an 20–23; diese
Fälle wurden weder vom Audit noch hier gerechnet.

### Status

**BESTÄTIGT** — und zwar in beiden Richtungen: der gebaute Umfang (15–19) ist
an der Testquelle belegt, die geforderte Spanne (15–23) an der Primärquelle.

### Vorschlag Einordnung

**DEFEKT** (Nachweis, nicht DSP-Kern) — M-02 und M-81 machen den Verbleib des
8×-Pfades ausdrücklich von „dem offiziellen EBU-Testset" abhängig; diese
Bedingung ist nicht nachgewiesen, die Matrixzeilen tragen dennoch
**BELEGT, gemessen**, und `docs/offene-punkte.md:221` schließt den
True-Peak-Teil als gedeckt. Damit ist eine Zusage verletzt, nicht bloß eine
schweigende Stelle — die Lücken-Klasse („Matrix und Entwurf schweigen") trifft
hier nicht zu.
Kleinster kohärenter Fix (Wahl beim Dirigenten, beides schließt den Punkt):
entweder die vier transienten Fälle 20–23 im Golden ergänzen und die
Statusspalten danach setzen, oder Zusage, Fallname, Korpus-`hinweis` und
NAK-199 auf den wirklich gemessenen Umfang zurückschreiben. Ein Austausch des
DSP-Kerns folgt daraus nicht.

### Test, der die Zusage tragen müsste

`eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp:127-147` ist genau
dieser Test. Er ist heute grün und **würde nicht rot**, weil die vier fehlenden
Fälle nicht existieren — die Lücke ist unsichtbar, solange der Fallname
`true_peak_matches_ebu_test_set` die Spanne nicht nennt.

---

## Gemeinsame Ursache

### Lebensdauer von Analysehistorien

F07, F08 und F10 sind dieselbe Klasse Fehler an drei Puffern: **eine Messung
trägt die Beschriftung eines Zeitintervalls, ihre Rechnung trägt aber die Länge
eines festen Puffers.**

| Größe | Beschriftung | tatsächliche Historie | Puffer |
|---|---|---|---|
| `truePeakPassageDb` | „Maximum der markierten Passage" (M-03/M-25) | Passage **plus** 12 Samples davor, wenn vorab gebunden | `TruePeakDetektor`, 24 Taps je Phase |
| `headroomP10/50/95` | „Verteilung über die Passage" (Entwurf §39.2) | die letzten ≤ 6,4 s | `VerteilungsRing`, 64 Plätze |
| `lufsS` / `lufsM` | „gemessen und sauber" (Präsenzbit, M-07) | 3 s bzw. 400 ms zurück, ersetztes Material eingeschlossen | `kurzZellen`, 30 Zellen |

In allen drei Fällen existiert der Riegel bereits, er hängt nur am falschen
Ereignis: am **Bindezeitpunkt** statt am Fensteranfang (F07), an der
**Ringkapazität** statt an der Passagenlänge (F08), am **Rahmen** statt am
jeweiligen Metrikfenster (F10). Ein gemeinsamer Fix ist deshalb möglich und
billiger als drei: jede Historie bekommt eine Gültigkeitsmarke, die dieselbe
Spanne beschreibt wie der Wert, den sie trägt, und jede Fenstergrenze wirkt auf
alle Historien, die sie überschreiten.

### Passagenanfang und Passagenende

Das Ende ist bereits zweimal nachgearbeitet (B09 Nachlauf am Fensterende,
R05 Nachlauf am Index `passBis`, `Spektrum.h:107-113`); der Anfang ist es nur
für den Sonderfall „Bindezeitpunkt = Fensteranfang" (`FeatureEngine.h:604-625`).
Die Symmetrie „öffnen ↔ schließen" aus CLAUDE.md ist an dieser Stelle
unvollständig: `setzePassagenfenster` macht **beim Binden** sauber, obwohl der
Anfang **beim Erreichen des Startsamples** liegt. `Spektrum.h:57-87` kennt den
Startindex `passVon` bereits und wertet ihn nur für die Zugehörigkeit aus,
nicht als Filtergrenze. F08 hängt an derselben Naht: der Headroomring wird beim
Binden geleert und danach nie mehr an der Passagenlänge gemessen.

F06 gehört nicht in diese Familie — es ist ein Einheitenfehler —, teilt mit ihr
aber die Ursache auf der Ebene darüber: **ein Bezeichner wird auf eine Größe
angewandt, für die er nicht definiert wurde** („Aktivgate" auf eine
Leistungsdichte; „Verteilung über die Passage" auf einen Ringinhalt;
„gemessen und sauber" auf eine Historie mit ersetzten Samples).

---

## Scope-Beweis

### Gelesen (Produktquellen, HEAD 39e18d3f)

- `eq-copilot/plugin/core/analysis/FeatureEngine.h` — Konstanten 107-118,
  200-247, `Stufe::vorbereiten` 918-946, `setzePassagenfenster` 589-625,
  `loeschePassagenfenster`-Umfeld 662, `zuruecksetzen` 420-430,
  Felder 1200-1290
- `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h` — 30-260
  (Blockschleife, Fensterausschnitt, NaN-Riegel, True Peak, Nachlauf),
  341-400 (`rechneFenster`, Gate, Bandakku), 473-479 (`summeBereich`)
- `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h` — 1-120
  (`zelleSchliessen`, `kurzLufs`, `momentanLufs`, Aktivzellen-Gate)
- `eq-copilot/plugin/core/analysis/featureengine/Frame.h` — 95-165
  (Headroom-Einschub, `evidenzLeeren`, `rahmenLeeren`), 520-650
  (`fuelleSkalare`, Verriegelung, Headroom-Perzentile)
- `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h` — 570-720
  (Frame-Felder, M-07-Zähler, Loudness-/Headroomgruppe, Abdeckung)
- `eq-copilot/plugin/core/analysis/featureengine/Zeit.h` — 460-520
  (`grenzeZiehen`, Epochen-/Segmentzählung)
- `eq-copilot/plugin/core/analysis/Konfidenz.h` — 85-170 (Abdeckung →
  Konfidenzklasse)
- `eq-copilot/plugin/src/prozessor/State.cpp` — 359-412
  (`merkeManuellePassage`, `vergissManuellePassage`)
- `eq-copilot/plugin/src/prozessor/Analyse.cpp` — 145-205 (Wunscheinlösung im
  Analyseworker)
- `eq-copilot/plugin/src/PluginProcessor.h` — 1385-1430 (Wunsch-Atomics,
  Generation)
- Tests: `Sonde013TruePeakGoldenTest.cpp` 1-175,
  `Sonde013DynamicsTest.cpp` 405-470 und 553-613,
  `Sonde013EventWireTest.cpp` 485-520 und 1000-1100
- Werkzeug: `tools/eq-copilot/erzeuge_p4_korpus.py` 85-120

### Gelesen (Anforderungs- und Belegquellen)

- `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md` §1-§8 vollständig
- `docs/audits/2026-09-12-code-review-2/pakete/05-analysis-models.md`
- `docs/audits/2026-09-12-code-review-2/evidence/models/models-results.txt`,
  `models-extra-results.txt` (nur gelesen, nicht ausgeführt)
- `docs/FL-Nakama-Sonden-Design-Entwurf.md` §39.1/§39.2 (2578-2620), §48.2
  (3446-3459)
- `docs/beweise/SONDE-013.md` — M-01 (470), M-02 (471), M-05 (474), M-07 (476),
  M-25 (505), M-81 (623), B08 (3794), B18 (4502), §-Abschnitte 2118-2140,
  2610-2630, 4414
- `docs/beweise/SONDE-014.md` — B17-Zeilen (4043 ff.)
- `docs/offene-punkte.md` — NAK-199 (221), NAK-249 (261), NAK-255 (267)
- `wissen/INDEX.md`, `wissen/dsp/2026-08-27-messwerte-brauchen-support-und-zeitanker.md`
- `.claude/skills/dirigent/SKILL.md` §3.4 (230-250)
- **Primärquelle:** EBU Tech 3341 v4.0 (2023), §2.6 und Table 1, Seiten 6-9

### Nicht gelesen (bewusst außerhalb dieses Auftrags)

- F01–F05, F09, F11–F14 (andere Validierungslose)
- `broker/` vollständig; `eq-copilot/plugin/dsp/`, `sonde/`, `state/`
- Die Harness-Quellen des Audits (`evidence/models/models.cpp`,
  `models_extra.cpp`) — nur als Existenzbeleg zur Kenntnis genommen, ihre
  Logik ist für die Quellenvalidierung ohne Beweiskraft
- `AnalyseEngine.cpp`/`Diagnose.cpp` über die Abdeckungs-Konsumenten hinaus

### Fehlt / nicht entscheidbar ohne Lauf

- **F06:** kein Dokument legt die **Einheit** oder den **Zahlenwert** des
  Aktivgates fest. Der Fix braucht eine Dirigentenregel, nicht nur eine
  Codekorrektur. Ob die Bassstufe (N = 16384) dieselbe Regel bekommen soll,
  ist ebenfalls offen.
- **F07:** die vollständige Kette UI → `merkeManuellePassage` →
  Analyseworker → Engine ist an der Quelle gelesen und enthält keinen Riegel
  gegen ein Zukunftsfenster; ein ausgeführter Produktbeleg fehlt (wäre ein
  Lauf, deshalb hier nicht erbracht).
- **F10:** die Zahlenwerte stammen aus dem archivierten Lauf. Die
  Schlussfolgerung hängt nicht daran, aber eine eigene Messung liegt nicht vor.
- **N01:** ob der 8×-Pfad die Fälle 20–23 numerisch bestehen würde, ist
  ungeprüft — hier wurde nur der Umfang der Zusage gegen den Umfang des
  Nachweises gestellt.
- Für alle fünf: kein frischer Kanon, kein Golden-Lauf, kein Manifest dieser
  Session. „Nicht gelaufen" ist nicht „bestanden".

---
