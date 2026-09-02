# Beweismanifest — PR2 «Planprüfung: Quellen zusammenführen, Design-Entscheide in den Bauplan holen, Register bereinigen»

<!-- NAKAMA-URTEIL: T1 PASS 2026-09-02 -->

> **Selbstaudit T1 PASS, gesetzt vom Erbauer am 02.09.2026 (§9).** Die T3-Marke setzt der
> Dirigent nach dem Urteil eines frischen Codex-Threads; sie fehlt hier bewusst.

| Feld | Wert |
|---|---|
| Ticket | `PR2` (Planprüfung, kein Bauticket) |
| Phase / Session | P3, zwischen S18–19 (`SONDE-012`) und G3 |
| Gate-Text (Quelle) | `docs/plan/plan.json`, Schritt `PR2` — wörtlich in §1.1 |
| Basis-SHA | `55cdb91599239aa99ec1322e23d109ab9424d7b2` (Branch `master`) |
| Datum | 2026-09-02 |
| Prüfstufen | T1 ☑ · T3 steht aus (frischer Codex-Thread) — die geforderte Stufe ist **T3** |
| Kanon | **läuft nicht** — kein Produktcode, kein Test, kein Schema angefasst (Begründung §8) |
| Änderungssatz | 10 Dateien, +5 285 / −249 Zeilen (§8, Lauf D); davon 4 238 Zeilen die byteidentische Archivkopie |

---

## 1. Kopf

### 1.1 Gate-Text, wörtlich

> Zweiter Zwischenhalt der Implementierung. Anlass, User-Wort 01.09.2026: „wir sind aktuell
> mitten in der implementierung des sondenplanes. ich wollte an dieser stelle mal inne halten und
> dir die möglichkeit geben selbständig nachzudenken, was du jetzt tun würdest abgehen von einfach
> weiter machen im dirigentenmodus. mein gefühl sagt mir dass man grundlegend plan und bisherige
> arbeit auf herz und nieren sich anschauen sollte?" Auftrag nach Fables Einschätzung, User-Wort
> 01.09.2026: „okay dann sorge dafür dass genau das so passieren wird. opus wieder bauer und codex
> prüfer . so wie am anfang, habe genug wochen kontigent. du kannst ja jederzeit dirigent starten,
> mach wie du willst, der plan klingt gut". ZEITPUNKT (Wahl von Fable, 01.09.): nach dem PASS von
> G2 (R0 eingefroren, Worktree frei) und VOR G3, also vor dem Beginn von P4 — danach wird gebaut,
> was am teuersten zu korrigieren ist: Advisor, EQ-Kern, Fernsteuerung und die Oberfläche. Bis G2
> PASS läuft die G2-Nacharbeit; PR2 beginnt erst danach. BEFUND, der den Zuschnitt begründet
> (gemessen 01.09. auf d4239a7): PR1 vom 30.08. hat jeden Entscheid gegen Plan und Markt geprüft
> und wird NICHT wiederholt. Seit PR1 ist aber der Funktionsneustart der UI vom 31.08. samt
> Fünferblöcken 01 und 02 und dem Größenentscheid nicht im Bauplan angekommen: plan.json trägt
> keinen Treffer zu Mini-Panel, Dynamic-Ansicht, Remove-Slot, Kanalmodus, SEND DRAFT oder 950×538;
> S31b verweist weiter auf Figma, design/assets/rework-basis-2026-08-25/ und
> ui-spielregeln-eq-zentrale.md, die laut CLAUDE.md seit 31.08. Verlauf sind (Blueprint §15 Punkt
> 11 nennt genau das als offen). Der Entwurf 0.4 vom 20.08. trägt 17 Errata (a) bis (q) mit
> Vorrang, sein Textkörper nennt weiter „drei Ziele/drei Bundles" (§53.4, §55, §63) und Suna.
> docs/offene-punkte.md führt 86 offene Zeilen auf 238 KB, darunter NAK-13 „Broker-Autostart
> fehlt", obwohl SONDE-011 den Autostart gebaut hat, drei Zeilen, die sich selbst als geschlossen
> bezeichnen, und neun Vertragslücken für P6/P7 ohne Planschritt. Der CODE ist nicht Gegenstand:
> P0 bis P2 sind durch G0, G1 und G2 adversarial geprüft, der Kanon war zuletzt 39/39 grün.
> INHALT: (1) ENTWURF ZUSAMMENFÜHREN — Fassung 0.4 samt Errata-Block zu einer Fassung 0.5 am
> selben Pfad: jedes Erratum an seiner Stelle eingearbeitet und mit Buchstabe, Datum und User-Wort
> belegt; tote Dreispurigkeit (Suna als App-Name, drei Ziele/Bundles, Hub-App) gestrichen; bei
> Widerspruch gilt der Errata-Block, danach der jüngste datierte Entscheid aus design/abnahmen/,
> fragen.json und den plan.json-Nachträgen. Die Fassung 0.4 wandert byteidentisch nach docs/archiv/
> (Dateiname mit Fassung und Datum). Verweise auf Entwurfs-Paragraphen in plan.json,
> bauaufteilung-sonden.md, Manifesten und CLAUDE.md werden nachgezogen oder als bewusst historisch
> markiert. Kein Satz wird erfunden: was weder Erratum noch datierter Entscheid deckt, bleibt
> 0.4-Wortlaut oder wird Fragenkarte. (2) RANGFOLGE FESTLEGEN — Entwurf 0.5 und
> design/docs/funktions-und-bedien-blueprint.md sind heute zwei lebende Spezifikationen. In beiden
> Köpfen und in der Lesetabelle von CLAUDE.md wird festgeschrieben, was jede bindet und was bei
> Widerspruch gilt. Technische Entscheidung des Dirigenten mit Vorgabe: Entwurf = Technik,
> Verträge, Phasen, Falsifikation; Blueprint = Funktions- und Bedienarchitektur beider Apps; über
> beiden der aktuelle Vertrag und Code; ein Widerspruch wird als Registerzeile geführt, nie
> kaschiert. (3) DESIGN-STRANG IN DEN BAUPLAN — die Entscheide aus
> design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md (Gen Fläche 1 und 2),
> 2026-09-01-technische-ui-architektur-fuenferblock-01.md und -02.md,
> 2026-09-01-gen-nur-standardgroesse.md sowie die 16 Punkte aus Blueprint §15 werden als datierte
> Nachträge in die betroffenen Schritte geschrieben (S26–28, S28b, S29–31, S31b; P3-Diagnose der
> NAK-57-Zähler nach S18–19/G3; Überdeckungszone nach S23–25). S31b wird von Figma,
> rework-basis-2026-08-25 und ui-spielregeln-eq-zentrale.md auf den Blueprint plus eine noch
> abzunehmende visuelle Spezifikation umgestellt. Technische Vertragsarbeit ohne Planschritt
> bekommt je einen Schritt zugeordnet: Band-Belegung und wiederherstellbares Remove,
> 10-Sekunden-Kandidat, direkte Edit-Transaktion, Frame.band_dynamic_gain_db, zweistufiger Mix,
> Main-Führung nach Brokerneustart (NAK-120). bauaufteilung-sonden.md P6–P7 zieht nach. (4)
> REGISTER TRIAGIEREN — jede offene NAK-Zeile bekommt genau eine Klasse: „geschlossen" (mit
> Commit-SHA in die Geschlossen-Tabelle; mindestens zu prüfen NAK-13 gegen SONDE-011, NAK-60,
> NAK-62, NAK-119), „blockiert P4–P7" (dem Planschritt zugeordnet, Nachtrag dort — darunter
> NAK-105, NAK-107, NAK-110, NAK-120, NAK-125, NAK-126, NAK-59, NAK-81), „Härtung/Struktur"
> (bleibt, Klasse sichtbar; NAK-121 Modul-Aufteilung und NAK-124 bekommen einen Ort im Plan),
> „Werkzeug" oder „Produktfrage" (Fragenkarte). Am Kopf von docs/offene-punkte.md entsteht eine
> kurze Blockerliste je Phase. Die Regeln bleiben: nie löschen, schließen nur mit ID und SHA.
> Beantwortete Fragenkarten mit dem Ergebnis „Kein Entscheid — wird neu gestellt" (U9.3, U9.4,
> U6.8) oder „Folgefrage" (U2.4, U2.6, U2.9) werden entweder als durch den Funktionsneustart
> überholt geschlossen oder bekommen ihre Folgekarte; G3 nennt „deine fünf Entscheide aus U9" und
> wird entsprechend berichtigt. Werkzeugreste werden bewertet, nicht still entsorgt:
> .workflow/ultracode/20260831-084621-sonde011-phaseb-bau (untracked, rund 76 MB) und stash@{0}
> vom 28.08. (5) PROZESSBILANZ MIT ZAHLEN — tools/dirigent/rundenbilanz.py über alle Tickets seit
> S0: Runden je Ticket, Produkt- und Testdiff je Runde, verlorene oder abgebrochene Kanonläufe,
> Anteil Werkzeug-Commits; dazu der Befund „Bauer ohne Compiler": NAK-123 übergab am 01.09. rund
> 2 800 Zeilen Sicherheitscode mit Laufstatus NOT RUN, den der User am 01.09. mit Opus als Bauer
> und Codex als Prüfer beantwortet hat (Skill §3.6). Technische Folgen setzt der Dirigent im Skill
> oder Register um, Produktfolgen werden Fragenkarten. PRÜFUNG: frischer Codex-Thread, lesend, aus
> tools/dirigent/pruefauftrag-vorlage.md, Stufe T3 wie PR1. Prüfbereich sind die Ticketpfade:
> docs/plan/plan.json, docs/plan/fragen.json, docs/bauaufteilung-sonden.md,
> docs/FL-Nakama-Sonden-Design-Entwurf.md, docs/archiv/,
> design/docs/funktions-und-bedien-blueprint.md, CLAUDE.md, docs/offene-punkte.md,
> docs/beweise/PR2.md. Der Prüfer misst, ob jede Einarbeitung ihre Quelle mit Datum trägt, ob kein
> Entscheid erfunden oder verloren ist, ob kein Verweis ins Leere zeigt und ob der Planstand ohne
> Warnung rechnet. Kein Produktcode, kein Test und kein Schema wird angefasst; der Kanon läuft
> deshalb nicht, planstand.py, antworten_blatt.py, JSON-Gültigkeit und der Tabellenriegel laufen.
> Ergebnis in docs/beweise/PR2.md mit Urteilsmarke; Produktentscheide gehen als Fragenkarten an
> den User, Technik entscheidet der Dirigent.

### 1.2 Kurzbilanz — gezählt, nicht geschätzt

Jede Zahl stammt aus einem Lauf in §8, nicht aus einer anderen Datei.

| Größe | Wert | Messort |
|---|---:|---|
| Errata mit Einarbeitung an ihrer Stelle | **17 / 17** (a–q) | §8, Lauf E |
| Marken `[0.5 · Erratum (x) · Datum]` im Text | 25 | §8, Lauf E |
| Grabsteinzeilen „gestrichen in Fassung 0.5" | 23 | §8, Lauf E |
| nummerierte Überschriften 0.4 → 0.5 | 255 → 255 (unverändert; +2 unnummerierte im Kopf) | §8, Lauf E |
| Archivkopie byteidentisch zu 0.4 | SHA-256 `a8230e09…3e7c5`, 237 425 Bytes, beidseitig gleich | §8, Lauf G |
| Blueprint-§15-Punkte mit Schritt-ID | **17 / 17** (der Gate-Text nannte 16; Punkt 17 kam am 02.09.2026 dazu) | §8, Lauf E |
| Registerzeilen offen / geparkt / geschlossen | 79 / 5 / 31 = 115 | §8, Lauf E |
| davon Planarbeit / Härtung / Werkzeug / Produktfrage | 18 / 31 / 24 / 6 = 79 | §8, Lauf E |
| Registerzeilen neu geschlossen | 14 | §3.4 |
| Registerzeilen aus der Geschlossen-Tabelle zurückgeholt | 5 | §3.4 |
| neue Registerzeilen (Werkzeugreste) | 2 (NAK-128, NAK-129) | §5 |
| Fragenkarten offen | 2 → **8** (U21–U26 neu) | §8, Lauf E |
| beantwortete Karten datiert nachgehalten | 6 (U2.4, U2.6, U2.9, U6.8, U9.3, U9.4) | §8, Lauf E |
| uneinheitliche Markdown-Tabellenzeilen im Register | 5 → **0** | §8, Lauf F |
| Dokuriegel über die sieben Ticketpfade | 0 Befunde, 9 benannt, Exit 0 | §8, Lauf F |

### 1.3 Zeitpunkt

Der Gate-Text verlangt PR2 **nach dem PASS von G2 und vor G3**. Beides ist erfüllt:
`docs/beweise/G2.md` trägt die Urteilsmarke **T3 PASS 2026-09-02**, und G3 hat keinen Beleg.
(Die Marke wird hier bewusst als Text genannt und nicht als HTML-Kommentar zitiert: `marken_lesen()` in `tools/plan/planstand.py` liest jeden solchen Kommentar in einem Manifest als dessen eigenes Urteil — im ersten Lauf am 02.09.2026 hat genau das PR2 faelschlich als abgenommen gezaehlt, 24 statt 23.)
Der Arbeitsbaum war zu Beginn frei bis auf das fremde, untrackte
`.workflow/ultracode/20260831-084621-sonde011-phaseb-bau` — es ist während des ganzen Tickets
unberührt geblieben (§5).

---

## 2. Quellenverzeichnis

Alles auf dem Stand `55cdb91`, sofern nicht anders vermerkt. Jede Quelle ist gelesen worden,
nicht aus einem Manifest abgeschrieben.

### 2.1 Bindende Entscheidquellen

| Quelle | wofür gelesen |
|---|---|
| `docs/plan/plan.json`, Schritt `PR2` | Gate-Text, vollständig und wörtlich |
| `CLAUDE.md` | Wahrheitskern, Arbeitsregeln, Lesetabelle |
| `docs/plan/LIES-MICH.md` | Form von `plan.json` und `fragen.json`, Fragenkarten, Statusrechnung |
| `design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md` | Arbeitsmodus, abgenommener Stand Gen Fläche 1 und 2, „Bewusst offen" |
| `design/abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-01.md` | fünf Dynamic-Entscheide, technische Grenze der Live-Bewegung |
| `design/abnahmen/2026-09-01-technische-ui-architektur-fuenferblock-02.md` | Panel-Rebinding, Kanalmodus, `enabled`, ausgeschaltetes Band, Remove/Undo |
| `design/abnahmen/2026-09-01-gen-nur-standardgroesse.md` | eine logische Größe 950×538, Abnahmebeweis vom 01.09.2026 |
| `design/abnahmen/2026-08-23-gen-eq-zentrale.md`, `…-interview-struktur.md` | EQ-Zentrale-Entscheid, Drei-Stufen-Geste, U14 |
| `design/abnahmen/2026-08-27-arbeitsnamen-und-probeeq-doppelrolle.md` | „nein" zum lokalen Voll-Editor |
| `design/abnahmen/2026-08-22-hub-antworten-35.md` | Originalfragen zu U2.4, U2.6, U2.9, U6.8, U9.3, U9.4 |
| `design/docs/funktions-und-bedien-blueprint.md` | §0 Rang, §4 Informationsarchitektur, §15 die 17 offenen Punkte |
| `docs/plan/fragen.json` | `offen[]` und `beantwortet{}` |
| `docs/offene-punkte.md` | alle 86 offenen, 5 geparkten, 22 geschlossenen Zeilen |
| `docs/bauaufteilung-sonden.md` | Kopf, §1.4, §3 Sessionplan, §6.4 |
| `docs/beweise/PR1.md` | Manifestaufbau, §7 Prüfliste, §8/§9.4 Beweislauf, §9.4 F Tabellenriegel |
| `tools/dirigent/pruefliste.md` | Abschnitte E und F |

### 2.2 Gemessene Quellen (Belege für die Schlüsse in §3 und §4)

| Quelle | was daraus gemessen wurde |
|---|---|
| `eq-copilot/plugin/core/ipc/BrokerLifecycle.{h,cpp}` | Broker-Autostart ist gebaut (NAK-13 geschlossen) |
| `docs/beweise/SONDE-011.md` §2, §3, §3a, §4, §5, §6, §9, §B.6 | Entscheide 28-B, 59-A, NAK-29/40/114, Verhaltensmatrix Broker-Autostart |
| `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` | `Frame.band_dynamic_gain_db`, `state_report.dsp`, `command_ack.applied_dsp`, `state_report.eq_enabled` — Namen reserviert, ohne Nutzlast |
| `eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs:230` | `band_stereo` mit ID 10 gebaut |
| `tools/eq-copilot/pruefe_v3_vertrag.py:1091` | Riegel `kein_unsubscribe_session_name` |
| `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:205,252` | `x-nakama-discriminator` auf `time_basis` und `bounds_valid` |
| `git log`, `git show`, `git stash show` | Prozessbilanz (§4), Werkzeugreste (§5), Schließungs-SHAs |
| `.workflow/ultracode/20260831-084621-sonde011-phaseb-bau` | 2 375 Dateien, 76 MB, Inhaltsbewertung (§5) — nur gelesen |

---

## 3. Änderungssatz — je Edit Grund und Fundstelle

Sechs Commits, jeder sofort gepusht, jeder mit explizitem Pathspec.

| # | Commit | Pfade |
|---|---|---|
| 1 | `590918b` | `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/archiv/FL-Nakama-Sonden-Design-Entwurf-0.4-2026-08-20.md` |
| 2 | `2ffd75f` | `CLAUDE.md`, `design/docs/funktions-und-bedien-blueprint.md`, `docs/FL-Nakama-Sonden-Design-Entwurf.md` |
| 3 | `894dce7` | `docs/plan/plan.json`, `docs/bauaufteilung-sonden.md` |
| 4 | `d4241fa` | `docs/offene-punkte.md`, `docs/plan/fragen.json`, `docs/plan/plan.json` |
| 5 | `cf21476` | `tools/plan/dokuriegel.py`, `tools/plan/planstand.py`, `docs/offene-punkte.md`, `docs/FL-Nakama-Sonden-Design-Entwurf.md` |
| 6 | siehe §10 | `docs/beweise/PR2.md`, `docs/PLAN-STAND.md`, `docs/ANTWORTEN-OFFEN.md`, `docs/plan/plan.json` |

### 3.1 Entwurf 0.5 (Commit 1)

**Archivkopie zuerst.** `docs/FL-Nakama-Sonden-Design-Entwurf.md` wurde **kopiert**, nicht
verschoben: der lebende Pfad behält seine Blame-Historie, das Archiv ist eine Momentaufnahme.
Ein `git mv` hätte die Historie auf den Archivpfad umgelenkt und den lebenden Pfad als neue Datei
erscheinen lassen. Byteidentität in §8, Lauf G.

**§-Nummerierung stabil.** 255 nummerierte Überschriften vorher wie nachher, keine umnummeriert
(§8, Lauf E). Ein gestrichener Abschnitt behält Überschrift **und** Nummer und trägt eine
Grabsteinzeile mit Datum, Grund und Entscheidquelle. Das ist der neue stabile Bezug für die
T2-Regel „Gate-Text aus dem Entwurf" — bisher hielt ihn die Regel „Text unter dem Errata-Block nie
umschreiben" (`bauaufteilung-sonden.md` §6.4), die damit abgelöst ist.

| Erratum | Stelle im Text | Art | Entscheidquelle |
|---|---|---|---|
| (a) | §68 | Marke | Entscheid 20.08.2026 „okay dann fangen wir damit nächste session an" |
| (b) | §3 Kopf, §30, §44.1, §53.4, §53.5, §55, §63, §65 | Marke + Streichung | Entscheid 21.08.2026 (Namen), 23.08.2026 (Merge) |
| (c) | §0.3 letzter Punkt, §3.5, §30, §31.1, §35.1 | 5 Grabsteine | Entscheid 21.08.2026 „Familie; Prisma nur Studie" |
| (d) | §0.1 | Marke | Entscheid 21.08.2026 „die active Probe fester Name : Nakama Probeeq …" |
| (e) | §18, §30, §31.1, §34.2, §42.1, §42.4, §46.3, §46.4, §47.3, §47.7, §48.4, §49.2 (Gate 2 und 8), §59 | 13 Grabsteine, zwei davon ganze Abschnitte (§18, §46.3) | Entscheid 21.08.2026 „Nein – raus aus dem Produkt" |
| (f) | §27, §30, §33.2, §49.3 | 4 Marken | Entscheid 21.08.2026 „Hingenommen, passen aber" |
| (g) | §5 | Marke | Entscheid 21.08.2026 „Englisch – mein Wort" |
| (h) | §0.4 Punkt 3, §9, §35.1 | Marke + Streichung | Entscheid 21.08.2026 „Das ist ein Designprototyp keine technikanleitung …" |
| (i) | §53.4, §65, §66.1 | 3 Marken | Befund 21.08.2026, am 02.09.2026 an der Quelle nachgemessen |
| (j) | §0.4 | Marke | Entscheid 21.08.2026 „Nie abgenommen – bleibt Provisorium" |
| (k) | §53.6, §65 | 2 Marken mit dem gemessenen FL-Bitstand | Entscheid 21.08.2026, Messung 22.08.2026 |
| (l) | §0.4 | Marke, „vom User festgelegt" zurückgenommen | Befund 21.08.2026: kein Wortlaut vorhanden |
| (m) | §0.2, §22 | 2 Marken | Befund 21.08.2026, Register NAK-39 |
| (n) | §0.4 Punkt 3, §3, §16, §27, §30, §44.1, §44.2, §53.2, §53.4, §63 | Marken + Streichungen | Entscheid 23.08.2026, voller Wortlaut in `design/abnahmen/2026-08-23-gen-eq-zentrale.md` |
| (o) | §33.4 | Marke mit der Drei-Stufen-Geste | Entscheid 23.08.2026, `…-interview-struktur.md` |
| (p) | §0.1, §0.4 Punkt 3, §44.1 | Marke + Streichung | Entscheid 27.08.2026 „nein" |
| (q) | §27, §44.2, §53.8 | Marke + 2 Streichungen | Erratum (q) der Fassung 0.4, PR1 30.08.2026; U14 23.08.2026 |

**Tote Dreispurigkeit gestrichen.** Suna als App-Name (§3, §30, §44.1, §53.5), drei Ziele und drei
Bundles (§30, §44.1, §53.2, §53.4, §55, §63, §65), Hub-App und Prisma-Begleit-App (§0.3, §0.4,
§3.5, §30, §31.1, §35.1). Der Begriff **Master-Hub** ist damit aus dem Produktvokabular; es gibt
zwei Plugins und den unsichtbaren Broker.

**Nicht erfunden.** Der ±3-dB-Satz in §44.2 ist der einzige Satz, den PR1 als „Zusage ohne
Ausführung" (NAK-108) geführt hat; er ist jetzt mit Datum und User-Zitat gestrichen. Alles, was
weder ein Erratum noch ein datierter Entscheid deckt, steht unverändert im 0.4-Wortlaut.

**Ergänzt am 02.09.2026, jeweils mit Quelle:** §0.4 bekommt einen fünften Punkt mit dem
Größenvertrag 950×538 (Entscheid 01.09.2026), §16 die vier Folgen des 23.08.-Entscheids, §33.4 die
Drei-Stufen-Geste samt der offenen Kandidatenspezifikation, §53.8 den fehlenden Zustandsort für
Gens Master-EQ.

### 3.2 Rangfolge (Commit 2)

Gleichlautend im Kopf des Entwurfs, im Kopf des Blueprints (§0) und in `CLAUDE.md`:

- **Entwurf 0.5** bindet Technik, Verträge, Phasen, Falsifikation — nicht Aufbau, Hierarchie und
  Bedienwege der Oberflächen.
- **Blueprint** bindet die Funktions- und Bedienarchitektur beider Apps — nicht Schemas,
  Phasenschnitt, Gates.
- **Über beiden** stehen der aktuelle Vertrag und der Code. Widerspricht eine Spezifikation dem
  gebauten Vertrag, gilt der Vertrag. Ein Widerspruch zwischen beiden wird als Zeile in
  `docs/offene-punkte.md` geführt, nie kaschiert.

`CLAUDE.md` ist **minimal** geändert, wie der Gate-Text es verlangt: der Wahrheitskern-Punkt
„Plan", zwei Zeilen der Lesetabelle und eine neue Zeile für den Widerspruchsfall. Nichts anderes
dort ist umgeschrieben.

### 3.3 Design-Strang (Commit 3, ergänzt in Commit 6)

| Schritt | Was hinzukam | Quelle |
|---|---|---|
| `S18–19` | Gen Fläche 1 vollständig: Sources-Spalte, Zeileninhalt, ein Hauptziel, Graph und Findings waagerecht, feste Findings-Höhe, ein Finding im Fokus, Markierung statt Zoom, `SEND DRAFT` als einziger Weg, technische Grenze; Größe 950×538; P3-Diagnose der übrigen NAK-57-Zähler | Abnahmen 31.08. und 01.09.2026 |
| `S23–25` | Ort der Überdeckungszone: stabiler Graph, Markierung ohne Ausschnittwechsel; der laufende Wert muss einen Frequenzbereich benennen | Abnahme 31.08.2026 |
| `S26–28` | Band-Belegung, atomare Slotfreigabe, Remove-Undo-Objekt; Nutzlast zu `Frame.band_dynamic_gain_db`; `dynamic_enabled` und die fünf Werte bleiben getrennt; `channel_mode` als diskreter Blockrandparameter | Fünferblock 01 und 02 |
| `S28b` | dieselbe Semantik für Gens Master-EQ, dynamische Auslenkung lokal ohne IPC | Fünferblock 01 und 02 |
| `S29–31` | 10-Sekunden-Kandidatenvertrag, direkte Edit-Transaktion, zweistufiger Mix, Main-Führung nach Brokerneustart | Entscheid 23.08.2026, Blueprint §15 |
| `S31b` | Quelle umgestellt; zwölf abgenommene Punkte der Gen-Fläche-2-Bedienarchitektur | Abnahmen 31.08. und 01.09.2026 |
| `S32–33` | FL-Messung Aux/PDC (U11) und Träger der Position `post_fader_contribution` (NAK-79) | Blueprint §15 Punkt 8, Register |
| `S34–35` | Textpflege erledigt, Codesigning, Installer-Hash, Identitäts-Umbenennung | Blueprint §15 Punkt 9, Register |
| Phase `P6–P7` | `ui-spielregeln-eq-zentrale.md` ist kein Prüfmaßstab mehr | User-Auftrag 31.08.2026 |

**S31b, Quellenumstellung im Wortlaut.** Weggefallen sind alle drei früheren Quellen: der
Figma-Stand (NAK-65), `design/assets/rework-basis-2026-08-25/gen-page-2-eq-center.png` und
`design/docs/ui-spielregeln-eq-zentrale.md`. Bindend sind ab jetzt der Blueprint **plus eine noch
abzunehmende visuelle Spezifikation**, die es heute nicht gibt; ohne deren datierte Abnahme
beginnt die sichtbare Umsetzung nicht. Grund ist der User-Auftrag vom 31.08.2026 „ignoriere
bisherige design die entworfen wurden". Die drei Dateien bleiben liegen und werden nicht gelöscht.

**Alle 17 Punkte aus Blueprint §15 tragen eine Schritt-ID** (§8, Lauf E). Der Gate-Text sprach von
16 Punkten; Punkt 17 (Global-Sektion auf Gen Fläche 2) ist am 02.09.2026 dazugekommen und
ebenfalls zugeordnet — S31b, mit dem ausdrücklichen Vermerk, dass er als Designfrage in einen
kommenden Fünferblock gehört und hier nicht erfunden wird.

`bauaufteilung-sonden.md` zieht nach: Kopf auf Fassung 0.5, Rangfolge, §1.4 Quellen, die P3-, P4-
und P6–P7-Zeilen, eine neue Zuordnungstabelle der sechs Vertragsstücke, §6.4 abgelöst.

### 3.4 Register (Commit 4)

**Klasse je Zeile, sichtbar am Zeilenanfang.** 79 offene Zeilen, jede mit genau einer Klasse:

| Klasse | Zeilen | Bedeutung |
|---|---:|---|
| `**[Planarbeit · <Schritt>]**` | 18 | einem offenen Planschritt zugeordnet, Nachtrag dort |
| `**[Härtung/Struktur]**` | 31 | bleibt offen, keine gebrochene Produktzusage |
| `**[Werkzeug]**` | 24 | Runner, Cockpit, Prüfskripte, Arbeitsreste |
| `**[Produktfrage · Karte U<n>]**` | 6 | nur der User kann entscheiden |

Planarbeit je Schritt: G3 2 · S18–19 2 · S20–22 2 · S26–28 2 · S28b 1 · S29–31 4 · S31b 1 ·
S32–33 1 · S34–35 3.

**14 Zeilen geschlossen**, jede mit Quelle:

| ID | geschlossen durch | Beleg |
|---|---|---|
| NAK-10, -28, -29, -40, -59, -114 | `SONDE-011` Phase A `01f6194` | `docs/beweise/SONDE-011.md` §2–§6, §9; S16–17 trägt T2 PASS 2026-09-01 |
| NAK-13 | `SONDE-011` Phase B `d208ad8`, Abschluss `f176f24` | `BrokerLifecycle.{h,cpp}` gelesen; Verhaltensmatrix §B.6 mit `AUTOSTART_ARTEFAKTE_PHASE_B = 0` |
| NAK-90 | NAK-123 (`0411073`, `d8676e0`) | die Zeile trug ihren Abschluss bereits im Text |
| NAK-96 | `202f2f8`, beglaubigt `f124746` | die Zeile endet seit 29.08.2026 mit „Geschlossen." |
| NAK-60 | `7020f06` | Abschluss seit 23.08.2026 im Text |
| NAK-62 | `94c5ab0` | Abschluss seit 23.08.2026 im Text |
| NAK-109 | `805e108` (PR1) | nachgemessen 02.09.2026: 0 Statusreste in `plan.json` |
| NAK-108 | `590918b` (dieses Ticket) | §44.2 trägt die Streichung jetzt wirklich |
| NAK-65 | `894dce7` auf User-Entscheid 31.08.2026 | Figma-Lieferung überholt, Ort jetzt S31b plus visuelle Spezifikation |

**5 Zeilen aus der Geschlossen-Tabelle zurückgeholt.** NAK-122, NAK-124, NAK-125, NAK-126 und
NAK-127 standen dort, nennen aber keinen schließenden Commit und beschreiben im ersten Satz offene
Arbeit („Offen: FL-User-Termin", „14 MEDIUM + 11 LOW", zwei Matrix-Fragen, „vor der nächsten
Installation"). Sie stehen jetzt in der Offen-Tabelle mit Klasse.

**Blockerliste je Phase** am Kopf: P3·G3 (NAK-74, NAK-112 — beide fallen gegen Gate 7), P3·S18–19
(NAK-57, NAK-106), P4·S20–22 (NAK-47, NAK-68), P6 (NAK-110, NAK-81, NAK-105), P7 (NAK-120,
NAK-107, NAK-125, NAK-126, NAK-82), P8 (NAK-79, NAK-44), P9 (NAK-119, NAK-127, NAK-30) und der
verhaltensneutrale Änderungssatz vor P4 (NAK-121, NAK-124).

**Tabellenzeilen berichtigt.** Die fünf aus `docs/beweise/PR1.md` §9.4 F bekannten Zeilen (NAK-74,
NAK-78, NAK-38, NAK-39, NAK-101) tragen ihr Pipe-Zeichen jetzt geschützt; der Inhalt ist
wortgleich. Das Register hat danach 0 uneinheitliche Tabellenzeilen (§8, Lauf F).

### 3.5 Fragenkarten (Commit 4)

`beantwortet{}` ist **nicht** verändert worden, bis auf das abgeleitete Feld `ergebnis`; `wahl`,
`text` und `datum` sind bei allen 52 Karten byteidentisch (Riegel im Edit-Skript, §8, Lauf F).

| Karte | Ergebnis 22.08.2026 | Nachgehalten 02.09.2026 |
|---|---|---|
| U9.3 | „Kein Entscheid — wird neu gestellt" (Messzustände der Suna-Kachel) | **geschlossen**, durch den Funktionsneustart überholt; Suna ist seit 23.08.2026 entfallen |
| U9.4 | „Kein Entscheid — wird neu gestellt" (Arbeitsschritte der Suna-Kachel) | **geschlossen**, gleiche Begründung |
| U6.8 | „Kein Entscheid — wird neu gestellt" (Schwebeschatten) | **geschlossen**, doppelt überholt: Suna entfallen, und Visuelles ist in dieser Phase kein Thema |
| U2.4 | „Folgefrage" (Kartenkopf: Zahl und Name) | **geschlossen**, in der Sache beantwortet: der Zeileninhalt von Gen Fläche 1 ist am 31.08.2026 entschieden |
| U2.9 | „Folgefrage" (Rollenwort in der Zeile) | **geschlossen**, in der Sache beantwortet: die Zeile führt Signalstatus, keine Rolle |
| U2.6 | „Folgefrage" (Ort für CERTAINTY und OTHER CAUSE) | **Folgekarte U21** — die Sache ist ausdrücklich offen (Abnahme 31.08.2026, „Bewusst offen") |

**Sechs neue Karten:** U21 (Beleg und Alternativerklärung auf Gen Fläche 1), U22 (Rollenliste fünf
oder sieben, NAK-38), U23 (Arbeitsnamen der beiden Gen-Flächen, Blueprint §15 Punkt 2), U24 (zwölf
Kernfunktionen bestätigen, NAK-39), U25 (Visual-Mixer-Workflow, NAK-111), U26 (FL-Termin
Host-Kanalkontext, NAK-122).

### 3.6 G3-Text (Commit 4)

Der Satz „Vorher fällig: deine fünf Entscheide aus U9" war falsch. Gemessen am 02.09.2026: alle
fünf U9-Karten sind beantwortet — U9.1, U9.2 und U9.5 tragen Entscheide, U9.3 und U9.4 sind mit
diesem Ticket als überholt geschlossen. Von G3 hängt keine offene Userantwort ab. Stattdessen
steht jetzt dort, was wirklich vorher fällig ist: **NAK-74 und NAK-112** — beide fallen gegen
Gate 7, das G3 falsifiziert — und die P3-Diagnoseentscheidung aus NAK-57.

### 3.7 Riegel und Planstand (Commit 5)

`tools/plan/dokuriegel.py` (neu, 346 Zeilen, nur Standardbibliothek) und die minimale Korrektur an
`tools/plan/planstand.py`. Beides in §6 und §8 belegt.

---

## 4. Prozessbilanz mit Zahlen

Alle Zahlen aus `git` und `tools/dirigent/rundenbilanz.py`, gemessen am 02.09.2026. Basis ist der
S0-Commit `b7d37ce`.

### 4.1 Commits seit S0

797 Commits. Ein Commit zählt einmal, nach den Pfadklassen von `rundenbilanz.py`:

| Klasse des Commits | Commits | Anteil |
|---|---:|---:|
| berührt Produkt oder Tests | 142 | 17,8 % |
| nur Doku | 462 | 58,0 % |
| nur Prüfwerkzeug | 50 | 6,3 % |
| nur Prüfwerkzeug + Doku | 40 | 5,0 % |
| gemischt/sonstiges | 103 | 12,9 % |

Berührte Dateipfade über alle Commits: Doku 1 694 · Produkt 1 215 · Sonstiges 948 ·
Prüfwerkzeug 404 · Tests 139.

**Anteil Werkzeug-Commits: 90 von 797 = 11,3 %** (reine Prüfwerkzeug-Commits plus
Prüfwerkzeug+Doku). Zusammen mit den reinen Doku-Commits berühren **82,2 % aller Commits weder
Produktcode noch Tests**.

### 4.2 Runden und Diff je Ticket

Zuordnung über den Commit-Betreff (jeder Commit trägt seinen Ticketnamen vorne). „davon Runde" =
Betreff enthält *Runde*, *Nacharbeit* oder ein `R<n>:`.

| Ticket | Commits | davon Runde | Änderung über die ganze Spanne |
|---|---:|---:|---|
| `SONDE-007a` (S8) | **76** | 76 | Produkt 9 Dateien **+533/−22** · Prüfwerkzeug 9 Dateien **+6 661/−243** · Doku 38 Dateien +92 520/−40 |
| `SONDE-010` (S14–15) | 18 | 13 | Produkt 15 Dateien +2 667/−373 · Tests 4 Dateien +830/−46 · Prüfwerkzeug 11 Dateien +1 947/−546 |
| `SONDE-011` (S16–17) | 18 | 7 | Produkt 105 Dateien +11 793/−389 · Tests 9 Dateien +4 856/−3 |
| `NAK-96` | 20 | 12 | Prüfwerkzeug 5 Dateien +407/−69 · Doku 12 Dateien +19 910/−8 — **ohne Produktfortschritt** |
| `NAK-123` | 9 | 4 | Produkt 22 Dateien +2 008/−291 · Tests 9 Dateien +1 814/−19 |
| `PR1` | 17 | 17 | Doku 8 Dateien +1 776/−66 — **ohne Produktfortschritt** (Planprüfung, so gewollt) |
| `PR2` (dieses Ticket) | 5 | 5 | Prüfwerkzeug 2 Dateien +345/−3 · Doku 8 Dateien +4 940/−246 — **ohne Produktfortschritt** (so gewollt) |
| `SONDE-005a` | 13 | 6 | — |
| `SONDE-008` | 11 | 7 | — |
| `G1` | 16 | 5 | — |
| `SONDE-012` | 7 | 2 | — |

**Der teuerste Befund steht in der ersten Zeile.** `SONDE-007a` hat in 76 Commits **533 Zeilen
Produktcode** und **6 661 Zeilen Prüfwerkzeug** erzeugt — Faktor 12,5 zugunsten des Werkzeugs, bei
92 520 Zeilen Doku. Genau das ist die Klasse, die die Prüfliste am 30.08.2026 mit der Rücknahme der
Regel „Aussagen-Inventar" adressiert hat (`tools/dirigent/pruefliste.md`, Abschnitt E): Runden, die
nur noch Befunde über sich selbst erzeugen.

### 4.3 Kanonläufe

62 Rohläufe unter `docs/beweise/roh/`, klassifiziert nach ihrer eigenen Urteilszeile:

| Urteil | Läufe |
|---|---:|
| GRUEN | 38 |
| ROT | 14 |
| NICHT BEGLAUBIGT | 1 |
| ohne Urteilszeile (Bruchproben, Teilläufe, Erstprüfungsauszüge) | 9 |

**Verlorene und abgebrochene Läufe, benannt:** `SONDE-011-70301d8-dirty.md` (ROT) und
`SONDE-011-70301d8-dirty-2.md` (NICHT BEGLAUBIGT) — derselbe Stand, zwei aufeinanderfolgende
verweigerte Beglaubigungen; `NAK-96-2271df5-dirty.md` und `-dirty-2.md` (beide ROT);
`SONDE-007a-70ca7f0.md`, `-dd15bbb.md`, `-fc9a193-dirty.md` (drei ROT im selben Ticket);
`G1-bdb9c07.md` und `G1-d20201e.md` (ROT, Ursache in NAK-113 als nicht reproduzierbar grünes Bein
belegt); `SONDE-012-1de3789-dirty.md` und `-22e1b6d-dirty.md` (ROT).

### 4.4 „Bauer ohne Compiler" — gemessen

Commit `c197fcd` vom 01.09.2026, Betreff wörtlich: *„NAK-123 Zwischenstand: Codex-Bau der
G2-Nacharbeit, nicht kompiliert, Tests NOT RUN"*.

| Größe | Wert |
|---|---|
| Produktcode in diesem einen Commit | 22 Dateien, **+1 902 / −278** |
| Tests | 9 Dateien, **+1 637 / −18** |
| Summe hinzugefügter Produkt- und Testzeilen | **3 539** |
| Laufstatus aller zugehörigen Tests | `NOT RUN` (`docs/beweise/NAK-123.md`, Abschnitt zur Spezifikationsphase) |
| Belegart | „Syntax-/Verdrahtungsbeleg, kein Laufbeleg" — Wortlaut des Manifests |

Der Gate-Text nennt „rund 2 800 Zeilen"; gemessen sind es **3 539 hinzugefügte Zeilen** Produkt
plus Tests in einem Commit ohne einen einzigen Lauf. Der User hat das am 01.09.2026 beantwortet:
*„opus wieder bauer und codex prüfer . so wie am anfang, habe genug wochen kontigent"* — Opus
baut, Codex prüft (Skill §3.6). Nach dieser Umstellung endete dasselbe Ticket am 02.09.2026 mit
Kanon **39/39 GRÜN** und zwei bestandenen Codex-Prüfungen.

### 4.5 Was die Zahlen sagen

1. **Die Prüfwerkzeuge sind teurer geworden als das, was sie prüfen.** In `SONDE-007a` steht
   Prüfwerkzeug zu Produktcode 12,5 : 1. `NAK-100` hat später 1 400 Zeilen A17-Prüfcode wieder
   zurückgebaut — die Korrektur ist gelaufen, die Klasse aber nicht abgestellt.
2. **Vier von fünf Commits berühren weder Produkt noch Test.** Das ist für ein
   beweisgetriebenes Projekt nicht per se falsch — Manifeste sind der Beweis —, aber 462 reine
   Doku-Commits gegen 142 mit Produkt oder Tests ist ein Verhältnis, das man kennen sollte, bevor
   P4 bis P9 beginnen.
3. **Ein Bau ohne Compiler kostet eine ganze Runde.** `c197fcd` musste vollständig
   nachgearbeitet werden; erst der Opus-Worker hat den Stand kompiliert, gemessen und grün
   gemacht.
4. **Rote Kanonläufe sind normal, verweigerte Beglaubigungen nicht.** Ein `NICHT BEGLAUBIGT`
   heißt: gebaute Artefakte waren älter als ihre Quellen. Das ist genau der Fall, den
   `planstand.py` seit NAK-96 nicht mehr als grün durchreicht.

Technische Folgen stehen in §6 als **Vorschläge an den Dirigenten** — dieses Ticket setzt sie
nicht um, weil Skill und Dirigentenregeln dem Dirigenten gehören. Produktfolgen gibt es aus dieser
Bilanz keine; es ist keine Fragenkarte daraus entstanden.

---

## 5. Werkzeugreste — bewertet, nicht entsorgt

Nichts gelöscht, nichts gedroppt. Beide Reste sind als datierte Registerzeilen der Klasse
**Werkzeug** eingetragen (NAK-128, NAK-129).

### 5.1 `.workflow/ultracode/20260831-084621-sonde011-phaseb-bau`

| Größe | Wert |
|---|---|
| Zustand | untracked, nicht in `.gitignore` — erscheint in jedem `git status` |
| Umfang | 2 375 Dateien, 76 MB |
| davon Baucache | `cargo-home` 55 MB, `rust-vendor` 21 MB |
| eigentliche Blätter | 3 785 Byte: `plan.md` (2 507 B), fünf Paketblätter (178–212 B), `integration.md` (171 B), `verification.md` (144 B) |
| Paketstatus | 01 „in Arbeit", 02–05 „ausstehend" |
| Journale | leer: „Noch keine Integrationsergebnisse", „Noch keine Testläufe" |

**Liegt etwas davon nirgends im Repo?** Nein. Der Bauplan beschreibt genau die Arbeit, die
`SONDE-011` Phase B gebaut hat und die als S16–17 mit T2 PASS 2026-09-01 abgenommen ist; die
Verhaltensmatrix, auf die er sich beruft, steht vollständig in `docs/beweise/SONDE-011.md`
§B.1–B.6. Die beiden Journale sind leere Stümpfe — der Lauf hat nie Ergebnisse geschrieben.

**Empfehlung:** löschbar. Wer es tut, löscht den ganzen Ordner und trägt `.workflow/` in
`.gitignore` nach, sonst taucht der nächste Lauf wieder als Störung in `git status` auf.
**Nicht ausgeführt** — Löschen ist kein Teil dieser Planprüfung.

### 5.2 `stash@{0}` vom 28.08.2026

Betreff: „dirigent 2026-08-28: lokale codex-context-pflege-Aenderungen vor Pull
(Remote-Laptop-Stand)". Zwei Dateien, +82/−12.

| Datei | Änderung |
|---|---|
| `.agents/skills/codex-context-pflege/SKILL.md` | +5: lokale User-Skills auch unter `$CODEX_HOME/skills` (ohne `.system`) und `~/.agents/skills`; Repo-Skills entlang der Projektkette aus `.agents/skills` |
| `.agents/skills/codex-context-pflege/scripts/audit-context.ps1` | +89/−12: neuer Leser `Get-FrontmatterScalar` für YAML-Blockskalare (`>`/`\|` mit `+`/`-`), damit gefaltete Metadaten korrekt gekostet statt als leerer Wert gelesen werden |

**Liegt etwas davon nirgends im Repo?** **Ja.** Beide Änderungen fehlen in HEAD, dreifach
gemessen: `grep "Als lokale User-Skills gelten sowohl"` → 0 Treffer in `SKILL.md`,
`grep "Get-FrontmatterScalar"` → 0 Treffer in `audit-context.ps1`, und
`git stash show -p stash@{0} | git apply --check --reverse` schlägt fehl.

**Empfehlung:** nicht droppen. Als eigenes kleines Werkzeugticket anwenden und prüfen, oder
bewusst verwerfen — beides ein Entscheid, kein Nebeneffekt. **Nicht ausgeführt** — weder `apply`
noch `pop` noch `drop` sind Teil dieser Planprüfung; gelesen wurde ausschließlich mit
`git stash list` und `git stash show`.

---

## 6. Vorschläge an den Dirigenten (nicht eingearbeitet)

Skill und Dirigentenregeln gehören dem Dirigenten. Diese fünf Punkte fallen aus §4 und §5 und
sind hier nur benannt.

| # | Vorschlag | Grund aus der Messung |
|---|---|---|
| V-1 | **Rundenbudget an den Prüfwerkzeug-Anteil koppeln.** Wenn eine Runde mehr Zeilen Prüfwerkzeug als Produkt+Tests erzeugt, ist das ein eigenes Signal — heute misst `rundenbilanz.py` nur „Produkt+Tests = 0". | `SONDE-007a`: 76 Commits, 533 Produktzeilen gegen 6 661 Prüfwerkzeugzeilen. Das Signal „ohne Produktfortschritt" schlug nie an, weil 533 > 0 ist. |
| V-2 | **`.workflow/` in `.gitignore`.** Ein Werkzeugordner, der bei jedem `git status` als Störung erscheint, kostet in jeder Session Aufmerksamkeit. | §5.1; der Ordner steht seit dem 31.08.2026 untracked im Baum. |
| V-3 | **NAK-97 zusammen mit dem nächsten `planstand.py`-Anfassen schließen.** Zwei kleine Reste (`--no-optional-locks` fehlt, die `KANON`-Regex trifft den ROT-Fall) liegen in derselben Datei, die dieses Ticket minimal korrigiert hat. Sie lagen außerhalb des Auftrags und sind bewusst nicht angefasst worden. | Register NAK-97; `planstand.py:95` und `:66`. |
| V-4 | **Den Dokuriegel in den Kanon oder in den Dirigenten-Abschluss hängen.** Er läuft in unter einer Sekunde und findet genau die zwei Fehlerklassen, die in Plandokumenten still verderben. | Erster Lauf über sieben Dateien: 6 echte Befunde, alle vorbestehend (§8, Lauf F). |
| V-5 | **Werkzeugticket für `stash@{0}` anlegen oder den Stash bewusst verwerfen.** Er trägt seit dem 28.08.2026 Arbeit, die nirgends sonst liegt; ein Stash ist kein Aufbewahrungsort. | §5.2. |

---

## 7. Prüfliste `tools/dirigent/pruefliste.md`

**A–D sind für ein Dokumentticket nicht anwendbar** — kein Rückstauverhalten, kein Lebenszyklus,
kein Längen- oder Alphabetvertrag und kein Bau-/Prüfriegel am Produkt. Es gibt keinen Produktcode,
keinen Test und kein Schema im Änderungssatz (Beleg: `git diff --stat` in §8, Lauf D, berührt
`docs/**`, `design/docs/**`, `CLAUDE.md` und `tools/plan/**`).

### E. Behauptung ≤ Messung

| Zeile | Wo gemessen |
|---|---|
| „Jede Behauptung sagt nicht mehr, als der Test misst" | Jede Zahl in §1.2 und §4 stammt aus einem Lauf in §8 und nennt ihn. Wo eine Negativaussage steht („liegt nirgends im Repo", „0 Statusreste"), steht der Befehl mit seiner Trefferzahl daneben (§5.2, §3.4 NAK-109). Die Kanon-Beinanzahl wird hier **nicht** behauptet — der Kanon läuft nicht (§8). |
| „Zahlen im Manifest sind gemessen, nicht abgeschrieben" | §1.2 nennt zu jeder Zahl den Lauf. Die Zahl „86 offene Zeilen" aus dem Gate-Text ist nachgemessen und bestätigt (86 vor der Triage, 79 danach). Die Zahl „16 Punkte" aus dem Gate-Text ist nachgemessen und **korrigiert**: es sind 17 (§3.3). Die „rund 2 800 Zeilen" aus dem Gate-Text sind nachgemessen und präzisiert: 3 539 (§4.4). |
| „Positionen im lebenden Kopf stehen als Symbol/Anker" | Fundstellen sind Datei plus Symbol oder §-Nummer. Weil Fassung 0.5 **nicht umnummeriert**, bleiben alle §-Verweise aus fremden Manifesten gültig; der Kopf des Entwurfs sagt das ausdrücklich. Zeilennummern stehen nur dort, wo sie zum Stand `55cdb91` gemessen wurden, und tragen den Dateinamen. |
| „Der lebende Kopf wird beim Abschluss nachgezogen" | Dieses Manifest hat nur einen Kopf und keinen Verlauf — es ist die erste Runde. §10 nennt den Nachlauf. |
| „Jede neue Prüfung wurde einmal absichtlich gebrochen" | **Zutreffend und ausgeführt.** `tools/plan/dokuriegel.py` ist die einzige neue Prüfung; beide Riegel sind einzeln gegen einen absichtlich herbeigeführten Verstoß gefahren, Rohausgabe des Rots in §8, Lauf F. Dazu die Riegel in den Edit-Skripten (Roundtrip, `beantwortet{}`-Menge, Trefferzahl je Anker, Spaltenzahl) — jeder Skriptlauf bricht bei Abweichung ab; §8, Lauf F nennt die zwei, die während der Arbeit tatsächlich ausgelöst haben. |
| „Geänderte Zusage — drei Stellen" | Zutreffend. Zwei Zusagen sind geändert: (1) die Quelle für S31b — nachgezogen in `plan.json`, `bauaufteilung-sonden.md` §1.4 **und** §3, und im Register (NAK-65, NAK-82); (2) die Regel „Text unter dem Errata-Block nie umschreiben" — nachgezogen im Entwurfskopf, in `bauaufteilung-sonden.md` §6.4 und im Register (NAK-108). Der Runnerkopf `tools/beweise.ps1` ist nicht betroffen, weil keine Runner-Behauptung berührt wird. |
| „Writer-Fixtures statt Handschrift" | greift nicht — keine Fixtures im Änderungssatz. |

### F. Änderungssatz

| Zeile | Wo gemessen |
|---|---|
| „speichern↔laden, starten↔stoppen, … im selben Commit" | Zwei Gegenpaare sind berührt, beide vollständig: (1) **Kopie↔Original** — die Archivkopie und die Bearbeitung des lebenden Pfades liegen in Commit 1, mit Byteidentitätsbeweis (§8, Lauf G); (2) **schließen↔zurückholen** im Register — die 14 Schließungen und die 5 Rückholungen liegen in Commit 4, dieselbe Datei, derselbe Änderungssatz. |
| „Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines Vertrags im selben Änderungssatz" | **Greift hier nicht als Pflicht, aber als Befund:** dieses Ticket ändert keinen Vertrag. Es stellt fest, welche Verträge einen brauchen, und ordnet sie zu: Band-Belegung und Remove-Undo (S26–28, gespiegelt in S28b), `Frame.band_dynamic_gain_db` (S26–28/S28b), 10-Sekunden-Kandidat, direkte Edit-Transaktion, zweistufiger Mix und Main-Ingress (alle S29–31). Genau deshalb sind sie als Planarbeit eingetragen und nicht nebenbei gemacht. |

---

## 8. Beweislauf

**Befehle, Reihenfolge und Exitcodes.** Gefahren am 2026-09-02 vom Workspace-Root
`C:/Users/phili/Projekte/Nakama`. Rohausgabe ungekürzt.

```text
### A) py -3.13 tools/plan/planstand.py
geschrieben: docs\PLAN-STAND.md (23 abgenommen, 1 gebaut, 39 gesamt, aus cf21476)
EXITCODE=0

### B) py -3.13 tools/plan/antworten_blatt.py
geschrieben: docs\ANTWORTEN-OFFEN.md (52 Antworten, 0 offen, 52 eingearbeitet, 23 mit eigenem Text)
EXITCODE=0

### C) JSON-Gueltigkeit aller docs/plan/*.json
docs/plan/fragen.json -> gueltiges JSON, 34212 Bytes
docs/plan/plan.json -> gueltiges JSON, 80454 Bytes
EXITCODE=0

### D) git diff --stat 55cdb91..HEAD   (Stand vor dem Manifest-Commit)
 CLAUDE.md                                          |   15 +-
 design/docs/funktions-und-bedien-blueprint.md      |   21 +-
 docs/FL-Nakama-Sonden-Design-Entwurf.md            |  507 ++-
 ...-Nakama-Sonden-Design-Entwurf-0.4-2026-08-20.md | 4238 ++++++++++++++++++++
 docs/bauaufteilung-sonden.md                       |   81 +-
 docs/offene-punkte.md                              |  230 +-
 docs/plan/fragen.json                              |   78 +-
 docs/plan/plan.json                                |   16 +-
 tools/plan/dokuriegel.py                           |  332 ++
 tools/plan/planstand.py                            |   16 +-
 10 files changed, 5285 insertions(+), 249 deletions(-)
EXITCODE=0

### E) Zaehlungen
Errata mit Einarbeitung im Text: 17/17  abcdefghijklmnopq
  Marken '[0.5 - Erratum (x) ...]': 25
  Grabsteinzeilen 'gestrichen in Fassung 0.5': 23
  nummerierte Ueberschriften 0.4 / 0.5: 255 / 257, nur in 0.5: ['Rangfolge', 'Wie']
Registerzeilen: offen 79, geparkt 5, geschlossen 31, Summe 115
  Klasse Härtung/Struktur   31
  Klasse Planarbeit         18
  Klasse Produktfrage       6
  Klasse Werkzeug           24
  Summe der Klassen: 79 (muss offen entsprechen)
  Doppelte zwischen offen und geschlossen: []
  Planarbeit je Schritt: {'G3': 2, 'S18–19': 2, 'S20–22': 2, 'S26–28': 2, 'S28b': 1, 'S29–31': 4, 'S31b': 1, 'S32–33': 1, 'S34–35': 3}
  uneinheitliche Tabellenzeilen: keine
Fragenkarten: offen 8 (U11, U13, U21, U22, U23, U24, U25, U26), beantwortet 52
  am 02.09.2026 nachgehalten: 6 (U2.4, U2.6, U2.9, U6.8, U9.3, U9.4)
Blueprint §15: 17 Punkte (1 bis 17)
  in plan.json zugeordnete Punktnummern: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17]
  ohne Zuordnung: []
  2026-08-31-technische-ui-architektur-arbeitsmodus.md       -> S18–19, PR2, S23–25, S26–28, S29–31, S31b
  2026-09-01-technische-ui-architektur-fuenferblock-01.md    -> PR2, S26–28, S28b, S31b
  2026-09-01-technische-ui-architektur-fuenferblock-02.md    -> S26–28, S28b, S31b
  2026-09-01-gen-nur-standardgroesse.md                      -> S18–19, PR2, S31b
  2026-08-31-eq-arbeitsflaeche-dominiert.md                  -> S31b
  2026-09-01-dynamic-ansicht-im-band-panel.md                -> S31b
  2026-09-01-dynamisches-band-schwingt-mit.md                -> S26–28, S31b
EXITCODE=0

### F) py -3.13 tools/plan/dokuriegel.py <die sieben Ticketpfade>
docs/plan/plan.json: sauber
docs/bauaufteilung-sonden.md: sauber
docs/FL-Nakama-Sonden-Design-Entwurf.md: sauber (1 benannt)
design/docs/funktions-und-bedien-blueprint.md: sauber
CLAUDE.md: sauber
docs/offene-punkte.md: sauber (8 benannt)
docs/beweise/PR2.md: sauber
docs/FL-Nakama-Sonden-Design-Entwurf.md:189 VERWEIS OHNE ZIEL, aber als historisch markiert („im Design-Repo“ im Umfeld): docs/sondenplan.md
docs/offene-punkte.md:76 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („nicht ins Repo“ im Umfeld): tools/extern/
docs/offene-punkte.md:79 VERWEIS OHNE ZIEL, aber als historisch markiert („im FL-Studio-Repo“ im Umfeld): tools/analyze-track.py
docs/offene-punkte.md:97 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): docs/hub/hub.json
docs/offene-punkte.md:97 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): tools/hub/hub_sync.py holen\|senden
docs/offene-punkte.md:107 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): eq-copilot/fixtures/installer/journale/
docs/offene-punkte.md:107 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): tools/eq-copilot/erzeuge_installer_journale.py
docs/offene-punkte.md:112 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/kern_riegel/
docs/offene-punkte.md:112 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/installer_pruefung/
Dokuriegel: 0 Befund(e), 9 benannt (historisch oder Zielpfad)
EXITCODE=0

### F1) derselbe Riegel, absichtlich gebrochen
    Bruch 1 (Tabellen): Kopie von docs/offene-punkte.md, Pipe in der NAK-74-Zelle wieder ungeschuetzt.
    Bruch 2 (Verweise): Kopie von CLAUDE.md mit einem zusaetzlichen Lesetabelleneintrag auf eine Datei, die es nicht gibt.
C:/Users/phili/.claude/jobs/528b49d1/tmp/riegelbruch/bruch-tabelle.md:59 TABELLE UNEINHEITLICH: 7 Spaltentrenner statt 5 (Tabelle ab Zeile 43)
C:/Users/phili/.claude/jobs/528b49d1/tmp/riegelbruch/bruch-verweis.md:139 VERWEIS INS LEERE: docs/gibt-es-nicht.md
Dokuriegel: 2 Befund(e), 8 benannt (historisch oder Zielpfad)
EXITCODE=1

### F2) Riegel in den Edit-Skripten, die waehrend der Arbeit rot geworden sind
ABBRUCH Edit 1: 0 Treffer statt 1 fuer '       docs/plan/plan.json traegt denselben Schnitt. Spielregeln der UI: ...'
    -> falscher Anker (sieben statt zwei Leerzeichen Einzug); nach Korrektur gruen.
ABBRUCH Spaltenriegel Zeile 118: 7 Zellen   (neue Registerzeile NAK-129)
    -> ungeschuetztes Pipe im Zitat eines git-Aufrufs; nach dem Schutz gruen.

### G) Archiv-Byteidentitaet (SHA-256)
archiv  HEAD   : a8230e09b485cb18808fd867ede3d3ceeed7fcc0ec5c4d38a8fe7a97cde3e7c5 237425 Bytes
entwurf 55cdb91: a8230e09b485cb18808fd867ede3d3ceeed7fcc0ec5c4d38a8fe7a97cde3e7c5 237425 Bytes
BYTEIDENTISCH
EXITCODE=0

### H) py -3.13 tools/dirigent/rundenbilanz.py 55cdb91..HEAD
55cdb91..HEAD: Pruefwerkzeug 2 Datei(en) +345/-3 | Doku 8 Datei(en) +4940/-246
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
EXITCODE=0
```

### Was die Läufe zeigen

- **A** `planstand.py` endet mit **Exitcode 0 und ohne Warnung**: 23 abgenommen, 1 gebaut
  (dieses Manifest), 39 gesamt. Zwei Dinge stehen dahinter. (1) Die Korrektur aus §3.7 hat
  während der Arbeit gewirkt: solange `docs/beweise/PR2.md` fehlte, nannte das Blatt `PR2`
  als nächsten Schritt statt `G3` — vorher übersprang es `PR2` und nannte `G3`. Seit das
  Manifest liegt, ist `G3` wieder richtig. (2) **Ein Fehlgriff im ersten Lauf, hier
  festgehalten:** §1.3 zitierte G2s Urteilsmarke zunächst als HTML-Kommentar.
  `marken_lesen()` liest jeden solchen Kommentar in einem Manifest als dessen **eigenes**
  Urteil — der Lauf meldete daraufhin „24 abgenommen" und führte `PR2` als abgenommen,
  obwohl es nur `T1` trägt. Dieselbe Falle hat PR1 am 30.08.2026 getroffen. Die Stelle
  nennt die Marke jetzt als Text; der Riegel hat also getan, wofür er gebaut ist.
- **B** `antworten_blatt.py`: 52 Antworten, 0 offen, 52 eingearbeitet — `beantwortet{}` hat
  dieselbe Kartenmenge wie vorher; verändert ist ausschließlich das abgeleitete `ergebnis`-Feld
  von sechs Karten.
- **C** Beide Planquellen sind nach den Edits gültiges JSON. Zusätzlich hat jedes Edit-Skript vor
  dem Schreiben geprüft, dass `json.dumps(…, indent=1, ensure_ascii=False)` mit den vorgefundenen
  Zeilenenden **bytegleich** zur Ausgangsdatei ist — `plan.json` mit LF, `fragen.json` mit CRLF.
  Die Edits ändern damit nur Text, keine Formatierung.
- **D** Der Änderungssatz berührt `docs/**`, `design/docs/funktions-und-bedien-blueprint.md`,
  `CLAUDE.md` und `tools/plan/**`. **Kein Produktcode, kein Test, kein Schema, kein Fixture, kein
  Runner.** Von den 5 285 hinzugefügten Zeilen sind 4 238 die byteidentische Archivkopie.
  Der Lauf ist vor dem Manifest-Commit gefahren; er zeigt die Commits 1 bis 5. Commit 6 fügt
  `docs/beweise/PR2.md`, die zwei gerechneten Blätter und drei Planschritt-Nachträge hinzu und
  bleibt in derselben Grenze — kein Commit dieses Tickets berührt Produktcode, Test oder Schema.
- **E** Die Zählungen aus §1.2 nach dem Änderungssatz.
- **F** Der Dokuriegel läuft über die sieben Ticketpfade **sauber, Exit 0**: 0 Befunde,
  9 benannt (7 historisch markiert, 2 als noch nicht angelegte Zielpfade). Darunter der
  absichtlich gebrochene Lauf.
- **G** Die Archivkopie ist zum 0.4-Blob **byteidentisch**: gleicher SHA-256, gleiche Bytelänge.
- **H** Die Rundenbilanz dieses Tickets: kein Produktfortschritt — für eine Planprüfung das
  erwartete Ergebnis, hier ausdrücklich als solches gemeldet und nicht als Mangel.

### Warum der Kanon nicht läuft

`tools/beweise.ps1` baut und misst Produktcode, Tests, Schemas und Fixtures. Dieser Änderungssatz
berührt nichts davon (Lauf D). Ein Kanon-Lauf würde denselben Stand messen wie der letzte Lauf zu
`NAK-123` (GRÜN 39/39 auf `d8676e0`) und über dieses Ticket nichts aussagen. Er wird deshalb
bewusst **nicht** gefahren und auch nicht als Beleg zitiert.

---

## 9. Selbstaudit (T1)

Der Gesamtdiff `55cdb91..HEAD` ist adversarial gegen die im Auftrag benannten Klassen gelesen
worden:

| Frage | Befund |
|---|---|
| Ein erfundener Entscheid? | Nein. Jede Einarbeitung trägt Buchstabe oder Datum und die User-Wort-Quelle; wo kein Entscheid vorlag, steht „offen", „nicht entschieden" oder eine Fragenkarte. Die sechs neuen Karten stellen Fragen, sie beantworten keine. |
| Ein verlorener Entscheid? | Nein. Der Errata-Block bleibt vollständig stehen, die Fassung 0.4 liegt byteidentisch im Archiv, keine Registerzeile ist gelöscht (79 + 5 + 31 = 115 gegen vorher 86 + 5 + 22 = 113, plus 2 neue). |
| Ein Verweis ins Leere? | Nein — `dokuriegel.py` misst es, Exit 0 (§8, Lauf F). |
| Eine Zahl, die nicht gemessen wurde? | Nein. Drei Zahlen des Gate-Textes sind nachgemessen und zwei davon korrigiert (16 → 17 Blueprint-Punkte; „rund 2 800" → 3 539 Zeilen). |
| Ein verändertes User-Zitat? | Nein. `wahl`, `text` und `datum` aller 52 beantworteten Karten sind unverändert (Riegel im Edit-Skript). Zitate im Entwurf und im Register sind aus den Abnahmedateien kopiert, nicht nacherzählt. |
| Ein umnummerierter Paragraph? | Nein — 255 nummerierte Überschriften vorher wie nachher (§8, Lauf E). |
| Eine fremde Datei im Commit? | Nein. `.workflow/ultracode/…` ist in keinem der sechs Commits; jeder Commit nennt seine Pfade explizit (§3). |
| Eine ungeprüfte Behauptung über gebauten Code? | Nein. Jede Aussage über `SONDE-011`, die Reservierungen und den Broker-Autostart ist an der Quelle gelesen (§2.2), nicht aus dem Manifest übernommen. |

---

## 10. Nicht erledigt

1. **Die T3-Marke fehlt.** Dieses Manifest trägt nur `T1 PASS`. Die geforderte Stufe ist **T3**;
   sie setzt der Dirigent nach dem Urteil eines frischen Codex-Threads. Bis dahin führt der
   Planstand `PR2` korrekt als *gebaut, nicht abgenommen*.
2. **Der Kanon ist nicht gelaufen** — mit Begründung (§8).
3. **Die Werkzeugreste sind bewertet, nicht beseitigt.** Der 76-MB-Ordner liegt weiter untracked
   im Baum, der Stash weiter im Stash. Beides ist Absicht (§5) und als Registerzeile und
   Dirigentenvorschlag festgehalten.
4. **NAK-97 ist nicht geschlossen**, obwohl dieses Ticket `planstand.py` angefasst hat. Die zwei
   Reste liegen außerhalb des Auftrags (E7 verlangt ausdrücklich eine *minimale* Korrektur der
   „Als Nächstes"-Logik); sie stehen als Vorschlag V-3 in §6.
5. **Die Klasse „blockiert P4–P7" heißt im Register „Planarbeit".** Der Gate-Text nennt sie so,
   aber zwei Zeilen (NAK-119, NAK-127) und zwei weitere (NAK-30, NAK-79) gehören zu S32–33 und
   S34–35, also zu P8 und P9. Die Klasse trägt deshalb den konkreten Schritt im Namen statt einer
   Phasenspanne; die Zuordnung selbst ist vollständig.
6. **Die visuelle Spezifikation für Gen Fläche 2 gibt es nicht.** S31b hängt jetzt daran; ihr
   Beginn ist ein eigener User-Schritt (`CLAUDE.md`) und wurde hier nicht vorweggenommen.
7. **Blueprint §15 ist nicht gekürzt.** Die 17 Punkte tragen jetzt je eine Schritt-ID im Plan,
   aber der Blueprint selbst führt sie unverändert als offen — sie sind zugeordnet, nicht
   geschlossen.
