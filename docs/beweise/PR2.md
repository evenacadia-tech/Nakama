# Beweismanifest — PR2 «Planprüfung: Quellen zusammenführen, Design-Entscheide in den Bauplan holen, Register bereinigen»

<!-- NAKAMA-URTEIL: T1 PASS 2026-09-02 -->
<!-- NAKAMA-URTEIL: T3 PASS 2026-09-02 -->

> **Urteil T3 PASS, gesetzt vom Dirigenten am 02.09.2026 (§14).** Drei frische Codex-Threads
> (`gpt-5.6-sol`, Effort max, lesend); nach Runde 2 ist kein Defekt offen. Das Selbstaudit T1 PASS
> des Erbauers (§9) bleibt als eigene Marke stehen.

| Feld | Wert |
|---|---|
| Ticket | `PR2` (Planprüfung, kein Bauticket) |
| Phase / Session | P3, zwischen S18–19 (`SONDE-012`) und G3 |
| Gate-Text (Quelle) | `docs/plan/plan.json`, Schritt `PR2` — wörtlich in §1.1 |
| Basis-SHA | `55cdb91599239aa99ec1322e23d109ab9424d7b2` (Branch `master`) |
| Datum | 2026-09-02 |
| Prüfstufen | T1 ☑ · **T3: Erstprüfung NEEDS_WORK 02.09.2026, Runde 1 nachgearbeitet, Wiederprüfung 1 NEEDS_WORK (D8: NAK-85, NAK-92), Runde 2 nachgearbeitet, Wiederprüfung 2 PASS 02.09.2026** — die geforderte Stufe **T3** ist erreicht (§12, §13, §14) |
| Kanon | **läuft nicht** — kein Produktcode, kein Test, kein Schema angefasst (Begründung §8) |
| Änderungssatz | Erstdurchgang: 10 Dateien, +5 285 / −249 Zeilen (§8, Lauf D); davon 4 238 Zeilen die byteidentische Archivkopie. **Runde 1 kommt dazu — gemessen in §12.4, Lauf D.** |

---

## 1. Kopf

### 1.0 Runden dieses Tickets

| Runde | Wofür | Stand am Ende | Urteil des frischen Prüfers |
|---|---|---|---|
| Erstdurchgang | Gate-Text PR2 in fünf Teilen (§1 bis §11) | `16be249` | **NEEDS_WORK** — Codex `gpt-5.6-sol`, Effort max, lesend, Thread `01a060fd-b78e-7d90-aee6-591dab8dd943`, 14 bestätigte Defekte (§12.1) |
| 1 (02.09.2026) | die 14 Defekte D1 bis D14, nichts sonst | `6b2539f` (siehe §12.3 und §12.7) | **NEEDS_WORK** — Codex `gpt-5.6-sol`, Effort max, lesend, Thread `01a06193-0eb9-7a42-b12d-4fe8a0bb9b38`: „D1–D7 und D9–D14 sind geschlossen; D8 bleibt für NAK-85 und NAK-92 offen“ (§13.1) |
| 2 (02.09.2026) | der Rest von D8 an NAK-85 und NAK-92 samt seiner Ursache, nichts sonst | `dca7654` bis zum Nachlauf (siehe §13.4 und §13.7) | **PASS** — Codex `gpt-5.6-sol`, Effort max, lesend, Thread `01a061b7-6c47-7201-834f-35c5444c3344`, Fixdiff `6b2539f...e51a7d3` (§14) |

Diese Tabelle ist der lebende Kopf des Tickets: jede weitere Runde bekommt hier eine Zeile,
bevor sie ihren eigenen Abschnitt bekommt.

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

**Stand dieser Tabelle: Erstdurchgang, `16be249`.** Runde 1 hat fünf dieser Zahlen bewegt; die
neuen Werte stehen in §12.4 (Lauf E) und sind in der Spalte *Messort* mit „berichtigt §12.4“
vermerkt. Zwei weitere Zahlen — Fragenkarten und Blueprint-§15-Punkte — hat **nicht** diese
Runde bewegt, sondern die parallel laufende Design-Session desselben Tages (Fünferblöcke 03
bis 05); auch sie sind in §12.4 neu gemessen, damit der Kopf nicht falsch stehen bleibt.

| Größe | Wert | Messort |
|---|---:|---|
| Errata mit Einarbeitung an ihrer Stelle | **17 / 17** (a–q) | §8, Lauf E |
| Marken `[0.5 · Erratum (x) · Datum]` im Text | 25 | §8, Lauf E |
| Grabsteinzeilen „gestrichen in Fassung 0.5" | 23 | §8, Lauf E |
| nummerierte Überschriften 0.4 → 0.5 | **190 → 190**, gleiche Reihenfolge, keine hinzu, keine weg | §8, Lauf E |
| alle Überschriften Ebene 2–3 | 255 → 257; die zwei neuen sind **unnummeriert** (`## Rangfolge …`, `## Wie Fassung 0.5 …`) | §8, Lauf E |
| Archivkopie byteidentisch zu 0.4 | SHA-256 `a8230e09…3e7c5`, 237 425 Bytes, beidseitig gleich | §8, Lauf G |
| Blueprint-§15-Punkte mit Schritt-ID | **17 / 17** (der Gate-Text nannte 16; Punkt 17 kam am 02.09.2026 dazu) | §8, Lauf E — **heute 19 Punkte: 18 und 19 sind am 02.09.2026 von der Design-Session ergänzt und in `plan.json` S31b zugeordnet worden, §12.4** |
| Registerzeilen offen / geparkt / geschlossen | 79 / 5 / 31 = 115 | §8, Lauf E — **berichtigt §12.4: 64 / 5 / 60 = 129** |
| davon Planarbeit / Härtung / Werkzeug / Produktfrage | 18 / 31 / 24 / 6 = 79 | §8, Lauf E — **berichtigt §12.4: 20 / 18 / 20 / 6 = 64** |
| Registerzeilen neu geschlossen | 14 | §3.4 — **in Runde 1 kamen 16 umgezogene und 13 zurückgeholte dazu, §12.2 D1/D2/D8** |
| Registerzeilen aus der Geschlossen-Tabelle zurückgeholt | 5 | §3.4 |
| neue Registerzeilen (Werkzeugreste) | 2 (NAK-128, NAK-129) | §5 |
| Fragenkarten offen | 2 → **8** (U21–U26 neu) | §8, Lauf E — **heute 7: U21 ist am 02.09.2026 von der Design-Session beantwortet worden, §12.4** |
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
| 6 | `1184b25` | `docs/beweise/PR2.md`, `docs/PLAN-STAND.md`, `docs/ANTWORTEN-OFFEN.md`, `docs/plan/plan.json`, `tools/plan/dokuriegel.py` |
| 7 | Nachlauf (§11) | `docs/beweise/PR2.md`, `docs/PLAN-STAND.md` — lebender Kopf nachgezogen |

### 3.1 Entwurf 0.5 (Commit 1)

**Archivkopie zuerst.** `docs/FL-Nakama-Sonden-Design-Entwurf.md` wurde **kopiert**, nicht
verschoben: der lebende Pfad behält seine Blame-Historie, das Archiv ist eine Momentaufnahme.
Ein `git mv` hätte die Historie auf den Archivpfad umgelenkt und den lebenden Pfad als neue Datei
erscheinen lassen. Byteidentität in §8, Lauf G.

**§-Nummerierung stabil.** 190 nummerierte Überschriften vorher wie nachher, **in identischer
Reihenfolge**, keine hinzugekommen und keine weggefallen (§8, Lauf E). Die zwei neuen
Überschriften im Kopf tragen bewusst keine Nummer. Ein gestrichener Abschnitt behält Überschrift **und** Nummer und trägt eine
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

> **Berichtigt am 02.09.2026 in Runde 1 (§12.2, Befunde D1, D2, D7, D8).** Die Zahlen dieses
> Abschnitts galten für den Erstdurchgang. Heute sind es **64 offene Zeilen** — Planarbeit 20,
> Härtung/Struktur 18, Werkzeug 20, Produktfrage 6 — sowie 5 geparkte und 60 geschlossene,
> Summe **129**. Der Erstdurchgang hatte 13 geschlossene Zeilen gelöscht (sie sind wortgleich
> zurück) und 16 Zeilen mit eigener Schließung in der Offen-Tabelle stehen lassen (sie sind
> umgezogen). Die Tabelle darunter bleibt als Stand des Erstdurchgangs stehen.

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
| V-3 | **NAK-97 zusammen mit dem nächsten `planstand.py`-Anfassen schließen.** Zwei kleine Reste (`--no-optional-locks` fehlt, die `KANON`-Regex trifft den ROT-Fall) liegen in derselben Datei, die dieses Ticket minimal korrigiert hat. Sie lagen außerhalb des Auftrags und sind bewusst nicht angefasst worden. | Register NAK-97; `planstand.py:95` und `:66`. **Gegenstandslos, berichtigt am 02.09.2026 (Runde 1, §12.2 Befund D8):** beide Reste sind seit `da1b04e` behoben — an der Quelle gemessen, `--no-optional-locks` steht in `tools/plan/planstand.py:151`, und die `KANON`-Regex trifft den ROT-Fall (`:102`, Zweig `rot_fehl`/`rot_ges`). NAK-97 ist deshalb in Runde 1 geschlossen worden. |
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
geschrieben: docs\PLAN-STAND.md (23 abgenommen, 1 gebaut, 39 gesamt, aus 1184b25)
EXITCODE=0

### B) py -3.13 tools/plan/antworten_blatt.py
geschrieben: docs\ANTWORTEN-OFFEN.md (52 Antworten, 0 offen, 52 eingearbeitet, 23 mit eigenem Text)
EXITCODE=0

### C) JSON-Gueltigkeit aller docs/plan/*.json
docs/plan/fragen.json -> gueltiges JSON, 34212 Bytes
docs/plan/plan.json -> gueltiges JSON, 80454 Bytes
EXITCODE=0

### D) git diff --stat 55cdb91..HEAD
 CLAUDE.md                                          |   15 +-
 design/docs/funktions-und-bedien-blueprint.md      |   21 +-
 docs/ANTWORTEN-OFFEN.md                            |   12 +-
 docs/FL-Nakama-Sonden-Design-Entwurf.md            |  507 ++-
 docs/PLAN-STAND.md                                 |   41 +-
 ...-Nakama-Sonden-Design-Entwurf-0.4-2026-08-20.md | 4238 ++++++++++++++++++++
 docs/bauaufteilung-sonden.md                       |   81 +-
 docs/beweise/PR2.md                                |  742 ++++
 docs/offene-punkte.md                              |  230 +-
 docs/plan/fragen.json                              |   78 +-
 docs/plan/plan.json                                |   20 +-
 tools/plan/dokuriegel.py                           |  346 ++
 tools/plan/planstand.py                            |   16 +-
 13 files changed, 6071 insertions(+), 276 deletions(-)
EXITCODE=0

### E) Zaehlungen
Errata mit Einarbeitung im Text: 17/17  abcdefghijklmnopq
  Marken '[0.5 - Erratum (x) ...]': 25
  Grabsteinzeilen 'gestrichen in Fassung 0.5': 23
  NUMMERIERTE Ueberschriften 0.4 / 0.5: 190 / 190, gleiche Reihenfolge: True, nur in 0.5: [], nur in 0.4: []
  ALLE Ueberschriften Ebene 2-3   0.4 / 0.5: 255 / 257, neu (unnummeriert): ['Rangfolge der lebenden Spezifikationen (', 'Wie Fassung 0.5 entstanden ist']
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
55cdb91..HEAD: Pruefwerkzeug 2 Datei(en) +359/-3 | Doku 11 Datei(en) +5712/-273
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
  Runner.** Von den hinzugefügten Zeilen sind 4 238 die byteidentische Archivkopie. `PLAN-STAND.md`
  und `ANTWORTEN-OFFEN.md` sind erzeugt, nicht von Hand editiert.
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
| Ein umnummerierter Paragraph? | Nein — 190 nummerierte Überschriften vorher wie nachher, in identischer Reihenfolge, keine hinzu und keine weg (§8, Lauf E). |
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
4. ~~**NAK-97 ist nicht geschlossen**, obwohl dieses Ticket `planstand.py` angefasst hat. Die zwei
   Reste liegen außerhalb des Auftrags (E7 verlangt ausdrücklich eine *minimale* Korrektur der
   „Als Nächstes"-Logik); sie stehen als Vorschlag V-3 in §6.~~ **Berichtigt am 02.09.2026
   (Runde 1, §12.2 Befund D8):** der Punkt war falsch. Die zwei Reste sind seit `da1b04e` behoben
   — nachgemessen an `tools/plan/planstand.py:151` und `:102`. NAK-97 trägt seine Schließung im
   eigenen Text und steht seit Runde 1 in der Geschlossen-Tabelle.
5. **Die Klasse „blockiert P4–P7" heißt im Register „Planarbeit".** Der Gate-Text nennt sie so,
   aber zwei Zeilen (NAK-119, NAK-127) und zwei weitere (NAK-30, NAK-79) gehören zu S32–33 und
   S34–35, also zu P8 und P9. Die Klasse trägt deshalb den konkreten Schritt im Namen statt einer
   Phasenspanne; die Zuordnung selbst ist vollständig.
6. **Die visuelle Spezifikation für Gen Fläche 2 gibt es nicht.** S31b hängt jetzt daran; ihr
   Beginn ist ein eigener User-Schritt (`CLAUDE.md`) und wurde hier nicht vorweggenommen.
7. **Blueprint §15 ist nicht gekürzt.** Die 17 Punkte tragen jetzt je eine Schritt-ID im Plan,
   aber der Blueprint selbst führt sie unverändert als offen — sie sind zugeordnet, nicht
   geschlossen.

---

## 11. Nachlauf (lebender Kopf nachgezogen, Prüfliste E)

Auf `1184b25` folgt genau ein weiterer Commit an diesem Ticket, und er bleibt in derselben
Grenze (`docs/beweise/PR2.md`, `docs/PLAN-STAND.md`):

1. **Commit 6 trägt jetzt seinen SHA** in der Tabelle in §3 — vorher stand dort ein Verweis auf
   einen Abschnitt, den es so nicht gab.
2. **Zwei Zahlen präzisiert.** Der erste Lauf zählte „255 nummerierte Überschriften"; gezählt
   hatte er in Wahrheit **alle** Überschriften der Ebenen 2 und 3. Nachgemessen mit einem Muster,
   das nur nummerierte Überschriften trifft: **190 → 190, identische Reihenfolge, keine hinzu,
   keine weg**; die 255 → 257 gelten für alle Überschriften und sind jetzt als eigene Zeile
   ausgewiesen. Der Zähler in Lauf E ist entsprechend berichtigt und trägt beide Zahlen getrennt.
   Die Aussage „keine umnummerierten Paragraphen" hat sich dadurch nicht geändert — sie ist jetzt
   nur mit der richtigen Zahl belegt.
3. **Lauf D neu gefahren**, weil er im ersten Durchgang vor dem Manifest-Commit lag und deshalb
   `PR2.md` und die zwei Blätter nicht zeigte. Die Grenze ist unverändert: kein Commit dieses
   Tickets berührt Produktcode, Test oder Schema.

---

## 12. Runde 1 — Nacharbeit nach Codex-Erstprüfung (02.09.2026)

Diese Runde behebt genau die vierzehn Defekte, die der Erstprüfer bestätigt hat, und nichts
sonst. Was darüber hinaus auffiel, steht als datierte Registerzeile im Register, nicht in
diesem Änderungssatz — die eine solche Zeile ist **NAK-130** (§12.2, unten).

### 12.1 Die Erstprüfung

| Feld | Wert |
|---|---|
| Prüfer | Codex `gpt-5.6-sol`, Effort **max**, **lesend** (read-only), frischer Thread |
| Thread | `01a060fd-b78e-7d90-aee6-591dab8dd943` |
| Geprüfter Stand | `16be249fd30569bf7eec3c5c8550f7c94212a937` (Ticketbasis war `55cdb91`) |
| Prüfbereich | die Ticketpfade aus dem Gate-Text: `docs/plan/plan.json`, `docs/plan/fragen.json`, `docs/bauaufteilung-sonden.md`, `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/archiv/`, `design/docs/funktions-und-bedien-blueprint.md`, `CLAUDE.md`, `docs/offene-punkte.md`, `docs/beweise/PR2.md` |
| **Urteil** | **NEEDS_WORK** |
| Bestätigte Defekte | **14** — D1 bis D14 |

**Woher die einzelnen Befunde stammen.** D1 hat der Dirigent selbst an der Quelle gemessen
(`git show 55cdb91:docs/offene-punkte.md` gegen `HEAD`), nicht der Prüfer. D2 stammt aus
Prüferabschnitt P1, D3 bis D14 aus P2. Jeden Befund hat der Dirigent vor der Übergabe an der
Quelle nachgeprüft und bestätigt; die Formulierungen in §12.2 sind seine Übergabefassung, nicht
eine Nacherzählung des Prüfertexts. Wo eine Formulierung des Prüfers wörtlich zitiert wird,
steht sie in Anführungszeichen.

**Zusammenfassung des Urteils, in einem Satz:** der Erstdurchgang hat die fünf Teile des
Gate-Textes ausgeführt, dabei aber Registerzeilen gelöscht statt verschoben (D1), geschlossene
Zeilen als offen geführt (D2, D8), einen bereits entschiedenen Produktpunkt erneut geöffnet
(D3), ein Erratum nur zur Hälfte eingearbeitet (D4), User-Zitate transliteriert (D5), zwei
Zusagen ohne Ort im Plan gelassen (D6, D7), fünf Stellen ohne die nötige Berichtigung
stehengelassen (D9, D10, D11, D12, D13) und die Prozessbilanz nur über elf Tickets und ohne
Runden geführt (D14).

### 12.2 Die vierzehn Defekte — Befund, Regel, Ausführung

**D1 — 13 Registerzeilen gelöscht.**
*Befund:* Auf `55cdb91` standen in der Offen-Tabelle 13 bereits geschlossene, durchgestrichene
Zeilen (NAK-32, -33, -50, -51, -52, -54, -55, -58, -61, -63, -64, -66, -87). Auf `16be249`
kamen diese IDs in der Datei nicht mehr vor. Das bricht den Gate-Satz „Die Regeln bleiben: nie
löschen, schließen nur mit ID und SHA" und die Kopfregel der Datei.
*Regel:* Jede der 13 Zeilen wird **wortgleich** aus der Basisfassung zurückgeholt und in die
Geschlossen-Tabelle gestellt; NAK-51 trägt ein User-Zitat, das nicht verloren gehen darf.
*Ausgeführt:* Alle 13 Zeilen sind byteweise aus `git show 55cdb91:docs/offene-punkte.md`
zurückgeholt — mit ihrer ursprünglichen Datumsspalte, ihrem `~~ID~~`-Durchstrich und ihrem
vollständigen Text. NAK-51 trägt sein User-Zitat unverändert („*upodate das openwiki und
verwerfe das mit dem API key wir machen das manuell*"). Über der Geschlossen-Tabelle steht
ein Vorspann, der die zwei Zugangsgruppen erklärt, damit die „seit"-Spalte nicht als
Schließdatum gelesen wird. Beweis: §12.4, Lauf E1 — der Mengenvergleich aller
`| NAK-n |`-Zeilen-IDs zwischen `55cdb91` und dem Arbeitsstand ergibt **null Abgänge**.

**D2 — NAK-74 ist geschlossen, wurde aber als offener G3-Blocker geführt.**
*Befund:* Die Zeile sagt selbst „GESCHLOSSEN 24.08. mit `ebfe02e` (Vertragsebene) + `a2fe0f5`
(State-Ebene)", trug aber die Klasse `[Planarbeit · G3]`, stand in der Blockerliste P3·G3, und
der berichtigte G3-Text nannte „NAK-74 und NAK-112" als zwei offene Blocker gegen Gate 7.
*Regel:* NAK-74 wandert mit seinen SHAs in die Geschlossen-Tabelle; Blockerliste und G3-Text
nennen als Gate-7-Blocker nur NAK-112, mit datiertem Nachtrag, der alte Satz bleibt sichtbar.
*Ausgeführt:* NAK-74 steht unverändert in der Geschlossen-Tabelle. Die Blockerzeile P3·G3
nennt nur NAK-112 und trägt die Berichtigung samt beider Schließungs-SHAs. Der G3-Text in
`docs/plan/plan.json` zitiert seinen alten Satz als Verlauf und stellt richtig: gegen Gate 7
fällt nur NAK-112; die Folgefragen von NAK-74 leben in NAK-79 (entschieden) und NAK-112
(offen). Die Schlusszeile „Register: NAK-74, NAK-112, NAK-57." heißt jetzt „Register:
NAK-112, NAK-57 (NAK-74 geschlossen, siehe Berichtigung oben)."

**D3 — der S32–33-Nachtrag öffnete einen entschiedenen Punkt erneut.**
*Befund:* Der Nachtrag „TRÄGER DER POSITION `post_fader_contribution`" behauptete, der Entwurf
lasse offen, WELCHE `plugin_kind` die Position tragen dürfe, und der Klassen-Riegel brauche
eine Antwort. NAK-79 hält aber fest: User-Wort 24.08.2026, PR1-Berichtigung „die Produktfrage
ist entschieden und wird nicht wieder gestellt", Dirigenten-Entscheid 30.08.2026.
*Regel:* Der Nachtrag trägt den Entscheid aus NAK-79 mit Datum und Wortlaut und nennt nur die
dort wirklich offene Technik; keine Frage nach der tragenden Sondenklasse; der falsche Satz
bleibt als Verlauf sichtbar.
*Ausgeführt:* Der Nachtrag heißt jetzt „BEITRAG JE GEN-EINGANG — BERICHTIGT 02.09.2026
(Runde 1)". Er zitiert den alten Satz als Verlauf, führt beide User-Sätze vom 24.08.2026
zeichengetreu, hält fest, dass keine Sondenklasse die Position trägt und der Empfänger Gen
ist, benennt die fail-closed-Vorgabe als **Dauerantwort** und nennt als einzig Offenes die
Technikform: ein Deskriptor je Gen-Eingang, Name in der v3-Vertragsrunde von S16–17 reserviert,
Nutzlast mit dem Aux-Bus in `SONDE-018`. Ausdrücklich steht dort auch, dass **nicht** eine
Zeile „Träger der Position" in §32.2 gehört, sondern der Satz, dass die Position kein
Instanzattribut ist.

**D4 — Erratum (c) war nur teilweise eingearbeitet.**
*Befund:* In Entwurf 0.5 §0.3 war nur der letzte Punkt gestrichen; die Punkte davor
definierten die Prisma-App weiter als eigenständige Begleit-App mit eigener Oberfläche.
*Regel:* An jeder von (c) genannten Stelle und im §0.3-Rest wird die Begleit-App-Aussage als
„Studie, kein Bauziel (Erratum (c), 21.08.2026)" markiert — Grabstein- oder Klammersatz mit
Buchstabe, Datum und User-Wort, kein umnummerierter Paragraph; die Trennungsaussage bleibt.
*Ausgeführt:* Drei neue Marken in §0.3. (1) Ein Klammersatz direkt unter der Überschrift sagt,
dass Überschrift und Punkte im 0.4-Wortlaut stehen bleiben, ihre Prisma-Hälfte aber seit dem
21.08.2026 Studie ist. (2) In Punkt 1 ist der Maßstabsatz zu `geschmacksprofil.md`
durchgestrichen — die Datei bindet nach (c) nur die Studie —, der Kernprodukt-Halbsatz bleibt.
(3) Punkt 2 (Begleit-App) ist durchgestrichen und trägt seinen Grabstein. (4) In Punkt 3 ist
die Aussage „gilt der Prisma-App … zwei verschiedene Oberflächen" durchgestrichen; der Konflikt
aus Befund A löst sich nicht über eine zweite Oberfläche, sondern weil der Zielvertrag im
Produkt nichts bindet. Die Vollständigkeitsprobe steht in §12.4, Lauf E2: **jede** Fundstelle
von `Prisma-App|Prisma daneben|Begleit-App` ist dort mit ihrer Behandlung genannt. Nummerierte
Überschriften **190 vorher wie nachher**, identische Reihenfolge (Lauf E3).

**D5 — User-Zitate transliteriert.**
*Befund:* `plan.json` zitierte als User-Wort „es waere sinnvoller …" und „es wird eine groesse
naemlich standard geben"; die Originale lauten „es wäre sinnvoller …"
(`design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md:147`) und „es wird eine
größe nämlich standard geben" (ebd.:257; `design/abnahmen/2026-09-01-gen-nur-standardgroesse.md:9`).
*Regel:* Jedes in dieser Runde und in PR2 neu eingefügte User-Zitat in `plan.json`,
`bauaufteilung-sonden.md`, `offene-punkte.md` und `fragen.json` wird zeichengetreu aus der
Abnahme übernommen (UTF-8, keine ae/oe/ue-Umschrift); jede neue Zitatstelle wird gegen ihre
Quelle geprüft.
*Ausgeführt:* Der Diff `55cdb91..16be249` über die vier Dateien enthält **sieben** neu
eingefügte User-Zitate. Jedes ist gegen seine Quelle gehalten worden:

| # | Zitat (Anfang) | Ort der Einfügung | Quelle, an der geprüft wurde | Befund und Behandlung |
|---|---|---|---|---|
| 1 | „figma ist keine technikreferenz, der implementplan ist es" | `plan.json` G3 | `fragen.json`, `beantwortet.U9.1.text` | zeichengleich — unverändert |
| 2 | „das ist eine technische frage. wenn es kein problem ist dann ja" | `plan.json` G3 | `fragen.json`, `beantwortet.U9.2.text` | zeichengleich — unverändert |
| 3 | „Figma gilt" | `plan.json` G3 | `fragen.json`, `beantwortet.U9.5.wahl` | zeichengleich — unverändert |
| 4 | „ignoriere bisherige design die entworfen wurden" | `plan.json` S31b, `bauaufteilung` §1.4 und §3, `offene-punkte` NAK-65, `fragen.json` U9.3 | `design/docs/funktions-und-bedien-blueprint.md` §0, Zeile „Bindender User-Wortlaut für die Quellenabgrenzung" | **zwei Abweichungen:** der Schlusspunkt fehlte, und als Quelle stand `design/LIES-MICH.md`, das den Wortlaut gar nicht trägt (`grep` 0 Treffer). Beides an allen fünf Stellen berichtigt |
| 5 | „es waere sinnvoller … waagrecht untereinander waeren …" | `plan.json` S18–19 | `…arbeitsmodus.md:147` | **transliteriert** — jetzt „es wäre sinnvoller … wären …" |
| 6 | „keine funktion um auf die 2. seite zu wechseln abgesehen von send draft" | `plan.json` S18–19 | `…arbeitsmodus.md:167` | Wortlaut gleich, gerade Anführungszeichen — jetzt mit den deutschen Zeichen des Originals |
| 7 | „compact und standard … es wird eine groesse naemlich standard geben …" | `plan.json` S18–19 | `…arbeitsmodus.md:257`, `…gen-nur-standardgroesse.md:9` | **transliteriert** — jetzt „… es wird eine größe nämlich standard geben …" |

Der Schritt `S18–19` trägt dazu einen eigenen datierten Satz, der die Nachziehung nennt. Der
Restbestand ist gemessen: `waere sinnvoller` und `groesse naemlich` kommen in `plan.json`
**null** Mal vor (§12.4, Lauf E4).

**D6 — Blueprint §15 Punkt 8 war nur halb zugeordnet.**
*Befund:* Punkt 8 lautet „[O] U11/U13: echte FL-Messungen für Aux/PDC und Projektzeit
bleiben …"; der S32–33-Nachtrag ordnete U11/NAK-44 und NAK-79 zu, U13 (FL-Kurztermin C,
Register NAK-56) kam in `plan.json` nicht vor.
*Regel:* U13 bekommt einen datierten Nachtrag in dem Schritt, dessen Zusage von der
Zeit-Annahme abhängt — der Schritt wird aus der Aussage von Karte und Registerzeile gewählt,
nicht aus der Nähe zu U11.
*Ausgeführt:* Der Nachtrag steht in **S18–19**, nicht in S32–33. Begründung aus den zwei
Quellen: NAK-56 sagt ausdrücklich, dass der Messkern **beide** Fälle ohne zu raten deckt
(Bein B5 §G10) — S10–11 und S12–13 hängen also nicht an der Annahme. Ablesbar ist der reale
Fall allein am Zähler `analyseKontinuitaetsbrueche()`, und Karte U13 trägt genau deshalb
`blockiert_durch: NAK-57` — die fehlende Anzeige. Die Zusage, die daran hängt, ist die von
S18–19: Gen Fläche 1 zeigt je Quelle den Fehlerzustand ehrlich. Der Nachtrag nennt Blueprint
§15 Punkt 8, U13, NAK-56 und das Datum und verlangt, dass `grenzenMitGrund()` seine neun
Ursachen getrennt zeigt; solange die Anzeige fehlt, darf keine Fläche behaupten, die Annahme
sei gemessen.

**D7 — NAK-121 und NAK-124 hatten keinen Ort im Plan.**
*Befund:* Der Gate-Satz „NAK-121 Modul-Aufteilung und NAK-124 bekommen einen Ort im Plan" war
mit einer Registerkopfzeile „vor P4" nicht erfüllt; `plan.json` und `bauaufteilung-sonden.md`
nannten beide nur im PR2-Gate-Text.
*Regel:* In `plan.json` entsteht in Phase P3 direkt nach G3 der Schritt `S19b` (Ticket
`NAK-121`, Stufe `T2`, Beleg `docs/beweise/NAK-121.md`) mit dem vom Dirigenten vorgegebenen
Inhalt; dieselbe Zeile in `bauaufteilung-sonden.md` §3; die Registerzeilen bekommen die Klasse
`[Planarbeit · S19b]`, die Blockerliste nennt S19b.
*Ausgeführt:* Der Beleg `docs/beweise/NAK-121.md` ist der **noch nicht angelegte Zielpfad** des Tickets; er
entsteht erst, wenn S19b gebaut wird, und der Planstand führt den Schritt bis dahin richtig
als offen. `S19b` steht in `plan.json` an der vorgegebenen Stelle — Phasenfolge P3 ist jetzt
`S18–19 · PR2 · G3 · S19b`. Der Text nennt die Modulaufteilung von `server_v3.rs`,
`coordinator.rs` und `store.rs` je Fachbereich mit erhaltener Historie, die 25 G2-Härtungen aus
NAK-124 (14 MEDIUM + 11 LOW, `docs/beweise/G2.md` §4.2) im selben Änderungssatz, die Grenze
(kein Wire-, State- oder Testvertrag), den Beweis (Kanon vor und nach der Aufteilung grün auf
demselben Stand) und den frischen Codex-Thread als Prüfer. `bauaufteilung-sonden.md` §3 trägt
dieselbe Zeile; §4 rechnet seitdem **41 statt 40** Bau-Sessions, bis R2 **29 statt 28**, bis R4
**51 statt 50** — mit datiertem Vermerk. Register: beide Zeilen tragen `[Planarbeit · S19b]`,
die Blockerzeile heißt jetzt „P3 · S19b (neu am 02.09.2026, direkt nach G3)".

**D8 — geschlossene Zeilen standen in der Offen-Tabelle.**
*Befund:* Mindestens NAK-24, -35, -42, -45, -75, -76, -77, -88, -89, -94, -95, -97, -98, -100,
-104 tragen im eigenen Text „GESCHLOSSEN …" mit Commit-SHA, standen aber mit einer Offen-Klasse
in der Offen-Tabelle; NAK-35 sagt wörtlich „diese Zeile bleibt geschlossen".
*Regel:* Jede Zeile der Offen-Tabelle, deren eigener Text eine Schließung mit SHA trägt, wird
an der Quelle geprüft — ist alles geschlossen, wandert sie unverändert in die
Geschlossen-Tabelle; ist ein Teil offen, bleibt sie mit ihrer Klasse und bekommt einen
datierten PR2-Nachtrag, der den offenen Rest in einem Satz benennt.
*Ausgeführt:* Geprüft wurden **nicht nur die 15 genannten**, sondern alle 79 offenen Zeilen
maschinell auf Schließungswörter (`GESCHLOSSEN`, `Geschlossen`, `VERWORFEN`, `GEGENSTANDSLOS`,
`Abschluss <datum>`, `erledigt`). Ergebnis je ID:

| ID | eigener Schließungstext | Ergebnis |
|---|---|---|
| NAK-24 | „GESCHLOSSEN 23.08. mit `SONDE-008`" (`7fa1cf5` + Folgecommit), beide Hälften benannt | **umgezogen** |
| NAK-35 | „GESCHLOSSEN 23.08. mit `SONDE-008`" … „diese Zeile bleibt geschlossen" | **umgezogen** |
| NAK-42 | „GESCHLOSSEN 24.08. mit `5ffcc60`", §55 Klausel 4 erfüllt | **umgezogen** |
| NAK-45 | „GESCHLOSSEN 25.08. mit `91e2cab`" auf User-Wort | **umgezogen** |
| NAK-74 | „GESCHLOSSEN 24.08. mit `ebfe02e` + `a2fe0f5`" (Befund D2) | **umgezogen** |
| NAK-75 | „GESCHLOSSEN 24.08. mit `fa976fd`" | **umgezogen** |
| NAK-76 | „GESCHLOSSEN 24.08. mit `fac67f4`" | **umgezogen** |
| NAK-77 | „GESCHLOSSEN 24.08. mit `ebfe02e`" | **umgezogen** |
| NAK-88 | „Geschlossen 30.08.2026: Runde 4 `1bfc8b0`/`3687ea4`", Prüfer PASS | **umgezogen** |
| NAK-89 | „Geschlossen 30.08.2026: Runde 4 `aadca53`/`fff9e09`, Nachtrag `977bf48`" | **umgezogen** |
| NAK-94 | „GESCHLOSSEN 30.08.2026 11:17", Prüfer 14 PASS, Kanon 32/32 auf `0e7a60e` | **umgezogen** |
| NAK-95 | „Geschlossen 30.08.2026: Rest über NAK-104 (`b4fe522`)", Wiederprüfung PASS | **umgezogen** |
| NAK-97 | „Abschluss 29.08.2026 … PASS Runde 4 … Geschlossen." | **umgezogen**, zusätzlich an der Quelle nachgemessen (siehe unten) |
| NAK-98 | „Geschlossen 30.08.2026: NAK-104 (1) … `b4fe522`", Wiederprüfung PASS | **umgezogen** |
| NAK-100 | „GESCHLOSSEN 08-30 — Rückbau gebaut (`e2551a9`, `4c87aab`)", Prüfer PASS | **umgezogen** |
| NAK-104 | „Geschlossen 30.08.2026: Runde 1 `b4fe522`, Runde 2 `e517165`", Wiederprüfung PASS | **umgezogen** |
| NAK-85 | „Nachtrag 29.08.2026 (Fortsetzung), Commit `d3c741c`: geschlossen — **Urteil offen**" | **bleibt offen**, Nachtrag benennt den Rest |
| NAK-92 | „alle sechs Befunde … geschlossen — `1bdb93d`, `c444ca3` … **Urteil offen — der Punkt bleibt offen**" | **bleibt offen**, Nachtrag benennt den Rest |
| NAK-56 | „Werkbankhälfte geschlossen 23.08. mit `SONDE-009`", nennt selbst zwei offene Teile (a) und (b) | **bleibt offen**, unverändert — der offene Rest steht schon im Text |
| NAK-38 | „U9 vom User geschlossen" (Nachtrag 29.08.) | **bleibt offen** — Befund D9, siehe dort |
| NAK-122, -124, -125, -126, -127 | „Zurück in die Offen-Tabelle am 02.09.2026 (PR2)" | **bleiben offen** — sie sind im Erstdurchgang genau deshalb zurückgeholt worden |

**NAK-97 an der Quelle nachgemessen.** Der Erstdurchgang hatte in §6 (V-3) und §10 (Punkt 4)
behauptet, zwei Reste lägen noch in `tools/plan/planstand.py`. Das ist falsch: `--no-optional-locks`
steht dort in `planstand.py:151` als erstes Argument vor `-C`, und die `KANON`-Regex trifft den
ROT-Fall über den Zweig `rot_fehl`/`rot_ges` (`planstand.py:102`). Beide Reste sind seit
`da1b04e` behoben, wie die Zeile selbst sagt. V-3 in §6 und Punkt 4 in §10 tragen deshalb ihre
datierte Berichtigung.

**D9 — NAK-38 war nur zum Teil Produktfrage.**
*Befund:* Die Zeile trug `[Produktfrage · Karte U22]`, U22 fragt aber nur nach fünf oder sieben
Rollen; die weiteren Punkte (Suna-Kachel, Messzustände 5 gegen 2, Arbeitsschritte 3 gegen 5,
Figma-Gitter gegen Heatmap) sind durch den Funktionsneustart vom 31.08.2026 überholt.
*Regel:* Datierter PR2-Nachtrag, der die Rollenfrage als einzig offenen Teil benennt und die
übrigen Teilfragen mit Datum und Quelle als überholt markiert; nichts wird gelöscht.
*Ausgeführt:* Der Nachtrag steht in NAK-38. Er nennt U22 als einzigen offenen Teil, markiert
die drei Suna-Punkte als überholt mit `design/abnahmen/2026-08-23-gen-eq-zentrale.md` und den
Figma-Punkt mit `design/docs/funktions-und-bedien-blueprint.md` §0 samt bindendem User-Wortlaut
vom 31.08.2026, verweist auf dieselbe Begründung bei U9.3 und U9.4 und hält fest, dass die vier
Teilfragen im Text stehen bleiben. Die Klasse bleibt `[Produktfrage · Karte U22]`.

**D10 — NAK-111 widersprach seiner Karte.**
*Befund:* Die Zeile trug `[Produktfrage · Karte U25]` und sagte im selben Text „… es gibt daher
auch keine Fragenkarte".
*Regel:* Datierter PR2-Nachtrag: der Satz vom 30.08.2026 ist durch Karte U25 (02.09.2026)
überholt; der alte Satz bleibt sichtbar.
*Ausgeführt:* Der Nachtrag steht in NAK-111 und sagt genau das; der alte Satz steht unverändert
darüber.

**D11 — Schließungen ohne Quelle.**
*Befund:* Die Nachhaltung von U9.4 nannte weder für den Suna-Entfall (23.08.2026) noch für den
Funktionsneustart (31.08.2026) eine Quelle; U6.8 nannte die Quelle des Suna-Entfalls nicht.
U9.3 zeigt die erwartete Form.
*Regel:* Beide Ergebnistexte tragen je Entscheid die Quelldatei; alle sechs nachgehaltenen
Karten werden nach demselben Maß geprüft.
*Ausgeführt:* Alle sechs geprüft, jede mit Ergebnis:

| Karte | Befund der Prüfung | Behandlung |
|---|---|---|
| U9.3 | trug beide Quellen — aber die Quelle des User-Wortlauts war falsch (`design/LIES-MICH.md` trägt ihn nicht) | Quelle auf `design/docs/funktions-und-bedien-blueprint.md` §0 berichtigt, Zitat zeichengetreu (D5) |
| U9.4 | **keine** Quelle für Suna-Entfall und Funktionsneustart | beide Quellen ergänzt, plus Vermerk, dass sie gefehlt haben |
| U6.8 | Quelle nur für die zweite Hälfte (visuelle Phase) | Quelle des Suna-Entfalls ergänzt, plus Vermerk |
| U2.4 | Quelle für den abgenommenen Zeileninhalt vorhanden, für den Funktionsneustart nicht | Quelle ergänzt, plus Vermerk |
| U2.9 | dasselbe Muster wie U2.4 | Quelle ergänzt, plus Vermerk |
| U2.6 | trägt für ihre einzige Entscheidaussage bereits die Quelle | unverändert, Prüfvermerk ergänzt |

`wahl`, `text` und `datum` **aller** beantworteten Karten sind dabei byteidentisch geblieben;
der Riegel im Edit-Skript vergleicht die drei Felder vor und nach dem Schreiben und bricht bei
Abweichung ab.

**D12 — bauaufteilung §1.4 schickte weiter zur Altquelle.**
*Befund:* Direkt vor dem neuen Quellenumschnitt stand: „Offen in NAK-65 bleibt der Umbau …
Ausgangsbasis ist der aktuelle Suna-Entwurf `Nakama-Design` Node `6:2864`". NAK-65 ist in
diesem Ticket geschlossen (`894dce7`), Suna als App entfallen, Figma-Stände sind Verlauf.
*Regel:* Der Absatz wird durchgestrichen und mit Datum und Quelle als historisch markiert
(Vorbild: die Spielregeln-Zeile im Kopf derselben Datei).
*Ausgeführt:* Der Absatz ist durchgestrichen und trägt den Grabstein „Historisch seit dem
02.09.2026 (PR2 Runde 1, Codex-Befund D12)" mit allen drei Gründen und ihren Quellen —
NAK-65 geschlossen mit `894dce7`, Suna entfallen mit
`../design/abnahmen/2026-08-23-gen-eq-zentrale.md`, Figma Verlauf seit dem Funktionsneustart —
und verweist für den Ort der Probeeq-Rückfallfläche auf **S31b** und den Absatz darunter.

**D13 — S23–25-Nachtrag in bauaufteilung ohne datierte Quelle.**
*Befund:* Die S23–25-Zeile nannte für den „abgenommenen Ort" der Überdeckungszone nur „Seit PR2
(02.09.2026)"; der Parallel-Nachtrag in `plan.json` nennt
`design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md` („Stabile Geometrie",
„Graphreaktion").
*Regel:* Dieselbe Quelle mit Datum an dieser Stelle; alle PR2-Nachträge in §3 nach demselben
Maß prüfen.
*Ausgeführt:* Alle acht PR2-Nachträge in §3 geprüft:

| Ort in §3 | Quelle vorher | Behandlung |
|---|---|---|
| S18–19, „Dazu seit PR2" | zwei Abnahmedateien mit Datum | unverändert — Maß erfüllt |
| S23–25, „Seit PR2" | **keine** | Quelle `…arbeitsmodus.md`, Abschnitte „Stabile Geometrie" und „Graphreaktion" nachgetragen |
| P6–P7, Vertragsarbeit „Band-Belegung" | „Fünferblock 02, 4/5 und 5/5 (01.09.2026)" — Datum, aber kein Pfad | Pfad `…-fuenferblock-02.md` nachgetragen |
| P6–P7, „`Frame.band_dynamic_gain_db`" | „Fünferblock 01, 5/5" | Pfad `…-fuenferblock-01.md` nachgetragen |
| P6–P7, „10-Sekunden-Kandidat" | „Entscheid 23.08.2026" ohne Pfad | Pfad `…2026-08-23-interview-struktur.md` nachgetragen |
| P6–P7, „Zweistufiger Mix" | „Entscheid 23.08.2026" ohne Pfad | Pfad `…2026-08-23-gen-eq-zentrale.md` nachgetragen |
| S26–28 und S28b, „Dazu seit PR2" | **keine** | beide Fünferblock-Pfade nachgetragen |
| S29–31, „Dazu seit PR2" | **keine** | Interview-Struktur, Fünferblock 01 und NAK-120 nachgetragen |
| S31b, „Abgenommene Bedienarchitektur (31.08./01.09.2026)" | Daten, keine Pfade | vier Abnahmepfade nachgetragen |
| S31b, „Quelle umgestellt" · §1.4, „Quellen umgestellt" | Pfade vorhanden | unverändert — Maß erfüllt |

**D14 — Prozessbilanz unvollständig.**
*Befund:* Der Gate-Satz verlangt `tools/dirigent/rundenbilanz.py` über alle Tickets seit S0 mit
Runden je Ticket und Diff je Runde. §4.2 zeigte elf Tickets, davon vier ohne Diff, nur Spannen
statt Runden, und §8 Lauf H fuhr das Werkzeug nur über `55cdb91..HEAD`.
*Regel:* Für jedes Ticket mit Manifest in `docs/beweise/` wird `rundenbilanz.py` über
Basis..Ende aus dem Manifestkopf gefahren und, wo das Manifest Runden-SHAs nennt, mit
`--runden` je Runde; die Tabelle wird vollständig neu geschrieben; Tickets ohne ermittelbare
SHAs stehen mit dem Grund; die Rohausgabe liegt unter `docs/beweise/roh/`.
*Ausgeführt:* **24 Ticketmanifeste** gemessen — alle SONDE-, NAK-, G- und PR-Manifeste seit
S0. Nicht gemessen sind `VORLAGE.md` (Vorlage, kein Ticket),
`KONTEXT-INVENTUR-2026-08-21.md` (kein Ticket) und die drei `lauf-…`-Dateien (Rohläufe). Die
`-verlauf`-Teile von `SONDE-007a`, `SONDE-007c` und `SONDE-010` sind für die Runden-SHAs
mitgelesen worden. Rohausgabe:
[`docs/beweise/roh/PR2-rundenbilanz-008989c.md`](roh/PR2-rundenbilanz-008989c.md).

Wie Basis und Ende bestimmt wurden — dieselbe Regel für alle 24 Manifeste, in dieser
Reihenfolge: (1) `Basis-SHA` und `End-SHA` aus dem Manifestkopf, wo beide stehen; (2) sonst
`Basis-SHA` beziehungsweise `Basis` aus dem Kopf als Basis; (3) sonst der Elternteil des
ältesten Commits, dessen Betreff den Ticketnamen trägt, vereinigt mit den im Kopf genannten
Commit-SHAs; (4) Ende: `End-SHA` aus dem Kopf, sonst der jüngste solche Commit. Runden-SHAs:
jede Zeile, die eine Runde benennt (`Runde <n>`, `Nacharbeit <n>`, `Phase A/B`), wird nach
Backtick-SHAs durchsucht, jeder Treffer gegen `git cat-file` geprüft, auf die Spanne
beschnitten und in Commit-Reihenfolge sortiert.

**Lesart, bevor die Zahlen kommen.** Eine Spanne ist ein *Zeitraum*, kein Ticketfilter. Liefen
zwischen dem ersten und dem letzten Commit eines Tickets fremde Commits, stehen deren Zeilen in
der Spannensumme mit drin — bei 19 der 23 Spannen ist das so, teils erheblich (`SONDE-005a`:
430 Commits in der Spanne gegen 15 mit Ticketnamen). Genau daran ist die alte §4.2 gescheitert.
Die erste Tabelle zeigt deshalb beide Zahlen nebeneinander; die **zweite** Tabelle ist die
ticketgenaue: nur die Commits, deren Betreff den Ticketnamen trägt, einzeln gemessen
(`<sha>^..<sha>`) und summiert.

**Tabelle 1 — Spanne je Ticket (Basis..Ende, wie der Gate-Satz es verlangt).**

| Ticket | Spanne `Basis..Ende` | Commits in der Spanne | davon mit Ticketnamen | Produkt | Tests | Prüfwerkzeug | Doku |
|---|---|---:|---:|---|---|---|---|
| `S0` | `734cf50..6c61ae4` | 2 | 1 | — | — | 1 D +609/−0 | 5 D +476/−39 |
| `SONDE-001/002` | `df84e20..0ba87cc` | 1 | 1 | 10 D +135/−42 | 2 D +489/−9 | 1 D +75/−39 | 6 D +845/−90 |
| `SONDE-003` | `0ba87cc..e9c6fa0` | 6 | 6 | 5 D +821/−1 | 1 D +577/−0 | 1 D +14/−2 | 5 D +2954/−59 |
| `SONDE-004` | `3d820c6..9d99f40` | 77 | 3 | 307 D +28186/−65 | 8 D +4210/−5 | 27 D +6889/−158 | 60 D +24579/−932 |
| `SONDE-004a` | `3d820c6..df84e20` | 1 | 1 | 7 D +776/−0 | 1 D +290/−0 | 2 D +90/−2 | 4 D +469/−42 |
| `SONDE-005a` | `5299037..6c314e1` | 430 | 15 | 360 D +40548/−645 | 12 D +8836/−58 | 93 D +13565/−302 | 324 D +126851/−1610 |
| `SONDE-005b` | `3c64b98..e99a211` | 429 | 10 | 310 D +30812/−8311 | 12 D +8128/−59 | 93 D +11831/−375 | 325 D +135801/−1689 |
| `SONDE-006` | `26b7680..3353fb6` | 400 | 8 | 152 D +20815/−851 | 12 D +7702/−61 | 89 D +9288/−258 | 303 D +132002/−1082 |
| `SONDE-007a` | `dafa5a5..478e564` | 459 | 80 | 167 D +26583/−931 | 14 D +9911/−49 | 94 D +15440/−1014 | 252 D +258261/−1193 |
| `SONDE-007b` | `4b500a4..36560b0` | 461 | 22 | 162 D +25912/−1084 | 14 D +9924/−49 | 95 D +10262/−1177 | 257 D +263868/−3536 |
| `SONDE-007c` | `9bb75ad..bae7a4e` | 216 | 13 | 70 D +8654/−93 | 6 D +3141/−6 | 13 D +3255/−575 | 54 D +163084/−10520 |
| `SONDE-008` | `e330052..5d26002` | 96 | 15 | 14 D +4191/−167 | 5 D +3915/−3 | 18 D +777/−724 | 55 D +41838/−374 |
| `SONDE-009` | `ff24908..9aa7887` | 67 | 11 | 11 D +2732/−64 | 1 D +2137/−0 | 18 D +749/−721 | 55 D +29739/−331 |
| `SONDE-010` | `a7b0740..d1ef796` | 211 | 45 | 66 D +9854/−49 | 5 D +3540/−25 | 12 D +2629/−536 | 61 D +164403/−25551 |
| `SONDE-011` | `b75ea06..9abb66f` | 23 | 19 | 105 D +11793/−389 | 9 D +4856/−3 | 6 D +678/−24 | 22 D +23097/−1361 |
| `SONDE-012` | `9abb66f..c978148` | 12 | 7 | 118 D +9801/−1925 | 11 D +3528/−135 | 9 D +767/−28 | 17 D +26248/−37 |
| `NAK-96` | `2271df5..c82d8ce` | 23 | 21 | — | — | 5 D +407/−69 | 12 D +19910/−8 |
| `NAK-123` | `e0a0dd6..5e41a28` | 23 | 10 | 22 D +2008/−291 | 9 D +1814/−19 | 4 D +399/−33 | 43 D +12586/−143 |
| `G0` | `0df26c8..48acd89` | 125 | 4 | 1 D +124/−0 | — | 30 D +1718/−899 | 118 D +22868/−878 |
| `G1` | `4ec7480..b75ea06` | 396 | 20 | 153 D +21621/−1212 | 12 D +5839/−38 | 79 D +8762/−658 | 246 D +246569/−4264 |
| `G2` | `e77ebf9..5e41a28` | 24 | 5 | 22 D +2008/−291 | 9 D +1814/−19 | 4 D +399/−33 | 43 D +12800/−142 |
| `PR1` | `6f40eed..d20201e` | 17 | 18 | — | — | — | 8 D +1776/−66 |
| `PR2` | `55cdb91..008989c` | 28 | 14 | — | — | 2 D +371/−3 | 30 D +7091/−584 |
| `SONDE-003b` | **keine Spanne ermittelbar** — Kopf nennt keinen SHA, und kein Commit trägt den Ticketnamen im Betreff | — | 0 | — | — | — | — |

**Tabelle 2 — ticketgenau: nur die Commits mit dem Ticketnamen im Betreff.**

| Ticket | Ticketcommits | Produkt | Tests | Prüfwerkzeug | Doku | Produkt+Tests | Prüfwerkzeug : Produkt+Tests |
|---|---:|---|---|---|---|---:|---:|
| `S0` | 2 | — | — | 1 D +609/−0 | 5 D +497/−60 | **0** | ∞ (0 Produktzeilen) |
| `SONDE-001/002` | 1 | 10 D +135/−42 | 2 D +489/−9 | 1 D +75/−39 | 6 D +845/−90 | 675 | 0.17 |
| `SONDE-003` | 6 | 5 D +856/−36 | 1 D +578/−1 | 1 D +14/−2 | 5 D +2981/−86 | 1471 | 0.01 |
| `SONDE-004` | 3 | 8 D +812/−26 | 1 D +290/−0 | 3 D +237/−15 | 11 D +896/−290 | 1128 | 0.22 |
| `SONDE-004a` | 1 | 7 D +776/−0 | 1 D +290/−0 | 2 D +90/−2 | 4 D +469/−42 | 1066 | 0.09 |
| `SONDE-005a` | 15 | 213 D +25830/−7768 | 2 D +978/−52 | 6 D +2947/−140 | 10 D +7621/−709 | 34628 | 0.09 |
| `SONDE-005b` | 10 | 69 D +2254/−75 | 2 D +254/−0 | 8 D +1503/−33 | 9 D +4530/−114 | 2583 | 0.59 |
| `SONDE-006` | 8 | 57 D +4836/−103 | 4 D +914/−13 | 2 D +699/−2 | 7 D +4309/−243 | 5866 | 0.12 |
| `SONDE-007a` | 80 | 8 D +644/−44 | — | 4 D +4493/−518 | 27 D +90954/−18662 | 688 | 7.28 |
| `SONDE-007b` | 22 | 14 D +1549/−59 | 4 D +1033/−68 | 3 D +481/−15 | 10 D +26089/−155 | 2709 | 0.18 |
| `SONDE-007c` | 13 | 7 D +419/−75 | 1 D +136/−10 | 3 D +643/−56 | 16 D +58145/−38056 | 640 | 1.09 |
| `SONDE-008` | 15 | 9 D +1566/−210 | 4 D +1783/−8 | 1 D +29/−4 | 7 D +12114/−207 | 3567 | 0.01 |
| `SONDE-009` | 11 | 11 D +2741/−73 | 1 D +2150/−15 | 2 D +234/−1 | 4 D +24588/−93 | 4979 | 0.05 |
| `SONDE-010` | 45 | 62 D +10371/−750 | 4 D +3589/−87 | 4 D +710/−4 | 19 D +71581/−38281 | 14797 | 0.05 |
| `SONDE-011` | 19 | 105 D +12145/−741 | 9 D +4990/−137 | 6 D +681/−27 | 15 D +21454/−1370 | 18013 | 0.04 |
| `SONDE-012` | 7 | 119 D +10055/−2179 | 11 D +3553/−160 | 9 D +781/−42 | 11 D +26001/−15 | 15947 | 0.05 |
| `NAK-96` | 21 | — | — | 5 D +467/−104 | 13 D +19700/−95 | **0** | ∞ (0 Produktzeilen) |
| `NAK-123` | 10 | 22 D +2041/−324 | 9 D +1835/−40 | — | 9 D +10028/−18 | 4240 | 0.00 |
| `G0` | 4 | — | — | 2 D +2/−1 | 7 D +510/−58 | **0** | ∞ (0 Produktzeilen) |
| `G1` | 20 | 27 D +1433/−236 | 1 D +72/−0 | 4 D +383/−12 | 19 D +19447/−110 | 1741 | 0.23 |
| `G2` | 5 | 22 D +1902/−278 | 9 D +1637/−18 | — | 8 D +611/−36 | 3835 | 0.00 |
| `PR1` | 18 | — | — | — | 8 D +1900/−180 | **0** | — |
| `PR2` | 14 | — | — | 2 D +360/−4 | 13 D +6015/−395 | **0** | ∞ (0 Produktzeilen) |
| `SONDE-003b` | 0 | — | — | — | — | **0** | — |

**Tabelle 3 — je Runde, dort wo das Manifest Runden-SHAs nennt.** 158 Runden über elf Tickets;
davon **113 ohne Produktfortschritt** (Produkt + Tests = 0 Zeilen).

| Ticket | Runde (Spanne) | Produkt | Tests | Prüfwerkzeug | Doku | Produkt+Tests |
|---|---|---|---|---|---|---:|
| `SONDE-003` | 1 · `0ba87cc..359b899` | 5 D +746/−1 | 1 D +490/−0 | 1 D +14/−2 | 5 D +1375/−59 | 1237 |
| `SONDE-003` | 2 · `359b899..3f2fccd` | 1 D +89/−19 | 1 D +88/−1 | — | 1 D +1056/−12 | 197 |
| `SONDE-003` | 3 · `3f2fccd..e9c6fa0` | 1 D +6/−1 | — | — | 4 D +542/−7 | 7 |
| `SONDE-007a` | 1 · `dafa5a5..a728fba` | 162 D +26222/−931 | 14 D +9911/−49 | 91 D +8968/−952 | 220 D +165559/−1189 | 37113 |
| `SONDE-007a` | 2 · `a728fba..d3c741c` | — | — | 6 D +709/−71 | 15 D +20141/−15 | **0** |
| `SONDE-007a` | 3 · `d3c741c..b6003c1` | — | — | — | 2 D +245/−1 | **0** |
| `SONDE-007a` | 4 · `b6003c1..facea2d` | — | — | — | 5 D +3026/−7 | **0** |
| `SONDE-007a` | 5 · `facea2d..5acf7f7` | — | — | 3 D +727/−58 | 1 D +499/−0 | **0** |
| `SONDE-007a` | 6 · `5acf7f7..0ea62e4` | — | — | — | 1 D +1/−1 | **0** |
| `SONDE-007a` | 7 · `0ea62e4..5538fb0` | — | — | — | 4 D +3030/−3 | **0** |
| `SONDE-007a` | 8 · `5538fb0..cd346e1` | — | — | — | 1 D +664/−0 | **0** |
| `SONDE-007a` | 9 · `cd346e1..3353301` | — | — | 2 D +1071/−82 | — | **0** |
| `SONDE-007a` | 10 · `3353301..da62dec` | 1 D +34/−8 | — | 1 D +16/−6 | 6 D +9963/−9 | 42 |
| `SONDE-007a` | 11 · `da62dec..ccb98cd` | 1 D +27/−3 | — | 3 D +624/−66 | 5 D +3698/−12 | 30 |
| `SONDE-007a` | 12 · `ccb98cd..370e513` | — | — | 2 D +180/−16 | 2 D +145/−1 | **0** |
| `SONDE-007a` | 13 · `370e513..b80fdce` | — | — | — | 4 D +3079/−4 | **0** |
| `SONDE-007a` | 14 · `b80fdce..69b4d20` | — | — | 1 D +430/−98 | 5 D +70/−4 | **0** |
| `SONDE-007a` | 15 · `69b4d20..60717c5` | — | — | 2 D +90/−2 | — | **0** |
| `SONDE-007a` | 16 · `60717c5..5df7497` | — | — | — | 3 D +415/−6 | **0** |
| `SONDE-007a` | 17 · `5df7497..a94c33e` | — | — | — | 3 D +3082/−2 | **0** |
| `SONDE-007a` | 18 · `a94c33e..3a20064` | — | — | 2 D +183/−49 | 2 D +44/−0 | **0** |
| `SONDE-007a` | 19 · `3a20064..5dfe3a3` | — | — | 2 D +94/−5 | — | **0** |
| `SONDE-007a` | 20 · `5dfe3a3..93e8a7c` | — | — | 1 D +1/−1 | 3 D +578/−5 | **0** |
| `SONDE-007a` | 21 · `93e8a7c..401d036` | — | — | — | 3 D +3056/−1 | **0** |
| `SONDE-007a` | 22 · `401d036..f808ad0` | — | — | 2 D +25/−13 | 2 D +53/−1 | **0** |
| `SONDE-007a` | 23 · `f808ad0..f131090` | — | — | 2 D +142/−16 | 3 D +476/−2 | **0** |
| `SONDE-007a` | 24 · `f131090..d4f7ed3` | — | — | — | 2 D +17/−11 | **0** |
| `SONDE-007a` | 25 · `d4f7ed3..dd896a5` | — | — | — | 3 D +3055/−0 | **0** |
| `SONDE-007a` | 26 · `dd896a5..32d86d9` | — | — | 1 D +2/−1 | 1 D +45/−10 | **0** |
| `SONDE-007a` | 27 · `32d86d9..a010d64` | — | — | 1 D +16/−0 | 3 D +48/−0 | **0** |
| `SONDE-007a` | 28 · `a010d64..70f5bad` | 5 D +298/−0 | — | 2 D +641/−249 | — | 298 |
| `SONDE-007a` | 29 · `70f5bad..308947d` | — | — | 2 D +44/−19 | — | **0** |
| `SONDE-007a` | 30 · `308947d..00d2796` | — | — | 1 D +7/−2 | 3 D +580/−5 | **0** |
| `SONDE-007a` | 31 · `00d2796..e9ea54b` | — | — | — | 3 D +3054/−3 | **0** |
| `SONDE-007a` | 32 · `e9ea54b..165d9ae` | — | — | — | 2 D +38/−0 | **0** |
| `SONDE-007a` | 33 · `165d9ae..b9f7ee1` | — | — | 2 D +38/−15 | 2 D +5/−1 | **0** |
| `SONDE-007a` | 34 · `b9f7ee1..9602d6c` | — | — | 3 D +209/−119 | 3 D +541/−1 | **0** |
| `SONDE-007a` | 35 · `9602d6c..4a379bb` | — | — | — | 4 D +3080/−1 | **0** |
| `SONDE-007a` | 36 · `4a379bb..d11be90` | — | — | — | 2 D +425/−0 | **0** |
| `SONDE-007a` | 37 · `d11be90..196f97e` | — | — | 2 D +100/−21 | 2 D +320/−1 | **0** |
| `SONDE-007a` | 38 · `196f97e..d084296` | — | — | — | 3 D +3083/−0 | **0** |
| `SONDE-007a` | 39 · `d084296..88255d8` | — | — | 2 D +14/−6 | 2 D +38/−0 | **0** |
| `SONDE-007a` | 40 · `88255d8..f423527` | — | — | 2 D +114/−13 | 3 D +448/−1 | **0** |
| `SONDE-007a` | 41 · `f423527..e63a53f` | — | — | — | 3 D +3111/−17 | **0** |
| `SONDE-007a` | 42 · `e63a53f..4287839` | — | — | 1 D +6/−1 | 2 D +47/−0 | **0** |
| `SONDE-007a` | 43 · `4287839..1991ff8` | — | — | 2 D +238/−4 | 3 D +613/−1 | **0** |
| `SONDE-007a` | 44 · `1991ff8..713f0ae` | — | — | — | 4 D +3135/−28 | **0** |
| `SONDE-007a` | 45 · `713f0ae..c212280` | — | — | 2 D +283/−12 | 2 D +42/−0 | **0** |
| `SONDE-007a` | 46 · `c212280..50615f7` | — | — | 3 D +457/−12 | — | **0** |
| `SONDE-007a` | 47 · `50615f7..12fcdab` | — | — | 2 D +2/−2 | 3 D +578/−3 | **0** |
| `SONDE-007a` | 48 · `12fcdab..4a2b8da` | — | — | — | 3 D +3092/−3 | **0** |
| `SONDE-007a` | 49 · `4a2b8da..3de3a13` | — | — | 2 D +250/−57 | 2 D +44/−0 | **0** |
| `SONDE-007a` | 50 · `3de3a13..219424f` | 1 D +15/−2 | — | 2 D +135/−37 | 4 D +745/−5 | 17 |
| `SONDE-007a` | 51 · `219424f..27865ca` | — | — | — | 3 D +3123/−0 | **0** |
| `SONDE-007a` | 52 · `27865ca..75466c0` | — | — | — | 2 D +40/−0 | **0** |
| `SONDE-007a` | 53 · `75466c0..65e5b77` | — | — | 2 D +254/−11 | — | **0** |
| `SONDE-007a` | 54 · `65e5b77..3ef3efa` | — | — | 4 D +177/−28 | 4 D +631/−6 | **0** |
| `SONDE-007a` | 55 · `3ef3efa..eb84bec` | — | — | — | 4 D +3149/−10 | **0** |
| `SONDE-007a` | 56 · `eb84bec..d4900ce` | — | — | 2 D +122/−10 | 2 D +40/−0 | **0** |
| `SONDE-007a` | 57 · `d4900ce..ae32ea4` | — | — | — | 1 D +32/−11 | **0** |
| `SONDE-007a` | 58 · `ae32ea4..0e7a60e` | — | — | 2 D +111/−10 | 3 D +510/−1 | **0** |
| `SONDE-007a` | 59 · `0e7a60e..6cd244d` | — | — | — | 3 D +3226/−21 | **0** |
| `SONDE-007a` | 60 · `6cd244d..c117e40` | — | — | 2 D +34/−16 | 3 D +35/−1 | **0** |
| `SONDE-007a` | 61 · `c117e40..f68cd9a` | — | — | — | 1 D +353/−15 | **0** |
| `SONDE-007a` | 62 · `f68cd9a..22d3695` | — | — | — | 2 D +3127/−5 | **0** |
| `SONDE-007a` | 63 · `22d3695..7a87b7d` | — | — | 2 D +21/−29 | 6 D +144/−17 | **0** |
| `SONDE-007a` | 64 · `7a87b7d..478e564` | — | — | — | 5 D +18435/−18344 | **0** |
| `SONDE-007b` | 1 · `4b500a4..a2fe0f5` | 44 D +7273/−260 | 8 D +4898/−45 | 24 D +2070/−748 | 56 D +55846/−388 | 12476 |
| `SONDE-007b` | 2 · `a2fe0f5..1ca5fdb` | 82 D +10686/−1184 | 9 D +2172/−44 | 73 D +5686/−334 | 192 D +72713/−1540 | 14086 |
| `SONDE-007b` | 3 · `1ca5fdb..79e337c` | — | — | — | 2 D +21/−5 | **0** |
| `SONDE-007b` | 4 · `79e337c..72146c1` | — | — | — | 2 D +2579/−9 | **0** |
| `SONDE-007b` | 5 · `72146c1..2da1ce8` | — | 1 D +53/−2 | 1 D +1/−1 | 3 D +599/−17 | 55 |
| `SONDE-007b` | 6 · `2da1ce8..d211431` | — | — | — | 2 D +2445/−2 | **0** |
| `SONDE-007b` | 7 · `d211431..1bfc8b0` | 68 D +8381/−68 | 6 D +2872/−29 | 13 D +2962/−551 | 52 D +145076/−20191 | 11350 |
| `SONDE-007b` | 8 · `1bfc8b0..3687ea4` | — | — | — | 2 D +3185/−0 | **0** |
| `SONDE-007b` | 9 · `3687ea4..36560b0` | — | — | — | 3 D +29/−9 | **0** |
| `SONDE-007c` | 1 · `9bb75ad..25b57ec` | 70 D +8656/−94 | 6 D +3128/−6 | 10 D +3977/−213 | 30 D +91532/−444 | 11884 |
| `SONDE-007c` | 2 · `25b57ec..f94d56e` | — | — | 3 D +612/−55 | 3 D +419/−9 | **0** |
| `SONDE-007c` | 3 · `f94d56e..370e513` | — | — | 1 D +1/−1 | — | **0** |
| `SONDE-007c` | 4 · `370e513..cb99ba0` | — | — | 2 D +518/−98 | 6 D +3147/−6 | **0** |
| `SONDE-007c` | 5 · `cb99ba0..5df7497` | — | — | 1 D +2/−2 | 3 D +415/−6 | **0** |
| `SONDE-007c` | 6 · `5df7497..a94c33e` | — | — | — | 3 D +3082/−2 | **0** |
| `SONDE-007c` | 7 · `a94c33e..5dfe3a3` | — | — | 3 D +277/−54 | 2 D +44/−0 | **0** |
| `SONDE-007c` | 8 · `5dfe3a3..93e8a7c` | — | — | 1 D +1/−1 | 3 D +578/−5 | **0** |
| `SONDE-007c` | 9 · `93e8a7c..898b28b` | — | — | — | 3 D +3101/−1 | **0** |
| `SONDE-007c` | 10 · `898b28b..53c10a3` | — | — | 3 D +152/−29 | 1 D +8/−1 | **0** |
| `SONDE-007c` | 11 · `53c10a3..f131090` | — | — | 1 D +16/−1 | 3 D +476/−2 | **0** |
| `SONDE-007c` | 12 · `f131090..d4f7ed3` | — | — | — | 2 D +17/−11 | **0** |
| `SONDE-007c` | 13 · `d4f7ed3..70f5bad` | 5 D +298/−0 | — | 4 D +659/−250 | 5 D +3148/−10 | 298 |
| `SONDE-007c` | 14 · `70f5bad..2f1f89b` | — | — | 3 D +51/−21 | — | **0** |
| `SONDE-007c` | 15 · `2f1f89b..00d2796` | — | — | — | 3 D +580/−5 | **0** |
| `SONDE-007c` | 16 · `00d2796..9602d6c` | — | — | 4 D +247/−134 | 4 D +3638/−5 | **0** |
| `SONDE-007c` | 17 · `9602d6c..4c3fbf8` | — | — | 2 D +100/−21 | 4 D +3505/−1 | **0** |
| `SONDE-007c` | 18 · `4c3fbf8..196f97e` | — | — | — | 2 D +320/−1 | **0** |
| `SONDE-007c` | 19 · `196f97e..f423527` | — | — | 3 D +128/−19 | 4 D +3569/−1 | **0** |
| `SONDE-007c` | 20 · `f423527..1991ff8` | — | — | 2 D +244/−5 | 4 D +3771/−18 | **0** |
| `SONDE-007c` | 21 · `1991ff8..12fcdab` | — | — | 3 D +740/−24 | 4 D +3755/−31 | **0** |
| `SONDE-007c` | 22 · `12fcdab..219424f` | 1 D +15/−2 | — | 3 D +385/−94 | 5 D +3881/−8 | 17 |
| `SONDE-007c` | 23 · `219424f..3ef3efa` | — | — | 4 D +416/−24 | 5 D +3794/−6 | **0** |
| `SONDE-007c` | 24 · `3ef3efa..e27974c` | — | — | — | 4 D +3189/−10 | **0** |
| `SONDE-007c` | 25 · `e27974c..0e7a60e` | — | — | 3 D +233/−20 | 3 D +542/−12 | **0** |
| `SONDE-007c` | 26 · `0e7a60e..aadca53` | 8 D +32/−347 | 1 D +38/−25 | 7 D +791/−5804 | 17 D +68864/−55453 | 442 |
| `SONDE-007c` | 27 · `aadca53..fff9e09` | — | — | — | 2 D +3169/−0 | **0** |
| `SONDE-007c` | 28 · `fff9e09..977bf48` | 1 D +11/−8 | — | — | 1 D +51/−0 | 19 |
| `SONDE-007c` | 29 · `977bf48..bae7a4e` | — | — | — | 3 D +27/−10 | **0** |
| `SONDE-009` | 1 · `ff24908..b2dc288` | 11 D +2732/−64 | 1 D +2135/−0 | 18 D +749/−721 | 55 D +24804/−331 | 4931 |
| `SONDE-009` | 2 · `b2dc288..3c0230a` | — | 1 D +4/−2 | — | 5 D +2308/−21 | 6 |
| `SONDE-009` | 3 · `3c0230a..9aa7887` | — | — | — | 4 D +2656/−8 | **0** |
| `SONDE-010` | 1 · `a7b0740..cdff93b` | 38 D +317/−0 | — | 1 D +449/−0 | — | 317 |
| `SONDE-010` | 2 · `cdff93b..2ac23d0` | 11 D +2652/−6 | 2 D +386/−0 | — | — | 3044 |
| `SONDE-010` | 3 · `2ac23d0..1b19cd1` | 12 D +2624/−1 | 2 D +1442/−0 | 1 D +13/−0 | — | 4067 |
| `SONDE-010` | 4 · `1b19cd1..10a4806` | — | — | 2 D +209/−0 | — | **0** |
| `SONDE-010` | 5 · `10a4806..97c956d` | — | 1 D +39/−0 | 1 D +1/−1 | — | 39 |
| `SONDE-010` | 6 · `97c956d..4500785` | 9 D +1509/−163 | 2 D +354/−41 | 1 D +23/−2 | 5 D +3547/−23 | 2067 |
| `SONDE-010` | 7 · `4500785..602e105` | 1 D +6/−1 | — | — | — | 7 |
| `SONDE-010` | 8 · `602e105..6fc3224` | 2 D +23/−3 | — | — | — | 26 |
| `SONDE-010` | 9 · `6fc3224..d137fa0` | — | — | — | 3 D +3354/−12 | **0** |
| `SONDE-010` | 10 · `d137fa0..65d46a0` | 9 D +650/−93 | 2 D +462/−25 | — | 4 D +195/−178 | 1230 |
| `SONDE-010` | 11 · `65d46a0..e5f5c27` | 1 D +14/−10 | — | — | — | 24 |
| `SONDE-010` | 12 · `e5f5c27..a0053e4` | 1 D +8/−0 | 1 D +114/−0 | — | 4 D +3448/−14 | 122 |
| `SONDE-010` | 13 · `a0053e4..1bdb93d` | 2 D +99/−44 | 1 D +53/−17 | — | 9 D +6578/−389 | 213 |
| `SONDE-010` | 14 · `1bdb93d..c444ca3` | 2 D +473/−31 | — | — | — | 504 |
| `SONDE-010` | 15 · `c444ca3..66e4a09` | 2 D +9/−0 | 1 D +30/−2 | 1 D +14/−0 | — | 41 |
| `SONDE-010` | 16 · `66e4a09..e3e8e57` | — | — | — | 3 D +438/−5 | **0** |
| `SONDE-010` | 17 · `e3e8e57..a88d32e` | 1 D +4/−4 | — | — | 2 D +3010/−0 | 8 |
| `SONDE-010` | 18 · `a88d32e..444e125` | — | — | — | 1 D +2960/−0 | **0** |
| `SONDE-010` | 19 · `444e125..05235cf` | 4 D +222/−23 | 1 D +38/−25 | 10 D +1932/−545 | 48 D +146587/−44089 | 308 |
| `SONDE-010` | 20 · `05235cf..b4fe522` | 9 D +1726/−343 | 2 D +709/−2 | 1 D +1/−1 | 7 D +6680/−5 | 2780 |
| `SONDE-010` | 21 · `b4fe522..e517165` | 1 D +201/−10 | — | — | 3 D +3386/−0 | 211 |
| `SONDE-010` | 22 · `e517165..53f517b` | — | — | — | 1 D +114/−0 | **0** |
| `SONDE-010` | 23 · `53f517b..83f7d7e` | — | — | — | 3 D +3248/−1 | **0** |
| `SONDE-010` | 24 · `83f7d7e..d1ef796` | — | — | — | 4 D +36/−13 | **0** |
| `SONDE-011` | 1 · `b75ea06..b327c1d` | 87 D +4658/−118 | 4 D +292/−2 | 4 D +549/−20 | 2 D +304/−1 | 5070 |
| `SONDE-011` | 2 · `b327c1d..d172b0a` | 9 D +83/−89 | 3 D +3/−3 | — | 5 D +6649/−7 | 178 |
| `SONDE-011` | 3 · `d172b0a..901598f` | 24 D +6887/−278 | 6 D +4307/−1 | 3 D +132/−7 | 16 D +12555/−1371 | 11473 |
| `SONDE-011` | 4 · `901598f..5882718` | 4 D +294/−41 | 3 D +142/−11 | — | 1 D +48/−0 | 488 |
| `SONDE-011` | 5 · `5882718..e917526` | 1 D +15/−10 | 2 D +129/−13 | — | 1 D +33/−0 | 167 |
| `SONDE-011` | 6 · `e917526..f176f24` | 1 D +13/−10 | 1 D +18/−8 | — | 1 D +38/−0 | 49 |
| `SONDE-011` | 7 · `f176f24..9abb66f` | — | — | — | 6 D +3505/−17 | **0** |
| `NAK-96` | 1 · `2271df5..8a1ea8a` | — | — | 2 D +182/−62 | 9 D +9085/−4 | **0** |
| `NAK-96` | 2 · `8a1ea8a..d993894` | — | — | — | 2 D +3064/−0 | **0** |
| `NAK-96` | 3 · `d993894..b4b1e29` | — | — | 3 D +199/−18 | 4 D +3876/−7 | **0** |
| `NAK-96` | 4 · `b4b1e29..f124746` | — | — | — | 2 D +4/−4 | **0** |
| `NAK-96` | 5 · `f124746..34491e0` | — | — | — | 3 D +3092/−2 | **0** |
| `NAK-96` | 6 · `34491e0..631ac34` | — | — | 2 D +7/−3 | 1 D +110/−22 | **0** |
| `NAK-96` | 7 · `631ac34..a9c6450` | — | — | — | 1 D +2/−2 | **0** |
| `NAK-96` | 8 · `a9c6450..7be6cd5` | — | — | 1 D +50/−22 | — | **0** |
| `NAK-96` | 9 · `7be6cd5..202f2f8` | — | — | 1 D +6/−1 | 3 D +722/−42 | **0** |
| `NAK-96` | 10 · `202f2f8..c82d8ce` | — | — | — | 3 D +34/−4 | **0** |
| `NAK-123` | 1 · `e0a0dd6..d8676e0` | 22 D +2008/−291 | 9 D +1814/−19 | 4 D +399/−33 | 40 D +7928/−135 | 4132 |
| `NAK-123` | 2 · `d8676e0..0b174d2` | — | — | — | 3 D +4584/−2 | **0** |
| `NAK-123` | 3 · `0b174d2..5e41a28` | — | — | — | 5 D +81/−13 | **0** |
| `G1` | 1 · `4ec7480..c72d51e` | 152 D +20977/−1211 | 12 D +5521/−38 | 79 D +8740/−658 | 236 D +224630/−4220 | 27747 |
| `G1` | 2 · `c72d51e..b75ea06` | 8 D +714/−71 | 1 D +318/−0 | 2 D +27/−5 | 22 D +21978/−83 | 1103 |
| `PR1` | 1 · `6f40eed..0e3908e` | — | — | — | 8 D +934/−55 | **0** |
| `PR1` | 2 · `0e3908e..b54a575` | — | — | — | 6 D +583/−38 | **0** |
| `PR1` | 3 · `b54a575..6127595` | — | — | — | 3 D +244/−10 | **0** |
| `PR1` | 4 · `6127595..d20201e` | — | — | — | 3 D +66/−14 | **0** |

**Was diese Bilanz sagt — und was sie gegenüber §4.2 korrigiert.**

1. **Die Zahl der Runden ist mehr als doppelt so hoch wie gedacht.** §4.2 nannte für
   `SONDE-007a` „76 Commits, davon 76 Runde"; gemessen sind es **80 Ticketcommits und 64
   Runden**, von denen **59 keine einzige Zeile Produktcode oder Test bewegt haben**. Über alle
   Tickets: 158 Runden, 113 ohne Produktfortschritt — **71,5 %**.
2. **Das Verhältnis Prüfwerkzeug zu Produkt war in §4.2 zu günstig gerechnet.** Dort stand für
   `SONDE-007a` „Produkt +533/−22 gegen Prüfwerkzeug +6 661/−243", also Faktor 12,5. Das waren
   Spannenzahlen. Ticketgenau sind es **688 Zeilen Produkt+Tests gegen 5 011 Zeilen
   Prüfwerkzeug — Faktor 7,3** — und **109 616 Zeilen Doku**, also Faktor **159** zwischen Doku
   und Produkt. Der Befund wird dadurch nicht kleiner, er wandert nur: das teuerste Erzeugnis
   dieses Tickets war nicht das Prüfwerkzeug, es war das Manifest.
3. **Fünf Tickets haben ticketgenau null Zeilen Produkt und Tests bewegt:** `S0`, `NAK-96`,
   `G0`, `PR1` und `PR2`. Bei `PR1`, `PR2` und `G0` ist das die Ticketart (Planprüfung, Gate);
   bei `S0` und `NAK-96` ist es Werkzeugarbeit. Das ist derselbe Befund wie in §4.2, nur
   vollständig statt an vier Beispielen.
4. **Ein Ticket hat keine ermittelbare Spanne:** `SONDE-003b`. Grund, gemessen: sein
   Manifestkopf nennt keinen SHA, und **kein** Commit seit S0 trägt `SONDE-003b` im Betreff —
   die Arbeit steckt in den `SONDE-003`-Commits. Es ist damit nicht ohne Beleg, sondern ohne
   eigene Commitspur.
5. **Zwei Commits haben keinen Elternteil** und stehen deshalb in der Klassenrechnung ohne
   Pfade: `ef3124d` („Nakama-Design: kontextreines Design-Projekt angelegt", 20.08.2026) und
   `0285465` („Build Nakama briefing hub", 22.08.2026) — beides Wurzelcommits eingehängter
   Fremdhistorien.
6. **Die Commitklassen am heutigen Stand** (821 Commits seit S0, S0 eingeschlossen; §4.1 hatte
   797 auf `16be249`): berührt Produkt oder Tests **142 (17,3 %)**, nur Doku **483 (58,8 %)**,
   nur Prüfwerkzeug **51 (6,2 %)**, nur Prüfwerkzeug + Doku **42 (5,1 %)**, gemischt/sonstiges
   **103 (12,5 %)**. Anteil Werkzeug-Commits **93 von 821 = 11,3 %**; Commits ohne jede
   Produkt- oder Testzeile **679 von 821 = 82,7 %**. Berührte Dateipfade: Doku 1 749 ·
   Produkt 1 215 · Sonstiges 917 · Prüfwerkzeug 405 · Tests 139.
7. **Kanonläufe am heutigen Stand:** 63 Rohläufe unter `docs/beweise/roh/` (§4.3 zählte 62),
   davon GRÜN 38, ROT 14, NICHT BEGLAUBIGT 1, ohne Urteilszeile 10 — die zehnte ist die
   Rohdatei dieser Bilanz selbst, die kein Kanonlauf ist. Die in §4.3 namentlich genannten
   verlorenen und abgebrochenen Läufe sind unverändert.

**Was diese Runde NICHT gemessen hat.** Die Klassenzuordnung stammt unverändert aus
`rundenbilanz.py`; das Werkzeug ist nicht angefasst worden. Ein Commit, dessen Betreff den
Ticketnamen nicht trägt, taucht in Tabelle 2 nicht auf — das ist eine Aussage über die
Betreffdisziplin, nicht über die Arbeit.

**Außerhalb der vierzehn Defekte, als Registerzeile statt im Änderungssatz: NAK-130.**
Beim Rebase auf `origin/master` kam die Arbeit der parallelen Design-Session desselben Tages
dazu (Fünferblöcke 03 bis 05). Dabei ist ein Widerspruch entstanden, der nicht zu den vierzehn
Defekten gehört: `plan.json` S18–19 Punkt (10) nennt `SEND DRAFT` weiter als einzigen Weg von
Gen Fläche 1 zu Fläche 2, während
`design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md` genau diesen Entscheid
seit dem 02.09.2026 als **überholt** führt (Fünferblock 03, Entscheidung 3/5). `S31b` trägt den
neuen Entscheid bereits, `S18–19` nicht. Der Auftrag dieser Runde verlangt für solche Funde
eine datierte Registerzeile mit sichtbarer Klasse — sie steht als **NAK-130**
`[Planarbeit · S18–19]` im Register und ist hier nicht behoben worden.

### 12.3 Änderungssatz der Runde

Vier Commits, jeder mit explizitem Pathspec, jeder sofort gepusht.

| # | Commit | Pfade | Defekte |
|---|---|---|---|
| 1 | `2a04a27` | `docs/offene-punkte.md` | D1, D2 (Register), D5 (Register), D7 (Register), D8, D9, D10 |
| 2 | `940406e` | `docs/plan/plan.json`, `docs/plan/fragen.json`, `docs/bauaufteilung-sonden.md`, `docs/offene-punkte.md` | D2 (G3-Text), D3, D5, D6, D7 (Plan), D11, D12, D13 + NAK-130 |
| 3 | `008989c` | `docs/FL-Nakama-Sonden-Design-Entwurf.md` | D4 |
| 4 | `5f83a2b` | `docs/beweise/PR2.md`, `docs/beweise/roh/PR2-rundenbilanz-008989c.md`, `docs/plan/plan.json`, `docs/PLAN-STAND.md`, `docs/ANTWORTEN-OFFEN.md` | D14, lebender Kopf, letzte D5-Zitatstelle |
| — | `3ad6e8f` und die Folgecommits mit demselben Betreffkopf | `docs/PLAN-STAND.md` | kein Arbeitscommit: das erzeugte Blatt, nach jedem Rebase und nach jedem Manifest-Commit einmal neu gerechnet und mit Pathspec committet (`CLAUDE.md`-Regel) |

`docs/offene-punkte.md` steht in zwei Commits: die Registerarbeit der Befunde D1/D2/D8/D9/D10
liegt in Commit 1, die eine Zeile NAK-130 in Commit 2, weil sie erst nach dem Rebase auf
`origin/master` messbar wurde. `docs/plan/plan.json` steht ebenfalls in zwei Commits: die
Befunde D2, D3, D5, D6 und D7 in Commit 2, und in Commit 4 die **fünfte** D5-Zitatstelle in
`S31b`, die der Riegel E4 erst nach Commit 2 gefunden hat — sie trug den Wortlaut mit einem
anderen Schlusszeichen und ist der Restsuche des ersten Durchlaufs entgangen. Jeder Commit nennt seine Pfade explizit; `.workflow/ultracode/…`
ist in keinem.

**Der Rebase.** Während dieser Runde hat eine parallele Session auf demselben Branch 18 Commits
gepusht (Fünferblöcke 03 bis 05, Blueprint, Skizze, `CLAUDE.md`, `dokuriegel.py`, `plan.json`
S23–25/S31b, `fragen.json` U21). Commit 1 ist darauf rebased worden; kein fremder Pfad ist
angefasst, kein fremder Commit verändert. Zwei Folgen sind in diesem Manifest vermerkt: die
Kartenzahl (U21 beantwortet) und die Blueprint-§15-Punkte (18 und 19 dazu) in §1.2, und der
Widerspruch NAK-130.

### 12.4 Beweislauf

Befehle, Reihenfolge und Exitcodes, gefahren am 2026-09-02 vom Workspace-Root
`C:/Users/phili/Projekte/Nakama`. Rohausgabe ungekürzt. **D und H messen den Arbeitsbaum gegen
den Stand vor der Runde** — sie enthalten also auch dieses Manifest und die zwei erzeugten
Blätter, die erst mit Commit 4 committet werden.

```text
### A) py -3.13 tools/plan/planstand.py
geschrieben: docs\PLAN-STAND.md (23 abgenommen, 1 gebaut, 40 gesamt, aus 940406e)
EXITCODE=0

### B) py -3.13 tools/plan/antworten_blatt.py
geschrieben: docs\ANTWORTEN-OFFEN.md (53 Antworten, 0 offen, 53 eingearbeitet, 24 mit eigenem Text)
EXITCODE=0

### C) JSON-Gueltigkeit aller docs/plan/*.json
docs/plan/fragen.json -> gueltiges JSON, 35953 Bytes
docs/plan/plan.json -> gueltiges JSON, 90860 Bytes
EXITCODE=0

### D) git diff --stat 8a8d2ce  (Arbeitsbaum gegen den Elternstand von Runde 1)
 docs/ANTWORTEN-OFFEN.md                 |  12 +-
 docs/FL-Nakama-Sonden-Design-Entwurf.md |  45 +-
 docs/PLAN-STAND.md                      |  26 +-
 docs/bauaufteilung-sonden.md            |  51 ++-
 docs/beweise/PR2.md                     | 718 +++++++++++++++++++++++++++++++-
 docs/offene-punkte.md                   |  99 +++--
 docs/plan/fragen.json                   |  12 +-
 docs/plan/plan.json                     |  16 +-
 8 files changed, 889 insertions(+), 90 deletions(-)
warning: in the working copy of 'docs/ANTWORTEN-OFFEN.md', LF will be replaced by CRLF the next time Git touches it
warning: in the working copy of 'docs/beweise/PR2.md', LF will be replaced by CRLF the next time Git touches it
EXITCODE=0

### D2) git diff --stat 16be249  (zusaetzlich: Runde 1 UND die 18 Commits der parallelen Design-Session)
 CLAUDE.md                                          |  25 +-
 design/LIES-MICH.md                                | 308 ++++-----
 .../2026-08-31-eq-arbeitsflaeche-dominiert.md      |  20 +
 ...08-31-technische-ui-architektur-arbeitsmodus.md |  73 ++-
 ...02-technische-ui-architektur-fuenferblock-03.md | 293 +++++++++
 ...02-technische-ui-architektur-fuenferblock-04.md | 218 +++++++
 ...02-technische-ui-architektur-fuenferblock-05.md | 105 +++
 design/docs/funktions-und-bedien-blueprint.md      | 107 ++-
 design/skizze/LIES-MICH.md                         | 106 +--
 .../2026-09-02-b03-gen-flaeche-1-draft-open.png    | Bin 0 -> 110768 bytes
 .../belege/2026-09-02-b03-gen-flaeche-1-kopf.png   | Bin 0 -> 111074 bytes
 .../2026-09-02-b03-gen-flaeche-2-global-offen.png  | Bin 0 -> 127860 bytes
 .../2026-09-02-b03-gen-flaeche-2-global-zeile.png  | Bin 0 -> 113348 bytes
 .../belege/2026-09-02-b03-probeeq-kachel-eq-an.png | Bin 0 -> 62222 bytes
 .../2026-09-02-b03-probeeq-kachel-eq-aus.png       | Bin 0 -> 56366 bytes
 .../2026-09-02-gen-flaeche-1-sources.png           | Bin
 .../2026-09-02-gen-flaeche-2-band2-aus.png         | Bin
 .../2026-09-02-gen-flaeche-2-eq.png                | Bin
 .../2026-09-02-gen-flaeche-2-filtertyp.png         | Bin
 .../2026-09-02-probeeq.png                         | Bin
 design/skizze/nakama-ui-technical-sketch.html      | 287 +++++---
 docs/ANTWORTEN-OFFEN.md                            |  18 +-
 docs/FL-Nakama-Sonden-Design-Entwurf.md            |  45 +-
 docs/PLAN-STAND.md                                 |  30 +-
 docs/bauaufteilung-sonden.md                       |  51 +-
 docs/beweise/PR2.md                                | 718 ++++++++++++++++++++-
 docs/offene-punkte.md                              |  99 ++-
 docs/plan/fragen.json                              |  31 +-
 docs/plan/plan.json                                |  18 +-
 tools/plan/dokuriegel.py                           |  22 +-
 30 files changed, 2167 insertions(+), 407 deletions(-)
warning: in the working copy of 'docs/ANTWORTEN-OFFEN.md', LF will be replaced by CRLF the next time Git touches it
warning: in the working copy of 'docs/beweise/PR2.md', LF will be replaced by CRLF the next time Git touches it
EXITCODE=0

### E1) Registerzeilen: Mengenvergleich der IDs 55cdb91 gegen Arbeitsbaum
Basis 55cdb91: 126 IDs
Arbeitsbaum: 129 IDs
ABGAENGE (muss leer sein): (leer)
ZUGAENGE: NAK-128, NAK-129, NAK-130
EXITCODE=0

### E2) rg -n "Prisma-App|Prisma daneben|Begleit-App" docs/FL-Nakama-Sonden-Design-Entwurf.md
94: konventionell / Prisma daneben gilt. **Aber** seit 21.08.: „Familie; Prisma nur
96: „Glas/Licht raus; Profil nur Studie". Folge: alle Aussagen über die Prisma-App
97: als Begleit-App, read-only-Spiegel oder Broker-Client ohne
99: §3.5 zweiter Absatz, §30 Zeile „Prisma-App", §31.1 letzter Punkt, §35.1) sind
464: ### 0.3 Produktentscheidung 20.08.2026 — Master-Plugin konventionell, Prisma-App als Zusatz
468: „Master-Plugin konventionell / Prisma daneben“ gilt weiter (Entscheid 21.08.2026 zur Festlegung
469: vom 20.08.: „Meine Entscheidung, so gesagt“) — **aber** alle Aussagen über die Prisma-App als
470: Begleit-App, read-only-Spiegel oder Broker-Client sind Studie und kein Bauziel: kein Ticket, kein
476: nur am letzten Punkt dieses Abschnitts eingearbeitet, die Punkte davor definierten die Prisma-App
477: weiter als eigenständige Begleit-App mit eigener Oberfläche.*
493: - ~~**Die Prisma-App ist eine eigenständige Begleit-App**, die NEBEN
497:   **[0.5 · Erratum (c) · 21.08.2026]** **Studie, kein Bauziel.** Es gibt keine Prisma-App als Produktteil:
508:   der Prisma-App**, nicht der Master-Plugin-UI. Damit ist der im
518: - ~~Technisch bindet sich die Prisma-App … als read-only Spiegel an Broker und Sitzung an.~~
519:   **Gestrichen in Fassung 0.5 am 02.09.2026.** Grund: die Prisma-App ist kein Bauziel mehr —
559: 3. ~~**Zwei getrennte Design-Spuren:** Prisma-App und konventionelle Plugin-UI mit drei
716: ~~Davon getrennt ist die Prisma-App (§0.3): eine eigenständige, optionale Begleit-App neben
717: Hauptplugin und Sonden.~~ **Gestrichen in Fassung 0.5 am 02.09.2026.** Grund: die Prisma-App
1661: | ~~Prisma-App~~ | **Gestrichen in Fassung 0.5 am 02.09.2026** — die Prisma-App ist Studie, kein Bauziel: kein Ticket,
1749: - ~~bleibt das auch neben der Prisma-App: diese ist nur ein optionaler read-only Spiegel (§0.3),
1750:   nie Steuer-Hub.~~ **Gestrichen in Fassung 0.5 am 02.09.2026.** Grund: die Prisma-App ist
2276: Detailansicht … der Hörkompass-Zielvertrag bindet die Prisma-App, nicht diese Ansicht.~~
2278: UI-Annahme dieses Entwurfs, keine Oberflächenvorgabe, und die Prisma-App ist kein Bauziel.
23 Fundstellen; Behandlung je Stelle in Paragraph 12.2, Befund D4.
EXITCODE=0

### E3) nummerierte Ueberschriften des Entwurfs vor und nach Runde 1
16be249: 190   Arbeitsbaum: 190   gleiche Reihenfolge: True   nur vorher: []   nur nachher: []
EXITCODE=0

### E4) Restsuche: transliterierte oder unvollstaendige User-Zitate
  waere sinnvoller                   plan.json 0 | bauaufteilung 0 | offene-punkte 0 | fragen.json 0
  groesse naemlich                   plan.json 0 | bauaufteilung 0 | offene-punkte 0 | fragen.json 0
  waagrecht untereinander waeren     plan.json 0 | bauaufteilung 0 | offene-punkte 0 | fragen.json 0
  design die entworfen wurden"       plan.json 0 | bauaufteilung 0 | offene-punkte 0 | fragen.json 0
  entworfen wurden.“ (korrekt)       plan.json 1 | bauaufteilung 2 | offene-punkte 2 | fragen.json 1
EXITCODE=0

### E5) Riegel in den Edit-Skripten, die waehrend der Runde rot geworden sind
ABBRUCH Kopfzahlen 1: Anker nicht genau einmal (Kopftabelle des Registers)
    -> das Skript lief ein zweites Mal auf einer schon berichtigten Tabelle; auf
       Zahlmuster statt Volltext umgestellt, danach gruen.
ABBRUCH Manifestkopf 1: Anker 0 Treffer statt 1 fuer der Zeile "Aenderungssatz | 10 Dateien, ..."
    -> im Skript stand ein schmales Leerzeichen (U+202F), in der Datei ein gewoehnliches;
       nach der Korrektur gruen.
ABBRUCH Manifestkopf 2: Anker 0 Treffer statt 1 fuer Paragraph 10 Punkt 4
    -> im Skript stand ein typografisches Schlusszeichen, in der Datei ein gerades;
       nach der Korrektur gruen.
Jeder Abbruch hat die Datei UNVERAENDERT gelassen; der halb angewandte Stand wurde mit
git checkout auf den committeten zurueckgestellt, bevor das Skript erneut lief.

### F) py -3.13 tools/plan/dokuriegel.py <die sieben Ticketpfade>
docs/plan/plan.json: sauber
docs/bauaufteilung-sonden.md: sauber
docs/FL-Nakama-Sonden-Design-Entwurf.md: sauber (1 benannt)
design/docs/funktions-und-bedien-blueprint.md: sauber
CLAUDE.md: sauber
docs/offene-punkte.md: sauber (8 benannt)
docs/beweise/PR2.md: sauber (1 benannt)
docs/FL-Nakama-Sonden-Design-Entwurf.md:189 VERWEIS OHNE ZIEL, aber als historisch markiert („im Design-Repo“ im Umfeld): docs/sondenplan.md
docs/offene-punkte.md:81 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („nicht ins Repo“ im Umfeld): tools/extern/
docs/offene-punkte.md:84 VERWEIS OHNE ZIEL, aber als historisch markiert („im FL-Studio-Repo“ im Umfeld): tools/analyze-track.py
docs/offene-punkte.md:99 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): docs/hub/hub.json
docs/offene-punkte.md:99 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): tools/hub/hub_sync.py holen\|senden
docs/offene-punkte.md:196 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): eq-copilot/fixtures/installer/journale/
docs/offene-punkte.md:196 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): tools/eq-copilot/erzeuge_installer_journale.py
docs/offene-punkte.md:200 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/kern_riegel/
docs/offene-punkte.md:200 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/installer_pruefung/
docs/beweise/PR2.md:941 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): docs/beweise/NAK-121.md
Dokuriegel: 0 Befund(e), 10 benannt (historisch oder Zielpfad)
EXITCODE=0

### F+) derselbe Riegel zusaetzlich ueber docs/plan/fragen.json (in dieser Runde geaendert)
docs/plan/fragen.json: sauber
Dokuriegel: 0 Befund(e), 0 benannt (historisch oder Zielpfad)
EXITCODE=0

### H) py -3.13 tools/dirigent/rundenbilanz.py 55cdb91 (ganzes Ticket, Arbeitsbaum)
55cdb91: Pruefwerkzeug 2 Datei(en) +371/-3 | Doku 30 Datei(en) +7793/-584
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
EXITCODE=0

### H2) py -3.13 tools/dirigent/rundenbilanz.py 8a8d2ce (nur Runde 1, Arbeitsbaum)
8a8d2ce: Doku 8 Datei(en) +889/-90
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
EXITCODE=0

### H3) py -3.13 tools/dirigent/rundenbilanz.py --runden 8a8d2ce 2a04a27 940406e 008989c
8a8d2ce..2a04a27: Doku 1 Datei(en) +67/-29
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
2a04a27..940406e: Doku 4 Datei(en) +56/-30
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
940406e..008989c: Doku 1 Datei(en) +39/-6
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 3 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill §3.4, keine weitere Punktkorrektur-Runde.
EXITCODE=0

### H4) py -3.13 tools/dirigent/rundenbilanz.py 16be249 (Runde 1 UND die parallele Design-Session)
16be249: Pruefwerkzeug 1 Datei(en) +17/-5 | Doku 29 Datei(en) +2150/-402
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
EXITCODE=0
```

### Was die Läufe zeigen

- **A** `planstand.py` endet mit Exitcode 0 **ohne Warnung**: 23 abgenommen, 1 gebaut,
  **40 gesamt** (vorher 39 — der neue Schritt `S19b`). `S19b` erscheint richtig als *offen*,
  weil sein Beleg noch nicht existiert.
- **B** `antworten_blatt.py`: 53 Antworten, 0 offen, 53 eingearbeitet. Die 53. ist U21, von der
  parallelen Design-Session beantwortet; `wahl`, `text` und `datum` aller Karten sind
  unverändert — der Riegel im Edit-Skript vergleicht die drei Felder vor und nach dem Schreiben.
- **C** Beide Planquellen sind nach den Edits gültiges JSON. Jedes Edit-Skript hat vor dem
  Schreiben geprüft, dass `json.dumps(…, indent=1, ensure_ascii=False)` mit den vorgefundenen
  Zeilenenden **bytegleich** zur Ausgangsdatei ist — beide Dateien tragen seit dem Rebase CRLF,
  vorher LF; genau dafür ist der Riegel da.
- **D** ist der Änderungssatz **dieser Runde allein**: acht Dateien unter `docs/**`,
  +889 / −90 Zeilen, gemessen gegen `8a8d2ce`, den Elternstand des ersten Rundencommits.
  **Kein Produktcode, kein Test, kein Schema, kein Fixture, kein Runner, kein Prüfwerkzeug.**
  **D2** zeigt daneben, was zwischen `16be249` und heute insgesamt passiert ist — dort stehen
  auch `CLAUDE.md`, `design/**` und `tools/plan/dokuriegel.py` drin; das ist die parallele
  Design-Session, nicht diese Runde. Beide Zahlen enthalten dieses Manifest, aber **nicht**
  diesen Rohausgabeblock selbst: er ist nach der Messung eingesetzt worden.
- **E1** Der Mengenvergleich zu Befund D1: 126 IDs auf `55cdb91`, 129 heute, **null Abgänge**,
  drei Zugänge (NAK-128 und NAK-129 aus dem Erstdurchgang, NAK-130 aus dieser Runde).
- **E2** Die Vollständigkeitsprobe zu Erratum (c): 23 Fundstellen, jede in §12.2 D4 mit ihrer
  Behandlung genannt.
- **E3** Der Entwurf ist **nicht umnummeriert**: 190 nummerierte Überschriften vor und nach der
  Runde, identische Reihenfolge, keine hinzu, keine weg.
- **E4** Die Restsuche zu Befund D5: alle transliterierten Formen stehen auf **0**; die
  zeichengetreue Form steht an allen fünf Orten, an denen sie stehen soll.
- **E5** Die Riegel der Edit-Skripte haben in dieser Runde dreimal ausgelöst und jedes Mal die
  Datei unverändert gelassen. Das ist der Zweck: ein Anker, der nicht genau einmal trifft, darf
  nicht schreiben.
- **F** Der Dokuriegel läuft über die sieben Ticketpfade sauber, **Exit 0**: 0 Befunde, 10
  benannt. Die zehnte Nennung ist neu und gewollt — `docs/beweise/NAK-121.md`, der noch nicht
  angelegte Beleg von `S19b`. Im ersten Lauf dieser Runde war sie ein **Befund** (Exit 1), weil
  der Zielpfad nicht als solcher markiert war; die Stelle sagt es jetzt. `docs/plan/fragen.json`
  ist zusätzlich gefahren, weil diese Runde es ändert — ebenfalls sauber.
- **H** Das ganze Ticket seit `55cdb91`: kein Produktfortschritt. **H2** diese Runde allein:
  kein Produktfortschritt, 8 Dateien Doku. **H3** je Rundencommit — und hier meldet das
  Werkzeug ein **KONVERGENZ-SIGNAL** („3 Runden in Folge ohne Produktfortschritt"). Das ist
  richtig gerechnet und hier **kein Mangel**: `rundenbilanz.py` kennt keine Ticketart, und eine
  Planprüfung bewegt keinen Produktcode. Das Signal gehört gelesen als das, was es ist — ein
  Zähler, kein Urteil. **H4** zeigt dieselbe Rechnung zusammen mit der parallelen
  Design-Session, ebenfalls ohne Produktzeilen.

### 12.5 Prüfliste E und F, je Zeile mit Messort

#### E. Behauptung ≤ Messung

| Zeile | Wo gemessen |
|---|---|
| „Jede Behauptung sagt nicht mehr, als der Test misst" | Jede Zahl in §12.2 und §12.4 nennt ihren Lauf. Die Negativaussagen tragen ihren Befehl mit Trefferzahl: „null Abgänge" (Lauf E1), „0 Vorkommen von `waere sinnvoller`" (Lauf E4), „`grep` 0 Treffer für den Wortlaut in `design/LIES-MICH.md`" (§12.2 D5). Die Kanon-Beinanzahl wird auch hier **nicht** behauptet — der Kanon läuft nicht. |
| „Zahlen im Manifest sind gemessen, nicht abgeschrieben" | Die Registerzahlen (64/5/60) stammen aus dem Zählskript, nicht aus dem Kopf der Datei. Die Bilanzzahlen stammen aus `rundenbilanz.py`; die Rohausgabe liegt vollständig unter `docs/beweise/roh/`. **Drei Zahlen des Erstdurchgangs sind nachgemessen und korrigiert:** die Runden von `SONDE-007a` (§4.2 nannte „76 Commits, davon Runde 76"; gemessen sind **80 Ticketcommits und 64 Runden**, davon 59 ohne Produktfortschritt), das Verhältnis Prüfwerkzeug zu Produkt (12,5 aus Spannenzahlen → **7,3 ticketgenau**) und die Zahl der bilanzierten Tickets (11 → **24**). |
| „Positionen im lebenden Kopf stehen als Symbol/Anker" | Fundstellen sind Datei plus Symbol oder §-Nummer. Zeilennummern stehen nur dort, wo sie in dieser Runde gemessen wurden, und tragen den Dateinamen (`planstand.py:151`, `:102`; `…arbeitsmodus.md:147`, `:167`, `:257`; `…gen-nur-standardgroesse.md:9`). Der Entwurf ist **nicht** umnummeriert (Lauf E3), alle §-Verweise fremder Manifeste bleiben gültig. |
| „Der lebende Kopf wird beim Abschluss nachgezogen" | Zutreffend und ausgeführt: die Prüfstufen-Zeile trägt jetzt das Erstprüfungsurteil, §1.0 ist die neue Rundentabelle, §1.2 nennt zu jeder bewegten Zahl ihren neuen Wert, §3.4 trägt die Berichtigung, §6 (V-3) und §10 (Punkt 4) tragen ihre. |
| „Jede neue Prüfung wurde einmal absichtlich gebrochen" | Diese Runde führt **keine** neue Prüfung ein — `dokuriegel.py` und `rundenbilanz.py` sind unverändert. Die Riegel der Edit-Skripte (Trefferzahl je Anker, JSON-Roundtrip bytegleich, `wahl`/`text`/`datum`-Menge, Mengenvergleich der Register-IDs) haben während der Arbeit **dreimal** ausgelöst und sind in §12.4, Lauf E5 mit ihrer Meldung genannt. |
| „Geänderte Zusage — drei Stellen" | Zwei Zusagen sind geändert. (1) **NAK-97 ist geschlossen** — nachgezogen im Register (Umzug), in §6 (V-3 gegenstandslos) und in §10 (Punkt 4 berichtigt). (2) **Die Zahl der Bau-Sessions** — nachgezogen in `bauaufteilung-sonden.md` §3 (neue Zeile), §4 (41/29/51) und im Registerkopf (Blockerliste P3 · S19b). Der Runnerkopf `tools/beweise.ps1` ist nicht betroffen: keine Runner-Behauptung wird berührt. |
| „Writer-Fixtures statt Handschrift" | greift nicht — keine Fixtures im Änderungssatz. |

#### F. Änderungssatz

| Zeile | Wo gemessen |
|---|---|
| „speichern↔laden, starten↔stoppen, … im selben Commit" | Zwei Gegenpaare sind berührt, beide vollständig. (1) **löschen↔zurückholen**: die 13 in der Triage verlorenen Zeilen und ihr Mengenbeweis liegen in Commit 1, derselben Datei, demselben Änderungssatz. (2) **öffnen↔schließen** im Register: die 16 Umzüge in die Geschlossen-Tabelle und die zwei begründeten Nicht-Umzüge (NAK-85, NAK-92) liegen ebenfalls in Commit 1. |
| „Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines Vertrags im selben Änderungssatz" | Greift nicht als Pflicht: diese Runde ändert keinen Vertrag. Als Befund gilt weiter, was §7 F festhält — der neue Schritt `S19b` ist ausdrücklich **vertragsfrei** definiert („kein Wire-, State- oder Testvertrag wird berührt"), damit die Modulaufteilung nicht heimlich zur Vertragsrunde wird. |

### 12.6 Nicht erledigt

1. **Die T3-Marke fehlt weiter.** Diese Runde setzt keine Urteilsmarke; die einzige im Manifest
   bleibt `T1 PASS` aus dem Erstdurchgang. Die Wiederprüfung durch einen frischen Codex-Thread
   steht aus.
2. **NAK-130 ist nicht behoben**, sondern nur als Registerzeile geführt — so verlangt es der
   Auftrag dieser Runde.
3. **Der Kanon ist nicht gelaufen**, aus demselben Grund wie im Erstdurchgang (§8): der
   Änderungssatz berührt keinen Produktcode, keinen Test und kein Schema.
4. **`SONDE-003b` bleibt ohne Bilanzspanne** — nicht aus Nachlässigkeit, sondern weil kein
   Commit seinen Namen trägt (§12.2, D14, Punkt 4).

### 12.7 Nachlauf (lebender Kopf, Prüfliste E)

Auf `5f83a2b` folgt genau ein weiterer Commit an dieser Runde, und er bleibt in seiner Grenze
(`docs/PLAN-STAND.md`).

1. **Die vier Arbeitscommits der Runde tragen jetzt ihre SHAs** in der Tabelle in §12.3 —
   vorher stand für den vierten „dieser Commit". Es sind `2a04a27` (Register), `940406e`
   (Plan, Bauaufteilung, Fragen), `008989c` (Entwurf) und `5f83a2b` (Manifest und
   Prozessbilanz) — genau die vier, die der Auftrag vorsah. Daneben stehen die Commits des
   **erzeugten** Blattes `docs/PLAN-STAND.md`, beginnend mit `3ad6e8f`: es wird nach jedem
   Rebase und nach jedem Manifest-Commit einmal neu gerechnet, weil sich sein Quellstand
   dadurch verschiebt, und `CLAUDE.md` verlangt dafür ausdrücklich einen eigenen Commit mit
   Pathspec. Diese Commits enthalten keine Arbeit und keine Behauptung, nur die Ausgabe von
   `tools/plan/planstand.py`.
2. **Zwei Rebases auf `origin/master`.** Die parallele Design-Session hat während dieser Runde
   zweimal gepusht: zuerst 18 Commits (Fünferblöcke 03 bis 05), danach vier weitere (Umbau von
   Block 04/05, zwei Planstandläufe, ein Selbstaudit). Beim zweiten Rebase kollidierte
   `docs/plan/plan.json` in `S31b` — beide Seiten hatten denselben Schritttext geändert. Die
   Auflösung ist **kein Verwerfen**: die fremde Fassung ist vollständig übernommen und die
   eigene Zitatkorrektur aus Befund D5 darauf erneut angewandt worden; ein Riegel im
   Auflösungsskript hat vorher geprüft, dass `S19b` in der fremden Fassung noch steht (40
   Schritte). `docs/PLAN-STAND.md` ist ein erzeugtes Blatt und wurde nach dem Rebase neu
   gerechnet statt zusammengeführt.
3. **Der Planstand rechnet ohne Warnung.** Nach dem Rebase trug das Blatt die Warnung
   „gerechnet aus dem Arbeitsbaum: unter `docs/plan/`, `docs/beweise/` oder `tools/plan/` liegen
   Änderungen, die noch nicht in … sind". Sie ist mit `3ad6e8f` verschwunden; die Zahlen sind
   unverändert (23 von 40 abgenommen, 1 gebaut, 16 offen, Quellstand `5f83a2b`).
4. **Der Arbeitsbaum ist sauber** bis auf das fremde, untrackte
   `.workflow/ultracode/20260831-084621-sonde011-phaseb-bau` — es ist während der ganzen Runde
   unberührt geblieben und steht in keinem der fünf Commits.

## 13. Runde 2 — Ursachenrunde nach Konvergenzentscheid (02.09.2026)

Diese Runde behebt genau einen Defekt und seine Ursache: den Rest von **D8** an den zwei Zeilen
**NAK-85** und **NAK-92**. Kein anderer Befund, kein anderer Pfad.

### 13.1 Die Wiederprüfung der Runde 1

| Feld | Wert |
|---|---|
| Prüfer | Codex `gpt-5.6-sol`, Effort **max**, **lesend** (read-only), frischer Thread |
| Thread | `01a06193-0eb9-7a42-b12d-4fe8a0bb9b38` |
| Geprüfter Stand | `6b2539f219639f451b530d52755c3e1cb0128c1a` (Ende der Runde 1; Ticketbasis `55cdb91`) |
| Prüfauftrag | die vierzehn Defekte D1 bis D14 aus §12.2 — hält jede Nacharbeit an der Quelle? |
| **Urteil** | **NEEDS_WORK** — ein offener Rest, an zwei Zeilen |

**Urteil, wörtlich:**

> URTEIL: NEEDS_WORK — D1–D7 und D9–D14 sind geschlossen; D8 bleibt für NAK-85 und NAK-92 offen.

Der Dirigent hat den Befund an der Quelle bestätigt. Beide Zeilen standen nach Runde 1 mit dem
Nachtrag „**Nachtrag 02.09.2026 (PR2 Runde 1, Codex-Befund D8) — an der Quelle geprueft, bleibt
offen.**“ in der Offen-Tabelle, obwohl das jüngste datierte Urteil ihres Ticketmanifests genau den
dort benannten offenen Rest deckt.

### 13.2 Konvergenzentscheid und Ursache

**Die Ursache in einem Satz:** Runde 1 hat den offenen Rest aus dem **alten Wortlaut der
Registerzeile** abgeleitet („Urteil offen“) statt aus dem **jüngsten datierten Urteil im
Ticketmanifest**. Das ist kein Flüchtigkeitsfehler an zwei Stellen, sondern eine falsche
Quellenrangfolge: die Zeile beschrieb ihren eigenen Stand vom 29.08.2026, das Manifest trug seit
dem 30.08.2026 ein PASS.

**Regel dieser Runde (Konvergenzentscheid des Dirigenten, Skill §3.4).** Für JEDE Zeile der
Offen-Tabelle, die in Runde 1 diesen Nachtrag bekommen hat, wird das jüngste Urteil des
zugehörigen Ticketmanifests an der Quelle gelesen — Urteilsmarken im Kopf, Dirigentenstand-
Abschnitte, Prüferzitate. Deckt ein datiertes PASS oder ein ausdrücklicher Dirigentenstand den
offenen Rest ab, wandert die Zeile **unverändert** (nie löschen) in die Geschlossen-Tabelle und
bekommt einen datierten Nachtrag „Geschlossen 02.09.2026 (PR2 Runde 2): <Manifest>
<Abschnitt/Zeilen>, <Marke>, <Thread>“ mit den SHAs, die die Zeile bereits nennt. Deckt kein Urteil
den Rest ab, bleibt die Zeile offen, und der Nachtrag nennt den Manifestort, an dem das fehlende
Urteil stehen müsste.

**Warum das die Schließregel nicht aufweicht.** `CLAUDE.md`, Arbeitsregeln: „Code und laufende
Beweise sind die Wahrheit … Doku, Memory und Audits sind Hinweise.“ Eine Registerzeile ist Doku;
das datierte Urteil steht im Ticketmanifest. Der Gate-Text bleibt unangetastet — geschlossen wird
weiter nur mit ID und SHA, gelöscht wird nichts, und die Zeile zieht mit ihrem vollen alten Text um.
Was sich ändert, ist allein, **woraus** der offene Rest abgelesen wird.

**Wie viele Zeilen die Regel trifft: genau zwei.** `grep -c "an der Quelle geprueft, bleibt offen"`
über `docs/offene-punkte.md` ergab vor dieser Runde **2** — NAK-85 und NAK-92. Eine dritte Zeile mit
diesem Nachtrag gibt es nicht, die Regel hat also keinen ungeprüften Rest.

### 13.3 Die geprüften Zeilen — Manifestort, Urteil, Ergebnis

| Zeile | Offener Rest laut Runde 1 | Manifestort (an der Quelle gelesen) | Urteil dort | Ergebnis |
|---|---|---|---|---|
| **NAK-85** | „das Urteil eines frischen Prüfers über genau diese A14-Voraussetzungsprüfung ist in dieser Zeile nicht nachgehalten“ | `docs/beweise/SONDE-007a.md` Zeilen 1227–1251 (Prüfer 20b) und 1486–1497 (NAK-100); Marke `T3 PASS 2026-08-30` im Kopf | **PASS**, zweimal, mit A14 ausdrücklich im Prüfumfang | **geschlossen** |
| **NAK-92** | „der frische Prüfer über den Ticketbereich `git diff a7b0740...HEAD` hat diese Arbeit nicht beurteilt“ | `docs/beweise/SONDE-010.md` Zeilen 838–841 (Prüfer Runde 4, genau dieser Bereich) und 1561–1573 (Dirigentenstand); Marke `T2 PASS 2026-08-30` im Kopf | **PASS** für die sechs Befunde dieser Zeile; das NEEDS_WORK galt sieben NEUEN Befunden | **geschlossen** |

**NAK-85 — was an der Quelle steht.** Der Abschnitt „Dirigentenstand — 2026-08-30 12:48 (Sitzung
9f06075a): Prüfer 20b **PASS** — S8 abgenommen“ nennt Codex xhigh, Thread
`01a0523e-12d7-7a61-afbb-10c5fc682e63`, lesend über `git diff dafa5a5...7a87b7d` **einschließlich**
`tools/eq-copilot/pruefe_kern_identitaetsfrei.py` — also genau des Skripts, dessen
Voraussetzungsprüfung NAK-85 betrifft. Wörtlich: „URTEIL: PASS — Geprüft: Scoped Diff, CMake-Genex-Selbsttest
26/26, alle generierten Konfigurationen, Release-Tlog sowie **A14-Scan** mit 17 kernfreien Nadeln,
erfolgreicher Bundle-Gegenprobe und erwarteten Archivmitgliedern; HEAD blieb unverändert.“ Ein
zweites, späteres Urteil deckt denselben Gegenstand nach dem Rückbau: „Dirigentenstand — 2026-08-30
(Sitzung 9f06075a): NAK-100 **PASS**“, Codex high, Thread `01a05263-507e-73d1-a0a9-523996fc1627`,
lesend über `git diff 478e564...dc7ce7d`, wörtlich „Geprüft: … **A14-Messungen und Exit-Semantik** …“.

**NAK-85 — der Fixweg überlebt den Rückbau, an der Quelle nachgemessen (02.09.2026).** NAK-100 hat
`pruefe_kern_identitaetsfrei.py` von 4 197 auf heute 748 Zeilen zurückgebaut; entfernt wurden die
Frischewachen, K1b, der Tlog-Ortsriegel und der JUCE-Baum-Riegel. Der **Weg**, mit dem NAK-85
geschlossen wurde, ist davon nicht betroffen und steht heute im Skript: Schritt `[0] Neubau` löscht
Objekte, Tlogs und Lib des Kernverzeichnisses vor jeder Messung und lässt neu übersetzen und linken
(`tools/eq-copilot/pruefe_kern_identitaetsfrei.py:154` und folgende, Beleg `neubau_belegt` bei `:277`), und
`voraussetzung_exit()` (`:133-144`) gibt ohne registrierten Befund **3**, mit Befund **2**, nie 0.
Damit ist die im Befund verlangte Wirkung („Exit 3 statt grün, wenn die Voraussetzung fehlt“) heute
am Skript messbar — der Rückbau hat sie nicht mitgenommen.

**NAK-92 — was an der Quelle steht.** Die Zeile vermisste ein Urteil „über den Ticketbereich
`git diff a7b0740...HEAD`“. Genau dieser Lauf steht im Manifest: „Prüfer Runde 4: Codex
`gpt-5.6-sol`, Effort `xhigh`, **frischer Thread** `01a04d2b-7926-7da2-b061-1991cd6d3a4b`, lesend
über den vollständigen Ticketbereich `git diff a7b0740...444e125`; HEAD vor und nach dem Lauf
`444e125`“, wörtlich: „Die sechs in §9 genannten NAK-92-Fixes und ihre Bruchproben stimmen mit
Quelle und Rohausgaben überein.“ Sein Gesamturteil war NEEDS_WORK, aber wegen **sieben neuer**
Befunde — die sechs Befunde dieser Zeile hat er ausdrücklich bestätigt. Die sieben neuen sind über
NAK-95, NAK-98 und NAK-104 weitergeführt und stehen selbst in der Geschlossen-Tabelle. Das
abschließende Urteil des Tickets: „S14–15 `SONDE-010` **T2 PASS** — NAK-95, NAK-98 und NAK-104
geschlossen“, darin wörtlich „Die Vorbedingung von `SONDE-011` (NAK-92 → NAK-95) ist damit erfüllt“.

**Keine Zeile bleibt mit fehlendem Urteil zurück.** Beide geprüften Zeilen sind geschlossen; es gibt
in dieser Runde keinen Fall „Urteil fehlt weiterhin, Manifestort benannt“.

### 13.4 Änderungssatz der Runde

Zwei Commits, jeder mit explizitem Pathspec, jeder sofort gepusht.

| # | Commit | Pfade | Inhalt |
|---|---|---|---|
| 1 | `dca7654` | `docs/offene-punkte.md` | NAK-85 und NAK-92 unverändert in die Geschlossen-Tabelle umgezogen, je mit datiertem Nachtrag; Kopfzahlen und Klassentabelle neu gerechnet; der Absatz über die zwei nicht umgezogenen Zeilen ersetzt |
| 2 | `2956f2d`, berichtigt mit `1540a7a`, Nachlauf in diesem Commit | `docs/beweise/PR2.md` | Abschnitt §13, lebender Kopf (Prüfstufen-Zeile, Rundentabelle §1.0); `1540a7a` berichtigt §13.4 und §13.5 über das Verhalten der erzeugten Blätter, an der Quelle nachgemessen; dieser Commit trägt den Nachlauf §13.7 mit den SHAs nach den zwei Rebases |
| — | `06e650f` und der Folgecommit mit demselben Betreffkopf | `docs/PLAN-STAND.md` | kein Arbeitscommit: das erzeugte Blatt, nach jedem Manifestcommit einmal neu gerechnet und mit Pathspec committet (`CLAUDE.md`-Regel) |

**Am Register geändert wurde ausschließlich:** die Klassenzahlen **[Härtung/Struktur]** 18 → 17 und
**[Werkzeug]** 20 → 19, die Gesamtrechnung **64 · 5 · 60** → **62 · 5 · 62** (Summe unverändert 129),
die Überschrift der Offen-Tabelle, ein neuer Runde-2-Absatz in der Triage, der Kopfabsatz der
Geschlossen-Tabelle und die Position der zwei Zeilen. **Der Text der zwei Zeilen selbst ist nicht
angefasst** — maschinell geprüft: der alte Zeilentext von `6b2539f` ist Präfix des neuen, angehängt
sind nur 2 126 (NAK-85) bzw. 1 439 (NAK-92) Zeichen Nachtrag.

**Die zwei erzeugten Blätter tragen keine Arbeit dieser Runde.** Diese Runde hat keine Planquelle
und keine Fragenkarte angefasst. `docs/ANTWORTEN-OFFEN.md` ist nach dem Lauf byteidentisch zu `HEAD`
(24 020 Bytes, vorher wie nachher) und steht deshalb in keinem Commit; das `M` von
`git status --short` daran ist ein reines Zeilenende-Artefakt (`core.autocrlf=true`, das Werkzeug
schreibt LF) ohne Inhaltsunterschied. `docs/PLAN-STAND.md` bewegt **keine Zahl** (23 von 40
abgenommen, 1 gebaut, 16 offen — wie in Runde 1), aber seinen **Quellstand-Zeiger**: er springt mit
jedem Commit unter `docs/beweise/` mit, hier auf den Manifestcommit dieser Runde. Zwei Stufen
gehören dazu, beide in `CLAUDE.md` so vorgesehen: solange das Manifest **uncommittet** liegt, setzt
`planstand.py` pflichtgemäß die Warnung „Gerechnet aus dem Arbeitsbaum" ins Blatt (drei Zeilen);
nach dem Manifestcommit wird ein zweites Mal gerechnet, die Warnung ist weg, und das Blatt geht als
eigener Commit mit Pathspec raus — **kein Arbeitscommit**, nur die Ausgabe des Werkzeugs, genau wie
`3ad6e8f` in Runde 1 (§12.3, letzte Zeile).

**Nicht angefasst:** `.workflow/ultracode/20260831-084621-sonde011-phaseb-bau` (fremd, untrackt) und
jeder Pfad außerhalb der Schreibfläche dieser Runde.

### 13.5 Beweislauf

Befehle, Reihenfolge und Exitcodes, gefahren am 2026-09-02 vom Workspace-Root
`C:/Users/phili/Projekte/Nakama`, nach dem Registercommit `dca7654`. Rohausgabe ungekürzt. Lauf F
misst den Arbeitsbaum und damit auch diesen Abschnitt §13, der erst mit Commit 2 committet wird.

```text
### A) py -3.13 tools/plan/planstand.py
geschrieben: docs\PLAN-STAND.md (23 abgenommen, 1 gebaut, 40 gesamt, aus 054ed59)
EXITCODE=0

### B) py -3.13 tools/plan/antworten_blatt.py
geschrieben: docs\ANTWORTEN-OFFEN.md (53 Antworten, 0 offen, 53 eingearbeitet, 24 mit eigenem Text)
EXITCODE=0

### F) py -3.13 tools/plan/dokuriegel.py <die sieben Ticketpfade>
docs/plan/plan.json: sauber
docs/bauaufteilung-sonden.md: sauber
docs/FL-Nakama-Sonden-Design-Entwurf.md: sauber (1 benannt)
design/docs/funktions-und-bedien-blueprint.md: sauber
CLAUDE.md: sauber
docs/offene-punkte.md: sauber (8 benannt)
docs/beweise/PR2.md: sauber (1 benannt)
docs/FL-Nakama-Sonden-Design-Entwurf.md:189 VERWEIS OHNE ZIEL, aber als historisch markiert („im Design-Repo“ im Umfeld): docs/sondenplan.md
docs/offene-punkte.md:96 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („nicht ins Repo“ im Umfeld): tools/extern/
docs/offene-punkte.md:99 VERWEIS OHNE ZIEL, aber als historisch markiert („im FL-Studio-Repo“ im Umfeld): tools/analyze-track.py
docs/offene-punkte.md:114 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): docs/hub/hub.json
docs/offene-punkte.md:114 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): tools/hub/hub_sync.py holen\|senden
docs/offene-punkte.md:215 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): eq-copilot/fixtures/installer/journale/
docs/offene-punkte.md:215 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): tools/eq-copilot/erzeuge_installer_journale.py
docs/offene-punkte.md:219 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/kern_riegel/
docs/offene-punkte.md:219 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/installer_pruefung/
docs/beweise/PR2.md:942 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): docs/beweise/NAK-121.md
Dokuriegel: 0 Befund(e), 10 benannt (historisch oder Zielpfad)
EXITCODE=0

### I) Registerzeilen: Mengenvergleich der IDs 55cdb91 gegen den Stand dieser Runde
Basis 55cdb91: 126 IDs
HEAD (Runde 2): 129 IDs
ABGAENGE (muss leer sein): (leer)
ZUGAENGE: NAK-128, NAK-129, NAK-130
davon in Runde 2 verschoben: NAK-85, NAK-92 (Offen -> Geschlossen, kein Abgang)
EXITCODE=0
```

**Was die Läufe zeigen.**

1. **A — der Planstand rechnet ohne Warnung.** Zahlen unverändert gegen Runde 1: 23 von 40
   abgenommen, 1 gebaut, 16 offen. Diese Runde hat keine Planquelle angefasst, also darf sich hier
   keine Zahl bewegen — und es bewegt sich keine. Der oben gezeigte Lauf ist der erste dieser Runde
   (Quellstand `054ed59`); nach dem Manifestcommit wird noch einmal gerechnet, dann steht der
   Quellstand auf diesem Commit und das Blatt geht als eigener Commit mit Pathspec raus (§13.4).
   Die Warnung „Gerechnet aus dem Arbeitsbaum" steht im Endstand des Blattes **nicht**.
2. **B — das Antwortenblatt ist unverändert.** 53 Antworten, 0 offen; keine Fragenkarte angefasst.
3. **F — der Dokuriegel ist über alle sieben Ticketpfade sauber.** 0 Befunde. Die benannten Zeilen
   sind dieselben wie in Runde 1 (historische oder noch nicht angelegte Zielpfade); §13 hat keinen
   neuen Verweis ins Leere gebracht.
4. **I — keine ID ist abgegangen.** Gemessen über jede Zeile, die in einer der drei Tabellen mit
   `| NAK-<n> |` oder `| ~~NAK-<n>~~ |` beginnt, `git show 55cdb91:docs/offene-punkte.md` gegen den
   Arbeitsbaum. Die Mengendifferenz ist in der Abgangsrichtung leer; die drei Zugänge sind die in
   Runde 1 angelegten NAK-128, NAK-129 und NAK-130. NAK-85 und NAK-92 sind in beiden Ständen
   enthalten — verschoben, nicht entfernt.

### 13.6 Selbstaudit dieser Runde

Gelesen wurde der Gesamtdiff `6b2539f..HEAD` mit den Fragen der Prüfliste E und F.

1. **Keine gelöschte Zeile.** Lauf I, Abgangsmenge leer; zusätzlich der Präfixtest aus §13.4 an den
   zwei umgezogenen Zeilen.
2. **Kein verändertes Zitat.** Die zwei Zeilen sind nur verlängert. Die in §13 zitierten Prüfersätze
   sind aus `docs/beweise/SONDE-007a.md` und `docs/beweise/SONDE-010.md` übernommen, nicht
   nacherzählt.
3. **Kein fremder Pfad.** Beide Commits nennen ihre Pfade explizit; der Diff `6b2539f..HEAD` enthält
   außerdem den fremden Commit `ff82162` der parallelen Design-Session (visuelle Phase, Figma) — er
   ist nicht von dieser Runde und in keinem ihrer Commits.
4. **Behauptung ≤ Messung.** Jede Zahl in §13 stammt aus einem Lauf oder einem Zitat: 748 Zeilen und
   die zwei Fundstellen im Prüfskript sind an der Quelle gelesen, die Zeichenzahlen der Nachträge
   maschinell gemessen, die Registerzahlen aus der Zählung der Tabellen.
5. **Keine T3-Marke gesetzt.** Der Kopf trägt weiter nur `T1 PASS`; über Runde 2 urteilt ein frischer
   Prüfer.

### 13.7 Nachlauf (lebender Kopf, Prüfliste E)

Nach `06e650f` folgt genau dieser eine Commit an der Runde, und er bleibt in seiner Grenze
(`docs/beweise/PR2.md`), gefolgt vom erzeugten Blatt.

1. **Die Commits der Runde tragen jetzt ihre SHAs** in der Tabelle in §13.4 — vorher stand dort für
   den Manifestcommit ein Stand, den der Rebase überholt hat. Es sind `dca7654` (Register),
   `2956f2d` (Manifest §13), `1540a7a` (Berichtigung §13.4/§13.5) und dieser Commit (Nachlauf).
   Daneben stehen die Commits des **erzeugten** Blattes `docs/PLAN-STAND.md`, beginnend mit
   `06e650f`: es wird nach jedem Manifestcommit einmal neu gerechnet, weil sich sein Quellstand
   dadurch verschiebt, und `CLAUDE.md` verlangt dafür einen eigenen Commit mit Pathspec. Diese
   Commits enthalten keine Arbeit und keine Behauptung, nur die Ausgabe von
   `tools/plan/planstand.py`. Der Text von `06e650f` nennt den Quellstand `bde9edf` — das ist der
   Stand **vor** dem zweiten Rebase, also der Vorgänger von `1540a7a`; inhaltlich derselbe Commit,
   nur mit der SHA vor dem Umschreiben.
2. **Zwei Rebases auf `origin/master`.** Die parallele Design-Session hat auch während dieser Runde
   gepusht: `ff82162` (visuelle Phase, Figma-Skizze), danach `1eb9012` und `b987fb4` (21 Zustands-
   und Panelrahmen, Nachbesserung). Auf `ff82162` ist vor dem Registercommit **ff-only** gepullt
   worden; die zwei anderen sind mit `git pull --rebase` bei sauberem Arbeitsbaum übernommen worden.
   **Kein Konflikt** — die Design-Session und diese Runde teilen keinen Pfad. Kein fremder Pfad ist
   angefasst, kein fremder Commit verändert.
3. **Der Planstand rechnet ohne Warnung.** Der Endstand des Blattes trägt keine Zeile „Gerechnet aus
   dem Arbeitsbaum" (`grep -c` = 0); die Zahlen sind unverändert (23 von 40 abgenommen, 1 gebaut,
   16 offen).
4. **Der Arbeitsbaum ist sauber** bis auf das fremde, untrackte
   `.workflow/ultracode/20260831-084621-sonde011-phaseb-bau` — es ist während der ganzen Runde
   unberührt geblieben und steht in keinem Commit dieser Runde.

---

## 14. Abschluss des Dirigenten (02.09.2026)

**Urteil: T3 PASS.** Gesetzt vom Dirigenten (Fable 5.1, Effort xhigh, Session `19819d5c`) am
02.09.2026 auf dem End-Stand `e51a7d3e309741102deeacc9db165bb6f1a44c36`; Ticketbasis
`55cdb91599239aa99ec1322e23d109ab9424d7b2`. Zwei Nacharbeitsrunden von drei im Budget.

### 14.1 Prüferkette

| Lauf | Prüfbereich | Prüfer | Thread | Urteil |
|---|---|---|---|---|
| Erstprüfung | `55cdb91...16be249`, 13 Ticketpfade (Vorlage A) | Codex `gpt-5.6-sol`, max, lesend | `01a060fd-b78e-7d90-aee6-591dab8dd943` | NEEDS_WORK — 13 Prüferbefunde (1 P1, 12 P2) plus Dirigentenbefund D1, zusammen D1–D14 (§12.1) |
| Wiederprüfung 1 | Fixdiff `16be249...6b2539f`, sieben Worker-Commits (Vorlage B) | Codex `gpt-5.6-sol`, max, lesend | `01a06193-0eb9-7a42-b12d-4fe8a0bb9b38` | NEEDS_WORK — D8 offen für NAK-85 und NAK-92 (§13.1) |
| Wiederprüfung 2 | Fixdiff `6b2539f...e51a7d3`, sechs Worker-Commits (Vorlage B) | Codex `gpt-5.6-sol`, max, lesend | `01a061b7-6c47-7201-834f-35c5444c3344` | **PASS** |

Urteil der Wiederprüfung 2, wörtlich:

> URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen. Geprüft: NAK-85 und NAK-92 stehen einmalig in der Geschlossen-Tabelle (docs/offene-punkte.md:221–222), mit bestätigten Manifestorten, PASS-Marken und Threads; ID-Menge, Zitate, Gate-Text, Planstand-Dry-run, sieben Dokuriegel, Worker-Pfadgrenze und unverändertes HEAD bestanden. Nicht geprüft wurden die ausgeschlossenen Produktcode-, Kanon- und Fremdcommit-Flächen; Python 3.13 war nicht startbar, daher lief derselbe planstand.py-Hauptpfad schreibfrei unter Python 3.14 byteidentisch und warnungsfrei.

HEAD war vor und nach jedem der drei Läufe identisch (Logdateien `nakama-<sha>-review.log`).

### 14.2 Konvergenz und Rundenbilanz

`py -3.13 tools/dirigent/rundenbilanz.py --runden 55cdb91 16be249 6b2539f e51a7d3`:

```text
55cdb91..16be249: Pruefwerkzeug 2 Datei(en) +359/-3 | Doku 11 Datei(en) +5734/-273
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
16be249..6b2539f: Pruefwerkzeug 1 Datei(en) +17/-5 | Doku 36 Datei(en) +3606/-464
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
6b2539f..e51a7d3: Doku 33 Datei(en) +620/-27
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 3 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill §3.4, keine weitere Punktkorrektur-Runde.
EXITCODE=0
```

Das Konvergenzsignal ist bei einer Planprüfung erwartbar: der Gate-Text schließt Produktcode,
Tests und Schemas ausdrücklich aus, Produkt + Tests sind deshalb in jeder Runde 0 Zeilen. Der
Konvergenzentscheid nach §3.4 wurde nach der Wiederprüfung 1 gefällt (§13.2); Runde 2 war die
Ursachenrunde, eine weitere Punktkorrektur-Runde gab es nicht. Die Rundenbilanz zählt in den
Runden 1 und 2 auch die Fremdcommits mit (unten).

### 14.3 Fremdcommits zwischen den Runden

Zwischen `16be249` und `e51a7d3` liegen 26 Commits einer parallelen Design-Session (Autor
„phil l", Fünferblöcke 03–05, Antwort U21, S31b-Punkte 18/19, visuelle Phase in Figma, CLAUDE.md,
`tools/plan/dokuriegel.py`); die Worker haben ihre Commits zweimal darauf rebased (§12.7, §13.7).
Die Prüfer hatten die Fremdcommits ausdrücklich ausgeschlossen und nur die Worker-Commits gelesen.
Vom Dirigenten am 02.09.2026 nachgemessen: die PR2-Inhalte haben die Fremdcommits überlebt
(Rangfolge in `CLAUDE.md` und Blueprint, S31b „QUELLE UMGESTELLT 02.09.2026", S19b, Entwurf 0.5).
Kein Halt nach Skill §3.3, weil die Fremdcommits aus einer eigenen Arbeitskopie kamen, gepusht
waren und keine PR2-Datei rückgängig gemacht haben.

### 14.4 Beweise des Abschlusses (vom Dirigenten gefahren auf `e51a7d3`)

Dokuriegel über die sieben Ticketpfade:

```text
docs/plan/plan.json: sauber
docs/bauaufteilung-sonden.md: sauber
docs/FL-Nakama-Sonden-Design-Entwurf.md: sauber (1 benannt)
design/docs/funktions-und-bedien-blueprint.md: sauber
CLAUDE.md: sauber
docs/offene-punkte.md: sauber (8 benannt)
docs/beweise/PR2.md: sauber (1 benannt)
docs/FL-Nakama-Sonden-Design-Entwurf.md:189 VERWEIS OHNE ZIEL, aber als historisch markiert („im Design-Repo“ im Umfeld): docs/sondenplan.md
docs/offene-punkte.md:96 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („nicht ins Repo“ im Umfeld): tools/extern/
docs/offene-punkte.md:99 VERWEIS OHNE ZIEL, aber als historisch markiert („im FL-Studio-Repo“ im Umfeld): tools/analyze-track.py
docs/offene-punkte.md:114 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): docs/hub/hub.json
docs/offene-punkte.md:114 VERWEIS OHNE ZIEL, aber als historisch markiert („abgeschafft“ im Umfeld): tools/hub/hub_sync.py holen\|senden
docs/offene-punkte.md:215 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): eq-copilot/fixtures/installer/journale/
docs/offene-punkte.md:215 VERWEIS OHNE ZIEL, aber als historisch markiert („entfernt“ im Umfeld): tools/eq-copilot/erzeuge_installer_journale.py
docs/offene-punkte.md:219 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/kern_riegel/
docs/offene-punkte.md:219 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): tools/eq-copilot/installer_pruefung/
docs/beweise/PR2.md:942 VERWEIS OHNE ZIEL, aber als noch nicht angelegter Zielpfad markiert („noch nicht angelegt“ im Umfeld): docs/beweise/NAK-121.md
Dokuriegel: 0 Befund(e), 10 benannt (historisch oder Zielpfad)
EXITCODE=0
```

Registerzeilen-IDs, Basis gegen End-Stand:

```text
Zeilen-IDs nur in 55cdb91 (Abgaenge):
(Ende der Liste; leer = keine Abgaenge)
Zeilen 55cdb91: 126 · Zeilen e51a7d3: 129
```

Planstand nach dem Setzen der Marke:

```text
geschrieben: docs\PLAN-STAND.md (24 abgenommen, 0 gebaut, 40 gesamt, aus 3a194b8)
```

### 14.5 Technische Folgen aus §6 — Entscheid des Dirigenten

| Vorschlag | Entscheid | Ausführung |
|---|---|---|
| V-1 Rundenbudget an den Prüfwerkzeug-Anteil koppeln | angenommen als Werkzeugpunkt | Registerzeile NAK-131 (Klasse Werkzeug), Umsetzung mit dem nächsten Anfassen von `rundenbilanz.py` |
| V-2 `.workflow/` in `.gitignore` | umgesetzt | `.gitignore` trägt `.workflow/`; der Ordner `.workflow/ultracode/20260831-084621-sonde011-phaseb-bau` (2 375 Dateien, 76 MB, nichts Einmaliges — §5.1) ist am 02.09.2026 gelöscht; NAK-128 geschlossen |
| V-3 NAK-97 mit dem nächsten `planstand.py`-Anfassen schließen | bleibt Registerzeile NAK-97 | keine Änderung |
| V-4 Dokuriegel in den Abschluss hängen | umgesetzt | `CLAUDE.md` Arbeitsregel (Commit `5cfc023`, parallele Session 02.09.2026) und Skill `dirigent` §3.5 (dieser Abschluss-Commit) |
| V-5 Werkzeugticket für `stash@{0}` oder bewusst verwerfen | Stash bleibt liegen, kein Drop | NAK-129 bleibt offen (Klasse Werkzeug); der Stash trägt Arbeit, die nirgends sonst liegt (§5.2) |

### 14.6 Worker und Aufsicht

| Rolle | Worker | Modell | Aufsicht | Stand |
|---|---|---|---|---|
| Bau | `528b49d1` `nakama-pr2-55cdb91-bau` | Opus, max, Auto-Modus | NORMAL | `55cdb91` → `16be249`, 8 Commits |
| Nacharbeit 1 | `b9042135` `nakama-pr2-16be249-nach1` | Opus, max, Auto-Modus | NORMAL | `16be249` → `6b2539f`, 7 Commits |
| Nacharbeit 2 (Ursachenrunde) | `4217a904` `nakama-pr2-6b2539f-nach2` | Opus, max, Auto-Modus | LOCKER | `6b2539f` → `e51a7d3`, 6 Commits |

Alle drei im sichtbaren Checkout, je mit Ereignisbeobachter (`cockpit.ps1 -WatchWorker`) und
stündlichem Kontrollloop; nach dem Abschluss sind Worker, Beobachter und Loops abgeräumt.
