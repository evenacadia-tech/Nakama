# SONDE-014 — S23–25: Absicht, Ursachenhypothese, Vorschlag und Assistentenschritt

| Merkmal | Wert |
|---|---|
| Ticket | S23–25, `SONDE-014` (Phase P4–P5), Leitungsname „Aus Messungen belegte Befunde und kleinste Tests ableiten" |
| Phase | **Etappe 2 — Bau, läuft.** Fertig: **Etappe A** (`SourceIntent` im Main-State, Commit `8f030f5`), **Etappe B** (Fassung 3 des Wire-Envelopes, Commits `edea7a9`, `baa6291`, `581431a`) **Etappe C** (Evidenzgraph und `CauseHypothesis`), **Etappe D** (Befundzustände), **Etappe E** (Maskierungs-Datenweg), **Etappe F** (Proposal-Policy) und **Etappe G** (`AssistantStep` im Main). Davor ohne Produktcode: **Matrixnacharbeit 1** (2026-09-06), **Etappe 1b — Entscheide E-01 bis E-10** (`fdb04e4`) und **Etappe 1 — Verhaltensmatrix** (`1f126a4`). Der Bauverlauf steht in §7. |
| Matrixprüfung 2 | Codex `gpt-6-astra`, Effort **max**, lesend, Thread `01a077a3-1411-7830-9bfd-e17d233baab1`; `HEAD` vor und nach dem Lauf `f90abf5`. **URTEIL: PASS** — D1 bis D4 geschlossen, nichts gebrochen. Auftrag `docs/beweise/roh/SONDE-014-matrixpruefung-2-auftrag.txt`, Rohurteil `docs/beweise/roh/SONDE-014-matrixpruefung-2-f90abf5.txt`. **Etappe 1 ist damit abgenommen; §3 ist ab hier die Spezifikation.** |
| Etappe 2 | Bauauftrag `docs/beweise/roh/SONDE-014-etappe-2-auftrag.txt`, Basis `f90abf5`; Fortsetzung 1 (Etappen C bis I) `docs/beweise/roh/SONDE-014-etappe-2-fortsetzung-1-auftrag.txt`, Startstand `6b96c64`, mit **Entscheid E-12** (`GATE_MINDEST_FENSTER` in Etappe C ohne Versionsschritt; der eine erlaubte Schritt der `metrics_version` liegt in Etappe H). Neun Bauetappen A bis I nach §5.1; Bauverlauf, gemessene Matrixzeilen, Rotbeweise, Abweichungen und Nebenbefunde in **§7**. |
| Entscheide | E-01 bis E-10 wörtlich in `docs/beweise/roh/SONDE-014-etappe-1-entscheid-auftrag.txt`. Eingearbeitet in Etappe 1b: neun angenommene Technikentscheide (E-01 bis E-09, E-01 und E-03 mit Präzisierung, E-08 mit Autoritätenzuweisung) und **eine neue Lücke E-10** (Transport des Intents vom Main zum Broker) mit den zwei zusätzlichen Matrixzeilen **M-85** und **M-86**. **Seit der Matrixnacharbeit 1 kommt E-11 dazu** (Transport und versionierter Spiegel des `AssistantStep`, Regel R3, §4.13) mit **M-88** und **M-89**. Je Entscheid steht ein Block „Etappe 1b (Entscheid des Dirigenten, 06.09.2026)" unter dem zugehörigen §4-Abschnitt; der vorige Vorschlagstext bleibt als Historie stehen. |
| Urteil | **offen** — weder Bau- noch Prüfurteil. Dieses Manifest friert die aus den verbindlichen Quellen belegbare Verhaltensgrenze ein und benennt die Stellen, an denen der spätere Bau ohne weiteren Vertragsentscheid nicht ehrlich fortfahren kann. |
| Prüfstufe | T1+T2 gefordert (`docs/bauaufteilung-sonden.md` Zeile 394 · `docs/plan/plan.json`, Schritt S23–25, `"stufe": "T2"`). Keine Marke vergeben. |
| Prüfmodell | Codex `gpt-6-astra`, Effort **max**, lesend — für Matrix-, Erst-, Wieder- und Abschlussprüfung. `gpt-5.6-sol` max **nur** als Gegenprüfer (User-Wort 05.09.2026, `feedback_astra-prueft-sol-gegenprueft`). |
| Matrixprüfung 1 | Codex `gpt-6-astra`, Effort **max**, lesend, Thread `01a0776e-ed49-7fd2-afaa-992997f6bf3e`; `HEAD` vor und nach der Prüfung `fdb04e4`. Urteil **NEEDS_WORK**, vier Defekte (D1 M-23, D2 M-65, D3 M-71/E-08, D4 Ausschlussgründe), alle an der Quelle bestätigt. Rohurteil `docs/beweise/roh/SONDE-014-matrixpruefung-1-fdb04e4.txt`, Auftrag `docs/beweise/roh/SONDE-014-matrixpruefung-1-auftrag.txt`; Wortlaut, Regel und geänderte Zeilen in §4.13. |
| Matrixnacharbeit 1 | Regeln R1 bis R4 wörtlich in `docs/beweise/roh/SONDE-014-matrixnacharbeit-1-auftrag.txt`. Eingearbeitet am 06.09.2026: **M-23**, **M-65** und **M-71** nachgezogen, drei neue Zeilen **M-87** (Ausschlussgründe, R4), **M-88** und **M-89** (Assistentenschritt, R3 → Entscheid **E-11**). §3.11, §5.1, §5.2, §5.4 und §6 nachgezogen. |
| Basis-SHA | `5e99a1361696be84a7174575ec7b9ad9e27e6a7f`, mit `git rev-parse HEAD` beim Start gemessen; Worktree bis auf die Auftragsdatei sauber (`git status --short`: eine Zeile `?? docs/beweise/roh/SONDE-014-etappe-1-auftrag.txt`). |
| Kanon vorher | **GRÜN 54/54 auf `aca0f28`**, Rohausgabe `docs/beweise/roh/NAK-182-aca0f28.md`. Eigene Messung in dieser Sitzung: `git diff --stat aca0f28..5e99a13 -- broker eq-copilot tools` ist **leer**. Der Codestand von `5e99a13` ist mit dem beglaubigten Kanonstand identisch; die Commits dazwischen berühren nur `docs/` und `design/`. |
| Änderungssatz dieser Etappe | **Matrixnacharbeit 1:** dieses Manifest und die drei unveränderten Rohdateien `docs/beweise/roh/SONDE-014-matrixpruefung-1-auftrag.txt`, `docs/beweise/roh/SONDE-014-matrixpruefung-1-fdb04e4.txt` und `docs/beweise/roh/SONDE-014-matrixnacharbeit-1-auftrag.txt`. **Etappe 1b:** dieses Manifest `docs/beweise/SONDE-014.md` und der unveränderte Entscheidauftrag `docs/beweise/roh/SONDE-014-etappe-1-entscheid-auftrag.txt`. **Etappe 1** (`1f126a4`): dieses Manifest und der unveränderte Auftrag `docs/beweise/roh/SONDE-014-etappe-1-auftrag.txt`. In beiden: kein Produkt-, Test-, Schema-, Fixture- oder Werkzeugcode; kein Eingriff in `docs/plan/`, `docs/offene-punkte.md`, `docs/PLAN-STAND.md`, `docs/NEXT-SESSION.md` oder `design/`. |
| Grenze | Etappe 2 baut ausschließlich, was §3 zusagt. Technikfragen ohne Quelle standen in §4 als ENTSCHEIDEN-VORSCHLAG und sind seit Etappe 1b als **ENTSCHIEDEN** geführt; Produktfragen werden benannt und einer Fläche zugeordnet, nie beantwortet. |
| Ticketpfade (Prüfbereich der Etappe 2) | Vollständig in §5.2. Kurzform: `broker/src/coordinator/` (neu: `intent.rs`, `hypothese.rs`, `proposal.rs`, `assistent.rs`, `maskierung.rs` — anzulegen), `broker/src/store/writer.rs`, `broker/src/dto.rs`, `broker/src/protokoll.rs`, `broker/src/vertrag.rs`, `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`, `eq-copilot/schemas/v3/README.md`, `eq-copilot/schemas/state/nakama-state-v2.md`, `eq-copilot/plugin/state/`, `eq-copilot/plugin/src/`, `eq-copilot/plugin/core/ipc/`, `eq-copilot/fixtures/`, `tools/eq-copilot/`, `tools/beweise.ps1`. Seit Etappe 1b enthalten, weil die **Fassung 3** des Wire-Envelopes (E-04, E-05, E-09, E-10) beide Vertragshälften anfasst. |
| Belegfeld und Urteilsmarke | **Bewusst nicht gesetzt.** Diese Etappe trägt keine `NAKAMA-URTEIL`-Marke und schreibt kein `beleg`-Feld nach `docs/plan/plan.json`; beides setzt erst Etappe 2 beziehungsweise der Abschluss. Sonst zeigte der gerechnete Planstand ein Spezifikationsmanifest als „gebaut". |

**Alles unter diesem Kopf ist append-only Verlauf.** Der lebende Kopf wird beim
Abschluss nachgezogen; Positionen im Kopf stehen als Symbol oder Anker, im
Verlauf gilt eine Zeilennummer zum Stand des Abschnitts, der sie trägt
(`tools/dirigent/pruefliste.md`, Abschnitt E).

**Die Matrix-IDs `M-01` bis `M-84` dieses Manifests sind eigen.** Sie sind
nicht die `M-01` bis `M-86` aus `docs/beweise/SONDE-013.md`; wo eine Zeile eine
SONDE-013-Zusage fortführt, nennt die Quellspalte sie ausdrücklich als
`SONDE-013 M-nn`.

**Warum Etappe 1 überhaupt.** Das Ticket berührt dieselben drei teuren Klassen
wie SONDE-013. **Verträge:** `SourceIntent` und `AssistantStep` gehören nach
Entwurf §33.5 in den Main-State, `Finding`, `CauseHypothesis` und `Proposal`
werden neue persistente oder übertragene Objekte, und der laufende
Maskierungswert aus U16 braucht möglicherweise ein eigenes v3-Feld —
`evidence_snapshot` und `session_snapshot` sind beide
`additionalProperties: false`. **Lebenszyklen:** ein `AssistantStep` mit
Abbruch, Zurück, Überspringen, Resume und Rekonstruktion nach Neustart, die
deterministische Invalidierung von Hypothesen bei Evidenzentfernung und das
Stale-Werden bei Intent-Änderung sind Gegenpfade, die im Nachhinein teuer sind.
**Nebenläufigkeit mit Kadenz:** der Maskierungswert muss die Anzeigekadenz
halten, ohne je den Audio-Thread zu berühren. Nach der Dirigentenregel
„Spezifikation vor Code" entsteht deshalb zuerst die Verhaltensmatrix; ein
lesender Codex-Thread prüft nur sie gegen Entwurf und Gate-Text, erst danach
baut ein frischer Worker in Etappe 2.

---

## 1. Gate-Text, wörtlich

### 1.1 Entwurf §59 „P5 — Intent, Ursachen, Proposal und passiver Assistent"

`docs/FL-Nakama-Sonden-Design-Entwurf.md`, Abschnitt 59 (Zeilen 4304–4325 zum
Stand `5e99a13`), vollständig:

> ## 59. P5 — Intent, Ursachen, Proposal und passiver Assistent
>
> **Zweck:** Aus Features entsteht eine begründete nächste Handlung, noch ohne Fernänderung.
>
> **Lieferumfang:**
>
> - `SourceIntent` mit Funktion, Front/Middle/Back, Schutz und gerichteter Priorität im Main-State;
> - versionierter Evidenzgraph, `CauseHypothesis`, Alternativen, Ausschlussgründe und Konfidenz;
> - deterministische Policy für den kleinsten `Proposal`, einschließlich `keine Änderung` und
>   `mehr Daten`;
> - persistenter `AssistantStep` mit Abbruch, Zurück, Überspringen und Resume;
> - manueller Busvorschlag und manueller Experimentabschluss;
> - Evaluationskorpus mit Precision/Recall, Kalibrierung, Brier Score, Coverage und Enthaltung;
> - ~~optionale KI nur als Renderer validierter Fakten, nie als Aktions- oder Zahlenquelle.~~
>   **Gestrichen in Fassung 0.5 am 02.09.2026** — der Advisor ist regelbasiert, es gibt keine
>   KI-Schicht im Lieferumfang. Entscheidquelle: **Erratum (e)**, User-Wort 21.08.2026 „Nein –
>   raus aus dem Produkt".
>
> **Exit-Gate:** Jede sichtbare Behauptung referenziert existente Evidenz-IDs, jedes Proposal nennt
> Ziel, Passage, Grenzen, Hörziel, Stopbedingung und Rückweg. Gegenbeispiele erzwingen nachweislich
> Enthaltung. `R2 · Passive Beta` ist erreicht.

Der Lieferumfang führt **sechs** lebende Punkte und einen gestrichenen. Der
gestrichene Punkt ist keine Option, die später wiederkommt: er ist mit
Erratum (e) und User-Wort dauerhaft aus dem Produkt genommen, und
`CLAUDE.md` führt „Keine KI-Erklärschicht im Produkt" als Wahrheitskern.

### 1.2 Tickettabelle §65, Zeile 4486

`docs/FL-Nakama-Sonden-Design-Entwurf.md`, Zeile 4486, wörtlich:

> \| `SONDE-014` \| Intent, CauseHypothesis, Proposal und AssistantStep \| jede Aussage evidenzgebunden; R2 erreicht \|

### 1.3 Bauaufteilung: Ticketzeile 394 und der G5-Absatz

`docs/bauaufteilung-sonden.md`, Zeile 394 (Tabelle „P4 · Vergleichsevidenz →
P5 · Ursachen → R2"), wörtlich und ungekürzt:

> \| S23–25 \| `SONDE-014` \| Intent, CauseHypothesis, Proposal, AssistantStep + Evaluationskorpus — **dazu seit Antwort U16 (30.08.2026, „Ja, erste Fassung")** der Datenweg für die Überdeckungsanzeige: die Überdeckung als laufender, je Frequenzbereich auflösbarer Wert mit Anzeigekadenz, nicht nur als Befundsatz. Braucht sie ein eigenes v3-Feld, gehört der Name in die Vertragsrunde von S16–17 (liegt vor R0). Die farbige Zone selbst ist S31b. **Seit PR2 (02.09.2026)** hat die Zone auf Gen Fläche 1 einen abgenommenen Ort — Quelle `design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md`, Abschnitte „Stabile Geometrie" und „Graphreaktion" (Quelle nachgetragen am 02.09.2026, PR2 Runde 1, Codex-Befund D13): das gewählte Finding markiert seinen Frequenzbereich im **stabilen** Gesamtgraph, ohne dessen Größe, Achsen oder Ausschnitt zu ändern; ein Zoom ist nur bewusst und reversibel zulässig. Der laufende Wert muss also einen Frequenzbereich benennen und die Anzeigekadenz halten. **Seit Antwort U22 (06.09.2026, „Fünf (Empfohlen)")** hat eine Quelle genau fünf Rollen (führt, trägt, begleitet, geschützt, bewusst verschmolzen); Transientenschutz und Raum-Verschmelzung sind Schutzangabe bzw. Verschmelzungs-Erlaubnis an der Quelle, keine Rollen — die Abbildung auf `SourceIntent` (Entwurf §37.1) ist Technik dieses Tickets und steht in seiner Verhaltensmatrix; Wortlaut `design/abnahmen/2026-09-06-fragenrunde-rollen-u22.md` \| T1+T2 \|

Die Gate-Zeile G5 derselben Tabelle und der Absatz direkt darunter, wörtlich:

> \| **G5** \| — \| **Gate:** Codex + adversariale Gegenbeispiele. Falsifikation: Gate 6, 7, 8 \| **T3** \|
>
> G5 ist das ungewöhnlichste Gate: Der Prüfer soll **eine falsche starke
> Ursachenbehauptung provozieren** — korrelierter Distraktor, Parent-Duplikat,
> verschobene Passage. Enthaltung („mehr Daten nötig") ist das gewünschte
> Ergebnis, nicht ein Treffer.
>
> **R2 · Passive Beta** — 9 von 12 Kernfunktionen, Audio vollständig passiv.
> **Das ist der Punkt, an dem das Produkt zum ersten Mal wirklich nützt.**

### 1.4 `docs/plan/plan.json`, Schritt S23–25, mit allen vier Nachträgen

Wörtlich aus dem Feld `text` (gelesen mit `py -3.13`, UTF-8; `"stufe": "T2"`,
`"beleg": ""`):

> Absicht, Ursachenhypothese, Vorschlag, Assistentenschritt — mit Prüfkorpus. NACHTRAG 30.08.2026 (Antwort U16, Wahl „Ja, erste Fassung (Empfohlen)"): Der Befund über eine Überdeckung zweier Quellen bleibt nicht bei Worten — er wird zusätzlich im Kurvenbild gezeigt, und zwar in der ersten nützlichen Fassung, nicht als späterer Ausbau. Dieses Ticket liefert dafür den DATENWEG: die Überdeckung muss als laufender, je Frequenzbereich auflösbarer Wert aus dem Befund herausfallen und mit der Anzeigekadenz mithalten, nicht nur als fertiger Satz mit Ort und Beleg. Zwei Dinge sind dabei bewusst getrennt: der Befundtext (mit Ort, Beleg und kleinstem Gegentest) bleibt unverändert Lieferumfang dieses Tickets; die farbige Zone auf der Kurve ist Anzeige und gehört zu S31b. Ob der laufende Wert ein eigenes Feld im v3-Vertrag braucht, entscheidet der Datenweg an der Quelle — fällt die Entscheidung auf ja, gehört der NAME in die v3-Vertragsrunde von S16–17, weil die vor R0 liegt und der Vertragsanteil, der später bricht, der Name ist. Wortlaut: design/abnahmen/2026-08-30-fragenrunde-marktstandard.md.
>
> NACHTRAG 02.09.2026 (PR2, Design-Strang): Die Ueberdeckungszone aus Antwort U16 hat auf Gen Flaeche 1 einen abgenommenen Ort. Die Findings-Flaeche behaelt ihre feste Hoehe, und das ausgewaehlte Finding markiert seinen Frequenzbereich im stabilen Gesamtgraph, ohne dessen Groesse, Achsen oder Ausschnitt zu veraendern (design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md, 'Stabile Geometrie' und 'Graphreaktion'). Fuer diesen Schritt heisst das: der laufende, je Frequenzbereich aufloesbare Ueberdeckungswert muss genau diese Markierung tragen koennen - also mit der Anzeigekadenz mithalten und einen Frequenzbereich benennen, nicht nur einen Satz. Ein Zoom auf den markierten Bereich ist nur als bewusste und reversible Aktion zulaessig und darf nicht aus dem Datenweg heraus ausgeloest werden. Die farbige Zone auf der EQ-Flaeche bleibt S31b.
>
> NACHTRAG 02.09.2026 (Antwort U21, Wahl „Nein, sie stecken schon drin (Empfohlen)", Fünferblock 03 Entscheidung 1/5): Der Ursachenbefund behält im Datenmodell seine sechs Teile (Ort, Beobachtung, Zusammenhang, Alternativen, Sicherheit, nächster Beweisschritt; Entwurf Abschnitt 8 „Ergebnisform"), aber Gen Fläche 1 zeigt davon genau drei Zeilen: wahrscheinliche Ursache, kleinster Test, worauf hören. Leitsatz des Users: „wenn wir 6 werte für eine messung anzeigen, ist das schlichtweg schlechtes design". Für diesen Schritt heißt das: (1) SICHERHEIT wird nicht als Wert ausgegeben, sondern auf den Zustand des Befunds abgebildet — nur ein sicherer Befund ist handelbar (READY TO SEND, Audition und Draft erlaubt); ein unsicherer Befund meldet MORE DATA, ein veralteter STALE, beide ohne Draft. Die Schwelle, ab der hoch/mittel/unklar als handelbar gilt, ist Technik dieses Schritts und wird mit dem Prüfkorpus kalibriert, nie in der UI erraten. (2) ALTERNATIVEN erscheinen nicht als Feld im fokussierten Befund, sondern als eigene Befunde in der Liste mit eigenem Zustand; der Advisor liefert einen zweiten möglichen Verursacher also als weiteren Befund, nicht als Anhang. (3) BELEG ist die markierte Frequenzzone mit Ziel- und Referenzkurve im Graphen (siehe die Markierung oben), nicht ein wiederholter Text. Wortlaut: design/abnahmen/2026-09-02-technische-ui-architektur-fuenferblock-03.md.
>
> NACHTRAG 06.09.2026 (Antwort U22, Wahl „Fünf (Empfohlen)"): Eine Quelle kann genau fünf musikalische Rollen bekommen — führt, trägt, begleitet, geschützt, bewusst verschmolzen. Impuls und Raum sind keine Rollen: kurze Anschläge werden über die Schutzangabe an der Quelle geschützt, Hall/Atmosphäre/Breite über die Verschmelzungs-Erlaubnis; der Advisor berücksichtigt beides aus diesen Angaben. Für diesen Schritt heißt das: das Intent-Modell (Entwurf §37.1) trägt genau fünf Rollenwerte als Produktunterscheidung; die Abbildung auf Vordergrund/Mitte/Hintergrund, Funktionstags, Schutz und gerichtete Beziehungen ist Technik dieses Schritts und steht in seiner Verhaltensmatrix. Damit ist vor S23–25 keine Produktfrage mehr offen. Wortlaut: design/abnahmen/2026-09-06-fragenrunde-rollen-u22.md.

### 1.5 Abnahme U16 (30.08.2026) — Folge, wörtlich

`design/abnahmen/2026-08-30-fragenrunde-marktstandard.md`, Abschnitt „U16 —
Überdeckungen direkt im Kurvenbild":

> **Antwort (Auswahl):** „Ja, erste Fassung (Empfohlen)" — die farbige
> Überdeckungszone kommt mit der EQ-Fläche in die erste nützliche Fassung.
>
> **Folge:** Die Überdeckung wird nicht nur gemeldet, sondern gezeigt. Sie ist
> Lieferumfang der ersten nützlichen Fassung der EQ-Fläche, kein Ausbau. Der
> Datenweg liegt bei S23–25 (die Überdeckung als laufender, je Frequenzbereich
> auflösbarer Wert mit Anzeigekadenz — der Befundtext bleibt unverändert
> daneben), die farbige Zone selbst bei S31b. Die Zone zeigt nur, was der Befund
> belegt hat, und setzt keine eigene Schwelle: sie ist keine zweite Wahrheit
> neben dem Befundtext. Zeigt die Bildvorlage vom 25.08. keine solche Zone, wird
> das als Lücke einzeln vorgelegt und nicht erfunden.

### 1.6 Abnahme U21 (02.09.2026, Fünferblock 03, Entscheidung 1/5) — Folge, wörtlich

`design/abnahmen/2026-09-02-technische-ui-architektur-fuenferblock-03.md`,
Abschnitt „1/5 — Sicherheit, Beleg und Alternativen stecken schon drin (U21)":

> Auf die neu gestellte Frage, ob Sicherheit, Beleg und Alternativen auf Gen
> Fläche 1 überhaupt als eigene Angaben zu den drei Zeilen (wahrscheinliche
> Ursache, kleinster Test, worauf hören) dazukommen, wählt der User am
> 02.09.2026 um 09:35 Uhr:
>
> > „Nein, sie stecken schon drin (Empfohlen)"
>
> Damit ist für Gen Fläche 1 gesammelt:
>
> - Die Findings-Fläche bleibt bei genau drei Zeilen je Befund: `LIKELY
>   CAUSE`, `SMALLEST TEST`, `LISTEN FOR`. Es kommt keine vierte Zeile, kein
>   Umschalter und kein Panel für Beleg, Sicherheit oder Alternativen dazu.
> - **Sicherheit** ist der Zustand des Befunds, nicht ein eigener Wert: Nur ein
>   sicherer Befund steht auf `READY TO SEND` und bietet `HOLD TO AUDITION`
>   und `SEND DRAFT → EQ` an. Ein unsicherer Befund meldet `MORE DATA`, ein
>   veralteter `STALE`; beide bieten keinen Draft an. Die Stufen hoch,
>   mittel, unklar aus dem Entwurf werden im Datenweg auf diese Zustände
>   abgebildet; die Abbildung ist Technik von S23–25.
> - **Beleg** ist die markierte Zone im Frequenzgraph mit den beiden Kurven
>   von Ziel und Referenz. Er wird nicht zusätzlich als Text wiederholt.
> - **Alternativen** sind weitere Befunde in der Liste (`ALL FINDINGS`), nicht
>   ein Feld im fokussierten Befund. Ein zweiter möglicher Verursacher wird
>   damit als eigener Befund geführt, mit eigenem Zustand.
> - Das Wort `CONFIDENCE` unter dem Graphen bleibt die Messqualität der
>   Passage (Blueprint Abschnitt 4.1, Zone 3), nicht die Sicherheit des
>   Befunds; beide dürfen nicht vermischt werden.

Und aus der Vorbemerkung desselben Abschnitts, ebenfalls bindend:

> - Die sechs Teile eines Ursachenbefunds aus dem Entwurf (Ort, Beobachtung,
>   Zusammenhang, Alternativen, Sicherheit, nächster Beweisschritt; Entwurf
>   Abschnitt 8, „Ergebnisform") sind das Datenmodell und die Regelbasis des
>   Advisors. Sie sind **keine** Vorgabe, sechs Werte sichtbar zu machen.

### 1.7 Abnahme U22 (06.09.2026) — Folge, wörtlich

`design/abnahmen/2026-09-06-fragenrunde-rollen-u22.md`:

> **Antwort (Auswahl):** „Fünf (Empfohlen)" — 2026-09-06 16:47.
>
> **Folge:**
>
> - Eine Quelle kann genau **fünf** musikalische Rollen bekommen: führt, trägt,
>   begleitet, geschützt, bewusst verschmolzen. Die sieben Rollen aus Entwurf §9
>   (Fokus, Fundament, Begleitung, Impuls, Raum, Geschützt, Bewusst verschmolzen)
>   sind damit Verlauf; §9 trägt den Entscheid als Erratum (h) vom 06.09.2026.
> - **Impuls** und **Raum** sind keine Rollen. Kurze Anschläge werden über die
>   Schutzangabe an der Quelle geschützt (Entwurf §37.1: geschützte Eigenschaften
>   wie Attack, Breite oder Ausklang); Hall, Atmosphäre und Breite dürfen über die
>   Verschmelzungs-Erlaubnis verschmelzen (§37.1: „A/B dürfen verschmelzen"). Der
>   Advisor berücksichtigt beides aus diesen Angaben, nicht aus einer sechsten oder
>   siebten Rolle.
> - Wie die fünf Rollen auf das Intent-Modell des Entwurfs (§37.1:
>   Vordergrund/Mitte/Hintergrund, Funktionstags, Schutz, gerichtete Beziehungen)
>   abgebildet werden, ist Technik von S23–25 und wird dort in der
>   Verhaltensmatrix festgelegt; die Produktzusage ist die Anzahl.
> - Die englischen Rollenwörter der Oberfläche sind nicht Gegenstand dieser
>   Karte; ob die Wörter aus der Hub-Runde vom 22.08.2026 (U6.3: LEADS · CARRIES
>   · SUPPORTS · PROTECTED · MERGED) weiter gelten, entscheidet die Bedienfläche
>   (S31b) am Figma-Stand.
> - Register: NAK-38 ist damit vollständig geschlossen (die Rollenanzahl war
>   seine letzte offene Teilfrage). `docs/plan/plan.json` (S23–25) und
>   `docs/bauaufteilung-sonden.md` tragen den Nachtrag; `docs/plan/fragen.json`
>   führt U22 als beantwortet.

Dazu der Entscheid, den Entwurf §9 seit dem 06.09.2026 als Erratum (h) trägt
(`docs/FL-Nakama-Sonden-Design-Entwurf.md`, ab Zeile 900), wörtlich:

> > **[0.5 · Erratum (h) · Entscheid 06.09.2026]** Die Rollenliste ist **entschieden: fünf Rollen** —
> > führt, trägt, begleitet, geschützt, bewusst verschmolzen. User-Auswahl auf Karte U22 in der
> > Dirigenten-Sitzung: „Fünf (Empfohlen)". Die sieben Rollen oben sind Verlauf. **Impuls** und
> > **Raum** sind keine Rollen: kurze Anschläge werden über die Schutzangabe an der Quelle
> > geschützt (§37.1, geschützte Eigenschaften wie Attack), Hall, Atmosphäre und Breite dürfen
> > über die Verschmelzungs-Erlaubnis verschmelzen (§37.1, „A/B dürfen verschmelzen"). Wie die
> > fünf Rollen auf `SourceIntent` (§37.1) abgebildet werden, ist Technik von `SONDE-014`
> > (S23–25) und steht in dessen Verhaltensmatrix; die Produktzusage ist die Anzahl. Register
> > NAK-38 damit geschlossen.

### 1.8 Register `docs/offene-punkte.md`, wörtlich

**NAK-168** (Zeile 180):

> **[Lücke · SONDE-014]** **Woher der adressierte Befund und die geschützten Bereiche der Guardrails kommen, ist nicht definiert.** `experiment_verdrahtung.rs:934-953` erklärt das Band mit dem größten absoluten Delta zum Ziel und nimmt es aus dem Guardrail; weder `experiment_begin` noch `experiment_candidate` tragen ein Feld dafür, der Entwurf nennt geschützte Bereiche ohne Quelle (Zeile 2868). Eigentümer: SONDE-014 (Absicht, Ursachenhypothese, Vorschlag) — der Vorschlag benennt Zielbereich und Schutzbereiche, das Experiment liest sie.

**NAK-190** (Zeile 199):

> **[Planarbeit · S23–25 / G5]** **M-85 Ziel 1 („korrelierter, aber nicht kausaler Distraktor wird als Alternative gezeigt") ist P5-Lieferumfang.** Es setzt Ursachen und Alternativen voraus (Entwurf §59); P4 gibt keine Ursachenaussage ab. Der Begriff kommt in keinem Code, Test oder Fixture vor; der P4-Korpus behauptet ihn zu Unrecht (Korrektur in NAK-182). Geht als Falsifikationsziel in die SONDE-014-Ticketquelle und in G5 („falsche starke Ursachenbehauptung provozieren").

**NAK-198** (Zeile 204):

> **[Lücke · P4 · Paarurteil]** **`Dreifachergebnis` trägt kein Intervall.** `broker/src/coordinator/prepost.rs:294-313` führt `roh_db`, `ausgerichtet_db`, `match_gain_db`, `wirkung` — keine Unsicherheit; die lebt nur im Experimentpfad (`Achsenrechnung.intervall`). M-83 Satz 2 ist deshalb im Experimentpfad gemessen (NAK-182 E4b, N-22). Eine spätere Sicht auf ein Paarurteil mit Gainaussage bräuchte ein Intervallfeld — Schemafrage, nie beiläufig additiv.

**NAK-204** (Zeile 210):

> **[Lücke · Vertrag · M-45]** **`guardrail_loudness_db` und `guardrail_peak_db` sind im Produktpfad hart `None`** (`experiment_verdrahtung.rs`), weil `evidence_snapshot` weder integrierte Lautheit noch True Peak trägt (bewusst, NAK-181 B11). Zwei der fünf M-45-Guardrails sind ohne Vertragsteil nicht messbar — gehört mit NAK-193 (`seitenanteil_db`) in denselben Skalar-Entscheid.

**Weitere Registerzeilen, die der Ist-Stand dieses Tickets als betroffen findet:**

**NAK-193** (Zeile 202) — ausdrücklich von NAK-204 als Teil desselben
Skalar-Entscheids benannt:

> **[Defekt · P4 · Vergleichsevidenz]** **`stereo.seitenanteil_db` erreicht den Broker nie — der Breite-Guardrail M-45 ist im Produktpfad tot.** `broker/src/coordinator/evidenz.rs:279-282` (Stand Etappe 1) liest `wert.pointer("/stereo/seitenanteil_db").and_then(Value::as_f64)`; das Feld ist im Schema ein Objekt (`$defs/stereo_evidenz.properties.seitenanteil_db` → `$ref: stereo_bandwerte`), und `NakamaEvidenz.cpp` schreibt es auch so. `as_f64` auf ein Objekt ist `None`, `Evidenzstand::seitenanteil_db` damit strukturell immer `None`; sein einziger Konsument (`experiment_verdrahtung.rs`, `.filter_map(s.seitenanteil_db)`) sieht nie einen Wert. Außerhalb von NAK-181 (kein benannter G4-Befund, kein Ticketpfad). Der Fix braucht den Entscheid, welcher Skalar aus den 221 Bändern der Guardrail ist (Technik beim Dirigenten; an den User nur, wenn die Wahl hörbar andere Urteile erzeugt). Kein Schemafeld nötig.

Warum betroffen: der Guardrail-Block, den NAK-168 diesem Ticket zuweist, ist
derselbe Block, in dem NAK-193 und NAK-204 hängen. Ein Proposal, das
Zielbereich und Schutzbereiche benennt (NAK-168), macht die Frage „welcher
Skalar ist der Breite-/Loudness-/Peak-Guardrail" nicht überflüssig — sie bleibt
Vorbedingung dafür, dass ein Guardrail überhaupt fallen kann. Die Matrix hält
das getrennt: **M-47** und **M-48** schließen NAK-168 (der Vorschlag benennt den
Zielbereich, das Experiment liest ihn), **M-50** verkettet NAK-193 und NAK-204
ehrlich — ein Proposal, dessen Stopbedingung auf einen nicht messbaren
Guardrail zeigt, ist nicht handelbar. Der Skalar-Entscheid selbst bleibt bei
NAK-193/NAK-204 und ist nicht Gegenstand dieses Tickets.

---
## 2. Ist-Stand, an der Quelle gemessen

Alle Angaben sind in dieser Sitzung auf `5e99a13` gelesen, nicht aus Doku
übernommen. Datei und Zeile gelten zu diesem Stand.

### 2.1 Belegklassen (Definition wie `docs/beweise/SONDE-012.md` §1.1)

- **BELEGT:** Die Zusage folgt aus einer verbindlichen Quelle. Dass der heutige
  Produktpfad sie noch nicht erfüllt, macht sie nicht offen.
- **BAULÜCKE:** Die Zusage ist belegt, aber mindestens eine heute nötige
  Implementierungs- oder Testkante fehlt.
- **ENTSCHEIDEN-VORSCHLAG:** Eine aus den Quellen nicht ableitbare **Technik**frage.
  Diese Matrix trägt Vorschlag, Begründung und Alternativen ein; der Dirigent
  entscheidet. Bis dahin ist die Zeile kein Bauauftrag.
- **ENTSCHIEDEN:** Eine zuvor nicht aus den Quellen ableitbare Technikfrage ist
  durch den Dirigentenentscheid geschlossen — hier durch E-01 bis E-10 vom
  06.09.2026, wörtlich in
  `docs/beweise/roh/SONDE-014-etappe-1-entscheid-auftrag.txt` und je Entscheid
  als Block „Etappe 1b" unter dem zugehörigen §4-Abschnitt. Eine weiterhin
  fehlende Implementierung bleibt **zusätzlich** als BAULÜCKE markiert.
- **OFFEN:** Mindestens ein Teil der Zusage lässt sich aus keiner Quelle
  ableiten und ist **Produkt**frage. Diese Zeile ist kein stiller Produktentscheid;
  sie wird benannt und einer Fläche zugeordnet, nie hier beantwortet.

Seit Etappe 1b trägt **keine** Matrixzeile mehr die Klasse
ENTSCHEIDEN-VORSCHLAG: alle neun Fragen sind entschieden. Die Klasse bleibt in
dieser Liste stehen, weil §4 den vorigen Wortlaut als Historie führt und ein
späterer Fund sie wieder brauchen kann.

Jede Matrixzeile nennt ein bestehendes Prüfbein namentlich aus
`tools/beweise.ps1` oder ein ausdrücklich mit **NEU** gekennzeichnetes. Ein
neuer Beinname ist Teil der späteren Bauaufgabe, nicht die Behauptung, das Bein
existiere.

### 2.2 (a) Evidenzgraph heute

`evidence_snapshot` ist im v3-Vertrag vollständig und strikt:
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `$defs/evidence_snapshot`,
`additionalProperties: false`, `required` = `type`, `evidence_id`, `adresse`,
`transport`, `metrics_version`, `verteilung`, `baender`, `abdeckung`,
`konvergenz`, `konfidenz`; optional `beeinflusst`, `ausschlussgrund`,
`ereignisse`, `stereo`. `abdeckung` und `konvergenz` sind `number` in `[0,1]`.
`verteilung` ist ausdrücklich **additiv** (`additionalProperties: true`,
`maxProperties: 11`, Pflicht `p10`, `p50`, `p95`), `konfidenz` ebenfalls
(`additionalProperties: true`, `maxProperties: 16`, Pflicht `metrics_version`
und `klasse` mit dem Enum `stark|mittel|schwach|unbrauchbar`).

**Der Snapshot selbst ist nicht additiv.** Genau darin liegt der
Vertragsentscheid dieses Tickets: ein neues Feld am Snapshot ist ein
Fassungsschritt, ein neues Feld in `verteilung` oder `konfidenz` ist keiner.

Empfänger ist `broker/src/coordinator/evidenz.rs`. `Evidenzstand` (Zeile 47)
ist die Speichersicht des Brokers: `evidence_id`, `sequence`,
`empfangsfolge` (die brokerweit monotone **Ankunfts**reihenfolge, Nacharbeit 2
Befund R17), `abdeckung`, `konvergenz`, `klasse`, `ereignisse`,
`ereignisse_verloren`, `verteilung_fenster`, `samples_nicht_endlich`,
`beeinflusst`, `hat_baender`, `hat_verteilung`, `hat_stereo`,
`ausschlussgrund`, `p50_db`/`p50_gueltig`, `p95_db`/`p95_gueltig`,
`seitenanteil_db`, `onset`, `transport_epoch`, `continuity_segment`,
`project_sample_start`, `sample_count`, `sample_rate`, `resolution`.
`EVIDENZ_RETENTION = 32` (Zeile 39) deckelt die Historie je Quelle; die
vollständigen Bytes liegen im Store, nicht hier.

Die Bandquantisierung liest `evidenz.rs`:323-335: `q_db_0p1_i16` und
`q_db_0p01_i16` unterscheiden sich um den Faktor 10 beziehungsweise 100; der
0,01-dB-Pfad ist seit NAK-182 beidseitig gemessen (B16 aus dem Wiretext des
echten Serializers, A4 bandweise am Produkt, gekoppelt über
`evidenz-0p01-paar-wire-v1.json` aus A8).

Store: `broker/src/store/migration.rs`, `MIGRATION_1_SQL`. Tabellen
`schema_migrations`, `conflict_guards`, `event_log` (append-only, mit
UPDATE-/DELETE-Trigger), `projects`, `sessions`, `passages`, `evidence`,
**`findings`** (Zeile 86), **`proposals`** (Zeile 95), `transactions`,
`experiments`, **`user_verdicts`** (Zeile 122), `experiment_events`, `outbox`.
Jede Domänentabelle trägt `<id>`, `project_binding_id`, `session_epoch`,
`last_event_ord` und `state_jcs`; die Projektion ist monoton
(`WHERE excluded.last_event_ord > <tabelle>.last_event_ord`).

**Der wichtigste Ist-Befund dieses Abschnitts:** die Projektion für `finding`,
`proposal` und `user_verdict` existiert bereits. `broker/src/store/writer.rs`
führt bei :569–578 eine geschlossene Zuordnung von `event_type` auf
Domänentabelle; darin stehen `"finding"` (:572), `"proposal"` (:573) und
`"user_verdict"` (:576) neben `"passage"`, `"evidence"`, `"transaction"` und
`"experiment"`.

Was fehlt, ist ausschließlich der **Produzent**. In dieser Sitzung gemessen:
`grep -rn '"finding"\|"proposal"\|"intent"\|"assistant_step"'` über
`broker/src` und `eq-copilot/plugin` (`.rs`, `.cpp`, `.h`) liefert **genau
zwei** Treffer, beide in `writer.rs`:572 und :573 — kein Sender, kein
Empfänger, kein Modell. Die Wörter `intent` und `assistant_step` kommen als
Ereignistyp **nirgends** vor.

SONDE-014 braucht damit **keine Store-Migration 2**: die Tabellen und ihre
Projektion stehen seit Migration 1 (SONDE-011), und die Prüfsumme des
Migrationsskripts (`migration_1_checksum()`) bleibt unberührt.

### 2.3 (b) Experimentkern aus SONDE-013

`broker/src/coordinator/experiment.rs` (1536 Zeilen) führt `Passage` (119),
`Experimentreferenz` (154, ohne Setter und ohne `&mut`), `Kandidat` (175),
`Achsenrechnung` (199), `Resultatmessung` (233), `Terminal` (538),
`Experiment` (560), `Ereignis` (678), `Ruecknahme` (725) und
`Experimentstore` (754). Die Schwellen stehen als Konstanten:
`GUARDRAIL_ABDECKUNG = 0.1` (514), `GUARDRAIL_LOUDNESS_DB = 1.0` (516),
`GUARDRAIL_PEAK_DB = 0.5` (519), `GUARDRAIL_TRANSIENT = 2.0` (521),
`GUARDRAIL_BREITE_DB = 1.5` (523), `GUARDRAIL_GESCHUETZT_DB = 1.0` (526),
`BOOTSTRAP_BLOCK = 4` (529), `BOOTSTRAP_ZIEHUNGEN = 400` (531),
`KLASSENORDNUNG = ["unbrauchbar","schwach","mittel","stark"]` (534).

`broker/src/coordinator/experiment_verdrahtung.rs` (1782 Zeilen) ist der
Produktpfad. Der Guardrail-Block liegt bei :1009–1088. **Zwei Guardrails sind
hart `None`:** `guardrail_loudness_db` und `guardrail_peak_db` (:1039–1042),
mit begründeter Meldung in `guardrail_nicht_gemessen` — `evidence_snapshot`
trägt weder integrierte Lautheit noch True Peak (NAK-204). **Der Zielbereich
wird geraten:** :1069–1088 sortiert die gültigen Banddeltas nach Betrag,
erklärt `sortiert[0].0` zum Ziel und nimmt es aus dem geschützten Bereich
heraus. Das ist genau NAK-168.

> **Nebenbefund (fremde Datei, nicht geändert):** NAK-168 zitiert diese Stelle
> als `experiment_verdrahtung.rs:934-953`. Zum Stand `5e99a13` steht dort der
> Bandmittelblock `mittel_je_band`; die Zielheuristik liegt bei :1069–1088. Die
> Zeilen sind seit dem 05.09.2026 durch `00fa14e` (NAK-182 B6) verrutscht. Die
> Aussage der Registerzeile stimmt, ihre Position nicht. Notiert in §4.11 als N-01.

`prepost.rs` (1170 Zeilen): `Alignmentklasse` (46), `Ausschlussgrund` (63),
`Herabstufungsgrund` (93), `Kettenbefund` (122), `Paarhaelfte` (144),
`Restlag` (241), `Frameschluessel` (271), **`Dreifachergebnis` (294)** mit
`roh_db`, `roh_gueltig`, `ausgerichtet_gueltig`, `baender_ohne_messung`,
`ausgerichtet_db`, `match_gain_db`, `wirkung` — **kein Intervallfeld**
(NAK-198). `Paarurteil` (318), `beurteile_paar` (974), `bilde_paare` (1146).

`vergleichbarkeit.rs` (367 Zeilen): `METRICS_VERSION = 20260904` (149),
`GATE_ZEITUEBERDECKUNG = 0.95` (152), `GATE_QUELLEN_JACCARD = 0.9` (154),
`GATE_MATERIAL_COSINE = 0.95` (156), `GATE_ABDECKUNG = 0.5` (160),
`beurteile` (280). Die Funktion `ueberdeckung` (168) rechnet
**Zeit**überdeckung zweier Projektfenster, normiert auf das kürzere.

`intervention.rs` (628 Zeilen): `intervention_begin` (104),
`intervention_begin_mit_art` (123), `intervention_end` (165),
`hoermarkierung_v2` (255), `intervention_overflow` (292),
`tail_fortschritt` (313), `neutral_resync` (379),
`experiment_intervalle_schliessen_fuer_link` (470),
`taint_verworfen_zaehler` (533), `evidence_dispatch_brokerweit` (610).

`invalidierung.rs` (285 Zeilen): geschlossene Gründemenge `Grund` (31) mit
`Intervention`, `RoutingUnbekannt`, `Sequenzluecke`, `Epochwechsel`,
`StateRevision`, `MaterialWechsel`, `MesspunktWechsel`; `Umfang` (64) als Enum
mit `Ids`, `Bereich { von, bis }`, `GanzeSitzung`; `Bruchart` (128),
`grund_fuer_bruch` (146), `quarantaene_fuer_intervention` (164),
`material_wechsel` (203), `messpunkt_wechsel` (229), `Segmentierung` (252).

**Der Invalidierungspfad kennt heute nur Evidenz.** Es gibt keinen Empfänger,
der aus einer zurückgenommenen Evidenz-ID eine abhängige Hypothese ableitet —
weil es keine Hypothesen gibt. `writer.rs` projiziert
`event_type == "evidence_invalidate"` in die betroffenen `evidence`-Zeilen
(`evidence_ausschluss_projizieren`), nicht darüber hinaus.

### 2.4 (c) Passage, Fingerprint, Taint und Hörmarker

`experiment.rs::Passage` (119) trägt `passage_id`, `projekt_von`,
`projekt_bis` (halboffen), `transport_epoch`, `aktive_quellen`,
`messpunktklassen` (parallel und in derselben Reihenfolge, Nacharbeit 2 R23),
`abdeckung` als **gemessene** Signalabdeckung in `[0,1]` (nicht Wanddauer,
SONDE-013 M-30), `label` als `Option<String>` (User-Wort, nie interpretiert)
und `fingerprint`. Der Wirevertrag ist `$defs/passage`,
`additionalProperties: false`, alle neun Felder Pflicht.

`$defs/fingerprint`, `additionalProperties: false`, Pflicht `version`,
`band_energie`, `chroma`, `onset`.

`$defs/audible_intervention_begin` trägt `type`, `intervention_id`, `adresse`,
`event_sequence`, `art`, optional `experiment_id` und `project_sample_start`.
Das `experiment_id` ist seit Fassung 2 (Nacharbeit 2, R22, M-59) optional
belegt; bei `art=experiment` ist es Pflicht — diese Kante gehört dem Consumer,
weil die geschlossene Schlüsselwortmenge keinen Feldvergleich kennt
(`reservierte-nachrichten-v1.json`, `belegte_felder`).

Plugin-Seite: `eq-copilot/plugin/src/HoerMarkierung.h`, Bein **A3**
`EqCopMarkierungTest` — „Hör-Markierung bleibt verriegelt: Render/Freilauf
bitgleich, Analyse-Abgriff sitzt vor der Färbung" (`tools/beweise.ps1`:449).

### 2.5 (d) Main-State heute

`eq-copilot/schemas/state/nakama-state-v2.md` beschreibt `NakamaState`
`schema="2"` mit den unabhängig versionierten Kindern `Common` (Pflicht),
`MainProject` (`schema = 1`, nur `plugin_kind = main`, dort Pflicht),
`Parameters` (nur `active_probe`), `Dsp` (Name reserviert, SONDE-015) und
`Pairing` (Name reserviert, SONDE-016). Die Kind-Matrix (§2.1) verbietet
`main` ausdrücklich `Parameters` **und** `Dsp`.

Im Code: `eq-copilot/plugin/state/NakamaState.cpp` führt
`kMainProject` (21), `kMainMitglieder = "confirmed_members_v1"` (31),
`kMainPassagen = "manual_passages_v1"` (36) und `kMainSchema = 1` (43).
`NakamaState.h` führt `MainProjectMitglied` (68) mit
`maxMainProjectMitglieder = 64` (77) und `ManuellePassage` (94) mit
`maxManuellePassagen = 64` (111); `Zustand` (123) hält
`mainProjectMitglieder` (129) und `manuellePassagen` (130).
Die Validierung ist streng: `main requires exactly one MainProject` (731),
`MainProject is not allowed for <klasse>` (732), unbekanntes MainProject-Schema
⇒ read-only (736–738).

**Was fehlt für dieses Ticket:** `MainProject` trägt heute genau zwei
Eigenschaften. Es gibt **keinen** Platz für `SourceIntent` und **keinen** für
`AssistantStep`, obwohl Entwurf §33.5 (Zeile 2147) beide ausdrücklich dem
`MainProjectState` zuweist und §53 (Zeile 4061) das Kind mit
„Intent, Mitgliedschaft, Passage, AssistantStep, Outbox" beschriftet. Beide
Objekte sind damit eine **additive Eigenschaft** in einem bekannten Kind
desselben Majors — laut §2.1 der Schemadatei „additiv, wird erhalten" —, kein
neues Kind und keine Root-Versionierung.

Host-Dirty: `eq-copilot/plugin/src/PluginProcessor.cpp`:2174 ruft
`updateHostDisplay (…ChangeDetails().withNonParameterStateChanged (true))`;
`PluginProcessor.h`:821 nennt den Weg („der VST3-Wrapper setzt daraus
`IComponentHandler2::setDirty`"). Gemessen wird er von Bein **B14**
`EqCopSonde012ProjectReloadTest`, Fall
`confirmed_join_ack_and_name_each_mark_host_dirty`
(`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp`:314).

> **Nebenbefund (fremde Datei, nicht geändert):** Der Zustandsbaum in
> `eq-copilot/schemas/state/nakama-state-v2.md` §2 führt unter `MainProject`
> nur `confirmed_members_v1`. Die Eigenschaft `manual_passages_v1`, die
> `NakamaState.cpp`:36 schreibt und liest, steht dort **nicht**;
> `grep -c "manual_passages"` auf die Schemadatei liefert `0`. Notiert in §4.11 als N-02.

### 2.6 (e) v3-Nachrichten und reservierte Namen

`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`, in dieser Sitzung mit
`py -3.13` gezählt: **22 definierte** Familien, **7 reservierte** Familien,
`gesamt_erwartet = 29`; dazu **5 belegte** Nachrichten, **14 belegte Felder**
und **5 reservierte Felder**. Die Zahl 29 wird von `pruefe_v3_vertrag.py`
(Bein **A5**) gegen das `oneOf` des Schemas gehalten und ist damit die
maschinenlesbare Wahrheit; der Fließtext im Feld `zweck` rechnet noch auf 28
und hat `experiment_candidate` nicht mitgezählt (§4.11, N-05).

**Für dieses Ticket entscheidend — zwei Namen gehören SONDE-014 bereits:**

> \| Name \| Eigentümer \| Grund (wörtlich) \|
> \|---\|---\|---\|
> \| `draft_offer` \| SONDE-014 (P5) \| „Traegt ein `Proposal` — kleinster Versuch mit Grenzen, Hoerziel und Stopbedingung. Das Objekt entsteht in P5." \|
> \| `user_verdict` \| SONDE-014 (P5) \| „Userurteil bindet an Finding/Proposal-IDs, die es vor P5 nicht gibt." \|

**Und drei Namen gibt es nicht:** `finding`, `intent` und `assistant_step`
kommen im Register weder als definiert noch als reserviert vor. Sie sind
**frei** — und genau deshalb ein Vertragsentscheid dieses Tickets, nicht eine
Nebensache: die Regel des Registers lautet wörtlich „Ihre NAMEN sind trotzdem
jetzt vergeben. Ein reservierter Discriminator wird vom v3-Parser ABGELEHNT …
Genau das ist der Vertragsanteil, der spaeter bricht, wenn man ihn heute offen
laesst."

Die nicht additiv erweiterbaren Teile: `x-nakama-discriminator` (Wurzel des
Schemas), `$defs/adresse` (Zieladresse), Revision und `$defs/capabilities`.
`$defs/session_snapshot` ist `additionalProperties: false` mit
`type`, `session_epoch`, `broker_epoch`, `fuehrendes_main`,
`beitritt_bestaetigung_noetig`, `experimente`, `paare`, `store_degraded`,
`mitglieder` — `experimente` und `paare` kamen in Fassung 2 als optionale
belegte Felder dazu (Nacharbeit 2, R14/R32). Derselbe Weg steht für Findings
offen und wäre ein Fassungsschritt, kein neuer Kanal.

Die Experimentfamilien tragen **kein** Feld für Zielbereich oder
Schutzbereiche — gemessen am Schema, alle `additionalProperties: false`:

> \| Familie \| Felder \|
> \|---\|---\|
> \| `experiment_begin` \| `type`, `kopf`, `experiment_id`, `execution_mode`, `reproduzierbarkeit`, `passage`, `referenz` \|
> \| `experiment_candidate` \| `type`, `kopf`, `experiment_id`, `referenz`, `blindreihenfolge` \|
> \| `experiment_abort` \| `type`, `kopf`, `experiment_id`, `grund` \|
> \| `experiment_manual_result` \| `type`, `kopf`, `experiment_id`, `hoerurteil`, `blindreihenfolge`, `notiz`, `werkzeug` \|
> \| `experiment_referenz` \| `passage_fingerprint`, `upstream_fingerprint`, `aktive_quellen`, `messpunktklassen`, `match_gain_db`, `nicht_endliche_samples`, `alignment` \|

Damit ist NAK-168 am Vertrag bestätigt: der Zielbereich hat heute keinen
Wireplatz.

### 2.7 (f) Korpus A25/A26/A27

`tools/eq-copilot/erzeuge_p4_korpus.py` (Bein **A25**, `--pruefen`) erzeugt
`eq-copilot/fixtures/p4-korpus/` bytegleich neu.
`tools/eq-copilot/pruefe_p4_korpus.py` ist zweimal im Kanon: **A26** ohne
Argument als Sammelbein des P4-Exit-Gates, **A27** mit `--selbsttest`.

`eq-copilot/fixtures/p4-korpus/MANIFEST.json`: `faelle_gesamt = 25`,
`klassen = ["referenz","stereo","vergleich","zeitachse"]`, dazu die vier
Dateien `referenz.json`, `stereo.json`, `vergleich.json`, `zeitachse.json`.
Ein Fall trägt `fall`, `bein`, `wahrheit`, `aussage`, `konfidenz`, `hinweis`
und `quelle` (`datei` plus wörtlicher `bezeichner`, der dort gesucht wird).

Die geschlossenen Mengen aus dem MANIFEST:

> \| Menge \| Werte \|
> \|---\|---\|
> \| `erlaubte_aussagen` \| `schwach`, `stark`, `unsicher` \|
> \| `keine_behauptung` \| `unbekannt`, `unvergleichbar`, `zeitvariabel` \|
> \| `keine_starke` \| `unbekannt`, `unvergleichbar`, `zeitvariabel`, `nicht_kausal`, `nicht_exakt` \|

`erlaubte_wahrheiten` führt 16 Werte (`getrennt`, `identisch`, `innerhalb`,
`invertiert`, `laufzeit`, `mono`, `nicht_exakt`, `nicht_kausal`, `segment`,
`ueberschreitet`, `unbekannt`, `unkorreliert`, `unvergleichbar`,
`wiedergefunden`, `zeitstabil`, `zeitvariabel`).

Die Riegel in `pruefe_p4_korpus.py`: `ENTHALTUNG = "unsicher"` (91),
`GATE_BRIER_MAX = 0.25` (96), `GATE_COVERAGE_MIN = 0.5` (97),
`GATE_KALIBRIERUNG_MAX = 0.15` (98), `GATE_QUOTE_MAX = 1.0` (99). Gerechnet
werden `starke_behauptungen`, `enthaltungen`, `enthaltungsrate`, `coverage`,
`precision`, `recall`, `brier`, `kalibrierung` (`_kennzahlen`, 219);
Brier und Kalibrierung rechnen **nur** über die Behauptungen, nicht über die
Enthaltungen (Kommentar bei 239).

`MANIFEST.json` trägt außerdem die gedruckte Lücke `nicht_gemessen`:
`{"zusage": "M-85 Ziel 1 (korrelierter, nicht kausaler Distraktor)",
"ticket": "NAK-190", "grund": "setzt Ursachen und Alternativen voraus und ist
damit P5-Lieferumfang (Entwurf §59)"}`. Die zitierte Zusage ist **SONDE-013 M-85**, nicht die M-85 dieses Manifests. Das ist die
Anschlussstelle dieses Tickets: der Korpus **weiß**, dass ihm dieser Fall fehlt.

### 2.8 (g) Bandgitter und Frequenzbereiche

`eq-copilot/schemas/v3/bandgitter/nakama_1_24_oct_30_18k_v1.json`:
`art = fraktionale_oktave`, `band_anzahl = 221`, Zweck „Evidenzgitter — voller
Bandsatz des Evidenzsnapshots (Entwurf §33.2, 1-4 Hz)", mit `kanten_hz` und
`mitten_hz` als hex64-Bitmuster („hex64 ist die Wahrheit … dezimal ist
Dokumentation und wird von keinem Konsumenten gelesen").

`eq-copilot/schemas/v3/bandgitter/nakama_log64_v1.json`:
`art = gruppierung`, `band_anzahl = 64`, `basis_gitter =
nakama_1_24_oct_30_18k_v1`, Zweck „Livegitter — 10-Hz-Telemetrie (Entwurf
§33.2)". Bein **A6** hält beide bytegleich und prüft, dass die 64 Gruppen eine
**exakte Partition** der 221 Bänder sind.

Der C++-Zugriff liegt in `eq-copilot/plugin/core/analysis/BandGrid.h` und
`BandGridZahlen.h`; **A19** `erzeuge_bandgitter_header.py` hält den Header
gegen das Register.

**Damit ist ein je Frequenzbereich auflösbarer Wert bereits adressierbar** —
ein Bandindex im 221er- oder 64er-Gitter benennt einen Frequenzbereich
eindeutig, ohne dass eine neue Achse entsteht. Die Nyquist-Kappe
`min(18 kHz, 0,95 · Nyquist)` (SONDE-013 M-10) gilt für jede bandweise Metrik;
es entsteht keine zweite Kappenregel.

### 2.9 (h) Datenkadenz heute

Drei getrennte Takte, die nicht vermischt werden dürfen:

1. **Livetelemetrie P2:** 10 Hz, 64 Gruppen, FlatBuffers (`FeatureBatch` mit
   CRC32C), Entwurf §33.1/§33.2.
2. **Evidenz P1:** 1 bis 4 Hz, JSON, 221 Bänder.
   `eq-copilot/plugin/core/analysis/FeatureEngine.h`:989–990 setzt
   `kEvidenzIntervallMinS = 0.25` (4 Hz) und `kEvidenzIntervallMaxS = 1.0`
   (1 Hz); `:3562` setzt `f.evidenzFrisch = evidenzS >= evidenzIntervallS`.
   `eq-copilot/plugin/sonde/SondeProcessor.cpp`:424 sendet nur bei
   `frame.evidenzFrisch`; `:489` reduziert die Kadenz bei P1-Rückstau
   (`stand.p1Tiefe + stand.p1WiederholTiefe > kEvidenzP1Schwelle` ⇒ Intervall
   verdoppeln, Zähler `evidenzKadenzReduktionen` und `evidenzNichtGesendet`
   hoch), `:601` halbiert es wieder. **Nie wird der Ereignisring stillschweigend
   geleert** (SONDE-013 M-05).
3. **Anzeigekadenz im Editor:** `eq-copilot/plugin/src/WorkerCadence.h`,
   `leichtesIntervall = 50 ms` (20 Hz) und `schweresIntervall = 250 ms` (4 Hz);
   die Zustandsmaschine plant von **jetzt** aus weiter und holt nach einer
   Pause nie mehrere verpasste Takte nach.

Paint-FPS ist keine dieser drei Größen (`CLAUDE.md`, „Begriffe nicht
vermischen").

Rückstau- und Prioritätsklassen: `eq-copilot/plugin/core/ipc/ControlClient.h`
führt P0 (Steuerung, 64er-Queue, Überlauf ⇒ Verbindung wird verworfen, Zeile
314) mit `P0Klasse` für Linkwechsel-Überleben (317, NAK-180 R7),
persistenzpflichtiges `sendePersistenzP0` (556) und P1 (558) mit
Schlüsselkoaleszierung („Leerer `schluessel` = Ereignis; ein nicht leerer
Schluessel koalesziert Snapshots desselben Objekts", 559) und einer in
**Einträgen** gedeckelten Queue (128 + 128, Zeile 242). P2 ist die
FlatBuffers-Fläche und reist **nicht** über den ControlClient (260).

### 2.10 (i) Gen-Modelle im Plugin und was heute „Befund" heißt

`eq-copilot/plugin/src/SourcesModel.h`: `Zeile` (49) trägt Identität,
`Mitgliedschaft`, `Control`, `Messung`, `Betrieb`, `Lautheit`,
`Namensherkunft`, `CapabilityEvidenz`, `Messpunkt`, Namen, Host-Angaben,
`p2Reject*`, Alter, `fensterDauerMs`, `lufsI`/`lufsIUnsicherheitLu` und
**`findingsOffen` (72)**. Dazu `Versuch` (82) und `Paar` (102) aus dem
Sessionsnapshot (Nacharbeit 2, R14/R32) sowie `Sicht` (122).

**`findingsOffen` ist heute ein toter Zähler.** `setzeFindings`
(`SourcesModel.h`:193, `SourcesModel.cpp`:1210) hat im ganzen Repo genau
**einen** Aufrufer, und der ist ein Test
(`eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp`:469). Im Produktpfad
ruft ihn niemand. Der Blueprint verlangt die Zahl ausdrücklich („Jede
Quellenzeile zeigt Identität, Signalstatus, Messaktualität und Anzahl offener
Findings", `design/docs/funktions-und-bedien-blueprint.md` §4.1) — die
Anzeigefläche und das Modellfeld existieren, die Quelle der Zahl nicht.

Ein „Befund" im Sinne von Entwurf §8 existiert im Code **nirgends**. Was heute
existiert, sind Messungen (`evidence_snapshot`), Vergleichsurteile
(`Paarurteil`, `Vergleichsurteil`) und Experimentergebnisse
(`Achsenrechnung`, `Achsen`, `Urteil`).

### 2.11 (j) Wo die Zusagen aus §1 heute nicht gehalten werden — BAULÜCKE

| Nr. | Zusage aus §1 | Heutiger Stand, gemessen | Ort |
|---|---|---|---|
| L1 | `SourceIntent` mit Funktion, Front/Middle/Back, Schutz und gerichteter Priorität im Main-State | `MainProject` trägt zwei Eigenschaften: `confirmed_members_v1` und `manual_passages_v1`. Kein Intent-Feld, kein Rollenwert, keine Beziehung, keine Zyklusprüfung. | `eq-copilot/plugin/state/NakamaState.cpp`:31,36 |
| L2 | Genau fünf Rollen (U22) | Der Begriff „Rolle" existiert im Produktcode nur als `prepost::Rolle` (PRE/POST-Rolle einer Paarhälfte), was etwas anderes ist. Keine musikalische Rolle. | `broker/src/coordinator/prepost.rs`:1135 |
| L3 | Versionierter Evidenzgraph, `CauseHypothesis`, Alternativen, Ausschlussgründe und Konfidenz | Evidenz ist ein flacher Bestand je Quelle plus Store-Tabelle; es gibt **keine** Kanten und **keine** Hypothese. `findings` ist eine leere Tabelle mit Projektion ohne Produzenten. | `broker/src/store/writer.rs`:572; `broker/src/coordinator/evidenz.rs`:39,47 |
| L4 | Jede sichtbare Behauptung referenziert existente Evidenz-IDs (Exit-Gate) | Es gibt keine sichtbare Behauptung. `Resultatmessung.baseline_evidence_ids` und `.resultat_evidence_ids` existieren (`experiment_verdrahtung.rs`:1004–1007), aber nur für Experimente. | dieselbe Datei |
| L5 | Deterministische Policy für den kleinsten `Proposal`, einschließlich `keine Änderung` und `mehr Daten` | Kein Proposal-Objekt, kein Aktionstemplate, kein Constraint-Solver. `draft_offer` ist ein reservierter Name ohne Nutzlast. | `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` |
| L6 | Persistenter `AssistantStep` mit Abbruch, Zurück, Überspringen und Resume | Kein Objekt, kein Feld, keine Zustandsmaschine. Der Experimentkern kennt `Terminal` und `Abbruchgrund`, aber keinen Assistentenschritt. | `broker/src/coordinator/experiment.rs`:64,538 |
| L7 | Evaluationskorpus mit Precision/Recall je **Ursachenklasse** | A26 rechnet Precision/Recall je **Korpusklasse** (`referenz`, `stereo`, `vergleich`, `zeitachse`) — das sind Messklassen, keine Ursachenklassen aus Entwurf §8. | `tools/eq-copilot/pruefe_p4_korpus.py`:219 |
| L8 | Gegenbeispiele erzwingen nachweislich Enthaltung (Exit-Gate, G5) | Der Korpus hält Enthaltung als Kennzahl, aber **kein** Fall trägt einen korrelierten Distraktor, ein Parent-Duplikat oder eine verschobene Passage. Der Korpus druckt diese Lücke selbst. | `eq-copilot/fixtures/p4-korpus/MANIFEST.json`, `nicht_gemessen` |
| L9 | Laufender, je Frequenzbereich auflösbarer Maskierungswert mit Anzeigekadenz (U16, PR2) | Existiert nicht. Im Code heißt „Überdeckung" heute **Zeit**überdeckung zweier Passagenfenster (`vergleichbarkeit.rs`:168) und `abdeckung` heißt **Mess**abdeckung `[0,1]` (`$defs/evidence_snapshot`). Eine spektrale Maskierung zweier Quellen gibt es nirgends. | `broker/src/coordinator/vergleichbarkeit.rs`:168 |
| L10 | Der Vorschlag benennt Zielbereich und Schutzbereiche, das Experiment liest sie (NAK-168) | Das Experiment **rät** den Zielbereich aus dem größten Betrag der Banddeltas; die Wire-Familien tragen kein Feld dafür. | `broker/src/coordinator/experiment_verdrahtung.rs`:1069–1088 |
| L11 | Anzahl offener Findings je Quellenzeile (Blueprint §4.1) | Feld und Setter existieren, der Produktpfad ruft sie nie. | `eq-copilot/plugin/src/SourcesModel.cpp`:1210 |
| L12 | Rollenänderung macht Vorschläge stale (§37.3) | Es gibt weder Rolle noch Vorschlag; ein Stale-Pfad existiert nur für Evidenz (`invalidierung.rs`) und Experimente. | `broker/src/coordinator/invalidierung.rs`:31 |

### 2.12 Begriffe, die diese Matrix getrennt hält

`CLAUDE.md` verlangt „Begriffe nicht vermischen". Für dieses Ticket sind drei
Kollisionen real und benannt:

| Begriff | Bedeutung heute im Code | Wo | Nicht zu verwechseln mit |
|---|---|---|---|
| `abdeckung` | gemessene Signalabdeckung `[0,1]` einer Passage beziehungsweise eines Belegs | `$defs/evidence_snapshot`, `experiment.rs::Passage.abdeckung` | Coverage als Kennzahl des Evaluationskorpus (`pruefe_p4_korpus.py`:261: Anteil der Fälle **ohne** Enthaltung) |
| `ueberdeckung` / `zeitueberdeckung` | Zeitüberlappung zweier Projektfenster, normiert auf das kürzere | `vergleichbarkeit.rs`:168,122 | die **spektrale Maskierung** zweier Quellen aus U16 |
| `Ueberdeckungszone` (Plantext U16/PR2) | die farbige Zone auf der Kurve, Anzeige, S31b | `docs/plan/plan.json` S23–25 | ihr **Datenweg**, der hier liegt |

Der Blueprint löst die dritte Kollision bereits auf und nennt die Sache
**Maskierungszone**: „frequenzaufgelöster Datenweg für die Maskierungszone"
(`design/docs/funktions-und-bedien-blueprint.md` §3.4) und „Die Maskierungszone
ist eine zweite Darstellung desselben Finding-Datenwegs, keine eigene Detektion
und keine konkurrierende Schwelle" (ebenda, Bedienvertrag P5). Diese Matrix
übernimmt das Wort **Maskierung** für die Sache und lässt `ueberdeckung` beim
Zeitbegriff. Der Vorschlag für den Feldnamen steht in §4.4.

---
## 3. Verhaltensmatrix

Die Spalte **Rotbeweis** nennt, woran die Prüfung fallen **muss**, wenn die
Zusage verletzt ist. Ein Rotbeweis, der an einem Nebeneffekt fällt — Filter mit
`t0`, Rundungsphase, Übersetzungsfehler, ein Test, der das Modul direkt ruft —,
ist ungültig; die Matrixprüfung von NAK-182 fand genau diesen Fehlertyp zweimal.
Wo ein bestehendes Bein genannt ist, steht sein Kürzel aus `tools/beweise.ps1`;
neue Beine tragen **NEU** und existieren heute nicht.

**Die IDs werden in der Reihenfolge ihrer Entstehung vergeben, nicht in der
Reihenfolge der Abschnitte.** Seit der Matrixnacharbeit 1 (06.09.2026) laufen sie
deshalb nicht mehr durchgehend aufsteigend durch §3.1 bis §3.10: **M-87**
steht in §3.2 (Ausschlussgründe, R4), **M-88** und **M-89** in §3.10
(Assistentenschritt, R3). Das ist Absicht — alles unter dem lebenden Kopf ist
append-only Verlauf, und eine vergebene ID wird nie umgehängt.

### 3.1 SourceIntent und die fünf Rollen (Gate: §59 Punkt 1, §37, U22)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-01 | Gen ist klassifiziert `main`; der User setzt eine Rolle an einer Quelle | Die Rollenmenge ist **geschlossen und hat genau fünf Werte**: führt, trägt, begleitet, geschützt, bewusst verschmolzen. Ein sechster Wert wird abgewiesen, nicht auf einen bekannten Zweig abgebildet. Die Menge lebt an genau einer Stelle im Vertrag; Leser in C++ und Rust prüfen gegen dieselbe Menge. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fall `rollenmenge_ist_geschlossen_und_fuenf`; **A5** erweitert um das Enum | Ein sechster Enumwert wird angenommen; oder C++- und Rust-Leser klassifizieren denselben Wert verschieden | U22 (§1.7); `docs/plan/plan.json` S23–25 Nachtrag 06.09.; Entwurf §9 Erratum (h) |
| M-02 | Rolle steht fest; das Intent-Objekt wird gebildet | **E-01 (A) mit Präzisierung E-01a, entschieden 06.09.2026.** `rolle` ist ein geschlossenes Feld mit genau fünf Werten; eine Quelle trägt **je Scope** (global oder Passage) genau eine Rolle oder keine. Die Belegung nach §37.1 wird **abgeleitet**, nicht getrennt gesetzt: führt → (`foreground`, `lead`), trägt → (`middle`, `foundation`), begleitet → (`background`, `texture`), geschützt → (`middle`, leerer Funktionstag, Veto-Kennzeichen `schutz`), bewusst verschmolzen → (`middle`, leerer Funktionstag, Veto-Kennzeichen `verschmolzen`). Das Veto-Kennzeichen ist **Teil der Belegung**; nur so ist die Abbildung total und injektiv und der Rückweg eindeutig (§33.5: keine zweite Wahrheit). | **ENTSCHIEDEN** (E-01, §4.1) | **NEU** `EqCopSonde014IntentTest`, Fälle `rolle_und_belegung_sind_umkehrbar` (Roundtrip über alle fünf) und `zwei_belegungen_lesen_nie_dieselbe_rolle_zurueck` | Zwei verschiedene Belegungen lesen auf dieselbe Rolle zurück; oder `geschützt` beziehungsweise `bewusst verschmolzen` erzeugt eine andere `prominence` als `middle` | E-01/E-01a (§4.1); U22 „die Abbildung … ist Technik von S23–25“; Entwurf §37.1:2427–2443 |
| M-03 | Quelle hat kurze Anschläge, die erhalten bleiben sollen | **Impuls ist keine Rolle.** Der Schutz kurzer Anschläge ist eine **Schutzangabe an der Quelle** (§37.1 „geschützte … Eigenschaften wie Attack, Breite oder Ausklang“). **E-01a:** geschützte Bereiche und Eigenschaften — Attack, Breite, Ausklang, Bandintervalle — bleiben **orthogonal zur Rolle** und unabhängig von ihr setzbar; eine Quelle mit Rolle `begleitet` kann Attack geschützt haben, und eine Rollenänderung löscht keinen Schutzbereich. Produktwirkung des Entscheids, ausdrücklich benannt: wer Position **und** Schutz will, setzt eine Positionsrolle (führt, trägt, begleitet) und dazu Schutzbereiche — die Rolle `geschützt` trägt selbst keine Prominenzposition. | **BELEGT** · **ENTSCHIEDEN** (E-01a, §4.1) | **NEU** `EqCopSonde014IntentTest`, Fälle `attack_schutz_ist_unabhaengig_von_der_rolle` und `rollenwechsel_loescht_keinen_schutzbereich` | Der Schutz lässt sich nur zusammen mit einer bestimmten Rolle setzen; oder eine Rollenänderung löscht ihn | E-01a (§4.1); U22 Folge Punkt 2; Entwurf §9 Erratum (h) |
| M-04 | Zwei Quellen sollen bewusst verschmelzen | **Raum ist keine Rolle.** **E-02, entschieden 06.09.2026:** die Rolle `bewusst verschmolzen` ist ein **globales Veto** gegen jede Entmaskierungsempfehlung mit Beteiligung dieser Quelle; die **paarweise Erlaubnis** beziehungsweise gerichtete Beziehung aus §37.1 ist das **spezifischere** Werkzeug und gewinnt, wo sie gesetzt ist — eine ausdrückliche Beziehung „A führt vor B“ hebt das globale Veto **nur für dieses Paar** auf. Beide liegen auf Stufe 2 der Konfliktregeln (expliziter Userintent), spezifisch vor global, analog zur Regel der Stufe 3. | **ENTSCHIEDEN** (E-02, §4.2) | **NEU** `EqCopSonde014IntentTest`, drei Fälle: `nur_rolle`, `nur_beziehung`, `beides_im_widerspruch` | Ein Proposal zur Entmaskierung entsteht trotz Veto ohne gesetzte Beziehung; oder die Beziehung hebt das Veto für ein anderes Paar mit auf | E-02 (§4.2); Entwurf §37.1:2427–2443, §37.2:2445–2459; U22 Folge Punkt 2 |
| M-05 | Zwei Quellen tragen widersprüchliche Intents | Die Konfliktauflösung folgt **genau der Reihenfolge aus §37.2**: 1. Schutz-/Sicherheitsgrenze, 2. expliziter Userintent, 3. passagespezifischer vor globalem Intent, 4. bestätigte Vorlage, 5. abgeleitete Vermutung. Die Stufen sind eine geordnete Liste, kein Score; eine höhere Stufe wird von keiner Summe niedrigerer geschlagen. **E-02:** Rolle und paarweise Beziehung liegen **beide** auf Stufe 2; innerhalb der Stufe gewinnt die spezifischere Angabe (paarweise vor global), analog zur Regel der Stufe 3. | **BELEGT** · **ENTSCHIEDEN** (Stufenzuordnung, E-02, §4.2) | **NEU** `EqCopSonde014IntentTest`, Fälle `konfliktregeln_sind_eine_ordnung_kein_score` — alle zehn Paarungen der fünf Stufen — und `innerhalb_stufe_zwei_gewinnt_das_spezifischere` | Eine Kombination aus zwei niedrigen Stufen schlägt eine hohe; die Reihenfolge der Auswertung ändert das Ergebnis; oder die globale Rolle schlägt die gesetzte Beziehung | Entwurf §37.2:2445–2459; E-02 (§4.2) |
| M-06 | Ein gerichteter Intent-Graph wird gespeichert | **Beim Speichern** wird auf Zyklen geprüft. Ein Zyklus muss aufgelöst oder als **nicht steuerbare Gleichrangigkeit** gespeichert werden; er wird nie stillschweigend angewendet. Reihenfolge: Zyklusprüfung → Entscheidung → Persistenz → Host-Dirty. Ein Zyklus erreicht die Persistenz nie unmarkiert. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fälle `zyklus_wird_beim_speichern_erkannt` und `zyklus_wird_als_gleichrangigkeit_gespeichert` | Ein Dreierzyklus A→B→C→A wird gespeichert und beim Laden als gerichtet gelesen; oder die Prüfung läuft erst beim Anwenden | Entwurf §37.2:2445–2459; §37.4 „Zyklische Entmaskierungsprioritäten können nicht angewendet werden" |
| M-07 | Intent wird geschrieben | Jedes `SourceIntent` trägt **Revision**, **Herkunft** `user\|template\|inferred` und **Konfidenz**. Eine abgeleitete Vermutung überschreibt **nie** einen Userwert; sie bleibt als abgeleitet erkennbar. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fall `abgeleitet_ueberschreibt_user_nie` | Ein `inferred`-Wert ersetzt einen `user`-Wert; oder die Herkunft geht beim Speichern verloren | Entwurf §37.1:2427–2443; §37.2:2445–2459 |
| M-08 | Eine Passage ist markiert; Intent wird gesetzt | Der **Passage-Scope ist optional**. Derselbe Bus kann **ohne Datenverlust** verschiedene Rollen in zwei Passagen besitzen: Passage A führt, Passage B begleitet, und beide Werte überleben nebeneinander. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fall `zwei_rollen_je_bus_in_zwei_passagen`; **B14** erweitert (Roundtrip) | Das Setzen der zweiten Passagenrolle überschreibt die erste; oder ein Speichern/Laden verliert eine der beiden | Entwurf §37.1:2427–2443; §37.4:2467–2473 |
| M-09 | Broker startet neu, oder das FL-Projekt wird neu geladen | **Userwerte überleben Broker-Neustart und Projekt-Recall**; abgeleitete Werte bleiben als solche erkennbar. Der Intent reist mit der FL-Projektdatei im `MainProject`-Kind, nicht im SQLite-Spiegel. Reihenfolge beim Laden: `MainProject` lesen → Intent rekonstruieren → erst danach Ranking oder Schutz anwenden. | **BELEGT** | **NEU** `EqCopSonde014IntentTest` plus **B14** `EqCopSonde012ProjectReloadTest` erweitert; **A4** Broker-Neustartfall | Nach `git`-losem Neustart des Brokers ist der Intent leer, obwohl der Plugin-State ihn trägt; oder der Spiegel überschreibt den Plugin-State | Entwurf §33.5:2140–2175; §37.4:2467–2473 |
| M-10 | Intent ist persistent; der User ändert eine Rolle | **Vorschläge werden stale, nie still umgerechnet.** Jede auf Intent basierende Empfehlung referenziert die verwendete **Intent-Revision**; steigt die Revision, geht jedes abhängige Proposal und jeder abhängige Befund sichtbar in `STALE`, ohne dass Zahlen nachgerechnet werden. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `rollenaenderung_macht_stale_und_rechnet_nicht_nach`; **A4** Verdrahtungsfall | Ein Proposal behält nach der Rollenänderung seinen Zustand `READY TO SEND`; oder seine Parameter ändern sich ohne neue Erzeugung | Entwurf §37.3:2461–2465; U21-Folge (Zustand `STALE`) |
| M-11 | Intent liegt vor; eine Messung wird gerechnet | **Intent verändert nur Ranking, Schutz und erlaubte Tests. Er verändert keine Messwerte.** Derselbe Messeingang liefert mit und ohne Intent bitgleich dieselben Bandwerte, Perzentile und Konfidenzen. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fall `intent_beruehrt_keine_messwerte` (bitgleicher Vergleich zweier Läufe) | Ein Bandwert, ein Perzentil oder eine Konfidenzklasse unterscheidet sich zwischen Lauf mit und ohne Intent | Entwurf §37.3:2461–2465; `CLAUDE.md` „Engine kennt keine Optik" |
| M-12 | Eine Quelle hat **keine** Rolle | Ohne Rolle darf gemessen werden; Interpretationen werden **vorsichtiger formuliert**. Konkret und prüfbar: ohne Intent-Beitrag erreicht die Rangkomponente „Intent-Relevanz" ihren neutralen Wert und kann eine fehlende Coverage oder ein falsches Alignment nicht kompensieren — ein Befund ohne Intent kann `READY TO SEND` erreichen, aber nur über die anderen Komponenten. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `ohne_rolle_wird_gemessen_und_vorsichtiger_gewertet` | Eine fehlende Rolle blockt die Messung; oder sie hebt den Rang statt ihn neutral zu lassen | Entwurf §9 „Rollen bleiben optional"; §36.2:2367–2386 |
| M-13 | Eine persistente Intent-Änderung ist geschrieben | Jede persistente Änderung meldet dem Host **Dirty-State** über `updateHostDisplay(… withNonParameterStateChanged(true))`. Reihenfolge: Validierung → Zyklusprüfung → `ValueTree` schreiben → Dirty melden. Ein abgewiesener Wert meldet **kein** Dirty. | **BELEGT** | **B14** `EqCopSonde012ProjectReloadTest` erweitert um den Intent-Fall (`DirtyZaehler.nonParam`) | Ein abgewiesener Rollenwert erhöht den Dirty-Zähler; oder ein angenommener erhöht ihn nicht | `CLAUDE.md` „Jede persistente Änderung meldet dem Host Dirty-State"; `eq-copilot/plugin/src/PluginProcessor.cpp`:2174 |

### 3.2 Evidenzgraph, `CauseHypothesis` und die Ursachenklassen (Gate: §59 Punkt 2, §8, §36)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-14 | Evidenz liegt vor; ein Masterbefund soll erklärt werden | Die **sieben Ursachenklassen aus §8** sind eine geschlossene Menge: einzelne Quelle mit Resonanz/Überbetonung; zwei Quellen konkurrieren im selben Bereich; Effektkette erzeugt das Problem zwischen PRE und POST; mehrere kleine Beiträge summieren sich erst auf dem Master; Peakproblem aus einem transienten Bus; Stereoveränderung aus einer Quelle oder Kette; **die Daten reichen noch nicht**. Die siebte ist ein reguläres Ergebnis, kein Fehlerzweig. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `ursachenklassen_sind_geschlossen_und_sieben`; **A5** erweitert | Eine achte Klasse entsteht; oder „Daten reichen nicht" wird als Fehler statt als Ergebnis geführt | Entwurf §8:842–861 |
| M-15 | Eine Hypothese wird gebildet | Das Ergebnisobjekt trägt **mindestens** die Felder aus §36.3: `finding_id`, `claim_class`, `target_metric`, `candidate_source`, `passage_id`, `band_hz`, `confidence` (`class` **und** `score`), `evidence_ids`, `alternatives`, `next_test`. `confidence.class` und `confidence.score` sind **zwei** Felder; die Klasse wird nicht aus dem Score gerundet. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `causehypothesis_traegt_die_zehn_felder`; **A5** Negativfixture je Feld | Ein Objekt ohne `evidence_ids` wird angenommen; oder `class` wird aus `score` abgeleitet | Entwurf §36.3:2388–2409 |
| M-16 | Eine Hypothese wird gebildet | Der Befund trägt im **Datenmodell** die **sechs Teile** aus §8 „Ergebnisform": Ort (Bus, optional PRE/POST-Stelle), Beobachtung, Zusammenhang, Alternativen, Sicherheit, nächster Beweisschritt. Sie sind Datenmodell und Regelbasis, **keine** Vorgabe, sechs Werte anzuzeigen. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `sechs_teile_im_datenmodell` | Ein Teil fehlt im Objekt; oder die Anzeige leitet aus dem Objekt sechs sichtbare Werte ab | Entwurf §8:842–861; U21-Vorbemerkung (§1.6) |
| M-17 | Telemetrie zeigt Korrelation zweier Busse | **Aus paralleler Telemetrie allein entsteht nie Klasse 2 oder 3.** Die drei Aussageklassen aus §36.1 sind sichtbar getrennt: 1. Zusammenhang (gleicher Bereich, gleiches Zeitfenster), 2. Wirkungsbeleg (PRE/POST-Paar mit reproduzierbarer Veränderung), 3. kontrollierter Ursachenbeleg (begrenzte Preview an genau dieser Quelle). Ohne PRE/POST-Paar beziehungsweise Preview bleibt jede Aussage Klasse 1. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `parallele_telemetrie_bleibt_klasse_eins` | Ein Befund aus zwei gleichzeitig gemessenen Quellen trägt `claim_class` 2 oder 3 | Entwurf §36.1:2354–2365 |
| M-18 | Stufe A läuft (günstiges Screening) | Screening prüft aktive Quellen und **64 Bänder** auf zeitliche Überlappung, verbindet Master-Anomalien, Quellenergie, Onset-/Peak-Koinzidenz und musikalische Priorität und reicht **pro Befund höchstens die besten fünf Kandidaten** weiter. Der Deckel ist hart, nicht heuristisch. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `screening_reicht_hoechstens_fuenf_weiter` | Ein sechster Kandidat erreicht Stufe B; oder das Screening nutzt das 221er-Gitter statt der 64 Gruppen | Entwurf §36.2:2367–2386; Bandgitter `nakama_log64_v1` (64 Gruppen) |
| M-19 | Stufe B läuft (fokussierte Evidenz) | Stufe B lädt den **221-Band-Verlauf** und die relevanten Ereignisfenster nach, vergleicht **bedingten Uplift** (Zielmetrik bei aktiver Quelle gegen ähnliche Fenster ohne diese Aktivität), prüft Stabilität über **Block-Bootstrap**, alternative Erklärungen und Parent-/Child-Abhängigkeit und bezieht PRE/POST oder eine kontrollierte Preview als stärkere Evidenz ein. In P5 gibt es keine eigene Preview — der stärkere Zweig bleibt PRE/POST beziehungsweise ein manueller Versuch. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `bedingter_uplift_braucht_fenster_ohne_die_quelle` | Der Uplift wird ohne Vergleichsfenster gerechnet; oder eine Preview-Aktion entsteht in P5 | Entwurf §36.2:2367–2386; §42.2 Punkt 5 („in P5 alles manual", §1.1 Lieferumfang „noch ohne Fernänderung") |
| M-20 | Kandidaten liegen vor; der Rang wird gebildet | Der Rang entsteht aus **sechs getrennten Komponenten**: Bandpassung, zeitliche Koinzidenz, bedingter Uplift, Intent-Relevanz, Wiederholbarkeit, Routingqualität. **Keine einzelne hohe Komponente darf fehlende Coverage oder falsches Alignment kompensieren.** Das ist eine harte Gate-Regel vor der Gewichtung, kein Gewicht. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `keine_komponente_kompensiert_coverage_oder_alignment` — je ein Fall mit Maximalwert in genau einer Komponente bei gerissener Coverage beziehungsweise gerissenem Alignment | Ein Kandidat mit 1,0 in fünf Komponenten und gerissener Coverage erreicht eine starke Aussage | Entwurf §36.2:2367–2386 |
| M-21 | Ein **korrelierter, nicht kausaler Distraktor** liegt im Korpus | Der Distraktor erzeugt **keine starke** Ursachenbehauptung. Er darf als **Alternative** erscheinen (`alternatives`) — das ist Ziel 1 aus SONDE-013 M-85, das NAK-190 diesem Ticket zuweist. Enthaltung ist das gewünschte Ergebnis, nicht ein Treffer. | **BAULÜCKE** (Korpus trägt den Fall heute nicht; die Lücke ist gedruckt) | **NEU** `pruefe_p5_korpus.py` (anzulegen) mit dem Fall `korrelierter_distraktor_wird_alternative_nicht_ursache`; **A26** liest die Lücke heute aus `MANIFEST.json` | Der Distraktorfall erzeugt `aussage = stark`; oder er erscheint gar nicht in `alternatives` | NAK-190 (§1.8); G5-Absatz (§1.3); Entwurf §36.4:2411–2421 |
| M-22 | Ein **Parent-Duplikat** liegt vor (Bus und sein Parent messen dasselbe Signal) | Parent und Child erzeugen **nicht zwei** starke Behauptungen über dieselbe Ursache. Die Routingqualität aus M-20 trennt sie; bei unbekanntem Routing gibt es keine starke Aussage. | **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen), Fall `parent_duplikat_erzeugt_keine_zwei_starken`; **NEU** `EqCopSonde014HypotheseTest` | Parent und Child tragen beide `claim_class` 2 oder 3 für denselben Zielbefund | Entwurf §36.4:2411–2421; §36.1:2354–2365 |
| M-23 | Die Passage ist zeitlich **verschoben**, eine **andere** oder **zu kurz** | Keine davon kann einen **starken** Ursachenbeleg erzeugen. Verschoben und anders messen die bestehenden Gates aus `vergleichbarkeit.rs` (`GATE_ZEITUEBERDECKUNG` = 0,95, `GATE_QUELLEN_JACCARD` = 0,9, `GATE_MATERIAL_COSINE` = 0,95, `GATE_ABDECKUNG` = 0,5). **R1 (Matrixnacharbeit 1, 06.09.2026):** „zu kurz“ ist damit **nicht** gemessen — `ueberdeckung` (`vergleichbarkeit.rs`:168–185) rechnet **relativ** und normiert auf das kürzere Fenster; zwei identische, beliebig kurze Passagen bestehen alle vier Gates. „Zu kurz“ wird deshalb als **gemessene Mindestzahl unabhängiger Evidenzfenster je beteiligter Quelle innerhalb der Passage** definiert — nicht als Wanddauer, konsistent mit `vergleichbarkeit.rs`:108–109 („GEMESSENE Signalabdeckung … nicht die Wanddauer“). Die Zahl steht als **eine** benannte Konstante `GATE_MINDEST_FENSTER` **neben** den vier bestehenden Gates in derselben Datei; es entsteht **keine zweite Schwellenfamilie**. **Startwert 8**, begründet aus der Evidenzkadenz: `kEvidenzIntervallMinS` = 0,25 s liefert bis zu vier Fenster je Sekunde, bei Kadenzreduktion eines; acht Fenster sind damit 2 bis 8 s und geben dem Block-Bootstrap (`BOOTSTRAP_BLOCK` = 4) mindestens zwei Blöcke — dieselbe Untergrenze, die SONDE-013 M-11 für die Welch-Mittelung setzt. Der Wert wird mit dem P5-Korpus kalibriert (M-64 bis M-67); die Korpusfälle, die ihn festziehen, sind die Sessions „zu kurze Passage“ und „verschobene Passage“ aus M-65. **Unter der Schwelle erreicht kein Befund die Sicherheit `hoch` und den Zustand `READY TO SEND`; das Ergebnis ist `MORE DATA` mit dem Ausschlussgrund `passage_zu_kurz`** (M-87). | **BELEGT** · **ENTSCHIEDEN** (R1, §4.13) · **BAULÜCKE** | **NEU** `EqCopSonde014HypotheseTest`, drei Fälle (verschoben, andere, zu kurz); **NEU** `A4`-Verdrahtungsfall für `GATE_MINDEST_FENSTER`; **A4** `vergleichbarkeit` bestehend | Zwei Rotbeweise, beide mit einem **ansonsten zulässigen P5-Ergebnis** (`claim_class` innerhalb der P5-Menge, Sicherheit hoch), damit sie am **Passagengate** fallen und nicht am Preview-Verbot aus M-45: (1) dieselbe Session mit einer Passage **unter** `GATE_MINDEST_FENSTER` liefert `READY TO SEND`; (2) eine um 30 % verschobene Passage liefert Sicherheit hoch | R1 (§4.13); Entwurf §36.4:2411–2421; `broker/src/coordinator/vergleichbarkeit.rs`:108–109,152–160,168–185 |
| M-24 | Eine Evidenz wird zurückgenommen (`evidence_invalidate`) | **Entfernen eines Evidenzobjekts invalidiert abhängige Hypothesen deterministisch.** Reihenfolge unter **einem** Lock: Umfang auflösen (`Ids`, `Bereich`, `GanzeSitzung`) → betroffene Evidenz-IDs markieren → jede Hypothese, deren `evidence_ids` eine davon enthält, terminal invalidieren → Projektion in `findings` → Outbox. „Deterministisch" heißt: dieselbe Rücknahme in anderer Reihenfolge trifft dieselbe Menge. | **BAULÜCKE** | **NEU** `EqCopSonde014HypotheseTest`, Fall `ruecknahme_invalidiert_abhaengige_hypothesen`; **A4** Verdrahtungsfall über Store und Neustart | Eine Hypothese überlebt die Rücknahme ihrer letzten Evidenz; oder zwei Reihenfolgen derselben Rücknahme treffen verschiedene Mengen | Entwurf §36.4:2411–2421; `broker/src/coordinator/invalidierung.rs`:64,96 |
| M-25 | Gleiche Eingaben, derselbe Build, dieselbe ISA | Das Ranking ist **bytegleich**. Dafür gehören **Bootstrap-Seed**, **Float-Reduktionsreihenfolge**, **kanonische Eingangsquantisierung** und ein **stabiler Tie-Break-Key** zur `metrics_version`. Plattformübergreifend gelten identische Rangfolge und festgelegte numerische Toleranzen statt unrealistischer Bytegleichheit. | **BELEGT** · **BAULÜCKE** (Tie-Break-Key existiert nicht) | **NEU** `EqCopSonde014HypotheseTest`, Fall `ranking_ist_bytegleich_ueber_hundert_laeufe`; **A5** erweitert um den Tie-Break-Key in `metriken-v1.json` | Zwei Läufe auf demselben Eingang liefern verschiedene Reihenfolgen bei Gleichstand; oder der Seed steht nicht in der `metrics_version` | Entwurf §36.4:2411–2421; `broker/src/coordinator/vergleichbarkeit.rs`:149 |
| M-26 | Zwei Kandidaten liegen praktisch gleich | **Beide werden gezeigt.** Der Tie-Break entscheidet nur die Anzeigereihenfolge, nicht die Auswahl; er ist stabil und Teil der `metrics_version`. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `gleichstand_zeigt_beide` | Bei Gleichstand verschwindet ein Kandidat; oder die Reihenfolge wechselt zwischen zwei Läufen | Entwurf §36.3:2388–2409 |
| M-27 | Die Datenlage reicht nicht | **„Mehr Daten nötig" ist ein reguläres Ergebnis** und kein Fehlerzustand: es hat eine eigene Ausprägung im Objekt, eine eigene Ursachenklasse (§8, siebte) und wird im Korpus als **Enthaltung** gezählt, nicht als Fehlschlag. | **BELEGT** | **NEU** `pruefe_p5_korpus.py` (anzulegen) plus `EqCopSonde014HypotheseTest` | „Mehr Daten" erzeugt einen Fehlercode statt eines Ergebnisses; oder es zählt im Korpus als falsche Aussage | Entwurf §36.3:2388–2409; §8:842–861; `tools/eq-copilot/pruefe_p4_korpus.py`:91,222 |
| M-28 | Eine Behauptung wird sichtbar | **Jede sichtbare Behauptung referenziert existente Evidenz-IDs** (Exit-Gate, wörtlich). „Existent" ist eine Prüfung gegen den Store, nicht gegen den Speicher: eine ID, deren `evidence`-Zeile fehlt oder ausgeschlossen ist, macht die Behauptung unsichtbar beziehungsweise `STALE` — sie wird nie ohne Beleg gezeigt. | **BELEGT** · **BAULÜCKE** | **NEU** `EqCopSonde014HypotheseTest`, Fall `behauptung_ohne_existente_evidenz_wird_nicht_sichtbar`; **A4** Storefall nach Neustart | Ein Befund mit einer gelöschten Evidenz-ID bleibt sichtbar und handelbar | Entwurf §59 Exit-Gate (§1.1); §34.2 |
| M-87 | Ein Kandidat scheidet aus dem Ranking einer Hypothese aus | **R4 (Matrixnacharbeit 1, 06.09.2026).** Entwurf §59 Lieferumfang Punkt 2 verlangt ausdrücklich **Ausschlussgründe**; bis zu dieser Nacharbeit maß keine Zeile ihre Ausgabe oder Erhaltung. **Jeder** Kandidat, der ausscheidet, trägt einen Grund aus einer **geschlossenen Menge**: `coverage_fehlt`, `alignment_falsch`, `passage_unvergleichbar`, `passage_zu_kurz` (M-23), `intent_veto_geschuetzt`, `intent_veto_verschmolzen` (M-04), `capability_fehlt`, `evidenz_zurueckgenommen` (M-24). Die Menge lebt an **einer** Stelle im Vertrag; C++ und Rust prüfen dieselbe (M-77). Die Ausschlüsse reisen **mit** der Hypothese — `ausschluesse` als Liste aus `candidate_source` und `grund`, Teil des Befunds in `session_snapshot.findings` (E-04) — und werden im Store beim Finding **erhalten**. **Ein kommentarlos entfernter Kandidat ist ein Defekt.** Ob und wie Gen die Ausschlüsse zeigt, ist Anzeige (S31b) und wird hier nicht entschieden; der Datenweg liefert sie. | **BELEGT** · **ENTSCHIEDEN** (R4, §4.13) · **BAULÜCKE** | **NEU** `EqCopSonde014HypotheseTest` **und** ein **A4**-Verdrahtungsfall (beidseitig, M-77); **A5** für die geschlossene Menge samt Negativfixture; **A4** Storefall für die Erhaltung nach Neustart | Ein Kandidat wird **ohne Grund** aus dem Ranking entfernt; oder ein Grund **außerhalb** der Menge wird von einer der beiden Seiten angenommen; oder die Ausschlüsse fehlen nach einem Neustart am Finding | R4 (§4.13); Entwurf §59 Lieferumfang Punkt 2 (§1.1, Zeile 4311); §36.3:2388–2409 |

### 3.3 Befundzustände nach U21 (Gate: Abnahme 02.09.2026, Fünferblock 03, 1/5)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-29 | Ein Befund liegt vor; sein Zustand wird bestimmt | Die drei Sicherheitsstufen des Entwurfs (**hoch, mittel, unklar**) werden im Datenweg auf **genau drei Zustände** abgebildet: `READY TO SEND`, `MORE DATA`, `STALE`. Die Abbildung ist eine **Funktion im Datenweg**, keine Anzeigeentscheidung; die UI liest den Zustand und rät nie. | **BELEGT** | **NEU** `EqCopSonde014BefundTest`, Fall `sicherheit_wird_auf_drei_zustaende_abgebildet` | Die UI leitet den Zustand aus einem Zahlenwert ab; oder ein vierter Zustand entsteht | U21-Folge (§1.6); `docs/plan/plan.json` S23–25 Nachtrag 02.09. (1) |
| M-30 | Der Befund steht auf `READY TO SEND` | **Nur** dieser Zustand erlaubt `HOLD TO AUDITION` und `SEND DRAFT → EQ`. `MORE DATA` und `STALE` bieten **keinen Draft** an — die Sperre liegt im Datenweg (die Aktion ist nicht verfügbar), nicht in einer ausgegrauten Schaltfläche, die trotzdem sendet. | **BELEGT** | **NEU** `EqCopSonde014BefundTest`, Fälle `more_data_bietet_keinen_draft` und `stale_bietet_keinen_draft` | Ein `MORE DATA`-Befund erzeugt ein `draft_offer`; oder die Sperre existiert nur in der Anzeige | U21-Folge (§1.6); Blueprint §4.1 „Befund-Dichte" |
| M-31 | Die Schwelle hoch/mittel/unklar → handelbar soll festgelegt werden | Die Schwelle wird **mit dem Prüfkorpus kalibriert, nie in der UI geraten**. Sie lebt versioniert in `metrics_version` und wird gegen den Korpus gemessen (Precision, Recall, Brier, Kalibrierung); eine Änderung braucht eine neue Metrikversion und einen Regressionslauf. | **BELEGT** · **BAULÜCKE** (Korpus für Ursachen fehlt) | **NEU** `pruefe_p5_korpus.py` (anzulegen): die Schwelle ist Ausgabe des Beins, nicht seine Eingabe | Die Schwelle steht als Literal im Editor; oder sie ändert sich ohne Metrikversionsschritt | `docs/plan/plan.json` S23–25 Nachtrag 02.09. (1); U21-Folge |
| M-32 | Ein zweiter möglicher Verursacher wird gefunden | **Alternativen sind eigene Befunde in der Liste mit eigenem Zustand**, kein Feld im fokussierten Befund. Das Feld `alternatives` aus §36.3 bleibt im Datenmodell (M-15) und trägt die **IDs** dieser eigenen Befunde; es trägt keinen Text und keine Zweitmeinung. | **BELEGT** | **NEU** `EqCopSonde014BefundTest`, Fall `alternative_ist_ein_eigener_befund_mit_eigenem_zustand` | `alternatives` trägt einen Freitext statt Befund-IDs; oder ein alternativer Verursacher erscheint nur als Anhang | U21-Folge (§1.6); `docs/plan/plan.json` S23–25 Nachtrag 02.09. (2) |
| M-33 | Der Beleg eines Befunds soll gezeigt werden | **Beleg ist die markierte Frequenzzone mit Ziel- und Referenzkurve**, nicht ein wiederholter Text. Der Datenweg liefert dafür Frequenzbereich und die beiden Kurvenidentitäten; er liefert **keinen** Belegtext. | **BELEGT** | **NEU** `EqCopSonde014BefundTest`, Fall `beleg_ist_zone_und_zwei_kurven_kein_text` | Der Befund trägt ein Belegtextfeld; oder die Zone kommt ohne benannte Referenzkurve | U21-Folge (§1.6); `docs/plan/plan.json` S23–25 Nachtrag 02.09. (3) |
| M-34 | Der Befund wird auf Gen Fläche 1 dargestellt | Der Datenweg liefert genau die drei Zeilen `LIKELY CAUSE`, `SMALLEST TEST`, `LISTEN FOR` als **Datenfelder des Befunds** — nicht als Anzeigezusammenfassung, die die UI selbst baut. `SMALLEST TEST` bildet `next_test` ab, `LISTEN FOR` das Hörziel des zugehörigen Proposals beziehungsweise des nächsten Beweisschritts. | **BELEGT** | **NEU** `EqCopSonde014BefundTest`, Fall `drei_zeilen_sind_datenfelder` | Die UI setzt eine der drei Zeilen aus mehreren Feldern zusammen; oder eine vierte Zeile entsteht im Datenweg | U21-Folge (§1.6); Blueprint §4.1 |
| M-35 | `CONFIDENCE` steht unter dem Graphen | `CONFIDENCE` ist die **Messqualität der Passage**, nicht die Sicherheit des Befunds. Beide dürfen nicht vermischt werden: die Passagenqualität kommt aus `konfidenz.klasse` des Belegs, der Befundzustand aus M-29. Zwei Felder, zwei Quellen, nie ein gemeinsames. | **BELEGT** | **NEU** `EqCopSonde014BefundTest`, Fall `messqualitaet_und_befundsicherheit_sind_zwei_felder` | Ein Befund mit `konfidenz.klasse = stark` steht deshalb auf `READY TO SEND`; oder umgekehrt | U21-Folge (§1.6); Blueprint §4.1, Zone 3 |

### 3.4 Maskierungs-Datenweg nach U16 und PR2 (Gate: Abnahmen 30.08. und 02.09.2026)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-36 | Ein Befund über eine Maskierung zweier Quellen entsteht | Die Maskierung **fällt aus dem Befund heraus** als **laufender, je Frequenzbereich auflösbarer Wert** — nicht nur als fertiger Satz mit Ort und Beleg. „Je Frequenzbereich auflösbar“ heißt: der Wert trägt einen Bandindex beziehungsweise ein Bandintervall des bestehenden Gitters, keine neue Frequenzachse. **E-04:** der Wert ist ein optionales Objekt `maskierung` **am Befund** (`quelle_a`, `quelle_b`, `band_von`, `band_bis`, `wert_db`, `gueltig`, `herabgesetzt`), **keine zweite Liste** — zwei Listen könnten auseinanderlaufen. | **BELEGT** · **ENTSCHIEDEN** (E-04, §4.4) · **BAULÜCKE** | **NEU** `EqCopSonde014MaskierungTest`, Fälle `maskierungswert_benennt_einen_frequenzbereich` und `maskierung_haengt_am_finding` | Der Wert ist ein Skalar ohne Frequenzbezug; er führt ein eigenes Gitter; oder er reist in einer eigenen Liste neben den Befunden | E-04 (§4.4); U16-Folge (§1.5); Bauaufteilung Zeile 394 (§1.3); `eq-copilot/schemas/v3/bandgitter/` |
| M-37 | Die Sonden messen; die Maskierung soll laufend gezeigt werden | **E-03, entschieden 06.09.2026.** Der Wert entsteht im Broker-Coordinator **bei jedem Evidenz-Eingang einer der beiden Quellen** und reist über den Rückweg aus E-04. Die Zusage lautet: bei jedem **schweren Editortakt (250 ms)** liegt der Wert so aktuell vor wie die jüngste Evidenz beider Quellen; im Normalbetrieb (`kEvidenzIntervallMinS` = 0,25 s) ist er **höchstens ein Evidenzintervall plus Transportlatenz** alt. Bei P1-Rückstau folgt er der bestehenden Kadenzreduktion bis 1 Hz und trägt dann das Kennzeichen `herabgesetzt` — ein Feld im Wert, **nie** ein interpolierter Zwischenwert. Paint-FPS ist nicht Datenkadenz. | **BELEGT** · **ENTSCHIEDEN** (E-03, §4.3) · **BAULÜCKE** | **NEU** `EqCopSonde014MaskierungTest`, Fall `maskierungswert_ist_hoechstens_ein_evidenzintervall_alt` — gemessen wird das **Alter relativ zur jüngsten Evidenz**, nicht eine Wallclock-Rate — sowie `rueckstau_setzt_das_kennzeichen_herabgesetzt` | Der Wert entsteht nur beim Neuberechnen eines Befunds; oder bei Rückstau wird interpoliert, statt `herabgesetzt` zu setzen | E-03 (§4.3); `eq-copilot/plugin/core/analysis/FeatureEngine.h`:989–990; `eq-copilot/plugin/src/WorkerCadence.h`:27–28 |
| M-38 | Ein Finding ist ausgewählt | Das gewählte Finding **markiert seinen Frequenzbereich im stabilen Gesamtgraph**, ohne dessen Größe, Achsen oder Ausschnitt zu ändern. Der Datenweg liefert dafür nur den Bereich; er löst **keine** Achsen- oder Ausschnittsänderung aus. | **BELEGT** | **NEU** `EqCopSonde014MaskierungTest`, Fall `markierung_aendert_keine_achse` | Der Datenweg liefert eine Achsen- oder Zoomangabe mit | PR2-Nachtrag (§1.4); `design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md` („Stabile Geometrie", „Graphreaktion") |
| M-39 | Der User zoomt auf den markierten Bereich | Ein Zoom ist **nur als bewusste und reversible Aktion** zulässig und **darf nicht aus dem Datenweg heraus ausgelöst werden**. Der Datenweg kennt kein Zoomkommando. | **BELEGT** | **NEU** `EqCopSonde014MaskierungTest`, Fall `datenweg_kennt_kein_zoomkommando` | Eine Befundänderung erzeugt eine Zoomanforderung | PR2-Nachtrag (§1.4) |
| M-40 | Der Maskierungswert soll die Leitung erreichen | **E-04, entschieden 06.09.2026: ja, ein eigenes v3-Feld — aber am Rückweg, nie an der Sondenmessung.** Kein Feld am `evidence_snapshot`. `session_snapshot` bekommt als **Wire-Envelope-`schema_minor` 3** genau **ein** neues optionales Feld `findings`: die Befunde mit Zustand (`READY TO SEND` \| `MORE DATA` \| `STALE`), den Datenfeldern aus M-15 und M-16 (sechs Teile, drei Anzeigezeilen, `evidence_ids`, Intent-Revision) und je Befund dem optionalen Objekt `maskierung`. Der Name lautet `maskierung`, **nicht** `ueberdeckung` (Kollision, §2.12). Abwesenheit heißt „nichts“, nie `null`; ein Leser der Fassung 2 sieht das Feld nicht. Der Name wird in diesem Ticket als **belegtes Feld** in `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` eingetragen und in `eq-copilot/schemas/v3/README.md` als Fassungsschritt geführt — derselbe Weg wie `experimente` und `paare` in SONDE-013. | **ENTSCHIEDEN** (E-04, §4.4) · **BAULÜCKE** | **NEU** `EqCopSonde014MaskierungTest`; **A5** um Negativfixture und **Fassungsleiter** erweitert (ein zurückgebauter Leser der Fassung 2 lehnt `findings` ab); Cross-Language-Fixtures für C++ und Rust über **A8** | Ein Leser der Fassung 2 nimmt ein `findings` an; das Feld entsteht ohne `schema_minor`-Schritt; oder Abwesenheit wird als `null` gelesen | E-04 (§4.4); `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` (`regel`, `wire_envelope_schema_minor`) |
| M-41 | Die farbige Zone wird gezeichnet | Die Zone **zeigt nur, was der Befund belegt hat, und setzt keine eigene Schwelle**: sie ist keine zweite Wahrheit neben dem Befundtext und keine eigene Detektion. **E-04** hält das strukturell: die Zone liest `maskierung` **am** Befund, und es gibt keine zweite Liste, von der sie abweichen könnte. Die Zone selbst gehört zu **S31b**; dieses Ticket liefert ausschließlich den Datenweg. | **BELEGT** | **NEU** `EqCopSonde014MaskierungTest`, Fall `zone_hat_keine_eigene_schwelle` (der Datenweg liefert keine Schwelle) | Der Datenweg trägt eine Anzeigeschwelle; oder eine Maskierung existiert ohne Befund | E-04 (§4.4); U16-Folge (§1.5); Blueprint P5-Bedienvertrag („keine eigene Detektion und keine konkurrierende Schwelle“) |

### 3.5 Proposal-Policy nach §42 (Gate: §59 Punkt 3, §42, NAK-168)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-42 | Ein Befund ist handelbar; ein Vorschlag entsteht | Eine Empfehlung ist **zuerst ein validiertes, versioniertes `Proposal`** und erst danach Text. Die Felder aus §42.1: `proposal_id`, `proposal_schema`, `target`, `base_revision`, `passage_id`, `action`, `parameters`, `allowed_bounds`, `evidence_ids`, `expected_effect`, `protected_traits`, `listen_for`, `stop_if`, `execution`, `confidence`. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `proposal_traegt_die_fuenfzehn_felder`; **A5** Negativfixture je Feld | Ein Proposal ohne `stop_if` oder ohne `allowed_bounds` wird angenommen | Entwurf §42.1:2762–2787 |
| M-43 | Ein Proposal wird sichtbar | Das Exit-Gate verlangt wörtlich, dass **jedes Proposal Ziel, Passage, Grenzen, Hörziel, Stopbedingung und Rückweg nennt**. Abbildung: Ziel → `target`, Passage → `passage_id`, Grenzen → `allowed_bounds`, Hörziel → `listen_for`, Stopbedingung → `stop_if`, **Rückweg** → **E-06, entschieden 06.09.2026:** ein Feld `revert` mit der geschlossenen Menge `experiment_abort` \| `manual_only` \| `none_needed`. `dsp_revert` gehört P7 (`revert_transaction`, SONDE-016) und ist in P5 ein Vertragsbruch wie `execution = previewable` (M-45). Der Rückweg ist ein **Feld**, kein Versprechen im Text. | **BELEGT** · **ENTSCHIEDEN** (E-06, §4.6) · **BAULÜCKE** | **NEU** `EqCopSonde014ProposalTest`, Fälle `sechs_gate_felder_sind_pflicht` und `revert_hat_drei_werte_je_proposal_typ`; die Ablehnung eines vierten Werts beidseitig in C++ und Rust (M-77) | Ein Proposal ohne `revert` wird sichtbar; ein vierter `revert`-Wert wird von einer der beiden Seiten angenommen; oder `dsp_revert` erreicht ein P5-Proposal | E-06 (§4.6); Entwurf §59 Exit-Gate (§1.1); §43.1:2837–2865 |
| M-44 | Ein Proposal wird erzeugt | Die Erzeugung ist **deterministisch** und läuft in genau dieser Reihenfolge (§42.2): 1. Befundtyp wählt eine geprüfte Aktionstemplate; 2. ein Constraint-Solver entfernt Aktionen, die **Intent, Capability, Headroom, Messqualität oder Schutzbereiche** verletzen; 3. für EQ eine regularisierte Zielfunktion (gewünschte Evidenzverbesserung bei möglichst wenig Bändern, Gain und spektraler Nebenwirkung); 4. **der kleinste sichere Kandidat gewinnt**, Alternativen bleiben sichtbar; 5. `execution` wird gesetzt. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fälle `derselbe_eingang_erzeugt_denselben_entwurf` (100 Läufe) und `kleinster_sicherer_kandidat_gewinnt` | Zwei Läufe auf demselben Eingang liefern verschiedene Parameter; oder ein größerer Kandidat gewinnt bei gleicher erwarteter Wirkung | Entwurf §42.2:2789–2804; §42.4:2822–2833 |
| M-45 | Eine Aktion würde eine eigene DSP-Fähigkeit brauchen | **In P5 ist jede Aktion `manual`, nichts ist `previewable`.** `previewable` setzt exakt unterstützte eigene DSP-Aktionen voraus (§42.2 Punkt 5), und der aktive Kern entsteht erst in P6 (`SONDE-015`). Ein Proposal mit `execution = previewable` ist in P5 ein Vertragsbruch, kein optimistischer Vorgriff. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `in_p5_ist_jede_aktion_manual` | Ein erzeugtes Proposal trägt `execution = previewable` | Entwurf §42.2 Punkt 5:2802–2803; §59 „noch ohne Fernänderung" (§1.1); Bauaufteilung P6-Zeile |
| M-46 | Kein sicherer Eingriff ist begründbar | **`Keine Änderung` ist ein gültiger Vorschlag**, ebenso **`mehr Daten`**. Beide sind reguläre Ergebnisse mit vollständigem Objekt (Ziel, Passage, Hörziel, Stopbedingung, Rückweg), nicht ein leerer Rückgabewert. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fälle `keine_aenderung_ist_ein_vorschlag` und `mehr_daten_ist_ein_vorschlag` | Der Erzeuger liefert `None` statt eines Objekts; oder eines der beiden Ergebnisse trägt keine Evidenz-IDs | Entwurf §59 Lieferumfang (§1.1); §42.1:2762–2787 |
| M-47 | Ein Proposal wird gebildet (**NAK-168, Hälfte 1**) | **Der Vorschlag benennt den Zielbereich.** Das Band beziehungsweise Bandintervall, das der Eingriff adressiert, kommt aus dem Befund (`band_hz` aus §36.3) und steht im Proposal (`parameters.frequency_hz` plus `allowed_bounds.frequency_hz`). Es wird **nicht** aus einem Messdelta geraten. | **BELEGT** · **BAULÜCKE** | **NEU** `EqCopSonde014ProposalTest`, Fall `zielbereich_kommt_aus_dem_befund_nicht_aus_dem_delta` | Der Zielbereich wird aus dem größten Betrag der Banddeltas abgeleitet | NAK-168 (§1.8); Entwurf §36.3:2388–2409; §42.1:2762–2787 |
| M-48 | Ein Experiment liest den Zielbereich (**NAK-168, Hälfte 2**) | **E-05, entschieden 06.09.2026.** `experiment_begin` bekommt in derselben **Fassung 3** ein optionales Objekt `ziel` (`band_von`, `band_bis`, `geschuetzte_baender` als Liste von Bandintervallen, `proposal_id` als Rückverweis). **Mit `ziel`** liest der Guardrail-Rechner Ziel- und Schutzbereiche und rät nichts. **Ohne `ziel`** — der manuelle Versuch ohne Vorschlag, der SONDE-013-Pfad — bleibt die heutige Heuristik (`experiment_verdrahtung.rs`:1069–1088) und das **Resultat trägt das Kennzeichen `ziel_geraten`**. Beide Pfade werden gemessen. | **ENTSCHIEDEN** (E-05, §4.5) · **BAULÜCKE** | **NEU** `EqCopSonde014ProposalTest` plus **A4**-Verdrahtungsfall für beide Pfade; **A5** Negativfixture für `ziel` | Zwei Rotbeweise: die Heuristik läuft **trotz** gesetztem `ziel`; und ein Resultat **ohne** `ziel` trägt kein `ziel_geraten` | E-05 (§4.5); NAK-168 (§1.8); `broker/src/coordinator/experiment_verdrahtung.rs`:1069–1088 |
| M-49 | Guardrails werden ausgewertet | Eine geschützte Eigenschaft erscheint als **harte Constraint** im Vorschlag (`protected_traits`) und im DSP-ACK. In P5 gibt es kein DSP-ACK — die Hälfte, die dieses Ticket bauen kann, ist die **Vorschlagsseite**; die ACK-Seite gehört S26–28/S29–31 und wird hier nicht vorgegriffen. | **BELEGT** (Vorschlagsseite) · **OFFEN für P7** | **NEU** `EqCopSonde014ProposalTest`, Fall `geschuetzte_eigenschaft_ist_harte_constraint` | Ein Proposal überschreitet einen geschützten Bereich; oder `protected_traits` bleibt leer, obwohl der Intent einen Schutz führt | Entwurf §37.4:2467–2473; §42.1:2762–2787 |
| M-50 | Guardrail-Skalare fehlen im Vertrag | **Ein Guardrail ohne Vertragsteil ist nicht messbar und wird als „nicht gemessen" ausgewiesen, nie als „unverändert".** `guardrail_loudness_db` und `guardrail_peak_db` sind heute hart `None` (NAK-204), `seitenanteil_db` erreicht den Broker strukturell nie (NAK-193). Ein Proposal, dessen `stop_if` auf einen nicht messbaren Guardrail zeigt, ist **nicht handelbar** und meldet `MORE DATA`. | **BAULÜCKE** (fremde Registerzeilen, hier nur verkettet) | **NEU** `EqCopSonde014ProposalTest`, Fall `stop_if_auf_nicht_messbarem_guardrail_ist_nicht_handelbar`; **A4** bestehend für `guardrail_nicht_gemessen` | Ein Proposal steht auf `READY TO SEND`, obwohl seine Stopbedingung auf `guardrail_peak_db` zeigt | NAK-204, NAK-193 (§1.8); `broker/src/coordinator/experiment_verdrahtung.rs`:1039–1042 |
| M-51 | Ein Proposal wird angezeigt | **Jeder angezeigte Zahlenwert lässt sich auf Proposal-Feld, Evidenz und Generatorversion zurückführen.** Kein Zahlenwert entsteht in der Anzeige. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `jede_zahl_hat_feld_evidenz_und_generatorversion` | Ein angezeigter Wert hat kein Feld im Objekt; oder die Generatorversion fehlt | Entwurf §42.4:2822–2833 |
| M-52 | Ein Proposal ist veraltet, ungültig oder außerhalb der Capability | Solche Vorschläge **erreichen keine Probe**. In P5 heißt das: sie werden gar nicht erst als `draft_offer` gebildet; der Zustand ist `STALE` beziehungsweise `MORE DATA`. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, drei Fälle (veraltet, ungültig, Capability fehlt) | Ein Proposal mit veralteter `base_revision` wird gesendet | Entwurf §42.4:2822–2833 |
| M-53 | Sicherheitsbudgets werden angewendet | Die Startwerte aus §42.3 gelten als **Produktgrenzen**: höchstens ein musikalischer Eingriff pro Assistentenschritt, höchstens drei EQ-Bänder pro Vorschlag; statischer EQ innerhalb ±3 dB; dynamische Reduktion standardmäßig höchstens 1,5 dB, Hard-Cap 3 dB; kein Lookahead, keine positive automatische Gesamtverstärkung, keine versteckte Normalisierung; **engere Usergrenzen gewinnen immer**. Jede Änderung braucht eine neue Policy-/Metrikversion und Regressionstests. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Property-Test über zufällige Eingänge: `hard_caps_und_engeres_userbudget_werden_nie_ueberschritten` | Ein erzeugtes Proposal überschreitet ein Hard Cap; oder eine engere Usergrenze wird von einer Produktgrenze geschlagen | Entwurf §42.3:2806–2820; §42.4:2822–2833 |
| M-54 | Ein Proposal wird gebildet | Jedes Proposal **referenziert Evidenz-IDs und die Intent-Revision**. Ohne beide entsteht kein Objekt; die Revision ist die, gegen die der Constraint-Solver gelaufen ist, nicht die zum Anzeigezeitpunkt. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `evidenz_ids_und_intent_revision_sind_pflicht` | Ein Proposal ohne `evidence_ids`; oder die Revision wird beim Anzeigen nachgezogen | Entwurf §37.3:2461–2465; §59 Exit-Gate (§1.1) |

### 3.6 `AssistantStep` (Gate: §59 Punkt 4 und 5, §46.1, Entwurf Zeile 3177/3182)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-55 | Ein aktives Problem wird bearbeitet | Der Assistent ist **eine deterministische Zustandsmaschine**, kein frei handelnder Chat. Die Folge aus §46.1 lautet `Coverage → Finding → Evidence → Listen → Proposal → Preview → Remeasure → Verdict`. **E-07, entschieden 06.09.2026:** `Preview` **bleibt in der Zustandsmenge**, die P5-Übergangstabelle führt **keine Kante** dorthin, und ein gespeicherter `AssistantStep` mit `Preview` ist in P5 ein **Lesefehler**, kein stiller Sprung. Die P5-Folge ist damit `Coverage → Finding → Evidence → Listen → Proposal → Remeasure → Verdict`. | **BELEGT** · **ENTSCHIEDEN** (E-07, §4.7) | **NEU** `EqCopSonde014AssistentTest`, Fälle `p5_uebergangstabelle_hat_keine_kante_nach_preview` und `gespeicherter_preview_schritt_ist_ein_lesefehler` | Ein Schritt erreicht `Preview`; der Zustandsname verschwindet aus dem Vertrag; oder ein gespeicherter `Preview`-Schritt wird still auf `Proposal` oder `Remeasure` abgebildet | E-07 (§4.7); Entwurf §46.1:3172–3184 |
| M-56 | Ein Zustand wird betreten | **Jeder Zustand besitzt Eintrittsbedingungen, Evidenz-IDs, Useraktion, Timeout und sichere Rückkante.** Fünf Angaben je Zustand, keine optional. Ein Zustand ohne Rückkante ist ein Vertragsbruch. | **BELEGT** | **NEU** `EqCopSonde014AssistentTest`, Fall `jeder_zustand_traegt_fuenf_angaben` (Tabellentest über alle sieben) | Ein Zustand ohne Timeout oder ohne Rückkante wird betreten | Entwurf §46.1:3172–3184 |
| M-57 | Ein Versuch läuft | **Es ist immer höchstens ein klanglicher Versuch aktiv.** Der Deckel ist strukturell (ein Slot), nicht ein Zähler, der auch 2 tragen könnte. Ein zweiter Startversuch wird abgewiesen, nicht eingereiht. | **BELEGT** | **NEU** `EqCopSonde014AssistentTest`, Fall `hoechstens_ein_klanglicher_versuch`; **A4** bestehend (`experiment.rs::offene`) | Zwei Versuche sind gleichzeitig offen; oder der zweite wird still eingereiht | Entwurf §46.1:3172–3184 |
| M-58 | Der User bricht ab, geht zurück, überspringt oder nimmt wieder auf | **Alle vier Gegenpfade existieren und sind persistent.** `Abbruch` ist terminal (`experiment_abort`, „Verwerfen ist ein terminales Ereignis, kein Löschen der Historie"), `Zurück` und `Überspringen` verändern den Zustand innerhalb desselben Schritts, `Resume` setzt an derselben belegten Stelle fort. Reihenfolge beim Abbruch: Terminalereignis anhängen → Projektion → Outbox → erst danach den Slot freigeben. | **BELEGT** | **NEU** `EqCopSonde014AssistentTest`, vier Fälle; **A4** Storefall | Ein Abbruch gibt den Slot vor dem Append frei; oder `Zurück` erzeugt ein zweites Experiment | Entwurf §46.1:3172–3184; §43.1:2837–2865 |
| M-59 | Main startet neu | Ein abgebrochener Schritt kann **aus dem gespeicherten `AssistantStep` rekonstruiert oder per `experiment_abort` endgültig verworfen** werden. Reihenfolge beim Start: `MainProject` lesen → `AssistantStep` rekonstruieren → Store abgleichen → **erst danach** eine Aktion anbieten. Der Store gewinnt bei Widerspruch nie gegen einen neueren Plugin-State (§33.5). | **BELEGT** · **BAULÜCKE** | **NEU** `EqCopSonde014AssistentTest` plus **B14** erweitert; **A4** Neustartfall mit OFFENEM Schritt | Nach dem Neustart bietet der Assistent eine Aktion an, bevor der Schritt rekonstruiert ist; oder ein alter Store-Eintrag setzt den Plugin-State zurück | Entwurf §46.1:3172–3184; §33.5:2140–2175 |
| M-60 | Der Assistent priorisiert | Ein **deterministisches Ranking** berücksichtigt erwarteten Nutzen, Intent-Relevanz, Konfidenz, Reversibilität, Messkosten und **bereits erfolglose Versuche**. **Harte Gates entfernen nicht vergleichbare oder unsichere Schritte** vor der Gewichtung. Der Assistent beginnt mit dem kleinsten hochrelevanten, reversiblen Test. | **BELEGT** | **NEU** `EqCopSonde014AssistentTest`, Fall `harte_gates_greifen_vor_der_gewichtung` | Ein nicht vergleichbarer Schritt erreicht die Gewichtung; oder ein erfolgloser Versuch wird erneut als bester vorgeschlagen | Entwurf §46.2:3186–3200 |
| M-61 | Kein sinnvoller Eingriff liegt an | Der Assistent kann ausdrücklich **`erst Passage messen`, `Routing bestätigen` oder `keine Änderung empfohlen`** sagen. Diese drei sind eigene, benannte Ergebnisse mit Objekt, keine Leerzustände. | **BELEGT** | **NEU** `EqCopSonde014AssistentTest`, drei Fälle | Einer der drei erscheint als leerer Zustand ohne Objekt | Entwurf §46.2:3186–3200 |
| M-62 | Ein manueller Versuch wird abgeschlossen | Der Anschluss an SONDE-013 ist **der bestehende Weg**: `experiment_begin` → `experiment_candidate` → `experiment_manual_result` beziehungsweise `experiment_abort`. Der Assistent erzeugt **keine** neue Experimentfamilie; er ruft die vorhandenen und liest `Achsenrechnung`. Das Paarurteil (`Paarurteil`, `Dreifachergebnis`) bleibt der PRE/POST-Weg. | **BELEGT** | **NEU** `EqCopSonde014AssistentTest` plus **B25** `EqCopSonde013ExperimentGoldenTest` und **A4** bestehend | Der Assistent erzeugt eine eigene Experimentnachricht; oder er schreibt in den Experimentstore, ohne den Coordinator zu rufen | Entwurf §59 „manueller Busvorschlag und manueller Experimentabschluss"; `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` |
| M-63 | Ein **manueller Busvorschlag** soll entstehen | Der Busvorschlag ist ein `Proposal` mit `execution = manual` und **neutralen Parameterbegriffen** plus optionalem Bedienprofil (§42.2 Punkt 5) — kein Fremdwerkzeugkommando. Er ist damit dieselbe Struktur wie M-42, nicht ein zweites Objekt. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `manueller_busvorschlag_ist_ein_proposal` | Ein Busvorschlag entsteht als eigenes Objekt ohne `allowed_bounds` oder ohne `stop_if` | Entwurf §59 Lieferumfang (§1.1); §42.2:2789–2804 |

### 3.7 Evaluationskorpus (Gate: §59 Punkt 6, §36.4, §49.4, NAK-182-Muster)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-64 | Der Korpus wird ausgewertet | Er weist **Precision und Recall pro Ursachenklasse** aus — den sieben Klassen aus §8, nicht den vier Messklassen des P4-Korpus (`referenz`, `stereo`, `vergleich`, `zeitachse`). Dazu **Kalibrierung, Brier Score, Coverage und Enthaltung**. Die Riegel aus NAK-182 gelten unverändert: `falsche_starke`, `falsche_schwache`, `precision ≤ 1`, `recall ≤ 1`; jede Verletzung ist rot. | **BELEGT** · **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen), plus ein **NEU** Selbsttestbein nach dem Muster von **A27** | Precision wird über alle Klassen gemittelt statt je Ursachenklasse ausgewiesen; oder ein Riegel hat keinen Weg zu fallen | Entwurf §59 Lieferumfang (§1.1); §36.4:2411–2421; `tools/eq-copilot/pruefe_p4_korpus.py`:219–309 |
| M-65 | Synthetische Sessions mit **bekannter Quelle** laufen | **R2 (Matrixnacharbeit 1, 06.09.2026): der P5-Korpus ist eine Kette, kein Deklarationsregister.** Jede synthetische Session — wahrer Kandidat, korrelierter Distraktor, Parent-Duplikat, verschobene und zu kurze Passage — wird als **Evidenzbestand durch die Coordinator-Verdrahtung** (`hypothese_verdrahtung.rs`, Produktpfad) gefahren; gelesen wird die **tatsächlich ausgegebene** Hypothese (`candidate_source`, `confidence.class`, `alternatives`, `ausschluesse`) und gegen die Wahrheit des Falls gehalten. Riegel im Prüfer: eine starke Behauptung (Sicherheit hoch beziehungsweise `READY TO SEND`) auf Distraktor oder Parent-Duplikat ist `falsche_starke` und rot; der korrelierte Distraktor **muss** in `alternatives` erscheinen (NAK-190, SONDE-013 M-85 Ziel 1); `precision` ≤ 1, `recall` ≤ 1, geschlossene Mengen wie NAK-182 R1a. Der bisherige Weg — der Erzeuger serialisiert deklarierte `aussage`- und `konfidenz`-Werte und prüft Quellenstellen (`erzeuge_p4_korpus.py`:277–326) — bleibt als **eigener Riegel der Quellenhygiene** bestehen, ist aber **nicht** der Beleg für Entwurf §36.4 Satz 1: eine falsche starke Produktbehauptung ändert diese Angaben nicht. | **BAULÜCKE** · **ENTSCHIEDEN** (R2, §4.13) | **Messender Weg (neu):** Rust-Integrationstest im **A4**-Bereich, der die Sessions über `hypothese_verdrahtung.rs` fährt und die Korpusdatei als **Erwartung** liest; Kennzahlen je Ursachenklasse nach dem **A26**-Muster im **NEU** `pruefe_p5_korpus.py` (anzulegen). **Hygieneriegel (bleibt):** **NEU** `erzeuge_p5_korpus.py` (anzulegen) mit `--pruefen`, Muster **A25** | **Primärer Rotbeweis an der Zusage:** die Rangkomponente „zeitliche Koinzidenz“ wird im Rechner abgeschaltet, oder der Distraktor wird mit gleicher Bandpassung als Kandidat durchgelassen — bei **unveränderten** Korpus-Quellenangaben gibt der Produktpfad den Distraktor als Ursache aus. **Zweiter Rotbeweis (Hygiene):** ein Fall nennt eine Quelle, deren Bezeichner in der Datei nicht wörtlich steht | R2 (§4.13); Entwurf §36.4:2411–2421; NAK-190 (§1.8); `tools/eq-copilot/erzeuge_p4_korpus.py` (Bein **A25**) |
| M-66 | Die Schwelle aus M-31 soll kalibriert werden | Der Korpus **erzeugt** die Schwelle: die Abbildung hoch/mittel/unklar → handelbar wird so gewählt, dass Precision, Recall, Brier und Kalibrierung ihre Riegel halten. Die Schwelle ist Ausgabe, nicht Eingabe; sie wird versioniert und mit einem Regressionslauf geändert. | **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen), Fall `schwelle_ist_ausgabe_nicht_eingabe` | Die Schwelle wird als Konstante in das Bein gegeben und der Lauf bleibt grün | `docs/plan/plan.json` S23–25 Nachtrag 02.09. (1); U21-Folge |
| M-67 | Eine **verschobene Passage** liegt im Korpus | Sie erzeugt **keine starke** Aussage. Der Wert steht in der geschlossenen Wahrheitsmenge; auf ihm ist die starke Aussage falsch. Die Menge wächst gegenüber P4 um die Ursachenwahrheiten und bleibt geschlossen. | **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen); Muster: die zwei Mengen `KEINE_BEHAUPTUNG`/`KEINE_STARKE` aus **A26** | Eine starke Aussage auf einer verschobenen Passage bleibt grün | Entwurf §36.4:2411–2421; `eq-copilot/fixtures/p4-korpus/MANIFEST.json` (`mengen`) |
| M-68 | Der Korpus prüft sich selbst | Ein **Selbsttestbein ohne Repo-Fixture** (Muster **A27**) fährt synthetische Fälle durch **dieselben Funktionen**, die das Sammelbein führt, und lässt **jede Erwartung mit ihrem Gegenteil** laufen. Ohne das Gegenteil wäre der Selbsttest auch dann grün, wenn ein Riegel immer rot meldete. | **BELEGT** (Muster existiert) · **BAULÜCKE** (für P5) | **NEU** Selbsttestbein (anzulegen), Muster **A27** | Eine Riegelerwartung läuft ohne ihr Gegenteil | `tools/eq-copilot/pruefe_p4_korpus.py`:471–560 (Bein **A27**) |
| M-69 | Ein Gegenbeispiel wird gefahren (G5) | **Gegenbeispiele erzwingen nachweislich Enthaltung.** „Nachweislich" heißt: der Lauf zeigt, dass auf jedem Gegenbeispiel die Aussage `unsicher` ist und keine starke Aussage entsteht — nicht, dass sie zufällig ausblieb. Enthaltung ist das gewünschte Ergebnis, nicht ein Treffer. | **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen), Abschnitt Gegenbeispiele; G5 fährt zusätzlich adversarial | Ein Gegenbeispiel erzeugt eine starke Aussage; oder das Bein zählt die Enthaltung als Fehlschlag | Entwurf §59 Exit-Gate (§1.1); G5-Absatz (§1.3) |
| M-70 | Der P4-Korpus trägt die gedruckte Lücke | Die Zeile `nicht_gemessen` in `eq-copilot/fixtures/p4-korpus/MANIFEST.json` (**SONDE-013 M-85** Ziel 1, NAK-190) wird **erst dann entfernt, wenn der P5-Korpus den Fall wirklich misst**. Bis dahin bleibt sie gedruckt; ein Entfernen ohne Messung ist ein Rückschritt hinter NAK-182. | **BELEGT** | **A26** bestehend (druckt die Lücke heute), plus **NEU** `pruefe_p5_korpus.py` (anzulegen) | Die Lücke verschwindet aus dem Manifest, ohne dass ein P5-Fall sie misst | NAK-190 (§1.8); `docs/beweise/G4.md` §8; `docs/beweise/NAK-182.md` |

### 3.8 Ort und Nebenläufigkeit

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-71 | Intent, Evidenzgraph, Hypothese, Proposal und `AssistantStep` sollen einen Ort bekommen | Der Ort folgt **§33.5 wörtlich**: `SourceIntent` und `AssistantStep` gehören in den `MainProjectState` (autoritativ), **mit versioniertem SQLite-Spiegel für Suche und Crashdiagnose**; `Evidenz, Findings, Proposals, Transaktionsereignisse, Experimente und Urteile` gehören in den **lokalen SQLite-Experimentstore mit einem Writer**, und der Main-State hält davon **nur kompakte aktuelle IDs und noch nicht bestätigte Outbox-Ereignisse**. **Kein Objekt besitzt zwei still konkurrierende Wahrheiten.** **E-08 — die Autoritäten:** die autoritative **Validierung des Intents** (M-05, M-06, M-13) und die **Zustandsmaschine des `AssistantStep`** (M-55 bis M-62) laufen **im Main, also im Gen-Plugin in C++**, persistent im `MainProjectState`. `intent.rs` und `assistent.rs` im Broker-Coordinator sind **Spiegel und Vertragsvalidierung beim Empfang** (M-77) sowie die Auswertung der Konfliktregeln beim Rechnen — **keine zweite Zustandsmaschine, keine zweite Wahrheit**. `hypothese.rs`, `proposal.rs` und `maskierung.rs` rechnen im Coordinator, weil dort Evidenzbestand und Store liegen. **R3 (Matrixnacharbeit 1, 06.09.2026), Entscheid E-11:** der **Weg** zum zugesagten Spiegel war nicht benannt — `state_report` ist `additionalProperties: false` mit sieben Feldern und trägt keinen Schritt, und E-10 transportiert nur den Intent. Der Schritt reist deshalb über die Schwesterfamilie **`assistant_step_update`** und wird vom Broker über den bestehenden Ein-Writer-Pfad (`broker/src/store/writer.rs`) **versioniert gespiegelt**; der Spiegel ist **nie autoritativ** (M-88, M-89). | **BELEGT** · **ENTSCHIEDEN** (E-08, §4.8; E-11 aus R3, §4.13) | **NEU** `EqCopSonde014IntentTest` und `EqCopSonde014AssistentTest` für die Main-Seite, **A4**-Verdrahtungsfälle für die Brokerseite; **B14** für die Persistenz; der Spiegel selbst in M-88 und M-89 | Der Broker führt eine eigene `AssistantStep`-Zustandsmaschine; der Broker validiert eine andere Rollenmenge als das Plugin; ein Finding wird im Plugin gerechnet und im Broker noch einmal; oder der zugesagte Spiegel hat keinen Produktpfad | E-08 (§4.8); E-11 aus R3 (§4.13); Entwurf §33.5:2140–2175, Zeile 2147; `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `$defs/state_report` |
| M-72 | Irgendein Teil dieses Tickets rechnet | **Der Audio-Thread ist nie beteiligt.** Keine Sperre, keine Allokation, kein Datei-, Pipe- oder Netzzugriff, kein Logging im Audio-Thread. Überlast verwirft **Analyseframes, nie Audio**. | **BELEGT** | **A1** `EqCopNullTest` und **B4** `EqCopQueueStressTest` bestehend; **NEU** ein Fall, der die neuen Pfade auf Audio-Thread-Freiheit prüft | Ein neuer Pfad wird aus `processBlock` gerufen; oder eine Überlast verwirft Audio | `CLAUDE.md` Grundgesetz; Entwurf §33.1 |
| M-73 | Eine Queue läuft voll (Prüfliste A) | **Je Klasse steht die Politik bei *voll* ausdrücklich.** P0: 64er-Queue, Überlauf ⇒ Verbindung wird verworfen und der Eintrag geht an `beiP0Verworfen` — nie stillschweigend gelöscht. P1: 128 + 128 Einträge, leerer Schlüssel = Ereignis, nicht leerer Schlüssel **koalesziert Snapshots desselben Objekts**. P2 reist nicht über den ControlClient. **E-09, entschieden 06.09.2026:** `draft_offer` reist als **P1 mit Schlüssel `proposal:<proposal_id>`**; `user_verdict` als **persistenzpflichtiger P0** über `sendePersistenzP0`, mit Wiederholung unter derselben `command_id` über einen Brokerkill hinweg (A4-SI-Muster) — ein Userurteil darf **nicht** koaleszieren. Findings und Maskierungen reisen über `session_snapshot.findings` (E-04) und erben dessen Koaleszierung. Es entsteht **keine neue Klasse**. | **BELEGT** · **ENTSCHIEDEN** (E-09, §4.8) · **BAULÜCKE** | **NEU** Lastfall im **A4**-Bereich plus **B10** `EqCopIpcTest` erweitert; **A4-SI** für die Wiederholung des `user_verdict` über einen Brokerkill | Ein `draft_offer` wird ohne Schlüssel eingereiht und verdrängt ein anderes Objekt; ein `user_verdict` koalesziert; oder ein Rückgabewert der Politik wird ignoriert | E-09 (§4.8); `tools/dirigent/pruefliste.md` A; `eq-copilot/plugin/core/ipc/ControlClient.h`:242,314,556,558–560 |
| M-74 | Ein Puffer soll später wiederholen (Prüfliste A) | Ein Puffer, der „später wiederholt", hat einen **Abflussweg ohne Reconnect**. „Nur beim Verbindungsaufbau leeren" ist ein Befund. Für dieses Ticket heißt das: die Outbox-Schuld eines Findings oder Proposals wird auch ohne Linkwechsel abgetragen. | **BELEGT** | **A4-SI** `subscription_server_integration` bestehend (koalesziert Snapshot-Schuld), erweitert um die neuen Objektschlüssel | Ein Proposal bleibt in der Outbox stehen, bis die Verbindung neu aufgebaut wird | `tools/dirigent/pruefliste.md` A; `broker/src/store/migration.rs` (`outbox`) |
| M-75 | Verbinden und Trennen (Prüfliste B) | **Reihenfolge beim Verbinden:** anmelden → Callback „verbunden" abgeschlossen → erst dann Welcome/Freigabe nach außen. **Beim Trennen:** zuerst abmelden/entkoppeln, dann Fristen und Joins; nichts bleibt registriert, während auf einen Verbraucher gewartet wird. Jeder Join hat eine Frist. Nach dem Schließen einer Queue wird nichts mehr geliefert; das **Schließflag wird vor dem Inhalt geprüft**. Die neuen Objekte erben diese Ordnung und führen keine eigene ein. | **BELEGT** | **A4** und **A4-SI** bestehend, erweitert um die neuen Objekte | Ein Finding wird nach dem Schließen der Queue noch geliefert; oder ein neuer Subscriber wird vor dem Verbunden-Callback freigegeben | `tools/dirigent/pruefliste.md` B; `broker/src/coordinator/subscription.rs` |
| M-76 | Der Broker startet neu, während ein Schritt offen ist | **Sessiongraph, Frische, Subscriptions und Broker-Cache sind flüchtig** und werden aus Plugin-Reports, Main-State und Store rekonstruiert. Der Broker ist **nicht berechtigt**, beim Reconnect blind seinen Cache auf die Probe zu drücken: zuerst meldet die Probe ihren Zustand, Main gleicht ab, Konflikte werden sichtbar entschieden. | **BELEGT** | **A4** bestehend (Killmatrix), erweitert um Intent und `AssistantStep` | Nach einem Brokerneustart überschreibt der Cache eine neuere Intent-Revision aus dem Plugin-State | Entwurf §33.5:2140–2175 |
| M-77 | Dieselbe Regel in beiden Sprachen (Prüfliste A) | Jede Vertragsregel dieses Tickets, die beide Seiten betrifft, existiert **in C++ und in Rust**, und der Test, der sie misst, existiert **beidseitig** und ist einmal gebrochen worden. Das betrifft mindestens: die geschlossene Rollenmenge (M-01), die Ursachenklassen (M-14), die Proposal-Feldmenge (M-42) und jede Längen- oder Enumgrenze. | **BELEGT** | **A5** (Textriegel, Fixtures) plus je ein C++- und ein Rust-Bein | Eine Enumgrenze wird nur auf einer Seite durchgesetzt | `tools/dirigent/pruefliste.md` A und C |

### 3.9 Produktinvarianten aus `CLAUDE.md`

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-78 | Der Advisor hat einen Befund und einen Vorschlag | **Nichts Ungefragtes.** Der Advisor **schlägt nur vor**; keine Verarbeitung ohne Einschalten durch den User, keine Fernänderung in P5. Ein Proposal ist ein Angebot, das ohne Useraktion folgenlos bleibt und nach seiner Frist verfällt. | **BELEGT** | **A1** `EqCopNullTest` bestehend; **NEU** `EqCopSonde014ProposalTest`, Fall `ein_proposal_ohne_useraktion_bleibt_folgenlos` | Ein Proposal löst ohne Useraktion eine Messung, eine Markierung oder eine Zustandsänderung an der Probe aus | `CLAUDE.md` Grundgesetz; Entwurf §59 Zweck |
| M-79 | Ein Befundtext oder ein Vorschlagstext entsteht | **Keine KI-Erklärschicht.** Der Advisor ist regelbasiert; die Empfehlungssprache entsteht aus **festen Bausteinen**. Es gibt kein Sprachmodell, also auch keinen Ausfallfall. Kein Text erfindet Frequenz, Güte, Gain, Zielinstanz oder Grenze. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `text_entsteht_aus_bausteinen_und_erfindet_keine_zahl` | Ein Textbaustein trägt eine Zahl, die in keinem Feld steht | `CLAUDE.md` Wahrheitskern; Entwurf §59 (gestrichener Punkt, §1.1); §42.2 (gestrichener Absatz) |
| M-80 | Die Engine liefert einen Snapshot | **Die Engine kennt keine Optik.** Sie liefert kohärente Mess-Snapshots; der Editor hält Anzeigezustand und rendert nur **neue Snapshot-Revisionen**. Der Maskierungswert aus M-36 ist Messgröße, keine Zeichenanweisung: er trägt Frequenzbereich und Wert, keine Farbe, keine Achse, keinen Zoom. | **BELEGT** | **NEU** `EqCopSonde014MaskierungTest`, Fall `datenweg_traegt_keine_zeichenanweisung` | Der Datenweg trägt Farbe, Achsenbereich oder Zoom | `CLAUDE.md` tragende Invarianten |
| M-81 | Ein Zustand mit unbekannten Feldern oder unbekanntem Major wird geladen | **State bleibt verlustfrei.** Eine unbekannte **Eigenschaft** in einem bekannten Kind desselben Majors ist additiv und wird erhalten; ein unbekanntes **Kind** oder ein unbekanntes Root-Major lädt audio-neutral und read-only, bewahrt die Originalbytes und schreibt sie nie durch einen Teilstate überschreibend zurück. Intent und `AssistantStep` sind **Eigenschaften** in `MainProject`, kein neues Kind — deshalb ist die Erweiterung additiv. | **BELEGT** | **B2** `EqCopStateMigrationTest` und **A12** `erzeuge_state_fixtures.py` bestehend, erweitert um die neuen Eigenschaften | Ein Stand mit unbekannter MainProject-Eigenschaft verliert sie beim Speichern; oder ein neues Kind entsteht | `CLAUDE.md`; `eq-copilot/schemas/state/nakama-state-v2.md` §2.1; Entwurf §53.8:4042–4102 |
| M-82 | Ein nicht endlicher Wert entsteht irgendwo in diesem Ticket | **NaN-Ehrlichkeit.** Nicht-endliche Werte werden **beim Erzeugen** verriegelt und gezählt, nie sanitisiert auf die Leitung gelassen; ein Band ohne Messung trägt 0 mit `gueltig = false`, nie `NaN`. Nyquist wird gekappt. Ohne genügend endliche Nachbarn gibt es **keine Basislinie** — und damit auch keinen Maskierungswert und keine starke Hypothese. | **BELEGT** | **A10**/**B3c** bestehend (beide Leser), **NEU** je ein Negativfixture für Maskierungswert und Konfidenzscore | Ein `NaN` erreicht die Leitung; oder ein Maskierungswert entsteht aus einem Fenster ohne endliche Nachbarn | `CLAUDE.md`; SONDE-013 M-07, M-10 |
| M-83 | Irgendetwas an Identität, Class-IDs oder Replace-V2 würde berührt | **Identität bleibt eingefroren** bis NAK-30. Dieses Ticket ändert keine Class-ID, keinen Hersteller-/Plugin-Code und kein Replace-V2-Verhalten. `MainProject` ist ein bestehendes Kind; Gen führt weiterhin **null** Hostparameter. | **BELEGT** | **B1** `EqCopIdentityTest` und **A14** `pruefe_kern_identitaetsfrei.py` bestehend | Eine Class-ID ändert sich; oder Gen bekommt einen Hostparameter | `CLAUDE.md`; Entwurf §53.8:4042–4102 |
| M-84 | Ein sichtbares Element entsteht auf Gen Fläche 1 | **Keine toten UI-Elemente.** Jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand. Für dieses Ticket konkret und heute messbar: `SourcesModel::findingsOffen` ist ein Feld ohne Produzenten (§2.10, L11) — es bekommt mit diesem Ticket seine Quelle **oder** es bleibt bis dahin ohne Anzeige. Ein Zähler, der immer 0 zeigt, ist ein totes Element. | **BAULÜCKE** | **B13** `EqCopSonde012SourcesModelTest` erweitert um den Produktpfadfall | `findingsOffen` wird angezeigt, ohne dass ein Produktpfad ihn setzt | `CLAUDE.md` „Keine toten UI-Elemente"; `eq-copilot/plugin/src/SourcesModel.cpp`:1210; Blueprint §4.1 |

### 3.10 Transport von Intent und Assistentenschritt vom Main zum Broker (E-10, E-11)

**M-85 und M-86 sind in Etappe 1b entstanden, M-88 und M-89 in der
Matrixnacharbeit 1** (E-11 aus Regel R3, §4.13). Beide Paare messen dieselbe
Klasse: ein Bestand, den der Main hält und der Broker braucht, und der ohne
eigene Nachricht keinen Weg hat.

**E-10 ist eine Lücke, die der Dirigent an der Matrix gefunden hat:** §3.1 legt den Intent in den
Main-State (M-09, M-71) und §3.2/§3.5 lassen den Broker damit rechnen (M-12,
M-20, M-44, M-54), aber keine Zeile nannte eine Nachricht dafür — und `intent`
ist im Register frei (§2.6). Ein Veto (`geschützt`, `bewusst verschmolzen`) darf
durch Transportverlust nie verschwinden; genau daran hängt M-86, und seit E-11
hängt M-89 mit derselben Begründung am Assistentenschritt.

Der Familienname ist **`intent_update`**. Begründung im Muster der bestehenden
Namen: der Vertrag benennt Familien nach *Gegenstand plus Vorgang*
(`session_command`, `evidence_invalidate`, `experiment_begin`,
`experiment_candidate`, `state_report`, `command_ack`). `intent_update` folgt
demselben Schnitt, ist eindeutig gegen `state_report` (Plugin-Identität statt
musikalischer Absicht) und gegen `session_command` (Sessiongraph statt Intent)
abgegrenzt und sagt zugleich, dass es sich um eine **Fortschreibung mit
Revision** handelt, nicht um eine Anfrage. Der Name wird in diesem Ticket in
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` reserviert **und**
belegt — beides in derselben Fassung 3 wie `session_snapshot.findings` (E-04),
`experiment_begin.ziel` (E-05) und die Belegung von `draft_offer` und
`user_verdict` (E-09).

**E-11 (Matrixnacharbeit 1, Regel R3) führt die Schwesterfamilie
`assistant_step_update`.** Der Prüfer hat gefunden, dass der von Entwurf
§33.5 (Zeile 2147) zugesagte **versionierte SQLite-Spiegel des aktuellen
`AssistantStep`** keinen Produktpfad hatte: `state_report` ist
`additionalProperties: false` mit sieben Feldern und trägt keinen Schritt, E-10
transportiert nur den Intent, und für keine Erweiterung der Fassung 3 war der
Schritt als Nutzlast festgelegt. Der Name folgt demselben Muster wie
`intent_update` — Gegenstand plus Vorgang, Fortschreibung mit Revision — und
liegt in **derselben** Fassung 3. Die Vollständigkeitsmarke aus M-86 deckt seit
E-11 **beide** Bestände: Intent **und** aktuellen Schritt.

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-85 | Der User setzt oder ändert einen Intent; Main ist mit dem Broker verbunden | Der Intent reist vom Main zum Broker über die eigene Familie **`intent_update`**, Klasse **P1 mit Schlüssel `intent:<quelle_id>:<passage_id oder global>`**. Koaleszierung ist **zulässig**, weil nur die jüngste Revision wirkt (M-10); genau deshalb ist der Schlüssel je Quelle **und** Scope eigen — zwei Passagen derselben Quelle sind zwei Objekte (M-08) und dürfen sich nicht gegenseitig verdrängen. Die Revisionsordnung ist total: eine ältere Revision überschreibt nach der Koaleszierung nie eine jüngere. | **BAULÜCKE** · **ENTSCHIEDEN** (E-10, §4.9) | **NEU** **A4**-Verdrahtungsfall `intent_update_koalesziert_je_quelle_und_scope` plus **B10** `EqCopIpcTest` für die Senderseite; **A5** für Discriminator, Feldmenge und Negativfixtures | Eine ältere Revision überschreibt nach der Koaleszierung eine jüngere; oder zwei Passagenrollen derselben Quelle verdrängen sich unter demselben Schlüssel | E-10 (§4.9); `tools/dirigent/pruefliste.md` A; Entwurf §37.3:2461–2465 |
| M-86 | Die Verbindung wird aufgebaut, oder der Broker startet neu | Main meldet seinen **vollständigen** Intent-Bestand — **auch einen leeren** — mit einer **Vollständigkeitsmarke samt Revision**, **bevor** der Broker eine Hypothese oder ein Proposal rechnet. Reihenfolge nach M-75 und M-76: anmelden → Verbunden-Callback abgeschlossen → Intent-Vollbestand mit Marke → erst dann Rechnen. **Bis die Marke vorliegt, entstehen keine Proposals; der Zustand ist `MORE DATA`** (M-29, M-30). Grund: ein Veto darf durch Transportverlust nie verschwinden — ein fehlender Intent sieht sonst aus wie „kein Schutz gewünscht". | **BAULÜCKE** · **ENTSCHIEDEN** (E-10, §4.9) | **NEU** **A4** Neustartfall `keine_rechnung_vor_der_vollstaendigkeitsmarke`; **A4-SI** für den Reconnect über einen Brokerkill | Ein Proposal entsteht vor der Marke; oder ein leerer Bestand wird gar nicht gemeldet und der Broker rechnet auf einem Rest aus seinem Cache (M-76) | E-10 (§4.9); `tools/dirigent/pruefliste.md` B; Entwurf §33.5:2140–2175 |
| M-88 | Der Assistentenschritt wechselt im Main — **ohne** Experimentabschluss und **ohne** Userurteil | **E-11 (R3, Matrixnacharbeit 1, 06.09.2026).** Der Schritt reist vom Main zum Broker über die Schwesterfamilie **`assistant_step_update`**, demselben Muster folgend wie `intent_update`: **Klasse P1 mit Schlüssel `assistant_step:<session_id>`**, koaleszierend, weil nur die jüngste Revision wirkt. In **derselben Fassung 3** reserviert und belegt. Die Vollständigkeitsmarke aus E-10 deckt **beide** Bestände: Main meldet nach Verbindungsaufbau und nach jedem Brokerneustart **Intent-Bestand und aktuellen Schritt**, bevor der Broker rechnet (M-86). Die Revisionsordnung ist total: eine ältere Revision überschreibt nach der Koaleszierung nie eine jüngere. | **BAULÜCKE** · **ENTSCHIEDEN** (E-11 aus R3, §4.13) | **NEU** **A4**-Verdrahtungsfall `assistant_step_update_koalesziert_je_session` plus **B10** `EqCopIpcTest` für die Senderseite; **A5** für Discriminator, Feldmenge und Negativfixtures | Ein Schrittwechsel im Main **ohne** Experimentabschluss und **ohne** Userurteil erreicht den Spiegel nicht; oder eine ältere Revision überschreibt nach der Koaleszierung eine jüngere | E-11 aus R3 (§4.13); Entwurf §33.5:2140–2175, Zeile 2147; `tools/dirigent/pruefliste.md` A |
| M-89 | Der Broker startet neu; ein Schritt liegt im Spiegel und im Main-State | **E-11 (R3).** Der Broker spiegelt den Schritt **versioniert** in den Store über den bestehenden Ein-Writer-Pfad (`broker/src/store/writer.rs`) — für Suche und Crashdiagnose (§33.5). **Der Spiegel ist nie autoritativ:** bei Widerspruch gewinnt der neuere Main-State, und ein alter Datenbankeintrag setzt **nie** einen neueren Plugin-State zurück (§33.5, M-59, M-76). Reihenfolge nach dem Neustart: Main meldet Bestand und Schritt mit Marke → Broker gleicht **seinen** Spiegel an den Main-State an → erst danach rechnet er. Writer und beidseitige Verbraucher liegen im **selben** Änderungssatz (Bauetappe B). | **BAULÜCKE** · **ENTSCHIEDEN** (E-11 aus R3, §4.13) | **NEU** **A4** Neustartfall `spiegel_weicht_dem_neueren_main_schritt`; **A4-SI** für den Reconnect über einen Brokerkill; **B14** für die Main-Seite | Nach einem Brokerneustart überschreibt der ältere Spiegel den neueren Main-Schritt; oder der Broker rechnet, bevor er seinen Spiegel angeglichen hat | E-11 aus R3 (§4.13); Entwurf §33.5:2140–2175, Zeile 2147; `broker/src/store/writer.rs` |

### 3.11 Zählung nach Belegklasse

Gezählt an den 89 Zeilen der zehn Tabellen dieses Abschnitts mit einem
escape-bewussten Parser, nicht abgeschrieben: `M-01` bis `M-89`, keine Lücke,
keine Doppelung, jede Zeile sieben Spalten. **25 Zeilen tragen mehr als eine
Marke**; die Summe der Spalte „Zeilen“ ist deshalb 120 und nicht 89.

| Belegklasse | Zeilen | Bedeutung | IDs |
|---|---|---|---|
| **BELEGT** | 72 | Die Zusage folgt aus einer verbindlichen Quelle. | M-01, M-03, M-05, M-06, M-07, M-08, M-09, M-10, M-11, M-12, M-13, M-14, M-15, M-16, M-17, M-18, M-19, M-20, M-23, M-25, M-26, M-27, M-28, M-29, M-30, M-31, M-32, M-33, M-34, M-35, M-36, M-37, M-38, M-39, M-41, M-42, M-43, M-44, M-45, M-46, M-47, M-49, M-51, M-52, M-53, M-54, M-55, M-56, M-57, M-58, M-59, M-60, M-61, M-62, M-63, M-64, M-68, M-70, M-71, M-72, M-73, M-74, M-75, M-76, M-77, M-78, M-79, M-80, M-81, M-82, M-83, M-87 |
| **ENTSCHIEDEN** | 19 | Durch E-01 bis E-11 beziehungsweise R1 bis R4 (06.09.2026) geschlossene Technikfrage. | M-02, M-03, M-04, M-05, M-23, M-36, M-37, M-40, M-43, M-48, M-55, M-65, M-71, M-73, M-85, M-86, M-87, M-88, M-89 |
| **BAULÜCKE** | 28 | Belegt beziehungsweise entschieden, aber eine Bau- oder Testkante fehlt. | M-21, M-22, M-23, M-24, M-25, M-28, M-31, M-36, M-37, M-40, M-43, M-47, M-48, M-50, M-59, M-64, M-65, M-66, M-67, M-68, M-69, M-73, M-84, M-85, M-86, M-87, M-88, M-89 |
| **ENTSCHEIDEN-VORSCHLAG** | 0 | Seit Etappe 1b leer — alle Fragen sind entschieden. | — |
| **OFFEN** | 1 | Produktfrage, benannt und einer Fläche zugeordnet, nie hier beantwortet. | M-49 |

Die 25 mehrfach markierten Zeilen im Einzelnen:

| Kombination | Anzahl | IDs |
|---|---|---|
| BELEGT · BAULÜCKE | 7 | M-25, M-28, M-31, M-47, M-59, M-64, M-68 |
| ENTSCHIEDEN · BAULÜCKE | 7 | M-40, M-48, M-65, M-85, M-86, M-88, M-89 |
| BELEGT · ENTSCHIEDEN · BAULÜCKE | 6 | M-23, M-36, M-37, M-43, M-73, M-87 |
| BELEGT · ENTSCHIEDEN | 4 | M-03, M-05, M-55, M-71 |
| BELEGT · OFFEN | 1 | M-49 |

**Rein ENTSCHIEDEN** — die 2 Zeilen, für die es weder eine Quelle noch eine
bestehende Baukante gab, sondern nur den Entscheid: **M-02**, **M-04**.
Beide gehen auf E-01 beziehungsweise E-02 zurück (Abbildung der fünf Rollen,
Verhältnis von Rolle und paarweiser Verschmelzungserlaubnis).

**Bilanz gegenüber Etappe 1b** (Commit `fdb04e4`, 86 Zeilen), Zeile für
Zeile gegen die dortige Zählung geprüft:

- **Drei Zeilen neu:** **M-87** (Ausschlussgründe, R4), **M-88** und **M-89**
  (Transport und Spiegel des Assistentenschritts, R3 → E-11).
- **Drei Zeilen nachgezogen:** **M-23** („zu kurz“ als
  `GATE_MINDEST_FENSTER`, zwei Rotbeweise am Passagengate), **M-65** (Korpus als
  Kette durch die Coordinator-Verdrahtung) und **M-71** (Weg zum Spiegel).
  Alle drei tragen seitdem zusätzlich **ENTSCHIEDEN**; M-23 zusätzlich
  BAULÜCKE, weil `GATE_MINDEST_FENSTER` heute nicht existiert.
- **BELEGT steigt von 71 auf 72** (M-87 kommt dazu; keine Zeile verliert ihre
  Quelle). **ENTSCHIEDEN steigt von 14 auf 19**, **BAULÜCKE von 25 auf 28**.
- **OFFEN bleibt bei einer Zeile** (M-49, die DSP-ACK-Hälfte für P7).

**Bilanz gegenüber Etappe 1** (Commit `1f126a4`, 84 Zeilen): fünf Zeilen
neu (M-85 bis M-89), neun ENTSCHEIDEN-VORSCHLAG-Marken zu ENTSCHIEDEN geworden
(M-02, M-04, M-37, M-40, M-43, M-48, M-55, M-71, M-73), fünf bereits belegte
Zeilen zusätzlich als ENTSCHIEDEN markiert (M-03, M-05, M-23, M-36, M-65) und
neun Zeilen um eine BAULÜCKE ergänzt (19 → 28).
---
## 4. Offene Punkte

Eine OFFEN-Zeile ist **kein stiller Produktentscheid**. Technikfragen ohne
Quelle stehen hier als **ENTSCHEIDEN-VORSCHLAG** mit Begründung und
Alternativen; der Dirigent entscheidet, bevor Etappe 2 sie baut. Produktfragen
werden **benannt und einer Fläche zugeordnet**, nie hier beantwortet. Die
offenen Karten U23 bis U33 in `docs/plan/fragen.json` sind bekannt und werden
nicht neu gestellt; was in den Abnahmen steht, wird nicht gefragt.

### 4.1 E-01 — Wie die fünf Rollen auf `SourceIntent` abgebildet werden (M-02)

**Frage.** U22 legt die **Anzahl** fest (fünf) und sagt ausdrücklich, die
Abbildung auf §37.1 sei Technik dieses Tickets. §37.1 führt `prominence`
(drei Werte), mehrere Funktionstags, geschützte Bereiche/Eigenschaften,
gerichtete Beziehungen, optionalen Passage-Scope, Revision, Herkunft und
Konfidenz. Fünf Rollen müssen darauf so fallen, dass die Abbildung total und
umkehrbar ist (M-02) und Impuls/Raum **nicht** als Rolle zurückkommen (M-03,
M-04).

**Vorschlag (A).** Die Rolle ist ein **eigenes, geschlossenes Feld**
`rolle` mit fünf Werten; `prominence` und die Funktionstags werden aus ihr
**abgeleitet** und nicht getrennt gesetzt. Ableitung:

| Rolle | `prominence` | Funktionstag | Wirkung im Ranking |
|---|---|---|---|
| führt | `foreground` | `lead` | Schutz vor Reduktion, höchste Intent-Relevanz |
| trägt | `middle` | `foundation` | Reduktion nur unterhalb der Schutzgrenze |
| begleitet | `background` | `texture` | darf nachgeben |
| geschützt | unverändert (orthogonal) | — | Veto gegen jeden automatischen Vorschlag in den geschützten Bereichen |
| bewusst verschmolzen | unverändert (orthogonal) | — | Veto gegen Entmaskierungsempfehlungen |

**Begründung.** Genau so liest sich U22: „Kurze Anschläge schützen … und
Hall/Atmosphäre verschmelzen lassen sind dann keine eigenen Rollen, sondern
eine Schutzangabe bzw. Verschmelzungs-Erlaubnis an der Quelle." Die beiden
letzten Zeilen sind deshalb bewusst **orthogonal** zu `prominence`: eine
geschützte Quelle kann vorne oder hinten liegen. Der Preis ist, dass „geschützt"
und „bewusst verschmolzen" in dieser Tabelle keine Prominenz setzen — sie sind
Vetos, keine Positionen. Das ist eine Produktwirkung, die der Dirigent kennen
muss, aber keine Produktfrage: U22 hat die Liste entschieden.

**Alternative (B).** `prominence`, Funktionstags, Schutz und Beziehungen
bleiben frei setzbar, und die fünf Rollen sind **Vorlagen**, die Startwerte
füllen (§37.2 „Vorlagen … füllen nur Startwerte"). Vorteil: näher am Wortlaut
von §37.2. Nachteil: die Rolle ist dann **nicht** aus der Belegung zurücklesbar
(M-02 fällt), und die UI müsste einen abgeleiteten Rollenwert anzeigen, der von
der gespeicherten Belegung abweichen kann — genau die „zwei still
konkurrierenden Wahrheiten", die §33.5 verbietet.

**Empfehlung: (A).** Sie hält M-02 und §33.5; (B) bricht beide. §37.2 bleibt
erfüllt, weil die Ableitung deterministisch ist und der User weiterhin
zusätzlich Schutzbereiche und Beziehungen setzt.


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-01 angenommen in Fassung (A),
mit der Präzisierung E-01a.** `rolle` ist ein geschlossenes Feld mit genau
fünf Werten; eine Quelle trägt **je Scope** — global oder Passage — genau eine
Rolle oder keine. Die Ableitung folgt der Tabelle oben für `führt`
(`foreground`, `lead`), `trägt` (`middle`, `foundation`) und `begleitet`
(`background`, `texture`).

**Korrigiert gegenüber dem Vorschlag:** für `geschützt` und `bewusst
verschmolzen` ist `prominence` **nicht** „unverändert", sondern abgeleitet auf
den festen Neutralwert `middle` mit leerem Funktionstag und einem
**Veto-Kennzeichen** (`schutz` beziehungsweise `verschmolzen`) als Teil der
Belegung. Nur so ist die Abbildung total und injektiv, der Rückweg eindeutig
und keine zweite Wahrheit nach Entwurf §33.5 möglich. Der Vorschlagstext oben
ließ hier eine Lücke: „unverändert (orthogonal)" ist keine Belegung, aus der
sich die Rolle zurückrechnen lässt.

**Orthogonal und von der Rolle unabhängig** bleiben die geschützten Bereiche
und Eigenschaften (Attack, Breite, Ausklang, Bandintervalle) sowie die
paarweisen Beziehungen aus §37.1 — das sind die Schutzangabe und die
Verschmelzungs-Erlaubnis, über die U22 Impuls und Raum abbildet (M-03, M-04).

**Produktwirkung, die daraus folgt und keine neue Produktfrage ist:** eine
Quelle mit Rolle `geschützt` trägt keine Prominenzposition; wer Position und
Schutz zugleich will, setzt die Positionsrolle und dazu Schutzbereiche.

**Test:** Roundtrip über alle fünf Rollen plus die Probe, dass zwei
verschiedene Belegungen nie auf dieselbe Rolle zurückgelesen werden.
Matrixzeilen: **M-02** (ENTSCHIEDEN), **M-03** (BELEGT · ENTSCHIEDEN).

### 4.2 E-02 — Verhältnis von Rolle „bewusst verschmolzen" zur paarweisen Erlaubnis (M-04)

**Frage.** §37.1 kennt die gerichtete beziehungsweise ungerichtete Beziehung
„A/B dürfen verschmelzen" **zwischen zwei Quellen**. U22 macht „bewusst
verschmolzen" zu einer Rolle **einer** Quelle. Ist die Rolle eine Abkürzung für
„diese Quelle darf mit jeder verschmelzen", oder sind es zwei Dinge?

**Vorschlag (A).** Zwei Dinge, beide erhalten. Die **Rolle** an der Quelle ist
ein globales Veto gegen Entmaskierungsempfehlungen, an denen diese Quelle
beteiligt ist; die **paarweise Erlaubnis** ist das gezieltere Werkzeug und
gewinnt, wo sie gesetzt ist. Beide werden in den Konfliktregeln (§37.2) auf
Stufe 2 „expliziter Userintent" behandelt; bei Widerspruch gewinnt die
spezifischere Angabe (paarweise vor global), analog zu Stufe 3
„passagespezifisch vor global".

**Alternative (B).** Nur die Rolle; die paarweise Erlaubnis entfällt.
Nachteil: §37.1 nennt sie ausdrücklich, und §46 (Entmaskierung, P8) baut auf
gerichteten Beziehungen auf. Ein Wegfall wäre ein Vorgriff, den P8 teuer
zurücknehmen müsste.

**Empfehlung: (A).**


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-02 angenommen in Fassung (A).**
Die Rolle `bewusst verschmolzen` ist ein **globales Veto** gegen
Entmaskierungsempfehlungen mit Beteiligung dieser Quelle; die paarweise
Erlaubnis beziehungsweise gerichtete Beziehung ist das **spezifischere**
Werkzeug und gewinnt, wo sie gesetzt ist — eine ausdrückliche Beziehung
„A führt vor B" hebt das globale Veto **nur für dieses Paar** auf. Beide
liegen auf Stufe 2 der Konfliktregeln, spezifisch vor global, analog zur Regel
der Stufe 3.

**Test:** drei Fälle — nur Rolle, nur Beziehung, beides im Widerspruch.
Matrixzeilen: **M-04** (ENTSCHIEDEN), **M-05** (BELEGT · ENTSCHIEDEN,
Stufenzuordnung).

### 4.3 E-03 — Auf welchem Takt der Maskierungswert reist (M-37)

**Frage.** U16 verlangt, dass der Wert „mit der Anzeigekadenz mithält". Es gibt
drei Takte (§2.9): P2-Telemetrie 10 Hz, Evidenz 1–4 Hz, Editor leicht 20 Hz /
schwer 4 Hz. Welcher trägt ihn?

**Vorschlag (A).** Der Wert entsteht **im Broker aus dem Evidenzbestand** und
reist auf dem bestehenden Snapshot-Rückweg zu Gen; die Anzeige liest ihn im
schweren Editortakt (4 Hz). Damit ist „mithalten" erfüllt: der Editor findet
bei jedem schweren Takt einen aktuellen Wert, weil der Evidenzpfad mit
mindestens 1 Hz und bis 4 Hz liefert und der Broker nicht langsamer rechnet als
er empfängt.

**Begründung.** Der Wert ist eine Aussage über **zwei** Quellen. Er kann
deshalb nicht im Plugin einer Quelle entstehen — dort liegt die zweite Quelle
nicht vor. Der Broker ist die einzige Stelle, an der beide Evidenzbestände
liegen (`Coordinator`, `EVIDENZ_RETENTION = 32` je Quelle).

**Bekannter Preis, ausdrücklich benannt.** Bei P1-Rückstau **halbiert sich die
Evidenzkadenz** (`SondeProcessor.cpp`:489–494, Intervall verdoppeln bis
`kEvidenzIntervallMaxS = 1.0`). Der Maskierungswert wird dann bis auf 1 Hz
langsamer. Das ist kein Bruch der Zusage, sondern die bestehende, gemessene
Überlastpolitik („Überlast verwirft Analyseframes, nie Audio"); die Zusage aus
M-37 gilt für den Normalbetrieb und wird bei Rückstau **ehrlich als
herabgesetzt gemeldet**, nicht interpoliert.

**Alternative (B).** Der Wert reist auf dem P2-Livepfad mit 10 Hz. Nachteil:
P2 ist FlatBuffers je **Quelle**, trägt 64 Gruppen und kennt keine zweite
Quelle; ein Zwei-Quellen-Wert dort wäre ein neues Feld mit neuer Feld-ID und
ein Vorgriff auf S16–17. Zudem müsste das Plugin eine Aussage über eine fremde
Quelle bilden — genau die zweite Wahrheit, die §33.5 verbietet.

**Empfehlung: (A).**


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-03 angenommen in Fassung (A), mit
präziserer Zusage.** Der Wert entsteht im Broker-Coordinator **bei jedem
Evidenz-Eingang einer der beiden Quellen** und reist über den Rückweg aus
E-04. Die Zusage von M-37 lautet ab jetzt:

1. Bei jedem **schweren Editortakt (250 ms)** liegt der Wert so aktuell vor wie
   die jüngste Evidenz beider Quellen.
2. Im Normalbetrieb (`kEvidenzIntervallMinS` = 0,25 s) ist er **höchstens ein
   Evidenzintervall plus Transportlatenz** alt.
3. Bei P1-Rückstau folgt er der bestehenden Kadenzreduktion bis 1 Hz und
   trägt dann das Kennzeichen `herabgesetzt` — ein **Feld im Wert**, nie ein
   interpolierter Zwischenwert.

**Der Test misst das Alter des Werts relativ zur jüngsten Evidenz, nicht eine
Wallclock-Rate.** Der Rotbeweis lässt den Wert nur beim Neuberechnen eines
Befunds entstehen. Matrixzeile: **M-37** (BELEGT · ENTSCHIEDEN · BAULÜCKE).

### 4.4 E-04 — Ob der Maskierungswert ein eigenes v3-Feld braucht (M-40)

**Frage.** Der Plantext lässt sie ausdrücklich offen: „Ob der laufende Wert ein
eigenes Feld im v3-Vertrag braucht, entscheidet der Datenweg an der Quelle."

**Messung an der Quelle.** `evidence_snapshot` trägt heute je Quelle
`baender` (221 quantisierte Bandwerte), `verteilung.p10/p50/p95`, `abdeckung`,
`konvergenz` und `konfidenz`. Ein Maskierungswert zweier Quellen A und B in
Band *b* ist aus **A.p50[b]**, **B.p50[b]** und den Gültigkeitsbits
**berechenbar** — das ist genau die Rechnung, die `experiment_verdrahtung.rs`
bereits für Banddeltas fährt (`mittel_je_band`, :901–944). Es fehlt **kein
Messwert**; es fehlt der Rechner und der Rückweg.

**Vorschlag (A) — kein neues Feld am `evidence_snapshot`.** Der Broker rechnet
den Wert aus der vorhandenen Bandevidenz. Der **Rückweg** zu Gen ist der
bestehende `session_snapshot` — dieselbe Stelle, an der SONDE-013 in Fassung 2
`experimente` und `paare` als optionale Felder ergänzt hat. Das ist ein
`schema_minor`-Schritt an einer Familie, die dafür ausgelegt ist, und **kein**
Eingriff in den strikten `evidence_snapshot`.

**Begründung.** Ein Feld am Snapshot wäre ein Fassungsschritt an einem
`additionalProperties: false`-Objekt und würde außerdem verlangen, dass die
**sendende Sonde** eine Aussage über eine **fremde** Quelle trifft — sie kennt
sie nicht. Der Wert ist strukturell eine Broker-Aussage, keine Sondenmessung.

**Wenn die Entscheidung dennoch auf ein eigenes Feld fällt**, lautet der
Namensvorschlag `session_snapshot.maskierungen` (Liste je Quellenpaar mit
`quelle_a`, `quelle_b`, `band_von`, `band_bis`, `wert_db`, `gueltig`,
`finding_id`) — **nicht** `ueberdeckung`, weil dieser Name im Code bereits die
Zeitüberdeckung zweier Passagenfenster bezeichnet (§2.12) und der Blueprint die
Sache selbst **Maskierungszone** nennt. Der Name gehört dann nach dem
Plantext in die v3-Vertragsrunde von S16–17. **Diese Etappe ändert kein
Schema.**

**Kann der Bau ohne diesen Entscheid ehrlich fortfahren?** **Teilweise.** Die
Rechnung (M-36), die Kadenz (M-37) und die Bindung an den Befund (M-38, M-41)
sind ohne Vertragsentscheid baubar und testbar — sie leben im Broker. **Der
Rückweg zu Gen ist es nicht:** `session_snapshot` ist
`additionalProperties: false`, ein Feld mehr ist ein Fassungsschritt, und ein
Fassungsschritt ohne Entscheid wäre genau der Vertragsanteil, der später
bricht. Etappe 2 baut deshalb bis zur Brokergrenze und hält vor dem Rückweg an,
bis E-04 entschieden ist.


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-04 entschieden: JA, ein eigenes
v3-Feld — aber am Rückweg, nie an der Sondenmessung.** Der Vorschlag oben ist
damit in seiner Begründung bestätigt und in seiner Schlussfolgerung
korrigiert: kein Feld am `evidence_snapshot`, aber ein Feld am
`session_snapshot`.

**Fassung 3.** `session_snapshot` bekommt als Wire-Envelope-`schema_minor` 3 —
ein Fassungsschritt wie Fassung 2 in SONDE-013 — genau **ein** neues optionales
Feld `findings`:

- die Liste der Befunde mit Zustand `READY TO SEND`, `MORE DATA` oder `STALE`;
- je Befund die Datenfelder aus M-15 und M-16: die sechs Teile, die drei
  Anzeigezeilen, `evidence_ids` und die Intent-Revision;
- je Befund ein **optionales Objekt `maskierung`** mit `quelle_a`, `quelle_b`,
  `band_von`, `band_bis`, `wert_db`, `gueltig` und `herabgesetzt`.

**Die Maskierung hängt am Finding, nicht in einer zweiten Liste.** Der
Blueprint nennt sie „zweite Darstellung desselben Finding-Datenwegs" (M-41),
und zwei Listen könnten auseinanderlaufen.

**Der Name lautet `maskierung`, nicht `ueberdeckung`** — die Kollision mit der
Zeitüberdeckung ist in §2.12 gemessen.

**Wie der Plantext-Satz „der Name gehört in die v3-Vertragsrunde von S16–17"
erfüllt wird:** S16–17 ist abgenommen. Der Name wird deshalb **in diesem
Ticket** als belegtes Feld in
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` eingetragen, in
`eq-copilot/schemas/v3/README.md` als Fassungsschritt geführt und mit
Cross-Language-Fixtures (C++ und Rust), Negativfixtures und der Fassungsleiter
von **A5** bewacht — genau der Weg, den SONDE-013 für `experimente` und
`paare` gegangen ist. Ein Leser der Fassung 2 sieht das Feld nicht; Abwesenheit
heißt „nichts", nie `null`.

**Damit entfällt die Sperre aus §5.4, Zeile 1:** Etappe 2 darf den Rückweg
bauen. Die Plantexte (Überdeckung → Maskierung) zieht der Dirigent beim
Abschluss nach; das ist nicht Auftrag dieser Etappe. Matrixzeilen: **M-36**
(BELEGT · ENTSCHIEDEN · BAULÜCKE), **M-40** (ENTSCHIEDEN · BAULÜCKE),
**M-41** (BELEGT).

### 4.5 E-05 — Wo Zielbereich und Schutzbereiche im Experimentvertrag stehen (M-48, NAK-168)

**Frage.** NAK-168, wörtlich: „der Vorschlag benennt Zielbereich und
Schutzbereiche, das Experiment liest sie." Heute trägt keine Experimentfamilie
ein Feld dafür (§2.6, am Schema gemessen), und `experiment_verdrahtung.rs`
rät den Zielbereich aus dem größten Betrag der Banddeltas (:1069–1088).

**Vorschlag (A) — das Feld hängt an `experiment_begin`, nicht am Proposal.**
`experiment_begin` trägt bereits die **vollständige** Passage (seit SONDE-013
Nacharbeit 1, Befund B23) und die Referenz. Ein zusätzliches, optionales
Objekt `ziel` mit `band_von`, `band_bis`, `proposal_id` und
`geschuetzte_baender` (Liste von Bandintervallen) liegt an derselben Stelle wie
die übrigen unveränderlichen Referenzen des Versuchs und überlebt damit
Reconnect und Neustart über den bestehenden Store-Pfad.

**Begründung.** Der Zielbereich ist eine **Eigenschaft des Versuchs**, nicht
des Vorschlags allein: derselbe Vorschlag kann in zwei Passagen zwei Versuche
erzeugen, und die Guardrail-Rechnung sitzt im Experimentpfad. Ein Feld am
Proposal allein erreichte den Rechner nie, weil der Coordinator das Proposal
beim Guardrail-Rechnen nicht in der Hand hat.

**Alternative (B).** Nur `proposal_id` an `experiment_begin`; der Coordinator
schlägt Ziel und Schutz im Store nach. Vorteil: ein Feld statt drei, keine
Duplikation. Nachteil: eine Rücknahme des Proposals machte einen **laufenden**
Versuch unlesbar, und die Zusage „unveränderliche Referenzen" (§43.1) wäre
verletzt — der Versuch hinge an einem Objekt, das sich ändern kann.

**Empfehlung: (A)**, mit `proposal_id` **zusätzlich** als Rückverweis. Damit
sind die Zahlen eingefroren und die Herkunft bleibt nachvollziehbar.

**Kann der Bau ohne diesen Entscheid ehrlich fortfahren?** **Nein für M-48.**
Die Proposal-Hälfte (M-47) ist baubar; die Experiment-Hälfte braucht das Feld,
sonst bleibt die Zielheuristik stehen und NAK-168 wäre nur halb geschlossen.


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-05 angenommen in Fassung (A), mit
`proposal_id` als Rückverweis.** `experiment_begin` bekommt in **derselben
Fassung 3** ein optionales Objekt `ziel` mit `band_von`, `band_bis`,
`geschuetzte_baender` (Liste von Bandintervallen) und `proposal_id`.

**Beide Pfade sind vertraglich benannt und beide werden gemessen:**

| Fall | Verhalten | Kennzeichen im Resultat |
|---|---|---|
| **mit** `ziel` | Der Guardrail-Rechner liest Ziel- und Schutzbereiche und rät nichts | keines |
| **ohne** `ziel` (manueller Versuch ohne Vorschlag, SONDE-013-Pfad) | Die heutige Heuristik bleibt (`experiment_verdrahtung.rs`:1069–1088) | `ziel_geraten` |

**Zwei Rotbeweise, je einer pro Pfad:** die Heuristik läuft trotz gesetztem
`ziel`; und ein Resultat ohne `ziel` trägt kein `ziel_geraten`.
Matrixzeile: **M-48** (ENTSCHIEDEN · BAULÜCKE); NAK-168 ist damit in beiden
Hälften adressiert (M-47 für die Vorschlagsseite).

### 4.6 E-06 — Wie der Rückweg eines Proposals heißt (M-43)

**Frage.** Das Exit-Gate verlangt, dass jedes Proposal einen **Rückweg** nennt.
§42.1 führt `stop_if`, aber kein Rückwegfeld; §43.1 nennt für den manuellen
Versuch ausdrücklich „erlaubt kein Nakama-Revert" und „manuell · nicht
automatisch wiederherstellbar".

**Vorschlag.** Ein Feld `revert` mit einer **geschlossenen** Wertemenge:
`experiment_abort` (der Versuch wird terminal verworfen), `manual_only` (der
User stellt im fremden Werkzeug selbst zurück; kein Nakama-Revert),
`none_needed` (bei `keine Änderung` und `mehr Daten`). In P5 kommt
`dsp_revert` nicht vor — der Wert wird für P7 (`revert_transaction`, bereits
reserviert für SONDE-016) freigehalten und ist in P5 ein Vertragsbruch, genau
wie `execution = previewable` (M-45).

**Begründung.** Der Rückweg als Feld mit geschlossener Menge ist prüfbar; als
Textbaustein ist er es nicht. Die drei Werte decken die P5-Fälle vollständig.


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-06 angenommen.** Feld `revert`
mit der geschlossenen Menge `experiment_abort`, `manual_only`, `none_needed`.
`dsp_revert` gehört P7 (`revert_transaction`, reserviert für SONDE-016) und
ist in P5 ein Vertragsbruch — dieselbe Klasse wie `execution = previewable`
(M-45).

**Test:** jeder der drei Werte einem Proposal-Typ zugeordnet; ein vierter Wert
fällt **in C++ und in Rust** (M-77). Matrixzeile: **M-43**
(BELEGT · ENTSCHIEDEN · BAULÜCKE).

### 4.7 E-07 — Ob `Preview` ein übersprungener oder ein fehlender Zustand ist (M-55)

**Vorschlag.** Übersprungen, nicht gelöscht: die Zustandsmenge behält alle acht
Namen aus §46.1, und die **Übergangstabelle** von P5 führt keine Kante nach
`Preview`. Damit bleibt der Vertrag zwischen P5 und P6/P7 stabil und ein
P6-Ticket fügt nur Kanten hinzu statt einen Zustand.

**Alternative.** `Preview` entfällt aus der Menge und kommt in P6 zurück.
Nachteil: eine gespeicherte P5-Zustandsmaschine könnte einen P6-Stand nicht
lesen, und der `AssistantStep` ist persistent — das wäre ein Migrationsschritt
für nichts.


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-07 angenommen.** `Preview`
bleibt in der Zustandsmenge, die P5-Übergangstabelle führt keine Kante
dorthin, und ein gespeicherter `AssistantStep` mit `Preview` ist in P5 ein
**Lesefehler**, kein stiller Sprung. Der letzte Halbsatz ist die Präzisierung
gegenüber dem Vorschlag: „übersprungen" allein ließe offen, was beim Lesen
eines fremden Stands geschieht. Matrixzeile: **M-55** (BELEGT · ENTSCHIEDEN).

### 4.8 E-08 und E-09 — Modulschnitt und Rückstau-Schlüssel (M-71, M-73)

**E-08 Modulschnitt.** Vorschlag: fünf neue Module unter
`broker/src/coordinator/` — `intent.rs` (Rollen, Konfliktregeln,
Zyklusprüfung), `hypothese.rs` (Kandidaten, Rang, `CauseHypothesis`),
`proposal.rs` (Aktionstemplates, Constraint-Solver, Budgets),
`assistent.rs` (Zustandsmaschine) und `maskierung.rs` (der Wert aus M-36) —
plus je eine Verdrahtungsdatei nach dem bestehenden Muster
(`experiment.rs`/`experiment_verdrahtung.rs`,
`prepost.rs`/`prepost_verdrahtung.rs`,
`invalidierung.rs`/`invalidierung_verdrahtung.rs`). Begründung: SONDE-013 hat
diese Trennung eingeführt, weil die Wiederprüfung 1 der Etappe 2 fand, dass
Tests, die das Modul direkt rufen, nicht fallen können — die Verdrahtungsdatei
ist der Ort, an dem der Produktpfad geschlossen wird. Die Aufteilung folgt
außerdem NAK-121 (verhaltensneutrale Modulaufteilung je Fachbereich).

**E-09 Rückstau-Schlüssel.** Vorschlag: `draft_offer` reist als P1 mit
Schlüssel `proposal:<proposal_id>` (koalesziert Neufassungen desselben
Vorschlags), `user_verdict` als **persistenzpflichtiger P0** (`sendePersistenzP0`),
weil ein Userurteil nicht koaleszieren darf und über einen Brokerkill hinweg
mit derselben `command_id` wiederholt werden muss — genau das Muster, das
**A4-SI** heute misst. Findings reisen ohne eigene Familie über den
`session_snapshot`-Rückweg (E-04) und erben dessen Koaleszierung.


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-08 angenommen, mit Zuweisung
der Autoritäten nach §33.5.** Der Vorschlag oben nannte fünf Module und ihre
Verdrahtung; der Entscheid legt zusätzlich fest, **wer die Wahrheit hält**:

| Gegenstand | Autoritative Stelle | Rolle des Brokers |
|---|---|---|
| Validierung des Intents: Rollenmenge, Zyklenprüfung, Konfliktauflösung beim Setzen (M-05, M-06, M-13) | **Main, also das Gen-Plugin in C++**, persistent im `MainProjectState` | `intent.rs` ist **Spiegel und Vertragsvalidierung beim Empfang** (M-77: dieselbe Rollenmenge, dieselben Enumgrenzen beidseitig) und wertet die Konfliktregeln beim **Rechnen** aus |
| Zustandsmaschine des `AssistantStep`: Übergänge, Abbruch, Zurück, Überspringen, Resume, Rekonstruktion (M-55 bis M-62) | **Main**, persistent im `MainProjectState` | `assistent.rs` ist Spiegel und Vertragsvalidierung — **keine zweite Zustandsmaschine** |
| Hypothese, Proposal, Maskierung | **Broker-Coordinator** (`hypothese.rs`, `proposal.rs`, `maskierung.rs`) | dort liegen Evidenzbestand und Store |

Je Modul eine Verdrahtungsdatei nach dem SONDE-013-Muster; die Produktpfade
werden **in der Verdrahtung** geschlossen, und Tests rufen die Verdrahtung,
nicht das Modul (Risiko R1 in §5.3). Matrixzeile: **M-71**
(BELEGT · ENTSCHIEDEN).

**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **E-09 angenommen.** `draft_offer`
(reserviert, wird belegt) reist als **P1 mit Schlüssel
`proposal:<proposal_id>`**; `user_verdict` (reserviert, wird belegt) als
**persistenzpflichtiger P0** über `sendePersistenzP0`, mit Wiederholung unter
derselben `command_id` über einen Brokerkill hinweg — das **A4-SI**-Muster.
Findings und Maskierungen reisen über `session_snapshot.findings` (E-04) und
erben dessen Koaleszierung. Es entsteht keine neue Prioritätsklasse.
Matrixzeile: **M-73** (BELEGT · ENTSCHIEDEN · BAULÜCKE).

**Matrixnacharbeit 1 (06.09.2026).** Die Matrixprüfung 1 hat an E-08 einen
vierten Defekt gefunden: der von Entwurf §33.5 (Zeile 2147) zugesagte
**versionierte SQLite-Spiegel des aktuellen `AssistantStep`** hatte keinen
Produktpfad. Der Entscheid **E-11** (Regel R3, §4.13) schließt ihn mit der
Schwesterfamilie `assistant_step_update` und dem Ein-Writer-Spiegel; M-71 ist
entsprechend nachgezogen, neu sind **M-88** und **M-89**.

### 4.9 E-10 — Transport des Intents vom Main zum Broker (Lücke, M-85, M-86)

**Diese Lücke stammt nicht aus Etappe 1, sondern aus dem Entscheid vom
06.09.2026.** Der Dirigent hat sie an der Matrix gefunden: §3.1 legt den Intent
in den Main-State (M-09, M-71), §3.2 und §3.5 lassen den Broker damit rechnen
(M-12, M-20, M-44, M-54) — aber keine Zeile nannte eine **Nachricht** dafür,
und `intent` ist im Register frei (§2.6). Etappe 1 hatte den Ort entschieden
und den Weg dorthin übersehen.

**Etappe 1b (Entscheid des Dirigenten, 06.09.2026) — Regel:**

- Der Intent reist vom Main zum Broker über eine **eigene Familie**, die dieses
  Ticket in `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` reserviert
  **und** belegt.
- **Klasse P1** mit Schlüssel `intent:<quelle_id>:<passage_id oder global>`.
  Koaleszierung ist zulässig, weil nur die jüngste Revision wirkt (M-10).
- **Nach Verbindungsaufbau und nach jedem Brokerneustart** meldet Main seinen
  **vollständigen** Intent-Bestand — auch einen leeren — mit einer
  **Vollständigkeitsmarke samt Revision**, **bevor** der Broker eine Hypothese
  oder ein Proposal rechnet (Reihenfolge nach M-75 und M-76). Bis diese Marke
  vorliegt, entstehen **keine** Proposals; der Zustand ist `MORE DATA`.
- **Grund:** ein Veto — `geschützt`, `bewusst verschmolzen` — darf durch
  Transportverlust nie verschwinden. Ein fehlender Intent sähe sonst aus wie
  „kein Schutz gewünscht".

**Name der Familie: `intent_update`.** Der Entscheid gibt den Namen als
Vorschlag und überlässt der Matrix die Begründung im Muster der bestehenden
Familiennamen. Gemessen an `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`
benennt der Vertrag Familien nach **Gegenstand plus Vorgang**:
`session_command`, `evidence_invalidate`, `experiment_begin`,
`experiment_candidate`, `experiment_abort`, `state_report`, `command_ack`,
`preview_renew`. `intent_update` folgt demselben Schnitt, grenzt sich eindeutig
gegen `state_report` (Plugin-Identität und bestätigter Zustand, nicht
musikalische Absicht) und gegen `session_command` (Sessiongraph, nicht Intent)
ab und sagt zugleich, dass es eine **Fortschreibung mit Revision** ist und
keine Anfrage. Ein Alternativname `intent_report` wäre irreführend, weil
`report` im Vertrag für den Zustand einer Plugininstanz steht, nicht für eine
Useranweisung.

**Beide Matrixzeilen tragen BAULÜCKE mit Entscheid:** **M-85** (Transport,
Schlüssel, Koaleszierung, Revisionsordnung — Test **A4**-Verdrahtungsfall plus
**B10**; Rotbeweis: eine ältere Revision überschreibt nach der Koaleszierung
eine jüngere) und **M-86** (Vollständigkeitsmarke vor jeder Rechnung — Test
**A4** Neustartfall; Rotbeweis: ein Proposal entsteht vor der Marke).

**Die Fassung 3 ist EIN Fassungsschritt.** `session_snapshot.findings` (E-04),
`experiment_begin.ziel` (E-05), die Belegung von `draft_offer` und
`user_verdict` (E-09) und die Reservierung samt Belegung von `intent_update`
(E-10) liegen zusammen in Wire-Envelope-`schema_minor` 3 und werden in **einer**
Bauetappe gebaut, nicht in vier (§5.1, Etappe B).

### 4.10 Produktfragen — benannt, einer Fläche zugeordnet, hier nicht beantwortet

| Nr. | Frage | Fläche | Warum sie hier nicht beantwortet wird |
|---|---|---|---|
| P-01 | Wie ein Befund entsteht, wenn der User **keine** Passage markiert hat — arbeitet der Advisor auf dem laufenden Fenster, oder verlangt er erst eine Passage? | Gen Fläche 1 | Der Assistent darf ausdrücklich `erst Passage messen` sagen (M-61). Ob das der Normalfall oder die Ausnahme ist, ist eine Bedienentscheidung. Berührt Karte **U28** (Passagengeste), die offen ist und nicht neu gestellt wird. |
| P-02 | Wie viele Befunde gleichzeitig offen sein dürfen, bevor die Liste selbst ein Problem wird | Gen Fläche 1 | Der Blueprint entscheidet Scrollliste und Filter für **Quellen** (§4.1, Fünferblock 04 5/5), nicht für Befunde. Ein Deckel wäre eine sichtbare Priorität. |
| P-03 | Ob ein Befund, dessen Evidenz zurückgenommen wurde, aus der Liste **verschwindet** oder als `STALE` stehen bleibt | Gen Fläche 1 | M-24 legt fest, **dass** invalidiert wird; ob der Befund sichtbar bleibt, ist Anzeigepriorität. Der Datenweg liefert beides. |
| P-04 | Ob der Advisor von sich aus einen Befund in den Fokus zieht, wenn ein neuer, sicherer entsteht | Gen Fläche 1 | „Normal ist genau ein Finding im Fokus" (Abnahme 31.08.); wer den Fokus wechselt, steht dort nicht. Grenzt an das Grundgesetz „nichts Ungefragtes" (M-78) und ist deshalb eine Produktfrage, keine Technikfrage. |

Diese vier Punkte blockieren Etappe 2 **nicht**: der Datenweg liefert in allen
vier Fällen beide Möglichkeiten, und keine der Zusagen aus §3 hängt an der
Antwort. Sie gehören datiert nach `docs/offene-punkte.md`, wenn der Dirigent
dieses Manifest abschließt — nicht von dieser Etappe geschrieben (Grenze).


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **Ohne Änderung
angenommen.** P-01 bis P-04 bleiben Produktfragen; der Dirigent zieht sie beim
Abschluss ins Register beziehungsweise als Karten nach `docs/plan/fragen.json`
nach. Etappe 2 baut sie nicht und beantwortet sie nicht.

### 4.11 Nebenbefunde in fremden Dateien (nicht geändert)

| Nr. | Befund | Ort | Warum hier nur notiert |
|---|---|---|---|
| N-01 | NAK-168 zitiert die Zielheuristik als `experiment_verdrahtung.rs:934-953`. Zum Stand `5e99a13` steht dort `mittel_je_band`; die Heuristik liegt bei :1069–1088. Die Zeilen sind durch `00fa14e` (NAK-182 B6) verrutscht. Die **Aussage** der Registerzeile stimmt. | `docs/offene-punkte.md`:180 | Fremde Datei; die Grenze dieser Etappe erlaubt keinen Eingriff in `docs/offene-punkte.md`. |
| N-02 | Der Zustandsbaum in der Schemadatei führt unter `MainProject` nur `confirmed_members_v1`. Die Eigenschaft `manual_passages_v1`, die `NakamaState.cpp`:36 schreibt und liest, fehlt dort; `grep -c "manual_passages"` liefert `0`. | `eq-copilot/schemas/state/nakama-state-v2.md` §2 | Schemadatei, außerhalb des Änderungssatzes dieser Etappe. Etappe 2 fasst sie ohnehin an (Intent, `AssistantStep`) und zieht die Zeile dann mit nach. |
| N-03 | `SourcesModel::findingsOffen` und `setzeFindings` haben im Produktcode keinen Aufrufer; einziger Aufrufer ist ein Test. Der Blueprint verlangt die Zahl. | `eq-copilot/plugin/src/SourcesModel.cpp`:1210 | Als Matrixzeile **M-84** geführt (BAULÜCKE), damit Etappe 2 sie schließt oder die Anzeige ehrlich unterlässt. |
| N-04 | Der Begriff „Überdeckung" trägt im Repo drei Bedeutungen (Messabdeckung, Zeitüberdeckung, spektrale Maskierung), von denen die dritte im Code nicht existiert. Der Blueprint nennt die dritte bereits **Maskierungszone**; Plan- und Bauaufteilungstext nennen sie weiter „Überdeckung". | `docs/plan/plan.json` S23–25, `docs/bauaufteilung-sonden.md`:394 gegen `design/docs/funktions-und-bedien-blueprint.md` §3.4 | Kein Widerspruch in der **Sache** — beide meinen dasselbe. Diese Matrix hält die Wörter getrennt (§2.12) und schlägt den Feldnamen entsprechend vor (E-04). Ein Angleichen der Plantexte ist Dirigentenarbeit, nicht Bauarbeit. |
| N-05 | Das Feld `gesamt_erwartet` steht auf **29** und wird von **A5** gegen das `oneOf` geprüft; der Fließtext im Feld `zweck` derselben Datei rechnet 26 + `session_command` + `experiment_manual_result` = **28** und lässt `experiment_candidate` aus, das seit Nacharbeit 2 (Befunde R16/R21) belegt ist und in `belegte_nachrichten` steht. Nur die Prosa ist hinterher, nicht der Vertrag. | `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`, Feld `zweck` | Vertragsdatei, außerhalb des Änderungssatzes dieser Etappe. Etappe 2 fasst sie an, sobald ein Name für dieses Ticket hinzukommt (E-04, E-05), und zieht den Satz dann mit nach. |


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **Ohne Änderung
angenommen.** N-01 bis N-05 gehen beim Abschluss ins Register. **N-02**
(`manual_passages_v1` fehlt im Zustandsbaum der Schemadatei) und **N-05**
(die `zweck`-Prosa rechnet noch auf 28 Namen) zieht **Etappe 2 mit nach**, wie
in diesem Manifest vorgeschlagen — sie fasst beide Dateien für die Fassung 3
ohnehin an.

### 4.12 Widerspruch Entwurf ↔ Blueprint

**Kein tragender Widerspruch gefunden.** Geprüft wurden Entwurf §8, §9, §36,
§37, §42, §43, §46 und §59 gegen Blueprint §3.4, §4.1, §11 und §12.1. Zwei
Stellen, an denen der Blueprint **präziser** ist als der Entwurf, ohne ihm zu
widersprechen — beide zugunsten des Blueprints gelesen, weil er die
Funktions- und Bedienarchitektur bindet:

1. Der Entwurf nennt die Sache in §59 nicht; der Blueprint nennt sie
   „frequenzaufgelöster Datenweg für die Maskierungszone" (§3.4) und legt fest,
   sie sei „eine zweite Darstellung desselben Finding-Datenwegs, keine eigene
   Detektion und keine konkurrierende Schwelle". Das ist die Auflösung der
   Namenskollision aus §2.12 und steht als M-41 in der Matrix.
2. Der Entwurf §46.1 führt `Preview` in der Zustandsfolge; der Blueprint
   ordnet Preview und Kandidat ausdrücklich P6/P7 zu (§3.5, §3.6). Daraus folgt
   M-55 (`Preview` in P5 übersprungen, nicht gelöscht) und E-07.

Über beiden stehen der aktuelle Vertrag und der Code (`CLAUDE.md`). Wo diese
Matrix von einer Spezifikation abweicht, tut sie es nur zugunsten einer
Messung, und die Messung steht in §2.


**Etappe 1b (Entscheid des Dirigenten, 06.09.2026).** **Ohne Änderung angenommen.**

### 4.13 Befunde der Matrixprüfung 1 (06.09.2026)

**Matrixprüfung 1** (Codex `gpt-6-astra`, Effort max, lesend, Thread
`01a0776e-ed49-7fd2-afaa-992997f6bf3e`; `HEAD` vor und nach dem Lauf `fdb04e4`)
endet mit **URTEIL: NEEDS_WORK** und **vier Defekten**. Der Dirigent hat jeden
an der Quelle bestätigt. Auftrag
`docs/beweise/roh/SONDE-014-matrixpruefung-1-auftrag.txt`, Rohurteil
`docs/beweise/roh/SONDE-014-matrixpruefung-1-fdb04e4.txt`, Nacharbeitsauftrag
mit den Regeln R1 bis R4
`docs/beweise/roh/SONDE-014-matrixnacharbeit-1-auftrag.txt`.

Alle vier Befunde treffen **dieselbe Klasse**: eine Zusage stand in der Matrix,
und der daneben genannte Test hätte sie nicht gemessen — dreimal, weil der
Rotbeweis an einem Nebeneffekt gefallen wäre, einmal, weil ein
Lieferumfangspunkt gar keine Zeile hatte. Das ist genau der Fehlertyp, gegen den
die Rotbeweis-Spalte in §3 steht.

#### D1 (M-23) — „zu kurz" war nicht gemessen

**Matrixnacharbeit 1 (06.09.2026).** Wortlaut des Prüfers:

> „Zwei identische, beliebig kurze Passagen können sämtliche genannten
> Vergleichbarkeitsgates bestehen: broker/src/coordinator/vergleichbarkeit.rs:168–185,280–359
> prüft relative Überdeckung, aber keine Mindestdauer. Damit deckt dieser
> Testweg die Zusage aus docs/FL-Nakama-Sonden-Design-Entwurf.md:2415–2416 für
> „zu kurz" nicht ab. Außerdem ist claim_class = 3 bereits durch M-17/M-45 in
> P5 ausgeschlossen; der angegebene Rotbeweis kann deshalb am Preview-Verbot
> statt am Passagengate fallen. Benenne einen messbaren
> Dauer-/Stichprobenriegel und einen Rotfall mit einem ansonsten zulässigen
> P5-Ergebnis."

**An der Quelle bestätigt.** `ueberdeckung` (`vergleichbarkeit.rs`:168–185)
rechnet `gemeinsam / kuerzer` und ist damit rein relativ; zwei identische
Fenster ergeben 1,0 unabhängig von ihrer Länge. Eine Mindestdauer oder
Mindeststichprobe gibt es in der Datei nicht. Der zweite Teil des Befunds
stimmt ebenso: `claim_class = 3` verlangt eine kontrollierte Preview, und M-45
verbietet jede Preview in P5 — der alte Rotbeweis wäre daran gefallen, nicht am
Passagengate.

**Regel R1, wörtlich übernommen und in M-23 eingearbeitet:** „zu kurz" ist
keine Wanddauer, sondern die **gemessene Mindestzahl unabhängiger
Evidenzfenster je beteiligter Quelle innerhalb der Passage** — konsistent mit
`vergleichbarkeit.rs`:108–109 („GEMESSENE Signalabdeckung … nicht die
Wanddauer"). Die Zahl steht als **eine** benannte Konstante
`GATE_MINDEST_FENSTER` neben den vier bestehenden Gates in derselben Datei;
**keine zweite Schwellenfamilie**. **Startwert 8**, begründet aus der
Evidenzkadenz (`kEvidenzIntervallMinS` = 0,25 s ⇒ bis zu vier Fenster je
Sekunde, bei Kadenzreduktion eines; acht Fenster sind 2 bis 8 s und geben dem
Block-Bootstrap mit `BOOTSTRAP_BLOCK` = 4 mindestens zwei Blöcke — dieselbe
Untergrenze, die SONDE-013 M-11 für die Welch-Mittelung setzt). Kalibriert wird
er mit dem P5-Korpus; die festziehenden Fälle sind die Sessions „zu kurze
Passage" und „verschobene Passage" aus M-65. Unter der Schwelle erreicht kein
Befund Sicherheit `hoch` und `READY TO SEND`; das Ergebnis ist `MORE DATA` mit
dem Ausschlussgrund `passage_zu_kurz` (M-87).

**Geänderte Zeilen:** **M-23** vollständig neu — Zusage, Belegklasse
(BELEGT · ENTSCHIEDEN · BAULÜCKE), Test (zusätzlicher A4-Verdrahtungsfall für
`GATE_MINDEST_FENSTER`) und **zwei** Rotbeweise, beide mit einem ansonsten
zulässigen P5-Ergebnis, damit sie am Passagengate fallen und nicht am
Preview-Verbot.

#### D2 (M-65) — der Korpus maß Hygiene, nicht die Unterscheidung

**Matrixnacharbeit 1 (06.09.2026).** Wortlaut des Prüfers:

> „Der festgelegte Rotbeweis entfernt einen Quellenbezeichner, misst aber nicht
> die zugesagte Unterscheidung von Kandidat, Distraktor und Parent. Das
> benannte A25-Muster serialisiert deklarierte aussage-/konfidenz-Werte und
> prüft Quellenstellen (tools/eq-copilot/erzeuge_p4_korpus.py:278–326); eine
> falsche starke Produktbehauptung verändert diese Angaben nicht automatisch.
> Damit bleibt für NAK-190 der messende Korpusweg aus
> docs/FL-Nakama-Sonden-Design-Entwurf.md:2413–2414 offen. Benenne die
> Auswertung derselben Session über die Coordinator-Verdrahtung und einen
> Rotbeweis, der bei unveränderten Quellenangaben an der tatsächlich
> ausgegebenen falschen Ursache fällt."

**An der Quelle bestätigt — und der Korpus sagt es selbst.**
`erzeuge_p4_korpus.py`:277–318 validiert `bein`, `fall`, `quelle`, `wahrheit`,
`aussage` und `konfidenz` gegen geschlossene Mengen, prüft, dass die Quelldatei
existiert, und sucht den Bezeichner wörtlich darin. Das ist
Deklarationshygiene: alle sechs Angaben stehen im Korpus und ändern sich nicht,
wenn der Produktpfad eine falsche Ursache ausgibt. Der Erzeuger schreibt
denselben Vorbehalt in das Feld `zweck` des Manifests (`:325–326`), wörtlich:

> „Der Korpus misst NICHT selbst - er waere sonst eine fuenfte Implementierung."

Für P4 war das richtig: dort misst jeder Fall über ein bestehendes Bein. Für P5
ist es der Defekt — Entwurf §36.4 Satz 1 verlangt, dass **synthetische Sessions
mit bekannter Quelle** wahren Kandidaten, korrelierten Distraktor und
Parent-Duplikat **unterscheiden**, und diese Unterscheidung trifft niemand
außer dem Produktpfad selbst.

**Regel R2, wörtlich übernommen und in M-65 eingearbeitet:** Der P5-Korpus ist
eine **Kette**, kein Deklarationsregister. Jede synthetische Session läuft als
Evidenzbestand durch die **Coordinator-Verdrahtung** (`hypothese_verdrahtung.rs`,
Produktpfad); gelesen wird die **tatsächlich ausgegebene** Hypothese
(`candidate_source`, `confidence.class`, `alternatives`, `ausschluesse`) und
gegen die Wahrheit des Falls gehalten. Riegel: eine starke Behauptung auf
Distraktor oder Parent-Duplikat ist `falsche_starke` und rot; der korrelierte
Distraktor **muss** in `alternatives` erscheinen (NAK-190, SONDE-013 M-85
Ziel 1); `precision` ≤ 1, `recall` ≤ 1, geschlossene Mengen nach NAK-182 R1a.
Die Quellenhygiene bleibt als **eigener** Riegel bestehen, ist aber nicht der
Beleg für Entwurf §36.4 Satz 1.

**Geänderte Zeilen:** **M-65** vollständig neu — Zusage, Belegklasse
(BAULÜCKE · ENTSCHIEDEN), Test in zwei benannten Teilen (messender Weg als
Rust-Integrationstest im A4-Bereich, der die Sessions fährt und die Korpusdatei
als **Erwartung** liest; Hygieneriegel nach A25-Muster) und zwei Rotbeweise mit
ausdrücklicher Rangfolge — der primäre fällt am Produktpfad bei **unveränderten**
Korpus-Quellenangaben.

#### D3 (M-71, E-08) — der zugesagte Spiegel hatte keinen Weg

**Matrixnacharbeit 1 (06.09.2026).** Wortlaut des Prüfers:

> „Für einen im Main gespeicherten Schrittwechsel ohne Experimentabschluss oder
> Userurteil fehlt der benannte Weg zum zugesagten SQLite-Spiegel. E-10
> transportiert nur Intent; für keine Erweiterung in Bauetappe B ist
> AssistantStep als Nutzlast festgelegt, und state_report trägt ausschließlich
> den geschlossenen Zustandsbericht aus
> eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1771–1793. Der versionierte
> Spiegel des aktuellen Schritts aus
> docs/FL-Nakama-Sonden-Design-Entwurf.md:2147 ist damit nicht durch einen
> Produktpfad samt Rotbeweis gedeckt. Ergänze Transport und
> Spiegel-/Reconnect-Test bereits zur Vertragsplanung; Writer und beidseitige
> Verbraucher gehören zusammen."

**An der Quelle bestätigt.** `$defs/state_report` ist
`additionalProperties: false` und trägt genau `type`, `adresse`,
`dsp_schema_version`, `state_revision`, `state_hash`, `record_state` und
`undo_tiefe` — keinen Schritt. Entwurf §33.5, Zeile 2147, weist dem
`MainProjectState` den aktuellen `AssistantStep` zu und nennt daneben
ausdrücklich den „versionierten SQLite-Spiegel für Suche und Crashdiagnose".
Etappe 1b hatte den Ort entschieden (E-08) und den Weg dorthin nicht benannt —
derselbe Fehler wie bei E-10, eine Stufe später.

**Regel R3 — Entscheid E-11 des Dirigenten, wörtlich übernommen:** Der
`AssistantStep` reist vom Main zum Broker über eine **Schwesterfamilie** von
`intent_update` mit demselben Muster — **`assistant_step_update`**, Klasse P1
mit Schlüssel `assistant_step:<session_id>` (koaleszierend, nur die jüngste
Revision wirkt), in **derselben Fassung 3** reserviert und belegt. Die
Vollständigkeitsmarke aus E-10 und M-86 deckt **beide** Bestände: Main meldet
nach Verbindungsaufbau und nach jedem Brokerneustart Intent-Bestand **und**
aktuellen Schritt, bevor der Broker rechnet. Der Broker spiegelt den Schritt
versioniert in den Store über den bestehenden Ein-Writer-Pfad
(`broker/src/store/writer.rs`); **der Spiegel ist nie autoritativ** — bei
Widerspruch gewinnt der neuere Main-State, und ein alter Datenbankeintrag setzt
nie einen neueren Plugin-State zurück (§33.5, M-59, M-76).

**Geänderte und neue Zeilen:** **M-71** um den Weg und den Verweis auf E-11
ergänzt (Belegklasse jetzt BELEGT · ENTSCHIEDEN mit E-08 **und** E-11); neu
**M-88** (Transport, Schlüssel, Koaleszierung, Revisionsordnung) und **M-89**
(Spiegel und Reconnect), beide BAULÜCKE · ENTSCHIEDEN. §3.10 heißt seitdem
„Transport von Intent und Assistentenschritt". §5.1 führt die Familie **und**
den Writer im selben Änderungssatz wie ihre beidseitigen Verbraucher
(Bauetappe B).

#### D4 (§5.1 C, M-14 bis M-28) — Ausschlussgründe hatten keine Zeile

**Matrixnacharbeit 1 (06.09.2026).** Wortlaut des Prüfers:

> „Lieferumfangspunkt 2 verlangt ausdrücklich Ausschlussgründe
> (docs/FL-Nakama-Sonden-Design-Entwurf.md:4311), aber keine Matrixzeile misst
> deren Ausgabe oder Erhaltung im Evidenzgraphen. M-20 prüft lediglich, dass
> Coverage und Alignment nicht überstimmt werden; M-15 prüft die zehn
> Hypothesenfelder. Eine Umsetzung, die ausgeschlossene Kandidaten
> kommentarlos entfernt, erfüllt diese Nachweise, lässt aber diesen
> Lieferumfangspunkt aus. Ergänze eine Zeile mit messendem Test und Rotbeweis
> für den fehlenden beziehungsweise falschen Ausschlussgrund."

**An der Quelle bestätigt.** Entwurf Zeile 4311 lautet wörtlich „versionierter
Evidenzgraph, `CauseHypothesis`, Alternativen, **Ausschlussgründe** und
Konfidenz". §36.3 führt `alternatives`, aber kein Ausschlussfeld; M-15 zählt
die zehn Felder aus §36.3 auf, M-20 misst die Rangkomponenten. Keine der
fünfzehn Zeilen aus §3.2 verlangte, dass ein ausgeschiedener Kandidat einen
Grund trägt.

**Regel R4, wörtlich übernommen und als M-87 eingearbeitet:** Jeder Kandidat,
der aus dem Ranking ausscheidet, trägt einen Ausschlussgrund aus einer
**geschlossenen Menge**. Die Matrix legt sie mit acht Werten fest:
`coverage_fehlt`, `alignment_falsch`, `passage_unvergleichbar`,
`passage_zu_kurz`, `intent_veto_geschuetzt`, `intent_veto_verschmolzen`,
`capability_fehlt`, `evidenz_zurueckgenommen`. Sie lebt an **einer** Stelle im
Vertrag; C++ und Rust prüfen dieselbe (M-77). Die Ausschlüsse reisen mit der
Hypothese (`ausschluesse`: Liste aus `candidate_source` und `grund`, Teil des
Befunds in `session_snapshot.findings`) und werden im Store beim Finding
erhalten. Ein kommentarlos entfernter Kandidat ist ein Defekt. **Ob und wie Gen
die Ausschlüsse zeigt, ist Anzeige (S31b) und wird hier nicht entschieden; der
Datenweg liefert sie.**

**Woher die acht Werte kommen** — jeder ist an eine bestehende Matrixzeile
gebunden, keiner ist erfunden:

| Grund | Zeile, die ihn erzeugt |
|---|---|
| `coverage_fehlt` | M-20 (keine Komponente kompensiert fehlende Coverage) |
| `alignment_falsch` | M-20 (dasselbe für falsches Alignment) |
| `passage_unvergleichbar` | M-23 (verschoben oder andere Passage) |
| `passage_zu_kurz` | M-23 (`GATE_MINDEST_FENSTER`, R1) |
| `intent_veto_geschuetzt` | M-03 (Schutzangabe an der Quelle) |
| `intent_veto_verschmolzen` | M-04 (Verschmelzungs-Erlaubnis als Veto) |
| `capability_fehlt` | M-44 und M-52 (Constraint-Solver, Capability) |
| `evidenz_zurueckgenommen` | M-24 (deterministische Invalidierung) |

**Geänderte und neue Zeilen:** neu **M-87** in §3.2 (BELEGT · ENTSCHIEDEN ·
BAULÜCKE) mit beidseitigem Test, A5 für die Menge, A4-Storefall für die
Erhaltung und zwei Rotbeweisen (Kandidat ohne Grund entfernt; Grund außerhalb
der Menge angenommen).

#### Zwei stehengebliebene Prosazellen aus Etappe 1b, hier mitkorrigiert

Beim Nachziehen von §5.1 und §5.4 sind zwei Zellen aufgefallen, die den
Entscheiden aus Etappe 1b bereits widersprachen und deren Korrektur damals
nicht in die Datei gelangt ist:

- **Risiko R4** in §5.3 sagte noch „Etappe D hält vor dem Rückweg an (E-04);
  Etappe E hält vor dem Experimentfeld an (E-05)". Beide Sperren sind seit
  E-04 und E-05 aufgehoben, und §5.4 sagt das seitdem. R4 nennt jetzt das
  Risiko, das wirklich besteht: ein Fassungsschritt, der in mehrere Etappen
  zerfällt — entschärft dadurch, dass die Fassung 3 **eine** Bauetappe ist.
- **Prüfliste F** in §6 verwies auf „Etappe F trägt Abbruch, Zurück,
  Überspringen, Resume und Rekonstruktion zusammen". Nach der Neuordnung in
  Etappe 1b ist das Etappe **G**; Etappe **B** trägt seit R3 zusätzlich den
  Ein-Writer-Spiegel im selben Änderungssatz wie seine beidseitigen
  Verbraucher.

Beides ist keine fünfte Zusage, sondern die Beseitigung eines Widerspruchs
innerhalb dieses Manifests. Keine Matrixzeile ist dadurch entstanden oder
weggefallen.


---
## 5. Bauplan für Etappe 2

### 5.1 Reihenfolge der Bauetappen

**Nachgezogen in Etappe 1b (06.09.2026).** Die Reihenfolge folgt der
Abhängigkeit, nicht der Reihenfolge der Matrix. Jede Etappe schließt einen
**Änderungssatz** im Sinne von `CLAUDE.md` (speichern↔laden, starten↔stoppen,
verbinden↔trennen gehören zusammen) und endet mit dem vollen Kanon, sobald sie
Schema-, Fixture- oder Runnerdateien berührt.

**Die wichtigste Änderung gegenüber Etappe 1:** die **Fassung 3** des
Wire-Envelopes ist **ein** Fassungsschritt und liegt deshalb in **einer**
Bauetappe — nicht verteilt auf vier. Sie ist neue Etappe **B** und rückt vor
alles, was auf ihr aufsetzt. Die früheren Etappen B bis H rücken um einen
Buchstaben nach hinten.

| Etappe | Inhalt | Matrixzeilen | Kanonbeine am Ende der Etappe |
|---|---|---|---|
| **A — Intent im Main-State** | `SourceIntent` als additive Eigenschaft in `MainProject`: fünf Rollen als geschlossenes Feld `rolle` mit abgeleiteter §37.1-Belegung samt Veto-Kennzeichen (E-01a), orthogonale Schutzangaben, gerichtete Beziehungen, Passage-Scope, Revision/Herkunft/Konfidenz, Zyklenprüfung beim Speichern, Host-Dirty. Konfliktregeln in fünf Stufen mit „spezifisch vor global" innerhalb Stufe 2 (E-02). Writer, Reader, Migration, Goldens und Fixtures im **selben** Änderungssatz. Reine Main-Seite, **ohne** Wire. | M-01 bis M-13 | **NEU** `EqCopSonde014IntentTest`; bestehend **B2**, **B14**, **A12**, **A11**; voller Kanon (Fixtures berührt) |
| **B — Fassung 3 des Wire-Envelopes (EIN Fassungsschritt)** | Alle Vertragsteile aus den Entscheiden zusammen, weil sie **einen** `schema_minor`-Schritt bilden: `session_snapshot.findings` samt eingebettetem `maskierung` und `ausschluesse` (E-04, R4), `experiment_begin.ziel` mit `proposal_id` (E-05), Belegung der reservierten Namen `draft_offer` und `user_verdict` (E-09), Reservierung **und** Belegung von `intent_update` (E-10) sowie der Schwesterfamilie `assistant_step_update` (E-11). **Seit der Matrixnacharbeit 1 liegt der versionierte Spiegel des `AssistantStep` in derselben Etappe:** die Familie und der Ein-Writer-Pfad (`broker/src/store/writer.rs`) gehen mit ihren **beidseitigen Verbrauchern** in **einen** Änderungssatz (R3). Dazu ebenda: Schema, `reservierte-nachrichten-v1.json`, `eq-copilot/schemas/v3/README.md`, Cross-Language-Fixtures für C++ und Rust, Negativfixture je Feld, die geschlossene Ausschlussmenge (R4) und die **Fassungsleiter** — ein zurückgebauter Leser der Fassung 2 lehnt jede Neuerung der Fassung 3 ab. Nebenbefund **N-05** wird hier mit nachgezogen. | M-40, M-48 und M-73 (Vertragshälfte), M-85, M-88 und M-89 (Transport und Spiegel), M-87 (Ausschlussmenge auf dem Draht), M-15 und M-42 (Feldmengen), M-77 | **A5** (Textriegel, Fassungsleiter, Negativfixtures), **A8** (Fixture-Korpus), **A4** (Rust-Leser, Storefall für den Spiegel), **A4-SI** (Reconnect über einen Brokerkill), **B3c** `EqCopSchemaTest` (C++-Leser), **B10**, **A10**; voller Kanon |
| **C — Evidenzgraph und `CauseHypothesis`** | Kandidatenbildung Stufe A/B, sechs Rangkomponenten mit hartem Coverage-/Alignment-Gate, `CauseHypothesis` mit zehn Feldern und den sechs Teilen, Determinismus (Seed, Reduktionsreihenfolge, Quantisierung, Tie-Break-Key in `metrics_version`), deterministische Invalidierung bei Evidenzrücknahme. **Seit der Matrixnacharbeit 1 zwei Stücke mehr:** die Konstante `GATE_MINDEST_FENSTER` neben den vier bestehenden Gates in `broker/src/coordinator/vergleichbarkeit.rs` (R1, Startwert 8) und die **Ausschlussgründe** aus der geschlossenen Menge, die mit der Hypothese ausgegeben und im Store beim Finding erhalten werden (R4). Produzent für `event_type = "finding"` — die Projektion existiert bereits (§2.2). | M-14 bis M-28, M-87 | **NEU** `EqCopSonde014HypotheseTest`; bestehend **A4**, **A5**, **A8**; voller Kanon |
| **D — Befundzustände** | Abbildung hoch/mittel/unklar → `READY TO SEND` / `MORE DATA` / `STALE`, Sperre von Audition und Draft **im Datenweg**, Alternativen als eigene Befunde, die drei Zeilen als Datenfelder, Trennung von `CONFIDENCE`. | M-29 bis M-35 | **NEU** `EqCopSonde014BefundTest`; bestehend **B13** |
| **E — Maskierungs-Datenweg** | Rechnung aus der vorhandenen Bandevidenz im Coordinator, Bindung **an den Befund** (E-04), Alter höchstens ein Evidenzintervall plus Transportlatenz mit Kennzeichen `herabgesetzt` bei Rückstau (E-03), keine Zeichenanweisung. **Die Sperre aus Etappe 1 entfällt** — der Rückweg steht seit Etappe B. | M-36 bis M-41 | **NEU** `EqCopSonde014MaskierungTest`; bestehend **A6**, **A19**, **A23** |
| **F — Proposal-Policy** | Objekt mit fünfzehn Feldern plus `revert` aus der geschlossenen Menge (E-06), deterministische Erzeugung in fünf Schritten, `keine Änderung` und `mehr Daten` als gültige Vorschläge, Sicherheitsbudgets als Property-Test, `execution = manual` in ganz P5, Zielbereich und Schutzbereiche aus dem Befund und **im Experiment gelesen** statt geraten (E-05, beide Pfade mit `ziel_geraten`). Produzent für `event_type = "proposal"`. | M-42 bis M-54, M-63 | **NEU** `EqCopSonde014ProposalTest`; bestehend **A4**, **A5**; voller Kanon |
| **G — `AssistantStep`** | Zustandsmaschine im **Main** (E-08), `Preview` in der Menge und ohne P5-Kante, ein gespeicherter `Preview`-Schritt ist ein Lesefehler (E-07), fünf Angaben je Zustand, ein Slot, vier Gegenpfade, Persistenz in `MainProject`, Rekonstruktion nach Main-Neustart, Anschluss an den bestehenden Experimentkern. | M-55 bis M-62 | **NEU** `EqCopSonde014AssistentTest`; bestehend **B14**, **B25**, **B26**, **A4**, **A4-SI**; voller Kanon |
| **H — Evaluationskorpus** | **Seit der Matrixnacharbeit 1 eine Kette, kein Deklarationsregister (R2):** jede synthetische Session läuft als Evidenzbestand durch `hypothese_verdrahtung.rs`, und gelesen wird die **tatsächlich ausgegebene** Hypothese gegen die Wahrheit des Falls. Erzeuger und Prüfer nach dem Muster **A25**/**A26**/**A27**: Precision und Recall je **Ursachenklasse**, Kalibrierung, Brier, Coverage, Enthaltung; Sessions mit wahrem Kandidaten, korreliertem Distraktor, Parent-Duplikat, verschobener **und zu kurzer** Passage; Riegel `falsche_starke`, `falsche_schwache`, `precision ≤ 1`, `recall ≤ 1`; Selbsttestbein mit Gegenteil je Erwartung. Die Schwelle aus M-31 **und** der Startwert von `GATE_MINDEST_FENSTER` aus M-23 sind **Ausgabe** dieser Etappe. | M-64 bis M-70, M-31, Kalibrierung von M-23 | **NEU** Erzeuger-, Prüf- und Selbsttestbein (anzulegen) plus ein Rust-Integrationsbein im **A4**-Bereich, das die Sessions fährt; bestehend **A26** (druckt die Lücke, bis H sie misst); voller Kanon |
| **I — Ort, Nebenläufigkeit und Invarianten** | Rückstau-Politik je Klasse für die neuen Objekte (E-09: `draft_offer` P1 mit Schlüssel, `user_verdict` persistenzpflichtiger P0), Verbinden/Trennen, **Vollständigkeitsmarke des Intents vor jeder Rechnung** (E-10, M-86), Brokerneustart mit offenem Schritt, Audio-Thread-Freiheit, NaN-Ehrlichkeit, State-Verlustfreiheit, `findingsOffen` bekommt seine Quelle. | M-71 bis M-84, M-86 | bestehend **A1**, **A4**, **A4-SI**, **A10**, **A21**, **A22**, **B1**, **B2**, **B3c**, **B4**, **B10**, **B13**, **A14**; voller Kanon |

**Keine Etappe hängt mehr an einem offenen Entscheid.** Die Sperren aus
Etappe 1 (Rückweg des Maskierungswerts, Experimentfeld für den Zielbereich)
sind durch E-04 und E-05 aufgehoben; was bleibt, ist eine
**Reihenfolgebedingung**: Etappe B liegt vor E, F und I, weil diese drei ihre
Vertragsteile brauchen. §5.4 führt das im Einzelnen.

### 5.2 Ticketpfade (Prüfbereich der Etappe 2)

Alles außerhalb dieser Liste ist im Bauverlauf mit Begründung zu benennen.

**Broker:** `broker/src/coordinator/mod.rs`,
`broker/src/coordinator/evidenz.rs`,
`broker/src/coordinator/experiment.rs`,
`broker/src/coordinator/experiment_verdrahtung.rs`,
`broker/src/coordinator/invalidierung.rs`,
`broker/src/coordinator/invalidierung_verdrahtung.rs`,
`broker/src/coordinator/vergleichbarkeit.rs`,
`broker/src/coordinator/sicht.rs`,
`broker/src/coordinator/subscription.rs`,
`broker/src/dto.rs`,
`broker/src/protokoll.rs`,
`broker/src/vertrag.rs`,
`broker/src/store/writer.rs`,
sowie die neu **anzulegenden** Module `intent.rs`, `hypothese.rs`,
`proposal.rs`, `assistent.rs`, `maskierung.rs` und ihre Verdrahtungsdateien
unter `broker/src/coordinator/` (E-08). `protokoll.rs` und `vertrag.rs` sind
seit Etappe 1b dabei: die **Fassung 3** (E-04, E-05, E-09, E-10, seit der
Matrixnacharbeit 1 auch E-11) hebt den Wire-Envelope-`schema_minor`, und die
Rust-Seite dieses Vertrags liegt dort. `vergleichbarkeit.rs` kommt mit der
Matrixnacharbeit 1 dazu: `GATE_MINDEST_FENSTER` (R1) steht dort neben den vier
bestehenden Gates und nirgendwo sonst.

**Verträge:** `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`,
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`,
`eq-copilot/schemas/v3/README.md`,
`eq-copilot/schemas/v3/metriken-v1.json`,
`eq-copilot/schemas/state/nakama-state-v2.md`. Das v3-README kommt in
Etappe 1b dazu, weil die Fassung 3 dort als Fassungsschritt geführt wird
(E-04); `nakama-state-v2.md` trägt zusätzlich den Nebenbefund **N-02** nach.

**Plugin:** `eq-copilot/plugin/state/NakamaState.cpp`,
`eq-copilot/plugin/state/NakamaState.h`,
`eq-copilot/plugin/src/SourcesModel.cpp`,
`eq-copilot/plugin/src/SourcesModel.h`,
`eq-copilot/plugin/src/PluginProcessor.cpp`,
`eq-copilot/plugin/src/PluginProcessor.h`,
`eq-copilot/plugin/core/ipc/ControlClient.cpp`,
`eq-copilot/plugin/core/ipc/ControlClient.h`,
`eq-copilot/plugin/CMakeLists.txt`.

**Tests, Fixtures, Werkzeug:** `eq-copilot/plugin/tests/`,
`eq-copilot/fixtures/`, `broker/tests/`, `tools/eq-copilot/`,
`tools/beweise.ps1`.

**Ausdrücklich nicht im Prüfbereich:** die Audio- und Sondenpfade
(`eq-copilot/plugin/sonde/`, `eq-copilot/plugin/probe/`,
`eq-copilot/plugin/core/analysis/`), die Hostbrücke
(`eq-copilot/plugin/hostbridge/`), die Identität (`eq-copilot/identity/`), der
Installer (`eq-copilot/install/`) und `design/`.

### 5.3 Risiken

| Nr. | Risiko | Warum es real ist | Gegenmaßnahme im Bauplan |
|---|---|---|---|
| R1 | **Ein Test, der das Modul direkt ruft, kann nicht fallen.** | Genau das war der Kernbefund der Wiederprüfung 1 der Etappe 2 von SONDE-013: „die Runde 1 hat Funktionen gebaut und Tests geschrieben, die das Modul direkt aufrufen — der Produktpfad war an vielen Stellen nicht geschlossen." | Jede Etappe hat **zwei** Testebenen: Modul und Verdrahtung. Der Rotbeweis fällt am Verdrahtungstest, nicht am Modultest. Der Modulschnitt aus E-08 spiegelt das. |
| R2 | **MSVC-Standardstack 1 MiB.** | NAK-175: Testfunktionen, die `FeatureEngine`-Objekte (rund 0,5 MB) oder ganze Prozessoren im Rahmen anlegen, reißen ihn — in SONDE-013 dreimal, jedes Mal an einem neuen Feld unter 2 KiB; der Fund zeigt sich erst im **vollen** Kanon, das betroffene Bein allein läuft grün. | Jede Engine und jeder Prozessor in einem neuen Test liegt auf dem **Heap** (`std::unique_ptr`). Der volle Kanon läuft am Ende jeder Etappe, nicht nur das neue Bein. |
| R3 | **Zwei still konkurrierende Wahrheiten.** | §33.5 verbietet sie ausdrücklich, und dieses Ticket legt zwei Objekte in den Main-State und drei in den Store. | M-71 misst die Aufteilung. Der Main-State hält von Store-Objekten **nur IDs**; ein vollständiges Proposal im Main-State ist ein Rotbeweis. |
| R4 | **Ein Fassungsschritt, der in mehrere Etappen zerfällt.** | `session_snapshot` und `evidence_snapshot` sind `additionalProperties: false`; nach dem ersten Cross-Language-Fixture ist ein Feld nicht mehr frei änderbar. Fünf Teilschritte an demselben `schema_minor` ergäben fünf Zwischenstände, von denen jeder eine eigene Fassungsleiter bräuchte. | **Seit Etappe 1b entschärft, in der Matrixnacharbeit 1 erweitert:** die Fassung 3 (E-04, E-05, E-09, E-10 und seit R3 auch E-11) ist **eine** Bauetappe (§5.1, Etappe B) mit **einem** Fassungsschritt, einer Fassungsleiter und einem Satz Cross-Language-Fixtures. Kein Schema wird außerhalb dieser Etappe angefasst. |
| R5 | **Der Korpus misst sich selbst grün.** | NAK-182 fand genau das: `recall > 1` und `precision > 1` waren strukturell unerreichbar, die Wachen hatten keinen Weg zu fallen. | Das Selbsttestbein (M-68) speist synthetische Kennzahlen **direkt** in die Riegelfunktion, und jede Erwartung läuft mit ihrem Gegenteil. |
| R6 | **Ein Rotbeweis, der am Nebeneffekt fällt.** | Die Matrixprüfung von NAK-182 fand diesen Fehlertyp zweimal. | Die Spalte **Rotbeweis** in §3 nennt für jede Zeile, woran die Prüfung fallen muss. Ein Rotbeweis, der an einem Übersetzungsfehler, einer Rundungsphase oder einem Filter mit `t0` fällt, wird nicht angenommen. |
| R7 | **Die Guardrail-Lücken aus NAK-193/NAK-204 schlagen durch.** | Zwei von fünf Guardrails aus **SONDE-013 M-45** sind ohne Vertragsteil nicht messbar; der Breite-Guardrail ist strukturell tot. Ein Proposal, dessen `stop_if` darauf zeigt, wäre nicht überprüfbar. | M-50 verkettet die Lücke ehrlich: ein solches Proposal ist **nicht handelbar** und meldet `MORE DATA`, statt so zu tun, als wäre der Guardrail grün. Der Skalar-Entscheid selbst gehört NAK-193/NAK-204, nicht diesem Ticket. |

### 5.4 Reihenfolgebedingungen statt Sperren (nachgezogen in Etappe 1b und Matrixnacharbeit 1)

**Es gibt keinen offenen Vertragsentscheid mehr, an dem der Bau anhalten
müsste.** E-01 bis E-11 sind entschieden, R1 bis R4 sind eingearbeitet, und die
Fassung 3 ist als **ein** Fassungsschritt benannt. Was bleibt, ist keine Sperre,
sondern eine **Reihenfolge**: sieben Stellen brauchen einen Vertragsteil, den
Etappe B liefert.

| Stelle | Zeile | Braucht aus Etappe B | Was ohne Etappe B baubar bleibt |
|---|---|---|---|
| Rückweg des Maskierungswerts zu Gen | M-40 | `session_snapshot.findings` samt eingebettetem `maskierung` (E-04) | Rechnung, Alter, Befundbindung und die Zusage „keine Zeichenanweisung" (M-36 bis M-39, M-41) — alles brokerseitig |
| Zielbereich und Schutzbereiche im Experiment | M-48 | `experiment_begin.ziel` mit `proposal_id` (E-05) | Die Proposal-Hälfte (M-47): der Vorschlag benennt den Zielbereich aus dem Befund |
| Rückwegfeld des Proposals | M-43 | `revert` mit geschlossener Menge (E-06) — Wire- und Storeform | Die übrigen fünf Gate-Felder (Ziel, Passage, Grenzen, Hörziel, Stopbedingung) |
| Rückstau-Politik der neuen Objekte | M-73 | Belegung von `draft_offer` und `user_verdict` (E-09) | Die bestehende P0/P1-Politik selbst; sie wird nicht geändert |
| Transport des Intents | M-85, M-86 | Reservierung und Belegung von `intent_update` (E-10) | Die Main-Seite des Intents vollständig (Etappe A) |
| Transport und Spiegel des Assistentenschritts | M-88, M-89 | Reservierung und Belegung von `assistant_step_update` und der Ein-Writer-Spiegel (E-11 aus R3) | Die Zustandsmaschine im Main vollständig (Etappe G) |
| Ausschlussgründe am Befund | M-87 | Die geschlossene Menge und ihr Platz in `session_snapshot.findings` (R4) | Die Rechnung selbst: welcher Kandidat aus welchem Grund ausscheidet (Etappe C) |

**Die verbleibenden inhaltlichen Abhängigkeiten** sind keine Vertragsfragen,
sondern Messungen. Beide zeigen auf Etappe H:

- die Schwelle hoch/mittel/unklar → handelbar (**M-31**) ist **Ausgabe** des
  Evaluationskorpus und darf bis dahin nicht als Konstante gesetzt werden;
  Etappe D baut die drei Zustände und ihre Sperren, die Kalibrierung kommt aus
  H nach;
- der Startwert von `GATE_MINDEST_FENSTER` (**M-23**, R1) wird ebenso am Korpus
  festgezogen. Etappe C baut die Konstante mit dem begründeten Startwert 8;
  die Sessions „zu kurze Passage“ und „verschobene Passage“ aus
  Etappe H entscheiden, ob er stehen bleibt.

**Was ausdrücklich nicht dieses Ticket entscheidet und den Bau auch nicht
aufhält:** der Skalar-Entscheid zu `guardrail_loudness_db`, `guardrail_peak_db`
(NAK-204) und `stereo.seitenanteil_db` (NAK-193). M-50 verkettet die Lücke
ehrlich — ein Proposal, dessen Stopbedingung auf einen nicht messbaren
Guardrail zeigt, ist nicht handelbar und meldet `MORE DATA`.

### 5.5 Regeln für jede neue Prüfung dieser Etappe

- **Jede neue Prüfung wird einmal absichtlich gebrochen**, und die Rohausgabe
  des Rots liegt unter `docs/beweise/roh/` bei. Eine Probe, die auch ohne den
  Fix rot ist, heißt Regressionswache, nicht Beleg
  (`tools/dirigent/pruefliste.md` E).
- **Engines und Prozessoren in Tests liegen auf dem Heap** (`std::unique_ptr`),
  nie im Funktionsrahmen (NAK-175, R2).
- **Ein Riegel ist fail-closed ohne Rohtextheuristik:** Unbekanntes ist ROT
  (`tools/dirigent/pruefliste.md` D).
- **Behauptung ≤ Messung.** Die Behauptung eines neuen Beins in
  `tools/beweise.ps1` nennt das Gate-Maß in wenigen Sätzen; Wachen, Vorstufen
  und Ausnahmen gehören in den Skriptkopf, nicht in die Behauptung.
- **Writer-Fixtures statt Handschrift:** ein Probe-Datensatz „in der Form des
  Writers" wird vom Writer erzeugt und eingefroren; synthetische Fälle sind
  deklarierte Mutanten genau eines Writer-Fixtures mit benannter Abweichung.
- **Geänderte Zusage — drei Stellen:** wer eine Zusage ändert, zieht
  Runnerkopf (`tools/beweise.ps1`), Skriptkopf und den lebenden Manifestkopf
  nach; ein `git grep` nach dem Kernbegriff genügt als Nachweis.

---

## 6. Prüfliste `tools/dirigent/pruefliste.md`, abgehakt

Die Prüfliste ist für den Worker eine Abhakliste und für den Prüfer Kontext,
**keine Anforderungsquelle** (Dirigent §3.4, 30.08.2026). Diese Etappe schreibt
keinen Code; „gemessen" heißt hier deshalb: die Zeile ist in der Matrix als
Zusage verankert und ihr Ort im Ist-Stand ist benannt.

| Abschnitt | Zeile | Zutreffend | Wo in diesem Manifest gemessen |
|---|---|---|---|
| **A** | Politik bei *voll* je Klasse ausdrücklich | ja | **M-73**; Ist-Stand §2.9 (`ControlClient.h`:242,314,556,558–560); Entscheide **E-09** (§4.8) und **E-10** (§4.9) |
| **A** | Puffer mit Abflussweg **ohne** Reconnect | ja | **M-74**; Bein **A4-SI** |
| **A** | Schlüssel überleben jeden Zwischenpuffer; Koaleszierung gilt auch dort | ja | **M-73** (E-09: `proposal:<proposal_id>` für `draft_offer`; `user_verdict` koalesziert **nicht**) sowie **M-85** (E-10: `intent:<quelle_id>:<passage_id oder global>`) und **M-88** (E-11: `assistant_step:<session_id>`) — bei beiden ist Koaleszierung zulässig, weil nur die jüngste Revision wirkt |
| **A** | Niedrigere Klasse blockiert, höhere wird weiter gelesen | ja | **M-73**, **M-75**; bestehender P0/P1-Vertrag aus SONDE-010 |
| **A** | Rückgabewerte und Zähler einer Politik werden ausgewertet | ja | **M-73** (Rotbeweis: „ein Rückgabewert der Politik wird ignoriert"); Vorbild `evidenzKadenzReduktionen`/`evidenzNichtGesendet` (§2.9) |
| **A** | Dieselbe Regel in **allen** Sprachen, Test beidseitig, einmal gebrochen | ja | **M-77** |
| **B** | Reihenfolge beim Verbinden festgelegt und gemessen | ja | **M-75**; seit E-10 zusätzlich **M-86**: die Vollständigkeitsmarke liegt **vor** der ersten Hypothese und dem ersten Proposal — seit E-11 deckt sie **beide** Bestände, Intent **und** aktuellen Schritt (M-88, M-89) |
| **B** | Beim Trennen zuerst abmelden, dann Fristen und Joins | ja | **M-75** |
| **B** | Jeder Join hat eine Frist; kein Self-Join | ja | **M-75**; bestehender Vertrag aus SONDE-011/012, von dieser Etappe nicht geändert |
| **B** | Nach dem Schließen einer Queue wird nichts geliefert; Schließflag **vor** dem Inhalt | ja | **M-75** |
| **B** | Stopp-Fenster: Registrierung nach begonnenem Stop ist ein Test | ja | **M-75**, **M-76** (Brokerneustart mit offenem Schritt), **M-86** (nach dem Neustart rechnet der Broker erst nach der Vollständigkeitsmarke) |
| **C** | Textfeld mit Schema-Länge im Leser gegen genau diese Länge, Negativfixture je Feld | ja | **M-15**, **M-42** (Negativfixture je Feld über **A5**); Muster: `probe_descriptor.host_bus_name` 1–120 Codepoints |
| **C** | Objekte mit exakter Feldmenge, `additionalProperties:false` in C++ **und** Rust | ja | **M-15**, **M-42**, **M-77**; seit R4 zusätzlich **M-87**: die acht Ausschlussgründe sind eine **geschlossene Menge** an genau einer Stelle im Vertrag, beidseitig geprüft; Ist-Stand §2.2/§2.6 |
| **C** | Discriminator, Familie und Typ vor dem Inhalt geprüft, beidseitig | ja | **M-77**; §2.6 (`x-nakama-discriminator` nicht additiv) |
| **C** | NaN/Inf/Grenzwerte an jeder Zahl über den Draht | ja | **M-82** |
| **D** | Riegel fail-closed ohne Rohtextheuristik | ja | §5.5; **M-64** (Riegel des Korpusbeins) |
| **D** | Bein prüft die Frische seiner Eingaben, meldet Exit 3 statt grün | ja | §5.5; bestehende Runnerregel, von dieser Etappe nicht geändert |
| **D** | Relink ändert Bundlebytes; feste Hashes gelten für eine Auslieferung | **nein** | Diese Etappe und Etappe 2 ändern kein Bundle und keinen Installer; `eq-copilot/install/` steht ausdrücklich außerhalb des Prüfbereichs (§5.2). |
| **D** | Was der Kanon nicht baut, darf er nicht als frisch bezeugen | ja | §5.1: der volle Kanon läuft am Ende jeder Etappe, die Schema-, Fixture- oder Runnerdateien berührt |
| **E** | Behauptung ≤ Messung, Gate-Maß in wenigen Sätzen | ja | §5.5; die Spalte **Rotbeweis** in §3 hält jede Zusage an ihrer eigenen Messung |
| **E** | Zahlen im Manifest sind gemessen, nicht abgeschrieben | ja | §2 durchgehend: Kanon 54/54 und der leere `git diff --stat` in dieser Sitzung gemessen; Bandzahlen 221/64 aus dem Register; Korpuszahl 25 aus `MANIFEST.json`; Kadenzen aus `FeatureEngine.h`:989–990 und `WorkerCadence.h`:27–28; §3.11 aus den Matrixzeilen gezählt (89 Zeilen, escape-bewusster Parser); der Startwert 8 für `GATE_MINDEST_FENSTER` ist aus `eq-copilot/plugin/core/analysis/FeatureEngine.h`:989–990 und `broker/src/coordinator/experiment.rs`:529 abgeleitet, nicht geraten |
| **E** | Positionen im lebenden Kopf als Symbol/Anker | ja | Kopf: Symbol- und Ankerangaben; Zeilennummern nur im Verlauf ab §1, mit dem Stand `5e99a13` benannt |
| **E** | Lebender Kopf wird beim Abschluss nachgezogen, alles darunter append-only | ja | Kopfhinweis unter der Merkmalstabelle |
| **E** | Jede neue Prüfung einmal absichtlich gebrochen, Rohausgabe liegt bei | ja | §5.5; Spalte **Rotbeweis** in §3 |
| **E** | Geänderte Zusage — drei Stellen, kein Inventar | ja | §5.5 |
| **E** | Writer-Fixtures statt Handschrift | ja | §5.5; **M-65** (Korpusfälle tragen wörtliche Quelle, Muster **A25**) |
| **F** | speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg im selben Commit | ja (ohne den Installer-Teil) | §5.1: Etappe A trägt Writer, Reader, Migration, Goldens und Fixtures zusammen; Etappe **B** trägt Schema, Register, README, beide Leser, den Ein-Writer-Spiegel des Assistentenschritts (R3) und die Fassungsleiter der Fassung 3 in **einem** Änderungssatz; Etappe **G** trägt Abbruch, Zurück, Überspringen, Resume und Rekonstruktion zusammen. Installieren↔Rückweg trifft nicht zu (kein Installereingriff, siehe **D**). |
| **F** | Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines Vertrags im selben Änderungssatz | ja | §5.1, Etappen A, B, E; **M-77** |

---

## 7. Bauverlauf Etappe 2

**Append-only.** Jede Bauetappe trägt hier einen eigenen Abschnitt: was gebaut
wurde, welche Matrixzeilen mit `Datei::Bezeichner` gemessen sind, welche
Rotbeweisdateien dazugehören, jede Abweichung von §5 mit Begründung und jeder
Nebenbefund. Nebenbefunde gehen **nicht** ins Register — der Dirigent zieht sie
nach.

### 7.1 Etappe A — `SourceIntent` im Main-State

**Gebaut** (Commit `8f030f5`, Basis `f90abf5`): der musikalische Intent als
**additive Eigenschaft in einem bekannten Kind desselben Majors** — drei Listen
plus die Bestandsrevision in `MainProject`, kein neues Kind und keine
Root-Versionierung (M-81). Reine Main-Seite, ohne Wire, wie §5.1 es für diese
Etappe vorschreibt.

| Stück | Ort |
|---|---|
| Rollenmenge, Belegung, Konfliktordnung, Zyklenriegel | `eq-copilot/plugin/state/NakamaState.h`, `NakamaState.cpp` — `Rolle`, `Belegung`, `belegung()`, `rolleAus()`, `Konfliktstufe`, `vergleicheAnsprueche()`, `hatZyklus()` |
| Persistenz (Writer, Leser, Riegel) | `NakamaState.cpp` — `synchronisiert()` (Schreiber), `leseSchema2()` (Leser), `schutzbereichGueltig()` (der EINE Bandriegel für beide Seiten) |
| Produkt-API | `NakamaState.cpp` — `setzeIntent`, `entferneIntent`, `findeIntent`, `wirkenderIntent`, `setzeSchutzangabe`, `entferneSchutzangabe`, `setzeBeziehung`, `speichereAlsGleichrangigkeit`, `entferneBeziehung`, `entmaskierungErlaubt` |
| Verdrahtung (Produktpfad) | `eq-copilot/plugin/src/PluginProcessor.cpp` — `setzeQuellenrolle`, `entferneQuellenrolle`, `schuetzeQuelle`, `hebeQuellenschutzAuf`, `setzeQuellenbeziehung`, `speichereQuellenGleichrangigkeit`, `entferneQuellenbeziehung`, `sourceIntents`, `intentSchutzangaben`, `intentBeziehungen`, `intentBestandRevision`, `wirkendeQuellenrolle`, `entmaskierungErlaubt` |
| Vertragstext | `eq-copilot/schemas/state/nakama-state-v2.md` §2, §2.1.1 |
| Golden (vom Writer erzeugt, §5.5) | `eq-copilot/fixtures/state/schema2/main-intent-v1.bin`, geschrieben von `EqCopStateMigrationTest --schreibe-goldens`, registriert in `tools/eq-copilot/erzeuge_state_fixtures.py` (Bein **A12**) |

**Gemessene Matrixzeilen.** Zwei Ebenen je Zeile: das Modul aus
`NakamaState.cpp` **und** derselbe Handgriff über den echten
`EqCopilotProcessor` (Risiko R1). Der Rotbeweis fällt jeweils an der
Verdrahtung.

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-01** | `Sonde014IntentTest.cpp::m01` — geschlossene Menge, fünf Werte, sechster Wert fällt in API **und** Leser | `roh/SONDE-014-rot-M-01.txt` |
| **M-02** | `Sonde014IntentTest.cpp::m02` — Belegungstafel, Roundtrip über alle fünf, alle zwanzig geordneten Paare kollisionsfrei, fremde Belegung gehört zu keiner Rolle | `roh/SONDE-014-rot-M-02.txt` |
| **M-03** | `Sonde014IntentTest.cpp::m03` — Schutz ohne Rolle, Rolle `begleitet` mit Attack-Schutz, fünf Rollenwechsel ohne Verlust, Bandränder 0/221 von beiden Seiten | `roh/SONDE-014-rot-M-03.txt` |
| **M-04** | `Sonde014IntentTest.cpp::m04` — `nur_rolle`, `nur_beziehung`, `beides_im_widerspruch`, plus die Gegenprobe für ein drittes Paar und für `darf_verschmelzen` | `roh/SONDE-014-rot-M-04.txt` |
| **M-05** | `Sonde014IntentTest.cpp::m05` — alle zehn Stufenpaarungen in beiden Leserichtungen, hundert Vermutungen gegen die Schutzgrenze, spezifisch vor global auf Stufe 2, Stufe 3 im Produktpfad | `roh/SONDE-014-rot-M-05.txt` |
| **M-06** | `Sonde014IntentTest.cpp::m06` — reiner Riegel (Kette, Dreierzyklus, aufgelöste Runde, `darf_verschmelzen`), Prüfung beim Speichern, Leser weist gespeicherten Zyklus als read-only ab, `gleichrangig` überlebt Save/Load | `roh/SONDE-014-rot-M-06.txt` |
| **M-07** | `Sonde014IntentTest.cpp::m07` — abgeleitet ersetzt user nie, umgekehrt schon, Herkunft/Revision/Konfidenz überleben Save/Load | `roh/SONDE-014-rot-M-07.txt` |
| **M-08** | `Sonde014IntentTest.cpp::m08` — zwei Passagenrollen desselben Busses, dritter globaler Scope, Entfernen eines Scopes, Save/Load bytegleich | `roh/SONDE-014-rot-M-08.txt` |
| **M-09** | `Sonde014IntentTest.cpp::m09` und `Sonde012ProjectReloadTest.cpp::intent_survives_project_recall_and_load_marks_no_dirty` (Bein **B14**) | `roh/SONDE-014-rot-M-09.txt` |
| **M-10** | *nicht in dieser Etappe* — Etappe F (`EqCopSonde014ProposalTest`), weil es ohne Proposal keinen Zustand gibt, der `STALE` werden kann | — |
| **M-11** | `Sonde014IntentTest.cpp::m11` — zwei echte Prozessoren, derselbe phasenstetige 1-kHz-Strom, einer mit vollem Intent; Evidenzbänder, drei Perzentile, Livebänder, `metrics_version` und NaN-Zähler bitgleich, mit Gegenprobe (70 Bänder tragen einen Wert) | `roh/SONDE-014-rot-M-11.txt` |
| **M-12** | *nicht in dieser Etappe* — Etappe C (`EqCopSonde014HypotheseTest`), weil die Rangkomponente „Intent-Relevanz" erst dort entsteht | — |
| **M-13** | `Sonde012ProjectReloadTest.cpp::intent_change_marks_host_dirty_once_and_noop_or_rejected_stays_silent` (Bein **B14**, `DirtyZaehler.nonParam`) und `Sonde014IntentTest.cpp::stateInvariante` | `roh/SONDE-014-rot-M-13.txt` |

**Läufe.** `EqCopSonde014IntentTest` **210 Prüfungen, 0 Fehler** (neues
Kanon-Bein **B27**); `EqCopSonde012ProjectReloadTest` **12/12** (Bein **B14**,
zwei Fälle neu); `EqCopStateMigrationTest` **180 Prüfungen, 0 Fehler** (Bein
**B2**, vier Prüfungen neu am Intent-Golden); `erzeuge_state_fixtures.py
--pruefen` **36 Dateien bytegleich** (Bein **A12**).
**Voller Kanon GRÜN 55/55 auf `8f030f5`**, Rohausgabe
`docs/beweise/roh/SONDE-014-8f030f5.md`. Die Zahl steigt von 54 auf 55, weil
**B27** dazukommt; kein Bein ist verschwunden.

**Rotbeweise.** Elf Dateien `docs/beweise/roh/SONDE-014-rot-M-*.txt`. Jede
enthält die gewertete Zusage, die minimale Rücknahme am **Produktcode** (nicht
am Test), den roten Lauf mit Exitcode, die zurückgenommene Rücknahme und den
grünen Lauf. Der Treiber ist reproduzierbar und liegt außerhalb des Repos;
seine Rücknahmen stehen wörtlich in den Dateien.

**Abweichungen von §5, mit Begründung.**

1. **M-01 wird in Etappe A nur einseitig gemessen.** Die Matrix nennt für M-01
   zusätzlich „**A5** erweitert um das Enum". A5 ist der v3-Schemariegel, und
   §5.3 R4 sagt: „kein Schema wird außerhalb von Etappe B angefasst". Die
   C++-Hälfte (geschlossene Menge, sechster Wert fällt in API und Leser) liegt
   deshalb hier, die Vertragsstelle und der beidseitige Vergleich (M-77) in
   Etappe B. Beide Hälften stehen am Ende, keine fällt weg.
2. **M-10 und M-12 liegen nicht in Etappe A.** §5.1 ordnet sie der Etappe A zu
   („M-01 bis M-13"), ihre Zusagen brauchen aber Objekte, die es dort noch
   nicht gibt: M-10 verlangt ein Proposal, das `STALE` wird (Etappe F), M-12
   die Rangkomponente „Intent-Relevanz" (Etappe C). Beide Zeilen sind in der
   Tabelle oben ausdrücklich als offen ausgewiesen statt still übersprungen.

**Nebenbefunde.**

- **N-02 geschlossen.** Der Zustandsbaum in
  `eq-copilot/schemas/state/nakama-state-v2.md` §2 führte `manual_passages_v1`
  nicht, obwohl `NakamaState.cpp`:36 es seit SONDE-013 schreibt und liest. Die
  Zeile steht jetzt an ihrem Platz, mit datiertem Nachtrag. Der Befund liegt
  innerhalb der Ticketpfade (§5.2) und ist deshalb hier behoben, nicht nur
  benannt.
- **N-06 (neu, keine Änderung).** `EqCopSonde012ProjectReloadTest` legt seine
  `EqCopilotProcessor`-Objekte im Funktionsrahmen an, nicht auf dem Heap
  (NAK-175, Risiko R2). Der volle Kanon ist damit grün — der Prozessor dieses
  Beins trägt keine `FeatureEngine` im kritischen Pfad —, aber die Regel gilt
  für neue Beine, und dieses ist ein altes. Die zwei **neuen** Fälle dieses
  Tickets halten sich daran nicht schlechter als ihr Wirt; ein Umbau des ganzen
  Beins wäre ein Neben-Refactor außerhalb des Auftrags.

**Technische Entscheide dieser Etappe** (innerhalb von Matrix, Entscheiden und
Invarianten selbst getroffen):

- **Drei Listen statt einer.** `source_intents_v1`, `intent_protections_v1` und
  `intent_relations_v1` sind getrennte Eigenschaften, weil sie verschiedene
  Schlüssel haben (Quelle+Scope, Quelle+Eigenschaft+Band, geordnetes Paar) und
  E-01a sie ausdrücklich orthogonal führt. Eine gemeinsame Liste hätte für jede
  Zeile eine Diskriminatorspalte gebraucht — eine zweite Wahrheit über den Typ.
- **`intent_revision_v1` als eigene Eigenschaft.** M-86 verlangt eine
  Vollständigkeitsmarke **samt Revision** für den ganzen Bestand. Die
  Objektrevisionen aus M-07 können das nicht leisten: ein leerer Bestand hätte
  keine. Revision 0 reist deshalb gar nicht, und Inhalt ohne Revision ist ein
  ungültiger Stand.
- **`-1/-1` statt `0/0` für Schutzangaben ohne Bandintervall.** Band 0 ist ein
  gültiger Index im 221er-Gitter; `0/0` wäre von „leeres Intervall am ersten
  Band" nicht zu unterscheiden.
- **Feste Obergrenzen 256/256/256.** §48.1 verlangt feste Grenzen. 64
  bestätigte Quellen mal vier Scopes ist die Größe, die eine Sitzung mit dem
  heutigen Passagendeckel erreicht; der Headroomriegel in `NakamaState.cpp`
  fährt den vollen Bestand und beweist, dass er in die Bytegrenze passt.

---

### 7.2 Etappe B — Fassung 3 des Wire-Envelopes

**Gebaut** (Commits `edea7a9` Vertrag, `baa6291` Produktpfad, `581431a`
Rotbeweise): **ein** Fassungsschritt des Wire-Envelope-`schema_minor`, wie
§5.3 R4 ihn verlangt. Alle P1-Vertragsänderungen dieses Tickets liegen in
Fassung 3, mit **einer** Fassungsleiter und **einem** Satz
Cross-Language-Fixtures.

| Stück | Ort |
|---|---|
| Vertrag | `eq-copilot/schemas/v3/eq-ipc-v3.schema.json` — 76 → **113** Definitionen, 22 → **26** Familien. Neu: `intent_update`, `assistant_step_update` (E-10, E-11); belegt: `draft_offer`, `user_verdict` (E-09); neue Felder: `session_snapshot.findings` samt eingebettetem `maskierung` und `ausschluesse` (E-04, R4) und `experiment_begin.ziel` mit `proposal_id` (E-05) |
| Namensregister | `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` — 29 → **31** Namen (26 definiert + 5 reserviert), `vorher`/`aktuell` 2 → 3, Fassungseintrag `"3"` mit jeder Neuerung und der Rückbauliste |
| Consumerregeln | `eq-copilot/schemas/v3/README.md`, Abschnitt „Regeln, die dem Consumer gehören" — **zwölf** neue Zeilen für das, was die geschlossene Schlüsselwortmenge nicht ausdrücken kann |
| Fassungsleiter (Python) | `tools/eq-copilot/pruefe_v3_vertrag.py` — `fassung_2_schema()` neu; `fassung_1_schema()` setzt darauf auf (**Kette**, nicht auf der committeten Fassung) |
| Fassungsleiter (Rust) | `broker/src/coordinator/schema.rs` — `v3_schema_minor_2_wurzel()`, `v3_schema(2)`, `v3_schema(3)`, `JSON_SCHEMA_MINOR_AKTIV = 3` |
| Transportfassung | `broker/src/transport/server_v3/mod.rs` (`P0_SCHEMA_MINOR`/`P1_SCHEMA_MINOR` 3) und `eq-copilot/plugin/core/ipc/WireEnvelope.h` (`kJsonSchemaMinor` 3) |
| Spiegel und Vertragsvalidierung | **NEU** `broker/src/coordinator/intent.rs` und `broker/src/coordinator/assistent.rs`, verdrahtet in `senke.rs`, Zustand in `zustand.rs`, Aufräumen in `link.rs`/`liveness.rs` |
| Store-Spiegel | `broker/src/store/handle.rs` — `assistent_schritte_lesen`, der versionierte Spiegel im append-only `event_log`, **ohne** Store-Migration 2 |
| Senderseite | `eq-copilot/plugin/src/PluginProcessor.cpp` — `v3IntentUpdateJson`, `v3AssistantStepJson`, `sendeIntentVollbestand`, `sendeIntentFortschreibung`, `setzeAssistentSchritt` |
| Fixtures | `eq-copilot/fixtures/v3/` — 317 → **385** (87 → 104 gültig, 230 → 281 ungültig) |

**Gemessene Matrixzeilen.**

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-15** | `pruefe_v3_vertrag.py::pruefe_sonde014_fassung_3` (`causehypothesis_traegt_die_zehn_felder_aus_36_3`); Fixtures `session-snapshot-mit-findings`, `finding-belegtext-statt-zone`, `finding-vierte-anzeigezeile`, `finding-ohne-evidenz-ids` | `roh/SONDE-014-rot-M-15.txt` |
| **M-40** | `schema.rs::minor_2_leser_lehnt_die_fassung_3_ab` und `fassung_1_und_0_erben_den_rueckbau_der_fassung_3`; `pruefe_v3_vertrag.py::fassung_2_leser_lehnt_session_snapshot_findings_ab`; C++-Hälfte in `IpcTestMain.cpp::fassung_3_familien_und_felder_werden_von_der_cpp_engine_angenommen` | `roh/SONDE-014-rot-M-40.txt` |
| **M-42** | `pruefe_v3_vertrag.py::proposal_traegt_die_fuenfzehn_felder_aus_42_1_plus_revert`; elf Negativfixtures am Proposal | `roh/SONDE-014-rot-M-42-erzeuger.txt` |
| **M-48** | `pruefe_v3_vertrag.py::fassung_2_leser_lehnt_experiment_begin_ziel_ab`; Fixtures `experiment-begin-mit-ziel`, `experiment-ziel-fremdes-feld`, `experiment-ziel-band-ueber-dem-rand`. **Die Leseseite des Guardrail-Rechners gehört Etappe F** — hier steht nur der Vertragsteil | `roh/SONDE-014-rot-M-48-rechner.txt` |
| **M-73** (Vertragshälfte) | Register (`draft_offer`/`user_verdict` belegt, Eigentümer SONDE-014); `IpcTestMain.cpp::draft_offer_reist_als_p1_mit_proposal_schluessel` und `user_verdict_ist_ein_persistenzpflichtiger_p0_befehl_und_koalesziert_nicht` | `roh/SONDE-014-rot-M-73.txt` |
| **M-77** | `intent.rs::geschlossene_mengen_des_lesers_decken_sich_mit_dem_vertrag` (Rust) **und** `IpcTestMain.cpp::sechster_rollenwert_faellt_an_der_cpp_engine`, `vierter_rueckweg_dsp_revert_faellt_an_der_cpp_engine`, `vierter_befundzustand_faellt_an_der_cpp_engine` (C++) | `roh/SONDE-014-rot-M-77.txt` |
| **M-85** | `sonde014_verdrahtung.rs::intent_update_koalesziert_je_quelle_und_scope` und `teilmeldung_mit_mehreren_objekten_wird_abgewiesen`; Senderhälfte `PluginProcessor.cpp::sendeIntentFortschreibung` | `roh/SONDE-014-rot-M-85.txt` |
| **M-86** | `sonde014_verdrahtung.rs::keine_rechnung_vor_der_vollstaendigkeitsmarke` und `sitzungsende_loescht_den_spiegel`; Senderhälfte `PluginProcessor.cpp::sendeIntentVollbestand` | `roh/SONDE-014-rot-M-86.txt` |
| **M-87** (Vertragshälfte) | `pruefe_v3_vertrag.py::ausschlussgruende_sind_eine_geschlossene_achtermenge`; `schema.rs::geschlossene_mengen_der_fassung_3_stehen_je_einmal_im_vertrag`; Fixtures `finding-ausschlussgrund-unbekannt`, `finding-ausschluss-fremdes-feld`. **Die Rechnung gehört Etappe C** | `roh/SONDE-014-rot-M-87.txt` |
| **M-88** | `sonde014_verdrahtung.rs::assistant_step_update_koalesziert_je_session` | `roh/SONDE-014-rot-M-88.txt` |
| **M-89** | `sonde014_verdrahtung.rs::spiegel_weicht_dem_neueren_main_schritt` — beide Hälften: der Schritt liegt wirklich im Log, **und** der frische Broker trägt keinen aus der Datenbank | `roh/SONDE-014-rot-M-89.txt` |

**Läufe.** `pruefe_v3_vertrag.py --abdeckung` **595 Prüfungen, 0 gescheitert**,
keine Definition ohne Negativfixture, `enum` 50/50 und `const` 16/16 gedeckt
(Bein **A5**); `erzeuge_v3_fixtures.py --pruefen` **389 Dateien bytegleich**
(Bein **A8**); `cargo test` vollständig grün, darunter **NEU**
`sonde014_verdrahtung` mit 11 Fällen (Bein **A4**); `EqCopSchemaTest` **134**
(Bein **B3c**); `EqCopIpcTest` **370** (Bein **B10**, acht Prüfungen neu);
`EqCopSonde014IntentTest` **210** (Bein **B27**).

**Abweichungen von §5, mit Begründung.**

1. **Zwei Dateien außerhalb der Ticketpfade.**
   `broker/src/transport/server_v3/mod.rs` und
   `eq-copilot/plugin/core/ipc/WireEnvelope.h` stehen nicht in §5.2. Sie
   tragen die Transportfassung, und der bestehende Riegel
   `transportfassung_und_json_leser_stimmen_ueberein` misst genau, dass sie
   dieselbe Zahl führt wie der JSON-Leser. Ein Fassungsschritt, der nur den
   Leser hebt, wiese jeden Rahmen ab, den der Coordinator lesen könnte — das
   ist der Fehler, den SONDE-013 am 04.09.2026 gemessen hat (vier
   `eqcop-store-crash-worker` warteten 17 Minuten auf ein `command_ack`, das
   nie kam). Die Änderung ist zwei Zahlen groß.
2. **Keine Store-Migration 2 für den `AssistantStep`-Spiegel.** §2.2 sagt
   ausdrücklich, dass dieses Ticket keine braucht, und Migration 1 legt
   `event_log` als **append-only** Tabelle mit `event_ord`, `sequence`,
   `schema_major`/`schema_minor` und Index je Sitzung an. Das **ist** ein
   versionierter Spiegel für Suche und Crashdiagnose (§33.5). Eine
   Domänentabelle `assistant_steps` hätte die Prüfsumme von
   `MIGRATION_1_SQL` berührt und einen zweiten, autoritativ **aussehenden**
   Ort geschaffen — der Spiegel ist aber nie autoritativ, und ein Ort, der so
   aussieht, ist die teuerste Sorte Fehler.
3. **M-73 und M-87 liegen nur zur Hälfte in dieser Etappe.** §5.1 ordnet
   ihnen die „Vertragshälfte" beziehungsweise „die Ausschlussmenge auf dem
   Draht" zu; die Rückstau-Politik der neuen Objekte gehört Etappe I, die
   Rechnung der Ausschlüsse Etappe C. Beide Hälften sind in der Tabelle oben
   ausdrücklich benannt statt still zusammengezogen.

**Nebenbefunde.**

- **N-05 geschlossen.** Der Fließtext in `zweck` und `pruefung` des Registers
  rechnete auf 28 Namen und hatte `experiment_candidate` nicht mitgezählt,
  während `gesamt_erwartet` bereits 29 sagte. Die maschinenlesbare Zahl war
  richtig, die Prosa nicht. Beide tragen jetzt 31 mit datiertem Nachtrag; das
  v3-README nennt die Zahl gar nicht mehr, sondern verweist auf die
  maschinenlesbare Stelle.
- **N-07 (neu, behoben).** `erzeuge_v3_fixtures.py::wende_an` kopierte die
  **Grundform** tief, den **eingesetzten Baustein** aber nicht. Zwanzig
  Fassung-3-Fixtures trugen dadurch die Felder ihrer Nachbarn; gemessen an
  `contract_cross_language.rs`. Der Fehler ist latent seit dem ersten
  Fixture, das einen gemeinsamen Baustein einsetzt — die Fassung 3 ist nur
  die erste, die genug davon hat, um ihn auszulösen. Behoben an der Stelle,
  an der er entsteht: `wende_an` kopiert jetzt auch den Wert.
- **N-08 (neu, behoben).** Die Rollenmenge des Rust-Lesers
  (`intent::ROLLEN`) war eine **zweite handgepflegte Kopie** der
  geschlossenen Menge aus dem Vertrag, ohne Riegel dagegen. Ein Leser mit
  einer sechsten Rolle nahm jedes gültige Fixture an und fiel an keinem
  einzigen — „die Menge lebt an genau einer Stelle im Vertrag" war damit eine
  Behauptung. Der Rotbeweis M-77 hat das aufgedeckt; der neue Test
  `geschlossene_mengen_des_lesers_decken_sich_mit_dem_vertrag` schließt es.
  Dieselbe Klasse wie T2-Runde 2 / BF-5.

**Technische Entscheide dieser Etappe.**

- **`band_hz` trägt ein Bandintervall, keine Hertzzahlen.** §36.3 nennt das
  Feld so, und der Name bleibt (M-15 zählt ihn wörtlich auf). Getragen wird
  ein Intervall des eingefrorenen 221-Band-Gitters; die Hertzwerte folgen aus
  dessen `kanten_hz`. Zwei Achsen — eine in Hz, eine in Bandindizes — wären
  genau die zweite Frequenzachse, die M-36 ausschließt.
- **`befund_konfidenz` ist strikt, `konfidenz` bleibt additiv.** Das eine ist
  die Sicherheit eines Befunds, das andere die Messqualität einer Passage.
  Abnahme U21 schließt aus, sie zu vermischen; die verschiedene
  Additivitätspolitik macht die Trennung im Vertrag sichtbar.
- **`intent_update` trägt beide Formen.** Vollbericht und Einzelfortschreibung
  liegen in **einer** Familie, unterschieden durch `vollstaendig`. Zwei
  Familien hätten zwei Discriminator-Zweige und zwei Fassungsleitern
  gebraucht, für denselben Gegenstand. Dass eine Teilmeldung genau ein Objekt
  trägt, ist eine Consumerregel mit Präzedenz im Vertrag
  (`audible_intervention_begin.experiment_id` bei `art=experiment`).
- **Der Vollbestand hat einen eigenen Koaleszierungsschlüssel.**
  `intent:vollstaendig` neben `intent:<quelle>:<scope>`: ein Vollbericht darf
  einen älteren Vollbericht verdrängen, aber nie eine Einzelfortschreibung —
  die trägt ein anderes Objekt.


### 7.3 Etappe C — Evidenzgraph und `CauseHypothesis`

**Gebaut:** die Kandidatenbildung, der Rang aus sechs Komponenten mit hartem
Coverage-/Alignment-Gate, die `CauseHypothesis` mit den zehn Feldern aus §36.3
und den sechs Teilen aus §8, der Determinismus (Saat, Reduktionsreihenfolge,
Quantisierung, Tie-Break-Key), die deterministische Invalidierung bei
Evidenzrücknahme, die geschlossene Achtermenge der Ausschlussgründe und die
Konstante `GATE_MINDEST_FENSTER` aus R1. Der Produzent für
`event_type = "finding"` existiert damit; die Projektion lag seit SONDE-011
ohne ihn da (§2.11 L3).

| Stück | Ort |
|---|---|
| Rechnung (rein, ohne Lock und Store) | **NEU** `broker/src/coordinator/hypothese.rs` — `hypothesen()`, `gate()`, `rangkomponenten()`, `masteranomalie()`, `finding_id()`, die sieben Mengen `URSACHENKLASSEN`/`AUSSCHLUSSGRUENDE`/`AUSSAGEKLASSEN`/`ZIELMETRIKEN`/`NAECHSTE_TESTS`/`BEFUNDZUSTAENDE`/`SICHERHEITSKLASSEN` |
| Produktpfad (Auslöser, Aufnahme, Ablage, Zustellung) | **NEU** `broker/src/coordinator/hypothese_verdrahtung.rs` — `hypothesen_bilden()`, `aufnahmen_sammeln()`, `befunde_eintragen()`, `befund_persistieren()`, `befund_json()`, `befunde_invalidieren_locked()` |
| Das Gate aus R1 | `broker/src/coordinator/vergleichbarkeit.rs` — `GATE_MINDEST_FENSTER = 8`, neben den vier bestehenden Gates und nirgendwo sonst |
| Auslöser 1 (neue Evidenz) | `broker/src/coordinator/evidenz.rs` — `hypothesen_bilden()` direkt nach `evidenz_paare_bilden()` |
| Auslöser 2 (Rücknahme) | `broker/src/coordinator/invalidierung_verdrahtung.rs` — `befunde_invalidieren_locked()` **unter demselben Lock** wie die Evidenzmarkierung, `befunde_nach_ruecknahme_zustellen()` danach |
| Rückweg zu Gen | `broker/src/coordinator/sicht.rs` — `findings` im `session_snapshot`, gedeckelt auf `SNAPSHOT_BEFUNDE_MAX = 64` |
| Flüchtiger Bestand | `broker/src/coordinator/zustand.rs` — `Stand::befunde`, `Stand::befunde_neu_bilden` |
| Bandbezogener Schutz | `broker/src/coordinator/intent.rs` — `IntentBestand::schutz_verletzt()` (additiv; `geschuetzt()` bleibt unverändert) |
| Passagenzugriff | `broker/src/coordinator/experiment.rs` — `Experimentstore::juengste_passage_im_projekt()` |
| Bein (Verdrahtung) | **NEU** `broker/tests/sonde014_hypothese.rs`, 17 Fälle |
| Bein (Modul) | `broker/src/coordinator/hypothese.rs` `#[cfg(test)] mod tests`, 13 Fälle |

**Gemessene Matrixzeilen.** Zwei Ebenen je Zeile (§5.3 R1): das reine Modul
**und** derselbe Handgriff über die Senke `p1`. Der Rotbeweis fällt jeweils an
der Verdrahtung.

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-14** | `sonde014_hypothese.rs::ursachenklassen_sind_geschlossen_und_sieben` (Vertragsmenge, Rückweg jedes Worts, achte Klasse fällt, siebte ist Enthaltung) und `hypothese.rs::geschlossene_mengen_sind_rund` | `roh/SONDE-014-rot-M-14.txt` |
| **M-15** | `sonde014_hypothese.rs::causehypothesis_traegt_die_zehn_felder` — jedes Pflichtfeld aus `$defs/session_finding.required` am **erzeugten** Objekt, `class` und `score` als zwei Felder. Die Vertragshälfte liegt in Etappe B (§7.2) | `roh/SONDE-014-rot-M-15-erzeuger.txt` |
| **M-16** | `sonde014_hypothese.rs::sechs_teile_im_datenmodell` und `hypothese.rs::sechs_teile_sind_sechs_und_benannt` — die sechs Teile aus §8 **und** die drei Anzeigezeilen als drei eigene Felder; kein Belegtextfeld | `roh/SONDE-014-rot-M-16.txt` |
| **M-17** | `sonde014_hypothese.rs::parallele_telemetrie_bleibt_klasse_eins` — zwei gleichzeitig gemessene Quellen bleiben `zusammenhang`, und `ursachenbeleg` ist im ganzen Pfad unerreichbar | `roh/SONDE-014-rot-M-17.txt` |
| **M-18** | `sonde014_hypothese.rs::screening_reicht_hoechstens_fuenf_weiter` — sieben gleichwertige Kandidaten, **fünf** kommen durch; die Zahl steht als Literal, nicht als `KANDIDATEN_DECKEL` | `roh/SONDE-014-rot-M-18.txt` |
| **M-19** | `sonde014_hypothese.rs::bedingter_uplift_braucht_fenster_ohne_die_quelle` — eine konstant laute Quelle liefert **keinen** Uplift, dieselbe Bühne mit stillen Fenstern schon | `roh/SONDE-014-rot-M-19.txt` |
| **M-20** | `sonde014_hypothese.rs::keine_komponente_kompensiert_coverage_oder_alignment` (je ein Fall mit Maximalwert bei gerissener Coverage beziehungsweise gerissenem Alignment) und `hypothese.rs::gate_faellt_in_der_zugesagten_reihenfolge` (Kante 0,5 von beiden Seiten) | `roh/SONDE-014-rot-M-20.txt` |
| **M-21** | *nicht in dieser Etappe* — der korrelierte Distraktor ist ein **Korpusfall** (Etappe H, `pruefe_p5_korpus.py`). Die Rangseite steht: ein zweiter Kandidat wird eigener Befund mit eigenem Zustand und erscheint als `alternatives` am führenden | — |
| **M-22** | `sonde014_hypothese.rs::parent_duplikat_erzeugt_keine_zwei_starken` — zwei Sonden auf demselben `host_mixer_index` erreichen beide nicht `hoch`; ohne `host_mixer_index` ist die Routingqualität 0 | `roh/SONDE-014-rot-M-22.txt` |
| **M-23** | `sonde014_hypothese.rs::passage_zu_kurz_traegt_keine_starke_aussage` — drei Fenster tragen keine starke Aussage, zwölf schon, die Kante fällt bei 7/8, und eine um 30 % verschobene Passage erreicht `hoch` nicht; `hypothese.rs::passagengates_trennen_verschoben_anders_und_zu_kurz` trennt die drei Gründe | `roh/SONDE-014-rot-M-23.txt` |
| **M-24** | `sonde014_hypothese.rs::ruecknahme_invalidiert_abhaengige_hypothesen` — die Rücknahme über den Produktweg „Material gewechselt", und derselbe Umfang über einen anderen Auslöser trifft dieselbe Menge | `roh/SONDE-014-rot-M-24.txt` |
| **M-25** | `sonde014_hypothese.rs::ranking_ist_bytegleich_ueber_hundert_laeufe` — hundert Bühnen aus **denselben Bytes**, bytegleiche serialisierte `findings`; `hypothese.rs::finding_id_ist_deterministisch_und_trennt` und `quantisierung_klemmt_rastet_und_ist_nan_ehrlich` | `roh/SONDE-014-rot-M-25.txt` |
| **M-26** | `sonde014_hypothese.rs::gleichstand_zeigt_beide` — beide bleiben, der Rang ist wirklich gleich, und der Tie-Break ist die `candidate_source` aufsteigend | `roh/SONDE-014-rot-M-26.txt` |
| **M-27** | `sonde014_hypothese.rs::mehr_daten_ist_ein_ergebnis` — ohne Kandidat entsteht ein **Befund** mit `daten_reichen_nicht`, `more_data`, `mehr_daten_sammeln` und eigenen Evidenz-IDs | `roh/SONDE-014-rot-M-27.txt` |
| **M-28** | `sonde014_hypothese.rs::behauptung_ohne_existente_evidenz_wird_nicht_sichtbar` (Teilverlust → `stale`) und `ruecknahme_invalidiert_abhaengige_hypothesen` (Vollverlust → unsichtbar, auch im Snapshot) | `roh/SONDE-014-rot-M-28.txt` |
| **M-87** | `sonde014_hypothese.rs::ausschlussgruende_sind_geschlossen_und_reisen_mit` — die Achtermenge gegen den Vertrag, ein neunter Grund fällt, und der ausgeschiedene Kandidat steht **mit Grund** im Snapshot. Die Vertragshälfte liegt in Etappe B (§7.2) | `roh/SONDE-014-rot-M-87-rechnung.txt` |
| **M-12** | `sonde014_hypothese.rs::rollenaenderung_bewegt_die_intent_relevanz` — dieselbe Quelle als `fuehrt` trägt eine höhere Rangkomponente als als `begleitet`. §7.1 hatte die Zeile ausdrücklich hierher vertagt | `roh/SONDE-014-rot-M-12.txt` |
| **M-86** (zweite Hälfte) | `sonde014_hypothese.rs::keine_hypothese_vor_der_vollstaendigkeitsmarke` und `hypothese.rs::ohne_vollstaendigkeitsmarke_rechnet_das_modul_nicht` — die Sperre aus E-10 gilt auch für die Ursachenrechnung, doppelt verriegelt | — (Rotbeweis in §7.2) |

**Läufe.** `cargo test --manifest-path broker/Cargo.toml` **581 Prüfungen,
0 Fehler**, darunter **NEU** `sonde014_hypothese` mit 17 Fällen und
`coordinator::hypothese::tests` mit 13 (Bein **A4**). `cargo clippy
--all-targets` meldet in beiden neuen Dateien nichts.

**Rotbeweise.** Sechzehn Dateien
`docs/beweise/roh/SONDE-014-rot-M-{12,14,15-erzeuger,16,17,18,19,20,22,23,24,25,26,27,28,87-rechnung}.txt`.
Jede enthält die gewertete Zusage, die minimale Rücknahme am **Produktcode**
(nie am Test) wörtlich, den roten Lauf mit Exitcode 101, die zurückgenommene
Rücknahme und den grünen Lauf mit Exitcode 0.

**Abweichungen von §5, mit Begründung.**

1. **Kein neues Kanon-Bein.** §5.1 nennt für diese Etappe „**NEU**
   `EqCopSonde014HypotheseTest`" — einen C++-Beinnamen. Entscheid **E-08**
   (§4.8, Etappe 1b und damit später als die Matrixzeile) weist Hypothese,
   Proposal und Maskierung ausdrücklich dem **Broker-Coordinator** zu: „dort
   liegen Evidenzbestand und Store". Ein C++-Bein hätte hier nichts zu messen.
   Gebaut ist deshalb ein Rust-Integrationsbein
   (`broker/tests/sonde014_hypothese.rs`), das unter **A4** läuft; die
   Behauptung von **A4** in `tools/beweise.ps1` ist entsprechend nachgezogen
   (geänderte Zusage — Runnerkopf, Skriptkopf und Manifestkopf gemeinsam).
   Ein eigenes Bein `A4-H` nach dem Muster von **A4-SI** wäre möglich gewesen
   und ist bewusst unterblieben: es liefe dieselben 37 Sekunden ein zweites
   Mal in jedem Kanonlauf, ohne eine Zusage zu messen, die **A4** nicht schon
   misst. Die Kanonzahl bleibt damit **55**.
2. **Der Tie-Break-Key steht noch nicht in `metriken-v1.json`.** M-25 nennt
   dafür „**A5** erweitert um den Tie-Break-Key". Entscheid **E-12** des
   Dirigenten (06.09.2026) erlaubt für dieses Ticket **genau einen**
   Versionsschritt der `metrics_version`, und der liegt in **Etappe H** —
   zusammen mit der Kalibrierung von `GATE_MINDEST_FENSTER` und der Schwelle
   aus M-31. Die vier Determinismusgrößen (`BOOTSTRAP_SAAT`, `RANG_QUANTUM`,
   `TIE_BREAK_KEY`, `RANGKOMPONENTEN` als Reduktionsreihenfolge) stehen
   deshalb als benannte Konstanten in `hypothese.rs` und wandern in H
   gemeinsam ins Register. Zwei Schritte für eine Sache wären zwei
   Kalibrierungen; **A5** duldet eine Codekonstante ohne Registereintrag
   ausdrücklich („Was er NICHT prüft: dass jede Codekonstante im Register
   steht").
3. **`M-21` liegt nicht in dieser Etappe.** §5.1 ordnet sie der Etappe C zu
   („M-14 bis M-28"), ihre Zusage ist aber ein **Korpusfall**: „Ein
   korrelierter, nicht kausaler Distraktor liegt im Korpus". Der Korpus
   entsteht in Etappe H; die Zeile ist oben ausdrücklich als offen
   ausgewiesen statt still übersprungen.
4. **Zwei Dateien außerhalb der neu anzulegenden Module.**
   `broker/src/coordinator/intent.rs` (`schutz_verletzt`) und
   `broker/src/coordinator/experiment.rs` (`juengste_passage_im_projekt`)
   stehen beide **in** den Ticketpfaden (§5.2). Beide Änderungen sind rein
   additiv: kein bestehender Bezeichner wurde umbenannt, keine bestehende
   Funktion geändert. Die Alternative — die Bandprüfung im Hypothesenmodul
   nachzubauen — wäre eine zweite Wahrheit über den Intent gewesen.

**Technische Entscheide dieser Etappe** (innerhalb von Matrix, Entscheiden und
Invarianten selbst getroffen).

- **Ausschluss ist nicht Rangschnitt.** Die geschlossene Menge aus M-87 kennt
  keinen Wert „Rang zu niedrig", und das ist kein Versehen: die acht Gründe
  sind **Gates**. Ein Kandidat, der ein Gate reißt, verlässt das Ranking mit
  Grund; der harte Deckel aus M-18 schneidet **danach** und trifft nur
  Kandidaten, die jedes Gate bestanden haben. Sie sind nicht ausgeschlossen,
  sondern überboten, und die deterministische Rangfolge ist ihr Protokoll.
  Ein neunter Grund „überboten" hätte die Menge geöffnet, ohne eine Zusage zu
  tragen.
- **Der Master ist der `main`, nicht der lauteste.** §8 erklärt einen
  *Master*befund. Gen ist als einzige Instanz `plugin_kind = "main"` und misst
  laut Vertrag ausschließlich am Insert. Eine Sitzung ohne Main rechnet gar
  nicht — das ist der normale Zustand einer Sondenrunde ohne Gen und kein
  Fehler.
- **„Routing bekannt" ist `host_mixer_index`, und „Parent-Duplikat" ist
  derselbe Kanal.** Der Deskriptor trägt **keine** Elternkante; der Broker
  kennt FL's Routinggraphen nicht. M-22 verlangt zwei Dinge, und beide sind
  mit dem messbar, was da ist: ohne `host_mixer_index` gibt es keine starke
  Aussage, und zwei Quellen auf demselben Kanal messen dasselbe Signal und
  erreichen beide nicht `hoch`. Eine erfundene Elternkante hätte wie Wissen
  ausgesehen, das der Broker nicht hat.
- **Die Sicherheit hängt nicht an der Aussageklasse.** `claim_class` sagt, wie
  stark die **Art** des Belegs ist; `confidence` sagt, wie sicher **diese**
  Aussage ist. Wer `hoch` an ein PRE/POST-Paar knüpfte, machte es in einer
  rein passiven Sitzung strukturell unerreichbar — und genau dort verlangt
  M-23 seinen Rotbeweis („dieselbe Session mit einer Passage **unter**
  `GATE_MINDEST_FENSTER` liefert `READY TO SEND`" ist der **gebrochene**
  Zustand, also muss dieselbe Session mit genug Fenstern ihn erreichen).
- **Gleichgewichtete Rangkomponenten.** §36.2 nennt sechs Komponenten und
  **keine** Gewichte. Ein erfundenes Gewicht wäre eine unkalibrierte Zahl im
  Rechenweg — genau das, was `metriken-v1.json` verhindern soll. Der Rang ist
  deshalb das quantisierte Mittel; die Kalibrierung gehört Etappe H.
- **Der Uplift wird auf die eigene Spanne normiert.** Eine feste dB-Schwelle
  wäre wieder eine unkalibrierte Konstante. Eine Erhöhung um 3 dB heißt wenig,
  wenn der Master dort ohnehin um 20 dB schwankt, und viel, wenn er still
  steht — die Bezugsgröße steckt damit in den Daten, nicht im Code.
- **Terminal heißt: keine sofortige Neurechnung.** Nach einer Rücknahme wird
  **nicht** neu gerechnet. Eine Neurechnung unmittelbar danach hätte die
  Hypothese aus den verbliebenen Belegen frisch aufgebaut und den Zustand
  `stale` in derselben Bewegung wieder weggeräumt — die Invalidierung wäre ein
  Flackern gewesen, kein Zustand. Neu gerechnet wird erst, wenn **neues**
  Material ankommt; dann ruht die Aussage auch wirklich auf neuem Material.
  Die Gegenrichtung ist symmetrisch verdrahtet: scheitert der Store-Append,
  nimmt `invalidierung_ruecknehmen` die Markierung zurück **und** setzt
  `befunde_neu_bilden`, damit die Sperre nicht ohne Anlass stehen bleibt.
- **Der Rückweg ist der bestehende Snapshotpfad.** `findings` reist wie
  `experimente` und `paare` im `session_snapshot` (E-04). Eine eigene Familie
  hätte eine zweite Reihenfolge und eine zweite Koaleszierung gebraucht.

**Nebenbefunde.**

- **N-09 (neu, behoben, im eigenen Diff).** Der erste Entwurf von
  `screening_reicht_hoechstens_fuenf_weiter` prüfte gegen `KANDIDATEN_DECKEL`
  — dieselbe Konstante, die der Produktpfad benutzt. Der Rotbeweis M-18
  (Deckel auf 6) lief damit **grün**: der Test wanderte mit. Der Fall trägt
  jetzt die **Zahl** aus M-18 („höchstens die besten fünf") und prüft die
  Konstante daneben. Dieselbe Klasse wie ein Test, der das Modul direkt ruft:
  er kann nicht fallen.
- **N-10 (neu, keine Änderung).** `EqCopSonde012ProjectReloadTest` legt seine
  Prozessoren weiter im Funktionsrahmen an (N-06 aus §7.1, NAK-175). Diese
  Etappe fügt dort nichts hinzu; der Befund bleibt offen und unverändert.


### 7.4 Etappe D — Befundzustände

**Gebaut:** die Abbildung Sicherheit → Zustand als **eine** Funktion im
Datenweg, die Sperre von Audition und Draft an **beiden** Enden, die
`STALE`-Quelle aus §37.3, und die Gen-Hälfte des Rückwegs — ein Befundmodell
in `SourcesModel` samt Leser für `session_snapshot.findings`.

**Der Defekt, den diese Etappe zuerst geschlossen hat.** Etappe C baute den
Produzenten; der Leser in `SourcesModel.cpp` kannte `findings` nicht. Die
Wurzelfeldmenge für `schema_minor ≥ 2` endete bei `paare`, und
`exakteFelder` ist fail-closed: **jeder** Snapshot mit einem Befund war
`ungueltig`. Der Rückweg wäre gebaut und trotzdem tot gewesen — genau der
Fehler, den SONDE-013 an `experimente` und `paare` schon einmal gemacht hat.
Kein Bein hätte ihn gefunden: **B13** baut seine Snapshots von Hand und trägt
keine Befunde. Er ist als **N-11** mit eigenem Rotbeweis geschlossen.

| Stück | Ort |
|---|---|
| Abbildung Sicherheit → Zustand | `broker/src/coordinator/hypothese.rs` — `zustand_aus_sicherheit()`, **eine** Stelle; `baue_befund` und `enthaltung` rufen sie |
| Sperre im Datenweg (Broker) | `hypothese.rs` — `Befundzustand::erlaubt_audition()`, `erlaubt_draft()`, `aus_wire()` |
| Abbildung `next_test` → `SMALLEST TEST` | `hypothese.rs` — `NaechsterTest::satz()`, `ALLE`, `aus_wire()`; total über alle sechs Werte |
| `STALE`-Quelle (§37.3) | `broker/src/coordinator/intent.rs` — eine gestiegene Bestandsrevision ruft unter demselben Lock `befunde_veralten_locked()`; `broker/src/coordinator/hypothese_verdrahtung.rs` — die Funktion selbst, **ohne** Nachrechnen |
| Befundmodell (Gen) | `eq-copilot/plugin/src/SourcesModel.h` — `struct Befund` mit `darfAudition()` und `darfDraft()`, `Sicht::befunde` |
| Leser (Gen) | `eq-copilot/plugin/src/SourcesModel.cpp` — `findings` in der Wurzelfeldmenge der **Fassung 3**, sechs geschlossene Mengen, Bandintervall, Beobachtung, Konfidenz, Rangkomponenten, Evidenz-IDs, Alternativen, Ausschlüsse, drei Anzeigezeilen; dazu der Helfer `endlicheZahl()` |
| Bein (Gen) | **NEU** `eq-copilot/plugin/tests/Sonde014BefundTest.cpp`, Kanonbein **B28**, 45 Prüfungen |
| Bein (Broker) | **NEU** `broker/tests/sonde014_befund.rs`, 7 Fälle, läuft unter **A4** |

**Gemessene Matrixzeilen.**

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-29** | `sonde014_befund.rs::sicherheit_wird_auf_drei_zustaende_abgebildet` (die Abbildung ist total, und `veraltet` schlägt **jede** Stufe) und `Sonde014BefundTest.cpp` (die drei Zustände kommen als Feld an, ein vierter macht den ganzen Snapshot ungültig) | `roh/SONDE-014-rot-M-29.txt` (Gen) und `roh/SONDE-014-rot-M-29-broker.txt` |
| **M-29** (`STALE`-Quelle) | `sonde014_befund.rs::hoehere_intent_revision_macht_den_befund_stale` — der Zustand wechselt, und Rang, Beobachtung, Evidenz-IDs und `finding_id` stehen **unverändert** (§37.3: „ohne dass Zahlen nachgerechnet werden") | `roh/SONDE-014-rot-M-29-stale.txt` |
| **M-30** | `sonde014_befund.rs::nur_ready_to_send_erlaubt_audition_und_draft` (beide Aktionen, beide Sperrzustände, mit Gegenprobe am offenen Fall) und `Sonde014BefundTest.cpp` (dieselbe Frage am Modell, das die Aktion anbietet) | `roh/SONDE-014-rot-M-30.txt` |
| **M-31** | *nicht in dieser Etappe* — die Schwelle hoch/mittel/unklar ist **Ausgabe** des Korpus (Etappe H) und steht bewusst nirgends als Konstante. Etappe D baut die drei Zustände und ihre Sperren; die Kalibrierung kommt aus H nach (§5.4) | — |
| **M-32** | `sonde014_befund.rs::alternative_ist_ein_eigener_befund_mit_eigenem_zustand` (die ID zeigt auf einen Befund, der **wirklich** in der Liste steht) und `Sonde014BefundTest.cpp` (ein Freitext in `alternatives` fällt) | `roh/SONDE-014-rot-M-32.txt` |
| **M-33** | `sonde014_befund.rs::beleg_ist_zone_und_kein_text` (Zone plus Quelle, **kein** Belegtext- und **kein** Optikfeld — das ist zugleich M-80) und `Sonde014BefundTest.cpp` (Bandrand 0 und 221 gültig, leeres Intervall und 222 nicht) | `roh/SONDE-014-rot-M-33.txt` |
| **M-34** | `sonde014_befund.rs::drei_zeilen_sind_datenfelder` (die Abbildung `next_test` → `SMALLEST TEST` ist total; genau **eine** bewusste Doppelung) und `Sonde014BefundTest.cpp` (eine vierte oder eine leere Zeile fällt) | `roh/SONDE-014-rot-M-34.txt` |
| **M-35** | `sonde014_befund.rs::messqualitaet_und_befundsicherheit_sind_zwei_felder` — der Beleg meldet `konfidenz.klasse = mittel`, und derselbe Befund erreicht `hoch`, weil seine Sicherheit aus den **Gates** kommt; im Snapshot reist die Messqualität nicht im Befund mit. Dazu `Sonde014BefundTest.cpp`: ein `more_data`-Befund mit `confidence.class = hoch` bleibt **nicht** handelbar | `roh/SONDE-014-rot-M-35.txt` |
| **N-11** | `Sonde014BefundTest.cpp` — die Fassungsleiter: der Leser der Fassung 3 nimmt `findings` an, ein Leser der Fassung 2 lehnt es ab, Abwesenheit heißt „keine Befunde" | `roh/SONDE-014-rot-N-11.txt` |

**Läufe.** `EqCopSonde014BefundTest` **45 Prüfungen, 0 Fehler** (neues
Kanon-Bein **B28**); `EqCopSonde012SourcesModelTest` **76/76** (Bein **B13**,
unverändert grün nach der Erweiterung des Lesers); `cargo test` **588
Prüfungen, 0 Fehler** (Bein **A4**, darunter **NEU** `sonde014_befund` mit 7
Fällen). Die Kanonzahl steigt von **55** auf **56**, weil **B28** dazukommt;
kein Bein ist verschwunden.

**Rotbeweise.** Neun Dateien
`docs/beweise/roh/SONDE-014-rot-{M-29,M-29-broker,M-29-stale,M-30,M-32,M-33,M-34,M-35,N-11}.txt`.
Sechs davon fallen an der **C++**-Hälfte: die Rücknahme wird gesetzt, das Ziel
neu gebaut und das Binary gefahren — der rote Lauf ist der Lauf des echten
Beins, nicht eine Simulation.

**Abweichungen von §5, mit Begründung.**

1. **Die Etappe hat zwei Beine statt einem.** §5.1 nennt „**NEU**
   `EqCopSonde014BefundTest`; bestehend **B13**". Das C++-Bein ist gebaut und
   trägt die Zusagen, die am Modell hängen. Die Abbildung selbst rechnet aber
   der Broker (E-08), und eine Funktion, die nur in C++ gemessen würde, wäre
   auf der Seite, die sie ausführt, ungeprüft. Deshalb liegt daneben ein
   Rust-Bein unter **A4**. Beide messen dieselbe Zusage von ihrer Seite —
   dasselbe Muster, das M-77 für jede beidseitige Regel verlangt.
2. **M-31 liegt nicht in dieser Etappe.** §5.1 ordnet sie der Etappe D zu
   („M-29 bis M-35"), §5.4 verlangt aber ausdrücklich, dass die Schwelle
   „bis dahin nicht als Konstante gesetzt werden" darf. Sie ist oben als
   offen ausgewiesen statt still übersprungen.

**Technische Entscheide dieser Etappe.**

- **`veraltet` ist ein eigener Eingang, kein Ergebnis der Klasse.** Ein Befund
  wird `stale`, weil sich **unter** ihm etwas geändert hat — nicht weil seine
  Sicherheit gesunken wäre. Beides in einen Wert zu falten hieße, dem User
  eine gesunkene Sicherheit zu melden, wo er selbst gerade seine Absicht
  geändert hat.
- **Die Veraltung rechnet nicht nach.** §37.3 sagt es wörtlich. Die
  Rangkomponenten, die Beobachtung und die Evidenz-IDs bleiben stehen; nur der
  Zustand wechselt. Das ist kein Sparbetrieb: der User soll sehen, dass
  **seine** Änderung den Befund entwertet hat, und nicht eine stillschweigend
  neue Zahl.
- **`NaechsterTest::satz()` gehört an das Enum.** Ein `match` im Erzeuger wäre
  eine zweite Stelle mit derselben Abbildung, und die Anzeige müsste raten,
  welche gilt. Zwei der sechs Werte teilen sich bewusst denselben Satz —
  `passage_messen` und `mehr_daten_sammeln` heißen beide „länger messen"; das
  Bein misst, dass es bei **genau einer** Doppelung bleibt.
- **`endlicheZahl` prüft die Endlichkeit vor den Grenzen.** `juce::var` trägt
  Zahlen als `double`; bei `NaN` sind `d < min` und `d > max` beide falsch,
  und ohne `std::isfinite` hinge das Urteil davon ab, in welche Richtung
  verglichen wird. NaN-Ehrlichkeit heißt hier: fail-closed, unabhängig von der
  Vergleichsrichtung.
- **Der Sitzungswechsel räumt die Befunde ab.** Dieselbe Regel wie für
  `experimente` und `paare`: eine Aussage der alten Sitzung über die neue wäre
  eine Falschaussage, und ihre Evidenz-IDs zeigten ins Leere. Beim bloßen
  Trennen bleiben sie stehen — wie die Versuche auch; der Zustand ist
  „getrennt", nicht „nie gemessen".

**Nebenbefunde.**

- **N-11 (neu, behoben).** Der Leser kannte `findings` nicht — siehe oben. Er
  ist mit dieser Etappe geschlossen und trägt einen eigenen Rotbeweis.
- **N-12 (neu, behoben, im eigenen Diff).** Der erste Entwurf des C++-Beins
  griff in einem Fall ungeprüft auf `sicht.befunde[0]` zu. Bei einem
  abgewiesenen Snapshot ist die Liste leer, und das Bein **stürzte ab**, statt
  einen Fehler zu melden — ein Bein, das abstürzt, misst nichts. Jeder
  indizierte Zugriff steht jetzt hinter seiner Größenwache, und die Ausgabe
  läuft ungepuffert, damit ein Absturz zeigt, wo er stand.


### 7.5 Etappe E — Maskierungs-Datenweg

**Gebaut:** der laufende, je Frequenzbereich auflösbare Maskierungswert aus
Antwort U16 — als optionales Objekt **am Befund** (E-04), mit Frequenzbereich,
Wert, Gültigkeitsbit und dem Kennzeichen `herabgesetzt`. Die farbige Zone
selbst bleibt Anzeige und gehört S31b; dieses Ticket liefert ausschließlich
den Datenweg.

| Stück | Ort |
|---|---|
| Rechnung (rein) | **NEU** `broker/src/coordinator/maskierung.rs` — `maskierung()`, `bandpegel()`, `kadenz_reduziert()`, `struct Maskierung` |
| Am Befund | `broker/src/coordinator/hypothese.rs` — `CauseHypothesis::maskierung: Option<Maskierung>`, gefüllt in `baue_befund` aus **denselben** zwei Quellen und **demselben** Bandbereich wie der Befund |
| Auf der Leitung | `broker/src/coordinator/hypothese_verdrahtung.rs` — `befund_json` schreibt `maskierung` genau dann, wenn der Befund einen trägt |
| Leser (Gen) | `eq-copilot/plugin/src/SourcesModel.cpp` — der Wert wird jetzt **geprüft**; `SourcesModel.h` — `maskierungVorhanden`, `maskierungQuelleA/B`, `maskierungBandVon/Bis`, `maskierungWertDb`, `maskierungGueltig`, `maskierungHerabgesetzt` |
| Bein (Broker) | **NEU** `broker/tests/sonde014_maskierung.rs`, 5 Fälle, läuft unter **A4** |
| Bein (Gen) | **B28** um 15 Prüfungen erweitert (45 → **60**) |

**Gemessene Matrixzeilen.**

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-36** | `sonde014_maskierung.rs::maskierung_haengt_am_finding_und_benennt_einen_frequenzbereich` — der Wert steht **im** Befund, sein Bereich ist **derselbe** wie `band_hz`, und der Snapshot trägt **keine** zweite Liste daneben; `Sonde014BefundTest.cpp` — er kommt an, benennt beide Quellen, und Abwesenheit heißt „dieser Befund trägt keinen" | `roh/SONDE-014-rot-M-36.txt` |
| **M-37** (Alter) | `sonde014_maskierung.rs::maskierungswert_ist_hoechstens_ein_evidenzintervall_alt` — ein einziger neuer Beleg der Sonde bewegt den Wert, und er trifft die Differenz der beiden **jüngsten** Bandpegel (nachgerechnet: 8,55 dB), nicht ein Mittel über die Historie | `roh/SONDE-014-rot-M-37.txt` |
| **M-37** (`herabgesetzt`) | `sonde014_maskierung.rs::rueckstau_setzt_das_kennzeichen_herabgesetzt` — eine Lücke setzt das Kennzeichen, der Wert bleibt dabei eine **gültige Messung** und wird nicht interpoliert; `maskierung.rs::herabgesetzt_misst_die_luecke_und_nicht_den_seek` trennt Lücke von Epochengrenze | `roh/SONDE-014-rot-M-37-herabgesetzt.txt` |
| **M-38 / M-39 / M-80** | `sonde014_maskierung.rs::datenweg_traegt_keine_zeichenanweisung` — der **ganze** Snapshottext enthält keines von neun Optikwörtern, und der Wert hat genau die sieben Vertragsfelder; `Sonde014BefundTest.cpp` — `zoom`, `achse`, `farbe` fallen am Vertrag; `maskierung.rs::der_wert_traegt_keine_zeichenanweisung` misst dasselbe am Typ | `roh/SONDE-014-rot-M-38.txt` |
| **M-40** | `sonde014_maskierung.rs` (derselbe Fall wie M-36, aber die Rücknahme sitzt im **Serialisierer**: der Wert entsteht und erreicht die Leitung trotzdem nicht) und `Sonde014BefundTest.cpp` (Fassungsleiter: ein Leser der Fassung 2 lehnt `findings` samt Maskierung ab). Die **Vertragshälfte** liegt in Etappe B (§7.2) | `roh/SONDE-014-rot-M-40.txt` |
| **M-41** | `sonde014_maskierung.rs::zone_hat_keine_eigene_schwelle` — gleiche Pegel ergeben rund 0 dB, und **auch dieser** Wert reist; kein Feld trägt eine Schwelle oder Sichtbarkeit; und eine Enthaltung trägt gar keinen Wert (keine Maskierung ohne Befund) | `roh/SONDE-014-rot-M-41.txt` |

**Läufe.** `cargo test` **599 Prüfungen, 0 Fehler** (Bein **A4**, darunter
**NEU** `sonde014_maskierung` mit 5 Fällen und `coordinator::maskierung::tests`
mit 6); `EqCopSonde014BefundTest` **60 Prüfungen, 0 Fehler** (Bein **B28**);
`cargo clippy --all-targets` meldet in den neuen Dateien nichts. Die Kanonzahl
bleibt **56** — diese Etappe legt kein neues Bein an, sondern erweitert zwei.

**Rotbeweise.** Sechs Dateien
`docs/beweise/roh/SONDE-014-rot-{M-36,M-37,M-37-herabgesetzt,M-38,M-40,M-41}.txt`.

**Abweichungen von §5, mit Begründung.**

1. **Kein Bein `EqCopSonde014MaskierungTest`.** §5.1 nennt es; die Rechnung
   liegt aber nach **E-08** im Broker-Coordinator („`hypothese.rs`,
   `proposal.rs` und `maskierung.rs` rechnen im Coordinator"). Gebaut ist
   deshalb ein Rust-Bein unter **A4** für die Rechnung und den Rückweg, und
   die Gen-Hälfte — Leser, Trennung „nicht vorhanden" gegen „nicht gemessen" —
   liegt in **B28**, das ohnehin die Befunde liest. Ein drittes Bein hätte
   dieselben Snapshots ein drittes Mal geparst. Dieselbe Begründung wie in
   §7.3 Abweichung 1.
2. **Die genannten Beine A6, A19 und A23 sind nicht erweitert.** §5.1 führt
   sie als „bestehend". **A6** prüft die Bandgitter auf Bytegleichheit — der
   Maskierungswert benutzt das Gitter, ändert es aber nicht; **A19** und
   **A23** messen Latenz und Sichtzeiten und werden von einem optionalen
   Snapshotfeld nicht berührt. Sie laufen unverändert grün; eine Erweiterung
   ohne neue Zusage wäre eine Behauptung über eine Messung, die es nicht gibt.

**Technische Entscheide dieser Etappe.**

- **Der Wert ist eine Differenz zweier Bandpegel, kein psychoakustisches
  Modell.** Ein Modell brächte Konstanten mit, die niemand kalibriert hat, und
  §42.4 verlangt, dass jeder angezeigte Zahlenwert auf **ein Feld und eine
  Evidenz** zurückführbar bleibt. Zwei Bandpegel sind das; eine Modellausgabe
  wäre es nicht.
- **Der Bandpegel ist eine Leistungssumme, kein dB-Mittel.** dB zu mitteln ist
  ein geometrisches Mittel und unterschätzt genau die Spitze, um die es bei
  einer Maskierung geht: zwei Bänder mit −20 und −40 dB ergeben gemittelt
  −30 dB, summiert aber −19,96 dB — und gehört wird die Summe. Ein eigener
  Fall misst das.
- **`herabgesetzt` misst die Lücke, nicht einen Zähler.** Die Kadenzreduktion
  passiert beim Sender; der Heartbeat trägt `queue_overflows`, aber der
  Coordinator hält ihn heute nicht, und ihn dafür einzuführen wäre eine zweite
  Wahrheit über dieselbe Sache. Was der Broker **selbst** sieht, ist die
  Folge: die Fenster liegen nicht mehr aneinander. Eine **Epochengrenze** ist
  ausdrücklich keine Reduktion — sonst meldete jeder Seek eine Herabsetzung.
- **Der Wert wird aus dem jüngsten Fenster gerechnet, nicht aus der
  Historie.** M-37 verlangt einen **laufenden** Wert, und ein Mittelwert über
  acht Sekunden läuft nicht. Der Fall misst das, indem er einen einzelnen
  neuen Beleg schickt und den Sprung nachrechnet.
- **Ohne Fenster gibt es kein Objekt, ohne Messung eine 0 mit
  `gueltig = false`.** Das sind zwei verschiedene Aussagen — „dieser Befund
  trägt keinen Wert" und „hier ist nichts gemessen" —, und die Anzeige braucht
  beide getrennt. Der Rückgabetyp `Option<Maskierung>` trägt die erste, das
  Bit die zweite.

**Nebenbefunde.**

- **N-13 (neu, behoben).** Etappe D hat `maskierung` in die erlaubte Feldmenge
  des Befunds aufgenommen, damit ein Fassung-3-Snapshot nicht schon am
  Feldnamen scheitert — den **Inhalt** prüfte niemand. Das war der einzige
  fail-open-Zweig dieses Lesers: ein Objekt beliebiger Form wäre
  durchgegangen, und die Anzeige hätte daraus eine Zone gebaut. Der Leser
  prüft ihn jetzt vollständig; sechs vertragswidrige Formen fallen einzeln.


### 7.6 Etappe F — Proposal-Policy

**Gebaut:** das `Proposal` als validiertes, versioniertes Objekt mit fünfzehn
Feldern plus `revert`, die deterministische Erzeugung in fünf Schritten, die
Sicherheitsbudgets aus §42.3, der gelesene statt geratene Zielbereich (E-05)
und die Sperre, die einen nicht handelbaren Vorschlag gar nicht erst zum
Angebot macht.

| Stück | Ort |
|---|---|
| Rechnung (rein) | **NEU** `broker/src/coordinator/proposal.rs` — `proposal()`, `aktionstemplates()`, `constraint_solver()`, `kosten()`, `eingriff()`, `darf_draft_offer()`, `bandmitte_hz()`, die fünf geschlossenen Mengen und die fünf Budgetkonstanten |
| Produktpfad | **NEU** `broker/src/coordinator/proposal_verdrahtung.rs` — `vorschlaege_bilden()` direkt hinter `befunde_eintragen`, `vorschlag_persistieren()` als `event_type = "proposal"`, `draft_offer_zustellen()` über den Pushpfad, `proposal_json()` |
| Zielbereich im Experiment (E-05) | `broker/src/coordinator/experiment.rs` — `struct Experimentziel`, `Experiment::ziel`, `Achsenrechnung::ziel_geraten`, `Resultatmessung::ziel_geraten`, `beginne(..., ziel)`; `experiment_verdrahtung.rs` — `ziel_aus_wert()` und der gelesene Zielbereich im Guardrail-Rechner |
| Flüchtiger Bestand | `broker/src/coordinator/zustand.rs` — `Stand::vorschlaege`, `Stand::draft_offers` |
| Bein | **NEU** `broker/tests/sonde014_proposal.rs`, 15 Fälle; ein Fall in `broker/tests/sonde013_verdrahtung.rs` für M-48 |

**Gemessene Matrixzeilen.**

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-42** | `sonde014_proposal.rs::proposal_traegt_die_fuenfzehn_felder` — jedes Pflichtfeld aus `$defs/proposal.required` am **erzeugten** Objekt, und **kein** Feld außerhalb des Vertrags | `roh/SONDE-014-rot-M-42.txt` |
| **M-43** | `…::sechs_gate_felder_sind_pflicht_und_revert_hat_drei_werte` — die sechs Gate-Felder einzeln, die geschlossene Dreiermenge gegen den Vertrag, `dsp_revert` fällt, und die Abbildung Aktion → Rückweg ist total | `roh/SONDE-014-rot-M-43.txt` |
| **M-44** | `…::derselbe_eingang_erzeugt_denselben_entwurf` — hundert Bühnen, bytegleiche serialisierte Vorschläge | `roh/SONDE-014-rot-M-44.txt` |
| **M-45** | `…::in_p5_ist_jede_aktion_manual` — im Modell **und** auf der Leitung | `roh/SONDE-014-rot-M-45.txt` |
| **M-46** | `…::keine_aenderung_und_mehr_daten_sind_vorschlaege` — vollständiges Objekt mit Hörziel, Stopbedingung, Rückweg und Evidenz-IDs; **keine** Parameter, denn ein Nichteingriff hat keine | `roh/SONDE-014-rot-M-46.txt` |
| **M-47** | `…::zielbereich_kommt_aus_dem_befund_nicht_aus_dem_delta` — mit **Gegenprobe**, dass mit Capability wirklich ein Eingriff entsteht; die Frequenz ist die Bandmitte des Befundbands, und die Grenzen liegen in diesem Band | `roh/SONDE-014-rot-M-47.txt` |
| **M-48** | `sonde013_verdrahtung.rs::sonde014_m48_gelesenes_ziel_schlaegt_die_heuristik` — derselbe Versuch zweimal, einmal mit `ziel` (Band 5) und einmal ohne. Der Fall ist so gebaut, dass die Pfade **auseinanderlaufen müssen**: die größte Bewegung liegt auf Band 100. Mit `ziel` sind es 12 dB außerhalb, ohne 6 dB — und `ziel_geraten` sagt beides Mal die Wahrheit | `roh/SONDE-014-rot-M-48.txt` |
| **M-49** | `…::geschuetzte_eigenschaft_ist_harte_constraint` — mit **Gegenprobe**: ohne Schutz entsteht ein Eingriff, mit Schutz keiner, und der Schutz steht als Constraint im Vorschlag | `roh/SONDE-014-rot-M-49.txt` |
| **M-50** | `…::stop_if_auf_nicht_messbarem_guardrail_ist_nicht_handelbar` — die Aktion ist `more_data`, der Guardrail bleibt trotzdem in `stop_if`, und es geht kein Angebot hinaus | `roh/SONDE-014-rot-M-50.txt` |
| **M-51** | `…::jede_zahl_hat_feld_evidenz_und_generatorversion` — jede Zahl im Wire-Objekt steht in einem benannten Feld; Generatorversion und Evidenz-IDs sind belegt | `roh/SONDE-014-rot-M-51.txt` |
| **M-52** | `…::veraltet_ungueltig_capability_erreichen_keine_probe` — drei Gründe einzeln (Capability, `stale`, `base_revision`), jeder gegen dieselbe **Gegenprobe**, in der ein Angebot entstünde | `roh/SONDE-014-rot-M-52.txt` |
| **M-53** | `…::hard_caps_und_engeres_userbudget_werden_nie_ueberschritten` — 500 zufällige Lagen, deterministischer Würfel; die engere Usergrenze gewinnt immer, und ein Zähler beweist, dass die Prüfung wirklich Eingriffe gesehen hat | `roh/SONDE-014-rot-M-53.txt` |
| **M-54** | `…::evidenz_ids_und_intent_revision_sind_pflicht` — die Revision ist die, **gegen die** der Solver gelaufen ist, und die Evidenz-IDs kommen aus dem Befund | `roh/SONDE-014-rot-M-54.txt` |
| **M-63** | `…::manueller_busvorschlag_ist_ein_proposal` — dieselbe Struktur, Rückweg `manual_only`, und kein Fremdwerkzeugfeld | `roh/SONDE-014-rot-M-63.txt` |
| **M-10** | `…::rollenaenderung_macht_den_vorschlag_stale` — §37.3; §7.1 hatte die Zeile ausdrücklich hierher vertagt | `roh/SONDE-014-rot-M-10.txt` |
| **Gitterregel** | `…::bandmitte_folgt_dem_eingefrorenen_gitter` — alle 221 Mitten der Regel gegen die **committete** Datei, plus beide Ränder | `roh/SONDE-014-rot-Gitter.txt` |

**Läufe.** `cargo test` **615 Prüfungen, 0 Fehler** (Bein **A4**, darunter
**NEU** `sonde014_proposal` mit 15 Fällen und ein neuer Fall in
`sonde013_verdrahtung`); `cargo clippy --all-targets` meldet in den neuen
Dateien nichts. Die Kanonzahl bleibt **56**.

**Rotbeweise.** Sechzehn Dateien
`docs/beweise/roh/SONDE-014-rot-{M-10,M-42-erzeuger,M-43..M-47,M-48-rechner,M-49..M-54,M-63,Gitter}.txt`. **M-42** und **M-48** tragen ihren Zusatz, weil Etappe B unter dem blanken Namen bereits die Vertragshälfte belegt — einen fremden Beweis zu überschreiben wäre Verlust.

**Abweichungen von §5, mit Begründung.**

1. **Kein Bein `EqCopSonde014ProposalTest`.** Wie in §7.3 und §7.5: E-08 legt
   die Rechnung in den Broker-Coordinator. Gebaut ist ein Rust-Bein unter
   **A4**; die C++-Hälfte der geschlossenen Mengen (Rückweg, `execution`)
   liegt seit Etappe B in **B10** (`IpcTestMain.cpp`).
2. **`ziel_geraten` reist nicht auf der Leitung.** M-48 verlangt das
   Kennzeichen am **Resultat**; `session_experiment` ist
   `additionalProperties: false` mit sieben Feldern, und ein achtes wäre ein
   zweiter Fassungsschritt (§5.3 R4). Das Kennzeichen steht deshalb in
   `Resultatmessung` und `Achsenrechnung` — dort, wo es entsteht und wo der
   Guardrail-Wert danebensteht, auf den es sich bezieht.
3. **`M-49` misst nur die Vorschlagsseite.** §3.5 führt die Zeile selbst als
   „**BELEGT** (Vorschlagsseite) · **OFFEN für P7**": das DSP-ACK gehört
   S26–28/S29–31 und wird hier nicht vorgegriffen.
4. **Der Zielbereich überlebt einen Brokerneustart nicht.** Der Store trägt
   `experiment_begin.ziel` heute nicht; ein restaurierter Versuch rechnet
   wieder mit der Heuristik — und **sagt es** (`ziel_geraten`). Das ist eine
   ehrliche Lücke und keine stille: ein restauriertes Resultat behauptet
   nirgends, sein Ziel gelesen zu haben. Eine Store-Migration dafür wäre ein
   Eingriff in `MIGRATION_1_SQL` und damit dieselbe Klasse Entscheidung wie
   die, die §7.2 Abweichung 2 abgelehnt hat.

**Technische Entscheide dieser Etappe.**

- **Ein Eingriff schlägt eine Anleitung — immer.** „Der kleinste sichere
  Kandidat gewinnt" meint den kleinsten **wirksamen**. `manual_guidance`
  kostet nichts, weil es nichts tut, und würde eine reine Kostenordnung jedes
  Mal gewinnen; der Vorschlag wäre dann immer „mach es selbst" — ein Advisor,
  der nie etwas vorschlägt. Die Anleitung ist der **Rückfall**, wenn kein
  Eingriff sicher ist. Gefunden wurde das an der Gegenprobe: die erste Fassung
  der Fälle M-47, M-49, M-52 und M-53 lief grün, **weil** sie nie einen
  Eingriff erzeugte.
- **Die Frequenz kommt aus der Gitterregel, nicht aus einer Tabelle.** Das
  Gitter trägt seine Erzeugungsregel selbst (`herkunft.regel`, IEC 61260-1);
  eine 221 Werte lange Tabelle im Broker wäre eine **Kopie** des
  eingefrorenen Gitters. Ein eigener Fall hält die Regel gegen die committete
  Datei — damit ist die Kopie vermieden und die Übereinstimmung gemessen.
- **Die Budgets sind Vertragswerte, keine Kalibrierung.** Die Zahlen aus §42.3
  stehen wörtlich im Entwurf und deshalb **nicht** in `metriken-v1.json`: sie
  zu kalibrieren wäre eine Produktentscheidung, keine Messung. §42.3 verlangt
  für jede Änderung eine neue Policy-Version und Regressionstests — genau das
  leistet die Benennung: wer eine Zahl ändert, ändert eine benannte Konstante,
  und die Eigenschaftsprüfung fällt darüber.
- **Der Guardrail bleibt in `stop_if`, auch wenn er nicht messbar ist.** Ihn
  wegzulassen hieße, den Abbruch stillschweigend zu streichen. Die Folge steht
  woanders: die Aktion ist `more_data`, und es geht kein Angebot hinaus.
- **`darf_draft_offer` verlangt die Passage.** Das Exit-Gate nennt sie
  wörtlich; ein Vorschlag, der nicht sagen kann, **wo** er gilt, wird nicht
  angeboten — auch wenn alles andere stimmt.
- **In P5 ist `capability_vorhanden` immer falsch.** Der aktive Kern entsteht
  in P6. Ein Broker, der hier optimistisch `true` schriebe, erzeugte Angebote,
  die keine Probe ausführen kann. Das Feld steht trotzdem da, weil M-52
  „Capability fehlt" als eigenen Grund führt — und ein Grund, den niemand
  setzen kann, ist keiner.

**Nebenbefunde.**

- **N-14 (neu, im Werkzeug behoben).** Der Rotbeweistreiber schrieb die
  Rücknahme und startete `cargo test` sofort. Cargos Änderungserkennung hängt
  an der Dateizeit mit **einer Sekunde** Auflösung: vier rote Läufe liefen
  gegen ein **altes** Binary und meldeten grün. Ein Rotbeweis, der das alte
  Binary misst, ist wertlos — und er sieht aus wie ein bestandener. Der
  Treiber wartet jetzt zwischen Schreiben und Lauf. Der Treiber liegt
  außerhalb des Repos; die Lehre steht hier.
- **N-15 (neu, keine Änderung).** `security_vectors.rs::zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz`
  fiel in **einem** Sammellauf rot und lief allein sowie in allen späteren
  Sammelläufen grün. Der Fall öffnet 96 Worker und zwei Listener auf echten
  Named Pipes; unter paralleler Last ist er zeitempfindlich. Er gehört nicht
  zu diesem Ticket und wurde nicht angefasst — der Dirigent entscheidet.


### 7.7 Etappe G — `AssistantStep` im Main

**Gebaut:** die deterministische Zustandsmaschine des Assistenten — **im
Main**, persistent im `MainProjectState` (E-08). Acht Zustände, eine
P5-Übergangstabelle ohne Kante nach `preview`, fünf Angaben je Zustand, ein
struktureller Slot, vier Gegenpfade, die drei benannten Ergebnisse aus §46.2
und das deterministische Ranking mit harten Gates vor der Gewichtung.

| Stück | Ort |
|---|---|
| Zustandsmenge, Vertrag je Zustand, Übergangstabelle | `eq-copilot/plugin/state/NakamaState.h` und `eq-copilot/plugin/state/NakamaState.cpp` — `Assistentenschritt`, `Schrittvertrag`, `schrittvertrag()`, `p5UebergangErlaubt()`, `p5Naechster()` |
| Die vier Gegenpfade | `NakamaState.cpp` — `setzeAssistentenschritt()`, `assistentAbbrechen()`, `assistentZurueck()`, `assistentUeberspringen()`, `assistentResume()` |
| Die drei benannten Ergebnisse | `NakamaState.cpp` — `Assistentenergebnis`, `setzeAssistentenergebnis()` |
| Ranking mit harten Gates | `NakamaState.cpp` — `Schrittkandidat`, `ordneSchritte()`, `schrittrang()` |
| Persistenz | `NakamaState.cpp` — `assistant_step_v1` als **eine** additive Eigenschaft mit Fassung im Namen; Schreiber, Leser und der Headroomriegel |
| Produktpfad | `eq-copilot/plugin/src/PluginProcessor.h` und `eq-copilot/plugin/src/PluginProcessor.cpp` — `assistentStarten()`, `assistentWeiter()`, `assistentZurueck()`, `assistentUeberspringen()`, `assistentAbbrechen()`, `assistentAntwort()`, `assistentAusState()`, `assistentFortsetzen()`, `assistentVersuchStarten()`, `assistentAenderungMelden()` |
| Bein | **NEU** `eq-copilot/plugin/tests/Sonde014AssistentTest.cpp`, Kanonbein **B29**, 119 Prüfungen |

**Gemessene Matrixzeilen.**

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-55** | `Sonde014AssistentTest.cpp` — die Menge hat **acht** Werte mit eigenem Rückweg, ein neunter fällt; nach `preview` führt **keine** Kante, in keiner Richtung; die P5-Folge ist vollständig verkettet; ein Sprung über zwei Zustände ist keine Kante; die API setzt `preview` nicht | `roh/SONDE-014-rot-M-55.txt` |
| **M-55** (Leser) | derselbe Fall: ein **gespeicherter** `preview`-Schritt wird read-only, der Grund nennt ihn beim Namen, und die **Gegenprobe** mit `proposal` lädt normal | `roh/SONDE-014-rot-M-55-leser.txt` |
| **M-56** | `…` — Tabellentest über **alle acht** Zustände: fünf Angaben, Timeout > 0, Rückkante im Wertebereich; auch `preview` trägt seinen vollständigen Vertrag; der erste Zustand ist seine eigene sichere Rückkante | `roh/SONDE-014-rot-M-56.txt` |
| **M-57** | `…` — ein zweiter Startversuch bei offenem Schritt wird **abgewiesen**, nicht eingereiht; der erste steht unverändert; nach dem terminalen Abbruch beginnt ein neuer. Verdrahtet am echten Prozessor | `roh/SONDE-014-rot-M-57.txt` |
| **M-58** | `…` — vier Gegenpfade einzeln: Zurück auf die Rückkante mit **genau einer** Revision, Überspringen auf den nächsten Zustand, Resume ohne Revisionssprung, Abbruch terminal statt gelöscht; ein zweiter Abbruch fällt | `roh/SONDE-014-rot-M-58.txt`, `roh/SONDE-014-rot-M-58-resume.txt` |
| **M-59** | `…` — Save/Load über den echten Prozessor, Rekonstruktion aus dem gespeicherten `MainProject`, Fortsetzung an derselben Stelle, bytegleiches Save/Load über zwei Runden | `roh/SONDE-014-rot-M-59.txt` |
| **M-60** | `…` — drei Kandidaten mit **perfektem** Rang und je einem gerissenen Gate verlassen die Liste, ein bescheidener bleibt; bei Gleichstand gewinnt die kleinere Kennung; NaN und Ausreißer vergiften den Rang nicht | `roh/SONDE-014-rot-M-60.txt` |
| **M-61** | `…` — die drei Ergebnisse einzeln, jedes mit eigener Revision und offenem Schritt; dasselbe Ergebnis zweimal hebt keine Revision | `roh/SONDE-014-rot-M-61.txt` |
| **M-62** | `…` — ohne offenen Schritt startet der Assistent nichts, und mit offenem Schritt hängt er **vollständig** an `beginneVersuch`: schlägt das fehl, trägt der Schritt **keine** Versuchskennung. Ein Assistent mit eigenem Weg hätte hier eine | `roh/SONDE-014-rot-M-62.txt` |
| **M-71** (Main-Hälfte) | dasselbe Bein: die Maschine läuft im Main, der Broker spiegelt nur (`assistent.rs`, §7.2). Die Wire-Hälfte liegt in M-88/M-89 | — |

**Läufe.** `EqCopSonde014AssistentTest` **119 Prüfungen, 0 Fehler** (neues
Kanon-Bein **B29**); `EqCopStateMigrationTest` **180** (Bein **B2**),
`EqCopSonde012ProjectReloadTest` **12/12** (Bein **B14**),
`EqCopSonde014IntentTest` **210** (Bein **B27**) — alle unverändert grün nach
der neuen Eigenschaft. Die Kanonzahl steigt von **56** auf **57**.

**Rotbeweise.** Zehn Dateien
`docs/beweise/roh/SONDE-014-rot-{M-55,M-55-leser,M-56,M-57,M-58,M-58-resume,M-59,M-60,M-61,M-62}.txt`.
Jede fährt den vollen Weg: Rücknahme setzen, Ziel neu bauen, Binary fahren.

**Abweichungen von §5, mit Begründung.**

1. **`B25` und `B26` sind nicht erweitert.** §5.1 führt sie als „bestehend".
   **B25** misst Bootstrap und FDR, **B26** die zwei Kanten des
   Blindvergleichs; beide gehören dem Experiment, nicht dem Assistenten. Der
   Anschluss aus M-62 ist gemessen — an der Stelle, an der er wirklich liegt:
   dass der Assistent **keinen eigenen** Weg hat. Eine Erweiterung von B25
   oder B26 ohne neue Zusage wäre eine Behauptung über eine Messung, die es
   nicht gibt.
2. **`A4` und `A4-SI` sind nicht erweitert.** Sie messen die **Brokerseite**;
   dort liegt seit Etappe B der Spiegel (`assistent.rs`, M-88/M-89) und ist
   dort gemessen. Diese Etappe fügt der Brokerseite nichts hinzu.

**Technische Entscheide dieser Etappe.**

- **`preview` trägt seinen vollständigen Vertrag.** Der Zustand existiert und
  wird in P6 gebraucht; ihn hier leer zu lassen hieße, ihn halb zu streichen.
  Was fehlt, ist die **Kante** — und die fehlt in beide Richtungen.
- **Die sichere Rückkante ist immer ein Zustand, nie ein `optional`.** Der
  erste Zustand zeigt auf sich selbst: „bleib, wo du bist". Ein `optional`
  hätte einen Zustand ohne Rückkante möglich gemacht, und genau den nennt
  M-56 einen Vertragsbruch.
- **Der Slot ist strukturell.** `Zustand` hält **einen** `Assistentenzustand`.
  Ein zweiter gleichzeitiger Schritt ist nicht „verboten", sondern nicht
  darstellbar — das ist der Unterschied zu einem Zähler, der auch 2 tragen
  könnte.
- **Der Abbruch gibt den Slot erst nach dem Terminalereignis frei.** M-58
  nennt die Reihenfolge wörtlich: Terminalereignis → Projektion → Outbox →
  erst danach der Slot. `assistentAbbrechen` meldet deshalb erst den
  terminalen Schritt und ruft **danach** `brichVersuchAb()`.
- **Resume ist eine Frage.** Es setzt nichts und hebt keine Revision. Ein
  Resume, das den Zustand anfasst, wäre nicht unterscheidbar von einem
  Fortschritt — und der User sähe eine Änderung, die er nicht gemacht hat.
- **Ein nicht gesetzter Schritt reist gar nicht.** „Noch nie einen Assistenten
  benutzt" und „ein Schritt mit leeren Feldern" wären in den Bytes sonst
  dasselbe, und der Broker könnte den Spiegel nicht davon unterscheiden
  (M-88). Dieselbe Begründung wie bei `intent_revision_v1` in §7.1.
- **Die Timeouts sind Produktgrenzen, keine Messung.** Sie stehen als Zahlen
  im Vertrag je Zustand und nicht in `metriken-v1.json`: eine Kalibrierung
  wäre eine Entscheidung über Bedienung, nicht über Messgenauigkeit. Die
  Größenordnung folgt der Handlung — Hören dauert länger als Lesen.

**Nebenbefunde.**

- **N-16 (neu, im Rotbeweistreiber behoben).** Die erste Fassung der Rücknahme
  für M-55 entfernte nur die Wache `nach == preview`. Sie lief **grün**: die
  P5-Folge führt ohnehin keine Kante dorthin, und die Wache war nur die zweite
  Verteidigungslinie. Die wirksame Rücknahme setzt die **Folge** zurück auf
  `proposal → preview` — dieselbe Klasse Fehler wie N-14: eine Rücknahme, die
  nichts zurücknimmt, sieht aus wie eine bestandene Prüfung.

### 7.8 Etappe H — Der P5-Evaluationskorpus als Kette

**Gebaut:** der Evaluationskorpus — und zwar als **Kette** (Regel R2), nicht
als Deklarationsregister. Sechs synthetische Sitzungen laufen als
Evidenzbestand durch `p1` und damit durch dieselbe Senke, die der echte
Transport ruft; gelesen wird die **tatsächlich ausgegebene** Hypothese und
gegen die Wahrheit des Falls gehalten. Eine falsche starke Produktbehauptung
ändert die Korpusdateien nicht — sie fällt am Vergleich.

Der Unterschied ist nicht theoretisch. Der Korpus hat in dieser Etappe **zwei
echte Produktfehler** gefunden, die kein Bein dieses Tickets gesehen hatte
(N-17 und N-18 unten). Genau dafür wurde er als Kette verlangt.

| Stück | Ort |
|---|---|
| Die sechs Sitzungen und ihre Wahrheit | **NEU** `tools/eq-copilot/erzeuge_p5_korpus.py` → `eq-copilot/fixtures/p5-korpus/sitzungen.json`, `eq-copilot/fixtures/p5-korpus/MANIFEST.json` |
| Der Produktpfad, der sie fährt | **NEU** `broker/tests/sonde014_p5_korpus.rs` — `korpus_kette_laeuft_durch_den_produktpfad`, schreibt `eq-copilot/build/p5-korpus-ergebnis.json` |
| Der Vergleich, Kennzahlen und Riegel | **NEU** `tools/eq-copilot/pruefe_p5_korpus.py` — `kennzahlen()`, `riegel()`, `schwelle_suchen()`, `produktschwelle()`, `p4_luecke()` |
| Der Selbsttest ohne Repo-Fixture | dieselbe Datei — `selbsttest()`, **38** Prüfungen, jede Erwartung mit ihrem Gegenteil |
| Die Trennungsregel im Produkt | `broker/src/coordinator/hypothese.rs` — der Riegel gegen den **ungetrennten** ersten Platz |
| Beine | **NEU** `A28` (Erzeuger, `--pruefen`), `A29` (Sammelbein), `A30` (Selbsttest) in `tools/beweise.ps1`; das Kettenbein läuft unter **A4** |

**Die sechs Sitzungen.** Jede trägt genau eine Wahrheit und eine Obergrenze
der erlaubten Sicherheit.

| Sitzung | Wahrheit | höchstens | wofür sie da ist |
|---|---|---|---|
| `wahrer_kandidat` | `wahre_ursache` | `hoch` | der **positive** Fall — ohne ihn wäre jeder Riegel trivial erfüllt |
| `korrelierter_distraktor` | `distraktor` | `mittel` | zwei Quellen laufen gleich; der Distraktor darf **Alternative** sein, nie Ursache |
| `parent_duplikat` | `parent_duplikat` | `mittel` | zwei Sonden auf demselben Mixerkanal messen dasselbe Signal (M-22) |
| `verschobene_passage` | `verschobene_passage` | `unklar` | das Alignmentgate reißt, der Kandidat scheidet **mit Grund** aus (M-20, M-87) |
| `zu_kurze_passage` | `zu_kurze_passage` | `mittel` | der Fall aus D1: die vier **relativen** Gates bestehen ihn alle, nur `GATE_MINDEST_FENSTER` fängt ihn |
| `daten_reichen_nicht` | `keine_ursache` | `unklar` | das Gegenbeispiel; die Enthaltung ist das **gewünschte** Ergebnis (M-27, §49.4) |

**Gemessene Matrixzeilen.**

| Zeile | Wo gemessen | Rotbeweis |
|---|---|---|
| **M-64** | `pruefe_p5_korpus.py` — Precision und Recall **je Ursachenklasse**, dazu Brier, Kalibrierung, Coverage und Enthaltung; die vier Riegel aus NAK-182 als eigene Funktion, die der Selbsttest synthetisch füttert. Die Rücknahme nimmt einem Riegel den Weg zu fallen | `roh/SONDE-014-rot-M-64.txt` |
| **M-65** | die Kette selbst: die Sitzung `korrelierter_distraktor` bei **unveränderten** Korpusdateien. Die Rücknahme sitzt im Produkt, und der Vergleich meldet `1 falsche starke Behauptung` | `roh/SONDE-014-rot-M-65.txt` |
| **M-65** (Hygiene) | `erzeuge_p5_korpus.py` — der Bezeichner des Kettenbeins steht **wörtlich** in seiner Datei; ein umbenannter Fall fällt beim Erzeuger, nicht erst dem Leser auf | `roh/SONDE-014-rot-M-65-hygiene.txt` |
| **M-66** | `selbsttest()` — die Schwelle wird **gesucht**, nicht gesetzt; beide Richtungen (nur `hoch` hält / jede Stufe hält) | `roh/SONDE-014-rot-M-66.txt` |
| **M-67** | die Kette: `verschobene_passage` trägt keine starke Aussage und nennt `alignment_falsch`. Ohne das Gate fehlt **beides** | `roh/SONDE-014-rot-M-67.txt` |
| **M-68** | `selbsttest()` — die Rücknahme macht einen Riegel **immer rot**; gefallen sind ausschließlich die **GEGENTEIL**-Erwartungen. Genau das ist der Satz aus M-68, gemessen statt behauptet | `roh/SONDE-014-rot-M-68.txt` |
| **M-69** | die Kette: das Gegenbeispiel erzwingt Enthaltung **nachweislich** — die Rücknahme lässt `enthaltung()` stark behaupten, und zwei Sitzungen fallen | `roh/SONDE-014-rot-M-69.txt` |
| **M-70** | `p4_luecke()` — die gedruckte Lücke aus `eq-copilot/fixtures/p4-korpus/MANIFEST.json` gegen ihre P5-Messung, in **beide** Richtungen gekoppelt | `roh/SONDE-014-rot-M-70.txt` |
| **M-31** | die Kette: handelbar (`ready_to_send`) ist genau `hoch`, nichts darunter. Die Rücknahme macht `mittel` handelbar, und fünf Befunde fallen | `roh/SONDE-014-rot-M-31.txt` |
| **M-23** (Kalibrierung) | die Kette: `GATE_MINDEST_FENSTER` am Korpus kalibriert. Die Rücknahme senkt den Wert auf 1, und `zu_kurze_passage` trägt eine starke Aussage | `roh/SONDE-014-rot-M-23-kalibrierung.txt` |
| **Voraussetzung** | ohne frische Ergebnisdatei meldet das Sammelbein **Exit 3**, nicht grün — der Fehler aus `tools/dirigent/pruefliste.md` Abschnitt D | `roh/SONDE-014-rot-Voraussetzung.txt` |

**Läufe.** `erzeuge_p5_korpus.py --pruefen` **bytegleich** (Bein **A28**);
`cargo test --test sonde014_p5_korpus` **1/1** (unter Bein **A4**);
`pruefe_p5_korpus.py` **6 Sitzungen, 8 ausgegebene Befunde, grün** (Bein
**A29**); `pruefe_p5_korpus.py --selbsttest` **38 Prüfungen, 0 Fehler** (Bein
**A30**). Die volle Broker-Suite **616 bestanden, 0 Fehler** nach beiden
Produktänderungen. Die Kanonzahl steigt von **57** auf **60**.

**Das Ergebnis des Korpus, als Zahl.**

| Kennzahl | Wert | was sie sagt |
|---|---|---|
| Precision / Recall | **1,000 / 1,000** je Ursachenklasse und gesamt | keine falsche starke, keine falsche schwache Behauptung |
| Brier | **0,135** | über die **Klasse**, nicht über den Rang — siehe Entscheid unten |
| Kalibrierung | **0,350** | das Produkt ist **unter**sicher, nicht fehlkalibriert: Nennwert 0,60 gegen gemessene Trefferquote 1,000 |
| Zuverlässigkeit `hoch` | n=1, Trefferquote **1,000** | von dem, was das Produkt `hoch` nannte, war alles richtig |
| Zuverlässigkeit `mittel` | n=5, Trefferquote **1,000** | dasselbe eine Stufe tiefer |
| Coverage / Enthaltung | **0,750 / 0,250** | zwei der acht Befunde sind Enthaltungen, und das ist kein Fehlschlag |
| Schwelle (M-31, **Ausgabe**) | niedrigste haltende Stufe = **`unklar`** | auf diesem Korpus hielte sogar jede Stufe; das Produkt handelt nur bei `hoch` und ist damit **strenger** als nötig |
| `GATE_MINDEST_FENSTER` | 2 Passagenfälle, **0 davon stark** | der **Startwert 8 hält** |

**Zwei Produktfunde des Korpus.**

- **N-17 — zwei starke Behauptungen über dieselbe Ursache.** Die Sitzung
  `korrelierter_distraktor` lieferte **beide** Kandidaten mit `hoch` und
  `READY TO SEND`. Das ist die falsche starke Behauptung, die §36.4 Satz 1
  verbietet, und dieselbe Klasse Fehler wie das Parent-Duplikat aus M-22.
  Behoben in `broker/src/coordinator/hypothese.rs`: jeder **nicht führende**
  Befund fällt auf `mittel` zurück.
- **N-18 — der führende Platz behauptete stark, obwohl ihn nichts trug.**
  Nach dem Fix zu N-17 blieb ein tieferer Fehler übrig, und er war schwerer zu
  sehen: In dieser Sitzung sind die zwei Kandidaten im Material **nicht
  unterscheidbar** — gleiches Band, gleiche Anhebung, gleiche Fenster,
  identischer Rang. Wer führt, entschied allein der Gleichstandsschlüssel, die
  aufsteigende `candidate_source`. Das Produkt setzte auf diesen Platz ein
  `hoch`. Die Behauptung wäre richtig oder falsch gewesen, je nachdem wie die
  Kennungen zufällig liegen — eine Münzwurf-Aussage mit dem Anschein von
  Sicherheit, also genau die „überzeugende falsche Ursache" aus §49.4.
  Behoben in derselben Datei: **ein ungetrennter erster Platz trägt keine
  starke Aussage.** Getrennt heißt, dass der Rangabstand größer ist als
  `RANG_QUANTUM` — dieselbe Auflösung, in der die Sortierung darüber
  vergleicht. Zwei Kandidaten, die die Sortierung nicht trennen konnte, darf
  die Sicherheit nicht trennen. Beide bleiben sichtbar, jeder als Alternative
  des anderen.

  ⚠️ **Der Fund traf auch den Korpus selbst.** Die Sitzung erwartete zuerst
  `hoch`, und die wahre Ursache lag auf der **kleineren** Kennung. Der Fall
  hätte also gemessen, ob die wahre Ursache zufällig die kleinere Kennung
  trägt — ein Bein, das aus dem richtigen Grund grün aussieht. Die Erwartung
  steht deshalb jetzt auf `mittel`, mit dieser Begründung im `hinweis` der
  Sitzung. Ein Korpus darf nicht messen, was das Produkt nicht wissen kann.

**Technische Entscheide dieser Etappe.**

- **Brier und Kalibrierung laufen über die Sicherheits-KLASSE, nicht über den
  `score`.** Der erste Entwurf rechnete sie über `confidence.score` und meldete
  Brier 0,588 — eine Zahl, die schlecht aussieht und nichts sagt. Der Grund:
  `score` ist `rang.rang()`, das quantisierte Mittel der sechs Rangkomponenten,
  also eine **Rangzahl** und keine Trefferwahrscheinlichkeit;
  `broker/src/coordinator/hypothese.rs` hält ausdrücklich fest, dass die Klasse
  nie aus dem Score gerundet wird (M-15, M-20). Brier über einen Rang zu rechnen
  ist ein Kategoriefehler. Vorhergesagt wird deshalb die Klasse über die Tabelle
  `KLASSENNENNWERT`, und diese Tabelle lebt **im Messwerkzeug**, nicht im
  Produkt: sie dorthin zurückzulesen wäre genau die von M-15 verbotene Rundung.
  Der Rang bleibt als `rangmittel` sichtbar (§42.4), nur getrennt von der
  Kalibrierung. Daneben steht die annahmefreie **Zuverlässigkeitstafel**: „von
  dem, was das Produkt `hoch` nannte, waren X % richtig."
- **Die Schwelle aus M-31 wird gesucht, und zusätzlich wird die des Produkts
  gelesen.** `schwelle_suchen()` misst die niedrigste haltende Stufe.
  `produktschwelle()` liest daneben, was das Produkt **wirklich** tut, aus dem
  `zustand` jedes ausgegebenen Befunds. Geprüft wird eine **Äquivalenz**: jeder
  handelnde Befund trägt `hoch`, und jeder frische `hoch`-Befund handelt. Nur
  eine Richtung zu prüfen ließe ein Produkt durch, das gar nichts mehr handelt
  — ein Riegel, der auf dem Weg zur Sicherheit die Zusage verliert. Ein
  veralteter Befund ist die benannte Ausnahme.
- **`GATE_MINDEST_FENSTER` bleibt bei 8 — E-12 Fall 3, kein Versionsschritt.**
  Der Entscheid E-12 erlaubt genau einen `metrics_version`-Schritt in dieser
  Etappe, und zwar dann, wenn die Kalibrierung den Wert bewegt. Sie bewegt ihn
  nicht: beide Passagensitzungen tragen **null** starke Aussagen, der Startwert
  hält. Der Rotbeweis `M-23-kalibrierung` zeigt, dass diese Aussage einen Weg
  hat zu fallen — mit dem Wert 1 trägt `zu_kurze_passage` sofort eine starke
  Aussage, und das Bein meldet wörtlich „`GATE_MINDEST_FENSTER` muss steigen".
  Damit ist der Startwert **gemessen bestätigt**, nicht bloß unverändert.
- **M-70 baut den Riegel, nicht die Entfernung.** Die Zeile `nicht_gemessen`
  in `eq-copilot/fixtures/p4-korpus/MANIFEST.json` bleibt stehen. Der P5-Korpus
  misst den Fall jetzt (die Sitzung `korrelierter_distraktor` gibt den
  Distraktor als Alternative aus), und das Sammelbein druckt genau das:
  „gedruckt, inzwischen gemessen — sie darf jetzt entfernt werden". Die
  **Entfernung** selbst gehört dem P4-Korpus und seinem Erzeuger; sie hier
  nebenbei mitzunehmen hieße, eine fremde Fixture während dieser Etappe neu zu
  erzeugen. Gebaut ist stattdessen die **Kopplung**: verschwindet die Zeile,
  ohne dass eine P5-Sitzung den Fall misst, ist das rot.
- **Die Voraussetzung ist eine Messung, keine Annahme.** Fehlt die
  Ergebnisdatei oder ist sie älter als `hypothese.rs`,
  `hypothese_verdrahtung.rs`, `sonde014_p5_korpus.rs` oder `sitzungen.json`,
  meldet das Sammelbein **Exit 3**. Ein Bein, das ein veraltetes Artefakt als
  frisch bezeugt, ist derselbe Fehler, den N-14 in Etappe F an vier
  Rotbeweisen gefunden hat.
- **Das Kettenbein rechnet keine Kennzahlen.** Es fährt und schreibt, was
  herauskam. Rechnete es selbst, wäre die Messung wieder im selben Prozess wie
  das Gemessene, und ein grünes Bein hieße nur noch, dass es mit sich selbst
  einig ist.

**Abweichungen von §5, mit Begründung.**

1. **Das Kettenbein läuft unter `A4`, nicht als eigenes Bein.** §5.1 verlangt
   „ein Rust-Integrationsbein im **A4**-Bereich"; genau dort liegt es. Die drei
   **neuen** Beine `A28`, `A29` und `A30` sind Erzeuger, Sammelbein und
   Selbsttest — die drei, die §5.1 als anzulegen führt.
2. **`A26` ist nicht erweitert.** Es misst den **P4**-Korpus und druckt dessen
   Lücke. Die Kopplung zwischen der gedruckten Lücke und ihrer P5-Messung
   gehört auf die Seite, die neu misst, und liegt deshalb in
   `pruefe_p5_korpus.py`. `A26` um eine Zusage über P5 zu erweitern hieße, dem
   P4-Bein eine Messung zuzuschreiben, die es nicht fährt.

**Nebenbefunde.**

- **N-17** und **N-18** stehen oben; beide sind behoben und mit Rotbeweis
  belegt.
- **N-19 (neu, im eigenen Werkzeug behoben).** Der erste Selbsttest enthielt
  die Prüfung `k["brier"] > 0.0 or k["brier"] == 0.0` — für jede
  nicht-negative Zahl wahr. Sie hatte **keinen Weg zu fallen** und ist damit
  genau der Fehler, den M-68 und Risiko **R5** benennen, im Werkzeug, das ihn
  finden soll. Ersetzt durch den echten Vergleich: derselbe Brier mit und ohne
  Enthaltung, dazu die Gegenrichtung, dass eine **echte** zweite Behauptung ihn
  sehr wohl bewegt.

---

---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-06 19:34 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 55/55 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-8f030f5.md](roh/SONDE-014-8f030f5.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-06 19:34:53 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 8f030f5 SONDE-014 Etappe A: SourceIntent im Main-State (M-01 bis M-13) |
| Commit (voll) | 8f030f5e8563f65e74e2dcf54c34fd3de7355809 |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,14 s | [A1](roh/SONDE-014-8f030f5.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,97 s | [A2](roh/SONDE-014-8f030f5.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,75 s | [A3](roh/SONDE-014-8f030f5.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 84,93 s | [A4](roh/SONDE-014-8f030f5.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,06 s | [A4-SI](roh/SONDE-014-8f030f5.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 65,73 s | [A4b](roh/SONDE-014-8f030f5.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 2,62 s | [A5](roh/SONDE-014-8f030f5.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,14 s | [A6](roh/SONDE-014-8f030f5.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [A7](roh/SONDE-014-8f030f5.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [A8](roh/SONDE-014-8f030f5.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,34 s | [A9](roh/SONDE-014-8f030f5.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,69 s | [A10](roh/SONDE-014-8f030f5.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [A11](roh/SONDE-014-8f030f5.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [A12](roh/SONDE-014-8f030f5.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,20 s | [A13](roh/SONDE-014-8f030f5.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 6,37 s | [A14](roh/SONDE-014-8f030f5.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,14 s | [A17](roh/SONDE-014-8f030f5.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 35,54 s | [A18](roh/SONDE-014-8f030f5.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [A19](roh/SONDE-014-8f030f5.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A20](roh/SONDE-014-8f030f5.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,34 s | [A21](roh/SONDE-014-8f030f5.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,06 s | [A22](roh/SONDE-014-8f030f5.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,61 s | [A24](roh/SONDE-014-8f030f5.md#a24) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,16 s | [A25](roh/SONDE-014-8f030f5.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,15 s | [A26](roh/SONDE-014-8f030f5.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,17 s | [A27](roh/SONDE-014-8f030f5.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,64 s | [A23](roh/SONDE-014-8f030f5.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,11 s | [A16](roh/SONDE-014-8f030f5.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,16 s | [B1](roh/SONDE-014-8f030f5.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,29 s | [B2](roh/SONDE-014-8f030f5.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/SONDE-014-8f030f5.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-014-8f030f5.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,37 s | [B3c](roh/SONDE-014-8f030f5.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,36 s | [B4](roh/SONDE-014-8f030f5.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B9](roh/SONDE-014-8f030f5.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,71 s | [B5](roh/SONDE-014-8f030f5.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 213,55 s | [B10](roh/SONDE-014-8f030f5.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,04 s | [B11](roh/SONDE-014-8f030f5.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,63 s | [B12](roh/SONDE-014-8f030f5.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/SONDE-014-8f030f5.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,17 s | [B14](roh/SONDE-014-8f030f5.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,39 s | [B15](roh/SONDE-014-8f030f5.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/SONDE-014-8f030f5.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,92 s | [B22](roh/SONDE-014-8f030f5.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,52 s | [B23](roh/SONDE-014-8f030f5.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,05 s | [B24](roh/SONDE-014-8f030f5.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/SONDE-014-8f030f5.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,47 s | [B26](roh/SONDE-014-8f030f5.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-8f030f5.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 2,03 s | [B19](roh/SONDE-014-8f030f5.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [B17](roh/SONDE-014-8f030f5.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 4,34 s | [B18](roh/SONDE-014-8f030f5.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,55 s | [B16](roh/SONDE-014-8f030f5.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,35 s | [B27](roh/SONDE-014-8f030f5.md#b27) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,29 s | [B8](roh/SONDE-014-8f030f5.md#b8) |


---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-06 20:37 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 55/55 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-581431a.md](roh/SONDE-014-581431a.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-06 20:37:52 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 581431a SONDE-014 Etappe B, Teil 3: elf Rotbeweise und der Riegel gegen die zweite Kopie |
| Commit (voll) | 581431a733cbbfc5335ce928f9113bd8e1830421 |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-014-581431a.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,87 s | [A2](roh/SONDE-014-581431a.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,62 s | [A3](roh/SONDE-014-581431a.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 91,99 s | [A4](roh/SONDE-014-581431a.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,60 s | [A4-SI](roh/SONDE-014-581431a.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 66,23 s | [A4b](roh/SONDE-014-581431a.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,09 s | [A5](roh/SONDE-014-581431a.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [A6](roh/SONDE-014-581431a.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/SONDE-014-581431a.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [A8](roh/SONDE-014-581431a.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [A9](roh/SONDE-014-581431a.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,88 s | [A10](roh/SONDE-014-581431a.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [A11](roh/SONDE-014-581431a.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,27 s | [A12](roh/SONDE-014-581431a.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,21 s | [A13](roh/SONDE-014-581431a.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 6,94 s | [A14](roh/SONDE-014-581431a.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,11 s | [A17](roh/SONDE-014-581431a.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 36,52 s | [A18](roh/SONDE-014-581431a.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/SONDE-014-581431a.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A20](roh/SONDE-014-581431a.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,34 s | [A21](roh/SONDE-014-581431a.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,02 s | [A22](roh/SONDE-014-581431a.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,57 s | [A24](roh/SONDE-014-581431a.md#a24) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,14 s | [A25](roh/SONDE-014-581431a.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,14 s | [A26](roh/SONDE-014-581431a.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/SONDE-014-581431a.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,54 s | [A23](roh/SONDE-014-581431a.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,09 s | [A16](roh/SONDE-014-581431a.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,13 s | [B1](roh/SONDE-014-581431a.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,36 s | [B2](roh/SONDE-014-581431a.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/SONDE-014-581431a.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-014-581431a.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,36 s | [B3c](roh/SONDE-014-581431a.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,37 s | [B4](roh/SONDE-014-581431a.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/SONDE-014-581431a.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,70 s | [B5](roh/SONDE-014-581431a.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 213,74 s | [B10](roh/SONDE-014-581431a.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 0,98 s | [B11](roh/SONDE-014-581431a.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,60 s | [B12](roh/SONDE-014-581431a.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/SONDE-014-581431a.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,21 s | [B14](roh/SONDE-014-581431a.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,40 s | [B15](roh/SONDE-014-581431a.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/SONDE-014-581431a.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,92 s | [B22](roh/SONDE-014-581431a.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,54 s | [B23](roh/SONDE-014-581431a.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,10 s | [B24](roh/SONDE-014-581431a.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,07 s | [B25](roh/SONDE-014-581431a.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,57 s | [B26](roh/SONDE-014-581431a.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-581431a.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,96 s | [B19](roh/SONDE-014-581431a.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B17](roh/SONDE-014-581431a.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 4,28 s | [B18](roh/SONDE-014-581431a.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,55 s | [B16](roh/SONDE-014-581431a.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,29 s | [B27](roh/SONDE-014-581431a.md#b27) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,28 s | [B8](roh/SONDE-014-581431a.md#b8) |


---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-06 21:58 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 55/55 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-2e2c97c.md](roh/SONDE-014-2e2c97c.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-06 21:58:38 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 2e2c97c SONDE-014 Etappe C, Teil 3: sechzehn Rotbeweise und Manifest 7.3 |
| Commit (voll) | 2e2c97cb69cd8352fc50720c7cafb3317b31cde6 |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-014-2e2c97c.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,69 s | [A2](roh/SONDE-014-2e2c97c.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,80 s | [A3](roh/SONDE-014-2e2c97c.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 130,61 s | [A4](roh/SONDE-014-2e2c97c.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 13,16 s | [A4-SI](roh/SONDE-014-2e2c97c.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 65,74 s | [A4b](roh/SONDE-014-2e2c97c.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 2,92 s | [A5](roh/SONDE-014-2e2c97c.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [A6](roh/SONDE-014-2e2c97c.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/SONDE-014-2e2c97c.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [A8](roh/SONDE-014-2e2c97c.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [A9](roh/SONDE-014-2e2c97c.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,25 s | [A10](roh/SONDE-014-2e2c97c.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [A11](roh/SONDE-014-2e2c97c.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,29 s | [A12](roh/SONDE-014-2e2c97c.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,21 s | [A13](roh/SONDE-014-2e2c97c.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 5,84 s | [A14](roh/SONDE-014-2e2c97c.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,11 s | [A17](roh/SONDE-014-2e2c97c.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 33,82 s | [A18](roh/SONDE-014-2e2c97c.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,16 s | [A19](roh/SONDE-014-2e2c97c.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,35 s | [A20](roh/SONDE-014-2e2c97c.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,32 s | [A21](roh/SONDE-014-2e2c97c.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,02 s | [A22](roh/SONDE-014-2e2c97c.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,60 s | [A24](roh/SONDE-014-2e2c97c.md#a24) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,14 s | [A25](roh/SONDE-014-2e2c97c.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,14 s | [A26](roh/SONDE-014-2e2c97c.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/SONDE-014-2e2c97c.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,68 s | [A23](roh/SONDE-014-2e2c97c.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,19 s | [A16](roh/SONDE-014-2e2c97c.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,13 s | [B1](roh/SONDE-014-2e2c97c.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,21 s | [B2](roh/SONDE-014-2e2c97c.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [B3](roh/SONDE-014-2e2c97c.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-014-2e2c97c.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,40 s | [B3c](roh/SONDE-014-2e2c97c.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,59 s | [B4](roh/SONDE-014-2e2c97c.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/SONDE-014-2e2c97c.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,39 s | [B5](roh/SONDE-014-2e2c97c.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 214,12 s | [B10](roh/SONDE-014-2e2c97c.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,08 s | [B11](roh/SONDE-014-2e2c97c.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,60 s | [B12](roh/SONDE-014-2e2c97c.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/SONDE-014-2e2c97c.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,20 s | [B14](roh/SONDE-014-2e2c97c.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,26 s | [B15](roh/SONDE-014-2e2c97c.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,07 s | [B20](roh/SONDE-014-2e2c97c.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,96 s | [B22](roh/SONDE-014-2e2c97c.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,50 s | [B23](roh/SONDE-014-2e2c97c.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 0,99 s | [B24](roh/SONDE-014-2e2c97c.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/SONDE-014-2e2c97c.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,49 s | [B26](roh/SONDE-014-2e2c97c.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-2e2c97c.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,91 s | [B19](roh/SONDE-014-2e2c97c.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [B17](roh/SONDE-014-2e2c97c.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 4,01 s | [B18](roh/SONDE-014-2e2c97c.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,51 s | [B16](roh/SONDE-014-2e2c97c.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,43 s | [B27](roh/SONDE-014-2e2c97c.md#b27) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,29 s | [B8](roh/SONDE-014-2e2c97c.md#b8) |


---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-06 22:30 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 56/56 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-c303554.md](roh/SONDE-014-c303554.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-06 22:30:04 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | c303554 SONDE-014 Etappe D, Teil 3: neun Rotbeweise und Manifest 7.4 |
| Commit (voll) | c3035542444e5cc14d2bbbb854e1b043814aa0e3 |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-014-c303554.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,63 s | [A2](roh/SONDE-014-c303554.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,77 s | [A3](roh/SONDE-014-c303554.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 126,76 s | [A4](roh/SONDE-014-c303554.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 13,01 s | [A4-SI](roh/SONDE-014-c303554.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 66,22 s | [A4b](roh/SONDE-014-c303554.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,84 s | [A5](roh/SONDE-014-c303554.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [A6](roh/SONDE-014-c303554.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [A7](roh/SONDE-014-c303554.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,26 s | [A8](roh/SONDE-014-c303554.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,40 s | [A9](roh/SONDE-014-c303554.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,83 s | [A10](roh/SONDE-014-c303554.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,28 s | [A11](roh/SONDE-014-c303554.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,29 s | [A12](roh/SONDE-014-c303554.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,33 s | [A13](roh/SONDE-014-c303554.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 6,02 s | [A14](roh/SONDE-014-c303554.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,15 s | [A17](roh/SONDE-014-c303554.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 33,82 s | [A18](roh/SONDE-014-c303554.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/SONDE-014-c303554.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A20](roh/SONDE-014-c303554.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,33 s | [A21](roh/SONDE-014-c303554.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,01 s | [A22](roh/SONDE-014-c303554.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,67 s | [A24](roh/SONDE-014-c303554.md#a24) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,34 s | [A25](roh/SONDE-014-c303554.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,14 s | [A26](roh/SONDE-014-c303554.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/SONDE-014-c303554.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,54 s | [A23](roh/SONDE-014-c303554.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/SONDE-014-c303554.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,14 s | [B1](roh/SONDE-014-c303554.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,18 s | [B2](roh/SONDE-014-c303554.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,13 s | [B3](roh/SONDE-014-c303554.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,10 s | [B3b](roh/SONDE-014-c303554.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,43 s | [B3c](roh/SONDE-014-c303554.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,41 s | [B4](roh/SONDE-014-c303554.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/SONDE-014-c303554.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,39 s | [B5](roh/SONDE-014-c303554.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 213,91 s | [B10](roh/SONDE-014-c303554.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,00 s | [B11](roh/SONDE-014-c303554.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,59 s | [B12](roh/SONDE-014-c303554.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,07 s | [B13](roh/SONDE-014-c303554.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,22 s | [B14](roh/SONDE-014-c303554.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,28 s | [B15](roh/SONDE-014-c303554.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/SONDE-014-c303554.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,91 s | [B22](roh/SONDE-014-c303554.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,50 s | [B23](roh/SONDE-014-c303554.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,03 s | [B24](roh/SONDE-014-c303554.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/SONDE-014-c303554.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,47 s | [B26](roh/SONDE-014-c303554.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-c303554.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,85 s | [B19](roh/SONDE-014-c303554.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [B17](roh/SONDE-014-c303554.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 4,08 s | [B18](roh/SONDE-014-c303554.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,51 s | [B16](roh/SONDE-014-c303554.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,37 s | [B27](roh/SONDE-014-c303554.md#b27) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,12 s | [B28](roh/SONDE-014-c303554.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,28 s | [B8](roh/SONDE-014-c303554.md#b8) |


---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-06 22:58 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 56/56 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-522e2ab.md](roh/SONDE-014-522e2ab.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-06 22:58:49 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 522e2ab SONDE-014 Etappe E, Teil 3: sechs Rotbeweise und Manifest 7.5 |
| Commit (voll) | 522e2ab73d567c24205e035381534b3771635bd1 |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,09 s | [A1](roh/SONDE-014-522e2ab.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,58 s | [A2](roh/SONDE-014-522e2ab.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,73 s | [A3](roh/SONDE-014-522e2ab.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 119,83 s | [A4](roh/SONDE-014-522e2ab.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,53 s | [A4-SI](roh/SONDE-014-522e2ab.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 65,72 s | [A4b](roh/SONDE-014-522e2ab.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 2,88 s | [A5](roh/SONDE-014-522e2ab.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,14 s | [A6](roh/SONDE-014-522e2ab.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/SONDE-014-522e2ab.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [A8](roh/SONDE-014-522e2ab.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,32 s | [A9](roh/SONDE-014-522e2ab.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,86 s | [A10](roh/SONDE-014-522e2ab.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,24 s | [A11](roh/SONDE-014-522e2ab.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [A12](roh/SONDE-014-522e2ab.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,20 s | [A13](roh/SONDE-014-522e2ab.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 6,39 s | [A14](roh/SONDE-014-522e2ab.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,00 s | [A17](roh/SONDE-014-522e2ab.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 34,06 s | [A18](roh/SONDE-014-522e2ab.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [A19](roh/SONDE-014-522e2ab.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A20](roh/SONDE-014-522e2ab.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,41 s | [A21](roh/SONDE-014-522e2ab.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,02 s | [A22](roh/SONDE-014-522e2ab.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,68 s | [A24](roh/SONDE-014-522e2ab.md#a24) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,14 s | [A25](roh/SONDE-014-522e2ab.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,14 s | [A26](roh/SONDE-014-522e2ab.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,16 s | [A27](roh/SONDE-014-522e2ab.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,62 s | [A23](roh/SONDE-014-522e2ab.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,13 s | [A16](roh/SONDE-014-522e2ab.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,21 s | [B1](roh/SONDE-014-522e2ab.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,22 s | [B2](roh/SONDE-014-522e2ab.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/SONDE-014-522e2ab.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-014-522e2ab.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,39 s | [B3c](roh/SONDE-014-522e2ab.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,44 s | [B4](roh/SONDE-014-522e2ab.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B9](roh/SONDE-014-522e2ab.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,34 s | [B5](roh/SONDE-014-522e2ab.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 214,03 s | [B10](roh/SONDE-014-522e2ab.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,00 s | [B11](roh/SONDE-014-522e2ab.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,60 s | [B12](roh/SONDE-014-522e2ab.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/SONDE-014-522e2ab.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,19 s | [B14](roh/SONDE-014-522e2ab.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,22 s | [B15](roh/SONDE-014-522e2ab.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/SONDE-014-522e2ab.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,94 s | [B22](roh/SONDE-014-522e2ab.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,52 s | [B23](roh/SONDE-014-522e2ab.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 0,99 s | [B24](roh/SONDE-014-522e2ab.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/SONDE-014-522e2ab.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,47 s | [B26](roh/SONDE-014-522e2ab.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-522e2ab.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,93 s | [B19](roh/SONDE-014-522e2ab.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [B17](roh/SONDE-014-522e2ab.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 3,97 s | [B18](roh/SONDE-014-522e2ab.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,54 s | [B16](roh/SONDE-014-522e2ab.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,32 s | [B27](roh/SONDE-014-522e2ab.md#b27) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,12 s | [B28](roh/SONDE-014-522e2ab.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,28 s | [B8](roh/SONDE-014-522e2ab.md#b8) |


---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-06 23:53 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 56/56 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-a9659b1.md](roh/SONDE-014-a9659b1.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-06 23:53:43 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | a9659b1 SONDE-014 Etappe F, Teil 3: sechzehn Rotbeweise und Manifest 7.6 |
| Commit (voll) | a9659b135cb54b39ebccb7655f61dca91ba3f29e |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-014-a9659b1.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,61 s | [A2](roh/SONDE-014-a9659b1.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,77 s | [A3](roh/SONDE-014-a9659b1.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. Seit Etappe F dazu die Proposal-Policy: ein Vorschlag entsteht MIT seinem Befund und traegt die fuenfzehn Felder aus 42.1 plus revert, der Rueckweg ist ein FELD mit drei Werten (dsp_revert faellt), in P5 ist jede Aktion manual, keine Aenderung und mehr Daten sind vollstaendige Objekte, der Zielbereich kommt aus dem Band des Befunds statt aus dem groessten Banddelta, ein geschuetztes Band ist eine HARTE Constraint mit Gegenprobe, ein stop_if auf einem nicht messbaren Guardrail meldet MORE DATA, und ueber fuenfhundert zufaellige Eingaenge wird kein Hard Cap und keine engere Usergrenze ueberschritten. Der Guardrail-Rechner LIEST seit E-05 den Zielbereich aus experiment_begin.ziel; ohne ihn bleibt die Heuristik und das Resultat traegt ziel_geraten - beide Pfade mit verschiedener Zahl gemessen. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 156,18 s | [A4](roh/SONDE-014-a9659b1.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 13,19 s | [A4-SI](roh/SONDE-014-a9659b1.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 65,73 s | [A4b](roh/SONDE-014-a9659b1.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,82 s | [A5](roh/SONDE-014-a9659b1.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [A6](roh/SONDE-014-a9659b1.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [A7](roh/SONDE-014-a9659b1.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,35 s | [A8](roh/SONDE-014-a9659b1.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,39 s | [A9](roh/SONDE-014-a9659b1.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,24 s | [A10](roh/SONDE-014-a9659b1.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,36 s | [A11](roh/SONDE-014-a9659b1.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A12](roh/SONDE-014-a9659b1.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,24 s | [A13](roh/SONDE-014-a9659b1.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 5,86 s | [A14](roh/SONDE-014-a9659b1.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,10 s | [A17](roh/SONDE-014-a9659b1.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 33,77 s | [A18](roh/SONDE-014-a9659b1.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,15 s | [A19](roh/SONDE-014-a9659b1.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,32 s | [A20](roh/SONDE-014-a9659b1.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,35 s | [A21](roh/SONDE-014-a9659b1.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,02 s | [A22](roh/SONDE-014-a9659b1.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,74 s | [A24](roh/SONDE-014-a9659b1.md#a24) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,24 s | [A25](roh/SONDE-014-a9659b1.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,15 s | [A26](roh/SONDE-014-a9659b1.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/SONDE-014-a9659b1.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,63 s | [A23](roh/SONDE-014-a9659b1.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,13 s | [A16](roh/SONDE-014-a9659b1.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,15 s | [B1](roh/SONDE-014-a9659b1.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,23 s | [B2](roh/SONDE-014-a9659b1.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,06 s | [B3](roh/SONDE-014-a9659b1.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-014-a9659b1.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,37 s | [B3c](roh/SONDE-014-a9659b1.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,43 s | [B4](roh/SONDE-014-a9659b1.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B9](roh/SONDE-014-a9659b1.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,35 s | [B5](roh/SONDE-014-a9659b1.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 214,05 s | [B10](roh/SONDE-014-a9659b1.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,02 s | [B11](roh/SONDE-014-a9659b1.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,60 s | [B12](roh/SONDE-014-a9659b1.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/SONDE-014-a9659b1.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,22 s | [B14](roh/SONDE-014-a9659b1.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,23 s | [B15](roh/SONDE-014-a9659b1.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,07 s | [B20](roh/SONDE-014-a9659b1.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 1,00 s | [B22](roh/SONDE-014-a9659b1.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,53 s | [B23](roh/SONDE-014-a9659b1.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,00 s | [B24](roh/SONDE-014-a9659b1.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/SONDE-014-a9659b1.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,47 s | [B26](roh/SONDE-014-a9659b1.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-a9659b1.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,95 s | [B19](roh/SONDE-014-a9659b1.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [B17](roh/SONDE-014-a9659b1.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 4,14 s | [B18](roh/SONDE-014-a9659b1.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,56 s | [B16](roh/SONDE-014-a9659b1.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,38 s | [B27](roh/SONDE-014-a9659b1.md#b27) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,14 s | [B28](roh/SONDE-014-a9659b1.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,28 s | [B8](roh/SONDE-014-a9659b1.md#b8) |


---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-07 00:25 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 57/57 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-d1f62d3.md](roh/SONDE-014-d1f62d3.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-07 00:25:41 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | d1f62d3 SONDE-014 7.7: zwei Verweise ins Leere geschlossen |
| Commit (voll) | d1f62d391e689f9ce5c2109ded2ab7d3862d7e25 |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-014-d1f62d3.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,52 s | [A2](roh/SONDE-014-d1f62d3.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,76 s | [A3](roh/SONDE-014-d1f62d3.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. Seit Etappe F dazu die Proposal-Policy: ein Vorschlag entsteht MIT seinem Befund und traegt die fuenfzehn Felder aus 42.1 plus revert, der Rueckweg ist ein FELD mit drei Werten (dsp_revert faellt), in P5 ist jede Aktion manual, keine Aenderung und mehr Daten sind vollstaendige Objekte, der Zielbereich kommt aus dem Band des Befunds statt aus dem groessten Banddelta, ein geschuetztes Band ist eine HARTE Constraint mit Gegenprobe, ein stop_if auf einem nicht messbaren Guardrail meldet MORE DATA, und ueber fuenfhundert zufaellige Eingaenge wird kein Hard Cap und keine engere Usergrenze ueberschritten. Der Guardrail-Rechner LIEST seit E-05 den Zielbereich aus experiment_begin.ziel; ohne ihn bleibt die Heuristik und das Resultat traegt ziel_geraten - beide Pfade mit verschiedener Zahl gemessen. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 147,99 s | [A4](roh/SONDE-014-d1f62d3.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,04 s | [A4-SI](roh/SONDE-014-d1f62d3.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 65,73 s | [A4b](roh/SONDE-014-d1f62d3.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 3,00 s | [A5](roh/SONDE-014-d1f62d3.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [A6](roh/SONDE-014-d1f62d3.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/SONDE-014-d1f62d3.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [A8](roh/SONDE-014-d1f62d3.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [A9](roh/SONDE-014-d1f62d3.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,27 s | [A10](roh/SONDE-014-d1f62d3.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [A11](roh/SONDE-014-d1f62d3.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,35 s | [A12](roh/SONDE-014-d1f62d3.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,21 s | [A13](roh/SONDE-014-d1f62d3.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 5,93 s | [A14](roh/SONDE-014-d1f62d3.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,02 s | [A17](roh/SONDE-014-d1f62d3.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 33,86 s | [A18](roh/SONDE-014-d1f62d3.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [A19](roh/SONDE-014-d1f62d3.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,39 s | [A20](roh/SONDE-014-d1f62d3.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,33 s | [A21](roh/SONDE-014-d1f62d3.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 9,95 s | [A22](roh/SONDE-014-d1f62d3.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,52 s | [A24](roh/SONDE-014-d1f62d3.md#a24) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,14 s | [A25](roh/SONDE-014-d1f62d3.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,13 s | [A26](roh/SONDE-014-d1f62d3.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/SONDE-014-d1f62d3.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,71 s | [A23](roh/SONDE-014-d1f62d3.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,17 s | [A16](roh/SONDE-014-d1f62d3.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,12 s | [B1](roh/SONDE-014-d1f62d3.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,18 s | [B2](roh/SONDE-014-d1f62d3.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [B3](roh/SONDE-014-d1f62d3.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-014-d1f62d3.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,37 s | [B3c](roh/SONDE-014-d1f62d3.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,46 s | [B4](roh/SONDE-014-d1f62d3.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B9](roh/SONDE-014-d1f62d3.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,33 s | [B5](roh/SONDE-014-d1f62d3.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 214,27 s | [B10](roh/SONDE-014-d1f62d3.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 0,98 s | [B11](roh/SONDE-014-d1f62d3.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,60 s | [B12](roh/SONDE-014-d1f62d3.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/SONDE-014-d1f62d3.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,20 s | [B14](roh/SONDE-014-d1f62d3.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,23 s | [B15](roh/SONDE-014-d1f62d3.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/SONDE-014-d1f62d3.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,89 s | [B22](roh/SONDE-014-d1f62d3.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,52 s | [B23](roh/SONDE-014-d1f62d3.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,03 s | [B24](roh/SONDE-014-d1f62d3.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,06 s | [B25](roh/SONDE-014-d1f62d3.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,48 s | [B26](roh/SONDE-014-d1f62d3.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-d1f62d3.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,86 s | [B19](roh/SONDE-014-d1f62d3.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [B17](roh/SONDE-014-d1f62d3.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 4,03 s | [B18](roh/SONDE-014-d1f62d3.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,51 s | [B16](roh/SONDE-014-d1f62d3.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,37 s | [B27](roh/SONDE-014-d1f62d3.md#b27) |
| B29 | Die Zustandsmaschine des AssistantStep im Main (46.1, E-07/E-08, M-55 bis M-62). Die Zustandsmenge hat ACHT Werte, jeder kommt ueber seinen eigenen Rueckweg zurueck, ein neunter faellt. Nach preview fuehrt KEINE Kante - weder hin noch zurueck -, ein gespeicherter preview-Schritt ist ein Lesefehler statt eines stillen Sprungs, und die Produkt-API setzt ihn nicht; die Gegenprobe mit proposal laedt normal. Jeder der acht Zustaende traegt fuenf Angaben (Eintritt, Evidenz, Useraktion, Timeout ueber 0, sichere Rueckkante), auch preview - der erste Zustand ist seine eigene Rueckkante. Der Deckel ist STRUKTURELL: ein zweiter Startversuch bei offenem Schritt wird abgewiesen, nicht eingereiht, und erst nach dem terminalen Abbruch beginnt ein neuer. Die vier Gegenpfade laufen einzeln: Zurueck geht auf die Rueckkante, Ueberspringen auf den naechsten Zustand, Resume ist eine FRAGE ohne Revisionssprung, und Verwerfen ist terminal statt geloescht - danach gibt es kein Resume mehr. Die drei benannten Ergebnisse aus 46.2 sind eigene Ergebnisse mit Objekt; dasselbe zweimal hebt keine Revision. Die harten Gates greifen VOR der Gewichtung: drei Kandidaten mit perfektem Rang und je einem gerissenen Gate verlassen die Liste, ein bescheidener bleibt. Verdrahtet am echten Prozessor mit Host-Dirty genau einmal je echter Aenderung, Rekonstruktion aus dem gespeicherten MainProject und bytegleichem Save/Load; jede Grenze faellt von beiden Seiten, und ein Altstand ohne die Eigenschaft laedt normal und schreibt sie nicht. | `eq-copilot\build\plugin\EqCopSonde014AssistentTest_artefacts\Release\EqCopSonde014AssistentTest.exe` | [OK] Exit 0 | 0,13 s | [B29](roh/SONDE-014-d1f62d3.md#b29) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,13 s | [B28](roh/SONDE-014-d1f62d3.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,29 s | [B8](roh/SONDE-014-d1f62d3.md#b8) |


---

## Kanon-Lauf - SONDE-014

**Lauf:** 2026-09-07 01:16 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 60/60 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-014-0e853f9.md](roh/SONDE-014-0e853f9.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-07 01:16:58 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 0e853f9 SONDE-014 Etappe H, Teil 2: elf Rotbeweise und Manifest 7.8 |
| Commit (voll) | 0e853f905ab1998761abd168b3b9db7f304e2d61 |
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
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-014-0e853f9.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,57 s | [A2](roh/SONDE-014-0e853f9.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 7,78 s | [A3](roh/SONDE-014-0e853f9.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. Seit SONDE-013 zusaetzlich die Fassungsleiter (jede Minorfassung wird aus der committeten zurueckgebaut, der Leser der Fassung 1 lehnt jede Neuerung der Fassung 2 ab, Fassung 0 erbt den Rueckbau) und der Empfaenger des evidence_snapshot: fremde Adresse verworfen, offene Intervention sperrt statt abzuschwaechen, nach Ende und Nachlauf nimmt er wieder an. Seit SONDE-014 zusaetzlich der Intent- und Assistentenspiegel (Koaleszierung je Quelle/Scope, keine Rechnung vor der Vollstaendigkeitsmarke) und der URSACHENPFAD: aus paralleler Telemetrie entsteht nie Aussageklasse 2 oder 3, das Screening reicht hoechstens fuenf Kandidaten weiter, fehlende Coverage und falsches Alignment sind ein GATE vor der Gewichtung und tragen einen Grund aus der geschlossenen Achtermenge, eine Passage unter GATE_MINDEST_FENSTER traegt keine starke Aussage (mit Gegenprobe an der Kante 7/8), eine Ruecknahme invalidiert die abhaengigen Hypothesen deterministisch, und hundert Laeufe ueber dieselben Bytes liefern bytegleich dieselbe Rangfolge. Seit Etappe F dazu die Proposal-Policy: ein Vorschlag entsteht MIT seinem Befund und traegt die fuenfzehn Felder aus 42.1 plus revert, der Rueckweg ist ein FELD mit drei Werten (dsp_revert faellt), in P5 ist jede Aktion manual, keine Aenderung und mehr Daten sind vollstaendige Objekte, der Zielbereich kommt aus dem Band des Befunds statt aus dem groessten Banddelta, ein geschuetztes Band ist eine HARTE Constraint mit Gegenprobe, ein stop_if auf einem nicht messbaren Guardrail meldet MORE DATA, und ueber fuenfhundert zufaellige Eingaenge wird kein Hard Cap und keine engere Usergrenze ueberschritten. Der Guardrail-Rechner LIEST seit E-05 den Zielbereich aus experiment_begin.ziel; ohne ihn bleibt die Heuristik und das Resultat traegt ziel_geraten - beide Pfade mit verschiedener Zahl gemessen. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 160,84 s | [A4](roh/SONDE-014-0e853f9.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 23,68 s | [A4-SI](roh/SONDE-014-0e853f9.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 66,09 s | [A4b](roh/SONDE-014-0e853f9.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 2,95 s | [A5](roh/SONDE-014-0e853f9.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,17 s | [A6](roh/SONDE-014-0e853f9.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,17 s | [A7](roh/SONDE-014-0e853f9.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,36 s | [A8](roh/SONDE-014-0e853f9.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,33 s | [A9](roh/SONDE-014-0e853f9.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 2,46 s | [A10](roh/SONDE-014-0e853f9.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,27 s | [A11](roh/SONDE-014-0e853f9.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,27 s | [A12](roh/SONDE-014-0e853f9.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,23 s | [A13](roh/SONDE-014-0e853f9.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 6,34 s | [A14](roh/SONDE-014-0e853f9.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,13 s | [A17](roh/SONDE-014-0e853f9.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 35,05 s | [A18](roh/SONDE-014-0e853f9.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [A19](roh/SONDE-014-0e853f9.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A20](roh/SONDE-014-0e853f9.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,44 s | [A21](roh/SONDE-014-0e853f9.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,01 s | [A22](roh/SONDE-014-0e853f9.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. Seit SONDE-013 laeuft dabei der Evidenzpfad mit: jede Sonde baut oberhalb einer kleinen P1-Fuellstandsschwelle GAR KEINEN evidence_snapshot mehr und zaehlt den verworfenen Analyseframe, statt die in EINTRAEGEN gedeckelte P1-Queue mit 10-KiB-Nachrichten zu fuellen. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,67 s | [A24](roh/SONDE-014-0e853f9.md#a24) |
| A28 | Der P5-Evaluationskorpus ist reproduzierbar (Muster A25): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt SHA-256 im Manifest, und eine verwaiste Datei faellt. Zusaetzlich der Hygieneriegel aus NAK-182 R2: der Bezeichner des Kettenbeins steht WOERTLICH in seiner Datei, sonst waere der Name ein Etikett. | `py -3.13 tools\eq-copilot\erzeuge_p5_korpus.py --pruefen` | [OK] Exit 0 | 0,14 s | [A28](roh/SONDE-014-0e853f9.md#a28) |
| A29 | Das Sammelbein des P5-Exit-Gates (59 Punkt 6, 36.4, M-64 bis M-70, R2). Es ist das dritte Glied einer KETTE: der Korpus traegt die Wahrheit, broker/tests/sonde014_p5_korpus.rs faehrt jede Sitzung durch p1 und schreibt die TATSAECHLICH ausgegebene Hypothese, und dieses Bein haelt beides gegeneinander - eine falsche starke Produktbehauptung aendert den Korpus nicht, sie faellt am Vergleich. Precision und Recall stehen JE URSACHENKLASSE, dazu Brier, Kalibrierung, Coverage und Enthaltungsrate; die vier Riegel (falsche_starke, falsche_schwache, precision und recall in [0,1]) laufen je Klasse und ueber die Gesamtmenge. Die Schwelle aus M-31 ist AUSGABE: gesucht wird die niedrigste Sicherheitsstufe, deren Riegel halten. Der Startwert von GATE_MINDEST_FENSTER wird an den zwei Passagensitzungen kalibriert. Ohne frische Ergebnisdatei meldet das Bein Voraussetzung-fehlt (Exit 3) statt gruen. | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py` | [OK] Exit 0 | 0,15 s | [A29](roh/SONDE-014-0e853f9.md#a29) |
| A30 | Der Selbsttest des P5-Sammelbeins (M-68, Muster A27). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch DIESELBEN Funktionen, die das Sammelbein fuehrt. Jede Erwartung laeuft mit ihrem GEGENTEIL - eine falsche starke Behauptung und die richtige daneben, eine Enthaltung und dieselbe Menge ohne sie, ein leerer Satz und ein voller. Die vier Riegel werden synthetisch gefuettert und muessen fallen: precision > 1, recall > 1 und brier > 1 sind ueber den Korpusweg strukturell unerreichbar, und eine Wache ohne ausfuehrbaren Negativtest ist keine gemessene Zusage. Zuletzt die Schwellensuche in beide Richtungen. | `py -3.13 tools\eq-copilot\pruefe_p5_korpus.py --selbsttest` | [OK] Exit 0 | 0,12 s | [A30](roh/SONDE-014-0e853f9.md#a30) |
| A25 | Der P4-Referenzkorpus ist reproduzierbar (M-80): der Erzeuger baut jede Datei BYTEGLEICH neu, --pruefen vergleicht den committeten Bestand gegen die Neuerzeugung samt MANIFEST und meldet jede verwaiste Datei. Dasselbe Muster wie A6, A7, A8, A10, A12 und A20; die gitattributes-Regel haelt die Zeilenenden. Seit NAK-182 weist der Erzeuger ausserdem ab, was der Korpus nicht tragen darf: eine Wahrheit ausserhalb der geschlossenen Menge, eine fehlende `quelle` und einen Bezeichner, der in der genannten Datei nicht woertlich steht. Die zwei Mengen KEINE_BEHAUPTUNG und KEINE_STARKE sowie die Zeile `nicht_gemessen` reisen im MANIFEST mit, damit der Pruefer sie von dort liest statt aus einer zweiten Kopie. | `py -3.13 tools\eq-copilot\erzeuge_p4_korpus.py --pruefen` | [OK] Exit 0 | 0,22 s | [A25](roh/SONDE-014-0e853f9.md#a25) |
| A26 | Das Sammelbein des P4-Exit-Gates (§58, §49.4, M-79/M-81 bis M-85): vier Klassen - Referenzkorpus, Loop-/Seek-/PDC-Goldens, adversariale Vergleichsfixtures und Stereo-Goldens -, alle muessen bestehen, und jedes genannte Bein steht wirklich im Kanon-Runner. Neben Precision und Recall zaehlen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl FALSCHER Behauptungen; die muss null sein. Eine Enthaltung ist dabei kein Fehler - §49.4 woertlich: ein konservatives unsicher ist besser als eine ueberzeugende falsche Ursache. Seit NAK-182 liest das Bein zwei geschlossene Wahrheitsmengen AUS DEM MANIFEST: auf KEINE_BEHAUPTUNG (unbekannt, unvergleichbar, zeitvariabel) ist jede Aussage falsch, auch die schwache; auf den zwei Werten, die nur in KEINE_STARKE stehen (nicht_kausal, nicht_exakt), ist es die starke. Zaehler und Nenner von Recall lesen dieselbe Menge, Precision und Recall sind als Quoten geriegelt, jeder Korpusfall nennt eine Quelldatei samt woertlichem Bezeichner und wird dort gesucht, die fuenf geschlossenen Mengen des v3-Vertrags werden auf kausale Bezeichner geprueft (Entscheid G4 §8), und die in P4 NICHT gemessene Zusage M-85 Ziel 1 wird im Kopf gedruckt statt still zu fehlen. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py` | [OK] Exit 0 | 0,13 s | [A26](roh/SONDE-014-0e853f9.md#a26) |
| A27 | Der Selbsttest des Exit-Gate-Beins (NAK-182 R1, MP1-6). Er laeuft OHNE Repo-Fixture: die Faelle entstehen im Speicher und laufen durch dieselben Funktionen, die A26 fuehrt. Auf dem Korpusweg fallen eine STARKE Aussage auf jedem Wert aus KEINE_STARKE und eine SCHWACHE auf jedem Wert aus KEINE_BEHAUPTUNG; eine schwache Aussage auf nicht_kausal oder nicht_exakt bleibt gruen - genau das trennt die zwei Mengen. Auf dem Kennzahlweg bekommt die Riegelfunktion synthetische Kennzahlen direkt eingespeist, weil Recall und Precision ueber 1 nach der Zweimengenlogik strukturell unerreichbar sind und die Wachen trotzdem einen Weg zu fallen brauchen. Dazu ein erfundener Bezeichner, ein Pfad ins Leere, eine fehlende Vertragsmenge und ein kausaler Enumwert. Seit Nacharbeit 1 laeuft auch S-07 durch die echte Validierung: ein synthetischer Fall mit einer Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN geht durch dieselbe Funktion _pruefe_wahrheiten, die A26 auf den geladenen Korpus anwendet, und muss genau eine rote Zeile mit dem Fallnamen ergeben; sein Gegenteil keine. Jede Erwartung laeuft MIT ihrem Gegenteil - sonst waere der Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete. | `py -3.13 tools\eq-copilot\pruefe_p4_korpus.py --selbsttest` | [OK] Exit 0 | 0,13 s | [A27](roh/SONDE-014-0e853f9.md#a27) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,63 s | [A23](roh/SONDE-014-0e853f9.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/SONDE-014-0e853f9.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,12 s | [B1](roh/SONDE-014-0e853f9.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,18 s | [B2](roh/SONDE-014-0e853f9.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/SONDE-014-0e853f9.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-014-0e853f9.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,37 s | [B3c](roh/SONDE-014-0e853f9.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten UND ueber 2000 weitere mit wechselnder Hoer-Markierung, Ein- und Ausfade und Interventionsring (SONDE-013 M-74), und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,50 s | [B4](roh/SONDE-014-0e853f9.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/SONDE-014-0e853f9.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen -, waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, und ein Drop zaehlt als Segment, nicht als Epoche. Seit NAK-182 misst ein Sweep zusaetzlich die ZEITLAGE: ein bekannter Impuls bei Stromsample 206336 laeuft ueber die fuenf Blockgroessen 1, 333, 512, 2048 und 16384 mal die fuenf Sampleraten 44,1 / 48 / 88,2 / 96 und 192 kHz, und der gemeldete stromSample ist je Samplerate ueber alle fuenf Blockgroessen IDENTISCH und liegt innerhalb einer Fensterlaenge (4096 Samples) vor dem Impuls. Die Auswahl des Ereignisses kennt die Sollzeit NICHT - gemessen wird das staerkste Flussereignis des Laufs, und dass es unzweifelhaft das staerkste ist, ist eine eigene Zusage. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 5,32 s | [B5](roh/SONDE-014-0e853f9.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 214,68 s | [B10](roh/SONDE-014-0e853f9.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 1,00 s | [B11](roh/SONDE-014-0e853f9.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,58 s | [B12](roh/SONDE-014-0e853f9.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,17 s | [B13](roh/SONDE-014-0e853f9.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,21 s | [B14](roh/SONDE-014-0e853f9.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,19 s | [B15](roh/SONDE-014-0e853f9.md#b15) |
| B20 | Gesamtklasse nach §34.3 (M-06): die vier Klassen sind geordnet, und `deckeln` ist ein Minimum, kein Mittelwert. Jeder der vier harten Maengel deckelt EINZELN auf schwach, ohne gegen die drei guten Nachbarquellen verrechnet zu werden; zwei Maengel ergeben in allen zwoelf Paarungen unbrauchbar. Was ein Erzeuger nicht beurteilen kann, deckelt ebenso auf mittel - eine Sonde mit PERFEKTER Abdeckung kommt deshalb nur auf mittel, weil sie Session, Passage und Alignment nicht sieht; das ist die Regel hinter dem Literal, das Etappe B an einer Stelle gesetzt hatte. Die zwei Abdeckungsschwellen und die Fensterschwelle fallen je an ihrer Kante und knapp darunter (Muster C-09). Die tragende Zusage ist eine INVARIANTE ueber den ganzen Raum: in 1536 Uebergaengen ueber alle 256 Bitkombinationen in drei Basislagen hebt ein zusaetzlicher harter Mangel die Klasse NIE - und alle vier Klassen kommen darin wirklich vor, die Invariante ist also nicht trivial erfuellt. Ein Mittelwert koennte diese Zusage nicht halten. | `eq-copilot\build\plugin\EqCopSonde013QualityClassTest_artefacts\Release\EqCopSonde013QualityClassTest.exe` | [OK] Exit 0 | 0,06 s | [B20](roh/SONDE-014-0e853f9.md#b20) |
| B22 | Content-Fingerprint einer Passage (§32.4, M-26/M-27/M-31): er entsteht erst ab genug Material und traegt die Zahl seiner Fenster mit; dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint, derselbe Akkord in zwei Lautstaerken bleibt ueber 0,95 aehnlich (jeder Verlauf ist auf sein eigenes Maximum normiert), ein anderer Akkord ist messbar unaehnlicher und Rauschen deutlich. Die adversariale Rueckrechenprobe zeigt nicht, dass eine Rueckrechnung schwer waere, sondern dass die Information nicht da ist: 76 Byte fuer 204800 Samples, und ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, und alle drei Verlaeufe sind Energien ohne Phase. Das Fingerprintfenster ueberbrueckt keine Epochengrenze: nach einer Stromluecke traegt die Engine den Fingerprint des NEUEN Materials (Aehnlichkeit 1,00 zur neuen Referenz, 0,00 zur alten), und der Fensterzaehler faellt mit - bei einer Ueberbrueckung waeren es doppelt so viele. Die Aehnlichkeit selbst ist ein MINIMUM ueber die drei Verlaeufe und kein Mittelwert; zwei Fingerprints ohne Bit sind nicht aehnlich, sondern gar nichts. | `eq-copilot\build\plugin\EqCopSonde013FingerprintGoldenTest_artefacts\Release\EqCopSonde013FingerprintGoldenTest.exe` | [OK] Exit 0 | 0,90 s | [B22](roh/SONDE-014-0e853f9.md#b22) |
| B23 | Die manuelle Passage als Projektintent im MainProjectState (§33.5, M-25/M-69). Das neue persistente Feld `manual_passages_v1` traegt seine Fassung im Namen; ein Altstand ohne das Feld laedt normal, ein unbekanntes Zusatzfeld aus einer spaeteren Fassung zerstoert den Leser nicht und steht unveraendert im naechsten Save, und Save/Load ist ueber zwei Runden bytegleich - auch am oberen int64-Rand und mit leerer Liste (die Eigenschaft entfaellt dann, statt als leeres Array zu reisen). Jede persistente Aenderung meldet GENAU einmal Host-Dirty, das Vergessen wie das Merken; ein read-only-Stand verweigert beides und gibt seine Originalbytes zurueck. Was hier NICHT reist, ist Absicht: Fingerprint, Quellenset, Abdeckung und Epoche sind Messergebnisse und bleiben im Store, sonst behauptete ein Projekt nach dem Loeschen der Datenbank weiter, es gebe Evidenz. Jede Grenze wird von beiden Seiten gefahren - was die Produkt-API ablehnt, lehnt auch der Leser ab, und zwar fail-closed als read-only statt still korrigiert. | `eq-copilot\build\plugin\EqCopSonde013PassageStateTest_artefacts\Release\EqCopSonde013PassageStateTest.exe` | [OK] Exit 0 | 11,52 s | [B23](roh/SONDE-014-0e853f9.md#b23) |
| B24 | PRE/POST auf der Audioseite (§38.3/§38.4, M-18/M-20/M-24). Die Magnitude-Squared Coherence ist 1 fuer JEDE lineare zeitinvariante Kette und faellt genau dann, wenn die Kette nichtlinear oder zeitvariabel ist: identische Kette, reiner Gain von +15 dB und ein linearer Tiefpass bleiben fast ueberall ueber der 0,8-Schwelle aus §38.3 - Kompression, Modulation, Saturation und wechselnde Latenz fallen darunter, und die beiden Gruppen ueberlappen nicht. Deshalb traegt keine der vier adversarialen Ketten eine statische EQ-Behauptung. Der Vergleichspegel wird vorab gemessen und eingefroren: sechs Sekunden mit +18 dB und zwei Sekunden Stille aendern ihn um kein Tausendstel, denn eine mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor; ohne genug Material friert er gar nicht erst ein, und NaN/Inf verriegeln statt ihn zu vergiften. Hoerbares Delta ist ohne nachgewiesenes Compare-Routing gesperrt - der Raum aus drei binaeren Nachweisen wird VOLLSTAENDIG abgefahren, genau eine der acht Kombinationen ist frei, und in P4 gibt es kein Compare-Routing. | `eq-copilot\build\plugin\EqCopSonde013PrePostGoldenTest_artefacts\Release\EqCopSonde013PrePostGoldenTest.exe` | [OK] Exit 0 | 1,08 s | [B24](roh/SONDE-014-0e853f9.md#b24) |
| B25 | Unsicherheit und Mehrfachtestung (§43.2, M-45). Der Block-Bootstrap ist deterministisch, enthaelt den Mittelwert und liefert bei bloeckigem Material ein BREITERES Intervall als ein Bootstrap ueber Einzelwerte - benachbarte Deltas sind korreliert, und wer Unabhaengigkeit annimmt, macht aus Rauschen eine belastbare Aenderung. Leere oder unmoegliche Eingaben ergeben KEIN Intervall statt eines um null. Bei 221 gleichverteilten p-Werten meldet der naive Scan zweistellig viele Baender und die FDR-Korrektur kein einziges, waehrend 20 klare Treffer durchkommen; die Grenze p_(k) = k/m*q faellt inklusiv an ihrem Wert. Cluster verlangen zusammenhaengende Baender - drei verstreute ergeben keinen, vier benachbarte schon, und auch am linken und rechten Rand. | `eq-copilot\build\plugin\EqCopSonde013ExperimentGoldenTest_artefacts\Release\EqCopSonde013ExperimentGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B25](roh/SONDE-014-0e853f9.md#b25) |
| B26 | Die zwei Kanten des Blindvergleichs (§43.1, §15, M-43/M-44). Ohne eingefrorenen Vergleichspegel wird KEIN Klangurteil angenommen - lauter klingt besser, und ein A/B ohne Pegelabgleich misst genau das; ein noch laufender Pegel zaehlt nicht als Abgleich. Die Blindreihenfolge wird vor dem Urteil gebunden und laesst sich nicht drehen; vor dem Urteil gibt der Typ sie GAR NICHT heraus, und der Ausgabeparameter bleibt unberuehrt statt still einen lesbaren Wert zu tragen. Erst das Urteil deckt genau die gebundene Reihenfolge auf, in beide Richtungen geprueft. Ein zweites Urteil und ein spaeterer Pegel aendern nichts mehr; jede Sperre traegt ein Wort, und der Passagenwechsel raeumt Urteil, Abgleich und Bindung gemeinsam ab. | `eq-copilot\build\plugin\EqCopSonde013ExperimentUiTest_artefacts\Release\EqCopSonde013ExperimentUiTest.exe` | [OK] Exit 0 | 0,47 s | [B26](roh/SONDE-014-0e853f9.md#b26) |
| B21 | RT→Control-Ring fuer hoerbare Eingriffe (M-37 bis M-39, §34.2): ein Begin steht SOFORT zur Abholung bereit, ohne Kadenzfenster - der Unterschied zum 1-Hz-Heartbeat-Bit, das ein 380-Sample-Ereignispaar nie gesehen haette. Ein Ende traegt einen Nachlauf, der mindestens so lang ist wie der Eingriff selbst, UND dessen Dauer, damit ein Empfaenger die Konservativitaet pruefen statt sie glauben kann. Der Ueberlauf ist die schaerfste Zusage: dieser Ring verwendet KEIN drop-oldest, das aelteste Ereignis steht unveraendert an erster Stelle, der Ueberlauf ist sticky gemeldet und gezaehlt, der Rest kommt lueckenlos aufsteigend heraus, und das Sticky-Bit heilt nicht von selbst - nur resync() loescht es. 200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen, mit Zaehler gemessen statt behauptet (M-74). Sequenzen am u64-Rand kommen unveraendert zurueck, und zuruecksetzen() leert Ring, Sticky-Bit und Zaehler gemeinsam. | `eq-copilot\build\plugin\EqCopSonde013InterventionRingTest_artefacts\Release\EqCopSonde013InterventionRingTest.exe` | [OK] Exit 0 | 0,07 s | [B21](roh/SONDE-014-0e853f9.md#b21) |
| B19 | Bandweise Stereoevidenz (§40.1, §40.3, M-08/M-10/M-11/M-12): die fuenf Klassen aus §40.3 treffen ihre ANALYTISCH bekannte Antwort - Mono ergibt Korrelation 1, Kohaerenz 1, Phase 0 und Folddown 0 dB innerhalb der 0,25 dB aus §40.3; Polaritaetsinvertierung ergibt Korrelation -1 bei Kohaerenz 1 (die Kohaerenz misst den Zusammenhang, nicht das Vorzeichen) und eine Monosumme, die an die Vertragsgrenze laeuft statt zu schweigen; eine bekannte Laufzeit ergibt eine Phase, die der Formel +2*pi*f*tau folgt, an drei Traegerfrequenzen gemessen; unkorrelierte Kanaele fallen in Korrelation und Kohaerenz, und bei niedriger Kohaerenz entsteht keine Lag- oder Polaritaetsempfehlung. Seit NAK-182 faehrt ein Sweep dieselben fuenf Klassen und den Folddown ueber die 18 Blockgroessen 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512, 1024, 2048, 4096, 8192, 12345 und 16384 - je Blockgroesse dieselbe Antwort, je Blockgroesse ein wirklich entstandenes Evidenzfenster und vergleichbar viele gemittelte Fenster (Freiheitsgrade innerhalb 20 Prozent des Medians). Das ist eine AUSWAHL aus 1 bis 16384 und keine Erschoepfung: die Obergrenze ist die Slotkapazitaet des Layouts, 256 zum Beispiel bleibt ungemessen. Seit Nacharbeit 1 wertet der Sweep bei bekannter Laufzeit ALLE DREI Traeger aus - 300, 900 und 2000 Hz, je Blockgroesse mit Praesenzbit und Phasenfehler gegen +2*pi*f*tau innerhalb 0,25 rad -, und der Unkorreliert-Fall misst je Blockgroesse zusaetzlich, dass KEIN Band mit Basis eine Phase traegt; das sind dieselben Felder, die der 512er-Abschnitt fuer M-12 nutzt. | `eq-copilot\build\plugin\EqCopSonde013StereoGoldenTest_artefacts\Release\EqCopSonde013StereoGoldenTest.exe` | [OK] Exit 0 | 1,85 s | [B19](roh/SONDE-014-0e853f9.md#b19) |
| B17 | True Peak nach ITU-R BS.1770-5 (M-02): der 8-fach-Polyphaseninterpolator trifft die fuenf True-Peak-Testfaelle aus EBU Tech 3341 (15 bis 19) innerhalb der SCHAERFEREN Toleranz aus §49.3, also +/-0,1 dB statt der +0,2/-0,4 der Norm; gemessen wird gegen die ANALYTISCHE Referenz - die Signale sind Sinus mit definierter Frequenz, Amplitude und Phase, und der wahre Scheitel eines Sinus IST seine Amplitude, also steht keine zweite eigene Implementierung als Vergleich dahinter. Der Gegenbeleg zeigt, dass ein Detektor ohne Ueberabtastung bei Testfall 16 um 3,01 dB zu optimistisch waere. Der Sampleraten-Sweep ueber 44,1/48/88,2/96/192 kHz haelt dieselbe Zusage, weil die EBU-Frequenzen an fs gebunden sind. Die 8-fach-Entscheidung wird als ZAHL getroffen: die geschlossene Fehlerschranke der Ueberabtastung haelt bei fs/4 mit Faktor 8 (-0,042 dB) und reisst mit Faktor 4 (-0,169 dB). Raender: Stille ergibt exakt 0, ein NaN bleibt nicht im Filterzustand, ein Gleichanteil bleibt bitgenau er selbst (jede der acht Phasen hat DC-Verstaerkung 1, nicht nur die Gesamtsumme 8), und ohne vorbereiten() misst der Detektor gar nicht, statt zu raten. | `eq-copilot\build\plugin\EqCopSonde013TruePeakGoldenTest_artefacts\Release\EqCopSonde013TruePeakGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [B17](roh/SONDE-014-0e853f9.md#b17) |
| B18 | Loudnessfenster, Headroom und Dynamik (M-01, M-03, M-04, M-07, M-09): Momentary (400 ms) und Short-term (3 s) sind wirklich zwei Fenster - bei stehendem Pegel gleich, 0,8 s nach einem Pegelsprung mehr als 5 LU auseinander, danach wieder beieinander; nach 0,6 s traegt der Frame Momentary und KEIN Short-term. integration_samples passt im Dauerbetrieb zur 10-Hz-Kadenz und faellt beim ersten Rahmen nach einer Grenze messbar kleiner aus. PSR rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters, nicht gegen den Sample-Peak des 100-ms-Rahmens; die Engine setzt selbst kein PLR, weil LUFS-I erst im Sondenprozessor zugemischt wird. Headroom ist eine Verteilung: zwei Signale mit gleichem Maximum trennen sich um mehr als 10 dB im P50. Crest steht in zwei Fenstern und trennt dichte kleine Spitzen von einer einzelnen grossen. LRA gibt es nach 30 s NICHT, nach 75 s mit passender Spanne, bei Material ohne Dynamik nahe null und nach 90 s Stille gar nicht - die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr. NaN und Inf im Eingang erzeugen in keiner der neun neuen Metrikstellen einen nichtendlichen Wert. Ein verworfener Analyseblock schliesst auch die neuen Fenster, und keines von ihnen ueberbrueckt eine Transportgrenze. | `eq-copilot\build\plugin\EqCopSonde013DynamicsTest_artefacts\Release\EqCopSonde013DynamicsTest.exe` | [OK] Exit 0 | 4,05 s | [B18](roh/SONDE-014-0e853f9.md#b18) |
| B16 | Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; P10/P50/P95 trennen ein springendes von einem ruhigen Signal gleicher mittlerer Lautheit (Spanne P95-P10 um mehr als 6 dB verschieden) und halten in JEDEM Band mit Bit die Ordnung P10<=P50<=P95; die Abdeckung faellt messbar, sobald ganze Analysefenster in Stille liegen, und die Konvergenz faellt bei wanderndem Pegel; der Ereignisstrom traegt Fluss- und Peakbit getrennt samt Verlustzaehler, ein Ereignis fremder Epoche oder ohne Anker entsteht gar nicht. Seit NAK-182 misst dasselbe Bein die C++-Haelfte der fokussierten 0,01-dB-Evidenz (M-83 Satz 2), seit Nacharbeit 1 vollstaendig am SERIALISIERTEN Snapshot: PRE und POST laufen durch den echten Serialisierer, und jede Zahl kommt aus verteilung.p50 des eigenen Wire-Textes - Encoding-Wort, Ganzzahlen und Praesenzbits. Daraus kommt ein bekannter Gain von 3,00 dB je Band mit Bit innerhalb 0,1 dB zurueck, und eine Leiter aus 20 Pegeln im Abstand 0,01 dB - je ein eigener Enginelauf samt Serialisierung - ergibt 20 streng monotone Ganzzahlen, deren Nachbarn sich um genau 1 unterscheiden. Material (Frequenz, Samplerate, Amplitude, Blockgroesse, Laufbegrenzung), Pegel, Gain und Aufloesung kommen aus den ZAHLEN der Fixture evidenz-0p01-paar-wire-v1.json, nicht aus ihren Bytes und nicht aus lokalen Konstanten. | `eq-copilot\build\plugin\EqCopSonde013EventWireTest_artefacts\Release\EqCopSonde013EventWireTest.exe` | [OK] Exit 0 | 0,51 s | [B16](roh/SONDE-014-0e853f9.md#b16) |
| B27 | Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. | `eq-copilot\build\plugin\EqCopSonde014IntentTest_artefacts\Release\EqCopSonde014IntentTest.exe` | [OK] Exit 0 | 0,42 s | [B27](roh/SONDE-014-0e853f9.md#b27) |
| B29 | Die Zustandsmaschine des AssistantStep im Main (46.1, E-07/E-08, M-55 bis M-62). Die Zustandsmenge hat ACHT Werte, jeder kommt ueber seinen eigenen Rueckweg zurueck, ein neunter faellt. Nach preview fuehrt KEINE Kante - weder hin noch zurueck -, ein gespeicherter preview-Schritt ist ein Lesefehler statt eines stillen Sprungs, und die Produkt-API setzt ihn nicht; die Gegenprobe mit proposal laedt normal. Jeder der acht Zustaende traegt fuenf Angaben (Eintritt, Evidenz, Useraktion, Timeout ueber 0, sichere Rueckkante), auch preview - der erste Zustand ist seine eigene Rueckkante. Der Deckel ist STRUKTURELL: ein zweiter Startversuch bei offenem Schritt wird abgewiesen, nicht eingereiht, und erst nach dem terminalen Abbruch beginnt ein neuer. Die vier Gegenpfade laufen einzeln: Zurueck geht auf die Rueckkante, Ueberspringen auf den naechsten Zustand, Resume ist eine FRAGE ohne Revisionssprung, und Verwerfen ist terminal statt geloescht - danach gibt es kein Resume mehr. Die drei benannten Ergebnisse aus 46.2 sind eigene Ergebnisse mit Objekt; dasselbe zweimal hebt keine Revision. Die harten Gates greifen VOR der Gewichtung: drei Kandidaten mit perfektem Rang und je einem gerissenen Gate verlassen die Liste, ein bescheidener bleibt. Verdrahtet am echten Prozessor mit Host-Dirty genau einmal je echter Aenderung, Rekonstruktion aus dem gespeicherten MainProject und bytegleichem Save/Load; jede Grenze faellt von beiden Seiten, und ein Altstand ohne die Eigenschaft laedt normal und schreibt sie nicht. | `eq-copilot\build\plugin\EqCopSonde014AssistentTest_artefacts\Release\EqCopSonde014AssistentTest.exe` | [OK] Exit 0 | 0,17 s | [B29](roh/SONDE-014-0e853f9.md#b29) |
| B28 | Die Befundzustaende auf der Gen-Seite (Abnahme U21, M-29 bis M-35). Die drei Sicherheitsstufen sind auf GENAU DREI Zustaende abgebildet, und die Abbildung ist ein FELD: ein vierter Wert macht den ganzen Snapshot ungueltig, statt still auf einen bekannten zu fallen. Nur READY TO SEND erlaubt HOLD TO AUDITION und SEND DRAFT - die Sperre liegt am Befund im Modell, nicht an einer ausgegrauten Schaltflaeche. Messqualitaet der Passage und Sicherheit des Befunds bleiben zwei Felder aus zwei Quellen: ein Befund mit confidence.class hoch, dessen Zustand more_data sagt, bleibt nicht handelbar. Alternativen sind EIGENE Befunde mit eigenem Zustand und reisen als IDs; ein Freitext faellt. Der Beleg ist die markierte Zone - Bandrand 0 und 221 gueltig, leeres Intervall und 222 nicht -, und ein Belegtextfeld gibt es nicht. Die drei Anzeigezeilen sind drei eigene Felder; eine vierte oder eine leere faellt. Die acht Ausschlussgruende kommen mit ihrem Grund an, ein neunter faellt. Jede Zahl faellt an ihrer Grenze und nicht erst danach. Zuletzt die Fassungsleiter: ein Leser der Fassung 2 lehnt findings ab, Abwesenheit heisst keine Befunde, und ein Sitzungswechsel raeumt sie ab. Seit Etappe E dazu der Maskierungswert AM Befund: er benennt Frequenzbereich und beide Quellen, gueltig und herabgesetzt sind zwei eigene Bits, Abwesenheit ist etwas anderes als ein ungueltiger Wert, und ein Objekt beliebiger Form faellt - sechs Formen einzeln geprueft. Eine Zeichenanweisung (Achse, Zoom, Farbe) faellt am Vertrag, auch im Maskierungswert. | `eq-copilot\build\plugin\EqCopSonde014BefundTest_artefacts\Release\EqCopSonde014BefundTest.exe` | [OK] Exit 0 | 0,14 s | [B28](roh/SONDE-014-0e853f9.md#b28) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,28 s | [B8](roh/SONDE-014-0e853f9.md#b8) |

