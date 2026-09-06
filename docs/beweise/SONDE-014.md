# SONDE-014 — S23–25: Absicht, Ursachenhypothese, Vorschlag und Assistentenschritt

| Merkmal | Wert |
|---|---|
| Ticket | S23–25, `SONDE-014` (Phase P4–P5), Leitungsname „Aus Messungen belegte Befunde und kleinste Tests ableiten" |
| Phase | **Etappe 1 — Verhaltensmatrix** (Spezifikation vor Code). In dieser Etappe entsteht kein Produktcode. |
| Urteil | **offen** — weder Bau- noch Prüfurteil. Dieses Manifest friert die aus den verbindlichen Quellen belegbare Verhaltensgrenze ein und benennt die Stellen, an denen der spätere Bau ohne weiteren Vertragsentscheid nicht ehrlich fortfahren kann. |
| Prüfstufe | T1+T2 gefordert (`docs/bauaufteilung-sonden.md` Zeile 394 · `docs/plan/plan.json`, Schritt S23–25, `"stufe": "T2"`). Keine Marke vergeben. |
| Prüfmodell | Codex `gpt-6-astra`, Effort **max**, lesend — für Matrix-, Erst-, Wieder- und Abschlussprüfung. `gpt-5.6-sol` max **nur** als Gegenprüfer (User-Wort 05.09.2026, `feedback_astra-prueft-sol-gegenprueft`). |
| Basis-SHA | `5e99a1361696be84a7174575ec7b9ad9e27e6a7f`, mit `git rev-parse HEAD` beim Start gemessen; Worktree bis auf die Auftragsdatei sauber (`git status --short`: eine Zeile `?? docs/beweise/roh/SONDE-014-etappe-1-auftrag.txt`). |
| Kanon vorher | **GRÜN 54/54 auf `aca0f28`**, Rohausgabe `docs/beweise/roh/NAK-182-aca0f28.md`. Eigene Messung in dieser Sitzung: `git diff --stat aca0f28..5e99a13 -- broker eq-copilot tools` ist **leer**. Der Codestand von `5e99a13` ist mit dem beglaubigten Kanonstand identisch; die Commits dazwischen berühren nur `docs/` und `design/`. |
| Änderungssatz dieser Etappe | Genau zwei Dateien: dieses Manifest `docs/beweise/SONDE-014.md` und der unveränderte Auftrag `docs/beweise/roh/SONDE-014-etappe-1-auftrag.txt`. Kein Produkt-, Test-, Schema-, Fixture- oder Werkzeugcode; kein Eingriff in `docs/plan/`, `docs/offene-punkte.md`, `docs/PLAN-STAND.md`, `docs/NEXT-SESSION.md` oder `design/`. |
| Grenze | Etappe 2 baut ausschließlich, was §3 zusagt. Technikfragen ohne Quelle stehen in §4 als ENTSCHEIDEN-VORSCHLAG; Produktfragen werden benannt und einer Fläche zugeordnet, nie beantwortet. |
| Ticketpfade (Vorschlag für den Prüfbereich der Etappe 2) | `broker/src/coordinator/` (neu: `intent.rs`, `hypothese.rs`, `proposal.rs`, `assistent.rs`, `maskierung.rs` — anzulegen), `broker/src/coordinator/mod.rs`, `broker/src/store/writer.rs`, `broker/src/dto.rs`, `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`, `eq-copilot/schemas/state/nakama-state-v2.md`, `eq-copilot/plugin/state/NakamaState.cpp`, `eq-copilot/plugin/state/NakamaState.h`, `eq-copilot/plugin/src/SourcesModel.cpp`, `eq-copilot/plugin/src/SourcesModel.h`, `eq-copilot/plugin/src/PluginProcessor.cpp`, `eq-copilot/fixtures/p4-korpus/`, `tools/eq-copilot/`, `tools/beweise.ps1`. |
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
- **OFFEN:** Mindestens ein Teil der Zusage lässt sich aus keiner Quelle
  ableiten und ist **Produkt**frage. Diese Zeile ist kein stiller Produktentscheid;
  sie wird benannt und einer Fläche zugeordnet, nie hier beantwortet.

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
> Aussage der Registerzeile stimmt, ihre Position nicht. Notiert in §4.10 als N-01.

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
> `grep -c "manual_passages"` auf die Schemadatei liefert `0`. Notiert in §4.10 als N-02.

### 2.6 (e) v3-Nachrichten und reservierte Namen

`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`, in dieser Sitzung mit
`py -3.13` gezählt: **22 definierte** Familien, **7 reservierte** Familien,
`gesamt_erwartet = 29`; dazu **5 belegte** Nachrichten, **14 belegte Felder**
und **5 reservierte Felder**. Die Zahl 29 wird von `pruefe_v3_vertrag.py`
(Bein **A5**) gegen das `oneOf` des Schemas gehalten und ist damit die
maschinenlesbare Wahrheit; der Fließtext im Feld `zweck` rechnet noch auf 28
und hat `experiment_candidate` nicht mitgezählt (§4.10, N-05).

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

### 3.1 SourceIntent und die fünf Rollen (Gate: §59 Punkt 1, §37, U22)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-01 | Gen ist klassifiziert `main`; der User setzt eine Rolle an einer Quelle | Die Rollenmenge ist **geschlossen und hat genau fünf Werte**: führt, trägt, begleitet, geschützt, bewusst verschmolzen. Ein sechster Wert wird abgewiesen, nicht auf einen bekannten Zweig abgebildet. Die Menge lebt an genau einer Stelle im Vertrag; Leser in C++ und Rust prüfen gegen dieselbe Menge. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fall `rollenmenge_ist_geschlossen_und_fuenf`; **A5** erweitert um das Enum | Ein sechster Enumwert wird angenommen; oder C++- und Rust-Leser klassifizieren denselben Wert verschieden | U22 (§1.7); `docs/plan/plan.json` S23–25 Nachtrag 06.09.; Entwurf §9 Erratum (h) |
| M-02 | Rolle steht fest; das Intent-Objekt wird gebildet | Die fünf Rollen sind die **Produktunterscheidung**, nicht das Speicherformat. Die Abbildung auf §37.1 (`prominence` `foreground\|middle\|background`, Funktionstags, geschützte Bereiche/Eigenschaften, gerichtete Beziehungen) ist Technik dieses Tickets und in §4.1 als ENTSCHEIDEN-VORSCHLAG geführt. Die Abbildung ist **total und injektiv**: jede Rolle erzeugt genau eine Belegung, und aus der Belegung ist die Rolle eindeutig zurücklesbar. | **ENTSCHEIDEN-VORSCHLAG** (§4.1) | **NEU** `EqCopSonde014IntentTest`, Fall `rolle_und_belegung_sind_umkehrbar` (Roundtrip über alle fünf) | Zwei Rollen erzeugen dieselbe Belegung, oder eine Belegung liest sich als andere Rolle zurück | U22 „die Abbildung … ist Technik von S23–25"; Entwurf §37.1:2427–2443 |
| M-03 | Quelle hat kurze Anschläge, die erhalten bleiben sollen | **Impuls ist keine Rolle.** Der Schutz kurzer Anschläge ist eine **Schutzangabe an der Quelle** (§37.1 „geschützte … Eigenschaften wie Attack, Breite oder Ausklang") und unabhängig von der gesetzten Rolle setzbar. Eine Quelle mit Rolle `begleitet` kann Attack geschützt haben. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fall `attack_schutz_ist_unabhaengig_von_der_rolle` | Der Schutz lässt sich nur zusammen mit einer bestimmten Rolle setzen, oder eine Rollenänderung löscht ihn | U22 Folge Punkt 2; Entwurf §9 Erratum (h) |
| M-04 | Zwei Quellen sollen bewusst verschmelzen | **Raum ist keine Rolle.** Die Verschmelzung ist eine gerichtete beziehungsweise ungerichtete **Erlaubnis** zwischen zwei Quellen (§37.1 „A/B dürfen verschmelzen") und wirkt als **Veto** gegen Entmaskierungsempfehlungen — nicht als Behauptung, jeder spektrale Konflikt sei gut. Die Rolle `bewusst verschmolzen` an einer Quelle und die paarweise Erlaubnis sind zwei Dinge; §4.1 entscheidet ihr Verhältnis. | **ENTSCHEIDEN-VORSCHLAG** (§4.1) | **NEU** `EqCopSonde014IntentTest`, Fall `verschmelzungserlaubnis_ist_veto_kein_gutachten` | Ein Proposal zur Entmaskierung entsteht trotz Erlaubnis; oder die Erlaubnis erzeugt selbst eine positive Aussage | Entwurf §37.2:2445–2459; U22 Folge Punkt 2 |
| M-05 | Zwei Quellen tragen widersprüchliche Intents | Die Konfliktauflösung folgt **genau der Reihenfolge aus §37.2**: 1. Schutz-/Sicherheitsgrenze, 2. expliziter Userintent, 3. passagespezifischer vor globalem Intent, 4. bestätigte Vorlage, 5. abgeleitete Vermutung. Die Stufen sind eine geordnete Liste, kein Score; eine höhere Stufe wird von keiner Summe niedrigerer geschlagen. | **BELEGT** | **NEU** `EqCopSonde014IntentTest`, Fall `konfliktregeln_sind_eine_ordnung_kein_score` — alle zehn Paarungen der fünf Stufen | Eine Kombination aus zwei niedrigen Stufen schlägt eine hohe; oder die Reihenfolge der Auswertung ändert das Ergebnis | Entwurf §37.2:2445–2459 |
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
| M-23 | Die Passage ist zeitlich **verschoben**, eine **andere** oder **zu kurz** | Keine davon kann einen **starken** Ursachenbeleg erzeugen. Die Prüfung nutzt die bestehenden Gates aus `vergleichbarkeit.rs` (`GATE_ZEITUEBERDECKUNG = 0.95`, `GATE_QUELLEN_JACCARD = 0.9`, `GATE_MATERIAL_COSINE = 0.95`, `GATE_ABDECKUNG = 0.5`); es entsteht **keine zweite Schwellenfamilie**. | **BELEGT** (Gates existieren) · **BAULÜCKE** (kein Konsument für Hypothesen) | **NEU** `EqCopSonde014HypotheseTest`, drei Fälle; **A4** `vergleichbarkeit` bestehend | Ein Befund auf einer um 30 % verschobenen Passage trägt `claim_class` 3; oder die Hypothese führt eine eigene Zeitschwelle | Entwurf §36.4:2411–2421; `broker/src/coordinator/vergleichbarkeit.rs`:152–160 |
| M-24 | Eine Evidenz wird zurückgenommen (`evidence_invalidate`) | **Entfernen eines Evidenzobjekts invalidiert abhängige Hypothesen deterministisch.** Reihenfolge unter **einem** Lock: Umfang auflösen (`Ids`, `Bereich`, `GanzeSitzung`) → betroffene Evidenz-IDs markieren → jede Hypothese, deren `evidence_ids` eine davon enthält, terminal invalidieren → Projektion in `findings` → Outbox. „Deterministisch" heißt: dieselbe Rücknahme in anderer Reihenfolge trifft dieselbe Menge. | **BAULÜCKE** | **NEU** `EqCopSonde014HypotheseTest`, Fall `ruecknahme_invalidiert_abhaengige_hypothesen`; **A4** Verdrahtungsfall über Store und Neustart | Eine Hypothese überlebt die Rücknahme ihrer letzten Evidenz; oder zwei Reihenfolgen derselben Rücknahme treffen verschiedene Mengen | Entwurf §36.4:2411–2421; `broker/src/coordinator/invalidierung.rs`:64,96 |
| M-25 | Gleiche Eingaben, derselbe Build, dieselbe ISA | Das Ranking ist **bytegleich**. Dafür gehören **Bootstrap-Seed**, **Float-Reduktionsreihenfolge**, **kanonische Eingangsquantisierung** und ein **stabiler Tie-Break-Key** zur `metrics_version`. Plattformübergreifend gelten identische Rangfolge und festgelegte numerische Toleranzen statt unrealistischer Bytegleichheit. | **BELEGT** · **BAULÜCKE** (Tie-Break-Key existiert nicht) | **NEU** `EqCopSonde014HypotheseTest`, Fall `ranking_ist_bytegleich_ueber_hundert_laeufe`; **A5** erweitert um den Tie-Break-Key in `metriken-v1.json` | Zwei Läufe auf demselben Eingang liefern verschiedene Reihenfolgen bei Gleichstand; oder der Seed steht nicht in der `metrics_version` | Entwurf §36.4:2411–2421; `broker/src/coordinator/vergleichbarkeit.rs`:149 |
| M-26 | Zwei Kandidaten liegen praktisch gleich | **Beide werden gezeigt.** Der Tie-Break entscheidet nur die Anzeigereihenfolge, nicht die Auswahl; er ist stabil und Teil der `metrics_version`. | **BELEGT** | **NEU** `EqCopSonde014HypotheseTest`, Fall `gleichstand_zeigt_beide` | Bei Gleichstand verschwindet ein Kandidat; oder die Reihenfolge wechselt zwischen zwei Läufen | Entwurf §36.3:2388–2409 |
| M-27 | Die Datenlage reicht nicht | **„Mehr Daten nötig" ist ein reguläres Ergebnis** und kein Fehlerzustand: es hat eine eigene Ausprägung im Objekt, eine eigene Ursachenklasse (§8, siebte) und wird im Korpus als **Enthaltung** gezählt, nicht als Fehlschlag. | **BELEGT** | **NEU** `pruefe_p5_korpus.py` (anzulegen) plus `EqCopSonde014HypotheseTest` | „Mehr Daten" erzeugt einen Fehlercode statt eines Ergebnisses; oder es zählt im Korpus als falsche Aussage | Entwurf §36.3:2388–2409; §8:842–861; `tools/eq-copilot/pruefe_p4_korpus.py`:91,222 |
| M-28 | Eine Behauptung wird sichtbar | **Jede sichtbare Behauptung referenziert existente Evidenz-IDs** (Exit-Gate, wörtlich). „Existent" ist eine Prüfung gegen den Store, nicht gegen den Speicher: eine ID, deren `evidence`-Zeile fehlt oder ausgeschlossen ist, macht die Behauptung unsichtbar beziehungsweise `STALE` — sie wird nie ohne Beleg gezeigt. | **BELEGT** · **BAULÜCKE** | **NEU** `EqCopSonde014HypotheseTest`, Fall `behauptung_ohne_existente_evidenz_wird_nicht_sichtbar`; **A4** Storefall nach Neustart | Ein Befund mit einer gelöschten Evidenz-ID bleibt sichtbar und handelbar | Entwurf §59 Exit-Gate (§1.1); §34.2 |

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
| M-36 | Ein Befund über eine Maskierung zweier Quellen entsteht | Die Maskierung **fällt aus dem Befund heraus** als **laufender, je Frequenzbereich auflösbarer Wert** — nicht nur als fertiger Satz mit Ort und Beleg. „Je Frequenzbereich auflösbar" heißt: der Wert trägt einen Bandindex beziehungsweise ein Bandintervall des bestehenden Gitters, keine neue Frequenzachse. | **BELEGT** · **BAULÜCKE** | **NEU** `EqCopSonde014MaskierungTest`, Fall `maskierungswert_benennt_einen_frequenzbereich` | Der Wert ist ein Skalar ohne Frequenzbezug; oder er führt ein eigenes Gitter | U16-Folge (§1.5); Bauaufteilung Zeile 394 (§1.3); `eq-copilot/schemas/v3/bandgitter/` |
| M-37 | Die Sonden messen; die Maskierung soll laufend gezeigt werden | Der Wert **hält die Anzeigekadenz mit**. Gemessen wird das an der Kadenz, die ihn trägt: der Evidenzpfad läuft mit 1 bis 4 Hz (`FeatureEngine.h`:989–990), der Editor publiziert leicht mit 20 Hz und schwer mit 4 Hz (`WorkerCadence.h`). Die Zusage ist erfüllt, wenn der Wert **mindestens** mit der schweren Editorkadenz (4 Hz) neu vorliegt, ohne dass ein Takt nachgeholt wird. Paint-FPS ist nicht Datenkadenz. | **BELEGT** · **ENTSCHEIDEN-VORSCHLAG** (Trägerkadenz, §4.4) | **NEU** `EqCopSonde014MaskierungTest`, Fall `maskierungswert_haelt_die_schwere_kadenz`; **A23** `pruefe_sonde012_sources_latency.py` als Muster | Der Wert erscheint nur beim Neuberechnen eines Befunds (also seltener als 4 Hz); oder eine Pause holt mehrere Takte nach | U16-Folge (§1.5); `eq-copilot/plugin/src/WorkerCadence.h`:27–28 |
| M-38 | Ein Finding ist ausgewählt | Das gewählte Finding **markiert seinen Frequenzbereich im stabilen Gesamtgraph**, ohne dessen Größe, Achsen oder Ausschnitt zu ändern. Der Datenweg liefert dafür nur den Bereich; er löst **keine** Achsen- oder Ausschnittsänderung aus. | **BELEGT** | **NEU** `EqCopSonde014MaskierungTest`, Fall `markierung_aendert_keine_achse` | Der Datenweg liefert eine Achsen- oder Zoomangabe mit | PR2-Nachtrag (§1.4); `design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md` („Stabile Geometrie", „Graphreaktion") |
| M-39 | Der User zoomt auf den markierten Bereich | Ein Zoom ist **nur als bewusste und reversible Aktion** zulässig und **darf nicht aus dem Datenweg heraus ausgelöst werden**. Der Datenweg kennt kein Zoomkommando. | **BELEGT** | **NEU** `EqCopSonde014MaskierungTest`, Fall `datenweg_kennt_kein_zoomkommando` | Eine Befundänderung erzeugt eine Zoomanforderung | PR2-Nachtrag (§1.4) |
| M-40 | Der Maskierungswert soll die Leitung erreichen | **Ob ein eigenes v3-Feld nötig ist, entscheidet der Datenweg an der Quelle.** Die Alternativen stehen in §4.4: (a) Ableitung aus der vorhandenen Bandevidenz (`evidence_snapshot.baender`, `verteilung.p50`) im Empfänger, ohne neues Feld; (b) ein neues Feld am Snapshot, das ein Fassungsschritt wäre. Fällt die Entscheidung auf ein Feld, gehört der **Name** in die v3-Vertragsrunde von S16–17, weil die vor R0 liegt. **Diese Etappe ändert kein Schema.** | **ENTSCHEIDEN-VORSCHLAG** (§4.4) | **NEU** `EqCopSonde014MaskierungTest` plus **A5** (falls ein Feld entsteht: Negativfixture und Fassungsleiter) | Ein Feld entsteht ohne `schema_minor`-Schritt; oder der Empfänger erfindet den Wert aus Anzeigedaten | U16-Folge (§1.5); `docs/plan/plan.json` S23–25 Nachtrag 30.08.; `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` (`regel`) |
| M-41 | Die farbige Zone wird gezeichnet | Die Zone **zeigt nur, was der Befund belegt hat, und setzt keine eigene Schwelle**: sie ist keine zweite Wahrheit neben dem Befundtext und keine eigene Detektion. Die Zone selbst gehört zu **S31b**; dieses Ticket liefert ausschließlich den Datenweg. | **BELEGT** | **NEU** `EqCopSonde014MaskierungTest`, Fall `zone_hat_keine_eigene_schwelle` (der Datenweg liefert keine Schwelle) | Der Datenweg trägt eine Anzeigeschwelle; oder die Zone entsteht ohne Befund | U16-Folge (§1.5); Blueprint P5-Bedienvertrag („keine eigene Detektion und keine konkurrierende Schwelle") |

### 3.5 Proposal-Policy nach §42 (Gate: §59 Punkt 3, §42, NAK-168)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-42 | Ein Befund ist handelbar; ein Vorschlag entsteht | Eine Empfehlung ist **zuerst ein validiertes, versioniertes `Proposal`** und erst danach Text. Die Felder aus §42.1: `proposal_id`, `proposal_schema`, `target`, `base_revision`, `passage_id`, `action`, `parameters`, `allowed_bounds`, `evidence_ids`, `expected_effect`, `protected_traits`, `listen_for`, `stop_if`, `execution`, `confidence`. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `proposal_traegt_die_fuenfzehn_felder`; **A5** Negativfixture je Feld | Ein Proposal ohne `stop_if` oder ohne `allowed_bounds` wird angenommen | Entwurf §42.1:2762–2787 |
| M-43 | Ein Proposal wird sichtbar | Das Exit-Gate verlangt wörtlich, dass **jedes Proposal Ziel, Passage, Grenzen, Hörziel, Stopbedingung und Rückweg nennt**. Abbildung: Ziel → `target`, Passage → `passage_id`, Grenzen → `allowed_bounds`, Hörziel → `listen_for`, Stopbedingung → `stop_if`, **Rückweg** → in P5 die Rücknahme des manuellen Versuchs (`experiment_abort`) beziehungsweise die ausdrückliche Angabe „manuell, nicht automatisch wiederherstellbar" (§43.1). Der Rückweg ist ein **Feld**, kein Versprechen im Text. | **BELEGT** · **ENTSCHEIDEN-VORSCHLAG** (Feldname des Rückwegs, §4.5) | **NEU** `EqCopSonde014ProposalTest`, Fall `sechs_gate_felder_sind_pflicht` | Ein Proposal ohne Rückweg wird sichtbar; oder der Rückweg steht nur in einem Textbaustein | Entwurf §59 Exit-Gate (§1.1); §43.1:2837–2865 |
| M-44 | Ein Proposal wird erzeugt | Die Erzeugung ist **deterministisch** und läuft in genau dieser Reihenfolge (§42.2): 1. Befundtyp wählt eine geprüfte Aktionstemplate; 2. ein Constraint-Solver entfernt Aktionen, die **Intent, Capability, Headroom, Messqualität oder Schutzbereiche** verletzen; 3. für EQ eine regularisierte Zielfunktion (gewünschte Evidenzverbesserung bei möglichst wenig Bändern, Gain und spektraler Nebenwirkung); 4. **der kleinste sichere Kandidat gewinnt**, Alternativen bleiben sichtbar; 5. `execution` wird gesetzt. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fälle `derselbe_eingang_erzeugt_denselben_entwurf` (100 Läufe) und `kleinster_sicherer_kandidat_gewinnt` | Zwei Läufe auf demselben Eingang liefern verschiedene Parameter; oder ein größerer Kandidat gewinnt bei gleicher erwarteter Wirkung | Entwurf §42.2:2789–2804; §42.4:2822–2833 |
| M-45 | Eine Aktion würde eine eigene DSP-Fähigkeit brauchen | **In P5 ist jede Aktion `manual`, nichts ist `previewable`.** `previewable` setzt exakt unterstützte eigene DSP-Aktionen voraus (§42.2 Punkt 5), und der aktive Kern entsteht erst in P6 (`SONDE-015`). Ein Proposal mit `execution = previewable` ist in P5 ein Vertragsbruch, kein optimistischer Vorgriff. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `in_p5_ist_jede_aktion_manual` | Ein erzeugtes Proposal trägt `execution = previewable` | Entwurf §42.2 Punkt 5:2802–2803; §59 „noch ohne Fernänderung" (§1.1); Bauaufteilung P6-Zeile |
| M-46 | Kein sicherer Eingriff ist begründbar | **`Keine Änderung` ist ein gültiger Vorschlag**, ebenso **`mehr Daten`**. Beide sind reguläre Ergebnisse mit vollständigem Objekt (Ziel, Passage, Hörziel, Stopbedingung, Rückweg), nicht ein leerer Rückgabewert. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fälle `keine_aenderung_ist_ein_vorschlag` und `mehr_daten_ist_ein_vorschlag` | Der Erzeuger liefert `None` statt eines Objekts; oder eines der beiden Ergebnisse trägt keine Evidenz-IDs | Entwurf §59 Lieferumfang (§1.1); §42.1:2762–2787 |
| M-47 | Ein Proposal wird gebildet (**NAK-168, Hälfte 1**) | **Der Vorschlag benennt den Zielbereich.** Das Band beziehungsweise Bandintervall, das der Eingriff adressiert, kommt aus dem Befund (`band_hz` aus §36.3) und steht im Proposal (`parameters.frequency_hz` plus `allowed_bounds.frequency_hz`). Es wird **nicht** aus einem Messdelta geraten. | **BELEGT** · **BAULÜCKE** | **NEU** `EqCopSonde014ProposalTest`, Fall `zielbereich_kommt_aus_dem_befund_nicht_aus_dem_delta` | Der Zielbereich wird aus dem größten Betrag der Banddeltas abgeleitet | NAK-168 (§1.8); Entwurf §36.3:2388–2409; §42.1:2762–2787 |
| M-48 | Ein Experiment liest den Zielbereich (**NAK-168, Hälfte 2**) | **Das Experiment liest Zielbereich und Schutzbereiche, statt sie zu raten.** Heute erklärt `experiment_verdrahtung.rs`:1069–1088 das Band mit dem größten absoluten Delta zum Ziel; weder `experiment_begin` noch `experiment_candidate` tragen ein Feld dafür (gemessen in §2.6). Die Vertragsfrage — welche Familie das Feld trägt und wie es heißt — steht in §4.5 als ENTSCHEIDEN-VORSCHLAG. | **BAULÜCKE** · **ENTSCHEIDEN-VORSCHLAG** (§4.5) | **NEU** `EqCopSonde014ProposalTest` plus **A4** Verdrahtungsfall; **A5** falls ein Feld entsteht | Das Experiment leitet den Zielbereich weiter aus dem Delta ab, obwohl das Proposal ihn nennt | NAK-168 (§1.8); `broker/src/coordinator/experiment_verdrahtung.rs`:1069–1088 |
| M-49 | Guardrails werden ausgewertet | Eine geschützte Eigenschaft erscheint als **harte Constraint** im Vorschlag (`protected_traits`) und im DSP-ACK. In P5 gibt es kein DSP-ACK — die Hälfte, die dieses Ticket bauen kann, ist die **Vorschlagsseite**; die ACK-Seite gehört S26–28/S29–31 und wird hier nicht vorgegriffen. | **BELEGT** (Vorschlagsseite) · **OFFEN für P7** | **NEU** `EqCopSonde014ProposalTest`, Fall `geschuetzte_eigenschaft_ist_harte_constraint` | Ein Proposal überschreitet einen geschützten Bereich; oder `protected_traits` bleibt leer, obwohl der Intent einen Schutz führt | Entwurf §37.4:2467–2473; §42.1:2762–2787 |
| M-50 | Guardrail-Skalare fehlen im Vertrag | **Ein Guardrail ohne Vertragsteil ist nicht messbar und wird als „nicht gemessen" ausgewiesen, nie als „unverändert".** `guardrail_loudness_db` und `guardrail_peak_db` sind heute hart `None` (NAK-204), `seitenanteil_db` erreicht den Broker strukturell nie (NAK-193). Ein Proposal, dessen `stop_if` auf einen nicht messbaren Guardrail zeigt, ist **nicht handelbar** und meldet `MORE DATA`. | **BAULÜCKE** (fremde Registerzeilen, hier nur verkettet) | **NEU** `EqCopSonde014ProposalTest`, Fall `stop_if_auf_nicht_messbarem_guardrail_ist_nicht_handelbar`; **A4** bestehend für `guardrail_nicht_gemessen` | Ein Proposal steht auf `READY TO SEND`, obwohl seine Stopbedingung auf `guardrail_peak_db` zeigt | NAK-204, NAK-193 (§1.8); `broker/src/coordinator/experiment_verdrahtung.rs`:1039–1042 |
| M-51 | Ein Proposal wird angezeigt | **Jeder angezeigte Zahlenwert lässt sich auf Proposal-Feld, Evidenz und Generatorversion zurückführen.** Kein Zahlenwert entsteht in der Anzeige. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `jede_zahl_hat_feld_evidenz_und_generatorversion` | Ein angezeigter Wert hat kein Feld im Objekt; oder die Generatorversion fehlt | Entwurf §42.4:2822–2833 |
| M-52 | Ein Proposal ist veraltet, ungültig oder außerhalb der Capability | Solche Vorschläge **erreichen keine Probe**. In P5 heißt das: sie werden gar nicht erst als `draft_offer` gebildet; der Zustand ist `STALE` beziehungsweise `MORE DATA`. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, drei Fälle (veraltet, ungültig, Capability fehlt) | Ein Proposal mit veralteter `base_revision` wird gesendet | Entwurf §42.4:2822–2833 |
| M-53 | Sicherheitsbudgets werden angewendet | Die Startwerte aus §42.3 gelten als **Produktgrenzen**: höchstens ein musikalischer Eingriff pro Assistentenschritt, höchstens drei EQ-Bänder pro Vorschlag; statischer EQ innerhalb ±3 dB; dynamische Reduktion standardmäßig höchstens 1,5 dB, Hard-Cap 3 dB; kein Lookahead, keine positive automatische Gesamtverstärkung, keine versteckte Normalisierung; **engere Usergrenzen gewinnen immer**. Jede Änderung braucht eine neue Policy-/Metrikversion und Regressionstests. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Property-Test über zufällige Eingänge: `hard_caps_und_engeres_userbudget_werden_nie_ueberschritten` | Ein erzeugtes Proposal überschreitet ein Hard Cap; oder eine engere Usergrenze wird von einer Produktgrenze geschlagen | Entwurf §42.3:2806–2820; §42.4:2822–2833 |
| M-54 | Ein Proposal wird gebildet | Jedes Proposal **referenziert Evidenz-IDs und die Intent-Revision**. Ohne beide entsteht kein Objekt; die Revision ist die, gegen die der Constraint-Solver gelaufen ist, nicht die zum Anzeigezeitpunkt. | **BELEGT** | **NEU** `EqCopSonde014ProposalTest`, Fall `evidenz_ids_und_intent_revision_sind_pflicht` | Ein Proposal ohne `evidence_ids`; oder die Revision wird beim Anzeigen nachgezogen | Entwurf §37.3:2461–2465; §59 Exit-Gate (§1.1) |

### 3.6 `AssistantStep` (Gate: §59 Punkt 4 und 5, §46.1, Entwurf Zeile 3177/3182)

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-55 | Ein aktives Problem wird bearbeitet | Der Assistent ist **eine deterministische Zustandsmaschine**, kein frei handelnder Chat. Die Folge aus §46.1 lautet `Coverage → Finding → Evidence → Listen → Proposal → Preview → Remeasure → Verdict`; **`Preview` entfällt in P5**, weil es keine eigene DSP-Aktion gibt (M-45). Die P5-Folge ist damit `Coverage → Finding → Evidence → Listen → Proposal → Remeasure → Verdict`. Der Wegfall ist ein Übergang, der übersprungen wird, **kein gelöschter Zustand**: der Vertrag behält den Namen für P6/P7. | **BELEGT** · **ENTSCHEIDEN-VORSCHLAG** (ob `Preview` als übersprungener Zustand oder als fehlender Wert modelliert wird, §4.6) | **NEU** `EqCopSonde014AssistentTest`, Fall `p5_folge_ueberspringt_preview_und_loescht_ihn_nicht` | Ein Schritt erreicht `Preview`; oder der Zustandsname verschwindet aus dem Vertrag | Entwurf §46.1:3172–3184 |
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
| M-65 | Synthetische Sessions mit **bekannter Quelle** laufen | Der Korpus unterscheidet **wahren Kandidaten, korrelierten Distraktor und Parent-Duplikat**. Jeder Fall trägt — wie im P4-Korpus — `fall`, `bein`, `wahrheit`, `aussage`, `konfidenz` und eine `quelle` mit Datei und **wörtlichem Bezeichner**, der dort gesucht wird; ein erfundener Bezeichner ist rot. | **BAULÜCKE** | **NEU** `erzeuge_p5_korpus.py` (anzulegen) mit `--pruefen` (bytegleiche Neuerzeugung, Muster **A25**) | Ein Fall nennt eine Quelle, deren Bezeichner in der Datei nicht wörtlich steht | Entwurf §36.4:2411–2421; `tools/eq-copilot/erzeuge_p4_korpus.py` (Bein **A25**) |
| M-66 | Die Schwelle aus M-31 soll kalibriert werden | Der Korpus **erzeugt** die Schwelle: die Abbildung hoch/mittel/unklar → handelbar wird so gewählt, dass Precision, Recall, Brier und Kalibrierung ihre Riegel halten. Die Schwelle ist Ausgabe, nicht Eingabe; sie wird versioniert und mit einem Regressionslauf geändert. | **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen), Fall `schwelle_ist_ausgabe_nicht_eingabe` | Die Schwelle wird als Konstante in das Bein gegeben und der Lauf bleibt grün | `docs/plan/plan.json` S23–25 Nachtrag 02.09. (1); U21-Folge |
| M-67 | Eine **verschobene Passage** liegt im Korpus | Sie erzeugt **keine starke** Aussage. Der Wert steht in der geschlossenen Wahrheitsmenge; auf ihm ist die starke Aussage falsch. Die Menge wächst gegenüber P4 um die Ursachenwahrheiten und bleibt geschlossen. | **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen); Muster: die zwei Mengen `KEINE_BEHAUPTUNG`/`KEINE_STARKE` aus **A26** | Eine starke Aussage auf einer verschobenen Passage bleibt grün | Entwurf §36.4:2411–2421; `eq-copilot/fixtures/p4-korpus/MANIFEST.json` (`mengen`) |
| M-68 | Der Korpus prüft sich selbst | Ein **Selbsttestbein ohne Repo-Fixture** (Muster **A27**) fährt synthetische Fälle durch **dieselben Funktionen**, die das Sammelbein führt, und lässt **jede Erwartung mit ihrem Gegenteil** laufen. Ohne das Gegenteil wäre der Selbsttest auch dann grün, wenn ein Riegel immer rot meldete. | **BELEGT** (Muster existiert) · **BAULÜCKE** (für P5) | **NEU** Selbsttestbein (anzulegen), Muster **A27** | Eine Riegelerwartung läuft ohne ihr Gegenteil | `tools/eq-copilot/pruefe_p4_korpus.py`:471–560 (Bein **A27**) |
| M-69 | Ein Gegenbeispiel wird gefahren (G5) | **Gegenbeispiele erzwingen nachweislich Enthaltung.** „Nachweislich" heißt: der Lauf zeigt, dass auf jedem Gegenbeispiel die Aussage `unsicher` ist und keine starke Aussage entsteht — nicht, dass sie zufällig ausblieb. Enthaltung ist das gewünschte Ergebnis, nicht ein Treffer. | **BAULÜCKE** | **NEU** `pruefe_p5_korpus.py` (anzulegen), Abschnitt Gegenbeispiele; G5 fährt zusätzlich adversarial | Ein Gegenbeispiel erzeugt eine starke Aussage; oder das Bein zählt die Enthaltung als Fehlschlag | Entwurf §59 Exit-Gate (§1.1); G5-Absatz (§1.3) |
| M-70 | Der P4-Korpus trägt die gedruckte Lücke | Die Zeile `nicht_gemessen` in `eq-copilot/fixtures/p4-korpus/MANIFEST.json` (**SONDE-013 M-85** Ziel 1, NAK-190) wird **erst dann entfernt, wenn der P5-Korpus den Fall wirklich misst**. Bis dahin bleibt sie gedruckt; ein Entfernen ohne Messung ist ein Rückschritt hinter NAK-182. | **BELEGT** | **A26** bestehend (druckt die Lücke heute), plus **NEU** `pruefe_p5_korpus.py` (anzulegen) | Die Lücke verschwindet aus dem Manifest, ohne dass ein P5-Fall sie misst | NAK-190 (§1.8); `docs/beweise/G4.md` §8; `docs/beweise/NAK-182.md` |

### 3.8 Ort und Nebenläufigkeit

| ID | Zustand × Ereignis | Zusage samt Reihenfolge und Frist | Belegklasse | Test | Rotbeweis an der Zusage | Quelle |
|---|---|---|---|---|---|---|
| M-71 | Intent, Evidenzgraph, Hypothese, Proposal und `AssistantStep` sollen einen Ort bekommen | Der Ort folgt **§33.5 wörtlich**: `SourceIntent` und `AssistantStep` gehören in den `MainProjectState` (autoritativ), mit versioniertem SQLite-Spiegel; `Evidenz, Findings, Proposals, Transaktionsereignisse, Experimente und Urteile` gehören in den **lokalen SQLite-Experimentstore mit einem Writer**, und der Main-State hält davon **nur kompakte aktuelle IDs und noch nicht bestätigte Outbox-Ereignisse**. **Kein Objekt besitzt zwei still konkurrierende Wahrheiten.** Die Rechenstelle folgt daraus: Hypothese und Proposal entstehen im Broker-Coordinator, weil dort der Evidenzbestand und der Store liegen. | **BELEGT** · **ENTSCHEIDEN-VORSCHLAG** (Modulschnitt, §4.8) | **NEU** `EqCopSonde014HypotheseTest` plus **A4**; **B14** für die Main-Seite | Ein Finding wird im Plugin gerechnet und im Broker noch einmal; oder der Main-State hält ein vollständiges Proposal statt einer ID | Entwurf §33.5:2140–2175 |
| M-72 | Irgendein Teil dieses Tickets rechnet | **Der Audio-Thread ist nie beteiligt.** Keine Sperre, keine Allokation, kein Datei-, Pipe- oder Netzzugriff, kein Logging im Audio-Thread. Überlast verwirft **Analyseframes, nie Audio**. | **BELEGT** | **A1** `EqCopNullTest` und **B4** `EqCopQueueStressTest` bestehend; **NEU** ein Fall, der die neuen Pfade auf Audio-Thread-Freiheit prüft | Ein neuer Pfad wird aus `processBlock` gerufen; oder eine Überlast verwirft Audio | `CLAUDE.md` Grundgesetz; Entwurf §33.1 |
| M-73 | Eine Queue läuft voll (Prüfliste A) | **Je Klasse steht die Politik bei *voll* ausdrücklich.** P0: 64er-Queue, Überlauf ⇒ Verbindung wird verworfen und der Eintrag geht an `beiP0Verworfen` — nie stillschweigend gelöscht. P1: 128 + 128 Einträge, leerer Schlüssel = Ereignis, nicht leerer Schlüssel **koalesziert Snapshots desselben Objekts**. P2 reist nicht über den ControlClient. Neue Objekte dieses Tickets bekommen **keine neue Klasse**: `draft_offer` und `user_verdict` sind P1 mit Schlüssel (Proposal- beziehungsweise Verdict-ID), Findings reisen über den bestehenden Snapshot-Pfad. | **BELEGT** · **ENTSCHEIDEN-VORSCHLAG** (Schlüsselwahl, §4.8) | **NEU** Lastfall im **A4**-Bereich plus **B10** `EqCopIpcTest` erweitert | Ein `draft_offer` wird ohne Schlüssel eingereiht und verdrängt ein anderes Objekt; oder ein Rückgabewert der Politik wird ignoriert | `tools/dirigent/pruefliste.md` A; `eq-copilot/plugin/core/ipc/ControlClient.h`:242,314,558–560 |
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

### 3.10 Zählung nach Belegklasse

Gezählt an den 84 Zeilen der neun Tabellen dieses Abschnitts mit einem
escape-bewussten Parser, nicht abgeschrieben: `M-01` bis `M-84`, keine Lücke,
keine Doppelung, jede Zeile sieben Spalten. **16 Zeilen tragen zwei Marken**
(eine belegte Zusage mit fehlender Baukante, oder eine belegte Zusage mit einer
offenen Technikfrage darin); die Summe der Spalte „Zeilen" ist deshalb 100 und
nicht 84.

| Belegklasse | Zeilen | IDs |
|---|---|---|
| **BELEGT** | 71 | M-01, M-03, M-05 bis M-20, M-23, M-25 bis M-39, M-41 bis M-47, M-49, M-51 bis M-64, M-68, M-70 bis M-83 |
| **BAULÜCKE** | 19 | M-21, M-22, M-23, M-24, M-25, M-28, M-31, M-36, M-47, M-48, M-50, M-59, M-64, M-65, M-66, M-67, M-68, M-69, M-84 |
| **ENTSCHEIDEN-VORSCHLAG** | 9 | M-02, M-04, M-37, M-40, M-43, M-48, M-55, M-71, M-73 |
| **OFFEN** | 1 | M-49 (nur die DSP-ACK-Hälfte, die P7 gehört) |

Die 16 doppelt markierten Zeilen im Einzelnen:

| Kombination | Anzahl | IDs |
|---|---|---|
| BELEGT · BAULÜCKE | 9 | M-23, M-25, M-28, M-31, M-36, M-47, M-59, M-64, M-68 |
| BELEGT · ENTSCHEIDEN-VORSCHLAG | 5 | M-37, M-43, M-55, M-71, M-73 |
| BELEGT · OFFEN | 1 | M-49 |
| BAULÜCKE · ENTSCHEIDEN-VORSCHLAG | 1 | M-48 |

Reine ENTSCHEIDEN-VORSCHLAG-Zeilen — die drei, für die es heute weder eine
Quelle noch eine Baukante gibt, sondern nur einen Entscheid: **M-02**
(Rollenabbildung, E-01), **M-04** (Verschmelzung, E-02) und **M-40**
(v3-Feld der Maskierung, E-04).

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

### 4.7 E-07 — Ob `Preview` ein übersprungener oder ein fehlender Zustand ist (M-55)

**Vorschlag.** Übersprungen, nicht gelöscht: die Zustandsmenge behält alle acht
Namen aus §46.1, und die **Übergangstabelle** von P5 führt keine Kante nach
`Preview`. Damit bleibt der Vertrag zwischen P5 und P6/P7 stabil und ein
P6-Ticket fügt nur Kanten hinzu statt einen Zustand.

**Alternative.** `Preview` entfällt aus der Menge und kommt in P6 zurück.
Nachteil: eine gespeicherte P5-Zustandsmaschine könnte einen P6-Stand nicht
lesen, und der `AssistantStep` ist persistent — das wäre ein Migrationsschritt
für nichts.

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

### 4.9 Produktfragen — benannt, einer Fläche zugeordnet, hier nicht beantwortet

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

### 4.10 Nebenbefunde in fremden Dateien (nicht geändert)

| Nr. | Befund | Ort | Warum hier nur notiert |
|---|---|---|---|
| N-01 | NAK-168 zitiert die Zielheuristik als `experiment_verdrahtung.rs:934-953`. Zum Stand `5e99a13` steht dort `mittel_je_band`; die Heuristik liegt bei :1069–1088. Die Zeilen sind durch `00fa14e` (NAK-182 B6) verrutscht. Die **Aussage** der Registerzeile stimmt. | `docs/offene-punkte.md`:180 | Fremde Datei; die Grenze dieser Etappe erlaubt keinen Eingriff in `docs/offene-punkte.md`. |
| N-02 | Der Zustandsbaum in der Schemadatei führt unter `MainProject` nur `confirmed_members_v1`. Die Eigenschaft `manual_passages_v1`, die `NakamaState.cpp`:36 schreibt und liest, fehlt dort; `grep -c "manual_passages"` liefert `0`. | `eq-copilot/schemas/state/nakama-state-v2.md` §2 | Schemadatei, außerhalb des Änderungssatzes dieser Etappe. Etappe 2 fasst sie ohnehin an (Intent, `AssistantStep`) und zieht die Zeile dann mit nach. |
| N-03 | `SourcesModel::findingsOffen` und `setzeFindings` haben im Produktcode keinen Aufrufer; einziger Aufrufer ist ein Test. Der Blueprint verlangt die Zahl. | `eq-copilot/plugin/src/SourcesModel.cpp`:1210 | Als Matrixzeile **M-84** geführt (BAULÜCKE), damit Etappe 2 sie schließt oder die Anzeige ehrlich unterlässt. |
| N-04 | Der Begriff „Überdeckung" trägt im Repo drei Bedeutungen (Messabdeckung, Zeitüberdeckung, spektrale Maskierung), von denen die dritte im Code nicht existiert. Der Blueprint nennt die dritte bereits **Maskierungszone**; Plan- und Bauaufteilungstext nennen sie weiter „Überdeckung". | `docs/plan/plan.json` S23–25, `docs/bauaufteilung-sonden.md`:394 gegen `design/docs/funktions-und-bedien-blueprint.md` §3.4 | Kein Widerspruch in der **Sache** — beide meinen dasselbe. Diese Matrix hält die Wörter getrennt (§2.12) und schlägt den Feldnamen entsprechend vor (E-04). Ein Angleichen der Plantexte ist Dirigentenarbeit, nicht Bauarbeit. |
| N-05 | Das Feld `gesamt_erwartet` steht auf **29** und wird von **A5** gegen das `oneOf` geprüft; der Fließtext im Feld `zweck` derselben Datei rechnet 26 + `session_command` + `experiment_manual_result` = **28** und lässt `experiment_candidate` aus, das seit Nacharbeit 2 (Befunde R16/R21) belegt ist und in `belegte_nachrichten` steht. Nur die Prosa ist hinterher, nicht der Vertrag. | `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`, Feld `zweck` | Vertragsdatei, außerhalb des Änderungssatzes dieser Etappe. Etappe 2 fasst sie an, sobald ein Name für dieses Ticket hinzukommt (E-04, E-05), und zieht den Satz dann mit nach. |

### 4.11 Widerspruch Entwurf ↔ Blueprint

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

---
## 5. Bauplan für Etappe 2

### 5.1 Reihenfolge der Bauetappen

Die Reihenfolge folgt der Abhängigkeit, nicht der Reihenfolge der Matrix. Jede
Etappe schließt einen **Änderungssatz** im Sinne von `CLAUDE.md`
(speichern↔laden, starten↔stoppen, verbinden↔trennen gehören zusammen) und
endet mit dem vollen Kanon, sobald sie Schema-, Fixture- oder Runnerdateien
berührt.

| Etappe | Inhalt | Matrixzeilen | Kanonbeine am Ende der Etappe |
|---|---|---|---|
| **A — Intent im Main-State** | `SourceIntent` als additive Eigenschaft in `MainProject`: fünf Rollen, Schutzangaben, gerichtete Beziehungen, Passage-Scope, Revision/Herkunft/Konfidenz, Zyklusprüfung beim Speichern, Host-Dirty. Writer, Reader, Migration, Goldens und Fixtures im **selben** Änderungssatz. | M-01 bis M-13 | **NEU** `EqCopSonde014IntentTest`; bestehend **B2**, **B14**, **A12**, **A11**; voller Kanon (Fixtures berührt) |
| **B — Evidenzgraph und `CauseHypothesis`** | Kandidatenbildung Stufe A/B, sechs Rangkomponenten mit hartem Coverage-/Alignment-Gate, `CauseHypothesis` mit zehn Feldern und den sechs Teilen, Determinismus (Seed, Reduktionsreihenfolge, Quantisierung, Tie-Break-Key in `metrics_version`), deterministische Invalidierung bei Evidenzrücknahme. Produzent für `event_type = "finding"` — die Projektion existiert bereits. | M-14 bis M-28 | **NEU** `EqCopSonde014HypotheseTest`; bestehend **A4**, **A5**, **A8**; voller Kanon |
| **C — Befundzustände** | Abbildung hoch/mittel/unklar → `READY TO SEND` / `MORE DATA` / `STALE`, Sperre von Audition und Draft im Datenweg, Alternativen als eigene Befunde, die drei Zeilen als Datenfelder, Trennung von `CONFIDENCE`. | M-29 bis M-35 | **NEU** `EqCopSonde014BefundTest`; bestehend **B13** |
| **D — Maskierungs-Datenweg** | Rechnung aus der vorhandenen Bandevidenz, Bindung an den Befund, Kadenz, keine Zeichenanweisung. **Hält vor dem Rückweg zu Gen an**, bis E-04 entschieden ist. | M-36 bis M-41 | **NEU** `EqCopSonde014MaskierungTest`; bestehend **A6**, **A19**, **A23** |
| **E — Proposal-Policy** | Objekt mit fünfzehn Feldern, deterministische Erzeugung in fünf Schritten, `keine Änderung` und `mehr Daten` als gültige Vorschläge, Sicherheitsbudgets als Property-Test, `execution = manual` in ganz P5, Zielbereich und Schutzbereiche aus dem Befund. Produzent für `event_type = "proposal"`. | M-42 bis M-54, M-63 | **NEU** `EqCopSonde014ProposalTest`; bestehend **A4**, **A5**; voller Kanon |
| **F — `AssistantStep`** | Zustandsmaschine mit sieben Zuständen (Preview übersprungen), fünf Angaben je Zustand, ein Slot, vier Gegenpfade, Persistenz in `MainProject`, Rekonstruktion nach Main-Neustart, Anschluss an den bestehenden Experimentkern. | M-55 bis M-62 | **NEU** `EqCopSonde014AssistentTest`; bestehend **B14**, **B25**, **B26**, **A4**, **A4-SI**; voller Kanon |
| **G — Evaluationskorpus** | Erzeuger und Prüfer nach dem Muster **A25**/**A26**/**A27**: Precision und Recall je **Ursachenklasse**, Kalibrierung, Brier, Coverage, Enthaltung; synthetische Sessions mit wahrem Kandidaten, korreliertem Distraktor, Parent-Duplikat und verschobener Passage; Riegel `falsche_starke`, `falsche_schwache`, `precision ≤ 1`, `recall ≤ 1`; Selbsttestbein mit Gegenteil je Erwartung. Die Schwelle aus M-31 ist **Ausgabe** dieses Beins. | M-64 bis M-70, M-31 | **NEU** Erzeuger-, Prüf- und Selbsttestbein (anzulegen); bestehend **A26** (druckt die Lücke, bis G sie misst); voller Kanon |
| **H — Ort, Nebenläufigkeit und Invarianten** | Rückstau-Politik je Klasse für die neuen Objekte, Verbinden/Trennen, Brokerneustart mit offenem Schritt, Audio-Thread-Freiheit, NaN-Ehrlichkeit, State-Verlustfreiheit, `findingsOffen` bekommt seine Quelle. | M-71 bis M-77, M-78 bis M-84 | bestehend **A1**, **A4**, **A4-SI**, **A10**, **A21**, **A22**, **B1**, **B2**, **B3c**, **B4**, **B10**, **B13**, **A14**; voller Kanon |

**Etappe D und E hängen an einem Entscheid** (E-04 beziehungsweise E-05) und
werden deshalb bis zu dieser Grenze gebaut und dort angehalten, nicht auf
Verdacht durchgezogen.

### 5.2 Ticketpfade (Prüfbereich der Etappe 2)

Alles außerhalb dieser Liste ist im Bauverlauf mit Begründung zu benennen.

**Broker:** `broker/src/coordinator/mod.rs`,
`broker/src/coordinator/evidenz.rs`,
`broker/src/coordinator/experiment.rs`,
`broker/src/coordinator/experiment_verdrahtung.rs`,
`broker/src/coordinator/invalidierung.rs`,
`broker/src/coordinator/invalidierung_verdrahtung.rs`,
`broker/src/coordinator/sicht.rs`,
`broker/src/coordinator/subscription.rs`,
`broker/src/dto.rs`,
`broker/src/store/writer.rs`,
sowie die neu **anzulegenden** Module `intent.rs`, `hypothese.rs`,
`proposal.rs`, `assistent.rs`, `maskierung.rs` und ihre Verdrahtungsdateien
unter `broker/src/coordinator/` (E-08).

**Verträge:** `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`,
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`,
`eq-copilot/schemas/v3/metriken-v1.json`,
`eq-copilot/schemas/state/nakama-state-v2.md`.

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
| R4 | **Ein Fassungsschritt ohne Entscheid.** | `session_snapshot` und `evidence_snapshot` sind `additionalProperties: false`; nach dem ersten Cross-Language-Fixture ist ein Feld nicht mehr frei änderbar. | Etappe D hält vor dem Rückweg an (E-04); Etappe E hält vor dem Experimentfeld an (E-05). Kein Schema wird ohne Dirigentenentscheid angefasst. |
| R5 | **Der Korpus misst sich selbst grün.** | NAK-182 fand genau das: `recall > 1` und `precision > 1` waren strukturell unerreichbar, die Wachen hatten keinen Weg zu fallen. | Das Selbsttestbein (M-68) speist synthetische Kennzahlen **direkt** in die Riegelfunktion, und jede Erwartung läuft mit ihrem Gegenteil. |
| R6 | **Ein Rotbeweis, der am Nebeneffekt fällt.** | Die Matrixprüfung von NAK-182 fand diesen Fehlertyp zweimal. | Die Spalte **Rotbeweis** in §3 nennt für jede Zeile, woran die Prüfung fallen muss. Ein Rotbeweis, der an einem Übersetzungsfehler, einer Rundungsphase oder einem Filter mit `t0` fällt, wird nicht angenommen. |
| R7 | **Die Guardrail-Lücken aus NAK-193/NAK-204 schlagen durch.** | Zwei von fünf Guardrails aus **SONDE-013 M-45** sind ohne Vertragsteil nicht messbar; der Breite-Guardrail ist strukturell tot. Ein Proposal, dessen `stop_if` darauf zeigt, wäre nicht überprüfbar. | M-50 verkettet die Lücke ehrlich: ein solches Proposal ist **nicht handelbar** und meldet `MORE DATA`, statt so zu tun, als wäre der Guardrail grün. Der Skalar-Entscheid selbst gehört NAK-193/NAK-204, nicht diesem Ticket. |

### 5.4 Wo der Bau ohne weiteren Vertragsentscheid nicht ehrlich fortfahren kann

| Stelle | Zeile | Blockierender Entscheid | Was ohne ihn baubar bleibt |
|---|---|---|---|
| Rückweg des Maskierungswerts zu Gen | M-40 | **E-04** (§4.4): eigenes Feld am `session_snapshot` oder nicht; falls ja, gehört der Name in die v3-Runde von S16–17 | Rechnung, Kadenz, Befundbindung und die Zusage „keine Zeichenanweisung" (M-36 bis M-39, M-41) — alles brokerseitig |
| Zielbereich und Schutzbereiche im Experiment | M-48 | **E-05** (§4.5): welches Feld an `experiment_begin` und wie es heißt | Die Proposal-Hälfte (M-47): der Vorschlag benennt den Zielbereich aus dem Befund |
| Rückwegfeld des Proposals | M-43 | **E-06** (§4.6): Name und geschlossene Wertemenge | Die übrigen fünf Gate-Felder (Ziel, Passage, Grenzen, Hörziel, Stopbedingung) |
| Abbildung der fünf Rollen | M-02 | **E-01** (§4.1): Rolle als eigenes Feld mit abgeleiteter Belegung (A) oder als Vorlage (B) | Nichts von Etappe A — die Abbildung ist ihre Grundlage. **E-01 ist deshalb der erste zu entscheidende Punkt.** |
| Verhältnis Rolle ↔ paarweise Verschmelzungserlaubnis | M-04 | **E-02** (§4.2) | Die übrigen vier Rollen |
| Trägerkadenz | M-37 | **E-03** (§4.3) | Der Wert selbst; nur seine Frist hängt daran |
| `Preview` in P5 | M-55 | **E-07** (§4.7) | Die übrigen sechs Zustände |
| Modulschnitt und Rückstau-Schlüssel | M-71, M-73 | **E-08**, **E-09** (§4.8) | Beide sind Formentscheide, kein Blocker: eine falsche Wahl kostet eine Umbenennung, keinen Vertragsbruch |

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
| **A** | Politik bei *voll* je Klasse ausdrücklich | ja | **M-73**; Ist-Stand §2.9 (`ControlClient.h`:242,314,558–560); Entscheid **E-09** (§4.8) |
| **A** | Puffer mit Abflussweg **ohne** Reconnect | ja | **M-74**; Bein **A4-SI** |
| **A** | Schlüssel überleben jeden Zwischenpuffer; Koaleszierung gilt auch dort | ja | **M-73** (`proposal:<id>` als Schlüssel), **E-09** |
| **A** | Niedrigere Klasse blockiert, höhere wird weiter gelesen | ja | **M-73**, **M-75**; bestehender P0/P1-Vertrag aus SONDE-010 |
| **A** | Rückgabewerte und Zähler einer Politik werden ausgewertet | ja | **M-73** (Rotbeweis: „ein Rückgabewert der Politik wird ignoriert"); Vorbild `evidenzKadenzReduktionen`/`evidenzNichtGesendet` (§2.9) |
| **A** | Dieselbe Regel in **allen** Sprachen, Test beidseitig, einmal gebrochen | ja | **M-77** |
| **B** | Reihenfolge beim Verbinden festgelegt und gemessen | ja | **M-75** |
| **B** | Beim Trennen zuerst abmelden, dann Fristen und Joins | ja | **M-75** |
| **B** | Jeder Join hat eine Frist; kein Self-Join | ja | **M-75**; bestehender Vertrag aus SONDE-011/012, von dieser Etappe nicht geändert |
| **B** | Nach dem Schließen einer Queue wird nichts geliefert; Schließflag **vor** dem Inhalt | ja | **M-75** |
| **B** | Stopp-Fenster: Registrierung nach begonnenem Stop ist ein Test | ja | **M-75**, **M-76** (Brokerneustart mit offenem Schritt) |
| **C** | Textfeld mit Schema-Länge im Leser gegen genau diese Länge, Negativfixture je Feld | ja | **M-15**, **M-42** (Negativfixture je Feld über **A5**); Muster: `probe_descriptor.host_bus_name` 1–120 Codepoints |
| **C** | Objekte mit exakter Feldmenge, `additionalProperties:false` in C++ **und** Rust | ja | **M-15**, **M-42**, **M-77**; Ist-Stand §2.2/§2.6 |
| **C** | Discriminator, Familie und Typ vor dem Inhalt geprüft, beidseitig | ja | **M-77**; §2.6 (`x-nakama-discriminator` nicht additiv) |
| **C** | NaN/Inf/Grenzwerte an jeder Zahl über den Draht | ja | **M-82** |
| **D** | Riegel fail-closed ohne Rohtextheuristik | ja | §5.5; **M-64** (Riegel des Korpusbeins) |
| **D** | Bein prüft die Frische seiner Eingaben, meldet Exit 3 statt grün | ja | §5.5; bestehende Runnerregel, von dieser Etappe nicht geändert |
| **D** | Relink ändert Bundlebytes; feste Hashes gelten für eine Auslieferung | **nein** | Diese Etappe und Etappe 2 ändern kein Bundle und keinen Installer; `eq-copilot/install/` steht ausdrücklich außerhalb des Prüfbereichs (§5.2). |
| **D** | Was der Kanon nicht baut, darf er nicht als frisch bezeugen | ja | §5.1: der volle Kanon läuft am Ende jeder Etappe, die Schema-, Fixture- oder Runnerdateien berührt |
| **E** | Behauptung ≤ Messung, Gate-Maß in wenigen Sätzen | ja | §5.5; die Spalte **Rotbeweis** in §3 hält jede Zusage an ihrer eigenen Messung |
| **E** | Zahlen im Manifest sind gemessen, nicht abgeschrieben | ja | §2 durchgehend: Kanon 54/54 und der leere `git diff --stat` in dieser Sitzung gemessen; Bandzahlen 221/64 aus dem Register; Korpuszahl 25 aus `MANIFEST.json`; Kadenzen aus `FeatureEngine.h`:989–990 und `WorkerCadence.h`:27–28; §3.10 aus den Matrixzeilen gezählt |
| **E** | Positionen im lebenden Kopf als Symbol/Anker | ja | Kopf: Symbol- und Ankerangaben; Zeilennummern nur im Verlauf ab §1, mit dem Stand `5e99a13` benannt |
| **E** | Lebender Kopf wird beim Abschluss nachgezogen, alles darunter append-only | ja | Kopfhinweis unter der Merkmalstabelle |
| **E** | Jede neue Prüfung einmal absichtlich gebrochen, Rohausgabe liegt bei | ja | §5.5; Spalte **Rotbeweis** in §3 |
| **E** | Geänderte Zusage — drei Stellen, kein Inventar | ja | §5.5 |
| **E** | Writer-Fixtures statt Handschrift | ja | §5.5; **M-65** (Korpusfälle tragen wörtliche Quelle, Muster **A25**) |
| **F** | speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg im selben Commit | ja (ohne den Installer-Teil) | §5.1: Etappe A trägt Writer, Reader, Migration, Goldens und Fixtures zusammen; Etappe F trägt Abbruch, Zurück, Überspringen, Resume und Rekonstruktion zusammen. Installieren↔Rückweg trifft nicht zu (kein Installereingriff, siehe **D**). |
| **F** | Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines Vertrags im selben Änderungssatz | ja | §5.1, Etappen A, B, E; **M-77** |

---
