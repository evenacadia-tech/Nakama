# tools/fl — Laufzeit-Arm

Misst den gebauten Stand im echten FL Studio (Plan S25e, Register NAK-286,
KONZEPT §4.6). Dateien:

| Datei | Rolle |
|---|---|
| `laufzeit.ps1` | Ablauf je Ticket: lohnt es (Diff), Installation über `\Nakama\installieren` mit Wertung von `\Nakama\pruefen`, Controller-Skript gegen das MCP-Repo, Projektordner mit SHA-256, Briefkasten-Ordner, Render für `nulltest-host`, loopMIDI vor FL, Restprozesse, FL-Start mit `eq-copilot/fixtures/fl/Nakama-Diagnose.flp`, Ping, Szenarien, Rückweg nach ABWEICHUNG, Rohdatei und Kopfzeile; `-Selbsttest` gegen Attrappen |
| `szenario.py` | fährt eine Szenariodatei aus `docs/gesundheit/szenarien/` über die Bibliothek des FL-Studio-MCP (`C:\Users\phili\Projekte\fl-studio-mcp`) und die lokalen Aktionen `lokal.*`, schreibt jeden Schritt roh; `--selbsttest`, `--rechne` (Rechnung aus F-28), `--energieprofil` (Stellen für U40) |
| `nulltest.py` | Nulltest im Host: Render gegen `eq-copilot/kalibration/Testtrack.wav` — Format, Versatz über die volle Songlänge, Urteil, `ergebnis.json`; `--selbsttest` |
| `selbsttest.py` | Bein A35: die drei Selbsttests ohne FL (Exit 0 grün, 4 rot, 2 Werkzeugfehler) |

Aufruf des Dirigenten nach grünem Kanon (Skill §3.3):

```powershell
pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Ticket <TICKET> -Basis <basis-sha>
```

Exit 0 = gemessen oder begründet übersprungen, 3 = Voraussetzung fehlt (auch
eine Szenario-Voraussetzung: Szenario-Exit 5), 4 = Szenario verfehlt oder
Diagnoseprojekt verändert (Nacharbeit). Rohdatei
`docs/beweise/roh/<TICKET>-laufzeit-<sha>.md`; die letzte stdout-Zeile
`LAUFZEIT …` ist die Kopfzeile fürs Manifest. Log:
`%LOCALAPPDATA%\evenacadia\nakama-laufzeit\laufzeit.log`.

## Ablauf

1. Diff-Entscheid: ohne Produktpfad `UEBERSPRUNGEN`.
2. Installation: `--hashen`, `\Nakama\installieren`, `\Nakama\pruefen`; nur
   „aktuell" für jedes Artefakt lässt den Lauf weiter, sonst Exit 3.
3. Controller-Skript: SHA-256 von `fl_controller/device_FLStudioMCP.py` gegen
   die installierte Kopie; bei Abweichung `setup-local.ps1`, Diagnose-FL
   beenden und eine Boot-Marke mit der `script_version` des Repo-Stands
   verlangen.
4. Projektordner unter `%LOCALAPPDATA%\evenacadia\nakama-laufzeit\projekt\`
   mit SHA-256 von Repo-Projekt und Arbeitskopie; jeder Ausgang vergleicht
   beide.
5. Briefkasten `%LOCALAPPDATA%\evenacadia\nakama\diagnose\` mit `antwort\`:
   zuerst eine liegende Anfrage, dann Antworten früherer Läufe entfernen (nur
   Antwort- und Temp-Namen der Instanzen); jeder Ausgang räumt `anfrage.json`
   ab.
6. Render (nur mit `nulltest-host.json`): `FL64.exe /R /Ewav /O"<render>"
   "<Arbeitskopie>"` nach dem FL-Handbuch („Exporting Audio & MIDI"), ohne FL
   mit Fenster, Frist 600 s, danach genau diese PID beendet; `render.json` im
   Ordner `…\nakama-laufzeit\render\` nennt Datei, Dauer, Exit, Fenstertitel
   oder Grund.
7. loopMIDI, Restprozesse, FL-Start, Boot-Marke, Ping.
8. Szenarien alphabetisch; `frischer_start` startet das Diagnose-FL vorher neu.
   Szenario-Exit 3 bricht ab, 5 lässt die Folge weiterlaufen. ABWEICHUNG im
   Nulltest: Diagnose-FL beenden, messen, dass kein FL läuft, `\Nakama\rueckweg`,
   `\Nakama\pruefen`; die Folgeszenarien entfallen mit Meldung, Exit 4.

## Lokale Aktionen

| Aktion | Wirkung |
|---|---|
| `lokal.briefkasten` | eine Anfrage (128 Bit Zufall), Antworten je Rolle sammeln (Frist 10 s), Auswahl an Name und Größe vor dem Öffnen (2 B bis 16 MiB, keine `.tmp-`), nur aktuelle Kennung und Diagnose-PID; der Broker nur, wenn `eqcop-broker` läuft — der Runner startet ihn nie |
| `lokal.nulltest` | `nulltest.py` über `render.json`; `verarbeitung_ein` und `ohne_slots` brauchen die Projekte aus Karte U43 |
| `lokal.fenster` | Bild des FL- oder Plugin-Fensters über `PrintWindow` ohne Vordergrundwechsel unter `…\nakama-laufzeit\bilder\`; einfarbig oder minimiert ist verfehlt |
| `lokal.umlauf` | Anfragen im Sekundentakt bis zum Wrap; gewertete Antwort je Rolle, Materialzeit, Fortlaufbedingung, Kopfverlust K, Referenzausschnitt, Rechnung aus F-28 (`py -3.13 szenario.py --rechne`), Bänder und Zuordnung |
| `lokal.stellen` | Karte U40: je Stelle Lage im Host mit v, Zeitplan aus den Takten der Briefkästen, Zählung nach dem Materialausschnitt des Rahmens, Rohzeilen, Differenzen kumulativer Zähler, Plausibilität ohne Sollwert |

## Voraussetzungen je Rechner

loopMIDI installiert, Aufgaben `\Nakama\*` registriert
(`tools/dirigent/install-aufgaben-registrieren.ps1`, einmal erhöht), FL Studio
2026 mit dem Controller „FL Studio MCP Controller" auf dem loopMIDI-Port
(Registry, einmalig), `uv` im Pfad, das MCP-Repo unter dem Standardpfad
(`-McpRepo` sonst): privates Repo `https://github.com/evenacadia-tech/fl-studio-mcp`,
Zweig `evenacadia-local`, nach `C:\Users\phili\Projekte\fl-studio-mcp` klonen
und dort einmal `setup-local.ps1` ausführen (Controller-Skript in FLs
Settings-Ordner). Für die Rechnung aus F-28
Python 3.13 mit numpy, scipy, soundfile, pyloudnorm und librosa sowie
`C:\Users\phili\FL-Studio\tools\analyze-track.py`.

## Gemessene Fallen

Gemessen ab 12.09.2026 mit FL 26.1.4.5589:

- Ein MIDI-Trigger, der FL während der Initialisierung trifft, lässt die
  Instanz danach dauerhaft stumm (Timeouts bei jedem weiteren Befehl, nur ein
  FL-Neustart hilft). Der Runner sendet deshalb vor der Boot-Marke
  `mcp_boot.json` des Controller-Skripts plus acht Sekunden Schonfrist keinen
  Trigger. Wer von Hand misst: nach dem FL-Start mindestens 15 s warten.
- FL sieht nur MIDI-Ports, die beim Start existieren: loopMIDI vor FL.
- `ui.getProgTitle()` liefert nur „FL Studio 2026"; das offene Projekt steht
  im Hauptfenster der Klasse `TFruityLoopsMainForm`. `MainWindowTitle` kann
  leer sein (TApplication-Fenster, NAK-297); der Runner liest das Hauptfenster.
- Die Antwort von `transport.setPosition` las die Position im selben Aufruf
  und war veraltet (13.09.2026: `1:01:00` nach Setzen von 2 s,
  `docs/beweise/NAK-286.md` §0.5 Punkt 1). Seit Controllerfassung 2026-09-15
  nennt sie nur den angeforderten Wert; die Position liefert
  `transport.getPosition` in einem eigenen Befehl (`ms`, ganze `seconds`).
- `channels.getChannelVolume(index, mode, useGlobalIndex)`: das zweite
  Argument ist der dB-Modus, nicht der globale Index.
- `channels.getInfo` mit globalem Index eines Kanals außerhalb der aktuellen
  Gruppe meldet „Index out of range"; `channels.getAll` führt ihn trotzdem.
  Die Szenarien fragen gruppenlokal (`use_global: false`).
- Probeeq antwortet zwei Briefkastentakte nach der Anfrage (Flag, dann
  Schreiben); eine neue Kennung verwirft eine wartende. Anfragen folgen
  deshalb erst nach allen erwarteten Antworten.
- Die Taktphase einer Instanz steht in der Schreibzeit ihrer Antwortdatei
  (15.09.2026: Gen je Sekunde zur selben Millisekunde, ±5 ms); der Eingang beim
  Runner liegt bis 0,1 s später. Ein Rahmen ist bis 0,64 s lang (28 244
  Samples), und jeder MCP-Befehl blockiert den Runner bis zur Antwort
  (`transport.start`, die Positionsabfrage vor jeder Anfrage). `lokal.stellen`
  plant danach; trägt ein gemeinsamer Plan nicht jede Kombination dieser
  Grenzen, misst jede Rolle in einem eigenen Durchlauf über die Stelle.
- `tasklist` schreibt in der OEM-Codepage; die deutsche Meldung ohne Treffer
  trägt einen Umlaut, mit `text=True` kam `stdout` als `None`. Der Runner liest
  Bytes und dekodiert mit Ersetzung.
- ctypes-Funktionszeiger sind nicht hashbar: eine Prototypentabelle als `dict`
  scheiterte in FL mit „unhashable type" (MCP `utils/fenster.py`, 15.09.2026).
- `Testtrack.wav` ist ein Maschinenartefakt (gitignoriert); der Runner sucht
  es in `eq-copilot/kalibration/` des Repos, im sichtbaren Checkout und im
  Desktop-Projekt des Users, sonst `-Sample <pfad>`.

## Grenzen

Render mit eingeschalteter Verarbeitung und ohne Nakama-Slots (Weg R2)
brauchen die Projekte aus Karte U43. Im Diagnoseprojekt antwortet kein Broker,
solange Gen dort `legacy` ist. Der Samplepfad im Projekt ist absolut
(`eq-copilot/fixtures/fl/LIES-MICH.md`); auf einem zweiten Rechner ist der
FL-Dialog „Sample nicht gefunden" zu prüfen. Bänder gegen den Referenzausschnitt
lösen Exit 4 nur mit einer Rechnung aus F-28 aus; ohne sie stehen die Werte roh.
