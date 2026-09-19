# Tiefenaudit mit Agenten — Spielregeln (Skill `/tiefenaudit`)

**Stand: Entwurf vom 19.09.2026. Noch kein Lauf.** Auftrag des Users,
19.09.2026, wörtlich: „ich möchte, dass du aus der befund.md ein
reproduzierbaren agentenskill ableitest so wie der health scan skill von
sonifold, der über 20 agenten mit bestimmten arbeitsbereichen spawnt. diesen
skill werden wir dann nach und nach erweitern um am ende die ganze app abdecken
zu könnne, samt bekannten problemstellen, verbindungen zwischen codebereichen,
abhängigkeiten. x verändert y , aber auch z. ich will nicht dass du ihn gleich
ausführst, nur schonmal entwerfen"

Der Skill ist die Agentenfassung des Audits `/tiefenaudit` aus
`docs/gesundheit/KONZEPT.md` §4.3. Sein Ausgangsmaterial ist der Tiefenaudit 3
(`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`): dessen 88 Befunde sind die
ersten bekannten Problemstellen, dessen Urteil („freigabeblockierende Fehler an
den Übergängen zwischen Host, State, Transport und Beweis") ist der Grund für
die zweite Agentenart, die Nahtprüfer.

## 1. Was dieser Entwurf am Konzept ändert, und was bleibt

`KONZEPT.md` §9 lehnt Sonifolds „36 handgepflegte Agentenaufträge mit
Zeilenbereich-Slices" ab, weil sie zur Wartungslast wurden. Der neue Auftrag
des Users verlangt feste Arbeitsbereiche je Agent. Beides zusammen ergibt:

- **Die Bereiche sind Daten, die Aufträge werden gerechnet.** Handgepflegt ist
  nur `karte.json` (Bereiche, Nähte, Kopplungen). `tiefenaudit_karte.py
  --auftraege` rechnet daraus je Lauf jeden Auftrag: Dateiliste mit
  Zeilenzahlen, Invarianten, Fragen, bekannte Problemstellen, Kopplungen. Kein
  Auftrag wird von Hand geschrieben, keine Agentenzahl steht im Skill.
- **Bekannte Problemstellen stehen nicht in der Karte.** Sie werden aus den
  Befundquellen gerechnet (`--befunde`): jeder Befund fällt über seine Orte in
  die Bereiche, deren Dateien er nennt. Ein Befund mit Orten in zwei Bereichen
  ist eine **gemessene Naht**.
- **Drift zeigt sich als Riss, nicht als stiller Blindfleck.** Die Vorprüfung
  (`--pruefen`) verlangt, dass jede Quelldatei der Codebase zu einem Bereich
  gehört, jeder Pfad existiert und kein Bereich sein Zeilenbudget reißt. Eine
  neue Datei ohne Bereich ist ein Riss, bevor ein Agent startet.

Unverändert aus dem Konzept: ein Ledger (`docs/offene-punkte.md`), Population
statt Stichprobe, Scope-Beweis je Bericht, Skeptiker je Befund, Urteil im
Hauptlauf, das Audit ändert keinen Code, der Dirigent auditiert nie selbst
(§5), Sicherheitsarbeit läuft in ihrer eigenen Spur.

## 2. Dateien

| Datei | Rolle |
|---|---|
| `docs/gesundheit/tiefenaudit/karte.json` | Bereiche, Nähte, Kopplungen, Invarianten, Ausschlüsse, Befundquellen; die einzige handgepflegte Quelle |
| `docs/gesundheit/tiefenaudit/auftrag-bereich.md`, `auftrag-naht.md` | Vorlagen mit Platzhaltern; das Werkzeug füllt sie |
| `docs/gesundheit/tiefenaudit/auftrag-skeptiker.md` | Vorlage; der Hauptlauf füllt sie je Befundpaket |
| `tools/pruefung/tiefenaudit_karte.py` | Vorprüfung, Befundzuordnung, Auftragsrechnung, Berichtsprüfung, Betroffenheit; Standardbibliothek, mit Selbsttest |
| `docs/gesundheit/tiefenaudit/SKILL-vorlage.md` | dünner Starter als Vorlage; nennt keine Agentenzahl und keinen Auftragstext. Der Skill-Ordner unter `.claude/` ist im Modus `dontAsk` für Claude gesperrt (gemessen 19.09.2026, in der Arbeitskopie und im Hauptcheckout); die Kopie `.claude/skills/tiefenaudit/SKILL.md` ist am 19.09.2026 im Auto-Modus angelegt (inhaltsgleich, LF). Ändert sich die Vorlage, zieht eine Session im Auto-Modus die Kopie nach |
| `docs/audits/<JJJJ-MM-TT>-tiefenaudit-agenten/` | je Lauf: `BEFUNDE.md`, `auftraege/` (gerechnete Aufträge als Beleg), `roh/` (Berichte der Agenten) |

## 3. Drei Agentenarten

1. **Bereichssammler** (Spur `sammler`): je Bereich ein Agent. Er liest seine
   Dateien vollständig, Produkt und zugehörige Tests zusammen, zählt
   Populationen, prüft die bekannten Stellen auf „besteht, behoben,
   Nachbarschaft" und liefert einen Kartennachtrag. Er ordnet nicht ein.
2. **Nahtprüfer**: je Naht ein Agent, nach den Sammlern. Er liest deren
   Berichte und folgt einer Kette über Bereichsgrenzen (Host bis DSP, Speichern
   bis Laden, Sonde bis Gen, Bau bis Beweis). Eine Übergabe, für die keine
   Seite die Verantwortung trägt, ist der Befund.
3. **Skeptiker**: je Paket von höchstens acht Roh-Befunden ein Agent. Sein Ziel
   ist die Widerlegung: Test, Riegel, Registerzeile, Erreichbarkeit, Zusage.

Die **Sicherheitsspur** (Bereiche mit `spur: "sicherheit"`) läuft nie als
Claude-Agent: Codex `gpt-5.6-sol` max über `tools/pruefung/codex-audit-lauf.ps1`
nach dem Muster der Phase 12 (User 15.09.2026). Sammler inventarisieren
Sicherheitsstellen nur und reichen sie dorthin.

## 4. Ablauf eines Laufs

| Phase | Was | Wer | Abbruchregel |
|---|---|---|---|
| 0 Vorprüfung | `py -3.13 tools/pruefung/tiefenaudit_karte.py --selbsttest`, dann `--pruefen` | Werkzeug | alles außer `TIEFENAUDIT_VORPRUEFUNG: PASS` beendet den Lauf; erst die Karte heilen |
| 1 Stand | Stand festhalten (`git rev-parse HEAD`), eigene Arbeitskopie an diesem Stand (`git clone --local -c core.longpaths=true` an einen kurzen Pfad, kein `git worktree`); alle Agenten lesen dort | Hauptlauf | der sichtbare Checkout wird nie berührt; ein laufender Worker stört nicht und wird nicht gestört |
| 2 Aufträge | `--auftraege docs/audits/<Lauf>/auftraege --sha <Stand>` | Werkzeug | die gerechneten Aufträge sind Beleg des Laufs und werden mitcommittet |
| 3 Sammler | je Auftrag der Art `bereich` ein Agent, in Wellen parallel, jeder schreibt genau `roh/<ID>.md` | Agenten | ein Auftrag wird wörtlich übergeben, nie umformuliert oder gebündelt |
| 3.5 Beweis | `--berichte docs/audits/<Lauf>/roh` | Werkzeug | fehlt ein Fuß oder stimmt die Dateizahl nicht, läuft genau dieser Bereich neu; nie über eine Lücke hinweg urteilen |
| 4 Nähte | je Auftrag der Art `naht` ein Agent; danach erneut `--berichte` | Agenten | wie 3.5 |
| 5 Sicherheit | Sicherheitsspur als Codex-Phase, mit den Inventaren der Sammler als Eingang | Codex | nur nach dem Codex-Kontingent; sonst im Kopf als „nicht gelaufen" ausweisen |
| 6 Skeptiker | Roh-Befunde in Paketen durch `auftrag-skeptiker.md` | Agenten | nur BESTÄTIGT und HERABGESTUFT überleben; WIDERLEGT wandert in den Beweisteil |
| 7 Urteil | Einordnung je Überlebendem in DEFEKT, LÜCKE, HÄRTUNG am Gate-Wortlaut und an den Invarianten; Kennung `T<Lauf>-<Bereich>-<nn>`; `BEFUNDE.md` im Tabellenformat des Tiefenaudits 3 (ID, Schwere, Klasse, Kategorie, Ort, Befund, Beleg, Bricht, Vorschlag, Status) | Hauptlauf | kein Agent ordnet ein; Produktfragen werden Kartenentwürfe, nie direkte Fragen |
| 8 Kartennachtrag | bestätigte neue Kopplungen, verschobene Dateien, neue Fragen nach `karte.json`; die neue `BEFUNDE.md` wird Befundquelle; danach `--pruefen` | Hauptlauf | eine Kopplung ohne Beleg auf beiden Seiten kommt nicht in die Karte |
| 9 Übergabe | Commit nur der Laufdateien mit Pathspec, Push; der Dirigent liest nur den Kopf von `BEFUNDE.md` und macht Registerzeilen und Tickets (Muster NAK-246, NAK-309) | Hauptlauf, dann Dirigent | kein zweites Ledger, keine Punktwertung |

**Wo der Lauf stattfindet.** Nie im Dirigentenkontext. Der Dirigent startet
eine Prüfsession wie einen Worker (`KONZEPT.md` §5), deren erster Auftrag
`/tiefenaudit lauf` ist. Weil alle Agenten in der Arbeitskopie am festgehaltenen
Stand lesen und nur unter `docs/audits/<Lauf>/` schreiben, darf die Prüfsession
neben einem Worker laufen; geschrieben und gepusht wird aus der Arbeitskopie.

**Modelle und Parallelität (Startwerte, im ersten Lauf zu messen).** Hauptlauf,
Nahtprüfer und Skeptiker auf dem stärksten verfügbaren Modell; Sammler nach
`KONZEPT.md` §5 als Fan-out auf Sonnet, in Wellen von höchstens zehn. Der erste
Lauf misst, ob größere Wellen und ein stärkeres Sammlermodell den Ratenbegrenzer
halten; das Ergebnis steht danach hier, nicht im Skill.

## 5. Betriebsarten

| Aufruf | Wirkung | Kostenstufe (`KONZEPT.md` §8) |
|---|---|---|
| `/tiefenaudit` oder `/tiefenaudit stand` | Selbsttest, Vorprüfung, Befundzuordnung; kein Agent | 0 |
| `/tiefenaudit betroffen <basis>..<ziel>` | welche Bereiche und Nähte ein Diff berührt, welches Kopplungsende nicht mitgeändert wurde; kein Agent | 0 |
| `/tiefenaudit bereich P02 P14` | nur diese Sammler, danach ihre Skeptiker und das Urteil | 2 |
| `/tiefenaudit naht N01` | die Sammler der beteiligten Bereiche, dann dieser Nahtprüfer | 2 |
| `/tiefenaudit lauf` | alle Phasen | 3: je Phasengate, nie beiläufig |
| `/tiefenaudit nachtrag` | nur Phase 8 auf einem fertigen Lauf | 1 |

`betroffen` ist der tägliche Nutzen der Kopplungen: nach einem Ticket zeigt es
ohne Modell, welche Bereiche der Diff berührt hat und wo „x geändert, y nicht"
steht. Ein offenes Ende ist eine Frage, kein Fehler (NAK-309 ließ
`PipeToken.h` absichtlich unverändert); der Lesende entscheidet.

## 6. Wie die Karte wächst

- **Neuer Bereich oder Teilung:** wenn die Vorprüfung „ohne Bereich" oder
  „über dem Budget" meldet, oder wenn ein Sammler im Kartennachtrag zeigt, dass
  eine Datei fachlich woanders liegt. Geteilt wird nach Fach, nie nach
  Zeilenbereichen; Produktdateien und ihre Tests bleiben zusammen.
- **Neue Kopplung:** nur mit Beleg auf beiden Seiten (Datei:Zeile aus einem
  Bericht, einem Manifest oder `CLAUDE.md`) und einem Satz „warum". `stand` ist
  `belegt` oder `vermutet`; eine vermutete Kopplung wird im nächsten Lauf
  bestätigt oder gestrichen.
- **Neue Naht:** wenn `--befunde` ein Bereichspaar mit wiederkehrenden
  Befunden zeigt, das in keiner Naht liegt (so entstand N09 aus acht Befunden
  in P01 und P09), oder wenn ein Nahtprüfer eine Übergabe außerhalb aller Nähte
  meldet.
- **Neue Fragen:** aus bestätigten Befunden. Eine Frage, die in zwei Läufen
  nichts geliefert hat, wird umformuliert oder gestrichen (Totes-Element-Regel,
  `KONZEPT.md` §2 Regel 3).
- **Neuer Produktumfang** (S26–28, S28b, S29–31): mit dem Ticket, das die
  Dateien anlegt, denn die Vorprüfung reißt sonst. So deckt die Karte am Ende
  die ganze App.
- Jede Änderung der Karte endet mit `--pruefen` und wird mit der Karte
  zusammen committet.

## 7. Grenzen

Der Lauf liest, er führt nichts aus: keine Racefreiheit, keine Host- und
OS-Grenzen, keine Worst-Case-CPU, kein Laufzeitbeweis. Dafür stehen der Kanon
(`tools/beweise.ps1`), der Laufzeit-Arm (`tools/fl/laufzeit.ps1`) und die
Mutanten. Ein Bereichsbericht ohne Befund ist keine Gesundmessung, sondern die
Aussage „bei vollständiger Lektüre nichts gefunden". Die Zuordnung der Tests zu
den Bereichen beruht im Entwurf auf Dateinamen und dem Tiefenaudit 3; der erste
Lauf prüft sie.
