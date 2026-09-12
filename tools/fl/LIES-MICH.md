# tools/fl — Laufzeit-Arm

Misst den gebauten Stand im echten FL Studio (Plan S25e, Register NAK-286,
KONZEPT §4.6). Zwei Dateien:

| Datei | Rolle |
|---|---|
| `laufzeit.ps1` | Ablauf je Ticket: lohnt es (Diff), Installation über `\Nakama\installieren` (NAK-285), loopMIDI vor FL, Restprozesse, Projektordner, FL-Start mit `eq-copilot/fixtures/fl/Nakama-Diagnose.flp`, Ping, Szenarien, Rohdatei und Kopfzeile |
| `szenario.py` | fährt eine Szenariodatei aus `docs/gesundheit/szenarien/` über die Bibliothek des FL-Studio-MCP (`C:\Users\phili\Projekte\fl-studio-mcp`) und schreibt jeden Schritt roh |

Aufruf des Dirigenten nach grünem Kanon (Skill §3.3):

```powershell
pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Ticket <TICKET> -Basis <basis-sha>
```

Exit 0 = gemessen oder begründet übersprungen, 3 = Voraussetzung fehlt,
4 = Szenario verfehlt (Nacharbeit). Rohdatei
`docs/beweise/roh/<TICKET>-laufzeit-<sha>.md`; die letzte stdout-Zeile
`LAUFZEIT …` ist die Kopfzeile fürs Manifest. Log:
`%LOCALAPPDATA%\evenacadia\nakama-laufzeit\laufzeit.log`.

Voraussetzungen je Rechner: loopMIDI installiert, Aufgaben `\Nakama\*`
registriert (`tools/dirigent/install-aufgaben-registrieren.ps1`, einmal
erhöht), FL Studio 2026 mit dem Controller „FL Studio MCP Controller" auf
dem loopMIDI-Port (Registry, einmalig), `uv` im Pfad, das MCP-Repo unter dem
Standardpfad (`-McpRepo` sonst).

Gemessene Fallen (12.09.2026, FL 26.1.4.5589):

- Ein MIDI-Trigger, der FL während der Initialisierung trifft, lässt die
  Instanz danach dauerhaft stumm (Timeouts bei jedem weiteren Befehl, nur
  ein FL-Neustart hilft). Der Runner sendet deshalb vor der Boot-Marke
  `mcp_boot.json` des Controller-Skripts plus acht Sekunden Schonfrist keinen
  Trigger. Wer von Hand misst: nach dem FL-Start mindestens 15 s warten.
- FL sieht nur MIDI-Ports, die beim Start existieren: loopMIDI vor FL.
- `ui.getProgTitle()` liefert nur „FL Studio 2026"; das offene Projekt steht
  im Fenstertitel (`Nakama-Diagnose.flp - FL Studio 2026`).
- `channels.getInfo` mit globalem Index eines Kanals außerhalb der aktuellen
  Gruppe meldet „Index out of range"; `channels.getAll` führt ihn trotzdem.
  Das Szenario fragt gruppenlokal (`use_global: false`).
- `Testtrack.wav` ist ein Maschinenartefakt (gitignoriert); der Runner sucht
  es in `eq-copilot/kalibration/` des Repos, im sichtbaren Checkout und im
  Desktop-Projekt des Users, sonst `-Sample <pfad>`.

Grenzen der ersten Fassung: kein Screenshot, kein Nulltest im Host, kein
Diagnose-Briefkasten; der Samplepfad im Projekt ist absolut
(`eq-copilot/fixtures/fl/LIES-MICH.md`), auf einem zweiten Rechner ist der
FL-Dialog „Sample nicht gefunden" zu prüfen. Der Installationsschritt
(`--hashen`, Aufgabe `\Nakama\installieren`) ist gebaut, aber noch nicht im
Lauf gemessen; der erste Lauf mit Installation ist Teil von S25e.
