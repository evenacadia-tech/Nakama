# G4 — Quellencheck der Codex-Befunde C2 bis C11

Repo `C:\Users\phili\Projekte\Nakama`, HEAD `9f3000881df632f4969d78551fb0a516b61288cb`,
Worktree sauber. Strikt lesend geprüft; keine Repo-Datei berührt.

Bereits vom Dirigenten geprüft und hier **nicht** erneut aufgerollt: C1 (= D-01),
C8 (StereoGolden `frames=512`), C10 (M-84-Sweep fehlt).

Referenztexte, wörtlich nachgeschlagen:

- **Exit-Gate P4** (`docs/FL-Nakama-Sonden-Design-Entwurf.md:4287-4289`), drei Sätze:
  1. „Referenzkorpus, Loop-/Seek-/PDC-Goldens und adversariale Vergleichsfixtures bestehen."
  2. „Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder PRE/POST-Aussage."
  3. „Hörmarker- und Experimentende schließen alle Taintintervalle; der reguläre Pfad
     nullt danach wieder."
- **§49.2 harte Systemgates**: Gate 5 `:3559`, **Gate 6 `:3560`** („Ein nicht vergleichbares
  Experiment erhält ein starkes Siegerurteil."), **Gate 7 `:3561`** („Eine Standard-Insertprobe
  wird als exakter Mastersummenbeitrag bezeichnet.").

Register `docs/offene-punkte.md` (Stand `f1566f5`, 100 offen · 5 geparkt · 73 geschlossen):
kein Treffer für `kettenbefund`, `aktiv_s`, `AktivzeitKlafft`, `p4-korpus`, `pruefe_p4`,
`Distraktor`, `Blockquarantaene`, `M-53`, `M-62`, `M-63`, `M-79`, `M-83`, `M-85`.
**Alle sieben Befunde sind neu.**

---

## C2 · Reconnect mit aktivem Hörmarker setzt kein lokales Unknown

**(a) BESTÄTIGT — und in der Same-Broker-Variante schärfer als vom Prüfer beschrieben.**

Entscheidende Quellzeilen:

- `eq-copilot/plugin/src/PluginProcessor.cpp:1199` —
  `if (interventionsRing.fuellstand() == 0 && ! markierung.hoerbar())`: läuft ein Marker,
  wird der ganze Block übersprungen; ein `else`-Zweig, der lokal Unknown setzte, existiert nicht.
- `eq-copilot/plugin/src/PluginProcessor.cpp:1146-1147` — `v3Status()` speist
  `interventionStateUnknown` **ausschließlich** aus
  `interventionsRingUeberlauf || interventionsRing.ueberlaufGesehen()`; ein laufender Marker
  geht dort nicht ein.
- `eq-copilot/plugin/core/ipc/ControlClient.cpp:408-410` — `heartbeatAlsJson` lässt das Feld
  bei `false` **weg** (C1). Auf dem Draht ist „neutral" damit von „schweigt" nicht
  unterscheidbar.
- `broker/src/coordinator/befehl.rs:573` —
  `Some(false) if self.link_ohne_ereignissequenz(link_id) => { let _ = self.resync_bestaetigen(link_id, 0); }`.
  `v3_nachricht_lesen_beliebig_mit_minor` liefert für ein **fehlendes** Feld `None`, nicht
  `Some(false)` → in dieser Konstellation greift `_ => {}`. Der Resync bleibt also aus, **aber**
  auch kein Unknown entsteht.

**Was der neue Broker nach dem Hello über den laufenden Marker weiß: nichts.**

- `ControlHello` (`PluginProcessor.cpp:1090-1112`) trägt Adresse, Version, Samplerate,
  Blockgröße, Kanäle, Host-PID — kein Markerfeld.
- Der v3-Telemetrieframe kennt kein `hoermarkierung`; das Feld existiert nur im **v2**-Heartbeat
  (`PluginProcessor.cpp:1357` in `messKompakt()`, „Kompakter Messstand für den v2-Heartbeat").
- Der v2-Weg schließt die Lücke **nicht**: `broker/src/server.rs:766` bildet
  `interventions_link_id = "v2:<len>:<sensor_id>:<nonce>"`; `session_des_links` findet diesen
  Schlüssel nie in `stand.links` → `hoermarkierung_v2` landet über
  `unwrap_or_else(SessionKey::unbekannt)` (`intervention.rs:249`) nur auf dem **Platzhalter** —
  und `neutral_resync` löscht den Platzhalter ausdrücklich mit
  (`intervention.rs:395`, `Self::taint_mut(&mut stand, &SessionKey::unbekannt()).unknown = false`).
- Der Taintzustand ist **nicht persistiert**: `taint` kommt nur in `intervention.rs`,
  `invalidierung.rs`, `link.rs`, `liveness.rs`, `zustand.rs`, `experiment_verdrahtung.rs` vor —
  in keinem Store-/Persistenzpfad. Ein Brokerneustart startet also mit leerem `Stand`.

**Wird das laufende Begin erneut gesendet? Nein.** `interventionenSenden`
(`PluginProcessor.cpp:1045-1088`) leert nur den Ring; für einen bereits entnommenen Begin gibt es
keinen Replay-Pfad. Der Ring wurde beim Begin geleert, deshalb ist `fuellstand() == 0` — genau die
Bedingung, die zusammen mit `hoerbar()` in die Lücke führt.

**Was beim späteren `end` ohne bekanntes Begin passiert (M-61):** Es setzt Unknown — aber erst
dann. Zwei Wege, beide in `intervention.rs`:
- `sequenz_annehmen` (`:65-73`): der neue Link führt `letzte_event_sequence = None`, nimmt also
  die erste Nummer vorbehaltlos an; hat der erste Heartbeat aber `resync_bestaetigen(link, 0)`
  ausgelöst, steht die Basis auf 0, und die vom Plugin unverändert weitergezählte Sequenz
  (z. B. 7) ist eine Lücke → `taint.unknown = true`.
- `intervention_end_mit_beginn` (`:213-219`): `passt == false`, weil `taint.interventionen` das
  Begin nicht kennt → `taint.unknown = true`, Rückgabe `(false, None)`.

**Ungeschützter Zeitraum:** vom Wiederaufbau des Control-Links bis zum Eintreffen des
`audible_intervention_end` desselben Markers — also die gesamte Restdauer des Markers plus
Zustellzeit. In diesem Fenster ist `taint` der Sitzung leer, `interventionssicht().starke_evidenz_erlaubt`
(`intervention.rs:550`) ist `true`, und `evidence_dispatch_locked` (`:577`) nimmt jeden
Evidenzsnapshot an — während der Marker das Audio hörbar färbt.

**Schärfer als der Prüfertext:** Bleibt derselbe Broker am Leben und fällt nur der Link,
setzt `control_ende` (`broker/src/coordinator/link.rs:452-454`) korrekt
`taint.unknown = true` und entfernt die aktive Intervention. Der **erste Heartbeat des neuen
Links** löscht das aber wieder: `link_ohne_ereignissequenz` ist für den frischen Link `true`,
und sobald ein `intervention_state_unknown: false` explizit auf der Leitung stünde, führte
`befehl.rs:573` → `resync_bestaetigen` → `neutral_resync` (`intervention.rs:386-389`,
`interventionen.clear(); tail_samples_offen = 0; unknown = false`) die Sitzung zurück auf
sauber — mit laufendem Marker. Heute rettet nur der C1-Bug (das Feld reist gar nicht), dass das
in dieser Variante nicht passiert; **wird C1 wie vorgeschlagen „sende explizit false" gefixt,
ohne C2 mitzufixen, wird die Lücke größer statt kleiner.**

**(b) Erreichbar im normalen Betrieb.** Aufruferkette:
`ControlClient` Verbindungsrückruf → `PluginProcessor.cpp:109`
`[this] (bool verbunden) { v3ControlLink (verbunden); }` → `:1172`. Auslöser sind alltäglich:
Brokerneustart durch den Supervisor, Brokerabsturz, Pipe-Abriss, Heartbeat-Timeout
(`broker/src/transport/server_v3/mod.rs:2107` `"Timeout" => coordinator.control_ende(&link)`).
Der Marker läuft dabei weiter — `v3ControlLink(false)` (`:1174-1179`) ruft nur
`sourcesModel.controlEnde()` und `telemetryV3.reconnect()`, die Markierung bleibt unberührt.
Keine Fehlerinjektion nötig.

**(c) Gebrochene Sätze, wörtlich:**
- Exit-Gate Satz 3 (`:4289`): „Hörmarker- und Experimentende schließen alle Taintintervalle;
  der reguläre Pfad nullt danach wieder." — Hier nullt der Pfad **vor** dem Ende.
- M-62 (`docs/beweise/SONDE-013.md:562`): „Die Taint-Logik ist **sitzungsweit** … Bei unbekanntem
  Routing ist der Umfang die ganze Sitzung." — Nach dem Reconnect ist der Umfang leer.
- M-63 (`:563`): „Der Riegel liegt **vor** dem Evidence-Commit … Gesperrt heißt: keine starke
  Aussage, nicht ‚leiser Wert'." — Der Riegel greift im Fenster gar nicht.
- §34.2 in Codekommentarform (`PluginProcessor.cpp:1140-1142`): „Ein verlorenes Begin oder End
  darf niemals eine scheinbar saubere Baseline erzeugen."
- CLAUDE.md-Invariante „Grundgesetz: nichts Ungefragtes" mittelbar: gefärbtes Audio geht als
  saubere Evidenz durch.

**(d) Register: neu.** Kein Treffer zu Reconnect/Marker/Taint in `docs/offene-punkte.md`.

**(e) Fixgröße: mittel.**
- `eq-copilot/plugin/src/PluginProcessor.cpp` — `else`-Zweig an `:1199`, der bei nicht
  erklärbarer Neutralität ein eigenes lokales Sticky setzt (neues Atomic, in `v3Status()` mit
  `||` verodert): ~15 Zeilen inkl. Kommentar.
- Gegenpfad (löschen↔setzen): dasselbe Sticky fällt nur im Resync-Zweig `:1201-1209`.
- Zusammen mit C1 (`ControlClient.cpp:408-410`, explizites `false`) und der Brokerregel
  `Some(false)` (`befehl.rs:573`) ergibt sich ein konsistenter Handschlag.
- **Messender Test: neu.** `eq-copilot/plugin/tests/Sonde013PassageStateTest.cpp` hat mit
  `c1Sequenzhandschlag` bereits den Rahmen, prüft aber ausdrücklich das Gegenteil:
  `:1174` `pruefe (! p->markierungHoerbar(), "C1: der Marker ist vor dem Resync still")`.
  Neuer Fall: Marker läuft → `v3LinkFuerTest(true)` → gebautes Heartbeat-JSON trägt
  `intervention_state_unknown:true`. Brokerhälfte: `broker/tests/sonde013_verdrahtung.rs`,
  neuer Fall „erster Heartbeat mit laufendem Marker resyncht nicht".

---

## C3 · Quarantänebruch geht im Evidenzstempel verloren

**(a) BESTÄTIGT.** Die Kette, Zeile für Zeile:

1. `eq-copilot/plugin/core/StampedAudioQueue.h:700-718` — `Blockquarantaene::schiebe`: gilt
   `schliesstAn(gehalten, neu) == false`, wird der gehaltene Block **verworfen**
   (`hoch(verworfen); hoch(brueche);`), es gibt **keine** `Freigabe`, und der erste Block nach
   dem Bruch wird gehalten (`gehalten = neu; belegt = true;`).
2. `eq-copilot/plugin/sonde/SondeProcessor.cpp:336-345` — der Worker sieht den geänderten
   Bruchzähler und setzt **die FeatureEngine vollständig zurück**:
   `analyseEngine.zuruecksetzen(); merkmale.zuruecksetzen(); producerStandLeeren();`
   (`merkmale` ist laut `SondeProcessor.h:265` die `nakama::analyse::FeatureEngine`).
3. `eq-copilot/plugin/core/analysis/FeatureEngine.h:1120-1123` — `zuruecksetzen()` setzt
   `habeVorigen = false; transportEpoche = 0; segmentInEpoche = 0; sequenz = 0;`.
4. `FeatureEngine.h:1158` — beim nächsten Block ist deshalb
   `grund = habeVorigen ? grenzeZwischen(...) : Grenzgrund::keine` → **`keine`**, also kein
   `grenzeZiehen` und kein Zähler-Inkrement (`:2102-2112`,
   `lokaleLuecke → ++segmentInEpoche`, sonst `++transportEpoche; segmentInEpoche = 0`).
5. `FeatureEngine.h:3629-3630` — der Stempel trägt genau diese zwei Felder:
   `t.transport_epoch = transportEpoche; t.continuity_segment = segmentInEpoche;`.
6. `broker/src/coordinator/evidenz.rs:294-295` — der Broker liest sie aus
   `/transport/transport_epoch` bzw. `/transport/continuity_segment`.
7. `broker/src/coordinator/invalidierung_verdrahtung.rs:433-439` —
   `invalidierung_aus_transportbruch` vergleicht **nur** die zwei jüngsten Historieneinträge:
   `if neu.transport_epoch != alt.transport_epoch { Epochwechsel } else if neu.continuity_segment != alt.continuity_segment { Sequenzluecke } else { None }`.
   Stand vor dem Bruch bei 0/0 → nach dem Reset wieder 0/0 → **`None`, keine Invalidierung**.

Zwei Folgen, beide real:
- **Stand war 0/0** (der Regelfall einer frischen Sitzung): der Bruch ist auf dem Draht
  unsichtbar; PRE/POST-Evidenz überbrückt eine unbekannte Zeitgrenze.
- **Stand war > 0** (eine engineinterne Grenze lag davor): der Broker sieht 1 → 0, also
  *Ungleichheit*, und meldet `Epochwechsel` — auch wenn der Auslöser ein **Drop** war, der nach
  M-53 zwingend `sequenzluecke` tragen muss. Der Grund auf dem Draht ist dann falsch.

**Gegenbeweis aus dem Repo selbst:** derselbe Wormerpfad in Gen macht es richtig.
`eq-copilot/plugin/src/PluginProcessor.cpp:904-912` setzt beim Quarantänebruch **nur**
`engine.zuruecksetzen()` und lässt `merkmale` stehen, mit dem Kommentar „FeatureEngine erkennt
die Grenze am naechsten freigegebenen Deskriptor selbst." Das stimmt: `vorigerBlock` der Engine
lebt unabhängig von `gehalten` der Quarantäne, `grenzeZwischen` erkennt den Sprung, und
`grenzeZiehen` leert **alle** Fenster *und* zieht Epoche/Segment hoch. Der Sondenpfad erreicht
mit `merkmale.zuruecksetzen()` dieselbe Fensterleerung, wirft dabei aber den Beweis weg.

**Misst ein Test die Kopplung? Nein.**
- `eq-copilot/plugin/tests/QueueStressTestMain.cpp:355-359` misst die Quarantäne allein
  (Flag `kFlagLueckeDavor`, `b->segment == 1`) — auf Ebene des Deskriptors, ohne Engine.
- `eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:631-664` (M-75/M-77) misst die Engine allein:
  `Speiser` speist die Blöcke **direkt**, `s.strom += 65536` erzeugt eine Stromlücke, die Engine
  zieht die Segmentgrenze korrekt. Der Reset aus dem Worker kommt in diesem Test nicht vor.
- Kein Test fährt `SondeProcessor` mit echtem Queue-Drop und liest danach `transport_epoch`
  bzw. `continuity_segment` auf der Leitung.

**(b) Erreichbar im normalen Betrieb.** Auslöser: Queue-Überlauf unter CPU-Last
(`kFlagLueckeDavor`, `StampedAudioQueue.h:729`), Seek bei laufendem oder gestopptem Transport
(Hostzeitprüfung `:750-765`), Beweislagewechsel (`bruchMaske`, `:738-741`), Kanal-/Tap-Wechsel.
Aufruferkette: Audiothread → `StampedAudioQueue` → Analyseworker
`SondeProcessor.cpp:334-345` → `merkmale.nimmBlock` → `evidenzSnapshotSenden` (`:408`) →
P1 → `broker/src/coordinator/evidenz.rs` → `invalidierung_aus_transportbruch`.

**(c) Gebrochene Sätze:**
- M-53 (`docs/beweise/SONDE-013.md:548`): „Der Bruch beendet die alte Zeitreihe. Kein Fenster
  wird über die Grenze interpoliert; ein Drop erzeugt Zähler, sichtbare Lücke und eine neue
  Kontinuitätsgrenze. **Der Grund auf dem Draht ist je Auslöser eindeutig** … ein **Drop** trägt
  `sequenzluecke`." — Weder sichtbare Lücke noch eindeutiger Grund.
- Exit-Gate Satz 2 (`:4288`): „Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder
  PRE/POST-Aussage."
- CLAUDE.md, tragende Invariante „Zeit ist aktive Musikzeit. Zonen-Ticks sind deterministisch
  und durch Goldens beweisbar."

**(d) Register: neu.**

**(e) Fixgröße: klein (Code), mittel (Beweis).**
- `eq-copilot/plugin/sonde/SondeProcessor.cpp:336-345`: `merkmale.zuruecksetzen()` entfernen und
  dem Gen-Muster folgen — die Engine zieht die Grenze selbst und stempelt sie. Alternativ eine
  ausdrückliche `merkmale.grenzeMelden(Grenzgrund)`-Öffnung. ~5-10 Zeilen plus Kommentar.
- **Messender Test: neu**, zweiseitig. Plugin-Hälfte: Drop im echten `SondeProcessor`, dann
  `continuity_segment` im gesendeten Snapshot > 0 (Anschluss an `Sonde013EventWireTest.cpp`,
  das `f.transport.transport_epoch/continuity_segment` bereits liest, `:484-485`).
  Broker-Hälfte: `broker/tests/sonde013_verdrahtung.rs:1049-1050` fährt die Zuordnung
  `("transport_epoch","epochwechsel")` / `("continuity_segment","sequenzluecke")` bereits —
  ergänzen um „zwei aufeinanderfolgende Snapshots mit 0/0 nach echtem Drop invalidieren".

---

## C4 · Ausgeschlossene PRE/POST-Kette meldet `stationaer` statt `nicht_beurteilbar`

**(a) BESTÄTIGT, wörtlich.**

- `broker/src/coordinator/prepost.rs:920-931` — `fn ausschluss(...)` setzt fest
  `klasse: Alignmentklasse::Unclear`, `ergebnis: None`, `restlag: None` — **und**
  `kettenbefund: Kettenbefund::Stationaer` (`:926`).
- Die ehrliche Variante existiert und ist im Code selbst als Gegensatz benannt:
  `prepost.rs:131-137` — „⚠️ Das ist ausdrücklich NICHT `Stationaer`. Eine Kette, über die
  niemand etwas sagen kann, darf keinen festen Übertragungsgang tragen — ‚nie beurteilt' und
  ‚stationär' sind zwei verschiedene Aussagen, und nur die zweite erlaubt eine EQ-Behauptung."
- **Alle zehn** Ausschlussgründe laufen durch `ausschluss()`: `prepost.rs:947, 950, 953, 956,
  959, 964, 970, 975, 1126 (Paarkonflikt), 1131 (HaelfteFehlt)`.
- Serialisiert wird bedingungslos: `broker/src/coordinator/sicht.rs:461-475` schreibt
  `kettenbefund` in **jedes** `session_paar`-Objekt, `ausschluss` dagegen nur `if let Some(a)`.
- Schema: `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1033` führt `kettenbefund` als
  **Pflichtfeld**, `:1039` trägt genau den Kommentar „`nicht_beurteilbar` ist ausdruecklich
  NICHT `stationaer`".
- Der Leser nimmt es an: `eq-copilot/plugin/src/SourcesModel.cpp:588, 596, 846` →
  `pp.kettenbefund` (`SourcesModel.h:102`). Gen bekommt also für jedes hart ausgeschlossene
  Paar die Zeichenkette `"stationaer"` in die Hand.

**Der Test prüft das Feld nicht:** `broker/tests/sonde013_prepost.rs:582-614`
`hard_exclusions_each_name_their_reason` prüft `klasse`, `ausschluss`, `ergebnis.is_none()`,
`restlag.is_none()` — `u.kettenbefund` kommt in der Funktion nicht vor.

**(b) Erreichbar im normalen Betrieb, sogar häufig.** Jeder der sieben gefahrenen Fälle ist
Alltag: eine Hälfte getrennt, stale, sammelt noch, andere Samplerate, keine Projektzeit, leeres
Fenster, Sprung im Fenster. Aufruferkette:
`paare_bei_bedarf_bilden` (`prepost_verdrahtung.rs:64`) → `bilde_paare` → `beurteile_paar`
(`prepost.rs:944ff`) → `stand.paarurteile` → `sicht.rs::session_paar_json` → `session_snapshot`
→ `SourcesModel::uebernehmeSessionSnapshot`.

**(c) Gebrochene Sätze:**
- M-18 (`docs/beweise/SONDE-013.md:493`): „Danach darf Nakama keinen festen
  Übertragungsfrequenzgang behaupten … das Ergebnis lautet dann ‚wahrscheinliche
  PRE/POST-Wirkung' statt einer kausalen Behauptung."
- M-22 (`:497`): „Harte Ausschlüsse führen zu `unclear` mit benanntem Grund, **nicht zu einer
  schwachen Zahl**". `"stationaer"` ist genau die schwache Aussage, die hier verboten ist.
- CLAUDE.md: „Keine toten UI-Elemente. Jedes sichtbare Element führt einen Handgriff aus oder
  meldet **ehrlich** einen Zustand."

**(d) Register: neu.**

**(e) Fixgröße: klein.**
- `broker/src/coordinator/prepost.rs:926` — eine Zeile: `Kettenbefund::NichtBeurteilbar`.
- Kein Schema-, Serialisierungs- oder Leserwechsel nötig (Wert ist bereits im Enum, im Schema
  `:1045` und im `match` von `sicht.rs:472`).
- **Messender Test: bestehend erweitern.** `broker/tests/sonde013_prepost.rs:598-603` um
  `assert_eq!(u.kettenbefund, Kettenbefund::NichtBeurteilbar, "{name} (Seite {seite})")`;
  dazu `bilde_paare`-Hälfte für `Paarkonflikt`/`HaelfteFehlt` (`prepost.rs:1126,1131`).
  ~6 Zeilen Test.

---

## C5 · Die berechnete Aktivzeit wird von der Projektspanne überschrieben

**(a) BESTÄTIGT.**

- `broker/src/coordinator/prepost_verdrahtung.rs:254-256` — die **echte** Rechnung:
  `if stand.sample_rate > 0.0 { aktiv_s += stand.sample_count as f64 / stand.sample_rate; }`,
  summiert über die nicht zurückgenommenen Historieneinträge.
- `:274-277` — die Überschreibung:
  ```
  aktiv_s: match (von, bis, letzter.sample_rate) {
      (Some(a), Some(b), r) if b > a && r > 0.0 => (b - a) as f64 / r,
      _ => aktiv_s,
  },
  ```
  Trifft der erste Zweig, ist `aktiv_s` die Min/Max-**Projektspanne** und nicht mehr die
  gemessene Zeit. Der Kommentar `:268-273` sagt das auch offen — er begründet es mit dem
  Suchraum für `schaetze_restlag` (Befund R30), ohne zu bemerken, dass dieselbe Zahl in M-23
  eine **andere** Frage beantworten muss.
- Der Fallback ist unerreichbar für die Herabstufung: `von`/`bis` speisen zugleich
  `projekt_fenster` (`:278-283`). Ist `(von, bis)` `None` oder `b <= a`, ist `projekt_fenster`
  `None` → harter Ausschluss `KeineProjektzeit` (`prepost.rs:958-960`). Ist
  `letzter.sample_rate <= 0`, greift `SamplerateVerschieden` (`prepost.rs:955`, `!(pre.sample_rate > 0.0)`).
  **Jeder Pfad, der die Herabstufungsstufe erreicht, trägt die Projektspanne.**
- Die Prüfung selbst: `prepost.rs:1033-1036` —
  `let (klein, gross) = (pre.aktiv_s.min(post.aktiv_s), pre.aktiv_s.max(post.aktiv_s));`
  `if gross > 0.0 && (gross - klein) > GATE_AKTIVZEIT_DIFFERENZ * gross { AktivzeitKlafft }`.
  Decken beide Hälften denselben ersten und letzten Projektzeitpunkt ab — genau der Fall bei
  Smart Disable oder einseitiger Stille mitten in der Passage —, sind beide Spannen gleich und
  der Riegel fällt nie.

**Misst der Unit-Test die Verdrahtung? Nein.** Alle Fundstellen setzen `aktiv_s` direkt am
`Paarhaelfte`-Literal: `broker/tests/sonde013_prepost.rs:71` (Helfer `haelfte`), `:307`, `:636`,
`:649`, `:863-864`. `haelfte_aus_historie` (`prepost_verdrahtung.rs:186`) ist **privat** und hat
außer `paare_bei_bedarf_bilden` keinen Aufrufer; kein Integrationstest fährt zwei Hälften mit
unterschiedlicher Snapshotdichte über denselben Projektbereich.

**(b) Erreichbar im normalen Betrieb.** Der Kommentar zu M-23 nennt die Auslöser selbst:
FL Smart Disable auf einem Bus, Stille auf einem Messpunkt, Rückstau-Kadenzreduktion des
Evidenzsenders (`SondeProcessor.cpp:472-478` verdoppelt das Evidenzintervall bei P1-Rückstau —
eine Hälfte liefert dann messbar weniger Snapshots als die andere). Aufruferkette:
Evidenzempfänger → `paare_bei_bedarf_bilden` → `haelfte_aus_historie` → `beurteile_paar`.

**(c) Gebrochener Satz:**
M-23 (`docs/beweise/SONDE-013.md:498`): „Im v3-Pfad sind es **genau drei** Gründe: Überlappung
unter 80 % des kürzeren Fensters; **aktive Messzeit um mehr als 10 % verschieden (Smart Disable
oder Stille auf einem Punkt)**; verschiedene Host-PIDs." Der zweite Grund ist im Produktpfad
tot. Weiter: „Keine Herabstufung ist ein stiller Ausschluss." — hier ist es umgekehrt ein
stilles **Hochstufen** auf `probable`/`feature_aligned`.
Mittelbar auch Exit-Gate Satz 2 (`:4288`), da die fehlende Herabstufung eine stärkere Klasse
stehen lässt, als das Material trägt.

**(d) Register: neu.**

**(e) Fixgröße: klein bis mittel.**
- `broker/src/coordinator/prepost_verdrahtung.rs`: zwei getrennte Felder statt einem —
  die Projektspanne für `schaetze_restlag`/`capture_s` (`prepost.rs:1010`) behalten, die
  summierte Messzeit als eigenes `gemessene_zeit_s` an `Paarhaelfte` hängen; `prepost.rs:1033`
  liest das neue Feld. Betroffen: `prepost.rs` (Struktur `Paarhaelfte` `:143ff`, Prüfung
  `:1033`), `prepost_verdrahtung.rs:254-277`, plus die Testhelfer in
  `broker/tests/sonde013_prepost.rs:59-84`. ~30-40 Zeilen.
- **Messender Test: neu** in `broker/tests/sonde013_verdrahtung.rs` — zwei Quellen, gleiche
  Projektspanne, eine Hälfte mit halber Snapshotzahl, Erwartung
  `herabstufungen.contains(&AktivzeitKlafft)`. Die bestehenden `sonde013_prepost.rs`-Fälle
  (`:643`, `:652`) bleiben als Einheitsprüfung des Riegels.

---

## C6 · `_passt` lässt starke Aussagen auf unvergleichbare Wahrheiten durch

**(a) BESTÄTIGT, und der Widerspruch steht neun Zeilen auseinander in derselben Datei.**

- `tools/eq-copilot/pruefe_p4_korpus.py:127-139` — `_passt`:
  ```
  if fall["aussage"] == ENTHALTUNG: return True
  return fall["wahrheit"] not in ("unbekannt",)
  ```
  Ein Fall mit `wahrheit="unvergleichbar"`, `"nicht_kausal"` oder `"nicht_exakt"` und
  `aussage="stark"` gibt **True** zurück.
- `:91-92` — `moegliche_starke` schließt genau diese drei zusätzlich aus:
  ```
  moegliche_starke = len([f for f in faelle if f["wahrheit"] not in
                          ("unbekannt", "unvergleichbar", "nicht_kausal", "nicht_exakt")])
  ```
- Folge in `:85`: `falsche_starke = [f for f in stark if not _passt(f)]` bleibt leer →
  `:177` und `:190` schlagen nicht an → `main` gibt 0 zurück und druckt `:211`
  „Exit-Gate P4: alle vier Klassen bestehen, keine falsche starke Behauptung."
- Nebenschaden, der den Widerspruch sichtbar macht: `:90-94` — `richtige_starke` kann größer
  werden als `moegliche_starke`, `recall` also über 1,0 steigen. Die Kennzahl widerlegt sich
  selbst, ohne dass ein Gate darauf reagiert.

**Negativtest des Prüfers: fehlt.** Es gibt kein Testartefakt für
`tools/eq-copilot/pruefe_p4_korpus.py` (Grep über das Repo: nur der Runner
`tools/beweise.ps1`, das Manifest und die Beweisdateien referenzieren es).
Der einzige Beleg ist der **Rotbeweis** `docs/beweise/roh/SONDE-013-rot-M-79.txt` — und der
verwendet ausgerechnet den einen Wert, den `_passt` fängt:
`"fall": "unknown_time_path_can_never_reach_strong"` mit `wahrheit: "unbekannt"`,
`aussage: "unsicher" → "stark"`. Dieselbe Manipulation an
`gate6_incomparable_never_gets_a_strong_winner` (`wahrheit: "unvergleichbar"`) oder
`nonlinear_chain_never_yields_static_eq_claim` (`wahrheit: "nicht_kausal"`) bliebe **grün**.
Der Rotbeweis erzeugt also Vertrauen, das er nicht deckt.

**(b) Kein Laufzeitpfad im Produkt — ein Prüfwerkzeug.** Die Erreichbarkeit ist die des
Kanons: `tools/beweise.ps1:577` führt A26 (`pruefe_p4_korpus.py`) als Pflichtbein der Phase P4.
Ein Regressionsfall, der genau die Gate-6/Gate-7-Klasse verletzt, würde nicht bemerkt.

**(c) Gebrochene Sätze:**
- M-79 (`docs/beweise/SONDE-013.md:621`): „Neben Precision und Recall zählen Kalibrierung,
  Brier Score, Coverage, Enthaltungsrate und die Zahl **falscher starker Behauptungen**."
  und die Belegzeile „die Zahl falscher starker Behauptungen (**sie muss null sein**)".
- **§49.2 Gate 6** (`:3560`) wörtlich: „Ein nicht vergleichbares Experiment erhält ein starkes
  Siegerurteil." — genau diese Verletzung wiese A26 als grün aus.
- **§49.2 Gate 7** (`:3561`): „Eine Standard-Insertprobe wird als exakter
  Mastersummenbeitrag bezeichnet." — dito, über `wahrheit="nicht_exakt"`.
- M-85 (`:627`) mittelbar, da die drei Falsifikationsziele über diese Kennzahl gemessen werden.
- CLAUDE.md, Arbeitsregel: „Fortschritt erst nach einem Beleg aus der laufenden Session
  behaupten."

**(d) Register: neu.**

**(e) Fixgröße: klein.**
- `tools/eq-copilot/pruefe_p4_korpus.py:139`: eine benannte Konstante
  `UNSICHERE_WAHRHEITEN = ("unbekannt", "unvergleichbar", "nicht_kausal", "nicht_exakt")`,
  von `_passt` **und** `moegliche_starke` (`:91-92`) gelesen — eine Wahrheit statt zwei.
  ~5 Zeilen.
- Zusätzlich sinnvoll: harter Riegel `recall <= 1.0`, sonst rot.
- **Messender Test: neu.** Ein Selbsttestmodus (`--selbsttest`) oder ein kleines Beweisbein,
  das je Wert aus `UNSICHERE_WAHRHEITEN` einen synthetischen Fall mit `aussage="stark"` durch
  `_kennzahlen` schickt und `falsche_starke == 1` erwartet — vier Fälle statt des einen, den
  der heutige Rotbeweis fährt.

---

## C7 · Der Korpus nennt Testfälle, die es nicht gibt

**(a) BESTÄTIGT — und breiter als beim Prüfer.**

- Der Prüfer schließt nur vom Kürzel auf die Messung:
  `tools/eq-copilot/pruefe_p4_korpus.py:161-163`
  ```
  for bein in klasse["beine"]:
      if not _bein_bekannt(bein, runnertext): rot.append(...)
  ```
  `_bein_bekannt` (`:62-74`) sucht per Regex `Kuerzel\s*=\s*'B18'` in `tools/beweise.ps1`.
  Das Feld `fall` jedes Korpuseintrags (`erzeuge_p4_korpus.py:66-137`) wird **nirgends**
  gegen eine Quelle geprüft; `baue()` (`:159-174`) validiert nur Feldmenge, Aussagewert und
  Konfidenzbereich.

**Fallnamen des Korpus gegen die Testquellen** (`rg` über `*.cpp *.h *.rs *.ps1`,
`eq-copilot/fixtures/p4-korpus/` ausgenommen):

| Klasse | Bein | Fallname | Existiert? | Datei |
|---|---|---|---|---|
| referenz | B17 | `true_peak_matches_ebu_test_set` | **nein** | – (Sachäquivalent: `Sonde013TruePeakGoldenTest.cpp:127` „EBU Tech 3341, Testfaelle 15 bis 19") |
| referenz | B17 | `eight_times_oversampling_error_bound` | **nein** | – (Sachäquivalent: `Sonde013TruePeakGoldenTest.cpp:216` „Die 8-fach-Entscheidung …") |
| referenz | B18 | `lufs_integrated_matches_reference` | **nein, auch nicht sachlich** | `Sonde013DynamicsTest.cpp` führt **keinen** LUFS-I-Referenzfall und keinen `LoudnessAccumulator`-Test; die ±0,1-LU-Prüfung liegt in **B9** `EqCopLoudnessGoldenTest` (`tools/beweise.ps1:619`) |
| referenz | B18 | `lra_below_sixty_seconds_is_unstable` | **nein** | Sachäquivalent heißt `lra_below_sixty_seconds_is_not_a_number` (`Sonde013DynamicsTest.cpp:497`) |
| referenz | B18 | `psr_against_true_peak_of_same_window` | **nein** | Sachäquivalent: `plr_psr_definitions_and_labels` (`Sonde013DynamicsTest.cpp:265`) |
| zeitachse | B5 | `no_window_crosses_an_epoch_boundary` | **nein** | – (`AnalysisGoldenTestMain.cpp`) |
| zeitachse | B5 | `drop_counts_as_segment_not_epoch` | **nein** | – |
| zeitachse | B4 | `drop_produces_counter_gap_and_boundary` | **nein** | – (`QueueStressTestMain.cpp`) |
| zeitachse | B22 | `fingerprint_window_never_crosses_epoch_boundary` | **ja** | `eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp` |
| zeitachse | A4 | `epoch_change_invalidates_open_evidence` | **ja** | `broker/tests/sonde013_taint.rs` |
| vergleich | B24 | `nonlinear_chain_never_yields_static_eq_claim` | **ja** | `eq-copilot/plugin/tests/Sonde013PrePostGoldenTest.cpp:273` |
| vergleich | A4 | `distributed_pair_never_produces_transfer_function` | **ja** | `broker/tests/sonde013_prepost.rs:475` |
| vergleich | A4 | `gate6_incomparable_never_gets_a_strong_winner` | **ja** | `broker/tests/sonde013_experiment.rs` |
| vergleich | A4 | `comparability_needs_all_five_and_hash_alone_never_wins` | **ja** | `broker/tests/sonde013_passage.rs` |
| vergleich | A4 | `unknown_time_path_can_never_reach_strong` | **ja** | `broker/tests/sonde013_prepost.rs:542` |
| vergleich | A16 | `insert_probe_is_never_called_master_sum` | **nein** | Sachäquivalent in `eq-copilot/plugin/tests/SondeNullTestMain.cpp:259` (`post_fader_contribution` read-only) |
| stereo | B19 | `mono_identity` | **ja** | `Sonde013StereoGoldenTest.cpp:160` |
| stereo | B19 | `identical_stereo` | **nein** | – |
| stereo | B19 | `polarity_inversion` | **nein** | Sachäquivalent `Sonde013StereoGoldenTest.cpp:692` „§40.3 Polaritaetsinvertierung …" |
| stereo | B19 | `known_delay` | **nein** | Sachäquivalent `:495` „bekannte Laufzeit: die Phase folgt -2*pi*f*tau" |
| stereo | B19 | `uncorrelated_channels` | **nein** | – |
| stereo | B19 | `mono_folddown_matches_folded_buffer` | **nein** | Sachäquivalent `:255` `folddown_matches_real_buffer_within_0p25db` |

**Bilanz: 8 von 22 Namen existieren wörtlich, 14 nicht.** 13 der 14 sind Namensdrift mit
vorhandenem Sachäquivalent; **einer ist substanzlos**: `B18::lufs_integrated_matches_reference`.
B18 (`EqCopSonde013DynamicsTest`) prüft laut Runner-Behauptung (`tools/beweise.ps1:659`)
Momentary/Short-term-Trennung, `integration_samples`, PSR, Headroom, Crest, LRA und
NaN-Ehrlichkeit — kein LUFS-I gegen eine validierte Referenz. Genau das ist die zweite
Hälfte von M-81.

**(b) Kein Produktpfad — dieselbe Werkzeugreichweite wie C6.** Ein Beweiskorpus, der auf einen
nicht existierenden Fall zeigt, meldet ihn als „gemessen"; ein Löschen oder Umbenennen eines
echten Falls fällt ebenfalls nicht auf. `_bein_bekannt` kann das strukturell nicht sehen.

**(c) Gebrochene Sätze:**
- M-81 (`docs/beweise/SONDE-013.md:623`): „Toleranz ±0,1 LU und ±0,1 dB gegen eine validierte
  Referenz auf Standard-Testmaterial." Belegzeile: „**A26**, Klasse `referenz` — die fünf Fälle
  aus **B17** und **B18** (… **LUFS gegen Referenz** …)". Die genannte Quelle trägt den Fall
  nicht; der Beleg zeigt ins Leere.
- Exit-Gate Satz 1 (`:4287`): „Referenzkorpus … bestehen." — Ein Korpus, dessen Fälle nicht
  auffindbar sind, belegt kein Bestehen.
- CLAUDE.md, Arbeitsregel: „Fortschritt erst nach einem Beleg aus der laufenden Session
  behaupten." und „Volatile Zahlen … aus Code, Manifest oder dem jüngsten Beweis lesen."

**(d) Register: neu.**

**(e) Fixgröße: mittel.**
- `tools/eq-copilot/erzeuge_p4_korpus.py`: je Fall ein Feld `quelle`
  (Datei + Suchmuster), Fallnamen auf die tatsächlichen Bezeichner ziehen
  (`abschnitt`-Text bei C++, `fn`-Name bei Rust). ~25 geänderte Zeilen im `FAELLE`-Dict.
- `tools/eq-copilot/pruefe_p4_korpus.py:161-163`: zweite Schleife, die jeden `fall` per
  `re.search` in seiner `quelle` sucht und sonst rot meldet. ~15 Zeilen.
- Sachlücke separat: entweder `B18::lufs_integrated_matches_reference` durch den echten
  **B9**-Fall ersetzen (mit korrigiertem `bein`) **oder** M-81 ehrlich auf „gegen die
  ausgebaute Vektorrechnung, nicht gegen ein externes Referenzset" zurücknehmen — das ist
  eine Zusagefrage, keine reine Werkzeugfrage.
- **Messender Test: neu**, derselbe `--selbsttest` wie in C6: ein erfundener Fallname muss
  Exitcode 2 erzeugen.

---

## C9 · Der gekoppelte 0,01-dB-Evidenzpfad ist ungemessen

**(a) BESTÄTIGT.**

M-83 wörtlich (`docs/beweise/SONDE-013.md:625`), Zusagespalte, drei Sätze:
> „Verteilte Feature-Goldens mit 0 bis 2 s bekannter Verzögerung finden den Lag innerhalb
> **eines Feature-Hops**. Der verteilte `q_db_0p1_i16`-Livepfad wird nicht gegen die
> 0,05-dB-Grenze des lokalen Audio-Compare geprüft; **fokussierte 0,01-dB-Evidenz muss Gain
> innerhalb ±0,1 dB samt statistischem Intervall wiederfinden.** Kompression, Modulation,
> Saturation und wechselnde Latenz erzeugen keine falsche statische EQ-Behauptung."

Der Belegtext derselben Zeile deckt nur Satz 1 und Satz 3 ab: „**A26**, Klasse `vergleich`
zusammen mit **B24** und **A4** — der Lag wird auf einen Feature-Hop genau gefunden (§10.7),
und Kompression, Modulation, Saturation und wechselnde Latenz tragen keine statische
EQ-Behauptung." **Satz 2 wird im Beleg nicht genannt und nirgends gemessen.**

Was wirklich existiert:

- Die Kodierung ist gebaut und wird beidseitig dekodiert:
  `eq-copilot/plugin/core/analysis/BandGrid.h:84,140`,
  `FeatureEngine.h:3519, 3856, 3887`,
  `broker/src/coordinator/evidenz.rs:335` (`Some("q_db_0p01_i16") => 100.0`),
  `broker/src/telemetrie.rs:980`.
- Gemessen wird davon nur die **Grenzwertkonsistenz** zwischen Schema, Rust und C++:
  `broker/tests/contract_cross_language.rs:304-305, 339-342`,
  `eq-copilot/plugin/tests/SchemaTestMain.cpp:485-529`.
- **B24** (`Sonde013PrePostGoldenTest.cpp`) hat drei Abschnitte: `:273` M-18 Kohärenz,
  `:410` M-20 `match_gain_is_frozen_before_ab_not_tracked`, `:507` M-24
  `audible_delta_locked_without_compare_routing`. Der +15-dB-Gain (`:141`) wird nur benutzt,
  um zu zeigen, dass die **Kohärenz pegelunabhängig** bleibt — der Gainwert wird nirgends
  aus verteilter Evidenz zurückgewonnen und gegen ±0,1 dB gehalten. `match_gain` ist der
  **lokale** Audio-Vergleichspegel, genau die Größe, die M-83 Satz 2 vom verteilten Pfad
  trennt.
- **B25** (`EqCopSonde013ExperimentGoldenTest`, `tools/beweise.ps1:654`) prüft den
  Block-Bootstrap **isoliert**: Determinismus, Mittelwert enthalten, breiteres Intervall bei
  blockigem Material, kein Intervall bei leerer Eingabe. Ohne Bezug zu einer dekodierten
  Bandevidenz.
- Die Rust-Seite baut Hüllkurven als **Float direkt** auf:
  `broker/tests/sonde013_prepost.rs:33-104` (`huelle`, `verzoegere`, `haelfte`, `perfekt`)
  — sie kommen nie durch `p50_dekodieren`. Der einzige Test, der `p50_db` überhaupt anfasst,
  ist `broker/tests/sonde013_verdrahtung.rs:425` (`!historie.last().unwrap().p50_db.is_empty()`).

**Es gibt also keinen Test, der Sender → Brokerdekodierung → PRE/POST-Auswertung als eine
Kette fährt und den bekannten Gain ±0,1 dB samt Intervall wiederfindet.**

**(b) Kein Defekt im Betrieb — eine ungemessene Zusage.** Der Pfad selbst ist gebaut; ob er
die Toleranz hält, ist nicht belegt. Die Reichweite ist die von A26/M-83 im Exit-Gate.

**(c) Gebrochene Sätze:**
- M-83 (`:625`), Satz 2 wörtlich: „fokussierte 0,01-dB-Evidenz muss Gain innerhalb ±0,1 dB
  samt statistischem Intervall wiederfinden." Der Belegstatus der Zeile lautet trotzdem
  „**BELEGT, gemessen**".
- Exit-Gate Satz 1 (`:4287`) mittelbar.

**(d) Register: neu.**

**(e) Fixgröße: mittel.**
- **Messender Test: neu**, brokerseitig am ehrlichsten: `broker/tests/sonde013_verdrahtung.rs`
  — zwei Quellen, PRE mit bekanntem Bandpegel, POST = PRE + bekannter Gain, beide als
  `evidence_snapshot` mit `encoding: "q_db_0p01_i16"` eingespeist; Erwartung: das
  `Paarurteil`-`ergebnis` gibt den Gain je Band innerhalb ±0,1 dB wieder und trägt ein
  Intervall. Vorhandene Fixtures unter `eq-copilot/fixtures/v3/` als Vorlage. ~80-120 Zeilen.
- Korpus: neuer Fall in `erzeuge_p4_korpus.py` Klasse `vergleich` mit dem neuen Beinnamen,
  Manifest neu erzeugen (`--pruefen` erzwingt Bytegleichheit).
- Kein Produktcode nötig, sofern der Test grün läuft; sonst wird daraus ein eigener Befund.

---

## C11 · Der korrelierte, nichtkausale Distraktor fehlt vollständig

**(a) BESTÄTIGT.**

M-85 wörtlich (`docs/beweise/SONDE-013.md:627`), Zusagespalte:
> „Ein **korrelierter, aber nicht kausaler Distraktor** wird als Alternative gezeigt und nicht
> als sicherer Auslöser bezeichnet. Ein nicht vergleichbares Experiment erhält kein starkes
> Siegerurteil. Eine Standard-Insertprobe wird nie als exakter Mastersummenbeitrag bezeichnet.
> Diese drei sind die Falsifikationsziele, die das Gate **G4** … später gegen dieses Ticket
> fährt."

Derselbe Wortlaut steht im Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md:2619`.

Die B24-Fälle des Korpus (`tools/eq-copilot/erzeuge_p4_korpus.py:101-119`):

- `:102-104` `B24::nonlinear_chain_never_yields_static_eq_claim`, `wahrheit: "nicht_kausal"`,
  `aussage: "unsicher"`. Der Test dahinter (`Sonde013PrePostGoldenTest.cpp:127-272`) führt
  **dasselbe Quellsignal** (`quelle()`, `:143-155`) durch identische Kette, +15 dB Gain,
  linearen Tiefpass, Kompressor, Modulation, Saturation und wechselnde Latenz. Das sind
  **kausale, zeitvariable** Ketten — geprüft wird, dass die Kohärenz fällt und deshalb keine
  statische EQ-Behauptung entsteht. Ein **zweites, nur korreliertes Signal ohne kausale
  Verbindung** wird nirgends hergestellt.
- `:105-107` `A4::distributed_pair_never_produces_transfer_function` (`nicht_kausal`,
  `schwach`) — `broker/tests/sonde013_prepost.rs:475` prüft, dass ein verteiltes Paar
  Banddifferenzen und keinen Frequenzgang liefert; auch hier ein PRE/POST-**Paar**, kein
  Distraktor.
- `:108-113` decken Falsifikationsziel 2 (Gate 6), `:117-119` Ziel 3 (Gate 7).
  **Ziel 1 hat keinen Fall.**

**Repoweiter Beleg:** `rg -i 'distraktor'` über `*.rs *.cpp *.h *.py *.md` findet den Begriff
**ausschließlich** in Prosa: `docs/FL-Nakama-Sonden-Design-Entwurf.md:2403, 2619`,
`docs/bauaufteilung-sonden.md:398`, `docs/beweise/SONDE-013.md:627` und der archivierten
Fassung 0.4. **Kein Code, kein Test, kein Fixture.**

Sachlich ist das erwartbar: „als **Alternative** gezeigt und nicht als sicherer Auslöser
bezeichnet" setzt Alternativen und Ausschlussgründe voraus — das ist P5
(`CauseHypothesis`, `docs/FL-Nakama-Sonden-Design-Entwurf.md` §59). Die Zusage steht aber im
P4-Exit-Gate und ist in der Matrix als **„BELEGT, gemessen"** markiert.

**(b) Kein Defekt im Betrieb — eine unbelegte Zusage im Gate-Text.** Reichweite: A26 meldet
grün; das G4-Gate fährt genau dieses Falsifikationsziel und hat nichts, wogegen es fahren kann.

**(c) Gebrochene Sätze:**
- M-85 (`:627`), Ziel 1 wörtlich: „Ein **korrelierter, aber nicht kausaler Distraktor** wird
  als Alternative gezeigt und nicht als sicherer Auslöser bezeichnet." Belegstatus der Zeile:
  „**BELEGT, gemessen**".
- Exit-Gate Satz 1 (`:4287`): „… und adversariale Vergleichsfixtures bestehen."
- `docs/bauaufteilung-sonden.md:398` („Ursachenbehauptung provozieren — korrelierter
  Distraktor, Parent-Duplikat …").
- CLAUDE.md, Entscheide-/Belegregel: „Fortschritt erst nach einem Beleg aus der laufenden
  Session behaupten."

**(d) Register: neu.**

**(e) Fixgröße: mittel bis groß — mit einer Produktentscheidung davor.**

Zwei saubere Wege, beide technisch:

1. **Jetzt bauen (mittel).** Adversariale Fixture: zwei Quellen, die dieselbe langsame
   Pegelhüllkurve teilen (also hoch korreliert sind), deren Bandinhalt aber aus unabhängigen
   Erzeugern stammt — Kohärenz je Band bleibt niedrig, Hüllkurvenkorrelation hoch. Erwartung:
   das Paarurteil bleibt `Unclear`/`NichtBeurteilbar`, `ergebnis` bleibt `None`, und kein
   Ausgang trägt eine kausale Bezeichnung. Ort: `broker/tests/sonde013_prepost.rs` (neuer
   `fn korrelierter_distraktor_wird_nie_sicherer_ausloeser`) plus Korpusfall in
   `erzeuge_p4_korpus.py`. ~100 Zeilen. **Vorbedingung: C4 ist gefixt** — solange
   `ausschluss()` `Stationaer` serialisiert, würde genau dieser Test die falsche Aussage
   messen.
2. **Ehrlich vertagen (klein).** M-85 Ziel 1 in `docs/beweise/SONDE-013.md:627` von
   „BELEGT, gemessen" auf „nicht gemessen, Eigentümer P5/SONDE-014" zurücknehmen und als
   Registerzeile führen — weil „als Alternative gezeigt" ohne `CauseHypothesis` gar keinen
   Adressaten hat.

Weg 1 ist der stärkere und bleibt in P4 messbar (das Paarurteil existiert); Weg 2 wäre nur
zulässig, wenn die Zusage bewusst nach P5 wandert. Diese Wahl ändert, was das G4-Gate heute
prüfen kann, und gehört damit vor die Umsetzung.

---

## Querbezüge zwischen den Befunden

- **C1 + C2** sind zwei Hälften derselben Drahtmehrdeutigkeit: `heartbeatAlsJson`
  (`ControlClient.cpp:408-410`) lässt `false` weg, `befehl.rs:573` reagiert nur auf
  `Some(false)`. „Neutral" und „schweigt" sind ununterscheidbar. **C1 allein zu fixen
  vergrößert C2** — dann triggert der erste Heartbeat eines Reconnects mit laufendem Marker
  den Resync und löscht das von `control_ende` gesetzte Unknown. Beide gehören in einen
  Änderungssatz.
- **C4 blockiert die saubere Umsetzung von C11 Weg 1**: ein Distraktortest, der auf einem
  ausgeschlossenen Paar `kettenbefund == "stationaer"` misst, prüft die falsche Zusage.
- **C6 + C7** treffen dasselbe Werkzeug (`pruefe_p4_korpus.py`) und lassen sich in einem
  Selbsttestbein gemeinsam absichern; der bestehende Rotbeweis
  `docs/beweise/roh/SONDE-013-rot-M-79.txt` deckt genau den einen Fall ab, den C6 nicht
  betrifft.
- **C3 und C5** sind beide „richtig gerechnet, dann überschrieben bzw. weggeworfen": die
  Größe existiert im Code und wird vom nächsten Schritt entwertet. Beide brauchen keinen neuen
  Algorithmus, nur getrennte Felder bzw. einen entfallenden Reset.
