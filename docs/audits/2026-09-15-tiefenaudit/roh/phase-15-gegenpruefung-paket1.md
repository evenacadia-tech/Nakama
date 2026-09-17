# Tiefenaudit 3, Phase 15 — Gegenprüfung Paket 1 (Skeptiker)

> **Rolle dieses Berichts:** Widerlegungsprüfung zu Paket 1 im Tiefenaudit 3, Phase 15
> (17.09.2026, Quellstand `e008811efa7650f3e0cc2332cc21f9e268cdd125`). Ein anderer
> lesender Prüfer als der Finder hat jeden Kandidaten gezielt zu widerlegen versucht;
> sein Urteil ist für Kategorie und Schwere verbindlich, soweit
> [phase-15-zentral.md](phase-15-zentral.md) nichts anderes begründet. Kandidat `K1` bis
> `K4` entspricht `P15-V-01` bis `P15-V-04` in
> [phase-15-paket1-verspaetete-ergebnisse.md](phase-15-paket1-verspaetete-ergebnisse.md).
> Nur lesend, keine dynamische Reproduktion.


HEAD `e008811efa7650f3e0cc2332cc21f9e268cdd125`, nur lesend, kein Bau, kein Lauf.
Alle unten genannten Stellen habe ich in dieser Session selbst geöffnet.

---

## K1 — Positiver Sources-ACK wird nach `reconnect()` vom Transport verbraucht

**URTEIL: BESTÄTIGT** (Zusage neu verankert)
**Kategorie: DEFEKT · Schwere: medium**

### Was ich zu brechen versucht habe

**(a) Quelle — gescheitert, Ablauf hält.**
`inFlightAck (antwort)` steht in `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:834`,
die Generationsprüfung `if (! sollAbbrechen (generation))` erst in `:836`;
`sollAbbrechen` ist `! laeuft || verbindungsGeneration != generation`
(`Verbindung.cpp:147-150`). `inFlightAck` löscht den Eintrag bedingungslos, wenn
keine Konfliktwiederholung greift (`controlclient/Nachrichten.cpp:362-363`).
`inFlightNachReconnect` iteriert nur über `inFlight` (`Nachrichten.cpp:264-273`) —
der gelöschte Eintrag wird nicht nachgespielt. Kein Recheck, keine Rückstellung.

**Gesuchter Heilpfad — gefunden und widerlegt.** `Nachrichten.cpp:374-384` ruft
`hookAuftragAbgeschlossen (commandId)`; dieser Haken wäre genau die Freigabe der
Zuordnung im Prozessor. Er ist aber **nicht gesetzt**: `setzeAuftragAbgeschlossenHook`
(`core/ipc/ControlClient.cpp:180-186`) hat im ganzen Produktcode keinen Aufrufer;
`eq-copilot/plugin/src/PluginProcessor.cpp:405-415` und `PluginProcessor.h:1610-1636`
begründen ausdrücklich, dass er mit Entscheid E-15 entfernt wurde. Nur Tests setzen ihn
(`tests/IpcTestMain.cpp:6951`, `:7009`, `:7138`). Der blockierende Eintrag bleibt also
wirklich stehen.

**Zweiter gesuchter Heilpfad — nicht vorhanden.** `zustand.mainProjectMitglieder`
hat genau drei Schreiber: den Drain bestätigter Befehle (`src/prozessor/Ipc.cpp:1433`,
`:1440`), den rein lokalen Unbind ohne Broker (`Ipc.cpp:1273`) und das Leeren beim
Rollenwechsel (`src/prozessor/State.cpp:322`). Kein Sessionsnapshot-Pfad führt die
Mitgliedschaft nach. Der Drain (`Ipc.cpp:1358-1412`) hilft nicht, weil er nur
`bestaetigteSourcesCommands` verarbeitet, und die Liste wird nur im unterdrückten
Callback gefüllt (`Ipc.cpp:1073-1075`). `mainDarfSchreiben` (`SourcesModel.cpp:1637-1639`)
ist kein Gegenriegel, sondern wird nach dem Reconnect wieder wahr.

**Fenster — größer als vom Finder angenommen, nicht kleiner.** Der Riss liegt nicht
nur zwischen `:834` und `:836`. `ControlClient::reconnect()` erhöht die Generation in
`ControlClient.cpp:154` und ruft `ioAbbrechen()` erst in `:160`; ein bereits
zurückgekehrter Read wird danach noch vollständig ausgewertet. Die Empfangsschleife
`for (;;)` in `Verbindung.cpp:778-843` leert außerdem den gesamten `StromLeser`-Puffer:
liegt vor dem ACK ein Sessionsnapshot im selben Batch, hält dessen Callback
(`Ipc.cpp:1096` → `SourcesModel::uebernehmeSessionSnapshot`, JSON-Parse auf dem
IO-Thread) das Fenster über die ganze Parsedauer offen. Das Fenster ist damit
Millisekunden-, nicht Instruktionsbreite.

**(b) Zusage — teilweise erfolgreich, Verankerung verschiebt sich.**
NAK-246 M-16 (`docs/beweise/NAK-246.md:623`) beschreibt wörtlich die Kette
„volle Queue → Join → Reconnect → ACK → Speichern → Laden": dort trifft der ACK in der
**neuen** Generation ein und der Callback läuft. K1 spielt in der **sterbenden**
Generation. Eine engere, vertretbare Lesart von M-16 deckt K1 also nicht. Was trägt,
ist das Schadensbild D4 (`NAK-246.md:69`) und die dort genannte Grundlage: Entwurf §57
Exit-Gate („Save/Reload … erhalten richtige Mitgliedschaft") sowie CLAUDE.md
„State bleibt verlustfrei … Jede persistente Änderung meldet dem Host Dirty-State".
Dazu kommt CLAUDE.md „Keine toten UI-Elemente" für den zweiten Handgriff.

**(c) Test — keiner gefunden.** Kein Bein treibt einen Generationswechsel gegen einen
laufenden Sources-ACK. `v3ReconnectFuerTest` (`PluginProcessor.h:761`) wird nur in
`tests/Sonde013PassageStateTest.cpp` benutzt, nicht im Sources-ACK-Zusammenhang.

**(d) Erreichbarkeit — real, aber Koinzidenz.** Vier Auslöser bestätigt:
Reconnect-Knopf (`src/PluginEditor.cpp:193-197` → `Ipc.cpp:1120-1123`), Host-Prepare
(`src/prozessor/Hostbruecke.cpp:146`, **bedingungslos**, Message-/Hostthread),
Rollenwahl (`State.cpp:361`), Kennungswechsel (`State.cpp:65`). `prepareToPlay` ist der
gefährlichste, weil der Host ihn ungefragt fährt. Der zweite Handgriff ist wirklich
gesperrt: `Ipc.cpp:1336-1341` weist jeden weiteren Befehl derselben `instanceId` ab,
bis `State.cpp:135` die Liste beim Reload leert.

**(e) Duplikat — nein, aber dicht an zweien.**
T3-03-03 nennt dieselbe Zeile `Verbindung.cpp:834`, meint aber den fehlenden
Zahlenriegel am ACK; sein Fix (Textriegel vor der Klassifikation) behebt K1 nicht.
T3-07-04 ist die Gegenrichtung (Callback läuft, `unknown_target`, Eintrag wird
gelöscht, Knopf bleibt stumm). Am nächsten liegt **Register NAK-278**
(`docs/offene-punkte.md:288`): dort ist genau die Broker/lokal-Divergenz der
Mitgliedschaft als **LÜCKE** eingeordnet, ausdrücklich mit „Keine Zusage regelt den
Fall" — allerdings für den Reload-Pfad, in dem der User den Handgriff wiederholen
kann. K1 ist der Nicht-Reload-Fall und fügt hinzu, was NAK-278 gerade nicht hat: die
Wiederholung ist gesperrt.

**(f) Kategorie und Schwere — halten.** Die Divergenzhälfte allein wäre nach NAK-278
LÜCKE; die dauerhaft gesperrte Bedienung trägt den Defekt (CLAUDE.md „Keine toten
UI-Elemente"). medium bleibt richtig: Rennen mit Koinzidenzbedarf, kein Datenverlust
im Audio, durch Projekt-Reload auflösbar.

**Stärkstes Gegenargument, das ich habe:** M-16 deckt den Fall wörtlich nicht, und
NAK-278 hat die gleichartige Divergenz bereits als „keine Zusage" eingeordnet. Ohne die
gesperrte Wiederholung wäre K1 herabzustufen.

**Selbst gelesen:** `Verbindung.cpp:147-150`, `:404-412`, `:760-844`, `:846-899`;
`Nachrichten.cpp:240-295`, `:297-400`; `ControlClient.cpp:120-186`;
`prozessor/Ipc.cpp:1020-1123`, `:1240-1300`, `:1300-1412`; `prozessor/State.cpp:120-223`,
`:287-362`; `prozessor/Hostbruecke.cpp:130-160`; `PluginProcessor.cpp:380-446`;
`PluginProcessor.h:1595-1639`; `SourcesModel.cpp:1610-1664`;
`docs/beweise/NAK-246.md:60-78`, `:615-640`; `docs/offene-punkte.md:288`.

---

## K2 — Messpunktwechsel über State-Reload wird nie erkannt

**URTEIL: BESTÄTIGT** (im Umfang sogar weiter als behauptet, in der Wirkung enger)
**Kategorie: DEFEKT · Schwere: medium**

### Was ich zu brechen versucht habe

**(a) Quelle — gescheitert; die Kette ist vollständig und der Code sagt es selbst.**
`broker/src/coordinator/link.rs:253-257` setzt `descriptor: None` beim Hello mit
ausdrücklichem Kommentar („Auch ein Reconnect derselben Runtime muss den … ersten
heartbeat.runtime erneut liefern"). Die Evidenz bleibt: sie wird nur bei
**abweichender** Nonce entfernt (`link.rs:132-143`), und `control_ende` entfernt sie
nur im Zweig `join_reconnect_ohne_tombstone` (`link.rs:536-556`), nicht beim normalen
Client-Reconnect. Der Vergleich in `liveness.rs:423-425` ist `if let (Some(a), Some(n))`;
der Deskriptor wird danach ersetzt (`liveness.rs:471-472`).
`SondeProcessor.cpp:1070` reconnectet bedingungslos nach dem Laden; die Nonce ist je
Instanz konstant (`SondeProcessor.cpp:1088`).

**(b) Anderer Produktweg — gesucht, keiner gefunden; der Befund wird dadurch breiter.**
- `descriptor_setzen` hat laut dem Quelltext selbst „unter `broker/src` keinen
  Aufrufer" (`liveness.rs:852-858`); der zweite `invalidierung_wegen_messpunkt`-Aufruf
  liegt genau dort (`liveness.rs:888-900`) und ist damit ebenfalls tot.
  Der Testeinstieg ist `invalidierung_wegen_messpunkt_fuer_link`
  (`invalidierung_verdrahtung.rs:748-757`, benutzt in
  `broker/src/transport/server_v3/tests_abonnement.rs:1045`).
- Gen sendet gar keinen Runtimeblock: in `eq-copilot/plugin/src/prozessor/Ipc.cpp` gibt
  es keine `s.runtime`-Zuweisung, `descriptor_aus_heartbeat` verlangt aber
  `wert.get("runtime")` (`liveness.rs:719-721`).
- Gens eigener Positionswechsel (`State.cpp:287-362`, einziger Aufrufer ist die
  Rollenwahl im Editor) reconnectet ebenfalls (`State.cpp:361`).
- Probeeqs Position kommt allein aus `zustand.common.position`
  (`SondeProcessor.cpp:1113-1140`), und diese schreibt nur `setStateInformation`.

→ **Kein Produktweg kann den Vergleich jemals ungleich erreichen.** Der Befund ist
nicht „der Reload-Pfad fehlt", sondern „die Wache ist produktiv unerreichbar".

**(c) Ersatzwache — teilweise gefunden; das senkt die Wirkung, nicht die Kategorie.**
`broker/src/coordinator/experiment_verdrahtung.rs:904-917` vergleicht die **heutige**
`measurement_position` der Quelle gegen die Klasse, die die Passage ihr zuweist, und
überspringt abweichende Belege — wörtlich mit Gate-7-Begründung. In diesem Pfad
wandert Evidenz also nicht mit. Der PRE/POST-Pfad
(`prepost_verdrahtung.rs:85-103`) leitet die Rolle dagegen aus dem **aktuellen**
Deskriptor ab und würde alte Historie umetikettieren — er ist aber heute ohnehin
unerreichbar, weil die `pair_id` den produktiven Deskriptor nicht erreicht
(bestätigter Befund T3-08-04).
Was übrig bleibt: es wird kein `evidence_invalidate` zugestellt, kein
`ausschlussgrund` gesetzt, der Invalidierungszähler bleibt 0, und eine **nach** dem
Wechsel gebildete Passage kann Belege von **vor** dem Wechsel konsumieren, weil der
Eintrag selbst keine Position trägt.

**(d) Zusage — hält gegen Umdeutung.** SONDE-013 M-55 (`docs/beweise/SONDE-013.md:550`)
sagt „Ein geänderter Messpunkt … invalidiert die abhängige Evidenz" und die
Nacharbeit 2 (R24) behauptet ausdrücklich, der Wechsel invalidiere „AUS DEM
DESKRIPTORVERGLEICH heraus — vorher hatte `invalidierung_wegen_messpunkt` ausserhalb
der Tests keinen Aufrufer". Genau diese Behauptung trägt nicht: der gemessene Fall
läuft über `descriptor_setzen`, der Produktingress kann nicht ungleich werden.

**(e) Erreichbarkeit** setzt einen Probeeq-Stateload mit geänderter Messposition auf
einer lebenden Instanz voraus (Presetwechsel). In dieser Session nicht verifiziert.

**(f) Duplikat — nein.** T3-08-04 betrifft `pair_id` und die Paarbildung, T3-14-03 die
sticky Sperre nach einem Sonden-Abbruch. Beide liegen in derselben Nachbarschaft, keiner
deckt den Messpunktvergleich.

**Stärkstes Gegenargument, das ich habe:** Die Gate-7-Wache in
`experiment_verdrahtung.rs:915` nimmt dem Befund den schlimmsten Schaden — Evidenz der
alten Position geht im Passagenurteil nicht ein. Ohne diese Wache wäre die Schwere
high.

**Selbst gelesen:** `link.rs:100-280`, `:495-617`; `liveness.rs:300-539`, `:714-760`,
`:800-909`; `invalidierung_verdrahtung.rs:701-757`; `prepost_verdrahtung.rs:75-119`;
`experiment_verdrahtung.rs:895-934`; `SondeProcessor.cpp:235-269`, `:1020-1141`;
`prozessor/State.cpp:287-362`; `docs/beweise/SONDE-013.md:545-560`.

---

## K3 — Intent-Vollbestand nach Same-Instance-Recall als `AeltereRevision` abgewiesen

**URTEIL: HERABGESTUFT**
**Kategorie: LÜCKE (vorher DEFEKT) · Schwere: low (vorher medium)**

### Was ich zu brechen versucht habe

**(a) Quelle — der Ablauf stimmt.** `broker/src/coordinator/intent.rs:504-518`:
`zuletzt = bestand.revision` für den Vollbericht, `zu_alt = revision < zuletzt` →
`Err(IntentAbweisung::AeltereRevision)`; der Spiegel bleibt unberührt. Die Sitzung ist
stabil: `v3SessionEpoch` ist `const` und entsteht einmal im Konstruktor
(`PluginProcessor.cpp:134`, `PluginProcessor.h:1908`). Der Spiegel überlebt den
Reconnect: `intent_spiegel_leeren_locked` läuft nur, wenn die Sitzung **keinen** Client
mehr hat (`link.rs:584-603`), und `control_ende` behält den Client
(`link.rs:557-564`). Die Bestandsrevision ist persistent und wird beim Laden
übernommen (`state/NakamaState.cpp:982`, `:2022`) und steigt je Änderung um genau 1
(`NakamaState.cpp:2209-2214`); `Ipc.cpp:564` liest sie aus dem geladenen Stand.
Ein Recall kann also wirklich eine kleinere Bestandsrevision erzeugen.

**(b) und (c) — hier bricht der Befund.** Es gibt einen **Test, der genau diesen Fall
als richtiges Verhalten festschreibt**:
`broker/tests/sonde014_verdrahtung.rs:510-518` — ein Vollbericht mit
`bestand_revision = 2` nach einer angenommenen 3 liefert
`Err(IntentAbweisung::AeltereRevision)` mit der Begründung „die Sitzungsrevision wird
als MAXIMUM fortgeschrieben". Die tragende Zusage ist M-85
(`docs/beweise/SONDE-014.md:882`): „Die Revisionsordnung ist total: eine ältere
Revision überschreibt nach der Koaleszierung nie eine jüngere", präzisiert durch NR-02.
Der Code folgt dieser Zusage.

Die zitierten Gegenzusagen tragen nicht:
- **M-09** (`SONDE-014.md:723`) verspricht, dass **Userwerte den Recall überleben** und
  der Plugin-State die Quelle bleibt; sein Rotbeweis ist „der Spiegel überschreibt den
  Plugin-State" — die umgekehrte Richtung. Der Plugin-State bleibt in K3 unangetastet.
- **M-86** (`:882`) verlangt, dass Main **meldet**, bevor der Broker rechnet. Main
  meldet (`Ipc.cpp:809-825`, aufgerufen in `Ipc.cpp:1059`); die Marke liegt im Spiegel
  bereits vor. Der Rotbeweis „ein Proposal entsteht vor der Marke" tritt nicht ein.
- Der Codekommentar `intent.rs:643-644` liegt unterhalb der Revisionsordnung, die in
  `:486-518` ausdrücklich vorgeschaltet ist.

**(d) Erreichbarkeit — doppelt unverifiziert.** Es braucht (1) `setStateInformation`
auf einer lebenden Gen-Instanz, (2) dieselbe `project_binding_id` (sonst wechselt der
Sitzungsschlüssel und der Spiegel ist ein anderer) und (3) eine kleinere
Bestandsrevision als die zuletzt gemeldete. Dass das Projekt „same-instance reload"
grundsätzlich als realen Fall führt, belegt der Testname
`same_instance_reload_discards_transient_snapshot_and_measurement` (NAK-246 M-13,
`docs/beweise/NAK-246.md:615`) — die konkrete Rückwärtsrevision ist damit aber nicht
belegt.

**(e) Duplikat — nein**, benachbart zu T3-02-03 (Revisionsbereiche State gegen Wire),
anderer Mechanismus.

**(f) Kategorie und Schwere.** Keine Zusage ist gebrochen — im Gegenteil, das Verhalten
ist die gemessene Zusage. Offen bleibt, dass für „Mains Bestand geht rückwärts" keine
Regel existiert und die Divergenz nicht selbst heilt (es braucht so viele Userschritte,
wie die Differenz groß ist). Das ist eine **LÜCKE**. Schwere **low**, weil die
Erreichbarkeit unverifiziert ist und die Wirkung nur in einem schmalen Eck entsteht.
Bei einem Beleg, dass FL `setStateInformation` mit älteren Bytes auf einer lebenden
Instanz fährt, wäre medium zu prüfen — dann steht die Veto-Wirkung von M-86 im Raum.

**Stärkstes Gegenargument, das ich habe:** `sonde014_verdrahtung.rs:514-518` misst das
behauptete Fehlverhalten als Zusage — ein Befund gegen diese Zeile wäre ein Befund
gegen M-85, nicht gegen M-09.

**Selbst gelesen:** `intent.rs:470-559`; `link.rs:495-617`;
`broker/tests/sonde014_verdrahtung.rs:470-519`; `prozessor/Ipc.cpp:540-585`, `:795-849`;
`prozessor/State.cpp:140-223`; `PluginProcessor.cpp:125-145`;
`state/NakamaState.cpp` (Fundstellen `:979`, `:982`, `:2022`, `:2209-2214`);
`docs/beweise/SONDE-014.md:715-730`, `:875-890`.

---

## K4 — Begonnene Snapshot-Übernahme läuft nach `controlEnde` weiter

**URTEIL: BESTÄTIGT**
**Kategorie: HÄRTUNG · Schwere: low** (beides unverändert)

### Was ich zu brechen versucht habe

**(a) Nebenläufigkeit — der stärkste Angriff scheitert.** Mein Hauptversuch war zu
zeigen, dass beide Wege auf demselben IO-Thread laufen. Sie tun es nicht:
`ControlClient::reconnect()` erhöht die Generation (`ControlClient.cpp:154`) und ruft
`meldeLinkStatus (false)` in `:162` — auf dem **Aufruferthread**.
`Verbindung.cpp:404-412` und der Kommentar in `prozessor/Ipc.cpp:855-860` sagen es
wörtlich: „`reconnect()`/`stop()` rufen den negativen synchron auf ihrem Aufruferthread
(MP3-1)". Von dort geht es über `v3ControlLink(false)` nach
`sourcesModel.controlEnde()` (`Ipc.cpp:891` bzw. `:900`). Gleichzeitig parst der
IO-Thread den Snapshot **außerhalb** des Modellmutex und nimmt ihn erst in
`SourcesModel.cpp:1235` — die Prüfung dort deckt nur Bindung und Sitzung
(`:1236-1242`). Danach `subscriptionAktiv = true` und `diagnose = keine`
(`SourcesModel.cpp:1355-1358`), `mainDarfSchreiben` wird wieder wahr
(`SourcesModel.cpp:1637-1639`). Das Rennen ist real.

**(b) Zusage — teilweise entschärfbar.** NAK-214 R3 lautet vollständig
(`docs/beweise/NAK-214.md:46`): „Verbindungsende (`controlEnde`) und Sitzungswechsel
setzen in Gen alle Befunde auf `stale` (nicht handelbar); **eine Rücknahme ohne
Folge-Snapshot** lässt keinen READY-Befund stehen." `SourcesModel.cpp:572-575` benennt
genau den nächsten vollständigen Snapshot als den vorgesehenen Rückweg. Der Snapshot,
der das Rennen gewinnt, **ist** ein vollständiger Snapshot mit der jüngsten
Brokerwahrheit. Eine vertretbare engere Lesart von R3 ist damit verfügbar; gebrochen
ist die Zusage dem Wortlaut nach nur, wenn man „Verbindungsende setzt alle Befunde
stale" als Endzustand liest.

**(c) Test — keiner gefunden**, der `controlEnde` gegen eine laufende
Snapshot-Übernahme fährt (`tests/SessionSoakMain.cpp:648`, `:663`, `:729` und
`tests/Sonde012SourcesLatencyTest.cpp:118-131` fahren beide nacheinander, nicht
überlappend).

**(d) Erreichbarkeit der Wirkung — hier bricht die Schwere.**
`darfAudition()` und `darfDraft()` (`SourcesModel.h:187`, `:189`) haben **im ganzen
Quellbaum keinen Aufrufer** — nur der Kommentar `SourcesModel.cpp:570` nennt sie. Die
Aussage „Befunde sind wieder handelbar" hat heute keinen Verbraucher. Was bleibt, ist
`mainDarfSchreiben == true` bis zum nächsten `beginneSubscription`
(`prozessor/Ipc.cpp:1035-1037`), also bis der Reconnect steht. Befehle in diesem Fenster
gehen über `sendePersistenzP0` und werden nach dem Reconnect nachgespielt
(`Nachrichten.cpp:241-255`) — kein Verlust, keine falsche Persistenz.

**(e) Duplikat — nein**, keine T3-Zeile und keine Registerzeile nennt dieses Rennen.

**(f) Kategorie und Schwere — halten.** HÄRTUNG/low ist korrekt und eher großzügig:
Rennen belegt, Wirkung ohne Verbraucher, Heilung durch den nächsten
`beginneSubscription`.

**Stärkstes Gegenargument, das ich habe:** Die einzige benannte Wirkung („Befunde
wieder handelbar") läuft über zwei Prädikate ohne jeden Aufrufer; R3 nennt den
Folge-Snapshot selbst als Rückweg.

**Selbst gelesen:** `SourcesModel.cpp:540-600`, `:1210-1270`, `:1330-1364`,
`:1605-1664`; `SourcesModel.h:187-189`, `:301-316`, `:367-381`;
`prozessor/Ipc.cpp:840-905`, `:1030-1101`; `ControlClient.cpp:120-186`;
`Verbindung.cpp:404-412`; `docs/beweise/NAK-214.md:40-50`.

---

## Gesamtbild

Kandidat · Urteil · Kategorie · Schwere · Kernbegründung

- **K1** · BESTÄTIGT · DEFEKT · medium · Reihenfolge `Verbindung.cpp:834` vor `:836` belegt; der einzige Heilhaken `setzeAuftragAbgeschlossenHook` ist seit E-15 ohne Produktaufrufer, kein Replay, kein Snapshotabgleich auf `mainProjectMitglieder`; Fenster reicht über einen ganzen Empfangsbatch. Zusage neu verankert auf Entwurf §57 plus CLAUDE.md „keine toten UI-Elemente" statt auf M-16; NAK-278 ist der Nachbar, nicht das Duplikat.
- **K2** · BESTÄTIGT · DEFEKT · medium · Kein Produktweg erreicht den Vergleich `(Some, Some)` je ungleich — Deskriptor wird bei jedem Hello genullt (`link.rs:257`), `descriptor_setzen` hat laut Quelle keinen Aufrufer, Gen sendet keinen Runtimeblock, jede Positionsänderung reconnectet. Gate-7-Wache in `experiment_verdrahtung.rs:915` verhindert den schlimmsten Schaden und hält die Schwere auf medium.
- **K3** · HERABGESTUFT · LÜCKE · low · `broker/tests/sonde014_verdrahtung.rs:514-518` misst die Abweisung als zugesagtes Verhalten; M-85 trägt die totale Revisionsordnung, M-09 und M-86 decken die Gegenrichtung. Offen bleibt nur die fehlende Regel für einen rückwärts laufenden Bestand; Erreichbarkeit unverifiziert.
- **K4** · BESTÄTIGT · HÄRTUNG · low · Nebenläufigkeit belegt (`controlEnde` auf dem Aufruferthread, Snapshot-Parse auf dem IO-Thread), aber `darfAudition`/`darfDraft` haben keinen Aufrufer und R3 nennt den Folge-Snapshot selbst als Rückweg; Heilung beim nächsten `beginneSubscription`.

## Gelesener Scope

**C++ Produkt:** `eq-copilot/plugin/core/ipc/ControlClient.cpp`,
`core/ipc/controlclient/Verbindung.cpp`, `core/ipc/controlclient/Nachrichten.cpp`,
`src/PluginProcessor.cpp`, `src/PluginProcessor.h`, `src/SourcesModel.cpp`,
`src/SourcesModel.h`, `src/prozessor/Ipc.cpp`, `src/prozessor/State.cpp`,
`src/prozessor/Hostbruecke.cpp`, `sonde/SondeProcessor.cpp`,
`state/NakamaState.cpp` (Fundstellen).
**Rust Broker:** `broker/src/coordinator/link.rs`, `liveness.rs`, `intent.rs`,
`invalidierung_verdrahtung.rs`, `prepost_verdrahtung.rs`, `experiment_verdrahtung.rs`.
**Tests (nur gelesen):** `broker/tests/sonde014_verdrahtung.rs`,
`eq-copilot/plugin/tests/` (Trefferlisten zu `setzeAuftragAbgeschlossenHook`,
`controlEnde`, `uebernehmeSessionSnapshot`, `v3ReconnectFuerTest`, `darfAudition`).
**Zusagen und Register:** `docs/beweise/NAK-246.md`, `NAK-214.md`, `SONDE-013.md`,
`SONDE-014.md`, `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`,
`docs/offene-punkte.md`, `CLAUDE.md`, `AGENTS.md`.
**Nicht gefahren:** kein Bau, kein Test, kein Kanon, kein Broker-, Pipe-, FL- oder
MCP-Aufruf. Keine Repo-Datei geändert.
