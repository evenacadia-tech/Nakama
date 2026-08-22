# Werkzeugplan — Skills, Agenten und Hooks für dieses Projekt

**Vorschlag vom 2026-08-21. Nichts davon ist gebaut, nichts davon ist
entschieden.** `.claude/skills/` enthält weiterhin nur `interview`,
`.claude/agents/` existiert nicht, `settings.json` ist unverändert.

> **Seit 22.08.2026 im Nakama-Repo:** `.claude/` meint die Repo-Wurzel (dort liegt
> neben `interview` auch der Skill `prompt`), die Hooks dieses Ordners liegen in
> `tools/hooks/` (`design-primer.sh`, `prototyp-schleuse.sh`, `schleusen-probe.sh`)
> und sind in der Wurzel-`settings.json` verdrahtet.

Diese Datei entscheidet nichts. Sie legt hin, **welche Fehler dieses Projekt
nachweislich wiederholt** und welches Werkzeug jeden davon trifft. Jeder
Vorschlag nennt seinen Anlass mit Belegstelle aus diesem Repo — was keinen
Anlass hat, steht nicht drin.

---

## 1. Der Befund: was hier wirklich schiefgeht

Nicht gesucht wurde nach „nützlichen Agenten", sondern nach den Fehlern, die
dieses Repo selbst dokumentiert. Es sind sechs Klassen.

| # | Fehlerklasse | Belege |
|---|---|---|
| **F1** | **Die eigene Zahl stimmt nicht.** | **Sieben** nachträgliche `⚠ Berichtigt`-Kästen im Repo (gezählt 2026-08-21 09:45, ohne die zwei Nennungen in dieser Datei: `sondenplan` 2 · `2026-08-21-sessionende` 3 · `werkzeug/LIES-MICH` 2). 370 → **368** px · „zehn Parameter" → **zwölf** (Inhalt 20 % größer als der Auftrag ansetzte) · „sieben Urteile" → **fünf** · „18 Runden" → **19** · 7.048 → **7.575** Zeilen · 20 → **25** Dateien · Messtabelle **6 von 15** Feldern falsch · Kopf sagte `11 of 16` bei fünf gezeigten Zeilen · Sondenplan Abschnitt 4: **10 von 19** nachprüfbaren Angaben falsch. Ein Commit heißt wörtlich *„Report-Audit: zwei eigene Zahlen waren falsch"*. |
| **F2** | **Das Blatt behauptet, was sein Zustand widerlegt.** | **18 Lügen** allein in `zustaende.html`, alle erst im Grenzfall sichtbar: Assistent riet auf getrennter Quelle · „Confidence: high" bei „Measurement not started" · „Width 62 %" neben „Width: unavailable" im selben Bild. |
| **F3** | **Entschiedenes wird versehentlich wieder aufgemacht.** | **Neun** Blätter verschwiegen ihren Stand, eines nannte die **verworfene** Variante „Deine Wahl". **Vier** Dokumentstellen zitierten Arbeitsannahmen als abgenommen — *„ein zweites Mal"*, sagt die PRÜFLISTE. |
| **F4** | **Prüfungen werden blind und melden trotzdem „heil".** | Prüfmuster griff ins Leere → „heil" · Schleuse ließ `Bash` durch (exit 0 statt 2) · **90 von 90** Beleg-Chips kaputt · der Standband-Test prüft die **Form** (`body > div[style*="padding:7px 26px"]`), nicht den Inhalt — belegt am 2026-08-21, als `formfaktor.html` über sein eigenes Messgerät log und die Prüfung zufrieden war. |
| **F5** | **Wissen ist sitzungslokal und geht verloren.** | *„die Skripte waren sitzungslokal und sind weg"* · Fingerabdruck, Belegprobe und Ehrlichkeitsprobe fehlen bis heute · die Verdeckungsprüfung existierte in **keinem** Blatt, bis `sonde-messung.html` sie brachte. |
| **F6** | **Die Reihenfolge wird übersprungen.** | Fünf Wireframes ohne Aufgabenliste · Licht vor Layout · Radien vor Welt · Material gemessen, während Phase 1 bei Schritt 1 steht. **Alle drei Male hat der User es gemerkt, nicht Claude** (`werkzeug/LIES-MICH.md`). |

---

## 2. Die Aufteilung: was Skill ist, was Agent, was Hook

- **Skill = Ritual.** Läuft im Hauptgespräch, der User ist im Loop, es entsteht
  ein Artefakt am lebenden Blatt. Trifft **F6** und **F5** — es schreibt eine
  Prozedur fest, die sonst jede Sitzung neu erfindet.
- **Agent = fremde Augen mit eigenem Kontext.** Genau dort, wo mein Kontext
  *befangen* ist: ich habe das Blatt gebaut, also weiß ich, was es sagen
  *soll*, und lese über die Lüge hinweg. Die falsche Zahl steht in meinem
  Gedächtnis — deshalb darf die Gegenprüfung nicht aus dem Gedächtnis kommen.
  Trifft **F1**, **F2**, **F3**.
- **Hook = Zwang.** Dort, wo Vergessen der wahrscheinliche Fehler ist.
  Trifft **F4**.

**Die Agenten-Doktrin dieses Projekts bleibt** (`abnahmen/2026-08-21-codereview.md`,
Abschnitt 1): *Agenten berichten, ich prüfe am Quelltext nach, ich repariere.*
Beim 8-Agenten-Review wurde jeder Befund vor der Übernahme am Quelltext
gegengeprüft; einer ließ sich nicht reproduzieren und flog raus. **Kein Agent
bekommt `Write` auf ein Blatt.**

---

## 3. Agenten — vier, nach `.claude/agents/`

### A1 · `luegen-detektiv`

Fährt **ein** Blatt in seine Extremzustände (alles getrennt · keine Quelle ·
8 Bänder aktiv **und** alle dynamisch **und** Bypass **und** Schutzbereich
getroffen) und sucht jede Aussage, die der Zustand widerlegt: Zähler gegen
gezeigte Zeilen, Knöpfe die nichts tun, Werte die im Streifen „unavailable"
heißen und im Detail eine Zahl zeigen, leere Felder ohne Erklärung.

> **Anlass:** 18 Lügen in einem Blatt, alle erst im Grenzfall sichtbar.
> PRÜFLISTE 7.4: *„Im Grenzfall prüfen, nicht im Schönfall."*

```yaml
---
name: luegen-detektiv
description: Prüft EIN lebendes Blatt in werkzeug/ auf Aussagen, die sein eigener
  Zustand widerlegt. Fährt die Grenzfälle an, misst nach dem Rendern, berichtet nur
  — repariert nie. Vor jedem Vorlegen eines Blattes beim User.
tools: Read, Grep, Glob, Bash, mcp__plugin_playwright_playwright__browser_navigate,
  mcp__plugin_playwright_playwright__browser_click,
  mcp__plugin_playwright_playwright__browser_evaluate,
  mcp__plugin_playwright_playwright__browser_take_screenshot
model: inherit
effort: high
---
```

**Warum Agent:** Ich habe das Blatt gebaut. Und ein Grenzfall-Durchlauf über
sechs Szenarien × drei Arbeitsschritte erzeugt DOM-Ausgabe, die im Hauptkontext
nur Platz frisst — behalten will ich zehn Zeilen Befund.
**Gegenprobe (Pflicht):** eine künstliche Lüge einbauen, er muss sie finden.
**Sein Maßstab** ist `werkzeug/PRUEFLISTE.md` — die liest er selbst, er hat
`Read`. (Das Feld `skills:` könnte sie ihm vorladen, aber das setzte voraus,
dass die Prüfliste als Skill verpackt wird. Ein siebter Skill nur als
Verpackung eines Dokuments lohnt nicht.)

### A2 · `zahlen-pruefer`

Bekommt ein Dokument (Abnahme, Sessionende, Sondenplan, Standband) und misst
**jede Zahl darin** am Quellblatt nach. Ausgabe ist stur eine Tabelle:
`Zahl | Behauptung | gemessen | belegt/falsch/unbelegbar | Fundstelle`.
Eine Zahl ohne Fundstelle heißt **unbelegbar**, nicht „stimmt wohl".

> **Anlass:** F1 in Reinform. Jede der fünf `⚠ Berichtigt`-Stellen ist eine
> falsche Zahl im eigenen Bericht; zwei davon standen im Auftrag für die
> laufende Runde.

```yaml
---
name: zahlen-pruefer
description: Misst jede Zahl in einem Projektdokument am Quellblatt oder Quelltext
  nach, nie aus dem Gedächtnis. Pflicht vor jedem Sessionende, jeder Abnahme und
  jedem Bericht, der Zahlen nennt.
tools: Read, Grep, Glob, Bash
model: inherit
---
```

**Warum Agent:** Die falsche Zahl steht in *meinem* Kontext. Ein frischer
Kontext hat nichts zu bestätigen.

### A3 · `abnahme-waechter`

Vor dem Öffnen einer Gestaltungsfrage: liest **alle** `abnahmen/` und antwortet
in einem Absatz — *schon entschieden* (Zitat + Datei + Datum), *Arbeitsannahme*
(+ benannter Prüfpunkt), oder *offen*.

> **Anlass:** `CLAUDE.md` Regel 4. `abnahmen/LIES-MICH.md` existiert nur, weil
> „logisch zu diesem Zeitpunkt" als „entschieden" verbucht wurde — und vier
> Stellen zitierten Arbeitsannahmen danach trotzdem als abgenommen.

```yaml
---
name: abnahme-waechter
description: Prüft vor dem Öffnen einer Gestaltungsfrage, ob sie in abnahmen/ bereits
  entschieden ist. Unterscheidet strikt Abnahme (bindend) von Arbeitsannahme (mit
  Prüfpunkt) und liefert das wörtliche User-Zitat.
tools: Read, Grep, Glob
model: inherit
---
```

**Warum Agent:** Viele Dateien lesen, einen Absatz behalten. **Nur lesend** —
kein `Write`, kein `Bash`.

### A4 · `spezifikations-deckung`

Gleicht **beidseitig** ab: (a) welche Pflichtangabe der Spezifikation hat einen
Ort auf dem Blatt — mit Zeilenbeleg, (b) **was zeigt das Blatt, das die
Spezifikation nicht kennt** (`CLAUDE.md` Regel 6). Die zweite Richtung wird
fast immer vergessen.

> **Anlass:** `docs/sondenplan.md` Abschnitt 7 — am Befund fehlen Begründung,
> Alternativursachen und billigster Beweisschritt; *„keine Änderung empfohlen"*
> existiert nicht; Suche, Pinning, Fokusgruppen und die musikalischen Rollen
> haben keinen Ort und keine benannte Tür.

```yaml
---
name: spezifikations-deckung
description: Gleicht ein Blatt beidseitig gegen docs/oberflaechen-spezifikation.md ab —
  welche Pflichtinhalte fehlen, und was das Blatt zeigt, das die Spezifikation nicht
  kennt. Vor jeder Abnahme einer Oberfläche.
tools: Read, Grep, Glob
model: inherit
---
```

**Warum Agent:** Braucht Spezifikation **und** das ganze Blatt vollständig — und
muss noch zweimal laufen (Editor, Kachel).

---

## 4. Skills — sechs, nach `.claude/skills/`

Hausstil wie `interview`: deutsch, `name` + `description`, Prozedur statt Prosa.

| Skill | Was es erzwingt | Anlass |
|---|---|---|
| **`/aufgabenliste`** | Die Häufigkeitsliste **vor** jedem Wireframe. Trennt hart: *Handgriffe* sind aus der Spezifikation abgeleitet (mit Belegstelle), *Häufigkeit* ist eine **Produktaussage und gehört dem User**. Endet damit, dass sie ihm vorgelegt wird. | `DESIGN-GESETZE` Folge 1 — *„ohne diese Liste kein Wireframe"*. Fünf Entwürfe entstanden ohne sie, alle fünf waren derselbe Fehler. `sondenplan` Abschnitt 3 ist genau dieses Artefakt. |
| **`/klemme`** | **Erst messen, dann entwerfen.** Vorrat rechnen (innen − Streifen − Lücken), Kosten *einer* wiederholten Zeile messen, N Anordnungen vergleichen. Trägt die fünf Messregeln mit: rekursiv · beide Richtungen · zweimal messen · vor dem Messen zurücksetzen · nie eine Höhe annehmen. **Schlimmer Fall ist Vorgabe, nicht Sonderfall.** | Das benannte Arbeitsmuster, das Main getragen hat. Der Editor braucht es sofort: 101 einstellbare Werte auf einer Fläche, die 10 px niedriger und 60 px schmaler ist als die bereits knappe von Main. |
| **`/variantenrunde`** | Varianten unterscheiden sich in der **Disclosure-Mechanik**, nie in der Kastenanordnung · jede mit benannter Hypothese · **kein Variantenname während der Wahl** · neutrales Mittelgrau als Chrome · Zweikampf bei **1:1** · **Ablehnung wird erfasst** · Ergebnis als JSON. | Die sieben Prinzipien stehen heute nur **im Code** von `werkzeug/`, nicht als Regel. `DESIGN-GESETZE` Folge 3 + 4, `CLAUDE.md` Regel 3. Das meistwiederholte Ritual des Projekts. |
| **`/blatt`** | Neues lebendes Blatt aus `zustaende.html` abzweigen nach der Kopierliste: was mitkommt (`listenEhrlich`, `listenZuruecksetzen`, `aufteilen`, `ueberlauf`, `LAGE`/`griff`), was **geleert** wird (`BELEG`), was **umbenannt** wird (`BAND`), was **ersetzt** wird (`ZUSTAENDE`). Pflichtausstattung: Standband · Prüfband (1:1) · Editor-Panel · Demo-Marke im **Chrome**. Schließt mit `pruefen.mjs`. | `sondenplan` Abschnitt 2 ist diese Liste. Wird zweimal ausgeführt (Editor, Kachel), später wieder in `prototyp/`. Die Falle steht dabei: *„Aus 1200 Zeilen Main wird ein Editor herausgeschnitten."* |
| **`/abnahme`** | Urteil festhalten: wörtliches Zitat (**nie geglättet**), was festliegt, was offen bleibt — und die **erzwungene Wahl**: Abnahme *oder* Arbeitsannahme mit benanntem Prüfpunkt. Verweigert „abgenommen" für etwas, das an einem Stellvertreter beurteilt wurde. | `abnahmen/LIES-MICH.md`. Der Fehler ist zweimal passiert. |
| **`/sessionende`** | Die Übergabe in bewährter Form — Stand · Entwurf an **einem** Ort · nächster Schritt konkret · Werkzeugtabelle · Fallen · offene Fragen · Prüfpunkt der Arbeitsannahmen. **Ruft vorher `zahlen-pruefer`.** | *„Diese Datei ist der einzige Wissenstransfer. Was hier nicht steht, ist weg."* Beide bisherigen Sessionende-Dateien brauchten nachträgliche Berichtigungen. |

**Zwei Mechanik-Entscheidungen:**

1. **`/klemme` bringt ein Skript mit** (`werkzeug/pruefung/messen.mjs`), nicht
   nur Prosa. Grund ist projekteigen: *„die Skripte waren sitzungslokal und sind
   weg"* ist ein **benannter Verlust** dieses Repos. Freigabe ohne Nachfrage
   über `allowed-tools: Bash(node ${CLAUDE_PROJECT_DIR}/werkzeug/pruefung/messen.mjs *)`.
2. **`/blatt` registriert den Blattprüfungs-Hook** über das `hooks:`-Feld im
   Frontmatter — ab dem Moment, wo ein neues Blatt entsteht, läuft die Prüfung
   für den Rest der Sitzung mit. Das ist der Unterschied zwischen „soll man
   machen" und „passiert".

---

## 5. Hooks — zwei

| Hook | Ereignis | Was er tut | Anlass |
|---|---|---|---|
| **`blattpruefung`** | `PostToolUse` auf `Write\|Edit\|Bash` | Wurde ein `werkzeug/*.html` berührt → `node werkzeug/pruefung/pruefen.mjs <blatt>`. Meldet an Claude zurück, wenn das Blatt tot ist. | *„Vier Blätter auf einmal tonlos tot, gefunden nach zwanzig Sekunden nur, weil eine Blattprüfung lief."* `PostToolUse` **kann nicht blocken** — muss es auch nicht, die Datei ist schon geschrieben; die Rückmeldung reicht. |
| **`standband-wahrheit`** | `Stop` | Prüft das mechanisch Prüfbare: geändertes Blatt ohne Prüflauf · Standband ohne Datum · Standband-Datum älter als die letzte Änderung des Blattes. | Genau der Fehler vom 2026-08-21 früh. Der bestehende Test prüft die **Form** des Standbands, nicht seinen **Inhalt**. `Stop` **kann** blocken (exit 2) — braucht deshalb einen ehrlichen Ausgang, sonst baut man sich eine Falle. |

Beide bekommen eine **Gegenprobe** nach dem Muster von `schleusen-probe.sh` —
was blocken muss **und** was durchmuss. Die Prototyp-Schleuse war blind und hat
nach der ersten Reparatur zweimal echte Arbeit abgewiesen; das ist der bekannte
Preis eines Wachhunds ohne Gegenprobe.

---

## 6. Was ausdrücklich NICHT vorgeschlagen wird

| Nicht | Warum |
|---|---|
| Generische `ui-ux-designer` / `accessibility-specialist`-Agenten (liegen im Plugin-Cache) | Sie bringen **fremde Design-Doktrin** mit. Das verletzt Regel 7 (Kein Kontext-Import) und arbeitet gegen `DESIGN-GESETZE.md`. Die Methodik dieses Projekts ist vollständig — sie braucht Vollzug, keine zweite Meinung. |
| Ein Token-/Design-System-Skill | Phase 3 ist weit weg. Tokens ohne abgenommene Richtung sind Erfindung. |
| Irgendein Material-, Licht- oder Korn-Werkzeug | Phase 1 läuft. Material ist dort kein Gegenstand, **auch nicht als Messung**. Siehe den dreifachen Reihenfolgefehler in `werkzeug/LIES-MICH.md`. |
| Ein stehender Prüf-Schwarm (8 parallele Agenten) | Hat als **Sonderrunde** funktioniert. Als Ritual widerspricht er *„ein Schritt pro Runde"* und produziert zehn gleichzeitige Entscheidungen. |
| Ein Screenshot-Skill | Playwright-MCP reicht. Die PRÜFLISTE sagt: Messung schlägt Screenshot. |
| Ein `/interview`-Ersatz | Existiert, ist gut, wird nicht angefasst. |

---

## 7. Reihenfolge, in der gebaut würde

1. **`zahlen-pruefer`** — kleinster Bau, häufigster Fehler, wirkt sofort auf
   jedes Dokument.
2. **Hook `blattpruefung`** + Gegenprobe — macht aus einer Regel einen Zwang.
3. **`/aufgabenliste`** und **`/klemme`** — der Editor braucht beide in der
   nächsten Runde, und zwar in dieser Reihenfolge.
4. **`luegen-detektiv`** — sobald das erste Editor-Blatt steht.
5. **`/blatt`**, **`/abnahme`**, **`/sessionende`**, **`abnahme-waechter`**.
6. **`/variantenrunde`**, **`spezifikations-deckung`**, Hook `standband-wahrheit`.

---

## 8. Formatfakten, damit sie niemand neu raten muss

Verifiziert am **2026-08-21** an `code.claude.com/docs` (searx war offline,
HTTP 000) sowie an der installierten CLI **2.1.238**.

**Skill (`SKILL.md`)** — alle Felder optional, `description` empfohlen:
`name` · `description` · `when_to_use` · `argument-hint` · `arguments` ·
`disable-model-invocation` · `user-invocable` · `allowed-tools` ·
`disallowed-tools` · `model` · `effort` · `context: fork` · `agent` ·
`background` · `hooks` · `paths` · `shell` · `metadata` · `license` ·
`compatibility`.
`description` + `when_to_use` werden in der Skill-Liste bei **1.536 Zeichen**
abgeschnitten. `${CLAUDE_SKILL_DIR}` und `${CLAUDE_PROJECT_DIR}` werden im
Skill-Text **und** in `allowed-tools`-Bash-Regeln ersetzt.

**Agent (`.claude/agents/*.md`)** — `name` und `description` sind **Pflicht**,
alles andere optional: `tools` · `disallowedTools` · `model` · `permissionMode` ·
`maxTurns` · `skills` · `mcpServers` · `hooks` · `memory` · `background` ·
`effort` · `isolation` · `color` · `initialPrompt`.

**Hook-Ereignisse — die hier einschlägigen, keine vollständige Liste.**
Blocken **können** (Exit 2): `PreToolUse` · `UserPromptSubmit` · `Stop` ·
`SubagentStop` · `PostToolBatch` · `PreCompact` · `ConfigChange` ·
`UserPromptExpansion` (die Doku nennt weitere: `Elicitation`, `TaskCreated`,
`TaskCompleted`, `TeammateIdle`, `WorktreeCreate` — Letzteres scheitert bei
**jedem** Exit ≠ 0).
Blocken **nicht**: `PostToolUse` · `PostToolUseFailure` · `SessionStart` ·
`SessionEnd` · `SubagentStart` · `Notification`.

---

## Was diese Datei nicht ist

Keine Abnahme. Der User hat weder die Auswahl noch die Reihenfolge beurteilt.
Was hier steht, ist eine Empfehlung mit Belegen — gebaut wird erst nach seinem
Wort.
