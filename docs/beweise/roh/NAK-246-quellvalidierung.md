# NAK-246 — Quellvalidierung des Codeaudits vom 10.09.2026 (lesender Opus-Agent, Snapshot 844b9c15)

# Validierung des Codeaudits vom 10.09.2026 gegen den Snapshot `844b9c15`

Prüfort: `C:\na-audit-20260910-844b9c1` (HEAD = `844b9c15`, Worktree bis auf `docs/audits/` sauber). Alle Zeilennummern aus diesem Snapshot.

---

## D1 — Slotbesitz im Audio-Markierungsring

**1. Quellkette, zitiert**

`eq-copilot/plugin/src/HoerMarkierung.h:660-661` (Speicher):
```
    std::array<MarkierungsAuftrag, 4> ring {};
    std::atomic<std::uint32_t> veroeffentlicht { 0 };
```
`:314-321` (Publisher, Message-Thread):
```
    void reicheEin (const MarkierungsAuftrag& a)
    {
        const std::uint32_t nr = veroeffentlicht.load (std::memory_order_relaxed) + 1;
        ring[nr % ring.size()] = a;
        veroeffentlicht.store (nr, std::memory_order_release);
```
`:377-391` (Audioleser):
```
        const auto nr = veroeffentlicht.load (std::memory_order_acquire);
        if (nr != gelesenNr)
        {
            if (ring[nr % ring.size()].modus == MarkierungsModus::aus)
                ausGewuenscht = true;
            else
            {
                lokal = ring[nr % ring.size()];
```
Auslöser-Umgebung: Publikationen entstehen ausschließlich aus dem Editor — `PluginEditor.cpp:800` (`processor.markierungEinreichen (auftrag)` in `schalteMarkierung`, ab `:754`), `:821` (`markierungBeenden` → `markierungAus`), `:237` (Editor-Destruktor). Die Timer-Pfade `:271/:275/:277` laufen über `markierungBeenden`, das bei `markModus == aus` sofort zurückkehrt (`:817-820`) — sie können den Zähler also nicht selbsttätig treiben. Struktur des kopierten Objekts: `:132-142`, u. a. `std::array<std::uint8_t, kPulsMaxChunks> stufenFolge {}` mit `kPulsMaxChunks = 2048` (`:40`) — rund 4,8 KB POD je Kopie.

Der Kopfkommentar bestätigt die Lücke wörtlich, `:16-18`:
```
//  · Ein Slot-Riss wäre nur möglich, wenn der Editor während EINER
//    Blockkopie vier Aufträge publiziert — menschlich unerreichbar; der
//    Zähler macht jede neue Publikation trotzdem sichtbar.
```

**Urteil: Auslöser folgt aus dem Code, aber mit einer Zusatzbedingung, die der Audit zu schwach fasst.** Kein Riegel, kein Lock, keine Generation und kein Besitzprotokoll verhindert die Slotwiederverwendung — das Release/Acquire ordnet nur die Sichtbarkeit. Der Auslöser verlangt jedoch, dass der Audiothread zwischen `:377` und `:387` unterbrochen wird **und** in diesem Fenster vier Editor-Publikationen stattfinden. Publikationen sind hier durchweg useraktiviert (ein Klick je Publikation); die Timer-Selbstheilung ist durch den Frühausstieg `:818-819` gedeckelt. Eine Unterbrechung, die vier menschliche Klicks überspannt, liegt bei ≥ 150–200 ms — das ist kein normales Schedulerquantum. Die Aussage „Eine Scheduler-Unterbrechung benötigt keine ungewöhnlich schnelle menschliche Bedienung" ist damit **nicht belegt**; die formale Race-Freiheit fehlt trotzdem.

**2. Bestehende Tests**

`eq-copilot/plugin/tests/MarkierungTestMain.cpp` — Bein **A3** `EqCopMarkierungTest`. `reicheEin` wird an `:575`, `:624`, `:734`, `:785`, `:821` gerufen, **immer sequenziell vor** `verarbeite`. `grep "std::thread|join()|detach"` über die Datei: **kein Treffer** (das `#include <thread>` in `:22` bleibt ungenutzt für Nebenläufigkeit). Der Fall des Audits ist damit **nicht** abgedeckt. Auch `A1 EqCopNullTest` misst nur den Passthrough.

**3. Welche Zusage bricht**

Keine Zusage nennt den Markierungsring ausdrücklich. Am nächsten liegt die **verbindliche Prüfebene** `docs/FL-Nakama-Sonden-Design-Entwurf.md:3549`:
> `| Audiothread | Allocation-/Lock-Guards, Worst-Case-CPU, zufällige und über `maximumExpected` liegende Blockgrößen, Bank-Reclaim-, Thread-/Race-Stress |`

Ein „Thread-/Race-Stress"-Bein für diesen Pfad existiert nicht. Der Entwurf formuliert das gewünschte Muster für die **DSP-Bänke** (`:2991-2996`, „Die vier Slots besitzen ein explizites lockfreies Ownership-Protokoll `free → preparing → ready(generation) → audio_active/fading → retired → free`… es gibt weder In-place-Überschreiben noch Heap-Reclaim") — das gilt aber ausdrücklich den Programmbänken, **nicht** dem Markierungsring. **Keine Zusage regelt den Fall direkt** → für den Dirigenten Härtung/Lücke, kein Defekt gegen eine gebrochene Zusage.

**4. Ticket/Bereich**

SONDE-013 (M-33 bis M-38, NAK-47) baute den Marker aus; NAK-180 R4 ergänzte den Endlichkeitsriegel (`docs/beweise/NAK-180.md:167`, `:479`, Ticketpfade `:12`). Registerpunkt in `docs/offene-punkte.md`: **NAK-35** (Hör-Markierungs-Verriegelung, Doku↔Code) und **NAK-179** (Marker stumm ohne Hostbrücke) betreffen andere Aspekte; **zum Slotbesitz gibt es keine Zeile** — nicht gefunden.

**5. Überschneidung mit Etappe 4a**

**Nein.** `HoerMarkierung.h` und `PluginEditor.cpp` liegen in `eq-copilot/plugin/src/**`, das der 4a-Auftrag ausdrücklich ausschließt (`docs/beweise/roh/SONDE-015-etappe-4-auftrag.txt:23`: „NICHT ANFASSEN in 4a: … `eq-copilot/plugin/src/**` (Gens Prozessor)").

**6. Fixumfang**

`eq-copilot/plugin/src/HoerMarkierung.h` (Ring + Leser, ~30–60 Zeilen), `eq-copilot/plugin/tests/MarkierungTestMain.cpp` (Nebenläufigkeitsfall, ~80–150 Zeilen). Beine danach: **A3**, **A1**, **A16**, zusätzlich **B21** (`EqCopSonde013InterventionRingTest`), da die Übergangsbuchführung am selben Objekt hängt.

---

## D2 — IPC-Laufzeit lebt länger als der Besitzer ihrer Callbacks

**1. Quellkette, zitiert**

`eq-copilot/plugin/core/ipc/TelemetryClient.cpp:30`: `constexpr int kStopFristMs = 2000;`

`eq-copilot/plugin/core/ipc/ControlClient.cpp:119-135` (`stop()` ab `:90`):
```
    // `B-CC-12`: auf einen laufenden Callback wird hoechstens `kStopFristMs`
    // gewartet. Danach wird der Thread ABGELOEST — er haelt die Laufzeit ueber
    // seinen eigenen `shared_ptr` am Leben und beruehrt den Client nie.
    …
            thread.detach();
            return;
```
Spiegelbildlich `TelemetryClient.cpp:313-328` (`stop()` ab `:285`), `:328 thread.detach();`.

Der Thread hält die Laufzeit, `ControlClient.cpp:81-86`:
```
    auto kern = k;
    …
    thread = std::thread ([kern, meinLauf, meine] {
        kern->threadLauf (meinLauf, std::move (meine));
    });
```
Die Laufzeit hält die Produkt-Lambdas als `std::function` (`controlclient/Laufzeit.h:119-123`: `helloProvider`, `beiAntwort`, `statusProvider`, `beiLinkStatus`, `beiVersionierterAntwort`).

Rohe `this`-Captures des Prozessors, `eq-copilot/plugin/src/PluginProcessor.cpp:112-121`:
```
      controlV3 ([this] { return v3Hello(); }, v3PipeName,
                 {},
                 [this] { return v3Status(); },
                  [this] (bool verbunden) { v3ControlLink (verbunden); },
                  [this] (const std::string& json, std::uint8_t schemaMinor)
                  { v3Antwort (json, schemaMinor); }, brokerServerErwartung()),
      telemetryV3 ([this] { return v3TelemetryHello(); }, v3PipeName,
```
Destruktor `:324-336`:
```
EqCopilotProcessor::~EqCopilotProcessor()
{
    brokerLifecycle.stop();
    telemetryV3.stop();
    controlV3.stop();
```

Gegenseite/Riegel: `controlclient/Verbindung.cpp:835-841` schaltet **vor** dem Aufruf:
```
            // Nach `stop()` wird kein Callback mehr gerufen (`B-CC-10`).
            if (! sollAbbrechen (generation))
            {
                if (beiVersionierterAntwort)
                    beiVersionierterAntwort (antwort, e.kopf.schemaMinor);
```
und `:851-853` für `statusProvider()`.

**Urteil: Auslöser folgt aus dem Code.** `B-CC-10` verhindert nur den **Start** eines Callbacks nach `stop()`; die Prüfung ist check-then-call, und ein bereits laufender Callback wird nicht zurückgeholt. `abgeloest()` (`Laufzeit.h:53-56`) sperrt nur das Schreiben gemeinsamen Zustands, nicht den Zugriff auf den fremden Empfänger. Es bleibt die Bedingung „Callback läuft > 2000 ms". Dafür gibt es einen im Register bereits belegten Weg: **NAK-185** nennt `NakamaVertrag.cpp:487` `teil()` als Θ(k²)-Stringaufbau über bis zu 262-KB-Token — und dieser Aufbau liegt **innerhalb** von `v3Antwort` (verifiziert: `NakamaVertrag.cpp:486-493`, das Lambda `teil` in `textriegel`).

**2. Bestehende Tests**

`eq-copilot/plugin/tests/IpcTestMain.cpp:4400-4437` (Bein **B10** `EqCopIpcTest`) fährt genau die Detach-Frist mit einem blockierenden Callback und prüft `dauerMs < 3500` sowie `stopFristUeberschritten >= 1`. Der Test misst aber ausdrücklich die **Client**-Lebensdauer, nicht die des Callback-Empfängers, `:4426-4428`:
```
            // Der Client ist zerstoert, der abgeloeste Thread laeuft noch: er
            // haelt seine Laufzeit selbst und beruehrt den Client nicht mehr.
```
Der Callback ist ein Testlambda über `shared_ptr<atomic>`; kein Test lässt einen Produkt-Callback über die Frist stehen und baut dabei den Prozessor ab. Ergänzend `:4447-4491` (2b) und `:4493 ff.` (2c) — beide zum abgelösten Lauf, nicht zum Empfänger. **Fall nicht abgedeckt.**

**3. Welche Zusage bricht**

Keine Zeile in Entwurf oder Schemata regelt den Besitz laufender Produkt-Callbacks. Der Header setzt die eigene Zusage bewusst enger (`ControlClient.h:664`: „…nach `kStopFristMs` ABLOESEN statt ihn zu…"). Nächstliegend wieder `Entwurf:3549` („Thread-/Race-Stress"). **Keine ausdrückliche Zusage** → Härtung/Lücke, formal aber Speichersicherheit.

**4. Ticket/Bereich**

`core/ipc/**` gehört zu NAK-123 (Serverauth), NAK-180/NAK-181 (Sendezustand, Modulaufteilung NAK-121). **Bereits im Register**, `docs/offene-punkte.md:210`:
> `| NAK-184 | 09-05, Gate-Lauf G4 … **[Härtung · Nebenläufigkeit]** **Verlorene Weckung und Detach-Callback im ControlClient.** … `:727-742`: ein nach Fristablauf detachter Thread ruft `std::function`-Callbacks, die den Prozessor fangen — Use-after-free, wenn ein Callback länger als 2 s läuft (ausgelöst über NAK-185/NAK-150, nicht über die Weckung). Kein Satz; Kandidat für G6 (Nebenläufigkeitsprüfung). |`

D2 ist damit **kein neuer Befund**, sondern die Bestätigung von NAK-184 (die dort genannten Zeilennummern stammen aus der Datei vor der NAK-121-Aufteilung; heute `ControlClient.cpp:119-135` und `controlclient/Verbindung.cpp:835-841`). Der Auslöserpfad NAK-185 steht in `:211`.

**5. Überschneidung mit Etappe 4a**

**Nein** — `eq-copilot/plugin/core/ipc/**` und `eq-copilot/plugin/src/**` sind in 4a ausdrücklich gesperrt.

**6. Fixumfang**

`core/ipc/ControlClient.cpp`, `core/ipc/ControlClient.h`, `core/ipc/TelemetryClient.cpp/.h`, `controlclient/Verbindung.cpp`, `src/PluginProcessor.cpp` (Callback-Besitz statt rohem `this`) — geschätzt 150–300 Zeilen über sechs Dateien, plus Test in `tests/IpcTestMain.cpp` (~120 Zeilen). Beine danach: **B10**, **A4-SI**, **A22**, **B8**, **A1/A16** (Regression Passthrough), **A4** (Rust-Gegenseite unberührt, aber A4-SI fährt beides).

---

## D3 — ACK-Übernahme in Projektstate hängt am Editor-Timer

**1. Quellkette, zitiert**

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1060-1073` (ACK wird nur vorgemerkt):
```
void EqCopilotProcessor::v3Antwort (const std::string& json,
                                    std::uint8_t schemaMinor)
{
    nakama::ipc::GelesenesCommandAck ack;
    if (nakama::ipc::commandAckHaeltVertrag (json, ack))
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        const auto it = ausstehendeSourcesCommands.find (ack.commandId);
        if (it != ausstehendeSourcesCommands.end())
        {
            if (ack.erfolgreich)
                bestaetigteSourcesCommands.push_back (it->second);
```
`:1108-1113` (einziger Drain):
```
void EqCopilotProcessor::sourcesTick()
{
    wendeBestaetigteSourcesCommandsAn();
```
`grep sourcesTick` über `eq-copilot/**` liefert genau **einen** Produktaufrufer: `eq-copilot/plugin/src/PluginEditor.cpp:247` (`processor.sourcesTick();` in `timerCallback()`, Timer `startTimerHz (30)` in `:230`), plus drei Testaufrufe. Der Editor-Destruktor `:233-239` stoppt den Timer implizit mit dem Objekt.

`eq-copilot/plugin/src/prozessor/State.cpp:63-67`:
```
void EqCopilotProcessor::getStateInformation (juce::MemoryBlock& ziel)
{
    std::lock_guard<std::mutex> l (bindungMutex);
    nakama::state::speichere (zustand, ziel);
}
```
`:79-83` (Reload leert die Warteliste):
```
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        ausstehendeSourcesCommands.clear();
        bestaetigteSourcesCommands.clear();
```
`Ipc.cpp:1327-1369` (`wendeBestaetigteSourcesCommandsAn`) ist die einzige Stelle, die `zustand.mainProjectMitglieder` ändert und `meldeHostDirty()` + `v3StateRevision.fetch_add (1)` auslöst (`:1365-1367`).

**Urteil: Auslöser folgt aus dem Code.** Kein zweiter Drain, kein Timer im Prozessor, kein Aufruf aus `getStateInformation`. Fenster besteht auch bei offenem Editor (bis 33 ms).

**2. Bestehende Tests**

`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` (Bein **B14** `EqCopSonde012ProjectReloadTest`), Fall `confirmed_join_ack_and_name_each_mark_host_dirty`. Die Gegenprüfung des Audits stimmt wörtlich — nach jedem ACK steht ein manueller Tick, `:294-296` und `:304-305`:
```
        vor.v3AntwortFuerTest (ack (commandId (fremdCommand), false));
        vor.sourcesTick();
        …
        vor.v3AntwortFuerTest (ack (bindId, true));
        vor.sourcesTick();
```
Erst danach `vor.getStateInformation (state);` (`:322`). Der Ablauf „ACK ohne Tick → speichern" wird **nirgends** gemessen.

**3. Welche Zusage bricht**

`docs/FL-Nakama-Sonden-Design-Entwurf.md:4273-4275`, Exit-Gate P3 (§57):
> „**Exit-Gate:** Save/Reload, Duplicate, Bridge/PID-Wechsel, zwei offene Projekte, Brokerneustart und 60-Minuten-Soak erhalten richtige Mitgliedschaft ohne falsche Steueradresse."

Dazu `CLAUDE.md`, Tragende technische Invarianten: „**State bleibt verlustfrei.** … Jede persistente Änderung meldet dem Host Dirty-State." Und `docs/beweise/NAK-123.md:78` (C-10) setzt den „Persistenz-Tick" als Teil der geprüften Kette voraus. **Zusage vorhanden und verletzt.**

**4. Ticket/Bereich**

SONDE-012 (Sources/MainProject, Bein B14), fortgeschrieben in SONDE-014 (`docs/beweise/SONDE-014.md:481` benennt genau diesen Testfall als Dirty-Beweis) und NAK-181 (`docs/beweise/NAK-181.md:1230` beschreibt die `setStateInformation`-Leerung). Registerpunkt zu genau dieser Editor-Abhängigkeit: **nicht gefunden**; verwandt ist `NAK-169` (`:198`, „Das Plugin setzt seinen Experimentzustand vor dem Broker-ACK").

**5. Überschneidung mit Etappe 4a**

**Nein** — `plugin/src/prozessor/Ipc.cpp`, `State.cpp`, `PluginEditor.cpp` liegen unter `eq-copilot/plugin/src/**` (in 4a gesperrt).

**6. Fixumfang**

`eq-copilot/plugin/src/prozessor/Ipc.cpp` (editorunabhängiger Drain, z. B. im vorhandenen Worker aus `Analyse.cpp` oder vor `speichere`), `eq-copilot/plugin/src/prozessor/State.cpp`, ggf. `PluginProcessor.h` — ~40–80 Zeilen; Test in `tests/Sonde012ProjectReloadTest.cpp` ~60–100 Zeilen. Beine: **B14**, **B13**, **B10**, **B2**, **A12**, **B8**.

---

## D4 — `false` und spätere Ausführung widersprechen sich über Modulgrenzen

**1. Quellkette, zitiert**

`eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:157` (`sendePersistenzP0`), Retention `:184`:
```
        inFlight.push_back (InFlightEintrag { commandId, json, 0, true });
```
Überlauf `:203-211`:
```
        if (! p0.einreihen (P0Eintrag { json, P0Klasse::ereignis, wireGeneration.load(), marke }))
        {
            inFlight.back().inQueue = false;
            verworfeneMarke = marke;
            ueberlauf = true;
```
`:216-226`:
```
    if (ueberlauf)
    {
        …
        if (beiP0Verworfen && verworfeneMarke != 0)
            beiP0Verworfen (verworfeneMarke);
        aktuelleVerbindung()->ioAbbrechen();
        return false;
```
Replay `:230 ff.` (`inFlightNachReconnect`, „reiht ihn nach dem naechsten Verbindungsaufbau unter DERSELBEN `command_id` erneut ein (idempotent, NR-10)", Kommentar `:188-200`).

Aufrufer `eq-copilot/plugin/src/prozessor/Ipc.cpp:1309-1323`:
```
        ausstehendeSourcesCommands.emplace (auftrag.commandId, auftrag);
    }
    if (controlV3.sendePersistenzP0 (auftrag.json))
        return true;
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    ausstehendeSourcesCommands.erase (auftrag.commandId);
    return false;
```
Fehlender Rückweg: der Aufräumhaken existiert (`ControlClient.cpp:180-186 setzeAuftragAbgeschlossenHook`, gerufen aus `Nachrichten.cpp:356-357`), wird vom Prozessor aber **nicht mehr gesetzt** — `PluginProcessor.cpp:298-302` erklärt seinen Wegfall („Beides ist mit E-15 fort"). `grep setzeAuftragAbgeschlossenHook eq-copilot/plugin/src/` liefert nur Kommentare.

Deckelung: `controlclient/Laufzeit.h:212` `std::vector<InFlightEintrag> inFlight;` — **kein Cap**; `core/ipc/IpcQueues.h:34` `inline constexpr std::size_t kCapP0 = 64;`. Die Aussage des Audits stimmt.

**Urteil: Auslöser folgt aus dem Code.**

**2. Bestehende Tests**

`eq-copilot/plugin/tests/IpcTestMain.cpp:5936-5953` (Bein **B10**) verlangt genau Retention + Replay — aber für die Familie `user_verdict`, nicht für `confirm_join`/`unbind_probe`:
```
        const bool abgewiesen = gefuellt > 0
                             && ! control.sendePersistenzP0 (userVerdictBefehl (hex32 ('7')));
        …
        pruefe (wiederholt,
                "user_verdict_ueberlebt_die_volle_p0_queue_und_wird_nach_dem_reconnect_angewandt");
```
`eq-copilot/plugin/tests/Sonde014AssistentTest.cpp:921-935` (Bein **B29**) prüft separat, dass der Aufrufer den `false`-Rückgabewert auswertet (WN-05). **Die Integrationskette „false → Aufrufer löscht Zuordnung → Replay → ACK ohne Zuordnung" misst niemand.** Gegenprüfung des Audits bestätigt.

**3. Welche Zusage bricht**

`docs/FL-Nakama-Sonden-Design-Entwurf.md:4120` (§53.9, Queuepolitik):
> `| P0 Control | 64 | nichts verwerfen; Verbindung schließen, Preview lokal auslaufen lassen |`

und `docs/beweise/SONDE-014.md:823` (M-73): „P0: 64er-Queue, Überlauf ⇒ Verbindung wird verworfen und der Eintrag geht an `beiP0Verworfen` — nie stillschweigend gelöscht." Beide Zusagen hält der **Client** ein. Verletzt wird die Zusage aus `Entwurf:4273-4275` (§57 Exit-Gate, s. D3), weil die brokerseitig angewandte Mitgliedschaft lokal nicht nachgeführt wird. Für die widersprüchliche Bedeutung des `bool` selbst: **keine Zusage** → Härtung.

**4. Ticket/Bereich**

Clientseite: SONDE-014 (M-73, WN-05) und NAK-180. Aufruferseite: SONDE-012/SONDE-014 (`prozessor/Ipc.cpp`). Registerpunkt zu diesem Widerspruch: **nicht gefunden**; verwandt `NAK-91` (`:155`, „P1 ist unter Ende-zu-Ende-Last ungeprüft").

**5. Überschneidung mit Etappe 4a**

**Nein** (`core/ipc/**` und `plugin/src/**` gesperrt).

**6. Fixumfang**

`core/ipc/controlclient/Nachrichten.cpp` + `ControlClient.h` (dreiwertiger Rückgabezustand statt `bool`, Deckel für `inFlight`), `src/prozessor/Ipc.cpp` (Zuordnung nicht löschen bzw. Wiederaufnahme) — ~80–160 Zeilen; Tests in `IpcTestMain.cpp` und `Sonde012ProjectReloadTest.cpp` ~120 Zeilen. Beine: **B10**, **B14**, **B29**, **A4-SI**, **A22**.

---

## D5 — Snapshot-Zustellung ist nach dem Commit nicht mehr geordnet

**1. Quellkette, zitiert**

`broker/src/coordinator/flush.rs:135-155`:
```
        // Die Reihenfolge ist bis einschliesslich Store-/Outbox-Commit
        // serialisiert. Externe Pipe-Arbeit laeuft danach ohne dieses Schloss;
        …
        drop(_flush_guard);
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        for (link_id, ziel) in ziele {
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, &ziel)
                && push
                    .as_ref()
                    .is_some_and(|push| push.snapshot_schreiben(&link_id, &payload));
```
`broker/src/transport/server_v3/griff.rs:72` / `:86` (Sender, fester Schlüssel):
```
    pub fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        …
        let Some(antwort) = ausgang.snapshot_einreihen_mit_antwort("session_snapshot", frame)
```
`broker/src/transport/server_v3/queues.rs:161-167` (Koaleszierung, altersblind):
```
            } else if let Ausgangsart::Snapshot(objekt_schluessel) = &eintrag.art {
                if let Some(position) = g.0.iter().position(|alt| {
                    matches!(&alt.art, Ausgangsart::Snapshot(alt_schluessel)
                        if alt_schluessel == objekt_schluessel)
                }) {
                    ersetzt = Some(std::mem::replace(&mut g.0[position], eintrag));
```
Gegenprüfung Riegel: `broker/src/coordinator/subscription.rs:377-401` (`push_ziel_noch_gueltig`) prüft Link, Trennen, Routing, Adresse, `project_binding_id`, `session_epoch`, `instance_id`, Guard und Alias — **kein Alter, keine Sequenz**.

Consumer `eq-copilot/plugin/src/SourcesModel.cpp:1152-1158`:
```
    std::lock_guard<std::mutex> l (mutex);
    if (session.toString().toStdString() != erwarteteSession
        || erwarteteBindung != erwarteteBindungLokal
        || erwarteteSession != erwarteteSessionLokal)
    {
        fehler = "session_snapshot belongs to another session";
```
Vertragsebene: `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `$defs.session_snapshot.properties` = `beitritt_bestaetigung_noetig, broker_epoch, experimente, findings, fuehrendes_main, mitglieder, paare, session_epoch, store_degraded, type` — **kein Sequenz-, Revisions- oder Ordinalfeld**. Die Aussage „eine monotone Snapshotfolge reist nicht mit" ist auf Vertragsebene bewiesen.

**Urteil: Auslöser folgt aus dem Code.** Der Session-Shard-Lock (`flush.rs:157-166 session_flush_shard`) serialisiert Commit, nicht Zustellung.

**2. Bestehende Tests**

`broker/tests/store_crash_matrix.rs:1155-1206`, `snapshot_commit_bleibt_bei_konkurrierenden_flushes_monoton` (Bein **A4-SI**). Der Test fährt genau das Interleaving, prüft danach aber **nur die Store-Projektion**, `:1197-1205`:
```
    let (_, payload) = writer
        .handle()
        .session_state_lesen(&si_hex(1), &si_hex(2))
        …
    assert_eq!(
        payload["mitglieder"][0]["probe_descriptor"]["label"],
        "neu-committed"
    );
```
Die `PushProbe` (`:396-416`) beobachtet zwar `snapshots()`, wird hier aber nicht auf die Zustellreihenfolge geprüft. Die Gegenprüfung des Audits stimmt. `cleanup_zwischen_zielermittlung_und_write_verhindert_den_alten_push` (`:1210`) prüft Zielgültigkeit, nicht Alter.

**3. Welche Zusage bricht**

`docs/FL-Nakama-Sonden-Design-Entwurf.md:3551` (verbindliche Prüfebene):
> `| IPC | Schema-/Fuzztests, Fragmentierung, Reconnect, Drop/Reorder/Duplicate, Backpressure |`

und `:4248-4249` (Exit-Gate P2/R0): „Null-/Realtime-Guards, EBU-/Analyse-Goldens, fragmentierte Frames, Reconnect, **Drop/Reorder/Duplicate**, 32-Sonden-Soak und Store-Killmatrix bestehen." **Zusage vorhanden**, allerdings als Prüfachse formuliert, nicht als Zustellinvariante.

**4. Ticket/Bereich**

`broker/src/coordinator/flush.rs` und `transport/server_v3/queues.rs` gehören zu **NAK-121** (Modulaufteilung/Reentranz, `docs/beweise/NAK-121.md`), Vertragsteil SONDE-011/SONDE-012. Registerpunkt: **NAK-165** (`docs/offene-punkte.md:194`) nennt den geteilten Queue-Key ausdrücklich („beim `V3Sender` teilen beide Payloads den Queue-Key … Wer es angeht: … eigener Queue-Key je `object_key`") — deckt D5 aber nur mittelbar; die Alterslosigkeit innerhalb desselben Keys steht dort nicht.

**5. Überschneidung mit Etappe 4a**

**Nein** — `broker/**` ist in 4a gesperrt.

**6. Fixumfang**

`broker/src/coordinator/flush.rs`, `broker/src/transport/server_v3/griff.rs`, `broker/src/transport/server_v3/queues.rs` (Ordnungsmarke bis in die Queue), optional `eq-copilot/schemas/v3/eq-ipc-v3.schema.json` + `SourcesModel.cpp` (Consumer-Riegel) — ~80–200 Zeilen; wenn das Schema angefasst wird, kommen Fixtures (`erzeuge_v3_fixtures.py`) dazu. Beine: **A4**, **A4-SI**, **A5**, **A8**, **B3c**, **B13**, **A22**; bei Vertragsänderung zusätzlich **A21**.

*Achtung Landmine:* `eq-ipc-v3.schema.json` liegt mit **CRLF** im Index (`.gitattributes`-Mischbaum, CLAUDE.md-Landmine vom 10.09.2026).

---

## D6 — Projekt-Reload leert erweiterten Sitzungszustand nicht vollständig

**1. Quellkette, zitiert**

`eq-copilot/plugin/src/SourcesModel.cpp:401-436` (`projektReload`) leert:
```
    erwarteteBindung.clear();
    erwarteteSession.clear();
    eigeneMainId.clear();
    brokerEpoch.clear();
    fuehrendesMain.clear();
    hauptziel.clear();
    subscriptionAktiv = false;
```
— und **nicht** `experimente`, `paare`, `befunde`, `evidenzRuecknahmen`, `ruecknahmeGrund`, `ruecknahmeUmfang`, `zeile.findingsOffen`.

Diese werden ausschließlich in `beginneSubscription` geleert, `:439-470`:
```
    experimente.clear();
    paare.clear();
    …
    befunde.clear();
    …
        e.zeile.findingsOffen = 0;
    evidenzRuecknahmen = 0;
    ruecknahmeGrund.clear();
    ruecknahmeUmfang.clear();
```
Aufrufer: `prozessor/State.cpp:111` (`sourcesModel.projektReload ({});` im **read-only**-Zweig) und `:126` (Vollrestore). `beginneSubscription` wird nur aus `prozessor/Ipc.cpp:1032` gerufen — hinter dem Riegel `:1032` davor:
```
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
        return;
```
Ein read-only geladener State ist nicht `main` (State.cpp:106-108: „read-only ist kein vollstaendiger State-Restore. Zurueck auf neutral"), und ohne Broker gibt es ohnehin keinen `v3ControlLink(true)`.

Sicht gibt die Listen aus, `:1560-1566`:
```
    s.experimente = experimente;
    s.befunde = befunde;
    s.paare = paare;
    s.evidenzRuecknahmen = evidenzRuecknahmen;
    s.ruecknahmeGrund = ruecknahmeGrund;
    s.ruecknahmeUmfang = ruecknahmeUmfang;
```
`controlEnde()` (`:472-501`) macht Befunde `stale` und Messungen `stale`, entfernt aber weder `experimente` noch `paare`.

**Urteil: Auslöser folgt aus dem Code.**

**2. Bestehende Tests**

`grep projektReload eq-copilot/plugin/tests/` → **kein Treffer**; die Methode wird nur mittelbar über `setStateInformation` gefahren (`Sonde012ProjectReloadTest.cpp:322`, `:337`, `:467`, Bein **B14**). `Sonde012SourcesModelTest.cpp` (Bein **B13**) befüllt `experimente`/`paare`/`befunde` (`:527`, `:792-817`), prüft aber nie einen anschließenden Reload. **Fall nicht abgedeckt** — Gegenprüfung des Audits bestätigt.

**3. Welche Zusage bricht**

`docs/beweise/SONDE-013.md:540` (M-50):
> „Reihenfolge: Store lesen → unveränderliche Referenzen wiederherstellen → offenen Zustand anzeigen → **keine** stillschweigende Fortsetzung einer Messung über den Neustart hinweg"

Der Produktcode wendet M-50 selbst auf genau diesen Projektwechsel an, `prozessor/State.cpp:94-99`:
```
    // 🔑 NAK-181 R3 (G4-Befund V03, M-50): der Projektwechsel beendet den
    // Vergleichszustand — VOR dem Tausch von `zustand`.
    …
    // genau das, was M-50 mit „keine stillschweigende Fortsetzung einer
    // Messung ueber den Neustart hinweg" ausschliesst.
```
**Zusage vorhanden und verletzt** — der Vergleichszustand wird geleert, die Sitzungslisten des Modells nicht.

**4. Ticket/Bereich**

SONDE-012 (Modell, B13/B14), erweitert um `experimente`/`paare`/`befunde` in SONDE-013 Nacharbeit 2 (R14/R32, Kommentar `SourcesModel.cpp:1558-1560`) und NAK-214 R3/R8 (`:494-505`). Registerpunkt: **nicht gefunden** (verwandt: NAK-186, `:212`).

**5. Überschneidung mit Etappe 4a**

**Nein** (`plugin/src/**` gesperrt).

**6. Fixumfang**

`eq-copilot/plugin/src/SourcesModel.cpp` (gemeinsame Reset-Helferfunktion für beide Wege, ~20–40 Zeilen), Test in `tests/Sonde012ProjectReloadTest.cpp` oder `Sonde012SourcesModelTest.cpp` (~60 Zeilen). Beine: **B13**, **B14**, **B28**, **B26**.

---

## D7 — Stereokanäle werden als zusätzliche Messdauer gezählt

**1. Quellkette, zitiert**

`eq-copilot/plugin/core/analysis/Vergleichspegel.h:35-37`:
```
    /** Wie viel Material der Pegel mindestens braucht, bevor er einfrierbar
        ist: 400 ms. Kürzer misst man einen Transienten und nennt ihn Pegel. */
    static constexpr double kMindestSekunden = 0.4;
```
`:70` (Umrechnung Zeit → Samples): `mindestSamples = (std::uint64_t) (kMindestSekunden * fs);`

`:92` `void speise (const float* a, const float* b, int n) noexcept` — pro Aufruf ein Kanalpaarabschnitt; Zähler `:117-121`:
```
                summeA += x * x;
                summeB += y * y;
                ++gesehen;
            }
            ++bloeckeAufgenommen;
```
`:314-321` (Readiness):
```
    bool bereitIntern() const noexcept
    {
        …
        return nichtEndlich == 0
            && gesehen >= mindestSamples && summeA > 0.0 && summeB > 0.0;
    }
```
Aufrufer, ein Aufruf je Kanal — `eq-copilot/plugin/src/PluginProcessor.cpp:777-782`:
```
    if (pegelSpeist)
        for (int c = 0; c < kanaele; ++c)
            vergleichspegel.speise (
                versuchTrocken.data() + (std::size_t) c * (std::size_t) buffer.getNumSamples()
                    + (std::size_t) pegelVon,
                buffer.getReadPointer (c) + pegelVon,
                pegelBis - pegelVon);
```

**Urteil: Auslöser folgt aus dem Code**, zusätzlich vom Audit gegen den unveränderten Header reproduziert (Exit 1, zwei Verletzungen). Kein Riegel: `mindestSamples` kennt die Kanalzahl nicht. **Zweitwirkung, vom Audit nicht genannt:** `bloeckeAufgenommen` (`:121`) zählt ebenfalls je Kanal, also doppelt bei Stereo — `versuchAufgenommeneBloecke()` ist damit ebenfalls kanalabhängig.

**2. Bestehende Tests**

Bein **B23** `EqCopSonde013PassageStateTest`. `Sonde013PassageStateTest.cpp:3449-3462` prüft nur die **negative** Vorbedingung:
```
        const auto bloeckeVorBeginn = p->versuchAufgenommeneBloecke();
        pruefe ((double) bloeckeVorBeginn * (double) kBlock
                    < nakama::analyse::Vergleichspegel::kMindestSekunden * kFs,
                "N-05 Vorbedingung: der lebende Pegel hat noch nicht genug "
                "Material, um einen Gain einzufrieren",
```
Der Helfer `fahreBisPegel` (`:300-310`) wartet auf `versuchAufgenommeneBloecke() >= 60` — also über den Bereitschaftspunkt hinaus, ohne ihn zu messen. Alle Puffer sind stereo (`:852`, `:926`, `:978`, …). **Der exakte Bereitschaftszeitpunkt und der Mono/Stereo-Unterschied werden von keinem Bein geprüft.**

**3. Welche Zusage bricht**

Erstens die Zusage im Typ selbst (`Vergleichspegel.h:35-37`, oben zitiert). Zweitens `docs/FL-Nakama-Sonden-Design-Entwurf.md:3550`, verbindliche Prüfebene:
> `| Analyse | Standardreferenzen, Sampleraten, **Mono/Stereo**, zu kurze/stille/unterbrochene Passagen |`

Das ist eine ausdrücklich zugesagte Prüfachse, die den Fall trägt. **Zusage vorhanden und verletzt.**

**4. Ticket/Bereich**

SONDE-013 (Vergleichspegel, Passage/Experiment; `docs/beweise/SONDE-013.md`, `docs/beweise/NAK-181.md`). Register: zwei benachbarte, **andere** Zeilen existieren —
`docs/offene-punkte.md:187` **NAK-158** („Der Vergleichspegel nimmt nach einem Seek … einen Block der neuen Epoche") und `:188` **NAK-159** („`friereEin` kann einen halben Stereoblock übernehmen. `PluginProcessor.cpp:549-553` ruft `speise` je Kanal …"). NAK-159 sitzt an derselben Aufrufstelle und nennt das Je-Kanal-Muster, benennt aber die Halbierung der Messdauer **nicht**. D7 ist insofern neu, gehört aber sachlich zu NAK-159.

**5. Überschneidung mit Etappe 4a**

**Nein.** `Vergleichspegel.h` liegt in `plugin/core/analysis` und wird von `plugin/sonde/` und `plugin/dsp/` nicht referenziert (grep: kein Treffer); `PluginProcessor.cpp` liegt im gesperrten `plugin/src/**`.

**6. Fixumfang**

`eq-copilot/plugin/core/analysis/Vergleichspegel.h` (Frames statt Samples zählen bzw. `speise` mit beiden Kanälen; ~20–40 Zeilen), `eq-copilot/plugin/src/PluginProcessor.cpp` (Aufruftopologie, ~10 Zeilen — schließt NAK-159 mit), Tests in `tests/Sonde013PassageStateTest.cpp` (~60–100 Zeilen). Beine: **B23**, **B24**, **B26**, **B25**, **B29**, **A1**.

---

## D8 — Bereichsprüfung kommt nach der Konvertierung

**1. Quellkette, zitiert**

`eq-copilot/plugin/state/NakamaParameter.cpp:388-396`:
```
        // Die `id` ist eine GANZE Zahl. JSON kennt den Unterschied nicht, der
        // Vertrag schon: 3.5 ist keine Zonenidentitaet, sondern ein Fehler.
        if (! std::isfinite (id->zahl) || id->zahl != std::floor (id->zahl))
        {
            grund = "bereich"; detail = wo + ".id";
            return false;
        }
        Schutzzone z;
        z.enabled = enabled->b;
        z.id      = (int) id->zahl;
```
`eq-copilot/plugin/state/NakamaPreset.cpp:243-247`:
```
        if (! std::isfinite (id->zahl) || id->zahl != std::floor (id->zahl))
        {
            grund = "bereich"; detail = woZone + ".id";
            return false;
        }
        s.zonen.push_back ({ (int) id->zahl, lowHz->zahl, highHz->zahl, enabled->b });
```
Typen: `NakamaKanon.h:34` `double zahl = 0.0;` — die Quelle ist ein `double`; `Schutzzone::id` ist `int`.

Bereichsprüfung **danach**, `NakamaParameter.cpp:202` (`validiereZonen`) `:219`:
```
        if (e.id < 0 || e.id >= kMaxZonen)      { grund = "bereich"; woId = wo + ".id";      return false; }
```
Aufgerufen aus `NakamaParameter.cpp:264`, `:534`, `NakamaPreset.cpp:250`, `NakamaState.cpp:1289` — jeweils **nach** dem Cast.

Rust-Gegenweg, `broker/src/dto.rs:461-475`:
```
        let id = o["id"].as_f64().ok_or(Grund::Typ)?;
        if !id.is_finite() || id.fract() != 0.0 {
            return Err(Grund::Bereich);
        }
        ids.push(id as i64);
    …
        if !(0..MAX_ZONEN as i64).contains(&ids[i]) {
            return Err(Grund::Bereich);
        }
```
**Urteil: Auslöser folgt aus dem Code.** Für `id = 2147483648` (endlich, integral) ist `(int) id->zahl` in C++20 undefiniertes Verhalten. Auf MSVC ergibt es praktisch `INT_MIN`, was `e.id < 0` auffängt — die *beobachtbare* Klassifikation stimmt dann zufällig mit Rust überein, die Konvertierung selbst bleibt UB.

**2. Bestehende Tests**

`eq-copilot/plugin/tests/StateMigrationTestMain.cpp` (Bein **B2**) prüft Zonenkanten — aber über direkt gebaute C++-`Schutzzone`-Werte, also **nach** dem Cast, `:743-755`:
```
                z.zonen.push_back ({ i, 100.0, 200.0, true });
            pruefe (! param::stateHash (z, hash, grund) && grund == "zone_anzahl", …
            …
            z.zonen[0] = { param::kMaxZonen, 100.0, 200.0, true };
```
Fixturekorpus `eq-copilot/fixtures/state/dto/ungueltig/`: `zone-id-8.json`, `zone-doppelte-id.json`, `zone-unsortiert.json`, `zone-unter-20-hz.json`, `zone-ueber-20-khz.json`, `zone-low-gleich-high.json`, `zone-neunte.json`, `zone-enabled-als-string.json`, `zone-zusatzfeld.json`, `schutz-zonen-fehlt.json` — **kein Fixture mit `-1`, `2147483648` oder einer großen sicheren JSON-Ganzzahl.** Fall nicht abgedeckt.

**3. Welche Zusage bricht**

`CLAUDE.md`, Tragende technische Invarianten: „**Schemas sind Verträge.** Neue persistente Felder zuerst versionieren; Altstände laden; unbekannte Felder dürfen alte Consumer nicht zerstören." — trifft nur mittelbar. Näher: `docs/FL-Nakama-Sonden-Design-Entwurf.md:3551` (IPC-Prüfebene „Schema-/Fuzztests") und die Cross-Language-Zusage aus `broker/tests/contract_cross_language.rs` (beide Leser klassifizieren denselben Korpus gleich). **Für „Bereich vor Cast" gibt es keine ausdrückliche Zusage** → Härtung mit UB-Charakter. Register: `docs/offene-punkte.md:344` **NAK-42** notiert bereits, dass Rust die ungültigen DTO-Fixtures nicht klassifiziert — dieselbe Naht.

**4. Ticket/Bereich**

**SONDE-015 Etappe 2** (`docs/beweise/SONDE-015.md` nennt `state/NakamaParameter` und `state/NakamaPreset` als Ticketpfade). Register: NAK-42 (verwandt), sonst nicht gefunden.

**5. Überschneidung mit Etappe 4a**

**Nein, aber knapp daneben.** Die geänderten Dateien des laufenden Workers umfassen `NakamaTransaktion.*` (neu, laut Auftrag „z. B. unter `eq-copilot/plugin/state/`"), nicht `NakamaParameter.cpp`/`NakamaPreset.cpp`. 4a liest jedoch `NakamaParameter.h` („die Kennungen und Reihenfolge kommen aus `NakamaParameter.h`, nie aus einer zweiten Liste") und arbeitet im selben Ordner — ein Fix sollte erst nach 4a angesetzt werden.

**6. Fixumfang**

`eq-copilot/plugin/state/NakamaParameter.cpp` und `eq-copilot/plugin/state/NakamaPreset.cpp` (Bereichsprüfung vor dem Cast, je ~6–10 Zeilen), zwei bis vier neue Fixtures unter `eq-copilot/fixtures/state/dto/ungueltig/` und `…/preset/ungueltig/` plus `tools/eq-copilot/erzeuge_state_fixtures.py`, Testfälle in `tests/StateMigrationTestMain.cpp` (~40 Zeilen). Beine: **B2**, **A12**, **B3c**, **A4** (Rust-Klassifikation gleicher Fixtures), **B6**, **B7**.

*Achtung Landmine:* Fixtures unter `eq-copilot/fixtures/**` sind `-text` mit gemischten Zeilenenden — vor dem Schreiben `git ls-files --eol` lesen.

---

## D9 — Rücknahmeereignisse teilen sich den Schlüssel mit Vollsnapshots

**1. Quellkette, zitiert**

`broker/src/coordinator/invalidierung_verdrahtung.rs:384-406` (`invalidierung_zustellen`):
```
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        for ziel in &wirkung.ziele {
            …
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, ziel)
                && push
                    .as_ref()
                    .is_some_and(|push| push.snapshot_schreiben(&link_id, &payload));
```
Derselbe Sender wie der Vollsnapshot — `griff.rs:86` reiht **beide** unter dem festen Objektschlüssel `"session_snapshot"` ein (zitiert unter D5). Ersetzung in `queues.rs:161-167`; die verdrängte Nachricht bekommt `false`, `:194-200`:
```
        if let Some(alt) = ersetzt {
            // Der alte Snapshot ist absichtlich NICHT geschrieben. Seine
            // Store-Schuld bleibt bestehen, …
            if let Some(antwort) = alt.geschrieben {
                let _ = antwort.send(false);
```
**Wichtige Präzisierung gegenüber dem Audit:** die Kollision besteht **nur in der Writerqueue**. Auf Store-/Outbox-Ebene trägt die Rücknahme einen eigenen Schlüssel — `invalidierung_verdrahtung.rs:219`: `object_key: "evidence_invalidate".into(),` und `:366`: `event.event_type = "evidence_invalidate".into();`. Deshalb bleibt die Schuld erhalten und wird beim nächsten Subscribe nachgespielt (`subscription.rs:212-231`, `:300-330` `outbox_wireform` mit Zweig `"evidence_invalidate"`).

Wirkung beim Consumer, `eq-copilot/plugin/src/SourcesModel.cpp:1421-1437`:
```
    std::lock_guard<std::mutex> l (mutex);
    for (auto& [_, e] : eintraege)
        if (e.zeile.messung != Messung::missing)
            e.zeile.messung = Messung::invalid;
    …
    setzeAlleBefundeStale();
```
**Urteil: Auslöser folgt aus dem Code.** Folge ist Verzögerung bis zum nächsten Subscribe/Evidenztakt, nicht endgültiger Verlust — die Einordnung des Audits stimmt.

**2. Bestehende Tests**

Bein **A4** (`cargo test` Broker) und **A4-SI**. Die Zustellung selbst ist geprüft (`broker/tests/sonde013_taint.rs`, `sonde014_*`), die Schuld und ihr Replay ebenfalls (`subscription.rs`-Pfad, B17). **Eine Koaleszierung genau dieser beiden Nachrichtenarten in einer angehaltenen Writerqueue prüft kein Test** — die `PushProbe` in `store_crash_matrix.rs:396-416` kann blockieren, wird für diesen Fall aber nicht eingesetzt. Gegenprüfung des Audits bestätigt.

**3. Welche Zusage bricht**

`docs/FL-Nakama-Sonden-Design-Entwurf.md:4121` (§53.9):
> `| P1 Zustand/Evidenz | 128 | Snapshots nach Objektschlüssel koaleszieren; nicht koaleszierbare Events bei Überlauf über Reconnect/Outbox wiederholen |`

Die Zusage lautet „nach **Objektschlüssel** koaleszieren". Der Sender vergibt für zwei verschiedene Objekte denselben Schlüssel — das ist die Verletzung, sauber zitierbar. Dazu `:3551` („Drop/Reorder/Duplicate").

**4. Ticket/Bereich**

SONDE-013 Nacharbeit 2 (Befund R27/R28, Kommentare in `invalidierung_verdrahtung.rs:358-366` und `prozessor/Ipc.cpp:1075-1082`), NAK-213/NAK-214 (`docs/beweise/NAK-213.md`, `NAK-214.md`), Transport aus NAK-121.
**Bereits im Register**, `docs/offene-punkte.md:194`:
> `| NAK-165 | 09-05 … **Der Re-Subscribe spielt Outbox-Deltas auch nach gescheitertem absolutem Snapshot-Push nach.** `subscription.rs:179` … **beim `V3Sender` teilen beide Payloads den Queue-Key.** … Wer es angeht: Replay erst nach erfolgreichem absolutem Snapshot, **eigener Queue-Key je `object_key`**. |`

D9 ist damit die Bestätigung und Zuspitzung von **NAK-165**; verwandt `NAK-166` (`:195`, eigener Outbox-Key je Experimentterminal).

**5. Überschneidung mit Etappe 4a**

**Nein** (`broker/**` gesperrt).

**6. Fixumfang**

`broker/src/transport/server_v3/griff.rs` (Objektschlüssel als Parameter), `broker/src/coordinator/invalidierung_verdrahtung.rs`, ggf. `broker/src/coordinator/proposal_verdrahtung.rs` und `flush.rs` (Aufrufer) — ~40–90 Zeilen; Test in `broker/tests/store_crash_matrix.rs` oder `sonde013_taint.rs` (~80 Zeilen). Gemeinsam mit D5 lösbar. Beine: **A4**, **A4-SI**, **B13**, **A22**.

---

## D10 — Vier belegte Slots sind kein Vier-Bank-Rechenbeweis

**1. Quellkette, zitiert**

`eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4065-4102` (B6-Messabschnitt):
```
    std::cout << std::endl << "== N - Messwert des Vier-Bank-Falls (R15, kein erfundener Deckel) ==" << std::endl;
    …
        const int blockGroesse = 256, bloecke = 4000;
    …
        kern->uebernehmeZustand (voll, Pfad::committed);
        fahreStille (*kern, blockGroesse, blockGroesse);
        kern->uebernehmeZustand (voll, Pfad::candidate);
        fahreStille (*kern, blockGroesse, blockGroesse);
        auto voll2 = voll;
        setzeGlobal (voll2, "v1.global.output_trim_db", -2.0);
        kern->uebernehmeZustand (voll2, Pfad::committed);
        fahreStille (*kern, blockGroesse, blockGroesse);
        kern->uebernehmeZustand (voll2, Pfad::candidate);

        pruefe (kern->pool().freieSlots() == 0,
                "der Messlauf faehrt wirklich vier Baenke (R15)",
```
`eq-copilot/plugin/dsp/DspProgramm.cpp:138-158` (`rampenKompatibel`) vergleicht nur Samplerate, Mono-Bass-Schalter und je Slot `aktiv/typ/modus/dynamisch/nutztSvf/quelle` — ein reiner `output_trim_db`-Wechsel ist damit **rampenkompatibel**.

`eq-copilot/plugin/dsp/DspKern.cpp:321-338` (Übergangswahl):
```
    const bool nurRampen = alt >= 0 && rampenKompatibel (baenke.bank (alt).programm, bankNeu.programm);
    …
    z.uebergang = nurRampen ? Uebergang::rampe : Uebergang::crossfade;
    z.rest      = nurRampen ? kRampeSamples : kFadeSamples;
```
`eq-copilot/plugin/dsp/DspKern.cpp:659-676` (zweiter Bankdurchlauf nur bei Crossfade):
```
    const bool crossfade = z.uebergang == Uebergang::crossfade && z.rest > 0;
    …
    if (crossfade)
    {
        if (z.quelle >= 0 && ! istPassthrough (z.quelle))
        {
            …
            verarbeiteBank (z, z.quelle, -1, 0, vonL, vonR, numSamples);
```
`eq-copilot/plugin/dsp/DspProgramm.h:57`: `inline constexpr int kRampeSamples = 256;` — genau die Blockgröße des Messabschnitts; `fahreStille(…, 256, 256)` verbraucht die Committed-Rampe, die Candidate-Rampe endet im ersten Messblock. Die übrigen 3 999 Blöcke publizieren nichts.

`eq-copilot/plugin/dsp/DspBankPool.cpp:99-106` (`freieSlots`) zählt nur `BankZustand::frei` — belegt heißt nicht rechnend.

**Urteil: Auslöser folgt aus dem Code.** Der Messabschnitt misst durchgehend zwei aktive Bankdurchläufe. Die Ausgabezeile behauptet mehr, als der Pfad tut — `roh/SONDE-015-etappe3-nacharbeit3-beine.txt:287`: „MESSWERT Vier-Bank-Fall: 0.241 s fuer 21.333 s Audio … = 1.128 % Echtzeit (acht dynamische Baender, Committed und Candidate, **je im Fade**)".

**2. Bestehende Tests**

Bein **B6** `EqCopDspGoldenTest` ist selbst der Nachweis; ein anderer misst den Fall nicht. Der Bankpool-Rennen-Abschnitt darüber (`DspGoldenTestMain.cpp:4030-4060`, `bankpool_parallel_ohne_rennen (M-122)`) prüft Invarianten und Generationen, keine Rechenlast.

**3. Welche Zusage bricht**

`docs/beweise/SONDE-015.md:1222` (M-118):
> „die gemessene Last des Vier-Bank-Falls (**Committed plus Candidate, gleichzeitiger Topologiewechsel, acht belegte dynamische Bänder**) steht als **Messwert** im Manifest … | **BELEGT** · **ENTSCHIEDEN (R15)** · BAULÜCKE | **NEU** **B6**, Fall `worst_case_vier_baenke_laeuft`"

und die Quelle dazu, `docs/FL-Nakama-Sonden-Design-Entwurf.md:2986-2987`:
> „Ein gleichzeitiger Topologiewechsel kann für die begrenzte Fadezeit drei beziehungsweise im Worst-Case vier Bänke benötigen. Diese Last ist Teil des Worst-Case-CPU-Goldens."

Der gemessene Ablauf enthält **keinen** gleichzeitigen Topologiewechsel. **Zusage vorhanden und verletzt** — es ist ein Nachweisfehler, kein DSP-Defekt. Ebenfalls betroffen: `SONDE-015.md:1462` („CPU-Last des Vier-Bank-Falls | Messwert aus B6 (M-118)").

**4. Ticket/Bereich**

**SONDE-015 Etappe 3** (B6, R15/M-118, M-46 `SONDE-015.md:1099`). Registerpunkt: nicht gefunden — der Befund ist neu und trifft ein am 10.09.2026 abgenommenes Manifest.

**5. Überschneidung mit Etappe 4a**

**Ja.** Der laufende Worker ändert `eq-copilot/plugin/dsp/DspKern.*`, `eq-copilot/plugin/CMakeLists.txt` (B7-Ziel „nach dem Muster von B6") und `tools/beweise.ps1` (Eintrag **B7** bei `:689`, direkt neben dem B6-Eintrag `:688`). Der 4a-Auftrag verlangt außerdem ausdrücklich, **B6 mitzufahren**. Ein D10-Fix in `DspGoldenTestMain.cpp` kollidiert nicht direkt mit den geänderten Dateien, wohl aber der begleitende Runner-/CMake-Eintrag. **Nach 4a ansetzen.**

**6. Fixumfang**

`eq-copilot/plugin/tests/DspGoldenTestMain.cpp` (echter Topologiewechsel auf beiden Pfaden mit überlappenden Crossfades, Bankidentitäten je gewerteter Region belegen, Übergangsregion getrennt messen: ~100–180 Zeilen), ggf. eine Testschnittstelle in `eq-copilot/plugin/dsp/DspKern.h`/`DspBankPool.h` für „rechnende Bankidentitäten" (~15–30 Zeilen), Nachtrag im Manifest `docs/beweise/SONDE-015.md` (M-118, §4.6, Kopftabelle). Beine: **B6**; danach **A1**, **A16**, **B7**, **B2**, **A12**, **A14** (Kernbibliothek wird neu übersetzt).

---

# Ressourcenrisiko, Optimierungen und offene Nachweise

**R1 — Historischer Brokerzustand vollständig im RAM.** Stimmt an der Quelle: `broker/src/store/handle.rs:301-318` (`domaene_lesen`) fährt `"SELECT state_jcs FROM passages ORDER BY last_event_ord"` (analog `experiments`, `evidence`) **ohne LIMIT** und sammelt in `Vec<Vec<u8>>`; die Restaurierer sind `coordinator/experiment_verdrahtung.rs:1372` und `coordinator/experiment.rs:882`. Die Funktion trägt selbst die Begründung („`Coordinator::mit_store` restaurierte ausschliesslich die Konfliktriegel; Experimentstore und Evidenzhistorie starteten immer leer … M-47 und M-50 sagen aber ausdruecklich zu, dass ein offener Versuch den Neustart ueberdauert"). Zusage zum Speicherbudget: `docs/FL-Nakama-Sonden-Design-Entwurf.md:3579` nennt nur „Soak | mindestens 60 min mit 16, mindestens 30 min mit 32 Sonden **ohne XRun/ungegrenztes Wachstum**" — das ist eine Wachstumszusage über die Laufzeit, keine über die Startspitze. **Beobachtung korrekt; keine Zusage zur Startspitze → Härtung/Lücke.**

**O1 — Zwei Analysepfade in der Sonde.** Stimmt: `eq-copilot/plugin/sonde/SondeProcessor.cpp:365-377` speist je freigegebenem Block sowohl `merkmale` (FeatureEngine) als auch `analyseEngine.verarbeite(...)`; `grep analyseEngine\.` über die Datei zeigt als einzigen Verbraucher `:383 const auto lautheit = analyseEngine.lautheitFuerTelemetrie();`. Der teure Teil liegt in `eq-copilot/plugin/src/AnalyseEngine.cpp:403-420` (8×-Polyphase True Peak je Sample und Kanal) plus den Welch-Stufen ab `:423`. Keine Zusage verlangt zwei Pfade; `CLAUDE.md` sagt nur „Überlast verwirft Analyseframes, nie Audio". **Härtung/Optimierung, korrekt beobachtet.** — **Überschneidung 4a: ja**, `SondeProcessor.cpp` wird gerade geändert.

**O2 — Komplexität an denselben Grenzen.** Die Zahl ist bereits registriert: `docs/offene-punkte.md:250` **NAK-235** „**28 Funktionen über 200 Zeilen, ohne Pflegeticket** (Ratsche 28 in `gesundheit.py` …)", dazu **NAK-236** (`:251`, Kommentar-Bezeichner) und **NAK-238** (`:253`, Dateien über dem Ziel 1 500). Die Schwellen stehen in `tools/plan/gesundheit.py:183-185` (`ZEILEN_GRENZE = 2000`, `ZEILEN_ZIEL = 1500`, `FUNKTION_GRENZE = 200`). **Beobachtung korrekt, Zusage nur als Pflegeregel (CLAUDE.md „Codebase-Gesundheit ist Planbestandteil"), kein Defekt.**

**O3 — Gesundheitsprüfer deckt neue Produktionsverzeichnisse nicht ab.** Exakt bestätigt. `tools/plan/gesundheit.py:176`:
```
QUELLORTE = ("broker/src", "eq-copilot/plugin/src", "eq-copilot/plugin/core")
```
Eigene Zählung im Snapshot über `plugin/{state,dsp,sonde,vertrag,hostbridge}`: 32 Dateien / 15 107 Zeilen, davon generiert `vertrag/generiert/nakama_telemetry_v1_generated.h` mit 1 547 Zeilen → **31 handgeschriebene Dateien, 13 560 Zeilen** — die Zahlen des Audits stimmen auf die Zeile. `eq-copilot/plugin/state/NakamaState.cpp` = **2 863** Zeilen, also über `ZEILEN_GRENZE = 2000`, aber außerhalb des Filters. **Keine Zusage verletzt (die Regel wurde nie auf diese Datei angewandt) → Lücke im Nachweis, korrekt eingeordnet.**

**M1 — Host-/Callback-Performance offen.** Korrekt: `tools/beweise.ps1:614` fährt A24 mit `@('--sonden','16','--minuten','2','--neustarts','1')`, während `docs/FL-Nakama-Sonden-Design-Entwurf.md:3579` „mindestens 60 min mit 16, mindestens 30 min mit 32 Sonden" nennt — und `:3573-3576` stellt diese Budgets ausdrücklich als „Startwerte des Users beziehungsweise des Entwurfs, **änderbar**" unter Erratum (f). Die Einordnung „kein erfundener Produktvertragsverstoß" ist damit richtig. B6 misst tatsächlich nur `dauer > 0.0 && std::isfinite (anteil)` (`DspGoldenTestMain.cpp:4131-4133`). **Beobachtung korrekt; D10 muss zuerst.**

**M2 — Kleine Hostblöcke gegen zeitgesteuertes Polling.** Stimmt an der Quelle: `eq-copilot/plugin/core/StampedAudioQueue.h:240` `using GenStrom = StreamLayout<1, 131072, 2048, 16384>;` (2 048 Blockdeskriptoren); `eq-copilot/plugin/src/prozessor/Analyse.cpp:337-339` wartet `50` ms, `eq-copilot/plugin/sonde/SondeProcessor.cpp:435-437` `20` ms; beide erst, wenn die Queue leer ist (`queueHatRest` → `yield`). Der Audiopfad weckt nicht: `workerWarte.notify_all()` steht nur in `PluginProcessor.cpp:333` (Destruktor) und `SondeProcessor.cpp:115/136` (Start/Stopp) — nie in `processBlock`. Rechnung 2048/48000 = 42,67 ms bzw. 10,67 ms bei 192 kHz nachvollzogen. Zusage: `CLAUDE.md` „Überlast verwirft Analyseframes, nie Audio" erlaubt Drops ausdrücklich. **Keine Zusage verletzt → Lücke im Nachweis.** — **Überschneidung 4a: ja** (`SondeProcessor.cpp`).

**V1 — Statewriter-Headroom prüft nur Bytes.** Bestätigt: `eq-copilot/plugin/state/NakamaState.cpp:1108-1121` prüft im Lambda `passt` ausschließlich `return bytes.getSize() <= kMaxStateBytes;`. Die zusätzlichen Lesergrenzen stehen unabhängig davon in `:85-86`:
```
constexpr int kMaxEintraegeJeSammlung = 65536;
constexpr int kMaxEintraegeGesamt = 262144;
```
Sie gehen in `hatWriterHeadroom` nicht ein. **Beobachtung korrekt; ohne Grenzfixture unentscheidbar, wie der Audit selbst sagt.**

**V2 — Ältere Hypothesenrechnung überschreibt neuere.** Bestätigt und präzisierbar: `broker/src/coordinator/hypothese_verdrahtung.rs:72-89` sammelt unter Lock, rechnet ohne Lock, trägt unter Lock ein. Der Riegel `:961-997` `ergebnis_ist_noch_gueltig` prüft genau zwei Dinge — Intentgeneration (`:974-979`) und dass jede verwendete `evidence_id` noch gültig ist (`:993-996`). Ein Vergleich gegen **neu hinzugekommene** Evidenz fehlt; A's älteres Ergebnis passiert beide Prüfungen und überschreibt B's neueres (`:475 stand.befunde.insert(...)`). Mildernd: bei neuer Evidenz wird `stand.befunde_neu_bilden` gesetzt und `hypothesen_bei_bedarf_bilden` (`:108-117`) rechnet neu — die Inversion heilt am nächsten Takt, sofern das Flag nicht bereits verbraucht war. **Unentscheidbar ohne Laufzeit; Verdacht bleibt berechtigt, keine Zusage nennt eine vollständige Input-Generationsprüfung.**

---

# Übersicht

| Befund | Auslöser folgt aus dem Code | Verletzte Zusage (Quelle:Zeile) | Ticket/Bereich | Übersch. 4a | Fixumfang (Dateien, Beine) |
|---|---|---|---|---|---|
| **D1** Markierungsring | **ja**, aber nur bei einer Unterbrechung über vier UI-Publikationen; kein Riegel, nur Kommentar `HoerMarkierung.h:16-18` | **keine** direkte; nächstliegend Prüfebene `Entwurf:3549` („Thread-/Race-Stress") | SONDE-013 / NAK-47 / NAK-180 R4; kein Registerpunkt gefunden | nein | `HoerMarkierung.h`, `MarkierungTestMain.cpp` (~110–210 Z.) → **A3, A1, A16, B21** |
| **D2** Callback-Lebensdauer | **ja** (Riegel `B-CC-10` `Verbindung.cpp:836` verhindert nur den Start, nicht die Fortsetzung) | **keine** direkte; `Entwurf:3549` | `core/ipc/**`, NAK-123/180/181 — **bereits offen als NAK-184** (`offene-punkte.md:210`), Auslöser NAK-185 (`:211`) | nein | `ControlClient.*`, `TelemetryClient.*`, `controlclient/Verbindung.cpp`, `PluginProcessor.cpp`, `IpcTestMain.cpp` (~270–420 Z.) → **B10, A4-SI, A22, B8, A1/A16** |
| **D3** ACK am Editor-Timer | **ja** (einziger Drain `PluginEditor.cpp:247`) | **`Entwurf:4273-4275`** (§57 Exit-Gate „Save/Reload … erhalten richtige Mitgliedschaft"); `CLAUDE.md` „State bleibt verlustfrei" | SONDE-012 (B14) / SONDE-014; kein Registerpunkt gefunden | nein | `prozessor/Ipc.cpp`, `prozessor/State.cpp`, `Sonde012ProjectReloadTest.cpp` (~100–180 Z.) → **B14, B13, B10, B2, A12, B8** |
| **D4** `false` + Replay | **ja** (`Nachrichten.cpp:216-226` vs. `Ipc.cpp:1319-1322`; Aufräumhaken seit E-15 ungenutzt) | Client hält `Entwurf:4120` / M-73 ein; verletzt wird `Entwurf:4273-4275`. Für die Bool-Semantik **keine Zusage** | SONDE-014 (M-73/WN-05) + SONDE-012; kein Registerpunkt gefunden | nein | `controlclient/Nachrichten.cpp`, `ControlClient.h`, `prozessor/Ipc.cpp`, 2 Tests (~200–280 Z.) → **B10, B14, B29, A4-SI, A22** |
| **D5** Zustellreihenfolge | **ja** (`flush.rs:137` entsperrt vor Zustellung; kein Alter im Schema) | **`Entwurf:3551`** („Drop/Reorder/Duplicate"), **`:4248-4249`** | Broker-Transport NAK-121 / SONDE-011-012; mittelbar **NAK-165** | nein | `flush.rs`, `griff.rs`, `queues.rs` (+ ggf. Schema/`SourcesModel.cpp`) (~80–200 Z.) → **A4, A4-SI, A5, A8, B3c, B13, A22** |
| **D6** Reload-Reset unvollständig | **ja** (`projektReload:401-436` vs. `beginneSubscription:439-470`; read-only erreicht `Ipc.cpp:1032` nie) | **`SONDE-013.md:540`** (M-50 „keine stillschweigende Fortsetzung"), vom Code selbst zitiert in `State.cpp:94-99` | SONDE-012 + SONDE-013 Nacharbeit 2 + NAK-214; kein Registerpunkt gefunden | nein | `SourcesModel.cpp`, ein Test (~80–100 Z.) → **B13, B14, B28, B26** |
| **D7** Stereo halbiert die Messdauer | **ja**, zusätzlich reproduziert; `bloeckeAufgenommen` zählt ebenfalls je Kanal | **`Vergleichspegel.h:35-37`** („400 ms") und **`Entwurf:3550`** (Prüfachse Mono/Stereo) | SONDE-013; benachbart **NAK-159** (`:188`, dieselbe Aufrufstelle) und NAK-158 | nein | `Vergleichspegel.h`, `PluginProcessor.cpp`, `Sonde013PassageStateTest.cpp` (~90–150 Z.) → **B23, B24, B25, B26, B29, A1** |
| **D8** Cast vor Bereichsprüfung | **ja** (UB; auf MSVC zufällig gleiche Klassifikation wie Rust) | **keine** direkte; Cross-Language-Vertrag mittelbar, Register **NAK-42** | **SONDE-015 Etappe 2** (`state/NakamaParameter`, `state/NakamaPreset`) | nein, aber selber Ordner (4a legt `NakamaTransaktion.*` dort ab und liest `NakamaParameter.h`) | 2 Quelldateien (~15 Z.), 2–4 Fixtures + `erzeuge_state_fixtures.py`, `StateMigrationTestMain.cpp` (~60 Z.) → **B2, A12, B3c, A4, B6, B7** |
| **D9** Rücknahme teilt Queue-Key | **ja**, Kollision **nur in der Writerqueue** (`griff.rs:86`); Outbox trägt eigenen Key (`invalidierung_verdrahtung.rs:219`) → Verzögerung, kein Verlust | **`Entwurf:4121`** („Snapshots nach **Objektschlüssel** koaleszieren") | SONDE-013 R27/R28, NAK-213/214, NAK-121 — **bereits offen als NAK-165** (`:194`), verwandt NAK-166 | nein | `griff.rs`, `invalidierung_verdrahtung.rs`, ggf. `proposal_verdrahtung.rs`/`flush.rs`, 1 Test (~120–170 Z.) → **A4, A4-SI, B13, A22** |
| **D10** Vier-Bank-Nachweis | **ja** (`rampenKompatibel` → `Uebergang::rampe`, zweiter Bankdurchlauf nur bei `crossfade`, `DspKern.cpp:659-676`) | **`SONDE-015.md:1222`** (M-118 „gleichzeitiger Topologiewechsel"), Quelle **`Entwurf:2986-2987`** | **SONDE-015 Etappe 3** (B6, R15/M-118); kein Registerpunkt | **ja** — `DspKern.*`, `CMakeLists.txt`, `tools/beweise.ps1` (B6/B7 benachbart) sind in Arbeit | `DspGoldenTestMain.cpp` (~100–180 Z.), ggf. Testschnittstelle in `DspKern.h`/`DspBankPool.h` (~30 Z.), Manifestnachtrag → **B6**, danach **A1, A16, B7, B2, A12, A14** |

**Zusatzzeilen (kein D-Befund):**

| Punkt | Beobachtung an der Quelle | Zusage | Übersch. 4a |
|---|---|---|---|
| R1 | korrekt, `store/handle.rs:301-318` ohne LIMIT | keine zur Startspitze (`Entwurf:3579` nur Soak-Wachstum) → Härtung | nein |
| O1 | korrekt, `SondeProcessor.cpp:365-377` + `:383`, `AnalyseEngine.cpp:403-420` | keine → Optimierung | **ja** |
| O2 | korrekt, `gesundheit.py:183-185` | bereits **NAK-235/236/238** | nein |
| O3 | exakt korrekt (31 Dateien / 13 560 Z. nachgerechnet; `NakamaState.cpp` 2 863 Z.) | keine → Nachweislücke | nein |
| M1 | korrekt (`beweise.ps1:614` 2 min vs. `Entwurf:3579` 60/30 min, dort Erratum (f) „änderbar") | keine → offener Nachweis | nein |
| M2 | korrekt (`StampedAudioQueue.h:240`, `Analyse.cpp:337`, `SondeProcessor.cpp:435`; kein Wecken aus `processBlock`) | `CLAUDE.md` erlaubt Framedrops → Lücke | **ja** |
| V1 | korrekt (`NakamaState.cpp:1108-1121` nur Bytes; Grenzen `:85-86` ungeprüft) | offen, kein Fixture → Verdacht | nein |
| V2 | korrekt und präzisiert (`:961-997` prüft Intentgeneration + Evidenzgültigkeit, nicht neue Evidenz; heilt am nächsten Takt) | keine → unentscheidbar ohne Laufzeit | nein |

**Drei Dinge, die der Dirigent aus dieser Prüfung mitnehmen sollte:** D2 und D9 sind seit dem 05.09.2026 als **NAK-184** bzw. **NAK-165** im Register — der Audit bestätigt sie, erfindet sie nicht. D10 trifft ein **am 10.09.2026 abgenommenes** Manifest (M-118, SONDE-015 Etappe 3) und ist der einzige Befund mit direkter Überschneidung zur laufenden Etappe 4a. D1 ist der einzige Befund, dessen Auslöserkette der Audit stärker darstellt, als der Code sie hergibt — die fehlende Besitzregel ist echt, das Argument „braucht keine ungewöhnlich schnelle Bedienung" ist es nicht.
