# Beweismanifest — PR1 «Planprüfung: Entscheide gegen Plan, Nakama gegen Marktstandard»

> **Ohne Urteilsmarke.** Die Marke setzt der Dirigent nach der Codex-Prüfung; bis dahin
> führt der gerechnete Planstand PR1 bewusst als *gebaut, nicht abgenommen*.

| Feld | Wert |
|---|---|
| Ticket | `PR1` (Planprüfung, kein Bauticket) |
| Phase / Session | P2, zwischen S14–15 (`SONDE-010`) und S16–17 (`SONDE-011`) |
| Gate-Text (Quelle) | `docs/plan/plan.json`, Schritt `PR1` — wörtlich unten |
| Basis-SHA | `6f40eed5e5e3969df2ac08232011247ff377e89c` (Branch `master`) |
| Datum | 2026-08-30 |
| Prüfstufen | T1 ☑ (Selbstaudit) · T2 ☐ · T3 ☐ — die geforderte Stufe ist **T3** |
| Kanon | **läuft nicht** — kein Produktcode, kein Test, kein Schema angefasst (Begründung §8) |

---

## 1. Kopf

### 1.1 Gate-Text, wörtlich

> Zwischenhalt der Implementierung. User-Wort 29.08.2026: „ich möchte dass du
> einen guten zeitpunkt im plan raussuchst, an dem wir dne implement temporär
> auf halt machen und eine ausführliche überprüfung des Gesamtplans machen. die
> muss besonders darauf abzielen ob änderungen die wir im verlauf beschlossen
> haben nicht an einer anderen stell iwas brechen. zudem müssen wir sichergehen
> keine standard features vergessen zu haben, die vergleichbare plugins von
> anderen herstellern haben. mögliche anpassungen müssen dann direkt sauber in
> den plan eingearbeitet werden" Zeitpunkt (Wahl des Dirigenten, 29.08.): nach
> dem PASS von G1 und den dritten Nacharbeitsrunden von S8, S9, S9b und S14–15,
> VOR S16–17 — weil SONDE-011 mit Koordinator, Datenbank-Migration und
> Ausgangspuffer die Datenformen festschreibt, die eine nachgeholte
> Standardfunktion am ehesten ändern würde, und G2 den Vertrag danach als R0
> einfriert. Inhalt: (1) jeden Entscheid seit Projektstart (design/abnahmen/,
> beantwortete Fragen in docs/plan/fragen.json, Nachträge in
> docs/offene-punkte.md, Wahrheitskern in CLAUDE.md) gegen Entwurf,
> Bauaufteilung, Verträge (v3, State, Installer) und die gebauten Tickets auf
> Widersprüche und Folgebrüche prüfen — jeder Treffer mit Fundstelle; (2)
> Funktionsvergleich mit vergleichbaren Produkten anderer Hersteller
> (Mix-Analyse mit Sonden, Spektrum-/Maskierungs-Werkzeuge, EQ-Fernsteuerung)
> anhand aktueller Herstellerdokumentation: Standardfunktionen, die Nakama
> fehlen oder anders löst, benennen und je Punkt sagen, ob Vertrag, State oder
> Oberfläche betroffen wären; (3) jede Anpassung sauber einarbeiten — plan.json,
> bauaufteilung-sonden.md, Entwurf, betroffene Ticketquellen; Produktentscheide
> gehen als Fragenkarten an den User, Technik entscheidet der Dirigent. Prüfung:
> frischer Codex-Thread über Plan und Änderungssatz, Recherche mit Belegen;
> Ergebnis in docs/beweise/PR1.md mit Urteilsmarke. Bis zu diesem Punkt läuft
> die Implementierung weiter.

### 1.2 Zeitpunkt-Revision und G1-Vorbehalt

**Revision (Dirigent, 30.08.2026, Sitzung `bf17b676`):** PR1 läuft **vor** dem
G1-Gate-Lauf, nicht danach. Begründung: G1 ist ein Phasengate und wartet auf
einen User-Handgriff; der Plan stünde sonst still (User-Wort 30.08.:
„voranschreiten ist das wichtigste"). Die vier anderen Vorbedingungen des
Gate-Textes sind erfüllt: S8, S9, S9b und S14–15 tragen jeweils eine
PASS-Urteilsmarke (`docs/PLAN-STAND.md`, Stand 2026-08-30).

**Vorbehalt, ausdrücklich:** Ändert der spätere G1-Lauf einen Vertrag — v3,
State, Installer oder Identität —, bekommt dieses Manifest einen datierten
Nachtrag; die Befunde und der Änderungssatz unten gelten dann nur bis zu diesem
Nachtrag. G1 steht derzeit auf `T3 NEEDS_WORK 2026-08-24 offen`
(`docs/beweise/G1.md`), sein offener Befund ist NAK-74/NAK-79 (Aussageklasse
`post_fader_contribution`); genau dieser Bereich ist unten in Befund D-3
mitgeführt.

### 1.3 Kurzbilanz — gezählt, nicht geschätzt

| Was | Anzahl | Quelle der Zählung |
|---|---:|---|
| Entscheidquellen `design/abnahmen/*.md` (ohne `LIES-MICH.md`) | **40** | `ls design/abnahmen/*.md \| grep -v LIES-MICH \| wc -l` |
| Beantwortete Fragenkarten in `docs/plan/fragen.json` (`beantwortet{}`) | **42** | `len(d["beantwortet"])` |
| Offene Fragenkarten vor diesem Ticket (`offen[]`) | **2** | `len(d["offen"])` |
| Registerzeilen `docs/offene-punkte.md` (`NAK-*`, alle drei Tabellen) | **90** | `grep -c '^\| NAK-'` |
| Höchste vergebene Registernummer vor diesem Ticket | **NAK-104** | `grep -o 'NAK-[0-9]+' \| sort -n \| tail -1` |
| Planschritte in `docs/plan/plan.json` | **38** in **8** Phasen | `sum(len(p["schritte"]))` |
| Geprüfte Herstellerprodukte mit abgerufener Herstellerdoku | **9** von 9 Herstellern (Pro-Q 4, Neutron 5 / Ozone, smart:EQ 4, MMultiAnalyzer, SPAN Plus, Trackspacer, Smooth Operator Pro, Gullfoss, TDR Nova) — unverändert nach Runde 2: H15 ist eine weitere Seite eines schon geführten Herstellers | §5, Quellenliste §2.2 |
| Einzeln abgerufene Herstellerseiten (H1–H15) | **15** | `grep -c '^\| H[0-9]' docs/beweise/PR1.md` |
| **Befunde: Defekt im Plan** | **6** | §4 |
| **Befunde: Lücke** | **4** | §4 |
| **Befunde: Härtung** | **3** | §4 |
| Neue Fragenkarten an den User (U15–U20) | **6** | §6.2 |
| Neue Registerzeilen (NAK-105 bis **NAK-111**) | **7** | §6.1 und §9 |
| Direkte Edits an Planquellen (E-1 bis E-17, dazu R1-1 bis R1-11 aus Runde 1 und R2-1 aus Runde 2) | **17 + 11 + 1** | §6.1, §9.3 und §10.3 |
| Vorschläge an den Dirigenten | **3**, davon **1 entschieden** (V-1, 30.08. Runde 1) | §6.3 |
| Registerzeilen `docs/offene-punkte.md` nach Runde 1 | **97**, höchste **NAK-111** | `grep -c '^\| NAK-'` |
| Offene Fragenkarten nach Runde 1 | **8** (U11, U13, U15–U20; U17 trägt fünf getrennt beantwortbare Einzelentscheide U17.1–U17.5) | `len(d["offen"])` |

### 1.4 Runden dieses Tickets

| Runde | Was | Prüfer / Stufe | Urteil | Offen danach |
|---|---|---|---|---|
| **0** | Erstdurchgang: Entscheidregister, Funktionsvergleich, Änderungssatz E-1 bis E-17, Karten U15–U20, Register NAK-105 bis NAK-109 (§1–§8) | Codex `gpt-5.6-sol`, Effort **xhigh**, lesend, Thread `01a053a7-d1dc-7441-8554-44d0fa14cc8f` (30.08.2026) | **NEEDS_WORK** | **7** Befunde, alle [P1] |
| **1** | Nacharbeit: genau diese sieben Befunde bearbeitet — Planedits, Register, Karten, zwei nachgereichte Herstellerquellen, Einordnung Visual Mixer (§9) | Worker, Selbstaudit (T1); der Dirigent hat alle sieben vorab an der Quelle als **Defekt** bestätigt | **NEEDS_WORK** — durch die Wiederprüfung in der Zeile darunter | **1** der sieben: **B6** blieb offen, sechs sind geschlossen |
| **1-P** | Wiederprüfung der Runde 1, Prüfbereich `git diff 0e3908e...b54a575 -- <Ticketpfade>`; `beantwortet{}` unverändert, beide Plan-JSON laden | Codex `gpt-5.6-sol`, Effort **xhigh**, lesend, Thread `01a053cf-9992-7910-b526-a7712881d9a3` (30.08.2026) | **NEEDS_WORK** | **1** Restbefund [P1] — B6, `docs/plan/fragen.json:72` |
| **2** | Nacharbeit: nur dieser eine Restbefund — U18 auf das Belegte eingegrenzt, Herstellerquelle **H15** nachgereicht, §5.3 mit datiertem Zusatz (§10). Keine andere Karte, kein anderer Plansatz angefasst | Worker, Selbstaudit (T1); der Dirigent hat den Restbefund vorab an der Quelle als **Defekt** bestätigt | offen — das T3-Urteil setzt der Dirigent | **0** |

Der lebende Kopf endet hier. §2 bis §8 sind der Verlauf des Erstdurchgangs und werden nicht umgeschrieben; Korrekturen daran stehen als datierte Nachträge an Ort und Stelle. §9 und §10 sind die append-only Rundenabschnitte.

---

## 2. Quellenverzeichnis

### 2.1 Entscheid- und Vertragsquellen im Repo (alle auf Basis `6f40eed`)

| Quelle | Was sie bindet | Stand |
|---|---|---|
| `CLAUDE.md` | Wahrheitskern, Invarianten, Rangfolge der Quellen | laufend |
| `design/abnahmen/*.md` (40 Dateien) | User-Wortlaut mit Datum = Entscheid | 20.08.–29.08.2026 |
| `docs/plan/fragen.json` `beantwortet{}` (42) | je Karte Wahl, Wortlaut, Datum, Folge | 22.08.–29.08.2026 |
| `docs/plan/fragen.json` `offen[]` (2: U11, U13) | offene User-Handgriffe | 22.08./23.08.2026 |
| `docs/offene-punkte.md` (90 `NAK-*`) | Befundregister, nicht Entscheidquelle | bis NAK-104, 30.08.2026 |
| `docs/FL-Nakama-Sonden-Design-Entwurf.md` | Entwurf 0.4 + Errata-Block (a)–(p), Block hat Vorrang | 20.08., Errata bis 27.08. |
| `docs/bauaufteilung-sonden.md` | Sessions, Gates, Prüfebenen | 21.08. mit Nachträgen bis 28.08. |
| `docs/plan/plan.json` / `docs/plan/LIES-MICH.md` | Schritttexte; Status wird gerechnet, nicht gepflegt | 30.08.2026 |
| `eq-copilot/schemas/v3/README.md`, `eq-ipc-v3.schema.json`, `reservierte-nachrichten-v1.json` | v3-Nachrichtenvertrag, 17 definiert + 8 reserviert = 25 | S5 |
| `eq-copilot/schemas/state/nakama-state-v2.md`, `nakama-parameter-v1.json` | State-Schema 2, Kind-Matrix, 109 Parameter-IDs | S7, geändert 24.08. |
| `eq-copilot/identity/plugin-identities-v1.json`, `host-capabilities-fl-v1.json` | eingefrorene Identität; zehn Capability-Bits, 2 supported | S2/S4, S9b |
| `eq-copilot/install/nakama-installer-v1.json` | Auslieferung und Rückweg | S9/S9b |
| `docs/beweise/S0-basislinie.md`, `SONDE-001-002 … SONDE-010.md`, `G0.md`, `G1.md` | lebende Köpfe und Urteilsmarken der gebauten Tickets | bis 30.08.2026 |
| `docs/beweise/termin-a/`, `termin-b/` | Rohmessungen im echten FL | 21.08./22.08.2026 |
| `design/docs/ui-spielregeln-eq-zentrale.md` | technischer Prüfmaßstab der zwei Gen-Oberflächen | 23.08., präzisiert 27.08. |
| `design/assets/rework-basis-2026-08-25/` (3 Bilder) | aktuelle visuelle Wahrheit in progress | 25.08.2026 |
| `tools/dirigent/pruefliste.md` | Prüfliste A–F für Worker und Prüfer | 23.–30.08.2026 |

### 2.2 Herstellerquellen für den Funktionsvergleich

**Alle zwölf Seiten sind am 2026-08-30 einzeln abgerufen worden**, nicht nur aus einer
Trefferliste zitiert; H11 liegt als PDF vor und ist mit `pdftotext` in Text gewandelt
worden. Nur Herstellerseiten, Herstellerhandbücher und die Formatspezifikation;
Händler-, Test- und Forenseiten sind **nicht** als Beleg verwendet.

| # | Hersteller / Produkt | URL | Wofür belegt |
|---|---|---|---|
| H1 | FabFilter Pro-Q 4 — Instance List | https://www.fabfilter.com/help/pro-q/using/instance-list | Fernbedienung fremder Instanzen, Reihenfolge = DAW-Spurreihenfolge, Kollisionsreferenz, EQ Match |
| H2 | FabFilter Pro-Q 4 — Spectrum analyzer | https://www.fabfilter.com/help/pro-q/using/analyzer | Pre/Post/External-Spektrum, Show Collisions, Freeze, Spectrum Grab, Range/Resolution/Speed/Tilt, Piano-Anzeige |
| H3 | FabFilter Pro-Q 4 — Overview | https://www.fabfilter.com/help/pro-q/using/overview | Phasenmodi (Zero-Latency / Natural Phase / Linear Phase), Band-Solo, Auto-Gain, Preset-Browser, Undo/Redo/A/B/Copy, MIDI Learn, Full Screen, Character-Modi |
| H4 | FabFilter Pro-Q 4 — Output (Auto Gain) | https://www.fabfilter.com/help/pro-q/using/output | Auto Gain ist „an educated guess based on the current EQ settings, and is **not** a dynamic process based on actually measured levels" |
| H5 | iZotope Neutron 5 — Features | https://www.izotope.com/en/products/neutron/features.html | Masking Meter, Visual Mixer, Relay (Inter-Plugin Communication), Unmask, Mix Assistant, Delta-Buttons |
| H6 | sonible smart:EQ 4 | https://www.sonible.com/smarteq4/ | Group View bis 10 Instanzen, Fernbedienung jeder Gruppeninstanz, smart:filter Track/Group/Track+Group, Prioritätsreihenfolge, Profile aus Referenztrack |
| H7 | MeldaProduction MMultiAnalyzer | https://www.meldaproduction.com/MMultiAnalyzer | Instanzen finden sich automatisch und tauschen Analysen; Kollisionsdetektor, Multitrack-Sonogramm, Lautheitsvergleich, Stereoanalyse |
| H8 | Voxengo SPAN Plus — Features | https://www.voxengo.com/product/spanplus/features/ | Spektrum-Export an andere Instanzen, Kanalgruppen, Korrelation, EBU R128 LUFS, Preset-Manager, Undo/Redo, A/B |
| H9 | Wavesfactory Trackspacer | https://www.wavesfactory.com/audio-plugins/trackspacer/ | 32-Band-Sidechain-Entmaskierung, L/R und M/S, Attack/Release, Low-/High-Cut des Wirkbereichs |
| H10 | Baby Audio Smooth Operator Pro | https://babyaud.io/smooth-operator-plugin | Spektrale Resonanzdämpfung, FOCUS (Detail/Isolation), Knoten mit eigenem Dynamikprofil, SC-Sektion — die vom User genannte zweite Referenz |
| H11 | Soundtheory Gullfoss — Operation Manual (PDF, 21 Seiten) | https://www.soundtheory.com/static/Gullfoss%20Operation%20Manual.pdf | Recover/Tame/Bias/Brighten/Boost als vollautomatischer, wahrnehmungsmodellbasierter EQ; Kurve „updated internally more than 300 times per second"; Lautheitskompensation eingebaut („preserves the original dynamics and perceived loudness … without being mislead by a slightly different perceived loudness"); Frequenzbereichs-Begrenzer im Graph; Sidechain-Eingang seit 1.9.0 |
| H13 | Tokyo Dawn Records TDR Nova | https://www.tokyodawn.net/tdr-nova/ | „With its four dynamic EQ bands and additional high-pass and low-pass filter sections, NOVA can meet the most exotic demand"; „each band also includes a full featured dynamics section" — Beleg für die Zuordnung „dynamische Bänder" in §5.3 (abgerufen 30.08.2026, Runde 1) |
| H14 | iZotope Ozone — Features (Match EQ) | https://www.izotope.com/en/products/ozone/features.html | „Match to any reference track with over 8,000 separate bands to get the most precise snapshot possible" — Beleg für die Zuordnung „iZotope hat EQ-Matching" in §5.4 und Karte U19 (abgerufen 30.08.2026, Runde 1) |
| H15 | FabFilter Pro-Q 4 — Undo, redo, A/B switch | https://www.fabfilter.com/help/pro-q/using/undoredo | „The Copy button copies the active state to the inactive state. This marks the current state of the plug-in and allows you to go back to it easily with the A/B button." — Pro-Q 4s „Copy" ist damit die A/B-Zustandskopie **innerhalb derselben Instanz**, kein Übertragen in eine andere. Grundlage der Eingrenzung von Karte U18 und der §5.3-Zeile „Undo/Redo, A/B, Kopieren" (abgerufen 30.08.2026, Runde 2) |
| H12 | Steinberg VST 3 — `ChannelContext::IInfoListener` | https://steinbergmedia.github.io/vst3_doc/vstinterfaces/classSteinberg_1_1Vst_1_1ChannelContext_1_1IInfoListener.html | Der Weg, über den ein Host Spurname, Farbe und Index an ein Plugin gibt — Grundlage von Befund D-2 |

**Nachtrag 30.08.2026 (Runde 1, Codex-Befund B6).** Der Satz oben („alle zwölf Seiten") gilt für den ersten Durchgang. In Runde 1 sind **zwei weitere Herstellerseiten am 2026-08-30 einzeln abgerufen** worden — **H13** (TDR Nova) und **H14** (iZotope Ozone) —, weil vier Marktzuordnungen über die damalige Quellenliste hinausreichten. Stand jetzt: **vierzehn** Quellen, davon zwölf aus dem ersten Durchgang. Zwei Zuordnungen sind dabei nicht belegt worden, sondern **gestrichen** (§5.1, Zeile Instanzliste). Für H13 und H14 gilt dieselbe Grenze wie unten für H1–H11.

**Nachtrag 30.08.2026 (Runde 2, Restbefund zu B6).** Eine **weitere Herstellerseite ist am 2026-08-30 einzeln abgerufen** worden — **H15**, die FabFilter-Hilfeseite „Undo, redo, A/B switch" —, weil die Zuordnung „Kopieren zwischen Instanzen" aus dem blossen Wort „Copy" in H3 abgeleitet worden war und H3 nicht sagt, was diese Taste tut. H15 sagt es: sie kopiert den aktiven A/B-Zustand auf den inaktiven, innerhalb derselben Instanz. Die Zuordnung ist damit nicht belegt, sondern **widerlegt**; Karte U18 und die §5.3-Zeile sind entsprechend eingegrenzt (§10). Stand jetzt: **fünfzehn** Quellen — zwölf aus dem Erstdurchgang, zwei aus Runde 1, eine aus Runde 2. Für H15 gilt dieselbe Grenze wie unten für H1–H11.

**Ehrliche Grenze der Recherche:** H1–H11 belegen, **dass** eine Funktion
existiert und wie der Hersteller sie beschreibt. Sie belegen nicht, wie sie
intern gebaut ist. Für H12 gilt: die Schnittstelle ist dokumentiert, ob **FL
Studio** sie bedient, ist weder vom Hersteller dokumentiert noch in diesem
Projekt je gemessen worden (siehe D-2).

---

## 3. Entscheidregister — jeder Entscheid gegen Plan, Vertrag und gebautes Ticket

Lesart der Spalte **Ergebnis**: `stimmig` = im geprüften Plan-, Vertrags- oder
Ticketstand wiedergefunden · `Widerspruch` = zwei Quellen sagen Verschiedenes ·
`Folgebruch` = der Entscheid ist widerspruchsfrei formuliert, aber eine andere,
bereits gebaute oder festgeschriebene Stelle kann ihn nicht tragen · `Verlauf` =
durch einen späteren Entscheid überholt, korrekt als Verlauf geführt.

### 3.1 `design/abnahmen/` — 40 Dateien

| # | Datum | Entscheid (Kurzform des User-Wortlauts) | Wogegen geprüft | Ergebnis |
|---:|---|---|---|---|
| 1 | 20.08. | `bedienebenen-beides` — „es ist 1 und 2 eine kombination daraus" | Entwurf §5 (vier Bedienebenen), §19 | stimmig |
| 2 | 20.08. | `fassung-lichtleiter` — dunkle Fassung mit leuchtendem Strich | Errata (c): Glas/Licht geparkt; Rework 25.08. | Verlauf (bewusst) |
| 3 | 20.08. | `groesse-main` — „die beiden kleinsten … oder freie skalierung" | überholt durch `groessen-alle-drei` und 24.08./25.08. | Verlauf |
| 4 | 20.08. | `groessen-alle-drei` — „so passt es 3 größen" (Gen 760×430) | `ui-spielregeln-eq-zentrale.md:30`; NAK-82 (Editor trägt anderes Verhältnis) | Widerspruch → **H-1** |
| 5 | 20.08. | `karte-alle-quellen` — Regelfall einstellig, 16 = Obergrenze | Entwurf Errata (f), §27/§30/§57; v3-Vertrag Obergrenze 32 | stimmig |
| 6 | 20.08. | `korrektur-formfaktor-zuerst` — keine tote Fläche | CLAUDE.md „Keine toten UI-Elemente" | stimmig |
| 7 | 20.08. | `korrektur-reihenfolge-und-lichtmodell` — „nie entweder oder" | Errata (c), Prisma geparkt | Verlauf |
| 8 | 20.08. | `mechanik-main-overview-detail` — Overview + Detail auf 760×430 | Rework 25.08. (drei Bilder) | Verlauf |
| 9 | 20.08. | `methode-visuelle-befragung` — „du musst mich visuell befragen" | Skill `/fragen`, `docs/plan/bilder/` | stimmig |
| 10 | 20.08. | `sessionende` — „glas und licht ist nicht entschieden" | Errata (c) | stimmig |
| 11 | 20.08. | `vorhoeren-markierte-zeile` — markierte Zeile + App-Umrandung | überholt durch U2.3 (22.08.) und Rework 25.08. | Verlauf |
| 12 | 20.08. | `welt-glas-und-licht` | Errata (c), geparkt | Verlauf |
| 13 | 20.08. | `zustaende-nur-ausnahme` — Zustände nur als Ausnahme zeigen | CLAUDE.md „ehrlich einen Zustand melden"; Entwurf §57 | stimmig |
| 14 | 21.08. | `codereview` — Prüfbericht, **kein** User-Urteil | — | kein Entscheid (korrekt so gekennzeichnet) |
| 15 | 21.08. | `codex-review-abgearbeitet` — Befundabarbeitung | — | kein Entscheid |
| 16 | 21.08. | `codex-review-volltext` — Volltext | — | kein Entscheid |
| 17 | 21.08. | `designvertrag` — drei Apps, eine Identität, Figma ist Quelle | Errata (b)/(h); überholt in Teilen durch 25.08. und 28.08. (zwei Apps) | Verlauf, sauber nachgezogen |
| 18 | 21.08. | `gen-auftrag-figma` | Rework 25.08. | Verlauf |
| 19 | 21.08. | `offen-codex-review` | — | kein Entscheid |
| 20 | 21.08. | `sessionende` | — | kein Entscheid |
| 21 | 21.08. | `suna-auftrag-figma` | Suna-Stilllegung 28.08. | Verlauf |
| 22 | 22.08. | `erfolgskriterium-exakt-wie-figma` — „in fl Studio … exakt aus wie in figma" | Rework 25.08. setzt neue Bildbasis; Kriterium selbst gilt weiter | stimmig |
| 23 | 22.08. | `figma-depot` — gemeinsames Depot | 24.08. bestätigt | stimmig |
| 24 | 22.08. | `hub-antworten-35` — 35 Antworten, u. a. „figma ist keine technikreferenz, der implementplan ist es" | Errata (h); U2/U6/U9 am 29.08. geschlossen | stimmig |
| 25 | **23.08.** | **`gen-eq-zentrale`** — Merge auf eine Sonde · Name Probeeq · **Master-EQ in Gen** · Seite-2-Durchschalter · Ein-Graph-Overlay | Errata (n); plan.json S28b/S29–31/S31b; **`nakama-state-v2.md:38` Kind-Matrix** und **`nakama-parameter-v1.json:9`** | **Folgebruch → D-1** |
| 26 | **23.08.** | **`interview-struktur`** — 21 Festlegungen (Drei-Stufen-Geste, Mix zweistufig, Schutz-Zonen hart, Live-Spektrum, **Durchschaltreihenfolge = FL-Mixer**, **Durchschalter zeigt Bus-Namen**, Undo je Sonde, GLOBAL einklappbar, ±3-dB-Limit entfällt) | plan.json S26–28/S29–31/S31b; `ui-spielregeln`; **v3 `probe_descriptor`**, `host-capabilities-fl-v1.json` | teils stimmig, **Folgebruch → D-2** |
| 27 | 23.08. | `offen-prototyp-umbau` — Vorschlag, nichts abgenommen | — | kein Entscheid (korrekt gekennzeichnet) |
| 28 | **24.08.** | **`auto-gain-schalter`** — Schalter AUTO, Standard aus; „gehört in S26–28" | **plan.json:280 (S26–28)** und **`bauaufteilung-sonden.md` §3 P6-Zeile** | **Widerspruch → D-3** |
| 29 | 24.08. | `figma-depot-immer-aktuell` | Rework 25.08. | Verlauf |
| 30 | 24.08. | `offene-figma-entscheide` — U2.8 Revisionsverlauf · U5 Sidechain unsichtbar | `ui-spielregeln:46` (Sidechain unsichtbar); Rework-Bild 1 zeigt Revision History | stimmig |
| 31 | **24.08.** | **`seite2-groesse-keine-toten-elemente`** — beide Seiten gleich groß (760×430); „die schlimmste ui/ux sünde sind sinnlose tote elemente" | `ui-spielregeln:30`; CLAUDE.md; NAK-82 | stimmig; Größenkonflikt siehe H-1 |
| 32 | 25.08. | `gen-aufwertung-varianten-duerfen-testen` — „Ja er kann kreativ sein und testen" | Rework-Leseregel 25.08. | stimmig |
| 33 | 25.08. | `offen-bausteine` — offene Farb-/Bausteinfragen | nicht im Plan geführt | siehe **L-4** |
| 34 | 25.08. | `openwiki-entfernt` — „entferne das komplette openwiki konstrukt" | NAK-45/NAK-67 | stimmig |
| 35 | **25.08.** | **`rework-referenz-drei-designs`** — „das ist ab jetzt die aktuelle wahrheit in progress" | CLAUDE.md Wahrheitskern; plan.json S31b (verweist auf **NAK-65 Figma-Stand**, nicht auf die drei Bilder) | **Widerspruch → D-4** |
| 36 | 25.08. | `seitenverhaeltnis-bleibt-fest` — „lassen wir es erstmal dabei" | `ui-spielregeln:30`; U2.17 (100/125/150/200 %) | stimmig |
| 37 | 25.08. | `skizzen-vor-figma` — Skizzenphase abgeschlossen | CLAUDE.md Design-Arbeitsmodell Punkt 2 | stimmig |
| 38 | **27.08.** | **`arbeitsnamen-und-probeeq-doppelrolle`** — Overview/EQ-Zentrale sind Arbeitsnamen; Probeeq = Messsonde **und** Fern-EQ-Ausführer; kein lokaler Voll-Editor | CLAUDE.md; Errata (p); **v3 `plugin_kind`** und **Kind-Matrix führen weiter `passive_probe` und `active_probe` als getrennte Klassen** | **Folgebruch → D-5** |
| 39 | **28.08.** | **`suna-stilllegung-vorgezogen`** — „moment. suna gibt es nichtmehr"; „ja stell das einfach richtig" | plan.json S9b ✔ · Entwurf §55-Nachtrag ✔ · `bauaufteilung` §3 ✔ / **§4 Summe nicht nachgezogen** | **Widerspruch → D-6** |
| 40 | 29.08. | `fragenrunde-ueberholte-karten` — U2/U6/U9/U12 geschlossen, U11 vertagt | `fragen.json` ✔ | stimmig |

### 3.2 `docs/plan/fragen.json` — 42 beantwortete Karten

Geprüft wurde bei jeder Karte, ob das Feld `ergebnis` im heutigen Plan-, Vertrags-
oder Designstand wiederauffindbar ist.

| Karten | Ergebnis der Prüfung |
|---|---|
| **U2.0–U2.17, U6.1–U6.10, U9.1–U9.5** (33 Karten, 22.08.) | Alle betreffen die alten Figma-Exporte. Am **29.08.** hat der User U2, U6 und U9 als Sammelkarten ausdrücklich geschlossen („Ja, schließen"), weil der Rework-Entscheid vom 25.08. sie überholt. **stimmig** — kein Fund. Die vier Unterkarten mit offener Folgefrage (U2.1, U2.4, U2.6, U2.9) sind damit gegenstandslos und werden beim Rework aus den drei Bildern gelesen. |
| **U5** (24.08., Sidechain unsichtbar) | `ui-spielregeln-eq-zentrale.md:46` führt „im ersten Release bleibt die Sidechain-Quelle unsichtbar, Default `none`"; `nakama-parameter-v1.json` hält `sidechain_source` mit Default `none`. **stimmig** |
| **U2.8** (24.08., kleines Revisionssymbol) | Rework-Bild Gen Seite 1 zeigt Revision History als Overlay. **stimmig** |
| **U7** (24.08., alte Plugin-Kopie löschen) | vertagt, Löschbefehl liegt in `docs/NEXT-SESSION.md`. **stimmig** |
| **U8** (29.08., Installation und FL-Termin) | in `docs/beweise/SONDE-007b.md`, Abschnitt „FL-Termin — 2026-08-29" belegt; NAK-87 geschlossen. **stimmig** |
| **U10** (29.08., Hör-Markierung nur mit Signal) | NAK-35 geschlossen in `SONDE-008`. **stimmig** |
| **U12** (29.08., Nakama-Design-Repo archivieren) | ausgeführt. **stimmig** |
| **U14** (23.08., ±3-dB-Remote-Limit entfällt) | Entwurf Erratum (n) ✔, `ui-spielregeln` Regel 6 ✔, plan.json S29–31 „volle Bereiche ohne Remote-Limit" ✔. **Aber:** Entwurf §44.2 (`:2660`) trägt den Satz „Remote-Proposals bleiben auf ±3 dB und zunächst Q 0,4–2 begrenzt" **unverändert weiter**; NAK-64 hatte seine Streichung im Umschnitt zugesagt und wurde geschlossen, ohne sie auszuführen. **Widerspruch → D-3 (b)** |
| **U2.16, U2.17** (22.08., Depot, Skalierungsstufen) | `ui-spielregeln`, Designvertrag. **stimmig** |

### 3.3 `docs/offene-punkte.md` — 90 Registerzeilen

Das Register ist **Befundquelle, kein Entscheid** (`CLAUDE.md`). Geprüft wurde,
ob ein offener Registerpunkt einen Entscheid trägt, der im Plan fehlt.

| Zeile | Prüfergebnis |
|---|---|
| **NAK-81 (c)** (24.08.) — „Der Master-EQ in Gen hat keinen Parameter-Ort für seine Trims … Gehört zu **S28b**" | Der Befund steht seit dem 24.08. im Register, **aber in keiner Planquelle**: `plan.json:288` (S28b) und die S28b-Zeile in `bauaufteilung-sonden.md` §3 nennen ihn nicht. Zusätzlich greift NAK-81 (c) zu kurz — es zitiert nur `nakama-parameter-v1.json:9`, nicht die **Kind-Matrix** `nakama-state-v2.md:38`, die `main` sowohl `Parameters` als auch `Dsp` **verbietet**. → **D-1** |
| **NAK-79** (24.08.) — welche Klasse darf `post_fader_contribution` führen? | offen, Eigentümer `SONDE-011`; plan.json S16–17 nennt die Frage nicht. → **L-1** |
| **NAK-65** (23.08.) — Figma-Flächen für Gen Seite 2 | plan.json S31b macht den Start davon abhängig; die Bildbasis ist seit 25.08. eine andere. → **D-4** |
| **NAK-39** (21.08.) — „12 von 20 Kernfunktionen" nicht rekonstruierbar | unverändert offen; §0.2/§22 tragen die Zahl weiter. → **H-2** |
| **NAK-13** (18.08.) — Broker-Autostart fehlt | keine Planzeile besitzt ihn; `darfBrokerStarten()` existiert seit S9 als Haken. → **L-2** |
| **NAK-10** (17.08.) — Broker liest `hoermarkierung` nicht | Entwurf §56 führt es im P2-Lieferumfang; plan.json S16–17 („Koordinator, Datenbank-Migration, Ausgangspuffer") nennt es nicht. → **D-6 (b)** |
| übrige 84 Zeilen | Befunde ohne Entscheidcharakter oder bereits geschlossen; kein Planfund. |

**Nachtrag 30.08.2026 (Runde 1, Codex-Befund B2) — die Sammelbewertung „übrige 84 Zeilen" war zu grob.** Fünf davon sind offene **Vertragsriegel**, die der nächste Datenweg verbraucht; ihr bisheriges Eigentümerticket ist abgeschlossen oder S16–17 nimmt die Daten als Nächstes. Ohne neue Zuordnung friert G2 sie als R0 ein. Gemessen mit `rg -n '^\| NAK-(10|28|29|40|59) ' docs/offene-punkte.md`:

| Zeile | Was offen ist | Bisheriger Eigentümer | Neu |
|---|---|---|---|
| **NAK-10** (17.08.) | Broker liest das `hoermarkierung`-Flag nicht; Entwurf §31.2/§56 führen es als offenen P2-Vertrag | „Zielphase P2", kein Ticket | **S16–17, v3-Vertragsrunde** |
| **NAK-28** (21.08.) | `subscribe_session` hat keinen Gegenpfad; die 25er-Liste aus §33.3 ist geschlossen, ein Nachtrag ist deshalb **nicht** additiv | offen seit SONDE-005a | **S16–17, v3-Vertragsrunde** |
| **NAK-29** (21.08.) | im **Erzeuger** seit SONDE-009 geschlossen, der **v3-Schema-Schnitt** (diskriminierte Union) ausdrücklich nicht — die Zeile nennt `SONDE-010` als Ort | `SONDE-010` (abgeschlossen) | **S16–17, v3-Vertragsrunde** |
| **NAK-40** (22.08.) | Abbildung einer nicht-hex32-`instance_id` auf die v3-Adresse. `SONDE-010` sollte es festlegen; gemessen: `grep -c 'NAK-40' docs/beweise/SONDE-010.md` = **0** — es ist nie entschieden worden | `SONDE-010` (abgeschlossen, ohne Entscheid) | **S16–17, v3-Vertragsrunde** |
| **NAK-59** (23.08.) | Band-Stereo hat keinen Platz im FlatBuffers-Frame; die Zeile nennt `SONDE-010` als Ort | `SONDE-010` (abgeschlossen) | **S16–17, v3-Vertragsrunde** |

Alle fünf tragen seit dem 30.08.2026 einen datierten Nachtrag im Register; eingearbeitet sind sie in `docs/plan/plan.json` S16–17 und in die S16–17-Zeile von `docs/bauaufteilung-sonden.md` §3. Der Vorschlag V-1 aus §6.3 ist damit **entschieden**.

### 3.4 `CLAUDE.md` Wahrheitskern

| Satz | Wogegen geprüft | Ergebnis |
|---|---|---|
| „Gen hat zwei Oberflächen … Probeeq misst passiv und beliefert Gen … dieselbe Instanz führt die von Gen ferngesteuerten Eingriffe aus" | v3 `plugin_kind`, State-Kind-Matrix | **Folgebruch → D-5** |
| „Grundgesetz: nichts Ungefragtes … ausgeschaltet bitidentisch" | Entwurf §44.2 Hard-Bypass, NullTest-Beine A15/A16 | stimmig |
| „Keine KI-Erklärschicht" | Errata (e), Gate 8 gegenstandslos, `bauaufteilung` §5 | stimmig |
| „Aktuelle visuelle Wahrheit: die drei Bilder vom 25.08." | plan.json S31b verweist auf NAK-65 | **Widerspruch → D-4** |
| „Keine toten UI-Elemente" | `ui-spielregeln`, Rework-Leseregel 5 | stimmig |
| „Identität bleibt eingefroren bis NAK-30" | `plugin-identities-v1.json`, `stillgelegt_bedeutet` | stimmig |
| „Beziehungen mitprüfen: speichern↔laden …" | Prüfliste F | stimmig |

---

## 4. Befunde

### 4.1 Defekt im Plan (6)

Ein Defekt ist ein Widerspruch oder Folgebruch, der ein späteres Ticket oder
einen Vertrag brechen würde.

---

#### D-1 — Gens Master-EQ hat im gebauten State-Vertrag keinen Platz: `main` darf weder `Parameters` noch `Dsp` führen

**Fundstelle.**
`eq-copilot/schemas/state/nakama-state-v2.md:38` (Kind-Matrix §2.1):

> `| main | Pflicht | **Pflicht** | verboten | verboten | ab SONDE-016 … |`

— Spaltenfolge `Common | MainProject | Parameters | Dsp | Pairing`. Dazu
`eq-copilot/schemas/state/nakama-parameter-v1.json:9`:

> „Der heutige Eqcp-Eintrag (Gen/Legacy) und die passive Sonde (Suna) tragen
> NULL Hostparameter; dieser Bestand gilt nur fuer plugin_kind = active_probe
> (§53.8)."

**Der Entscheid dagegen.** `design/abnahmen/2026-08-23-gen-eq-zentrale.md` §3,
User-Wortlaut: „es ist ein vollwertiger eq für den master wie andere eq auf dem
markt". Entwurf Erratum (n): „Gen erhält einen zuschaltbaren, minimalphasigen
Master-EQ". `docs/plan/plan.json:288` (S28b): „Derselbe EQ-Kern auf dem Master:
Gen bekommt seinen eigenen EQ-Pfad".

**Warum das bricht.** Der State-Vertrag sagt über sich selbst
(`nakama-state-v2.md:6`): „eine Änderung an Root-Major, Kind-Major oder
Kind-Matrix ist eine **Versionierung mit Beleg, kein Edit**." S7 ist mit
`T3 PASS 2026-08-28` abgenommen, das Bein B2 misst die Matrix, und
`NakamaState.cpp:652` setzt sie durch. S28b, wie heute im Plan formuliert, ist
damit **nicht baubar**, ohne vorher eine Vertragsversionierung zu fahren, die in
keinem Schritt steht. Betroffen sind drei Dinge zugleich: Hostautomation von
Gens EQ, `state_hash`/Recall des Master-EQ und der Undo-Ring (§44.3).

**Beleg, dass es nicht schon jemand aufgeschrieben hat.** `grep -c 'Kind-Matrix'
docs/offene-punkte.md` = **0**. NAK-81 (c) nennt nur die Parameterdatei und nur
die Trims.

**Was getan.** Edit E-1 (plan.json S28b), E-2 (Bauaufteilung §3), E-3 (Entwurf,
Nachtrag zu Erratum (n)); Registerzeile **NAK-105**. **Kein** Fragenkarte: die
Frage „welcher Ort im Zustandsvertrag" ist technisch und liegt beim Dirigenten
(CLAUDE.md Hard Rule). Die Auslegung als *additive* Änderung an einer
eingefrorenen Identität ist unbedenklich — Gen führt heute **null** Parameter,
es verschiebt sich also kein bestehender Index.

---

#### D-2 — „Durchschaltreihenfolge = FL-Mixer-Reihenfolge" und „Durchschalter zeigt den Bus-Namen" haben weder Datenquelle noch Vertragsfeld noch Messung

**Fundstelle des Entscheids.**
`design/abnahmen/2026-08-23-interview-struktur.md`, Festlegung 11 und 12:

> „mixer reihenfolge, master erhält ui element" · „Den Namen auf welchem Bus er
> sitzt mehr nicht"

Übernommen in `design/docs/ui-spielregeln-eq-zentrale.md:124` und `:183`/`:201`.

**Fundstelle des Bruchs — drei Messungen.**

1. **Der Vertrag kann es nicht ausdrücken.**
   `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:446` (`probe_descriptor_insert`)
   ist `additionalProperties: false` und führt genau: `adresse`, `plugin_kind`,
   `measurement_position`, `aussageklasse`, `label`, `pair_id`, `capabilities`,
   `frische`. Kein Spurname, kein Mixerindex. `probe_descriptor` sagt über sich
   selbst „**NICHT additiv**" (`:435`). Ein Feld dort ist eine v3-Versionierung.
   `label` ist ausdrücklich **User-Wort, nicht Hostwahrheit**
   (`nakama-state-v2.md:20`: „User-Wort, untrusted, nie interpretiert").
2. **Es gibt kein Capabilitybit dafür.** `eq-copilot/identity/host-capabilities-fl-v1.json`
   führt die zehn Bits aus Entwurf §53.6; NAK-27 hält fest, dass diese Liste
   **abschließend** ist. Ein Bit für Kanalkontext existiert nicht.
3. **Es ist nie gemessen worden.** Die Rohberichte aus Termin B
   (`docs/beweise/termin-b/host-probe-20260822-130657.json` und `-132644.json`)
   haben die Schlüssel `schema, erzeugt, host, wrapper, bruecke_liefert, bloecke,
   gueltig_mindestens_einmal, gueltig_immer, projektzeit, automation,
   presentation_latency, ereignisse_je_art, ereignisse_gesamt,
   ereignisse_gespeichert, ereignisse` — **kein Kanalkontextfeld**. Der Weg wäre
   VST3 `ChannelContext::IInfoListener::setChannelContextInfos` (H12, „mainly
   channel based info (color, name, index, ...)"), im gepinnten JUCE 8.0.9 als
   `AudioProcessor::updateTrackProperties` erreichbar. Ob **FL Studio** ihn
   bedient, ist beim Hersteller nicht dokumentiert und hier nie gefahren worden:
   `grep -rn 'getTrackProperties|setChannelContextInfos'` über
   `eq-copilot/plugin`, `broker`, `docs`, `design` liefert **0** Treffer außerhalb
   der gevendorten JUCE-Quellen unter `eq-copilot/build/_deps/`.

**Warum das jetzt zählt.** Genau das ist der Zeitpunktgrund von PR1: `SONDE-011`
schreibt die Datenformen fest, `G2` friert sie als R0 ein. Ein Spurname, der
danach kommt, ist eine v3-Versionierung mit Cross-Language-Nacharbeit statt eine
Zeile im Descriptor.

**Was getan.** Edit E-4 (plan.json S18–19: Messung und Vertragsort benennen),
E-5 (Bauaufteilung §3 P3-Zeile), Registerzeile **NAK-106**, Vorschlag V-1 an den
Dirigenten (v3-Erweiterung vor S16–17 entscheiden oder ausdrücklich auf v3.1
vertagen) und **Fragenkarte U20** — die Produktfrage ist nicht *ob* gemessen
wird, sondern **was der Durchschalter zeigt, wenn FL den Namen nicht hergibt**.

---

#### D-3 — Zwei Entscheide vom 23./24.08. sind nie in die Ticketquellen gewandert

**(a) Auto-Gain.** `design/abnahmen/2026-08-24-auto-gain-schalter.md` §4 sagt
wörtlich: „Es reist damit im selben Änderungssatz wie der Mix/Dry-Wet-Regler
(NAK-64 Punkt 4, Interview 23.08.) — beide sind Layout-v2-Erweiterungen, beide
gehören in **S26–28** (State/Parameter). Der aktive Audiokern … entsteht in
**P6**." Gemessen: `docs/plan/plan.json:280` (S26–28) nennt „Mix je Sonde und
die Schutz-Zonen", **nicht** Auto-Gain; dieselbe Lücke in
`docs/bauaufteilung-sonden.md` §3, Zeile S26–28 („`mix` je Instanz und die
Schutz-Zonen im gespeicherten Zustand"). Der Mix ist nachgezogen worden (NAK-64
Punkt 4), Auto-Gain kam einen Tag später und ist es nicht.

**(b) Das ±3-dB-Remote-Limit steht noch im Entwurf.** U14 (23.08.) hat es
abgeschafft; Erratum (n), `ui-spielregeln` Regel 6 und plan.json S29–31 tragen
das. Aber `docs/FL-Nakama-Sonden-Design-Entwurf.md` §44.2 (`:2660`) sagt
unverändert: „Remote-Proposals bleiben auf ±3 dB und zunächst Q 0,4–2 begrenzt."
NAK-64 hatte die Streichung als Punkt 2 seines Umschnitts zugesagt und ist am
23.08. geschlossen worden, ohne sie auszuführen. Der Errata-Block hat zwar
Vorrang, aber §44.2 ist die Stelle, die ein Ticketprüfer als Gate-Text liest
(`bauaufteilung` §2, T2-Regel „Gate-Text aus dem Entwurf").

**Was getan.** Edit E-6 (plan.json S26–28), E-7 (Bauaufteilung §3 S26–28).
Für (b) **kein** Edit an §44.2: der Text unter dem Errata-Block wird
grundsätzlich nicht umgeschrieben, damit die T2-Regel „Gate-Text aus dem
Entwurf" einen stabilen Bezug behält (`bauaufteilung-sonden.md` §6.4, berichtigt
21.08.). Die Zusage in NAK-64, den Satz zu **streichen**, war damit selbst falsch.
Stattdessen E-8: das neue **Erratum (q)** im Entwurf sagt ausdrücklich, dass der
Satz als Verlauf stehen bleibt und seit U14 aufgehoben ist. Registerzeile NAK-108.

---

#### D-4 — Der Plan macht den UI-Schritt S31b von einer Bildbasis abhängig, die seit dem 25.08. Verlauf ist

**Fundstelle.** `docs/plan/plan.json:320` (S31b): „Gen Seite 2 gegen deinen
Figma-Stand gebaut … **Startet erst, wenn dein Figma-Stand vorliegt (NAK-65)**."
NAK-65 (23.08.) beschreibt „Figma-Flächen zum EQ-Zentrale-Entscheid".

**Der Entscheid dagegen.** `design/abnahmen/2026-08-25-rework-referenz-drei-designs.md`:
„das ist ab jetzt die aktuelle wahrheit in progress. egal was du davor dachtest.
das nutzen wir als grundlage für den großen rework in zukunft" — mit drei
Bilddateien und ihren SHA-256. Leseregel 3: „Frühere Referenzen sind Verlauf:
insbesondere Figma `25:444`, `28:994`, der Suna-Knoten `6:2864`". CLAUDE.md führt
denselben Satz im Wahrheitskern.

**Warum das bricht.** S31b würde nach heutigem Plantext auf eine Lieferung
warten, die der Nachfolgeentscheid ersetzt hat — der Schritt bliebe formal
blockiert, obwohl seine Grundlage im Repo liegt
(`design/assets/rework-basis-2026-08-25/gen-page-2-eq-center.png`).

**Was getan.** Edit E-9 (plan.json S31b), E-10 (Bauaufteilung §3 S31b-Zeile).
Kein Fragenkarte — der Entscheid ist eindeutig, nur nicht nachgezogen.

---

#### D-5 — Der Klassenvertrag kennt zwei Sonden-Produktklassen; das Produkt hat seit dem 27.08. eine mit zwei Betriebszuständen

**Fundstelle.** `eq-copilot/schemas/state/nakama-state-v2.md:18` führt
`plugin_kind` als `main | passive_probe | active_probe | legacy`, die Kind-Matrix
(`:38-41`) gibt `passive_probe` **keine** `Parameters`/`Dsp`, `active_probe`
beide, und §2.3 (`:94`) bindet die Klassen an Bundles: `NkPr => {passive_probe}`,
`NkAc => {active_probe}`. Dasselbe Wortpaar im v3-Vertrag
(`eq-ipc-v3.schema.json`, `$defs/plugin_kind`).

**Der Entscheid dagegen.** `design/abnahmen/2026-08-27-arbeitsnamen-und-probeeq-doppelrolle.md`:

> „doch er ist auch der ferngesteuerte eq ausführer auf den verschiedenen bussen.
> es gibt nicht nur entweder oder"

und im selben Blatt: „‚Passiv messen' und ‚EQ ausführen' sind Betriebszustände
**derselben** Probeeq-Rolle, keine zwei Apps und keine Entweder-oder-Entscheidung."
CLAUDE.md trägt den Satz im Wahrheitskern.

**Warum das (noch) kein akuter Bruch ist — und wo er zuschlägt.** Heute ist das
ausgelieferte Probeeq-Bundle `NkAc` (`active_probe`); `NkPr` ist seit S9b
stillgelegt (`plugin-identities-v1.json`, Feld `stillgelegt`). Die gebaute
Klassenmenge passt also. Der Bruch liegt in der **Bedeutung**: eine
`active_probe` mit ausgeschaltetem EQ ist im Vertrag nicht von einer mit
eingeschaltetem unterscheidbar, während das Produkt genau diese Unterscheidung
sichtbar macht (Rückfallfläche: „EQ mit rotem oder Eq mit grünem punkt",
Interview 23.08. Frage 13). Für Gens Landkarte (S18–19) und die Aussageklasse
(NAK-79) ist das eine echte Zustandsangabe, die kein Feld hat: `capabilities`
trägt `remote_control`, nicht „EQ ist zugeschaltet".

Zusätzlich ist die Vertragsprosa nicht nachgezogen: `nakama-state-v2.md:94`
sagt weiterhin „S8: **drei** Bundles"; `eq-copilot/plugin/state/NakamaLebenslauf.h:72`
sagt „Die beiden **neuen** Bundles (NkPr/NkAc)".

**Was getan.** Registerzeile **NAK-107** (Vertragsprosa und fehlender
EQ-Zustand im Descriptor; die Dateien selbst liegen außerhalb der Ticketgrenze),
Edit E-11 (plan.json S18–19 nennt den EQ-Zustand als anzuzeigende Wahrheit),
Vorschlag V-1 (dasselbe v3-Fenster wie D-2).

**Nachtrag 30.08.2026 (Runde 1) — zwei Berichtigungen an D-5.**

**(1) Codex-Befund B4: der EQ-Punkt gehört nicht auf Gens Landkarte.** Der Absatz oben leitet aus dem Interview vom 23.08. ab, Gens Landkarte (S18–19) müsse den EQ-Zustand je Sonde zeigen. Das ist falsch: `design/abnahmen/2026-08-23-interview-struktur.md` bindet den roten/grünen EQ-Punkt in **Frage 13** und **Festlegung 16** an Probeeqs *lokale Rückfallfläche* („Rückfallfläche präzisiert: Connected/Disconnected · Bypass · bei EQ on die Werte + der Mode der Probe"). Eine Anzeige auf der passiven Landkarte ist dort nicht entschieden; sie zu planen hieße, eine sichtbare Funktion zu erfinden und dafür den strikten `probe_descriptor` unnötig zu versionieren. **Edit E-11 ist deshalb zurückgenommen**: der Satz in `plan.json` S18–19 und der entsprechende Teil der Bauaufteilungszeile sind gestrichen. Was bleibt, ist Technik ohne Anzeigezusage — Gen muss den Betriebszustand für die **Fernsteuerung** (S29–31) kennen; der Feldname wird in der v3-Vertragsrunde von S16–17 reserviert, Eigentümer S29–31. NAK-107 trägt den Nachtrag.

**(2) Codex-Befund B3: der Vertragsbruch ist größer als ein EQ-An/Aus-Feld.** D-5 hat nur den Betriebszustand gesehen. Gemessen am 30.08.2026 fehlt der **gesamte DSP-Zustandsrückweg**: `state_report` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `additionalProperties: false`) trägt nur `dsp_schema_version`, `state_revision`, `state_hash`, `record_state` und `undo_tiefe`; alle fünf `command_ack`-Zweige tragen nur `command_id`, `ergebnis`, `state_revision`, `state_hash` und `code`. Der Entwurf verlangt beides ausdrücklich — §33.5 (`:1898`) „bestätigter DSP samt Schutzgrenzen … Broker/Main lesen per `state_report`, überschreiben nie blind" und §33.4 (`:1881`) „bestätigt `state_hash` plus tatsächlich angewandte Werte". **Folge:** nach Project-Reload oder Reconnect kann Main den autoritativen Probeeq-Zustand nicht rekonstruieren; ein Hash kann Bänder, Bypass, Mix und Schutzbereiche weder anzeigen noch konfliktfrei wiederherstellen. In `reservierte-nachrichten-v1.json` ist dafür nichts reserviert — das DSP-DTO ist dort nur `apply_transaction` (`SONDE-016`) zugeordnet. **Neue Registerzeile NAK-110**; eingearbeitet in `plan.json` S16–17 (Name `state_report.dsp` reserviert), S26–28 (Nutzlast mit dem DSP-DTO) und S29–31 (Bestätigung der angewandten Werte) sowie in die drei zugehörigen Zeilen der Bauaufteilung.

---

#### D-6 — Zwei nachweisbar falsche Statusaussagen in Planquellen

**(a) `docs/plan/plan.json:329`, Phasenziel P8–P9:** „(nur wenn Termin A und
Gate G0 grün sind — Termin A ist grün, **G0 steht aus**)". G0 trägt seit dem
22.08. `T3 PASS` (`docs/beweise/G0.md`, in `docs/PLAN-STAND.md` als „abgenommen ·
T3 PASS 2026-08-22" gerechnet). Der Satz erscheint wörtlich im erzeugten
Planstand und behauptet dort das Gegenteil der danebenstehenden Messung.

**(b) Sieben Schritttexte tragen Statusbehauptungen, die die gerechnete Marke
widerlegt.** `docs/plan/LIES-MICH.md` sagt über `plan.json`: „Phasen, Schritte,
Ticket, Klartext … **Kein Statusfeld.**" Gemessen tragen die Texte trotzdem
Status, und zwar veralteten:

| Zeile | Text sagt | Gerechnet |
|---|---|---|
| `plan.json:100` (S5) | „das abschließende Prüfurteil … steht noch aus" | abgenommen, T3 PASS 2026-08-28 |
| `:108` (S6) | „Prüfurteil wie S5 noch offen" | abgenommen, T3 PASS 2026-08-28 |
| `:124` (S8) | „Auf den nachgebesserten Stand fehlt ein zweites Urteil" | abgenommen, T3 PASS 2026-08-30 |
| `:132` (S9) | „NICHT abgenommen … STATUS BLEIBT 'gebaut'" | abgenommen, T3 PASS 2026-08-30 |
| `:163` (S10–11) | „Steht weiterhin auf 'gebaut' und nicht auf 'erledigt'" | abgenommen, T2 PASS 2026-08-24 |
| `:171` (S12–13) | „Nicht abgenommen; Nacharbeit ist der naechste Schritt" | abgenommen, T2 PASS 2026-08-24 |
| `:179` (S14–15) | „STATUS weiter 'gebaut', nicht 'erledigt'" | abgenommen, T2 PASS 2026-08-30 |

**(c) `docs/bauaufteilung-sonden.md` §3 führt eine zweite, handgepflegte
Statusspalte,** die dasselbe Problem hat: die P1-Zeilen S5/S6 sagen „**T2 offen**
— kein PASS im Manifest", S8 „**Kein PASS auf den neuen Stand**", S9
„T1+T2 — **beide offen**", S12–13 „**T2 offen** — kein Prüfer gelaufen". Alle vier
sind heute abgenommen.

**Warum das ein Defekt ist und keine Kosmetik.** Fail-closed schützt gegen
Übertreibung (LIES-MICH: „Vergessen führt zur Untertreibung, nie zur
Übertreibung") — hier steht die Untertreibung aber **als Prosa neben der
richtigen Messung in derselben Zeile** des erzeugten Blatts. Ein frischer
Prüfer, der laut Prüfliste E „Behauptung ≤ Messung" misst, findet an dieser
Stelle eine Behauptung, die von der Messung widerlegt wird. Das ist genau die
Klasse, die die Prüfliste meint.

**Was getan.** Edit E-12 (plan.json:329, G0-Satz berichtigt), E-13 (die sieben
Schritttexte bekommen je einen datierten Abschlusssatz statt einer
Statusbehauptung — der Verlauf bleibt wortgleich stehen), E-14 (Bauaufteilung §3
bekommt einen Kopfhinweis, dass die Prüfspalte Verlauf ist und der verbindliche
Status aus `docs/PLAN-STAND.md` kommt, plus Berichtigung der vier
widersprechenden Zellen).

**(d) Nebenbefund im selben Zug, `docs/bauaufteilung-sonden.md` §4:** die
Summentabelle zählt P1 = 5 Sessions und „Gesamt 39 · Bis R4 39 + 10 = 49". Seit
dem 28.08. gibt es S9b (P1 = 6) und seit dem 29.08. PR1. Der 28.08.-Entscheid
sagt, er sei „eingearbeitet in … `docs/bauaufteilung-sonden.md` §3" — §3 ja, §4
nein. Mit Edit E-14 nachgezogen.

---

### 4.2 Lücke (4)

Eine Lücke ist eine Stelle, an der der Plan nichts sagt und ein Entscheid nötig
ist.

| # | Lücke | Fundstelle | Was getan |
|---|---|---|---|
| **L-1** | **Wer darf `post_fader_contribution` führen, wenn es den Bus gibt?** Heute ist die Position für **keine** Klasse erlaubt (`nakama-state-v2.md:54`, fail-closed seit der G1-Nacharbeit). Der Entwurf beantwortet die Frage nicht (§32.2 ordnet Positionen Aussageklassen zu, nennt aber keine Produktklasse). Eigentümer laut Vertrag: `SONDE-011`. | `nakama-state-v2.md:54-90`; NAK-79; `plan.json:195` (S16–17) nennt es nicht | Edit **E-15** (S16–17 trägt die Frage als Lieferumfang); die Produktseite ist **Fragenkarte U19** nicht wert — sie ist eine Vertragszuordnung, nicht sichtbare Funktion, und liegt beim Dirigenten. Registerzeile bleibt NAK-79. **Nachtrag 30.08.2026 (Runde 1, Codex-Befund B1): die Lücke war falsch gestellt.** Die Produktfrage ist seit dem **24.08.2026** entschieden — der NAK-79-Nachtrag hält den User-Wortlaut fest („das prinzip ist doch jeweils eine sonde auf ein bus -> Gen, wieso sollte eine sonde als empfänger genutzt werden?" · „EINE Sonde kann nur dort messen wo sie steckt … das ist kein prinzip das ist die einzige realität") und schließt daraus: **keine Sondenklasse trägt die Position, Empfänger ist Gen.** Nach `CLAUDE.md` ist dieses datierte Registerzitat bindend; ein Edit, der die Frage in S16–17 erneut stellt, widerspricht einem Entscheid. **Edit E-15 ist entsprechend umgeschrieben**: S16–17 trägt nur noch die **Technikfrage** aus demselben Nachtrag — die Vertragsform des Beitrags („aus welchem Gen-Eingang stammt die Messung", ein Deskriptor je Gen-Eingang statt eines Wertes im Instanzfeld). Entscheid des Dirigenten 30.08.2026: der **Name** wird in der v3-Vertragsrunde von S16–17 reserviert, das Instanzfeld bleibt fail-closed, die **Nutzlast** entsteht mit dem Aux-Bus in `SONDE-018`. |
| **L-2** | **Broker-Autostart hat keine Planzeile.** NAK-13 (18.08.) offen; `state::Lebenslauf::darfBrokerStarten()` existiert seit S9 als Haken, aber kein Schritt baut den Start. Ohne ihn misst ein frisch installiertes Nakama nur lokal. | NAK-13; `plan.json` — `grep -c 'Autostart'` = 0 | Edit **E-16**: S16–17 bekommt den Brokerstart als benannten Lieferumfang (er gehört zum Koordinator-Ticket, weil vorher niemand da ist, der etwas entgegennimmt). |
| **L-3** | **Keine Voreinstellungen (Presets) im ganzen Plan.** Weder Parameterbestand, State-Kind-Matrix, v3-Familienliste noch die SQLite-Tabellen aus §53.9 kennen ein Preset-Objekt. Vergleichbare Produkte führen einen Preset-Browser als Grundausstattung (H3, H8). | `grep -nic 'preset' docs/FL-Nakama-Sonden-Design-Entwurf.md` = **1** Treffer, und der ist keine Produktfunktion (`:2576`: ein vom User bereitgestellter Preset-**Hash** als Experimentmerkmal) | **Fragenkarte U18** (Produktumfang) |
| **L-4** | **Die offenen Punkte aus `2026-08-25-offen-bausteine.md` (Akzentfarbe je App u. a.) stehen in keiner Fragenkarte.** Der Entscheid liegt beim User am Blatt, aber `fragen.json` `offen[]` führt nur U11 und U13. | `design/abnahmen/2026-08-25-offen-bausteine.md`; `fragen.json` | Vorschlag **V-2** an den Dirigenten: eigene Fragenrunde, nicht in PR1 hineinziehen (es ist Gestaltung, kein Planbruch) |

### 4.3 Härtung (3)

| # | Härtung | Fundstelle | Was getan |
|---|---|---|---|
| **H-1** | Abgenommenes Maß 760×430 und laufender Editor tragen verschiedene Seitenverhältnisse (NAK-82, 25.08., `PluginEditor.cpp:184-190`). Der Entscheid vom 25.08. („festes Seitenverhältnis bleibt") macht das Verhältnis zur Produktzusage, nicht nur zum Startwert. | NAK-82; `design/abnahmen/2026-08-25-seitenverhaeltnis-bleibt-fest.md` | Nur benannt — die Quelle liegt in `eq-copilot/**`, außerhalb der Ticketgrenze. Vorschlag **V-3**: an S31b hängen. |
| **H-2** | „12 von 20 gewählten Kernfunktionen" (§0.2, §22) ist seit dem 21.08. als nicht rekonstruierbar bekannt (NAK-39, Erratum (m)) und steht unverändert im Entwurf. | Entwurf `:361-370`, `:1179-1183`; NAK-39 | Nur benannt; eine Korrektur wäre eine Zahlenänderung ohne Beleg. |
| **H-3** | Entwurf §55 Lieferumfang nennt weiterhin „drei VST3-Ziele" und „Installer-Manifest für alle **drei** Bundles"; der 28.08.-Nachtrag darüber korrigiert nur Klausel 1 des Exit-Gates. | Entwurf `:3827`, `:3830`, `:3839` | Edit **E-17**: Der Nachtrag wird um Lieferumfang und Exit-Gate-Wortlaut erweitert (derselbe Entscheid, nur vollständig nachgezogen). |

---

## 5. Funktionsvergleich mit vergleichbaren Produkten

Maßstab ist der vom User selbst gesetzte: „er muss das rad nicht neu erfinden,
aber vorbild ist auf jedenfall besagte eq" und „smooth operator pro , oder pro-q
sind die 2 besten. dafür kannst du alles studieren wie sie das handhaben und das
beste kopieren" (`design/abnahmen/2026-08-23-interview-struktur.md`, Fragen 4
und 6).

Legende **betroffen**: `V` = v3-Nachrichtenvertrag · `S` = State/Parameter ·
`O` = Oberfläche · `C` = Capability-/Hostvertrag.

### 5.1 Mehrinstanz-Analyse und Fernsteuerung — Nakamas Kernversprechen

| Standardfunktion | Wer hat sie (Beleg) | Nakama | betroffen | Ticket | Folge |
|---|---|---|---|---|---|
| Alle Instanzen der Sitzung in einer Liste sehen | Pro-Q 4 (H1), smart:EQ 4 bis 10 pro Gruppe (H6) · ~~MMultiAnalyzer (H7)~~ ~~SPAN Plus (H8)~~ **gestrichen 30.08.2026 (Runde 1, Befund B6):** H8 führt keine Instanzliste (die Feature-Seite nennt „Multi-track spectrum import/export", keine Liste), und H7 belegt nur, dass Instanzen sich finden und Analysen tauschen — beides steht in den passenden Zeilen weiter unten | **hat** — Gen Seite 1, Sonden bis 16 sichtbar / 32 im Vertrag | — | S18–19 | — |
| Fremde Instanz **fernbedienen** | Pro-Q 4: „add and change curves just like you are used to" (H1); smart:EQ 4: „remotely controlled from any other instance" (H6) | **hat** — und macht es zum Hauptbedienweg (Gen Seite 2) | — | S29–31, S31b | Der Markt-Befund aus dem 23.08.-Blatt bestätigt sich: die Fernsteuerung selbst ist **nicht** die Neuheit; der Hub mit Advisor ist es. |
| **Reihenfolge = DAW-Spurreihenfolge** | Pro-Q 4: „matching the track order in your DAW … Instances will order themselves according to how they are inserted" (H1) | **entschieden, aber ohne Datenweg** | **V, C** | S18–19 | **D-2** |
| Spurname am Eintrag | Pro-Q 4 (H1, per Track gruppiert); smart:EQ 4 Gruppenliste (H6) | **entschieden, aber ohne Datenweg** | **V, C** | S18–19 | **D-2**, Karte **U20** |
| Prioritäts-/Rangfolge zwischen Instanzen | smart:EQ 4: „drag and drop them into an order that decides the precedence" (H6) | **löst anders** — musikalische Rollen (führt/trägt/begleitet/geschützt/verschmolzen), Entwurf §9/§37 | — | S23–25 | — |
| Instanzen finden sich selbst | MMultiAnalyzer: „they automatically find each other and exchange their respective analyses" (H7); iZotope Relay als Inter-Plugin-Communication (H5) | **hat** — Broker + Discovery, `session_snapshot` | — | S16–17, S18–19 | Autostart des Brokers fehlt: **L-2** |
| **Pegel und Panorama aller Spuren aus einem Fenster** (Mehrinstanz-Mixworkflow) | Neutron 5 Visual Mixer (H5) | **außerhalb des Produktumfangs** — Nakama regelt Klang über EQ (Wahrheitskern: Quellen, Befunde, Advisor, EQ-Zentrale); Pegel und Pan fremder Spuren zu setzen ist kein Nakama-Versprechen und **nicht** mit dem EQ-Dry/Wet-Regler gleichzusetzen | — (weder Vertrag noch State noch Oberfläche betroffen) | — | **Eingeordnet 30.08.2026 (Runde 1, Befund B7)**, Register **NAK-111**. Keine Fragenkarte: es ist kein Planbruch, sondern eine Grenze. Ein späteres Ja wäre ein Produktentscheid mit eigener v3-Nachrichtenfamilie (schreibender Eingriff außerhalb des EQ, mit Lease und Rückweg) |
| Spektrum einer anderen Spur im eigenen Graph | Pro-Q 4 External/SC-Spektrum (H2); SPAN Plus „Multi-track spectrum import/export" (H8; das frühere Zitat „Export To" steht so nicht auf der Herstellerseite — berichtigt 30.08.2026, Runde 1); smart:EQ 4 Gruppen-Analyzer (H6) | **hat** (Datenweg) — Gen Seite 2 zeigt zwei EQ-Spuren in einem Graph (Entscheid 23.08.) | — | S31b | Live-Spektrum bei zwei Spuren nur für die bearbeitete Spur (Festlegung 14) |

### 5.2 Maskierung und Entmaskierung

| Standardfunktion | Wer hat sie (Beleg) | Nakama | betroffen | Ticket | Folge |
|---|---|---|---|---|---|
| **Kollisions-/Maskierungsanzeige direkt im EQ-Graph** | Pro-Q 4 „Show Collisions … red glow" (H2); Neutron 5 Masking Meter (H5); MMultiAnalyzer Kollisionsdetektor (H7) | **löst anders und später** — Maskierung ist eine Ursachenklasse des Advisors (Entwurf §8) und wird als Text-Befund gezeigt; die vollständige Matrix ist Roadmap 3 (§47.1), also **nach R4** | **O** (Anzeige), V (nur wenn Live-Paarwerte in den Graph sollen) | S23–25 / S31b | **Fragenkarte U16** |
| Automatische Entmaskierung über Sidechain | Trackspacer 32 Bänder (H9); Neutron Unmask (H5); smart:EQ 4 Group-Modus (H6) | **hat, geplant** — Kernfunktion 17, P8 (`SONDE-018`), aber ausdrücklich **kein** globaler Auto-Spectral-Ducker (§17) | — | S32–33 | Freigabe hängt an `aux_priority_sidechain`, heute `unsupported` bis Termin A2 (U11) |
| Vollautomatischer wahrnehmungsbasierter Ausgleich | Gullfoss Recover/Tame (H11); smart:EQ smart:filter (H6) | **bewusst nicht** — CLAUDE.md: „Der Advisor schlägt nur vor"; Grundgesetz „nichts Ungefragtes" | — | — | kein Fund; ausdrücklicher Produktentscheid |
| Lautheitskompensation, damit „lauter" nicht als „besser" gehört wird | Gullfoss: die Kurve kombiniert Anheben und Absenken und „preserves the original dynamics and perceived loudness" (H11); Pro-Q: Auto Gain als Schätzung aus der Kurve (H4) | **entschieden 24.08., nicht im Plan** — und im Vergleich der schwächere Weg: Nakamas Auto-Gain ist wie Pro-Q eine Funktion der Kurve, Gullfoss kompensiert durchgehend | S, O | S26–28 | **D-3 (a)**; Pro-Q ist das vom User genannte Vorbild, Gullfoss nur Vergleichspunkt |
| Resonanzdämpfung mit einstellbarer Auflösung | Smooth Operator Pro FOCUS/Detail/Isolation (H10) | **teilweise** — dynamische Bänder je Slot, aber acht Slots statt Tausenden Knoten; der User nennt den Resonanzfokus selbst als „spätere Ausbaustufe" (Interview 23.08., Annahmen) | S, O | nach R4 | kein neuer Fund |

### 5.3 EQ-Grundausstattung

| Standardfunktion | Wer hat sie (Beleg) | Nakama | betroffen | Ticket | Folge |
|---|---|---|---|---|---|
| Minimalphasige Filter, Bells/Shelves/Cuts/Notch | Pro-Q 4 (H3) | **hat** — §30.1, §44.2, acht Slots × 13 Parameter | — | S26–28 | — |
| Dynamische Bänder | Pro-Q 4 (H3); TDR Nova (**H13**, nachgereicht 30.08.2026 in Runde 1: „four dynamic EQ bands", „each band also includes a full featured dynamics section"); Neutron 5 (H5) | **hat** — `dynamic_enabled`, Threshold/Range/Attack/Hold/Release | — | S26–28 | — |
| Externer Sidechain je Band | Pro-Q 4 (H3); Neutron „band-specific internal or external sidechain" (H5) | **hat im Vertrag** (`sidechain_source`), im ersten Release **unsichtbar** (U5, 24.08.) | — | S26–28 | bewusster Entscheid |
| M/S und L/R je Band | Pro-Q 4 (H3); Trackspacer (H9) | **hat** — `channel_mode` | — | S26–28 | — |
| **Phasenmodi: Natural Phase / Linear Phase** | Pro-Q 4: „zero-latency mode, Natural Phase mode or in linear-phase mode with variable processing resolution" (H3) | **bewusst nicht im ersten Kern** — §30.1 schließt lineare Phase aus, §44.2 fordert konstant 0 Samples Latenz, kein Lookahead | **C** (Latenzmeldung), **S** (neuer Parameter), **O** (Umschalter) | S26–28 / S28b, G6 | **Fragenkarte U15** — der User nannte Pro-Q als Vorbild und „alle gängigen filter die es gibt" |
| Auto-Gain | Pro-Q 4 (H4) | **entschieden 24.08., nicht im Plan** | S, O | S26–28 | **D-3 (a)** |
| Band-Solo / einzelnes Band abhören | Pro-Q 4 Band solo (H3) | **löst anders** — Hörmodus `FOCUS` (§41.1: „bearbeiteter Frequenz-/M/S-Anteil der Zielsonde") | — | S20–22 | kein Fund |
| Delta-Abhören (nur die Änderung) | Neutron 5 Delta-Buttons (H5) | **hat** — Hörmodus `DELTA` (§41.1) | — | S20–22 | — |
| Ausgangs-Trim, Width, Mono-Maker | Pro-Q 4 Output (H4) | **hat** — `input_trim_db`, `output_trim_db`, `width`, `mono_bass_hz` | — | S26–28 | — |
| **Voreinstellungen (Preset-Browser)** | Pro-Q 4 (H3); SPAN Plus Preset-Manager (H8) | **fehlt als eigene Funktion** — Projekt-State reist über den Host, aber es gibt kein Preset-Objekt, keinen Browser, keine Werksvoreinstellungen | **S**, O | offen | **Fragenkarte U18**, Lücke **L-3** |
| Undo/Redo, A/B, Kopieren | Pro-Q 4 „Undo, Redo, A/B and Copy" (H3); SPAN Plus (H8). **Nachtrag 30.08.2026 (Runde 2, Restbefund zu B6):** für **Undo/Redo und A/B** tragen H3 und H8 die Zuordnung. Für **Kopieren** tragen sie sie nicht: Pro-Q 4s „Copy" ist laut Herstellerhilfe „The Copy button copies the active state to the inactive state" (**H15**) — die A/B-Zustandskopie innerhalb derselben Instanz —, und die SPAN-Plus-Feature-Seite nennt kein Kopieren. **Das Übertragen einer Kurve zwischen Instanzen ist damit keine belegte Marktstandardfunktion**, sondern ein Nakama-eigener Wunsch; die Zelle rechts und Karte U18 führen ihn weiter, jetzt ohne Marktbehauptung | **teilweise** — eigener Undo-Ring je Sonde (Entscheid 23.08., Festlegung 21), A/B als Experiment (§43). **Kopieren einer Kurve zwischen Sonden fehlt**; `nakama-state-v2.md:94` behandelt Copy/Paste nur als Gefahr (Fremdklasse ⇒ read-only) | **S**, O | S26–28 / S31b | in **U18** mitgeführt |
| MIDI Learn | Pro-Q 4 (H3) | **löst anders** — FL bindet Hostparameter selbst; für die Sonde gehen 109 Parameter an den Host, **für Gens Master-EQ heute null** | S | S28b | Folge von **D-1** |
| Fenstergröße frei / Vollbild | Pro-Q 4 „Full Screen mode and resizable interface" (H3) | **bewusst nicht** — festes Verhältnis, Stufen 100/125/150/200 % (U2.17; Entscheid 25.08.: „lassen wir es erstmal dabei") | — | — | ausdrücklicher Entscheid, kein Fund |

### 5.4 Analyse und Anzeige

| Standardfunktion | Wer hat sie (Beleg) | Nakama | betroffen | Ticket | Folge |
|---|---|---|---|---|---|
| Live-Spektrum hinter der Kurve | Pro-Q 4 (H2) | **hat** — Festlegung 14 (23.08.) | — | S31b | — |
| **Spectrum Grab** (Band direkt aus dem Spektrumgipfel ziehen) | Pro-Q 4 (H2) | **fehlt** | **O** | S31b | **Fragenkarte U17** |
| **Freeze / Spitzenwerthalten** | Pro-Q 4 „stop falling and build up a maximum over time" (H2) | **fehlt** | **O** | S31b | **U17** |
| **Anzeigeeinstellungen: Range, Resolution, Speed, Tilt** | Pro-Q 4 (H2: 60/90/120 dB, 1024–8192 Punkte, Tilt 4,5 dB/Okt) | **fehlt** — Nakama hat feste Bandgitter (64 live, 221 Evidenz) und feste Kadenzen | **O** (nur Darstellung; die Bandgitter selbst bleiben Vertrag) | S31b | **U17** |
| **Noten-/Klaviaturanzeige** | Pro-Q 4 (H2) | **fehlt** — und hat eine Landmine: „FL zeigt MIDI 60 als C5" (CLAUDE.md) | **O** | S31b | **U17** |
| Lautheits- und True-Peak-Anzeige | SPAN Plus EBU R128 LUFS/LU, True Peak (H8) | **hat gemessen** (`LoudnessAccumulator`, S10–11), **Anzeige nicht entschieden** — NAK-57: „auslesbar, aber nirgends sichtbar" | **O** | S18–19 | in **U17** mitgeführt |
| Korrelations-/Stereoanzeige | SPAN Plus (H8); MMultiAnalyzer Stereoanalyse (H7) | **hat geplant** — Kernfunktion 10, §40 | — | S20–22 | — |
| Referenz-/Zielkurve übernehmen (EQ Match) | Pro-Q 4 EQ Match gegen andere Instanz oder Datei (H1, H3); smart:EQ 4 „load up a reference track to create a custom profile" (H6); iZotope Ozone Match EQ (**H14**, nachgereicht 30.08.2026 in Runde 1: „Match to any reference track with over 8,000 separate bands") | **löst später** — Roadmap 11 (§47.5), **nach R4** | **V** (neue Familie), S, O | nach R4 | **Fragenkarte U19** |
| Assistent, der eine Kette vorschlägt | Neutron 5 Mix Assistant (H5); smart:EQ Profile (H6) | **hat, regelbasiert** — Kernfunktion 18, §46; ausdrücklich **ohne** KI-Schicht (Errata (e)) | — | S23–25 | ausdrücklicher Entscheid |

### 5.5 Was der Vergleich **nicht** gefunden hat

- Kein Hersteller im Vergleich liefert einen **Advisor mit belegter Ursache,
  Alternativen, Sicherheitsangabe und kleinstem Gegenversuch** in Nakamas Form
  (Entwurf §8, §36). Der 23.08.-Markt-Befund („die echte Innovation ist der dedizierte
  Hub mit Advisor") hält der Nachprüfung stand.
- Kein Hersteller im Vergleich macht die **Messposition** (PRE/INSERT/POST) und
  die daraus folgende **Aussageklasse** zum Vertragsbestandteil. Das ist
  Nakama-eigen und deutlich strenger als der Marktstand.
- Kein Vergleichsprodukt garantiert **bitidentischen Nulltest im ausgeschalteten
  Zustand** als geprüfte Zusage.

---

## 6. Änderungssatz

### 6.1 Edits — je Edit Grund und Fundstelle

| # | Datei | Was | Grund |
|---|---|---|---|
| E-1 | `docs/plan/plan.json` S28b (`:288`) | Zustandsversionierung für `plugin_kind = main` als Lieferumfang benannt (Kind-Matrix, Parameterlayout, `state_hash`) | **D-1** |
| E-2 | `docs/bauaufteilung-sonden.md` §3, Zeile S28b | dieselbe Zeile nachgezogen | **D-1** |
| E-3 | `docs/FL-Nakama-Sonden-Design-Entwurf.md`, neues **Erratum (q)**, erster Punkt | stellt klar, dass „§53.8-State unverändert" aus Erratum (n) der Sonde gilt und nicht Gens Master-EQ; benennt die Versionierung und ihren Ort (S28b). Erratum (n) selbst bleibt unverändert stehen | **D-1** |
| E-4 | `docs/plan/plan.json` S18–19 (`:218`) | Messung des Host-Kanalkontexts und Vertragsort für Bus-Name/Reihenfolge als Lieferumfang | **D-2** |
| E-5 | `docs/bauaufteilung-sonden.md` §3, Zeile S18–19 | dieselbe Zeile nachgezogen | **D-2** |
| E-6 | `docs/plan/plan.json` S26–28 (`:280`) | Auto-Gain als Layout-v2-Erweiterung ergänzt | **D-3 (a)** |
| E-7 | `docs/bauaufteilung-sonden.md` §3, Zeile S26–28 | dieselbe Zeile nachgezogen | **D-3 (a)** |
| E-8 | `docs/FL-Nakama-Sonden-Design-Entwurf.md`, **Erratum (q)**, zweiter Punkt | hält fest, dass der ±3-dB-Satz in §44.2 (`:2660`) Verlauf ist und seit U14 aufgehoben — und dass NAK-64s Zusage, ihn zu streichen, gegen §6.4 der Bauaufteilung verstieß. **Kein Edit an §44.2 selbst** | **D-3 (b)** |
| E-9 | `docs/plan/plan.json` S31b (`:320`) | Startbedingung auf die drei Rework-Bilder vom 25.08. umgestellt | **D-4** |
| E-10 | `docs/bauaufteilung-sonden.md` §3, Zeile S31b | dieselbe Zeile nachgezogen | **D-4** |
| E-11 | `docs/plan/plan.json` S18–19 (`:218`) | EQ-Zustand je Sonde (zugeschaltet/aus) als anzuzeigende Wahrheit | **D-5** |
| E-12 | `docs/plan/plan.json` (`:329`) | „G0 steht aus" berichtigt | **D-6 (a)** |
| E-13 | `docs/plan/plan.json` (`:100`, `:108`, `:124`, `:132`, `:163`, `:171`, `:179`) | je ein datierter Abschlusssatz statt einer widerlegten Statusbehauptung | **D-6 (b)** |
| E-14 | `docs/bauaufteilung-sonden.md` §3-Kopf, vier Prüfzellen, §4 Summentabelle | Statuswahrheit auf `docs/PLAN-STAND.md` verwiesen; S9b und PR1 in der Summe nachgezogen | **D-6 (c)/(d)** |
| E-15 | `docs/plan/plan.json` S16–17 (`:195`) | NAK-79 (Aussageklasse) als Lieferumfang benannt | **L-1** |
| E-16 | `docs/plan/plan.json` S16–17 (`:195`) | Broker-Autostart als Lieferumfang benannt | **L-2** |
| E-17 | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §55-Nachtrag | Nachtrag auf Lieferumfang und Exit-Gate-Wortlaut erweitert | **H-3** |

Neue Registerzeilen in `docs/offene-punkte.md` (fortlaufend nach NAK-104):
**NAK-105** (D-1), **NAK-106** (D-2), **NAK-107** (D-5), **NAK-108** (D-3 b),
**NAK-109** (D-6 als Klasse: Status in zwei Quellen).

### 6.2 Neue Fragenkarten an den User

Alle in `docs/plan/fragen.json` `offen[]`, `seit: "2026-08-30"`, `status: "offen"`.
`beantwortet{}` ist unangetastet, keine Karte geschlossen.

| Karte | Titel | betroffen | Ticket |
|---|---|---|---|
| **U15** | Soll der EQ einen zweiten Filtermodus ohne Phasendrehung bekommen? | Vertrag (Latenz), State, Oberfläche | S26–28, S28b, G6 |
| **U16** | Soll Nakama Frequenzkollisionen direkt im Kurvenbild zeigen? | Oberfläche (Vertrag nur bei Live-Paarwerten) | S23–25, S31b |
| **U17** | Was der Graph außer der Kurve können muss (Spektrum greifen, festhalten, Anzeigeeinstellungen, Notennamen, Lautheitsanzeige) | Oberfläche | S31b, S18–19 |
| **U18** | Soll es gespeicherte Voreinstellungen geben — und darf man Einstellungen zwischen Sonden kopieren? | State, Oberfläche | S26–28, S31b |
| **U19** | Soll Nakama den Klang eines anderen Busses oder einer Referenzdatei nachbilden können? | Vertrag, State, Oberfläche | nach R4 |
| **U20** | Was zeigt der Sonden-Durchschalter, wenn FL Studio den Spurnamen nicht herausgibt? | Vertrag, Capability, Oberfläche | S18–19, S31b |

### 6.3 Vorschläge an den Dirigenten (nicht eingearbeitet)

1. **V-1 — Ein v3-Fenster vor S16–17.** ✅ **ENTSCHIEDEN am 30.08.2026 (Runde 1, Dirigent, Technik): die v3-Vertragsrunde wird als erster Lieferumfang von `SONDE-011` vor R0 gefahren.** Umfang, Registerzuordnung und Reservierungen stehen in §9, Befund B2. Der Vorschlagstext darunter bleibt als Verlauf stehen. D-2 und D-5 brauchen beide ein Feld im
   `probe_descriptor`, der ausdrücklich „NICHT additiv" ist. Entweder wird eine
   v3-Erweiterung **vor** `SONDE-011` entschieden (dann trägt sie R0), oder sie
   wird ausdrücklich auf eine v3.1 nach G2 vertagt. Beides ist vertretbar; still
   offenlassen ist es nicht, weil G2 den Stand als R0 einfriert. Empfehlung:
   Erweiterung vor S16–17 aufnehmen, weil der Descriptor sonst zweimal
   versioniert wird.
2. **V-2 — Eigene Fragenrunde für `2026-08-25-offen-bausteine.md`** (Akzentfarbe
   je App u. a.). Gestaltung, kein Planbruch — gehört nicht in PR1.
3. **V-3 — NAK-82 an S31b hängen.** Das Seitenverhältnis ist seit dem 25.08. eine
   Produktzusage; die abweichende Editorquelle liegt in `eq-copilot/**` und
   damit außerhalb dieser Ticketgrenze.

---

## 7. Prüfliste `tools/dirigent/pruefliste.md`

**A–D sind für ein Dokumentticket nicht anwendbar** — hier wird kein
Rückstauverhalten, kein Lebenszyklus, kein Längen- oder Alphabetvertrag und kein
Bau-/Prüfriegel geändert. Es gibt keinen Produktcode, keinen Test und kein
Schema in diesem Änderungssatz (Beleg: `git diff --stat` in §8, Lauf D, berührt
ausschließlich `docs/**`).

### E. Behauptung ≤ Messung

| Zeile | Wo gemessen |
|---|---|
| „Jede Behauptung sagt nicht mehr, als der Test misst" | Jeder Befund in §4 nennt Datei und Zeile und wurde an der Quelle gelesen, nicht aus einem Manifest abgeschrieben. Wo ich eine Negativaussage mache („es gibt keinen Datenweg"), steht der `grep`-Befehl mit seiner Trefferzahl daneben (D-2, L-3). |
| „Zahlen im Manifest sind gemessen, nicht abgeschrieben" | §1.3 nennt zu jeder Zahl den Befehl, mit dem sie gezählt wurde. Die Beinanzahl des Kanons wird hier **nicht** behauptet — der Kanon läuft nicht (§8). |
| „Positionen im lebenden Kopf stehen als Symbol/Anker" | Fundstellen sind Datei plus Zeilennummer **zum Stand `6f40eed`**; wo eine Zeile wandern kann (`plan.json`-Texte), steht zusätzlich die Schritt-ID als Anker. **Gemessener Versatz nach diesem Änderungssatz:** Erratum (q) fügt 25 Zeilen oberhalb von §0 in den Entwurf ein, alle zitierten Entwurfszeilen unterhalb des Errata-Blocks liegen auf `1a17221` deshalb **+25** (z. B. §44.2 `:2660` → `:2685`, §55 `:3827` → `:3852`). Abschnittsnummern (§30.1, §44.2, §55) sind der stabile Anker. |
| „Der lebende Kopf wird beim Abschluss nachgezogen" | Dieses Manifest hat nur einen Kopf und keinen Verlauf — es ist die erste Runde. |
| „Jede neue Prüfung wurde einmal absichtlich gebrochen" | **Greift hier nicht:** dieses Ticket baut keine Prüfung. Der einzige ausführbare Riegel im Änderungssatz ist die JSON-Gültigkeit von `plan.json`/`fragen.json`; sie ist in §8, Lauf C, nach dem Edit gefahren, und jedes Edit-Skript hat vor dem Schreiben zusätzlich den bytegleichen JSON-Roundtrip der Ausgangsdatei geprüft. |
| „Geänderte Zusage — drei Stellen" | Zutreffend und ausgeführt: **D-3 (a)** und **D-4** ändern je eine Zusage; beide sind in `plan.json` **und** `bauaufteilung-sonden.md` nachgezogen (E-6/E-7, E-9/E-10), die dritte Stelle (`tools/beweise.ps1`-Runnerkopf) ist nicht betroffen, weil keine Runner-Behauptung berührt wird. |
| „Writer-Fixtures statt Handschrift" | greift nicht — keine Fixtures im Änderungssatz. |

### F. Änderungssatz

| Zeile | Wo gemessen |
|---|---|
| „speichern↔laden, starten↔stoppen, … im selben Commit" | Ein Gegenpaar ist betroffen: **starten↔stoppen des Brokers** (L-2/E-16). Der Edit benennt beide Hälften in derselben Schrittzeile (Autostart **und** Rückweg über den vorhandenen `Install-Nakama.ps1`-Pfad), nicht nur den Start. |
| „Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines Vertrags im selben Änderungssatz" | **Greift hier nicht als Pflicht, aber als Befund:** dieses Ticket ändert keinen Vertrag — es stellt fest, dass zwei Entscheide einen brauchen (D-1: State-Kind-Matrix mit C++-Leser, B2-Bein und Goldens; D-2/D-5: v3-Descriptor mit C++- und Rust-Leser und den Fixtures). Genau deshalb sind sie als Planarbeit eingetragen und nicht nebenbei gemacht. |

---

## 8. Beweislauf

**Befehle, Reihenfolge und Exitcodes.** Gefahren am 2026-08-30 vom Workspace-Root
`C:/Users/phili/Projekte/Nakama` auf dem Stand `805e108` (dem Änderungssatz aus §6.1),
wortgleich unten. Rohausgabe ungekürzt:

```text
### A) py -3.13 tools/plan/planstand.py
geschrieben: docs\PLAN-STAND.md (18 abgenommen, 2 gebaut, 38 gesamt, aus 805e108)
EXITCODE=0

### B) py -3.13 tools/plan/antworten_blatt.py
geschrieben: docs\ANTWORTEN-OFFEN.md (42 Antworten, 0 offen, 42 eingearbeitet, 13 mit eigenem Text)
EXITCODE=0

### C) JSON-Gueltigkeit
docs/plan/plan.json -> gueltiges JSON 34829 Zeichen
docs/plan/fragen.json -> gueltiges JSON 26332 Zeichen
EXITCODE=0

### D) git diff --stat 6f40eed..HEAD
 docs/ANTWORTEN-OFFEN.md                 |  12 +-
 docs/FL-Nakama-Sonden-Design-Entwurf.md |  32 +-
 docs/bauaufteilung-sonden.md            |  42 +-
 docs/beweise/PR1.md                     | 697 ++++++++++++++++++++++++++++++++
 docs/offene-punkte.md                   |   7 +-
 docs/plan/fragen.json                   |  72 ++++
 docs/plan/plan.json                     |  26 +-
 7 files changed, 854 insertions(+), 34 deletions(-)
EXITCODE=0

### E) Zaehlungen
fragen.json offen: 8
fragen.json beantwortet: 42
Registerzeilen NAK-: 95
hoechste Registernummer: NAK-109
```

**Was die Läufe zeigen.**

- **A** `planstand.py` endet mit **Exitcode 0 und ohne Warnung**. Der erste Lauf am
  30.08. endete mit **Exitcode 4**: der Kopf dieses Manifests trug einen HTML-Kommentar,
  in dem das Wort der Urteilsmarke vorkam; `MARKE_ROH` (`tools/plan/planstand.py:65`)
  fängt jeden Kommentar mit diesem Wort ab und meldet ihn fail-closed als unlesbare
  Marke. Der Riegel hat also genau das getan, wofür er gebaut ist. Der Kommentar ist
  durch einen normalen Merksatz ersetzt; PR1 wird bis zum Urteil des Dirigenten
  korrekt als *gebaut, nicht abgenommen* geführt.
- **B** `antworten_blatt.py`: 42 Antworten, 0 offen, 42 eingearbeitet — `beantwortet{}`
  ist unverändert, wie die Ticketgrenze es verlangt.
- **C** Beide Planquellen sind nach den Edits gültiges JSON. Zusätzlich hat jedes
  Edit-Skript vor dem Schreiben geprüft, dass `json.dumps(…, indent=1,
  ensure_ascii=False)` mit CRLF **bytegleich** zur Ausgangsdatei ist — die Edits ändern
  damit nur Text, keine Formatierung.
- **D** Der Änderungssatz berührt **ausschließlich `docs/**`**: sieben Dateien,
  854 Zeilen dazu, 34 weg. Kein Produktcode, kein Test, kein Schema, kein Fixture,
  kein Runner. `docs/ANTWORTEN-OFFEN.md` und `docs/PLAN-STAND.md` sind erzeugt, nicht
  von Hand editiert.
- **E** Die Zählungen aus §1.3 nach dem Änderungssatz: `offen[]` wächst von 2 auf **8**
  (U15–U20 neu), `beantwortet{}` bleibt bei **42**, das Register wächst von 90 auf **95**
  Zeilen, höchste Nummer **NAK-109**.

**Nachlauf nach dem Beweislauf (lebender Kopf nachgezogen, Prüfliste E).** Auf `805e108`
folgten zwei weitere Commits an diesem Ticket, beide dokumentiert und beide wieder nur
unter `docs/**`: `1663895` zieht §6 und §8 dieses Manifests nach, `c994381` ist das
Selbstaudit (fünf Herstellerquellen erstmalig einzeln abgerufen statt aus einer
Trefferliste zitiert, zwei Kartenaussagen ohne Beleg entschärft, Zeilenversatz benannt).
Dazwischen und danach je ein gerechneter Planstand. Die Zahlen im Lauf D oben gelten
deshalb für den Änderungssatz aus §6.1, nicht für den Stand von HEAD; der Umfang der
Grenze ändert sich dadurch nicht — kein Commit dieses Tickets berührt etwas außerhalb
von `docs/**`.

### Warum der Kanon nicht läuft

`tools/beweise.ps1` baut und misst Produktcode, Tests, Schemas und Fixtures.
Dieser Änderungssatz berührt ausschließlich `docs/**` (Beleg: der
`git diff --stat` in §8, Lauf D). Ein Kanon-Lauf würde denselben Stand messen wie der
letzte Lauf zu `SONDE-010` und keine Aussage über dieses Ticket treffen. Er wird
deshalb bewusst **nicht** gefahren und auch nicht als Beleg zitiert.

---

## 9. Runde 1 — Nacharbeit nach Codex-Erstprüfung (30.08.2026)

**Append-only.** Dieser Abschnitt schreibt §1–§8 nicht um; Korrekturen dort stehen
als datierte Nachträge an ihrer Stelle. Der lebende Kopf (§1.3, §1.4) ist nachgezogen.

### 9.1 Die Erstprüfung

| Feld | Wert |
|---|---|
| Prüfer | Codex `gpt-5.6-sol`, Effort **xhigh**, **lesend** |
| Thread | `01a053a7-d1dc-7441-8554-44d0fa14cc8f` |
| Datum | 30.08.2026 |
| Stand vor der Runde | `0e3908ee7a342e8d3d4c88f811e8f2ec011ae926` (Ticketbasis `6f40eed5e5e3969df2ac08232011247ff377e89c`, Branch `master`) |
| Prüfbereich | `git diff 6f40eed...0e3908e -- docs/beweise/PR1.md docs/plan/plan.json docs/plan/fragen.json docs/bauaufteilung-sonden.md docs/FL-Nakama-Sonden-Design-Entwurf.md docs/offene-punkte.md docs/PLAN-STAND.md docs/ANTWORTEN-OFFEN.md` |
| Urteil | **NEEDS_WORK**, sieben Befunde, alle **[P1]** |
| Nachmessung | Der Dirigent hat alle sieben an der Quelle nachgemessen und als **Defekt** bestätigt |

Diese Runde schließt genau diese sieben Befunde — nichts sonst. Die Ticketgrenze ist
unverändert: schreibend nur `docs/beweise/PR1.md`, `docs/plan/plan.json`,
`docs/plan/fragen.json` (nur `offen[]`), `docs/bauaufteilung-sonden.md`,
`docs/FL-Nakama-Sonden-Design-Entwurf.md` (nur Errata/Nachträge),
`docs/offene-punkte.md`, sowie `docs/PLAN-STAND.md` und `docs/ANTWORTEN-OFFEN.md`
über ihre Generatoren. Kein Produktcode, kein Schema, kein Test, kein Fixture,
nichts unter `tools/**`, `design/abnahmen/` oder `.claude/**`.

---

### 9.2 Die sieben Befunde — Urteilstext, Regel und Ausführung

---

#### B1 — [P1] Übernimm den bindenden NAK-79-Entscheid — `docs/plan/plan.json:195`

**Urteilstext der Erstprüfung, wörtlich:**

> Wenn SONDE-011 nach diesem Text gebaut wird, wird erneut entschieden, welche Produktklasse `post_fader_contribution` tragen darf. `rg -n '^\| NAK-79' docs/offene-punkte.md` zeigt jedoch den datierten User-Wortlaut und das bindende Ergebnis: Keine Sondenklasse trägt die Position; der Beitrag ist eine Gen-Eingangsachse, nur ihre Vertragsform bleibt Technikentscheidung. Der Edit widerspricht damit einem Entscheid und verletzt den Gate-Satz, alle Entscheide sauber einzuarbeiten; nach `CLAUDE.md:54-55` ist dieses datierte Registerzitat bindend.

**Nachmessung.** `docs/offene-punkte.md`, Zeile NAK-79, trägt den User-Nachtrag vom
24.08.2026 im Wortlaut: „das prinzip ist doch jeweils eine sonde auf ein bus -> Gen,
wieso sollte eine sonde als empfänger genutzt werden?" und „EINE Sonde kann nur dort
messen wo sie steckt. und sie kann nur dort EQ anwenden durch GEN wo sie steckt. das
ist kein prinzip das ist die einzige realität." Ergebnis der Zeile: **keine
Sondenklasse trägt die Position, Empfänger ist Gen.** Derselbe Nachtrag benennt, was
offen bleibt: „eine reine Vertragsfrage und meine Entscheidung … Vorschlag: ein
Deskriptor je Gen-Eingang statt eines Wertes im Instanzfeld." Der Befund ist **Defekt**.

**Regel des Dirigenten.** S16–17 und die Bauaufteilungszeile S16–17 so umschreiben,
dass die Produktfrage nicht erneut gestellt wird; S16–17 trägt nur die Technikfrage aus
dem NAK-79-Nachtrag. Die Lücke L-1 in §4.2 bekommt einen datierten Nachtrag. **Entscheid
des Dirigenten (Technik):** der Deskriptorname je Gen-Eingang wird in der v3-Vertragsrunde
von S16–17 reserviert, die Nutzlast entsteht mit `SONDE-018`.

**Ausgeführt.**

| Stelle | Vorher | Nachher |
|---|---|---|
| `docs/plan/plan.json`, Schritt `S16–17` (Edit **R1-1**) | „(1) NAK-79 — welche Produktklasse die Messposition post_fader_contribution führen darf, wenn es den Aux-Bus gibt. Heute ist sie seit der G1-Nacharbeit für keine Klasse erlaubt (fail-closed), und der Zustandsvertrag nennt SONDE-011 als Eigentümer der Antwort." | Reservierung (a) der v3-Vertragsrunde: „Der Entscheid vom 24.08.2026 im NAK-79-Nachtrag ist bindend und wird NICHT erneut gestellt: keine Sondenklasse trägt die Position, eine Sonde misst an ihrem Slot, Empfänger ist Gen. Offen ist allein die Vertragsform — der Beitrag ist eine Achse „aus welchem Gen-Eingang stammt die Messung" und kein Instanzattribut; das Instanzfeld bleibt fail-closed (beide Riegel in positionErlaubt bleiben zu), und die Nutzlast entsteht mit dem diskreten Aux-Bus in SONDE-018." |
| `docs/bauaufteilung-sonden.md` §3, Zeile `S16–17` (Edit **R1-5**) | Zeile nannte NAK-79 nicht | „ein Deskriptor je Gen-Eingang für `post_fader_contribution` (NAK-79 ist am 24.08. entschieden — keine Sondenklasse trägt die Position, Empfänger ist Gen; das Instanzfeld bleibt fail-closed, Nutzlast mit dem Aux-Bus in `SONDE-018`)" |
| `docs/offene-punkte.md`, Zeile NAK-79 | ohne Runde-1-Nachtrag | datierter Nachtrag: Produktfrage entschieden und wird nicht wieder gestellt; Technikentscheid des Dirigenten benannt |
| `docs/beweise/PR1.md` §4.2, Lücke **L-1** | „Edit E-15 (S16–17 trägt die Frage als Lieferumfang)" | datierter Nachtrag: „die Lücke war falsch gestellt"; E-15 umgeschrieben auf die Vertragsform |

**Geschlossen.** Die Produktfrage steht in keiner Planquelle mehr:
`grep -c 'welche Produktklasse' docs/plan/plan.json` = **0**.

---

#### B2 — [P1] Plane die offenen Vertragsriegel vor G2 ein — `docs/beweise/PR1.md:225`

**Urteilstext der Erstprüfung, wörtlich:**

> Diese Sammelbewertung lässt mehrere weiterhin offene, für den nächsten Datenweg relevante Registerpunkte aus: `rg -n '^\| NAK-(10|28|29|40|59) ' docs/offene-punkte.md` zeigt den fehlenden Hörmarker-Verbrauch, den Gegenpfad zu `subscribe_session`, unvollständige Transportstempel-Validierung, die nicht abbildbare Legacy-UUID und fehlendes Band-Stereo im Binärvertrag. Ihre bisherigen Eigentümertickets sind abgeschlossen oder S16–17 verbraucht die Daten als Nächstes; ohne neue Zuordnung friert G2 bekannte Vertragsbrüche ein. Das verletzt den Gate-Auftrag zur Prüfung der gebauten Tickets und Verträge sowie die Lebenszyklusregel `AGENTS.md:48-51`.

**Nachmessung.** Alle fünf stehen in der Tabelle **Offen** von `docs/offene-punkte.md`.
NAK-40 nennt `SONDE-010` als Entscheider; gemessen: `grep -c 'NAK-40'
docs/beweise/SONDE-010.md` = **0** — das Ticket hat die Frage nie entschieden.
NAK-29 ist im **Erzeuger** mit SONDE-009 geschlossen worden, der v3-Schema-Schnitt
ausdrücklich nicht. Der Befund ist **Defekt**.

**Regel des Dirigenten (Technik, entschieden 30.08.2026).** `SONDE-011` beginnt mit einer
**v3-Vertragsrunde vor R0** als erstem benannten Lieferumfang — das ist Vorschlag V-1 aus
§6.3, jetzt entschieden. Sie umfasst NAK-28, NAK-29, NAK-40, NAK-59 und NAK-10, dazu die
Reservierungen aus B1, B3 und B4 und die optionalen hostgelieferten Felder aus D-2. Jede
der fünf Registerzeilen bekommt einen datierten Nachtrag, angehängt statt umgeschrieben.

**Ausgeführt.**

| Stelle | Was |
|---|---|
| `docs/plan/plan.json` `S16–17` (**R1-1**) | Die v3-Vertragsrunde ist Punkt (1) und damit der erste benannte Lieferumfang; alle fünf Registerpunkte stehen mit ihrem konkreten Schnitt im Text, dazu die vier reservierten Namen |
| `docs/bauaufteilung-sonden.md` §3, `S16–17` (**R1-5**) | dieselbe Zeile nachgezogen, mit allen fünf Registernummern und den vier Reservierungen |
| `docs/offene-punkte.md`, NAK-10 / NAK-28 / NAK-29 / NAK-40 / NAK-59 | je ein datierter Nachtrag „Eigentümer S16–17 v3-Vertragsrunde (PR1 Runde 1)", angehängt; NAK-40 zusätzlich mit der gemessenen Trefferzahl 0 aus `SONDE-010.md` |
| `docs/beweise/PR1.md` §3.3 | datierter Nachtrag mit allen fünf Fundstellen als eigene Tabelle: was offen ist, bisheriger Eigentümer, neuer Eigentümer |
| `docs/beweise/PR1.md` §6.3 | V-1 als **entschieden am 30.08.2026** markiert; der Vorschlagstext bleibt als Verlauf |

**Reservierte Namen der Runde (ohne Nutzlast).** Nach der Regel aus
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`: „der Vertragsanteil, der später
bricht, ist der NAME — nicht die Nutzlast."

| # | Name | Woher | Eigentümer der Nutzlast |
|---|---|---|---|
| a | Deskriptor je Gen-Eingang für `post_fader_contribution` | B1 / NAK-79 | `SONDE-018` (Aux-Bus) |
| b | `state_report.dsp` und die Bestätigung der angewandten Werte | B3 / NAK-110 | S26–28 (DTO), S29–31 (Bestätigung) |
| c | Betriebszustand des Sonden-EQ | B4 / NAK-107 | S29–31, **ohne Anzeigezusage** |
| d | optionale hostgelieferte Felder für Bus-Name und Mixer-Index im `probe_descriptor` | D-2 / NAK-106 | S18–19 misst, ob FL sie füllt |

---

#### B3 — [P1] Plane den DSP-Zustandsrückweg vor G2 — `docs/beweise/PR1.md:419-423`

**Urteilstext der Erstprüfung, wörtlich:**

> Nach Project-Reload oder Reconnect kann Main den autoritativen Probeeq-Zustand nicht rekonstruieren: `state_report` in `eq-ipc-v3.schema.json:993-1015` überträgt nur Version, Revision, Hash, Record-State und Undo-Tiefe, und erfolgreiche `command_ack`-Antworten ebenfalls keine tatsächlich angewandten Werte. Das widerspricht Entwurf §33.4/§33.5, wonach Main den bestätigten DSP per `state_report` liest und Apply die tatsächlichen Werte bestätigt; ein Hash kann Bänder, Bypass, Mix und Schutzbereiche weder anzeigen noch konfliktfrei wiederherstellen. D-5 plant nur ein EQ-An/Aus-Feld und lässt diesen umfassenderen Vertragsbruch vor G2 unerfasst (`CLAUDE.md:122-123,139-140`).

**Nachmessung.** `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`: `state_report` ist
`additionalProperties: false` und führt genau `type`, `adresse`, `dsp_schema_version`,
`state_revision`, `state_hash`, `record_state`, `undo_tiefe` — keine DSP-Nutzlast. Alle
fünf `command_ack`-Zweige (`angewandt`, `abgelehnt`, `konflikt`, `abgelaufen`,
`idempotent_wiederholt`) tragen nur `command_id`, `ergebnis`, `state_revision`,
`state_hash` und `code`. Der Entwurf verlangt beides: §33.5 (`:1898`) „bestätigter DSP
samt Schutzgrenzen … Broker/Main lesen per `state_report`, überschreiben nie blind" und
§33.4 (`:1881`) „bestätigt `state_hash` plus tatsächlich angewandte Werte". In
`reservierte-nachrichten-v1.json` ist dafür nichts reserviert — das DSP-DTO ist dort nur
`apply_transaction` (`SONDE-016`) zugeordnet, und `reservierte_felder` enthält genau
einen Eintrag (`evidence_snapshot.ereignisse`). Der Befund ist **Defekt**.

**Regel des Dirigenten.** Neue Registerzeile (Defektklasse **Vertrag**); in der
v3-Vertragsrunde von S16–17 wird `state_report.dsp` reserviert und die Bestätigung der
angewandten Werte für `apply_transaction`/`command_ack` benannt; die Nutzlast entsteht mit
dem DSP-DTO in S26–28, die Bestätigung in S29–31. D-5 bekommt einen datierten Nachtrag;
§4.1 wird nicht umgeschrieben.

**Ausgeführt.**

| Stelle | Was |
|---|---|
| `docs/plan/plan.json` `S16–17` (**R1-1**, Reservierung b) | Name `state_report.dsp` reserviert, dazu der Name für die Bestätigung der tatsächlich angewandten Werte in `apply_transaction`/`command_ack` |
| `docs/plan/plan.json` `S26–28` (**R1-3**) | neuer Nachtrag: die **Nutzlast** entsteht hier mit dem DSP-DTO, mit der gemessenen Feldliste von `state_report` als Begründung |
| `docs/plan/plan.json` `S29–31` (**R1-4**) | neuer Nachtrag: Apply bestätigt die tatsächlich angewandten Werte, nicht nur den Hash — mit dem §33.4-Zitat und der gemessenen Zweigliste von `command_ack` |
| `docs/bauaufteilung-sonden.md` §3, Zeilen `S16–17`, `S26–28`, `S29–31` (**R1-5**, **R1-7**, **R1-8**) | dieselben drei Zeilen nachgezogen |
| `docs/offene-punkte.md` | **neue Zeile NAK-110** mit Fundstellen, Folge und Technikentscheid |
| `docs/beweise/PR1.md` §4.1, Befund **D-5** | datierter Nachtrag (2): „der Vertragsbruch ist größer als ein EQ-An/Aus-Feld"; §4.1 selbst unverändert |

---

#### B4 — [P1] Beschränke den EQ-Punkt auf die Rückfallfläche — `docs/plan/plan.json:218`

**Urteilstext der Erstprüfung, wörtlich:**

> Bei Umsetzung von S18–19 würde diese Zeile einen EQ-An/Aus-Status in Gens passiver Landkarte verlangen. Der zitierte Entscheid in `design/abnahmen/2026-08-23-interview-struktur.md:139-142,223-226` bindet den roten/grünen EQ-Punkt jedoch ausdrücklich an Probeeqs lokale Rückfallfläche; er verlangt keine solche Anzeige auf Gens Landkarte. Die Erweiterung erfindet damit eine sichtbare Funktion und erzwingt dafür unnötig eine Versionierung des strikten Descriptors, statt den Entscheid an seiner tatsächlichen UI-Stelle umzusetzen (`CLAUDE.md:54-55`).

**Nachmessung.** `design/abnahmen/2026-08-23-interview-struktur.md`, **Frage 13**
(„Runde 4 — die letzten Buttons", Überschrift „BYPASS ↔ EQ-Mode-Punkt") und
**Festlegung 16** („Rückfallfläche präzisiert: Connected/Disconnected · Bypass · bei EQ
on die Werte + der Mode der Probe") betreffen beide die **Rückfallfläche der Sonde**.
Eine Anzeige auf Gens passiver Landkarte steht dort nicht. Der Befund ist **Defekt** —
und er trifft zugleich das Gesetz „keine toten UI-Elemente" von der anderen Seite: eine
Anzeige ohne Entscheid ist eine Zusage, die niemand verlangt hat.

**Regel des Dirigenten.** Satz (2) aus `plan.json:218` (Edit E-11) und den entsprechenden
Teil der Bauaufteilungszeile S18–19 entfernen; keine sichtbare Landkartenfunktion. Was
bleibt, ist Technik: für die Kopplung in S29–31 muss Gen den Betriebszustand kennen —
dieses Vertragsfeld gehört als Reservierung in die v3-Vertragsrunde mit Eigentümer S29–31,
ohne Anzeigezusage. NAK-107 datiert nachtragen.

**Ausgeführt.**

| Stelle | Vorher | Nachher |
|---|---|---|
| `docs/plan/plan.json` `S18–19` (**R1-2**) | „(2) Die Landkarte zeigt je Sonde, ob ihr EQ zugeschaltet ist (Entscheid 23.08., Rückfallfläche: EQ-Punkt grün oder rot); heute kann kein Vertragsfeld das ausdrücken …" | Satz gestrichen; an seiner Stelle die datierte Berichtigung, warum, und der Verweis auf die Reservierung mit Eigentümer S29–31. Der Zählsatz „fehlen zwei Wahrheiten" ist im selben Zug auf „fehlten nach dem ersten Durchgang zwei; nach der Berichtigung bleibt eine" berichtigt, damit die Zahl nicht gegen den Inhalt steht |
| `docs/bauaufteilung-sonden.md` §3, `S18–19` (**R1-6**) | „…; dazu der EQ-Zustand je Sonde als anzeigbare Wahrheit." | Teilsatz gestrichen, datierte Berichtigung angehängt, Feld als Reservierung mit Eigentümer S29–31 benannt |
| `docs/plan/plan.json` `S29–31` (**R1-4**) | — | der Betriebszustand wird hier geführt, ausdrücklich „Technik ohne Anzeigezusage" |
| `docs/offene-punkte.md`, NAK-107 | „Eingearbeitet 30.08.2026: der EQ-Zustand je Sonde steht als anzuzeigende Wahrheit in plan.json S18–19." | datierter Nachtrag: Landkarten-Anzeige gestrichen, Feld bleibt Technik. Der alte Satz bleibt als Verlauf stehen |
| `docs/beweise/PR1.md` §4.1, **D-5** | — | datierter Nachtrag (1) mit der Fundstelle des Entscheids |

**Geschlossen.** Kein Planquellentext verlangt die Anzeige mehr:
`grep -c 'dazu der EQ-Zustand je Sonde als anzeigbare Wahrheit'
docs/bauaufteilung-sonden.md` = **0**; der Begriff kommt dort nur noch im Zitat der
Berichtigung selbst vor. In `plan.json` steht der gestrichene Satz nicht mehr.

---

#### B5 — [P1] Teile U17 in einzelne Button-Entscheide auf — `docs/plan/fragen.json:57-60`

**Urteilstext der Erstprüfung, wörtlich:**

> U17 erklärt fünf unabhängige Funktionen ausdrücklich zu einem Entscheid und fragt sie gemeinsam ab. Der zitierte User-Entscheid in `design/abnahmen/2026-08-23-interview-struktur.md:92-96,210-211` verlangt dagegen eine Klärung „button für button" und verwirft „so halb halb"; eine Sammelantwort kann daher nicht festlegen, welche einzelnen Funktionen gewollt sind. Die Karte muss getrennte Antworten ermöglichen, sonst bricht der Edit genau den Entscheid, den seine Begründung zitiert.

**Nachmessung.** Die Karte trug `art: "Produktentscheid (mehrere kleine Handgriffe, ein
Entscheid)"` und im `warum` den Satz „vier fehlende Handgriffe auf einmal statt vier
Einzelfragen". Der zitierte Entscheid sagt das Gegenteil: Interview 23.08., Frage 7 —
„das müssen wir gesondert button für button machen … so halb halb bringt das nix",
Festlegung 10 — „Die Überladen-Grenze wird Button für Button geklärt". Der Befund ist
**Defekt**.

**Regel des Dirigenten.** U17 bleibt **eine** Karte — der Skill `/fragen` kennt
Folgefragen je Karte, Vorbild U2/U6 (`.claude/skills/fragen/SKILL.md`: „Mehrere
Folgefragen einer Karte (U2, U6) → ein `AskUserQuestion`-Aufruf mit bis zu vier Fragen;
sind es mehr, in derselben Runde aufteilen. Das ist die eine erlaubte Ausnahme von ‚eine
Frage je Runde': es ist eine Karte."). Sie wird zu fünf getrennt beantwortbaren
Einzelentscheiden U17.1–U17.5 umgebaut. Die Lautheitsanzeige ist U17.5.

**Ausgeführt** (Edit **R1-9**, `docs/plan/fragen.json`, Karte `U17`):

| Feld | Vorher | Nachher |
|---|---|---|
| `art` | „Produktentscheid (mehrere kleine Handgriffe, **ein Entscheid**)" | „**fünf Einzelentscheide, Button für Button**" |
| `titel` | „Was soll das Kurvenbild außer der Kurve können?" | „Fünf einzelne Handgriffe fürs Kurvenbild — jeder für sich Ja oder Nein" |
| `was` | Fließtext „Vier Handgriffe … Dazu eine fünfte Frage" | nummerierte Liste **U17.1** Spektrum greifen · **U17.2** Bild festhalten · **U17.3** Anzeige einstellbar · **U17.4** Tonnamen · **U17.5** Lautheit sichtbar. Je Punkt eine eigene Ja/Nein-Frage und ein Satz Folge mit Oberfläche und Ticket |
| `warum` | „vier fehlende Handgriffe auf einmal statt vier Einzelfragen" | „Deshalb steht hier jeder Punkt für sich mit einer eigenen Ja/Nein-Frage; eine Sammelantwort gibt es bewusst nicht." Dazu: wenn ein Punkt abgelehnt wird, bleibt er draußen |
| `wo` | Tickets | zusätzlich: „Beim Festhalten wird jede der fünf Antworten als eigener Eintrag U17.1 bis U17.5 abgelegt — Vorbild U2 und U6; eine Sammelantwort wird nicht geschrieben." Dazu die datierte Notiz, warum umgebaut wurde |

Kein Satz erklärt die fünf mehr zu einem Entscheid; Wortlaut durchgehend Klartext ohne
unübersetztes Fachwort. Die Ticketzuordnung ist getrennt: U17.1–U17.4 → S31b,
U17.5 → S18–19.

---

#### B6 — [P1] Belege jede Marktzuordnung mit einer Quelle — `docs/beweise/PR1.md:548`

**Urteilstext der Erstprüfung, wörtlich:**

> Mehrere Herstelleraussagen reichen über die eigene Quellenliste hinaus: Für TDR wird hier gar keine Quelle geführt; H8 belegt laut §2.2 Spektrum-Export, aber nicht die Instanzliste aus §5.1; U18 behauptet Presets und Kopieren bei jedem Vergleichsprodukt, obwohl §5.3 nur H3/H8 nennt; und U19 schreibt iZotope EQ-Matching zu, während §5.4 dafür nur H1/H6 anführt. Vor einer Produktentscheidung anhand dieser Karten müssen die Aussagen durch konkrete Herstellerquellen gedeckt oder entsprechend eingegrenzt werden; andernfalls ist der Gate-Satz „Funktionsvergleich anhand aktueller Herstellerdokumentation" verletzt.

**Nachmessung.** Alle vier Stellen bestätigt. Der Befund ist **Defekt**.

**Zwei Herstellerseiten am 2026-08-30 einzeln abgerufen** — nur Herstellerseiten, keine
Händler-, Test- oder Forenseiten:

| # | Quelle | URL | Wörtlicher Beleg |
|---|---|---|---|
| **H13** | Tokyo Dawn Records — TDR Nova | `https://www.tokyodawn.net/tdr-nova/` | „With its four dynamic EQ bands and additional high-pass and low-pass filter sections, NOVA can meet the most exotic demand"; „each band also includes a full featured dynamics section" |
| **H14** | iZotope Ozone — Features (Match EQ) | `https://www.izotope.com/en/products/ozone/features.html` | „Match to any reference track with over 8,000 separate bands to get the most precise snapshot possible" |

**Ausgeführt, Stelle für Stelle.**

| Stelle | Urteil | Vorher | Nachher |
|---|---|---|---|
| **(a)** §5.3, Zeile „Dynamische Bänder" | belegt | „Pro-Q 4 (H3); TDR/Neutron" — TDR ohne Quelle, Neutron ohne Nummer | „Pro-Q 4 (H3); TDR Nova (**H13**, nachgereicht 30.08.2026 …); Neutron 5 (H5)" |
| **(b)** §5.1, Zeile „Alle Instanzen der Sitzung in einer Liste sehen" | **gestrichen** | „Pro-Q 4 (H1), smart:EQ 4 bis 10 pro Gruppe (H6), MMultiAnalyzer (H7), SPAN Plus (H8)" | „Pro-Q 4 (H1), smart:EQ 4 bis 10 pro Gruppe (H6) · ~~MMultiAnalyzer (H7)~~ ~~SPAN Plus (H8)~~ gestrichen" — die Voxengo-Feature-Seite führt keine Instanzliste (sie nennt „Multi-track spectrum import/export"), und H7 belegt nur, dass Instanzen sich finden und Analysen tauschen |
| **(c)** Karte **U18**, `warum` | eingegrenzt | „Voreinstellungen und Kopieren gehören bei **jedem** verglichenen Produkt zur Grundausstattung." | „Bei den beiden verglichenen Produkten, deren Herstellerseiten das ausdrücklich führen — FabFilter Pro-Q 4 („Undo, Redo, A/B and Copy" und der Preset-Browser) und Voxengo SPAN Plus („Preset manager", „Undo/redo history", „A/B comparisons") — … für die übrigen sechs Hersteller haben wir dazu keine Herstellerseite gelesen und behaupten es deshalb nicht." |
| **(d)** Karte **U19**, `warum` | belegt | „iZotope hat es ebenfalls." | „iZotope führt es in Ozone als Match EQ („Match to any reference track with over 8,000 separate bands", Herstellerseite abgerufen 30.08.2026)" |

**Drei Stellen derselben Klasse ohne eigenen Codex-Befund, im selben Zug behoben** —
weil sie sonst als dieselbe Klasse stehen geblieben wären:

1. §5.1, Zeile „Spektrum einer anderen Spur im eigenen Graph": das Zitat `SPAN Plus
   „Export To" (H8)` steht so **nicht** auf der Herstellerseite. Berichtigt auf den
   tatsächlichen Wortlaut „Multi-track spectrum import/export", mit Datum.
2. §5.4, Zeile „Referenz-/Zielkurve übernehmen (EQ Match)": um H14 ergänzt, damit die
   Karte U19 und die Tabelle dieselbe Quelle nennen.
3. Karte **U19**, `warum`: der Satz „Es ist die **einzige** verbreitete Funktion, bei der
   Nakama deutlich später dran ist als der Markt" wird von der eigenen Tabelle widerlegt
   (§5.3 „Voreinstellungen … fehlt als eigene Funktion", §5.4 vier fehlende Handgriffe).
   Berichtigt auf „die Funktion, bei der Nakama am weitesten hinter dem Markt liegt —
   nicht die einzige fehlende", mit Verweis auf U18 und U17.

§2.2 trägt beide neuen Quellen und einen datierten Nachtragssatz: der alte Satz „alle
zwölf Seiten" bleibt als Verlauf stehen, der Nachtrag schreibt ihn auf **vierzehn** fort
und benennt, welche zwei Zuordnungen nicht belegt, sondern gestrichen wurden.

---

#### B7 — [P1] Ordne den Visual-Mixer-Workflow ein — `docs/beweise/PR1.md:125`

**Urteilstext der Erstprüfung, wörtlich:**

> H5 führt den Neutron Visual Mixer selbst als belegte Funktion auf, aber der gesamte Vergleich §5 enthält dafür weder Registerzeile noch Fragenkarte noch eine Aussage, wie Nakama den zentralen Mehrinstanz-Mixworkflow löst. Dieser Workflow ist nicht mit dem EQ-Dry/Wet-Regler gleichzusetzen und fällt in die ausdrücklich geprüfte Kategorie „Mix-Analyse mit Sonden". Der Gate-Auftrag verlangt mindestens eine Einordnung als vorhanden, anders gelöst, außerhalb des Umfangs oder fehlend samt Auswirkung auf Vertrag, State und Oberfläche.

**Nachmessung.** §5.1 hat keine Zeile dazu; H5 in §2.2 nennt den Visual Mixer
ausdrücklich als belegte Funktion. Der Befund ist **Defekt**.

**Einordnung des Dirigenten (30.08.2026): außerhalb des Produktumfangs.** Nakama regelt
Klang über EQ — der Wahrheitskern in `CLAUDE.md` nennt Quellen, Befunde, Advisor und die
EQ-Zentrale. Pegel und Pan aller Spuren aus einem Fenster zu stellen ist kein
Nakama-Versprechen; Vertrag, State und Oberfläche sind nicht betroffen. Keine Fragenkarte,
weil es kein Planbruch ist, sondern eine Grenze.

**Ausgeführt.**

| Stelle | Was |
|---|---|
| `docs/beweise/PR1.md` §5.1 | neue Zeile „Pegel und Panorama aller Spuren aus einem Fenster (Mehrinstanz-Mixworkflow)", Beleg H5, Einordnung „außerhalb des Produktumfangs", Spalte *betroffen* ausdrücklich leer mit Begründung |
| `docs/offene-punkte.md` | **neue Zeile NAK-111** (Klasse Lücke): falls der User Pegel-/Pan-Steuerung will, ist das ein Produktentscheid mit eigener v3-Nachrichtenfamilie samt Lease, Rückweg und Schutzregeln — nach G2 eine Versionierung, keine Ergänzung |

---

### 9.3 Änderungssatz der Runde

**Edits an Planquellen (R1-1 bis R1-11).**

| # | Datei | Stelle | Befund |
|---|---|---|---|
| R1-1 | `docs/plan/plan.json` | `S16–17` | B1, B2, B3, B4 |
| R1-2 | `docs/plan/plan.json` | `S18–19` | B4 |
| R1-3 | `docs/plan/plan.json` | `S26–28` | B3 |
| R1-4 | `docs/plan/plan.json` | `S29–31` | B3, B4 |
| R1-5 | `docs/bauaufteilung-sonden.md` | §3, Zeile `S16–17` | B1, B2, B3, B4 |
| R1-6 | `docs/bauaufteilung-sonden.md` | §3, Zeile `S18–19` | B4 |
| R1-7 | `docs/bauaufteilung-sonden.md` | §3, Zeile `S26–28` | B3 |
| R1-8 | `docs/bauaufteilung-sonden.md` | §3, Zeile `S29–31` | B3, B4 |
| R1-9 | `docs/plan/fragen.json` | `offen[]`, Karte `U17` | B5 |
| R1-10 | `docs/plan/fragen.json` | `offen[]`, Karte `U18` (`warum`) | B6 |
| R1-11 | `docs/plan/fragen.json` | `offen[]`, Karte `U19` (`warum`) | B6 |

**Neue Registerzeilen.** **NAK-110** (B3, Klasse Vertrag) und **NAK-111** (B7, Klasse
Lücke). Fortlaufend nach der höchsten vorhandenen Nummer NAK-109.

**Datierte Nachträge an vorhandenen Registerzeilen** (angehängt, nie umgeschrieben):
NAK-10, NAK-28, NAK-29, NAK-40 (zwei Nachträge), NAK-59, NAK-79, NAK-107 — sieben Zeilen.

**Karten.** Eine umgebaut (U17 → fünf Einzelentscheide U17.1–U17.5), zwei im `warum`
berichtigt (U18, U19). Keine neue Karte, keine geschlossene Karte; `beantwortet{}`
unangetastet.

**Nicht angefasst.** `docs/FL-Nakama-Sonden-Design-Entwurf.md` — keiner der sieben
Befunde verlangt einen Errata-Eintrag; die Entwurfsstellen §33.4/§33.5 sind die
**Anforderung**, gegen die der Vertrag gemessen wurde, und bleiben unverändert richtig.

---

### 9.4 Beweislauf

Gefahren am 2026-08-30 vom Workspace-Root `C:/Users/phili/Projekte/Nakama` auf dem Stand
`2397790` (Planedits + Register + Karten committet, Manifest noch offen). Rohausgabe
ungekürzt:

```text
### A) py -3.13 tools/plan/planstand.py
geschrieben: docs\PLAN-STAND.md (18 abgenommen, 2 gebaut, 38 gesamt, aus 2397790)
EXITCODE=0

### B) py -3.13 tools/plan/antworten_blatt.py
geschrieben: docs\ANTWORTEN-OFFEN.md (42 Antworten, 0 offen, 42 eingearbeitet, 13 mit eigenem Text)
EXITCODE=0

### C) JSON-Gueltigkeit
docs/plan/plan.json -> gueltiges JSON 39714 Zeichen
docs/plan/fragen.json -> gueltiges JSON 28485 Zeichen
EXITCODE=0

### D) git diff --stat 0e3908e..HEAD
 docs/bauaufteilung-sonden.md |  8 ++++----
 docs/offene-punkte.md        | 16 +++++++++-------
 docs/plan/fragen.json        | 14 +++++++-------
 docs/plan/plan.json          |  8 ++++----
 4 files changed, 24 insertions(+), 22 deletions(-)
EXITCODE=0

### E) Zaehlungen
fragen.json offen: 8
fragen.json beantwortet: 42
U17 art: fünf Einzelentscheide, Button für Button
U17 Einzelentscheide im was: 5
Registerzeilen NAK-: 97
hoechste Registernummer: NAK-111
Herstellerquellen H1-Hn: 14

### F) Tabellenriegel (jede Markdown-Tabelle durchgehend gleich viele Spalten)
docs/offene-punkte.md:13 TABELLE UNEINHEITLICH: Zeile 22 NAK-74 (5), Zeile 26 NAK-78 (4),
   Zeile 57 NAK-38 (4), Zeile 58 NAK-39 (4), Zeile 101 NAK-101 (5) - Bestand
docs/beweise/PR1.md: sauber
docs/bauaufteilung-sonden.md: sauber
Tabellenriegel: 1 uneinheitliche Tabelle(n)
EXITCODE=1
```

**Was die Läufe zeigen.**

- **A** endet mit **Exitcode 0**. Das Blatt trug in diesem Lauf einen Hinweis, dass unter
  `docs/beweise/` noch uncommittete Änderungen liegen — das war dieses Manifest selbst.
  Der Lauf nach dem Manifest-Commit steht unten in §9.6 und ist ohne diesen Hinweis.
- **B** `beantwortet{}` ist unverändert bei 42 Antworten, 0 offen — die Ticketgrenze hält.
- **C** Beide Planquellen sind nach den Edits gültiges JSON. Zusätzlich hat jedes
  Edit-Skript **vor** dem Schreiben geprüft, dass `json.dumps(…, indent=1,
  ensure_ascii=False)` plus Schluss-Newline mit CRLF **bytegleich** zur Ausgangsdatei ist;
  die Edits ändern damit nur Text, keine Formatierung. Beleg dafür ist der Diff: vier
  geänderte Zeilen in `plan.json` bei vier geänderten Schritten, sieben in `fragen.json`
  bei drei geänderten Karten.
- **D** Der Änderungssatz berührt ausschließlich `docs/**` — vier Dateien, keine
  Zeile Produktcode, Schema, Test, Fixture oder Runner.
- **E** `offen[]` bleibt bei **8** Karten (keine geschlossen, keine neue), `beantwortet{}`
  bei **42**, das Register wächst von 95 auf **97** Zeilen, höchste Nummer **NAK-111**,
  die Quellenliste von 12 auf **14**.
- **F** Ein zusätzlicher Riegel dieser Runde: er prüft, dass jede Markdown-Tabelle in
  den drei geänderten Markdown-Dateien durchgehend gleich viele Spalten trägt — die
  Klasse Fehler, die ein Zellen-Edit mit einem ungeschützten `|` erzeugt. **PR1.md und
  `bauaufteilung-sonden.md` sind sauber.** In `docs/offene-punkte.md` meldet er fünf
  Zeilen: NAK-74, NAK-78, NAK-38, NAK-39 und NAK-101. **Keine davon ist in dieser Runde
  angefasst worden** — sie tragen seit ihrer Entstehung ungeschützte Pipes im Fließtext.
  Alle **neun** in dieser Runde geschriebenen oder ergänzten Registerzeilen (NAK-10, -28,
  -29, -40, -59, -79, -107, -110, -111) haben die korrekten drei Spalten. Der Riegel ist
  bewusst mit Exitcode 1 stehen gelassen und nicht auf grün gedreht: er meldet einen
  echten, vorbestehenden Darstellungsfehler, und ihn hier zu beheben wäre ein
  Neben-Refactor an fremden Registerzeilen.

**Riegel absichtlich gebrochen (Prüfliste E).** Die vier ausführbaren Riegel dieser Runde
sitzen in den Edit-Skripten. Jeder ist einmal gegen einen absichtlich herbeigeführten
Verstoß gefahren worden; Rohausgabe:

```text
--- Riegel 1: Roundtrip-Riegel gegen eine manipulierte Ausgangsdatei ---
ABBRUCH: Roundtrip der Ausgangsdatei ist nicht bytegleich.
EXITCODE=1

--- Riegel 2: beantwortet{}-Riegel gegen eine absichtliche Aenderung ---
ABBRUCH: beantwortet{} wurde veraendert.
EXITCODE=1

--- Riegel 3: Trefferzahl-Riegel der Markdown-Edits gegen einen nicht vorhandenen Anker ---
ABBRUCH: 0 Treffer fuer '| Dynamische Baender | Pro-Q 4 (H3); TDR/Neutron |'
EXITCODE=1

--- Riegel 4: offen[]-Mengenriegel gegen eine geschlossene Karte ---
ABBRUCH: offen[] hat sich in Menge oder Reihenfolge geaendert.
EXITCODE=1
```

Alle vier sind rot geworden, keiner hat den Verstoß durchgelassen. Riegel 2 und 4 sind
genau die Ticketgrenze („`beantwortet{}` nie anfassen, keine Karte schließen"); Riegel 3
verhindert, dass ein Markdown-Edit an einer Stelle landet, die es gar nicht gibt, oder
mehrfach zuschlägt.

**Warum der Kanon nicht läuft.** Unverändert der Grund aus §8: `tools/beweise.ps1` baut
und misst Produktcode, Tests, Schemas und Fixtures. Diese Runde berührt ausschließlich
`docs/**` (Lauf D). Ein Kanon-Lauf würde denselben Stand messen wie der letzte Lauf zu
`SONDE-010` und über diese Runde nichts aussagen.

---

### 9.5 Prüfliste — E und F, je Zeile mit Messort

**A–D sind für ein Dokumentticket weiterhin nicht anwendbar**: kein Rückstauverhalten,
kein Lebenszyklus, kein Längen- oder Alphabetvertrag, kein Bau- oder Prüfriegel im
Änderungssatz (Beleg: Lauf D).

#### E. Behauptung ≤ Messung

| Zeile der Prüfliste | Wo in dieser Runde gemessen |
|---|---|
| „Jede Behauptung … sagt nicht mehr, als der Test misst — und nicht mehr, als das Gate verlangt" | Jede Nachmessung in §9.2 nennt Datei und Fundstelle und wurde an der Quelle gelesen. Negativaussagen tragen ihren Befehl mit Trefferzahl: `grep -c 'NAK-40' docs/beweise/SONDE-010.md` = 0 (B2), `grep -c 'welche Produktklasse' docs/plan/plan.json` = 0 (B1), `grep -c 'dazu der EQ-Zustand je Sonde als anzeigbare Wahrheit' docs/bauaufteilung-sonden.md` = 0 (B4; der Begriff kommt dort nur noch im Zitat der Berichtigung vor). Die zwei neuen Herstellerquellen tragen den wörtlichen Satz, den sie belegen — nicht eine Zusammenfassung davon (B6). |
| „Zahlen im Manifest sind gemessen, nicht abgeschrieben" | §1.3 und §9.4, Lauf E: 8 offene Karten, 42 Antworten, 97 Registerzeilen, NAK-111, 14 Quellen — jede Zahl aus dem danebenstehenden Befehl. Die Zahlen des Erstdurchgangs (90 Registerzeilen, NAK-104, „2 offen vor diesem Ticket") bleiben unverändert stehen: sie sind mit ihrem Stichtag beschriftet. |
| „Positionen im lebenden Kopf stehen als Symbol/Anker; im Verlauf gilt die Zeilennummer zum Stand ihres Abschnitts" | Der lebende Kopf (§1.3/§1.4) nennt keine Zeilennummern, nur Schritt-IDs, Kartenkennungen und Registernummern. Die Zeilennummern in §9.2 sind Zitate aus dem Codex-Urteil und stehen zum Stand `0e3908e`, ausdrücklich so bezeichnet; die stabilen Anker daneben sind Schritt-ID (`S16–17`), Abschnittsnummer (§33.4, §33.5) und Feldname (`state_report`, `command_ack`). |
| „Der lebende Kopf wird beim Abschluss nachgezogen; alles darunter ist append-only" | §1.3 ist auf die Zahlen nach Runde 1 nachgezogen, §1.4 (Rundentabelle) neu. §2–§8 sind unverändert bis auf **datierte Nachträge an Ort und Stelle**; wo eine Aussage falsch war, steht sie durchgestrichen mit der Berichtigung daneben (§5.1) statt still ersetzt. Kein Absatz des Erstdurchgangs ist umgeschrieben worden. |
| „Jede neue Prüfung wurde einmal absichtlich gebrochen; Rohausgabe des Rots liegt bei" | §9.4, Abschnitt „Riegel absichtlich gebrochen": vier Riegel, vier rote Läufe, Rohausgabe im Manifest. |
| „Geänderte Zusage — drei Stellen, kein Inventar" | Drei Zusagen sind geändert worden, alle drei sind in **Plan und Bauaufteilung** nachgezogen: der EQ-Punkt auf der Landkarte (gestrichen, R1-2 + R1-6), die v3-Vertragsrunde (neu, R1-1 + R1-5), der DSP-Rückweg (neu, R1-1/R1-3/R1-4 + R1-5/R1-7/R1-8). Die dritte Stelle — der Runnerkopf in `tools/beweise.ps1` — ist nicht betroffen: keine Runner-Behauptung berührt eine dieser Zusagen (`grep -c 'state_report' tools/beweise.ps1` = 0). |
| „Writer-Fixtures statt Handschrift" | greift nicht — kein Fixture im Änderungssatz. |

#### F. Änderungssatz

| Zeile der Prüfliste | Wo in dieser Runde gemessen |
|---|---|
| „speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg im selben Commit" | Zwei Gegenpaare sind berührt, beide beidseitig. **starten↔stoppen des Brokers:** unverändert aus dem Erstdurchgang in derselben Schrittzeile S16–17 („Starten und Beenden gehören in denselben Änderungssatz, ebenso der Rückweg über den vorhandenen Installer") — der Umbau der Zeile hat den Satz mitgenommen, nicht verloren. **verbinden↔trennen:** NAK-28 kommt in dieser Runde als `unsubscribe_session` in die v3-Vertragsrunde; der Gegenpfad ist damit erstmals einem Ticket zugeordnet und nicht nur benannt. Dazu **speichern↔laden** auf der Vertragsebene: NAK-110 verlangt den DSP-Rückweg (`state_report.dsp`) im selben Ticketzug wie das DSP-DTO, das ihn schreibt — S26–28 baut beides. |
| „Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines Vertrags im selben Änderungssatz" | **Greift als Befund, nicht als Pflicht:** diese Runde ändert keinen Vertrag, sie plant ihn. Genau deshalb ist die v3-Vertragsrunde als **ein** Lieferumfang in S16–17 eingetragen und nicht auf mehrere Tickets verteilt: NAK-29 (Schema mit C++- und Rust-Leser), NAK-59 (`.fbs` mit `flatc`-Codegen, Drift-Test und beiden handgeschriebenen Lesern) und NAK-40 (Adressableitung mit State-Migration) tragen jeweils beide Sprachhälften und ihre Fixtures — sie in getrennten Tickets zu fahren wäre genau der Bruch, den diese Zeile verbietet. Die vier Reservierungen sind bewusst **Namen ohne Nutzlast**: ein Name bricht nichts, eine halbe Nutzlast schon. |

---

### 9.6 Nachlauf nach dem Beweislauf

Auf `2397790` folgen der Manifest-Commit dieser Runde und der gerechnete Planstand, beide
wieder ausschließlich unter `docs/**`. Der Planstand-Lauf auf dem finalen Stand:

```text
### A) py -3.13 tools/plan/planstand.py   (Stand c9eece7, Manifest committet)
geschrieben: docs\PLAN-STAND.md (18 abgenommen, 2 gebaut, 38 gesamt, aus c9eece7)
EXITCODE=0

### B) py -3.13 tools/plan/antworten_blatt.py
geschrieben: docs\ANTWORTEN-OFFEN.md (42 Antworten, 0 offen, 42 eingearbeitet, 13 mit eigenem Text)
EXITCODE=0

### Blatt-Warnung "Gerechnet aus dem Arbeitsbaum"
grep -c aus docs/PLAN-STAND.md: 0
```

Dieser Lauf ist **Exitcode 0 ohne Warnung im Blatt** — der Hinweis aus Lauf A in §9.4
war dieses Manifest im uncommitteten Zustand und ist mit seinem Commit verschwunden.
`docs/ANTWORTEN-OFFEN.md` bleibt byte-gleich: `beantwortet{}` ist unangetastet, wie die
Ticketgrenze es verlangt. Der Umfang der Grenze ändert sich durch den Nachlauf nicht —
kein Commit dieser Runde berührt etwas außerhalb von `docs/**`.

**Commits der Runde** (alle mit explizitem Pathspec, kein `git add -A`, kein `--amend`):

| SHA | Was | Dateien |
|---|---|---|
| `61228c4` | Planedits und Register — B1 bis B4, B7 | `plan.json`, `bauaufteilung-sonden.md`, `offene-punkte.md` |
| `2397790` | Karten — B5, B6 | `fragen.json` |
| `c9eece7` | Manifest — §9, zwei Herstellerquellen, lebender Kopf | `beweise/PR1.md` |
| `ca5a305` | gerechneter Planstand und dieser Nachlauf | `PLAN-STAND.md`, `beweise/PR1.md` |
| letzter | Planstand ohne Arbeitsbaum-Warnung | `PLAN-STAND.md` |

**Zur Reihenfolge, damit die Aussage oben nicht mehr sagt als die Messung.** Der Generator
setzt die Warnung, solange unter `docs/plan/`, `docs/beweise/` oder `tools/plan/` etwas
uncommittet liegt — und dieses Manifest liegt genau dort. Deshalb braucht ein
warnungsfreies Blatt zwingend **zwei** Läufe: einen vor dem Manifest-Commit (der die
Warnung trägt) und einen danach (der sie nicht mehr trägt). Die Rohausgabe oben ist der
zweite Lauf. Der Stand `ca5a305` selbst trug im Blatt noch die Warnung, weil dieser
Nachlaufabschnitt beim Rechnen noch nicht committet war; der letzte Commit der Runde
ist deshalb ein reiner Planstand-Commit, und erst er trägt das warnungsfreie Blatt.
Das ist keine Nachlässigkeit, sondern die einzige Reihenfolge, in der ein Manifest über
seinen eigenen Planstand-Lauf berichten kann.


**Keine Urteilsmarke.** Diese Runde setzt keine `NAKAMA-URTEIL`-Zeile; PR1 bleibt im
gerechneten Planstand *gebaut, nicht abgenommen*, bis der Dirigent nach einer frischen
Prüfung urteilt.

---

## 10. Runde 2 — Nacharbeit nach Wiederprüfung (30.08.2026)

Diese Runde schließt **genau einen** Befund: den Restbefund der Codex-Wiederprüfung zu B6.
Keine andere Fragenkarte, keine Planzeile, keine Registerzeile und kein Absatz aus §1–§9 ist
umgeschrieben worden; Korrekturen in §2.2 und §5.3 stehen als **datierte Nachträge an Ort und
Stelle**. `beantwortet{}` in `docs/plan/fragen.json` ist unangetastet.

### 10.1 Die Wiederprüfung

| Feld | Wert |
|---|---|
| Prüfer | Codex `gpt-5.6-sol`, Effort **xhigh**, **lesend** |
| Thread | `01a053cf-9992-7910-b526-a7712881d9a3` |
| Datum | 30.08.2026 |
| Prüfbereich | `git diff 0e3908e...b54a575 -- <Ticketpfade>` |
| Stand vor dieser Runde | `b54a575d92635e2e6f184cd81c12c023e1d395d5` (Branch `master`) |
| Ticketbasis | `6f40eed5e5e3969df2ac08232011247ff377e89c` |
| Urteil | **NEEDS_WORK** — ein Restbefund, [P1] |

**Urteilstext, wörtlich:**

> B1 ja (`plan.json:195`), B2 ja (`plan.json:195`), B3 ja (`plan.json:195,280,304`), B4 ja
> (`plan.json:218`), B5 ja (`fragen.json:55-61`), B6 nein (`fragen.json:72`), B7 ja
> (`PR1.md:565`). Weitere Entscheid- oder CLAUDE-Brüche wurden nicht gefunden; `beantwortet{}`
> ist unverändert und beide JSON-Dateien laden.
>
> - [P1] Belege das Kopieren zwischen Instanzen — docs/plan/fragen.json:72
>   Wenn U18 als Grundlage für die Produktentscheidung über sondenübergreifendes Kopieren dient,
>   bleibt B6 offen: Die neue Formulierung sagt, bei beiden Produkten seien „Voreinstellungen und
>   Kopieren" Grundausstattung, aber H8 führt für SPAN Plus nur Preset-Manager, Undo/Redo und A/B
>   (`docs/beweise/PR1.md:140`) und belegt kein Kopieren; auch H3s generisches A/B/Copy (`:135`)
>   belegt nicht den in U18 beschriebenen Transfer zwischen Instanzen. Damit ist die Aussage
>   weiterhin nicht durch aktuelle Herstellerdokumentation gedeckt und bricht den PR1-Gate-Satz in
>   `docs/plan/plan.json:187`; bitte die Kopierzuordnung eingrenzen oder eine passende
>   Herstellerquelle ergänzen.

*Zum Zitat:* wörtlich übernommen. Die im Transport ASCII-ersetzten Umlaute (`ae`, `oe`, `ue`) sind
wiederhergestellt; kein Wort, keine Fundstelle und keine Zahl ist geändert.

**Der Dirigent hat den Befund vorab an der Quelle nachgemessen und als Defekt bestätigt.**

### 10.2 Nachmessung, gewählter Weg und Grund

**Eigene Nachmessung vor der Änderung** — jede Zeile an der Quelle gelesen, Stand `b54a575`:

| Was gemessen | Befehl / Ort | Ergebnis |
|---|---|---|
| Behauptung der Karte | `docs/plan/fragen.json:72` | „… gehören Voreinstellungen und Kopieren zur Grundausstattung" — für **beide** Produkte |
| Was H8 im Manifest trägt | `docs/beweise/PR1.md:140` | „Spektrum-Export an andere Instanzen, Kanalgruppen, Korrelation, EBU R128 LUFS, Preset-Manager, Undo/Redo, A/B" — `grep -oi 'copy\|kopier'` auf dieser Zeile: **0** Treffer |
| Was die H8-Herstellerseite trägt | `https://www.voxengo.com/product/spanplus/features/`, erneut abgerufen 30.08.2026 | im Seitentext **0** Treffer für `cop(y\|ies\|ied)`; die Feature-Liste nennt „Preset manager", „Undo/redo history", „A/B comparisons" |
| Was H3 trägt | `docs/beweise/PR1.md:135` | „Undo/Redo/A/B/Copy" — ein Tastenname. H3 sagt nicht, was die Taste tut, und nennt keine zweite Instanz |

Der Befund ist damit **Defekt**: die Karte behauptete mehr, als ihre beiden Quellen tragen.

**Weg 1 zuerst geprüft (Beleg nachreichen).** Die FabFilter-Hilfeseite zu Undo/Redo/A/B/Copy ist
am 30.08.2026 einzeln abgerufen worden — `https://www.fabfilter.com/help/pro-q/using/undoredo`,
verlinkt aus dem Inhaltsverzeichnis von H3. Sie belegt das Kopieren zwischen Instanzen **nicht**;
sie widerlegt es:

> „The Copy button copies the active state to the inactive state. This marks the current state of
> the plug-in and allows you to go back to it easily with the A/B button. After clicking Copy, the
> button disables itself to show that both states are equal, so there is nothing to copy anymore."
>
> — FabFilter Pro-Q 4 online help, „Undo, redo, A/B switch", abgerufen 30.08.2026

Gemessen am Artikelrumpf derselben Seite: **4** Treffer für `cop(y|ies)`, **0** für `instance`,
**0** für `paste`. Pro-Q 4s „Copy" ist die A/B-Zustandskopie innerhalb einer Instanz.

**Gewählt ist deshalb Weg 2 (Eingrenzen).** Die Karte behauptet jetzt nur noch das Gemessene.

**Eine Abweichung vom Vorlagentext, mit Grund.** Die Vorlage des Dirigenten sah für Weg 2 vor:
«„Copy" nennt Pro-Q 4 (H3), ohne zu sagen, ob damit die Übertragung in eine andere Instanz gemeint
ist.» Nach dem Abruf wäre dieser Satz selbst eine Untertreibung: die Herstellerhilfe **sagt** es,
und sie sagt das Gegenteil. Prüfliste E („Behauptung ≤ Messung") verlangt Deckung mit der Messung,
nicht nur eine Obergrenze — die Karte trägt deshalb den gemessenen Sachverhalt. Aus demselben Grund
ist die Seite als **H15** in §2.2 eingetragen, obwohl der Dirigent H15 für den positiven Fall
vorgesehen hatte: die eingegrenzte Aussage stützt sich auf diese Seite, und eine Aussage ohne ihre
Quelle zu führen wäre genau der Fehler, den B6 rügt.

**Die Frage an den User bleibt unverändert.** Titel und `was` von U18 sind nicht angefasst; gefragt
wird weiterhin beides — Voreinstellungen ja/nein und Kopieren zwischen Sonden ja/nein. Geändert ist
nur die Begründung: sie sagt jetzt, dass hinter dem Kopieren zwischen Sonden **kein belegter
Marktstandard** steht, sondern ein Nakama-eigener Wunsch. Für die Entscheidung ist das eher mehr
Information als vorher, nicht weniger.

### 10.3 Änderung — vorher/nachher, Datei:Stelle

**Edit an einer Planquelle (R2-1).**

| # | Datei:Stelle | Feld | Befund |
|---|---|---|---|
| R2-1 | `docs/plan/fragen.json`, `offen[]`, Karte `U18` | `warum` und `wo` | B6 (Restbefund) |

| Stelle | Vorher (Stand `b54a575`) | Nachher |
|---|---|---|
| `fragen.json:72`, `U18.warum` | „Bei den beiden verglichenen Produkten, deren Herstellerseiten das ausdrücklich führen — FabFilter Pro-Q 4 … und Voxengo SPAN Plus … —, **gehören Voreinstellungen und Kopieren zur Grundausstattung**" | „Voreinstellungen sind bei zwei der verglichenen Produkte durch die Herstellerseite belegt … **Das Kopieren zwischen zwei Instanzen ist bei keinem der beiden belegt.** FabFilters „Copy"-Taste … ist laut Herstellerhilfe etwas anderes: „The Copy button copies the active state to the inactive state" — also die A/B-Zustandskopie innerhalb derselben Instanz … Für SPAN Plus nennt die Herstellerseite überhaupt kein Kopieren" |
| `fragen.json:73`, `U18.wo` | „Beleg: docs/beweise/PR1.md Abschnitt 5.3 und Lücke L-3." | dieselben Belege, aufgeschlüsselt auf **H3 / H8 / H15**, dazu der datierte Satz „Eingegrenzt am 30.08.2026 in PR1 Runde 2 (Codex-Befund B6) …" |

**Änderungen am Manifest (M2-1 bis M2-4).**

| # | Stelle | Was |
|---|---|---|
| M2-1 | §2.2, Quellentabelle | neue Zeile **H15** — URL, Abrufdatum 30.08.2026, wörtliches Zitat, wofür sie belegt |
| M2-2 | §2.2, unter dem Runde-1-Nachtrag | **neuer datierter Nachtrag** (Runde 2): warum H15 nachgereicht ist, dass die Zuordnung nicht belegt, sondern widerlegt wurde, Fortschreibung zwölf → vierzehn → **fünfzehn** Quellen. Der Runde-1-Nachtrag bleibt wörtlich stehen |
| M2-3 | §5.3, Zeile „Undo/Redo, A/B, Kopieren", Zelle „Wer hat sie (Beleg)" | **datierter Zusatz in der Zelle**: Undo/Redo und A/B tragen H3/H8, das Kopieren tragen sie nicht; „Das Übertragen einer Kurve zwischen Instanzen ist damit keine belegte Marktstandardfunktion". Die anderen Zellen dieser Zeile und alle anderen Zeilen sind unberührt |
| M2-4 | §1.3 und §1.4 (lebender Kopf) | Seitenzahl 14 → **15** (H1–H15), Editzahl `17 + 11` → `17 + 11 + 1`, Produktzeile als „unverändert nach Runde 2" beschriftet; Rundentabelle um **1-P** (Wiederprüfung) und **2** (diese Runde) ergänzt und Zeile **1** auf das tatsächliche Ergebnis berichtigt (**NEEDS_WORK**, einer der sieben Befunde blieb offen) |

**Nicht angefasst.** `docs/plan/plan.json`, `docs/bauaufteilung-sonden.md`,
`docs/offene-punkte.md`, `docs/FL-Nakama-Sonden-Design-Entwurf.md`, die Karten U11, U13, U15–U17,
U19, U20 und `beantwortet{}`. Keine neue Registerzeile: der Restbefund ist eine Belegkorrektur an
einer bestehenden Karte, kein neuer Produkt- oder Vertragspunkt.

### 10.4 Prüfliste — E und F, je Zeile mit Messort

**A–D sind für ein Dokumentticket weiterhin nicht anwendbar**: kein Rückstauverhalten, kein
Lebenszyklus, kein Längen- oder Alphabetvertrag, kein Bau- oder Prüfriegel im Änderungssatz.

#### E. Behauptung ≤ Messung

| Zeile der Prüfliste | Wo in dieser Runde gemessen |
|---|---|
| „Jede Behauptung … sagt nicht mehr, als der Test misst — und nicht mehr, als das Gate verlangt" | §10.2, Tabelle „Eigene Nachmessung": vier Messungen mit Befehl und Trefferzahl — `grep -oi 'copy\|kopier'` auf `PR1.md:140` = 0, `cop(y\|ies\|ied)` auf der Voxengo-Feature-Seite = 0, `cop(y\|ies)` im H15-Artikelrumpf = 4, `instance` im H15-Artikelrumpf = 0. Die eingegrenzte Karte behauptet genau diese Messungen und nichts darüber hinaus; die Negativaussage „für SPAN Plus ist Kopieren nicht belegt" trägt ihren Befehl mit Trefferzahl |
| „Zahlen im Manifest sind gemessen, nicht abgeschrieben" | §1.3: die einzige geänderte Zahl ist die Seitenzahl **15**, gemessen mit dem daneben stehenden Befehl `grep -c '^\| H[0-9]' docs/beweise/PR1.md`. Die Editzahl `17 + 11 + 1` zählt die Zeilen der Tabellen in §6.1, §9.3 und §10.3. Alle mit Stichtag beschrifteten Zahlen der Runden 0 und 1 bleiben unverändert stehen |
| „Positionen im lebenden Kopf stehen als Symbol/Anker; im Verlauf gilt die Zeilennummer zum Stand ihres Abschnitts" | Der lebende Kopf nennt keine Zeilennummern, nur Quellen-ID (H15), Kartenkennung (U18) und Befund (B6). Die Zeilennummern in §10.1 und §10.3 sind Zitate bzw. Messorte **zum Stand `b54a575`** und ausdrücklich so bezeichnet; die stabilen Anker daneben sind Quellen-ID, Feldname (`warum`, `wo`) und Abschnittsnummer (§2.2, §5.3) |
| „Der lebende Kopf wird beim Abschluss nachgezogen; alles darunter ist append-only" | §1.3 und §1.4 sind nachgezogen (M2-4). §2–§9 sind unverändert bis auf die zwei **datierten Nachträge an Ort und Stelle** (M2-2, M2-3); keine Zeile und keine Tabellenzelle aus Runde 0 oder 1 ist umgeschrieben oder entfernt. Auch der in Runde 1 als geschlossen gemeldete B6-Block (§9.2) bleibt wörtlich stehen — was daran zu kurz griff, steht hier in §10 und nicht dort korrigiert |
| „Jede neue Prüfung wurde einmal absichtlich gebrochen; Rohausgabe des Rots liegt bei" | greift nicht — diese Runde führt keine neue Prüfung ein. Die vier Riegel aus Runde 1 laufen unverändert mit; ihr Ergebnis auf dem Endstand steht in §10.5 |
| „Geänderte Zusage — drei Stellen, kein Inventar" | **Keine Zusage geändert.** Diese Runde ändert keinen Plansatz, keinen Vertrag und keinen Lieferumfang; sie grenzt eine Marktbehauptung in einer noch offenen Fragenkarte ein. Belegt in §10.5, Lauf D: der Diff berührt weder `docs/plan/plan.json` noch `docs/bauaufteilung-sonden.md` noch `tools/beweise.ps1` |
| „Writer-Fixtures statt Handschrift" | greift nicht — kein Fixture im Änderungssatz |

#### F. Änderungssatz

| Zeile der Prüfliste | Wo in dieser Runde gemessen |
|---|---|
| „speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg im selben Commit" | **Kein Gegenpaar berührt.** Der Änderungssatz besteht aus einem Begründungstext in einer offenen Fragenkarte und vier Manifestnachträgen; er legt keinen Lebenszyklus, keinen Speicherweg und keinen Installationsschritt fest. Belegt durch §10.5, Lauf D: der Diff berührt ausschließlich `docs/plan/fragen.json`, `docs/beweise/PR1.md` und `docs/PLAN-STAND.md` |
| „Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines Vertrags im selben Änderungssatz" | greift nicht — kein Vertrag geändert. Was U18 später auslösen kann (ein Preset-Objekt im gespeicherten Zustand), bleibt genau das, was es vorher war: eine **offene** Karte mit Ticketzuordnung S26–28 und S31b. Die v3-Vertragsrunde aus Runde 1 ist unberührt |
