# tools/fl — Laufzeit-Arm

Misst den gebauten Stand im echten FL Studio (Plan S25e, Register NAK-286,
KONZEPT §4.6). Dateien:

| Datei | Rolle |
|---|---|
| `laufzeit.ps1` | Ablauf je Ticket: lohnt es (Diff), MCP-Stand gegen den Pin `mcp-stand.json`, Installation über `\Nakama\installieren` mit Wertung von `\Nakama\pruefen`, Controller-Skript gegen das MCP-Repo, Projektordner mit SHA-256, Briefkasten-Ordner, Renders für `nulltest-host` vor dem FL-Start (Diagnoseprojekt und Referenzprojekte, P-21), loopMIDI vor FL, eigene Restprozesse, FL-Start mit `eq-copilot/fixtures/fl/Nakama-Diagnose.flp`, Ping, Szenarien, Rückweg nach ABWEICHUNG, Rohdatei und Kopfzeile; beendet nur Eigenes (Besitzliste); `-Selbsttest` gegen Attrappen |
| `mcp-stand.json` | Pin des MCP-Repos: Zweig, Commit und Git-Blob-Kennung je Erstcode-Datei; ändert sich nur per Commit (Abschnitt MCP-Stand) |
| `szenario.py` | fährt eine Szenariodatei aus `docs/gesundheit/szenarien/` über die Bibliothek des FL-Studio-MCP (`C:\Users\phili\Projekte\fl-studio-mcp`) und die lokalen Aktionen `lokal.*`, schreibt jeden Schritt roh; `--selbsttest`, `--rechne` (Rechnung aus F-28), `--energieprofil` (Stellen für U40) |
| `nulltest.py` | Nulltest im Host: Render gegen `eq-copilot/kalibration/Testtrack.wav` — Format, Versatz über die volle Songlänge, Urteil, `ergebnis.json`; Weg R2 mit `--vergleich verarbeitung_ein` (gegen die Quelle, umgekehrt bewertet) und `--vergleich ohne_slots` (SHA-256 des Datenbereichs gegen den Auslieferungsrender); `--selbsttest` |
| `selbsttest.py` | Bein A35: die drei Selbsttests ohne FL (Exit 0 grün, 4 rot, 2 Werkzeugfehler) |

Aufruf des Dirigenten nach grünem Kanon (Skill §3.3):

```powershell
pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Ticket <TICKET> -Basis <basis-sha> -Beenden
```

`-Beenden` gehört zu jedem Aufruf: kein FL bleibt nach dem Lauf offen (User 22.09.2026, Register NAK-358).

Exit 0 = gemessen oder begründet übersprungen, 3 = Voraussetzung fehlt (auch
eine Szenario-Voraussetzung: Szenario-Exit 5), 4 = Szenario verfehlt oder
Diagnose- oder Referenzprojekt verändert (Nacharbeit). Rohdatei
`docs/beweise/roh/<TICKET>-laufzeit-<sha>.md`; die letzte stdout-Zeile
`LAUFZEIT …` ist die Kopfzeile fürs Manifest. Log:
`%LOCALAPPDATA%\evenacadia\nakama-laufzeit\laufzeit.log`.

## Ablauf

1. Diff-Entscheid: ohne Produktpfad `UEBERSPRUNGEN`. Vorher bereinigt der
   Runner seine Besitzliste (Abschnitt Besitz).
2. MCP-Stand gegen `tools/fl/mcp-stand.json` (Abschnitt MCP-Stand); jede
   Abweichung, eine fehlende oder unlesbare Pin-Datei: Exit 3 vor
   Installation, `setup-local.ps1` und dem ersten `uv run`.
3. Installation: `--hashen`, `\Nakama\installieren`, `\Nakama\pruefen`; nur
   „aktuell" für jedes Artefakt lässt den Lauf weiter, sonst Exit 3.
4. Controller-Skript: SHA-256 von `fl_controller/device_FLStudioMCP.py` gegen
   die installierte Kopie; bei Abweichung `setup-local.ps1`, das eigene
   Diagnose-FL beenden und eine Boot-Marke mit der `script_version` des
   Repo-Stands verlangen.
5. Projektordner unter `%LOCALAPPDATA%\evenacadia\nakama-laufzeit\projekt\`
   mit SHA-256 von Repo-Projekt und Arbeitskopie; dazu die Referenzprojekte
   aus Karte U43 unter genau den Namen aus `nulltest-host.json`
   (`Nakama-Diagnose-Referenz.flp`, `Nakama-Diagnose-Verarbeitung.flp`), wenn
   sie neben dem Diagnoseprojekt liegen (P-18). Jeder Ausgang vergleicht alle
   kopierten Projekte; ein verändertes ist Exit 4.
6. Briefkasten `%LOCALAPPDATA%\evenacadia\nakama\diagnose\` mit `antwort\`:
   zuerst eine liegende Anfrage, dann Antworten früherer Läufe entfernen (nur
   Antwort- und Temp-Namen der Instanzen); jeder Ausgang räumt `anfrage.json`
   ab.
7. Renders vor dem FL-Start (nur mit `nulltest-host.json`), die Folge steht
   vorab im Protokoll: `FL64.exe /R /Ewav /O"<Ordner>" "<Arbeitskopie>"` nach
   dem FL-Handbuch („Exporting Audio & MIDI"), ohne FL mit Fenster, Frist 600 s
   je Render, danach genau diese eigene PID beendet; `render.json` im Ordner nennt
   Projekt, SHA-256 des Projekts, Datei, Dauer, Exit, Fenstertitel oder Grund.
   Zuerst das Diagnoseprojekt nach `…\nakama-laufzeit\render\`, danach jedes
   Referenzprojekt aus Karte U43 in der Reihenfolge der Szenariodatei in seinen
   eigenen Ordner `…\render\referenz\<Name>\` (P-21); ein fehlendes
   Referenzprojekt bekommt dort einen Renderstatus mit Grund „Referenzprojekt
   fehlt (Karte U43, K-286-1)" und keinen Render.
8. loopMIDI, eigene Restprozesse, FL-Start, Boot-Marke, Ping; die
   Diagnose-PID ist nur die selbst gestartete.
9. Szenarien alphabetisch; `frischer_start` startet das eigene Diagnose-FL
   vorher neu. Szenario-Exit 3 bricht ab, 5 lässt die Folge weiterlaufen.
   ABWEICHUNG im Nulltest: eigenes Diagnose-FL beenden, messen, dass kein FL
   läuft, `\Nakama\rueckweg`, `\Nakama\pruefen`; die Folgeszenarien entfallen
   mit Meldung, Exit 4. Läuft danach noch ein FL (fremd oder nicht
   beendbar), verweigert der Rückweg mit PID und Titel.

## Besitz: der Runner beendet nur Eigenes (NAK-309, R-309-1')

Eigen ist nur ein FL-Prozess, den dieser Runner gestartet hat. Die Liste
`%LOCALAPPDATA%\evenacadia\nakama-laufzeit\eigene-prozesse.json` hält je
Prozess PID, Startzeit, Befehlszeile (aus `Win32_Process`) und Zweck (`fl`
oder `render`); Besitz gilt nur, wenn der laufende Prozess in allen drei
Merkmalen gleicht — die PID allein vergibt Windows neu. Dazu nennt die
Befehlszeile ein Projekt der Arbeitskopie dieses Laufs (FL: das
Diagnoseprojekt; Render: ein Projekt der Arbeitskopie), und das Hauptfenster
ist leer (NAK-297) oder beginnt mit `<Projektdatei> - `. Eingetragen wird
direkt nach dem Start, ausgetragen nach bestätigtem Ende; der Laufstart
entfernt Einträge ohne passenden lebenden Prozess. Ein FL, das nach einem
Lauf ohne `-Beenden` offen bleibt, ist im nächsten Lauf eigen.

Alles andere ist fremd — auch mit `Nakama-Diagnose.flp` im Titel, auch ohne
Fenster, auch ohne lesbare Befehlszeile, auch bei unlesbarer Liste — und wird
nie beendet. Neben einem fremden FL wird weder beendet noch gemessen: Exit 0
`UEBERSPRUNGEN`, der Grund nennt PID, Titel und warum. Jedes Beenden prüft den
Besitz selbst. Handgriff K-286-2 unter dieser Regel: ein von Hand geöffnetes
Diagnoseprojekt (auch `eq-copilot/fixtures/fl/Nakama-Diagnose.flp` aus dem
Repo) ist fremd; der Runner überspringt, solange es offen ist, und beendet es
nie.

## Antwortzuordnung (NAK-309, R-309-2)

`szenario.py` gibt jedem MCP-Versuch eine neue Anforderungskennung
(`request_id`, 32 Hex-Zeichen), auch der Wiederholung nach einem Timeout. Nur
eine Antwort mit genau dieser Kennung ist eine Messung; eine mit fremder oder
ohne Kennung heißt `UNGEMESSEN`, wird verworfen und gezählt (Szenario-Exit 5,
der Runner macht daraus Exit 3); ein Ping ohne passende Kennung ist Exit 3.
Die Szenariodateien fragen zusätzlich das Echo ab (`track`, `index`,
`mode`). Die Kennung braucht das MCP-Repo ab `0e6912db` (Controller
`SCRIPT_VERSION` 2026-09-18): der Client schreibt sie in den Auftrag und
übergeht Antworten mit fremder Kennung, der Controller kopiert sie in jede
Antwort. Ein Controller alten Stands antwortet ohne Kennung — jeder Schritt
endet dann `UNGEMESSEN`, nie mit einer fremden Messung; `Pruefe-Controller`
installiert den neuen vor dem ersten Trigger. Den Importweg hält
`szenario.py` selbst: `fl_studio_mcp/utils/__init__.py` (lädt `fl_trigger`)
läuft nie, ein Piano-Roll-Modul im Prozess ist Exit 3, und die Ping-Zeile
nennt die geladenen `fl_studio_mcp`-Module.

## MCP-Stand (NAK-309, T3-13-06)

`tools/fl/mcp-stand.json` pinnt das MCP-Repo: Zweig, Commit (`revision`) und
die Git-Blob-Kennung jeder Erstcode-Datei (unabhängig von den Zeilenenden auf
PC oder Laptop). Vor Installation, `setup-local.ps1` und dem ersten `uv run`
verlangt der Runner genau diesen Commit, einen sauberen Arbeitsbaum
(`git status --porcelain --untracked-files=no`) und je Datei dieselbe
Blob-Kennung; die Zeile `MCP-Stand:` im Protokoll nennt Revision, Zweig,
Sauberkeit, Dateizahl und den SHA-256 von `uv.lock` (im MCP-Repo ungetrackt,
je Rechner eigen: protokolliert, nicht verglichen). Der Runner schreibt die
Pin-Datei nie. Nachzug nur per Commit: MCP-Commit auf `evenacadia-local`
nach `origin` pushen (der Laptop prüft denselben Pin), die Blob-Kennungen mit
`git -C <MCP-Repo> rev-parse HEAD:<pfad>` lesen und `revision`, `dateien`,
`stand_vom` und `ticket` in einem datierten Nakama-Commit nachziehen.

## Lokale Aktionen

| Aktion | Wirkung |
|---|---|
| `lokal.briefkasten` | eine Anfrage (128 Bit Zufall), Antworten je Rolle sammeln (Frist 10 s), Auswahl an Name und Größe vor dem Öffnen (2 B bis 16 MiB, keine `.tmp-`), nur aktuelle Kennung und Diagnose-PID; der Broker nur, wenn `eqcop-broker` läuft — der Runner startet ihn nie |
| `lokal.nulltest` | `auslieferung`: `nulltest.py` über `render.json` gegen die Quelle (Weg R1). `verarbeitung_ein` (Projekt `Nakama-Diagnose-Verarbeitung.flp`) und `ohne_slots` (Projekt `Nakama-Diagnose-Referenz.flp`), beide aus Karte U43 im Ordner des Diagnoseprojekts (P-18), lesen den Renderstatus ihres Projekts unter `…\render\referenz\<Name>\` (Weg R2, M-64, P-21): ohne Render Szenario-Voraussetzung mit dem Grund aus dem Renderstatus; `verarbeitung_ein` vergleicht wie R1 gegen die Quelle und verlangt Abweichungen (0 Abweichungen ist verfehlt, der Vergleich wäre blind); `ohne_slots` verlangt den SHA-256 des Datenbereichs, den die `ergebnis.json` des Auslieferungsrenders als `sha256_render_bereich` trägt, ohne bitidentischen Auslieferungsrender Szenario-Voraussetzung. Das Ergebnis steht im Ordner des Projekts; kein Rückweg aus diesen Schritten (M-65) |
| `lokal.fenster` | Bild des FL- oder Plugin-Fensters über `PrintWindow` ohne Vordergrundwechsel unter `…\nakama-laufzeit\bilder\`, als Unterprozess mit erzwungener Frist 5 s (P-19: bei Ablauf beendet, „Frist 5 s ueberschritten", verfehlt); einfarbig oder minimiert ist verfehlt; ein Plugin-Fensterbild unter 200 × 100 Pixel (Breite unter 200 oder Höhe unter 100) ist ein eingeklappter FL-Wrapper: Szenario-Voraussetzung `eingeklappt` mit Breite × Höhe in der Zeile (P-17) |
| `lokal.umlauf` | Anfragen im Sekundentakt bis zum Wrap; gewertete Antwort je Rolle, Materialzeit, Fortlaufbedingung, Kopfverlust K, Referenzausschnitt, Rechnung aus F-28 (`py -3.13 szenario.py --rechne`), Bänder und Zuordnung. Die erwartete `metrics_version` liest der Runner je Schritt aus der Zeile `kMetricsVersion` in `eq-copilot/plugin/src/AnalyseEngine.h`, keine Kopie (R-380-17); fehlt die Datei oder trifft das Muster nicht genau einmal, Szenario-Voraussetzung mit Pfad und Muster, vor der ersten Anfrage und mit gestopptem Transport; eine andere Fassung im Snapshot heißt „Vergleichsbasis verschieden" |
| `lokal.stellen` | Karte U40: je Stelle Lage im Host mit v, Zeitplan aus den Takten der Briefkästen, Zählung nach dem Materialausschnitt des Rahmens, Rohzeilen, Differenzen kumulativer Zähler, Plausibilität ohne Sollwert. Rohzeile je Antwort in der Spaltenfolge `Stelle`, `Rolle`, `durchlauf` (`gemeinsam` oder `getrennt`), `kombinationen` (`<getragen>/<alle>`), `Kennung`, `Zaehlung`, die Felder aus F-23 (`evidenz_frisch` bis `spielt`), `lage im Host` (Hostframes mit `v = …` oder `v unbekannt`), `p_vor ms`, `p_nach ms`; die Differenzzeile je Stelle und Rolle trägt `durchlauf` und `kombinationen` (P-16) |

## Voraussetzungen je Rechner

loopMIDI installiert, Aufgaben `\Nakama\*` registriert
(`tools/dirigent/install-aufgaben-registrieren.ps1`, einmal erhöht), FL Studio
2026 mit dem Controller „FL Studio MCP Controller" auf dem loopMIDI-Port
(Registry, einmalig), `uv` im Pfad, das MCP-Repo unter dem Standardpfad
(`-McpRepo` sonst): privates Repo `https://github.com/evenacadia-tech/fl-studio-mcp`,
Zweig `evenacadia-local`, nach `C:\Users\phili\Projekte\fl-studio-mcp` klonen,
auf der Revision aus `tools/fl/mcp-stand.json` und ohne Änderung an getrackten
Dateien, und dort einmal `setup-local.ps1` ausführen (Controller-Skript in FLs
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
- Nach `mixer.focusEditor` erfasste `lokal.fenster` das Fenster von Nakama
  Probeeq auf Insert 1 im Diagnoseprojekt nur als Kopfleiste des eingeklappten
  FL-Wrappers (`TPluginForm`, 67 × 31 Pixel, 15.09.2026). Aufklappen und
  Speichern ist der Handgriff K-286-2 (Karte U43); bis dahin endet der Schritt
  mit `eingeklappt` (P-17).
- `Testtrack.wav` ist ein Maschinenartefakt (gitignoriert); der Runner sucht
  es in `eq-copilot/kalibration/` des Repos, im sichtbaren Checkout und im
  Desktop-Projekt des Users, sonst `-Sample <pfad>`.

## Grenzen

Screenshot, Nulltest im Host (Auslieferungszustand) und Diagnose-Briefkasten
sind gebaut (NAK-286 Etappen 2 bis 4), der Render mit eingeschalteter
Verarbeitung und ohne Nakama-Slots (Weg R2, M-64) seit P-21. Gemessen wird Weg
R2 im Host erst mit den Projekten aus Karte U43, deren Namen
`nulltest-host.json` trägt (P-18); bis dahin enden beide Schritte mit
Szenario-Voraussetzung. Die Trennung Clip-Lage gegen Latenz bei einem Versatz
leistet Weg R2 nicht: `ohne_slots` verlangt einen bitidentischen
Auslieferungsrender. Im
Diagnoseprojekt antwortet kein Broker,
solange Gen dort `legacy` ist. Der Samplepfad im Projekt ist absolut
(`eq-copilot/fixtures/fl/LIES-MICH.md`); auf einem zweiten Rechner ist der
FL-Dialog „Sample nicht gefunden" zu prüfen. Bänder gegen den Referenzausschnitt
lösen Exit 4 nur mit einer Rechnung aus F-28 aus; ohne sie stehen die Werte roh.

## Uhrzeit

Der Laufzeit-Arm darf zu jeder Uhrzeit laufen, auch nachts und unbeaufsichtigt
(User 19.09.2026, Karte U53, Wortlaut in `docs/plan/fragen.json` unter
`beantwortet.U53`; Register NAK-325). FL Studio startet sichtbar und ist dabei
möglicherweise über das Audiogerät zu hören; am Audiogerät stellt der Runner
und der Dirigent nichts um. Fragen und Handgriffe an den User bleiben auf 9 bis
23 Uhr beschränkt (User 11.09.2026).
