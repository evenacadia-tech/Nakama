# Nachtrag für den Dirigenten-Skill: Laufzeit-Arm (12.09.2026)

**Eingearbeitet 12.09.2026 (Dirigent, Abschlussfenster NAK-283 Etappe 1): alle vier Einfügungen stehen im Skill; diese Datei ist Verlauf.**

Anlass: User-Wort 12.09.2026 im Register NAK-286 (fester Bestandteil,
automatisch, ohne Nachfrage, immer wenn lohnenswert). Register, Plan S25e
und KONZEPT §4.6/§10 sind nachgezogen (Session `nakama-a7`). Den Skill
selbst ändert der Dirigent im nächsten Abschlussfenster; diese Datei trägt
den exakten Wortlaut der vier Einfügungen und ist danach Verlauf.

## 1. §3.3 „Messen" — neuer Aufzählungspunkt nach der Rundenbilanz

```markdown
- **Laufzeit-Arm** (User 12.09.2026, Register NAK-286, Plan S25e): nach
  grünem Kanon auf dem End-Stand `pwsh -NoProfile -File
  tools/fl/laufzeit.ps1 -Ticket <TICKET> -Basis <basis-sha>`. Der Runner
  entscheidet selbst, ob der Diff lohnt (Plugin, Schemas,
  Broker, Installer), zieht die Manifest-Hashes nach, installiert über die
  erhöhte Aufgabe `\Nakama\installieren`, startet FL mit dem Diagnoseprojekt
  und fährt die Szenarien. Exit 0 = gemessen oder begründet übersprungen
  (Diff ohne Produktpfad, fremdes FL-Projekt offen), Exit 4 = Szenario
  verfehlt → Nacharbeit wie ein Codex-Defekt. Rohdaten
  `docs/beweise/roh/<TICKET>-laufzeit-<sha>.md`, Kopfzeile ins Manifest.
  FL-Start, Installation und Messung sind nie Haltgrund und nie Frage.
```

## 2. §3.5 „Abschluss" — Satz nach „Befund gegen den Runner … im Manifest nur die Kopfzeile."

```markdown
Nach `EXIT=0` folgt der Laufzeit-Arm (§3.3); die dabei nachgezogenen Hashes
in `eq-copilot/install/nakama-installer-v1.json` gehören zu den
Abschlussdateien.
```

## 3. §4 „Haltgründe" — erste Zeile ersetzen

Alt:

```markdown
- User-, Figma-, FL- oder Installationsschritt,
```

Neu:

```markdown
- User- oder Figma-Handgriff (FL-Start, Installation und Messung laufen
  seit 12.09.2026 automatisch, §3.3, und sind keiner; einziger FL-Handgriff
  bleibt das Diagnoseprojekt nach NAK-30 neu anzulegen),
```

## 4. §6 „Was der Dirigent nie tut" — letzter Punkt

```markdown
- Den Laufzeit-Arm auslassen oder den User nach FL, Installation oder
  Messung fragen.
```

Prüfung danach: `py -3.13 tools/plan/dokuriegel.py .claude/skills/dirigent/SKILL.md`
(`tools/fl/laufzeit.ps1` liegt seit 12.09.2026 vor) und die Byte-Grenze aus
`docs/context-hygiene-playbook.md`.
