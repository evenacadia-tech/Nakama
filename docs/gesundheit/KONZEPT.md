# Nakama Prüfsystem — Konzept

**Status: abgenommen am 12.09.2026.** User-Wort in der Session `nakama-d8`
(Fable), 12.09.2026 nach 00:37, wörtlich: „alles klar leg los, das konzept
ist abgenommen von mir. sowohl eure kommunikation sowie das konzept.md ihr
braucht mich dazu jetzt nichtmehr, ich werde schlafen". Die Abnahme umfasst
das Konzept und den Kanal zwischen Sessions (§5.1). Bindend im Sinne von
`CLAUDE.md` wird sie mit der Registerzeile in `docs/offene-punkte.md`
(Wortlaut in Anhang A.1, Eintrag durch den Dirigenten in Schritt 0); bis
dahin ist dieses Dokument die Quelle des Zitats. Erarbeitet am
11./12.09.2026 lesend, parallel zum laufenden Bau (NAK-246 Etappe 6,
SONDE-015 D8); kein Produkt-, Test- oder Werkzeugcode ist berührt.

Über diesem Konzept stehen `CLAUDE.md` und der Dirigenten-Skill; über beiden
stehen Code und laufende Beweise. Das Konzept bindet nur die Prüfspur: was
geprüft wird, wann, von wem, wohin die Befunde gehen und wie die Prüfspur
selbst wächst. Es ist ein lebendes Dokument mit Änderungsprotokoll (§12);
jede Änderung trägt Datum und Anlass.

## Auftrag (User-Wortlaut)

11.09.2026: „DER BAU ist gerade voll im gange, ich würde gerne mit dir
parallel an der wartbarkeit arbeiten, verschiedene möglichkeiten wie wir
nakama durch skills und agenten abgesehen vom standard verfahren durch den
plan und dirigent überprüfen können. im sonifold projekt hab ich damals einen
check skill nach und nach erweitert, es gab verschiedene agentenskills um
spezifische probleme abzudecken."

12.09.2026: „ich möchte das nicht mal eben so nebenbei nach einem schnellen
durchgang blind einbauen. ich möchte dass du ein vollständiges konzept
ausarbeitest, das während des baues von nakama zum einsatz kommt, sich
konstant weiterentwickelt, verbessert, anpasst und fester teil des
dirigentenkonzeptes wird. ich bin kein coder, daher muss ich meine umgebung
so bauen und den dirigenten so ermächtigen um maximale qualität aus dem
nakama projekt rauszuholen. es soll nicht überladen werden mit zeremonie und
dadurch mehr token verbrennen als nutzen haben. es soll gezielt da nutzen wo
sonst fehler auftreten. wenn der implement plan in zukunft fertig ist, dürfen
wir nicht nackt darstehen sondern müssen auf hilfsmittel zurückgreifen können
die sich mit uns mit entwickelt haben im prozess."

Vier Forderungen daraus, gegen die jeder Absatz unten geprüft ist:

| Forderung | Was sie im Konzept erzwingt |
|---|---|
| während des Baus im Einsatz | jeder Baustein hat einen Platz im Dirigentenzyklus (§6), keiner wartet auf „nach dem Plan" |
| entwickelt sich mit, verbessert, passt sich an | Detektoren entstehen nur aus Befunden, tragen ihren Anlass und werden stillgelegt, wenn sie nie mehr rot werden (§2) |
| keine Zeremonie, kein Tokenverbrennen | vier Kostenstufen (§8); mechanische Tore laufen ohne Modell; ein Modell liest nur Rot; Audits laufen nie im Dirigentenkontext |
| nach dem Plan nicht nackt | der Pflegebetrieb als zweiter Betriebsmodus des Dirigenten mit derselben Mechanik (§7, §10) |

---

## 1. Was Nakama heute prüft, und was nicht

Gemessen am 11./12.09.2026 auf dem sichtbaren Checkout (HEAD `818673cd`).
Größe mit `scc` über `broker/`, `eq-copilot/plugin/`, `tools/`: rund
70 000 Codezeilen Rust, 62 000 C++ plus 13 000 in Headern, 17 600 Python
(Werkzeuge), 3 500 PowerShell. Das ist die Größenordnung von Sonifold, aber
anders geschnitten: zwei Sprachen, zwei Prozesse, ein Audio-Thread mit
Nulltest-Pflicht.

### 1.1 Vorhanden

| Fläche | Werkzeug | Was es fängt |
|---|---|---|
| Verhaltensbeweis je Ticket | `tools/beweise.ps1` (Kanon): 64 Namenseinträge, darunter 41 C++-Testprogramme, 22 Broker-Integrationstests, Python-Riegel, flatc-Drift, Sichtprüfung `EqCopShot` | die Instanzen, für die ein Test geschrieben wurde; baut selbst, verweigert Beglaubigung bei veralteten Binaries |
| Codebase- und Kontextmaße | `tools/plan/gesundheit.py` (Bein A32, Exit 4 bei Riss) | Zeilen je Datei, Funktionen über 200, Clippy-Ratsche, aufruferlose Helfer, Kommentar-Bezeichner, Bytes der Always-on-Fläche |
| C++-Statikanalyse (seit NAK-287, 12.09.2026) | `tools/plan/tidy.py` (Bein A33, Exit 4 bei Riss; Regelsatz `eq-copilot/plugin/.clang-tidy`, eigener Ninja-Baum `eq-copilot/build-tidy`) | clang-tidy über alle Übersetzungseinheiten des Plugins ohne Tests und ohne den flatc-Codegen: Lebensdauer, Aliasing, nicht initialisierte Werte, unbeabsichtigte Kopien, thread-unsichere Bibliotheksaufrufe, Objektschneiden — eindeutige Fundstellen als Ratsche, Ziel 0 |
| Plandokumente | `tools/plan/dokuriegel.py`, `planstand.py`, `rundenbilanz.py` | zerrissene Tabellen, Verweise ins Leere, Status als Messwert, Produktfortschritt je Runde |
| Ticketprüfung | Dirigent §3.4: Codex `gpt-6-astra` lesend, gebundene Vorlage, Validator-Agent je Befund, Verhaltensmatrix, drei Runden, Konvergenzentscheid | Defekte gegen Gate-Text, Matrix und Invarianten, im Ticketbereich |
| Befundklassen als Prosa | `tools/dirigent/pruefliste.md` A–F (40 bestätigte Befunde aus S8–S15) | nur, wenn der Bauer sie liest und der Prüfer sie kennt |
| Riegel und Erinnerungen | `tools/hooks/`: Git-Riegel, Kreativschleuse, Fremdmodell-Riegel, Erinnerung für Plugin- und Schema-Pfade, Planstand-Hook | Destruktives, ungefragte Designartefakte, vergessene Realtime-Regeln |
| Dritte Spur | externer Codex-Audit auf Wunsch des Users (`docs/audits/`, Muster NAK-246); an G6–G9 die Torläufe nach §6.6 (Dirigent §3.7; Ultra-Review seit 12.09.2026 überholt, Karte U39) | was Ticketprüfung und Kanon gemeinsam übersehen |

### 1.2 Nicht vorhanden

- **Kein Clippy im Kanon.** `broker/Cargo.toml` sagt es selbst: „tools/beweise.ps1 faehrt in keinem Bein Clippy". Nur `gesundheit.py --clippy` zählt Fundstellen als Ratsche.
- **Keine Supply-Chain-Prüfung** des Brokers (`cargo deny`, `cargo audit`, `osv-scanner`), kein Geheimnis-Scan, kein Tippfehler-Scan. Alle vier Werkzeuge sind auf diesem Rechner installiert und ungenutzt.
- **Keine Mutationstests, keine Abdeckungsmessung** in keiner Sprache. Ob ein Golden eine Änderung im Messkern bemerkt, ist ungemessen.
- **Kein Klassen-Detektor** für die Klassen, die am häufigsten wiederkamen (§3). Sie leben als Prosa.
- **Kein statischer Blick auf den Audio-Thread.** Der Nulltest beweist Bitgleichheit; eine Sperre oder Allokation, die Samples nicht verändert, besteht ihn. Seit NAK-287 (12.09.2026) sieht clang-tidy (Bein A33) Lebensdauer, Aliasing und thread-unsichere Bibliotheksaufrufe im ganzen Plugin; die Echtzeitregel selbst (Sperren, Allokationen, Datei- und Pipe-Zugriffe im Audio-Pfad) kennt kein clang-tidy-Check — das bleibt das Tor Echtzeit-Hygiene (§4.1).
- **Kein Rundgang über den ganzen Baum.** Geprüft wird, was ein Ticket berührt; Drift außerhalb des Ticketbereichs findet nur ein externer Audit.
- **Keine Karte**, welche Zusage welchen Detektor besitzt. Ohne sie entsteht das nächste Prüfwerkzeug aus einem Gefühl.
- **Ein Riss ohne Zeile.** `gesundheit.py` meldete am 12.09.2026 drei gerissene Grenzen (Funktionen über 200 Zeilen, Kommentar-Bezeichner 32 von 30, Dirigenten-Skill 38 448 von 36 864 Bytes). Für die ersten beiden stehen NAK-235, NAK-236 und NAK-255 im Register; eine Zeile, die den Skill-Riss nennt, fand `grep` nicht. Der Kreis „Riss → Registerzeile" schließt sich also nur, wenn jemand daran denkt. Genau das schließt der Prüfgang (§4.1, Wache W-Riss).

---

## 2. Grundsatz: Detektoren wachsen aus Befunden, nirgends sonst

Sonifolds Lehre in einem Satz: Ein Test beweist die Instanz, für die er
geschrieben wurde; ein **Klassen-Detektor** misst die *nächste* Instanz, die
eine künftige Änderung einführt. Sonifold hat zuletzt für die Klassen mit
belegter Wiederkehr Detektoren gebaut und die Klassen ohne Besitzer in einer
Karte sichtbar gehalten, samt der ehrlichen Restliste ohne Besitzer. Diese
Idee wird übernommen. Die Träger werden geändert (§9), weil Sonifolds Träger
zur größten Wartungslast des Systems wurden.

Fünf Regeln, die verhindern, dass daraus Zeremonie wird:

1. **Zweitbefund-Regel.** Ein Detektor entsteht, wenn dieselbe Befundklasse
   zum zweiten Mal bestätigt ist: durch ein Codex-Urteil, einen externen
   Audit, ein rotes Bein oder einen Riss. Der erste Befund wird behoben und
   klassifiziert (§6.4), sonst nichts. Ausnahme: die tragenden Invarianten
   aus `CLAUDE.md` (Grundgesetz Nulltest, Audio echtzeitfest, Identität
   eingefroren, State verlustfrei, Schemas als Verträge, Musikzeit
   deterministisch, NaN-Ehrlichkeit) dürfen eine Wache ohne Zweitbefund
   bekommen, weil ihr Bruch Produkt oder Daten zerstört.
2. **Anlass und Rotbeweis.** Jeder Detektor trägt im Kopf die Befund-IDs,
   die ihn verlangt haben, und wurde einmal absichtlich gebrochen: am
   historischen Stand vor dem Fix des auslösenden Befunds muss er rot sein
   (`git archive <sha>` in ein Temp-Verzeichnis, Detektor darauf fahren).
   Ein Detektor, der auch ohne den Fix grün wäre, ist kein Detektor.
3. **Totes-Element-Regel.** Ein Detektor, der über zwei Pflegeschritte hinweg
   nie rot war und dessen Klasse nicht wiederkam, wird im Änderungsprotokoll
   stillgelegt (nicht gelöscht). Ein neuer Befund seiner Klasse belebt ihn.
   Dieselbe Regel, die `gesundheit.py` für seine Maße hat: „Ein Maass, das
   immer gruen ist, waere ein totes Element."
4. **Ein Ledger.** Befunde landen im Register `docs/offene-punkte.md` mit den
   bestehenden Klassen. Es gibt keine zweite Befundbasis mit eigener
   Punktwertung. Berichte der Audits sind Beweisdateien, keine Ledger.
5. **Ohne Modell, wo es geht.** Mechanische Tore sind Python ohne
   Abhängigkeiten und laufen ohne Claude. Ein Modell liest nur, was rot ist.
   Audits, die Urteil brauchen, laufen als eigene Prüfsession (§5), nie im
   Dirigentenkontext.

---

## 3. Wo Fehler bisher auftraten

Die Befundklassen K1 bis K8 sind das Klassenregister dieses Konzepts. Sie
sind nicht erfunden, sondern aus drei Quellen gerechnet: der Prüfliste A–F
(40 bestätigte Befunde aus S8, S9, S9b, S14–15), den zehn Befunden des
externen Audits vom 10.09.2026 (`docs/audits/2026-09-10-code-review/AUDIT.md`,
D1–D10) und den Klassenmarken des Registers (gezählt 11.09.2026 über offene
und geschlossene Zeilen: 47-mal [Härtung/Struktur], 35-mal [Werkzeug]).

| Klasse | Inhalt | Belege | Detektor heute | Stärke |
|---|---|---|---|---|
| **K1 Rückstau und Ordnung** | Politik bei voll je P0/P1/P2, Abfluss ohne Reconnect, Schlüssel überleben Puffer, Lesepfad nie hinter „erst alles senden" | Prüfliste A; D5 (Zustellung nach Commit ungeordnet), D9 (Rücknahme teilt Schlüssel mit Vollsnapshot); NAK-165 | QueueStressTest, `pruefe_ipc_last.py`; Prosa | ◐ Beispiel |
| **K2 Lebenslauf und Besitz** | Reihenfolge verbinden/trennen, Join-Fristen, Close-Flag vor Inhalt, Callback-Besitz, Registrierung im Stopp-Fenster | Prüfliste B; D1 (Slotbesitz), D2 (IPC-Laufzeit überlebt Besitzer), D3 (ACK am Editor-Timer); NAK-184 | LebenslaufTest, PipeClientLifecycleTest, `lebenslauf.rs`; Prosa | ◐ Beispiel |
| **K3 Vertrag und Zahlenränder** | Längen im Leser, exakte Feldmenge beidseitig, Discriminator vor Inhalt, NaN/Inf an jeder Zahl über den Draht, Bereichsprüfung vor Konvertierung | Prüfliste C; D7 (Stereokanäle als Messdauer, schließt NAK-159 mit), D8 (Bereich nach Konvertierung); SONDE-013 P4 | Broker: generische Schema-Engine für alle Grenzen (`broker/src/vertrag.rs`); Plugin: geschlossene Feldmengen und Enums, Zahlengrenzen handgeschrieben je Leser; SchemaTest, `contract_cross_language.rs`, `pruefe_v3_vertrag.py` (Metaschema, Fixture-Deckung, Fassungsschritt), flatc-Drift | Rust ✔ · C++ ◐ |
| **K4 Zustand und Paarung** | Save↔Load, Migration je Version, unbekannte Major read-only mit Originalbytes, Dirty-State an Host, Beziehungspaare im selben Änderungssatz | `CLAUDE.md` State-Invariante, Prüfliste F; D6 (Reload leert Sitzungszustand nicht) | StateMigrationTest (133 von 136 Feldern mit eingefrorenem Fixture, unbekannte Major read-only mit Test), Fixtures unter `eq-copilot/fixtures/state/`, ProjectReloadTest, TransactionTest; Dirty-State an allen Rufstellen belegt; alle fünf Paare beidseitig mit Test | ◐ dicht |
| **K5 Behauptung ≤ Messung, Werkzeug-Ehrlichkeit** | Runner bezeugt nur, was er baut; Frische der Eingaben; jede Prüfung einmal gebrochen; Zahlen gemessen, nicht abgeschrieben; grüner Lauf auf altem Binary | Prüfliste D und E; D4 (false und spätere Ausführung), D10 (vier Slots kein Vier-Bank-Beweis); NAK-93, NAK-94, NAK-230 (fünf Fehler als grüner Lauf), N-29 (cmake fehlt in Bash); 35 Registerzeilen [Werkzeug] | Frischeprüfung im Runner, Stillgelegt-Marke; Prosa | ◐ Beispiel |
| **K6 Echtzeit** | keine Sperre, Allokation, Datei, Pipe, Netz, kein Logging auf dem Audio-Thread; Überlast verwirft Frames, nie Audio | `CLAUDE.md` Grundgesetz; kein bestätigter Befund bisher | `RtWache` am EQ-Kern (Sperren, Allokation, Wanduhr je Callback; `eq-copilot/plugin/dsp/DspRtWache.h`, gemessen in DspGoldenTest); Allokationszähler in QueueStress für `processBlock`; Nulltest (Bitgleichheit); kein statischer Blick, Messpfad ohne Wache | ◐ (EQ-Kern ✔) |
| **K7 Größe und Kontext** | Dateien über 2 000 Zeilen, Funktionen über 200, Clippy-Fundstellen, tote Helfer, Kommentar-Bezeichner, Always-on-Bytes | NAK-223, NAK-235, NAK-236, NAK-255; Skill-Riss 12.09.2026 ohne Zeile | `gesundheit.py` | ✔ Detektor |
| **K8 Bedienehrlichkeit** | keine toten Elemente (User-Gesetz 24.08.2026), Tasten sind Material (25.08.2026), Zustand ehrlich gemeldet | Skizzenbelege; Oberfläche im Funktionsneustart, noch kein Befund | Belege in `design/skizze/belege/`; Gesetze | ○ Prosa |

Lesart der Stärke: ✔ Detektor misst die nächste Instanz · ◐ Beispieltests
und Prosa fangen die bekannten · ○ nur Prosa. Verifiziert am 12.09.2026 in
`docs/gesundheit/abdeckungskarte.md` (fünf Leser, drei Skeptiker, 11 von 19
gemeldeten Lücken widerlegt); die Karte ist ab jetzt die Wahrheit dieser
Tabelle, die Tabelle nur ihre Zusammenfassung.

Die vier Klassen mit belegter Rückfallquote sind K1 bis K4. Acht der zehn
Audit-Befunde vom 10.09.2026 fallen in genau diese vier, obwohl die Prüfliste
sie seit dem 30.08.2026 als Klassen nennt. Prosa allein hat sie nicht
gehalten. Das ist die Evidenz, aus der dieses Konzept seine Reihenfolge zieht
(§10): zuerst Detektoren für K1 bis K4, alles andere danach oder nie.

---

## 4. Bausteine

Jeder Baustein beantwortet eine Frage, schreibt an einen festen Ort, läuft zu
einem festen Anlass und hat eine Kostenstufe (§8). Kein Baustein ändert
Produktcode; Änderungen entstehen nur über Tickets im Dirigenten.

### 4.1 Prüfgang `/pruefen` — Stufe 0

**Frage:** Was ist am Baum falsch, ohne dass ein Ticket es berührt hat?

**Träger.** Ein neues Verzeichnis `pruefung/` unter `tools/` mit einem Modul
je Tor, nur Standardbibliothek, nach dem Muster von `gesundheit.py`: jedes Tor hat
`--selbsttest`, einen Kopf mit Anlass (Befund-IDs) und Rotbeweis (SHA, an dem
es rot war), und eine Klasse **GRENZE** (reißt, Exit 4), **ZIEL** (wird
genannt, Exit 0) oder **HINWEIS** (Liste zur Sichtung). Ein Läufer
`pruefgang.py` fährt alle Tore, sammelt alles, bricht nie beim ersten Rot ab
und schreibt eine Tabelle. Das Skill `/pruefen` ist dünn: es ruft den Läufer,
liest nur die roten Zeilen und ordnet sie ein.

**Tore der ersten Fassung** (alle ohne Urteilsbedarf; Anlass in Klammern):

| Tor | Klasse | Misst | Anlass |
|---|---|---|---|
| `gesundheit.py` | GRENZE | wie heute; wird erstes Tor, bleibt Bein A32 | NAK-223 |
| `tidy.py` | GRENZE | wie heute: clang-tidy-Ratsche über das Plugin, bleibt Bein A33 (Minuten, weil der Ninja-Baum konfiguriert und jede Einheit geparst wird — im Prüfgang vor einem Ticket deshalb nur mit `--nur` auf die Ticketpfade) | NAK-287 |
| Bau und Clippy | GRENZE | `cargo clippy --all-targets` im Broker mit `-D warnings` auf der Ratsche aus `gesundheit.py`; CMake-Konfiguration des Plugins ohne Bau | `broker/Cargo.toml` (Clippy nie im Kanon) |
| Supply-Chain | GRENZE | `cargo deny check`, `osv-scanner --lockfile broker/Cargo.lock` | K5, Werkzeuge installiert und ungenutzt |
| Geheimnisse und Tippfehler | GRENZE / HINWEIS | `gitleaks protect --staged`, `typos` mit Ausnahmeliste | Steam-/Signatur-Lehre aus Sonifold |
| Vakuum-Wachen | GRENZE | Fixture-Ordner nicht leer, Golden-Zähler über Boden, jedes `EqCop*`-CMake-Ziel ist Bein oder stillgelegt, jeder `broker/tests/*.rs` läuft in einem Bein, Fuzz-Korpus nicht leer | K5 (NAK-93: „Was der Kanon nicht baut, darf er nicht bezeugen") |
| W-Riss | GRENZE | jeder aktuelle GRENZE-Riss der Codebase-Maße von `gesundheit.py` hat eine Registerzeile, die das Maß nennt; ein Riss der Kontextfläche (`MEMORY.md`, `CLAUDE.md`, Dirigenten-Skill) wird nicht registriert, sondern im Abschlussfenster sofort behoben (User 12.09.2026, `CLAUDE.md` Arbeitsregeln) und gilt bis dahin als offener Riss | §1.2 (Skill-Riss 12.09.2026) |
| Volatile Zahlen | GRENZE | `CLAUDE.md` enthält keine Zahl vor „Tests", „Beine", „Fixtures", „Capabilities" (Regel aus `CLAUDE.md` „Arbeitsregeln", bisher ohne Riegel) | Kontexthygiene-Playbook Phase B |
| Beziehungspaare im Diff | HINWEIS | `git diff <basis>...HEAD`: berührt eine Seite eines Paares (speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg) ohne die andere Seite im selben Satz | K4, Prüfliste F, D6 |
| NaN-Wachen | HINWEIS | Parse-Stellen von Gleitkommazahlen an Pipe- und Schemagrenzen ohne Endlichkeitsprüfung in Sichtweite | K3, D8 |
| Echtzeit-Hygiene | HINWEIS | verbotene Symbole (Sperren, `new`/`malloc`, Datei-, Pipe-, Netz-, Log-Aufrufe) in den Quelldateien des Audio-Pfads; Kalibrierung durch das Echtzeit-Audit (§4.3), danach GRENZE | K6, Invariante |
| Duplikate | ZIEL | `jscpd` über Rust und C++, Schwelle aus der ersten Messung | K7 |

**Wohin.** Die Datei `pruefung.md` unter `docs/gesundheit/`, kanonischer
Name, bei jedem Lauf überschrieben; Git-Historie ist das Archiv. Jeder
GRENZE-Riss eines Codebase-Maßes wird eine datierte Registerzeile
[Werkzeug] oder [Planarbeit · Pflegeschritt], wie es NAK-223 für
`gesundheit.py` festlegt; die Wache W-Riss prüft, dass sie steht. Ein Riss
der Kontextfläche wird nicht registriert, sondern sofort behoben (User
12.09.2026).

**Wann.** Dirigent §3.1 vor jedem Ticket (Minuten, kein Kanon); als nicht
blockierendes Kanon-Bein neben A32; nächtlich durch die Windows-Aufgabenplanung
ohne Claude (der Läufer schreibt nur die Datei; ein Rot liest die nächste
Dirigentensession in §3.1).

**Rotbeweis je Tor.** Vor der Aufnahme in den Läufer läuft jedes Tor gegen
den Stand vor dem Fix seines Anlasses und ist dort rot; die Rohausgabe steht
im Manifest des Pflegetickets. Ein Tor ohne Rotbeweis wird nicht aufgenommen.

### 4.2 Abdeckungskarte `docs/gesundheit/abdeckungskarte.md` — Stufe 2, einmalig, dann Stufe 0

**Frage:** Welche Zusage hat einen Detektor, welche nur Prosa, welche
niemanden?

Erste Fassung am 12.09.2026 als Prüfsession in `nakama-d8` erstellt
(Schritt 1 erledigt). Die Tabelle aus §3, verifiziert und fortgeschrieben: je Zusage die Besitzer
aus Kanon, Tests, Hooks, Riegeln und Prosa, mit `Datei:Zeile` oder Beinname;
je Klasse die Befund-IDs und ihr Zähler. Erstellung als Prüfsession (§5):
Leseagenten sammeln die Besitzer je Zeile, Skeptiker versuchen jede Zeile
„kein Besitzer" zu widerlegen (den Test finden, der es doch fängt), das
Urteil fällt im Hauptlauf der Session. Danach ist die Karte Stufe 0: sie
ändert sich nur, wenn ein Detektor dazukommt oder stillgelegt wird, und das
trägt der Dirigent im Abschlussfenster nach (§6.5).

Sonifold hat diese Karte erst am 05.06.2026 gebaut, nachdem alle anderen
Werkzeuge standen, und daraus das nächste Audit abgeleitet. Nakama beginnt
mit ihr, weil sie entscheidet, was gebaut wird, und weil sie die
Prosa-Klassen sichtbar macht, bevor der nächste externe Audit sie findet.

### 4.3 Spezialaudits — Stufe 2

**Frage, immer dieselbe:** Würde die nächste Änderung diese Zusage still
brechen?

Gemeinsame Mechanik, übernommen aus `/save-fidelity` und `/logic-audit`,
weil sie Prozessregeln sind und keine Modellkrücken:

- **Audit-only.** Die Session ändert keinen Code. Ergebnis ist ein Bericht
  und Registerzeilen.
- **Population, nicht Stichprobe.** Agenten zählen die ganze Population
  eines Musters (jede Queue, jedes Feld, jeden Aufruf) und liefern
  `Datei:Zeile`; „ein paar Beispiele" ist kein Bericht. Jeder Bericht endet
  mit einem Scope-Beweis: was gelesen wurde, was nicht, ob Pfade fehlten.
- **Urteil im Hauptlauf.** Agenten sammeln mechanisch; die Einordnung fällt
  im Hauptlauf der Prüfsession, nie in einem Agenten.
- **Skeptiker je Befund.** Ein zweiter Agent sucht den Test, den Riegel, die
  Registerzeile, die den Befund doch abdeckt. Nur Überlebende werden
  Zeilen. Sonifold maß rund ein Viertel falsche Befunde; ohne diesen Pass
  wären sie Arbeit geworden.
- **Klassen wie im Dirigenten.** Ein Befund gegen Gate-Text, Matrix, Test
  oder `CLAUDE.md`-Invariante ist **Defekt** und wird ein Ticket nach dem
  Muster NAK-246 (Quellvalidierung, Matrix, Etappen). Alles andere ist
  **Härtung** und wird Registerzeile [Härtung/Struktur] mit K-Kennung.
  Produktfragen, die dabei entstehen, werden Karten in
  `docs/plan/fragen.json`, nie direkte Fragen.
- **Bericht.** `docs/gesundheit/<audit>.md`, kanonisch, überschrieben; Kopf
  mit Urteil, Befundliste und Klassenzählern; darunter die Populationen mit
  Evidenz, damit der nächste Lauf gegen den vorigen diffen kann.

Die Audits, in der Reihenfolge ihres Bedarfs (§3):

| Audit | Klasse | Was es enumeriert | Wann |
|---|---|---|---|
| `/lebenslauf-audit` | K1, K2 | jede Queue, jeden Kanal, jeden Thread, jeden Join, jeden Callback in `broker/src/`, `eq-copilot/plugin/core/ipc/`, `PipeClient`; je Objekt die Matrix aus Prüfliste A und B: Politik bei voll, Abfluss ohne Reconnect, Close-Flag vor Inhalt, Join-Frist, Besitz des Callbacks, Test in beiden Sprachen. Eine leere Zelle ist der Befund. | erster Lauf nach der Karte; danach nach jedem Ticket, das Transport, Queues oder Lebenslauf berührt |
| `/zustandstreue` | K4 | vier Gänge nach `/save-fidelity`: Parität (jedes persistente Feld ↔ Schreiber ↔ Leser ↔ Migration ↔ Fixture), Signal (jede persistente Änderung meldet Dirty-State), Migration (jede Version hat ein eingefrorenes Fixture; unbekannte Major bleibt read-only mit Originalbytes), Lebenszyklus (Schreiben unter Abbruch; Broker hat `store_crash_matrix.rs`, Plugin-Seite zu prüfen) | vor S26–28, weil Layout v2, Preset-Objekt und `state_report.dsp` neue Felder bringen; danach nach jedem Ticket, das `eq-copilot/plugin/state/` oder `eq-copilot/schemas/state/` berührt |
| `/vertragstreue` | K3 | jedes Feld der v3-Schemas: C++-Leser und Rust-Leser mit derselben Grenze, Negativfixture je Feld, Discriminator und Familie vor Inhalt, DTO ↔ Schema, flatc-Stand, Bereichsprüfung vor Konvertierung. `pruefe_v3_vertrag.py` prüft heute das Schema und den Fassungsschritt; das Audit prüft die Leser. | mit `/zustandstreue`; danach nach jedem Ticket, das `eq-copilot/schemas/` berührt |
| `/echtzeit-audit` | K6 | statischer Aufrufgraph ab `processBlock` und den Analyse-Einstiegen: Sperren, Allokationen, Datei-, Pipe- und Netzzugriffe, Logging; Ergebnis kalibriert das Tor Echtzeit-Hygiene (Ausnahmeliste, dann GRENZE). Der Codebase-Graph (`codebase-memory`, für Nakama indiziert) dient der Erkundung; das Tor selbst braucht einen eigenen deterministischen Läufer ohne MCP. | nach dem Prüfgang; danach nach jedem Ticket unter `eq-copilot/plugin/src/prozessor/` oder `core/analysis/` |
| `/zwecktreue` | Produktmodell | nach `/logic-audit`: jede v3-Nachricht, jedes persistente Feld, jeder Parameter, jede Capability gegen Wahrheitskern, Blueprint und Entwurf 0.5; Klassen vestigial, verwaist, Modellwiderspruch, Duplikat, unehrliche Fläche. Erklärt außer Frage: Legacy-Bezeichner bis NAK-30, geparkte Studien. Empfiehlt Rückbau, entscheidet nie. | einmal vor S29–31 (Fernsteuerung), danach je Phasengate |
| `/bedienehrlichkeit` | K8 | nach `/compositor-ui` auf die zwei User-Gesetze: jedes sichtbare Element führt einen Handgriff aus oder meldet einen Zustand; kein Zustand ändert Maße (gemessen am Layoutrechteck, `offsetWidth`/`offsetHeight`); dazu stille Pipe-Fehler ohne Anzeige, veraltete Anzeige, Aktion ohne Rücknahme. Belege mit `EqCopShot` beziehungsweise Playwright auf der Skizze. | Zeitpunkt ist Produktentscheid (§11): jetzt auf der Skizze oder nach S26–28 auf der gebauten Oberfläche |
| `/tiefenaudit` | alle | nach `/deep-audit`, selten: acht Linsen (Grenze Plugin/Broker, Korrektheit und Zahlenränder, Audio-Pfad Bus → Sonde → Broker → Gen, Vertrag, Zustand, Echtzeit und Latenz, Bedienung gegen den Blueprint, Beinlücken: „welcher Bug käme an allen Beinen vorbei?"). Ausgabe: fünf bis zehn Einsichten und ein Planvorschlag, kein Code. | je Phasengate als erster Torschritt (Dirigent §3.7; User 12.09.2026, Karte U39) |

### 4.4 Mutanten `/mutanten` — Stufe 0 (Rechenzeit), Stufe 1 (Lesen)

**Frage:** Merken die Tests, wenn der Code sich ändert?

`cargo-mutants` (installiert) über den Broker. Ratsche je Crate in einem
Unterordner `mutanten/` unter `docs/gesundheit/` mit Tötungsrate,
Überlebenden-Hash und Hochzieh-Protokoll, nach dem Muster von `/aftercheck`: sinkt die Rate, ist der
Lauf rot; steigt sie, wird die Marke in einem eigenen Schritt hochgezogen;
ändert sich der Hash bei gleicher Rate, ist eine andere Lücke entstanden.
Diff-Modus (`--in-diff`) je Ticket als Kanon-Bein mit Zeitlimit; Vollmodus je
Pflegeschritt, abgekoppelt, wie der Kanon (`--in-place` unter Windows, weil
NTFS keine Reflinks kann und die Kopie je Mutant sonst dominiert).

**C++.** Für MSVC gibt es kein reifes Werkzeug; `mull` braucht clang. Vorschlag
als **Experiment mit ehrlichem Ergebnis**: ein eigener Operator-Mutator (Python)
auf `eq-copilot/plugin/core/analysis/`, Richter sind die Golden-Beine. Das
Manifest darf „nicht tragfähig" heißen; dann bleibt es bei Rust.

Überlebende, die kein dokumentiertes Äquivalent sind, werden Registerzeilen
[Härtung · Test] mit Klasse; ein Überlebender in einem Pfad mit Invariante
(Nulltest, State, Identität) ist ein Defekt.

### 4.5 Wächter (neue Agentendateien unter `.claude/`, Ordner `agents/`) — Stufe 1

**Frage:** Hat diese Änderung die heikle Grenze sauber gehalten?

Vier kleine lesende Agenten auf Sonnet, mechanisch, mit fester Checkliste je
Klasse und dem Berichtsformat „Tabelle Objekt · Zusage · gemessen an":

| Agent | Klasse | Pfade, bei denen er fällig ist | Prüft |
|---|---|---|---|
| `vertrags-waechter` | K3 | `eq-copilot/schemas/`, `broker/src/dto.rs`, `broker/src/vertrag.rs`, `core/ipc/` | Feld beidseitig, Länge im Leser, Discriminator vor Inhalt, Negativfixture, flatc frisch |
| `echtzeit-waechter` | K6 | `eq-copilot/plugin/src/prozessor/`, `core/analysis/`, `PluginProcessor.*` | verbotene Aufrufe im Audio-Pfad, Allokation in `processBlock`-Reichweite, Logging |
| `zustands-waechter` | K4 | `eq-copilot/plugin/state/`, `eq-copilot/schemas/state/`, `fixtures/state/` | Schreiber ↔ Leser ↔ Migration ↔ Fixture ↔ Dirty-State im selben Satz |
| `beziehungs-waechter` | K1, K2, K4 | jeder Änderungssatz mit Lebenslauf-, Queue- oder Paar-Berührung | die Paare aus `CLAUDE.md`, Prüfliste A und B als Zeilenliste |

Der bestehende Hook `tools/hooks/agent-reminder-nakama.sh` nennt bei Treffer
den fälligen Wächter (eine Zeile, nicht blockierend). Der Ticketauftrag des
Dirigenten (§3.2) verlangt: vor dem Commit den fälligen Wächter dispatchen und
sein Ergebnis im Manifest unter „eigene Ticketproben" nennen.

**Regel, damit daraus keine Zeremonie wird.** Wächterbefunde sind Selbstaudit
des Bauers, keine Anforderungsquelle. Dirigent §3.4 bleibt unverändert:
Nacharbeit nur aus bestätigten Defekten des Prüfers. Der Prüfer liest den
Wächterbericht als Kontext, wie die Prüfliste.

### 4.6 Laufzeit-Arm — Stufe 2, seit 12.09.2026 fester Bestandteil jedes lohnenden Tickets

**Frage:** Kommt das, was die Sonde misst, wirklich in Gen an — im echten
Host, mit dem installierten Bau?

**Stand 12.09.2026.** Der MCP-Server (`C:\Users\phili\Projekte\fl-studio-mcp`:
loopMIDI → Controller-Skript in FL → JSON-Dateien) ist gemessen und
erreichbar, sobald loopMIDI vor FL läuft; das Diagnoseprojekt liegt vor
(`eq-copilot/fixtures/fl/Nakama-Diagnose.flp`, Aufbau gemessen in
`eq-copilot/fixtures/fl/LIES-MICH.md`); die Installation läuft ohne
Handgriff über die erhöhten Aufgaben `\Nakama\*` (NAK-285). Der Arm ist der
Nachfolger der Handmessungen aus Termin A und B. Sein Bau ist Plan-Schritt
S25e (Register NAK-286) in vier Schichten: Bereitschaft selbstheilend
(loopMIDI, Restprozesse, Manifest-Hashes, Installation, FL-Start mit Projekt,
Ping), MCP-Härtung (nur loopMIDI-Port, Ping, Pegel, Fenster, Screenshot,
berichtigte Modi), Diagnose-Briefkasten in Plugin und Broker (Snapshot und
Aggregat ohne Klick, nur auf Anfragedatei, keine Host-Parameter) und
Szenarien als JSON unter `docs/gesundheit/szenarien/` mit dem Runner
`tools/fl/szenario.py` und dem Ablauf `tools/fl/laufzeit.ps1` (erste Fassung
12.09.2026, Bereitschaftsszenario gemessen). Der Telemetrie-Leser auf dem
Pipenamen bleibt Stufe 2 dahinter, weil er eine Vertragsfläche ist.

**Kadenz.** Je Ticket nach grünem Kanon (Dirigenten-Skill §3.3); ob der
Lauf lohnt, entscheidet der Runner am Diff Basis..HEAD (`eq-copilot/plugin/`,
`eq-copilot/schemas/`, `broker/src/`, `eq-copilot/install/`), nie der
Dirigent. Dazu der Volllauf im nächtlichen Prüfgang (§10 Schritt 2). Ein
verfehltes Szenario ist Nacharbeit (§6). Kein User-Handgriff außer dem
Neuanlegen des Projekts nach NAK-30.

**User-Wort 12.09.2026 (Session `nakama-a7`), wörtlich:** „ich möchte dass du alles dafür tust was nötig ist, dass das fester bestandteil wird, der automatisch ohne nachfragen absolviert wird immer dann wenn es lohnenswert ist. das sollte natürlicher bestandteil sein beim bauen eines plugins. das ist ja auch normaler bestandteil einer jeden softwareentwicklung. du kannst das alles erledigen und dem dirigenten mitteilen, dafür brauchst du mich nicht". Folge: Skill §3.3, §3.5, §4 und §6, Plan S25e, diese Fassung.

**User-Wort 12.09.2026 (Karte U40, Dirigentensession), wörtlich:** „wir haben ein fl studio mcp. du kannst selbst messungen und alles was benötigt wird vornehmen. der mcp server kann auch weiter ausgebaut werden oder ich kann eine diagnose fl studio instanz vorbereiten die immer offen ist. der komplette bau kann sowieso viel mehr direkt mit fl studio interagieren. alle tools sind da oder können erstellt werden“.
Folge (Dirigent): Produktwirkungen, die nur hörbar oder messbar entscheidbar
sind (zuerst U40: Aktivitätsgate nach F06, NAK-283 Etappe 5), misst der
Dirigent selbst — headless auf echtem Material und über den FL-MCP-Server mit
dem installierten Bau — und legt dem User Messwerte vor, keine Blindwahl. Der
Laufzeit-Arm rückt damit vor (Register NAK-284): Szenarien je Karte unter
`docs/gesundheit/szenarien/` (anzulegen), Telemetrie-Leser über den
Probe-Pipenamen, MCP-Erweiterungen nach Bedarf. Das Diagnoseprojekt hat der
User am 12.09.2026 um 15:44 Uhr angelegt; die Instanz startet der Runner
selbst.

---

## 5. Prüfsessions: wie Audits laufen, ohne den Dirigenten zu belasten

Der Dirigent baut nie selbst; analog **auditiert er nie selbst**. Ein Audit
ist eine eigene Session, die der Dirigent wie einen Worker startet und deren
Ergebnis er am Repo liest:

```powershell
claude "<Prüfauftrag aus tools/pruefung/auftrag-<audit>.md, Platzhalter gefüllt>" `
  --model opus --effort max --permission-mode dontAsk `
  --name "nakama-pruef-<audit>-<basis-kurz>" `
  --allowed-tools "<eng: Read, Grep, Glob, Bash(lesende Kommandos), Agent, Write(docs/gesundheit/**)>" `
  --bg
```

- **Auftrag aus Vorlage**, nie frei formuliert: je Audit eine Datei
  `tools/pruefung/auftrag-<audit>.md` mit Scope, Population, Berichtsformat,
  Skeptikerpass, Ausschlüssen (Register-Härtungen, NAK-30, geparkte Studien)
  und dem Satz, dass die Session keinen Code ändert.
- **Ein Schreiber.** Die Prüfsession schreibt nur unter `docs/gesundheit/`
  und nur, wenn kein Worker läuft. Sie läuft deshalb zwischen zwei Tickets:
  nach dem Abschlussfenster (§3.5), vor dem nächsten §3.1. Läuft ein Worker,
  wartet das Audit; es wird nie parallel gestartet.
- **Hauptlauf Opus, Fan-out Sonnet.** Das Urteil fällt im Hauptlauf; die
  Populationen sammeln Sonnet-Agenten über das Workflow-Werkzeug (Fan-out
  plus Skeptikerpass als Pipeline). Sonifold maß im Juni 2026, dass mehr als
  sechs parallele Opus-Agenten den Ratenbegrenzer reißen; die Fan-out-Ebene
  bleibt deshalb Sonnet.
- **Der Dirigent liest nur den Kopf**: Urteil, Befundliste mit K-Kennung,
  Klassenzähler. Die Populationen unter dem Kopf liest er nicht. Aus dem
  Kopf macht er Registerzeilen und, bei Defekten, ein Ticket. Damit kostet
  ein Audit den Dirigentenkontext wenige tausend Token.
- **Beweis für die Session selbst.** Jeder Bericht endet mit dem
  Scope-Beweis (gelesene Pfade, Zahl der Objekte, fehlende Pfade). Fehlt er
  oder ist er unvollständig, gilt der Bericht als nicht gelaufen; der
  Dirigent startet die Session neu mit engerem Auftrag, wie er einen
  abgebrochenen Worker neu startet.

### 5.1 Kanal zwischen Sessions

Sessions auf demselben Rechner sehen einander über `ListAgents` und
schreiben sich über `SendMessage`; der Empfänger bekommt die Nachricht als
markierten Fremdblock mit Absendernamen und antwortet an diesen Namen. Am
12.09.2026 um 00:37 verifiziert: Kanaltest von `nakama-d8` an
`nakama-dirigent`, Antwort „empfangen" kam von selbst, ebenso die
Idle-Meldung. Der Kanal ersetzt den User als Übersetzer zwischen einer
Konzept- oder Prüfsession und dem Dirigenten. Er kostet Dirigentenkontext
(600k-Grenze, Skill §5), deshalb vier Regeln:

1. Eine Nachricht trägt einen Zeiger (Datei, Abschnitt, Commit) und eine
   Frage oder einen Entscheid, nie Inhalt. Eine Nachricht je Thema.
2. Der Dirigent antwortet mit Entscheidung und dem Ort, an dem sie steht
   (Manifest, Register, Commit), nie mit Bericht. Er verarbeitet Nachrichten
   im Abschlussfenster, nicht mitten in einer Prüfrunde.
3. Eine Nachricht ist nie Haltgrund und nie Anforderungsquelle für einen
   Worker. Umsetzung bleibt Ticketarbeit; der eine Schreiber im sichtbaren
   Checkout bleibt der Dirigent.
4. Keine Session bittet die andere um etwas, das bei ihr selbst geblockt ist.

Ein Idle-Abonnement ohne Text kostet den Empfänger nichts und ist der Weg,
auf ein freies Fenster zu warten; nie pollen, nie „bist du fertig" senden.
Was über den Kanal entschieden wurde, steht danach im Repo, nie nur im Kopf
einer Session.

---

## 6. Einbau in den Dirigentenzyklus

Der Zyklus aus dem Dirigenten-Skill §3 bleibt; die Prüfspur hängt sich an
vier Stellen ein. Jede Stelle nennt ihre Kosten, damit sichtbar bleibt, dass
sie sich lohnt.

### 6.1 §3.1 Vorher: Prüfgang

Vor dem Workerstart läuft `/pruefen` (Stufe 0, Minuten). Ein GRENZE-Riss
außerhalb des Ticketbereichs wird Registerzeile und, wenn ein Pflegeschritt
ihn schon kennt, dort Nachtrag; das Ticket startet trotzdem. Liegt der Riss
im Ticketbereich, geht er als Satz in den Ticketauftrag, nicht als
Punktkorrektur nach dem Bau. Rote Tore werden gelesen, grüne nicht.

### 6.2 §3.2 Bauen: Wächter im Auftrag

Der Ticketauftrag erhält einen Satz: „Vor jedem Commit den fälligen Wächter
aus `docs/gesundheit/KONZEPT.md` §4.5 dispatchen und sein Ergebnis im
Manifest unter den eigenen Ticketproben nennen." Kosten: ein Sonnet-Aufruf je
Commit mit Pfadtreffer, sonst keiner.

### 6.3 §3.3 Messen: Diff-Tore

Zur Rundenbilanz kommt der Prüfgang auf dem HEAD, beschränkt auf die
Diff-Tore (Beziehungspaare, NaN-Wachen, Vakuum). Ein Treffer ist ein Satz im
Dirigentenstand, keine Nacharbeitsrunde; er geht in den Erstprüfungsauftrag
als Prüffrage.

### 6.4 §3.4 Prüfen: eine Kennung je Befund

Jeder bestätigte Befund (Defekt, Lücke oder Härtung) bekommt zusätzlich die
Klassen-Kennung K1 bis K8 aus §3, oder „K-neu: <Vorschlag>", wenn keine
passt. Das ist ein Wort in der Manifestzeile, die der Dirigent ohnehin
schreibt. Ohne diese Kennung gibt es keine Zweitbefund-Regel; mit ihr kostet
sie nichts.

### 6.5 §3.5 Abschluss: Zweitbefund-Prüfung

Im Abschlussfenster, wo heute `gesundheit.py` läuft:

1. `/pruefen` voll (statt nur `gesundheit.py`); Risse wie in §6.1.
2. **Zweitbefund-Prüfung.** Für jede K-Kennung der Befunde dieses Tickets
   in der Abdeckungskarte nachsehen: Hat die Klasse einen Detektor, der
   diese Instanz gefangen hätte? Nein, und der Zähler der Klasse steht nach
   diesem Ticket auf zwei oder mehr: Registerzeile [Werkzeug · Prüfsystem]
   „Detektor für Kn" mit den Befund-IDs. Der Detektor wird im nächsten
   Pflegeschritt gebaut, nie im Feature-Ticket (NAK-223-Regel). Zähler und
   Zeile trägt der Dirigent in die Karte.
3. **Fällige Audits.** Hat das Ticket Pfade berührt, für die ein Audit
   „danach" fällig ist (§4.3, Spalte Wann), startet der Dirigent die
   Prüfsession jetzt, zwischen den Tickets, und liest ihren Kopf, bevor das
   nächste Ticket beginnt. Höchstens eine Prüfsession je Ticketabschluss.
4. Karte nachziehen nur, wenn ein Detektor dazukam, stillgelegt oder ein
   Zähler geändert wurde. Sonst nicht anfassen.

Die Kontext- und Codebase-Hygiene aus §3.5 (Bytes, Index-Zeilen, `/freshen`
an Gates) bleibt unverändert; `/pruefen` schließt sie ein.

### 6.6 Phasengates

Am Gate laufen als erster Torschritt (User 12.09.2026, Karte U39, Wahl
wörtlich „Nur Prüfsystem durch mich“; Ultra-Review und Astra-Audit sind kein
Torschritt): `/tiefenaudit`, die bis dahin fälligen Spezialaudits, der
Mutanten-Vollmodus, der volle `/freshen`-Lauf. Ergebnisse werden wie ein
externer Audit behandelt (Muster NAK-246). Ein Gate ohne diese Läufe ist
nicht sauber; der User tut am Tor nichts.

### 6.7 Pflegeschritte S31c und S35b

Die Pflegeschritte verbrauchen künftig nicht nur den Befund von
`gesundheit.py`, sondern: den Prüfgang, die in §6.5 angesammelten
Detektor-Zeilen (bauen, mit Rotbeweis, Kanon vorher und nachher), die
Abdeckungskarte (nachziehen, Totes-Element-Regel anwenden), den
Mutanten-Vollmodus und ein Spezialaudit ihrer Wahl. Der Wortlaut in
`docs/plan/plan.json` S31c und S35b wird entsprechend ergänzt (§10, Schritt
0). Der Dirigent zieht einen Pflegeschritt vor, sobald eine GRENZE reißt,
ohne zu fragen (`CLAUDE.md`: „Schwellen, die sofort ein Pflegeticket
auslösen").

### 6.8 Was im Dirigenten-Skill steht

Der Skill lag am 12.09.2026 mit 38 448 Bytes über seiner Grenze von 36 864;
im Abschlussfenster desselben Tages hat der Dirigent ihn auf 23,3 KB und die
`CLAUDE.md` auf 17,7 KB gekürzt und die Grenzen auf 24 KB und 20 KB gesetzt
(Commit `fdeef23c`). Das Konzept steht deshalb **nicht** als Prosa im
Skill, sondern als Abschnitt „7. Prüfsystem und Kanal" mit Verweis,
eingepflegt mit `fdeef23c` nach dem Wortlaut in Anhang A.4:

> Die Prüfspur neben Kanon und Codex-Review steht in
> `docs/gesundheit/KONZEPT.md` und ist bindend für §3.1 (Prüfgang vor dem
> Ticket), §3.2 (Wächter im Auftrag), §3.4 (Klassen-Kennung je Befund), §3.5
> (Zweitbefund-Prüfung, fällige Prüfsession) und für den Pflegebetrieb nach
> dem Plan. Der Dirigent auditiert nie selbst: Audits laufen als Prüfsession
> zwischen zwei Tickets, er liest nur Kopf und Befundliste.

Damit ist der Skill die Regel, das Konzept die Ausführung, und beides wird
von `gesundheit.py` gemessen.

---

## 7. Pflegebetrieb: der zweite Betriebsmodus des Dirigenten

Heute ist der Plan die einzige Ticketquelle. Ist `docs/plan/plan.json` leer
(nach R4), endet der Dirigent nicht, sondern wechselt in den Pflegebetrieb.
Mechanik und Rollen bleiben dieselben (Opus baut, Codex prüft, Kanon
beweist, Register führt); nur die Quelle der Tickets ändert sich:

| Quelle | Wird Ticket, wenn | Prüfstufe |
|---|---|---|
| Prüfgang | eine GRENZE reißt | T2 |
| Prüfsession | ein Defekt gegen eine Invariante steht | T2, Muster NAK-246 |
| Zweitbefund-Zeilen | ein Detektor aussteht | T2, Rotbeweis Pflicht |
| Mutanten | Überlebender in einem Invariantenpfad | T2 |
| Laufzeit-Arm | ein Szenario verfehlt seine Erwartung | T2 |
| Register [Härtung/Struktur] | nach Klassenzähler und Datum, älteste zuerst | T2 |
| Zwecktreue-Empfehlung | der User den Rückbau entschieden hat (Karte in `fragen.json`) | T2 |

Kadenz im Pflegebetrieb, ereignisgetrieben, nicht kalendarisch: Prüfgang
nächtlich (Stufe 0); Prüfsessions, wenn ein Ticket ihre Pfade berührt hat;
Tiefenaudit und Mutanten-Vollmodus je abgeschlossenem Pflegeblock von fünf
Tickets; `/freshen` wie im Playbook. Neue Produktarbeit entsteht aus
Tiefenaudit-Einsichten und User-Entscheiden und wird als neue Planphase
eingetragen; dann gilt wieder der Bauplan-Modus, mit denselben Werkzeugen.

Das ist die Antwort auf „nicht nackt dastehen": Am Tag, an dem der Plan
leer ist, gibt es einen Prüfgang, der jede Nacht läuft, eine Karte, die sagt,
was gedeckt ist, Audits, die auf Knopfdruck wiederholbar sind, Ratschen, die
nur steigen, Wächter an den vier heiklen Grenzen und einen Laufzeit-Arm
gegen FL Studio. Alles davon ist aus den Befunden des Baus gewachsen und hat
jeden seiner Anlässe im Kopf stehen.

---

## 8. Kostenstufen und Sparsamkeitsregeln

| Stufe | Was | Modell | Wann | Grenze |
|---|---|---|---|---|
| 0 | Tore, Ratschen, Karte lesen, nächtlicher Läufer | keines | jederzeit | keine; kostet nur Rechenzeit |
| 1 | Wächter, Mutanten-Diff lesen | Sonnet | nur bei Pfadtreffer | ein Aufruf je Commit |
| 2 | Prüfsession (Spezialaudit, Karte, Laufzeit-Arm) | Opus-Hauptlauf, Sonnet-Fan-out | ereignisgetrieben zwischen Tickets | höchstens eine je Ticketabschluss |
| 3 | Tiefenaudit, Mutanten-Vollmodus | Opus, Sonnet | je Phasengate, je Pflegeblock | nie zwischen Tickets |

Regeln:

- **Rot lesen, grün nicht.** Kein Modell liest eine grüne Tabelle.
- **Ein Audit, das zweimal in Folge ohne Befund endet, wechselt auf
  „nur auf Anforderung"**; ein neuer Befund seiner Klasse holt es zurück.
  Vermerk im Änderungsprotokoll.
- **Kein Detektor ohne Rotbeweis, kein Befund ohne Klasse, kein zweiter
  Ledger.**
- **Keine Meldungen.** Die Regel aus dem Dirigenten-Skill §5 gilt auch für
  die Prüfspur: der einzige Text an den User ist eine Produktfrage oder ein
  Handgriff. Ein Prüfgang-Rot ist eine Registerzeile, kein Text.
- **Die Prüfspur misst sich selbst** mit denselben Maßen wie der Rest: die
  Bytes der Auftragsvorlagen und Wächterdateien fallen unter K7; ein
  Prüfwerkzeug, das nie rot war, ist ein totes Element (§2, Regel 3).

---

## 9. Was von Sonifold bewusst nicht übernommen wird

| Sonifold | Grund |
|---|---|
| Tore als Shell in einer 1 929-Zeilen-Skill-Datei | wurde zur größten Wartungslast: ein SessionStart-Hook und ein eigenes Tor nur, um das Tor-Skript gegen Drift zu prüfen. Tore werden Code mit Selbsttest; Drift zeigt sich als roter Selbsttest. |
| 36 handgepflegte Agentenaufträge (111 KB) mit Zeilenbereich-Slices und Wortbudgets | Kontextnot der Werkzeuge von damals; die Topologie eines Audits wird aus dem Baum gerechnet (Dateien, Fachbereich, Zeilen), nicht als Prosa gepflegt. |
| eigene Befundbasis mit Punktwertung Impact × Aufwand | Nakama hat das Register mit Klassen und Datum. Ein Ledger. |
| SonarQube, WCAG-Gate, Bundle-Größe | Docker-gebunden und auf ein TypeScript-Frontend zugeschnitten; Nakama hat keins. |
| Monatskadenz | ersetzt durch Ereignisse (berührte Pfade, Gates, Risse). Ein Kalender läuft auch, wenn nichts passiert ist. |

Was bleibt, weil es Prozessregeln sind: Scope-Beweis je Bericht, Quelle vor
Urteil, Skeptikerpass, keine festgeschriebenen Zahlen, Audit ändert nichts,
das Urteil bleibt im Hauptlauf, die Datei ist der Zustand.

---

## 10. Aufbauplan

Jeder Schritt ist ein eigenes, verhaltensneutrales Pflegeticket im Dirigenten
mit vollem Kanon vorher und nachher (Muster S19b, NAK-223) und Prüfstufe T2.
Der User tut nichts, außer wo es steht. Die Reihenfolge folgt der Evidenz aus
§3 und dem Plan: die Detektoren für K3 und K4 müssen stehen, bevor S26–28
neue persistente und vertragliche Felder bringt.

| # | Ticket | Inhalt | Beweis | Voraussetzung | User |
|---|---|---|---|---|---|
| 0 | Konzept abnehmen, Plan und Register nachziehen | datiertes User-Zitat im Register; Registerzeilen [Werkzeug · Prüfsystem] für die Schritte 1–8; Wortlaut von S31c/S35b ergänzt; Absatz im Dirigenten-Skill (§6.8) | `dokuriegel.py` auf Register, Plan, Skill | dieses Dokument | Entscheid §11 |
| 1 | Abdeckungskarte — **erledigt 12.09.2026** | Prüfsession nach §4.2; Karte committen; erste Zähler je Klasse aus Register und Audit D1–D10; Befunde A-1 bis A-7 in der Karte | Scope-Beweis im Bericht; Karte durch `dokuriegel.py` | 0 | nichts |
| 2 | Prüfgang, erste Fassung | Verzeichnis `pruefung/` unter `tools/` mit Läufer und den Toren aus §4.1, jedes mit Selbsttest und Rotbeweis; Kanon-Bein; §3.1-Aufruf; nächtliche Aufgabe | Kanon vorher = nachher, plus Rohausgabe jedes Rotbeweises | 1 | nichts |
| 3 | Lebenslauf-Audit — **erster Lauf erledigt 12.09.2026** (`tools/pruefung/auftrag-lebenslauf-audit.md`, Bericht `docs/gesundheit/lebenslauf-audit.md`, Befunde B-1 bis B-8, kein Defekt) | Auftragsvorlage, Prüfsession, Bericht, Registerzeilen; Defekte werden Tickets | Scope-Beweis; Zähler K1/K2 in der Karte | 1 | nichts |
| 4 | Zustandstreue und Vertragstreue | wie 3, für K4 und K3; vor S26–28 | wie 3 | 1 | nichts |
| 5 | Mutanten im Broker | Ratsche setzen, Diff-Modus als Bein, Überlebende klassifizieren | Baseline-Dateien, Kanon | 2 | nichts |
| 6 | Wächter | vier Agentendateien, Satz im Ticketauftrag, Hook nennt den Wächter | ein Probelauf je Wächter gegen einen historischen Befund | 1 | nichts |
| 7 | Echtzeit-Audit und Kalibrierung des Tors | Aufrufgraph, Ausnahmeliste, Tor von HINWEIS auf GRENZE | Rotbeweis mit absichtlicher Sperre in `processBlock` | 2 | nichts |
| 8 | Zwecktreue | Prüfsession, Empfehlungen als Karten in `fragen.json` | Scope-Beweis | 1 | Entscheid je Rückbau |
| 9 | Bedienehrlichkeit, Laufzeit-Arm, Tiefenaudit | Bedienehrlichkeit in zwei Läufen (Entscheid §11 Punkt 2, 12.09.2026, NAK-281): jetzt auf der Skizze, nach S26–28 auf der gebauten Oberfläche; Laufzeit-Arm als Plan-Schritt S25e (NAK-286, User-Entscheid 12.09.2026) vor S26–28, danach automatisch je lohnendem Ticket (§4.6); Tiefenaudit am Gate G6 | Belege, Szenarien, Bericht | 2 | nichts (Installation über NAK-285; nur das Diagnoseprojekt nach NAK-30 neu) |

Schritte 1 bis 4 sind die Substanz; ohne sie läuft der Rest ins Leere.
Schritt 0 ist Dirigentenarbeit im nächsten Abschlussfenster, kein Worker.

**Definition „eingebaut".** Das Konzept gilt als im Dirigenten angekommen,
wenn der Absatz aus §6.8 im Skill steht, `/pruefen` in §3.1 läuft, die Karte
existiert, die Klassen-Kennung in mindestens einem Ticketmanifest vergeben
wurde und ein Spezialaudit einmal als Prüfsession gelaufen ist. Bis dahin ist
es ein Plan.

---

## 11. Was der User entscheidet

Alles Technische in diesem Konzept (Werkzeugwahl, Pfade, Schwellen, Reihenfolge
innerhalb der Evidenz, Modellwahl) entscheidet Claude und begründet es hier.
Zwei Punkte sind Produkt:

1. **Ob diese Prüfspur als Ganzes gewollt ist** und Pflegeschritte künftig
   Prüfgang, Karte und Audits verbrauchen statt nur die Größenmaße.
   **Entschieden 12.09.2026** (Wortlaut im Kopf dieses Dokuments).
2. **Wann das Bedienehrlichkeits-Audit läuft**: jetzt auf der technischen
   Skizze (findet Verstöße gegen die Gesetze, bevor sie in JUCE gebaut
   werden) oder erst nach S26–28 auf der gebauten Oberfläche (misst das
   echte Layoutrechteck). Beides ist möglich; die Wahl bestimmt, was der
   nächste UI-Bauer als Auftrag bekommt. **Entschieden 12.09.2026, 11:24
   Uhr** (Karte U38 in `docs/plan/fragen.json`, gestellt über `/fragen`;
   Wahl wörtlich: „Beides: jetzt Skizze, später Oberfläche (Empfohlen)"):
   das Audit läuft zweimal — erster Lauf jetzt auf der technischen Skizze
   (Gesetz 1 „keine toten Elemente" vollständig, Gesetz 2 „Tasten sind
   Material" soweit die Skizze Zustände trägt; Verstöße gehen vor dem
   JUCE-Bau als Auftrag an den nächsten UI-Bauer), zweiter Lauf nach S26–28
   auf der gebauten Oberfläche (Gesetz 2 am gemessenen Layoutrechteck,
   Gesetz 1 erneut). Schritt 9 in §10 trägt beide Läufe; Register NAK-281.
   Davor: Offen seit 12.09.2026 (`fdeef23c`).
3. **Welche Prüfspuren am Tor zusammenkommen.** **Entschieden 12.09.2026,
   12:00 Uhr** (Karte U39 in `docs/plan/fragen.json`, gestellt über `/fragen`;
   Wahl wörtlich: „Nur Prüfsystem durch mich“): an G6 bis G9 läuft
   ausschließlich das Mehragenten-Audit dieses Prüfsystems (§6.6) durch den
   Dirigenten als erster Torschritt; Ultra-Review und Astra-Audit sind kein
   Torschritt, der User tut am Tor nichts. Einen externen Audit setzt er
   weiterhin nach eigenem Ermessen an (Muster NAK-246). Register NAK-282.

Offene technische Punkte, die Claude später an der Quelle entscheidet und
die hier stehen, damit sie nicht verloren gehen: Tragfähigkeit der
C++-Mutation (§4.4); Präzision des Aufrufgraphen ohne clang (§4.3); ob der
nächtliche Läufer über die Windows-Aufgabenplanung oder einen Cron der
Dirigentensession startet; Zeitlimit des Mutanten-Diff-Beins; Kürzung des
Dirigenten-Skills unter 36 KB.

---

## Anhang A. Wortlaut zum Einpflegen (Schritt 0, Dirigent)

Die Texte sind so formuliert, dass der Dirigent sie im Abschlussfenster
übernimmt; Nummern vergibt er (nächste freie NAK-Nummer). Die Nachricht an
ihn trägt nur den Zeiger auf diesen Anhang. **Eingepflegt am 12.09.2026 mit
Commit `fdeef23c`:** Register NAK-259 (Abnahme mit Zitat), NAK-260 bis
NAK-266 (Schritte 2 bis 8), NAK-267 (Abdeckungskarte A-1 bis A-5 als
Härtung; A-2 kein Defekt, weil der Gate-Text von S14–15 die Hüllenprüfung
verlangt, nicht die Verdrahtung im Produkt-Client); Nachtrag in `plan.json`
S31c/S35b; Karte U38 in `fragen.json`; Dirigenten-Skill §7. Der Anhang
bleibt als Wortlautquelle stehen.

### A.1 Registerzeile Abnahme

Klasse **[Werkzeug · Prüfsystem]**, Titel **Prüfsystem-Konzept und Kanal
abgenommen**, Datum 09-12, Anlass „User-Abnahme in Session nakama-d8
(Fable)". Text: das User-Zitat aus dem Kopf dieses Dokuments wörtlich, dann
„Umfasst das Konzept `docs/gesundheit/KONZEPT.md` (Commit `c66a010c`,
Abnahme-Nachtrag im Folgecommit) und den Kanal zwischen Sessions (§5.1).
Aufbauplan §10; Schritt 0 durch den Dirigenten, Schritt 1 durch die Session
nakama-d8."

### A.2 Registerzeilen Aufbauschritte

Je Schritt 2 bis 8 aus §10 eine Zeile, Klasse **[Werkzeug · Prüfsystem]**,
Datum 09-12, Anlass „Prüfsystem-Konzept §10 Schritt n", Titel = Spalte
„Ticket" der Tabelle, Text = Spalte „Inhalt" plus „Beweis: <Spalte
Beweis>; Voraussetzung: Schritt <n>; verhaltensneutrales Pflegeticket, T2,
Kanon vorher und nachher (NAK-223-Regel)". Schritt 1 (Abdeckungskarte)
bekommt keine Zeile: er läuft am 12.09.2026 als Prüfsession in `nakama-d8`
und wird mit seinem Commit gemeldet. Schritt 9 wartet auf A.5.

### A.3 Ergänzung in `docs/plan/plan.json` (S31c und S35b)

An das Feld `text` beider Schritte anhängen: „NACHTRAG 12.09.2026
(Prüfsystem-Konzept abgenommen, `docs/gesundheit/KONZEPT.md` §6.7): Der
Schritt verbraucht zusätzlich den Prüfgang, die angesammelten Zeilen
[Werkzeug · Prüfsystem] (Detektoren mit Rotbeweis, Kanon vorher und
nachher), die Abdeckungskarte (nachziehen, Totes-Element-Regel) und ein
Spezialaudit seiner Wahl; ein leerer Befund bleibt gültig." Danach
`planstand.py` und `dokuriegel.py`.

### A.4 Absatz für den Dirigenten-Skill

Als Abschnitt „7. Prüfsystem und Kanal", kurz, weil der Skill über seiner
Byte-Grenze liegt (§6.8):

> Die Prüfspur neben Kanon und Codex-Review steht in
> `docs/gesundheit/KONZEPT.md` und ist bindend für §3.1 (Prüfgang vor dem
> Ticket), §3.2 (Wächter im Auftrag), §3.4 (Klassen-Kennung K1–K8 je
> Befund), §3.5 (Zweitbefund-Prüfung, fällige Prüfsession) und für den
> Pflegebetrieb nach dem Plan. Der Dirigent auditiert nie selbst: Audits
> laufen als Prüfsession zwischen zwei Tickets, er liest nur Kopf und
> Befundliste. Nachrichten anderer Sessions (`SendMessage`, Absender
> `nakama-*`) tragen Zeiger und Entscheid, nie Inhalt; der Dirigent
> antwortet mit Entscheidung und Ort im Repo, verarbeitet sie im
> Abschlussfenster, nimmt sie nie als Haltgrund oder Anforderungsquelle für
> Worker und bittet keine Session um Geblocktes (§5.1).

### A.5 Karte für `docs/plan/fragen.json`

Frage an den User, im Schema der Datei, gestellt über `/fragen` nach 9:00:
„Das Bedienehrlichkeits-Audit prüft die zwei UI-Gesetze (keine toten
Elemente; Tasten ändern nie ihre Maße). Soll es jetzt auf der technischen
Skizze laufen, damit Verstöße vor dem JUCE-Bau auffallen, oder erst nach
S26–28 auf der gebauten Oberfläche, wo das echte Layoutrechteck messbar
ist?" Zwei Optionen: „Jetzt auf der Skizze" und „Nach S26–28 auf der
Oberfläche". Folge im Konzept: Schritt 9 in §10.

---

## 12. Änderungsprotokoll

| Datum | Änderung | Anlass |
|---|---|---|
| 12.09.2026 | §4.6 neu gefasst: Laufzeit-Arm fester, automatischer Bestandteil jedes lohnenden Tickets (User-Wort 12.09.2026, Register NAK-286, Plan S25e); Installation ohne Handgriff (NAK-285); Diagnoseprojekt liegt vor (NAK-284 Nachtrag); §10 Schritt 9 nachgezogen | Session `nakama-a7`, Konzept zum FL-MCP |
| 12.09.2026 | §4.6: User-Wort zu Messungen über den FL-MCP-Server (Karte U40, 14:00 Uhr); Laufzeit-Arm rückt vor, Register NAK-284 | Dirigentensession, `/fragen U40` |
| 12.09.2026 | §11 Punkt 3 entschieden (Karte U39, 12:00 Uhr, Wahl wörtlich „Nur Prüfsystem durch mich“): Torläufe §6.6 sind erster Torschritt an G6–G9, Ultra-Review überholt; §1.1, §4.3, §6.6 nachgezogen; Register NAK-282; Dirigenten-Skill §3.7 „Phasengate-Prüfung“ | Dirigentensession, `/fragen U39` |
| 12.09.2026 | §11 Punkt 2 entschieden (Karte U38, 11:24 Uhr, Wahl wörtlich „Beides: jetzt Skizze, später Oberfläche (Empfohlen)"): Bedienehrlichkeits-Audit in zwei Läufen; §10 Schritt 9 nachgezogen; Register NAK-281 | Dirigentensession, `/fragen U38` |
| 12.09.2026 | Schritt 3 erster Lauf (Auftragsvorlage, Bericht B-1 bis B-8); §4.1 W-Riss unterscheidet Codebase-Maße (Registerzeile) und Kontextfläche (sofort beheben, User 12.09.2026); §6.8, Anhang A und §11 auf Schritt 0 nachgezogen (`fdeef23c`: NAK-259 bis NAK-267, U38, Skill §7, Skill 23,3 KB / CLAUDE.md 17,7 KB) | Dirigenten-Meldung 12.09.2026; Lebenslauf-Audit |
| 12.09.2026 | §3: K3 geteilt (Rust ✔, C++ ◐), K4 „dicht", K6 auf ◐ (RtWache am EQ-Kern) nach Verifikation; §4.2 und §10 Schritt 1 als erledigt | Abdeckungskarte, erste Fassung (5 Leser, 3 Skeptiker) |
| 12.09.2026 | Status auf abgenommen; §5.1 Kanal zwischen Sessions; §11 Punkt 1 entschieden, Punkt 2 als Karte; Anhang A mit Wortlaut für Register, Plan, Skill und Karte | User-Abnahme 12.09.2026 (Wortlaut im Kopf); Kanaltest 00:37 |
| 12.09.2026 | Erste Fassung als Vorschlag | User-Auftrag 11./12.09.2026 (Wortlaut oben); Messungen an HEAD `818673cd`; Sonifold-System gelesen (`check`, `aftercheck`, `code-health`, `compositor-scan`, `compositor-ui`, `freshen`, `save-fidelity`, `logic-audit`, `deep-audit`, `dev-recorder-loop`, Coverage-Map, Agenten, Hooks) |
