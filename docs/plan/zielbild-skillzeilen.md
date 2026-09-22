# Zielbild — Wortlaut für die Skills (Übergabe an den Dirigenten)

Vereinbart mit dem Dirigenten am 23.09.2026 (Antwort auf `nakama-1f`: (a) `/fragen` zieht
im selben Änderungssatz nach, (b) Exit 4 zieht der Dirigent im Abschlussfenster selbst nach,
(c) die Skillzeilen schreibt der Dirigent). Der Dirigent übernimmt die Zeilen unten wörtlich
und löscht diese Datei im selben Commit. Hintergrund: `docs/plan/LIES-MICH.md`, Abschnitt
„Das Zielbild".

## 1. `.claude/skills/dirigent/SKILL.md` §0, Absatz „Plan-Tab und STAND"

Nach dem Satz „`start-dirigent.ps1 -NurPlanTab` öffnet ihn neu." einfügen:

```text
Taste Z zeigt dort das Zielbild (`docs/ZIELBILD.md`), P den Plan (23.09.2026).
```

## 2. `.claude/skills/dirigent/SKILL.md` §3.5, Absatz „Hygiene in jedem Abschlussfenster"

Nach „`py -3.13 tools/plan/tidy.py` (clang-tidy-Ratsche, Bein A33)" einfügen:

```text
, `py -3.13 tools/plan/zielbild.py pruefen` (Exit 4: Zielbild selbst
nachziehen, `abgleichen`, Pathspec-Commit)
```

und im selben Satz „beide nicht blockierend" durch „alle nicht blockierend" ersetzen.

## 3. `.claude/skills/fragen/SKILL.md` §5, nach dem Absatz „Der Planstand selbst …"

```text
Jede Antwort zieht im selben Änderungssatz `docs/ZIELBILD.md` nach (Stelle in
Alltagssprache; neue Karte unter „Noch offen"), dann `py -3.13 tools/plan/zielbild.py abgleichen`.
```

## 4. `.claude/skills/fragen/SKILL.md` §6, Beispielzeile

```text
git add -- docs/plan/fragen.json docs/ZIELBILD.md <konkreter Entscheidungsort> <weitere betroffene Pfade>
```

## Messung

Mehrbytes gegen `tools/plan/gesundheit.py` (Dirigenten-Skill, Grenze 24 576 Bytes, Stand
vor der Übernahme 24 100): Stelle 1 und 2 zusammen rund 200 Bytes.
