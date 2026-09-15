# Tiefenaudit 3 — Befunde (kumulativ, Abarbeitung ab 18.09.2026)

**Auftrag (User, 15.09.2026, 17:3x Uhr, wörtlich):** „planänderung. kontingent bei 87 % beim nächsten workerende, kein weiteres bauticket. ich habe noch einen freien wochenreset für codex. ich will dass du stattdessen einen umfassenden codex prüfungslauf machst. aber ich rede nicht von einem, sondern mehrphasig, das gesamte plugin, blind spots, nahtstellen, alles was eben dazu gehört um eine astreine codebase zu garantieren. alle funde werden dann nach und nach in einem MD file festgehalten, welches wir dann ab freitag wenn claude wieder resetet wurde abarbeiten."

**Form:** Prüfsessionen nach `docs/gesundheit/KONZEPT.md` §4.3 und §5 (Spezialaudits und Tiefenaudit, vorgezogen vor G6), je Phase ein frischer Codex-Thread `gpt-6-astra` (Effort max, lesend; Schreibrecht nur für den eigenen Bericht unter `roh/` und den eigenen Abschnitt dieser Datei), gestartet vom Dirigenten über `tools/pruefung/codex-audit-lauf.ps1 -Phase <NN> -HeadSha <sha>`; Aufträge unter `auftraege/` (Kopf plus Phasenblock), der zusammengesetzte Auftrag je Lauf als Beleg unter `roh/phase-<NN>-auftrag.txt`, Schlussnachricht unter `roh/phase-<NN>-last.txt`. Kein Bau, kein Test, kein Kanon, keine Nacharbeit vor dem 18.09.2026; Einordnung Defekt/Lücke/Härtung durch den Dirigenten an der Quelle ab dem 18.09.2026 (Muster NAK-246/NAK-283: Quellvalidierung, Bündel als Tickets mit Matrix, Codex-Erstprüfung je Etappe). Befund-IDs `T3-<Phase>-<nn>` sind keine NAK-Nummern.

**Stand:** Basis der Prüfung ist der End-Stand von NAK-289 Etappe 1 (siehe Phasen-Tabelle, HEAD je Lauf). Produktcode bleibt während des Audits unverändert; jede Phase prüft HEAD vor und nach dem Lauf.

## Phasen

| Phase | Name | Klasse | Status | HEAD | Bericht |
|---|---|---|---|---|---|
| 00 | Erstprüfung NAK-289 Etappe 1 (Vorlage A, Ticketdiff) | Ticket | **PASS** 15.09.2026, Thread `01a0a5f8` | `c8286582` | `roh/phase-00-last.txt` |
| 01 | Echtzeit: Audio-Pfad Gen und Probeeq | K6 | offen | – | `roh/phase-01-echtzeit.md` |
| 02 | Zustandstreue: jedes persistente Feld | K4 | offen | – | `roh/phase-02-zustand.md` |
| 03 | Vertragstreue: v2/v3 beidseitig | K3 | offen | – | `roh/phase-03-vertrag.md` |
| 04 | Lebenslauf und Nebenläufigkeit, Naht zum Broker | K1, K2 | offen | – | `roh/phase-04-lebenslauf.md` |
| 05 | Hostkante und Nahtstellen: VST3/JUCE/FL, Sonde↔Gen, Identität, Installer | Naht | offen | – | `roh/phase-05-hostkante.md` |
| 06 | Messkern und Numerik | Numerik | offen | – | `roh/phase-06-messkern.md` |
| 07 | Modell und Fläche: Zwecktreue, Bedienehrlichkeit technisch | K8, Modell | offen | – | `roh/phase-07-modell.md` |
| 08 | Broker: Transport, Store, Coordinator, Sicherheit | Rust | offen | – | `roh/phase-08-broker.md` |
| 09 | Beweislücken: welcher Bug käme an allen Beinen vorbei | K5 | offen | – | `roh/phase-09-beweisluecken.md` |
| 10 | Skeptiker: jeden Befund widerlegen | alle | offen | – | `roh/phase-10-skeptiker.md` |
| 11 | Synthese: Bündel, Priorität, Abarbeitungsplan | alle | offen | – | `roh/phase-11-synthese.md` |

## Kopf

Wird von Phase 11 (Synthese) geschrieben: Urteil, Zähler nach Skeptiker, Bündeltabelle, Abarbeitungsliste, Detektorlücken, Blind Spots, Produktfragen. Bis dahin gilt: Befunde unten sind Codex-Urteile, nicht validiert.

## Phase 00 — Erstprüfung NAK-289 Etappe 1 (2026-09-15 18:48–18:59, HEAD c8286582)

URTEIL: PASS — kein DEFEKT (Codex `gpt-6-astra` max, lesend, Vorlage A, Thread `01a0a5f8-5b6a-7603-8c01-93253e901830`; HEAD vor und nach dem Lauf gleich; Auftrag `auftraege/phase-00.txt`, Schlussnachricht `roh/phase-00-last.txt`).

Geprüft laut Prüfer: vollständiger begrenzter Diff `432f209c...c8286582` über die 15 Ticketpfade; vorhandene Release-Tests für Nulltest, DSP, True Peak, IPC-Lifecycle, Vertrag, Identität und State grün; drei Terminate-Kinder je Exit 71; tidy-Selbsttest 39/39; alle 16 Mutations- und Rücknahme-Hashes der Rotbeweise bestätigt; Ratsche 58, keine Regeländerung, keine NOLINT-Ausnahme. Nicht erneut ausgeführt: Neubau, Vollkanon, A33-Vollmessung, Rotläufe, FL-Laufzeitprüfung (Belege gelesen). Befunde: keine (0 Defekte, 0 Lücken, 0 Härtungen).

