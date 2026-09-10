# SONDE-015 — S26–28: Lokaler EQ-Kern in der Sonde, Zustand und Automation, A/B, Layout v2

| Merkmal | Wert |
|---|---|
| Ticket | S26–28, `SONDE-015` (Phase P6), Leitungsname „Die Klangregelung in der Sonde sicher und speicherbar machen" |
| Etappe | **Etappe 4a gebaut auf `dc6c148a`; Erstprüfung 4a NEEDS_WORK mit sechs Defekten (§10.8), Nacharbeit VERTAGT hinter den Audit (User-Wort 10.09.2026, Register NAK-246). SONDE-015-Audit-Nacharbeit D8/D10/N-12/N-9 gebaut auf `4711dab0` und mit Wiederprüfung PASS abgeschlossen (§11, §11.8).** Reihenfolge: NAK-246 (`docs/beweise/roh/NAK-246-auftrag.txt`) → `/code-review ultra` (Skill §3.7) → Nacharbeit 1 der Etappe 4a → Etappe 4b (Fernweg-Anteile, voller Kanon). Etappe 1 (Matrix) nach sieben Matrixprüfungen, Konvergenzentscheid und NAK-245-Runde (§7); Etappe 2 (Verträge in drei Sprachen) nach Erstprüfung und zwei Nacharbeiten (§8); Etappe 3 (DSP-Kern als Bibliothek, B6 scharf) nach Erstprüfung und drei Nacharbeiten (§9). |
| Phase | **Etappe 2 abgenommen auf `6f2baba6`; Etappe 3 abgenommen auf `156445a8` (Erstprüfung NEEDS_WORK → Nacharbeit 1 → Wiederprüfung 1 NEEDS_WORK → Nacharbeit 2 → Wiederprüfung 2 NEEDS_WORK → Nacharbeit 3 → Wiederprüfung 3 PASS; das Budget von drei Runden ist genau ausgeschöpft).** Etappe 4a ist gebaut, ihre Prüfung steht aus (§10); Etappe 4b folgt. Die Abschlussprüfung über den ganzen Ticketbereich und der volle Kanon folgen nach Etappe 4b. |
| Urteil | **Offen.** Weder T1 noch T2 vergeben. Etappe 1 baut nichts und behauptet deshalb kein gemessenes Produktverhalten; ihr einziger Gegenstand ist, ob §3 die Zusagen des Gate-Textes vollständig und widerspruchsfrei trägt. |
| Prüfstufe | **T2 gefordert** (`docs/plan/plan.json`, Schritt S26–28, `"stufe": "T2"`). Heute vergeben: **keine**. |
| Prüfmodell | Codex `gpt-6-astra`, Effort **max**, lesend — für Matrix-, Erst-, Wieder- und Abschlussprüfung. `gpt-5.6-sol` max nur als Gegenprüfer. |
| Basis-SHA | **Etappe 1:** `e9dbf4b9c11d1cf72aebbecb835c9391a57a0532` — alle Zeilen- und Zeichenangaben in §1 und §2 gelten zu diesem Stand. **Etappe 2:** `2026031f0a8bb0baf12135020be654a82d354a6f`. **Nacharbeit 1:** `3df34963de3a887600f5f629fc637f52256ace78`. **Nacharbeit 2:** `c727e620c1d48bcee8ad698a27321980bbd7776e`. **Etappe 3:** `2672ed2bf9858c2c45a8ebbc5afbe0999fff6554`. **Etappe 3, Nacharbeit 1:** `bc76e98c3dc4a83b87c24bc81386ba8ca183863e`. **Etappe 3, Nacharbeit 2:** `5f4f72c804770534c0412735f277f5653b86de9f`. **Etappe 3, Nacharbeit 3:** `72851d299bd65429b6e99cba5fa9219a71b7b899`; jeweils beim Start gemessen, `git status --short` leer. **Etappe 4a:** `844b9c15935377e89d124ac33848e9cba1ecf4c2`. **Audit-Nacharbeit:** `07dfcb77c14f82ab3b3c77f7d277474458937520`. |
| Kanon vorher | **GRÜN 62/62, Exit 0 auf `1867cac2`** (Abschlusskanon NAK-230, Rohausgabe `docs/beweise/roh/NAK-230-1867cac.md`). Eigene Messung in dieser Sitzung: `git diff --stat 1867cac2..e9dbf4b9 -- eq-copilot broker tools` ist **leer**. Der Codestand des Basis-SHA ist mit dem beglaubigten Kanonstand identisch; die Commits dazwischen berühren nur `docs/`. |
| Kanon nachher | **Steht aus** — der volle abgekoppelte Kanon gehört an das Ende der Etappe 4b. Etappe 2 fuhr die Beine ihres Bauplans einzeln (§8.4), Nacharbeit 1 die betroffenen (§8.9), Nacharbeit 2 die zwei Broker-Beine (§8.11), Etappe 3 die ihren (§9.4); alle grün laut Rohdateien. Etappe 4a fuhr die Beine, deren Ziel eine geänderte Quelle übersetzt oder linkt, samt der gemessenen Bundles und Release-Broker (§10.4): 36 von 37 mit Exit 0; rot ist B28, seit dem Fassungsschritt der Etappe 2 und nicht durch 4a (§10.7 N-12). Die Audit-Nacharbeit fuhr zehn Beine einzeln (§11.4), alle mit Exit 0; B28 ist wieder grün (81/81). |
| Ticketpfade | **Etappe 1:** diese eine Datei, `docs/beweise/SONDE-015.md`. **Etappe 2:** die Vertrags-, Plugin-, Broker- und Werkzeugpfade aus §4.5, ohne `eq-copilot/plugin/dsp/` (Etappe 3) und ohne `SondeProcessor.*` (Etappe 4). **Etappe 3:** `eq-copilot/plugin/dsp/` (neu), `eq-copilot/plugin/tests/DspGoldenTestMain.cpp` (neu), `eq-copilot/plugin/CMakeLists.txt`, `tools/beweise.ps1`, `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` (A14-Objektliste, §9.6), dieses Manifest und die Rohdateien. Der Prozessor bleibt nach §4.1 unberührt; der Auftrag wird nicht angefasst. **Etappe 4a:** `eq-copilot/plugin/state/NakamaTransaktion.{h,cpp}` (neu), `eq-copilot/plugin/sonde/SondeProcessor.{h,cpp}`, `eq-copilot/plugin/dsp/DspKern.{h,cpp}` (Schnittstelle, §10.6 A-1), `eq-copilot/plugin/tests/TransactionTestMain.cpp` (neu), `eq-copilot/plugin/tests/SondeNullTestMain.cpp`, `eq-copilot/plugin/CMakeLists.txt`, `tools/beweise.ps1`, `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` (A14-Liste, §10.6 A-2), dieses Manifest und die Rohdateien. **Audit-Nacharbeit:** `eq-copilot/plugin/state/NakamaParameter.cpp`, `eq-copilot/plugin/state/NakamaPreset.cpp`, `eq-copilot/plugin/dsp/DspKern.h` (lesender Testzugang, §11.6 A-1), `eq-copilot/plugin/tests/StateMigrationTestMain.cpp`, `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`, `eq-copilot/plugin/tests/Sonde014BefundTest.cpp`, `tools/eq-copilot/erzeuge_state_fixtures.py`, `eq-copilot/fixtures/state/` (sieben Fixtures und `MANIFEST.json`), dieses Manifest und die Rohdateien `docs/beweise/roh/SONDE-015-audit-*` und `SONDE-015-rot-*-audit.txt`. |
| Grenze | Etappe 2 baut ausschließlich, was §3 zusagt. Technikfragen ohne Quelle sind vom Dirigenten als R1 bis R15 entschieden (§5); meine Feinheiten stehen je Regel darunter, Abweichungen mit Begründung. Produktfragen werden benannt und einer Fläche zugeordnet, nie hier beantwortet (§5.16). |

<!-- NAKAMA-URTEIL: OFFEN -->

**Alles unter diesem Kopf ist append-only Verlauf.** Der lebende Kopf wird beim
Abschluss nachgezogen; Positionen im Kopf stehen als Symbol oder Anker, im
Verlauf gilt eine Zeilennummer zum Stand des Abschnitts, der sie trägt
(`tools/dirigent/pruefliste.md`, Abschnitt E).

**Die Matrix-IDs `M-01` bis `M-125` dieses Manifests sind eigen.** Sie sind
nicht die `M-01` bis `M-89` aus `docs/beweise/SONDE-014.md`; wo eine Zeile eine
fremde Zusage fortführt, nennt die Quellspalte sie ausdrücklich.

**Warum Etappe 1 überhaupt.** Dieses Ticket ist das erste, das **Audio
verändert**. Bis heute ist jede Nakama-Instanz ein bewiesener Passthrough, und
jeder Nulltest im Kanon misst genau das. Ab hier gilt der Nulltest nur noch für
einen von zwei Betriebszuständen, und die Grenze zwischen beiden ist selbst eine
Zusage, die gemessen werden muss. Dazu kommen drei teure Klassen in einem
einzigen Ticket: **Verträge** — ein neues Parameterlayout, ein neues
State-Kind, ein neues Presetschema, ein eingelöstes v3-Feld und ein neues
FlatBuffers-Feld, jedes mit Writer, Leser, Migration, Fixtures und
Cross-Language-Verbraucher; **Echtzeitcode** — vier Bänke mit lockfreiem
Ownership, Blockrandtausch, Crossfade und Rampen, wo ein Fehler nicht rot wird,
sondern knackt; **Transaktionen** — Apply, Revert, Neutralisieren, Remove,
Undo, Redo und Preset-Laden, die je genau eine Revision erzeugen und nie einen
halben Zustand hinterlassen. Nach der Dirigentenregel „Spezifikation vor Code"
entsteht deshalb zuerst die Verhaltensmatrix; ein lesender Codex-Thread prüft
nur sie gegen Entwurf, Verträge und Gate-Text, erst danach baut ein frischer
Worker.

---

## 1. Gate-Text, wörtlich

Jede Zeile in §3 ist eine Behauptung aus diesem Abschnitt, nicht aus meinem
Kopf. Der Abschnitt zitiert; er fasst nicht zusammen.

### 1.1 `docs/plan/plan.json`, Schritt S26–28, vollständig

Gelesen am Basis-SHA aus `docs/plan/plan.json`, Phase P6, Schritt mit
`"id": "S26–28"`, `"ticket": "SONDE-015"`, `"stufe": "T2"`, Leitungsname
„Die Klangregelung in der Sonde sicher und speicherbar machen". Das Feld
`"text"` in voller Länge, einschließlich aller Nachträge vom 30.08., 01.09. und
02.09.2026:

> Lokaler EQ-Kern in der Sonde: vier vorbereitete Bänke, Zustand und
> Automation, A/B — dazu die versionierte Zustands-Erweiterung aus dem
> 23.08.-Entscheid: Mix je Sonde und die Schutz-Zonen bekommen ihren Platz im
> gespeicherten Zustand (Layout v2, neue Kennungen, nie umgewidmet). NACHTRAG
> 30.08.2026 (PR1, Befund D-3): Auto-Gain gehört in denselben Änderungssatz.
> Der Entscheid vom 24.08.2026
> (design/abnahmen/2026-08-24-auto-gain-schalter.md) legt einen Schalter AUTO
> neben OUTPUT fest, Standard aus, mit sichtbar mitwanderndem Ausgangswert; er
> nennt S26–28 selbst als Ort. Das verlangt eine neue globale Parameter-ID —
> nakama-parameter-v1.json kennt global nur Bypass, Input-Trim, Output-Trim,
> Width und Mono-Bass. Auto-Gain ist damit die dritte Layout-v2-Erweiterung
> neben Mix und Schutz-Zonen; ob der Ausgleich als eigener persistenter
> Parameter mitläuft oder aus der Kurve abgeleitet wird, entscheidet die
> Technik an der Quelle. Der Rechenweg entsteht im aktiven Kern derselben
> Phase. NACHTRAG 30.08.2026 (PR1 Runde 1, Codex-Befund B3): Mit dem DSP-DTO
> entsteht hier auch die Nutzlast von state_report.dsp — der bestätigte DSP
> samt Schutzgrenzen, dessen NAME in der v3-Vertragsrunde von S16–17
> reserviert wird. Entwurf §33.5 verlangt, dass Broker und Main den
> bestätigten DSP per state_report lesen und ihn nie blind überschreiben; das
> Feld gibt es heute nicht — state_report in
> eq-copilot/schemas/v3/eq-ipc-v3.schema.json ist additionalProperties = false
> und trägt nur dsp_schema_version, state_revision, state_hash, record_state
> und undo_tiefe. Ohne es kann Main nach einem Project-Reload oder einem
> Reconnect Bänder, Bypass, Mix und Schutzbereiche weder anzeigen noch
> konfliktfrei wiederherstellen; ein Hash sagt nur, DASS etwas anders ist.
> Register NAK-110. ENTSCHIEDEN UND GESCHLOSSEN 30.08.2026 (Antwort U15, Wahl
> „Nein, dauerhaft ohne“): Nakama bleibt dauerhaft minimalphasig. Ein zweiter,
> linearphasiger Betriebsmodus wird NICHT gebaut — weder in der ersten Fassung
> noch als späterer Ausbau — und es wird dafür auch KEIN Name reserviert. Am
> Bau ändert sich damit nichts, und genau das ist hier festzuhalten: die an FL
> gemeldete Verzögerung bleibt null, der gespeicherte Zustand bekommt keinen
> Phasenmodus-Wert (Layout v2 wächst dafür nicht) und die Oberfläche keinen
> Umschalter. Die Karte hatte S26–28, S28b und das Prüftor G6 als betroffen
> genannt; alle drei bleiben unverändert. Die in der Karte genannte
> Abhängigkeit von der ungemessenen Frage, ob FL eine gemeldete Plugin-Latenz
> sauber herausrechnet (Termin A2, Karte U11), entfällt damit für den EQ; ob
> U11 aus anderen Gründen offen bleibt, entscheidet U11 selbst. Wortlaut:
> design/abnahmen/2026-08-30-fragenrunde-marktstandard.md. LIEFERUMFANG
> ERWEITERT 30.08.2026 (Antwort U18, Wahl „Beides (Empfohlen)“): Nakama
> bekommt gespeicherte Voreinstellungen UND den Weg, eine fertige Kurve von
> einer Sonde auf eine andere zu übertragen. Für dieses Ticket heißt das:
> Layout v2 bekommt den Platz für ein eigenes Preset-Objekt. Eine
> Voreinstellung ist ausdrücklich NICHT der Projektzustand — sie lebt
> außerhalb des Projekts, wird ohne Projekt geladen und braucht deshalb eigene
> Kennungen und eine eigene Version, die nie mit der Zustandsversion vermischt
> wird; Altstände müssen laden, und unbekannte Felder dürfen alte Consumer
> nicht zerstören. Was das Preset trägt, ist Klanginhalt: Bänder, Bypass, Mix,
> die Schutz-Zonen und Auto-Gain. Was es ausdrücklich NICHT trägt, ist
> Identität — Zieladresse, instance_id, Bus-Name und das label der Sonde
> bleiben draußen, sonst würde das Laden einer Voreinstellung die Sonde
> umverdrahten statt ihren Klang zu setzen. Dieselbe Grenze gilt für das
> Kopieren von Sonde zu Sonde: die Kurve reist, die Identität bleibt. Der Weg
> dafür ist die bestehende Fernsteuerung über Gen (S29–31) und kein neuer
> Kanal zwischen zwei Sonden — die vollständige EQ-Bedienung existiert nur in
> Gen. Die Bedienung selbst gehört zu S31b. Belegt ist der Marktstandard nur
> für die Voreinstellungen (FabFilter Pro-Q 4 Preset-Browser, Voxengo SPAN
> Plus Preset-Manager); das Kopieren zwischen Instanzen ist bei keinem
> verglichenen Produkt belegt und damit ein Nakama-eigener Handgriff — Lücke
> L-3 aus docs/beweise/PR1.md. Wortlaut:
> design/abnahmen/2026-08-30-fragenrunde-marktstandard.md. NACHTRAG 01.09.2026
> (User-Meldung, design/abnahmen/2026-09-01-dynamisches-band-schwingt-mit.md):
> Der dynamische EQ liefert je Band die momentane dynamische
> Verstaerkungsaenderung als laufenden Telemetriewert mit Anzeigekadenz - denn
> das Band muss im Kurvenbild sichtbar mitschwingen (User-Wort: „bei
> einschalten des dynamischen eq muss das band dynmaisch mitschwingen wie man
> es von anderen eq kennt“). Der Feldname im Featureframe wird nach der
> Reservierungsregel noch vor G2/R0 reserviert (Eigentuemer dieses Ticket);
> die Nutzlast entsteht hier mit dem EQ-Kern. Gens Master-EQ (S28b) fuehrt
> denselben Wert lokal ohne IPC. Die Anzeige gehoert zu S31b. NACHTRAG
> 02.09.2026 (PR2, Design-Strang, Blueprint Paragraph 15 Punkte 6, 10, 15 und
> 16): Drei Stuecke Vertragsarbeit, die die Abnahmen vom 31.08. und 01.09.2026
> verlangen und die bisher keinen Planschritt hatten, liegen hier. (1)
> BAND-BELEGUNG UND WIEDERHERSTELLBARES REMOVE. Abgenommen ist
> (design/abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-02.md,
> 4/5 und 5/5, sowie der Sammelstand in
> 2026-08-31-technische-ui-architektur-arbeitsmodus.md): 'enabled = false' und
> 'Slot frei' sind zwei verschiedene Zustaende; ein ausgeschaltetes Band
> behaelt Slot, Werte und Band-ID; nur ein ausdrueckliches Remove gibt einen
> der acht Slots frei; Remove ist eine atomare Transaktion, andere Slot-IDs
> werden niemals umnummeriert; direkt danach steht ein eindeutiger
> Undo-Rueckweg bereit, der Slot-ID, Typ, Kanalmodus, Dynamic-Zustand und
> saemtliche Werte als EIN Objekt wiederherstellt. Der heutige
> Parametervertrag (eq-copilot/schemas/state/nakama-parameter-v1.json) kennt
> 'enabled' als reversiblen Bypass, aber keinen persistenten Belegungszustand
> und keine Slotfreigabe. Dieser Schritt liefert deshalb die versionierte
> Erweiterung: persistenter Belegungszustand, atomare Slotfreigabe,
> Undo-Objekt - Writer, Leser, Migration, Goldens und Kanon-Bein im selben
> Aenderungssatz, unter derselben festen Band-ID. Bis dahin bleibt der native
> Remove-Weg technisch blockiert; ein sichtbarer produktiver Remove-Control
> ohne diesen Vertrag waere unehrlich (Blueprint Paragraph 15 Punkt 16). (2)
> FRAME.BAND_DYNAMIC_GAIN_DB. Der Name ist reserviert und traegt heute keine
> Nutzlast - gemessen am 02.09.2026 in
> eq-copilot/schemas/v3/reservierte-nachrichten-v1.json, Eintrag
> 'Frame.band_dynamic_gain_db', Eigentuemer 'S26-28 (SONDE-015)'. Die Nutzlast
> entsteht hier: Probeeq liefert den Wert mit Anzeigekadenz ueber den
> Featureframe. Er muss gegen stale und nicht-endliche Werte abgesichert sein
> und darf nie aus den Einstellwerten erfunden werden
> (design/abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-01.md,
> 5/5, und die dortige 'Technische Grenze der Live-Bewegung'). Unterhalb des
> Thresholds oder bei dynamic_range_db = 0 darf ein korrekt aktives Band
> stillstehen - der persistente Aktivbeleg ist die zweite Kontur am Bandpunkt,
> nicht die Bewegung. (3) DYNAMIC-PARAMETER BLEIBEN GETRENNT. dynamic_enabled
> und die fuenf Werteparameter dynamic_range_db, threshold_db, attack_ms,
> hold_ms und release_ms sind getrennte persistente Parameter; Ausschalten
> aendert ausschliesslich dynamic_enabled und setzt die fuenf Werte NICHT
> still auf Defaults zurueck (Fuenferblock 01, 1/5 bis 3/5). Dasselbe gilt
> fuer channel_mode: diskreter Blockrandparameter mit Default 'stereo', kein
> Freitext, keine Mehrfachwahl, kein Zwischenzustand, nie mit Filtertyp oder
> Dynamic-Zustand gekoppelt (Fuenferblock 02, 2/5).
> BLUEPRINT-PARAGRAPH-15-ZUORDNUNG: Punkt 6 (P6-Testzugriff - der EQ-Kern
> braucht Beweise, aber keinen reaktivierten Probeeq-Voll-Editor; Entscheid
> 27.08.2026 'nein') und Punkt 10 fuer state_report.dsp (reserviert,
> Eigentuemer S26-28) liegen ebenfalls hier.

Der Schritttext führt damit **fünf** Lieferblöcke: den lokalen EQ-Kern mit vier
vorbereiteten Bänken, Zustand und Automation und A/B; die versionierte
Zustands-Erweiterung Layout v2 mit Mix, Schutz-Zonen und (Nachtrag 30.08.)
Auto-Gain; die Nutzlast von `state_report.dsp` (Nachtrag 30.08., Register
NAK-110); das Preset-Objekt (U18, 30.08.); und die drei Stücke Vertragsarbeit
des PR2-Nachtrags vom 02.09. — Band-Belegung mit wiederherstellbarem Remove,
`Frame.band_dynamic_gain_db` und die getrennten Dynamic-Parameter samt
`channel_mode`. Dazu **eine** ausdrückliche Streichung: U15 schließt den
linearphasigen Modus dauerhaft aus, ohne Namensreservierung.

### 1.2 Entwurf §30.1 „Aktiver Werkzeugumfang" (`:1681-1696`)

> Der erste belastbar zu implementierende aktive Kern besteht aus:
>
> - Input-/Output-Trim;
> - minimumphasigen Hoch-/Tiefpässen, Bells, Shelves und Notches;
> - statischen und bandbezogenen dynamischen EQ-Bändern;
> - externer Sidechain je dynamischer Beziehung;
> - Stereo-, Links/Rechts- oder Mitte/Seite-Zuordnung pro geeignetem Band;
> - lokalem Dry/Processed/Delta-Hörpfad mit festem Lautheitsabgleich.
>
> Lineare Phase, breitbandige Sättigung, Limiting, Reverb, universelle Laufzeitkorrektur und ein
> vollspektraler FFT-Ducker gehören **nicht** in diesen ersten aktiven Kern.

### 1.3 Entwurf §44.2 „DSP-Kern" (`:2955-3018`), die bindenden Sätze

> Die aktive Probe besitzt acht feste Band-Slots mit stabilen Parameter-IDs und einen versionierten
> State-Tree. Unbenutzte Slots bleiben neutral; die Hostparameterliste ändert sich nie dynamisch.
> Manuelle Parameter dürfen beispielsweise 20 Hz bis `min(20 kHz, 0,45·fₛ)`, ±12 dB und Q
> 0,15–24 nutzen.

Der darauf folgende Satz „Remote-Proposals bleiben auf ±3 dB und zunächst Q 0,4–2
begrenzt" trägt in Fassung 0.5 den Grabstein „Gestrichen in Fassung 0.5 am
02.09.2026" und ist **Verlauf**; bindend sind ±12 dB und Q 0,15–24
(Erratum (n)/(q), Register NAK-108). Weiter wörtlich:

> - minimumphasige RBJ-Biquads als Referenz für statische Bell-, Shelf-, Notch- und Cut-Filter;
>   nahe Nyquist wird ein matched-analog-/Orfanidis-artiges Decramping geprüft;
> - topology-preserving State-Variable-Filter beziehungsweise robuste Biquads für dynamische
>   Bänder;
> - RMS-/Peak-Hüllkurven mit expliziter Attack/Hold/Release-Smoothing;
> - Stereo-, L/R- und M/S-Matrix mit normalisierter Energie;
> - vier vorallokierte Programmbänke: je ein Double-Buffer für Committed und Candidate, damit beide
>   unabhängig warm bleiben und innerhalb ihres Pfads klickfrei die Topologie wechseln können;
> - 64-Bit-Koeffizienten und -Filterzustände; 32-Bit- und 64-Bit-Hostpuffer werden unterstützt.
>
> Der Audiothread liest ein unveränderliches `DspProgram`, allokiert und sperrt nie. Stetige
> Parameter werden geglättet; Bandtyp, Kanalmodus oder Bandanzahl wechseln per kurzem Crossfade
> zwischen zwei vollständig vorbereiteten Programmen. Der erste aktive Kern meldet konstant
> 0 Samples Pluginlatenz und verwendet deshalb kein Lookahead oder lineare Phase.
>
> Im Normalbetrieb rechnet nur der aktive Committed-Pfad; Compare rechnet Committed plus Candidate.
> Ein gleichzeitiger Topologiewechsel kann für die begrenzte Fadezeit drei beziehungsweise im
> Worst-Case vier Bänke benötigen. Diese Last ist Teil des Worst-Case-CPU-Goldens. Reicht das
> Realtime-Budget nicht, wird Candidate vor dem Wechsel neutral beendet — nie eine Bank oder ein
> Filterzustand zwischen beiden Pfaden geteilt.
>
> Die vier Slots besitzen ein explizites lockfreies Ownership-Protokoll
> `free → preparing → ready(generation) → audio_active/fading → retired → free`. Der Control-Worker
> schreibt ausschließlich `free`-Slots und publiziert Index plus Generation mit Release-Semantik.
> Der Audiothread übernimmt nur am Blockrand, liest bis Fade-Ende unverändert und meldet die
> ausgediente Generation über einen vorallokierten Audio→Control-SPSC-Ring zurück. Erst nach diesem
> ACK darf der Worker Filterzustand oder Koeffizienten überschreiben. Ist kein Slot frei, erhält der
> Befehl `busy_retry`; es gibt weder In-place-Überschreiben noch Heap-Reclaim, `shared_ptr`-
> Destruktor oder Deallokation im Callback.
> Der ACK-Ring fasst mehr Einträge als es Slots gibt und droppt nie. Ein dennoch erkannter Overflow
> setzt zusätzlich eine atomare `reclaim_pending_mask`; der betroffene Slot bleibt damit
> dauerhaft nicht frei, bis der Worker ihn bestätigt. Reclaim-Sicherheit gewinnt über Verfügbarkeit.
>
> 32-/64-Bit-Puffer sind ein Zielvertrag; der heutige Code besitzt nur den Float-Callback. Vor einer
> 64-Bit-Capability müssen beide Callbackpfade, Analyse, M/S-Matrix, Filter und Nullpfad denselben
> Golden-Korpus bestehen. Meldet der Build diese Capability nicht, darf der Host keinen impliziten
> Konvertierungspfad als getestete Doppelpräzision ausgeben.
>
> Ein neutraler Active-State besitzt einen expliziten Hard-Bypass vor M/S-Matrix und Filterbank. Er
> schreibt bei In-place-Verarbeitung keine Samples und ist deshalb wie die passive Probe bitgenau
> sampleidentisch; ein rechnerischer Identity-Filter reicht nicht als Nullvertrag.
>
> Die aktive Probe führt drei logisch getrennte Analysetaps: `pre_nakama` vor eigenem DSP,
> `post_committed` hinter dem bestätigten Parallelpfad und, nur im Experiment,
> `post_candidate`. Session-Landkarte und Recall beziehen sich auf `post_committed`; Candidate-
> Frames tragen Experiment-ID und dürfen nie Baseline werden. Preview-/Focus-/Delta-/Marker-Matrix
> liegt **hinter** diesen Taps.

### 1.4 Entwurf §44.3 „Zwei Zustände und atomare Transaktion" (`:3020-3076`), die bindenden Sätze

> `CommittedState` und flüchtiger `PreviewOverlay` sind getrennt. Eine Netzwerk-/UI-Nachricht baut
> und validiert auf einem Nicht-Audiothread ein neues Programm. Erst am Blockrand tauscht die Probe
> atomisch die Generation. […] Das ACK enthält:
>
> - `command_id`, Ergebnis und angewendete `state_revision`;
> - SHA-256-`state_hash` über RFC-8785-kanonisiertes State-JSON;
> - tatsächlich geklemmte Parameter;
> - aktive Capability-/Policy-Version;
> - Fehler- oder Konfliktcode.
>
> `committed_revision` und `active_generation` bleiben getrennte Felder; ein Timeout darf einen
> gespeicherten Apply nicht fälschlich als verworfen darstellen.
>
> Committed-Parameter werden auf dem Message-Thread mit den exponierten Hostparametern
> synchronisiert. Trotzdem bleibt der eigene Revisions-/Undo-Ring die Rückfallgarantie: Remote-
> Änderungen, MIDI und Automation erzeugen nicht zuverlässig einen Plugin-Undo-Schritt.
>
> `state_revision` zählt ausschließlich diskrete Änderungen des bestätigten Basiszustands: Apply,
> Revert, Neutralisieren oder einen abgeschlossenen manuellen Parametergestus. Sample-offset-
> Automationspunkte erzeugen **keine** Revision pro Punkt. Sie bilden einen getrennten flüchtigen
> `AutomationOverlay` mit `automation_epoch`; der Epochzähler wechselt einmal beim Beginn und Ende
> einer Hostgeste beziehungsweise nach einer definierten Ruhegrenze. […] State-Tree-Copy/Replace und
> Parametergesten laufen nie im Audiothread. Sample-Offsets bleiben als Segmentgrenzen erhalten:
> kontinuierliche Werte verwenden eine spezifizierte, an den Hostpunkten endende Sicherheitsrampe;
> diskrete oder topologische Werte wechseln am Offset über das vorbereitete Crossfade. […]
> Realtime- und Offline-Render müssen denselben Parameterverlauf erzeugen.

Der Absatz danach beschreibt den JUCE-VST3-Bridge-Patch und den Rückfall
`sample_accurate_automation=false`. Er ist für dieses Ticket **Randbedingung,
nicht Lieferumfang**: der Patch existiert seit S3/SONDE-003 (Bein B3), und
welche Fassung die Sonde nutzt, entscheidet die gemessene Capability, nicht
dieses Manifest (§2.9, M-119).

### 1.5 Entwurf §44.4 „Ausfall- und Recallverhalten" (`:3078-3085`), vollständig

> - Bestätigter DSP bleibt bei Main-/Broker-Ausfall lokal unverändert und rendert offline.
> - Preview endet bei Lease-Ablauf und wird nie serialisiert.
> - Ungültiger State lädt neutral und meldet einen reparierbaren Migrationsfehler, statt teilweise
>   Parameter anzuwenden.
> - Ein Reconnect beginnt mit `state_report`; der Broker überschreibt keinen neueren lokalen State.
> - `Neutralisieren` ist selbst eine versionierte Transaktion und kein Löschen der Historie.

### 1.6 Entwurf §44.5 „Abnahmekriterien" (`:3087-3099`), vollständig

> - Passive Probe **und neutraler Hard-Bypass der Active Probe** bestehen bitgenaue Nulltests;
>   Active Probe besteht zusätzlich Filter-, Automations-, State-Migrations- und Offline-Render-
>   Goldens in Float und, sobald deklariert, Double.
> - Audio-Callback: keine Heapallokation, Locks, Pipe-, Log- oder Dateizugriffe; ThreadSanitizer-
>   beziehungsweise äquivalente Stressläufe finden keine Zustandsrennen.
> - 10.000 doppelte, vertauschte und veraltete Befehle erzeugen höchstens eine gültige Revision und
>   niemals einen Mischzustand.
> - Projekt-Reload rekonstruiert denselben State-Hash und innerhalb numerischer Toleranz denselben
>   Audioausgang.
> - Filtergoldens bleiben typisch innerhalb ±0,05 dB und an Extrempunkten innerhalb ±0,1 dB;
>   Automations-Zipperresiduen bleiben im definierten Ramp-Test unter −100 dBFS.

### 1.7 Entwurf §49.2 „Harte Systemgates", Gates 1 bis 5 (`:3557-3577`)

> Ein Build ist nicht freigabefähig, wenn eines dieser Gates fällt:
>
> 1. Passive Probe, neutrales Main oder neutraler Hard-Bypass der Active Probe verändert einen
>    gültigen Audiopuffer außerhalb der bereits dokumentierten, useraktivierten Hörmarker-Ausnahme.
> 2. Ein IPC-, Broker-, Datenbank- oder UI-Fehler blockiert den Audiothread.
> 3. Eine Preview überlebt Lease, Stop, Offline-Render oder Project-Reload oder beginnt bei
>    Recording beziehungsweise unbekanntem Record-State.
> 4. Ein Remote-Apply kann Hard Cap, `base_revision`, Ziel-Nonce, User-Schutz oder das gültige
>    `recording=false`-Gate umgehen.
> 5. Telemetrie steuert samplegenauen Gain oder erzeugt hörbares PRE/POST-Delta.

Gate 3 und Gate 4 nennen Preview und Remote-Apply. Beide **gehören S29–31**
(Ticketgrenze DRAUSSEN); hier bindet an ihnen genau das, was auch ohne
Fernweg gilt: eine Preview wird nie serialisiert (M-83), und der User-Schutz
aus Gate 4 existiert lokal als Zonenriegel (§3.8), damit S29–31 ihn nicht
erfinden muss.

### 1.8 Entwurf §49.3 „Messbare Startbudgets" (`:3586-3597`), die bindenden Zeilen

> | Größe | Startbudget ab der zugehörigen Phase (änderbar, versioniert; Erratum (f)) |
> |---|---:|
> | Pluginlatenz erster Active-Kern | 0 Samples, konstant |
> | A/B-Zustandswechsel | kein Klick; identischer Zustand nullt innerhalb numerischer Toleranz |
> | Soak | mindestens 60 min mit 16, mindestens 30 min mit 32 Sonden ohne XRun/ungegrenztes Wachstum |

Die Tabelle trägt **keine CPU-Zeile**. §44.2 nennt einen „Worst-Case-CPU-Golden",
ohne eine Zahl zu setzen. Daraus folgt R15 wörtlich: es entsteht **kein
erfundener Deckel**; die gemessene Last steht als Zahl im Manifest (M-118).

### 1.9 Entwurf §53.8 „Parameter- und Statevertrag" (`:4042-4102`), die bindenden Sätze

> State-Schema 2 verwendet den Root `NakamaState` und unabhängig versionierte Kinder:
>
> ```text
> NakamaState schema="2"
>   Common schema="1"           # Identität, Produktklasse, Messpunkt, Binding
>   MainProject schema="1"      # Intent, Mitgliedschaft, Passage, AssistantStep, Outbox
>   Parameters schema="1"       # APVTS-/Hostparameter
>   Dsp schema="1"              # Revision, Schutz, bestätigter DspState, Undo-Ring
>   Pairing schema="1"          # Ziel-IDs und Current-User-DPAPI-Blobs, nie Klartext
> ```
>
> `Common` ist Pflicht. `MainProject` existiert nur im klassifizierten Main, `Parameters` und `Dsp`
> nur in der Active Probe […]
>
> Der erste feste Parameterbestand lautet:
>
> - global: `v1.global.{bypass,input_trim_db,output_trim_db,width,mono_bass_hz}`;
> - je Slot `0..7`: `enabled`, `type`, `freq_hz`, `q`, `gain_db`, `channel_mode`,
>   `dynamic_enabled`, `dynamic_range_db`, `threshold_db`, `attack_ms`, `hold_ms`, `release_ms` und
>   `sidechain_source`, jeweils unter `v1.band.<slot>.`;
> - freie Slots bleiben neutral; IDs werden nie umbenannt oder wiederverwendet;
> - `sidechain_source` kennt nur `none|internal|priority_sidechain`: Es existiert genau ein
>   externer Sidechain-Aux (§30) und pro Zielsonde höchstens eine Prioritätsbeziehung (§45.3);
>   der Parameter wählt je Band nur zwischen interner Hüllkurve und diesem einen Bus;
> - Remoteverträge verwenden typisierte physikalische Werte, nie VST-normalisierte Zahlen;
> - `type`, `channel_mode`, `dynamic_enabled` und `sidechain_source` sind topologisch. Sie werden im
>   ersten Active-Release nicht samplegenau automatisiert, sondern nur am validierten Blockrand mit
>   vorbereitetem Crossfade gewechselt;
> - kontinuierliche Parameter verwenden genau eine definierte Rampe. Hostautomation, lokale UI und
>   Remote-Transaktion durchlaufen denselben Validator und dieselbe Programmbaugrenze.
>
> `state_hash` umfasst ausschließlich das validierte DSP-DTO in RFC-8785-kanonischem JSON, ohne
> UI-, Undo- oder Transientfelder. Nichtendliche Zahlen, doppelte Schlüssel und unbekannte
> Pflichtfelder werden vor dem Hash abgelehnt.

### 1.10 Entwurf §60 „P6 — Active Probe lokal und fernsteuerungsfrei" (`:4328-4350`), vollständig

> **Zweck:** Der aktive Audiokern wird zunächst ohne Netzwerkautorität als normales lokales Plugin
> bewiesen.
>
> **Lieferumfang:**
>
> - Hard-Bypass, Input-/Output-Trim, acht feste EQ-Slots, Channelmode, Breite und Mono-Bass;
> - minimumphasige Float-/Double-Kerne entsprechend deklarierter Capability, keine lineare Phase,
>   kein Lookahead und 0 gemeldete Samples Latenz;
> - `DspState → DspProgram → DspBankPool` mit vier festen Ownership-Bänken und atomischem
>   Blockrandtausch;
> - lokale APVTS-/Parameterbedienung, State-Hash, Revisions-/Undo-Ring und Neutralisieren;
> - AutomationOverlay gemäß Hostbridge oder Blockrampen-Fallback;
> - lokale Dry/Processed-A/B-Matrix und drei Analysetaps ohne Candidate-Leak in Baseline;
> - Offline-Render, Smart-Disable-Resume, Oversize-Chunking und Denormal-/NaN-Schutz.
>
> **Exit-Gate:** Filter-, Null-, State-, Automation-, Realtime-/Offline- und Worst-Case-CPU-Goldens
> aus Abschnitt 44/49 bestehen. Ein ungültiger oder neuerer State lädt neutral. Es existiert noch
> kein Remote-Apply-Endpunkt und keine Control-Capability.
>
> **Rollback:** Active-Bundle bleibt unveröffentlicht; P0–P5 und der passive Kern bleiben
> unverändert nutzbar.

### 1.11 Entwurf §65, Tickettabelle, Zeile `SONDE-015` (`:4487`)

> | `SONDE-015` | lokaler Active-DSP, Bankpool, State/Automation und A/B | P6-Null-/Filter-/Recall-/CPU-Gates grün |
### 1.12 Errata (d), (n), (o), (p), (q) — die für dieses Ticket tragenden Sätze

**(d) Probeeq ist ein EQ** (`:107-122`), Entscheid 21.08.2026:

> „die active Probe fester Name : Nakama Probeeq ist ein vollwertiger hochwertiger EQ der mit Nakama
> kommuniziert. er kann von nakama direkt anweisungen umsetzen aber auch ganz
> normal manuell vom user benutzt werden"

und der Befund desselben Erratums:

> Befund: §30.1 definiert den **ersten** aktiven Kern (ohne
> lineare Phase, Sättigung, Limiting); ob „vollwertig, hochwertig" später mehr
> verlangt, ist nicht entschieden.

**(n)** (`:291-303`):

> - **Parameter- und Zustands-Hoheit bleibt in der Sonde** (Host-Automation
>   ist instanzgebunden; §53.8-State unverändert); Gen ist Fernbedienung und
>   Ansicht. Der eigene Revisions-/Undo-Ring (§44) wird wichtiger, weil der
>   Host Ferneingriffe nicht zuverlässig in sein Undo nimmt — Befund, keine
>   neue Regel.
> - **§0.1 „Gen berät nur" gilt fortan als „nichts Ungefragtes":** Gen erhält
>   einen zuschaltbaren, minimalphasigen Master-EQ; aus = bitidentisch
>   (NullTest-Standard, dasselbe verriegelte-und-bewiesene Muster wie die
>   Hör-Markierung).

**(o)** (`:305-328`) betrifft die Drei-Stufen-Geste und den flüchtigen
Zehn-Sekunden-Kandidaten. Sie ist **Gens Hauptbedienweg** und liegt in S29–31
beziehungsweise S31b; für dieses Ticket bindet daraus nur der Schlusssatz:

> Erst der zweite Klick erzeugt `apply_transaction`, eine bestätigte
> `state_revision`, Host-/Plugin-State und den eigenen Undo-Eintrag. Damit
> bleiben §33.5 (Draft/Preview flüchtig) und §44.3 (nur bestätigte Revisionen
> persistent) erhalten.

**(p) Probeeq hat keinen lokalen Voll-Editor** (`:330-338`), Entscheid 27.08.2026:

> Auf die Frage, ob Probeeq künftig auch ohne Gen lokal als vollständiger EQ
> bedienbar sein soll, antwortet der User „nein". Damit überschreibt dieser
> Entscheid den manuellen Bedienanteil aus Erratum (d), nicht aber Probeeqs
> Bus-DSP: Probeeq misst passiv und führt bei zugeschaltetem EQ Gens
> Fernsteuerung lokal auf dem Bus aus.

**(q)** (`:340-363`), zweiter Punkt, für den Wertebereich bindend:

> **§44.2 behält seinen ±3-dB-Satz — er ist Verlauf, nicht Vorgabe.** […]
> Der Errata-Block hat Vorrang; Drafts nutzen die manuellen Bereiche (±12 dB, Q 0,15–24).
> Register: NAK-108.

### 1.13 Abnahmen, wörtlich

**Interview 23.08.2026** (`design/abnahmen/2026-08-23-interview-struktur.md`).
Schutz-Zone (`:128-135`):

> Erklärt wurde: Schutz-Zone = unantastbarer Frequenzbereich; technisch
> existieren bereits „User-Schutz" als Ablehnungsgrund im Fernweg und die
> Guardrail „geschützte Bereiche" im Advisor-Urteil.
>
> - Wirkung: Wahl **„Harte Sperre für alle"** — auch eigene Bänder lassen
>   sich nicht in die Zone ziehen, bis sie gelöst wird.
> - Anlegen: Wahl **„Ich + Gen darf vorschlagen"** — Gen darf aus einem
>   Befund eine Zone vorschlagen, angelegt wird sie erst durch User-Klick.

Mix (`:150-155`, `:204-205`, `:229-231`):

> „BEZÜGLICH DEM mixer knob der ist für die ganze app. jeder sonde
> braucht die funktion für sich den dry wet zu stellen. das heißt der
> knob bekommt klick funktion um von global auf nur für das gewählte band
> umzuschalten."

> 8. **Mix (Dry/Wet je Sonde) ist beschlossen** — „und natürlich mix also
>    wieviel sie auf den mix anwendet des eq"

> 18. **Der Mix-Knob ist zweistufig:** er gilt der ganzen App UND jede
>     Sonde braucht ihren eigenen Dry/Wet — ein Klick auf den Knob schaltet
>     zwischen global und der gewählten Spur um.

Folgen (`:252-263`):

> - Mix wird eine neue versionierte Parameter-ID (Technik, NAK-64 Punkt 4).
> - „Lösen" der Schutz-Zone = entfernen/deaktivieren über dasselbe Element,
>   mit dem sie angelegt wurde.
> - Ehrliche Grenze der harten Sperre: sie bindet Nakamas Bedienwege
>   (Gen-Maus, Advisor, Fernweg) — FLs Host-Automation schreibt an der
>   Sonden-Instanz vorbei und „gewinnt sichtbar" (Entwurf); die Zone zeigt
>   eine solche Verletzung, verhindern kann sie sie dort nicht.
> - Der globale Mix wirkt als zweite Schicht über dem Sonden-Mix

**Auto-Gain 24.08.2026** (`design/abnahmen/2026-08-24-auto-gain-schalter.md`),
gewählte Option (`:22-25`) und Folgen (`:53-55`, `:60-70`):

> „Schalter, Standard aus (Empfehlung) — Ein kleiner AUTO-Schalter neben
> OUTPUT. Aus = alles bleibt wie heute, du drehst selbst. An = Nakama
> gleicht aus. Passt zum Grundgesetz ‚nichts Ungefragtes' und zum Vorbild
> Pro-Q. Du entscheidest pro Situation."

> | Standard | **Aus.** Ein frisch eingefügtes Nakama verhält sich exakt wie heute; nichts bewegt sich ungefragt (Grundgesetz). |
> | Wirkung an | Nakama rechnet den Pegelgewinn der aktuellen Kurve und zieht ihn am Ausgang ab. Der angezeigte Output-Wert wandert dabei sichtbar mit — ein unsichtbar wirkender Ausgleich wäre ein stiller Eingriff. |
> | Verhältnis zum Handregler | Auto-Gain **ersetzt** den Output-Trim nicht. Der bleibt, was er ist; Auto-Gain legt seinen Ausgleich darauf. |

> Offen bleibt eine reine Technikfrage (meine, nicht die des Users): ob der
> Ausgleich als eigener Parameter mitläuft oder aus der Kurve abgeleitet
> wird.

**U15 und U18, 30.08.2026** (`design/abnahmen/2026-08-30-fragenrunde-marktstandard.md`,
`:120-128` und `:147-157`):

> **Antwort (Auswahl):** „Nein, dauerhaft ohne" — Nakama bleibt bewusst
> minimalphasig.
>
> **Folge:** Kein zweiter Betriebsmodus — nicht in der ersten Fassung und nicht
> als späterer Ausbau. Es wird dafür auch kein Name reserviert; die Tür bleibt
> nicht angelehnt. […] die
> an FL gemeldete Verzögerung bleibt null, der gespeicherte Zustand bekommt
> keinen Phasenmodus-Wert, die Oberfläche keinen Umschalter.

> **Antwort (Auswahl):** „Beides (Empfohlen)"
>
> **Folge:** […] Layout v2 bekommt ein eigenes
> Preset-Objekt. Eine Voreinstellung ist **nicht** der Projektzustand — sie lebt
> außerhalb des Projekts, wird ohne Projekt geladen und bekommt deshalb eigene
> Kennungen und eine eigene Version, die nie mit der Zustandsversion vermischt
> wird. Sie trägt den Klanginhalt: Bänder, Bypass, Mix, Schutz-Zonen, Auto-Gain.
> Sie trägt **keine** Identität: Zieladresse, Instanzkennung, Bus-Name und das
> selbst vergebene Label bleiben draußen

**Dynamisches Band, 01.09.2026**
(`design/abnahmen/2026-09-01-dynamisches-band-schwingt-mit.md`, `:5-6` und `:22-29`):

> „habe eine lücke im plan gefunden. bei einschalten des dynamischen eq muss
> das band dynmaisch mitschwingen wie man es von anderen eq kennt"

> 3. **Datenweg:** Die Dynamik rechnet in der Sonde (S26–28, `SONDE-015`). Der
>    laufende Wert je Band (momentane dynamische Verstärkungsänderung) muss
>    deshalb mit Anzeigekadenz von der Sonde zu Gen reisen — ein
>    Telemetriefeld im Featureframe.

**Fünferblock 01, 01.09.2026**
(`design/abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-01.md`,
`:32-35` und `:113-127`):

> Technisch bleiben `dynamic_enabled` und die fünf Werteparameter getrennte
> persistente Parameter. Ein späteres Ausschalten darf `dynamic_range_db`,
> `threshold_db`, `attack_ms`, `hold_ms` und `release_ms` daher nicht still auf
> Defaults zurücksetzen.

> - Die Bewegung ergänzt den persistenten Aktivbeleg, ersetzt ihn aber nicht:
>   Unterhalb des Thresholds oder bei `dynamic_range_db = 0` kann ein korrekt
>   aktives Band zeitweise stillstehen.
> - Es gibt keine dekorative Endlosschwingung. Die native Bewegung muss aus
>   einer autoritativen, bandbezogenen Live-Auslenkung stammen.

**Fünferblock 02, 01.09.2026**
(`design/abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-02.md`,
`:65-72`, `:117-121`, `:136-150`):

> - Eine Auswahl ändert ausschließlich `channel_mode` desselben festen
>   Band-Slots. […]
> - Beim Wechsel zu einem anderen Band zeigt der Control dessen eigenen
>   Kanalmodus. Ein Modus wird weder vom vorherigen Band übernommen noch mit
>   Filtertyp oder Dynamic-Zustand gekoppelt.

> - Erneute Aktivierung verwendet denselben Slot, dieselben Werte und dieselbe
>   Band-ID. Ausschalten gibt keinen der acht Plätze frei.
> - Nur ein ausdrücklich separates Entfernen darf den Bandpunkt aus der
>   Arbeitsfläche nehmen. `enabled = false` und „Slot frei" sind damit zwei
>   verschiedene Zustände.

> - Entfernen ist eine atomare UI-Transaktion: Der belegte Slot wird frei, das
>   Panel schließt und der Graph zeigt die entfernte Band-ID nicht mehr. Andere
>   Slot-IDs werden niemals umnummeriert.
> - Direkt nach dem Entfernen steht ein eindeutiger Undo-Rückweg bereit. Undo
>   stellt Slot-ID, Typ, Channelmode, Dynamic-Zustand und sämtliche Werte als ein
>   Objekt wieder her.

**Arbeitsmodus 31.08.2026**
(`design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md`,
`:204-205`, `:229-231`, `:250-251`):

> acht Band-Slots, wählt ihn aus und öffnet dasselbe Mini-Panel. Sind alle acht
> Slots belegt, meldet die Oberfläche das am Graphen und überschreibt nichts.

> - `Remove Band` ist eine getrennte, wiederherstellbare Aktion. Nur sie gibt
>   einen der acht Slots frei; weder Bypass noch Panel-Schließen tun das.

> Abweichungen beim Namen; aufgeklappt liegen Input, Output mit Auto-Gain,
> Width, Mono-Bass, Bypass, Mix, A/B, Voreinstellungen und Verlauf.

**Gen als EQ-Zentrale 23.08.2026**
(`design/abnahmen/2026-08-23-gen-eq-zentrale.md`, `:30`):

> vom User gewählt): Suna und Probeeq werden EINE Sonde, EQ per Klick in Gen
> zuschaltbar; aus „beobachten" wird „korrigieren" ohne Plugin-Tausch im
> Mixer; EQ aus = beweisbar bitidentisch (NullTest-Standard).

### 1.14 Blueprint §3.5 (`:360-399`) und §15 Punkte 6, 10, 15, 16 (`:1425-1474`)

§3.5, technischer Lieferumfang und Bedienvertrag, wörtlich:

> - acht feste EQ-Slots, Hard-Bypass, Input-/Output-Trim, Channelmode, Width,
>   Mono-Bass und Mix;
> - lokaler dynamischer Bandmodus mit dynamic_enabled, Range, Threshold,
>   Attack, Hold und Release; priority_sidechain bleibt bis P8
>   capability-gated;
> - minimalphasiger Float-Kern ohne Lookahead und mit 0 gemeldeten Samples
>   Latenz. Double wird nur bei deklarierter Capability und eigenen Goldens
>   aktiv; FL meldet diese Capability derzeit unsupported;
> - vorallokierte DSP-Bänke, State, Automation, Revision, Undo und Neutralize;
> - lokale A/B-Matrix, Schutz-Zonen und Auto-Gain standardmäßig aus;
> - eigenes versioniertes Presetobjekt;
> - derselbe Kern mit eigenem State und Parameterlayout auf Gen-Master.

> - **[U]** Probeeq bekommt keinen lokalen Voll-Editor.
> - **[A]** Vor P7 darf nur ein Test-/Hostzugriff existieren, der keine
>   endgültige Produktoberfläche vortäuscht.
> - **[U]** Linear Phase existiert nicht: kein deaktivierter Schalter, kein
>   späterer Teaser und keine Namensreservierung.
> - Auto-Gain ist standardmäßig aus. Aktivität und tatsächlich mitwandernder
>   Ausgangswert sind gemeinsam sichtbar.
> - Presets tragen Klanginhalt, nie instance_id, Zieladresse, Busname oder Label.
> - Ein unbekannter oder ungültiger neuerer State lädt den Audiopfad neutral,
>   erklärt den Zustand und bewahrt die Originalbytes verlustfrei read-only für
>   Rückweg oder spätere Migration; er wird nicht halb angewandt.

§15, die vier Punkte:

> 6. **[O] P6-Testzugriff:** Der technische EQ-Kern braucht Beweise, aber keinen
>    reaktivierten Probeeq-Voll-Editor.

> 10. **[O] Vertragliche Nutzlasten:** state_report.dsp,
>     command_ack.applied_dsp, EQ-Betriebszustand,
>     probe_descriptor.host_bus_name/host_mixer_index und Contribution sind
>     derzeit reserviert, nicht fertig nutzbar.

> 15. **[O] Live-Dynamic-Auslenkung:** Aktivierung, Ausschalten, Rückweg,
>     Schließen und der persistente Konturmarker sind entschieden. Der aktuelle
>     v3-Vertrag reserviert `Frame.band_dynamic_gain_db` für S26–28, liefert aber
>     noch keine Feld-ID oder Runtime-Nutzlast. Vor nativer Bewegung von
>     Bandpunkt und Kurvenzug muss der Probeeq-Featureframe-Weg beziehungsweise
>     der lokale Master-Weg gebaut und gegen stale sowie nicht-endliche Werte
>     abgesichert werden.

> 16. **[O] Band-Occupancy und Remove-Undo:** `enabled` ist im aktuellen
>     109-Parametervertrag nur der reversible Band-Bypass. Für die abgenommene
>     getrennte Remove-Aktion fehlen ein persistenter Belegungszustand, die
>     atomare Slotfreigabe und die Einordnung in die direkte Edit-/Undo-
>     Transaktion. Vor nativer Anzeige müssen Save/Load, Host-Automation,
>     Migration und Wiederherstellung unter derselben festen Band-ID gemeinsam
>     versioniert und getestet werden.

### 1.15 Register `docs/offene-punkte.md`, die vier Zeilen

**Zeile P6** (`:100`):

> | **P6 · S26–28 / S28b** | NAK-110, NAK-81, NAK-105 | `state_report.dsp`, Gain-In/Out und der fehlende Zustandsort für Gens Master-EQ |

**NAK-110** (`:121`), Kern und Entscheid:

> **[Planarbeit · S26–28]** **Der DSP-Zustandsrückweg fehlt im v3-Vertrag: `state_report` trägt keine DSP-Nutzlast, und ein erfolgreiches `command_ack` bestätigt keine angewandten Werte.** […] **Folge:** nach einem Project-Reload oder einem Reconnect kann Main den autoritativen Probeeq-Zustand nicht rekonstruieren — ein Hash sagt nur, DASS etwas anders ist, er kann Bänder, Bypass, Mix und Schutzbereiche weder anzeigen noch konfliktfrei wiederherstellen. […] **Entscheid des Dirigenten 30.08.2026 (Technik):** der Feldname `state_report.dsp` und der Name für die Bestätigung der tatsächlich angewandten Werte werden in der v3-Vertragsrunde von S16–17 reserviert; die Nutzlast entsteht mit dem DSP-DTO in S26–28, die Bestätigung in S29–31.

**NAK-81** (`:128`), die drei Punkte (a), (b), (c). Für dieses Ticket binden
(a) und (b); (c) gehört S28b:

> **Offen bleibt nur noch das Bauen:** neue globale Parameter-ID, versionierte Layout-v2-Erweiterung im selben Änderungssatz wie der Mix/Dry-Wet-Nachtrag aus NAK-64 Punkt 4 (S26–28), Rechenweg in P6.

> **(b) „Default = Hard-Bypass-äquivalent" ist eine Zusage ohne Bein.** […] sie verlangt damit, dass 0 dB Trim nicht als `powf(10, 0/20)`-Multiplikation durch die Samples läuft, sondern per Unity-Kurzschluss übersprungen wird — inklusive der Rampe, die auf dem Weg zu 1.0 durch `0.99999994f` laufen kann. §44.2 deckt nur den **expliziten** Hard-Bypass […] Wer es angeht: Satz in §44.2 **und** ein NullTest-Bein „bypass=false, alles neutral ⇒ bitidentisch" in P6 — nicht nur „bypass=true".

**NAK-105** (`:122`) gehört **S28b**, nicht diesem Ticket, und steht hier nur,
damit die Grenze sichtbar ist:

> **[Planarbeit · S28b]** **Gens Master-EQ hat im abgenommenen Zustandsvertrag keinen Ort — die Kind-Matrix verbietet `plugin_kind = main` sowohl `Parameters` als auch `Dsp`.** […] Offen bleibt die **Bauarbeit** in S28b.

---

## 2. Ist-Stand, an der Quelle gemessen

Alle Angaben sind in dieser Sitzung auf `e9dbf4b9` gelesen, nicht aus Doku
übernommen. Datei und Zeile gelten zu diesem Stand.

### 2.1 Belegklassen (Definition wie `docs/beweise/SONDE-014.md` §2.1)

- **BELEGT:** Die Zusage folgt aus einer verbindlichen Quelle. Dass der heutige
  Produktpfad sie noch nicht erfüllt, macht sie nicht offen.
- **BAULÜCKE:** Die Zusage ist belegt, aber mindestens eine heute nötige
  Implementierungs- oder Testkante fehlt.
- **ENTSCHIEDEN:** Eine aus den Quellen nicht ableitbare **Technik**frage ist
  durch den Dirigentenentscheid geschlossen — hier durch **R1 bis R15** vom
  09.09.2026, wörtlich in `docs/beweise/roh/SONDE-015-auftrag.txt` und in §5
  dieses Manifests, je Regel mit meinen Feinheiten darunter. Eine weiterhin
  fehlende Implementierung bleibt **zusätzlich** als BAULÜCKE markiert.
- **OFFEN:** Mindestens ein Teil der Zusage lässt sich aus keiner Quelle
  ableiten und ist **Produkt**frage. Diese Zeile ist kein stiller
  Produktentscheid; sie wird benannt und einer Fläche zugeordnet, nie hier
  beantwortet (§5.16).

Die Klasse **ENTSCHEIDEN-VORSCHLAG** aus SONDE-013 und SONDE-014 kommt in
diesem Manifest **nicht** vor: der Dirigent hat mit R1 bis R15 vor der Matrix
entschieden, nicht danach. Wo eine seiner Regeln eine Lücke offen lässt, füllt
sie meine Feinheit unter derselben Regel — das ist Teil der Regel, kein neuer
Vorschlag. Sieben Stellen weichen begründet ab — **zwei davon sind
zurückgenommen** (Nummer 3 in der Nacharbeit 1, Nummer 6 in der Nacharbeit 3);
sie stehen gesammelt in §5.17.

Jede Matrixzeile nennt ein bestehendes Prüfbein namentlich aus
`tools/beweise.ps1` oder ein ausdrücklich mit **NEU** gekennzeichnetes. Ein
neuer Beinname ist Teil der späteren Bauaufgabe, nicht die Behauptung, das Bein
existiere.

### 2.2 (a) Probeeq heute: ein Passthrough ohne Parameter, ohne DSP, ohne Editor

- `eq-copilot/plugin/sonde/SondeProcessor.cpp:155` ist der ganze
  `processBlock`. Er kopiert Leseblöcke in die vorallokierte Analysequeue
  (`:214-217`) und schreibt an keiner Stelle in den Audiopuffer; die einzige
  Schreiboperation ist die JUCE-Sicherheitsschleife für überzählige
  Ausgangskanäle (`:222-223`), die bei gleicher Busbelegung null Mal läuft.
  **Es gibt keine Filterbank, keinen Trim, keine M/S-Matrix, keinen Bypass.**
- `eq-copilot/plugin/sonde/SondeProcessor.h:134` meldet `hasEditor()` = false,
  `:149` `getTailLengthSeconds()` = 0.0, `:151` genau ein Programm. Ein Aufruf
  von `setLatencySamples` existiert in der ganzen Datei nicht — die gemeldete
  Latenz ist JUCEs Default 0.
- **Kein `AudioProcessorValueTreeState` im ganzen Baum:**
  `grep -rn "AudioProcessorValueTreeState" eq-copilot/plugin/` liefert null
  Treffer. Probeeq meldet dem Host heute **null** Hostparameter
  (`SondeProcessor.h:40-45`: „KEINE HOSTPARAMETER … Parameter, die nichts tun,
  waeren eine Oberflaeche, die luegt").
- Es gibt **kein** `dsp/`-Verzeichnis unter `eq-copilot/plugin/`; vorhanden
  sind `core`, `hostbridge`, `hostprobe`, `probe`, `sonde`, `spike`, `src`,
  `state`, `tests`, `vertrag`.

**Folge für die Matrix:** jede Zeile in §3 ist heute eine **BAULÜCKE**. Das ist
kein Befund gegen den Code, sondern die Ausgangslage des Tickets: P6 baut den
aktiven Kern erstmals.

### 2.3 (b) Der Nulltest verlangt seine eigene Umschreibung

`eq-copilot/plugin/tests/SondeNullTestMain.cpp:9-13`, wörtlich:

> Probeeq ist laut User ein vollwertiger EQ; seine DSP kommt in P6. Bis
> dahin ist auch er ein Passthrough, und genau das misst dieser Test.
> Wenn P6 die DSP bringt, MUSS dieser Test fuer NkAc umgeschrieben werden -
> faellt er dann, ist das die richtige Reaktion und kein Aergernis.

Das Bein heißt **A16** `EqCopProbeeqNullTest` (`tools/beweise.ps1:647`,
`AbPhase='jetzt'`). Seine heutige Behauptung sagt unter anderem „heute
ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6) … kein
Hostparameter". Beide Teile werden mit diesem Ticket **falsch** und müssen im
selben Änderungssatz nachgezogen werden (Prüfliste E, „Geänderte Zusage — drei
Stellen").

### 2.4 (c) Parametervertrag v1: 109 IDs, eingefroren, mit der Regel für v2

- `eq-copilot/schemas/state/nakama-parameter-v1.json:20` führt
  `"anzahl_parameter": 109`, `:18` `"dsp_schema_version": 1`, `:19`
  `"slot_anzahl": 8`.
- Global sind es genau fünf IDs (`:23-27`): `v1.global.bypass` (bool, Default
  false, blockrand), `v1.global.input_trim_db` und `v1.global.output_trim_db`
  (±24 dB, Default 0, rampe), `v1.global.width` (0,0–2,0, Default 1,0, rampe)
  und `v1.global.mono_bass_hz` (0–500 Hz, Default 0, rampe). **Weder Mix noch
  Auto-Gain noch ein EQ-Rollenschalter existieren.**
- Je Slot 13 Namen (`:31-43`). `enabled` ist dort ausdrücklich „Band aktiv" —
  ein reversibler Bypass; **`occupied` gibt es nicht**.
- Die Regel für dieses Ticket steht in `:8`, wörtlich:
  „IDs werden nie umbenannt oder wiederverwendet (§53.8). Ein neuer Parameter
  bekommt eine neue ID und ein neues Layout-Praefix (v2.\*), nie einen freien
  Platz."
- `:13` nennt den Laufzeitdeckel: „Zur Laufzeit deckelt der DSP-Validator
  freq_hz zusaetzlich auf 0,45·fs (§44.2); der Vertrag hier nennt den
  statischen Bereich."
- `:15` trägt die Zusage, die NAK-81 (b) als „ohne Bein" führt: „Freie Slots
  bleiben neutral: die Defaults unten ergeben zusammen einen
  Hard-Bypass-aequivalenten Zustand (enabled=false, gain 0, dynamic aus)."
- C++-Spiegel: `eq-copilot/plugin/state/NakamaParameter.h:28-34` mit
  `kSlots = 8`, `kGlobal = 5`, `kJeSlot = 13`, `kAnzahl = 109`,
  `kDspSchemaVersion = 1` und dem `static_assert` in `:33`.
- Die Werkzeuge über dem Vertrag stehen schon:
  `nakama::parameter::validiere`, `ausDtoText`, `dtoKanon`, `stateHash`,
  `schreibeInBaum` und `leseAusBaum` (`NakamaParameter.h:72-94`). **Der
  RFC-8785-Kanon und der SHA-256-Hash sind gebaut und dreisprachig gemessen**
  (Bein A12, B2, `cargo test`); dieses Ticket erweitert sie, schreibt sie
  nicht neu.

### 2.5 (d) State-Schema 2: das Kind `Dsp` ist ein reservierter Name, kein Inhalt

- `eq-copilot/schemas/state/nakama-state-v2.md:33`, wörtlich:
  „`Dsp` NAME RESERVIERT — Inhalt definiert SONDE-015 (Revision, Schutz,
  bestätigter DspState, Undo-Ring)".
- Die Kind-Matrix (`:41-46`) sagt für `active_probe` in der Spalte `Dsp`:
  „ab SONDE-015 (heute: nicht lesbar ⇒ read-only)".
- Der Code setzt das durch: `eq-copilot/plugin/state/NakamaState.cpp:1059`
  lehnt jeden Stand mit einem `Dsp`-Kind ab —
  `grund = "child Dsp is not readable by this version (SONDE-015)"` — und der
  Aufrufer macht daraus `nurLesen(...)` (`:1635-1645`, `:1686-1697`).
  **Ein v2-Stand mit `Dsp` ist heute read-only mit Originalbytes.** Genau
  dieser Zweig muss mit diesem Ticket die Seite wechseln.
- `:48` regelt, was additiv bleibt: „Eine **unbekannte Eigenschaft** in einem
  bekannten Kind desselben Majors ⇒ additiv, wird erhalten." Ein **Kind** mit
  unbekanntem Namen dagegen ⇒ read-only.
- Die Byte- und Knotengrenzen des Lesers (`:188-191`) sind für den Undo-Ring
  bindend: höchstens 16 MiB, **höchstens 64 `ValueTree`-Knoten inklusive
  Wurzel**, höchstens 65.536 Einträge je Sammlung, 262.144 im ganzen Baum,
  dazu die Headroom-Prüfung `hatWriterHeadroom`. Deshalb führt `MainProject`
  seine Listen als **flache Arrays** (`:24-29`) und nicht als Kindknoten.
- `:214` zählt auf, was heute **nicht** im Schema liegt: „Undo-Ring,
  `state_revision`, bestätigter DspState, Schutzgrenzen (→ `Dsp`,
  SONDE-015)".

### 2.6 (e) DSP-DTO und `state_hash` heute

`eq-copilot/schemas/state/nakama-state-v2.md:170-176`:

- DTO-Form heute: `{"dsp_schema_version":1,"parameters":{<id>:<Wert>,…}}` —
  genau die 109 IDs, physikalische Werte.
- Validierungsreihenfolge fest, erster Fehler zählt: Textstufe doppelter
  Schlüssel → Typ/Struktur → `dsp_schema_version` → unbekannter Schlüssel →
  fehlender Schlüssel → Typ je Parameter → nichtendlich → Bereich/Enum.
- Kanon RFC 8785, `state_hash` = SHA-256-Hex (64 Kleinbuchstaben).
- Drei Beine gegen dieselben Fixtures: C++ (`NakamaKanon`), Python
  (`rfc8785`), Rust (`serde_json_canonicalizer` + `sha2`).
- Der Kanon liest JSON **selbst** (`kanon::lies`), nicht über `juce::JSON`,
  weil JUCEs Zahlenleser subnormale Werte auf 0 flusht.

Der Fixture-Korpus liegt unter `eq-copilot/fixtures/state/` mit `MANIFEST.json`,
`jcs/`, `dto/` und `schema2/`. **`eq-copilot/fixtures/state/dto/ungueltig/dsp-schema-version-2.json`
ist heute ein Negativfixture** — es misst, dass Version 2 abgelehnt wird. Mit
R1 wechselt es die Seite. *(Verlauf, Stand `e9dbf4b9`: die Datei ist mit der
Etappe 2 entfallen; ihren Negativplatz nimmt seither
`dto/ungueltig/dsp-schema-version-1.json` ein, §8.1.)*

### 2.7 (f) v3-Vertrag: `state_report` ohne DSP, zwei reservierte Namen

- `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2300-2323` führt `state_report`
  mit `additionalProperties: false` und genau `type`, `adresse`,
  `dsp_schema_version`, `state_revision`, `state_hash`, `record_state`
  (Pflicht) sowie `undo_tiefe` (optional, `minimum: 0`, `maximum: 32`).
  **`dsp` und `eq_enabled` fehlen.**
- `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json:215-241` führt fünf
  reservierte **Felder**, darunter `Frame.band_dynamic_gain_db`
  (Eigentümer „S26-28 (SONDE-015)"), `state_report.dsp` (Eigentümer „S26-28"),
  `command_ack.applied_dsp` (Eigentümer „S26-28/S29-31") und
  `state_report.eq_enabled` (Eigentümer „S29-31").
- `:242` trägt die Feldregel wörtlich: „Dieselbe Regel wie fuer die
  Nachrichtennamen, eine Ebene tiefer: der Vertragsanteil, der spaeter bricht,
  wenn man ihn heute offen laesst, ist der NAME - nicht die Nutzlast."
- Der Wire-Envelope-Minor der Familie P1 steht auf **4** (`:6-10`,
  `"vorher": 3, "aktuell": 4`), Fassungen 0 bis 3 sind belegt; Fassung 3 ist
  SONDE-014.
- Das Prüfwerkzeug verriegelt beides:
  `tools/eq-copilot/pruefe_v3_vertrag.py:1609-1623` prüft, dass jedes
  reservierte Feld genau die Schlüssel `name`, `eigentuemer`, `grund` trägt
  **und dass die Menge der reservierten Feldnamen exakt jene fünf ist**;
  `:1694-1704` prüft, dass `state_report` weder `dsp` noch `eq_enabled` und
  `command_ack` kein `applied_dsp` besitzt. Beide Riegel müssen in Etappe 2
  mitwandern, sonst wird der Vertrag grün gegen sich selbst.
- Der Rust-Leser liegt in `broker/src/coordinator/liveness.rs:549-585`. Er
  liest heute `adresse`, `state_revision`, `state_hash` und beide
  `record_state`-Felder und schreibt sie in `client`; **`undo_tiefe` liest er
  nicht**. Der Aufrufweg ist `broker/src/coordinator/senke.rs:100-102`, ein
  P1-Zweig, dessen Ergebnis bewusst verworfen wird (ein abgelehnter P1
  schließt keine Verbindung, er wird gezählt).
- Der C++-Sender liegt in
  `eq-copilot/plugin/core/ipc/controlclient/Intern.h:437-447`
  (`stateReportJson`). Er baut heute genau die sechs Pflichtfelder und sendet
  **kein** `undo_tiefe`. Getaktet wird er in
  `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:758`, `:858-867`:
  ein P1-Versand im 1-Hz-Takt, **nur wenn sich der Text geändert hat**
  (`if (report != letzterStateReport)`).
- Die Nutzlast speist sich aus `ControlStatus`
  (`eq-copilot/plugin/core/ipc/ControlClient.h:149-158`) mit
  `dspSchemaVersion`, `stateRevision`, `stateHash`, `recordStateValid` und
  `recording`. Der Kommentar `:145-148` sagt: „Ein leerer `stateHash` bedeutet
  vertragsgemaess JSON-null; der Client erfindet weder einen DSP-Stand noch
  einen Aufnahmezustand."

### 2.8 (g) FlatBuffers-Featureframe: höchste Feld-ID ist 21

- `eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs:239-319`,
  `table Frame`: die Felder tragen die IDs 0 bis 21, zuletzt
  `headroom:Headroomverteilung (id: 21)`. **Die nächste freie ID ist 22.**
- `eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json:83-108` spiegelt genau
  diese Liste, mit `"verbrannt": []`.
- Der C++-Encoder steht in
  `eq-copilot/plugin/core/ipc/TelemetryClient.cpp:154-173` und übergibt die
  Argumente **in Feld-ID-Folge** an `fb::CreateFrame`; ein neues Feld hängt am
  Ende an.
- Der Schema-Minor der FlatBuffers-Fläche P2 ist eine eigene Konstante:
  `eq-copilot/plugin/core/ipc/TelemetryClient.h:43`,
  `inline constexpr std::uint8_t kFeatureBatchSchemaMinor = 1;` mit dem
  Kommentar „Der echte P2-Sendepfad benutzt diese Konstante; Tests duerfen den
  Wert nicht am Envelope vorbei selbst einsetzen."
- Der C++-Leser ist `eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:432`
  (`pruefeFrame`), der Rust-Leser ist `broker/src/telemetrie.rs:403` (`fuer_broker`).
- Der Drift-Riegel ist Bein **A9** `pruefe_flatc_drift.py`
  (`tools/beweise.ps1:530`): „Codegen-Drift ist 0 … jedes Tabellenfeld traegt
  eine explizite Feld-ID." Der Fixture-Erzeuger ist **A10**
  `erzeuge_fb_fixtures.py` (`:531`).

### 2.9 (h) Filter-Vorbilder und die analytische Referenz existieren schon

- `eq-copilot/plugin/core/analysis/KGewichtung.h:39-53` trägt den
  Direct-Form-II-transponierten `Biquad` in `double` mit dem Kommentar:
  „bewusst dieselbe Anordnung der Rechenschritte, weil Gleitkommaaddition
  nicht assoziativ ist: eine andere Klammerung waere eine andere Zahl, und die
  Bitgleichheit des GoldenTests haengt daran."
- `:63-110` enthält zwei RBJ-Entwürfe (`rbjHighShelf`, Highpass) „RBJ Audio EQ
  Cookbook, wie `pyloudnorm` sie verwendet — ausdruecklich NICHT die
  DeMan-Variante."
- `eq-copilot/plugin/src/HoerMarkierung.h:45-138` ist das zweite Vorbild.
- **Die analytische Filterreferenz ist gebaut:**
  `eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:745-753`,
  `biquadLeistung` rechnet `|H(e^{jw})|²` direkt aus den Koeffizienten. Sie ist
  der Vergleichsmaßstab — **nicht** der Messweg (§3.2, M-13).
- Der Hostkontext- und Automationspfad steht seit S3/SONDE-003: Bein **B3**
  `EqCopHostContextTest` (`tools/beweise.ps1`) misst „Anwesenheit,
  Parameterpunkte, Buslatenz … Quellhash-Gate des JUCE-Patches gruen". Ob die
  Sonde samplegenaue Automationspunkte bekommt, ist damit eine **gemessene
  Capability**, keine Annahme dieses Manifests.

### 2.10 (i) Kanon heute: B6 und B7 sind Platzhalter, nicht scharf

- `tools/beweise.ps1:679`:
  `Kuerzel='B6'; Name='EqCopDspGoldenTest'; AbPhase='P6'; Behauptung='Aktiver
  DSP-Kern liefert die eingefrorene Referenzantwort.'`
- `tools/beweise.ps1:680`:
  `Kuerzel='B7'; Name='EqCopTransactionTest'; AbPhase='P6'; Behauptung='Apply/Revert
  ist transaktional - kein halber Zustand ueberlebt.'`
- Die Phasenmechanik steht in `tools/beweise.ps1:1093-1107`: fehlt das Binär,
  wird ein Bein mit `AbPhase='jetzt'` als `[FEHLT]` gezählt und der Lauf
  scheitert, jedes andere als `[GEPLANT]` gemeldet. **„Scharf" heißt also
  genau zweierlei: `AbPhase` auf `'jetzt'` UND ein gebautes Ziel.** Beides
  gehört in denselben Änderungssatz, sonst ist der Kanon entweder blind oder
  rot.
- Der letzte grüne Kanon meldet „2 geplante Prüfung(en) noch nicht gebaut" —
  das sind genau B6 und B7 (`docs/beweise/NAK-230.md`, Zeile „Kanon nachher").
  Die Kanonzahl steigt mit diesem Ticket von **62** auf **64**, wenn kein Bein
  verschwindet.
- Die Beine, die nachziehen müssen, mit ihrem heutigen Ort:
  **A5** `pruefe_v3_vertrag.py --abdeckung` (`:520`), **A8**
  `erzeuge_v3_fixtures.py --pruefen` (`:523`), **A9** `pruefe_flatc_drift.py`
  (`:530`), **A10** `erzeuge_fb_fixtures.py --pruefen` (`:531`), **A11**
  `pruefe_v2_schemas.py` (`:538`), **A12** `erzeuge_state_fixtures.py
  --pruefen` (`:546`), **A16** `EqCopProbeeqNullTest` (`:647`), **B2**
  `EqCopStateMigrationTest` (`:652`), **B3c** `EqCopSchemaTest` (`:661`),
  **A4** `cargo test` des Brokers.

### 2.11 (j) Wo die Zusagen aus §1 heute nicht gehalten werden — die Baulücke in einem Satz

**Nichts davon existiert:** kein Filter, keine Bank, kein Bypass, kein Trim,
keine M/S-Matrix, kein Mix, kein Auto-Gain, keine Zone, kein `occupied`, keine
Transaktion, keine Revision, kein Undo-Ring, kein Preset, kein `Dsp`-Kind,
keine `state_report.dsp`-Nutzlast, kein `band_dynamic_gain_db`, kein APVTS.
Vorhanden sind: der Passthrough (§2.2), der Parametervertrag v1 samt DTO,
Kanon und Hash (§2.4, §2.6), der reservierte Kindname (§2.5), die zwei
reservierten Feldnamen (§2.7), die Biquad-Vorbilder und die analytische
Referenz (§2.9) sowie zwei Platzhalterbeine (§2.10).

### 2.12 Begriffe, die diese Matrix getrennt hält

| Begriff | Nicht dasselbe wie | Warum die Trennung trägt |
|---|---|---|
| `v2.global.eq_enabled` | `v1.global.bypass` | Der Rollenschalter entscheidet, **ob** dieser Bus überhaupt ein EQ ist (Grundgesetz „nichts Ungefragtes"); der Hard-Bypass ist der neutrale Zustand **innerhalb** des eingeschalteten EQ (§44.2). Beide sind bitidentisch, aber nur der erste hält die Bänke kalt. |
| `enabled` | `occupied` | `enabled=false` ist der reversible Band-Bypass mit erhaltenen Werten; `occupied=false` ist der freie Slot. Fünferblock 02: „zwei verschiedene Zustände". |
| `state_revision` | `automation_epoch` | Die Revision zählt bestätigte Basiszustände; die Epoche markiert Hostgesten und erzeugt nie eine Revision (§44.3). |
| `state_hash` | `dsp.jcs` | Seit MN1 (B-07) sind beide **dasselbe Material**: `dsp.jcs` ist genau der RFC-8785-Text, über den der Hash gebildet wurde, und jeder Leser rechnet `SHA-256(dsp.jcs)` nach (M-105). Der Unterschied ist nur noch die Form: der Hash sind 64 Hexzeichen, die Nutzlast der Text selbst. Eine Teilmenge des DTO auf dem Draht gibt es **nicht** mehr — die frühere Fassung ist mit Abweichung 3 in §5.17 zurückgenommen. |
| abgeleiteter Auto-Gain-Wert | `v1.global.output_trim_db` | Der Ausgleich ist kein persistenter Parameter und nicht im Hash; der Output-Trim bleibt, was er ist (Abnahme 24.08.: „ersetzt den Output-Trim nicht"). |
| `dsp_schema_version` | `preset_schema_version` | Projektzustand und Voreinstellung sind zwei Versionsleitern; U18 verlangt ausdrücklich, dass sie nie vermischt werden. |
| Wire-Envelope-Minor P1 | `kFeatureBatchSchemaMinor` P2 | Der eine versioniert JSON-Nachrichten, der andere die FlatBuffers-Fläche. Dieses Ticket hebt **beide**, aber getrennt und aus getrennten Gründen. |
| dynamische Verstärkung | `dynamic_range_db` | Der eine Wert ist gemessen und läuft, der andere ist die eingestellte Grenze. Fünferblock 01: „niemals aus den Einstellwerten erfunden". |
| Probe-Pipe | Produktions-Pipe | Bestandsregel aus `CLAUDE.md`; für dieses Ticket relevant, sobald ein Ende-zu-Ende-Lauf gegen den Broker fährt. |

---

## 3. Verhaltensmatrix

Die Spalte **Rotbeweis** nennt, woran die Prüfung fallen **muss**, wenn die
Zusage verletzt ist. Ein Rotbeweis, der an einem Nebeneffekt fällt — ein Test,
der das Modul direkt ruft statt durch den Audiopfad zu gehen; ein Vergleich der
Koeffizienten mit der Formel, aus der sie entstanden sind; ein Bitvergleich, der
schon an der Rundungsphase scheitert —, ist **ungültig**. Für dieses Ticket ist
das kein Formalismus: die teuerste Klasse hier sind Zusagen über den
Audiopfad, und ein Filter beweist sich nur, indem Audio durch ihn läuft.

Wo ein bestehendes Bein genannt ist, steht sein Kürzel aus `tools/beweise.ps1`;
neue Beine tragen **NEU** und existieren am Basis-SHA nicht.

**Alle Zeilen sind heute BAULÜCKE** (§2.11). Die Spalte Belegklasse nennt
deshalb zusätzlich, woher die Zusage kommt: **BELEGT** aus einer verbindlichen
Quelle, **ENTSCHIEDEN (Rn)** aus dem Dirigentenentscheid samt der Feinheit in
§5, **OFFEN** für die wenigen Produktfragen, die hier benannt und zugeordnet,
aber nicht beantwortet werden.

### 3.0 Die Signalkette, auf die sich jede Zeile bezieht

Die Reihenfolge ist selbst eine Zusage (R3: „die Position von Mix gegenüber
Output-Trim und Auto-Gain legt die Matrix fest und der Golden friert sie ein").
Sie lautet:

```text
Eingang
  └─ Tap pre_nakama                (Dry-Quelle, vor jedem eigenen DSP)
     └─ eq_enabled == false ────────────────────────────► Ausgang (nichts geschrieben)
     └─ eq_enabled == true
        └─ v1.global.bypass == true ───────────────────► Ausgang (nichts geschrieben)
        └─ v1.global.bypass == false
           ├─ Input-Trim              (Rampe, Unity-Kurzschluss bei 0 dB)
           ├─ M/S-Hinmatrix           (Width, Mono-Bass, normalisierte Energie)
           ├─ 8 Band-Slots            (statisch + dynamisch, je Kanalzuordnung)
           ├─ M/S-Rückmatrix
           ├─ Auto-Gain               (abgeleitet, Rampe wie Output-Trim)
           ├─ Mix                     (Crossfade gegen den Tap pre_nakama)
           └─ Output-Trim             (Rampe, Unity-Kurzschluss bei 0 dB)
              └─ Tap post_committed
                 └─ Hörmatrix         (Dry / Processed / Delta / Candidate)
                    └─ Ausgang
```

Der Candidate-Pfad ist derselbe Aufbau auf einer zweiten Bank; sein Tap heißt
`post_candidate` und wird nie Baseline (§44.2). Die Hörmatrix liegt
**hinter** allen drei Taps (§44.2 letzter Absatz).

### 3.1 Rollenschalter, Passthrough und Hard-Bypass (Gate: R2, §44.2, §49.2 Gate 1, Grundgesetz)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-01 | Frische Instanz, nichts eingestellt | `v2.global.eq_enabled` steht auf **false**. Der Audiopfad ist exakt der heutige: `processBlock` schreibt keinen einzigen Sample in den Eingangsbereich des Puffers, meldet 0 Samples Latenz, hält keine Sperre und bereitet **keine** Bank vor. Über 1000 Blöcke wechselnder Größe bei 44,1 / 48 / 96 / 192 kHz ist der Ausgang bitgleich zum Eingang. | **BELEGT** · **ENTSCHIEDEN (R2)** · BAULÜCKE | **A16** umgeschrieben, Fall `default_ist_bitidentisch_ueber_1000_bloecke` | Ein einziger Sample unterscheidet sich; oder der Bankpool meldet nach dem Lauf eine belegte Bank | Grundgesetz `CLAUDE.md`; §49.2 Gate 1; R2 |
| M-02 | `eq_enabled` **true**, `bypass` false, alle 120 Werte sonst auf Default | Auch **ohne** Hard-Bypass ist der engagierte, aber neutrale Kern bitidentisch: 0 dB Trim läuft **nicht** als Multiplikation durch die Samples, sondern über einen Unity-Kurzschluss — einschliesslich der Rampe, die auf dem Weg zu 1,0 durch `0.99999994f` laufen kann. Dasselbe gilt für `width` = 1,0, `mono_bass_hz` = 0, `mix` = 1,0 und `auto_gain` = false: der vollständige Weg aus §3.0 ist durchlaufen und schreibt trotzdem keinen Sample. **MN1 (B-08):** die Zeile lief bis zur Matrixprüfung 1 mit `eq_enabled` = false und endete damit vor Trims, M/S und Filterbank — sie wäre auch ohne jeden Kurzschluss grün geblieben. Der Fall mit `eq_enabled` = false ist M-01. | **BELEGT** (NAK-81 b) · BAULÜCKE · **MN1** | **A16**, Fall `eq_an_bypass_aus_alles_neutral_ist_bitidentisch`; **NEU** **B6**, Fall `unity_kurzschluss_statt_multiplikation` | Der Ausgang weicht um ein LSB ab, weil `x * powf(10, 0/20)` gerechnet wurde; oder die Rampe endet auf `0.99999994f` statt exakt `1.0f`; oder der Test läuft mit ausgeschaltetem EQ und bleibt deshalb auch ohne die Kurzschlüsse grün | NAK-81 (b) wörtlich (§1.15); `nakama-parameter-v1.json:15`; Matrixprüfung 1 B-08 |
| M-03 | `eq_enabled` false → true bei laufendem Audio | Der Wechsel ist **klickfrei** und topologisch: die Übernahme geschieht am Blockrand, davor liegt ein Crossfade fester Länge (`kFadeSamples`, §5.2). Reihenfolge: Programm außerhalb des Audiothreads bauen → Bank publizieren → Blockrand → Crossfade beginnen → nach `kFadeSamples` ist der Wet-Pfad allein aktiv. | **ENTSCHIEDEN (R2)** · BAULÜCKE | **NEU** **B6**, Fall `engagieren_ist_klickfrei_und_endet_deterministisch` | Ein Sprung größer als die Fadeschrittweite tritt am Umschaltsample auf; oder der Fade dauert bei Blockgröße 1 anders lang als bei 2048 | R2; §44.2 „kurzem Crossfade" |
| M-04 | `eq_enabled` true → false bei laufendem Audio | Spiegelbildlich: Crossfade zurück, und **nach** dem Fade ist der Passthrough wieder **bitidentisch** — nicht „numerisch nahe". Der Beweis misst getrennt: die Fadesamples liegen innerhalb der Toleranz, die Samples danach sind bitgleich. | **ENTSCHIEDEN (R2)** · BAULÜCKE | **A16**, Fall `nach_dem_fade_wieder_bitidentisch`; **NEU** **B6**, Fall `ausschalten_ist_klickfrei` | Nach dem Fade bleibt ein Restfaktor, sodass der Ausgang nur „fast" gleich ist; oder der Fade endet nie, weil sein Zähler an einer Blockgrenze zurückgesetzt wird | R2; §49.2 Gate 1 |
| M-05 | `eq_enabled` true, `v1.global.bypass` true | Der Hard-Bypass liegt **vor** M/S-Matrix und Filterbank. Bei In-place-Verarbeitung schreibt er keine Samples; das Ergebnis ist bitgenau sampleidentisch. Ein rechnerischer Identity-Filter genügt ausdrücklich **nicht**. | **BELEGT** · BAULÜCKE | **A16**, Fall `eq_an_bypass_an_ist_bitidentisch`; **NEU** **B6**, Fall `hard_bypass_schreibt_nicht` | Der Bypass wird als Kette neutraler Biquads gerechnet und liefert ein LSB Abweichung; oder er liegt hinter der M/S-Matrix, sodass Width ungleich 1 durchschlägt | §44.2 „Ein neutraler Active-State besitzt einen expliziten Hard-Bypass vor M/S-Matrix und Filterbank" |
| M-06 | `bypass` false → true bei laufendem Audio | Auch der Hard-Bypass wechselt am **Blockrand mit Crossfade** (`wechsel: blockrand` im Vertrag), nicht abrupt. Nach dem Fade gilt M-05 bitgenau. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `bypasswechsel_ist_klickfrei_und_endet_bitgleich` | Ein Knacks am Umschaltsample; oder der Bypass wird sofort wirksam und die letzte Bank bleibt in `audio_active` hängen | `nakama-parameter-v1.json:23` (`wechsel = blockrand`); §44.2 |
| M-07 | `eq_enabled` false, Filterzustände einer früheren aktiven Phase | Der ausgeschaltete Pfad **rechnet nicht weiter**: keine Bank ist `audio_active`, kein Filterzustand wird fortgeschrieben, kein Detektor läuft. Beim erneuten Einschalten starten alle Filter- und Hüllkurvenzustände auf 0 — die Sonde beginnt neu, sie setzt nicht fort. | **ENTSCHIEDEN (R2)** · BAULÜCKE | **NEU** **B6**, Fall `ausgeschaltet_rechnet_nichts_und_startet_kalt` | Nach aus/ein klingt ein alter Filterzustand hörbar aus; oder ein CPU-Zähler zeigt Bandarbeit im ausgeschalteten Zustand | R2 „keine Bankaktivität"; Grundgesetz „nichts Ungefragtes" |
| M-08 | `eq_enabled` in einem gespeicherten Projekt auf true, Projekt wird geladen | Der Schalter ist **persistent und Host-Parameter**: er überlebt Speichern und Laden bitgleich und liegt in der APVTS. Reihenfolge beim Laden: State lesen → validieren → Committed setzen → Hostparameter synchronisieren → erst beim ersten Audioblock die Bank aktivieren (§44.3 „Der nächste `prepareToPlay`/Audioblock übernimmt den bestätigten Zustand **vor** seiner Ausgabe"). | **ENTSCHIEDEN (R2)** · BAULÜCKE | **B2** erweitert, Fall `eq_enabled_ueberlebt_speichern_laden`; **NEU** **B7**, Fall `bank_wird_erst_beim_ersten_block_aktiv` | Ein geladenes Projekt startet mit `eq_enabled=false`, obwohl der State true trägt; oder die Bank wird schon beim Laden aktiv und der erste Block läuft halb | R2; §44.3 |
| M-09 | Der Host fährt Smart Disable oder es läuft kein Callback | Ohne Callback bleibt ein bestätigter Zustand **gespeichert und wartend**; er geht nicht verloren und wird nicht als verworfen dargestellt. Der nächste Block übernimmt ihn vor seiner Ausgabe. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `ohne_callback_bleibt_gespeichert_und_wird_nachgeholt` | Ein Apply ohne laufenden Callback meldet Fehler oder wird beim Wiederanlauf übersprungen | §44.3 „Läuft kein Callback, bleibt Main sichtbar bei gespeichert · wird bei Audio fortgesetzt aktiv"; §60 „Smart-Disable-Resume" |

### 3.2 Filterbank: Typen, Bereiche, Nyquist (Gate: §30.1, §44.2, §44.5, Erratum (q))

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-10 | Ein Slot ist belegt und eingeschaltet, Typ `bell` | Die gemessene Betragsantwort des **Audiopfads** liegt typisch innerhalb ±0,05 dB und an den Extrempunkten innerhalb ±0,1 dB an der analytischen RBJ-Antwort. Gemessen wird an einem festen Gitter über 20 Hz bis min(20 kHz, 0,45·fs) bei 44,1 / 48 / 96 / 192 kHz. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `bell_gegen_analytische_antwort` | Eine Gitterstelle weicht um mehr als 0,05 dB ab; oder die Extrempunkte reißen die 0,1 dB | §44.5 „Filtergoldens bleiben typisch innerhalb ±0,05 dB"; §44.2 „minimumphasige RBJ-Biquads als Referenz" |
| M-11 | Dasselbe für `low_shelf`, `high_shelf`, `notch`, `low_cut`, `high_cut` | Jeder der sechs Vertragstypen wird **einzeln** gegen die analytische Antwort gemessen, nicht stellvertretend durch einen Nachbarn. Die Enumliste ist geschlossen; ein siebter Wert wird abgewiesen, nicht auf `bell` abgebildet. | **BELEGT** · BAULÜCKE | **NEU** **B6**, fünf weitere Fälle je Typ; **A12** und **B2** für die Enumgrenze | Ein Typ fehlt im Goldenkorpus; oder ein unbekannter Enumwert wird still zu `bell` | `nakama-parameter-v1.json:32`; §30.1 |
| M-12 | Slot mit `freq_hz` = 19000 Hz, Samplerate 44,1 kHz | Zur Laufzeit wird die Frequenz auf **0,45·fs** gedeckelt (hier 19845 Hz — der Wert bleibt), bei 20000 Hz greift der Deckel. Die Kappung geschieht beim **Bauen des Programms**, nicht im Audiothread, und der gekappte Wert wird als **geklemmt** gemeldet, nicht still angewandt. Der **persistente** Wert bleibt unverändert: ein bei 96 kHz gesetztes 20-kHz-Band verliert seine Einstellung nicht, wenn das Projekt bei 44,1 kHz geöffnet wird. | **BELEGT** · **ENTSCHIEDEN (R6-Feinheit, §5.6)** · BAULÜCKE | **NEU** **B6**, Fälle `nyquist_kappung_bei_045_fs` und `kappung_aendert_den_persistenten_wert_nicht` | Der gekappte Wert wird zurück in den State geschrieben und geht beim Samplerate-Wechsel verloren; oder die Kappung fehlt und der Biquad wird instabil | `nakama-parameter-v1.json:13`; §44.2 |
| M-13 | Filtergolden wird gefahren | Der Golden misst den **Audiopfad**, nicht die Koeffizientenformel: je Gitterstelle läuft ein eingeschwungener Sinus durch den echten `DspProgram`, und der gemessene Pegelabstand wird gegen eine im Test **eigenständig** ausgeschriebene RBJ-Formel gehalten. Ein Vergleich der Produktkoeffizienten mit derselben Produktformel gilt ausdrücklich nicht als Beweis. | **ENTSCHIEDEN (R15-Feinheit, §5.15)** · BAULÜCKE | **NEU** **B6**, Aufbau des Goldens selbst | Der Golden ruft `entwurf()` des Produkts und vergleicht mit `biquadLeistung()` derselben Koeffizienten — dann bleibt er grün, auch wenn der Audiopfad die Bank gar nicht anwendet | `tools/dirigent/pruefliste.md` E; Vorbild `eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:745-753` |
| M-14 | Ein Wert liegt außerhalb seines Vertragsbereichs, etwa `gain_db` = 18 | Der Validator lehnt ihn ab, **bevor** ein Programm gebaut wird; die Reihenfolge der Prüfstufen ist die des DTO-Vertrags (Typ → nichtendlich → Bereich/Enum). Bindend sind ±12 dB und Q 0,15–24; der ±3-dB-Satz aus §44.2 ist Verlauf. | **BELEGT** · BAULÜCKE | **A12**, **B2** (Bereichsfixtures); **NEU** **B7**, Fall `apply_mit_bereichsverletzung_wird_abgewiesen` | Ein Wert über ±12 dB wird geklemmt statt abgelehnt; oder der Remote-Verlaufssatz ±3 dB wird als aktive Grenze gebaut | `nakama-parameter-v1.json:35`; Erratum (n)/(q), Register NAK-108 |
| M-15 | Zwei Slots sind belegt und eingeschaltet | Die Bänder wirken **als Kaskade in Slotreihenfolge 0 → 7**, nicht als Parallelsumme und nicht rückwärts. **Zwei getrennte Zusagen, zwei getrennte Beweise.** *Kaskade statt Parallelsumme:* der gemessene Gesamtfrequenzgang des Audiopfads deckt sich mit dem **Produkt** der analytischen Einzelantworten (bei Parallelsumme fällt der Vergleich: zwei +6-dB-Bells ergeben kaskadiert +12 dB, parallel rund +9,5 dB im Scheitel). *Richtung 0 → 7:* der Test rechnet die nicht kommutierende Kombination — ein dynamisches Band auf Slot 2 vor einem statischen auf Slot 5 auf derselben Frequenz, dazu `left` auf Slot 1 vor `mid` auf Slot 4 — **selbst, Sample für Sample, mit denselben Filterprimitiven in der festgelegten Reihenfolge Slot 0 zuerst**, friert dieses Ergebnis als **geordnetes Golden** ein, und der Audiopfad muss es innerhalb der Goldentoleranz treffen. Die Belegungsreihenfolge ändert daran nie etwas; das ist ein **eigener** Fall und nicht der Richtungsbeweis. **MN2 (B-09):** „beide Anordnungen liefern verschiedene Ergebnisse" trennt 0 → 7 nicht von 7 → 0 — eine rückwärts laufende Bank besteht diese Probe genauso. Erst ein unabhängig gerechnetes, **geordnetes** Referenzergebnis legt die Richtung fest. | **ENTSCHIEDEN (R-Feinheit, §5.3.1)** · BAULÜCKE · **MN1** · **MN2** | **NEU** **B6**, Fälle `kaskade_gegen_produkt_der_einzelantworten`, `richtung_null_nach_sieben_gegen_geordnetes_golden` und `belegungsreihenfolge_aendert_nichts` | Der Gesamtgang deckt sich mit der Parallelsumme statt mit dem Produkt; oder **ein Rückwärtslauf-Mutant (Bank läuft 7 → 0) besteht das geordnete Golden**; oder das Golden wird aus demselben Bankdurchlauf gewonnen, den es prüfen soll, und ist damit eine Tautologie | §53.8 „acht feste Band-Slots mit stabilen Parameter-IDs"; §5.3.1; Matrixprüfung 1 B-09, Matrixprüfung 2 B-09 |
| M-16 | Koeffizienten und Filterzustände | Beide sind **64 Bit** (`double`), auch wenn der Hostpuffer 32 Bit trägt. Der Float-Callback konvertiert am Rand, nicht in der Kette. Eine 64-Bit-Capability wird **nicht** gemeldet, solange der Double-Callbackpfad nicht denselben Goldenkorpus besteht. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `koeffizienten_und_zustaende_sind_double`; **A13** unverändert (Capabilityreport) | Ein Filterzustand ist `float`; oder der Build meldet eine Double-Capability, ohne dass ein Double-Golden existiert | §44.2 „64-Bit-Koeffizienten und -Filterzustände"; Blueprint §3.5 |
| M-17 | Ein kontinuierlicher Wert wird bewegt | Er läuft über **genau eine** definierte Rampe; Zipperresiduen bleiben im Ramp-Test unter −100 dBFS. Hostautomation, lokale Änderung und (später) Fernweg durchlaufen **denselben** Validator und dieselbe Programmbaugrenze — es gibt keine zweite freie Glättung. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `zipperresiduen_unter_minus_100_dbfs` | Ein Residuum über −100 dBFS; oder zwei Wege glätten verschieden, sodass derselbe Zielwert je nach Quelle anders klingt | §44.5; §53.8 „genau eine definierte Rampe" |

### 3.3 Dynamische Bänder, Detektor und Sidechain (Gate: R7, R8, §30.1, §44.2, Fünferblock 01)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-18 | `dynamic_enabled` true, Pegel **über** Threshold | Die dynamische Verstärkung folgt der Kennlinie aus §5.7: `g_dyn = dynamic_range_db · min(1, max(0, e_db − threshold_db) / 12 dB)`, wobei `e_db` der RMS-Pegel des Detektors nach Attack/Hold/Release ist. Sie wirkt **zusätzlich** zu `gain_db` und ist durch die Konstruktion auf ±\|dynamic_range_db\| begrenzt. | **ENTSCHIEDEN (R7)** · BAULÜCKE | **NEU** **B6**, Fall `dynamische_kennlinie_golden` | Die gemessene Auslenkung überschreitet \|dynamic_range_db\|; oder die Kennlinie hängt vom Vorzeichen von `gain_db` ab | R7; §44.2 „RMS-/Peak-Hüllkurven mit expliziter Attack/Hold/Release-Smoothing" |
| M-19 | `dynamic_enabled` true, Pegel **unter** Threshold | Die dynamische Verstärkung ist **exakt 0,0** — nicht „nahe null". Das Band steht still und klingt wie sein statischer Zustand; das ist ein regulärer Betriebsfall, kein Fehler. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `unter_threshold_steht_das_band_exakt_still` | Ein Restwert ungleich 0 lässt das Band kriechen; oder der Test prüft nur „klein" statt „exakt 0" | Fünferblock 01 `:113-115`; R7 |
| M-20 | `dynamic_range_db` = 0, alles andere dynamisch | Ebenfalls **exakt 0,0** Auslenkung und keine Detektorarbeit: bei Range 0 wird der Detektor gar nicht erst gerechnet. Das Band bleibt trotzdem persistent als „dynamisch aktiv" erkennbar — der Aktivbeleg hängt nicht an der Bewegung. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `range_null_rechnet_keinen_detektor` | Der Detektor läuft und kostet CPU, obwohl er nichts bewirken kann; oder das Band verliert seinen Aktivzustand, weil es sich nicht bewegt | Fünferblock 01 `:113-115`; R7 |
| M-21 | `sidechain_source` = `internal` | Der Detektor hört das **bandgefilterte Eingangssignal des Bandes vor dem Band**: ein RBJ-Bandpass auf `freq_hz` und `q` desselben Slots, gerechnet auf der Kanalkomponente, die das Band bearbeitet. Er hört nicht das Bandausgangssignal — sonst wäre die Regelung rückgekoppelt. | **ENTSCHIEDEN (R7)** · BAULÜCKE | **NEU** **B6**, Fälle `detektor_hoert_vor_dem_band` und `detektor_ist_bandbegrenzt` | Ein Signal weit außerhalb des Bandes löst die Dynamik aus; oder die Auslenkung schwingt auf, weil der Detektor hinter dem Band hängt | R7; §30.1 „bandbezogenen dynamischen EQ-Bändern" |
| M-22 | `sidechain_source` = `none`, `dynamic_enabled` true | `none` heißt: kein Detektor, keine dynamische Auslenkung. Das Band verhält sich statisch, bleibt aber als dynamisch konfiguriert erhalten. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `sidechain_none_liefert_keine_auslenkung` | Bei `none` läuft still der interne Detektor weiter | `nakama-parameter-v1.json:43` (drei Werte, Default `none`); R7 |
| M-23 | `sidechain_source` = `priority_sidechain` | Der Wert wird **auf `internal` geklemmt** und die Klemmung gemeldet: der persistente Parameter behält `priority_sidechain`, das gebaute Programm rechnet `internal`, und der Bericht führt den Slot in der Klemmliste. Nichts wird still umgedeutet, und der Wert wird nicht auf `none` gesetzt. | **ENTSCHIEDEN (R7)** · BAULÜCKE | **NEU** **B6**, Fall `priority_sidechain_wird_geklemmt_und_gemeldet`; **NEU** **B7**, Fall `klemmliste_erscheint_im_bericht` | Der Parameter wird beim Speichern auf `internal` umgeschrieben und der Userwunsch geht verloren; oder die Klemmung ist unsichtbar und P8 findet einen stillschweigend umgedeuteten Bestand vor | R7; §44.3 „tatsächlich geklemmte Parameter"; Blueprint §3.5 „priority_sidechain bleibt bis P8 capability-gated" |
| M-24 | `dynamic_enabled` true → false → true | Ausschalten ändert **ausschließlich** `dynamic_enabled`. Die fünf Werte `dynamic_range_db`, `threshold_db`, `attack_ms`, `hold_ms`, `release_ms` bleiben **bitgleich** erhalten und stehen beim Wiedereinschalten unverändert bereit. | **BELEGT** · BAULÜCKE | **B2** erweitert, Fall `dynamicwerte_ueberleben_aus_und_ein_bitgleich`; **NEU** **B7**, derselbe Weg über eine Transaktion | Einer der fünf Werte steht nach dem Wiedereinschalten auf Default; oder das Ausschalten schreibt sie „vorsorglich" zurück | Fünferblock 01 `:32-35`; R8 |
| M-25 | `dynamic_enabled` wird gewechselt, Audio läuft | Der Wechsel ist **topologisch**: er geschieht nur am validierten Blockrand über ein vorbereitetes Crossfade zwischen zwei vollständigen Programmen, nie samplegenau mitten im Block. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `dynamic_enabled_wechselt_nur_am_blockrand` | Der Wechsel wirkt an einem Sample-Offset innerhalb des Blocks; oder er wirkt sofort ohne Crossfade | `nakama-parameter-v1.json:37` (`topologisch: true`); §53.8 |
| M-26 | Attack, Hold und Release werden gesetzt | Alle drei wirken als **getrennte** Stufen in dieser Reihenfolge: steigender Detektorwert folgt der Attack-Zeitkonstante, nach dem letzten Anstieg hält der Wert `hold_ms` lang, danach fällt er mit `release_ms`. Die Zeitkonstanten sind samplerateunabhängig in Millisekunden definiert und werden je Samplerate in Koeffizienten umgerechnet. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `attack_hold_release_sind_drei_stufen` (Sprungantwort bei vier Sampleraten) | Hold wird ignoriert und der Wert fällt sofort; oder dieselbe Millisekundenangabe ergibt bei 96 kHz eine andere Zeit als bei 48 kHz | §44.2; `nakama-parameter-v1.json:40-42` |
| M-27 | Die dynamische Auslenkung soll ausgelesen werden | Je Slot liefert der Kern die **momentane** Auslenkung in dB — der zuletzt gerechnete Wert am Ende des Analysefensters, nicht ein Mittelwert und nie aus den Einstellwerten hergeleitet. Freie, ausgeschaltete oder nicht dynamische Slots liefern exakt 0,0. | **BELEGT** · **ENTSCHIEDEN (R14-Feinheit, §5.14)** · BAULÜCKE | **NEU** **B6**, Fall `auslenkung_ist_gemessen_nicht_gerechnet` | Der gelieferte Wert entspricht `dynamic_range_db` statt der Messung; oder ein stillstehendes Band meldet Bewegung | Fünferblock 01 `:113-127`; Abnahme 01.09.2026; R14 |

### 3.4 Kanalmodus, M/S-Matrix, Trims, Mix und Auto-Gain (Gate: R3, R4, R8, §30.1, §44.2, Abnahme 24.08.)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-28 | `channel_mode` eines Slots wird gesetzt | Der Wert ist **diskret** aus genau fünf Werten (stereo / left / right / mid / side), Default `stereo`, topologisch, und **nie** mit Filtertyp oder Dynamic-Zustand gekoppelt. Ein Wechsel ändert ausschließlich `channel_mode` desselben Slots; ein anderer Slot erbt ihn nicht. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fälle `channel_mode_ist_diskret_und_ungekoppelt` und `kein_slot_erbt_den_modus`; **A12** für die Enumgrenze | Ein Typwechsel setzt `channel_mode` mit zurück; oder das Auswählen eines anderen Slots übernimmt den Modus des vorigen | Fünferblock 02 `:65-72`; R8; `nakama-parameter-v1.json:36` |
| M-29 | `channel_mode` wird bei laufendem Audio gewechselt | Topologischer Wechsel: nur am Blockrand über ein vorbereitetes Crossfade zweier vollständiger Programme, klickfrei. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `channel_mode_wechselt_am_blockrand_klickfrei` | Ein Knacks am Umschaltsample; oder der Wechsel wirkt innerhalb des Blocks | §53.8; §44.2 |
| M-30 | `width` ungleich 1,0 | Die M/S-Matrix arbeitet mit **normalisierter Energie**: bei `width` = 1,0 ist der Hin-und-Rück-Weg bitidentisch (Unity-Kurzschluss), bei 0,0 ist der Ausgang mono, bei 2,0 ist die Seite verdoppelt. `mono_bass_hz` entfernt unterhalb der Frequenz die Seite; 0 heißt aus und schaltet die Stufe ganz ab. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fälle `width_eins_ist_bitidentisch`, `width_null_ist_mono`, `mono_bass_null_ist_aus` | `width` = 1,0 liefert ein LSB Abweichung, weil die Matrix immer gerechnet wird; oder `mono_bass_hz` = 0 filtert trotzdem | §44.2 „Stereo-, L/R- und M/S-Matrix mit normalisierter Energie"; `nakama-parameter-v1.json:26-27` |
| M-31 | Input-Trim und Output-Trim auf 0 dB | Beide sind **Unity-Kurzschlüsse**, keine Multiplikation mit 1,0 (M-02). Auf anderen Werten laufen sie über dieselbe eine Rampe. Der Input-Trim liegt **vor** der Filterbank, der Output-Trim **nach** Mix (§3.0). | **BELEGT** · **ENTSCHIEDEN (R3)** · BAULÜCKE | **NEU** **B6**, Fall `trims_auf_null_db_sind_kurzschluss` und `kettenreihenfolge_golden` | 0 dB Trim erzeugt eine Abweichung; oder der Output-Trim liegt vor Mix und wird bei Mix 0 wirkungslos | NAK-81 (b); R3; `nakama-parameter-v1.json:24-25` |
| M-32 | `v2.global.mix` = 1,0 (Default) | Der Mix ist ein **Unity-Kurzschluss**: der Dry-Zweig wird nicht gerechnet, der Ausgang ist exakt der Wet-Pfad. | **ENTSCHIEDEN (R3)** · BAULÜCKE | **NEU** **B6**, Fall `mix_eins_umgeht_den_dry_zweig` | Bei Mix 1,0 wird `1·wet + 0·dry` gerechnet und ein `-0.0` oder ein NaN aus dem Dry-Zweig schlägt durch | R3; §5.3 |
| M-33 | `v2.global.mix` = 0,0, Output-Trim 0 dB | Der Ausgang ist **bitidentisch zum Eingang** — der Dry-Zweig ist der Tap `pre_nakama`, also das Signal vor Input-Trim und Filterbank, und beide neutralen Stufen sind Kurzschlüsse. Auch der Wet-Zweig wird bei Mix 0 nicht in den Ausgang gerechnet. | **ENTSCHIEDEN (R3)** · BAULÜCKE | **A16**, Fall `mix_null_ist_bitidentisch`; **NEU** **B6**, derselbe Fall am Kern | Bei Mix 0 bleibt ein Rest des Wet-Zweigs hörbar; oder der Dry-Zweig wird hinter dem Input-Trim abgegriffen und ein Trim ungleich 0 dB schlägt durch | R3 „Dry ist das Signal am Tap pre_nakama" |
| M-34 | `v2.global.mix` = 0,5 | Linearer Crossfade: `0,5 · wet + 0,5 · dry`, **nicht** equal-power. Beide Zweige sind hochkorreliert; eine Wurzelkennlinie erzeugte in der Mitte bis zu +3 dB Überhöhung. Der Wert läuft über dieselbe Rampe wie die anderen kontinuierlichen Parameter. | **ENTSCHIEDEN (R3-Feinheit, §5.3)** · BAULÜCKE | **NEU** **B6**, Fall `mix_ist_linear_nicht_equal_power` | Bei Mix 0,5 mit identischem Dry und Wet ist der Ausgang um 3 dB lauter als beide | R3; §5.3 |
| M-35 | `v2.global.auto_gain` = false (Default) | Der angewandte Ausgleich ist **exakt 0 dB** und wird als Kurzschluss übersprungen. Der **abgeleitete** Wert wird trotzdem gerechnet und ist lesbar — damit die Oberfläche später zeigen kann, was Auto-Gain täte, ohne dass etwas geschieht. | **ENTSCHIEDEN (R4-Feinheit, §5.4)** · BAULÜCKE | **NEU** **B6**, Fall `auto_gain_aus_wirkt_nicht_und_ist_trotzdem_lesbar` | Der Ausgleich wirkt trotz ausgeschaltetem Schalter; oder der abgeleitete Wert ist nur bei eingeschaltetem Schalter lesbar | R4; Abnahme 24.08. `:53`; Grundgesetz |
| M-36 | `auto_gain` = true, Kurve flach (kein Band belegt oder alle neutral) | Der abgeleitete Wert ist **exakt 0,0 dB**, und der Ausgang bleibt bitidentisch zum ausgeschalteten Fall. | **ENTSCHIEDEN (R4)** · BAULÜCKE | **NEU** **B6**, Fall `flache_kurve_ergibt_exakt_null_db` | Der Wert ist −0,0001 dB, weil über ein Gitter gemittelt statt kurzgeschlossen wurde, und der Ausgang ist nicht mehr bitgleich | R4 „flache Kurve → exakt 0 dB" |
| M-37 | `auto_gain` = true, ein High-Shelf mit +6 dB überstreicht das ganze Gitter | Der abgeleitete Wert liegt bei **−6 dB ±0,1 dB**. Gerechnet wird der pink-gewichtete Energiemittelwert der statischen Betragsantwort über ein logarithmisches Gitter von 20 Hz bis 20 kHz in 1/12-Oktav-Schritten (121 Stellen, gleiche Gewichtung je Oktave); **Mid und Side gehen als getrennte Kanalkomponenten** mit dem Diagonalterm `(H_M + H_S)/2` ein, Formel vollständig in §5.4. Der Referenzkorpus trägt ausdrücklich das Paar **zwei identische +6-dB-Shelves auf `mid` und `side`** und erwartet **−6 dB ±0,1 dB**, weil ein identisches Mid/Side-Paar exakt auf den Stereo-Fall zusammenfällt. **MN1 (B-04):** die vorige Faltung multiplizierte beide Filter in `H_L` und `H_R` und hätte dasselbe Paar wie +12 dB kompensiert. | **ENTSCHIEDEN (R4, B-04-Regel)** · BAULÜCKE · **MN1** | **NEU** **B6**, Fälle `shelf_plus_sechs_ergibt_minus_sechs`, `identisches_mid_side_paar_fällt_auf_stereo_zusammen` und der Referenzkurvenkorpus | Der Wert weicht um mehr als 0,1 dB ab; oder **das identische Mid/Side-Paar ergibt −12 dB statt −6 dB**; oder der Wert kippt bei einem schmalen tiefen Notch ins Absurde, weil in dB statt in Energie gemittelt wurde | R4; Abnahme 24.08. `:54`; Matrixprüfung 1 B-04 |
| M-38 | `auto_gain` = true, ein dynamisches Band bewegt sich | Der **dynamische** Anteil geht in die Ableitung **nicht** ein: der Ausgleich folgt nur der statischen Kurve des bestätigten Programms und wandert nicht mit dem Programmmaterial. Sonst wäre Auto-Gain ein Telemetriepfad, der Audio steuert (§49.2 Gate 5). | **BELEGT** · **ENTSCHIEDEN (R4)** · BAULÜCKE | **NEU** **B6**, Fall `dynamik_bewegt_den_auto_gain_nicht` | Der Ausgleichswert ändert sich, während nur der Pegel des Eingangs schwankt | R4; §49.2 Gate 5; NAK-81 (a) „nicht a dynamic process based on actually measured levels" |
| M-39 | Ein Programm wird vorbereitet | Der Ausgleich wird **außerhalb des Audiothreads** beim Bauen des Programms gerechnet und im Audiothread über **denselben Rampenweg wie der Output-Trim** angewandt. Er ist **kein** persistenter Parameter und **nicht** Teil des `state_hash`. | **ENTSCHIEDEN (R4)** · BAULÜCKE | **NEU** **B6**, Fall `auto_gain_wird_nicht_im_audiothread_gerechnet`; **NEU** **B7**, Fall `auto_gain_ist_nicht_im_hash` | Ein Logarithmus läuft im Callback; oder zwei Stände mit gleichem DTO, aber verschiedenem abgeleitetem Wert liefern verschiedene Hashes | R4; §53.8 „state_hash umfasst ausschließlich das validierte DSP-DTO" |
| M-40 | Auto-Gain ist an, der User dreht am Output-Trim | Auto-Gain **ersetzt** den Output-Trim nicht: beide wirken, der Ausgleich liegt auf dem Handregler. Die Reihenfolge ist Auto-Gain (im Wet-Zweig, vor Mix), danach Output-Trim (hinter Mix). | **BELEGT** · **ENTSCHIEDEN (R3, R4)** · BAULÜCKE | **NEU** **B6**, Fall `auto_gain_und_output_trim_addieren_sich` | Der Handregler wird bei aktivem Auto-Gain ignoriert oder überschrieben | Abnahme 24.08. `:55`; R3; R4 |

### 3.5 Bankpool, Audiothread, Nicht-Endlichkeit und Latenz (Gate: R9, §44.2, §44.5, §49.2 Gate 2)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-41 | Der Pool wird angelegt | **Vier** Bänke, alle vorallokiert, je mit Double-Buffer für Committed und Candidate. Nach `prepareToPlay` findet keine weitere Allokation statt — auch nicht beim ersten Programmwechsel. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `vier_baenke_vorallokiert_und_null_allokationen_danach` | Der Allokationszähler steigt beim ersten Programmwechsel | §44.2; §60 „vier festen Ownership-Bänken" |
| M-42 | Ein Programm wird vorbereitet | Der Ownership-Automat läuft genau die Kette `free → preparing → ready(generation) → audio_active/fading → retired → free`. Der Control-Worker schreibt **ausschließlich** `free`-Slots und veröffentlicht Index plus Generation mit Release-Semantik; der Audiothread übernimmt nur am Blockrand. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `ownership_automat_je_uebergang` (jeder der sechs Übergänge einzeln) | Ein Übergang ist im Automaten möglich, den der Vertrag nicht nennt; oder der Worker schreibt einen Slot, der nicht `free` ist | §44.2 wörtlich |
| M-43 | Der Audiothread hat eine Bank ausgedient | Er meldet die ausgediente Generation über einen **vorallokierten Audio→Control-SPSC-Ring** zurück. **Erst nach diesem ACK** darf der Worker Filterzustand oder Koeffizienten überschreiben. Es gibt kein In-place-Überschreiben, keinen Heap-Reclaim, keinen `shared_ptr`-Destruktor und keine Deallokation im Callback. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `reclaim_erst_nach_ack` | Der Worker überschreibt eine Bank, die der Audiothread noch liest; oder ein Destruktor läuft im Callback | §44.2 wörtlich |
| M-44 | Kein Slot ist `free`, ein **bankpflichtiges** Programm soll gebaut werden | Der Befehl erhält **`busy_retry`** — er wird nicht gepuffert, nicht verworfen und nicht mit Gewalt durchgesetzt. **Der Aufrufer wiederholt mit derselben Transaktions-ID:** `busy_retry` ist §5.11.4 **T5** — kein Commit, keine Revision, **nicht memoisiert**. Weil `r` dabei unverändert bleibt, ist die alte `base_revision` weiterhin gültig, und der nächste Versuch durchläuft die Stufen erneut; wann er committet, sagt §5.11.4 **T7**. Eine neue ID braucht der Aufrufer **nicht**. Ein **bankfreier** Zustand (`eq_enabled = false`; Begriff in §5.11.4 Teil 2) bekommt dagegen kein `busy_retry`: er committet auch bei vollem Pool, wenn die übrigen Stufen bestehen (§5.11.4 T6 und T9), und wird nach E-18 als ENDE-Marke ohne Bank publiziert (§5.11.4 S8). **MK:** die Zeile zitiert die Tabelle und rechnet nicht selbst. **MR:** bis zur NAK-245-Runde hieß es hier „die Wiederholung kommt durch“ — ohne die Bedingung, unter der T7 committet (Matrixprüfung 5). **MR1:** bis zur Nacharbeit 1 der NAK-245-Runde nannte das Ereignis den vollen Pool ohne die Bankpflicht und sagte damit auch einem bankfreien Zustand `busy_retry` zu, obwohl der gebaute Kern ihn ohne Reservierung publiziert (Matrixprüfung 6). | **BELEGT** · **ENTSCHIEDEN (§5.11.4 Teil 2, T5, T6, T7, T9)** · BAULÜCKE · **MN3** · **MK** · **MR** · **MR1** | **NEU** **B6**, Fall `busy_retry_wenn_kein_slot_frei`; **NEU** **B7**, Fälle `busy_retry_wird_nicht_gemerkt` (T5), `wiederholung_nach_bankfreigabe_committet` (T7) und `retry_nach_zonenloesung_bei_vollem_pool_ohne_bankpflicht_committet` (T9, bankfrei bei vollem Pool) | Ein fünfter **bankpflichtiger** Programmwunsch verdrängt eine aktive Bank; oder er wird still verworfen und die Transaktion meldet Erfolg; oder die Wiederholung nach der Freigabe bekommt erneut `busy_retry` aus dem Register statt gebaut zu werden; oder **ein bankfreier Zustand bekommt bei vollem Pool `busy_retry` statt eines Commits** | §44.2 wörtlich; R9; §5.11.4 Teil 2, T5, T6, T7 und T9; E-18 (§9.2), `eq-copilot/plugin/dsp/DspKern.cpp:170-177`; Matrixprüfung 3 D-2; Matrixprüfung 5; Matrixprüfung 6 |
| M-45 | Der ACK-Ring läuft trotz Überkapazität über | Der Ring **fasst mehr Einträge als es Slots gibt und droppt nie**. Ein dennoch erkannter Overflow setzt zusätzlich eine atomare `reclaim_pending_mask`; der betroffene Slot bleibt **dauerhaft nicht frei**, bis der Worker ihn bestätigt. Reclaim-Sicherheit gewinnt über Verfügbarkeit. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `reclaim_pending_mask_haelt_den_slot` | Nach einem erzwungenen Overflow wird der Slot wieder `free`, ohne dass der Worker ihn bestätigt hat | §44.2 wörtlich |
| M-46 | Gleichzeitiger Topologiewechsel auf Committed und Candidate | Der Fall braucht für die begrenzte Fadezeit drei, im schlechtesten Fall vier Bänke. Reicht das Budget nicht, wird **Candidate vor dem Wechsel neutral beendet** — nie eine Bank und nie ein Filterzustand zwischen beiden Pfaden geteilt. | **BELEGT** · BAULÜCKE · **MA** (§11) | **NEU** **B6**, Fall `vier_baenke_im_schlimmsten_fall_und_candidate_endet_neutral` | Committed und Candidate teilen sich eine Bank; oder der Wechsel scheitert statt Candidate zu beenden | §44.2 wörtlich |
| M-47 | Ein voller Audioblock läuft | Im Callback gibt es **keine Sperre, keine Allokation, keinen Datei-, Pipe- oder Netzzugriff, kein Logging und keine geworfene Ausnahme**. Gemessen wird mit einem thread-lokalen Allokations- und einem Sperrzähler über mindestens 4000 Blöcke wechselnder Größe mit Transportkanten und Programmwechseln. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `null_allokationen_null_sperren_im_callback`; **B4** unverändert als Gegenprobe | Der Zähler steht auf 1; oder der Zähler ist global statt thread-lokal und kann Worker und Audiothread nicht trennen | §44.5; §49.2 Gate 2; `CLAUDE.md` |
| M-48 | Überlast: die Analyse kommt nicht nach | Verworfen werden **Analyseframes, nie Audio**. Der Audiopfad läuft unverändert weiter, und die Verwerfung wird gezählt. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `ueberlast_verwirft_analyse_nicht_audio`; **B4** Bestandsweg | Ein Block Audio fällt aus, weil die Analysequeue voll war | `CLAUDE.md` Grundgesetz; §44.2 |
| M-49 | Ein nicht-endlicher Eingangssample erreicht den **aktiven** Pfad | Er wird **vor jedem Filterzustand** durch 0,0 ersetzt und gezählt; der Zähler ist lockfrei lesbar. Zusätzlich prüft der Kern am **Blockrand** die Filterzustände: ist einer nicht endlich, wird der Zustand dieses Bandes genullt und ein zweiter Zähler erhöht. Damit bleiben die Filterzustände endlich, ohne dass pro Sample die ganze Bank geprüft wird. | **ENTSCHIEDEN (R9-Feinheit, §5.9)** · BAULÜCKE | **NEU** **B6**, Fälle `nichtendlicher_eingang_wird_verriegelt_und_gezaehlt` und `nichtendlicher_filterzustand_wird_am_blockrand_geheilt` | Ein NaN vergiftet den Filterzustand dauerhaft und alle folgenden Blöcke sind still; oder der Zähler bleibt 0 | R9; `CLAUDE.md` „NaN-Ehrlichkeit" |
| M-50 | Ein nicht-endlicher Eingangssample erreicht den **Passthrough** (`eq_enabled` false oder Hard-Bypass) | Der Passthrough **sanitisiert nichts**: derselbe nicht-endliche Wert kommt unverändert heraus, und kein Zähler steigt. Ein Passthrough, der Werte repariert, wäre eine Verarbeitung, die niemand eingeschaltet hat. | **ENTSCHIEDEN (R9)** · BAULÜCKE | **A16**, Fall `passthrough_sanitisiert_nichts`; **NEU** **B6**, Gegenprobe zu M-49 | Ein NaN wird im ausgeschalteten Zustand zu 0 — der Nulltest ist dann keine Bitgleichheit mehr, sondern eine Behauptung | R9 wörtlich; Grundgesetz |
| M-51 | Der Host fragt Latenz und Tail | Die gemeldete Latenz bleibt **konstant 0 Samples** — `setLatencySamples` wird nie mit einem Wert ungleich 0 gerufen —, `getTailLengthSeconds()` bleibt 0,0, und es gibt kein Lookahead und keine lineare Phase. Der **Tailbeweis** gilt für den Passthrough- und Bypasszustand; ein eingeschalteter resonanter Filter klingt naturgemäß aus, und die Runner-Behauptung sagt genau das und nicht mehr. | **BELEGT** · **ENTSCHIEDEN (R9-Feinheit, §5.9)** · BAULÜCKE | **A16**, Fälle `latenz_bleibt_null` und `kein_tail_im_passthrough` | Ein Aufruf von `setLatencySamples` mit ungleich 0 taucht auf; oder die Runner-Behauptung sagt „kein Tail", während der Test den eingeschalteten EQ gar nicht misst | §49.3 „0 Samples, konstant"; §60; `tools/dirigent/pruefliste.md` E |
| M-122 | Control-Worker und Audiothread laufen **echt parallel** am neuen Bankpool | Über viele tausend Übergänge — Publikation, Blockrandübernahme, Fade, ACK, Reclaim, Slot-Wiederverwendung — hält der Pool seine Invarianten **je Block** geprüft: nie zwei Schreiber auf einer Bank, nie ein Reclaim vor dem ACK, Generationen streng monoton, kein Slot gleichzeitig `audio_active` und `preparing`. Entwurf §44.5 verlangt „ThreadSanitizer- beziehungsweise **äquivalente** Stressläufe": trägt die MSVC-Toolchain dieses Projekts keinen ThreadSanitizer, **ist** der äquivalente Stresslauf der Nachweis, und die Zeile sagt das ausdrücklich statt ein Werkzeug zu behaupten, das es hier nicht gibt. **MN1 (B-10):** M-42 bis M-47 messen einzelne Übergänge und Zähler; Rennenfreiheit zwischen zwei Threads folgt daraus nicht. Das genannte Bestandsbein **B4** fährt den `EqCopilotProcessor`, nicht den Sonden-Pool (`eq-copilot/plugin/tests/QueueStressTestMain.cpp:9-24`), und kann diesen Nachweis deshalb nicht tragen. | **BELEGT** (§44.5) · **ENTSCHIEDEN (B-10-Regel)** · BAULÜCKE · **MN1** | **NEU** **B6**, Fall `bankpool_parallel_ohne_rennen` (zwei echte Threads, Invarianten je Block, Generationenfolge protokolliert) | Ein absichtlich **vor** das ACK gezogenes Reclaim bleibt unentdeckt; oder der Lauf ist sequentiell und der Rotbeweis fällt deshalb nie; oder zwei Schreiber greifen auf dieselbe Bank zu, ohne dass eine Invariante bricht | Entwurf §44.5 `:3092-3093`; `CLAUDE.md` Audiothread-Invarianten; Matrixprüfung 1 B-10 |

### 3.6 Hörmatrix und die drei Taps (Gate: R10, §30.1, §44.2, §49.3)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-52 | Ein Projekt wird geladen | Die Hörmatrix steht **immer auf Processed**. Die Auswahl ist **transient**: kein persistenter Parameter, kein Host-Parameter, kein Feld im DTO, nichts im `state_hash`. Ein gespeichertes Projekt kann nicht in Delta oder Candidate aufwachen. | **ENTSCHIEDEN (R10)** · BAULÜCKE | **B2** erweitert, Fall `hoermatrix_ist_nicht_persistent`; **NEU** **B7**, Fall `nach_laden_immer_processed` | Ein geladenes Projekt startet in Delta; oder die Auswahl taucht im DTO auf und verändert den Hash | R10; Grundgesetz „nichts Ungefragtes" |
| M-53 | Auswahl `Dry` bei eingeschaltetem EQ | Der Ausgang ist der Tap `pre_nakama`, also **bitidentisch zum Eingang** — dieselbe Zusage wie M-33, hier über die Hörmatrix statt über Mix. | **ENTSCHIEDEN (R10)** · BAULÜCKE | **NEU** **B6**, Fall `hoermatrix_dry_ist_bitidentisch` | Dry führt durch den Input-Trim und ist nur „nahe" am Eingang | R10; §30.1 |
| M-54 | Auswahl `Delta` | `Delta = Processed − Dry`, danach ein **fester** Lautheitsabgleich (Konstante, §5.10) — fest heißt: er folgt **nicht** dem Material. Sind Processed und Dry identisch, ist das Delta **exakt 0** in jedem Sample, unabhängig vom Abgleich. | **ENTSCHIEDEN (R10-Feinheit, §5.10)** · BAULÜCKE | **NEU** **B6**, Fälle `delta_ist_differenz_mit_festem_abgleich` und `delta_bei_gleichheit_ist_exakt_null` | Der Abgleich wird aus dem laufenden Pegel gerechnet und ist damit ein Kompressor; oder das Delta bei gleichem Signal ist nicht exakt 0 | R10; §30.1 „festem Lautheitsabgleich" |
| M-55 | Wechsel zwischen zwei Hörmatrix-Zuständen bei laufendem Audio | **Kein Klick**; und ein A/B-Wechsel zwischen zwei **identischen** Zuständen nullt innerhalb numerischer Toleranz. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `hoermatrixwechsel_ist_klickfrei_und_nullt_bei_gleichheit` | Ein Sprung am Umschaltsample; oder identische Zustände liefern hörbare Differenz | §49.3 Zeile „A/B-Zustandswechsel" |
| M-56 | Auswahl `Candidate`, aber es liegt kein Candidate-Programm vor | Die Auswahl fällt sichtbar auf **Processed** zurück und meldet das über ihren Getter; sie tut nicht so, als spielte sie einen Kandidaten. Ein Element, das einen Zustand vortäuscht, den es nicht gibt, ist ein totes Element. | **ENTSCHIEDEN (R10-Feinheit, §5.10)** · BAULÜCKE | **NEU** **B6**, Fall `candidate_ohne_kandidat_faellt_sichtbar_zurueck` | Die Auswahl bleibt auf Candidate stehen und liefert stumm den Committed-Pfad | `CLAUDE.md` „Keine toten UI-Elemente"; R10 |
| M-57 | Die drei Taps liefern Blöcke an die bestehende Analyse | `pre_nakama`, `post_committed` und `post_candidate` liefern **kohärente** Blöcke desselben Zeitfensters. Candidate-Frames werden **nie** Baseline; die Session-Landkarte bezieht sich auf `post_committed`. Der Hörmatrix-Ausgang ist **kein** Messpunkt: er liegt hinter allen drei Taps. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fälle `taps_sind_kohaerent` und `hoermatrix_liegt_hinter_den_taps`; **B5** unverändert | Ein Delta-Hörzustand verändert die gemessenen Bandwerte; oder ein Candidate-Frame landet in der Baseline | §44.2 letzter Absatz; §49.2 Gate 5; `CLAUDE.md` „Engine kennt keine Optik" |

### 3.7 Band-Belegung, Remove und Undo (Gate: R5, Fünferblock 02, Arbeitsmodus 31.08., Blueprint §15 Punkt 16)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-58 | Ein Slot hat `occupied` = false | Er **verarbeitet nichts**, gleichgültig was seine dreizehn v1-Werte sagen: kein Filter, kein Detektor, kein Beitrag zur Auto-Gain-Ableitung, kein Eintrag im dynamischen Telemetriewert. | **ENTSCHIEDEN (R5)** · BAULÜCKE | **NEU** **B6**, Fall `freier_slot_verarbeitet_nichts` | Ein freier Slot mit `enabled` = true und `gain_db` = 12 färbt den Klang | R5; Fünferblock 02 `:117-121` |
| M-59 | Ein Slot ist belegt und `enabled` wird false | Der Slot bleibt **belegt**: Slot-ID, alle Werte und die Band-ID bleiben erhalten, und erneutes Einschalten verwendet denselben Slot und dieselben Werte. Ausschalten gibt **keinen** der acht Plätze frei. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `ausschalten_gibt_keinen_slot_frei` | Nach `enabled=false` meldet der Kern den Slot als frei; oder die Werte fallen auf Default | Fünferblock 02 `:117-121`; Arbeitsmodus `:229-231` |
| M-60 | Remove auf einem belegten Slot | **Eine** Transaktion mit **genau einer** neuen `state_revision`: `occupied` wird false **und** alle dreizehn v1-Werte des Slots gehen auf ihren Vertragsdefault. Andere Slot-IDs werden **niemals** umnummeriert. | **ENTSCHIEDEN (R5)** · BAULÜCKE | **NEU** **B7**, Fälle `remove_ist_eine_transaktion_und_eine_revision` und `remove_nummeriert_nicht_um` | Remove erzeugt zwei Revisionen (Freigabe und Defaultsetzen); oder Slot 5 rutscht nach dem Entfernen von Slot 3 auf Position 3 | R5; Fünferblock 02 `:136-150` |
| M-61 | Direkt nach dem Remove — auch wenn das Band unter einer nachträglich eingeschalteten Zone lag | Der Undo-Ring trägt **ein** Objekt mit Slot-ID, Typ, `channel_mode`, Dynamic-Zustand und **sämtlichen** Werten. Undo stellt es als **ein** Objekt in **einer** Transaktion wieder her — Slot-ID und Band-ID sind dieselben wie vorher. **Der Zonenriegel hält Undo nicht auf:** eine Wiederherstellung eines ganzen, bereits bestätigten Zustands ist keine Bedienänderung (§5.6.3 Feinheit 3, Ausnahme); der Ausgang ist §5.11.4 **T16**. Der Pflichtweg der Zeile ist deshalb: Band einschalten → Zone darüber einschalten (M-68 erlaubt das) → Remove → **unmittelbar** Undo → das Band liegt wieder da, gilt als **verletzt** und wird so gemeldet. **MN1 (B-02):** vor der Nacharbeit hätte der für jede Transaktion vorgeschriebene Riegel genau diesen zugesagten Rückweg abgewiesen. **MR:** dieselbe Lücke stand bis zur NAK-245-Runde noch in der Falltabelle — T4 verlangte für diesen Weg User-Schutz (Matrixprüfung 5); T4 gilt jetzt nur für Bedienänderungen, T16 trägt den Weg, und Redo und Preset-Laden trägt M-126. | **BELEGT** · **ENTSCHIEDEN (B-02-Regel, §5.6.3, §5.11.4 T16)** · BAULÜCKE · **MN1** · **MR** | **NEU** **B7**, Fälle `undo_stellt_das_entfernte_band_als_ein_objekt_her` und `undo_unter_einer_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` (T16) | Undo stellt die Werte her, aber nicht `channel_mode`; oder es braucht zwei Schritte; oder das Band landet auf einem anderen Slot; oder der Zonenriegel weist das Undo ab und der zugesagte unmittelbare Rückweg fehlt; oder das Undo committet, aber die Verletzung wird nicht gemeldet | Fünferblock 02 `:136-150`, besonders `:139-141`; R5; Matrixprüfung 1 B-02; §5.11.4 T16; Matrixprüfung 5 |
| M-62 | Alle acht Slots sind belegt, ein neuntes Band wird verlangt | Der Kern **überschreibt nichts** und meldet den Zustand „alle Slots belegt" als eigenen, unterscheidbaren Fehler. Er wählt nicht selbst ein Opfer. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `acht_belegte_slots_ueberschreiben_nichts` | Der Kern räumt den ältesten Slot; oder er meldet einen generischen Fehler, aus dem die Oberfläche nicht ableiten kann, was zu tun ist | Arbeitsmodus `:204-205` |
| M-63 | Host-Automation schreibt auf einen Parameter eines **freien** Slots | Der Wert **ändert sich** — Automation ist instanzgebunden und wird nicht blockiert —, aber weder die Belegung noch der Klang ändern sich. Gemessen wird beides: der Parameter trägt den neuen Wert, der Audioausgang ist bitgleich zum Lauf ohne die Automation. | **ENTSCHIEDEN (R5)** · BAULÜCKE | **NEU** **B7**, Fall `automation_auf_freien_slot_aendert_werte_nicht_klang` | Der Slot wird durch Automation belegt; oder die Automation wird abgewiesen und der Host sieht einen Parameter, der nicht folgt | R5; Erratum (n) „Host-Automation ist instanzgebunden" |
| M-64 | `occupied` soll automatisiert werden | `v2.band.<slot>.occupied` ist **kein Host-Parameter**: es steht nur im Kind `Dsp`, taucht nicht in der APVTS auf und ist damit von außen nicht automatisierbar. Belegung entsteht ausschließlich durch eine Transaktion. | **ENTSCHIEDEN (R5-Feinheit, §5.5)** · BAULÜCKE | **NEU** **B7**, Fall `occupied_ist_kein_hostparameter`; **B2** über den Save-Weg | `occupied` erscheint in der Hostparameterliste; oder eine Automationskurve gibt einen Slot frei und der Undo-Ring hat nichts davon | R5; §5.5 |
| M-65 | Ein v1-Stand (`dsp_schema_version` 1) wird geladen | Migration nach R5: `occupied` := `enabled` **oder** mindestens ein Wert des Slots weicht bitgenau vom Vertragsdefault ab. Ein v1-Stand verliert damit **kein** vom User gesetztes Band, und ein nie berührter Slot bleibt frei. Die Migration ist deterministisch und golden-fähig. | **ENTSCHIEDEN (R5)** · BAULÜCKE | **B2** erweitert, Fälle `migration_v1_setzt_occupied` und `migration_ist_golden`; **A12** für den Fixturekorpus | Ein Band mit `enabled=false`, aber gesetzter Frequenz gilt nach der Migration als frei und verschwindet; oder ein unberührter Slot gilt als belegt, weil ein Gleitkommavergleich mit Epsilon statt bitgenau prüft | R5 |

### 3.8 Schutz-Zonen (Gate: R6, Interview 23.08., §49.2 Gate 4)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-66 | Eine Zone wird angelegt | Die Liste liegt im Kind `Dsp`, ist **kein** Host-Parameter und fasst **0 bis 8** Einträge. Je Zone `id` (0 bis 7, im Bestand eindeutig), `low_hz`, `high_hz`, `enabled`. Das DTO prüft 20 Hz ≤ `low_hz` < `high_hz` ≤ 20 kHz; die Laufzeit kappt `high_hz` zusätzlich auf 0,45·fs, ohne den persistenten Wert zu ändern (§5.6). | **ENTSCHIEDEN (R6)** · BAULÜCKE · **MA** (§11) | **NEU** **B7**, Fälle `zone_dto_grenzen` und `zone_laufzeitkappung_aendert_nichts_persistentes`; **A12** Negativfixtures | `low_hz` == `high_hz` wird angenommen; oder ein bei 96 kHz gesetzter 20-kHz-Zonenrand macht das Projekt bei 44,1 kHz unlesbar | R6; §5.6 |
| M-67 | Eine Transaktion lässt ein belegtes, eingeschaltetes Band **neu in eine** eingeschaltete Zone eintreten | Die Transaktion wird **abgewiesen**, mit dem eigenen Grund „User-Schutz" und **ohne halbe Anwendung**: der bestätigte Zustand, die Revision und der Hash bleiben unverändert. Das Prädikat wird **je Zone einzeln** ausgewertet: abgewiesen wird, sobald für **irgendeine** eingeschaltete Zone der Eintritt dieses Slots von falsch auf wahr kippt und die Transaktion mindestens eines der Felder `occupied`, `enabled` oder `freq_hz` dieses Slots ändert. **MN1 (B-03):** eine Sammelprüfung „liegt in irgendeiner Zone" liess ein in Zone A verletzt liegendes Band nach Zone B wandern, weil das Prädikat dabei wahr blieb. | **ENTSCHIEDEN (R6, B-03-Regel)** · BAULÜCKE · **MN1** | **NEU** **B7**, Fälle `band_in_zone_ziehen_wird_abgewiesen`, `einschalten_in_der_zone_wird_abgewiesen`, `abweisung_lässt_hash_und_revision_unverändert` und `von_zone_a_nach_zone_b_wird_abgewiesen` | Das Band landet in der Zone; oder die Frequenz wird gesetzt und `enabled` nicht, sodass ein halber Zustand entsteht; oder **der Zwei-Zonen-Fall passiert den Riegel:** Band bei 100 Hz, Zone A 80–120 Hz nachträglich eingeschaltet, Zone B 900–1100 Hz eingeschaltet, Frequenzänderung auf 1000 Hz wird angenommen | R6; Interview `:128-135` „Harte Sperre für alle", besonders `:132-133`; Matrixprüfung 1 B-03 |
| M-68 | Eine Zone wird **über** einem bereits liegenden Band eingeschaltet | Das ist **erlaubt**. Das Band gilt danach als **verletzt** und wird in `state_report.dsp` als solches gemeldet. Es klingt weiter, es wird nicht stumm geschaltet und nicht verschoben. | **ENTSCHIEDEN (R6)** · BAULÜCKE | **NEU** **B7**, Fall `zone_ueber_band_ist_erlaubt_und_meldet_verletzung` | Das Einschalten der Zone wird abgewiesen; oder das Band wird still deaktiviert | R6 wörtlich |
| M-69 | Ein bereits verletztes Band wird weiter bedient, etwa `gain_db` geändert | Die Transaktion geht **durch**: das Prädikat war schon wahr und kippt nicht. Ein verletztes Band bleibt bedienbar; nur der Weg **hinein** ist gesperrt. | **ENTSCHIEDEN (R6-Feinheit, §5.6)** · BAULÜCKE | **NEU** **B7**, Fall `verletztes_band_bleibt_bedienbar` | Jede Änderung an einem verletzten Band wird abgewiesen und der User kommt nicht mehr heraus | §5.6; R6 |
| M-70 | Host-Automation schiebt die Frequenz eines Bandes in eine eingeschaltete Zone | Die Automation **schreibt vorbei**: sie wird nicht verhindert. Es entsteht **dieselbe** Verletzungsmeldung wie in M-68 — nicht ein zweiter Meldeweg und nicht ein stiller Zustand. | **BELEGT** · **ENTSCHIEDEN (R6)** · BAULÜCKE | **NEU** **B7**, Fall `automation_verletzt_und_meldet_denselben_zustand` | Die Automation wird geklemmt und der Host sieht einen Parameter, der nicht folgt; oder die Verletzung bleibt unsichtbar | Interview `:252-263` „Ehrliche Grenze der harten Sperre"; R6 |
| M-71 | Eine Zone wird gelöst | „Lösen" ist entweder `enabled` = false **oder** das Entfernen des Eintrags, **beides** als Transaktion mit genau einer Revision. Nach dem Lösen ist kein Band mehr verletzt, und der Weg in den Bereich ist wieder frei. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fälle `zone_deaktivieren_ist_eine_transaktion` und `zone_entfernen_ist_eine_transaktion` | Das Entfernen läuft ohne Revision und der Hash bleibt stehen; oder nach dem Lösen gilt ein Band weiter als verletzt | Interview `:252-263`; R6 |
| M-72 | Der Zustand wird gespeichert oder berichtet | Zonen sind Teil des **DTO**, des **`state_hash`**, des **Presets** und von **`state_report.dsp`**. Vier Orte, eine Quelle: es gibt keine zweite Zonenliste. | **ENTSCHIEDEN (R6)** · BAULÜCKE | **B2**, **NEU** **B7**, **A12**, **A8** je einen Fall | Zwei Stände mit verschiedenen Zonen liefern denselben Hash; oder das Preset trägt Zonen, die der Bericht nicht kennt | R6 wörtlich |
| M-73 | Neun Zonen sollen angelegt werden | Der Kern **weist ab** — die Liste ist auf acht begrenzt, und die Begrenzung wird im DTO geprüft, nicht erst in der Oberfläche. Doppelte `id` werden ebenfalls abgewiesen. | **ENTSCHIEDEN (R6)** · BAULÜCKE | **NEU** **B7**, Fälle `neunte_zone_wird_abgewiesen` und `doppelte_zonen_id_wird_abgewiesen`; **A12** Negativfixtures | Die neunte Zone wird stillschweigend verworfen und der Aufrufer glaubt, sie sei angelegt | R6 |
| M-126 | Redo oder Preset-Laden lässt ein Band **neu** in eine eingeschaltete Zone eintreten (etwa: ein Preset legt ein Band in eine eingeschaltete Zone → Undo → Redo) | Beide sind **Ganzzustands-Wiederherstellungen** und passieren den Zonenriegel nicht (§5.6.3 Feinheit 3, Ausnahme): die Transaktion committet mit genau einer Revision, das Band gilt als **verletzt** und wird wie in M-68 gemeldet. Der Ausgang ist §5.11.4 **T16** — dieselbe Zeile, die M-61 für Undo zitiert; scheitert eine andere Stufe, gilt nach **I6** deren Zeile. **MR, neu:** die Ausnahme stand für Redo und Preset-Laden bis zur NAK-245-Runde nur in §5.6.3, ohne Matrixzeile und ohne Test; der Pflichtweg aus M-61 deckt nur Undo. | **ENTSCHIEDEN (B-02-Regel, §5.6.3 Feinheit 3, §5.11.4 T16)** · BAULÜCKE · **MR** | **NEU** **B7**, Fälle `redo_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` und `preset_laden_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` (T16) | Redo oder Preset-Laden wird mit User-Schutz abgewiesen; oder es committet, aber die Verletzung wird nicht gemeldet | §5.6.3 Feinheit 3 (Ausnahme, Dirigent-Entscheid 10.09.2026); R6; R11; M-68; Matrixprüfung 5 |

### 3.9 Transaktion, Revision, Hash, Undo-Ring und Automation (Gate: R11, §44.3, §44.4, §44.5, §53.8)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-74 | Apply, Revert, Neutralisieren, Remove, Undo, Redo oder Preset-Laden | Jede dieser sieben Handlungen ist **eine atomare Transaktion mit genau einer neuen `state_revision`** — nicht null, nicht zwei; wie sich `r` dabei bewegt, steht in §5.11.4 Teil 1. Die **Stufenfolge S0 bis S8** und der Commit-Punkt stehen in §5.11.4 Teil 2; diese Zeile wiederholt sie **nicht**, sie misst sie. Zugesagt sind hier: genau eine Revision je Handlung (§5.11.4 **T6**; für eine Ganzzustands-Wiederherstellung, die ein Band in eine Zone führt, **T16**), alle fehlbaren und allozierenden Stufen **vor** dem Commit-Punkt, und ein Registereintrag **ausschließlich** in S8 für eine committete Transaktion (Invariante **I4**). **MK:** bis zur Konvergenzrunde trug diese Zeile die Stufenliste ein zweites Mal — in drei Runden ist sie dabei dreimal von §5.11.4 abgewichen (§7.8). **MR:** der Satz „`r` steigt um 1 und sinkt nie“ stand hier als eigene Arithmetik; seit dem Ladestart `r0` gilt „sinkt nie“ nur innerhalb einer Sitzung, und die Zeile zitiert Teil 1. | **BELEGT** · **ENTSCHIEDEN (§5.11.4 Teil 1, Teil 2, T6, T16, I4)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** · **MR** | **NEU** **B7**, sieben Fälle `<handlung>_erzeugt_genau_eine_revision`, dazu `commit_erzeugt_genau_eine_revision_und_einen_eintrag` (T6) und `alle_fehlbaren_stufen_liegen_vor_dem_commit_punkt` | Neutralisieren zählt zwei Revisionen; oder Undo zählt gar keine; oder eine Stufe hinter dem Commit-Punkt kann fehlschlagen; oder ein Ausgang ohne Commit legt einen Registereintrag an (I4) | §44.3; §44.4 „`Neutralisieren` ist selbst eine versionierte Transaktion"; R11; §5.11.4; Matrixprüfung 5 |
| M-75 | Ein Fehler tritt in **irgendeiner** fehlbaren Stufe eines Apply auf | Der bestätigte Zustand bleibt **unverändert**, der `state_hash` bleibt **unverändert**, `r` steigt **nicht**, der Undo-Ring bekommt **keinen** Eintrag — **und das Register auch nicht** (§5.11.4 **I4**; die Fehlerausgänge der Falltabelle sind sämtlich „nicht memoisiert"). Gemessen wird mit **je einem Einspritzpunkt pro fehlbarer Stufe** aus §5.11.4 Teil 2, alle vor dem Commit-Punkt. Welchen Ausgang jeder Einspritzfall hat, sagt die Tabelle und nicht diese Zeile: **T2**, **T3**, **T4**, **T13**, **T5**, **T14** und **T15**, in der Reihenfolge der Stufen. **S0 ist bewusst nicht darunter:** der Nachschlag liest ein vorallokiertes Register fester Größe, allokiert nicht und kann nicht fehlschlagen — ein Treffer ist ein Ergebnis, ein Fehltreffer der Normalfall. Ein Einspritzpunkt dort wäre ein Fehler, den es nicht geben kann, und er stünde vor allen anderen. **MK:** die Stufennamen kommen aus §5.11.4 Teil 2, nicht aus dieser Zeile. **MR:** bis zur NAK-245-Runde zählte die Zeile die Stufen noch selbst auf, und für Fehler in S4, S6 und S7 hatte die Tabelle keine Zeile (Matrixprüfung 5); jetzt zitiert sie die Zeilen. | **BELEGT** · **ENTSCHIEDEN (§5.11.4 Teil 2, I4, T13, T14, T15)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** · **MR** | **NEU** **B7**, sieben Fälle `fehler_in_S<n>_laesst_committed_und_hash_unveraendert` (je einer an der Tabellenzeile seiner Stufe), dazu `fehler_hinterlaesst_keinen_registereintrag` und `nachschlag_allokiert_nicht_und_schlaegt_nicht_fehl` (Allokationszähler über 10.000 Nachschläge) | Nach einem Fehler in S6 ist `r` erhöht und der Committed-Zustand getauscht; oder ein Fehler in S7 hinterlässt einen halben Ringeintrag; oder ein abgewiesener Ausgang landet im Register und blockiert die Wiederholung; oder der Nachschlag allokiert | R11; §44.3; §5.11.4; Matrixprüfung 5 |
| M-76 | 10.000 doppelte, vertauschte und veraltete Transaktionen, **fernsteuerungsfrei** gegen den lokalen Transaktionskern | Sie erzeugen **höchstens eine** gültige Revision und **niemals** einen Mischzustand — und diese Zeile misst **nur** diesen Satz aus §44.5. Alle 10.000 Eingaben stammen aus **einer** logischen Transaktion T mit fester Nutzlast: dieselbe `tid` wiederholt (doppelt), in vertauschter Ankunftsreihenfolge (vertauscht), mit veralteter oder aktueller `base_revision` (veraltet). Der Lauf beginnt unmittelbar nach dem Ladestart eines Standes mit `r0 ≥ 1` (§5.11.4 Teil 1), damit es ältere Revisionen gibt, und für T bestehen S2 bis S7. Per Konstruktion kann der Lauf höchstens **einen** Commit erzeugen. Zugesagt ist: `r − r0 ≤ 1` am Ende, und der bestätigte Zustand ist entweder der Ausgangszustand oder genau der Zustand nach T, nie eine Mischung. Im Lauf kommen nur die Ausgänge dreier Tabellenzeilen vor: **T2** (veraltete `base_revision` vor dem Commit), **T6** (der eine Commit) und **T1** (jede Eingabe danach). Die Zeile fährt die Falltabelle **nicht**; die vollständige, tabellengetriebene Prüfung ist allein **M-125**. **MK:** die Zeile nennt keine Zahl selbst; Fenster und Kapazität stehen in §5.11.4 Teil 1. **MR:** bis zur NAK-245-Runde sollte dieser Lauf die ganze Falltabelle fahren und zugleich höchstens eine Revision erzeugen — T6, T7 und T9 committen aber je einmal, und ein korrekter Kern wäre am Rotbeweis dieser Zeile gefallen (Matrixprüfung 5). | **BELEGT** · **ENTSCHIEDEN (§5.11.4 T1, T2, T6)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** · **MR** | **NEU** **B7**, Fall `zehntausend_transaktionen_erzeugen_hoechstens_eine_revision` (alle Eingaben aus einer logischen Transaktion; kein Tabellenlauf) | Zwei Revisionen entstehen; oder der bestätigte Zustand trägt Werte aus dem Ausgangszustand und aus T zugleich; oder der Beweis läuft über den Fernweg-ACK statt gegen den lokalen Kern und ist damit in P6 nicht fahrbar | §44.5 wörtlich; `CLAUDE.md` „Zustands- und Parameterhoheit bleibt in der Audio führenden Instanz"; §5.11.4; Matrixprüfung 5 |
| M-123 | Eine **committete** Transaktion wird nach ihrem Abschluss erneut angeboten | Liegt ihre `tid` im Fenster, liefert **S0** sofort das gespeicherte Ergebnis — dieselbe Revision `e`, denselben Hash — **ohne** zweite Revision und ohne eine weitere Stufe (§5.11.4 **T1**). Fenster, Kapazität und Belegung stehen in §5.11.4 Teil 1 und in **I1** bis **I3**; die Zeile rechnet nicht selbst. Jenseits des Fensters ist die `tid` unbekannt, und die veraltete `base_revision` ergibt **Konflikt** (**T10**). Nach einem **Ladestart** ist das Register leer, die geladene Revision bleibt (`r0`, §5.11.4 Teil 1), und eine Wiederholung aus der vorigen Sitzung ist ein **neuer Versuch** (**T17**) — die Konsequenz aus I5, kein Fehler. Vier Pflichtfälle: der **Fensterrand als Paar** (T10 und T11), der **Mischfall** (T12 — Abweisungen verdrängen T nicht), die **Sitzungsgrenze** (Ladestart und T17) und der **Zonenablauf** aus MN2 (1000 Hz → T auf 1050 Hz → 500 Hz → T wiederholt → T's Ergebnis, kein „User-Schutz"). **MK:** die Zeile sagte bis zur Konvergenzrunde „höchstens 32 Revisionen hinter" **und** „nie mehr als 32 Einträge" — das ergab 33 gegen 32 und war K-1. **MR:** I1 und I2 zählten bis zur NAK-245-Runde ab Revision 0; ein geladener Stand mit hoher `state_revision` hätte sofort volle Belegung verlangt, obwohl das Register nach I5 leer ist (Matrixprüfung 5). Die Zeile zitiert beide jetzt in ihrer Fassung mit `r0`. | **ENTSCHIEDEN (§5.11.4 Teil 1, I1, I2, T1, T10, T11, T12, T17)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** · **MR** | **NEU** **B7**, Fälle `wiederholung_im_fenster_liefert_dasselbe_ergebnis` (T1), `fensterrand_erste_transaktion_ist_draussen` (T10), `fensterrand_zweite_transaktion_ist_drinnen` (T11), `abweisungen_verdraengen_keinen_committeten_eintrag` (T12), `wiederholung_unter_einer_zone_bekommt_nicht_user_schutz`, `ladestart_hat_leeres_register_und_haelt_die_revision` (I1 mit `r0`, M-93) und `wiederholung_ueber_sitzungsgrenze_ist_neuer_versuch` (T17) | Die Wiederholung erzeugt eine zweite Revision; oder **am Fensterrand liegt die erste Transaktion der Sitzung noch im Register** (T10 — dann ist die Kapazität größer als die Undo-Tiefe und I1 verletzt); oder **die zweite ist verdrängt** (T11 — dann ist das Fenster kleiner als die Undo-Tiefe); oder der Mischfall endet mit Konflikt; oder die Wiederholung läuft in den Zonenriegel; oder **nach dem Laden liegen Einträge im Register** (es wurde mitgespeichert, I5 gebrochen); oder **das Laden setzt `r` auf 0 zurück** (M-93 gebrochen) | `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2372`; §44.5; §5.11.4; Matrixprüfung 4 K-1; Matrixprüfung 5 |
| M-124 | Eine Transaktion endete **ohne Commit** und wird mit derselben `tid` erneut angeboten | Sie ist ein **neuer Versuch**: nichts wurde memoisiert (**I4**), also findet **S0** nichts, und die Eingabe durchläuft die volle Stufenfolge und erhält das Urteil des **aktuellen** Zustands; nach **I6** entscheidet die erste Stufe, die nicht besteht. **Der Ausgang hängt davon ab, ob sich `r` zwischenzeitlich bewegt hat** — und genau hier liegt der Unterschied, den die Konvergenzrunde festgeschrieben hat. Nach **`busy_retry`** hat sich nichts bewegt: die Bankfreigabe ist ein Audio-ACK und **keine** Transaktion, die alte `base_revision` gilt weiter, und die Wiederholung committet unter den Bedingungen von **T7**. Nach **User-Schutz** dagegen ist das Lösen der Zone **selbst eine Transaktion** und hat `r + 1` erzeugt (M-71): dieselbe `tid` mit der **alten** `base_revision` ergibt **Konflikt** — und das ist **richtiges** Verhalten nach M-76 (**T8**); mit **aktualisierter** `base_revision` committet sie unter den Bedingungen von **T9** — ist der Pool dann voll und das Programm **bankpflichtig** (§5.11.4 Teil 2), ist der Ausgang **T5**; ein bankfreies Programm committet auch dann (T9). **MK:** der MN3-Rotbeweis „eine Wiederholung nach behobenem Zonenkonflikt wird weiter abgewiesen" hätte T8 als Fehler gewertet, obwohl T8 korrekt ist — das war K-2. Er ist auf T9 eingegrenzt, und T8 ist als eigener Fall dazugekommen. **MR:** T9 verlangte bis zur NAK-245-Runde nur die aktualisierte `base_revision` und widersprach bei vollem Pool T5 (Matrixprüfung 5); die Abgrenzung ist jetzt ein eigener Fall. **MR1:** bis zur Nacharbeit 1 der NAK-245-Runde nannte die Abgrenzung den vollen Pool ohne die Bankpflicht, und ihr Rotbeweis hätte den korrekten Commit eines bankfreien Programms (`eq_enabled = false`, E-18) als Fehler gewertet (Matrixprüfung 6); jetzt ist sie zweigeteilt. | **ENTSCHIEDEN (§5.11.4 Teil 2, T5, T7, T8, T9, I4, I6)** · BAULÜCKE · **MN2** · **MN3** · **MK** · **MR** · **MR1** | **NEU** **B7**, Fälle `wiederholung_nach_bankfreigabe_committet` (T7), `retry_nach_zonenloesung_mit_alter_base_revision_ist_konflikt` (T8), `retry_nach_zonenloesung_mit_aktueller_base_revision_committet` (T9), `retry_nach_zonenloesung_bei_vollem_pool_ist_busy_retry` (Abgrenzung T9 gegen T5, bankpflichtig), `retry_nach_zonenloesung_bei_vollem_pool_ohne_bankpflicht_committet` (T9, bankfrei bei vollem Pool) und `register_ist_nicht_teil_des_zustands` (I5: Speichern schreibt es nicht, der Ladestart leert es, der Hash ändert sich nicht) | Eine Wiederholung nach `busy_retry` bekommt das gespeicherte `busy_retry` zurück; oder **T8 committet trotz veralteter `base_revision`** und umgeht M-76; oder T9 wird weiter mit User-Schutz abgewiesen; oder **ein bankpflichtiges T9 committet bei vollem Pool statt `busy_retry`**; oder **ein bankfreies T9 bekommt bei vollem Pool `busy_retry` statt Commit**; oder das Register taucht im gespeicherten Zustand auf | §5.11.4 Teil 2, T5, T7 bis T9; M-71; M-76; Matrixprüfung 4 K-2; Matrixprüfung 5; Matrixprüfung 6 |
| M-125 | Der Transaktionskern wird gegen die **ganze** Falltabelle gefahren | Für **jede** Zeile der Falltabelle aus §5.11.4 Teil 4 — ihre Zahl steht dort und nur dort — stellt der Test den beschriebenen Zustand her, gibt die beschriebene Eingabe und prüft **alle vier** Spalten: Ausgang, `r` danach, memoisiert ja/nein und — bei einem Treffer — die zurückgegebene Revision und den Hash. Dazu drei Invarianten aus §5.11.4 Teil 3 als Wachen **nach jeder Eingabe und nach jedem Ladestart**, in ihrer Fassung mit `r0`: die Belegung (**I1**), die Fensterzugehörigkeit (**I2**) und „kein Registereintrag ohne Commit" (**I4**). Der Lauf ist **tabellengetrieben**: fällt eine Zeile aus der Tabelle, fällt der Test, und eine neue Zeile ohne Fall ist ein Übersetzungsfehler. Jede Commit-Zeile erzeugt dabei genau eine Revision; die Zusage „höchstens eine Revision“ aus §44.5 misst dieser Lauf **nicht**, sie gehört M-76. **MK, neu:** ohne diese Zeile wäre die Tabelle eine Beschreibung; mit ihr ist sie eine Zusage. **MR:** bis zur NAK-245-Runde nannte die Zeile die Zeilenzahl und die Formeln von I1 und I2 selbst, und M-76 sollte dieselbe Tabelle fahren; jetzt zitiert sie beides nur (Matrixprüfung 5). | **ENTSCHIEDEN (§5.11.4 Teil 3, Teil 4)** · BAULÜCKE · **MK** · **MR** | **NEU** **B7**, Fall `falltabelle_vollstaendig_gefahren` (jede Zeile der Tabelle, vier Spalten je Zeile, drei Invariantenwachen) | Eine Tabellenzeile hat keinen Fall; oder ein Fall prüft nur den Ausgang und nicht `memoisiert`, sodass I4 unbemerkt brechen kann; oder die Invariantenwachen laufen nur am Ende statt nach jeder Eingabe; oder **die Wache I1 rechnet ohne `r0`** und verlangt nach dem Laden eines Standes volle Belegung, obwohl das Register nach I5 leer ist | §5.11.4 Teil 3 und Teil 4; `tools/dirigent/pruefliste.md` E; Matrixprüfung 5 |
| M-77 | `state_hash` wird gebildet | SHA-256 über die **RFC-8785-Form des validierten DTO**, gebildet mit der bestehenden Implementierung aus SONDE-006 — nicht neu geschrieben. Die JCS-Zahlen- und Dokumentvektoren aus `eq-copilot/fixtures/state/jcs/` bleiben unverändert gültig, und die drei Sprachen bilden weiterhin denselben Hash über denselben Text. | **BELEGT** · **ENTSCHIEDEN (R11)** · BAULÜCKE | **A12**, **B2**, **A4** (Rust) gegen dieselben Fixtures; **NEU** **B7**, Fall `hash_kommt_aus_dem_bestandskanon` | Der Kanon wird für v2 neu geschrieben und ein RFC-Vektor kippt; oder C++ und Rust liefern für dasselbe DTO verschiedene Hashes | R11; `nakama-state-v2.md:170-176` |
| M-78 | Der Undo-Ring wächst | Tiefe **höchstens 32** (Vertragsgrenze `undo_tiefe`), persistiert im Kind `Dsp`. Beim 33. Eintrag fällt der älteste heraus; der Ring wächst nie über seine Grenze und die Bytegrenzen des State-Lesers bleiben eingehalten. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `undo_ring_haelt_tiefe_32`; **B2** über den Save-Weg mit vollem Ring | Der 33. Eintrag verdrängt keinen und der State wächst; oder der volle Ring reißt die 16-MiB-Grenze und der Stand wird beim nächsten Laden read-only | `eq-ipc-v3.schema.json:2321` (`maximum: 32`); `nakama-state-v2.md:188-191` |
| M-79 | Undo, dann eine neue Transaktion | Der Redo-Zweig wird **abgeschnitten**: nach einer neuen Transaktion gibt es kein Redo mehr auf den verworfenen Ast. Undo und Redo bewegen einen Cursor im selben Ring; sie legen keinen zweiten Ring an. | **ENTSCHIEDEN (R11-Feinheit, §5.11)** · BAULÜCKE | **NEU** **B7**, Fall `neue_transaktion_schneidet_den_redo_zweig_ab` | Ein Redo nach einer neuen Transaktion stellt einen Zustand her, den es nie gab | §5.11; R11 |
| M-80 | Neutralisieren | Es setzt alle Werte auf ihren Vertragsdefault und ist **selbst eine versionierte Transaktion** — kein Löschen der Historie. Der Undo-Ring behält seine Einträge und bekommt einen weiteren; Undo führt zurück in den Stand davor. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `neutralisieren_loescht_die_historie_nicht` | Neutralisieren leert den Undo-Ring; oder es wirkt ohne Revision | §44.4 wörtlich |
| M-81 | Host-Automation läuft auf einem Parameter | Sie wirkt im flüchtigen **`AutomationOverlay`** mit `automation_epoch` und zählt **keine** Revision — auch nicht pro Automationspunkt. Die Epoche wechselt einmal zu Beginn und einmal am Ende einer Hostgeste beziehungsweise nach einer definierten Ruhegrenze. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fälle `automation_erzeugt_keine_revision` und `epoche_wechselt_genau_zweimal_je_geste` | Eine Automationsfahrt erzeugt hunderte Revisionen; oder die Epoche zählt pro Punkt | §44.3 wörtlich |
| M-82 | Ein abgeschlossener manueller Parametergestus | Er erzeugt **genau eine** Revision — nicht eine pro Zwischenwert. Der bestätigte Zustand nimmt den Endwert auf. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `abgeschlossener_gestus_ist_eine_revision` | Jeder Zwischenwert einer Reglerfahrt landet als eigene Revision im Undo-Ring | §44.3 wörtlich |
| M-83 | Eine Vorschau ist aktiv, das Projekt wird gespeichert | Eine Preview wird **nie serialisiert**: der gespeicherte Zustand trägt ausschließlich den bestätigten. Ein Speichern während einer laufenden Vorschau erzeugt denselben Hash wie ohne. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `preview_wird_nie_serialisiert`; **B2** über den Save-Weg | Der gespeicherte Hash trägt den Vorschauwert | §44.4; §49.2 Gate 3 |
| M-84 | Ein Projekt wird neu geladen | Der Reload rekonstruiert **denselben `state_hash`** und innerhalb numerischer Toleranz denselben Audioausgang. Reihenfolge: State lesen → validieren → Committed setzen → Hostparameter synchronisieren → erster Block. | **BELEGT** · BAULÜCKE | **B2** erweitert, Fall `reload_rekonstruiert_denselben_hash`; **NEU** **B7**, Fall `reload_rekonstruiert_denselben_audioausgang` | Der Hash nach dem Reload unterscheidet sich, weil ein transienter Wert mitgehasht wurde | §44.5 wörtlich |
| M-85 | Eine persistente Änderung ist geschrieben | Sie meldet dem Host **Dirty-State** über `updateHostDisplay(… withNonParameterStateChanged(true))`. Eine **abgewiesene** Transaktion meldet **kein** Dirty. Laden und Migration melden ebenfalls nicht (Bestandsregel). | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fälle `angenommene_transaktion_meldet_dirty` und `abgewiesene_meldet_nicht` | Ein am Zonenriegel abgewiesenes Apply erhöht den Dirty-Zähler | `nakama-state-v2.md:206`; `CLAUDE.md` |

### 3.10 Layout v2, Save/Load und Migration (Gate: R1, §53.8, `nakama-state-v2.md`, `nakama-parameter-v1.json:8`)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-86 | Layout v2 entsteht | `dsp_schema_version` wird **2**. Alle **109** v1-Kennungen bleiben unverändert in ID, Typ, Grenzen, Default und Wechselart. Die neuen Werte tragen ausnahmslos das Präfix `v2.*`; es sind **elf**: `v2.global.eq_enabled`, `v2.global.mix`, `v2.global.auto_gain` und acht `v2.band.<slot>.occupied`. Das Layout umfasst damit **120** Kennungen. | **ENTSCHIEDEN (R1)** · BAULÜCKE | **A12** erweitert, Fall `layout_v2_hat_120_kennungen_und_109_unveraenderte`; **B2** über den C++-Spiegel | Eine v1-Kennung ändert ihren Default; oder ein neuer Wert bekommt einen freien Platz im v1-Bestand statt ein neues Präfix | R1; `nakama-parameter-v1.json:8` wörtlich |
| M-87 | Der Vertrag wird abgelegt | v1 bleibt als Datei **eingefroren**; v2 liegt in einer eigenen Datei, die v1 unverändert listet **plus** die Erweiterung. Der C++-Spiegel trägt `static_assert` auf die neuen Zahlen. Jede v2-Kennung trägt ein Attribut, ob sie **Host-Parameter** ist oder **nur persistenter Zustand im Kind `Dsp`**. | **ENTSCHIEDEN (R1)** · BAULÜCKE | **A12** erweitert; **NEU** **B7**, Fall `jede_v2_kennung_erklaert_ihren_ort` | Die v1-Datei wird editiert statt eingefroren; oder eine v2-Kennung sagt nicht, ob sie automatisierbar ist, und landet versehentlich in der APVTS | R1 |
| M-88 | Die APVTS wird aufgebaut | Sie führt **112** Host-Parameter: die 109 v1-Kennungen in Vertragsreihenfolge, danach `v2.global.eq_enabled`, `v2.global.mix`, `v2.global.auto_gain`. Die v2-Host-Parameter stehen **hinter** allen v1-Kennungen, damit ein späteres Layout wieder anhängen kann, ohne einen bestehenden Index zu verschieben. `occupied` ist **nicht** dabei (M-64). Probeeq führte bisher **null** Host-Parameter, es verschiebt sich also kein bestehender Index und die eingefrorene Identität bleibt unberührt. | **ENTSCHIEDEN (R1-Feinheit, §5.1)** · BAULÜCKE | **NEU** **B7**, Fälle `apvts_fuehrt_112_parameter_in_vertragsreihenfolge` und `v2_haengt_hinten_an`; **B1** unverändert (Identität) | Ein v2-Parameter wird zwischen die v1-Kennungen einsortiert; oder `occupied` erscheint in der Liste; oder eine Class-ID ändert sich | R1; `CLAUDE.md` „Identität bleibt eingefroren"; §53.8 „Die Hostparameterliste ändert sich nie dynamisch" |
| M-89 | Das Kind `Dsp` wird geschrieben | Es trägt `state_revision`, die acht `occupied`, die Zonenliste und den Undo-Ring — jeweils als **flache Arrays**, nicht als Kindknoten, weil der State-Leser höchstens 64 `ValueTree`-Knoten im ganzen Baum zulässt. Der bestätigte DspState wird **nicht** ein zweites Mal abgelegt: die 112 Werte stehen in `Parameters`, und eine zweite Kopie könnte auseinanderlaufen (§5.1, Abweichung 1). | **ENTSCHIEDEN (R1-Feinheit, §5.1)** · BAULÜCKE | **B2** erweitert, Fälle `dsp_kind_nutzt_flache_arrays` und `keine_zweite_wertekopie` | Der Baum überschreitet 64 Knoten und wird beim nächsten Laden read-only; oder `Parameters` und `Dsp` tragen verschiedene Werte für dasselbe Band | §5.1; `nakama-state-v2.md:188-191`; §33.5 „keine zweite Wahrheit" |
| M-90 | Ein v1-Stand wird geladen | Er lädt **verlustfrei** mit Defaults für alle v2-Werte, `occupied` nach M-65. Nichts geht verloren, nichts wird erfunden. | **ENTSCHIEDEN (R1)** · BAULÜCKE | **B2** erweitert, Fall `v1_stand_laedt_verlustfrei_nach_v2`; **A12** Goldens | Ein v1-Stand landet read-only; oder ein v1-Wert wird beim Umschreiben verändert | R1 |
| M-91 | Ein v2-Stand mit **unbekannten** Eigenschaften im Kind `Dsp` | Der Leser **ignoriert** sie und erhält sie beim Speichern; das DTO selbst bleibt **exakt** geprüft (unbekannter Schlüssel im DTO ist ein Fehler). Beide Regeln gelten nebeneinander und widersprechen sich nicht: additiv ist der **Baum**, exakt ist das **DTO**. | **ENTSCHIEDEN (R1)** · BAULÜCKE | **B2** erweitert, Fälle `unbekannte_dsp_eigenschaft_bleibt_erhalten` und `unbekannter_dto_schluessel_faellt` | Eine unbekannte Eigenschaft macht den Stand read-only; oder ein unbekannter DTO-Schlüssel wird durchgelassen und der Hash meint etwas anderes als er sagt | R1; `nakama-state-v2.md:48`, `:171` |
| M-92 | Ein Stand mit unbekanntem **Major** | Er bleibt **read-only mit Originalbytes** wie heute: audio-neutral, keine Bindung, `getStateInformation` liefert die Bytes unverändert zurück. Der bestehende Zweig in `NakamaState.cpp` bleibt in seiner Wirkung unverändert; nur die Bedingung „`Dsp` vorhanden" fällt weg. | **BELEGT** · BAULÜCKE | **B2** unverändert plus Fall `dsp_kind_ist_nicht_mehr_readonly` | Ein fremdes Major wird plötzlich geladen; oder ein `Dsp`-Kind bleibt read-only, obwohl dieser Build es liest | `nakama-state-v2.md:179-191`; `NakamaState.cpp:1059` |
| M-93 | Speichern, laden, speichern | Bytegleich für jeden Stand, den dieser Schreiber selbst geschrieben hat — inklusive `Dsp`. Die Schreibreihenfolge ist fest: Common, MainProject, Parameters, Dsp. | **BELEGT** · BAULÜCKE | **B2** erweitert, Fall `roundtrip_mit_dsp_ist_bytegleich`; **A16**, Fall `speichern_laden_speichern_bytegleich_in_v2` | Zwei Speicherläufe desselben Zustands liefern verschiedene Bytes, weil die Zonenliste unsortiert geschrieben wird | `nakama-state-v2.md:37` |
| M-94 | Der Fixture-Korpus wird neu erzeugt | `dto/ungueltig/dsp-schema-version-2.json` wechselt die Seite und wird ein **gültiges** Fixture. Der ganze Korpus wird vom **Writer** neu erzeugt und eingefroren, nicht von Hand editiert; synthetische Fälle sind deklarierte Mutanten genau eines Writer-Fixtures mit benannter Abweichung. | **ENTSCHIEDEN (R1)** · BAULÜCKE | **A12** `--pruefen` (bytegleich zur Neuerzeugung); **A8** für den v3-Korpus | Ein Fixture wird von Hand editiert und weicht in einem Feld von der Writerform ab — genau die Klasse, die in NAK-94 dreimal an je einem weiteren Feld fiel | R1; `tools/dirigent/pruefliste.md` E „Writer-Fixtures statt Handschrift" |

### 3.11 Preset (Gate: R12, U18 30.08., Blueprint §3.5)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-95 | Ein Preset wird geschrieben | Eigenes Schema mit `preset_schema_version` = 1, **nie** mit `dsp_schema_version` vermischt. Es trägt zusätzlich die `dsp_schema_version`, mit der es geschrieben wurde — damit ein späteres Layout weiß, was es liest. **MN1 (B-12):** die Schemagültigkeit prüft **A12** `erzeuge_state_fixtures.py`, erweitert um das Presetschema und seine Writer-Fixtures — dieses Bein liest bereits `eq-copilot/schemas/state/` (`:58`). **A11** `pruefe_v2_schemas.py` ist dafür der falsche Ort: es prüft eine **eingefrorene Familie von fünf** v2-Verträgen unter `eq-copilot/schemas/` (`:38-45`) und sieht `schemas/state/` nicht; würde es dennoch erweitert, müsste seine Runner-Behauptung im selben Änderungssatz mitwandern, weil sie heute „Die fuenf v2-Vertraege" sagt. | **BELEGT** · **ENTSCHIEDEN (R12, B-12-Regel)** · BAULÜCKE · **MN1** | **NEU** **B7**, Fall `preset_traegt_zwei_getrennte_versionen`; **A12** für Schemagültigkeit und Writer-Fixtures | Das Preset trägt nur eine Version und ein v3-Layout kann seinen Inhalt nicht mehr einordnen; oder das genannte Bein liest das Presetschema gar nicht und die Zusage ist unbelegt | R12; U18 „eigene Kennungen und eine eigene Version, die nie mit der Zustandsversion vermischt wird"; Matrixprüfung 1 B-12 |
| M-96 | Was im Preset steht | **Klanginhalt:** alle acht Bänder einschließlich `occupied`, Bypass, Mix, Schutz-Zonen, Auto-Gain, Input- und Output-Trim, Width, Mono-Bass. | **BELEGT** · **ENTSCHIEDEN (R12)** · BAULÜCKE | **NEU** **B7**, Fall `preset_traegt_den_klanginhalt_vollstaendig` | Ein Feld des Klanginhalts fehlt und ein geladenes Preset klingt anders als das gespeicherte | R12; U18 |
| M-97 | Was **nicht** im Preset steht | **Identität und Betrieb:** Zieladresse, `instance_id`, Bus-Name, Label, `v2.global.eq_enabled` und der Undo-Ring haben **kein Feld**. Nicht „leer", sondern kein Feld — sonst wäre die Struktur ein Angriffsweg auf die Verdrahtung. Ein Preset mit einem Identitätsfeld wird **abgelehnt**. | **BELEGT** · **ENTSCHIEDEN (R12)** · BAULÜCKE | **NEU** **B7**, Fälle `preset_hat_keine_identitaetsfelder` und `identitaetsfeld_wird_abgelehnt` (Negativfixture je Feld) | Ein Preset mit `instance_id` wird geladen und verdrahtet die Sonde um | R12; U18 „sonst würde das Laden einer Voreinstellung die Sonde umverdrahten, statt ihren Klang zu setzen"; Blueprint §3.5 |
| M-98 | Ein Preset mit **unbekannten** Top-Level-Feldern | Sie werden **ignoriert**; das Preset lädt. | **BELEGT** · **ENTSCHIEDEN (R12)** · BAULÜCKE | **NEU** **B7**, Fall `unbekanntes_top_level_feld_wird_ignoriert` | Das Laden scheitert an einem Feld, das ein neuerer Schreiber ergänzt hat | R12; Gate-Text „unbekannte Felder dürfen alte Consumer nicht zerstören" |
| M-99 | Ein Preset mit unbekanntem **Major** | Es wird mit einem **klaren Fehler abgelehnt**. Die read-only-Haltung des Projektzustands gilt hier **nicht**: ein Preset ist kein Projektzustand, und ein halb verstandenes Preset stumm zu halten hilft niemandem. | **ENTSCHIEDEN (R12)** · BAULÜCKE | **NEU** **B7**, Fall `unbekanntes_preset_major_wird_abgelehnt` | Ein fremdes Preset-Major wird read-only gehalten wie ein Projektstand und der User bekommt keine Meldung | R12 wörtlich |
| M-100 | Ein Preset wird geladen | **Eine** Transaktion mit genau einer Revision und **einem** Undo-Eintrag; der Zustand davor ist mit einem Undo vollständig zurück. Nicht angetastet: Identität, `eq_enabled` und der bestehende Undo-Ring (er bekommt einen Eintrag, er wird nicht ersetzt). | **ENTSCHIEDEN (R12)** · BAULÜCKE | **NEU** **B7**, Fälle `preset_laden_ist_eine_transaktion` und `undo_nach_preset_stellt_alles_her` | Ein Preset-Laden erzeugt acht Revisionen (eine je Band); oder Undo bringt nur einen Teil zurück | R12; R11 |

### 3.12 `state_report.dsp` und der Fassungsschritt (Gate: R13, NAK-110, §33.5, §44.4)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-101 | Das Feld wird definiert | Die Reservierung wird **eingelöst**: `state_report.dsp` wandert im Register von den reservierten zu den belegten Feldern, das v3-Schema bekommt die Nutzlast, und der Riegel in `pruefe_v3_vertrag.py`, der heute prüft, dass `state_report` **kein** `dsp` trägt, wird auf die belegte Form umgestellt. `command_ack.applied_dsp` und `state_report.eq_enabled` bleiben **reserviert** und weiterhin ohne Nutzlast. | **BELEGT** · **ENTSCHIEDEN (R13)** · BAULÜCKE | **A5** erweitert, Fälle `dsp_ist_belegt`, `applied_dsp_bleibt_reserviert`, `eq_enabled_bleibt_reserviert` | Der Riegel bleibt auf „abgelehnt" stehen und das Schema wird grün gegen sich selbst; oder ein zweiter reservierter Name wird beiläufig mit eingelöst | R13; `reservierte-nachrichten-v1.json:242`; `pruefe_v3_vertrag.py:1694-1704` |
| M-102 | Der Wire-Envelope-Minor der Familie P1 | Er steigt **genau einmal** von 4 auf **5**. Jede P1-Vertragsänderung dieses Tickets liegt in dieser einen Fassung. Ein zurückgebauter Leser der Fassung 4 **lehnt** das neue Feld ab, statt es still auf einen bekannten Zweig abzubilden — in **beiden** Sprachen, so wie der Fassungsschritt es heute für die Fassungen 1 und 2 misst. | **BELEGT** · **ENTSCHIEDEN (R13)** · BAULÜCKE | **A5** (Fassungsleiter Python), **B3c** (C++), **A4** (Rust) je ein Fall | Der Minor steigt zweimal und es entstehen zwei Zwischenstände mit eigener Leiter; oder ein Fassung-4-Leser nimmt das Feld an | R13; `reservierte-nachrichten-v1.json:6-10`; Muster Fassung 2 und 3 |
| M-103 | Die Nutzlast | `dsp` trägt das bestätigte DTO als **genau die kanonisierte RFC-8785-Zeichenkette, über die der `state_hash` gebildet wurde** (JSON-String-Feld `dsp.jcs`), nicht als eingebettetes JSON-Objekt. Daneben stehen die abgeleiteten Angaben `auto_gain_db`, Klemmliste und verletzte Bänder als **eigene** Felder. **MN1 (B-07):** eine eingebettete DTO-Objektnutzlast wäre am 15-Ziffern-Riegel des Schemawegs gescheitert — `q = 0.7071067811865476` ist auf dem DTO-Weg ausdrücklich zulässig (`eq-copilot/schemas/v3/README.md:132-138`, `eq-copilot/plugin/state/NakamaParameter.cpp:190-197`), und der Broker führt den Textriegel **vor** dem Schema aus (`broker/src/coordinator/schema.rs:383-386`). Ein gültiger bestätigter Zustand wäre damit abgewiesen worden, und Runden hätte den Wert verändert. | **BELEGT** · **ENTSCHIEDEN (R13, B-07-Regel)** · BAULÜCKE · **MN1** | **A5**, **A8** (Fixtures mit `q = 0.7071067811865476`), **B3c**, **A4**; **NEU** **B7**, Sendepfad | Ein Bericht mit `q = 0.7071067811865476` wird abgewiesen, weil die Nutzlast als Objekt durch den 15-Ziffern-Riegel läuft; oder der Sender rundet und der übertragene Wert ist nicht mehr der bestätigte | R13; B-07-Regel; §5.13 |
| M-104 | `v2.global.eq_enabled` und die Nutzlast | Der Schlüssel bleibt im DTO, im `state_hash` **und damit auch in der übertragenen Zeichenkette `dsp.jcs`** — anders wäre `SHA-256(dsp.jcs) == state_hash` nicht prüfbar (M-105). **Es entsteht kein Feld des reservierten Namens `state_report.eq_enabled`** und kein typisiertes Wire-Feld für den Betriebszustand; die Reservierung bleibt für S29–31 unangetastet, und der Riegel des Vertragswerkzeugs hält sie weiter. **MN1 (B-07):** die vorige Fassung nahm den Schlüssel aus der Nutzlast heraus; mit der hashverifizierbaren Zeichenkette ist das nicht mehr möglich, und die Rücknahme steht als Abweichung 3 in §5.17. | **ENTSCHIEDEN (R13, B-07-Regel)** · BAULÜCKE · **MN1** | **A5**, Fall `kein_feld_namens_state_report_eq_enabled`; **B3c** und **A4** je ein Fall; **NEU** **B7**, Sendepfad | Ein Feld `state_report.eq_enabled` oder ein anderes typisiertes Betriebszustandsfeld entsteht und greift S29–31 vor; oder der Sender schneidet den Schlüssel aus der Zeichenkette und der Hash lässt sich nicht mehr nachrechnen | R2 (Name bleibt reserviert); R13; B-07-Regel; `reservierte-nachrichten-v1.json:242` |
| M-105 | Ein Empfänger will den Hash prüfen | Er **rechnet `SHA-256(dsp.jcs)` nach und vergleicht mit `state_hash`**; weichen sie ab, wird der Bericht **ganz** abgewiesen. Beide Leser tun das — C++ und Rust —, und die Zeichenkette wird mit dem **exakten Zahlenleser des DTO-Wegs** gelesen, nicht mit dem binary64-Leser des Schemawegs. **MN1 (B-07):** vorher war die Nutzlast eine Teilmenge und der Hash deshalb ausdrücklich **nicht** nachrechenbar; mit der gehashten Zeichenkette ist die Prüfung nicht nur möglich, sondern Pflicht. | **BELEGT** · **ENTSCHIEDEN (B-07-Regel, §5.13)** · BAULÜCKE · **MN1** | **A4** (Rust) und **B3c** (C++), Fälle `hash_stimmt_mit_der_zeichenkette` und `abweichender_hash_wird_abgewiesen`; **A8** Negativfixture | Ein Bericht mit manipulierter Zeichenkette und altem Hash wird angenommen; oder der Leser liest die Zeichenkette mit dem binary64-Leser und verliert die letzte Ziffer | B-07-Regel; §5.13; §2.12 |
| M-106 | Verbindungsaufbau oder Reconnect | Der **Reconnect beginnt mit `state_report`**. Probeeq sendet den Bericht beim Verbindungsaufbau **und** nach **jeder** Transaktion. Der Broker liest, validiert und hält den bestätigten DSP je Sonde; er **drückt niemals** seinen Cache auf die Sonde. | **BELEGT** · BAULÜCKE | **A4** (Rust), Fälle `reconnect_beginnt_mit_state_report` und `broker_drueckt_nie_seinen_cache`; **B10** für den C++-Sendepfad | Nach einem Reconnect schickt der Broker einen Apply mit seinem Cachestand; oder eine Transaktion erzeugt keinen Bericht und Gen zeigt einen veralteten Stand | §44.4 wörtlich; §33.5; R13 |
| M-107 | Zwei Berichte derselben Verbindung, der zweite mit **kleinerer** Revision | Der Broker behält den **neueren**. Ein verspäteter Bericht überschreibt keinen aktuelleren Stand. Nach einem **neuen** Link gilt der erste Bericht immer als autoritativ, weil die Sonde Eigentümerin ihres Zustands ist und ihre Revision beim Neustart zurückgeht. | **ENTSCHIEDEN (R13-Feinheit, §5.13)** · BAULÜCKE | **A4** (Rust), Fälle `alter_bericht_ueberschreibt_nicht` und `neuer_link_setzt_neu_auf` | Ein verspäteter Bericht setzt den Cache zurück; oder ein Sondenneustart wird als „veraltet" abgelehnt und der Broker hält für immer einen toten Stand | §5.13; §44.4 |
| M-108 | Gens `ControlClient` liest einen Bericht mit `dsp` | Er liest das Feld **tolerant**: ein Bericht **ohne** `dsp` bleibt gültig (Altsender), ein Bericht **mit** `dsp` wird nach den bestehenden Regeln geprüft und bei Verstoß abgewiesen — nicht halb übernommen. Gens Prozessor bleibt sonst unberührt. | **BELEGT** · **ENTSCHIEDEN (R13)** · BAULÜCKE | **B10** erweitert, Fälle `bericht_ohne_dsp_bleibt_gueltig` und `ungueltiges_dsp_wird_ganz_abgewiesen` | Ein Bericht ohne `dsp` wird abgewiesen und Altsonden verlieren die Verbindung; oder ein halb gültiges `dsp` wird teilweise übernommen | R13; Ticketgrenze „außer der Lesetoleranz seiner Vertragsleser" |

### 3.13 `Frame.band_dynamic_gain_db` (Gate: R14, Abnahme 01.09., Blueprint §15 Punkt 15)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-109 | Das Feld wird angelegt | Feld-ID **22** in `table Frame` — die nächste freie; die heutige höchste ist 21. Typ: Vektor aus `float`. `FELD-IDS.json` wird fortgeschrieben, `flatc` regeneriert, und der Drift-Riegel meldet **0**. Keine ID wird verbrannt oder wiederverwendet. | **BELEGT** · **ENTSCHIEDEN (R14)** · BAULÜCKE | **A9** (Drift 0), **A10** (Fixtures), **B3c** und **A4** (beide Leser) | Die ID 22 wird doppelt vergeben; oder der generierte Code weicht vom committeten ab und A9 bleibt trotzdem grün | R14; `nakama_telemetry_v1.fbs:239-319`; `FELD-IDS.json:83-108` |
| M-110 | Ein Frame trägt das Feld | Es hat **genau 8** Einträge in Slot-Reihenfolge 0 bis 7. Jeder Wert ist die **momentane** dynamische Verstärkung dieses Slots in dB, begrenzt auf ±\|dynamic_range_db\|. Freie, ausgeschaltete oder nicht dynamische Slots tragen **exakt 0,0**. Der Wert wird **nie** aus den Einstellwerten erfunden (M-27). | **BELEGT** · **ENTSCHIEDEN (R14)** · BAULÜCKE | **A10** (Fixtures), **B3c**, **A4**; **NEU** **B6**, Quelle des Werts | Der Sender füllt den Vektor mit `dynamic_range_db`; oder ein freier Slot trägt einen Wert ungleich 0 | R14; Fünferblock 01 `:113-127` |
| M-111 | Ein Frame **ohne** das Feld | Bleibt **gültig**. Abwesenheit heißt „der Erzeuger sagt es nicht" und ist ausdrücklich **keine 0** — dieselbe Regel wie bei `integration_samples` (Feld-ID 14). Ein Altsender bleibt damit lesbar. | **BELEGT** · **ENTSCHIEDEN (R14)** · BAULÜCKE | **A10**, **B3c**, **A4** je ein Fall `altsender_ohne_feld_bleibt_gueltig` | Ein Frame ohne das Feld wird abgewiesen und jede alte Sonde fällt aus; oder der Leser liest Abwesenheit als acht Nullen | R14; `nakama_telemetry_v1.fbs:262-275` (Muster ID 14) |
| M-112 | Ein Frame mit einer Länge ungleich 0 und ungleich 8 | Der Leser **weist ihn ab** — in beiden Sprachen, nach den bestehenden Regeln des Rahmens. Eine Länge 3 ist ein Senderfehler, kein Teilbestand. | **BELEGT** · **ENTSCHIEDEN (R14)** · BAULÜCKE | **A10** Negativfixture, **B3c**, **A4** | Ein Vektor der Länge 3 wird als „die ersten drei Bänder" gelesen | R14 |
| M-113 | Ein nicht-endlicher Wert erreicht den Sender | Er wird durch **0** ersetzt und **gezählt**; ein nicht-endlicher Wert verlässt den Sender nie. Der Leser prüft trotzdem auf Endlichkeit und weist einen fremden Frame mit NaN ab. | **BELEGT** · **ENTSCHIEDEN (R14)** · BAULÜCKE | **NEU** **B6**, Fall `nichtendliche_auslenkung_wird_null_und_gezaehlt`; **A10** Negativfixture; **B3c**, **A4** | Ein NaN reist über den Draht; oder der Leser lässt ihn durch, weil er sich auf den Sender verlässt | R14; `CLAUDE.md` „NaN-Ehrlichkeit"; `tools/dirigent/pruefliste.md` C |
| M-114 | Wann das Feld überhaupt reist | **Nur, wenn es etwas zu melden gibt:** mindestens ein Slot ist belegt, eingeschaltet und dynamisch und der EQ-Pfad ist engagiert. Sonst bleibt es abwesend. Damit bleibt der Frame in der Regel unverändert groß, und Abwesenheit bleibt eine ehrliche Aussage über die Messung — nicht über den Sender. Der FlatBuffers-Schema-Minor P2 steigt dafür von 1 auf **2**. | **ENTSCHIEDEN (R14-Feinheit, §5.14)** · BAULÜCKE | **NEU** **B6** und **B10**, Fall `feld_reist_nur_bei_vorhandener_dynamik`; **A10** Fixtures für beide Fälle | Jeder Frame trägt acht Nullen und der Datenstrom wächst ohne Aussage; oder das Feld fehlt, obwohl ein Band sich bewegt | §5.14; `TelemetryClient.h:43` |
| M-121 | Ein Programmwechsel oder ein Remove mit Neubelegung desselben Slots trifft auf **verzögerte** Telemetrie | Jeder Eintrag des Vektors ist dem **Analysefenster** zugeordnet, aus dem er stammt, und trägt die **Generation** des Programms, das ihn erzeugt hat. Wechselt Programm, Belegung oder Slotinhalt — Remove, Neubelegung, Typwechsel, `dynamic_enabled` aus —, wird der gehaltene Wert dieses Slots **verworfen** (0) statt weitergemeldet. Ein Frame trägt damit nie eine Auslenkung, die zu einem Zustand gehört, den es nicht mehr gibt. **MN1 (B-06):** M-109 bis M-114 prüfen Länge, Endlichkeit, Wertequelle und Feldanwesenheit — acht **alte**, endliche Werte bestehen alle diese Prüfungen. Das Gate verlangt ausdrücklich Absicherung „gegen stale sowie nicht-endliche Werte"; `CLAUDE.md` verlangt kohärente Mess-Snapshots. | **BELEGT** (Gate §1.1, Blueprint §15 Punkt 15) · **ENTSCHIEDEN (B-06-Regel)** · BAULÜCKE · **MN1** | **NEU** **B6**, Fälle `programmwechsel_verwirft_die_alte_auslenkung` und `remove_mit_neubelegung_verwirft_die_alte_auslenkung`; **B10** für den Sendepfad | Die alte Auslenkung wird nach dem Programmwechsel erneut gesendet und besteht dabei Länge, Endlichkeit und Anwesenheitsprüfung; oder ein neu belegter Slot erbt den Wert seines Vorgängers | Gate-Text §1.1 (Nachtrag 02.09., „gegen stale … abgesichert"); `CLAUDE.md` „kohärente Mess-Snapshots"; Matrixprüfung 1 B-06 |

### 3.14 Kanon, Budgets und Randbedingungen (Gate: R15, §44.2, §49.3, `tools/dirigent/pruefliste.md`)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-115 | B6 und B7 werden gebaut | Beide entstehen als eigene Ziele **und** ihre `AbPhase` wechselt im Runner von `P6` auf `jetzt` — beides im selben Änderungssatz. Nur so zählt ein fehlendes Binär als `[FEHLT]` statt als `[GEPLANT]`. Die Kanonzahl steigt von **62** auf **64**; kein Bein verschwindet. | **BELEGT** · **ENTSCHIEDEN (R15)** · BAULÜCKE | Der Kanonlauf selbst (Übersicht und Beinzahl) | Die `AbPhase` bleibt auf `P6` und der Kanon meldet weiterhin „2 geplante Prüfung(en) noch nicht gebaut", während das Ticket als fertig gilt | R15; `tools/beweise.ps1:679-680`, `:1093-1107` |
| M-116 | Die Behauptungstexte im Runner | Sie sagen **nur, was gemessen wird** — nicht mehr als das Gate verlangt und nicht mehr, als der Test prüft. Die A16-Behauptung, die heute „heute ebenfalls Passthrough … kein Hostparameter" sagt, wird im selben Änderungssatz nachgezogen; Skriptkopf und Manifestkopf ebenfalls. | **BELEGT** · BAULÜCKE | Ein `git grep` auf den Kernbegriff im Rundenabschnitt; **A16** selbst | Der Runner behauptet „kein Hostparameter", während die Sonde 112 führt | `tools/dirigent/pruefliste.md` E; §2.3 |
| M-117 | Jede neue Prüfung dieses Tickets | Sie wurde **einmal absichtlich gebrochen**, und die Rohausgabe des Rots liegt unter `docs/beweise/roh/` je Matrixzeile. Der Rotbeweis fällt an **der Zeile, die die Zusage trägt** — nicht an einem Nebeneffekt. Eine Probe, die auch ohne den Fix rot ist, heißt Regressionswache, nicht Beleg. | **BELEGT** · BAULÜCKE | Die Rotbeweisdateien selbst, je Zeile eine | Ein Rotbeweis fällt an der Rundungsphase eines Bitvergleichs statt an der zugesagten Eigenschaft | `tools/dirigent/pruefliste.md` E; Auftrag §Regeln |
| M-118 | CPU-Last des schlimmsten Falls | §44.2 nennt einen Worst-Case-CPU-Golden, §49.3 nennt **keinen** Zahlenwert. Es entsteht deshalb **kein erfundener Deckel**: die gemessene Last des Vier-Bank-Falls (Committed plus Candidate, gleichzeitiger Topologiewechsel, acht belegte dynamische Bänder) steht als **Messwert** im Manifest, und der Golden hält nur fest, dass der Fall **läuft** und keine Bank teilt. | **BELEGT** · **ENTSCHIEDEN (R15)** · BAULÜCKE · **MA** (§11) | **NEU** **B6**, Fall `worst_case_vier_baenke_laeuft`; Messwert in §4.6 | Ein Zahlenbudget wird erfunden und der Golden fällt später an einer schnelleren Maschine grundlos grün oder an einer langsameren grundlos rot | R15 wörtlich; §49.3 |
| M-119 | Samplegenaue Automation | Ob kontinuierliche Werte samplegenau oder nur vom vorigen zum letzten Blockwert gerampt werden, entscheidet die **gemessene** Hostbridge-Capability (B3), nicht eine Annahme. Meldet sie `sample_accurate_automation=false`, rampt die Sonde ehrlich blockweise, Topologieautomation wird deaktiviert, und Realtime/Offline-Gleichheit wird nur für denselben Event- und Blockverlauf behauptet. | **BELEGT** · BAULÜCKE | **B3** unverändert; **NEU** **B7**, Fall `capability_entscheidet_den_automationspfad` | Die Sonde behauptet samplegenaue Automation, ohne dass die Capability sie meldet | §44.3 letzter Absatz; Blueprint §3.5 |
| M-120 | Realtime- und Offline-Render | Beide erzeugen **denselben Parameterverlauf** und damit innerhalb numerischer Toleranz denselben Ausgang, bei gleichem Event- und Blockverlauf. Offline-Render läuft mit dem bestätigten Zustand, nie mit einer Vorschau. | **BELEGT** · BAULÜCKE · **MA** (§11) | **NEU** **B6**, Fall `realtime_und_offline_gleich`; **NEU** **B7**, Fall `offline_render_nutzt_den_bestaetigten_zustand` | Ein Offline-Render liefert einen anderen Verlauf, weil die Rampe an der Wanduhr statt an Samples hängt | §44.3; §60 „Offline-Render"; §44.4 |

### 3.15 Zählung nach Belegklasse

Gezählt nach der Nacharbeit 1 der NAK-245-Runde (10.09.2026), aus den Zeilen selbst, nicht abgeschrieben:

| Klasse | Zeilen | Anteil |
|---|---:|---:|
| **BELEGT** (aus verbindlicher Quelle, ganz oder teilweise) | 76 | 60 % |
| **ENTSCHIEDEN** (R1 bis R15, die Regeln B-01 bis B-12, D-1, D-2 und die Protokollspezifikation §5.11.4) | 82 | 65 % |
| davon **allein** durch einen Entscheid getragen | 50 | 40 % |
| **BAULÜCKE** | 126 | 100 % |
| **OFFEN** (Produktfrage) | 0 | 0 % |
| davon mit der Marke **MN1** (Nacharbeit 1) | 15 | 12 % |
| davon mit der Marke **MN2** (Nacharbeit 2) | 6 | 5 % |
| davon mit der Marke **MN3** (Nacharbeit 3) | 6 | 5 % |
| davon mit der Marke **MK** (Konvergenzrunde) | 7 | 6 % |
| davon mit der Marke **MR** (NAK-245-Runde) | 9 | 7 % |
| davon mit der Marke **MR1** (Nacharbeit 1 der NAK-245-Runde) | 2 | 2 % |

Mehrfachmarken sind normal: M-74, M-75, M-76 und M-123 tragen MN1, MN2, MN3, MK und MR; M-124 trägt MN2, MN3, MK, MR und MR1; M-44 trägt MN3, MK, MR und MR1; M-61 trägt MN1 und MR; M-125 trägt MK und MR; M-15 trägt MN1 und MN2; M-126 nur MR. Der Verlauf der Zeilenzahl: **120** nach Etappe 1 (BELEGT 73, ENTSCHIEDEN 72, nur entschieden 47) → **123** nach der Nacharbeit 1 (76 / 78 / 47) → **124** nach der Nacharbeit 2 (76 / 79 / 48) → **124** nach der Nacharbeit 3 (76 / 80 / 48) → **125** nach der Konvergenzrunde (76 / 81 / 49) → **126** nach NAK-245 (76 / 82 / 50) → **126** nach der NAK-245-Nacharbeit 1 (76 / 82 / 50). Die Konvergenzrunde fügte genau eine Zeile hinzu — **M-125**, die die Falltabelle aus §5.11.4 als ganze misst; ohne sie wäre die Tabelle eine Beschreibung und keine Zusage. Die NAK-245-Runde fügt ebenfalls genau eine hinzu — **M-126**, die die Zonenausnahme für Redo und Preset-Laden an T16 bindet; bis dahin trug nur Undo sie als Matrixzeile (M-61). Die NAK-245-Nacharbeit 1 fügt keine Zeile hinzu; sie begrenzt M-44 und M-124 auf bankpflichtige Programme (Marke **MR1**).

Die Prozentsätze summieren sich nicht auf 100, weil eine Zeile beide Klassen
tragen kann: **BELEGT · ENTSCHIEDEN** heißt, dass die Quelle die Zusage
verlangt und der Dirigentenentscheid die technische Ausführung festlegt.

**Null OFFEN-Zeilen ist eine Aussage, keine Bequemlichkeit.** Der Dirigent hat
mit R1 bis R15 vor der Matrix entschieden; jede Technikfrage, die die Quellen
offen ließen, ist damit geschlossen oder liegt als benannte Feinheit in §5. Die
Produktfragen, die dieses Ticket **berührt**, aber nicht beantwortet, stehen in
§5.16 mit ihrer Fläche — sie sind keine Matrixzeilen, weil sie hier nichts
verlangen.

---

## 4. Bauplan für die Etappen 2 bis 4

### 4.0 Zielpfade, die es am Basis-SHA nicht gibt

Die folgenden Pfade sind in den Etappen 2 bis 4 **anzulegen**; sie existieren
zum Stand `e9dbf4b9` nicht, und jede spätere Nennung in diesem Manifest meint
denselben Zielpfad:

- `eq-copilot/schemas/state/nakama-parameter-v2.json` — Layout v2, 120 Kennungen.
- `eq-copilot/schemas/state/nakama-preset-v1.json` — Presetschema, `preset_schema_version` 1.
- `eq-copilot/plugin/dsp/` — der neue DSP-Kern als Bibliothek (Ort und Dateinamen im Detail entscheidet Etappe 3).
- Bein B6, anzulegen als `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`.
- Bein B7, anzulegen als `eq-copilot/plugin/tests/TransactionTestMain.cpp`.
- Je Matrixzeile ein Rotbeweis, anzulegen nach dem Muster `docs/beweise/roh/SONDE-015-rot-M-01.txt`.

### 4.1 Warum die Reihenfolge Verträge → Kern → Integration lautet

Drei Zwänge legen sie fest, und keiner davon ist Geschmack.

**Erstens:** der DSP-Kern braucht die Parameter-IDs, die Zonenform und das
DTO, bevor er ein `DspState` bauen kann. Ein Kern gegen eine geratene
Vertragsform muss zweimal geschrieben werden.

**Zweitens:** die Vertragsarbeit ist die einzige, die **fünf** Sprachen
gleichzeitig berührt (JSON-Schema, C++, Rust, Python-Erzeuger, FlatBuffers).
Prüflistenzeile F verlangt Writer, Leser, Migration, Fixtures und
Cross-Language-Verbraucher im selben Änderungssatz; das geht nur, wenn diese
Arbeit **einen** Block bildet und nicht in drei Etappen zerfällt.

**Drittens:** der Prozessor darf erst angefasst werden, wenn Kern und Verträge
stehen. Solange er unberührt ist, misst A16 den heutigen Passthrough und ist
eine echte Regressionswache. Schreibt Etappe 4 den Prozessor um, wechselt A16
mit — und ab dann gibt es keinen unabhängigen Zeugen mehr dafür, dass der
Passthrough vorher heil war.

### 4.2 Etappe 2 — Verträge in drei Sprachen

**Reihenfolge innerhalb der Etappe** (jede Stufe grün, bevor die nächste beginnt):

1. **Layout v2 und Presetschema.** `nakama-parameter-v2.json` (v1 unverändert
   gelistet plus die elf `v2.*`-Kennungen mit Attribut Host-Parameter
   ja/nein), `nakama-preset-v1.json`, `nakama-state-v2.md` mit dem definierten
   Kind `Dsp` und der berichtigten Kind-Matrix. Beine: **A11**, **A12**.
2. **C++-Spiegel.** `NakamaParameter.h/.cpp` mit den v2-Konstanten und
   `static_assert`; `NakamaState` Writer, Leser und Migration für das Kind
   `Dsp`; Preset-Writer und -Leser. Der Zweig `NakamaState.cpp:1059`, der
   `Dsp` heute ablehnt, wechselt die Seite. Beine: **B2**,
   `EqCopStateMigrationTest`, **A12** für die Goldens.
3. **v3-Vertrag.** `eq-ipc-v3.schema.json` mit der Nutzlast von
   `state_report.dsp`; `reservierte-nachrichten-v1.json` mit dem
   Fassungsschritt P1 4 → 5 und der Umbuchung des Feldes von reserviert nach
   belegt; `pruefe_v3_vertrag.py` mit dem umgestellten Riegel und der neuen
   Fassungsleiter; v3-Fixtures über `erzeuge_v3_fixtures.py`; C++- und
   Rust-Leser mit Fassungsleiter. Beine: **A5**, **A8**, **B3c**, **A4**.
4. **FlatBuffers.** `nakama_telemetry_v1.fbs` mit Feld 22, `FELD-IDS.json`,
   `flatc`-Regeneration, `erzeuge_fb_fixtures.py`, beide Leser,
   `kFeatureBatchSchemaMinor` 1 → 2. Beine: **A9**, **A10**, **B3c**, **A4**.

**Beine dieser Etappe:** A4, A4-SI, A5, A8, A9, A10, A11, A12, B2, B3c,
`EqCopSchemaTest`, `EqCopStateMigrationTest` und `cargo test` der betroffenen
Broker-Crates.

**Matrixzeilen, die hier fallen müssen:** M-14, M-24 (Vertragshälfte), M-28 (Enumgrenze), M-65, M-66, M-72 und M-73 (DTO-Hälfte), M-77, M-78 (Vertragsgrenze), **M-86, M-87, M-89 bis M-94** (die reinen Vertrags- und State-Zeilen), M-95 bis M-99, M-101 bis M-105, M-108 bis M-112.

**MN1 (B-11): M-88 gehört nicht hierher, sondern in die Pflichtmenge der Etappe 4.** Die Zeile fordert **112 tatsächlich exponierte APVTS-Parameter im Sondenprozessor** und deren Reihenfolge. Die APVTS entsteht nach §4.4 erst in Etappe 4, und bis dahin bleibt der Prozessor nach §4.1 ausdrücklich unberührt — Etappe 2 könnte den Nachweis nur führen, indem sie ihre eigene Baugrenze verletzt. Getrennt ist damit: die **Vertragsseite** (welche elf `v2.*`-Kennungen es gibt, welche davon Host-Parameter sind, in welcher Reihenfolge der Vertrag sie listet) liegt in M-86 und M-87 und fällt in Etappe 2; die **Prozessorseite** (was die APVTS wirklich exponiert) liegt in M-88 und fällt in Etappe 4. Ebenso trägt die Prozessorseite von M-89 (Save/Load des Kinds `Dsp` **im Prozessor**) die Etappe 4; Etappe 2 belegt dort Writer, Leser und Migration an `NakamaState` selbst.

**N1 (B-01): Punkt 4 nennt nur den Sender — die Empfängergrenze gehört dazu.**
Die Bauanweisung oben schreibt „`kFeatureBatchSchemaMinor` 1 → 2" und lässt
offen, dass ein Fassungsschritt der Familie P2 erst vollständig ist, wenn
**Sender, Empfänger und der Transport dazwischen** ihn kennen. Gebaut wurde
zunächst nur der Sender; `P2_SCHEMA_MINOR` im Rust-Transport blieb bei 1, und
`verbindung.rs` schloss damit jede Telemetrieverbindung beim **ersten** Frame —
derselbe Vorfall, den der Kommentar in `broker/src/transport/server_v3/mod.rs`
für P0/P1 vom 04.09.2026 festhält. Punkt 4 lautet deshalb vollständig:
`nakama_telemetry_v1.fbs` mit Feld 22, `FELD-IDS.json` **samt der P2-Fassungszahl
als einziger Quelle**, `flatc`-Regeneration, `erzeuge_fb_fixtures.py`, beide
Leser, `kFeatureBatchSchemaMinor` 1 → 2 **und `P2_SCHEMA_MINOR` 1 → 2 mit den
Randwerten der Fassungsleiter**. Beine unverändert: **A9**, **A10**, **B3c**,
**A4**.

**Riegel dieser Etappe.** Drei Prüfwerkzeuge behaupten heute das Gegenteil
dessen, was gebaut wird, und müssen im selben Änderungssatz mitwandern, sonst
ist der Vertrag grün gegen sich selbst:
`pruefe_v3_vertrag.py:1694-1704` („reservierte Felder sind im aktiven Vertrag
weiter abgelehnt"), `:1609-1623` (die exakte Menge der reservierten
Feldnamen) und die Fixture `dto/ungueltig/dsp-schema-version-2.json`.

### 4.3 Etappe 3 — DSP-Kern als Bibliothek mit Goldens

Ein neues Modul unter `eq-copilot/plugin/dsp/`, **ohne** den Prozessor zu
berühren. Inhalt: `DspState`, `DspProgram`, `DspBankPool`, die sechs
Filtertypen, die Hüllkurven, die M/S-Matrix, Trims, Mix, die
Auto-Gain-Ableitung, der Hard-Bypass, die Hörmatrix, die drei Taps, die Quelle
für `band_dynamic_gain_db`, der Nicht-Endlich-Riegel mit seinen zwei Zählern
und die Allokations- und Sperrzähler für Tests.

**Warum als Bibliothek und nicht im Prozessor:** der Kern muss vier Bänke,
`busy_retry`, Reclaim-Overflow und den NaN-Riegel deterministisch
herbeiführbar prüfen können. Im Prozessor sind diese Zustände nur über echtes
Audio und echte Threads erreichbar; als Bibliothek sind sie ansteuerbar. Das
ist dasselbe Zweiebenenmuster, das B4 und B5 schon tragen: die Einheit pur
**und** derselbe Weg verdrahtet.

**Bein: B6 `EqCopDspGoldenTest`**, im Runner scharf. Es misst Filtergoldens
gegen die analytische Antwort (M-10 bis M-13), Bypass-Bitidentität (M-02,
M-05), Zipper (M-17), Crossfades (M-03, M-04, M-06, M-25, M-29),
Auto-Gain-Referenzkurven (M-36 bis M-40), Mix (M-32 bis M-34), Hörmatrix-Null
(M-53 bis M-57), den NaN-Riegel (M-49, M-50) und den Bankpool-Ownership-Automaten
(M-41 bis M-46).

**Landmine, die hier zuschlägt:** der MSVC-Standardstack ist 1 MiB. `DspProgram`
mit acht Bändern in `double`, mal vier Bänken, mal zwei Pfaden ist keine
Rahmengröße. Jede Testfunktion legt Pool und Programme auf den **Heap**
(`std::unique_ptr`); der Fund zeigt sich sonst erst im vollen Kanon, während
das Bein allein grün läuft (Register NAK-175).

### 4.4 Etappe 4 — Integration, Transaktion, Telemetrie, Nulltest

APVTS in `SondeProcessor` (112 Host-Parameter nach M-88), `CommittedState`,
`PreviewOverlay`, `AutomationOverlay`, die sieben Transaktionen, `state_revision`
und `state_hash`, der persistente Undo-Ring, die Preset-API, der
`state_report.dsp`-Sender in Probeeqs `ControlClient`, das Halten im Broker,
Gens Lesetoleranz, `band_dynamic_gain_db` im `TelemetryClient` und Save/Load des
Kinds `Dsp` im Prozessor.

**Bein: B7 `EqCopTransactionTest`**, im Runner scharf: Transaktionen und
Revisionen (M-74 bis M-85, M-123 bis M-125), Belegung und Remove/Undo (M-58 bis M-65),
Zonen (M-66 bis M-73, M-126), Preset (M-95 bis M-100).

**MN1 (B-11): diese Etappe trägt zusätzlich die Prozessorseite des Layouts.**
**M-88** (112 tatsächlich exponierte APVTS-Parameter und ihre Reihenfolge) und
die Prozessorhälfte von **M-89** (Save/Load des Kinds `Dsp` im Prozessor)
fallen hier, nicht in Etappe 2 — dort ist der Prozessor nach §4.1 unberührt.
Ebenso hier: **M-106** und **M-107** (Sendezeitpunkt und Halten des Berichts),
**M-113** und **M-114** (Senderseite des Telemetriefeldes) sowie **M-121** (das
Verwerfen veralteter Auslenkungen am Sendepfad).

**Bein A16 `EqCopProbeeqNullTest`, umgeschrieben.** Es misst danach: Default-Stand
bitidentisch über 1000 Blöcke bei vier Sampleraten (M-01), **`eq_enabled` true mit
`bypass` false und sonst neutralen Defaults bitidentisch** (M-02 — der Fall, den
MN1/B-08 eingesetzt hat; er ist der einzige, der NAK-81 (b) wirklich einlöst),
0 Latenz und kein Tail im Passthrough (M-51), Save/Load/Save bytegleich in v2
(M-93), `eq_enabled` true mit `bypass` true bitidentisch (M-05), nach
`eq_enabled` false wieder bitidentisch nach dem Fade (M-04), Mix 0 bitidentisch
(M-33), Passthrough sanitisiert nicht (M-50). Die alte Behauptung „kein
Hostparameter" fällt (M-116).

Danach der **volle Kanon, abgekoppelt** nach dem Muster im Auftrag; er wird nie
als sitzungsgebundener Hintergrundbefehl gestartet, und während er läuft kommt
nichts in den Worktree.

### 4.5 Ticketpfade der Etappen 2 bis 4 (Prüfbereich)

**Verträge und Fixtures:** `eq-copilot/schemas/state/` (Parameter v2, Preset v1,
`nakama-state-v2.md`), `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`,
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`,
`eq-copilot/schemas/v3/README.md`,
`eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs`,
`eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json`,
`eq-copilot/plugin/vertrag/generiert/`, `eq-copilot/fixtures/state/`,
`eq-copilot/fixtures/v3/`.

**Plugin:** `eq-copilot/plugin/dsp/` (neu), `eq-copilot/plugin/state/`
(`NakamaParameter`, `NakamaState`), `eq-copilot/plugin/sonde/SondeProcessor.*`,
`eq-copilot/plugin/core/ipc/` (`ControlClient`, `controlclient/`,
`TelemetryClient`), `eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp`,
`eq-copilot/plugin/CMakeLists.txt`.

**Plugin-Tests:** `eq-copilot/plugin/tests/` (B6, B7 neu; `SondeNullTestMain.cpp`,
`StateMigrationTestMain.cpp`, `SchemaTestMain.cpp`, `IpcTestMain.cpp` erweitert).

**Broker:** `broker/src/coordinator/liveness.rs` (`state_report`-Leser),
`broker/src/coordinator/schema.rs` (**MN1, B-12** — die Fassungsleiter
`v3_schema(schema_minor)` trägt heute `MINOR_0` bis `MINOR_4` und
`JSON_SCHEMA_MINOR_AKTIV` = 4; Minor 5 aus M-102 braucht dort einen eigenen
Zweig, sonst lehnt der Broker die neue Fassung ab),
`broker/src/generiert/nakama_telemetry_v1_generated.rs` (**MN1, B-12** — das
Rust-Codegen-Ziel aus `eq-copilot/schemas/v3/flatbuffers/WERKZEUG.json`, ohne
das A9 für Feld 22 nicht driftfrei sein kann),
`broker/src/telemetrie.rs`, `broker/src/vertrag.rs`, `broker/tests/`.

**Werkzeug:** `tools/beweise.ps1`, `tools/eq-copilot/pruefe_v3_vertrag.py`,
`tools/eq-copilot/erzeuge_v3_fixtures.py`,
`tools/eq-copilot/erzeuge_fb_fixtures.py`,
`tools/eq-copilot/erzeuge_state_fixtures.py`,
`tools/eq-copilot/pruefe_flatc_drift.py`,
`tools/eq-copilot/pruefe_v2_schemas.py` (**MN1, B-12** — nur falls die
Entscheidung aus M-95 dieses Bein auf `schemas/state/` erweitert; heute liest
es eine eingefrorene Familie von fünf Dateien unter `eq-copilot/schemas/` und
sieht `schemas/state/` nicht).

**Nicht angefasst:** `eq-copilot/plugin/src/` (Gens Prozessor) außer der
Lesetoleranz seiner Vertragsleser, `eq-copilot/identity/`,
`eq-copilot/install/`, `eq-copilot/plugin/probe/`,
`eq-copilot/plugin/hostbridge/`, `design/`, `docs/offene-punkte.md`,
`docs/PLAN-STAND.md`, `docs/plan/` (außer dem Feld `beleg` des Schritts).

### 4.6 Was im Manifest gemessen, nicht behauptet wird

| Zahl | Quelle beim Abschluss |
|---|---|
| Kanonbeine | Übersicht des Abschlusslaufs, nicht aus §2.10 abgeschrieben |
| Prüfungen je Bein (B6, B7, B2, B3c, B10, A5, A8, A10, A12, A4) | Rohausgabe des jeweiligen Laufs |
| CPU-Last des Vier-Bank-Falls | Messwert aus B6 (M-118), ohne erfundenen Deckel |
| Bytegröße eines vollen `Dsp`-Kinds mit 32 Undo-Einträgen | Messung am Writer, gegen die 16-MiB- und Knotengrenze gehalten |
| Zahl der Rotbeweise | `ls docs/beweise/roh/SONDE-015-rot-*` |

### 4.7 Risiken dieser Bauarbeit, benannt

1. **Der Kanon wird grün gegen sich selbst.** Drei Riegel behaupten heute das
   Gegenteil des Ziels (§4.2). Wird einer vergessen, bleibt der Lauf grün und
   der Vertrag ist trotzdem gebrochen. Gegenmittel: die drei Stellen stehen
   namentlich im Bauplan und jede bekommt einen eigenen Rotbeweis.
2. **Der Rotbeweis fällt am falschen Ort.** Bei Audio ist das besonders leicht:
   ein Bitvergleich fällt schon an der Rundungsphase, und ein Golden gegen die
   eigenen Koeffizienten fällt nie. Gegenmittel: M-13 macht die Messart selbst
   zur Zusage.
3. **Der Stack reißt im vollen Kanon.** NAK-175, siehe §4.3.
4. **`Copy-Item` überträgt `LastWriteTime`.** Eine zurückgespielte Quelle kann
   älter sein als ihr Objektfile; MSBuild übersetzt nicht neu, meldet Exit 0,
   und der Lauf misst das alte Binär (NAK-230). Gegenmittel: vor dem Neubau
   den Zeitstempel setzen und den des Binärs gegen die Quelle prüfen.
5. **`cmake` liegt nicht im Bash-`PATH`.** C++-Beine aus pwsh bauen; ein
   `cmake --build` aus Bash endet still in `command not found`, und das danach
   gefahrene Bein misst ein altes Binär.
6. **Zwei Schema-Minor gleichzeitig.** P1 (JSON) steigt auf 5, P2
   (FlatBuffers) auf 2. Werden sie verwechselt, entsteht eine Fassungsleiter,
   die auf die falsche Fläche zeigt. §2.12 hält die beiden auseinander.

### 4.8 Regeln für jede neue Prüfung dieser Etappen

- Jede neue Prüfung wurde **einmal absichtlich gebrochen**; die Rohausgabe des
  Rots liegt unter `docs/beweise/roh/` und trägt die Matrix-ID im Namen.
- Der Rotbeweis fällt an der **Zeile, die die Zusage trägt**, nicht an einem
  Nebeneffekt (M-117).
- Eine Behauptung im Runner sagt **nicht mehr, als der Test misst** und nicht
  mehr, als das Gate verlangt (M-116).
- Zahlen im Manifest sind **gemessen**, nicht abgeschrieben (§4.6).
- Ein Probe-Datensatz „in der Form des Writers" wird **vom Writer erzeugt**;
  synthetische Fälle sind deklarierte Mutanten genau eines Writer-Fixtures
  (M-94).
- Speichern↔Laden, Apply↔Revert, Remove↔Undo, Verbinden↔`state_report`,
  Aktivieren↔Abklingen liegen jeweils im **selben** Änderungssatz.
---

## 5. Entscheide R1 bis R15 und ihre Feinheiten

R1 bis R15 sind **Technikentscheide des Dirigenten** vom 09.09.2026, wörtlich
in `docs/beweise/roh/SONDE-015-auftrag.txt`. Jeder Abschnitt zitiert die Regel
vollständig und stellt darunter die Feinheiten, die sie offen lässt und die
diese Matrix festlegt. Eine Feinheit ist **Teil** der Regel, kein neuer
Vorschlag: sie füllt eine Lücke, die der Bau sonst stillschweigend füllen
würde.

Sieben Stellen weichen vom Wortlaut einer Regel oder von der gewählten Form ab
— **zwei davon sind zurückgenommen**: Nummer 3 in der Matrix-Nacharbeit 1,
Nummer 6 in der Matrix-Nacharbeit 3. Sie stehen jeweils unter ihrer Regel
**und** gesammelt in §5.17.


### 5.1 R1 — Layout v2 ist additiv und versioniert

**Der Entscheid, wörtlich:**

> R1 — Layout v2 ist additiv und versioniert. `dsp_schema_version` wird 2.
> Alle 109 v1-Kennungen bleiben unverändert (ID, Typ, Grenzen, Default,
> Wechselart). Neue persistente Werte tragen das Präfix `v2.*`
> (nakama-parameter-v1.json:8), stehen in einer neuen Datei
> eq-copilot/schemas/state/nakama-parameter-v2.json (v1-Datei bleibt
> eingefroren als Verlauf; v2 listet v1 unverändert plus die Erweiterung) und
> in NakamaParameter.h mit static_assert. Jede v2-Kennung trägt ein Attribut,
> ob sie Host-Parameter ist (APVTS/FL-Automation) oder nur persistenter
> Zustand im Kind `Dsp`. Ein v1-Stand (dsp_schema_version 1) lädt verlustfrei
> mit Defaults für v2; ein v2-Stand mit unbekannten v2-Feldern zerstört keinen
> alten Consumer (Leser ignoriert unbekannte Felder im Dsp-Kind, das DTO
> selbst bleibt exakt geprüft); unbekannte Major bleibt read-only mit
> Originalbytes wie heute (NakamaState.cpp:1635-1705). Die Fixture
> dto/ungueltig/dsp-schema-version-2.json wechselt die Seite — der Korpus wird
> vom Writer neu erzeugt (Prüfliste E, Writer-Fixtures statt Handschrift), nie
> von Hand editiert.

**Feinheiten.**

1. **Die elf v2-Kennungen und ihr Ort.** `v2.global.eq_enabled` (bool, Default false, blockrand, **Host-Parameter**), `v2.global.mix` (float 0,0–1,0, Default 1,0, rampe, **Host-Parameter**), `v2.global.auto_gain` (bool, Default false, blockrand, **Host-Parameter**) und acht `v2.band.<slot>.occupied` (bool, Default false, **kein** Host-Parameter, persistenter Zustand im Kind `Dsp`). Layout v2 zählt damit **120** Kennungen, die APVTS **112** Host-Parameter.
2. **Die v2-Host-Parameter hängen hinten an.** Reihenfolge in der APVTS: die 109 v1-Kennungen in Vertragsreihenfolge, danach die drei v2-Host-Parameter. Grund: ein späteres Layout v3 kann dann wieder anhängen, ohne einen bestehenden Index zu verschieben. Probeeq führt heute **null** Host-Parameter (`SondeProcessor.h:40-45`), es verschiebt sich also kein Index und die eingefrorene Identität bleibt unberührt (NAK-30).
3. **`occupied` ist bewusst kein Host-Parameter.** Wäre es automatisierbar, könnte eine Automationskurve einen Slot belegen oder freigeben — ohne Transaktion, ohne Revision und ohne Undo-Objekt. Genau das würde R5 brechen: Remove ist **eine** Transaktion mit **einem** Undo-Objekt, und ein Weg daran vorbei macht die Zusage wertlos.
4. **Abweichung 1 (§5.17): das Kind `Dsp` trägt keinen zweiten Wertesatz.** §53.8 beschreibt `Dsp` als „Revision, Schutz, bestätigter DspState, Undo-Ring". Gebaut wird `Dsp` mit `state_revision`, den acht `occupied`, der Zonenliste und dem Undo-Ring — **ohne** eine zweite Kopie der 112 Werte. Der bestätigte DspState ist die Vereinigung aus `Parameters` (die Werte) und `Dsp` (Belegung und Zonen). Grund: zwei Kopien derselben Werte können auseinanderlaufen, und §33.5 verbietet ausdrücklich eine zweite Wahrheit. Ein Leser, der beides liest, müsste bei Widerspruch raten.
5. **Flache Arrays, keine Kindknoten.** Der State-Leser erlaubt höchstens 64 `ValueTree`-Knoten im ganzen Baum (`nakama-state-v2.md:188-191`). Acht Zonen und 32 Undo-Einträge als Knoten wären allein 40. `Dsp` führt seine Listen deshalb als flache Arrays, wie `MainProject` es seit SONDE-012 tut.
6. **Der Undo-Eintrag ist ein voller Schnappschuss, kein Delta.** Jeder Eintrag trägt einen Kopf (Art, betroffene Slot-ID oder −1, Revision) und danach den vollständigen DTO-Zustand (120 Werte plus Zonenliste) als flaches Array. Grund: ein Deltaformat braucht je Handlungsart einen eigenen Decoder, und ein falscher Decoder erzeugt lautlos einen halben Zustand. Mit einem einheitlichen Schnappschuss ist „Undo stellt es als EIN Objekt in EINER Transaktion wieder her" (R5) durch die Konstruktion wahr. Die Kosten sind begrenzt und werden gemessen (§4.6): 32 mal rund 120 Werte liegen weit unter den 65.536 Einträgen je Sammlung und den 16 MiB.
7. **Bitgenauer Defaultvergleich in der Migration.** „Weicht vom v1-Default ab" (R5) wird bitgenau geprüft, nicht mit Epsilon. Ein Epsilonvergleich würde einen minimal verstellten Wert als unberührt lesen und ein Band verlieren.


### 5.2 R2 — Die EQ-Rolle ist ein eigener Schalter und standardmäßig aus

**Der Entscheid, wörtlich:**

> R2 — Die EQ-Rolle ist ein eigener Schalter und standardmäßig aus.
> `v2.global.eq_enabled` (bool, Default false, blockrand, Host-Parameter) ist
> der lokale persistente Rollenschalter aus CLAUDE.md („Wird ihr EQ
> zugeschaltet …“). false = der Audiopfad ist exakt der heutige Passthrough:
> bitidentisch, 0 Samples Latenz, kein Tail, keine Bankaktivität (Grundgesetz
> „nichts Ungefragtes“). true = der Kern ist engagiert; darin ist
> `v1.global.bypass` der Hard-Bypass vor M/S und Filterbank (bitgenau,
> §44.2:3008-3010, Gate 1). Ein Wechsel true→false ist klickfrei; nach dem
> Blockrand-Crossfade ist der Passthrough wieder bitidentisch — die Fadelänge
> nimmst du aus dem Entwurf, sonst dokumentierst du sie als Technikkonstante
> mit Test. Das Wire-Feld `state_report.eq_enabled` bleibt reserviert
> (Eigentümer S29–31); hier reist nichts davon.

**Feinheiten.**

1. **`kFadeSamples` = 256 Samples, samplerateunabhängig gezählt.** Der Entwurf sagt „kurzem Crossfade", nennt aber keine Länge, und §49.3 trägt kein Fadebudget. Der Wert ist damit eine **Technikkonstante mit Test** (R2 verlangt genau das). 256 Samples sind 5,8 ms bei 44,1 kHz und 1,3 ms bei 192 kHz — lang genug gegen einen Knacks beim Zuschalten einer vollen Bank, kurz genug, dass ein Bypass sich nicht zäh anfühlt. Gezählt wird in **Samples**, nicht in Blöcken: sonst dauerte derselbe Fade bei Blockgröße 1 und bei 2048 verschieden lang, und M-03 fiele an der Blockgröße statt an der Zusage.
2. **Linearer Crossfade, nicht equal-power.** Dry und Wet sind hochkorreliert; eine Wurzelkennlinie erzeugte in der Mitte bis zu +3 dB. Dieselbe Begründung wie bei Mix (§5.3).
3. **Nach dem Fade ist es wieder ein echter Kurzschluss.** Der Fadezähler erreicht 0, und danach schreibt der Passthrough keine Samples mehr — er multipliziert nicht mit 1,0. Das ist der Unterschied zwischen „bitidentisch" und „numerisch nahe" und der Grund, warum M-04 den Beweis in zwei Teile trennt.
4. **Ausgeschaltet heißt kalt.** Bei `eq_enabled` false ist keine Bank `audio_active`, kein Filterzustand wird fortgeschrieben und kein Detektor läuft; beim Einschalten starten alle Zustände auf 0 (M-07). Das folgt aus R2 („keine Bankaktivität") und aus dem Grundgesetz: ein Pfad, der im ausgeschalteten Zustand rechnet, verbraucht ungefragt Rechenzeit.
5. **`eq_enabled` ist im DTO und im Hash — und reist seit MN1 innerhalb der gehashten Zeichenkette mit.** Es ist ein persistenter Host-Parameter wie `bypass` und gehört damit in den bestätigten Zustand; ohne ihn im Hash könnten zwei Stände mit hörbar verschiedenem Ausgang denselben Hash melden und §44.5 („Projekt-Reload rekonstruiert denselben State-Hash und … denselben Audioausgang") wäre verletzt. **Weil `state_report.dsp` seit B-07 genau den gehashten Text trägt (§5.13), ist der Schlüssel damit auch auf dem Draht.** Unangetastet bleibt, worum es R2 geht: es entsteht **kein Feld des Namens `state_report.eq_enabled`** und kein typisiertes Betriebszustandsfeld — S29–31 bleibt frei, den reservierten Namen als das zu definieren, was ein Leser auswerten kann. Die frühere Nutzlast-Teilmenge ist als Abweichung 3 in §5.17 **zurückgenommen**.


### 5.3 R3 — Mix je Sonde

**Der Entscheid, wörtlich:**

> R3 — Mix je Sonde: `v2.global.mix` (float, 0.0–1.0 als Verhältnis wie width,
> Default 1.0, rampe, nicht topologisch, Host-Parameter). Dry ist das Signal
> am Tap pre_nakama, Wet der verarbeitete Pfad; die Position von Mix gegenüber
> Output-Trim und Auto-Gain legt die Matrix fest und der Golden friert sie
> ein. Der globale Mix als zweite Schicht (Interview :263) ist Gen-Sache
> (S28b/S31b), nicht hier.

**Feinheiten.**

1. **Die Kettenreihenfolge, eingefroren.** Eingang → Tap `pre_nakama` → Input-Trim → M/S-Hinmatrix → acht Bänder → M/S-Rückmatrix → **Auto-Gain** → **Mix** → **Output-Trim** → Tap `post_committed` → Hörmatrix → Ausgang (§3.0).
   **Innerhalb der Bänder ist die Richtung Teil der Zusage (MN2, B-09):** die acht Slots wirken als **Kaskade von Slot 0 nach Slot 7**, nie rückwärts und nie parallel summiert. Die Richtung ist kein Implementierungsdetail: bei nicht kommutierenden Kombinationen — ein dynamisches Band vor einem statischen, oder zwei Bänder mit verschiedenem `channel_mode` — klingt 7 → 0 messbar anders als 0 → 7, und die Slotnummer ist zugleich die feste, im Vertrag sichtbare Kennung, an der eine spätere Oberfläche das Band wiederfindet. Belegt wird sie deshalb gegen ein **unabhängig gerechnetes, geordnetes Referenzergebnis** (M-15) und nicht dadurch, dass zwei Anordnungen verschiedene Ausgaben liefern — das täte eine rückwärts laufende Bank auch.
2. **Warum Auto-Gain **vor** Mix liegt.** Der Ausgleich gehört zur Kurve. Steht Mix auf 0,5, wirkt die Kurve zur Hälfte — und damit auch ihr Ausgleich zur Hälfte, weil er im Wet-Zweig sitzt. Läge Auto-Gain hinter Mix, würde er bei Mix 0,5 doppelt so stark ausgleichen wie die Kurve anhebt, und bei Mix 0 würde er ein Signal korrigieren, das gar nicht bearbeitet wurde.
3. **Warum Output-Trim **hinter** Mix liegt.** Der Output-Trim ist der Ausgangsregler des Users, kein Teil der Kurve. Läge er im Wet-Zweig, wäre er bei Mix 0 wirkungslos — ein Regler, der je nach Stellung eines anderen Reglers nichts tut, ist ein totes Element. Der Vertragstext „Ausgangs-Trim nach der Filterbank" (`nakama-parameter-v1.json:25`) ist mit beiden Lagen vereinbar; diese ist die ehrliche.
4. **Linearer Mix, keine equal-power-Kennlinie.** `aus = mix · wet + (1 − mix) · dry`. Bei hochkorrelierten Zweigen — und das sind Dry und Wet eines EQ — erzeugt eine Wurzelkennlinie in der Mitte bis zu +3 dB Überhöhung. Der Rotbeweis zu M-34 misst genau diesen Fall.
5. **Unity-Kurzschluss an beiden Enden.** Bei `mix` = 1,0 wird der Dry-Zweig nicht gerechnet, bei 0,0 nicht der Wet-Zweig. Das ist nicht nur schneller: `0 · wet` würde ein NaN oder ein `-0.0` aus dem Wet-Zweig in den Ausgang tragen, und die Bitidentität aus M-33 wäre keine.
6. **Der globale Mix bleibt draußen.** Interview `:263` („zweite Schicht über dem Sonden-Mix") gehört Gen (S28b, S31b), nicht diesem Ticket. Hier entsteht **nur** der Sonden-Mix.


### 5.4 R4 — Auto-Gain

**Der Entscheid, wörtlich:**

> R4 — Auto-Gain: `v2.global.auto_gain` (bool, Default false, blockrand, nicht
> topologisch, Host-Parameter). Der Ausgleichsbetrag ist KEIN persistenter
> Parameter und nicht Teil des state_hash: er wird deterministisch aus dem
> bestätigten Programm abgeleitet (Referenz: pink-gewichtete mittlere
> Betragsantwort der statischen Kurve über ein logarithmisches Gitter 20 Hz–20
> kHz, d. h. gleiche Gewichtung je Oktave; flache Kurve → exakt 0 dB; ein
> +6-dB-Shelf, der das ganze Gitter überstreicht → −6 dB ±0,1 dB; Details und
> Gitterdichte legt die Matrix fest, der Golden friert sie ein). Er wird beim
> Vorbereiten eines Programms außerhalb des Audiothreads gerechnet und im
> Audiothread über denselben Rampenweg wie Output-Trim angewandt. Der
> abgeleitete Wert in dB ist lesbar (Prozessor-Getter und Feld im
> state_report.dsp als „abgeleitet“), damit S31b den Output-Wert sichtbar
> mitwandern lassen kann; Auto-Gain ersetzt Output-Trim nicht (Abnahme 24.08.
> :55). Dynamische Bandanteile gehen nicht in die Ableitung ein.

**Feinheiten.**

1. **Das Gitter.** 20 Hz bis 20 kHz in **1/12-Oktav-Schritten**, also **121** Stellen (log₂(1000) ≈ 9,966 Oktaven, aufgerundet auf 120 Intervalle). Das Gitter ist log-gleichverteilt, und alle Stellen haben dasselbe Gewicht — das **ist** die gleiche Gewichtung je Oktave, die R4 verlangt. Es ist samplerateunabhängig definiert; da 20 kHz bei jeder unterstützten Rate unter der Nyquistgrenze liegt (kleinste Rate 44,1 kHz), braucht es keine Kappung. Der ausgewertete Frequenzgang hängt trotzdem von der Samplerate ab, weil ein digitaler Biquad das tut — der Golden friert ihn je Rate ein.
2. **Abweichung 5 (§5.17): Energiemittel, nicht Mittel der Beträge.** R4 sagt „mittlere Betragsantwort“. Gerechnet wird `g_db = −10 · log₁₀( (1/N) · Σₖ E(fₖ) )` mit `E = \|H\|²`, also das Mittel der **Energie**, nicht der Beträge. Beide von R4 genannten Proben halten in beiden Lesarten exakt (flache Kurve → 0 dB, +6-dB-Shelf über dem ganzen Gitter → −6 dB); sie unterscheiden sich erst bei schmalen Merkmalen. Ein +12-dB-Bell mit Q 12 ergibt im Betragsmittel −0,28 dB, im Energiemittel −0,95 dB. Wahrgenommene Lautheit folgt der Energie in den Frequenzgruppen, nicht dem Betrag; das Betragsmittel unterkompensierte eine schmale Anhebung hörbar. Das Energiemittel entspricht dem, was Pro-Q als „an educated guess based on the current EQ settings“ beschreibt (zitiert in NAK-81 a).
3. **Kanalfaltung — Mid und Side sind getrennte Komponenten (MN1, B-04).** Die je Seite wirksame Übertragung ist `H_L(f) = H_stereo(f) · H_left(f) · (H_M(f) + H_S(f))/2` und `H_R(f) = H_stereo(f) · H_right(f) · (H_M(f) + H_S(f))/2`, wobei `H_stereo`, `H_left`, `H_right`, `H_M` und `H_S` jeweils das Produkt der Bänder mit dem entsprechenden `channel_mode` sind; danach `E(f) = ½ · (\|H_L(f)\|² + \|H_R(f)\|²)`. Der Term `(H_M + H_S)/2` ist der **Diagonalterm** der M/S-Matrix: aus `M = (L+R)/2`, `S = (L−R)/2`, `L' = H_M·M + H_S·S` und `R' = H_M·M − H_S·S` folgt `L' = L·(H_M+H_S)/2 + R·(H_M−H_S)/2`. Damit fällt ein **identisches** Mid/Side-Paar `H_M = H_S = H` exakt auf den Stereo-Fall zusammen (`L' = H·L`, `R' = H·R`) — genau das, was M-37 als Golden fordert.
   **Was die Näherung auslässt und warum.** Der Kreuzterm `(H_M − H_S)/2` geht nicht ein. Er ist null für das identische Paar und für jedes rein stereo-/L/R-wirkende Programm, also in allen Fällen, die der Referenzkorpus als exakt zusagt. Er ist ungleich null, wenn ein Band **nur** auf Mid oder **nur** auf Side wirkt: ein +6-dB-Shelf nur auf Mid ergibt mit dem Diagonalterm +3,52 dB, bei korreliertem Material wären es +6,00 dB und bei dekorreliertem +3,98 dB. Ihn exakt zu behandeln hieße, zwischen diesen beiden Lesarten zu wählen — **das** ist die Materialannahme, die R4 ausschließt („nicht a dynamic process based on actually measured levels", NAK-81 a). Der Diagonalterm liegt zwischen beiden und ist materialfrei.
   **MN1 (B-04):** die vorige Fassung multiplizierte `H_M` und `H_S` in **beide** Seiten. Zwei identische +6-dB-Shelves auf `mid` und `side` hätten im Plateau `\|H\|⁴` ergeben und wären wie **+12 dB** kompensiert worden — nicht der Pegelgewinn der aktuellen Kurve, den R4 und die Abnahme vom 24.08. `:54` verlangen.
4. **Die beiden genannten Proben halten exakt.** Flache Kurve: ein Bell mit `gain_db` = 0 hat in RBJ `b0 = a0`, `b1 = a1`, `b2 = a2`, also `H ≡ 1` **bitgenau**; damit ist `E ≡ 1,0` und `log₁₀(1,0) = 0,0` exakt. Ein +6,0-dB-Shelf über dem ganzen Gitter: `\|H\| = 10^(6/20)`, `E = 10^(6/10)`, `−10 · log₁₀(E) = −6,000 dB` — innerhalb der geforderten ±0,1 dB.
5. **Was **nicht** eingeht.** Der dynamische Anteil (R4 wörtlich), Input- und Output-Trim (Pegel, keine Kurve), Width und Mono-Bass (Stereofeld, kein Betragsgang), Mix (der Ausgleich sitzt im Wet-Zweig, §5.3) und freie oder ausgeschaltete Slots.
6. **Der abgeleitete Wert wird immer gerechnet, auch bei ausgeschaltetem Schalter.** Angewandt wird er nur bei `auto_gain` = true; gerechnet und lesbar ist er immer. Nur so kann S31b später zeigen, was Auto-Gain **täte**, ohne dass etwas geschieht — und nur so ist die Abnahme vom 24.08. („der angezeigte Output-Wert wandert sichtbar mit") ohne einen zweiten Rechenweg erfüllbar.
7. **Er ist kein Parameter und nicht im Hash.** Zwei Stände mit identischem DTO haben denselben Hash, auch wenn ihr abgeleiteter Wert sich unterscheidet — was er nicht kann, weil er aus dem DTO folgt. Genau deshalb braucht er kein Feld: ein abgeleiteter Wert im Hash wäre eine zweite Wahrheit, die driften kann.


### 5.5 R5 — Band-Belegung und Remove

**Der Entscheid, wörtlich:**

> R5 — Band-Belegung und Remove: `v2.band.<slot>.occupied` (bool, Default
> false, kein Host-Parameter, persistenter Zustand im Kind `Dsp`) ist der
> Belegungszustand; `v1.band.<slot>.enabled` bleibt der reversible Bypass. Ein
> freier Slot (occupied=false) verarbeitet nichts, gleichgültig was seine
> Werte sagen; ein belegter, ausgeschalteter Slot behält Slot-ID, Werte und
> Band-ID. Remove ist EINE Transaktion (eine state_revision): occupied=false
> und alle 13 v1-Werte des Slots auf Default; andere Slot-IDs werden nie
> umnummeriert. Das Undo-Objekt ist ein Eintrag des Undo-Rings mit Slot-ID,
> Typ, channel_mode, Dynamic-Zustand und sämtlichen Werten; Undo stellt es als
> EIN Objekt in EINER Transaktion wieder her. Host-Automation auf einen freien
> Slot ändert Werte, aber weder Belegung noch Klang (gemessen). Migration
> v1→v2 (Lücke, Dirigent-Entscheid): occupied := enabled ODER mindestens ein
> Wert des Slots weicht vom v1-Default ab — ein v1-Stand verliert damit kein
> vom User gesetztes Band.

**Feinheiten.**

1. **Was „verarbeitet nichts" heißt.** Ein freier Slot liefert keinen Filter, keinen Detektor, keinen Beitrag zur Auto-Gain-Ableitung und keinen Wert im dynamischen Telemetrievektor (dort exakt 0,0). Er kostet auch keine Rechenzeit: das Programm enthält ihn nicht.
2. **Remove setzt alle dreizehn v1-Werte des Slots auf Default**, nicht nur `enabled`. Sonst bliebe ein „freier" Slot mit 12 dB Gain zurück, den ein späterer Automationsschreiber hörbar machen könnte, ohne ihn zu belegen.
3. **Der Undo-Eintrag ist der Schnappschuss aus §5.1 Punkt 6**, mit `art = remove` und der Slot-ID im Kopf. Er enthält damit Slot-ID, Typ, `channel_mode`, Dynamic-Zustand und sämtliche Werte, wie Fünferblock 02 es verlangt — und zusätzlich alles andere, was die Transaktion berührt hat.
4. **Automation auf einem freien Slot wird gemessen, nicht behauptet.** M-63 prüft **beides**: dass der Parameterwert dem Host folgt (sonst wäre die Sonde ein Plugin, dessen Parameter hängen) und dass der Audioausgang bitgleich zum Lauf ohne die Automation bleibt.
5. **Der Fall „alle acht belegt" ist ein eigener, unterscheidbarer Fehlerzustand** (M-62), kein generischer. Die Oberfläche muss daraus ableiten können, dass sie den Zustand am Graphen meldet, statt einen Slot zu opfern (Arbeitsmodus `:204-205`).


### 5.6 R6 — Schutz-Zonen

**Der Entscheid, wörtlich:**

> R6 — Schutz-Zonen (Lücke im Vertrag, Dirigent-Entscheid, Produktwortlaut
> Interview 23.08.): Liste im Kind `Dsp` (kein Host-Parameter), 0 bis 8
> Einträge (höchstens eine Zone je Band-Slot), je Zone {id, low_hz, high_hz,
> enabled} mit 20 Hz ≤ low < high ≤ min(20 kHz, 0,45·fs) im DTO geprüft.
> Wirkung lokal: eine Transaktion, die ein belegtes, eingeschaltetes Band NEU
> mit seiner freq_hz in eine eingeschaltete Zone setzt oder dort einschaltet,
> wird abgewiesen (Fehlercode „User-Schutz“, keine halbe Anwendung); das
> Einschalten einer Zone über einem bereits liegenden Band ist erlaubt, das
> Band gilt dann als „verletzt“ und wird im state_report.dsp so gemeldet;
> Host-Automation schreibt vorbei und erzeugt dieselbe Verletzungsmeldung,
> verhindert wird sie dort nicht (Interview :252-263). „Lösen“ = enabled=false
> oder Eintrag entfernen, beides Transaktion. Zonen sind Teil des DTO, des
> state_hash, des Presets und des state_report.dsp.

**Feinheiten.**

1. **Die `id` gehört der Zone, nicht einem Slot.** R6 schreibt „0 bis 8 Einträge (höchstens eine Zone je Band-Slot)". Gelesen wird das als **Obergrenze acht**, nicht als Bindung an einen Slot: `id` ist eine Ganzzahl 0 bis 7, im Bestand eindeutig, und eine Zone gehört keinem Band. Grund: eine an einen Slot gebundene Zone verschwände beim Remove dieses Slots — und R6 sagt ausdrücklich, dass „Lösen" ausschließlich `enabled = false` oder das Entfernen des Eintrags ist. Zwei Wege, eine Zone loszuwerden, wären zwei Wahrheiten.
2. **Abweichung 2 (§5.17): die Nyquistgrenze prüft die Laufzeit, nicht das DTO.** R6 nennt `20 Hz ≤ low < high ≤ min(20 kHz, 0,45·fs)` „im DTO geprüft". Gebaut wird die statische Grenze **20 Hz ≤ low < high ≤ 20 kHz** im DTO und die Kappung auf `0,45·fs` **zur Laufzeit** — genau so, wie `freq_hz` es seit v1 hält (`nakama-parameter-v1.json:13`). Grund: die Samplerate ist beim Laden eines Projekts nicht bekannt, und eine samplerateabhängige DTO-Grenze würde einen bei 96 kHz gespeicherten Stand bei 44,1 kHz **unlesbar** machen. Das wäre ein echter Save/Load-Bruch für einen Wert, den der User gesetzt hat.
3. **Die Kollisionsregel prüft eine Kante je Zone, keinen Sammelzustand (MN1, B-03).** Sei für einen Slot und eine **einzelne** eingeschaltete Zone `Z` das Prädikat `P(Slot, Z)` = (belegt ∧ eingeschaltet ∧ `freq_hz` liegt in `Z`). Abgewiesen wird eine Transaktion genau dann, wenn es **irgendein** Paar (Slot, Z) gibt, für das `P` **von falsch auf wahr kippt**, UND die Transaktion mindestens eines der Felder `occupied`, `enabled` oder `freq_hz` dieses Slots ändert. Damit gilt: hineinziehen wird abgewiesen (M-67), eine Zone über einem liegenden Band einschalten ist erlaubt und meldet Verletzung (M-68), ein bereits verletztes Band bleibt **in seiner Zone** bedienbar (M-69) — und es kann trotzdem **nicht neu in eine andere Zone** eintreten. Die vorige Fassung wertete „liegt in irgendeiner eingeschalteten Zone" als **einen** Wahrheitswert aus; ein bei 100 Hz unter Zone A (80–120 Hz) verletzt liegendes Band hätte sich damit nach Zone B (900–1100 Hz) verschieben lassen, weil der Sammelwert wahr blieb. Das widerspricht der harten Sperre aus dem Interview `:132-133`.
   **Ausnahme: Ganzzustands-Wiederherstellungen passieren den Riegel nicht (MN1, B-02, Dirigent-Entscheid 10.09.2026).** Der Zonenriegel bewacht **Bedienänderungen**, die ein Band neu in eine Zone bewegen. **Undo, Redo und Projekt-Laden** ersetzen einen ganzen, **bereits bestätigten** Zustand; sie sind keine Bedienänderung und laufen nicht erneut durch den Riegel. Eine dabei entstehende Lage in einer Zone wird als **Verletzung gemeldet** wie in M-68. **Preset-Laden** gilt ebenso als Ganzzustands-Ersetzung: der User hat diese Lage gespeichert, also Meldung statt Abweisung. Ohne diese Ausnahme wäre der Pflichtweg aus M-61 unmöglich — Band einschalten, Zone darüber einschalten (erlaubt), Remove, Undo: beim Undo kippt `P` von falsch auf wahr, und der Riegel hätte genau den unmittelbaren Rückweg abgewiesen, den Fünferblock 02 `:139-141` zusagt.
4. **Der Fehlergrund ist lokal, nicht auf dem Draht.** „User-Schutz" ist der Grund des lokalen Transaktionsergebnisses. Ein v3-`fehlercode` dafür entsteht **nicht** in diesem Ticket: es gibt hier keinen Fernweg, der ihn tragen könnte, und einen Wire-Code ohne Sender zu erfinden wäre der Vorgriff, den die Ticketgrenze verbietet. S29–31 legt ihn an, wenn `apply_transaction` entsteht — der lokale Zustand liegt dann schon fertig da.
5. **Verletzte Bänder werden gemeldet, nicht behandelt.** Ein verletztes Band klingt weiter, wird nicht verschoben und nicht stumm geschaltet. Die Zone zeigt die Verletzung; verhindern kann sie sie am Automationsweg nicht (Interview `:252-263`, „Ehrliche Grenze der harten Sperre").


### 5.7 R7 — Sidechain und dynamische Kennlinie

**Der Entscheid, wörtlich:**

> R7 — Sidechain: `sidechain_source` behält seine drei Vertragswerte. `none`
> und `internal` sind gebaut (interner Detektor = das bandgefilterte
> Eingangssignal des Bandes vor dem Band). `priority_sidechain` wird auf
> `internal` GEKLEMMT und als geklemmter Parameter gemeldet (§44.3
> ACK-Klemmliste; im state_report.dsp als Klemmung sichtbar), bis P8 den
> Nebenweg liefert; nichts wird still umgedeutet. Detektor = RMS mit
> Attack/Hold/Release des Bandes (Lücke, Dirigent-Entscheid; kein neuer
> Parameter). Die dynamische Verstärkung eines Bandes ist auf
> ±|dynamic_range_db| begrenzt, wirkt zusätzlich zu gain_db, und ihre
> Kennlinie legt die Matrix fest (mit Golden). Bei dynamic_range_db = 0 oder
> unter Threshold steht das Band still (Fünferblock 01, :113-115).

**Feinheiten.**

1. **Die Kennlinie.** Mit `e_db` als RMS-Pegel des Detektors nach Attack/Hold/Release, `t` = `threshold_db`, `r` = `dynamic_range_db` und der festen Kniebreite `kKniebreiteDb` = **12 dB**:

   `u = min(1, max(0, e_db − t) / 12)` und `g_dyn_db = r · u`.

   Die Auslenkung ist damit durch die Konstruktion auf ±\|r\| begrenzt; ein zweiter Klemmschritt ist unnötig und wäre eine zweite Wahrheit. Das Vorzeichen von `r` trägt die Richtung: `r` < 0 senkt über dem Threshold ab (kompressorartig), `r` > 0 hebt an. Die 12 dB sind eine Technikkonstante mit Golden, kein aus einer Quelle abgeleiteter Wert — der Entwurf nennt keine.
2. **Der Summenwert wird nicht nachgeklemmt.** Der wirksame Bandgain ist `gain_db + g_dyn_db` und kann ±24 dB erreichen. Die Vertragsgrenzen ±12 dB gelten **je Parameter**, nicht für ihre Summe; eine Klemmung der Summe würde die dynamische Bewegung an den Gain-Extremen unsichtbar machen, und genau die soll der User sehen (Abnahme 01.09.).
3. **Der Detektor.** RBJ-Bandpass mit konstanter Spitzenverstärkung auf `freq_hz` und `q` desselben Slots, angewandt auf das **Eingangssignal des Bandes**, danach quadratischer Mittelwert über eine Ein-Pol-Hüllkurve mit den Koeffizienten `exp(−1/(fs · τ))` und getrenntem Hold-Zähler. Der Detektor hört **vor** dem Band; hörte er dahinter, wäre die Regelung rückgekoppelt und die Kennlinie nicht mehr die zugesagte.
4. **`none` heißt kein Detektor.** Der Wert `none` ist nicht „internal mit Range 0": bei `none` wird der Detektor gar nicht gebaut. Sonst kostete ein ausdrücklich abgewählter Sidechain Rechenzeit.
5. **Die Klemmung von `priority_sidechain` lässt den persistenten Wert stehen.** Geklemmt wird beim **Bauen des Programms**, nicht beim Speichern: der State trägt weiter `priority_sidechain`, und wenn P8 den Nebenweg liefert, wirkt der Userwunsch ohne dass er ihn neu setzen muss. Ein beim Speichern umgeschriebener Wert wäre unwiederbringlich verloren.


### 5.8 R8 — Getrennte Dynamic-Parameter und `channel_mode`

**Der Entscheid, wörtlich:**

> R8 — Getrennte Dynamic-Parameter und channel_mode: `dynamic_enabled` und die
> fünf Werte bleiben getrennte persistente Parameter; Ausschalten ändert nur
> `dynamic_enabled` (Test: Werte nach aus/ein bitgleich). `channel_mode` ist
> diskret, Default stereo, topologisch, nie mit Typ oder Dynamic gekoppelt.
> Topologische Wechsel (type, channel_mode, dynamic_enabled, sidechain_source,
> occupied) nur am Blockrand mit Crossfade; kontinuierliche Werte über Rampen;
> Zipperresiduen < −100 dBFS (§44.5).

**Feinheiten.**

1. **„Bitgleich" heißt bitgleich.** M-24 vergleicht die fünf Werte nach aus/ein als Doubles bitgenau, nicht mit Toleranz. Der Fehler, gegen den Fünferblock 01 sich richtet, ist ein stiller Default-Reset — und der fiele bei einem Toleranzvergleich genauso auf, aber ein Rundungsfehler beim Durchreichen fiele nicht. Beide sind Befunde.
2. **Die fünf topologischen Parameter, vollständig.** `type`, `channel_mode`, `dynamic_enabled`, `sidechain_source` (die vier aus §53.8) und **`occupied`** (neu, R8 nennt es mit). Alle fünf wechseln nur am validierten Blockrand mit vorbereitetem Crossfade.
3. **Kein Modus wird geerbt.** Die Auswahl eines anderen Slots übernimmt weder `channel_mode` noch sonst einen Wert des vorigen (Fünferblock 02 `:65-72`). Das ist eine Zusage an den **Kern**, nicht nur an eine spätere Oberfläche: der Kern hält je Slot einen vollständigen eigenen Zustand und kennt keinen „zuletzt benutzten" Wert.


### 5.9 R9 — Bankpool und Audiothread

**Der Entscheid, wörtlich:**

> R9 — Bankpool und Audiothread: vier vorallokierte Bänke (je Double-Buffer
> Committed/Candidate), Ownership free → preparing → ready(generation) →
> audio_active/fading → retired → free, busy_retry und reclaim_pending_mask
> wie §44.2:2991-3001; Programme werden außerhalb des Audiothreads
> vorbereitet, der Audiothread tauscht nur am Blockrand per Generation;
> Koeffizienten 64 Bit. Im Callback: keine Sperre, keine Allokation, kein IO,
> kein Logging, keine Ausnahme; Überlast verwirft Analysefenster, nie Audio.
> Nicht-endliche Eingangssamples im aktiven Pfad werden verriegelt und gezählt
> (Filterzustände bleiben endlich, Zähler lockfrei lesbar); der Passthrough
> (eq_enabled=false oder Hard-Bypass) sanitisiert NICHTS. Latenz an den Host
> bleibt 0 (kein setLatencySamples ≠ 0, kein Tail).

**Feinheiten.**

1. **Der ACK-Ring fasst acht Einträge** bei vier Slots — mehr als es Slots gibt, wie §44.2 verlangt, mit Reserve für die Fadephase, in der bis zu vier Bänke gleichzeitig leben.
2. **Die Generation ist ein monoton steigender 64-Bit-Zähler.** Sie wird nie zurückgesetzt, auch nicht beim Neuanlauf des Audiothreads; ein Zähler, der von vorn beginnt, könnte einen alten ACK auf eine neue Bank beziehen.
3. **Abweichung 4 (§5.17): der Nicht-Endlich-Riegel prüft zweistufig.** R9 verlangt, dass nicht-endliche Eingangssamples verriegelt und gezählt werden und die Filterzustände endlich bleiben. Gebaut wird: **je Sample** ein `isfinite` auf dem Eingang (das ist der Eintrittsweg für NaN und Inf), und **je Blockrand** eine Prüfung der Filterzustände mit Nullung des betroffenen Bandes und eigenem Zähler. Grund: eine Prüfung aller Zustände je Sample kostet bei acht Bändern mal vier Kanalkomponenten mal zwei Zustandswerten das Vielfache der eigentlichen Filterarbeit. Die zweistufige Fassung hält die Zusage („Filterzustände bleiben endlich") und ist bezahlbar; sie deckt zusätzlich den Fall, in dem ein Zustand ohne nicht-endlichen Eingang entgleist (extreme Q nahe Nyquist).
4. **Zwei Zähler, nicht einer.** Ein Zähler für verriegelte Eingangssamples, einer für geheilte Filterzustände. Ein gemeinsamer Zähler könnte die beiden Ursachen nicht trennen, und die Diagnose „woher kam das NaN" wäre wertlos.
5. **Der Tailbeweis gilt für Passthrough und Bypass.** `getTailLengthSeconds()` bleibt 0,0 (der Wert ist für einen minimalphasigen EQ ohne Lookahead konventionell und richtig), aber die **gemessene** Zusage „kein Tail" gilt für den ausgeschalteten und den überbrückten Zustand. Ein eingeschalteter resonanter Filter klingt aus; das ist kein Tail im Sinne des Hostvertrags, und die Runner-Behauptung sagt genau das und nicht mehr (Prüfliste E).


### 5.10 R10 — Hörmatrix und Taps

**Der Entscheid, wörtlich:**

> R10 — Hörmatrix und Taps: die Auswahl Dry/Processed/Delta/Candidate ist
> transient (kein persistenter Parameter, kein Host-Parameter, nach Laden
> immer Processed — nichts Ungefragtes); Delta = Processed − Dry mit festem
> Lautheitsabgleich (§30.1); die drei Taps liefern kohärente Blöcke für die
> bestehende Analyse (Engine kennt keine Optik). Ein Wechsel ist klickfrei;
> identischer Zustand in A und B nullt innerhalb numerischer Toleranz (§49.3).

**Feinheiten.**

1. **`kDeltaMakeupDb` = +12 dB, fest.** §30.1 verlangt einen „festen Lautheitsabgleich"; fest heißt hier ausdrücklich **materialunabhängig**. Ein aus dem laufenden Pegel gerechneter Abgleich wäre ein Kompressor im Hörweg und verstieße gegen §49.2 Gate 5. +12 dB bringt ein typisches Delta in einen hörbaren Bereich, ohne ein starkes Delta zu übersteuern; der Wert ist eine Technikkonstante mit Golden.
2. **Bei Gleichheit ist das Delta exakt 0.** `Processed − Dry` ist bitgenau 0, und `0 · 10^(12/20)` bleibt 0. Der Rotbeweis zu M-54 misst das, weil ein Delta, das bei identischem Signal rauscht, den A/B-Vergleich unbrauchbar macht.
3. **Candidate ohne Kandidat fällt sichtbar zurück.** Der Getter meldet den **wirksamen** Zustand, nicht den gewünschten. Ein Element, das behauptet, einen Kandidaten zu spielen, den es nicht gibt, wäre ein totes Element (`CLAUDE.md`).
4. **Die Auswahl liegt in keinem Vertrag.** Kein DTO-Feld, kein Host-Parameter, kein Eintrag im `Dsp`-Kind, nichts in `state_report`. Sie lebt ausschließlich im Prozessor und stirbt mit ihm.


### 5.11 R11 — Transaktion, Revision, Hash, Undo

**Der Entscheid, wörtlich:**

> R11 — Transaktion, Revision, Hash, Undo:
> Apply/Revert/Neutralisieren/Remove/Undo/Redo/Preset-Laden sind atomare
> Transaktionen mit genau einer neuen state_revision; Automation läuft im
> AutomationOverlay mit automation_epoch und zählt keine Revision
> (§44.3:3049-3053). state_hash = SHA-256 über die RFC-8785-Form des
> validierten DTO (bestehende JCS-Implementierung und Vektoren aus SONDE-006
> wiederverwenden, nicht neu schreiben). Undo-Ring mit Tiefe ≤ 32 (Vertrag
> undo_tiefe), persistiert im Kind `Dsp`. 10.000 doppelte, vertauschte oder
> veraltete Befehle erzeugen höchstens eine gültige Revision (§44.5). Kein
> halber Zustand überlebt einen Fehler (Rotbeweis: Fehler mitten in Apply →
> Committed unverändert, Hash unverändert).

**Feinheiten.**

1. **Die sieben Transaktionen, vollständig:** Apply, Revert, Neutralisieren, Remove, Undo, Redo, Preset-Laden. Dazu der abgeschlossene manuelle Parametergestus (§44.3) als achter Weg zu einer Revision.
2. **Die feste Reihenfolge innerhalb einer Transaktion.** Sie steht seit der **Konvergenzrunde (10.09.2026)** an **genau einer** Stelle: in der geschlossenen Protokollspezifikation **§5.11.4**, Teil 2. Dieser Punkt wiederholt sie nicht mehr — eine Reihenfolge, die an zwei Stellen steht, ist in drei Runden dreimal auseinandergelaufen (§7.8). Er nennt nur, was **an** ihr hängt und warum:
   **Alle fehlbaren und allozierenden Stufen liegen vor einem einzigen Commit-Punkt (MN1, B-01).** `nakama::parameter::stateHash` ruft `dtoKanon`, baut einen `juce::MemoryBlock` und gibt bei nicht kanonisierbarem Inhalt `false` mit `grund` zurück (`eq-copilot/plugin/state/NakamaParameter.cpp:163-183`). Er ist eine **fehlbare, allozierende** Stufe und kein zugesicherter Nachlauf; dasselbe gilt für die Undo-Ablage, die einen vollständigen Schnappschuss baut. Lägen beide hinter dem Tausch, wäre „Scheitert eine Stufe, wird nichts getauscht" nicht erfüllbar — es gäbe eine Revision ohne Hash oder ohne Rückweg. Beide sind vor dem Tausch bildbar: der Hash gehört zum **Kandidaten**-DTO, der Undo-Eintrag zum **noch bestätigten** Zustand.
   **Der Nachschlag steht vor jeder zustandsabhängigen Stufe (MN2, B-05).** Ein Riegel urteilt über den **heutigen** Zustand; eine Wiederholung fragt nach dem Urteil von **damals**. In der falschen Reihenfolge bekommt derselbe Aufruf zwei verschiedene Antworten, sobald sich der Zustand dazwischen bewegt hat (§5.11.4, Zeile T1 gegen T4). Der Nachschlag ist dafür der richtige Ort: er liest ein vorallokiertes Register fester Größe, allokiert nicht und kann nicht fehlschlagen (M-75).
   **Hinter dem Commit-Punkt steht nur, was nicht fehlschlagen und nicht allozieren kann** — einschließlich des Registereintrags, der ausschließlich dort entsteht (§5.11.4, Teil 3, I4).
3. **Undo und Redo bewegen einen Cursor im selben Ring.** Eine neue Transaktion nach einem Undo **schneidet den Redo-Zweig ab**. Zwei Ringe oder ein Redo-Stapel könnten auseinanderlaufen und einen Zustand herstellen, den es nie gab.
4. **Idempotenz, Konflikt und Wiederholung.** Sie sind seit der Konvergenzrunde nicht mehr Prosa, sondern die geschlossene **Protokollspezifikation §5.11.4** unmittelbar unter dieser Liste. Sie ist die **einzige** Stelle, an der die Transaktionsarithmetik definiert wird; **M-44, M-74, M-75, M-76, M-123, M-124 und M-125 leiten sich daraus ab** und wiederholen keine Zahl und keine abweichende Prosa, sondern zitieren die Tabelle.
5. **Der Kanon wird wiederverwendet, nicht neu geschrieben.** `nakama::parameter::dtoKanon` und `stateHash` aus SONDE-006 bekommen die v2-Felder; die JCS-Vektoren aus `eq-copilot/fixtures/state/jcs/` bleiben unverändert gültig. Ein zweiter Kanonisierer wäre eine zweite Wahrheit über denselben Text — und der ganze Sinn des dreisprachigen Hashes ist, dass es nur eine gibt.
6. **Die DTO-Form wächst additiv.** `{"dsp_schema_version": 2, "parameters": {…120 IDs…}, "schutz_zonen": [ … ]}`. RFC 8785 sortiert die drei Schlüssel nach UTF-16-Code-Units, also `dsp_schema_version` < `parameters` < `schutz_zonen`; die Zonenliste wird nach `id` aufsteigend geschrieben, damit dieselbe Menge immer denselben Text ergibt.


#### 5.11.4 Protokollspezifikation der Transaktion (geschlossen, Konvergenzrunde und NAK-245-Runde 10.09.2026)

Diese Spezifikation ersetzt die Prosa der Runden MN1 bis MN3. Sie ist die
**einzige** Stelle, an der die Transaktionsarithmetik definiert wird. **M-44,
M-74, M-75, M-76, M-123, M-124 und M-125 leiten sich aus ihr ab** — sie
zitieren, sie definieren nicht. Jede Zahl steht genau einmal: hier. Die
NAK-245-Runde hat in derselben Spezifikation fünf innere Widersprüche
geschlossen (§7.10); M-61 und M-126 zitieren zusätzlich T16 für die
Zonenausnahme, tragen aber keine Arithmetik.

##### Teil 1 — Definitionen und Arithmetik

| Zeichen | Bedeutung |
|---|---|
| `r` | Aktuelle `state_revision` des bestätigten Zustands. Frischer Zustand: `r = 0`; nach dem Ladestart `r = r0`. Jede committete Transaktion erhöht `r` um genau 1 (M-74); sonst ändert sich `r` nur beim Ladestart. `state_revision` zählt „ausschließlich diskrete Änderungen des bestätigten Basiszustands" (§44.3), und jede davon ist eine committete Transaktion — auch der abgeschlossene manuelle Parametergestus (§5.11 Feinheit 1, M-82). Innerhalb einer Sitzung sinkt `r` **nie** — auch Undo und Redo sind committete Transaktionen und zählen vorwärts. |
| `r0` | Revision beim **Sitzungs- oder Ladestart**. Frischer Zustand: `r0 = 0`. Nach dem Projekt-Laden ist `r0` die geladene `state_revision` — sie steht im Kind `Dsp` (`eq-copilot/schemas/state/nakama-state-v2.md:36`, M-89), und M-93 hält sie beim Byte-Roundtrip bytegleich —, und `R` wird geleert (I5). Eine **Sitzung** reicht von einem Sitzungs- oder Ladestart bis zum nächsten. Das Projekt-Laden selbst ist **keine** Transaktion: es trägt keine `tid`, erzeugt keine Revision und durchläuft keine Stufe aus Teil 2; eine dabei entstehende Lage in einer Zone wird als Verletzung gemeldet (§5.6.3 Feinheit 3, Ausnahme). Deshalb steht es hier und nicht in Teil 4. |
| `e` | Die von einer committeten Transaktion erzeugte Revision. Die erste Transaktion einer Sitzung hat `e = r0 + 1`. |
| `tid` | Transaktions-ID, vom Aufrufer vergeben, je logischem Auftrag genau eine. |
| `base_revision` | Die Revision, gegen die der Aufrufer seine Transaktion gebildet hat. |
| `R` | Wiederholungsregister: eine Menge von Tripeln `(tid, e, state_hash)`, genau ein Tripel je **in dieser Sitzung** committeter Transaktion im Fenster. |

**Fenster.** Ein Tripel `(tid, e, hash)` ist **im Fenster** genau dann, wenn

> `e > r0` ∧ `0 ≤ r − e ≤ 31`

also für die Revisionen `max(r0 + 1, r − 31) … r` — die **letzten 32** in dieser
Sitzung committeten Transaktionen, oder alle, solange es weniger sind. Der Rand
ist **inklusiv** an beiden Enden.

**Kapazität.** `R` hat **genau 32** vorallokierte Plätze. Weil jede committete
Transaktion genau eine Revision erzeugt und das Fenster höchstens 32 Revisionen
umfasst, sind **Kapazität und Fensterrand dieselbe Zahl**. Es gibt keinen
zweiten, unabhängig gezählten Deckel — genau dieser Widerspruch war K-1.

**Woher die 32 kommt.** Aus der Tiefe des Undo-Rings
(`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2372`, `undo_tiefe`,
`maximum: 32`). Der Undo-Ring kann die Revisionen `r … r−31` rückgängig machen —
**dieselbe** Menge. Ein Ergebnis wird also genau so lange vorgehalten, wie der
Zustand, auf den es sich bezieht, noch erreichbar ist — **innerhalb einer
Sitzung**. Über den Ladestart hinaus reicht nur der Ring: er ist persistent und
steht im Kind `Dsp` (M-89); das Register ist transient und beginnt leer (I5).
Beide tragen dieselbe Zahl 32, keine zweite: nach dem Ladestart ist das Register
nur **leerer**, nicht kleiner. Eine Zahl, zwei Verwendungen, kein erfundener
Deckel.

**Belegung, ausgerechnet.** `|R| = min(r − r0, 32)`.

| `r0` | `r` | `\|R\|` | ältestes `e` im Fenster | jüngstes `e` |
|---:|---:|---:|---:|---:|
| 0 | 1 | 1 | 1 | 1 |
| 0 | 32 | 32 | 1 | 32 |
| 0 | 33 | 32 | 2 | 33 |
| 0 | 34 | 32 | 3 | 34 |
| 0 | 100 | 32 | 69 | 100 |
| 33 | 33 | 0 | — | — |
| 33 | 34 | 1 | 34 | 34 |
| 33 | 65 | 32 | 34 | 65 |
| 33 | 66 | 32 | 35 | 66 |

Die unteren vier Zeilen sind der Ladestart eines Standes mit
`state_revision = 33`: unmittelbar danach ist `R` leer, obwohl `r = 33` ist, und
das Fenster füllt sich von dort aus genauso wie im frischen Zustand von 0 aus.

**Verdrängung.** Sie geschieht **ausschließlich am Commit-Punkt**, betrifft
**genau einen** Eintrag und nur, wenn `r − r0` nach dem Commit größer als 32 ist:
der Eintrag mit dem kleinsten `e` fällt heraus, weil für ihn `r − e = 32` gilt.
Bei `r0 = 0` und `r = 33` verlässt `e = 1` das Fenster, `e = 2` bleibt
(`33 − 2 = 31`). Nicht-committete Ausgänge verdrängen **nichts**, denn sie legen
nichts ab — genau das war D-1. Das Leeren beim Ladestart ist **keine**
Verdrängung, sondern der Beginn einer neuen Sitzung (I5).

##### Teil 2 — Die Stufenfolge

Genau eine Reihenfolge, hier definiert und nirgends sonst:

| Stufe | Was sie tut | fehlbar? | allokiert? |
|---|---|---|---|
| **S0** | `tid` in `R` nachschlagen. Treffer ⇒ Ausgang T1, **keine** weitere Stufe. | nein | nein |
| **S1** | `base_revision` gegen `r` prüfen. | ja (Konflikt) | nein |
| **S2** | DTO validieren (Typ, Bereich, Enum, Endlichkeit; Reihenfolge nach `nakama-state-v2.md:171`). | ja | ja |
| **S3** | Zonenriegel, nur bei **Bedienänderungen** (§5.6.3 Feinheit 3). Ganzzustands-Wiederherstellungen — Undo, Redo, Preset-Laden — durchlaufen ihn nicht (Ausnahme dort; T16). | ja (User-Schutz) | nein |
| **S4** | Kandidaten-DTO vollständig bauen. | ja | ja |
| **S5** | Programm bauen, **ohne** eine Bank zu belegen; ist das Programm **bankpflichtig** (Begriff im Absatz „Bankpflichtig und bankfrei“ unter dieser Tabelle) und keine Bank frei ⇒ `busy_retry` (M-44). Ein bankfreies Programm scheitert in S5 nicht. | ja (**nur** `busy_retry`) | nein |
| **S6** | `state_hash` über das Kandidaten-DTO bilden. | ja | ja |
| **S7** | Undo-Eintrag vorbereiten (Schnappschuss des **noch** bestätigten Zustands). | ja | ja |
| **— COMMIT-PUNKT —** | Ab hier kann nichts mehr fehlschlagen und nichts mehr allozieren. | — | — |
| **S8** | `r` erhöhen, Committed tauschen — dabei ein **bankpflichtiges** Programm aus S5 in eine freie Bank legen und publizieren, ein **bankfreies** als ENDE-Marke ohne Bank publizieren (E-18); beides kann hier nicht scheitern —, Hash und Undo-Eintrag übernehmen, Host-Dirty melden, `state_report` senden, **Registereintrag anlegen**, gegebenenfalls einen Eintrag verdrängen. | nein | nein |

**Warum `base_revision` (S1) vor der Validierung (S2) steht.** Eine veraltete
`base_revision` heißt, dass die ganze Nutzlast gegen einen Zustand gebildet
wurde, den es nicht mehr gibt; ihren Inhalt zu beurteilen wäre gegenstandslos.
Erst diese Reihenfolge macht die Falltabelle **eindeutig**: eine Transaktion
mit veralteter `base_revision` **und** ungültigem Wert hat genau einen Ausgang,
nämlich Konflikt. Bis zur Konvergenzrunde stand die Validierung davor, und die
beiden Fälle überlappten. An B-01 ändert das nichts: S1 ist weder fehlbar im
Sinne einer Allokation noch teuer, und alle allozierenden Stufen liegen
unverändert vor dem Commit-Punkt.

**Bankpflichtig und bankfrei (NAK-245, Nacharbeit 1).** Ein Programm ist
**bankpflichtig** genau dann, wenn der Kandidatenzustand — das Kandidaten-DTO
aus S4 — `eq_enabled = true` trägt, auch bei eingeschaltetem Hard-Bypass: der
Kern fragt vor der Reservierung nur `eq_enabled` ab
(`eq-copilot/plugin/dsp/DspKern.cpp:170`, `:179`). Ein Zustand mit
`eq_enabled = false` ist **bankfrei** und wird nach E-18 (§9.2) als ENDE-Marke
ohne Bank publiziert (`:170-177`). Der Begriff ist nur hier definiert; er ist
keine neue Stufe und keine neue Zahl. Er begrenzt, was an einer Bank hängt: die
Abweisung bei vollem Pool in S5 und die Bankbelegung in S8 gelten nur für
bankpflichtige Programme.

**Warum S5 keine Bank belegt und nur einen Fehlerausgang hat (NAK-245).** Der
gebaute Kern kennt beim Programmbau genau ein Scheitern: für ein bankpflichtiges
Programm ist keine Bank frei. `DspKern::uebernehmeZustand` reserviert, baut und
publiziert dann in einem Zug und liefert `false` nur in diesem Fall
(`eq-copilot/plugin/dsp/DspKern.cpp:179-196`); ein bankfreies Programm belegt
keine Bank, und der Aufruf liefert immer `true` (`:170-177`). `baueProgramm`
gibt nichts zurück und schreibt in ein `DspProgramm` fester Größe
(`eq-copilot/plugin/dsp/DspProgramm.h:158`, `:183-184`). Einen zweiten
Fehlerausgang bei freier Bank gibt es also nicht, und für ein bankfreies
Programm gar keinen; S5 bekommt neben T5 keine eigene Fehlerzeile. Belegte S5
aber schon eine Bank, überlebte die Reservierung einen Fehler in S6 oder S7: der
Pool führt einen Slot aus `vorbereitend` nur nach `bereit`
(`eq-copilot/plugin/dsp/DspBankPool.cpp:143`), außer beim Zurücksetzen des
ganzen Pools. Nach einigen solchen Fehlern wäre der Pool voll, ohne dass ein
Audio-ACK ihn je leerte, und T7 wäre unerreichbar. Deshalb baut S5 ohne Bank,
und die Bank eines bankpflichtigen Programms wird erst in S8 belegt. Dort kann
das nicht mehr scheitern: Bänke belegt nur der Control-Worker (§44.2), und
zwischen S5 und S8 belegt er keine. Für ein bankfreies Programm publiziert S8 eine
ENDE-Marke; auch das kann nicht scheitern und alloziert nicht:
`DspBankPool::publiziereEnde` tauscht das Publikationswort des Pfades atomar aus
und liefert nur einen dabei verdrängten, noch nicht übernommenen Slot oder −1
zurück, keinen Fehler (`eq-copilot/plugin/dsp/DspBankPool.cpp:153-158`,
`eq-copilot/plugin/dsp/DspBankPool.h:172-176`).

##### Teil 3 — Invarianten

| Nr. | Invariante |
|---|---|
| **I1** | `\|R\| = min(r − r0, 32)`, immer — auch unmittelbar nach dem Ladestart (`r = r0`, `R` leer). |
| **I2** | `(tid, e, hash) ∈ R` ⇔ `e > r0` ∧ `0 ≤ r − e ≤ 31`. |
| **I3** | Kapazität von `R` = Fensterspanne = Tiefe des Undo-Rings = **32**. Eine Zahl. |
| **I4** | Ein Registereintrag entsteht **ausschließlich** in S8 und nur für eine committete Transaktion. Kein Ausgang vor dem Commit-Punkt legt einen an. |
| **I5** | `R` ist **transient**: es lebt im Prozessor, stirbt mit ihm, **wird beim Ladestart geleert**, steht nicht im Kind `Dsp`, nicht im DTO und nicht im `state_hash`. Begründung: eine Wiederholung kann nur von einem Aufrufer kommen, der die Transaktion noch für offen hält — und der stirbt mit derselben Sitzung. Kommt sie nach dem Ladestart trotzdem, ist sie ein neuer Versuch (T17). |
| **I6** | Genau ein Ausgang je Eingabe. Die Stufen **S0 bis S7** werden in fester Reihenfolge geprüft; die **erste Stufe, die entscheidet**, bestimmt den Ausgang — S0 durch einen Treffer, S1 bis S7, indem sie nicht bestehen —, und entscheidet keine, committet **S8**. Jede Zeile in Teil 4 hat genau einen Ausgang und setzt voraus, dass keine Stufe vor ihrer ersten Stufe entscheidet; entscheidet in einem beschriebenen Ablauf eine frühere Stufe, gilt die Zeile dieser Stufe (ein bankpflichtiges T9 bei vollem Pool endet in T5). Jede fehlbare Stufe S1 bis S7 ist in mindestens einer Zeile erste Stufe. Zeilen mit derselben ersten Stufe tragen denselben Ausgang und unterscheiden sich nur im Vorlauf oder in der Art der Transaktion, die sie messen — zwei Zeilen, deren Bedingungen dieselbe Eingabe zulassen, haben deshalb nie verschiedene Ausgänge. |

##### Teil 4 — Falltabelle: Zustand × Eingabe → Ausgang

Eingabe ist stets `(tid, base_revision, Nutzlast)`. „Erste Stufe" nennt die
Stufe, die den Ausgang bestimmt; wie die Zeilen zusammen zu lesen sind, sagt
**I6**. Die Tabelle hat **17 Zeilen**, T1 bis T17 — die Zeilenzahl steht nur
hier. Die Fehlerausgänge von S4, S6 und S7 heißen wie in T3 „Fehler" mit der
Stufe als Grund; ein Wire-Code entsteht dafür nicht (§5.6.3 Feinheit 4 gilt
sinngemäß). Die Einspritzfälle aus M-75 stehen als B7-Fall an der Zeile ihrer
Stufe.

| Nr. | Zustand | Eingabe | Erste Stufe | Ausgang | `r` danach | memoisiert | Test (B7) | Rotbeweis an der Zusage |
|---|---|---|---|---|---|---|---|---|
| **T1** | `tid` liegt in `R` (I2) | dieselbe `tid`, beliebige `base_revision`, beliebige Nutzlast | **S0** | **Gespeichertes Ergebnis**: Revision `e`, Hash aus `R` | `r` (unverändert) | bleibt | `wiederholung_im_fenster_liefert_dasselbe_ergebnis` | Es entsteht eine zweite Revision; oder die Eingabe durchläuft die Stufen ab S1 und bekommt ein Urteil des heutigen Zustands |
| **T2** | `tid` unbekannt | `base_revision ≠ r` | **S1** | **Konflikt** | `r` | nein | `veraltete_base_revision_ist_konflikt_ohne_wirkung`; `fehler_in_S1_laesst_committed_und_hash_unveraendert` (M-75) | Die Transaktion wird angewandt; oder der Konflikt landet in `R` und blockiert spätere Versuche |
| **T3** | `tid` unbekannt | `base_revision = r`, Nutzlast ungültig | **S2** | **Fehler** (Grund aus dem DTO-Vertrag) | `r` | nein | `ungueltige_nutzlast_faellt_und_wird_nicht_gemerkt`; `fehler_in_S2_laesst_committed_und_hash_unveraendert` (M-75) | Ein Bereichs- oder Enumfehler wird geklemmt statt abgelehnt; oder der Fehler wird memoisiert |
| **T4** | `tid` unbekannt; die Transaktion ist eine **Bedienänderung** nach §5.6.3 Feinheit 3: sie ändert `occupied`, `enabled` oder `freq_hz` eines Slots, und `P` kippt für ein Paar (Slot, Z) dieses Slots von falsch auf wahr | `base_revision = r`, Nutzlast gültig | **S3** | **User-Schutz** | `r` | nein | `zonenriegel_weist_ab_und_merkt_nichts`; `fehler_in_S3_laesst_committed_und_hash_unveraendert` (M-75) | Das Band landet in der Zone; oder die Abweisung wird memoisiert und sperrt jeden späteren Versuch |
| **T5** | `tid` unbekannt, alle vier Bänke belegt und das Programm ist **bankpflichtig** (Teil 2) | `base_revision = r`, Nutzlast gültig, Zone frei | **S5** | **`busy_retry`** (M-44) | `r` | nein | `busy_retry_wird_nicht_gemerkt`; `fehler_in_S5_laesst_committed_und_hash_unveraendert` (M-75) | Eine aktive Bank wird verdrängt; oder `busy_retry` wird memoisiert und kommt für immer zurück |
| **T6** | `tid` unbekannt, alle Stufen bestanden | `base_revision = r`, Nutzlast gültig | **S8** | **Commit** | `r + 1` | **ja** (`tid`, `e = r+1`, Hash) | `commit_erzeugt_genau_eine_revision_und_einen_eintrag` | Zwei Revisionen; oder der Eintrag fehlt und die Wiederholung committet erneut |
| **T7** | wie T5, danach hat ein Audio-ACK eine Bank freigegeben (**keine** Revision); alle übrigen Stufen **S2 bis S7** bestehen | **dieselbe** `tid`, `base_revision = r` — unverändert gültig, weil T5 keine Revision erzeugte | **S8** | **Commit** | `r + 1` | **ja** | `wiederholung_nach_bankfreigabe_committet` | Die Wiederholung bekommt erneut `busy_retry` aus dem Register und erreicht S5 nie |
| **T8** | wie T4, danach wurde die Zone gelöst — **das ist selbst eine Transaktion** und hat `r + 1` erzeugt (M-71) | **dieselbe** `tid`, **alte** `base_revision = r` | **S1** | **Konflikt** — und das ist **richtig** | `r + 1` | nein | `retry_nach_zonenloesung_mit_alter_base_revision_ist_konflikt` | Die Transaktion committet trotz veralteter `base_revision` und umgeht damit M-76 |
| **T9** | wie T8; alle übrigen Stufen **S2 bis S7** bestehen — S3, weil die Zone gelöst ist, und S5, weil das Programm bankfrei ist oder eine Bank frei ist (Teil 2) | **dieselbe** `tid`, **aktualisierte** `base_revision = r + 1` | **S8** | **Commit** | `r + 2` | **ja** | `retry_nach_zonenloesung_mit_aktueller_base_revision_committet`; Abgrenzung bei vollem Pool, zweigeteilt: **bankpflichtig** → `retry_nach_zonenloesung_bei_vollem_pool_ist_busy_retry` (endet nach I6 in T5, nicht memoisiert, danach der Weg aus T7); **bankfrei** → `retry_nach_zonenloesung_bei_vollem_pool_ohne_bankpflicht_committet` (S5 besteht; Ausgang dieser Zeile) | Die Wiederholung wird weiter mit User-Schutz abgewiesen, obwohl die Zone gelöst ist; oder **ein bankpflichtiges T9 committet bei vollem Pool statt `busy_retry`**; oder **ein bankfreies T9 bekommt bei vollem Pool `busy_retry` statt Commit** |
| **T10** | 33 **in dieser Sitzung** committete Transaktionen (`r − r0 = 33`) | Wiederholung der **ersten** dieser Sitzung (`e = r0 + 1`, `r − e = 32`) | **S1** (S0 findet nichts) | **Konflikt** | `r` | nein | `fensterrand_erste_transaktion_ist_draussen` | Die erste Transaktion liegt noch in `R` — dann wäre die Kapazität 33 und I1 verletzt |
| **T11** | dieselbe Lage wie T10 | Wiederholung der **zweiten** (`e = r0 + 2`, `r − e = 31`) | **S0** | **Gespeichertes Ergebnis** | `r` | bleibt | `fensterrand_zweite_transaktion_ist_drinnen` | Die zweite Transaktion ist verdrängt — dann ist das Fenster kleiner als die Undo-Tiefe und die Vorhaltegarantie gebrochen |
| **T12** | T committet in dieser Sitzung (`e`), U committet (`e+1`, also `r = e+1`), danach **31 Abweisungen** wegen veralteter `base_revision` | Wiederholung von **T** | **S0** | **Gespeichertes Ergebnis von T** | `r` | bleibt | `abweisungen_verdraengen_keinen_committeten_eintrag` | T ist verdrängt und die Wiederholung endet mit Konflikt, obwohl seit T nur **eine** Revision vergangen ist und Undo T noch erreicht |
| **T13** | `tid` unbekannt; S1 bis S3 bestehen; das Kandidaten-DTO lässt sich nicht vollständig bauen | `base_revision = r`, Nutzlast gültig | **S4** | **Fehler** (Grund: Stufe S4) — bestätigter Zustand und `state_hash` unverändert, kein Ring- und kein Registereintrag | `r` | nein | `fehler_in_S4_laesst_committed_und_hash_unveraendert` (M-75) | Nach einem Fehler in S4 ist `r` erhöht, oder der bestätigte Zustand trägt einen Teil des Kandidaten; oder der Fehler wird memoisiert und sperrt die Wiederholung |
| **T14** | `tid` unbekannt; S1 bis S5 bestehen; der `state_hash` über das Kandidaten-DTO lässt sich nicht bilden (`nakama::parameter::stateHash` liefert `false`, §5.11 Feinheit 2) | `base_revision = r`, Nutzlast gültig | **S6** | **Fehler** (Grund: Stufe S6) — bestätigter Zustand und `state_hash` unverändert, kein Ring- und kein Registereintrag | `r` | nein | `fehler_in_S6_laesst_committed_und_hash_unveraendert` (M-75) | Nach einem Fehler in S6 ist `r` erhöht und der bestätigte Zustand getauscht — eine Revision ohne Hash; oder der Fehler wird memoisiert |
| **T15** | `tid` unbekannt; S1 bis S6 bestehen; der Undo-Eintrag lässt sich nicht vorbereiten | `base_revision = r`, Nutzlast gültig | **S7** | **Fehler** (Grund: Stufe S7) — bestätigter Zustand und `state_hash` unverändert, kein Ring- und kein Registereintrag | `r` | nein | `fehler_in_S7_laesst_committed_und_hash_unveraendert` (M-75) | Nach einem Fehler in S7 ist `r` erhöht, oder ein halber Ringeintrag liegt vor — eine Revision ohne Rückweg; oder der Fehler wird memoisiert |
| **T16** | `tid` unbekannt; die Transaktion ist eine **Ganzzustands-Wiederherstellung** — Undo, Redo oder Preset-Laden (§5.6.3 Feinheit 3, Ausnahme) —, und dabei tritt ein Band neu in eine eingeschaltete Zone ein; S2 und S4 bis S7 bestehen | `base_revision = r`, Nutzlast gültig | **S8** (S3 gilt nicht) | **Commit** — das Band gilt als **verletzt** und wird gemeldet (M-61, M-68) | `r + 1` | **ja** | `undo_unter_einer_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` (M-61), `redo_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` und `preset_laden_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` (M-126) | Die Wiederherstellung wird mit User-Schutz abgewiesen; oder sie committet, aber die Verletzung wird nicht gemeldet |
| **T17** | Ladestart eines Standes mit `state_revision = 33`: `r0 = r = 33`, `R` leer; S2 bis S7 bestehen | eine `tid`, die in der **vorigen** Sitzung committet wurde, `base_revision = 33`, Nutzlast gültig | **S8** (S0 findet nichts, S1 besteht) | **Commit** — ein neuer Versuch; die Konsequenz aus I5, kein Fehler | `r + 1` (= 34) | **ja** | `wiederholung_ueber_sitzungsgrenze_ist_neuer_versuch` | Die Wiederholung bekommt ein gespeichertes Ergebnis aus der vorigen Sitzung — das Register wurde mitgespeichert, I5 ist gebrochen; oder das Laden hat `r` auf 0 zurückgesetzt, und S1 meldet Konflikt — M-93 ist gebrochen |

**T10 und T11 sind ein Paar** und messen beide Seiten desselben Randes; einzeln
belegt keiner von beiden, dass der Rand an der richtigen Stelle liegt. Dasselbe
gilt für **T8 und T9**: erst zusammen zeigen sie, dass nach einer gelösten Zone
nicht der Zonenriegel, sondern die `base_revision` über den Ausgang entscheidet.
Seit der NAK-245-Runde kommen zwei Paare hinzu. **T4 und T16** zeigen erst
zusammen, dass der Zonenriegel nach der Art der Transaktion unterscheidet:
dieselbe Lage in einer Zone wird bei einer Bedienänderung abgewiesen und bei
einer Ganzzustands-Wiederherstellung committet und gemeldet. **T1 und T17**
zeigen erst zusammen, dass das Register an die Sitzung gebunden ist: dieselbe
Wiederholung bekommt innerhalb der Sitzung das gespeicherte Ergebnis und nach
dem Ladestart einen neuen Versuch. T9 hat dazu eine zweigeteilte Abgrenzung
statt eines Partners: bei vollem Pool gilt für ein bankpflichtiges Programm nach
I6 T5; ein bankfreies besteht S5 und committet nach T9 selbst (Teil 2, E-18).

**Was die Tabelle nicht sagt.** Sie regelt den lokalen Transaktionskern. Das
In-Flight-Register des `ControlClient` registriert **ausgehende**
P0-Nachrichten, löscht seinen Eintrag beim terminalen ACK und erwartet die
Idempotenz ausdrücklich vom **Broker**
(`eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:157-184`,
`:301-304`, `:332-336`). Es ist der **Fernweg-Anteil** und gehört S29–31; der
10.000-Befehle-Beweis aus §44.5 muss in P6 **fernsteuerungsfrei** fahrbar sein,
und die Zustandshoheit bleibt nach `CLAUDE.md` in der Audio führenden Instanz.
Ebenso wenig steht das **Projekt-Laden** in der Tabelle: es ist keine
Transaktion und setzt nur den Ausgangspunkt `r0` einer Sitzung (Teil 1).


### 5.12 R12 — Preset

**Der Entscheid, wörtlich:**

> R12 — Preset: eigenes Schema eq-copilot/schemas/state/nakama-preset-v1.json
> mit `preset_schema_version` 1 (nie mit dsp_schema_version vermischt; trägt
> zusätzlich die dsp_schema_version, mit der es geschrieben wurde). Inhalt:
> alle Bänder (inkl. occupied), Bypass, Mix, Schutz-Zonen, Auto-Gain, Trims,
> Width, Mono-Bass — also der Klanginhalt des DTO. Strukturell OHNE Identität:
> Zieladresse, instance_id, Bus-Name, Label, eq_enabled und Undo-Ring haben
> kein Feld. Unbekannte Top-Level-Felder werden ignoriert; unbekannte Major
> wird mit klarem Fehler abgelehnt (ein Preset ist kein Projektzustand,
> read-only-Haltung gilt nicht). Laden = eine Transaktion. Writer/Leser in C++
> mit Writer-Fixtures und Negativfixtures (Identitätsfeld → abgelehnt).
> Datei-Dialoge und Browser sind S31b.

**Feinheiten.**

1. **Zwei Versionsfelder, ein Zweck.** `preset_schema_version` = 1 sagt, wie das **Preset** gebaut ist; das mitgeführte `dsp_schema_version` sagt, welches **Parameterlayout** sein Inhalt spricht. Ein Preset aus Layout v2, gelesen von einem späteren Layout v3, ist damit einordenbar, ohne dass die Presetversion steigen müsste.
2. **Fehlende Felder statt leerer Felder.** Zieladresse, `instance_id`, Bus-Name, Label, `eq_enabled` und der Undo-Ring haben im Schema **kein** Feld. Ein optionales, leeres Identitätsfeld wäre eine offene Tür: ein Schreiber könnte es füllen, ein Leser es lesen. Ein Preset, das eines dieser Felder trägt, wird abgewiesen — mit einem Negativfixture je Feld.
3. **Unbekannte Top-Level-Felder werden ignoriert, unbekannte Major abgelehnt.** Der Unterschied ist Absicht: ein zusätzliches Feld ist eine additive Erweiterung, ein anderer Major ist eine andere Sprache. Und anders als beim Projektzustand gibt es hier **keine** read-only-Haltung: ein Preset ist keine Nutzerarbeit, die man verlustfrei bewahren müsste, sondern eine Datei, die man neu laden kann.
4. **Laden ist eine Transaktion mit einem Undo-Eintrag.** Der bestehende Undo-Ring wird **ergänzt**, nicht ersetzt: der Zustand vor dem Preset ist mit einem Undo zurück.
5. **Datei-Dialoge, Browser und das Kopieren von Sonde zu Sonde sind S31b.** Dieses Ticket liefert Schema, Writer, Leser und die Transaktion — den Weg, nicht die Hand daran.


### 5.13 R13 — `state_report.dsp`

**Der Entscheid, wörtlich:**

> R13 — state_report.dsp (NAK-110): das Feld wird definiert (Reservierung
> eingelöst, Register-Eintrag „definiert“, Feldregel :242),
> Wire-Envelope-Minor der Familie P1 steigt um eins mit Fassungsleiter (der
> zurückgebaute Leser der alten Fassung lehnt das Feld ab — beide Sprachen,
> wie der A5-Fassungsschritt es heute für Fassung 1/2 misst). Nutzlast: das
> validierte DSP-DTO (dsp_schema_version 2, global v1+v2, acht Bänder mit
> occupied, Zonen) plus abgeleitete Angaben (auto_gain_db, Klemmliste,
> verletzte Bänder). Probeeq sendet es bei Verbindungsaufbau/Reconnect und
> nach jeder Transaktion; der Broker liest, validiert, hält den bestätigten
> DSP je Sonde und überschreibt nie einen neueren lokalen Stand (§44.4:3084,
> §33.5) — Rust-Tests dazu; Gens ControlClient liest das Feld tolerant.
> `command_ack.applied_dsp` bleibt reserviert.

**Feinheiten.**

1. **Die Nutzlast ist die gehashte RFC-8785-Zeichenkette, nicht ein eingebettetes Objekt (MN1, B-07, Dirigent-Entscheid 10.09.2026).** `dsp.jcs` trägt als JSON-String **genau** den kanonischen Text, über den der `state_hash` gebildet wurde. Grund: der P1-Schemaweg führt einen **15-Ziffern-Riegel** auf Zahlenliteralen (`eq-copilot/schemas/v3/README.md:132-138`), und der Broker fährt den Textriegel **vor** der Schemaprüfung (`broker/src/coordinator/schema.rs:383-386`). Ein eingebettetes DTO-Objekt wäre damit an einem **gültigen** bestätigten Wert gescheitert — `q = 0.7071067811865476` ist auf dem DTO-/JCS-Weg ausdrücklich zulässig (`eq-copilot/plugin/state/NakamaParameter.cpp:190-197`), und Runden hätte den bestätigten Zustand verändert statt ihn zu übertragen. Als **String** passiert derselbe Text den Riegel unverändert.
   Die abgeleiteten Angaben `auto_gain_db`, Klemmliste und verletzte Bänder stehen als **eigene** Felder daneben. Sie sind nicht gehasht und unterliegen dem 15-Ziffern-Riegel wie jede andere Zahl auf diesem Weg — für einen dB-Wert und zwei Listen ist das folgenlos.
2. **Der Empfänger rechnet den Hash nach und muss es tun.** Beide Leser — C++ und Rust — bilden `SHA-256(dsp.jcs)` und vergleichen mit `state_hash`; weichen sie ab, wird der Bericht **ganz** abgewiesen (M-105). Die Zeichenkette wird mit dem **exakten Zahlenleser des DTO-Wegs** gelesen, nicht mit dem binary64-Leser des Schemawegs — sonst verlöre gerade der Wert seine letzte Ziffer, für den dieser Weg gebaut ist. **MN1 (B-07):** die vorige Fassung verbot die Nachrechnung ausdrücklich, weil die Nutzlast eine Teilmenge des DTO war. Das war die schwächere Lösung: ein Hash, den niemand prüfen kann, schützt nichts.
3. **Wann gesendet wird.** Beim Verbindungsaufbau und Reconnect (§44.4 „Ein Reconnect beginnt mit `state_report`") und **nach jeder Transaktion**. Der bestehende Sendepfad schickt einen P1-Bericht im 1-Hz-Takt nur bei Textänderung (`Verbindung.cpp:858-867`); eine neue Transaktion ändert Revision und Hash und löst damit ohnehin aus. Zusätzlich wird nach einer Transaktion **sofort** gesendet, statt bis zu einer Sekunde zu warten — sonst sähe Gen einen bestätigten Zustand mit bis zu einer Sekunde Verzug.
4. **Die Vergleichsregel im Broker.** Ein Bericht mit **kleinerer** Revision als der gehaltene überschreibt ihn nicht — aber nur **innerhalb desselben Links**. Ein neuer Link (Reconnect, Sondenneustart) setzt den Stand immer neu auf, weil die Sonde Eigentümerin ihres Zustands ist und ihre Revision beim Neustart bei 0 beginnt. Ohne diese Unterscheidung hielte der Broker nach einem Sondenneustart für immer einen toten Stand.
5. **`undo_tiefe` reist mit.** Das Feld existiert im Schema seit SONDE-010, wird aber heute nie gesendet (`Intern.h:437-447`). Mit dem Undo-Ring bekommt es seinen Wert — kein Vertragsschritt, nur eine eingelöste Zusage.


### 5.14 R14 — `Frame.band_dynamic_gain_db`

**Der Entscheid, wörtlich:**

> R14 — Frame.band_dynamic_gain_db: FlatBuffers-Feld mit der nächsten freien
> ID (22) in table Frame, Typ Vektor aus float mit genau 8 Einträgen
> (Slot-Reihenfolge) oder leer; FELD-IDS.json fortschreiben, flatc
> regenerieren (A9 Drift 0), Fixtures A10 neu erzeugen (Writer),
> Telemetrie-Schema-Minor nachziehen. Sender (Probeeq) schreibt je Frame die
> momentane dynamische Verstärkung in dB aus dem Kern (0 für freie,
> ausgeschaltete oder nicht dynamische Bänder), begrenzt auf
> ±|dynamic_range_db|, niemals aus Einstellwerten erfunden; nicht-endlich → 0
> und gezählt. Leser (C++ NakamaTelemetrie und Rust) prüfen Länge 0 oder 8 und
> Endlichkeit und weisen sonst nach den bestehenden Regeln des Rahmens ab; ein
> Frame ohne das Feld bleibt gültig (Altsender).

**Feinheiten.**

1. **Vektor, nicht acht Skalare.** `band_dynamic_gain_db:[float] (id: 22)`. Acht Einzelfelder verbrauchten acht IDs für eine Größe, die immer gemeinsam auftritt, und ein halb gefüllter Satz wäre nicht als solcher erkennbar. Länge **0 oder 8**, nichts dazwischen.
2. **Der Wert ist der letzte gerechnete des Fensters, kein Mittel.** „Momentan" heißt momentan. Ein Mittelwert glättete genau die Bewegung weg, die der User sehen soll (Abnahme 01.09.: „das band muss dynamisch mitschwingen").
3. **Das Feld reist nur, wenn es etwas zu melden gibt.** Präsent genau dann, wenn mindestens ein Slot belegt, eingeschaltet und dynamisch ist **und** der EQ-Pfad engagiert ist. Sonst abwesend — und Abwesenheit heißt weiterhin „der Erzeuger sagt es nicht", nie „acht Nullen" (Muster `integration_samples`, Feld-ID 14). Acht Nullen in jedem Frame wären Bytes ohne Aussage, in einem Strom, der mit Anzeigekadenz läuft.
4. **Nicht-endlich wird beim Sender geheilt und beim Leser trotzdem geprüft.** Der Sender ersetzt durch 0 und zählt; der Leser prüft Endlichkeit selbstständig und weist einen fremden Frame ab. Ein Leser, der sich auf den Sender verlässt, ist kein Riegel (Prüfliste C).
5. **Der P2-Schema-Minor steigt von 1 auf 2** (`TelemetryClient.h:43`). Er ist von der P1-Fassungsleiter getrennt und wird nicht mit ihr verwechselt (§2.12).


### 5.15 R15 — Kanon

**Der Entscheid, wörtlich:**

> R15 — Kanon: B6 und B7 werden angelegt und im Runner scharf (wie B5 mit
> AbPhase; die Phasenmechanik im Runner lesen, nicht raten); A16 misst neu:
> Default-Stand bitidentisch über 1000 Blöcke, 0 Latenz, 0 Tail, Save/Load
> bytegleich in v2; eq_enabled=true und bypass=true bitidentisch; nach
> eq_enabled false wieder bitidentisch nach dem Fade; die Behauptungstexte im
> Runner sagen nur, was gemessen wird (Prüfliste E). Filtergoldens ±0,05 dB
> typisch, ±0,1 dB an Extrempunkten gegen die analytische RBJ-Antwort (Vorbild
> AnalysisGoldenTestMain.cpp:745-1034), an einem festen Gitter über 20
> Hz–min(20 kHz, 0,45·fs) bei 44,1/48/96/192 kHz; CPU-Golden nach
> §44.2:2986-2989 und §49.3 nur, wenn dort ein Budget steht — sonst Messwert
> im Manifest, kein erfundener Deckel.

**Feinheiten.**

1. **„Scharf" heißt zweierlei im selben Änderungssatz:** `AbPhase` von `P6` auf `jetzt` **und** ein gebautes Ziel. Nur die Kombination macht ein fehlendes Binär zu `[FEHLT]` statt `[GEPLANT]` (`tools/beweise.ps1:1093-1107`). Wird nur die Phase gewechselt, ist der Kanon rot; wird nur gebaut, bleibt er blind.
2. **Der Filtergolden misst den Audiopfad.** Je Gitterstelle läuft ein eingeschwungener Sinus durch das echte `DspProgram`; verglichen wird gegen eine im Test **eigenständig ausgeschriebene** RBJ-Formel. Ein Vergleich der Produktkoeffizienten mit `biquadLeistung()` derselben Koeffizienten bliebe grün, selbst wenn der Audiopfad die Bank gar nicht anwendet — genau die Klasse ungültiger Rotbeweise, gegen die die Matrixprüfung von NAK-182 zweimal fand.
3. **Das Gitter der Filtergoldens:** 1/24 Oktave von 20 Hz bis min(20 kHz, 0,45·fs), zusätzlich je Prüfling die exakte Eck- oder Mittenfrequenz und bei Shelves die Plateaupunkte bei `f/4` und `4f`. Als **Extrempunkte** im Sinne der ±0,1 dB gelten die Gitterstellen unmittelbar an beiden Enden des Bereichs und der Scheitel eines Bells mit Q ≥ 12.
4. **Kein CPU-Deckel.** §49.3 nennt keine CPU-Zeile, §44.2 nennt einen Golden ohne Zahl. Der Golden hält deshalb fest, **dass** der Vier-Bank-Fall läuft und keine Bank teilt; die gemessene Last steht als Zahl im Manifest (M-118). Ein erfundener Deckel wäre auf einer schnelleren Maschine grundlos grün und auf einer langsameren grundlos rot.
5. **A16 misst danach sieben Dinge** (§4.4). Seine heutige Behauptung „kein Hostparameter" wird falsch und wandert im selben Änderungssatz mit — zusammen mit Skriptkopf und Manifestkopf (Prüfliste E, „Geänderte Zusage — drei Stellen").


### 5.16 Produktfragen — benannt, einer Fläche zugeordnet, hier nicht beantwortet

Diese Punkte berührt das Ticket, entscheidet sie aber nicht. Sie sind keine
Matrixzeilen, weil sie hier nichts verlangen; sie stehen hier, damit niemand
sie in Etappe 2 bis 4 beiläufig beantwortet.

| Frage | Fläche und Stand |
|---|---|
| Der **zweistufige Mix** — ganze App gegenüber gewählter Spur, Klickumschaltung am Knopf (Interview `:229-231`) | **Gen**, S28b und S31b. Hier entsteht ausschließlich der Sonden-Mix. Blueprint §15 Punkt 7 führt die genaue Zustands- und Umschaltsemantik weiterhin als offen. |
| Wie der **abgeleitete Auto-Gain-Wert sichtbar mitwandert** (Abnahme 24.08.: „der angezeigte Output-Wert wandert dabei sichtbar mit") | **S31b.** Dieses Ticket liefert den lesbaren Wert und sein Feld im Bericht; wie er angezeigt wird, ist Oberfläche. |
| Ob „**vollwertig, hochwertig**" (Erratum d) später mehr verlangt als der erste Kern aus §30.1 | **Nicht entschieden**, ausdrücklich im Erratum als Befund vermerkt. Dieses Ticket baut den ersten Kern und nichts darüber hinaus. |
| Der **Zehn-Sekunden-Kandidat** samt Kandidaten-ID, Timer, ACK, Reconnect und Ablauf (Erratum o, Blueprint §15 Punkt 12) | **S29–31 und S31b.** Hier entsteht nur der Candidate-**Pfad** im DSP (zweite Bank, dritter Tap), nicht sein Bedienvertrag. |
| Die **direkte Edit-Transaktion** — Band-Drag, Textwert, Preset-Load und Curve-Copy auf eine gemeinsame Abbildung (Blueprint §15 Punkt 14) | **S31b.** Dieses Ticket liefert die Transaktionsseite; die Gestenseite fehlt bewusst. |
| Das **Kopieren einer Kurve von Sonde zu Sonde** (U18) | **S29–31** über die bestehende Fernsteuerung, Bedienung in S31b. Hier entsteht nur das Presetobjekt, das dabei reist. |

### 5.17 Abweichungen vom Wortlaut der Regeln, gesammelt

Acht Stellen bauen etwas anderes, als der Wortlaut einer Regel sagt; **zwei
sind zurückgenommen** — Nummer 3 seit der Matrix-Nacharbeit 1, Nummer 6 seit
der Matrix-Nacharbeit 3. Beide bleiben als Verlauf stehen, damit die
Wiederprüfung die Kehre sieht. Heute wirksam sind also **sechs**. Jede steht mit
ihrer Begründung auch unter ihrer Regel; hier stehen sie zusammen, damit die
Prüfung sie an einer Stelle findet. Drei fallen aus dem Muster: **Nummer 6**
weicht nicht vom Wortlaut einer R-Regel ab, sondern von einer früheren Regel
des Dirigenten — er hat sie in der Nacharbeit 3 selbst zurückgenommen. **Nummer
7** weicht von keiner Regel ab, sondern von der bis dahin gewählten **Form**;
sie ist der Wegwechsel der Konvergenzrunde und steht hier, weil eine
Formänderung dieser Größe genauso begründungspflichtig ist wie eine inhaltliche.
**Nummer 8** weicht vom Wortlaut der Protokollspezifikation ab, die Nummer 7
geschaffen hat: die NAK-245-Runde schließt fünf innere Widersprüche ihrer
Tabelle, bevor Etappe 4 gegen sie baut.

| Nr. | Regel und Wortlaut | Was gebaut wird | Begründung |
|---|---|---|---|
| 1 | **R1 / §53.8:** das Kind `Dsp` trägt „Revision, Schutz, bestätigter DspState, Undo-Ring" | `Dsp` trägt Revision, die acht `occupied`, die Zonen und den Undo-Ring — **keine** zweite Kopie der 112 Werte. Der bestätigte DspState ist `Parameters` plus `Dsp`. | Zwei Kopien derselben Werte können auseinanderlaufen; §33.5 verbietet eine zweite Wahrheit, und ein Leser müsste bei Widerspruch raten. Zusätzlich sprengte eine zweite Wertekopie den Knoten- und Byteheadroom des State-Lesers nicht, wohl aber die Klarheit. |
| 2 | **R6:** Zonengrenzen „mit 20 Hz ≤ low < high ≤ min(20 kHz, 0,45·fs) im DTO geprüft" | Das **DTO** prüft 20 Hz ≤ low < high ≤ 20 kHz; die **Laufzeit** kappt zusätzlich auf 0,45·fs, ohne den persistenten Wert zu ändern. | Die Samplerate ist beim Laden nicht bekannt. Eine samplerateabhängige DTO-Grenze machte einen bei 96 kHz gespeicherten Stand bei 44,1 kHz unlesbar — ein echter Save/Load-Bruch. Genau so hält es `freq_hz` seit v1 (`nakama-parameter-v1.json:13`). |
| 3 | **R13:** die Nutzlast ist „das validierte DSP-DTO" · **R2:** „`state_report.eq_enabled` bleibt reserviert; hier reist nichts davon" | **ZURÜCKGENOMMEN in MN1 (B-07).** Gebaut wird jetzt `dsp.jcs` — genau die gehashte RFC-8785-Zeichenkette, einschließlich `v2.global.eq_enabled` —, und beide Leser rechnen den Hash nach. Eine Teilmenge auf dem Draht gibt es nicht mehr. | Die vorige Fassung nahm den Schlüssel aus der Nutzlast heraus und musste dafür verbieten, den Hash nachzurechnen (damaliges M-105). Der Dirigent-Entscheid zu B-07 verlangt die hashverifizierbare Zeichenkette; eine Projektion ist damit unvereinbar. R2 bleibt gewahrt, weil kein **Feld** des reservierten Namens entsteht — und die Reservierungsregel schützt ausdrücklich den NAMEN (`reservierte-nachrichten-v1.json:242`). Ein Hash, den niemand prüfen kann, war die schwächere Lösung. |
| 4 | **R9:** „Nicht-endliche Eingangssamples … werden verriegelt und gezählt (Filterzustände bleiben endlich)" | Je Sample ein `isfinite` auf dem **Eingang**; je **Blockrand** eine Prüfung der Filterzustände mit Nullung und eigenem Zähler. | Alle Zustände je Sample zu prüfen kostet bei acht Bändern mal vier Kanalkomponenten ein Vielfaches der Filterarbeit selbst. Die zweistufige Fassung hält die Zusage und deckt zusätzlich den Fall, in dem ein Zustand ohne nicht-endlichen Eingang entgleist (extreme Q nahe Nyquist). |
| 5 | **R4:** der Ausgleich folgt der „pink-gewichteten mittleren **Betragsantwort**" | Gemittelt wird die **Energie** `\|H\|²` über dieselben 121 Gitterstellen, danach `−10 · log₁₀`. | Beide von R4 genannten Proben halten in beiden Lesarten exakt; sie trennen sich erst bei schmalen Merkmalen. Ein +12-dB-Bell mit Q 12 ergibt im Betragsmittel −0,28 dB, im Energiemittel −0,95 dB. Wahrgenommene Lautheit folgt der Energie in den Frequenzgruppen; das Betragsmittel unterkompensierte eine schmale Anhebung hörbar. |
| 6 | **MN2-Regel zu B-05:** „Das gespeicherte Ergebnis umfasst deshalb auch abgewiesene Transaktionen (Konflikt, User-Schutz)" | **ZURÜCKGENOMMEN in MN3 (D-Regel des Dirigenten).** Gemerkt wird ausschließlich, was eine Revision erzeugt hat; ein Ausgang ohne Commit hinterlässt keinen Eintrag, und dieselbe ID darf danach erneut laufen. Das Fenster ist in **Revisionen** definiert (höchstens 32 hinter der aktuellen), nicht in Einträgen. | Die zurückgenommene Fassung erzeugte zwei Widersprüche, die die Matrixprüfung 3 gefunden hat. **D-1:** ein Ring fester Eintragszahl, der auch Abweisungen aufnahm, verdrängte eine committete Transaktion T schon nach 31 Abweisungen — obwohl seit T erst **eine** Revision vergangen war und Undo T noch erreichte. Die zugesagte Spanne „über die ein alter Zustand per Undo noch erreichbar ist" war damit gebrochen. **D-2:** ein gemerktes `busy_retry` kam bei jeder Wiederholung derselben ID zurück, auch nachdem ein Audio-ACK längst eine Bank freigegeben hatte — der in M-44 zugesagte Weg „Der Aufrufer wiederholt" war unausführbar. Die neue Regel löst beides mit **einem** Satz, weil beide Widersprüche denselben Ursprung hatten: gemerkt wurde etwas, das gar keinen Zustand erzeugt hat. |
| 7 | **Die Arbeitsweise selbst:** Transaktionssemantik als Prosa in den Feinheiten von §5.11 | **Eine geschlossene Protokollspezifikation** (§5.11.4) mit Arithmetik, Stufenfolge, sechs Invarianten und einer vollständigen Falltabelle T1 bis T12. Die Matrixzeilen M-44, M-74, M-75, M-76, M-123, M-124 und M-125 werden daraus **abgeleitet** und zitieren sie; jede Zahl steht genau einmal. | Konvergenzentscheid des Dirigenten (10.09.2026, §7.7) nach drei Wiederprüfungen ohne PASS. Die Ursache war nicht ein einzelner falscher Satz, sondern die **Form**: drei Runden zogen Prosa nach, und jede schloss einen Satz und öffnete den nächsten Randfall (Verdrängung → `busy_retry` → inklusiver Fensterrand → `base_revision` beim Retry). Eine Reihenfolge, die an zwei Stellen steht, läuft auseinander; eine Zahl, die an zwei Stellen steht, widerspricht sich. Deshalb ist §5.11.4 ab hier die einzige Quelle, und die Zeilen tragen keine eigene Arithmetik mehr. |
| 8 | **§5.11.4 in der Fassung der Konvergenzrunde:** „die Zeilen T1 bis T12 sind vollständig und paarweise unvereinbar, weil S0 bis S5 in fester Reihenfolge geprüft werden“ (I6); „`\|R\| = min(r, 32)`, immer“ (I1); T4 „User-Schutz“ für jede Transaktion, die ein Band neu in eine Zone führt; T9 „Commit“ allein wegen der aktualisierten `base_revision`; M-76 „gefahren als tabellengetriebener Lauf über T1 bis T12“ | **Die NAK-245-Runde** (§7.10) hält die Form aus Nummer 7 und schließt fünf innere Widersprüche derselben Tabelle: eigene Zeilen für Fehler in S4, S6 und S7 (T13 bis T15), während S5 an der Quelle nur `busy_retry` kennt und keine Bank vor dem Commit-Punkt belegt; T4 nur für Bedienänderungen, Ganzzustands-Wiederherstellungen als eigene Zeile (T16, neue Matrixzeile M-126); T7 und T9 committen nur, wenn S2 bis S7 bestehen; der Ladestart `r0` in I1 und I2 samt der Wiederholung über die Sitzungsgrenze (T17); M-76 misst nur noch den Ein-Revisions-Satz aus §44.5, den Tabellenlauf trägt allein M-125. | Matrixprüfung 5 (§7.9) fand die fünf Widersprüche; der Dirigent hat sie als DEFEKT eingeordnet und als NAK-245 vor dem Bau der Etappe 4 in derselben Tabelle schließen lassen, weil der Transaktionskern (B7) gegen genau diese Tabelle gebaut und von M-125 gegen sie gefahren wird. Die Vollständigkeit aus Nummer 7 galt für die zwölf beschriebenen Fälle, nicht für jede Eingabe: ein Fehler in S7 hatte keine Zeile, ein korrektes Undo unter einer Zone wäre an T4 gefallen und ein korrekter Kern am Rotbeweis von M-76. |

---

## 6. Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für Etappe 1

> **Fortgeschrieben am 10.09.2026:** die Prüfliste für die **Matrix-Nacharbeit 1**
> steht in §7.2. Dieser Abschnitt bleibt als Stand der Etappe 1 vor der
> Matrixprüfung 1 stehen und wird nicht umgeschrieben.

Etappe 1 ändert **keinen** Produkt-, Test-, Schema-, Fixture- oder
Werkzeugcode. Die Zeilen A bis D und F beschreiben Eigenschaften von Code, den
diese Etappe nicht anfasst; sie werden hier deshalb als **für die Bauetappen
verplant** abgehakt, mit dem Ort, an dem sie fallen. E gilt für dieses
Manifest selbst.

| Zeile | Für Etappe 1 | Wo sie in den Bauetappen gemessen wird |
|---|---|---|
| **A** Rückstau und Prioritätsklassen | Nicht berührt — kein IPC-Code in diesem Änderungssatz. | Der `state_report`-Sendepfad ist P1 und nutzt die bestehende Politik unverändert; M-106 misst den Sendezeitpunkt, nicht eine neue Klasse. Beine B10, A4. |
| **B** Lebenszyklus | Nicht berührt. | Verbinden↔`state_report` liegt als Paar in M-106; Aktivieren↔Abklingen in M-03 und M-04; Bank-Ownership vollständig in M-42 bis M-46. |
| **C** Verträge und Längen | Nicht berührt. | Feldmenge exakt (M-91, M-97), Discriminator vor Inhalt (M-102), NaN und Grenzen an jeder Zahl über den Draht (M-113), Negativfixture je Definition (M-73, M-97, M-112). |
| **D** Bau- und Prüfriegel | Nicht berührt. | Fail-closed ohne Rohtextheuristik und Frischeprüfung: M-115 (Phase und Ziel im selben Satz), §4.7 Punkt 4 (`Copy-Item`-Zeitstempel), §4.7 Punkt 5 (`cmake` nicht im Bash-`PATH`). |
| **E** Behauptung ≤ Messung | **Gilt hier.** Dieses Manifest behauptet **kein** gemessenes Produktverhalten: der Kopf trägt „Urteil offen", §2 nennt zu jeder Aussage Datei und Zeile am Basis-SHA, §3.15 zählt aus den Zeilen statt aus einer anderen Datei, und §4.6 listet vorab, welche Zahlen beim Abschluss gemessen werden. Die einzige eigene Messung dieser Etappe ist `git diff --stat 1867cac2..e9dbf4b9 -- eq-copilot broker tools` (leer) im Kopf. | M-116 (Runner-Behauptungen), M-117 (Rotbeweis je neuer Prüfung), M-94 (Writer-Fixtures statt Handschrift), §4.6 (gemessene Zahlen). |
| **F** Änderungssatz | Nicht berührt. | §4.8 letzter Punkt listet die fünf Paare; §4.2 hält Writer, Leser, Migration, Fixtures und Cross-Language-Verbraucher in **einem** Block. |

**Selbstaudit dieser Etappe.** Gegen den Diff gelesen: das Manifest ist die
einzige geänderte Datei (`git status --short`), es trägt kein Urteil und keine
Prüfmarke, es setzt das Feld `beleg` in `docs/plan/plan.json` **nicht** (das
gehört an den ersten Bau-Commit), und es fasst weder `docs/offene-punkte.md`
noch `docs/PLAN-STAND.md` noch `design/` an. Der `dokuriegel` läuft sauber; die
sechs benannten Verweise sind Zielpfade aus §4.0, die die Bauetappen anlegen.
Die 120 Matrix-IDs sind lückenlos von M-01 bis M-120 vergeben, jede genau
einmal, und jede Zeile trägt sieben Spalten.

---

## 7. Matrixprüfung 1 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend (`codex exec review`, Sandbox read-only); Thread `01a08848-73c2-7d20-a765-733d79fac37b`; Lauf 00:27–00:43 |
| Prüfbereich | `git diff e9dbf4b9...047b4dc5 -- docs/beweise/SONDE-015.md` (Vorlage: Matrixprüfung nach Variante A); HEAD während des Laufs `68310869` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-matrixpruefung-1-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-matrixpruefung-1-047b4dc.txt` |
| Urteil | **NEEDS_WORK** — zwölf Befunde [P2], vom Prüfer als DEFEKT eingeordnet |
| Quellencheck des Dirigenten | lesender Opus-Agent, jede zitierte Zeile geöffnet: alle zwölf Zeilenangaben stimmen, alle zwölf Befunde sind **DEFEKT** (keine erfundene Anforderung; Präzisierung zu B-07: `README.md:132-138` erlaubt den Wert auf dem DTO-Weg, der Befund trägt, weil die Nutzlast über den Schemaweg mit `schema.rs:383-386` reist) |
| Rundenbilanz | `e9dbf4b9..047b4dc5: Doku 1 Datei(en) +1901/-0 → OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)` — Matrixrunde vor dem Bau, zählt strukturell null und löst keinen Konvergenzentscheid aus (Dirigent §3.3) |
| Nacharbeit | Matrix-Nacharbeit 1 als Zusatzauftrag an den Bauer: `docs/beweise/roh/SONDE-015-matrix-nacharbeit-1-auftrag.txt` (nur die zwölf Defekte, je Defekt die schließende Regel) |
| Prozessentscheid | User-Wort 10.09.2026 (während dieses Laufs): „codex immer astra mit xhigh oder max effort. kein Sol mehr" — eingetragen im Dirigenten-Skill §3.4, Commit `a026c44f` |

| Befund | Kurzform | Einordnung | Ansatzpunkt |
|---|---|---|---|
| B-01 | Hash und Undo liegen hinter dem Commit-Punkt (§5.11.2, M-74/M-75 ohne Einspritzpunkt an Hash/Undo) | DEFEKT (R11 „Hash unverändert"; §5.11.2 widerspricht M-74) | §5.11.2, M-74, M-75 |
| B-02 | Undo nach Remove unter einer Zone würde vom Zonenriegel abgewiesen | DEFEKT (Fünferblock-02 `:139-141`, M-61) | M-61, §5.6.3, §5.11.2 |
| B-03 | Prädikat prüft die Zonenmenge, nicht die einzelne Zone (Band von Zone A nach Zone B) | DEFEKT (R6, Interview 23.08. `:132-133`) | §5.6.3, M-67 |
| B-04 | Auto-Gain-Faltung multipliziert Mid und Side in beide Seiten (H² statt H) | DEFEKT (R4, Abnahme 24.08. `:54`) | §5.4.3, M-37 |
| B-05 | In-Flight-Register des ControlClient ist kein lokaler Transaktionsriegel | DEFEKT (R11, §44.5; Code `Nachrichten.cpp:157-184, 301-304, 335-336`) | §5.11.4, M-76 |
| B-06 | Kein Rotbeweis gegen veraltete Dynamic-Telemetrie | DEFEKT (Gate §1.1 „gegen stale … abgesichert") | §3.13, neue Zeile |
| B-07 | 15-Ziffern-Textriegel des Schemawegs trifft die DTO-Nutzlast | DEFEKT (bestehender Vertrag `README.md:132-138`, `schema.rs:383-386`) | M-103, §5.13 |
| B-08 | Neutraler Active-Nulltest läuft mit `eq_enabled=false` | DEFEKT (NAK-81 b, `nakama-parameter-v1.json:15`) | M-02 |
| B-09 | Slot-Kaskaden-Rotbeweis unterscheidet Kaskade nicht von Parallelsumme | DEFEKT (Rotbeweis-Regel des Auftrags `:3`) | M-15 |
| B-10 | Kein echt paralleler Lauf am neuen Bankpool; B4 fährt Gens Prozessor | DEFEKT (Entwurf §44.5 `:3092-3093`) | §3.5, neue Zeile |
| B-11 | M-88 (APVTS) in der Pflichtmenge der Etappe 2, obwohl die APVTS erst in Etappe 4 entsteht | DEFEKT (innerer Widerspruch §4.2 gegen §4.1/§4.4) | §4.2 |
| B-12 | Ticketpfade ohne `schema.rs`, Rust-Codegen-Ziel und `pruefe_v2_schemas.py`; A11 prüft kein Presetschema | DEFEKT (M-102, M-109, M-95 nicht erfüllbar) | §4.5, M-95 |

Lücken, die der Dirigent in derselben Runde entschieden hat (Technik, in den Regeln des Nacharbeitsauftrags): Zonenriegel bewacht Bedienänderungen, nicht Ganzzustands-Wiederherstellungen (B-02); `state_report.dsp` trägt das DTO als die gehashte RFC-8785-Zeichenkette, hashverifizierbar in beiden Lesern (B-07); äquivalenter Stresslauf gilt, wo die Toolchain keinen ThreadSanitizer trägt (B-10). Härtungen: keine.

### 7.1 Matrix-Nacharbeit 1 — was eingearbeitet wurde (10.09.2026)

Auftrag wörtlich: `docs/beweise/roh/SONDE-015-matrix-nacharbeit-1-auftrag.txt`
(nur die zwölf Defekte, je Defekt die schließende Regel des Dirigenten).
Startstand `e5a1cb20`, Worktree sauber. **Kein Produkt-, Test-, Schema-,
Fixture- oder Werkzeugcode**; einzige geänderte Datei ist dieses Manifest.

Die Matrix ist vor dem Bau **Spezifikation und kein Verlauf**: betroffene
Zeilen sind deshalb an Ort und Stelle korrigiert und tragen in der Spalte
Belegklasse die Marke **MN1**, damit die Wiederprüfung sie findet. Neue Zeilen
haben die nächsten freien IDs ab **M-121**; keine ID ist umgewidmet.

| Befund | Eingearbeitet in | Was sich geändert hat |
|---|---|---|
| **B-01** | §5.11.2, **M-74**, **M-75** | Alle fehlbaren und allozierenden Stufen liegen jetzt vor **einem** Commit-Punkt: validieren → Zonenriegel → Transaktions-/Revisionsriegel → Kandidaten-DTO → Programm → **`state_hash`** → **Undo-Eintrag** → COMMIT-PUNKT → Revision, Tausch, Dirty, Bericht. M-74 trägt die Liste wörtlich; M-75 hat **sieben** Einspritzpunkte statt vier, darunter Hashbildung und Undo-Vorbereitung, mit Rotbeweis an „Hash unverändert". Belegt an der Quelle: `stateHash` ruft `dtoKanon`, baut einen `MemoryBlock` und kann mit `grund` scheitern (`eq-copilot/plugin/state/NakamaParameter.cpp:163-183`). |
| **B-02** | **M-61**, §5.6.3, §5.11.2 | Neue Ausnahme (Dirigent-Entscheid): der Zonenriegel bewacht **Bedienänderungen**. Undo, Redo, Projekt-Laden und Preset-Laden ersetzen einen ganzen, bereits bestätigten Zustand und passieren den Riegel **nicht** erneut; eine dabei entstehende Lage in einer Zone wird als Verletzung **gemeldet** wie in M-68. M-61 führt den Pflichtweg jetzt ausdrücklich: Band ein → Zone darüber ein → Remove → unmittelbar Undo. |
| **B-03** | §5.6.3, **M-67** | Das Prädikat wird **je Zone einzeln** ausgewertet: abgewiesen wird, sobald für irgendeine eingeschaltete Zone der Eintritt dieses Slots von falsch auf wahr kippt. Neuer Rotbeweis: Band bei 100 Hz, Zone A 80–120 Hz nachträglich ein, Zone B 900–1100 Hz ein, Frequenzänderung auf 1000 Hz → **abgewiesen**. |
| **B-04** | §5.4.3, **M-37** | Mid und Side gehen als getrennte Kanalkomponenten ein; die je Seite wirksame Übertragung trägt den Diagonalterm `(H_M + H_S)/2`. Ein identisches Mid/Side-Paar fällt damit exakt auf den Stereo-Fall zusammen. Neuer Golden: zwei identische +6-dB-Shelves auf `mid` und `side` → **−6 dB ±0,1 dB** statt −12 dB. Der ausgelassene Kreuzterm ist benannt und mit Zahlen begründet. |
| **B-05** | §5.11.4, **M-76**, **M-123** (neu) | Die Konflikt- und Wiederholungserkennung liegt im **lokalen** Transaktionskern: Transaktions-ID plus `base_revision`. Das In-Flight-Register des `ControlClient` ist ab hier nur noch der Fernweg-Anteil, der es ist (`controlclient/Nachrichten.cpp:157-184`, `:301-304`, `:332-336`). M-76 läuft fernsteuerungsfrei; **M-123** trägt die Wiederholung nach Abschluss mit dem Fenster **32** — die Tiefe des Undo-Rings und die einzige Grenze, die der Vertrag setzt. |
| **B-06** | **M-121** (neu), §3.13 | Jeder Vektoreintrag ist seinem Analysefenster zugeordnet und trägt die Generation seines Programms; bei Programmwechsel, Remove, Neubelegung, Typwechsel oder `dynamic_enabled` aus wird der gehaltene Wert **verworfen** (0). Rotbeweis: die alte Auslenkung wird nach dem Programmwechsel erneut gesendet und besteht dabei Länge, Endlichkeit und Anwesenheitsprüfung. |
| **B-07** | §5.13.1, §5.13.2, **M-103**, **M-104**, **M-105**, §2.12, §5.2.5, §5.17 Nr. 3 | `state_report.dsp` trägt das DTO als **`dsp.jcs`** — genau die gehashte RFC-8785-Zeichenkette —, und beide Leser rechnen `SHA-256(dsp.jcs)` nach und weisen bei Abweichung ab. Grund an der Quelle: der 15-Ziffern-Riegel des Schemawegs (`eq-copilot/schemas/v3/README.md:132-138`) läuft **vor** der Schemaprüfung (`broker/src/coordinator/schema.rs:383-386`) und hätte ein eingebettetes Objekt mit `q = 0.7071067811865476` abgewiesen, obwohl der DTO-Weg den Wert ausdrücklich erlaubt (`NakamaParameter.cpp:190-197`). **Folge: Abweichung 3 ist zurückgenommen** — `v2.global.eq_enabled` reist innerhalb der Zeichenkette mit; es entsteht kein Feld des reservierten Namens. |
| **B-08** | **M-02** | Die Zeile läuft jetzt mit `eq_enabled` = **true**, `bypass` = false und sonst neutralen Defaults durch den echten Audiopfad. Nur dieser Zustand löst NAK-81 (b) ein; mit `eq_enabled` = false endete der Weg vor Trims, M/S und Filterbank und die Zeile wäre auch ohne jeden Unity-Kurzschluss grün geblieben. Der ausgeschaltete Fall bleibt M-01. |
| **B-09** | **M-15** | Gemessen wird der Gesamtfrequenzgang gegen das **Produkt** der analytischen Einzelantworten (Parallelsumme fällt), und die Slotreihenfolge an **nicht kommutierenden** Kombinationen: dynamisches Band vor statischem, zwei Bänder mit verschiedenem `channel_mode`. Der vorige Rotbeweis verglich identische Bänder und konnte Kaskade von Parallelsumme nicht unterscheiden. |
| **B-10** | **M-122** (neu), §3.5 | Eigene Zeile für den echt parallelen Lauf von Control-Worker und Audiothread am neuen Bankpool über Publikation, Fade, ACK und Slot-Wiederverwendung, Invarianten je Block geprüft. Entwurf §44.5 sagt „ThreadSanitizer- beziehungsweise **äquivalente** Stressläufe"; trägt die Toolchain keinen ThreadSanitizer, ist der äquivalente Lauf der Nachweis, und die Zeile sagt das. Rotbeweis: ein vorgezogenes Reclaim vor dem ACK fällt. Dass **B4** den `EqCopilotProcessor` fährt und diesen Nachweis nicht tragen kann, ist in der Zeile vermerkt (`eq-copilot/plugin/tests/QueueStressTestMain.cpp:9-24`). |
| **B-11** | §4.2, §4.4 | **M-88** ist aus der Pflichtmenge der Etappe 2 entfernt und liegt jetzt in Etappe 4; ebenso die Prozessorhälfte von M-89. Etappe 2 behält von M-86 bis M-94 die reinen Vertrags- und State-Zeilen. §4.4 nennt die hinzugekommenen Zeilen ausdrücklich. |
| **B-12** | §4.5, **M-95** | Ticketpfade ergänzt um `broker/src/coordinator/schema.rs` (die Fassungsleiter trägt heute `MINOR_0` bis `MINOR_4` und `JSON_SCHEMA_MINOR_AKTIV` = 4; Minor 5 braucht dort einen Zweig), `broker/src/generiert/nakama_telemetry_v1_generated.rs` (Rust-Codegen-Ziel aus `eq-copilot/schemas/v3/flatbuffers/WERKZEUG.json`) und `tools/eq-copilot/pruefe_v2_schemas.py`. **M-95 nennt jetzt A12** statt A11: `erzeuge_state_fixtures.py` liest `eq-copilot/schemas/state/` bereits (`:58`), während `pruefe_v2_schemas.py` eine eingefrorene Familie von fünf Dateien unter `eq-copilot/schemas/` prüft (`:38-45`) und `schemas/state/` nicht sieht. |

**Zählung nach der Nacharbeit** (§3.15, aus den Zeilen gemessen): **123**
Matrixzeilen, lückenlos M-01 bis M-123, jede mit sieben Spalten; 76 BELEGT,
78 ENTSCHIEDEN, 123 BAULÜCKE, 0 OFFEN, **15** mit der Marke MN1 (zwölf
korrigierte und drei neue).

**Was diese Runde nicht getan hat.** Sie hat keinen Befund des Prüfers
bestritten, keine Härtung am Prüfwerkzeug vorgenommen (es gab keinen Befund
gegen eines) und keine Zeile geschlossen, die der Prüfer nicht genannt hat.
Drei Stellen sind über den Wortlaut der zwölf Defekte hinaus mitgezogen, weil
sie sonst widersprüchlich geblieben wären: §2.12 (Begriffstabelle `state_hash`
gegen `dsp.jcs`), §5.2.5 (`eq_enabled` reist jetzt mit) und §5.17 Nr. 3
(Abweichung zurückgenommen). Alle drei sind Folgen von B-07 und in der Tabelle
oben dort vermerkt.

**Selbstaudit dieser Runde.** Gegen den Diff gelesen: die zwölf Regeln sind
einzeln abgearbeitet und je Befund in der Tabelle oben verortet. Jede
zitierte Quellstelle ist in dieser Sitzung geöffnet und bestätigt —
`NakamaParameter.cpp:163-183` und `:190-197`, `Nachrichten.cpp:157-184`,
`:301-304`, `:332-336`, `README.md:132-138`, `schema.rs:343-372` und
`:383-386`, `QueueStressTestMain.cpp:9-24`, `WERKZEUG.json`,
`pruefe_v2_schemas.py:38-45`, `erzeuge_state_fixtures.py:58`,
`eq-ipc-v3.schema.json:2321`. Die Zeilennummern in §2 und §5 sind unverändert
gültig, weil kein Quellcode angefasst wurde. Keine Zeile behauptet ein
gemessenes Produktverhalten; der Kopf trägt weiter „Urteil offen". Der
`dokuriegel` läuft sauber. Die drei neuen Zeilen tragen keine ID, die es schon
gab, und keine bestehende ID hat ihre Bedeutung gewechselt.

**Ein Widerspruch, der in dieser Runde aufgelöst wurde und benannt gehört.**
Die Regel zu B-07 (hashverifizierbare Zeichenkette) und meine frühere
Abweichung 3 (Nutzlast ohne `v2.global.eq_enabled`) sind **nicht gleichzeitig
erfüllbar**: wer den Schlüssel aus dem übertragenen Text schneidet, macht
`SHA-256(dsp.jcs) == state_hash` unprüfbar. Der Entscheid des Dirigenten
gewinnt, die Abweichung ist zurückgenommen, und R2 bleibt gewahrt, weil kein
**Feld** des reservierten Namens entsteht — die Reservierungsregel schützt den
NAMEN (`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json:242`). Die
Alternative, `eq_enabled` ganz aus dem DTO zu nehmen, ist verworfen: dann läge
der Schalter, der über bitidentisch entscheidet, nicht im Hash, und zwei
hörbar verschiedene Zustände könnten denselben Hash melden — gegen §44.5.

### 7.2 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Matrix-Nacharbeit 1

| Zeile | Für diese Runde |
|---|---|
| **A** Rückstau und Prioritätsklassen | Nicht berührt — kein IPC-Code. Die Nutzlastform aus B-07 bleibt ein P1-Bericht auf dem bestehenden Weg; die Klasse ändert sich nicht. |
| **B** Lebenszyklus | Nicht berührt. Das in B-02 neu geregelte Paar Remove↔Undo liegt vollständig in M-60 und M-61, im selben Abschnitt. |
| **C** Verträge und Längen | Gilt für die Spezifikation: B-07 nennt für `dsp.jcs` den exakten Leser und die Hashprüfung in **beiden** Sprachen (M-105) und verlangt ein Fixture mit mehr als 15 signifikanten Ziffern (M-103). B-12 ergänzt die fehlenden Gegenstücke, damit beide Vertragsseiten zusammen bleiben. |
| **D** Bau- und Prüfriegel | B-12 nennt die Fassungsleiter und das Codegen-Ziel, ohne die A9 und M-102 nicht fail-closed wären. |
| **E** Behauptung ≤ Messung | **Die tragende Zeile dieser Runde.** B-08, B-09 und B-10 waren alle drei derselbe Fehlertyp: eine Prüfung, die ihre Zusage nicht treffen kann und deshalb auch ohne den Fix grün bleibt. B-01 und B-06 ergänzen fehlende Einspritz- beziehungsweise Invalidierungspunkte. §3.15 ist neu **gezählt**, nicht fortgeschrieben. |
| **F** Änderungssatz | B-12 stellt genau das her: `schema.rs` gehört zu M-102, das Rust-Codegen-Ziel zu M-109, und das Presetschema zu dem Bein, das es tatsächlich liest. |

### 7.3 Matrixprüfung 2 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08871-cc40-7e51-a554-a055b60212c7`; Lauf 01:12–01:21 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff e5a1cb20...59dbcaaf -- docs/beweise/SONDE-015.md`; HEAD während des Laufs `c0f7afca` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-matrixpruefung-2-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-matrixpruefung-2-59dbcaa.txt` |
| Urteil | **NEEDS_WORK** — B-01 bis B-04, B-06 bis B-08, B-10 bis B-12 **geschlossen** (zehn von zwölf, je mit Beleg); **B-05** und **B-09** offen; §3.15 stimmt mit 123 IDs und 15 MN1-Zeilen überein; kein weiterer Bruch |
| Quellencheck des Dirigenten | direkt an M-74 (`:1145`), M-123 (`:1148`), §5.11.2 Punkt 2 (`:1771`) und M-15 (`:1050`): beide Befunde bestätigt, beide **DEFEKT** — B-05: Zonenriegel steht vor der Transaktions-ID-Prüfung, M-123 verspricht „dasselbe Ergebnis"; B-09: der Rotbeweis „beide Reihenfolgen liefern verschiedene Ergebnisse" trennt 0→7 nicht von 7→0 |
| Rundenbilanz | `e5a1cb20..59dbcaaf: Doku 1 Datei(en) +167/-47 → OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)` — Matrixrunde, strukturell null |
| Nacharbeit | Matrix-Nacharbeit 2: `docs/beweise/roh/SONDE-015-matrix-nacharbeit-2-auftrag.txt` (nur B-05 und B-09; Regeln: Wiederholungserkennung als erste Stufe vor jedem zustandsabhängigen Riegel, gespeichertes Ergebnis auch für abgewiesene Transaktionen; Richtungsbeweis gegen ein unabhängig gerechnetes geordnetes Golden mit Rückwärtslauf-Mutant) |

### 7.4 Matrix-Nacharbeit 2 — was eingearbeitet wurde (10.09.2026)

Auftrag wörtlich: `docs/beweise/roh/SONDE-015-matrix-nacharbeit-2-auftrag.txt`
(nur die zwei offenen Defekte, je Defekt die schließende Regel des Dirigenten).
Startstand `c37ab19b`, Worktree sauber. **Kein Produkt-, Test-, Schema-,
Fixture- oder Werkzeugcode**; einzige geänderte Datei ist dieses Manifest.
Geänderte Matrixzeilen tragen die Marke **MN2**; die neue Zeile hat die nächste
freie ID **M-124**, keine ID ist umgewidmet.

| Befund | Eingearbeitet in | Was sich geändert hat |
|---|---|---|
| **B-05** (Runde 2) | §5.11.2 Punkt 2, §5.11.4 Punkt 4, **M-74**, **M-75**, **M-76**, **M-123**, **M-124** (neu) | Die **Wiederholungserkennung ist die erste Stufe** jeder Transaktion, vor jedem zustandsabhängigen Riegel: Transaktions-ID im Fenster nachschlagen → Treffer liefert sofort das gespeicherte Ergebnis und durchläuft **keine** weitere Stufe. Erst eine unbekannte ID geht in die Kette validieren → Zonenriegel → `base_revision` → … → Commit-Punkt. **Gespeichert wird jedes terminale Urteil**, auch das abgewiesene (Konflikt, User-Schutz), damit eine Wiederholung nie ein anderes Urteil bekommt als das Original — dafür ist **M-124** neu. M-123 trägt den Pflicht-Rotbeweis aus dem Urteil (1000 Hz, Zone 900–1100 Hz nachträglich ein, T auf 1050 Hz, weitere Transaktion auf 500 Hz, T wiederholt → T's Ergebnis, **kein „User-Schutz"**). M-75 sagt ausdrücklich, dass die Nachschlagestufe **nicht allozierend und nicht fehlbar** ist und deshalb keinen Einspritzpunkt bekommt — sonst wäre die vorgezogene Stufe selbst ein Weg an allen Garantien aus B-01 vorbei. |
| **B-09** (Runde 2) | **M-15**, §5.3.1 | Die Richtung **0 → 7** wird gegen ein **unabhängig gerechnetes, geordnetes Referenzergebnis** belegt: der Test wendet die nicht kommutierende Kombination (dynamisches Band Slot 2 vor statischem Slot 5; `left` Slot 1 vor `mid` Slot 4) Sample für Sample mit denselben Filterprimitiven in der festgelegten Reihenfolge Slot 0 zuerst an und friert das als Golden ein; der Audiopfad muss es treffen. Rotbeweis: ein **Rückwärtslauf-Mutant (7 → 0)** fällt an genau diesem Golden. Die Zusage „die Reihenfolge hängt nie von der Belegungsreihenfolge ab" bleibt als **eigener** Fall, ist aber nicht mehr der Richtungsbeweis. §5.3.1 führt die Slotrichtung ab hier als Teil der eingefrorenen Kettenreihenfolge. |

**Warum die vorigen Proben nicht reichten — beide Male derselbe Fehlertyp.**
B-05 war eine **Reihenfolge**, die erst auffällt, wenn sich der Zustand
zwischen Original und Wiederholung bewegt: ein Riegel urteilt über den
heutigen Zustand, eine Wiederholung fragt nach dem Urteil von damals, und in
der falschen Reihenfolge bekommt derselbe Aufruf zwei verschiedene Antworten.
B-09 war eine **Probe, die ihre Zusage nicht trifft**: „beide Anordnungen
liefern verschiedene Ergebnisse" besteht eine rückwärts laufende Bank genauso,
also trennt sie 0 → 7 nicht von 7 → 0. Das ist dieselbe Klasse wie B-08, B-09
und B-10 der ersten Runde und dieselbe Klasse, die Prüflistenzeile E benennt.

**Zählung nach der Nacharbeit** (§3.15, aus den Zeilen gemessen): **124**
Matrixzeilen, lückenlos M-01 bis M-124, jede mit sieben Spalten; 76 BELEGT,
79 ENTSCHIEDEN, 124 BAULÜCKE, 0 OFFEN, 15 mit der Marke MN1 und **5** mit der
Marke MN2 (vier Zeilen tragen beide).

**Was diese Runde nicht getan hat.** Sie hat keinen Befund bestritten, keine
Härtung am Prüfwerkzeug vorgenommen (es gab keinen Befund gegen eines) und
keine der zehn in der Matrixprüfung 2 geschlossenen Zeilen erneut angefasst.
Über den Wortlaut der zwei Defekte hinaus sind **zwei** Stellen mitgezogen,
weil sie sonst widersprüchlich geblieben wären: **M-76** sagte „eine bereits
**angewandte** Transaktions-ID" und hätte damit die abgewiesenen ausgeschlossen,
die B-05 ausdrücklich einschließt; und **§5.3.1** trug die Kettenreihenfolge
ohne die Slotrichtung, die M-15 seit MN2 beweist. Beide sind in der Tabelle
oben verortet.

**Selbstaudit dieser Runde.** Gegen den Diff gelesen: beide Regeln sind
einzeln abgearbeitet und je Befund verortet. Die zwei Defekte sind an meinem
eigenen Text nachvollzogen, bevor ich ihn geändert habe — der Zonenriegel stand
tatsächlich vor der ID-Prüfung, und der alte M-15-Rotbeweis konnte die Richtung
tatsächlich nicht trennen. Beim Einarbeiten sind **drei Waisenzeilen** aus der
Nacharbeit 1 aufgefallen und entfernt worden: die mehrzeiligen Feinheiten
§5.11.2 und §5.11.4 waren dort zeilenweise ersetzt worden, sodass je eine
Fortsetzung doppelt stand. Ein Duplikatscan über das ganze Dokument ist danach
sauber (die zwei verbleibenden Mehrfachzeilen sind ein zweimal zitierter
Abnahmesatz und die vierzehn gleichlautenden Tabellenköpfe). Die Zeilennummern
in §2 und §5 bleiben gültig, weil kein Quellcode angefasst wurde. Keine Zeile
behauptet ein gemessenes Produktverhalten; der Kopf trägt weiter „Urteil
offen". Der `dokuriegel` läuft sauber, alle 124 Zeilen haben sieben Spalten,
und M-124 hat keine ID übernommen, die es schon gab.

**Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für diese Runde.**

| Zeile | Für diese Runde |
|---|---|
| **A** Rückstau und Prioritätsklassen | Nicht berührt — kein IPC-Code, keine Klasse geändert. |
| **B** Lebenszyklus | Nicht berührt. Das Wiederholungsfenster ist ausdrücklich **transient** und hat deshalb kein Speichern/Laden-Gegenstück (M-124). |
| **C** Verträge und Längen | Berührt über M-123 und M-124: das Fenster ist auf **32** begrenzt, und die Grenze kommt aus `undo_tiefe` (`eq-ipc-v3.schema.json:2321`), nicht aus einer Schätzung. |
| **D** Bau- und Prüfriegel | Nicht berührt. |
| **E** Behauptung ≤ Messung | **Die tragende Zeile.** B-09 war genau dieser Fehler: eine Probe, die auch bei verletzter Zusage grün bleibt. Der neue Rotbeweis ist ein benannter Mutant (Bank läuft 7 → 0), der an einem unabhängig gerechneten Golden fällt — kein Nebeneffekt. §3.15 ist neu **gezählt**. |
| **F** Änderungssatz | B-05 hält Original und Wiederholung im selben Satz zusammen: wer ein terminales Urteil ablegt, legt auch das abgewiesene ab (M-124), sonst zerfällt das Paar. |

### 7.5 Matrixprüfung 3 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08883-33e0-7211-9c6d-f8bc440e4b8e`; Lauf 01:31–01:38 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff c37ab19b...e77555ed -- docs/beweise/SONDE-015.md`; HEAD während des Laufs `522c5f9c` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-matrixpruefung-3-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-matrixpruefung-3-e77555e.txt` |
| Urteil | **NEEDS_WORK** — B-05 (Runde 2) und B-09 (Runde 2) **geschlossen**; §3.15 stimmt mit 124 IDs; der Fix erzeugt zwei Folgewidersprüche: D-1 (Abweisungen verdrängen committete Einträge aus dem 32er-Wiederholungsregister, obwohl Undo den Zustand noch erreicht) und D-2 (gemerktes `busy_retry` macht den in M-44 zugesagten Wiederholungsweg unausführbar) |
| Quellencheck des Dirigenten | direkt an M-123 (`:1148`), M-124 (`:1149`), M-44 (`:1094`), §5.11.4: beide **DEFEKT** — innere Widersprüche, die die MN2-Regel „gespeichertes Ergebnis auch für abgewiesene Transaktionen" erzeugt hat; der Dirigent nimmt diese Regel zurück |
| Rundenbilanz | `c37ab19b..e77555ed: Doku 1 Datei(en) +89/-19 → OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)` — Matrixrunde, strukturell null |
| Nacharbeit | Matrix-Nacharbeit 3 (dritte und letzte Runde vor dem Konvergenzentscheid): `docs/beweise/roh/SONDE-015-matrix-nacharbeit-3-auftrag.txt` — eine gemeinsame Regel: gemerkt wird nur, was eine Revision erzeugt hat, Fenster in Revisionen (32 = Undo-Tiefe), nicht-committete Ausgänge (Validierung, Konflikt, User-Schutz, `busy_retry`) werden nicht gemerkt und sind mit derselben ID neu ausführbar |

### 7.6 Matrix-Nacharbeit 3 — was eingearbeitet wurde (10.09.2026)

Auftrag wörtlich: `docs/beweise/roh/SONDE-015-matrix-nacharbeit-3-auftrag.txt`
(zwei Defekte, **eine** gemeinsame Regel, mit der der Dirigent seine eigene
MN2-Regel zu gespeicherten Abweisungen ausdrücklich zurücknimmt). Startstand
`bc4fa3df`, Worktree sauber. **Kein Produkt-, Test-, Schema-, Fixture- oder
Werkzeugcode**; einzige geänderte Datei ist dieses Manifest. Geänderte
Matrixzeilen tragen die Marke **MN3**; **keine neue Zeile** — die Regel
korrigiert bestehende Zusagen, sie stellt keine neue auf.

| Befund | Eingearbeitet in | Was sich geändert hat |
|---|---|---|
| **D-1** | §5.11.4, **M-123**, **M-74**, **M-75**, **M-76** | **Das Fenster zählt Revisionen, nicht Einträge.** Das Register hält je committeter Transaktion ihre Transaktions-ID, die erzeugte `state_revision` und den `state_hash`; ein Eintrag bleibt genau so lange, wie seine Revision höchstens **32** Revisionen hinter der aktuellen liegt — die Tiefe des Undo-Rings und exakt die Spanne, über die der Zustand per Undo noch erreichbar ist. Weil jede committete Transaktion genau eine Revision erzeugt, hält das Register nie mehr als 32 Einträge; ein zweiter, in Einträgen gezählter Deckel entfällt. Neuer Rotbeweis in M-123: **T, dann U, dann 31 Abweisungen wegen veralteter `base_revision`, dann T wiederholt → Revision und Hash von T**, keine neue Revision. |
| **D-2** | §5.11.4, **M-44**, **M-124**, **M-74**, **M-75**, **M-76** | **Nicht-committete Ausgänge werden nicht gemerkt.** Validierungsfehler, Konflikt, User-Schutz und `busy_retry` haben keine Revision erzeugt und nichts angewandt; eine Wiederholung derselben ID danach ist ein **neuer Versuch**, läuft die volle Kette und bekommt das Urteil des aktuellen Zustands. Damit ist `busy_retry` von selbst **nichtterminal**, und der in M-44 zugesagte Weg „Der Aufrufer wiederholt" ist ausführbar — **ohne** neue Transaktions-ID. Neuer Rotbeweis in M-44 und M-124: **Pool belegt → `busy_retry` → Audio-ACK gibt eine Bank frei → dieselbe ID erneut → committet mit genau einer neuen Revision.** |

**Die MN2-Regel ist zurückgenommen, und das steht auch so da.** §5.17 trägt sie
als **Abweichung 6** mit Grund; M-124 sagt in der Zeile selbst, dass ihre
MN2-Fassung das Gegenteil versprach. **Als lebende Zusage kommt der Satz nirgends mehr vor** — ein Textscan über die ganze Datei findet ihn noch an genau drei Stellen, und alle drei sind ausdrücklich Verlauf: als **zitierte** MN2-Fassung in M-74 und M-124 (jeweils hinter der Marke „MN3" und mit der Rücknahme im selben Satz) und im append-only Abschnitt **§7.4**, der die Nacharbeit 2 protokolliert und nicht umgeschrieben wird. Der MN2-Rotbeweis von M-123 (1000 Hz → T auf 1050 Hz → 500 Hz → T wiederholt) **bleibt** bestehen: er betrifft eine committete Transaktion und ist von der Rücknahme nicht berührt.

**Warum eine Regel für zwei Defekte reicht.** Beide hatten denselben Ursprung:
gemerkt wurde etwas, das **gar keinen Zustand erzeugt hat**. Ein Register, das
Abweisungen aufnimmt, wird von ihnen geflutet (D-1) und hält gleichzeitig
Ergebnisse fest, die nur für einen Augenblick galten (D-2). Sobald es
ausschließlich Committetes hält, verschwinden beide: es kann nur so voll werden,
wie Revisionen entstanden sind, und ein `busy_retry` liegt nie darin. Das ist
auch der Grund, warum die Nacharbeit keine neue Matrixzeile braucht — es ist
keine zusätzliche Zusage, sondern eine engere.

**Zählung nach der Nacharbeit** (§3.15, aus den Zeilen gemessen): **124**
Matrixzeilen, lückenlos M-01 bis M-124, jede mit sieben Spalten; 76 BELEGT,
80 ENTSCHIEDEN, 124 BAULÜCKE, 0 OFFEN; 15 mit der Marke MN1, **6** mit MN2 und
**6** mit MN3.

**Was diese Runde nicht getan hat.** Sie hat keinen Befund bestritten, keine
Härtung am Prüfwerkzeug vorgenommen und keine der in den Matrixprüfungen 2 und
3 geschlossenen Zeilen erneut geöffnet. Über den Wortlaut der zwei Defekte
hinaus sind **zwei** Stellen mitgezogen, beide zwingend: **M-76** sagte „eine
bereits beurteilte Transaktions-ID — angewandt oder abgewiesen" und hätte der
neuen Regel widersprochen; **M-75** sagte nichts darüber, dass ein Fehler auch
keinen Registereintrag hinterlässt, was seit MN3 Teil der Zusage „kein halber
Zustand überlebt einen Fehler" ist.

**Selbstaudit dieser Runde.** Beide Defekte sind an meinem eigenen Text
nachvollzogen, bevor ich ihn geändert habe: der Ring zählte tatsächlich
Einträge statt Revisionen, und M-124 versprach tatsächlich das gespeicherte
`busy_retry` zurück. **Zum zweiten Mal** sind beim Ersetzen Waisenzeilen
aufgefallen — die mehrzeiligen Feinheiten §5.11.2 und §5.11.4 waren in der
Nacharbeit 2 zeilenweise ersetzt worden, sodass fünf Fortsetzungen doppelt
standen; sie sind entfernt. Der Duplikatscan über die ganze Datei findet danach
**keine** Waise mehr: die sieben verbliebenen Mehrfachzeilen sind die vierzehn
gleichlautenden Matrix-Tabellenköpfe, ein zweimal zitierter Abnahmesatz und
fünf gleichlautende Standardsätze, die §7.1, §7.4 und §7.6 als Abschnitte
derselben Bauart teilen (Änderungssatz, Zählung, „Was diese Runde nicht getan
hat", Prüflisten-Überschrift). **Die Lehre daraus steht hier, weil sie sich wiederholt
hat:** wer einen mehrzeiligen Absatz durch einen neuen ersetzt, muss die
Fortsetzungszeilen mitnehmen; ein Scan auf eingerückte Zeilen und ein
Duplikatscan finden es sofort. In §5.17 stand die neue Abweichung 6 nach dem
Einfügen vor der 5 — die Reihenfolge ist auf 1 bis 6 gebracht. Die
Zeilennummern in §2 und §5 bleiben gültig, weil kein Quellcode angefasst wurde.
Keine Zeile behauptet ein gemessenes Produktverhalten; der Kopf trägt weiter
„Urteil offen". Der `dokuriegel` läuft sauber, alle 124 Zeilen haben sieben
Spalten, und keine ID ist umgewidmet.

**Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für diese Runde.**

| Zeile | Für diese Runde |
|---|---|
| **A** Rückstau und Prioritätsklassen | Berührt in der Sache: `busy_retry` ist die Politik bei vollem Bankpool, und sie sagt jetzt ausdrücklich, wie der Abflussweg aussieht („Der Aufrufer wiederholt mit derselben ID") — genau das, was Zeile A für einen Puffer verlangt, der „später wiederholt". |
| **B** Lebenszyklus | Das Register ist ausdrücklich **transient** und hat deshalb kein Speichern/Laden-Gegenstück; das ist in M-124 und §5.11.4 gesagt, nicht stillschweigend angenommen. |
| **C** Verträge und Längen | Die Fenstergrenze **32** kommt weiter aus `undo_tiefe` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2321`); neu ist, dass sie in **Revisionen** und nicht in Einträgen gilt — die Zahl ist dieselbe, ihre Einheit ist jetzt die richtige. |
| **D** Bau- und Prüfriegel | Nicht berührt. |
| **E** Behauptung ≤ Messung | D-1 und D-2 waren keine zu schwachen Proben, sondern zwei Zusagen, die einander widersprachen — der andere Fehlertyp. Die neue Regel macht beide gleichzeitig haltbar, und die zwei Rotbeweise (Mischfall, Wiederholung nach Bankfreigabe) fallen jeweils an der Zeile, die die Zusage trägt. §3.15 ist neu **gezählt**. |
| **F** Änderungssatz | Die Regel hält das Paar zusammen: wer einen Registereintrag anlegt, legt ihn **am Commit-Punkt** an, und wer keinen Commit erreicht, legt keinen an. Beide Hälften stehen in derselben Stufenliste (§5.11.2, M-74). |

### 7.7 Matrixprüfung 4 — NEEDS_WORK, und Konvergenzentscheid (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08893-9fd2-74d0-9ee5-732018678864`; Lauf 01:49–01:55 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff bc4fa3df...455b06c0 -- docs/beweise/SONDE-015.md`; HEAD während des Laufs `bdc89d04` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-matrixpruefung-4-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-matrixpruefung-4-455b06c.txt` |
| Urteil | **NEEDS_WORK** — D-2 **geschlossen**; D-1 offen als K-1 (Fensterrand: „höchstens 32 Revisionen hinter" ergibt 33 Einträge gegen „nie mehr als 32"); Folgebefund K-2 (M-124-Rotbeweis erwartet nach gelöster Zone Erfolg, obwohl die veraltete `base_revision` nach M-76 Konflikt verlangt) |
| Rundenbilanz | `bc4fa3df..455b06c0: Doku 1 Datei(en) +110/-27 → OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)` — Matrixrunde, strukturell null |
| **Konvergenzentscheid** | Dritte Wiederprüfung (MP4 nach MN3) ohne PASS → keine vierte Punktkorrektur-Runde (Dirigent §3.4). Einordnung an der Quelle (M-123 `:1149`, M-124 `:1150`, M-71 `:1138`, M-76, §5.11.4): **K-1 DEFEKT** (die Matrixzeile widerspricht sich selbst: Fensterrand inklusiv, Kapazität 32; die Unschärfe stammt aus der Dirigenten-Regel D-1 „höchstens 32 Revisionen hinter"), **K-2 DEFEKT** (Rotbeweis wertet richtiges Verhalten nach M-76 als Fehler). Kein Befund ist Lücke oder Härtung. **Ursache:** die Transaktionssemantik wurde in MN1–MN3 als Prosa nachgezogen, nie als geschlossene Spezifikation mit exakter Arithmetik und vollständiger Fallmenge. **Wegwechsel:** genau eine weitere Runde mit der Ursache als Auftrag — §5.11.4 wird als geschlossene Protokollspezifikation (Definitionen mit Arithmetik: Eintrag im Fenster ⇔ r − e ≤ 31, exakt 32 Einträge = Undo-Tiefe; vollständige Falltabelle Zustand × Eingabe → Ausgang, memoisiert ja/nein; je Zeile Test und Rotbeweis) neu geschrieben, M-74/M-75/M-76/M-123/M-124/M-44 leiten sich daraus ab. Auftrag: `docs/beweise/roh/SONDE-015-matrix-konvergenz-auftrag.txt`. Danach frischer Prüfer (Matrixprüfung 5). Bleibt danach ein Defekt, wird er als eigener Registerpunkt mit Matrix ausgegliedert und die Etappen 2 und 3 (Verträge, Kern) beginnen; der Rest gehört ohnehin zur Transaktionsseite der Etappe 4 und wird dort geschlossen (User-Wort 30.08.2026: „voranschreiten ist das wichtigste") |

### 7.8 Matrix-Konvergenzrunde — was eingearbeitet wurde (10.09.2026)

Auftrag wörtlich: `docs/beweise/roh/SONDE-015-matrix-konvergenz-auftrag.txt` —
die **eine** weitere Runde nach dem Konvergenzentscheid (§7.7). Ihr Gegenstand
ist nicht K-1 und K-2 einzeln, sondern ihre **Ursache**. Startstand
`73bc10b6`, Worktree sauber. **Kein Produkt-, Test-, Schema-, Fixture- oder
Werkzeugcode**; einzige geänderte Datei ist dieses Manifest.

**Die Ursache, wie der Dirigent sie benannt hat.** Die Transaktionssemantik —
Wiederholungsregister, Fenster, Konflikt, nicht-committete Ausgänge — wurde über
drei Runden als Prosa nachgezogen und dabei nie als geschlossene Spezifikation
mit exakter Arithmetik und vollständiger Fallmenge festgelegt. Jede Runde
schloss einen Satz und öffnete den nächsten Randfall: **MN1** setzte das
Register auf, **MN2** stellte die Reihenfolge um und nahm Abweisungen mit auf,
**MN3** nahm die Abweisungen wieder heraus und stellte auf Revisionen um — und
hinterließ dabei einen Fensterrand, der zur Kapazität nicht passte (K-1) und
einen Rotbeweis, der richtiges Verhalten als Fehler wertete (K-2). Das Muster
ist deutlich: **eine Reihenfolge, die an zwei Stellen steht, läuft auseinander;
eine Zahl, die an zwei Stellen steht, widerspricht sich.**

**Der Wegwechsel.** §5.11.4 ist ab dieser Runde eine **geschlossene
Protokollspezifikation** in vier Teilen, und sie ist die **einzige** Stelle, an
der die Transaktionsarithmetik definiert wird:

| Teil | Inhalt |
|---|---|
| **1 — Definitionen und Arithmetik** | `r`, `e`, `tid`, `base_revision`, `R`. Fenster: `0 ≤ r − e ≤ 31`, also die letzten **32** committeten Transaktionen, Rand an beiden Enden inklusiv. Kapazität **genau 32** — dieselbe Zahl, weil jede committete Transaktion genau eine Revision erzeugt. Herkunft der 32: die Undo-Tiefe (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2321`), die genau dieselbe Revisionsmenge erreicht. Belegungstabelle für `r` = 1, 32, 33, 34, 100. Verdrängung: nur am Commit-Punkt, genau ein Eintrag, nur wenn `r > 32`. |
| **2 — Stufenfolge** | **S0** Nachschlag, **S1** `base_revision`, **S2** Validierung, **S3** Zonenriegel, **S4** Kandidaten-DTO, **S5** Programm (`busy_retry`), **S6** Hash, **S7** Undo-Vorbereitung, **COMMIT-PUNKT**, **S8** Übernahme und Registereintrag. Je Stufe steht, ob sie fehlbar ist und ob sie allokiert. |
| **3 — Invarianten** | **I1** `\|R\| = min(r, 32)` · **I2** Fensterzugehörigkeit ⇔ `0 ≤ r − e ≤ 31` · **I3** Kapazität = Fensterspanne = Undo-Tiefe = 32 · **I4** Registereintrag nur in S8 und nur bei Commit · **I5** `R` ist transient · **I6** genau ein Ausgang je Eingabe. |
| **4 — Falltabelle** | **T1 bis T12**, Zustand × Eingabe → Ausgang, mit `r` danach, memoisiert ja/nein, dem B7-Fall und dem Rotbeweis je Zeile. |

| Befund | Eingearbeitet in | Wie die Tabelle ihn schließt |
|---|---|---|
| **K-1** (inklusiver Fensterrand gegen Kapazität) | §5.11.4 Teil 1 und Teil 3, **M-123** | Der Rand ist `r − e ≤ 31` statt „höchstens 32 hinter". Damit umfasst das Fenster die Revisionen `r−31 … r` — **32** Stück — und `\|R\| = min(r, 32)` gilt widerspruchsfrei (**I1**, **I2**, **I3**). Bei `r = 33` ist `e = 1` draußen (`33 − 1 = 32`) und `e = 2` drinnen (`33 − 2 = 31`). Beide Seiten des Randes sind eigene Tabellenzeilen (**T10**, **T11**) und ein **Paar**: einzeln belegt keine von beiden, dass der Rand an der richtigen Stelle liegt. |
| **K-2** (Basisrevision beim Retry nach Zonenlösung) | §5.11.4 Teil 4, **M-124** | Die Tabelle trennt, was vorher eine Zeile war. Nach **`busy_retry`** hat sich `r` nicht bewegt — die Bankfreigabe ist ein Audio-ACK und keine Transaktion —, also gilt die alte `base_revision` weiter und die Wiederholung committet (**T7**). Nach **User-Schutz** ist das Lösen der Zone **selbst eine Transaktion** und hat `r + 1` erzeugt (M-71): mit **alter** `base_revision` ergibt die Wiederholung **Konflikt**, und das ist **richtig** (**T8**); erst mit **aktualisierter** `base_revision` committet sie (**T9**). Der MN3-Rotbeweis ist auf T9 eingegrenzt, T8 ist als eigener Fall dazugekommen. |

**Was sich an den Matrixzeilen geändert hat.** Sechs Zeilen sind auf
Ableitungen zurückgeführt und tragen die Marke **MK**: **M-44** (zitiert T5 und
T7), **M-74** (Stufenfolge und I4 statt eigener Liste), **M-75** (die sieben
Einspritzpunkte heißen jetzt S1 bis S7, und S0 ist begründet ausgenommen),
**M-76** (verweist auf die ganze Tabelle), **M-123** (Fenster, Rand-Paar,
Mischfall) und **M-124** (T7, T8, T9). Dazu **eine neue Zeile, M-125**: sie
fährt die Falltabelle als ganze — jede Zeile, alle vier Spalten, dazu I1, I2
und I4 als Wachen nach **jeder** Eingabe. Ohne sie wäre die Tabelle eine
Beschreibung; mit ihr ist sie eine Zusage.

**Auch §5.11.2 ist entlastet.** Der Punkt trug die Stufenfolge ein zweites Mal —
genau die Doppelung, aus der die Widersprüche entstanden. Er nennt jetzt nur
noch, **was an der Reihenfolge hängt und warum** (Commit-Punkt, Nachschlag
vorn), und verweist für die Folge selbst auf §5.11.4 Teil 2.

**Eine Reihenfolgeänderung, die die Tabelle erzwingt und die benannt gehört.**
`base_revision` (**S1**) steht jetzt **vor** der Validierung (**S2**); bis zur
Konvergenzrunde war es umgekehrt. Grund: die Falltabelle des Auftrags führt
„ID unbekannt und `base_revision ≠ r` → Konflikt" **ohne** Vorbehalt zur
Gültigkeit der Nutzlast. Liefe die Validierung zuerst, hätte eine Transaktion
mit veralteter `base_revision` **und** ungültigem Wert zwei mögliche Ausgänge,
und **I6** („genau ein Ausgang je Eingabe") wäre nicht haltbar. Sachlich ist es
ohnehin richtig: eine Nutzlast, die gegen einen nicht mehr existierenden
Zustand gebildet wurde, muss man nicht erst auf Bereichsgrenzen prüfen. An
B-01 ändert es nichts — S1 allokiert nicht, und alle allozierenden Stufen
liegen unverändert vor dem Commit-Punkt.

**Die Arithmetik ist gerechnet, nicht behauptet.** Vor dem Schreiben ist die
Fensterformel in dieser Sitzung durchgerechnet worden: `|R| = min(r, 32)` für
`r` = 1, 32, 33, 34, 100; der Grenzfall `r = 33` mit `e = 1` außerhalb und
`e = 2` innerhalb; und der Mischfall aus D-1 (T bei `e`, U bei `e+1`, 31
Abweisungen ändern `r` nicht, also `r − e = 1`). Die Belegungstabelle in
§5.11.4 Teil 1 zeigt genau diese Werte.

**Zählung nach der Runde** (§3.15, aus den Zeilen gemessen): **125**
Matrixzeilen, lückenlos M-01 bis M-125, jede mit sieben Spalten; 76 BELEGT,
81 ENTSCHIEDEN, 125 BAULÜCKE, 0 OFFEN; 15 MN1, 6 MN2, 6 MN3, **7 MK**.

**Was diese Runde nicht getan hat.** Sie hat keinen Befund bestritten, keine
Härtung am Prüfwerkzeug vorgenommen und keine Zeile außerhalb der
Transaktionssemantik angefasst. §5.17 trägt den Wegwechsel als **Abweichung 7**
mit Grund — nicht weil er vom Wortlaut einer R-Regel abweicht, sondern weil
eine Formänderung dieser Größe genauso begründungspflichtig ist wie eine
inhaltliche.

**Selbstaudit dieser Runde.** Beide Befunde sind an meinem eigenen Text
nachvollzogen, bevor ich ihn geändert habe: „höchstens 32 Revisionen hinter"
ergab tatsächlich 33 Einträge, und der MN3-Rotbeweis hätte T8 tatsächlich als
Fehler gewertet. Beim Ersetzen habe ich diesmal **beide** mehrzeiligen
Feinheiten als Block genommen (Anfangszeile plus alle eingerückten
Fortsetzungen) — die Waisenzeilen, die in MN2 und MN3 je einmal aufgetreten
sind, kommen dadurch nicht wieder. Ein Duplikatscan über die ganze Datei findet
**acht** Mehrfachzeilen, alle harmlos und alle erklärbar: die vierzehn
gleichlautenden Matrix-Tabellenköpfe, ein zweimal zitierter Abnahmesatz und **sechs** Standardsätze, die §7.1, §7.4, §7.6 und §7.8 als Abschnitte derselben
Bauart teilen (Änderungssatz, Zählung, „Was diese Runde nicht getan hat",
Prüflisten-Überschrift, Selbstaudit-Schlusssatz). Es sind acht statt der sieben
aus §7.6, weil dieser Abschnitt dazugekommen ist und dieselben Formeln benutzt —
die Zahl ist gemessen, nicht aus §7.6 fortgeschrieben. In §5.17 stand die neue Abweichung 7 nach dem Einfügen
vor der 6; die Reihenfolge ist auf 1 bis 7 gebracht. Die Zeilennummern in §2
und §5 bleiben gültig, weil kein Quellcode angefasst wurde. Keine Zeile
behauptet ein gemessenes Produktverhalten; der Kopf trägt weiter „Urteil
offen". Der `dokuriegel` läuft sauber, alle Tabellen in §5.11.4 tragen je Zeile
gleich viele Spalten, und keine ID ist umgewidmet.

**Was auch nach dieser Runde offen bleibt — ehrlich benannt.** Die Tabelle ist
für die zwölf genannten Fälle vollständig und paarweise unvereinbar (**I6**),
aber Vollständigkeit über **alle** denkbaren Eingaben ist damit nicht bewiesen,
sondern durch die feste Stufenfolge S0 bis S5 **konstruiert**: jede Eingabe
trifft die erste zutreffende Stufe. Fällt in Etappe 4 ein Fall auf, den keine
Zeile trägt, ist das ein Übersetzungsfehler zwischen Tabelle und Kern — und
M-125 fällt genau daran, weil ein Fall ohne Tabellenzeile dort keinen Test hat.

**Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für diese Runde.**

| Zeile | Für diese Runde |
|---|---|
| **A** Rückstau und Prioritätsklassen | `busy_retry` ist die Politik bei vollem Bankpool; T5 und T7 sagen jetzt ausdrücklich, wie ihr Abflussweg aussieht und dass er **ohne** neue ID funktioniert — genau das, was Zeile A für einen Puffer verlangt, der „später wiederholt". |
| **B** Lebenszyklus | `R` ist ausdrücklich transient (**I5**) und hat deshalb kein Speichern/Laden-Gegenstück; das steht als Invariante, nicht als Nebensatz. |
| **C** Verträge und Längen | Die **32** kommt weiter aus `undo_tiefe` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2321`); neu ist, dass Fensterspanne, Kapazität und Undo-Tiefe **dieselbe** Zahl sind (**I3**) und nicht mehr getrennt behauptet werden. |
| **D** Bau- und Prüfriegel | Nicht berührt. |
| **E** Behauptung ≤ Messung | Die tragende Zeile. Jede Tabellenzeile nennt ihren B7-Fall **und** den Rotbeweis, und M-125 macht die Tabelle selbst prüfbar. Die Arithmetik ist vor dem Schreiben gerechnet worden; §3.15 ist neu **gezählt**. |
| **F** Änderungssatz | Die Paare stehen jetzt beieinander statt verstreut: T5↔T7 (`busy_retry` und sein Retry), T8↔T9 (Retry nach Zonenlösung, beide Basisrevisionen), T10↔T11 (beide Seiten des Fensterrandes). Keines lässt sich einzeln ändern, ohne dass sein Partner in derselben Tabelle sichtbar mitwandert. |

### 7.9 Matrixprüfung 5 — NEEDS_WORK; Restbefunde als NAK-245 ausgegliedert, Etappe 2 beginnt (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a088a5-018a-7f12-bc04-2832fcc71e1a`; Lauf 02:08–02:17 |
| Prüfbereich | Wiederprüfung der Konvergenzrunde über `git diff 73bc10b6...f636a09f -- docs/beweise/SONDE-015.md`; HEAD während des Laufs `1ab5ec8a` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-matrixpruefung-5-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-matrixpruefung-5-f636a09.txt` |
| Urteil | **NEEDS_WORK** — **K-1 und K-2 geschlossen** (§5.11.4 „0 ≤ r − e ≤ 31", 32 Plätze, T10/T11 mit beidseitigen Rotbeweisen; T8 Konflikt, T9 Commit); §3.15 stimmt mit 125 IDs. Fünf neue Widersprüche in der neuen Tabelle: Fehler in S4/S6/S7 ohne Zeile; T4 nicht auf Bedienänderungen begrenzt (gegen M-61); T9 gegen T5 (`busy_retry`); I1 gegen Ladestart (M-89/M-93/I5); M-76 mischt Tabellenlauf und Ein-Revisions-Stresstest |
| Rundenbilanz | `73bc10b6..f636a09f: Doku 1 Datei(en) +282/-35 → OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)` — Matrixrunde, strukturell null; kumuliert Etappe 1: `e9dbf4b9..f636a09f` nur Doku |
| **Entscheid** | Die Ursachenrunde nach dem Konvergenzentscheid (§7.7) hat K-1/K-2 geschlossen; die fünf Restbefunde sind alle DEFEKT (innere Widersprüche der neuen Tabelle), betreffen ausschließlich §5.11.4/M-76/M-125 — den Transaktionskern der **Etappe 4** — und keine Zusage der Etappen 2 und 3. Nach Dirigent §3.4 wird die Ursache als eigener Registerpunkt ausgegliedert: **NAK-245** (`docs/offene-punkte.md`, Klasse [Planarbeit · S26–28]); die fünf Punkte werden vor dem Bau der Etappe 4 in derselben Tabelle geschlossen und von der Erstprüfung der Etappe 4 mitgeprüft. Die Matrix ist damit Referenz für **Etappe 2 (Verträge in drei Sprachen)**, die jetzt beginnt (User-Wort 30.08.2026: „voranschreiten ist das wichtigste"). Matrixrunden gesamt: fünf Prüfungen, vier Nacharbeiten, Konvergenzentscheid nach der dritten Wiederprüfung |

### 7.10 NAK-245-Runde — was eingearbeitet wurde (10.09.2026)

Auftrag wörtlich: `docs/beweise/roh/SONDE-015-nak245-auftrag.txt`; Anlass ist das
Urteil der Matrixprüfung 5 (`docs/beweise/roh/SONDE-015-matrixpruefung-5-f636a09.txt`,
§7.9). Startstand `4d769e43`, Worktree sauber. **Kein Produkt-, Test-, Schema-,
Fixture- oder Werkzeugcode**; einzige geänderte Datei ist dieses Manifest. Die
abgenommenen Etappen 2 und 3 und ihre Zusagen bleiben unberührt; ihr Code und
ihre Verträge sind nur als Quelle gelesen worden (Stufe S5, `state_revision`,
`undo_tiefe`).

**Der Grundsatz der Runde.** §5.11.4 bleibt die einzige Stelle der
Transaktionsarithmetik. Die fünf Widersprüche lagen **in** der Tabelle der
Konvergenzrunde, nicht zwischen ihr und einer anderen Stelle; geschlossen sind
sie deshalb dort, und die abgeleiteten Matrixzeilen zitieren danach nur. Neue
Tabellenzeilen laufen ab T13, keine bestehende ist umnummeriert oder umgewidmet;
geänderte Matrixzeilen tragen die Marke **MR**.

| Defekt | Regel des Dirigenten, kurz | Geänderte Stellen |
|---|---|---|
| **D-245-1** — Fehler in S4, S6 und S7 hatten keine Zeile (I6) | Jede fehlbare Stufe hat mindestens eine Zeile mit genau einem Ausgang; eigene Zeilen für S4, S6 und S7; eine zweite S5-Zeile nur, wenn S5 neben `busy_retry` scheitern kann; I6 nennt S0 bis S7; M-75 zitiert die Zeilen | §5.11.4 Teil 2 (**S5**, **S8**, neuer Absatz „Warum S5 keine Bank belegt“), Teil 3 (**I6**), Teil 4 (neue Zeilen **T13** für S4, **T14** für S6, **T15** für S7; die Einspritzfälle aus M-75 an **T2** bis **T5**); **M-75** |
| **D-245-2** — T4 verlangte User-Schutz auch für das Undo aus M-61 | T4 nur für Bedienänderungen nach §5.6.3 Feinheit 3; eine eigene Zeile für Ganzzustands-Wiederherstellungen, die eine Transaktion sind; Projekt-Laden ist keine | §5.11.4 Teil 1 (`r0`: Projekt-Laden), Teil 2 (**S3**), Teil 4 (**T4** eingegrenzt, neue Zeile **T16**, Paar T4 und T16); **M-61**, **M-74**, neue Matrixzeile **M-126** |
| **D-245-3** — T9 und T5 gaben bei vollem Pool zwei Ausgänge | T7 und T9 setzen S2 bis S7 ausdrücklich voraus; der volle Pool ist ein eigener B7-Fall und Rotbeweis an T9; die Reihenfolge aus Teil 2 entscheidet | §5.11.4 Teil 3 (**I6**), Teil 4 (**T7**, **T9**, Satz zur Abgrenzung unter der Tabelle); **M-44**, **M-124** |
| **D-245-4** — I1 verlangte nach dem Laden volle Belegung (gegen M-89, M-93, I5) | `r0` als Revision beim Sitzungs- oder Ladestart; I1 und I2 mit `r0`; Belegung mit Ladestart; T10 und T11 in dieser Sitzung; die Vorhaltegarantie gilt innerhalb der Sitzung; eine Zeile für die Wiederholung über die Sitzungsgrenze | §5.11.4 Teil 1 (`r`, neu `r0`, `e`, `R`, Fenster, Kapazität, „Woher die 32 kommt“, Belegung, Verdrängung), Teil 3 (**I1**, **I2**, **I5**), Teil 4 (**T1**, **T10**, **T11**, **T12**, neue Zeile **T17**, Paar T1 und T17, „Was die Tabelle nicht sagt“); **M-123**, **M-124**, **M-125** |
| **D-245-5** — M-76 mischte den Tabellenlauf mit dem Ein-Revisions-Stresstest | M-76 misst nur den Satz aus §44.5, mit Eingaben aus einer logischen Transaktion, und zitiert T1, T2 und T6; den Tabellenlauf trägt allein M-125; die Zeilenzahl steht nur in der Tabelle | **M-76**, **M-125**, §5.11.4 Teil 4 (Vorspann mit der Zeilenzahl) |

Dazu: §3.15 neu gezählt, §5.17 Abweichung 8, die B7-Liste in §4.4 (sie nennt
jetzt M-123 bis M-125 und M-126) und die Kopfzeile von §5.11.4.

**Entscheidungen innerhalb der Regeln, begründet.**

1. **S5 hat keinen zweiten Fehlerausgang.** Die Regel zu D-245-1 macht eine zweite S5-Zeile davon abhängig, ob S5 bei freier Bank scheitern kann. An der Quelle kann es das nicht: `DspKern::uebernehmeZustand` liefert `false` nur ohne freie Bank (`eq-copilot/plugin/dsp/DspKern.cpp:179-180`), und `baueProgramm` gibt nichts zurück und schreibt in ein `DspProgramm` fester Größe (`eq-copilot/plugin/dsp/DspProgramm.h:158`, `:183-184`). Der Einspritzfall `fehler_in_S5_laesst_committed_und_hash_unveraendert` ist deshalb der Fall „keine Bank frei“ und steht an T5. Teil 2 führt S5 jetzt als „fehlbar: nur `busy_retry`“ und „allokiert: nein“; die Konvergenzrunde hatte beides mit „ja“ eingetragen, bevor der Kern gebaut war.
2. **S5 belegt keine Bank, erst S8 legt das Programm hinein.** Eine in S5 reservierte Bank überlebte einen Fehler in S6 oder S7: der Pool führt einen Slot aus `vorbereitend` nur nach `bereit` (`eq-copilot/plugin/dsp/DspBankPool.cpp:143`). T14 und T15 hätten „ohne Commit“ zugesagt, während eine Bank belegt bleibt, und nach einigen solchen Fehlern wäre T7 unerreichbar. Die Reihenfolge der Stufen bleibt, wie Teil 2 sie festlegt; nur die Belegung liegt jetzt ausdrücklich in S8. Feststellen, dass eine Bank frei ist, ohne sie zu belegen, kann S5 schon mit dem gebauten Kern (`DspKern::pool()`, `eq-copilot/plugin/dsp/DspKern.h:124-125`; `DspBankPool::freieSlots()`, `eq-copilot/plugin/dsp/DspBankPool.h:238`), und weil nur der Control-Worker Bänke belegt (§44.2), scheitert das Belegen in S8 danach nicht mehr.
3. **E-18 steht in S5 und T5.** Ein Zustand mit `eq_enabled = false` braucht keine Bank (Entscheid E-18, `eq-copilot/plugin/dsp/DspKern.cpp:170-177`). T5 sagte „alle vier Bänke belegt ⇒ `busy_retry`“ ohne diese Ausnahme; weil die Regel zu D-245-3 die Wiederholung bei vollem Pool nach T5 schickt, muss T5 für genau die Eingaben stimmen, die dort ankommen.
4. **I6 sagt nicht mehr „paarweise unvereinbar“.** Wörtlich stimmte das schon in der Konvergenzfassung nicht — T2 und T10 lassen dieselbe Eingabe zu —, und die neuen Zeilen T16 und T17 liegen ebenso innerhalb von T6. Wahr und ausreichend ist: die erste entscheidende Stufe einer Eingabe ist eindeutig, und Zeilen mit derselben ersten Stufe tragen denselben Ausgang. I6 sagt jetzt genau das, nennt S0 bis S7 und sagt, dass S0 durch einen Treffer entscheidet und S1 bis S7, indem sie nicht bestehen.
5. **Redo und Preset-Laden bekommen eine eigene Matrixzeile (M-126).** Die Regel zu D-245-2 verlangt je einen Fall für Redo und Preset-Laden, falls M-61 sie nicht schon trägt. M-61 trägt sie nicht, und ihr Ereignis heißt „direkt nach dem Remove“: ein Redo- oder Preset-Fall dort fiele an einer Zeile, die seine Zusage nicht trägt. M-126 steht deshalb in §3.8 bei den Zonen.
6. **Der abgeschlossene Parametergestus ist eine committete Transaktion.** I1 mit `r0` hält nur, wenn `r` zwischen zwei Ladestarts ausschließlich in S8 steigt. §44.3 zählt die Revision „ausschließlich“ für diskrete Änderungen, und der Gestus ist eine davon (§5.11 Feinheit 1, M-82); Teil 1 sagt das jetzt ausdrücklich, damit I1 keine Revision ohne Registereintrag kennt.
7. **M-76 beginnt nach einem Ladestart mit `r0 ≥ 1`.** Im frischen Zustand gibt es keine ältere `base_revision`, und „veraltete Befehle“ wären nicht bildbar. Die Zusage bleibt `r − r0 ≤ 1`, wie die Regel sie nennt.
8. **Die Zeile der Undo-Tiefe ist nachgemessen.** `undo_tiefe` steht am Basis-SHA in `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2372`, nicht mehr bei `:2321`, weil Etappe 2 das Schema davor erweitert hat. §5.11.4 und M-123 tragen die neue Zeile; M-78 und der Verlauf ab §7.8 gehören nicht zu dieser Runde und bleiben stehen.

**Die Arithmetik ist gerechnet, nicht behauptet.** Vor dem Schreiben ist ein
Modell der Stufen S0, S1 und S8 mit Ladestart in dieser Sitzung durchgerechnet
worden, mit `|R| = min(r − r0, 32)` als Wache nach jedem Commit: die Belegung für
`r0 = 0` und `r` = 1, 32, 33, 34, 100 sowie für `r0 = 33` und `r` = 33, 34, 65, 66
ergibt genau die Werte der Tabelle in Teil 1; T10 und T11 liefern bei `r0 = 0` und
bei `r0 = 33` Konflikt für die erste und das gespeicherte Ergebnis für die zweite
Transaktion; T12 liefert bei `r − e = 1` das gespeicherte Ergebnis; T17 committet
bei 34, danach ist `|R| = 1`; und 10.000 Eingaben einer einzigen Transaktion nach
einem Ladestart bei 33 ergeben einen Commit, sonst nur Konflikt und gespeichertes
Ergebnis, und `r − r0 = 1`.

**Zählung nach der Runde** (§3.15, aus den Zeilen gemessen): **126**
Matrixzeilen, lückenlos M-01 bis M-126, jede mit sieben Spalten; 76 BELEGT,
82 ENTSCHIEDEN (davon 50 allein), 126 BAULÜCKE, 0 OFFEN; 15 MN1, 6 MN2, 6 MN3,
7 MK, **9 MR** (M-44, M-61, M-74, M-75, M-76, M-123, M-124, M-125, M-126). Alle
Tabellen in §5.11.4 tragen je Zeile gleich viele Spalten.

**Was diese Runde nicht getan hat.** Sie hat keinen Befund bestritten, keine Zahl
erfunden, keine Zeile umnummeriert oder umgewidmet und nichts außerhalb der
Transaktionssemantik angefasst. §8, §9, der lebende Kopf und
`docs/offene-punkte.md` sind unberührt; das Register schließt der Dirigent nach
der Prüfung. §5.6.3 bleibt wörtlich: seine Ausnahme nennt Projekt-Laden neben
Undo und Redo, und das bleibt richtig — Projekt-Laden durchläuft den Riegel
nicht, weil es gar keine Transaktion ist (Teil 1, `r0`).

**Selbstaudit dieser Runde.** Alle fünf Befunde sind vor dem Ändern an der
Konvergenzfassung nachvollzogen: ein Fehler in S7 traf keine Zeile; T4 hätte das
Undo aus M-61 abgewiesen; T9 und T5 gaben bei vollem Pool zwei Ausgänge; I1
verlangte nach dem Laden eines Standes mit Revision 33 sofort volle Belegung;
M-76 verlangte drei Commits und höchstens eine Revision zugleich. Die
Matrixzeilen sind zellweise geändert, und der Wortdiff über §3 zeigt nur die
beabsichtigten Stellen. Die Zeilenzahl der Falltabelle steht nur in deren
Vorspann; M-125 nennt sie nicht. `git grep` nach den alten Formeln
(`min(r, 32)`, „T1 bis T12“, „S0 bis S5“) trifft außerhalb dieses Abschnitts nur
noch Verlauf und Zitat: §7.8 sowie die Nummern 7 und 8 in §5.17. Ein Scan auf
doppelte Zeilen findet nur die Standardsätze und Tabellenköpfe, die die
Rundenabschnitte teilen. Keine Zeile behauptet ein gemessenes Produktverhalten —
B7 entsteht erst in Etappe 4. Der `dokuriegel` läuft sauber.

**Was auch nach dieser Runde offen bleibt — ehrlich benannt.** Die
Vollständigkeit der Tabelle ist weiterhin konstruiert und nicht bewiesen: jede
Eingabe trifft die erste entscheidende Stufe, und jede fehlbare Stufe hat eine
Zeile. Fällt in Etappe 4 ein Fall auf, den keine Zeile trägt, fällt M-125 daran.
Dass S5 die Bank nur feststellt und S8 sie belegt, ist Bauarbeit der Etappe 4
gegen T5, T7, T14 und T15 — der gebaute Kern bietet beides schon an (Punkt 2).

**Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für diese Runde.**

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | `busy_retry` bleibt die Politik bei vollem Pool; sein Abflussweg hängt jetzt ausdrücklich an den übrigen Stufen (T7, T9 mit Abgrenzung). An der Quelle gemessen: eine Reservierung vor S6 und S7 hätte den Abflussweg ohne ACK verstopft, deshalb belegt erst S8 die Bank (§5.11.4 Teil 2; `eq-copilot/plugin/dsp/DspBankPool.cpp:143`). |
| **B** Lebenszyklus | speichern↔laden steht als Paar in der Spezifikation: gespeichert wird `state_revision` (M-89), der Ladestart übernimmt sie als `r0` und leert das Register (Teil 1, I5); `ladestart_hat_leeres_register_und_haelt_die_revision` und T17 messen beide Seiten. |
| **C** Verträge und Längen | Die 32 kommt weiter aus `undo_tiefe` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2372`), `state_revision` aus `eq-copilot/schemas/state/nakama-state-v2.md:36`. Kein neuer Wire-Code für die Fehlerausgänge (§5.6.3 Feinheit 4 sinngemäß). |
| **D** Bau- und Prüfriegel | Nicht berührt. |
| **E** Behauptung ≤ Messung | Die tragende Zeile. Jede neue Tabellenzeile trägt ihren B7-Fall und einen Rotbeweis an der eigenen Zusage; M-76 behauptet nur noch, was sein Lauf misst, und M-125 nur, was die Tabelle trägt. Die Arithmetik ist gerechnet, §3.15 gezählt. |
| **F** Änderungssatz | Die neuen Paare stehen beieinander: T4↔T16 (Bedienänderung gegen Ganzzustands-Wiederherstellung) und T1↔T17 (Wiederholung innerhalb der Sitzung gegen über die Sitzungsgrenze), dazu T9 mit seiner Abgrenzung gegen T5. M-76 und M-125 sind im selben Änderungssatz getrennt worden, damit keine von beiden allein wandert. |

---

### 7.11 Matrixprüfung 6 — NEEDS_WORK; vier von fünf NAK-245-Befunden geschlossen, ein Restdefekt (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08bf0-57cd-7b03-9c03-0e9dac477a95`; Lauf 17:29–17:41 |
| Prüfbereich | Wiederprüfung (Vorlage B) über `git diff 4d769e43...f8f74880 -- docs/beweise/SONDE-015.md`; HEAD während des Laufs `834d5ab9` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-matrixpruefung-6-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-matrixpruefung-6-f8f7488.txt` |
| Urteil | **NEEDS_WORK** — **D-245-1, D-245-2, D-245-4 und D-245-5 geschlossen** (M-75 zitiert T2/T3/T4/T13/T5/T14/T15; T4 auf Bedienänderung begrenzt, T16 committet mit Verletzungsmeldung; `r0` definiert, I1 `\|R\| = min(r − r0, 32)`, I2 `e > r0`, T10/T11 zählen diese Sitzung; M-76 mit einer logischen Transaktion, Tabellenlauf allein M-125; 126 Matrix-IDs, 17 T-IDs, neun MR bestätigt). **D-245-3 offen:** die E-18-Ausnahme (ein Zustand mit `eq_enabled = false` braucht keine Bank) steht in S5 und T5, aber nicht in S8, in der T9-Abgrenzung, in M-44 und in M-124 — bei vollem Pool verlangen diese `busy_retry` oder eine freie Bank auch für ein bankfreies Programm, während T6 Commit verlangt und der gebaute Kern eine ENDE-Marke ohne Bank publiziert (`eq-copilot/plugin/dsp/DspKern.cpp:170-177`, E-18); der gebaute Vertrag hat Vorrang |
| Rundenbilanz | `4d769e43..f8f74880: Doku 1 Datei(en) +220/-63 → OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)` — Matrixrunde, strukturell null |
| Quellencheck des Dirigenten | S5 (Teil 2) und T5 tragen die Ausnahme („braucht das Programm eine Bank“); S8 sagt „das Programm aus S5 in eine freie Bank legen und publizieren“ ohne Ausnahme; T9 setzt „S5, weil eine Bank frei ist“ voraus, und der Rotbeweis „T9 committet bei vollem Pool statt `busy_retry`“ träfe einen korrekten bankfreien Commit; M-44 („kein Slot ist `free`, ein Programm soll gebaut werden“) und M-124 („ist der Pool dann voll, ist der Ausgang T5“) nennen den vollen Pool ohne Ausnahme; ebenso der Absatz unter der Tabelle („bei vollem Pool gilt nach I6 T5“). Einordnung: **DEFEKT** — innerer Widerspruch zwischen S5/T5 und S8/T9/M-44/M-124, und ein Rotbeweis, der richtiges Verhalten als Fehler wertet |
| Entscheid | **Nacharbeit 1 der NAK-245-Runde (Runde 1 von 3)** mit genau diesem Defekt. Regel des Dirigenten: „bankpflichtig“ (`eq_enabled = true`) wird als Begriff in Teil 2 definiert; S8 publiziert ein bankfreies Programm als ENDE-Marke ohne Bank (E-18); Vollpool-Abweisung und Bankbedingung in T5, T7, T9 (Zustand, Abgrenzung, Rotbeweis), M-44, M-124 und im Absatz unter der Tabelle werden auf bankpflichtige Programme begrenzt; der bankfreie Retry bei vollem Pool ist ein eigener B7-Fall mit Ausgang Commit. Auftrag `docs/beweise/roh/SONDE-015-nak245-nacharbeit-1-auftrag.txt`; danach Matrixprüfung 7 über den Fixdiff (Vorlage B) |

### 7.12 NAK-245-Runde, Nacharbeit 1 — was eingearbeitet wurde (10.09.2026)

Auftrag wörtlich: `docs/beweise/roh/SONDE-015-nak245-nacharbeit-1-auftrag.txt`;
Anlass ist das Urteil der Matrixprüfung 6
(`docs/beweise/roh/SONDE-015-matrixpruefung-6-f8f7488.txt`, §7.11). Startstand
`a7061af5`, Worktree sauber. **Kein Produkt-, Test-, Schema-, Fixture- oder
Werkzeugcode**; einzige geänderte Datei ist dieses Manifest. Die Regeln der
NAK-245-Runde (`docs/beweise/roh/SONDE-015-nak245-auftrag.txt`) gelten weiter.
Der Kern ist nur als Quelle gelesen worden: `DspKern::uebernehmeZustand`
(`eq-copilot/plugin/dsp/DspKern.cpp:164-200`), `DspBankPool::publiziereEnde`
(`eq-copilot/plugin/dsp/DspBankPool.cpp:153-158`) und `baueProgramm`
(`eq-copilot/plugin/dsp/DspProgramm.cpp:162-176`).

**Der Defekt.** Matrixprüfung 6 hat D-245-3 nicht geschlossen (Urteil, Review
comment [P2]): Die E-18-Ausnahme — ein Zustand mit `eq_enabled = false` braucht
keine Bank — stand in S5 und T5, aber nicht in S8, nicht in Abgrenzung und
Rotbeweis von T9, nicht in M-44 und nicht in M-124. Für einen Zonen-Retry mit
aktueller `base_revision`, vollem Pool und `eq_enabled = false` verlangten T9
und M-124 `busy_retry` und S8 eine freie Bank, T6 dagegen Commit; der gebaute
Kern publiziert ohne Reservierung eine ENDE-Marke
(`eq-copilot/plugin/dsp/DspKern.cpp:170-177`, E-18). Ein korrekter bankloser
Commit wäre am Rotbeweis von T9 und M-124 gefallen, und die vorgeschriebene
Bankbelegung in S8 wäre für ihn nicht ausführbar gewesen.

**Die Regel des Dirigenten, kurz.** (1) Teil 2 definiert „bankpflichtig“ an
genau einer Stelle: `eq_enabled = true` im Kandidatenzustand; ein Zustand mit
`eq_enabled = false` ist bankfrei und wird nach E-18 als ENDE-Marke ohne Bank
publiziert; keine neue Zahl, keine neue Stufe. (2) S8 legt ein bankpflichtiges
Programm in eine freie Bank und publiziert ein bankfreies als ENDE-Marke; beides
scheitert in S8 nicht. (3) T5 „bankpflichtig“; T7 bleibt über „wie T5“
bankpflichtig; T9 mit „S5, weil das Programm bankfrei ist oder eine Bank frei
ist“, zweigeteilter Abgrenzung, begrenztem Rotbeweis und Gegenstück. (4) M-44,
M-124 und der Absatz unter der Tabelle nur für bankpflichtige Programme; M-125
bleibt tabellengetrieben; eine neue T-Zeile nur, wenn eine Eingabe zwei Ausgänge
hat. (5) `git grep` über §3 und §5.11.4.

| Stelle | Änderung |
|---|---|
| §5.11.4 Teil 2, **S5** | `busy_retry` nur noch, wenn das Programm bankpflichtig und keine Bank frei ist; der Satz „Ein Zustand mit `eq_enabled = false` braucht keine (E-18)“ ist durch „Ein bankfreies Programm scheitert in S5 nicht“ ersetzt, damit `eq_enabled` in Teil 2 nur in der Definition steht |
| §5.11.4 Teil 2, **S8** | bankpflichtig: in eine freie Bank legen und publizieren; bankfrei: ENDE-Marke ohne Bank (E-18); „beides kann hier nicht scheitern“ |
| §5.11.4 Teil 2, neuer Absatz **„Bankpflichtig und bankfrei“** | die Definition, unmittelbar vor „Warum S5 keine Bank belegt“ |
| §5.11.4 Teil 2, Absatz **„Warum S5 keine Bank belegt“** | das eine Scheitern gilt einem bankpflichtigen Programm; dessen Bank wird in S8 belegt; neu die Quelle, warum die ENDE-Marke in S8 nicht scheitert und nicht alloziert |
| §5.11.4 Teil 3, **I6** | „(ein bankpflichtiges T9 bei vollem Pool endet in T5)“ — die Regel nennt I6 nicht; gefunden hat die Stelle der `git grep` aus Punkt 5 |
| §5.11.4 Teil 4, **T5** | Zustand „alle vier Bänke belegt und das Programm ist bankpflichtig (Teil 2)“ |
| §5.11.4 Teil 4, **T7** | ohne Textänderung: „wie T5“ trägt die Bankpflicht mit |
| §5.11.4 Teil 4, **T9** | Zustand „S5, weil das Programm bankfrei ist oder eine Bank frei ist“; Abgrenzung zweigeteilt: bankpflichtig `retry_nach_zonenloesung_bei_vollem_pool_ist_busy_retry` (T5), bankfrei der neue Fall `retry_nach_zonenloesung_bei_vollem_pool_ohne_bankpflicht_committet` (Commit); Rotbeweis „ein bankpflichtiges T9 committet bei vollem Pool statt `busy_retry`“ und Gegenstück „ein bankfreies T9 bekommt bei vollem Pool `busy_retry` statt Commit“ |
| §5.11.4, Absatz unter der Falltabelle | zweigeteilte Abgrenzung: bankpflichtig nach I6 T5, bankfrei Commit nach T9 |
| **M-44** (§3.5) | Ereignis „ein bankpflichtiges Programm“; Satz zum bankfreien Zustand; B7-Fall an T9; Rotbeweis „Ein fünfter bankpflichtiger Programmwunsch …“ und neu „ein bankfreier Zustand bekommt bei vollem Pool `busy_retry` statt eines Commits“; Quelle E-18 und Matrixprüfung 6; Marke **MR1** |
| **M-124** (§3.9) | T5 nur für ein bankpflichtiges Programm, ein bankfreies committet (T9); neuer B7-Fall; beide Rotbeweise der zweigeteilten Abgrenzung; Marke **MR1** |
| §3.15 | Stand, Zeile MR1, Mehrfachmarken ausgeschrieben, Verlauf |

**Entscheidungen innerhalb der Regel, begründet.**

1. **Keine neue T-Zeile.** Geprüft ist jede Zeile, die S5 einschließt, gegen beide Programmarten bei vollem Pool. **Bankpflichtig:** S5 entscheidet, Ausgang T5; T6, T7, T9 und T14 bis T17 setzen ein Bestehen von S5 voraus und scheiden aus, und ein T9-, T16- oder T17-Ablauf endet nach I6 ebenfalls in T5 — „Zone frei“ in T5 heißt, dass S3 nicht entscheidet, und das gilt auch für eine Ganzzustands-Wiederherstellung, für die S3 nicht gilt. **Bankfrei:** S5 besteht, T5 scheidet aus, und der Ausgang ist der der Zeile, deren übrige Bedingungen zutreffen — T6 für eine neue Transaktion, T9 für den Zonen-Retry, T16 und T17 für ihre Abläufe, T14 und T15, wenn S6 oder S7 scheitert. T13 entscheidet in S4, vor jeder Bankfrage. Jede Eingabe hat damit genau einen Ausgang; die Zeilenzahl der Tabelle bleibt, und die beiden Fälle hängen an T5 und T9.
2. **Die Bankpflicht gilt auch bei Hard-Bypass.** Der Kern prüft vor der Reservierung nur `eq_enabled` (`eq-copilot/plugin/dsp/DspKern.cpp:170`, `:179`); Hard-Bypass ist nur ein Wert des gebauten Programms (`eq-copilot/plugin/dsp/DspProgramm.cpp:171`), und ein eingeschalteter Zustand mit Hard-Bypass belegt deshalb eine Bank wie jeder andere. R9 und M-50 fassen „`eq_enabled` false oder Hard-Bypass“ als Passthrough zusammen; für die Bankfrage trennen sich beide, und die Definition sagt das, damit aus dem Passthrough keine Bankfreiheit abgeleitet wird.
3. **`eq_enabled` steht in §5.11.4 nur in der Definition.** Die Regel verlangt den Begriff an genau einer Stelle. S5 trug die E-18-Ausnahme bisher als eigenen Satz mit `eq_enabled = false`; er verwendet jetzt den Begriff. M-44 nennt `eq_enabled = false` als Klammer, weil die Regel den Satz so vorgibt, und verweist für den Begriff auf Teil 2.
4. **Die ENDE-Marke in S8 ist an der Quelle gemessen.** `DspBankPool::publiziereEnde` ist `noexcept`, tauscht das Publikationswort des Pfades atomar und liefert nur einen verdrängten, noch nicht übernommenen Slot oder −1 — keinen Fehler, keine Allokation (`eq-copilot/plugin/dsp/DspBankPool.cpp:153-158`, `eq-copilot/plugin/dsp/DspBankPool.h:172-176`). S8 behält deshalb „fehlbar: nein“ und „allokiert: nein“.
5. **M-44 misst den bankfreien Satz am neuen B7-Fall, nicht an einem neuen B6-Fall.** Die Zusage ist ein Commit und damit eine Aussage des Transaktionskerns (B7); B6 kennt keine Transaktion, und Etappe 3 ist abgeschlossen (§9.14). Der Fall `retry_nach_zonenloesung_bei_vollem_pool_ohne_bankpflicht_committet` gibt dem Kern genau die Eingabe, um die es geht — unbekannte `tid`, weil T4 nichts memoisiert hat (I4), aktuelle `base_revision`, gültige bankfreie Nutzlast, voller Pool. Ein Kern, der ihr `busy_retry` gibt, fällt dort; das ist der Rotbeweis an M-44. M-44 nennt für die Zusage T6 und T9, weil T6 dieselbe Eingabe ohne den Zonen-Vorlauf trägt.
6. **§1.3 und §5.17 bleiben unverändert.** Entwurf §44.2 sagt „Ist kein Slot frei, erhält der Befehl `busy_retry`“ im Zusammenhang des Control-Workers, der „ausschließlich `free`-Slots“ schreibt (§1.3); für einen bankfreien Zustand schreibt er keinen Slot (E-18). Die Begrenzung auf bankpflichtige Programme weicht deshalb von keinem Wortlaut ab und bekommt keine Zeile in §5.17.
7. **Der MR-Vermerk in M-124 bleibt stehen.** „widersprach bei vollem Pool T5“ beschreibt den Stand vor der NAK-245-Runde und sagt nichts zu; der MR1-Vermerk dahinter nennt die zweigeteilte Abgrenzung.

**Ergebnis des `git grep` (Punkt 5).** Befehl: `git grep -n -e "vollem Pool" -e
"alle vier Bänke" -e "kein Slot" -e "Kein Slot" -e "keine Bank frei" -e "freie
Bank" -- docs/beweise/SONDE-015.md`, gefahren nach dem Umschreiben und vor dem
Einfügen dieses Abschnitts; die Zeilennummern gelten zu diesem Stand. Treffer:
236, 1097, 1105, 1153, 1890, 1894, 1914, 1919, 1950, 1968, 1972, 1993, 2619,
2628, 2630, 2664, 2687, 2703, 2705, 2706, 3325.

| Zeile | Stelle | Urteil |
|---|---|---|
| 1097 | M-44 (§3.5) | Ereignis bankpflichtig; „vollem Pool“ nur im Satz und im Rotbeweis zum bankfreien Zustand |
| 1105 | M-122 (§3.5) | keine Vollpool-Aussage („kein Slot gleichzeitig `audio_active` und `preparing`“) |
| 1153 | M-124 (§3.9) | Zusage, Fälle und Rotbeweise zweigeteilt; der MR-Vermerk ist Verlauf (Punkt 7) |
| 1890 | §5.11.4 **S5** | bankpflichtig |
| 1894 | §5.11.4 **S8** | Bankbelegung nur für ein bankpflichtiges Programm |
| 1914 | Absatz „Bankpflichtig und bankfrei“ | die Definition selbst |
| 1919 | Absatz „Warum S5 keine Bank belegt“ | bankpflichtig |
| 1950 | **I6** | bankpflichtig |
| 1968 | **T5** | bankpflichtig |
| 1972 | **T9** | beide Hälften |
| 1993 | Absatz unter der Falltabelle | beide Hälften |
| 236 | §1.3, Entwurf §44.2 wörtlich | Zitat, unverändert (Punkt 6) |
| 2619 bis 2706 | §7.10 und §7.11 | Verlauf, unverändert |
| 3325 | §9.3, M-28 | Verlauf, keine Vollpool-Aussage |

Keine Stelle in §3 oder §5.11.4 nennt die Vollpool-Abweisung oder die
Bankbelegung ohne die Bankpflicht. Varianten der Suchwörter („Pool voll“,
„vollen Pool“, „eine Bank frei“, „freier Bank“, „Bänke belegt“) treffen dort
zusätzlich nur Stellen ohne eine solche Aussage: „vollen Pool“ in den
MR1-Vermerken von M-44 und M-124 (Verlauf); „wäre der Pool voll“ und „Bänke
belegt nur der Control-Worker“ in der Begründung, warum S5 keine Bank reserviert;
„bei freier Bank“ (kein zweiter Fehlerausgang); T7 „eine Bank freigegeben“
(bankpflichtig über „wie T5“).

**Zählung nach der Nacharbeit** (§3.15, aus den Zeilen gemessen): **126**
Matrixzeilen, lückenlos M-01 bis M-126, jede mit sieben Spalten; 76 BELEGT, 82
ENTSCHIEDEN (davon 50 allein), 126 BAULÜCKE, 0 OFFEN; 15 MN1, 6 MN2, 6 MN3, 7 MK,
9 MR, **2 MR1** (M-44, M-124). Die Falltabelle trägt unverändert T1 bis T17, jede
Zeile mit neun Spalten; alle Tabellen in §5.11.4 tragen je Zeile gleich viele
Spalten.

**Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für diese Nacharbeit.**

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Die Politik bei vollem Pool steht für beide Programmarten ausdrücklich: bankpflichtig `busy_retry`, nicht memoisiert, Abfluss über T7 (T5, M-44); bankfrei keine Abweisung, sondern Commit mit ENDE-Marke (S8, T9, M-44). Nichts Angenommenes wird still verworfen. |
| **B** Lebenszyklus | aktivieren↔abklingen: der bankfreie Zustand ist der reguläre Weg in die Ruhe (E-18: ENDE-Marke, Fade, ACK). Hinge er am vollen Pool, bekäme das Ausschalten `busy_retry`, obwohl es keine Bank braucht; gemessen an T9 (bankfrei) und M-44. |
| **C** Verträge und Längen | Kein Vertragsfeld und keine Zahl geändert; der Begriff hängt am bestehenden Parameter `eq_enabled` (`param::kIndexEqEnabled`, `eq-copilot/plugin/dsp/DspKern.cpp:170`) und an E-18. |
| **D** Bau- und Prüfriegel | Nicht berührt. |
| **E** Behauptung ≤ Messung | Die tragende Zeile. Jeder Rotbeweis fällt an der Zeile, die seine Zusage trägt: an T9 der bankpflichtige Vollpool-Commit (Abgrenzung) und der bankfreie `busy_retry` (Commit-Zusage von T9), an M-44 der bankfreie `busy_retry` (der neue Satz), an M-124 beide Hälften der Abgrenzung. „S8 kann nicht scheitern“ und „bankpflichtig auch bei Hard-Bypass“ sind an der Quelle gemessen (Punkte 2 und 4); §3.15 ist gezählt; der `git grep` nach den Kernbegriffen steht oben. |
| **F** Änderungssatz | Bankbelegung (S8) und Vollpool-Abweisung (S5, T5) hängen am selben Begriff und sind im selben Änderungssatz begrenzt; die Abgrenzung von T9 trägt beide Seiten mit je einem Fall und Rotbeweis, und M-44 und M-124 ziehen im selben Commit nach. |

**Selbstaudit.** Der Prüferfall ist an der neuen Fassung durchgegangen: ein
Zonen-Retry mit aktueller `base_revision`, vollem Pool und `eq_enabled = false`
besteht S5, committet nach T9 mit `r + 2` und wird memoisiert; derselbe Retry mit
`eq_enabled = true` endet in T5 und nach dem Audio-ACK in T7. S8 ist für beide
Wege ausführbar. Der Wortdiff über §3 und §5.11.4 zeigt nur die Stellen der
Tabelle oben. Der `dokuriegel` läuft sauber.

**Was offen bleibt.** Wie nach §7.10: Die Vollständigkeit der Tabelle ist
konstruiert, nicht bewiesen; M-125 fällt in Etappe 4 an jedem Fall, den keine
Zeile trägt. Dass S5 die Bankpflicht vor der Bankfrage prüft und S8 für ein
bankfreies Programm die ENDE-Marke publiziert, ist Bauarbeit der Etappe 4; der
gebaute Kern trifft dieselbe Unterscheidung schon
(`eq-copilot/plugin/dsp/DspKern.cpp:170-177`).

---

### 7.13 Matrixprüfung 7 — PASS; NAK-245 geschlossen, §5.11.4 ist Referenz für Etappe 4 (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08c15-7c06-7620-9f66-a87b4fa214b2`; Lauf 18:10–18:17 |
| Prüfbereich | Wiederprüfung (Vorlage B) über `git diff a7061af5...c07dc5f2 -- docs/beweise/SONDE-015.md`; HEAD während des Laufs `333c1594` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-matrixpruefung-7-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-matrixpruefung-7-c07dc5f.txt` |
| Urteil | **PASS** — Bankpflicht einmalig über `eq_enabled = true` definiert, übereinstimmend mit E-18 und `DspKern.cpp:170-180`; S8 publiziert bankpflichtig über eine freie Bank, bankfrei als ENDE-Marke ohne Bank; T5/T7/T9, M-44/M-124 und der Absatz unter der Tabelle begrenzen Vollpool-Abweisung und Bankbedingung entsprechend; eigener B7-Fall `retry_nach_zonenloesung_bei_vollem_pool_ohne_bankpflicht_committet`; nichts gebrochen (I6, Gate, Bauplan, Entscheide, Invarianten); 126 Matrix-IDs, 17 T-IDs ohne Umwidmung |
| Rundenbilanz Nacharbeit 1 | `a7061af5..c07dc5f2: Doku 1 Datei(en) +170/-24 → OHNE PRODUKTFORTSCHRITT` — Matrixrunde, strukturell null; NAK-245-Runde gesamt: `4d769e43..c07dc5f2` nur Doku (zwei Matrixrunden, kein Konvergenz-Signal, weil Matrixrunden strukturell null zählen) |
| Etappenstand | **NAK-245 geschlossen** (Register `docs/offene-punkte.md`, Geschlossen-Tabelle, 10.09.2026): die fünf Restwidersprüche der Konvergenzrunde und der Folgebefund zur Bankpflicht sind in §5.11.4 geschlossen; Matrixrunden gesamt: sieben Prüfungen, sechs Nacharbeiten (vier vor der Konvergenzrunde, die Konvergenzrunde, die NAK-245-Runde mit einer Nacharbeit). §5.11.4 (Teil 1 mit `r0`, Teil 2 S0–S8 mit Bankpflicht, Teil 3 I1–I6, Teil 4 T1–T17) ist ab hier die Referenz für den Transaktionskern der Etappe 4; die Erstprüfung der Etappe 4a prüft den Bau gegen sie |
| Nächster Schritt | **Etappe 4a — Prozessor und Transaktionskern** nach `docs/beweise/roh/SONDE-015-etappe-4-auftrag.txt` (Dirigenten-Entscheid: Etappe 4 in 4a Prozessor/Transaktionskern/B7/A16 und 4b Fernweg-Anteile/Kanon geteilt, wegen der Threadlänge eines Bauers); danach Erstprüfung 4a (Vorlage A über den 4a-Diff), 4b, Erstprüfung 4b, Abschlussprüfung über `e9dbf4b9...HEAD`, voller Kanon abgekoppelt |

---

## 8. Bauetappe 2 — Verträge in drei Sprachen (10.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **Etappe 2 — Verträge in drei Sprachen**, nach dem Bauplan §4.2. Die Matrix (§3) ist die Referenz; NAK-245 (§7.9) gehört der Etappe 4 und wurde nicht angefasst. |
| Basis-SHA | `2026031f0a8bb0baf12135020be654a82d354a6f`, mit `git rev-parse HEAD` beim Start gemessen; `git status --short` war leer. |
| Urteil | **Offen.** Die Erstprüfung dieser Etappe steht aus. Was hier steht, ist gemessen, nicht beurteilt. |
| Beine | zwölf gefahren, **alle grün**; Rohausgabe `docs/beweise/roh/SONDE-015-etappe2-beine.txt` (A4 grün erst im Wiederholungslauf, §8.7 N-1) |
| Rotbeweise | **32**, je Matrixzeile einer, `docs/beweise/roh/SONDE-015-rot-<ID>.txt` (gezählt mit `ls docs/beweise/roh/SONDE-015-rot-*`) |
| Kanon | **Nicht gefahren.** Der volle abgekoppelte Lauf gehört an das Ende der Etappe 4 (Bauplan §4.4); diese Etappe fährt die in §4.2 genannten Beine einzeln. |

### 8.1 Was gebaut wurde

**Stufe 1 — Layout v2 und Presetschema.**

- `eq-copilot/schemas/state/nakama-parameter-v2.json` (neu): **120** Kennungen,
  davon **112** Host-Parameter als Präfix der Vertragsreihenfolge. Die Datei
  ist aus der eingefrorenen v1-Datei **abgeleitet**: jeder Eintrag mit
  `layout: "v1"` ist der v1-Eintrag gleicher ID, wörtlich, ergänzt um genau
  die zwei Schlüssel `layout` und `host_parameter`. Die Ableitungsregel steht
  als `ableitung_aus_v1` in der Datei und wird von A12 und B2 bei jedem Lauf
  nachgerechnet — daran fällt eine still geänderte v1-Kennung (M-86).
  Dazu der Zonenvertrag (`schutz_zonen`) und die DTO-Form mit drei
  Wurzelschlüsseln.
- `eq-copilot/schemas/state/nakama-preset-v1.json` (neu): `preset_schema_version`
  1, die mitgeführte `dsp_schema_version`, die **119** Kennungen des
  Klanginhalts (alle außer `v2.global.eq_enabled`), die **sechs** verbotenen
  Top-Level-Namen mit Grund, die Ablehnungsgründe und die Prüfreihenfolge.
- `eq-copilot/schemas/state/nakama-state-v2.md`: das Kind `Dsp` ist definiert
  (§2.0), die Kind-Matrix trägt `active_probe × Dsp = optional`, §4 beschreibt
  das DTO des Layouts v2, §5 nennt die neuen read-only-Gründe, §8 ist um die
  Punkte bereinigt, die jetzt im Schema liegen.

**Stufe 2 — C++-Spiegel.**

- `NakamaParameter.{h,cpp}`: 120 Beschreibungen in Vertragsreihenfolge mit
  `hostParameter`- und `layout`-Attribut, drei `static_assert` auf 109/112/120,
  `Schutzzone`, `DspSatz`, `validiereZonen`, DTO-Kanon und `state_hash` über
  Werte **und** Zonen, `ausDtoText` mit den Zonenstufen, `setzeOccupiedAusV1`
  (bitgenau), `berichtDtoPruefen` (Hash zuerst, dann DTO).
- `NakamaState.{h,cpp}`: `UndoArt`, `UndoEintrag`, Schreiber und Leser des
  Kindes `Dsp` (flache und verschachtelte Arrays), Layoutmigration v1→v2,
  Kind-Matrix, `Zustand::dspDto()`, und der Headroomriegel deckt jetzt auch
  den vollen Undo-Ring ab.
- `NakamaPreset.{h,cpp}` (neu): Writer (RFC-8785-kanonisch) und Leser mit der
  Stufenfolge aus dem Vertrag; `v2.global.eq_enabled` bleibt beim Laden
  unberührt.
- `eq-copilot/plugin/CMakeLists.txt`: `state/NakamaPreset.cpp` im Kern.

**Stufe 3 — v3-Vertrag `state_report.dsp`.**

- `eq-ipc-v3.schema.json`: `$defs/dsp_bericht` und `$defs/dsp_klemmung`,
  `state_report.dsp` optional. Die Nutzlast ist die gehashte
  RFC-8785-**Zeichenkette** (`jcs`), daneben `auto_gain_db`, `klemmungen` und
  `verletzte_baender` als eigene Pflichtfelder.
- `reservierte-nachrichten-v1.json`: Fassung P1 **4 → 5**, Eintrag
  `fassungen."5"`, `state_report.dsp` von `reservierte_felder` nach
  `belegte_felder` umgebucht. `command_ack.applied_dsp` und
  `state_report.eq_enabled` bleiben unangetastet reserviert.
- `pruefe_v3_vertrag.py`: der Riegel wechselt die Seite (`dsp_ist_belegt`
  statt „reservierte Felder … abgelehnt"), `fassung_4_schema()` neu,
  `fassung_3_schema()` setzt jetzt darauf auf (Kette),
  `pruefe_sonde015_fassung_5()` neu.
- `erzeuge_v3_fixtures.py`: drei gültige Fälle (`state-report-mit-dsp`,
  `state-report-dsp-ohne-klemmung`, `state-report-dsp-hash-passt-nicht`) und
  sechs Negativfälle; die alte Fixture `reserviertes-feld-state-report-dsp`
  entfällt (Seitenwechsel). Die Nutzlast kommt aus dem **State**-Korpus
  (`dto/gueltig/gemischt.json`), nicht aus einer zweiten Wahrheit.
- Rust: `JSON_SCHEMA_MINOR_AKTIV` 4 → 5, `v3_schema_minor_4_wurzel()`,
  `MINOR_5`, `P0_/P1_SCHEMA_MINOR` 5, `liveness.rs` rechnet `SHA-256(dsp.jcs)`
  nach und weist bei Abweichung den **ganzen** Bericht ab, `ClientStand.dsp_jcs`
  hält den bestätigten DSP je Sonde und erbt ihn über einen Reconnect.
- C++: `kJsonSchemaMinor` 4 → 5; `berichtDtoPruefen` in `NakamaParameter`;
  B3c baut den Fassung-4-Leser aus der committeten Fassung 5 zurück.
- `broker/src/dto.rs`: liest jetzt `nakama-parameter-v2.json`, baut die 120
  Einträge **über `ids`** (die Vertragsreihenfolge ist nicht mehr
  `global` + `band_vorlage` × Slots), und `pruefe_zonen` trägt die Zonenleiter
  mit den drei neuen Gründen.

**Stufe 4 — FlatBuffers Feld 22.**

- `nakama_telemetry_v1.fbs`: `band_dynamic_gain_db:[float] (id: 22)`.
- `FELD-IDS.json`: ID 22 eingetragen, `erwartet_gesamt.felder` 63 → 64.
- `flatc` regeneriert (C++ und Rust, beide committet); A9 meldet Drift 0.
- `broker/src/telemetrie.rs`: die Funktion `strukturriegel` kennt
  `Frame::VT_BAND_DYNAMIC_GAIN_DB` (ohne diese Zeile liefe das Offsetfeld
  still an ihm vorbei — die Lücke aus T2-Runde 4).
- Beide Leser prüfen Länge 0 oder 8 und Endlichkeit; Abwesenheit bleibt gültig.
- `erzeuge_fb_fixtures.py`: drei gültige und sechs ungültige Fälle.
- `kFeatureBatchSchemaMinor` (P2) 1 → 2.

### 8.2 Entscheide dieser Etappe (Lücken, die der Bau sonst still gefüllt hätte)

| Nr. | Lücke | Entschieden | Warum |
|---|---|---|---|
| E2-1 | Woran erkennt der State-Leser, ob `Parameters` im Layout v1 oder v2 steht? | Eine **additive Eigenschaft** `dsp_schema_version` am Knoten `Parameters`: fehlt sie, ist es Layout v1; 2 ist Layout v2; jede andere Zahl macht den Stand read-only. | Aus der Anwesenheit der `v2.*`-Eigenschaften zu schließen wäre implizit und bei einem halb geschriebenen Stand falsch. Die Eigenschaft ist additiv — ein Build, der sie nicht kennt, liest die 109 v1-Werte weiter (`nakama-state-v2.md` §2.1). |
| E2-2 | Wird `Dsp` immer geschrieben? | **Nur wenn es etwas trägt**: `state_revision != 0` oder ein `occupied` oder eine Zone oder ein Undo-Eintrag. | Ein frischer, unberührter Stand bleibt damit für einen Build lesbar, der `Dsp` noch nicht kennt. Dieselbe Regel wie beim nie gesetzten `MainProject.assistant_step_v1`. |
| E2-3 | Form eines Undo-Eintrags im Baum | Ein **eigenes verschachteltes Array** je Eintrag: `[art, slot, revision, 120 Werte, 4·z Zonenwerte]`, Länge `123 + 4·z`. | Die Eintragsgrenze folgt damit aus der Struktur statt aus einer mitgeschriebenen Zählung, die von ihr abweichen kann. Gemessen: der volle Ring aus 32 Einträgen bleibt weit unter 16 MiB und unter den 64 Baumknoten. |
| E2-4 | Wörter der geschlossenen `art`-Menge | `apply`, `revert`, `neutralisieren`, `remove`, `preset_laden`, `gestus`. `undo` und `redo` fehlen absichtlich. | §5.11 Feinheit 3: Undo und Redo bewegen einen **Cursor** im selben Ring, sie legen keinen Eintrag ab. Zwei Ringe könnten auseinanderlaufen. |
| E2-5 | Ablehnungsgründe der Zonen | Drei neue Wörter: `zone_anzahl`, `zone_doppelt`, `zone_sortierung`. Struktur, Typ, Bereich und Nichtendlichkeit nutzen die **bestehenden** Wörter. | Die drei neuen betreffen die **Liste**, nicht einen Wert — als `bereich` gemeldet wären sie in allen drei Beinen von einem Wertfehler nicht unterscheidbar. Die übrigen Fälle brauchen kein neues Wort. |
| E2-6 | Reihenfolge der Zonenliste | **Streng aufsteigend nach `id`**, im DTO geprüft. | Eine Menge hat keine Reihenfolge; der Vertrag legt eine fest, damit dieselbe Zonenmenge immer denselben kanonischen Text und damit denselben `state_hash` ergibt (M-72, M-93). |
| E2-7 | Obergrenzen der abgeleiteten Angaben in `state_report.dsp` | `jcs` höchstens **8192** Zeichen, `auto_gain_db` in **±120 dB**, `klemmungen` und `verletzte_baender` je höchstens **8**. | Gemessen: das größte kanonische DTO des Layouts v2 misst **3857** Bytes. Acht Bänder zu je 12 dB im selben Kanalmodus ergeben höchstens 96 dB Plateau; 120 ist die Reserve. Höchstens eine Klemmung und ein verletztes Band je Slot. |

### 8.3 Rotbeweise

Je Zeile wurde die **Zusage an ihrer Stelle** gebrochen, das Bein gefahren, die
Rohausgabe abgelegt und die Quelle wiederhergestellt. Ein Bruch am
Prüfwerkzeug statt am Vertrag zählt nicht: der erste Versuch für M-104 brach
`pruefe_v3_vertrag.py` und blieb **grün** — er wurde verworfen und durch den
Bruch am Schema ersetzt (ein Feld des reservierten Namens `eq_enabled` wirklich
anlegen).

| ID | Zusage, die gebrochen wurde | Gebrochene Stelle | Bein |
|---|---|---|---|
| M-14 | Wert außerhalb des Bereichs wird abgelehnt, nicht geklemmt | `NakamaParameter.cpp` Bereichsprüfung | B2 |
| M-24 | Ausschalten ändert ausschließlich `dynamic_enabled` | `schreibeInBaum` setzt die fünf Werte „vorsorglich" zurück | B2 |
| M-28 | `channel_mode` ist diskret aus genau fünf Werten | sechstes Enumwort im Vertrag | A12 |
| M-65 | Migration bitgenau, nicht mit Epsilon | `weichtVomDefaultAb` mit 1e-9 | B2 |
| M-66 | `low_hz < high_hz`, Gleichheit fällt | `>=` zu `>` gelockert | B2 |
| M-72 | Zonen sind Teil des `state_hash` | leere Zonenliste im DTO-Kanon | B2 |
| M-73 | Die neunte Zone wird abgewiesen | Grenze auf 12 gehoben | B2 |
| M-77 | Hash aus dem Bestandskanon | `dsp_schema_version` im Kanon verfälscht | B2 |
| M-78 | Undo-Ring hält Tiefe 32 | Grenze auf 128 gehoben | B2 |
| M-86 | Die 109 v1-Kennungen bleiben unverändert | `v1.global.width` Default 1.0 → 1.5 | A12 |
| M-87 | `occupied` ist kein Host-Parameter | `host_parameter: true` gesetzt | A12 |
| M-89 | `Dsp` führt flache Arrays | `occupied` als Kindknoten | B2 |
| M-90 | Ein v1-Stand lädt verlustfrei | Leser liest 112 statt 109 Eigenschaften | B2 |
| M-91 | DTO exakt, Baum additiv | Wurzelschlüsselzahl nicht mehr geprüft | B2 |
| M-92 | `Dsp` nur bei `active_probe` | Kind-Matrix-Zeile entfernt | B2 |
| M-93 | Roundtrip bytegleich | Zonenliste unsortiert geschrieben | B2 |
| M-94 | Korpus vom Writer, nicht von Hand | Fixture von Hand editiert | A12 |
| M-95 | Zwei getrennte Versionen im Preset | `dsp_schema_version` weggelassen | B2 |
| M-96 | Klanginhalt vollständig | `mix` aus dem Preset entfernt | B2 |
| M-97 | Identitätsfeld wird abgewiesen | Riegel entfernt | B2 |
| M-98 | Unbekanntes Top-Level-Feld wird ignoriert | exakte Wurzelschlüsselmenge erzwungen | B2 |
| M-99 | Unbekannter Preset-Major wird abgelehnt | Versionsvergleich entfernt | B2 |
| M-101 | `state_report.dsp` ist wirklich belegt | Feld im Schema umbenannt | A5 |
| M-102 | Der Minor steigt genau einmal | `vorher` auf 3 gesetzt | A5 |
| M-103 | Die Nutzlast ist eine Zeichenkette | als Objekt typisiert | A5 |
| M-104 | `state_report.eq_enabled` bleibt reserviert | Feld wirklich angelegt | A5 |
| M-105 | Der Empfänger rechnet den Hash nach | Vergleich abgeschaltet | B3c |
| M-108 | Ein Bericht ohne `dsp` bleibt gültig | `dsp` zur Pflicht gemacht | A5 |
| M-109 | Feld-ID 22 ist die nächste freie | ID auf 21 gesetzt | A9 |
| M-110 | Genau acht Einträge in Slotreihenfolge | Fixture mit sieben Werten | A10 |
| M-111 | Ein Frame ohne das Feld bleibt gültig | Feld als `required` deklariert | A9 |
| M-112 | Länge ≠ 0 und ≠ 8 wird abgewiesen | Längenprüfung abgeschaltet | B3c |

**M-88 fällt hier nicht** und ist keine Lücke: MN1 (B-11, §4.2) hat die Zeile
ausdrücklich in die Pflichtmenge der **Etappe 4** verschoben — sie fordert 112
tatsächlich exponierte APVTS-Parameter, und der Prozessor bleibt nach §4.1 in
dieser Etappe unberührt. Die **Vertragsseite** derselben Frage liegt in M-86
und M-87 und ist oben belegt.

### 8.4 Die Beine dieser Etappe

Alle Zahlen sind aus der Rohausgabe gelesen, nicht abgeschrieben
(`docs/beweise/roh/SONDE-015-etappe2-beine.txt`).

| Bein | Ergebnis |
|---|---|
| **A5** `pruefe_v3_vertrag.py --abdeckung` | GRÜN — 630 Prüfungen, 0 gescheitert; 115 Definitionen, 26 Nachrichtenfamilien, 401 Fixtures |
| **A8** `erzeuge_v3_fixtures.py --pruefen` | GRÜN — 405 Dateien bytegleich, 111 gültige und 290 ungültige Fixtures |
| **A9** `pruefe_flatc_drift.py` | GRÜN — Drift 0, Feld-IDs 0 rot, 9 Tabellen mit 64 Feldern |
| **A10** `erzeuge_fb_fixtures.py --pruefen` | GRÜN — 129 Dateien bytegleich, 26 gültige und 102 ungültige Fixtures |
| **A11** `pruefe_v2_schemas.py` | GRÜN — die fünf eingefrorenen v2-Verträge unberührt |
| **A12** `erzeuge_state_fixtures.py --pruefen` | GRÜN — 64 Dateien bytegleich, keine verwaiste Datei; 120 Kennungen, 112 Host-Parameter, 109 v1-Einträge wörtlich abgeleitet; DTO 7 gültig / 26 ungültig, Preset 3 gültig / 13 ungültig |
| **B2** `EqCopStateMigrationTest` | GRÜN — 243 Prüfungen, 0 Fehler |
| **B3c** `EqCopSchemaTest` | GRÜN — 149 bestanden, 0 gescheitert |
| **A16** `EqCopProbeeqNullTest` | GRÜN — 87 Prüfungen, 0 Fehler (der Prozessor ist unberührt; der Passthrough bleibt der bewiesene) |
| **B10** `EqCopIpcTest` | GRÜN — 389 Prüfungen, 0 Fehler |
| **A4** `cargo test` (Broker) | GRÜN — 20 Suiten, 0 Fehler, Exit 0. Ein früherer Lauf fiel einmal an einem Lastvektor, der keine Zeile dieser Etappe berührt; der Wiederholungslauf und der Abschlusslauf waren beide grün (§8.7 N-1). |
| **A4-SI** `store_crash_matrix --ignored` | GRÜN |

### 8.5 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Etappe 2

| Zeile | Wo gemessen |
|---|---|
| **A. Rückstau und Prioritätsklassen** | Nicht berührt: diese Etappe ändert keine Queue-Politik. Der P1-Fassungsschritt reist über den bestehenden Weg; B10 (389 Prüfungen) misst ihn unverändert. |
| **B. Lebenszyklus** | Nicht berührt. Der Reconnect-Pfad bekommt in `link.rs` nur ein weiteres geerbtes Feld (`dsp_jcs`), gemessen in `state_report_dsp_wird_gegen_den_state_hash_nachgerechnet`. |
| **C. Verträge und Längen** | `jcs` (maxLength 8192), `dsp_klemmung.id/gemeldet/wirksam` (je 64), `verletzte_baender` (maxItems 8, items 0..7), `klemmungen` (maxItems 8) — jedes mit Negativfixture in `erzeuge_v3_fixtures.py`. Exakte Feldmenge: `dsp_bericht` und `dsp_klemmung` sind `additionalProperties:false` mit C++- (B3c) und Rust-Hälfte (A4). NaN/Inf: `band_dynamic_gain_db` in beiden Lesern, Zonen und Parameterwerte in allen drei DTO-Beinen. |
| **D. Bau- und Prüfriegel** | A9 ist fail-closed: das neue Offsetfeld ohne Riegelzeile in `telemetrie.rs` war **rot**, bevor die Zeile stand (gemessen beim Bau, §8.7 N-2). A12 und A8 melden verwaiste Dateien — beide taten es für die zwei Fixtures, die die Seite gewechselt haben. |
| **E. Behauptung ≤ Messung** | 32 Rotbeweise, je einer an der Zeile, die die Zusage trägt (§8.3). Alle Zahlen dieses Abschnitts sind aus der Rohausgabe gelesen. Der lebende Kopf ist nachgezogen; alles darunter bleibt Verlauf. **Writer-Fixtures statt Handschrift:** die zwei neuen Goldens `schema2/dsp-v2-voll.bin` und `schema2/layout-v1.bin` kommen aus `state::speichere`, der Presetkorpus aus `nakama::preset::schreibe` (B2 misst die Bytegleichheit), die v3-Nutzlast aus dem State-Korpus. |
| **F. Änderungssatz** | speichern↔laden: Writer, Leser und Migration des Kindes `Dsp` sowie Writer und Leser des Presets liegen in **einem** Commit, jeder mit seinem Gegenpfad im selben Test. Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher des Layouts v2: `nakama-parameter-v2.json`, `NakamaParameter.cpp`, `broker/src/dto.rs`, `erzeuge_state_fixtures.py`, Fixtures — ebenfalls **ein** Änderungssatz. |

### 8.6 Abweichungen dieser Etappe

**Keine Abweichung von R1 bis R15.** Zwei Stellen sind Feinheiten, die die
Regeln offen ließen und die §8.2 als E2-1 bis E2-7 benennt; sie widersprechen
keiner Regel.

Eine Stelle weicht vom **Bauplan §4.2** ab und ist hier benannt: die Stufe 3
nennt „C++-Leser und Rust-Leser mit Fassungsleiter". Der C++-Leser des
`state_report` ist die **Schema-Engine** (`nakama::vertrag::Schema`), die den
committeten Vertrag liest — sie braucht keinen eigenen Handparser. Gebaut
wurde deshalb nur, was sie **nicht** kann: die Nachrechnung des `state_hash`
(`parameter::berichtDtoPruefen`) und der Rückbau auf die Fassung 4 im Bein B3c.
Ein handgeschriebener zweiter `state_report`-Parser in C++ wäre die zweite
Leserwahrheit, gegen die SONDE-005a die Engine überhaupt gebaut hat.

### 8.7 Nebenbefunde (nicht ins Register)

**N-1 — `cargo test` fiel einmal an einem Lastvektor, nicht an dieser Etappe.**
Im ersten vollen Lauf fiel `security_vectors::zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz`
an `warten(4000, || griff.aktive_worker() == MAX_VERBINDUNGEN)` — ein
Zeitfenster von vier Sekunden für 96 Worker, gefahren parallel zu einem
laufenden `cmake --build` derselben Maschine. Der Einzellauf desselben Tests
war grün (0,60 s); der **volle Wiederholungslauf auf demselben Stand, ohne
parallelen Bau, war grün mit Exit 0** (angehängt an dieselbe Rohdatei). Der
Vektor liegt in `broker/tests/security_vectors.rs` und berührt keine Zeile
dieser Etappe: der Diff dieses Commits fasst weder Listener noch Worker noch
die Verbindungsobergrenze an. Kein Registerpunkt — der Vektor bindet eine
Wanduhr an eine Maschine unter Last; er ist als Befund über den Lauf, nicht
über das Produkt, hier festgehalten.

**N-2 — der Feld-ID-Riegel greift beim Anlegen eines Offsetfeldes doppelt.**
`pruefe_fbs_feldids.py` meldete beim ersten Lauf nach dem Anlegen von
`band_dynamic_gain_db` zwei rote Zeilen: die fehlende Riegelzeile in
der Funktion `strukturriegel` in `broker/src/telemetrie.rs` und die Feldzahl 63 gegen 64. Beides
ist die zugesagte Wirkung (T2-Runde 4, BL-A), hier zum ersten Mal an einem
Vektorfeld gemessen: der Riegel verlangt für **jedes** Offsetfeld eine Zeile,
auch für einen Vektor aus Skalaren.

**N-3 — `juce::var` hält 0.0 und −0.0 für gleich; `ValueTree::setProperty`
überspringt den Schreibvorgang dann.** Gefunden beim Bau des
Migrations-Rotbeweises M-65: der Wert −0.0 kam nach `speichere/lade` als 0.0
zurück, und ein Slot, den die Migrationsregel R5 als belegt lesen muss, wäre
beim nächsten Laden als frei zurückgekommen. `schreibeInBaum` erzwingt den
Schreibvorgang jetzt über einen leeren `var`, **ohne** die Position der
Eigenschaft im Knoten zu ändern (`removeProperty` hätte sie ans Ende gehängt
und die Bytegleichheit gebrochen). B2 misst beide Hälften: dass der Writer das
Vorzeichen hält und dass die Migration den Slot als belegt liest. Der Fund
gehört in diese Etappe und ist hier behoben — kein Registerpunkt.

**N-5 — drei `-text`-Dateien liegen im Index mit CRLF, ihre Nachbarn mit LF.**
`.gitattributes` markiert `eq-copilot/schemas/**` und `eq-copilot/fixtures/**`
als `-text`, damit keine Zeilenenden-Normalisierung sie still verändert. Der
Baum ist darunter aber **gemischt**: `nakama-parameter-v1.json`,
`nakama_telemetry_v1.fbs`, `reservierte-nachrichten-v1.json` und der ganze
Fixture-Korpus liegen mit LF im Index, `nakama-state-v2.md`,
`eq-ipc-v3.schema.json` und `FELD-IDS.json` mit CRLF. Ein Werkzeug, das mit
`newline="
"` schreibt, schreibt die drei damit vollständig um — der Diff
zeigte 4739 statt 51 geänderte Zeilen an `eq-ipc-v3.schema.json`. Die
Zeilenenden sind für diesen Commit wiederhergestellt und der Diff auf die
echten Änderungen zurückgeschrumpft; ein Nachlauf auf dem Endstand (angehängt
an dieselbe Rohdatei) hält A5, A8, A9, A12, B2, B3c und die Rust-Bibliothek
grün. Kein Registerpunkt, aber eine Landmine für jedes Werkzeug, das eine
dieser Dateien schreibt: **vor dem Schreiben `git ls-files --eol <datei>`
lesen und dieselben Zeilenenden zurückschreiben.**

**N-4 — die Vertragsreihenfolge ist nicht mehr `global` + `band_vorlage` × Slots.**
Alle drei Beine bauten den Bestand bisher aus dieser Formel. Mit den drei
v2-Globalen **hinter** allen v1-Slotfeldern stimmt sie nicht mehr; alle drei
lesen die Reihenfolge jetzt aus `ids`. Ein Bein, das die Formel behalten hätte,
wäre still in einer anderen Reihenfolge gelaufen — und die
Reihenfolge entscheidet über den APVTS-Index (M-88, Etappe 4).

### 8.8 Erstprüfung Etappe 2 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08939-cb7c-7543-bf7c-ce2733441a1f`; Lauf 04:51–05:09 |
| Prüfbereich | Erstprüfung (Vorlage A) über `git diff 2026031f...94a8a336 -- eq-copilot/schemas eq-copilot/fixtures eq-copilot/plugin broker/src broker/tests tools/eq-copilot tools/beweise.ps1`; HEAD während des Laufs `8242fb13` (trägt nur den Prüfauftrag und die CLAUDE.md-Textkorrektur), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-2-erstpruefung-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-2-erstpruefung-94a8a33.txt` |
| Urteil | **NEEDS_WORK** — fünf Befunde: ein P1 (Telemetrie-Sender Minor 2 gegen Broker-Grenze 1 → Verbindung schließt beim ersten Frame), vier P2 |
| Quellencheck des Dirigenten | lesender Opus-Agent, jede zitierte Zeile geöffnet: alle fünf **DEFEKT**. B-01 bricht Produktverhalten seit BASIS (`kFeatureBatchSchemaMinor` 1→2 im Diff, `P2_SCHEMA_MINOR` blieb 1, `tests_fristen.rs:126` sichert das Gegenteil zu); B-02 R13 „validiert" unerfüllt (`dto::pruefe` existiert, wird nicht gerufen; C++ prüft); B-03 M-91/R1/„State bleibt verlustfrei" (Weglassregel entfernt unbekannte Eigenschaft); B-04 und B-05 Rotbeweis nicht an der Zusage (M-105 fällt am Bereichsgrund, M-89-Zusicherung fällt gar nicht). Zusatzfund: §4.2 Punkt 4 nannte nur den Sender, nicht die Broker-Grenze |
| Rundenbilanz | `2026031f..94a8a336: Produkt 101 Datei(en) +7053/-202 · Tests 5 Datei(en) +856/-47 · Pruefwerkzeug 5 Datei(en) +768/-49 · Doku 35 Datei(en) +1395/-6` — Bau mit Produktfortschritt. *(Nacharbeit 1: die vier Trenner standen als rohes Pipe-Zeichen in der Zelle und rissen `tools/plan/dokuriegel.py` — die bekannte Landmine aus `CLAUDE.md`. Ersetzt durch `·`; die Zahlen sind unverändert.)* |
| Nebenbefund des Dirigenten | Der Bauer hat `CLAUDE.md` (Maschinen-Landmine „gemischte Zeilenenden unter `eq-copilot/schemas/**`") mitcommittet — außerhalb der Ticketpfade, inhaltlich richtig und behalten; die zerbrochene Code-Spanne darin hat der Dirigent in `8242fb13` repariert |
| Nacharbeit | Etappe 2, Nacharbeit 1 (Runde 1 von 3): `docs/beweise/roh/SONDE-015-etappe-2-nacharbeit-1-auftrag.txt` — nur die fünf Defekte mit je einer schließenden Regel; nur betroffene Beine |

| Befund | Kurzform | Einordnung | Ansatzpunkt |
|---|---|---|---|
| B-01 | P2-Empfängergrenze nicht mit dem Sender gehoben | DEFEKT (R14, Cross-Language, Bruch seit BASIS) | `broker/src/transport/server_v3/mod.rs:193`, `tests_fristen.rs:126` |
| B-02 | Rust-Leser validiert das DSP-DTO nach der Hashprüfung nicht | DEFEKT (R13, M-105, gleich streng in beiden Sprachen) | `broker/src/coordinator/liveness.rs:585-590`, `broker/src/dto.rs:312` |
| B-03 | Leeres `Dsp`-Kind verliert unbekannte Eigenschaften beim Speichern | DEFEKT (M-91, R1, State verlustfrei) | `eq-copilot/plugin/state/NakamaState.cpp:720-728`, `:1039-1041` |
| B-04 | Hash-Negativfixture verletzt zusätzlich die Bereichsgrenze | DEFEKT des Rotbeweises (M-105) | `tools/eq-copilot/erzeuge_v3_fixtures.py:1461-1462` |
| B-05 | M-89-Zusicherung misst Knotenzahl, nicht Array-Struktur | DEFEKT des Rotbeweises (M-89) | `eq-copilot/plugin/tests/StateMigrationTestMain.cpp:1406-1408` |

### 8.9 Nacharbeit 1 — was eingearbeitet wurde (10.09.2026)

| Merkmal | Wert |
|---|---|
| Auftrag | `docs/beweise/roh/SONDE-015-etappe-2-nacharbeit-1-auftrag.txt` — genau die fünf bestätigten Defekte B-01 bis B-05 mit je einer schließenden Regel |
| Basis-SHA | `3df34963de3a887600f5f629fc637f52256ace78`, mit `git pull --ff-only` geholt; `git status --short` war leer |
| Umfang | fünf Defekte geschlossen, **kein** weiterer Bau. Diff kompakt: 16 Dateien, +477/−25 |
| Beine | **elf gefahren, alle grün** (A5, A8, A9, A9b, A10, A12, B2, B3c, B10, A16, A4); Rohausgabe `docs/beweise/roh/SONDE-015-etappe2-nacharbeit1-beine.txt` |
| Rotbeweise | **sechs**, je an der Zeile, die die Zusage trägt: `SONDE-015-rot-B-01-n1.txt` bis `-B-05-n1.txt` plus `SONDE-015-rot-M-105-n1.txt` (die Gegenprobe zu B-04 am korrigierten Fixture) |
| Urteil | **Offen.** Die Wiederprüfung dieser Runde steht aus. |

| Befund | Geschlossen mit | Datei:Zeile | Test | Rotbeweis |
|---|---|---|---|---|
| **B-01** | Die P2-Fassungszahl steht **genau einmal**: im neuen Register `wire_envelope_schema_minor` von `FELD-IDS.json` (Familie P2, 1 → 2, mit Begründung je Fassung). `P2_SCHEMA_MINOR` steigt auf 2; **beide** Seiten werden gegen dieses Register geprüft — Rust im Test, C++ als Textriegel im Werkzeug. | `eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json` (neuer Block), `broker/src/transport/server_v3/mod.rs:206`, `tools/eq-copilot/pruefe_fbs_feldids.py` (Prüfung 8) | `transportfassung_p2_bindet_den_sender` (neu), `unbekannter_schema_minor_wird_vor_der_senke_abgewiesen` (P2 1 und 2 bekannt, 3 nicht), `tests_abonnement` sendet jetzt mit der **Registerzahl** statt mit der Konstante, A9 `pruefe_fbs_feldids.py` Prüfung 8 | `SONDE-015-rot-B-01-n1.txt` |
| **B-02** | Der Rust-Leser hält dieselbe Reihenfolge wie C++: erst `SHA-256`, dann der exakte DTO-Weg `crate::dto::pruefe`. Bei `Err` wird der Bericht **ganz** abgewiesen; `client.dsp_jcs` bleibt unverändert. | `broker/src/coordinator/liveness.rs:599-601` | `state_report_dsp_wird_gegen_den_state_hash_nachgerechnet` erweitert: Hash stimmt, DTO fällt → abgewiesen, und der gehaltene DSP ist **noch der alte**; C++ misst dasselbe Fixture in B3c | `SONDE-015-rot-B-02-n1.txt` |
| **B-03** | Weggelassen wird das Kind `Dsp` nur, wenn `dspTraegtEtwas` falsch ist **und** der gehaltene Knoten nichts Unbekanntes trägt (`dspTraegtUnbekanntes`, das auch einen Kindknoten als unbekannt zählt). | `eq-copilot/plugin/state/NakamaState.cpp:731-763` und `:1013`, Vertrag `nakama-state-v2.md` §2.0 | **B2**, Fälle `ein Dsp-Kind ohne bekannten Inhalt lädt schreibbar`, `leerer Dsp-Inhalt mit unbekannter Eigenschaft: das Kind bleibt, die Eigenschaft auch` und die Gegenprobe `ohne unbekannte Eigenschaft fällt das leere Dsp-Kind weiterhin weg` | `SONDE-015-rot-B-03-n1.txt` |
| **B-04** | Die Mutation bleibt **innerhalb** der Vertragsgrenzen (`width` 1.25 → 1.5), und der Erzeuger **sichert zu**, dass die mutierte Zeichenkette den DTO-Weg passiert — mit demselben Referenzvalidator, den A12 führt. | `tools/eq-copilot/erzeuge_v3_fixtures.py:1483-1489` (Zusicherung), `dto_grund()` | **A8** (der Erzeuger bricht ab, wenn die Mutation den DTO-Weg nicht passiert), **B3c** misst dasselbe an der Quelle: `die mutierte Zeichenkette passiert den DTO-Weg` | `SONDE-015-rot-B-04-n1.txt` und die Gegenprobe `SONDE-015-rot-M-105-n1.txt` |
| **B-05** | Die M-89-Zusicherung ist **strukturell**: `Dsp` hat null Kindknoten, `occupied_v1`, `schutz_zonen_v1` und `undo_ring_v1` sind Array-Eigenschaften der erwarteten Form, und der Ring führt seine Einträge als eigene Arrays. Die Knotenobergrenze bleibt als **zweite** Zusicherung daneben. | `eq-copilot/plugin/tests/StateMigrationTestMain.cpp:1447-1494` | **B2**, Fälle `Dsp nutzt flache Arrays: kein Kindknoten, occupied_v1/schutz_zonen_v1/undo_ring_v1 sind Arrays` und `und der Baum bleibt weit unter der 64-Knoten-Grenze` | `SONDE-015-rot-B-05-n1.txt` |

**Warum der Rotbeweis zu B-01 nicht durch `tests_abonnement` läuft.** Der volle
Weg durch `verbindung.rs` ist der richtige *Test*, aber der falsche
*Rotbeweis*: fällt die Empfängergrenze zurück, schließt der Broker die
Quellverbindung, und der Test blockiert danach auf einer Pipe, die nie etwas
liefert — ein Hänger ist kein Rot. Gemessen wird deshalb die **Gate-Funktion**,
die `verbindung.rs` unmittelbar vor dem Schließen befragt
(`schema_minor_bekannt`), zusammen mit dem Koppeltest und dem Werkzeugriegel.
Der volle Weg bleibt als grüner Test im Bestand und sendet seither mit der
Registerzahl statt mit der Serverkonstante — vorher folgte er einer
zurückgedrehten Konstante stumm nach unten.

**Was zu B-01 zusätzlich geprüft und für richtig befunden wurde.** Die dritte
P2-Beteiligte ist Gens Empfänger: `SourcesModel::uebernehmeP2` weist einen
Frame ab, dessen `schema_minor` **größer** ist als `kFeatureBatchSchemaMinor`.
Da der Broker seine Pushes mit `P2_SCHEMA_MINOR` schreibt und beide Zahlen
jetzt am selben Register hängen, sind Sender, Transport und Empfänger auf
derselben Fassung. Gemessen an der Quelle
(`eq-copilot/plugin/src/SourcesModel.cpp:1448`), nicht angenommen.

**§4.2 Punkt 4 ist nachgezogen** (Eintrag **N1 (B-01)** dort): die
Bauanweisung nannte nur den Sender; sie nennt jetzt Register, Sender **und**
Empfängergrenze samt Randwerten der Fassungsleiter.

#### 8.9.1 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Nacharbeit 1

| Zeile | Wo gemessen |
|---|---|
| **A. Rückstau und Prioritätsklassen** | Nicht berührt: keine Queue-Politik geändert. B10 (`EqCopIpcTest`) misst den unveränderten Weg. |
| **B. Lebenszyklus** | B-01 berührt genau den Punkt, an dem `verbindung.rs` eine Verbindung schließt. Gemessen wird die Gate-Funktion davor und der volle Weg danach (`tests_abonnement`, grün mit der Registerzahl). |
| **C. Verträge und Längen** | B-02 macht den Rust-Leser **gleich streng** wie C++: Discriminator und Hülle prüft das Schema, den Inhalt der Zeichenkette der DTO-Weg. Beide Sprachen klassifizieren `state-report-dsp-dto-ungueltig` und `state-report-dsp-hash-passt-nicht` identisch (B3c und A4 an derselben Datei). |
| **D. Bau- und Prüfriegel** | Prüfung 8 in `pruefe_fbs_feldids.py` ist fail-closed: fehlt das Register, fehlt eine der zwei Zahlen oder weicht sie ab, ist das **ROT** — kein Default, keine Heuristik. Sie war beim ersten Lauf nach dem Anlegen des Registers von selbst rot (der Rust-Wert stand noch auf 1); der Fix hat sie grün gemacht. |
| **E. Behauptung ≤ Messung** | Sechs Rotbeweise, jeder an der Zeile, die die Zusage trägt (Tabelle oben). Die zwei Rotbeweise der Etappe 2, die die Erstprüfung als defekt benannt hat, sind ersetzt: `M-105` fällt jetzt mit **leerem** Grund („angenommen") statt mit `[bereich v1.global.width]`, `M-89` fällt an der Zeile `Dsp nutzt flache Arrays` mit der gemessenen Ursache `[1 Kindknoten]`. Die alten Dateien bleiben als Verlauf liegen; §8.8 zitiert sie. |
| **F. Änderungssatz** | speichern↔laden: B-03 ändert Schreiber und Vertrag, und B2 misst beide Richtungen im selben Commit. Sender↔Empfänger: B-01 hebt beide Seiten und den Transport dazwischen in **einem** Änderungssatz — genau das war der Defekt. |

#### 8.9.2 Nebenbefund dieser Runde

**N1-1 — die Rundenbilanz in §8.8 riss den Dokuriegel.** Die Zelle trug ihre
vier Abschnitte mit rohen Pipe-Zeichen getrennt; `tools/plan/dokuriegel.py`
zählt sie als Spaltentrenner und meldete „7 Spaltentrenner statt 4". Das ist
genau die Landmine, die `CLAUDE.md` führt („Ein Pipe-Zeichen in einer
Markdown-Tabellenzelle reißt `tools/plan/dokuriegel.py`"). Die vier Trenner
sind zu `·` geworden, die Zahlen sind unverändert; der Riegel ist danach mit
0 Befunden grün. Kein Registerpunkt — eine Textkorrektur an einer fremden
Zelle, hier benannt statt still gemacht.

#### 8.9.3 Die Beine dieser Runde

Alle Zahlen sind aus der Rohausgabe gelesen, nicht abgeschrieben.

| Bein | Ergebnis |
|---|---|
| **A5** `pruefe_v3_vertrag.py --abdeckung` | GRÜN — 631 Prüfungen, 0 gescheitert (630 vor dieser Runde: die neue Prüfung stammt aus dem zusätzlichen Fixture) |
| **A8** `erzeuge_v3_fixtures.py --pruefen` | GRÜN — 406 Dateien bytegleich (das neue Fixture `state-report-dsp-dto-ungueltig` dazu), 112 gültige und 290 ungültige |
| **A9** `pruefe_flatc_drift.py` | GRÜN — Drift 0 |
| **A9b** `pruefe_fbs_feldids.py` | GRÜN — 0 rot, darin die neue Prüfung 8: `P2-Wire-Envelope-Minor: 1 -> 2 (Sender und Empfaenger gegen dieses Register geprueft)` |
| **A10** `erzeuge_fb_fixtures.py --pruefen` | GRÜN — 129 Dateien bytegleich |
| **A12** `erzeuge_state_fixtures.py --pruefen` | GRÜN — 64 Dateien bytegleich, keine verwaiste Datei |
| **B2** `EqCopStateMigrationTest` | GRÜN — 247 Prüfungen, 0 Fehler (243 vor dieser Runde: vier neue aus B-03 und B-05) |
| **B3c** `EqCopSchemaTest` | GRÜN — 152 bestanden, 0 gescheitert (149 vor dieser Runde: drei neue aus B-02 und B-04) |
| **B10** `EqCopIpcTest` | GRÜN — 389 Prüfungen, 0 Fehler |
| **A16** `EqCopProbeeqNullTest` | GRÜN — 87 Prüfungen, 0 Fehler |
| **A4** `cargo test` (Broker) | GRÜN — alle Suiten, 0 Fehler, Exit 0 |

### 8.10 Wiederprüfung 1 Etappe 2 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a089a4-db48-7973-860b-3ec74f4208dc`; Lauf 06:47–06:55 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff 3df34963...9306740d -- eq-copilot/schemas eq-copilot/fixtures eq-copilot/plugin broker/src broker/tests tools/eq-copilot tools/beweise.ps1`; HEAD während des Laufs `8ebcfa91` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-2-wiederpruefung-1-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-2-wiederpruefung-1-9306740.txt` |
| Urteil | **NEEDS_WORK** — **B-02, B-03, B-04, B-05 geschlossen** (je mit Beleg und Rotbeweis an der Zusage); **B-01 nicht vollständig**: Empfängergrenze 2, Annahme 2/Ablehnung 3 und Registerbindung umgesetzt, der von der Regel verlangte Verbindungs-Rotbeweis fehlt (der bestehende Test `tests_abonnement.rs:275-276` würde unter der Mutation `P2_SCHEMA_MINOR = 1` an fristlosem `ReadFile` hängen statt zu fallen); kein weiterer Bruch |
| Quellencheck des Dirigenten | am Urteil und an der Rohdatei `SONDE-015-rot-B-01-n1.txt` (nur Koppeltest, Minor-Prädikat, Textriegel): **DEFEKT** des Rotbeweises zur B-01-Regel |
| Rundenbilanz | `3df34963..9306740d: Produkt 12 Datei(en) +256/-17 · Tests 2 Datei(en) +123/-4 · Pruefwerkzeug 2 Datei(en) +114/-4 · Doku 8 Datei(en) +788/-5` — Runde mit Produktfortschritt. *(Nacharbeit 2: dieselbe Landmine wie in §8.8 — rohe Pipe-Zeichen in der Zelle rissen `tools/plan/dokuriegel.py`. Ersetzt durch `·`; die Zahlen sind unverändert.)* |
| Nacharbeit | Etappe 2, Nacharbeit 2 (Runde 2 von 3): `docs/beweise/roh/SONDE-015-etappe-2-nacharbeit-2-auftrag.txt` — nur der B-01-Rest: Broker-Test über den echten Transportweg mit begrenztem Warten, der unter der Mutation das Schließen der Quellverbindung sieht |

### 8.11 Nacharbeit 2 — was eingearbeitet wurde (10.09.2026)

| Merkmal | Wert |
|---|---|
| Auftrag | `docs/beweise/roh/SONDE-015-etappe-2-nacharbeit-2-auftrag.txt` — genau der eine Restdefekt **B-01 (Rest)**: der Verbindungs-Rotbeweis über den echten Transportweg mit begrenztem Warten |
| Basis-SHA | `c727e620c1d48bcee8ad698a27321980bbd7776e`, mit `git pull --ff-only` geholt; `git status --short` war leer |
| Umfang | **zwei Dateien, +101/−0** — beide im Rust-Transport. Kein Vertrag, kein C++-Ziel, kein Fixture, kein Prüfwerkzeug berührt; die Empfängergrenze selbst stand seit der Nacharbeit 1 richtig. |
| Beine | **A4 und A4-SI, beide grün** (Exit 0); Rohausgabe `docs/beweise/roh/SONDE-015-etappe2-nacharbeit2-beine.txt` |
| Rotbeweis | `docs/beweise/roh/SONDE-015-rot-B-01-n2.txt` |
| Urteil | **Offen.** Die Wiederprüfung dieser Runde steht aus. |

**Was fehlte.** Die Empfängergrenze, die Randwerte und die Registerbindung
waren mit der Nacharbeit 1 umgesetzt und von der Wiederprüfung 1 bestätigt.
Offen blieb der von der Regel ausdrücklich verlangte **Verbindungs**-Rotbeweis:
ein Broker-Test, der unter der Mutation `P2_SCHEMA_MINOR = 1` das **Schließen
der Quellverbindung** sieht. Der erste Anlauf war daran gescheitert, dass
`frame_roh_lesen` ein synchrones `ReadFile` **ohne Frist** fährt: fällt die
Grenze zurück, schließt `verbindung.rs` die *Quell*verbindung, und der Test
wartete danach auf der weiterhin offenen *Main*-Pipe. Ein Hänger ist kein Rot —
deshalb hatte die Nacharbeit 1 den vollen Weg aus dem Rotbeweis genommen und
statt seiner die Gate-Funktion gemessen. Das war die halbe Zusage.

**Wie es geschlossen ist.**

| Teil | Datei:Zeile | Was |
|---|---|---|
| Beobachtung ohne Blockieren | `broker/src/transport/server_v3/tests_hilfe.rs:98-127` | `Testclient::zustand()` fragt mit `PeekNamedPipe` den Pipezustand ab, **ohne zu warten**: `None` = die Gegenseite hat geschlossen (`ERROR_BROKEN_PIPE` / `ERROR_PIPE_NOT_CONNECTED`), `Some(n)` = die Verbindung steht und `n` Bytes liegen bereit. Darauf `verbindung_steht()` und `bytes_verfuegbar()`. `lesen` bleibt unverändert — der Vertrag der bestehenden Tests ist nicht angefasst. |
| Begrenztes Warten | `broker/src/transport/server_v3/tests_hilfe.rs:296-317` | `FRIST_P2_ENTSCHIEDEN_MS` = **2000 ms**, benannt und klein: beide Ausgänge fallen im Lesethread des Servers unmittelbar, die zwei Sekunden sind Reserve für eine belastete Maschine. `warte_auf_p2_entscheidung` kehrt zurück, sobald **eines** von beidem gilt — der Abonnent hat Bytes **oder** die Quellverbindung ist zu. `false` heißt „innerhalb der Frist ist gar nichts passiert" und ist selbst ein Befund, kein Hänger. |
| Die Zusicherung | `broker/src/transport/server_v3/tests_abonnement.rs:275-306` | Der bestehende Test über den **echten** Transportweg beobachtet die Quellverbindung jetzt **vor** jedem blockierenden Lesen. Drei Zusicherungen in dieser Reihenfolge: die Entscheidung fällt innerhalb der Frist · **die Quellverbindung bleibt offen** · `geschlossen_envelope` steht still. Erst danach der Push an den Abonnenten. |

**Warum die Reihenfolge tragend ist.** Stünde die bounded-Beobachtung hinter
dem blockierenden Lesen, wäre nichts gewonnen: unter der Mutation käme der Test
gar nicht bis dorthin. Sie steht deshalb **davor**, und die Zusicherung, die
unter der Mutation fällt, ist genau die Zusage — `die Quellverbindung bleibt
offen`. Der Zähler `geschlossen_envelope` steht als zweite, unabhängige
Beobachtung daneben: er ist die Zahl, die `verbindung.rs` beim Schließen wegen
eines unbekannten `schema_minor` selbst hochzählt.

**Der Rotbeweis, gemessen.** Mit `P2_SCHEMA_MINOR = 1` fällt der Lauf in
**0,51 s** an `tests_abonnement.rs:298` mit dem Text
`die Quellverbindung bleibt offen: der Broker kennt die Fassung 2 des Senders …`;
ohne die Mutation ist derselbe Lauf in **0,51 s** grün. Kein Hänger, kein
Nebeneffekt. Der Rotlauf hat eine eigene Zeitgrenze von 180 s bekommen — hinge
der Test statt zu fallen, stünde **das** als Befund in der Rohdatei, statt die
Sitzung zu blockieren.

#### 8.11.1 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Nacharbeit 2

| Zeile | Wo gemessen |
|---|---|
| **A. Rückstau und Prioritätsklassen** | Nicht berührt: keine Politik, keine Queue, kein Zähler geändert. Der Zähler `geschlossen_envelope` wird nur **gelesen**. |
| **B. Lebenszyklus** | Genau der Punkt dieser Runde. „Verbinden↔trennen" ist jetzt beidseitig gemessen: am Zielstand bleibt die Quellverbindung offen und der Frame kommt an, unter der Mutation fällt sie innerhalb der Frist. Beides an derselben Zeile, beides begrenzt. |
| **C. Verträge und Längen** | Nicht berührt: kein Vertrag, kein Feld, keine Länge geändert. Die P2-Fassungszahl steht unverändert im Register aus der Nacharbeit 1. |
| **D. Bau- und Prüfriegel** | Der Rotlauf ist fail-closed **auch gegen sich selbst**: ein Hänger wird nach 180 s abgebrochen und als „ABGEBROCHEN — der Test haengt, statt zu fallen. Ein Haenger ist kein Rot." in die Rohdatei geschrieben. Genau die Klasse, die diese Runde überhaupt ausgelöst hat, kann sich damit nicht als Grün tarnen. |
| **E. Behauptung ≤ Messung** | Der Rotbeweis liegt vollständig bei (Mutation eingebaut → rot, Mutation entfernt → grün, beide Läufe im selben Dokument), und die Fallstelle ist die Zusage selbst. Die Frist steht als benannte Konstante mit Begründung, nicht als Zahl im Aufruf. |
| **F. Änderungssatz** | Testhilfe und Zusicherung liegen in **einem** Commit; die Hilfe hat ohne den Test keinen Zweck und der Test ohne die Hilfe keine Frist. |

#### 8.11.2 Die Beine dieser Runde

Nur die Broker-Beine: der Restdefekt liegt vollständig im Rust-Transport und
seinen Testhilfen. Kein Vertrag, kein C++-Ziel und kein Fixture ist berührt —
die übrigen Beine der Etappe messen unverändert den Stand aus §8.9.3.

| Bein | Ergebnis |
|---|---|
| **A4** `cargo test` (Broker, alle Suiten) | GRÜN — Exit 0, keine Suite mit `FAILED`, keine `failures:`-Zeile in der Rohausgabe |
| **A4-SI** `store_crash_matrix --ignored` | GRÜN — Exit 0 |

### 8.12 Wiederprüfung 2 Etappe 2 — PASS; Etappe 2 abgeschlossen, Übergabe (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a089c6-43b8-7803-b8b0-3786ae2a3fef`; Lauf 07:24–07:28 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff c727e620...6f2baba6 -- <Ticketpfade>`; HEAD während des Laufs `6588c290` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-2-wiederpruefung-2-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-2-wiederpruefung-2-6f2baba.txt` |
| Urteil | **PASS** — B-01 geschlossen (`tests_abonnement.rs:272-315` sendet Minor 2 über den echten Transport, Quellverbindung bleibt offen, bytegleiche Zustellung; `tests_hilfe.rs:114-136, 339-364` beobachtet die Quellpipe mit 2000-ms-Frist; Rohdatei `SONDE-015-rot-B-01-n2.txt` zeigt unter Mutation Rot genau an dieser Zusicherung in 0,51 s und Grün nach Rücknahme); nichts gebrochen |
| Rundenbilanz Etappe 2, kumuliert | `2026031f..94a8a336: Produkt 101 +7053/-202 · Tests 5 +856/-47 · Prüfwerkzeug 5 +768/-49 · Doku 35 +1395/-6` · `94a8a336..9306740d: Produkt 12 +256/-17 · Tests 2 +123/-4 · Prüfwerkzeug 2 +114/-4 · Doku 12 +891/-6` · `9306740d..6f2baba6: Produkt 2 +101/-0 · Doku 6 +398/-4` — kein Konvergenz-Signal |
| Etappenstand | **Etappe 2 abgeschlossen** auf `6f2baba6` nach Erstprüfung und zwei Nacharbeitsrunden (Budget 3). Etappe 1 (Matrix) abgeschlossen mit Rest NAK-245 (Etappe 4). |
| Hygiene im Abschlussfenster | `py -3.13 tools/plan/gesundheit.py`: URTEIL alle Grenzen gehalten (Exit 0); `MEMORY.md` 3,4 KB, `CLAUDE.md` 23 657 B, Dirigenten-Skill 35 901 B — alle unter den Schwellen; Planstand neu gerechnet (Quellstand `6588c290`) |
| **Übergabe an eine frische Dirigenten-Session** | Die native Statuszeile misst den Kontext dieser Session mit 47 % des 1M-Fensters (rund 470k); eine ganze Etappe 3 mit Bau und bis zu drei Prüfrunden überschritte die 500k-Grenze (Skill §5) mitten in der Etappe. Der Dirigent beendet deshalb an dieser sauberen Etappengrenze planmäßig (Markerdatei, Neustart durch den Starter). **Nächster Schritt:** frischer Opus-Worker (max, Aufsicht ENG) für **Etappe 3** nach `docs/beweise/roh/SONDE-015-auftrag.txt` (Abschnitt ETAPPE 3; Matrix §3 Referenz, Bauplan §4.3, Entscheide §5, NAK-245 bleibt Etappe 4), Erstprüfung der Etappe über deren Diff (Astra max, Vorlage A wie `docs/beweise/roh/SONDE-015-etappe-2-erstpruefung-auftrag.txt`), danach Etappe 4 (vor deren Bau NAK-245 in §5.11.4 schließen), Abschlussprüfung über `e9dbf4b9...HEAD`, voller Kanon abgekoppelt. Worker dieser Session: `0b83e3c2`, `62c0d8d3`, `1c019f61`, `b7fded1d`, `18e7aa67` (Etappe 1), `a7b07100`, `433bbc2e`, `a429b027` (Etappe 2) — alle beendet und entfernt. |

## 9. Bauetappe 3 — DSP-Kern als Bibliothek mit Goldens (10.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **Etappe 3 — DSP-Kern als Bibliothek mit Goldens**, nach dem Bauplan §4.3. Die Matrix (§3) ist die Referenz; NAK-245 (§7.9, §5.11.4) gehört der Etappe 4 und wurde nicht angefasst. |
| Basis-SHA | `2672ed2bf9858c2c45a8ebbc5afbe0999fff6554`, mit `git rev-parse HEAD` beim Start gemessen; `git status --short` war leer. |
| Urteil | **Offen.** Die Erstprüfung dieser Etappe steht aus. Was hier steht, ist gemessen, nicht beurteilt. |
| Beine | **sechs** gefahren, alle grün; Rohausgabe `docs/beweise/roh/SONDE-015-etappe3-beine.txt` |
| Rotbeweise | **98**, `docs/beweise/roh/SONDE-015-rot-<ID>.txt` (gezählt mit `ls docs/beweise/roh/SONDE-015-rot-*`; davon 59 aus dieser Etappe) |
| Kanon | **Nicht gefahren.** Der volle abgekoppelte Lauf gehört an das Ende der Etappe 4 (Bauplan §4.4); diese Etappe fährt die in §4.3 genannten Beine einzeln. |
| Prozessor | **Unberührt.** `eq-copilot/plugin/sonde/SondeProcessor.{h,cpp}` ist nicht im Diff — A16 misst weiter den heutigen Passthrough und bleibt damit der unabhängige Zeuge aus §4.1. |

### 9.1 Was gebaut wurde

**Das Modul `eq-copilot/plugin/dsp/` (neu, 9 Dateien, 2592 Zeilen).**

- `DspFilter.h` — die Filterprimitiven in `double`: `Biquad` (Direct-Form-II
  transponiert, dieselbe Klammerung wie `core/analysis/KGewichtung.h:39-49`,
  weil Gleitkommaaddition nicht assoziativ ist), `BiquadZustand` getrennt von
  den Koeffizienten, die sechs RBJ-Entwürfe der Vertragstypen, der
  Detektor-Bandpass mit konstanter Spitzenverstärkung, `kappeNyquist`, die
  Hüllkurve mit Attack, Hold und Release als drei getrennten Stufen, die
  dynamische Kennlinie, der Denormal-Riegel und `dbInLinear`.
- `DspSvf.h` — der topology-preserving State-Variable-Filter der dynamischen
  Bänder (Cytomic-Form). Entwurf §44.2 nennt die Arbeitsteilung wörtlich:
  RBJ-Biquads für **statische**, TPT-SVF für **dynamische** Bänder. Bei
  Gain 0 ist er bitgenau neutral (`m0` = 1, `m1` = `m2` = 0).
- `DspRtWache.h` — die Echtzeitwache. Thread-lokales Flag beim Betreten des
  Audiopfads, zwei atomare Zähler. Der Test verdrahtet seinen globalen
  `operator new` mit `RtWache::meldeAllokation`, damit der Zähler des Kerns
  **echte** Allokationen sieht und kein strukturelles Null bleibt.
- `DspProgramm.{h,cpp}` — `BandProgramm` und `DspProgramm`: alles, was der
  Audiothread unveränderlich liest, samt Klemmliste je Slot. `baueProgramm`
  leitet es außerhalb des Audiothreads aus dem DTO ab; `leiteAutoGainAb`
  rechnet den Ausgleich über die 121 Gitterstellen mit dem Diagonalterm der
  M/S-Matrix.
- `DspBankPool.{h,cpp}` — `BandZustand`, `DspBank`, der Ownership-Automat mit
  seinen sechs Zuständen, vier vorallokierte Bänke, der SPSC-ACK-Ring mit acht
  Plätzen, `reclaim_pending_mask` und der Verriegelungszähler. **Jeder
  Übergang ist ein `compare_exchange`** — die Zusage aus M-42 lautet „genau
  die Kette", und ein `store()` könnte jeden Zustand in jeden anderen
  überführen.
- `DspKern.{h,cpp}` — der Audiopfad aus §3.0: Passthrough-Kurzschluss,
  Nicht-Endlich-Riegel, Input-Trim, M/S-Stufe, acht Bänder in Slotreihenfolge,
  Auto-Gain, Mix, Output-Trim, die drei Taps, die Hörmatrix, die Crossfades
  beider Pfade, die fünf globalen Rampen und die fünf Zähler (nicht-endliche Eingänge, geheilte Filterzustände, verworfene Analyseframes, nicht-endliche Auslenkungen, geriegelte Denormale).

**Die Signalkette, wie sie gebaut ist** (§3.0, eingefroren durch den Golden):

```text
Eingang
  -> Tap pre_nakama
     -> eq_enabled == false  ------------------> Ausgang (nichts geschrieben)
     -> v1.global.bypass == true --------------> Ausgang (nichts geschrieben)
     -> Nicht-Endlich-Riegel (nur im aktiven Pfad)
        Input-Trim  (Rampe, Unity-Kurzschluss bei 0 dB)
        M/S-Stufe   (Width, Mono-Bass; Kurzschluss bei width 1 und mono_bass 0)
        8 Band-Slots in Reihenfolge 0 -> 7, je Kanalzuordnung
        Auto-Gain   (abgeleitet, Rampe wie Output-Trim)
        Mix         (linear gegen den Tap pre_nakama)
        Output-Trim (Rampe, Unity-Kurzschluss bei 0 dB)
        -> Tap post_committed
           -> Hoermatrix (Dry / Processed / Delta / Candidate)
              -> Ausgang
```

**Technikkonstanten — je mit Quelle oder als deklarierte Konstante mit Test.**

| Konstante | Wert | Quelle | Wo gemessen |
|---|---|---|---|
| `kFadeSamples` | 256 Samples | §5.2 Feinheit 1 (Technikkonstante, der Entwurf nennt keine Länge) | B6 M-03, M-04, M-06 |
| `kRampeSamples` | 256 Samples | §53.8 „genau eine definierte Rampe" | B6 M-17 |
| `kDeltaMakeupDb` | +12 dB | §5.10 Feinheit 1 (fest heißt materialunabhängig) | B6 M-54 |
| `kKniebreiteDb` | 12 dB | §5.7 Feinheit 1 (Technikkonstante mit Golden) | B6 M-18 |
| `kDynamikSchritt` | 8 Samples | **neu, Entscheid E-6** — die Steuerrate der dynamischen Bänder | B6 M-18, M-26 |
| `kAutoGainStellen` | 121 (20 Hz bis 20 kHz, 1/12 Oktave) | §5.4 Feinheit 1 | B6 M-36, M-37 |
| `kNyquistAnteil` | 0,45 | §44.2 „20 Hz bis min(20 kHz, 0,45 fs)" | B6 M-12 |
| `kBaenke` | 4 | §44.2 „vier vorallokierte Programmbänke" | B6 M-41, M-46 |
| `kAckKapazitaet` | 8 | §5.9 Feinheit 1 (mehr Einträge als Slots) | B6 M-45 |
| `kStilleDb` | −240 dB | **neu, Entscheid E-13** — die Untergrenze des Detektorpegels | B6 M-19 |
| `kDenormalSchwelle` | 1e-300 | **neu, Entscheid E-14** — der Denormal-Riegel am Blockrand | B6 Abschnitt J, Zähler `geriegelteDenormale()` |
| Gitter der Filtergoldens | 1/24 Oktave, 20 Hz bis min(20 kHz, 0,45 fs) | §5.15 Feinheit 3 | B6 Abschnitt C |
| Detektorform | RMS über eine Ein-Pol-Hüllkurve, Koeffizient `exp(-1/(fs*tau))` | §5.7 Feinheit 3 | B6 M-26 |
| Lautheitsabgleich Delta | fest, materialunabhängig | §30.1, §5.10 | B6 M-54 |

### 9.2 Entscheide dieser Etappe (Lücken, die der Bau sonst still gefüllt hätte)

| Nr. | Lücke | Entschieden | Warum |
|---|---|---|---|
| E-1 | Der Auftrag nennt `DspState` als eigenen Typ des Kerns | Es entsteht **kein** neuer Zustandstyp. `nakama::parameter::DspSatz` aus Etappe 2 IST der DspState; der Kern leitet daraus nur Programm-Größen ab. | R1 und §33.5 verbieten eine zweite Wahrheit für Parameter, Grenzen, Defaults und Reihenfolge. Ein eigener Kerntyp müsste bei jeder Vertragsänderung nachgezogen werden und könnte auseinanderlaufen; der Auftrag verlangt ausdrücklich „keine zweite Wahrheit". |
| E-2 | Wo das Modul liegt — eigene Bibliothek oder Teil von `NakamaKern` | In **`NakamaKern`**. Die drei Quellen stehen in `NAKAMA_KERN_QUELLEN`, das Verzeichnis `dsp` ist ein PUBLIC-Include des Kerns, und A14 führt die drei Objekte in seiner Liste. | Der Kern ist identitätsfrei (er kennt nur das DTO, keine `JucePlugin_*`-Konstante) und gehört damit in dieselbe Menge wie State und Vertrag. Gen bekommt in S28b denselben Master-EQ; zwei Apps dürfen nicht zwei Filterbänke bauen. Eine zweite Static-Lib brächte eigene Riegelfragen mit, denn K2, K2b und K2c gelten nur für `NakamaKern`. |
| E-3 | Was die dynamische Auslenkung bei `notch`, `low_cut` und `high_cut` tut | Sie ist dort **exakt 0,0**. Die Auslenkung wirkt nur bei `bell`, `low_shelf` und `high_shelf`. | M-18 sagt „wirkt **zusätzlich zu** `gain_db`". Bei den drei Typen ohne Gain lässt der Vertrag `gain_db` zwar stehen, es wirkt aber nicht — und was nicht wirkt, kann auch nicht dynamisch wirken. Jede andere Deutung müsste einen Angriffspunkt erfinden, den der Vertrag nicht hat. |
| E-4 | Wo die M/S-Matrix der Signalkette liegt, wenn ein Band `left` und ein anderes `mid` ist | Die Stufe zwischen Input-Trim und Bändern ist **Width und Mono-Bass**; die Kanalzuordnung geschieht **je Band lokal**. | `left` und `mid` können im selben Programm vorkommen. Eine globale Hinmatrix vor allen Bändern müsste `left` im M/S-Bereich rekonstruieren; eine globale L/R-Kette müsste dasselbe für `mid`. Die lokale Zuordnung ist die einzige Lesart, in der beide Modi gleichzeitig definiert sind, und sie hält die Kaskadenreihenfolge aus M-15 unverändert. |
| E-5 | Was der Detektor bei `channel_mode` = `stereo` hört | Die **mittlere Leistung beider Komponenten**, und **eine** Auslenkung wirkt auf beide. | Zwei getrennte Auslenkungen zögen das Stereobild auseinander, sobald sich L und R im Pegel trennen — der Kanalmodus `stereo` sagt gerade, dass das Band beide Seiten gleich behandelt. |
| E-6 | Mit welcher Rate Detektorpegel, Kennlinie und SVF-Koeffizienten neu gerechnet werden | `kDynamikSchritt` = **8 Samples**; der Detektor selbst läuft mit voller Audiorate, dazwischen wird der Koeffizientensatz linear überblendet. | `log10` und `pow` je Sample und Band kosteten bei acht Bändern ein Vielfaches der Filterarbeit. 8 Samples sind 0,167 ms bei 48 kHz und damit feiner als die kürzeste Attack (0,1 ms); die lineare Überblendung dazwischen hält den Gain sample-genau stetig statt in Stufen zu springen. |
| E-7 | Wie kontinuierliche Werte wirken, ohne den Bankpool leerlaufen zu lassen | Jede DTO-Änderung erzeugt eine **neue Bank mit Crossfade**; die fünf globalen Gains (Input-Trim, Width, Auto-Gain, Mix, Output-Trim) laufen **zusätzlich** über Rampen im Pfad, die beide Bänke eines Fades gemeinsam lesen. | R8 verlangt beides: topologische Wechsel mit Crossfade, kontinuierliche Werte über Rampen. Lägen die Gains in der Bank, sähen die zwei Durchläufe eines Crossfades zwei verschiedene Werte — die globale Stufe ist EINE Stufe. Der Rampenstand wird deshalb zwischen den Durchläufen gesichert und zurückgesetzt, billiger und exakter als fünf Vorabpuffer über die Blockgröße. |
| E-7 (Berichtigung) | wie oben | **berichtigt in Nacharbeit 1:** Kontinuierliche Bandwerte (`freq_hz`, `q`, `gain_db` und die übrigen `rampe`-Werte eines Bandes) erzeugen **keinen** Crossfade mehr, sondern einen Rampenübergang mit Zustandsübernahme (E-19); nur topologische Wechsel blenden über zwei vollständige Programme. Die fünf globalen Rampen gehören seit E-16 dem **Pfad**, nicht dem Kern. | R8 verlangt „kontinuierliche Werte über Rampen" — ein Programmcrossfade mit kalter Bank setzte bei laufender Automation Filter- und Hüllkurvenzustände zurück (Erstprüfung B-4). |
| E-8 | Ob eine neue Bank den Filterzustand der alten erbt | **Nein** — sie startet kalt, alle Zustände auf 0. | M-07 verlangt genau das („beim erneuten Einschalten starten alle Filter- und Hüllkurvenzustände auf 0"). Der Crossfade über 256 Samples deckt die Transiente ab; ein übertragener Zustand wäre ein geteilter Zustand zwischen zwei Bänken, und §44.2 verbietet das ausdrücklich. |
| E-9 | Was mit einer verblassenden **Candidate**-Bank geschieht | Sie dient **sofort** aus; der Candidate blendet vom unveränderten Eingang her ein. | §44.2 nennt vier Bänke als schlechtesten Fall — zwei je Pfad. Eine zusätzlich gerechnete verblassende Candidate-Bank kostete eine fünfte. Der Candidate wird ohnehin nie Baseline. |
| E-9 (Berichtigung) | wie oben | **berichtigt in Nacharbeit 1:** Die verblassende Candidate-Bank dient **nicht** sofort aus. Ein Candidate-Wechsel blendet von der bisherigen Candidate-Bank auf die neue; nur der erste Candidate ohne Vorgänger blendet vom Eingang ein (B-7). Zwei Bänke je Pfad reichen — **vier** insgesamt, nicht fünf: ein weiterer Wechsel wartet das Ende des laufenden Übergangs ab (E-17), und ohne diese Regel fehlte nicht eine fünfte Bank, sondern nur ein Pufferpaar. | Die Begründung „kostete eine fünfte Bank" war falsch (Erstprüfung B-7, Quellencheck §9.8). §44.2 verlangt Double-Buffer, die „innerhalb ihres Pfads klickfrei die Topologie wechseln können". |
| E-10 | In welcher Auflösung die drei Taps liegen | **`double`**, nicht `float`. | Der Kern rechnet in `double` (§44.2), und ein Tap ist eine Messung. Die Konvertierung für die bestehende Analyse gehört in Etappe 4, an den Rand — nicht in die Kette. Zugleich misst der Filtergolden damit ohne float-Quantisierung. |
| E-11 | Was bei einem Block größer als `maxBlock` geschieht | Er läuft **in Stücken** durch; verworfen wird nur der Tap dieses Blocks, und der Zähler steigt. | „Überlast verwirft Analyseframes, nie Audio" (M-48). Ein Rückweg hier hieße, dass ein Host mit größerem Puffer als angekündigt Stille bekäme — ein Audioausfall. Der Fund kam aus dem Rotbeweis zu M-48 (§9.7 N-2). |
| E-12 | Welche Antwort der Auto-Gain für ein **dynamisches** Band rechnet | Die **SVF-Ruheantwort** dieses Bandes, analytisch geschlossen — nicht die RBJ-Antwort einer Familie, die dieses Band gar nicht benutzt. | §5.4 sagt „die statische Kurve des bestätigten Programms". Die statische Kurve eines dynamischen Bandes ist sein Gang bei Auslenkung 0, und der läuft über den SVF. Dafür tragen die SVF-Koeffizienten `g` und `k` mit; sie aus `a1..a3` zurückzurechnen wäre bei `k` = 0 nicht eindeutig. |
| E-14 | Was mit **denormalen** Filterzuständen geschieht (der Selbstaudit des Auftrags nennt sie ausdrücklich) | Ein Zustandswert unter `kDenormalSchwelle` = 1e-300 wird **am Blockrand** genullt, an derselben Stelle wie die Endlichkeitsprüfung, und der Treffer wird gezählt. | Ein abklingender Filterzustand läuft nach genügend Stille in den denormalen Bereich; auf x86 kostet jede Rechnung damit ein Vielfaches, und ein Kern, der im **Leerlauf** langsamer wird als unter Last, verletzt die Echtzeitfestigkeit aus §44.5. Der Riegel am Blockrand braucht keine Intrinsics und keine Plattformannahme — anders als FTZ/DAZ, die eine Entscheidung der Zielschicht sind und dem Prozessor in Etappe 4 offenstehen. 1e-300 liegt unter −6000 dBFS und trägt hörbar nichts. |
| E-13 | Was der Detektorpegel bei Stille meldet | `kStilleDb` = **−240 dB**, endlich. | Ein `-inf` liefe durch die Kennlinie und machte den Nicht-Endlich-Riegel zum Dauergast. −240 dB liegt weit unter jedem Vertragsthreshold (Minimum −60 dB) und ist damit klanglich dasselbe wie Stille. |
| E-15 | Wie Übernahme und ACK die Generation tragen, ohne dass zwischen Lesen und Übergang ein Fenster bleibt (B-1, B-12) | Zustand und Generation liegen in **einem** atomaren 64-Bit-Wort je Slot (3 Bit Zustand, 61 Bit Generation). Publikations- und ACK-Wort tragen Generation und `slot + 1` (0 = leer). Die Übernahme ist **ein** CAS `bereit(g) -> audioAktiv(g)` mit der beobachteten Generation; die Ernte ist ein CAS `ausgedient(g) -> frei` nur für die gemeldete Generation. Der Generationszähler liegt außerhalb von `zuruecksetzen` und wird nie zurückgesetzt. | Ein zweites Atomic neben dem Zustand ließe genau das Fenster aus B-1 offen: Prüfung und Übergang wären zwei Schritte. In einem Wort ist die Generationsprüfung Teil des Übergangs selbst. R9 Feinheit 2 verlangt einen nie zurückgesetzten Zähler — ein Lebenszeitzähler ist kein Ressourcenzustand. |
| E-16 | Welche Zustände einem **Pfad** gehören (B-6, B-7, B-9) | Je Pfad ein vollständiger `PfadZustand`: aktive Bank, Quellbank des Übergangs, Übergangsart und -rest, **eigene** fünf Rampen und **eigene** acht Auslenkungen. Getter `auslenkungenDb` (Committed, Quelle für `band_dynamic_gain_db`, kohärent zu `post_committed`), `auslenkungenCandidateDb`, `autoGainDb` und `autoGainCandidateDb`. | §3.0: der Candidate-Pfad ist „derselbe Aufbau auf einer zweiten Bank". Geteilte Rampen oder Messwerte machten den zweiten Pfad zu einem Nebenweg des ersten. |
| E-17 | Was ein Wechsel tut, der während eines laufenden Übergangs eintrifft (B-5) | Der laufende Übergang wird **zu Ende geführt**. Der neue Wechsel bleibt publiziert — der Worker darf ihn durch einen neueren verdrängen — und wird am ersten Blockrand **nach** dem Übergangsende genommen. | So beginnt jeder Übergang beim zuletzt ausgegebenen Signal, ohne eine dritte Bank je Pfad; die Klickfreiheit folgt aus der Struktur statt aus einer Mischformel. Kosten: ein Wechsel wartet höchstens `kFadeSamples` (5,8 ms bei 44,1 kHz). Die Alternative „neuer Fade ab dem Mischsignal" bräuchte je Pfad drei rechnende Bänke. |
| E-18 | Wie Ausschalten und Candidate-Ende über den regulären Weg laufen, ohne eine Bank zu reservieren (B-8, B-10) | Beide publizieren eine **ENDE-Marke** (Slotfeld `kBaenke`, eigene Generation). Der Blockrand nimmt sie wie ein Programm, blendet die aktive Bank über `kFadeSamples` in die Ruhe und meldet sie danach mit ACK aus. Der abgeleitete Auto-Gain eines ausgeschalteten Zustands entsteht im Bauplatz des Workers (`arbeitsProgramm`), ohne Bank. | M-07: „keine Bank ist `audio_active`". Eine eigene Aus-Bank wäre genau die reservierte Bank aus B-10; ein nur gelöschtes Flag genau der hängende Candidate aus B-8. |
| E-19 | Wie kontinuierliche Bandwerte rampen, obwohl der Worker nur freie Bänke beschreiben darf (B-4) | Unterscheiden sich altes und neues Programm nur in `rampe`-Werten (`rampenKompatibel`), nimmt der Audiothread am Blockrand den Filter- und Hüllkurvenzustand der alten Bank in die neue und interpoliert die Koeffizienten über `kRampeSamples` linear: fünf Biquad-Koeffizienten, beim SVF `grundG`, `q`, `gain_db`, Threshold und Range je Steuerschritt, dazu Detektor-Biquad und Mono-Bass-Hochpass. Das erste Sample trägt 1/256, das letzte 1. Die alte Bank bleibt bis Rampenende Koeffizientenquelle und dient dann mit ACK aus. | Die Menge stabiler Nennerpaare (Betrag von `a2` unter 1, Betrag von `a1` unter `1 + a2`) ist ein Dreieck und damit konvex: jeder Zwischenpunkt zweier stabiler Entwürfe ist stabil. Eine Neuberechnung je Sample bräuchte `sin`, `cos` und `pow` im Callback. |
| E-19 (Berichtigung) | wie oben | **berichtigt in Nacharbeit 2:** Zur Interpolation gehören auch die Hüllkurvenparameter `attack_ms`, `hold_ms` und `release_ms` (Vertrag `wechsel = rampe`): beide Pole linear je Sample, die Haltezeit als gerundete lineare Mischung der Samplezahl (E-28). Detektor-Bandpass und Hüllkurve sind entworfen, sobald das Band einen Detektor hat, auch bei Range 0; im Rampenübergang läuft der Detektor, solange eines der beiden Programme ihn laufen lässt (E-29). | Die Aufzählung „`grundG`, `q`, `gain_db`, Threshold und Range" ließ die drei Hüllkurvenwerte aus, und der Range-Übergang auf 0 lief am Flag des Zielprogramms vorbei (Wiederprüfung 1, W-2 und W-3, §9.10). |
| E-20 | Ob das Ein- und Ausschalten der Mono-Bass-Stufe rampt | `mono_bass_hz` 0 ↔ > 0 ist ein **Crossfade**; Änderungen innerhalb > 0 rampen (E-19). | Bei 0 existiert die Stufe nicht (M-30); eine Rampe müsste einen Hochpass aus „nicht vorhanden" interpolieren. |
| E-21 | Was der Candidate-Pfad tut, wenn Committed ausgeschaltet oder überbrückt ist | Er rechnet weiter und füllt `post_candidate`; geschrieben wird der Ausgang nicht (B-3). | Ein Vergleich ist ausdrücklich gestartet; das Grundgesetz schützt den Ausgang, nicht den Messpunkt eines laufenden Vergleichs. Der Bedienvertrag des Candidate bleibt S29–31. |
| E-22 | Wann der zweite Riegel heilt (B-20) | Am Blockrand **vor** dem ersten Sample, für beide Bänke beider Pfade. | Ein zwischen zwei Blöcken entgleister Zustand rechnet dann keinen Sample mehr; am Blockende geheilt, trüge der Block ihn schon im Ausgang. |
| E-23 | Was `bestaetigeReclaim` an einem Slot ohne Maskenbit tut (B-17) | Nichts — die Bestätigung wirkt nur auf einen verriegelten Slot und löscht dann das Bit. | Ohne Bit liegt der ACK im Ring; eine Bestätigung daran vorbei wäre ein zweiter Freigabeweg. Damit hängt die Freigabe nach einem Überlauf an der Maske und nicht an einem Zähler. |
| E-24 | Welche Testzugänge der Kern trägt | `publikation`/`uebernehme` getrennt (B-1), `ackEinreihen` öffentlich (B-12), `ernteAcks (hoechstens)` (B-12), Ringkapazität als Konstruktorparameter bis `kAckKapazitaet` (B-17), `setzeTeilstueckHaken` (B-11), `msStufenLaeufe` und `uebernahmen` (B-19, B-11). Keiner ändert das Produktverhalten; `uebernehmeBereiten` ist weiter der eine Weg des Audiothreads. | Die Befunde verlangen deterministische Interleavings und erzwungene Überläufe; ein Stresslauf allein trifft sie nicht reproduzierbar. |
| E-25 | Wie B6 Knie, Reihenfolge und Hüllkurvenzeiten misst (B-14, B-15, B-16) | Ein **Stereoton** auf der Bandmitte (L sin, R cos) hält die Detektorleistung nach dem Bandpass konstant `a²/2`. Knie und Reihenfolge laufen mit Attack und Release 500 ms. Die Sprungantwort misst Attack bis `1 − 1/e` der Zielleistung, Hold bis zum Verlassen des Plateaus um 0,01 dB, Release bis `1/e`; Toleranz **1 ms** je Stufe. | Ein einkanaliger Sinus hätte eine Detektorwelligkeit, die jede Zeitmessung verschmiert. 1 ms deckt die Gruppenlaufzeit des Detektor-Bandpasses (0,23 ms bei 1 kHz, Q 0,707) und die Steuerrate (höchstens 0,18 ms); eine auf 48 kHz festgeschriebene Umrechnung verfehlt 44,1 kHz um 1,77 ms und 96 kHz um 20 ms. |
| E-26 | Was eine **Transportkante** für die Bibliothek ist (B-25) | Eine Aufrufpause, in der der Worker publiziert oder erntet, die Kante Signal ↔ Stille und der Wechsel der Blockgröße zwischen 1, Blöcken bis `maxBlock` und einem übergroßen, stückelnden Block. `bereiteVor` und `freigeben` sind keine Kante — sie dürfen allozieren (M-41 „nach `prepareToPlay`"). | Die Bibliothek kennt keinen Host-Transport; Play/Stop und Positionssprünge des Hosts sind Etappe 4 (Prozessor). Was die Bibliothek davon sieht, ist genau diese Aufrufform. |
| E-27 | Wie das Golden an einer Nullstelle misst (B-24) | Liegt die Referenz unter −60 dB — die exakte Mitte eines Notch —, gilt: gemessen ebenfalls unter −60 dB. | Die Referenz liegt dort bei −300 dB, die gemessene Impulsantwort an ihrem numerischen Boden; ein dB-Abstand hat an einer Nullstelle keinen Maßstab. Über −60 dB gelten die Toleranzen aus R15 unverändert. |
| E-28 | Wie die Hüllkurvenkoeffizienten rampen und woran das gemessen wird (W-2) | Im Rampenübergang mischt der Audiothread je Sample `attackPol` und `releasePol` linear mit dem Rampenfortschritt t (erstes Sample 1/256, ab dem 256. das Ziel) und `holdSamples` als gerundete lineare Mischung; der Hüllkurvenzustand wandert mit (E-19). B6 misst Sample für Sample gegen eine im Test ausgeschriebene Idealrampe: die Hüllkurvenleistung (Residuum unter 1e-6 der Detektorleistung), die daraus gemeldete Auslenkung an jedem Steuerschritt (unter 1e-4 dB) und bei `hold_ms` den Holdzähler (höchstens 1 Sample). | Die Pole sind die einzigen Koeffizienten der Hüllkurve, und die Auslenkung ist eine feste Funktion ihrer Leistung (Kennlinie, M-18): ein Residuum der Leistung von 1e-6 der Detektorleistung Pss bindet die Auslenkung im Knie auf höchstens Range/12 · 4,34 dB · 1e-6 · Pss/L, bei Range 6 dB und L ab 0,05 Pss also unter 5e-5 dB — die Schranke des Falls ist 1e-4 dB. Die zweite Differenz der Auslenkung sähe den Hold nie — er verschiebt nur den Releasebeginn —, der Holdzähler zeigt ihn dort, wo der Koeffizient wirkt; seine Toleranz ist die Rundung der gemischten Samplezahl. Linear in den Polen statt in Millisekunden: jeder Zwischenpol liegt zwischen zwei stabilen Polen, und der Callback braucht kein `exp`. |
| E-29 | Wie der Übergang `dynamic_range_db` auf 0 und von 0 weg rampt (W-3) | `baueProgramm` entwirft Detektor-Bandpass und Hüllkurve, sobald das Band einen Detektor hat (`nutztSvf` und Sidechain nicht `none`), auch bei Range 0; `detektorLaeuft` bleibt an Range ungleich 0 gebunden (M-20). Im Rampenübergang läuft der Detektor, wenn Quell- oder Zielprogramm ihn laufen lässt, und die Kennlinie liest die interpolierte Range; das Flag des Zielprogramms gilt ab dem Rampenende. Ein Detektor, der am Ende eines Fensters nicht läuft, wird genullt. | Ohne entworfene Koeffizienten auf der Range-0-Seite gäbe es nichts zu interpolieren. Das Nullen macht „danach ist der Detektor aus" zu einem Zustand statt zu einem eingefrorenen Pegel: eine spätere Rampe von 0 weg beginnt wie ein frischer Detektor, nicht bei einem Pegel von damals. Gemessen im Plateau (Pegel weit über Threshold plus Knie), wo die Auslenkung die Range IST: die Idealrampe ist dort exakt; Residuum unter 1e-6 dB, als Gain unter −138 dBFS bei Vollaussteuerung. |
| E-30 | Was die Hörmatrix bei einem Wechsel während ihres laufenden Fades tut (W-5) | Zurück zur Quelle des Fades (etwa ein Candidate-Abbruch im Einblenden): Richtung umkehren — Quelle und Ziel tauschen, Restzähler `kFadeSamples − Rest − 1`; das Gewicht beider Seiten bleibt am Umschaltsample stehen und läuft von dort in Fadeschritten zurück. Zu einem dritten Zustand: der laufende Fade endet zuerst, der Wechsel beginnt am ersten Stückbeginn danach. `wirksameHoermatrix` meldet weiter die Auswahl, auf die die Hörmatrix zuläuft. | Ein Mischstand aus zwei Seiten lässt sich ohne dritten Puffer nur zu einer seiner Seiten stetig fortsetzen — dieselbe Struktur wie E-17 für die Bänke. Der alte Zweig ließ bei einem Rückwechsel beide Seiten des Fades auf dasselbe Ziel fallen und sprang (Wiederprüfung 1, W-5). |
| E-31 | Woran B-5 und B-7 den Sprung messen (W-4) — **Dirigenten-Entscheid, Quelle §9.10** | Am Umschaltsample: die Nachbarsample-Differenz am ersten Sample des neuen Fades ist höchstens die Fadeschrittweite plus die Rundungstoleranz 2^−23 = 1,19e-7. Die Maximumsprüfung über den ganzen Lauf bleibt als Wache mit einer Schranke, die B6 aus dem Einschwingen der kalt startenden Zielbank herleitet (`kaltSchranke`: RBJ-Referenz ab Zustand 0, je Sample der Betrag von (y(n−1) − Quelle)/K plus t mal dem Betrag von y(n) − y(n−1), Maximum über den Lauf). | Rundungstoleranz: jeder Ausgang unter 1,0 liegt auf einem float-Raster von höchstens 2^−24, die Differenz zweier gerundeter Nachbarn verschiebt sich um höchstens zwei Rasterschritte; die Fadeschrittweiten der Proben liegen um 2e-3. Die kalte Zielbank schwingt während des Fades ein und hebt das Maximum über den Fadeschritt (0,0025299 gegen 0,0021309, §9.10) — ein Faktor ohne Herleitung ließe beliebige Sprünge durch. |
| E-32 | Wo der Schreibzugriff bei einem Fade in den Passthrough endet (W-1) | Am ersten Sample, dessen Crossfadegewicht 1 ist — auch mitten im Teilstück. Die Ausgabeschleife schreibt nur bis dorthin; danach ruht die Hörmatrix wie in einem Stück mit `committedRuht`. | M-05 und REGEL B-3: nach dem Fade wird kein Sample geschrieben. Die Ruheprüfung am Stückbeginn allein schrieb den Rest des Stücks als float(double(x)) zurück (Wiederprüfung 1, W-1). |
| E-33 | Was beim Candidate-Ende geschieht, solange die Hörmatrix noch Candidate-Anteile mischt, und was „auf Candidate" in REGEL X-1 heißt (X-1) | Die ENDE-Marke setzt den Candidate-Pfad in den Übergang `hoerHalt`: die endende Bank liegt in `quelle` und rechnet unverändert mit ihren Rampen und Auslenkungen weiter, ohne Mischgewicht und ohne Rest. „Mischt Candidate-Anteile" heißt: das Ziel der Hörmatrix ist Candidate, oder ihr laufender Fade kommt noch von Candidate — auch im Ausblenden. Sobald der Hörmatrix-Zustand eines Stücks feststeht und keine Candidate-Anteile mehr enthält, meldet `beendeHoerHalt` die Bank mit `meldeAusgedient` aus (ACK, danach Reclaim durch den Worker). Mischt die Hörmatrix beim Nehmen der Marke keinen Candidate, bleibt der Crossfade in die Ruhe. | Die Hörmatrix liest den Candidate-Puffer; blendet die Bank zugleich nach Dry, liegen zwei Fades übereinander, und identische Zustände nullen nicht (M-55). Der erste Satz der Regel („solange die Hörmatrix noch Candidate-Anteile mischt") deckt einen dritten Abbruchzeitpunkt mit, das Ausblenden zurück auf Processed. Der Halt ist begrenzt: `aktiv` ist währenddessen −1, die Auswahl Candidate damit nicht wirksam, und die Hörmatrix läuft in höchstens zwei Fadelängen auf einen anderen Zustand zu. Das Ende ohne eigene Blende ist unhörbar, weil keine Auswahl die Bank dann noch liest; eine neue Publikation wartet wie in E-17 bis nach dem Halt. |
| E-34 | Was die Hörmatrix tut, wenn ein Candidate aus der Ruhe einblendet, während die Auswahl schon auf Candidate steht — das Gegenstück zu X-1 beim Einblenden | Ein Candidate-Pfad, der aus der Ruhe einblendet (Crossfade ohne Quellbank), gilt für die Hörmatrix noch nicht als vorhanden: `wirksameHoermatrix` meldet bis zum Ende dieses Einblendens Processed (M-56), danach blendet die Hörmatrix als einzige Blende auf Candidate. Ein Candidate-Wechsel mit Quellbank (B-7) bleibt unberührt. | Sonst laufen der Hörmatrix-Fade und das Einblenden von Dry her übereinander, und identische Zustände nullen auch hier nicht (M-55; Messwert in §9.13). `CLAUDE.md` („Beziehungen mitprüfen", aktivieren und abklingen) und `AGENTS.md:48-50` verlangen die gekoppelte Hälfte im selben Änderungssatz. Kosten: beim ersten Candidate wirkt die Auswahl um das Einblenden des Pfades später — `kFadeSamples` plus den Rest des Stücks, in dem es endet, weil die wirksame Auswahl je Stückbeginn gilt. |

### 9.3 Messabdeckung

Eine Tabelle über **alle** Matrixzeilen der Abschnitte §3.1 bis §3.6 und
§3.13 — auch die, die diese Etappe nicht misst. Ein Bauer meldet nie „fertig"
mit stillschweigend ungemessenen Zeilen.

| ID | Testfunktion in B6 (sofern diese Etappe sie misst) | Rotbeweis | Status |
|---|---|---|---|
| M-01 | `default_ist_bitidentisch bei 44100 / 48000 / 96000 / 192000 Hz` (250 Blöcke wechselnder Größe je Rate) plus `ausgeschaltet ist keine Bank audio_active` | `SONDE-015-rot-M-01.txt` | **gemessen** — A16 misst dieselbe Zusage in Etappe 4 am Prozessor |
| M-02 | `unity_kurzschluss_statt_multiplikation` — über eine Rampe, die auf 1,0 zuläuft | `SONDE-015-rot-M-02.txt` | **gemessen** |
| M-03 | `engagieren_ist_klickfrei_und_endet_deterministisch`, `fade_ist_in_samples_gezaehlt_nicht_in_bloecken` | `SONDE-015-rot-M-03.txt` | **gemessen** |
| M-04 | `nach_dem_fade_wieder_bitidentisch` — Blockgröße 300, der Fade endet mitten im Block | `SONDE-015-rot-M-04.txt` | **gemessen** |
| M-05 | `hard_bypass_schreibt_nicht` — mit Width 1,7 und einem +12-dB-Bell am Eingang | `SONDE-015-rot-M-05.txt` | **gemessen** |
| M-06 | `bypasswechsel_ist_klickfrei`, `bypasswechsel_endet_bitgleich` | `SONDE-015-rot-M-06.txt` | **gemessen** |
| M-07 | `ausgeschaltet_rechnet_nichts_und_startet_kalt` | `SONDE-015-rot-M-07.txt` | **gemessen** |
| M-08 | — | — | **Etappe 4**: der Schalter überlebt Speichern und Laden (B2) und die Bank wird beim ersten Block aktiv (B7). Beides braucht den Prozessor. |
| M-09 | — | — | **Etappe 4**: Smart Disable und der wartende bestätigte Zustand liegen in der Transaktionsschicht (B7). |
| M-10 | `bell_gegen_analytische_antwort` bei 44,1 / 48 / 96 / 192 kHz, 240 Gitterstellen | `SONDE-015-rot-M-10.txt` | **gemessen** |
| M-11 | `low_shelf_`, `high_shelf_`, `notch_`, `low_cut_`, `high_cut_gegen_analytische_antwort`, je vier Sampleraten | `SONDE-015-rot-M-11a.txt` bis `-M-11e.txt` | **gemessen** — fünf getrennte Rotbeweise, weil M-11 jeden Typ einzeln verlangt (§9.6 A-3) |
| M-12 | `nyquist_kappung_bei_045_fs`, `kappung_aendert_den_persistenten_wert_nicht`, `bei_96_khz_greift_der_deckel_nicht` | `SONDE-015-rot-M-12.txt` | **gemessen** |
| M-13 | Der Aufbau des Goldens selbst plus `impulsantwort_misst_wie_ein_eingeschwungener_sinus` | `SONDE-015-rot-M-13.txt` | **gemessen** — die Mutation lässt den Audiopfad die Bank nicht anwenden; ein Golden gegen die eigene Formel bliebe dabei grün |
| M-14 | — | — | **Etappe 2 und 4**: die Prüfstufenfolge des DTO liegt in `NakamaParameter::validiere` (A12, B2); die Abweisung eines Apply misst B7. |
| M-15 | `kaskade_gegen_produkt_der_einzelantworten`, `richtung_null_nach_sieben_gegen_geordnetes_golden`, `ein_rueckwaertslauf_wuerde_das_golden_reissen` | `SONDE-015-rot-M-15.txt` | **gemessen** |
| M-16 | `koeffizienten_und_zustaende_sind_double` | `SONDE-015-rot-M-16.txt` | **gemessen** — die Double-Capability wird nicht gemeldet (A13 unverändert) |
| M-17 | `zipperresiduen_unter_minus_100_dbfs`, `die_rampe_folgt_der_ausgeschriebenen_ideallinie`, `rampe_endet_bitgenau_auf_dem_ziel` | `SONDE-015-rot-M-17.txt` | **gemessen** |
| M-18 | `dynamische_kennlinie_golden_plateau`, `auslenkung_bleibt_innerhalb_range`, `kennlinie_im_knie_ist_linear` | `SONDE-015-rot-M-18.txt` | **gemessen** |
| M-19 | `unter_threshold_steht_das_band_exakt_still` | `SONDE-015-rot-M-19.txt` | **gemessen** |
| M-20 | `range_null_rechnet_keinen_detektor`, `range_null_ergibt_exakt_null_auslenkung` | `SONDE-015-rot-M-20.txt` | **gemessen** |
| M-21 | `detektor_ist_bandbegrenzt`, `detektor_hoert_vor_dem_band` | `SONDE-015-rot-M-21.txt` | **gemessen** |
| M-22 | `sidechain_none_liefert_keine_auslenkung` | `SONDE-015-rot-M-22.txt` | **gemessen** |
| M-23 | `priority_sidechain_wird_geklemmt_und_gemeldet`, `der persistente Wert bleibt priority_sidechain` | `SONDE-015-rot-M-23.txt` | **gemessen** — die Klemmliste im Bericht ist Etappe 4 (B7) |
| M-24 | `ausschalten_aendert_nur_dynamic_enabled` (die fünf Werte bitgleich am DTO) | — | **teilweise**: der Kern zeigt, dass er die fünf Werte unverändert liest. Dass sie Speichern und Laden bitgleich überleben, misst **B2** (Etappe 2, bereits grün); die Transaktion misst B7 (Etappe 4). Eine eigene Trägerzeile im Kern gibt es nicht. |
| M-25 | `dynamic_enabled_wechselt_nur_am_blockrand` (zweite Differenz gegen den ruhigen Lauf) | `SONDE-015-rot-M-25.txt` | **gemessen** |
| M-26 | `attack_hold_release_sind_drei_stufen` | `SONDE-015-rot-M-26.txt` | **gemessen** |
| M-27 | `auslenkung_ist_gemessen_nicht_gerechnet` | `SONDE-015-rot-M-27.txt` | **gemessen** |
| M-28 | — | — | **Etappe 4**: dass kein Slot den Modus eines anderen erbt, ist eine Zusage an den Zustand, nicht an den Audiopfad (B7); die Enumgrenze misst A12. |
| M-29 | `channel_mode_wechselt_am_blockrand_klickfrei` | `SONDE-015-rot-M-29.txt` | **gemessen** |
| M-30 | `width_eins_ist_bitidentisch`, `width_null_ist_mono` | `SONDE-015-rot-M-30.txt` | **gemessen** — `mono_bass_hz` = 0 schaltet die Stufe mit ab (dieselbe Bedingung) |
| M-31 | `trims_auf_null_db_sind_kurzschluss` (in M-02 mitgemessen), `input_trim_wirkt_und_liegt_vor_der_bank` | `SONDE-015-rot-M-31.txt` | **gemessen** |
| M-32 | `mix_eins_umgeht_den_dry_zweig` | `SONDE-015-rot-M-32.txt` | **gemessen** |
| M-33 | `mix_null_ist_bitidentisch` — trotz +9 dB Input-Trim und +12-dB-Bell | `SONDE-015-rot-M-33.txt` | **gemessen** |
| M-34 | `mix_ist_linear_nicht_equal_power` | `SONDE-015-rot-M-34.txt` | **gemessen** |
| M-35 | `auto_gain_aus_wirkt_nicht_und_ist_trotzdem_lesbar`, `der_ausgleich_wirkt_bei_aus_nicht` | `SONDE-015-rot-M-35.txt` | **gemessen** |
| M-36 | `flache_kurve_ergibt_exakt_null_db`, `neutraler_bell_ergibt_exakt_null_db` — bitgenau +0,0, nicht −0,0 | `SONDE-015-rot-M-36.txt` | **gemessen** |
| M-37 | `shelf_plus_sechs_ergibt_minus_sechs`, `identisches_mid_side_paar_faellt_auf_stereo_zusammen`, `und es wird NICHT wie der doppelte Gain kompensiert` | `SONDE-015-rot-M-37.txt` | **gemessen** — Prüflingswahl in §9.6 A-4 |
| M-38 | `dynamik_bewegt_den_auto_gain_nicht`, `und die Dynamik hat sich wirklich bewegt` | `SONDE-015-rot-M-38.txt` | **gemessen** |
| M-39 | `auto_gain_wird_nicht_im_audiothread_gerechnet` | `SONDE-015-rot-M-39.txt` | **gemessen** — dass er nicht im `state_hash` steht, misst B7 (Etappe 4) |
| M-40 | `auto_gain_und_output_trim_addieren_sich` | `SONDE-015-rot-M-40.txt` | **gemessen** |
| M-41 | `vier_baenke_vorallokiert`, `alle vier Baenke reservierbar`, `null_allokationen_im_callback` | `SONDE-015-rot-M-41.txt` | **gemessen** |
| M-42 | Sechs Fälle, je ein Übergang, plus `ein_nicht_genannter_uebergang_scheitert` | `SONDE-015-rot-M-42.txt` | **gemessen** |
| M-43 | `reclaim_erst_nach_ack`, `nach dem ACK wird sie frei` | `SONDE-015-rot-M-43.txt` | **gemessen** |
| M-44 | `busy_retry_wenn_kein_slot_frei`, `wiederholung_nach_bankfreigabe_gelingt`, `und der fuenfte Wunsch bekommt busy_retry` | `SONDE-015-rot-M-44.txt` | **gemessen** — dass die Wiederholung nicht memoisiert wird (§5.11.4 T5), misst B7 |
| M-45 | `reclaim_pending_mask_haelt_den_slot`, `der Ring droppt bei regulaerem Betrieb nie`, `kein stiller Ringverlust unter Last` | `SONDE-015-rot-M-45.txt` | **gemessen als Wache** — der Ring fasst mehr Einträge als es Slots gibt und kann strukturell nicht überlaufen; gemessen wird die Kopplung Überläufe gleich Verriegelungen, und der Rotbeweis senkt dafür die Ringgröße unter die Slotzahl |
| M-46 | `vier_baenke_im_schlimmsten_fall`, `candidate_endet_neutral` | `SONDE-015-rot-M-46.txt` | **gemessen** — die Vier-Bank-Hälfte teilt ihre Trägerzeile mit M-41 (`kBaenke`); eigenständig ist das neutrale Ende des Candidate |
| M-47 | `null_allokationen_im_callback`, `null_sperren_im_callback`, `der_zaehler_trennt_worker_und_audiothread` (4000 Blöcke wechselnder Größe) | `SONDE-015-rot-M-47.txt` | **gemessen** |
| M-48 | `ueberlast_verwirft_analyse`, `und Audio laeuft unveraendert weiter` | `SONDE-015-rot-M-48.txt` | **gemessen** — der Rotbeweis deckte einen echten Defekt auf (§9.7 N-2) |
| M-49 | `nichtendlicher_eingang_wird_verriegelt_und_gezaehlt`, `kein_nan_vergiftet_den_filterzustand`, `der_pfad_bleibt_dauerhaft_endlich` | `SONDE-015-rot-M-49.txt` | **gemessen** |
| M-50 | `passthrough_sanitisiert_nichts`, `passthrough zaehlt keinen nicht-endlichen Eingang` | `SONDE-015-rot-M-50.txt` | **gemessen** |
| M-51 | — | — | **Etappe 4**: Latenz und Tail meldet der Prozessor an den Host, nicht die Bibliothek. A16 misst sie. Der Kern hat kein Lookahead und keine lineare Phase, weil er keinen Speicher über den Block hinaus hält außer den Filterzuständen. |
| M-52 | `nach_dem_anlegen_immer_processed` | `SONDE-015-rot-M-52.txt` | **gemessen** — dass die Auswahl nicht persistiert, misst B2 und B7 (Etappe 4) |
| M-53 | `hoermatrix_dry_ist_bitidentisch` | `SONDE-015-rot-M-53.txt` | **gemessen** |
| M-54 | `delta_bei_gleichheit_ist_exakt_null`, `delta_ist_differenz_mit_festem_abgleich` | `SONDE-015-rot-M-54.txt` | **gemessen** |
| M-55 | `hoermatrixwechsel_ist_klickfrei`, `identische_zustaende_nullen_beim_wechsel` | `SONDE-015-rot-M-55.txt` | **gemessen** — der Bau des eigenen Übergangs war ein Befund dieser Etappe (§9.7 N-3) |
| M-56 | `candidate_ohne_kandidat_faellt_sichtbar_zurueck`, `mit_kandidat_greift_die_auswahl` | `SONDE-015-rot-M-56.txt` | **gemessen** |
| M-57 | `taps_sind_kohaerent`, `hoermatrix_liegt_hinter_den_taps` | `SONDE-015-rot-M-57.txt` | **gemessen** |
| M-109 | — | — | **Etappe 2, bereits grün**: Feld-ID 22, `FELD-IDS.json`, `flatc`-Regeneration, A9 Drift 0 (§8.1 Stufe 4). |
| M-110 | Der Kern ist die **Quelle** der acht Werte: `auslenkung_ist_gemessen_nicht_gerechnet` (M-27) und die Slot-Reihenfolge in `auslenkungenDb` | — | **Quelle gemessen**, Wire-Form in Etappe 2 (A10, B3c, A4). Der Sender ist Etappe 4. |
| M-111 | — | — | **Etappe 2, bereits grün**: Abwesenheit bleibt gültig und ist keine 0. |
| M-112 | — | — | **Etappe 2, bereits grün**: Länge ungleich 0 und 8 wird abgewiesen. |
| M-113 | `nichtendliche_auslenkung_wird_null_und_gezaehlt` | `SONDE-015-rot-M-113.txt` | **gemessen als Wache** — im regulären Betrieb kann keine nicht-endliche Auslenkung entstehen (`leistungInDb` klemmt auf `kStilleDb`); der Rotbeweis stellt den Fall her und nimmt den Riegel weg, dieselbe Form wie M-45 |
| M-114 | `feld_reist_nur_bei_vorhandener_dynamik`, `mit dynamischem Band reist das Feld`, `bei ausgeschaltetem EQ reist es nicht` | `SONDE-015-rot-M-114.txt` | **gemessen** — der Sendepfad (B10) ist Etappe 4 |
| M-121 | `remove_verwirft_die_alte_auslenkung`, `remove_mit_neubelegung_verwirft_die_alte_auslenkung` | `SONDE-015-rot-M-121.txt` | **gemessen** — der Rotbeweis deckte einen echten Defekt auf (§9.7 N-4) |
| M-122 | `bankpool_parallel_ohne_rennen` (zwei echte Threads, Invarianten vor und nach jedem Block, 3000 Publikationen), `generationen_bleiben_streng_monoton`, `kein stiller Ringverlust unter Last` | `SONDE-015-rot-M-122.txt` | **gemessen** — die MSVC-Toolchain dieses Projekts trägt keinen ThreadSanitizer; der äquivalente Stresslauf **ist** der Nachweis (§44.5 „beziehungsweise äquivalente Stressläufe"), und diese Zeile sagt das statt ein Werkzeug zu behaupten, das es hier nicht gibt |

### 9.4 Die Beine dieser Etappe

Alle einzeln gefahren, aus pwsh gebaut (`cmake` liegt nicht im Bash-`PATH`;
CLAUDE.md Maschinen-Landminen). Rohausgabe:
`docs/beweise/roh/SONDE-015-etappe3-beine.txt`.

| Bein | Ziel | Exit | Ergebnis | Warum es hier laeuft |
|---|---|---|---|---|
| **B6** | `EqCopDspGoldenTest` | 0 | DSP-GOLDEN OK | Der aktive DSP-Kern gegen die Matrix - das neue Bein dieser Etappe. |
| **A14** | `pruefe_kern_identitaetsfrei.py` | 0 | 25 ok, 0 Fehler | Der gemeinsame Kern traegt keine Bundle-Identitaet; die drei neuen Dsp-Objekte stehen in der Liste. |
| **B5** | `EqCopAnalysisGoldenTest` | 0 | Ergebnis: 248 bestanden, 0 Fehler. | Regressionswache: die FeatureEngine ist unberuehrt. |
| **A16** | `EqCopProbeeqNullTest` | 0 | SONDE-NULLTEST OK - 87 Pruefungen ok, 0 Fehler | Regressionswache: der heutige Passthrough von Probeeq ist unberuehrt - der Prozessor liegt nicht im Diff. |
| **B2** | `EqCopStateMigrationTest` | 0 | STATE-MIGRATION-TEST OK - 247 Pruefungen ok, 0 Fehler | Regressionswache: State, Parametertabelle und Preset aus Etappe 2 sind unberuehrt. |
| **B3c** | `EqCopSchemaTest` | 0 | 152 bestanden, 0 gescheitert | Regressionswache: der v3-Vertrag aus Etappe 2 ist unberuehrt. |

### 9.5 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Etappe 3

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Der ACK-Ring hat eine ausdrückliche Politik bei *voll*: er verriegelt den Slot statt den ACK zu verwerfen (`DspBankPool::meldeAusgedient`). Der Zähler, der strukturell 0 sein muss, ist eine **Wache mit Test** — B6 misst die Kopplung `ackUeberlaeufe() == reclaimVerriegelungen()`, und der Rotbeweis M-45 senkt dafür die Ringgröße unter die Slotzahl. Derselbe Gedanke trägt den Denormal-Riegel (E-14) und den Endlichkeitsriegel der Auslenkung (M-113): beide zählen, was sie abfangen. |
| **B** Lebenszyklus | `bereiteVor` ↔ `freigeben` im selben Änderungssatz; `reserviere` ↔ `ernteAcks`, `beginneVerblassen` ↔ `meldeAusgedient`, `uebernehmeZustand(candidate)` ↔ `beendeCandidate` je paarweise gebaut und gemessen (B6 Abschnitt I, M-46). |
| **C** Verträge und Längen | Der Kern liest ausschließlich `nakama::parameter::DspSatz`; er prüft keine Längen selbst, sondern kappt samplerateabhängig (M-12) und klemmt, was der Vertrag heute nicht liefern kann (M-23). NaN/Inf an jedem Eingang: M-49, M-50, M-113. |
| **D** Bau- und Prüfriegel | B6 ist im Runner **scharf** (`AbPhase` `jetzt`) und gebaut — beides im selben Änderungssatz, sonst ist der Kanon rot oder blind (§5.15 Feinheit 1). A14 führt die drei neuen Kernobjekte; ohne die Zeile wäre `[S6]` rot (§9.7 N-1). |
| **E** Behauptung ≤ Messung | Die Runner-Behauptung zu B6 nennt genau, was der Test misst, und nicht mehr: keine Transaktion, kein Zustand, keine Telemetrie, kein Prozessor — die gehören zu B7 und A16. Zahlen in diesem Abschnitt sind gemessen (§9.4), nicht abgeschrieben. Jede neue Prüfung wurde **einmal absichtlich gebrochen** (§9.3, Spalte Rotbeweis). |
| **F** Änderungssatz | Kern, Test, CMake-Ziel, Runner-Eintrag und A14-Objektliste liegen in demselben Änderungssatz. Speichern↔Laden und Apply↔Revert berührt diese Etappe nicht — sie sind Etappe 4. |

### 9.6 Abweichungen dieser Etappe

| Nr. | Wortlaut | Was gebaut wurde | Begründung |
|---|---|---|---|
| A-1 | **§5.15 Feinheit 2 / M-13:** „je Gitterstelle läuft ein eingeschwungener Sinus durch das echte `DspProgram`" | Je Prüfling läuft **ein Impuls** durch den echten `DspKern`, abgegriffen am Tap `post_committed`; die Übertragung an den 240 Gitterstellen entsteht als DFT dieser Impulsantwort. Ein **eingeschwungener Sinus** läuft zusätzlich an drei Stellen je Prüfling und bestätigt, dass beide Messarten denselben Wert liefern (Abschnitt C1). | Ein Sinuslauf je Gitterstelle bräuchte bei 20 Hz und 192 kHz rund 10<sup>6</sup> Samples je Stelle, mal 240 Stellen mal 28 Prüflinge — der Kanon liefe Stunden. Die Impulsantwort misst **denselben Audiopfad** (dieselbe Bank, dieselben Koeffizienten, dieselben Zustände) und ist genauer, weil sie am `double`-Tap abgegriffen wird. Die Zusage, dass beide dasselbe messen, wird nicht behauptet, sondern gemessen; damit bleibt die Messart selbst Gegenstand des Tests, wie M-13 es verlangt. |
| A-2 | **Ticketpfade:** Etappe 3 fasst `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` nicht an | Die A14-Objektliste wurde um **`DspProgramm.obj`, `DspBankPool.obj` und `DspKern.obj`** ergänzt — und um **`NakamaPreset.obj`**, das seit Etappe 2 fehlte. | Der Auftrag nennt A14 ausdrücklich als zu fahrendes Bein und verlangt, dass neue Kernquellen dort weiter grün sind. A14 führt seine Objektliste bewusst von Hand, „damit das Bein beim ersten Lauf spricht, statt still durchzurutschen" — genau das ist geschehen (§9.7 N-1). |
| A-3 | **§4.8:** je Matrixzeile **ein** Rotbeweis | **M-11 trägt fünf** Rotbeweise (`M-11a` bis `M-11e`), je einen für `low_shelf`, `high_shelf`, `notch`, `low_cut` und `high_cut`. | M-11 sagt selbst: „Jeder der sechs Vertragstypen wird **einzeln** gemessen, nicht stellvertretend durch einen Nachbarn." Ein einziger Rotbeweis an einem Typ ließe die anderen vier ungeprüft — genau der Fehler, gegen den die Zeile gebaut ist. Fünf getrennte Dateien sind stärker als eine. |
| A-4 | **M-37:** „ein +6-dB-Shelf, der das ganze Gitter überstreicht" | Der Referenzprüfling ist ein `high_shelf` bei 20 Hz mit **Q 1,0** statt 0,707. | Mit Q 0,707 liegen die ersten rund 16 % der Gitterstellen noch im Übergang, und der Energiemittelwert landet bei −5,86 dB — der Prüfling **überstreicht das Gitter nicht**. Das ist keine Toleranzfrage: 20 Hz ist die kleinste zulässige `freq_hz`, ein Shelf mit tieferer Ecke ist im Vertrag nicht baubar. Mit Q 1,0 erreicht das Plateau das Gitter praktisch vollständig (−5,997 dB). Die eigentliche Zusage von M-37 — dass ein identisches Mid/Side-Paar **auf den Stereo-Fall zusammenfällt** — wird zusätzlich direkt gegen den Stereo-Fall gemessen und hängt an keinem Prüfling. |

### 9.7 Nebenbefunde (nicht ins Register)

| Nr. | Befund | Wo gefunden | Was geschah |
|---|---|---|---|
| N-1 | **A14 war am Basis-SHA rot.** `state/NakamaPreset.cpp` steht seit Etappe 2 in `NAKAMA_KERN_QUELLEN`, aber nicht in der Objektliste von `pruefe_kern_identitaetsfrei.py`. Abschnitt `[3]` meldete „Archivmitglieder sind genau die 15 Kernobjekte" mit 16 gefundenen und endete mit Exit 2. | Erster Lauf dieser Etappe, noch vor der ersten Zeile Code. | Behoben: `NakamaPreset.obj` und die drei neuen `Dsp*.obj` stehen jetzt in der Liste, A14 meldet 19 Kernobjekte und Exit 0 (§9.6 A-2). Genau dafür führt das Bein seine Liste von Hand — es hat beim ersten Lauf gesprochen, statt still durchzurutschen. |
| N-2 | **`verarbeite` verwarf Blöcke größer als `maxBlock` ganz.** Ein Host mit größerem Puffer als angekündigt hätte Stille bekommen — ein Audioausfall, wo nur die Analyse ausfallen darf. | Rotbeweis zu M-48: der Test fand, dass der Block gar nicht verarbeitet wurde. | Behoben: der Block läuft in Stücken durch, verworfen wird nur der Tap (Entscheid E-11). |
| N-3 | **Die Hörmatrix wechselte hart.** Sie liegt hinter allen drei Taps und fährt bei keinem Bankwechsel mit; ein Wechsel Processed nach Dry sprang bei aktivem EQ um die volle Differenz beider Wege. M-55 sagt „Ein Wechsel ist klickfrei". | Beim Bau der M-55-Prüfung, die in der ersten Fassung dieser Etappe fehlte. | Behoben: die Hörmatrix hat einen **eigenen** Übergang über `kFadeSamples`, linear wie jeder andere Übergang des Kerns. |
| N-4 | **Stale Auslenkung nach einem Remove.** `verarbeiteBand` kehrt bei einem freien Slot früh zurück und schrieb die Auslenkung nie; wurde ein dynamisches Band entfernt, meldete `auslenkungenDb` weiter den alten, endlichen Wert. Genau der Fall, gegen den M-121 gebaut ist: „acht **alte**, endliche Werte bestehen alle diese Prüfungen." | Rotbeweis zu M-121, nachdem der Test um den Remove-Fall erweitert wurde — die erste Fassung wechselte nur auf ein statisches Band und traf den Fall nicht. | Behoben: `verarbeiteBank` nullt vor der Bandschleife die Auslenkung jedes Slots, der keine erzeugt. |
| N-5 | **Zwei Zusagen sind Wachen, nicht Wege.** Der Overflow-Zweig des ACK-Rings (M-45) und der Endlichkeitsriegel der Auslenkung (M-113) können im regulären Betrieb nicht auslösen: der Ring fasst mehr Einträge als es Slots gibt, und `leistungInDb` klemmt auf `kStilleDb`. Ein Zähler, der strukturell 0 bleibt, ist nach Prüfliste A eine Wache mit Test. | Beim Bau der beiden Rotbeweise, die zunächst nichts rot bekamen. | Gelöst: beide werden über eine **Kopplung** gemessen (Überläufe gleich Verriegelungen; jeder gemeldete Wert endlich), und der Rotbeweis stellt den unerreichbaren Fall her, bevor er die Zusage bricht. Die Rohdatei nennt beide Stellen getrennt. |
| N-6 | **Vier Rotbeweise fielen zunächst an einem Nebeneffekt.** Ein Bypasswechsel, der mit einem High-Shelf bei 4 kHz an DC gemessen wird, ändert nichts; ein `dynamic_enabled`-Wechsel, der an DC gemessen wird, regt den Detektor nicht an; ein Mix-Kurzschluss, der bei laufender Rampe gemessen wird, ist gar nicht gefragt; ein Sprungdeckel, der den Eigensprung eines Sinus zulässt, deckt auch den Wechselsprung. | Systematisch beim Erzeugen der Rotbeweise — das Skript prüft, ob **die erwartete** Prüfung fällt, und meldet sonst einen Befund. | Alle vier Prüfungen wurden geschärft: DC nur, wo die Kette DC verändert; Sinus mit **zweiter Differenz** als Klickmaßstab; Messung bei ruhender Rampe. §4.8 verlangt genau das: der Rotbeweis fällt an der Zeile, die die Zusage trägt. |
| N-7 | **Der Auto-Gain-Prüfling aus M-37 überstreicht das Gitter nicht.** Ein High-Shelf bei 20 Hz mit Q 0,707 erreicht sein Plateau erst deutlich über 20 Hz; der Energiemittelwert landet bei −5,86 dB statt −6,0 ± 0,1. 20 Hz ist die kleinste zulässige `freq_hz`, ein Shelf mit tieferer Ecke ist im Vertrag nicht baubar. | Erster Lauf des Auto-Gain-Abschnitts. | Prüflingswahl auf Q 1,0 geändert (§9.6 A-4) und die eigentliche B-04-Zusage direkt gegen den Stereo-Fall gemessen, wo sie an keinem Prüfling hängt. |
| N-8 | **Denormals waren nicht behandelt.** Der Selbstaudit des Auftrags nennt sie ausdrücklich; ein abklingender Filterzustand läuft nach genügend Stille in den denormalen Bereich, und auf x86 kostet jede Rechnung damit ein Vielfaches — ein Kern, der im **Leerlauf** langsamer wird als unter Last, verletzt die Echtzeitfestigkeit aus §44.5. | Beim Durchgehen der Selbstaudit-Liste vor dem Commit. | Behoben: ein Riegel am Blockrand nullt Zustandswerte unter 1e-300 und zählt die Treffer (Entscheid E-14). B6 misst, **dass** er greift — ein Impuls, dann 200 000 Samples Stille, dann `geriegelteDenormale() > 0` — und dass er nichts Hörbares ändert. Die FTZ/DAZ-Schalter bleiben der Zielschicht überlassen; sie sind eine Entscheidung des Prozessors, nicht der Bibliothek. |

### 9.8 Erstprüfung Etappe 3 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08a5a-51da-7960-bd3e-88e44c7a4b23`; Lauf 10:06–10:38 |
| Prüfbereich | Erstprüfung (Vorlage A) über `git diff 2672ed2b...2c99fc8a -- eq-copilot/plugin/dsp eq-copilot/plugin/tests/DspGoldenTestMain.cpp eq-copilot/plugin/CMakeLists.txt tools/beweise.ps1 tools/eq-copilot/pruefe_kern_identitaetsfrei.py`; HEAD während des Laufs `8cd2b2b6` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-3-erstpruefung-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-3-erstpruefung-2c99fc8.txt` |
| Urteil | **NEEDS_WORK** — 26 Befunde: drei P1 (Slot-Übernahme ohne Generation; Vorzeichen des High-Shelf-SVF-Mischterms; Delta-Hörmatrix bleibt bei EQ-aus und Hard-Bypass stumm statt Passthrough), 23 P2 (neun am Kern, dreizehn an B6 und den Rotbeweisen, einer an der Runner-Behauptung) |
| Quellencheck des Dirigenten | zwei lesende Opus-Agenten, jede zitierte Zeile geöffnet, jede Zusage wörtlich aus Matrix, Regel oder Entwurf gezogen: alle 26 **DEFEKT**, keine Lücke, keine Härtung. Vier Zitatkorrekturen, die den Befund nicht kippen: B-2 trägt M-19 nur über die Nebenklausel „klingt wie sein statischer Zustand" (die Hauptzusage „exakt 0,0" hält der Code) und U15 steht im Kontext „kein linearphasiger Modus"; B-5 trifft §44.2 „liest bis Fade-Ende unverändert" nicht, tragend sind M-03/M-06 (Sprung am Umschaltsample); B-10 greift M-07, nicht M-01 („frische Instanz"); B-18 ist enger als dargestellt (der Getter vor dem ersten Block fängt eine vollständig verlegte Ableitung, ungedeckt bleibt nur eine zusätzliche im Callback). Zwei Nebenbefunde an der Matrix: die Rotbeweisspalte von M-30 („ein LSB Abweichung") ist mit `double`-Arithmetik nicht erfüllbar; die Begründung von E-9 („kostete eine fünfte Bank") ist falsch, zwei Bänke je Pfad reichen. Gemeinsame Ursache der Kernbefunde B-1, B-5 bis B-12: der Lebenszyklus der Bänke und der zwei Pfade ist nicht als geschlossener Automat mit Generation, je Pfad eigenem Zustand und gekoppeltem Ende gebaut |
| Rundenbilanz | `2672ed2b..2c99fc8a: Produkt 10 Datei(en) +2621/-1 · Tests 1 Datei(en) +2560/-0 · Pruefwerkzeug 2 Datei(en) +16/-1 · Doku 61 Datei(en) +3143/-5` — Bau mit Produktfortschritt |
| Nacharbeit | Etappe 3, Nacharbeit 1 (Runde 1 von 3): `docs/beweise/roh/SONDE-015-etappe-3-nacharbeit-1-auftrag.txt` — nur die 26 Defekte mit je einer schließenden Regel, die Ursache als Auftrag für B-1 und B-5 bis B-12; nur betroffene Beine (B6, A14, B5, A16, B2, B3c) |

| Befund | Kurzform | Einordnung | Ansatzpunkt |
|---|---|---|---|
| B-1 | Audiothread übernimmt Slot ohne Generationsnachweis; wiederverwendeter Slot wird Committed | DEFEKT (R9 „per Generation", §44.2 „Index plus Generation", M-42) | `eq-copilot/plugin/dsp/DspBankPool.cpp:176-190` |
| B-2 | High-Shelf-SVF `m1 = k·(A−1)·A` statt `k·(1−A)·A`; falsche Ruheantwort, ab −12 dB Gesamtgain Nullstellen rechts | DEFEKT (M-19 „klingt wie sein statischer Zustand", U15 minimalphasig) | `eq-copilot/plugin/dsp/DspSvf.h:126-137` |
| B-3 | Bei Delta oder Candidate greift der Passthrough-Kurzschluss nicht; EQ-aus schreibt dauerhaft Stille | DEFEKT (M-05 „schreibt keine Samples", §3.0, Grundgesetz) | `eq-copilot/plugin/dsp/DspKern.cpp:550-554` |
| B-4 | Bandwerte (freq, q, gain) nur über neue kalte Bank plus Crossfade, keine Rampe | DEFEKT (R8 „kontinuierliche Werte über Rampen", Vertrag `wechsel = rampe`) | `eq-copilot/plugin/dsp/DspKern.cpp:208-217`, `:130` |
| B-5 | Zweiter Wechsel im laufenden Fade dient die verblassende Bank sofort aus; Sprung 0,75·(B−A) | DEFEKT (M-03/M-06 klickfrei) | `eq-copilot/plugin/dsp/DspKern.cpp:193-202` |
| B-6 | Candidate liest die Rampen und den Auto-Gain des Committed-Programms | DEFEKT (§3.0 „derselbe Aufbau auf einer zweiten Bank") | `eq-copilot/plugin/dsp/DspKern.cpp:643-646` |
| B-7 | Candidate-Wechsel blendet aus Dry statt aus der bisherigen Candidate-Bank | DEFEKT (§44.2 „innerhalb ihres Pfads klickfrei") | `eq-copilot/plugin/dsp/DspKern.cpp:649-661` |
| B-8 | `beendeCandidate` löscht nur das Flag; Bänke bleiben belegt, kein ACK, Vorblockdaten im Rückfall | DEFEKT (M-46 „neutral beendet", Prüfliste B paarweise) | `eq-copilot/plugin/dsp/DspKern.cpp:153-156` |
| B-9 | Committed und Candidate schreiben dieselben acht Auslenkungs-Atomics; Candidate zuletzt | DEFEKT (M-27, R14 Klemmung, „kohärente Snapshots") | `eq-copilot/plugin/dsp/DspKern.cpp:357`, `:438-443` |
| B-10 | Zustand mit `eq_enabled=false` reserviert und publiziert eine Bank; sie bleibt `audio_active` | DEFEKT (M-07 „keine Bank ist audio_active"; Test-Toleranz `<= 1`) | `eq-copilot/plugin/dsp/DspKern.cpp:121-125`, B6 `:444-446` |
| B-11 | Programmübernahme an jedem Teilstückrand innerhalb eines äußeren Blocks | DEFEKT (M-25 „nie samplegenau mitten im Block", R9) | `eq-copilot/plugin/dsp/DspKern.cpp:511-532` |
| B-12 | `zuruecksetzen` setzt den Generationszähler auf 0; Neuanlauf vergibt wieder 1 | DEFEKT (R9 Feinheit 2 „nie zurückgesetzt") | `eq-copilot/plugin/dsp/DspBankPool.cpp:82-86` |
| B-13 | `meldeSperre` ohne Aufrufer; Sperrzähler strukturell 0 ohne Wache | DEFEKT (M-47 Sperrzähler, Prüfliste A „Wache mit Test") | `eq-copilot/plugin/dsp/DspRtWache.h:46-52` |
| B-14 | Knietest vergleicht zwei Konstanten; der Kern wird für den Knie-Punkt nicht angeregt | DEFEKT (M-18, Nebeneffekt-Regel §3) | B6 `:1189-1193` |
| B-15 | M-26 nur bei 48 kHz, nur Endwert zweier Hold-Läufe; keine Sprungantwort bei vier Raten | DEFEKT (M-26 Testspalte „Sprungantwort bei vier Sampleraten") | B6 `:1058`, `:1353-1357` |
| B-16 | M-31 mit neutralem Filter; Rotbeweis fällt am Gainverlust, nicht an der Reihenfolge | DEFEKT (M-31, §4.8 Rot an der Zusage) | B6 `:1436-1449`, `SONDE-015-rot-M-31.txt` |
| B-17 | M-45 vergleicht nur zwei Zähler; Maske und Slotzustand nie assertiert | DEFEKT (M-45 „Slot bleibt dauerhaft nicht frei") | B6 `:1856-1863`, `SONDE-015-rot-M-45.txt` |
| B-18 | M-39-Rotbeweis mutiert den Berichtsspeicher, nicht den Rechenort | DEFEKT (§4.8 Rot an der Zusage; M-39) | B6 `:1720-1725`, `SONDE-015-rot-M-39.txt` |
| B-19 | M-30-Rotbeweis braucht eine zweite Mutation (float); der Kurzschluss allein ist nicht beobachtbar | DEFEKT (§4.8 eine Mutation an der Zusage; M-30-Spalte berichtigen) | B6 `:1410-1411`, `SONDE-015-rot-M-30.txt` |
| B-20 | Fall `nichtendlicher_filterzustand_wird_am_blockrand_geheilt` fehlt; `geheilteFilterzustaende()` nie gelesen | DEFEKT (M-49 Testspalte nennt den Fall) | B6 `:2033-2047`, `DspKern.h:150-151` |
| B-21 | M-113 prüft nur Endlichkeit; „exakt 0" und Zähleranstieg nicht assertiert | DEFEKT (R14 „nicht-endlich → 0 und gezählt", NaN-Ehrlichkeit) | B6 `:2079-2091` |
| B-22 | M-48: `audioGelaufen` wird durch ein geändertes Sample wahr; genullter oder teilverarbeiteter Block besteht | DEFEKT (M-48 „Audiopfad läuft unverändert weiter") | B6 `:1971-1980` |
| B-23 | M-54 vergleicht zwei normierte Läufe nur miteinander; stummes oder vertauschtes Delta besteht | DEFEKT (M-54 „Delta = Processed − Dry") | B6 `:2162-2173` |
| B-24 | Golden-Gitter ohne Eck-/Mittenfrequenz und Shelf-Punkte f/4, 4f je Prüfling | DEFEKT (R15 Feinheit 3 wörtlich) | B6 `:365-374`, `:786` |
| B-25 | Zähler erst nach dem ersten Programmwechsel scharf; keine Wechsel, Fades, Reclaims, Transportkanten im Lauf | DEFEKT (M-47 „mit Transportkanten und Programmwechseln", M-41 „auch nicht beim ersten Programmwechsel") | B6 `:1910-1938` |
| B-26 | Runner-Behauptung „bei Mix 0 wird kein Sample geschrieben (bitgleich)" nennt Output-Trim 0 dB nicht; Samples werden geschrieben | DEFEKT (M-116, §4.8 „nicht mehr, als der Test misst") | `tools/beweise.ps1:688` |

### 9.10 Wiederprüfung 1 Etappe 3 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08afd-4ccf-78f3-a8c0-7b598dbe0cf5`; Lauf 13:04–13:18 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff bc76e98c...ca7e7c50 -- eq-copilot/plugin/dsp eq-copilot/plugin/tests/DspGoldenTestMain.cpp eq-copilot/plugin/CMakeLists.txt tools/beweise.ps1`; HEAD während des Laufs `fb269b99` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-3-wiederpruefung-1-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-3-wiederpruefung-1-ca7e7c5.txt` |
| Urteil | **NEEDS_WORK** — 21 von 26 Befunden geschlossen (B-1, B-2, B-6, B-9 bis B-26 je mit Beleg und Rotbeweis); offen B-3, B-4 (zwei Teilbefunde), B-5/B-7 (gemeinsam) und B-8; nichts gebrochen (keine neue Rennen-, Ownership- oder Reclaim-Verletzung, A14, CMake und Prozessor ohne Bruch) |
| Quellencheck des Dirigenten | lesender Opus-Agent, jede zitierte Zeile geöffnet: **vier DEFEKTE, eine LÜCKE.** W-1 (B-3): `committedRuht` wird je Teilstück am Stückbeginn gebildet, nach Fade-Ende im selben Teilstück wird `(float)(double)Eingang` zurückgeschrieben — der Schreibverzicht aus REGEL B-3 bricht, die Bitidentität nicht (nur ein sNaN würde ruhiggestellt). W-2 (B-4): attack_ms, hold_ms, release_ms tragen im Vertrag `wechsel = rampe`, die Interpolation E-19 lässt sie aus — Knick statt Sprung, aber REGEL B-4 nicht erfüllt. W-3 (B-4): der interpolierte `rangeDb` wird am Flag `detektorLaeuft` des neuen Programms vorbeigeführt, bis 12 dB in 8 Samples — R8 und M-17 brechen; M-20 ist eine Ruhezusage, keine Übergangszusage. W-5 (B-8): beim Abbruch im laufenden Hörmatrix-Fade werden beide Seiten des Fades `processed`, der Mischwert fällt in einem Sample — REGEL B-8 „blendet klickfrei aus" bricht in der Hörmatrix, nicht im Bank-Lebenszyklus. W-4 (B-5/B-7): die Metrik `groessterSprung` ist das Maximum der Nachbarsample-Differenz über den ganzen Lauf; damit ist 1× Fadeschritt unerreichbar, weil die kalt startende Bank einschwingt (Beine: 0,0025299 gegen 0,0021309, Faktor 1,187; Gegenprobe N-12 ohne kalte Bank exakt 1×) — am Umschaltsample selbst entsteht kein Sprung, M-03/M-06 halten. REGEL B-5/B-7 sagte „≤ Fadeschritt", ohne die Messgröße festzulegen: **Lücke** |
| Entscheid zur Lücke W-4 (Dirigent, Technik) | Der Sprung wird **am Umschaltsample** gemessen — die Nachbarsample-Differenz am ersten Sample des neuen Fades gegen die Fadeschrittweite plus begründete Rundungstoleranz (M-03 Rotbeweisspalte „am Umschaltsample"). Die Maximumsprüfung über den ganzen Lauf bleibt als Wache mit einer aus dem Einschwingen der kalten Bank begründeten Schranke, nie als unbegründeter Faktor. Die Regel geht als REGEL W-4 in den Nacharbeitsauftrag 2 und als Entscheid nach §9.2 |
| Rundenbilanz Nacharbeit 1 | `bc76e98c..ca7e7c50: Produkt 8 Datei(en) +965/-539 · Tests 1 Datei(en) +1200/-55 · Pruefwerkzeug 1 Datei(en) +1/-1 · Doku 31 Datei(en) +2123/-3` — Runde mit Produktfortschritt, kein Konvergenz-Signal |
| Nacharbeit | Etappe 3, Nacharbeit 2 (Runde 2 von 3): `docs/beweise/roh/SONDE-015-etappe-3-nacharbeit-2-auftrag.txt` — die vier Defekte W-1, W-2, W-3, W-5 mit je einer schließenden Regel und die entschiedene Regel W-4; nur betroffene Beine (B6, A14, B5, A16, B2, B3c) |

| Befund | Kurzform | Einordnung | Ansatzpunkt |
|---|---|---|---|
| W-1 | B-3: nach Fade-Ende werden die Restsamples des Teilstücks zurückgeschrieben | DEFEKT (REGEL B-3, M-05 „schreibt keine Samples") | `eq-copilot/plugin/dsp/DspKern.cpp:743`, `:862-883` |
| W-2 | B-4: attack_ms, hold_ms, release_ms nicht gerampt | DEFEKT (REGEL B-4, Vertrag `wechsel = rampe`, E-19 lückenhaft) | `eq-copilot/plugin/dsp/DspKern.cpp:394`, `:411`; `DspProgramm.cpp:154-156` |
| W-3 | B-4: Range → 0 springt über `detektorLaeuft = false` in 8 Samples | DEFEKT (R8, M-17, REGEL B-4) | `eq-copilot/plugin/dsp/DspKern.cpp:425-430`; `DspProgramm.cpp:245` |
| W-4 | B-5/B-7: Testtoleranz 4× Fadeschritt | LÜCKE, entschieden (Messung am Umschaltsample) | B6 `:469-474`, `:973-978`, `:3288-3292` |
| W-5 | B-8: Candidate-Abbruch im laufenden Hörmatrix-Fade springt auf Processed | DEFEKT (REGEL B-8 „blendet klickfrei aus") | `eq-copilot/plugin/dsp/DspKern.cpp:192-193`, `:720-722`, `:811-821` |

### 9.9 Nacharbeit 1 — was eingearbeitet wurde (10.09.2026)

| Merkmal | Wert |
|---|---|
| Auftrag | `docs/beweise/roh/SONDE-015-etappe-3-nacharbeit-1-auftrag.txt` — die 26 bestätigten Defekte der Erstprüfung (§9.8) mit je einer schließenden Regel; die Ursache der Befunde B-1 und B-5 bis B-12 als Auftrag |
| Basis-SHA | `bc76e98c3dc4a83b87c24bc81386ba8ca183863e`, mit `git rev-parse HEAD` beim Start gemessen; `git status --short` war leer |
| Urteil | **Offen.** Die Wiederprüfung steht aus. |
| Ursache | Der Lebenszyklus ist jetzt ein geschlossener Automat: Generation im Zustandswort (E-15), je Pfad ein vollständiger eigener Zustand (E-16), jeder Übergang endet vor dem nächsten (E-17), Ausschalten und Candidate-Ende über die ENDE-Marke (E-18), Übernahme einmal je äußerem Aufruf (B-11), Generationszähler getrennt vom Ressourcenreset (B-12). Die neun Matrixzeilen bleiben die Messung; jede fällt an ihrer Trägerzeile (Tabelle unten, Spalte Rotbeweis). |
| Beine | **6** gefahren (alle grün), Rohausgabe `docs/beweise/roh/SONDE-015-etappe3-nacharbeit1-beine.txt` (§9.9.3) |
| Rotbeweise | **29** Dateien dieser Runde (`ls docs/beweise/roh/SONDE-015-rot-B-*-e3-n1.txt`), davon eine Nachweisdatei ohne Laufbein (B-26); insgesamt **127** Rotbeweisdateien des Tickets (`ls docs/beweise/roh/SONDE-015-rot-*`) |
| Kanon | **Nicht gefahren** — gehört an das Ende der Etappe 4. |
| Prozessor | **Unberührt.** `eq-copilot/plugin/sonde/SondeProcessor.{h,cpp}` und alles aus Etappe 2 sind nicht im Diff. |

| Befund | Was gebaut wurde | Trägerzeile (Datei:Zeile) | Test in B6 | Rotbeweis | Status |
|---|---|---|---|---|---|
| B-1 | Generation im Zustandswort; Übernahme nur in der publizierten Generation (E-15) | `eq-copilot/plugin/dsp/DspBankPool.cpp:230` | `uebernahme_nur_in_der_publizierten_generation`, `keine_bank_liegt_je_in_zwei_pfaden` (M-122) | `SONDE-015-rot-B-01-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-2 | Mischterm `k·(1−A)·A` | `eq-copilot/plugin/dsp/DspSvf.h:140` | `dynamisches_high_shelf_`, `_low_shelf_`, `_bell_ruhe_gegen_rbj` (vier Raten), `dynamischer_high_shelf_bei_minus_24_db_gegen_rbj` | `SONDE-015-rot-B-02-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-3 | EQ aus und Hard-Bypass überbrücken jede Hörmatrix; Fade auf den Eingang | `eq-copilot/plugin/dsp/DspKern.cpp:743` | `hoermatrix_blendet_bei_aus_klickfrei_auf_den_eingang` und `nach_dem_fade_schreibt_keine_hoermatrix_ein_sample`, je Delta und Candidate über EQ aus und Hard-Bypass | `SONDE-015-rot-B-03-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-4 | Rampenübergang mit Zustandsübernahme und Koeffizienteninterpolation (E-19, E-20) | `eq-copilot/plugin/dsp/DspKern.cpp:446`, `eq-copilot/plugin/dsp/DspKern.cpp:287`, `eq-copilot/plugin/dsp/DspProgramm.cpp:139` | `bandwert_gain_db_`, `bandwert_freq_hz_`, `bandwert_q_rampt_ohne_zustandsreset` | `SONDE-015-rot-B-04-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-5 | laufender Übergang wird zu Ende geführt (E-17) | `eq-copilot/plugin/dsp/DspKern.cpp:254` | `zweiter_wechsel_im_laufenden_fade_springt_nicht`, `der_zweite_wechsel_wird_nach_dem_fade_uebernommen` | `SONDE-015-rot-B-05-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-6 | eigene Rampen und eigener Auto-Gain je Pfad (E-16) | `eq-copilot/plugin/dsp/DspKern.cpp:480` | `candidate_fuehrt_eigene_rampen`, `candidate_auto_gain_aus_eigener_kurve` | `SONDE-015-rot-B-06-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-7 | Candidate blendet aus seiner bisherigen Bank (E-16, E-17) | `eq-copilot/plugin/dsp/DspKern.cpp:601` | `candidate_wechsel_blendet_aus_der_bisherigen_candidate_bank` | `SONDE-015-rot-B-07-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-8 | Candidate-Ende über ENDE-Marke, Fade, ACK (E-18) | `eq-copilot/plugin/dsp/DspKern.cpp:193` | `candidate_ende_blendet_aus_und_gibt_beide_baenke_frei` | `SONDE-015-rot-B-08-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-9 | acht Auslenkungen je Pfad, zwei Getter (E-16) | `eq-copilot/plugin/dsp/DspKern.cpp:547`, `eq-copilot/plugin/dsp/DspKern.cpp:472` | `auslenkungen_liegen_je_pfad_getrennt` | `SONDE-015-rot-B-09-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-10 | Aus-Zustand reserviert keine Bank (E-18) | `eq-copilot/plugin/dsp/DspKern.cpp:155` | `ausschalten_gibt_jede_bank_frei`, `ausgeschaltet ist keine Bank belegt` (vier Raten) | `SONDE-015-rot-B-10-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-11 | Übernahme einmal je äußerem Aufruf | `eq-copilot/plugin/dsp/DspKern.cpp:679` | `uebernahme_nur_am_aeusseren_blockrand` | `SONDE-015-rot-B-11-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-12 | Generationszähler außerhalb des Ressourcenresets | `eq-copilot/plugin/dsp/DspBankPool.cpp:85` | `generation_waechst_ueber_den_neuanlauf`, `ack_einer_alten_generation_gibt_keine_bank_frei` | `SONDE-015-rot-B-12-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-13 | Sperrwrapper mit Meldung, Textriegel | `eq-copilot/plugin/dsp/DspRtWache.h:84` | `sperrzaehler_sieht_eine_sperre_im_audiopfad`, `keine_sperre_am_wrapper_vorbei_im_kern` | `SONDE-015-rot-B-13a-e3-n1.txt`, `SONDE-015-rot-B-13b-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-14 | Knie am Audiopfad angeregt (E-25) | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1616` | `kennlinie_im_knie_wirkt_im_audiopfad`, `plateau_wirkt_zusaetzlich_zu_gain_db_im_audiopfad` | `SONDE-015-rot-B-14-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-15 | Sprungantwort bei vier Raten (E-25) | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1861` | `attack_hold_release_als_sprungantwort_bei_vier_raten`, `dieselbe_ms_angabe_ergibt_bei_jeder_rate_dieselbe_zeit` | `SONDE-015-rot-B-15-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-16 | Reihenfolge mit unterscheidbaren Wegen (E-25) | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2027` | `input_trim_liegt_vor_der_bank_der_detektor_hoert_ihn`, `output_trim_liegt_hinter_mix` | `SONDE-015-rot-B-16-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-17 | erzwungener Überlauf an Maske und Slotzustand (E-23) | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2592`, `eq-copilot/plugin/dsp/DspBankPool.cpp:277` | `erzwungener_ueberlauf_verriegelt_den_slot_bis_zur_bestaetigung`, `bestaetigung_ohne_verriegelung_gibt_nichts_frei` | `SONDE-015-rot-B-17-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-18 | Ableitungszähler am Rechenort | `eq-copilot/plugin/dsp/DspProgramm.cpp:89` | `auto_gain_wird_nicht_im_audiothread_gerechnet` (Ableitungszähler) | `SONDE-015-rot-B-18-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-19 | zwei Zusagen, je eine Mutation | `eq-copilot/plugin/dsp/DspKern.cpp:516`, `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1964` | `width_eins_rechnet_die_ms_stufe_nicht`, `ms_stufe_ist_bei_width_eins_bitgenau_reversibel` | `SONDE-015-rot-B-19a-e3-n1.txt`, `SONDE-015-rot-B-19b-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-20 | zweiter Riegel eigenständig, Heilung vor dem ersten Sample (E-22) | `eq-copilot/plugin/dsp/DspKern.cpp:320`, `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2981` | `nichtendlicher_filterzustand_wird_am_blockrand_geheilt` | `SONDE-015-rot-B-20-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-21 | exakt 0,0 und Zähleranstieg | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:3040` | `nichtendliche_auslenkung_wird_null_und_gezaehlt` | `SONDE-015-rot-B-21-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-22 | sample-exakter Referenzlauf | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2826` | `der_uebergrosse_block_gleicht_sample_exakt_dem_lauf_ohne_ueberlast` | `SONDE-015-rot-B-22-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-23 | analytischer Sollwert | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:3131` | `delta_ist_differenz_mit_festem_abgleich` | `SONDE-015-rot-B-23-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-24 | Prüflingspunkte im Gitter, Scheitelmutation (E-27) | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:396` | `gitter_traegt_die_prueflingspunkte` je Prüfling und Rate, die sieben `_gegen_analytische_antwort` | `SONDE-015-rot-B-24a-e3-n1.txt`, `SONDE-015-rot-B-24b-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-25 | Zähler über den ganzen Lauf mit Wechseln und Transportkanten (E-26) | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2715` | `null_allokationen_im_callback_samt_programmwechseln` | `SONDE-015-rot-B-25-e3-n1.txt` | geschlossen — rot unter Mutation, grün nach Rücknahme |
| B-26 | Behauptung nennt Output-Trim 0 dB | `tools/beweise.ps1:688` | kein Laufbein — M-116 misst mit `git grep` (Nachweisdatei) | `SONDE-015-rot-B-26-e3-n1.txt` | geschlossen — Nachweis per `git grep` |

**Korrekturen an Zeilen der Messabdeckung.** §9.3 bleibt als Stand der Etappe 3 stehen; was sich an einer seiner Zeilen geändert hat, steht hier.

| ID | §9.3 sagt … | … jetzt |
|---|---|---|
| M-01 | Testfunktion „plus `ausgeschaltet ist keine Bank audio_active`" | Die Prüfung heißt `ausgeschaltet ist keine Bank belegt` und verlangt **0** belegte Bänke statt höchstens einer (B-10). |
| M-03, M-06 | zwei beziehungsweise zwei Testfunktionen | zusätzlich `zweiter_wechsel_im_laufenden_fade_springt_nicht` und `der_zweite_wechsel_wird_nach_dem_fade_uebernommen` (B-5); Rotbeweis B-05. |
| M-05 | `hard_bypass_schreibt_nicht` | zusätzlich je Hörmatrix-Auswahl (Delta, Candidate) und Weg (EQ aus, Hard-Bypass) `hoermatrix_blendet_bei_aus_klickfrei_auf_den_eingang` und `nach_dem_fade_schreibt_keine_hoermatrix_ein_sample` mit Wachmarke (B-3); Rotbeweis B-03. |
| M-07 | `ausgeschaltet_rechnet_nichts_und_startet_kalt` | zusätzlich `ausschalten_gibt_jede_bank_frei` (B-10); Rotbeweis B-10. |
| M-10, M-11 | 240 Gitterstellen | Das Gitter trägt je Prüfling die Eck- oder Mittenfrequenz und bei Shelves f/4 und 4f (241 bis 244 Stellen), `gitter_traegt_die_prueflingspunkte` misst ihre Anwesenheit; eine Nullstelle wird nach E-27 gemessen (B-24); Rotbeweise B-24a und B-24b. |
| M-17 | Output-Trim-Rampe | zusätzlich `bandwert_gain_db_`, `bandwert_freq_hz_` und `bandwert_q_rampt_ohne_zustandsreset` gegen eine stetige Referenzrampe (B-4); Rotbeweis B-04. |
| M-18 | `kennlinie_im_knie_ist_linear` | Der Konstantenvergleich ist ersetzt durch `kennlinie_im_knie_wirkt_im_audiopfad` und `plateau_wirkt_zusaetzlich_zu_gain_db_im_audiopfad` (B-14); Rotbeweis B-14. |
| M-19 | `unter_threshold_steht_das_band_exakt_still` | zusätzlich `dynamisches_high_shelf_`, `_low_shelf_` und `_bell_ruhe_gegen_rbj` bei vier Raten und `dynamischer_high_shelf_bei_minus_24_db_gegen_rbj` (B-2); Rotbeweis B-02. |
| M-25 | `dynamic_enabled_wechselt_nur_am_blockrand` | zusätzlich `uebernahme_nur_am_aeusseren_blockrand` (B-11); Rotbeweis B-11. |
| M-26 | `attack_hold_release_sind_drei_stufen` | zusätzlich `attack_hold_release_als_sprungantwort_bei_vier_raten` und `dieselbe_ms_angabe_ergibt_bei_jeder_rate_dieselbe_zeit` (B-15, E-25); Rotbeweis B-15. |
| M-27 | `auslenkung_ist_gemessen_nicht_gerechnet` | zusätzlich `auslenkungen_liegen_je_pfad_getrennt` (B-9); Rotbeweis B-09. |
| M-30 | ein Rotbeweis `SONDE-015-rot-M-30.txt` mit zwei Mutationen | zwei Zusagen mit je einer Mutation: `width_eins_rechnet_die_ms_stufe_nicht` (Rotbeweis B-19a) und `ms_stufe_ist_bei_width_eins_bitgenau_reversibel` (Rotbeweis B-19b). Die Rotbeweisspalte von M-30 („ein LSB Abweichung, weil die Matrix immer gerechnet wird") ist mit `double` nicht erfüllbar (§9.9.2 N-13); §3 bleibt unverändert. |
| M-31 | `input_trim_wirkt_und_liegt_vor_der_bank`; Rotbeweis fiel am Gainverlust | Die Prüfung heißt `input_trim_wirkt_mit_seinem_gesamtgain` — sie misst den Gesamtgain und nicht mehr. Die Reihenfolge messen `input_trim_liegt_vor_der_bank_der_detektor_hoert_ihn` und `output_trim_liegt_hinter_mix` (B-16); Rotbeweis B-16 belegt, dass der Gesamtgain unter der Verschiebung grün bleibt. |
| M-39 | `auto_gain_wird_nicht_im_audiothread_gerechnet` am Berichtswert | Der Berichtsfall heißt `auto_gain_steht_vor_dem_ersten_block_fest`; der Rechenort wird mit dem Ableitungszähler gemessen, im neuen Fall `auto_gain_wird_nicht_im_audiothread_gerechnet` (B-18); Rotbeweis B-18. |
| M-42 | sechs Übergänge einzeln | zusätzlich `uebernahme_nur_in_der_publizierten_generation` (B-1); Rotbeweis B-01. |
| M-45 | „gemessen als Wache", `reclaim_pending_mask_haelt_den_slot` verglich zwei Zähler | Die Kopplung heißt `ueberlaeufe_und_verriegelungen_sind_gekoppelt`; die Zusage misst **gemessen am erzwungenen Überlauf** `erzwungener_ueberlauf_verriegelt_den_slot_bis_zur_bestaetigung` an Maske und Slotzustand, dazu `bestaetigung_ohne_verriegelung_gibt_nichts_frei` (B-17, E-23); Rotbeweis B-17. |
| M-46 | `candidate_endet_neutral` prüfte das Flag | zusätzlich `candidate_ende_blendet_aus_und_gibt_beide_baenke_frei` (B-8); Rotbeweis B-08. |
| M-47 | 4000 Blöcke nach dem ersten Wechsel, Sperrzähler ohne Aufrufer | `null_allokationen_im_callback_samt_programmwechseln` hält die Zähler über den ganzen Lauf mit Programm-, Rampen-, Candidate- und Ausschaltwechseln, Ernte und Transportkanten (B-25, E-26); `sperrzaehler_sieht_eine_sperre_im_audiopfad` und `keine_sperre_am_wrapper_vorbei_im_kern` verdrahten den Sperrzähler (B-13); Rotbeweise B-25, B-13a, B-13b. |
| M-48 | `und Audio laeuft unveraendert weiter` am ersten geänderten Sample | `der_uebergrosse_block_gleicht_sample_exakt_dem_lauf_ohne_ueberlast` vergleicht alle 1024 Samples bitgenau mit einem Referenzlauf in 4 x 256 (B-22); Rotbeweis B-22. |
| M-49 | drei Fälle am Eingangsriegel | zusätzlich `nichtendlicher_filterzustand_wird_am_blockrand_geheilt` (B-20, E-22); Rotbeweis B-20. |
| M-54 | zwei normierte Läufe miteinander verglichen | `delta_ist_differenz_mit_festem_abgleich` vergleicht gegen den unabhängig gerechneten Sollwert (B-23); Rotbeweis B-23. |
| M-56, §3.0 | Candidate-Auswahl greift | zusätzlich `candidate_fuehrt_eigene_rampen`, `candidate_auto_gain_aus_eigener_kurve` (B-6) und `candidate_wechsel_blendet_aus_der_bisherigen_candidate_bank` (B-7); Rotbeweise B-06 und B-07. |
| M-113 | „gemessen als Wache", nur Endlichkeit | Die Wache heißt `im_regulaeren_betrieb_ist_jede_auslenkung_endlich`; die Zusage misst **gemessen am erzwungenen Fehlerfall** `nichtendliche_auslenkung_wird_null_und_gezaehlt` mit exakt 0,0 und Zähleranstieg (B-21); Rotbeweis B-21. |
| M-122 | Committed publiziert allein | Beide Pfade publizieren parallel, dazu Candidate-Enden; `keine_bank_liegt_je_in_zwei_pfaden` und Generationen je Pfad monoton (B-1). |
| R9 Feinheit 2 | — | `generation_waechst_ueber_den_neuanlauf` und `ack_einer_alten_generation_gibt_keine_bank_frei` (B-12); Rotbeweis B-12. |

#### 9.9.1 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Nacharbeit 1

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Der ACK-Ring hat seine Politik bei *voll* jetzt am erzwungenen Überlauf gemessen: Maskenbit gesetzt, Slot bleibt `ausgedient`, erst die Bestätigung gibt frei (B-17, E-23). Der Sperrzähler ist keine strukturelle Null mehr: der Wrapper `RtWache::GemeldeteSperre` meldet jede Sperrnahme, ein Lebendigkeitsfall zählt genau eine, ein Textriegel hält jede Sperrklasse am Wrapper vorbei fern (B-13). |
| **B** Lebenszyklus | `uebernehmeZustand(candidate)` und `beendeCandidate` sind gekoppelt über Fade, ACK und Ernte (B-8); Einschalten und Ausschalten laufen beide über den Blockrand, Ausschalten ohne Bank (B-10, E-18); `freigeben` und `bereiteVor` lassen den Generationszähler stehen (B-12). Jeder Übergang endet, bevor der nächste beginnt (E-17). |
| **C** Verträge und Längen | Nicht-endliche Werte an beiden Riegeln mit exaktem Zähler: Filterzustand am Blockrand (B-20), Auslenkung exakt 0,0 (B-21). Die Wire-Form bleibt Etappe 2. |
| **D** Bau- und Prüfriegel | Jeder Rotbeweis baut unter Mutation und nach Rücknahme neu und prüft, dass das Binary jünger als die Quelle ist; ein älteres Binary bricht den Lauf ab (Erzeuger im Job-Verzeichnis, Rohdateien je Befund). Der Textriegel aus B-13 ist fail-closed: fehlt das Verzeichnis oder sind es weniger als neun Dateien, ist er rot. |
| **E** Behauptung ≤ Messung | Die Runner-Behauptung zu B6 nennt für Mix 0 die Bedingung Output-Trim 0 dB und behauptet dort keinen Schreibverzicht (B-26, Nachweis `SONDE-015-rot-B-26-e3-n1.txt`). Drei Testnamen, die mehr sagten als sie maßen, heißen jetzt nach ihrer Messung (`input_trim_wirkt_mit_seinem_gesamtgain`, `auto_gain_steht_vor_dem_ersten_block_fest`, `ueberlaeufe_und_verriegelungen_sind_gekoppelt`). Jede neue Prüfung wurde einmal gebrochen (Tabelle oben). Zahlen dieses Abschnitts sind gemessen (§9.9.3). |
| **F** Änderungssatz | Kern, B6, Runner-Behauptung und Manifest liegen im selben Änderungssatz; `publiziere` ↔ `uebernehme`, `beginneVerblassen` ↔ `meldeAusgedient` ↔ `ernteAcks` und `publiziereEnde` ↔ Übernahme der ENDE-Marke sind je paarweise gebaut und gemessen. |

#### 9.9.2 Nebenbefunde dieser Runde (nicht ins Register)

| Nr. | Befund | Wo gefunden | Was geschah |
|---|---|---|---|
| N-9 | **Die exakte Mitte eines Notch ist eine Nullstelle.** Mit B-24 kam sie ins Gitter; die Referenz liegt dort bei −300 dB, die Impulsantwort an ihrem numerischen Boden, und der dB-Abstand riss 4,5 bis 147 dB. | Erster Lauf nach dem Gitterausbau. | E-27: unter −60 dB Referenz muss auch der gemessene Wert unter −60 dB liegen; darüber gelten die R15-Toleranzen unverändert. |
| N-10 | **Drei Bankpool-Fälle publizierten ohne Generation** (M-43, M-44 und die Kopplung aus M-45). Seit E-15 ist ein Programm ohne Generation nicht publizierbar; die Fälle liefen danach ins Leere, der M-45-Lauf erreichte `ausgedient` nie. | Erster Lauf nach dem Umbau des Pools. | Die Fälle setzen die Generation wie der Kern (`naechsteGeneration`); die Zusagen sind unverändert. |
| N-11 | **Ein neutraler Band-Biquad nach einer Rampe auf 0 dB trug noch Zustand.** Das Überspringen neutraler Bänder setzte einen leeren Zustand voraus, den nur eine kalte Bank garantiert; nach E-19 kann ein Band mit Restzustand neutral werden, und ein späterer Rampenübergang begänne mit diesem Rest. | Selbstaudit vor den Rotbeweisen. | Übersprungen wird erst, wenn der Zustand leer ist — der neutrale Biquad spült ihn in genau zwei Samples auf exakt 0. |
| N-12 | **Beim Ausschalten liefen die Rampen der ausblendenden Bank auf neutral.** Der Fade zum Eingang wurde dadurch steiler als die Fadeschrittweite (0,0081 gegen 0,0035 bei Delta), während derselbe Übergang über Hard-Bypass genau im Fadeschritt blieb. | B-3-Fall „delta + eq_enabled aus". | Die ENDE-Marke setzt keine neuen Rampenziele; die Bank klingt mit ihren Gains aus (0,0034714 gegen 0,0034713). |
| N-13 | **Die Rotbeweisspalte von M-30 ist mit `double` nicht erfüllbar.** „`width` = 1,0 liefert ein LSB Abweichung, weil die Matrix immer gerechnet wird" — die Hin- und Rückrechnung ist für Float-Eingänge in `double` bitgenau. | §9.8 (Quellencheck des Dirigenten), bestätigt durch Rotbeweis B-19a: ohne Kurzschluss bleibt `width_eins_ist_bitidentisch` grün. | Zwei Zusagen mit je einem Rotbeweis (B-19a, B-19b); §3 bleibt unverändert. |
| N-14 | **Die Begründung von E-9 war falsch** — zwei Bänke je Pfad reichen. | §9.8. | Berichtigt in §9.2 (E-9, Berichtigung), gebaut nach E-16 und E-17. |
| N-15 | **Drei Rotbeweise der ersten Fassung fielen an einem Rampenwechsel statt an einem Crossfade.** Seit E-19 ist ein Wechsel nur eines `rampe`-Werts kein Crossfade mehr; die Fälle zu B-5, B-7 und B-8 wechselten nur `gain_db` und fuhren damit gar keine verblassende Bank. Der Rotbeweis zu B-7 blieb grün und hat es gezeigt. | Rotbeweis B-07, erster Lauf. | Die drei Fälle wechseln jetzt topologisch (Typ oder Kanalmodus). Danach wurden **alle** Rotbeweise am endgültigen Stand von Kern und B6 neu gefahren; die Rohdateien tragen nur diesen Lauf. |
| N-16 | **Die Kniemutation aus B-14 reißt auch B-15 und B-16a.** Beide benutzen das lineare Knie als Messinstrument (1 dB Leistung gleich 1 dB Auslenkung). | Rotbeweis B-14. | Deklariert in der Rohdatei; der Plateaufall bleibt unter derselben Mutation grün. |
| N-17 | **Der erste B-6-Fall maß im Einschwingen.** Ein 20-Hz-Shelf braucht nach einem DC-Sprung Zehntelsekunden; gemessen wurde nach 10 ms. | Erster Lauf. | Der Fall fährt 32768 Samples DC vor der Messung. |

#### 9.9.3 Die Beine dieser Runde

Einzeln aus pwsh gebaut und gefahren; jedes Binary ist jünger als die jüngste geänderte Quelle. A14 erzeugt `NakamaKern.lib` neu, die vier folgenden Ziele wurden danach neu gelinkt. Rohausgabe: `docs/beweise/roh/SONDE-015-etappe3-nacharbeit1-beine.txt`.

| Bein | Exit | Binary frisch | Letzte Zeile |
|---|---|---|---|
| **B6** | 0 | ja | DSP-GOLDEN OK |
| **A14** | 0 | ja | 25 ok, 0 Fehler |
| **B5** | 0 | ja | Ergebnis: 248 bestanden, 0 Fehler. |
| **A16** | 0 | ja | SONDE-NULLTEST OK - 87 Pruefungen ok, 0 Fehler |
| **B2** | 0 | ja | STATE-MIGRATION-TEST OK - 247 Pruefungen ok, 0 Fehler |
| **B3c** | 0 | ja | 152 bestanden, 0 gescheitert |

### 9.11 Nacharbeit 2 — was eingearbeitet wurde (10.09.2026)

| Merkmal | Wert |
|---|---|
| Auftrag | `docs/beweise/roh/SONDE-015-etappe-3-nacharbeit-2-auftrag.txt` — die vier bestätigten Defekte W-1, W-2, W-3, W-5 und die vom Dirigenten entschiedene Lücke W-4 aus der Wiederprüfung 1 (§9.10), je mit schließender Regel |
| Basis-SHA | `5f4f72c804770534c0412735f277f5653b86de9f`, mit `git rev-parse HEAD` beim Start gemessen; `git status --short` war leer |
| Urteil | **Offen.** Die Wiederprüfung 2 steht aus. |
| Entscheide | E-28 bis E-32 und die Berichtigung von E-19 (§9.2); E-31 ist der Dirigenten-Entscheid zu W-4 mit Quelle §9.10 |
| Beine | **6** gefahren, Rohausgabe `docs/beweise/roh/SONDE-015-etappe3-nacharbeit2-beine.txt` (§9.11.3) |
| Rotbeweise | **6** Dateien dieser Runde (`ls docs/beweise/roh/SONDE-015-rot-W-*-e3-n2.txt`): je Punkt einer, dazu W-5b für die Wartezusage aus E-30; alle am Endstand von Kern und B6 gefahren; insgesamt **133** Rotbeweisdateien des Tickets (`ls docs/beweise/roh/SONDE-015-rot-*`) |
| Kanon | **Nicht gefahren** — gehört an das Ende der Etappe 4. |
| Prozessor | **Unberührt.** `eq-copilot/plugin/sonde/SondeProcessor.{h,cpp}`, alles aus Etappe 2, `tools/beweise.ps1` und `eq-copilot/plugin/CMakeLists.txt` sind nicht im Diff. |

| Punkt | Was gebaut wurde | Trägerzeile (Datei:Zeile) | Test in B6 | Rotbeweis | Status |
|---|---|---|---|---|---|
| W-1 | Schreibgrenze am ersten Sample mit Crossfadegewicht 1, auch mitten im Teilstück; danach ruht die Hörmatrix (E-32) | `eq-copilot/plugin/dsp/DspKern.cpp:923` | `schreibverzicht_beginnt_exakt_am_fade_ende` je Processed und Delta über EQ aus und Hard-Bypass (`DspGoldenTestMain.cpp:1029`): 512-Sample-Block, Fade in 0 bis 255, sNaN-Wachmarken an 256, 300 und 511, Rohpuffervergleich der Samples 256 bis 511 | `SONDE-015-rot-W-1-e3-n2.txt` | geschlossen — rot unter Mutation (drei geänderte Samples je Fall), grün nach Rücknahme; die alten B-3-Fälle bleiben unter der Mutation grün |
| W-2 | `mische` für Hüllkurvenkoeffizienten, im Rampenübergang je Sample angewandt (E-28) | `eq-copilot/plugin/dsp/DspKern.cpp:439` | `huellkurvenwert_attack_ms_`, `_hold_ms_`, `_release_ms_rampt_ohne_zustandsreset` (`DspGoldenTestMain.cpp:2042`): Leistung, Auslenkung je Steuerschritt und Holdzähler gegen die ausgeschriebene Idealrampe | `SONDE-015-rot-W-2-e3-n2.txt` | geschlossen — alle drei Fälle rot unter Sofortübernahme, grün nach Rücknahme |
| W-3 | Detektor läuft im Rampenübergang, solange ein Programm ihn laufen lässt; Entwurf auch bei Range 0; Nullen des stehenden Detektors (E-29) | `eq-copilot/plugin/dsp/DspKern.cpp:414`, `eq-copilot/plugin/dsp/DspProgramm.cpp:253`, `eq-copilot/plugin/dsp/DspKern.cpp:495` | `range_minus_12_nach_0_` und `range_0_nach_minus_12_rampt_ueber_die_volle_rampe` (`DspGoldenTestMain.cpp:2098`): Auslenkung an 48 Steuerschritten gegen die Idealrampe, danach Detektor aus mit Leistung exakt 0 beziehungsweise an | `SONDE-015-rot-W-3-e3-n2.txt` | geschlossen — rot am ersten Steuerschritt (0 statt −11,953125 dB), grün nach Rücknahme; M-20 bleibt unter der Mutation grün |
| W-4 | Messung am Umschaltsample mit Rundungstoleranz 2^−23, Wache mit hergeleiteter Schranke `kaltSchranke` (E-31); Faktor 4 entfernt | `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:482`, `:496`, `:1070`, `:3569`; Zusage im Kern `eq-copilot/plugin/dsp/DspKern.cpp:682` | `zweiter_wechsel_im_laufenden_fade_springt_nicht` und `wache_der_kalt_startenden_bank_ueber_den_ganzen_lauf` (B-5, `:1070`, `:1080`), `candidate_wechsel_blendet_aus_der_bisherigen_candidate_bank` und `wache_der_kalt_startenden_candidate_bank_ueber_den_ganzen_lauf` (B-7, `:3569`, `:3583`) | `SONDE-015-rot-W-4-e3-n2.txt` | geschlossen — Fade startet unter Mutation bei t = 1, beide Umschaltsample-Prüfungen rot, grün nach Rücknahme |
| W-5 | Rückwechsel zur Quelle des laufenden Hörmatrix-Fades kehrt die Richtung vom aktuellen Mischstand um (E-30) | `eq-copilot/plugin/dsp/DspKern.cpp:865` | `candidate_abbruch_im_hoermatrix_fade_blendet_vom_mischstand_zurueck` (`DspGoldenTestMain.cpp:3667`): Committed neutral, Candidate gewählt, Abbruch nach 64 Samples | `SONDE-015-rot-W-5-e3-n2.txt` | geschlossen — alter Zweig rot am Abbruchsample (0,0330 gegen Fadeschritt 0,0021), grün nach Rücknahme |
| W-5b | Wechsel zu einem dritten Hörmatrix-Zustand wartet das Fade-Ende ab (E-30) | `eq-copilot/plugin/dsp/DspKern.cpp:868` | `hoermatrix_wechsel_zu_drittem_zustand_wartet_das_fade_ende_ab` (`DspGoldenTestMain.cpp:3462`) | `SONDE-015-rot-W-5b-e3-n2.txt` | geschlossen — rot unter dem wieder eingefügten alten Zweig, grün nach Rücknahme |

**Korrekturen an Zeilen der Messabdeckung.** §9.3 und die Korrekturtabelle in §9.9 bleiben stehen; was sich an einer ihrer Zeilen geändert hat, steht hier.

| ID | bisher | jetzt |
|---|---|---|
| M-03, M-06 | `zweiter_wechsel_im_laufenden_fade_springt_nicht` mit vierfacher Fadeschrittweite über den ganzen Lauf | gemessen am Umschaltsample gegen Fadeschrittweite plus 2^−23; die Wache über den Lauf heißt `wache_der_kalt_startenden_bank_ueber_den_ganzen_lauf` und trägt die hergeleitete Schranke (W-4, E-31); Rotbeweis W-4. |
| M-05 | Wachmarken erst in Folgeblöcken | zusätzlich `schreibverzicht_beginnt_exakt_am_fade_ende` mit Wachmarken hinter dem Fade-Ende im selben Teilstück (W-1); Rotbeweis W-1. |
| M-17 | `bandwert_gain_db_`, `bandwert_freq_hz_`, `bandwert_q_rampt_ohne_zustandsreset` | zusätzlich die drei Hüllkurvenwerte (W-2) und beide Range-Richtungen über 0 (W-3); Rotbeweise W-2 und W-3. |
| M-46 | `candidate_ende_blendet_aus_und_gibt_beide_baenke_frei` mit eingeschwungener Hörmatrix | zusätzlich `candidate_abbruch_im_hoermatrix_fade_blendet_vom_mischstand_zurueck` (W-5); Rotbeweis W-5. |
| M-55 | `hoermatrixwechsel_ist_klickfrei` mit einem einzelnen Wechsel | zusätzlich `hoermatrix_wechsel_zu_drittem_zustand_wartet_das_fade_ende_ab` (E-30); Rotbeweis W-5b. |
| M-56, §3.0 | `candidate_wechsel_blendet_aus_der_bisherigen_candidate_bank` mit 0,01 für dieselbe Kurve und vierfacher Fadeschrittweite | am Umschaltsample: dieselbe Kurve höchstens 2^−23, andere Kurve höchstens Fadeschrittweite plus 2^−23; Wache `wache_der_kalt_startenden_candidate_bank_ueber_den_ganzen_lauf` (W-4, E-31); Rotbeweis W-4. |

#### 9.11.1 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Nacharbeit 2

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Keine Politik geändert. Der Candidate-Abbruch aus W-5 prüft nebenbei, dass der reguläre ACK-Weg genau eine Bank erntet und danach nur die Committed-Bank belegt ist. |
| **B** Lebenszyklus | `beendeCandidate` ↔ Hörmatrix-Rückblende vom Mischstand (W-5), Ausschalten ↔ Schreibende exakt am Fade-Ende (W-1), Range an ↔ aus in beiden Richtungen mit Detektor danach aus beziehungsweise an (W-3); ein Hörmatrix-Wechsel endet vor dem nächsten (E-30, W-5b). |
| **C** Verträge und Längen | Die Vertragsspalte `wechsel = rampe` aus `nakama-parameter-v1.json` gilt jetzt auch für `attack_ms`, `hold_ms`, `release_ms` und den Übergang von `dynamic_range_db` über 0 (W-2, W-3). Keine Wire- oder Schemaänderung. |
| **D** Bau- und Prüfriegel | Jeder Rotbeweis baut unter Mutation und nach Rücknahme neu und bricht ab, wenn das Binary älter als die Quelle ist. Die Beine laufen mit Frischespalte gegen die jüngste Quelle; A14 erzeugt `NakamaKern.lib` neu, die übrigen Ziele werden danach neu gelinkt (§9.11.3). |
| **E** Behauptung ≤ Messung | Jede Messgröße ist benannt und hergeleitet statt gesetzt: Idealrampe der Hüllkurve (E-28), Plateau der Range (E-29), Umschaltsample mit 2^−23 und `kaltSchranke` (E-31). Der Detailtext des Release-Falls meldet den Holdzähler als ungemessen, statt 0 zu zeigen. Die Wartezusage aus E-30 bekam einen eigenen Fall, weil sie sonst nur behauptet wäre. Jede neue Prüfung wurde einmal gebrochen (Tabelle oben). Die Runner-Behauptung zu B6 sagt nichts über diese Punkte und bleibt unverändert. |
| **F** Änderungssatz | Kern (Commit `8a8aa1b7`), B6, Rotbeweise und Manifest in dieser Runde; der Entwurf im Worker (`DspProgramm.cpp:253`) und seine Interpolation im Audiothread (`DspKern.cpp:414`, `:439`) liegen im selben Satz. |

#### 9.11.2 Nebenbefunde dieser Runde (nicht ins Register)

| Nr. | Befund | Wo gefunden | Was geschah |
|---|---|---|---|
| N-18 | **Vier weitere Klickproben tragen noch den Faktor 4:** `bypasswechsel_ist_klickfrei` (M-06), `hoermatrix_blendet_bei_aus_klickfrei_auf_den_eingang` (B-3), `hoermatrixwechsel_ist_klickfrei` (M-55) und `candidate_ende_blendet_aus_und_gibt_beide_baenke_frei` (B-8). Ihre Messwerte liegen zwischen dem 1,00- und dem 1,20-Fachen der Fadeschrittweite. | Selbstaudit beim Umbau von B-5 und B-7. | Nicht geändert — REGEL W-4 gilt ausdrücklich für den Committed- und den Candidate-Fall aus B-5 und B-7. Zur Kenntnis für die Wiederprüfung 2. |
| N-19 | **Die Wartezusage aus E-30 war zunächst ungemessen.** Der alte Zweig ließ bei einem Wechsel zu einem dritten Zustand das Ziel wandern und sprang; die neue Struktur wartet das Fade-Ende ab, ohne dass ein Fall es prüfte. | Selbstaudit des Kerndiffs nach den ersten fünf Rotbeweisen. | Fall `hoermatrix_wechsel_zu_drittem_zustand_wartet_das_fade_ende_ab` mit Rotbeweis W-5b; danach wurden alle sechs Rotbeweise am Endstand neu gefahren. |
| N-20 | **Die Mutation zu W-4 (Fade startet bei t = 1) reißt 17 Prüfungen,** darunter W-1 und W-5. Alle messen Klickfreiheit eines Übergangs, dessen erstes Sample das Fadegewicht trägt. | Rotbeweis W-4. | Deklariert in der Rohdatei; die beiden erwarteten Prüfungen sind die Umschaltsample-Prüfungen aus W-4. |
| N-21 | **Die Umgebungsvariable des Job-Verzeichnisses zeigte im Bash-Werkzeug auf ein anderes Verzeichnis als das der Sitzung.** Der erste Rotbeweislauf fand deshalb seine Mutationsdatei nicht und brach vor jeder Mutation ab. | Erster Rotbeweislauf. | Keine Repo-Änderung; die Läufe wurden mit absoluten Pfaden wiederholt. |

#### 9.11.3 Die Beine dieser Runde

Einzeln aus pwsh gebaut und gefahren, nach dem letzten Rotbeweislauf; jedes Binary ist jünger als die jüngste Quelle (`DspKern.cpp`, zuletzt vom Rotbeweis-Erzeuger bei der Rücknahme berührt). A14 erzeugt `NakamaKern.lib` neu, die vier folgenden Ziele wurden danach neu gelinkt. B6 fährt **240** Prüfungen mit 0 Fehlern. Rohausgabe: `docs/beweise/roh/SONDE-015-etappe3-nacharbeit2-beine.txt`.

| Bein | Exit | Binary frisch | Letzte Zeile |
|---|---|---|---|
| **B6** | 0 | ja | DSP-GOLDEN OK |
| **A14** | 0 | ja | 25 ok, 0 Fehler |
| **B5** | 0 | ja | Ergebnis: 248 bestanden, 0 Fehler. |
| **A16** | 0 | ja | SONDE-NULLTEST OK - 87 Pruefungen ok, 0 Fehler |
| **B2** | 0 | ja | STATE-MIGRATION-TEST OK - 247 Pruefungen ok, 0 Fehler |
| **B3c** | 0 | ja | 152 bestanden, 0 gescheitert |

### 9.12 Wiederprüfung 2 Etappe 3 — NEEDS_WORK (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08b85-b6a3-7301-add9-4aed2cfa2c36`; Lauf 15:33–15:42 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff 5f4f72c8...00627114 -- eq-copilot/plugin/dsp eq-copilot/plugin/tests/DspGoldenTestMain.cpp eq-copilot/plugin/CMakeLists.txt tools/beweise.ps1`; HEAD während des Laufs `fda96533` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-3-wiederpruefung-2-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-3-wiederpruefung-2-0062711.txt` |
| Urteil | **NEEDS_WORK** — W-1 bis W-4 geschlossen (je Beleg und Rotbeweis mit einer Mutation); W-5 schließt die ursprüngliche Reproduktion, lässt aber den Nullvergleich identischer Zustände beim Candidate-Abbruch offen; sonst nichts gebrochen |
| Workerausfall dieser Runde | Der erste Nacharbeits-Worker `17561b95` endete nach 55 Minuten ohne Dateiänderung mit dem API-Fehler „response exceeded the 64000 output token maximum"; der Dirigent setzte ihn als Kopie `b913d829` mit demselben Auftrag und der Anweisung „kleine Schritte, je Aufruf rund 250 Zeilen" fort. Die Kopie erbte den gelesenen Kontext und lieferte in drei Commits (`8a8aa1b7`, `1018b746`, `00627114`) |
| Quellencheck des Dirigenten | lesender Opus-Agent: **DEFEKT gegen M-55**, vorbestehend. Der Zweig `kEnde` in `blockrand` setzt den Candidate nach dem Abbruch auf einen Crossfade nach Dry, die Hörrückblende liest genau diesen wandernden Puffer; nachgerechnet Ausgang(j) = P + (D−P)·j·(63−j)/256², Spitze 0,0045189 bei j = 31 — Zahl des Prüfers exakt bestätigt. Derselbe Doppelfade liegt im alten Zweig (Abbruch nach abgeschlossenem Fade, Spitze 0,0746) schon auf `5f4f72c8`: der Fix hat den Bruch nicht eingeführt, aber nicht geschlossen. REGEL W-5 selbst (Sprung ≤ Fadeschritt) ist erfüllt; gebrochen ist M-55 „ein A/B-Wechsel zwischen zwei identischen Zuständen nullt innerhalb numerischer Toleranz". Lücke daneben: W-5-Test und M-55-Nulltest decken den Fall „Committed = Candidate" nicht ab |
| Rundenbilanz Nacharbeit 2 | `5f4f72c8..00627114: Produkt 3 Datei(en) +96/-18 · Tests 1 Datei(en) +336/-9 · Doku 8 Datei(en) +1426/-3` — Runde mit Produktfortschritt, kein Konvergenz-Signal |
| Nacharbeit | Etappe 3, Nacharbeit 3 (Runde 3 von 3, Budget erschöpft danach): `docs/beweise/roh/SONDE-015-etappe-3-nacharbeit-3-auftrag.txt` — der eine Defekt X-1 mit der schließenden Regel (Hörrückblende ist die einzige Blende, Bank erst danach ausgedient, Nullvergleich bei identischen Zuständen für beide Abbruchzeitpunkte); nur betroffene Beine (B6, A14, B5, A16, B2, B3c) |

| Befund | Kurzform | Einordnung | Ansatzpunkt |
|---|---|---|---|
| X-1 | W-5: Candidate-Bank fadet beim Abbruch parallel nach Dry, während die Hörrückblende sie noch mischt; identische A/B-Zustände nullen nicht | DEFEKT (M-55, M-46, REGEL B-8), vorbestehend | `eq-copilot/plugin/dsp/DspKern.cpp:277-286`, `:858-866`, `:887`; B6 `:3636-3672` |

### 9.13 Nacharbeit 3 — was eingearbeitet wurde (10.09.2026)

| Merkmal | Wert |
|---|---|
| Auftrag | `docs/beweise/roh/SONDE-015-etappe-3-nacharbeit-3-auftrag.txt` — der eine bestätigte Defekt X-1 aus der Wiederprüfung 2 (§9.12) mit schließender Regel |
| Basis-SHA | `72851d299bd65429b6e99cba5fa9219a71b7b899`, mit `git rev-parse HEAD` beim Start gemessen; `git status --short` war leer |
| Urteil | **Offen.** Die Wiederprüfung 3 steht aus. |
| Entscheide | E-33 (Hörhalt, Lesart „mischt Candidate-Anteile") und E-34 (das Gegenstück beim Einblenden), §9.2 |
| Beine | **6** gefahren, Rohausgabe `docs/beweise/roh/SONDE-015-etappe3-nacharbeit3-beine.txt` (§9.13.3) |
| Rotbeweise | **3** Dateien dieser Runde (`ls docs/beweise/roh/SONDE-015-rot-X-1*-e3-n3.txt`): X-1 an der Zusage der Regel, X-1r für das Ausdienen nach der Rückblende, X-1b für E-34; alle am Endstand von Kern und B6 gefahren, jede mit bytegleicher Rücknahme (SHA-256 in der Datei); insgesamt **136** Rotbeweisdateien des Tickets (`ls docs/beweise/roh/SONDE-015-rot-*`) |
| Kanon | **Nicht gefahren** — gehört an das Ende der Etappe 4. |
| Prozessor | **Unberührt.** `eq-copilot/plugin/sonde/SondeProcessor.{h,cpp}`, alles aus Etappe 2, `tools/beweise.ps1` und `eq-copilot/plugin/CMakeLists.txt` sind nicht im Diff. |

| Punkt | Was gebaut wurde | Trägerzeile (Datei:Zeile) | Test in B6 | Rotbeweis | Status |
|---|---|---|---|---|---|
| X-1 | Übergang `hoerHalt` (E-33): mischt die Hörmatrix beim Nehmen der Candidate-ENDE-Marke noch Candidate-Anteile, klingt die endende Bank ohne Mischgewicht unverändert weiter — kein paralleler Fade nach Dry | `eq-copilot/plugin/dsp/DspKern.cpp:315`; Klang im Halt `:690`, kein Ende über den Rest `:733` | `candidate_abbruch_bei_identischen_zustaenden_nullt` (a) im Einblenden, (b) nach abgeschlossenem Fade, (c) im Ausblenden (`DspGoldenTestMain.cpp:3733`): Committed und Candidate je Output-Trim +6 dB, eingeschwungen, DC 0,3, Block 64, 48 kHz; jedes Sample ab der Wahl gegen Processed, Toleranz 2^−23 | `SONDE-015-rot-X-1-e3-n3.txt` | geschlossen — unter der Mutation (paralleler Fade wieder aktiv) rot mit 0,004519463 (a), 0,074644625 (b) und 0,041987598 (c), dazu der deklarierte W-5-Fall; die Lebenszyklusfälle bleiben grün; nach Rücknahme grün mit Abweichung 0 in allen drei Fällen |
| X-1r | Ausdienen erst nach der Rückblende über den regulären Weg: `beendeHoerHalt` meldet die gehaltene Bank mit `meldeAusgedient` aus, sobald der Hörmatrix-Zustand eines Stücks keine Candidate-Anteile mehr enthält | `eq-copilot/plugin/dsp/DspKern.cpp:274`; gerufen in `:885` und `:998` | `candidate_bank_klingt_bis_zum_ende_der_rueckblende_und_dient_dann_aus` (a), (b), (c) (`DspGoldenTestMain.cpp:3738`): 32 Samples nach dem Abbruch ist die Bank gehalten, verblassend und ohne ACK; nach weiteren 2048 Samples genau ein ACK geerntet, eine belegte Bank (Committed), beide Candidate-Slots −1 | `SONDE-015-rot-X-1r-e3-n3.txt` | geschlossen — ohne `meldeAusgedient` rot in allen drei Fällen (geerntet 0, belegt 2), dazu die deklarierten Fälle W-5 und B-8 mit derselben ACK-Zusage; die Nullvergleiche bleiben grün; nach Rücknahme grün |
| X-1b | E-34, das Gegenstück beim Einblenden: ein Candidate, der aus der Ruhe einblendet, ist für die Hörmatrix noch nicht wirksam; sie blendet erst danach auf Candidate | `eq-copilot/plugin/dsp/DspKern.cpp:798`, Bedingung in `:799` | `candidate_einblendung_bei_identischen_zustaenden_nullt` (`DspGoldenTestMain.cpp:3773`): Auswahl Candidate vor dem Candidate, dann ein Candidate gleich dem Committed; jedes Sample über 2304 Samples gegen Processed, am Ende ist die Auswahl wirksam | `SONDE-015-rot-X-1b-e3-n3.txt` | geschlossen — unter der Mutation rot mit 0,114533335, alle übrigen Candidate-Fälle grün; nach Rücknahme grün |

**Korrekturen an Zeilen der Messabdeckung.** §9.3 und die Korrekturtabellen in §9.9 und §9.11 bleiben stehen; was sich an einer ihrer Zeilen geändert hat, steht hier.

| ID | bisher | jetzt |
|---|---|---|
| M-46 | `candidate_ende_blendet_aus_und_gibt_beide_baenke_frei` und `candidate_abbruch_im_hoermatrix_fade_blendet_vom_mischstand_zurueck` (§9.11) | zusätzlich `candidate_bank_klingt_bis_zum_ende_der_rueckblende_und_dient_dann_aus` für drei Abbruchzeitpunkte (X-1); im W-5-Fall blendet der Candidate jetzt vor der Wahl ein (`DspGoldenTestMain.cpp:3653`), damit sein Abbruch weiter im Hörmatrix-Fade liegt (N-23); Rotbeweise X-1 und X-1r. |
| M-55 | `hoermatrixwechsel_ist_klickfrei`, `identische_zustaende_nullen_beim_wechsel` und `hoermatrix_wechsel_zu_drittem_zustand_wartet_das_fade_ende_ab` | zusätzlich der Nullvergleich identischer A/B-Zustände beim Candidate-Abbruch (a), (b), (c) und beim Einblenden eines Candidate (E-34); Rotbeweise X-1 und X-1b. |
| M-56 | `candidate_ohne_kandidat_faellt_sichtbar_zurueck` und `mit_kandidat_greift_die_auswahl` | Der sichtbare Rückfall gilt auch, solange ein Candidate aus der Ruhe einblendet (E-34); `mit_kandidat_greift_die_auswahl` bleibt grün, auch unter der Mutation X-1b. |

#### 9.13.1 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Nacharbeit 3

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Keine Politik geändert. Der Hörhalt reiht keinen zusätzlichen ACK ein: je Candidate-Bank bleibt es genau einer (X-1r, geerntet 1 je Fall); Ringgrenze und Maske aus M-45 laufen unverändert über `meldeAusgedient`. |
| **B** Lebenszyklus | `beendeCandidate` ↔ Hörmatrix-Rückblende: die Bank bleibt bis zu deren Ende gehalten und dient erst dann aus, zu drei Abbruchzeitpunkten (X-1, X-1r); Candidate-Einblenden ↔ Hörmatrix-Einblenden im selben Änderungssatz (E-34, X-1b). Der Rückweg `bereiteVor` und `freigeben` setzt einen laufenden Hörhalt über `PfadZustand::ruhe` zurück (`DspKern.cpp:83`); eine Publikation während des Halts wartet wie in E-17. |
| **C** Verträge und Längen | Keine Wire-, Schema- oder Parameteränderung. |
| **D** Bau- und Prüfriegel | Jeder Rotbeweis baut unter Mutation und nach Rücknahme neu und hält in der Datei fest, dass das Binary jünger als die Quelle ist und die Rücknahme bytegleich (SHA-256). Die Beine laufen mit Frischespalte gegen die jüngste Quelle (§9.13.3). |
| **E** Behauptung ≤ Messung | Jede neue Prüfung wurde einmal gebrochen: die drei Nullvergleiche unter X-1, die drei Lebenszyklusfälle unter X-1r, der Einblendfall unter X-1b. Der Hörhalt fügt dem Callback keinen Aufruf hinzu, der allokieren oder sperren könnte: er rechnet über `verarbeiteBank` und endet über `meldeAusgedient`, beide im M-47-Lauf unter Allokations- und Sperrzähler; den Halt selbst fährt dieser Lauf nicht, weil seine Hörmatrix auf Processed steht. Die Runner-Behauptung zu B6 nennt weder den Nullvergleich identischer A/B-Zustände noch das Candidate-Ende und bleibt unverändert. `git grep -n -e hoerHalt -e candEinblendend -e hoermatrixMischtCandidate -- eq-copilot tools` trifft nur `DspKern.h` und `DspKern.cpp`. |
| **F** Änderungssatz | Kern und B6 in Commit `2c98b075`; Rotbeweise, Beine und Manifest in dieser Runde. |

#### 9.13.2 Nebenbefunde dieser Runde (nicht ins Register)

| Nr. | Befund | Wo gefunden | Was geschah |
|---|---|---|---|
| N-22 | **Das Einblenden eines Candidate trug denselben Doppelfade wie sein Abbruch.** Stand die Auswahl schon auf Candidate, blendete die Hörmatrix im selben Stück auf Candidate, in dem der Candidate-Pfad selbst von Dry her einblendete; bei identischen Zuständen wich der Ausgang um 0,114533335 ab (Rotlauf X-1b). Kein bestehender Fall maß das. | Selbstaudit beim Bau von X-1, Gegenstück aktivieren und abklingen (`CLAUDE.md` „Beziehungen mitprüfen", `AGENTS.md:48-50`) | Geschlossen als E-34 mit eigenem Fall und Rotbeweis X-1b. |
| N-23 | **Der W-5-Fall aus Nacharbeit 2 läge mit E-34 nicht mehr im Hörmatrix-Fade.** Er publizierte und wählte den Candidate im selben Moment und brach nach 64 Samples ab; mit E-34 steht die Hörmatrix dann noch auf Processed, und der Fall bestünde ohne Aussage. Mit eingeblendetem Candidate zeigt er den Doppelfade als Sprung 0,002647012 gegen die Fadeschrittweite 0,002130918 (Rotlauf X-1). | Durchsicht der bestehenden Candidate-Fälle gegen E-34 | Der Candidate blendet vor der Wahl 1024 Samples ein (`DspGoldenTestMain.cpp:3653`); der Fall fällt unter der Mutation X-1 (deklariert) und ist nach Rücknahme grün. |
| N-24 | **REGEL X-1 nennt zwei Abbruchzeitpunkte; ihr erster Satz deckt einen dritten.** Endet der Candidate, während die Hörmatrix schon von Candidate zurück auf Processed blendet, mischt sie weiter Candidate-Anteile; der alte Zweig wich dort um 0,041987598 ab (Rotlauf X-1). | Herleitung der Bedingung „mischt Candidate-Anteile" | Die Bedingung umfasst ihn (E-33), Fall (c) misst ihn, der Rotbeweis X-1 enthält ihn. |

#### 9.13.3 Die Beine dieser Runde

Einzeln aus pwsh gebaut und gefahren, nach dem letzten Rotbeweislauf; jedes Binary ist jünger als die jüngste Quelle (`DspKern.cpp`, zuletzt von der bytegleichen Rücknahme des Rotbeweises X-1b berührt). A14 erzeugt `NakamaKern.lib` neu, die vier folgenden Ziele wurden danach neu gelinkt. B6 fährt **247** Prüfungen mit 0 Fehlern. Rohausgabe: `docs/beweise/roh/SONDE-015-etappe3-nacharbeit3-beine.txt`.

| Bein | Exit | Binary frisch | Letzte Zeile |
|---|---|---|---|
| **B6** | 0 | ja | DSP-GOLDEN OK |
| **A14** | 0 | ja | 25 ok, 0 Fehler |
| **B5** | 0 | ja | Ergebnis: 248 bestanden, 0 Fehler. |
| **A16** | 0 | ja | SONDE-NULLTEST OK - 87 Pruefungen ok, 0 Fehler |
| **B2** | 0 | ja | STATE-MIGRATION-TEST OK - 247 Pruefungen ok, 0 Fehler |
| **B3c** | 0 | ja | 152 bestanden, 0 gescheitert |

### 9.14 Wiederprüfung 3 Etappe 3 — PASS; Etappe 3 abgeschlossen, Übergabe (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08bb8-6d6b-7571-8c50-2273e49fe237`; Lauf 16:28–16:33 |
| Prüfbereich | Wiederprüfung (Vorlage B) über den Fixdiff `git diff 72851d29...156445a8 -- eq-copilot/plugin/dsp eq-copilot/plugin/tests/DspGoldenTestMain.cpp eq-copilot/plugin/CMakeLists.txt tools/beweise.ps1`; HEAD während des Laufs `ea08dc5d` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-3-wiederpruefung-3-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-3-wiederpruefung-3-156445a.txt` |
| Urteil | **PASS** — X-1 geschlossen (`DspKern.cpp:315-317`, `:690-695` verhindern den parallelen Dry-Fade, `:266-277` und `:998` dienen danach regulär aus; B6 `:3692-3743` prüft identische +6-dB-Zustände für beide Abbruchzeitpunkte samt Bankfreigabe und ACK-Ernte; Rotbeweis X-1 mit genau einer Mutation, Rot am Nullvergleich, Grün nach Rücknahme; ein erneuter Candidate wartet über die Übergangssperre bis nach dem Hörhalt); nichts gebrochen |
| Rundenbilanz Nacharbeit 3 | `72851d29..156445a8: Produkt 2 Datei(en) +86/-15 · Tests 1 Datei(en) +107/-3 · Doku 5 Datei(en) +1347/-3` — Runde mit Produktfortschritt |
| Rundenbilanz Etappe 3, kumuliert (`rundenbilanz.py --runden 2672ed2b 2c99fc8a ca7e7c50 00627114 156445a8`) | `2672ed2b..2c99fc8a: Produkt 10 +2621/-1 · Tests 1 +2560/-0 · Prüfwerkzeug 2 +16/-1 · Doku 61 +3143/-5` · `2c99fc8a..ca7e7c50: Produkt 8 +965/-539 · Tests 1 +1200/-55 · Prüfwerkzeug 1 +1/-1 · Doku 34 +2428/-3` · `ca7e7c50..00627114: Produkt 3 +96/-18 · Tests 1 +336/-9 · Doku 11 +1663/-3` · `00627114..156445a8: Produkt 2 +86/-15 · Tests 1 +107/-3 · Doku 8 +1439/-3` — kein Konvergenz-Signal |
| Etappenstand | **Etappe 3 abgeschlossen** auf `156445a8` nach Erstprüfung und drei Nacharbeitsrunden (Budget 3 genau ausgeschöpft, kein Konvergenzentscheid nötig). Offen aus dieser Etappe für die Abschlussprüfung: Nebenbefund N-18 (§9.11.2, vier weitere Klickproben mit Faktor 4 an M-06, B-3, M-55, B-8) wird vom Dirigenten dort eingeordnet — REGEL W-4 galt ausdrücklich nur für B-5 und B-7 |
| Hygiene im Abschlussfenster | `py -3.13 tools/plan/gesundheit.py`: URTEIL alle Grenzen gehalten (Exit 0, drei nicht blockierende Ziele verfehlt); Worker `ea8fd935` (Bau), `65c35bbe` (Nacharbeit 1), `17561b95` (Nacharbeit 2, ausgefallen am 64k-Ausgabelimit) , `b913d829` (Nacharbeit 2, Fortsetzung), `8a8a61d4` (Nacharbeit 3) beendet und entfernt; kein Loop, kein Beobachter offen |
| **Übergabe an eine frische Dirigenten-Session** | Die native Statuszeile misst den Kontext dieser Session mit 52 % des 1M-Fensters; über der 500k-Grenze (Skill §5) beginnt keine neue Etappe und keine neue Prüfrunde. Der Dirigent beendet deshalb an dieser sauberen Etappengrenze planmäßig (Markerdatei, Neustart durch den Starter). **Nächster Schritt:** Etappe 4 nach `docs/beweise/roh/SONDE-015-auftrag.txt` (Abschnitt ETAPPE 4; Bauplan §4.4, Matrix §3 Referenz, Entscheide §5 und §9.2) — **davor** NAK-245 schließen: ein Worker arbeitet die fünf Restwidersprüche der Transaktionstabelle §5.11.4 (M-76, M-125, §7.9) in die Matrix ein, ein lesender Codex-Thread (Astra max) prüft nur diese Stellen, erst dann startet der Bau-Worker (max, Aufsicht ENG, kleine Schritte: je Werkzeugaufruf rund 250 Zeilen — Lehre aus dem Ausfall `17561b95`). Danach Erstprüfung der Etappe 4 über deren Diff (Vorlage A), Abschlussprüfung über `e9dbf4b9...HEAD`, voller Kanon abgekoppelt |

## 10. Bauetappe 4a — Prozessor und Transaktionskern (10.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **Etappe 4a — Prozessor und Transaktionskern**, nach Bauplan §4.4 und dem Zusatzauftrag `docs/beweise/roh/SONDE-015-etappe-4-auftrag.txt`. Matrix §3 und §5.11.4 sind die Referenz; die Fernweg-Anteile (Sender von `state_report.dsp`, Broker, Telemetrie, voller Kanon) gehören Etappe 4b und wurden nicht angefasst. |
| Basis-SHA | `844b9c15935377e89d124ac33848e9cba1ecf4c2`, mit `git rev-parse HEAD` beim Start gemessen; `git status --short` war leer. |
| Urteil | **Offen.** Die Prüfung dieser Teiletappe steht aus. Was hier steht, ist gemessen, nicht beurteilt. |
| Beine | **37** einzeln gefahren, **36** mit Exit 0; Rohausgabe `docs/beweise/roh/SONDE-015-etappe4a-beine.txt` |
| Rotbeweise | **221** (gezählt mit `ls docs/beweise/roh/SONDE-015-rot-* \| wc -l`); davon **85** aus dieser Teiletappe, je Datei Rot unter Mutation und Grün nach Rücknahme |
| Kanon | **Nicht gefahren.** Der volle abgekoppelte Lauf gehört an das Ende von Etappe 4b (Zusatzauftrag). |
| Prozessor | `eq-copilot/plugin/sonde/SondeProcessor.{h,cpp}` trägt jetzt den aktiven DSP-Kern, die 112 Host-Parameter und den Transaktionskern. A16 misst ihn als Nulltest, B7 als Transaktionsweg. |

### 10.1 Was gebaut wurde

**Der Transaktionskern `eq-copilot/plugin/state/NakamaTransaktion.{h,cpp}` (neu, 393 und 820 Zeilen), Teil von `NakamaKern`.** Er ist identitätsfrei: der Kernriegel steht am Anfang und am Ende der Quelle, es gibt keinen Prozessorbezug, und A14 führt `NakamaTransaktion.obj` in seiner Objektliste. Er kennt das DTO (`parameter::DspSatz`), den State (`state::Zustand`) und den DSP-Kern nur über die Schnittstelle `Ausfuehrung`.

- **Typen** (`NakamaTransaktion.h`): `Stufe` S0 bis S8, `Art` (Apply, Revert, Neutralisieren, Remove, Undo, Redo, Preset-Laden, dazu Gestus und Band-Belegen als Formen des Apply), `Ausgang` (commit, gespeichertes Ergebnis, Konflikt, Fehler, User-Schutz, busy_retry), `Tid`, `Auftrag`, `Ergebnis`, `AutomationOverlay`, `PreviewOverlay`, `Transaktionskern`, `DspKernAusfuehrung`, `Klemmung`, `DspBericht`. Konstanten: `kRegisterPlaetze` gleich Undo-Tiefe gleich 32 mit `static_assert` (`:48-49`), `kHoechsteRevision` gleich größter `int64` (`:55`), `kAutomationsRuheSekunden` gleich 0,25 (`:62`).
- **CommittedState** ist genau ein `DspSatz` (E-1, keine zweite Wahrheit); dazu `r`, `r0`, der Hash, der Undo-Ring mit Cursor, das Register und beide Overlays. Die Vorschau (`PreviewOverlay`) und die Hostautomation (`AutomationOverlay`) liegen daneben; `wirksam()` legt das Automationsoverlay über den bestätigten Zustand, `schreibeIn()` schreibt nur den bestätigten.
- **Ladestart** `Transaktionskern::ladestart` (`NakamaTransaktion.cpp:234-270`): validiert DTO, Hash, Revision und Ring, setzt `r0 = r` gleich der gespeicherten Revision, leert das Register (`:263`) und beendet beide Overlays. Es entsteht keine Revision (T17).
- **`DspKernAusfuehrung`** (`:711-770`) führt S5 und S8 auf dem `DspKern` aus: `baueVor` baut das Programm aus der Kernsicht (freie Slots auf Default, E4-18), `publiziereVorbau` legt es in eine Bank oder publiziert die ENDE-Marke. Host-Dirty und Berichtsbedarf merkt sie als Atomics.
- **`baueBericht`** (`:773`) baut den Inhalt von `state_report.dsp`: `jcs` aus demselben Kanonisierer wie der Hash, `auto_gain_db` und Klemmliste aus dem Programm des bestätigten Zustands, verletzte Bänder aus dem wirksamen Zustand, Undo-Tiefe. Den Sender baut Etappe 4b.

**Die Stufenfolge im Code** (`NakamaTransaktion.cpp`, `Transaktionskern::fuehreAus` ab `:404`):

| Stufe | Datei:Zeile | Was dort geschieht | Ausgang ohne Commit |
|---|---|---|---|
| S0 | `:408`, Schleife `:409-422`, Fensterwache `:414` | Nachschlag der `tid` im vorallokierten Register; liest nur, alloziert nicht, scheitert nicht | gespeichertes Ergebnis (T1) |
| S1 | `:427-429` | `base_revision` gegen `r`, **vor** der Validierung | Konflikt (T2, T8, T10) |
| S2 | `:432-476` | Nutzlast gegen den DTO-Vertrag: Apply und Gestus mit `param::validiere`, Band-Belegen gegen die Slotvorlage, Remove mit Slotbereich, Revert mit Zielbereich, Preset-Laden liest in eine Kopie des bestätigten Zustands | Fehler (T3) |
| S3 | `:482-504`; die Ausnahme für Undo, Redo und Preset-Laden steht in genau `:487` | Zonenriegel nur für Bedienänderungen: `zonenziel` (`:383`) liefert den Zielzustand, `zoneneintritt` prüft je Zone und je Slot | User-Schutz (T4) |
| S4 | `:507-587`, Revisionswache `:511` | Kandidat vollständig bauen und gegen das DTO validieren | Fehler (T13) |
| S5 | `:591-593` | Programm bauen, ohne eine Bank zu belegen (`DspKern::baueVor`); bankpflichtig nur mit `eq_enabled` true (E-18) | busy_retry (T5, T9) |
| S6 | `:597-609` | `state_hash` über den Kandidaten | Fehler (T14) |
| S7 | `:614-637`, Ringkapazität `:621-622` | Undo-Eintrag aus dem noch bestätigten Zustand vorbereiten | Fehler (T15) |
| Commit-Punkt | `:639` | Ab hier scheitert nichts und nichts alloziert; B7 Abschnitt C misst 0 Allokationen ab S8 über 47 Commits | — |
| S8 | `:643-707` | Tausch des Kandidaten `:647`, Ring `:650-680`, `r = e` `:682`, Automationsoverlay geleert `:686`, Registereintrag `:691-695`, Publikation und Dirty-Marke `:697-698` | commit (T6) |

**Register.** 32 vorallokierte Plätze; der Eintrag entsteht ausschließlich in S8 auf Platz `(e − r0 − 1) mod 32` und verdrängt damit ab dem 33. Commit einer Sitzung genau den Eintrag, der das Fenster verlässt. S0 prüft `e > r0` und `r − e ≤ 31` zusätzlich als Wache (I2). Das Register wird nie gespeichert (B7 `register_ist_nicht_teil_des_zustands`).

**Undo-Ring.** Persistent im Kind `Dsp`, Tiefe `parameter::kUndoTiefe` gleich 32, Kapazität im Konstruktor, beim Ladestart und in S7 reserviert. Eine neue Transaktion schneidet den Redo-Zweig ab (`:669-674`), der 33. Eintrag verdrängt den ältesten (`:675-677`); Undo und Redo tauschen den Zustand mit dem Eintrag am Cursor (`:652-667`, E4-6).

**DSP-Kern.** `DspKern::uebernehmeZustand` (`DspKern.cpp:164`) ist jetzt `baueVor` (`:173`) gefolgt von `publiziereVorbau` (`:185`) — ein Publikationsweg, den S5 und S8 getrennt rufen (§10.6 A-1).

**Der Prozessor** (`SondeProcessor.cpp`, 1260 Zeilen; `SondeProcessor.h`, 461 Zeilen):

- **APVTS** `parameterBaum` (`:143`) aus `baueParameterLayout` (`:86`): die 112 Host-Parameter aus `parameter::tabelle()` in Vertragsreihenfolge, Kennung als `ParameterID` Version 1 und als Name, Bool, Float oder Choice nach Vertragstyp mit Grenzen und Default; Skew mit Zentrum am Default für `freq_hz`, `q`, `attack_ms` und `release_ms`. `occupied` steht nicht darin (M-64).
- **Kern und Audiothread.** Der Konstruktor legt `DspKern`, `DspKernAusfuehrung` und `Transaktionskern` auf den Heap, meldet sich als Listener aller 112 Parameter an und setzt die Capability `sample_accurate_automation` auf false (`:155`, M-119); der Destruktor meldet die Listener als Erstes wieder ab (`:202`). `prepareToPlay` (`:212`) bereitet den Kern unter Callback- und Zustandsschloss vor und publiziert den wirksamen Zustand; die Bank wird erst am ersten Block aktiv. `releaseResources` (`:251`) gibt den Kern wieder frei (E4-22). `processBlock` ruft nur `dspKern->verarbeite` (`:342`); die Analyse liest den Tap `post_committed`.
- **Transaktionen.** `fuehreTransaktionAus` (`:1009`): ein read-only gehaltener Stand nimmt keine an (`:1016`); nach einem Commit folgen außerhalb des Schlosses Hostparameterabgleich und Host-Dirty (`:1042`). `neueTid` (`:1004`) vergibt Transaktions-IDs.
- **Automation und Gestus.** `parameterValueChanged` (`:1110`) schreibt nur Atomics und läuft auch im Audiothread; `dspKontrollTakt` (`:1227`) läuft im bestehenden Analyseworker, frühestens alle 5 ms und spätestens nach dessen 20-ms-Warten: ACK-Ernte (`:1232`), Hostereignisse ins Automationsoverlay, Ruhegrenze (`:1252`), Publikation des wirksamen Zustands. Der Herkunftstag (`:76`) trennt den eigenen Abgleich von Hostereignissen. `parameterGestureChanged` (`:1120`) und `gestusAbschliessen` (`:1138`) machen einen abgeschlossenen Gestus zu einer Transaktion. `zelleAusHost` (`:1174`) weist nicht-endliche Hostwerte ab (`:1188`, E4-10).
- **Speichern und Laden.** `getStateInformation` (`:804`) schreibt `gehaltenerStand()` (`:816`): Common und Klassifikation aus dem gehaltenen State, die Dsp-Hälfte aus dem Kern (E4-14). `setStateInformation` (`:831`) lädt, klassifiziert, fährt den Ladestart (`:862`) oder lädt neutral, setzt die Hörmatrix auf Processed (`:872`) und gleicht die Hostparameter ab; Laden meldet kein Dirty.
- **Status und Offline.** Der v3-Status meldet `stateRevision` und `stateHash` aus dem Kern (`:923`, `:925`). `setNonRealtime` (`:1101`) beendet beim Wechsel in den Offline-Betrieb die Vorschau und stellt die Hörmatrix auf Processed.

**Die Beine.**

- **B7** `eq-copilot/plugin/tests/TransactionTestMain.cpp` (neu, 2251 Zeilen), Ziel `EqCopTransactionTest` in `eq-copilot/plugin/CMakeLists.txt`, im Runner scharf (`AbPhase='jetzt'`). Fünfzehn Abschnitte: A Falltabelle, B Fehler je Stufe, C Commit-Punkt, D sieben Transaktionen, E 10.000 Eingaben, F Wiederholung und Ladestart, G Belegung, H Zonen, I Undo-Ring, J Overlays, K Vertragsweg, L Preset, M bis O am echten `SondeProcessor`. **167** Prüfungen.
- **A16** `eq-copilot/plugin/tests/SondeNullTestMain.cpp`, umgeschrieben (734 Zeilen): M-01, M-51, M-50, M-02, M-05, M-04, M-33 und M-93 am Prozessor mit aktivem Kern, dazu die bestehenden Gate-7- und Bundleprüfungen. **57** Prüfungen. Die Runner-Behauptung ist neu geschrieben; „kein Hostparameter" ist gefallen (M-116).

### 10.2 Entscheide dieser Teiletappe (Lücken, die der Bau sonst still gefüllt hätte)

| Nr. | Lücke | Entschieden | Warum |
|---|---|---|---|
| E4-1 | Ort und Zuschnitt des Transaktionskerns | `state/NakamaTransaktion.{h,cpp}` in `NakamaKern`; der DSP-Kern hängt über die Schnittstelle `Ausfuehrung` daran | Der Auftrag verlangt eine identitätsfreie Einheit ohne Prozessorbezug. A14 misst genau das im Kern; die Schnittstelle lässt B7 den Kern mit dem echten `DspKern` fahren, ohne den Prozessor. |
| E4-2 | Wann eine Bank belegt wird | S5 baut nur das Programm (`DspKern::baueVor`), S8 belegt und publiziert (`publiziereVorbau`) | Eine in S5 belegte Bank überlebte einen Fehler in S6 oder S7 im Zustand `vorbereitend`, und kein Audio-ACK gäbe sie je frei; nach vier solchen Fehlern wäre der Pool voll. Die Trennung hält §5.11.4 Teil 2 (nichts Fehlbares hinter dem Commit-Punkt) und E-18 (bankpflichtig nur mit `eq_enabled`). |
| E4-3 | Was Revert herstellt | Genau den Zustand der referenzierten Revision: den bestätigten (Ziel gleich `r`) oder den Ring-Eintrag mit dieser Revision. Ein Ziel über `r` ist ein S2-Fehler `bereich`, ein nicht mehr gehaltenes ein S4-Fehler `revision_nicht_erreichbar`. Revert läuft als Bedienänderung durch den Zonenriegel | Der Undo-Ring ist die einzige Quelle früherer Zustände (M-78); ein nachgebauter Zwischenstand wäre eine zweite Wahrheit. Revert gehört nicht zu den Ganzzustands-Wiederherstellungen aus §5.6.3 Feinheit 3. |
| E4-4 | Was Neutralisieren zurücksetzt | Alle Werte auf den Vertragsdefault; `eq_enabled` und die Schutz-Zonen bleiben | `eq_enabled` ist der Rollenschalter (R2), keine Klangeinstellung. Zonen schützen den User (R6); ein Neutralisieren, das sie löscht, öffnete still den Weg in geschützte Bereiche. M-80 misst beides. |
| E4-5 | Unterscheidbare Fehlergründe | S4: `undo_leer`, `redo_leer`, `slot_frei` (Remove auf freiem Slot), `alle_slots_belegt` (M-62), `revision_nicht_erreichbar`, `revision_erschoepft`, `kandidat_ungueltig`; S2: der Grund des DTO-Vertrags, `bereich` für Slot und Zielrevision; im Prozessor `schreibgeschuetzt` | Jeder Ausgang ohne Commit ist unterscheidbar und nicht gemerkt (M-62, M-75); die Gründe kommen aus dem Vertrag, wo er einen hat. |
| E4-6 | Wie Undo und Redo den Ring bewegen | Ein Ring, ein Cursor. Undo tauscht den bestätigten Zustand mit dem Eintrag über dem Cursor, Redo mit dem Eintrag darunter; `revision` eines Eintrags ist die Revision des Zustands, den er hält; Undo und Redo legen keinen Eintrag an | M-79 verlangt einen Cursor im selben Ring. Der Kommentar „der VOLLE Zustand VOR der Handlung" (`NakamaState.h:435`) gilt für die Handlung, die den Eintrag zuletzt geschrieben hat; die State-Datei blieb unberührt. |
| E4-7 | Band-Belegen | Eigene Art `bandBelegen`: der erste freie Slot bekommt die dreizehn Werte und `occupied`; ohne freien Slot S4-Fehler `alle_slots_belegt`; S3 prüft das neue Band gegen jede eingeschaltete Zone; im Undo-Ring zählt die Handlung als Apply | M-62 („ein neuntes Band wird verlangt") braucht eine Handlung, die selbst einen Slot wählt; ein Apply mit vollständigem DTO wählt keinen. |
| E4-8 | Zonenprädikat | Inklusiv (`low ≤ f ≤ high`). Mit bekannter Samplerate werden Bandfrequenz und beide Grenzen auf 0,45·fs gekappt. Je Zone und je Slot; nur ein Wechsel von `occupied`, `enabled` oder `freq_hz` kann einen Eintritt erzeugen | §5.6.3 Feinheit 3 und B-03. Dieselbe Kappung wie beim Programmbau (M-12, M-66): eine Zone über 0,45·fs verhielte sich sonst anders als das Band, das sie schützt. S3 und Bericht rufen dasselbe Prädikat. |
| E4-9 | Woher der Bericht seine abgeleiteten Angaben nimmt | `auto_gain_db` und Klemmliste aus dem Programm des **bestätigten** Zustands in Kernsicht, verletzte Bänder aus dem **wirksamen** Zustand. Die Klemmliste führt nur die R7-Klemmung, höchstens eine je Slot | R4 und R13; M-70 verlangt dieselbe Verletzungsmeldung auch unter Automation. `dsp_klemmung` beschreibt die R7-Klemmung, E2-7 begrenzt die Liste auf eine je Slot; eine zweite Klemmart je Slot ergäbe bei acht Slots bis zu 16 Einträge gegen `maxItems` 8 (§10.7 N-3). |
| E4-10 | Nicht-endlicher Hostwert | Ein NaN- oder Inf-Wert eines Hostparameters zählt als unverändert: er erreicht weder Overlay noch Gestus noch Programm | Der Automationsweg geht an S2 vorbei; ohne die Wache landete ein NaN im Programm und im Audio (§10.7 N-5). |
| E4-11 | Automationsweg im Prozessor | `parameterValueChanged` zählt Hostereignisse in Atomics; der Kontrolltakt im Analyseworker (frühestens alle 5 ms, spätestens nach dessen 20-ms-Warten) schreibt sie ins Automationsoverlay. Die Epoche wechselt beim ersten Punkt und nach 0,25 s **verarbeiteten Audios** ohne neuen Punkt. Ein Commit leert das Overlay, die Hostparameter folgen dem neuen bestätigten Zustand; der eigene Abgleich trägt einen Herkunftstag und zählt nicht als Hostereignis | §44.3 und M-81. Die Ruhegrenze in Audiosamples statt Wandzeit hält Realtime und Offline gleich (M-120). Der VST3-Wrapper reicht keine Hostgeste an das Plugin weiter; ohne Ruhegrenze endete eine Automationsgeste nie. |
| E4-12 | Topologische Automation | Bei `sample_accurate_automation` unsupported wird sie nicht angewandt; der Parameter folgt trotzdem | M-119; die Capability ist in `host-capabilities-fl-v1.json` gemessen. Kontinuierliche Werte wirken blockweise. |
| E4-13 | Gestus | Nur die Gesten der eigenen Oberfläche (`beginChangeGesture` und `endChangeGesture`); Commit, wenn die letzte offene Geste endet; Nutzlast ist der bestätigte Zustand mit den Hostwerten der beteiligten Parameter; ein Gestus ohne Änderung erzeugt keine Revision; ein abgewiesener löst das Overlay und gleicht die Regler zurück | M-82. Ein Band-Drag bewegt Frequenz und Gain in zwei überlappenden Gesten und ist trotzdem eine Bedienung. |
| E4-14 | Wer den gespeicherten Zustand hält | Nur der Kern: `getStateInformation` setzt die Dsp-Hälfte beim Schreiben aus dem Kern ein; nach einem Commit wird nichts gespiegelt | E-1. Die erste Fassung spiegelte den Undo-Ring nach jedem Commit in den gehaltenen State und allozierte damit hinter dem Commit-Punkt (§10.7 N-4). |
| E4-15 | Laden eines read-only gehaltenen Stands | Der Kern lädt neutral (`eq_enabled` aus), der Stand bleibt mit Originalbytes gehalten, der Prozessor nimmt keine Transaktion an (`schreibgeschuetzt`) | §53.8 und M-92: audio-neutral und verlustfrei. Eine Transaktion auf einem nicht verstandenen Stand änderte einen Zustand, dessen Bytes der Host unverändert zurückbekommen muss. |
| E4-16 | `stateRevision` und `stateHash` im v3-Status | Aus dem Kern | Bis Etappe 3 meldete der Status einen Ladezähler als Revision; die Revision ist jetzt die des Transaktionskerns (R11). |
| E4-17 | Wo die Analyse misst | Am Tap `post_committed`, als `float`-Kopie; ohne gültigen Tap und bei ruhendem Passthrough am Puffer | §44.2 und M-57: gemessen wird der bestätigte Pfad, nie der Hörmatrix-Ausgang. |
| E4-18 | Kernsicht | Freie Slots gehen mit Defaultwerten in den DSP-Kern | Der Kern rechnet freie Slots ohnehin nicht (M-58). Ohne Kernsicht löste aber jede Automation auf einem freien Slot eine Programmpublikation aus; B7 misst an M-63, dass keine entsteht. |
| E4-19 | Namen und Wertverteilung der Hostparameter | Name gleich Vertragskennung, `ParameterID` Version 1; Skew mit Zentrum am Default für `freq_hz`, `q`, `attack_ms` und `release_ms` | Anzeigenamen sind Oberfläche (S31b). Ohne Skew läge der Default von `freq_hz` bei 4,9 % des Reglerwegs. |
| E4-20 | Revisionsgrenze | `r` endet bei 2^63 − 1; S4 weist davor mit `revision_erschoepft` ab | `state_revision` persistiert als `int64`; hinter dem Commit-Punkt darf `r + 1` nicht überlaufen. |
| E4-21 | Testzugänge | Kern: `setzeEinspritzung` (S1 bis S7) und `setzeStufenHaken`; Prozessor: `dspKernFuerTest`, `kontrollTaktFuerTest`, `zustandLesen` | M-75 verlangt einen Einspritzpunkt je Stufe; im Produkt sind beide Haken leer. `zustandLesen` liefert genau den Stand, den `getStateInformation` schreibt. |
| E4-22 | Freigabe des Kerns | `releaseResources` gibt Blockpuffer und Bänke des Kerns frei; der bestätigte Zustand bleibt im Transaktionskern und klingt nach der nächsten Vorbereitung; ein Block ohne Vorbereitung läuft unberührt durch | Paarregel starten und stoppen. Vorher blieb `releaseResources` leer, obwohl `prepareToPlay` den Kern belegte (§10.7 N-6). |
| E4-23 | Listener-Paar | Der Konstruktor meldet den Prozessor als Listener aller 112 Parameter an, der Destruktor meldet ihn als Erstes wieder ab | Keine Geste und kein Parameterereignis darf Kern oder Transaktionskern erreichen, während sie abgebaut werden (§10.7 N-11). |

### 10.3 Messabdeckung

Eine Tabelle über alle Matrixzeilen, die diese Teiletappe misst oder die der Zusatzauftrag ihr zuweist — dazu die Zeilen aus §9.3, die dort „Etappe 4" hießen, und die Nachbarzeilen aus §3.12 bis §3.14 mit ihrem Grund, wenn 4a sie nicht misst. Rotbeweise stehen unter `docs/beweise/roh/SONDE-015-rot-<ID>.txt`; die Spalte nennt nur die ID. Jede Datei trägt die mutierte Zeile, den roten Lauf mit den gefallenen Prüfungen und den grünen Lauf nach der bytegleichen Rücknahme.

| ID | Prüfung (Bein, Fall) | Rotbeweis | Status |
|---|---|---|---|
| M-01 | A16 `default_ist_bitidentisch_ueber_1000_bloecke` bei 44,1 / 48 / 96 / 192 kHz, dazu `eq_enabled steht auf false, und nach dem Lauf ist keine Bank belegt` | M-01-e4 | **gemessen** |
| M-02 | A16 `eq_an_bypass_aus_alles_neutral_ist_bitidentisch` bei 44,1 und 96 kHz | M-02-e4 | **gemessen** — den Unity-Kurzschluss selbst misst B6 (M-02, Etappe 3); §10.6 A-6 |
| M-04 | A16 `nach_dem_fade_wieder_bitidentisch` | M-04-e4 | **gemessen** |
| M-05 | A16 `eq_an_bypass_an_ist_bitidentisch` | M-05-e4 | **gemessen** |
| M-08 | B7 `eq_enabled_ueberlebt_speichern_laden`, `bank_wird_erst_beim_ersten_block_aktiv` | M-08, M-08b | **gemessen** (aus §9.3) |
| M-09 | B7 `ohne_callback_bleibt_gespeichert_und_wird_nachgeholt` | M-09 | **gemessen** (aus §9.3) |
| M-14 | B7 `apply_mit_bereichsverletzung_wird_abgewiesen` | M-14-e4 | **gemessen** (Rest aus §9.3; die Prüfstufenfolge des DTO misst Etappe 2) |
| M-23 | B7 `klemmliste_erscheint_im_bericht` (mit zusätzlich gekapptem Slot) | M-23-e4, E2-7 | **gemessen** (Rest aus §9.3) |
| M-24 | B7 `dynamicwerte_ueberleben_aus_und_ein_bitgleich` | M-24-e4 | **gemessen** (Rest aus §9.3) |
| M-28 | B7 `channel_mode_ist_diskret_und_ungekoppelt`, `kein_slot_erbt_den_modus` | M-28-e4 | **gemessen** (aus §9.3) |
| M-33 | A16 `mix_null_ist_bitidentisch` mit Gegenprobe Mix 1 | M-33-e4 | **gemessen** |
| M-39 | B7 `auto_gain_ist_nicht_im_hash` | M-39-e4 | **gemessen** (Rest aus §9.3) |
| M-44 | B7 T5 `busy_retry_wird_nicht_gemerkt`, T7, T9 (drei Fälle, darunter der bankfreie bei vollem Pool) | M-44-e4, T5 | **gemessen** (Rest aus §9.3: nicht gemerkt, Bankpflicht E-18) |
| M-50 | A16 `passthrough_sanitisiert_nichts` | M-50-e4 | **gemessen** |
| M-51 | A16 `latenz_bleibt_null`, `kein_tail_im_passthrough` | M-51, M-51b | **gemessen** (aus §9.3) |
| M-52 | B7 `nach_laden_immer_processed` | M-52-e4 | **gemessen** (Rest aus §9.3) |
| M-58 | B7 `freier_slot_verarbeitet_nichts` am Kern selbst; am Prozessor M-63 | M-58 | **gemessen** |
| M-59 | B7 `ausschalten_gibt_keinen_slot_frei` | M-59 | **gemessen** |
| M-60 | B7 `remove_ist_eine_transaktion_und_eine_revision`, `remove_nummeriert_nicht_um` | M-60, M-60b | **gemessen** |
| M-61 | B7 `undo_stellt_das_entfernte_band_als_ein_objekt_her` | M-61 | **gemessen** |
| M-62 | B7 `acht_belegte_slots_ueberschreiben_nichts` | M-62 | **gemessen** |
| M-63 | B7 `automation_auf_freien_slot_aendert_werte_nicht_klang` (Wert, Belegung, Programmübernahmen, Audio) | M-63, M-63b | **gemessen** |
| M-64 | B7 `occupied_ist_kein_hostparameter` | M-64 | **gemessen** |
| M-65 | B7 `altstand_v1_laedt_im_prozessor_verlustfrei` prüft das Ergebnis am Prozessor mit (Slot 4 belegt nach R5) | M-65 (Etappe 2) | **Etappe 2** (B2, A12); in B7 Regressionswache ohne eigenen Bruch |
| M-66 | B7 `zone_dto_grenzen`, `zone_laufzeitkappung_aendert_nichts_persistentes` | M-66-e4, M-66b | **gemessen** |
| M-67 | B7 `band_in_zone_ziehen_wird_abgewiesen`, `abweisung_laesst_hash_und_revision_unveraendert`, `einschalten_in_der_zone_wird_abgewiesen`, `von_zone_a_nach_zone_b_wird_abgewiesen` | M-67, M-67b | **gemessen** |
| M-68 | B7 `zone_ueber_band_ist_erlaubt_und_meldet_verletzung` | M-68 | **gemessen** am lokal gebauten Bericht; gesendet wird er ab Etappe 4b |
| M-69 | B7 `verletztes_band_bleibt_bedienbar` (Gain und Frequenz innerhalb der Zone) | M-69 | **gemessen** |
| M-70 | B7 `automation_verletzt_und_meldet_denselben_zustand` | M-70 | **gemessen** |
| M-71 | B7 `zone_deaktivieren_ist_eine_transaktion`, `zone_entfernen_ist_eine_transaktion` | M-71 | **gemessen** |
| M-72 | B7 `zonen_sind_teil_von_hash_preset_bericht_und_state` | M-72-e4 | **gemessen** |
| M-73 | B7 `neunte_zone_wird_abgewiesen`, `doppelte_zonen_id_wird_abgewiesen` | M-73-e4 | **gemessen** |
| M-74 | B7 `apply_` bis `preset_laden_erzeugt_genau_eine_revision`, `alle_fehlbaren_stufen_liegen_vor_dem_commit_punkt` | M-74 | **gemessen** |
| M-75 | B7 `fehler_in_S1` bis `fehler_in_S7_laesst_committed_und_hash_unveraendert`, `fehler_hinterlaesst_keinen_registereintrag`, `nachschlag_allokiert_nicht_und_schlaegt_nicht_fehl`, `hinter_dem_commit_punkt_wird_nicht_alloziert` | M-75 | **gemessen** |
| M-76 | B7 `zehntausend_transaktionen_erzeugen_hoechstens_eine_revision` mit Wachen nach jeder Eingabe, Lauf nach einem Ladestart mit `r0` = 7 | M-76 | **gemessen** |
| M-77 | B7 `hash_kommt_aus_dem_bestandskanon` (jedes gültige DTO-Fixture wie `MANIFEST.json`) | M-77-e4 | **gemessen**; die JCS-Vektoren selbst hält Etappe 2 (A12, B2) |
| M-78 | B7 `undo_ring_haelt_tiefe_32` | M-78-e4 | **gemessen** |
| M-79 | B7 `neue_transaktion_schneidet_den_redo_zweig_ab` | M-79 | **gemessen** |
| M-80 | B7 `neutralisieren_loescht_die_historie_nicht` | M-80 | **gemessen** |
| M-81 | B7 `automation_erzeugt_keine_revision` und `epoche_wechselt_genau_zweimal_je_geste`, je am Kern und am Prozessor | M-81, M-81b, M-81c | **gemessen** |
| M-82 | B7 `abgeschlossener_gestus_ist_eine_revision` (zwei überlappende Gesten) | M-82 | **gemessen** |
| M-83 | B7 `preview_wird_nie_serialisiert`, am Kern und am Prozessor | M-83, M-83b | **gemessen** |
| M-84 | B7 `reload_rekonstruiert_denselben_audioausgang` (Hash gleich, Ausgang bitgleich) | M-84, M-84b | **gemessen**; die Reihenfolge bis zum ersten Block misst M-08 |
| M-85 | B7 `angenommene_transaktion_meldet_dirty`, `abgewiesene_meldet_nicht` | M-85, M-85b | **gemessen**; dass Laden nicht meldet, steht im selben Fall als Regressionswache |
| M-86 | — | M-86 (Etappe 2) | **Etappe 2** (A12, B2) |
| M-87 | B7 `jede_v2_kennung_erklaert_ihren_ort` | M-87-e4 | **gemessen** an der Hostliste; der Vertrag selbst ist Etappe 2 |
| M-88 | B7 `apvts_fuehrt_112_parameter_in_vertragsreihenfolge`, `v2_haengt_hinten_an` | M-88 | **gemessen** |
| M-89 | B7 `prozessor_speichert_und_laedt_das_kind_dsp` | M-89-e4 | **gemessen** (Prozessorhälfte; Writer und flache Arrays sind Etappe 2) |
| M-90 | B7 `altstand_v1_laedt_im_prozessor_verlustfrei` | M-90-e4 | **gemessen** (Prozessorhälfte) |
| M-91 | — | M-91 (Etappe 2) | **Etappe 2** (B2) |
| M-92 | B7 `fremdes_major_laedt_neutral_und_bleibt_schreibgeschuetzt`; A16 Gate-7-Kette (read-only mit Originalbytes) | M-92-e4, M-92b | **gemessen** (Prozessorhälfte) |
| M-93 | A16 `speichern_laden_speichern_bytegleich_in_v2` | M-93-e4 | **gemessen** |
| M-94 | — | M-94 (Etappe 2) | **Etappe 2** (A12) |
| M-95 | B7 `preset_traegt_zwei_getrennte_versionen` | M-95-e4 | **gemessen** |
| M-96 | B7 `preset_traegt_den_klanginhalt_vollstaendig` | M-96-e4 | **gemessen** |
| M-97 | B7 `preset_hat_keine_identitaetsfelder`, `identitaetsfeld_wird_abgelehnt` | M-97-e4, M-97b | **gemessen** |
| M-98 | B7 `unbekanntes_top_level_feld_wird_ignoriert` | M-98-e4 | **gemessen** |
| M-99 | B7 `unbekanntes_preset_major_wird_abgelehnt` | M-99-e4 | **gemessen** |
| M-100 | B7 `preset_laden_ist_eine_transaktion`, `undo_nach_preset_stellt_alles_her`, `preset_laesst_eq_enabled_unberuehrt` | M-100, M-100b | **gemessen** |
| M-101 bis M-108 | — | — | **Etappe 4b.** Der Sender von `state_report.dsp` im ControlClient und das Halten im Broker fehlen noch; 4a baut nur den Inhalt (`baueBericht`). Die B7-„Sendepfade" aus M-103 und M-104 entstehen mit 4b. |
| M-109 bis M-114, M-121 | — | Etappe 2 und 3 | **Etappe 2 und 3** für Vertrag und Quelle (§8, §9.3); Sender und Sendepfad (B10) sind **Etappe 4b** |
| M-115 | — | — | **Etappe 4b**: die Kanonzahl steht erst im vollen Lauf |
| M-116 | Runner-Behauptungen B7 und A16 in `tools/beweise.ps1`; `git grep "kein Hostparameter"` trifft nur noch die stillgelegte A15-Zeile und zwei Kommentare, die sagen, dass der Satz gefallen ist | — | **gemessen** am Text; eine Behauptung ist keine Prüfung und hat keinen Rotbeweis |
| M-117 | die Rotbeweisdateien dieser Teiletappe | alle in dieser Spalte | **gemessen** — 85 Dateien |
| M-118 | — | — | **Lücke, nicht 4a:** der Fall `worst_case_vier_baenke_laeuft` steht nicht in B6 (Etappe 3 führte §3.14 nicht in ihrer Abdeckung); für die Abschlussprüfung benannt (§10.7 N-9) |
| M-119 | B7 `capability_entscheidet_den_automationspfad` mit Gegenprobe für kontinuierliche Werte; B3 unverändert | M-119, M-119b | **gemessen** (B7-Hälfte) |
| M-120 | B7 `offline_render_nutzt_den_bestaetigten_zustand` | M-120, M-120b | **gemessen** (B7-Hälfte); **Lücke, nicht 4a:** `realtime_und_offline_gleich` fehlt in B6 (§10.7 N-9) |
| M-123 | B7 T1, T11, `wiederholung_unter_einer_zone_bekommt_nicht_user_schutz` | M-123, M-123b | **gemessen** |
| M-124 | B7 T2, T5, T7, T8, T9, `fehler_hinterlaesst_keinen_registereintrag` | M-124, M-124b | **gemessen** |
| M-125 | B7 Abschnitt A: T1 bis T17 tabellengetrieben, alle vier Spalten, Wachen I1, I2 und I4 nach jeder Eingabe; `falltabelle_vollstaendig_gefahren`; Ladestart mit leerem Register | M-125, M-125b, T17 | **gemessen** |
| M-126 | B7 T16, `preset_laden_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung`, `redo_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` | M-126 | **gemessen** |
| E2-7 | B7 `klemmliste_erscheint_im_bericht`: höchstens eine Klemmung je Slot | E2-7 | **gemessen** (Etappe-2-Entscheid am Berichtsbau) |
| E4-10 | B7 `nichtendlicher_hostwert_erreicht_das_programm_nie` | E4-10 | **gemessen** (Entscheid dieser Teiletappe) |
| E4-22 | B7 `freigabe_und_vorbereitung_sind_ein_paar` | E4-22 | **gemessen** (Entscheid dieser Teiletappe) |

**Die Falltabelle T1 bis T17** (§5.11.4 Teil 4) läuft in B7 Abschnitt A tabellengetrieben; nach jeder Eingabe prüfen Wachen I1, I2 und I4, und `falltabelle_vollstaendig_gefahren` misst, dass jede Zeile genau einmal in Tabellenreihenfolge lief. Die letzte Spalte nennt den Rotbeweis, der für die Zeile gebaut ist, und jeden weiteren Rotbeweis dieser Teiletappe, unter dem die Zeilenprüfung ebenfalls fiel (aus den Rohdateien gelesen).

| Zeile | B7-Fall | Gemessen: Ausgang, `r`, gemerkt | Rotbeweis |
|---|---|---|---|
| T1 | `wiederholung_im_fenster_liefert_dasselbe_ergebnis` | gespeichertes Ergebnis in S0, `r` unverändert, gemerkt | **M-123**; fällt auch unter M-74, M-125 |
| T2 | `veraltete_base_revision_ist_konflikt_ohne_wirkung` | Konflikt in S1, nicht gemerkt; dieselbe `tid` mit aktueller Basis committet danach | **M-124b**; fällt auch unter M-74 |
| T3 | `ungueltige_nutzlast_faellt_und_wird_nicht_gemerkt` | Fehler in S2 mit dem Grund des DTO-Vertrags, nicht gemerkt; dieselbe `tid` mit gültiger Nutzlast committet danach | **M-14-e4**; fällt auch unter M-74 |
| T4 | `zonenriegel_weist_ab_und_merkt_nichts` | User-Schutz in S3, nicht gemerkt, das Band bleibt außerhalb der Zone | **M-67**; fällt auch unter M-74, M-76 |
| T5 | `busy_retry_wird_nicht_gemerkt` | busy_retry in S5 bei wirklich vollem Pool, nicht gemerkt | **T5**; fällt auch unter M-74, M-124 |
| T6 | `commit_erzeugt_genau_eine_revision_und_einen_eintrag` | commit in S8, `r` + 1, Registereintrag mit `tid`, `e` und neuem Hash | **M-125**; fällt auch unter M-74 |
| T7 | `wiederholung_nach_bankfreigabe_committet` | nach einem Audio-ACK ohne Revision committet dieselbe `tid` | **M-124**; fällt auch unter M-74, M-125, T5 |
| T8 | `retry_nach_zonenloesung_mit_alter_base_revision_ist_konflikt` | Konflikt in S1 | **M-124b**; fällt auch unter M-67, M-74, M-76 |
| T9 | `retry_nach_zonenloesung_mit_aktueller_base_revision_committet`, `…bei_vollem_pool_ist_busy_retry`, `…bei_vollem_pool_ohne_bankpflicht_committet` | commit; busy_retry in S5 (bankpflichtig); commit (bankfrei, E-18) | **M-44-e4**; fällt auch unter M-67, M-71, M-74, M-76, M-124, M-125, T5 |
| T10 | `fensterrand_erste_transaktion_ist_draussen` | nach 33 Commits: Konflikt in S1, nicht im Register | fällt unter M-74, M-124b |
| T11 | `fensterrand_zweite_transaktion_ist_drinnen` | gespeichertes Ergebnis in S0 | **M-123b**; fällt auch unter M-74, M-123, M-125 |
| T12 | `abweisungen_verdraengen_keinen_committeten_eintrag` | nach 31 Abweisungen gespeichertes Ergebnis in S0 | fällt unter M-74, M-123, M-124b, M-125 |
| T13 | `fehler_in_S4_laesst_committed_und_hash_unveraendert` | Fehler in S4 (Einspritzung), Zustand, Hash und Ring unverändert | fällt unter M-74 |
| T14 | `fehler_in_S6_laesst_committed_und_hash_unveraendert` | Fehler in S6 (Einspritzung), Zustand, Hash und Ring unverändert | fällt unter M-74 |
| T15 | `fehler_in_S7_laesst_committed_und_hash_unveraendert` | Fehler in S7 (Einspritzung), Zustand, Hash und Ring unverändert | **M-75** |
| T16 | `undo_unter_einer_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` | commit in S8; das Band liegt wieder da und gilt als verletzt | **M-126**; fällt auch unter M-61, M-68, M-76, M-125 |
| T17 | `wiederholung_ueber_sitzungsgrenze_ist_neuer_versuch` | Ladestart mit `r0 = r` = 33 und leerem Register, danach commit in S8 | **T17**; fällt auch unter M-78-e4, M-125 |

### 10.4 Die Beine dieser Teiletappe

Einzeln gefahren, kein voller Kanon. Alle Ziele — dazu die Bundles `EqCopilot_VST3` und `NakamaProbeeq_VST3`, die B1 misst — wurden in einem Aufruf aus pwsh gebaut (`cmake` liegt nicht im Bash-`PATH`), die Release-Broker der Lastbeine wie im Runner mit `cargo build --release`. Die Rohdatei nennt beide Befehle, je C++-Bein den Zeitstempel des Binaries gegen die jüngste geänderte oder vom Rotskript zurückgespielte Quelle, den Stand der Broker-Binaries und die vollständige Ausgabe. Gefahren wurde jedes Bein, dessen Ziel eine geänderte Quelle übersetzt oder linkt — `NakamaKern` enthält jetzt `NakamaTransaktion` und den erweiterten `DspKern`, und jedes Plugin-Bein des Runners linkt ihn. A15 (`EqCopSunaNullTest`) ist seit 28.08.2026 stillgelegt und wurde nicht gefahren. Rohausgabe: `docs/beweise/roh/SONDE-015-etappe4a-beine.txt`.

| Bein | Ziel | Exit | Binary jünger | Letzte Zeile | Warum es hier läuft |
|---|---|---|---|---|---|
| **B7** | `EqCopTransactionTest` | 0 | ja | TRANSAKTION OK | Neues Bein dieser Teiletappe: Transaktionskern und Prozessorseite. |
| **A16** | `EqCopProbeeqNullTest` | 0 | ja | SONDE-NULLTEST OK - 57 Pruefungen ok, 0 Fehler | Umgeschrieben: Nulltest am Prozessor mit aktivem Kern. |
| **B6** | `EqCopDspGoldenTest` | 0 | ja | DSP-GOLDEN OK | Die öffentliche Schnittstelle des DSP-Kerns ist erweitert (§10.6 A-1). |
| **B5** | `EqCopAnalysisGoldenTest` | 0 | ja | Ergebnis: 248 bestanden, 0 Fehler. | Linkt `NakamaKern`; die FeatureEngine ist unberührt. |
| **B2** | `EqCopStateMigrationTest` | 0 | ja | STATE-MIGRATION-TEST OK - 247 Pruefungen ok, 0 Fehler | Linkt `NakamaKern`; State, Parametertabelle und Preset sind unberührt. |
| **B3c** | `EqCopSchemaTest` | 0 | ja | 152 bestanden, 0 gescheitert | Linkt `NakamaKern`; der v3-Vertrag ist unberührt. |
| **A12** | `erzeuge_state_fixtures.py` | 0 | - | STATE-FIXTURES OK (64 Dateien bytegleich, keine verwaiste Datei) | Fixturekorpus der Etappe 2, im Auftrag genannt. |
| **A14** | `pruefe_kern_identitaetsfrei.py` | 0 | - | 25 ok, 0 Fehler | Der Kern trägt `NakamaTransaktion.obj`; die Objektliste ist erweitert. |
| **B1** | `EqCopIdentityTest` | 0 | ja | IDENTITY-TEST OK - 120 Pruefungen ok, 0 Fehler | Linkt `NakamaKern` und misst die gebauten Bundles `EqCopilot_VST3` und `NakamaProbeeq_VST3` (moduleinfo.json gegen `CMakeLists.txt`). |
| **B3** | `EqCopHostContextTest` | 0 | NEIN | HOSTKONTEXT OK — 91/91 Pruefungen bestanden. | Gegenprobe ohne geänderte Quelle: das Ziel linkt `NakamaKern` nicht und übersetzt keine geänderte Datei; MSBuild baute es deshalb nicht neu. |
| **B3b** | `EqCopHostProbeTest` | 0 | NEIN | HOSTPROBE OK - 85/85 Pruefungen bestanden. | Gegenprobe ohne geänderte Quelle: das Ziel linkt `NakamaKern` nicht und übersetzt keine geänderte Datei; MSBuild baute es deshalb nicht neu. |
| **B4** | `EqCopQueueStressTest` | 0 | ja | QUEUE-STRESSTEST OK | Linkt `NakamaKern`. |
| **B8** | `EqCopLebenslaufTest` | 0 | ja | LEBENSLAUF-TEST OK - 79 Pruefungen ok, 0 Fehler | Linkt `NakamaKern`. |
| **B9** | `EqCopLoudnessGoldenTest` | 0 | NEIN | LOUDNESS-GOLDEN OK | Gegenprobe ohne geänderte Quelle: das Ziel linkt `NakamaKern` nicht und übersetzt keine geänderte Datei; MSBuild baute es deshalb nicht neu. |
| **B10** | `EqCopIpcTest` | 0 | ja | ALLE PRUEFUNGEN GRUEN — 389 Pruefungen, 0 Fehler | Linkt `NakamaKern`. |
| **B11** | `EqCopSonde012HostChannelContextTest` | 0 | ja | SONDE-012 HOST CHANNEL CONTEXT: 21 bestanden, 0 fehlgeschlagen | Übersetzt `SondeProcessor.cpp` und ruft `releaseResources`. |
| **B12** | `EqCopSonde012LoudnessSourceTest` | 0 | ja | SONDE-012 LOUDNESS SOURCE: 4 bestanden, 0 fehlgeschlagen | Übersetzt `SondeProcessor.cpp`. |
| **B13** | `EqCopSonde012SourcesModelTest` | 0 | ja | SONDE-012 SourcesModel: 85/85 gruen | Linkt `NakamaKern`. |
| **B14** | `EqCopSonde012ProjectReloadTest` | 0 | ja | SONDE-012 ProjectReload: 12/12 gruen | Linkt `NakamaKern`. |
| **B15** | `EqCopShot` | 0 | ja | SONDE-012 SHOTS 25/25 gruen | Linkt `NakamaKern`. |
| **B16** | `EqCopSonde013EventWireTest` | 0 | ja | 153 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B17** | `EqCopSonde013TruePeakGoldenTest` | 0 | ja | 23 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B18** | `EqCopSonde013DynamicsTest` | 0 | ja | 46 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B19** | `EqCopSonde013StereoGoldenTest` | 0 | ja | 91 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B20** | `EqCopSonde013QualityClassTest` | 0 | ja | 24 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B21** | `EqCopSonde013InterventionRingTest` | 0 | ja | 22 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B22** | `EqCopSonde013FingerprintGoldenTest` | 0 | ja | 24 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B23** | `EqCopSonde013PassageStateTest` | 0 | ja | 343 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B24** | `EqCopSonde013PrePostGoldenTest` | 0 | ja | 30 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B25** | `EqCopSonde013ExperimentGoldenTest` | 0 | ja | 27 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B26** | `EqCopSonde013ExperimentUiTest` | 0 | ja | 52 bestanden, 0 gescheitert | Linkt `NakamaKern`. |
| **B27** | `EqCopSonde014IntentTest` | 0 | ja | SONDE-014-INTENT-TEST OK - 210 Pruefungen ok, 0 Fehler | Linkt `NakamaKern`. |
| **B28** | `EqCopSonde014BefundTest` | 1 | ja | SONDE-014 Befund und Maskierung: 78/80 gruen | Linkt `NakamaKern`; rot seit Etappe 2 an K-47, nicht durch 4a (§10.7 N-12). |
| **B29** | `EqCopSonde014AssistentTest` | 0 | ja | SONDE-014 AssistantStep: 185/185 gruen | Linkt `NakamaKern`. |
| **A22** | `pruefe_ipc_last.py` | 0 | - | GRUEN | Fährt `EqCopIpcLast`, das `NakamaKern` linkt, gegen den Release-Broker. |
| **A23** | `pruefe_sonde012_sources_latency.py` | 0 | - | GRUEN - sechs p95-Grenzen und Anzeige-Revisionen | Fährt `EqCopSonde012SourcesLatencyTest`, das `NakamaKern` linkt, gegen den Release-Broker. |
| **A24** | `pruefe_session_soak.py` | 0 | - | GRUEN | Fährt `EqCopSessionSoak`, das `SondeProcessor.cpp` übersetzt, gegen den Release-Broker. |

### 10.5 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Etappe 4a

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Voller Bankpool: S5 liefert `busy_retry`, nichts wird gemerkt, und die Wiederholung committet nach dem Audio-ACK (T5, T7, T9; Rotbeweise T5, M-44-e4, M-124). Hostereignisse stauen sich nicht: je Parameter ein Zähler und der jüngste Wert, der Kontrolltakt übernimmt den jüngsten Stand; findet ein Takt keine freie Bank, bleibt `publikationOffen` stehen und der nächste Takt wiederholt. Rückstau der Analyse verwirft weiter nur Analyse; der Kern schreibt vor ihr. |
| **B** Lebenszyklus | `prepareToPlay` und `releaseResources` (E4-22, `freigabe_und_vorbereitung_sind_ein_paar`), Vorschau an und aus samt Offline-Wechsel (M-83, M-120), Automationsgeste Beginn und Ende über die Ruhegrenze (M-81), Gestus Beginn und Ende (M-82), Ladestart und Speichern mit leerem Register (T17, M-89, M-93), Parameter-Listener an und ab (E4-23). Der Kontrolltakt läuft im bestehenden Analyseworker; dessen Start und Stopp blieben unverändert (B11 grün). |
| **C** Verträge und Längen | S1 vor S2: eine veraltete Nutzlast wird nicht validiert (T2, Stufenfolge in `alle_fehlbaren_stufen_liegen_vor_dem_commit_punkt`). S2 prüft jede Nutzlast gegen den DTO-Vertrag: NaN, Inf und Bereich (M-14, T3), Zonen 0 bis 8 mit Grenzen und eindeutigen IDs (M-66, M-73), Slot 0 bis 7 beim Remove, Presets mit Identitätsfeld oder fremdem Major (M-97, M-99). Die Klemmliste hält höchstens eine Klemmung je Slot (E2-7); ein nicht-endlicher Hostwert erreicht nie das Programm (E4-10); die Revision läuft als `uint64` ohne Überlauf (E4-20), das Fenster umfasst 32, die Undo-Tiefe kommt aus dem Vertrag. |
| **D** Bau- und Prüfriegel | B7 ist im selben Änderungssatz gebaut (`EqCopTransactionTest` in `CMakeLists.txt`) und im Runner scharf; A14 führt `NakamaTransaktion.obj`. Die Beinrohdatei nennt je C++-Bein den Zeitstempel des Binaries gegen die jüngste geänderte oder zurückgespielte Quelle; das Rotskript prüft dasselbe je Rotbeweis. |
| **E** Behauptung ≤ Messung | Die Runner-Behauptungen von B7 und A16 nennen nur, was die Tests messen (M-116). Die Zahlen dieses Abschnitts stammen aus den Rohdateien dieser Sitzung. Jede neue Prüfung wurde einmal gebrochen (85 Dateien, §10.3); was ohne eigenen Bruch mitläuft, heißt Regressionswache (M-65 am Prozessor, M-85 Laden). |
| **F** Änderungssatz | Speichern und Laden (`gehaltenerStand` und Ladestart), Aktivieren und Abklingen (`eq_enabled` an und aus, M-02, M-04), Vorbereiten und Freigeben, Vorschau an und aus, Automation Beginn und Ende liegen je im selben Änderungssatz. Writer, Reader und Migration des Kinds `Dsp` sind Etappe 2 und blieben unberührt; der Prozessor nutzt sie. |

### 10.6 Abweichungen dieser Teiletappe

| Nr. | Wortlaut | Was gebaut wurde | Begründung |
|---|---|---|---|
| A-1 | Zusatzauftrag: `eq-copilot/plugin/dsp/**` nicht anfassen; eine kleine Ergänzung der öffentlichen Schnittstelle ist erlaubt, wird begründet, und B6 läuft mit | `DspKern::baueVor` und `DspKern::publiziereVorbau` sind neu und öffentlich; `uebernehmeZustand` ruft beide nacheinander. Der private Bauplatz `arbeitsProgramm` wurde zu `vorbau` je Pfad | S5 muss das Programm bauen, ohne eine Bank zu belegen, S8 muss nicht-fehlbar publizieren (E4-2). `uebernehmeZustand` verhält sich wie vorher; B6 lief mit (Exit 0, `DSP-GOLDEN OK`). |
| A-2 | Ticketpfade: `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` ist nicht genannt | Die A14-Objektliste führt `NakamaTransaktion.obj` | Der Auftrag verlangt A14 grün mit dem neuen Kern; die Liste wird von Hand geführt, damit ein neues Kernobjekt nie still durchrutscht (§9.6 A-2). |
| A-3 | B7-Ziel „nach dem Muster von B6" | Das Ziel übersetzt zusätzlich `sonde/SondeProcessor.cpp`, `src/AnalyseEngine.cpp` und `core/ipc/PipeToken.cpp` und trägt die Sondendefinitionen wie A16 | B7 misst den echten Prozessor (M-63, M-81 bis M-85, M-88 bis M-92, M-119, M-120); B6 kennt nur den Kern. |
| A-4 | Bestehende Prozessor-Schnittstelle | `SondeProcessor::zustandLesen` liefert den Stand als Wert statt als Referenz (E4-14); neu sind `dspKernFuerTest` und `kontrollTaktFuerTest`, und `releaseResources` ist nicht mehr leer (E4-22) | Die Dsp-Hälfte entsteht beim Lesen. A16 und B11 übersetzen unverändert und laufen grün. |
| A-5 | §4.8: je Matrixzeile ein Rotbeweis | Zeilen mit mehreren Trägerzeilen tragen mehrere Rotbeweise (etwa M-60 und M-60b, M-81 bis M-81c); dazu Rotbeweise für T5, T17, E2-7, E4-10 und E4-22 | Wie §9.6 A-3: eine Zusage mit zwei Trägerzeilen braucht zwei Brüche. T5 und T17 sind Zeilen der Falltabelle mit eigener Trägerzeile; E2-7, E4-10 und E4-22 sind Entscheide, die B7 misst. |
| A-6 | Rotbeweis „an der Zeile, die die Zusage trägt" für M-02 in A16 | Die Mutation liegt an der exakten Einheit des neutralen Input-Trims (`DspProgramm.cpp`), nicht am Unity-Kurzschluss im Audiopfad | Am Prozessor rechnet der Kern in `double` und schreibt `float` zurück; eine Multiplikation mit genau 1,0 wäre bitgleich, ein Bruch des Kurzschlusses deshalb am Prozessor unsichtbar. Den Kurzschluss selbst bricht B6 (Etappe 3, M-02). |
| A-7 | Rotbeweis durch Mutation einer Produktzeile | M-125b mutiert die Falltabelle des Tests; M-08b, M-83b, M-97b und M-124 fügen die verbotene Handlung an der tragenden Stelle ein | M-125 ist eine Zusage an den Test selbst. Die vier anderen Zusagen sind Abwesenheiten — kein Block vor dem ersten Callback, keine Vorschau im geschriebenen Stand, kein Identitätsfeld im Preset, kein Registereintrag ohne Commit —, und ihr Bruch ist die Einfügung. |

### 10.7 Nebenbefunde (nicht ins Register)

| Nr. | Befund | Wo gefunden | Was geschah |
|---|---|---|---|
| N-1 | **Ein Rotbeweis ließ B7 abstürzen, statt es rot werden zu lassen.** Unter M-76 blieb die Zonenliste im bestätigten Zustand leer; der Test indizierte `zonen[0]` ungeschützt und endete mit einer Zugriffsverletzung, bevor die erwartete Prüfung lief. | Erster Rotlauf | Test gehärtet: kein ungeschützter Zonenindex mehr. M-76 fällt jetzt an `zehntausend_transaktionen_erzeugen_hoechstens_eine_revision`. |
| N-2 | **M-63 maß die Programmpublikation nicht.** Der Rotbeweis M-63b (Kernsicht entfernt) blieb zweimal grün: zuerst fehlte die Messung, danach verdeckte eine noch nicht übernommene Publikation des Commits die zweite. | Erster und dritter Rotlauf | Die Prüfung vergleicht jetzt die Programmübernahmen nach einem Vorlauf; M-63b fällt. |
| N-3 | **Matrix M-12 gegen Vertrag E2-7.** M-12 sagt, der gekappte Wert werde „als geklemmt gemeldet". `dsp_klemmung` beschreibt die R7-Klemmung, und E2-7 erlaubt höchstens eine Klemmung je Slot (`maxItems` 8). Die erste Fassung von `baueBericht` meldete beide Klemmarten und hätte bei acht gekappten Slots auf `priority_sidechain` 16 Einträge erzeugt. | Selbstaudit gegen `eq-ipc-v3.schema.json` (`$defs/dsp_bericht`) | Der Bericht folgt dem Vertrag (E4-9). Die Nyquist-Kappung steht im Programm (`gekapptFreq`, B6 M-12) und erreicht Gen heute nicht. Welche Fläche sie meldet, ist eine Vertragsfrage für die Abschlussprüfung. |
| N-4 | **Allokation hinter dem Commit-Punkt im Prozessor.** Die erste Fassung spiegelte nach jedem Commit den bestätigten Zustand samt Undo-Ring in den gehaltenen State — nach dem Commit-Punkt des Kerns, und bei Speichermangel wäre der gespeicherte Stand vom bestätigten abgewichen. | Selbstaudit „keine Allokation hinter dem Commit-Punkt" | Behoben (E4-14): gespiegelt wird nichts, `getStateInformation` setzt die Dsp-Hälfte beim Schreiben ein. |
| N-5 | **NaN über den Hostparameter.** `zelleAusHost` reichte einen nicht-endlichen normierten Wert über `convertFrom0to1` ins Automationsoverlay; der Automationsweg geht an S2 vorbei. | Selbstaudit „NaN/Inf an jedem Eingang" | Behoben (E4-10), mit Prüfung und Rotbeweis. |
| N-6 | **`releaseResources` war leer.** Der Prozessor bereitete den Kern in `prepareToPlay` vor und gab ihn nie frei (Paarregel starten und stoppen). | Selbstaudit Prüfliste B | Behoben (E4-22), mit Prüfung und Rotbeweis. Ein Block nach der Freigabe läuft unberührt durch; B11 ruft `releaseResources` und blieb grün. |
| N-7 | **B7 hat 2251 Zeilen.** `tools/plan/gesundheit.py` misst die Grenze von 2000 Zeilen nur unter `broker/src`, `eq-copilot/plugin/src` und `eq-copilot/plugin/core`; Testquellen liegen außerhalb. | Selbstaudit Codebase-Gesundheit | Nicht geteilt: Pflege läuft nie im Feature-Ticket. Benannt für den nächsten Pflegeschritt. |
| N-8 | **Zeilenenden.** `git diff` meldet für sechs geänderte Dateien „LF will be replaced by CRLF"; der Index ist normalisiert, die Diffzahlen sind die echten. | `git diff --stat` | Keine Handlung; keine der Dateien liegt unter `schemas/` oder `fixtures/`. |
| N-9 | **M-118 und die B6-Hälfte von M-120 fehlen.** `worst_case_vier_baenke_laeuft` und `realtime_und_offline_gleich` stehen in keinem Test; §9.3 führte §3.14 nicht. | Abgleich der Prüfspalte von §3.14 mit B6 | Nicht in 4a gebaut (Fälle des DSP-Kerns); für die Abschlussprüfung benannt. |
| N-10 | **Rücknahme bei zwei Mutationen derselben Datei.** Das Rotskript spielte zunächst in Anwendungsreihenfolge zurück; bei zwei Mutationen derselben Datei hätte die zweite Rücknahme den Zwischenstand geschrieben. | Vor dem ersten betroffenen Rotbeweis (M-75) | Behoben, bevor M-75 lief: Rücknahme rückwärts, SHA-256 gegen die Ursprungsbytes; jede Datei nennt den Hash. |
| N-11 | **Parameter-Listener blieben bis zum Abbau angemeldet.** Der Konstruktor meldete den Prozessor an allen 112 Parametern an, der Destruktor meldete ihn nicht ab; eine Geste während des Abbaus hätte `gestusAbschliessen` auf einen fallenden Transaktionskern geführt. | Selbstaudit Prüfliste B | Behoben (E4-23). Ohne eigene Prüfung und ohne Rotbeweis: der Wettlauf ist im Test nicht deterministisch herstellbar; B7, A16 und B11 bauen und laufen grün. |
| N-12 | **B28 ist seit Etappe 2 rot.** `EqCopSonde014BefundTest` prüft unter K-47 (NAK-213), dass `nakama::ipc::kJsonSchemaMinor` auf 4 steht und ein Rahmen der Fassung 5 abgewiesen wird. Etappe 2 (`94a8a336`, 10.09.2026) hob C++ und Broker (`JSON_SCHEMA_MINOR_AKTIV` in `broker/src/coordinator/schema.rs:8`) auf Fassung 5; B28 lief in Etappe 2 und 3 nicht mit. Die beiden Prüfungen fallen deterministisch an Quellen, die 4a nicht berührt: `git diff HEAD --stat -- broker eq-copilot/plugin/core eq-copilot/plugin/vertrag eq-copilot/schemas eq-copilot/fixtures eq-copilot/plugin/tests/Sonde014BefundTest.cpp` ist leer. | Beinlauf dieser Teiletappe (§10.4) | Nicht in 4a behoben: der Fall gehört zum Fassungsschritt der Etappe 2 und zum vollen Kanon der Etappe 4b. Für den Dirigenten benannt. |
| N-13 | **Der erste Beinlauf baute zwei gemessene Artefakte nicht mit.** Er baute die Beinziele, aber weder die Bundles `EqCopilot_VST3` und `NakamaProbeeq_VST3` noch die Release-Broker der Lastbeine: B1 fiel an der Frische von `moduleinfo.json` gegen die geänderte `CMakeLists.txt`, und A22 bis A24 liefen gegen Broker-Binaries vom 09.09.2026, älter als der Fassungsschritt der Etappe 2 (der Broker schloss 556 Verbindungen am Envelope). | Erster Beinlauf dieser Teiletappe | Das Beinskript baut jetzt wie `tools/beweise.ps1 -Bauen` die gemessenen Bundles und die drei Release-Broker mit; im zweiten Lauf sind B1 und A22 bis A24 grün. §10.4 zeigt nur den zweiten Lauf. |

### 10.8 Erstprüfung Etappe 4a — NEEDS_WORK; Nacharbeit vertagt hinter den Audit (Dirigent, 2026-09-10)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08cfc-095f-7741-81f6-3913f5cb55b4`; Lauf 22:22–22:42 |
| Prüfbereich | Erstprüfung (Vorlage A) über `git diff 844b9c15...dc6c148a -- eq-copilot/plugin/state/NakamaTransaktion.{h,cpp} eq-copilot/plugin/sonde/SondeProcessor.{h,cpp} eq-copilot/plugin/dsp/DspKern.{h,cpp} eq-copilot/plugin/tests/TransactionTestMain.cpp eq-copilot/plugin/tests/SondeNullTestMain.cpp eq-copilot/plugin/CMakeLists.txt tools/beweise.ps1 tools/eq-copilot/pruefe_kern_identitaetsfrei.py`; HEAD während des Laufs `12415184` (trägt nur docs/: Prüfauftrag, Audit, Register, Aufträge), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-etappe-4a-erstpruefung-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-etappe-4a-erstpruefung-dc6c148.txt` |
| Urteil | **NEEDS_WORK — sechs Defekte** (noch nicht vom Dirigenten an der Quelle eingeordnet): (1) P1 Abgleich verwirft überholte Hostereignisse nicht — abgewiesener Gestus wird über die Mailbox doch wirksam (M-67, M-84; `SondeProcessor.cpp:1208-1210`); (2) P1 Gestus-Listener hält `listenerLock` und `zustandSchloss` und rechnet die Transaktion synchron, der VST3-Audiothread kann darauf warten (CLAUDE.md Audiothread; `:1135`); (3) P2 Hostautomation bei read-only gehaltenem Zustand publiziert ein hörbares Programm (M-92; `:1239-1242`); (4) P2 `setNonRealtime` nimmt `zustandSchloss` im Audiothread des VST3-Wrappers (`:1104-1106`); (5) P2 verworfene nicht-endliche Hostwerte werden nicht gezählt (CLAUDE.md NaN-Ehrlichkeit; `:1188-1189`); (6) P2 M-125-Wachen laufen im 33-Commit-Vorlauf und bei T12 nicht nach jeder Eingabe (`TransactionTestMain.cpp:564-566`) |
| Rundenbilanz Etappe 4a | `844b9c15..dc6c148a: Produkt 7 Datei(en) +1913/-48 · Tests 2 Datei(en) +2663/-161 · Prüfwerkzeug 2 Datei(en) +14/-2 · Doku 87 Datei(en) +8539/-5` — Runde mit Produktfortschritt |
| Entscheid | **Nacharbeit 1 der Etappe 4a ist VERTAGT** (User-Wort 10.09.2026: „bei der nächsten guten möglichkeit den implement anzuhalten und zuerst die gefundenen probleme dann zu beheben. den worker aber zuende bauen lassen"). Reihenfolge: SONDE-015-Audit-Nacharbeit D8/D10/N-12/N-9 (`docs/beweise/roh/SONDE-015-audit-nacharbeit-auftrag.txt`) → NAK-246 (Etappen 1 bis 6, Kanon) → `/code-review ultra` durch den User (Skill §3.7) → Quellvalidierung der sechs Befunde durch einen lesenden Opus-Agenten, Einordnung, Nacharbeit 1 der Etappe 4a (Runde 1 von 3) → Wiederprüfung → Etappe 4b. Die Zeilennummern des Urteils gelten zum Stand `dc6c148a` |

---

## 11. Audit-Nacharbeit D8/D10 (10.09.2026)

| Merkmal | Wert |
|---|---|
| Etappe | **Audit-Nacharbeit** der abgenommenen Etappen 2 und 3 nach dem Zusatzauftrag `docs/beweise/roh/SONDE-015-audit-nacharbeit-auftrag.txt`: die Befunde D8 und D10 des unabhängigen Codeaudits vom 10.09.2026 (`docs/audits/2026-09-10-code-review/AUDIT.md` §2, an der Quelle validiert in `docs/beweise/roh/NAK-246-quellvalidierung.md`), dazu die Nebenbefunde N-12 und N-9 der Etappe 4a (§10.7). Die übrigen Auditbefunde laufen als NAK-246; die sechs Defekte der Erstprüfung 4a (§10.8) sind nicht Gegenstand. Der Ticketauftrag `docs/beweise/roh/SONDE-015-auftrag.txt` gilt. |
| Basis-SHA | `07dfcb77c14f82ab3b3c77f7d277474458937520`, mit `git rev-parse HEAD` beim Start gemessen; `git status --short` war leer. |
| Urteil | **Offen.** Die Prüfung dieser Nacharbeit steht aus. Was hier steht, ist gemessen, nicht beurteilt. |
| Beine | **10** einzeln gefahren (B28, B6, B2, B3c, A1, A16, B7, A12, A4, A14), **10** mit Exit 0; Rohausgabe `docs/beweise/roh/SONDE-015-audit-nacharbeit-beine.txt` |
| Rotbeweise | **12** aus dieser Nacharbeit, je Datei Rot unter Mutation und Grün nach bytegleicher Rücknahme mit SHA-256; zusammen **233** Rotbeweisdateien des Tickets (gezählt mit `ls docs/beweise/roh/SONDE-015-rot-* \| wc -l`). Dazu die Diagnose `docs/beweise/roh/SONDE-015-audit-nacharbeit-m46-diagnose.txt`, das Rotskript `docs/beweise/roh/SONDE-015-audit-nacharbeit-rotskript.ps1` und das Beinskript `docs/beweise/roh/SONDE-015-audit-nacharbeit-beinskript.ps1`. |
| Kanon | **Nicht gefahren.** Der volle abgekoppelte Lauf gehört an das Ende von Etappe 4b. |
| Marke | **MA** (Audit-Nacharbeit) steht in der Spalte Belegklasse der Matrixzeilen, deren Beleg sich hier ändert: M-46 (§3.5), M-66 (§3.8), M-118 und M-120 (§3.14). Die Zeilen selbst sind Verlauf und bleiben, wie sie sind. |

### 11.1 Was geändert wurde

**D8 — der Bereich der Zonen-id vor jeder Konvertierung (M-66).**

- `eq-copilot/plugin/state/NakamaParameter.cpp:395-409` (`leseZonen`): Die ganze Zahl wird nicht mehr per `(int)` gewandelt. Die id gilt genau dann, wenn sie einer der Slotnummern 0 bis 7 gleicht (`:404-406`); jede andere wird −1. `validiereZonen` (`:202-258`, unverändert) weist −1 an seiner bisherigen Stelle der Prüfreihenfolge als `bereich` ab, mit derselben Fundstelle `schutz_zonen[i].id`.
- `eq-copilot/plugin/state/NakamaPreset.cpp:248-255` (`lies`): derselbe Riegel im Preset-Leser.
- `tools/eq-copilot/erzeuge_state_fixtures.py:827-838` und `:999-1008`: sieben Negativfixtures vom Erzeuger, Grund je `bereich` — im DTO `zone-id-minus-1`, `zone-id-2147483648` und `zone-id-9007199254740991` (`zone-id-8` bestand), im Preset dieselben drei und `zone-id-8`. Der Python-Referenzleser (`validiere_zonen_python`) klassifiziert sie beim Erzeugen selbst. `9007199254740991` ist 2^53 − 1: die größte Ganzzahl, die der Textriegel aller drei Sprachen durchlässt; sie erreicht damit den Zonenleser.
- `eq-copilot/fixtures/state/`: die sieben Dateien und `MANIFEST.json` (+32 Zeilen), vom Erzeuger geschrieben. Vor dem Schreiben gelesen: `git ls-files --eol` zeigt Manifest und Korpus als `i/lf w/lf attr/-text`; der Erzeuger schreibt LF, und alle übrigen Dateien blieben bytegleich (A12).
- `eq-copilot/plugin/tests/StateMigrationTestMain.cpp:27-28` (`<cfenv>`, `<cmath>`), `:70-81` (`setztFeInvalid`), `:739-793` (DTO-Leser) und `:880-929` (Preset-Leser): je Leser die Gegenprobe (`sqrt` von −1 setzt FE_INVALID), ein gültiger Stand als Ziel, für die vier ids Grund `bereich` mit Fundstelle `schutz_zonen[0].id`, kein FE_INVALID und ein unverändertes Ziel, dazu die gültigen Rand-ids 0 bis 7 ohne Flag (DTO aus `eq-copilot/fixtures/state/dto/gueltig/maxima.json`, Preset vom Writer geschrieben). **29** neue Prüfungen; B2 steigt von 247 auf 276.
- Rust unverändert: `broker/tests/contract_cross_language.rs:525-569` nimmt die Fixtureliste aus `MANIFEST.json`, und `broker/src/dto.rs:439-499` hält die id als `i64` und prüft ihren Bereich, bevor sie verwendet wird. A4 klassifiziert die drei neuen DTO-Fixtures mit. Einen Rust-Presetleser gibt es nicht (R12).

**D10 — der Messwert des Vier-Bank-Falls (M-118) und dieselbe Klasse an M-46.**

- `eq-copilot/plugin/dsp/DspKern.h:207-223`: lesender Testzugang `rechnendeSlots (Pfad, klingend, quelle)` (§11.6 A-1). `DspKern.cpp` ist unberührt.
- `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4081-4301` (Abschnitt N, neu gefasst): Committed wechselt zwischen zwei Programmen, Candidate zwischen zwei weiteren; jeder Wechsel dreht den `channel_mode` aller acht dynamischen Bänder weiter und ist damit rampeninkompatibel, und beide Wünsche liegen vor demselben Blockrand. Blockgröße 128, 4000 Übergänge zu je zwei Callbacks und ebenso viele stationäre Callbacks, im Wechsel. Gemessen wird je Callback nur `verarbeite`; das Signal liegt vorher im Speicher, Publikation und Ernte liegen außerhalb. Prüfungen: `worst_case_vier_baenke_laeuft (M-118)`, `jeder Uebergangscallback rechnet vier echte Bankdurchlaeufe (M-118)`, `jeder stationaere Callback rechnet zwei Baenke, je Pfad eine (M-118)`, `jede Publikation des Messlaufs kam durch (kein busy_retry)` und die zwei Wachen „gemessen, nicht gedeckelt"; zwei Messwertzeilen mit Mittel, p99 und Maximum.
- `:2845-2888` (Abschnitt I, M-46): beide Pfade laufen zuerst eingeschwungen auf je einer Bank; dann liegen zwei rampeninkompatible Wünsche vor demselben Blockrand, und `vier_baenke_im_schlimmsten_fall` prüft mitten im Crossfade die vier rechnenden Identitäten und `freieSlots() == 0`.
- `:5-7` und `:567`: Dateikopf und Gate-Zeile nennen M-118 und M-120.

**N-9 — M-120, die Hälfte des Kerns.**

- `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4303-4469` (Abschnitt O, neu): zwei frische Kerne fahren dieselben 180 Blöcke von 1 bis 512 Samples mit denselben Worker-Aufrufen an denselben Blockrändern — Einschalten, Rampenübergang mit globalen Rampen, Candidate, Hörmatrix auf Candidate und zurück, Crossfade, Candidate-Ende, Ausschalten —, einmal im Takt der Wanduhr (`sleep_until` auf die Audiozeit), einmal ohne Takt. Ein dynamisches Band liegt auf einem Ton, dessen Hüllkurve durch Threshold und Knie pendelt. Prüfungen: `realtime_und_offline_gleich (M-120): derselbe Ausgang` (bitgleich über 40 446 Samples) und `realtime_und_offline_gleich (M-120): derselbe Parameterverlauf` (Auslenkungen und Auto-Gain je Block bitgleich), dazu vier Wachen: der Realtime-Lauf folgte der Wanduhr, der Offline-Lauf lief ohne Takt, beide nahmen dieselben sechs Publikationen, und die Auslenkung bewegte sich. B6 steigt mit N, I und O von 247 auf **257**.

**N-12 — B28 K-47 folgt dem Vertrag.**

- `eq-copilot/plugin/tests/Sonde014BefundTest.cpp:487-541`: die unabhängige Fassungszahl kommt aus `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` (`wire_envelope_schema_minor.aktuell`, heute 5), nicht mehr aus einem Literal. Prüfungen: `K-47: die aktive P1-Fassung steht im Register`, `K-47: ein Rahmen der aktiven Fassung passiert den Leser`, `K-47 Gegenprobe nach oben: ein Rahmen der naechsten, unbekannten Fassung faellt am Leser` und `K-47: die C++-Fassungszahl steht auf der aktiven Registerfassung`. B28 steigt von 78/80 auf **81/81**.

### 11.2 Entscheide dieser Nacharbeit (Lücken, die der Bau sonst still gefüllt hätte)

| Nr. | Lücke | Entschieden | Warum |
|---|---|---|---|
| EA-1 | R-D8 sagt „prüfen den Bereich … am double VOR dem Cast (Grund bereich, Detail wie bisher); die Reihenfolge der Prüfungen bleibt sonst unverändert" — offen ist, wo die Ablehnung fällt | Der Leser prüft den Bereich vor jeder Konvertierung, lehnt aber nicht selbst ab: eine id außerhalb 0..7 wird −1, und `validiereZonen` weist sie an ihrer bisherigen Stelle ab | Eine Ablehnung schon im Leser läge **vor** der Anzahlprüfung. `eq-copilot/fixtures/state/dto/ungueltig/zone-neunte.json` (ids 0 bis 8) fiele dann mit `bereich` statt `zone_anzahl`, und C++ wiche von Rust (`broker/src/dto.rs:439-499`) und Python (`validiere_zonen_python`) ab, die beide erst die Anzahl und dann den id-Bereich prüfen. So bleibt die Reihenfolge in allen drei Sprachen gleich, und Grund und Fundstelle sind dieselben wie vorher. |
| EA-2 | Wie die id ohne Konvertierung zur Ganzzahl wird | Vergleich mit den acht Slotnummern (`id->zahl == (double) k`) statt eines bereichsgeprüften `(int)` | Ein `(int)` hinter einer Bereichsprüfung ist korrekt, doch ein optimierender Compiler darf die Konvertierung vor den Vergleich ziehen und ihr Ergebnis danach verwerfen; dann setzte sie das Invalid-Flag, ohne dass ein Wert falsch würde, und B2 meldete falsch rot. Der Vergleich enthält keine Konvertierung von double nach int, auf keinem Compiler. |
| EA-3 | Woran der Rotbeweis D8 fällt | Am Invalid-Flag der Gleitkommaeinheit (`FE_INVALID`), nicht am Grund | Unter der Mutation bleibt der Grund `bereich`, und der ganze Korpus fällt weiter mit seinem Manifestgrund (Rotbeweisdateien, Abschnitt „Weitere Zeilen"): die Konvertierung von 2147483648 liefert hier INT_MIN, das `validiereZonen` zufällig abweist — genau die Zufälligkeit, die der Audit benennt. Die Konvertierung setzt aber das Flag, gemessen im roten Lauf. MSVC hat keinen UBSan; das Flag ist hier die passende Sanitizerprüfung. Die Gegenprobe (`sqrt` von −1) belegt, dass das Bein das Flag sieht. |
| EA-4 | Die Matrixzeile der Zonen-id-Grenze | M-66, getrennt für DTO- und Preset-Leser; nicht M-90 und M-91 | Der Auftrag nennt M-90/M-91 mit dem Vorbehalt „oder die IDs, die die Zusage tragen". M-90 trägt das Laden eines v1-Stands, M-91 unbekannte Eigenschaften im Kind `Dsp`. Die Grenze der Zonen-id trägt M-66 („Je Zone `id` (0 bis 7 …)"), und der Preset-Leser folgt denselben Zonenregeln (`erzeuge_state_fixtures.py`: „dieselben Zonenregeln wie im DTO"). |
| EA-5 | Blockgröße und Zahl der Wiederholungen im Abschnitt N (R-D10) | Blockgröße 128 unter kFadeSamples 256; `uebergangsCallbacks` = ceil(256/128) = 2; je Region dieselbe gewertete Audiozeit wie der bisherige Messwert (4000 Callbacks zu 256 Samples, also 1 024 000 Samples) — das sind 4000 Übergänge und 8000 Callbacks je Region | Die Zahl folgt aus kFadeSamples und Blockgröße (Code am Abschnittsbeginn). Bei einer Blockgröße ab 256 endete jeder Crossfade im Callback, in dem er beginnt; die Bankidentitäten wären dann erschlossen, nicht gemessen. 8000 Callbacks je Region stellen p99 auf 80 Callbacks. Übergangs- und stationäre Callbacks wechseln sich je Zyklus ab (zwei und zwei), damit Takt und Wärme des Rechners beide Regionen gleich treffen. |
| EA-6 | Woran „vier Bänke rechnen" fällt | An zwei getrennten Messungen: den Identitäten (`rechnendeSlots` beider Pfade: vier verschiedene Slots, klingend `audioAktiv`, Quelle `verblassend`) und den echten Bankdurchläufen (`msStufenLaeufe()` steigt je Übergangscallback um 4, je stationärem Callback um 2) | `rechnendeSlots` liest den Pfadzustand nach derselben Regel wie `verarbeitePfad`; stimmte diese Regel einmal nicht mehr, sähe es der Durchlaufzähler. Bei `width` 1,3 rechnet jeder Bankdurchlauf die M/S-Stufe genau einmal (`eq-copilot/plugin/dsp/DspKern.cpp:607-613`); der bestehende Zugang aus E-24 (B-19) zählt damit Durchläufe. `freieSlots() == 0` ist kein Beweis mehr. |
| EA-7 | Was „Übergangslast" und „stationäre Last" als Zahl sind | Je Callback die Dauer von `verarbeite`, geteilt durch Blockgröße durch fs; gemeldet werden Mittel (Summe der Dauern durch Summe der Budgets), p99 und Maximum je Region | R15: kein Deckel. Der Golden hält fest, dass der Fall läuft, keine Bank teilt und gemessen wurde; die Zahlen stehen in §11.4. Das Maximum ist ein Einzelwert unter dem Takt des Betriebssystems, kein Budget. |
| EA-8 | M-46 trug dieselbe Klasse wie D10 | `vier_baenke_im_schlimmsten_fall` fährt den gleichzeitigen Wechsel aus eingeschwungenen Pfaden und prüft die rechnenden Identitäten mitten im Crossfade | R-D10: „`freieSlots() == 0` ist kein Beweis mehr". Die Diagnose zeigt an der bisherigen Prüfstelle zwei rechnende und zwei bereite Bänke, während die Prüfung „beide Pfade mitten im Fade: vier lebende Baenke" meldete (§11.7 N-A2). |
| EA-9 | Die unabhängige Fassungszahl in K-47 (R-N12) | Aus dem Register (`wire_envelope_schema_minor.aktuell`), nicht aus `kJsonSchemaMinor` | K-47 ist ausdrücklich der messende Rotbeweis für `kJsonSchemaMinor` (NB-3): der Rahmen gibt seine Fassung unabhängig vor. Nähme der Test die Konstante selbst, liefen Rahmen und Leserobergrenze gemeinsam, und eine hinter dem Vertrag zurückgebliebene Konstante bliebe unsichtbar; der Rotbeweis `K-47-konstante-audit` misst genau das. Die Registerzahl ist dieselbe, an die A5 den C++-Text bindet (`tools/eq-copilot/pruefe_v3_vertrag.py:1293-1299`), und reißt am nächsten Fassungsschritt nicht. |
| EA-10 | Was M-120 ohne Prozessor misst (R-N9) | Die Hälfte des Kerns: derselbe Event- und Blockverlauf im Takt der Wanduhr und ohne Takt; Ausgang, Auslenkungen und Auto-Gain je Block bitgleich | M-120 sagt „innerhalb numerischer Toleranz". Derselbe Build rechnet dieselben Samples; die ehrliche Toleranz ist deshalb 0. Die Prozessorhälfte (Offline nutzt den bestätigten Zustand) misst B7 seit 4a; eine offene Frage des Prozessors steht in §11.7 N-A3. |
| EA-11 | Welche Prüfungen einen eigenen Bruch brauchen | Zwölf Rotbeweise statt der drei genannten: je Leser der Bruch am Flag, am gültigen Rand und am Ziel; K-47 an der Gegenprobe und an der Konstante; M-118; M-46; M-120 am Ausgang und am Parameterverlauf. Die übrigen neuen Prüfungen heißen Wachen (§11.3) | Prüfliste E: jede neue Prüfung mit Zusage wurde einmal gebrochen. Rand, Ziel und die positive Hälfte von K-47 tragen eigene Teile der Zusage; die Wachen sichern nur, dass ein Lauf gilt, und tragen keine. |

### 11.3 Messabdeckung

Rotbeweise liegen unter `docs/beweise/roh/SONDE-015-rot-<ID>.txt`; die Spalte nennt nur die ID. Jede Datei trägt die mutierte Zeile, den roten Lauf mit den gefallenen Prüfungen, die bytegleiche Rücknahme mit SHA-256 und den grünen Lauf; Binary jünger als die mutierte und die zurückgespielte Quelle.

| ID | Prüfung (Bein, Fall) | Rotbeweis | Status |
|---|---|---|---|
| M-66, DTO-Leser | B2 `D8 zone_id_ausserhalb_faellt_ohne_konvertierung (M-66, DTO)` für `zone-id-2147483648` und `zone-id-9007199254740991` | M-66-dto-audit | **gemessen** — unter der Mutation blieben Grund und Korpus (29/29) grün, es fiel das Flag |
| M-66, DTO-Leser | B2 `D8 zone_id_ausserhalb_faellt_mit_bereich (M-66, DTO)` für alle vier ids; Korpus G6 (29/29); A4 (Rust) und A12 (Python) für dieselben Fixtures | M-66 (Etappe 2, Bereich in `validiereZonen`) | **gemessen**; −1 und 8 fielen schon vor D8 so (Regressionswache) |
| M-66, DTO-Leser | B2 `D8 gueltige Rand-ids 0 bis 7 kommen exakt an, ohne FE_INVALID (M-66, DTO …)` | M-66-dto-rand-audit | **gemessen** |
| M-66, DTO-Leser | B2 `D8 das Ziel bleibt unveraendert (M-66, DTO)` | M-66-dto-ziel-audit | **gemessen** — Regressionswache mit eigenem Bruch: die Eigenschaft bestand vor D8 |
| M-66, Preset-Leser | dieselben vier Prüfungen mit `(M-66, Preset)`; Korpus G6b (17/17); B7 fährt die vier neuen Preset-Fixtures zusätzlich als Preset-Laden (S2) | M-66-preset-audit, M-66-preset-rand-audit, M-66-preset-ziel-audit | **gemessen** |
| D8, Beobachter | B2 `D8 Gegenprobe: dieses Bein sieht FE_INVALID (sqrt von -1)`, `D8 Ziel: … gelesen` | — | **Wache**: ohne sie wäre ein grünes „kein FE_INVALID" wertlos |
| M-118 | B6 N `worst_case_vier_baenke_laeuft (M-118)`, `jeder Uebergangscallback rechnet vier echte Bankdurchlaeufe (M-118)` | M-118-audit | **gemessen**; Messwerte in §11.4 |
| M-118, Wachen | B6 N `jede Publikation des Messlaufs kam durch (kein busy_retry)`, `jeder stationaere Callback rechnet zwei Baenke, je Pfad eine (M-118)`, `die Uebergangslast wurde gemessen, nicht gedeckelt`, `die stationaere Last wurde gemessen, nicht gedeckelt` | — | **Wachen**: sie sichern, dass der Messlauf gilt |
| M-46 | B6 I `vier_baenke_im_schlimmsten_fall (M-46)` | M-46-audit | **gemessen**; `candidate_endet_neutral` behält seinen Rotbeweis M-46 aus Etappe 3 |
| M-120, Kern | B6 O `realtime_und_offline_gleich (M-120): derselbe Ausgang` | M-120-audit | **gemessen** — die globalen Rampen an der Wanduhr; mit ihnen fielen auch `… derselbe Parameterverlauf`, weil die Input-Trim-Rampe vor dem Detektor liegt, und `die_rampe_folgt_der_ausgeschriebenen_ideallinie (M-17)` (§11.7 N-A5) |
| M-120, Kern | B6 O `realtime_und_offline_gleich (M-120): derselbe Parameterverlauf` | M-120-verlauf-audit | **gemessen** — die Steuerrate der dynamischen Bänder an der Wanduhr; mit ihr fiel auch der Ausgang |
| M-120, Wachen | B6 O `der Realtime-Lauf folgte der Wanduhr`, `der Offline-Lauf lief ohne Takt`, `beide Laeufe nehmen dieselben sechs Publikationen`, `die dynamische Auslenkung bewegte sich im Lauf` | — | **Wachen** |
| M-120, Prozessor | B7 `offline_render_nutzt_den_bestaetigten_zustand` | M-120, M-120b (Etappe 4a) | **gemessen** (4a); beide Hälften sind damit belegt, eine offene Frage steht in §11.7 N-A3 |
| K-47 (NAK-213, B28) | B28 `K-47 Gegenprobe nach oben: ein Rahmen der naechsten, unbekannten Fassung faellt am Leser` | K-47-audit | **gemessen** |
| K-47 | B28 `K-47: ein Rahmen der aktiven Fassung passiert den Leser`, `K-47: die C++-Fassungszahl steht auf der aktiven Registerfassung` | K-47-konstante-audit | **gemessen** |
| K-47, Wache | B28 `K-47: die aktive P1-Fassung steht im Register` | — | **Wache** |
| M-90, M-91 | — | — | unverändert (§8, §10.3); sie tragen die Zonen-id-Grenze nicht (EA-4) |
| §10.3 M-118 und M-120 | — | — | Die dortigen Zeilen „Lücke, nicht 4a" sind durch diese Tabelle abgelöst. |

### 11.4 Die Beine dieser Nacharbeit

Einzeln gefahren, kein voller Kanon. Alle C++-Ziele samt der Bundles `NakamaProbeeq_VST3` und `EqCopilot_VST3` wurden in einem Aufruf aus pwsh gebaut (`cmake` liegt nicht im Bash-`PATH`; Bau Exit 0, 180 s). Gefahren wurden die zehn Beine des Auftrags: B28 für N-12, B6 für D10 und N-9, B2, A12, B3c und A4 für D8 und, weil `DspKern.h` die Kernbibliothek neu übersetzt und der Prozessor sie einbindet, A14, A1, A16 und B7. Die Rohdatei nennt je C++-Bein das Binary gegen die jüngste geänderte oder von einem Rotbeweis zurückgespielte Quelle (`eq-copilot/plugin/dsp/DspKern.cpp`, 23:43:51), dazu die vollständige Ausgabe. Rohausgabe: `docs/beweise/roh/SONDE-015-audit-nacharbeit-beine.txt`.

| Bein | Ziel | Exit | Binary jünger | Letzte Zeile | Warum es hier läuft |
|---|---|---|---|---|---|
| **B28** | `EqCopSonde014BefundTest` | 0 | ja | SONDE-014 Befund und Maskierung: 81/81 gruen | N-12: K-47 folgt dem Register; seit Etappe 2 rot. |
| **B6** | `EqCopDspGoldenTest` | 0 | ja | DSP-GOLDEN OK (257 geprueft, 0 Fehler) | D10 (Abschnitt N und M-46) und N-9 (Abschnitt O). |
| **B2** | `EqCopStateMigrationTest` | 0 | ja | STATE-MIGRATION-TEST OK - 276 Pruefungen ok, 0 Fehler | D8 an beiden C++-Lesern, dazu der Korpus. |
| **B3c** | `EqCopSchemaTest` | 0 | ja | 152 bestanden, 0 gescheitert | Im Auftrag für D8 genannt: der C++-Leser von `state_report.dsp` (`berichtDtoPruefen`) liest das DTO über dieselbe `leseZonen`. |
| **A1** | `EqCopNullTest` | 0 | ja | NULLTEST OK | Linkt die neu übersetzte Kernbibliothek. |
| **A16** | `EqCopProbeeqNullTest` | 0 | ja | SONDE-NULLTEST OK - 57 Pruefungen ok, 0 Fehler | Prozessor mit dem neu übersetzten Kern. |
| **B7** | `EqCopTransactionTest` | 0 | ja | TRANSAKTION OK (167 geprueft, 0 Fehler) | Transaktionskern und Prozessor; fährt die vier neuen Preset-Fixtures als Preset-Laden in S2. |
| **A12** | `erzeuge_state_fixtures.py` | 0 | - | STATE-FIXTURES OK (71 Dateien bytegleich, keine verwaiste Datei) | Erzeuger und Korpus. |
| **A4** | `broker` (`cargo test`) | 0 | - | jeder Testblock `test result: ok`, 0 failed (der letzte ist der Doctest-Block mit 0 Tests) | Rust klassifiziert die neuen DTO-Fixtures (`dto_korpus_klassifiziert_wie_das_manifest`). |
| **A14** | `pruefe_kern_identitaetsfrei.py` | 0 | - | 25 ok, 0 Fehler | Der Kern trägt den neuen Testzugang; A14 baut ihn selbst neu. |

**Messwert M-118** (B6 Abschnitt N im Lauf der Rohdatei; 48 kHz, Blockgröße 128, je Region 8000 Callbacks und 21,333 s Audio):

| Region | Rechnende Bänke | Mittel | p99 | Maximum |
|---|---|---|---|---|
| Übergang, beide Pfade im Crossfade | 4 | 2,429 % Echtzeit | 7,459 % | 41,617 % |
| stationär | 2 | 1,197 % Echtzeit | 3,053 % | 30,086 % |

Das Mittel des Übergangs ist das 2,03-Fache des stationären Mittels: vier gegen zwei Bankdurchläufe. Es gibt keinen Deckel (R15); das Maximum ist ein Einzelwert unter dem Takt des Betriebssystems und kein Budget. Die Zeile „CPU-Last des Vier-Bank-Falls" aus §4.6 ist damit belegt.

### 11.5 Prüfliste `tools/dirigent/pruefliste.md`, abgehakt für die Audit-Nacharbeit

| Zeile | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Nicht berührt: keine Queue, kein Ring und keine Politik geändert. Der Messlauf N zählt `busy_retry`, statt ihn zu übergehen (`jede Publikation des Messlaufs kam durch`, 0 abgewiesen). |
| **B** Lebenszyklus | Nicht berührt. Die neuen Läufe fahren bestehende Paare vollständig: Einschalten und Ausschalten, Candidate Beginn und Ende, Hörmatrix hin und zurück (Abschnitt O); das Candidate-Ende nach dem gleichzeitigen Wechsel (M-46). |
| **C** Verträge und Längen | Die Zonen-id an beiden Rändern (−1, 0, 7, 8), jenseits des int-Bereichs (2^31) und an der größten sicheren JSON-Ganzzahl (2^53 − 1), mit demselben Grund in C++ (B2), Rust (A4) und Python (A12); die Prüfreihenfolge des Vertrags bleibt (EA-1). Eine nicht-endliche oder gebrochene id fällt unverändert vorher als `bereich`. |
| **D** Bau- und Prüfriegel | Die Beinrohdatei nennt je C++-Bein das Binary gegen die jüngste geänderte oder zurückgespielte Quelle, jede der zwölf Rotbeweisdateien dasselbe für ihren roten und grünen Lauf. A14 baut den Kern selbst neu; A12 prüft den Korpus bytegleich zur Neuerzeugung. |
| **E** Behauptung ≤ Messung | Die Ausgabezeilen von N sagen „vier rechnende Bänke" nur mit Identitäts- und Durchlaufprüfung; „je im Fade" ist gefallen. Kopf und Gate-Zeile von B6 nennen M-118 und M-120; die Runner-Behauptung von B6 nennt beide nicht und behauptet damit nichts Ungemessenes (§11.6 A-8). `git grep -n -e "je im Fade" -e "Vier-Bank-Fall wurde gemessen" -e "faehrt wirklich vier Baenke"` außerhalb von `docs/beweise/roh/` trifft nur das Auditpaket `docs/audits/2026-09-10-code-review/packets/audio-dsp.md`, das den alten Stand zitiert. Die Fixtures kommen vom Erzeuger. Jede neue Prüfung mit Zusage wurde einmal gebrochen (zwölf Dateien); was ohne eigenen Bruch mitläuft, heißt Wache (§11.3). |
| **F** Änderungssatz | Beide C++-Leser, Erzeuger, Fixtures, Manifest des Korpus und der Cross-Language-Verbraucher liegen im selben Änderungssatz; Rust und Python brauchten keine Änderung und laufen mit (A4, A12). Messwert, Rotbeweise und dieses Manifest ebenso. |

### 11.6 Abweichungen dieser Nacharbeit

| Nr. | Wortlaut | Was gebaut wurde | Begründung |
|---|---|---|---|
| A-1 | „fehlt ein Zugang für rechnende Bankidentitäten, ergänze ihn lesend in DspKern.h und begründe es in §11.6" | `DspKern::rechnendeSlots` (`eq-copilot/plugin/dsp/DspKern.h:207-223`), inline; liest `pfade[p]` und `istPassthrough` | `gefahreneSlots` (E-24) meldet die Quelle auch in einem Rampenübergang, in dem sie nicht gerechnet wird; der Rotbeweis M-118 wäre daran nicht gefallen. Das Produktverhalten ist unverändert, der Zugang liest nur. |
| A-2 | R-D8: „prüfen den Bereich … VOR dem Cast (Grund bereich, Detail wie bisher)" | Es gibt keinen Cast mehr; die Ablehnung bleibt in `validiereZonen` | EA-1 und EA-2 |
| A-3 | Rotbeweis D8: „wird mit einem anderen Grund als bereich oder gar nicht abgelehnt" | Rot am FE_INVALID-Flag; der Grund blieb `bereich` | EA-3: der erwartete Bruch hätte auf diesem Toolchain nicht stattgefunden — genau die Zufälligkeit, die der Audit benennt. |
| A-4 | Rotbeweisdateien „-M-90-audit.txt, -M-91-audit.txt (oder die IDs, die die Zusage tragen)" | M-66-dto-audit und M-66-preset-audit, dazu je ein Bruch am Rand und am Ziel | EA-4 und EA-11 |
| A-5 | M-46 nennt der Auftrag nur zum Lesen | Der Fall `vier_baenke_im_schlimmsten_fall` ist umgebaut | EA-8: ohne den Umbau stünde in B6 weiter eine Prüfung mit dem Fehler aus D10. |
| A-6 | „Die Ausgabezeile und die Manifesttexte (M-118, §4.6, §9.3-Zeile M-118, Kopftabelle, roh) sagen nur noch, was gemessen ist" | Ausgabezeilen und lebender Kopf sind nachgezogen; der Verlauf bleibt | Append-only (Kopf dieses Manifests, Prüfliste E): §3.14 M-118 trägt die Marke MA; §4.6 bleibt, seine Zeile „CPU-Last des Vier-Bank-Falls: Messwert aus B6 (M-118)" ist jetzt wahr; §9.3 führt keine Zeile M-118; die §10.3-Zeilen M-118 und M-120 sind durch §11.3 abgelöst; die Rohzeilen „MESSWERT Vier-Bank-Fall" der Etappen 3 und 4a bleiben Rohausgabe und sind in §11.7 N-A1 eingeordnet. |
| A-7 | Marke MA an M-118 | zusätzlich an M-46, M-66 und M-120 | Ihr Beleg ändert sich hier ebenso; die Marke macht die Zeilen für die Prüfung auffindbar. |
| A-8 | Runner-Behauptung von B6 in `tools/beweise.ps1` | unverändert | Sie nennt weder den Vier-Bank-Messwert noch M-120 und behauptet damit nichts, was nicht gemessen ist; `tools/beweise.ps1` liegt nicht im Pfadumfang dieses Auftrags. |
| A-9 | Pfadumfang `docs/beweise/roh/SONDE-015-audit-*` | Diagnose, Rotskript und Beinskript liegen unter diesem Muster | Das Rot- und das Beinskript sind die Werkzeuge dieser Sitzung; ihre Pfade in den Job-Ordner sind rechnerlokal, der Ablauf steht in ihnen vollständig. |

### 11.7 Nebenbefunde (nicht ins Register)

| Nr. | Befund | Wo gefunden | Was geschah |
|---|---|---|---|
| N-A1 | **Die bisherigen Messwerte „Vier-Bank-Fall" maßen zwei rechnende Bänke.** Etappe 3 meldete 0,922 bis 1,128 % Echtzeit, 4a 1,004 % (Blockgröße 256). | Audit D10; im Rotbeweis M-118 (rampenkompatibler Wechsel) meldet die „Übergangslast" 1,403 % gegen 1,194 % stationär, also nahezu dasselbe; im Beinlauf liegt die echte Übergangslast beim 2,03-Fachen der stationären (§11.4) | Die Rohzeilen bleiben; der neue Messwert steht in §11.4. |
| N-A2 | **M-46 trug dieselbe Klasse.** An der bisherigen Prüfstelle rechneten Committed Slot 0 und Candidate Slot 1, beide ohne Quelle; Slot 2 und 3 waren `bereit`. | `docs/beweise/roh/SONDE-015-audit-nacharbeit-m46-diagnose.txt` | Behoben (EA-8), Rotbeweis M-46-audit. |
| N-A3 | **Die Prozessorhälfte von M-120 unter Hostautomation hängt am Takt der Wanduhr.** `parameterValueChanged` schreibt nur Atomics; erst `dspKontrollTakt` im Analyseworker (frühestens alle 5 ms, spätestens nach dessen 20-ms-Warten, §10.1) schreibt das Automationsoverlay und publiziert, und der Kern nimmt die Publikation am nächsten Blockrand (`eq-copilot/plugin/sonde/SondeProcessor.cpp:1227-1257`). Bei gleichem Event- und Blockverlauf hängt der Block, an dem ein Automationspunkt wirkt, damit am Worker-Takt; ein Offline-Render ohne Takt nimmt ihn in Audiozeit später. B7 misst an M-120 nur, dass Offline den bestätigten Zustand nutzt. | Selbstaudit beim Bau von Abschnitt O | Nicht gemessen und nicht geändert: Prozessor, Etappe 4a und 4b, außerhalb des Pfadumfangs. Für die Prüfung der Etappe 4a benannt. |
| N-A4 | **B10 hält `kJsonSchemaMinor` gegen das Literal 5** (`eq-copilot/plugin/tests/IpcTestMain.cpp:5858`, `der_wire_envelope_traegt_die_fassung_fuenf`) — dieselbe Klasse wie N-12: heute grün, rot am nächsten Fassungsschritt. | Suche nach `kJsonSchemaMinor` vor dem Rotbeweis K-47-konstante | Nicht geändert (außerhalb des Pfadumfangs); für den nächsten Fassungsschritt benannt. |
| N-A5 | Unter der Rampenmutation (Rotbeweis M-120) fiel zusätzlich `die_rampe_folgt_der_ausgeschriebenen_ideallinie (M-17)`: auch der M-17-Lauf hat Lücken über einer Millisekunde zwischen seinen Blöcken. | `docs/beweise/roh/SONDE-015-rot-M-120-audit.txt` | Keine Handlung: dieselbe Rampe, dieselbe Zusage „Rampe in Samples". |
| N-A6 | **Der erste Rotbeweis „M-120 Verlauf" blieb grün.** Das dynamische Band lag bei 1,5 kHz im Side-Kanal, das Testsignal bei 99 und 16 Hz; der Detektor hörte nichts über dem Threshold, die Auslenkungen waren durchgehend 0, und `derselbe Parameterverlauf` verglich nur Nullen. | Rotbeweislauf der zweiten Runde | Behoben: das Band liegt auf dem Ton, das Signal pendelt mit seiner Hüllkurve durch Threshold und Knie, und die Wache `die dynamische Auslenkung bewegte sich im Lauf` hält das fest (−6 bis 0 dB). Alle vier B6-Rotbeweise liefen danach auf dem Endstand erneut; nur deren Dateien liegen bei. |
| N-A7 | `git diff` meldet für die geänderten Quellen „LF will be replaced by CRLF". | `git diff --stat` | Wie §10.7 N-8 keine Handlung; die Fixtures liegen LF im Index und im Arbeitsbaum. |

### 11.8 Wiederprüfung der Audit-Nacharbeit — PASS; D8, D10, N-12, N-9 geschlossen (Dirigent, 2026-09-11)

| Merkmal | Wert |
|---|---|
| Prüfer | Codex `gpt-6-astra`, Effort max, lesend; Thread `01a08d67-93a9-7800-8ea9-2e3de16e5434`; Lauf 00:19–00:26 |
| Prüfbereich | Wiederprüfung (Vorlage B) über `git diff 07dfcb77...4711dab0 -- eq-copilot/plugin/state/NakamaParameter.cpp eq-copilot/plugin/state/NakamaPreset.cpp eq-copilot/plugin/dsp/DspKern.h eq-copilot/plugin/tests/DspGoldenTestMain.cpp eq-copilot/plugin/tests/Sonde014BefundTest.cpp eq-copilot/plugin/tests/StateMigrationTestMain.cpp tools/eq-copilot/erzeuge_state_fixtures.py eq-copilot/fixtures/state`; HEAD während des Laufs `232bc3f5` (trägt nur den Prüfauftrag), vorher und nachher identisch |
| Auftrag und Urteil | `docs/beweise/roh/SONDE-015-audit-nacharbeit-wiederpruefung-auftrag.txt`; Urteil wörtlich `docs/beweise/roh/SONDE-015-audit-nacharbeit-wiederpruefung-4711dab.txt` |
| Urteil | **PASS** — D8 geschlossen (keine Double→Int-Konvertierung mehr, Fixtures für DTO und Preset, C++/Rust/Python klassifizieren gleich, Rotbeweis trifft mit FE_INVALID den unsicheren Cast selbst); D10 geschlossen (vier getrennte rechnende Bänke und vier Durchläufe je Übergangscallback, Übergangs- und stationäre Last getrennt; M-46 nach demselben Muster); N-12 geschlossen (K-47 ans Register gebunden, B28 81/81); N-9 für die B6-Hälfte geschlossen (Abschnitt O); nichts gebrochen, Fixture-Bytes und Zeilenenden erhalten, `DspKern.h:216-223` rein lesend |
| Rundenbilanz | `07dfcb77..4711dab0: Produkt 11 Datei(en) +1002/-2 · Tests 3 Datei(en) +574/-78 · Prüfwerkzeug 1 Datei(en) +22/-0 · Doku 17 Datei(en) +3430/-8` — Runde mit Produktfortschritt |
| Einordnung der Nebenbefunde §11.7 | N-A3 (Prozessorhälfte von M-120 hängt am Wanduhr-Takt des Analyseworkers): **Lücke**, wird mit der Nacharbeit 1 der Etappe 4a eingeordnet (dort liegt der Prozessor); N-A4 (B10 hält `kJsonSchemaMinor` gegen das Literal 5): **Härtung**, gehört in den nächsten Fassungsschritt (S29–31), datiert als Nebenbefund hier, kein Registerpunkt; N-A1, N-A2, N-A5, N-A6, N-A7 erledigt oder ohne Handlung |
| Etappenstand | Die Auditbefunde D8 und D10 innerhalb von SONDE-015 sind geschlossen; N-12 (B28 rot seit Etappe 2) und N-9 (M-118, M-120 B6-Hälfte) ebenfalls. **Als Nächstes NAK-246** (Auditbefunde D1–D7, D9 außerhalb von SONDE-015; `docs/beweise/roh/NAK-246-auftrag.txt`, Etappe 1 Matrix); danach `/code-review ultra` (Skill §3.7), Nacharbeit 1 der Etappe 4a (§10.8), Etappe 4b |

---
