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
| Geprüfte Herstellerprodukte mit abgerufener Herstellerdoku | **8** von 8 Herstellern (Pro-Q 4, Neutron 5, smart:EQ 4, MMultiAnalyzer, SPAN Plus, Trackspacer, Smooth Operator Pro, Gullfoss) | §5, Quellenliste §2.2 |
| **Befunde: Defekt im Plan** | **6** | §4 |
| **Befunde: Lücke** | **4** | §4 |
| **Befunde: Härtung** | **3** | §4 |
| Neue Fragenkarten an den User (U15–U20) | **6** | §6.2 |
| Neue Registerzeilen (NAK-105 bis NAK-109) | **5** | §6.1 |
| Direkte Edits an Planquellen (E-1 bis E-17, über 3 Dateien) | **17** | §6.1 |
| Vorschläge an den Dirigenten (nicht eingearbeitet) | **3** | §6.3 |

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
| H12 | Steinberg VST 3 — `ChannelContext::IInfoListener` | https://steinbergmedia.github.io/vst3_doc/vstinterfaces/classSteinberg_1_1Vst_1_1ChannelContext_1_1IInfoListener.html | Der Weg, über den ein Host Spurname, Farbe und Index an ein Plugin gibt — Grundlage von Befund D-2 |

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
| **L-1** | **Wer darf `post_fader_contribution` führen, wenn es den Bus gibt?** Heute ist die Position für **keine** Klasse erlaubt (`nakama-state-v2.md:54`, fail-closed seit der G1-Nacharbeit). Der Entwurf beantwortet die Frage nicht (§32.2 ordnet Positionen Aussageklassen zu, nennt aber keine Produktklasse). Eigentümer laut Vertrag: `SONDE-011`. | `nakama-state-v2.md:54-90`; NAK-79; `plan.json:195` (S16–17) nennt es nicht | Edit **E-15** (S16–17 trägt die Frage als Lieferumfang); die Produktseite ist **Fragenkarte U19** nicht wert — sie ist eine Vertragszuordnung, nicht sichtbare Funktion, und liegt beim Dirigenten. Registerzeile bleibt NAK-79. |
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
| Alle Instanzen der Sitzung in einer Liste sehen | Pro-Q 4 (H1), smart:EQ 4 bis 10 pro Gruppe (H6), MMultiAnalyzer (H7), SPAN Plus (H8) | **hat** — Gen Seite 1, Sonden bis 16 sichtbar / 32 im Vertrag | — | S18–19 | — |
| Fremde Instanz **fernbedienen** | Pro-Q 4: „add and change curves just like you are used to" (H1); smart:EQ 4: „remotely controlled from any other instance" (H6) | **hat** — und macht es zum Hauptbedienweg (Gen Seite 2) | — | S29–31, S31b | Der Markt-Befund aus dem 23.08.-Blatt bestätigt sich: die Fernsteuerung selbst ist **nicht** die Neuheit; der Hub mit Advisor ist es. |
| **Reihenfolge = DAW-Spurreihenfolge** | Pro-Q 4: „matching the track order in your DAW … Instances will order themselves according to how they are inserted" (H1) | **entschieden, aber ohne Datenweg** | **V, C** | S18–19 | **D-2** |
| Spurname am Eintrag | Pro-Q 4 (H1, per Track gruppiert); smart:EQ 4 Gruppenliste (H6) | **entschieden, aber ohne Datenweg** | **V, C** | S18–19 | **D-2**, Karte **U20** |
| Prioritäts-/Rangfolge zwischen Instanzen | smart:EQ 4: „drag and drop them into an order that decides the precedence" (H6) | **löst anders** — musikalische Rollen (führt/trägt/begleitet/geschützt/verschmolzen), Entwurf §9/§37 | — | S23–25 | — |
| Instanzen finden sich selbst | MMultiAnalyzer: „they automatically find each other and exchange their respective analyses" (H7); iZotope Relay als Inter-Plugin-Communication (H5) | **hat** — Broker + Discovery, `session_snapshot` | — | S16–17, S18–19 | Autostart des Brokers fehlt: **L-2** |
| Spektrum einer anderen Spur im eigenen Graph | Pro-Q 4 External/SC-Spektrum (H2); SPAN Plus „Export To" (H8); smart:EQ 4 Gruppen-Analyzer (H6) | **hat** (Datenweg) — Gen Seite 2 zeigt zwei EQ-Spuren in einem Graph (Entscheid 23.08.) | — | S31b | Live-Spektrum bei zwei Spuren nur für die bearbeitete Spur (Festlegung 14) |

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
| Dynamische Bänder | Pro-Q 4 (H3); TDR/Neutron | **hat** — `dynamic_enabled`, Threshold/Range/Attack/Hold/Release | — | S26–28 | — |
| Externer Sidechain je Band | Pro-Q 4 (H3); Neutron „band-specific internal or external sidechain" (H5) | **hat im Vertrag** (`sidechain_source`), im ersten Release **unsichtbar** (U5, 24.08.) | — | S26–28 | bewusster Entscheid |
| M/S und L/R je Band | Pro-Q 4 (H3); Trackspacer (H9) | **hat** — `channel_mode` | — | S26–28 | — |
| **Phasenmodi: Natural Phase / Linear Phase** | Pro-Q 4: „zero-latency mode, Natural Phase mode or in linear-phase mode with variable processing resolution" (H3) | **bewusst nicht im ersten Kern** — §30.1 schließt lineare Phase aus, §44.2 fordert konstant 0 Samples Latenz, kein Lookahead | **C** (Latenzmeldung), **S** (neuer Parameter), **O** (Umschalter) | S26–28 / S28b, G6 | **Fragenkarte U15** — der User nannte Pro-Q als Vorbild und „alle gängigen filter die es gibt" |
| Auto-Gain | Pro-Q 4 (H4) | **entschieden 24.08., nicht im Plan** | S, O | S26–28 | **D-3 (a)** |
| Band-Solo / einzelnes Band abhören | Pro-Q 4 Band solo (H3) | **löst anders** — Hörmodus `FOCUS` (§41.1: „bearbeiteter Frequenz-/M/S-Anteil der Zielsonde") | — | S20–22 | kein Fund |
| Delta-Abhören (nur die Änderung) | Neutron 5 Delta-Buttons (H5) | **hat** — Hörmodus `DELTA` (§41.1) | — | S20–22 | — |
| Ausgangs-Trim, Width, Mono-Maker | Pro-Q 4 Output (H4) | **hat** — `input_trim_db`, `output_trim_db`, `width`, `mono_bass_hz` | — | S26–28 | — |
| **Voreinstellungen (Preset-Browser)** | Pro-Q 4 (H3); SPAN Plus Preset-Manager (H8) | **fehlt als eigene Funktion** — Projekt-State reist über den Host, aber es gibt kein Preset-Objekt, keinen Browser, keine Werksvoreinstellungen | **S**, O | offen | **Fragenkarte U18**, Lücke **L-3** |
| Undo/Redo, A/B, Kopieren | Pro-Q 4 „Undo, Redo, A/B and Copy" (H3); SPAN Plus (H8) | **teilweise** — eigener Undo-Ring je Sonde (Entscheid 23.08., Festlegung 21), A/B als Experiment (§43). **Kopieren einer Kurve zwischen Sonden fehlt**; `nakama-state-v2.md:94` behandelt Copy/Paste nur als Gefahr (Fremdklasse ⇒ read-only) | **S**, O | S26–28 / S31b | in **U18** mitgeführt |
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
| Referenz-/Zielkurve übernehmen (EQ Match) | Pro-Q 4 EQ Match gegen andere Instanz oder Datei (H1, H3); smart:EQ 4 „load up a reference track to create a custom profile" (H6) | **löst später** — Roadmap 11 (§47.5), **nach R4** | **V** (neue Familie), S, O | nach R4 | **Fragenkarte U19** |
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

1. **V-1 — Ein v3-Fenster vor S16–17.** D-2 und D-5 brauchen beide ein Feld im
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

### Warum der Kanon nicht läuft

`tools/beweise.ps1` baut und misst Produktcode, Tests, Schemas und Fixtures.
Dieser Änderungssatz berührt ausschließlich `docs/**` (Beleg: der
`git diff --stat` in §8, Lauf D). Ein Kanon-Lauf würde denselben Stand messen wie der
letzte Lauf zu `SONDE-010` und keine Aussage über dieses Ticket treffen. Er wird
deshalb bewusst **nicht** gefahren und auch nicht als Beleg zitiert.
