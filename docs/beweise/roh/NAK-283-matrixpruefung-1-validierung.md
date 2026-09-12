# NAK-283 — Validierung der Codex-Matrixprüfung 1 (lesend, 12.09.2026)

HEAD `5e68baf2954902be1a9da221f9523f2697bb00b1`; Produktcode identisch mit
`ea6ddd57`. Nur gelesen, nichts gebaut, nichts ausgeführt, Git nur lesend.

**Zeilenversatz.** Das Urteil zitiert `docs/beweise/NAK-283.md` zum Stand
`23578268`; HEAD fügt in der Kopftabelle eine Zeile („Gate") ein. Alle
Zeilenangaben des Urteils liegen an HEAD um **+3** höher (Beispiel: Urteil
`:589` = M-20 = HEAD-Zeile 592; Urteil `:575` = M-11 = HEAD-Zeile 578).
Unten steht jeweils die HEAD-Zeile. Kein Befund gegen Codex — der Versatz
stammt aus dem Messeintrag des Dirigenten.

---

## Befund 1 — „Ziehe die Frischemarke auch bei inhaltsgleichen Publikationen nach"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:794-795 — DEFEKT, M-05/§6.3: Die
Gleichheitsausnahme lässt einen Rücksprung offen: Modell zunächst leer; ein
bestätigter Join erzeugt eine angehaltene Kopie `[A]`; ein später bestätigter
Unbind publiziert wieder `[]`. Wird wegen Inhaltsgleichheit die zuletzt
übernommene Folgenummer nicht fortgeschrieben, kann anschließend `[A]`
angenommen werden. `SourcesModel.cpp:370–371` vergleicht gerade mit dem
sichtbaren Modellbestand, nicht mit dem zwischenzeitlich geänderten
Prozessorstate.`

**Quelle an HEAD — `eq-copilot/plugin/src/SourcesModel.cpp:360-372`:**

```cpp
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

Der frühe `return true` liegt **vor** jeder Zustandsfortschreibung des
Modells. Eine Folgenummer, die erst hinter dieser Zeile übernommen würde,
bleibt beim inhaltsgleichen Stand stehen.

**Gegenstand — Bauplan §6.3, HEAD-Zeilen 794-798:**

> „`setzePersistenteMitglieder` vergleicht unter demselben `mutex` zuerst die
> Generation, dann die Folgenummer, und weist jede Publikation ab, deren
> Folgenummer nicht größer ist als die zuletzt übernommene. **Eine
> inhaltsgleiche Publikation meldet weiterhin Erfolg, ohne die Folgenummer zu
> verbrauchen.**"

und Matrixzeile **M-05** (HEAD-Zeile 572):

> „ein inhaltsgleicher Stand meldet weiterhin Erfolg (`neu ==
> persistenteMitglieder → true`, `SourcesModel.cpp:370-371`), **ohne die
> Folgenummer zu verbrauchen**"

**Status: BESTÄTIGT.** Die Folge `[] → [A] (Join) → [] (Unbind)` ist erreichbar
(drei der vier Produktaufrufer publizieren genau solche Stände). Der
`[]`-Stand ist inhaltsgleich mit dem leeren Modell, fällt in den frühen
`return true` und lässt die zuletzt übernommene Folgenummer stehen; die später
freigegebene Worker-Kopie `[A]` trägt eine **größere** Folgenummer als die
zuletzt übernommene und wird angenommen. F01 bleibt nach bestätigtem Unbind
möglich — genau der Zustand, den §4 F01 und `CLAUDE.md:56-57` („meldet ehrlich
einen Zustand") ausschließen.

**Einordnung: DEFEKT** (Matrixzeile und Bauplan verletzen die Zusage aus §4
F01 und die Invariante `CLAUDE.md:56-57`).

**Schließende Änderung:** M-05 und §6.3 trennen Nummernvergabe und Übernahme:
auch die inhaltsgleiche Publikation schreibt die zuletzt übernommene
Folgenummer fort (Erfolg bleibt), und M-05 misst das Interleaving
`[] → [A] → []` mit angehaltener älterer Kopie.

---

## Befund 2 — „Erhalte Dirty und Revision bei überholter Publikation"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:797-802 — DEFEKT, M-01/§6.3: Der neue
Ablehnungsfall benötigt eine eigene Behandlung beim Aufrufer. Am ZIEL führt eine
abgewiesene Publikation in `Ipc.cpp:1475–1484` zum Ausstieg vor Host-Dirty und
Revision; die State-Mutation ist bei M-01 aber bereits erfolgt und wurde nicht
durch einen Reload ersetzt. … Reload-Ablehnung und überholte Darstellung müssen
getrennt werden; der erzwungene Publikationswettlauf muss auch Dirty und
Revision prüfen.`

**Quelle an HEAD — `eq-copilot/plugin/src/prozessor/Ipc.cpp:1473-1486`:**

```cpp
    if (sourcesPublikationHakenFuerTest)
        sourcesPublikationHakenFuerTest (kopie.size());
    if (! sourcesModel.setzePersistenteMitglieder (kopie, generationBeimAbholen))
    {
        // Das Modell hat den Reload schon uebernommen: die Publikation ist GANZ
        // unterblieben. Kein Dirty, keine Revision - der geladene State ist die
        // Wahrheit, und eine Aenderung des Users ist das Laden nicht.
        sourcesNachfuehrungNachReloadUnterblieben.fetch_add (1);
        return;
    }
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
```

Der Ausstieg gilt heute **nur** dem Reload-Fall und ist im Kommentar genau so
begründet. Der tragende Gegenbeleg steht in
`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:1021-1026`:

```cpp
    pruefe (dirty.nonParam == 2,
            "M-12: Dirty-Zaehler == Zahl der State-aendernden Befehle (2)",
            juce::String (dirty.nonParam.load()));
    pruefe (p->v3StateRevisionFuerTest() == revisionVor + 2,
            "M-12: Revision-Delta == 2",
            juce::String ((juce::int64) (p->v3StateRevisionFuerTest() - revisionVor)));
```

**Gegenstand — Bauplan §6.3, HEAD-Zeilen 800-805** (der einzige `Ipc.cpp`-Absatz):

> „`Ipc.cpp`: Alle drei Publikationswege (`:1233`, `:1275`, `:1475`) ziehen ihre
> Folgenummer **unter derselben Sperre**, unter der sie ihre Kopie ziehen —
> nicht davor und nicht danach. Der Quellenkommentar `:1229-1233` … wird auf den
> Worker-Drain nachgezogen; der Kommentar `:1444-1447` … wird auf den Riegel
> umgeschrieben."

Kein Satz über den neuen Ablehnungszweig. M-01 (HEAD-Zeile 568) verlangt nur
„die ältere Publikation wird abgewiesen und gezählt".

**Status: BESTÄTIGT.** Nach dem Fix kann `setzePersistenteMitglieder` aus zwei
verschiedenen Gründen `false` liefern: Reload (Anwendung ist nichtig) und
Folgenummer (die eigene, angewandte Änderung wurde nur von einer jüngeren
Publikation überholt). Der zweite Grund lässt eine tatsächlich angewandte
persistente Änderung ohne Dirty und ohne Revisionsschritt — gegen
`nakama-state-v2.md:139` („steigt bei jeder persistenten Änderung genau
einmal") und `CLAUDE.md:194-196` („jede persistente Änderung meldet dem Host
Dirty-State"), und gegen den bestehenden B14-Nachweis Dirty == 2 / Revision
+2 bei zwei zustandsändernden Befehlen.

**Einordnung: DEFEKT** (Bauplan unvollständig gegenüber der Invariante und dem
bestehenden Nachweis; Zähler-/Statusvermischung mit dem Reloadfall).

**Schließende Änderung:** §6.3 gibt dem Folgenummer-Ausstieg in `Ipc.cpp` einen
eigenen Zweig mit eigenem Zähler, der Dirty und Revision weiterhin meldet;
M-01 prüft im erzwungenen Wettlauf zusätzlich Dirty-Zähler und Revisionsdelta.

---

## Befund 3 — „Sichere die Hypothesenfrische bis zur Store-Persistenz"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:863-868 — DEFEKT, M-13/§6.4: Der
zusätzliche Vergleich schützt nur den Cache-Eintrag. In
`hypothese_verdrahtung.rs:478–481` wird der Standlock vor der Persistenz
freigegeben: A kann dort anhalten, B mit neuer Evidenz vollständig persistieren
und A anschließend den alten Payload schreiben. Bei gleicher `finding_id`
überschreibt dessen späterer Event die Projektion (`store/writer.rs:589–593`);
die ID enthält die Eingangsmenge nicht (`hypothese/befund.rs:353–372`).`

**Quelle an HEAD — `broker/src/coordinator/hypothese_verdrahtung.rs:465-486`:**

```rust
            if !Self::ergebnis_ist_noch_gueltig(&stand, session, &befunde) {
                stand.befunde_neu_bilden = true;
                return false;
            }
            let alt = stand.befunde.get(session);
            if alt.map(Vec::as_slice) == Some(befunde.as_slice()) {
                return false;
            }
            if befunde.is_empty() {
                stand.befunde.remove(session);
                Vec::new()
            } else {
                stand.befunde.insert(session.clone(), befunde.clone());
                befunde
            }
        };
        for befund in &neue {
            self.befund_persistieren(session, befund);
        }
```

Der Block endet mit `};` — der `stand`-Lock fällt, **bevor**
`befund_persistieren` läuft.

`broker/src/store/writer.rs:587-593`:

```rust
        let sql = format!(
            "INSERT INTO {tabelle}({id_feld},project_binding_id,session_epoch,last_event_ord,state_jcs) \
             VALUES(?1,?2,?3,?4,?5) ON CONFLICT({id_feld}) DO UPDATE SET \
             project_binding_id=excluded.project_binding_id,session_epoch=excluded.session_epoch,\
             last_event_ord=excluded.last_event_ord,state_jcs=excluded.state_jcs \
             WHERE excluded.last_event_ord > {tabelle}.last_event_ord"
        );
```

`broker/src/coordinator/hypothese/befund.rs:353-372`:

```rust
pub fn finding_id(aufnahme: &Aufnahme, metrik: Zielmetrik, band: Bandintervall, quelle: &str) -> String {
    let mut hasher = Sha256::new();
    hasher.update(b"nakama.sonde014.finding.v1\0");
    hasher.update(aufnahme.session_epoch.as_bytes());
    hasher.update(metrik.wire().as_bytes());
    hasher.update(band.von.to_be_bytes());
    hasher.update(band.bis.to_be_bytes());
    hasher.update(quelle.as_bytes());
    hasher.update(aufnahme.metrics_version.to_be_bytes());
```

Die ID trägt Sitzung, Metrik, Band, Quelle, Metrikfassung — **nicht** die
Eingangsmenge. Der später geschriebene Event gewinnt die Projektion.

**Gegenstand — Bauplan §6.4, HEAD-Zeilen 863-868:**

> „`hypothese_verdrahtung.rs`: `aufnahmen_sammeln` gibt zusätzlich die **Menge
> der stabilen Evidence-IDs** zurück … `ergebnis_ist_noch_gueltig` vergleicht
> diese Menge beim Rückschreiben gegen den aktuellen Stand; weicht sie ab …,
> fällt das Ergebnis."

und **M-13** (HEAD-Zeile 585): Test `neue_evidenz_waehrend_der_rechnung_
verwirft_das_aeltere_ergebnis`, Zusage auf `stand.befunde` — also auf den Cache.

**Anforderungsquelle:** `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md:88`:

> „**Korrekturziel:** Der Eingangsstand einer Rechnung muss bis zu
> Ergebnisübernahme, **Persistenz und Publikation** identifizierbar bleiben."

**Status: BESTÄTIGT.** Der Riegel hält vor dem Cache-Eintrag an; das Fenster
zwischen Lockfreigabe und `befund_persistieren` ist ungeschützt, und die
Projektion entscheidet nach `last_event_ord`, also nach Schreibzeitpunkt, nicht
nach Eingangsstand. R-283-1 („nie über ein jüngeres Ergebnis geschrieben") ist
damit nur zur Hälfte gemessen.

**Einordnung: DEFEKT** (Regel R-283-1 und das Korrekturziel sind von keiner
Zeile mit messendem Test bis zur Persistenz gedeckt; M-13s Beweis fällt am
Cache statt an der Zusage).

**Schließende Änderung:** §6.4 zieht die Frischeprüfung bis zur Persistenz
(Persistieren unter derselben Sperrklammer oder Eingangsstand als
Ordnungsmerkmal im Event), und M-13 bekommt einen deterministischen
Store-/Projektionsnachweis statt nur `stand.befunde`.

---

## Befund 4 — „Löse vor dem Roundtrip die überlaufende Änderung aus"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:575 — DEFEKT, M-11: Die Vorbedingung
setzt beide Revisionen auf `int64max`, danach erfolgen ausschließlich Speichern
und Laden. Dieser Wert wird bereits am ZIEL akzeptiert
(`NakamaState.cpp:1905–1909`); ohne Aufruf des inkrementierenden Handgriffs
bleibt auch die vorgeschlagene Mutation aus M-10 wirkungslos. Der beschriebene
Fall ist deshalb nicht am Basisstand rot.`

**Quelle an HEAD — `eq-copilot/plugin/state/NakamaState.cpp:1904-1909`:**

```cpp
            if (! (revision.isInt() || revision.isInt64())
                || static_cast<juce::int64> (revision) < 1)
            {
                grund = "MainProject.assistant_step_v1 revision must be at least 1";
                return false;
            }
```

Der Reader kennt **nur** die untere Schranke; `int64max` lädt heute fehlerfrei.

**Gegenstand — M-11 (HEAD-Zeile 578):**

> „Zustand: Eine Instanz hält `intentBestandRevision = int64max` und einen
> Assistentenschritt mit `revision = int64max` | Ereignis: Der Host speichert;
> eine neue Instanz lädt die Bytes | … | Rotbeweis: Mutation: die obere
> Schranke aus M-10 entfernen → der gespeicherte Stand trägt
> `-9223372036854775808` und lädt read-only … **ROT am Basis-SHA**"

**Status: BESTÄTIGT.** Die Bühne enthält kein Ereignis, das inkrementiert;
ohne Inkrement erzeugt weder der Basisstand noch die Mutation den negativen
Wert. Der Fall ist am Basis-SHA grün, und das persistente F11-Gegenbeispiel
(Persistenzbeweis am geladenen Zustand) wird von ihm nicht getragen — Verstoß
gegen die eigene Matrixregel „Baulücke = heute rot oder heute nicht messbar"
und gegen die Beweisregel des Prüfauftrags („Persistenzbeweis F11 … am
geladenen Zustand").

**Einordnung: DEFEKT** (Klassifikation falsch: als Baulücke geführte Zeile ist
heute grün; Rotbeweis greift nicht an der Zusage).

**Schließende Änderung:** M-11 löst vor dem Speichern die Änderung an der
Obergrenze aus (`schrittAendern`/`bestandsrevisionHeben` bei `int64max`) und
prüft danach den tatsächlich **geladenen** Zustand.

---

## Befund 5 — „Führe die bestehende Queue-Begrenzung als Regressionswache"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:589 — DEFEKT, M-20: Die behauptete
Nichtmessbarkeit am Basisstand stimmt nicht: `queues.rs:223–229` ersetzt
vorhandene Einträge mit `mem::replace`, sodass die beschriebene Folge bereits
heute genau drei Deque- und Hochwassereinträge hinterlässt. … Außerdem erzeugt
die Mutation „zwei Sperrabschnitte" bei den beschriebenen seriellen Ersetzungen
kein Wachstum; dafür fehlt ein erzwungenes konkurrierendes Einreihen zwischen
Entfernen und Anhängen.`

**Quelle an HEAD — `broker/src/transport/server_v3/queues.rs:220-238`:**

```rust
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
                } else if g.0.len() >= CAP_WRITER {
                    false
                } else {
                    g.2.insert(schluessel, eintrag.marke);
                    g.0.push_back(eintrag);
                    true
                }
```

**Gegenstand — M-20 (HEAD-Zeile 592):**

> „Zusage: **Die Ersetzung ans Ende lässt die Queue nicht wachsen.** Die Deque
> trägt danach genau drei Einträge — einen je Schlüssel —, und die
> Hochwasserkarte `g.2` … trägt genau drei Einträge | … Entfernen und Anhängen
> sind **eine** Operation unter derselben Sperre …; zwischen beiden liegt kein
> Fenster, in dem der Schlüssel fehlt | … Rotbeweis: Mutation: entfernen und
> anhängen in zwei Sperrabschnitte trennen bzw. das Entfernen weglassen …
> **ROT** gegen die Fixfassung; **am Basis-SHA nicht messbar** (es gibt kein
> Anhängen)"

**Status: BESTÄTIGT.** Die Zusage („genau drei Einträge") ist heute messbar und
grün: `mem::replace` an alter Position hält die Deque bei drei Einträgen, `g.2`
ebenso. Damit ist die Zeile nach der Matrixdefinition (§5, HEAD-Zeilen 521-523)
eine Regressionswache, keine Baulücke. Zusätzlich trägt die Atomaritätszusage
(„kein Fenster") kein Gegenstück im Rotbeweis: serielle Ersetzungen in zwei
Sperrabschnitten wachsen nicht, sichtbar wird die Lücke nur mit einem
deterministisch erzwungenen, konkurrierenden Einreihen dazwischen.

**Einordnung: DEFEKT** (Klassifikation Baulücke/Regressionswache falsch;
zweite Zusage ohne tragenden Rotbeweis).

**Schließende Änderung:** M-20 wird als Regressionswache geführt, und die
Atomaritätszusage bekommt einen eigenen Rotbeweis mit erzwungenem
konkurrierendem Einreihen zwischen Entfernen und Anhängen.

---

## Befund 6 — „Trenne den vollständigen Rollback vom Retention-Fall"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:591 — DEFEKT, M-22: Mit der
ausdrücklich übernommenen Vorbedingung aus M-21 existiert E0 beim Rollback nicht
mehr: Der neu aufgenommene E32 verdrängt E0 durch `evidenz.rs:186–189`, genau wie
im Audit-Gegenbeispiel (`UEBERGABE-AN-CLAUDE.md:254`). Daher kann anschließend
nicht E0 mit `ausschlussgrund = None` vorliegen; M-23 verlangt für denselben Fall
korrekt das Überspringen.`

**Quelle an HEAD — `broker/src/coordinator/evidenz.rs:179-189`:**

```rust
            let historie = stand.evidenz.entry(key.clone()).or_default();
            let mut eintrag = stand_neu;
            // Die Ankunftsreihenfolge wird UNTER DEM LOCK vergeben …
            eintrag.empfangsfolge = self.evidenz_folge.fetch_add(1, Ordering::SeqCst);
            let folge = eintrag.empfangsfolge;
            historie.push_back(eintrag);
            while historie.len() > EVIDENZ_RETENTION {
                historie.pop_front();
            }
```

`docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md:254`:

> „Die Historie steht bei 32 Belegen. E1 ist durch eine frühere erfolgreiche
> Intervention ausgeschlossen. Eine neue Invalidierung von E0 wartet auf den
> Store. Währenddessen wird E32 aufgenommen und **E0 durch Retention vorne
> entfernt**."

**Gegenstand — M-21 (HEAD-Zeile 593) und M-22 (HEAD-Zeile 594):**

> M-21: „Die Evidenzhistorie eines `ClientKey` steht am Retention-Deckel
> (`EVIDENZ_RETENTION = 32` …); … eine neue Invalidierung hat E0 unter dem
> Standlock markiert … | Ein zweiter Verbindungsthread nimmt Evidenz auf,
> `pop_front()` verschiebt alle Indizes um eins …"
>
> M-22: „**Wie M-21** | Derselbe fehlgeschlagene Append | **Der eigene
> Ausschluss fällt vollständig.** E0 trägt danach `ausschlussgrund = None` …"

**Status: BESTÄTIGT.** In M-21s Bühne ist E0 das älteste Element und wird von
`pop_front()` entfernt; eine Aussage „E0 trägt danach `ausschlussgrund = None`"
ist in derselben Bühne nicht herstellbar. M-23 (HEAD-Zeile 595) fordert für
genau diesen Fall korrekt den Übersprung. Die gemischte Einstufung „heute grüne
Einzelfall-Wache, ROT gegen die Fixfassung" (§5.6) trägt die Bühne ebenfalls
nicht.

**Einordnung: DEFEKT** (Matrixzeile widerspricht dem Audit-Gegenbeispiel und
der eigenen Nachbarzeile; Bühne nicht herstellbar).

**Schließende Änderung:** M-22 bekommt eine eigene Bühne ohne Retention-Abbau
mit **zwei erhalten gebliebenen** markierten Belegen; der von der Retention
entfernte Beleg bleibt ausschließlich Gegenstand von M-23.

---

## Befund 7 — „Verwende für M-31 eine tatsächlich zustandsändernde Mutation"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:605 — DEFEKT, M-31/§8.1 Feinheit 4:
`DspKernAusfuehrung::vergissLetztePublikation()` setzt am ZIEL lediglich
`zuletztGueltig = false` (`NakamaTransaktion.h:352`). Es verwirft keinen
bestätigten Zustand; `getStateInformation()` serialisiert weiterhin den
Transaktionszustand (`SondeProcessor.cpp:804–827`). Der vorgeschriebene
zusätzliche Aufruf lässt deshalb den angekündigten Bytevergleich nicht fallen,
und die gegenteilige technische Aussage in Feinheit 4 ist falsch.`

**Quelle an HEAD — `eq-copilot/plugin/state/NakamaTransaktion.h:350-360`:**

```cpp
    /** Die Kernsicht der letzten Committed-Publikation, sofern es eine gab. */
    bool zuletztPubliziert (parameter::Satz& aus) const noexcept;
    void vergissLetztePublikation() noexcept { zuletztGueltig = false; }

private:
    dsp::DspKern&      kern;
    parameter::DspSatz sichtCommit;
    parameter::DspSatz sichtWeg;
    parameter::Satz    zuletzt = parameter::standardSatz();
    bool               zuletztGueltig = false;
```

`eq-copilot/plugin/sonde/SondeProcessor.cpp:804-827`:

```cpp
void SondeProcessor::getStateInformation (juce::MemoryBlock& ziel)
{
    const juce::ScopedLock l (zustandSchloss);
    nakama::state::speichere (gehaltenerStand(), ziel);
}
…
nakama::state::Zustand SondeProcessor::gehaltenerStand() const
{
    auto stand = zustand;
    if (! stand.nurLesen && stand.hatParameters)
        transaktion->schreibeIn (stand);
    return stand;
}
```

`zuletztGueltig` wird ausschließlich in `NakamaTransaktion.cpp:727/730/758/761/767`
gelesen und geschrieben (Publikationsvergleich), nie in `schreibeIn`.

**Gegenstand — M-31 (HEAD-Zeile 608) und §8.1 Feinheit 4 (HEAD-Zeilen 1332-1339):**

> M-31 Rotbeweis: „Mutation: `reset()` zusätzlich
> `dspAusfuehrung->vergissLetztePublikation()` rufen lassen (also
> `releaseResources` nachbauen) → der Zustandsvergleich fällt"
>
> Feinheit 4: „`reset()` ruft **nicht**
> `dspAusfuehrung->vergissLetztePublikation()` — das gehört zu
> `releaseResources` und **würde den bestätigten Zustand verwerfen**, den
> R-283-3 ausdrücklich stehen lässt (M-31)."

**Status: BESTÄTIGT.** Der Aufruf markiert nur den Publikationscache; die von
M-31 zugesagten bytegleichen `getStateInformation`-Bytes entstehen weiterhin
aus dem Transaktionskern. Der Rotbeweis fällt an einem Nebeneffekt (genauer:
an gar nichts), und Feinheit 4 begründet die Entscheidung mit einer falschen
technischen Aussage.

**Einordnung: DEFEKT** (Rotbeweis trägt die Zusage nicht; Feinheit 4
widerspricht dem gemessenen Code).

**Schließende Änderung:** M-31 bekommt einen Rotbeweis, der Programm,
Parameter oder persistierten Zustand tatsächlich ändert (z. B. `reset()` den
Transaktions-/Parameterstand zurücksetzen lassen), und Feinheit 4 streicht die
falsche Begründung.

---

## Befund 8 — „Klassifiziere den Reset-Nulltest als Regressionswache"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:608 — DEFEKT, M-34: Der ausgeschaltete
Pfad besteht den beschriebenen Fall bereits am ZIEL: Die Sonde besitzt keine
`reset()`-Überschreibung (`SondeProcessor.h:134–137`), und der in F05
festgehaltene leere JUCE-Basispfad verändert weder Samples noch Zähler.`

**Quelle an HEAD — `eq-copilot/plugin/sonde/SondeProcessor.h:130-138`:**

```cpp
public:
    SondeProcessor();
    ~SondeProcessor() override;

    void prepareToPlay (double samplerate, int maxBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layout) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void nakamaBlockEmpfangen (const eqcop::hostbruecke::Blockbefund&) noexcept override;
```

`grep -n "void reset\|::reset" eq-copilot/plugin/sonde/SondeProcessor.{h,cpp}`
liefert **keinen Treffer** — es gibt keine Überschreibung; die JUCE-Basis ist
leer (§1.1 F05).

**Gegenstand — M-34 (HEAD-Zeile 611):**

> „**Der ausgeschaltete Pfad bleibt bitidentisch.** Der Nulltest über 1000
> Blöcke … bleibt bitgleich, auch mit `reset()`-Aufrufen dazwischen … |
> Rotbeweis: Mutation: `reset()` den Ausgangspuffer nullen lassen → der
> Nulltest reißt"

Die Zeile trägt kein „Regressionswache" und zählt damit nach §5.6 zu den 55
Baulücken.

**Status: BESTÄTIGT.** Eingestreute `reset()`-Aufrufe laufen heute in die leere
JUCE-Basis; der Nulltest bleibt bitgleich. Die Zeile ist heute grün und schützt
`CLAUDE.md:29-31`, belegt aber nicht die Behebung von F05 (aktiver
Filterzustand).

**Einordnung: DEFEKT** (Klassifikation: als Baulücke geführte Zeile ist heute
grün; §5.6-Zählung 55/15 verschiebt sich entsprechend).

**Schließende Änderung:** M-34 wird ausdrücklich als Regressionswache geführt
(und die Zählung in §5.6 nachgezogen); der F05-Beleg bleibt bei M-30 bis M-33.

---

## Befund 9 — „Wähle für M-45 ein Signal, bei dem die Mutation das Gate kippt"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:624 — DEFEKT, M-45: „Dasselbe
Material" übernimmt den −55-dBFS-Sinus aus M-44. Mit korrektem Δf sind beide
Stufen oberhalb des −60-dB-Gates; die vorgeschriebene Verwendung von
`kHauptPunkte` erhöht die Bassenergie um 6,02 dB, lässt sie aber ebenfalls
aktiv. Die einzige zugesagte Messgröße, gleiche Gate-Entscheidung, bleibt somit
trotz Mutation erfüllt.`

**Quelle an HEAD — `eq-copilot/plugin/core/analysis/FeatureEngine.h:232-247`:**

```cpp
    /** Bassstufe: aufloesungsbestimmend unter `kTrennungHz`. */
    static constexpr int kBassPunkte = 16384;
    /** Hauptstufe. */
    static constexpr int kHauptPunkte = 4096;
…
    /** Aktivitaetsgate, wie M1. */
    static constexpr double kAktivGateDb = -60.0;
```

`eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:355-362`:

```cpp
    const double norm = 2.0 / (s.fs * s.fensterEnergie);
    const int bins = s.punkte / 2 + 1;
    for (int k = 0; k < bins; ++k)
        s.psd[(std::size_t) k] = (s.fftM.leistung (k) + s.fftS.leistung (k)) * norm;

    const double gesamt = summeBereich (s, 0, bins);
    const bool aktiv = gesamt > 0.0
                    && 10.0 * std::log10 (gesamt) > kAktivGateDb;
```

`10·log10(16384/4096) = 6,02 dB`.

**Gegenstand — M-45 (HEAD-Zeile 627):**

> „Zustand: **Dasselbe Material**, eine Instanz, beide Stufen … | Zusage:
> **Gleiche Signalenergie bekommt dieselbe Bewertung, unabhängig von der
> FFT-Länge.** Beide Stufen entscheiden das Gate gleich … | Rotbeweis:
> Mutation: `Δf` fest aus `kHauptPunkte` bilden statt aus `s.punkte` → die
> Bassstufe kippt um 6,02 dB. **ROT am Basis-SHA**"

M-44 (HEAD-Zeile 626) definiert „dasselbe Material" als „1-kHz-Stereosinus mit
gemessen −55 dBFS RMS über drei Sekunden".

**Status: BESTÄTIGT.** −55 dBFS liegt 5 dB über dem Gate; +6,02 dB hebt die
Bassstufe auf ≈ −49 dBFS — beide Stufen bleiben aktiv, die Gate-Entscheidung
bleibt gleich. Der Rotbeweis kippt die zugesagte Messgröße nicht („Rotbeweis
fällt an einem Nebeneffekt", Befundklasse des Prüfauftrags).

**Einordnung: DEFEKT** (Rotbeweis trägt R-283-4 an dieser Zeile nicht).

**Schließende Änderung:** M-45 nennt einen eigenen Pegel im Fenster zwischen
−66,02 und −60 dBFS, bei dem die Fehlskalierung die Gate-Entscheidung der
Bassstufe tatsächlich kippt.

---

## Befund 10 — „Binde den Filterreset an den echten Passagenbeginn"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:1042-1045 — DEFEKT, M-49/M-50/§6.6:
`passVon` ist nicht ausschließlich der Passagenbeginn, sondern der blocklokale
Anfang des geschnittenen Ausschnitts: `max(startSample, b0) - b0`
(`featureengine/Spektrum.h:75–79`). In jedem späteren Block innerhalb der
Passage ist er erneut 0. Der hier vorgeschriebene Reset bei `i == passVon` leert
daher die Filterhistorie an jedem Blockanfang und macht die Messung
blockgrößenabhängig.`

**Quelle an HEAD — `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:74-86`:**

```cpp
            const std::int64_t von = std::max (passagenfenster.startSample, b0);
            const std::int64_t bis = std::min (passagenfenster.endeSample, bEnde);
            if (bis > von)
            {
                passVon = (int) (von - b0);
                passBis = (int) (bis - b0);
                // `bis` ist bereits auf `endeSample` gekappt: Gleichheit
                // heisst, dass die Passage genau hier zu Ende ist.
                fensterEndetHier = bis >= passagenfenster.endeSample;
            }
```

und `:105-113` (das bestehende Gegenstück am Ende):

```cpp
        if (fensterEndetHier && i == passBis)
        {
            const double rest = tp.nachlauf();
            rahmenTruePeak = std::max (rahmenTruePeak, rest);
            passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, rest);
            fensterEndetHier = false;   // genau EINMAL je Block
        }
```

Für jeden Block, der vollständig **innerhalb** der Passage liegt, gilt
`von == b0` und damit `passVon == 0`. Ein Startmerker analog zu
`fensterEndetHier` existiert nicht.

**Gegenstand — Bauplan §6.6 (HEAD-Zeilen 1044-1047) und M-49/M-50:**

> „`Spektrum.h` (Passagenanfang): Im Samplelauf wird an der Grenze
> `i == passVon` der Polyphaseninterpolator geleert, symmetrisch zum
> bestehenden Nachlauf an `i == passBis` (`:103-113`)."
>
> M-49 (HEAD-Zeile 631): „er läuft im Samplelauf an der Grenze `i == passVon`
> (`Spektrum.h:57-87` kennt den Index bereits), symmetrisch zum bestehenden
> Nachlauf an `i == passBis`"
>
> M-50 (HEAD-Zeile 632): „Der Reset an `passVon` leert die 24-Tap-Kette **vor**
> dem ersten Fenstersample"

**Status: BESTÄTIGT.** Die vorgeschriebene Bedingung feuert in jedem Block der
Passage, nicht einmalig am Startsample. Das verletzt R-283-5 („Der Filterreset
hängt am Startsample der Passage") und macht das Passagenmaximum
blockgrößenabhängig — der Fehler, den F07 gerade beheben soll.

**Einordnung: DEFEKT** (Bauplan und Rotbeweis widersprechen R-283-5 und dem
gemessenen Code).

**Schließende Änderung:** §6.6, M-49 und M-50 binden den Reset an einen
einmaligen Startmerker (Gegenstück zu `fensterEndetHier`, nur wenn
`von == passagenfenster.startSample`) und misst die Blockgrößenunabhängigkeit.

---

## Befund 11 — „Miss die Erholung an der tatsächlichen Fensterfrist"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:637 — DEFEKT, M-58: Die einzige
Messung fünf Sekunden nach der Ersetzung kann eine zu frühe Freigabe nicht
erkennen. Bereits am ZIEL sind dann alle 30 Loudnesszellen durch sauberes
Material ersetzt (`featureengine/Lautheit.h:35–44,90–102`); der Fall ist grün,
obwohl die Präsenzbits zuvor zu früh zurückkehrten. Auch eine fälschlich vier
Sekunden dauernde Sperre würde diese Endprüfung bestehen.`

**Quelle an HEAD — `eq-copilot/plugin/core/analysis/featureengine/Lautheit.h:31-44`:**

```cpp
inline void FeatureEngine::zelleSchliessen() noexcept
{
    const double mittel = zelleKEnergie / (double) zellenSamples;
    kurzZellen[(std::size_t) kurzStand] = mittel;
    …
    kurzStand = (kurzStand + 1) % kKurzZellen;
    if (kurzGefuellt < kKurzZellen) ++kurzGefuellt;
```

und `:90-102`:

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

(`kZelleSekunden = 0,1`, „LUFS-S ueber 3 s = 30 Zellen", `FeatureEngine.h:249-251`.)

**Gegenstand — M-58 (HEAD-Zeile 640):**

> „Ereignis: **5 s nach dem Ende der Ersetzung** | Zusage: **Die Verriegelung
> hebt sich wieder auf.** Beide Präsenzbits sind gesetzt … | Rotbeweis:
> Mutation: die Verriegelung an das Evidenzfenster statt an die Metrikfenster
> hängen → die Erholung kommt zu spät oder zu früh. **ROT am Basis-SHA** für
> „zu früh""

**Status: BESTÄTIGT.** Nach 5 s ist der 3-s-Ring vollständig mit sauberem
Material gefüllt; jede Sperrfrist ≤ 5 s besteht die Endprüfung. Eine zu frühe
Freigabe ist an dieser Messstelle unsichtbar, die Behauptung „ROT am Basis-SHA
für zu früh" trägt nicht.

**Einordnung: DEFEKT** (Zeitbeweis nicht am Fensterrand; Klassifikation als
Baulücke falsch, solange nur am Ende gemessen wird).

**Schließende Änderung:** M-58 misst am Fensterrand (unmittelbar vor und nach
3 s nach Ende der Ersetzung) statt nur nach 5 s; sonst wird die Zeile
ausdrücklich als Regressionswache geführt.

---

## Befund 12 — „Behalte das Referenzbudget für die transienten EBU-Fälle bei"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:1406-1413 — DEFEKT, M-65/§8.1
Feinheit 14: Die Beschränkung des ±0,1-dB-Budgets auf analytische Sinus steht
nicht in der Zusage: SONDE-013 M-02/M-81 und Entwurf:2626–2627 verlangen diese
Genauigkeit gegen eine validierte Referenz auf Standard-Testmaterial. Die
größere Normtoleranz ersetzt dieses zusätzliche Projektbudget nicht. Eine
Budgetänderung ist laut Entwurf:3599–3600 erst nach Messung mit versionierter
Evidenz und Userwirkung zulässig; Etappe 1 liefert keine solche Messung.`

**Quelle an HEAD — `docs/FL-Nakama-Sonden-Design-Entwurf.md:2624-2627`:**

```
### 39.3 Abnahmekriterien

- Loudness und True Peak stimmen auf Standard-Testmaterial mit einer validierten Referenz
  innerhalb ±0,1 LU beziehungsweise ±0,1 dB überein.
```

`docs/FL-Nakama-Sonden-Design-Entwurf.md:3599-3600`:

```
Diese Budgets sind Hypothesen mit Abnahmetest. Die ausführende Phase darf sie nach Messung ändern,
muss Änderung, Evidenz und Userwirkung aber versioniert dokumentieren.
```

`docs/beweise/SONDE-013.md:623` (M-81, Zusagespalte):

> „Toleranz ±0,1 LU und ±0,1 dB gegen eine validierte Referenz auf
> Standard-Testmaterial. Der 8-fach-True-Peak-Pfad besteht das offizielle
> EBU-Testset bei 48 kHz **plus** äquivalente generierte
> Mehrsampleraten-Goldens …"

(`:471`, M-02, nennt die analytische Referenz nur in der **Nachweisspalte**,
nicht in der Zusage; die Zusage lautet dort „Toleranz ±0,1 dB (§49.3)".)

**Gegenstand — §8.1 Feinheit 14 (HEAD-Zeilen 1409-1416) und M-65 (HEAD-Zeile 652):**

> „Für 20 bis 23 gilt die Normtoleranz **+0,2/−0,4 dBTP**, für 15 bis 19 bleibt
> es bei ±0,1 dB (§49.3). Begründung: die schärfere Nakama-Toleranz ist damit
> begründet, dass 15 bis 19 gegen eine **analytische** Referenz gemessen
> werden … Für 20 bis 23 gibt es keine analytische Referenz …"
>
> M-65: „der gemessene True Peak liegt bei **0,0 dBTP mit der Normtoleranz
> +0,2/−0,4 dBTP**"

**Status: BESTÄTIGT.** Die zitierte Zusage bindet ±0,1 dB an „Standard-
Testmaterial … validierte Referenz", nicht an analytisch geschlossene Signale;
die EBU-Fälle 20 bis 23 sind Standard-Testmaterial. Feinheit 14 verschiebt
damit eine Zusage und ändert faktisch ein Budget, ohne die vom Entwurf
geforderte Messung mit versionierter Evidenz und Userwirkung — Etappe 1 misst
nichts.

**Einordnung: DEFEKT** (Feinheit verschiebt eine Zusage der Quelle und erfindet
für die neuen Fälle eine andere Grenze ohne die verlangte Legitimation).

**Schließende Änderung:** M-65 und Feinheit 14 behalten ±0,1 dB gegen eine
validierte Referenz für 20 bis 23 (Referenzvergleich benennen) oder führen die
Budgetänderung nach Entwurf `:3599-3600` mit Messung, Evidenz und Userwirkung
versioniert ein.

---

## Befund 13 — „Lass den zusätzlichen State.cpp-Pfad vor Etappe 2 freigeben"

**Wörtlich (Urteil):** `[P2] … NAK-283.md:779 — DEFEKT, §6.3: Der Bauplan nimmt
`eq-copilot/plugin/src/prozessor/State.cpp` als Änderungspfad auf und verlangt an
Zeile 803 eine Signaturanpassung. Dieser Produktpfad fehlt jedoch in der
verbindlichen Pfadliste für Etappe 2 (§3:126); die ausdrücklich genehmigten
Ergänzungen aus §8.4 betreffen ausschließlich Etappen 4 und 6.`

**Quelle an HEAD — `eq-copilot/plugin/src/prozessor/State.cpp:341-352`:**

```cpp
    // Dieselbe Wache wie bei den Handgriffen in `Ipc.cpp`: Rollenwahl und
    // `setStateInformation` laufen heute beide auf dem Message-Thread, ein Reload
    // kann zwischen Block und Publikation nicht liegen. Kommt er doch, ist die
    // Publikation ganz unterblieben - kein Dirty, keine Revision; der geladene
    // State ist die Wahrheit. …
    if (sourcesModel.setzePersistenteMitglieder (mainMitglieder, generation))
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
    }
    else
        sourcesNachfuehrungNachReloadUnterblieben.fetch_add (1);
```

Aufrufer von `setzePersistenteMitglieder` im **Produkt** (grep über
`eq-copilot/`): `Ipc.cpp:1233`, `Ipc.cpp:1275`, `Ipc.cpp:1475`,
**`State.cpp:347`** — dazu der Testpfad `Sonde012SourcesModelTest.cpp`. Das
deckt sich mit §9 („F01 hat **vier** Produktaufrufer … plus einen Testpfad").

**Gegenstand — §3, Etappe 2 (HEAD-Zeile 129) gegen §6.3 (HEAD-Zeilen 779, 806-807):**

> §3: „`eq-copilot/plugin/src/prozessor/Ipc.cpp`,
> `eq-copilot/plugin/src/SourcesModel.h` und `.cpp`,
> `eq-copilot/plugin/state/NakamaState.cpp`, zugehörige Tests unter
> `eq-copilot/plugin/tests/`, State-Fixtures nur über den Erzeuger"
> — **ohne** `State.cpp`.
>
> §6.3 Ticketpfade: „`eq-copilot/plugin/src/prozessor/State.cpp` (`:347`,
> Reloadaufrufer)"; §6.3 Änderungen: „`State.cpp`: Der Reloadaufrufer `:347`
> reicht die Folgenummer mit; die Generationsprüfung bleibt die erste
> Entscheidung (M-04)."
>
> §8.4 (HEAD-Zeilen 1470-1482) führt genau zwei Abweichungen, beide zu
> **Etappe 4** und **Etappe 6**.

**Status: BESTÄTIGT.** `State.cpp` ist ein Produktpfad außerhalb der gebundenen
Etappe-2-Liste und außerhalb der genehmigten Abweichungen — ein Befund nach der
Klasse „ein Fix berührt einen Pfad außerhalb der Ticketpfade aus §3 samt den
zwei Ergänzungen in §8.4".

**Zusatzfrage (Dirigent).** Ja: `State.cpp:347` **ist** einer der vier
Produktaufrufer von `setzePersistenteMitglieder`, und F01 deckt ausdrücklich
alle Aufrufer (§1.1: „F01 betrifft alle … Aufrufer"; M-06 zählt `State.cpp:347`
als einen der fünf Publikationswege). Zu ändern ist dort genau: der Aufruf
reicht die unter derselben Sperre gezogene **Folgenummer** als zweiten Parameter
mit (Signaturfolge), der Generationsvergleich bleibt die erste Entscheidung
(M-04), und der `else`-Zweig muss vom neuen Folgenummer-Ausstieg unterschieden
werden (siehe Befund 2). Technisch unvermeidbar, sobald
`setzePersistenteMitglieder` einen Parameter bekommt — der Defekt ist ein
Ticketgrenzen-, kein Technikdefekt.

**Einordnung: DEFEKT** (Bauplan überschreitet die gebundene Ticketgrenze §3;
Abweichung nicht in §8.4 geführt und nicht datiert genehmigt).

**Schließende Änderung:** §8.4 nimmt `eq-copilot/plugin/src/prozessor/State.cpp`
(nur `:347`, Folgenummer mitreichen) als dritte, datiert genehmigte Abweichung
auf und §3 Etappe 2 wird entsprechend abgeglichen.

---

## Gemeinsame Ursache

1. **Klassifikationsfehler Baulücke ↔ Regressionswache** (Befunde 4, 5, 8, 11;
   Randanteil 6): „ROT am Basis-SHA" wurde daraus geschlossen, dass der
   **Mechanismus** heute fehlt, nicht daraus, dass die **Messung** heute
   fällt. Wo die zugesagte Größe schon heute stimmt (Deque-Zahl, Nulltest,
   Erholung nach 5 s, Save/Load von `int64max`), ist die Zeile grün und
   schützt nur.
2. **Rotbeweise, die den Fixfall nicht kippen** (Befunde 4, 7, 9, 11): die
   Mutation trifft eine Nachbargröße (Publikationscache statt Zustand, 6 dB
   über dem Gate statt am Gate, ein Inkrement, das nie gerufen wird) — die
   zugesagte Messgröße bleibt erfüllt.
3. **Frischeprüfung nur bis zum Cache statt bis zur Persistenz und
   Publikation** (Befunde 1, 2, 3): der Riegel sitzt am sichtbaren
   In-Memory-Stand; danach fallen Lock (Broker) beziehungsweise
   Dirty/Revision (Plugin) aus der Betrachtung, obwohl Audit-Korrekturziel und
   `nakama-state-v2.md:139` genau dorthin reichen.
4. **Bühne von der Nachbarzeile geerbt, ohne ihre Invarianten neu zu prüfen**
   (Befunde 6, 9; Muster auch 10): „Wie M-21" / „Dasselbe Material" übernimmt
   Vorbedingungen, die die eigene Zusage unmöglich oder wirkungslos machen.
5. **Ticketgrenze nicht mitgeführt** (Befund 13): §5/§6 verlangen einen Pfad,
   den §3 nicht führt und §8.4 nicht als Abweichung nennt.

## Scope-Beweis

Gelesen: `docs/beweise/NAK-283.md` (§1–§9), `docs/beweise/roh/NAK-283-matrix
pruefung-1-auftrag.txt`, das Urteil unter `%TEMP%\nakama-nak283-matrix1-…-last.txt`,
`docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md` (:86-90, :250-258),
`docs/FL-Nakama-Sonden-Design-Entwurf.md` (:2620-2632, :3595-3605),
`docs/beweise/SONDE-013.md` (:471, :623), `CLAUDE.md` (:29-31, :56-57,
:178-180, :194-196). An HEAD nachgemessen: `SourcesModel.cpp:349-380`,
`Ipc.cpp:1440-1540`, `State.cpp:338-356`, `NakamaState.cpp:1898-1915`,
`Sonde012ProjectReloadTest.cpp:1010-1035`, `hypothese_verdrahtung.rs:435-500`,
`store/writer.rs:580-600`, `coordinator/hypothese/befund.rs:344-378`,
`transport/server_v3/queues.rs:198-240`, `coordinator/evidenz.rs:172-195`,
`NakamaTransaktion.h:340-360` (+ `zuletztGueltig`-Grep),
`SondeProcessor.cpp:798-830`, `SondeProcessor.h:128-140` (+ `reset`-Grep),
`FeatureEngine.h:228-250`, `featureengine/Spektrum.h:50-115` und `:345-370`,
`featureengine/Lautheit.h:25-50` und `:85-105`.

Nicht ausgeführt: kein Build, kein Test, kein Skript, kein Produktlauf, keine
Repro. Git nur lesend (`git rev-parse`, `git diff --stat`, `git diff`).
Geschrieben wurde ausschließlich diese Datei im Scratchpad; HEAD blieb
`5e68baf2954902be1a9da221f9523f2697bb00b1`, der Arbeitsbaum unverändert.
