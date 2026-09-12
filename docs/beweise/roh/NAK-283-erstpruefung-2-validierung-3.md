# NAK-283 Erstprüfung 2 — Validierung Befund 3 (Host-Dirty an den Revisionsgrenzen)

**Stand:** Branch `master`, HEAD `92386c37`; Etappenstand `988dc44d`, Basis
`3cfa4176`. `git diff --stat 988dc44d HEAD` berührt nur
`.claude/skills/dirigent/SKILL.md`, `docs/beweise/NAK-283.md`,
`docs/offene-punkte.md` und zwei Rohdateien; unter `eq-copilot/` ist kein Byte
verschieden, und `git diff HEAD -- eq-copilot/` ist leer. Alle hier gelesenen
Quell- und Testdateien gelten damit unverändert für `988dc44d`. Diese
Validierung ist **lesend**: kein Bau, kein Bein, keine Änderung außer dieser
Datei.

**Befund 3 (Codex Astra, Thread `01a0965b`), wörtlich:**

> [P2] Miss Host-Dirty an den zugesagten Revisionsgrenzen —
> eq-copilot/plugin/tests/Sonde014IntentTest.cpp:1526-1530. DEFEKT in der
> Messabdeckung von M-12: Die vier MAX−1-Fälle prüfen ausschließlich
> `state::...` ohne Prozessor oder Hostlistener. Dieser Dirty-Block lädt
> dagegen keinen Grenzstand und misst nur Intent bei Revision 1→2, nicht alle
> vier Handgriffe bei MAX−1. Nachvollziehbar mit
> `git show 988dc44d:eq-copilot/plugin/tests/Sonde014IntentTest.cpp`, Zeilen
> 1470-1549. Auch die MAX-Ablehnungen für Schutz und Beziehung in M-08/M-09
> besitzen keine Hostmessung; M-11 ergänzt dort nur Intent und Assistent. Die
> Grenzstände müssen zusätzlich durch die Prozessorhandgriffe mit Listener
> laufen, um die zugesagten null beziehungsweise genau eine Dirty-Meldung
> nachzuweisen (CLAUDE.md:195-197).

---

## 1. Quellzitate je Teilbehauptung

### 1.1 Was M-12 zusagt (die Spezifikation)

`docs/beweise/NAK-283.md:608`, Zeile M-12, Spalte **Zustand und
Vorbedingung**:

> `intentBestandRevision == int64max − 1` bzw. Assistentenrevision
> `int64max − 1`; alle vier Handgriffe

Spalte **Ereignis**:

> Je ein tatsächlicher Entfern- bzw. Änderungsschritt

Spalte **Zusage**:

> **Unter der Grenze ändert sich nichts am heutigen Verhalten.** Der Eintrag
> wird entfernt, die Revision steigt um genau 1, Host-Dirty wird **genau
> einmal** gemeldet

Spalte **Reihenfolge und Frist**:

> Dirty nach der Mutation, außerhalb eigener Prozessorsperren

Spalte **Test**:

> `Sonde014IntentTest.cpp` bestehend (`:565-570`, `:623-632`, `:747-759`), um
> die MAX−1-Fälle erweitert, Bein **B27**

Spalte **Rotbeweis**:

> **Regressionswache (heute grün)** — hält fest, dass die Vorprüfung den
> Normalfall nicht verschiebt; absichtlich gebrochen mit einer Schranke bei
> `int64max − 1`

Spalte **Quelle**:

> `nakama-state-v2.md:139` „steigt bei jeder persistenten Änderung genau
> einmal"

Die drei Größen der Zusagespalte stehen in **einem** Satz unter **einer**
Vorbedingung, und die Vorbedingungsspalte sagt ausdrücklich „alle vier
Handgriffe". Die Zusage lautet damit: bei `int64max − 1` und je Handgriff —
Eintrag entfernt, Revision plus 1, **und** genau eine Host-Dirty-Meldung.

### 1.2 Was M-07 bis M-09 zusagen

`docs/beweise/NAK-283.md:603` (M-07), Spalte **Zusage**:

> **Ablehnung ohne Mutation.** Der Handgriff liefert `false` mit
> `grund = "intent revision would overflow"`, `veraendert` bleibt `false`,
> **und `z.sourceIntents` ist byteweise unverändert**

Spalte **Reihenfolge und Frist**:

> Die Revisionsprüfung steht **vor** dem `erase`, nicht danach; kein
> Host-Dirty, keine Revision. Frist: innerhalb desselben Aufrufs, kein
> späterer Ausgleich

Spalte **Test**:

> `eq-copilot/plugin/tests/Sonde014IntentTest.cpp`, **NEU**
> `entferne_intent_an_der_revisionsobergrenze_mutiert_nichts`, Bein **B27**

`:604` (M-08), Spalte **Zusage**: „**Ablehnung ohne Mutation**, wie M-07, für
`z.schutzangaben` (gemessener Feldname, `NakamaState.cpp:2340-2346`)", Spalte
**Reihenfolge und Frist**: „Wie M-07". `:605` (M-09), Spalte **Zusage**:
„**Ablehnung ohne Mutation**, wie M-07, für `z.intentBeziehungen`", Spalte
**Reihenfolge und Frist**: „Wie M-07".

**Wichtig für die Einordnung:** „kein Host-Dirty" steht in M-07 bis M-09 in
der Spalte *Reihenfolge und Frist*, nicht in der Spalte *Zusage*. Die
Spaltendefinition `docs/beweise/NAK-283.md:506-508` lautet:

> *Zusage* einen prüfbaren Satz; *Reihenfolge und Frist* Callback, Thread,
> Generation oder Fenster, an denen die Zusage hängt

Bei M-12 dagegen steht „Host-Dirty wird **genau einmal** gemeldet" **in der
Zusagespalte**.

### 1.3 §4 F11 wörtlich

`docs/beweise/NAK-283.md:383-397`:

> ### F11 — Akzeptierte maximale Revision verletzt Mutation und Roundtrip
>
> - Vertragssatz `eq-copilot/schemas/state/nakama-state-v2.md:139`:
>   „`intent_revision_v1` · `int64` ≥ 1 · Revision des **ganzen** Bestands,
>   **steigt bei jeder persistenten Änderung genau einmal**. Fehlt sie, ist der
>   Bestand nie beschrieben worden; **Inhalt ohne Revision** ist ein ungültiger
>   Stand, weil die Vollständigkeitsmarke aus SONDE-014 M-86 dann keine Zahl
>   hätte."
> - Vertragssatz `:136` (`source_intents_v1`): „`revision` `int64` ≥ 1".
> - `CLAUDE.md`, Tragende technische Invarianten: „**State bleibt
>   verlustfrei.** … jede persistente Änderung meldet dem Host Dirty-State."
> - `CLAUDE.md`, Tragende technische Invarianten: „**Schemas sind Verträge.**
>   … **Save und Load gemeinsam testen.**"

### 1.4 §6.3 Bauplan und §8.1 Feinheit 12

`docs/beweise/NAK-283.md:930` (Testtabelle der Etappe 2), Zeile M-12, wörtlich
die vier Zellen: „bestehende MAX−1-Fälle erweitert" — `Sonde014IntentTest.cpp`
— **B27** — M-12.

`docs/beweise/NAK-283.md:1703-1707`:

> **Feinheit 12 zu F11 — prüfen statt zurückrollen.** Die Revisionsprüfung
> wandert **vor** die Mutation; es wird kein Rollback gebaut. Begründung: die
> Übergabe fordert „Ablehnung muss vor der ersten Mutation feststehen"; ein
> Rollback bräuchte eine Kopie des Containers im Erfolgspfad und wäre teurer
> als eine Prüfung.

### 1.5 Was die drei benannten bestehenden Fälle am Basisstand messen

`git show 3cfa4176:eq-copilot/plugin/tests/Sonde014IntentTest.cpp`, `:560-571`
(der Fall, den M-12 als `:565-570` nennt):

>     auto p = mainProzessor();
>     pruefe (p->setzeQuellenbeziehung (kQuelleA, kQuelleB, state::Beziehungsart::fuehrtVor), …
>     const auto revVorher = p->intentBestandRevision();
>     pruefe (! p->setzeQuellenbeziehung (kQuelleC, kQuelleA, state::Beziehungsart::fuehrtVor), …
>     pruefe (p->intentBestandRevision() == revVorher,
>             "und ein abgewiesener Wert hebt die Bestandsrevision nicht");

`:619-633` (M-12 nennt `:623-632`): `auto p = mainProzessor();` …
`const auto revNachUser = p->intentBestandRevision();` …
`pruefe (p->intentBestandRevision() == revNachUser, …)`.

`:740-761` (M-12 nennt `:747-759`): `auto p = mainProzessor();` …
`const auto revision = p->intentBestandRevision();` … Save, neue Instanz,
`pruefe (q->intentBestandRevision() == revision, …)`.

**Gemessen:** alle drei bestehenden Fälle laufen **über den Prozessor**
(`mainProzessor()`, `p->setzeQuellenbeziehung`, `p->intentBestandRevision()`),
aber **keiner** von ihnen hängt einen `DirtyZaehler` an. Die Host-Dirty-Messung
des Basisstands liegt an einer anderen Stelle derselben Datei,
`3cfa4176:…/Sonde014IntentTest.cpp:842-892`:

>     // 5. Host-Dirty: jede persistente Aenderung meldet GENAU einmal, ein
>     //    No-op und ein abgewiesener Wert melden nichts (M-13).
>     auto p = mainProzessor();
>     DirtyZaehler dirty;
>     p->addListener (&dirty);
>     … p->setzeQuellenrolle … dirty.nonParam == 1 …
>     … p->schuetzeQuelle … dirty.nonParam == 3 …
>     … p->setzeQuellenbeziehung … dirty.nonParam == 4 …
>     … p->entferneQuellenrolle … dirty.nonParam == 5 …
>     … p->hebeQuellenschutzAuf … dirty.nonParam == 6 …
>     … p->entferneQuellenbeziehung … dirty.nonParam == 7 …

Dieser Block misst **alle** Handgriffe mit Listener — aber am Normalstand, nie
an einem Rand. Der Zähler selbst ist `3cfa4176:…:82-90`
(`struct DirtyZaehler final : public juce::AudioProcessorListener`, Feld
`nonParam`); am Stand `988dc44d` liegt er unverändert bei `:87-90`.

### 1.6 Was der gebaute Test am Stand `988dc44d` misst

`git show 988dc44d:eq-copilot/plugin/tests/Sonde014IntentTest.cpp:1470-1551`,
Funktion `m283_12`. Der Grenzstand entsteht `:1474-1475`:

>     const auto writer = writerMitBestand();
>     const auto knappDrunter = mitMainFeld (writer, "intent_revision_v1", juce::var (kMax - 1));

Die vier Blöcke `:1477-1521` laden diesen Stand **ins Modul**, nicht in einen
Prozessor — `state::Zustand geladen (const juce::MemoryBlock&, const char*)`
(`:1282-1290`) ruft `state::lade (…)` ohne Prozessor. Danach je Block:

>     const bool ok = state::entferneIntent (z, kQuelleA, {}, veraendert, grund);          // :1483
>     const bool ok = state::entferneSchutzangabe (z, kQuelleA, …, veraendert, grund);     // :1493-1494
>     const bool ok = state::entferneBeziehung (z, kQuelleA, kQuelleB, veraendert, grund); // :1503
>     const bool ok = state::assistentUeberspringen (z, veraendert, grund);                // :1516

Gemessen wird je Block ausschließlich `ok`, `veraendert`, `grund`, der
Containerinhalt und `z.intentBestandRevision` bzw. `z.assistent.revision`
(`:1484-1487`, `:1495-1497`, `:1504-1506`, `:1517-1520`). **Kein
`mainProzessor()`, kein `DirtyZaehler`, kein `addListener` in diesen vier
Blöcken** — geprüft über die vollständige Trefferliste von `p->`,
`mainProzessor` und `DirtyZaehler` im Bereich `:1230-1552`: die einzigen
Treffer liegen in den Hilfen `writerMitBestand` (`:1260-1269`),
`writerMitAssistent` (`:1272-1279`) und im Produktpfad-Block ab `:1526`.

Der Dirty-Block `:1523-1550` wörtlich (Kopf, `:1523-1533`):

>     // Und der Weg ueber den PRODUKTPFAD: ein echter Handgriff meldet genau
>     // einmal Host-Dirty (`nakama-state-v2.md:139`).
>     {
>         auto p = mainProzessor();
>         DirtyZaehler dirty;
>         p->addListener (&dirty);
>         pruefe (p->setzeQuellenrolle (kQuelleA, {}, state::Rolle::fuehrt,
>                                       state::IntentHerkunft::user, 1.0),
>                 "M-12 Produktpfad: ein Intent wird gesetzt");
>         const auto dirtyNachSetzen = dirty.nonParam;
>         const auto revNachSetzen = p->intentBestandRevision();

`mainProzessor()` (`:162-168`) setzt einen **frischen** Prozessor auf die
Saat `mainBaum()`:

>     std::unique_ptr<EqCopilotProcessor> mainProzessor()
>     {
>         auto p = std::make_unique<EqCopilotProcessor>();
>         const auto saat = alsBlock (mainBaum());
>         p->setStateInformation (saat.getData(), (int) saat.getSize());
>         return p;
>     }

`knappDrunter` wird diesem Prozessor **nie** übergeben. Der Block misst
anschließend (`:1534-1548`) `entferneQuellenrolle` einmal mit Wirkung und
einmal folgenlos. Gemessene Revisionen: `setzeQuellenrolle` hebt von 0 auf 1,
`entferneQuellenrolle` von 1 auf 2 — belegt durch die Rohausgabe des
Rotbeweises, die den gemessenen Wert `[1]` als Delta führt
(`docs/beweise/roh/NAK-283-rot-M-12-etappe-2.txt:306`, `:608`).

### 1.7 Wo Host-Dirty im Produktpfad entsteht

`git show 988dc44d:eq-copilot/plugin/src/prozessor/State.cpp:509-524`:

>     bool EqCopilotProcessor::entferneQuellenrolle (…)
>     {
>         bool veraendert = false;
>         {
>             std::lock_guard<std::mutex> l (bindungMutex);
>             …
>             if (! nakama::state::entferneIntent (zustand, quelleId, passageId, veraendert, grund))
>                 return false;
>         }
>         if (veraendert)
>         {
>             meldeHostDirty();
>             v3StateRevision.fetch_add (1);

Gleich gebaut: `hebeQuellenschutzAuf` (`:563-580`, ruft
`state::entferneSchutzangabe`, dann `meldeHostDirty()` `:579`) und
`entferneQuellenbeziehung` (`:627-642`, ruft `state::entferneBeziehung`, dann
`meldeHostDirty()` `:641`). **Host-Dirty existiert nur im Prozessor** — die
`state::`-Funktionen kennen ihn nicht; sie liefern `veraendert`, und erst der
Prozessor macht daraus eine Meldung. Der Riegel selbst steht in
`988dc44d:eq-copilot/plugin/state/NakamaState.cpp:2207-2216`:

>     bool bestandsrevisionHeben (Zustand& z, juce::String& grund)
>     {
>         if (z.intentBestandRevision >= std::numeric_limits<juce::int64>::max())
>         {
>             grund = "intent revision would overflow";
>             return false;
>         }
>         ++z.intentBestandRevision;
>         return true;
>     }

und wird in `entferneIntent` (`:2295-2304`), `entferneSchutzangabe`
(`:2355-2362`) und `entferneBeziehung` (`:2471-2481`) jeweils **vor** dem
`erase` und **vor** `veraendert = true` gerufen.

### 1.8 Was M-11 zusätzlich misst

`git show 988dc44d:eq-copilot/plugin/tests/StateMigrationTestMain.cpp:2366-2454`.
Dort läuft der **Grenzstand durch den Prozessor mit Listener** (`:2397-2408`):

>         auto p = std::make_unique<EqCopilotProcessor>();
>         DirtyZaehler dirty;
>         p->addListener (&dirty);
>         p->setStateInformation (amRand.getData(), (int) amRand.getSize());
>         …
>         const auto dirtyNachLaden = dirty.nonParam;

Gemessen werden zwei Handgriffe an `int64max`: `p->assistentUeberspringen()`
(`:2411`) und `p->entferneQuellenrolle (qa, {})` (`:2420`), mit der Zusage
(`:2428-2430`):

>         pruefe (dirty.nonParam == dirtyNachLaden,
>                 "M-11: kein abgewiesener Handgriff meldet Host-Dirty",
>                 juce::String (dirty.nonParam - dirtyNachLaden));

`p->hebeQuellenschutzAuf` und `p->entferneQuellenbeziehung` kommen in diesem
Block nicht vor (vollständige Trefferliste `p->` und `neu->` über `:2366-2454`
gelesen).

### 1.9 Der Rotbeweis M-12 — welche Zeilen unter der Mutation fallen

`docs/beweise/roh/NAK-283-rot-M-12-etappe-2.txt:1-8`:

> NAK-283 Etappe 2 - Rotbeweis MUT-J fuer M-12 (Regressionswache)
> Mutation:   NakamaState: die Bestandsschranke auf int64max - 1 vorziehen -
> der Normalfall unter der Grenze wird abgewiesen
> Urteil:     ROT Exit 1 / GRUEN Exit 0

Der mutierte Lauf, `:293-310`:

>   FEHLER  M-12: der Intent wird entfernt, und die Revision steigt um GENAU 1  [9223372036854775806]
>   FEHLER  M-12: die Schutzangabe wird entfernt, Revision +1
>   FEHLER  M-12: die Beziehung wird entfernt, Revision +1
>   ok      M-12: unter_der_grenze_aendert_sich_nichts - der Assistentenschritt wechselt, und die Revision steigt um GENAU 1  [9223372036854775807]
>   ok      M-12 Produktpfad: GENAU eine Dirty-Meldung und GENAU eine Revision je persistenter Aenderung  [1]
>   ok      M-12 Produktpfad: und er schweigt - kein Dirty, keine Revision  [1]
> SONDE-014-INTENT-TEST FEHLGESCHLAGEN - 3 Fehler, 251 ok

Die **drei** Fehler liegen ausschließlich in den `state::`-Blöcken. Der
Dirty-Block bleibt unter der Mutation, die die Zeile trägt, **grün**.

### 1.10 Was das Manifest über die Messung behauptet

`docs/beweise/NAK-283.md:2743` (§20.2), Zeile M-12: „**Regressionswache**:
`Sonde014IntentTest.cpp:1470` `m283_12` (vier Handgriffe plus Produktpfad)" —
B27 — „`NakamaState.cpp:2209`: Bestandsschranke auf `int64max − 1`
vorgezogen" — `NAK-283-rot-M-12-etappe-2.txt` — Exit 1 / Exit 0.

`docs/beweise/NAK-283.md:2840` (§20.5, Zeile „Dirty je persistenter
Änderung"):

> M-01 (Delta 2 statt 1), M-05 (Delta 2 trotz inhaltsgleicher Publikation),
> M-11 (kein Dirty bei abgewiesenem Handgriff), M-12 Produktpfad (genau eine
> Meldung, folgenloser Aufruf schweigt), M-72 (nur der Reloadfall unterdrückt).

Die Zeile nennt den Produktpfad, **nennt aber nicht**, dass er am Normalstand
läuft, während die Vorbedingungsspalte der Matrixzeile `int64max − 1` fordert.

### 1.11 Vertrag und Grundgesetz

`eq-copilot/schemas/state/nakama-state-v2.md:139`, Zelle „Regeln" wörtlich:

> Revision des **ganzen** Bestands, steigt bei jeder persistenten Änderung
> genau einmal. Fehlt sie, ist der Bestand nie beschrieben worden; **Inhalt
> ohne Revision** ist ein ungültiger Stand, weil die Vollständigkeitsmarke aus
> SONDE-014 M-86 dann keine Zahl hätte.

`:136` (`source_intents_v1`) trägt in derselben Spalte „`revision` `int64`
≥ 1".

`CLAUDE.md:195-197`:

> - **State bleibt verlustfrei.** Unbekannte Major-Versionen oder unzulässige
>   Matrizen werden read-only mit Originalbytes gehalten; jede persistente
>   Änderung meldet dem Host Dirty-State.

---

## 2. Ist jede Teilbehauptung am Stand `988dc44d` wahr?

| # | Teilbehauptung des Befunds | Urteil | Beleg |
|---|---|---|---|
| 1 | Die vier MAX−1-Fälle prüfen ausschließlich `state::…`, ohne Prozessor und ohne Hostlistener | **wahr** | `Sonde014IntentTest.cpp:1477-1521`; die einzigen `p->`- und `DirtyZaehler`-Treffer im Bereich `:1230-1552` liegen in `writerMitBestand`, `writerMitAssistent` und ab `:1526` |
| 2 | Der Dirty-Block `:1526-1530` lädt keinen Grenzstand | **wahr** | `mainProzessor()` `:162-168` setzt die Saat `mainBaum()`; `knappDrunter` (`:1475`) wird ihm nie übergeben |
| 3 | Der Dirty-Block misst nur Intent bei Revision 1 nach 2 | **wahr** | `:1529-1548` ruft nur `setzeQuellenrolle` und `entferneQuellenrolle`; Delta `[1]` in der Rohausgabe `:306` und `:608` |
| 4 | Er misst nicht alle vier Handgriffe bei MAX−1 | **wahr** | weder Schutz noch Beziehung noch Assistent kommen im Block vor; MAX−1 kommt im Block nicht vor |
| 5 | Die MAX-Ablehnungen für Schutz und Beziehung (M-08/M-09) haben keine Hostmessung | **wahr** | `:1334-1371` ohne Prozessor; `hebeQuellenschutzAuf` und `entferneQuellenbeziehung` fehlen auch in `StateMigrationTestMain.cpp:2366-2454` |
| 6 | M-11 ergänzt dort nur Intent und Assistent | **wahr** | `StateMigrationTestMain.cpp:2411`, `:2420`, Zusage `:2428-2430` |
| 7 | Die Zeilenangaben des Befunds stimmen | **wahr** | `:1470` ist `void m283_12()`, `:1526-1530` der Kopf des Dirty-Blocks, `:1549` das `removeListener`; `CLAUDE.md:195-197` trägt den zitierten Satz |
| 8 | „Die Grenzstände müssen zusätzlich durch die Prozessorhandgriffe mit Listener laufen" (die Forderung) | **teils** — für M-12 von der Matrix gedeckt, für M-08/M-09 nicht wörtlich gefordert | M-12-Zusage `:608` gegen die Spaltenlage von M-07 `:603` (siehe §1.2) |

**Zusatzbefund dieser Validierung, den der Befund selbst nicht nennt** — und
der ihn härter macht, als er behauptet: Die Dirty-Hälfte von M-12 hat **keinen
Rotbeweis**. Die Mutation, die die Zeile trägt (`NakamaState.cpp:2209`,
Bestandsschranke auf `int64max − 1`), lässt den Produktpfad-Block grün:
`NAK-283-rot-M-12-etappe-2.txt:306` und `:308` melden „ok" im **mutierten**
Lauf. Eine Prüfung, die unter der Mutation an der Zusagezeile nicht fällt,
belegt die Zusage dieser Zeile nicht (§6.1 Schritt 4,
`docs/beweise/NAK-283.md:810-811`). Der Rotbeweis der Zeile fällt an drei
Revisionszusagen, nie an der Dirty-Zusage.

**Antworten auf die gestellten Einzelfragen.**

1. *Messen die MAX−1-Fälle in `m283_12` Host-Dirty?* Nein. Sie messen
   ausschließlich Rückgabewert, `veraendert`, `grund`, Containerinhalt und die
   Revisionszahl der `state::`-Funktion.
2. *Was misst der Dirty-Block `:1526-1530` genau?* Einen **Normalstand**
   (frischer Prozessor auf `mainBaum()`), **einen** Handgriff (Intent setzen,
   Intent entfernen, Revision 1 nach 2) und zusätzlich den folgenlosen zweiten
   Aufruf.
3. *Messen M-08/M-09 und M-11 Host-Dirty, für welche Handgriffe?* M-08 und
   M-09 messen keinen Host-Dirty. M-11 misst ihn am Grenzstand `int64max` für
   **zwei** Handgriffe — Assistent (`assistentUeberspringen`) und Intent
   (`entferneQuellenrolle`) — mit der Erwartung **null** Meldungen. Schutz und
   Beziehung fehlen an beiden Rändern.
4. *Wie haben die bestehenden Fälle am Basisstand Host-Dirty gemessen?* Die
   drei von M-12 **namentlich genannten** Fälle (`:565-570`, `:623-632`,
   `:747-759`) messen Host-Dirty **gar nicht**; sie messen
   `p->intentBestandRevision()` über den Prozessor. Die Host-Dirty-Messung des
   Basisstands liegt in einem anderen Block derselben Datei (`:842-892`) und
   misst dort **alle** Handgriffe mit Listener — am Normalstand.
5. *Verlangt M-12 eine Hostmessung am Grenzstand?* Ja. „Host-Dirty wird genau
   einmal gemeldet" steht in der **Zusagespalte**, unter der Vorbedingung
   „`int64max − 1` … alle vier Handgriffe". Host-Dirty ist nur über den
   Prozessor beobachtbar (`State.cpp:521-523`, `:577-579`, `:639-641`); eine
   Messung ohne Prozessor kann diese Zusage nicht prüfen.

---

## 3. Welcher Satz bricht

**Es bricht kein Satz des Vertrags und kein Satz von `CLAUDE.md`.** Der
Produktcode erfüllt sie: `bestandsrevisionHeben` steht vor jeder Mutation, und
`meldeHostDirty()` hängt in allen drei Entfern-Handgriffen an `veraendert`
(`State.cpp:521-523`, `:577-579`, `:639-641`) — genau einmal je wirksamer
Änderung, nie bei einer Ablehnung. `nakama-state-v2.md:139` und
`CLAUDE.md:195-197` sind am Stand `988dc44d` **implementiert**; F11 ist als
Verhalten geschlossen.

**Es bricht ein Satz der Matrixzeile M-12** — als Messabdeckung, nicht als
Verhalten:

> Der Eintrag wird entfernt, die Revision steigt um genau 1, Host-Dirty wird
> **genau einmal** gemeldet

unter der Vorbedingung

> `intentBestandRevision == int64max − 1` bzw. Assistentenrevision
> `int64max − 1`; alle vier Handgriffe

Die ersten beiden Glieder sind bei `int64max − 1` für alle vier Handgriffe
gemessen. Das dritte Glied ist bei `int64max − 1` für **keinen** Handgriff
gemessen; es ist nur an einem Normalstand für **einen** Handgriff gemessen.

Die richtige der drei angebotenen Kategorien ist damit **„Der Test misst die
Zusage nicht"**: die Matrixzeile verlangt die Hostmessung am Grenzstand
ausdrücklich, und sie ist dort nicht vorhanden. Es ist **nicht** „gemessen,
nur an anderer Stelle" — die andere Stelle (§1.5, `:842-892` am Basisstand;
`:1526-1550` am Etappenstand) misst einen anderen Zustand, und der Zustand ist
in F11 genau das Prüfmerkmal. Es ist auch **nicht** „die Matrix verlangt es
nicht" — sie verlangt es in der Zusagespalte.

**Abgrenzung für M-07 bis M-09:** Dort steht „kein Host-Dirty" in der Spalte
*Reihenfolge und Frist*, also als Mechanismus, an dem die Zusage hängt, nicht
als eigener prüfbarer Satz (§1.2). Für diese drei Zeilen ist die fehlende
Hostmessung bei `int64max` deshalb eine **Lücke**, kein gebrochener Satz —
zumal M-11 dieselbe Nullmeldung für zwei der vier Handgriffe am selben
Grenzstand bereits misst und der Code für Schutz und Beziehung strukturgleich
ist.

---

## 4. Einordnung

**DEFEKT** — begrenzt auf M-12, mit einer angehängten **LÜCKE** bei M-08/M-09.

Die Zusagespalte von M-12 nennt Host-Dirty als eine ihrer drei Größen und
bindet sie an den Grenzstand `int64max − 1` und an alle vier Handgriffe; der
gebaute Fall misst diese Größe nirgends am Grenzstand und nur für einen von
vier Handgriffen überhaupt, weil die vier Grenzblöcke unter den
`state::`-Funktionen laufen, die Host-Dirty gar nicht kennen. Dass die Lücke
nicht kosmetisch ist, zeigt der eigene Rotbeweis der Zeile: unter der Mutation,
die M-12 trägt, bleibt der gesamte Dirty-Block grün
(`NAK-283-rot-M-12-etappe-2.txt:306`, `:308`) — die Dirty-Zusage von M-12 ist
damit eine Behauptung ohne Falsifikation, was `tools/dirigent/pruefliste.md` E
und §6.1 Schritt 4 ausschließen. Die fehlende Hostmessung für Schutz und
Beziehung bei `int64max` (M-08/M-09) ist dagegen nur eine Lücke, weil ihre
Zusagespalte Host-Dirty nicht führt; sie gehört trotzdem in denselben
Änderungssatz, weil M-11 die identische Messung für die anderen beiden
Handgriffe bereits fährt und die Asymmetrie sonst ungeprüft bleibt.

---

## 5. Die schließende Regel

Kein Codevorschlag; nur, was messbar sein muss, damit die Zeilen ihre Zusagen
tragen.

**a) Der Weg.** Beide Grenzstände sind bereits als `juce::MemoryBlock`
vorhanden und damit ohne neue Bühne prozessorfähig: `knappDrunter`
(`Sonde014IntentTest.cpp:1475`) und `amRand` (`:1308`) entstehen als
deklarierte Mutanten eines Writer-Standes über `mitMainFeld` und
`mitAssistentenPlatz` (`:1236-1257`) und gehen genauso in
`setStateInformation` wie in `StateMigrationTestMain.cpp:2400` bereits
vorgeführt. Die Messung läuft über einen Prozessor mit angehängtem
`DirtyZaehler` (`:87-90`) und den **Produkthandgriffen**, nicht über die
`state::`-Funktionen: `entferneQuellenrolle`, `hebeQuellenschutzAuf`,
`entferneQuellenbeziehung`, `assistentUeberspringen`. Die Fixture-Regel §6.2
bleibt gewahrt — es entsteht keine neue Datei, nur ein zweiter Träger
desselben deklarierten Mutanten.

**b) Was bei `int64max − 1` messbar sein muss (M-12).** Je Handgriff, jeweils
aus einem frisch in den Prozessor geladenen Grenzstand:

| Handgriff (Produktpfad) | Vorzustand | Erwartung |
|---|---|---|
| `entferneQuellenrolle` | Bestandsrevision `int64max − 1`, ein passender Intent | Rückgabe `true`, Eintrag weg, Bestandsrevision `int64max`, **genau eine** Dirty-Meldung |
| `hebeQuellenschutzAuf` | Bestandsrevision `int64max − 1`, eine passende Schutzangabe | Rückgabe `true`, Angabe weg, Bestandsrevision `int64max`, **genau eine** Dirty-Meldung |
| `entferneQuellenbeziehung` | Bestandsrevision `int64max − 1`, eine passende Beziehung | Rückgabe `true`, Kante weg, Bestandsrevision `int64max`, **genau eine** Dirty-Meldung |
| `assistentUeberspringen` | Assistentenrevision `int64max − 1`, offener Schritt | Rückgabe `true`, Schritt gewechselt, Assistentenrevision `int64max`, **genau eine** Dirty-Meldung |

Dazu, je Handgriff im selben Block, der folgenlose Zweitaufruf: Rückgabe
`true`, Revision unverändert, **null** weitere Dirty-Meldungen — damit die
vorgezogene Vorprüfung den No-op-Pfad nachweislich nicht verschoben hat
(`NakamaState.cpp:2297-2298`, `:2355-2357`, `:2474-2475`).

**c) Was bei `int64max` messbar sein muss (M-07 bis M-09, Ergänzung zu M-11).**
Je Handgriff aus einem Grenzstand `int64max`: Rückgabe `false`, Container
unverändert, Revision unverändert und **null** Dirty-Meldungen — für **Schutz**
und **Beziehung**, die M-11 heute nicht führt. Intent und Assistent sind durch
`StateMigrationTestMain.cpp:2428-2430` bereits abgedeckt; die Ergänzung
schließt die Asymmetrie, ohne M-11 anzufassen.

**d) Wie der Rotbeweis dann fällt.** Die heutige Mutation der Zeile M-12 —
Bestandsschranke in `bestandsrevisionHeben` (`NakamaState.cpp:2209`) auf
`int64max − 1` vorgezogen — lässt den Riegel bei `int64max − 1` bereits
scheitern. `entferneIntent`, `entferneSchutzangabe` und `entferneBeziehung`
kehren dann mit `false` und `veraendert == false` zurück, der Prozessor
überspringt seinen `if (veraendert)`-Block und **meldet kein Dirty**
(`State.cpp:521-523`, `:577-579`, `:639-641`). Mit der Messung aus (b) fällt
derselbe Rotbeweis damit **zusätzlich** an der Dirty-Erwartung — erwartet 1,
gemessen 0 — für die drei Bestandshandgriffe, also an genau der Zeile, die die
Zusage trägt, statt nur an den Revisionszahlen. Für den Assistentenzweig fällt
die Dirty-Erwartung erst unter der spiegelbildlichen Mutation an
`assistentenrevisionHeben` (`NakamaState.cpp:2677`); ob M-12 dafür einen
zweiten Mutanten bekommt oder der Assistentenzweig ausdrücklich als von MUT-J
unberührt bezeichnet wird, ist eine Entscheidung des Bauers — belegt sein muss,
dass **jede** Größe der Zusagespalte mindestens einmal fällt. Für (c) fällt der
bestehende M-07-Mutant (Revisionsprüfung hinter das `erase` zurück): dann
entfernt der abgewiesene Handgriff den Eintrag, `veraendert` bleibt `false`,
der Prozessor schweigt — und die Zusage „eine persistente Änderung ohne Dirty
gibt es nicht" ist am Grenzstand für alle vier Handgriffe falsifizierbar statt
für zwei.

---

## 6. Scope-Beweis — gelesene Dateien mit Zeilenbereichen

Nur gelesen; nichts gebaut, nichts gefahren, nichts geändert außer dieser
Datei.

| Datei | Stand | Zeilen |
|---|---|---|
| `docs/beweise/NAK-283.md` | Arbeitsbaum (= `92386c37`) | 383-400 (§4 F11), 496-612 (§5 Kopf und §5.1, Zeilen M-01 bis M-12, M-71, M-72), 793-936 (§6.1, §6.2, §6.3), 1597-1740 (§8.1, Feinheiten 1 bis 14), 2659-2930 (§20.1 bis §20.7) |
| `docs/beweise/roh/NAK-283-rot-M-12-etappe-2.txt` | Arbeitsbaum | 1-8 (Kopf, Hashes, Urteil), 293-310 (mutierter Lauf), 595-612 (grüner Lauf) |
| `eq-copilot/plugin/tests/Sonde014IntentTest.cpp` | `988dc44d` | 87-90, 155-185, 1230-1552 |
| `eq-copilot/plugin/tests/Sonde014IntentTest.cpp` | `3cfa4176` | 80-110, 550-580, 610-640, 735-765, 835-930 |
| `eq-copilot/plugin/tests/StateMigrationTestMain.cpp` | `988dc44d` | 2335-2460 |
| `eq-copilot/plugin/state/NakamaState.cpp` | `988dc44d` | 2200-2220, 2275-2315, 2348-2375, 2460-2488 |
| `eq-copilot/plugin/src/prozessor/State.cpp` | `988dc44d` | 505-530, 560-585, 625-648 |
| `eq-copilot/schemas/state/nakama-state-v2.md` | Arbeitsbaum | 130-145 |
| `CLAUDE.md` | Arbeitsbaum | 186-202 |
| `docs/beweise/roh/NAK-283-erstpruefung-2-validierung-1.md` | Arbeitsbaum | 1-45 (nur die Form dieser Reihe) |

Zusätzlich als Trefferlisten gelesen, ohne weitere Dateiinhalte: `git grep -n`
über `entferneIntent`, `entferneSchutzangabe`, `entferneBeziehung` und
`meldeHostDirty` in `988dc44d -- eq-copilot/plugin/src`; `git diff --stat
988dc44d HEAD`; `git diff --stat HEAD -- eq-copilot/`.
