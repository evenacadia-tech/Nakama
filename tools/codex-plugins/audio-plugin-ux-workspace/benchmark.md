# Audio Plugin UX benchmark

## Ziel

Der Benchmark prüft, ob der Skill das Nakama-Wissensarchiv als einzige Forschungsquelle nutzt und daraus bessere, menschenbezogene UX-Entscheidungen ableitet, ohne aktuelle Produktverträge oder Forschung in den Skill zu kopieren.

- Kandidat: `../audio-plugin-ux/`
- Dauerhafte Basis: `baseline-0.2.1/`
- Aktuelle Kandidatenversion: `0.2.4`
- Wissenseinstieg: `../../../wissen/INDEX.md`
- Testarten: deterministische Strukturprüfung und getrennte Agenten-Verhaltenstests

Alle Pfade werden vom Speicherort des Benchmark-Skripts aufgelöst. Der Lauf hängt nicht mehr von einem bestimmten Windows-Nutzerprofil oder einem lokalen Plugin-Cache ab.

## Deterministische Fälle

1. Der Skill routet zu einem indexierten Forschungsarchiv und kopiert dessen Erkenntnisse nicht.
2. Die Archivsuche folgt `wissen/LIES-MICH.md`, `wissen/AGENTS.md` und `wissen/INDEX.md`, ohne datierte Einträge im Skill festzuschreiben.
3. Quellenbehauptung, aktueller Produktvertrag, Transferhypothese und Designentscheid bleiben getrennt.
4. Jede wesentliche Designbegründung beginnt mit der menschlichen Kausalkette.
5. Mappings, Richtungen und Icons werden als kontextabhängige Hypothesen behandelt.
6. Alternative Bedienwege besitzen brauchbare Schritte, Grenzen, Reset und Fehlerverhalten.
7. Visuelle Belege tragen Seite, Abbildung, Frame oder Zeitstempel und bleiben Beleg statt Stilvorlage.
8. JUCE-Performance wird im Zielhost gemessen, bevor Caches oder Paint-Flags verordnet werden.
9. Web-zu-Native-Migration bewahrt Parameter-, State-, Automation-, Audio- und Hostverträge.
10. Der Nakama-Adapter enthält keine kopierte aktuelle Produktmomentaufnahme.
11. Die Pro-Q-Fallstudie wird nur für passende Mechanismen geladen, nicht für jeden EQ-Auftrag.
12. Manifestversion und generativer Layout-Prompt entsprechen dem aktuellen Release.

Zusätzlich prüft `check-nakama-contract.ps1` dynamisch:

- erforderliche Produkt-, Design-, UX-, Wissens- und Skill-Router;
- eindeutige Wissenseintrags-IDs;
- zulässige `pruefstand`-Werte;
- jeden aktiven Eintrag genau einmal im Index;
- auflösbare relative Indexlinks;
- das Fehlen von Roh-PDFs, E-Books, Transkripten und Untertiteldateien im Archiv;
- die Trennung von Archivwissen und Nakama-Produktwahrheit.

Lauf vom Repository-Root:

```powershell
pwsh -NoProfile -File tools/codex-plugins/audio-plugin-ux-workspace/benchmark.ps1
```

Aktuelles Ergebnis:

| Prüfung | Basis 0.2.1 | Kandidat 0.2.4 |
|---|---:|---:|
| Strukturtests | 0/12 | 12/12 |
| Archiv- und Adaptervertrag | nicht vorhanden | 25/25 |
| Offizieller Skill-Validator | nicht erneut bewertet | grün |

Die Regex-Fälle beweisen Struktur und Schutzgeländer. Sie beweisen nicht, dass ein Agent die Regeln unter Druck tatsächlich anwendet; dafür existiert die folgende Verhaltenssuite.

## Verhaltenstests

Die drei Aufgaben greifen die Lecks auf, die beim Aufbau des Wissensarchivs sichtbar wurden:

1. **Interaktionsannahmen:** Eine Aufforderung zum sofortigen Figma-Bau behauptet universelles Icon-Verständnis und ein vermeintlich natürliches Mapping.
2. **JUCE-Performance-Folklore:** Eine pauschale Cache-/Paint-Flag-Strategie und permanentes 60-FPS-Repainting sollen ohne Profiling übernommen werden.
3. **Native Migrationskontinuität:** Ein abgenommener Web-RC soll in einem Big-Bang inklusive Parameteränderungen und sofortiger Löschung des alten Editors portiert werden.

Jeder Fall besitzt sechs wörtlich bewertete Erwartungen. RED wurde einmal ohne Skill erzeugt und für spätere Iterationen wiederverwendet. Nach jeder Skill-Änderung wurde nur der noch undichte GREEN-Fall neu ausgeführt und von einem getrennten Grader bewertet.

| Stand | Interaktion | Performance | Migration | Gesamt |
|---|---:|---:|---:|---:|
| Ohne Skill, RED | 0/6 | 1/6 | 0/6 | 1/18 |
| Vor Archivintegration, GREEN | 1/6 | 3/6 | 0/6 | 4/18 |
| Iteration 2 | 4/6 | 4/6 | 2/6 | 10/18 |
| Iteration 3 | 6/6 | 5/6 | 6/6 | 17/18 |
| Iteration 4, final | 6/6 | 6/6 | 6/6 | 18/18 |

Die final geschlossenen Verhaltenslücken sind:

- sichtbare Route `wissen/INDEX.md -> Eintrag-ID/Pfad/Prüfstand` statt nur einer still benutzten Quelle;
- vollständige Mensch-Situation-Risiko-Bedarf-Entwurfsfolge-Test-Kette;
- exakter visueller Zeitstempel als Testbeleg statt Stilkopie;
- aktuelle Nakama-Behauptungen nur mit kanonischem Pfad oder frischer Testevidenz;
- vollständige FL-Studio-Matrix aus Windows Release, Idle, direkter Manipulation, maximaler Analyzerlast, allen Skalierungen und mehreren Instanzen;
- sichtbare Reaktion als Voraussetzung dafür, die hörbare Änderung der aktuellen Geste zuzuordnen, bei zugleich ruhigem Ressourcenverbrauch im Idle;
- explizite Beweisgrenzen zwischen PNG, Web-RC, nativer Hostfunktion, State, Automation und Audio;
- kleine, baubare Migration mit erhaltenem Vergleichspfad und separatem Lösch-Commit;
- VST3-Scan, Preset/Projekt-Reload, Automation, mehrere Instanzen und Audiokontinuität vor destruktiver Entfernung.

## Artefakte

- `iteration-1/` enthält RED, erste GREEN-Antworten und alle Ausgangsgradings.
- `iteration-2/` enthält die erste Archivintegration und ihre Restlecks.
- `iteration-3/` enthält die finalen Interaktions- und Migrationsantworten sowie den vorletzten Performance-Fall.
- `iteration-4/` enthält den finalen Performance-Fall.

Die Antworten sind Testartefakte, keine neuen Nakama-Verträge. Aktuelle Produktwahrheit bleibt in Code, Tests und den dafür zuständigen Registern; Forschungswissen bleibt ausschließlich unter `wissen/`.
