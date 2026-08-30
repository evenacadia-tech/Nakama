# Beweismanifest — SONDE-010 «v3-Control-/Telemetry-Clients und Rust-Envelopeparser»

<!-- NAKAMA-URTEIL: T1 PASS 2026-08-29 -->
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-29 nachgearbeitet -->

| Feld | Wert |
|---|---|
| Ticket | `SONDE-010` |
| Phase / Session | P2 / S14–15 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §65, Zeile `SONDE-010`: **„v3-Control-/Telemetry-Clients und Rust-Envelopeparser — fertig, wenn: CRC/Fuzz/Backpressure/Reconnect ohne P0-Starvation"** |
| Commits | `cdff93b` · `2ac23d0` · `1b19cd1` · `10a4806` · `97c956d` (Basis `a7b0740`) · **Nacharbeit Runde 1:** `4500785` · `602e105` · `6fc3224` · **Nacharbeit Runde 2:** `65d46a0` · `e5f5c27` · `a0053e4` · **Nacharbeit Runde 3:** `1bdb93d` · `c444ca3` · `66e4a09` |
| Datum | 2026-08-29 |
| Prüfstufen | T1 ☑ · T2 ☐ · T3 ☐ (kein Gate) |

> Das T1-Häkchen steht **nach** dem Lauf, nicht davor. T2 setzt später ein
> frischer Prüfer; hier steht bewusst keine T2-Marke.

---


> **Verlauf ausgegliedert (30.08.2026, NAK-100 / Dirigent §3.5):** Nacharbeitsrunden, Dirigentenstände und
> ältere Kanon-Rohläufe stehen byteweise unverändert in `SONDE-010-verlauf.md` (append-only, kein Prüfgegenstand;
> jeder Abschnitt gilt zu dem Stand, den er nennt). Hier bleiben Urteilsmarken, Behauptungen, Belege, der
> jüngste Kanon-Lauf und der jüngste Dirigentenstand. Stand der Ausgliederung: `dc7ce7d`.

## 0. Was gebaut wurde — und was ausdrücklich nicht

**Gebaut.** Der v3-Nachrichtenweg in beiden Sprachen: der 16-Byte-Envelope
(§33.1) als reine Funktion über Bytes in Rust *und* C++, der Bootstrap mit der
Kopplung zweier Verbindungen (§53.9), die vier Backpressure-Politiken, der
Pipename als reine Funktion mit dem Golden aus §48.3, und die beiden
C++-Clients `ControlClient` (P0/P1) und `TelemetryClient` (P2) mit eigenen
Threads, Reconnect-Backoff und vorallokierter SPSC-Übergabe.

**Nicht gebaut, bewusst.** Der `Coordinator` — Session, Eviction, Store,
Outbox — ist `SONDE-011`. Und: **Gen und Probeeq sprechen weiterhin v2.** Der
heutige Heartbeat-Pfad über `plugin/src/PipeClient` bleibt unangetastet der
Produktpfad; die neuen Clients sind gebaut, gelinkt und bewiesen, ihre
Produktverdrahtung folgt mit Broker-Coordinator und Landkarte
(`SONDE-011`/`SONDE-012`). Kein toter Code: das Kanon-Bein **B10** linkt und
fährt genau diesen Produktionsquellpfad, und **A22** fährt ihn über eine echte
Pipe gegen den echten Rust-Listener.

### Die eine technische Entscheidung, die das Ticket offen ließ

> „ob der Broker die SID-gebundene v3-Pipe bereits produktiv zusätzlich öffnet,
> entscheidest du technisch und begründest es im Manifest"

**Entscheidung: nein, in diesem Ticket nicht.** Ein zusätzlicher, SID-gebundener
v3-Endpunkt in `eqcop-broker.exe` wäre heute ein Endpunkt, der Verbindungen
annimmt, sein `welcome` schickt — und danach nichts damit anfangen kann: ohne
Coordinator gibt es keine Session, kein Register, keinen Store. Das ist genau
die Sorte Element, die das Grundgesetz „keine toten Elemente" verbietet, eine
Ebene unter der Oberfläche. Stattdessen:

* `transport::pipetoken` liegt fertig, ist eine reine Funktion und trägt das
  Golden aus §48.3 **in beiden Sprachen**;
* der Listener (`transport::server_v3`) ist vollständig gebaut und wird über
  einen **Probe-Namen** gefahren — nie über die Produktions-Pipe. Beide
  Programme des Lastbeins verweigern den Dienst, wenn ihnen
  `\\.\pipe\evenacadia.eq-copilot.v1` genannt wird;
* v2-Pipe, v2-Verhalten und Pipe-Sicherheit (SDDL nur aktueller User,
  `PIPE_REJECT_REMOTE_CLIENTS`, `FILE_FLAG_FIRST_PIPE_INSTANCE`) sind
  unverändert — der v3-Listener benutzt **dieselben** Sicherheitshelfer aus
  `server.rs`, damit es nur eine Wahrheit über die Pipe-Sicherheit gibt.

`SONDE-011` schaltet den Endpunkt zusammen mit dem Coordinator ein.

---

## Verhaltensmatrix — Referenz seit 2026-08-30 (Ursachenrunde NAK-95/NAK-98)

Die Spezifikation der Rückstau- und Lebenszykluspolitik des v3-Transports.
**Anforderungsquellen sind ausschließlich** Gate-Text §65 `SONDE-010`
(„CRC/Fuzz/Backpressure/Reconnect ohne P0-Starvation"), Entwurf §53.9
(Queuetabelle, Bootstrapreihenfolge), §33.1 (Envelope, zwei Verbindungen) und
die acht Regeln des Konvergenzentscheids am Ende dieses Manifests. Wo der
Entwurf schweigt, gilt die dort genannte Regel des Dirigenten; wo auch die
schweigt, steht die kleinste tragfähige Festlegung, markiert mit
**(Festlegung Worker)**. Die Matrix erfindet nichts darüber hinaus.

**Lesart der Spalten.** *Zusage* nennt, was passiert, was NIE passiert, und den
Zähler, der es sichtbar macht. *Test* nennt die Stelle, die es misst; „(neu)"
heißt: dieser Test existiert am Stand `05235cf` nicht und wird in Phase 2
gebaut. C++-Beleg ist `eq-copilot/plugin/tests/IpcTestMain.cpp` (Kanon-Bein
B10, Abschnitte A…H), Rust-Beleg der Modulname plus Testfunktion.

**Geltungsbereich beider Sprachen.** C++ hält die Client-Hälfte
(`eq-copilot/plugin/core/ipc/IpcQueues.h`), Rust die Broker-Hälfte
(`broker/src/transport/warteschlange.rs`). Im Listener verdrahtet ist heute nur
`IngressWarteschlange`; `P0/P1/P2Warteschlange` in Rust sind ausgelieferte
Politik ohne Verbraucher (der Coordinator ist `SONDE-011`). Regel 3 gilt
trotzdem: dieselbe Politik, derselbe Test, beidseitig.

### Pflichtzeilen — wo die acht Befunde des Konvergenzentscheids stehen

| Regel | Kurz | Zeile(n) |
|---|---|---|
| 1 | Wiederholpuffer fließt ohne Reconnect ab, in Annahmereihenfolge, vor jedem Neuzugang | `A-P1-06`, `A-P1-07` |
| 2 | Schlüssel überlebt den Wiederholpuffer; Koaleszierung gilt auch dort | `A-P1-03` |
| 3 | Rust verwirft nie Akzeptiertes; Politik identisch zu C++ | `A-P1-05` (und die Rust-Spalte jeder `A-P1`-Zeile) |
| 4 | P0 wird gelesen und beantwortet, während P1 rückstaut — in beiden Sprachen | `B-CC-06`, `B-CC-07`, `C-LS-07` (Nachtrag 30.08., Matrixprüfung) |
| 5 | Listener-Kopplung: Reihenfolge beim Verbinden und beim Trennen, je Callback genau einmal je Kopplung | `C-LS-02`, `C-LS-03`, `C-LS-04`, `C-LS-06` (Nachtrag 30.08., Matrixprüfung) |
| 6 | `stop()` kehrt immer zurück; kein Self-Join; Frist; danach kein Callback — in JEDEM Zustand | `B-CC-10`, `B-CC-11`, `B-CC-12`, `B-CC-16`, `B-CC-17`, `B-TC-07`, `B-TC-09` (Nachtrag 30.08., Matrixprüfung) |
| 7 | A21-Behauptung auf das Gemessene begrenzt | `D-A21-01` |
| 8 | P2: es fällt nie der neueste Frame, auch bei Slot-Kollision | `A-P2-03`, `A-P2-04` |

### Fristen und Kapazitäten dieser Matrix

| Größe | Wert | Quelle / Begründung |
|---|---:|---|
| `kCapP0` / `CAP_P0` | 64 | §53.9 |
| `kCapP1` / `CAP_P1` | 128 Hauptqueue + 128 Wiederholpuffer | §53.9 belegt nur die Hauptqueue 128; der Wiederholpuffer 128 ist **(Festlegung Worker)** — die Clienthälfte des Outboxgedankens (Nachtrag 30.08., Matrixprüfung) |
| `kCapP2JeSonde` / `CAP_P2_JE_SONDE` | 2 | §53.9; C++ hält 3 Slots — zwei wartende plus den, in den gerade geschrieben wird |
| `CAP_INGRESS` | 256 | §53.9 |
| `kIoFristMs` | 5000 ms | absolute Frist je Schreibvorgang und je Bootstrapantwort, `IpcVerbindung.h`; **(Festlegung Worker, übernommen aus dem heutigen Stand)** — §33.1 und §53.9 verlangen Fristen und Ratenlimits, legen die Zahlen aber nicht fest (Nachtrag 30.08., Matrixprüfung) |
| `kLeseTaktMs` (Control) | 20 ms | Lesetakt, wenn nichts zu senden ist; kurz genug, dass ein P0-Befehl nicht hinter Stille wartet; **(Festlegung Worker, übernommen aus dem heutigen Stand)** — §33.1 und §53.9 verlangen Fristen und Ratenlimits, legen die Zahlen aber nicht fest (Nachtrag 30.08., Matrixprüfung) |
| `kLeerlaufMs` (Telemetry) | 5 ms | Leerlauflesen statt Schlaf; bei 10 Hz Livekadenz (§33.2) reichlich; **(Festlegung Worker, übernommen aus dem heutigen Stand)** — §33.1 und §53.9 verlangen Fristen und Ratenlimits, legen die Zahlen aber nicht fest (Nachtrag 30.08., Matrixprüfung) |
| Backoff | 500 ms, verdoppelt bis 8000 ms | `IpcVerbindung.h`, wie im v2-Client; **(Festlegung Worker, übernommen aus dem heutigen Stand)** — §33.1 und §53.9 verlangen Fristen und Ratenlimits, legen die Zahlen aber nicht fest (Nachtrag 30.08., Matrixprüfung) |
| `SENKE_FRIST` (Rust) | 2000 ms | Frist auf jeden Lebenszyklus- und Diagnoseaufruf und auf jeden Join im Listener; **(Festlegung Worker, übernommen aus dem heutigen Stand)** — §33.1 und §53.9 verlangen Fristen und Ratenlimits, legen die Zahlen aber nicht fest (Nachtrag 30.08., Matrixprüfung) |
| `kStopFristMs` (C++, neu) | 2000 ms | **(Festlegung Worker)** derselbe Wert wie `SENKE_FRIST`: beide Seiten geben fremdem Code dieselbe Gnadenfrist; lang genug für einen normalen Callback (Mikro- bis Millisekunden), kurz genug, dass das Schließen eines Plugins im Host nicht spürbar hängt |
| Ratengrenze | 4000 Nachrichten je 1000 ms | `WireEnvelope.h`, §33.1 „Nachrichtenratenlimits", beidseitig gleich; **(Festlegung Worker, übernommen aus dem heutigen Stand)** — §33.1 und §53.9 verlangen Fristen und Ratenlimits, legen die Zahlen aber nicht fest (Nachtrag 30.08., Matrixprüfung) |

---

### A. Queues

#### A.1 P0 Control — Cap 64, „nichts verwerfen; Verbindung schließen"

| ID | Zustand | Ereignis | Zusage | Test C++ | Test Rust |
|---|---|---|---|---|---|
| `A-P0-01` | leer / teilweise | einreihen Ereignis | angenommen, ans Ende; Reihenfolge bleibt FIFO | E · „P0 nimmt 64 Nachrichten" | `warteschlange::p0_verwirft_nie_und_meldet_ueberlauf` |
| `A-P0-02` | leer / teilweise | einreihen Snapshot mit Schlüssel | P0 kennt keinen Schlüssel: wie ein Ereignis behandelt, NIE koalesziert; Steuerung wird nicht zusammengefasst | Typebene: `P0Warteschlange::einreihen` nimmt gar keinen Schlüssel entgegen (Abgrenzung, kein Test) | dieselbe Signatur in Rust (Abgrenzung, kein Test) |
| `A-P0-03` | voll (`groesse + inFlug == 64`) | einreihen | abgewiesen (`false` bzw. `Err(P0Ueberlauf)`); NIE fällt Akzeptiertes; Zähler `ueberlauf()`; die laufende Verbindung wird geschlossen. §53.9 nennt daneben „Preview lokal auslaufen lassen": ihr heutiger Haken ist der Zustand `getrennt` plus der Zähler `p0Ueberlaeufe`, an dem der P6-Preview-Pfad die Preview auslaufen lässt; heute leer, weil keine Preview existiert — der aktive EQ-Pfad kommt in P6, kein Code in dieser Runde | E · „die 65. meldet Ueberlauf (nichts verwerfen ⇒ Verbindung schliessen)" | `p0_verwirft_nie_und_meldet_ueberlauf` (Nachtrag 30.08., Matrixprüfung) |
| `A-P0-04` | teilweise / voll | entnehmen | Eintrag verlässt die Queue, sein Platz bleibt RESERVIERT (`inFlug` + 1) bis bestätigen oder zurücklegen; Invariante `groesse + inFlug` höchstens `kap` | E · „der Sender entnimmt den ersten Befehl und reserviert seinen Platz" | keine Entsprechung: die Rust-P0 ist Broker-Ingress ohne Writezusage (Abgrenzung) |
| `A-P0-05` | reserviert | bestätigen (auf dem Draht) | Platz frei; `p0Gesendet` + 1 | E · „64 neue Befehle waehrend des Writes finden KEINEN Platz — ehrlich gezaehlt" | entfällt, siehe `A-P0-04` |
| `A-P0-06` | reserviert | Verbindungsverlust oder gescheiterter Write | `zuruecklegen` an die ursprüngliche Position (vorn); dank der Reservierung ohne Fehlerfall; NIE geht ein entnommener Befehl verloren | E · „der gescheiterte Write legt ihn zurueck; die Queue traegt wieder genau 64" und „und zwar VORN — kein P0-Befehl ist verlorengegangen" | entfällt |
| `A-P0-07` | beliebig | Reconnect | Inhalt bleibt unverändert; P0 hat keinen Wiederholpuffer, weil nie etwas verworfen wurde | G9 · „nach dem Reconnect kommt JEDER angenommene Befehl an — auch der, dessen Write scheiterte" | entfällt |
| `A-P0-08` | beliebig | Stop | Inhalt bleibt im Objekt; nach `stop()` wird nichts mehr gesendet und nichts stillschweigend verworfen | G1 · „stop() trennt beide Verbindungen und kehrt zurueck" | entfällt |
| `A-P0-09` | geschlossen | entnehmen | liefert nichts mehr; das Schließflag wird VOR dem Inhalt geprüft | entfällt (die Clientqueue kennt kein Schließen) | `server_v3::geschlossener_eingang_liefert_nichts_mehr` |

#### A.2 P1 Zustand/Evidenz — Cap 128 plus Wiederholpuffer 128

§53.9: „Snapshots nach Objektschlüssel koaleszieren; nicht koaleszierbare
Events bei Überlauf über Reconnect/Outbox wiederholen." Die Outbox ist
`SONDE-011`; hier gelten der Reconnect-Weg und Regel 1.

| ID | Zustand | Ereignis | Zusage | Test C++ | Test Rust |
|---|---|---|---|---|---|
| `A-P1-01` | leer / teilweise | einreihen Ereignis (leerer Schlüssel) | angenommen, ans Ende; NIE überschrieben | E · „P1 nimmt Ereignis" | `warteschlange::p1_koalesziert_snapshots_an_ihrer_position` |
| `A-P1-02` | teilweise, Schlüssel K liegt in der Hauptqueue | einreihen Snapshot K | koalesziert an seiner Position, Inhalt ist der neuere, die Länge wächst nicht | E · „ein zweiter Snapshot desselben Objekts koalesziert, ohne zu wachsen" und „Koaleszieren behaelt die Position, tauscht nur den Inhalt" | `p1_koalesziert_snapshots_an_ihrer_position` |
| `A-P1-03` | **Regel 2** — Schlüssel K liegt im Wiederholpuffer | einreihen Snapshot K | koalesziert DORT, seine Position im Wiederholpuffer bleibt; der Schlüssel überlebt jeden Zwischenpuffer; NIE wird aus einem Snapshot ein schlüsselloses Ereignis, NIE wird der neuere Snapshot abgewiesen, während der ältere vorgehalten bleibt; kein Zähler wächst, Koaleszieren ist kein Überlauf | E · „ein Snapshot im Wiederholpuffer behaelt seinen Schluessel und koalesziert dort" (neu) | `p1_wiederholpuffer_haelt_den_schluessel` (neu) |
| `A-P1-04` | Hauptqueue voll, Wiederholpuffer hat Platz | einreihen Ereignis | `zurWiederholung` bzw. `ZurWiederholung`; Zähler `wiederholungen()`; die Nachricht ist ANGENOMMEN und geht später raus | E · „nicht koaleszierbare Ereignisse gehen in den Wiederholpuffer" | `p1_haelt_ereignisse_fuer_den_reconnect_vor` (Erwartung wird nach `A-P1-05` berichtigt) |
| `A-P1-05` | **Regel 3** — Hauptqueue voll UND Wiederholpuffer voll | einreihen | der NEUZUGANG wird abgewiesen (`abgewiesen`, in Rust gleichbedeutend), gezählt in `abgewiesene()`; NIE `pop_front()` auf dem Wiederholpuffer, NIE fällt Akzeptiertes; das Rust-Ergebnis `WiederholungVerdraengt` entfällt ersatzlos | E · „ein voller Wiederholpuffer weist das NEUE Ereignis ab, gezaehlt, nie still" | `p1_haelt_ereignisse_fuer_den_reconnect_vor` — muss auf Abweisen des Neuzugangs umgeschrieben werden; heute schreibt er den Verlust fest |
| `A-P1-06` | **Regel 1** — Wiederholpuffer nicht leer | entnehmen oder bestätigen, also Platz wird frei | der Wiederholpuffer fließt sofort ab, ohne Reconnect: die älteste Wiederholung zuerst, ans Ende der Hauptqueue, solange Platz ist; NIE bleibt eine Wiederholung liegen, während die Hauptqueue Platz hat | E · „der Wiederholpuffer fliesst ab, sobald Platz frei wird — ohne Reconnect" (neu) | `p1_wiederholpuffer_fliesst_ohne_reconnect_ab` (neu) |
| `A-P1-07` | **Regel 1** — Wiederholpuffer nicht leer, Hauptqueue hat Platz | einreihen (Neuzugang) | zuerst der Abfluss aus `A-P1-06`, dann das Urteil über den Neuzugang; ein Neuzugang überholt NIE eine bereits angenommene Wiederholung; damit gilt die Annahmereihenfolge über beide Puffer hinweg | E · „ein Neuzugang ueberholt keine Wiederholung" (neu) | `p1_neuzugang_ueberholt_keine_wiederholung` (neu) |
| `A-P1-08` | teilweise / voll | entnehmen | liefert Objektschlüssel UND Nachricht; der Platz bleibt reserviert (`inFlug`) | E · „entnehmen liefert Objektschluessel UND Nachricht" | keine Reservierung in Rust (Abgrenzung) |
| `A-P1-09` | reserviert, Ereignis | Write scheitert oder Verbindungsverlust | zurück an seine Position (vorn); dank der Reservierung ist dort Platz; kein Umweg über den Wiederholpuffer | E · „ein gescheiterter Write legt das Ereignis an seinen Platz zurueck" und „und es steht wieder VORN, nicht hinten" | entfällt |
| `A-P1-10` | reserviert, Snapshot K, ein neuerer K wartet | Write scheitert | der zurückgelegte Snapshot weicht dem neueren (`koalesziert`); der einzige Fall, in dem Angenommenes fällt — und es fällt gegen seinen eigenen Nachfolger, nie gegen eine fremde Nachricht | E · „ein zurueckgelegter Snapshot weicht dem neueren, statt ihn zu verdraengen" | entfällt |
| `A-P1-11` | Wiederholpuffer nicht leer | Reconnect | Sonderfall von `A-P1-06`: der Abfluss läuft beim Verbindungsaufbau erneut, in Annahmereihenfolge, vor jedem Neuzugang | E · „Reconnect holt beide zurueck" und „und zwar JEDES angenommene Ereignis, in der urspruenglichen Reihenfolge" | `p1_haelt_ereignisse_fuer_den_reconnect_vor` |
| `A-P1-12` | beliebig | Stop | Inhalt und Wiederholpuffer bleiben im Objekt; nach `stop()` wird nichts mehr gesendet; NIE wird beim Stop verworfen | G1 · „stop() trennt beide Verbindungen und kehrt zurueck" | entfällt |
| `A-P1-13` | beliebig | einreihen einer Nachricht über `kMaxPayloadBytes` | an der TÜR abgewiesen (`zuGross`), kommt nie in die Queue; sonst ließe sie jede neue Verbindung an derselben Stelle scheitern | G15 · „und ein P1 ebenso — der Aufrufer erfaehrt es sofort" | Längenriegel im Envelope: `v3::grenzen_des_rahmens` |

#### A.3 P2 Live je Probe — Cap 2, „ältesten ungesendeten Frame ersetzen"

C++ ist eine vorallokierte SPSC-Schleuse mit drei Slots und einem Besitz-Atomic
je Slot (§48.1); Rust hält dieselbe Politik als Queue.

| ID | Zustand | Ereignis | Zusage | Test C++ | Test Rust |
|---|---|---|---|---|---|
| `A-P2-01` | leer / teilweise | veröffentlichen | angenommen, ohne Allokation, ohne Lock, ohne Warten | E2 · „100 000 Uebergaben mit 0 Allokationen" samt Gegenprobe am selben Zähler | `warteschlange::p2_ersetzt_den_aeltesten_ungesendeten` |
| `A-P2-02` | voll (zwei wartende Frames) | veröffentlichen | der ÄLTESTE ungesendete Frame weicht; der Zähler `ersetzt` ist eine OBERE Schranke und behauptet nie weniger Verlust als eingetreten ist | E2 · „der dritte Frame ersetzt den aeltesten ungesendeten (Cap 2)" und „abgeholt wird der zweite, nicht der erste" | `p2_ersetzt_den_aeltesten_ungesendeten` |
| `A-P2-03` | **Regel 8** — der Zielplatz gehört gerade dem Verbraucher | veröffentlichen (Slot-Kollision) | die Position wird zum LOCH und der Frame geht in den nächsten Platz; der Verbraucher erkennt das Loch an der nicht passenden Folgenummer und überspringt es; NIE fällt der neueste Frame, NIE wird ein beanspruchter Platz beschrieben; der Zähler `kollisionsLoecher` belegt, dass der Fall wirklich eintritt | E2 · „der Erzeuger traf den beanspruchten Slot WIRKLICH — und hat ihn nicht beschrieben" | keine Entsprechung: die Rust-Queue liegt einthreadig hinter einem Mutex (Abgrenzung) |
| `A-P2-04` | **Regel 8** — beide erreichbaren Plätze beansprucht, weil der Verbraucher zwischen den zwei Ansprüchen des Erzeugers weiterrückt | veröffentlichen | der Erzeuger bekommt IMMER einen Platz; `beanspruchtVerworfen` bleibt 0 und ist die Wache über „replace-oldest wird nie replace-newest"; der Kollisionsfall wird DETERMINISTISCH erzwungen statt unter Last erwartet — NAK-98: unter Baulast fiel der Test mit 2 verworfenen neuesten Frames, einzeln fünfmal grün | E2 · „der NEUESTE Frame faellt dabei NIE" bleibt als Lastwache; dazu E2 · „erzwungene Slot-Kollision: der neueste Frame findet immer einen Platz" (neu, ohne Zeitfensterzufall) | entfällt |
| `A-P2-05` | beliebig | veröffentlichen eines Frames über der Slotgröße | gezählt verworfen (`zuGross`), NIE halb geschrieben | E2 · „ein Frame ueber der Slotgroesse wird gezaehlt verworfen, nie halb geschrieben" | Längenriegel im Envelope: `v3::grenzen_des_rahmens` |
| `A-P2-06` | teilweise, ein Loch liegt vor | abholen bzw. entnehmen | das Loch wird übersprungen; kein zerrissener Frame, keine falsche Länge, keine rückläufige und keine doppelte Folgenummer | E2 · „und keine ruecklaeufige oder doppelte Folgenummer" | `p2_ersetzt_den_aeltesten_ungesendeten` |
| `A-P2-07` | beliebig | Verbindungsverlust oder Reconnect | die Schleuse ist verbindungsunabhängig; was drin liegt, geht auf der neuen Verbindung raus; P2 ist per Vertrag verlusttolerant, sein Ausfall degradiert Analyse, nie Steuerung (§33.1) | G13 · „und koppelt sich mit ihnen neu — ohne dass je ein P2-Frame floss" | entfällt |
| `A-P2-08` | beliebig | Stop | nach `stop()` wird nichts mehr abgeholt; `veroeffentlichen` bleibt erlaubt, zählt weiter und stürzt nie ab — der erzeugende Worker darf den Stop nicht bemerken müssen | G1 · „stop() trennt beide Verbindungen und kehrt zurueck" | entfällt |

#### A.4 Broker-Ingress je Verbindung — Cap 256, „P2 zuerst droppen; P0-Überlauf trennt"

Die vierte Politik aus §53.9. Sie hat bewusst kein C++-Gegenstück: ein Client,
der seinen eigenen Ingress verwaltete, bildete eine Entscheidung des Servers nach.

| ID | Zustand | Ereignis | Zusage | Test C++ | Test Rust |
|---|---|---|---|---|---|
| `A-IN-01` | teilweise | einreihen beliebiger Familie | angenommen; der Höchststand wird gemeldet | entfällt | `warteschlange::ingress_droppt_p2_zuerst_und_trennt_bei_p0` |
| `A-IN-02` | voll, mindestens ein P2 enthalten | einreihen P0, P1 oder P2 | der ÄLTESTE P2 weicht, der Neuzugang wird angenommen; Zähler `p2_verworfen` | entfällt | `ingress_droppt_p2_zuerst_und_trennt_bei_p0`, `p2_flut_hungert_p0_nicht_aus` |
| `A-IN-03` | voll, kein P2 enthalten | einreihen P0 | Client trennen; NIE einen P0 still verwerfen | entfällt | `server_v3::p0_ueberlauf_trennt_die_verbindung` |
| `A-IN-04` | voll, kein P2 enthalten | einreihen P1 | die Verbindung wird GETRENNT wie bei P0; NIE fällt eine P1-Nachricht still, während die Verbindung gesund wirkt — der Sender könnte sie sonst nicht wiederholen (§53.9 P1 „über Reconnect/Outbox wiederholen"); Zähler `p1_ueberlauf_trennt`. Die Ingresszeile von §53.9 schweigt zu P1, deshalb die Regel des Dirigenten; die Ende-zu-Ende-Wiederholung NACH der Trennung ist Outbox und Coordinator (`SONDE-011`) und steht unter „bewusst nicht festgelegt" | entfällt | `ingress_voll_ohne_p2_trennt_auch_bei_p1` (neu) (Nachtrag 30.08., Matrixprüfung) |

---

### B. ControlClient und TelemetryClient

Zustände: **gestoppt** · **verbindend** · **verbunden** · **rückstauend**
(verbunden, aber ein Write steht oder eine Queue ist voll) · **stoppend**.
Callbacks sind `beiAntwort` und der `helloProvider`; beide laufen auf dem
Clientthread, nie im Audiothread.

#### B.1 ControlClient (P0/P1)

| ID | Zustand | Ereignis | Zusage, Callback-Reihenfolge, Frist | Test |
|---|---|---|---|---|
| `B-CC-01` | gestoppt | `start()` | Thread startet, Status `verbindet`; ein zweiter `start()` ist wirkungslos, NIE entsteht ein zweiter Thread | G1 · „Control verbindet und bekommt ein v3-gerahmtes welcome" |
| `B-CC-02` | verbindend | Hello mit ungültiger Adresse oder nicht endlichen Audiofeldern | es wird gar nicht erst verbunden; `letzterFehler` nennt den Vertragsbruch; Status `getrennt`, danach Backoff; NIE wird NaN oder Inf vor der Prüfung gewandelt | G8 · „nicht endliche Audiofelder werden VOR der Wandlung verriegelt" |
| `B-CC-03` | verbindend | `welcome` hält den vollständigen Vertrag | Status `verbunden`; erst danach liefert `kopplung()` `link_id` und `challenge`; vorher NIE erfundene Werte | G1 · „welcome liefert link_id und challenge"; G10 · vollständiger Vertrag |
| `B-CC-04` | verbindend | `reject`, vertragswidriges `welcome`, falsche Familie oder keine Antwort binnen `kIoFristMs` (5000 ms) | schließen, Status `getrennt`, `letzterFehler` benennt den Grund; Wiederversuch nach Backoff 500 ms, verdoppelt bis 8000 ms | G10 · „beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag" |
| `B-CC-05` | verbunden | Sendeentscheid je Runde | P0 zuerst, immer; P1 kommt erst dran, wenn P0 leer ist — die Clienthälfte von „kein P0 wartet hinter Daten" (§33.1) | G1 · „P0 kommt beim Server als P0-Familie an" und „P1 kommt als P1-Familie an" |
| `B-CC-06` | **Regel 4** — verbunden oder rückstauend | ein P0-ACK liegt an, während P1-Nachrichten warten | der Lesepfad läuft in JEDER Runde, bevor der nächste Frame geschrieben wird; er wird NIE übersprungen, weil noch etwas zu senden ist. Frist: Poll mit 0 ms nach einem erfolgreichen Send **(Festlegung Worker** — hält den Durchsatz, weil kein Send auf den Lesetakt wartet**)**, `kLeseTaktMs` (20 ms) nur, wenn nichts zu senden ist. Ein bereits eingetroffener ACK erreicht `beiAntwort` spätestens nach dem laufenden Write, also höchstens `kIoFristMs` (5000 ms) nach seinem Eintreffen; Zähler `empfangen` | G16 · „ein P0-ACK kommt an, waehrend P1 rueckstaut" (neu); die Rust-Hälfte steht mit getrennten Leser-, Verbraucher- und Schreiberthreads: `server_v3::blockierende_senke_haelt_den_leser_nicht_auf` |
| `B-CC-07` | **Regel 4** — rückstauend | P1-Write scheitert oder läuft in `kIoFristMs` | der Eintrag geht zurück (`A-P1-09` bzw. `A-P1-10`); die bereits vollständig empfangenen Frames werden noch gemeldet, BEVOR die Verbindung endet **(Festlegung Worker** — sonst ginge genau der ACK verloren, auf den der Aufrufer wartet**)**; danach schließen und Backoff | G16 · „ein vor dem Verbindungsende empfangener ACK geht nicht verloren" (neu) |
| `B-CC-08` | verbunden | P0-Überlauf während der stehenden Verbindung | die Verbindung wird geschlossen statt still gekürzt; der Abbruch läuft sofort über `ioAbbrechen`, nicht erst nach `kIoFristMs`; Zähler `p0Ueberlaeufe` | G5 · „die Verbindung wird deswegen geschlossen, nicht stillschweigend gekuerzt" |
| `B-CC-09` | verbunden | Envelope-Verstoß, P2 oder Nicht-JSON auf der Controlpipe, überschrittene Ratengrenze | Verbindung schließen; je eigener Zähler (`envelopeAbweisungen`, `familieAbweisungen`, `rateAbweisungen`); NIE erreicht ein Frame der falschen Familie den Aufrufer | G4, G11, G12 |
| `B-CC-10` | **Regel 6** — verbunden, kein Callback läuft | `stop()` von außen | Generation hochzählen, I/O abbrechen, Thread joinen, Verbindung schließen, Status `getrennt`; nach Rückkehr von `stop()` wird KEIN Callback mehr gerufen; `stop()` ist idempotent, auch aus dem Destruktor | G1 · „stop() trennt beide Verbindungen und kehrt zurueck" |
| `B-CC-11` | **Regel 6** — verbunden, der Aufruf kommt aus `beiAntwort` oder dem `helloProvider` | `stop()` (Reentranz) | KEIN Self-Join: der Aufruf markiert nur (`laeuft` auf false, Generation, `ioAbbrechen`) und kehrt sofort zurück; der Thread endet nach Rückkehr des Callbacks von selbst; den `join` holt der nächste `stop()` oder der Destruktor von außen nach; NIE `std::system_error` oder `std::terminate` | G17 · „stop() aus beiAntwort ist kein Self-Join und kehrt zurueck" (neu) |
| `B-CC-12` | **Regel 6** — stoppend, ein Callback blockiert | `stop()` von außen | der Join hat die Frist `kStopFristMs` (2000 ms, Begründung oben); läuft sie ab, wird der Thread ABGELÖST statt gejoint, `stop()` kehrt trotzdem zurück; der abgelöste Thread berührt danach nur noch gemeinsam gehaltenen Zustand, nie den Client **(Festlegung Worker** — dasselbe Muster wie `Senkenruf` und `join_mit_frist` im Listener**)**; sichtbarer Zähler `stopFristUeberschritten` im Snapshot; nach Rückkehr von `stop()` kein Callback mehr | G17 · „ein blockierender Callback haelt stop() hoechstens die Frist auf" (neu) |
| `B-CC-13` | gestoppt | `stop()` erneut, Destruktor | wirkungslos, kehrt sofort zurück | G1 · dieselbe Zeile |
| `B-CC-14` | verbunden | `reconnect()` | Generation hochzählen, I/O abbrechen, sofort zurück; der nächste Durchlauf sendet ein frisches Hello, ohne den Backoff zu verlängern | G3 · „der Client verbindet von selbst wieder (Backoff 500..8000 ms)" |
| `B-CC-15` | verbunden | Verbindungsverlust, Broker weg | Status `getrennt`, `letzterFehler`, `verbindungsVersuche` wächst ehrlich; P1-Wiederholpuffer und P0-Inhalt bleiben erhalten und gehen nach dem Reconnect raus | G3 · „nach dem Serverende faellt der Client auf getrennt" und „und zaehlt die Versuche ehrlich mit"; G6 · „das Ereignis, dessen Write scheiterte, kommt nach dem Reconnect an" |
| `B-CC-16` | **Regel 6** — verbindend (der `helloProvider` läuft, oder der Aufbau wartet auf das `welcome`) | `stop()` von außen | `ioAbbrechen` bricht das wartende Öffnen bzw. Lesen sofort ab; `stop()` kehrt binnen `kStopFristMs` (2000 ms) zurück, nie erst nach `kIoFristMs`; blockiert der `helloProvider` selbst, gilt `B-CC-12` — Ablösen statt Joinen, Zähler `stopFristUeberschritten`; nach Rückkehr kein Callback mehr | G17 · „stop() im Zustand verbindend kehrt binnen Frist zurueck" (neu) (Nachtrag 30.08., Matrixprüfung) |
| `B-CC-17` | **Regel 6** — rückstauend (ein Write steht in `kIoFristMs`) | `stop()` von außen | `ioAbbrechen` löst den laufenden Write sofort; der entnommene Eintrag geht nach `A-P0-06` bzw. `A-P1-09` an seinen Platz zurück, NIE geht er beim Stop verloren; `stop()` kehrt binnen `kStopFristMs` (2000 ms) zurück, nicht erst nach der Schreibfrist | G17 · „stop() waehrend eines blockierenden Writes kehrt binnen Frist zurueck" (neu) (Nachtrag 30.08., Matrixprüfung) |

#### B.2 TelemetryClient (P2)

| ID | Zustand | Ereignis | Zusage, Callback-Reihenfolge, Frist | Test |
|---|---|---|---|---|
| `B-TC-01` | gestoppt | `start()` ohne Kopplungswerte | Status `wartetAufKopplung` — ein EIGENER Zustand, nicht `getrennt`; es wird NICHT verbunden, weil ein ungekoppelter Connect geschlossen würde und nur einen Verbindungsslot verbrennt | G2 · „der Server weist die ungekoppelte Telemetrieverbindung ab" und „der Client gilt nicht als verbunden" |
| `B-TC-02` | wartetAufKopplung | Control liefert `link_id` und `challenge` | verbindet mit beiden Werten und derselben `runtime_nonce`; NIE werden Kopplungswerte erfunden | G1 · „Telemetry koppelt mit link_id + challenge + derselben runtime_nonce" |
| `B-TC-03` | verbindend | `welcome` mit fremder `link_id` oder Challenge | abgelehnt, die Verbindung endet; ein fremdes welcome bestätigt die Kopplung einer anderen Instanz | G7 · „der TelemetryClient prueft sein welcome vollstaendig" |
| `B-TC-04` | verbunden, Schleuse leer | die Kopplung wechselt, weil Control neu verbunden hat | die Verbindung endet und koppelt neu, auch OHNE eine einzige Veröffentlichung; Zähler `kopplungswechsel` | G13 · „die Telemetrie bemerkt die neuen Kopplungswerte im Leerlauf" |
| `B-TC-05` | verbunden, Schleuse leer | Leerlauf | LESEN mit `kLeerlaufMs` (5 ms) statt schlafen — ein Schlaf sähe den Pipe-Abschluss nicht; `stop` und `reconnect` brechen es über `ioAbbrechen` sofort ab | G13 · „ohne eine einzige Veroeffentlichung bemerkt der Client den Pipe-Abschluss" |
| `B-TC-06` | verbunden | P0- oder P1-Frame auf der Telemetriepipe | Verbindung schließen, `familieAbweisungen`; spiegelbildlich zu `B-CC-09` und zur Gegenrichtung im Broker | G14 · „was der TelemetryClient auf SEINER Verbindung annimmt"; Rust: `server_v3::p0_auf_der_telemetriepipe_wird_abgewiesen` |
| `B-TC-07` | **Regel 6** — verbunden, rückstauend (ein P2-Write blockiert) oder stoppend | `stop()` von außen, `stop()` aus dem `helloProvider`, blockierender Provider | wortgleich zu `B-CC-10`, `B-CC-11` und `B-CC-12`: kein Self-Join, Frist `kStopFristMs` (2000 ms), Ablösen statt Joinen, Zähler `stopFristUeberschritten`, nach Rückkehr kein Callback mehr. Blockiert ein P2-Write, bricht `stop()` ihn über `ioAbbrechen` ab und kehrt binnen `kStopFristMs` zurück, NIE erst nach `kIoFristMs` (5000 ms) — damit gilt Regel 6 in JEDEM Zustand beider Clients. (Nachtrag 30.08., Wiederprüfung) | G17 · „dieselbe Stop-Zusage gilt fuer den TelemetryClient" (neu) (Nachtrag 30.08., Matrixprüfung); G17 · „stop() waehrend eines blockierenden P2-Writes kehrt binnen Frist zurueck" (neu) (Nachtrag 30.08., Wiederprüfung) |
| `B-TC-08` | beliebig | `veroeffentlichen()` aus dem erzeugenden Worker | allokationsfrei, lockfrei, wartefrei; kehrt IMMER sofort zurück, auch während Verbindungsaufbau, Rückstau und Stop; NIE blockiert die Erzeugerseite auf I/O | E2 · „100 000 Uebergaben mit 0 Allokationen" samt Gegenprobe |
| `B-TC-09` | **Regel 6** — wartetAufKopplung oder verbindend | `stop()` von außen | im Zustand `wartetAufKopplung` kehrt `stop()` sofort zurück — der Thread wartet nur auf der Condvar und wird geweckt; im Zustand `verbindend` bricht `ioAbbrechen` das Öffnen bzw. das Lesen des `welcome` ab und `stop()` kehrt binnen `kStopFristMs` (2000 ms) zurück; in beiden Fällen danach kein Callback mehr | G17 · „stop() gilt auch vor der Kopplung und waehrend des Verbindens" (neu) (Nachtrag 30.08., Matrixprüfung) |
| `B-TC-10` | verbunden oder rückstauend, ein P2-Write läuft | Verbindungsverlust oder Kopplungswechsel | der Frame in Arbeit gilt als verloren — P2 ist per Vertrag verlusttolerant (§33.1); die Schleuse behält ihren Inhalt (`A-P2-07`); die Verbindung endet über `ioAbbrechen`, OHNE auf `kIoFristMs` (5000 ms) zu warten; der Reconnect läuft mit den AKTUELLEN Kopplungswerten; Zähler `verbindungsVersuche` | G18 · „die Telemetrie verliert die Verbindung mitten im Write" (neu, Prüfername `telemetrie_verliert_die_verbindung_mitten_im_write`) (Nachtrag 30.08., Matrixprüfung) |

---

### C. Listener `server_v3` — Bootstrap, Kopplung, Fristen

Drahtreihenfolge nach §53.9: Control-Hello (`u32`-präfigiertes JSON, höchstens
16 KiB) → v3-gerahmtes `welcome {link_id, challenge}` als P0-Frame → erst
danach Telemetry-Hello mit `connection_kind=telemetry`, derselben
`runtime_nonce`, `link_id` und der Challenge → dessen `welcome`, ebenfalls
v3-gerahmt. Danach ausschließlich der 16-Byte-Header (§33.1).

| ID | Zustand | Ereignis | Zusage, Reihenfolge, Frist | Test Rust |
|---|---|---|---|---|
| `C-LS-01` | horchend | ein Bootstrap-Hello trifft ein | `protocol=2` bleibt vollständig im v2-Parser und bekommt am v3-Endpunkt einen v2-gerahmten `reject`; ein Binärframe statt Hello und ein Hello über 16 KiB werden vor jeder Allokation geschlossen | `bootstrap::v2_hello_bleibt_v2`, `binaerframe_statt_hello_wird_abgelehnt`, `hello_ueber_16_kib_faellt_vor_dem_parser` |
| `C-LS-02` | **Regel 5** — Control-Hello gültig | Kopplung anmelden | Reihenfolge: `control_anmelden` → `control_verbunden` ABGESCHLOSSEN → erst dann verlässt das Welcome den Draht. NIE geht das Welcome vor dem abgeschlossenen `control_verbunden` raus; damit kann `telemetrie_gekoppelt` nicht auf einem anderen Thread vorlaufen. Je `link_id` läuft `control_verbunden` GENAU EINMAL | `server_v3::welcome_folgt_dem_abgeschlossenen_control_verbunden` (neu); `server_v3::connect_callbacks_je_kopplung_genau_einmal` (neu) (Nachtrag 30.08., Matrixprüfung) |
| `C-LS-03` | **Regel 5** — `control_verbunden` blockiert | die Frist läuft ab | Frist `SENKE_FRIST` (2000 ms); danach wird der Aufruf abgelöst, es verlässt KEIN Welcome den Draht, die Kopplung wird über `kopplung_loesen` abgebaut, die Verbindung endet; `lebenszyklus_abgeloest` + 1; ein `control_getrennt` folgt bewusst NICHT, weil der abgelöste `control_verbunden` noch steht — sichtbar an `lebenszyklus_uebersprungen` | `server_v3::stoppen_endet_auch_bei_haengendem_lebenszyklusaufruf`; Ergänzung „kein Welcome bei abgeloestem control_verbunden" (neu) |
| `C-LS-04` | **Regel 5** — Control gekoppelt, Telemetry-Hello trifft ein | koppeln | die Kopplung verlangt ALLE drei Merkmale (`link_id`, Challenge, gleiche `runtime_nonce`); Reihenfolge: koppeln → Welcome → `telemetrie_gekoppelt`; `telemetrie_gekoppelt` läuft NIE vor `control_verbunden`. Je `link_id` läuft `telemetrie_gekoppelt` HÖCHSTENS EINMAL: ein zweites Telemetry-Hello auf dieselbe `link_id` wird abgewiesen, ohne den Callback erneut auszulösen | `bootstrap::kopplung_verlangt_alle_drei_merkmale`; Reihenfolgeprobe in `welcome_folgt_dem_abgeschlossenen_control_verbunden` (neu); `server_v3::connect_callbacks_je_kopplung_genau_einmal` (neu) (Nachtrag 30.08., Matrixprüfung) |
| `C-LS-05` | horchend | ungekoppelter Telemetry-Connect | geschlossen, `abgewiesen` mit Grund; `geschlossen_bootstrap` + 1 | `bootstrap::kopplung_verlangt_alle_drei_merkmale`; C++-Gegenprobe G2 |
| `C-LS-06` | **Regel 5** — verbunden | Verbindungsende der Control-Seite | **Solange jede Senke ihre Frist hält**, läuft je Kopplung jeder Callback GENAU EINMAL, in der Reihenfolge Kopplung lösen (VOR den fristbegrenzten Joins, mitsamt Abbruch der I/O der mitfallenden Telemetrieverbindung) → `telemetrie_getrennt` → `control_getrennt`. Die Control-Seite wartet dafür höchstens `SENKE_FRIST` (2000 ms) auf das `telemetrie_getrennt` der Telemetrieseite und meldet danach ihr `control_getrennt` in jedem Fall, gezählt in `lebenszyklus_reihenfolge_verletzt` **(Festlegung Worker** — ohne Wartepunkt ist die Reihenfolge zwischen zwei Verbindungsthreads unbestimmt, ohne Obergrenze wäre der Stop unbegrenzt**)**. Überschreitet `telemetrie_getrennt` seine Frist, wird es ABGELÖST (Zähler `lebenszyklus_abgeloest`) und `control_getrennt` läuft TROTZDEM: ein abgelöster Trenn-Callback zählt als gelaufen, kein Trenn-Callback entfällt und keiner läuft doppelt. Folgeaufrufe entfallen AUSSCHLIESSLICH nach einem abgelösten `control_verbunden` (`C-LS-03`) — dann gibt es keine gültige Kopplung —, sichtbar an `lebenszyklus_uebersprungen`. (Nachtrag 30.08., Wiederprüfung) | Normalfall: `server_v3::trennreihenfolge_je_callback_genau_einmal` (neu); Fristfall: `server_v3::abgeloestes_telemetrie_getrennt_haelt_control_getrennt_nicht_auf` (neu) (Nachtrag 30.08., Wiederprüfung) und `server_v3::stoppen_endet_auch_bei_haengendem_lebenszyklusaufruf`; dazu `control_ende_beendet_die_telemetrie` und `kopplung_faellt_mit_dem_leserende_nicht_erst_nach_den_joins` (Nachtrag 30.08., Matrixprüfung) |
| `C-LS-07` | verbunden | die Senke blockiert in `p0`, `p1` oder `p2` | der Leser läuft weiter, der Ingress füllt sich bis 256 und droppt P2 zuerst; der Höchststand wird gemeldet; P0 hungert nicht aus. **Regel 4 auf der Rust-Seite:** blockiert die Senke in `p1`, werden P0-Frames weiter gelesen UND BEANTWORTET — der Antwortweg hängt nie hinter einem blockierten P1-Verbraucher. Diese Zusage gilt, SOLANGE der Ingress nicht voll ist (unter 256); ist er voll ohne P2, greifen `A-IN-03` und `A-IN-04` und die Verbindung wird getrennt — das ist kein Aushungern, sondern ein sichtbares Verbindungsende nach §53.9. (Nachtrag 30.08., Wiederprüfung) | `server_v3::blockierende_senke_haelt_den_leser_nicht_auf`; `server_v3::p0_wird_beantwortet_waehrend_p1_die_senke_blockiert` (neu) (Nachtrag 30.08., Matrixprüfung) — misst die P0-Antwort bei blockierter `p1`-Senke und Ingress unter der Grenze; ein zweiter Fall im selben Test zeigt, dass der 257. Frame nach `A-IN-03`/`A-IN-04` trennt (Nachtrag 30.08., Wiederprüfung) |
| `C-LS-08` | verbunden | `stoppen()` bei hängender Senke oder hängendem Schreiber | jeder Join hat die Frist `SENKE_FRIST` (2000 ms); danach wird der Thread abgelöst statt gejoint (`senke_abgeloest`, `schreiber_abgeloest`); `stoppen()` kehrt IMMER zurück | `server_v3::stoppen_endet_auch_bei_haengender_senke`, `stoppen_endet_auch_bei_haengendem_lebenszyklusaufruf` |
| `C-LS-09` | startend oder stoppend | Stop im Fenster vor der Bedienung; beendete Verbindungen | hängt nicht; beendete Verbindungsthreads werden geerntet, damit weder der Vektor noch die nativen Handles unbegrenzt wachsen | `server_v3::stop_im_fenster_vor_der_bedienung_haengt_nicht`, `beendete_verbindungen_werden_geerntet`, `acceptor_ueberlebt_die_verbindungsgrenze` |

---

### D. Prüfwerkzeug — die A21-Behauptung (Regel 7)

| ID | Ort | Zustand | Ereignis | Zusage | Test |
|---|---|---|---|---|---|
| `D-A21-01` | `tools/beweise.ps1`, Bein `A21` | Runner-Behauptung A21 am Stand vor dieser Runde | Kanonlauf A21 | Die Behauptung sagt nicht mehr, als der Test misst. Der Teilsatz „feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus" lautet künftig **„feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge"**. Kein Allokationszähler; der übrige Behauptungstext bleibt unverändert. Der Testname trägt dieselbe Begrenzung **(Festlegung Worker** — ein Testname ist dieselbe Art Behauptung wie die Runnerzeile**)** | heute `transport_fuzz::feindliche_laengen_loesen_keine_allokation_aus`; nach der Begrenzung `transport_fuzz::feindliche_laengen_enden_in_der_erwarteten_fehlerklasse` (Nachtrag 30.08., Matrixprüfung) |

---

### Was die Matrix bewusst nicht festlegt

* **Coordinator, Session, Store, Outbox** — `SONDE-011`. Der Wiederholpuffer
  hier ist die Clienthälfte des Outboxgedankens; die Outbox selbst und die
  at-least-once-Zustellung mit dem Killtest an jeder Outboxgrenze (§53.9)
  gehören dorthin.
* **„Ein blockierender Pipe-Write hält weder Coordinator noch Storelock"
  (§53.9)** — die Zusage gilt ab `SONDE-011`, weil es heute weder Coordinator
  noch Store gibt. Ihr heutiges Gegenstück steht in `C-LS-07` und `C-LS-08`: ein
  blockierender Write hält weder den Leser noch den Stop auf. (Nachtrag 30.08., Matrixprüfung).
* **Produktverdrahtung.** Gen und Probeeq sprechen weiterhin v2 über
  `plugin/src/PipeClient`; wann der Broker die SID-gebundene v3-Pipe produktiv
  öffnet, entscheidet `SONDE-011` (§0 dieses Manifests).
* **HMAC, Pairing, Replaycache und die Impersonation-Kette** — `SONDE-016`
  bzw. NAK-90. Die Kopplung hier ist `link_id + challenge + runtime_nonce` über
  eine per DACL begrenzte Pipe und ersetzt keine Signatur.
* **Broker→Main-Bündelung** mehrerer Quellen in einem Write (§33.1) und die
  Landkarte, die diese Frames verteilt — `SONDE-012`. Der TelemetryClient zählt
  sie heute und verwirft sie sichtbar.
* **P1 unter Ende-zu-Ende-Last** — NAK-91. Das Lastbein A22 fährt P0 und P2;
  die P1-Zeilen dieser Matrix sind Unittest- und Bein-Zusagen, keine Lastzusage.
* **Zahlen für Durchsatz und Latenz** über die Schranke des Lastbeins hinaus
  (P0-Antwortlatenz unter 1000 ms bei vollem P2-Rückstau). Die Matrix sagt,
  WAS passiert, nicht wie schnell.
* **Ein Race-Detector-Nachweis** für die P2-Schleuse. ThreadSanitizer gibt es
  für MSVC nicht; der Beweis bleibt strukturell (ein Atomic je Slot) plus
  äußere Vermessung (§6).
* **Reihenfolge zwischen zwei unabhängigen Kopplungen.** Die Matrix ordnet
  Callbacks nur INNERHALB einer `link_id`; zwischen verschiedenen Sitzungen
  gibt es keine Reihenfolgezusage.

---

## 1. Ticket-Behauptungen

| # | Behauptung (Gate-Text §65 / Lieferumfang) | Befehl | Ergebnis | Rohausgabe |
|---|---|---|---|---|
| 1 | **CRC** — CRC32C trifft die RFC-3720-Vektoren; P0/P1 tragen exakt 0, P2 die Pflichtsumme über genau die `payload_len` Payloadbytes; ein gekipptes Bit fällt. In **beiden** Sprachen. | `EqCopIpcTest` · `cargo test --lib transport::` | ✅ | [↓ B1](#b1), [↓ B2](#b2) |
| 2 | **Ein Korpus, zwei Leser** — C++ und Rust klassifizieren 37 Envelope-Fixtures identisch (Urteil **und** Verstoßmenge); jede der 14 Regeln hat ein Negativfixture. | `cargo test --test contract_cross_language envelope` · `EqCopIpcTest` | ✅ | [↓ B3](#b3), [↓ B1](#b1) |
| 3 | **Fuzz** — Längen, Header, Flags, CRC, Fragmentierung, Ratengrenzen; auch byteweise zugestellte Frames. | `cargo test --test transport_fuzz` | ✅ | [↓ B4](#b4) |
| 4 | **Bootstrap und Kopplung** — v2 bleibt v2; `protocol=3` bekommt ein v3-gerahmtes `welcome{link_id, challenge}`; die Telemetrieverbindung koppelt nur mit allen drei Merkmalen; ungekoppelter Connect und Binärframe statt Hello werden geschlossen. **v2-Isolation in beide Richtungen.** | `cargo test --lib transport::` · `EqCopIpcTest` | ✅ | [↓ B2](#b2), [↓ B1](#b1) |
| 5 | **Backpressure** — P0 64 / nichts verwerfen, P1 128 / koaleszieren + Wiederholpuffer, P2 je Sonde 2 / replace-oldest, Ingress 256 / P2 zuerst. Übergabe zur Telemetrie vorallokiert und ohne Allokation. Seit der Nacharbeit (§7.3) sind die Ingresspfade auch **im echten Listener** erreichbar: Leser, Verbraucher und Schreiber sind getrennte Threads. | `EqCopIpcTest` · `cargo test --lib transport::` | ✅ | [↓ B1](#b1), [↓ B2](#b2) |
| 6 | **Reconnect** — nach Broker-Neustart verbindet der Client von selbst wieder; ein kaputter Envelope und ein P0-Überlauf schließen die Verbindung, statt still zu kürzen. | `EqCopIpcTest` | ✅ | [↓ B1](#b1) |
| 7 | **Pipe-Token** — `S-1-5-21-111111111-222222222-333333333-1001 → BNSM62JZZCCXIDV3PJZAEHMZPA` in C++ **und** Rust. | `EqCopIpcTest` · `cargo test --lib transport::` | ✅ | [↓ B1](#b1), [↓ B2](#b2) |
| 8 | **„ohne P0-Starvation" als Zahl** — 32 Sonden fluten P2 über eine Probe-Pipe gegen den echten Rust-Listener; im Kanon-Lauf der Nacharbeit (`d137fa0`, Arbeitsbaum sauber) **10 016 von 10 016** P0-Frames beantwortet, max. **21 ms**, p99 **20 ms**, bei **43 133** ersetzten P2-Frames und einem Ingress-Höchststand von **3** (vor §7.3 strukturell nie über 1). | `py -3.13 tools/eq-copilot/pruefe_ipc_last.py` | ✅ | [↓ B5](#b5) |
| 9 | **Jedes neue Bein schlägt an** — A20, A21, A22 und B10 einmal absichtlich gebrochen; dazu **jede** der zehn Prüfungen aus der Nacharbeit — die Rohausgaben stehen bei ihrem Befund in §7. | siehe unten | ✅ | [↓ B6](#b6) |

---

## 2. Rohe Ausgaben

<a id="b1"></a>
### B1 · C++-Bein B10 — `EqCopIpcTest`

**Befehl:** `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` · **Datum:** 2026-08-29

```text
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  ok      MANIFEST.json gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\v3\envelope\MANIFEST.json]
  ok      Regelmenge und ihre Reihenfolge stimmen mit dem Manifest  [14 Regeln]
  ok      die SAMMELNDE Stufe 3 ist in Deklarationsreihenfolge namenssortiert
  ok      alle Fixtures klassifiziert wie das Manifest (Urteil UND Verstossmenge)  [8 gueltig / 29 ungueltig]
  ok      Korpus ist substanziell  [37 Fixtures]
  ok      jede der 14 Regeln hat mindestens ein Negativfixture  [14/14]

== B · CRC32C und Envelope-Rundlauf ==
  ok      CRC32C("") == 0
  ok      CRC32C("123456789") == 0xE3069283 (RFC 3720)
  ok      CRC32C(32x 0x00) == 0x8A9136AA
  ok      CRC32C(32x 0xFF) == 0x62A8AB43
  ok      P0-Frame geschrieben
  ok      P0 ist JSON und traegt CRC exakt 0
  ok      P2 traegt die Pflicht-CRC ueber genau die Payloadbytes
  ok      ein gekipptes Payloadbit faellt an der CRC
  ok      Payload an der Paketgrenze wird angenommen
  ok      ein Byte darueber wird nicht einmal geschrieben

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  ok      kein angenommener Zufallsframe verletzt eine Kopfregel  [0 angenommen / 40000 abgelehnt]
  ok      ein gekipptes Bit in einem GUELTIGEN Frame: was durchkommt, haelt jede Kopfregel  [7671 angenommen / 12329 abgelehnt]
  ok      3000 gekippte P2-Payloadbits: jedes einzelne faellt an der CRC
  ok      byteweise Zustellung liefert exakt dieselben 40 Frames  [40 Frames, Rest 0]
  ok      ein kaputter Frame im Strom wird gemeldet, nicht uebersprungen

== D · v3-Pipename als reine Funktion ==
  ok      Golden aus Entwurf §48.3
  ok      vollstaendiger Pipename
  ok      Schreibweise der SID ist egal (Grossschreibung vor dem Hashen)
  ok      verschiedene SIDs, verschiedene Token
  ok      SHA-256("abc") wie FIPS 180-4
  ok      SHA-256 des 56-Byte-Vektors (Zweiblockfall)
  ok      Base32 gegen die Testvektoren aus RFC 4648 §10
  ok      eigene Logon-SID ist wohlgeformt  [S-1-5-21-369263037-1611576211-2081231688-1001]

== E · Backpressure: P0 · P1 · P2 ==
  ok      P0 nimmt 64 Nachrichten
  ok      die 65. meldet Ueberlauf (nichts verwerfen ⇒ Verbindung schliessen)
  ok      alle 64 kommen unveraendert und in Reihenfolge zurueck
  ok      P1 nimmt Snapshot a
  ok      P1 nimmt Ereignis
  ok      P1 nimmt Snapshot b
  ok      ein zweiter Snapshot desselben Objekts koalesziert, ohne zu wachsen
  ok      Koaleszieren behaelt die Position, tauscht nur den Inhalt
  ok      nicht koaleszierbare Ereignisse gehen in den Wiederholpuffer
  ok      ein voller Wiederholpuffer verdraengt gezaehlt, nie still
  ok      Reconnect holt beide zurueck
  ok      und zwar in der urspruenglichen Reihenfolge

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  ok      der dritte Frame ersetzt den aeltesten ungesendeten (Cap 2)
  ok      abgeholt wird der zweite, nicht der erste  [2]
  ok      danach der dritte
  ok      danach ist die Schleuse leer
  ok      ein Frame ueber der Slotgroesse wird gezaehlt verworfen, nie halb geschrieben
  ok      100 000 Uebergaben mit 0 Allokationen  [0]
  ok      Gegenprobe: derselbe Zaehler sieht eine echte Allokation  [1]
  ok      unter Flut ist kein abgeholter Frame zerrissen  [29236 geholt, 0 zerrissen, 197342 ersetzt]

== F · Ratengrenze ==
  ok      5000 Nachrichten in derselben Millisekunde: 1000 gehen durch  [1000]
  ok      ein volles Fenster spaeter wieder 1000
  ok      gleitend: 1 Nachricht je 10 ms laeuft unbegrenzt

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  ok      Control verbindet und bekommt ein v3-gerahmtes welcome
  ok      welcome liefert link_id und challenge
  ok      Telemetry koppelt mit link_id + challenge + derselben runtime_nonce
  ok      P0 kommt beim Server als P0-Familie an
  ok      und die Antwort erreicht den Client
  ok      P1 kommt als P1-Familie an
  ok      P2 fliesst ueber die zweite Verbindung  [20 Frames]
  ok      beide Bootstrap-Hellos tragen dieselbe runtime_nonce
  ok      das Bootstrap-Hello ist v3 und nicht v2
  ok      stop() trennt beide Verbindungen und kehrt zurueck

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  ok      der Server weist die ungekoppelte Telemetrieverbindung ab
  ok      der Client gilt nicht als verbunden

== G3 · Reconnect nach Serverneustart ==
  ok      erste Verbindung steht
  ok      nach dem Serverende faellt der Client auf getrennt
  ok      der Client verbindet von selbst wieder (Backoff 500..8000 ms)
  ok      und zaehlt die Versuche ehrlich mit  [2]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  ok      der Client weist den Frame ab und verbindet nicht
  ok      Status bleibt ungleich 'verbunden'

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  ok      64 P0-Nachrichten passen
  ok      die 65. meldet Ueberlauf an den Aufrufer
  ok      der Ueberlauf ist gezaehlt
  ok      Verbindung steht (der Server liest nur nicht mehr)
  ok      ein 200-KiB-Befehl wird eingereiht
  ok      die Queue laeuft ueber, weil der Peer nicht liest
  ok      die Verbindung wird deswegen geschlossen, nicht stillschweigend gekuerzt  [P0-Ueberlauf: Verbindung wird geschlossen]

== H · Bootstrapgrenze und JSON-Riegel ==
  ok      16 KiB sind noch ein Bootstrap-Hello
  ok      ein Byte darueber nicht mehr
  ok      flaches Objekt wird gelesen
  ok      Zahlen kommen als Text zurueck
  ok      Verschachtelung wird ABGELEHNT, nicht geraten
  ok      Arrays ebenso
  ok      ein Escape wird abgelehnt statt interpretiert
  ok      doppelter Schluessel ist eine Ablehnung, keine 'letzter gewinnt'-Regel
  ok      Text hinter dem Objekt wird abgelehnt
  ok      das leere Objekt ist gueltig

ALLE PRUEFUNGEN GRUEN — 87 Pruefungen, 0 Fehler
EXIT_B10=0
```

<a id="b2"></a>
### B2 · Rust-Unittests des Transports

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --lib transport:: --color never` · **Datum:** 2026-08-29

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)

running 32 tests
test transport::bootstrap::tests::hex32_riegel ... ok
test transport::bootstrap::tests::binaerframe_statt_hello_wird_abgelehnt ... ok
test transport::bootstrap::tests::hello_ueber_16_kib_faellt_vor_dem_parser ... ok
test transport::legacy_v2::tests::v2_json_ist_kein_gueltiger_v3_envelope ... ok
test transport::legacy_v2::tests::echtes_v2_json_geht_weiterhin_durch ... ok
test transport::bootstrap::tests::offene_kopplungen_sind_gedeckelt ... ok
test transport::bootstrap::tests::v3_control_wird_vollstaendig_validiert ... ok
test transport::bootstrap::tests::unbekanntes_feld_im_hello_faellt ... ok
test transport::bootstrap::tests::koppeln_und_entkoppeln_sind_symmetrisch ... ok
test transport::legacy_v2::tests::v3_binaerframe_wird_nie_als_v2_json_gedeutet ... ok
test transport::bootstrap::tests::v2_hello_bleibt_v2 ... ok
test transport::bootstrap::tests::kopplung_verlangt_alle_drei_merkmale ... ok
test transport::pipetoken::tests::base32_gegen_rfc_4648 ... ok
test transport::pipetoken::tests::golden_aus_dem_entwurf ... ok
test transport::pipetoken::tests::schreibweise_der_sid_ist_egal ... ok
test transport::pipetoken::tests::tokenform_ist_stabil ... ok
test transport::pipetoken::tests::verschiedene_sids_verschiedene_token ... ok
test transport::v3::tests::crc32c_gegen_bekannte_vektoren ... ok
test transport::v3::tests::grenzen_des_rahmens ... ok
test transport::v3::tests::jeder_regelname_ist_rueckuebersetzbar ... ok
test transport::v3::tests::json_rundlauf ... ok
test transport::v3::tests::p2_traegt_pflicht_crc ... ok
test transport::v3::tests::ratengrenze_haelt_das_fenster ... ok
test transport::v3::tests::stromleser_vertraegt_byteweise_zustellung ... ok
test transport::v3::tests::stufe_drei_sammelt_und_sortiert ... ok
test transport::v3::tests::u32_ueberlauf_wird_nicht_akzeptiert ... ok
test transport::warteschlange::tests::ingress_droppt_p2_zuerst_und_trennt_bei_p0 ... ok
test transport::warteschlange::tests::p0_verwirft_nie_und_meldet_ueberlauf ... ok
test transport::warteschlange::tests::p1_haelt_ereignisse_fuer_den_reconnect_vor ... ok
test transport::warteschlange::tests::p1_koalesziert_snapshots_an_ihrer_position ... ok
test transport::warteschlange::tests::p2_ersetzt_den_aeltesten_ungesendeten ... ok
test transport::warteschlange::tests::p2_flut_hungert_p0_nicht_aus ... ok

test result: ok. 32 passed; 0 failed; 0 ignored; 0 measured; 90 filtered out; finished in 0.00s

EXIT=0
```

<a id="b3"></a>
### B3 · Envelope-Korpus in `contract_cross_language`

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --test contract_cross_language envelope --color never` · **Datum:** 2026-08-29

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-0cc75d7ee20a9bba.exe)

running 1 test
test envelope_korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 1 passed; 0 failed; 0 ignored; 0 measured; 8 filtered out; finished in 0.02s

EXIT=0
```

<a id="b4"></a>
### B4 · `transport_fuzz`

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` · **Datum:** 2026-08-29

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.07s
     Running tests\transport_fuzz.rs (broker\target\debug\deps\transport_fuzz-c05695697064a160.exe)

running 9 tests
test ein_kaputter_frame_im_strom_beendet_den_strom ... ok
test feindliche_laengen_loesen_keine_allokation_aus ... ok
test ratengrenze_haelt_unter_flut ... ok
test zufallsbytes_bringen_den_pruefer_nie_aus_dem_tritt ... ok
test jedes_gekippte_payloadbit_eines_p2_frames_faellt_an_der_crc ... ok
test payload_an_der_paketgrenze_wird_angenommen_darueber_nicht ... ok
test schreiben_und_pruefen_sind_zueinander_invers ... ok
test fragmentierung_aendert_das_ergebnis_nie ... ok
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.15s

EXIT=0
```

<a id="b5"></a>
### B5 · Ende-zu-Ende-Lastbein A22 — zwei Sprachen, ein Draht

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_ipc_last.py` · **Datum:** 2026-08-29

Der Aufbau: `eqcop-broker-v3probe.exe` öffnet den echten Rust-Listener auf einer
Probe-Pipe; `EqCopIpcLast.exe` fährt 32 **echte** `ControlClient`/
`TelemetryClient`-Paare dagegen. Ein Erzeugerthread flutet alle 32 P2-Schleusen,
ein zweiter sendet jeder Sonde alle 25 ms einen `heartbeat` und misst die Zeit
bis zum `heartbeat_ack`.

```text
Probe-Pipe: \\.\pipe\evenacadia.nakama.v3.last.22332.1787977039
  BEREIT \\.\pipe\evenacadia.nakama.v3.last.22332.1787977039
SONDE-010 | Lastbein: 32 Sonden, 8 s, Pipe \\.\pipe\evenacadia.nakama.v3.last.22332.1787977039
  verbunden: 32 Control, 32 Telemetrie
{"sonden":32,"sekunden":8,"p2_veroeffentlicht":133568,"p2_gesendet":92903,"p2_ersetzt":40800,"p0_gesendet":10048,"p0_beantwortet":10048,"p0_latenz_max_ms":21,"p0_latenz_p99_ms":20,"p0_ueberlaeufe":0,"envelope_abweisungen":0}
  ok      P0-Befehle wurden ueberhaupt gesendet  [10048]
  ok      KEIN P0-Frame geht verloren  [10048/10048]
  ok      P0-Antwortlatenz bleibt beschraenkt  [max 21 ms, p99 20 ms, Schranke 1000 ms]
  ok      es lag WIRKLICH Rueckstau an (Cap 2 hat ersetzt)  [40800 ersetzte P2-Frames]
  ok      kein Envelope wurde abgewiesen  [0]
  ok      keine P0-Queue lief ueber  [0]
LASTBEIN GRUEN
Brokerbericht: {"abweisungen": 0, "angenommen": 64, "control_verbindungen": 32, "geschlossen_bootstrap": 0, "geschlossen_envelope": 0, "geschlossen_p0_ueberlauf": 0, "geschlossen_rate": 0, "ingress_p1_verworfen": 0, "ingress_p2_verworfen": 0, "letzter_grund": "", "p0": 10048, "p0_beantwortet": 10048, "p1": 0, "p2": 92903, "pipe": "\\\\.\\pipe\\evenacadia.nakama.v3.last.22332.1787977039", "telemetrie_verbindungen": 32}
  ok      das C++-Lastbein selbst ist gruen  [Exit 0]
  ok      der Rust-Broker hat einen Bericht geliefert  [16 Felder]
  ok      32 Control-Verbindungen im Broker angekommen  [32]
  ok      32 Telemetrieverbindungen GEKOPPELT (nicht nur verbunden)  [32]
  ok      der Broker hat P0 gesehen und beantwortet  [10048 empfangen / 10048 beantwortet]
  ok      und P2 ueber die zweiten Verbindungen  [92903]
  ok      keine Verbindung wegen Envelope, Rate oder P0-Ueberlauf geschlossen  [envelope=0 rate=0 p0=0]
  ok      kein P0-Frame geht verloren (Clientseite)  [10048/10048]
  ok      es lag wirklich Rueckstau an  [40800 ersetzte P2-Frames]
GRUEN
EXIT=0
```

**Was die Zahlen sagen.** 133 568 P2-Frames veröffentlicht, 92 903 gesendet,
40 800 vom Cap-2-Ersetzen verworfen — der Rückstau war also real und nicht
behauptet. Währenddessen: 10 048 P0-Frames gesendet, **10 048** beantwortet,
Maximum **21 ms**, p99 **20 ms** bei einer Schranke von 1000 ms. Keine
Verbindung wurde wegen Envelope, Rate oder P0-Überlauf geschlossen.

Der strukturelle Grund steht im Design und wird hier **gemessen**, nicht
erfunden: P0 und P2 laufen über getrennte Pipe-Verbindungen (§33.1), ein
Telemetriestau kann Steuerung deshalb gar nicht erst blockieren. Das Lastbein
zeigt, dass diese Trennung im gebauten Weg auch wirklich trägt.

**Ein ehrlicher Zwischenstand.** Ein Lauf dieses Beins war zwischendurch ROT —
`9920/10048`. Ursache war kein Fehler im Weg, sondern ein **veraltetes
Artefakt**: der Probe-Broker war noch aus dem absichtlich gebrochenen Stand
gebaut (jeder 100. Heartbeat unbeantwortet; 4 × 32 = 128 fehlende ACKs, exakt
die Differenz). Genau dafür baut `tools/beweise.ps1 -Bauen` den Probe-Broker
jetzt ausdrücklich im Release-Profil mit.

<a id="b6"></a>
### B6 · Jedes neue Bein einmal absichtlich gebrochen

Ein Riegel, der nie angeschlagen hat, ist keiner.

**A20 — ein Fixturebyte gekippt:**

```text
=== A20 absichtlich gebrochen ===
8 gueltige, 29 ungueltige Envelope-Fixtures; 14 Regeln, jede mit Negativfall
  ROT: eq-copilot\fixtures\v3\envelope\ungueltig\crc-p2-null-statt-summe.bin weicht ab
EXIT=2
=== wiederhergestellt ===
8 gueltige, 29 ungueltige Envelope-Fixtures; 14 Regeln, jede mit Negativfall
  bytegleich: 38 Dateien, MANIFEST sha256=efa50c0384865d8a3d4eb560c7037937b60af6a4f32731f7d7dc5e953cf13307
EXIT=0
```

**A21 und der Envelope-Korpus — `16 + payload_len` wieder in u32 gerechnet.**
Der Unterschied zeigt sich im REGELNAMEN: die naive Rechnung sagt
`ueberlaenge`, die richtige `unterlaenge`.

```text
=== A21 absichtlich gebrochen: 16+payload_len wieder in u32 gerechnet ===
test ratengrenze_haelt_unter_flut ... ok
test zufallsbytes_bringen_den_pruefer_nie_aus_dem_tritt ... ok
test jedes_gekippte_payloadbit_eines_p2_frames_faellt_an_der_crc ... ok
test payload_an_der_paketgrenze_wird_angenommen_darueber_nicht ... ok
test schreiben_und_pruefen_sind_zueinander_invers ... ok
test fragmentierung_aendert_das_ergebnis_nie ... ok
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

failures:

---- feindliche_laengen_loesen_keine_allokation_aus stdout ----

thread 'feindliche_laengen_loesen_keine_allokation_aus' (18068) panicked at tests\transport_fuzz.rs:171:9:
assertion `left == right` failed: payload_len 4294967295
  left: Err([Ueberlaenge])
 right: Err([Unterlaenge])
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    feindliche_laengen_loesen_keine_allokation_aus

test result: FAILED. 8 passed; 1 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.16s

error: test failed, to rerun pass `--test transport_fuzz`
--- und der Envelope-Korpus (A4/contract_cross_language) dazu ---
thread 'envelope_korpus_klassifiziert_wie_das_manifest' (20788) panicked at tests\contract_cross_language.rs:640:5:
1 von 37 Envelope-Fixtures weichen ab:
ungueltig/kopf-payload-len-u32-ueberlauf.bin: Verstossmenge weicht ab
  soll [Unterlaenge]
  ist  [Ueberlaenge]
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    envelope_korpus_klassifiziert_wie_das_manifest

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 8 filtered out; finished in 0.02s

error: test failed, to rerun pass `--test contract_cross_language`
=== wiederhergestellt ===
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.19s
```

**B10 — dieselbe Rechnung in der C++-Fassung gebrochen:**

```text
=== B10 absichtlich gebrochen: 16+payload_len wieder in 32 Bit gerechnet ===

  FEHLER  alle Fixtures klassifiziert wie das Manifest (Urteil UND Verstossmenge)  [8 gueltig / 29 ungueltig; ungueltig/kopf-payload-len-u32-ueberlauf.bin: soll unterlaenge, ist ueberlaenge]
FEHLER — 86 Pruefungen, 1 Fehler
```

**A22 — der Broker lässt jeden 100. Heartbeat unbeantwortet:**

```text
=== A22 absichtlich gebrochen: der Broker laesst jeden 100. Heartbeat unbeantwortet ===

SONDE-010 | Lastbein: 32 Sonden, 4 s, Pipe \\.\pipe\evenacadia.nakama.v3.last.17892.1787976657
{"sonden":32,"sekunden":4,"p2_veroeffentlicht":66720,"p2_gesendet":46075,"p2_ersetzt":20704,"p0_gesendet":5024,"p0_beantwortet":4960,"p0_latenz_max_ms":20,"p0_latenz_p99_ms":20,"p0_ueberlaeufe":0,"envelope_abweisungen":0}
  FEHLER  KEIN P0-Frame geht verloren  [4960/5024]
LASTBEIN ROT
  ROT     das C++-Lastbein selbst ist gruen  [Exit 1]
  ROT     kein P0-Frame geht verloren (Clientseite)  [4960/5024]
ROT
```

Alle vier Eingriffe sind zurückgenommen; die Läufe B1–B5 stehen auf dem
committeten Stand.

---

## 4. T1 — Selbstaudit

Feste Liste aus `docs/bauaufteilung-sonden.md` §2, nie abgekürzt.

| # | Punkt | Befund |
|---|---|---|
| 1 | Numerische Ränder: NaN, ±inf, 0, negativ, Überlauf, Wrap | **Der Kern des Envelope.** `frame_len == 16 + payload_len` wird in **u64** gerechnet; die u32-Fassung liefe bei `payload_len = 0xFFFFFFFF` auf 15 um und akzeptierte den Frame. Das Fixture `kopf-payload-len-u32-ueberlauf.bin` unterscheidet beide über den REGELNAMEN, und beide Sprachen wurden damit einmal absichtlich gebrochen (B6). Dazu: `frame_len` 0, 15, 262 144, 262 145, 0x7FFFFFFF, 0xFFFFFFFF; `payload_len` 0 (gültig), an der Grenze (gültig) und darüber; Bootstrap 16 KiB und 16 KiB + 1. Nicht-endliche Zahlen gibt es im Envelope nicht — er urteilt nie über Inhalt. **Berichtigt am 29.08. (§7.9):** die `is_finite()`-Prüfung stand nur im Broker; der C++-Client wandelte `samplerate` VOR jeder Prüfung nach `long long`, was für NaN, ±Inf und Unwandelbares undefiniertes Verhalten ist. `audioGueltig` verriegelt jetzt clientseitig, vor dem Öffnen der Pipe. |
| 2 | Gegenpfad vorhanden? | Ja, jeweils im selben Änderungssatz: `control_anmelden`↔`control_abmelden`, `telemetrie_koppeln`↔`telemetrie_entkoppeln` (mit Test, dass zweimal Abmelden **kein** Erfolg ist), `start`↔`stop`, `oeffnen`↔`schliessen`, `v3_server_starten`↔`V3Griff::stoppen` (auch im `Drop`), `veroeffentlichen`↔`abholen`, `entnehmen`↔`zuruecklegen`. **Berichtigt am 29.08. (§7.2):** der Satz „die Control-Verbindung besitzt die Kopplung, geht sie, geht der Telemetrieplatz mit" stimmte für das REGISTER und nicht für den ARBEITER — der Telemetriethread lief weiter und lieferte P2 zu einer Sitzung, die es nicht mehr gab. Das Abmelden bricht ihn jetzt ab, jeder Telemetrieframe prüft zusätzlich die noch lebende Kopplung, und `telemetrie_getrennt`/`control_getrennt` melden unbedingt. |
| 3 | Behauptungs-Integrität: existiert jede genannte Datei / Funktion / Zeile wirklich? | Jede in diesem Manifest genannte Datei wurde in dieser Session geschrieben oder gelesen. Die Regelmenge wird nicht behauptet, sondern **gemessen**: `contract_cross_language` vergleicht die Regelliste des Manifests elementweise mit `ALLE_VERSTOESSE` des Lesers, das C++-Bein tut dasselbe — eine Regel im Code ohne Zeile im Manifest fällt sofort auf. Der Envelope-Erzeuger bricht ab, wenn eine Regel ohne Negativfixture bleibt. |
| 4 | Lügt der UI-Text? | Keine UI in diesem Ticket. Die entsprechende Ebene ist die Zählerwahrheit, und dort wurde **eine Lüge gefunden und behoben**: `P2Schleuse::ersetzteFrames()` zählte anfangs jede Veröffentlichung nach den ersten beiden als Ersetzung, auch wenn der Verbraucher längst leergeräumt hatte (133 056 „Ersetzungen" bei 133 120 Veröffentlichungen und 92 857 gesendeten Frames — die Zahlen widersprachen einander). Der Erzeuger kennt jetzt den Fortschritt des Verbrauchers. Was bleibt, steht als Schranke im Header: die Zahl ist eine **obere** Grenze, nie eine kleinere als der eingetretene Verlust. |
| 5 | Anzeige-Pflichten (nur bei UI-Diff) | Nicht betroffen — kein UI-Diff. Die Zustände, die eine spätere Oberfläche zeigen muss, liegen als ehrliche Felder bereit: `ControlClient::Snapshot` trägt Status, `letzterFehler`, Versuche, P0-Überläufe, P1-Wiederholungen und Envelope-Abweisungen; `TelemetryClient::Snapshot` zusätzlich `wartetAufKopplung` als **eigenen** Zustand — „noch nicht gekoppelt" ist etwas anderes als „getrennt". |
| 6 | Audiothread: nachweislich keine Allokation, kein Lock, kein I/O, kein Logging? | Der Audiothread wird in diesem Ticket **nicht angefasst**; kein Sample geht durch neuen Code. Die Grenze dorthin ist die `P2Schleuse`: vorallokiert, lockfrei, wartefrei, und mit dem thread-lokalen Allokationszähler gemessen — **100 000 Übergaben mit 0 Allokationen**, mit Gegenprobe am selben Zähler (er sieht eine echte Allokation, also schweigt er nicht bloß). Nebenläufigkeit **neu vermessen (§7.1):** 2 000 000 kleine plus 300 000 große Frames, kein zerrissener Frame und keine falsche Länge — und dabei traf der Erzeuger den vom Verbraucher beanspruchten Platz **12 210-mal**, ohne ihn zu beschreiben. Ein Race Detector steht auf dieser Maschine nicht zur Verfügung (ThreadSanitizer gibt es für MSVC nicht, `/fsanitize=address` findet keine Datenrennen); der Beweis ist deshalb strukturell: ein einziges Atomic entscheidet den Besitz je Slot. Beide Clients führen ihre I/O ausschließlich auf eigenen Threads; `sendeP0`/`sendeP1`/`veroeffentlichen` reihen nur ein. |

---

## 5. Befunde aus diesem Ticket

> Die zehn Befunde des T2-Prüfers vom 29.08. stehen mit ihren Fixes und Bruchproben in **§7**.

| Befund | Quelle | Gegen die Quelldatei verifiziert? | Ausgang |
|---|---|---|---|
| `PipeToken.cpp` trug den eingefrorenen Herstellernamen `evenacadia` in `NakamaKern.lib` | T1 / Kanon-Bein **A14** beim ersten Lauf nach dem Zuwachs | ☑ ja — A14-Rohausgabe, Abschnitt [2] | **gefixt**, ohne den Riegel anzufassen: die Datei ist aus dem geteilten Boden genommen. Welche Pipe eine Instanz wählt, ist dieselbe Art Aussage wie welche Identität sie trägt — sie gehört in die dünne Zielschicht. Der Kern bekommt den Pipenamen als Zeichenkette. |
| `ersetzteFrames()` widersprach den übrigen Zählern | T1, Lastbein-Ausgabe | ☑ ja — `IpcQueues.h`, `veroeffentlichen()` | **gefixt**: der Erzeuger liest jetzt auch den Fortschritt des Verbrauchers. Die verbleibende Ungenauigkeit (~1 ‰, konservativ) steht als Schranke im Header. |
| Der C++-Fuzz sprach über eine leere Menge (`0 angenommen / 40000 abgelehnt`) | T1, eigener Lauf | ☑ ja — `IpcTestMain.cpp`, Abschnitt C | **gefixt** (`97c956d`): dieselbe Invariante zusätzlich an Einbit-Mutanten gültiger Frames — 7671 von 20 000 kommen durch und halten jede Kopfregel. |
| 27 von 32 Clientpaaren standen nach 20 s; die übrigen warteten auf verdoppelten Backoff | Lastbein A22, erster Lauf | ☑ ja — `IpcVerbindung::oeffnen` | **gefixt**: `ERROR_PIPE_BUSY` heißt nicht „nicht da", sondern „gerade horcht keine Instanz". Der Client nutzt jetzt `WaitNamedPipe` statt Backoff. Danach 32/32. |
| Die Bootstrapschranke prüfte **vor** dem Parsen und hätte ein legitimes Hello mit angehängten v3-Frames in einem einzigen Read abgewiesen | T1, Diff-Lesung | ☑ ja — `server_v3.rs`, Bootstrap-Leseschleife | **gefixt**: die Längengrenze des Hellos prüft `bootstrap_lesen` selbst (ab dem vierten Byte, vor jeder Allokation); die Schleife bewacht nur noch den pipelinenden Peer. |
| A22 lief einmal ROT (`9920/10048`) | eigener Lauf | ☑ ja — Differenz exakt 4 × 32 = 128, also die Bruchprobe | **kein Befund am Weg**, sondern ein veraltetes Artefakt: der Probe-Broker war noch aus dem gebrochenen Stand gebaut. `tools/beweise.ps1 -Bauen` baut ihn jetzt ausdrücklich mit. Das Bein hat dabei genau das getan, wofür es da ist. |
| NAK-07 (Pipe-Backpressure/Deadlines) | `docs/offene-punkte.md` | ☑ ja | **geschlossen** — siehe die Zeile dort. |
| NAK-49 / ACCESS-001 (kein `SECURITY_SQOS_PRESENT`) | `docs/offene-punkte.md` | ☑ ja — `IpcVerbindung::oeffnen` | **nicht wiederholt**: der neue Client setzt `SECURITY_SQOS_PRESENT \| SECURITY_IDENTIFICATION` von Anfang an. Der Altbestand `src/PipeClient.cpp` trug den Fix bereits (Bein A4b). |

---

## 6. Was dieses Ticket **nicht** geprüft hat

Damit ein frischer Prüfer nicht danach sucht:

* **Kein Race Detector.** ThreadSanitizer gibt es für MSVC/Windows nicht, und `/fsanitize=address` findet keine Datenrennen. Die Nebenläufigkeit der `P2Schleuse` ist deshalb strukturell bewiesen (ein Atomic je Slot entscheidet den Besitz) und äußerlich vermessen (Millionen Frames, kein zerrissener) — nicht von einem Werkzeug bestätigt. Siehe §7.1.

* **Kein FL.** Nichts hier lief in einem Host. Der Weg ist headless bewiesen.
* **Keine HMAC-Authentisierung, kein Pairing, kein Replaycache.** Das ist
  `SONDE-016` (§48.4). Die Kopplung hier ist `link_id + challenge +
  runtime_nonce` über eine per DACL auf den aktuellen User begrenzte Pipe —
  sie ersetzt keine Signatur und behauptet es nirgends.
* **Keine Impersonation-Kette im v3-Listener.** §48.4 verlangt
  `ImpersonateNamedPipeClient` nach dem Bootstrap-Hello mit `RevertToSelf` in
  jedem Pfad. Der v3-Listener fährt heute nur über Probe-Namen und trägt sie
  deshalb noch nicht; sie gehört in denselben Änderungssatz wie das produktive
  Öffnen des Endpunkts (`SONDE-011`). Neu als **NAK-90** eingetragen.
* **Kein Coordinator, kein Store, keine Outbox** — `SONDE-011`.
* **Keine P1-Last.** Das Lastbein fährt P0 und P2; P1 ist in den Unittests und
  im C++-Bein geprüft, aber nicht unter Ende-zu-Ende-Last. Neu als **NAK-91**
  eingetragen.

---

## Kanon-Lauf - SONDE-010 Nacharbeit Runde 3 - Abschluss

**Lauf:** 2026-08-29 12:51 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 12:51:34 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | a88d32e SONDE-010 Runde 3: Auslieferungspaket neu festgeschrieben, roter Kanonlauf erklaert statt entfernt |
| Commit (voll) | a88d32ee8573c524ce160c66853e22d715d576b0 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-29 12:51:36 | `02C1CFEB9CBB5126` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-29 12:51:39 | `96A4F56A20EB201E` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-29 12:51:43 | `3E88CC83CA0E2DBE` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-29 12:51:45 | `6A34D33C6501E426` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-29 12:51:47 | `F017F72D30335F9D` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-29 12:51:52 | `AFA1ACE590F2D8C0` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-29 12:51:54 | `76EB72CC3354E579` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-29 12:51:57 | `5CFFE1D9A047ED3B` | frisch (Bau bestaetigt) |
| `EqCopIpcTest` | 2026-08-29 12:52:04 | `D3095444C406911C` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-29 12:52:06 | `A292406B0F3A83A5` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-29 12:48:29**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,75 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,34 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,06 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,07 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,51 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,20 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,03 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,19 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,50 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,87 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 35,51 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A19](#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [↓ A20](#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,29 s | [↓ A21](#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,39 s | [↓ A22](#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,09 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,12 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,37 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,23 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,38 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,15 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,36 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 31,78 s | [↓ B10](#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,11 s | [↓ B8](#b8) |

**Rohausgabe:** `docs/beweise/roh/SONDE-010-a88d32e.md` (stdout/stderr aller Beine und der Bau vor dem Lauf; ausgegliedert 30.08.2026)

## Abschluss der Dirigentenrunde — 2026-08-29 (Runde 3): NEEDS_WORK, dreimal nachgearbeitet, Urteil offen

**Marke:** `T2 NEEDS_WORK 2026-08-29 nachgearbeitet` (unverändert; Präzedenz S8/S9). Stand `444e125`; Ticketbasis `a7b0740`; Basis dieser Runde `e9ae2e9`. Die dritte Nacharbeitsrunde lief auf ausdrückliche User-Freigabe vom 29.08. (`docs/beweise/G1.md` §12) durch einen frischen Opus-Worker (`max`, Aufsicht ENG): Commits `1bdb93d` · `c444ca3` · `66e4a09` · `e3e8e57` · `a88d32e` · `833baff` · `444e125`; Kanon beglaubigt **GRÜN 32/32** auf `a88d32e` (12:51, Exit 0), davor ein ROT 1/32 an A17 durch Relink, in §9.9 erklärt und als NAK-93/NAK-94 registriert.

**Prüfer Runde 4:** Codex `gpt-5.6-sol`, Effort `xhigh`, frischer Thread `01a04d2b-7926-7da2-b061-1991cd6d3a4b`, lesend über den vollständigen Ticketbereich `git diff a7b0740...444e125`; HEAD vor und nach dem Lauf `444e125` (12:58–13:16). **Urteil: NEEDS_WORK** — die sechs NAK-92-Schlüsse sind laut Prüfer an Quelle und Rohausgaben bestätigt; sieben neue Befunde (6 P1, 1 P2). Wortlaut:

> Die sechs in §9 genannten NAK-92-Fixes und ihre Bruchproben stimmen mit Quelle und Rohausgaben überein. Zusätzliche P1-, Lifecycle- und Stop-Lücken brechen jedoch weiterhin Backpressure- und P0-Starvation-Zusagen; außerdem überzieht A21 seine Messung.
>
> Full review comments:
>
> - [P1] Dräne den P1-Wiederholpuffer auch ohne Reconnect — eq-copilot/plugin/core/ipc/ControlClient.cpp:439-439
>   Bei nur vorübergehendem P1-Rückstau verbleiben akzeptierte Ereignisse unbegrenzt im Wiederholpuffer, weil dessen einzige Entleerung hier beim Verbindungsaufbau erfolgt. Repro: mit Kapazität 1 E1 aktiv und E2 als `zurWiederholung` einreihen, E1 senden und die Verbindung offen lassen; E2 wird nie gesendet, während spätere Nachrichten sie überholen. Damit fehlt der Fortschritt für die Wiederholungszusage aus `docs/FL-Nakama-Sonden-Design-Entwurf.md:3749`.
>
> - [P1] Bewahre Snapshot-Schlüssel im Wiederholpuffer — eq-copilot/plugin/core/ipc/IpcQueues.h:191-191
>   Bei einer vollen Hauptqueue verliert ein Snapshot hier seinen Objektschlüssel. Repro: Kapazität 1/1, einen fremden Eintrag aktiv halten, Snapshot K=alt in den Wiederholpuffer legen und danach K=neu einreihen; der neuere Snapshot wird abgewiesen und nach Reconnect erscheint der alte als nicht koaleszierbares Ereignis. Das kehrt die Snapshot-Koaleszierung aus `docs/FL-Nakama-Sonden-Design-Entwurf.md:3749` um.
>
> - [P1] Verwirf keine akzeptierten Rust-P1-Ereignisse — broker/src/transport/warteschlange.rs:148-151
>   Die als eine der vier Backpressure-Politiken ausgelieferte Rust-Queue enthält weiterhin exakt den behobenen C++-Fehler: bei Kapazität 2/2 und Ereignissen 1–5 wird das zuvor akzeptierte Ereignis 3 entfernt und nach Reconnect kommen nur 4 und 5. Der bestehende Test `p1_haelt_ereignisse_fuer_den_reconnect_vor` kodifiziert diesen Verlust sogar, obwohl §53.9 Wiederholung statt Verdrängung verlangt.
>
> - [P1] Lies P0-Antworten trotz wartender P1-Nachrichten — eq-copilot/plugin/core/ipc/ControlClient.cpp:525-525
>   Solange irgendeine Ausgangsnachricht wartet, überspringt dieses `continue` den Lesepfad vollständig. Repro: der Peer liest und beantwortet zuerst einen P0-Heartbeat, liest danach aber eine große wartende P1-Nachricht nicht mehr; der Client blockiert fünf Sekunden im P1-Write, schließt anschließend und verarbeitet den bereits verfügbaren P0-ACK nie. Damit kann P1-Backpressure einen bestätigten P0-Rundlauf aushungern, was dem Gate „ohne P0-Starvation“ widerspricht.
>
> - [P1] Ordne Control- vor Telemetry-Lifecycle — broker/src/transport/server_v3.rs:1245-1247
>   Das Control-Welcome wird gesendet, bevor `control_verbunden` abgeschlossen ist, obwohl die Kopplung bereits registriert wurde. Repro: die Senke in `control_verbunden` blockieren, nach Empfang des Welcome sofort Telemetry verbinden und die Callback-Reihenfolge protokollieren; `telemetrie_gekoppelt` läuft auf dem anderen Verbindungsthread zuerst, selbst wenn Control später in die Frist läuft. Da dieser Callback nur die `link_id` erhält, kann die Senke den fehlenden Control-Kontext nicht rekonstruieren; das verletzt die gekoppelte Lifecycle-Regel aus `AGENTS.md:48-50`.
>
> - [P1] Entkopple Client-Callbacks vom stop()-Join — eq-copilot/plugin/core/ipc/ControlClient.cpp:162-163
>   Die Provider und `beiAntwort` laufen synchron auf demselben Clientthread, während `stop()` diesen Thread ohne Frist joint. Ein blockierender Callback lässt daher Stop und Destruktor unbegrenzt hängen; ruft `beiAntwort` selbst `stop()` auf, versucht der Thread sich selbst zu joinen und endet typischerweise per `std::system_error`/`std::terminate`. Die öffentliche API verbietet diese Reentranz nicht; sie benötigt einen begrenzten oder vom Worker entkoppelten Abschlussweg.
>
> - [P2] Belege die Allokationsaussage von A21 — tools/beweise.ps1:345-345
>   A21 behauptet, feindliche Längen lösten keine Allokation aus, aber der Test zählt Allokationen überhaupt nicht und `envelope_pruefen` erzeugt auf diesen Fehlerpfaden jeweils ein `Vec<Verstoss>`. Er prüft ausschließlich die Fehlerklasse; selbst eine neue, begrenzte Allokation anhand von `frame_len` bliebe grün. Der Kanon-Lauf belegt daher nur, dass keine riesige Eingabeallokation bzw. kein Absturz auftrat, nicht die hier festgeschriebene Allokationsfreiheit.

**Vom Dirigenten an der Quelle bestätigt: alle sieben.** `ControlClient.cpp:439` ist der einzige Aufrufer von `nachReconnectWiederholen()` — der Wiederholpuffer leert sich nur beim Verbindungsaufbau; `IpcQueues.h:191` legt den Snapshot ohne Schlüssel in `wiederholung` (Deque von Strings); `warteschlange.rs:148-151` trägt weiterhin `pop_front()` mit `WiederholungVerdraengt` — der in C++ geschlossene Befund 1 ist in Rust offen, und `p1_haelt_ereignisse_fuer_den_reconnect_vor` (Zeile 359) schreibt den Verlust fest; `ControlClient.cpp:525` `continue` überspringt den Lesepfad, solange irgendetwas wartet; `server_v3.rs:1225/1245/1274`: `control_anmelden` → Welcome schreiben → erst dann `control_verbunden`; `ControlClient.cpp:155-166`: `stop()` joint ohne Frist unter `lebenslaufMutex`, Callbacks laufen auf demselben Thread; `tools/beweise.ps1:345` behauptet für A21 „lösen keine Allokation aus", der Fuzz-Test zählt keine Allokationen.

**Warum hier gestoppt:** Die vom User freigegebene dritte Runde ist verbraucht; jede Runde schloss ihre Befunde und legte eine neue Schicht derselben Frage frei (Rückstau- und Lebenszykluspolitik unter Stop, Reconnect und Fehlerpfaden). Die sieben Restbefunde sind reproduzierbar und klein umrissen, aber materiell: vier davon berühren direkt den Gate-Satz „ohne P0-Starvation" bzw. §53.9. Eine vierte Runde nur auf ausdrückliche User-Freigabe; bis dahin bleibt S14–15 „gebaut, nachgearbeitet, frisches Urteil fehlt". Datiert als **NAK-95** in `docs/offene-punkte.md`. Für `SONDE-011` bleibt NAK-95 (wie zuvor NAK-92) Vorbedingung — der Coordinator ist die Senke, deren Lebenszyklus hier offen ist.

**Tatsächlich gelaufene Beweise:** Kanon `-Bauen` zweimal in Runde 3 (ROT 1/32 auf `e3e8e57` durch A17-Relink, GRÜN 32/32 auf `a88d32e`), B10 159 → 161, Rust-Tests 150 → 154, A22 grün (32 Sonden, 9984/9984 P0 beantwortet, max 21 ms); ein Codex-Lauf read-only, HEAD vor/nach identisch. Nicht gelaufen: kein zweiter FL-Scan des neu festgeschriebenen Pakets (§9.9).

---

## Konvergenzentscheid des Dirigenten — 2026-08-30 (Sitzung 3e24ab41): NAK-95 und NAK-98 — vier Defekte, vier Lücken; eine Runde mit der Ursache als Auftrag

**Stand:** `bae7a4e`. Dirigentenvertrag §3.4: Die vierte Wiederprüfung (Thread `01a04d2b…`, Stand `444e125`) endete ohne PASS; das Rundenbudget ist verbraucht. Jeder offene Befund wird hier am wörtlichen Gate-Text und an der Quelle eingeordnet. Die Quelle der Ticketpfade ist seit `444e125` unverändert (`git log 444e125..bae7a4e -- eq-copilot/plugin/core/ipc broker/src/transport eq-copilot/plugin/tests/IpcTestMain.cpp broker/tests` leer); die Fundstellen wurden heute erneut gelesen.

**Gate-Text, wörtlich** (Entwurf §65, Zeile `SONDE-010`): „CRC/Fuzz/Backpressure/Reconnect ohne P0-Starvation". **Zusagen des Entwurfs §53.9**, wörtlich aus der Tabelle: P0 „nichts verwerfen; Verbindung schließen"; P1 „Snapshots nach Objektschlüssel koaleszieren; nicht koaleszierbare Events bei Überlauf über Reconnect/Outbox wiederholen"; P2 „ältesten ungesendeten Frame ersetzen"; Bootstrap: „Bei `protocol=3` authentisiert der Server zuerst den Control-Client; seine erste Antwort ist bereits ein v3-gerahmtes `welcome {link_id, challenge}`. Erst danach darf ein zweites Bootstrap-Hello … die Telemetry-Verbindung koppeln."

| # | Befund (NAK-95 / NAK-98) | Klasse | Regel bzw. brechender Satz |
|---|---|---|---|
| 1 | `ControlClient.cpp` `nachReconnectWiederholen()` ist der einzige Abfluss des P1-Wiederholpuffers; ohne Reconnect bleiben akzeptierte Ereignisse liegen, spätere überholen sie | **Lücke** — §53.9 nennt Reconnect/Outbox als Wiederholweg, die Outbox ist `SONDE-011`; zum Abfluss ohne Reconnect sagt der Entwurf nichts | **Regel:** Der Wiederholpuffer fließt ab, sobald die Hauptqueue wieder Platz hat — in Annahmereihenfolge und **vor** jedem Neuzugang; ein Reconnect ist dafür nicht nötig. |
| 2 | `IpcQueues.h` legt einen Snapshot ohne Objektschlüssel in `wiederholung`; ein neuerer Snapshot desselben Objekts wird abgewiesen, nach Reconnect kommt der alte als Ereignis | **Defekt** — §53.9 „Snapshots nach Objektschlüssel koaleszieren" | Der Schlüssel überlebt jeden Zwischenpuffer; Koaleszierung gilt auch dort. |
| 3 | `broker/src/transport/warteschlange.rs` `pop_front()` verdrängt ein akzeptiertes, nicht koaleszierbares P1-Ereignis; `p1_haelt_ereignisse_fuer_den_reconnect_vor` schreibt den Verlust fest | **Defekt** — §53.9 „… bei Überlauf … wiederholen" (nicht verwerfen); in C++ als NAK-92 (1) geschlossen, in Rust offen | Dieselbe Politik in beiden Sprachen: voll heißt abweisen des Neuzugangs, nie Löschen von Akzeptiertem; der Test misst es beidseitig. |
| 4 | `ControlClient.cpp` `continue` nach dem Senden überspringt den Lesepfad, solange irgendetwas wartet; ein blockierender P1-Write hungert einen bereits vorliegenden P0-ACK aus | **Defekt** — Gate-Satz „ohne P0-Starvation" | P0 wird gelesen und beantwortet, während P1 rückstaut; der Lesepfad hängt nie hinter „erst alles senden". |
| 5 | `server_v3.rs`: Welcome wird geschrieben, bevor `control_verbunden` gelaufen ist; `telemetrie_gekoppelt` kann auf dem anderen Thread zuerst laufen | **Lücke** — §53.9 legt die Drahtreihenfolge (Control, Welcome, dann Telemetry) fest, sagt zur Senken-Callback-Reihenfolge nichts; `AGENTS.md` ist Änderungsdisziplin, kein Vertrag | **Regel:** `control_verbunden` ist abgeschlossen, bevor das Welcome den Draht verlässt; `telemetrie_gekoppelt` läuft nie vor `control_verbunden`; beim Trennen erst Kopplung lösen, dann `telemetrie_getrennt`, dann `control_getrennt` — je Callback genau einmal. |
| 6 | `ControlClient::stop()` joint den Clientthread ohne Frist unter `lebenslaufMutex`; Provider und `beiAntwort` laufen auf demselben Thread: blockierender Callback hängt Stop und Destruktor, `stop()` aus `beiAntwort` ist ein Self-Join | **Lücke** — Gate und Entwurf sagen zu Stop-Fristen und Reentranz nichts | **Regel:** `stop()` kehrt immer zurück: aus einem Callback heraus ohne Self-Join (der Thread endet nach Rückkehr des Callbacks von selbst), von außen mit Frist auf einen laufenden Callback (Frist in der Matrix benannt, Überschreitung sichtbar gezählt); nach Rückkehr von `stop()` wird kein Callback mehr gerufen. Dasselbe gilt für `TelemetryClient`. |
| 7 | `tools/beweise.ps1` A21-Behauptung „lösen keine Allokation aus", der Fuzz-Test zählt keine Allokationen | **Lücke** — Behauptung > Messung an einem Prüfwerkzeug; keine Zusage verlangt Allokationsfreiheit im Fehlerpfad des Rust-Prüfers | **Regel:** Die Behauptung wird auf das Gemessene begrenzt (feindliche Längen enden in der erwarteten Fehlerklasse ohne Absturz und ohne Eingabeallokation in Größe der behaupteten Länge); kein Allokationszähler. |
| 8 | NAK-98: B10 `EqCopIpcTest` fällt unter Baulast am P2-Slot-Anspruch („der NEUESTE Frame faellt dabei NIE … [2 neueste wegen fremden Anspruchs verworfen]"), einzeln fünfmal grün | **Defekt** — ein Kanon-Test bricht; §53.9 „ältesten ungesendeten Frame ersetzen" | Der Kollisionsfall (Slot beansprucht, während ersetzt wird) wird deterministisch erzwungen statt über Last erwartet; Produkt und Test messen dieselbe Politik: es fällt nie der neueste Frame. |

**Ursache, nicht Punktkorrektur.** Die Rückstau- und Lebenszykluspolitik des v3-Transports — P0/P1/P2-Queues in C++ und Rust, `ControlClient`/`TelemetryClient` (start/stop/reconnect/Callbacks), Listener-Kopplung im Broker — wurde nie als Spezifikation festgeschrieben. Vier Prüfrunden schlossen je ihre Befunde und legten die nächste Schicht derselben Frage frei (Runde 1: 10, Runde 2: 7, Runde 3: 6, Runde 4: 7 Befunde). Deshalb bekommt das Ticket nach §3.4 genau **eine** weitere Runde mit der Ursache als Auftrag, als Wegwechsel:

1. **Verhaltensmatrix zuerst** (Dirigent §3.2, seit 29.08.2026): Zustände × Ereignisse × Zusage für die drei Queue-Klassen (voll/leer/Reconnect/Stop), für beide Clients (gestoppt/verbindend/verbunden/rückstauend/stoppend × start/stop/Verbindungsverlust/Callback-Reentranz/P0-ACK während P1-Rückstau) und für den Listener (Bootstrap-Reihenfolge, Callback-Reihenfolge, Fristen) — je Zeile der Test, der sie misst, in **beiden** Sprachen, wo beide betroffen sind. Die acht Zeilen oben sind Pflichtzeilen der Matrix. Die Matrix steht im lebenden Kopf des Manifests und ist ab dann die Referenz für Worker und Prüfer.
2. **Matrixprüfung** durch einen lesenden Codex-Thread (`high`) nur gegen §53.9, §33.1 und den Gate-Text — vor dem Bau.
3. **Implementierung nach Matrix** in C++ und Rust, je Zeile ein Test, der ohne den Fix rot ist; NAK-98 reproduzierbar erzwungen; A21-Behauptung begrenzt.
4. Kanon auf dem committeten Stand; Wiederprüfung nach Vorlage B (Fixdiff der Runde, die acht Befunde, Matrix als Referenz).

Bleibt danach ein Defekt, wird seine Ursache als eigener Registerpunkt mit Matrix ausgegliedert, das Ticket bleibt `gebaut`, und der Dirigent zieht das nächste Ticket vor (User-Wort 30.08.2026: „voranschreiten ist das wichtigste"). **Aufsicht:** ENG.

---

## Kanon-Lauf - SONDE-010 Ursachenrunde Phase 2

**Lauf:** 2026-08-30 16:02 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-010-7457684.md](roh/SONDE-010-7457684.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-30 16:02:37 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 7457684 SONDE-010 Ursachenrunde: Implementierung nach Verhaltensmatrix (Regel 1-8) |
| Commit (voll) | 74576849c1b09094dbfa5e98b61df93193d2ef1d |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,14 s | [A1](roh/SONDE-010-7457684.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 10,01 s | [A2](roh/SONDE-010-7457684.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,37 s | [A3](roh/SONDE-010-7457684.md#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,08 s | [A4](roh/SONDE-010-7457684.md#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,06 s | [A4b](roh/SONDE-010-7457684.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,49 s | [A5](roh/SONDE-010-7457684.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [A6](roh/SONDE-010-7457684.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [A7](roh/SONDE-010-7457684.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,19 s | [A8](roh/SONDE-010-7457684.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,29 s | [A9](roh/SONDE-010-7457684.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,47 s | [A10](roh/SONDE-010-7457684.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,27 s | [A11](roh/SONDE-010-7457684.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,26 s | [A12](roh/SONDE-010-7457684.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,21 s | [A13](roh/SONDE-010-7457684.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 5,67 s | [A14](roh/SONDE-010-7457684.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen (Zielkollision, state_schema-Drift, Stilllegung umgangen, Marke unlesbar, ungueltige ziel_id-Typen, Pfadtraversal) fallen einzeln. [4] vergleicht im Kanon weich (Abweichung nach Relink = Hinweis, fehlendes Artefakt oder nicht bildbarer Ordner-Hash = Fehler), hart nur mit --release; [4b] berichtet den installierten Stand ohne Urteil, ok nur bei Journalstatus OK; [5] Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide ab. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft, jede unerwartete Ausnahme endet als Klartext mit Exit 2 statt als Traceback. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,00 s | [A17](roh/SONDE-010-7457684.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 37,05 s | [A18](roh/SONDE-010-7457684.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,15 s | [A19](roh/SONDE-010-7457684.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A20](roh/SONDE-010-7457684.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,36 s | [A21](roh/SONDE-010-7457684.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,51 s | [A22](roh/SONDE-010-7457684.md#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/SONDE-010-7457684.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,10 s | [B1](roh/SONDE-010-7457684.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,35 s | [B2](roh/SONDE-010-7457684.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/SONDE-010-7457684.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,11 s | [B3b](roh/SONDE-010-7457684.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,20 s | [B3c](roh/SONDE-010-7457684.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,40 s | [B4](roh/SONDE-010-7457684.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B9](roh/SONDE-010-7457684.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,26 s | [B5](roh/SONDE-010-7457684.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 35,72 s | [B10](roh/SONDE-010-7457684.md#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,12 s | [B8](roh/SONDE-010-7457684.md#b8) |


---

## Ursachenrunde Phase 2 — 2026-08-30 (Worker, hat nicht geurteilt)

**Stand.** Basis `7deff2e` (Matrix nach Wiederprüfung), Implementierung
`7457684`, Kanon auf genau diesem committeten Stand **GRÜN 32/32**
(`docs/beweise/roh/SONDE-010-7457684.md`). Referenz für jede Zeile unten ist
die Verhaltensmatrix im lebenden Kopf dieses Manifests; die drei
Restwidersprüche der Wiederprüfung (Thread `01a052cf`) stehen dort als
„(Nachtrag 30.08., Wiederprüfung)" in `C-LS-06`, `B-TC-07` und `C-LS-07`.

**Zahlen dieser Runde.** B10 `EqCopIpcTest` 161 → **188 Prüfungen, 0 Fehler**;
Rust **146** Lib-Tests (vorher 140) plus 9 `contract_cross_language` und 9
`transport_fuzz`; A22 unverändert grün (32 Sonden, **10 016/10 016** P0
beantwortet, max 22 ms, p99 20 ms, 40 756 ersetzte P2-Frames).

### Was je Matrixzeile gebaut wurde

| Zeile | Datei · Symbol | Test (grün) | Bruchprobe (rot) |
|---|---|---|---|
| `A-P1-03` (Regel 2) | `IpcQueues.h` · `P1Warteschlange::einreihen`, `zuruecklegen`, `Eintrag` im Wiederholpuffer · `warteschlange.rs` · `P1Warteschlange::einreihen` | B10 E · „ein Snapshot im Wiederholpuffer behaelt seinen Schluessel und koalesziert dort"; `warteschlange::p1_wiederholpuffer_haelt_den_schluessel` | C2 / R3 — Schlüsselsuche entfernt: beide rot ([↓ Bruchproben](roh/SONDE-010-bruchproben-phase2.md)) |
| `A-P1-06` (Regel 1) | `IpcQueues.h` · `P1Warteschlange::abfliessen`, gerufen aus `bestaetigen`/`zuruecklegen`/`einreihen` · `warteschlange.rs` · `P1Warteschlange::abfliessen` aus `entnehmen`/`einreihen` | B10 E · „ein einziges Senden zieht eine Wiederholung nach — ohne Reconnect"; `p1_wiederholpuffer_fliesst_ohne_reconnect_ab` | C1 / R2 — Abfluss nur beim Reconnect: rot |
| `A-P1-07` (Regel 1) | dieselbe Stelle, `abfliessen()` VOR dem Urteil über den Neuzugang | B10 E · „und er ueberholt sie nicht — Annahmereihenfolge ueber beide Puffer"; `p1_neuzugang_ueberholt_keine_wiederholung` | C1 (dieselbe Wurzel) |
| `A-P1-05` (Regel 3) | `warteschlange.rs` · `P1Ergebnis::Abgewiesen` statt `WiederholungVerdraengt`, Zähler `abgewiesene()` | `p1_haelt_ereignisse_fuer_den_reconnect_vor` — auf die Zusage berichtigt | R1 — `pop_front()` zurück: rot |
| `A-IN-04` | `warteschlange.rs` · `IngressWarteschlange::einreihen` (P1 → `ClientTrennen`), `p1_ueberlauf_trennt`; `server_v3.rs` · `V3Statistik::ingress_p1_ueberlauf_trennt` | `ingress_voll_ohne_p2_trennt_auch_bei_p1` | R4 — P1 fällt wieder still: rot |
| `B-CC-06` (Regel 4) | `ControlClient.cpp` · `Laufzeit::eineVerbindung`, Lambda `empfangenes` ersetzt das `continue` | B10 G16 · „der ACK erreicht beiAntwort, obwohl P1 rueckstaut" | C4 — `continue` zurück: rot (`0 ACKs`) |
| `B-CC-07` (Regel 4) | dieselbe Stelle, `empfangenes (0)` vor dem Verbindungsende | dieselbe Probe (der ACK trifft vor dem Ende ein) | C4 |
| `C-LS-07` (Regel 4, Rust) | `server_v3.rs` · `verbraucher_p0` / `verbraucher_rest`, `Eingang::entnehmen_p0`/`entnehmen_ohne_p0`, `IngressWarteschlange::entnehmen_p0` | `p0_wird_beantwortet_waehrend_p1_die_senke_blockiert` (beide Fälle: Antwort bei blockierter `p1`-Senke; 257. Frame trennt nach `A-IN-04`) | — der Test war vor der Trennung der Verbraucher nicht formulierbar; er misst die neue Zusage |
| `C-LS-02`, `C-LS-04` (Regel 5) | `server_v3.rs` · `verbindung_bedienen`, `control_verbunden` VOR dem Welcome; `trennmelder_anlegen` | `welcome_folgt_dem_abgeschlossenen_control_verbunden`, `connect_callbacks_je_kopplung_genau_einmal` | R5 — Welcome wieder zuerst: rot (`Welcome kam schon nach 440.9µs`) |
| `C-LS-06` (Regel 5) | `server_v3.rs` · `TrennMelder`, `auf_telemetrie_getrennt_warten`, `telemetrie_getrennt_gemeldet`, `kopplung_loesen` liefert jetzt `bool` | `trennreihenfolge_je_callback_genau_einmal`, `abgeloestes_telemetrie_getrennt_haelt_control_getrennt_nicht_auf` | R6 — kein Wartepunkt: rot (`control_getrennt kam vor telemetrie_getrennt`) |
| `B-CC-10`…`B-CC-12`, `B-CC-16`, `B-CC-17`, `B-TC-07`, `B-TC-09` (Regel 6) | `ControlClient.{h,cpp}` und `TelemetryClient.{h,cpp}` · `struct Laufzeit` hinter `shared_ptr`, `stop()` mit Reentranzprüfung über `threadId`, Frist `kStopFristMs`, `Snapshot::stopFristUeberschritten` | B10 G17 (drei Fälle: `stop()` aus `beiAntwort`; blockierender Callback binnen Frist; Telemetrie vor der Kopplung und im blockierenden P2-Write) | C5 — `kStopFristMs` 60 000: rot (`20000 ms`, Zähler 0) |
| `B-TC-10` | `TelemetryClient.cpp` · `Laufzeit::eineVerbindung`, Abbruch über `ioAbbrechen` | B10 G18 · Verbindungsverlust mitten im Write, neuer Versuch, Schleuse nimmt weiter an | — |
| `A-P2-03`, `A-P2-04` (Regel 8) | `IpcQueues.h` · `P2Schleuse::veroeffentlichen` geht den Ring ab (`kSlots * 2` Versuche); Testhaken `testSlotBeanspruchen`/`testSlotFreigeben`/`testSchreibstand` | B10 E2 · „erzwungene Slot-Kollision: der neueste Frame findet immer einen Platz" (deterministisch, ohne Zeitfensterzufall) plus die bestehende Lastwache | C3 — wieder zwei Versuche: rot (`1 verworfen, 2 Loecher`) |
| `D-A21-01` (Regel 7) | `tools/beweise.ps1` · A21-Behauptung; `transport_fuzz.rs` · `feindliche_laengen_enden_in_der_erwarteten_fehlerklasse` | Kanon A21 grün mit der begrenzten Behauptung (Kopfzeile unten) | — Behauptung und Testname tragen jetzt dasselbe Maß; kein Allokationszähler |

### Technische Entscheidungen innerhalb der Matrix

* **Die geteilte Laufzeit.** Regel 6 erlaubt `stop()`, einen Thread nach
  `kStopFristMs` ABZULÖSEN. Ein abgelöster Thread, der danach noch Member des
  Clients läse, wäre undefiniertes Verhalten, sobald das Objekt zerstört wird.
  Deshalb liegt alles, was der Clientthread anfasst, in `struct Laufzeit`
  hinter einem `shared_ptr`, von dem der Thread eine eigene Referenz hält —
  dasselbe Muster wie `Senkenruf`/`join_mit_frist` im Rust-Listener. Die
  öffentliche API beider Clients ist unverändert.
* **Zwei Verbraucher im Listener.** `C-LS-07` verlangt, dass P0 beantwortet
  wird, während die Senke in `p1` steht. Mit EINEM Ingressthread ist das nicht
  erreichbar, gleich wie schnell der Leser ist. Die Ingressqueue bleibt EINE
  Queue mit Cap 256 und der Politik aus §53.9; getrennt ist nur die Sicht
  darauf (`entnehmen_p0` / `entnehmen_ohne_p0`).
* **Abflussrichtung des Wiederholpuffers.** Die älteste Wiederholung wandert
  ans ENDE der Hauptqueue, nicht an den Anfang: alles dort wurde vor ihr
  angenommen, und wegen `A-P1-07` kann nichts Späteres dort stehen. Damit gilt
  die Annahmereihenfolge über beide Puffer, ohne Sonderfall.
* **`p0_ueberlauf_trennt_die_verbindung` musste getaktet werden.** Seit
  `A-IN-04` trennt schon der 257. P1; der Test füllt deshalb bis zum
  Höchststand und legt erst dann P0 nach — sonst spräche er über den falschen
  Pfad.
* **Testhaken statt Last für NAK-98.** `testSlotBeanspruchen` benutzt dasselbe
  Besitz-Atomic wie beide echten Seiten und kann an der Politik nichts ändern;
  er macht den Kollisionsfall nur reproduzierbar. Die alte Lastprobe bleibt als
  Wache daneben stehen.

### Kanon

```text
GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e)
Manifest:   docs\beweise\SONDE-010.md
Rohausgabe: docs\beweise\roh\SONDE-010-7457684.md
```

**NAK-98 gilt damit als geschlossen:** `EqCopIpcTest` läuft im Kanon direkt
nach dem Release-Bau (35,72 s, Exit 0) und trägt den Kollisionsfall jetzt
deterministisch statt unter Baulast.

### Prüfliste abgehakt (`tools/dirigent/pruefliste.md`)

| Abschnitt | Wo gemessen |
|---|---|
| **A** Rückstau und Prioritätsklassen | Politik bei *voll* je Klasse ausdrücklich (`A-P0-03`, `A-P1-05`, `A-P2-02`); Abflussweg OHNE Reconnect (`A-P1-06`); Schlüssel überlebt den Zwischenpuffer (`A-P1-03`); höhere Klasse wird weiter gelesen und beantwortet (`B-CC-06`, `C-LS-07`); die Zähler wertet A22 aus (`p2_neueste_verworfen`, `p2_kollisionsloecher`), `beanspruchtVerworfen` ist die Wache mit Test; dieselbe Regel in beiden Sprachen, beidseitiger Test, beidseitig einmal gebrochen (R1–R4, C1–C3). |
| **B** Lebenszyklus | Reihenfolge beim Verbinden gemessen (`C-LS-02`); beim Trennen erst entkoppeln, dann Fristen und Joins (`C-LS-06`); jeder Join hat eine Frist, und `stop()` von innen ist erlaubt UND getestet (`B-CC-11`, G17 Fall 1); nach dem Schließen liefert der Eingang nichts mehr, Schließflag vor Inhalt (`A-P0-09`, jetzt für beide Sichten); Stopp-Fenster bleibt eigener Test. |
| **E** Behauptung ≤ Messung | A21 auf das Gemessene begrenzt, Testname mit (`D-A21-01`); alle Zahlen hier sind gemessen (188, 146/9/9, 32/32, 10 016/10 016) und nicht abgeschrieben; Matrixzeilen werden als Kennung zitiert, nicht als Zeilennummer; der lebende Kopf ist nachgezogen, dieser Abschnitt hängt unten an; jede neue Prüfung wurde einmal absichtlich gebrochen, die roten Rohausgaben liegen in `roh/SONDE-010-bruchproben-phase2.md`. |
| **F** Änderungssatz | `verbinden↔trennen` im selben Commit (`trennmelder_anlegen` ↔ `auf_telemetrie_getrennt_warten` + Entfernen aus dem Register; `kopplung_loesen` liefert das Gegenstück selbst); `starten↔stoppen` beidseitig (`start`/`stop` beider Clients samt Reentranz und Frist); Writer, Reader und Cross-Language-Verbraucher der P1-Politik im selben Änderungssatz (C++ `IpcQueues.h`, Rust `warteschlange.rs`, beide Testseiten, Runnerbehauptung A21). |

### Was diese Runde NICHT getan hat

* **Kein Urteil.** Diese Runde ist Arbeit des Workers; die Wiederprüfung nach
  Vorlage B sieht Matrix und Code gemeinsam.
* **Keine Produktverdrahtung.** Gen und Probeeq sprechen weiterhin v2 über
  `plugin/src/PipeClient`; der SID-gebundene v3-Endpunkt bleibt `SONDE-011`.
* **Kein FL, kein Race Detector** — unverändert zu §6.
* **Keine P1-Last** (NAK-91): A22 fährt P0 und P2. Die P1-Zeilen sind Unittest-
  und Bein-Zusagen.
