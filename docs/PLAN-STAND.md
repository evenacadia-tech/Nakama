# Planstand Nakama

<!-- quellstand: 4cc0a2d -->

> **Gerechnet, nicht gepflegt.** Dieses Blatt entsteht aus dem Repo:
> `py -3.13 tools/plan/planstand.py`. Es wird **nie** von Hand editiert —
> jeder Lauf überschreibt es. Was hier steht, ist gemessen:
> ein Schritt gilt als *gebaut*, wenn sein Beweismanifest liegt, und als
> *abgenommen* erst, wenn dort eine Urteilsmarke der geforderten Prüfstufe
> mit **PASS** steht. Fehlt sie, gilt der Schritt als nicht abgenommen.

**Stand:** 2026-08-29 · Quellstand `4cc0a2d` · **14 von 38 abgenommen** · 5 gebaut · 19 offen

> ⚠️ Gerechnet aus dem Arbeitsbaum: unter `docs/plan/`, `docs/beweise/`
> oder `tools/plan/` liegen Änderungen, die noch nicht in `4cc0a2d` sind.

`███████████████▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░` 37 % abgenommen · 50 % gebaut

**Als Nächstes:** **Nacharbeit an G1** — der Prüfer hat einen Befund offen gelassen (docs/beweise/G1.md).

**Wartet auf ein Urteil** (gebaut, nachgemessen, aber ohne PASS eines frischen Prüfers): `S8` · `S9` · `S9b` · `S14–15`

**Bei dir liegen 2 Fragen** — `U11, U13`. Sie werden im Chat gestellt: Skill `/fragen`.

## Phasen auf einen Blick

| Phase | Fortschritt | abgenommen | gebaut | offen |
|---|---|---:|---:|---:|
| **Vorlauf** — Beweisen statt behaupten | `████████████████████████` | 1 | 0 | 0 |
| **P0** — Bestand einfrieren, Hostgrenzen beweisen | `████████████████████████` | 8 | 0 | 0 |
| **P1** — Verträge, gespeicherter Zustand, neutrale Hüllen | `██████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓` | 3 | 4 | 0 |
| **P2** — Messkern, Nachrichtenweg, Speicher | `████████▓▓▓▓░░░░░░░░░░░░` | 2 | 1 | 3 |
| **P3** — Passive Landkarte | `░░░░░░░░░░░░░░░░░░░░░░░░` | 0 | 0 | 2 |
| **P4–P5** — Vergleichsevidenz und Ursachen | `░░░░░░░░░░░░░░░░░░░░░░░░` | 0 | 0 | 4 |
| **P6–P7** — Aktiver Kern: EQ in Sonde und Gen | `░░░░░░░░░░░░░░░░░░░░░░░░` | 0 | 0 | 6 |
| **P8–P9** — Entmaskierung und Härtung | `░░░░░░░░░░░░░░░░░░░░░░░░` | 0 | 0 | 4 |

## Der Weg

```mermaid
flowchart LR
  P0["Vorlauf<br/>1/1 abgenommen"]:::fertig
  P1["P0<br/>8/8 abgenommen"]:::fertig
  P0 --> P1
  P2["P1<br/>3/7 abgenommen<br/>+4 gebaut"]:::laeuft
  P1 --> P2
  P3["P2<br/>2/6 abgenommen<br/>+1 gebaut"]:::laeuft
  P2 --> P3
  P4["P3<br/>0/2 abgenommen"]:::offen
  P3 --> P4
  P5["P4–P5<br/>0/4 abgenommen"]:::offen
  P4 --> P5
  P6["P6–P7<br/>0/6 abgenommen"]:::offen
  P5 --> P6
  P7["P8–P9<br/>0/4 abgenommen"]:::offen
  P6 --> P7
  classDef fertig fill:#1f6f43,stroke:#2ea36a,color:#eaf6ef
  classDef laeuft fill:#7a5a12,stroke:#c99a2e,color:#fdf6e6
  classDef offen  fill:#2a2f36,stroke:#4a525c,color:#c7ced8
```

## Alle Schritte

### Vorlauf — Beweisen statt behaupten  (1/1 abgenommen)

*Ein Befehl fährt alle Prüfungen und schreibt die rohe Ausgabe in ein Manifest — das ersetzt eine CI.*

- ■ **S0** — Beweis-Runner, Manifest-Vorlage, Basislinie. Heute 18 Prüfbeine. (abgenommen · T1 PASS 2026-08-20 · Kanon 4/4 grün)

### P0 — Bestand einfrieren, Hostgrenzen beweisen  (8/8 abgenommen)

*Nichts bauen, was FL Studio am Ende nicht hergibt: erst messen, was der Host kann, und die Plugin-Kennungen festschreiben, damit alte Projekte immer laden.*

- ■ **S1** `SONDE-004a` — Wegwerf-Messgerät für Termin A: zwei Nebenwege (Aux) und Latenzausgleich messbar machen. (abgenommen · T1 PASS 2026-08-20 · Kanon 5/5 grün)
- ■ **Termin A** `FL, du` — Nebenwege und Latenzausgleich in FL gemessen: geht, samplegenau, überlebt Speichern/Laden. (abgenommen · gemessen (Rohdaten))
- ■ **S2** `SONDE-001/002` — Identität eingefroren: Kennungen aller drei Plugins festgeschrieben und mit 63 Prüfungen bewacht, damit FL alte Projekte weiter erkennt. (abgenommen · T2 PASS 2026-08-20 · Kanon 5/5 grün)
- ■ **S3** `SONDE-003` — Hostbrücke: ein Patch am Plugin-Rahmenwerk macht sichtbar, was FL liefert (Transport, Latenz, Automationspunkte). 91 Prüfungen. (abgenommen · T2 PASS 2026-08-21 · Kanon 6/6 grün)
- ■ **S3b** `Nachtrag` — Messgerät für Termin B (Host-Probe): zeichnet Zeitsprünge, Render, Automation als JSON auf. 85 Prüfungen. (abgenommen · T2 PASS 2026-08-21 · Kanon 7/7 grün)
- ■ **Termin B** `FL, du + Claude` — Hostzeit und Automation in FL gemessen (12:45–13:27): Kontext in allen 259 298 Blöcken, Sprünge/Schleifen/Render/Automation sauber getrennt gemeldet; FL liefert nur float. Du hast aufgebaut, Claude ist über den FL-MCP gefahren. (abgenommen · gemessen (Rohdaten))
- ■ **S4** — Capabilityreport: die zehn Fähigkeitsbits für FL an die Rohdaten aus Termin A und B gebunden — zwei bestätigt (Hostkontext, Projektzeit), acht nicht: zwei gemessen „kann FL nicht“ (feine Automation, double), drei „noch nicht bewiesen“ (Latenzangabe, beide Nebenwege — Termin A2), eines ungemessen, zwei warten auf ihre Tickets. Prüfbein A13 (61 Prüfungen) misst den Report selbst gegen die Rohdaten; Kanon 18/18 grün. Frischer Prüfer: Runde 1 NEEDS_WORK (zwei Bits zu optimistisch), nachgearbeitet, Runde 2 PASS. (abgenommen · T2 PASS 2026-08-22 · Kanon 18/18 grün)
- ■ **G0** `Gate` — Erste adversariale Pruefrunde (C++-Review + Codex) ueber P0 — gefahren 22.08., Urteil PASS: beide Bruchauftraege (Gate 1, Gate 5) gescheitert, die P0-Kernflaeche traegt keinen Befund. Manifest docs/beweise/G0.md. Damit ist P0 vollstaendig. (abgenommen · T3 PASS 2026-08-22)

### P1 — Verträge, gespeicherter Zustand, neutrale Hüllen  (3/7 abgenommen, 4 gebaut)

*Alles, was zwischen den drei Apps und dem Broker hin- und hergeht, ist als Vertrag festgeschrieben und in drei Sprachen gleich geprüft — bevor der Messkern darauf baut.*

- ■ **S5** `SONDE-005a` — Nachrichtenverträge (JSON) mit Bandgitter und 153 Prüffällen; in Python, C++ und Rust gleich gelesen. Gebaut und nachgearbeitet — das abschließende Prüfurteil eines frischen Prüfers steht noch aus. (abgenommen · T3 PASS 2026-08-28 · Kanon 29/29 grün)
- ■ **S6** `SONDE-005b` — Binärformat für Messdaten (FlatBuffers) mit festen Feldnummern und zwei handgeschriebenen Lesern; 6215 Byte-Mutanten bestanden. Prüfurteil wie S5 noch offen. (abgenommen · T3 PASS 2026-08-28 · Kanon 29/29 grün)
- ■ **S7** `SONDE-006` — Gespeicherter Zustand Schema 2: alte Projekte wandern verlustfrei, fremde Versionen werden nur-lesend geöffnet, FL sieht jede Änderung als „ungespeichert“. 109 Parameter-Kennungen festgeschrieben. (abgenommen · T3 PASS 2026-08-28 · Kanon 29/29 grün)
- ▣ **S8** `SONDE-007a` — Gemeinsamer Kern fuer alle drei Plugins, der keine Bundle-Konstanten sieht — sonst bekaemen zwei Plugins die Identitaet des dritten. Gebaut: der geteilte Code wird jetzt EINMAL uebersetzt statt einmal je Programm, und fuenf unabhaengige Sperren passen auf, dass keine Kennung hineinrutscht. Jede Sperre wurde absichtlich ausgeloest, um zu zeigen, dass sie wirklich zufasst. Eine davon hat dabei einen Fehler in sich selbst gefunden. NACHGEPRUEFT am 23.08.: das Herzstueck haelt (der Kern traegt nachweislich keine Kennung), aber das Urteil lautete 'nachbessern' — fuenf Punkte, darunter eine echte Verschlechterung durch den Umbau selbst: der geteilte Code hatte still die schaerfste Warnstufe des Compilers verloren. Alle fuenf noch am selben Tag geschlossen, die fuenfte Sperre ist genau daraus entstanden, danach wieder 19 von 19 Pruefungen gruen. Auf den nachgebesserten Stand fehlt ein zweites Urteil. (gebaut · T3 NEEDS_WORK 2026-08-28 · nachgearbeitet, frisches Urteil fehlt · Kanon 29/29 grün)
- ▣ **S9** `SONDE-007b` — Drei eigene Plugin-Ziele, Rollen-Erkennung, Installer-Manifest. ALLE DREI TEILE GEBAUT am 23.08. (1) Die Kennung der Programme stand bisher als Text im Bauskript UND in der Kennungsdatei - zwei Wahrheiten, die auseinanderlaufen koennen. Jetzt liest das Bauskript die Kennungsdatei; der Test misst weiter das fertige Programm gegen dieselbe Datei. (2) Nakama Suna und Nakama Probeeq sind gebaut, aus EINER gemeinsamen Quelle, und tragen nachweislich ihre eigenen, seit Tagen reservierten Kennungen - keines traegt die eines anderen. Beide sind heute noch stumm: Ton geht unveraendert durch, keine Regler, keine Oberflaeche. Das ist Absicht. (3) Das Hauptprogramm erkennt jetzt seine Rolle, statt sie anzunehmen: beim Laden weiss es nichts ueber sich und bleibt still; erst ein geladenes Projekt entscheidet, ob es ein alter Messpunkt (dann fuer immer passiv) oder ein Hauptfenster ist. Eine frische Instanz wird nur dann zum Hauptfenster, wenn du das Fenster geoeffnet UND die Rolle gewaehlt hast. Ein Scannerlauf entscheidet nichts. Dazu die Packliste fuer die Auslieferung (drei Programme plus Broker, Pruefsumme, Rueckweg mit Warnung vor Datenverlust) - und das Installationsskript liegt endlich im Projekt statt nur auf einem Rechner. FOLGE, DIE MAN HOERT: ein reiner Messpunkt faerbt beim Anhoeren nichts mehr ein. 23 von 23 Pruefungen gruen. NICHT abgenommen: die Nachpruefung durch einen frischen Pruefer steht aus,; das fremde Pruefprogramm sagt bei allen drei Programmen SUCCESS. NACHARBEIT 23.08. nachmittags: alle vier Funde der Nachpruefung geschlossen, jeder erst an der Quelle nachgemessen und jede neue Sicherung beim Anschlagen vorgefuehrt; fuenf weitere Funde kamen dabei heraus und sind mit erledigt. Ein Plugin wird ab jetzt als ganzer ORDNER ausgeliefert statt nur als die Datei darin. Das Zurueckgehen wird ab jetzt bei jedem Pruefdurchlauf wirklich AUSGEFUEHRT (neues Pruefbein, Kanon 23 -> 24) - dabei fielen zwei echte Fehler heraus, die drei Leser uebersehen hatten. Selbstpruefung ueber den Gesamtstand gefahren, 24/24 gruen und beglaubigt. STATUS BLEIBT 'gebaut': wer repariert, spricht sich nicht selbst frei - der nachgebesserte Stand braucht einen frischen Pruefer. NACHTRAG 28.08.: Suna ist seit 23.08. in Probeeq aufgegangen; die Stilllegung des dritten Ziels ist als S9b (SONDE-007c) vorgezogen. Der offene FL-Schritt gilt danach fuer beide verbleibenden Ziele: Nakama Gen und Nakama Probeeq. (gebaut · T3 NEEDS_WORK 2026-08-29 · nachgearbeitet, frisches Urteil fehlt · Kanon 32/32 grün)
- ▣ **S9b** `SONDE-007c` — Suna ist seit dem 23.08. in Probeeq aufgegangen; Bau, Installer-Manifest und Identitätsdatei sind aber noch dreispurig. Vorgezogen aus S28b am 28.08.2026 (User: „ja stell das einfach richtig“, design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md), damit der FL-Schritt von S9 das Produkt misst, das es gibt. Das Ziel NakamaSuna_VST3 wird stillgelegt; sein Prüfbein A15 verabschiedet sich deklariert aus dem Kanon (tools/beweise.ps1-Tabelle), nicht still; das Installer-Manifest liefert zwei Bundles plus Broker; der Suna-Eintrag in plugin-identities-v1.json wird als stillgelegt markiert, ohne eine eingefrorene Kennung von Gen oder Probeeq anzufassen (NAK-30 gilt weiter); der Rückweg bleibt vollständig. Gate: Kanon grün ohne A15, Installer-Manifestprüfung A17 grün mit zwei Bundles, pluginval SUCCESS an beiden Bundles, kein Bundle trägt eine fremde CID. Danach lautet §55 Klausel 1: beide Ziele scannen und laden in FL. (gebaut · T2 NEEDS_WORK 2026-08-29 · nachgearbeitet, frisches Urteil fehlt · Kanon 28/28 grün)
- ▣ **G1** `Gate` — Prüfrunde über P1 (C++- und Rust-Review + Codex) — gefahren 24.08., Urteil NEEDS_WORK: Gate 1 hielt (beide Modelle „kein Pfad gefunden“), Gate 7 fiel — von Claude über das v3-Schema (insert + beitrag validiert) und unabhängig davon von Codex über den gespeicherten Zustand (eine Sonde ohne Aux-Bus darf sich post_fader_contribution nennen). Dazu zwei offene §55-Exit-Klauseln: die drei Bundles sind nie in FL gescannt worden, und Rust klassifiziert die 15 ungültigen State-DTOs nicht (NAK-42). Kanon 28/28 grün und beglaubigt. Manifest docs/beweise/G1.md. (gebaut · T3 NEEDS_WORK 2026-08-24 · Befund offen · Kanon 28/28 grün)

### P2 — Messkern, Nachrichtenweg, Speicher  (2/6 abgenommen, 1 gebaut)

*Die größte Phase: Audio wird zeitgestempelt gemessen, über den Broker verteilt und gespeichert — ohne je den Audiothread zu blockieren. Danach: Release R0 (Vertrag steht, intern).*

- ■ **S10–11** `SONDE-008` — Zeitgestempelte Audio-Warteschlange, Quarantaene fuer kaputte Bloecke, Lautheitsmessung mit festem Speicher. GEBAUT 23.08. — der gefaehrlichste Eingriff der ganzen Phase, weil er mitten im Audiothread sitzt. Bisher gab die Weitergabe an die Messung bei Platzmangel einen HALBEN Block weiter und zaehlte den Rest; die Messung sah danach einen lueckenlosen Strom, dem in der Mitte Zeit fehlte, und konnte das nicht mehr merken. Jetzt gilt ganz oder gar nicht: passt ein Block nicht, faellt er komplett, wird gezaehlt, und der naechste traegt die Markierung 'hier fehlt etwas'. Dazu haelt die Messung jeden Block einen Moment zurueck, bis der naechste beweist, dass er lueckenlos anschliesst — sonst koennte ein erst nachtraeglich sichtbarer Schleifensprung eine schon veroeffentlichte Auswertung verderben. Und die Lautheitsmessung sammelt nicht mehr endlos: sie braucht ab jetzt immer gleich viel Speicher, egal ob fuenf Minuten oder fuenf Stunden laufen — eine Million Messzellen ohne eine einzige Speicheranforderung, vorgefuehrt. Zwei neue Pruefungen (Kanon 24 → 26), alle 26 gruen und beglaubigt, Fremdpruefer pluginval auf hoechster Stufe an allen drei Plugins bestanden. Kein Sample Audio hat sich geaendert. NACHGEPRUEFT UND NACHGEBESSERT 23.08. abends: ein frischer Pruefer hat vier Sachen gefunden, alle vier haben sich bestaetigt, alle vier sind geschlossen - die Lautheitsmessung ist jetzt auch bei absurd lautem Material genau (statt nur ehrlich darueber), ihre eingebaute Pruefung kann den Fall ueberhaupt erst sehen, deine Entscheidung vom 22.08. hat eine Sicherung, und zwei Raender an der Zeitrechnung stimmen. Jede neue Pruefung wurde einmal absichtlich kaputtgemacht und schlug an. Alle 26 gruen und beglaubigt. Steht weiterhin auf 'gebaut' und nicht auf 'erledigt': wer repariert, spricht sich nicht selbst frei - das bestaetigt ein frischer Pruefer. (abgenommen · T2 PASS 2026-08-24 · Kanon 28/28 grün)
- ■ **S12–13** `SONDE-009` — Messkern v2: Zeit-, Gueltigkeits-, Ereignis- und Bandvertraege. GEBAUT 23.08. — die Schicht, die aus gemessenem Audio ehrliche Zahlen macht. Der Auftrag stand in einem einzigen Satz: 'Drop/Seek/Loop trennt jedes offene Fenster.' Was das heisst: die Messung sammelt staendig ueber laengere Abschnitte (bis zu einer Drittelsekunde), um ueberhaupt etwas ueber tiefe Toene sagen zu koennen. Springt der Abspielzeiger mittendrin an eine andere Stelle, oder wiederholt sich eine Schleife, oder faellt ein Stueck Audio weg, dann liegen in so einem angefangenen Abschnitt ZWEI verschiedene Stellen der Musik. Die daraus gerechnete Zahl sieht aus wie eine Messung und ist keine. Jetzt wird an jeder solchen Stelle alles Angefangene weggeworfen und neu begonnen — und zwar an NEUN verschiedenen Ereignissen, jedes einzeln nachgewiesen. Dazu die Bandaufteilung des Vertrags als eingefrorene Zahlen im Programm (mit zwei unabhaengigen Waechtern) und der eine Weg, ueber den das Plugin im echten FL seine Zeitinformation bekommt, ist ERSTMALS wirklich gefahren worden statt nur gelesen. T2-GEPRUEFT 23.08.: Urteil NEEDS_WORK. Der schwierige Teil haelt - alles, was waehrend eines angefangenen Abschnitts Klang sammelt, wird wirklich weggeworfen, auch der unsichtbare Nachklang des Filters. Gefunden wurde ein Fehler eine Ebene darueber: die fertigen Messwerte (die 64 Balken, der feine Schnappschuss, die Angabe wieviel Klang da war) werden beim Sprung NICHT mit weggeworfen, so dass eine Meldung mit 'neue Stelle' beschriftet sein und ausschliesslich den Klang der alten enthalten kann - gemessen an 80 von 120 durchprobierten Zeitpunkten. Nicht abgenommen; Nacharbeit ist der naechste Schritt. (abgenommen · T2 PASS 2026-08-24 · Kanon 28/28 grün)
- ▣ **S14–15** `SONDE-010` — Nachrichten-Clients in den Plugins und der Parser im Broker. GEBAUT 29.08. — die Leitung, über die Sonden und Vermittler künftig miteinander reden. Bisher gab es dafür nur den alten, einspurigen Weg: ein Plugin, eine Verbindung, alles hintereinander. Das Problem daran hört man nicht, aber es wäre da: fließen viele Messdaten, dann steht ein Befehl ("stell den EQ um") hinter tausend Spektren in der Schlange. Jetzt gibt es ZWEI getrennte Leitungen je Sonde — eine für Befehle, eine für Messdaten — und sie gehören nachweislich zusammen: die zweite wird nur angenommen, wenn sie drei Merkmale der ersten zitieren kann. GEMESSEN, nicht behauptet: 32 Sonden fluten gleichzeitig Messdaten, so stark, dass 43 133 Messframes bewusst verworfen werden — und in derselben Zeit kommen 10 016 von 10 016 Befehlen an, die langsamste Antwort nach 21 Millisekunden. Dazu ein Briefumschlag mit Prüfsumme vor jedem Paket: 37 Testdateien werden von C++ und Rust BUCHSTABENGLEICH beurteilt, jede der 14 Ablehnungsregeln hat ihren eigenen Fehlerfall, und ein einziges gekipptes Bit fällt in 3000 von 3000 Versuchen auf. Kanon 28 → 32 Prüfungen; jede neue wurde einmal absichtlich kaputtgemacht und hat angeschlagen. WAS BEWUSST NICHT PASSIERT IST: die beiden Apps sprechen noch den alten Weg. Der neue ist gebaut, gelinkt und über eine echte Leitung bewiesen, aber erst der nächste Schritt (S16–17) schaltet ihn ein — vorher gäbe es niemanden, der die Daten entgegennimmt und aufhebt. Ein Anschluss, der nichts tut, wäre genau die Art totes Element, die hier nicht gebaut wird. NACHGEARBEITET 29.08.: der frische Prüfer hat NICHT freigegeben, sondern zehn echte Fehler gefunden — alle zehn sind an der Quelle behoben und mit je einer Prüfung belegt, die ohne den Fix rot ist. Die größten drei: die Leitung nahm Messdaten und Befehle wahllos auf beiden Spuren an; fiel die Befehlsleitung weg, sendete die Messleitung weiter ins Leere; und der Vermittler blieb bei 96 gleichzeitigen Verbindungen für immer taub, auch nachdem alle wieder frei waren. Dazu: eine Meldung, deren Übertragung mitten im Abbruch scheiterte, ging verloren statt zurück in die Schlange. ZWEITE NACHARBEIT 29.08.: der nächste frische Prüfer hat wieder NICHT freigegeben — sieben weitere Fehler, alle sieben behoben und mit je einer Prüfung belegt, die ohne den Fix rot ist. Der schwerste: ein Befehl, der gerade unterwegs zur Leitung war, konnte verlorengehen, wenn währenddessen die Warteschlange volllief — genau das, was bei Befehlen nie passieren darf, und der Zähler meldete den Verlust nicht einmal. Dazu: die Messleitung merkte nicht, dass ihre Befehlsleitung fort war, solange gerade nichts zu messen war; die Kontrolle des Begrüßungspakets war nachlässiger als der Vertrag verlangt — eine leere Angabe kam als das Wort für „leer“ durch; Messdaten auf der Befehlsspur wurden weitergereicht statt abgewiesen; das vereinbarte Tempolimit galt nur im Test, nicht im Betrieb; und der Vermittler konnte beim Beenden für immer warten — einmal auf einen gerade erst gestarteten Anschluss, einmal auf fremden Code, der nicht zurückkam. Die Prüfungen des Plugin-Beins wuchsen dabei von 115 auf 159. STATUS weiter 'gebaut', nicht 'erledigt': wer repariert, spricht sich nicht selbst frei — das bestätigt erneut ein frischer Prüfer. (gebaut · T2 NEEDS_WORK 2026-08-29 · nachgearbeitet, frisches Urteil fehlt · Kanon 32/32 grün)
- □ **PR1** `Planprüfung` — Zwischenhalt der Implementierung. User-Wort 29.08.2026: „ich möchte dass du einen guten zeitpunkt im plan raussuchst, an dem wir dne implement temporär auf halt machen und eine ausführliche überprüfung des Gesamtplans machen. die muss besonders darauf abzielen ob änderungen die wir im verlauf beschlossen haben nicht an einer anderen stell iwas brechen. zudem müssen wir sichergehen keine standard features vergessen zu haben, die vergleichbare plugins von anderen herstellern haben. mögliche anpassungen müssen dann direkt sauber in den plan eingearbeitet werden" Zeitpunkt (Wahl des Dirigenten, 29.08.): nach dem PASS von G1 und den dritten Nacharbeitsrunden von S8, S9, S9b und S14–15, VOR S16–17 — weil SONDE-011 mit Koordinator, Datenbank-Migration und Ausgangspuffer die Datenformen festschreibt, die eine nachgeholte Standardfunktion am ehesten ändern würde, und G2 den Vertrag danach als R0 einfriert. Inhalt: (1) jeden Entscheid seit Projektstart (design/abnahmen/, beantwortete Fragen in docs/plan/fragen.json, Nachträge in docs/offene-punkte.md, Wahrheitskern in CLAUDE.md) gegen Entwurf, Bauaufteilung, Verträge (v3, State, Installer) und die gebauten Tickets auf Widersprüche und Folgebrüche prüfen — jeder Treffer mit Fundstelle; (2) Funktionsvergleich mit vergleichbaren Produkten anderer Hersteller (Mix-Analyse mit Sonden, Spektrum-/Maskierungs-Werkzeuge, EQ-Fernsteuerung) anhand aktueller Herstellerdokumentation: Standardfunktionen, die Nakama fehlen oder anders löst, benennen und je Punkt sagen, ob Vertrag, State oder Oberfläche betroffen wären; (3) jede Anpassung sauber einarbeiten — plan.json, bauaufteilung-sonden.md, Entwurf, betroffene Ticketquellen; Produktentscheide gehen als Fragenkarten an den User, Technik entscheidet der Dirigent. Prüfung: frischer Codex-Thread über Plan und Änderungssatz, Recherche mit Belegen; Ergebnis in docs/beweise/PR1.md mit Urteilsmarke. Bis zu diesem Punkt läuft die Implementierung weiter. (offen · Beleg `docs/beweise/PR1.md` fehlt)
- □ **S16–17** `SONDE-011` — Koordinator im Broker, Datenbank-Migration, Ausgangspuffer. (offen)
- □ **G2** `Gate` — Volles Programm: C++-, Rust- und Sicherheits-Review + Codex. (offen)

### P3 — Passive Landkarte  (0/2 abgenommen)

*Gen zeigt alle Quellen mit Frische und Messpunkt — ehrlich, auch wenn etwas fehlt. Danach: Release R1.*

- □ **S18–19** `SONDE-012` — Quellen verbinden und führen, Frische anzeigen, Messpunkt-Wahrheit, Fehlerzustände. (offen)
- □ **G3** `Gate` — Rust-Review + Codex + 60-Minuten-Dauerlauf. Vorher fällig: deine fünf Entscheide aus U9. (offen)

### P4–P5 — Vergleichsevidenz und Ursachen  (0/4 abgenommen)

*Der Advisor: aus Messungen werden belegte Befunde, mögliche Ursachen und der kleinste Test — regelbasiert, ohne KI-Schicht. Danach: Release R2, die erste Fassung, die wirklich nützt (passiv, 9 von 12 Kernfunktionen).*

- □ **S20–22** `SONDE-013` — Dynamik, Stereo, vor/nach der Kette, Passagen — und der manuelle Experimentkern. (offen)
- □ **G4** `Gate` — C++-Review (DSP) + Codex. (offen)
- □ **S23–25** `SONDE-014` — Absicht, Ursachenhypothese, Vorschlag, Assistentenschritt — mit Prüfkorpus. (offen)
- □ **G5** `Gate` — Codex + Gegenbeispiele: der Prüfer soll eine falsche starke Ursachenbehauptung provozieren. (offen)

### P6–P7 — Aktiver Kern: EQ in Sonde und Gen  (0/6 abgenommen)

*Der EQ entsteht: er rechnet in der Sonde (und als Master-EQ in Gen), bedient wird alles zentral auf Gens zweiter Seite — mit Kopplung, Sicherheit und Rückweg (Entscheid 23.08., Umschnitt gefahren; Spielregeln der UI: design/docs/ui-spielregeln-eq-zentrale.md). Danach: Release R3.*

- □ **S26–28** `SONDE-015` — Lokaler EQ-Kern in der Sonde: vier vorbereitete Bänke, Zustand und Automation, A/B — dazu die versionierte Zustands-Erweiterung aus dem 23.08.-Entscheid: Mix je Sonde und die Schutz-Zonen bekommen ihren Platz im gespeicherten Zustand (Layout v2, neue Kennungen, nie umgewidmet). (offen)
- □ **S28b** `SONDE-015b` — Derselbe EQ-Kern auf dem Master: Gen bekommt seinen eigenen EQ-Pfad — ausgeschaltet beweisbar bitidentisch, das NullTest-Bein wächst mit. Die Stilllegung des Suna-Ziels ist am 28.08.2026 nach S9b (SONDE-007c) vorgezogen und gehört nicht mehr hierher. (offen)
- □ **G6** `Gate` — Härtestes Gate des Plans: C++-Review auf höchster Stufe, Nebenläufigkeits-Prüfung, Worst-Case-CPU — deckt seit dem Umschnitt auch Gens Master-Pfad. (offen)
- □ **S29–31** `SONDE-016/017` — Kopplung Gen↔Sonde als Hauptbedienweg: Pairing, Verschlüsselung, Lease, Anwenden/Zurücknehmen, aktiver Vergleich — dazu die Drei-Stufen-Geste (Halten = hören · 1. Klick = 10-Sekunden-Probe · 2. Klick = fest), volle Bereiche ohne Remote-Limit (Entscheid 23.08.), die harte Schutz-Zonen-Sperre und der zweistufige Mix (ganze App ↔ gewählte Spur). (offen)
- □ **G7** `Gate` — Sicherheits- und Rust-Review + Codex + 10 000 Befehle Stress. (offen)
- □ **S31b** `SONDE-020` — EQ-Zentrale-UI: Gen Seite 2 gegen deinen Figma-Stand gebaut (Prüfmaßstab: design/docs/ui-spielregeln-eq-zentrale.md), dazu die Minimal-Rückfallfläche der Sonde. Startet erst, wenn dein Figma-Stand vorliegt (NAK-65). (offen)

### P8–P9 — Entmaskierung und Härtung  (0/4 abgenommen)

*Sidechain-Entmaskierung (nur wenn Termin A und Gate G0 grün sind — Termin A ist grün, G0 steht aus) und alles für die Auslieferung: Verteilung, Migration, Dauerlauf, Privatsphäre, Rollback. Danach: Release R4, der volle Sondenkern.*

- □ **S32–33** `SONDE-018` — Sidechain-Entmaskierung. (offen)
- □ **G8** `Gate` — C++-Review + Codex + Hör-/Stem-Korpus. (offen)
- □ **S34–35** `SONDE-019` — Verteilung, Migration, Dauerlauf, Privatsphäre, Rollback. (offen)
- □ **G9** `Gate` — Alle acht harten Gates + pluginval Stufe 8 + 30 Minuten mit 32 Sonden. (offen)

---

**■ abgenommen** — Beweismanifest liegt UND ein Prüfer der geforderten Stufe hat **PASS** gegeben.
**▣ gebaut** — Manifest liegt, Prüfungen sind gefahren, aber es gibt kein PASS: der Prüfer steht aus oder hat NEEDS_WORK gesagt. Zählt nicht als fertig.
**□ offen** — noch kein Beleg.

Quelle des Textes: `docs/plan/plan.json` · Quelle des Status: die Urteilsmarken in `docs/beweise/` · Fragen an dich: `docs/plan/fragen.json`.
