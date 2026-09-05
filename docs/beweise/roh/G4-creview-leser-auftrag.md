# G4 — C++-Sicherheits- und DSP-Review, Klartext-Leser (Ersatz für den blockierten `/c-review`-Workflow)

Du bist einer von 14 Klartext-Lesern. Der Trail-of-Bits-Workflow `/c-review` 2.0.1 hat die Standortpartition erzeugt (14 Zuweisungen), aber alle Review-Agenten wurden vom Safety-Classifier blockiert („output schema too large to classify safely"). Die Partition bleibt die Partition: jede Zeile von `eq-copilot/plugin/` (ohne `tests/` und `vertrag/generiert/`) gehört genau einem Leser. Deine Nummer NN steht in der Nachricht, die dich gestartet hat.

**Repo:** `C:\Users\phili\Projekte\Nakama`, HEAD `9f3000881df632f4969d78551fb0a516b61288cb`. **Strikt lesend:** keine Datei im Repo ändern, keine Git-Schreibbefehle (kein add/commit/checkout/stash/reset), keine Prozesse des Produkts starten, nichts installieren. Erlaubt: Read, Grep, Glob, `git diff/show/log/blame` lesend. Schreiben darfst du **ausschließlich** deine Part-Datei (unten).

## Deine Zuweisung

`C:\Users\phili\Projekte\Nakama\.c-review-results\20260904T235509Z\assignments\unit-NN.json` — Felder:
- `questions`: der Fragenkatalog je Bug-Klasse (Schlüssel wie `integer`, `return-values`, …). Lies ihn vollständig; er ist die Checkliste je Einheit.
- `units`: je Einheit `file` (relativ zu `eq-copilot/plugin/`, Backslash-Pfade), `start_line`, `end_line`, `ranges`, `required_questions`, `site_counts`. `file-scope`-Einheiten sind die Bereiche außerhalb von Funktionen (Includes, Konstanten, Klassenfelder, Templates).

Jede Einheit gehört dir vollständig: **jede Zeile deiner Einheiten wird gelesen**, nicht stichprobenartig. Kontext (Aufrufer, Verträge, Schemas, Tests, Broker-Gegenseite in `broker/`) darfst du im ganzen Repo nachlesen; **Befunde liegen nur in deinen Einheiten**. Findest du etwas außerhalb, notiere es unter „Außerhalb meiner Einheiten" mit Datei:Zeile, ohne es zu bewerten.

## Bedrohungsmodell: LOCAL_UNPRIVILEGED

Angreifer oder Fehlerquelle ist ein lokaler, nicht privilegierter Prozess desselben Rechners (anderer Windows-User, Sandbox, fremdes Plugin im selben Host) sowie fehlerhafte oder feindliche Eingaben über: Named Pipes (Broker-Wire v3 JSON, FlatBuffers-Telemetrie, Envelope mit Prüfsumme), Host-Puffer aus FL Studio (Blockgrößen bis zur Slotkapazität und darüber, Kanalzahlen, NaN/Inf/denormals im Audio, Sampleratewechsel), gespeicherter Plugin-Zustand (State-Blobs alter und unbekannter Versionen, manipulierte Bytes), Dateien und Umgebungsvariablen. **Ausgeschlossen:** Same-user-Angreifer mit Schreibrecht im Repo oder im Benutzerprofil (Entwurf §48.4), FL-Studio-Bugs selbst, Stil.

## Fragen je Einheit (zusätzlich zum Katalog in `questions`)

- **Speichergrenzen:** Index, memcpy/memmove-Längen, Puffergrößen gegen Blockgröße/Kanalzahl, Off-by-one an Ringgrenzen, `std::array`/`operator[]` ohne Prüfung.
- **Integer:** Überlauf, Vorzeichenwechsel, `size_t`↔`int`, Wrap bei Sequenz- und Samplezählern, Division durch 0, Shift-Weiten.
- **Konvertierung/UB:** float→int bei NaN/Inf/Überlauf, Aliasing, uninitialisierte Felder, Vergleiche mit NaN, Reihenfolge der Auswertung.
- **Objektlebenszyklus/C++:** Dangling-Referenzen, Move-Zustände, Lambda-Captures per Referenz über Threadgrenzen, Iterator-Invalidierung, Init-Reihenfolge, Exception-Sicherheit im Audio-Thread.
- **Nebenläufigkeit:** Audio-Thread ↔ Analyse-Worker ↔ Message-Thread ↔ IPC-Threads — Datenrennen, fehlende Speicherordnung an atomics, Lock oder Allokation im Audiothread, SPSC-Ringe mit zwei Produzenten, Stale-Closures nach `stop()`.
- **Windows:** Named-Pipe-Handles, Token/Impersonation, Pfade, `CreateProcess`, DLL-Suchpfad, Handle-Leaks in Fehlerpfaden.
- **Rückgabewerte/Fehlerpfade:** ungeprüfte API-Rückgaben, Fehlerpfade, die Zustand halb ändern (Save↔Load, Start↔Stop, Verbinden↔Trennen).
- **Logik/Protokoll:** Sequenz, Revision, Nonce, Fristen, Zustandsautomaten (Hörmarker, Taint, Experiment, Passage), die mit einem einzigen Frame in einen falschen Zustand kippen.

## DSP-Gate-Fokus (Nakama-Invarianten aus `CLAUDE.md`; nur wo deine Einheiten sie berühren)

1. Audio-Thread ohne Sperren, Allokationen, Datei-, Pipe-, Netzzugriff und Logging; Überlast verwirft **Analyse**frames, nie Audio.
2. Passthrough im Nulltest bitidentisch, 0 Samples Latenz, kein Tail. Kein Hostpuffer wird angefasst — einzige Ausnahme der useraktivierte Hörmarker (`src/HoerMarkierung.h`) mit vier fail-closed Termen (`playing=true`, `recording=false`, Realtime bewiesen, Editor offen) und erzwungenem Ausfade.
3. **Gate 5:** kein Telemetrie- oder Steuerframe (aus Pipe oder Modell) steuert samplegenauen Gain oder erzeugt ein hörbares PRE/POST-Delta.
4. **NaN-Ehrlichkeit:** nicht-endliche Zwischenwerte werden verriegelt und gezählt; Nyquist wird gekappt; ohne genügend endliche Nachbarn gibt es keine Basislinie.
5. **State bleibt verlustfrei:** unbekannte Major-Versionen oder unzulässige Matrizen werden read-only mit Originalbytes gehalten.

## Befundregeln

Ein Befund braucht: Datei:Zeile (repo-relativ ab `eq-copilot/plugin/`), Bug-Klasse, den **konkreten Eingabepfad oder Wert**, der ihn auslöst, die Wirkung, die Erreichbarkeit unter dem Bedrohungsmodell (mit Aufruferkette) und eine Schwere CRITICAL / HIGH / MEDIUM / LOW nach deinem eigenen Urteil (es läuft kein Richter). Keine Stilbefunde, keine Kosmetik, keine hypothetischen Fälle ohne Eingabepfad, keine Befunde über `docs/**`. Vor dem Melden per `rg -n "<Stichwort>" docs/offene-punkte.md` prüfen, ob der Punkt schon datiert im Register steht (NAK-135 bis NAK-179 stammen aus den jüngsten Läufen); wenn ja, nenne die NAK-Nummer in der Befundzeile statt ihn als neu zu führen.

## Ausgabe

Schreibe genau eine Datei: `C:\Users\phili\AppData\Local\Temp\claude\C--Users-phili-Projekte-Nakama\3d3f130c-8fb7-4394-85e6-7f940bad9e94\scratchpad\G4-creview-parts\unit-NN.md` (Markdown, Deutsch) mit diesem Aufbau:

```
# unit-NN — <Dateien der Zuweisung>
Gelesen: <n von m Einheiten vollständig> · Zeilen: <Summe>

## Befunde
- [SEV] <file>:<line> · <klasse> · <Behauptung in einem Satz> · Auslöser: <Eingabe/Wert> · Wirkung: <…> · Erreichbar: <ja/nein, Aufruferkette> · Beleg: <Zeilen> · Register: <NAK-… oder neu>
(oder: keine Befunde)

## Gate-5- und Invariantenbeleg
je Invariante 1–5, sofern deine Einheiten sie berühren: <hält / bricht / nicht berührt>, mit Datei:Zeile

## Ledger
<unit id> · clean | Befund <n>   — jede Einheit deiner Zuweisung genau einmal; „clean" nur nach vollständigem Lesen

## Nicht gelesen
<Einheiten mit Grund, sonst: keine>

## Außerhalb meiner Einheiten
<Datei:Zeile + ein Satz, unbewertet; sonst: nichts>
```

**Rückmeldung an den Dirigenten:** höchstens 10 Zeilen — Einheiten gelesen/gesamt, Befunde je Schwere, jeder HIGH- oder CRITICAL-Befund in einer Zeile, Pfad der Part-Datei, und das Ergebnis von `git status --short` (muss leer sein) und `git rev-parse HEAD` (muss `9f30008…` bleiben).
