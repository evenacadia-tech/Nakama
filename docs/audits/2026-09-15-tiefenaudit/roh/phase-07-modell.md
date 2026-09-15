URTEIL: DEFEKTE 5 · LÜCKEN 1 · HÄRTUNGEN 0 — Die Modelle haben erkennbare Besitzer, doch Intent-Vorrang, Evidenzgültigkeit und mehrere Übergänge zur heutigen Bedienfläche sind nicht durchgängig zwecktreu.

# Tiefenaudit 3 — Phase 07: Modell und Fläche

| Laufangabe | Wert |
|---|---|
| Auftrag | Gebundener Prüfauftrag des Dirigenten, 2026-09-16 00:26; Kurzname modell |
| Beginn | 2026-09-16, 00:26:36 CEST (22:26:36 UTC am Vortag) |
| Ende / Dauer | Berichtsabschluss 2026-09-16 01:15:35 CEST; 48 Minuten 59 Sekunden bis zum Abschlussstand |
| HEAD / Branch | db58ee7a23b16e8b486f688e08b3764523469ca5 / master; vor dem ersten Quellzugriff kontrolliert |
| Prüfer | Codex; gpt-6-astra, Effort max laut Laufvorgabe in BEFUNDE.md:5. Kein gesondertes Startlog als weitere Belegdatei geöffnet; keine unabhängig ausgelesene Modelltelemetrie behauptet. |
| Methode | Statische Populationserfassung, Leser-/Schreiber-/Aufruferverfolgung, eigener Skeptikerpass; keine Subagenten |
| Schreibrecht | Nur dieser Bericht und der neue Abschnitt Phase 07 in BEFUNDE.md |
| Beweisgrenze | Kein Build, Test, Kanon, Installation oder FL-/EqCopShot-Lauf; neue Befunde ausschließlich aus Quellherleitung |

Die Einordnung gilt für den gebundenen Stand und garantiert keine fehlerfreie Codebase. Referenzdokumente und Register dienen nur als Zweck-, Zusagen- und Dublettenquellen. Keine Prosa-, Optik-, Figma- oder Legacy-Namensbefunde.

## Befundtabelle

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
|---|---|---|---|---|---|---|---|---|---|
| T3-07-01 | high | K4 | DEFEKT | eq-copilot/plugin/state/NakamaState.cpp:2316; eq-copilot/plugin/state/NakamaState.cpp:2497; broker/src/coordinator/intent.rs:155 | Eine abgeleitete Passagenrolle verdrängt einen expliziten globalen Userintent; dadurch kann auch dessen Verschmelzungs-Veto verschwinden. | Quellherleitung: Quelle A global mit rolle=verschmolzen/herkunft=user und für Passage P mit rolle=fuehrt/herkunft=abgeleitet speichern. setzeIntent erlaubt beide Schlüssel; wirkenderIntent und IntentBestand::wirkend liefern P ohne Herkunftsvergleich. entmaskierungErlaubt(A,B,P) wird ohne paarweise Ausnahme true. Der reine Ordnungsvergleich hat keinen Produktaufrufer; Sonde014IntentTest.cpp:521 prüft nur user gegen user. | „Die Konfliktauflösung folgt **genau der Reihenfolge aus §37.2**: 1. Schutz-/Sicherheitsgrenze, 2. expliziter Userintent, 3. passagespezifischer vor globalem Intent, 4. bestätigte Vorlage, 5. abgeleitete Vermutung.“ (SONDE-014 M-05) | Die wirksame Rolle in C++ und Rust zuerst nach Herkunftsstufe und erst innerhalb derselben Stufe nach Passage auflösen; den User-global/Inferred-Passage-Fall samt Veto prüfen. | offen |
| T3-07-02 | high | K8 | DEFEKT | eq-copilot/plugin/src/PluginEditor.cpp:438; eq-copilot/plugin/src/PluginEditor.cpp:800; eq-copilot/plugin/src/PluginEditor.cpp:245; eq-copilot/plugin/src/PluginEditor.cpp:840; eq-copilot/plugin/src/PluginProcessor.cpp:848 | bekannt NAK-179: Auch mit funktionierender Hostbrücke sind die sichtbaren Solo/Puls-Aktionen der Legacy-Fläche durch die Main-Klassifikation gesperrt; die Fläche meldet dennoch Engagement. Ein dort vorgemerkter Marker wird beim Wechsel zu hub ausführbar, während Aus-Knopf und Timer-Rücknahmen verschwinden. | Quellherleitung: Legacy-Befund → Solo/Puls → markierungEinreichen ohne Klassifikationsprüfung; DSP verlangt istMainKlassifiziert. Anschließend Messpunkt=hub: setzeBindung spiegelt main, wechsleFlaecheWennNoetig versteckt markierungAus, timerCallback kehrt im Main-Zweig vor Deadman/Sampleraten-/Kill-Behandlung zurück. Kein Aus-Auftrag beim Flächenwechsel. MarkierungTestMain.cpp:179 klassifiziert direkt main und klickt diese Oberfläche nicht. Neu gegenüber NAK-179 ist die Klassen-/Flächenkombination, unabhängig vom Hostpatch. | „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand.“ (CLAUDE.md:51) | Engage an einen ehrlichen Modellstatus binden und vor dem Verbergen der Markierungsbedienung den vorgemerkten beziehungsweise aktiven Auftrag beenden. | offen |
| T3-07-03 | medium | K8 | DEFEKT | eq-copilot/plugin/src/SourcesModel.cpp:1505; eq-copilot/plugin/src/SourcesModel.cpp:1832; eq-copilot/plugin/src/PluginEditor.cpp:1299 | Zurückgenommene Messevidenz kann ohne neuen gültigen Frame wieder als alte gültige Lautheit erscheinen: der Alterungs-/Disconnect-Pfad überschreibt invalid mit stale. | Quellherleitung: gültiges P2-Lautheitspaar übernehmen, evidence_invalidate akzeptieren, danach tick hinter max(2×Fensterdauer,1000 ms) oder bei Control≠verbunden. Die Rücknahme setzt nur messung=invalid; hatMessZeit, Lautheit::gueltig und Paar bleiben. aktualisiereAbgeleiteteZustaende setzt stale; paintMainFlaeche lässt das Paar wieder zu. R28 und NAK214_V24 in Sonde012SourcesModelTest.cpp:883/:970 betrachten nur die unmittelbare Sicht, keinen nachfolgenden tick. Die Befunde bleiben korrekt stale. | „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand.“ (CLAUDE.md:51) | Die Evidenz-Rücknahme als eigenen verriegelten Zustand bis zu einem neuen akzeptierten Frame halten und Rücknahme → Tick/Disconnect → neuer Frame prüfen. | offen |
| T3-07-04 | high | K8 | DEFEKT | eq-copilot/plugin/src/SourcesModel.cpp:1319; eq-copilot/plugin/src/prozessor/Ipc.cpp:1257; broker/src/coordinator/befehl.rs:549; eq-copilot/plugin/src/prozessor/Ipc.cpp:1073 | Ein zuvor verbundenes, inzwischen aus dem Broker entferntes Mitglied lässt sich im selben Main-Lauf über Remove source nicht mehr entfernen; der negative ACK verschwindet ohne Rückmeldung. | Quellherleitung: bestätigte Quelle mit gültiger runtimeNonce übernehmen, Quelle abmelden/evicten, absoluten Snapshot ohne sie übernehmen. Die persistente Zeile kopiert ihre alte Nonce. entferneSourcesHauptziel wählt deshalb den Wire-Pfad statt der lokalen Entfernung. session_command findet kein Client-Ziel und antwortet unknown_target; v3Antwort löscht lediglich den ausstehenden Befehl. Mitglied und Button bleiben. Der lokale Gegenbeweis für nie verbundene Mitglieder greift nur bei leerer/ungültiger Nonce; NAK-246 D3/D4 behandeln erfolgreiche ACKs. | „Gen führt die Quelle nicht länger als bestätigtes Mitglied; bleibt sie flüchtig entdeckt, ist sie ausdrücklich ungebunden/unclassified statt joined.“ (SONDE-012 L18) | Eine sicher als flüchtig abwesend erkannte Quelle über ihre persistente Mitgliedschaft entfernen und abgelehnte Sources-Aktionen als Ergebnis an das Modell zurückgeben. | offen |
| T3-07-05 | medium | K8 | DEFEKT | eq-copilot/plugin/src/PluginEditor.cpp:858; eq-copilot/plugin/src/PluginEditor.cpp:951; eq-copilot/plugin/src/PluginEditor.cpp:974; eq-copilot/plugin/tests/ShotTestMain.cpp:114 | Ab Quelle 21 fehlen Auswahl und quellenbezogene Handgriffe: das Modell akzeptiert bis zu 64 Mitglieder, die feste Main-Fläche stellt höchstens 20 anklickbare Zeilen her. | Lesende Rechnung aus sourcesZeilen: (430−2×8−48)/18 = 20 ganze Zeilen. Die Schleife bricht danach ab; paint und mouseDown benutzen genau diese Liste. Kein Scroll-/Filter-/Tastaturpfad für Sources vorhanden; der Viewport bei :527 gehört ausschließlich den Legacy-Befundkarten. EqCopShot hat 25 Quellenzustände, aber höchstens 16 Quellen; der 32-Quellen-Test in Sonde012SourcesModelTest.cpp:815 misst Modellrevisionen. | „Gen Fläche 1 zeigt je Quelle den Fehlerzustand ehrlich.“ (docs/plan/plan.json:218, S18–19) | Alle vom Modell angenommenen Quellen mit derselben Zielprüfung erreichbar machen und Auswahl/Entfernen des letzten Eintrags bei 21, 32 und 64 Quellen prüfen. | offen |
| T3-07-06 | medium | K4 | LÜCKE | eq-copilot/plugin/src/prozessor/State.cpp:167; eq-copilot/plugin/src/prozessor/Analyse.cpp:576; eq-copilot/plugin/src/PluginEditor.cpp:287; eq-copilot/plugin/src/PluginProcessor.h:1051 | Beim State-Reload einer bestehenden Gen/Legacy-Instanz bleibt die lokale Messung einschließlich eingefrorener Vergleichslinie und Befundkarten dem vorigen Lauf zugeordnet, ohne dass die Fläche den Identitätswechsel kennzeichnet. | Quellherleitung: auf bestehendem Prozessor messen und Vergleich festhalten; bei offenem Editor anderen gültigen Legacy-State mit anderer Identität laden, ohne anschließendes prepareToPlay oder neue Samples. setStateInformation leert Sources und Vergleichsversuch, fordert aber keinen Reset der AnalyseEngine an; der Editor behält anzeige/vergleichRohDb und liest die neue Rolle/Identität daneben. prepareToPlay würde die Engine über queue.neustartAnfordern zurücksetzen, ist für setStateInformation aber kein interner Gegenpfad. Kein ausgeführter Host-Repro; die fehlende Regel betrifft genau diesen erreichbaren API-Ablauf. | keine Zusage | Eine explizite Reload-Regel für lokale Messung, Befunde und eingefrorenen Vergleich festlegen und alle drei an derselben Projekt-/Messgeneration ausrichten. | offen |

### Skeptikerpass zu den Überlebenden

- **T3-07-01:** Zwei gültige, unterschiedlich gescopte Intent-Einträge genügen. Die Same-Key-Sperre gegen das Überschreiben eines Userwerts greift gerade nicht. Der Test des abstrakten Ordnungshelfers belegt den echten Resolver nicht: C++ und Rust wählen zuerst die Passage. Die Verschmelzungsregel liefert einen konkreten Verhaltensunterschied.
- **T3-07-02:** NAK-179 erklärt die fehlende Hostbrücke. Neu ist die davon unabhängige Klassen-/Flächenkombination: Legacy zeigt die Aktion, Main gibt das Audio frei. Nachträgliche Klassifikation kann den gespeicherten Wunsch aktivieren, während Aus-Knopf und Timer-Rücknahme verschwinden. Die übrigen Audio-Freigaben müssen für diesen Zweig zusätzlich erfüllt sein; deren Sicherungen werden nicht bestritten.
- **T3-07-03:** Das alte Zahlenpaar bleibt im Objekt und wird zunächst durch invalid von der Anzeige ausgeschlossen. Erst der folgende Alters-/Control-Übergang öffnet die Anzeige wieder. Der bestehende NAK-214-Schutz für READY-Befunde bleibt wirksam. Die zusätzliche Prüfung muss Rücknahme und nachfolgenden Tick verbinden.
- **T3-07-04:** Erforderlich ist ein früher lebendes Mitglied mit gültiger Nonce, dessen Broker-Client danach entfernt wird. Ein nie verbundenes State-Mitglied funktioniert als lokaler Gegenfall. D3/D4 decken erfolgreiche Bestätigungen; hier verschwindet die Ablehnung.
- **T3-07-05:** Befundfläche ist die verlorene Auswahl-/Entfernbarkeit akzeptierter Quellen. Kein bestimmtes Layout oder Fenstermaß wird gefordert. Die Rechnung benutzt denselben begrenzten Zeilenvektor wie Paint und Mouse-Hit-Test. 32 Quellen im Modelltest beweisen keine 32 erreichbaren Quellen auf der Fläche.
- **T3-07-06:** Ausdrücklich eine LÜCKE. Ein nachfolgendes prepareToPlay würde den beschriebenen API-Ablauf ändern. Kein konkreter FL-Reload-Ablauf und kein Stateverlust werden behauptet. Die fehlende Regel betrifft die Zuordnung flüchtiger lokaler Messung und Vergleichslinie zum neu geladenen Projekt.

## Population und Zweckzuordnung

### Zählregeln

Die ausdrücklich genannte Codepopulation umfasst **35 Dateien**. Davon wurden **34** zumindest auf Deklarationen und relevante Verwendungen untersucht. Der generierte FlatBuffers-Header wurde nur als vorhanden erfasst; seine neun Quelltabellen wurden im .fbs vollständig inventarisiert. Die Tiefe je Datei steht im Scope-Beweis.

| Population | Umfang | Zähleinheit / Quelle |
|---|---:|---|
| Aktive JSON-Familien | 26 | Root-oneOf, eq-ipc-v3.schema.json; hello mit beiden Unterformen zählt einmal |
| Reservierte JSON-Familiennamen | 5 | reservierte-nachrichten-v1.json; Reservierung ist kein Defekt |
| Benannte JSON-Definitionen | 115 | Alle $defs; vollständiges Register in Anhang A |
| JSON-Objektdeklarationen / Property-Vorkommen | 123 / 567 | 76 direkte Objekte mit 452 Properties plus 47 verschachtelte Objekte mit 115; wiederholte Zweigfelder zählen als Vorkommen |
| FlatBuffers-Tabellen / Felder | 9 / 64 | Alle Tabellen im .fbs, Anhang B |
| Capability-Schlüssel | 10 | capabilities / capabilities_beitrag, gleiche Namen |
| Parametersatz | 120 | 112 Hostwerte + 8 persistente occupied-Werte |
| Öffentliche Sources-Nutzdatenfelder | 81 | Zeile 28 + Versuch 7 + Paar 4 + Befund 29 + Sicht 13 |
| MessSnapshot | 39 | Direkte Datenfelder in AnalyseEngine.h:89; Untertypen getrennt |
| Technische UI-Aktionsarten | 22 | Return/Focusloss für dieselbe Aktion zusammengezählt; Popout-Einstieg und enthaltene Zustandsänderung getrennt |
| Qualifizierte Funktionsdefinitionen in 8 .cpp-Dateien | 187 | Anhang C; inkl. Konstruktoren/Destruktoren/Testhaken, ohne freie Helfer und Inline-Funktionen |
| HoerMarkierung.h-Funktionen | 24 | 7 freie Builder/Filterfunktionen + 17 Methoden inkl. Testhaken |
| EqCopShot-Sources-Szenarien | 25 | faelle.push_back; größte Quellenpopulation dort 16 |

Eine Feldzählung beweist keine Ausführung und keine vollständige Parserkorrektheit. Geprüft wurden Zweck, Besitzer, Publikation und technische Bedienwirkung.

### Alle 26 Nachrichtenfamilien

Schemazeilen beziehen sich auf eq-copilot/schemas/v3/eq-ipc-v3.schema.json. In der Routenspalte liegen SourcesModel/PluginEditor/prozessor unter eq-copilot/plugin/src/, controlclient unter eq-copilot/plugin/core/ipc/ und NakamaTransaktion.h unter eq-copilot/plugin/state/. Bei „Control-Vertrag.cpp“ ist controlclient/Vertrag.cpp gemeint.

| Familie | Schemazeile | Zweck / Richtung | Writer-/Readeranker | Zwecktreue |
|---|---|---|---|---|
| hello | 838 | Control-/Telemetry-Bootstrap; zwei Unterformen | Control-Vertrag.cpp:142; prozessor/Ipc.cpp:460/:520; Broker verbindung.rs:301 | Verbindungsidentität; kein Befundzustand. |
| welcome | 892 | Broker → Control-/Telemetry-Link | broker/src/transport/server_v3/verbindung.rs:369/:420 | Epoch/Link/Challenge werden gebraucht; keine optische Capability. |
| reject | 910 | Broker → Control-Status | verbindung.rs:111; prozessor/Ipc.cpp:484 | Statuspfad vorhanden; kurzlebige Anzeige bekannt NAK-186. |
| heartbeat | 922 | Client → Broker-Liveness/Descriptor | controlclient/Vertrag.cpp; broker/src/coordinator/befehl.rs:673 | Revision, Fähigkeiten, Zähler und Runtime sind unterschiedliche Achsen. |
| heartbeat_ack | 942 | Broker → Client | broker/src/coordinator/befehl.rs:764; controlclient/Nachrichten.cpp | Sequenz/ID-Konflikt; kein Beitritts-ACK. |
| subscribe_session | 956 | Main → Broker → absolute Sicht | prozessor/Ipc.cpp:530/:1041; broker/src/coordinator/subscription.rs:19 | Eigener Sitzungs-Lebenslauf; kein permanenter Mitgliedsdatensatz. |
| session_command | 967 | Main bind/remove → Broker → command_ack | prozessor/Ipc.cpp:1289; broker/src/coordinator/befehl.rs:474 | Zwei geschlossene Kommandos; T3-07-04. |
| session_snapshot | 1070 | Broker → SourcesModel → Editor | broker/src/coordinator/sicht.rs:345; SourcesModel.cpp:621; PluginEditor.cpp:245 | Quellen/Experimente/Paare/Befunde; nicht alle gespeicherten Teile haben heute Pixel. |
| evidence_snapshot | 1119 | Messworker → Broker-Evidenz | prozessor/Analyse.cpp:345; broker/src/coordinator/evidenz.rs:119 | Evidenzobjekt, nicht der lokale Legacy-MessSnapshot. |
| audible_intervention_begin | 1167 | Audio-Ereignis → Worker → Broker | prozessor/Ipc.cpp:74/:118; broker/src/coordinator/befehl.rs:769 | Kennzeichnet beeinflusste Messungen; T3-07-02 betrifft den auslösenden UI-Weg. |
| audible_intervention_end | 1190 | Gegenereignis → Broker | prozessor/Ipc.cpp:74/:118; broker/src/coordinator/befehl.rs:806 | Ende/Tail gehören zum Beginn; keine zweite Uhr für GUI-Animation. |
| evidence_invalidate | 1208 | Broker → SourcesModel | SourcesModel.cpp:1366/:1505 | Rücknahme wird gelesen; T3-07-03 zeigt die spätere Überschreibung. |
| experiment_begin | 2103 | Main-Versuchsmodell → Broker | prozessor/Analyse.cpp:790; broker/src/coordinator/befehl.rs:933 | Passage/Referenz/Ziel; kein aktueller Findings-Knopf. |
| experiment_candidate | 2126 | Main → laufender Versuch | prozessor/Analyse.cpp:934; broker/src/coordinator/befehl.rs:934 | Kandidat/Blindreihenfolge haben eigenen Zweck. |
| experiment_abort | 2146 | Main → terminaler Versuch | prozessor/Analyse.cpp:1023; broker/src/coordinator/befehl.rs:935 | Abbruch wird als Ereignis geführt. |
| experiment_manual_result | 2159 | Userurteil eines Versuchs → Broker | prozessor/Analyse.cpp:965; broker/src/coordinator/befehl.rs:936 | Terminal/Rückmeldung vorhanden; voreilige lokale Fertigmeldung bekannt NAK-169. |
| preview_begin | 2191 | Preview-Lease am Ziel | broker/src/coordinator/befehl.rs:864; NakamaTransaktion.h:161 | Vertrags-/Zielpfad; keine heute sichtbare U21-Aktion daraus ableiten. |
| preview_renew | 2204 | Lease-Verlängerung | broker/src/coordinator/befehl.rs:864 | Erneuerung des flüchtigen Zustands, keine Persistenz. |
| preview_end | 2216 | Lease-Ende | broker/src/coordinator/befehl.rs:864 | Gegenpfad vorhanden; kein Main-UI-Aufrufer in PluginEditor.cpp. |
| command_ack | 2228 | Broker-/Zielergebnis → Aufrufer | prozessor/Ipc.cpp:1063; broker/src/coordinator/befehl.rs:41 | Fünf Ergebniszweige; erfolgreiche Sources-ACKs werden persistiert, negative verschwinden: T3-07-04. |
| state_report | 2347 | Bestätigter DSP-/Identitätszustand → Broker | NakamaTransaktion.h:377; broker/src/coordinator/befehl.rs:1147 (Gegenprobe) | dsp/jcs/hash/Revision/Undo sind gekoppelt; Parser/Hash-Mathematik nicht erneut voll auditiert. |
| intent_update | 1662 | Main-Autorität → Broker-Spiegel | prozessor/Ipc.cpp:548; broker/src/coordinator/intent.rs:317 | Vollbestand/Delta und Bestandrevision nötig; gleiche falsche Auflösung in beiden Sprachen: T3-07-01. |
| assistant_step_update | 1703 | Main-Automat → Broker-Spiegel | prozessor/Ipc.cpp:655; broker/src/coordinator/assistent.rs:83 | Kein zweiter Automat; fehlende UI-Finding-Zuweisung bekannt NAK-208. |
| draft_offer | 2044 | Broker-Proposal → adressierter Empfänger | broker/src/coordinator/subscription.rs:358; Schema:2044 | Vorschlag/Übergabe, kein bestätigter DSP-Commit; heutige U21-Fläche fehlt bewusst. |
| user_verdict | 2061 | Main-Userurteil → persistenter Broker-P0 | prozessor/Ipc.cpp:696; prozessor/Analyse.cpp:1225; broker/src/coordinator/befehl.rs:626 | Urteil ist Userdatum, nicht Messung und nicht ein zweites AssistantStep-Feld. |
| error | 2376 | Protokoll-/Befehlsfehler → Gegenstelle | controlclient/Nachrichten.cpp; Schema:2376 | Typ/Code/Kontext/Rückweg haben Vertragszweck; keine pauschale Garantie, dass jeder Fehler einen UI-Text erreicht. |

Die fünf reservierten Namen sind telemetry_frame, apply_transaction, revert_transaction, experiment_result und reference_match. telemetry_frame hat einen aktiven **binären** P2-Träger; die fehlende JSON-Nutzlast ist keine verwaiste Messung. Für die Preview-Familien wurde der vorhandene Dispatch-/Vertragszweck erfasst, keine vollständig bedienbare heutige Preview-Fläche oder erfolgreiche Fernsteuerung behauptet.

### Capability-Population

Alle zehn Schlüssel: host_context_presence, project_time_samples, sample_accurate_automation, presentation_latency, aux_compare_pre, aux_priority_sidechain, contribution_aux, float64_processing, binary_telemetry, remote_control (Schema:98/:128).

core/ipc/controlclient/Intern.h:393 weist die ersten zwei anhand der Hostbasis aus; die übrigen acht stehen in dieser Basismeldung auf unsupported. SourcesModel.cpp:178 validiert den Block, führt ihn aber nicht als zehn sichtbare Schalter weiter. CapabilityEvidenz ist eine andere Größe: Ohne Messbeweis steht sie ehrlich auf nochNichtGemessen. Daraus wurde kein neues sichtbares Capabilityversprechen abgeleitet.


### Persistente Modelle: vollständige Felder und Gegenpfade

State-Besitzer ist der Prozessor; NakamaState.cpp paart Writer und Reader. Die Felder der Listenobjekte sind unten vollständig benannt. Unbekannte Eigenschaften bleiben im gehaltenen Baum beziehungsweise bei fremdem Major in den Originalbytes erhalten.

| Modell / Ort | Felder | Zweck / Besitzer / Gegenpfad |
|---|---|---|
| Common, NakamaState.h:50, 6 | instanceId, klasse, position, label, pairId, projectBindingId | Bindung/Identität → Statewriter/-reader → Hello/Editor. setzeBindung und Reload sind die Schreiber. |
| MainProjectMitglied, :68, 2 | instanceId, label | Bestätigte Mitgliedschaft → Sources. Runtime-Nonce und Messframes bleiben flüchtig; T3-07-04 entsteht beim Zusammenführen. |
| ManuellePassage, :94, 4 | passageId, label, projektStart, projektEnde | Projektintent; prozessor/State.cpp:365/:403 setzen/entfernen. Fingerprint und Messbeweis gehören dem Store. |
| SourceIntent, :201, 6 | quelleId, passageId, rolle, revision, herkunft, konfidenz | setzeQuellenrolle/entferneQuellenrolle → Persistenz + intent_update → wirksame Rolle; T3-07-01. |
| Schutzangabe, :220, 4 | quelleId, eigenschaft, bandVon, bandBis | Schützen/Aufheben, Reader/Writer, Broker-Abbild; Fachgrenzen statt GUI-Geometrie. |
| IntentBeziehung, :235, 3 | quelleA, quelleB, art | Gerichtete Beziehung/Gleichrangigkeit, Zyklusprüfung, Gegenaktion entfernen. |
| Intent-Bestandrevision, :465, 1 | intentBestandRevision | Auch ein bestätigter leerer Vollbestand muss von „noch nicht angekommen“ unterscheidbar sein. |
| Assistentenzustand, :393, 8 persistente Felder | stepId, schritt, revision, offen, findingId, proposalId, experimentId, ergebnis | Ein Slot; Writer NakamaState.cpp:997, Reader :1880; gesetzt ist zusätzliches Laufzeit-Presence-Bit. NAK-208/PF-03. |
| MainProject, 7 Eigenschaften | confirmed_members_v1, manual_passages_v1, source_intents_v1, intent_protections_v1, intent_relations_v1, intent_revision_v1, assistant_step_v1 | Container für obige Modelle; schema ist Kind-Metadatum. |
| Parameters, 112 Werte | Alle Host-IDs der Tabelle unten; schema und dsp_schema_version als Metadaten | Parametertabelle gemeinsam für Writer NakamaParameter.cpp:563 und Reader :624. |
| Dsp, 5 Eigenschaften | state_revision, occupied_v1, schutz_zonen_v1, undo_ring_v1, undo_cursor | NakamaState.cpp:1059/:2031; bestätigter Zustand getrennt von Preview/Automation. |
| Schutzzone, NakamaParameter.h:132, 4 | id, lowHz, highHz, enabled | Persistenz, DSP-DTO, Schutzprüfung; deaktivieren oder entfernen. |
| UndoEintrag, NakamaState.h:430, 4 | art, slot, revision, zustand | Voller DspSatz vor der Handlung; Undo/Redo bewegen denselben Cursor. |
| Preset-Wurzel, NakamaPreset.cpp:77, 4 | dsp_schema_version, parameters, preset_schema_version, schutz_zonen | 119 der 120 Parameterzellen, ohne eq_enabled; keine Identität oder Projektbindung. |
| State-Erhaltung, NakamaState.h:444 | baum; herkunft, nurLesen, originalBytes, grund, fremdesMajor; hatParameters, hatDsp, layoutV1Migriert | Verlustfreie Erhaltung und Ladeklassifikation. Keine zusätzlichen User- oder Messfelder. |

Alle NakamaState-Anker in dieser Tabelle liegen unter eq-copilot/plugin/state/. Gegenpfade wurden auf Modellzweck gelesen; dies wiederholt keinen vollständigen Migrationstest.

#### Alle 120 Parameter

| Menge | Vollständige IDs / Expansion | Zahl | Deklaration / DSP-Consumer |
|---|---|---:|---|
| v1 global | v1.global. + bypass, input_trim_db, output_trim_db, width, mono_bass_hz | 5 | NakamaParameter.cpp:49; DspProgramm.cpp:162 |
| v1 je Slot i=0..7 | v1.band.i. + enabled, type, freq_hz, q, gain_db, channel_mode, dynamic_enabled, dynamic_range_db, threshold_db, attack_ms, hold_ms, release_ms, sidechain_source | 104 | NakamaParameter.cpp:59; DspProgramm.cpp:181/:229 |
| v2 global | v2.global.eq_enabled, v2.global.mix, v2.global.auto_gain | 3 | NakamaParameter.cpp:79; DspProgramm.cpp:162 |
| v2 belegt, i=0..7 | v2.band.i.occupied | 8 | NakamaParameter.cpp:86; DspProgramm.cpp:181 |
| Gesamt | 112 Hostwerte + 8 Statewerte | 120 | Keine Parameterfamilie ohne DSP-Zuordnung gefunden. |

DspProgramm.cpp liegt unter eq-copilot/plugin/dsp/. Die Expansion über alle acht Slots ist die Population, kein exemplarischer Slot. Der Priority-Sidechain-Fallback hält gemeldeten und wirksamen Wert auseinander; daraus wurde kein verwaister persistenter Parameter konstruiert.

#### Weitere Modelltypen unter state/

- **NakamaParameter.h:87:** Beschreibung = id, typ, min, max, standardZahl, standardBool, werte, standardIndex, wechsel, topologisch, hostParameter, layout (12). Zelle (:113) = b, zahl, enumIndex (3; genau ein Feld je Typ gültig). DspSatz (:151) = werte, zonen (2).
- **NakamaState.h:143:** Belegung = prominenz, tag, veto (3), aus der Rolle abgeleitet. Anspruch (:193) = stufe, spezifisch (2), abstrakte Konfliktrangfolge. Bundle (:291) = klassen (1), zulässige Produktklasse.
- **Schrittvertrag, :337:** schritt, eintritt, traegtEvidenz, useraktion, timeoutMs, rueckkante (6); fünffache Vertragsangabe plus Zustandskennung. Schrittkandidat (:608) = findingId, schritt, erwarteterNutzen, intentRelevanz, konfidenz, reversibilitaet, messkosten, vergleichbar, sicher, bereitsErfolglos (10). Heutige Consumergrenze: PF-03.
- **NakamaTransaktion.h:84:** Tid = hoch, tief (2). Auftrag (:99) = tid, baseRevision, art, satz, band, slot, zielRevision, preset (8). Ergebnis (:112) = ausgang, stufe, revision, hash, memoisiert, slot, grund (7).
- **AutomationOverlay, :150:** gesetzt, werte, epoche, laeuft (4). PreviewOverlay (:161) = aktiv, satz (2). Unterschiedliche flüchtige Schichten mit verschiedenen Rücknahmen, keine zwei persistente Wahrheiten.
- **RegisterEintrag, :279:** belegt, tid, e, hash (4). Klemmung (:370) = id, gemeldet, wirksam (3). DspBericht (:377) = revision, hash, jcs, autoGainDb, klemmungen, verletzteBaender, undoTiefe (7). Commit-Ergebnis/Idempotenz sind von Bedienwunsch und wirksamem Audiozustand getrennt.
- **NakamaKanon.h:28:** Wert = art, b, zahl, text, liste, objektSchluessel, objektWerte (7), zweckgebundener JSON-/Kanonbaum.
- **NakamaLebenslauf.h:142:** zustand, editor, festeKlasse, produkt (4). Klassifikation und Editorleben werden getrennt geführt; heutige Fehlkopplung des Markers: T3-07-02.
- **NakamaKernRiegel.h:** technische Kernkopplung; kein zusätzlicher persistenter Projekt-/Befundzustand. Private Arbeitskopien, Locks und Caches der Kernklassen sind nicht als zusätzliche persistente Felder gezählt.

### SourcesModel: alle 81 öffentlichen Nutzdatenfelder

Alle Deklarationsanker liegen in eq-copilot/plugin/src/SourcesModel.h; der Writer in SourcesModel.cpp. Persistente Mitglieder kommen vom Prozessor, der Editor erhält eine Sichtkopie.

| Modell | Vollständige Feldliste | Zweck / heutiger Consumer |
|---|---|---|
| Zeile, 28, :50 | instanceId, runtimeNonce, pluginKind, mitgliedschaft, control, messung, betrieb, lautheit, namensherkunft, capabilityEvidenz, messpunkt, sichtbarerName, userLabel, hostBusName, hostMixerIndexVorhanden, hostMixerIndex, descriptorVorhanden, hauptziel, p2RejectAktiv, p2RejectGrund, p2RejectZaehler, controlAlterMs, messAlterMs, fensterDauerMs, lufsPaarVorhanden, lufsI, lufsIUnsicherheitLu, findingsOffen | Identität, getrennte Zustandsachsen, Namen, Ziel, Alter/Lautheit/Findingszahl → Main-Sources. T3-07-03/04/05. |
| Versuch, 7, :83 | experimentId, ereignis, offen, hoerurteil, blindreihenfolge, vergleichbarkeit, urteil | Validierter Session-Rückweg; heutiger Editor ist keine vollständige Experimentfläche. |
| Paar, 4, :103 | pairId, klasse, kettenbefund, ausschluss | Validiertes PRE/POST-Ergebnis; kein vollständiger aktueller UI-Consumer. |
| Befund, 29, :132 | findingId, candidateSource, passageId, claimClass, ursachenklasse, targetMetric, bandVon, bandBis, beobachtungWertDb, beobachtungGueltig, zustand, confidenceKlasse, confidenceScore, alternatives, ausschluesse, evidenceIds, maskierungVorhanden, maskierungQuelleA, maskierungQuelleB, maskierungBandVon, maskierungBandBis, maskierungWertDb, maskierungGueltig, maskierungHerabgesetzt, nextTest, intentRevision, likelyCause, smallestTest, listenFor | Broker-Befund einschließlich U21/Maskierung; nicht die lokalen Diagnosekarten. Spätere native Befundfläche geparkt. |
| Sicht, 13, :192 | revision, subscriptionAktiv, diagnose, diagnoseHatHandgriff, mainDarfSchreiben, fuehrendesMain, quellen, experimente, paare, befunde, evidenzRuecknahmen, ruecknahmeGrund, ruecknahmeUmfang | Ein revisionsgebundener Publikationswert; Schreibberechtigung und Rücknahmen bleiben ausdrücklich sichtbar im Modell. |

Private Empfangszeiten, Generation und Publikationsfolge haben einen Leser. **sichtZeit** dagegen (SourcesModel.h:447) hat nur vier Zuweisungen (SourcesModel.cpp:502/:1246/:1549/:1611) und keinen Leser. Ohne nachgewiesenen Verhaltensfehler bleibt dies Rückbaukandidat PF-02, kein kosmetischer Low-Befund.

Bei pre_post und sechs Rangkomponenten validiert der Snapshotleser die Vertragsform und verwirft die lokale Kopie (SourcesModel.cpp:910/:980). Ein vollständiger lokaler Darstellungsverbraucher ist nicht vorhanden; der Absichtskommentar und die geparkte U21-Fläche verhindern, daraus ungefragt eine heutige UI-Pflicht zu machen.

### Messmodell und Optikgrenze

**MessSnapshot, AnalyseEngine.h:89, 39 direkte Felder:** zustand, aktivSekunden, gesamtSekunden, samplerate, lufsGueltig, lufsIntegriert, lufsShortGueltig, lufsShort, truePeakDb, crestDb, crestGueltig, spektralGueltig, centroidMagHz, rolloffHz, lowFrac, flatness, band8Prozent, width, corr, stereoGueltig, ltasZentrenHz, ltasKompositDb, ltasReferenzDb, ltasLiveDb, ltasGueltig, abdeckung, resonanzen, perzentilP10, perzentilP50, perzentilP95, perzentileGueltig, zonenZeit, konvergenzMeanDb, konvergenzMaxDb, konvergenzGueltig, revision, verarbeiteteSamples, schwerVerarbeiteteSamples, nanErsetzt.

Untertypen derselben Datei:

- ResonanzKandidat (:73): freqHz, dbOver, persistenzAnteil, breiteOktaven, klasse (5).
- DrittelOktavAbdeckung (:82): zentrumHz, anteil, klasse (3).
- MessSnapshot::ZonenZeit (:141): gueltig, anteil, ticks (3).
- LautheitsTelemetrie (:173): paar, lufsI, unsicherheitLu, status (4).

Frequenz-/Bandachsen sind Fachwerte. Keine neue Farbe, Pixelachse, Zoom- oder Layoutentscheidung im Messkern gefunden. Glättung/Neigung und Pixelumrechnung geschehen auf Kopien im Editor. Die lokale Diagnose wird aus dem Messsnapshot abgeleitet; sie ist kein zweiter Broker-Befund und führt keine zweite Lautheitsintegration aus. Die Pulsstärke aus truePeakDb (PluginEditor.cpp:773) begrenzt den angeforderten Eingriff; sie behauptet keine neue Messung.

### U21, AssistantStep, Intent und Passagen

- **U21:** ready_to_send, more_data, stale; zwei Gates darfAudition/darfDraft, ausschließlich ready_to_send erlaubt. Die ganze gefundene C++-Aufruferpopulation besteht aus Tests; keine dieser beiden U21-Aktionen im heutigen PluginEditor. Die alten Solo/Puls-Karten sind getrennt zu beurteilen: T3-07-02.
- **AssistantStep:** Acht deklarierte Schritte, sieben in P5: coverage → finding → evidence → listen → proposal → remeasure → verdict. preview ist bewusst ohne P5-Kante und als gespeicherter P5-Schritt abgewiesen (NakamaState.h:307; .cpp:2585). Main besitzt Übergänge/Abbruch/Zurück/Überspringen/Resume; Broker spiegelt.
- **Schrittvertrag:** Angaben sind belegt; timeoutMs wird auf >0 geprüft, aber kein heutiger UI-Timer vollzieht diese Frist. Schrittkandidat/ordneSchritte haben derzeit Testaufrufer. PF-03 führt diese Anschlussfrage, NAK-208 die fehlende findingId-Zuweisung.
- **Intent 37.1:** Fünf Rollen werden auf Belegung/Veto abgebildet, diese Ableitungen nicht parallel persistiert. Der getestete Ordnungshelfer wird vom wirksamen Resolver nicht verwendet; dessen konkreter Widerspruch ist T3-07-01.
- **Passagen:** Projektintent speichert ID/Label/Grenzen; Evidenzmodell trägt zusätzlich Fingerprint, Transportepoche, aktive Quellen und Messpunktklassen. Unterschiedliche Besitzer/Zwecke, kein unnötiges Duplikat.

### Revisionen, Lebenslauf und Message-Thread

1. Main-Timer (PluginEditor.cpp:242) ersetzt seine Sicht bei neuer Revision oder uiDirty; Legacy (:287) übernimmt neue Messrevisionen. Rolle/Label/Readonly, Pipe-Status und Markerphase kommen zusätzlich über Prozessor-Getter. Kein direkter Editorzugriff auf einen Broker-Store gefunden.
2. SourcesModel.cpp:1628 kopiert/sortiert Container schon vor dem Revisionsvergleich. Paint und sourcesZeilen erzeugen lokale Strings/Vektoren. Das sind reale Message-Thread-Kosten, aber ohne Messung keine belegte Budgetüberschreitung.
3. Disconnect (:555) markiert Befunde stale; Reload/Sessionneustart (:456) leert die laufbezogene Sicht. NAK-186 deckt die kurzlebige Transportdiagnose. Die nachfolgende Überschreibung der Evidenz-Messachse ist neu: T3-07-03.
4. Main/Legacy-Wechsel und Reload wurden getrennt gelesen. T3-07-02 betrifft die versteckte Marker-Rücknahme, T3-07-06 die fehlende gemeinsame Generation für lokale Messung/Freeze und neuen State.
5. Editor: 30 Hz; Worker: 50-/250-ms-Deadlines; Aux: 8 Hz. Messfortschritt hängt an Samples, keine Gleichsetzung von Paint-FPS und Datenkadenz.
6. PluginEditor.cpp:233 beendet Markierung und meldet Editorende. Das beweist keine pauschale Popout-Lebensdauersicherheit: MesspunktPanel bleibt der bekannte T3-04-03. Kein neuer eindeutiger zusätzlicher Listener-/Ownership-Fehler als Befund behauptet.


### Alle technischen UI-Handlungen

PluginEditor-Anker liegen unter eq-copilot/plugin/src/; AuxSpike unter eq-copilot/plugin/spike/.

| Nr. | Handlung | Ort | Wirkung, Rücknahme, Bestätigung |
|---|---|---|---|
| 01 | Glättung wählen | PluginEditor.cpp:104 | Lokaler Anzeigeparameter; jederzeit zurückwählbar, keine Messwert-Persistenz. |
| 02 | Ansicht wechseln | PluginEditor.cpp:121 | Lokale Darstellung; erneuter Klick ist Gegenaktion. |
| 03 | Vergleich festhalten | PluginEditor.cpp:132 | Kopiert lokale Messlinie; Gegenaktion 04. Projektwechsel-Regel fehlt: T3-07-06. |
| 04 | Vergleich aus | PluginEditor.cpp:147 | Leert Vergleich; Freigabe folgt vergleichAktiv. |
| 05 | Messung zurücksetzen | PluginEditor.cpp:154 | Resetwunsch an Processor/Worker; kein Zurück versprochen. Unterschied zum State-Reload: T3-07-06. |
| 06 | Legacy-Befunde öffnen | PluginEditor.cpp:162/:345 | Popout aus lokaler Diagnose, schließbar; enthält 15/16. |
| 07 | Messpunkt öffnen | PluginEditor.cpp:165/:625 | Panel bearbeitet Rolle/Label/Paar; übernimmt beim Schließen. Besitzrisiko bereits T3-04-03. |
| 08 | Kennungskonflikt öffnen | PluginEditor.cpp:166/:546 | Echter Konfliktstatus und Handlung 20; keine bloße Dekoration. |
| 09 | Markierung aus | PluginEditor.cpp:173/:817 | Audio-Gegenauftrag; beim Main-Wechsel versteckt: T3-07-02. |
| 10 | Add source / Bind | PluginEditor.cpp:179; prozessor/Ipc.cpp:1200 | Führender Main + Hauptziel; lokaler bestätigter Zustand erst nach erfolgreichem ACK. |
| 11 | Remove source | PluginEditor.cpp:179; prozessor/Ipc.cpp:1245 | Gegenaktion zu 10; abwesende alte Nonce und negativer ACK: T3-07-04. |
| 12 | Reconnect | PluginEditor.cpp:193; prozessor/Ipc.cpp:1120 | Verbindungsversuch, Status danach aus Modell; kein behaupteter synchroner Erfolg. |
| 13 | Quellenlabel übernehmen | PluginEditor.cpp:209/:874; prozessor/Ipc.cpp:1205 | Return oder Fokusverlust; prüft bool-Ergebnis, erneutes Umbenennen ist Gegenaktion. |
| 14 | Hauptziel auswählen | PluginEditor.cpp:974 | Modell-Zielwechsel; andere Zeile als Gegenaktion. Einträge nach 20 unerreichbar: T3-07-05. |
| 15 | Solo am Legacy-Befund | PluginEditor.cpp:438/:754 | Erneuter Klick/Aus/Editor-Ende als Rücknahme; Klassengate und Flächenwechsel: T3-07-02. |
| 16 | Puls am Legacy-Befund | PluginEditor.cpp:438/:754 | Wie 15; Headroom aus Snapshot, Auftrag wird vor dem Audioergebnis als engagiert gemeldet. |
| 17 | Rolle/Messpunkt wählen | PluginEditor.cpp:657; prozessor/State.cpp:287 | Persistenz/Dirty/Reconnect vorhanden; Wechsel zu hub entfernt den heutigen Einstieg zum Gegenwechsel. Produktfrage PF-04. |
| 18 | Lokales Instanzlabel ändern | PluginEditor.cpp:625/:707 | Panel → setzeBindung; erneute Eingabe als Gegenaktion, keine erfundene Hostfarbe. |
| 19 | PRE/POST-Paar-ID ändern | PluginEditor.cpp:625/:707 | Panel → setzeBindung; leeren/ersetzen möglich, Pair-ID nicht aus UI-Reihenfolge geraten. |
| 20 | Neue Sensor-ID bei Konflikt | PluginEditor.cpp:589; prozessor/State.cpp:54 | Explizite Identitätshandlung; kein automatisch erfundener Reconnect-Erfolg. Eigentum des Popouts siehe W-12. |
| 21 | AuxSpike-Bericht schreiben | spike/AuxSpikeEditor.cpp:16 | Aufruf des Diagnosewerkzeugs; im Audit nicht betätigt und keine Datei erzeugt. |
| 22 | AuxSpike-Zähler zurücksetzen | spike/AuxSpikeEditor.cpp:16 | Tatsächlicher Reset der Messhistorie; keine Undo-Zusage. |

Text-, Toggle-, Enabled- und Visible-Wechsel wurden technisch gelesen. Keine zusätzliche zustandsabhängige Tastenmaßänderung als neuer Befund belegt. Die geparkten Fenster-/Designentscheidungen werden nicht reaktiviert.

### EqCopShot und AuxSpike: vorhandene und fehlende Zustände

EqCopShot (tests/ShotTestMain.cpp) baut 25 Sources-Sichten: leere/einzelne/16 Quellen; frische, teilweise, stale, getrennte und unzulässige Messung; ungültige oder fehlende Lautheitspaare; Namenwechsel/Fallback; brokerUnavailable, inkompatibel und storeDegraded; Beitrittsbestätigung, ungebundene und nie gesehene bestätigte Mitglieder; offline/suspended; gültige Lautheit; leere und befüllte Befundzustände; Main-Führung und gesperrtes Label beim nichtführenden Main. Das sind erzeugbare Zustände im Quellcode, **keine in diesem Lauf angesehenen Screenshots**.

Sein Legacy-Zweig speist synthetisches Audio in den Prozessor. Das bleibt im Testprogramm. Kein Fall beweist den Klick auf Quelle 21/32/64, abgelehnten Remove-ACK, Tick nach evidence_invalidate, Rollenwechsel mit vorgemerkter Markierung oder Reload bei offenem Legacy-Editor. Die vorhandenen Fälle widerlegen diese Befunde deshalb nicht.

AuxSpikeEditor zeigt kumulative Mess-/Routingdaten des Hilfsprozessors und bietet Bericht/Reset. Es ist keine Datenquelle für SourcesModel oder persistenten Main-State. Der 8-Hz-Timer holt einen Snapshot; die Signatur steuert Repaint. Kein Bericht wurde erzeugt.

## Widerlegte Verdachte

Bei bekannten Punkten bedeutet „als neuer Befund verworfen“ Dublettenausschluss; es erklärt die Implementierung nicht für korrekt.

### W-01 — Demo-/Fixture-Werte könnten in einem normalen Plugin-Editor eingespeist werden.

Als neuer Befund verworfen: SourcesModel-Fixture bei src/SourcesModel.cpp:1747 und Processor-Testzugriff liegen unter NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3; die Testziel-Definition steht in plugin/CMakeLists.txt:1329. EqCopShot ist ein Testprogramm; sein synthetisches Signal ist kein produktiver Fallback.

### W-02 — Die 120 Parameter seien nur Schemafüllung ohne DSP-Leser.

Widerlegt für die Zweckzuordnung: NakamaParameter.cpp:43 erzeugt die gesamte Population; dsp/DspProgramm.cpp:162 und :229 lesen globale, Slot-, Dynamik- und Occupied-Werte. Priority-sidechain-Fallback erhält den gemeldeten Wert und führt den wirksamen Zustand getrennt; keine unbemerkte Löschung nachgewiesen.

### W-03 — Die Engine enthalte GUI-Achsen, Zoom oder Farbgestaltung.

Die untersuchten Achsen sind Frequenz-/Bandgitter der Messung; AnalyseEngine.h:89 und AnalyseEngine.cpp:1048 publizieren Zahlen, Gültigkeit und Revision. PluginEditor.cpp:1331 transformiert Kopien für die Anzeige. Text-/JSON-Ausgabe des Prozessors ist Diagnose-/Wireformatierung, keine Pixelgeometrie. Keine neue Verletzung der Optikgrenze gefunden.

### W-04 — MORE DATA/STALE könnten über heutige HOLD TO AUDITION- oder SEND DRAFT-Knöpfe handeln.

Widerlegt als erreichbarer U21-UI-Bypass: SourcesModel.h:187/:189 geben ausschließlich ready_to_send frei. Alle gefundenen C++-Aufrufe dieser beiden Gates liegen in Tests (Sonde014BefundTest.cpp:189; Sonde012SourcesModelTest.cpp:977); PluginEditor.cpp hat keine solchen U21-Aktionen. Seine alten Solo/Puls-Karten sind der separate lokale Diagnosepfad, dessen eigener Defekt T3-07-02 ist. Fehlende spätere Oberfläche ist S31b, geparkt.

### W-05 — Das lokale Ende des Experiments werde vor dem Brokererfolg gemeldet.

Als neuer Befund verworfen, nicht sachlich widerlegt: bekannt NAK-169. prozessor/Analyse.cpp:965 und :1023 wurden mit dem Register abgeglichen; in dieser Phase kein zusätzlicher unabhängiger Ablauf belegt.

### W-06 — AssistantStep.findingId sei ein neu entdecktes verwaistes persistentes Feld.

Als neuer Befund verworfen: bekannt NAK-208; Main-Slot und Reader/Writer existieren, die produktive UI-Zuweisung wartet auf S31b. Quellen: NakamaState.h:393, NakamaState.cpp:997, prozessor/Analyse.cpp:1138. Fehlende aktuelle Fläche wird nicht als neue Produktpflicht erfunden.

### W-07 — Incompatible-/Pipe-Status verschwindet kurz nach seiner Anzeige.

Als neuer Befund verworfen: bekannt NAK-186; SourcesModel.cpp:587 und prozessor/Ipc.cpp:484 sind der bereits registrierte Transportstatuspfad. Kein zusätzlicher unabhängiger Registerdelta in diesem Lauf.

### W-08 — Drop-, Grenz- und Analysezähler ohne Gen-Anzeige seien tote oder unehrliche Modelle.

Widerlegt als Anzeigeanforderung: NAK-57, Schlussnachtrag 02.09.2026, dokumentiert ausdrücklich die User-Entscheidung gegen diese Entwicklerdiagnose in der Produktfläche. Die auslesbaren Diagnosefelder sind deshalb nicht allein wegen fehlender Pixel vestigial.

### W-09 — NakamaVertrag::Schema ohne produktiven Komplettaufruf sei ein neuer offener Vertragsdefekt.

Als neuer Befund verworfen: Abdeckungskarte A-2 und bekannt NAK-267 führen genau die generische Schema-/Produktkanten-Härtung. Vertragscode wird hier auf Modellzweck geprüft; keine Sabotage an Schemas oder Runnern als Eingabe konstruiert.

### W-10 — Ohne offenen Editor gingen erfolgreiche Sources-ACKs beim Save verloren.

Widerlegt für den aktuellen Stand: prozessor/Analyse.cpp:391 drainiert im Worker, prozessor/State.cpp:69 drainiert vor der Serialisierung, Ipc.cpp:1538 wendet bestätigte Befehle an. Damit ist NAK-246 D3/D4 nicht mehr nur ein Timerpfad. Negative ACKs bleiben der gesonderte neue T3-07-04.

### W-11 — Main → Legacy lasse persistente Projektmodelle beim Speichern verschwinden.

Als neuer Befund verworfen: bereits T3-02-06 im kumulativen BEFUNDE.md. prozessor/State.cpp:287 und NakamaState-Writer bestätigen die betroffene Lebenslaufkante; hier kein zweites Ticket für denselben Stateverlust.

### W-12 — Der asynchrone Messpunkt-Popout könne den Prozessor nach dessen Ende benutzen.

Als neuer Befund verworfen: bereits T3-04-03. PluginEditor.cpp:688/:707 hält eine Prozessorreferenz im Panel; ein SafePointer auf den Editor heilt dessen Destruktorpfad nicht. JUCE-CallOutBox.cpp:267 wurde zum Besitzweg gelesen. Dieser Punkt wird ausdrücklich nicht als sicher erklärt.

### W-13 — 30 Paint-/Timer-Ticks pro Sekunde bedeuteten 30 neue Messungen.

Widerlegt: PluginEditor.cpp:230/:242 prüft Revision/uiDirty; die Workerpublikation hat 50-/250-ms-Deadlines (prozessor/Analyse.cpp:139). Neue Samples und Analysefortschritt bestimmen die Messrevision. Allokations- und Laufzeitkosten auf dem Message-Thread wurden nicht gemessen.

### W-14 — Eine überholte Sources-Publikation verliere weiterhin zwingend Host-Dirty.

Widerlegt für den untersuchten F01-Pfad: SourcesModel.h:241 trennt übernommen/überholt/reloadAbgewiesen; prozessor/Ipc.cpp:1462 behandelt eine in derselben Generation überholte Änderung anders als den Reload. Das ist der aktuelle NAK-283-Gegenpfad, kein erneuter Befund aus dem Vorgängeraudit.

### W-15 — CapabilityEvidenz zeige ohne Writer erfundene Unterstützung an.

Widerlegt: SourcesModel verwendet den ehrlichen Default nochNichtGemessen; setzeCapabilityEvidenz bei :1729 ist ein separater Evidenzweg. Der zehnteilige Wire-Capability-Block wird validiert, ist aber kein Beweis eines real gemessenen Aux-/Hostpfades. Die beiden Konzepte dürfen nicht zu einem supported-Lämpchen zusammengezogen werden.

## Produktfragen

Vier Entscheidungsvorlagen für die spätere Abarbeitung; keine Fragen an den User und keine Umsetzung in diesem Lauf.

- **PF-01:** Zu T3-07-06: verbindliche Lebensdauer der lokalen Legacy-Messung, ihrer Befunde und des eingefrorenen Vergleichs bei State-Reload festlegen. Empfehlung: gemeinsame Messgeneration; eine bewusste laufübergreifende Referenz bräuchte eine ausdrückliche Kennzeichnung. Keine Produktentscheidung dieses Audits.
- **PF-02:** Welche heute nur transportierten/validierten Felder sollen später tatsächlich bedient werden? SourcesModel.cpp:910 verwirft pre_post nach Prüfung, :980 die Rangkomponenten; Descriptor-Capabilities/Paarmetadaten und Sicht.experimente/paare haben keinen vollständigen heutigen Darstellungsweg. Empfehlung: geplanten Verbraucher benennen oder lokalen DTO-Ballast zurückbauen; geschlossene Wirefelder niemals unversioniert entfernen. sichtZeit (SourcesModel.h:447; vier Zuweisungen, kein Leser) ist ein konkreter lokaler Rückbaukandidat ohne eigenen Funktionsdefekt.
- **PF-03:** Assistentenvertrag versus Laufzeit: Schrittvertrag enthält Eintrittskennung, Evidenzbedarf, Useraktion, Timeout und Rückkante; timeoutMs wird heute nur auf >0 validiert, ordneSchritte/Schrittkandidat haben Testaufrufer, aber keinen aktuellen UI-Workflow. Empfehlung: vor dem späteren Anschluss einen Besitzer für Frist und Priorisierung festlegen oder ungenutzte Laufzeitversprechen aus dem lokalen Modell reduzieren. preview bleibt in P5 bewusst reserviert.
- **PF-04:** Soll die heutige Rollenwahl nach hub eine einmalige technische Initialisierung sein oder einen sichtbaren Rückweg behalten? setzeBindung unterstützt beide Richtungen, PluginEditor.cpp:840 verbirgt den einzigen Rollenwahleinstieg. Empfehlung: bei späterer technischer Integration einen klaren Gegenweg oder eine ausdrückliche Einmaligkeit festlegen; keine neue Designfläche in diesem Lauf. Die bereits jetzt schädliche Markierung beim Wechsel bleibt unabhängig davon T3-07-02.

## Nicht geprüft

- Keine Tests, Builds, Nulltests, Broker-E2E, Kanonläufe, Installation, FL-Interaktion, Audioaufnahme oder Screenshot-Ausführung. EqCopShot wurde ausschließlich gelesen.
- Keine Messung von Message-Thread-Laufzeit, Allokationsmengen, Audioaussetzern oder tatsächlicher Datenkadenz.
- Keine erneute vollständige DSP-/NaN-, Zahlenparser-, Hash-, Migration-, Store- oder Transport-Sicherheitsprüfung. Diese Implementierungen wurden nur soweit für Modellbesitz/Publikationsgrenzen nötig gelesen beziehungsweise nach Verwendungen durchsucht.
- Kein Beweis der realen FL-Aufrufreihenfolge beim Reload. T3-07-06 ist deshalb konditional und als LÜCKE eingeordnet.
- Keine optischen Qualitätsurteile, Farb-/Material-/Layoutvorschläge, Figma-Arbeit oder Reaktivierung geparkter Studien. T3-07-05 betrifft Handlungsreichweite bei zulässiger Quellenzahl.
- Keine neue UI-Pflicht für vorbereitete Assistenten-/Proposalmodelle. Register und bereits vorhandene Auditabschnitte bleiben unberührt.
- Generierter FlatBuffers-Header nicht Zeile für Zeile gelesen; sein Quellschema vollständig als Feldpopulation erfasst. Fremde JUCE-Implementierung nur am relevanten CallOutBox-Lebenslauf.

## Anhang A — vollständiges JSON-Modellregister

Quelle für jede folgende Zeile: eq-copilot/schemas/v3/eq-ipc-v3.schema.json, angegebene Definitionszeile. Alle 115 Definitionen sind enthalten. Felder, Verzweigungen und skalare Domänen sind unterschieden. Der fachliche Besitzer ergibt sich aus der Nachrichten-/Modellzuordnung oben; nicht jedes Wirefeld ist ein UI-Feld.


| Definition | Zeile | Vollständige direkte Felder / Domäne |
|---|---|---|
| hex32 | 40 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| state_hash | 48 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| state_hash_erfolg | 56 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| sid | 64 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| adresse | 71 | logon_sid, project_binding_id, session_epoch, instance_id, runtime_nonce |
| plugin_kind | 88 | Domäne: main, passive_probe, active_probe, legacy |
| capability_wert | 93 | Domäne: supported, unsupported |
| capabilities | 98 | host_context_presence, project_time_samples, sample_accurate_automation, presentation_latency, aux_compare_pre, aux_priority_sidechain, contribution_aux, float64_processing, binary_telemetry, remote_control |
| capabilities_beitrag | 128 | host_context_presence, project_time_samples, sample_accurate_automation, presentation_latency, aux_compare_pre, aux_priority_sidechain, contribution_aux, float64_processing, binary_telemetry, remote_control |
| audio_lage | 158 | samplerate, block_size, channels |
| validity | 169 | project_time, play_state, record_state, cycle_bounds, continuous_time, input_presentation_latency, output_presentation_latency |
| derived_sample_bounds | 196 | start, end, derivation |
| cycle | 208 | active, bounds_valid, start_ppq, end_ppq, derived_sample_bounds |
| transportstempel | 244 | process_context_present, transport_epoch, continuity_segment, sequence, time_basis, project_sample_start, sample_count, sample_rate, playing, recording, continuous_time_samples, cycle, input_presentation_latency, output_presentation_latency, validity |
| zaehler | 373 | frames_dropped, parse_errors, queue_overflows |
| konfidenz | 386 | metrics_version, klasse, timing_alignment, messpunkt_routing, alternativerklaerungen, bootstrap_stabilitaet, verteilung_fenster, samples_nicht_endlich |
| frische | 412 | stale, letzter_kontakt_ms |
| bandwerte | 423 | Verzweigung: bandwerte_fein, bandwerte_grob |
| bandwerte_fein | 432 | Verzweigung: Inline-Objekt (siehe unten), Inline-Objekt (siehe unten), Inline-Objekt (siehe unten) |
| bandwerte_grob | 511 | Verzweigung: Inline-Objekt (siehe unten), Inline-Objekt (siehe unten), Inline-Objekt (siehe unten) |
| probe_descriptor | 590 | Verzweigung: probe_descriptor_insert, probe_descriptor_pre, probe_descriptor_post, probe_descriptor_beitrag |
| probe_descriptor_insert | 601 | adresse, plugin_kind, measurement_position, aussageklasse, betrieb, label, pair_id, host_bus_name, host_mixer_index, capabilities, frische |
| probe_descriptor_pre | 630 | adresse, plugin_kind, measurement_position, aussageklasse, betrieb, label, pair_id, host_bus_name, host_mixer_index, capabilities, frische |
| probe_descriptor_post | 659 | adresse, plugin_kind, measurement_position, aussageklasse, betrieb, label, pair_id, host_bus_name, host_mixer_index, capabilities, frische |
| probe_descriptor_beitrag | 688 | adresse, plugin_kind, measurement_position, aussageklasse, betrieb, label, pair_id, host_bus_name, host_mixer_index, capabilities, frische |
| probe_label | 717 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| host_bus_name | 723 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| host_mixer_index | 731 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| betrieb | 738 | Domäne: active, suspended, offline |
| heartbeat_runtime | 744 | messpunkt, betrieb, label, host_bus_name, host_mixer_index |
| p2_reject | 758 | grund, zaehler |
| session_mitglied | 778 | adresse, plugin_kind, frische, probe_descriptor, p2_reject |
| pair_id | 792 | Skalare Kennung/Zahl/Textregel; kein zusätzliches Objektfeld |
| steuerkopf | 799 | command_id, ziel, base_revision, ttl_ms, schema_major, schema_minor |
| fehlercode | 814 | Domäne: protocol_mismatch, unknown_message, schema_violation, unauthorized, unknown_target, revision_conflict, capability_missing, record_state_unknown, recording_active, lease_expired, rate_limited, internal, abdeckung_zu_gering, schon_terminal, ohne_lautheitsabgleich, ohne_resultatmessung, blindreihenfolge_widerspruch, reihenfolge_nicht_gebunden |
| hello | 838 | Verzweigung: hello_control, hello_telemetry |
| hello_control | 847 | type, connection_kind, protocol, plugin_version, plugin_kind, adresse, host, audio |
| hello_telemetry | 864 | type, connection_kind, protocol, plugin_version, adresse, link_id, challenge |
| host_angabe | 880 | pid, name, version |
| welcome | 892 | type, protocol, broker_version, broker_epoch, link_id, challenge |
| reject | 910 | type, code, reason |
| heartbeat | 922 | type, adresse, sequence, state_revision, capabilities, zaehler, intervention_state_unknown, runtime |
| heartbeat_ack | 942 | type, sequence, duplicate_instance_id |
| subscribe_session | 956 | type, adresse, session_epoch |
| session_command | 967 | Verzweigung: Inline-Objekt (siehe unten), Inline-Objekt (siehe unten) |
| session_experiment | 998 | experiment_id, ereignis, offen, hoerurteil, blindreihenfolge, vergleichbarkeit, urteil |
| session_paar | 1034 | pair_id, klasse, kettenbefund, ausschluss |
| session_snapshot | 1070 | type, session_epoch, broker_epoch, fuehrendes_main, beitritt_bestaetigung_noetig, experimente, paare, store_degraded, findings, mitglieder |
| evidence_snapshot | 1119 | type, evidence_id, adresse, transport, metrics_version, baender, verteilung, abdeckung, konvergenz, beeinflusst, ausschlussgrund, konfidenz, ereignisse, stereo |
| audible_intervention_begin | 1167 | type, intervention_id, adresse, event_sequence, art, experiment_id, project_sample_start |
| audible_intervention_end | 1190 | type, intervention_id, adresse, event_sequence, project_sample_end, tail_samples |
| evidence_invalidate | 1208 | type, grund, umfang |
| invalidate_umfang | 1223 | Verzweigung: invalidate_ids, invalidate_bereich, invalidate_sitzung |
| invalidate_ids | 1233 | art, evidence_ids |
| invalidate_bereich | 1248 | art, sample_start, sample_end |
| invalidate_sitzung | 1260 | art |
| dynamics_ereignis | 1269 | sample_offset, staerke_mad, band_zentrum_hz, dauer_samples, qualitaet_fluss, qualitaet_peak |
| evidence_ereignisse | 1288 | liste, verloren |
| stereo_bandwerte | 1308 | gitter_id, encoding, werte, gueltig_bitmap |
| stereo_bandwerte_normiert | 1332 | gitter_id, encoding, werte, gueltig_bitmap |
| stereo_bandwerte_phase | 1355 | gitter_id, encoding, werte, gueltig_bitmap |
| stereo_evidenz | 1378 | fenster_dauer_ms, freiheitsgrade, mid_db, side_db, seitenanteil_db, korrelation_kurz, korrelation_mittel, kohaerenz, phase_rad, persistenz, zeitperzentile, mono_folddown_db, lr_balance_db |
| fingerprint | 1447 | version, band_energie, chroma, onset |
| alignment_klasse | 1475 | Domäne: feature_aligned, audio_aligned, probable, unclear |
| passage | 1480 | passage_id, projekt_von, projekt_bis, transport_epoch, aktive_quellen, messpunktklassen, abdeckung, label, fingerprint |
| experiment_referenz | 1558 | passage_fingerprint, upstream_fingerprint, aktive_quellen, messpunktklassen, match_gain_db, nicht_endliche_samples, alignment |
| intent_rolle | 1589 | Domäne: fuehrt, traegt, begleitet, geschuetzt, verschmolzen |
| intent_herkunft | 1594 | Domäne: user, template, inferred |
| intent_beziehungsart | 1599 | Domäne: fuehrt_vor, darf_verschmelzen, gleichrangig |
| intent_schutzeigenschaft | 1604 | Domäne: attack, breite, ausklang, band |
| bandintervall | 1609 | von, bis |
| source_intent | 1620 | quelle_id, passage_id, rolle, revision, herkunft, konfidenz |
| intent_schutz | 1638 | quelle_id, eigenschaft, band |
| intent_beziehung | 1650 | quelle_a, quelle_b, art |
| intent_update | 1662 | type, adresse, session_epoch, bestand_revision, vollstaendig, intents, schutzangaben, beziehungen |
| assistant_schritt | 1698 | Domäne: coverage, finding, evidence, listen, proposal, preview, remeasure, verdict |
| assistant_step_update | 1703 | type, adresse, session_epoch, step_id, schritt, revision, offen, finding_id, proposal_id, experiment_id |
| ursachenklasse | 1725 | Domäne: quelle_resonanz, zwei_quellen_konkurrenz, effektkette_pre_post, summe_auf_master, peak_aus_transient, stereo_aus_quelle_oder_kette, daten_reichen_nicht |
| aussageklasse | 1738 | Domäne: zusammenhang, wirkungsbeleg, ursachenbeleg |
| sicherheitsklasse | 1743 | Domäne: hoch, mittel, unklar |
| befund_zustand | 1748 | Domäne: ready_to_send, more_data, stale |
| befund_konfidenz | 1753 | class, score |
| zielmetrik | 1764 | Domäne: band_pegel_db, band_spanne_db, peak_ereignisrate, fluss_ereignisrate, stereo_seitenanteil_db |
| naechster_test | 1775 | Domäne: passage_messen, routing_bestaetigen, pre_post_paar_messen, manueller_versuch, keine_aenderung_empfohlen, mehr_daten_sammeln |
| ausschlussgrund | 1787 | Domäne: coverage_fehlt, alignment_falsch, passage_unvergleichbar, passage_zu_kurz, intent_veto_geschuetzt, intent_veto_verschmolzen, capability_fehlt, evidenz_zurueckgenommen, screening_ueberboten, master_duplikat |
| finding_ausschluss | 1803 | candidate_source, grund |
| maskierung | 1814 | quelle_a, quelle_b, band_von, band_bis, wert_db, gueltig, herabgesetzt |
| rangkomponenten | 1835 | bandpassung, koinzidenz, uplift, intent_relevanz, wiederholbarkeit, routingqualitaet |
| beobachtung | 1850 | wert_db, gueltig |
| session_finding | 1861 | finding_id, claim_class, ursachenklasse, target_metric, candidate_source, pre_post, passage_id, band_hz, beobachtung, rang, confidence, evidence_ids, alternatives, ausschluesse, maskierung, next_test, zustand, intent_revision, likely_cause, smallest_test, listen_for |
| proposal_aktion | 1923 | Domäne: static_eq_cut, dynamic_eq_cut, trim, manual_guidance, no_change, more_data |
| proposal_execution | 1928 | Domäne: manual, previewable |
| proposal_revert | 1933 | Domäne: experiment_abort, manual_only, none_needed |
| proposal_wirkung | 1938 | Domäne: reduce_masking_pressure, reduce_peak_pressure, restore_stereo_balance, none |
| proposal_stopbedingung | 1943 | Domäne: guardrail_abdeckung, guardrail_loudness, guardrail_peak, guardrail_transient, guardrail_breite, guardrail_geschuetzt, keine_wiederholbare_masteraenderung |
| wertebereich | 1956 | von, bis |
| proposal_parameter | 1967 | frequency_hz, q, max_gain_db |
| proposal_grenzen | 1978 | frequency_hz, q, gain_db |
| proposal | 1989 | proposal_id, proposal_schema, target, base_revision, passage_id, finding_id, action, parameters, allowed_bounds, evidence_ids, expected_effect, protected_traits, listen_for, stop_if, execution, confidence, revert, intent_revision, generatorversion |
| draft_offer | 2044 | type, kopf, proposal |
| user_urteil | 2056 | Domäne: angenommen, abgelehnt, spaeter, enthaltung |
| user_verdict | 2061 | type, kopf, user_verdict_id, finding_id, proposal_id, urteil, notiz |
| experiment_ziel | 2082 | band_von, band_bis, geschuetzte_baender, proposal_id |
| experiment_begin | 2103 | type, kopf, experiment_id, execution_mode, reproduzierbarkeit, passage, referenz, ziel |
| experiment_candidate | 2126 | type, kopf, experiment_id, referenz, blindreihenfolge |
| experiment_abort | 2146 | type, kopf, experiment_id, grund |
| experiment_manual_result | 2159 | type, kopf, experiment_id, hoerurteil, blindreihenfolge, notiz, werkzeug |
| preview_begin | 2191 | type, kopf, lease_duration_ms, renew_id |
| preview_renew | 2204 | type, kopf, renew_id |
| preview_end | 2216 | type, kopf, grund |
| command_ack | 2228 | Verzweigung: Inline-Objekt (siehe unten), Inline-Objekt (siehe unten), Inline-Objekt (siehe unten), Inline-Objekt (siehe unten), Inline-Objekt (siehe unten) |
| dsp_klemmung | 2300 | id, gemeldet, wirksam |
| dsp_bericht | 2312 | jcs, auto_gain_db, klemmungen, verletzte_baender |
| state_report | 2347 | type, adresse, dsp_schema_version, state_revision, state_hash, dsp, record_state, undo_tiefe |
| error | 2376 | type, code, meldung, command_id, betroffene_revision, rueckweg |

### Alle 47 verschachtelten Objektdeklarationen

Der JSON-Pointer benennt den genauen Zweig; die Zeile ist der Anker seiner umschließenden Definition. Zusammen 115 zusätzliche Property-Vorkommen. Wiederholte Validity-Felder in Alternativzweigen sind Vertragsrelationen, keine zusätzlichen persistierten Größen.

| JSON-Pointer | Definitionsanker | Properties |
|---|---|---|
| #/$defs/cycle/oneOf/0 | 208 | bounds_valid |
| #/$defs/cycle/oneOf/1 | 208 | bounds_valid, derived_sample_bounds |
| #/$defs/cycle/oneOf/1/properties/derived_sample_bounds | 208 | derivation |
| #/$defs/transportstempel/oneOf/0 | 244 | time_basis, validity |
| #/$defs/transportstempel/oneOf/0/properties/validity | 244 | project_time |
| #/$defs/transportstempel/oneOf/0/oneOf/0 | 244 | validity, cycle |
| #/$defs/transportstempel/oneOf/0/oneOf/0/properties/validity | 244 | cycle_bounds |
| #/$defs/transportstempel/oneOf/0/oneOf/0/oneOf/0 | 244 | validity |
| #/$defs/transportstempel/oneOf/0/oneOf/0/oneOf/0/properties/validity | 244 | continuous_time |
| #/$defs/transportstempel/oneOf/0/oneOf/0/oneOf/1 | 244 | validity |
| #/$defs/transportstempel/oneOf/0/oneOf/0/oneOf/1/properties/validity | 244 | continuous_time |
| #/$defs/transportstempel/oneOf/0/oneOf/1 | 244 | validity |
| #/$defs/transportstempel/oneOf/0/oneOf/1/properties/validity | 244 | cycle_bounds |
| #/$defs/transportstempel/oneOf/0/oneOf/1/oneOf/0 | 244 | validity |
| #/$defs/transportstempel/oneOf/0/oneOf/1/oneOf/0/properties/validity | 244 | continuous_time |
| #/$defs/transportstempel/oneOf/0/oneOf/1/oneOf/1 | 244 | validity |
| #/$defs/transportstempel/oneOf/0/oneOf/1/oneOf/1/properties/validity | 244 | continuous_time |
| #/$defs/transportstempel/oneOf/1 | 244 | time_basis, project_sample_start, validity |
| #/$defs/transportstempel/oneOf/1/properties/validity | 244 | project_time |
| #/$defs/transportstempel/oneOf/1/oneOf/0 | 244 | validity, cycle |
| #/$defs/transportstempel/oneOf/1/oneOf/0/properties/validity | 244 | cycle_bounds |
| #/$defs/transportstempel/oneOf/1/oneOf/0/oneOf/0 | 244 | validity |
| #/$defs/transportstempel/oneOf/1/oneOf/0/oneOf/0/properties/validity | 244 | continuous_time |
| #/$defs/transportstempel/oneOf/1/oneOf/0/oneOf/1 | 244 | validity |
| #/$defs/transportstempel/oneOf/1/oneOf/0/oneOf/1/properties/validity | 244 | continuous_time |
| #/$defs/transportstempel/oneOf/1/oneOf/1 | 244 | validity |
| #/$defs/transportstempel/oneOf/1/oneOf/1/properties/validity | 244 | cycle_bounds |
| #/$defs/transportstempel/oneOf/1/oneOf/1/oneOf/0 | 244 | validity |
| #/$defs/transportstempel/oneOf/1/oneOf/1/oneOf/0/properties/validity | 244 | continuous_time |
| #/$defs/transportstempel/oneOf/1/oneOf/1/oneOf/1 | 244 | validity |
| #/$defs/transportstempel/oneOf/1/oneOf/1/oneOf/1/properties/validity | 244 | continuous_time |
| #/$defs/bandwerte_fein/oneOf/0 | 432 | gitter_id, encoding, werte, gueltig_bitmap, saturated |
| #/$defs/bandwerte_fein/oneOf/1 | 432 | gitter_id, encoding, werte, gueltig_bitmap, saturated |
| #/$defs/bandwerte_fein/oneOf/2 | 432 | gitter_id, encoding, werte, gueltig_bitmap, saturated |
| #/$defs/bandwerte_grob/oneOf/0 | 511 | gitter_id, encoding, werte, gueltig_bitmap, saturated |
| #/$defs/bandwerte_grob/oneOf/1 | 511 | gitter_id, encoding, werte, gueltig_bitmap, saturated |
| #/$defs/bandwerte_grob/oneOf/2 | 511 | gitter_id, encoding, werte, gueltig_bitmap, saturated |
| #/$defs/session_command/oneOf/0 | 967 | type, command, command_id, ziel, session_epoch |
| #/$defs/session_command/oneOf/1 | 967 | type, command, command_id, ziel, session_epoch |
| #/$defs/evidence_snapshot/properties/verteilung | 1119 | p10, p50, p95 |
| #/$defs/stereo_evidenz/properties/zeitperzentile | 1378 | p10, p50, p95 |
| #/$defs/command_ack/oneOf/0 | 2228 | type, command_id, ergebnis, state_revision, state_hash, code |
| #/$defs/command_ack/oneOf/1 | 2228 | type, command_id, ergebnis, state_revision, state_hash, code |
| #/$defs/command_ack/oneOf/2 | 2228 | type, command_id, ergebnis, state_revision, state_hash, code |
| #/$defs/command_ack/oneOf/3 | 2228 | type, command_id, ergebnis, state_revision, state_hash, code |
| #/$defs/command_ack/oneOf/4 | 2228 | type, command_id, ergebnis, state_revision, state_hash, code |
| #/$defs/state_report/properties/record_state | 2347 | valid, recording |


## Anhang B — alle binären Modelle und Felder

Quelle: eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs. Neun Tabellen, 64 Felder; die Zahl hinter jedem Namen ist seine Deklarationszeile. Gemeinsame Writer-/Readergrenze: Vertrag/NakamaTelemetrie und P2-Empfang → Broker beziehungsweise Sources-Messsicht. Zahlen/Gültigkeit/Transport bleiben getrennt von Paint. Keine vollständige Decoder- oder Codegeneratorprüfung in dieser Phase.

| Tabelle | Anker | Alle Felder (Zeile) |
|---|---|---|
| Adresse | 121 | logon_sid (122), project_binding_id (123), session_epoch (124), instance_id (125), runtime_nonce (126) |
| AbgeleiteteGrenzen | 133 | start (134), ende (135), herleitung (136) |
| Schleife | 142 | active (143), bounds_valid (144), start_ppq (145), end_ppq (146), abgeleitete_grenzen (147) |
| Transportstempel | 157 | transport_epoch (158), continuity_segment (159), sequence (160), zeitbasis (161), project_sample_start (162), sample_count (163), sample_rate (164), playing (165), recording (166), continuous_time_samples (167), schleife (168), input_presentation_latency (169), output_presentation_latency (170), gueltigkeit (171), process_context_present (181) |
| Bandwerte | 199 | gitter (200), encoding (201), werte_i16 (202), werte_f32 (203), gueltig_bitmap (204), saturated (205) |
| Headroomverteilung | 224 | p10_db (225), p50_db (226), p95_db (227), fenster (228) |
| Frame | 239 | transport (240), baender (241), metrics_version (242), aktivitaet (243), lufs_s (244), peak_db (245), crest_db (246), psr_db (247), breite (248), korrelation (249), band_stereo (253), lufs_i (257), lufs_i_unsicherheit_lu (258), lufs_i_status (261), integration_samples (275), lufs_m (281), true_peak_db (289), true_peak_passage_db (296), plr_db (303), lra_lu (309), crest_kurz_db (316), headroom (318), band_dynamic_gain_db (338) |
| QuellenEintrag | 342 | quelle (343), frame (344) |
| FeatureBatch | 361 | eintraege (362) |

Die sieben Transport-Validity-Bits und die zwei Bandgitter gehören zum Messvertrag; ein fehlender Wert wird nicht durch eine Demo-Messung ersetzt. band_dynamic_gain_db ist als reales Framefeld mit erfasst; alte Reservierungsprosa wurde nicht als Befundfläche verwendet.


## Anhang C — vollständiger Funktionsindex der Modell-/Editor-.cpp-Grenzen

Gezählt sind qualifizierte Definitionen, keine gleichnamigen Aufrufe. Die zwei anfänglich vom Suchmuster erfassten Aufrufe von SourcesModel::Uhr::now wurden aus der Zählung entfernt. Insgesamt 187 in acht Dateien. Dieser klar abgegrenzte Index zählt weder freie Helfer noch alle Funktionen der Parser-/DSP-Unterbäume mit.

### eq-copilot/plugin/src/PluginEditor.cpp — 24

- EqCopilotEditor::EqCopilotEditor — Zeile 87
- EqCopilotEditor::~EqCopilotEditor — Zeile 233
- EqCopilotEditor::timerCallback — Zeile 242
- EqCopilotEditor::zeigeHinweise — Zeile 496
- EqCopilotEditor::zeigeKonflikt — Zeile 538
- EqCopilotEditor::comboBoxChanged — Zeile 601
- EqCopilotEditor::zeigeMesspunkt — Zeile 614
- EqCopilotEditor::markierungIstAktiv — Zeile 748
- EqCopilotEditor::schalteMarkierung — Zeile 754
- EqCopilotEditor::markierungBeenden — Zeile 817
- EqCopilotEditor::istMainFlaeche — Zeile 828
- EqCopilotEditor::wechsleFlaecheWennNoetig — Zeile 833
- EqCopilotEditor::uebernehmeSourcesLabel — Zeile 874
- EqCopilotEditor::aktualisiereSourcesSteuerung — Zeile 902
- EqCopilotEditor::sourcesSpalte — Zeile 946
- EqCopilotEditor::sourcesZeilen — Zeile 951
- EqCopilotEditor::mouseMove — Zeile 969
- EqCopilotEditor::mouseDown — Zeile 974
- EqCopilotEditor::graphFlaeche — Zeile 990
- EqCopilotEditor::resized — Zeile 996
- EqCopilotEditor::anzeigeKurve — Zeile 1047
- EqCopilotEditor::zeichneKurve — Zeile 1068
- EqCopilotEditor::paintMainFlaeche — Zeile 1101
- EqCopilotEditor::paint — Zeile 1331

### eq-copilot/plugin/src/SourcesModel.cpp — 28

- SourcesModel::setzePersistenteMitglieder — Zeile 349
- SourcesModel::publikationenUeberholt — Zeile 438
- SourcesModel::publikationenNachReloadAbgewiesen — Zeile 444
- SourcesModel::persistenteMitgliederKopie — Zeile 450
- SourcesModel::projektReload — Zeile 456
- SourcesModel::sitzungszustandLeeren — Zeile 515
- SourcesModel::beginneSubscription — Zeile 539
- SourcesModel::controlEnde — Zeile 555
- SourcesModel::setzeControlTransport — Zeile 587
- SourcesModel::uebernehmeSessionSnapshot — Zeile 621
- SourcesModel::uebernehmeEvidenzruecknahme — Zeile 1366
- SourcesModel::uebernehmeP2 — Zeile 1526
- SourcesModel::tick — Zeile 1608
- SourcesModel::sicht — Zeile 1628
- SourcesModel::waehleHauptziel — Zeile 1670
- SourcesModel::aktuellesHauptziel — Zeile 1680
- SourcesModel::istAktuellesHauptziel — Zeile 1686
- SourcesModel::zaehleOffeneFindings — Zeile 1693
- SourcesModel::setzeAlleBefundeStale — Zeile 1714
- SourcesModel::setzeCapabilityEvidenz — Zeile 1729
- SourcesModel::setzeDiagnoseFuerSichtbeweis — Zeile 1739
- SourcesModel::setzeFixtureFuerTest — Zeile 1748
- SourcesModel::stelleZielSicher — Zeile 1772
- SourcesModel::aktualisiereName — Zeile 1779
- SourcesModel::aktualisiereAbgeleiteteZustaende — Zeile 1798
- SourcesModel::messStaleFristMs — Zeile 1844
- SourcesModel::platzhalter — Zeile 1855
- SourcesModel::sortierSchluessel — Zeile 1860

### eq-copilot/plugin/src/AnalyseEngine.cpp — 19

- AnalyseEngine::WelchStufe::init — Zeile 65
- AnalyseEngine::WelchStufe::schiebe — Zeile 102
- AnalyseEngine::AnalyseEngine — Zeile 163
- AnalyseEngine::vorbereiten — Zeile 169
- AnalyseEngine::zuruecksetzen — Zeile 277
- AnalyseEngine::verarbeite — Zeile 338
- AnalyseEngine::segmentInBaender — Zeile 440
- AnalyseEngine::zonenTick — Zeile 517
- AnalyseEngine::finalisiereLtas — Zeile 628
- AnalyseEngine::finalisiereLoudness — Zeile 692
- AnalyseEngine::lautheitFuerTelemetrie — Zeile 710
- AnalyseEngine::fuelleBasis — Zeile 741
- AnalyseEngine::auswertenLeicht — Zeile 787
- AnalyseEngine::berechnePerzentile — Zeile 801
- AnalyseEngine::finalisiereSkalar — Zeile 838
- AnalyseEngine::berechneAbdeckung — Zeile 929
- AnalyseEngine::findeResonanzen — Zeile 965
- AnalyseEngine::auswerten — Zeile 1048
- AnalyseEngine::snapshot — Zeile 1126

### eq-copilot/plugin/src/prozessor/Analyse.cpp — 38

- EqCopilotProcessor::workerLauf — Zeile 133
- EqCopilotProcessor::setzeAssistentSchritt — Zeile 426
- EqCopilotProcessor::assistentSchrittKopie — Zeile 476
- EqCopilotProcessor::bindePassagenfenster — Zeile 492
- EqCopilotProcessor::bindePassagenfensterMitEpoche — Zeile 513
- EqCopilotProcessor::passagenfensterWunschFuerTest — Zeile 548
- EqCopilotProcessor::passagenfensterFuehrt — Zeile 557
- EqCopilotProcessor::versuchAufgenommeneBloecke — Zeile 571
- EqCopilotProcessor::vergleichszustandLeeren — Zeile 576
- EqCopilotProcessor::loesePassagenfenster — Zeile 626
- EqCopilotProcessor::engineabzugLesen — Zeile 645
- EqCopilotProcessor::passagenfensterInEngine — Zeile 662
- EqCopilotProcessor::versuchReferenzJson — Zeile 675
- EqCopilotProcessor::versuchKopfJson — Zeile 750
- EqCopilotProcessor::sendeVersuchP0 — Zeile 777
- EqCopilotProcessor::beginneVersuch — Zeile 790
- EqCopilotProcessor::erfasseKandidat — Zeile 934
- EqCopilotProcessor::urteileVersuch — Zeile 965
- EqCopilotProcessor::brichVersuchAb — Zeile 1023
- EqCopilotProcessor::laufenderVersuch — Zeile 1048
- EqCopilotProcessor::versuchLautheitAbgeglichen — Zeile 1054
- EqCopilotProcessor::versuchMatchGainDb — Zeile 1077
- EqCopilotProcessor::versuchLautheitAbgeglichenLebendFuerTest — Zeile 1089
- EqCopilotProcessor::vergleichspegelZaehlerstand — Zeile 1094
- EqCopilotProcessor::versuchNichtEndlicheSamples — Zeile 1112
- EqCopilotProcessor::assistentAenderungMelden — Zeile 1117
- EqCopilotProcessor::assistentStarten — Zeile 1138
- EqCopilotProcessor::assistentWeiter — Zeile 1154
- EqCopilotProcessor::assistentZurueck — Zeile 1173
- EqCopilotProcessor::assistentUeberspringen — Zeile 1187
- EqCopilotProcessor::assistentAbbrechen — Zeile 1201
- EqCopilotProcessor::assistentAntwort — Zeile 1225
- EqCopilotProcessor::assistentAusState — Zeile 1279
- EqCopilotProcessor::assistentFortsetzen — Zeile 1285
- EqCopilotProcessor::assistentVersuchStarten — Zeile 1291
- EqCopilotProcessor::snapshotObjektBauen — Zeile 1321
- EqCopilotProcessor::diagnoseAntwort — Zeile 1344
- EqCopilotProcessor::schreibeSnapshotDatei — Zeile 1395

### eq-copilot/plugin/src/prozessor/Ipc.cpp — 35

- EqCopilotProcessor::interventionsWireJson — Zeile 74
- EqCopilotProcessor::interventionenSenden — Zeile 118
- EqCopilotProcessor::v3Hello — Zeile 460
- EqCopilotProcessor::v3Status — Zeile 484
- EqCopilotProcessor::v3TelemetryHello — Zeile 520
- EqCopilotProcessor::v3SubscribeJson — Zeile 530
- EqCopilotProcessor::v3IntentUpdateJson — Zeile 548
- EqCopilotProcessor::v3AssistantStepJson — Zeile 655
- EqCopilotProcessor::v3UserVerdictJson — Zeile 696
- EqCopilotProcessor::urteilMitFrischemKopf — Zeile 779
- EqCopilotProcessor::sendeIntentVollbestand — Zeile 809
- EqCopilotProcessor::sendeIntentFortschreibung — Zeile 827
- EqCopilotProcessor::v3ControlLink — Zeile 851
- EqCopilotProcessor::v3Antwort — Zeile 1063
- EqCopilotProcessor::v3Frame — Zeile 1103
- EqCopilotProcessor::sourcesTick — Zeile 1111
- EqCopilotProcessor::reconnectSources — Zeile 1120
- EqCopilotProcessor::statsSnapshot — Zeile 1125
- EqCopilotProcessor::messKompakt — Zeile 1150
- EqCopilotProcessor::bindeSourcesHauptziel — Zeile 1200
- EqCopilotProcessor::benenneSourcesHauptziel — Zeile 1205
- EqCopilotProcessor::entferneSourcesHauptziel — Zeile 1245
- EqCopilotProcessor::sendeSourcesCommand — Zeile 1289
- EqCopilotProcessor::bestaetigteSourcesCommandsAbholen — Zeile 1402
- EqCopilotProcessor::wendeSourcesCommandAnUnterBindung — Zeile 1414
- EqCopilotProcessor::naechsteSourcesFolgeUnterBindung — Zeile 1446
- EqCopilotProcessor::werteSourcesPublikationAus — Zeile 1462
- EqCopilotProcessor::meldeSourcesMitgliederNachBefehl — Zeile 1492
- EqCopilotProcessor::wendeBestaetigteSourcesCommandsAn — Zeile 1538
- EqCopilotProcessor::ausstehenderSourcesCommandFuerTest — Zeile 1592
- EqCopilotProcessor::bestaetigteSourcesCommandsFuerTest — Zeile 1599
- EqCopilotProcessor::sourcesDrainRiegelGehaltenFuerTest — Zeile 1605
- EqCopilotProcessor::sourcesMitgliederFolgeFuerTest — Zeile 1614
- EqCopilotProcessor::setzeSourcesMitgliederFolgeFuerTest — Zeile 1622
- EqCopilotProcessor::merkeSourcesCommandFuerTest — Zeile 1633

### eq-copilot/plugin/src/prozessor/State.cpp — 33

- EqCopilotProcessor::neueSensorId — Zeile 54
- EqCopilotProcessor::getStateInformation — Zeile 69
- EqCopilotProcessor::sourcesListenLeerenUndGenerationErhoehen — Zeile 115
- EqCopilotProcessor::setStateInformation — Zeile 140
- EqCopilotProcessor::holeSensorId — Zeile 225
- EqCopilotProcessor::holeRolle — Zeile 226
- EqCopilotProcessor::holeLabel — Zeile 227
- EqCopilotProcessor::holePaarId — Zeile 228
- EqCopilotProcessor::stateNurLesen — Zeile 230
- EqCopilotProcessor::holeStateHerkunft — Zeile 236
- EqCopilotProcessor::holeStateGrund — Zeile 242
- EqCopilotProcessor::holeStateFremdesMajor — Zeile 248
- EqCopilotProcessor::holeZustandKopie — Zeile 254
- EqCopilotProcessor::spiegleKlassifikation — Zeile 262
- EqCopilotProcessor::holeKlassifikation — Zeile 275
- EqCopilotProcessor::darfBrokerStarten — Zeile 281
- EqCopilotProcessor::setzeBindung — Zeile 287
- EqCopilotProcessor::merkeManuellePassage — Zeile 365
- EqCopilotProcessor::vergissManuellePassage — Zeile 403
- EqCopilotProcessor::manuellePassagen — Zeile 445
- EqCopilotProcessor::setzeQuellenrolle — Zeile 480
- EqCopilotProcessor::entferneQuellenrolle — Zeile 509
- EqCopilotProcessor::schuetzeQuelle — Zeile 535
- EqCopilotProcessor::hebeQuellenschutzAuf — Zeile 563
- EqCopilotProcessor::setzeQuellenbeziehung — Zeile 591
- EqCopilotProcessor::speichereQuellenGleichrangigkeit — Zeile 620
- EqCopilotProcessor::entferneQuellenbeziehung — Zeile 627
- EqCopilotProcessor::sourceIntents — Zeile 653
- EqCopilotProcessor::intentSchutzangaben — Zeile 659
- EqCopilotProcessor::intentBeziehungen — Zeile 665
- EqCopilotProcessor::intentBestandRevision — Zeile 671
- EqCopilotProcessor::wirkendeQuellenrolle — Zeile 677
- EqCopilotProcessor::entmaskierungErlaubt — Zeile 689

### eq-copilot/plugin/src/prozessor/Hostbruecke.cpp — 5

- EqCopilotProcessor::prepareToPlay — Zeile 34
- EqCopilotProcessor::isBusesLayoutSupported — Zeile 149
- EqCopilotProcessor::meldeHostDirty — Zeile 160
- EqCopilotProcessor::setzeEditorOffen — Zeile 170
- EqCopilotProcessor::createEditor — Zeile 182

### eq-copilot/plugin/spike/AuxSpikeEditor.cpp — 5

- AuxSpikeEditor::AuxSpikeEditor — Zeile 16
- AuxSpikeEditor::resized — Zeile 42
- AuxSpikeEditor::signatur — Zeile 50
- AuxSpikeEditor::timerCallback — Zeile 60
- AuxSpikeEditor::paint — Zeile 70

### eq-copilot/plugin/src/HoerMarkierung.h — 24

Freie Funktionen (7): rbjAlphaBw:62, biquadStabil:68, rbjPeaking:76, rbjBandpass0dB:94, rbjHochpass:110, rbjTiefpass:125, baueMarkierungsAuftrag:166.

Methoden (17): vorbereiten:297, reicheEin:345, uebersprungenePublikationen:372, setzeLeserHakenFuerTest:383, setzePublisherHakenFuerTest:388, gehaltenerPufferFuerTest:396, leserstandFuerTest:407, reicheAus:411, verarbeite:431, setzeSamplerate:665, nichtEndlicheWetSamples:690, hoerbar:695, phase:696, zielGesetzt:697, tdf2Lauf:728, resetZustaende:753, hartAus:760.


## Scope-Beweis

### Integrität und Schreibgrenze

Abschlusskontrolle am 16.09.2026: HEAD unverändert db58ee7a23b16e8b486f688e08b3764523469ca5, Branch master. Der originale 51333-Byte-Präfix von BEFUNDE.md ist bytegleich (SHA-256 unten); git diff meldet dort ausschließlich 16 hinzugefügte und 0 entfernte Zeilen. Die beiden Befundtabellen sind zeichengetreu gleich. Sechs fortlaufende IDs, 15 Skeptiker-Einträge und vier Produktfragen wurden am geschriebenen Text nachgezählt. git diff --check meldet keine Fehler. Der Status enthält ausschließlich die zwei autorisierten eigenen Auditpfade zusätzlich zu den vier unveränderten fremden Untracked-Pfaden.

Ausgangs-HEAD: db58ee7a23b16e8b486f688e08b3764523469ca5. Ausgangsinhalt BEFUNDE.md: 51333 Bytes, SHA-256 F35996CF7A01F0C7C2BD2A74297756CD59946B674A7DA92B0037AB8A63374091. Nur der neue Phase-07-Abschnitt darf hinter diesem Präfix liegen.

Schon zu Beginn fremd/untracked und unberührt: .claude/settings.local.json, briefing-hub/, docs/audits/2026-09-15-tiefenaudit/roh/phase-07-auftrag.txt, nimbalyst-local/. Keine Bereinigung, kein Staging, Commit oder Push.

### Gelesene Primärpfade — 34 von 35 vorhandenen Dateien

Die folgende Liste enthält die vollständige vorgegebene Code-Dateipopulation. „Gelesen“ heißt modell-/fragestellungsbezogene Quelllektüre, nicht erfolgreicher Test. Zeilenzahlen sind physische Zeilen am gebundenen HEAD. Größere Ausgaben wurden teilweise gekürzt; deshalb wird für Parser-/Hilfsimplementierungen ausdrücklich keine lückenlose Zeilenlektüre behauptet. Alle entscheidenden neuen Befundpfade wurden in kleineren Bereichen nachgelesen.

| Pfad | Dateizeilen | Lesetiefe |
|---|---|---|
| eq-copilot/plugin/src/PluginEditor.h | 183 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |
| eq-copilot/plugin/src/PluginEditor.cpp | 1665 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |
| eq-copilot/plugin/src/SourcesModel.h | 458 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |
| eq-copilot/plugin/src/SourcesModel.cpp | 1947 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |
| eq-copilot/plugin/src/HoerMarkierung.h | 816 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |
| eq-copilot/plugin/src/AnalyseEngine.cpp | 1132 | Messmodell, Reset und Publikation; keine erneute Vollprüfung der DSP-Mathematik |
| eq-copilot/plugin/src/prozessor/State.cpp | 697 | Editor-Publikation, Identität, ACK, Reload und relevante APIs; nicht jeder Hilfszweig |
| eq-copilot/plugin/src/prozessor/Ipc.cpp | 1663 | Editor-Publikation, Identität, ACK, Reload und relevante APIs; nicht jeder Hilfszweig |
| eq-copilot/plugin/src/prozessor/Intern.h | 49 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/src/prozessor/Hostbruecke.cpp | 187 | Editor-Publikation, Identität, ACK, Reload und relevante APIs; nicht jeder Hilfszweig |
| eq-copilot/plugin/src/prozessor/Analyse.cpp | 1438 | Editor-Publikation, Identität, ACK, Reload und relevante APIs; nicht jeder Hilfszweig |
| eq-copilot/plugin/vertrag/NakamaEvidenz.cpp | 606 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/vertrag/NakamaVertrag.h | 149 | Öffentliche Vertrags-/Nutzdatenmodelle und Verwendungsgrenzen |
| eq-copilot/plugin/vertrag/NakamaVertrag.cpp | 1190 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/vertrag/NakamaUtf8.h | 81 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/vertrag/NakamaTelemetrie.h | 142 | Öffentliche Vertrags-/Nutzdatenmodelle und Verwendungsgrenzen |
| eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp | 761 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/vertrag/NakamaEvidenz.h | 112 | Öffentliche Vertrags-/Nutzdatenmodelle und Verwendungsgrenzen |
| eq-copilot/plugin/state/NakamaLebenslauf.h | 148 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaLebenslauf.cpp | 119 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaKernRiegel.h | 122 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/state/NakamaKanon.h | 88 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaKanon.cpp | 428 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/state/NakamaPreset.cpp | 269 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/state/NakamaParameter.h | 233 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaParameter.cpp | 701 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/state/NakamaState.cpp | 2911 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaPreset.h | 74 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaState.h | 704 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaTransaktion.h | 393 | Modelldeklarationen bzw. relevante Writer-/Reader-/Übergangsfunktionen |
| eq-copilot/plugin/state/NakamaTransaktion.cpp | 820 | Deklarations-/Funktions- und Verwendungsindex; ausgewählte Gegenpfade, keine Vollprüfung |
| eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h | 1547 | Nur enumeriert; Quell-.fbs stattdessen gelesen |
| eq-copilot/plugin/spike/AuxSpikeEditor.h | 36 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |
| eq-copilot/plugin/spike/AuxSpikeEditor.cpp | 179 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |
| eq-copilot/plugin/tests/ShotTestMain.cpp | 318 | Editor-/Modellfläche gelesen und Aufrufe verfolgt; Befundstellen gezielt nachgelesen |

### Ergänzend gelesene Pfade — 49 Dateien

Zusammen mit den 34 gelesenen Primärdateien sind **83 eindeutige, hier namentlich belegte Lese-/Suchpfade** erfasst. Breite rg-Trefferlisten sind keine zusätzliche Behauptung vollständig gelesener Dateien.

**Kontext und Zusagen (16 Dateien):** CLAUDE.md und Root-AGENTS vollständig, KONZEPT §3/§4.3, Abdeckungskarte A-1 bis A-7 und Lebenslauf-Befunde B-1 bis B-8, relevante Matrix-/Gate-/Registerstellen; Blueprint und Entwurf nur als Zweckreferenz. Skills wurden gelesen, der engere Userauftrag hat Vorrang vor Build-/Fix-Empfehlungen. Keine dieser Dateien wurde als Prosa-Befundfläche benutzt.

- .agents/skills/nakama-plugin-quality/SKILL.md
- .agents/skills/sondenplan-audit/SKILL.md
- AGENTS.md
- CLAUDE.md
- design/docs/funktions-und-bedien-blueprint.md
- docs/FL-Nakama-Sonden-Design-Entwurf.md
- docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md
- docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md
- docs/beweise/SONDE-012.md
- docs/beweise/SONDE-014.md
- docs/gesundheit/KONZEPT.md
- docs/gesundheit/abdeckungskarte.md
- docs/gesundheit/lebenslauf-audit.md
- docs/offene-punkte.md
- docs/plan/plan.json
- docs/plugin-wissen.md

**Weitere Quellen und Gegenproben (33 Dateien):** Zusatzheader und DSP-Verbraucher feldbezogen; Broker/Control-Vertrag und Testquellen an relevanten Deklarations-, Dispatch-, Writer-/Reader- oder Teststellen; keine vollständige Prüfung dieser Unterbäume. Die .schema.json- und .fbs-Feldinventuren umfassen jeweils alle Deklarationen.

- broker/src/coordinator/assistent.rs
- broker/src/coordinator/befehl.rs
- broker/src/coordinator/evidenz.rs
- broker/src/coordinator/hypothese_verdrahtung.rs
- broker/src/coordinator/intent.rs
- broker/src/coordinator/liveness.rs
- broker/src/coordinator/mod.rs
- broker/src/coordinator/schema.rs
- broker/src/coordinator/sicht.rs
- broker/src/coordinator/subscription.rs
- broker/src/coordinator/zustand.rs
- broker/src/transport/server_v3/verbindung.rs
- eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/windows/juce_CallOutBox.cpp
- eq-copilot/plugin/CMakeLists.txt
- eq-copilot/plugin/core/ipc/controlclient/Intern.h
- eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp
- eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp
- eq-copilot/plugin/dsp/DspProgramm.cpp
- eq-copilot/plugin/spike/AuxSpikeProcessor.cpp
- eq-copilot/plugin/src/AnalyseEngine.h
- eq-copilot/plugin/src/Diagnose.cpp
- eq-copilot/plugin/src/PluginProcessor.cpp
- eq-copilot/plugin/src/PluginProcessor.h
- eq-copilot/plugin/tests/MarkierungTestMain.cpp
- eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp
- eq-copilot/plugin/tests/Sonde014AssistentTest.cpp
- eq-copilot/plugin/tests/Sonde014BefundTest.cpp
- eq-copilot/plugin/tests/Sonde014IntentTest.cpp
- eq-copilot/schemas/v3/README.md
- eq-copilot/schemas/v3/eq-ipc-v3.schema.json
- eq-copilot/schemas/v3/flatbuffers/README.md
- eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs
- eq-copilot/schemas/v3/reservierte-nachrichten-v1.json

### Nicht gelesene und fehlende Pfade

- Vollständig ungelesener Implementierungspfad innerhalb der 35er-Population: eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h (1547 Zeilen). Nur Existenz/Zeilenzahl bestimmt; fachliche Quelltabellen vollständig inventarisiert.
- Außerhalb der verfolgten Nahtstellen nicht gelesen: übrige Broker-Transport-/Store-/Analyseimplementierungen, übrige DSP-/RT-Kernimplementierungen, gesamter Fixturekorpus, alle übrigen Tests, Installations-/Build-/Beweisrunner und geparkte Design-/Studienbäume. Native Buildartefakte, Startlog und Shot-PNGs wurden nicht als aktuelle Laufzeitbelege gelesen.
- Bei indexierten state-/vertrag-Implementierungen sind Parser-/Mathematik-Hilfszweige nicht vollinhaltlich geprüft. Die Tabellen oben benennen genau diese geringere Tiefe; kein „komplett geprüft“ aus bloßem Dateifund.
- **Fehlende vorgegebene Pfade: keine.** HoerMarkierung ist header-only; EqCopShot wird durch tests/ShotTestMain.cpp repräsentiert. Falsch angenommene Suchpfade wie broker/src/schema.rs beziehungsweise flache server_v3.rs wurden durch rg --files auf coordinator/schema.rs und transport/server_v3/ aufgelöst; Windows-Wildcard-Suchfehler wurden mit -g/Verzeichnissuche korrigiert. Das sind keine Produktbefunde.
- Der Python-Launcher war für eine lesende Hilfsabfrage nicht startbar; die Inventuren wurden mit PowerShell/.NET gelesen. Keine Python-Datei angelegt, kein dadurch fehlender Befundbeweis. Git meldete fehlenden Zugriff auf eine globale Ignore-Datei außerhalb des Repos; HEAD, Repository-Status und eigener Diff blieben lesbar.

**Scopeabschluss:** 83 benannte Lese-/Suchpfade, davon 34 primäre Codedateien; 1 primäre generierte Datei nur enumeriert; 26 aktive Familien, 115 JSON-Definitionen mit 567 Property-Vorkommen, 64 binäre Felder, 120 Parameter, 10 Capability-Schlüssel und 22 UI-Handlungsarten inventarisiert. Keine Laufzeitverifikation behauptet. Befundbilanz: 5 DEFEKTE, 1 LÜCKE, 0 HÄRTUNGEN; 0 critical, 3 high, 3 medium, 0 low; 15 widerlegte/als neu verworfene Verdachte, 4 Produktfragen.
