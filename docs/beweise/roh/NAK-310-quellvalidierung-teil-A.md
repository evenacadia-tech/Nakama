# NAK-310 — Quellvalidierung Teil A (lesender Opus-Agent, HEAD f02ae1b4)

**Ticket:** NAK-310 · Planschritt S25i „Tiefenaudit 3 abarbeiten, Teil 2: Pipe-Sicherheit" ·
Etappe 0 „Quellvalidierung", **Teil A** (Installation und Diagnose)
**HEAD:** `f02ae1b481b5584b91ba6ff64521922f82ceeb29` (Zweig `master`)
**Datum:** 23.09.2026 · **Modell:** Opus 5.5 (lesend, Effort max)
**IDs (2):** T3-12-01 (Bündel B02 „Vertrauensanker der erhöhten Installation"), T3-12-04
(Bündel B30 „Diagnosezugriff mit definierter Berechtigung").
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` — Phase 12 `:475-487`
(Befundzeilen `:481`, `:484`, Zähler `:487`), Synthese-Bündel `:72` (B02) und `:100` (B30),
Skeptiker Phase 10 `:549`, `:552` mit Schlusszeile `:557`; Rohberichte unter
`docs/audits/2026-09-15-tiefenaudit/roh/`: `phase-12-pipesicherheit.md` (vollständig, 104
Zeilen), `phase-10-skeptiker.md` (`:30`, `:90`, `:93`, `:120`, `:139-140`),
`phase-11-synthese.md` (`:62`, `:90`, `:106`, `:128`, `:150`, `:171`, `:185-186`),
`phase-09-beweisluecken.md` (`:113`, `:129`, `:247`, `:372`, `:1019-1057`).
**Gate:** Schritt S25i, `docs/plan/plan.json:345` (Feld `text`); Registerzeile
`docs/offene-punkte.md:315`; Manifest `docs/beweise/NAK-310.md` (Kopf `:1-13`, §0 `:15-54`).
**Muster:** `docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md` (Trennung Technik und Produkt,
Kartenprüfung).

**Gate-Text wörtlich** (`docs/plan/plan.json:345`, Feld `text`, vollständig):

> „ANGELEGT 18.09.2026 (Register NAK-310; Tiefenaudit 3 Phase 12 mit gpt-5.6-sol max,
> User-Wort 15.09.2026 „pipe sicherheit eher mit Sol max“; Bündel B02, B04, B05, B30 der
> Synthese, alle P1). INHALT: T3-12-01 (Vertrauensanker der erhöhten Installation:
> Aufgabenaktion und Helfer an unabhängig geschützten Code binden, Artefaktautorisierung
> außerhalb der beschreibbaren Quelle), T3-12-02 (verifizierten Peer und zulässige Rolle
> gemeinsam binden; v2-Token schaltet keine fremde Instanzrolle frei), T3-12-03 und T3-12-05
> (v2-Namenskollision vom v3-Start entkoppeln, v2-Handshake, Leerlauf und Rate begrenzen),
> T3-12-04 (Diagnoseanfrage an kontrollierte Berechtigung binden, Token in Antworten entfernen
> oder maskieren). Sicherheitsarbeit nur in Worker und Prüfer, nie im Dirigentenkontext (User
> 01.09.2026); Prüfer gpt-5.6-sol max, deshalb erst nach dem Codex-Wochenreset, bis dahin
> laufen S25j ff. vor. GRENZE: kein Angriffscode, kein Bruch der eingefrorenen Identität
> (NAK-30), Installer-Rückweg bleibt bytegleich, Probe-Pipe nie Produktions-Pipe. BEWEIS:
> docs/beweise/NAK-310.md mit Matrix, Rotbeweisen an A4 security_vectors, A17/A18, B10, B30,
> vollem Kanon GRÜN und Laufzeit-Arm (Installation über die erhöhte Aufgabe nach der
> Reparatur)."

**Berichtigung einer Auftragsangabe.** Auftrag (Leseliste Punkt 3) und Manifest nennen
`BEFUNDE.md:549` und `:552` „Bestätigungszeilen der Phase 16" (`docs/beweise/NAK-310.md:5`:
„Bestätigung in Phase 16, Zeilen 549 bis 553"; `:33`: „alle in Phase 16 als BESTÄTIGT
geführt"). Das trifft nicht zu. Beide Zeilen stehen im Abschnitt „Phase 10 — Skeptiker"
(`BEFUNDE.md:489-557`). Phase 16 hat die Sicherheitsbefunde ausgeklammert:
„Sicherheitsbereiche, Installation, laufende FL-Projekte, Produktions-Pipes und geparktes
Design waren ausgeschlossen." (`BEFUNDE.md:708`) und „Sicherheitsbefunde T3-12-01 bis T3-12-05
werden hier weder bearbeitet noch neu eingeordnet." (`:796`). Die Skeptikerprüfung hat
Phase 12 „ausschließlich auf Quellenherleitung und Duplikate geprüft; Schwere und Kategorie
werden dort unverändert übernommen und nicht neu begründet" (`roh/phase-10-skeptiker.md:30`,
ebenso `BEFUNDE.md:557`). **Folge:** Schwere und Kategorie beider IDs sind bis heute nur vom
Erstauditor begründet; die Einstufung in diesem Bericht ist die erste unabhängige. Den
Manifestkopf zu berichtigen ist Sache des Dirigenten.

**Prüfform:** Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon, kein Skript, kein
FL Studio, kein Codex, keine Fehlerinjektion, keine Named Pipe, kein Start von Produktteilen,
kein Zugriff auf die Aufgabenplanung oder auf Program Files, kein Angriffscode. Git nur lesend
(`rev-parse`, `status`, `log`, `show`, `diff`, `grep`, `ls-files`). Jede Zeilennummer unten
ist am HEAD `f02ae1b4` selbst nachgelesen, keine aus dem Audit übernommen. Für drei
Windows-Tatsachen, auf die die Fixvorschläge bauen, habe ich die Herstellerdokumentation
gelesen (Adressen in „Nicht geprüft"); am System ist nichts gemessen. Legacy-Bezeichner sind
kein Befund; die Identität bleibt eingefroren (NAK-30); der Designteil ist geparkt, unten
steht keine Oberfläche.

**Arbeitsbaum, geprüft mit `git rev-parse HEAD` und `git status --short`.** Zu Beginn
(08:16:49 Uhr) und unmittelbar vor dem Schreiben dieser Datei (08:40:58 Uhr) identisch: HEAD
`f02ae1b4`, nur die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/`. Der
HEAD ist nicht gewandert; ein Abgleich der Zeilennummern war nicht nötig. Diese Datei ist die
einzige Schreibaktion; `docs/beweise/roh/NAK-310-quellvalidierung-teil-B.md` gehört dem
parallelen Teil B.

**Basis gegen HEAD.** Das Manifest nennt als Basis `80098c3c`; `git diff --stat 80098c3c
f02ae1b4` berührt nur vier Dokumente (Manifest, zwei Auftragsdateien, Registerzeile), keine
zitierte Quellzeile. Phase 12 las am Stand `d20b8c15` (16.09.2026, `BEFUNDE.md:475`). Seither,
je Datei mit `git log d20b8c15..HEAD`:

- **unverändert:** `tools/dirigent/install-aufgaben-registrieren.ps1`,
  `eq-copilot/install/Install-Nakama.ps1`, `eq-copilot/install/NakamaOrdnerHash.ps1`,
  `eq-copilot/plugin/core/diagnose/Briefkasten.h`, `…/Briefkasten.cpp`,
  `eq-copilot/plugin/src/prozessor/Analyse.cpp`, `eq-copilot/plugin/src/DiagnoseAntwort.h`,
  `eq-copilot/plugin/tests/BriefkastenTestMain.cpp`, `broker/tests/briefkasten.rs`,
  `broker/src/lebenslauf.rs`, `broker/src/protokoll.rs`, `broker/src/server.rs`,
  `eq-copilot/schemas/diagnose/`, `eq-copilot/schemas/installer/nakama-installer-v1.md`. Die
  Auditzeilen dieser Dateien treffen am HEAD genau.
- **`broker/src/briefkasten.rs`:** nur `3b2d182c` (NAK-309 Etappe 4), 90 Zeilen im Testmodul ab
  `:1805`; die Produktzeilen `:663`, `:710`, `:787`, `:821-823` sind unverändert.
- **`tools/eq-copilot/pruefe_installer_manifest.py`:** `d1676a63` (NAK-309 Etappe 2) fügt einen
  zweiten Manifestschreiber `--broker-pin` hinzu; die Auditzeilen `:1117-1163` und
  `:1650-1665` liegen heute bei `:1209-1261` (`hashen`) und `:2007-2025` (Aufrufweiche).
- **`tools/fl/laufzeit.ps1`:** `afc6fe60` (NAK-309 Etappe 3) und `d03067de` (NAK-312 Etappe 6);
  die Auditzeilen `:402-416` liegen heute bei `:596-620` (`Installiere`).
- **`eq-copilot/plugin/sonde/SondeProcessor.cpp`:** zehn Commits (NAK-311, NAK-312); die
  Auditzeile `:921` liegt heute bei `:1133`. `diagnoseAntwort` und `briefkastenStarten` sind
  wörtlich gleich (Vergleich der Funktionsrümpfe `d20b8c15` gegen HEAD).
- **`eq-copilot/plugin/src/PluginProcessor.cpp`:** zwei Commits (NAK-309, NAK-312) an anderer
  Stelle; `briefkastenStarten` ist wörtlich gleich.
- **`eq-copilot/install/nakama-installer-v1.json`:** 17 Commits; jeder ändert nur
  `sha256`-Zeilen und `hashes_erzeugt_am` (Hashnachzüge des Laufzeit-Arms).

Kein Commit seit dem Audit hat eine der beiden Quellketten verändert.

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-12-01 (B02) | BESTÄTIGT, vier Ergänzungen: direkt gebrochen ist der Installer-Vertrag §4; der NAK-285-Weg gibt jedem Prozess des Kontos mehr als das verworfene Ordnerrecht; ein befüllter Thumbprint schlösse nichts, weil die Signaturpflicht selbst im beschreibbaren Manifest steht; der Checkout bestimmt auch die Zielnamen, und der erhöhte Prozess schreibt in Benutzerorte | DEFEKT | critical | A17, neuer Block mit Rotmutation an `install-aufgaben-registrieren.ps1:57`; A18, Skriptort ungleich Datenwurzel an `Install-Nakama.ps1:87` und `:96-101`; Vorprüfung des Laufzeit-Arms im Selbsttest A35 | **nein** — NAK-285 entscheidet („schreibrecht ja"); ein Handgriff je Rechner |
| T3-12-04 (B30) | BESTÄTIGT, drei Ergänzungen: der Token steht an drei Stellen und ist in Schema und Code als „persistente Instanzkennung" falsch benannt; A4 verlangt ihn heute ausdrücklich; der Briefkasten läuft in jeder FL-Sitzung, auch in den eigenen des Musikers | LÜCKE (Token: Technikregel; Zulassung: Technikweg ohne Handgriff; Reichweite: Produktentscheid) | medium | A4 neben `broker/src/briefkasten.rs:1469-1493`, Mutation an `:791`/`:821`; B30 neben `BriefkastenTestMain.cpp:1037-1091`, Mutation an `briefkastenStarten` | **ja**, nur die Reichweite — Kartenvorschlag unten |

---

## 1. T3-12-01 — die erhöhte Aufgabe führt Code und Vertrauensanker aus dem beschreibbaren Checkout aus (B02)

*Befundzeile: `BEFUNDE.md:481` · Skeptiker Phase 10: `BEFUNDE.md:549`,
`roh/phase-10-skeptiker.md:90`, `:140` · Synthese: `BEFUNDE.md:72`,
`roh/phase-11-synthese.md:106`, `:150`, `:171`, `:186` · Rohbeleg:
`roh/phase-12-pipesicherheit.md:18`, Schwere `:26`, V-04 `:38`, V-09 `:43`, Vorlauf `:49`,
`:55`*

### a) Behauptung des Audits

Die mit höchsten Rechten registrierte Aufgabe `\Nakama\installieren` führt `Install-Nakama.ps1`
samt Helfer, Manifest und Artefakten aus dem Checkout aus, den derselbe Benutzer ohne Erhöhung
beschreiben kann; das Hashen gegen ein ebenso beschreibbares Manifest und der leere
Signer-Thumbprint sind kein unabhängiger Anker, und ein nicht erhöhter Prozess desselben
Benutzers kann die erhöhte Aktion deshalb umlenken (`BEFUNDE.md:481`).

### b) Quellkette am HEAD

**Station 1 — die Registrierung legt Aktion und Arbeitsverzeichnis in den Checkout.**
`tools/dirigent/install-aufgaben-registrieren.ps1`, einmal erhöht ausgeführt (`:39-44`):

```
:33      [string]$Repo = 'C:\Users\phili\Projekte\Nakama',
:57  $installer = Join-Path $Repo 'eq-copilot\install\Install-Nakama.ps1'
:62  $pwsh = (Get-Command pwsh.exe).Source
:66  $principal = New-ScheduledTaskPrincipal -UserId "$env:USERDOMAIN\$env:USERNAME" -LogonType Interactive -RunLevel Highest
:74      $befehl = "& '$installer'$($modi[$m]) *> '$log'; exit `$LASTEXITCODE"
:75      $arg    = "-NoProfile -ExecutionPolicy Bypass -Command `"$befehl`""
:76      $action = New-ScheduledTaskAction -Execute $pwsh -Argument $arg -WorkingDirectory $Repo
```

`:47` registriert drei Modi (`installieren`, `pruefen` mit `-Pruefen`, `rueckweg` mit
`-Rueckweg`), alle mit demselben Prinzipal `-RunLevel Highest` (`:66`, `:77-78`). `$log` liegt
unter `%LOCALAPPDATA%\evenacadia\nakama-installer\` (`:63-64`, `:71`).

**Station 2 — gestartet wird ohne Erhöhung, routinemäßig.** Der Kopf der Registrierung sagt es
(`:8-12`: „Danach startet eine NICHT erhoehte Session die Aufgabe"). Der Laufzeit-Arm tut genau
das: `tools/fl/laufzeit.ps1:312` `Start-ScheduledTask -TaskPath '\Nakama\' -TaskName $name`,
eingebettet in `Installiere` (`:596-620`) — erst `--hashen` (`:601-604`), dann
`Aufgabe 'installieren'` (`:605-606`), dann `pruefen` (`:609`). Gelaufen ist das zuletzt heute
früh, `docs/beweise/roh/NAK-312-laufzeit-6ecfa301.md:47`: „installieren: Exit 0, Zustand Ready,
Log:   ok      EQ-Copilot installiert  [C:\Program Files\Common Files\VST3\EQ-Copilot.vst3]
…". Der zitierte Log stammt aus der Umleitung der Aufgabenaktion (`laufzeit.ps1:316-320` liest
`%LOCALAPPDATA%\evenacadia\nakama-installer\<name>.log`, `:74` schreibt dorthin) — ein
mittelbarer Beleg, dass die registrierte Aktion der Quelle entspricht. Die Aufgabe selbst habe
ich nicht gelesen (Grenze).

**Station 3 — der erhöhte Prozess lädt Code aus dem Checkout.**
`eq-copilot/install/Install-Nakama.ps1`:

```
:87  $wurzel      = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
:88  $manifestWeg = Join-Path $PSScriptRoot 'nakama-installer-v1.json'
:89  $ergebnisWeg = Join-Path $PSScriptRoot 'install-ergebnis.json'
:96  $ordnerHashWeg = Join-Path $PSScriptRoot 'NakamaOrdnerHash.ps1'
:101 . $ordnerHashWeg
```

Getrackt sind unter `eq-copilot/install/` genau diese drei Dateien (`git ls-files`):
`Install-Nakama.ps1`, `NakamaOrdnerHash.ps1`, `nakama-installer-v1.json`. `$PSScriptRoot` ist
der Checkout; jede Funktion aus `NakamaOrdnerHash.ps1` (`:47-119`) läuft mit dem erhöhten Token.

**Station 4 — die Vertrauensentscheidungen stammen aus dem Checkout.**

```
:215 $manifest   = Get-Content -LiteralPath $manifestWeg -Raw | ConvertFrom-Json
:216 $identWeg   = Loese-Pfad $manifest.identitaetsquelle
:218 $identitaet = Get-Content -LiteralPath $identWeg -Raw | ConvertFrom-Json
:896     $quelle = Loese-Pfad $a.quelle
:908     $ist = Hash-Von $quelle $a.art
:909     if ($ist -ne $a.sha256) {
:922 if ($null -ne $manifest.signatur.authenticode_thumbprint) {
:944     Write-Host '  hinweis Authenticode wird NICHT geprueft - es gibt kein Zertifikat (siehe signatur.warum_null).' -ForegroundColor DarkGray
```

Das Manifest trägt `"authenticode_thumbprint": null` (`eq-copilot/install/nakama-installer-v1.json:16`)
und sagt selbst: „Der SHA-256-Riegel dieses Manifests ist damit HEUTE die ganze Pruefung"
(`:17`). Die Artefaktquellen sind Bauordner im Checkout (`:38`, `:45`, `:51`).

**Station 5 — das Manifest schreibt die Automatik selbst, unmittelbar vor dem Aufgabenstart.**
`tools/eq-copilot/pruefe_installer_manifest.py`, `hashen` (`:1209-1261`) mit `:1242`
`a["sha256"] = artefakt_hash(pfad, art)` und `:1255`
`ziel.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")`;
seit NAK-309 zusätzlich `broker_pin` (`:1294-1330`, `:1328` `os.replace(zwischen, ziel)`), den
`tools/beweise.ps1 -Bauen` zwischen Brokerbau und C++-Bau ruft (Dokstring `:95-103`). Der
Laufzeit-Arm ruft `--hashen` in `laufzeit.ps1:602`, vier Zeilen vor dem Aufgabenstart. Das
Manifest ist damit kein unabhängiger Maßstab, sondern das Protokoll dessen, was gerade im
Bauordner liegt; 17 Hashnachzüge seit dem Audit belegen die Frequenz.

**Station 6 — die Wirkung.** `:1147` `Kopiere-Stand $p.Quelle $p.Ziel $art` schreibt nach
`C:\Program Files\Common Files\VST3\<bundle>` und `C:\Program Files\evenacadia\Nakama\`
(Zielpolicy `:226-237`); `:1149-1152` prüft das Ergebnis gegen dasselbe Manifest.

**Was die vorhandenen Riegel halten (nachgelesen; Phase 12 V-09, `roh/phase-12-pipesicherheit.md:43`).**
Die Zielverzeichnisse sind fest verdrahtet und vom Manifest unabhängig (`:232-237`);
Sicherungen und Transaktionsanker liegen im geschützten Ordner
`Program Files\evenacadia\.nakama-installer` (`:238-240`); Reparse-Punkte werden abgewiesen
(`:634`, `:901`); nach dem Kopieren wird erneut gehasht (`:1149-1152`); der Rückweg vertraut
dem Repo ausdrücklich nicht (`:657-660` „Das Benutzer-Repo liefert nur Daten, niemals
privilegierte Pfade", `:766-768`). Das sind Riegel gegen Drift, Rennen und fremde Journale
innerhalb einer vertrauenswürdigen Ausführung — nicht gegen den Autor von Skript, Helfer und
Manifest.

**Eigene Ergänzung 1 — die Signaturpflicht selbst steht im beschreibbaren Manifest.** `:922`
prüft Authenticode nur, wenn das Checkout-Manifest einen Thumbprint trägt. Wer den Checkout
beschreiben kann, kann das Feld ebenso leeren. Das Befüllen in S34–35 (Register NAK-119,
`docs/offene-punkte.md:114`: „S34–35 liefern Zertifikat und befüllten Thumbprint") schließt den
Befund deshalb nicht; die Installerhälfte der Prüfkette ist bauartbedingt abschaltbar.

**Eigene Ergänzung 2 — der Checkout bestimmt auch, welcher Ordner ersetzt wird.** Der
Bundlename kommt aus der Identitätsdatei, die das Checkout-Manifest benennt (`:216-218`), und
bildet das Ziel (`:434` `$ziel = Kanonischer-Pfad (Join-Path $basis $ident.bundle)`); geprüft
wird nur, dass er ein einzelner Dateiname ist (`:260-263`, `:430-432`). A17 liest die
Identitätsdatei zum Zeitpunkt des Kanonlaufs (`pruefe_installer_manifest.py:1996-1997`), nicht
die, die der erhöhte Lauf liest. Denselben Unterschied benennt der Installer für das Manifest
selbst: A17 „misst das COMMITTETE Manifest", der Installer „das Manifest, das GERADE
ausgefuehrt wird" (`Install-Nakama.ps1:294-296`). Ein Anker muss die eingefrorenen Zielnamen
deshalb mit umfassen.

**Eigene Ergänzung 3 — der erhöhte Prozess schreibt an zwei Orte, die der nicht erhöhte
Benutzer kontrolliert.** Die Aufgabenaktion leitet die gesamte Ausgabe in eine Datei unter
`%LOCALAPPDATA%` um (`install-aufgaben-registrieren.ps1:74`, Ordner `:63-64`), und
`Install-Nakama.ps1` schreibt `install-ergebnis.json` in den Checkout (`:89`,
`Schreibe-Ergebnis` `:182-192`, bei jeder Journaländerung). Heute ist das neben dem
Hauptbefund bedeutungslos; nach einem Fix, der nur den Skriptort schützt, bliebe es eine
Restlücke derselben Klasse.

**Eigene Ergänzung 4 — auch `\Nakama\pruefen` läuft erhöht, obwohl `-Pruefen` nichts
schreibt.** Der Zweig endet bei `:949-963`, vor dem Ruhe-Riegel (`:966`) und der Sperre im
geschützten Ordner (`:972`); gelesen werden Checkout und Installationsziele. Die Erhöhung ist
dort überflüssig und vergrößert die erhöhte Fläche.

**Erreichbarkeit.** Jeder Prozess unter dem angemeldeten Konto, der in den Checkout schreiben
kann — dazu gehören die Bauwerkzeuge und Agenten dieses Kontos —, bestimmt, was beim nächsten
Aufgabenlauf mit höchsten Rechten läuft. Den Lauf muss er nicht selbst auslösen; der
Laufzeit-Arm startet die Aufgabe nach jedem lohnenden Ticket (`CLAUDE.md:211-216`). Weiter
als der Audit beschreibe ich den Weg nicht.

### c) Zusage wörtlich

1. **Installer-Vertrag §4** (`eq-copilot/schemas/installer/nakama-installer-v1.md:298-309`;
   das Dokument ist nach eigenem Wortlaut ein Vertrag, `:5-7`), `:303-307` und `:309`:
   > „Ein per-User-Pfad (`%LOCALAPPDATA%`) ist ohne Rechteerhöhung beschreibbar — jeder
   > Nutzerprozess könnte die Datei austauschen, die ein Plugin später startet. Der
   > Hash-Riegel des Manifests schützt die *Installation*, nicht den späteren Start.
   > Deshalb dieselbe geschützte Ebene wie die Bundles; §53.9 nennt Installationspfad
   > und Betriebssystemschutz ausdrücklich als maßgeblich. […] Das ist eine
   > Technikentscheidung dieses Tickets, kein User-Entscheid."

   Dieselbe Begründung tragen das Manifest (`nakama-installer-v1.json:12`,
   `broker_verzeichnis_warum`) und der Dokstring der A17-Regel
   (`pruefe_installer_manifest.py:817-819`). Über die Aufgabe kann heute jeder Nutzerprozess
   die Datei, die ein Plugin später startet, ohne Rückfrage austauschen lassen — genau das,
   was §4 ausschließt.
2. **Entwurf §48.4** (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3534-3536`; als Gate NAK-123
   zitiert in `docs/beweise/NAK-123.md:15`, dort mit der älteren Stelle `:3243-3245`):
   > „Das schützt vor fremden Windows-Usern, Verwechslung und zufälligen lokalen Clients.
   > Code, der bereits unter demselben User in FL injiziert wurde, liegt außerhalb des
   > IPC-Threat-Models; dafür bleiben Signaturprüfung, Installationspfad und
   > Betriebssystemschutz maßgeblich."

   Die zweite Satzhälfte benennt Installationspfad und Betriebssystemschutz als die
   verbleibende Wehr gegen Code desselben Benutzers. Die Aufgabe hebt beide für den
   Nakama-Installationspfad auf.
3. **Entwurf §48.3** (`:3481-3482`): „…darf nur ein **positiv als Main klassifizierter** Worker
   den signierten Broker aus dem installierten, verifizierten Pfad **ohne Shell** anfordern".
   „Verifiziert" heißt im Plugin: gegen den Pin, den der Bau aus demselben Checkout-Manifest
   erzeugt (`nakama-installer-v1.json:53`) — kein unabhängiger Maßstab.
4. **Technikbegründung zu NAK-285** (`docs/offene-punkte.md:294`; Wortlaut des Dirigenten,
   nicht das User-Zitat): „Technischer Weg statt Schreibrecht auf die Ordner: ein Ordnerrecht
   hätte `Common Files\VST3` und das Broker-Spawnziel unter Program Files für jeden
   Benutzerprozess beschreibbar gemacht … So bleibt Program Files geschützt; die Riegel des
   Installers (Manifest-Hash, Ruhe bei laufendem FL, Rückweg mit NAK-41-Riegel) laufen
   unverändert". Am HEAD gilt das Gegenteil: der gewählte Weg gibt jedem Benutzerprozess mehr
   als das verworfene Ordnerrecht — nicht Schreiben in zwei Ordner, sondern Ausführung
   beliebigen Codes mit dem erhöhten Token.

**Präzisierung.** Der Audit zitiert nur (2). Direkt und ohne Auslegung gebrochen ist (1): ein
Vertrag, der nach `CLAUDE.md:61-62` über dem Entwurf steht. Microsoft führt UAC als
Verteidigung in der Tiefe, nicht als Sicherheitsgrenze (Windows Security Servicing Criteria);
der Befund ist damit keine Windows-Lücke, aber er bricht die projekteigene Zusage, die sich auf
genau diesen Betriebssystemschutz stützt.

### d) Heutige Abdeckung

| Prüfstelle | Behauptung | Was sie misst | Was sie nicht misst |
|---|---|---|---|
| A17 `pruefe_installer_manifest.py` (`tools/beweise.ps1:608`) | „Zielverzeichnisse geschuetzt, Signaturzeile ehrlich …" | die Zielpfade als Zeichenketten im Manifest (`:816-827`), einen Begründungstext bei `null`-Thumbprint (`:830-838`), die WinVerifyTrust-Kette des **Plugins** an einer Systemfixture (`:2060-2061`) | die Aufgabenaktion, den Ort des ausgeführten Skripts, irgendeine Datei- oder Aufgabenberechtigung, ob der Installer die Signatur verlangen muss |
| A18 `pruefe_installer_gegenpfad.py` (`tools/beweise.ps1:617`) | „Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert)" | das echte `Install-Nakama.ps1` in einer Sandbox unter `%TEMP%` ohne Erhöhung (`pruefe_installer_gegenpfad.py:26-32`) | Aufgabe, Erhöhung, Vertrauensanker |
| A35 `selbsttest.py` (`tools/beweise.ps1:642`) | Selbsttest des Laufzeit-Arms gegen Attrappen | Ablauf `--hashen`, Aufgabe, `pruefen` mit Attrappen-Aufgaben (`laufzeit.ps1:1436`, `:1457-1478`) | was die echte Aufgabe ausführt |

Kein Bein liest `tools/dirigent/install-aufgaben-registrieren.ps1` (`git grep` über `tools`,
`eq-copilot`, `broker`: nur Nennungen in `Install-Nakama.ps1:70`, `tools/fl/LIES-MICH.md:140`
und der Fehlermeldung `laufzeit.ps1:311`). Phase 10 hält fest, dass auch ihre Bestätigung „der
Quellkonfiguration, nicht einer ausgelesenen Live-Task-ACL" gilt
(`roh/phase-10-skeptiker.md:90`, `:140`); die Synthese nennt dieselbe Lücke im Prüfsystem
(`roh/phase-11-synthese.md:171`: „Die vier geplanten Wächter haben keinen expliziten Besitzer
für eine erhöhte Skriptkette").

### e) Urteil: BESTÄTIGT

Die Kette schließt am HEAD ohne Lücke; die drei tragenden Dateien sind seit dem Auditstand
`d20b8c15` unverändert. Die zwei gewanderten Stellen (`pruefe_installer_manifest.py`,
`laufzeit.ps1`) tragen dieselbe Aussage an neuer Zeile; NAK-309 hat mit `--broker-pin` einen
zweiten Manifestschreiber hinzugefügt, was die Prämisse stärkt. Vier Ergänzungen: die direkt
gebrochene Zusage ist der Installer-Vertrag §4 (c 1); der NAK-285-Weg ist weiter als die
verworfene Alternative (c 4); ein befüllter Thumbprint schlösse den Befund nicht (b,
Ergänzung 1); Zielnamen aus dem Checkout und erhöhte Schreibwege in Benutzerorte gehören in
denselben Anker (b, Ergänzungen 2 und 3).

### f) Kategorie und Schwere

**DEFEKT, critical.** Gebrochen sind ein geschriebener Vertrag (§4) und die zweite Hälfte des
Threat-Model-Satzes (§48.4). Die Wirkung ist die größte, die es auf diesem Rechner gibt: Code
eines beliebigen Prozesses unter dem Konto des Users läuft ohne Rückfrage mit
Administratorrechten, und Nakamas Binärdateien im geschützten Pfad sind dauerhaft ersetzbar.
Mildernd: Einzelplatzrechner ohne zweites Konto, und wer den Weg nutzt, läuft bereits unter dem
Konto. Gegen eine Herabstufung spricht, dass der Weg nicht selten, sondern routinemäßig durch
die Automatik ausgelöst wird. Ich teile die Einstufung des Audits.

### g) Kleinster Fix

**Was ohne Signaturzertifikat überhaupt möglich ist.** Auf einem Einzelplatzrechner ohne
Zertifikat und ohne Handgriff je Installation kann kein Anker unterscheiden, ob die Automatik
oder ein anderer Prozess desselben Kontos einen neuen Bau freigibt; beide haben dieselben
Rechte. Erreichbar ist, dass der erhöhte Teil nur noch geschützten, festen Code ausführt und
nur in fest benannte Ziele schreibt. Für andere Prozesse des Kontos bleibt dann genau das, was
der User am 12.09.2026 freigegeben hat („schreibrecht ja", NAK-285): den **Inhalt** der zwei
Bundles und der Brokerdatei zu bestimmen — ohne Administratorrechte.

**Anker 1 — geschützter Kopierort (empfohlen).** Kern sind die Punkte 1 bis 3; 4 und 5 gehören
zwingend dazu, sonst bleibt eine Restlücke derselben Klasse; 6 und 7 kosten im selben Satz fast
nichts.

1. Die Registrierung (einmal erhöht) kopiert `Install-Nakama.ps1`, `NakamaOrdnerHash.ps1` und
   die eingefrorene Identitätsdatei in einen Ordner unter `Program Files`, neben den schon
   geschützten Transaktionsordner (`Install-Nakama.ps1:238`). Sie prüft Besitzer,
   Schreibrechte (nur Administratoren, SYSTEM, TrustedInstaller) und Reparse-Freiheit und hält
   die Hashes der Kopien fest. `Program Files` statt `ProgramData`, weil Standardbenutzer unter
   `ProgramData` in der Voreinstellung Unterordner anlegen dürfen (Sekundärquelle, siehe
   „Nicht geprüft").
2. Die Aufgabenaktion ruft ein `pwsh.exe` aus einem geschützten Verzeichnis (die Registrierung
   verweigert sonst; heute nimmt `:62` den ersten Treffer im Suchpfad) mit dem geschützten
   Skript als Datei statt als zusammengesetztem `-Command`, Arbeitsverzeichnis der geschützte
   Ordner. Die Repo-Wurzel geht als ausdrücklicher Datenparameter hinein; `Install-Nakama.ps1`
   leitet Skriptseitiges aus `$PSScriptRoot` ab und Daten nur aus diesem Parameter (heute
   leitet `:87` beides aus dem Skriptort ab).
3. Die drei Aufgaben bekommen einen ausdrücklichen Sicherheitsdeskriptor: der Benutzer darf
   lesen und starten, ändern und löschen dürfen nur Administratoren und SYSTEM. Nach Microsoft
   darf der erstellende Benutzer eine Aufgabe in der Voreinstellung lesen, ändern, löschen und
   starten; eine Registrierung mit höchsten Rechten aus einem nicht erhöhten Prozess ist
   dagegen nicht möglich (Task Scheduler, „Security Contexts for Tasks"). Welche Rechte die
   registrierten Aufgaben heute tragen, ist am lebenden System zu messen (B02:
   „tatsächliche Aufgaben-/Dateirechte getrennt prüfen", `BEFUNDE.md:72`).
4. Der erhöhte Prozess schreibt nur in den geschützten Ordner: Log und Ergebnisjournal dort;
   die Diagnosekopie im Repo legt der nicht erhöhte Laufzeit-Arm selbst an. Betroffen:
   `laufzeit.ps1:316-320` (liest heute den Log unter `%LOCALAPPDATA%`), A17 `[4b]` (liest
   `install-ergebnis.json` aus dem Repo, `pruefe_installer_manifest.py:127`).
5. Der erhöhte Prozess lädt keinen Code aus Suchpfaden, die der Benutzer kontrolliert. Die
   Modulsuche von PowerShell 7 beginnt mit dem Benutzerpfad und übernimmt Einstellungen aus der
   Benutzerumgebung und einer benutzerbezogenen `powershell.config.json` (Microsoft,
   `about_PSModulePath`); das geschützte Skript muss seine Modulauflösung vor dem ersten
   Cmdlet auf die PowerShell-Installation beschränken. Das Wie ist Technik des Bauers, mit
   Gegenprobe.
6. `\Nakama\pruefen` wird ohne `-RunLevel Highest` registriert (b, Ergänzung 4).
7. Ob eine Signatur Pflicht ist und welcher Signer gilt, kommt aus dem geschützten Ort, nicht
   aus dem Checkout-Manifest (heute `:922`). Bis S34–35 steht dort „kein Signer gepinnt";
   danach wird NAK-119 ein Anker statt einer abschaltbaren Prüfung.

*Restlücke:* Inhalt und Hashes der zwei Bundles und der Brokerdatei kommen weiter aus dem
Checkout — das freigegebene Schreibrecht, ohne Administratorrechte. Ändert sich eine der
kopierten Dateien, ist die geschützte Kopie veraltet: der Laufzeit-Arm muss das erkennen und
mit `VORAUSSETZUNG` stehen bleiben, nie auf den Checkout zurückfallen.

**Anker 2 — Hash-Pinning in der Aufgabenaktion, ohne Kopie.** Die Aktion trägt den erwarteten
SHA-256 von Skript und Helfer und führt nur die geprüften Bytes aus (einmal lesen, prüfen,
aus dem Speicher ausführen; ein zweites Lesen öffnet ein Zeitfenster). *Restlücke:* wie
Anker 1, dazu das Zeitfenster bei „prüfen, dann starten"; der Helfer muss mitgepinnt oder
eingebettet sein, jede Änderung verlangt eine neue Registrierung, und ohne Punkt 3 kann die
Aktion selbst geändert werden. Schwächer als Anker 1 ohne Vorteil — nicht empfohlen.

**Anker 3 — Manifest im geschützten Ort.** Hilft nur, wenn nicht der Checkout das geschützte
Manifest schreibt. Schreibt es die Aufgabe aus dem Checkout, ist nichts gewonnen; schreibt es
eine eigene erhöhte Bestätigung je Bau, ist das ein Handgriff je Installation — gegen NAK-285.
Ohne Zertifikat trägt Anker 3 deshalb nur die **eingefrorenen** Teile: Bundlenamen,
Zielverzeichnisse, Brokername (Punkt 1, Identitätskopie); sie ändern sich bis NAK-30 nicht.

**Anker 4 — Codesignatur (S34–35, NAK-119).** Der einzige Anker, der **Inhalte** ohne Handgriff
je Installation freigibt: Zertifikat, ein Signierschlüssel, den Prozesse des Kontos nicht still
benutzen können, und der erwartete Signer im geschützten Ort (Punkt 7).

**Handgriff des Users.** Einmal je Rechner — PC und Laptop — die neue Registrierung erhöht
ausführen; dieselbe Art Handgriff wie bei NAK-285 („je Rechner einmal erhöht",
`CLAUDE.md:220-221`). Danach nur, wenn sich `Install-Nakama.ps1`, `NakamaOrdnerHash.ps1`, die
Identitätsdatei oder die Registrierung ändern. Verlauf: `Install-Nakama.ps1` neun Commits vom
23.08. bis 15.09.2026, seither keiner; `NakamaOrdnerHash.ps1` einer (23.08.); die Registrierung
einer (12.09.).

**Mitzuprüfende Beziehungen.**

- **installieren↔Rückweg bytegleich:** Der geschützte Transaktionsordner bleibt die einzige
  Rückweg-Autorität (`:238-240`, Rückwegzweig `:637-882`). A18 beweist die Logik am
  Repo-Stand, die Aufgabe führt die geschützte Kopie aus; beide müssen bytegleich sein
  (Hashvergleich in Registrierung und Laufzeit-Arm), sonst ist bewiesen, was nicht läuft.
- **registrieren↔entfernen:** `-Entfernen` (`install-aufgaben-registrieren.ps1:49-55`) muss
  Aufgaben **und** geschützte Kopie austragen, aber nie den Transaktionsordner mit einem
  unfertigen Anker (`Install-Nakama.ps1:990-992`).
- **Laufzeit-Arm↔Aufgabe:** Logort (`laufzeit.ps1:316-320`), Wertung von `pruefen`
  (`:609-618`), Attrappen des Selbsttests (A35).
- Die zwei Manifestschreiber (`--hashen`, `--broker-pin`) bleiben Datenlieferanten; an ihnen
  ändert sich nichts.

**Irreführende Texte, die der Fix mitberichtigen muss.**

- `tools/dirigent/install-aufgaben-registrieren.ps1:14-18`: „Program Files bleibt fuer jeden
  anderen Benutzerprozess geschuetzt … die Riegel des Installers - Manifest-Hash, … - laufen
  unveraendert."
- `docs/offene-punkte.md:294` (NAK-285): „So bleibt Program Files geschützt".
- `eq-copilot/install/Install-Nakama.ps1:27-28` („Echtheit - jede Quelldatei wird gegen ihren
  Manifest-Hash geprueft"), `:893` („Riegel 2: Echtheit der Quellen"), `:918` („Quelle echt");
  ebenso „wie ihre Echtheit geprueft wird" in `nakama-installer-v1.json:5` und „wie ihre
  Echtheit geprüft wird" in `nakama-installer-v1.md:4`. Gemessen wird Übereinstimmung mit
  einem gleich beschreibbaren Manifest, keine Echtheit.
- `eq-copilot/schemas/installer/nakama-installer-v1.md:295-296` („Installieren ist ein
  bewusster Admin-Handgriff des Users") und `:445-446` („Sie bleibt ein Klick des Users mit
  Rechteerhöhung (NAK-32); keine Automatik fährt dieses Skript.") — seit NAK-285 falsch.
  NAK-286 M-73 hat `Install-Nakama.ps1` und das Manifest nachgezogen, den Vertrag nicht
  (`docs/beweise/NAK-286.md:639`). Nach `CLAUDE.md:61-63` ist das ein Widerspruch zwischen
  Vertrag und Code, der als Registerzeile zu führen ist.
- `tools/eq-copilot/pruefe_installer_gegenpfad.py:28-29` („Die Installation bleibt ein Klick
  des Users mit Rechteerhoehung") — ebenso veraltet.
- „§53.9 nennt Installationspfad und Betriebssystemschutz …" in `nakama-installer-v1.md:12` und
  `:306`, `nakama-installer-v1.json:12`, `pruefe_installer_manifest.py:817-818`: der Satz steht
  in §48.4 (`Entwurf:3534-3536`); §53.9 ist „IPC-, Coordinator- und Storebindung"
  (`Entwurf:4114`).
- `CLAUDE.md:223-224` („Der Installer verweigert … bei Manifest-Hash-Abweichung") beschreibt das
  Verhalten richtig, liest sich aber als Schutz; es ist eine Driftwache.

### h) Ort des Rotbeweises

- **A17** (`tools/beweise.ps1:608`), `tools/eq-copilot/pruefe_installer_manifest.py`, neuer
  Block nach dem Muster `[3d]` und `[3e]` (`:741-813`: statisches Lesen einer anderen Datei mit
  eingebauten Rotmutationen). Er liest `tools/dirigent/install-aufgaben-registrieren.ps1` und
  verlangt: ausgeführtes Skript aus dem geschützten Ort (nicht aus `$Repo`), Arbeitsverzeichnis
  nicht `$Repo`, `pruefen` ohne `-RunLevel Highest`, ausdrücklicher Sicherheitsdeskriptor,
  keine Umleitung nach `%LOCALAPPDATA%`. **Eingebaute Rotmutation an der Zeile, die die Zusage
  trägt:** der heutige Wortlaut von `:57`
  (`Join-Path $Repo 'eq-copilot\install\Install-Nakama.ps1'`) muss den Block rot machen. Zweite
  Mutation: die Signaturpflicht wieder aus dem Checkout-Manifest (`Install-Nakama.ps1:922`) —
  rot.
- **A18** (`tools/beweise.ps1:617`): Sandboxlauf mit Skriptordner ungleich Datenwurzel und
  einem Köder-Helfer neben der Datenwurzel. Mutation: `Install-Nakama.ps1:96-101` lädt den
  Helfer aus der Datenwurzel, oder `:87` leitet die Datenwurzel aus dem Skriptort ab — der
  Köder wird geladen, das Bein ist rot. Gegenfall: installieren↔Rückweg bleibt bytegleich.
- **Vorprüfung des Laufzeit-Arms** vor `laufzeit.ps1:605-606`: registrierte Aktion,
  Aufgabenrechte und Hash der geschützten Kopie lesen; Aktion im Checkout, vom Benutzer
  änderbare Aufgabe oder veraltete Kopie ergibt Exit 3 `VORAUSSETZUNG`, kein Start. Rotbeweis
  in **A35** mit einer Attrappen-Aufgabe, deren Aktion in den Checkout zeigt: ohne die
  Vorprüfung wird sie gestartet — rot.
- Kein Angriffscode: die Proben prüfen, ob der Anker da ist und ob der Arm verweigert, nicht
  eine Ausnutzung.

---

## 2. T3-12-04 — der Diagnose-Briefkasten beantwortet jede gültige Anfrage und gibt den v2-Sitzungstoken heraus (B30)

*Befundzeile: `BEFUNDE.md:484` · Skeptiker Phase 10: `BEFUNDE.md:552`,
`roh/phase-10-skeptiker.md:93`, `:120` · Synthese: `BEFUNDE.md:100`,
`roh/phase-11-synthese.md:90`, `:128`, `:171`, `:185` · Rohbeleg:
`roh/phase-12-pipesicherheit.md:21`, Schwere `:29`, V-08 `:42`, Vorlauf `:56` ·
Beweisinventar: `roh/phase-09-beweisluecken.md:113`, `:129`, `:247`, `:372`*

### a) Behauptung des Audits

Der Briefkasten unter `%LOCALAPPDATA%` akzeptiert jede syntaktisch gültige Anfrage mit frei
wählbarer 32-Hex-Kennung ohne Zulassung; jeder Prozess desselben Benutzers kann sie anlegen und
die Antwort lesen, das Plugin antwortet mit Snapshot und Rahmen, der Broker mit Aggregat und
dem vollständigen v2-Sitzungstoken (`BEFUNDE.md:484`).

### b) Quellkette am HEAD

**Station 1 — der Briefkasten läuft in jeder Produktinstanz.** Gen startet ihn am Ende des
Produktkonstruktors (`eq-copilot/plugin/src/PluginProcessor.cpp:464-471`, `:470`
`briefkastenStarten (true);`), Probeeq ebenso (`eq-copilot/plugin/sonde/SondeProcessor.cpp:309-318`),
der Broker nach beiden Servern (`broker/src/lebenslauf.rs:170-173`). Die Konfiguration kennt
Wurzel, Rolle und Laufzeitkennung, keine Berechtigung
(`eq-copilot/plugin/core/diagnose/Briefkasten.h:252-258`); der Start prüft nur Konfiguration,
Wurzel und Reparse-Freiheit (`Briefkasten.cpp:917-1006`). Die Wurzel ist fest:
`eq-copilot/plugin/src/DiagnoseAntwort.h:36` `L"evenacadia\\nakama\\diagnose"` unter
`%LOCALAPPDATA%` (`Briefkasten.cpp:453-460`; Broker `broker/src/briefkasten.rs:795-803`). Das
gilt in **jeder** FL-Sitzung mit Nakama-Plugins, auch in den eigenen Sitzungen des Musikers,
nicht nur im Diagnose-FL des Laufzeit-Arms.

**Station 2 — die Annahme hat drei Bedingungen: Form, Kennung, Ring.** C++,
`Briefkasten.cpp`, Takt `rumpf` (`:620-706`):

```
:623         const auto a = fs->attribute (anfrage);
:642             if (a.groesse < 0 || (std::uint64_t) a.groesse > kAnfrageGrenzeBytes)
:654             if (gelesen > kAnfrageGrenzeBytes || ! anfrageLesen (puffer.data(), gelesen, kennung))
:664         if (ringEnthaelt (kennung))
:684         const auto antwort = quelle (anf);
:694         if (schreibe (kennung, antwort))
```

`anfrageLesen` (`:169-245`) nimmt genau `format` und `anfrage_id`, `kennungGueltig`
(`:159-167`) 32 Zeichen aus `0-9a-f`. Rust entspricht dem: `broker/src/briefkasten.rs:607-661`,
`:676` `Some(gelesen) => anfrage_lesen(&puffer[..gelesen]).map_or(Lesung::Fremd, Lesung::Kennung),`,
`anfrage_lesen` `:108-125`. Wer die Datei geschrieben hat, prüft keine Seite; es gibt keinen
Wert, gegen den geprüft werden könnte.

**Station 3 — Gen antwortet mit Messinhalt.** `eq-copilot/plugin/src/prozessor/Analyse.cpp:1344-1393`:
Kopie von Snapshot, Rahmen und Zählern unter der Steuersperre (`:1355-1365`), `:1377`
`u.instanzId = zustand.common.instanceId;`, `:1381` `u.laufzeitId = instanceNonce.toStdString();`,
der Snapshot samt Befundkarten, Label und Paarkennung (`:1330-1341`, `:1386-1388`).

**Station 4 — Probeeq antwortet nach einer zusätzlichen Auswertung.** `SondeProcessor.cpp:1133-1200`:
der erste Takt einer Kennung setzt `:1138` `briefkastenAnfrage.store (true);` — der Worker wertet
daraufhin einmal aus (NAK-286 F-5, `docs/beweise/NAK-286.md:358`: „Ohne Anfrage wertet Probeeq
nie aus") —, der nächste Takt kopiert (`:1150-1161`) und antwortet mit `:1186`
`u.instanzId  = instanz;` und `:1187` `u.laufzeitId = v3RuntimeNonce;`.

**Station 5 — der Broker antwortet mit Aggregat und Token.** Der Token entsteht je Brokerlauf
(`broker/src/lebenslauf.rs:142` `let session_token = uuid::Uuid::new_v4().to_string();`), geht
an den v2-Server (`:152-158`) und an den Briefkasten (`:173`). In `broker/src/briefkasten.rs`:

```
:791     let laufzeit32: String = session_token.chars().filter(|zeichen| *zeichen != '-').collect();
:821         instanz_id: session_token.to_owned(),
:822         instanz16: instanz16(session_token),
:823         laufzeit32,
:714             "instanz_id": self.instanz_id,
:715             "laufzeit_id": self.laufzeit32,
:730         let name = format!("{}.{ROLLE}.{}.{}.{}.json", kennung_text(kennung), self.pid, self.instanz16, self.laufzeit32);
```

Der vollständige Token steht damit an drei Stellen: im Feld `instanz_id` (mit Bindestrichen),
im Feld `laufzeit_id` und im **Dateinamen** (ohne Bindestriche). Das Aggregat trägt zusätzlich
die ersten acht Zeichen (`:685`, `broker/src/aggregat.rs:128`), wie im bestehenden Knopfweg.

**Station 6 — die Wirkung.** Eine Datei im festen Antwortordner (C++ `Briefkasten.cpp:599-617`,
Rust `briefkasten.rs:736-750`), lesbar für jeden Prozess des Kontos; die Instanzen löschen nie
(NAK-286 F-9, `NAK-286.md:362`).

**Eigene Ergänzung 1 — was der Token heute freischaltet.** Der v2-Server gibt ihn jedem
syntaktisch gültigen Hello zurück (`broker/src/protokoll.rs:307-311`,
`BrokerNachricht::Welcome { …, session_token: session_token.to_string() }`) und prüft ihn bei
Heartbeat (`broker/src/server.rs:786`) und Bye (`:865`). Die v2-Pipe lässt nur den aktuellen
Benutzer zu (`server.rs:222-224`, `sicherheit_nur_user` mit
`let sddl = format!("D:P(A;;GA;;;{sid})");`, angewandt `:482`, `:497`), also dieselbe Person,
die das Profil lesen kann. Der Briefkasten reicht den Token damit an niemanden weiter, der ihn
nicht ohnehin bekäme; Audit und Skeptiker (`roh/phase-10-skeptiker.md:120`) stimmen. Ob v2
überhaupt einen gemeinsamen Bearer behält, entscheidet Teil B (T3-12-02).

**Eigene Ergänzung 2 — der Token ist falsch benannt, und so ist er hineingekommen.** Das
Antwortschema beschreibt `instanz_id` als „persistente Instanzkennung (Plugin:
zustand.common.instanceId; Broker: session_token)"
(`eq-copilot/schemas/diagnose/nakama-diagnose-antwort-v1.schema.json:13`, ebenso `:14`), der
Code als „Sitzungskuerzel" (`briefkasten.rs:534`), die Bauregel NAK-286 F-3 ebenso
(`NAK-286.md:356`). Der v2-Vertrag sagt dagegen: „zufällig pro Broker-Lauf; Client zitiert es
in jedem heartbeat" (`eq-copilot/schemas/eq-ipc.schema.json:60`), und der Aggregatvertrag führt
ihn bewusst nur gekürzt: „erste 8 Zeichen des Broker-Session-Tokens — zum Wiedererkennen in
Logs" (`eq-copilot/schemas/eq-aggregat.schema.json:13`). Der Token ist weder persistent noch ein
Kürzel; er ist der Bearer der v2-Sitzung.

**Eigene Ergänzung 3 — der Weg in committete Rohbelege existiert.** Der Runner schreibt
Antwortnamen in die Rohdatei des Laufzeit-Arms (`tools/fl/szenario.py:820-821` gewertete,
`:823-824` mehrdeutige samt `instanz_id` und `laufzeit_id`, `:828` fremde);
`docs/gesundheit/szenarien/snapshot-runde01.json:18` erwartet die Rolle `broker`, wenn er läuft.
Beim letzten Lauf lief er nicht („broker: Broker laeuft nicht - keine Erwartung an die Rolle
broker", `docs/beweise/roh/NAK-312-laufzeit-6ecfa301.md:319`), und `git grep` über die
committeten Laufzeit-Rohdateien findet keinen Brokerantwortnamen. Heute steht also kein Token
im Repo; läuft der Broker während eines Laufs, landet er im nächsten Commit.

**Eigene Ergänzung 4 — eine Anfrage löst in Probeeq Arbeit aus.** Jede neue Kennung erzwingt
eine zusätzliche Auswertung im Worker (Station 4), begrenzt durch den 1-Hz-Takt und eine offene
Kennung je Instanz, ohne Audiowirkung (NAK-286 M-54, `NAK-286.md:594`). Kein eigener Befund.

**Erreichbarkeit.** Jeder Prozess des Kontos; fehlen die Ordner, legt der Anfragende sie an
(die Instanzen legen nie an, F-1, `NAK-286.md:354`).

### c) Zusage wörtlich

**Zur Berechtigung des Anfragenden schweigen die Quellen.** R-286-1
(`docs/beweise/NAK-286.md:350`) regelt den Briefkasten als „Trigger mit einer
Anfragekennung", R-286-2 (`:368`) die Neutralität, die Sicherheitszeilen M-48 bis M-50
(`:583-585`) Pfadtraversal, Reparse-Punkte und Label-Maskierung. Gate S25e (3)
(`NAK-286.md:8`) und KONZEPT (`docs/gesundheit/KONZEPT.md:337-339`) verlangen einen
„Messkanal ohne Klick". Der Threat-Model-Satz (`Entwurf:3534`) schützt „vor fremden
Windows-Usern, Verwechslung und zufälligen lokalen Clients" — das leistet der Briefkasten
(Profilrechte; eine zufällige Datei trifft Format und Kennung nicht); über absichtliche Prozesse
desselben Kontos außerhalb von FL sagt er nichts.

**Zum Token schweigt der Wortlaut, die Richtung ist gesetzt.** Kein Satz nennt den
v2-Sitzungstoken geheim. Drei Nachbarsätze halten andere Geheimnisse aus Diagnose und Export
heraus: die `control_capability` „erscheint nie in Log, Telemetrie oder Export"
(`Entwurf:3521-3522`), das Pairingsecret ist „weder Teil von `state_hash` noch Undo, SQLite,
Diagnose oder normalem Export" (`:4110-4111`), und P9 verlangt ein „Diagnosepaket ohne Audio,
Capabilitysecret, Username oder rohe SID" (`:4436`). Dazu der Aggregatvertrag mit acht Zeichen
(`eq-aggregat.schema.json:13`).

Das Grundgesetz (`CLAUDE.md:28-32`) berührt der Befund nicht: der Briefkasten lässt Audio
bitgleich (NAK-286 M-35, `NAK-286.md:558`).

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B30 `EqCopBriefkastenTest` (`tools/beweise.ps1:777`) | Neutralität, genau eine Antwort je Kennung, eigener Schreibweg, fremd geformte, zu große und verlinkte Anfragen ohne Antwort | Der Gutfall M-24 (`BriefkastenTestMain.cpp:1037-1091`) verlangt, dass eine gültige Anfrage **ohne jede Zulassung** genau eine Antwort mit Snapshot bekommt — das kritisierte Verhalten ist der spezifizierte Gutfall. Einen Fall „gültig, aber nicht zugelassen" gibt es nicht. |
| A4 `cargo test` (`tools/beweise.ps1:511-519`) | führt die Briefkastentests des Brokers mit aus, nennt sie in der Behauptung aber nicht | `anfrage_broker_genau_eine_antwort` verlangt den Token im Namen und im Kopf (`broker/src/briefkasten.rs:1471-1472`, `:1482`); der Ende-zu-Ende-Test verlangt `laufzeit_id` gleich der Laufzeitkennung des Laufs, also dem Token ohne Bindestriche (`broker/tests/briefkasten.rs:115-116`, Zugriff `briefkasten.rs:940-941`). Die Tests **fordern** den Token. |
| A35 `selbsttest.py` (`tools/beweise.ps1:642`) | Auswahl der Antworten im Runner nach Name und Größe vor dem Öffnen | keine Aussage über den Anfragenden |

**Irreführende Wörter.** Die B30-Behauptung (`tools/beweise.ps1:777`: „fremde oder zu grosse
Anfragen … bleiben ohne Antwort") und der Fall M-27 (`BriefkastenTestMain.cpp:1158-1225`,
`fremde_anfragen_werden_ignoriert`) meinen „fremd geformt" (F-10, `NAK-286.md:363`), nicht
„von einem fremden Anfragenden". Wer die Zeile als Sicherheitsaussage liest, glaubt an eine
Zulassung, die es nicht gibt. Ebenso irreführend ist die Beschreibung des Tokens als
„persistente Instanzkennung" (b, Ergänzung 2).

### e) Urteil: BESTÄTIGT

Beide Implementierungen stehen in der Produktlogik unverändert seit dem Audit (einzige
Änderungen: Testmodul in `briefkasten.rs`, Zeilenwanderung in `SondeProcessor.cpp`). Drei
Ergänzungen: der Token steht an drei Stellen und ist als „persistente Instanzkennung" falsch
beschrieben (b, Ergänzung 2); die Tests von A4 verlangen ihn heute ausdrücklich (d); der
Briefkasten läuft in jeder FL-Sitzung, auch in den eigenen des Musikers (b, Station 1). Dass
der Token keine zusätzliche v2-Berechtigung verschafft, bestätige ich an der Quelle (b,
Ergänzung 1).

### f) Kategorie und Schwere

**LÜCKE, medium.** Die Quellen schweigen zur Berechtigung des Anfragenden. Drei Teile, drei
Entscheidungswege:

- **Token aus den Antworten — Technikregel, keine Wahl.** Ein Bearer der v2-Sitzung gehört in
  keine Diagnosedatei; die Nachbarsätze (c) und der Aggregatvertrag geben die Richtung vor.
- **Zulassung — Technikweg ohne Handgriff.** Ein Opt-in des Users je Lauf scheidet aus: der User
  will den Laufzeit-Arm „automatisch ohne nachfragen" (NAK-286, `NAK-286.md:7`), das Gate
  verlangt einen „Messkanal ohne Klick". Eine harte Grenze gegen Prozesse desselben Kontos gibt
  es ohne Handgriff nicht: was der Laufzeit-Arm kann, kann jeder andere Prozess des Kontos
  nachmachen. Technisch erreichbar ist, den Briefkasten an die FL-Sitzung zu binden, die die
  Automatik selbst startet.
- **Reichweite — Produktentscheid.** Ob der Briefkasten in den eigenen Sitzungen des Musikers
  offen bleibt, ändert, was ein anderes Programm dort abholen kann und ob der Dirigent eine vom
  User geöffnete Sitzung ausmessen kann. Kartenvorschlag unter „Produktfragen".

*Schwere medium:* keine Grenze zwischen Windows-Konten verletzt, und der Zugewinn für einen
Angreifer im selben Konto ist klein (Messwerte und v2-Token erreicht er auch anders, T3-12-02).
Es bleibt aber eine im Produkt immer aktive, nicht authentisierte lokale Datenschnittstelle,
die einen Sitzungs-Bearer in Dateien schreibt und ihn über den Runner in committete Belege
tragen kann.

### g) Kleinster Fix

1. **Token entfernen (Technik, sofort baubar).** Der Broker bekommt für den Briefkasten eine
   eigene, zufällige, nicht geheime Laufkennung, die nicht aus `session_token` abgeleitet ist:
   `briefkasten_starten` und `starten_mit` (`briefkasten.rs:779-823`, Ableitung `:791`,
   `:821-823`), Aufrufer `lebenslauf.rs:173`. Name und Kopf behalten ihre Form (32
   Hex-Zeichen); der Runner prüft nur Name gegen Kopf (`szenario.py:93`, `:576-584`) und bleibt
   unverändert. Die acht Zeichen im Aggregat sind bereits maskiert und bestehender Vertrag
   (`eq-aggregat.schema.json:13`); sie auszutauschen wäre Härtung, nicht Teil des kleinsten
   Fixes. Vertrag: die Beschreibungen `nakama-diagnose-antwort-v1.schema.json:13-14` und die
   Regel F-3 ändern ihre Bedeutung für die Brokerrolle; Feldmenge und Muster bleiben. Ob das
   ein Versionsschritt ist, entscheidet der Bauer nach „Schemas sind Verträge"
   (`CLAUDE.md:178-181`); mindestens eine datierte Berichtigung der Beschreibung.
2. **Zulassung an die Sitzung der Automatik binden (Technik; Weg 1 der Karte).** Der
   Laufzeit-Arm startet FL als Kindprozess (`laufzeit.ps1:336-343`) und setzt dabei eine Marke
   in dessen Prozessumgebung; der Broker erbt die Umgebung von FL
   (`eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:618-621`, Umgebungsblock `nullptr`). Gen,
   Probeeq und Broker starten den Briefkasten nur mit Marke (`PluginProcessor.cpp:474-481`,
   `SondeProcessor.cpp:321-328`, `lebenslauf.rs:170-173`); ohne Marke kein Timer und keine
   Existenzprüfung. Das Anfrageformat bleibt, beide Leser bleiben wortgleich (NAK-286 T-26).
   *Optionale Härtung:* ein Zufallsschlüssel je Lauf in der Marke und ein Nachweis in der
   Anfrage — verlangt Anfrageformat v2 in beiden Lesern, Schema und Runner in einem Satz und
   bringt wenig, weil Prozesse desselben Kontos die Umgebung des FL-Prozesses lesen können.
   *Restlücke:* Während eines Laufzeit-Arm-Laufs kann jeder Prozess des Kontos anfragen; das
   Diagnose-FL trägt das Testprojekt, keine Musik des Users. *Bei Weg 2 der Karte* entfällt
   Punkt 2; die erste Gate-Hälfte („an kontrollierte Berechtigung binden") bleibt dann bewusst
   offen und gehört als User-Entscheid ins Manifest.

**Mitzuprüfende Beziehungen.**

- **Anfrage↔Antwort:** der Runner schreibt die Anfrage unverändert (`szenario.py:528`,
  `:553`); geantwortet wird nur aus markierten Sitzungen. „Berechtigter Lauf bleibt messbar"
  (B30-Beweis, `BEFUNDE.md:100`): `snapshot-runde01.json` und `schleife-dauerlauf.json` müssen
  weiter gewertete Antworten liefern.
- **schreiben↔lesen:** das Abräumen durch den Runner (M-55, `NAK-286.md` §4.7) bleibt, die
  Namensform bleibt.
- **starten↔stoppen:** M-38 und M-77 werden um „ohne Marke kein Start, 0 Existenzprüfungen"
  erweitert; der Stoppweg bleibt.
- **Teil B:** den Token fasst T3-12-02 an. Die Entkopplung in Punkt 1 macht den Briefkasten
  davon unabhängig.

### h) Ort des Rotbeweises

- **A4** (`tools/beweise.ps1:511-519`), `broker/src/briefkasten.rs`, neben
  `anfrage_broker_genau_eine_antwort` (`:1469-1493`): weder Dateiname noch ein Kopffeld noch
  das Aggregat enthalten den Sitzungstoken, mit oder ohne Bindestriche. **Mutation an der
  Zeile, die die Zusage trägt:** `:791` beziehungsweise `:821` wieder aus `session_token`
  ableiten — rot. Der Ende-zu-Ende-Test (`broker/tests/briefkasten.rs:115-116`) prüft danach,
  dass `laufzeit_id` die Laufkennung ist und nicht der Token.
- **B30** (`tools/beweise.ps1:777`), `BriefkastenTestMain.cpp`, neuer Fall neben M-24
  (`:1037-1091`): Gen und Probeeq ohne Marke, gültige Anfrage — 0 Antworten, 0 Leseöffnungen,
  0 Existenzprüfungen im Taktfenster. **Mutation:** die Markenprüfung in `briefkastenStarten`
  (`PluginProcessor.cpp:474-481`, `SondeProcessor.cpp:321-328`) entfernen — eine Antwort
  erscheint, rot. Gegenfall: mit Marke genau eine Antwort (M-24 im Inhalt unverändert).
- **A4** für den Brokerstart (`lebenslauf.rs:173`): ohne Marke kein Thread
  `nakama-briefkasten` (Startzähler `briefkasten_starts`, `briefkasten.rs:74-76`).
- **A35:** der Laufzeit-Arm setzt die Marke beim FL-Start (die Attrappe prüft die Umgebung des
  gestarteten Prozesses); fehlt sie, ist der Selbsttest rot.

---

## Gemeinsame Ursachen

**Eine Wurzel: die Automatik läuft als derselbe Benutzer, und beide Entwürfe behandeln „wer in
diesen Benutzerort schreiben kann" als „unsere Automatik".** NAK-285 (Installation über die
Aufgabe, 12.09.2026) und NAK-286 (Briefkasten, 13. bis 15.09.2026) entstanden im selben Ausbau
des Laufzeit-Arms unter dem User-Wort „automatisch ohne nachfragen". Beide geben eine Fähigkeit
an jeden, der einen benutzerbeschreibbaren Ort befüllt — den Checkout für die erhöhte
Installation, `%LOCALAPPDATA%` für die Diagnose —, und beide Male stand daneben ein Satz, der
das Gegenteil annahm („So bleibt Program Files geschützt"; „nur ein Auslöser"). Die Reparaturen
folgen deshalb einem Prinzip: eine Fähigkeit nur an etwas binden, das die Automatik kontrolliert
und andere Prozesse desselben Kontos nicht still ändern können. Für die Installation gibt es
eine echte Grenze (Program Files, einmalige Erhöhung). Für die Diagnose gibt es innerhalb des
Kontos keine; dort heißt die Reparatur: nichts Geheimes herausgeben und nur aktiv sein, wenn
die Automatik die Sitzung selbst gestartet hat.

**Zwei getrennte Änderungssätze.** T3-12-01 berührt `tools/dirigent/`, `eq-copilot/install/`,
`eq-copilot/schemas/installer/`, `tools/eq-copilot/pruefe_installer_*.py` und
`tools/fl/laufzeit.ps1`; T3-12-04 berührt `eq-copilot/plugin/core/diagnose/`, beide
Prozessoren, `broker/src/briefkasten.rs`, `broker/src/lebenslauf.rs`,
`eq-copilot/schemas/diagnose/` und `tools/fl/`. Gemeinsam sind nur `tools/fl/laufzeit.ps1`
(Vorprüfung der Aufgabe gegen Marke beim FL-Start) und der Selbsttest A35: nacheinander bauen,
nie parallel.

**Berührung mit Teil B über den v2-Token.** Die Schnittstelle ist eine einzige Übergabe:
`lebenslauf.rs:142` erzeugt den Token, `:152-158` gibt ihn an den v2-Server, `:173` an den
Briefkasten (`briefkasten.rs:779-823`); der v2-Server gibt ihn jedem gültigen Hello
(`protokoll.rs:307-311`) und prüft ihn bei Heartbeat und Bye (`server.rs:786`, `:865`). Ich habe
nur diese Zeilen gelesen, um die Aussage „keine zusätzliche Berechtigung" zu prüfen; die
Bewertung des v2-Tokens selbst gehört Teil B. Ändert T3-12-02 den Token (etwa: kein gemeinsamer
Bearer mehr), bricht der heutige Briefkasten mit, weil `instanz_id`, `laufzeit_id` und
Dateiname daran hängen — deshalb gehört die Entkopplung (T3-12-04, Punkt 1) vor oder in den
Änderungssatz von T3-12-02. Eine zweite Berührung nenne ich nur: die Plugin-Antworten tragen
`instanceNonce` (Gen) und `v3RuntimeNonce` (Probeeq), Teile der v3-Adresse
(`broker/src/coordinator/link.rs:105`, `:133`); ob ihre Offenlegung für die Identitätsbindung
zählt, ist eine Frage von T3-12-02.

---

## Produktfragen

**Phase 12 nennt keine** (`roh/phase-12-pipesicherheit.md:58-60`; `BEFUNDE.md:487`
„Produktfragen 0"). Die Lektüre ergibt eine Frage, die die Quellen schon beantwortet haben, und
eine echte Wahl.

**T3-12-01 — keine Karte; die Quellen haben entschieden.** NAK-285, User-Wort auf die Frage, ob
die Installation Admin-Handgriff bleibt oder der Dirigent selbst installiert: „schreibrecht ja,
gib mir den befehl" (`docs/offene-punkte.md:294`). Der User hat der Automatik damit Schreibrecht
auf die Installation gegeben. Die Restlücke nach Anker 1 ist genau dieses Schreibrecht, ohne
Administratorrechte. Ein Klick je Installation würde den Rest schließen, widerspräche aber dem
Entscheid; das Zertifikat kommt mit S34–35.

*Folge für das Gate:* dessen zweite Hälfte „Artefaktautorisierung außerhalb der beschreibbaren
Quelle" ist vor S34–35 nur für die eingefrorenen Ziele (Bundlenamen, Verzeichnisse, Brokername)
erfüllbar, nicht für die Inhalte. Der datierte User-Entscheid geht dem Plantext vor; die
Abweichung gehört als Matrixzeile ins Manifest.

*Mitteilung statt Frage:* die Technikbegründung zu NAK-285 („So bleibt Program Files
geschützt") war falsch; das sollte der User in wenigen Sätzen erfahren. Vorschlag für den
Wortlaut:

> „Die automatische Installation hatte eine Lücke: jedes Programm unter deinem Windows-Konto
> hätte darüber Administratorrechte bekommen können. Das wird geschlossen. Was bleibt, ist das
> Schreibrecht, das du am 12.09. freigegeben hast: ein Programm unter deinem Konto könnte
> Nakamas Dateien austauschen, bekommt dadurch aber keine Administratorrechte mehr. Dafür
> brauche ich dich einmal je Rechner, den Laptop eingeschlossen, für einen Klick als
> Administrator."

**T3-12-04 — eine echte Wahl: die Reichweite.** Token und Zulassungsweg sind Technik (f); ein
Opt-in je Lauf schließt NAK-286 aus. Offen ist nur, ob der Briefkasten in den eigenen Sitzungen
des Musikers offen bleibt. Das ändert Produktverhalten: was ein anderes Programm aus einer
laufenden Musik-Sitzung abholen kann und ob der Dirigent eine vom User geöffnete Sitzung
ausmessen kann. Kein Quellsatz entscheidet das. NAK-286 regelt die Automatik; das Angebot des
Users vom 12.09., „eine diagnose fl studio instanz" vorzubereiten, „die immer offen ist"
(`docs/plan/fragen.json:713`, U40 `zwischenantwort`), gilt einer eigenen Diagnose-Instanz, nicht
den Musik-Sitzungen.

**Kartenvorschlag** (Alltagssprache, keine Oberfläche, keine Technik):

> **titel:** Messwerte an andere Programme: nur in den Messläufen der Automatik oder in jeder
> FL-Sitzung?
>
> **was:** Nakama kann seine Messwerte über eine Datei an Werkzeuge herausgeben. Die Automatik
> nutzt das, um nach jedem Bau in FL nachzumessen. Heute ist dieser Weg in jeder FL-Sitzung
> offen, auch in deinen eigenen: jedes Programm, das unter deinem Windows-Konto läuft, kann dort
> Messwerte deines laufenden Projekts abholen — Lautheit, Spektrum, Stereobild und die Namen
> deiner Messpunkte. Am Klang ändert das nichts.
> (1) **Empfohlen:** Der Weg ist nur in den FL-Sitzungen offen, die die Automatik für ihre
> Messläufe selbst startet. In deinen eigenen Sitzungen ist er zu. Soll der Dirigent doch
> einmal eine deiner Sitzungen ausmessen, startest du FL dafür über eine eigene Verknüpfung,
> die er dir gibt.
> (2) Wie heute: offen in jeder Sitzung. Dann kann der Dirigent jederzeit auch eine Sitzung
> ausmessen, die du selbst geöffnet hast.
>
> **warum:** Audit-Befund T3-12-04 (Bündel B30). Ohne Antwort baut der Dirigent Weg 1
> (Planschritt S25i, Ticket NAK-310). Dass ein interner Schlüssel des Brokers nicht mehr in
> diesen Antworten steht, gilt in beiden Fällen und ist nicht Teil der Frage.
>
> **wo:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` T3-12-04, B30;
> `docs/offene-punkte.md` NAK-310; `docs/beweise/roh/NAK-310-quellvalidierung-teil-A.md`.

Produktfragen nur zwischen 9 und 23 Uhr (Dirigentenregel).

---

## Reihenfolge

1. **Etappe 1 — T3-12-01 zuerst.** Das Register stellt B02 vor jede weitere Installation
   (`docs/offene-punkte.md:315`), die Synthese an Platz 2 ohne Abhängigkeit
   (`roh/phase-11-synthese.md:106`), und es ist der einzige critical-Befund dieses Teils.
   - **1a, ohne Handgriff baubar:** Registrierung mit Anker 1 (Punkte 1 bis 7), Datenparameter
     in `Install-Nakama.ps1`, Vorprüfung im Laufzeit-Arm, Rotbeweise A17, A18 und A35,
     Textberichtigungen, Registerzeile zum Widerspruch zwischen Installer-Vertrag und Code.
   - **1b, Handgriff:** der User führt die neue Registrierung einmal erhöht aus, auf dem PC
     **und** auf dem Laptop (die Aufgaben sind rechnerlokal, `CLAUDE.md:220-221`). Bis dahin
     verweigert die Vorprüfung jede Installation mit `VORAUSSETZUNG` — gewollt, weil sonst die
     alte Aufgabe weiter den Checkout ausführt. Erst danach fährt der Laufzeit-Arm die
     „Installation über die erhöhte Aufgabe nach der Reparatur" (Gate-Beweis).
2. **Etappe 2 — T3-12-04, Punkt 1 (Token raus).** Unabhängig von der Karte; vor oder zusammen
   mit T3-12-02 aus Teil B, weil beide den Token anfassen. Die Synthese reiht B30 „nach B04"
   (`roh/phase-11-synthese.md:128`); das gilt für die Zulassung, nicht für die Entkopplung.
3. **Etappe 3 — T3-12-04, Punkt 2 (Zulassung und Reichweite)** nach der Antwort auf die Karte,
   ohne Antwort Weg 1. Nach Etappe 1, weil beide `tools/fl/laufzeit.ps1` und A35 anfassen.

**Sofort und ohne Bau (Dirigentensache):** den Kopf des NAK-310-Manifests berichtigen (Phase 10
statt Phase 16, `docs/beweise/NAK-310.md:5`, `:33`) und den Widerspruch zwischen Installer-Vertrag
§6 und NAK-285 als Registerzeile führen.

---

## Nicht geprüft

- **Der lebende Zustand.** Die registrierten Aufgaben (Aktion, Prinzipal,
  Sicherheitsdeskriptor), die Rechte an Checkout, `%LOCALAPPDATA%` und `Program Files`, der Ort
  von `pwsh.exe`, die UAC-Stufe des Rechners: nicht gelesen (Grenze: keine Aufgabenplanung,
  kein Program Files, kein Prozessstart). Mein Urteil gilt der Quellkonfiguration, mittelbar
  gestützt durch die Rohzeile `NAK-312-laufzeit-6ecfa301.md:47`. Vor dem Bau zu messen.
- **Ob ein nicht erhöhter Prozess die registrierte Aufgabe ändern kann.** Nach Microsoft darf
  der erstellende Benutzer eine Aufgabe in der Voreinstellung lesen, ändern, löschen und
  starten, eine Registrierung mit höchsten Rechten aus einem nicht erhöhten Prozess ist nicht
  möglich. Wie beides für eine Aufgabe gilt, die ein erhöhter Prozess desselben Benutzers
  angelegt hat, ist nicht gemessen und entscheidet über jeden Anker (Anker 1, Punkt 3).
- **Der Laptop.** Seine Registrierung ist aus dem Repo nicht sichtbar.
- **Kein dynamischer Lauf, kein Angriffsversuch.** Dass jeder Prozess des Kontos die Aufgabe
  auslösen kann, stützt sich auf die Quelle (`install-aufgaben-registrieren.ps1:8-12`,
  `laufzeit.ps1:312`), auf die Messung vom 12.09.2026 im Register (`docs/offene-punkte.md:294`)
  und auf die heutige Rohzeile.
- **Die Teil-B-IDs.** T3-12-02, T3-12-03 und T3-12-05 sind nicht validiert. Den v2-Token habe ich
  nur an `lebenslauf.rs:142-173`, `protokoll.rs:307-311`, `server.rs:224`, `:786`, `:865` und
  `eq-ipc.schema.json:60` gelesen. Die Frage nach `instanceNonce` und `v3RuntimeNonce` in den
  Plugin-Antworten gehört dorthin.
- **Die Wirkung der Marke im Host.** Dass FL eine Umgebungsmarke an Plugins und Broker
  durchreicht, ist aus `laufzeit.ps1:336-343` (Start als Kindprozess) und
  `BrokerLifecycle.cpp:618-621` hergeleitet, nicht in FL gemessen; ebenso nicht, ob FL Plugins
  in einem Brückenprozess lädt.
- **Externe Dokumentation, am System nicht gemessen:**
  - UAC als Verteidigung in der Tiefe, keine Sicherheitsgrenze: Microsoft, Windows Security
    Servicing Criteria, https://www.microsoft.com/en-us/msrc/windows-security-servicing-criteria
  - Rechte an Aufgaben und höchste Rechte: Microsoft Learn, Security Contexts for Tasks,
    https://learn.microsoft.com/en-us/windows/win32/taskschd/security-contexts-for-running-tasks
  - Modulsuche in PowerShell 7: Microsoft Learn, about_PSModulePath, Fassung 7.5, Stand
    25.08.2026,
    https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_psmodulepath?view=powershell-7.5
  - Voreinstellung für `ProgramData`, nur Sekundärquelle: Microsoft Q&A,
    https://learn.microsoft.com/en-us/answers/questions/4328564/default-group-permissions-for-programdata-folder
- **Kein Dokuriegel.** `tools/plan/dokuriegel.py` ist ein Skript, der Auftrag verbietet Skripte;
  diese Datei ist ein Rohbeleg, kein Plandokument. `docs/plan/fragen.json` ist nicht geändert,
  der Kartenvorschlag steht nur hier.

---

FERTIG Teil A, 2 bestätigt, 0 präzisiert, 0 widerlegt.
