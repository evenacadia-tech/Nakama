# G4 — Quellencheck der 15 HIGH/MEDIUM-Befunde

Basis: HEAD `9f3000881df632f4969d78551fb0a516b61288cb`, Worktree sauber, strikt lesend.
Geprüft wurde jeweils die Quelle samt Aufrufern, nicht die Part-Datei.

**Vorbemerkung zu den Zeilennummern der Gates.** Der Auftrag nennt Gate 5/6 bei
`docs/FL-Nakama-Sonden-Design-Entwurf.md:3546-3558`; an der Quelle stehen sie eine
Zeile weiter: Gate 5 in `:3559`, Gate 6 in `:3560`, der Abschnittskopf §49.2 in `:3546`.
§48.4 endet mit dem Threat-Model-Satz in `:3513-3515`. Alle Zitate unten stammen aus
diesen Zeilen.

**Vorbemerkung zum Experimentpfad (V01–V03).** `beginneVersuch`, `erfasseKandidat`,
`urteileVersuch`, `brichVersuchAb` und `merkeManuellePassage` haben heute **keinen
Aufrufer im Editor**: die Suche über `eq-copilot/plugin` findet sie nur in
`src/PluginProcessor.{h,cpp}` und in `tests/Sonde013ExperimentUiTest.cpp` /
`tests/Sonde013PassageStateTest.cpp`; `src/PluginEditor.cpp` (1665 Zeilen) enthält
weder „Versuch" noch „Passage". Die drei Defekte sind damit **verdrahtet, aber noch
nicht bedienbar** — sie werden in dem Moment scharf, in dem die Gen-Fläche den
Handgriff bekommt. Das ist die ehrliche Schranke für (b) und für die Dringlichkeit,
kein Grund, sie zu entschärfen.

---

## V01 — `erfasseKandidat` sendet einen Lautheitsabgleich, den es nicht gab

### (a) BESTÄTIGT

- `eq-copilot/plugin/src/PluginProcessor.cpp:1937` — `versuchReferenzJson` schreibt
  `match_gain_db` aus `zahl (vergleichspegel.gainDb())` **unbedingt**, ohne
  `gainGesetzt()` zu fragen.
- `:1946-1947` — `nicht_endliche_samples` kommt aus dem **LIVE**-Zähler
  `vergleichspegel.nichtEndlicheSamples()`, nicht aus dem bei Versuchsbeginn
  eingefrorenen `versuchNichtEndlich` (`:2023`).
- `:2026` — der Riegel, den `beginneVersuch` hat:
  `if (! eingefroren || ! vergleichspegel.gainGesetzt()) return false;`.
- `:2075-2104` — `erfasseKandidat` hat diesen Riegel nicht; es prüft nur
  `versuchIdAktiv`, bindet die Blindreihenfolge und ruft
  `versuchReferenzJson (engineabzugLesen())`.
- `:211` — `prepareToPlay` ruft `vergleichspegel.vorbereiten (sichereSamplerate)`.
- `:1817` — `bindePassagenfensterMitEpoche` ruft
  `vergleichspegel.vorbereiten (getSampleRate())`; beide Wege landen in
  `core/analysis/Vergleichspegel.h:66-72` → `leerenIntern()`.
- `core/analysis/Vergleichspegel.h:323-332` — `leerenIntern()` setzt `gehalten = 0.0`,
  `gehaltenGesetzt = false`, `nichtEndlich = 0`.
- `core/analysis/Vergleichspegel.h:29-31` — „Eine gemeldete 0 dB wäre die Behauptung
  ‚die beiden sind gleich laut' — ‚nie gemessen' ist etwas anderes."
- `core/analysis/Vergleichspegel.h:134-135` — „0 heisst nachweislich keines, nicht
  ‚nicht gemessen'."
- **Schranke, die der Leser selbst nennt und die stimmt:**
  `core/analysis/Blindvergleich.h:95` kopiert `gainDb` als Wert in
  `uebernimmVergleichspegel`; der **Urteilspfad** (`urteileVersuch:2106`) ist deshalb
  nicht betroffen. Falsch ist nur die Referenz **auf der Leitung** im
  `experiment_candidate`.

### (b) Erreichbar

Heute **nicht im Normalbetrieb**, weil `erfasseKandidat` keinen UI-Aufrufer hat
(siehe Vorbemerkung). Sobald er existiert, ist die Kette rein bedienend und braucht
keinen Angreifer:

- (a) Host → `EqCopilotProcessor::prepareToPlay` (`:180`) → `vergleichspegel.vorbereiten`
  (`:211`); ausgelöst durch Sampleraten- oder Blockgrößenwechsel oder Resume nach
  Suspend. Danach Editor → `erfasseKandidat` (`:2075`) → `versuchReferenzJson` (`:2090`)
  → `sendeVersuchP0` (`:2103`) → `controlV3.sendePersistenzP0`.
- (b) Editor → `merkeManuellePassage` (`:1648`, Passage B) → `bindePassagenfenster`
  (`:1680`) → `bindePassagenfensterMitEpoche` (`:1817`) — bei laufendem Versuch auf
  Passage A. Keiner der beiden Wege schließt den offenen Versuch.

### (c) Welcher Satz bricht

**M-43** (`docs/beweise/SONDE-013.md:533`), wörtlich:

> „Der Match-Gain wird für die Passage **vorab** gemessen und für die Dauer des
> Versuchs eingefroren; er ist Teil der unveränderlichen Referenzen. Eine
> Klangwertung ohne vorherigen Lautheitsabgleich ist unzulässig."

Zusätzlich **M-07** (`:476`): „Der Zähler wird ausgewertet, nicht ignoriert." — der
Nichtendlich-Zähler reist hier frisch genullt und behauptet damit „nachweislich
keines". Ebenfalls berührt: CLAUDE.md-Invariante **„NaN-Ehrlichkeit. Nicht-endliche
Werte werden verriegelt und gezählt"**.

### (d) Register

**Neu.** `docs/offene-punkte.md` führt zum Experimentpfad NAK-158 (Pegelfenster an der
Transportkante), NAK-159 (halber Stereoblock in `friereEin`), NAK-162
(Upstream-Fingerprint), NAK-166 (Outbox-Key), NAK-169 (Zustand vor dem ACK) — keine
dieser Zeilen nennt die fehlende `gainGesetzt()`-Prüfung in `versuchReferenzJson`.

### (e) Fixgröße — klein

- `src/PluginProcessor.cpp:1901-1951` (~10 Zeilen): `versuchReferenzJson` bekommt
  denselben Riegel wie `beginneVersuch` (leerer String bei fehlendem `gainGesetzt()`),
  und `nicht_endliche_samples` liest `versuchNichtEndlich` statt des Live-Zählers.
- `:2075-2104` (~4 Zeilen): `erfasseKandidat` gibt `false` zurück, sobald die Referenz
  leer ist — das tut es automatisch, wenn `versuchReferenzJson` fail-closed wird.
- Test: **neu** in `eq-copilot/plugin/tests/Sonde013PassageStateTest.cpp` —
  `kandidat_ohne_eingefrorenen_pegel_entsteht_nicht` (Versuch beginnen →
  `prepareToPlay` mit anderer Samplerate → `erfasseKandidat` muss `false` liefern, und
  der letzte Wirezustand darf kein `match_gain_db` mit 0 tragen). Nachbar zum Anlehnen:
  `Sonde013PassageStateTest.cpp:1035-1050` (C5-Fall).

---

## V02 — `beginneVersuch` schaltet die Pegelspeisung ab und auf dem Fehlerweg nie wieder an

### (a) BESTÄTIGT

- `src/PluginProcessor.cpp:2022` — `versuchspegelSpeist.store (false, …)` **vor**
  `friereEin()`.
- `:2023-2024` — `versuchNichtEndlich.store (vergleichspegel.nichtEndlicheSamples(), …)`.
- `:2026-2027` — `if (! eingefroren || ! vergleichspegel.gainGesetzt()) return false;` —
  Rückkehr ohne Wiederanschalten.
- `core/analysis/Vergleichspegel.h:203-208` — im Zweig „zu wenig Material" friert
  `friereEin()` **absichtlich nicht** ein („ein gesperrter Zustand ohne Wert ist
  ehrlich"), ist also auf einen Retry ausgelegt.
- `src/PluginProcessor.cpp:1819` — **einziger** Setzer
  `versuchspegelSpeist.store (true, …)`, in `bindePassagenfensterMitEpoche`. Die Suche
  über `eq-copilot/plugin` liefert nur `:506`, `:1819`, `:1866`, `:2022`.
- `:506-507` — der Audiothread speist nur bei `versuchspegelSpeist && pegelFensterAktiv`;
  ohne das erste Bit wächst `gesehen` nie über `mindestSamples`, und jeder Folgeaufruf
  scheitert erneut an derselben Stelle.
- `:2012-2016` — der einzige Ausweg innerhalb von `beginneVersuch` ist der Zweig
  „Passage NICHT gebunden", der `bindePassagenfenster` ruft. Bei gebundener Passage
  wird er nie erreicht.
- **Präzisierung gegen den Part:** „dauerhaft tot" gilt für die gebundene Passage bei
  wiederholtem Druck auf denselben Handgriff. Ein Rückweg existiert nur über einen
  fachfremden Umweg: `vergissManuellePassage` + `merkeManuellePassage`
  (`:1686-1714`/`:1648`) oder das Binden einer fremden Passage und Zurückwechseln.
  `brichVersuchAb` (`:2164`) hilft nicht — `versuchIdAktiv` ist nach dem gescheiterten
  Begin leer. Deshalb **BESTÄTIGT mit dieser Schranke**: der Handgriff selbst hat
  keinen Rückweg.

### (b) Erreichbar

Reine Bedienfolge, kein Angreifer — sobald der Handgriff existiert. Auslöser ist der
Normalfall „markieren und sofort starten" oder „markieren bei stehendem Transport":
weniger als `Vergleichspegel::kMindestSekunden` = 0,4 s Material im Fenster.
Kette: Editor → `beginneVersuch` (`:1975`) → `:2022` → `:2026`.

### (c) Welcher Satz bricht

**Kein Satz** aus den fünf genannten Quellen. M-40/M-43 sind fail-closed erfüllt: ohne
Lautheitsabgleich entsteht korrekt kein Versuch. Es bricht die
CLAUDE.md-**Wahrheitskern**-Zeile „Keine toten UI-Elemente. Jedes sichtbare Element
führt einen Handgriff aus oder meldet ehrlich einen Zustand." — die steht ausdrücklich
**nicht** im Abschnitt „Tragende technische Invarianten"; „aktivieren↔abklingen" ist
Arbeitsregel, nicht Invariante. Der Code benennt den Fehlertyp bei `:2152-2159` selbst:
„Ein Handgriff, der beim zweiten Mal stumm nicht mehr geht, ist derselbe Fehler wie ein
totes Element." ⇒ Einstufung: **Lücke/Härtung mit Produktwirkung**, kein Vertragsbruch.

### (d) Register

**Neu.** Keine Zeile in `docs/offene-punkte.md` nennt `versuchspegelSpeist` oder den
Fehlerweg von `beginneVersuch`.

### (e) Fixgröße — klein

- `src/PluginProcessor.cpp:2020-2030` (~6 Zeilen): auf dem Zweig „zu wenig Material"
  (also wenn `vergleichspegel.eingefroren()` danach **falsch** ist)
  `versuchspegelSpeist` wieder auf `true` setzen. Der Fall `nichtEndlich > 0` bleibt
  bewusst Endzustand (`Vergleichspegel.h:187-201`).
- Test: **neu** in `Sonde013PassageStateTest.cpp` —
  `zweiter_versuch_nach_zu_wenig_material_gelingt` (Passage binden, `beginneVersuch`
  ohne 400 ms → `false`, weiter speisen, `beginneVersuch` erneut → `true`). Messbar
  über `versuchAufgenommeneBloecke()` (`:1852-1855`), das genau dafür gebaut ist.

---

## V03 — `setStateInformation` lässt Experiment- und Passagenzustand stehen

### (a) BESTÄTIGT

- `src/PluginProcessor.cpp:1404-1408` — geleert werden **nur**
  `ausstehendeSourcesCommands` und `bestaetigteSourcesCommands`.
- `:1410-1446` — der restliche Rumpf tauscht `zustand`, ruft
  `lebenslauf.stateRestauriert`, `spiegleKlassifikation`, `sourcesModel.projektReload`,
  `pipe.start/reconnect`, `v3StateRevision.fetch_add`, `controlV3.reconnect()`. Kein
  Zugriff auf `versuchIdAktiv`, `versuchPassageId`, `gebundenePassageId`,
  `passagenfenster*`, `pegelFensterAktiv`, `versuchspegelSpeist`, `vergleichspegel`
  oder `blindvergleich`.
- `:1953-1963` — `versuchKopfJson` baut den Kopf aus dem **frischen** `v3Hello()`;
  ein danach gesendetes `experiment_candidate` oder `experiment_manual_result` trägt
  die alte `experiment_id` unter der **neuen** `project_binding_id` und `instance_id`.
- `:2291-2294` — der Gegenbeweis, dass die Reload-Grenze anderswo bewusst gezogen ist:
  `wendeBestaetigteSourcesCommandsAn` vergleicht `projectBindingId` **und**
  `v3SessionEpoch` und überspringt ACKs eines vor dem Reload gültigen Laufs.
- `:1832-1843` — `passagenfensterFuehrt(id)` behauptet danach eine Passage, die im
  geladenen State nicht mehr steht.
- **State-Bytes selbst sind nicht betroffen**: `getStateInformation:1389-1393` und der
  `nurLesen`-Zweig `:1410-1429` bleiben verlustfrei; der Befund betrifft ausschließlich
  den Laufzeitzustand daneben.

### (b) Erreichbar

Host-getrieben und damit im Normalbetrieb — aber erst, wenn ein Versuch überhaupt
begonnen werden kann (siehe Vorbemerkung). Kette: FL → `setStateInformation` (`:1395`;
Projektwechsel, Preset-Recall, Copy/Paste der Instanz) → danach Editor →
`erfasseKandidat` (`:2075`) bzw. `urteileVersuch` (`:2106`).

### (c) Welcher Satz bricht

**M-50** (`docs/beweise/SONDE-013.md:540`), wörtlich:

> „Reihenfolge: Store lesen → unveränderliche Referenzen wiederherstellen → offenen
> Zustand anzeigen → **keine** stillschweigende Fortsetzung einer Messung über den
> Neustart hinweg."

Genau das passiert: die Messung (Vergleichspegel im Sample-Fenster des alten Projekts,
offener Versuch) läuft über die Projektgrenze weiter. Sekundär **M-13** (`:488`,
Paarbildung „in derselben Session") als dieselbe Klasse Sitzungsgrenze.

### (d) Register

**Neu.** NAK-169 (`docs/offene-punkte.md:175`) betrifft den ACK-**Zeitpunkt** („Das
Plugin setzt seinen Experimentzustand vor dem Broker-ACK") und sagt dort ausdrücklich
„Keine Zusage bricht" — das gilt für V03 nicht.

### (e) Fixgröße — mittel

- `src/PluginProcessor.cpp:1395-1446` (~20 Zeilen): ein
  `experimentzustandLeeren()`-Zug neben dem Sources-Zug —
  `versuchIdAktiv`/`versuchPassageId` leeren, `blindvergleich.loeschen()`,
  `loesePassagenfenster` für die gebundene Passage bzw. `pegelFensterAktiv` und
  `versuchspegelSpeist` auf `false`, `vergleichspegel.loeschen()`. Beide Zweige
  (`nurLesen` und Vollrestore) brauchen ihn.
- Produktfrage im selben Satz: ein offener Versuch, der beim Reload wegfällt, sollte
  vorher ein `experiment_abort` senden — sonst bleibt er nach M-48 offen und fällt erst
  in die Retention. Gehört ins Ticket, nicht in meine Entscheidung.
- Test: **neu** in `Sonde013PassageStateTest.cpp` —
  `reload_schliesst_passage_und_versuch` (Versuch beginnen, `setStateInformation` mit
  einem anderen Projekt-State, danach `laufenderVersuch().isEmpty()` und
  `passagenfensterFuehrt(alteId) == false`). Nachbar: `Sonde012ProjectReloadTest.cpp`.

---

## V04 — State-Report-Sender verwirft das `sendeP1`-Ergebnis

### (a) BESTÄTIGT

- `eq-copilot/plugin/core/ipc/ControlClient.cpp:1410-1414` — der ganze Block:
  bei `report != letzterStateReport` wird `sendeP1 ("produkt-state-report", report)`
  gerufen und danach `letzterStateReport = report;` **unbedingt** gesetzt; der
  Rückgabewert wird verworfen.
- `core/ipc/ControlClient.cpp:951-970` — `sendeP1` liefert `P1Ergebnis`, u. a.
  `zuGross` (`:958`) und das Ergebnis von `p1.einreihen`.
- `core/ipc/IpcQueues.h:148-153` — die Regel, gegen die verstoßen wird: `p1Uebergeben`
  zählt `abgewiesen` und `zuGross` ausdrücklich **nicht** als übergeben („dort bleibt
  die Quelle stehen"); der Kommentar `:136-147` nennt genau diesen Fehlertyp als schon
  einmal begangen.
- `core/ipc/IpcQueues.h:220-226` — `abgewiesen` entsteht, wenn Hauptqueue (128) und
  Wiederholpuffer (128) voll sind und kein Koaleszenzpartner existiert.
- Gegenprobe im Produkt: `sonde/SondeProcessor.cpp:563-578` hält die Regel ein.

### (b) Erreichbar — schmal

Kette: `Laufzeit::threadLauf` → `eineVerbindung` (Client-Thread) → `:1403-1414`,
im 1-Hz-Heartbeat-Takt. Kein Angreiferpfad nötig, aber ein enges Fenster:

- `zuGross` ist praktisch unerreichbar — der `state_report` ist ein kurzes Objekt
  (`:340-359`), weit unter `kMaxPayloadBytes`.
- `abgewiesen` verlangt 256 anstehende P1-Nachrichten **und** dass der Schlüssel
  `produkt-state-report` weder in `inhalt` noch in `wiederholung` steht — sonst
  koalesziert er (`IpcQueues.h:189-210`) und gilt korrekt als übergeben.
- Ehrliche Schranke, die der Part selbst nennt und die stimmt: `letzterStateReport`
  ist eine Lokale von `eineVerbindung` (`:1311`); der Verlust heilt beim nächsten
  Reconnect, und das sticky `intervention_state_unknown` reist zusätzlich im
  1-Hz-P0-Heartbeat (`:1416-1418`).

### (c) Welcher Satz bricht

**Kein Satz** aus den fünf genannten Quellen. M-05 (`:474`) gilt dem Ereignisring
(„bei Überlast wird die Kadenz reduziert, nie der Ring stillschweigend geleert"),
M-78 (`:615`) der P2-Degradation. Gebrochen wird der **Codevertrag** `p1Uebergeben`
(`IpcQueues.h:148-153`), der zwar für M-05 geschrieben wurde, aber als allgemeine
Senderregel formuliert ist. ⇒ **Härtung mit klarer Hausregel**, kein Vertragsbruch
gegen Entwurf oder Matrix.

### (d) Register

**Neu.** NAK-91 (`docs/offene-punkte.md:132`) betrifft die fehlende
Ende-zu-Ende-Last auf P1, nicht diesen Sender; NAK-95 die Koaleszenz im
Wiederholpuffer.

### (e) Fixgröße — klein

- `core/ipc/ControlClient.cpp:1410-1414` (~3 Zeilen): `letzterStateReport` nur setzen,
  wenn `p1Uebergeben (sendeP1 (…))` gilt.
- Sichtbarkeit gleich mitnehmen (~4 Zeilen): `P1Warteschlange::abgewiesene()`
  (`IpcQueues.h:319`) hat heute keinen Leser — in `Snapshot` heben, sonst bleibt der
  Verlust auch nach dem Fix zählerlos.
- Test: **neu** in `eq-copilot/plugin/tests/IpcTestMain.cpp` —
  `state_report_wird_nach_abweisung_wiederholt` (P1-Queue mit 256 nicht
  koaleszierbaren Ereignissen füllen, State-Report senden, Queue leeren, nächster Takt
  muss denselben Report erneut einreihen).

---

## V05 — `zahlJson` nutzt `std::to_string(double)`

### (a) BESTÄTIGT — und breiter als gemeldet

- `eq-copilot/plugin/vertrag/NakamaEvidenz.cpp:78` — `return std::to_string (w);` ist
  der **Bruch**zweig (Zeile 77 ist der lokaleunabhängige `long long`-Zweig).
  `std::to_string(double)` ist unter MSVC `sprintf` mit `%f` und damit von `LC_NUMERIC`
  abhängig **und** auf sechs Nachkommastellen fixiert.
- `vertrag/NakamaEvidenz.cpp:259` — der Aufrufer, der es scharf macht:
  `stereoBandsatzJson` schreibt `zahlJson (wert)` in ein **Array** aus 221 Werten. Bei
  Komma-Locale wird aus einem Wert `1,5` ein Paar aus zwei Arrayelementen — die
  `gueltig_bitmap` (`:262-264`) passt danach nicht mehr zur Werteliste.
- Weitere Aufrufer desselben Zweigs: `:294` (Ereignisdauer), `:344` und `:347`
  (Mono-Folddown, L/R-Balance), `:484` und `:486` (Ereignisstärke, Bandzentrum),
  `:508` und `:510` (Abdeckung, Konvergenz).
- **Zweite, im Part nicht genannte Fundstelle:**
  `eq-copilot/plugin/core/ipc/ControlClient.cpp:374` — dieselbe Funktion (`zahl`),
  dieselbe letzte Zeile `return std::to_string (w);`. Der Kommentar in
  `NakamaEvidenz.cpp:64-70` verweist ausdrücklich auf sie als Vorbild.
- **Gegenprobe des Parts trifft zu:** `src/PluginProcessor.cpp:1752-1760` macht es
  richtig — `aus.imbue (std::locale::classic());` und `setprecision (10)`. Die Suche
  nach `std::locale` oder `imbue` über `eq-copilot/plugin` findet **nur** diese eine
  Stelle.

### (b) Erreichbar — ohne Angreifer, aber nicht selbstverschuldet

Der Prozess startet in der C-Locale; kein Nakama-Code ruft `setlocale`. Der Auslöser
ist ein **fremdes Modul im FL-Prozess**: unter `/MD` teilen alle Module dieselbe UCRT,
und ein `setlocale(LC_ALL, "")` eines anderen Plugins oder einer Fremdbibliothek wirkt
prozessweit. Das ist ein bekannter Audio-Plugin-Fallstrick, kein Angriff und keine
Fehlerinjektion. Kette: Analyseworker → Evidenz-/Statussender → `NakamaEvidenz`-JSON →
Pipe → Rust-Leser (`serde_json` ⇒ Parse-Fehler oder falsche Arraylänge).
Unabhängig von der Locale liegt in `%f` außerdem ein Genauigkeitsverlust auf sechs
Nachkommastellen, während `PluginProcessor.cpp:1757` mit `setprecision(10)` sendet —
zwei Zahlformate im selben Produkt.

### (c) Welcher Satz bricht

**CLAUDE.md-Invariante „Schemas sind Verträge."** — ein Writer, dessen Ausgabe der
Gegenleser je nach Prozessumgebung ablehnt oder anders liest, erfüllt den Vertrag
nicht. Aus der Matrix bricht **kein** Satz wörtlich; M-70 (`:575`, „Writer, Reader,
Migration, Fixtures und **beide** Cross-Language-Verbraucher liegen im selben
Änderungssatz") beschreibt die Pflicht, nicht diesen Fall.
⇒ **Härtung mit Vertragsbezug.**

### (d) Register

**Neu.** Die Suche nach `to_string`, `locale` oder `zahlJson` in
`docs/offene-punkte.md` liefert keinen Treffer.

### (e) Fixgröße — klein

- `vertrag/NakamaEvidenz.cpp:71-79` und `core/ipc/ControlClient.cpp:367-375`
  (je ~6 Zeilen): beide Bruchzweige auf `std::to_chars` (locale-frei, kürzeste
  Rundreise) oder auf denselben `ostringstream` mit `std::locale::classic()` und
  `setprecision(10)` wie `PluginProcessor.cpp:1757` umstellen. Ein gemeinsamer Helfer
  wäre schöner, aber `vertrag/` und `core/ipc/` teilen heute keine Einheit — zwei
  kleine identische Fassungen sind hier ehrlicher als ein Querschnitt.
- Test: **neu** in `eq-copilot/plugin/tests/SchemaTestMain.cpp` —
  `zahlen_bleiben_locale_unabhaengig` (im Test eine Komma-Locale setzen, einen
  Evidenzsnapshot bauen, gegen die bestehende Fixture bytegleich prüfen und die Locale
  zurücksetzen). Deckt beide Beine ab, weil A5 und B3c dieselben Fixtures fahren.

---

## V06 — `korrelationKurz` und `persistenz` reisen als gemessene 0.0

### (a) BESTÄTIGT

- `core/analysis/FeatureEngine.h:2908` — das Kurzfenster schließt erst bei
  `++stereoKurzFrames >= kKorrelationKurzFrames`; die Konstante steht in `:856` und
  ist **8**.
- `:2999-3000` — die Zuweisung `e.korrelationKurz = stereoKorrKurz[b]` hängt an
  `stereoKorrKurzGesetzt[b] != 0u` und läuft nie; `e.korrelationKurz` bleibt auf dem
  `StereoBandwert{}`-Startwert `0.0f`.
- `:3025-3028` — `if (stereoKurzfenster > 0) e.persistenz = …` — ebenso nie.
- `:2986` — `e.basisGesetzt = true;` wird trotzdem gesetzt, sobald `gesamt > 0.0`.
- `:3074-3086` — `stereoLeeren()` nullt `stereoKorrKurzGesetzt`,
  `stereoPersistenzZaehler`, `stereoKurzFrames` und `stereoKurzfenster`.
- `:3572` — `evidenzLeeren()` ruft `stereoLeeren()` bei **jedem** fälligen Snapshot
  (`:3558-3560`).
- Rechnung an der Quelle: `kHauptHop = kHauptPunkte / 2 = 2048` (`:894`, `:899`);
  `evidenzIntervallS` startet auf `kEvidenzIntervallMinS = 0.25` (`:956`, `:4176`);
  `evidenzSamples` zählt jeden Sample (`:2418`), nicht nur aktive. 0,25 s mal 48 kHz
  sind 12000 Samples, geteilt durch 2048 ergibt **5,86 Hauptstufen-Hops je Fenster** —
  dauerhaft unter 8.
- `vertrag/NakamaEvidenz.cpp:313-315` und `:325-327` — beide Felder werden mit
  `b[i].basisGesetzt` als Präsenzbit auf die Leitung geschrieben.
- `core/analysis/FeatureEngine.h:469-471` — der Vertragskommentar macht `basisGesetzt`
  ausdrücklich zum Präsenzbit „in KEINEM der fuenf Felder"; `:485-489` sagt zu
  `persistenz`, 0 heiße „in keinem Kurzfenster kohaerent".
- Gegenprobe im selben Block: `kohaerenz` und `phaseRad` (`:3002-3022`) sowie die
  Zeitperzentile (`:3030-3035`) tragen eigene Bits und schweigen im selben Lauf
  korrekt — die Matrixzeile M-11 belegt das sogar mit „149 Bänder mit Basis, 0 mit
  Kohärenzbit".

### (b) Erreichbar — ja, im reinen Normalbetrieb

Kein Angreifer, keine Sonderlage: Vorgabekadenz, 44,1 oder 48 kHz, beliebiges
Stereomaterial über dem Aktivgate. Kette: `sonde/SondeProcessor.cpp:350 nimmBlock` →
`FeatureEngine::baueFrame` → `stereoAuswerten()` (`FeatureEngine.h:3532`) →
`SondeProcessor.cpp:550` → `vertrag/NakamaEvidenz.cpp:313/325` → Evidenz-JSON.
Bei `kEvidenzIntervallMaxS` = 1 s oder bei 96 kHz schließen 1 bis 2 Kurzfenster; der
Fehler hängt an der Vorgabe, nicht am Rand.

### (c) Welcher Satz bricht

**M-11** (`docs/beweise/SONDE-013.md:480`), wörtlich:

> „Fail-closed in zwei Stufen: Kohärenz ist `null` bei zu wenig Energie oder weniger
> als acht Frames, und die **Phase wird nur** in Bändern ausgewertet, deren Kohärenz
> eine benannte Schwelle überschreitet — sonst ist auch sie `null`, nie ein
> geschätzter Wert."

Die Zusage nennt zwei Fail-closed-Stufen; `korrelation_kurz` und `persistenz` haben
gar keine und senden statt `null` eine gemessene Zahl.

Dieselbe Regel steht ausdrücklich in **M-08** (`:477`): „bei einem stillen Kanal fällt
das Präsenzbit weg, **statt 0 oder NaN zu senden**." Und in der CLAUDE.md-Invariante
**„NaN-Ehrlichkeit … ohne genügend endliche Nachbarn gibt es keine Basislinie."**

### (d) Register

**Neu**, unmittelbar neben NAK-177 (`docs/offene-punkte.md:183`, „Bänder unter 200 Hz
tragen nie Kohärenz und Phase"). NAK-177 beschreibt den **fail-closed** Nachbarfall;
V06 ist der fail-**open** Fall derselben Fläche und ist dort nicht mitgemeint.

### (e) Fixgröße — mittel

Zwei Wege, beide klein im Code, unterschiedlich in der Produktwirkung — die Technik
entscheide ich, die Wirkung gehört ins Ticket:

- **Präsenzbits nachziehen (der ehrliche Mindestfix):** `core/analysis/FeatureEngine.h`
  `StereoBandwert` (zwei Felder) plus `:2999-3000` und `:3025-3028` (~6 Zeilen) sowie
  `vertrag/NakamaEvidenz.cpp:313-315` und `:325-327` (~4 Zeilen). Beide Felder liegen
  im `stereoBandsatzJson`-Format, das die Bitmap bereits trägt — also keine
  Vertragserweiterung, nur ein anderer Wähler.
- **Fenster entkoppeln:** Stereokurzfenster nicht mit `evidenzLeeren()` nullen
  (`:3572`), sondern über die Snapshotgrenze weiterlaufen lassen — dann liefern beide
  Felder wieder echte Werte, aber die Zusage „gehört zu GENAU diesem Evidenzfenster"
  (`:3565-3568`) fällt. Der Mindestfix ist deshalb der bessere Einstieg; die
  Entkopplung gehört zusammen mit NAK-177 in ein eigenes Ticket.
- Test: **neu** in `eq-copilot/plugin/tests/Sonde013StereoGoldenTest.cpp` —
  `kurzkorrelation_und_persistenz_schweigen_ohne_kurzfenster` (Vorgabekadenz, 48 kHz,
  Stereorauschen: kein Band darf `korrelation_kurz` oder `persistenz` mit gesetztem Bit
  tragen) und der Gegenfall bei `evidenzIntervallSetzen(1.0)`.

---

## V07 — verlorene Weckung am unbefristeten `warte.wait`

### (a) BESTÄTIGT

- `core/ipc/ControlClient.cpp:1023-1027` — `warte.wait (l, pred)` **ohne Frist**; das
  Prädikat liest `laeuft` und `verbindungsGeneration`.
- `:707`, `:709`, `:711` — `stop()` schreibt `laeuft.store(false)`,
  `verbindungsGeneration.fetch_add(1)` und ruft `warte.notify_all()` **ohne**
  `wartemutex`.
- `:759`, `:766` — `reconnect()` ebenso.
- Das ist die Lehrbuchform der verlorenen Weckung: `wait(lock, pred)` ist
  `while (!pred()) wait(lock);`. Zwischen der Prädikatsauswertung (Prädikat noch
  `false`) und dem Parken ist der Thread noch nicht als Waiter registriert; eine
  `notify_all` in diesem Fenster verpufft. Dass die Prädikatsvariablen atomar sind,
  ändert daran nichts — sie müssen unter dem Mutex geändert werden, damit die Änderung
  gegenüber dem Waiter publiziert ist.
- Gegenprobe im selben File: der zweite Wartepunkt `:1032-1036` benutzt
  `wait_for (l, backoffMs, pred)` und heilt deshalb spätestens nach `backoffMs`.
- Folgekosten belegt: `:729-742` — `stop()` wartet `kStopFristMs = 2000` (`:35`),
  zählt `stopFristUeberschritten` und ruft `thread.detach()`;
  `src/PluginProcessor.cpp:167` — `controlV3.stop()` ist die dritte Anweisung von
  `~EqCopilotProcessor()`, läuft also auf dem Host- und Nachrichtenthread.

### (b) Erreichbar — Zustand fremdauslösbar, Interleaving nicht steuerbar

Der Zweig verlangt `serverPruefstatus == belegtAberUnverifiziert` (`:1015-1017`).
Diesen Zustand erzeugt `core/ipc/IpcVerbindung.cpp:98-127`: alles außer
`ERROR_FILE_NOT_FOUND` und `ERROR_PIPE_BUSY` — insbesondere `ERROR_ACCESS_DENIED` —
fällt fail-closed dorthin. Ein lokaler, nicht privilegierter Prozess kann den
v3-Pipenamen belegen; das ist genau das NAK-123-Szenario. Danach muss ein `stop()`
oder `reconnect()` in das schmale Fenster fallen — nicht gezielt steuerbar, aber bei
jedem Schließen einer Plugininstanz erneut gewürfelt. Kette:
`Laufzeit::threadLauf:1002` → `:1015` → `:1023`; Gegenseite
`~EqCopilotProcessor:167` → `ControlClient::stop:695`.

### (c) Welcher Satz bricht

**Gate 2 aus §49.2** (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3552`), wörtlich:

> „Ein IPC-, Broker-, Datenbank- oder UI-Fehler blockiert den Audiothread."

— **bricht nicht**: der Audiothread ist nicht betroffen; es hängt der Host- und
Nachrichtenthread für 2 s, und danach bleibt ein Thread liegen. **Kein Satz** aus den
fünf Quellen bricht wörtlich; §48.4 (`:3513-3515`) ordnet den Auslöser sogar
ausdrücklich ein („Das schützt vor fremden Windows-Usern, Verwechslung und zufälligen
lokalen Clients"), und der Belegungsfall ist seit NAK-123 bewusst fail-closed.
⇒ **Härtung**, aber mit realem Produktschaden (2 s Stillstand beim Schließen, ein
Thread je betroffener Instanz bis zum Prozessende) und als Voraussetzung für V08.

### (d) Register

**Neu.** NAK-134 (`docs/offene-punkte.md:299`, geschlossen) betraf das Parken nach
erschöpftem `ERROR_PIPE_BUSY` — genau der Fall, der seither über
`belegtNichtErreicht` in den Backoff geht. Das dauerhafte Parken im
`belegtAberUnverifiziert`-Zweig ist die verbleibende Kante und steht nirgends.
NAK-144 („`PipeClient::stop()` joint ohne Frist") betrifft den v2-Client.

### (e) Fixgröße — klein

- `core/ipc/ControlClient.cpp:707-711` und `:759-766` (~6 Zeilen): `laeuft` und
  `verbindungsGeneration` unter `wartemutex` ändern, `notify_all()` nach dem
  Freigeben. Alternativ (~2 Zeilen) den Wartepunkt `:1023` auf `wait_for` mit grober
  Frist umstellen — das repariert die Liveness, nicht die Regel. Der erste Weg ist der
  richtige; beide zusammen kosten nichts extra.
- Test: **neu** in `eq-copilot/plugin/tests/IpcTestMain.cpp` oder
  `PipeClientLifecycleTestMain.cpp` — `stop_loest_das_auth_parken_deterministisch`
  (Client in `belegtAberUnverifiziert` fahren, in einer Schleife über viele Runden
  `stop()` aus dem Fremdthread; `stopFristUeberschritten` muss 0 bleiben).

---

## V08 — abgelöster Thread ruft Callbacks, die den Prozessor fangen

### (a) BESTÄTIGT

- `core/ipc/ControlClient.cpp:724-742` — der Kommentar sagt, der abgelöste Thread
  „haelt die Laufzeit ueber seinen eigenen `shared_ptr` am Leben und beruehrt den
  Client nie"; `:734-741` detacht nach `kStopFristMs`.
- `core/ipc/ControlClient.h:301-311` — dieselbe Begründung ausformuliert: „Ein
  abgeloester Thread, der danach noch Member des Clients laese, waere undefiniertes
  Verhalten". Die Aussage gilt für `ControlClient`, **nicht** für die gespeicherten
  `std::function`.
- `src/PluginProcessor.cpp:106-111` — genau diese Funktionen fangen den Prozessor:
  der Hello-Provider, der Statusprovider `v3Status()`, der Linkstatus-Callback
  `v3ControlLink(bool)` und der Antwort-Callback `v3Antwort(json, schemaMinor)` —
  alle vier als Lambda mit `this`.
- `src/PluginProcessor.cpp:165-167` — `~EqCopilotProcessor()` ruft `controlV3.stop()`;
  kehrt `stop()` nach der Frist zurück, läuft der Destruktor weiter und zerstört
  `sourcesModel`, `merkmale`/`engine`, `zustand` und die Mutexe, während der abgelöste
  Thread noch **in** `v3Status()` oder `v3Antwort()` steht.
- `core/ipc/ControlClient.cpp:685-692` — `start()` gibt dem Thread eine
  `shared_ptr`-Kopie der Laufzeit mit; die Laufzeit überlebt, der **Prozessor** nicht.
- Zwei Teilentwarnungen, die zutreffen: `meldeLinkStatus` (`:988-995`) verhindert über
  `linkAlsVerbundenGemeldet.exchange` den doppelten Ende-Callback am Schluss von
  `eineVerbindung` (`:1550-1560`), und der Loop bricht nach `laeuft == false` ab
  (`:1004-1008`). Der Schaden liegt deshalb nicht in einem *späteren* Aufruf, sondern
  im **noch laufenden**.

### (b) Erreichbar — nur bei einem Stall über 2 s, kein direkter Angreiferpfad

Der Öffnungspfad selbst hängt nicht: `IpcVerbindung::oeffnen` prüft `abbruch.load()`
zu Beginn jeder der 20 Runden (`IpcVerbindung.cpp:76-77`), und `stop()` ruft
`verbindung->ioAbbrechen()` (`:710`) vor dem Warten — die Schleife bricht binnen
höchstens 200 ms ab. Es braucht also einen Callback, der über 2 s steht. **Zwei
belegte Kandidaten im selben Repo:**

- **V13**: ein einzelnes 262-KB-Token im `session_snapshot` hält `textriegel` über
  `NakamaVertrag.cpp:487` zig Sekunden fest — und das läuft in `v3Antwort`, also genau
  in einem dieser Callbacks (Kette: `ControlClient`-Thread → `v3Antwort`
  `PluginProcessor.cpp:1233` → `SourcesModel::uebernehmeSessionSnapshot:459` →
  `textriegelBytes`).
- **V15**: ein FeatureBatch mit aliasierten Offsets erzeugt Millionen Allokationen —
  derselbe Effekt auf dem Telemetriethread, der denselben Detach-Vertrag trägt
  (`TelemetryClient.h:99`, `:119`, `:137`).

V07 dagegen führt **nicht** zum Use-after-free: der dort geparkte Thread ruft nie
wieder einen Callback, er leckt nur. Part 04 formuliert es korrekt als
„Use-after-free, wenn ein Callback über 2 s dauert".

### (c) Welcher Satz bricht

**Kein Satz** aus den fünf Quellen. §48.4 (`:3513-3515`) schließt den lokalen
Same-User-Angreifer aus, Gate 2 (`:3552`) betrifft den Audiothread. Es bricht die
**eigene, im Header ausgeschriebene Begründung** des Detach-Vertrags
(`ControlClient.h:301-311`) — die Bedingung, unter der Ablösen zulässig ist, hält
nicht. ⇒ **Härtung/Lebenszyklus**, aber die schwerste der 15: der Endzustand ist
undefiniertes Verhalten im Hostprozess mit ungesichertem Projekt.

### (d) Register

**Neu.** NAK-144 („`PipeClient::stop()` joint ohne Frist") ist das Spiegelbild auf dem
v2-Client — dort ohne Frist, hier mit Frist und Ablösung. Der Callback-Eigentümer
steht in keiner Zeile.

### (e) Fixgröße — mittel

- `core/ipc/ControlClient.cpp` (~25 Zeilen): die fünf `std::function` hinter ein Flag
  legen, das `stop()` **vor** dem Warten löscht, und jeden Aufruf im Client-Thread
  durch einen gemeinsamen Zug führen. Wer das Flag hält, ruft; wer es nach dem Ablösen
  liest, ruft nicht mehr. Gleiches Muster in `core/ipc/TelemetryClient.{h,cpp}`
  (~15 Zeilen) — die Gegenseite hat denselben Vertrag.
- Der zweite, kleinere Teil ist die eigentliche Ursache: V13 und V15 wegnehmen, damit
  die 2-s-Frist gar nicht erst reißt.
- Test: **neu** in `IpcTestMain.cpp` — `abgeloester_thread_ruft_keinen_callback_mehr`
  (Callback künstlich über `kStopFristMs` blockieren, `stop()` fahren, danach den
  Callback freigeben und prüfen, dass kein weiterer Aufruf ankommt; der Zähler lebt im
  Test, nicht im Produkt).

---

## V09 — `experimente` und `paare` ohne geschlossene Enums und ohne `pair_id`-Länge

### (a) BESTÄTIGT

- `eq-copilot/plugin/src/SourcesModel.cpp:550-573` — geprüft werden nur
  `id.isString()` mit `hex32(...)`, `ereignis.isString()` und `offen.isBool()`; die
  vier optionalen Felder gehen durch den Helfer `wort()` (`:562-567`), der jeden String
  annimmt.
- `:594-609` — dasselbe für `paare`: `id.isString()` und nicht leer,
  `klasse.isString()`, `kette.isString()`. **Keine Längengrenze auf `pair_id`.**
- `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:994 ff.` (`$defs/session_experiment`) —
  `ereignis` ist ein Enum aus `begonnen`, `kandidat`, `ergebnis`, `abgebrochen`,
  `verdraengt`; `hoerurteil`, `blindreihenfolge`, `vergleichbarkeit` und `urteil` sind
  ebenfalls geschlossene Enums.
- `:1030 ff.` (`$defs/session_paar`) — `klasse` verweist auf `alignment_klasse`,
  `kettenbefund` und `ausschluss` sind geschlossene Enums, `pair_id` verweist auf
  `$defs/pair_id`.
- `:788 ff.` (`$defs/pair_id`) — „Strings tragen 1 bis 64 Unicode-Codepoints".
- Gegenprobe **in derselben Datei, für dasselbe Feld**: `SourcesModel.cpp:217-224`
  (`liesDescriptor`) prüft `pair_id` auf nicht leer und Länge höchstens 64. Weitere
  geschlossene Mengen, die der Leser sehr wohl erzwingt: `measurement_position`
  `:135-141`, `aussageklasse` `:200`, `betrieb` `:206-210`, `p2_reject.grund`
  `:247-258`, `plugin_kind` `:652-654`.
- `src/SourcesModel.h:93` sagt zu `urteil` „Eine der fuenf zulaessigen Aussagen aus
  M-46, oder leer" — das ist heute unwahr.

### (b) Erreichbar — mit Peer-Vorbehalt

Kette: Broker/Control-Pipe → `ControlClient` → `EqCopilotProcessor::v3Antwort`
(`src/PluginProcessor.cpp:1266`) → `SourcesModel::uebernehmeSessionSnapshot`.
Der Peer ist authentisiert; nach §48.4 ist ein injizierter Same-User-Prozess außerhalb
des Threat-Models. Der reale Auslöser ist deshalb **ein Broker mit einem Fehler oder
einer neueren Fassung**, nicht ein Angreifer. Es gibt heute **keinen
Anzeigekonsumenten**: `Sicht::experimente` und `Sicht::paare` werden in `sicht()`
(`:1064-1065`) gefüllt und von `src/PluginEditor.cpp` nicht gelesen. Der Defekt ist der
Riegel, nicht das Pixel — er kippt zur Falschaussage auf dem Schirm, sobald die Fläche
gebaut wird.

### (c) Welcher Satz bricht

**M-46** (`docs/beweise/SONDE-013.md:536`), wörtlich:

> „Genau fünf zulässige Aussagen: Ziel verbessert und Guardrails stabil; messbar
> anders, musikalisches Urteil offen; keine belastbare Änderung; Ziel verbessert, aber
> geschützte Eigenschaft verschlechtert; Vergleich nicht gültig. **Nie** ‚objektiv
> besser' allein aus einem Metrikdelta."

Das Gen-Modell hält heute jeden String in `urteil`, also auch „objektiv besser".
Sekundär **M-22** (`:497`): „Harte Ausschlüsse führen zu `unclear` mit benanntem
Grund, nicht zu einer schwachen Zahl" — `klasse` und `kettenbefund` außerhalb der
Menge sind kein benannter Grund. Dazu die CLAUDE.md-Invariante **„Schemas sind
Verträge."**

### (d) Register

**Neu.** NAK-77 (`docs/offene-punkte.md:272`) beschreibt genau diese Klasse für den
v3-Vertrag („prüft an drei Stellen die Länge, aber nicht das Alphabet"), nennt aber
nicht `experimente` und `paare`; die Felder sind erst mit der Fassung 2 in SONDE-013
entstanden. NAK-141 betrifft die Klasse-gegen-Position-Matrix.

### (e) Fixgröße — klein

- `src/SourcesModel.cpp:530-612` (~35 Zeilen): je eine Mengenprüfung für `ereignis`,
  `hoerurteil`, `blindreihenfolge`, `vergleichbarkeit`, `urteil`, `klasse`,
  `kettenbefund`, `ausschluss`; `pair_id` auf höchstens 64 wie in `:217-224`. Alles
  Abweichende ergibt `SnapshotErgebnis::ungueltig`, also denselben Ausgang wie jedes
  andere Typvergehen in derselben Funktion.
- Test: **bestehend erweitern** — `eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp`
  (dort liegen die Snapshot-Riegelfälle); neue Fälle
  `session_experiment_urteil_ausserhalb_der_menge` und
  `session_paar_pair_id_zu_lang`. Fixtures unter `eq-copilot/fixtures/v3/` gehören in
  denselben Änderungssatz, damit beide Beine dieselbe Menge sehen (M-70).

---

## V10 — nicht-String `ausschluss` wird still verworfen (fail-open)

### (a) BESTÄTIGT

- `src/SourcesModel.cpp:588-589` — `exakteFelder` lässt `ausschluss` als optionales
  Feld zu.
- `:607-609` — das Feld wird gelesen und nur bei `aus.isString()` übernommen; ein
  vorhandenes, aber nicht-String-Feld fällt still weg.
- `src/SourcesModel.h:103` — „Leer heisst ‚das Paar traegt eine Aussage'." Aus „trägt
  keine Aussage" wird damit „trägt eine".
- Richtungsvergleich in derselben Funktion: jedes andere Typvergehen liefert
  `SnapshotErgebnis::ungueltig` (`:546-548`, `:552-556`, `:586-591`, `:598-602`).
- Schemaseitig ist `ausschluss` ein geschlossenes Enum
  (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `$defs/session_paar`), also gilt hier
  zusätzlich alles aus V09.

### (b) Erreichbar

Dieselbe Kette wie V09, derselbe Peer-Vorbehalt, ebenfalls noch ohne
Anzeigekonsumenten.

### (c) Welcher Satz bricht

**M-22** (`docs/beweise/SONDE-013.md:497`), wörtlich:

> „Harte Ausschlüsse führen zu `unclear` mit benanntem Grund, nicht zu einer schwachen
> Zahl: fehlende Hälfte, getrennte Hälfte, stale Hälfte, Zustand nicht messbereit,
> fehlende Projektzeit, Fensterlänge ≤ 0."

Der Ausschluss verliert hier seinen Grund und damit seine Wirkung. Sekundär die
CLAUDE.md-Invariante **„Schemas sind Verträge."**

### (d) Register

**Neu**, gehört mit V09 in eine Zeile.

### (e) Fixgröße — klein

- `src/SourcesModel.cpp:607-609` (~4 Zeilen): ein vorhandenes `ausschluss` muss String
  **und** aus der Enum-Menge sein, sonst `ungueltig`.
- Test: **bestehend erweitern** — `Sonde012SourcesModelTest.cpp`, Fall
  `session_paar_ausschluss_nicht_string_ist_ungueltig`.

---

## V11 — `Diagnose::incompatible` wird vom nächsten Editor-Tick gelöscht

### (a) BESTÄTIGT

- `src/PluginProcessor.cpp:1263` und `:1270` — `v3Antwort` setzt bei ungültiger
  Rücknahme bzw. ungültigem Snapshot
  `sourcesModel.setzeDiagnoseFuerSichtbeweis (Diagnose::incompatible, false)`.
- `src/SourcesModel.cpp:1134-1140` — `setzeDiagnoseFuerSichtbeweis` schreibt
  `diagnose` und `revidiere()`.
- `src/SourcesModel.cpp:429-431` — `setzeControlTransport`: bei aktiver Subscription
  und einer Diagnose, die weder `confirmationRequired` noch `storeDegraded` ist, wird
  `neu = Diagnose::keine` gesetzt. `incompatible` steht **nicht** in der
  Ausnahmeliste.
- Gegenprobe 30 Zeilen darüber: `controlEnde()` (`:396-398`) schützt genau diesen Wert
  und listet `incompatible`, `storeDegraded` und `serverUnverified` auf. Dieselbe
  Datei, dieselbe Absicht, eine Stelle vergessen.
- Taktung: `src/PluginEditor.cpp:230` `startTimerHz (30)`; `:242-247` `timerCallback`
  → `processor.sourcesTick()` → `src/PluginProcessor.cpp:1286`
  `sourcesModel.setzeControlTransport (controlV3.snapshot())` ⇒ **rund 33 ms**.
- `subscriptionAktiv` bleibt bei `SnapshotErgebnis::ungueltig` unverändert (die
  Funktion kehrt vor jeder Zustandsänderung um) — in einer laufenden Sitzung ist es
  also `true`, und der Reset greift beim ersten Tick.
- Sichtbarkeit: `src/SourcesModel.cpp:1337` gibt für `incompatible` den Text
  „Incompatible broker or protocol" aus.

### (b) Erreichbar — im Normalbetrieb, sichtbar nur einen Frame lang

Kein Angreifer nötig: ein Broker mit einem Vertragsfehler oder einer neueren
Snapshot-Fassung genügt. Kette: `ControlClient`-Thread → `v3Antwort:1266/1270` setzt;
Nachrichtenthread → `EqCopilotEditor::timerCallback:242` → `sourcesTick:1281` →
`setzeControlTransport:429` löscht rund 33 ms später.

### (c) Welcher Satz bricht

**Kein Satz** aus den fünf Quellen. Es bricht die CLAUDE.md-**Wahrheitskern**-Zeile
„Jedes sichtbare Element führt einen Handgriff aus oder **meldet ehrlich einen
Zustand**" — die steht nicht im Abschnitt „Tragende technische Invarianten".
⇒ **Lücke/Härtung mit Produktwirkung**: die einzige Meldung über einen
Vertragsverstoß des Brokers ist für den User unsichtbar.

### (d) Register

**Neu.** Die Suche nach `Diagnose`, `incompatible` oder `Diagnoseband` in
`docs/offene-punkte.md` liefert keinen Treffer.

### (e) Fixgröße — klein

- `src/SourcesModel.cpp:429-431` (~1 Zeile): `incompatible` und `serverUnverified` in
  dieselbe Ausnahmeliste wie in `controlEnde()` (`:396-398`).
- Gegenpfad mitdenken (Arbeitsregel „aktivieren↔abklingen"): wenn `incompatible`
  klebt, braucht es einen Löschpfad. Der natürliche ist `reconnectSources()`
  (`src/PluginProcessor.cpp:1285-1288`) — der Handgriff, den das Diagnoseband ohnehin
  anbietet. Rund 3 Zeilen.
- Test: **bestehend erweitern** — `Sonde012SourcesModelTest.cpp:415-460` fährt bereits
  `setzeControlTransport`-Folgen; neuer Fall
  `incompatible_ueberlebt_den_naechsten_transport_tick`.

---

## V12 — `e.tailSamples` rechnet mit der unbereinigten `getSampleRate()`

### (a) BESTÄTIGT

- `src/PluginProcessor.cpp:611-612` — `e.tailSamples` ist
  `schritt.dauerSamples * 2u` plus `std::max (1, (int) getSampleRate() / 10)`.
- `juce::AudioProcessor::getSampleRate()` liefert den vom Host über
  `setRateAndBufferSizeDetails` gesetzten **Rohwert**. Der Beweis, dass das Projekt ihn
  selbst als unvertrauenswürdig führt, steht 400 Zeilen darüber: `:182-184` bildet
  `sichereSamplerate` über `std::isfinite`, größer 0 und höchstens 768000, und `:190`
  legt genau diesen geprüften Wert in `samplerateAtomic` ab.
- Ein `(int)`-Cast auf einen nicht-endlichen oder außerhalb `int` liegenden `double` ist
  undefiniertes Verhalten. Auf x86 liefert `cvttsd2si` den „integer indefinite"-Wert
  `INT_MIN`; `INT_MIN / 10` ist negativ, `std::max (1, …)` klemmt auf **1** ⇒ Nachlauf
  `dauer*2 + 1 Sample` statt `dauer*2 + 100 ms`.
- Der Kommentar `:604-610` benennt die Absicht ausdrücklich: der Polster ist für den
  **biquadratischen Filternachklang** der Markierung da, der den Eingriff überlebt.
  Genau dieser Anteil fällt weg.
- Kontext: die Stelle liegt in `processBlock` (Audiothread). `getSampleRate()` ist ein
  einfacher Memberlesevorgang — **kein** Verstoß gegen „Audio bleibt echtzeitfest".

### (b) Erreichbar — nur über einen defekten oder feindlichen Host

FL Studio liefert eine gültige Rate; ein fremder unprivilegierter Prozess kann den
`ProcessSetup` nicht schreiben. Kette: Host → JUCE-VST3-Wrapper →
`AudioProcessor::setRateAndBufferSizeDetails` → `getSampleRate()` in
`processBlock:611`. Dieselbe Klasse wie die beiden LOW-Befunde aus unit-02
(`FeatureEngine.h:1784`, `:1893`) — Hostkontext, nicht Angreifer.

### (c) Welcher Satz bricht

**CLAUDE.md-Invariante**, wörtlich:

> „**NaN-Ehrlichkeit.** Nicht-endliche Werte werden verriegelt und gezählt; Nyquist
> wird gekappt; ohne genügend endliche Nachbarn gibt es keine Basislinie."

Hier wird ein nicht-endlicher Hostwert weder verriegelt noch gezählt, sondern in eine
`int`-Wandlung gegeben.

**M-38** (`docs/beweise/SONDE-013.md:523`) bricht **nicht** wörtlich: „der Bereich wird
länger quarantänisiert, als die Intervention gedauert hat" — `dauer*2 + 1` ist immer
noch länger. Was fällt, ist der beabsichtigte Nachklangpolster, also die Wirkung von
M-39 („**Ein verlorenes Begin oder End darf niemals eine scheinbar saubere Baseline
erzeugen**", `:524`) an der Nachlaufkante. Ehrlich eingestuft: **Invariantenbruch bei
NaN-Ehrlichkeit, kein wörtlicher M-38-Bruch.**

### (d) Register

**Neu.** Keine Zeile nennt `tailSamples` oder `getSampleRate()` im Interventionspfad.
NAK-158 und NAK-159 betreffen den Vergleichspegel, nicht den Nachlauf.

### (e) Fixgröße — klein

- `src/PluginProcessor.cpp:611-612` (~3 Zeilen): `samplerateAtomic.load()` statt
  `getSampleRate()` und die Wandlung fail-closed (endlich und größer 0, sonst ein
  fester Mindestpolster). Ein fester Mindestpolster ist hier ehrlicher als 1 Sample.
- Denselben Griff mitprüfen: `src/PluginProcessor.cpp:1817`
  (`vergleichspegel.vorbereiten (getSampleRate())`) benutzt dieselbe rohe Quelle —
  gehört in denselben Änderungssatz, sonst bleibt die Hälfte stehen.
- Test: **bestehend erweitern** —
  `eq-copilot/plugin/tests/Sonde013InterventionRingTest.cpp` (dort liegt
  `end_carries_conservative_tail_and_follows_the_fade`); neuer Fall
  `tail_bleibt_konservativ_bei_ungueltiger_hostrate` (prepareToPlay mit NaN- bzw.
  0-Rate, Marker an und aus, `tailSamples` muss den Mindestpolster tragen).

---

## V13 — `teil()` baut Strings zeichenweise, quadratischer Aufwand

### (a) BESTÄTIGT

- `eq-copilot/plugin/vertrag/NakamaVertrag.cpp:486-492` — das Lambda `teil` legt eine
  leere `juce::String` an und hängt in einer Schleife je Zeichen
  `juce::String::charToString (z[k])` an.
- `juce::String::operator+=` ruft `appendCharPointer`, das erst `getByteOffsetOfEnd()`
  (Lauf bis zum NUL) und dann `preallocateBytes` benutzt; letzteres reallokiert auf die
  exakte neue Länge, ohne geometrisches Wachstum. Beides ist je Zeichen linear ⇒
  quadratische Kopierarbeit und rund 2·k Heapallokationen.
- Aufrufer für dasselbe Token: `:604` (Literal), `:621` (`ganz`), `:629` (führende
  Null), `:668` (`teil (i, j)` für `zahlPruefen`). Ein Token wird also bis zu dreimal
  vollständig aufgebaut.
- Dieselbe Form nebenan in `zahlPruefen`: `:340-341` und `:403-404` — jedes
  `substring` ist eine volle Kopie.
- Rahmengrenze: `core/ipc/WireEnvelope.h:38` = 262144 Bytes; `kMaxDokumentBytes`
  (`NakamaVertrag.h:29`, geprüft in `:444`) liegt darüber. Ein Token von rund 262 000
  Zeichen ist damit zulässig.
- Der Riegel weist das Token danach ab — bezahlt ist der Aufwand trotzdem, und er ist
  wiederholbar.

### (b) Erreichbar — mit Peer-Vorbehalt, aber mit Folgeschaden

Kette: Broker → `ControlClient`-Thread → `EqCopilotProcessor::v3Antwort`
(`src/PluginProcessor.cpp:1233`) → `SourcesModel::uebernehmeSessionSnapshot:459` bzw.
`uebernehmeEvidenzruecknahme:813` → `nakama::vertrag::textriegelBytes`
(`NakamaVertrag.cpp:469`) → `textriegel:474`.
Nach §48.4 (`:3513-3515`) ist der authentisierte Same-User-Peer außerhalb des
IPC-Threat-Models; realistischer Auslöser ist ein fehlerhafter Broker. **Wichtig ist
die Kopplung:** dieser Stillstand ist der belegte Weg, auf dem V08 scharf wird — der
Control-Thread steht länger als `kStopFristMs`, `stop()` löst ihn ab, und der Callback
läuft weiter, während `~EqCopilotProcessor` fortschreitet.

### (c) Welcher Satz bricht

**Gate 2 aus §49.2** (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3552`), wörtlich:

> „Ein IPC-, Broker-, Datenbank- oder UI-Fehler blockiert den Audiothread."

— bricht **nicht**: der Audiothread ist nicht betroffen. **Kein Satz** aus den fünf
Quellen bricht. ⇒ **Härtung**, mit der Besonderheit, dass sie die Voraussetzung für
V08 liefert und deshalb zusammen mit ihm gewertet werden sollte.

### (d) Register

**Neu.** NAK-78 (`docs/offene-punkte.md:101`) sammelt die G1-Befunde am Textriegel
(u. a. den unbewachten Längen-Cast), nennt aber nicht den quadratischen
Ausschnittsbau.

### (e) Fixgröße — klein

- `vertrag/NakamaVertrag.cpp:486-492` (~6 Zeilen): `teil()` baut über einen einzigen
  Aufbau aus dem vorhandenen `std::vector<juce::juce_wchar>` statt Zeichen für
  Zeichen. Zusätzlich (~4 Zeilen) die **gemeldete** Ausschnittslänge kappen — der
  Fehlertext braucht nie mehr als die 20 Zeichen, die `:629` ohnehin schon nimmt.
- `:340-341` und `:403-404` (~6 Zeilen): die `substring`-Schleifen durch einen Index
  ersetzen.
- Test: **neu** in `eq-copilot/plugin/tests/SchemaTestMain.cpp` —
  `textriegel_haelt_ein_rahmengrosses_token_aus` (ein Dokument mit einem
  262 000-Zeichen-Token und einer Laufzeitschranke im Test). Fixture nach
  `eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json` — die Datei ist laut
  `NakamaVertrag.h:97-99` der einzige zulässige Ort für Riegelfälle.

---

## V14 — Textriegel ohne Tiefenregel, `juce::JSON::parse` rekursiert ungeschützt

### (a) BESTÄTIGT

- `vertrag/NakamaVertrag.h:66-84` — die **neun** Regeln des Riegels; keine betrifft
  Struktur oder Verschachtelungstiefe (führende Null, Ganzzahlbereich, Exponent,
  u-Escape, NUL-Escape, Surrogate, leerer Schlüssel, Literale, Byteebene).
- `vertrag/NakamaVertrag.cpp:474-680` — kein Tiefenzähler; öffnende Klammern fallen in
  `:675` durch das schlichte Weiterrücken und laufen durch.
- `src/SourcesModel.cpp:465-467` und `:817-819` — beide Produktionsaufrufer geben den
  Text **unmittelbar danach** an `juce::JSON::parse`.
- `eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp` (JUCE 8.0.9,
  gepinnt in `eq-copilot/CMakeLists.txt:32`): `parseAny:202` → `parseArray:336` →
  `parseAny` in `:352`, bzw. `parseObject:290` → `parseAny` in `:324`. Die Suche nach
  `depth`, `Depth` oder `recursion` über die Datei findet **nur** den Testgenerator
  `createRandomVar:679` — im Parser keinen Tiefenzähler.
- Gegenprobe im Repo: der **eigene** Leser des Projekts kappt bei 64 —
  `state/NakamaKanon.h:83`, `state/NakamaKanon.cpp:396` und `:424`.
- Ein Stack-Overflow ist unter Windows kein fangbarer C++-Fehler; der Hostprozess
  stirbt mit ungesichertem Projekt.

### (b) Erreichbar — mit Peer-Vorbehalt

Kette identisch zu V13 bis `textriegelBytes`; der Absturz fällt eine Zeile später bei
`src/SourcesModel.cpp:466` bzw. `:820`. Rund 10 KB aus lauter öffnenden Klammern
genügen. Derselbe §48.4-Vorbehalt.

### (c) Welcher Satz bricht

**Kein Satz** aus den fünf Quellen. §48.4 (`:3505`) verlangt vom **Server** „Danach
prüft der Server Clienttoken, Sitzung, Handshake, Nachrichtentyp, **Tiefe**, Länge und
Rate" — das ist die Brokerseite, nicht der Pluginleser; die Zeile zeigt aber, dass
„Tiefe" im Vertrag als Prüfdimension geführt wird und im C++-Leser fehlt.
⇒ **Härtung**, aber die Asymmetrie ist im eigenen Repo dokumentiert (`NakamaKanon`
kappt, `juce::JSON` nicht) und die Schadensform ist Prozesstod.

### (d) Register

**Neu.** NAK-78 (`docs/offene-punkte.md:101`) führt den Tiefenvergleich ausdrücklich
**nur für den Binärpfad** (`state/NakamaState.cpp:353`); der JSON-Pfad steht dort
nicht.

### (e) Fixgröße — klein

- `vertrag/NakamaVertrag.cpp:474-680` (~10 Zeilen): ein Tiefenzähler über öffnende und
  schließende Klammern mit derselben Grenze 64 wie `state/NakamaKanon.h:83`, als
  **zehnte** Regel. Der Riegel läuft ohnehin schon zeichenweise über den ganzen Text —
  es kostet keinen zusätzlichen Durchlauf.
- `vertrag/NakamaVertrag.h:66-84` (~3 Zeilen): die Regelliste nachziehen; sie ist
  Vertragstext und darf nicht hinter dem Code zurückbleiben.
- Cross-Language-Pflicht (M-70): dieselbe Regel im Rust-Bein und ein Fixture in
  `eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json` — sonst laufen die Beine auseinander,
  was der Riegel gerade verhindern soll.
- Test: **neu** in `SchemaTestMain.cpp` —
  `textriegel_lehnt_zu_tiefe_verschachtelung_ab` (10 000 offene Klammern ergeben
  `false` mit gesetztem Fehler, **bevor** ein Parser läuft).

---

## V15 — `pruefe` meldet die Eintragsgrenze und läuft trotzdem über jeden Eintrag

### (a) BESTÄTIGT

- `eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:621-624` — bei Größe 0 wird
  `eintraege_leer` gemeldet, bei mehr als `maxEintraege` wird `eintraege_zu_viele`
  gemeldet; beides nur gemeldet, nicht abgebrochen.
- `:631-645` — die Schleife läuft danach über **jeden** Eintrag: `pruefeAdresse`, ein
  `std::set`-Insert der `instance_id`, `pruefeFrame`.
- `:549-558` — `kanonisch` legt anschließend jeden Verstoß in ein `std::set`.
- `:133-136` — der Anhänger baut je Verstoß zwei `juce::String`.
- `vertrag/NakamaTelemetrie.h:56` — `maxEintraege = 32`.
- FlatBuffers erlaubt aliasierende Offsets; die Verifier-Grenze `max_tables` bleibt bei
  rund fünf Tabellenbesuchen je Eintrag weit entfernt.
- **Die Erreichbarkeitsaussage stimmt und ist neu:** `lese` (`:650-657`) ruft `pruefe`
  unbedingt, und `lese` hat einen Produktionsaufrufer — `src/SourcesModel.cpp:959`
  (`uebernehmeP2`) ← `src/PluginProcessor.cpp:1277` (`v3Frame`) ←
  `TelemetryClient`-Thread. NAK-116 sagt wörtlich „und `pruefe` hat heute keinen
  Produktionsaufrufer — Härtung für S6, kein Defekt" (`docs/offene-punkte.md:93`); das
  gilt für den C++-Leser seit SONDE-012/013 nicht mehr.

### (b) Erreichbar — mit Peer-Vorbehalt, Erschöpfung statt Fehlklassifikation

Kette: Broker → `TelemetryClient`-Thread → `v3Frame`
(`src/PluginProcessor.cpp:1273-1279`) → `SourcesModel::uebernehmeP2:959` →
`nakama::telemetrie::lese:655` → `pruefe:621`. §48.4-Vorbehalt wie bei V13 und V14.
Die Klassifikation bleibt korrekt — es ist reine Ressourcenerschöpfung auf dem
Telemetriethread. Wie V13 kann sie die 2-s-Ablösefrist reißen und damit V08 füttern
(`TelemetryClient.h:99`, `:119`, `:137` tragen denselben Detach-Vertrag).

### (c) Welcher Satz bricht

**Kein Satz.** NAK-150 hält bereits fest, dass die Klassifikation unberührt bleibt und
eine frühe Rückkehr **kein Teilfix** ist, weil die Verstoßliste Cross-Language-Vertrag
ist. Am nächsten liegt **M-78** (`docs/beweise/SONDE-013.md:615`): „P2 darf
`drop-oldest` verwenden; sein Ausfall degradiert Analyse, nicht Control." — der Ausfall
degradiert hier nicht Control, aber er blockiert den Telemetriethread; wörtlich bricht
die Zeile nicht. ⇒ **Härtung.**

### (d) Register

**NAK-150** (`docs/offene-punkte.md:156`) — nicht neu. Die Zeile nennt beide Beine
(`broker/src/telemetrie.rs:307-308` und `NakamaTelemetrie.cpp:558-568`; die C++-Zeilen
sind seither auf `:621-624` und `:631-645` gewandert) und beschreibt die
Cross-Language-Pflicht. **Neu ist ausschließlich die Erreichbarkeit**: NAK-116
(`:93`), auf die NAK-150 zurückgeht, stützt sich auf „kein Produktionsaufrufer", und
das ist für den C++-Leser überholt. Die Registerzeile braucht diesen Nachtrag.

### (e) Fixgröße — mittel

Nicht der Abbruch ist die Arbeit, sondern der Vertrag drumherum — genau das sagt
NAK-150 schon:

- `vertrag/NakamaTelemetrie.cpp:621-645` und `broker/src/telemetrie.rs:368-392`
  (je ~5 Zeilen): nach `eintraege_zu_viele` die Schleife auf `maxEintraege` Durchläufe
  kappen — nicht abbrechen, damit die Verstoßliste aussagekräftig bleibt und trotzdem
  gedeckelt ist.
- Neues Fixture unter `eq-copilot/fixtures/v3/flatbuffers/ungueltig/` mit **zugleich**
  mehr als `MAX_EINTRAEGE` und fehlerhaften Einträgen; das bestehende
  `eintraege-33.bin` kann den Unterschied nicht messen.
- Beide Beine, Fixture und Vertragsrunde in **einem** Änderungssatz (M-70).
- Test: **bestehend erweitern** — die Kanonbeine A5, A8 und B3c sowie
  `broker/tests/contract_cross_language.rs` fahren dieselben Fixtures; die neue Datei
  wirkt dort ohne neuen Testrahmen. Zusätzlich ein Laufzeitfall in
  `eq-copilot/plugin/tests/SchemaTestMain.cpp`:
  `pruefe_bleibt_bei_aliasierten_offsets_beschraenkt`.

---

## Zusammenfassung

| Befund | Urteil | bricht | Register | Fix |
|---|---|---|---|---|
| V01 | BESTÄTIGT | M-43 (dazu M-07, NaN-Ehrlichkeit) | neu | klein |
| V02 | BESTÄTIGT | kein Satz (Wahrheitskern „keine toten Elemente") | neu | klein |
| V03 | BESTÄTIGT | M-50 | neu | mittel |
| V04 | BESTÄTIGT | kein Satz (Codevertrag `p1Uebergeben`) | neu | klein |
| V05 | BESTÄTIGT (+ 2. Fundstelle) | Invariante „Schemas sind Verträge" | neu | klein |
| V06 | BESTÄTIGT | M-11 (dazu M-08, NaN-Ehrlichkeit) | neu (neben NAK-177) | mittel |
| V07 | BESTÄTIGT | kein Satz | neu | klein |
| V08 | BESTÄTIGT | kein Satz (eigener Detach-Vertrag) | neu | mittel |
| V09 | BESTÄTIGT | M-46 (dazu M-22, Schemas sind Verträge) | neu | klein |
| V10 | BESTÄTIGT | M-22 | neu | klein |
| V11 | BESTÄTIGT | kein Satz (Wahrheitskern „ehrlicher Zustand") | neu | klein |
| V12 | BESTÄTIGT | Invariante „NaN-Ehrlichkeit" (M-38 **nicht** wörtlich) | neu | klein |
| V13 | BESTÄTIGT | kein Satz | neu | klein |
| V14 | BESTÄTIGT | kein Satz | neu | klein |
| V15 | BESTÄTIGT | kein Satz | **NAK-150** (neu: Erreichbarkeit) | mittel |

**Kein Befund widerlegt.** Drei Präzisierungen gegen die Part-Dateien:

1. V01 bis V03 haben heute **keinen UI-Aufrufer**; sie sind verdrahtet, aber nicht
   bedienbar (`src/PluginEditor.cpp` kennt den Experimentpfad nicht).
2. V05 hat eine **zweite Fundstelle**, die der Part nicht nennt:
   `core/ipc/ControlClient.cpp:374`.
3. V08 wird **nicht** durch V07 ausgelöst (der geparkte Thread ruft keinen Callback
   mehr), sondern durch V13 bzw. V15, die den über 2 s stehenden Callback liefern.
   Diese drei gehören in eine gemeinsame Bewertung.

**Fünf Befunde brechen eine Zusage wörtlich:** V01 (M-43), V03 (M-50), V06 (M-11),
V09 (M-46), V10 (M-22). Zwei brechen eine CLAUDE.md-Invariante: V05 („Schemas sind
Verträge") und V12 („NaN-Ehrlichkeit"). Die übrigen acht sind Lücke oder Härtung —
darunter mit V08 die schwerste Einzelfolge (undefiniertes Verhalten im Hostprozess).
