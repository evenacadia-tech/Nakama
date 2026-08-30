# Prüfauftrag-Vorlage für Codex (Dirigent §3.4, seit 30.08.2026)

Der Dirigent füllt die Platzhalter `{{…}}` und übergibt den Text als
`$reviewPrompt`. Nichts wird weggelassen; die Ausschlüsse sind der Kern der
Vorlage. Zwei Varianten: **Erst-/Abschlussprüfung** (ganzer Ticketbereich,
nur die Ticketpfade) und **Wiederprüfung** (nur der Fixdiff der Runde).

Vorgeschichte: S8 `SONDE-007a` lief 19 Prüfrunden, weil der Prüfer den
ganzen, wachsenden Bereich `basis...HEAD` (522 Dateien) samt Manifesten las
und jede Runde einen neuen Umgehungsweg des Prüfskripts fand — ohne dass
sich eine Zeile Produktcode änderte. Der gebundene Abschlussprüfer mit dieser
Vorlage brauchte 334 Sekunden und gab PASS.

---

## Variante A — Erst- oder Abschlussprüfung

```text
{{ART}} {{SCHRITT}} / Ticket {{TICKET}} — gebundener Prüfauftrag des Dirigenten ({{DATUM}})

Du bist der frische, lesende Prüfer. Prüfbereich ist ausschließlich der Änderungssatz dieses Tickets:
`git diff {{BASIS}}...{{ZIEL}} -- {{PFADE}}`
mit BASIS = {{BASIS}} und ZIEL = {{ZIEL}} (HEAD; er muss vor und nach deinem Lauf identisch sein).
Alles andere, was seit BASIS im Repo geändert wurde, gehört zu anderen Tickets und ist hier nicht Prüfgegenstand.
Prüfstufe {{STUFE}}, verlangt vom Plan für {{SCHRITT}}.

GATE-TEXT, wörtlich und vollständig ({{GATE_QUELLE}}):
„{{GATE_TEXT}}"

Mehr verlangt das Gate nicht. {{MATRIX_SATZ}}
Die Riegel/Tests, die es messen: {{RIEGEL}}. Jüngster Kanon: {{KANON}}.

BEFUNDKLASSEN — du ordnest jeden Befund ein, der Dirigent prüft die Einordnung an der Quelle:
- DEFEKT: verletzt einen Satz des Gate-Textes oben, eine Zeile der Verhaltensmatrix, einen Test oder eine Invariante aus CLAUDE.md (Identität eingefroren, Audio echtzeitfest, Nulltest bitidentisch, State verlustfrei). Reproduzierbar, mit Kommando; je Defekt: Datei:Zeile und der Satz, der bricht.
- LÜCKE: Gate, Matrix und Entwurf sagen zu dem Fall nichts — benennen, keine Nacharbeit.
- HÄRTUNG: wünschenswert, von keiner Zusage verlangt — benennen, keine Nacharbeit.

AUSDRÜCKLICH AUSGESCHLOSSEN (kein Befund, keine Erwähnung als NEEDS_WORK-Grund):
1. docs/** — Manifeste, Register, Planstand, Prosa, Zeilenverweise, Trefferzahlen, Abschnittsstände, Kommentar-Wortlaute. Manifeste sind Kontext, kein Prüfgegenstand.
2. Im Register datierte Härtungen: {{AUSSCHLUESSE}}. Ein Befund gegen ein Prüfskript, einen Riegel oder den Runner, der absichtliche Sabotage in repo-eigenen Quellen voraussetzt, ist HÄRTUNG — das Bedrohungsmodell der Riegel ist die versehentliche Regression, nicht der Angreifer mit Schreibrecht.
3. tools/dirigent/pruefliste.md ist eine Arbeitsliste des Workers, keine Anforderungsquelle. §2.4 des Codex-Skills sondenplan-audit („Wiederholung bis lückenlos") gilt für Gate-Audits, nicht für diesen Ticketreview. Kosmetik, Stil, optionale Härtung, theoretische Randfälle, Ticketfremdes: nein.

WAS DU TUST:
1. Lies den Diff im Prüfbereich.
2. Prüfe am Zielstand, ob Tests und Riegel messen, was Gate und Matrix verlangen: {{PRUEFFRAGEN}}. Du darfst lesen und lesende Kommandos fahren; du schreibst nichts.
3. Prüfe, ob der Änderungssatz seit BASIS etwas am Produktverhalten gebrochen hat (Identität, Passthrough/Nulltest, State, Audio-Thread) — nenne, was du selbst nachgemessen und was du nur gelesen hast.

URTEIL — genau eine der drei Formen, als erste Zeile:
URTEIL: PASS — kein DEFEKT (Lücken und Härtungen dürfen benannt sein).
URTEIL: NEEDS_WORK — mindestens ein DEFEKT; je Defekt: Datei:Zeile, Reproduktion, welcher Satz von Gate oder Matrix bricht.
URTEIL: BLOCKED — du konntest den Prüfbereich nicht lesen.
Danach: geprüft / nicht geprüft, je ein Absatz.
```

## Variante B — Wiederprüfung nach einer Nacharbeitsrunde

```text
Wiederprüfung {{SCHRITT}} / Ticket {{TICKET}}, Runde {{RUNDE}} — gebundener Prüfauftrag des Dirigenten ({{DATUM}})

Du bist der frische, lesende Prüfer dieser Runde. Prüfbereich ist ausschließlich der Fixdiff der Runde:
`git diff {{VORHER}}...{{ZIEL}} -- {{PFADE}}`
(ZIEL = HEAD, vor und nach deinem Lauf identisch). Der übrige Ticketbereich wurde bereits geprüft und ist hier nicht Prüfgegenstand.

BEFUNDE, die diese Runde schließen sollte (wörtlich aus dem vorigen Urteil):
{{BEFUNDLISTE}}

GATE-TEXT, wörtlich ({{GATE_QUELLE}}): „{{GATE_TEXT}}"

Du beantwortest genau zwei Fragen:
1. Ist jeder der genannten Befunde am Zielstand geschlossen (je Befund: ja/nein, mit Beleg)?
2. Hat der Fixdiff etwas gebrochen — einen Test, eine Matrixzeile, einen Satz des Gate-Textes, eine Invariante aus CLAUDE.md?

Neue Befunde außerhalb dieser zwei Fragen erhebst du nicht. Befundklassen und Ausschlüsse wie in Variante A (docs/** ausgeschlossen; Härtungen {{AUSSCHLUESSE}}; Prüfliste keine Anforderung; Sabotage-Szenarien gegen Prüfwerkzeuge = Härtung).

URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.
URTEIL: NEEDS_WORK — je offenem Befund oder Bruch: Datei:Zeile, Reproduktion, welcher Satz bricht.
URTEIL: BLOCKED — Prüfbereich nicht lesbar.
Danach: geprüft / nicht geprüft, je ein Absatz.
```

## Platzhalter

| Platzhalter | Inhalt |
|---|---|
| `{{ART}}` | `Erstprüfung` oder `Abschlussprüfung` |
| `{{SCHRITT}}`, `{{TICKET}}` | z. B. `S8`, `SONDE-007a` |
| `{{BASIS}}`, `{{ZIEL}}`, `{{VORHER}}` | volle oder 7-stellige SHAs; `VORHER` = Stand vor der Runde |
| `{{PFADE}}` | die Pfade des Tickets, durch Leerzeichen getrennt — aus `git diff --stat` der Ticket-Commits, nie `.` |
| `{{STUFE}}` | `T2` oder `T3` laut `docs/plan/plan.json` |
| `{{GATE_QUELLE}}`, `{{GATE_TEXT}}` | Fundstelle und wörtlicher Satz |
| `{{MATRIX_SATZ}}` | `Die Verhaltensmatrix steht in <Manifest>, Abschnitt <…>; sie ist Referenz.` oder `Das Ticket hat keine Verhaltensmatrix.` |
| `{{RIEGEL}}`, `{{KANON}}` | Tests/Riegel mit Datei; Kanonbilanz mit Roh-Datei |
| `{{AUSSCHLUESSE}}` | Register-IDs der datierten Härtungen, z. B. `NAK-103 (K1b eingefroren)` |
| `{{PRUEFFRAGEN}}` | zwei bis vier konkrete Fragen, die das Gate stellt |
| `{{BEFUNDLISTE}}` | die Befunde des vorigen Urteils, wörtlich |
