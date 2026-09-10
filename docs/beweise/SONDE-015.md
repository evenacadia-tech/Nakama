# SONDE-015 — S26–28: Lokaler EQ-Kern in der Sonde, Zustand und Automation, A/B, Layout v2

| Merkmal | Wert |
|---|---|
| Ticket | S26–28, `SONDE-015` (Phase P6), Leitungsname „Die Klangregelung in der Sonde sicher und speicherbar machen" |
| Etappe | **Etappe 1 — Spezifikation vor Code, nach der Matrix-Konvergenzrunde (10.09.2026).** Dieses Manifest trägt Gate-Text, Ist-Stand, Verhaltensmatrix, Bauplan und Entscheide. **Kein Produkt-, Test-, Schema-, Fixture- oder Werkzeugcode in diesem Änderungssatz.** Zwei Prüfrunden: **Matrixprüfung 1** NEEDS_WORK mit zwölf Defekten (§7), eingearbeitet als **Matrix-Nacharbeit 1** (§7.1) — zwölf Zeilen korrigiert, drei neue (M-121 bis M-123), sechs Feinheiten neu gefasst, Abweichung 3 zurückgenommen. **Matrixprüfung 2** (§7.3) schloss zehn davon und fand **B-05** und **B-09** weiter offen; die **Matrix-Nacharbeit 2** (§7.4) hat sie geschlossen: vier Zeilen korrigiert, eine neue (**M-124**), drei Feinheiten neu gefasst. **Matrixprüfung 3** (§7.5) bestätigte B-05 und B-09 als geschlossen, fand aber **zwei Folgewidersprüche** (**D-1**, **D-2**); die **Matrix-Nacharbeit 3** (§7.6) schloss sie mit einer Regel. **Matrixprüfung 4** (§7.7) schloss D-2, ließ D-1 als **K-1** offen und fand **K-2** — dritte Wiederprüfung ohne PASS. Der **Konvergenzentscheid** benannte die **Ursache**: die Transaktionssemantik war über drei Runden als Prosa nachgezogen und nie als geschlossene Spezifikation festgelegt worden. Die **Matrix-Konvergenzrunde** (§7.8) hat den Weg gewechselt: **§5.11.4 ist jetzt eine geschlossene Protokollspezifikation** mit Arithmetik, Stufenfolge S0 bis S8, sechs Invarianten und einer vollständigen Falltabelle T1 bis T12; **M-44, M-74, M-75, M-76, M-123, M-124 und M-125 leiten sich daraus ab** und tragen keine eigene Arithmetik mehr (§5.17, Abweichung 7). Etappen 2 bis 4 (Verträge, DSP-Kern, Integration) beginnen erst nach der Wiederprüfung und dem Fortsetzungsauftrag des Dirigenten. **Etappe 2 (Verträge in drei Sprachen) ist seit dem 10.09.2026 gebaut; ihr Abschnitt ist §8.** **Etappe 2 (Verträge in drei Sprachen) ist seit dem 10.09.2026 gebaut (§8); Erstprüfung §8.8, Nacharbeit 1 §8.9, Wiederprüfung 1 §8.10, Nacharbeit 2 §8.11.** |
| Phase | **Etappe 2 gebaut; Erstprüfung und Wiederprüfung 1 NEEDS_WORK, Nacharbeiten 1 und 2 eingearbeitet.** Etappe 1 abgeschlossen nach fünf Matrixprüfungen und vier Nacharbeiten (§7 bis §7.9); die fünf Restwidersprüche der Transaktionstabelle sind als **NAK-245** ausgegliedert und gehören der Etappe 4. **Etappe 2 — Verträge in drei Sprachen** ist gebaut (§8). Ihre **Erstprüfung** endete NEEDS_WORK mit fünf bestätigten Defekten (§8.8) → **Nacharbeit 1** (§8.9); die **Wiederprüfung 1** schloss B-02 bis B-05 und ließ den B-01-Verbindungsrotbeweis offen (§8.10) → **Nacharbeit 2** (§8.11), zwei Dateien +101/−0, A4 und A4-SI grün, Rotbeweis in 0,51 s an der Zusage. Die Wiederprüfung 2 steht aus (Runde 2 von 3); danach folgt Etappe 3 (DSP-Kern als Bibliothek). |
| Urteil | **Offen.** Weder T1 noch T2 vergeben. Etappe 1 baut nichts und behauptet deshalb kein gemessenes Produktverhalten; ihr einziger Gegenstand ist, ob §3 die Zusagen des Gate-Textes vollständig und widerspruchsfrei trägt. |
| Prüfstufe | **T2 gefordert** (`docs/plan/plan.json`, Schritt S26–28, `"stufe": "T2"`). Heute vergeben: **keine**. |
| Prüfmodell | Codex `gpt-6-astra`, Effort **max**, lesend — für Matrix-, Erst-, Wieder- und Abschlussprüfung. `gpt-5.6-sol` max nur als Gegenprüfer. |
| Basis-SHA | **Etappe 1:** `e9dbf4b9c11d1cf72aebbecb835c9391a57a0532` — alle Zeilen- und Zeichenangaben in §1 und §2 gelten zu diesem Stand. **Etappe 2:** `2026031f0a8bb0baf12135020be654a82d354a6f`. **Nacharbeit 1:** `3df34963de3a887600f5f629fc637f52256ace78`. **Nacharbeit 2:** `c727e620c1d48bcee8ad698a27321980bbd7776e`; jeweils beim Start gemessen, `git status --short` leer. |
| Kanon vorher | **GRÜN 62/62, Exit 0 auf `1867cac2`** (Abschlusskanon NAK-230, Rohausgabe `docs/beweise/roh/NAK-230-1867cac.md`). Eigene Messung in dieser Sitzung: `git diff --stat 1867cac2..e9dbf4b9 -- eq-copilot broker tools` ist **leer**. Der Codestand des Basis-SHA ist mit dem beglaubigten Kanonstand identisch; die Commits dazwischen berühren nur `docs/`. |
| Kanon nachher | **Steht aus.** Etappe 2 fährt die Beine ihres Bauplans einzeln (§8.4), die Nacharbeit 1 die davon betroffenen (§8.9.3), die Nacharbeit 2 die zwei Broker-Beine (§8.11.2) — alle grün; der volle abgekoppelte Lauf gehört an das Ende der Etappe 4. |
| Ticketpfade | **Etappe 1:** diese eine Datei, `docs/beweise/SONDE-015.md`. **Etappe 2:** die Vertrags-, Plugin-, Broker- und Werkzeugpfade aus §4.5, ohne `eq-copilot/plugin/dsp/` (Etappe 3) und ohne `SondeProcessor.*` (Etappe 4) — der Prozessor bleibt nach §4.1 unberührt. Der Auftrag `docs/beweise/roh/SONDE-015-auftrag.txt` wird nicht angefasst. |
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
| M-44 | Kein Slot ist `free`, ein Programm soll gebaut werden | Der Befehl erhält **`busy_retry`** — er wird nicht gepuffert, nicht verworfen und nicht mit Gewalt durchgesetzt. **Der Aufrufer wiederholt mit derselben Transaktions-ID, und die Wiederholung kommt durch:** `busy_retry` ist §5.11.4 **T5** — kein Commit, keine Revision, **nicht memoisiert**. Weil `r` dabei unverändert bleibt, ist die alte `base_revision` weiterhin gültig, und der nächste Versuch erreicht S5 (§5.11.4 **T7**). Eine neue ID braucht der Aufrufer **nicht**. **MK:** die Zeile zitiert die Tabelle und rechnet nicht selbst. | **BELEGT** · **ENTSCHIEDEN (§5.11.4 T5, T7)** · BAULÜCKE · **MN3** · **MK** | **NEU** **B6**, Fall `busy_retry_wenn_kein_slot_frei`; **NEU** **B7**, Fälle `busy_retry_wird_nicht_gemerkt` (T5) und `wiederholung_nach_bankfreigabe_committet` (T7) | Ein fünfter Programmwunsch verdrängt eine aktive Bank; oder er wird still verworfen und die Transaktion meldet Erfolg; oder die Wiederholung nach der Freigabe bekommt erneut `busy_retry` aus dem Register statt gebaut zu werden | §44.2 wörtlich; R9; §5.11.4 T5 und T7; Matrixprüfung 3 D-2 |
| M-45 | Der ACK-Ring läuft trotz Überkapazität über | Der Ring **fasst mehr Einträge als es Slots gibt und droppt nie**. Ein dennoch erkannter Overflow setzt zusätzlich eine atomare `reclaim_pending_mask`; der betroffene Slot bleibt **dauerhaft nicht frei**, bis der Worker ihn bestätigt. Reclaim-Sicherheit gewinnt über Verfügbarkeit. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `reclaim_pending_mask_haelt_den_slot` | Nach einem erzwungenen Overflow wird der Slot wieder `free`, ohne dass der Worker ihn bestätigt hat | §44.2 wörtlich |
| M-46 | Gleichzeitiger Topologiewechsel auf Committed und Candidate | Der Fall braucht für die begrenzte Fadezeit drei, im schlechtesten Fall vier Bänke. Reicht das Budget nicht, wird **Candidate vor dem Wechsel neutral beendet** — nie eine Bank und nie ein Filterzustand zwischen beiden Pfaden geteilt. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `vier_baenke_im_schlimmsten_fall_und_candidate_endet_neutral` | Committed und Candidate teilen sich eine Bank; oder der Wechsel scheitert statt Candidate zu beenden | §44.2 wörtlich |
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
| M-61 | Direkt nach dem Remove — auch wenn das Band unter einer nachträglich eingeschalteten Zone lag | Der Undo-Ring trägt **ein** Objekt mit Slot-ID, Typ, `channel_mode`, Dynamic-Zustand und **sämtlichen** Werten. Undo stellt es als **ein** Objekt in **einer** Transaktion wieder her — Slot-ID und Band-ID sind dieselben wie vorher. **Der Zonenriegel hält Undo nicht auf:** eine Wiederherstellung eines ganzen, bereits bestätigten Zustands ist keine Bedienänderung (§5.6.3, §5.11.2). Der Pflichtweg der Zeile ist deshalb: Band einschalten → Zone darüber einschalten (M-68 erlaubt das) → Remove → **unmittelbar** Undo → das Band liegt wieder da, gilt als **verletzt** und wird so gemeldet. **MN1 (B-02):** vor der Nacharbeit hätte der für jede Transaktion vorgeschriebene Riegel genau diesen zugesagten Rückweg abgewiesen. | **BELEGT** · **ENTSCHIEDEN (B-02-Regel, §5.6.3)** · BAULÜCKE · **MN1** | **NEU** **B7**, Fälle `undo_stellt_das_entfernte_band_als_ein_objekt_her` und `undo_unter_einer_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung` | Undo stellt die Werte her, aber nicht `channel_mode`; oder es braucht zwei Schritte; oder das Band landet auf einem anderen Slot; oder der Zonenriegel weist das Undo ab und der zugesagte unmittelbare Rückweg fehlt | Fünferblock 02 `:136-150`, besonders `:139-141`; R5; Matrixprüfung 1 B-02 |
| M-62 | Alle acht Slots sind belegt, ein neuntes Band wird verlangt | Der Kern **überschreibt nichts** und meldet den Zustand „alle Slots belegt" als eigenen, unterscheidbaren Fehler. Er wählt nicht selbst ein Opfer. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fall `acht_belegte_slots_ueberschreiben_nichts` | Der Kern räumt den ältesten Slot; oder er meldet einen generischen Fehler, aus dem die Oberfläche nicht ableiten kann, was zu tun ist | Arbeitsmodus `:204-205` |
| M-63 | Host-Automation schreibt auf einen Parameter eines **freien** Slots | Der Wert **ändert sich** — Automation ist instanzgebunden und wird nicht blockiert —, aber weder die Belegung noch der Klang ändern sich. Gemessen wird beides: der Parameter trägt den neuen Wert, der Audioausgang ist bitgleich zum Lauf ohne die Automation. | **ENTSCHIEDEN (R5)** · BAULÜCKE | **NEU** **B7**, Fall `automation_auf_freien_slot_aendert_werte_nicht_klang` | Der Slot wird durch Automation belegt; oder die Automation wird abgewiesen und der Host sieht einen Parameter, der nicht folgt | R5; Erratum (n) „Host-Automation ist instanzgebunden" |
| M-64 | `occupied` soll automatisiert werden | `v2.band.<slot>.occupied` ist **kein Host-Parameter**: es steht nur im Kind `Dsp`, taucht nicht in der APVTS auf und ist damit von außen nicht automatisierbar. Belegung entsteht ausschließlich durch eine Transaktion. | **ENTSCHIEDEN (R5-Feinheit, §5.5)** · BAULÜCKE | **NEU** **B7**, Fall `occupied_ist_kein_hostparameter`; **B2** über den Save-Weg | `occupied` erscheint in der Hostparameterliste; oder eine Automationskurve gibt einen Slot frei und der Undo-Ring hat nichts davon | R5; §5.5 |
| M-65 | Ein v1-Stand (`dsp_schema_version` 1) wird geladen | Migration nach R5: `occupied` := `enabled` **oder** mindestens ein Wert des Slots weicht bitgenau vom Vertragsdefault ab. Ein v1-Stand verliert damit **kein** vom User gesetztes Band, und ein nie berührter Slot bleibt frei. Die Migration ist deterministisch und golden-fähig. | **ENTSCHIEDEN (R5)** · BAULÜCKE | **B2** erweitert, Fälle `migration_v1_setzt_occupied` und `migration_ist_golden`; **A12** für den Fixturekorpus | Ein Band mit `enabled=false`, aber gesetzter Frequenz gilt nach der Migration als frei und verschwindet; oder ein unberührter Slot gilt als belegt, weil ein Gleitkommavergleich mit Epsilon statt bitgenau prüft | R5 |

### 3.8 Schutz-Zonen (Gate: R6, Interview 23.08., §49.2 Gate 4)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-66 | Eine Zone wird angelegt | Die Liste liegt im Kind `Dsp`, ist **kein** Host-Parameter und fasst **0 bis 8** Einträge. Je Zone `id` (0 bis 7, im Bestand eindeutig), `low_hz`, `high_hz`, `enabled`. Das DTO prüft 20 Hz ≤ `low_hz` < `high_hz` ≤ 20 kHz; die Laufzeit kappt `high_hz` zusätzlich auf 0,45·fs, ohne den persistenten Wert zu ändern (§5.6). | **ENTSCHIEDEN (R6)** · BAULÜCKE | **NEU** **B7**, Fälle `zone_dto_grenzen` und `zone_laufzeitkappung_aendert_nichts_persistentes`; **A12** Negativfixtures | `low_hz` == `high_hz` wird angenommen; oder ein bei 96 kHz gesetzter 20-kHz-Zonenrand macht das Projekt bei 44,1 kHz unlesbar | R6; §5.6 |
| M-67 | Eine Transaktion lässt ein belegtes, eingeschaltetes Band **neu in eine** eingeschaltete Zone eintreten | Die Transaktion wird **abgewiesen**, mit dem eigenen Grund „User-Schutz" und **ohne halbe Anwendung**: der bestätigte Zustand, die Revision und der Hash bleiben unverändert. Das Prädikat wird **je Zone einzeln** ausgewertet: abgewiesen wird, sobald für **irgendeine** eingeschaltete Zone der Eintritt dieses Slots von falsch auf wahr kippt und die Transaktion mindestens eines der Felder `occupied`, `enabled` oder `freq_hz` dieses Slots ändert. **MN1 (B-03):** eine Sammelprüfung „liegt in irgendeiner Zone" liess ein in Zone A verletzt liegendes Band nach Zone B wandern, weil das Prädikat dabei wahr blieb. | **ENTSCHIEDEN (R6, B-03-Regel)** · BAULÜCKE · **MN1** | **NEU** **B7**, Fälle `band_in_zone_ziehen_wird_abgewiesen`, `einschalten_in_der_zone_wird_abgewiesen`, `abweisung_lässt_hash_und_revision_unverändert` und `von_zone_a_nach_zone_b_wird_abgewiesen` | Das Band landet in der Zone; oder die Frequenz wird gesetzt und `enabled` nicht, sodass ein halber Zustand entsteht; oder **der Zwei-Zonen-Fall passiert den Riegel:** Band bei 100 Hz, Zone A 80–120 Hz nachträglich eingeschaltet, Zone B 900–1100 Hz eingeschaltet, Frequenzänderung auf 1000 Hz wird angenommen | R6; Interview `:128-135` „Harte Sperre für alle", besonders `:132-133`; Matrixprüfung 1 B-03 |
| M-68 | Eine Zone wird **über** einem bereits liegenden Band eingeschaltet | Das ist **erlaubt**. Das Band gilt danach als **verletzt** und wird in `state_report.dsp` als solches gemeldet. Es klingt weiter, es wird nicht stumm geschaltet und nicht verschoben. | **ENTSCHIEDEN (R6)** · BAULÜCKE | **NEU** **B7**, Fall `zone_ueber_band_ist_erlaubt_und_meldet_verletzung` | Das Einschalten der Zone wird abgewiesen; oder das Band wird still deaktiviert | R6 wörtlich |
| M-69 | Ein bereits verletztes Band wird weiter bedient, etwa `gain_db` geändert | Die Transaktion geht **durch**: das Prädikat war schon wahr und kippt nicht. Ein verletztes Band bleibt bedienbar; nur der Weg **hinein** ist gesperrt. | **ENTSCHIEDEN (R6-Feinheit, §5.6)** · BAULÜCKE | **NEU** **B7**, Fall `verletztes_band_bleibt_bedienbar` | Jede Änderung an einem verletzten Band wird abgewiesen und der User kommt nicht mehr heraus | §5.6; R6 |
| M-70 | Host-Automation schiebt die Frequenz eines Bandes in eine eingeschaltete Zone | Die Automation **schreibt vorbei**: sie wird nicht verhindert. Es entsteht **dieselbe** Verletzungsmeldung wie in M-68 — nicht ein zweiter Meldeweg und nicht ein stiller Zustand. | **BELEGT** · **ENTSCHIEDEN (R6)** · BAULÜCKE | **NEU** **B7**, Fall `automation_verletzt_und_meldet_denselben_zustand` | Die Automation wird geklemmt und der Host sieht einen Parameter, der nicht folgt; oder die Verletzung bleibt unsichtbar | Interview `:252-263` „Ehrliche Grenze der harten Sperre"; R6 |
| M-71 | Eine Zone wird gelöst | „Lösen" ist entweder `enabled` = false **oder** das Entfernen des Eintrags, **beides** als Transaktion mit genau einer Revision. Nach dem Lösen ist kein Band mehr verletzt, und der Weg in den Bereich ist wieder frei. | **BELEGT** · BAULÜCKE | **NEU** **B7**, Fälle `zone_deaktivieren_ist_eine_transaktion` und `zone_entfernen_ist_eine_transaktion` | Das Entfernen läuft ohne Revision und der Hash bleibt stehen; oder nach dem Lösen gilt ein Band weiter als verletzt | Interview `:252-263`; R6 |
| M-72 | Der Zustand wird gespeichert oder berichtet | Zonen sind Teil des **DTO**, des **`state_hash`**, des **Presets** und von **`state_report.dsp`**. Vier Orte, eine Quelle: es gibt keine zweite Zonenliste. | **ENTSCHIEDEN (R6)** · BAULÜCKE | **B2**, **NEU** **B7**, **A12**, **A8** je einen Fall | Zwei Stände mit verschiedenen Zonen liefern denselben Hash; oder das Preset trägt Zonen, die der Bericht nicht kennt | R6 wörtlich |
| M-73 | Neun Zonen sollen angelegt werden | Der Kern **weist ab** — die Liste ist auf acht begrenzt, und die Begrenzung wird im DTO geprüft, nicht erst in der Oberfläche. Doppelte `id` werden ebenfalls abgewiesen. | **ENTSCHIEDEN (R6)** · BAULÜCKE | **NEU** **B7**, Fälle `neunte_zone_wird_abgewiesen` und `doppelte_zonen_id_wird_abgewiesen`; **A12** Negativfixtures | Die neunte Zone wird stillschweigend verworfen und der Aufrufer glaubt, sie sei angelegt | R6 |

### 3.9 Transaktion, Revision, Hash, Undo-Ring und Automation (Gate: R11, §44.3, §44.4, §44.5, §53.8)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-74 | Apply, Revert, Neutralisieren, Remove, Undo, Redo oder Preset-Laden | Jede dieser sieben Handlungen ist **eine atomare Transaktion mit genau einer neuen `state_revision`** — nicht null, nicht zwei; `r` steigt um 1 und sinkt nie, auch bei Undo und Redo nicht. Die **Stufenfolge S0 bis S8** und der Commit-Punkt stehen in §5.11.4 Teil 2; diese Zeile wiederholt sie **nicht**, sie misst sie. Zugesagt sind hier: genau eine Revision je Handlung (§5.11.4 **T6**), alle fehlbaren und allozierenden Stufen **vor** dem Commit-Punkt, und ein Registereintrag **ausschließlich** in S8 für eine committete Transaktion (Invariante **I4**). **MK:** bis zur Konvergenzrunde trug diese Zeile die Stufenliste ein zweites Mal — in drei Runden ist sie dabei dreimal von §5.11.4 abgewichen (§7.8). | **BELEGT** · **ENTSCHIEDEN (§5.11.4 Teil 2, T6, I4)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** | **NEU** **B7**, sieben Fälle `<handlung>_erzeugt_genau_eine_revision`, dazu `commit_erzeugt_genau_eine_revision_und_einen_eintrag` (T6) und `alle_fehlbaren_stufen_liegen_vor_dem_commit_punkt` | Neutralisieren zählt zwei Revisionen; oder Undo zählt gar keine; oder eine Stufe hinter dem Commit-Punkt kann fehlschlagen; oder ein Ausgang ohne Commit legt einen Registereintrag an (I4) | §44.3; §44.4 „`Neutralisieren` ist selbst eine versionierte Transaktion"; R11; §5.11.4 |
| M-75 | Ein Fehler tritt in **irgendeiner** fehlbaren Stufe eines Apply auf | Der bestätigte Zustand bleibt **unverändert**, der `state_hash` bleibt **unverändert**, `r` steigt **nicht**, der Undo-Ring bekommt **keinen** Eintrag — **und das Register auch nicht** (§5.11.4 **I4**; die Ausgänge **T2** bis **T5** sind sämtlich „nicht memoisiert"). Gemessen wird mit **je einem Einspritzpunkt pro fehlbarer Stufe** nach §5.11.4 Teil 2: **S1** bis **S7**, also `base_revision`, Validierung, Zonenriegel, Kandidaten-DTO, Programmbau, Hashbildung und Undo-Vorbereitung — **sieben** Punkte, alle vor dem Commit-Punkt. **S0 ist bewusst nicht darunter:** der Nachschlag liest ein vorallokiertes Register fester Größe, allokiert nicht und kann nicht fehlschlagen — ein Treffer ist ein Ergebnis, ein Fehltreffer der Normalfall. Ein Einspritzpunkt dort wäre ein Fehler, den es nicht geben kann, und er stünde vor allen anderen. **MK:** die Stufennamen kommen aus §5.11.4 Teil 2, nicht aus dieser Zeile. | **BELEGT** · **ENTSCHIEDEN (§5.11.4 Teil 2, I4)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** | **NEU** **B7**, sieben Fälle `fehler_in_S<n>_laesst_committed_und_hash_unveraendert`, dazu `fehler_hinterlaesst_keinen_registereintrag` und `nachschlag_allokiert_nicht_und_schlaegt_nicht_fehl` (Allokationszähler über 10.000 Nachschläge) | Nach einem Fehler in S6 ist `r` erhöht und der Committed-Zustand getauscht; oder ein Fehler in S7 hinterlässt einen halben Ringeintrag; oder ein abgewiesener Ausgang landet im Register und blockiert die Wiederholung; oder der Nachschlag allokiert | R11; §44.3; §5.11.4 |
| M-76 | 10.000 doppelte, vertauschte und veraltete Transaktionen, **fernsteuerungsfrei** gegen den lokalen Transaktionskern | Sie erzeugen **höchstens eine** gültige Revision und **niemals** einen Mischzustand. Der Kern entscheidet jede Eingabe nach der Falltabelle in §5.11.4 Teil 4 — und **nur** danach. Unter den 10.000 Aufrufen sind deshalb beide Sorten: Wiederholungen **committeter** Transaktionen, die im Fenster ihr altes Ergebnis bekommen (**T1**), und Wiederholungen **nicht-committeter** Ausgänge, die neu beurteilt werden (**T2** bis **T5**, **T7** bis **T9**). Eine veraltete `base_revision` ergibt Konflikt, gleichgültig ob die Nutzlast gültig wäre (**T2**, Stufenfolge S1 vor S2). **MK:** die Zeile nennt keine Zahl selbst; Fenster und Kapazität stehen in §5.11.4 Teil 1. | **BELEGT** · **ENTSCHIEDEN (§5.11.4 Teil 4)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** | **NEU** **B7**, Fall `zehntausend_transaktionen_erzeugen_hoechstens_eine_revision`, gefahren als **tabellengetriebener** Lauf über T1 bis T12 (M-125) | Zwei Revisionen entstehen; oder ein vertauschtes Paar hinterlässt Werte aus beiden Transaktionen; oder der Beweis läuft über den Fernweg-ACK statt gegen den lokalen Kern und ist damit in P6 nicht fahrbar | §44.5 wörtlich; `CLAUDE.md` „Zustands- und Parameterhoheit bleibt in der Audio führenden Instanz"; §5.11.4 |
| M-123 | Eine **committete** Transaktion wird nach ihrem Abschluss erneut angeboten | Liegt ihre `tid` im Fenster, liefert **S0** sofort das gespeicherte Ergebnis — dieselbe Revision `e`, denselben Hash — **ohne** zweite Revision und ohne eine weitere Stufe (§5.11.4 **T1**). Das Fenster ist `0 ≤ r − e ≤ 31`, also die letzten **32** committeten Transaktionen; Kapazität und Fensterrand sind dieselbe Zahl und stammen aus der Undo-Tiefe (§5.11.4 Teil 1, **I1** bis **I3**). Jenseits davon ist die `tid` unbekannt und die veraltete `base_revision` ergibt **Konflikt** (**T10**). Drei Pflichtfälle: der **Fensterrand als Paar** (T10 und T11 — bei `r = 33` ist `e = 1` draußen, `e = 2` drinnen), der **Mischfall** (T12 — 31 Abweisungen verdrängen T nicht) und der **Zonenablauf** aus MN2 (1000 Hz → T auf 1050 Hz → 500 Hz → T wiederholt → T's Ergebnis, kein „User-Schutz"). **MK:** die Zeile sagte bis zur Konvergenzrunde „höchstens 32 Revisionen hinter" **und** „nie mehr als 32 Einträge" — das ergab 33 gegen 32 und war K-1. | **ENTSCHIEDEN (§5.11.4 Teil 1, T1, T10, T11, T12)** · BAULÜCKE · **MN1** · **MN2** · **MN3** · **MK** | **NEU** **B7**, Fälle `wiederholung_im_fenster_liefert_dasselbe_ergebnis` (T1), `fensterrand_erste_transaktion_ist_draussen` (T10), `fensterrand_zweite_transaktion_ist_drinnen` (T11), `abweisungen_verdraengen_keinen_committeten_eintrag` (T12) und `wiederholung_unter_einer_zone_bekommt_nicht_user_schutz` | Die Wiederholung erzeugt eine zweite Revision; oder **bei `r = 33` liegt `e = 1` noch im Register** (dann ist die Kapazität 33 und I1 verletzt); oder **`e = 2` ist verdrängt** (dann ist das Fenster kleiner als die Undo-Tiefe); oder der Mischfall endet mit Konflikt; oder die Wiederholung läuft in den Zonenriegel | `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2321`; §44.5; §5.11.4; Matrixprüfung 4 K-1 |
| M-124 | Eine Transaktion endete **ohne Commit** und wird mit derselben `tid` erneut angeboten | Sie ist ein **neuer Versuch**: nichts wurde memoisiert (**I4**), also findet **S0** nichts, und die Eingabe durchläuft die volle Stufenfolge und erhält das Urteil des **aktuellen** Zustands. **Der Ausgang hängt davon ab, ob sich `r` zwischenzeitlich bewegt hat** — und genau hier liegt der Unterschied, den die Konvergenzrunde festgeschrieben hat. Nach **`busy_retry`** hat sich nichts bewegt: die Bankfreigabe ist ein Audio-ACK und **keine** Transaktion, die alte `base_revision` gilt weiter, die Wiederholung committet (**T7**). Nach **User-Schutz** dagegen ist das Lösen der Zone **selbst eine Transaktion** und hat `r + 1` erzeugt (M-71): dieselbe `tid` mit der **alten** `base_revision` ergibt **Konflikt** — und das ist **richtiges** Verhalten nach M-76 (**T8**); erst mit **aktualisierter** `base_revision` committet sie (**T9**). **MK:** der MN3-Rotbeweis „eine Wiederholung nach behobenem Zonenkonflikt wird weiter abgewiesen" hätte T8 als Fehler gewertet, obwohl T8 korrekt ist — das war K-2. Er ist auf T9 eingegrenzt, und T8 ist als eigener Fall dazugekommen. | **ENTSCHIEDEN (§5.11.4 T7, T8, T9, I4)** · BAULÜCKE · **MN2** · **MN3** · **MK** | **NEU** **B7**, Fälle `wiederholung_nach_bankfreigabe_committet` (T7), `retry_nach_zonenloesung_mit_alter_base_revision_ist_konflikt` (T8), `retry_nach_zonenloesung_mit_aktueller_base_revision_committet` (T9) und `register_ist_nicht_teil_des_zustands` (I5: Save/Load lässt es unberührt, der Hash ändert sich nicht) | Eine Wiederholung nach `busy_retry` bekommt das gespeicherte `busy_retry` zurück; oder **T8 committet trotz veralteter `base_revision`** und umgeht M-76; oder T9 wird weiter mit User-Schutz abgewiesen; oder das Register taucht im gespeicherten Zustand auf | §5.11.4 T7 bis T9; M-71; M-76; Matrixprüfung 4 K-2 |
| M-125 | Der Transaktionskern wird gegen die **ganze** Falltabelle gefahren | Für **jede** Zeile T1 bis T12 aus §5.11.4 Teil 4 stellt der Test den beschriebenen Zustand her, gibt die beschriebene Eingabe und prüft **alle vier** Spalten: Ausgang, `r` danach, memoisiert ja/nein und — bei einem Treffer — die zurückgegebene Revision und den Hash. Dazu die drei Invarianten als Wachen über den ganzen Lauf: `\|R\| = min(r, 32)` (**I1**), Fensterzugehörigkeit ⇔ `0 ≤ r − e ≤ 31` (**I2**) und „kein Registereintrag ohne Commit" (**I4**). Der Lauf ist **tabellengetrieben**: fällt eine Zeile aus der Tabelle, fällt der Test, und eine neue Zeile ohne Fall ist ein Übersetzungsfehler. **MK, neu:** ohne diese Zeile wäre die Tabelle eine Beschreibung; mit ihr ist sie eine Zusage. | **ENTSCHIEDEN (§5.11.4 Teil 4)** · BAULÜCKE · **MK** | **NEU** **B7**, Fall `falltabelle_vollstaendig_gefahren` (zwölf Zeilen, vier Spalten je Zeile, drei Invariantenwachen) | Eine Tabellenzeile hat keinen Fall; oder ein Fall prüft nur den Ausgang und nicht `memoisiert`, sodass I4 unbemerkt brechen kann; oder die Invariantenwachen laufen nur am Ende statt nach jeder Eingabe | §5.11.4 Teil 3 und Teil 4; `tools/dirigent/pruefliste.md` E |
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
| M-118 | CPU-Last des schlimmsten Falls | §44.2 nennt einen Worst-Case-CPU-Golden, §49.3 nennt **keinen** Zahlenwert. Es entsteht deshalb **kein erfundener Deckel**: die gemessene Last des Vier-Bank-Falls (Committed plus Candidate, gleichzeitiger Topologiewechsel, acht belegte dynamische Bänder) steht als **Messwert** im Manifest, und der Golden hält nur fest, dass der Fall **läuft** und keine Bank teilt. | **BELEGT** · **ENTSCHIEDEN (R15)** · BAULÜCKE | **NEU** **B6**, Fall `worst_case_vier_baenke_laeuft`; Messwert in §4.6 | Ein Zahlenbudget wird erfunden und der Golden fällt später an einer schnelleren Maschine grundlos grün oder an einer langsameren grundlos rot | R15 wörtlich; §49.3 |
| M-119 | Samplegenaue Automation | Ob kontinuierliche Werte samplegenau oder nur vom vorigen zum letzten Blockwert gerampt werden, entscheidet die **gemessene** Hostbridge-Capability (B3), nicht eine Annahme. Meldet sie `sample_accurate_automation=false`, rampt die Sonde ehrlich blockweise, Topologieautomation wird deaktiviert, und Realtime/Offline-Gleichheit wird nur für denselben Event- und Blockverlauf behauptet. | **BELEGT** · BAULÜCKE | **B3** unverändert; **NEU** **B7**, Fall `capability_entscheidet_den_automationspfad` | Die Sonde behauptet samplegenaue Automation, ohne dass die Capability sie meldet | §44.3 letzter Absatz; Blueprint §3.5 |
| M-120 | Realtime- und Offline-Render | Beide erzeugen **denselben Parameterverlauf** und damit innerhalb numerischer Toleranz denselben Ausgang, bei gleichem Event- und Blockverlauf. Offline-Render läuft mit dem bestätigten Zustand, nie mit einer Vorschau. | **BELEGT** · BAULÜCKE | **NEU** **B6**, Fall `realtime_und_offline_gleich`; **NEU** **B7**, Fall `offline_render_nutzt_den_bestaetigten_zustand` | Ein Offline-Render liefert einen anderen Verlauf, weil die Rampe an der Wanduhr statt an Samples hängt | §44.3; §60 „Offline-Render"; §44.4 |

### 3.15 Zählung nach Belegklasse

Gezählt nach der Matrix-Konvergenzrunde (10.09.2026), aus den Zeilen selbst, nicht abgeschrieben:

| Klasse | Zeilen | Anteil |
|---|---:|---:|
| **BELEGT** (aus verbindlicher Quelle, ganz oder teilweise) | 76 | 61 % |
| **ENTSCHIEDEN** (R1 bis R15, die Regeln B-01 bis B-12, D-1, D-2 und die Protokollspezifikation §5.11.4) | 81 | 65 % |
| davon **allein** durch einen Entscheid getragen | 49 | 39 % |
| **BAULÜCKE** | 125 | 100 % |
| **OFFEN** (Produktfrage) | 0 | 0 % |
| davon mit der Marke **MN1** (Nacharbeit 1) | 15 | 12 % |
| davon mit der Marke **MN2** (Nacharbeit 2) | 6 | 5 % |
| davon mit der Marke **MN3** (Nacharbeit 3) | 6 | 5 % |
| davon mit der Marke **MK** (Konvergenzrunde) | 7 | 6 % |

Mehrfachmarken sind normal: M-74, M-75, M-76 und M-123 tragen alle vier; M-124 trägt MN2, MN3 und MK; M-44 trägt MN3 und MK; M-15 trägt MN1 und MN2; M-125 nur MK. Der Verlauf der Zeilenzahl: **120** nach Etappe 1 (BELEGT 73, ENTSCHIEDEN 72, nur entschieden 47) → **123** nach der Nacharbeit 1 (76 / 78 / 47) → **124** nach der Nacharbeit 2 (76 / 79 / 48) → **124** nach der Nacharbeit 3 (76 / 80 / 48) → **125** heute (76 / 81 / 49). Die Konvergenzrunde fügt genau eine Zeile hinzu — **M-125**, die die Falltabelle aus §5.11.4 als ganze misst; ohne sie wäre die Tabelle eine Beschreibung und keine Zusage.

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
Revisionen (M-74 bis M-85, M-123), Belegung und Remove/Undo (M-58 bis M-65),
Zonen (M-66 bis M-73), Preset (M-95 bis M-100).

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


#### 5.11.4 Protokollspezifikation der Transaktion (geschlossen, Konvergenzrunde 10.09.2026)

Diese Spezifikation ersetzt die Prosa der Runden MN1 bis MN3. Sie ist die
**einzige** Stelle, an der die Transaktionsarithmetik definiert wird. **M-44,
M-74, M-75, M-76, M-123, M-124 und M-125 leiten sich aus ihr ab** — sie
zitieren, sie definieren nicht. Jede Zahl steht genau einmal: hier.

##### Teil 1 — Definitionen und Arithmetik

| Zeichen | Bedeutung |
|---|---|
| `r` | Aktuelle `state_revision` des bestätigten Zustands. Frischer Zustand: `r = 0`. Jede committete Transaktion erhöht `r` um genau 1 (M-74). `r` sinkt **nie** — auch Undo und Redo sind committete Transaktionen und zählen vorwärts. |
| `e` | Die von einer committeten Transaktion erzeugte Revision. Die erste Transaktion hat `e = 1`. |
| `tid` | Transaktions-ID, vom Aufrufer vergeben, je logischem Auftrag genau eine. |
| `base_revision` | Die Revision, gegen die der Aufrufer seine Transaktion gebildet hat. |
| `R` | Wiederholungsregister: eine Menge von Tripeln `(tid, e, state_hash)`, genau ein Tripel je committeter Transaktion im Fenster. |

**Fenster.** Ein Tripel `(tid, e, hash)` ist **im Fenster** genau dann, wenn

> `0 ≤ r − e ≤ 31`

also für die Revisionen `r−31 … r` — die **letzten 32** committeten
Transaktionen. Der Rand ist **inklusiv** an beiden Enden.

**Kapazität.** `R` hat **genau 32** vorallokierte Plätze. Weil jede committete
Transaktion genau eine Revision erzeugt und das Fenster genau 32 Revisionen
umfasst, sind **Kapazität und Fensterrand dieselbe Zahl**. Es gibt keinen
zweiten, unabhängig gezählten Deckel — genau dieser Widerspruch war K-1.

**Woher die 32 kommt.** Aus der Tiefe des Undo-Rings
(`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2321`, `undo_tiefe`,
`maximum: 32`). Der Undo-Ring kann die Revisionen `r … r−31` rückgängig machen —
**dieselbe** Menge. Ein Ergebnis wird also genau so lange vorgehalten, wie der
Zustand, auf den es sich bezieht, noch erreichbar ist. Eine Zahl, zwei
Verwendungen, kein erfundener Deckel.

**Belegung, ausgerechnet.** `|R| = min(r, 32)`.

| `r` | `\|R\|` | ältestes `e` im Fenster | jüngstes `e` |
|---:|---:|---:|---:|
| 1 | 1 | 1 | 1 |
| 32 | 32 | 1 | 32 |
| 33 | 32 | 2 | 33 |
| 34 | 32 | 3 | 34 |
| 100 | 32 | 69 | 100 |

**Verdrängung.** Sie geschieht **ausschließlich am Commit-Punkt**, betrifft
**genau einen** Eintrag und nur, wenn `r` nach dem Commit größer als 32 ist:
der Eintrag mit dem kleinsten `e` fällt heraus, weil für ihn `r − e = 32` gilt.
Bei `r = 33` verlässt `e = 1` das Fenster, `e = 2` bleibt (`33 − 2 = 31`).
Nicht-committete Ausgänge verdrängen **nichts**, denn sie legen nichts ab —
genau das war D-1.

##### Teil 2 — Die Stufenfolge

Genau eine Reihenfolge, hier definiert und nirgends sonst:

| Stufe | Was sie tut | fehlbar? | allokiert? |
|---|---|---|---|
| **S0** | `tid` in `R` nachschlagen. Treffer ⇒ Ausgang T1, **keine** weitere Stufe. | nein | nein |
| **S1** | `base_revision` gegen `r` prüfen. | ja (Konflikt) | nein |
| **S2** | DTO validieren (Typ, Bereich, Enum, Endlichkeit; Reihenfolge nach `nakama-state-v2.md:171`). | ja | ja |
| **S3** | Zonenriegel, nur bei Bedienänderungen (§5.6.3). | ja (User-Schutz) | nein |
| **S4** | Kandidaten-DTO vollständig bauen. | ja | ja |
| **S5** | Programm bauen; kein freier Slot ⇒ `busy_retry` (M-44). | ja | ja |
| **S6** | `state_hash` über das Kandidaten-DTO bilden. | ja | ja |
| **S7** | Undo-Eintrag vorbereiten (Schnappschuss des **noch** bestätigten Zustands). | ja | ja |
| **— COMMIT-PUNKT —** | Ab hier kann nichts mehr fehlschlagen und nichts mehr allozieren. | — | — |
| **S8** | `r` erhöhen, Committed tauschen, Hash und Undo-Eintrag übernehmen, Host-Dirty melden, `state_report` senden, **Registereintrag anlegen**, gegebenenfalls einen Eintrag verdrängen. | nein | nein |

**Warum `base_revision` (S1) vor der Validierung (S2) steht.** Eine veraltete
`base_revision` heißt, dass die ganze Nutzlast gegen einen Zustand gebildet
wurde, den es nicht mehr gibt; ihren Inhalt zu beurteilen wäre gegenstandslos.
Erst diese Reihenfolge macht die Falltabelle **eindeutig**: eine Transaktion
mit veralteter `base_revision` **und** ungültigem Wert hat genau einen Ausgang,
nämlich Konflikt. Bis zur Konvergenzrunde stand die Validierung davor, und die
beiden Fälle überlappten. An B-01 ändert das nichts: S1 ist weder fehlbar im
Sinne einer Allokation noch teuer, und alle allozierenden Stufen liegen
unverändert vor dem Commit-Punkt.

##### Teil 3 — Invarianten

| Nr. | Invariante |
|---|---|
| **I1** | `\|R\| = min(r, 32)`, immer. |
| **I2** | `(tid, e, hash) ∈ R` ⇔ `0 ≤ r − e ≤ 31`. |
| **I3** | Kapazität von `R` = Fensterspanne = Tiefe des Undo-Rings = **32**. Eine Zahl. |
| **I4** | Ein Registereintrag entsteht **ausschließlich** in S8 und nur für eine committete Transaktion. Kein Ausgang vor dem Commit-Punkt legt einen an. |
| **I5** | `R` ist **transient**: es lebt im Prozessor, stirbt mit ihm, steht nicht im Kind `Dsp`, nicht im DTO und nicht im `state_hash`. Begründung: eine Wiederholung kann nur von einem Aufrufer kommen, der die Transaktion noch für offen hält — und der stirbt mit derselben Sitzung. |
| **I6** | Genau ein Ausgang je Eingabe: die Zeilen T1 bis T12 sind vollständig und paarweise unvereinbar, weil S0 bis S5 in fester Reihenfolge geprüft werden und die erste zutreffende Stufe entscheidet. |

##### Teil 4 — Falltabelle: Zustand × Eingabe → Ausgang

Eingabe ist stets `(tid, base_revision, Nutzlast)`. „Erste Stufe" nennt die
Stufe, die den Ausgang bestimmt.

| Nr. | Zustand | Eingabe | Erste Stufe | Ausgang | `r` danach | memoisiert | Test (B7) | Rotbeweis an der Zusage |
|---|---|---|---|---|---|---|---|---|
| **T1** | `tid` liegt in `R` (`r − e ≤ 31`) | dieselbe `tid`, beliebige `base_revision`, beliebige Nutzlast | **S0** | **Gespeichertes Ergebnis**: Revision `e`, Hash aus `R` | `r` (unverändert) | bleibt | `wiederholung_im_fenster_liefert_dasselbe_ergebnis` | Es entsteht eine zweite Revision; oder die Eingabe durchläuft S1 bis S5 und bekommt ein Urteil des heutigen Zustands |
| **T2** | `tid` unbekannt | `base_revision ≠ r` | **S1** | **Konflikt** | `r` | nein | `veraltete_base_revision_ist_konflikt_ohne_wirkung` | Die Transaktion wird angewandt; oder der Konflikt landet in `R` und blockiert spätere Versuche |
| **T3** | `tid` unbekannt | `base_revision = r`, Nutzlast ungültig | **S2** | **Fehler** (Grund aus dem DTO-Vertrag) | `r` | nein | `ungueltige_nutzlast_faellt_und_wird_nicht_gemerkt` | Ein Bereichs- oder Enumfehler wird geklemmt statt abgelehnt; oder der Fehler wird memoisiert |
| **T4** | `tid` unbekannt, Band würde neu in eine eingeschaltete Zone eintreten | `base_revision = r`, Nutzlast gültig | **S3** | **User-Schutz** | `r` | nein | `zonenriegel_weist_ab_und_merkt_nichts` | Das Band landet in der Zone; oder die Abweisung wird memoisiert und sperrt jeden späteren Versuch |
| **T5** | `tid` unbekannt, alle vier Bänke belegt | `base_revision = r`, Nutzlast gültig, Zone frei | **S5** | **`busy_retry`** (M-44) | `r` | nein | `busy_retry_wird_nicht_gemerkt` | Eine aktive Bank wird verdrängt; oder `busy_retry` wird memoisiert und kommt für immer zurück |
| **T6** | `tid` unbekannt, alle Stufen bestanden | `base_revision = r`, Nutzlast gültig | **S8** | **Commit** | `r + 1` | **ja** (`tid`, `e = r+1`, Hash) | `commit_erzeugt_genau_eine_revision_und_einen_eintrag` | Zwei Revisionen; oder der Eintrag fehlt und die Wiederholung committet erneut |
| **T7** | wie T5, danach hat ein Audio-ACK eine Bank freigegeben (**keine** Revision) | **dieselbe** `tid`, `base_revision = r` — unverändert gültig, weil T5 keine Revision erzeugte | **S8** | **Commit** | `r + 1` | **ja** | `wiederholung_nach_bankfreigabe_committet` | Die Wiederholung bekommt erneut `busy_retry` aus dem Register und erreicht S5 nie |
| **T8** | wie T4, danach wurde die Zone gelöst — **das ist selbst eine Transaktion** und hat `r + 1` erzeugt (M-71) | **dieselbe** `tid`, **alte** `base_revision = r` | **S1** | **Konflikt** — und das ist **richtig** | `r + 1` | nein | `retry_nach_zonenloesung_mit_alter_base_revision_ist_konflikt` | Die Transaktion committet trotz veralteter `base_revision` und umgeht damit M-76 |
| **T9** | wie T8 | **dieselbe** `tid`, **aktualisierte** `base_revision = r + 1` | **S8** | **Commit** | `r + 2` | **ja** | `retry_nach_zonenloesung_mit_aktueller_base_revision_committet` | Die Wiederholung wird weiter mit User-Schutz abgewiesen, obwohl die Zone gelöst ist |
| **T10** | 33 committete Transaktionen, `r = 33` | Wiederholung der **ersten** (`e = 1`, `r − e = 32`) | **S1** (S0 findet nichts) | **Konflikt** | `r` | nein | `fensterrand_erste_transaktion_ist_draussen` | Die erste Transaktion liegt noch in `R` — dann wäre die Kapazität 33 und I1 verletzt |
| **T11** | dieselbe Lage wie T10 | Wiederholung der **zweiten** (`e = 2`, `r − e = 31`) | **S0** | **Gespeichertes Ergebnis** | `r` | bleibt | `fensterrand_zweite_transaktion_ist_drinnen` | Die zweite Transaktion ist verdrängt — dann ist das Fenster kleiner als die Undo-Tiefe und die Vorhaltegarantie gebrochen |
| **T12** | T committet (`e`), U committet (`e+1`, also `r = e+1`), danach **31 Abweisungen** wegen veralteter `base_revision` | Wiederholung von **T** | **S0** | **Gespeichertes Ergebnis von T** | `r` | bleibt | `abweisungen_verdraengen_keinen_committeten_eintrag` | T ist verdrängt und die Wiederholung endet mit Konflikt, obwohl seit T nur **eine** Revision vergangen ist und Undo T noch erreicht |

**T10 und T11 sind ein Paar** und messen beide Seiten desselben Randes; einzeln
belegt keiner von beiden, dass der Rand an der richtigen Stelle liegt. Dasselbe
gilt für **T8 und T9**: erst zusammen zeigen sie, dass nach einer gelösten Zone
nicht der Zonenriegel, sondern die `base_revision` über den Ausgang entscheidet.

**Was die Tabelle nicht sagt.** Sie regelt den lokalen Transaktionskern. Das
In-Flight-Register des `ControlClient` registriert **ausgehende**
P0-Nachrichten, löscht seinen Eintrag beim terminalen ACK und erwartet die
Idempotenz ausdrücklich vom **Broker**
(`eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:157-184`,
`:301-304`, `:332-336`). Es ist der **Fernweg-Anteil** und gehört S29–31; der
10.000-Befehle-Beweis aus §44.5 muss in P6 **fernsteuerungsfrei** fahrbar sein,
und die Zustandshoheit bleibt nach `CLAUDE.md` in der Audio führenden Instanz.


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

Sieben Stellen bauen etwas anderes, als der Wortlaut einer Regel sagt; **zwei
sind zurückgenommen** — Nummer 3 seit der Matrix-Nacharbeit 1, Nummer 6 seit
der Matrix-Nacharbeit 3. Beide bleiben als Verlauf stehen, damit die
Wiederprüfung die Kehre sieht. Heute wirksam sind also **fünf**. Jede steht mit
ihrer Begründung auch unter ihrer Regel; hier stehen sie zusammen, damit die
Prüfung sie an einer Stelle findet. Zwei fallen aus dem Muster: **Nummer 6**
weicht nicht vom Wortlaut einer R-Regel ab, sondern von einer früheren Regel
des Dirigenten — er hat sie in der Nacharbeit 3 selbst zurückgenommen. **Nummer
7** weicht von keiner Regel ab, sondern von der bis dahin gewählten **Form**;
sie ist der Wegwechsel der Konvergenzrunde und steht hier, weil eine
Formänderung dieser Größe genauso begründungspflichtig ist wie eine inhaltliche.

| Nr. | Regel und Wortlaut | Was gebaut wird | Begründung |
|---|---|---|---|
| 1 | **R1 / §53.8:** das Kind `Dsp` trägt „Revision, Schutz, bestätigter DspState, Undo-Ring" | `Dsp` trägt Revision, die acht `occupied`, die Zonen und den Undo-Ring — **keine** zweite Kopie der 112 Werte. Der bestätigte DspState ist `Parameters` plus `Dsp`. | Zwei Kopien derselben Werte können auseinanderlaufen; §33.5 verbietet eine zweite Wahrheit, und ein Leser müsste bei Widerspruch raten. Zusätzlich sprengte eine zweite Wertekopie den Knoten- und Byteheadroom des State-Lesers nicht, wohl aber die Klarheit. |
| 2 | **R6:** Zonengrenzen „mit 20 Hz ≤ low < high ≤ min(20 kHz, 0,45·fs) im DTO geprüft" | Das **DTO** prüft 20 Hz ≤ low < high ≤ 20 kHz; die **Laufzeit** kappt zusätzlich auf 0,45·fs, ohne den persistenten Wert zu ändern. | Die Samplerate ist beim Laden nicht bekannt. Eine samplerateabhängige DTO-Grenze machte einen bei 96 kHz gespeicherten Stand bei 44,1 kHz unlesbar — ein echter Save/Load-Bruch. Genau so hält es `freq_hz` seit v1 (`nakama-parameter-v1.json:13`). |
| 3 | **R13:** die Nutzlast ist „das validierte DSP-DTO" · **R2:** „`state_report.eq_enabled` bleibt reserviert; hier reist nichts davon" | **ZURÜCKGENOMMEN in MN1 (B-07).** Gebaut wird jetzt `dsp.jcs` — genau die gehashte RFC-8785-Zeichenkette, einschließlich `v2.global.eq_enabled` —, und beide Leser rechnen den Hash nach. Eine Teilmenge auf dem Draht gibt es nicht mehr. | Die vorige Fassung nahm den Schlüssel aus der Nutzlast heraus und musste dafür verbieten, den Hash nachzurechnen (damaliges M-105). Der Dirigent-Entscheid zu B-07 verlangt die hashverifizierbare Zeichenkette; eine Projektion ist damit unvereinbar. R2 bleibt gewahrt, weil kein **Feld** des reservierten Namens entsteht — und die Reservierungsregel schützt ausdrücklich den NAMEN (`reservierte-nachrichten-v1.json:242`). Ein Hash, den niemand prüfen kann, war die schwächere Lösung. |
| 4 | **R9:** „Nicht-endliche Eingangssamples … werden verriegelt und gezählt (Filterzustände bleiben endlich)" | Je Sample ein `isfinite` auf dem **Eingang**; je **Blockrand** eine Prüfung der Filterzustände mit Nullung und eigenem Zähler. | Alle Zustände je Sample zu prüfen kostet bei acht Bändern mal vier Kanalkomponenten ein Vielfaches der Filterarbeit selbst. Die zweistufige Fassung hält die Zusage und deckt zusätzlich den Fall, in dem ein Zustand ohne nicht-endlichen Eingang entgleist (extreme Q nahe Nyquist). |
| 5 | **R4:** der Ausgleich folgt der „pink-gewichteten mittleren **Betragsantwort**" | Gemittelt wird die **Energie** `\|H\|²` über dieselben 121 Gitterstellen, danach `−10 · log₁₀`. | Beide von R4 genannten Proben halten in beiden Lesarten exakt; sie trennen sich erst bei schmalen Merkmalen. Ein +12-dB-Bell mit Q 12 ergibt im Betragsmittel −0,28 dB, im Energiemittel −0,95 dB. Wahrgenommene Lautheit folgt der Energie in den Frequenzgruppen; das Betragsmittel unterkompensierte eine schmale Anhebung hörbar. |
| 6 | **MN2-Regel zu B-05:** „Das gespeicherte Ergebnis umfasst deshalb auch abgewiesene Transaktionen (Konflikt, User-Schutz)" | **ZURÜCKGENOMMEN in MN3 (D-Regel des Dirigenten).** Gemerkt wird ausschließlich, was eine Revision erzeugt hat; ein Ausgang ohne Commit hinterlässt keinen Eintrag, und dieselbe ID darf danach erneut laufen. Das Fenster ist in **Revisionen** definiert (höchstens 32 hinter der aktuellen), nicht in Einträgen. | Die zurückgenommene Fassung erzeugte zwei Widersprüche, die die Matrixprüfung 3 gefunden hat. **D-1:** ein Ring fester Eintragszahl, der auch Abweisungen aufnahm, verdrängte eine committete Transaktion T schon nach 31 Abweisungen — obwohl seit T erst **eine** Revision vergangen war und Undo T noch erreichte. Die zugesagte Spanne „über die ein alter Zustand per Undo noch erreichbar ist" war damit gebrochen. **D-2:** ein gemerktes `busy_retry` kam bei jeder Wiederholung derselben ID zurück, auch nachdem ein Audio-ACK längst eine Bank freigegeben hatte — der in M-44 zugesagte Weg „Der Aufrufer wiederholt" war unausführbar. Die neue Regel löst beides mit **einem** Satz, weil beide Widersprüche denselben Ursprung hatten: gemerkt wurde etwas, das gar keinen Zustand erzeugt hat. |
| 7 | **Die Arbeitsweise selbst:** Transaktionssemantik als Prosa in den Feinheiten von §5.11 | **Eine geschlossene Protokollspezifikation** (§5.11.4) mit Arithmetik, Stufenfolge, sechs Invarianten und einer vollständigen Falltabelle T1 bis T12. Die Matrixzeilen M-44, M-74, M-75, M-76, M-123, M-124 und M-125 werden daraus **abgeleitet** und zitieren sie; jede Zahl steht genau einmal. | Konvergenzentscheid des Dirigenten (10.09.2026, §7.7) nach drei Wiederprüfungen ohne PASS. Die Ursache war nicht ein einzelner falscher Satz, sondern die **Form**: drei Runden zogen Prosa nach, und jede schloss einen Satz und öffnete den nächsten Randfall (Verdrängung → `busy_retry` → inklusiver Fensterrand → `base_revision` beim Retry). Eine Reihenfolge, die an zwei Stellen steht, läuft auseinander; eine Zahl, die an zwei Stellen steht, widerspricht sich. Deshalb ist §5.11.4 ab hier die einzige Quelle, und die Zeilen tragen keine eigene Arithmetik mehr. |

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
