# U55 — Zahlenvergleich Worker-Effort `max` gegen `xhigh` (Stand 21.09.2026)

Quellen: `docs/offene-punkte.md` Zeile NAK-335; `docs/beweise/NAK-309.md`,
`docs/beweise/NAK-309-verlauf.md`; `docs/beweise/NAK-311.md`,
`docs/beweise/NAK-311-verlauf.md`; `docs/beweise/NAK-312.md`;
Telemetrie-Cache `%TEMP%\nakama-dirigent-<session>-telemetry.json` (Feld
`ClaudeSevenUsed`, die vom Register benannte Quelle; außerhalb des Repos).
Alles nur gelesen.

## 0. Was am 19.09.2026 entschieden wurde

`docs/offene-punkte.md:338` (NAK-335), User-Wort 19.09.2026 wörtlich: „NEIN
MACHE DIE worker generell auf xhigh. … prüfer bleiben auf max. dann beobachtest
du ob es merkbare unterschiede gibt und in 2 tagen evaluieren wir dann neu".
Umsetzung laut Registerzeile: Opus-Worker (Bauer, Matrix-, Nacharbeits-Worker)
`xhigh`, Prüfer und Validierer `max`, Dirigent `xhigh`, Codex unverändert.
Festzuhalten sind laut Registerzeile: Nacharbeitsrunden bis PASS, Defekte je
Prüfung, Laufzeit je Worker, Claude-Wochenanteil je Tag. Vergleichsbasis `max`:
NAK-309 und NAK-311 bis zur Matrixnacharbeit 1. Erster `xhigh`-Worker:
NAK-311 Matrixnacharbeit 2 (`docs/beweise/NAK-311-verlauf.md:1071`).

## 1. Tabelle je Worker-Lauf

Spalte „Defekte" = vom Dirigenten nach Validierung bestätigte Defekte der
unmittelbar folgenden Prüfung. Spalte „Runden" = Nacharbeitsrunden, bis dieser
Prüfgegenstand PASS war (über alle beteiligten Worker gezählt).

### 1.1 Läufe unter `max`

| Ticket | Etappe / Runde | Rolle | Start–Ende (Laufzeit) | Folgeprüfung | Urteil | Defekte | Runden | Beleg |
|---|---|---|---|---|---|---|---|---|
| NAK-309 | Etappe 1 | Matrix + Bauplan | 18.09. 16:03–17:52 (109 min) | Matrixprüfung 1 | PASS | 0 | 0 | NAK-309-verlauf §9, §10 |
| NAK-309 | Etappe 2 | Bauer (Code) | 18.09. 18:23:36–22:13 (ca. 230 min) | Erstprüfung 2 | PASS | 0 | 0 | NAK-309-verlauf §12, §13 |
| NAK-309 | Etappe 3 | Bauer (Code) | 18.09. 23:01–19.09. 03:51 (ca. 290 min) | Erstprüfung 3 | PASS | 0 | 0 | NAK-309-verlauf §15, §16 |
| NAK-309 | Etappe 4 | Bauer (Code) | 19.09. 04:34:27–09:12:04 (ca. 278 min) | Erstprüfung 4 | PASS | 0 | 0 | NAK-309-verlauf §19, §20 |
| NAK-311 | Etappe 1 | Matrix + Bauplan | 19.09. 10:29–11:58 (89 min) | Matrixprüfung 1 | NEEDS_WORK | 4 Klassen (D-1 bis D-4; drei vom Prüfer benannt, D-4 vom Dirigenten aus H-2 hochgestuft) | 2 | NAK-311-verlauf §10.1, §11, §12.1 |
| NAK-311 | Matrixnacharbeit 1 | Nacharbeit (Doku) | 19.09. Ende 15:49 nach 92 min (Register nennt 1 h 33 min) | Matrixprüfung 2 | NEEDS_WORK | 3 (D-2 Rest, D-3 Rest, Bruch B-1) | — | NAK-311-verlauf §14, §15 |

Zusatz NAK-309 gesamt: Abschlussprüfung T2 PASS, „Keine Nacharbeitsrunde im
ganzen Ticket", vier Baurunden (`docs/beweise/NAK-309.md` §22, Zeile
Prüfkette / Rundenbilanz).

### 1.2 Läufe unter `xhigh`

| Ticket | Etappe / Runde | Rolle | Start–Ende (Laufzeit) | Folgeprüfung | Urteil | Defekte | Runden | Beleg |
|---|---|---|---|---|---|---|---|---|
| NAK-311 | Matrixnacharbeit 2 (erster xhigh-Worker) | Nacharbeit (Doku) | 19.09. 17:46–18:17 (31 min) | Matrixprüfung 3 | PASS | 0 | schließt die 2 Runden der Etappe-1-Matrix ab | NAK-311-verlauf §17, §18 |
| NAK-311 | Etappe 2 Teil a | Bauer (Messgerät, Kanon vorher) | 19.09. 19:25–20:28 (63 min) | – (Erstprüfung 2 prüft a+b) | – | – | – | NAK-311-verlauf §21 |
| NAK-311 | Etappe 2 Teil b | Bauer (Code) | 19.09. 20:33–22:05 (92 min) | Erstprüfung 2 | PASS | 0 | 0 | NAK-311-verlauf §23, §24 |
| NAK-311 | Etappe 3 Teil a | Bauer (Code) | 19.09. 22:42–23:19 (37 min) | – | – | – | – | NAK-311-verlauf §27 |
| NAK-311 | Etappe 3 Teil b | Bauer (Code) | 19./20.09. bis 01:08 (105 min) | Erstprüfung 3 | PASS | 0 | 0 | NAK-311-verlauf §29, §30 |
| NAK-311 | Etappe 4 Teil a | Bauer (Code) | 20.09. 01:33–02:19 (46 min) | – | – | – | – | NAK-311-verlauf §33 |
| NAK-311 | Etappe 4 Teil b | Bauer (Code) | 20.09. 02:21–04:43 (142 min, davon 84 min zwei Kanonläufe) | Erstprüfung 4 | PASS | 0 | 0 | NAK-311-verlauf §35, §36 |
| NAK-311 | Matrixnachtrag Etappe 5 | Matrix | 20.09. 05:07–05:48 (42 min) | Matrixprüfung 4 | NEEDS_WORK | 5 (D-1 bis D-5, alle validiert) | 2 | NAK-311-verlauf §42.1, §43 |
| NAK-311 | Matrixnacharbeit 3 | Nacharbeit (Doku) | 20.09. 07:45–08:20 (35 min) | Matrixprüfung 5 | NEEDS_WORK | 3 offen (D-1 a und d, D-5 neue Fundstelle, R-311-18) | — | NAK-311-verlauf §45, §46 |
| NAK-311 | Matrixnacharbeit 4 | Nacharbeit (Doku) | 20.09. 09:24–09:55 (31 min) | Matrixprüfung 6 | Prüfer NEEDS_WORK, Dirigent: kein Defekt = PASS | 0 | – | NAK-311-verlauf §48, §49.1 |
| NAK-311 | Etappe 5 Satz A | Bauer (Code) | 20.09. 10:33–11:22 (49 min) | – (Erstprüfung 5 prüft A–D) | – | – | – | NAK-311-verlauf §52 |
| NAK-311 | Etappe 5 Satz B | Bauer (Code) | 20.09. 11:27–13:42 (135 min, inkl. Kanon nachher) | – | – | – | – | NAK-311-verlauf §54 |
| NAK-311 | Etappe 5 Satz C, Messlauf C-0 | Bauer (Messlauf) | 20.09. 13:47–14:11 (24 min, auftragsgemäß angehalten) | – | – | – | – | NAK-311-verlauf §56 |
| NAK-311 | Etappe 5 Satz C, Fortsetzung | Bauer (Code) | 20.09. 14:14–15:19 (65 min) | – | – | – | – | NAK-311-verlauf §58 |
| NAK-311 | Etappe 5 Satz D | Bauer (Code) | 20.09. ca. 15:29–17:24 (ca. 115 min, hergeleitet aus den Commitzeiten `cded8a20` und `1183ad9f`; im Manifest keine Uhrzeit) | Erstprüfung 5 (über A–D) | NEEDS_WORK | 2 (D-1 an 311/M-133, D-2 an 311/M-73 bis M-75) | 2 | NAK-311-verlauf §61, §62 |
| NAK-311 | Etappe 5 Nacharbeit 1 | Nacharbeit (Tests) | 20.09. ca. 19:22–19:44 (ca. 22 min, hergeleitet aus `1fff97a5`/`1cf25cba`) | Wiederprüfung 1 | NEEDS_WORK | 1 (Rest von D-2) | — | NAK-311-verlauf §64, §65 |
| NAK-311 | Etappe 5 Nacharbeit 2 | Nacharbeit (Tests) | 20.09. ca. 20:09–20:21 (ca. 12 min, hergeleitet aus `bb210b5e`/`f14daf9d`) | Wiederprüfung 2 | PASS | 0 | – | NAK-311-verlauf §67, §68 |
| NAK-311 | Abschluss Nacharbeit 1 | Nacharbeit (Wortlaut) | 20.09. ca. 21:55–22:17 (ca. 22 min, hergeleitet aus `c4725f00`/`502eb032`) | Wiederprüfung des Abschlusses | PASS | 0 | 1 (Abschlussprüfung T2 hatte 1 Textdefekt) | NAK-311-verlauf §70, §72, §73 |
| NAK-312 | Etappe 1 | Matrix + Bauplan | 21.09. 00:12–00:5x (ca. 40 min) | Matrixprüfung 1 | NEEDS_WORK (Rohurteil, während dieser Auswertung eingetroffen) | 9 vom Prüfer benannt, **noch nicht validiert und nicht vom Dirigenten eingeordnet** | noch 0 gelaufen | NAK-312 §9.1, §9.3; `docs/beweise/roh/NAK-312-matrixpruefung-1-urteil.md` Zeile 1 |

## 2. Aggregat je Effort-Stufe

Matrix- und Nacharbeitsläufe (nur Doku/Matrix) sind von Code-Bauetappen
getrennt; ein Bauwerkzeug- und Kanonlauf kostet Wandzeit, die mit der Denkstufe
nichts zu tun hat.

### 2.1 Matrix- und Matrix-Nacharbeitsläufe

| Größe | `max` | `xhigh` |
|---|---|---|
| Läufe | 3 (NAK-309 E1, NAK-311 E1, NAK-311 Nacharbeit 1) | 5 (NAK-311 Nacharbeit 2, Matrixnachtrag E5, Nacharbeit 3, Nacharbeit 4, NAK-312 E1) |
| Laufzeiten (min) | 109, 89, 92 | 31, 42, 35, 31, ca. 40 |
| Mittlere Laufzeit | 96,7 min | 35,8 min |
| Defekte je Folgeprüfung (validiert und eingeordnet) | 0, 4, 3 → Mittel 2,33 (2 von 3 Prüfungen NEEDS_WORK) | 0, 5, 3, 0 → Mittel 2,0 (2 von 4 NEEDS_WORK); dazu NAK-312 E1 mit 9 Rohdefekten, noch nicht validiert |
| Nacharbeitsrunden bis PASS | NAK-309-Matrix 0; NAK-311-Etappe-1-Matrix 2 (Runde 1 max, Runde 2 xhigh) | Etappe-5-Matrix von NAK-311: 2 Runden (beide xhigh) |

### 2.2 Code-Bauetappen

| Größe | `max` (NAK-309 E2–E4) | `xhigh` (NAK-311 E2–E5 und Abschluss) |
|---|---|---|
| Bauläufe | 3 | 11 (Etappen in zwei bis vier Änderungssätze geteilt) |
| Laufzeit je Lauf (min) | 230, 290, 278 | 63, 92, 37, 105, 46, 142, 49, 135, 24, 65, ca. 115 |
| Mittlere Laufzeit je Lauf | 266 min | 79,4 min |
| Wandzeit je Etappe | E2 230, E3 290, E4 278 min | E2 155, E3 142, E4 188, E5 388 min |
| Prüfungen nach den Bauetappen | 4 (Erstprüfung 2–4, Abschlussprüfung) | 5 (Erstprüfung 2–5, Abschlussprüfung T2) |
| Bestätigte Defekte gesamt / je Prüfung | 0 / 0,0 | 3 / 0,6 (Erstprüfung 5: 2, Abschlussprüfung: 1; Erstprüfungen 2–4: 0) |
| Nacharbeitsrunden bis PASS | 0 | 3 (zwei in Etappe 5, eine im Abschluss) |
| Art der Defekte | – | kein Defekt im Produktcode; drei Textdefekte an Etikett, Zusagetext und Behauptungstext (`docs/beweise/NAK-311.md` §74) |

### 2.3 Der eine fast kontrollierte Vergleich

Dieselbe Aufgabe, derselbe Gegenstand, dieselbe Auftragsvorlage, ein Tag:

- Matrixnacharbeit 1, `max`: 92 min, +404/−82, schloss D-1 und D-4, ließ D-2
  und D-3 offen und brachte den Bruch B-1 (NAK-311-verlauf §14, §15).
- Matrixnacharbeit 2, `xhigh`: 31 min, +379/−62, schloss alle drei Restpunkte,
  Matrixprüfung 3 PASS ohne Defekt (NAK-311-verlauf §17, §18).

Der Dirigent hat schon im Manifest dazugeschrieben: „Die Umfänge sind nicht
gleich" (NAK-311-verlauf §17).

Zweiter, schwächerer Vergleich — Etappe-1-Matrix je Audit-Ticket:
NAK-309 `max` 109 min / 73 Zeilen / +1339−2 → PASS beim ersten Mal;
NAK-311 `max` 89 min / 83 Zeilen / +1344−2 → NEEDS_WORK, 4 Defektklassen,
2 Runden; NAK-312 `xhigh` ca. 40 min / 77 Zeilen / +1210−2 → NEEDS_WORK mit
9 Rohdefekten (Prüferurteil vom 21.09.2026, noch nicht validiert; zum Vergleich:
bei NAK-311 benannte der Prüfer 3, der Dirigent bestätigte nach Validierung 4).
Unter `max` liegt damit sowohl das beste (NAK-309, PASS beim ersten Mal) als
auch ein gerissenes Matrixergebnis; der bisher höchste Rohdefektstand liegt bei
einem `xhigh`-Lauf.

## 3. Wochenanteil (Claude)

Datierte Werte. Reset am 18.09.2026, 15:26 Uhr auf 0 % (`docs/beweise/NAK-309.md`
§0). Die Werte mit „(M)" stehen in den Manifesten, die übrigen im
Telemetrie-Cache, den die Registerzeile NAK-335 als Quelle nennt.

| Zeitpunkt | Claude-Woche | Quelle |
|---|---|---|
| 18.09. 15:26 | 0 % | NAK-309 §0 (M) |
| 19.09. 04:30 | 15 % | Telemetrie-Cache |
| 19.09. 10:03 | 20 % | Telemetrie-Cache |
| 19.09. 13:16 | 25 % | Telemetrie-Cache |
| 19.09. 19:20 / 19:22 | 32 % / 33 % | Cache / NAK-311-verlauf §19 (M) |
| 20.09. 01:29 | 39 % | NAK-311-verlauf §31 (M) |
| 20.09. 07:44 | 45 % | Telemetrie-Cache |
| 20.09. 10:31 | 49 % | NAK-311-verlauf §50 (M) |
| 20.09. 15:23 / 15:25 | 54 % | Cache / NAK-311-verlauf §59 (M) |
| 20.09. 23:31 | 61 % | Telemetrie-Cache |
| 21.09. 01:10 | 64 % | Telemetrie-Cache |

Verbrauch je Abschnitt:

- 18.09. ab 15:26 bis 19.09. 04:30: **15 Punkte** (Nacht mit NAK-309 Etappen 2
  und 3, `max`).
- 19.09. 04:30 bis 20.09. 07:44: **30 Punkte**, davon bis 19:20 Uhr 17 Punkte.
- 20.09. 07:44 bis 23:31 (voller `xhigh`-Tag): **16 Punkte**.
- 20.09. 23:31 bis 21.09. 01:10: **3 Punkte**.

Nach Effort-Phase (Schnitt 19.09. 17:45 Uhr, erster `xhigh`-Worker 17:46 Uhr):

- `max`-Phase 18.09. 15:26 bis 19.09. ca. 17:45 (26,3 h): 0 % → ca. 30 %
  (zwischen 25 % um 13:16 und 32 % um 19:20 interpoliert) ≈ **1,14 Punkte je
  Stunde Wandzeit**.
- `xhigh`-Phase 19.09. 17:46 bis 21.09. 01:10 (31,4 h): ca. 30 % → 64 %
  = 34 Punkte ≈ **1,08 Punkte je Stunde Wandzeit**.

Der Unterschied von rund 5 % liegt innerhalb der Interpolationsunsicherheit und
ist **nicht belegbar** als Effekt der Denkstufe: in beiden Phasen zählen
Dirigent, Prüfer und Validierer (alle `max`) sowie Kanonläufe mit. Was sich
unterscheidet: in der `xhigh`-Phase liefen in 31 h 14 Bau- und
Nacharbeitsworker plus 5 Matrixläufe, in der `max`-Phase in 26 h 6 Läufe — je
Lauf kostet `xhigh` also deutlich weniger Wochenanteil, je Stunde praktisch
gleich viel.

## 4. Ehrliche Einschränkungen

1. **Stichprobe.** 6 Läufe unter `max` gegen 19 unter `xhigh`, aus drei
   Tickets, in 2,5 Tagen, alle von einem Dirigenten beauftragt. Für Aussagen
   über Güte ist das zu wenig; ein Unterschied in „Defekte je Prüfung" von 2,33
   gegen 2,0 (Matrix) ist bei diesen Zahlen Rauschen.
2. **Nicht vergleichbare Gegenstände.** NAK-309 (Beweiswerkzeuge, Baukette,
   Hostwerkzeuge) und NAK-311 (Audio-Kern: Pfadrampen, Slot-Lebenszyklus,
   Auto-Gain, Dynamikschwelle) sind verschieden schwer. Die Etappe-5-Matrix von
   NAK-311, die 5 Defekte kassierte, ist der schwerste Matrixgegenstand beider
   Tickets (47 Zeilen DSP-Dynamik).
3. **Zuschnitt geändert.** NAK-309 fuhr eine Etappe mit einem Worker, NAK-311
   teilte jede Etappe in zwei bis vier Änderungssätze. Die mittlere Laufzeit je
   Lauf (266 gegen 79 min) misst vor allem diesen Zuschnitt, nicht die
   Denkstufe. Aussagekräftiger ist die Wandzeit je Etappe (230/290/278 min
   unter `max` gegen 155/142/188/388 min unter `xhigh`) — und auch die ist
   durch den Inhalt der Etappen verzerrt.
4. **Kanonläufe in der Laufzeit.** Mehrere `xhigh`-Läufe enthalten volle
   Kanonläufe (Etappe 2 Teil a, Etappe 4 Teil b mit 84 min Kanon von 142 min,
   Etappe 5 Satz B und D). Die `max`-Läufe von NAK-309 ebenso. Reine Denkzeit
   ist in keiner Quelle getrennt ausgewiesen.
5. **Verschärftes Prüfregime.** Zwischen NAK-309 und NAK-311 wurden
   Prüfvorlagen, Regeln und die Validierungsstufe verschärft; eine Prüfung im
   `xhigh`-Zeitraum sucht schärfer als eine im `max`-Zeitraum. Ein Defektrunde
   in NAK-311 ging zudem auf eine Regel des Dirigenten selbst zurück
   (`docs/beweise/NAK-311.md` §74, Verweis auf §65).
6. **Was in den Quellen fehlt.** Für vier `xhigh`-Läufe (Satz D, Etappe-5-
   Nacharbeit 1 und 2, Abschluss-Nacharbeit 1) steht keine Uhrzeit im Manifest;
   die Laufzeiten sind aus Commitzeiten hergeleitet und damit Obergrenzen. Die
   Kontextverdichtungen und Werkzeugaufrufe je Worker sind nirgends gezählt
   (für Prüfer teils schon: 46/75/84 Aufrufe in NAK-309). Für NAK-312 liegt nur
   das Rohurteil vor: neun Defekte sind die Zählung des Prüfers, ohne
   Validierung und ohne Einordnung des Dirigenten; bei NAK-311 wurden aus 3
   benannten nach Validierung 4 bestätigte, bei der Matrixprüfung 6 aus einem
   benannten Defekt keiner. Die Zahl 9 ist deshalb noch keine Defektzahl im
   Sinn dieser Tabelle. Ein Claude-Wochenwert je Tag „ab Reset" lag nur im
   Telemetrie-Cache, nicht in den Manifesten; der Cache ist rechnerlokal und
   nicht Teil des Repos.
7. **Keine Empfehlung aus diesen Zahlen.** Belegbar ist: unter `xhigh` sind
   die Läufe kürzer (Matrixarbeit 36 gegen 97 min im Mittel), im Produktcode
   entstand in NAK-311 kein einziger bestätigter Defekt, und die drei
   Defektrunden betrafen Text (Etikett, Zusage, Behauptung). Ebenfalls
   belegbar: unter `max` hatte NAK-309 null Nacharbeitsrunden im ganzen Ticket,
   und die einzige `max`-Matrix mit vier Defektklassen (NAK-311 Etappe 1) zeigt,
   dass auch `max` reißt. Gegen `xhigh` steht das frische Rohurteil der
   NAK-312-Matrixprüfung 1 mit neun benannten Defekten — bis zur Validierung
   trägt es aber keine Aussage. Ob die Denkstufe die Ursache eines dieser
   Unterschiede ist, ist **nicht belegbar**.

## 5. Nachtrag des Dirigenten (21.09.2026, 02:3x Uhr)

Zusammengestellt von einem lesenden Opus-Agenten im Auftrag des Dirigenten (Session `90fe90ab`), 21.09.2026, 01:1x Uhr; vom Dirigenten gelesen, nicht Zeile für Zeile nachgemessen.
Seitdem ist die Matrixprüfung 1 von NAK-312 an der Quelle validiert und eingeordnet (`docs/beweise/NAK-312.md` §11): aus neun Rohdefekten, drei Lücken und drei Härtungen wurden **13 bestätigte Defekte** für die Matrixnacharbeit 1, eine Lücke mit Regel und eine Härtung. Das ist der höchste Defektstand einer Etappe-1-Matrix der drei Tickets; der Gegenstand (Hostkante, Lebenslauf, Nebenläufigkeit über 14 Befunde) ist zugleich der breiteste.
Die Matrixnacharbeit 1 von NAK-312 lief unter `xhigh` von 01:59 bis 02:32 Uhr (33 min, +728/−147); ihre Wiederprüfung steht aus.
Die Einschränkungen aus Abschnitt 4 gelten unverändert: die Zahlen tragen keine Empfehlung.
